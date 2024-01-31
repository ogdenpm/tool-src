#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAXMEM    0x10000 // 0-0xffff memory
#define MAXAPPEND 256
#define LXISP     0x31
#define JMP       0xc3
#define HEXBYTES  16


/* the use types for data */
enum { UNINITIALISED = 0, DATA, PADDING };

/* types for values */
enum { EOL, HEXVAL, STRING, SKIP, DEINIT, APPEND, START, BASE, INVALID, ERROR };
/* output file type */
enum { NOTSET, OMF85, HEX, BIN, IMAGE, MULTISET};
enum { BAD = -2, BADCRC, VALID };

// Intel AOMF85 record types
#define MODHDR     2
#define MODEND     4
#define MODCONTENT 6
#define MODEOF     0xe



extern int low;     // low address of load
extern int high;    // high address of load
extern int base;    // base address for binary load
extern int start;   // start address derived from load


extern uint8_t inMem[MAXMEM + MAXAPPEND];
extern uint8_t use[MAXMEM + MAXAPPEND];
extern uint8_t modhdr[38];

_Noreturn void usage(char *fmt, ...);
_Noreturn void error(char *fmt, ...);
void warning(char *fmt, ...);

bool loadFile(char *s);
void insertJmpEntry();
int saveFile(char *s, int oufFormat);


int parseHex(char *s, char **end);
void patchfile(char *s);