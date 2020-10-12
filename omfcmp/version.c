/****************************************************************************
 *  omfcmp: compare two omf85 files                                         *
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
#include <stdbool.h>
#include "Generated/version.h"

// use the following function declaration in the main code
void showVersion(FILE *fp, bool full);

#ifdef GIT_APPID
#define APPNAME GIT_APPID
#else
#define APPNAME GIT_APPDIR
#endif

void showVersion(FILE *fp, bool full) {

    fputs(APPNAME " " GIT_VERSION, fp);
#ifdef _DEBUG
    fputs(" {debug}", fp);
#endif
    fputs("  (C)" GIT_YEAR " Mark Ogden\n", fp);
    if (full) {
        fputs(sizeof(void *) == 4 ? "32bit target" : "64bit target", fp);
        fprintf(fp, " Git: %s [%.10s]", GIT_SHA1, GIT_CTIME);
#if GIT_BUILDTYPE == 2
        fputs(" +uncommitted files", fp);
#elif GIT_BUILDTYPE == 3
        fputs(" +untracked files", fp);
#endif
        fputc('\n', fp);
    }
}
