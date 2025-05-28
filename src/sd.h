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

    This is for version 29. */

#define TRUE 1
#define FALSE 0
#define NULLCONCEPTPTR (concept_descriptor *) 0
#define NULLCALLSPEC (callspec_block *) 0

typedef int long_boolean;

/* We would like this to be a signed char, but not all compilers are fully ANSI compliant. */
/* The IBM AIX compiler, for example, considers char to be unsigned. */
typedef short veryshort;

#define Private static

#include <setjmp.h>
#include "database.h"

#define MAX_ERR_LENGTH 200
#define MAX_FILENAME_LENGTH 260
#define MAX_PEOPLE 24
/* Actually, we don't make a resolve bigger than 3.  This is how much space
   we allocate for things.  Just being careful. */
#define MAX_RESOLVE_SIZE 5

/* Probability (out of 8) that a concept will be placed on a randomly generated call. */
#define CONCEPT_PROBABILITY 2

/* Absolute maximum length we can handle in text operations, including
   writing to file.  If a call gets more complicated than this, stuff
   will simply not be written to the file.  Too bad. */
#define MAX_TEXT_LINE_LENGTH 200

/* This defines a person in a setup.  Unfortunately, there is too much data to fit into 32 bits. */
typedef struct {
   unsigned int id1;       /* Frequently used bits go here. */
   unsigned int id2;       /* Bits used for evaluating predicates. */
} personrec;

/* Person bits for "id1" field are:
 20 000 000 000 -
 10 000 000 000 - 
  4 000 000 000 - 
  2 000 000 000 -
  1 000 000 000 -
    400 000 000 - roll direction is CCW
    200 000 000 - roll direction is neutral
    100 000 000 - roll direction is CW
     40 000 000 - fractional stability enabled
     20 000 000 - stability "v" field -- 2 bits
     10 000 000 -     "
      4 000 000 - stability "r" field -- 3 bits
      2 000 000 -     "
      1 000 000 -     "
        400 000 -
        200 000 -
        100 000 -
         40 000 -
         20 000 -
         10 000 -
          4 000 - 
          2 000 - virtual person (created by tandem/couples concept, can't print out)
          1 000 - live person (just so at least one bit is always set)
            700 - these 3 bits identify actual person
             60 - these 2 bits must be clear for rotation
             10 - part of rotation (facing north/south)
              4 - bit must be clear for rotation
              2 - part of rotation
              1 - part of rotation (facing east/west)
*/

/* These are a 3 bit field -- ROLL_BIT tells where its low bit lies. */
#define ROLL_MASK   0700000000
#define ROLL_BIT    0100000000
#define ROLLBITL    0400000000
#define ROLLBITM    0200000000
#define ROLLBITR    0100000000
#define STABLE_MASK  077000000
#define STABLE_ENAB  040000000
#define STABLE_VBIT  010000000
#define STABLE_RBIT  001000000
#define BIT_VIRTUAL 02000
#define BIT_PERSON  01000
#define d_mask  01013
#define d_north 01010
#define d_south 01012
#define d_east  01001
#define d_west  01003

/* Person bits for "id2" field are:
 20 000 000 000 -
 10 000 000 000 - 
  4 000 000 000 - center 2
  2 000 000 000 - belle
  1 000 000 000 - beau
    400 000 000 - center 6
    200 000 000 - outer 2
    100 000 000 - outer 6
     40 000 000 - trailer
     20 000 000 - leader
     10 000 000 - not side girl
      4 000 000 - not side boy
      2 000 000 - not head girl
      1 000 000 - not head boy
        400 000 - head corner
        200 000 - side corner
        100 000 - head
         40 000 - side
         20 000 - boy
         10 000 - girl
          4 000 - center
          2 000 - end
          1 000 - near column
            400 - near line
            200 - near box
            100 - far column
             40 - far line
             20 - far box
             10 -
              4 -
              2 -
              1 -
*/

#define ID2_CTR2    04000000000
#define ID2_BELLE   02000000000
#define ID2_BEAU    01000000000
#define ID2_CTR6     0400000000
#define ID2_OUTR2    0200000000
#define ID2_OUTR6    0100000000
#define ID2_TRAILER   040000000
#define ID2_LEAD      020000000
#define ID2_NSG       010000000
#define ID2_NSB        04000000
#define ID2_NHG        02000000
#define ID2_NHB        01000000
#define ID2_HCOR        0400000
#define ID2_SCOR        0200000
#define ID2_HEAD        0100000
#define ID2_SIDE         040000
#define ID2_BOY          020000
#define ID2_GIRL         010000
#define ID2_CENTER        04000
#define ID2_END           02000
#define ID2_NEARCOL       01000
#define ID2_NEARLINE       0400
#define ID2_NEARBOX        0200
#define ID2_FARCOL         0100
#define ID2_FARLINE         040
#define ID2_FARBOX          020


typedef struct {
   setup_kind skind;
   int srotation;
} small_setup;

