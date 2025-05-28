/* SD -- square dance caller's helper.

    Copyright (C) 1990-1994  William B. Ackerman.

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

/* dbcomp.c */

/* ***  This next test used to be
    ifdef _POSIX_SOURCE
   We have taken it out and replaced with what you see below.  If this breaks
   anything, let us know. */
#if defined(_POSIX_SOURCE) || defined(sun)
#include <unistd.h>
#endif

/* We take pity on those poor souls who are compelled to use
    troglodyte development environments. */

/* ***  This next test used to be
    if defined(__STDC__) && !defined(athena_rt) && !defined(athena_vax)
   We have taken it out and replaced with what you see below.  If this breaks
   anything, let us know. */
#if defined(__STDC__) || defined(sun)
#include <stdlib.h>
#else
extern void free(void *ptr);
extern char *malloc(unsigned int siz);
extern char *realloc(char *oldp, unsigned int siz);
extern void exit(int code);
#endif

#include <string.h>

#include "database.h"
#include "paths.h"

/* We would like to think that we will always be able to count on compilers to do the
   right thing with "int" and "long int" and so on.  What we would really like is
   for compilers to be counted on to make "int" at least 32 bits, because we need
   32 bits in many places.  However, some compilers don't, so we have to use
   "long int" or "unsigned long int".  We think that all compilers we deal with
   will do the right thing with that, but, just in case, we use a typedef.

   The type "uint32" must be an unsigned integer of at least 32 bits. */

typedef unsigned long int uint32;

/* These things come from mkcalls.c for the standalone compiler, or from
   sdtables.c or sdui-mac.c for the built-in compiler. */
extern int begin_sizes[];
extern void do_exit(void);
extern void dbcompile_signoff(int bytes, int calls);
extern int do_printf(char *fmt, ...);
extern char *db_gets(char *s, int n);
extern void db_putc(char ch);
extern void db_rewind_output(int pos);
extern void db_close_input(void);
extern void db_close_output(void);

extern void dbcompile(void);

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
   "trngl4",
   "ptrngl4",
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
   "dhrglass",
   "pdhrglass",
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
   "bigdmd",
   "pbigdmd",
   ""};

/* This table is keyed to "setup_kind". */
char *estab[] = {
   "nothing",
   "1x1",
   "1x2",
   "1x3",
   "2x2",
   "1x4",
   "dmd",
   "star",
   "trngl",
   "trngl4",
   "bone6",
   "short6",
   "qtag",
   "bone",
   "rigger",
   "spindle",
   "hrglass",
   "dhrglass",
   "hyperglass",
   "crosswave",
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
   "???",
   "???",
   "bigdmd",
   "???",
   "normal_concentric",
   ""};

/* This table is keyed to "calldef_schema". */
char *schematab[] = {
   "conc",
   "crossconc",
   "singleconc",
   "singlecrossconc",
   "singleconc_together",
   "maybesingleconc",
   "maybesinglecrossconc",
   "conc_diamond_line",
   "conc6_2",
   "conc2_6",
   "conc6_2_tgl",
   "conc_star",
   "conc_star12",
   "conc_star16",
   "maybematrix_conc_star",
   "checkpoint",
   "reverse_checkpoint",
   "ckpt_star",
   "conc_triple_lines",
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
   "in_or_out",
   "miniwaves",
   "right_wave",
   "left_wave",
   "3_4_tag",
   "dmd_same_point",
   "dmd_facing",
   "true_Z",
   "ctrwv_end2fl",
   "ctr2fl_endwv",
   "split_dixie",
   "not_split_dixie",
   "8_chain",
   "trade_by",
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
   "leads_only",
   "couples_only",
   "3x3couples_only",
   "4x4couples_only",
   "awkward_centers",
   "diamond_like",
   "qtag_like",
   "nice_diamonds",
   "magic_only",
   "peelable_box",
   "ends_are_peelable",
   "not_tboned",
   "opposite_sex",
   "quarterbox_or_col",
   "quarterbox_or_magic_col",
   "???",
   "???",
   "???",
   "???",
   "???",
   "???",
   "???",
   ""};

/* This table is keyed to the constants "DFM1_***".  These are the general
   definition-modifier flags.  They go in the "modifiers1" word of a by_def_item. */
