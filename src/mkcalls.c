/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993  William B. Ackerman.

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

/* mkcalls.c */

#include <stdio.h>

#ifdef _POSIX_SOURCE
#include <unistd.h>
#endif

/* We take pity on those poor souls who are compelled to use
    troglodyte development environments. */

#if defined(__STDC__) && !defined(athena_rt) && !defined(athena_vax)
#include <stdlib.h>
#else
extern void exit(int code);
#endif

#ifndef SEEK_SET		/* stdlib.h may not define it */
#define SEEK_SET 0
#endif

#include <string.h>
#include <errno.h>

#include "database.h"
#include "paths.h"

typedef enum {
   tok_string, tok_symbol, tok_lbkt, tok_rbkt, tok_number} toktype;


/* Several of the top-level flags have different meanings for calls
defined by array or defined by (concentric or sequential) definition.

    flag                   meaning by array                      meaning by (concentric or
                                                                   sequential) definition

left_means_mirror        The "left" modifier is legal          The "left" modifier is legal
                         and causes the call to be             and will be passed on to those
                         done mirror.                          subcalls that have the
                                                               "inherit_left" property set in
                                                               the subcall invocation.  Its
                                                               meaning in that subcall is
                                                               determined according to the way
                                                               that subcall is defined, by
                                                               applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

reverse_means_mirror     The "reverse" modifier is legal       The "reverse" modifier is legal
                         and causes the call to be             and will be passed on to those
                         done mirror.                          subcalls that have the
                         You may also use the                  "inherit_left" property set in
                         "reverse_coming_next"                 the subcall invocation.  Its
                         flag in the array list to             meaning in that subcall is
                         get a completely different            determined according to the way
                         meaning.  You should of               that subcall is defined, by
                         course not use both.                  applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

diamond_is_inherited     The "diamond" modifier is legal,      The "diamond" modifier is legal
                         and its meaning is determined by an   and will be passed on to those
                         item in the array list that has the   subcalls that have the
                         "alternate_definition [diamond]"      "inherit_diamond" property set in
                         flag.                                 the subcall invocation.  Its
                                                               meaning in that subcall is
                                                               determined according to the way
                                                               that subcall is defined, by
                                                               applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

magic_is_inherited       Has no meaning.  If the "magic"       The "magic" modifier is legal
                         modifier is given, its legality       and will be passed on to those
                         and meaning are determined by an      subcalls that have the
                         item in the array list that has the   "inherit_magic" property set in
                         "alternate_definition [magic]"        the subcall invocation.  Its
                         flag.                                 meaning in that subcall is
                                                               determined according to the way
                                                               that subcall is defined, by
                                                               applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

intlk_is_inherited       Has no meaning.  If the "interlocked" The "interlocked" modifier is legal
                         modifier is given, its legality       and will be passed on to those
                         and meaning are determined by an      subcalls that have the
                         item in the array list that has the   "inherit_intlk" property set in
                         "alternate_definition [interlocked]"  the subcall invocation.  Its
                         flag.                                 meaning in that subcall is
                                                               determined according to the way
                                                               that subcall is defined, by
                                                               applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

12_matrix_is_inherited   Has no meaning.  If the "12 matrix"   The "12 matrix" modifier is legal
                         modifier is given, its legality       and will be passed on to those
                         and meaning are determined by an      subcalls that have the
                         item in the array list that has the   "inherit_12_matrix" property set in
                         "alternate_definition [12matrix]"     the subcall invocation.  Its
                         flag.                                 meaning in that subcall is
                                                               determined according to the way
                                                               that subcall is defined, by
                                                               applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

funny_is_inherited       Has no meaning.  Use the              The "funny" modifier is legal
                         "simple_funny" flag at the            and will be passed on to those
                         head of each array list.              subcalls that have the
                         This will make "funny" legal          "inherit_funny" property set in
                         for that call from that setup.        the subcall invocation.  Its
                                                               meaning in that subcall is
                                                               determined according to the way
                                                               that subcall is defined, by
                                                               applying this rule recursively.
                                                               Presumably at least one subcall
                                                               has that property set.

Properties applied to a call are only legal (whether they arose from being clicked on at
the top level or being inherited from a higher level call) if they can be interpreted according
to one of the mechanisms described above.  For example, if we have:

call "scoot and plenty" c1
            left_means_mirror
            intlk_is_inherited
   seq scootback [inherit_intlk]
   seq plenty [inherit_left inherit_intlk]

then "left scoot and plenty" is only legal if "left plenty" is legal.
(The legality of "left scoot back" is irrelevant, since the "inherit_left"
property is not present on that invocation.)

Now if we have:

call "plenty" c1 tag plenty
            left_means_mirror
            intlk_is_inherited
   seq startplenty [inherit_left inherit_intlk]
   seq turnstar2 [or_anycall]
   seq finishplenty []

then "left scoot and plenty" is only legal if "left _start plenty" is legal.

Now if we have:

call "_start plenty" dontshow tag startplenty
            fudge_to_q_tag left_means_mirror
   setup qtag 2x4
      array [3S 4S 2S 2N 0 0 1S 1N]
               intlk_coming_next
   setup qtag 2x4
      array [3S 4S 1S 2N 0 0 2S 1N]

then "left scoot and plenty" is legal.  The "_start plenty" will be
done mirror, but the scoot back, star turn, and finish will not.
*/

