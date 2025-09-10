#include "utility.h"
#include "verInfo.h"
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef _DEBUG
#define BUILD "Debug "
#else
#define BUILD
#endif
/// <summary>
/// Shows the version information combining the static information from appinfo.h
/// and the generated version information derived from git in _version.h
/// Usually called from chkStdOptions which can be
/// invoked directly or indirectly via getopt
/// </summary>
/// <param name="fp">File stream to write to</param>
/// <param name="what">if 0 (false) show simple version info
/// else if 1 (true) show extended version info
/// else show simple version and description</param>
void showVersion(FILE *fp, bool full) {
    fprintf(fp, "%s (C) %s", appInfo.product, appInfo.owner);
    if (appInfo.mods)
        fprintf(fp, " <%s>", appInfo.mods);
    fprintf(fp, " %s\n", appInfo.version);
    if (appInfo.description)
        fprintf(fp, "%s\n", appInfo.description);
    if (full) {
        fprintf(fp, BUILD "%d bit build: ", (int)(sizeof(void *) * CHAR_BIT));
#ifdef _MSC_VER
        fprintf(fp, "(msvc %d.%d.%d) ", _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 100000);
#elif __GNUC__
        fprintf(fp, "(gcc %s) ", __VERSION__);
#endif
        fprintf(fp, "%s\n", appInfo.build);
        if (appInfo.contributors)
            fprintf(fp, "Contributors: %s\n", appInfo.contributors);
        fprintf(fp, "Support email: %s\n", appInfo.email);
    }
}
