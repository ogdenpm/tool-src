#ifndef PCOBJ_H

/*
PCOBJ -- definitions for generating INTEL format object decks

   modified:	by:		reason:
   ---------	---		-------

   93-may-05	TIS		
*/

#include <stdint.h>

/*
 * Library stuff
 */
typedef struct hash_entry {
    uint16_t     minor_class;
    uint16_t     minor_inc;
    uint16_t     major_class;
    uint16_t     major_inc;
} hash_entry;

typedef struct lib_header {
    uint8_t      cmd;
    uint16_t     length;
    uint32_t     dict_start;
    uint16_t     dict_size;
} lib_header;

enum {
    LIB_FULL_PAGE       = 0xff,
    LIB_NOT_FOUND       = 0,
    LIB_HEADER_REC      = 0xf0,
    LIB_TRAILER_REC     = 0xf1
};

enum {
    DIC_REC_SIZE        = 512U,     /* record size of dictionary entry */
    IS_2_BYTES          = 0x80      /* bit set indicating 2 byte value */
};

/*
 *  Combination Attributes
 */

enum {
    COMB_INVALID        = 0,
    COMB_ABOVEALL       = 1,
    COMB_ADDOFF         = 2,
    COMB_BAD            = 3,
    COMB_FOUR           = 4,
    COMB_STACK          = 5,
    COMB_COMMON         = 6,
    COMB_ALIGNTOP       = 7,

/*  Bits in FIXUPP records */

    FIXUPP_FIXUP        = 0x80,

    FIXDAT_FTHREAD      = 0x80,
    FIXDAT_TTHREAD      = 8,
    FIXDAT_PBIT         = 4,
    FIXDAT_MBIT         = 0x40,
    TRDDAT_DBIT         = 0x40,

/*
 *  INTEL Frame Specifiers
 */

    F_SEG               = 0,        /* segment index                    */
    F_GRP               = 1,        /* group index                      */
    F_EXT               = 2,        /* external index                   */
    F_ABS               = 3,        /* absolute frame number            */
    F_LOC               = 4,        /* frame containing location        */
    F_TARG              = 5,        /* frame same as target             */
    F_NONE              = 6,        /* no frame                         */

/*
 *  INTEL Target Specifiers
 */

    T_SEGWD             = 0,        /* segment index with displacement  */
    T_GRPWD             = 1,        /* group index with displacement    */
    T_EXTWD             = 2,        /* external index with displacement */
    T_ABSWD             = 3,        /* abs frame num with displacement  */
    T_SEG               = 4,        /* segment index, no displacement   */
    T_GRP               = 5,        /* group index, no displacement     */
    T_EXT               = 6,        /* external index, no displacement  */
    T_ABS               = 7,        /* abs frame num, no displacement   */

/*
 *  INTEL Segment Alignment Specifiers
 */

    ALIGN_ABS           = 0,        /* absolute segment - no alignment  */
    ALIGN_BYTE          = 1,        /* relocatable seg - byte aligned   */
    ALIGN_WORD          = 2,        /* relocatable seg - word aligned   */
    ALIGN_PARA          = 3,        /* relocatable seg - para aligned   */
    ALIGN_PAGE          = 4,        /* relocatable seg - page aligned   */
    ALIGN_DWORD         = 5,        /* relocatable seg - dword aligned  */
    ALIGN_LTRELOC       = 6,        /* load-time relocatable segment    */
/* this encountered in 386 object files only. */
    ALIGN_4KPAGE        = 6,        /* relocatable seg - 4k page aligned*/
    ALIGN_UNABS         = 5,        /* unnamed absolute segment         */

/*
 *  INTEL Group Specifiers
 */

    GRP_SEGIDX          = 0xff,     /* group segment index              */
    GRP_EXTIDX          = 0xfe,     /* group external index             */
    GRP_FULLNAME        = 0xfd,     /* full name indices                */
    GRP_LTLDATA         = 0xfb,     /* load time data info              */
    GRP_ADDR            = 0xfa,     /* load time addr for the group     */

/*
 *  INTEL Object Record Types
 */

