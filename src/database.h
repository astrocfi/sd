/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */


/* We customize the necessary declarations for functions
   that don't return.  Alas, this requires something in front
   and something in back. */

#if defined(__GNUC__)
#define NORETURN1
#define NORETURN2 __attribute__ ((noreturn))
#elif defined(WIN32)
// This declspec only works for VC++ version 6.
#define NORETURN1 /*__declspec(noreturn)*/
#define NORETURN2
#else
#define NORETURN1
#define NORETURN2
#endif

/* We used to do some stuff to cater to compiler vendors
   (e.g. Sun Microsystems) that couldn't be bothered to
   do the "const" attribute correctly.  We no longer have
   any patience with such things. */

#define Const const


/* We use "Private" on procedures and "static" on variables.  It makes things clearer. */
#define Private static

/* We would like "veryshort" to be a signed char, but not all compilers are fully ANSI compliant.
   The IBM AIX compiler, for example, considers char to be unsigned.  The switch "NO_SIGNED_CHAR"
   alerts us to that fact.  The configure script has checked this for us. */
#ifdef NO_SIGNED_CHAR
typedef short veryshort;
#else
typedef char veryshort;
#endif

/* We would like to think that we will always be able to count on compilers to do the
   right thing with "int" and "long int" and so on.  What we would really like is
   for compilers to be counted on to make "int" at least 32 bits, because we need
   32 bits in many places.  However, some compilers don't, so we have to use
   "long int" or "unsigned long int".  We think that all compilers we deal with
   will do the right thing with that, but, just in case, we use a typedef.

   The type "uint32" must be an unsigned integer of at least 32 bits.
   The type "uint16" must be an unsigned integer of at least 16 bits.

   Note also:  There are many places in the program (not just in database.h and sd.h)
   where the suffix "UL" is put on constants that are intended to be of type "uint32".
   If "uint32" is changed to anything other than "unsigned long int", it may be
   necessary to change all of those. */

typedef unsigned long int uint32;
typedef unsigned short int uint16;
typedef unsigned char uint8;
typedef int long_boolean;
typedef Const char *Cstring;

#define TRUE 1
#define FALSE 0

/* These are written as the first two halfwords of the binary database file.
   The format version is not related to the version of the program or database.
   It is used only to make sure that the "mkcalls" program that compiled
   the database and the "sd" program that reads it are in sync with each
   other, as indicated by the version of this file.  Whenever we change
   anything in this file that could cause an incompatibility, we bump the
   database format version. */

#define DATABASE_MAGIC_NUM 21316
#define DATABASE_FORMAT_VERSION 148

/* BEWARE!!  These must track the items in "tagtabinit" in dbcomp.c . */
typedef enum {
   base_call_unused,
   base_call_null,
   base_call_null_second,
   base_call_basetag0,
   base_call_armturn_34,
   base_call_ends_shadow,
   base_call_chreact_1,
   base_call_makepass_1,
   base_call_scoottowave,
   base_call_backemup,
   base_call_circulate,
   base_call_trade,
   base_call_check_cross_counter,
   base_call_lockit,
   base_call_disband1,
   base_call_slither,
      /* the next "NUM_TAGGER_CLASSES" (that is, 4) must be a consecutive group. */
   base_call_tagger0,
   base_call_tagger1_noref,
   base_call_tagger2_noref,
   base_call_tagger3_noref,
   base_call_circcer,
   base_call_turnstar_n
} base_call_index;
#define num_base_call_indices (((int) base_call_turnstar_n)+1)


/* Number of items in the above list. */
#ifdef DONTUSETHESEANYMORE
#define NUM_BASE_CALLS 16

#define BASE_CALL_NULL        1
#define BASE_CALL_NULL_SECOND 2
#define BASE_CALL_CAST_3_4    3
#define BASE_CALL_ENDS_SHADOW 4
#define BASE_CALL_BACKEMUP    7
#define BASE_CALL_CIRCULATE   8
#define BASE_CALL_TRADE       9
/* The next "NUM_TAGGER_CLASSES" (that is, 4) must be a consecutive group. */
#define BASE_CALL_TAGGER0    10
#define BASE_CALL_TAGGER1    11
#define BASE_CALL_TAGGER2    12
#define BASE_CALL_TAGGER3    13
#define BASE_CALL_CIRCCER    14
#define BASE_CALL_TURNSTAR_N 15
#endif

/* BEWARE!!  This list must track the tables "flagtabh", "defmodtabh",
   "forcetabh", and "altdeftabh" in dbcomp.c .  The "K" items also track
   the tables "mxntabforce", "nxntabforce", "nxntabplain", "mxntabplain",
   and "reverttabplain" in dbcomp.c .

   These are the infamous "heritable flags".  They are used in generally
   corresponding ways in the "callflagsh" word of a top level callspec_block,
   the "modifiersh" word of a "by_def_item", and the "modifier_seth" word of a
   "calldef_block", and the "cmd_final_flags.herit" of a setup with its command block.
*/

