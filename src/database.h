/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992  William B. Ackerman.

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

    This is for version 28. */

/* These are written as the first two halfwords of the binary database file.
   The format version is not related to the version of the program or database.
   It is used only to make sure that the "mkcalls" program that compiled
   the database and the "sd" program that reads it are in sync with each
   other, as indicated by the version of this file.  Whenever we change
   anything in this file that could cause an incompatibility, we bump the
   database format version. */

#define DATABASE_MAGIC_NUM 21316
#define DATABASE_FORMAT_VERSION 32

/* BEWARE!!  This list must track the tables "flagtab" and "nexttab"" in mkcalls.c .
   Because the constants are not defined contiguously, there are spacer items
   in those tables.
   The last bunch of flags are pushed up against the high end of the word, so that
   they can exactly match some other flags.  The constant HERITABLE_FLAG_MASK
   embraces them.  The flags that must stay in step are in the "FINAL__XXX" group
   in sd.h, the "cflag__xxx" group in database.h, and the "dfm_xxx" group in
   database.h . There is compile-time code in sdinit.c to check that these
   constants are all in step.
*/

#define cflag__step_to_wave               0x00000001
#define cflag__rear_back_from_r_wave      0x00000002
#define cflag__rear_back_from_qtag        0x00000004
#define cflag__dont_use_in_resolve        0x00000008
#define cflag__requires_selector          0x00000010
#define cflag__requires_number            0x00000020
#define cflag__sequence_starter           0x00000040
#define cflag__split_like_square_thru     0x00000080
#define cflag__split_like_dixie_style     0x00000100
#define cflag__parallel_conc_end          0x00000200
#define cflag__take_right_hands           0x00000400
#define cflag__is_tag_call                0x00000800
#define cflag__is_scoot_call              0x00001000
#define cflag__is_star_call               0x00002000
#define cflag__split_large_setups         0x00004000
#define cflag__fudge_to_q_tag             0x00008000
#define cflag__visible_fractions          0x00010000
#define cflag__first_part_visible         0x00020000
#define cflag__12_16_matrix_means_split   0x00040000
/* space left for                         0x00080000 */
/* space left for                         0x00100000 */
/* space left for                         0x00200000 */
#define cflag__diamond_is_inherited       0x00400000
#define cflag__reverse_means_mirror       0x00800000
#define cflag__left_means_mirror          0x01000000
#define cflag__funny_is_inherited         0x02000000
#define cflag__intlk_is_inherited         0x04000000
#define cflag__magic_is_inherited         0x08000000
#define cflag__grand_is_inherited         0x10000000
#define cflag__12_matrix_is_inherited     0x20000000
#define cflag__cross_is_inherited         0x40000000
#define cflag__single_is_inherited        0x80000000

#define HERITABLE_FLAG_MASK               0xFFC00000

/* Beware!!  This list must track the table "matrixcallflagtab" in mkcalls.c . */

#define MTX_USE_SELECTOR           0001
#define MTX_STOP_AND_WARN_ON_TBONE 0002
#define MTX_TBONE_IS_OK            0004
#define MTX_IGNORE_NONSELECTEES    0010
#define MTX_MUST_FACE_SAME_WAY     0020


/* BEWARE!!  This list must track the table "leveltab" in mkcalls.c . */
/* BEWARE!!  This list must track the table "getout_strings" in sdtables.c . */
/* BEWARE!!  This list must track the table "filename_strings" in sdtables.c . */

typedef enum {
   l_mainstream, l_plus,
   l_a1, l_a2, l_c1, l_c2,
   l_c3a, l_c3, l_c3x, l_c4a, l_c4,
   l_dontshow, l_nonexistent_concept
} level;

/* These are the states that people can be in, and the "ending setups" that can appear
   in the call data base. */
