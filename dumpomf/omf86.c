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

#include "omf86.h"
#include "omf.h"

#define IASCIICOL (MAXPOS - 51)
void omf86_6E(int type);
void omf86_70(int type);
void omf86_72(int type);
void omf86_74(int type);
void omf86_76(int type);
void omf86_78(int type);
void omf86_7A(int type);
void omf86_7C(int type);
void omf86_7E(int type);
void omf86_80(int type);
void omf86_82(int type);
void omf86_84(int type);
void omf86_86(int type);
void omf86_88(int type);
void omf86_8A(int type);
void omf86Ext(int type);
void omf86_8E(int type);
void omf86PubLoc(int type);
void omf86Names(int type);
void omf86_94(int type);
void omf86_98(int type);
void omf86_9A(int type);
void omf86_9C(int type);
void omf86_A0(int type);
void omf86_A2(int type);
void omf86Com(int type);
void omf86_B2(int type);
void omf86_BC(int type);
void omf86_C2(int type);
void omf86_C4(int type);
void omf86_C6(int type);
void omf86_C8(int type);
void omf86_CC(int type);
void omf86_CE(int type);

decodeSpec_t omf86Decode[] = {
    /*XX*/ { "INVALID", invalidRecord, NULL },
    /*6E*/ { "RHEADR", omf86_6E, NULL },
    /*70*/ { "REGINT", omf86_70, NULL },
    /*72*/ { "REDATA", omf86_72, NULL },
    /*74*/ { "RIDATA", omf86_74, NULL },
    /*76*/ { "OVLDEF", omf86_76, NULL },
    /*78*/ { "ENDREC", omf86_78, NULL },
    /*7A*/ { "BLKDEF", omf86_7A, NULL },
    /*7C*/ { "BLKEND", omf86_7C, NULL },
    /*7E*/ { "DEBSYM", omf86_7E, NULL },
    /*80*/ { "THEADR", omf86_80, NULL },
    /*82*/ { "LHEADR", omf86_82, NULL },
    /*84*/ { "PEDATA", omf86_84, NULL }, // not MS
    /*86*/ { "PIDATA", omf86_86, NULL }, // not MS
    /*88*/ { "COMENT", omf86_88, NULL },
    /*8A*/ { "MODEND", omf86_8A, NULL },
    /*8C*/ { "EXTDEF", omf86Ext, NULL },
    /*8E*/ { "TYPDEF", omf86_8E, NULL },
    /*90*/ { "PUBDEF", omf86PubLoc, NULL },
    /*92*/ { "LOCSYM", omf86PubLoc, NULL },
    /*94*/ { "LINNUM", omf86_94, NULL },
    /*96*/ { "LNAMES", omf86Names, NULL },
    /*98*/ { "SEGDEF", omf86_98, NULL },
    /*9A*/ { "GRPDEF", omf86_9A, NULL },
    /*9C*/ { "FIXUPP", omf86_9C, NULL },
    /*9E*/ { NULL, invalidRecord, NULL },
    /*A0*/ { "LEDATA", omf86_A0, NULL },
    /*A2*/ { "LIDATA", omf86_A2, NULL },
    /*A4*/ { "LIBHED", omfLIBHDR, NULL },    // not MS
    /*A6*/ { "LIBNAM", omfLIBNAM, NULL },    // not MS
    /*A8*/ { "LIBLOC", omfLIBLOC, NULL },    // not MS
    /*AA*/ { "LIBDIC", omfLIBDIC, NULL },    // not MS
    /*AC*/ { NULL, invalidRecord, NULL },    // not INTEL
    /*AE*/ { NULL, invalidRecord, NULL },    // not INTEL
    /*B0*/ { "COMDEF", omf86Com, NULL },     // not INTEL
    /*B2*/ { "BAKPAT", omf86_B2, NULL },     // not INTEL
    /*B4*/ { "LEXTDEF", omf86Ext, NULL },    // not INTEL
    /*B6*/ { "LPUBDEF", omf86PubLoc, NULL }, // not INTEL
    /*B8*/ { "LCOMDEF", omf86Com, NULL },    // not INTEL
    /*BA*/ { NULL, invalidRecord, NULL },    // not INTEL
    /*BC*/ { "CEXTDEF", omf86_BC, NULL },    // not INTEL
    /*BE*/ { NULL, invalidRecord, NULL },    // not INTEL
    /*C0*/ { NULL, invalidRecord, NULL },    // not INTEL
    /*C2*/ { "COMDAT", omf86_C2, NULL },     // not INTEL
    /*C4*/ { "LINSYM", omf86_C4, NULL },     // not INTEL
    /*C6*/ { "ALIAS", omf86_C6, NULL },      // not INTEL
    /*C8*/ { "NBKPAT", omf86_C8, NULL },     // not INTEL
    /*CA*/ { "LLNAMES", omf86Names, NULL },  // not INTEL
    /*CC*/ { "VERNUM", omf86_CC, NULL },     // not INTEL
    /*CE*/ { "VENDEXT", omf86_CE, NULL },    // not INTEL

};