static Const uint32 INHERITFLAG_DIAMOND    = 0x00000001UL;
static Const uint32 INHERITFLAG_REVERSE    = 0x00000002UL;
static Const uint32 INHERITFLAG_LEFT       = 0x00000004UL;
static Const uint32 INHERITFLAG_FUNNY      = 0x00000008UL;
#define             INHERITFLAG_INTLK        0x00000010UL
static Const uint32 INHERITFLAG_MAGIC      = 0x00000020UL;
static Const uint32 INHERITFLAG_GRAND      = 0x00000040UL;
static Const uint32 INHERITFLAG_12_MATRIX  = 0x00000080UL;
static Const uint32 INHERITFLAG_16_MATRIX  = 0x00000100UL;
static Const uint32 INHERITFLAG_CROSS      = 0x00000200UL;
#define             INHERITFLAG_SINGLE       0x00000400UL
static Const uint32 INHERITFLAG_SINGLEFILE = 0x00000800UL;
static Const uint32 INHERITFLAG_HALF       = 0x00001000UL;
static Const uint32 INHERITFLAG_YOYO       = 0x00002000UL;
static Const uint32 INHERITFLAG_STRAIGHT   = 0x00004000UL;
static Const uint32 INHERITFLAG_TWISTED    = 0x00008000UL;
static Const uint32 INHERITFLAG_LASTHALF   = 0x00010000UL;
static Const uint32 INHERITFLAG_FRACTAL    = 0x00020000UL;

/* This is a 3 bit field. */
#define             INHERITFLAG_MXNMASK      0x001C0000UL
/* This is its low bit. */
#define             INHERITFLAG_MXNBIT       0x00040000UL

/* These 4 things are the choices available inside. */
#define             INHERITFLAGMXNK_1X2      0x00040000UL
#define             INHERITFLAGMXNK_2X1      0x00080000UL
#define             INHERITFLAGMXNK_1X3      0x000C0000UL
#define             INHERITFLAGMXNK_3X1      0x00100000UL

/* This is a 3 bit field. */
#define             INHERITFLAG_NXNMASK      0x00E00000UL
/* This is its low bit. */
#define             INHERITFLAG_NXNBIT       0x00200000UL

/* These 7 things are the choices available inside. */
#define             INHERITFLAGNXNK_2X2      0x00200000UL
#define             INHERITFLAGNXNK_3X3      0x00400000UL
#define             INHERITFLAGNXNK_4X4      0x00600000UL
#define             INHERITFLAGNXNK_5X5      0x00800000UL
#define             INHERITFLAGNXNK_6X6      0x00A00000UL
#define             INHERITFLAGNXNK_7X7      0x00C00000UL
#define             INHERITFLAGNXNK_8X8      0x00E00000UL

/* This is a 3 bit field. */
#define             INHERITFLAG_REVERTMASK   0x07000000UL
/* This is its low bit. */
#define             INHERITFLAG_REVERTBIT    0x01000000UL

/* These 7 things are the choices available inside. */
#define             INHERITFLAGRVRTK_REVERT  0x01000000UL
#define             INHERITFLAGRVRTK_REFLECT 0x02000000UL
#define             INHERITFLAGRVRTK_RVF     0x03000000UL
#define             INHERITFLAGRVRTK_RFV     0x04000000UL
#define             INHERITFLAGRVRTK_RVFV    0x05000000UL
#define             INHERITFLAGRVRTK_RFVF    0x06000000UL
#define             INHERITFLAGRVRTK_RFF     0x07000000UL


/* BEWARE!!  This list must track the table "flagtab1" in dbcomp.c .
   These flags go into the "callflags1" word of a callspec_block,
   and the "topcallflags1" word of the parse_state. */

/* This is a 2 bit field -- VISIBLE_FRACTION_BIT tells where its low bit lies. */
static Const uint32 CFLAG1_VISIBLE_FRACTION_MASK     = 0x00000003UL;
static Const uint32 CFLAG1_VISIBLE_FRACTION_BIT      = 0x00000001UL;

static Const uint32 CFLAG1_12_16_MATRIX_MEANS_SPLIT  = 0x00000004UL;
static Const uint32 CFLAG1_IMPRECISE_ROTATION        = 0x00000008UL;
static Const uint32 CFLAG1_SPLIT_LIKE_DIXIE_STYLE    = 0x00000010UL;
static Const uint32 CFLAG1_PARALLEL_CONC_END         = 0x00000020UL;
static Const uint32 CFLAG1_TAKE_RIGHT_HANDS          = 0x00000040UL;
static Const uint32 CFLAG1_IS_STAR_CALL              = 0x00000080UL;
static Const uint32 CFLAG1_SPLIT_LARGE_SETUPS        = 0x00000100UL;
static Const uint32 CFLAG1_FUDGE_TO_Q_TAG            = 0x00000200UL;

/* This is a 3 bit field. */
static Const uint32 CFLAG1_STEP_REAR_MASK            = 0x00001C00UL;
/* Here are the 3 bits. */
static Const uint32 CFLAG1_STEP_TO_WAVE              = 0x00000400UL;
static Const uint32 CFLAG1_REAR_BACK_FROM_R_WAVE     = 0x00000800UL;
static Const uint32 CFLAG1_REAR_BACK_FROM_QTAG       = 0x00001000UL;

