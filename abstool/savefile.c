#include <stdio.h>
#include <stdlib.h>
#include "abstool.h"


uint8_t modhdr[38] = {
    MODHDR, 11, 0, 7, 'U', 'N', 'K', 'N', 'O', 'W', 'N', 0, 0, 0xbc
};                                                // default MODHDR record
uint8_t modend[] = { MODEND, 5, 0, 1, 0, 0, 0, 0xf6 }; // default MODEND record
uint8_t modeof[] = { MODEOF, 1, 0, 0xf1 };           // MODEOF record



void putword(int n, FILE *fp) {
    putc(n, fp);
    putc(n / 256, fp);
}

uint8_t crcSum(uint8_t *blk, int len) {
    uint8_t sum = 0;
    while (len--)
        sum += *blk++;
    return sum;
}

bool fputblock(FILE *fp, unsigned low, unsigned high, int outFormat) {
    unsigned len = high - low;
    uint8_t crc;
    switch (outFormat) {
    case BIN:
        putword(len, fp);
        putword(low, fp);
        /* FALL THROUGH */
    case IMAGE:
        fwrite(&inMem[low], 1, len, fp);
        break;
    case OMF85:
        putc(MODCONTENT, fp);
        putword(len + 4, fp);
        putc(0, fp);
        putword(low, fp);
        fwrite(inMem + low, 1, len, fp);
        crc = 0 - MODCONTENT - (len + 4) - (len + 4) / 256 - crcSum(inMem + low, len);
        putc(crc, fp);
        break;
    case HEX:
        while (len) {
            int chunk = len < HEXBYTES ? len : HEXBYTES;
            fprintf(fp, ":%02X%04X00", chunk, low);
            for (int i = 0; i < chunk; i++)
                fprintf(fp, "%02X", inMem[low + i]);
            crc = 0 - chunk - crcSum(inMem + low, chunk);
            fprintf(fp, "%02X\r\n", crc);
            low += chunk;
            len -= chunk;
        }
        break;
    }
    return ferror(fp) == 0;
}

void insertJmpEntry() {
    int loc = low;
    // remove lxi sp,... if present and there is potentially a jmp after
    if (low <= high - 6 && use[low] == DATA && inMem[low] == LXISP && use[low + 1] && use[low + 2])
        loc += 3;

    if (loc > high - 3 || use[loc] != DATA || inMem[loc] != JMP) {
        fprintf(stderr, "Failed pre-checks: Skipping writing jmp to entry\n");
        return;
    }
    // sanity check for start
    // shouldn't jump into (lxi sp,xxx) jmp yyy code at low, or after end of code
    if (start < loc + 3 || start >= high) {
        fprintf(stderr, "jmp %04X out of bounds: Skipping writing jmp entry\n", start);
        return;
    }
    inMem[loc] = JMP;
    use[loc++] = DATA;
    inMem[loc] = start % 256;
    use[loc++] = DATA;
    inMem[loc] = start / 256;
    use[loc]   = DATA;
}

int saveFile(char *file, int outFormat) {
    FILE *fp;
    bool isOk = true;
    int addr;



    if (high == low)
        error("Nothing to save");

    if ((fp = fopen(file, "wb")) == NULL)
        error("can't create output file %s\n", file);

    if (outFormat == IMAGE) {

        isOk = fputblock(fp, low, high, outFormat);
    } else {
        if (outFormat == OMF85)
            fwrite(modhdr, 1, modhdr[1] + 3, fp);

        for (addr = low; addr < high && isOk;) {
            while (addr < high && use[addr] == UNINITIALISED)
                addr++;
            if (addr == high || use[addr] != DATA)
                break;
            low = addr;
            while (addr < high && use[addr] == DATA)
                addr++;
            isOk = fputblock(fp, low, addr, outFormat);
        }

        if (isOk) {
            switch (outFormat) {
            case BIN:
                putword(0, fp);
                putword(start, fp);
                break;
            case OMF85:
                modend[5] = start; // insert start
                modend[6] = start / 256;
                modend[7] = -crcSum(modend, 7); // new CRC
                fwrite(modend, 1, 8, fp);
                fwrite(modeof, 1, 4, fp);
                break;
            case HEX:
                fprintf(fp, ":00%04X01%02X\r\n", start, (0 - (start + start / 256 + 1)) & 0xff);
                break;
            }
            isOk = ferror(fp) == 0;
        }

        if (isOk && addr < high) // apply any padding
            isOk = fwrite(&inMem[addr], 1, high - addr, fp) == high - addr;
    }
    fclose(fp);
    if (!isOk)
        fprintf(stderr, "write failure on %s\n", file);
    return isOk ? EXIT_SUCCESS : EXIT_FAILURE;
}
