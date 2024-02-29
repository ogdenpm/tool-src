/****************************************************************************
 *  patch.c is part of abstool                                         *
 *  Copyright (C) 2024 Mark Ogden <mark.pm.ogden@btinternet.com>            *
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// abstool.h should be after std includes
#include "abstool.h"

#ifndef min
#define min(a,b)   ((a) <= (b) ? (a) : (b))
#endif
/*
 * The optional patch file has two modes, PATCH and APPEND, with PATCH being the initial mode
 *
 * Unless part of a string, blanks are ignored and other than $$ a punctuation symbol
 * ends line processing
 *
 * In PATCH mode each line starts with a patch address followed by any number of patch data values.
 * The address is treated as a 16 bit value, a simple hex number or $number or $START can be used
 * In APPEND mode, only patch data values are supported; the patch address is implicit
 *
 * Any number of meta token assignments (see below) can be interspersed between patch values
 *
 * Patch data values
 * =================
 * Patch data values can be either of the following
 *
 * 'APPEND'                  switch to APPEND mode
 * 
 * value ['x' repeatCnt]     where repeatCnt is a hex number and value is one of
 *       number       hex number, no prefix/suffix
 *       'string'     C string escapes \a \b \f \n \r \t \v \' \" \\ \xnn and \nnn are supported
 *       -            set to uninitialised. (error in APPEND mode)
 *       =            leave unchanged i.e. skip the bytes. (error in APPEND mode)
 *       $START       patches with the two byte start address
 *       $number      patches with the 16bit number
 *
 * Meta token assignments
 * ======================
 *
 * metaToken ['='] value
 *      where metaToken is one of
 *      TARGET      issues a warning if the specified target format is different
 *      SOURCE      issues a warning if the actual source file format is different
 *      LOAD        issues a warning if the actual load address is different
 *      START       set the start address if not set, else warn if source file start is different
 *      NAME        sets the name for AOMFxx formats otherwise ignored
 *      DATE        sets the date field for AOM96 otherwise ignored
 *      TRN         sets the TRN value for AOMFxx otherwise ignored. Error if invalid
 *      VER         sets the VER value for AOMF85 otherwise ignored
 *      MAIN        sets the MAIN module value for AOMF85 & AOMF96 (bit 0 only use)
 *      MASK        sets the MASK value f or AOMF51 (low 4 bits only)
 *
 *  and value is one of
 *      fileFormat, used for TARGET and SOURCVE
 *          AOMF51   - Intel absolute OMF for 8051
 *          AOMF85   - Intel absolute OMF for 8080/8085
 *          AOMF96   - Intel absolute OMF for 8096
 *          ISISBIN  - Intel ISIS I binary
 *          HEX      - Intel Hex
 *          IMAGE    - Binary Image
 *
 *      string for NAME and DATE
 *      hex number for all others. Note LOAD and START are word values others are byte values
 *
 *  The genpatch tool can be used to create patch files in the right format
 *
 *  Note APPEND mode is needed to support output files that are not simple binary images.
 *  A normal patch would incorrectly include the extra data within the loaded image.
 *
 */

/* context for parsing*/
enum { PATCHADDRESS, PATCHVAL, APPENDVAL, METAOPT };
int context    = PATCHADDRESS;

char *tokens[] = { "APPEND", "AOMF51", "AOMF85", "AOMF96", "ISISBIN", "HEX",    "IMAGE", "TARGET",
                   "SOURCE", "NAME",   "DATE",   "START",  "LOAD",    "TRN",    "VER",   "MAIN",
                   "MASK",   "$START", "=",      "-",      "HEXBYTE",  "HEXWORD", "STRING", "EOL",   "ERROR" };

typedef struct {
    uint8_t type;
    union {
        unsigned hval;
        char str[256]; // used to store token, for STRING this is in pascal string format else C
    };
    int repeatCnt;
} value_t;

char *skipSpc(char *s) {
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}

