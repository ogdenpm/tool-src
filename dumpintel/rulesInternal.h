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
/* stringified versions of the IFx Tags*/
#define IF0(cond)       "\x80" cond "\b"
#define IF1(cond)       "\x81" cond "\b"
#define ELSIF0(cond)    "\x84" cond "\b"
#define ELSIF1(cond)    "\x85" cond "\b"
#define ELSE0           ELSIF0("1")
#define ELSE1           ELSIF1("1")
#define END0            "\x88"
#define END1            "\x89"

/* stringified version of the rule separators*/
#define FIXTOK      "\x1"
#define FIXSTR      "\x2"
#define REPTOK      "\x3"
#define REPSTR      "\x4"
#define USRTOK      "\x5"
#define DMPDAT      "\x6"
#define DMPIDAT     "\x7"



// stringified versions of width / spacing
#define WIDE(var, width)  #var "\x90" #width "\b"
#define SPC(width)       WIDE($, width)