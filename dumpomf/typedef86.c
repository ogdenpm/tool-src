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

#include "omf.h"
#include "omf86.h"

enum {
    FAR = 97,
    NEAR,
    INTERRUPT,
    TFILE,
    PACKED,
    UNPACKED,
    SET,
    RES104,
    CHAMELEON,
    BOOLEAN,
    TRUE,
    FALSE,
    CHAR,
    INTEGER,
    CONST,
    RES112,
    LABEL,
    LONG,
    SHORT,
    PROCEDURE,
    PARAMETER,
    DIMENSION,
    ARRAY,
    RES120,
    STRUCTURE,
    POINTER,
    SCALAR,
    UNSIGNED_INTEGER,
    SIGNED_INTEGER,
    REAL,
    LIST
};

char const *leaf86Names[] = {
    "FAR",
    "NEAR",
    "INTERRUPT",
    "TFILE",
    "PACKED",
    "UNPACKED",
    "SET",
    "#104",
    "CHAMELEON",
    "BOOLEAN ",
    "TRUE",
    "FALSE",
    "CHAR",
    "INTEGER",
    "CONST",
    "#112",
    "LABEL",
    "LONG",
    "SHORT",
    "PROCEDURE",
    "PARAMETER",
    "DIMENSION",
    "ARRAY",
    "#120",
    "STRUCTURE",
    "POINTER",
    "SCALAR",
    "UNSIGNED_INTEGER",
    "SIGNED_INTEGER",
    "REAL",
    "LIST",
};

/*
* due to the overlap of small integers, with some of the predefined numerics
* the code uses a bit mask to determine whether a single byte value should be
* treated as a number (1) or predefined (0)
* in the list below . stands for predefined and 1 stands for number
* the binary is read left to right
* Note only known type representations with numeric values are considered
* elements before one of these types are assumed to be predefined

    116  PROCEDURE ... n .   0001
    119  ARRAY n .           1000
    121  STRUCTURE n n .     1100
    123  SCALAR  n .         1000
*/

char const *procLabels[]   = { "", "retType: ", " ret: ", " params: ", " types: ", NULL };
char const *labelLabels[]  = { "", "jmp: ", NULL };
char const *paramLabels[]  = { "type: ", NULL };
char const *scalarLabels[] = { "bits: ", " type: ", NULL };
char const *structLabels[] = { "bits: ", " members: ", " types: ", " names: ", NULL };
char const *arrayLabels[]  = { "bits: ", " type: ", NULL };
char const * noLabels[] = { NULL };

void descriptor86() {
    char const **labels = noLabels;
    uint8_t labelIndex  = 0;
    uint8_t pattern     = 0;
    uint8_t patBit      = 0;
    bool isFirst        = true;
    uint16_t val;

    uint8_t en;
    uint8_t enMsk = 0;

    while (!atEndRec()) {
        if (enMsk == 0) {
            en = getu8();
            if (atEndRec())
                return;
            enMsk = 0x80;
        }
        if (labels[labelIndex])
            add(labels[labelIndex++]);

        if (enMsk & en)
            add("'");
        enMsk >>= 1;
        uint8_t leaf = getu8();
        if (isFirst) {
            isFirst = false;
            patBit  = 0x10;
            switch (leaf) {
            case PROCEDURE:
                pattern = 1;
                labels  = procLabels;
                break;
            case ARRAY:
                pattern = 0x8;
                labels  = arrayLabels;
                break;
            case SCALAR:
                pattern = 0x8;
                labels  = scalarLabels;
                break;
            case STRUCTURE:
                pattern = 0xc;
                labels  = structLabels;
                break;
            case LIST:
                break;
            case PARAMETER:
                labels = paramLabels;
                break;
            case LABEL:
                labels = labelLabels;
                break;
            default:
                isFirst = false;
            }
        }
        if (FAR <= leaf && leaf <= LIST && !(pattern & patBit))
            add("%s", leaf86Names[leaf - FAR]);
        else if (leaf < 128)
            add("%u", leaf);
        else
            switch (leaf) {
            case 128:
                add("nil");
                break;
            case 129:
                add("%u", getu16());
                break;
            case 130:
                add("'%s'", getName());
                break;
            case 131:
                val = getIndex();
                if (val < 10)
                    add("@%u", val);
                else
                    add("@%u", val);
                break;
            case 132:
                add("%u", getu24());
                break;
            case 133:
                add("*");
                break;
            case 134:
                add("%d", geti8());
                break;
            case 135:
                add("%d", geti16());
                break;
            case 136:
                add("%d", geti32());
                break;
            default:
                add("leaf %d", leaf);
                break;
            }
        patBit >>= 1;
        add(" ");
    }
}
