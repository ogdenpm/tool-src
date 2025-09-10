#include "utility.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
/// <summary>
/// Equivalent to POSIX basename for visual C
/// </summary>
/// <param name="path">full path to file</param>
/// <returns>pointer filename only</returns>

char const *basename(char const *path) {
    char *s;
#ifdef _WIN32
    if (path[0] && path[1] == ':') // skip leading device
        path += 2;
#ifdef UNC
    else if (path[0] == '\\' && path[1] == '\\') // skip \\computer\share\ part
        if ((s = strchr(path + 2, '\\')) && (s = strchr(s, '\\')))
            path = s + 1;
        else
            path = strchr(path, '\0');
#endif
#endif
    if ((s = strrchr(path, '/')))
        path = s + 1;
#if _WIN32
    if ((s = strrchr(path, '\\')))
        path = s + 1;
#endif
    return path;
}

#endif

