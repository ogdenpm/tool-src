/****************************************************************************
 *                                                                          *
 *  program: modomf85 - modify omf85 files                                  *
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
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <io.h>

#define MAXNAME  31
#define CHUNK   32

void showVersion(FILE *fp, bool full);
char *program;

uint8_t recBuf[0x10000];        // big enough for any record

#define getRecType(buf)    ((buf)[0])
#define setRecType(buf, n)  ((buf)[0] = n)
#define getRecLen(buf)     ((buf)[2] * 256 + (buf)[1])
#define setRecLen(buf, n)  ((buf)[1] = (n) % 256, (buf)[2] = (n) / 256)

enum { MODHDR = 2, MODEND = 4, CONTENT = 6, LINNUM = 8, ENDFIL = 0xE, ANCEST=0x10, LOCALS = 0x12, PUBLIC = 0x16, EXTERN = 0x18,
       EXTREF = 0x20, RELLOC = 0x22, INTSEG = 0x24, LIBLOC = 0x26, LIBNAM = 0x28, LIBDIC = 0x2A, LIBHDR = 0x2C, COMMON=0x2E};

typedef struct {
    const uint8_t *oldname;
    const uint8_t *newname;
} rename_t;

typedef struct {
    uint16_t    loc;
    uint8_t     val;
} patch_t;


int addrSeg = -1;

int inRecCnt = 0;
int outRecCnt = 0;


typedef struct {
    int trn;
    int ver;
    bool lflag;
    bool mflag;
    struct {
        int cnt;
        patch_t *patchList;
    } patches;
    struct {
        int cnt;
        rename_t *renameList;
    } renames;
    struct {
        int cnt;
        uint16_t *locs;
    } splits;
    const char *infile;
    const char *outfile;
} option_t;


void *safeRealloc(void *p, size_t size) {
    if ((p = realloc(p, size)) == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return p;
}

void addRename(const char *oldname, const char *newname, option_t *options) {
    if (options->renames.cnt % CHUNK == 0)
        options->renames.renameList = safeRealloc(options->renames.renameList, (options->renames.cnt + CHUNK) * sizeof(rename_t));
    options->renames.renameList[options->renames.cnt].oldname = oldname;
    options->renames.renameList[options->renames.cnt++].newname = newname;

}

void addPatch(uint16_t loc, uint8_t val, option_t *options) {
    if (options->patches.cnt % CHUNK == 0)
        options->patches.patchList = safeRealloc(options->patches.patchList, (options->patches.cnt + CHUNK) * sizeof(patch_t));
    options->patches.patchList[options->patches.cnt].loc = loc;
    options->patches.patchList[options->patches.cnt++].val = val;
}
void addSplit(uint16_t loc, option_t *options) {
    if (options->splits.cnt % CHUNK == 0)
        options->splits.locs = safeRealloc(options->splits.locs, (options->splits.cnt + CHUNK) * sizeof(uint16_t));
    // locs need to be maintained in address order in case of multiple splits in a record
    int i;
    for (i = options->splits.cnt++; i != 0 && loc < options->splits.locs[i - 1]; i--)
        options->splits.locs[i] = options->splits.locs[i - 1];

    options->splits.locs[i] = loc;
}


void usage() {
    printf("Usage: %s [-(v|V)] |  [-l] [-m] [-p file] [-t(f|p|u)]  [-v hh] infile [outfile]\n"
        "Where:\n"
        "  -v | -V     shows version information - must be only option\n"
        "  -l          remove @Pnnnn library references\n"
        "  -m          clear the main module flag\n"
        "  -p file     parses the file for patch information. See below\n"
        "  -tf         sets translator to FORT80\n"
        "  -tp         sets translator to PLM80\n"
        "  -tu         sets translator to Unspecified/ASM80\n"
        "  -v hh       sets version to hh hex value\n"
        "  outfile     optional output file, default is to replace infile\n"
       "\n"
        ""
        "Using the -p option supports more advanced patching\n"
        "the file can contain multiple instances of the following line types\n"
        "p addr [val]*         patch from addr onwards with the given hex values\n"
        "                      addr is absolute for apps, else code relative\n"
        "r oldname [newname]   renames public/external symbols from oldname to newname\n"
        "                      names are converted to uppercase and $ is ignored\n"
        "                      omitting newname deletes, only vaild for public\n"
        "                      valid chars are ?@A-Z0-9 and length < 32\n"
        "s addr                force split in record at absolute addr\n"
        "text from # onwards is treated as a comment and blank lines are skipped\n"
        "\n"
        "WARNING: When patching CSEG, relocation records are not modified\n"
        , program);

}


char *formats[] = {
    NULL,           // 00
    "S2R4",         // MODHDR
    "4R1",          // MODEND
    "3R1",          // CONTENT
    "1R4",          // LINNUM
    NULL, NULL,     // 0A, 0C
    "",             // ENDFIL
    "S",            // ANSTOR
    "1R2SZ",        // LOCALS
    NULL,           // 14
    "1R2PZ",        // PUBLIC
    "REZ",          // EXTERN           
    NULL, NULL, NULL, // 1A, 1C, 1E
    "1R4",          // EXTREF
    "1R2",          // RELOC
    "2R2",          // INTSEG
    "R4",           // LIBLOC
    "RS",           // LIBNAM
    "RS",           // LIBDIC
    "6",            // LIBHDR
    "R1S"           // CNAMES
};

const char *skipBlank(const char *s) {
    while (isblank(*s))
        s++;
    return s;
}

const char *parseName(const char **src) {
    char name[MAXNAME + 2];
    uint8_t nameLen;
    const char *s = skipBlank(*src);
    for (nameLen = 0; isalnum(*s) || *s == '$' || *s == '@' || *s == '?'; s++) {
        if (*s != '$' && nameLen < MAXNAME)
            name[1 + nameLen++] = toupper(*s);
    }
    name[0] = nameLen;      // make pascal style, a zero length will return ""
    name[1 + nameLen] = 0;
    *src = s;
    return _strdup(name);
}


bool parsePatch(const char *s, option_t *options) {
    s = skipBlank(s);
    char *end;

    unsigned long loc;
    unsigned long val;
    if (!isxdigit(*s))
        return false;
    loc = strtoul(s, &end, 16);
    if (loc > 0xffff)
        return false;

    while (1) {
        s = skipBlank(end);
        if (*s == '\n' || *s == '#')
            return true;
        val = strtoul(s, &end, 16);
        if (s == end || val > 255 || !isxdigit(*s))
            return false;
        addPatch((uint16_t)loc++, (uint8_t)val, options);
    }
}

bool parsePatchFile(char *file, option_t *options) {
    char line[257];
    const char *oldname, *newname;
    unsigned long loc;
    FILE *fp = fopen(file, "rt");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open patch file %s\n", file);
        return false;
    }
    bool ok = true;
    while (ok && fgets(line, 256, fp)) {
        ok = false;
        if (!strchr(line, '\n')) {
            fprintf(stderr, "Error: Patch file %s - line too long %s\n", file, line);
            break;
        }
        const char *s = skipBlank(line);
        if (tolower(*s) == 'r') {
            s++;
            oldname = parseName(&s);
            newname = parseName(&s);
            s = skipBlank(s);
            if (*oldname == 0 || (*s != '\n' && *s != '#'))
                fprintf(stderr, "Invalid rename line: %s\n", line);
            else {
                addRename(oldname, newname, options);
                ok = true;
            }
        } else if (tolower(*s) == 'p') {
            if (!(ok = parsePatch(s + 1, options)))
                fprintf(stderr, "Invalid patch line: %s", line);
        } else if (tolower(*s) == 's') {
            char *end;
            s = skipBlank(s + 1);
            if (!isxdigit(*s) || (loc = strtoul(s, &end, 16)) > 0xffff || (*(s = skipBlank(end)) != '\n' && *s != '#'))
                fprintf(stderr, "Invalid split line: %s", line);
            else {
                addSplit((uint16_t)loc, options);
                ok = true;
            }
        } else if (*s != '\n' && *s != '#')         // blank line and # lines are ok
            fprintf(stderr, "Unknown advanced option %s\n", s);
    }
    fclose(fp);
    return ok;
}


// check the record format
// returns number of repeated components or -1 if error
// in case of error it will print the reason to stderr
int chkRecord(uint8_t *recBuf) {
    char *fmt;
    uint8_t type = getRecType(recBuf);
    uint16_t len = getRecLen(recBuf);       // record len

    if (type % 2 || type > 0x2e || !(fmt = formats[type / 2])) {
        fprintf(stderr, "Error: Illegal record %d type %02XH\n", inRecCnt, type);
        return -1;
    }

    if (len > 1025 && type != CONTENT) {
        fprintf(stderr, "Error: Record %d type %02XH, length %d exceeds limit\n", inRecCnt, type, len);
        return -1;
    }
    len += 2;                               // len now index of last non crc byte
    uint16_t pos;                           // index of current data in recBuf
    for (pos = 3; pos < len && *fmt && *fmt != 'R'; fmt++) {
        if (isdigit(*fmt))
            pos += *fmt - '0';
        else                                  // is a string type, note Z not used for fixed part of record
            pos += recBuf[pos] + 1;
    }
    uint16_t cnt = 0;                       // number of repeated items
    if (pos < len) {
        if (!*fmt) {
            fprintf(stderr, "Error: Extra information at end of record %d type %02XH\n", inRecCnt, type);
            return -1;
        }
        char *rep = ++fmt;
        if (isdigit(*fmt) && fmt[1] == 0) {     // quick calculation so we don't check long content records byte by byte
            uint16_t left = len - pos;
            if (left % (*fmt - '0')) {
                fprintf(stderr, "Error: Badly formed repeating section of record %d type %02XH\n", inRecCnt, type);
                return -1;
            }
            return left / (*fmt - '0');
        }

        while (pos < len) {
            fmt = rep;
            for (fmt = rep; *fmt; fmt++) {
                if (isdigit(*fmt))
                    pos += *fmt - '0';
                else if (*fmt == 'Z') {
                    if (recBuf[pos] != 0) {
                        fprintf(stderr, "Warning: Record %d type %02XH - field should be Zero - fixing\n", inRecCnt, type);
                        recBuf[pos] = 0;
                    }
                    pos++;
                } else {                 // must be string type
                    if (recBuf[pos] > MAXNAME)
                        fprintf(stderr, "Warning: Record %d type %02XH - name %.*s > %d characters\n", inRecCnt, type, recBuf[pos], &recBuf[pos + 1], MAXNAME);
                    pos += recBuf[pos] + 1;
                }
            }
            cnt++;
        }
    }
    if (pos > len) {
        fprintf(stderr, "Error: Missing information for record %d type %02XH\n", inRecCnt, type);
        return -1;
    }
    return cnt;
}

int readRecord(FILE *fpin, uint8_t *recBuf) {
    if (fread(recBuf, 1, 3, fpin) != 3) { // read type and len
        fprintf(stderr, "Error: EOF reading record %d\n", inRecCnt + 1);
        return -1;
    }
    uint16_t len = getRecLen(recBuf);
    if (fread(recBuf + 3, 1, len, fpin) != len) {
        fprintf(stderr, "Error: EOF reading record %d\n", inRecCnt + 1);
        return -1;
    }
    inRecCnt++;
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len + 3; i++)
        crc += recBuf[i];
    if (crc != 0)
        fprintf(stderr, "Warning: CRC error in record %d\n", inRecCnt++);
    return chkRecord(recBuf);
}



bool writeRecord(FILE *fpout, uint8_t *recBuf, uint16_t len) {
    if (len == 0)
        len = getRecLen(recBuf);
    else
        setRecLen(recBuf, len);
    uint8_t crc = 0;                    // calc the new crc (exclude the crc byte itself)
    for (int i = 0; i < len + 2; i++)
        crc += recBuf[i];
    uint8_t tmp = recBuf[len + 2];      // protect the buffer byte in case the record is being split
    recBuf[len + 2] = 0 - crc;
    size_t actual = fwrite(recBuf, 1, len + 3, fpout);
    recBuf[len + 2] = tmp;
    outRecCnt++;
    if (actual == len + 3)
        return true;
    fprintf(stderr, "Error writing record %d\n", outRecCnt);
    return false;

}

bool appendItem(FILE *fpout, uint8_t *recBuf, uint8_t *item, uint16_t ilen, uint16_t  fixedLen) {
    bool ok = true;
    uint16_t len = getRecLen(recBuf);

    if (len + ilen >= 1025) {
        ok = writeRecord(fpout, recBuf, len + 1);       // allow for crc byte
        len = fixedLen;
    }
    memcpy(&recBuf[len + 3], item, ilen);
    len += ilen;
    setRecLen(recBuf, len);
    return ok;
}



bool rewriteModhdr(FILE *fpout, uint8_t *recBuf, const option_t *options) {
    int trnIdx = recBuf[3] + 4; // skip name
    int verIdx = trnIdx + 1;

    if (recBuf[trnIdx] > 2) {
        fprintf(stderr, "ERROR: not an OMF85 object or executable file\n");
        return false;
    }
    if (options->trn >= 0)
        recBuf[trnIdx] = options->trn;
    if (options->ver >= 0)
        recBuf[verIdx] = options->ver;

    return writeRecord(fpout, recBuf, 0);
}

bool rewriteModend(FILE *fpout, uint8_t *recBuf, const option_t *options) {
    if (options->mflag)
        memset(recBuf + 3, 0, 4);               // clear mod type, seg & offset
    return writeRecord(fpout, recBuf, 0);
}

bool rewriteContent(FILE *fpout, uint8_t *recBuf, int repCnt, const option_t *options) {
    uint16_t len = getRecLen(recBuf);
    uint16_t offset = recBuf[5] * 256 + recBuf[4];

    if (recBuf[3] == addrSeg) {
        for (int i = 0; i < options->patches.cnt; i++) {
            if (offset <= options->patches.patchList[i].loc && options->patches.patchList[i].loc < offset + repCnt)
                recBuf[6 + options->patches.patchList[i].loc - offset] = options->patches.patchList[i].val;
        }

    }
    if (recBuf[3] == 0) {   // only split absolute addresses
        for (int i = 0; i < options->splits.cnt; i++) {
            if (options->splits.locs[i] > offset && options->splits.locs[i] < offset + repCnt) {
                uint16_t splitPt = options->splits.locs[i] - offset;
                if (!writeRecord(fpout, recBuf, splitPt + 4))       // alloc for seg, offset + crc
                    return false;
                memmove(recBuf + 6, recBuf + 6 + splitPt, len - splitPt);
                len -= splitPt;
                offset += splitPt;
                setRecLen(recBuf, len);
                recBuf[5] = offset / 256;
                recBuf[4] = offset % 256;
            }
        }
    }
    return writeRecord(fpout, recBuf, 0);
}

// check name for rename, return the new name if it exists, else return name
// name is in pascal string format
const uint8_t *getNewName(const uint8_t *name, uint8_t rtype, const option_t *options) {
    unsigned libfn;
    if (rtype == 'P' && options->lflag && sscanf(name, "\x06@P%4d", &libfn) == 1 && 0 <= libfn && libfn <= 116)
        return "";

    for (int i = 0; i < options->renames.cnt; i++)
        if (memcmp(name, options->renames.renameList[i].oldname, *name + 1) == 0) // name sure name matches
            if (options->renames.renameList[i].newname[0] == 0 && rtype == 'E') {
                fprintf(stderr, "Warning: Record %d - cannot delete external name %.*s\n", inRecCnt, *name, name + 1);
                return name;
            } else
                return options->renames.renameList[i].newname;
    return name;
}



bool rewriteNames(FILE *fpout, uint8_t *recBuf, int repCnt, const option_t *options) {

    if (!options->lflag && options->renames.cnt == 0)   // no changes possible, just copy
        return writeRecord(fpout, recBuf, 0);

    uint8_t type = getRecType(recBuf);

    uint8_t newRec[1028];                               // where the new record is built
    setRecType(newRec, type);                           // init from existing type
    setRecLen(newRec, 0);
    char *fmt;
    uint16_t pos = 3;                                   // copy start point

    for (fmt = formats[type / 2]; *fmt && *fmt != 'R'; fmt++) {
        if (isdigit(*fmt)) {
            memcpy(newRec + pos, recBuf + pos, *fmt - '0');
            pos += *fmt - '0';
        } else {                                        // a string, no Z for fixed part
            memcpy(newRec + pos, recBuf + pos, recBuf[pos] + 1);
            pos += recBuf[pos] + 1;
        }
    }

    uint16_t fixedLen = pos - 3;
    setRecLen(newRec, fixedLen);

    char *rept = fmt + 1;
    uint8_t reptItem[2 + 1 + 255 + 1];      // worst case record PUBLIC with 255 byte name!!!

    bool ok = true;
    while (ok && repCnt-- > 0) {
        uint16_t rlen = 0;
        bool skip = false;
        for (fmt = rept; *fmt; fmt++) {
            if (isdigit(*fmt)) {
                memcpy(reptItem + rlen, recBuf + pos, *fmt - '0');
                rlen += *fmt - '0';
                pos += *fmt - '0';
            } else if (*fmt == 'Z')
                reptItem[rlen++] = recBuf[pos++];
            else {
                const uint8_t *name = getNewName(&recBuf[pos], *fmt, options);
                if (*name == 0)
                        skip = true;
                memcpy(reptItem + rlen, name, *name + 1);
                rlen += *name + 1;
                pos += recBuf[pos] + 1;
            }
        }
        if (!skip)
            ok = appendItem(fpout, newRec, reptItem, rlen, fixedLen);
    }
    //
    if (ok && getRecLen(newRec) != fixedLen)
        return writeRecord(fpout, newRec, getRecLen(newRec) + 1);   // allow for crc
    return ok;
}

bool rewriteObj(FILE *fpin, FILE *fpout, const option_t *options) {
    int repCnt;
    bool ok = true;
    while (ok && (repCnt = readRecord(fpin, recBuf)) >= 0) {
        if (inRecCnt == 1 && getRecType(recBuf) != MODHDR) {
            fprintf(stderr, "ERROR: not an OMF85 object or executable file\n");
            return false;
        } else if (inRecCnt != 1 && getRecType(recBuf) == MODHDR) {
            fprintf(stderr, "ERROR: multiple modules not supported\n");
            return false;
        }

        switch (getRecType(recBuf)) {
        case LINNUM: case ANCEST: case LOCALS: case RELLOC: case INTSEG: case COMMON: case EXTREF: case ENDFIL:
            ok = writeRecord(fpout, recBuf, 0);
            if (getRecType(recBuf) == ENDFIL)
                return ok;
            break;
        case MODHDR:
            ok = rewriteModhdr(fpout, recBuf, options);
            addrSeg = repCnt != 0;      // ABS if no seg info, else CODE
            break;
        case MODEND:
            ok = rewriteModend(fpout, recBuf, options);
            break;
        case CONTENT:
            ok = rewriteContent(fpout, recBuf, repCnt, options);
            break;
        case PUBLIC:
        case EXTERN:
            ok = rewriteNames(fpout, recBuf, repCnt, options);
            break;
        case LIBLOC: case LIBNAM: case LIBDIC: case LIBHDR:
            fprintf(stderr, "Error: Unsupported library type, record %d type %02XH\n", inRecCnt, getRecType(recBuf));
            return false;
        default:
            fprintf(stderr, "Error: Program error record %d type %02XH \n", inRecCnt, recBuf[0]);
            return false;
        }
    }
    return false;
}

bool parseOptions(int argc, char **argv, option_t *options) {
    for (; argc > 1 && argv[1][0] == '-'; argc--, argv++) {
        if (strcmp(argv[1], "--") == 0) {
            argc--, argv++;
            break;
        }
        for (char *s = argv[1] + 1; *s; s++) {
            switch (*s) {
            case 'l':
                options->lflag = true;
                break;
            case 'm':
                options->mflag = true;
                break;
            case 't':
                switch (*s++) {
                case 'u': options->trn = 0; break;
                case 'p': options->trn = 1; break;
                case 'f': options->trn = 2; break;
                default:
                    fprintf(stderr, "Unknown option -t%c\n", *s ? *s : ' ');
                    return false;
                }
                break;
            case 'v':
                if (!*++s) {
                    if (--argc <= 1) {
                        fprintf(stderr, "Missing value for -v option\n");
                        exit(1);
                    }
                    s = (++argv)[1];
                }
                if (sscanf(s, "%X", &options->ver) != 1 || options->ver > 0xff) {
                    fprintf(stderr, "Bad value %s for -v option\n", s);
                    return false;
                }
                s = " ";        // force pick up of next argv
                break;
            case 'p':
                if (!*++s) {
                    if (--argc <= 1) {
                        fprintf(stderr, "Missing file for -p option\n");
                        return false;
                    }
                    s = (++argv)[1];
                }
                if (!parsePatchFile(s, options))
                    return false;
                s = " ";        // force pick up of next argv
                break;
            default:
                fprintf(stderr, "Unknown option %s\n", argv[1]);
                return false;
            }
        }
    }

    if (argc != 2 && argc != 3)
        return false;
    options->infile = argv[1];
    options->outfile = argc == 2 ? argv[1] : argv[2];
    return true;
}



int main(int argc, char **argv) {
    program = argv[0];
    FILE *fpin, *fpout;
    option_t options = { -1, -1 };

    if (argc == 2 && _stricmp(argv[1], "-v") == 0) {
        showVersion(stdout, argv[1][1] == 'V');
        exit(0);
    }

    if (!parseOptions(argc, argv, &options)) {
        usage();
        exit(1);
    }

    if ((fpin = fopen(options.infile, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", options.infile);
        exit(1);
    }

    char tmpFile[10];
    int i;
    for (i = 0; i < 20; i++) {
        sprintf(tmpFile, "tmp%06d", i);
        if (fpout = fopen((const char *)tmpFile, "wbx"))
            break;
    }

    if (!fpout) {
        fprintf(stderr, "can't create tmpFile %s\n", tmpFile);
        fclose(fpin);
        exit(1);
    }
    bool ok = rewriteObj(fpin, fpout, &options);
    fclose(fpin);
    fclose(fpout);
    if (ok) {
        _unlink(options.outfile);       // delete the file if it exists
        if (rename(tmpFile, options.outfile) != 0)
            fprintf(stderr, "couldn't save file as %s, saved as %s\n", options.outfile, tmpFile);
    } else
        _unlink(tmpFile);
}
