/* SD -- square dance caller's helper.

    Copyright (C) 1990-1995  William B. Ackerman.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    This is for version 31. */

/* These are written as the first two halfwords of the binary database file.
   The format version is not related to the version of the program or database.
   It is used only to make sure that the "mkcalls" program that compiled
   the database and the "sd" program that reads it are in sync with each
   other, as indicated by the version of this file.  Whenever we change
   anything in this file that could cause an incompatibility, we bump the
   database format version. */

#define DATABASE_MAGIC_NUM 21316
#define DATABASE_FORMAT_VERSION 56




/* BEWARE!!  These must track the items in tagtabinit in dbcomp.c . */
#define BASE_CALL_CAST_3_4   2
/* These 4 must be consecutive. */
#define BASE_CALL_TAGGER0    3
#define BASE_CALL_TAGGER1    4
#define BASE_CALL_TAGGER2    5
#define BASE_CALL_TAGGER3    6


/* BEWARE!!  This list must track the tables "flagtabh", "defmodtabh",
   "forcetabh", and "altdeftabh" in dbcomp.c .  These are the infamous
   "heritable flags".  They are used in generally corresponding ways in
   the "callflagsh" word of a top level callspec_block, the "modifiersh"
   word of a "by_def_item", and the "modifier_seth" word of a "calldef_block".
   The constant HERITABLE_FLAG_MASK embraces them.
*/

#define INHERITFLAG_DIAMOND               0x00000001UL
#define INHERITFLAG_REVERSE               0x00000002UL
#define INHERITFLAG_LEFT                  0x00000004UL
#define INHERITFLAG_FUNNY                 0x00000008UL
#define INHERITFLAG_INTLK                 0x00000010UL
#define INHERITFLAG_MAGIC                 0x00000020UL
#define INHERITFLAG_GRAND                 0x00000040UL
#define INHERITFLAG_12_MATRIX             0x00000080UL
#define INHERITFLAG_16_MATRIX             0x00000100UL
#define INHERITFLAG_CROSS                 0x00000200UL
#define INHERITFLAG_SINGLE                0x00000400UL
#define INHERITFLAG_1X2                   0x00000800UL
#define INHERITFLAG_2X1                   0x00001000UL
#define INHERITFLAG_2X2                   0x00002000UL
#define INHERITFLAG_1X3                   0x00004000UL
#define INHERITFLAG_3X1                   0x00008000UL
#define INHERITFLAG_3X3                   0x00010000UL
#define INHERITFLAG_4X4                   0x00020000UL
#define INHERITFLAG_SINGLEFILE            0x00040000UL
#define INHERITFLAG_HALF                  0x00080000UL

#define HERITABLE_FLAG_MASK               0x000FFFFFUL

/* These spare bits are used in the include file sd.h to allocate flag bits
   that will share a word with the heritable flags.  Those flag bits are
   used internally by the program and are not part of the database definition,
   so they don't belong here.  By setting up these spare bit definitions we
   can ensure that those bits will not conflict with the heritable flags.
   That is, if we add a heritable flag, we necessarily destroy a spare bit.
   If the definitions in sd.h find themselves using an undefined spare bit,
   we know we are in serious trouble. */

/* A 3-bit field. */
#define CFLAGH__TAG_CALL_RQ_MASK          0x00700000UL
#define CFLAGH__TAG_CALL_RQ_BIT           0x00100000UL
#define CFLAGH__REQUIRES_SELECTOR         0x00800000UL
#define CFLAGH__REQUIRES_DIRECTION        0x01000000UL
#define INHERITSPARE_1                    0x02000000UL
#define INHERITSPARE_2                    0x04000000UL
#define INHERITSPARE_3                    0x08000000UL
#define INHERITSPARE_4                    0x10000000UL
#define INHERITSPARE_5                    0x20000000UL
#define INHERITSPARE_6                    0x40000000UL
#define INHERITSPARE_7                    0x80000000UL

