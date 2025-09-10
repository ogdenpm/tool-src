/****************************************************************************
 *  utility.h is part of utility                                         *
 *  Copyright (C) 2025 Mark Ogden <mark.pm.ogden@btinternet.com>            *
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
#include <stdio.h>

// exit.c
_Noreturn void Exit(int exitCode); // allows user defined exit handler

// message.c
extern FILE *logFp;
_Noreturn void fatal(char const *fmt, ...);
void warn(char const *fmt, ...);

// file.c
char const *basename(char const *path);
char *makeFilename(char const *path, char const *ext, bool force);

// memory.c
void *safeMalloc(size_t size);
void *safeRealloc(void *old, size_t size);

// option.c
/*
    Help contains the usage syntax and the option descriptions.
    When shown  it is preceded by the simple version information and app desciption
    And followed by a note about the standard -v, -V and -h options.
    The string can contain one %s which is replaced by the program name used to
    run the application
*/
#define MISSINGARG  0x8000 // option missing required argument
#define BADOPTION 0x4000 // invalid option
extern char const help[];
extern char const *optArg; // optional argument (or error message)
extern int optInd;         // current index into argv
extern int optOpt;         // option scanned
extern char *programName;

int getOpt(int argc, char **argv, const char *options);
void chkStdOptions(int argc, char **argv);
_Noreturn void usage(char *fmt, ...);
void showVersion(FILE *fp, bool full);

// getch.c
int _getch(void);
