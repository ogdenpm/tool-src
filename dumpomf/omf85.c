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

void omf85_02(int type);
void omf85_04(int type);
void omf85_06(int type);
void omf85_08(int type);
void omfEOF(int type);
void omf85_10(int type);
void omf85_12_16(int type);
void omf85_18(int type);
void omf85_20(int type);
void omf85_22_24(int type);
void omfLIBLOC(int type);
void omfLIBNAM(int type);
void omfLIBDIC(int type);
void omfLIBHDR(int type);
void omf85_2E(int type);

void segId96(uint8_t segId);

decodeSpec_t omf85Decode[] = {
    /* 00 */ { "INVALID", invalidRecord, NULL },
    /* 02 */ { "MODHDR", omf85_02, NULL },
    /* 04 */ { "MODEND", omf85_04, NULL },
    /* 06 */ { "CONTENT", omf85_06, NULL },
    /* 08 */ { "LINNUM", omf85_08, NULL },
    /* 0A */ { NULL, invalidRecord, NULL },
    /* 0C */ { NULL, invalidRecord, NULL },
    /* 0E */ { "EOF", omfEOF, NULL },
    /* 10 */ { "ANCESTOR", omf85_10, NULL },
    /* 12 */ { "LOCALS", omf85_12_16, NULL },
    /* 14 */ { NULL, invalidRecord, NULL },
    /* 16 */ { "PUBLICS", omf85_12_16, NULL },
    /* 18 */ { "EXTDEF", omf85_18, NULL },
    /* 1A */ { NULL, invalidRecord, NULL },
    /* 1C */ { NULL, invalidRecord, NULL },
    /* 1E */ { NULL, invalidRecord, NULL },
    /* 20 */ { "EXTFIX", omf85_20, NULL },
    /* 22 */ { "FIXUP", omf85_22_24, NULL },
    /* 24 */ { "SEGFIX", omf85_22_24, NULL },
    /* 26 */ { "LIBLOC", omfLIBLOC, NULL },
    /* 28 */ { "LIBNAM", omfLIBNAM, NULL },
    /* 2A */ { "LIBDIC", omfLIBDIC, NULL },
    /* 2C */ { "LIBHDR", omfLIBHDR, NULL },
    /* 2E */ { "COMNAM", omf85_2E, NULL },

};

char const *nameAlign85[4] = { "Absolute", "InPage", "Page", "Byte" };
char const *nameFixup85[4] = { "Unknown", "Low", "High", "Both" };
uint16_t dicIdx;
uint16_t namIdx;
uint16_t locIdx;

void init85() {
    resetNames();

    setIndex(ISEG, 0, "ABS");
    setIndex(ISEG, 1, "CODE");
    setIndex(ISEG, 2, "DATA");
    setIndex(ISEG, 3, "STACK");
    setIndex(ISEG, 4, "MEMORY");
    setIndex(ISEG, 5, "RESERVED");
    setIndex(ISEG, 255, "COMMON");
    extIndex = 0;
}

void loadCommonNames() {
    if (peekNextRecType() != 0x2e) /* don't have common names so all done*/
        return;
    long where = start;
    while (getrec() >= 0 && recType == 0x2e) {
        while (!atEndRec()) {
            uint8_t segId    = getu8();
            char const *name = getName();
            if (malformed)
                break;
            setIndex(ISEG, segId, name);
        }
        if (malformed)
            break;
    }
    fseek(src, where, SEEK_SET);
    loadRec();
}

void omf85_02(int type) {
    char const *trnName[]         = { "UKN80", "PL/M-80", "FORT80" };
    static field_t const header[] = { { "Segment", WNAME }, { "Size", 4 }, { "Align", 8 }, { NULL } };

    loadCommonNames();    // peek ahead for common names
    add("%s", getName()); // module name
    uint8_t trn = getu8();
    add(" - %s", trn < 3 ? trnName[trn] : "Bad TRN");

    uint8_t ver = getu8();
    if (ver)
        add(" v%d.%d", ver / 16, ver % 16);
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%s", getIndexName(ISEG, getu8())); // Seg name
        addField("%04X", getu16());                  // size
        addField("%s", nameAlign85[getu8() & 3]);    // align
    }
}

