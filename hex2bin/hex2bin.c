/****************************************************************************
 *                                                                          *
 *  hex2bin: convert intel hex to bin with optional patches                 *
 *  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>            *
 *                                                                          *
 *  This program is free software; you can redistribute it and/or           *
 *  modify it under the terms of the GNU General Public License             *
 *  as published by the Free Software Foundation; either version 2          *
 *  of the License, or (at your option) any later version.                  *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program; if not, write to the Free Software             *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,              *
 *  MA  02110-1301, USA.                                                    *
 *                                                                          *
 ****************************************************************************/

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "showversion.h"

/*
 * hex2bin is similar to obj2bin but for intel hex files cf. Intel OMF85 files
 * It convertes intel hex into a binary image e.g. a CP/M .COM file or an ISIS I bin file
 * In addition, as .COM files often contained additional data to pad to a sector
 * boundary and did not support uninitalised data, the tool allows a patch file to be applied The
 * tool now also supports a -i option to allow Intel .BIN files to be created
 *
 *   THe patch file has two modes, PATCH and APPEND.
 *   Initially the mode is PATCH and if the key word APPEND (case insensitive) is seen at the start
 * of a line the mode switches to APPEND In patch mode each line starts with a patch start address
 * in hex, anything other than a hex value is ignore In append mode the next available address is
 * implied. All other data one the line is assumed to be a set of space separated values to use in
 * one of several formats
 *
 *   value ['x' repeatCnt]
 *       where value can be either
 *           hexvalue
 *           'string'                        note string supports \n \r \t \' and \\ escapes
 *           -                               set to uninitialised (really only useful for Intel .BIN
 * files) =                               leave unchanged i.e. skip the bytes
 *
 *   If the first item on a line doesn't match a hex address in PATCH mode or a value in APPEND mode
 * the line is silently skipped In APPEND mode - and = are treated as 0
 *
 *  whilst anything that doesn't look like a value terminate the line a value followed by a bad
 * repeat count notes an error. The above noted a ; is treated as the end of line
 *
 *   Note APPEND mode is needed for Intel .BIN files to place data after the start address record
 *   A normal patch would incorrectly include the extra data before this start address
 *
 */

char *invokedBy;

int start;

#define MAXMEM                                                                                     \
    0x11000 /* appended data is placed after the program code,  allow 4k for worst case of code    \
               ending at 0xffff */

uint8_t mem[MAXMEM];
uint8_t use[MAXMEM];

/* the use types for data */
enum { UNINITIALISED = 0, DATA, PADDING };

/* types for values */
enum { EOL, HEXVAL, STRING, SKIP, DEINITIALISE, APPEND, INVALID, ERROR };
typedef struct {
    uint8_t type;
    union {
        unsigned hval;
        char *str;
    };
    int repeatCnt;
} value_t;

int low  = MAXMEM;
int high = 0;

bool loadfile(char *s);
int dumpfile(char *s, bool intelBin);
void patchfile(char *s);
void insertJmpEntry();

_Noreturn void usage(char *fmt, ...) {

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    fprintf(stderr,
            "\nUsage: %s [-v | -V | -h  | [-i | -j] infile [patchfile] outfile\n"
            "Where -v/-V provide version information\n"
            "      -h    shows this help\n"
            "      -i    produces Intel bin files\n"
            "      -j    writes a jmp to entry at the start of file after skipping any lxi sp,\n"
            "            the first byte must either be uninitalised or a jmp (0c3h)\n",
            invokedBy);
    exit(1);
}

int main(int argc, char **argv) {
    bool intelBin   = false;
    bool writeEntry = false;
    bool intelHex   = false;
    char *beginAddr = NULL;
    char *endAddr   = NULL;

    invokedBy       = argv[0];

    CHK_SHOW_VERSION(argc, argv);

    while (argc >= 2 && argv[1][0] == '-') {
        switch (tolower(argv[1][1])) {
        case 'h':
            usage(NULL);
            break;
        case 'i':
            intelBin = true;
            break;
        case 'j':
            writeEntry = true;
            break;
        default:
            usage("Unknown option %s\n", argv[1]);
            break;
        }
        argc--;
        argv++;
    }
    if (intelBin && writeEntry)
        usage("-j option invalid for Intel ISIS I bin file");

    if (argc < 3 || argc > 4)
        usage("Incorrect number of files");

    if (loadfile(argv[1])) {

        if (writeEntry)
            insertJmpEntry();

        if (argc == 4)
            patchfile(argv[2]);

        return dumpfile(argv[argc - 1], intelBin);
    } else
        return EXIT_FAILURE;
}

int getHex1(FILE *fp) {
    int c = getc(fp);
    return !isxdigit(c) ? -1 : isdigit(c) ? c - '0' : toupper(c) - 'A' + 10;
}
int getHex2(FILE *fp) {
    int high  = getHex1(fp);
    int low = getHex1(fp);
    return low < 0 ? low : high * 16 + low;
}


