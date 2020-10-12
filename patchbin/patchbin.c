/****************************************************************************
 *  patchbin: apply patches to a CPM .com file                              *
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


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

void showVersion(FILE *fp, bool full);
char *invokedBy;

char *gethex(char *s, unsigned *val)
{
    *val = 0;
    while (*s == ' ' || *s == '\t')
        s++;
    if (!isxdigit(*s))
        return 0;
    while (isxdigit(*s)) {
        *val = *val * 16 + (isdigit(*s) ? *s - '0' : toupper(*s) - 'A' + 10);
        s++;
    }
    return s;
}


__declspec(noreturn) void usage(char *fmt, ...) {

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
    }
    fprintf(stderr, "\nUsage: %s -v | -V | patchfile filetopatch\n", invokedBy);

    exit(1);
}

int main(int argc, char **argv)
{
    FILE *pfp, *fp;
    unsigned addr, patch;
    char line[256];
    char *s;
    long fsize;
    int append;
    invokedBy = argv[0];

    if (argc == 2 && _stricmp(argv[1], "-v") == 0) {
        showVersion(stdout, argv[1][1] == 'V');
        exit(0);
    }
    if (argc != 3 || (pfp = fopen(argv[1], "rt")) == NULL || (fp = fopen(argv[2], "r+b")) == NULL)
        usage(NULL);

    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);


    while (fgets(line, 256, pfp)) {
        for (s = line; *s && (*s == ' ' || *s == '\t' || *s == '\n'); s++)
            ;
        if (!*s || *s == '#')
            continue;

        if ((s = gethex(s, &addr)) == NULL || addr < 0x100) 
            fprintf(stderr, "bad address: %s", line);
        else {
            if ((long)addr - 0x100 >= fsize) {
                fseek(fp, 0, SEEK_END);

                while ((long)addr - 0x100 > fsize) {
                    putc(0, fp);
                    fsize++;
                }
                append = 1;
            }
            else {
                fseek(fp, addr - 0x100, SEEK_SET);
                append = 0;
            }

            while ((s = gethex(s, &patch))) {
                if (patch < 0 || patch >= 0x100) {
                    fprintf(stderr, "bad patch (%04X) in line %s", patch, line);
                    break;
                }
                else {
                    putc(patch, fp);
                    fsize += append;
                }
            }
        }
    }
    fclose(pfp);
    fclose(fp);
    return 0;
}