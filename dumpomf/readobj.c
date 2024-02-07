#include "omf.h"

long start; // file start position of current record

uint8_t rec[MAXREC];
int recType;
uint8_t *recPtr;
uint8_t *recEndPtr;
uint8_t *recMark;

int loadRec() {
    unsigned char hdr[3];
    uint16_t len;
    uint8_t crc;

    if ((len = (uint16_t)fread(hdr, 1, 3, src)) != 3)
        return len == 0 ? Eof : Junk;

    recType = hdr[0];
    len     = hdr[1] + hdr[2] * 256;
    if (fread(rec, 1, len, src) != len)
        return Junk;

    crc = hdr[0] + hdr[1] + hdr[2];
    for (unsigned int i = 0; i < len; i++)
        crc += rec[i];
    recMark = recPtr = rec;
    recEndPtr        = rec + (len - 1);
    malformed        = false;
    return crc == 0 ? Ok : BadCRC;
}

int getrec() {
    int status;

    start = ftell(src);

    if ((status = loadRec()) == BadCRC) {
        if (loadRec() != Ok) /* see if the next record is ok */
            status = Junk;
        fseek(src, start, SEEK_SET); /* reload the record with a bad CRC */
        loadRec();
    }
    return status;
}

bool atEndRec() {
    return recPtr >= recEndPtr;
}

void markRecPos() {
    recMark = recPtr;
}

uint16_t getRecPos() {
    return (uint16_t)(recPtr - rec);
}

void setRecPos(uint16_t pos) {
    recPtr    = rec + pos;
    malformed = recPtr >= recEndPtr;
}

uint16_t revertRecPos() {
    recPtr    = recMark;
    malformed = recPtr >= recEndPtr;
    return (uint16_t)(recPtr - rec);
}

uint8_t getu8() {
    if (recPtr < recEndPtr)
        return *recPtr++;
    malformed = true;
    return 0;
}

uint8_t peekNextRecType() {
    int c = getc(src);
    ungetc(c, src);
    return c == EOF ? 0 : c;
}

uint16_t getu16() {
    uint16_t val = getu8();
    return val + (getu8() << 8);
}

uint32_t getu24() {
    uint32_t val = getu16();
    return val + (getu8() << 16);
}

uint32_t getu32() {
    uint32_t val = getu16();
    return val + (getu16() << 16);
}

int8_t geti8() {
    if (recPtr < recEndPtr)
        return (int8_t)*recPtr++;
    malformed = true;
    return 0;
}

int16_t geti16() {
    int16_t val = getu8();
    return (val + (geti8() << 8));
}

int32_t geti24() {
    int32_t val = getu16();
    return val + (geti8() << 16);
}

int32_t geti32() {
    int32_t val = getu16();
    return val + (geti16() << 16);
}

uint16_t getIndex() {
    uint16_t index = getu8();
    return index & 0x80 ? getu8() + ((index & 0x7f) << 8) : index;
}

char const *getName() {
    uint8_t len = getu8();
    char const *p;
    if (recPtr + len <= recEndPtr) {
        p = pstrdup(len, recPtr);
        recPtr += len;
    } else {
        p         = "";
        malformed = true;
        recPtr    = recEndPtr;
    }
    return p;
}

static void skipName() {
    uint8_t len = getu8();
    if ((recPtr += len) > recEndPtr)
        recPtr = recEndPtr;
}

void flagMalformed() {
    recPtr = recEndPtr;
    malformed = true;
}

int detectOMF() {
    bool lib   = false;
    int status = getrec();

    if (status >= 0 && recType == 0x2c) { /* skip OMF51 / OMF85 LIBHDR */
        status = getrec();
        lib    = true;
    }
    rewind(src); /* rewind the file so next getrec gets the first record */
    if (status < 0)
        return OMFUKN;
    
    if (lib && recType == 0x28) // empty library use any
        return OMF85;

    if (recType == 2) {
        skipName();
        uint8_t trn = getu8();

        if (trn < 3)
            return malformed ? OMFUKN : OMF85;
        if (0xfd <= trn && trn <= 0xff)
            return OMF51;
        if (strchr("\x04\x06\x20\x24\x26\x40\x44\x46\xe0\xe4\xe6", trn & 0xfe))
            return OMF96;
    } else if (recType == 0x2e) /* OMF96 LIBHDR */
        return OMF96;
    else if (strchr("\x6e\x80\x82\xa4", recType)) { /* RHEADR, THEADR, LHEADER, LIBHED */
        omfFlavour = ANY;
        return OMF86;
    }
    return OMFUKN;
}