/* A few notes about "definition modifiers":

These appear within the brackets after each call in a concdefine or seqdefine or whatever.
They are only meaningful as indicated in the table in database.h.  For example, repeat_n
is meaningful only in a "seq" definition, not in a "conc" definition.

Full documentation of these doesn't exist (and, when it does, it will belong elsewhere),
but the following notes need to be written down.

When "repeat_n_alternate" is used on a part of a "seq" definition, there MUST be
another part following.  No flags in that part are meaningful -- it gets its flags
from the first part.

When "roll_transparent" is used on a part of a "seq" definition, that call must
have the property that anyone who is marked roll-neutral (that is, with an "M" roll
letter), must not have moved at all, and must consider this part not to have applied
to him, so that the call "roll" will direct him to use the roll direction arising
from what he did previously.  The call "centers hinge" obeys this property, and so
the "roll_transparent" flag is appropriate.  The purpose of this flag is to make
the centers hinge part of peel/trail to a diamond work correctly, so that the
new points can roll.
*/



typedef struct {
   int def;
   char *s;
} tagtabitem;


/* This table is keyed to "level". */
char *leveltab[] = {
   "mainstream", "plus", "a1", "a2", "c1", "c2", "c3a", "c3", "c3x", "c4a", "c4", "dontshow", ""};

/* This table is keyed to "begin_kind". */
char *sstab[] = {
   "??",
   "1x1",
   "1x2",
   "2x1",
   "1x3",
   "3x1",
   "2x2",
   "dmd",
   "pmd",
   "star",
   "trngl",
   "ptrngl",
   "bone6",
   "pbone6",
   "short6",
   "pshort6",
   "qtag",
   "pqtag",
   "bone",
   "pbone",
   "rigger",
   "prigger",
   "spindle",
   "pspindle",
   "hrglass",
   "phrglass",
   "crosswave",
   "pcrosswave",
   "1x4",
   "4x1",
   "1x8",
   "8x1",
   "2x4",
   "4x2",
   "2x3",
   "3x2",
   "1x6",
   "6x1",
   "3x4",
   "4x3",
   "2x6",
   "6x2",
   "2x8",
   "8x2",
   "4x4",
   "1x10",
   "10x1",
   "1x12",
   "12x1",
   "1x14",
   "14x1",
   "1x16",
   "16x1",
   "c1phan",
   "galaxy",
   "4x6",
   "6x4",
   "thar",
   "ptpd",
   "pptpd",
   "3x1dmd",
   "p3x1dmd",
   "3dmd",
   "p3dmd",
   "4dmd",
   "p4dmd",
   ""};

/* This table is keyed to "setup_kind". */
char *estab[] = {
   "nothing",
   "1x1",
   "1x2",
   "1x3",
   "2x2",
   "dmd",
   "star",
   "trngl",
   "bone6",
   "short6",
   "qtag",
   "bone",
   "rigger",
   "spindle",
   "hrglass",
   "hyperglass",
   "crosswave",
   "1x4",
   "1x8",
   "2x4",
   "2x3",
   "1x6",
   "3x4",
   "2x6",
   "2x8",
   "4x4",
   "???",
   "1x10",
   "1x12",
   "1x14",
   "1x16",
   "c1phan",
   "bigblob",
   "ptpd",
   "3x1dmd",
   "3dmd",
   "4dmd",
   "wingedstar",
   "wingedstar12",
   "wingedstar16",
   "galaxy",
   "4x6",
   "thar",
   "???",
   "???",
   "normal_concentric",
   ""};

/* This table is keyed to "calldef_schema". */
char *schematab[] = {
   "conc",
   "crossconc",
   "singleconc",
   "singlecrossconc",
   "maybesingleconc",
   "conc_diamond_line",
   "conc6_2",
   "conc2_6",
   "conc_star",
   "conc_star12",
   "conc_star16",
   "maybematrix_conc_star",
   "checkpoint",
   "reverse_checkpoint",
   "ckpt_star",
   "???",
   "???",
   "seq",
   "splitseq",
   "setup",
   "nulldefine",
   "matrix",
   "partnermatrix",
   "rolldefine",
   ""};

/* This table is keyed to "search_qualifier". */
char *qualtab[] = {
   "none",
   "wave_only",
   "2fl_only",
   "miniwaves",
   "right_wave",
   "left_wave",
   "3_4_tag",
   "dmd_same_point",
   "dmd_facing",
   "true_Z",
   "ctrwv_end2fl",
   "ctr2fl_endwv",
   ""};

/* This table is keyed to "call_restriction". */
char *crtab[] = {
   "???",
   "alwaysfail",
   "wave_only",
   "wave_unless_say_2faced",
   "all_facing_same",
   "1fl_only",
   "2fl_only",
   "3x3_2fl_only",
   "4x4_2fl_only",
   "couples_only",
   "3x3couples_only",
   "4x4couples_only",
   "awkward_centers",
   "nice_diamonds",
   "magic_only",
   "peelable_box",
   "ends_are_peelable",
   "not_tboned",
   "opposite_sex",
   "quarterbox_or_col",
   "quarterbox_or_magic_col",
   ""};

