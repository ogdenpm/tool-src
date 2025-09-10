#include <stdio.h>
#include <ctype.h>
#include "utility.h"
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#endif
/*
    simple cat file program that handles binary and ascii files
    when catting ascii files it converts line endings to \r\n and also stops
    copying a file if ^Z is seen
*/

char const help[] = "Usage: %s [options] file*\n"
                    "Concatenate files, or use standard input if no file is specified\n"
                    "Options are:\n"
                    "-b[n]    Binary copy. If n present force append at 2^n boundary\n"
                    "-o file  Write to specified file, default is standard output in option is omitted\n"
                    "Default copy mode is text. Converting line endings to \\r\\n and stopping at ^Z\n";



void copy(FILE *inFp, FILE *outFp, bool binaryMode) {
    int c, prevch = 0;
    while ((c = getc(inFp)) != EOF) {
        if (!binaryMode) {
            if (c == 26) // ^Z
                break;
            if (c == '\n' && prevch != '\r')
                putc('\r', outFp);
            prevch = c;
        }
        putc(c, outFp);

    }
}


int main(int argc, char **argv) {
    bool binaryMode = false;
    FILE *outFp     = NULL;
    int align   = 0;

    while (getOpt(argc, argv, "b*o:") != EOF) {
        switch (optOpt) {
        case 'b':
            binaryMode = true;
            while (isdigit(*optArg))
                align = align * 10 + *optArg++ - '0';
            break;
        case 'o':
            if (!(outFp = fopen(optArg, "wb")))
                usage("Cannot open output file %s", optArg);
            break;
        default:
            if (optOpt & MISSINGARG)
                usage("Option -%c missing argument", optOpt & 0xff);
            else
                usage("Unknown option -%c", optOpt);
            break;
        }
    }
    if (!outFp) {
        outFp = stdout;
#ifdef _WIN32
        (void)_setmode(_fileno(outFp), _O_BINARY);
#endif
    }
    if (argc == optInd) {
#ifdef _WIN32
        (void)_setmode(_fileno(stdin), _O_BINARY);
#endif
        copy(stdin, outFp, binaryMode);
    }
    while (optInd < argc) {
        FILE *inFp;
        if (binaryMode && align) {
            long where = ftell(outFp);
            while (where & ((1 << align) - 1)) {
                putc(0, outFp);
                where++;
            }
        }
        if (!(inFp = fopen(argv[optInd], "rb")))
            warn("Cannot open input file %s", argv[optInd]);
        else {
            copy(inFp, outFp, binaryMode);
            fclose(inFp);
        }
        optInd++;
    }
    fclose(outFp);
}