char *skipEqu(char *s) {
    while (*(s = skipSpc(s)) == '=')
        s++;
    return s;
}

int parseHex(char *s, char **end) {
    int val = 0;
    s       = skipSpc(s);
    if (isxdigit(*s)) {
        while (isxdigit(*s)) {
            val = val * 16 + (isdigit(*s) ? *s - '0' : tolower(*s) - 'a' + 10);
            s++;
        }
    } else
        val = -1;
    if (end)
        *end = s;
    return val;
}

char *parseToken(char *s, value_t *val) {
    static char simple[]        = "-=\n";
    static uint8_t stype[]      = { DEINIT, SKIP, EOL };
    static char const escchar[] = "abfnrtv'\"\\";
    static char const mapchar[] = "\a\b\f\n\r\t\v'\"\\";
    char token[8];
    char *t;

    s = skipSpc(s);

    if (isalpha(*s) || *s == '$') {
        int i, j;
        token[0] = toupper(s[0]);
        for (i = 1; i < 7 && isalnum(s[i]); i++)
            token[i] = toupper(s[i]);
        token[i] = '\0';

        for (j = APPEND; j <= STARTADDR; j++)
            if (strcmp(tokens[j - APPEND], token) == 0) {
                val->type = j;
                return s + i;
            }
    }

    if (isxdigit(*s) || (*s == '$' && isxdigit(s[1]))) {
        if (*s == '$') {
            val->type = HEXWORD;
            s++;
        } else
            val->type = HEXBYTE;

        val->hval = 0;
        while (isxdigit(*s)) {
            val->hval = val->hval * 16 + (isdigit(*s) ? *s - '0' : tolower(*s) - 'a' + 10);
            s++;
        }
        return s;
    } else if (*s == '\'') {
        int len;
        for (len = 0, ++s; len < 256 && *s && *s != '\''; len++) {
            if (*s == '\\') {
                if (!*++s)
                    break;
                if ((t = strchr(escchar, *s))) {
                    val->str[len + 1] = mapchar[t - escchar];
                    s++;
                } else if (*s == 'x') {
                    int n;
                    if ((n = parseHex(s + 1, &s)) < 0) {
                        warning("Patch file string \\x missing value using 0\n");
                        n = 0;
                    } else if (n > 255)
                        warning("\\x%X is too large, using \\x%X\n", n, n & 0xff);
                    val->str[len + 1] = (uint8_t)n;
                } else if (isdigit(*s)) {
                    int n = 0;
                    for (int i = 0; i < 3 && '0' <= *s && *s <= '7'; s++)
                        n = n * 8 + *s - '0';
                    if (n > 255)
                        warning("\\%o value is too large, using \\%o\n", n, n & 0xff);
                    val->str[len + 1] = (uint8_t)n;
                } else {
                    if (!isprint(*s)) {
                        val->type = ERROR;
                        sprintf(val->str, "String contains non printable char %02XH", *s);
                        return s;
                    }
                    val->str[len + 1] = *s++;
                }
            } else {
                if (!isprint(*s)) {
                    val->type = ERROR;
                    sprintf(val->str, "String contains non printable char %02XH", *s);
                    return s;
                }
                val->str[len + 1] = *s++;
            }
        }
        if (*s == '\'') {
            val->type   = STRING;
            val->str[0] = (uint8_t)len;
        } else {
            val->type = ERROR;
            strcpy(val->str, *s ? "string too long" : "unterminated string");
        }
    } else if ((t = strchr(simple, *s)))
        val->type = stype[t - simple];
    else if (!*s || *s == '\n' || *s == '\r' || ispunct(*s))
        val->type = EOL;
    else {
        val->type = ERROR;
        sprintf(val->str, isprint(*s) ? "Unexpected char %c" : "Non printable char %02XH", *s);
    }
    return s + 1;
}
// scan the string starting at s for a value and optional repeat count
// sets the parsed information in *val (a value_t)
// returns a pointer to the next character to parse