/* This table is keyed to the constants "dfm_???". */
char *defmodtab[] = {
   "conc_demand_lines",
   "conc_demand_columns",
   "conc_force_lines",
   "conc_force_columns",
   "conc_force_otherway",
   "conc_force_spots",
   "conc_concentric_rules",
   "suppress_elongation_warnings",
   "or_anycall",
   "mandatory_anycall",
   "repeat_n",
   "repeat_nm1",
   "repeat_n_alternate",
   "endscando",
   "allow_forced_mod",
   "roll_transparent",
   "must_be_tag_call",
   "must_be_scoot_call",
   "cpls_unless_single",
   "??",
   "??",
   "inherit_diamond",
   "inherit_reverse",
   "inherit_left",
   "inherit_funny",
   "inherit_intlk",
   "inherit_magic",
   "inherit_grand",
   "inherit_12_matrix",
   "inherit_16_matrix",
   "inherit_cross",
   "inherit_single",
   ""};

/* This table is keyed to the constants "cflag__???". */
char *flagtab[] = {
   "step_to_wave",
   "rear_back_from_r_wave",
   "rear_back_from_qtag",
   "dont_use_in_resolve",
   "needselector",         /* This actually never appears in the text -- it is automatically added. */
   "neednumber",
   "sequence_starter",
   "split_like_square_thru",
   "split_like_dixie_style",
   "parallel_conc_end",
   "take_right_hands",
   "is_tag_call",
   "is_scoot_call",
   "is_star_call",
   "split_large_setups",
   "fudge_to_q_tag",
   "visible_fractions",
   "first_part_visible",
   "12_16_matrix_means_split",
   "??",
   "??",
   "diamond_is_legal",
   "reverse_means_mirror",
   "left_means_mirror",
   "funny_is_inherited",
   "intlk_is_inherited",
   "magic_is_inherited",
   "grand_is_inherited",
   "12_matrix_is_inherited",
   "16_matrix_is_inherited",
   "cross_is_inherited",
   "single_is_inherited",
   ""};

/* This table is keyed to the constants "cflag__???".
   Notice that it looks like the end of flagtab, with an offset defined here. */

#define NEXTTAB_OFFSET 21

char *nexttab[] = {
   "diamond",
   "reverse",
   "left",
   "???",
   "interlocked",
   "magic",
   "grand",
   "12matrix",
   "16matrix",
   "cross",
   "single",
   ""};

/* This table is keyed to the constants "MTX_???". */
char *matrixcallflagtab[] = {
   "use_selector",
   "stop_and_warn_on_tbone",
   "tbone_is_ok",
   "ignore_nonselectees",
   "must_face_same_way",
   ""};

/* BEWARE!!  This list must track the array "pred_table" in sdpreds.c . */

/* The first 6 of these (the constant to use is SELECTOR_PREDS) take a predicate.
   Any call that uses one of these predicates in its definition will cause a
   popup to appear asking "who?". */

char *predtab[] = {
   "select",
   "unselect",
   "select_near_select",
   "select_near_unselect",
   "unselect_near_select",
   "unselect_near_unselect",
   "always",
   "x22_miniwave",
   "x22_couple",
   "x22_facing_someone",           /* does anyone use this? */
   "x14_once_rem_miniwave",
   "x14_once_rem_couple",
   "lines_miniwave",
   "lines_couple",
   "lines_magic_miniwave",
   "lines_magic_couple",
   "lines_once_rem_miniwave",
   "lines_once_rem_couple",
   "lines_tandem",
   "lines_antitandem",
   "columns_tandem",
   "columns_antitandem",
   "columns_magic_tandem",
   "columns_magic_antitandem",
   "columns_once_rem_tandem",
   "columns_once_rem_antitandem",
   "columns_couple",
   "columns_miniwave",
   "1x2_beau_or_miniwave",
   "1x4_wheel_and_deal",
   "1x6_wheel_and_deal",
   "1x8_wheel_and_deal",
   "vert1",
   "vert2",
   "inner_active_lines",
   "outer_active_lines",
   "judge_is_right",
   "judge_is_left",
   "socker_is_right",
   "socker_is_left",
   "inroller_is_cw",
   "magic_inroller_is_cw",
   "outroller_is_cw",
   "outposter_is_cw",
   "outposter_is_ccw",
   "nexttrnglspot_is_tboned",
   "next62spot_is_tboned",
   "next_magic62spot_is_tboned",
   "next_galaxyspot_is_tboned",
   "column_double_down",
   "boyp",
   "girlp",
   "roll_is_cw",
   "roll_is_ccw",
   "x12_boy_facing_girl",
   "x12_girl_facing_boy",
   "x22_boy_facing_girl",
   "x22_girl_facing_boy",
   "dmd_ctrs_rh",
   "trngl_pt_rh",
   "q_tag_front",
   "q_tag_back",
   "q_line_front",
   "q_line_back",
   ""};

