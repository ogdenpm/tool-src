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
#include "showversion.h"

#ifdef _WIN32
#define DIRSEP "\\/"
#else
#define DIRSEP "/"
#endif

/*
 * abstool is and extension to the previous hex2bin and obj2bin tools
 * It converts between 4 types of absolute file formats, optionally applying patches
 * The formats are Intel AOMF85, Intel Hex, Intel ISIS I Bin and binary images
 * Note any input file symbols and debug information is discarded.
 *
 * See patch.c or the documentation for the patch file format
 */

char *invokedBy;

image_t inFile;

_Noreturn void usage(char *fmt, ...) {

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "Usage error: ");
        vfprintf(stderr, fmt, args);
        putc('\n', stderr);
        va_end(args);
    }
    fprintf(
        fmt ? stderr : stdout,
        "Usage: %s [-v|-V|-h] |  [-l addr] [-a|-a51|-a85|-a96|-h|-i] infile [[patchfile] outfile]\n"
        "Where -v/-V   provide version information\n"
        "      -h      shows this help, if it is the only option\n"
        "      -l addr override the load address for binary images, default is 100H (CP/M)\n"
        "      -a51    produce AOMF51 file, note no symbols or debug info\n"
        "      -a|-a85 produce AOMF85 file, note no symbols or debug info\n"
        "      -a96    produce AOMF96 file, note no symbols or debug info\n"
        "      -h      produce Intel Hex file, note no symbols\n"
        "      -i      produce Intel ISIS I bin file\n"
        "File format can be AOMF51, AOMF85, AOMF96, Intel Hex, Intel ISIS I Bin or binary image\n"
        "The last format specified is used, default is binary image\n"
        "If outfile is omitted, only a summary of the infile is produced\n",
        invokedBy);
    exit(1);
}

_Noreturn void error(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, args);
    putc('\n', stderr);
    va_end(args);
    exit(1);
}

void warning(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, fmt, args);
    putc('\n', stderr);
    va_end(args);
}

char *getInvokeName(char *path) {
    char *s;
#ifdef _WIN32
    if (path[0] && path[1] == ':') // skip leading device
        path += 2;
#endif
    while ((s = strpbrk(path, DIRSEP))) // skip all directory components
        path = s + 1;
    s = strrchr(path, '.');
    if (s && stricmp(s, ".exe") == 0)
        *s = '\0';
    return path;
}

void resetMeta(image_t *image) {
    for (int i = 0; i < sizeof(image->meta) / sizeof(image->meta[0]); i++)
        image->meta[i] = -1;
}

int main(int argc, char **argv) {
    resetMeta(&inFile);
    inFile.target = IMAGE;
    inFile.mLoad  = 0x100;

    invokedBy = getInvokeName(argv[0]);

    CHK_SHOW_VERSION(argc, argv);
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
        usage(NULL);

    while (argc >= 2 && argv[1][0] == '-') {
        switch (argv[1][1]) {
        case 'a':
            if (strcmp(argv[1], "-a51") == 0)
                inFile.target = AOMF51;
            else if (strcmp(argv[1], "-a96") == 0)
                inFile.target = AOMF96;
            else if (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "-a85") == 0)
                inFile.target = AOMF85;
            else
                usage("Unknown option %s\n", argv[1]);
            break;
        case 'h':
            inFile.target = HEX;
            break;
        case 'i':
            inFile.target = ISISBIN;
            break;
        case 'l':
            if (argc < 3 || (inFile.mLoad = parseHex(argv[2], NULL)) < 0)
                usage("-l option missing address");
            argc--, argv++;
            break;
        default:
            usage("Unknown option %s\n", argv[1]);
            break;
        }
        argc--;
        argv++;
    }

    if (argc < 2 || argc > 4)
        usage("Incorrect number of files");

    if (loadFile(argv[1], &inFile)) {
        if (argc == 2)
            return EXIT_SUCCESS;
        if (argc == 3 && inFile.source == inFile.target)
            error("Nothing to do. Input and output files same format with no patching");
        if (inFile.source <= AOMF96 && inFile.target <= AOMF96 && inFile.source != inFile.target)
            warning("Did you mean to convert between AOMFxx file formats");
        if (argc == 4)
            patchfile(argv[2], &inFile);
        return saveFile(argv[argc - 1], &inFile);
    } else {
        fprintf(stderr, "Nothing loaded\n");
        return EXIT_FAILURE;
    }
}
