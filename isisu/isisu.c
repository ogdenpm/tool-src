/****************************************************************************
 *  isisu: show length and start address of isis bin file                   *
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
