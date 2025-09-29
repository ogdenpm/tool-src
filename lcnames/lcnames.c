#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char const help[] = "Usage: %s [options] [file...]\n"
                    "Options:\n"
                    " -v   Show progress\n"
                    "Note rename failure will always be reported\n";

int main(int argc, char **argv) {
    bool quiet = true;
    while (getOpt(argc, argv, "v") != EOF) {
        switch (optOpt) {
        case 'v':
            quiet = false;
            break;
        default:
            usage("Unknown option -%c\n", optOpt);

            return 1;
        }
    }
    while (optInd < argc) {
        char *filename = argv[optInd++];
        char const *s  = basename(filename);
        while (*s && !isupper(*s))
            s++;
        if (*s) {
            char *newname = strdup(filename);
            for (char *p = (char *)basename(newname); *p; p++)
                *p = (char)tolower(*p);
            if (rename(filename, newname) != 0)
                fprintf(stderr, "Failed to rename %s to %s\n", filename, newname);
            else if (!quiet)
                printf("%s -> %s\n", filename, newname);
            free(newname);
        } else if (!quiet)
            printf("%s unchanged\n", filename);
    }
    return 0;
}