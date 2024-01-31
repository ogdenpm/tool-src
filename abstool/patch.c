#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "abstool.h"



typedef struct {
    uint8_t type;
    union {
        unsigned hval;
        char *str;
    };
    int repeatCnt;
} value_t;


char *skipSpc(char *s) {
    while (*s == ' ' || *s == '\t')
        s++;
    return s;
}



int parseHex(char *s, char **end) {
    int val = 0;
    s       = skipSpc(s);
    if (isxdigit(*s)) {
        while (isxdigit(*s)) {
            val = val * 16 + (isdigit(*s) ? *s - '0' : tolower(*s) - 'a' + 10);
            s++;
        }
    } else
        val = -1;
    *end = s;
    return val;
}

// scan the string starting at s for a value and optional repeat count
// sets the parsed information in *val (a value_t)
// returns a pointer to the next character to parse

char *getValue(char *s, value_t *val) {
    int hval;
    val->repeatCnt = -1;
    val->type      = ERROR;

    s              = skipSpc(s);
    if (strnicmp(s, "append", 6) == 0) {
        val->type = APPEND;
        return s + 6;
    }
    if (strnicmp(s, "start", 5) == 0) {
        if ((hval = parseHex(s + 5, &s)) >= 0) {
            val->type = START;
            val->hval = hval;
        }
        return s;
    }
    if ((hval = parseHex(s, &s)) >= 0) {
        val->type = HEXVAL;
        val->hval = hval;
    } else if (*s == '\'') {
        val->str = s + 1;
        while (*++s && *s != '\'') {
            if (*s == '\\' && s[1])
                s++;
        }
        if (!*s)
            return s;
        val->type = STRING;
    } else {
        if (*s == '-')
            val->type = DEINIT;
        else if (*s == '=')
            val->type = SKIP;
        else {
            val->type = !*s || *s == '\n' || *s == ';' ? EOL : INVALID;
            return *s ? s + 1 : s;
        }
        s++;
    }
    s = skipSpc(s);
    if (tolower(*s) == 'x') { // we have a repeat count
        if ((hval = parseHex(s + 1, &s)) >= 1)
            val->repeatCnt = hval;
        else
            val->type = ERROR;
    }
    return s;
}

unsigned fill(int addr, value_t *val, bool appending) {
    char *s;

    if (addr < low)
        low = addr;
    if (val->repeatCnt < 0)
        val->repeatCnt = 1;
    for (int i = 0; i < val->repeatCnt; i++) {
        if (addr >= MAXMEM) {
            fprintf(stderr, "appending past 0xFFFF\n");
            break;
        }
        switch (val->type) {
        case HEXVAL:
            inMem[addr] = val->hval;
            use[addr++] = appending ? PADDING : DATA;
            break;
        case SKIP:
            if (use[addr] == UNINITIALISED)
                fprintf(stderr, "Skipping uninitialised data at %04X\n", addr);
            addr++;
            break;
        case DEINIT:
            use[addr++] = UNINITIALISED;
            break;
        case STRING:
            for (s = val->str; *s && *s != '\'' && addr < MAXMEM;) {
                if (*s == '\\') {
                    static char const escchar[] = "abfnrtv'\"\\";
                    static char const mapchar[] = "\a\b\f\n\r\t\v'\"\\";
                    char const *t               = strchr(escchar, *++s);
                    int n                       = 0;
                    if (t) {
                        inMem[addr] = mapchar[t - escchar];
                        s++;
                    } else if (*s == 'x') {
                        if ((n = parseHex(s + 1, &s)) < 0) {
                            fprintf(stderr, "Warning: \\x missing value\n");
                            n = 0;
                        } else if (n > 255)
                            fprintf(stderr, "Warning: \\x%X is too large, using \\x%X\n", n,
                                    n & 0xff);
                        inMem[addr] = (uint8_t)n;
                    } else if (isdigit(*s)) {
                        for (int i = 0; i < 3 && '0' <= *s && *s <= '7'; s++)
                            n = n * 8 + *s - '0';
                        if (n > 255)
                            fprintf(stderr, "Warning: \\%o value is too large, using \\%o\n", n,
                                    n & 0xff);
                        inMem[addr] = (uint8_t)n;
                    } else {
                        if (isprint(*s))
                            fprintf(stderr, "Warning: invalid escaped char %c\n", *s);
                        else
                            fprintf(stderr, "Warning: invalid escaped char %02X\n", *s);
                        inMem[addr] = *s++;
                    }
                } else {
                    if (!isprint(*s))
                        fprintf(stderr, "warning invalid char %02X\n", *s);
                    inMem[addr] = *s++;
                }
                use[addr++] = appending ? PADDING : DATA;
            }
        }
    }
    if (addr > high)
        high = addr;
    return addr;
}

void patchfile(char *fname) {
    char line[256];
    value_t val;
    char *s;
    bool append = false;
    int addr;

    FILE *fp = fopen(fname, "rt");
    if (fp == NULL) {
        fprintf(stderr, "can't load patchfile (ignoring)\n");
        return;
    }

    while (fgets(line, 256, fp)) {
        for (s = line; *s; s++)
            if ((*s < ' ' && *s != '\r' && *s != '\n' && *s != '\t') || *(uint8_t *)s >= 0x80)
                error("Patch file contains binary data");

        s = getValue(line, &val);
        if (val.type == EOL || val.type == INVALID)
            continue;
        if (val.type == ERROR) {
            fprintf(stderr, "invalid patch line: %s", line);
            continue;
        }

        if (val.type == START) {
            start = val.hval;
            continue;
        } else if (append) {
            if (val.type == APPEND)
                s = getValue(s, &val);
        } else if (val.type == APPEND) {
            /* before append data may have been deleted so get the correct end */
            while (high > low && use[high - 1] == UNINITIALISED)
                high--;
            addr   = high;
            append = true;
            s      = getValue(s, &val);
        } else { // get the start address
            if (val.hval >= 0x10000) {
                fprintf(stderr, "Address (%04X) too big: %s", val.hval, line);
                continue;
            } else if (val.repeatCnt >= 0) {
                fprintf(stderr, "Address cannot have repeat count: %s", line);
                continue;
            }
            addr = val.hval;
            s    = getValue(s, &val);
        }

        // at this point addr has the insert point and val has the first value to put in
        bool ok = true;
        while (ok && val.type != EOL && val.type != INVALID && val.type != ERROR) {
            if (val.type == HEXVAL && val.hval >= 0x100) {
                fprintf(stderr, "bad hex value: %s", line);
                ok = false;
            } else if (append && (val.type == SKIP || val.type == DEINIT)) {
                fprintf(stderr, "'%c' invalid in append mode: %s", val.type == SKIP ? '=' : '-',
                        line);
                ok = false;
            } else if (val.type == APPEND || val.type == START) {
                fprintf(stderr, "%s only valid at start of line: %s",
                        val.type == APPEND ? "APPEND" : "START", line);
                ok = false;
            } else {
                addr = fill(addr, &val, append);
                s    = getValue(s, &val);
            }
        }
        if (val.type == ERROR)
            fprintf(stderr, "bad value: %s", line);
    }

    fclose(fp);
    // trim off any deleted data at start or end
    while (low < high && use[low] == UNINITIALISED)
        low++;
    while (high > low && use[high - 1] == UNINITIALISED)
        high--;
}