static Const uint32 CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK = 0x00002000UL;

/* This is a 3 bit field -- NUMBER_BIT tells where its low bit lies. */
static Const uint32 CFLAG1_NUMBER_MASK               = 0x0001C000UL;
static Const uint32 CFLAG1_NUMBER_BIT                = 0x00004000UL;

static Const uint32 CFLAG1_SEQUENCE_STARTER          = 0x00020000UL;
static Const uint32 CFLAG1_SPLIT_LIKE_SQUARE_THRU    = 0x00040000UL;
static Const uint32 CFLAG1_DISTRIBUTE_REPETITIONS    = 0x00080000UL;
static Const uint32 CFLAG1_DONT_USE_IN_RESOLVE       = 0x00100000UL;
static Const uint32 CFLAG1_DONT_USE_IN_NICE_RESOLVE  = 0x00200000UL;
static Const uint32 CFLAG1_YIELD_IF_AMBIGUOUS        = 0x00400000UL;
static Const uint32 CFLAG1_NO_ELONGATION_ALLOWED     = 0x00800000UL;

/* This is a 3 bit field -- BASE_TAG_CALL_BIT tells where its low bit lies. */
static Const uint32 CFLAG1_BASE_TAG_CALL_MASK        = 0x07000000UL;
static Const uint32 CFLAG1_BASE_TAG_CALL_BIT         = 0x01000000UL;

static Const uint32 CFLAG1_BASE_CIRC_CALL            = 0x08000000UL;
static Const uint32 CFLAG1_ENDS_TAKE_RIGHT_HANDS     = 0x10000000UL;
static Const uint32 CFLAG1_FUNNY_MEANS_THOSE_FACING  = 0x20000000UL;
static Const uint32 CFLAG1_ONE_PERSON_CALL           = 0x40000000UL;
static Const uint32 CFLAG1_PRESERVE_Z_STUFF          = 0x80000000UL;

/* These are the continuation of the "CFLAG1" bits, that have to overflow into this word.
   They must lie in the top 8 bits for now. */
static Const uint32 CFLAG2_YOYO_FRACTAL_NUM          = 0x01000000UL;
static Const uint32 CFLAG2_CAN_BE_FAN                = 0x02000000UL;

/* Beware!!  This list must track the table "matrixcallflagtab" in dbcomp.c . */

#define MTX_USE_SELECTOR           0x01
#define MTX_STOP_AND_WARN_ON_TBONE 0x02
#define MTX_TBONE_IS_OK            0x04
#define MTX_IGNORE_NONSELECTEES    0x08
#define MTX_MUST_FACE_SAME_WAY     0x10
#define MTX_FIND_JAYWALKERS        0x20
#define MTX_BOTH_SELECTED_OK       0x40
#define MTX_FIND_SQUEEZERS         0x80
#define MTX_FIND_SPREADERS         0x100
#define MTX_USE_VEER_DATA          0x200
#define MTX_USE_NUMBER             0x400



/* These definitions help to encode things in the "qualifierstuff" field
   of a call definition.  That field is an unsigned 16 bit integer.  The low
   7 bits are a cast of the qualifier kind itself (hence we allow 127
   nontrivial qualifiers).  The high 9 bits are as follows. */

/* These two must be consecutive for encoding in the "assump_both" field. */
#define QUALBIT__LEFT           0x8000U
#define QUALBIT__RIGHT          0x4000U
#define QUALBIT__LIVE           0x2000U
#define QUALBIT__TBONE          0x1000U
#define QUALBIT__NTBONE         0x0800U
/* A 4 bit field.  If nonzero, there is a number requirement, and the field is that number plus 1. */
#define QUALBIT__NUM_MASK       0x0780U
#define QUALBIT__NUM_BIT        0x0080U
#define QUALBIT__QUAL_CODE      0x007FU



/* BEWARE!!  This list must track the table "leveltab" in dbcomp.c . */
/* BEWARE!!  This list must track the table "getout_strings" in sdtables.c . */
/* BEWARE!!  This list must track the table "filename_strings" in sdtables.c . */
/* BEWARE!!  This list must track the table "level_threshholds" in sdtables.c . */
/* BEWARE!!  This list must track the table "higher_acceptable_level" in sdtables.c . */

typedef enum {
   l_mainstream,
   l_plus,
   l_a1,
   l_a2,
   l_c1,
   l_c2,
   l_c3a,
   l_c3,
   l_c3x,
   l_c4a,
   l_c4,
   l_c4x,
   l_dontshow,
   l_nonexistent_concept
} dance_level;

/* These are the states that people can be in, and the "ending setups" that can appear
   in the call data base. */
/* BEWARE!!  This list must track the array "estab" in dbcomp.c . */
/* BEWARE!!  This list must track the array "setup_attrs" in sdtables.c . */
/* BEWARE!!  This list must track the array "split_lists" in sdtables.c . */
/* BEWARE!!  The procedure "merge_setups" canonicalizes pairs of setups by their
   order in this list, and will break if it is re-ordered randomly.  See the comments
   there before changing the order of existing setups. In general, keep small setups
   first, particularly 4-person setups before 8-person setups. */

