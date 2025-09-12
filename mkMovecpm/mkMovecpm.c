#include "utility.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

const char help[] = "Usage: %s cpmovefile bootfile cpmfile [outputfile]\n"
                    "Where cpmovefile and output file are binary files\n"
                    "bootfile absolute load file with .spr reloc info and cpmfile is a .spr file\n"
                    "If outputfile is not specified movecpm.com is generated\n";

char *outfile     = "movcpm.com";

uint8_t image[0x4000]; // max 16K image of final file
uint8_t reloc[0x800];  // max 2K relocation table

uint16_t top = 0; // current top of image

#define MOVEBASE  0x100
#define SIZELOC   0x801
#define IMAGEBASE 0x900
#define BOOTBASE  0x980
#define CPMBASE   0xA00

void loadFile(char *name, uint16_t load) {
    if (load < top)
        fatal("File %s overlaps previous files", name);
    FILE *fp = fopen(name, "rb");
    if (!fp)
        fatal("Cannot open %s", name);
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (sz < 0)
        fatal("Cannot determine size of %s", name);

    uint16_t progSize = (uint16_t)sz;
    uint16_t relSize  = 0;
    if (load != MOVEBASE) { // it's a .spr file
        if (sz > 3) {
            getc(fp); // skip 0
            progSize = getc(fp);
            progSize |= getc(fp) << 8;
            relSize = (progSize + 7) / 8;
        }
        if (sz < progSize + relSize + 256)
            fatal("File %s too small to be a .spr file", name);
        fseek(fp, 256, SEEK_SET);
    }
    top = load + progSize;
    if (top > sizeof(image))
        fatal("File %s too large", name);
    if (fread(&image[load], 1, progSize, fp) != (size_t)progSize)
        fatal("Error reading file %s", name);
    if (relSize) {
        uint16_t baseReloc = (load - IMAGEBASE) / 8;
        if (fread(&reloc[baseReloc], 1, relSize, fp) != (size_t)relSize)
            fatal("Error reading relocation data from file %s", name);
    }
    fclose(fp);
}

int main(int argc, char **argv) {

    chkStdOptions(argc, argv);
    if (argc == 5)
        outfile = argv[4];
    else if (argc != 4)
        usage("Incorrect number of files");
    loadFile(argv[1], MOVEBASE); // movecpm file
    loadFile(argv[2], BOOTBASE); // boot file
    loadFile(argv[3], CPMBASE);  // cpm file includes ccp, bdos and bios
    // round top to a page boundary
    top = (top + 255) & ~255;
    image[SIZELOC + 1] = (top - IMAGEBASE) >> 8;
    FILE *fp             = fopen(outfile, "wb");
    if (!fp)
        fatal("Cannot create output file %s", outfile);
    if (fwrite(&image[MOVEBASE], 1, top - MOVEBASE, fp) != (size_t)(top - MOVEBASE) ||
        fwrite(reloc, 1, (top - IMAGEBASE) / 8, fp) != (size_t)((top - IMAGEBASE) / 8))
        fatal("Error writing output file %s", outfile);
    fclose(fp);
}