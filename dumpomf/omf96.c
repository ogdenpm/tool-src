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
/* OMF96P - Record Types  */
#define MODHDR96                    0x02
#define MODEND96                    0x04
#define CONTENT96                   0x06
#define LINNUM96                    0x08
#define BLKDEF96                    0x0a
#define BLKEND96                    0x0c
#define EOF96                       0x0e
#define ANCESTOR96                  0x10
#define LOCALS96                    0x12
#define TYPEDEF96                   0x14
#define PUBLICS96                   0x16
#define EXTERNALS96                 0x18
#define SEGDEF96                    0x20
#define FIXUP96                     0x22
#define LIBLOC96                    0x26
#define LIBNAM96                    0x28
#define LIBDIC96                    0x2a
#define LIBBDR96                    0x2e

/* OMF96P - Translator Id */

#define O96_Trn_Id_Mask             0xe0
#define O96_Trn_ASM96               0x00
#define O96_Trn_PLM96               0x20
#define O96_Trn_C96                 0x40
#define O96_Trn_Unspec              0xe0

/* OMF96P - OMF_Ver */

#define O96_OMF_Ver_Mask            0x1e
#define O96_Min_OMF_Ver             0x00
#define O96_OMF96_V1dot0_DOC_V1dot4 0x00
#define O96_OMF96_V2dot0_DOC_V2dot0 0x04
#define O96_OMF96_V3dot0_DOC_V3dot0 0x06
#define O96_Max_OMF_Ver             0x06

/* OMF96P - Generator */

#define O96_Generator_Mask          0x01
#define O96_Translator96            0x00
#define O96_RL96                    0x01

#define O96_Ref_Type_Mask           0x3c
#define O96_Reg_Simple              0x00
#define O96_Reg_Auto_Incr           0x04
#define O96_Sh_Count_Imm            0x08
#define O96_Sh_Count_Reg            0x0c
#define O96_DCB_Const               0x10
#define O96_Short_Direct            0x14
/* O96_Short_Direct is not used */
#define O96_Short_Jmp               0x18
#define O96_Medium_Jmp              0x1c
#define O96_Medium_Call             0x20
#define O96_Long_Jmp_Call           0x24
#define O96_Long_Direct             0x28
#define O96_Far_Jmp_Call            0x2c // not yet supported
#define O96_Far_Direct              0x30 // not yet supported
#define O96_DCL_Const               0x34 // not yet supported
#define O96_Max_Byte_Ref_Type       0x18
#define O96_Max_Word_Ref_Type       0x28
#define O96_Max_24Bit_Ref_Type      0x30
#define O96_Max_Ref_Type            0x34

/* OMF96P - Nice and Easy */

#define O96_Nice_Mask               0x80
#define O96_Easy                    0x00
#define O96_Nice                    0x80

/* OMF96P - Types of Leaves */

#define O96_Leaf_Mask               0x7f
#define O96_Max_One_Byte_Sgnint     99
#define O96_Nil_Leaf                100
#define O96_Two_Byte_SgnInt         101
#define O96_Four_Byte_SgnInt        102
#define O96_String_Leaf             103
#define O96_Index_Leaf              104
#define O96_Repeat_Leaf             105
#define O96_End_Of_Branch_Leaf      106
#define O96_Max_Leaf_Type           106

/* OMF96P - Predefined Values for Numeric Leaves */

#define O96_Numeric_Pointer         110
#define O96_Numeric_Bit             109
#define O96_Numeric_Enum            108
#define O96_Numeric_Union           107
#define O96_Numeric_Scalar          99
#define O96_Numeric_Real            98
#define O96_Numeric_Entry           97
#define O96_Numeric_UnsInt          96
#define O96_Numeric_SgnInt          95
#define O96_Numeric_Array           94
#define O96_Numeric_Structure       93
#define O96_Numeric_List            92
#define O96_Numeric_Procedure       91
#define O96_Numeric_Label           90
#define O96_Numeric_Whole           89

#define WSEGID96                    19
#define WTYPEREF96                  8
#define WTYPEDEF96                  35

char const *typeLabel[] = { "Whole",  "Label",  "Procedure", "List",   "Structure", "Array",
                            "SgnInt", "UnsInt", "Entry",     "Real",   "Scalar",    "Nil",
                            "Int",    "Long",   "String",    "Index",  "Repeat",    "EoB",
                            "Union",  "Enum",   "Bit",       "Pointer" };