/* Flags that reside in the "setupflags" word of a setup BEFORE a call is executed.

   BEWARE!! These flags co-exist in the setupflags word with copies of some
   of the call invocation flags.  The mask for those flags is
   DFM_CONCENTRICITY_FLAG_MASK.  Those flags, and that mask, are defined
   in database.h .  We define these flags at the extreme left end of the
   word in order to keep them away from the concentricity flags.

   SETUPFLAG__DISTORTED means that we are at a level of recursion in which some
   distorted-setup concept has been used.  When this is set, "matrix" (a.k.a.
   "space invader") calls, such as press and truck, are not permitted.  Sorry, Clark.

   SETUPFLAG__OFFSET_Z means that we are doing a 2x3 call (the only known case
   of this is "Z axle") that came from Z's picked out of a 4x4 setup.  The map
   that we are using has a "50% offset" map schema, so that, if the call goes
   into 1x4's (which Z axle does) it will put the results into a parallelogram,
   in accordance with customary usage.  The problem is that, if the call goes
   into a setup other than a 1x4 oriented in the appropriate way, using the
   50% offset schema will lead to the wrong result.  For example, if a call
   were defined that went from Z's into 2x2's, the program would attempt to
   bizarrely offset boxes.  (Yes, I know that Z axle is the only call in the
   database that can cause Z's to be picked out of a 4x4.  The program is just
   trying to be very careful.)  So, when this flag is on and a shape-changer
   occurs, the program insists on a parallelogram result.

   SETUPFLAG__SAID_SPLIT means that we are at a level of recursion in which
   the "split" concept has been used.  When this is on, it is not permissible
   to do a 4-person call that has not 8-person definition, and hence would have
   been done the same way without the "split" concept.  This prevents superfluous
   things like "split pass thru".

   SETUPFLAG__SAID_TRIANGLE means that we are at a level of recursion in which
   the word "triangle" has been uttered, so it is OK to do things like "triangle
   peel and trail" without giving the explicit "triangle" concept again.  This
   makes it possible to say things like "tandem-based triangles peel and trail".

   SETUPFLAG__FRACTIONALIZE_MASK is a 9 bit field that is nonzero when some form
   of fractionalization control is in use.  These bits are set up by concepts like
   "random" and "fractional", and are used in sdmoves.c to control sequentially
   defined calls.  See the comments there for details.

   SETUPFLAG__ELONGATE_MASK is a 2 bit field that tells, for a 2x2 setup prior to
   having a call executed, how that 2x2 is elongated (due to these people
   being the outsides) in the east-west or north-south direction.  Since 2x2's are
   always totally canonical, the interpretation of the elongation direction is
   always absolute.  A 1 in this field means the elongation is east-west.  A 2
   means the elongation is north-south.  A zero means the elongation is unknown.

   SETUPFLAG__NO_CHK_ELONG means that the elongation of the incoming setup is for
   informational purposes only (to tell where people should finish) and should not
   be used for raising error messages.  It suppresses the error that would be
   raised if we have people in a facing 2x2 star thru when they are far from the
   ones they are facing.

   SETUPFLAG__PHANTOMS means that we are at a level of recursion in which some phantom concept
   has been used.  When this is set, the "tandem" or "as couples" concepts will
   forgive phantoms wherever they might appear, so that "split phantom lines tandem
   <call>" is legal.  Otherwise, we would have to say "split phantom lines phantom
   tandem <call>.

   SETUPFLAG__NO_STEP_TO_WAVE means that we are at a level of recursion that no longer permits us to do the
   implicit step to a wave or rear back from one that some calls permit at the top level.

   SETUPFLAG__EXPLICIT_MATRIX means that the caller said "4x4 matrix" or "2x6 matrix" or whatever,
   so we got to this matrix explicitly.  This enables natural splitting of the setup, e.g. form
   a parallelogram, "2x6 matrix 1x2 checkmate" is legal -- the 2x6 gets divided naturally
   into 2x3's.

   SETUPFLAG__NO_EXPAND_MATRIX means that we are at a level of recursion that no longer permits us to do the
   implicit expansion of the matrix (e.g. add outboard phantoms to turn a 2x4 into a 2x6
   if the concept "triple box" is used) that some concepts perform at the top level.

   SETUPFLAG__DOING_ENDS means that this call is directed only to the ends (and the setup is the ends
   of the original setup.  If the call turns out to be an 8-person call with distinct
   centers and ends parts, we may want to just apply the ends part.  This is what
   makes "ends detour" work.
*/

#define SETUPFLAG__EXPLICIT_MATRIX    0x00000800
#define SETUPFLAG__NO_EXPAND_MATRIX   0x00001000
#define SETUPFLAG__DISTORTED          0x00002000
#define SETUPFLAG__OFFSET_Z           0x00004000
#define SETUPFLAG__SAID_SPLIT         0x00008000
#define SETUPFLAG__SAID_TRIANGLE      0x00010000
/* This one is a 9 bit field -- FRACTIONALIZE_BIT tells where its low bit lies. */
#define SETUPFLAG__FRACTIONALIZE_MASK 0x03FE0000
#define SETUPFLAG__FRACTIONALIZE_BIT  0x00020000
/* This one is a 2 bit field -- ELONGATE_BIT tells where its low bit lies. */
#define SETUPFLAG__ELONGATE_MASK      0x0C000000
#define SETUPFLAG__ELONGATE_BIT       0x04000000
#define SETUPFLAG__NO_CHK_ELONG       0x10000000
#define SETUPFLAG__PHANTOMS           0x20000000
#define SETUPFLAG__NO_STEP_TO_WAVE    0x40000000
#define SETUPFLAG__DOING_ENDS         0x80000000

/* Flags that reside in the "setupflags" word of a setup AFTER a call is executed.
   There shouldn't be a problem of conflict with the "before" flags, but we take
   no chances.

   RESULTFLAG__EXPAND_TO_2X3 means that a call was executed that takes a four person
   starting setup (typically a line) and yields a 2x3 result setup.  Two of those
   result spots will be phantoms, of course.  When recombining a divided setup in
   which such a call was executed, if the recombination wuold yield a 2x6, and if
   the center 4 spots would be empty, this flag directs divided_setup_move to
   collapse the setup to a 2x4.  This is what makes calls like "pair the line"
   and "step and slide" behave correctly from parallel lines and from a grand line.

   RESULTFLAG__DID_LAST_PART means that, when a sequentially defined call was executed
   with the SETUPFLAG__FRACTIONALIZE_MASK nonzero, so that just one part was done,
   that part was the last part.  Hence, if we are doing a call with some "piecewise"
   or "random" concept, we do that parts of the call one at a time, with appropriate
   concepts on each part, until it comes back with this flag set.

   RESULTFLAG__ELONGATE_MASK is a 2 bit field that tells, for a 2x2 setup after
   having a call executed, how that 2x2 is elongated in the east-west or
   north-south direction.  Since 2x2's are always totally canonical, the
   interpretation of the elongation direction is always absolute.  A 1 in this
   field means the elongation is east-west.  A 2 means the elongation is
   north-south.  A zero means the elongation is unknown.  These bits will be set
   whenever we know which elongation is preferred.  This will be the case if the
   setup was an elongated 2x2 prior to the call (because they are the ends), or
   the setup was a 1x4 prior to the call.  So, for example, after any recycle
   from a wave, these bits will be set to indicate that the dancers want to
   elongate the 2x2 perpendicular to their original wave.  These bits will be
   looked at only if the call was directed to the ends (starting from a grand
   wave, for example.)  Furthermore, the bits will be overridden if the
   "concentric" or "checkpoint" concepts were used, or if the ends did this
   as part of a concentrically defined call for which some forcing modifier
   such as "force_columns" was used.

   RESULTFLAG__NEED_DIAMOND means that a call has been executed with the "magic" or
   "interlocked" modifier, and it really should be changed to the "magic diamond"
   concept.  Otherwise, we might end up saying "magic diamond single wheel" when
   we should have said "magic diamond, diamond single wheel".
*/

#define RESULTFLAG__EXPAND_TO_2X3   0x00000002
#define RESULTFLAG__DID_LAST_PART   0x00000004
/* This one is a 2 bit field -- ELONGATE_BIT tells where its low bit lies. */
#define RESULTFLAG__ELONGATE_MASK   0x00000018
#define RESULTFLAG__ELONGATE_BIT    0x00000008
#define RESULTFLAG__NEED_DIAMOND    0x00000020

