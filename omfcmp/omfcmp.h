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


#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef unsigned char byte;
typedef unsigned short word;

#include "util.h"
#include "file.h"
#include "omf.h"
#include "library.h"


#define MAXNAME	31
#define INITIALCHUNK 10

enum {
    MODHDR = 0x2,
    MODEND = 0x4,
    CONTENT = 0x6,
    LINNUM = 0x8,
    EOFREC = 0xE,
    PARENT = 0x10,
    LOCNAM = 0x12,
    PUBNAM = 0x16,
    EXTNAM = 0x18,
    EXTREF = 0x20,
    RELOC = 0x22,
    INTSEG = 0x24,
    LIBHDR = 0x2C,
    LIBNAM = 0x28,
    LIBLOC = 0x26,
    LIBDIC = 0x2a,
    COMMON = 0x2e,
};

extern int returnCode;
void diffBinary(omf_t *left, omf_t *right);
void cmpModule(omf_t *lomf, omf_t *romf);
void usage(char *fmt, ...);