/* BEWARE!!  This list must track the array "estab" in mkcalls.c . */
/* BEWARE!!  This list must track the array "keytab" in sdtables.c . */
/* BEWARE!!  This list must track the array "setup_coords" in sdtables.c . */
/* BEWARE!!  This list must track the array "nice_setup_coords" in sdtables.c . */
/* BEWARE!!  This list must track the array "setup_limits" in sdtables.c . */
/* BEWARE!!  This list must track the array "map_lists" in sdtables.c . */
/* BEWARE!!  This list must track the array "bigconctab" in sdconc.c . */
/* BEWARE!!  The procedure "merge_setups" canonicalizes pairs of setups by their
   order in this list, and will break if it is re-ordered randomly.  See the comments
   there before changing the order of existing setups. In general, keep small setups
   first, particularly 4-person setups before 8-person setups. */

typedef enum {
   nothing,
   s_1x1,
   s_1x2,
   s_1x3,
   s2x2,
   sdmd,
   s_star,
   s_trngl,
   s_bone6,
   s_short6,
   s_qtag,
   s_bone,
   s_rigger,
   s_spindle,
   s_hrglass,
   s_hyperglass,
   s_crosswave,
   s1x4,
   s1x8,
   s2x4,
   s_2x3,
   s_1x6,
   s3x4,
   s2x6,
   s2x8,
   s4x4,
   s1x10,
   s1x12,
   s1x14,
   s1x16,
   s_c1phan,
   s_bigblob,
   s_ptpd,
   s_3x1dmd,
   s_3dmd,
   s_4dmd,
   s_wingedstar,
   s_wingedstar12,
   s_wingedstar16,
   s_galaxy,
   s4x6,
   s_thar,
   s_6x6,    /* These are too big to actaully represent -- */
   s_8x8,    /* we don't let them out of their cage. */
   s_normal_concentric
} setup_kind;

/* These are the "beginning setups" that can appear in the call data base. */
/* BEWARE!!  This list must track the array "sstab" in mkcalls.c . */
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
   b_p4dmd
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
#define CAF__RESTR_UNUSUAL 0x10
#define CAF__RESTR_FORBID 0x20
#define CAF__PREDS 0x40

/* These qualifiers are "overloaded" -- their meaning depends on the starting setup. */
/* BEWARE!!  This list must track the array "qualtab" in mkcalls.c . */

typedef enum {
   sq_none,
   sq_wave_only,                    /* 1x4 or 2x4 - waves */
   sq_2fl_only,                     /* 1x4 or 2x4 - 2FL's */
   sq_miniwaves,                    /* 1x2, 1x4, 1x8, 2x4, 2x2, dmd, qtag, trngl - people
                                       are paired in miniwaves of various handedness.
                                       For diamonds and qtags, this applies just to the centers.
                                       For triangles, it applies just to the base.
                                       So this includes waves, inverted lines, columns,
                                       magic columns, diamonds, wave-based triangles... */
   sq_rwave_only,                   /* As above, but all the miniwaves must be right-handed */
   sq_lwave_only,                   /* As above, but all the miniwaves must be left-handed */
   sq_3_4_tag,                      /* dmd, qtag - this is a 3/4 tag, i.e. points are looking out */
   sq_dmd_same_pt,                  /* dmd - centers would circulate to same point */
   sq_dmd_facing,                   /* dmd - diamond is fully occupied and fully facing */
   sq_true_Z,                       /* 2x3, 3x4, 2x6 - setup is a genuine Z */
   sq_ctrwv_end2fl,                 /* crosswave - center line is wave, end line is 2fl */
   sq_ctr2fl_endwv                  /* crosswave - center line is 2fl, end line is wave */
} search_qualifier;

/* These restrictions are "overloaded" -- their meaning depends on the starting setup. */
/* BEWARE!!  This list must track the array "crtab" in mkcalls.c . */