char *getRepeat(char *s, value_t *val) {
    value_t rept;
    s = skipSpc(s);
    if (tolower(*s) == 'x') {
        s = parseToken(s + 1, &rept);
        if (rept.type == HEXBYTE)
            val->repeatCnt = rept.hval;
        else {
            val->type = ERROR;
            strcpy(val->str, "Hex repeat count expected");
        }
    } else
        val->repeatCnt = 1;
    return s;
}

char *getValue(char *s, value_t *val) {
    value_t opt;

    s = parseToken(s, val);
    // get meta value if needed
    if (TARGET <= val->type && val->type <= MASK) {
        if (*(s = skipSpc(s)) == '=')
            s++;
        s = parseToken(s, &opt);
    }

    switch (val->type) {
    case APPEND:
        break;
    case HEXBYTE:
    case HEXWORD:
    case STRING:
    case SKIP:
    case DEINIT:
    case STARTADDR:
        s = getRepeat(s, val);
        break;
    case TARGET:
    case SOURCE:
        if (opt.type < AOMF51 || opt.type > IMAGE) {
            sprintf(val->str, "Invalid value for %s", tokens[val->type - APPEND]);
            val->type = ERROR;
        } else
            val->hval = opt.type;
        break;
    case NAME:
    case DATE:
        if (opt.type == STRING) {
            val->str[0] = min(opt.str[0], val->type == NAME ? 40 : 64);
            memcpy(val->str, opt.str, opt.str[0] + 1);
        } else {
            if (opt.type == ERROR)
                strcpy(val->str, opt.str);
            else
                sprintf(val->str, "%s requires a string value", tokens[val->type - APPEND]);
            val->type = ERROR;
        }
        break;
    case START:
    case LOAD:
        if (opt.type != HEXBYTE || opt.hval > 0xffff) {
            sprintf(val->str, "Invalid value for %s", tokens[val->type - APPEND]);
            val->type = ERROR;
        } else
            val->hval = opt.hval;
        break;
    case TRN:
    case VER:
    case MAIN:
    case MASK:
        if (opt.type != HEXBYTE || opt.hval > 0xff) {
            sprintf(val->str, "Invalid value for %s", tokens[val->type - APPEND]);
            val->type = ERROR;
        } else
            val->hval = opt.hval;
        break;
    case EOL:
    case ERROR:
        break;
    default:
        sprintf(val->str, "Invalid use of %s", tokens[val->type - APPEND]);
        val->type = ERROR;
        break;
    }

    return s;
}

unsigned fill(image_t *image, int addr, value_t *val, bool appending) {
    uint8_t bytepair[2];
    if (addr < image->low)
        image->low = addr;
    bytepair[0] = (uint8_t)val->hval;
    if (val->type == HEXWORD) { // check if writing a word
        val->repeatCnt *= 2;
        bytepair[1] = (uint8_t)(val->hval / 256);
    } else
        bytepair[1] = (uint8_t)val->hval;

    for (int i = 0; i < val->repeatCnt; i++) {
        if (appending) {
            if (addr >= MAXMEM + MAXAPPEND) {
                warning("Too much append data");
                break;
            }
        } else if (addr >= MAXMEM) {
            warning("Patching above 0xffff");
            break;
        }

        switch (val->type) {
        case HEXWORD:
        case HEXBYTE:
            image->mem[addr]   = bytepair[i % 2];
            image->use[addr++] = appending ? APPEND : SET;
            break;
        case SKIP:
            addr++;
            break;
        case DEINIT:
            image->use[addr++] = NOTSET;
            break;
        case STRING:
            for (int i = 1; i <= val->str[0] && addr < (appending ? MAXMEM + MAXAPPEND : MAXMEM);
                 i++) {
                image->mem[addr]   = val->str[i];
                image->use[addr++] = appending ? APPEND : SET;
            }
            break;
        }
    }
    if (!appending && addr > image->high)
        image->high = addr;
    return addr;
}