/* It should be noted that the SETUPFLAG__XXX and RESULTFLAG__XXX bits have
   nothing to do with each other.  They just happen to occupy the same word
   before and after the call is executed, respectively.  It is not intended that
   the flags resulting from one call be passed to the next.  In fact, that
   would be incorrect.  The SETUPFLAG__XXX bits should start at zero at the
   beginning of each call, and accumulate stuff as the call goes deeper into
   recursion.  The RESULTFLAG__XXX bits should, in general, be the OR of the
   bits of the components of a compound call, though this is not so for
   RESULTFLAG__PAR_CONC_END. */



typedef struct {
   setup_kind kind;
   int setupflags;
   personrec people[MAX_PEOPLE];
   int rotation;
   small_setup inner;
   small_setup outer;
   int outer_elongation;
} setup;

/* BEWARE!!  If change these next definitions, be sure to update the definition of
   "warning_strings" in sdutil.c . */
#define warn__do_your_part        0
#define warn__tbonephantom        1
#define warn__ends_work_to_spots  2
#define warn__awkward_centers     3
#define warn__bad_concept_level   4
#define warn__not_funny           5
#define warn__hard_funny          6
#define warn__unusual             7
#define warn__rear_back           8
#define warn__awful_rear_back     9
#define warn__excess_split        10
#define warn__lineconc_perp       11
#define warn__dmdconc_perp        12
#define warn__lineconc_par        13
#define warn__dmdconc_par         14
#define warn__xclineconc_perp     15
#define warn__xcdmdconc_perp      16
#define warn__ctrstand_endscpls   17
#define warn__ctrscpls_endstand   18
#define warn__each2x2             19
#define warn__each1x4             20
#define warn__each1x2             21
#define warn__take_right_hands    22
#define warn__ctrs_are_dmd        23
#define warn__full_pgram          24
#define warn__offset_gone         25
#define warn__overlap_gone        26
#define warn__to_o_spots          27
#define warn__to_x_spots          28
#define warn__some_rear_back      29
#define warn__not_tbone_person    30
#define warn__check_c1_phan       31
#define warn__check_dmd_qtag      32
#define warn__check_2x4           33
#define warn__check_pgram         34
#define warn__dyp_resolve_ok      35
#define warn__ctrs_stay_in_ctr    36


/* BEWARE!!  The warning numbers in this set must all be <= 31.  This is a
   mask that is checked against the first word in the warning struct.
   It just isn't worth doing this the really right way. */
#define Warnings_That_Preclude_Searching (1<<warn__do_your_part | 1<<warn__ends_work_to_spots | 1<<warn__awkward_centers | \
                                          1<<warn__hard_funny   | 1<<warn__rear_back          | 1<<warn__awful_rear_back | \
                                          1<<warn__excess_split | \
                                          1<<warn__unusual      | 1<<warn__bad_concept_level  | 1<<warn__not_funny)

/* BEWARE!!  The warning numbers in this set must all be <= 31.  This is a
   mask that is cleared if a concentric call was done and the "suppress_elongation_warnings"
   flag was on.  It just isn't worth doing this the really right way. */
#define Warnings_About_Conc_elongation   (1<<warn__lineconc_perp | 1<<warn__dmdconc_perp    | 1<<warn__lineconc_par | \
                                          1<<warn__dmdconc_par   | 1<<warn__xclineconc_perp | 1<<warn__xcdmdconc_perp)


/* BEWARE!!  If change this next definition, be sure to update the definition of
   "resolve_names" in sdtables.c . */
typedef enum {
   resolve_none,
   resolve_rlg, resolve_la,
   resolve_ext_rlg, resolve_ext_la,
   resolve_slipclutch_rlg, resolve_slipclutch_la,
   resolve_circ_rlg, resolve_circ_la,
   resolve_pth_rlg, resolve_pth_la,
   resolve_tby_rlg, resolve_tby_la,
   resolve_xby_rlg, resolve_xby_la,
   resolve_dixie_grand,
   resolve_prom, resolve_at_home
} resolve_kind;

typedef struct {
   resolve_kind kind;
   int distance;
} resolve_indicator;

typedef enum {
   mode_none,     /* Not a real mode; used only for
                     fictional purposes in the user interface;
                     never appears in the rest of the program. */
   mode_normal,
   mode_startup,
   mode_resolve
} mode_kind;

typedef enum {
    modify_popup_any,
    modify_popup_only_tag,
    modify_popup_only_scoot
} modify_popup_kind;

/* These are the values returned by "uims_get_command". */

typedef enum {
   ui_command_select,   /* (normal/resolve) User chose one of the special buttons like "resolve" or "quit". */
   ui_resolve_select,   /* (resolve only) User chose one of the various actions peculiar to resolving. */
   ui_start_select,     /* (startup only) User chose something. This is the only outcome in startup mode. */
   ui_concept_select,   /* (normal only) User clicked in the miscellaneous concept menu. */
   ui_call_select,      /* (normal only) User clicked in the current call menu. */
   ui_special_concept   /* (normal only) User clicked in the menu of special concept popup names. */
} uims_reply;

/* In each case, an integer or enum is deposited into the global variable uims_menu_index.  Its interpretation
   depends on which of the replies above was given.  For some of the replies, it gives the index
   into a menu.  For "ui_start_select" it is a start_select_kind.
   For other replies, it is one of the following constants: */

/* BEWARE!!  This list may need to track some stuff in sdui.c and/or sd.dps . */
/* BEWARE!!  This list must track all the "startinfolist" definitions in sdtables.c . */
typedef enum {
   start_select_exit,        /* Don't start a sequence; exit from the program. */
   start_select_h1p2p,       /* Start with Heads 1P2P. */
   start_select_s1p2p,       /* Etc. */
   start_select_heads_start,
   start_select_sides_start,
   start_select_as_they_are
} start_select_kind;
#define NUM_START_SELECT_KINDS (((int) start_select_as_they_are)+1)

/* For ui_command_select: */
typedef enum {
   command_quit,
   command_undo,
   command_abort,
   command_allow_modification,
   command_create_comment,
   command_change_outfile,
   command_getout,
   command_resolve,
   command_reconcile,
   command_anything,
   command_nice_setup,
   command_neglect,
   command_save_pic
} command_kind;
#define NUM_COMMAND_KINDS (((int) command_save_pic)+1)

/* For ui_resolve_select: */
typedef enum {
   resolve_command_abort,
   resolve_command_find_another,
   resolve_command_goto_next,
   resolve_command_goto_previous,
   resolve_command_accept,
   resolve_command_raise_rec_point,
   resolve_command_lower_rec_point
} resolve_command_kind;
#define NUM_RESOLVE_COMMAND_KINDS (((int) resolve_command_lower_rec_point)+1)

