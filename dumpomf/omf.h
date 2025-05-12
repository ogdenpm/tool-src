/****************************************************************************
 *  omf.h is part of dumpomf                                         *
 *  Copyright (C) 2022 Mark Ogden <mark.pm.ogden@btinternet.com>            *
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

#ifndef _OMF_H
#define _OMF_H


#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXPOS   110 /* max normal position excluding the indent*/
#define MAXNAME  20  /* max name stored for indexes, greater than this then simple @n is used */
#define MAXTOKEN 10
// normal width allowances
#define WNAME85  12
#define WNAME51  16
#define WNAME96  20
#define WNAME86  25
#define WNAME   24
#define WFIXUP51    30
#define WINFO51  22

#define HEXWIDTH 3
#define ASCIICOL (16 * HEXWIDTH + 4 + 2) /* addr + 16 hex + 3 extra separators + '|' + 2 spaces */

#define is32bit  (recType & 1)

typedef struct {
    uint8_t len;
    char str[];
} pstr;

enum { Byte = 0, Word, DWord, Decimal, EIndex, String, Expr, Error };

extern bool atEnd;

#define MAXREC    0x10000
#define MAXEXTERN 2048
extern uint8_t rec[MAXREC];
extern int recType;
extern uint8_t *recPtr;
extern bool malformed;

enum { Junk = -2, Eof = -1, BadCRC = 0, Ok = 1 };

enum omf_e { OMFUKN = 0, OMF85, OMF51, OMF51K, OMF96, OMF86};
enum flavour_e { ANY, INTEL, MS, IBM, PHARLAP};

extern enum flavour_e omfFlavour;

typedef struct {
    char const *name; /* starts with + if odd record number is supported */
    void (*handler)(int type);
    uint8_t *reserved;
} decodeSpec_t;

typedef struct _omfDispatch {
    void (*init)();
    uint8_t low;
    uint8_t high;
    decodeSpec_t *decodeTable;
} omfDispatch_t;

typedef struct {
    uint16_t len;
    uint32_t ival;
    char sval[256];
} var_t;

typedef struct {
    uint16_t tabStop;
    char const *label;
} ofield_t;

typedef struct field {
    char const *label;
    uint16_t width;
} field_t;



extern long start;
#define INDEXCHUNK 256

#define FIXED      0
#define REPEAT     1

extern uint8_t const *indexKeys;

extern FILE *src;
extern FILE *dst;
extern int extIndex;
extern int segIndex;

void Log(char const *fmt, ...);

enum { ISEG = 0, IEXT, INAME, ITYPEDEF, IOVERLAY, IGROUP , IBLOCK, ICOMDAT, INDEXTABLES};

    /* mem.c */
void resetNames(void);
char const *pstrdup(uint16_t len, char const *s);
char const *concat(char *s, ...);
void setIndex(uint8_t tableIdx, uint16_t idx, char const *name);
char const *getIndexName(uint8_t tableIdx, uint16_t idx);

/* main.c */
_Noreturn void usage(char const *s);
int loadRec(void);
int getrec(void);
bool atEndRec(void);
void markRecPos(void);
uint16_t revertRecPos(void);
uint8_t getu8(void);
uint16_t getu16(void);
uint32_t getu24(void);
uint32_t getu32(void);
int8_t geti8(void);
int16_t geti16(void);
int32_t geti24(void);
int32_t geti32(void);
uint16_t getIndex(void);
char const *getName(void);
int detectOMF(void);
int main(int argc, char **argv);

/* common.c */
void startCol(int n);
void _add(char const *fmt, va_list args);
void addAt(int col, char const *fmt, ...);
void addField(char const *fmt, ...);
void add(char const *fmt, ...);
void splitLine(void);
void displayLine(void);
void Log(char const *fmt, ...);
void invalidRecord(int type);
void displayFile(int spec);
void hexDump(unsigned addr, bool showLoc);

void oaddHeader(uint8_t cols, ofield_t const *fields);

void omf85_06(int type);
void omfLINNUM();
void omfLIBLOC(int type);
void omfLIBNAM(int type);
void omfLIBDIC(int type);
void omfLIBHDR(int type);
void undoCol();
uint16_t getCol();

bool isValidRec(int spec);
uint16_t getRecPos();
void setRecPos(uint16_t pos);

void base86();
void descriptor86();
void fixCol();
uint8_t peekNextRecType();
void flagMalformed();
int addReptHeader(field_t const *fields);
void addFixedHeader(field_t const *fields);
char const *hexStr(uint32_t n);

#endif