/* BEWARE!!  This list is keyed to the definition of "begin_kind" in sd.h . */
/*   It must also match the similar table in the sdtables.c. */
int begin_sizes[] = {
   0,          /* b_nothing */
   1,          /* b_1x1 */
   2,          /* b_1x2 */
   2,          /* b_2x1 */
   3,          /* b_1x3 */
   3,          /* b_3x1 */
   4,          /* b_2x2 */
   4,          /* b_dmd */
   4,          /* b_pmd */
   4,          /* b_star */
   6,          /* b_trngl */
   6,          /* b_ptrngl */
   6,          /* b_bone6 */
   6,          /* b_pbone6 */
   6,          /* b_short6 */
   6,          /* b_pshort6 */
   8,          /* b_qtag */
   8,          /* b_pqtag */
   8,          /* b_bone */
   8,          /* b_pbone */
   8,          /* b_rigger */
   8,          /* b_prigger */
   8,          /* b_spindle */
   8,          /* b_pspindle */
   8,          /* b_hrglass */
   8,          /* b_phrglass */
   8,          /* b_crosswave */
   8,          /* b_pcrosswave */
   4,          /* b_1x4 */
   4,          /* b_4x1 */
   8,          /* b_1x8 */
   8,          /* b_8x1 */
   8,          /* b_2x4 */
   8,          /* b_4x2 */
   6,          /* b_2x3 */
   6,          /* b_3x2 */
   6,          /* b_1x6 */
   6,          /* b_6x1 */
   12,         /* b_3x4 */
   12,         /* b_4x3 */
   12,         /* b_2x6 */
   12,         /* b_6x2 */
   16,         /* b_2x8 */
   16,         /* b_8x2 */
   16,         /* b_4x4 */
   10,         /* b_1x10 */
   10,         /* b_10x1 */
   12,         /* b_1x12 */
   12,         /* b_12x1 */
   14,         /* b_1x14 */
   14,         /* b_14x1 */
   16,         /* b_1x16 */
   16,         /* b_16x1 */
   16,         /* b_c1phan */
   8,          /* b_galaxy */
   24,         /* b_4x6 */
   24,         /* b_6x4 */
   8,          /* b_thar */
   8,          /* b_ptpd */
   8,          /* b_pptpd */
   8,          /* b_3x1dmd */
   8,          /* b_p3x1dmd */
   12,         /* b_3dmd */
   12,         /* b_p3dmd */
   16,         /* b_4dmd */
   16};        /* b_p4dmd */


/* The "tag table" is the table that we use to bind together things like

            "seq flipdiamond []"

   in a definition, and

            call "flip the diamond" plus tag flipdiamond

   in another definition.  We keep a list of the strings, and turn
   each tag into a number that is its index in the list.  The binary
   database deals with these numbers.  The main sd program will make
   its own copy of the table, containing pointers to the actual call
   descriptors. */

int tagtabsize = 2;      /* Number of items we currently have in tagtab -- we initially have two; see below. */
int tagtabmax = 100;     /* Amount of space allocated for tagtab; must be >= tagtabsize at all times, obviously. */

tagtabitem *tagtab;      /* The dynamically allocated tag list. */

tagtabitem tagtabinit[] = {
      {1, "+++"},         /* Must be unused -- call #0 signals end of list in sequential encoding. */
      {0, "nullcall"}};   /* Must be next -- the database initializer uses call #1 for any mandatory
                                    modifier, e.g. "clover and [anything]" is executed as
                                    "clover and [call #1]". */

int eof;
int chars_left;
char *lineptr;
int linelen;
int lineno;
int tok_value;
int letcount;
toktype tok_kind;
char tok_str[80];
int char_ct;

FILE *infile;
FILE *outfile;
char line[200];
char ch;
char *return_ptr;
int callcount;
int filecount;
int dumbflag;
int call_flags;
int call_tag;
char call_name[80];
int call_namelen;
int call_level;
int call_startsetup;
int call_qualifier;
int call_endsetup;
int call_endsetup_in;
int call_endsetup_out;
int bmatrix, gmatrix;
int restrstate;
int callarray_flags1;
int callarray_flags2;


static void errexit(char s[])
{
   char my_line[80];
   int i;

   if (!call_namelen) {
      if (eof) {
         printf("Error at line %d:\n%s.\n", lineno, s);
      }
      else {
         strncpy(my_line, lineptr, linelen-1);
         my_line[linelen-1] = '\0';
         printf("Error at line %d:\n%s.\n%s\n", lineno, s, my_line);
         for (i = 1; i <= linelen-chars_left-1; i++) printf(" ");
         printf("|\n");
      }
   }
   else {
      if (eof) {
         printf("Error at line %d:\n%s.  Last call was: %s\n", lineno, s, call_name);
      }
      else {
         strncpy(my_line, lineptr, linelen-1);
         my_line[linelen-1] = '\0';
         printf("Error at line %d:\n%s.  Last call was: %s\n%s\n", lineno, s, call_name, my_line);
         for (i = 1; i <= linelen-chars_left-1; i++) printf(" ");
         printf("|\n");
      }
   }

   exit(1);
}


static int get_char(void)
{
   if (!chars_left) {
      lineno++;
      return_ptr = fgets(line, 199, infile);
      lineptr = return_ptr;
      linelen = strlen(line);
      if (!return_ptr) {
         if (feof(infile)) {
            eof = 1;
            return 1;
         }
         else {
            perror("Can't read input file");
            exit(1);
         }
      }
      chars_left = linelen;
   }
   ch = *return_ptr++;
   chars_left--;
   return 0;
}

static int symchar(void)
{
   if (ch == '[' || ch == ']' || ch == ',' || ch == ':' || (int)ch <= 32) return 0;
   else return 1;
}

