#include "utility.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

/// <summary>
/// generated a new file name by replacing/adding the ext
/// unless force is false and an ext exists already
/// </summary>
/// <param name="path">base filename including path</param>
/// <param name="ext">new extent including '.' to append</param>
/// <param name="force">if true will always replace/add ext, else only add if no existing
/// ext</param> <returns> created file, note will be freed at next call</returns>
static char *newFile;

char *makeFilename(char const *path, char const *ext, bool force) {
    char *s = strrchr(basename(path), '.');
    if (s && !force) // keep existing ext
        ext = s;
    int prefixLen = (s ? (int)(s - path) : (int)strlen(path));
    free(newFile);
    newFile = safeMalloc(prefixLen + strlen(ext) + 1);
    strncpy(newFile, path, prefixLen);
    strcpy(newFile + prefixLen, ext);
    return newFile;
}