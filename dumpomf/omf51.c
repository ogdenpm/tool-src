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
#include <time.h>

void omf51_02(int type);
void omf51_04(int type);
void omf51_06(int type);
void omf51_08(int type);
void omf51_0E(int type);
void omf51_10(int type);
void omf51_12(int type);
void omf51_16(int type);

void omf51_18(int type);
void omf51k_20(int type);
void omf51k_24(int type);
void omf51k_70(int type);
void omf51k_72(int type);

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
    /* 20 */ { "TYPDEF", omf51k_20, NULL },
    /* 22 */ { "KDBGITEM", omf51_12, NULL },
    /* 24 */ { "SOURCE", omf51k_24, NULL },
    /* 26 */ { "LIBLOC", omfLIBLOC, NULL },
    /* 28 */ { "LIBNAM", omfLIBNAM, NULL },
    /* 2A */ { "LIBDIC", omfLIBDIC, NULL },
    /* 2C */ { "LIBHDR", omfLIBHDR, NULL },
    /* 2E */ { NULL, invalidRecord, NULL },
    /* 30 */ { NULL, invalidRecord, NULL },
    /* 32 */ { NULL, invalidRecord, NULL },
    /* 34 */ { NULL, invalidRecord, NULL },
    /* 36 */ { NULL, invalidRecord, NULL },
    /* 38 */ { NULL, invalidRecord, NULL },
    /* 3A */ { NULL, invalidRecord, NULL },
    /* 3C */ { NULL, invalidRecord, NULL },
    /* 3E */ { NULL, invalidRecord, NULL },
    /* 40 */ { NULL, invalidRecord, NULL },
    /* 42 */ { NULL, invalidRecord, NULL },
    /* 44 */ { NULL, invalidRecord, NULL },
    /* 46 */ { NULL, invalidRecord, NULL },
    /* 48 */ { NULL, invalidRecord, NULL },
    /* 4A */ { NULL, invalidRecord, NULL },
    /* 4C */ { NULL, invalidRecord, NULL },
    /* 4E */ { NULL, invalidRecord, NULL },
    /* 50 */ { NULL, invalidRecord, NULL },
    /* 52 */ { NULL, invalidRecord, NULL },
    /* 54 */ { NULL, invalidRecord, NULL },
    /* 56 */ { NULL, invalidRecord, NULL },
    /* 58 */ { NULL, invalidRecord, NULL },
    /* 5A */ { NULL, invalidRecord, NULL },
    /* 5C */ { NULL, invalidRecord, NULL },
    /* 5E */ { NULL, invalidRecord, NULL },
    /* 60 */ { NULL, invalidRecord, NULL },
    /* 62 */ { NULL, invalidRecord, NULL },
    /* 64 */ { NULL, invalidRecord, NULL },
    /* 66 */ { NULL, invalidRecord, NULL },
    /* 68 */ { NULL, invalidRecord, NULL },
    /* 6A */ { NULL, invalidRecord, NULL },
    /* 6C */ { NULL, invalidRecord, NULL },
    /* 6E */ { NULL, invalidRecord, NULL },
    /* 70 */ { "DEPLST", omf51k_70, NULL },
    /* 72 */ { "REGMSK", omf51k_72, NULL },

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
        Log("Non-zero bank(%d) for non-overlayable segment", bank);
}

char const *getTiStr(uint16_t ti) {
    static char const *types[] = { "None",     "sbit",    "char",     "uint8_t", "int16_t",
                                   "uint16_t", "int32_t", "uint32_t", "resv8",   "resv09",
                                   "resv10",   "void",    "float" };
    static char userType[16];
    if (ti <= 12)
        return types[ti];
    else if (ti < 32)
        sprintf(userType, "#%d", ti);
    else
        sprintf(userType, "@%d", ti);
    return userType;
}