static void get_tok_or_eof(void)
{
   int digit;

   char_ct = 0;
   tok_value = 0;
   letcount = 0;
   while ((ch == ' ') || (ch == '/') || (ch == '\n')) {
      if (ch == '/') {
         if (get_char()) errexit("End of file in comment starter");
         if (ch != '*') errexit("Incorrect comment starter");
         while (1) {
            if (get_char()) errexit("End of file inside comment");
            if (ch == '*') {
               while (1) {
                  if (get_char()) errexit("End of file inside comment");
                  if (ch != '*') break;
               }
               if (ch == '/') break;
            }
         }
      }
      if (get_char()) return;
   }
   /* Now have a real character. */
   switch (ch) {
      case '[': tok_kind = tok_lbkt; ch = ' '; break;
      case ']': tok_kind = tok_rbkt; ch = ' '; break;
      case '"':
         while (1) {
            if (get_char()) {
               printf("end of file inside string\n");
               exit(1);
            }
            if (ch == '"') break;
            else if (ch == '\\') {
               if (get_char()) {
                  printf("end of file inside string\n");
                  exit(1);
               }
            }
            if (char_ct > 80) {
               printf("String too long\n");
               exit(1);
            }
            tok_str[char_ct++] = ch;
         }

         ch = ' ';
         tok_kind = tok_string;

         /* Pack a null. */

         if (char_ct > 80) {
            printf("String too long\n");
            exit(1);
         }
         tok_str[char_ct] = '\0';
         break;
      default:
         while (1) {
            if (char_ct > 80) {
               printf("Symbol too long\n");
               exit(1);
            }
            tok_str[char_ct++] = ch;

            digit = ch - '0';
            if (digit < 0 || digit > 9) letcount++;
            else tok_value = tok_value*10 + digit;

            if (get_char()) {
               printf("End of file inside symbol\n");
               exit(1);
            }

            if (!symchar()) break;
         }

         /* Pack a null. */

         if (char_ct > 80) {
            printf("Symbol too long\n");
            exit(1);
         }
         tok_str[char_ct] = '\0';

         if (letcount)
            tok_kind = tok_symbol;
         else
            tok_kind = tok_number;
         break;
   }
}


/* This returns -1 if the item is not found. */

static int search(char *table[])
{
   int i;

   i = -1;
   while (*table[++i]) {
      if (strcmp(tok_str, table[i]) == 0)
         return i;
   }
   return -1;
}


static int tagsearch(int def)
{
   int i;

   for (i = 0; i < tagtabsize; i++) {
      if (!strcmp(tok_str, tagtab[i].s)) goto done;
   }

   i = tagtabsize++;
   if (i >= tagtabmax) {
      tagtabmax <<= 1;
      tagtab = (tagtabitem *) realloc(tagtab, tagtabmax * sizeof(tagtabitem));
      if (!tagtab) errexit("Out of memory!!!!!!");
   }

   tagtab[i].s = (char *) malloc(strlen(tok_str)+1);
   if (!tagtab[i].s) errexit("Out of memory!!!!!!");

   strcpy(tagtab[i].s, tok_str);
   tagtab[i].def = 0;

   done:
   if (def) {
      if (tagtab[i].def) errexit("Multiple definition of a call tag");
      tagtab[i].def = 1;
   }
   return i;
}


static void get_tok(void)
{
   get_tok_or_eof();
   if (eof) errexit("Unexpected end of file");
}


static int get_num(char s[])
{
   get_tok();
   if ((tok_kind != tok_number)) errexit(s);
   return tok_value;
}


static int dfmsearch(void)
{
   int i;
   int rrr = 0;

   get_tok();
   if (tok_kind != tok_lbkt)
      errexit("Missing left bracket in defmod list");

   get_tok();
   if (tok_kind != tok_rbkt) {
      while (1) {
         if ((tok_kind != tok_symbol))
            errexit("Improper defmod key");

         if ((i = search(defmodtab)) < 0) errexit("Unknown defmod key");
         rrr |= (1 << i);

         get_tok();
         if (tok_kind == tok_rbkt) break;
      }
   }

   return rrr;
}




static void write_halfword(int n)
{
   fputc((n>>8) & 0xFF, outfile);
   fputc((n) & 0xFF, outfile);
   filecount += 2;
}



static void write_fullword(int n)
{
   fputc((n>>24) & 0xFF, outfile);
   fputc((n>>16) & 0xFF, outfile);
   fputc((n>>8) & 0xFF, outfile);
   fputc((n) & 0xFF, outfile);
   filecount += 4;
}



static void write_callarray(int num, int doing_matrix)
{
   int count;

   if (tok_kind != tok_lbkt)
      errexit("Missing left bracket in callarray list");

   for (count=0; ; count++) {
      int dat = 0;
      int p = 0;
      stability stab = stb_none;

      get_tok();
      if (tok_kind == tok_rbkt) break;
      else if (tok_kind == tok_number && tok_value == 0)
         write_halfword(0);
      else if (tok_kind == tok_symbol) {
         if (letcount > 1) {
            if      (tok_str[0] == 'Z' || tok_str[0] == 'z') { stab = stb_z; p++; }
            else if (tok_str[0] == 'A' || tok_str[0] == 'a') { stab = stb_a; p++; }
            else if (tok_str[0] == 'C' || tok_str[0] == 'c') { stab = stb_c; p++; }
         }

         if (letcount-p == 2) {
            if (tok_str[p] == 'L' || tok_str[p] == 'l') dat = 4;
            else if (tok_str[p] == 'M' || tok_str[p] == 'm') dat = 2;
            else if (tok_str[p] == 'R' || tok_str[p] == 'r') dat = 1;
            else errexit("Improper callarray specifier");
         }
         else if (letcount-p != 1)
            errexit("Improper callarray specifier");

         if (doing_matrix) {
            dat = (dat << 12) | (tok_value << 7);
         }
         else {
            dat = (dat * DBROLL_BIT) | (tok_value << 4) | (((unsigned int) stab) * DBSTAB_BIT);
         }

         /* We now have roll indicator and position, need to get direction. */
         switch (tok_str[char_ct-1]) {
            case 'N': case 'n': dat |= 010; break;
            case 'E': case 'e': dat |= 001; break;
            case 'S': case 's': dat |= 012; break;
            case 'W': case 'w': dat |= 003; break;
            default:
               errexit("Improper callarray direction specifier");
         }

         if (doing_matrix) {
            dat &= ~0x7C;
            dat |= (get_num("Improper y coordinate") & 0x1F) << 2;
         }

         write_halfword(dat);
      }
      else
         errexit("Improper callarray element");
   }

   if (count < num) errexit("Callarray list is too short for this call");
   else if (count > num) errexit("Callarray list is too long for this call");
}


