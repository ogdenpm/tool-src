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

void omf51_02(int type);
void omf51_04(int type);
void omf51_06(int type);
void omf51_08(int type);
void omf51_0E(int type);
void omf51_10(int type);
void omf51_12(int type);
void omf51_16(int type);

void omf51_18(int type);

decodeSpec_t omf51Decode[] = {
    /* 00 */ { "INVALID", invalidRecord, NULL },
    /* 02 */ { "MODHDR", omf51_02, NULL },
    /* 04 */ { "MODEND", omf51_04, NULL },
    /* 06 */ { "CONTENT", omf51_06, NULL },
    /* 08 */ { "FIXUP", omf51_08, NULL },
    /* 0A */ { NULL, invalidRecord, NULL },
    /* 0C */ { NULL, invalidRecord, NULL },
    /* 0E */ { "SEGDEF", omf51_0E, NULL },
    /* 10 */ { "SCOPEDEF", omf51_10, NULL },
    /* 12 */ { "DBGITEM", omf51_12, NULL },
    /* 14 */ { NULL, invalidRecord, NULL },
    /* 16 */ { "PUBLICS", omf51_16, NULL },
    /* 18 */ { "EXTDEF", omf51_18, NULL },
    /* 1A */ { NULL, invalidRecord, NULL },
    /* 1C */ { NULL, invalidRecord, NULL },
    /* 1E */ { NULL, invalidRecord, NULL },
    /* 20 */ { NULL, invalidRecord, NULL },
    /* 22 */ { NULL, invalidRecord, NULL },
    /* 24 */ { NULL, invalidRecord, NULL },
    /* 26 */ { "LIBLOC", omfLIBLOC, NULL },
    /* 28 */ { "LIBNAM", omfLIBNAM, NULL },
    /* 2A */ { "LIBDIC", omfLIBDIC, NULL },
    /* 2C */ { "LIBHDR", omfLIBHDR, NULL },
};

void init51() {
    resetNames();
    setIndex(ISEG, 0, "ABS");
    extIndex = segIndex = 0;
}

void segInfo51(uint8_t n) { // Max width = 5 + 6 + 11 = 22
    char const *segTypes[] = { "CODE", "XDATA", "DATA", "IDATA", "BIT", "STYP5", "STYP6", "STYP7" };
    addField("%s", segTypes[n & 0x7]);
    if (n & 0x80)
        add(" Empty");
    uint8_t bank = (n >> 3) & 3;
    if (n & 0x20)
        add(" Ovl bank %d", bank);
    else if (bank)
        Log("Non-zero bank(%d) for nonoverlayable segment", bank);
}

void symInfo51(uint8_t n) { // Max width = 6 + 5 + 4 + 7 = 22
    char const *usage[] = { "CODE", "XDATA", "DATA", "IDATA", "BIT", "NUMBER", "INFO6", "INFO7" };

    addField("%s", usage[n & 7]);
    if (n & 0x40) // variable
        add(" VAR");
    else {
        add(" PROC");
        if (n & 0x80)
            add(" IND");
        if (n & 0x10)
            add(" bank %d", (n >> 3) & 3);
    }
}

void omf51_02(int type) {
    char const *trnName[] = { "ASM51", "PL/M-51", "RL51", "Bad TRN" };

    init51();
    char const *modName = getName();
    uint8_t trn         = getu8();
    getu8();
    add("%s - %s", modName, trn > -0xfd ? trnName[trn - 0xfd] : trnName[3]);
}

void omf51_04(int type) {
    char const *modName = getName();
    getu16();
    uint8_t regMsk = getu8();
    getu8();
    add("%s", modName);
    if (regMsk & 0xf) {
        add(" Uses banks ");
        char *space = "";
        for (int i = 0; i < 4; i++)
            if (regMsk & (1 << i)) {
                add("%s%d", space, i);
                space = ", ";
            }
    }
}

void omf51_06(int type) {
    add("Seg[%s]", getIndexName(ISEG, getu8()));
    hexDump(getu16(), peekNextRecType() == 8);
}

void omf51_08(int type) {
    /* longest fixup field is
     * RELATIVE((PSeg[name] + 20)*8 + xxxx)
     * i.e. 21 + MAXNAME
     */
    char const *fixups[] = { "Low", "Byte", "Relative", "High", "Word", "Inblock", "Bit", "Conv" };

    static field_t const header[] = { { "Loc", 4 }, { "FixupOp(Target)", WFIXUP51 }, { NULL } };
    int cols                      = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        addField("%04X", getu16()); // loc
        uint8_t refTyp = getu8();
        addField("%s(", fixups[refTyp & 7]);
        uint8_t idBlk   = getu8();
        uint8_t id      = getu8();
        uint16_t offset = getu16();

        if ((refTyp & 7) == 7)
            add("(");
        switch (idBlk) {
        case 0:
            add("Seg[%s]", getIndexName(ISEG, id));
            break;
        case 1:
            add("PSeg[%s]", getIndexName(ISEG, id));
            break;
        case 2:
            add("%s", getIndexName(IEXT, id));
            break;
        default:
            add("ID%d", idBlk);
            break;
        }
        if ((refTyp & 7) == 7)
            add("-20H)*8");
        if (offset) {
            if (offset >= 0x8000)
                add(" - %s", hexStr(0x10000 - offset));
            else
                add(" + %s", hexStr(offset));
        }
        add(")");
    }
}

