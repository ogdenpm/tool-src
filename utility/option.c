#include "utility.h"
#include "verInfo.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
 * modified version of the public domain AT&T getopt
 * in addition to : being used to indicate a required argument to the option
 * = is supported to allow an optional argument to the option for example if opts contains
 * "i=" then -i, -i=10 and -i =10 are supported. note -i= 10 is treated as argument of "" and 10 is another option
 * '*' passes rest of option as an argument but does not extend into next option
 * e.g. -a* accepts -a85 as -a option and argument as 85
 * returns EOF on end of processing, else the option or option|OPTNOARG if missing argument
 * Note invalid options are assumed to have no argument
 * sets the globals
 *   optopt    -> the option
 *   optarg    -> the argument or NULL
 *   optind    -> argv index to process next
 */

int optInd = 1;
int optOpt;
char const *optArg;

/// <summary>
/// simple option processing
/// Any argv argument starting -- is treated as end of processing, as such long options are not
/// supported It extends AT&T getopt by supporting optional =value arguments and * for longer named arg
/// When arguments are needed then it consumes the rest of the current argv argument being processed if not at end of
/// string or the next argv argument
/// </summary>
/// <param name="argc">The number of arguments in argv</param>
/// <param name="argv">The array of arguments</param>
/// <param name="opts">A string containing the characters of supported options.
/// If the character is followed by a ':' then it requires an argument
/// If the character is followed by a '=' then it accepts an optional argument
/// If the character is followed by a '*' then it accepts rest of the current argv argument as an argument</param>
/// <returns>
/// The option or EOF on end of options. If a required argument is missing the OPTNOARG is added to
/// the return value. In addition the following globals are updated optopt contains the option
/// selected, optarg is the argument or NULL optind is index of the next argv argument to process
/// </returns>
///
static char specialOpt[] = ":=*";   // special option separators
int getOpt(int argc, char **argv, char const *opts) {
    static char const *place = ""; // force new arg

    if (!*place) {
        if (optInd == 1)
            chkStdOptions(argc, argv);
        if (optInd >= argc || *(place = argv[optInd]) != '-' || place[1] == '\0')
            return (EOF);
        else if (*++place == '-') { // -- finishes
            optInd++;
            return (EOF);
        }
    }
    optOpt = *place++;
    char *cp = strchr(specialOpt, optOpt) ? NULL : strchr(opts, optOpt);
    if (cp && strchr(specialOpt, *++cp)) {
        if (*place || *cp == '*')   // arg or attached arg only
            optArg = place;
        else {
            if (++optInd >= argc) {
                if (*cp != '=')
                    optOpt |= MISSINGARG;
                else
                    optArg = "";
            } else
                optArg = argv[optInd];
        }
        if (*cp == '=' && *optArg++ != '=')
            optArg = NULL; // backup and leave for next option
        else {
            optInd++;
            place = "";
        }
    } else  {
        optArg = NULL;
        if (!*place)
            optInd++;
    }
    return optOpt;
}