bool isValidRec(int spec) {
    if ((recType & 1) == 0)
        return true;
//    if (spec == OMF51K)
//        return strchr("\x7\x9\xf\x97\x19", recType);
    return strchr("\x8b\x91\x95\x99\xa1\xa3\xb3\xb5\xb7\xc3\xc5\xc9", recType);
}

int nameIndex;
int ovlIndex;
int grpIndex;
int blkIndex;
extern int typeIndex;
uint16_t iDataBlock;

char const *enumModDat86[5] = { "ModDat", "ABSOLUTE", "RELOCATABLE", "PIC", "LTL" };

void init86() {
    resetNames();
    setIndex(ISEG, 0, "Unnamed Abs");
    for (int i = INAME; i <= INDEXTABLES; i++)
        setIndex(i, 0, "");
    nameIndex = 1;
    segIndex  = 1;
    extIndex  = 1;
    grpIndex  = 1;
    blkIndex  = 1;
    typeIndex = 1;
}

void base86() {
    uint16_t grpIdx = getIndex();
    uint16_t segIdx = getIndex();
    if (grpIdx)
        add("Grp[%s].Seg[%s]", getIndexName(IGROUP, grpIdx), getIndexName(ISEG, segIdx));
    else if (segIdx)
        add("Seg[%s]", getIndexName(ISEG, segIdx));
    else
        add("Frame: %04X", getu16());
}

static void doFrame(uint8_t frame) {
    switch (frame) {
    case F_SEG:
        add("SI[%s]", getIndexName(ISEG, getIndex()));
        break;
    case F_GRP:
        add("GI[%s]", getIndexName(IGROUP, getIndex()));
        break;
    case F_EXT:
        add("EI[%s]", getIndexName(IEXT, getIndex()));
        break;
    case F_ABS:
        add("%04X", getu16());
        break;
    case F_LOC:
        add("LOCATION");
        break;
    case F_TARG:
        add("TARGET  ");
        break;
    case F_NONE:
        add("NONE    ");
        break;
    default:
        add("Unknown frame(%d)", frame);
        break;
    }
}

static void doTarget(uint8_t target) {
    switch (target & 0x03) {
    case T_SEGWD:
        add("Seg[%s]", getIndexName(ISEG, getIndex()));
        break;
    case T_GRPWD:
        add("Grp[%s]", getIndexName(IGROUP, getIndex()));
        break;
    case T_EXTWD:
        add("Ext[%s]", getIndexName(IEXT, getIndex()));
        break;
    case T_ABSWD:
        add("Frame %04X", getu16());
        break;
    }
}

void fixupDat() {
    uint8_t typ       = getu8();
    uint8_t frame     = (typ >> 4) & 0x07;

    uint16_t startCol = getCol();

    if (typ & FIXDAT_FTHREAD)
        add("THREAD(%d)", frame & 3);
    else
        doFrame(frame);

    addAt(startCol + 17, "");
    if (typ & FIXDAT_TTHREAD)
        add("THREAD(%d)", typ & 3);
    else
        doTarget(typ & 3);

    if (!(typ & 0x04))
        add(",%04X", is32bit ? getu32() : getu16());
}

