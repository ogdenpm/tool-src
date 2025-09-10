/****************************************************************************
 *  usage.c is part of utility                                         *
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
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// <summary>
/// Display the application help
/// if the optional fmt, ... is present the formatted
/// string is written to stderr, preceeded by "Error: ", followed by the help and the program exits
/// with 1
/// if fmt is NULL then help is printed to stdout and program exits with 0
/// </summary>
/// <param name="fmt">optional format</param>
/// <param name="">and arguments</param>
_Noreturn void usage(char *fmt, ...) {
    FILE *fp = fmt ? stderr : stdout;
    showVersion(fp, false);
    putc('\n', fp);

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        fputs("Error: ", fp);
        vfprintf(fp, fmt, args);
        fputs("\n\n", fp);
        va_end(args);
    }

    for (char const *s = help; *s; s++)
        if (*s == '%') {
            if (s[1] == 's') {
                fputs(basename(programName), fp); // %s replaced by program name
                s++;
            } else {
                putc('%', fp); // %% treated as %
                if (s[1] == '%')
                    s++;
            }
        } else
            putc(*s, fp);
    fputs("\nAlso supports single arguments -v, -V for version info and -h for help\n", fp);
    Exit(fmt ? 1 : 0);
}