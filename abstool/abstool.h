/****************************************************************************
 *  abstool.h is part of abstool                                         *
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
#include <stdbool.h>
#include <stdint.h>
#include "image.h"

#define HEXBYTES  16

#ifndef _MSC_VER
#define stricmp strcasecmp
#endif

_Noreturn void usage(char *fmt, ...);
_Noreturn void error(char *fmt, ...);
void warning(char *fmt, ...);

void setOMFRec(image_t *image, int outFormat);
void insertJmpEntry();
int saveFile(char *s, image_t *image);

int parseHex(char *s, char **end);
void patchfile(char *s, image_t *image);