static void explicitFixup(uint8_t typ) {
    static char const *locations[] = { "LoByte",      "Offset16",   "Base",        "Pointer32",
                                       "HiByte",      "LrOffset16", "Pointer48",   "Undefined7",
                                       "Undefined8",  "Offset32",   "Undefined10", "Pointer48",
                                       "Undefined12", "rOffset32",  "Undefined14", "Undefined15" };
    uint8_t loc;
    uint16_t offset = ((typ & 0x03) * 256) + getu8();

    add("%03X>", offset);
    addAt(6, typ & FIXDAT_MBIT ? "Self" : "Seg");
    loc = ((typ >> 2) & 0x0f);
    if (omfFlavour == ANY && loc >= LOC_MS_LINK_OFFSET)
        omfFlavour = MS;
    else if (omfFlavour == PHARLAP && loc == LOC_MS_LINK_OFFSET)
        loc = LOC_MS_OFFSET_32;

    addAt(19, "%s", locations[loc]);
    addAt(31, "");
    fixupDat();
}

void threadFixup(uint8_t typ) {
    uint8_t thred  = typ & 3;
    uint8_t method = (typ >> 2) & 7;

    if ((typ & 0x40)) {
        add("Thread FRAME(%d)  =", thred);
        doFrame(method);
    } else {
        add("Thread TARGET(%d) =", thred);
        doTarget(method);
    }
}

uint32_t blockContent86(uint32_t addr) {
    uint16_t indent   = getCol() + 1;
    uint8_t len       = getu8();
    uint8_t offset    = 0;
    uint8_t addrWidth = addr > 0x10000 ? 6 : 4;

    char ascii[17];
    uint8_t i = 0;
    while (len-- && !malformed) {
        if (i == 16 || (i > 8 && getCol() > ASCIICOL + addrWidth - 3)) {
            ascii[i] = '\0';
            addAt(ASCIICOL, "|%s|", ascii);
            i = 0;
            startCol(1);
            add("%04X ", addr + offset);
        }
        if (i == 0)
            addAt(indent, "%03X>", getRecPos() - iDataBlock);
        uint8_t c = getu8();
        offset++;
        add(" %02X", c);
        ascii[i++] = ' ' <= c && c <= '~' ? c : '.';
    }
    if (i) {
        ascii[i] = '\0';
        addAt(IASCIICOL, "|%s|", ascii);
    }
    return offset;
}

uint32_t block86(uint32_t addr, uint16_t blkCnt) {

    uint16_t indent   = getCol();
    uint32_t delta    = 0;
    for (uint16_t i = 1; !malformed && i <= blkCnt; i++) {
        if (getCol() == 0)
            add("%04X", addr + delta);
        if (indent) {
            if (indent > getCol())
                addAt(indent, "");
            add(" %d.", i);
        }
        uint32_t repeatCnt = is32bit ? getu32() : getu16();
        if (repeatCnt > 1)
            add(" %u x", repeatCnt);

        uint16_t blockCnt = getu16();
        if (blockCnt)
            delta += repeatCnt * block86(addr + delta, blockCnt);
        else
            delta += repeatCnt * blockContent86(addr + delta);

        startCol(1);
    }
    return delta;
}

void iData86(uint32_t addr) {
    iDataBlock = getRecPos();
    while (!atEndRec()) {
        startCol(1);
        addr += block86(addr, 1);
    }
    if (!malformed) {
        displayLine();
        add("%04X", addr);
    }
}

void memoryModel() {
    static char const *mapstr[] = { "8086",  "80186",  "80286",   "80386", "Optimised",
                                    "Small", "Medium", "Compact", "Large", "Huge",
                                    "68000", "68010",  "68020",   "68030" };
    static char *map            = "0123OsmclhABCD";
    uint8_t c;

    startCol(1);
    while (!atEndRec()) {
        char *s = strchr(map, c = getu8());
        if (s)
            add("%s ", mapstr[s - map]);
        else
            add("Unknown item %02X ", c);
    }
}

void commentStr() {
    startCol(1);
    while (!atEndRec()) {
        uint8_t c = getu8();
        if (' ' <= c && c <= '~')
            add("%c", c);
        else
            add("\\%02x", c);
    }
}

