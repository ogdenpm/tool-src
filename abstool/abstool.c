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

#include "showversion.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abstool.h"

#ifdef _WIN32
#define DIRSEP "\\/"
#else
#define DIRSEP "/"
#endif


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


_Noreturn void usage(char *fmt, ...) {

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "Usage error: ");
        vfprintf(stderr, fmt, args);
        putc('\n', stderr);
        va_end(args);
    }
    fprintf(fmt ? stderr : stdout,
            "Usage: %s [-v|-V|-h] |  [-b addr] [-a|-h|-i|-j] infile outfile [patchfile]\n"
            "Where -v/-V   provide version information\n"
            "      -h      shows this help, if it is the only option\n"
            "      -b addr override base address for binary image infile, default is 100H (CP/M)\n"
            "      -a      produce AOMF85 file, note no symbols or debug info\n"
            "      -h      produce an Intel Hex file, note no symbols\n"
            "      -i      produces Intel ISIS I bin file\n"
            "      -j      produce binary image file, modifying the jmp start if possible\n"
            "Supported file formats are AOMF85, Intel Hex, Intel ISIS I binary and "
            "binary image\n"
            "The default outfile format is binary image\n",
            invokedBy);
    exit(1);
}

_Noreturn void error(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(1);
}

void warning(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, fmt, args);
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


int main(int argc, char **argv) {
    int outFormat   = NOTSET;
    bool writeEntry = false;

    invokedBy       = getInvokeName(argv[0]);

    CHK_SHOW_VERSION(argc, argv);
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
        usage(NULL);

    while (argc >= 2 && argv[1][0] == '-') {
        switch (tolower(argv[1][1])) {
        case 'a':
            outFormat = outFormat == NOTSET || outFormat == OMF85 ? OMF85 : MULTISET;
            break;
        case 'h':
            outFormat = outFormat == NOTSET || outFormat == HEX ? HEX : MULTISET;
            break;
        case 'i':
            outFormat = outFormat == NOTSET || outFormat == BIN ? BIN : MULTISET;
            break;
        case 'j':
            outFormat  = outFormat == NOTSET || outFormat == IMAGE ? IMAGE : MULTISET;
            writeEntry = true;
            break;
        case 'b':
            if (argc > 2 || (base = parseHex(argv[2], NULL)) < 0)
                usage("-b option missing address");
            argc--, argv++;
        default:
            usage("Unknown option %s\n", argv[1]);
            break;
        }
        argc--;
        argv++;
    }
    if (outFormat == NOTSET)
        outFormat = IMAGE;
    else if (outFormat == MULTISET)
        usage("more than one output format specified");


    if (argc < 3 || argc > 4)
        usage("Incorrect number of files");

    if (loadFile(argv[1])) {
        if (argc == 3) {
            if (writeEntry)
                insertJmpEntry();
        } else {
            if (writeEntry)
                warning("-j option to modify the jmp start ignored with patch file");
            patchfile(argv[3]);
        }

        return saveFile(argv[2], outFormat);
    } else {
        fprintf(stderr, "Nothing loaded\n");
        return EXIT_FAILURE;
    }
}

