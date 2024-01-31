#include "abstool.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

uint8_t inMem[MAXMEM + MAXAPPEND];
uint8_t use[MAXMEM + MAXAPPEND];

int low  = MAXMEM;
int high = 0;
int base = -1;
int start;

uint8_t record[MAXMEM];
int recLen;
int recAddr;


/* read a word from a file,  return the value if not EOF else -1 */
int getword(FILE *fp) {
    int low  = getc(fp);
    int high = getc(fp);
    return high == EOF ? -1 : low + high * 256;
}

/*
   support functions to handle reading ascii hex from a file
   they return the value or -1 on error
*/

int getHex1(FILE *fp) {
    int c = getc(fp);
    return !isxdigit(c) ? -1 : isdigit(c) ? c - '0' : toupper(c) - 'A' + 10;
}
int getHex2(FILE *fp) {
    int high = getHex1(fp);
    int low  = getHex1(fp);
    return low < 0 ? -1 : high * 16 + low;
}

int getHex4(FILE *fp) {
    int high = getHex2(fp);
    int low  = getHex2(fp);
    return low < 0 ? -1 : high * 256 + low;
}


/*
   read an intel AOMF85 record from file into "record"
   returns the type if valid, BAD for invalid type and BADCRC if crc check fails
   sets "recLen"
*/
int readOMF(FILE *fp) {
    static char *validRec = "\x2\x4\x6\x8\xe\x10\x12\x16\x18\x20";
    int type              = getc(fp);
    if (!strchr(validRec, type) || (recLen = getword(fp)) < 1 ||
        fread(record, 1, recLen, fp) != recLen)
        return BAD;
    uint8_t crc = type + recLen / 256 + recLen;
    for (int i = 0; i < recLen; i++)
        crc += record[i];
    recLen--; // remove CRC
    return crc == 0 ? type : BADCRC;
}


/*
    read an intel Hex record into "record"
    sets recLen and recAddr
*/
int readHex(FILE *fp) {
    int c;
    while ((c = getc(fp)) != ':')
        if (!isprint(c) && c != '\r' && c != '\n' && c != '\t' && c != '\f')
            return BAD;
    int type;
    if ((recLen = getHex2(fp)) < 0 || (recAddr = getHex4(fp)) < 0 || (type = getHex2(fp)) < 0 ||
        type > 1)
        return BAD;

    uint8_t crc = type + recLen + recAddr / 256 + recAddr;
    for (int i = 0; i < recLen + 1; i++) {
        if ((c = getHex2(fp)) < 0)
            return BAD;
        crc += record[i] = c;
    }
    return crc == 0 ? type : BADCRC;
}

/*
   read an ISIS I Bin block into "record", sets recLen and recAddr
   Note assumes chkBin has been used to check that the format is valid.
*/
int readBin(FILE *fp) {
    recLen  = getword(fp);
    recAddr = getword(fp);
    fread(record, 1, recLen, fp);
    return VALID;
}


/*
   check for a plausible ISIS I Bin file
   the block chain should be valid, ending in a 0 length
   each address block should be to RAM
   and there should be no more than MAXAPPEND bytes remaining
*/
bool chkBin(FILE *fp) {
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    for (;;) {
        int len  = getword(fp);
        int addr = getword(fp);
        if (len < 0 || addr < 0 || addr + len >= 0xe000) // EOF reached or addr in ROM!!
            return false;
        long here = ftell(fp);
        if (len == 0) { // possible end of BIN, check not too much following it
            return fileSize - here < MAXAPPEND;
        } else if (here + len > fileSize)
            return false;
        fseek(fp, len, SEEK_CUR); // skip actual data
    }
}


/* determine the file type, fp is assumed to have been opened using "rb" */
int fileType(FILE *fp) {
    if (readOMF(fp) == MODHDR) /* got a valid MODHDR */
        return OMF85;
    rewind(fp);
    if (readHex(fp) >= 0) /* got a valid hex record */
        return HEX;
    rewind(fp);
    if (chkBin(fp)) /* looks like an ISIS I Bin file */
        return BIN;
    else
        return IMAGE; /* treat as simple image */
}



/*
  utility function to copy read data into the memory image
  updates memory bounds and usage.
*/

void addContent(int addr, uint8_t *data, int len) {
    if (addr + len > MAXMEM) {
        fprintf(stderr, "Warning input file: data beyond 0FFFFH ignored\n");
        len = MAXMEM - addr;
    }

    if (addr < low)
        low = addr;
    if (addr + len > high)
        high = addr + len;
    memcpy(inMem + addr, data, len);
    memset(use + addr, DATA, len);
}


/* load an AOMF85 file into memory */
void loadOMF(FILE *fp) {
    rewind(fp);
    int type;
    while ((type = readOMF(fp)) != MODEOF) {
        if (type < MODHDR)
            error("Invalid AOMF85 record");
        else if (type == MODCONTENT) {
            if (record[0] != 0)
                error("AOMF85 CONTENT record has relocatable data");
            recAddr = record[1] + record[2] * 256;
            addContent(recAddr, record + 3, recLen - 3);
        } else if (type == MODEND) {
            if (record[1] != 0)
                error("AOMF85 MODEND record has relocatable start");
            recAddr = record[2] + record[3] * 256;
            break;
        } else if (type == MODHDR && recLen < 35 && 1 <= record[0] && record[0] <= 31) {
            modhdr[1] = recLen + 1;
            memcpy(modhdr + 3, record, recLen + 1); // copy name and existing  CRC
        }
    }
    if (type == MODEND && readOMF(fp) != MODEOF)
        warning("AOMF85 MODEOF record expected after MODEND\n");
}

/* load an Intel Hex file into memory*/
void loadHex(FILE *fp) {
    rewind(fp);
    int type;
    while ((type = readHex(fp)) >= 0) {
        if (type == 0) /* data record */
            addContent(recAddr, record, recLen);
        else
            return; /* end record - recAddr = start address*/
    }
    error("Intel Hex file missing end record");
}

/*
   load an ISIS I bin file into memory
   Assumes chkBin has been used to verify it is a valid file so no checks here
*/
void loadBin(FILE *fp) {
    rewind(fp);
    while (readBin(fp), recLen != 0)
        addContent(recAddr, record, recLen);
    /* on reaching here recAddr = start address */
}

/* load binary image into memory */
void loadImage(FILE *fp) {
    rewind(fp);
    recLen  = (int)fread(record, 1, MAXMEM, fp);
    recAddr = base >= 0 ? base : 0x100; // assume CP/M unless base specified
    addContent(recAddr, record, recLen);
    /* recAddr is the base address and is the assumed start address */
}

bool loadFile(char *file) {
    FILE *fp;
    if ((fp = fopen(file, "rb")) == NULL)
        error("Cannot open input file %s", file);
    switch (fileType(fp)) {
    case OMF85:
        loadOMF(fp);
        break;
    case HEX:
        loadHex(fp);
        break;
    case BIN:
        loadBin(fp);
        break;
    default:
        loadImage(fp);
        break;
    }
    fclose(fp);
    start = recAddr;
    if (low < high) {
        printf("Load %04XH-%04XH  Start %04XH\n", low, high - 1, start);
        return true;
    }
    return false;
}