void omf51_0E(int type) {
    char const *relTypes[] = { "ABS", "UNIT", "BITADDRESSABLE", "INPAGE", "INBLOCK", "PAGE" };

    static field_t const header[] = { { "Id", 4 },      { "Name", WNAME },      { "Base:Size" },
                                      { "RelTyp", 14 }, { "SegInfo", WINFO51 }, { NULL } };
    int cols                      = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        uint8_t segId = getu8();

        if (segId && segId != ++segIndex) {
            Log("Unexpected Segment Definition %d - expected %d", segId, segIndex);
            segIndex = segId;
        }

        uint8_t segInfo = getu8();
        uint8_t relTyp  = getu8();
        getu8();
        uint16_t segBase    = getu16();
        uint16_t segSize    = getu16();
        char const *segName = getName();

        if (segId)
            setIndex(ISEG, segId, segName);
        addField("@%d", segId);
        addField("%s", *segName ? segName : "*Unnamed*");
        addField("%04X:%04X", segBase, segSize);
        if (relTyp < 6)
            addField("%s", relTypes[relTyp]);
        else
            addField("Rel_%d", relTyp);
        segInfo51(segInfo);
    }
}

void omf51_10(int type) {
    char const *scope[] = { "MODULE", "DO", "PROCEDURE", "MODULE END", "DO END", "PROCEDURE END" };

    uint8_t blkTyp      = getu8();
    char const *blkName = getName();
    add("%s: ", blkName);
    if (blkTyp <= 5)
        add("%s", scope[blkTyp]);
    else
        add("Scope@%d", blkTyp);
}

void omf51_12(int type) {
    static char const *types[]    = { "Locals", "Publics", "Segments", "Line Numbers" };

    static field_t const header[] = {
        { "Name", WNAME }, { "Segment:Offset", MAXNAME + 5 }, { "Info", WINFO51 }, { NULL }
    };

    static field_t const linHeader[] = { { "Segment:Offset", MAXNAME + 5 },
                                         { "Line", 5 },
                                         { NULL } };

    uint8_t defTyp                   = getu8();
    if ((malformed = malformed || (defTyp > 3)))
        return;
    add("%s", types[defTyp]);

    int cols = addReptHeader(defTyp < 3 ? header : linHeader);

    while (!atEndRec()) {
        startCol(cols);
        if (defTyp < 3) {
            uint8_t segId   = getu8();
            uint8_t info    = getu8();
            uint16_t offset = getu16();
            getu8();
            addField("%s", getName());
            addField("%s:%04X", getIndexName(ISEG, segId), offset);
            if (defTyp < 2)
                symInfo51(info);
            else
                segInfo51(info);
        } else {
            uint8_t segId   = getu8();
            uint16_t offset = getu16();
            addField("%s:%04X", getIndexName(ISEG, segId), offset);
            addField("#%d", getu16()); // line number
        }
    }
}

void omf51_16(int type) {
    static field_t const header[] = {
        { "Name", WNAME }, { "Segment:Offset", MAXNAME + 5 }, { "SymInfo", WINFO51 }, { NULL }
    };

    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        uint8_t segId   = getu8();
        uint8_t symInfo = getu8();
        uint16_t offset = getu16();
        getu8();

        addField("%s", getName()); // name
        addField("%s:%04X", getIndexName(ISEG, segId), offset);
        symInfo51(symInfo);
    }
}

void omf51_18(int type) {
    static field_t const header[] = {
        { "Id", 4 }, { "Name", WNAME }, { "SymInfo", WINFO51 }, { NULL }
    };

    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        /*  uint8_t idBlk = */ getu8(); // always 2
        uint8_t extId   = getu8();
        uint8_t symInfo = getu8();
        getu8();
        char const *extName = getName();
        if (malformed)
            return;
        if (extId != extIndex) {
            Log("Unexpected External %d - expected %d", extId, extIndex);
            extIndex = extId;
        }
        setIndex(IEXT, extIndex++, extName);

        addField("@%d", extId);
        addField("%s", extName);
        symInfo51(symInfo);
    }
}