/****************************************************************************
 *                                                                          *
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


#pragma once

/* tags used in parsing for separating rule elements*/
#define FIXTOKTAG   '\x1'
#define FIXSTRTAG   '\x2'
#define REPTOKTAG   '\x3'
#define REPSTRTAG   '\x4'
#define USRTOKTAG   '\x5'
#define DMPDATTAG   '\x6'
#define DMPIDATTAG  '\x7'

#define SEGFULL     ('/' + 0x80)        // for seg names the <class,overlay> which follows should be added
#define SEGSHORT    (0x80)              // for seg names the <class,overlay> is omitted

/* tags & flags used for condiitonal processing */
#define IF0TAG          0x80
#define IF1TAG          0x81
#define ELSIFFLAG       4
#define ENDIFFLAG       8

/* tag for reformatting in wider field*/
#define WIDETAG           0x90

extern uint8_t const **rules;
extern uint8_t minRecType;
extern uint8_t maxRecType;
extern uint32_t contentAddr;

extern char const *enumAlign85[5];
extern char const *enumFixup85[5];
extern char const *enumRelType51[7];
extern char const *enumRefType51[9];
extern char const *enumScope51[7];
extern char const *enumUsage51[7];
extern char const *enumSegType51[6];
extern char const *enumModDat86[5];
char const *enumAlign86[8];
char const *enumCombine86[9];
char const *enumAlign96[4];

void init85();
bool isTrn85(uint8_t trn);
void fmtTrn85(var_t *pvar);
void init51();
bool isTrn51(uint8_t trn);
void fmtTrn51(var_t *pvar);
bool isTrn96(uint8_t trn);
void init96();
void fmtTrn96(var_t *pvar);
void init86();
void printComment86(FILE *fpout);
void printTypedef86(FILE *fpout);