    CMD_MIN_CMD         = 0x6e,     /* minimum cmd enum                 */
    CMD_RHEADR          = 0x6e,
    CMD_REGINT          = 0x70,
    CMD_REDATA          = 0x72,
    CMD_RIDATA          = 0x74,
    CMD_OVLDEF          = 0x76,
    CMD_ENDREC          = 0x78,
    CMD_BLKDEF          = 0x7a,     /* block definition record          */
    CMD_BLKEND          = 0x7c,     /* block end record                 */
    CMD_DEBSYM          = 0x7e,
    CMD_THEADR          = 0x80,     /* header record                    */
    CMD_LHEADR          = 0x82,
    CMD_PEDATA          = 0x84,
    CMD_PIDATA          = 0x86,
    CMD_COMENT          = 0x88,     /* comment record                   */
    CMD_MODEND          = 0x8a,     /* end of module record             */
    CMD_MODE32          = 0x8b,     /* 32-bit end of module record      */
    CMD_EXTDEF          = 0x8c,     /* import names record              */
    CMD_TYPDEF          = 0x8e,     /* type definition record           */
    CMD_PUBDEF          = 0x90,     /* export names record              */
    CMD_PUBD32          = 0x91,     /* 32-bit export names record       */
    CMD_LOCSYM          = 0x92,
    CMD_LINNUM          = 0x94,     /* line number record               */
    CMD_LINN32          = 0x95,     /* 32-bit line number record.       */
    CMD_LNAMES          = 0x96,     /* list of names record             */
    CMD_SEGDEF          = 0x98,     /* segment definition record        */
    CMD_SEGD32          = 0x99,     /* 32-bit segment definition        */
    CMD_GRPDEF          = 0x9a,     /* group definition record          */
    CMD_FIXUP           = 0x9c,     /* relocation record                */
    CMD_FIXUPP          = 0x9c,     /* for those who stuttttttter       */
    CMD_FIXU32          = 0x9d,     /* 32-bit relocation record         */
    CMD_LEDATA          = 0xa0,     /* object record                    */
    CMD_LEDA32          = 0xa1,     /* 32-bit object record             */
    CMD_LIDATA          = 0xa2,     /* repeated data record             */
    CMD_LIDA32          = 0xa3,     /* 32-bit repeated data record      */
    CMD_LIBHED          = 0xa4,
    CMD_LIBNAM          = 0xa6,
    CMD_LIBLOC          = 0xa8,
    CMD_LIBDIC          = 0xaa,
    CMD_COMDEF          = 0xb0,     /* communal definition              */
    CMD_BAKPAT          = 0xb2,	    /* backpatch record */
    CMD_BAKP32          = 0xb3,
    CMD_STATIC_EXTDEF   = 0xb4, 
    CMD_STATIC_EXTD32   = 0xb5,
    CMD_STATIC_PUBDEF   = 0xb6,
    CMD_STATIC_PUBD32   = 0xb7,
    CMD_STATIC_COMDEF   = 0xb8,
    CMD_CEXTDF		= 0xbc,	    /* external reference to a COMDAT */
    CMD_COMDAT		= 0xc2,	    /* initialized communal data record */
    CMD_COMD32		= 0xc3,	    /* initialized 32-bit communal data record */
    CMD_LINSYM		= 0xc4,	    /* LINNUM for a COMDAT */
    CMD_LINS32		= 0xc5,	    /* 32-bit LINNUM for a COMDAT */
    CMD_ALIAS		= 0xc6,	    /* alias definition record		*/
    CMD_NBKPAT		= 0xc8,     /* named backpatch record */
    CMD_NBKP32		= 0xc9,     /* 32-bit named backpatch record */
    CMD_LLNAME		= 0xca,	    /* a "local" lnames */
    CMD_LLNAMES		= 0xca,
    CMD_VERNUM		= 0xcc,	    /* TIS version number record	*/
    CMD_VENDEXT		= 0xce,	    /* TIS vendor extension record	*/
    CMD_MAX_CMD         = 0xce      /* maximum cmd enum                 */
};

