#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <memory.h>

#include "utility.h"

const char help[] = "Usage: %s [-a] [-z] [-u nnnn] basehexfile relochexfile outputfile\n"
                    "Where:\n"
                    "-a       suppress warning on reloc for absoute file\n"
                    "-u nnnn  add nnnn (hex) bytes to BSS\n"
                    "-z       skip zero page data in reloc file\n";

/* extra output file type */
enum { BAD = -2, BADCRC, VALID };

#define MAXMEM 0x10000

uint8_t record[32];
int recLen;
int recAddr;

typedef struct {
    int low;
    int high;
    uint16_t mStart;
    uint8_t mem[MAXMEM];
} image_t;

image_t baseImage  = { MAXMEM, 0, 0, { 0 } };
image_t relocImage = { MAXMEM, 0, 0, { 0 } };

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
  utility function to copy read data into the memory image
  updates memory bounds and usage.
*/

void addContent(image_t *image, int addr, uint16_t len) {

    if (addr + len > MAXMEM) {
        fprintf(stderr, "Warning: data beyond 0FFFFH ignored\n");
        len = MAXMEM - addr;
    }
    if (image->high == 0) {
        image->low  = addr;
        image->high = addr + len;
    } else {
        if (addr < image->low)
            image->low = addr;
        if (addr + len > image->high)
            image->high = addr + len;
    }
    memcpy(&image->mem[addr], record, len);
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

/* load an Intel Hex file into memory*/
void loadHex(char *name, image_t *image) {
    FILE *fp = fopen(name, "r");
    if (!fp)
        fatal("Cannot open input file %s", name);
    int type;
    while ((type = readHex(fp)) >= 0) {
        if (type == 0 && recLen) /* data record */
            addContent(image, recAddr, recLen);
    }
    fclose(fp);
    if (image->high == 0)
        fatal("Input file %s contains no valid data", name);
}

void writeSPR(char *name, image_t *base, image_t *reloc, bool skipZero, uint16_t bssSize) {
    FILE *fp = fopen(name, "wb");
    if (!fp)
        fatal("Cannot open output file %s", name);
    uint16_t imageSize = base->high - base->low;

    if (fputc(0, fp) == EOF || fputc(imageSize & 0xff, fp) == EOF ||
        fputc(imageSize >> 8, fp) == EOF || fputc(0, fp) == EOF ||
        fputc(bssSize & 0xff, fp) == EOF || fputc(bssSize >> 8, fp) == EOF ||
        fseek(fp, 256, SEEK_SET) != 0)
        fatal("Error writing header to %s", name);

    if (fwrite(&base->mem[base->low], 1, imageSize, fp) != (size_t)imageSize)
        fatal("Error writing image to %s", name);

    uint8_t relocBits = 0;
    uint8_t *baseByte = &base->mem[base->low];
    uint8_t *relocByte = &reloc->mem[reloc->low];

    for (int i = 0; i < imageSize; i++, baseByte++, relocByte++) {
        if (i % 8 == 0 && i) {
            if (fputc(relocBits, fp) == EOF)
                fatal("Error writing relocation information to %s", name);
            relocBits = 0;
        }
        if (*relocByte || !skipZero) {
            int16_t delta = *baseByte - *relocByte;
            if (delta) {
                if (delta != 1 && delta != -1)
                    warn("Relocation difference %d at address %04XH not +/- 1", delta,
                         base->low + i);
                relocBits |= 0x80 >> (i % 8);
            }
        }
    }
    if (imageSize && fputc(relocBits, fp) == EOF)
        fatal("Error writing relocation information to %s", name);
    fclose(fp);
}

int main(int argc, char **argv) {
    bool skipZero    = false;
    bool absfile     = false;
    uint16_t bssSize = 0;

    while (getOpt(argc, argv, "zu:") != EOF) {
        switch (optOpt) {
        case 'a':
            absfile = true;
            break;
        case 'z':
            skipZero = true;
            break;
        case 'u':
            bssSize = (uint16_t)strtoul(optArg, NULL, 16);
            break;
        default:
            if (optOpt & MISSINGARG)
                usage("Option -%c requires an argument", optOpt & 0xff);
            else
                usage("Unknown option -%c", optOpt);
        }
    }
    if (optInd + 3 != argc)
        usage("Incorrect number of files");
    loadHex(argv[optInd], &baseImage);      // base hex file
    loadHex(argv[optInd + 1], &relocImage); // reloc hex file
    if (relocImage.high - relocImage.low != baseImage.high - baseImage.low)
        fatal("Reloc file size does not match base file size");
    if (((baseImage.low ^ relocImage.low) & 0xff) != 0)
        fatal("Base and reloc files are not at same page offset");
    if (relocImage.low != baseImage.low + 0x100 && relocImage.low != baseImage.low - 0x100)
        if (!absfile || relocImage.low != baseImage.low)
            warn("Reloc file is not at +/- 100h of base file");
    if (baseImage.low & 0xff) {
        warn("Base file is not on page boundary - forced");
        baseImage.low &= ~0xff;
        relocImage.low &= ~0xff;
    }

    writeSPR(argv[optInd + 2], &baseImage, &relocImage, skipZero, bssSize);
}