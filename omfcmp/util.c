/****************************************************************************
 *                                                                          *
 *  omfcmp: compare two omf85 files                                         *
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

#include "omfcmp.h"

void *xmalloc(size_t len)
{
    void *result = malloc(len);
    if (result == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return result;
}

void *xrealloc(void *buf, size_t len)
{
    void *result = (void *)realloc(buf, len);
    if (result == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return result;
}

void *xcalloc(size_t len, size_t size)
{
    void *result = calloc(len, size);
    if (result == NULL) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return result;
}


int pstrEqu(byte *s, byte *t)
{
    return (*s == *t && strncmp((char *)s + 1, (char *)t + 1, *s) == 0);
}

int pstrCmp(byte *s, byte *t)
{
    int cmp;
    if ((cmp = strncmp((char *)s + 1, (char *)t + 1, *s <= *t ? *s : *t)) == 0)
        cmp = *s - *t;
    return cmp;
}