enum {
    LOC_OFFSET_LO       = 0,        /* relocate lo byte of offset       */
    LOC_OFFSET          = 1,        /* relocate offset                  */
    LOC_BASE            = 2,        /* relocate segment                 */
    LOC_BASE_OFFSET     = 3,        /* relocate segment and offset      */
    LOC_OFFSET_HI       = 4,        /* relocate hi byte of offset       */
    LOC_MS_LINK_OFFSET  = 5,        /* like OFFSET but loader resolved  */
    LOC_OFFSET_32       = 5,        /* relocate 32-bit offset           */
    LOC_BASE_OFFSET_32  = 6,        /* relocate segment and 32-bit offset*/
    LOC_MS_OFFSET_32    = 9,        /* MS 32-bit offset                 */
    LOC_MS_BASE_OFFSET_32= 11,      /* MS 48-bit pointer                */
    LOC_MS_LINK_OFFSET_32= 13       /* like OFFSET_32 but loader resolved*/
};

typedef struct obj_record {
    uint8_t      command;
    uint16_t     length;
} obj_record;

typedef struct obj_name {
    uint8_t      len;
    char        name[ 1 ];
} obj_name;

/*
    Comment classes
*/
enum {
    CMT_LANGUAGE_TRANS  = 0x00, /* Language translator comment          */
    CMT_INTEL_COPYRIGHT = 0x01, /* INTEL Copyright record               */
    CMT_MS_PADDING      = 0x01, /* Microsoft uses this for padding      */
    CMT_WAT_PROC_MODEL  = 0x9b, /* Watcom processor & model info        */
    CMT_MS_DOS_VERSION  = 0x9c, /* obsolete                             */
    CMT_MS_PROC_MODEL   = 0x9d, /* Microsoft processor & model info     */
    CMT_DOSSEG          = 0x9e, /* DOSSEG directive                     */
    CMT_DEFAULT_LIBRARY = 0x9f, /* Default library cmd                  */
    CMT_DLL_ENTRY       = 0xa0, /* MS extension (misleading name!)      */
    CMT_MS_OMF          = 0xa1, /* Microsoft's brand of OMF flag        */
    CMT_MS_END_PASS_1   = 0xa2, /* MS end of linker pass 1              */
    CMT_LIBMOD          = 0xa3, /* Record specifying name of object     */
    CMT_EXESTR		= 0xa4,	/* Executable string                    */
    CMT_INCERR		= 0xa6,	/* Incremental Compilation Error        */
    CMT_NOPAD		= 0xa7,	/* No segment padding                   */
    CMT_WKEXT		= 0xa8,	/* Weak external record                 */
    CMT_LZEXT		= 0xa9,	/* Lazy external record                 */
    CMT_EASY_OMF        = 0xaa, /* Easy OMF signature record            */
    CMT_DEPENDENCY	= 0xe9, /* Borland dependency record		*/
    CMT_DISASM_DIRECTIVE= 0xfd, /* Directive to disassemblers           */
    CMT_LINKER_DIRECTIVE= 0xfe, /* Linker directive                     */
    CMT_COMPILER_OPTIONS= 0xff, /* Microsoft: incremental compiler opts */
    CMT_SOURCE_NAME     = 0xff  /* name of the source file              */
};

/*
 * Comment Class Subtype
 */
enum {
    DLL_IMPDEF      = 0x01, /* Subtype IMPDEF of comment class DLL  */
    DLL_EXPDEF      = 0x02, /* Subtype EXPDEF of comment class DLL  */
};

#define EASY_OMF_SIGNATURE  "80386"

/*
    this is the data that is given in the object file to determine the
    target processor.
*/

typedef struct {
    char    processor;
    char    mem_model;
    char    unknown;
    char    emulation;
} cpu_data;

/*
    Linker directives (mostly WLINK directives)
*/
enum {
    LDIR_SOURCE_LANGUAGE= 'D',  /* dbg maj/min and source language      */
    LDIR_DEFAULT_LIBRARY= 'L',  /* default library cmd                  */
    LDIR_OPT_FAR_CALLS  = 'O',  /* optimize far calls/jmps for this seg */
    LDIR_OPT_UNSAFE     = 'U',  /* far call optimization unsafe for fixup*/
    LDIR_VF_TABLE_DEF	= 'V',	/* virtual function table lazy extdef   */
    LDIR_VF_PURE_DEF	= 'P',	/* as above for pure functions */
    LDIR_VF_REFERENCE	= 'R'	/* virtual function reference */
};