/* BEWARE!!  There may be tables in the user interface file keyed to this enumeration. */
/* In particular, this list must track the array "menu_names" in sdtables.c . */
typedef enum {
   call_list_none, call_list_empty, /* Not real call list kinds; used only for
                                       fictional purposes in the user interface;
				       never appear in the rest of the program. */
   call_list_any,                   /* This is the "universal" call list; used
                                       whenever the setup isn't one of the known ones. */
   call_list_1x8, call_list_l1x8,
   call_list_dpt, call_list_cdpt,
   call_list_rcol, call_list_lcol,
   call_list_8ch, call_list_tby,
   call_list_lin, call_list_lout,
   call_list_rwv, call_list_lwv,
   call_list_r2fl, call_list_l2fl,
   call_list_gcol, call_list_qtag
} call_list_kind;
#define NUM_CALL_LIST_KINDS (((int) call_list_qtag)+1)


/* These flags go along for the ride, in some parts of the code, in the same word
   as the heritable flags, but are not part of the inheritance mechanism.  We use
   symbols that have been graciously provided for us from database.h to tell us
   what bits may be safely used next to the heritable flags. */

#define FINAL__SPLIT                      INHERITSPARE_1
#define FINAL__SPLIT_SQUARE_APPROVED      INHERITSPARE_2
#define FINAL__SPLIT_DIXIE_APPROVED       INHERITSPARE_3
#define FINAL__MUST_BE_TAG                INHERITSPARE_4
#define FINAL__MUST_BE_SCOOT              INHERITSPARE_5
#define FINAL__TRIANGLE                   INHERITSPARE_6
#define FINAL__SPLIT_SEQ_DONE             INHERITSPARE_7

typedef unsigned int final_set;

typedef long_boolean (*predicate_ptr)(
   setup *real_people,
   int real_index,
   int real_direction,
   int northified_index);

typedef struct glosk {
   predicate_ptr pred;
   struct glosk *next;
   /* Dynamically allocated to whatever size is required. */
   unsigned short arr[4];
} predptr_pair;

typedef struct glork {
   struct glork *next;
   unsigned int callarray_flags;
   call_restriction restriction;
   search_qualifier qualifier;
   begin_kind start_setup;
   setup_kind end_setup;
   setup_kind end_setup_in;             /* only if end_setup = concentric */
   setup_kind end_setup_out;            /* only if end_setup = concentric */
   union {
      /* Dynamically allocated to whatever size is required. */
      unsigned short def[4];     /* only if pred = false */
      struct {                   /* only if pred = true */
         predptr_pair *predlist;
         /* Dynamically allocated to whatever size is required. */
         char errmsg[4];
      } prd;
   } stuff;
} callarray;

typedef struct glonk {
   char txt[80];
   struct glonk *nxt;
} comment_block;

typedef struct flonk {
   char txt[80];
   struct flonk *nxt;
} outfile_block;

typedef int defmodset;

typedef struct {
   short call_id;
   unsigned int modifiers1;
   unsigned int modifiersh;
} by_def_item;

typedef struct glowk {
   unsigned int modifier_seth;
   callarray *callarray_list;
   struct glowk *next;
   dance_level modifier_level;
} calldef_block;

typedef struct {
   unsigned int callflags1;
   unsigned int callflagsh;
   int age;
   calldef_schema schema;
   union {
      struct {
         calldef_block *def_list;
      } arr;            /* if schema = schema_by_array */
      struct {
         int flags;
         short stuff[8];
      } matrix;         /* if schema = schema_matrix or schema_partner_matrix */
      struct {
         by_def_item *defarray;  /* Dynamically allocated, ends with a zero. */
      } def;            /* if schema = schema_by_def */
      struct {
         by_def_item innerdef;
         by_def_item outerdef;
      } conc;           /* if schema = schema_concentric, schema_concentric_6_2, schema_concentric_2_6, schema_cross_concentric, schema_concentric_diamond_line */
   } stuff;
   /* Dynamically allocated to whatever size is required, will have trailing null. */
   char name[4];
} callspec_block;

/* BEWARE!!  This list must track all the "map_hunk" definitions in sdtables.c . */
typedef enum {
   MPKIND__NONE,
   MPKIND__SPLIT,
   MPKIND__REMOVED,
   MPKIND__OVERLAP,
   MPKIND__INTLK,
   MPKIND__CONCPHAN,
   MPKIND__OFFS_L_HALF,
   MPKIND__OFFS_R_HALF,
   MPKIND__OFFS_L_FULL,
   MPKIND__OFFS_R_FULL,
   MPKIND__O_SPOTS,
   MPKIND__X_SPOTS,
   MPKIND__4_QUADRANTS,
   MPKIND__4_EDGES,
   MPKIND__DMD_STUFF
} mpkind;

typedef struct {
   veryshort map1[8];
   veryshort map2[8];
   veryshort map3[8];
   veryshort map4[8];
   mpkind map_kind;
   int arity;
   setup_kind outer_kind;
   setup_kind inner_kind;
   int rot;
   int vert;
} map_thing;

typedef map_thing *map_hunk[][2];

typedef struct {
   setup_kind result_kind;
   int xfactor;
   veryshort xca[24];
   veryshort yca[24];
   veryshort diagram[64];
} coordrec;