void commentClassA0() {
    static ofield_t const header01[] = { { 0, "Internal Name" },
                                         { WNAME86, "Module Name" },
                                         { WNAME86 + WNAME86, "Entry Ident" },
                                         { 0, NULL } };
    static ofield_t const header02[] = { { 0, "Exported Name" },
                                         { WNAME86, "Internal Name" },
                                         { WNAME86 + WNAME86, "Ordinal" },
                                         { WNAME86 + WNAME86 + 8, "Attributes" },
                                         { 0, NULL } };
    uint8_t subtype                  = getu8();
    uint8_t flag;
    char const *name1, *name2;

    add("Subtype(%02X) ", subtype);
    switch (subtype) {
    case 1:
        add("IMPDEF: Import Definition Record");
        oaddHeader(1, header01);
        startCol(1);
        flag = getu8();
        add("%s", name1 = getName());
        addAt(header01[1].tabStop, getName());
        if (flag == 0) {
            name2 = getName();
            addAt(header01[2].tabStop, "%s", *name2 ? name2 : name1);
        } else
            addAt(header01[2].tabStop, "#%d", getu16());
        break;
    case 2:
        add("EXPDEF: Export Definition Record");
        oaddHeader(1, header02);
        startCol(1);
        flag = getu8();
        add("%s", getName());
        addAt(header02[1].tabStop, getName());
        if (flag & 0x80)
            addAt(header02[2].tabStop, "#%d", getu16());
        if (flag & 0x7f)
            addAt(header02[3].tabStop, "");
        if (flag & 0x40)
            add("Resident Name ");
        if (flag & 0x20)
            add("No Data ");
        if (flag & 0x1f)
            add("Parm Count: #%d", flag & 0x1f);
        break;
    case 3:
        add("INCDEF: Incremental Compilation Record");
        add("  ExtDef Delta: #%d", geti16());
        add("  LinNum Delta: #%d", geti16());
        while (!atEndRec())
            getu8();

        break;
    case 4:
        add("Protected Memory Library");
        break;
    case 5:
        add("LNKDIR: Microsoft C++ Directives Record");
        flag = getu8();
        if (flag & 1)
            add("  New .EXE ");
        if (flag & 2)
            add("  No $PUBLICS ");
        if (flag & 4)
            add("  Run MPC ");
        add("  PseudoCode v%02X ", getu8());
        add("  CodeView v%02X", getu8());
        break;
    case 6:
        add("Big-endian");
        break;
    case 7:
        add("PRECOMP");
        break;
    default:
        add("Reserved");
        break;
    }
}

void omf86_6E(int type) { // RHEADR
    char const *modType[]   = { "ABS", "REL", "PIC", "LTL" };
    ofield_t const header[] = { { 0, "Segments" },     { 10, "Groups" },       { 18, "Overlays" },
                                { 33, "Static Size" }, { 52, "Dynamic Size" }, { 0, NULL } };

    add("%s", getName());
    add(" - %s", modType[getu8() & 3]);
    uint16_t nSeg   = getu16();
    uint16_t nGrp   = getu16();
    uint16_t nOvl   = getu16();
    uint32_t offset = getu32();
    oaddHeader(1, header);
    startCol(1);
    add("#%d", nSeg);
    addAt(header[1].tabStop, "#%d", nGrp);
    addAt(header[2].tabStop, "#%d", nOvl);
    if (nOvl)
        add(" @%04X:%02X", offset / 128, offset % 128);

    uint32_t size       = getu32();
    uint32_t maxSize    = getu32();
    uint32_t dynamic    = getu32();
    uint32_t maxDynamic = getu32();
    addAt(header[3].tabStop, "%X", size);
    if (size != maxSize)
        add("-%X", maxSize);
    addAt(header[4].tabStop, "%X", dynamic);
    if (dynamic != maxDynamic)
        add("-%X", maxDynamic);
}

void omf86_70(int type) { // REGINT
    char const *regs[] = { "CS,IP", "SS,SP", "DS", "ES" };

    while (!atEndRec()) {
        startCol(3);
        uint8_t regTyp = getu8();
        add(regs[regTyp >> 6]);
        addAt(6, "");
        if (regTyp & 1)
            fixupDat();
        else {
            base86();
            if ((regTyp >> 6) <= 1)
                add(",%04X", getu16());
        }
    }
}
void omf86_72(int type) { // REDATA
    base86();
    hexDump(getu16(), strchr("\x9c\x9d", peekNextRecType()) != NULL);
}
void omf86_74(int type) { // RIDATA
    base86();
    iData86(getu16());
}
void omf86_76(int type) { // OVLDEF
    char const *name = getName();
    uint32_t loc     = getu32();
    uint8_t sa       = getu8();
    setIndex(IOVERLAY, ovlIndex, name);
    add("%s @%04X:%02X", name, loc / 128, loc % 128);
    if (sa & 2)
        add(" Shared(%s)", getIndexName(IOVERLAY, getIndex()));
    if (sa & 2)
        add(" Adjacent(%s)", getIndexName(IOVERLAY, getIndex()));
}
void omf86_78(int type) { // ENDREC
    uint8_t endTyp = getu8();
    add("%s", endTyp == 0 ? "Overlay" : endTyp == 1 ? "Block" : "(Illegal)");
}