void symInfo51(uint8_t n, bool keil) { // Max width = 6 + 5 + 4 + 7 = 22
    char const *usage[] = { "CODE", "XDATA", "DATA", "IDATA", "BIT", "NUMBER", "INFO6", "INFO7" };

    addField("%s", usage[n & 7]);
    if (n & 0x40) // variable
        add(" VAR");
    else {
        if (!keil)
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
    hexDump(type == 6 ? getu16() : getu24(), peekNextRecType() == 8);
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

    static field_t const header[] = { { "Id", 4 },      { "Name", WNAME },      { "Base:Size", 12 },
                                      { "RelTyp", 14 }, { "SegInfo", WINFO51 }, { NULL } };
    int cols                      = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        uint8_t segId = getu8();

        if (segId && segId != ++segIndex) {
            Log("Unexpected Segment Definition %d - expected %d", segId, segIndex);
            segIndex = segId;
        }

        uint8_t segInfo     = getu8();
        uint8_t relTyp      = getu8();
        uint8_t pad         = getu8();

        uint32_t segBase    = type == 0xe ? getu16() : getu24();
        uint16_t segSize    = getu16();
        char const *segName = getName();
        if (segId)
            setIndex(ISEG, segId, segName);
        addField("@%d", segId);
        addField("%s", *segName ? segName : "*Unnamed*");
        if (pad != (type & 1))
            add(" pad=%02X");
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

    static field_t const headerk[]   = { { "Name", WNAME },
                                         { "Segment:Offset", MAXNAME + 5 },
                                         { "Type", 13 },
                                         { "Info", WINFO51 },

                                         { NULL } };

    static field_t const linHeader[] = { { "Segment:Offset", MAXNAME + 5 },
                                         { "Line", 5 },
                                         { NULL } };

    uint8_t defTyp                   = getu8();
    if ((malformed = malformed || (defTyp > 3)))
        return;
    add("%s", types[defTyp]);

    int cols = addReptHeader(defTyp < 3 ? type == 0x12 ? header : headerk : linHeader);

    while (!atEndRec()) {
        startCol(cols);
        if (defTyp < 3) {
            uint8_t segId = getu8();
            uint8_t info  = getu8();
            if (type & 1) // fits but may be wrong
                getu8();

            uint32_t offset = getu16();
            uint8_t ti      = getu8();

            addField("%s", getName());
            if (type == 0x22 && segId == 0 && info == 2 && offset && (offset < 8 || offset == 0x82)) {
                addField("Reg: ");
                if (offset == 1)
                    add("R1-R3");
                else if (offset == 0x82)
                    add("DPTR");
                else if (ti < 4)    // 1 byte value
                    add("R%d", offset);
                else if (ti < 13)   // <= float. if < 6 then 2 byte value else 4 byte value
                    add("R%d-R%d", offset, ti < 6 ? offset + 1 : offset + 3);
                else
                    add("%d+", offset);

            } else
                addField("%s:%04X", getIndexName(ISEG, segId), offset);
            if (defTyp < 2) {
                if (type != 0x12)
                    addField("%s", getTiStr(ti));
                symInfo51(info, type != 0x12);
            } else
                segInfo51(info);
        } else {
            uint8_t segId   = getu8();
            if (type & 1)   // fits but may be wrong
                getu8();
            uint32_t offset = getu16();
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
        uint32_t offset = type & 1 ? getu24() : getu16();
        getu8();

        addField("%s", getName()); // name
        addField("%s:%04X", getIndexName(ISEG, segId), offset);
        symInfo51(symInfo, type & 1);
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
        uint16_t extId  = type & 1 ? getu16() : getu8();
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
        symInfo51(symInfo, type & 1);
    }
}

void rawBytes(uint16_t cnt) {
    while (cnt-- && !malformed)
        add(" %02X", getu8());
}

void omf51k_20(int type) {
    static char *namespaces[] = { "", " idata", " xdata", " [3]", " data", " code" };
    static char *scopes[]     = { "[0]", "Global", "Specific", "Stack" };
    static uint8_t sizes[]    = { 3, 1, 2, 0, 1, 2 };
    static char *reg[]        = { "R3", "R5,R3", "R7,R5,R3", "R1/R2/R3" };
    static char *regPair[]    = { "DPTR", "R2/R3",  "R4/R5,R2/R2", "R6/R7,R4/R5,R2/R3" };

    uint16_t typeIndex        = 32;
    while (!atEndRec() && !malformed) {
        startCol(1);
        uint8_t subtype = getu8();
        uint16_t cnt;
        uint16_t val;
        uint16_t offset;
        uint16_t index;
        const char *name;

        add("@%-2d ", typeIndex++);
        switch (subtype) {
        case 0x20:
            cnt = getu16();
            add("Components:");
            while (cnt-- != 0) {
                offset = getu16();
                index  = getIndex();
                name   = getName();
                if (malformed)
                    return;
                startCol(1);
                add("    %04X %-9s %s", offset, getTiStr(index), name);
            }
            break;
        case 0x22:
            cnt = getu8(); // dimensions
            add("Array: ");
            while (cnt-- && !malformed)
                add("[%d]", getu16());
            add(" %s", getTiStr(getIndex()));
            break;
        case 0x23:
            add("Procedure: %s", getTiStr(getIndex()));
            add("(%s)", getTiStr(getIndex()));

            break;
        case 0x24:
            val  = getu8();
            name = getName();
            if (!malformed)
                add("Tag: %d %s", val, name);
            break;
        case 0x25:
            add("Struct/Union: size=%d", getu16());
            add(" components=%s", getTiStr(getIndex()));
            add(" tag=%s", getTiStr(getIndex()));
            break;
        case 0x26: // bitfield
            val = getu8();
            if (val <= 1)
                add("Bitfield: %s offset=%d", val == 0 ? "8-Bit " : "16-Bit", getu8());
            else
                add("Bitfield: [%02X] offset=%d", val, getu8());
            add(" width=%d", getu8());
            break;

        case 0x28:
            {
                uint8_t scope     = getu8();
                uint8_t size      = getu8();
                uint8_t namespc   = getu8();
                uint8_t ptrtype   = getu8();
                uint8_t regAlloc  = getu8();
                val               = getu16();
                char const *tistr = getTiStr(getIndex());

                if (malformed)
                    return;
                if (ptrtype == 1)
                    add("Data Pointer:");
                else if (ptrtype == 2)
                    add("Function Pointer:");
                else
                    add("Pointer type %d", ptrtype);

                if (regAlloc) {
                    if ((regAlloc & 1) && regAlloc <= 9)
                        add(" Reg:%s", reg[(regAlloc - 3) / 2]);
                    else if (!(regAlloc & 1) && 0x10 <= regAlloc && regAlloc <= 0x16)
                        add(" Reg:%s", regPair[(regAlloc - 0x10) / 2]);
                    else
                        add(" Reg:%02X", regAlloc);
                }

                if (namespc <= 5) {
                    if ((namespc == 0 && scope != 1) || (namespc > 0 && scope != 2)) {
                        if (scope <= 3)
                            add(" %s", scopes[scope]);
                        else
                            add(" scope_%d", scope);
                    }
                    if (size != sizes[namespc])
                        add(" size=%d", size);
                    add(" %s%s *", tistr, namespaces[namespc]);
                } else {
                    if (scope <= 3)
                        add(" %s", scopes[scope]);
                    else
                        add(" scope_%d", scope);
                    add(" size=%d", size);
                    add(" %s unknown_%d *", tistr, namespc);
                }

                if (val)
                    add(" Reserved=[%02x %02x", val % 256, val / 256);
            }
            break;
        default:
            Log("Unknown TYPDEF subtype %d", subtype);
            return;
        }
    }
}
void omf51k_24(int type) {
    uint32_t zeros   = getu24();
    char const *name = getName();
    if (!malformed) {
        if (zeros)
            add(" %s", hexStr(zeros));
        add(" %s", name);
    }
}

void addTime(time_t *timestamp) {
    struct tm *tm = localtime(timestamp);
    add("[%04d-%02d-%02d %02d:%02d:%02d]", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
        tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void omf51k_70(int type) {
    static char *types[] = { "Output", "Input", "Include", "Script", "Object" };
    while (!atEndRec() && !malformed) {
        uint8_t type = getu8();
        startCol(1);
        if (type == 0xff) {
            char const *name = getName();
            add("Invoke: %s", name);

        } else if (type <= 4) {
            if (getu8() != 0) {
                Log("Non zero mark");
                return;
            }
            time_t timestamp = getu32();
            char const *name = getName();
            if (malformed)
                return;
            add("  %s:%*s", types[type], 8 - strlen(types[type]), "");
            addTime(&timestamp);
            add(" %s", name);
        } else {
            Log("Unknown DEPLST file type %d", type);
            return;
        }
    }
}

void omf51k_72(int type) {
    while (!atEndRec() && !malformed) {
        startCol(1);
        uint8_t e8       = getu8();
        uint16_t r16     = getu16();
        char const *name = getName();
        if (malformed)
            return;
        add("%s mask=%04X %s", e8 == 0 ? "Public  " : "External", r16, name);
    }
}