/* BEWARE!!  This list must track the array "concept_table" in sdconcpt.c . */
typedef enum {

/* These next few are not concepts.  Their appearance marks
   the final end of the concept list.  It has a selector and number
   specifier following, and then the call itself. */

   concept_another_call_next_mod,         /* calla modified by callb */
   concept_another_call_next_modreact,    /* calla, which is a "scoot reaction" type, modified by callb */
   concept_another_call_next_modtag,      /* calla, which is a "tag your neighbor" type, modified by callb */
   concept_another_call_next_force,       /* calla, old subcall, changed to callb */
   concept_mod_declined,                  /* user was queried about modification, and said no. */
   marker_end_of_list,                    /* normal case */

/* This is not a concept.  Its appearance indicates a comment is to be placed here. */

   concept_comment,

/* Everything after this is a real concept. */

   concept_concentric,
   concept_single_concentric,
   concept_tandem,
   concept_gruesome_tandem,
   concept_some_are_tandem,
   concept_frac_tandem,
   concept_gruesome_frac_tandem,
   concept_some_are_frac_tandem,
   concept_checkerboard,
   concept_reverse,
   concept_left,
   concept_grand,
   concept_magic,
   concept_cross,
   concept_single,
   concept_interlocked,
   concept_12_matrix,
   concept_16_matrix,
   concept_1x2,
   concept_2x1,
   concept_2x2,
   concept_1x3,
   concept_3x1,
   concept_3x3,
   concept_4x4,
   concept_2x6_matrix,
   concept_2x8_matrix,
   concept_4x4_matrix,
   concept_4dmd_matrix,
   concept_funny,
   concept_randomtrngl,
   concept_selbasedtrngl,
   concept_split,
   concept_each_1x4,
   concept_diamond,
   concept_triangle,
   concept_do_both_boxes,
   concept_once_removed,
   concept_do_phantom_2x2,
   concept_do_phantom_boxes,
   concept_do_phantom_diamonds,
   concept_do_phantom_1x6,
   concept_do_phantom_1x8,
   concept_do_phantom_2x4,
   concept_do_phantom_lines,
   concept_do_phantom_2x3,
   concept_divided_2x4,
   concept_divided_2x3,
   concept_distorted,
   concept_single_diagonal,
   concept_double_diagonal,
   concept_parallelogram,
   concept_triple_lines,
   concept_triple_lines_together,
   concept_quad_lines,
   concept_quad_lines_together,
   concept_quad_boxes,
   concept_quad_boxes_together,
   concept_triple_boxes,
   concept_triple_boxes_together,
   concept_triple_diamonds,
   concept_triple_diamonds_together,
   concept_quad_diamonds,
   concept_quad_diamonds_together,
   concept_triple_diag,
   concept_triple_diag_together,
   concept_triple_twin,
   concept_misc_distort,
   concept_old_stretch,
   concept_new_stretch,
   concept_c1_phantom,
   concept_grand_working,
   concept_centers_or_ends,
   concept_so_and_so_only,
   concept_some_vs_others,
   concept_stable,
   concept_so_and_so_stable,
   concept_frac_stable,
   concept_so_and_so_frac_stable,
   concept_standard,
   concept_double_offset,
   concept_checkpoint,
   concept_on_your_own,
   concept_trace,
   concept_ferris,
   concept_centers_and_ends,
   concept_twice,
   concept_sequential,
   concept_meta,
   concept_so_and_so_begin,
   concept_nth_part,
   concept_replace_nth_part,
   concept_interlace,
   concept_fractional,
   concept_rigger,
   concept_slider,
   concept_callrigger
} concept_kind;

/* These flags go into the "concept_prop" field of a "concept_table_item". */

/* This means that the concept takes a second call, so a sublist must
   be created, with a pointer to same just after the concept pointer. */
#define CONCPROP__SECOND_CALL     0x00000001
/* This means that the concept requires a selector, which must be
   inserted into the concept list just after the concept pointer. */
#define CONCPROP__USE_SELECTOR    0x00000002
/* These mean that the concept requires the indicated setup, and, at
   the top level, the existing setup should be expanded as needed. */
#define CONCPROP__NEED_4X4        0x00000004
#define CONCPROP__NEED_2X8        0x00000008
#define CONCPROP__NEED_2X6        0x00000010
#define CONCPROP__NEED_4DMD       0x00000020
#define CONCPROP__NEED_BLOB       0x00000040
#define CONCPROP__NEED_4X6        0x00000080
/* This means that phantoms are in use under this concept, so that,
   when looking for tandems or couples, we shouldn't be disturbed if we
   pair someone with a phantom.  It is what makes "split phantom lines tandem"
   work, so that "split phantom lines phantom tandem" is unnecessary. */
#define CONCPROP__SET_PHANTOMS    0x00000100
/* This means that stepping to a wave or rearing back from one is not
   allowed under this concept. */
#define CONCPROP__NO_STEP         0x00000200
/* This means that tbonetest & livemask need to be computed before executing the concept. */
#define CONCPROP__GET_MASK        0x00000400
/* This means that the concept can be "standard". */
#define CONCPROP__STANDARD        0x00000800
/* If a concept takes one number, only CONCPROP__USE_NUMBER is set.
   If it takes two numbers both bits are set. */
#define CONCPROP__USE_NUMBER      0x00001000
#define CONCPROP__USE_TWO_NUMBERS 0x00002000
#define CONCPROP__NEED_3DMD       0x00004000
#define CONCPROP__NEED_3X4_1X12   0x00008000
#define CONCPROP__NEED_3X4        0x00010000
#define CONCPROP__NEED_4X4_1X16   0x00020000

typedef enum {    /* These control error messages that arise when we divide a setup
                     into subsetups (e.g. phantom lines) and find that one of
                     the setups is empty.  For example, if we are in normal lines
                     and select "phantom lines", it will notice that one of the setups
                     (the outer one) isn't occupied and will give the error message
                     "Don't use phantom concept if you don't mean it." */

   phantest_ok,               /* Anything goes */
   phantest_impossible,       /* Can't happen in symmetric stuff?? */
   phantest_both,             /* Require both setups (partially) occupied */
   phantest_only_one,         /* Require only one setup occupied, don't care which */
   phantest_only_first_one,   /* Require only first setup occupied, second empty */
   phantest_only_second_one,  /* Require only second setup occupied, first empty */
   phantest_first_or_both,    /* Require first setup only, or a mixture */
   phantest_2x2_both,
   phantest_2x2_only_two,
   phantest_not_just_centers
} phantest_kind;

typedef enum {
   disttest_t, disttest_nil, disttest_only_two,
   disttest_any, disttest_offset, disttest_z} disttest_kind;

/* BEWARE!!  If change this next definition, be sure to update the definition of
   "selector_names" and "selector_singular" in SDUTIL, and also necessary stuff in SDUI. */
typedef enum {
   selector_uninitialized,
   selector_boys,
   selector_girls,
   selector_heads,
   selector_sides,
   selector_headcorners,
   selector_sidecorners,
   selector_headboys,
   selector_headgirls,
   selector_sideboys,
   selector_sidegirls,
   selector_centers,
   selector_ends,
   selector_leads,
   selector_trailers,
   selector_beaux,
   selector_belles,
   selector_center2,
   selector_center6,
   selector_outer2,
   selector_outer6,
   selector_nearline,
   selector_farline,
   selector_nearcolumn,
   selector_farcolumn,
   selector_nearbox,
   selector_farbox,
   selector_all,
   selector_none
} selector_kind;
#define last_selector_kind ((int) selector_none)

typedef struct {
   char *name;
   concept_kind kind;
   dance_level level;
   struct {
      map_thing *maps;
      int arg1;
      int arg2;
      int arg3;
      int arg4;
      int arg5;
   } value;
} concept_descriptor;

typedef enum {
   simple_normalize,
   normalize_before_isolated_call,
   normalize_before_merge
} normalize_level;