void omf86_7A(int type) { // BLKDEF
    static ofield_t const header[] = { { 0, "Id" },
                                       { 5, "Base" },
                                       { 45, "Name" },
                                       { 45 + WNAME86, "BlkTyp" },
                                       { 45 + WNAME86 + 11, "Offset" },
                                       { 45 + WNAME86 + 11 + 7, "Len" },
                                       { 45 + WNAME86 + 11 + 7 + 5, "RetAddr" },
                                       { 45 + WNAME86 + 11 + 7 + 5 + 9, "Type" },
                                       { 0, NULL } };
    oaddHeader(1, header);
    startCol(1);
    add("#%-3d ", blkIndex);
    base86();
    char const *name = getName();
    uint16_t offset  = getu16();
    uint16_t length  = getu16();
    uint8_t pi       = getu8();

    setIndex(IBLOCK, blkIndex++, name);
    if (!*name)
        name = "*NoName*";
    addAt(header[2].tabStop, "%s", name);
    addAt(header[3].tabStop, "");

    if (pi & 0x80)
        add("%s PROC", pi & 0x40 ? "FAR" : "NEAR");
    else
        add("DO");
    addAt(header[4].tabStop, "%04X   %04X ", offset, length);
    if (pi & 0x80)
        add("[BP+%X]", getu16());
    if (*name)
        addAt(header[7].tabStop, "#%d", getIndex());
}
void omf86_7C(int type) { // BLKEND
}

void omf86_7E(int type) { // DEBSYM
    static field_t const header[] = {
        { "Name", WNAME }, { "Offset" }, {  "Type" }, {  NULL }
    };
    uint8_t frameInfo = getu8();
    uint8_t method    = frameInfo & 7;
    if (frameInfo & 0x80)
        add("BASED POINTER%d ", frameInfo & 0x40 ? 32 : 16);
    switch (method) {
    case 0:
        base86();
        break;
    case 1:
        add("EI[%s]", getIndexName(IEXT, getIndex()));
        break;
    case 2:
        {
            uint16_t idx = getIndex();
            add("BI[%s<#%d>]", getIndexName(IBLOCK, idx), idx);
        }
        break;
    default:
        Log("Invalid Datum Method (%d)", method);
        malformed = true;
        return;
    }

    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        char const *name = getName();
        uint16_t offset  = getu16();
        uint16_t tindex  = getIndex();
        addField("%s", name);
        switch (method) {
        case 0:
            addField( "%04X", offset);
            break;
        case 1:
            addField("%+d", offset);
            break;
        case 2:
            addField("[BP%+d]", offset);
            break;
        }
        addField("@%d", tindex);
    }
}
void omf86_80(int type) { // THEADR
    add(getName());
}

void omf86_82(int type) { // LHEADR
    add(getName());
}

void omf86_84(int type) { // PEDATA
    uint16_t frame = getu16();
    add("Frame:%04X", frame);
    hexDump(getu8(), strchr("\x9c\x9d", peekNextRecType()) != NULL);
}

