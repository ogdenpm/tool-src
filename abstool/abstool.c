/****************************************************************************
 *  abstool.c is part of abstool                                         *
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
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// local includes after std includes
#include "abstool.h"
#include "image.h"
#include "utility.h"


/*
 * abstool is and extension to the previous hex2bin and obj2bin tools
 * It converts between 4 types of absolute file formats, optionally applying patches
 * The formats are Intel AOMF85, Intel Hex, Intel ISIS I Bin and binary images
 * Note any input file symbols and debug information is discarded.
 *
 * See patch.c or the documentation for the patch file format
 */

image_t inFile;

char const help[] =
        "Usage: %s [-v|-V|-h] | [-l addr] [-a|-a51|-a85|-a96|-h|-i] -p patchfile -o outfile infile+\n"
        "Where:\n"
        "-l addr  override the load address for binary images, default is 100H (CP/M)\n"
        "         subsequent binary images are concatenated\n"
        "-a51     produce AOMF51 file, note no symbols or debug info\n"
        "-a|-a85  produce AOMF85 file, note no symbols or debug info\n"
        "-a96     produce AOMF96 file, note no symbols or debug info\n"
        "-h       produce Intel Hex file, note no symbols\n"
        "-i       produce Intel ISIS I bin file\n"
        "-p file  specify patch file\n"
        "-o file  specify output file, if omitted only a summary of infile is produced\n"
        "File format can be AOMF51, AOMF85, AOMF96, Intel Hex, Intel ISIS I Bin or binary image\n"
        "The last format specified is used, default is binary image\n";

void resetMeta(image_t *image) {
    for (int i = 0; i < (int)sizeof(image->meta) / (int)sizeof(image->meta[0]); i++)
        image->meta[i] = -1;
}

int main(int argc, char **argv) {
    resetMeta(&inFile);
    inFile.target = IMAGE;
    inFile.mLoad  = 0x100;
    char const *patch   = NULL;
    char const *outfile = NULL;

    while (getOpt(argc, argv, "l:a*hip:o:") != EOF) {
        switch (optOpt) {
        case 'a':
            if (strcmp(optArg, "51") == 0)
                inFile.target = AOMF51;
            else if (strcmp(optArg, "96") == 0)
                inFile.target = AOMF96;
            else if (!*optArg || strcmp(optArg, "85") == 0)
                inFile.target = AOMF85;
            else
                usage("Unknown option AOMF format -a%s", optArg);
            break;
        case 'h':
            inFile.target = HEX;
            break;
        case 'i':
            inFile.target = ISISBIN;
            break;
        case 'l':
            if ((inFile.mLoad = parseHex(optArg, NULL)) < 0)
                usage("Bad address for -l option");
            break;
        case 'p':
            patch = optArg;
            break;
        case 'o':
            outfile = optArg;
            break;
        default:
            if (optOpt & MISSINGARG)
                usage("Missing argument for option -%c", optOpt & 0xff);
            else
                usage("Unknown option %c", optOpt);
            break;
        }
    }
    int inCount = 0;
    while (optInd < argc) {
        if (!loadFile(argv[optInd], &inFile))
            fatal("Problem loading input file %s", argv[optInd]);
        optInd++;
        inCount++;
    }
    if (inCount == 0)
        usage("No input files specified");
    if (outfile) {
        if (!patch && inCount == 1 && inFile.source == inFile.target)
            fatal("Nothing to do. Input and output files same format with no patching");
        if (inFile.source <= AOMF96 && inFile.target <= AOMF96 && inFile.source != inFile.target)
            warn("Did you mean to convert between AOMFxx file formats");
        if (patch)
            patchfile(patch, &inFile);
    
        return saveFile(outfile, &inFile);
    }
    return EXIT_SUCCESS;
}
