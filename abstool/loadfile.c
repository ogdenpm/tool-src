/****************************************************************************
 *  loadfile.c is part of abstool                                         *
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

//#include "abstool.h"
#include "image.h"
#ifndef min
#define min(a,b)   ((a) <= (b) ? (a) : (b))
#endif

/* extra output file type */
enum { BAD = -2, BADCRC, VALID };

uint8_t record[MAXMEM];
int recLen;
int recAddr;

#define getRecWord(n) (record[n] + record[(n) + 1] * 256)

char validOMF51[] = "\x2\x4\x6\xe\x10\x12\x16\x18";
char validOMF85[] = "\x2\x4\x6\x8\xe\x10\x12\x16\x18\x20";
char validOMF96[] = "\x2\x4\x6\x8\xe\x10\x12\x14\x16\x18\x20";



char *validOMF    = validOMF85; // any will do as fixed after MODHDR (2)

char *formats[]   = { "AOMF51", "AOMF85", "AOMF96", "ISISBIN", "HEX", "IMAGE" };



/* read a word from a file,  return the value if not EOF else -1 */
int getword(FILE *fp) {
    int low  = getc(fp);
    int high = getc(fp);
    return high == EOF ? -1 : low + high * 256;
}

/*
   support functions to handle reading ascii hex from a file
   they return the value or -1 on error
*/

int getHex1(FILE *fp) {
    int c = getc(fp);
    return !isxdigit(c) ? -1 : isdigit(c) ? c - '0' : toupper(c) - 'A' + 10;
}
int getHex2(FILE *fp) {
    int high = getHex1(fp);
    int low  = getHex1(fp);
    return low < 0 ? -1 : high * 16 + low;
}

int getHex4(FILE *fp) {
    int high = getHex2(fp);
    int low  = getHex2(fp);
    return low < 0 ? -1 : high * 256 + low;
}

/*
   read an intel AOMF85 record from file into "record"
   returns the type if valid, BAD for invalid type and BADCRC if crc check fails
   sets "recLen"
*/
int readOMF(FILE *fp) {

    int type = getc(fp);
    if (!strchr(validOMF, type) || (recLen = getword(fp)) < 1 ||
        fread(record, 1, recLen, fp) != recLen)
        return BAD;
    uint8_t crc = type + recLen / 256 + recLen;
    for (int i = 0; i < recLen; i++)
        crc += record[i];
    recLen--; // remove CRC
    return crc == 0 ? type : BADCRC;
}

/*
    read an intel Hex record into "record"
    sets recLen and recAddr
*/
int readHex(FILE *fp) {
    int c;
    while ((c = getc(fp)) != ':')
        if (!isprint(c) && c != '\r' && c != '\n' && c != '\t' && c != '\f')
            return BAD;
    int type;
    if ((recLen = getHex2(fp)) < 0 || (recAddr = getHex4(fp)) < 0 || (type = getHex2(fp)) < 0 ||
        type > 1)
        return BAD;

    uint8_t crc = type + recLen + recAddr / 256 + recAddr;
    for (int i = 0; i < recLen + 1; i++) {
        if ((c = getHex2(fp)) < 0)
            return BAD;
        crc += record[i] = c;
    }
    return crc == 0 ? type : BADCRC;
}

/*
   read an ISIS I Bin block into "record", sets recLen and recAddr
   Note assumes chkBin has been used to check that the format is valid.
*/
int readBin(FILE *fp) {
    recLen  = getword(fp);
    recAddr = getword(fp);
    if (fread(record, 1, recLen, fp) != recLen) // chkbin checks
        error("Failed to read bin record");
    return VALID;
}

/*
   check for a plausible ISIS I Bin file
   the block chain should be valid, ending in a 0 length
   each address block should be to RAM
   and there should be no more than MAXAPPEND bytes remaining
*/
bool chkBin(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    for (;;) {
        int len  = getword(fp);
        int addr = getword(fp);
        if (len < 0 || addr < 0 || addr + len >= 0x10000) // EOF reached or addr in ROM!!
            return false;
        long here = ftell(fp);
        if (len == 0) { // possible end of BIN, check not too much following it
            return fileSize - here < MAXAPPEND;
        } else if (here + len > fileSize)
            return false;
        fseek(fp, len, SEEK_CUR); // skip actual data
    }
}

/* determine the file type, fp is assumed to have been opened using "rb" */
int fileType(FILE *fp, image_t *image) {
    if (readOMF(fp) == MODHDR) {                 /* got a valid MODHDR */
        uint8_t *p = record;                     // pick up the meta data here
        memcpy(image->name, p, min(*p, 40) + 1); // name
        p += *p + 1;
        image->mTrn = *p++;

        if (image->mTrn >= 0xfd) {
            validOMF = validOMF51;
            return AOMF51;
        } else if ((image->mTrn & 0xf0) == 0xe0) {
            validOMF = validOMF96;
            if (*p > 64) // shouldn't be > 64 chars
                *p = 64;
            memcpy(image->date, p, *p + 1); // date
            return AOMF96;
        } else if (image->mTrn < 3) {
            validOMF    = validOMF85;
            image->mVer = *p; // ver
            return AOMF85;
        } else
            error("Unknown AOMF format");
    }
    rewind(fp);
    if (readHex(fp) >= 0) /* got a valid hex record */
        return HEX;
    rewind(fp);
    if (chkBin(fp)) /* looks like an ISIS I Bin file */
        return ISISBIN;
    else
        return IMAGE; /* treat as simple image */
}