/* BEWARE!!  This list must track the table "flagtab1" in dbcomp.c .
   These flags go into the "callflags1" word of a callspec_block. */

#define CFLAG1_VISIBLE_FRACTIONS          0x00000001UL
#define CFLAG1_FIRST_PART_VISIBLE         0x00000002UL
#define CFLAG1_12_16_MATRIX_MEANS_SPLIT   0x00000004UL
#define CFLAG1_IMPRECISE_ROTATION         0x00000008UL
#define CFLAG1_SPLIT_LIKE_DIXIE_STYLE     0x00000010UL
#define CFLAG1_PARALLEL_CONC_END          0x00000020UL
#define CFLAG1_TAKE_RIGHT_HANDS           0x00000040UL
#define CFLAG1_IS_STAR_CALL               0x00000080UL
#define CFLAG1_SPLIT_LARGE_SETUPS         0x00000100UL
#define CFLAG1_FUDGE_TO_Q_TAG             0x00000200UL
#define CFLAG1_STEP_TO_WAVE               0x00000400UL
#define CFLAG1_REAR_BACK_FROM_R_WAVE      0x00000800UL
#define CFLAG1_REAR_BACK_FROM_QTAG        0x00001000UL
#define CFLAG1_DONT_USE_IN_RESOLVE        0x00002000UL
/* This is a 3 bit field -- NUMBER_BIT tells where its low bit lies. */
#define CFLAG1_NUMBER_MASK                0x0001C000UL
#define CFLAG1_NUMBER_BIT                 0x00004000UL
#define CFLAG1_SEQUENCE_STARTER           0x00020000UL
#define CFLAG1_SPLIT_LIKE_SQUARE_THRU     0x00040000UL
#define CFLAG1_FINISH_MEANS_SKIP_FIRST    0x00080000UL
#define CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK  0x00100000UL
#define CFLAG1_CAN_BE_FAN_OR_YOYO         0x00200000UL
#define CFLAG1_NO_CUTTING_THROUGH         0x00400000UL
#define CFLAG1_NO_ELONGATION_ALLOWED      0x00800000UL
/* This is a 3 bit field -- BASE_TAG_CALL_BIT tells where its low bit lies. */
#define CFLAG1_BASE_TAG_CALL_MASK         0x07000000UL
#define CFLAG1_BASE_TAG_CALL_BIT          0x01000000UL
#define CFLAG1_YIELD_IF_AMBIGUOUS         0x08000000UL

/* Beware!!  This list must track the table "matrixcallflagtab" in dbcomp.c . */

#define MTX_USE_SELECTOR           0001
#define MTX_STOP_AND_WARN_ON_TBONE 0002
#define MTX_TBONE_IS_OK            0004
#define MTX_IGNORE_NONSELECTEES    0010
#define MTX_MUST_FACE_SAME_WAY     0020


/* BEWARE!!  This list must track the table "leveltab" in dbcomp.c . */
/* BEWARE!!  This list must track the table "getout_strings" in sdtables.c . */
/* BEWARE!!  This list must track the table "filename_strings" in sdtables.c . */

typedef enum {
   l_mainstream, l_plus,
   l_a1, l_a2, l_c1, l_c2,
   l_c3a, l_c3, l_c3x, l_c4a, l_c4,
   l_dontshow, l_nonexistent_concept
} dance_level;

/* These are the states that people can be in, and the "ending setups" that can appear
   in the call data base. */
