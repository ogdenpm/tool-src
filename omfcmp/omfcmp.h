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