typedef struct glock {
   concept_descriptor *concept;   /* the concept or end marker */
   callspec_block *call;          /* if this is end mark, gives the call; otherwise unused */
   struct glock *next;            /* next concept, or, if this is end mark, points to substitution list */
   struct glock *subsidiary_root; /* for concepts that take a second call, this is its parse root */
   struct glock *gc_ptr;          /* used for reclaiming dead blocks */
   selector_kind selector;        /* selector, if any, used by concept or call */
   int number;                    /* number, if any, used by concept or call */
} parse_block;

typedef struct {
   int bits[2];
} warning_info;

typedef struct {           /* This record is one state in the evolving sequence. */
   parse_block *command_root;
   setup state;
   resolve_indicator resolve_flag;
   long_boolean draw_pic;
   warning_info warnings;
   int centersp;           /* only nonzero for history[1] */
   int text_line;          /* how many lines of text existed after this item was written,
                              only meaningful if "written_history_items" is >= this index */
} configuration;

typedef struct {
   parse_block **concept_write_save_ptr;
   concept_kind save_concept_kind;
} parse_stack_item;

typedef struct {
   parse_stack_item parse_stack[40];
   int parse_stack_index;
   parse_block **concept_write_ptr;
   parse_block **concept_write_base;
   char *specialprompt;
   int topcallflags1;
   call_list_kind call_list_to_use;
} parse_state_type;

typedef struct {
   unsigned int concept_prop;      /* Takes bits of the form CONCPROP__??? */
   void (*concept_action)(setup *, parse_block *, setup *);
} concept_table_item;

typedef enum {
   file_write_no,
   file_write_double
} file_write_flag;

typedef enum {
   call_list_mode_none,
   call_list_mode_writing,
   call_list_mode_writing_full,
   call_list_mode_abridging
} call_list_mode_t;

typedef struct {
   char *name;
   long_boolean into_the_middle;
   setup the_setup;
} startinfo;

/* BEWARE!!  This next two definitions are keyed to stuff in SDRESOLVE or 
   the UI, particularly the array "title_string". */
typedef enum {
   search_anything,
   search_nice_setup,
   search_resolve,
   search_reconcile
} search_kind;

typedef enum {
   resolver_display_ok,
   resolver_display_searching,
   resolver_display_failed
} resolver_display_state;

#define cross_by_level l_c1
#define dixie_grand_level l_plus

typedef struct {           /* This is done to preserve the encapsulation of type "jmp_buf".                  */
   jmp_buf the_buf;        /*   We are going to use pointers to these things.  If we simply used             */
} real_jmp_buf;            /*   pointers to jmp_buf, the semantics would not be transparent if jmp_buf       */
                           /*   were defined as an array.  The semantics of pointers to arrays are different */
                           /*   from those of pointers to other types, and are not transparent.              */
                           /*   In particular, we would need to look inside the include file to see what     */
                           /*   underlying type jmp_buf is an array of, which would violate the principle    */
                           /*   of type encapsulation.                                                       */


/* Values returned by the various popup routines: */
#define POPUP_DECLINE 0
#define POPUP_ACCEPT  1
#define POPUP_ACCEPT_WITH_STRING 2


/* VARIABLES */

extern real_jmp_buf longjmp_buffer;                                 /* in SDUTIL */
extern real_jmp_buf *longjmp_ptr;                                   /* in SDUTIL */
extern configuration *history;                                      /* in SDUTIL */
extern int history_allocation;                                      /* in SDUTIL */
extern int history_ptr;                                             /* in SDUTIL */
extern int written_history_items;                                   /* in SDUTIL */
extern int written_history_nopic;                                   /* in SDUTIL */
extern parse_block *last_magic_diamond;                             /* in SDUTIL */
extern char error_message1[MAX_ERR_LENGTH];                         /* in SDUTIL */
extern char error_message2[MAX_ERR_LENGTH];                         /* in SDUTIL */
extern unsigned int collision_person1;                              /* in SDUTIL */
extern unsigned int collision_person2;                              /* in SDUTIL */
extern long_boolean enable_file_writing;                            /* in SDUTIL */
extern char *selector_names[];                                      /* in SDUTIL */

extern int global_tbonetest;                                        /* in SD */
extern int global_livemask;                                         /* in SD */
extern int global_selectmask;                                       /* in SD */
extern concept_table_item concept_table[];                          /* in SD */

extern concept_descriptor special_magic;                            /* in SDCTABLE */
extern concept_descriptor special_interlocked;                      /* in SDCTABLE */
extern concept_descriptor mark_end_of_list;                         /* in SDCTABLE */
extern concept_descriptor marker_decline;                           /* in SDCTABLE */
extern concept_descriptor marker_concept_mod;                       /* in SDCTABLE */
extern concept_descriptor marker_concept_modreact;                  /* in SDCTABLE */
extern concept_descriptor marker_concept_modtag;                    /* in SDCTABLE */
extern concept_descriptor marker_concept_force;                     /* in SDCTABLE */
extern concept_descriptor marker_concept_comment;                   /* in SDCTABLE */
extern callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];       /* in SDCTABLE */
extern int number_of_calls[NUM_CALL_LIST_KINDS];                    /* in SDCTABLE */
extern dance_level calling_level;                                   /* in SDCTABLE */
extern concept_descriptor concept_descriptor_table[];               /* in SDCTABLE */
extern int nice_setup_concept[];                                    /* in SDCTABLE */
extern int general_concept_offset;                                  /* in SDCTABLE */
extern int general_concept_size;                                    /* in SDCTABLE */
extern int *concept_offset_tables[];                                /* in SDCTABLE */
extern int *concept_size_tables[];                                  /* in SDCTABLE */
extern char *concept_menu_strings[];                                /* in SDCTABLE */