/* BEWARE!!  This list must track the array "estab" in dbcomp.c . */
/* BEWARE!!  This list must track the array "setup_attrs" in sdtables.c . */
/* BEWARE!!  This list must track the array "map_lists" in sdtables.c . */
/* BEWARE!!  This list must track the array "printing_tables" in sdutil.c . */
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
   s_qtag,
   s_bone,
   s_rigger,
   s_spindle,
   s_hrglass,
   s_dhrglass,
   s_hyperglass,
   s_crosswave,
   s1x8,
   s2x4,
   s2x3,
   s1x6,
   s3x4,
   s2x6,
   s2x8,
   s4x4,
   sx1x6,  /* Crossed 1x6's -- internal use only. */
   s1x10,
   s1x12,
   s1x14,
   s1x16,
   s_c1phan,
   s_bigblob,
   s_ptpd,
   s3x1dmd,
   s3dmd,
   s4dmd,
   s_wingedstar,
   s_wingedstar12,
   s_wingedstar16,
   s_galaxy,
   s4x6,
   s_thar,
   sx4dmd,   /* These are too big to actually represent -- */
   s8x8,     /* we don't let them out of their cage. */
   sfat2x8,  /* Same here.  These are big setups that are the size of 4x8's, */
   swide4x4, /* but only have 16 people.  The reason is to prevent loss of phantoms. */
   sbigdmd,
   sminirigger,  /* Used internally by concentric stuff. */
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
   b_qtag,
   b_pqtag,
   b_bone,
   b_pbone,
   b_rigger,
   b_prigger,
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
   b_1x6,
   b_6x1,
   b_3x4,
   b_4x3,
   b_2x6,
   b_6x2,
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
   b_4x6,
   b_6x4,
   b_thar,
   b_ptpd,
   b_pptpd,
   b_3x1dmd,
   b_p3x1dmd,
   b_3dmd,
   b_p3dmd,
   b_4dmd,
   b_p4dmd,
   b_bigdmd,
   b_pbigdmd
} begin_kind;

/* These bits are used in the "callarray_flags" field of a "callarray". */

/* This one must be 1!!!! */
#define CAF__ROT 0x1
#define CAF__FACING_FUNNY 0x2
/* Next one says this is concentrically defined --- the "end_setup" slot
   has the centers' end setup, and there is an extra slot with the ends' end setup. */
#define CAF__CONCEND 0x4
/* Next one meaningful only if previous one is set. */
#define CAF__ROT_OUT 0x8
/* This is a 2 bit field. */
#define CAF__RESTR_MASK 0x30
/* These next 3 are the nonzero values it can have. */
#define CAF__RESTR_UNUSUAL 0x10
#define CAF__RESTR_FORBID 0x20
#define CAF__RESTR_RESOLVE_OK 0x30
#define CAF__PREDS 0x40

/* These qualifiers are "overloaded" -- their meaning depends on the starting setup. */
/* BEWARE!!  This list must track the array "qualtab" in dbcomp.c . */

typedef enum {
   sq_none,
   sq_wave_only,                    /* 1x4 or 2x4 - waves; 2x2 - real RH or LH box */
   sq_2fl_only,                     /* 1x4 or 2x4 - 2FL; 4x1 - single DPT or single CDPT */
   sq_in_or_out,                    /* 2x2 - all facing in or all facing out */
   sq_miniwaves,                    /* 1x2, 1x4, 1x8, 2x4, 2x2, dmd, qtag, trngl - people
                                       are paired in miniwaves of various handedness.
                                       For diamonds and qtags, this applies just to the centers.
                                       For triangles, it applies just to the base.
                                       So this includes waves, inverted lines, columns,
                                       magic columns, diamonds, wave-based triangles... */
   sq_rwave_only,                   /* As above, but all the miniwaves must be right-handed */
   sq_lwave_only,                   /* As above, but all the miniwaves must be left-handed */
   sq_1_4_tag,                      /* dmd, qtag - this is a 1/4 tag, i.e. points are looking in */
   sq_3_4_tag,                      /* dmd, qtag - this is a 3/4 tag, i.e. points are looking out */
   sq_dmd_same_pt,                  /* dmd - centers would circulate to same point */
   sq_dmd_facing,                   /* dmd - diamond is fully occupied and fully facing */
   sq_true_Z,                       /* 2x3, 3x4, 2x6 - setup is a genuine Z */
   sq_ctrwv_end2fl,                 /* crosswave - center line is wave, end line is 2fl */
   sq_ctr2fl_endwv,                 /* crosswave - center line is 2fl, end line is wave */
   sq_split_dixie,                  /* 2x2 - invoked with "split" for dixie style */
   sq_not_split_dixie,              /* 2x2 - invoked without "split" for dixie style */
   sq_8_chain,                      /* 4x1 - setup is single 8 chain */
   sq_trade_by,                     /* 4x1 - setup is single trade by */
   sq_said_tgl,                     /* short6/pshort6 - caller said the word "triangle" */
   sq_didnt_say_tgl                 /* short6/pshort6 - caller didn't say the word "triangle" */
} search_qualifier;

