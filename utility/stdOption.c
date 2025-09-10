#include "utility.h"
#include "verInfo.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *programName = "???";

/// <summary>
/// Process standard options, currently -v, -V and -h
/// also captures argv[0] as programName
/// Options must be the only ones present
/// </summary>
/// <param name="s">The s.</param>
void chkStdOptions(int argc, char **argv) {
    programName = argv[0];
#ifdef _WIN32
    char *s = strrchr(programName, '.');
    if (s && stricmp(s, ".exe") == 0)
        *s = '\0';
#endif
    if (argc == 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "-V") == 0) {
            showVersion(stdout, argv[1][1] == 'V');
            exit(0);
        } else if (strcmp(argv[1], "-h") == 0)
            usage(NULL);
    }
}