int getHex4(FILE *fp) {
    int high = getHex2(fp);
    int low = getHex2(fp);
    return low < 0 ? low: high * 256 + low;
}

bool loadfile(char *file) {
    FILE *fp;
    if ((fp = fopen(file, "rt")) == NULL)
        usage("can't open input file %s\n", file);

    int c;
    while ((c = getc(fp)) != EOF) {
        if (c == ':') {
            int len, addr, type;
            if ((len = getHex2(fp)) < 0 || (addr = getHex4(fp)) < 0 || (type = getHex2(fp)) < 0 || type > 1) {
                fprintf(stderr, "Corrupt hex file in header\n");
                return false;
            }
            uint8_t crc = len + addr / 256 + addr + type;
            uint8_t data[256];
            for (int i = 0; i < len; i++) {
                int c = getHex2(fp);
                if (c < 0) {
                    fprintf(stderr, "Corrupt hex file in data\n");
                    return false;
                }
                data[i] = c;
                crc += c;
            }
            int crcchk = getHex2(fp);
            crc += crcchk;
            if (crcchk < 0 || crc) {
                fprintf(stderr, "Corrupt hex file at CRC\n");
                return false;
            }
            if (type == 1) {
                start = addr;
                break;
            }
            else {
                for (int i = 0; i < len; i++) {
                    mem[addr + i] = data[i];
                    use[addr + i] = DATA;
                }
                if (addr < low)
                    low = addr;
                if (addr + len > high)
                    high = addr + len;
                if (high > 0x10000) {
                    fprintf(stderr, "Ignoring data after 0xffff\n");
                    high = 0x10000;
                }
            }
        }
    }
    fclose(fp);
    printf("Load %04X-%04X, start=%04X\n", low, high - 1, start);
    return true;
}

bool fputblock(FILE *fp, unsigned low, unsigned high, bool intelBin) {
    unsigned len = high - low;
    if (intelBin) {
        fputc(len % 256, fp);
        fputc(len / 256, fp);
        fputc(low % 256, fp);
        fputc(low / 256, fp);
    }
    return fwrite(&mem[low], 1, high - low, fp) == high - low;
}

void insertJmpEntry() {
    unsigned loc;

    loc = use[low] == DATA && mem[low] == 0x31 && use[low + 1] && use[low + 2] ? low + 3 : low;

    if (use[loc] == DATA && mem[loc] != 0xc3) {
        fprintf(stderr, "Failed prechecks: Skipping writing jmp to entry\n");
        return;
    }
    if (start < low || start > high) {
        if (start)
            fprintf(stderr, "skipping writing jmp to start %04X as it is outside image bounds\n",
                    start);
        return;
    }
    mem[loc]   = 0xc3;
    use[loc++] = DATA;
    mem[loc]   = start % 256;
    use[loc++] = DATA;
    mem[loc]   = start / 256;
    use[loc]   = DATA;
}

int dumpfile(char *file, bool intelBin) {
    FILE *fp;
    bool isOk = true;
    int addr;

    if ((fp = fopen(file, "wb")) == NULL)
        usage("can't create output file %s\n", file);

    if (!intelBin)
        isOk = fputblock(fp, low, high, false);
    else {
        unsigned len = 0;
        for (addr = low; addr < high && isOk && use[addr] != PADDING; addr++) {
            if (use[addr] != UNINITIALISED) {
                if (len++ == 0)
                    low = addr;
            } else if (len) {
                isOk = fputblock(fp, low, low + len, true); // write the block initialised data
                len  = 0;
            }
        }
        if (isOk && len)
            isOk = fputblock(fp, low, low + len, true); // write the last block
        if (isOk) {
            fputc(0, fp);
            fputc(0, fp);
            fputc(start % 256, fp);
            fputc(start / 256, fp);
        }

        if (isOk && addr < high)
            isOk = fwrite(&mem[addr], 1, high - addr, fp) == high - addr;
    }
    fclose(fp);
    if (!isOk)
        fprintf(stderr, "write failure on %s\n", file);
    return isOk ? EXIT_SUCCESS : EXIT_FAILURE;
}