typedef enum {
   nothing,
   s1x1,
   s1x2,
   s1x3,
   s2x2,
   s1x4,
   sdmd,
   s_star,
   s_trngl,
   s_trngl4,
   s_bone6,
   s_short6,
   s1x6,
   s2x3,
   s_1x2dmd,
   s_2x1dmd,
   s_qtag,
   s_bone,
   s1x8,
   slittlestars,
   s_2stars,
   s1x3dmd,
   s3x1dmd,
   s_spindle,
   s_hrglass,
   s_dhrglass,
   s_hyperglass,
   s_crosswave,
   s2x4,
   s2x5,
   s_rigger,
   s3x4,
   s2x6,
   s_d3x4,
   s1p5x8,   /* internal use only */
   s2x8,
   s4x4,
   s1x10,
   s1x12,
   s1x14,
   s1x16,
   s_c1phan,
   s_hyperbone,   /* internal use only */
   s_bigblob,
   s_ptpd,
   s3dmd,
   s4dmd,
   s3ptpd,
   s4ptpd,
   s_wingedstar,
   s_wingedstar12,
   s_wingedstar16,
   s_barredstar,
   s_barredstar12,
   s_barredstar16,
   s_galaxy,
   s3x6,
   s3x8,
   s4x5,
   s4x6,
   s2x10,
   s2x12,
   sdeepqtg,
   sdeepbigqtg,
   sdeepxwv,
   s3oqtg,
   s_thar,
   s_alamo,
   sx4dmd,   /* These are too big to actually represent -- */
   s8x8,     /* we don't let them out of their cage. */
   sx1x16,
   sfat2x8,  /* Same here.  These are big setups that are the size of 4x8's, */
   swide4x4, /* but only have 16 people.  The reason is to prevent loss of phantoms. */
   s_323,
   s_343,
   s_525,
   s_545,
   sh545,
   s_3mdmd,
   s_3mptpd,
   s_4mdmd,
   s_4mptpd,
   sbigh,
   sbigx,
   sbigbigh,
   sbigbigx,
   sbigrig,
   sbighrgl,
   sbigdhrgl,
   sbigbone,
   sbigdmd,
   sbigptpd,
   s_dead_concentric,
   s_normal_concentric
} setup_kind;

/* These are the "beginning setups" that can appear in the call data base. */
/* BEWARE!!  This list must track the array "sstab" in dbcomp.c . */
/* BEWARE!!  This list must track the array "begin_sizes" in mkcalls.c . */
/* BEWARE!!  This list must track the array "begin_sizes" in sdtables.c . */

typedef enum {
   b_nothing,
   b_1x1,
   b_1x2,
   b_2x1,
   b_1x3,
   b_3x1,
   b_2x2,
   b_dmd,
   b_pmd,
   b_star,
   b_trngl,
   b_ptrngl,
   b_trngl4,
   b_ptrngl4,
   b_bone6,
   b_pbone6,
   b_short6,
   b_pshort6,
   b_1x2dmd,
   b_p1x2dmd,
   b_2x1dmd,
   b_p2x1dmd,
   b_qtag,
   b_pqtag,
   b_bone,
   b_pbone,
   b_rigger,
   b_prigger,
   b_2stars,
   b_p2stars,
   b_spindle,
   b_pspindle,
   b_hrglass,
   b_phrglass,
   b_dhrglass,
   b_pdhrglass,
   b_crosswave,
   b_pcrosswave,
   b_1x4,
   b_4x1,
   b_1x8,
   b_8x1,
   b_2x4,
   b_4x2,
   b_2x3,
   b_3x2,
   b_2x5,
   b_5x2,
   b_1x6,
   b_6x1,
   b_3x4,
   b_4x3,
   b_2x6,
   b_6x2,
   b_d3x4,
   b_d4x3,
   b_2x8,
   b_8x2,
   b_4x4,
   b_1x10,
   b_10x1,
   b_1x12,
   b_12x1,
   b_1x14,
   b_14x1,
   b_1x16,
   b_16x1,
   b_c1phan,
   b_galaxy,
   b_3x6,
   b_6x3,
   b_3x8,
   b_8x3,
   b_4x5,
   b_5x4,
   b_4x6,
   b_6x4,
   b_2x10,
   b_10x2,
   b_2x12,
   b_12x2,
   b_deepqtg,
   b_pdeepqtg,
   b_deepbigqtg,
   b_pdeepbigqtg,
   b_deepxwv,
   b_pdeepxwv,
   b_3oqtg,
   b_p3oqtg,
   b_thar,
   b_alamo,
   b_ptpd,
   b_pptpd,
   b_1x3dmd,
   b_p1x3dmd,
   b_3x1dmd,
   b_p3x1dmd,
   b_3dmd,
   b_p3dmd,
   b_4dmd,
   b_p4dmd,
   b_3ptpd,
   b_p3ptpd,
   b_4ptpd,
   b_p4ptpd,
   b_323,
   b_p323,
   b_343,
   b_p343,
   b_525,
   b_p525,
   b_545,
   b_p545,
   bh545,
   bhp545,
   b_3mdmd,
   b_p3mdmd,
   b_3mptpd,
   b_p3mptpd,
   b_4mdmd,
   b_p4mdmd,
   b_4mptpd,
   b_p4mptpd,
   b_bigh,
   b_pbigh,
   b_bigx,
   b_pbigx,

   b_bigbigh,
   b_pbigbigh,
   b_bigbigx,
   b_pbigbigx,

   b_bigrig,
   b_pbigrig,
   b_bighrgl,
   b_pbighrgl,
   b_bigdhrgl,
   b_pbigdhrgl,
   b_bigbone,
   b_pbigbone,
   b_bigdmd,
   b_pbigdmd,
   b_bigptpd,
   b_pbigptpd
} begin_kind;