void omf96_02(int type);
void omf96_04(int type);
void omf96_06(int type);
void omf96_08(int type);
void omf96_0A(int type);
void omf96_0C(int type);
void omfEOF(int type);
void omf96_10(int type);
void omf96_12_16(int type);
void omf96_14(int type);
void omf96_18(int type);
void omf96_20(int type);
void omf96_22(int type);
void omfLIBLOC(int type);
void omfLIBNAM(int type);
void omfLIBDIC(int type);
void omfLIBHDR(int type);
void omf85_2E(int type);

decodeSpec_t omf96Decode[] = {
    /* 00 */ { "INVALID", invalidRecord, NULL },
    /* 02 */ { "MODHDR", omf96_02, NULL },
    /* 04 */ { "MODEND", omf96_04, NULL },
    /* 06 */ { "CONTENT", omf96_06, NULL },
    /* 08 */ { "LINNUM", omf96_08, NULL },
    /* 0A */ { "BLKDEF", omf96_0A, NULL },
    /* 0C */ { "BLKEND", omf96_0C, NULL },
    /* 0E */ { "EOF", omfEOF, NULL },
    /* 10 */ { "ANCESTOR", omf96_10, NULL },
    /* 12 */ { "LOCALS", omf96_12_16, NULL },
    /* 14 */ { "TYPEDEF", omf96_14, NULL },
    /* 16 */ { "PUBLICS", omf96_12_16, NULL },
    /* 18 */ { "EXTDEF", omf96_18, NULL },
    /* 1A */ { NULL, invalidRecord, NULL },
    /* 1C */ { NULL, invalidRecord, NULL },
    /* 1E */ { NULL, invalidRecord, NULL },
    /* 20 */ { "SEGDEF", omf96_20, NULL },
    /* 22 */ { "FIXUP", omf96_22, NULL },
    /* 24 */ { NULL, invalidRecord, NULL },
    /* 26 */ { "LIBLOC", omfLIBLOC, NULL },
    /* 28 */ { "LIBNAM", omfLIBNAM, NULL },
    /* 2A */ { "LIBDIC", omfLIBDIC, NULL },
    /* 2C */ { NULL, invalidRecord, NULL },
    /* 2E */ { "LIBHDR", omfLIBHDR, NULL },
};

char const *predef[] = { "NULL",  "BYTE", "WORD",  "LONG",   "ENTRY",  "INT8", "INT16",
                         "INT32", "REAL", "UINT8", "UINT16", "UINT32", "LABEL" };

int typeIndex;

void init96() {
    resetNames();
    setIndex(ISEG, 0, "CODE");
    setIndex(ISEG, 1, "DATA");
    setIndex(ISEG, 2, "REGISTER");
    setIndex(ISEG, 3, "OVERLAY");
    setIndex(ISEG, 4, "STACK");
    setIndex(ISEG, 5, "DYNAMIC");
    setIndex(ISEG, 6, "SEG_NULL");
    extIndex  = 0;
    typeIndex = 32;
}

void segId96(uint8_t segId) { // maxwidth = maxlen(predef) + 5 + 6 = 19
    addField("%s[%s%s]", getIndexName(ISEG, segId & 0x7), segId & 0x80 ? "REL" : "ABS",
             segId & 0x40 ? " BASED" : "");
}

void typeref96(int type) { // maxwidth = 8

    if (type <= 12)
        addField("%s", predef[type]);
    else
        addField("@%d", type);
}

/*
* due to the overlap of O96_Max_One_Byte_Sgnint, with some of the predefined numerics
* the code uses a bit mask to determine whether a value < O96_Max_One_Byte_Sgnint should be
* treated as a number (1) or predefiend (0)
* in the list below . stands for predefined and 1 stands for number
* the binary is read left to right
* unlike typedefs for 8086, each descriptor is terminated by an End of Block marker
    92  LIST ...            00 0000
    93  STRUCTURE n n ...   11 0000
    94  ARRAY n .           10 0000
    97  ENTRY . . . n .     00 0100
    99  SCALAR  n   ...     10 0000
    107 UNION n n ...       11 0000
    108 ENUM n . . . . n    10 0001
    109 BIT n n             11 0000
*/
/* don't need to encode lIST as 0 is the default */

static struct {
    uint8_t nleaf;
    uint8_t pat;
} firsts[] = { { O96_Numeric_Structure, 0x30 }, { O96_Numeric_Array, 0x20 },
               { O96_Numeric_Entry, 4 },        { O96_Numeric_Scalar, 0x20 },
               { O96_Numeric_Union, 0x30 },     { O96_Numeric_Enum, 0x21 },
               { O96_Numeric_Bit, 0x30 } };