static void write_call_header(calldef_schema schema)
{
   int i, j;

   i = call_namelen;
   write_halfword(0x2000 | call_tag);
   write_halfword(call_level);
   write_fullword(call_flags);
   write_halfword((i << 8) | (int)schema);
   for (j = 1; j <= (i >> 1); j++)
      write_halfword((((int)(call_name[2*j-2])) << 8) | (int)(call_name[2*j-1]));
   if (i&1)
      write_halfword(((int)(call_name[i-1])) << 8);

   callcount++;
}


static void write_conc_stuff(calldef_schema schema)
{
   int defin, minn, defout, mout;

   write_call_header(schema);

   /* Write two level 2 concdefine groups. */

   get_tok();
   if ((tok_kind != tok_symbol)) errexit("Improper conc symbol");
   defin = tagsearch(0);

   minn = dfmsearch();

   get_tok();
   if ((tok_kind != tok_symbol)) errexit("Improper conc symbol");
   defout = tagsearch(0);

   mout = dfmsearch();

   write_halfword(0x4000 | defin);
   write_fullword(minn);
   write_halfword(0x4000 | defout);
   write_fullword(mout);
}


static void write_seq_stuff(void)
{
   int deff, mods;

   get_tok();
   if ((tok_kind != tok_symbol)) errexit("Improper seq symbol");
   deff = tagsearch(0);

   mods = dfmsearch();

   write_halfword(0x4000 | deff);
   write_fullword(mods);
}


static void write_level_3_group(int predbit)
{
   write_halfword(0x6000 | predbit | callarray_flags1 | callarray_flags2);
   write_halfword(call_startsetup | (call_qualifier << 8));
   if (callarray_flags1 & CAF__CONCEND) {
      write_halfword(call_endsetup_in | (restrstate << 8));
      write_halfword(call_endsetup_out);
   }
   else {
      write_halfword(call_endsetup | (restrstate << 8));
   }
}