char *defmodtab1[] = {
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
   "allow_forced_mod",
   "repeat_n",
   "repeat_n_alternate",
   "endscando",
   "repeat_nm1",
   "roll_transparent",
   "must_be_tag_call",
   "cpls_unless_single",
   "shift_one_number",
   "shift_two_numbers",     /* The constant "shift_three_numbers" is elsewhere. */
   ""};

/* This table is keyed to the constants "CFLAG1_***".  These are the
   general top-level call flags.  They go into the "callflags1" word. */

char *flagtab1[] = {
   "visible_fractions",
   "first_part_visible",
   "12_16_matrix_means_split",
   "imprecise_rotation",
   "split_like_dixie_style",
   "parallel_conc_end",
   "take_right_hands",
   "is_tag_call",
   "is_star_call",
   "split_large_setups",
   "fudge_to_q_tag",
   "step_to_wave",
   "rear_back_from_r_wave",
   "rear_back_from_qtag",
   "dont_use_in_resolve",
   "needselector",         /* This actually never appears in the text -- it is automatically added. */
   "neednumber",
   "need_two_numbers",     /* The constant "need_three_numbers" is elsewhere. */
   "need_four_numbers",
   "sequence_starter",
   "split_like_square_thru",
   "finish_means_skip_first_part",
   "need_direction",       /* This actually never appears in the text -- it is automatically added. */
   "left_means_touch_or_check",
   "can_be_fan_or_yoyo",
   "no_cutting_through",
   "no_elongation_allowed",
   "need_tag_call",        /* This actually never appears in the text -- it is automatically added. */
   "base_tag_call",
   ""};

/* The next three tables are all in step with each other, and with the "heritable" flags. */

/* This table is keyed to the constants "cflag__???".  The bits indicated by it
   are encoded into the "callflags" word of the top-level call descriptor. */
char *flagtabh[] = {
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
   "1x2_is_inherited",
   "2x1_is_inherited",
   "2x2_is_inherited",
   "1x3_is_inherited",
   "3x1_is_inherited",
   "3x3_is_inherited",
   "4x4_is_inherited",
   "singlefile_is_inherited",
   "half_is_inherited",
   ""};

/* This table is keyed to the constants "cflag__???".
   Notice that it looks like flagtabh. */
char *altdeftabh[] = {
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
   "1x2",
   "2x1",
   "2x2",
   "1x3",
   "3x1",
   "3x3",
   "4x4",
   "singlefile",
   "half",
   ""};

/* This table is keyed to the constants "dfm_***".  These are the heritable
   definition-modifier flags.  They go in the "modifiersh" word of a by_def_item.
   Notice that it looks like flagtabh. */
char *defmodtabh[] = {
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
   "inherit_1x2",
   "inherit_2x1",
   "inherit_2x2",
   "inherit_1x3",
   "inherit_3x1",
   "inherit_3x3",
   "inherit_4x4",
   "inherit_singlefile",
   "inherit_half",
   ""};

/* This table is keyed to the constants "dfm_***".  These are the heritable
   definition-modifier force flags.  They go in the "modifiersh" word of a by_def_item.
   These are the words that one uses if the top-level enabling bit is OFF.  The
   bit in the "modifiersh" is the same in either case -- the different interpretation
   simply depends on whether the top level bit is on or off.
   Notice that it looks like flagtabh. */