void patchfile(char *fname, image_t *image) {
    char line[256];
    value_t val;
    char *s;
    int addr      = -1;
    bool append   = false;
    image->padLen = 0; // ignore any input file padding

    FILE *fp      = fopen(fname, "rt");
    if (fp == NULL) {
        fprintf(stderr, "can't load patchfile (ignoring)\n");
        return;
    }

    while (fgets(s = line, 256, fp)) {
        if (!append)
            addr = -1;

        while ((s = getValue(s, &val)), val.type != EOL && val.type != ERROR) {
            switch (val.type) {
            case APPEND:
                if (!append) {  // switch to append mode, ignore further requests
                    append = true;
                    // trim off any deleted data at the end
                    while (image->high > image->low && image->use[image->high - 1] == NOTSET)
                        image->high--;
                    addr = image->high;
                }
                break;
            case TARGET:
                if (image->target != val.hval)
                    warning("patch file was written for target %s not %s",
                            tokens[val.hval - APPEND], tokens[image->target - APPEND]);
                break;
            case SOURCE:
                if (image->source != val.hval)
                    warning("patch file was written for %s source not %s",
                            tokens[val.hval - APPEND], tokens[image->source - APPEND]);
                break;
            case NAME:
                if (val.str[0] == 0 || isdigit(val.str[0]))
                    val.type = ERROR;
                else {
                    for (int i = 1; i <= val.str[0]; i++) {
                        uint8_t c = val.str[i];
                        if (isalnum(c) || c == '?' || c == '@' || c == '_')
                            image->name[i] = toupper(c);
                        else {
                            val.type = ERROR;
                            break;
                        }
                    }
                }
                if (val.type == ERROR)
                    strcpy(val.str, "NAME - module name is invalid");
                else
                    image->name[0] = val.str[0];
                break;
            case LOAD:
                if (image->mLoad != val.hval)
                    warning("patch file was written for %04XH load address not %04XH", val.hval,
                            image->mLoad);
                break;
            case START:
                if (image->mStart >= 0 && image->mStart != val.hval) {
                    warning("patch file was written for %04XH start not %04XH", val.hval,
                            image->mStart);
                    break;
                }
                // fall through, to set start
            case TRN:
            case VER:
            case MAIN:
            case MASK:
                image->meta[val.type - START] = val.hval;
                break;
            case STARTADDR:
                val.hval = image->mStart;
                val.type = HEXWORD;
            case HEXBYTE:
            case HEXWORD:
                if (addr < 0) {
                    addr = val.hval;
                    if (val.repeatCnt != 1)
                        error("Repeat count invalid for patch address");
                    break;
                }
            case STRING:
            case SKIP:
            case DEINIT:
                if (addr < 0) {
                    strcpy(val.str, "Patch data with no patch address");
                    val.type = ERROR;
                } else if (append && (val.type == SKIP || val.type == DEINIT)) {
                    sprintf(val.str, "%s not valid in append mode", tokens[val.type - APPEND]);
                    val.type = ERROR;
                } else
                    addr = fill(image, addr, &val, append);
                break;
            }
            if (val.type == ERROR)
                break;
        }
        if (val.type == ERROR)
            fprintf(stderr, "Invalid patch line: %s in %s\n", val.str, line);
    }
    if (append)
        image->padLen = addr - image->high;

    fclose(fp);
    while (image->low < image->high && image->use[image->low] == NOTSET)
        image->low++;
    while (image->high > image->low && image->use[image->high - 1] == NOTSET)
        image->high--;
    printf("Output file: Load %04XH-%04XH  Start ", image->low, image->high - 1);

    if (image->target == AOMF51 || image->target == AOMF96 || image->target == IMAGE)
        printf("IMPLCIT");
    else if (image->mStart < 0) {
        printf("NOT SET");
    } else
        printf("%04XH", image->mStart);

    if (image->padLen)
        printf("  Padding %04XH", image->padLen);
    putchar('\n');
}