/*
    Microsoft coment class A0 extensions
*/
enum {
    MOMF_IMPDEF = 1,
    MOMF_EXPDEF = 2,
    MOMF_INCDEF = 3,
    MOMF_PROT_LIB = 4,
    MOMF_LNKDIR = 5,
    MOMF_BIG_ENDIAN = 6
};

/*
    Disasm directives
*/
enum {
/*
    DDIR_SCAN_TABLE is used by the code generator to indicate data in a
    code segment.  i.e., scan tables generated for switch()s, floating point
    constants and string constants.  The 'S' is followed by a segment index,
    then the start and end+1 offsets into the segment which are words in
    regular object files, and longs in EasyOMF and Microsoft 386.
    If the segment index is zero, then it is followed by a LNAME index which
    identifies the COMDAT symbol that the scan table belongs to.
*/
    DDIR_SCAN_TABLE_32  = 'S',
    DDIR_SCAN_TABLE     = 's'
};

/*
    COMDEF types
*/
enum {
    COMDEF_FAR          = 0x61, /* FAR variable                         */
    COMDEF_NEAR         = 0x62, /* NEAR variable                        */
    COMDEF_LEAF_SIZE    = 0x80, /* bit set if size > 0x7f               */
    COMDEF_LEAF_2       = 0x81, /* 2 byte size field                    */
    COMDEF_LEAF_3       = 0x84, /* 3 byte size field                    */
    COMDEF_LEAF_4       = 0x88  /* 4 byte size field                    */
};

enum {
/*
    COMDAT flags
*/
    COMDAT_CONTINUE	= 0x01, /* continuation of previous COMDAT */
    COMDAT_ITERATED	= 0x02, /* LIDATA form of COMDAT */
    COMDAT_LOCAL	= 0x04, /* COMDAT is local to this module */
/*
    COMDAT allocation type
*/
    COMDAT_ALLOC_MASK	= 0x0f,
    COMDAT_EXPLICIT	= 0x00,	/* in given segment */
    COMDAT_FAR_CODE	= 0x01, /* allocate CODE use16 segment */
    COMDAT_FAR_DATA	= 0x02, /* allocate DATA use16 segment */
    COMDAT_CODE32	= 0x03,	/* allocate CODE use32 segment */
    COMDAT_DATA32	= 0x04, /* allocate DATA use32 segment */
/*
    COMDAT selection criteria
*/
    COMDAT_MATCH_MASK	= 0xf0,
    COMDAT_MATCH_NONE	= 0x00,	/* don't match anyone */
    COMDAT_MATCH_ANY	= 0x10, /* pick any instance */
    COMDAT_MATCH_SAME	= 0x20, /* must be same size */
    COMDAT_MATCH_EXACT  = 0x30, /* must be exact match */
/*
    COMDAT alignment
*/
    COMDAT_ALIGN_SEG	= 0x00, /* align from SEGDEF */
    COMDAT_ALIGN_BYTE	= 0x01,
    COMDAT_ALIGN_WORD	= 0x02,
    COMDAT_ALIGN_PARA	= 0x03,
    COMDAT_ALIGN_4K	= 0x04,
    COMDAT_ALIGN_DWORD	= 0x05
};


/*
    PharLap/EASY_OMF segment attribute byte
*/
enum {
    EASY_USE32_FIELD    = 0x04, /* USE32 bit                            */
    EASY_PROTECT_FIELD  = 0x03, /* protection field                     */
    EASY_READ_ONLY      = 0x00, /* read-only                            */
    EASY_EXEC_ONLY      = 0x01, /* exec-only                            */
    EASY_EXEC_READ      = 0x02, /* exec-read                            */
    EASY_READ_WRITE     = 0x03  /* read-write                           */
};

#define PCOBJ_H
#endif