extern char *getout_strings[];                                      /* in SDTABLES */
extern char *filename_strings[];                                    /* in SDTABLES */
extern char *resolve_names[];                                       /* in SDTABLES */
extern char *resolve_distances[];                                   /* in SDTABLES */
extern char *menu_names[];                                          /* in SDTABLES */
extern begin_kind keytab[][2];                                      /* in SDTABLES */
extern coordrec *setup_coords[];                                    /* in SDTABLES */
extern coordrec *nice_setup_coords[];                               /* in SDTABLES */
extern int setup_limits[];                                          /* in SDTABLES */
extern int begin_sizes[];                                           /* in SDTABLES */
extern startinfo startinfolist[];                                   /* in SDTABLES */
extern map_thing map_b6_trngl;                                      /* in SDTABLES */
extern map_thing map_s6_trngl;                                      /* in SDTABLES */
extern map_thing map_2x2v;                                          /* in SDTABLES */
extern map_thing map_2x4_magic;                                     /* in SDTABLES */
extern map_thing map_qtg_magic;                                     /* in SDTABLES */
extern map_thing map_qtg_intlk;                                     /* in SDTABLES */
extern map_thing map_qtg_magic_intlk;                               /* in SDTABLES */
extern map_thing map_ptp_magic;                                     /* in SDTABLES */
extern map_thing map_ptp_intlk;                                     /* in SDTABLES */
extern map_thing map_ptp_magic_intlk;                               /* in SDTABLES */
extern map_thing map_2x4_diagonal;                                  /* in SDTABLES */
extern map_thing map_2x4_int_pgram;                                 /* in SDTABLES */
extern map_thing map_2x4_trapezoid;                                 /* in SDTABLES */
extern map_thing map_3x4_2x3_intlk;                                 /* in SDTABLES */
extern map_thing map_3x4_2x3_conc;                                  /* in SDTABLES */
extern map_thing map_4x4_ns;                                        /* in SDTABLES */
extern map_thing map_4x4_ew;                                        /* in SDTABLES */
extern map_thing map_phantom_box;                                   /* in SDTABLES */
extern map_thing map_intlk_phantom_box;                             /* in SDTABLES */
extern map_thing map_phantom_dmd;                                   /* in SDTABLES */
extern map_thing map_intlk_phantom_dmd;                             /* in SDTABLES */
extern map_thing map_split_f;                                       /* in SDTABLES */
extern map_thing map_intlk_f;                                       /* in SDTABLES */
extern map_thing map_full_f;                                        /* in SDTABLES */
extern map_thing map_stagger;                                       /* in SDTABLES */
extern map_thing map_stairst;                                       /* in SDTABLES */
extern map_thing map_ladder;                                        /* in SDTABLES */
extern map_thing map_but_o;                                         /* in SDTABLES */
extern map_thing map_o_s2x4_3;                                      /* in SDTABLES */
extern map_thing map_x_s2x4_3;                                      /* in SDTABLES */
extern map_thing map_offset;                                        /* in SDTABLES */
extern map_thing map_4x4v;                                          /* in SDTABLES */
extern map_thing map_blocks;                                        /* in SDTABLES */
extern map_thing map_trglbox;                                       /* in SDTABLES */
extern map_thing map_hv_2x4_2;                                      /* in SDTABLES */
extern map_thing map_3x4_2x3;                                       /* in SDTABLES */
extern map_thing map_4x6_2x4;                                       /* in SDTABLES */
extern map_thing map_hv_qtg_2;                                      /* in SDTABLES */
extern map_thing map_2x6_2x3;                                       /* in SDTABLES */
extern map_thing map_ov_s2x4_k;                                     /* in SDTABLES */
extern map_thing map_dbloff1;                                       /* in SDTABLES */
extern map_thing map_dbloff2;                                       /* in SDTABLES */
extern map_thing map_trngl_box1;                                    /* in SDTABLES */
extern map_thing map_trngl_box2;                                    /* in SDTABLES */
extern map_thing map_lh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_lh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_lf_s2x4_r;                                     /* in SDTABLES */
extern map_thing map_rf_s2x4_r;                                     /* in SDTABLES */
extern map_thing map_dmd_1x1;                                       /* in SDTABLES */
extern map_thing map_qtag_f0;                                       /* in SDTABLES */
extern map_thing map_qtag_f1;                                       /* in SDTABLES */
extern map_thing map_qtag_f2;                                       /* in SDTABLES */
extern map_thing *maps_3diag[4];                                    /* in SDTABLES */
extern map_thing *maps_3diagwk[4];                                  /* in SDTABLES */
extern map_hunk *map_lists[][4];                                    /* in SDTABLES */

/*
extern comment_block *comment_root;
extern comment_block *comment_last;
*/

extern int abs_max_calls;                                           /* in SDMAIN */
extern int max_base_calls;                                          /* in SDMAIN */
extern callspec_block **base_calls;                                 /* in SDMAIN */
extern char outfile_string[];                                       /* in SDMAIN */
extern int last_file_position;                                      /* in SDMAIN */
extern int global_age;                                              /* in SDMAIN */
extern parse_state_type parse_state;                                /* in SDMAIN */
extern int uims_menu_index;                                         /* in SDMAIN */
extern char database_version[81];                                   /* in SDMAIN */
extern int whole_sequence_low_lim;                                  /* in SDMAIN */
extern long_boolean not_interactive;                                /* in SDMAIN */
extern long_boolean initializing_database;                          /* in SDMAIN */
extern long_boolean testing_fidelity;                               /* in SDMAIN */
extern selector_kind selector_for_initialize;                       /* in SDMAIN */
extern int allowing_modifications;                                  /* in SDMAIN */

extern int random_number;                                           /* in SDSI */
extern int hashed_randoms;                                          /* in SDSI */
extern char *database_filename;					    /* in SDSI */

extern selector_kind current_selector;                              /* in PREDS */
extern long_boolean selector_used;                                  /* in PREDS */
extern long_boolean (*pred_table[])(                                /* in PREDS */
   setup *real_people,
   int real_index,
   int real_direction,
   int northified_index);

#ifdef __GNUC__
#define nonreturning volatile
#else
#define nonreturning
#endif

/* In SDMAIN */

extern char *sd_version_string(void);
extern parse_block *mark_parse_blocks(void);
extern void release_parse_blocks_to_mark(parse_block *mark_point);
extern void initialize_parse(void);
extern parse_block *copy_parse_tree(parse_block *original_tree);
extern void save_parse_state(void);
extern long_boolean restore_parse_state(void);
extern long_boolean deposit_call(callspec_block *call);
extern long_boolean deposit_concept(concept_descriptor *conc);
extern long_boolean query_for_call(void);
extern void write_header_stuff(void);
extern void get_real_subcall(
   parse_block *parseptr,
   by_def_item *item,
   final_set concin,
   parse_block **concptrout,
   callspec_block **callout,
   final_set *concout);
extern long_boolean sequence_is_resolved(void);
extern long_boolean write_sequence_to_file(void);

/* In PREDS */

extern long_boolean selectp(setup *ss, int place);

/* In SDINIT */

extern void initialize_menus(call_list_mode_t call_list_mode);

/* In SDSI */

extern void general_initialize(void);
extern int generate_random_number(int modulus);
extern long_boolean generate_random_concept_p(void);
extern void *get_mem(unsigned int siz);
extern void *get_mem_gracefully(unsigned int siz);
extern void *get_more_mem(void *oldp, unsigned int siz);
extern void *get_more_mem_gracefully(void *oldp, unsigned int siz);
extern void free_mem(void *ptr);
extern void get_date(char dest[]);
extern void unparse_number(int arg, char dest[]);
extern void open_file(void);
extern long_boolean probe_file(char filename[]);
extern void write_file(char line[]);
extern void close_file(void);
extern void print_line(char s[]);
extern void print_id_error(int n);
extern void init_error(char s[]);
extern void add_resolve_indices(char junk[], int cur, int max);
extern void final_exit(int code);
extern void open_database(void);
extern unsigned int read_8_from_database(void);
extern unsigned int read_16_from_database(void);
extern void close_database(void);
extern void fill_in_neglect_percentage(char junk[], int n);
extern int parse_number(char junk[]);
extern long_boolean open_call_list_file(call_list_mode_t call_list_mode, char filename[]);
extern char *read_from_call_list_file(char name[], int n);
extern void write_to_call_list_file(char name[]);
extern long_boolean close_call_list_file(void);

