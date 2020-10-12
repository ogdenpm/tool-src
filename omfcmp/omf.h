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