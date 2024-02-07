/****************************************************************************
 *  savefile.c is part of abstool                                         *
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

#include "abstool.h"
#include "image.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#define LXISP 0x31
#define JMP   0xc3

uint8_t omfRecord[128];
uint8_t *outP;

uint8_t crcSum(uint8_t *blk, int len) {
    uint8_t sum = 0;
    while (len--)
        sum += *blk++;
    return sum;
}

void initOMF(int rtype) {
    omfRecord[0] = rtype;
    outP         = omfRecord + 3;
}

void writeOMF(FILE *fp) {
    int rlen     = (int)(outP - (omfRecord + 3) + 1);
    omfRecord[1] = rlen;
    omfRecord[2] = rlen / 256;
    *outP        = -crcSum(omfRecord, (int)(outP - omfRecord));
    fwrite(omfRecord, 1, rlen + 3, fp);
}

void OMFByte(int n) {
    *outP++ = n;
}

void OMFWord(int n) {
    *outP++ = n;
    *outP++ = n / 256;
}

void OMFName(uint8_t *s) {
    memcpy(outP, s, *s + 1);
    outP += *s + 1;
}

void putword(int n, FILE *fp) {
    putc(n, fp);
    putc(n / 256, fp);
}

bool fputblock(FILE *fp, image_t *image, int low, int high) {
    int len = high - low;
    uint8_t crc;
    switch (image->target) {
    case ISISBIN:
        putword(len, fp);
        putword(low, fp);
        /* FALL THROUGH */
    case IMAGE:
        fwrite(&image->mem[low], 1, len, fp);
        break;
    case AOMF51:
    case AOMF85:
    case AOMF96:
        putc(MODCONTENT, fp);
        putword(len + 4, fp);
        putc(0, fp);
        putword(low, fp);
        fwrite(&image->mem[low], 1, len, fp);
        crc = 0 - MODCONTENT - (len + 4) - (len + 4) / 256 - crcSum(&image->mem[low], len);
        putc(crc, fp);
        break;
    case HEX:
        while (len) {
            int chunk = len < HEXBYTES ? len : HEXBYTES;
            fprintf(fp, ":%02X%04X00", chunk, low);
            crc = 0 - chunk - crcSum(&image->mem[low], chunk);
            while (chunk-- > 0) {
                fprintf(fp, "%02X", image->mem[low++]);
                len--;
            }
            fprintf(fp, "%02X\r\n", crc);
        }
        break;
    }
    return ferror(fp) == 0;
}

void writeModHdr(FILE *fp, image_t *image) {
    initOMF(MODHDR);
    if (image->name[0] == 0)
        memcpy(image->name, (uint8_t *)"\x7UNKNOWN", 8);
    else
        image->name[0] = min(image->name[0], image->target == AOMF85 ? 31 : 40);
    image->date[0] = min(image->date[0], 64);
    switch (image->target) {
    case AOMF85:
        OMFName(image->name);
        if (image->mTrn > 3) {
            warning("Invalid AOMF85 TRN %02XH", image->mTrn);
            image->mTrn = -1;
        }
        OMFByte(image->mTrn >= 0 ? image->mTrn : 0);
        OMFByte(image->mVer >= 0 ? image->mVer : 0);
        break;
    case AOMF51:
        OMFName(image->name);
        if (image->mTrn >= 0 && image->mTrn < 0xfd) {
            warning("Invalid AOMF85 TRN %02XH", image->mTrn);
            image->mTrn = -1;
        }
        OMFByte(image->mTrn >= 0 ? image->mTrn : 0xff);
        OMFByte(0);
        break;
    case AOMF96:
        OMFName(image->name);
        if (image->mTrn > 3) {
            warning("Invalid AOMF85 TRN %02XH", image->mTrn);
            image->mTrn = 0;
        }
        OMFByte(image->mTrn >= 0 ? image->mTrn : 0);
        OMFName(image->date);
        break;
    }
    writeOMF(fp);
}


void writeModEnd(FILE *fp, image_t *image) {
    initOMF(MODEND);
    switch (image->target) {
    case AOMF85:
        OMFByte(image->mMain >= 0 ? image->mMain & 1 : 1);
        OMFByte(0);
        OMFWord((image->mMain & 1) ? image->mStart: 0);
        break;
    case AOMF51:
        OMFName(image->name);
        OMFWord(0);
        OMFByte(image->mMask >= 0 ? image->mMask & 0xf : 0);
        OMFByte(0);
        break;
    case AOMF96:
        OMFByte(image->mMain >= 0 ? image->mMain & 1 : 1);
        OMFByte(0);
        break;
    }
    writeOMF(fp);
}

int saveFile(char *file, image_t *image) {
    FILE *fp;
    bool isOk = true;
    int addr;

    if (image->high == image->low)
        error("Nothing to save");

    if ((fp = fopen(file, "wb")) == NULL)
        error("can't create output file %s\n", file);

    if (image->target == IMAGE) {
        isOk = fputblock(fp, image, image->low, image->high);
    } else {
        if (image->target <= AOMF96)
            writeModHdr(fp, image);

        for (addr = image->low; addr < image->high && isOk;) {
            while (addr < image->high && image->use[addr] == NOTSET)
                addr++;
            if (addr == image->high || image->use[addr] != SET)
                break;
            image->low = addr;
            while (addr < image->high && image->use[addr] == SET)
                addr++;
            isOk = fputblock(fp, image, image->low, addr);
        }

        if (isOk) {
            if (image->mStart < 0)
                image->mStart = 0;
            if (image->target <= AOMF96)
                writeModEnd(fp, image);
            else if (image->target == ISISBIN) {
                putword(0, fp);
                putword(image->mStart, fp);
            } else // HEX
                fprintf(fp, ":00%04X01%02X\r\n", image->mStart,
                        (0 - (image->mStart + image->mStart / 256 + 1)) & 0xff);
        }
        isOk = ferror(fp) == 0;
    }

    if (isOk && image->padLen) // apply any padding
        isOk = fwrite(&image->mem[image->high], 1, image->padLen, fp) == image->padLen;

    fclose(fp);
    if (!isOk)
        fprintf(stderr, "write failure on %s\n", file);
    return isOk ? EXIT_SUCCESS : EXIT_FAILURE;
}