typedef enum {
   cr_none,
   cr_alwaysfail,                   /* any setup - this always fails (presumably to give the "unusual position" warning) */
   cr_wave_only,                    /* 1x2 - a miniwave; 1x4 - a wave; 2x4 - waves; 1x8 - a grand wave; 2x2 - real box;
                                       qtag - wave in center; pqtag - wave in center (use only if center people have no legal
                                       move from pqtag, only from qtag); 4x2 or 3x2 - column */
   cr_wave_unless_say_2faced,
   cr_all_facing_same,              /* 2x2, 2x3, or 2x4 - all people facing the same way. */
   cr_1fl_only,                     /* 1x4 - a 1FL; 2x3 or 2x4 - 1FL's */
   cr_2fl_only,                     /* 1x2 - a couple; 1x4 - a 2FL; 2x4 - 2FL's; 1x8 - a grand 2FL; 2x2 - "1-faced" box; qtag - 2FL in center */
   cr_3x3_2fl_only,                 /* 1x6 - 3 facing one way, 3 the other */
   cr_4x4_2fl_only,                 /* 1x8 - 4 facing one way, 4 the other */
   cr_couples_only,                 /* 1x2 or 1x4 or 2x2 or 2x4 lines, or 2x4 columns - people are in genuine couples, not miniwaves */
   cr_3x3couples_only,              /* 1x6 lines - each group of 3 people are facing the same way */
   cr_4x4couples_only,              /* 1x8 lines - each group of 4 people are facing the same way */
   cr_awkward_centers,              /* 1x4 or 1x2 - centers must not have left hands with each other */
   cr_nice_diamonds,                /* qtag or ptpd - diamonds have consistent handedness */
   cr_magic_only,                   /* 2x2 - split-trade-circulate type of box; 3x2 or 4x2 - magic column */
   cr_peelable_box,                 /* 2x2 or 3x2 or 4x2 - all people in each column are facing same way */
   cr_ends_are_peelable,            /* 2x4 - ends are a box with each person in genuine tandem */
   cr_not_tboned,                   /* 2x2 - people must not be T-boned */
   cr_quarterbox_or_col,            /* 4x2 - acceptable setup for "triple cross" */
   cr_quarterbox_or_magic_col       /* 4x2 - acceptable setup for "make magic" */
} call_restriction;

