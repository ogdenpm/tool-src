#include <stdio.h>
#include "utility.h"
#include <fcntl.h>

/*
    simple cat file program that handles binary and ascii files
    when catting ascii files it converts line endings to \r\n and also stops
    copying a file if ^Z is seen
*/

#ifndef _WIN32
#define _setmode(a,b)
#endif


char const help[] = "Usage: %s [options] file*\n"
                    "Concatenate files, or use standard input if no file is specified\n"
                    "Options are:\n"
                    "-b      Binary mode, copy file exactly\n"
                    "        default is ascii mode, convert line endings to \\r\\n and stop at ^Z\n"
                    "-o file Write to specified file, default is standard output in option is omitted\n";



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

    while (getOpt(argc, argv, "bo:") != EOF) {
        switch (optOpt) {
        case 'b':
            binaryMode = true;
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
        _setmode(_fileno(outFp), _O_BINARY);
    }
    if (argc == optInd) {
        _setmode(_fileno(stdin), _O_BINARY);
        copy(stdin, outFp, binaryMode);
    }
    while (optInd < argc) {
        FILE *inFp;
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