static void write_array_def(void)
{
   int i, iii, jjj;

   write_call_header(schema_by_array);

   /* Write a level 2 array define group. */
   write_halfword(0x4000);
def2:
   restrstate = 0;
   callarray_flags2 = 0;
   call_qualifier = 0;

   get_tok();
   if ((tok_kind != tok_symbol)) errexit("Improper starting setup");
   if ((call_startsetup = search(sstab)) < 0) errexit("Unknown start setup");

   get_tok();
   if ((tok_kind != tok_symbol)) errexit("Improper ending setup");
   if ((call_endsetup = search(estab)) < 0) errexit("Unknown ending setup");

   /* Should actually look for anomalous concentric and do it specially */

   while (1) {
      get_tok();
      if ((tok_kind != tok_symbol)) errexit("Missing indicator");

      if (!strcmp(tok_str, "array")) {
         write_level_3_group(0);             /* Pred flag off. */
         get_tok();
         write_callarray(begin_sizes[call_startsetup], 0);
         get_tok_or_eof();
         break;
      }
      else if (!strcmp(tok_str, "preds")) {
         write_level_3_group(CAF__PREDS);        /* Pred flag on. */
         get_tok();
         if (tok_kind != tok_string) errexit("Missing string");
         iii = char_ct;
         write_halfword(iii);
         for (jjj = 1; jjj <= (iii >> 1); jjj++)
            write_halfword((((int)(tok_str[2*jjj-2])) << 8) | ((int)(tok_str[2*jjj-1])));
         if (iii&1)
            write_halfword(((int)(tok_str[iii-1])) << 8);

         while (1) {
            get_tok_or_eof();
            if (eof) break;

            if ((tok_kind != tok_symbol)) break;    /* Will give an error. */

            if (!strcmp(tok_str, "if")) {
               get_tok();
               if ((tok_kind != tok_symbol)) errexit("Improper predicate");
               if ((iii = search(predtab)) < 0) errexit("Unknown predicate");

               /* Write a level 4 group. */
               write_halfword(0x8000);
               write_halfword(iii);
               get_tok();
               write_callarray(begin_sizes[call_startsetup], 0);
            }
            else
               break;
         }

         break;
      }
      else if (!strcmp(tok_str, "qualifier")) {
         get_tok();
         if ((tok_kind != tok_symbol)) errexit("Improper qualifier");
         if ((call_qualifier = search(qualtab)) < 0) errexit("Unknown qualifier");
      }
      else if (!strcmp(tok_str, "restriction")) {
         get_tok();
         if ((tok_kind != tok_symbol)) errexit("Improper restriction specifier");

         if (!strcmp(tok_str, "unusual")) {
            callarray_flags2 |= CAF__RESTR_UNUSUAL;
            get_tok();
            if ((tok_kind != tok_symbol)) errexit("Improper restriction specifier");
         }
         else if (!strcmp(tok_str, "forbidden")) {
            callarray_flags2 |= CAF__RESTR_FORBID;
            get_tok();
            if ((tok_kind != tok_symbol)) errexit("Improper restriction specifier");
         }
         else if (!strcmp(tok_str, "resolve_ok")) {
            callarray_flags2 |= CAF__RESTR_RESOLVE_OK;
            get_tok();
            if ((tok_kind != tok_symbol)) errexit("Improper restriction specifier");
         }

         if ((restrstate = search(crtab)) < 0) errexit("Unknown restriction specifier");
      }
      else if (!strcmp(tok_str, "rotate")) {
         callarray_flags1 |= CAF__ROT;
      }
      else if ((!(callarray_flags1 & CAF__CONCEND)) && (!strcmp(tok_str, "concendsetup"))) {
         if (call_endsetup != (int)s_normal_concentric)
            errexit("concendsetup with wrong end_setup");
         get_tok();
         if ((tok_kind != tok_symbol)) errexit("Improper setup specifier");
         if ((call_endsetup_in = search(estab)) < 0) errexit("Unknown setup specifier");

         get_tok();
         if ((tok_kind != tok_symbol)) errexit("Improper setup specifier");
         if ((call_endsetup_out = search(estab)) < 0) errexit("Unknown setup specifier");

         jjj = get_num("Improper second rotation");
         if (jjj != (jjj & 1))
            errexit("Improper second rotation");
         if (jjj) callarray_flags1 |= CAF__ROT_OUT;

         callarray_flags1 |= CAF__CONCEND;
      }
      else
         errexit("Item in illegal context while parsing setups/preds/arrays");
   }

   if (eof) return;
   if ((tok_kind != tok_symbol)) errexit("Missing indicator");

   /* If see "setup", just do another basic array. */

   if (!strcmp(tok_str, "setup")) {
      callarray_flags1 = 0;
      goto def2;
   }

   /* Otherwise, it's either an alternate definition to start another group
      of arrays, or it's the end of the whole call.
      We make use of the fact that the constants "cflag__reverse_means_mirror"
      etc. all lie in the left half of the word.  Sdinit.c checks that. */

   if (!strcmp(tok_str, "alternate_definition")) {
      int rrr;
      int alt_level;
      rrr = 0;

      get_tok();
      if (tok_kind != tok_lbkt)
         errexit("Missing left bracket in alternate_definition list");

      get_tok();
      if (tok_kind != tok_rbkt) {
         while (1) {
            if ((tok_kind != tok_symbol))
               errexit("Improper alternate_definition key");

            if ((i = search(nexttab)) < 0) errexit("Unknown alternate_definition key");
            rrr |= (1 << i);

            get_tok();
            if (tok_kind == tok_rbkt) break;
         }
      }

      get_tok();
      if ((tok_kind != tok_symbol)) errexit("Improper alternate_definition level");
      if ((alt_level = search(leveltab)) < 0) errexit("Unknown alternate_definition level");

      write_halfword(0x4000 | alt_level);
      write_halfword(rrr << (NEXTTAB_OFFSET-16));
   }
   else
      return;       /* Must have seen next 'call' indicator. */

   /* Saw "alternate_definition", ready to do another group of arrays. */

   get_tok();
   if ((tok_kind != tok_symbol)) errexit("Missing indicator");

   callarray_flags1 = 0;
   if (!strcmp(tok_str, "simple_funny")) {
      callarray_flags1 = CAF__FACING_FUNNY;
      get_tok();
      if ((tok_kind != tok_symbol)) errexit("Missing indicator");
   }

   if (!strcmp(tok_str, "setup")) {
      callarray_flags1 = 0;
      goto def2;
   }
   else
      errexit("Need \"setup\" indicator");
}