void omf86_86(int type) { // PIDATA
    add("Frame:%04X", getu16());
    iData86(getu8());
}
void omf86_88(int type) { // COMENT
    static ofield_t const headerA7[] = { { 0, "Segment" }, { 0, NULL } };
    static ofield_t const headerA8[] = { { 0, "Weak Ext" },
                                         { WNAME86, "Default Ext" },
                                         { 0, NULL } };

    static ofield_t const headerA9[] = { { 0, "Lazy Ext" },
                                         { WNAME86, "Default Ext" },
                                         { 0, NULL } };

    uint8_t ctype                    = getu8();
    uint8_t cclass                   = getu8();
    uint8_t c;
    if (ctype & 0x80)
        add("No Purge ");
    if (ctype & 0x40)
        add("No List ");
    add("Comment Class(%02X) ", cclass);
    switch (cclass) {
    case 0x80:
        add("Translator & Build");
        break;
    case 0:
        add("Translator");
        break;
    case 1:
        add("Intel Copyright");
    case 0x9c:
        add("MS-DOS Version");
        break;
        break;
    case 0x9d:
        add("Memory Model");
        memoryModel();
        break;
    case 0x9e:
        add("DOSSEG");
        break;
    case 0x81:
    case 0x9f:
        add("Default Library Search Name");
        break;
    case 0xa0:

        commentClassA0();
        break;
    case 0xa1:
        add("New OMF Extension");
        break;
    case 0xa2:
        c = getu8();
        if (c == 1)
            add("Link Pass Separator - Start Pass 2");
        else
            add("Link Pass Separator %d", c);
        break;
    case 0xa3:
        add("Library Module Comment Record Module: '%s'", getName());
        break;
    case 0xa4:
        add("EXESTR: Executable String Record");
        break;
    case 0xa6:
        add("INCERR: Incremental Compilation Error");
        break;
    case 0xa7:
        add("NOPAD: No Segment Padding");
        oaddHeader(3, headerA7);
        while (!atEndRec()) {
            startCol(3);
            add("%s", getIndexName(ISEG, getIndex()));
        }
        break;
    case 0xa8:
    case 0xa9:
        add(cclass == 0xa8 ? "WKEXT: Weak Extern Record" : "LZEXT: Lazy Extern Record");
        oaddHeader(2, cclass == 0xa8 ? headerA8 : headerA9);
        while (!atEndRec()) {
            startCol(2);
            add("%s", getIndexName(IEXT, getIndex()));
            addAt(headerA8[1].tabStop, "%s", getIndexName(IEXT, getIndex()));
        }
        break;
    case 0xda:
        add("Random Comment");
        break;
    case 0xdb:
        add("Pragma Comment(compiler version)");
        break;
    case 0xdc:
        add("Pragma Comment(date stamp)");
        break;
    case 0xdd:
        add("Pragma Comment(timestamp)");
        break;
    case 0xdf:
        add("Pragma Comment(user)");
        break;
    case 0xe9:
        add("Borland Dependency File");
        break;
    case 0xff:
        add("QuickC Command Line");
        break;
    default:
        add("Reserved");
        break;
    }
    if (!atEndRec())
        commentStr();
}

void omf86_8A(int type) { // MODEND
    uint8_t modTyp = getu8();
    if (modTyp & 0x80)
        add("Main Module");
    if (modTyp & 0x40) {
        add(" CS,%s = ", is32bit ? "EIP" : "IP");
        if (modTyp & 1)
            fixupDat();
        else {
            add("%04X", getu16());
            add(",%04X", getu16());
        }
    }
    init86();
}
void omf86Ext(int type) { // EXTDEF
    field_t const header[] = { { "Id", 4 }, { "Name", WNAME }, { "Type" }, { NULL } };
    int cols               = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        char const *name = getName();
        setIndex(IEXT, extIndex, name);
        addField("@%d", extIndex++);
        addField("%s", name);
        addField("@%d", getIndex()); // external type
    }
}
void omf86_8E(int type) { // TYPDEF
    add("%s", getName());
    add("#%d ", typeIndex++);
    descriptor86();
}

void omf86PubLoc(int type) { // PUBDEF / LOCSYM
    static field_t const header[] = { { "Offset" }, { "Type" }, { "Name", WNAME }, { NULL } };
    base86();
    if (atEndRec())
        return;
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        char const *name = getName();
        addField("%04X", getu16());  // offset
        addField("@%d", getIndex()); // index
        addField("%s", name);        // name
    }
}

void omf86_94(int type) { // LINNUM
    base86();
    omfLINNUM();
}

void omf86Names(int type) { // LNAMES
    field_t const header[] = { { "Id", 4 }, { "Name", WNAME }, { NULL } };
    int cols               = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        char const *name = getName();
        setIndex(INAME, nameIndex, name);
        addField("@%d", nameIndex++);
        addField("%s", *name ? name : "*blank*");
    }
}

/* segdef has a conflict between Intel and MS/later definitions
   the key conflict is the A=5 which for Intel requires the frame and offset info
   but for MS doesn't.
*/

char const *enumAlign86[]   = { "Abs", "Byte", "Word", "Para", "Page", "MAS", "LTL", "???" };
char const *enumCombine86[] = { "Private",    "Memory", "Public", "Reserved",
                                "PubNoAlign", "Stack",  "Common", "CommonHi" };

