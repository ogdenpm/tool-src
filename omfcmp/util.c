/****************************************************************************
 *  omfcmp: compare two omf85 files                                         *
 *  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>            *
 *                                                                          *
 *  Permission is hereby granted, free of charge, to any person             *
 *  obtaining a copy of this software and associated documentation          *
 *  files (the "Software"), to deal in the Software without restriction,    *
 *  including without limitation the rights to use, copy, modify, merge,    *
 *  publish, distribute, sublicense, and/or sell copies of the Software,    *
 *  and to permit persons to whom the Software is furnished to do so,       *
 *  subject to the following conditions:                                    *
 *                                                                          *
 *  The above copyright notice and this permission notice shall be          *
 *  included in all copies or substantial portions of the Software.         *
 *                                                                          *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,         *
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF      *
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  *
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY    *
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,    *
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE       *
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                  *
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
    return (*s == *t && strncmp(s + 1, t + 1, *s) == 0);
}

int pstrCmp(byte *s, byte *t)
{
    int cmp;
    if ((cmp = strncmp(s + 1, t + 1, *s <= *t ? *s : *t)) == 0)
        cmp = *s - *t;
    return cmp;
}