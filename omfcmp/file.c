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

file_t *newFile(char *fn)
{
    FILE *fp;
    long fileSize;
    file_t *fi = NULL;

    if ((fp = fopen(fn, "rb")) == NULL) {
        usage("can't open %s\n", fn);
        return NULL;                        
    }
    fseek(fp, 0, SEEK_END);
    if ((fileSize = ftell(fp)) <= 0)
        fprintf(stderr, "error reading %s\n", fn);
    else {
        rewind(fp);
        fi = (file_t *)xmalloc(sizeof(file_t));
        fi->image = (byte *)xmalloc(fileSize);

        if (fread(fi->image, 1, fileSize, fp) != fileSize) {
            fprintf(stderr, "error reading %s\n", fn);
            free(fi->image);
            free(fi);
            fi = NULL;
        }
        else {
            fi->size = fileSize;
            fi->name = fn;
        }
    }
    fclose(fp);
    return fi;
}