/* These bits are used in the "callarray_flags" field of a "callarray".
   There is room for 21 of them. */

/* This one must be 1!!!! */
#define CAF__ROT                    0x1
#define CAF__FACING_FUNNY           0x2
/* Next one says this is concentrically defined --- the "end_setup" slot
   has the centers' end setup, and there is an extra slot with the ends' end setup. */
#define CAF__CONCEND                0x4
/* Next one meaningful only if previous one is set. */
#define CAF__ROT_OUT                0x8
/* This is a 3 bit field. */
#define CAF__RESTR_MASK            0x70
/* These next 5 are the nonzero values it can have. */
#define CAF__RESTR_UNUSUAL         0x10
#define CAF__RESTR_FORBID          0x20
#define CAF__RESTR_RESOLVE_OK      0x30
#define CAF__RESTR_CONTROVERSIAL   0x40
#define CAF__RESTR_BOGUS           0x50
#define CAF__PREDS                 0x80
#define CAF__NO_CUTTING_THROUGH   0x100
#define CAF__LATERAL_TO_SELECTEES 0x200
#define CAF__VACATE_CENTER        0x400
#define CAF__OTHER_ELONGATE       0x800
#define CAF__SPLIT_TO_BOX        0x1000
#define CAF__REALLY_WANT_DIAMOND 0x2000

/* BEWARE!!  This list must track the array "qualtab" in dbcomp.c . */
typedef enum {
   cr_none,                /* Qualifier only. */
   cr_alwaysfail,          /* Restriction only. */
   cr_wave_only,
   cr_wave_unless_say_2faced, /* Qualifier only. */
   cr_all_facing_same,
   cr_1fl_only,
   cr_2fl_only,
   cr_3x3_2fl_only,
   cr_4x4_2fl_only,
   cr_leads_only,          /* Restriction only. */
   cr_trailers_only,       /* Restriction only. */
   cr_couples_only,
   cr_3x3couples_only,
   cr_4x4couples_only,
   cr_ckpt_miniwaves,      /* Restriction only. */
   cr_ctr_miniwaves,       /* Restriction only. */
   cr_ctr_couples,         /* Restriction only. */
   cr_awkward_centers,     /* Restriction only. */
   cr_dmd_same_pt,         /* Qualifier only. */
   cr_dmd_facing,          /* Qualifier only. */
   cr_diamond_like,
   cr_qtag_like,
   cr_pu_qtag_like,
   cr_reg_tbone,
   cr_gen_qbox,            /* Qualifier only. */
   cr_nice_diamonds,       /* Restriction only. */
   cr_magic_only,
   cr_li_lo,               /* Qualifier only. */
   cr_ctrs_in_out,         /* Qualifier only. */
   cr_indep_in_out,        /* Qualifier only. */
   cr_miniwaves,
   cr_not_miniwaves,       /* Qualifier only. */
   cr_as_couples_miniwaves,/* Qualifier only. */
   cr_true_Z_cw,           /* Qualifier only. */
   cr_true_Z_ccw,          /* Qualifier only. */
   cr_lateral_cols_empty,  /* Qualifier only. */
   cr_ctrwv_end2fl,        /* Qualifier only. */
   cr_ctr2fl_endwv,        /* Qualifier only. */
   cr_split_dixie,         /* Qualifier only. */
   cr_not_split_dixie,     /* Qualifier only. */
   cr_dmd_ctrs_mwv,        /* Qualifier only. */
   cr_qtag_mwv,            /* Qualifier only. */
   cr_qtag_mag_mwv,        /* Qualifier only. */
   cr_dmd_ctrs_1f,         /* Qualifier only. */
   cr_dmd_intlk,
   cr_dmd_not_intlk,
   cr_ctr_pts_rh,          /* Qualifier only. */
   cr_ctr_pts_lh,          /* Qualifier only. */
   cr_said_tgl,            /* Qualifier only. */
   cr_didnt_say_tgl,       /* Qualifier only. */
   cr_occupied_as_stars,   /* Qualifier only. */
   cr_occupied_as_h,       /* Qualifier only. */
   cr_occupied_as_qtag,    /* Qualifier only. */
   cr_occupied_as_3x1tgl,  /* Qualifier only. */
   cr_line_ends_looking_out, /* Qualifier only. */
   cr_col_ends_lookin_in,  /* Qualifier only. */
   cr_ripple_one_end,      /* Qualifier only. */
   cr_ripple_both_ends,    /* Qualifier only. */
   cr_ripple_both_centers, /* Qualifier only. */
   cr_ripple_any_centers,  /* Qualifier only. */
   cr_people_1_and_5_real, /* Qualifier only. */
   cr_ctrs_sel,            /* Qualifier only. */
   cr_ends_sel,            /* Qualifier only. */
   cr_all_sel,             /* Qualifier only. */
   cr_none_sel,            /* Qualifier only. */
   cr_explodable,          /* Restriction only. */
   cr_rev_explodable,      /* Restriction only. */
   cr_peelable_box,        /* Restriction only. */
   cr_ends_are_peelable,   /* Restriction only. */
   cr_siamese_in_quad,     /* Restriction only. */
   cr_not_tboned,          /* Restriction only. */
   cr_opposite_sex,        /* Restriction only. */
   cr_quarterbox_or_col,   /* Restriction only. */
   cr_quarterbox_or_magic_col, /* Restriction only. */
   cr_all_ns,              /* Restriction only. */
   cr_all_ew,              /* Restriction only. */
   cr_real_1_4_tag,        /* Restriction only. */
   cr_real_3_4_tag,        /* Restriction only. */
   cr_real_1_4_line,       /* Restriction only. */
   cr_real_3_4_line,       /* Restriction only. */
   cr_jleft,               /* Restriction only. */
   cr_jright,              /* Restriction only. */
   cr_ijleft,              /* Restriction only. */
   cr_ijright              /* Restriction only. */
} call_restriction;
#define NUM_QUALIFIERS (((int) cr_ijright)+1)

