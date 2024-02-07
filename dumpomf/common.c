
#include "omf.h"
#include <io.h>
#include <stdarg.h>
#include <ctype.h>

#define MAXOVER  10 /* maximum line malformed */
#define MINGAP   3  /* min gap between columns */
#define INDENT   8  /* indent of non record header lines */
#define LOCWIDTH 13 /* width of start of record inof 'XXXX:XX #nnn '*/

extern bool malformed;

static uint16_t pPos   = 0;                 /* current position in line */
static uint16_t cCol   = 0;                 /* current pos within column */
static uint16_t sPos   = 0;                 /* current start pos of column */
static uint16_t nCol   = 0;                 /* number of columns */
static uint16_t cWidth = MAXPOS - LOCWIDTH; /* width of a column */
static int16_t cEnd    = MAXPOS - LOCWIDTH; /* nominal end of line pos */

extern long start;

static int recCnt;
static char line[MAXPOS * 2]; /* line content */

int curField;
#define MAXFIELDS 10
uint16_t fieldTabs[MAXFIELDS];

uint16_t getCol() {
    return cCol;
}

void startCol(int n) {
    if (nCol != n || n <= 1) {
        displayLine(); /* flush any pending line */
        nCol   = n;
        cWidth = nCol ? (MAXPOS - INDENT) / nCol : MAXPOS - LOCWIDTH;
        cEnd   = nCol ? cWidth * nCol : MAXPOS - LOCWIDTH;
    } else {
        while ((sPos += cWidth) < cEnd && pPos + MINGAP > sPos)
            ;
        if (sPos >= cEnd)
            displayLine();
        else {
            while (pPos < sPos - 2)
                line[pPos++] = ' ';
            line[pPos++] = '|';
            line[pPos++] = ' ';
            cCol         = 0;
        }
    }
    if (n == 0 && recCnt++)
        putc('\n', dst);
    markRecPos();
    curField = -1;
}

void undoCol() {
    line[sPos] = 0;
}

/* only used for header line to fix record type info, even if col is undone */
void fixCol() {
    sPos = pPos;
}

void _add(char const *fmt, va_list args) {
    pPos += vsprintf(line + pPos, fmt, args);
    if (pPos >= cEnd + MAXOVER)
        splitLine();
    cCol = pPos - sPos;
}

void addAt(int col, char const *fmt, ...) {
    if (col) {
        do {
            line[pPos++] = ' ';
        } while (col > ++cCol);
    }
    va_list args;
    va_start(args, fmt);
    _add(fmt, args);
    va_end(args);
}

void addField(char const *fmt, ...) {
    if (nCol) {
        int col = ++curField < MAXFIELDS ? fieldTabs[curField] : 0;
        if (col > cCol || (col <= cCol && cCol && line[pPos - 1] != ' '))
            do {
                line[pPos++] = ' ';
            } while (col > ++cCol);
    } else if (cCol) {
        line[pPos++] = ' ';
        cCol++;
    }
    va_list args;
    va_start(args, fmt);
    _add(fmt, args);
    va_end(args);
}

void add(char const *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _add(fmt, args);
    va_end(args);
}

void splitLine() {
    if (nCol <= 1 || sPos == 0)
        displayLine(); /* TODO split on space if required */
    else {
        int colSplit   = sPos;
        int colOff     = pPos - sPos;
        int ch         = line[colSplit];
        line[colSplit] = 0; /* separate off the earlier columns */
        displayLine();
        line[colSplit] = ch;
        strcpy(line, line + colSplit); /* move down the original line and '\0' */
        cCol = colOff;                 /* fixup to insert point */
    }
}

void displayLine() {
    while (pPos && line[pPos - 1] == ' ')
        pPos--;
    line[pPos] = 0;
    if (*line) {
        if (nCol)
            fprintf(dst, "%*s%s\n", INDENT, "", line);
        else {
            fprintf(dst, "%04X:%02X #%u %s\n", start / 128, start % 128, recCnt, line);
            //nCol = 1;
        }
            *line = 0;
    }
    pPos = sPos = 0;
    cCol        = 0;
}

void Log(char const *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char logMsg[512];
    if (nCol)
        sprintf(logMsg, "%*s", INDENT, "");
    else
        fprintf(dst, "%04X:%02X =%u ", start / 128, start % 128, recCnt);

    vsprintf(logMsg + strlen(logMsg), fmt, args);
    strcat(logMsg, "\n");
    fputs(logMsg, dst);
    if (dst != stdout && !_isatty(_fileno(dst)))
        fputs(logMsg, stderr);
    va_end(args);
}

void invalidRecord(int type) {
    hexDump(0, false);
}

void hexDump(unsigned addr, bool showLoc) {
    unsigned rowAddr = addr & ~0xf;
    int idx          = addr & 0xf;
    char ascii[17]; /* 16 chars + '\0' */
    int i;
    uint16_t loc = 0;
    int dataCol  = 0;

    if (addr == 0) /* don't need offsets if address is 0 */
        showLoc = false;
    startCol(1);

    while (!atEndRec()) {
        if (showLoc) {
            add("%03X> ", loc);
            loc += 16;
        }
        add("%04X", rowAddr);
        if (!dataCol)
            dataCol = getCol() + 2;
        for (i = 0; i < 16 && !atEndRec(); i++) {
            if (i == 8)
                addAt(dataCol + 8 * HEXWIDTH + 1, "|");
            if (i >= idx) {
                uint8_t c = getu8();

                addAt(dataCol + i * HEXWIDTH + i / 4 + i / 8, "%02X", c);
                ascii[i] = ' ' <= c && c < 0x7f ? c : '.';
            }
        }
        ascii[i] = 0;
        addAt(dataCol + ASCIICOL + idx, "|%s|", ascii + idx);
        displayLine();
        rowAddr += 16;
        idx = 0;
    }
}

void oaddHeader(uint8_t cols, ofield_t const *fields) {
    if (malformed)
        return;
    for (int i = 0; i < cols; i++) {
        startCol(cols);
        for (ofield_t const *p = fields; p->label; p++)
            addAt(p->tabStop, p->label);
    }
}


int setFieldTabs(field_t const *fields) {
    int i, ts;
    memset(fieldTabs, 0, sizeof(fieldTabs));
    for (i = 0, ts = 0; i < MAXFIELDS && fields[i].label; i++) {
        fieldTabs[i] = ts;
        int lw       = strlen(fields[i].label);
        ts += lw > fields[i].width ? lw + 1 : fields[i].width + 1;
    }
    return ts;
}




void addFixedHeader(field_t const *fields) {
    if (malformed)
        return;
    setFieldTabs(fields);
    startCol(1);
    while(fields->label)
        addField("%s", (fields++)->label);
}


int addReptHeader(field_t const *fields) {
    if (malformed || atEndRec())
        return 1;
    int width    = setFieldTabs(fields) - 1 + MINGAP;
    uint8_t cols = (MAXPOS - INDENT + MINGAP) / width;
    if (cols == 0)
        cols = 1;
    for (int i = 0; i < cols; i++) {
        startCol(cols);
        for (int j = 0; fields[j].label; j++)
            addField("%s", fields[j].label);
    }
    return cols;
}


char const *hexStr(uint32_t n) {
    static char numstr[sizeof(uint32_t) * 2 + 3];
    sprintf(numstr, "0%X%s", n, n <= 9 ? "" : "H");
    return isdigit(numstr[1]) ? numstr + 1 : numstr;

}