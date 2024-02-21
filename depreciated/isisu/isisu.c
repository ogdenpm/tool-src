/****************************************************************************
 *                                                                          *
 *  isisc: show length and start address of isis bin file                   *
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

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

void showVersion(FILE *fp, bool full);
char *invokedBy;

int getword(FILE *fp)
{
	int cl = getc(fp);
	int ch = getc(fp);
	if (cl == EOF || ch == EOF)
		return -1;
	return (ch << 8) + cl;
}

void usage(char *fmt, ...) {
	if (fmt) {
		va_list args;
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
	}
	fprintf(stderr, "\nUsage: %s -v | -V | file\n", invokedBy);

	exit(1);
}




int main(int argc, char **argv)
{
	FILE *fp;
	invokedBy = argv[0];

	if (argc != 2)
		usage(NULL);
	if (_stricmp(argv[1], "-v") == 0) {
		showVersion(stdout, argv[1][1] == 'V');
		exit(0);
	}
	if ((fp = fopen(argv[1], "rb")) == NULL)
		usage("can't open %s\n", argv[1]);

	while (1) {
		int len = getword(fp);
		int start = getword(fp);
		if (start < 0 || len <= 0) {
			if (len == 0)
				printf("load point: %04X\n", start);
			break;
		}
		printf("%04X - %04X\n", start, start + len - 1);
		while (len-- > 0)
			(void)getc(fp);
	}
	fclose(fp);
	return 0;
}