/* BEWARE!!  This list must track the array "schematab" in dbcomp.c .
   Also, "schema_sequential" must be the start of all the sequential ones. */
typedef enum {
   schema_concentric,
   schema_cross_concentric,
   schema_single_concentric,
   schema_single_cross_concentric,
   schema_grand_single_concentric,
   schema_grand_single_cross_concentric,
   schema_single_concentric_together,
   schema_maybe_matrix_single_concentric_together,
   schema_maybe_single_concentric,
   schema_maybe_single_cross_concentric,
   schema_maybe_grand_single_concentric,
   schema_maybe_grand_single_cross_concentric,
   schema_maybe_special_single_concentric,
   schema_3x3_concentric,
   schema_4x4_lines_concentric,
   schema_4x4_cols_concentric,
   schema_maybe_nxn_lines_concentric,
   schema_maybe_nxn_cols_concentric,
   schema_maybe_nxn_1331_lines_concentric,
   schema_maybe_nxn_1331_cols_concentric,
   schema_1331_concentric,
   schema_1313_concentric,       /* Not for public use! */
   schema_concentric_diamond_line,
   schema_concentric_diamonds,
   schema_cross_concentric_diamonds,
   schema_concentric_or_diamond_line,
   schema_concentric_6_2,
   schema_concentric_2_6,
   schema_concentric_2_4,
   schema_concentric_big2_6,     /* Not for public use! */
   schema_concentric_2_6_or_2_4,
   schema_concentric_6p,
   schema_concentric_6p_or_normal,
   schema_concentric_6p_or_sgltogether,
   schema_cross_concentric_6p_or_normal,
   schema_concentric_others,
   schema_concentric_6_2_tgl,
   schema_concentric_to_outer_diamond,
   schema_conc_12,
   schema_conc_16,
   schema_conc_star,
   schema_conc_star12,
   schema_conc_star16,
   schema_conc_bar,
   schema_conc_bar12,
   schema_conc_bar16,
   schema_conc_o,
   schema_maybe_matrix_conc,
   schema_maybe_matrix_conc_star,
   schema_maybe_matrix_conc_bar,
   schema_checkpoint,
   schema_cross_checkpoint,
   schema_rev_checkpoint,
   schema_ckpt_star,
   schema_in_out_triple_squash,
   schema_in_out_triple,
   schema_in_out_quad,
   schema_in_out_12mquad,
   schema_in_out_triple_zcom,
   schema_select_leads,
   schema_select_headliners,
   schema_select_sideliners,
   schema_select_original_rims,
   schema_select_original_hubs,
   schema_select_ctr2,
   schema_select_ctr4,
   schema_select_ctr6,
   schema_lateral_6,             /* Not for public use! */
   schema_vertical_6,            /* Not for public use! */
   schema_intlk_lateral_6,       /* Not for public use! */
   schema_intlk_vertical_6,      /* Not for public use! */
   schema_by_array,
   schema_nothing,
   schema_matrix,
   schema_partner_matrix,
   schema_roll,
   schema_sequential,            /* All after this point are sequential. */
   schema_split_sequential,
   schema_sequential_with_fraction,
   schema_sequential_with_split_1x8_id
} calldef_schema;


