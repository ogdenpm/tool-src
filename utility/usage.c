#include "utility.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// <summary>
/// Display the application help
/// if the optional fmt, ... is present the formatted
/// string is written to stderr, preceeded by "Error: ", followed by the help and the program exits
/// with 1
/// if fmt is NULL then help is printed to stdout and program exits with 0
/// </summary>
/// <param name="fmt">optional format</param>
/// <param name="">and arguments</param>
_Noreturn void usage(char *fmt, ...) {
    FILE *fp = fmt ? stderr : stdout;
    showVersion(fp, false);
    putc('\n', fp);

    if (fmt) {
        va_list args;
        va_start(args, fmt);
        fputs("Error: ", fp);
        vfprintf(fp, fmt, args);
        fputs("\n\n", fp);
        va_end(args);
    }

    for (char const *s = help; *s; s++)
        if (*s == '%') {
            if (s[1] == 's') {
                fputs(basename(programName), fp); // %s replaced by program name
                s++;
            } else {
                putc('%', fp); // %% treated as %
                if (s[1] == '%')
                    s++;
            }
        } else
            putc(*s, fp);
    fputs("\nAlso supports single arguments -v, -V for version info and -h for help\n", fp);
    Exit(fmt ? 1 : 0);
}