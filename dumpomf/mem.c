/****************************************************************************
 *                                                                          *
 *  dumpintel: dump content of an Intel omf file                            *
 *  Copyright (C) 2020 Mark Ogden <mark.pm.ogden@btinternet.com>            *
 *                                                                          *
 *  This program is free software; you can redistribute it and/or           *
 *  modify it under the terms of the GNU General Public License             *
 *  as published by the Free Software Foundation; either version 2          *
 *  of the License, or (at your option) any later version.                  *
 *                                                                          *
 *  This program is distributed in the hope that it will be useful,         *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with this program; if not, write to the Free Software             *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,              *
 *  MA  02110-1301, USA.                                                    *
 *                                                                          *
 ****************************************************************************/

#include "omf.h"
/* string storage */
#define STRCHUNK   8192
#define INDEXCHUNK 256

typedef struct str {
    struct str *next;
    size_t pos;
    char buf[STRCHUNK];
} str_t;

typedef struct _index {
    struct _index *next;
    char const *names[INDEXCHUNK];
} index_t;

int extIndex;
int segIndex;

static index_t *itable[INDEXTABLES] = { 0 };
static str_t strings;

static void *alloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr, "Fatal: out of memory\n");
        exit(1);
    }
    return p;
}

static char *allocStrSpace(size_t len) {
    str_t *p;
    for (p = &strings; len + p->pos > STRCHUNK; p = p->next)
        if (!p->next) {
            p->next       = alloc(sizeof(str_t));
            p->next->next = NULL;
            p->next->pos  = 0;
        }
    char *s = p->buf + p->pos;
    p->pos += len;
    return s;
}

static index_t *newIndexBlock() {
    index_t *p = alloc(sizeof(index_t));
    memset(p, 0, sizeof(*p));
    return p;
}

void resetNames() {
    for (str_t *p = &strings; p; p = p->next)
        p->pos = 0;
    for (int i = 0; i < INDEXTABLES; i++)
        for (index_t *p = itable[i]; p; p = p->next)
            memset((void *)p->names, 0, sizeof(p->names));
}

char const *pstrdup(uint16_t len, char const *s) {
    if (len == 0)
        return "";
    char *newstr = allocStrSpace(len + 1);
    memcpy(newstr, s, len);
    newstr[len] = 0;
    return newstr;
}

void setIndex(uint8_t tableIdx, uint16_t idx, char const *name) {
    if (tableIdx >= INDEXTABLES)
        return;
    if (!itable[tableIdx])
        itable[tableIdx] = newIndexBlock();

    index_t *p;
    for (p = itable[tableIdx]; idx >= INDEXCHUNK; idx -= INDEXCHUNK) {
        if (!p->next)
            p->next = newIndexBlock();
        p = p->next;
    }
    if (strlen(name) <= MAXNAME)
        p->names[idx] = name;
    else {
        p->names[idx] = pstrdup(MAXNAME, name);
        /* rewrite end of truncated name, note requires removing const */
        char ending[9];
        strcpy((char *)p->names[idx] + MAXNAME - sprintf(ending, "..@%d", idx), ending);
    }
}

char const *getIndexName(uint8_t tableIdx, uint16_t idx) {
    if (tableIdx >= INDEXTABLES)
        return "Bad Index";
    index_t *p = itable[tableIdx];
    uint16_t i = idx;
    while (p && i >= INDEXCHUNK) {
        if (!p->next)
            p->next = newIndexBlock();
        p = p->next;
        i -= INDEXCHUNK;
    }
    if (!p->names[i]) {
        char tmp[7];
        int len     = sprintf(tmp, "@%d", idx);
        p->names[i] = pstrdup(len, tmp);
    }

    return p->names[i];
}