/* These restrictions are "overloaded" -- their meaning depends on the starting setup. */
/* BEWARE!!  This list must track the array "crtab" in dbcomp.c . */

typedef enum {
   cr_none,
   cr_alwaysfail,                   /* any setup - this always fails (presumably to give the "unusual position" warning) */
   cr_wave_only,                    /* 1x2 - a miniwave; 1x4 - a wave; 2x4/3x4/2x6/2x8 - waves; 1x6-1x16 - a grand wave;
                                       2x2 - real box; 4x4 - consistent waves; qtag - wave in center; pqtag - wave in center
                                       (use only if center people have no legal move from pqtag, only from qtag); 3x2/4x2/6x2/8x2 - column */
   cr_wave_unless_say_2faced,
   cr_all_facing_same,              /* 2x2/2x3/2x4/1x4/1x6/1x8 - all people in the setup facing the same way */
   cr_1fl_only,                     /* 1x4/1x6/1x8 - a 1FL; 2x3/2x4 - individual 1FL's */
   cr_2fl_only,                     /* 1x2 - a couple; 1x4 - a 2FL; 2x4/3x4 - 2FL's; 1x8 - a grand 2FL; 2x2 - "1-faced" box;
                                       4x4 - consistent 2FL's; 2x4 columns - a DPT or CDPT; qtag - 2FL in center */
   cr_3x3_2fl_only,                 /* 1x6 - 3 facing one way, 3 the other */
   cr_4x4_2fl_only,                 /* 1x8 - 4 facing one way, 4 the other */
   cr_leads_only,                   /* 2x2 - all people are leads, and not T-boned */
   cr_couples_only,                 /* 1x2 or 1x4 or 2x2 or 2x4 lines, or 2x4 columns - people are in genuine couples, not miniwaves */
   cr_3x3couples_only,              /* 1x6 lines - each group of 3 people are facing the same way */
   cr_4x4couples_only,              /* 1x8 lines - each group of 4 people are facing the same way */
   cr_awkward_centers,              /* 1x2, 1x4, 1x6, or 1x8 - centers must not have left hands with each other */
   cr_diamond_like,                 /* dmd, qtag or ptpd - in some kind of diamond(s), as opposed to single quarter-tag(s), i.e. points look sideways */
   cr_qtag_like,                    /* dmd, qtag or ptpd - in some kind of single quarter-tag(s), as opposed to diamond(s), i.e. points look in or out */
   cr_nice_diamonds,                /* dmd, qtag or ptpd - diamond(s) have consistent handedness, assuming they are diamonds and not qtags */
   cr_magic_only,                   /* 2x2 - split-trade-circulate type of box; 3x2 or 4x2 - magic column; 2x4 - inverted lines; 1x4 - single inverted line */
   cr_peelable_box,                 /* 2x2, 3x2, 4x2, 6x2 or 8x2 - all people in each column are facing same way */
   cr_ends_are_peelable,            /* 2x4 - ends are a box with each person in genuine tandem */
   cr_not_tboned,                   /* 2x2 - people must not be T-boned */
   cr_opposite_sex,                 /* 2x1 - people must be opposite sexes facing each other */
   cr_quarterbox_or_col,            /* 4x2 - acceptable setup for "triple cross" */
   cr_quarterbox_or_magic_col,      /* 4x2 - acceptable setup for "make magic" */
   cr_gen_1_4_tag,                  /* SPECIAL -- used for "assume" stuff */
   cr_gen_3_4_tag,                  /* SPECIAL -- used for "assume" stuff */
   cr_jleft,                        /* SPECIAL -- used for "assume" stuff */
   cr_jright,                       /* SPECIAL -- used for "assume" stuff */
   cr_ijleft,                       /* SPECIAL -- used for "assume" stuff */
   cr_ijright,                      /* SPECIAL -- used for "assume" stuff */
   cr_li_lo                         /* SPECIAL -- used for "assume" stuff */
} call_restriction;