char *skipSpc(char *s) {
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

// scan the string starting at s for a value and optional repeat count
// sets the parsed information in *val (a value_t)
// returns a pointer to the next character to parse

char *getValue(char *s, value_t *val) {
    val->repeatCnt = -1;

    s              = skipSpc(s);
    if (_strnicmp(s, "append", 6) == 0) {
        val->type = APPEND;
        return s + 6;
    }
    if (!isxdigit(*s)) {
        if (*s == '\'') {
            val->str  = s + 1;
            val->type = STRING;
            while (*++s && *s != '\'') {
                if (*s == '\\' && s[1])
                    s++;
            }
            if (!*s) {
                val->type = ERROR;
                return s;
            }

        } else if (*s == '-')
            val->type = DEINITIALISE;
        else if (*s == '=')
            val->type = SKIP;
        else {
            val->type = !*s || *s == '\n' || *s == ';' ? EOL : INVALID;
            return *s ? s + 1 : s;
        }
        s++;
    } else {
        unsigned hval = 0;
        while (isxdigit(*s)) {
            hval = hval * 16 + (isdigit(*s) ? *s - '0' : toupper(*s) - 'A' + 10);
            s++;
        }
        val->type = HEXVAL;
        val->hval = hval;
    }
    s = skipSpc(s);
    if (tolower(*s) == 'x') { // we have a repeat count
        s = skipSpc(s + 1);
        if (!isxdigit(*s)) {

            val->type = ERROR; // oops not a valid repeat cht
        } else {
            unsigned hval = 0;

            while (isxdigit(*s)) {
                hval = hval * 16 + (isdigit(*s) ? *s - '0' : toupper(*s) - 'A' + 10);
                s++;
            }
            val->repeatCnt = hval;
        }
    }
    return s;
}

unsigned fill(int addr, value_t *val, bool appending) {
    char *s;

    if (addr < low)
        low = addr;
    if (val->repeatCnt < 0)
        val->repeatCnt = 1;
    for (int i = 0; i < val->repeatCnt; i++) {
        if (addr >= MAXMEM) {
            fprintf(stderr, "appending past end of memory\n");
            break;
        }
        switch (val->type) {
        case HEXVAL:
            mem[addr]   = val->hval;
            use[addr++] = appending ? PADDING : DATA;
            break;
        case SKIP:
            if (use[addr] == UNINITIALISED)
                fprintf(stderr, "Skipping uninitialised data at %04X\n", addr);
            addr++;
            break;
        case UNINITIALISED:
            use[addr++] = UNINITIALISED;
            break;
        case STRING:
            for (s = val->str; *s && *s != '\''; s++) {
                if (addr >= MAXMEM) {
                    fprintf(stderr, "appending past end of memory\n");
                    break;
                }
                if (*s == '\\') {
                    switch (*++s) {
                    case 'r':
                        mem[addr] = '\r';
                        break;
                    case 'n':
                        mem[addr] = '\n';
                        break;
                    case 't':
                        mem[addr] = '\t';
                        break;
                    case '\\':
                        mem[addr] = '\\';
                        break;
                    case '\'':
                        mem[addr] = '\'';
                        break;
                    default:
                        fprintf(stderr, "warning invalid escape char \\%c\n", *s);
                        mem[addr] = *s;
                    }
                } else {
                    if (*s < ' ' || *s > '~')
                        fprintf(stderr, "warning invalid char %02X\n", *s);
                    mem[addr] = *s;
                }
                use[addr++] = appending ? PADDING : DATA;
            }
        }
    }
    if (addr > high)
        high = addr;
    return addr;
}

void patchfile(char *fname) {
    char line[256];
    value_t val;
    char *s;
    bool append = false;
    int addr;

    FILE *fp = fopen(fname, "rt");
    if (fp == NULL) {
        fprintf(stderr, "can't load patchfile (ignoring)\n");
        return;
    }

    while (fgets(line, 256, fp)) {
        for (s = line; *s; s++)
            if ((*s < ' ' && *s != '\r' && *s != '\n') || *(uint8_t *)s >= 0x80)
                usage("\nInvalid patch file: contains binary data");

        s = getValue(line, &val);
        if (val.type == EOL || val.type == INVALID)
            continue;
        if (val.type == ERROR ||
            (!append && (val.type == SKIP || val.type == DEINITIALISE || val.type == STRING))) {
            fprintf(stderr, "invalid patch line: %s", line);
            continue;
        }
        if (val.type == APPEND) {
            if (!append) // only set address on first append detected
                addr = high;
            append = true;
            s      = getValue(s, &val);
        } else if (!append) { // get the start address
            if (val.hval >= 0x10000) {
                fprintf(stderr, "Address (%04X) too big: %s", val.hval, line);
                continue;
            } else if (val.repeatCnt >= 0) {
                fprintf(stderr, "Address cannot have repeat count: %s", line);
                continue;
            }
            addr = val.hval;
            s    = getValue(s, &val);
        }
        // at this point addr has the insert point and val has the first value to put in
        bool ok = true;
        while (ok && val.type != EOL && val.type != INVALID && val.type != ERROR) {
            if (val.type == HEXVAL && val.hval >= 0x100) {
                fprintf(stderr, "bad hex value: %s", line);
                ok = false;
            } else {
                addr = fill(addr, &val, append);
                s    = getValue(s, &val);
            }
        }
        if (val.type == ERROR)
            fprintf(stderr, "bad value: %s", line);
    }

    fclose(fp);
}