void descriptor96() {
    uint8_t pattern;
    uint8_t patBit;
    bool isFirst = true;
    addField("");
    while (!atEndRec()) {
        uint8_t leaf = getu8();
        uint16_t index;

        if ((leaf & O96_Leaf_Mask) == O96_End_Of_Branch_Leaf)
            return;
        if (isFirst) {
            pattern = 0;
            for (int i = 0; i < sizeof(firsts) / sizeof(firsts[0]); i++)
                if (firsts[i].nleaf == leaf) {
                    pattern = firsts[i].pat;
                    break;
                }
            patBit  = 0x40;
            isFirst = false;
        } else {
            patBit >>= 1;
            add(" ");
        }
        if (leaf & O96_Nice_Mask) // put the nice marker
            add("'");
        leaf &= O96_Leaf_Mask;

        if (leaf <= O96_Max_One_Byte_Sgnint && (pattern & patBit))
            add("%d", leaf);
        else
            switch (leaf) {

            case O96_Numeric_Pointer:
            case O96_Numeric_Bit:
            case O96_Numeric_Enum:
            case O96_Numeric_Union:
            case O96_Nil_Leaf:
            case O96_Numeric_Scalar:
            case O96_Numeric_Real:
            case O96_Numeric_Entry:
            case O96_Numeric_UnsInt:
            case O96_Numeric_SgnInt:
            case O96_Numeric_Array:
            case O96_Numeric_Structure:
            case O96_Numeric_List:
            case O96_Numeric_Procedure:
            case O96_Numeric_Label:
            case O96_Numeric_Whole:
            case O96_Repeat_Leaf:
                add(typeLabel[leaf - O96_Numeric_Whole]);
                break;
            case O96_Two_Byte_SgnInt:
                add("%+d", geti16());
                break;
            case O96_Four_Byte_SgnInt:
                add("%+d", geti32());
                break;
            case O96_String_Leaf:
                add("%s", getName());
                break;
            case O96_Index_Leaf:
                index = getIndex();
                if (index <= 12)
                    add(predef[index]);
                else
                    add("@%d", index);
                break;
            default:
                add("Leaf%u", leaf);
                break;
            }
    }
}

void omf96_02(int type) { // MODHDR
    static char const *trnName[]  = { "ASM-96",  "PL/M-96", "C-96",    "UNK3-96",
                                      "UNK4-96", "UNK5-96", "UNK6-96", "ANY-96" };
    static char const *specName[] = { "OMF96 v1.4", "OMF96 v1.?", "OMF96 v2.0", "OMF96 v3.0",
                                      "OMF96 v?.?" };
    init96();

    char const *modName = getName();
    uint8_t verGen      = getu8();
    char const *timeStr = getName();

    uint8_t ver         = (verGen & O96_OMF_Ver_Mask);
    if (ver > (O96_Max_OMF_Ver))
        ver = O96_Max_OMF_Ver + 2;
    add("%s - %s", modName, specName[ver >> 1]);
    add(" - %s%s", trnName[(verGen & O96_Trn_Id_Mask) >> 5],
        verGen & O96_Generator_Mask ? "" : "|RL-96");
    if (*timeStr)
        add(" - %s", timeStr);
}

void omf96_04(int type) { // MODEND
    uint8_t mtype    = getu8();
    uint8_t validity = getu8();

    add("%s%s", mtype & 1 ? "Main " : "", validity & 1 ? "Invalid" : "");
}

void omf96_06(int type) {
    segId96(getu8());
    hexDump(getu16(), peekNextRecType() == FIXUP96);
}

void omf96_08(int type) { // LINNUM
    segId96(getu8());
    omfLINNUM();
}

void omf96_0A(int type) { // BLKDEF
    field_t const doHeader[] = {
        { "Location", WSEGID96 + 5 }, { "Size", 4 }, { "Type", WTYPEREF96 }, { NULL }
    };
    field_t const procHeader[] = { { "Location", WSEGID96 + 5 },
                                   { "Size", 4 },
                                   { "Type", WTYPEREF96 },
                                   { "Frame", (MAXNAME > WSEGID96 ? MAXNAME : WSEGID96) + 5 },
                                   { "RetOff", 8 },
                                   { "PrologSize" },
                                   { NULL } };

    char const *name           = getName();
    uint8_t segId              = getu8();
    uint16_t offset            = getu16();
    uint16_t size              = getu16();
    uint8_t flags              = getu8();
    uint16_t btype             = getIndex();
    add("%s:%s", flags & 0x40 ? "PROC" : "DO", name);

    addFixedHeader(flags & 0x40 ? procHeader : doHeader);

    startCol(1);
    segId96(segId);
    add(",%04X", offset);
    addField("%04X", size);
    if (*name)
        typeref96(btype);
    else
        addField("");
    if (flags & 0x40) { // proc
        if (flags & 0x80)
            addField("%s", getIndexName(IEXT, getu16()));
        else
            segId96(getu8());
        add(",%04X", getu16());

        addField("[FP+%u]", getu16()); // return offset
        addField("%02X", getu8());     // prologue size
    }
}