/* In SDUI */

extern void uims_process_command_line(int *argcp, char ***argvp);
extern void uims_preinitialize(void);
extern void uims_add_call_to_menu(call_list_kind cl, int call_menu_index, char name[]);
extern void uims_finish_call_menu(call_list_kind cl, char menu_name[]);
extern void uims_postinitialize(void);
extern int uims_do_outfile_popup(char dest[]);
extern int uims_do_comment_popup(char dest[]);
extern int uims_do_getout_popup(char dest[]);
extern int uims_do_abort_popup(void);
extern int uims_do_neglect_popup(char dest[]);
extern int uims_do_selector_popup(void);
extern int uims_do_quantifier_popup(void);
extern int uims_do_modifier_popup(char callname[], modify_popup_kind kind);
extern int uims_do_concept_popup(int kind);
extern void uims_reduce_line_count(int n);
extern void uims_add_new_line(char the_line[]);
extern uims_reply uims_get_command(mode_kind mode, call_list_kind call_menu, int modifications_flag);
extern void uims_begin_search(search_kind goal);
extern void uims_update_resolve_menu(search_kind goal, int cur, int max, resolver_display_state state);
extern int uims_begin_reconcile_history(int currentpoint, int maxpoint);
extern int uims_end_reconcile_history(void);
extern void uims_terminate(void);
extern char *uims_version_string(void);
extern void uims_database_tick_max(int n);
extern void uims_database_tick(int n);
extern void uims_database_tick_end(void);
extern void uims_database_error(char *message, char *call_name);
extern void uims_bad_argument(char *s1, char *s2, char *s3);

/* In SDUTIL */

extern void clear_screen(void);
extern void writestuff(char s[]);
extern void newline(void);
extern void doublespace_file(void);
extern void exit_program(int code);
extern void nonreturning fail(char s[]);
extern void nonreturning fail2(char s1[], char s2[]);
extern void specialfail(char s[]);
extern void string_copy(char **dest, char src[]);
extern void display_initial_history(int upper_limit, int num_pics);
extern void write_history_line(int history_index, char *header, long_boolean picture, file_write_flag write_to_file);
extern void warn(int w);
extern call_list_kind find_proper_call_list(setup *s);
extern callarray *assoc(begin_kind key, setup *ss, callarray *spec);
extern unsigned int find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index);
extern void clear_people(setup *z);
extern unsigned int rotperson(unsigned int n, int amount);
extern unsigned int rotcw(unsigned int n);
extern unsigned int rotccw(unsigned int n);
extern void clear_person(setup *resultpeople, int resultplace);
extern unsigned int copy_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace);
extern unsigned int copy_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount);
extern void swap_people(setup *ss, int oneplace, int otherplace);
extern void install_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace);
extern void install_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount);
extern parse_block *process_final_concepts(
   parse_block *cptr,
   long_boolean check_errors,
   final_set *final_concepts);
extern long_boolean fix_n_results(int arity, setup z[]);
extern void divided_setup_move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   map_thing *maps,
   phantest_kind phancontrol,
   long_boolean recompute_id,
   setup *result);
extern void overlapped_setup_move(setup *s, map_thing *maps,
   int m1, int m2, int m3, parse_block *parseptr, setup *result);

/* In SDGETOUT */

extern resolve_indicator resolve_p(setup *s);
extern uims_reply full_resolve(search_kind goal);
extern int concepts_in_place(void);
extern int reconcile_command_ok(int **permutation_map_p, int *accept_extend_p);
extern int resolve_command_ok(void);
extern int nice_setup_command_ok(void);
extern void create_resolve_menu_title(search_kind goal, int cur, int max, resolver_display_state state, char *title);

/* In SDBASIC */

extern void mirror_this(setup *s);
extern void do_stability(unsigned int *personp, unsigned int def_word, int turning);
extern void basic_move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   int tbonetest,
   long_boolean fudged,
   setup *result);

/* In SDMOVES */

extern void reinstate_rotation(setup *ss, setup *result);
extern void canonicalize_rotation(setup *result);

extern void move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   long_boolean qtfudged,
   setup *result);

/* In SDISTORT */

extern void distorted_2x2s_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void distorted_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void triple_twin_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void do_concept_rigger(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void do_concept_slider(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void do_concept_callrigger(
   setup *ss,
   parse_block *parseptr,
   setup *result);

/* In SD12 */

extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

/* In SD16 */

extern void phantom_2x4_move(
   setup *ss,
   int lineflag,
   phantest_kind phantest,
   map_thing *maps,
   parse_block *parseptr,
   setup *result);

/* In SD */

extern long_boolean do_big_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result);

/* In SDTAND */

extern void tandem_couples_move(
   setup *ss,
   parse_block *parseptr,
   callspec_block *callspec,
   final_set final_concepts,
   selector_kind selector,
   int twosome,               /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,              /* number, if doing fractional twosome/solid */
   int phantom,               /* normal=0 / phantom=1 / gruesome=2 */
   int tnd_cpl_siam,          /* tandem=0 / couples=1 / siamese=2 / skew=3 */
   setup *result);

extern void initialize_tandem_tables(void);

/* In SDCONC */

extern void concentric_move(
   setup *ss,
   parse_block *parsein,
   parse_block *parseout,
   callspec_block *callspecin,
   callspec_block *callspecout,
   final_set final_conceptsin,
   final_set final_conceptsout,
   calldef_schema analyzer,
   defmodset modifiersin1,
   defmodset modifiersout1,
   setup *result);

extern void normalize_concentric(
   calldef_schema synthesizer,
   int center_arity,
   setup inners[],
   setup *outers,
   int outer_elongation,
   setup *result);

extern void merge_setups(setup *ss, setup *result);

extern void on_your_own_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void so_and_so_only_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

/* In SDTOP */

extern void update_id_bits(setup *ss);

extern void touch_or_rear_back(
   setup *scopy,
   int callflags1);

extern void do_matrix_expansion(
   setup *ss,
   unsigned int concprops,
   long_boolean recompute_id);

extern void normalize_setup(setup *ss, normalize_level nlevel);

extern void toplevelmove(void);