/* BEWARE!!  This list must track the array "schematab" in mkcalls.c . */
typedef enum {
   schema_concentric,
   schema_cross_concentric,
   schema_single_concentric,
   schema_single_cross_concentric,
   schema_maybe_single_concentric,
   schema_concentric_diamond_line,
   schema_concentric_6_2,
   schema_concentric_2_6,
   schema_conc_star,
   schema_conc_star12,
   schema_conc_star16,
   schema_maybe_matrix_conc_star,
   schema_checkpoint,
   schema_rev_checkpoint,
   schema_ckpt_star,
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

/* Maximum number of subcalls in a sequential definition ("schema_sequential") list. */
#define SEQDEF_MAX 8

/* BEWARE!!  This list must track the table "defmodtab" in mkcalls.c .
   Because the constants are not defined contiguously, there are spacer items
   in that table. */
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
   dfm_conc_concentric_rules         --  concdefine outers: apply actual concentric ("lines-to-lines/columns-to-columns") rule
   dfm_suppress_elongation_warnings  --  concdefine outers: suppress warn_lineconc_perp etc.
           NOTE: the above 8 flags are specified only in the second spec, even if the concept is
              cross-concentric, in which case the "demand" flags might be considered to belong
              with the first spec.
   dfm_or_anycall                    --  concdefine/seqdefine: can substitute something if clicked on "allow mods"
   dfm_mandatory_anycall             --  concdefine/seqdefine: always substitute something
   dfm_repeat_n                      --  seqdefine: take a numeric argument and replicate this part N times
   dfm_repeat_nm1                    --  seqdefine: take a numeric argument and replicate this part N-1 times
   dfm_repeat_n_alternate            --  seqdefine: take a numeric argument and replicate this part and the next one N times alternately
   dfm_endscando                     --  concdefine outers: can tell ends only to do this
   dfm_allow_forced_mod              --  concdefine/seqdefine: can forcibly substitute something if clicked on "allow forcible mods"
   dfm_roll_transparent              --  seqdefine: any person who is marked roll-neutral after this call has his previous roll status restored
   dfm_must_be_tag_call              --  seqdefine: the subject call (or any replacement for it) must be a tagging call
   dfm_must_be_scoot_call            --  seqdefine: the subject call (or any replacement for it) must be a scoot-back-type call
   dfm_cpls_unless_single            --  seqdefine: the do this part as couples, unless this call is being done "single"
                                                               and "single_is_inherited" was set
   dfm_inherit_diamond               --  concdefine/seqdefine: if original call said "diamond" apply it to this part
   dfm_inherit_left                  --  concdefine/seqdefine: if original call said "left" apply it to this part
   dfm_inherit_funny                 --  concdefine/seqdefine: if original call said "funny" apply it to this part
   dfm_inherit_intlk                 --  concdefine/seqdefine: if original call said "interlocked" apply it to this part
   dfm_inherit_magic                 --  concdefine/seqdefine: if original call said "magic" apply it to this part
   dfm_inherit_grand                 --  concdefine/seqdefine: if original call said "grand" apply it to this part
   dfm_inherit_12_matrix             --  concdefine/seqdefine: if original call said "12matrix" apply it to this part
   dfm_inherit_cross                 --  concdefine/seqdefine: if original call said "cross" apply it to this part
   dfm_inherit_single                --  concdefine/seqdefine: if original call said "single" apply it to this part
*/


/* Start of concentricity flags. */

#define dfm_conc_demand_lines             0x00000001
#define dfm_conc_demand_columns           0x00000002
#define dfm_conc_force_lines              0x00000004
#define dfm_conc_force_columns            0x00000008
#define dfm_conc_force_otherway           0x00000010
#define dfm_conc_force_spots              0x00000020
#define dfm_conc_concentric_rules         0x00000040
#define dfm_suppress_elongation_warnings  0x00000080

/* End of concentricity flags.  This constant embraces them. */
#define DFM_CONCENTRICITY_FLAG_MASK       0x000000FF

/* Start of miscellaneous flags. */

#define dfm_or_anycall                    0x00000100
#define dfm_mandatory_anycall             0x00000200
#define dfm_repeat_n                      0x00000400
#define dfm_repeat_nm1                    0x00000800
#define dfm_repeat_n_alternate            0x00001000
#define dfm_endscando                     0x00002000
#define dfm_allow_forced_mod              0x00004000
#define dfm_roll_transparent              0x00008000
#define dfm_must_be_tag_call              0x00010000
#define dfm_must_be_scoot_call            0x00020000
#define dfm_cpls_unless_single            0x00040000

/* End of miscellaneous flags. */

/* space left for                         0x00080000 */
/* space left for                         0x00100000 */
/* space left for                         0x00200000 */

/* Start of inheritance flags. */

#define dfm_inherit_diamond               0x00400000
#define dfm_inherit_reverse               0x00800000
#define dfm_inherit_left                  0x01000000
#define dfm_inherit_funny                 0x02000000
#define dfm_inherit_intlk                 0x04000000
#define dfm_inherit_magic                 0x08000000
#define dfm_inherit_grand                 0x10000000
#define dfm_inherit_12_matrix             0x20000000
#define dfm_inherit_cross                 0x40000000
#define dfm_inherit_single                0x80000000

/* The first 6 predicates (in "pred_table" in preds.c and "predtab" in mkcalls.c)
   take selectors.  The following constant indicates that. */
#define SELECTOR_PREDS 6

typedef enum {
   stb_none,
   stb_z,
   stb_a,
   stb_c
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