/* BEWARE!!  This list must track the array "schematab" in dbcomp.c . */
typedef enum {
   schema_concentric,
   schema_cross_concentric,
   schema_single_concentric,
   schema_single_cross_concentric,
   schema_single_concentric_together,
   schema_maybe_single_concentric,
   schema_maybe_single_cross_concentric,
   schema_concentric_diamond_line,
   schema_concentric_6_2,
   schema_concentric_2_6,
   schema_concentric_6_2_tgl,
   schema_conc_star,
   schema_conc_star12,
   schema_conc_star16,
   schema_maybe_matrix_conc_star,
   schema_checkpoint,
   schema_rev_checkpoint,
   schema_ckpt_star,
   schema_conc_triple_lines,
   schema_lateral_6,       /* Not for public use! */
   schema_vertical_6,      /* Not for public use! */
   schema_sequential,
   schema_split_sequential,
   schema_by_array,
   schema_nothing,
   schema_matrix,
   schema_partner_matrix,
   schema_roll
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
/* BEWARE!!  The union of all of these flags, which is encoded in DFM1_CONCENTRICITY_FLAG_MASK,
   must coexist with the CMD_MISC__ flags defined in sd.h .  Note that the bit definitions
   of those flags start where these end.  Keep it that way.  If any flags are added here,
   they must be taken away from the CMD_MISC__ flags. */
/* Start of concentricity flags.  These go in the "modifiers1" word of a by_def_item. */

#define DFM1_CONC_DEMAND_LINES            0x00000001
#define DFM1_CONC_DEMAND_COLUMNS          0x00000002
#define DFM1_CONC_FORCE_LINES             0x00000004
#define DFM1_CONC_FORCE_COLUMNS           0x00000008
#define DFM1_CONC_FORCE_OTHERWAY          0x00000010
#define DFM1_CONC_FORCE_SPOTS             0x00000020
#define DFM1_CONC_CONCENTRIC_RULES        0x00000040
#define DFM1_SUPPRESS_ELONGATION_WARNINGS 0x00000080

/* End of concentricity flags.  This constant embraces them. */
#define DFM1_CONCENTRICITY_FLAG_MASK      0x000000FF

/* BEWARE!!  This list must track the table "defmodtab1" in dbcomp.c . */
/* Start of miscellaneous flags.  These go in the "modifiers1" word of a by_def_item. */

/* This is a 3 bit field -- CALL_MOD_BIT tells where its low bit lies. */
#define DFM1_CALL_MOD_MASK                0x00000700
#define DFM1_CALL_MOD_BIT                 0x00000100
#define DFM1_REPEAT_N                     0x00000800
#define DFM1_REPEAT_N_ALTERNATE           0x00001000
#define DFM1_ENDSCANDO                    0x00002000
#define DFM1_REPEAT_NM1                   0x00004000
#define DFM1_ROLL_TRANSPARENT             0x00008000
/* spare:                                 0x00010000 */
#define DFM1_CPLS_UNLESS_SINGLE           0x00020000
/* This is a 2 bit field -- NUM_SHIFT_BIT tells where its low bit lies. */
#define DFM1_NUM_SHIFT_MASK               0x000C0000
#define DFM1_NUM_SHIFT_BIT                0x00040000

/* The first 10 predicates (in "pred_table" in preds.c and "predtab" in dbcomp.c)
   take selectors.  The following constant indicates that. */
#define SELECTOR_PREDS 10

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
