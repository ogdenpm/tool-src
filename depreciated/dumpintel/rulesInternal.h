/****************************************************************************
 *                                                                          *
 *  dumpintel: dump content of an Intel omf file                            *
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