/* BEWARE!!  Some of these flags co-exist with other flags defined elsewhere.
   The early ones are "concentricity" flags.  They must co-exist with the
   setupflags defined in sd.h because they share the same word.  For that reason,
   the latter flags are defined at the high end of the word, and the concentricity
   flags shown here are at the low end.
   The last bunch of flags are pushed up against the high end of the word, so that
   they can exactly match some other flags.  The constant HERITABLE_FLAG_MASK
   embraces them.  The flags that must stay in step are in the "FINAL__XXX" group
   in sd.h, the "cflag__xxx" group in database.h, and the "dfm_xxx" group in
   database.h . There is compile-time code in sdinit.c to check that these
   constants are all in step.

   dfm_conc_demand_lines             --  concdefine outers: must be ends of lines at start
   dfm_conc_demand_columns           --  concdefine outers: must be ends of columns at start
   dfm_conc_force_lines              --  concdefine outers: force them to line spots when done
   dfm_conc_force_columns            --  concdefine outers: force them to column spots when done
   dfm_conc_force_otherway           --  concdefine outers: force them to other spots when done
   dfm_conc_force_spots              --  concdefine outers: force them to same spots when done
   dfm1_conc_concentric_rules        --  concdefine outers: apply actual concentric ("lines-to-lines/columns-to-columns") rule
   dfm1_suppress_elongation_warnings --  concdefine outers: suppress warn_lineconc_perp etc.
           NOTE: the above 8 flags are specified only in the second spec, even if the concept is
              cross-concentric, in which case the "demand" flags might be considered to belong
              with the first spec.
   DFM1_CALL_MOD_MASK                 --  concdefine/seqdefine: can substitute something under certain circumstances
                  specific encodings within this mask are:
                     1   "or_anycall"
                     2   "mandatory_anycall"
                     3   "allow_plain_mod"
                     4   "allow_forced_mod"
                     5   "or_secondary_call"
                     6   "mandatory_secondary_call"
   dfm1_repeat_n                      --  seqdefine: take a numeric argument and replicate this part N times
   dfm1_repeat_n_alternate            --  seqdefine: take a numeric argument and replicate this part and the next one N times alternately
   dfm1_endscando                     --  concdefine outers: can tell ends only to do this
   dfm1_repeat_nm1                    --  seqdefine: take a numeric argument and replicate this part N-1 times
   dfm1_roll_transparent              --  seqdefine: any person who is marked roll-neutral after this call has his previous roll status restored
   dfm1_cpls_unless_single            --  seqdefine: the do this part as couples, unless this call is being done "single"
                                                               and "single_is_inherited" was set

   DFM1_NUM_INSERT_MASK              --  if nonzero, shift that 3-bit number into the number fields
   DFM1_FRACTAL_INSERT               --  if on, any number insertion is to have 1<->3 switched.
   DFM1_NO_CHECK_MOD_LEVEL           --  don't check the level of a modifier like "interlocked" -- if the subcall
                                             says it is only legal at C3, accept it anyway.

   INHERITFLAG_DIAMOND               --  concdefine/seqdefine: if original call said "diamond" apply it to this part
   INHERITFLAG_LEFT                  --  concdefine/seqdefine: if original call said "left" apply it to this part
   INHERITFLAG_FUNNY                 --  concdefine/seqdefine: if original call said "funny" apply it to this part
   INHERITFLAG_INTLK                 --  concdefine/seqdefine: if original call said "interlocked" apply it to this part
   INHERITFLAG_MAGIC                 --  concdefine/seqdefine: if original call said "magic" apply it to this part
   INHERITFLAG_GRAND                 --  concdefine/seqdefine: if original call said "grand" apply it to this part
   INHERITFLAG_12_MATRIX             --  concdefine/seqdefine: if original call said "12matrix" apply it to this part
   INHERITFLAG_16_MATRIX             --  concdefine/seqdefine: if original call said "16matrix" apply it to this part
   INHERITFLAG_CROSS                 --  concdefine/seqdefine: if original call said "cross" apply it to this part
   INHERITFLAG_SINGLE                --  concdefine/seqdefine: if original call said "single" apply it to this part
*/


/* BEWARE!!  This list must track the table "defmodtab1" in dbcomp.c . */
/* BEWARE!!  The "SEQ" stuff must track the table "seqmodtab1" in dbcomp.c . */
/* BEWARE!!  The union of all of these flags, which is encoded in DFM1_CONCENTRICITY_FLAG_MASK,
   must coexist with the CMD_MISC__ flags defined in sd.h .  Note that the bit definitions
   of those flags start where these end.  Keep it that way.  If any flags are added here,
   they must be taken away from the CMD_MISC__ flags. */
/* Start of concentricity flags.  These go in the "modifiers1" word of a by_def_item. */

static Const uint32 DFM1_CONC_DEMAND_LINES            = 0x00000001;
static Const uint32 DFM1_CONC_DEMAND_COLUMNS          = 0x00000002;
static Const uint32 DFM1_CONC_FORCE_LINES             = 0x00000004;
static Const uint32 DFM1_CONC_FORCE_COLUMNS           = 0x00000008;
static Const uint32 DFM1_CONC_FORCE_OTHERWAY          = 0x00000010;
static Const uint32 DFM1_CONC_FORCE_SPOTS             = 0x00000020;
static Const uint32 DFM1_CONC_CONCENTRIC_RULES        = 0x00000040;
static Const uint32 DFM1_SUPPRESS_ELONGATION_WARNINGS = 0x00000080;

