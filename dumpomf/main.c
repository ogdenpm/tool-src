/****************************************************************************
 *  main.c is part of dumpomf                                               *
 *  Copyright (C) 2022 Mark Ogden <mark.pm.ogden@btinternet.com>            *
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

#include "omf.h"
#include <ctype.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "showVersion.h"

typedef unsigned char byte;
typedef unsigned short address;
FILE *logfp = NULL;
char *invoke;
bool lastTypeWasEnd;
bool malformed;

int detectOMF();
enum flavour_e omfFlavour = ANY;
bool rawMode              = false;


FILE *src;
FILE *dst;

_Noreturn void usage(char const *s) {
    if (s && *s)
        fputs(s, stderr);
    fprintf(stderr, "usage: %s -v | -V | [-r] objfile [outputfile]\n", invoke);
    exit(1);
}

void dumprec(FILE *fpout, bool isGood);

extern decodeSpec_t omfUknDecode[];
extern decodeSpec_t omf85Decode[];
extern decodeSpec_t omf51Decode[];
extern decodeSpec_t omf96Decode[];
extern decodeSpec_t omf86Decode[];
void init85();
void init51();
void init96();
void init86();
void initUkn();

omfDispatch_t dispatchTable[] = { { initUkn, 0, 0, omfUknDecode },
                                  { init85, 2, 0x2e, omf85Decode },
                                  { init51, 2, 0x2c, omf51Decode },
                                  { init51, 2, 0x72, omf51Decode },
                                  { init96, 2, 0x2e, omf96Decode },
                                  { init86, 0x6e, 0xce, omf86Decode } };

decodeSpec_t omfUknDecode[] = {
    { "UNKNOWN", invalidRecord, NULL }
};


void initUkn() {
}

void displayFile(int spec) {
    omfDispatch_t *dispatch = &dispatchTable[spec];
    int status;

    dispatch->init();

    while ((status = getrec()) >= 0) {
        startCol(0);
        if (status == BadCRC)
            Log("-- Warning CRC error --");

        if (omfFlavour == ANY) { // see if we can resolve flavour of OMF86
            if (recType < 0x80 || recType == 0x84 || recType == 0x86)
                omfFlavour = INTEL;
            else if (recType > 0xaa || is32bit)
                omfFlavour = MS;
        }

        int idx = dispatch->low <= recType && recType <= dispatch->high
                      ? (recType - dispatch->low) / 2 + 1
                      : 0;
        if (!dispatch->decodeTable[idx].name || !isValidRec(spec))
            idx = 0;

        add("%s(%s): ", dispatch->decodeTable[idx].name, hexStr(recType));
        fixCol();
        if (rawMode)
            invalidRecord(recType);
        else
            dispatch->decodeTable[idx].handler(recType);
        if (malformed || !atEndRec()) {
            if (malformed)
                undoCol();
            else
                markRecPos();
            Log("-- Malformed record --");
            hexDump(revertRecPos(), false);
        }
        displayLine(); // flush line
        if (recType == 0xe &&
            (spec == OMF85 || spec == OMF96)) // for OMF85/OMF96 type 0xe is EOF
            break;
    }
    if (status == Junk) {
        startCol(0);
        Log("Unexpected data at end of file\n");
    }
}


int main(int argc, char **argv) {
    int spec;

    invoke = argv[0];
    CHK_SHOW_VERSION(argc, argv);

    if (argc > 1 && strcmp(argv[1], "-r") == 0) {
        argc--, argv++;
        rawMode = true;
    }

    if (argc < 2 || (src = fopen(argv[1], "rb")) == NULL)
        usage("can't open input file\n");

    if (argc != 3 || (dst = fopen(argv[2], "w")) == NULL)
        dst = stdout;

    if ((spec = detectOMF()) == OMFUKN)
        fprintf(stderr, "%s cannot determine OMF spec\n", argv[1]);
    displayFile(spec);
    fclose(dst);
    fclose(src);
    return 0;
}