char *forcetabh[] = {
   "force_diamond",
   "???",    /* We don't allow "reverse" or "left" -- the bits move around during inheritance. */
   "???",
   "force_funny",
   "force_intlk",
   "force_magic",
   "force_grand",
   "force_12_matrix",
   "force_16_matrix",
   "force_cross",
   "force_single",
   "force_1x2",
   "force_2x1",
   "force_2x2",
   "force_1x3",
   "force_3x1",
   "force_3x3",
   "force_4x4",
   "force_singlefile",
   "force_half",
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

/* The first 10 of these (the constant to use is SELECTOR_PREDS) take a predicate.
   Any call that uses one of these predicates in its definition will cause a
   popup to appear asking "who?". */

char *predtab[] = {
   "select",
   "unselect",
   "select_near_select",
   "select_near_unselect",
   "unselect_near_select",
   "unselect_near_unselect",
   "once_rem_from_select",
   "conc_from_select",
   "select_once_rem_from_unselect",
   "unselect_once_rem_from_select",
   "always",
   "x22_miniwave",
   "x22_couple",
   "x22_facing_someone",
   "x22_tandem_with_someone",
   "x14_once_rem_miniwave",
   "x14_once_rem_couple",
   "lines_miniwave",
   "lines_couple",
   "cast_normal",
   "cast_pushy",
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
   "magic_outroller_is_cw",
   "inroller_is_cw_2x3",
   "magic_inroller_is_cw_2x3",
   "outroller_is_cw_2x3",
   "magic_outroller_is_cw_2x3",
   "inroller_is_cw_2x6",
   "outroller_is_cw_2x6",
   "inroller_is_cw_2x8",
   "outroller_is_cw_2x8",
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
   "leftp",
   "rightp",
   "inp",
   "outp",
   "zigzagp",
   "zagzigp",
   "zigzigp",
   "zagzagp",
   "no_dir_p",
   "dmd_ctrs_rh",
   "trngl_pt_rh",
   "q_tag_front",
   "q_tag_back",
   "q_line_front",
   "q_line_back",
   ""};

/* The "tag table" is the table that we use to bind together things like

            "seq flipdiamond []"

   in a definition, and

            call "flip the diamond" plus tag flipdiamond

   in another definition.  We keep a list of the strings, and turn
   each tag into a number that is its index in the list.  The binary
   database deals with these numbers.  The main sd program will make
   its own copy of the table, containing pointers to the actual call
   descriptors. */

int tagtabsize = 3;      /* Number of items we currently have in tagtab -- we initially have three; see below. */
int tagtabmax = 100;     /* Amount of space allocated for tagtab; must be >= tagtabsize at all times, obviously. */

tagtabitem *tagtab;      /* The dynamically allocated tag list. */

tagtabitem tagtabinit[] = {
      {1, "+++"},         /* Must be unused -- call #0 signals end of list in sequential encoding. */
      {0, "nullcall"},    /* Must be #1 -- the database initializer uses call #1 for any mandatory
                                    modifier, e.g. "clover and [anything]" is executed as
                                    "clover and [call #1]". */
      {0, "armturn_34"}};   /* Must be #2 -- this is used for "yo-yo". */

int eof;
int chars_left;
char *lineptr;
int linelen;
int lineno;
int error_is_fatal;
int tok_value;
int letcount;
toktype tok_kind;
char tok_str[80];
int char_ct;

char line[200];
char ch;
char *return_ptr;
int callcount;
int filecount;
int dumbflag;
unsigned int call_flags;
unsigned int call_flags1;
unsigned int call_tag;
char call_name[80];
int call_namelen;
int call_level;
int call_startsetup;
int call_qualifier;
int call_qual_num;
int call_endsetup;
int call_endsetup_in;
int call_endsetup_out;
int bmatrix, gmatrix;
int restrstate;





static void errexit(char s[])
{
   char my_line[1000];
   int i;

   if (error_is_fatal)
      do_printf("Error");
   else
      do_printf("Warning");

   if (lineno == 0) {
      do_printf(":\n%s.\n", s);
   }
   else {
      do_printf(" at line %d:\n%s.", lineno, s);

      if (!call_namelen)
         do_printf("\n");
      else
         do_printf("  Last call was: %s\n", call_name);

      if (!eof) {
         strncpy(my_line, lineptr, linelen-1);
         my_line[linelen-1] = '\0';
         do_printf("%s\n", my_line);
         for (i = 1; i <= linelen-chars_left-1; i++) do_printf(" ");
         do_printf("|\n");
      }
   }

   if (error_is_fatal) {
      free(tagtab);
      do_exit();
   }
}


static int get_char(void)
{
   if (!chars_left) {
      lineno++;
      return_ptr = db_gets(line, 199);
      lineptr = return_ptr;
      linelen = strlen(line);
      if (!return_ptr) {
         eof = 1;
         return 1;
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
         for (;;) {
            if (get_char()) errexit("End of file inside comment");
            if (ch == '*') {
               for (;;) {
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
         for (;;) {
            if (get_char())
               errexit("End of file inside symbol\n");

            if (ch == '"') break;
            else if (ch == '\\') {
               if (get_char())
                  errexit("End of file inside symbol\n");
            }
            if (char_ct > 80)
               errexit("String too long\n");

            tok_str[char_ct++] = ch;
         }

         ch = ' ';
         tok_kind = tok_string;

         /* Pack a null. */

         if (char_ct > 80)
            errexit("String too long\n");

         tok_str[char_ct] = '\0';
         break;
      default:
         for (;;) {
            if (char_ct > 80)
               errexit("Symbol too long\n");

            tok_str[char_ct++] = ch;

            digit = ch - '0';
            if (digit < 0 || digit > 9) letcount++;
            else tok_value = tok_value*10 + digit;

            if (get_char())
               errexit("End of file inside symbol\n");

            if (!symchar()) break;
         }

         /* Pack a null. */

         if (char_ct > 80)
            errexit("Symbol too long\n");

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


/* The returned value fits into 13 bits. */
static unsigned int tagsearch(int def)
{
   int i;

   for (i = 0; i < tagtabsize; i++) {
      if (!strcmp(tok_str, tagtab[i].s)) goto done;
   }

   i = tagtabsize++;
   /* Independently of the way we reallocate memory, the tag field must be able
      to fit into 13 bits in order to be packed into the binary database file. */
   if (i >= 8192) errexit("Sorry, too many tagged calls");

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



static void write_halfword(unsigned int n)
{
   db_putc((n>>8) & 0xFF);
   db_putc((n) & 0xFF);
   filecount += 2;
}



static void write_fullword(unsigned int n)
{
   db_putc((n>>24) & 0xFF);
   db_putc((n>>16) & 0xFF);
   db_putc((n>>8) & 0xFF);
   db_putc((n) & 0xFF);
   filecount += 4;
}


static void write_defmod_flags(void)
{
   int i;
   uint32 rr1 = 0;
   uint32 rrh = 0;

   get_tok();
   if (tok_kind != tok_lbkt)
      errexit("Missing left bracket in defmod list");

   get_tok();
   if (tok_kind != tok_rbkt) {
      for (;;) {
         if (tok_kind != tok_symbol)
            errexit("Improper defmod key");

         if ((i = search(defmodtab1)) >= 0)
            rr1 |= (1 << i);
         else if (strcmp(tok_str, "allow_plain_mod") == 0)
            rr1 |= (3*DFM1_CALL_MOD_BIT);
         else if (strcmp(tok_str, "or_secondary_call") == 0)
            rr1 |= (5*DFM1_CALL_MOD_BIT);
         else if (strcmp(tok_str, "mandatory_secondary_call") == 0)
            rr1 |= (6*DFM1_CALL_MOD_BIT);
         else if (strcmp(tok_str, "shift_three_numbers") == 0)
            rr1 |= (3*DFM1_NUM_SHIFT_BIT);
         else if ((i = search(defmodtabh)) >= 0) {
            uint32 bit = 1 << i;

            /* Don't check that left/reverse flags -- they are complicated, so there is no "force" word for them. */
            if (bit & ~(call_flags | INHERITFLAG_REVERSE | INHERITFLAG_LEFT))
               errexit("Can't use an \"inherit\" flag unless corresponding top level flag is on");

            rrh |= bit;
         }
         else if ((i = search(forcetabh)) >= 0) {
            uint32 bit = 1 << i;

            /* Don't check that left/reverse flags -- they are complicated, so there is no "force" word for them. */
            if (bit & call_flags & ~(INHERITFLAG_REVERSE | INHERITFLAG_LEFT))
               errexit("Can't use a \"force\" flag unless corresponding top level flag is off");

            rrh |= bit;
         }
         else
            errexit("Unknown defmod key");

         get_tok();
         if (tok_kind == tok_rbkt) break;
      }
   }

   write_fullword(rr1);
   write_fullword(rrh);
}



static void write_callarray(int num, int doing_matrix)
{
   int count;

   if (tok_kind != tok_lbkt)
      errexit("Missing left bracket in callarray list");

   for (count=0; ; count++) {
      unsigned int dat = 0;
      int p = 0;
      stability stab = stb_none;

      get_tok();
      if (tok_kind == tok_rbkt) break;
      else if (tok_kind == tok_number && tok_value == 0)
         write_halfword(0);
      else if (tok_kind == tok_symbol) {
         int repetition = 0;

         for (; letcount-p >= 2; p++) {
            switch (tok_str[p]) {
               case 'Z': case 'z':
                  if (stab == stb_none) stab = stb_z;
                  else errexit("Improper callarray specifier");
                  break;
               case 'A': case 'a':
                  switch (stab) {
                     case stb_none: stab = stb_a; break;
                     case stb_c: stab = stb_ca; break;
                     case stb_a: stab = stb_aa; break;
                     case stb_aa: repetition++; break;
                     case stb_cc:
                        if (repetition == 0)
                           stab = stb_cca;
                        else if (repetition == 1)
                           { stab = stb_ccca; repetition = 0; }
                        else if (repetition == 2)
                           { stab = stb_cccca; repetition = 0; }
                        break;
                     default: errexit("Improper callarray specifier");
                  }
                  break;
               case 'C': case 'c':
                  switch (stab) {
                     case stb_none: stab = stb_c; break;
                     case stb_a: stab = stb_ac; break;
                     case stb_c: stab = stb_cc; break;
                     case stb_cc: repetition++; break;
                     case stb_aa:
                        if (repetition == 0)
                           stab = stb_aac;
                        else if (repetition == 1)
                           { stab = stb_aaac; repetition = 0; }
                        else if (repetition == 2)
                           { stab = stb_aaaac; repetition = 0; }
                        break;
                     default: errexit("Improper callarray specifier");
                  }
                  break;
               default:
                  goto stability_done;
            }
         }

         stability_done:

         if (repetition != 0) errexit("Improper callarray specifier");

         if (letcount-p == 2) {
            switch (tok_str[p]) {
               case 'L': case 'l': dat = 4; break;
               case 'M': case 'm': dat = 2; break;
               case 'R': case 'r': dat = 1; break;
               default:
                  errexit("Improper callarray specifier");
            }
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
   int j;

   write_halfword(0x2000 | call_tag);
   write_halfword(call_level);
   write_fullword(call_flags1);
   write_fullword(call_flags);
   write_halfword((call_namelen << 8) | (unsigned int) schema);

   for (j=0; j<call_namelen; j++)
      db_putc(((unsigned int) call_name[j]) & 0xFF);

   filecount += call_namelen;
   callcount++;
}


static void write_conc_stuff(calldef_schema schema)
{
   write_call_header(schema);

   /* Write two level 2 concdefine records. */

   get_tok();
   if (tok_kind != tok_symbol) errexit("Improper conc symbol");

   write_halfword(0x4000 | tagsearch(0));
   write_defmod_flags();

   get_tok();
   if (tok_kind != tok_symbol) errexit("Improper conc symbol");

   write_halfword(0x4000 | tagsearch(0));
   write_defmod_flags();
}


static void write_seq_stuff(void)
{
   get_tok();
   if (tok_kind != tok_symbol) errexit("Improper seq symbol");
   write_halfword(0x4000 | tagsearch(0));
   write_defmod_flags();
}


static void write_level_3_group(unsigned int arrayflags)
{
   write_halfword(0x6000 | arrayflags);
   write_halfword(call_startsetup | (call_qualifier << 8));
   write_halfword(call_qual_num);
   if (arrayflags & CAF__CONCEND) {
      write_halfword(call_endsetup_in | (restrstate << 8));
      write_halfword(call_endsetup_out);
   }
   else {
      write_halfword(call_endsetup | (restrstate << 8));
   }
}


static void write_array_def(unsigned int funnyflag)
{
   int i, iii, jjj;
   unsigned int callarray_flags1, callarray_flags2;

   write_call_header(schema_by_array);

   /* Write a level 2 array define group. */
   write_halfword(0x4000);
   callarray_flags1 = funnyflag;

def2:
   restrstate = 0;
   callarray_flags2 = 0;
   call_qualifier = 0;
   call_qual_num = 0;

   get_tok();
   if (tok_kind != tok_symbol) errexit("Improper starting setup");
   if ((call_startsetup = search(sstab)) < 0) errexit("Unknown start setup");

   get_tok();
   if (tok_kind != tok_symbol) errexit("Improper ending setup");
   if ((call_endsetup = search(estab)) < 0) errexit("Unknown ending setup");

   /* Should actually look for anomalous concentric and do it specially */

   for (;;) {
      get_tok();
      if (tok_kind != tok_symbol) errexit("Missing indicator");

      if (!strcmp(tok_str, "array")) {
         write_level_3_group(callarray_flags1 | callarray_flags2);             /* Pred flag off. */
         get_tok();
         write_callarray(begin_sizes[call_startsetup], 0);
         get_tok_or_eof();
         break;
      }
      else if (!strcmp(tok_str, "preds")) {
         write_level_3_group(CAF__PREDS | callarray_flags1 | callarray_flags2);        /* Pred flag on. */
         get_tok();
         if (tok_kind != tok_string) errexit("Missing string");
         iii = char_ct;
         write_halfword(iii);
         for (jjj = 1; jjj <= (iii >> 1); jjj++)
            write_halfword((((int)(tok_str[2*jjj-2])) << 8) | ((int)(tok_str[2*jjj-1])));
         if (iii&1)
            write_halfword(((int)(tok_str[iii-1])) << 8);

         for (;;) {
            get_tok_or_eof();
            if (eof) break;

            if (tok_kind != tok_symbol) break;    /* Will give an error. */

            if (!strcmp(tok_str, "if")) {
               get_tok();
               if (tok_kind != tok_symbol) errexit("Improper predicate");
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
         call_qual_num = 0;
         get_tok();
         if (tok_kind != tok_symbol) errexit("Improper qualifier");
         if ((call_qualifier = search(qualtab)) < 0) errexit("Unknown qualifier");
      }
      else if (!strcmp(tok_str, "nqualifier")) {
         call_qual_num = get_num("Need a qualifier number here")+1;
         get_tok();
         if (tok_kind != tok_symbol) errexit("Improper qualifier");
         if ((call_qualifier = search(qualtab)) < 0) errexit("Unknown qualifier");
      }
      else if (!strcmp(tok_str, "restriction")) {
         get_tok();
         if (tok_kind != tok_symbol) errexit("Improper restriction specifier");

         if (!strcmp(tok_str, "unusual")) {
            callarray_flags2 |= CAF__RESTR_UNUSUAL;
            get_tok();
            if (tok_kind != tok_symbol) errexit("Improper restriction specifier");
         }
         else if (!strcmp(tok_str, "forbidden")) {
            callarray_flags2 |= CAF__RESTR_FORBID;
            get_tok();
            if (tok_kind != tok_symbol) errexit("Improper restriction specifier");
         }
         else if (!strcmp(tok_str, "resolve_ok")) {
            callarray_flags2 |= CAF__RESTR_RESOLVE_OK;
            get_tok();
            if (tok_kind != tok_symbol) errexit("Improper restriction specifier");
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
         if (tok_kind != tok_symbol) errexit("Improper setup specifier");
         if ((call_endsetup_in = search(estab)) < 0) errexit("Unknown setup specifier");

         get_tok();
         if (tok_kind != tok_symbol) errexit("Improper setup specifier");
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
   if (tok_kind != tok_symbol) errexit("Missing indicator");

   /* If see something other than "setup", it's either an alternate definition
      to start another group of arrays, or it's the end of the whole call. */

   callarray_flags1 = 0;

   if (strcmp(tok_str, "setup") != 0) {
      int alt_level;
      unsigned int rrr = 0;

      if (strcmp(tok_str, "alternate_definition") != 0) {
         return;       /* Must have seen next 'call' indicator. */
      }

      get_tok();
      if (tok_kind != tok_lbkt)
         errexit("Missing left bracket in alternate_definition list");

      get_tok();
      if (tok_kind != tok_rbkt) {
         for (;;) {
            if (tok_kind != tok_symbol)
               errexit("Improper alternate_definition key");

            if ((i = search(altdeftabh)) < 0) errexit("Unknown alternate_definition key");
            rrr |= (1 << i);

            get_tok();
            if (tok_kind == tok_rbkt) break;
         }
      }

      get_tok();
      if (tok_kind != tok_symbol) errexit("Improper alternate_definition level");
      if ((alt_level = search(leveltab)) < 0) errexit("Unknown alternate_definition level");

      write_halfword(0x4000 | alt_level);
      write_fullword(rrr);
   
      /* Now do another group of arrays. */
   
      get_tok();
      if (tok_kind != tok_symbol) errexit("Missing indicator");
   
      if (!strcmp(tok_str, "simple_funny")) {
         callarray_flags1 = CAF__FACING_FUNNY;
         get_tok();
         if (tok_kind != tok_symbol) errexit("Missing indicator");
      }

      if (strcmp(tok_str, "setup") != 0)
         errexit("Need \"setup\" indicator");
   }

   /* Must have seen "setup" -- do another basic array. */

   goto def2;
}



extern void dbcompile(void)
{
   int i, iii;
   unsigned int funnyflag;

   tagtabmax = 100; /* try to make it reentrant */
   tagtabsize = 3;
   eof = 0;
   lineno = 0;
   chars_left = 0;
   error_is_fatal = 1;
   ch = ' ';
   call_namelen = 0;   /* So won't print error first time around. */
   tagtab = (tagtabitem *) malloc(tagtabmax * sizeof(tagtabitem));
   if (!tagtab)
      errexit("Out of memory!!!!!!");

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
   if (tok_kind != tok_symbol) errexit("Improper \"version\" indicator");
   if (strcmp(tok_str, "version")) errexit("Missing version specification");
   get_tok();
   if (tok_kind != tok_string) errexit("Improper version string -- must be in quotes");
   write_halfword(char_ct);
   for (i=0; i<char_ct; i++)
      db_putc(((unsigned int) tok_str[i]) & 0xFF);
   filecount += char_ct;

   callcount = 0;
   for (;;) {

      get_tok_or_eof();
   startagain:
      if (eof) break;

      if (tok_kind != tok_symbol) errexit("Missing indicator");

      if (!strcmp(tok_str, "call")) {
         unsigned int matrixflags;
         int bit;
         calldef_schema ccc;

         get_tok();
         if (tok_kind != tok_string) errexit("Improper call name");

         strcpy(call_name, tok_str);
         call_namelen = char_ct;
         call_flags = 0;
         call_flags1 = 0;
         call_tag = 0;

         get_tok();
         if (tok_kind != tok_symbol) errexit("Improper level");
         if ((call_level = search(leveltab)) < 0) errexit("Unknown level");

         /* Get toplevel options. */

         for (;;) {
            get_tok();
            if (tok_kind != tok_symbol) errexit("Missing indicator");
            if (!strcmp(tok_str, "tag")) {
               get_tok();
               if (tok_kind != tok_symbol) errexit("Improper tag");
               call_tag = tagsearch(1);
            }
            else {
               if ((iii = search(flagtab1)) >= 0)
                  call_flags1 |= (1 << iii);
               else if (strcmp(tok_str, "need_three_numbers") == 0)
                  call_flags1 |= (3*CFLAG1_NUMBER_BIT);
               else if ((iii = search(flagtabh)) >= 0)
                  call_flags |= (1 << iii);
               else
                  break;
            }
         }

         /* Process the actual definition.  First, check for the "simple_funny" indicator. */

         funnyflag = 0;

         if (!strcmp(tok_str, "simple_funny")) {
            funnyflag = CAF__FACING_FUNNY;
            get_tok();
            if (tok_kind != tok_symbol) errexit("Missing indicator");
         }

         /* Find out what kind of call it is. */
         iii = search(schematab);

         if (iii < 0) errexit("Can't determine call definition type");

         ccc = (calldef_schema) iii;

         if (funnyflag != 0 && ccc != schema_by_array)
            errexit("Simple_funny out of place");

         switch(ccc) {
            case schema_by_array:
               write_array_def(funnyflag);
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

               for (;;) {     /* Get partner matrix call options. */
                  get_tok();
                  if (tok_kind != tok_symbol) break;
                  if ((bit = search(matrixcallflagtab)) < 0) errexit("Unknown matrix call flag");
                  matrixflags |= (1 << bit);
               }
   
               if (matrixflags & MTX_USE_SELECTOR) call_flags1 |= CFLAG1_REQUIRES_SELECTOR;
               write_call_header(ccc);
               write_halfword(matrixflags >> 8);
               write_halfword(matrixflags);
               write_callarray(8, 1);
               break;
            case schema_roll:
               write_call_header(ccc);
               break;
            case schema_sequential:
            case schema_split_sequential:
               write_call_header(ccc);
               write_seq_stuff();

               for (;;) {               /* Write a level 2 seqdefine group. */
                  get_tok_or_eof();
                  if (eof) break;
                  if ((tok_kind == tok_symbol) && (!strcmp(tok_str, "seq"))) {
                     /* Write a level 2 seqdefine group. */
                     write_seq_stuff();
                  }
                  else
                     break;       /* Must have seen next 'call' indicator. */
               }
               goto startagain;
            default:
               write_conc_stuff(ccc);
               break;
         }
      }
      else
         errexit("Item in illegal context");
   }

   write_halfword(0);         /* final end mark */

   dumbflag = 0;

   for (i = 0; i < tagtabsize; i++) {
      if (!tagtab[i].def) {
         if (!dumbflag++) do_printf("Tags not defined:\n");
         do_printf("   %s\n", tagtab[i].s);
      }
   }

   db_close_input();

   dbcompile_signoff(filecount, callcount);

   db_rewind_output(4);

   write_halfword(callcount);
   write_halfword(tagtabsize);

   db_close_output();

   free(tagtab);
}