/* End of concentricity flags.  This constant embraces them. */
static Const uint32 DFM1_CONCENTRICITY_FLAG_MASK      = 0x000000FF;

/* These are the "seq" flags.  They overlay the "conc" flags. */
/* Under normal conditions, we do *not* re-evaluate between parts.  This
   flag overrides that and makes us re-evaluate. */
static Const uint32 DFM1_SEQ_RE_EVALUATE              = 0x00000001;
static Const uint32 DFM1_SEQ_DO_HALF_MORE             = 0x00000002;
/* But, if we break up a call with something like "random", the convention
   is to re-evaluate at the break point.  This flag, used for calls like
   "patch the <anyone>" or "rims trade back", says that we *never* re-evaluate,
   even if the call is broken up. */
static Const uint32 DFM1_SEQ_NO_RE_EVALUATE           = 0x00000004;
static Const uint32 DFM1_SEQ_REENABLE_ELONG_CHK       = 0x00000008;
static Const uint32 DFM1_SEQ_REPEAT_N                 = 0x00000010;
static Const uint32 DFM1_SEQ_REPEAT_N_ALTERNATE       = 0x00000020;
static Const uint32 DFM1_SEQ_REPEAT_NM1               = 0x00000040;
static Const uint32 DFM1_SEQ_NORMALIZE                = 0x00000080;

/* BEWARE!!  This list must track the table "defmodtab1" in dbcomp.c . */
/* Start of miscellaneous flags.  These go in the "modifiers1" word of a by_def_item. */

/* This is a 3 bit field -- CALL_MOD_BIT tells where its low bit lies. */
static Const uint32 DFM1_CALL_MOD_MASK                = 0x00000700;
#define             DFM1_CALL_MOD_BIT                   0x00000100
/* unused:                                = 0x00000800 */
/* unused:                                = 0x00001000 */
static Const uint32 DFM1_ENDSCANDO                    = 0x00002000;
static Const uint32 DFM1_FINISH_THIS                  = 0x00004000;
static Const uint32 DFM1_ROLL_TRANSPARENT             = 0x00008000;
static Const uint32 DFM1_PERMIT_TOUCH_OR_REAR_BACK    = 0x00010000;
static Const uint32 DFM1_CPLS_UNLESS_SINGLE           = 0x00020000;
/* This is a 2 bit field -- NUM_SHIFT_BIT tells where its low bit lies. */
static Const uint32 DFM1_NUM_SHIFT_MASK               = 0x000C0000;
static Const uint32 DFM1_NUM_SHIFT_BIT                = 0x00040000;
/* This is a 3 bit field -- NUM_INSERT_BIT tells where its low bit lies. */
static Const uint32 DFM1_NUM_INSERT_MASK              = 0x00700000;
static Const uint32 DFM1_NUM_INSERT_BIT               = 0x00100000;
static Const uint32 DFM1_NO_CHECK_MOD_LEVEL           = 0x00800000;
static Const uint32 DFM1_FRACTAL_INSERT               = 0x01000000;


typedef enum {
   stb_none,      /* unknown */
   stb_z,         /* "Z" - person does not turn */
   stb_a,         /* "A" - person turns anticlockwise from 1 to 4 quadrants */
   stb_c,         /* "C" - person turns clockwise from 1 to 4 quadrants */
   stb_ac,        /* "AC" - person turns anticlockwise once, then clockwise 1 to 4 quadrants */
   stb_ca,        /* "CA" - person turns clockwise once, then anticlockwise 1 to 4 quadrants */
   stb_aac,       /* "AAC" - person turns anticlockwise twice, then clockwise 1 to 4 quadrants */
   stb_cca,       /* "CCA" - person turns clockwise twice, then anticlockwise 1 to 4 quadrants */
   stb_aaac,      /* "AAAC" - person turns anticlockwise 3 times, then clockwise 1 to 4 quadrants */
   stb_ccca,      /* "CCCA" - person turns clockwise 3 times, then anticlockwise 1 to 4 quadrants */
   stb_aaaac,     /* "AAAAC" - person turns anticlockwise 4 times, then clockwise 1 to 4 quadrants */
   stb_cccca,     /* "CCCCA" - person turns clockwise 4 times, then anticlockwise 1 to 4 quadrants */
   stb_aa,        /* "AA" - person turns anticlockwise from 5 to 8 quadrants */
   stb_cc         /* "CC" - person turns clockwise from 5 to 8 quadrants */
} stability;

/* These define the format of the short int (16 bits, presumably) items emitted
   for each person in a by-array call definition.  These will get read into the
   "arr" array of a predptr_pair or the "stuff.def" array of a callarray.

   The format of this item is:
       stability     roll     where     direction
         info        info     to go      to face
        4 bits      3 bits    5 bits      4 bits

   The direction is in the special format
            north   10 octal
            south   12 octal
            east    01 octal
            west    03 octal
   that makes procedures "rotcw" etc. work correctly.  The constants
   d_north, d_south, d_east, d_west in sd.h are just these numbers
   with the 1000 (octal) bit, which is BIT_PERSON, added. */

#define DBROLL_BIT 0x200
#define DBSTAB_BIT 0x1000
