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



omf_t *newOMF(file_t *fi, int mod, int start, int end)
{
    omf_t *omf;
    char modStr[8];

    if (mod < 0)
        modStr[0] = 0;
    else
        sprintf(modStr, "[%d]", mod & 0xffff);

    omf = (omf_t *)xcalloc(1, sizeof(omf_t));
    omf->name = (char *)xmalloc(strlen(fi->name) + strlen(modStr)  + 1);

    strcpy(omf->name, fi->name);
    strcat(omf->name, modStr);
    omf->size = end - start;
    omf->image = fi->image + start;
    seekRecord(omf, 0);
    return omf;
}

void deleteOMF(omf_t *omf)
{
    free(omf->name);
    free(omf);
}


/* utility functions to help with processing the object file */

int seekRecord(omf_t *omf, int pos)		// forces seek to new location in file
{
    omf->pos = pos;
    omf->lengthRec = 0;
    if (omf->pos >= omf->size)
        omf->error = 1;
    return !omf->error;
}

byte getRecord(omf_t *omf)					// sets up the next record and returns its type. The length is also skipped
{
    int crc, i;

    if (omf->lengthRec)	// skip any existing record
        omf->pos = omf->startRec + omf->lengthRec + 3;
    if (omf->pos < omf->size - 3) {
        omf->startRec = omf->pos;
        omf->lengthRec = omf->image[omf->pos + 1] + omf->image[omf->pos + 2] * 256;
        omf->pos += 3;
        for (crc = i = 0; i < omf->lengthRec + 3; i++)
            crc += omf->image[omf->startRec + i];
        if (crc & 0xff)
            omf->error = 1;
        return omf->error == 0 ? omf->image[omf->startRec] : 0;
    }
    omf->error = 1;
    return 0;
}

byte getByte(omf_t *omf)					// gets byte within a record
{
    if (omf->pos < omf->startRec + omf->lengthRec + 2)
        return omf->image[omf->pos++];
    omf->error = 1;
    return 0;
}

byte *getName(omf_t *omf)					// returns a name value with in the record
{
    byte *name = omf->image + omf->pos;

    if ((omf->pos += name[0] + 1) > omf->startRec + omf->lengthRec + 1)
        omf->error = 1;
    return name;

}

word getWord(omf_t *omf)					// returns a word within the record
{
    word c = getByte(omf);
    return c + getByte(omf) * 256;
}

int getLoc(omf_t *omf)						// returns a location offset within the record
{
    int c = getWord(omf) * 128;
    return c + getWord(omf);
}


int atEndOfRecord(omf_t *omf)
{
    return omf->pos >= omf->startRec + omf->lengthRec + 2;
}

