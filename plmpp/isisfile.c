/****************************************************************************
 *                                                                          *
 *  plmpp: plm pre-processor                                                *
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

/*
	routines to map isis files to unix/windows files
	code adapted from Thames source simplified as only :Fx: supported
*/
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
//#include <direct.h>

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif

int isiswarning[10];

/*
* Given a drv number look up the appropriate environment
* variable(eg: ISIS_F0)
*/
static const char *xlt_device(unsigned drv)
{
	char buf[8] = { "ISIS_F0" };

	buf[6] = drv + '0';

	return getenv(buf);
}

static void mapIsisName(const char *isisName, char *mappedName)
{
	const char *fname;
	char *s;
	const char *path;
	int drv;

	/* only make :Fx: names */
	if (*isisName != ':')
		strcpy(mappedName, isisName);
	else {
		/* check ISIS filename - if it doesn't start with a device
		specifier, assume :F0:
		*/
		if (strlen(isisName) < 4 || isisName[0] != ':' || isisName[3] != ':') {
			fname = isisName;
			drv = 0;
		}
		else if (toupper(isisName[1]) != 'F' || !isdigit(isisName[2])) {
			fprintf(stderr, "Cannot map ISIS file %s\n", isisName);
			strcpy(mappedName, isisName);
			return;
		}
		else {
			fname = isisName + 4;	// past the :Fx:
			drv = isisName[2] - '0';
		}

		if (path = xlt_device(drv))
			strcpy(mappedName, path);
		else {
			if (isiswarning[drv]++ == 0)
				fprintf(stderr, "No mapping for ISIS_F%d, assuming current directory\n", drv);
			*mappedName = 0;
		}

		if (mappedName[0] != 0) {
			// make sure a directory separator is present
			if (*(s = strchr(mappedName, 0) - 1) != '/' && *s != '\\') {
				*++s = '/';
				*++s = 0;
			}
		}
		s = strchr(mappedName, 0);
		while (*s++ = tolower(*fname++))
			;
	}
}


FILE *openIsisFile(const char *isisName, char *mode)
{
	char mappedName[_MAX_PATH];
	mapIsisName(isisName, mappedName);
	if (mappedName[0] == ':')
		return NULL;
	return fopen(mappedName, mode);
}
