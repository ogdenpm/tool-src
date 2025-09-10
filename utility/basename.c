/****************************************************************************
 *  basename.c is part of utility                                         *
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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
/// <summary>
/// Equivalent to POSIX basename for visual C
/// </summary>
/// <param name="path">full path to file</param>
/// <returns>pointer filename only</returns>

char const *basename(char const *path) {
    char *s;
#ifdef _WIN32
    if (path[0] && path[1] == ':') // skip leading device
        path += 2;
#ifdef UNC
    else if (path[0] == '\\' && path[1] == '\\') // skip \\computer\share\ part
        if ((s = strchr(path + 2, '\\')) && (s = strchr(s, '\\')))
            path = s + 1;
        else
            path = strchr(path, '\0');
#endif
#endif
    if ((s = strrchr(path, '/')))
        path = s + 1;
#if _WIN32
    if ((s = strrchr(path, '\\')))
        path = s + 1;
#endif
    return path;
}

#endif

