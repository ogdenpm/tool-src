/****************************************************************************
 *  stdOption.c is part of utility                                         *
 *  Copyright (C) 2025 Mark Ogden <mark.pm.ogden@btinternet.com>            *
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
#include "verInfo.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *programName = "???";

/// <summary>
/// Process standard options, currently -v, -V and -h
/// also captures argv[0] as programName
/// Options must be the only ones present
/// </summary>
/// <param name="s">The s.</param>
void chkStdOptions(int argc, char **argv) {
    programName = argv[0];
#ifdef _WIN32
    char *s = strrchr(programName, '.');
    if (s && stricmp(s, ".exe") == 0)
        *s = '\0';
#endif
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-V") == 0) {
            showVersion(stdout, argv[1][1] == 'V');
            exit(0);
        } else if (strcmp(argv[1], "-h") == 0)
            usage(NULL);
    }
}