void omf96_0C(int type) { // BLKEND
}

void omf96_10(int type) { // ANCESTOR
    char const *name = getName();
    add("%s", name);
    if (!malformed)
        omf96_20(type); // process the seg defs
}

void omf96_12_16(int type) { // LOCAL/PUBLIC SYMBOLS
    field_t const header[] = { { "Offset" }, { "Name", WNAME }, { "Type", WTYPEREF96 }, { NULL } };
    uint8_t segId          = getu8();

    segId96(segId);
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%04X", getu16()); // offset
        addField("%s", getName());
        typeref96(getIndex()); // type
    }
}

void omf96_14(int type) { // TYPEDEFS
    field_t const header[] = { { "Index" }, { "Definition", WTYPEDEF96 }, { NULL } };
    int cols               = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        addField("@%d", typeIndex++);
        descriptor96();
    }
}

void omf96_18(int type) { // EXTERNAL SYMBOLS
    field_t const header[] = { { "Id", 4 }, { "Type", WTYPEREF96 }, { "Name", WNAME }, { NULL } };
    uint8_t segId          = getu8();

    segId96(segId);
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        char const *name = getName();
        uint16_t type    = getIndex();
        if (malformed)
            return;
        setIndex(IEXT, extIndex, name);
        addField("@%d", extIndex++);
        typeref96(type);
        addField("%s", name);
    }
}

void omf96_20(int type) { // SEGDEF
    static char const *relocNames[] = { "Byte", "Word", "Long", "????" };
    static field_t const header[] = { { "SegId", WSEGID96 }, { "Reloc", 8 }, { "Size" }, { NULL } };

    int cols                      = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        uint8_t segId = getu8();
        segId96(segId);
        if (segId & 0x80)
            addField("%s", relocNames[getu8() & 0x3]); // relocation mode
        else
            addField("Abs %04X", getu16()); // absolute
        addField("%04X", getu16());
    }
}

void omf96_22(int type) { // FIXUP
    static field_t const header[] = { { "Type", 15 },
                                      { "Align" },
                                      { "Value", (MAXNAME > WSEGID96 ? MAXNAME : WSEGID96) + 5 },
                                      { NULL } };

    static char const *align[]    = { "Byte", "Word", "Long", "????" };

    int cols                      = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        uint8_t ftype = getu8();
        uint16_t pcr  = getu16();
        switch (ftype & O96_Ref_Type_Mask) {
        case O96_Reg_Simple:
            addField("Reg(%u)", pcr & 0xff);
            break;
        case O96_Reg_Auto_Incr:
            addField("RegIncr(%u)", pcr & 0xff);
            break;
        case O96_Sh_Count_Imm:
            addField("Shift(%u)", pcr & 0xf);
            break;
        case O96_Sh_Count_Reg:
            addField("ShiftReg(%02X)", pcr & 0xff);
            break;
        case O96_DCB_Const:
            addField("DcbConst(%d)", (int16_t)pcr);
            break;
        case O96_Short_Jmp:
            addField("SJmp(%d)", (int16_t)pcr);
            break;
        case O96_Medium_Jmp:
            addField("MJmp(%d)", (int16_t)pcr);
            break;
        case O96_Medium_Call:
            addField("MCall(%d)", (int16_t)pcr);
            break;
        case O96_Long_Jmp_Call:
            addField("JmpCall(%s)", hexStr(pcr));
            break;
        case O96_Long_Direct:
            addField("Direct(%s)", hexStr(pcr));
            break;
        default:
            addField("Fixup%d(%s)", (ftype >> 2) & 0xf, hexStr(pcr));
            break;
        }
        addField("%s", align[ftype & 3]);

        if (ftype & 0x80)
            addField("%s", getIndexName(IEXT, getu16()));
        else
            segId96(getu8());
        if (!(ftype & 0x40))
            add(",%04X", getu16());
    }
}