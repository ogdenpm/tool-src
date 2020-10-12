/****************************************************************************
 *  plmpp: plm pre-processor                                                *
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
