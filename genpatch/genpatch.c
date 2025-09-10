/****************************************************************************
 *  genpatch.c is part of genpatch                                         *
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

#include "utility.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "genpatch.h"

#ifdef _WIN32
#define DIRSEP "\\/"
#else
#define DIRSEP "/"
#endif

char *invokedBy;
image_t inFile, targetFile;
char *tokens[] = { "AOMF51", "AOMF51K", "AOMF85", "AOMF96", "ISISBIN", "HEX",
                   "IMAGE",  "TARGET",  "SOURCE", "NAME",   "DATE",    "START",
                   "LOAD",   "TRN",     "VER",    "MAIN",   "MASK" };

char const help[] =
    "Usage: %s [-l addr]  [-s | infile] targetfile [patchfile]\n"
    "Where\n"
    " -l addr  set explicit load address for binary image files. Default 100H (CP/M)\n"
    " -s       generate simple patch file from targetfile only\n"
    "Supported file formats are AOMF51, AOMF85, AOMF96 Intel Hex, Intel ISIS I binary and "
    "binary image\n"
    "If infile is empty or a null device, targetfile is converted to a patch file\n"
    "If patchfile is omitted then the patch data is output to stdout";



void resetMeta(image_t *image) {
    for (int i = 0; i < (int)sizeof(image->meta) / (int)sizeof(image->meta[0]); i++)
        image->meta[i] = -1;
}

int typeRunLen(image_t *image, int addr, int top) {
    int i;
    for (i = addr + 1; i < top && image->use[addr] == image->use[i]; i++)
        ;
    return i - addr;
}

int valRunLen(image_t *image, int addr, int top) {
    int i;
    for (i = addr + 1; i < top && image->mem[addr] == image->mem[i]; i++)
        ;
    return i - addr;
}

void genPatch(FILE *fp, image_t *src, image_t *dst, int useType, char *heading) {
    bool haveSection = false;
    unsigned runlen;
    unsigned col = 0;
    int sameLen;
    image_t *image;
    int bottom;
    int top;

    if (useType == APPEND) {
        image  = dst;       // for append the data in dst
        bottom = dst->high; // set the location limits
        top    = dst->high + dst->padLen;
    } else {
        image  = src;      // for others the data is in the src image
        bottom = src->low; // set the patch area to look at
        top    = src->high;
    }

    for (int addr = bottom; addr < top; addr += runlen) {
        runlen = 1;                        // assume single byte
        if (image->use[addr] == useType) { // look for the next occurrence
            if (!haveSection) {            // if any data print heading once
                fprintf(fp, "%s\n", heading);
                haveSection = true;
            }
            runlen = typeRunLen(image, addr, top); // how many we have
            switch (useType) {
            case CHANGE: // show the old values as comments
            case UNSET:
                if (useType == UNSET) { // delete just show as single block
                    fprintf(fp, "%04X   -", addr);
                    if (runlen > 1)
                        fprintf(fp, " x %02X\n ", runlen);
                    else
                        putc('\n', fp);
                }

                for (unsigned i = 0; i < runlen; i += 16) {
                    if (useType == CHANGE) { // change show block of new values
                        fprintf(fp, "%04X", addr + i);
                        for (unsigned j = 0; j < 16 && i + j < runlen; j++)
                            fprintf(fp, " %02X", dst->mem[addr + i + j]);
                        putc('\n', fp);
                    }
                    fprintf(fp, ";>>>");
                    for (unsigned j = 0; j < 16 && i + j < runlen; j++)
                        fprintf(fp, " %02X", src->mem[addr + i + j]);
                    fputs(" <<<\n", fp);
                }
                break;
            case SET:
            case APPEND:
                col = 0;
                for (unsigned i = 0; i < runlen; i += sameLen) {
                    if (col > MAXCOL) {
                        putc('\n', fp);
                        col = 0;
                    }
                    if (col == 0 && useType == SET)
                        col += fprintf(fp, "%04X", addr + i);

                    if ((sameLen = valRunLen(dst, addr + i, addr + runlen)) >= MINRUN)
                        col += fprintf(fp, " %02X x %02X", dst->mem[addr + i], sameLen);
                    else {
                        col += fprintf(fp, " %02X", dst->mem[addr + i]);
                        sameLen = 1;
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

/*
    compare the src and dst images
    and modify the src image to reflect any patch data
    by setting the use value to one of the following for the data at the address
    NOTSET  - src and dst are same
    SET     - use the dst image data at this address as an initialisation value
    CHANGE  - use dst image data as the new value, but show old value from src
    UNSET   - mark the value to be assumed uninitialised, show what data is being removed

    For dst binary images a guess is made at what should be padding vs. real data
*/
void markup(image_t *src, image_t *dst) {
    if (src->low == src->high) {
        src->low  = dst->low;
        src->high = dst->high;
    }
    if (src->low >= dst->high || src->high < dst->low)
        fatal("Input and target files don't have any memory in common");

    int addr = dst->low;
    for (int saddr = src->low; saddr < addr; saddr++) // remove src data below target
        src->use[saddr] = UNSET;
    for (int saddr = src->low; addr < saddr; addr++) { // mark as init real target data below src
        if (dst->use[addr] == SET)
            src->use[addr] = SET;
    }
    if (dst->low < src->low) // lower limit of patch area
        src->low = dst->low;
    // binary images don't have an explicit boundary between data and padding
    // so see if we can guess one
    // if not all the padding will be treated as patch data
    if (dst->source == IMAGE) {
        dst->padLen = 0;
        while (dst->high > src->high) { // mark any dst data after src end as being append
            dst->use[--dst->high] = APPEND;
            dst->padLen++; // updates the pad count
        }
    }
    for (; addr < dst->high; addr++) { // process the common address space
        if (addr >= src->high)
            src->use[addr] = SET; // add if not in source and before append
        else if (dst->use[addr] == SET) {
            src->use[addr] = src->use[addr] == NOTSET
                                 ? SET
                                 : (src->mem[addr] != dst->mem[addr] ? CHANGE : NOTSET);
        } else if (src->use[addr] == SET)
            src->use[addr] = UNSET;
    }
    for (; addr < src->high; addr++) // unset src data above dst end
        if (src->use[addr] == SET)
            src->use[addr] = UNSET;
    if (src->high < dst->high) // upper bound of patch area
        src->high = dst->high;
}

