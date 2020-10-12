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
#include "omfcmp.h"


typedef struct {
    char *name;
    int size;
    byte *image;
    int startRec;
    int lengthRec;
    int pos;
    int error;
} omf_t;


omf_t *newOMF(file_t *fi, int mod, int start, int end);
void deleteOMF(omf_t *omf);

int seekRecord(omf_t *omf, int pos);
byte getByte(omf_t *omf);
byte *getName(omf_t *omf);
word getWord(omf_t *omf);
int getLoc(omf_t *omf);
int atEndOfRecord(omf_t *omf);
byte getRecord(omf_t *omf);