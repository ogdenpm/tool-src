/****************************************************************************
 *  memory.c is part of utility                                         *
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

#include <stdlib.h>
#include "utility.h"
/// <summary>
/// helper to malloc memory and check for out of memory
/// </summary>
/// <param name="size">Amount of memory to allocate in bytes.</param>
/// <returns>pointer to the allocated block</returns>
void *safeMalloc(size_t size) {
    void *p = malloc(size);
    if (!p)
        fatal("Out of memory");
    return p;
}


/// <summary>
/// helper to relloc memory and check for out of memory
/// </summary>
/// <param name="old">The old memory block</param>
/// <param name="size">The new size</param>
/// <returns></returns>
void *safeRealloc(void *old, size_t size) {
    void *p = realloc(old, size);
    if (!p)
        fatal("Out of memory");
    return p;
}