/*
  utility function to copy read data into the memory image
  updates memory bounds and usage.
*/

void addContent(image_t *image, int addr, uint8_t offset) {
    uint16_t len = recLen - offset;

    if (addr + len > MAXMEM) {
        fprintf(stderr, "Warning: data beyond 0FFFFH ignored\n");
        len = MAXMEM - addr;
    }
    if (image->high == 0) {
        image->low  = addr;
        image->high = addr + len;
    } else {
        if (addr < image->low)
            image->low = addr;
        if (addr + len > image->high)
            image->high = addr + len;
    }
    memcpy(&image->mem[addr], &record[offset], len);
    memset(&image->use[addr], SET, len);
}

/* load an AOMF85 file into memory */
void loadOMF(FILE *fp, image_t *image) {
    rewind(fp);
    int type;
    while ((type = readOMF(fp)) != MODEND) {
        if (type < MODHDR)
            error("Invalid AOMF record %02XH", type);
        else if (type == MODCONTENT) {
            if (record[0] != 0)
                error("AOMF CONTENT record has relocatable data");
            addContent(image, record[1] + record[2] * 256, 3);
        }
    }

    switch (image->source) {
    case AOMF51:
        image->mMask = record[record[0] + 4];
        break;
    case AOMF85:
        image->mMain = record[0];
        if (record[1] != 0)
            error("AOMF85 MODEND record has relocatable start");
        image->mStart = record[2] + record[3] * 256;
    case AOMF96:
        image->mMain = record[0];
        break;
    }
    if (image->source != AOMF51 && readOMF(fp) != MODEOF)
        warning("Missing AOMF MODEOF record");
}

/* load an Intel Hex file into memory*/
void loadHex(FILE *fp, image_t *image) {
    rewind(fp);
    image->mStart = -1;
    int type;
    while ((type = readHex(fp)) >= 0) {
        if (recLen == 0 && type == 1) {
            image->mStart = recAddr;
            return;
        }
        if (type == 0) /* data record */
            addContent(image, recAddr, 0);
    }
    error("Intel Hex file missing end record");
}

/*
   load an ISIS I bin file into memory
   Assumes chkBin has been used to verify it is a valid file so no checks here
*/
void loadBin(FILE *fp, image_t *image) {
    rewind(fp);
    while (readBin(fp), recLen != 0)
        addContent(image, recAddr, 0);
    image->mStart = recAddr;
}

/* load binary image into memory */
void loadImage(FILE *fp, image_t *image) {
    rewind(fp);
    recLen = (int)fread(record, 1, MAXMEM, fp);
    addContent(image, image->mLoad, 0);
}

void loadPadding(FILE *fp, image_t *image) {
    int c;
    int addr = image->high;
    while (addr < MAXMEM + MAXAPPEND && (c = getc(fp)) != EOF) {
        image->mem[addr]   = c;
        image->use[addr++] = APPEND;
    }
    image->padLen = addr - image->high;
    if (c != EOF)
        warning("excess file padding ignored");
}

bool loadFile(char *file, image_t *image) {
    FILE *fp;
    if ((fp = fopen(file, "rb")) == NULL)
        error("Cannot open input file %s", file);

    switch (image->source = fileType(fp, image)) {
    case AOMF51:
    case AOMF85:
    case AOMF96:
        loadOMF(fp, image);
        loadPadding(fp, image);
        break;
    case HEX:
        loadHex(fp, image);
        break;
    case ISISBIN:
        loadBin(fp, image);
        loadPadding(fp, image);
        break;
    default:
        loadImage(fp, image);
        break;
    }

    fclose(fp);
    image->mLoad = image->low; // update to real load
    if (image->low < image->high) {
        printf("%s: Format %s  Load %04X-%04X  Start ", file, formats[image->source - AOMF51], image->low, image->high - 1);
        if (image->mStart == -1)
            printf("IMPLICIT");
        else
            printf("%04X", image->mStart);
        if (image->padLen)
            printf("  Padding %X\n", image->padLen);
        else
            putchar('\n');
        if (image->source <= AOMF96) {
            printf("%*sNAME='%.*s' TRN=%X", (int)(strlen(file) + 2), "", image->name[0], image->name + 1, image->mTrn);
            if (image->source == AOMF85)
                printf(" VER=%02X", image->mVer);
            if (image->source == AOMF51)
                printf(" MASK=%X", image->mMask);
            else
                printf(" MAIN=%X", image->mMain);
            if (image->source == AOMF96)
                printf(" DATE='%.*s'", image->date[0], image->date + 1);
            putchar('\n');


        }
        putchar('\n');
        return true;
    }
    return false;
}
