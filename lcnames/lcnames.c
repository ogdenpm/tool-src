#include "utility.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char const help[] = "Usage: %s [options] [file...]\n"
                    "Options:\n"
                    " -u  Rename to upper case\n"
                    " -v  Show progress\n"
                    "Note rename failure will always be reported\n";

int main(int argc, char **argv) {
    bool quiet     = true;
    bool lowercase = true;

    while (getOpt(argc, argv, "uv") != EOF) {
        switch (optOpt) {
        case 'v':
            quiet = false;
            break;
        case 'u':
            lowercase = false;
            break;
        default:
            usage("Unknown option -%c\n", optOpt);
        }
    }
    if (argc == 1)
        usage("Nothing to do");
    while (optInd < argc) {
        char *filename = argv[optInd++];
        char const *s  = basename(filename);
        while (*s && !isalpha(*s))
            s++;
        if (*s) {
            char *newname = strdup(filename);
            for (char *p = (char *)basename(newname); *p; p++)
                *p = lowercase ? (char)tolower(*p) : (char)toupper(*p);

            if (rename(filename, newname) != 0)
                fprintf(stderr, "Failed to rename %s to %s\n", filename, newname);
            else if (!quiet)
                printf("%s renamed\n", newname);
            free(newname);
        } else if (!quiet)
            printf("%s skipped\n", filename);
    }
    return 0;
}