void omf85_04(int type) {
    uint8_t modTyp  = getu8();
    uint8_t segId   = getu8();
    uint16_t offset = getu16();
    if (modTyp)
        add("Entry %s:%04X", getIndexName(ISEG, segId), offset);
    if (!atEndRec()) {
        add(" Optional Info:");
        hexDump(revertRecPos(), false);
    }
    init85();
}

void omf85_06(int type) {
    add("Seg[%s] ", getIndexName(ISEG, getu8()));
    /* dump the content, adding offset info if fixup follows */
    hexDump(getu16(), false);
}

void omf85_08(int type) { // LINNUM
    add("Seg[%s] ", getIndexName(ISEG, getu8()));
    omfLINNUM();
}

void omfLINNUM() {
    static field_t const header[] = { { "Offset" }, { "Line", 5}, { NULL } };

    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%04X", getu16()); // location
        addField("#%u", getu16());  // stmt number
    }
}

void omfEOF(int type) {
}

void omf85_10(int type) {
    add("%s", getName());
}

void omf85_12_16(int type) {
    static field_t const header[] = { { "Offset" }, { "Name", WNAME }, { NULL } };

    add("Seg[%s]", getIndexName(ISEG, getu8()));
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%04X", getu16()); // offset
        addField("%s", getName());  // name
        getu8(); // trailing 0
    }
}

void omf85_18(int type) {
    static field_t const header[] = { {"Index" }, { "Name", WNAME }, { NULL } };
    int cols                       = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        char const *name = getName();
        setIndex(IEXT, extIndex, name);
        addField("@%d", extIndex++);
        addField("%s", name);
        getu8();        // junk 0
    }
}

void omf85_20(int type) {
    static field_t const header[] = { { "Offset" }, { "Name", WNAME }, { NULL } };

    uint8_t hilo                   = getu8();

    add("Fixup: %s", nameFixup85[hilo & 3]);
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        uint16_t eIdx   = getu16();
        uint16_t offset = getu16();
        addField("%04X", offset);
        addField("%s", getIndexName(IEXT, eIdx));
    }
}
void omf85_22_24(int type) {
    static field_t const header[] = { { "Offset" }, { NULL } };

    uint8_t segId                  = type == 0x24 ? getu8() : 0;
    uint8_t hilo                   = getu8();

    if (type == 0x24)
        add("Seg[%s] ", getIndexName(ISEG, segId));
    add("Fixup: %s", nameFixup85[hilo & 3]);
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%04X", getu16()); // offset
    }
}

void omfLIBLOC(int type) {
    static field_t const header[] = { { "Module"}, { "Block:Byte" }, { NULL } };
    int cols = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        uint16_t block = getu16();
        uint16_t byte  = getu16();
        addField("$%u", ++locIdx);
        addField("%04X:%02X", block, byte);
    }
}

void omfLIBNAM(int type) {
    static field_t const header[] = { { "Mod", 5 }, { "Name", WNAME }, { NULL } };
    int cols                       = addReptHeader(header);

    while (!atEndRec()) {
        startCol(cols);
        addField("$%u", ++namIdx);
        addField("%s", getName());
    }
}
void omfLIBDIC(int type) {
    static field_t const header[] = { { "Mod", 5 }, { "Name", WNAME }, { NULL } };
    int cols                        = addReptHeader(header);
    bool start = true;

    while (!atEndRec()) {
        markRecPos();
        char const *name = getName();
        if (malformed)
            return;
        if (start) {
            startCol(cols);
            addField("$%u", ++dicIdx);
            addField("%s", *name ? name : "*None*");
            start = !*name;
        } else if (*name) {
            startCol(cols);
            addField("");
            addField(name);
        } else
            start = true;
    }
    malformed = !start;
}
void omfLIBHDR(int type) {
    uint16_t mcount = getu16();
    uint16_t block  = getu16();
    uint16_t byte   = getu16();
    add("+%u Modules Dictionary at %04X:%02X", mcount, block, byte);
    locIdx = namIdx = dicIdx = 0;
}

void omf85_2E(int type) {
    static field_t const header[] = { { "Id", 4 }, { "Name", WNAME }, {  NULL } };
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("@%u", getu8());
        addField("%s", getName());
    }
}