void genPatchFile(char *file, image_t *src, image_t *dst) {
    FILE *fp;

    if (file == NULL)
        fp = stdout;
    else if ((fp = fopen(file, "wt")) == NULL) {
        fprintf(stderr, "can't create patch file %s\n", file);
        return;
    }
    if (src->low == src->high) {
        src->low  = dst->low;
        src->high = dst->high;
        markup(src, dst);
        genPatch(fp, src, dst, SET, "; SIMPLE PATCH FILE");
    } else {
        // emit the meta data
        fprintf(fp, "TARGET=%s SOURCE=%s", tokens[dst->source - AOMF51],
                tokens[src->source - AOMF51]);
        if (dst->mStart >= 0 || src->mStart >= 0)
            fprintf(fp, " START=%04X", dst->mStart >= 0 ? dst->mStart : src->mStart);
        fprintf(fp, " LOAD=%04X\n", dst->mLoad);
        if (dst->source <= AOMF96) {
            fprintf(fp, "NAME='%.*s' TRN=%X", dst->name[0], dst->name + 1, dst->mTrn);
            if (dst->source == AOMF85)
                fprintf(fp, " VER=%02X", dst->mVer);
            if (dst->source == AOMF51)
                fprintf(fp, " MASK=%X", dst->mMask);
            else
                fprintf(fp, " MAIN=%X", dst->mMain);
            if (dst->source == AOMF96)
                fprintf(fp, "\nDATE='%.*s'", dst->date[0], dst->date + 1);
            putc('\n', fp);
        }
        markup(src, dst); // tag what needs to be done and put out patch data in a logical order
        genPatch(fp, src, dst, CHANGE, "; PATCHES");
        genPatch(fp, src, dst, UNSET, "; DELETIONS");
        genPatch(fp, src, dst, SET, "; UNIITIALISED - RANDOM DATA");
        genPatch(fp, src, dst, APPEND, "APPEND");
    }
    if (fp != stdout)
        fclose(fp);
}

int main(int argc, char **argv) {

    bool needIn = true;

    resetMeta(&inFile);
    resetMeta(&targetFile);
    inFile.source = targetFile.source = IMAGE;
    inFile.mLoad                      = 0x100;

    while (getOpt(argc, argv, "l:s") != EOF) {
        switch (optOpt) {
        case 'l':
            char const *s = optArg;
            if (isxdigit(*s)) {
                inFile.mLoad = 0;
                while (isxdigit(*s)) {
                    inFile.mLoad =
                        inFile.mLoad * 16 + (isdigit(*s) ? *s - '0' : tolower(*s) - 'a' + 10);
                    s++;
                }
            } else
                usage("-l option missing address");
            break;
        case 's':
            needIn = false;
            break;
        default:
            if (optOpt & MISSINGARG)
                usage("Missing argument for option -%c", optOpt & 0xff);
            usage("Unknown option -%c", optOpt & 0xff);
        }
    }
    targetFile.mLoad = inFile.mLoad;

    if (needIn) {
        if (optInd == argc)
            usage("Missing input file");
        if (!loadFile(argv[optInd], &inFile))
            fatal("input file %s failed to load any data", argv[optInd]);
        optInd++;
    } else
        inFile.low  = inFile.high = 0; // mark as empty file
    if (argc - optInd < 1 || argc - optInd > 2)
        usage("Incorrect number of files");
    if (!loadFile(argv[optInd], &targetFile) || targetFile.low == targetFile.high)
        fatal("Target file %s failed to load any data", argv[optInd]);
    genPatchFile(++optInd != argc ? argv[optInd] : NULL, &inFile, &targetFile);
}