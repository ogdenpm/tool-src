/****************************************************************************
 *  image.h is part of abstool                                         *
 *  Copyright (C) 2024 Mark Ogden <mark.pm.ogden@btinternet.com>            *
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
#include <stdint.h>
#include <stdbool.h>

#define MAXMEM  0x10000     // max image size
#define MAXAPPEND   256     // allow for 2 x 128 byte sectors of appended data even after MAXMEM 


/* types for values */
enum {
    NOTSET = 0, // memory use values
    SET,
    UNSET,       // use updates for patches
    CHANGE,
    APPEND,     // also start of patch keywords
    AOMF51,      // binary file types, also meta values
    AOMF85,
    AOMF96,
    ISISBIN,
    HEX,
    IMAGE,
    TARGET,     // meta tokens
    SOURCE,
    NAME,
    DATE,
    START,
    LOAD,
    TRN,
    VER,
    MAIN,
    MASK,
    STARTADDR,
    SKIP,       // patch values
    DEINIT,
    HEXBYTE,
    HEXWORD,
    STRING,
    EOL,
    ERROR,
};


// Intel AOMFXX record types
#define MODHDR     2
#define MODEND     4
#define MODCONTENT 6
#define MODEOF     0xe

#define mTRN       meta[TRN - TRN


typedef struct {
    int low;
    int high;
    int padLen;
    uint8_t mem[MAXMEM + MAXAPPEND + 1]; // allows for append and a final NOTSET sentinal
    uint8_t use[MAXMEM + MAXAPPEND + 1];
    // meta data
    int8_t target;   // AOMF51, AOMF85, AOMF96, ISISI, HEX, IMAGE
    int8_t source;
    uint8_t name[41];
    uint8_t date[65];
    // numeric meta data
    int meta[MASK - START + 1];
} image_t;

#define mStart meta[0]
#define mLoad  meta[1]
#define mTrn   meta[2]
#define mVer    meta[3]
#define mMain  meta[4]
#define mMask  meta[5]

extern int loadAddr;

bool loadFile(char *s, image_t *image);
_Noreturn void error(char *fmt, ...);
void warning(char *fmt, ...);