void omf86_98(int type) { // SEGDEF
    static ofield_t const header[] = { { 0, "Id" },       { 5, "Segment:Class:Overlay" },
                                       { 40, "Len" },     { 50, "Align" },
                                       { 68, "Combine" }, { 0, NULL } };

    uint8_t segAttr                = getu8();
    uint16_t frame                 = 0;
    uint8_t frameOffset            = 0;
    uint8_t ltlDat                 = 0;
    uint32_t maxLen                = 0;
    uint16_t grpOffset             = 0;
    /* parse the seg attr */
    uint8_t a = segAttr >> 5;
    uint8_t c = (segAttr >> 2) & 7;
    if (omfFlavour == ANY && (a == 6 || c == 1 || c == 3))
        omfFlavour = INTEL;

    if (a == 0 || (a == 5 && omfFlavour == INTEL)) {
        frame       = getu16();
        frameOffset = getu8();
    } else if (a == 6) {
        ltlDat    = getu8();
        maxLen    = getu16();
        grpOffset = getu16();
        if (ltlDat & 1)
            maxLen = 0x10000;
    }
    uint32_t segLen = type & 1 ? getu32() : getu16();
    if ((segAttr & 2) && !(type & 1))
        segLen = 0x10000;

    char const *fullName = "*Unnamed*";

    if (a != 5 || omfFlavour != INTEL) {
        char const *segName = getIndexName(INAME, getIndex());
        char const *clsName = getIndexName(INAME, getIndex());
        char const *ovlName = getIndexName(INAME, getIndex());

        char tmpName[256];
        strcpy(tmpName, segName);
        if (*clsName)
            strcat(strcat(tmpName, ":"), clsName);
        if (*ovlName)
            strcat(strcat(tmpName, *clsName ? ":" : "::"), ovlName);
        fullName = pstrdup((uint16_t)strlen(tmpName), tmpName);
    }
    setIndex(ISEG, segIndex, fullName);

    if (malformed)
        return;
    oaddHeader(1, header);
    startCol(1);

    add("#%-3d %s", segIndex++, fullName);
    if ((type & 1) && segAttr & 2)
        addAt(header[2].tabStop, "100000000");
    else
        addAt(header[2].tabStop, "%04X", segLen);
    if (a == 6 && maxLen != segLen)
        add("-%04X", maxLen);

    addAt(header[3].tabStop, "%s", omfFlavour != INTEL && a == 5 ? "DWord" : enumAlign86[a]);
    if (a == 0 || (a == 5 && omfFlavour == INTEL))
        add(" %04X:%02X", frame, frameOffset);
    else if (a == 6 && (ltlDat & 0x80))
        add(" Group+%04X", grpOffset);

    addAt(header[4].tabStop, "%s",
          omfFlavour != INTEL && (c == 4 || c == 7) ? "Public" : enumCombine86[c]);
    if (omfFlavour == MS)
        add(segAttr & 1 ? " Use32" : " Use16");
    else if (segAttr & 1)
        add(" InPage");
}
void omf86_9A(int type) { // GRPDEF
    static ofield_t const header[] = { { 0, "Component" }, { 0, NULL } };

    char const *grpName            = getIndexName(INAME, getIndex());
    setIndex(IGROUP, grpIndex, grpName);
    add("#%d %s", grpIndex++, grpName);
    oaddHeader(4, header);
    while (!atEndRec()) {
        uint8_t ltlDat;
        uint32_t len;
        uint32_t maxLen;
        startCol(4);
        uint8_t cTyp = getu8();
        switch (cTyp) {
        case 0xff:
            add("SI %s", getIndexName(ISEG, getIndex()));
            break;
        case 0xfe:
            add("EI %s", getIndexName(IEXT, getIndex()));
            break;
        case 0xfd:
            add("SCO %s", getIndexName(INAME, getIndex()));
            add(":%s", getIndexName(INAME, getIndex()));
            add(":%s", getIndexName(INAME, getIndex()));
            break;
        case 0xfb:
            ltlDat = getu8();
            maxLen = getu16();
            len    = getu16();
            if (ltlDat & 1)
                maxLen = 0x10000;
            if (ltlDat & 2)
                len = 0x10000;
            add("LTL %04X", len);
            if (len != maxLen)
                add("-%04X", maxLen);
            break;
        case 0xfa:
            add("ABS %04X:", getu16());
            add("%02X", getu8());
            break;
        default:
            malformed = true;
            return;
        }
    }
}
void omf86_9C(int type) { // FIXUPP
    static ofield_t const header[] = { { 0, "Locat" },
                                       { 6, "Mode" },
                                       { 19, "Method" },
                                       { 31, "Frame" },
                                       { 48, "Target(,displacement)" },
                                       { 0, NULL } };
    if (!atEndRec())
        oaddHeader(1, header);
    while (!atEndRec()) {
        startCol(1);
        uint16_t typ = getu8();
        if (typ & 0x80)
            explicitFixup((uint8_t)typ);
        else
            threadFixup((uint8_t)typ);
    }
}
void omf86_A0(int type) { // LEDATA
    add("%s", getIndexName(ISEG, getIndex()));
    hexDump(type & 1 ? getu32() : getu16(), strchr("\x9c\x9d", peekNextRecType()) != NULL);
}