void main(void)
{
   int i, iii;

   infile = fopen(CALLS_FILENAME, "r");
   if (!infile) {
      printf("Can't open input file\n");
      perror(CALLS_FILENAME);
      exit(1);
   }

   eof = 0;
   lineno = 0;
   chars_left = 0;
   ch = ' ';
   call_namelen = 0;   /* So won't print error first time around. */

   if (remove(DATABASE_FILENAME)) {
      if (errno != ENOENT) {
         printf("Error deleting old output file\n");
         perror(DATABASE_FILENAME);
      }
   }

   /* The "b" in the mode is meaningless and harmless in POSIX.  Some systems,
      however, require it for correct handling of binary data. */
   outfile = fopen(DATABASE_FILENAME, "wb");
   if (!outfile) {
      printf("Can't open output file\n");
      perror(DATABASE_FILENAME);
      exit(1);
   }

   tagtab = (tagtabitem *) malloc(tagtabmax * sizeof(tagtabitem));
   if (!tagtab) {
      printf("Can't allocate memory\n");
      exit(1);
   }
   memcpy(tagtab, tagtabinit, sizeof(tagtabinit));   /* initialize first two entries in tagtab */

   filecount = 0;

   write_halfword(DATABASE_MAGIC_NUM);
   write_halfword(DATABASE_FORMAT_VERSION);
   /* Next two halfwords in the file will be the call count and the tag table size.
      The latter tells sd how much memory to allocate internally for its reconstruction
      of the tag table. */
   write_halfword(0);
   write_halfword(0);

   get_tok();
   if (strcmp(tok_str, "version")) errexit("Missing version specification");
   get_tok();
   if (tok_kind != tok_number) errexit("Improper major version specification");
   write_halfword(tok_value);
   get_tok();
   if (tok_kind != tok_number) errexit("Improper minor version specification");
   write_halfword(tok_value);

   callcount = 0;
   while (1) {

      get_tok_or_eof();
   startagain:
      if (eof) break;

      if ((tok_kind != tok_symbol)) errexit("Missing indicator");

      if (!strcmp(tok_str, "call")) {
         int count;
         int matrixflags;
         int bit;
         calldef_schema ccc;

         get_tok();
         if (tok_kind != tok_string) errexit("Improper call name");

         strcpy(call_name, tok_str);
         call_namelen = char_ct;
         call_flags = 0;
         call_tag = 0;

         get_tok();
         if ((tok_kind != tok_symbol)) errexit("Improper level");
         if ((call_level = search(leveltab)) < 0) errexit("Unknown level");

         /* Get toplevel options. */

         while (1) {
            get_tok();
            if ((tok_kind != tok_symbol)) errexit("Missing indicator");
            if (!strcmp(tok_str, "tag")) {
               get_tok();
               if ((tok_kind != tok_symbol)) errexit("Improper tag");
               call_tag = tagsearch(1);
            }
            else {
               iii = search(flagtab);
               if (iii < 0) break;
               call_flags |= (1 << iii);
            }
         }

         /* Process the actual definition.  First, check for the "simple_funny" indicator. */

         callarray_flags1 = 0;

         if (!strcmp(tok_str, "simple_funny")) {
            callarray_flags1 = CAF__FACING_FUNNY;
            get_tok();
            if ((tok_kind != tok_symbol)) errexit("Missing indicator");
         }

         /* Find out what kind of call it is. */
         iii = search(schematab);

         if (iii < 0) errexit("Can't determine call definition type");

         ccc = (calldef_schema) iii;

         if (callarray_flags1 != 0 && ccc != schema_by_array)
            errexit("Simple_funny out of place");


         switch(ccc) {
            case schema_concentric:
            case schema_cross_concentric:
            case schema_single_concentric:
            case schema_single_cross_concentric:
            case schema_maybe_single_concentric:
            case schema_concentric_diamond_line:
            case schema_concentric_6_2:
            case schema_concentric_2_6:
            case schema_conc_star:
            case schema_conc_star12:
            case schema_conc_star16:
            case schema_maybe_matrix_conc_star:
            case schema_checkpoint:
            case schema_rev_checkpoint:
            case schema_ckpt_star:
               write_conc_stuff(ccc);
               break;
            case schema_sequential:
            case schema_split_sequential:
               count = 0;
   
               write_call_header(ccc);
               /* Write a level 2 seqdefine group. */
   
               write_seq_stuff();
               while (1) {
                  get_tok_or_eof();
                  if (eof) break;
                  if ((tok_kind == tok_symbol) && (!strcmp(tok_str, "seq"))) {
                     /* Write a level 2 seqdefine group. */
                     if (++count == SEQDEF_MAX) errexit("Too many parts in sequential definition");
                     write_seq_stuff();
                  }
                  else
                     break;       /* Must have seen next 'call' indicator. */
               }
               goto startagain;
            case schema_by_array:
               write_array_def();
               goto startagain;
            case schema_nothing:
               write_call_header(ccc);
               break;
            case schema_matrix:
               matrixflags = 0;
   
               write_call_header(ccc);
               write_halfword(matrixflags >> 8);
               write_halfword(matrixflags);
               get_tok();
               write_callarray(2, 2);
               break;
            case schema_partner_matrix:
               matrixflags = 0;
   
               /* Get partner matrix call options. */
   
               while (1) {
                  get_tok();
                  if ((tok_kind != tok_symbol)) break;
                  if ((bit = search(matrixcallflagtab)) < 0) errexit("Unknown matrix call flag");
                  matrixflags |= (1 << bit);
               }
   
               if (matrixflags & MTX_USE_SELECTOR) call_flags |= cflag__requires_selector;
               write_call_header(ccc);
               write_halfword(matrixflags >> 8);
               write_halfword(matrixflags);
               write_callarray(8, 1);
               break;
            case schema_roll:
               write_call_header(ccc);
               break;
            default:
               errexit("Can't determine call definition type");
         }
      }
      else
         errexit("Item in illegal context");
   }

   write_halfword(0);         /* final end mark */

   dumbflag = 0;

   for (i = 0; i < tagtabsize; i++) {
      if (!tagtab[i].def) {
         if (!dumbflag++) printf("Tags not defined:\n");
         printf("   %s\n", tagtab[i].s);
      }
   }

   if (fclose(infile)) {
      perror("Can't close input file");
      exit(1);
   }

   printf("%d bytes written, %d calls\n", filecount, callcount);

   if (fseek(outfile, 4, SEEK_SET)) {
      perror("Can't seek output file back to beginning");
      exit(1);
   }

   write_halfword(callcount);
   write_halfword(tagtabsize);

   if (fclose(outfile)) {
      perror("Can't close output file");
      exit(1);
   }

   exit(0);
}
