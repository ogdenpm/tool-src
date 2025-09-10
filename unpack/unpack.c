/****************************************************************************
 *                                                                          *
 *  unpack: unpack packed source file                                       *
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <direct.h>
#define mkdir(p, m) _mkdir(p)
#else
#include <limits.h>
#include <unistd.h>
#define _MAX_PATH  PATH_MAX
#define _MAX_FNAME PATH_MAX
#endif
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "utility.h"

#define MAX_LINE 512

char const help[] =
    "Usage: %s [-r] [file]\n"
    "Where -r unpacks recursively\n";

// nested files are added to the end of the list below
// this allows for multiple levels of nesting
typedef struct _nested {
    struct _nested *next;
    char *file;
} nested_t;
nested_t *nestedFiles = NULL;

void addNestedFile(char *file) {
    nested_t *p = malloc(sizeof(nested_t));
    p->next     = NULL;
    p->file     = strdup(file);
    if (nestedFiles == NULL)
        nestedFiles = p;
    else {
        nested_t *q;
        for (q = nestedFiles; q->next; q = q->next)
            ;
        q->next = p;
    }
}

char *defaultIn() {
    char path[_MAX_PATH + 1];
    static char src[_MAX_FNAME];
    if (getcwd(path, _MAX_PATH + 1) == NULL) {
        fprintf(stderr, "problem finding current directory\n");
        exit(1);
    }
    char *parent = path;
    char *s;
    while ((s = strpbrk(parent, "\\/:")))
        parent = s + 1;
    strcpy(src, parent);
    strcat(src, "_all.src");
    return src;
}

FILE *newFile(char *parent, char *fname) {
    char path[_MAX_PATH];
    FILE *fp;

    if (*fname == '/' || *fname == '\\') // rooted filename
        strcpy(path, fname);
    else {
        strcpy(path, parent); // assume start directory is same as parent
        char *fpart = path;
        for (char *s = path; (s = strpbrk(s, "\\/")); s++)
            fpart = s + 1;
        strcpy(fpart, fname);
    }

    if ((fp = fopen(path, "wt")) == NULL) { // try to create
        // see if the problem is missing directories
        for (char *s = path; (s = strpbrk(s, "\\/")); s++) {
            *s = 0;
            mkdir(path, 0666);
            *s = '/';
        }
        fp = fopen(path, "wt");
    }
    if (fp == NULL) {
        warn("Can't create %s - using stdout", path);
        fp = stdout;
    } else
        printf("%s\n", path);
    return fp;
}

void unpack(char *fname) {
    char line[MAX_LINE];
    char curfile[_MAX_PATH];
    bool isnested = false;
    FILE *fpin;
    FILE *fpout = stdout;
    *curfile    = 0;
    if ((fpin = fopen(fname, "rt")) == NULL)
        usage("can't open %s", fname);

    while (fgets(line, MAX_LINE, fpin)) {
        if (*line == '\f') {
            if (line[1] == '?') {
                if (!isnested) {
                    addNestedFile(curfile);
                    isnested = true;
                }
                putc('\f', fpout);
                fputs(line + 2, fpout);
            } else {
                if (fpout != stdout)
                    fclose(fpout);
                char *s;
                for (s = line + 1; (s = strchr(s, '`')); s++) // replace escaped spaces
                    *s = ' ';
                if ((s = strchr(line + 1, '\n')))
                    *s = 0;
                strcpy(curfile, line + 1);
                fpout    = newFile(fname, curfile);
                isnested = false;
            }
        } else
            fputs(line, fpout);
    }
    if (fpout != stdout)
        fclose(fpout);
    fclose(fpin);
}

int main(int argc, char **argv) {
    char *infile;
    bool recurse = false;

    while (getOpt(argc, argv, "r") != EOF) {
        switch (optOpt) {
        case 'r':
            recurse = true;
            break;
        default:
            usage("Unknown option -%c", optOpt & 0xff);
        }
    }

    if (optInd == argc)
        infile = defaultIn();
    else if (optInd + 1 == argc)
        infile = *argv;
    else
        usage("Too many input files");

    unpack(infile);
    if (recurse)
        while (nestedFiles) { // rely on OS to clean up memory
            unpack(nestedFiles->file);
            nestedFiles = nestedFiles->next;
        }
}