void omf86_A2(int type) { // LIDATA
    add("%s", getIndexName(ISEG, getIndex()));
    iData86(type & 1 ? getu32() : getu16());
}

int communalLen() {
    uint8_t c = getu8();
    if (c <= 128)
        return c;
    else if (c == 0x81)
        return getu16();
    else if (c == 0x84)
        return getu32();
    else if (c == 0x88)
        return geti32();

    Log("Invalid Communal Length Component");
    flagMalformed();
    return 0;
}

void omf86Com(int type) { // COMDEF
    static field_t const header[] = {
        { "Communal Name", WNAME }, { "Type" }, { "Length" }, { NULL }
    };
    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%s", getName());   // communal name
        addField("@%d", getIndex()); // type
        uint8_t dataType = getu8();
        if (1 <= dataType && dataType < 0x60)
            addField("Borland[@%d]", dataType);
        else if (dataType == 0x61) {
            addField("%+d x ", communalLen());
            add("%+d", communalLen());
        } else if (dataType == 0x62)
            addField("%+d", communalLen());
        else {
            Log("Invalid Communal Length Field");
            flagMalformed();
        }
    }
}
void omf86_B2(int type) { // BAKPAT
}
// LEXTDEF

// LPUBDEF

// LCOMDEF

void omf86_BC(int type) { // CEXTDEF
    field_t const header[] = { { "Id", 4 }, { "Type" }, { "Name", WNAME }, { NULL } };
    int cols               = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        char const *name = getIndexName(INAME, getIndex());
        uint16_t extTyp  = getIndex();
        // TODO - check if a new ext id is generated
        setIndex(IEXT, extIndex, name);
        addField("@%d", extIndex++);
        addField("@%d", extTyp);
        addField("%s", name);
    }
}
void omf86_C2(int type) { // COMDAT
}

void omf86_C4(int type) { // LINSYM
    static field_t const header[] = { { "Offset" }, { "Line", 5 }, { NULL } };

    uint8_t flags                 = getu8();
    add(" %s%s", omfFlavour == IBM ? getName() : getIndexName(INAME, getIndex()),
        flags & 1 ? " continued" : ""); // TODO check ICOMDAT

    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        uint16_t lineNum = getu16();
        addField("%04X", is32bit ? getu32() : getu16());
        addField("+%d", lineNum);
    }
}
void omf86_C6(int type) { // ALIAS
    static field_t const header[] = { { "Alias", WNAME }, { "Substitute", WNAME }, { NULL } };
    int cols                      = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%s", getName()); // alias
        addField("%s", getName()); // substitute
    }
}

void omf86_C8(int type) { // NBKPAT
    static char const *locname[]  = { "Byte", "Word", "DWord" };
    static field_t const header[] = { { "Offset" }, { "Value" }, { NULL } };

    uint8_t locTyp                = getu8();
    add("%s", locTyp < 2 || (is32bit && locTyp == 2) ? locname[locTyp] : "???");
    add(" %s",
        omfFlavour == IBM ? getName() : getIndexName(INAME, getIndex())); // TODO check ICOMDAT

    int cols = addReptHeader(header);
    while (!atEndRec()) {
        startCol(cols);
        addField("%04X", is32bit ? getu32() : getu16()); // offset
        addField("%04X", is32bit ? getu32() : getu16()); // value
    }
}

// LLNAMES

void omf86_CC(int type) { // VERNUM
    add("v%s", getName());
}

void omf86_CE(int type) { // VENDEXT
    add("vendor %d Extension Info:", getu8());
    hexDump(0, false);
}
