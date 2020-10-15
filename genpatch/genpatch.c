/****************************************************************************
 *                                                                          *
 *  genpatch: Generate patch file                                           *
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define MAXMEM 0x11000
#define MAXCOL 60
#define MINRUN 5

uint8_t objMem[MAXMEM];
uint8_t binMem[MAXMEM];
uint8_t use[MAXMEM];

unsigned low = 0x10000;
unsigned high = 0;

unsigned start = 0x10000;

enum { UNUSED = 0, OBJ, SKIP, INIT, CHANGE, DELETE, APPEND };

void showVersion(FILE *fp, bool full);

int readRec(FILE *fp);
bool read4(FILE *fp, int len);
bool read6(FILE *fp, int len);


unsigned getword(FILE *fp) {
    int c = getc(fp);
    return c + getc(fp) * 256;
}


bool loadBin(char *file, bool isIntel, bool defaultZero) {
    FILE *fp;
    unsigned addr;
    unsigned len;
    bool result = true;
    int c;

    if ((fp = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "Can't open bin file %s\n", file);
        return false;
    }
    if (!isIntel) {
        if (low < 0x400 && low != 0x100)
            fprintf(stderr, "load address != 0x100, possibly a problem for CP/M .COM file\n");
        addr = low;
        while ((c = getc(fp)) != EOF && addr < MAXMEM) {
            if ((use[addr] == UNUSED && (c != 0 || !defaultZero)))
                use[addr] = addr < high ? INIT : APPEND;
            else if (use[addr] == OBJ && objMem[addr] != c)
                use[addr] = addr < high ? CHANGE : APPEND;
            else
                use[addr] = addr < high ? SKIP : APPEND;
            binMem[addr++] = c;
        }
        if (addr >= MAXMEM)
            fprintf(stderr, "Too many patch bytes\n");
        if (addr > high)
            high = addr;
        else
            while (addr < high)
                use[addr++] = DELETE;
                
    } else {
        c = 0;
        while (c != EOF && (len = getword(fp)) != 0 && len < 0x10000) {
            addr = getword(fp);
            if (addr + len > high)
                high = addr;
            if (addr < low)
                low = addr;
            while (len-- > 0 && (c = getc(fp)) != EOF && addr < MAXMEM) {
                if (use[addr] = UNUSED && (c != 0 || !defaultZero))
                    use[addr] = addr < high ? INIT : APPEND;
                else if (use[addr] == OBJ && objMem[addr] != c)
                    use[addr] = addr < high ? CHANGE : APPEND;
                else
                    use[addr] = addr < high ? SKIP : APPEND;
                    binMem[addr++] = c;
            }
        }
        addr = getword(fp);
        if (addr >= 0x10000 || addr != start)
            fprintf(stderr, "Warning start address mismatch OBJ: %04X, BIN: %04X\n", start, addr);
        while ((c = getc(fp)) != EOF && high < MAXMEM) {
            objMem[high] = c;
            use[high++] = APPEND;
        }
        if (c != EOF)
            fprintf(stderr, "Too many patch bytes\n");
    }
    fclose(fp);
    return true;
}




bool loadObj(char *file) {

    FILE *fp;
    int result;

    if ((fp = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "can't open obj file %s\n", file);
        return false;
    }

    while ((result = readRec(fp)) > 0)
        ;
    fclose(fp);
    if (result == EOF) {
        fprintf(stderr, "Corrupt obj file %s\n", file);
        return false;
    }
    return true;
}

int readRec(FILE *fp) {
    int type;
    int len;
    bool ok = true;

    type = getc(fp);
    if (type == EOF)
        return EOF;
    if (type == 0xe)
        return 0;
    len = getword(fp);

    if (type == 6)
        ok = read6(fp, len - 1);
    else if (type == 4)
        ok = read4(fp, len - 1);
    else
        fseek(fp, len - 1, SEEK_CUR);

    return getc(fp) != EOF && ok ? 1 : EOF;
}



bool read6(FILE *fp, int len) {
    unsigned short addr;
    int c;

    if (len < 3) {
        fprintf(stderr, ">>>corrupt type 6 field\n");
        return false;
    }

    (void)getc(fp);	// Seg
    addr = getword(fp);
    len -= 3;
    if (addr < low)
        low = addr;
    c = 0;
    while (len-- > 0 && (c = getc(fp)) != EOF) {
        objMem[addr] = c;
        use[addr++] = OBJ;
    }
    if (addr > high)
        high = addr;
    return c != EOF;

}

bool read4(FILE *fp, int len) {
    unsigned modType;
    unsigned segId;
    unsigned offset;

    modType = getc(fp);
    segId = getc(fp);
    offset = getc(fp);
    offset += getc(fp) * 256;
    len -= 4;
    if (len > 0)
        fseek(fp, len, SEEK_CUR);
    if (segId != 0) {
        fprintf(stderr, "ERROR: Object file has relocatable offset\n");
        return false;
    }
    start = offset;
    return true;
}

int typeRunLen(unsigned addr) {
    unsigned i;
    for (i = addr + 1; i < high && use[addr] == use[i]; i++)
        ;
    return i -addr;
}

int valRunLen(unsigned addr) {
    unsigned i;
    for (i = addr + 1; i < high && use[addr] == use[i] && binMem[addr] == binMem[i] ; i++)
        ;
    return i -addr;

}


void genPatch(FILE *fp, int usage, char *heading,  bool showString) {
    bool haveSection = false;
    unsigned runlen;
    unsigned samelen;
    unsigned col = 0;
    unsigned prevAddr = ~0;

    for (unsigned addr = low; addr < high; addr += runlen) {
        runlen = 1;
        if (use[addr] == usage) {
            if (!haveSection) {
                fprintf(fp, "%s\n", heading);
                haveSection = true;
            }
            runlen = typeRunLen(addr);      // how many we have
            switch (usage) {
            case CHANGE:                    // show the old values as comments
            case DELETE:
                if (usage == DELETE)        // delete just show as single block
                    fprintf(fp, "%04X   - x %02X\n", addr, runlen);

                for (unsigned i = 0; i < runlen; i += 16) {
                    if (usage == CHANGE) {  // change show block of new values
                        fprintf(fp, "%04X", addr + i);
                        for (unsigned j = 0; j < 16 && i + j < runlen; j++)
                            fprintf(fp, " %02X", binMem[addr + i + j]);
                        putc('\n', fp);
                    }
                    fprintf(fp, ";>>>");
                    for (unsigned j = 0; j < 16 && i + j < runlen; j++)
                        fprintf(fp, " %02X", objMem[addr + i + j]);
                    fputs(" <<<\n", fp);
                }
                break;
            case INIT:
            case APPEND:
                col = 0;
                for (unsigned i = 0; i < runlen; i += samelen) {
                    if (col > MAXCOL) {
                        putc('\n', fp);
                        col = 0;
                    }
                    if (col == 0 && usage == INIT)
                        col += fprintf(fp, "%04X", addr + i);

                    samelen = valRunLen(addr + i);
                    if (samelen >= MINRUN)
                        col += fprintf(fp, " %02X x %02X", binMem[addr + i], samelen);
                    else {
                        col += fprintf(fp, " %02X", binMem[addr + i]);
                        samelen = 1;
                    }
                }
                if (col)
                    putc('\n', fp);
            }
        }

    }
    if (haveSection)
        putc('\n', fp);

}

void genPatchFile(char *file, bool showString) {
    FILE *fp;


    if ((fp = fopen(file, "wt")) == NULL) {
        fprintf(stderr, "can't create patch file %s\n", file);
        return;
    }
    genPatch(fp, CHANGE, "; PATCHES", showString);
    genPatch(fp, DELETE, "; DELETIONS", showString);
    genPatch(fp, INIT, "; UNINITALISED RANDOM DATA", showString);
    genPatch(fp, APPEND, "APPEND", showString);

    fclose(fp);
}


int main(int argc, char **argv) {
    char *invoke = argv[0];
    bool intelBin = false;
    bool showString = false;
    bool defaultZero = true;

    if (argc == 2 && stricmp(argv[1], "-v") == 0) {
        showVersion(stdout, argv[1][1] == 'V');
        exit(0);
    }

    while (argc > 1 && argv[1][0] == '-') {
        if (strcmp(argv[1], "-i") == 0)
            intelBin = true;
        else if (strcmp(argv[1], "-s]") == 0)
            showString = true;
        else if (strcmp(argv[1], "-z") == 0)
            defaultZero = false;
        else
            fprintf(stderr, "Unknown option %s\n", argv[1]);
        argc--;
        argv++;
    }
    if (argc != 4) {
        fprintf(stderr, "usage: %s [-i] [-s] [-z] objFile binfile patchfile\n", invoke);
        exit(1);
    }
    if (loadObj(argv[1]) && loadBin(argv[2], intelBin, defaultZero))
        genPatchFile(argv[3], showString);
}