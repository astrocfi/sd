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

/* We would like to not need to customize things for different "dialects" of
   ANSI C, because we would like to think that there are no "dialects".  But, alas,
   there are two issues:
   (1) Some versions of GNU C (gcc) recognize the "volatile" keyword on a procedure
      as indicating that its call-return behavior is anomalous, and generate
      better code with that knowledge.  We can take advantage of that for some
      of our functions that never return, so we define a keyword "nonreturning".
   (2) Some compilers trying to pass for ANSI C have been observed failing
      to handle the "const" attribute.  (Yes, an ANSI C compiler that doesn't handle
      "const" is an oxymoron.)  We grudgingly accept such compilers if the
      "CONST_IS_BROKEN" symbol is defined.  We allow that to be set by a Makefile,
      and we set it ourselves for those compilers that we know about. */

/* Default is that "nonreturning" is meaningless. */
#define nonreturning

#ifdef __GNUC__
#if __GNUC__ >= 2
/* GNU C versions 2 or greater recognize volatile procedures. */
#undef nonreturning
#define nonreturning volatile
#else
/* GNU C versions less than 2 can't do "const". */
#define CONST_IS_BROKEN
#endif
#endif

#ifdef __CODECENTER_4__
#define CONST_IS_BROKEN		/* in CodeCenter 4.0.2 */
#endif

/* We will use "Const" with a capital "C" for our attempts at the "const" attribute. */
#ifndef CONST_IS_BROKEN
#define Const const
#else
/* Too bad.  Define it as nothing. */
#define Const
#endif

/* We use "Private" on procedures and "static" on variables.  It makes things clearer. */
#define Private static

/* We would like this to be a signed char, but not all compilers are fully ANSI compliant. */
/* The IBM AIX compiler, for example, considers char to be unsigned. */
typedef short veryshort;

/* We would like to think that we will always be able to count on compilers to do the
   right thing with "int" and "long int" and so on.  What we would really like is
   for compilers to be counted on to make "int" at least 32 bits, because we need
   32 bits in many places.  However, some compilers don't, so we have to use
   "long int" or "unsigned long int".  We think that all compilers we deal with
   will do the right thing with that, but, just in case, we use a typedef.

   The type "uint32" must be an unsigned (shifts don't propagate sign bits)
   datum on which we can do bitwise operations for at least the lowest 32 bits. */

typedef unsigned long int uint32;

#include <setjmp.h>
#include "database.h"

#define TRUE 1
#define FALSE 0
#define NULLCONCEPTPTR (concept_descriptor *) 0
#define NULLCALLSPEC (callspec_block *) 0

typedef int long_boolean;

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
   uint32 id1;       /* Frequently used bits go here. */
   uint32 id2;       /* Bits used for evaluating predicates. */
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
          4 000 - live person (just so at least one bit is always set)
          2 000 - active phantom (see below, under XPID_MASK)
          1 000 - virtual person (see below, under XPID_MASK)
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
#define BIT_PERSON   04000
#define BIT_ACT_PHAN 02000
#define BIT_TANDVIRT 01000

/* Person ID.  These bit positions are extremely hard wired into, among other
   things, the resolver and the printer. */
#define PID_MASK  0700

/* Extended person ID.  These 5 bits identify who the person is for the purpose
   of most manipulations.  0 to 7 are normal live people (the ones who squared up).
   8 to 15 are virtual people assembled for tandem or couples.  16 to 31 are
   active (but nevertheless identifiable) phantoms.  This means that, when
   BIT_ACT_PHAN is on, it usurps the meaning of BIT_TANDVIRT. */
#define XPID_MASK 03700

#define d_mask  04013
#define d_north 04010
#define d_south 04012
#define d_east  04001
#define d_west  04003

/* Person bits for "id2" field are:
 20 000 000 000 - headliner
 10 000 000 000 - sideliner
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
             10 - center 4
              4 - outside pairs
              2 -
              1 -
*/

#define ID2_HEADLINE 020000000000
#define ID2_SIDELINE 010000000000
#define ID2_CTR2      04000000000
#define ID2_BELLE     02000000000
#define ID2_BEAU      01000000000
#define ID2_CTR6       0400000000
#define ID2_OUTR2      0200000000
#define ID2_OUTR6      0100000000
#define ID2_TRAILER     040000000
#define ID2_LEAD        020000000
#define ID2_NSG         010000000
#define ID2_NSB          04000000
#define ID2_NHG          02000000
#define ID2_NHB          01000000
#define ID2_HCOR          0400000
#define ID2_SCOR          0200000
#define ID2_HEAD          0100000
#define ID2_SIDE           040000
#define ID2_BOY            020000
#define ID2_GIRL           010000
#define ID2_CENTER          04000
#define ID2_END             02000
#define ID2_NEARCOL         01000
#define ID2_NEARLINE         0400
#define ID2_NEARBOX          0200
#define ID2_FARCOL           0100
#define ID2_FARLINE           040
#define ID2_FARBOX            020
#define ID2_CTR4              010
#define ID2_OUTRPAIRS         004


typedef struct {
   setup_kind skind;
   int srotation;
} small_setup;

/* Flags that reside in the "cmd_misc_flags" word of a setup BEFORE a call is executed.

   BEWARE!! These flags co-exist in the cmd_misc_flags word with copies of some
   of the call invocation flags.  The mask for those flags is
   DFM1_CONCENTRICITY_FLAG_MASK.  Those flags, and that mask, are defined
   in database.h .  We define these flags at the extreme left end of the
   word in order to keep them away from the concentricity flags.

   CMD_MISC__EXPLICIT_MIRROR means that the setup has been mirrored by the "mirror"
   concept, separately from anything done by "left" or "reverse".  Such a mirroring
   does NOT cause the "take right hands" stuff to compensate by going to left hands,
   so it has the effect of making the people physically take left hands.

   CMD_MISC__MATRIX_CONCEPT means that the "matrix" concept has been given, and we have
   to be very careful about what we can do.  Specifically, this will make everything illegal
   except calls to suitable "split phantom" concepts.

   CMD_MISC__VERIFY_WAVES means that, before doing the call, we have to act as though the
   "assume waves" concept had been given.  That is, we have to verify that the setup is in fact
   waves, and we have to set the assumption stuff, so that a "with active phantoms" concept
   can be handled.  This flag is used only through "divided_setup_move", to propagate
   information from a concept like "split phantom waves" through the division, and then
   cause the assumption to be acted upon in each resulting setup.

   CMD_MISC__EXPLICIT_MATRIX means that the caller said "4x4 matrix" or "2x6 matrix" or whatever,
   so we got to this matrix explicitly.  This enables natural splitting of the setup, e.g. form
   a parallelogram, "2x6 matrix 1x2 checkmate" is legal -- the 2x6 gets divided naturally
   into 2x3's.

   CMD_MISC__NO_EXPAND_MATRIX means that we are at a level of recursion that no longer permits us to do the
   implicit expansion of the matrix (e.g. add outboard phantoms to turn a 2x4 into a 2x6
   if the concept "triple box" is used) that some concepts perform at the top level.

   CMD_MISC__DISTORTED means that we are at a level of recursion in which some
   distorted-setup concept has been used.  When this is set, "matrix" (a.k.a.
   "space invader") calls, such as press and truck, are not permitted.  Sorry, Clark.

   CMD_MISC__OFFSET_Z means that we are doing a 2x3 call (the only known case
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

   CMD_MISC__SAID_SPLIT means that we are at a level of recursion in which
   the "split" concept has been used.  When this is on, it is not permissible
   to do a 4-person call that has not 8-person definition, and hence would have
   been done the same way without the "split" concept.  This prevents superfluous
   things like "split pass thru".

   CMD_MISC__SAID_TRIANGLE means that we are at a level of recursion in which
   the word "triangle" has been uttered, so it is OK to do things like "triangle
   peel and trail" without giving the explicit "triangle" concept again.  This
   makes it possible to say things like "tandem-based triangles peel and trail".

   CMD_MISC__DO_AS_COUPLES means the obvious thing.

   CMD_FRAC__FRACTIONALIZE_MASK is a 9 bit field that is nonzero when some form
   of fractionalization control is in use.  These bits are set up by concepts like
   "random" and "fractional", and are used in sdmoves.c to control sequentially
   defined calls.  See the comments there for details.

   CMD_MISC__NO_CHK_ELONG means that the elongation of the incoming setup is for
   informational purposes only (to tell where people should finish) and should not
   be used for raising error messages.  It suppresses the error that would be
   raised if we have people in a facing 2x2 star thru when they are far from the
   ones they are facing.

   CMD_MISC__PHANTOMS means that we are at a level of recursion in which some phantom
   concept has been used, or when "on your own" or "so-and-so do your part" concepts
   are in use.  It indicates that we shouldn't be surprised if there are phantoms
   in the setup.  When this is set, the "tandem" or "as couples" concepts will
   forgive phantoms wherever they might appear, so that "split phantom lines tandem
   <call>" is legal.  Otherwise, we would have to say "split phantom lines phantom
   tandem <call>.  This flag also allows "basic_move" to be more creative about
   inferring the starting position for a call.  If I know that I am doing a
   "so-and-so do your part" operation, it may well be that, even though the setup
   is, say, point-to-point diamonds, the centers of the diamonds are none of my
   business (I am a point), and I want to think of the setup as a grand wave,
   so I can do my part of a grand mix.  The "CMD_MISC__PHANTOMS" flag makes it
   possible to do a grand mix from point-to-point diamonds in which the centers
   are absent.

   CMD_MISC__NO_STEP_TO_WAVE means that we are at a level of recursion that no longer permits us to do the
   implicit step to a wave or rear back from one that some calls permit at the top level.

   CMD_MISC__DOING_ENDS means that this call is directed only to the ends (and the setup is the ends
   of the original setup.  If the call turns out to be an 8-person call with distinct
   centers and ends parts, we may want to just apply the ends part.  This is what
   makes "ends detour" work.
*/

#define CMD_MISC__EXPLICIT_MIRROR    0x00000100
#define CMD_MISC__MATRIX_CONCEPT     0x00000200
#define CMD_MISC__VERIFY_WAVES       0x00000400
#define CMD_MISC__EXPLICIT_MATRIX    0x00000800
#define CMD_MISC__NO_EXPAND_MATRIX   0x00001000
#define CMD_MISC__DISTORTED          0x00002000
#define CMD_MISC__OFFSET_Z           0x00004000
#define CMD_MISC__SAID_SPLIT         0x00008000
#define CMD_MISC__SAID_TRIANGLE      0x00010000
#define CMD_MISC__PUT_FRAC_ON_FIRST  0x00020000
#define CMD_MISC__DO_AS_COUPLES      0x00040000
#define CMD_MISC__RESTRAIN_CRAZINESS 0x00080000
/* available:                        0x00100000
                                     0x00200000
                                     0x00400000
                                     0x00800000
                                     0x01000000
                                     0x02000000 */
#define CMD_MISC__MUST_SPLIT         0x04000000
#define CMD_MISC__CENTRAL            0x08000000
#define CMD_MISC__NO_CHK_ELONG       0x10000000
#define CMD_MISC__PHANTOMS           0x20000000
#define CMD_MISC__NO_STEP_TO_WAVE    0x40000000
#define CMD_MISC__DOING_ENDS         0x80000000

/* Flags that reside in the "cmd_frac_flags" word of a setup BEFORE a call is executed. */

/* This is a 9 bit field -- FRACTIONALIZE_BIT tells where its low bit lies. */
#define CMD_FRAC__FRACTIONALIZE_MASK 0x3FE00000
#define CMD_FRAC__FRACTIONALIZE_BIT  0x00200000



/* Flags that reside in the "result_flags" word of a setup AFTER a call is executed.

   The low two bits tell, for a 2x2 setup after
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
   such as "force_columns" was used.  Note that these two bits are in the same
   format as the "cmd.prior_elongation" field of a setup before the call is
   executed.  In many case, that field can be simply copied into the "result_flags"
   field.

   RESULTFLAG__DID_LAST_PART means that, when a sequentially defined call was executed
   with the CMD_FRAC__FRACTIONALIZE_MASK nonzero, so that just one part was done,
   that part was the last part.  Hence, if we are doing a call with some "piecewise"
   or "random" concept, we do that parts of the call one at a time, with appropriate
   concepts on each part, until it comes back with this flag set.

   RESULTFLAG__EXPAND_TO_2X3 means that a call was executed that takes a four person
   starting setup (typically a line) and yields a 2x3 result setup.  Two of those
   result spots will be phantoms, of course.  When recombining a divided setup in
   which such a call was executed, if the recombination wuold yield a 2x6, and if
   the center 4 spots would be empty, this flag directs divided_setup_move to
   collapse the setup to a 2x4.  This is what makes calls like "pair the line"
   and "step and slide" behave correctly from parallel lines and from a grand line.

   RESULTFLAG__NEED_DIAMOND means that a call has been executed with the "magic" or
   "interlocked" modifier, and it really should be changed to the "magic diamond"
   concept.  Otherwise, we might end up saying "magic diamond single wheel" when
   we should have said "magic diamond, diamond single wheel".

   RESULTFLAG__SPLIT_AXIS_MASK has info saying whether the call was split
   vertically (2) or horizontally (1) in "absolute space".  We can't say for
   sure whether the orientation is absolute, since a client may have stripped
   out rotation info.  What we mean is that it is relative to the incoming setup
   including the effect of its rotation field.  So, if the incoming setup was
   a 2x4 with rotation=0, that is, horizontally oriented in "absolute space",
   1 means it was done in 2x2's (split horizontally) and 2 means it was done
   in 1x4's (split vertically).  If the incoming setup was a 2x4 with
   rotation=1, that is, vertically oriented in "absolute space", 1 means it
   was done in 1x4's (split horizontally) and 2 means it was done in 2x2's
   (split vertically).  3 means that the call was a 1 or 2 person call, that
   could be split either way, so we have no information.
*/

/* The two low bits are used for result elongation, so we start with 0x00000004. */
#define RESULTFLAG__DID_LAST_PART   0x00000004
#define RESULTFLAG__EXPAND_TO_2X3   0x00000008
#define RESULTFLAG__NEED_DIAMOND    0x00000020
#define RESULTFLAG__IMPRECISE_ROT   0x00000040
/* This is a two-bit field. */
#define RESULTFLAG__SPLIT_AXIS_MASK 0x00000180
#define RESULTFLAG__SPLIT_AXIS_BIT  0x00000080

/* It should be noted that the CMD_MISC__??? and RESULTFLAG__XXX bits have
   nothing to do with each other.  It is not intended that
   the flags resulting from one call be passed to the next.  In fact, that
   would be incorrect.  The CMD_MISC__??? bits should start at zero at the
   beginning of each call, and accumulate stuff as the call goes deeper into
   recursion.  The RESULTFLAG__XXX bits should, in general, be the OR of the
   bits of the components of a compound call, though this may not be so for
   the elongation bits at the bottom of the word. */



typedef struct glork {
   struct glork *next;
   uint32 callarray_flags;
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
         struct predptr_pair_struct *predlist;
         /* Dynamically allocated to whatever size is required. */
         char errmsg[4];
      } prd;
   } stuff;
} callarray;

typedef struct {
   short call_id;
   uint32 modifiers1;
   uint32 modifiersh;
} by_def_item;

typedef struct glowk {
   uint32 modifier_seth;
   callarray *callarray_list;
   struct glowk *next;
   dance_level modifier_level;
} calldef_block;

typedef struct {
   uint32 callflags1;
   uint32 callflagsh;
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
         int howmanyparts;
         by_def_item *defarray;  /* Dynamically allocated, there are "howmanyparts" of them. */
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
   MPKIND__ALL_8,
   MPKIND__DMD_STUFF,
   MPKIND__STAG
} mpkind;

typedef struct {
   Const veryshort maps[32];
   Const mpkind map_kind;
   Const int arity;
   Const setup_kind outer_kind;
   Const setup_kind inner_kind;
   Const int rot;
   Const int vert;
} map_thing;

/* BEWARE!!  This list must track the array "concept_table" in sdconcpt.c . */
typedef enum {

/* These next few are not concepts.  Their appearance marks the end of a parse tree. */

   concept_another_call_next_mod,         /* calla modified by callb */
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
   concept_singlefile,
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
   concept_1x12_matrix,
   concept_1x16_matrix,
   concept_2x6_matrix,
   concept_2x8_matrix,
   concept_3x4_matrix,
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
   concept_do_phantom_qtags,
   concept_do_phantom_1x6,
   concept_do_phantom_1x8,
   concept_do_phantom_2x4,
   concept_do_phantom_2x3,
   concept_divided_2x4,
   concept_divided_2x3,
   concept_do_divided_diamonds,
   concept_do_divided_qtags,
   concept_distorted,
   concept_single_diagonal,
   concept_double_diagonal,
   concept_parallelogram,
   concept_triple_lines,
   concept_triple_lines_tog,
   concept_triple_lines_tog_std,
   concept_quad_lines,
   concept_quad_lines_tog,
   concept_quad_lines_tog_std,
   concept_quad_boxes,
   concept_quad_boxes_together,
   concept_triple_boxes,
   concept_triple_boxes_together,
   concept_triple_diamonds,
   concept_triple_diamonds_together,
   concept_quad_diamonds,
   concept_quad_diamonds_together,
   concept_inner_boxes,
   concept_triple_diag,
   concept_triple_diag_together,
   concept_triple_twin,
   concept_misc_distort,
   concept_old_stretch,
   concept_new_stretch,
   concept_assume_waves,
   concept_active_phantoms,
   concept_mirror,
   concept_central,
   concept_crazy,
   concept_frac_crazy,
   concept_fan_or_yoyo,
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
   concept_matrix,
   concept_double_offset,
   concept_checkpoint,
   concept_on_your_own,
   concept_trace,
   concept_ferris,
   concept_all_8,
   concept_centers_and_ends,
   concept_twice,
   concept_sequential,
   concept_special_sequential,
   concept_meta,
   concept_so_and_so_begin,
   concept_nth_part,
   concept_replace_nth_part,
   concept_interlace,
   concept_fractional,
   concept_rigger,
   concept_diagnose
} concept_kind;

typedef struct {
   Const char *name;
   Const concept_kind kind;
   Const long_boolean dup;
   Const dance_level level;
   Const struct {
      Const map_thing *maps;
      Const int arg1;
      Const int arg2;
      Const int arg3;
      Const int arg4;
      Const int arg5;
   } value;
} concept_descriptor;

/* BEWARE!!  If change this next definition, be sure to update the definition of
   "selector_names" and "selector_singular" in sdutil.c, and also necessary stuff in the
   user interfaces.  The latter includes the definition of "task$selector_menu" in sd.dps
   in the Domain/Dialog system, and the DITL "Select Dancers" in *.rsrc in
   the Macintosh system. */
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
   selector_beaus,
   selector_belles,
   selector_center2,
   selector_center6,
   selector_outer2,
   selector_outer6,
   selector_center4,
   selector_outerpairs,
   selector_headliners,
   selector_sideliners,
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

/* BEWARE!!  If change this next definition, be sure to update the definition of
   "direction_names" in sdutil.c, and also necessary stuff in the user interfaces.
   The latter includes the definition of "task$direction_menu" in sd.dps in the
   Domain/Dialog system, and the DITL "which direction" in *.rsrc in the Macintosh
   system. */
typedef enum {
   direction_uninitialized,
   direction_left,
   direction_right,
   direction_in,
   direction_out,
   direction_zigzag,
   direction_zagzig,
   direction_zigzig,
   direction_zagzag,
   direction_no_direction
} direction_kind;
#define last_direction_kind ((int) direction_no_direction)

typedef struct glock {
   concept_descriptor *concept;   /* the concept or end marker */
   callspec_block *call;          /* if this is end mark, gives the call; otherwise unused */
   struct glock *next;            /* next concept, or, if this is end mark, points to substitution list */
   struct glock *subsidiary_root; /* for concepts that take a second call, this is its parse root */
   struct glock *gc_ptr;          /* used for reclaiming dead blocks */
   selector_kind selector;        /* selector, if any, used by concept or call */
   direction_kind direction;      /* direction, if any, used by concept or call */
   uint32 number;                 /* number, if any, used by concept or call */
} parse_block;

/* The following items are not actually part of the setup description,
   but are placed here for the convenience of "move" and similar procedures.
   They contain information about the call to be executed in this setup.
   Once the call is complete, that is, when printing the setup or storing it
   in a history array, this stuff is meaningless. */

typedef struct {
   unsigned int assump_col: 16;  /* Stuff to go with assumption -- col vs. line. */
   unsigned int assump_both: 8;  /* Stuff to go with assumption -- "handedness" enforcement --
                                                0/1/2 = either/1st/2nd. */
   unsigned int assump_cast: 8;  /* Nonzero means there is an "assume normal casts" assumption. */
   call_restriction assumption;  /* Any "assume waves" type command. */
} assumption_thing;


typedef struct {
   parse_block *parseptr;        /* The full parse tree for the concept(s)/call(s) we are trying to do.
                                    While we traverse the big concepts in this tree, the "callspec" and
                                    "cmd_final_flags" are typically zero.  They only come into use when
                                    we reach the end of a subtree of big concepts, at which point we read
                                    the remaining "small" (or "final") concepts and the call name out of
                                    the end of the concept parse tree. */
   callspec_block *callspec;     /* The call, after we reach the end of the parse tree. */
   uint32 cmd_final_flags;       /* The various "final concept" flags with names INHERITFLAG_??? and FINAL__???.
                                    The INHERITFLAG_??? bits contain the final concepts like "single" that
                                    can be inherited from one part of a call definition to another.  The
                                    FINAL__??? bits contain other miscellaneous final concepts. */
   uint32 cmd_misc_flags;        /* Other miscellaneous info controlling the execution of the call,
                                    with names like CMD_MISC__???. */
   uint32 cmd_frac_flags;        /* Fractionalization info controlling the execution of the call. */
   assumption_thing cmd_assume;  /* Any "assume waves" type command. */
/*
   This field tells, for a 2x2 setup prior to having a call executed, how that
   2x2 is elongated (due to these people being the outsides) in the east-west
   or north-south direction.  Since 2x2's are always totally canonical, the
   interpretation of the elongation direction is always absolute.  A 1 means
   the elongation is east-west.  A 2 means the elongation is north-south.
   A zero means the elongation is unknown.
*/

   uint32 prior_elongation_bits;
} setup_command;


/* Warning!  Do not rearrange these fields without good reason.  There are data
   initializers instantiating these in sdinit.c (test_setup_???) and in sdtables.c
   (startinfolist) that will need to be rewritten. */
typedef struct {
   setup_kind kind;
   int rotation;
   setup_command cmd;
   personrec people[MAX_PEOPLE];

   /* The following item is not actually part of the setup description, but contains
      miscellaneous information left by "move" and similar procedures, for the
      convenience of whatever called same. */
   uint32 result_flags;           /* Miscellaneous info, with names like RESULTFLAG__???. */

   /* The following three items are only used if the setup kind is "s_normal_concentric".  Note in particular that
      "outer_elongation" is thus underutilized, and that a lot of complexity goes into storing similar information
      (in two different places!) in the "prior_elongation_bits" and "result_flags" words. */
   small_setup inner;
   small_setup outer;
   int outer_elongation;
} setup;

typedef long_boolean (*predicate_ptr)(
   setup *real_people,
   int real_index,
   int real_direction,
   int northified_index);

typedef struct predptr_pair_struct {
   predicate_ptr pred;
   struct predptr_pair_struct *next;
   /* Dynamically allocated to whatever size is required. */
   unsigned short arr[4];
} predptr_pair;

/* BEWARE!!  If change these next definitions, be sure to update the definition of
   "warning_strings" in sdutil.c . */
typedef enum {
   warn__none,
   warn__do_your_part,
   warn__tbonephantom,
   warn__awkward_centers,
   warn__bad_concept_level,  /* This must be in 1st 32, because of some sleaziness in resolver. */
   warn__not_funny,
   warn__hard_funny,
   warn__unusual,
   warn__rear_back,
   warn__awful_rear_back,
   warn__excess_split,
   warn__lineconc_perp,
   warn__dmdconc_perp,
   warn__lineconc_par,
   warn__dmdconc_par,
   warn__xclineconc_perp,
   warn__xcdmdconc_perp,
   warn__ctrstand_endscpls,
   warn__ctrscpls_endstand,
   warn__each2x2,
   warn__each1x4,
   warn__each1x2,
   warn__take_right_hands,
   warn__ctrs_are_dmd,
   warn__full_pgram,
   warn__offset_gone,
   warn__overlap_gone,
   warn__to_o_spots,
   warn__to_x_spots,
   warn__some_rear_back,
   warn__not_tbone_person,
   warn__check_c1_phan,
   warn__check_dmd_qtag,
   warn__check_2x4,
   warn__check_pgram,
   warn__dyp_resolve_ok,
   warn__ctrs_stay_in_ctr,
   warn__check_c1_stars,
   warn__bigblock_feet,
   warn__some_touch,
   warn__split_to_2x4s,
   warn__split_to_2x3s,
   warn__split_to_1x8s,
   warn__split_to_1x6s,
   warn__take_left_hands,
   warn__evil_interlocked,
   warn__split_phan_in_pgram,
   warn__bad_interlace_match,
   warn__not_on_block_spots,
   warn__canonicalize_bug,
   warn__did_not_interact
} warning_index;
#define NUM_WARNINGS (((int) warn__did_not_interact)+1)

/* BEWARE!!  This list must track the definition of "resolve_table" in sdgetout.c . */
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
   resolve_prom, resolve_revprom,
   resolve_sglfileprom, resolve_revsglfileprom,
   resolve_circle
} resolve_kind;

typedef struct {
   resolve_kind kind;
   int distance;
} resolve_indicator;

typedef enum {
   mode_none,     /* Not a real mode; used only for fictional purposes
                        in the user interface; never appears in the rest of the program. */
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
   ui_special_concept,  /* Not a real return; used only for fictional purposes
                              in the user interface; never appears in the rest of the program. */
   ui_command_select,   /* (normal/resolve) User chose one of the special buttons like "resolve" or "quit". */
   ui_resolve_select,   /* (resolve only) User chose one of the various actions peculiar to resolving. */
   ui_start_select,     /* (startup only) User chose something. This is the only outcome in startup mode. */
   ui_concept_select,   /* (normal only) User selected a concept. */
   ui_call_select       /* (normal only) User selected a call from the current call menu. */
} uims_reply;

/* In each case, an integer or enum is deposited into the global variable uims_menu_index.  Its interpretation
   depends on which of the replies above was given.  For some of the replies, it gives the index
   into a menu.  For "ui_start_select" it is a start_select_kind.
   For other replies, it is one of the following constants: */

/* BEWARE!!  This list must track the array "startup_commands" in sdmatch.c . */
/* BEWARE!!  If change this next definition, be sure to update the definition of
   "startinfolist" in sdtables.c, and also necessary stuff in the user interfaces.
   The latter includes the definition of "start_choices" in sd.dps
   in the Domain/Dialog system, and the corresponding CNTLs in *.rsrc
   in the Macintosh system.  You may also need changes in create_controls() in
   macstuff.c. */
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
   command_create_comment,
   command_change_outfile,
   command_getout,
   command_resolve,
   command_reconcile,
   command_anything,
   command_nice_setup,
#ifdef NEGLECT
   command_neglect,
#endif
   command_save_pic,
   command_refresh
} command_kind;
#define NUM_COMMAND_KINDS (((int) command_refresh)+1)

/* For ui_resolve_select: */
/* BEWARE!!  This list must track the array "resolve_commands" in sdmatch.c . */
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

/* BEWARE!!  There may be tables in the user interface file keyed to this enumeration.
   In particular, this list must track the array "menu_names" in sdtables.c . */
/* BEWARE!!  This list is keyed to some messy stuff in the procedure "initialize_concept_sublists".
   In particular, there are octal constants like "MASK_CTR_2" that contain bits assigned
   according to these items.  Changing these items is not recommended. */

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

typedef uint32 final_set;

typedef struct glonk {
   char txt[80];
   struct glonk *nxt;
} comment_block;

typedef struct flonk {
   char txt[80];
   struct flonk *nxt;
} outfile_block;

typedef uint32 defmodset;

typedef map_thing *map_hunk[][2];


/* These flags go into the "concept_prop" field of a "concept_table_item".

   CONCPROP__SECOND_CALL means that the concept takes a second call, so a sublist must
      be created, with a pointer to same just after the concept pointer.

   CONCPROP__USE_SELECTOR means that the concept requires a selector, which must be
      inserted into the concept list just after the concept pointer.

   CONCPROP__NEED_4X4   mean that the concept requires the indicated setup, and, at
   CONCPROP__NEED_2X8   the top level, the existing setup should be expanded as needed.
   CONCPROP__NEED_2X6 
   CONCPROP__NEED_4DMD
   CONCPROP__NEED_BLOB
   CONCPROP__NEED_4X6 

   CONCPROP__SET_PHANTOMS means that phantoms are in use under this concept, so that,
      when looking for tandems or couples, we shouldn't be disturbed if we
      pair someone with a phantom.  It is what makes "split phantom lines tandem"
      work, so that "split phantom lines phantom tandem" is unnecessary.

   CONCPROP__NO_STEP means that stepping to a wave or rearing back from one is not
      allowed under this concept.

   CONCPROP__GET_MASK means that tbonetest & livemask need to be computed before executing the concept.

   CONCPROP__STANDARD means that the concept can be "standard".

   CONCPROP__USE_NUMBER         If a concept takes one number, only CONCPROP__USE_NUMBER is set.
   CONCPROP__USE_TWO_NUMBERS    If it takes two numbers both bits are set.

   CONCPROP__SHOW_SPLIT means that the concept prepares the "split_axis" bits properly
      for transmission back to the client.  Normally this is off, and the split axis bits
      will be cleared after execution of the concept.
*/



#define CONCPROP__SECOND_CALL      0x00000001
#define CONCPROP__USE_SELECTOR     0x00000002
#define CONCPROP__NEED_4X4         0x00000004
#define CONCPROP__NEED_2X8         0x00000008
#define CONCPROP__NEED_2X6         0x00000010
#define CONCPROP__NEED_4DMD        0x00000020
#define CONCPROP__NEED_BLOB        0x00000040
#define CONCPROP__NEED_4X6         0x00000080
#define CONCPROP__SET_PHANTOMS     0x00000100
#define CONCPROP__NO_STEP          0x00000200
#define CONCPROP__GET_MASK         0x00000400
#define CONCPROP__STANDARD         0x00000800
#define CONCPROP__USE_NUMBER       0x00001000
#define CONCPROP__USE_TWO_NUMBERS  0x00002000
#define CONCPROP__NEED_3DMD        0x00004000
#define CONCPROP__NEED_1X12        0x00008000
#define CONCPROP__NEED_3X4         0x00010000
#define CONCPROP__NEED_1X16        0x00020000
#define CONCPROP__NEED_4X4_1X16    0x00040000
#define CONCPROP__NEED_3X4_1X12    0x00080000
#define CONCPROP__MATRIX_OBLIVIOUS 0x00100000
#define CONCPROP__PERMIT_MATRIX    0x00200000
#define CONCPROP__SHOW_SPLIT       0x00400000

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

typedef enum {
   simple_normalize,
   normalize_before_isolated_call,
   normalize_before_merge
} normalize_action;

typedef enum {
   merge_strict_matrix,
   merge_c1_phantom,
   merge_c1_phantom_nowarn,
   merge_without_gaps
} merge_action;

typedef enum {
   chk_none,
   chk_wave,
   chk_1_group,
   chk_2_groups,
   chk_4_groups,
   chk_box,
   chk_peelable
} chk_type;

typedef struct {
   int size;
   int map1[8];
   int map2[8];
   int map3[8];
   int map4[8];
   long_boolean ok_for_assume;
   chk_type check;
} restriction_thing;

typedef struct {
   uint32 bits[2];
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
   uint32 concept_prop;      /* Takes bits of the form CONCPROP__??? */
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

typedef struct {
   int *full_list;
   int *on_level_list;
   int full_list_size;
} nice_setup_thing;

typedef struct {
   setup_kind result_kind;
   int xfactor;
   veryshort xca[24];
   veryshort yca[24];
   veryshort diagram[64];
} coordrec;

typedef struct {
   int setup_limits;
   /* These "coordrec" items have the fudged coordinates that are used for doing
      press/truck calls.  For some setups, the coordinates of some people are
      deliberately moved away from the obvious precise matrix spots so that
      those people can't press or truck.  For example, the lateral spacing of
      diamond points is not an integer.  If a diamond point does any truck or loop
      call, he/she will not end up on the other diamond point spot (or any other
      spot in the formation), so the call will not be legal.  This enforces our
      view, not shared by all callers (Hi, Clark!) that the diamond points are NOT
      as if the ends of lines of 3, and hence can NOT trade with each other by
      doing a right loop 1. */
   coordrec *setup_coords;
   /* The above table is not suitable for performing mirror inversion because,
      for example, the points of diamonds do not reflect onto each other.  This
      table has unfudged coordinates, in which all the symmetries are observed.
      This is the table that is used for mirror reversal.  Most of the items in
      it are the same as those in the table above. */
   coordrec *nice_setup_coords;
   begin_kind keytab[2];
   /* In the bounding boxes, we do not fill in the "length" of a diamond, nor
      the "height" of a qtag.  Everyone knows that the number must be 3, but it
      is not really accepted that one can use that in instances where precision
      is required.  That is, one should not make "matrix" calls depend on this
      number.  Witness all the "diamond to quarter tag adjustment" stuff that
      callers worry about, and the ongoing controversy about which way a quarter
      tag setup is elongated, even though everyone knows that it is 4 wide and 3
      deep, and that it is generally recognized, by the mathematically erudite,
      that 4 is greater than 3. */
   short int bounding_box[2];
   /* This is true if the setup has 4-way symmetry.  Such setups will always be
      canonicalized so that their rotation field will be zero. */
   long_boolean four_way_symmetry;
} setup_attr;

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
extern uint32 collision_person1;                                    /* in SDUTIL */
extern uint32 collision_person2;                                    /* in SDUTIL */
extern long_boolean enable_file_writing;                            /* in SDUTIL */
extern char *selector_names[];                                      /* in SDUTIL */
extern char *direction_names[];                                     /* in SDUTIL */
extern char *warning_strings[];                                     /* in SDUTIL */

extern uint32 global_tbonetest;                                     /* in SDCONCPT */
extern uint32 global_livemask;                                      /* in SDCONCPT */
extern uint32 global_selectmask;                                    /* in SDCONCPT */
extern uint32 global_tboneselect;                                   /* in SDCONCPT */
extern concept_table_item concept_table[];                          /* in SDCONCPT */

extern concept_descriptor special_magic;                            /* in SDCTABLE */
extern concept_descriptor special_interlocked;                      /* in SDCTABLE */
extern concept_descriptor mark_end_of_list;                         /* in SDCTABLE */
extern concept_descriptor marker_decline;                           /* in SDCTABLE */
extern concept_descriptor marker_concept_mod;                       /* in SDCTABLE */
extern concept_descriptor marker_concept_comment;                   /* in SDCTABLE */
extern callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];       /* in SDCTABLE */
extern int number_of_calls[NUM_CALL_LIST_KINDS];                    /* in SDCTABLE */
extern dance_level calling_level;                                   /* in SDCTABLE */
extern concept_descriptor concept_descriptor_table[];               /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_4x4;                       /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_3x4;                       /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_2x8;                       /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_2x6;                       /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_1x12;                      /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_1x16;                      /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_3dmd;                      /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_4dmd;                      /* in SDCTABLE */
extern nice_setup_thing nice_setup_thing_4x6;                       /* in SDCTABLE */
extern int phantom_concept_index;
extern int general_concept_offset;                                  /* in SDCTABLE */
extern int general_concept_size;                                    /* in SDCTABLE */
extern int *concept_offset_tables[];                                /* in SDCTABLE */
extern int *concept_size_tables[];                                  /* in SDCTABLE */
extern Const char *concept_menu_strings[];                          /* in SDCTABLE */

extern char *getout_strings[];                                      /* in SDTABLES */
extern char *filename_strings[];                                    /* in SDTABLES */
extern char *menu_names[];                                          /* in SDTABLES */
extern setup_attr setup_attrs[];                                    /* in SDTABLES */
extern int begin_sizes[];                                           /* in SDTABLES */
extern startinfo startinfolist[];                                   /* in SDTABLES */
extern map_thing map_b6_trngl;                                      /* in SDTABLES */
extern map_thing map_s6_trngl;                                      /* in SDTABLES */
extern map_thing map_bone_trngl4;                                   /* in SDTABLES */
extern map_thing map_rig_trngl4;                                    /* in SDTABLES */
extern map_thing map_s8_tgl4;                                       /* in SDTABLES */
extern map_thing map_p8_tgl4;                                       /* in SDTABLES */
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
extern map_thing map_vv_qtg_2;                                      /* in SDTABLES */
extern map_thing map_tgl4_1;                                        /* in SDTABLES */
extern map_thing map_tgl4_2;                                        /* in SDTABLES */
extern map_thing map_2x6_2x3;                                       /* in SDTABLES */
extern map_thing map_dbloff1;                                       /* in SDTABLES */
extern map_thing map_dbloff2;                                       /* in SDTABLES */
extern map_thing map_trngl_box1;                                    /* in SDTABLES */
extern map_thing map_trngl_box2;                                    /* in SDTABLES */
extern map_thing map_inner_box;                                     /* in SDTABLES */
extern map_thing map_lh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_lh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_lz12;                                          /* in SDTABLES */
extern map_thing map_rz12;                                          /* in SDTABLES */
extern map_thing map_lof12;                                         /* in SDTABLES */
extern map_thing map_rof12;                                         /* in SDTABLES */
extern map_thing map_lof16;                                         /* in SDTABLES */
extern map_thing map_rof16;                                         /* in SDTABLES */
extern map_thing map_dmd_1x1;                                       /* in SDTABLES */
extern map_thing map_star_1x1;                                      /* in SDTABLES */
extern map_thing map_qtag_f0;                                       /* in SDTABLES */
extern map_thing map_qtag_f1;                                       /* in SDTABLES */
extern map_thing map_qtag_f2;                                       /* in SDTABLES */
extern map_thing map_diag2a;                                        /* in SDTABLES */
extern map_thing map_diag2b;                                        /* in SDTABLES */
extern map_thing map_f2x8_4x4;                                      /* in SDTABLES */
extern map_thing map_w4x4_4x4;                                      /* in SDTABLES */
extern map_thing map_f2x8_2x8;                                      /* in SDTABLES */
extern map_thing map_w4x4_2x8;                                      /* in SDTABLES */
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
extern long_boolean allowing_all_concepts;                          /* in SDMAIN */
extern long_boolean resolver_is_unwieldy;                           /* in SDMAIN */
extern long_boolean diagnostic_mode;                                /* in SDMAIN */
extern selector_kind current_selector;                              /* in SDMAIN */
extern direction_kind current_direction;                            /* in SDMAIN */
extern uint32 current_number_fields;                                /* in SDMAIN */
extern warning_info no_search_warnings;                             /* in SDMAIN */
extern warning_info conc_elong_warnings;                            /* in SDMAIN */
extern warning_info dyp_each_warnings;                              /* in SDMAIN */

extern int random_number;                                           /* in SDSI */
extern int hashed_randoms;                                          /* in SDSI */
extern char *database_filename;                                     /* in SDSI */

extern long_boolean selector_used;                                  /* in PREDS */
extern long_boolean (*pred_table[])(                                /* in PREDS */
   setup *real_people,
   int real_index,
   int real_direction,
   int northified_index);

/* In SDMAIN */

extern char *sd_version_string(void);
extern parse_block *mark_parse_blocks(void);
extern void release_parse_blocks_to_mark(parse_block *mark_point);
extern void initialize_parse(void);
extern parse_block *copy_parse_tree(parse_block *original_tree);
extern void save_parse_state(void);
extern long_boolean restore_parse_state(void);
extern long_boolean deposit_call(callspec_block *call);
extern long_boolean deposit_concept(concept_descriptor *conc, uint32 number_fields);
extern long_boolean query_for_call(void);
extern void write_header_stuff(long_boolean with_ui_version);
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
extern void uims_add_call_to_menu(call_list_kind cl, int call_menu_index, Const char name[]);
extern void uims_finish_call_menu(call_list_kind cl, Const char menu_name[]);
extern void uims_postinitialize(void);
extern int uims_do_outfile_popup(char dest[]);
extern int uims_do_comment_popup(char dest[]);
extern int uims_do_getout_popup(char dest[]);
extern int uims_do_abort_popup(void);
extern int uims_do_neglect_popup(char dest[]);
extern int uims_do_selector_popup(void);
extern int uims_do_direction_popup(void);
extern int uims_do_modifier_popup(char callname[], modify_popup_kind kind);
extern uint32 uims_get_number_fields(int nnumbers);
extern void uims_reduce_line_count(int n);
extern void uims_add_new_line(char the_line[]);
extern uims_reply uims_get_command(mode_kind mode, call_list_kind *call_menu);
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
extern void uims_debug_print(char *s);		/* Alan's code only */

/* In SDUTIL */

extern void clear_screen(void);
extern void writestuff(Const char s[]);
extern void newline(void);
extern void doublespace_file(void);
extern void exit_program(int code);
extern void nonreturning fail(Const char s[]);
extern void nonreturning fail2(Const char s1[], Const char s2[]);
extern void nonreturning specialfail(Const char s[]);
extern void string_copy(char **dest, char src[]);
extern void display_initial_history(int upper_limit, int num_pics);
extern void write_history_line(int history_index, Const char *header, long_boolean picture, file_write_flag write_to_file);
extern void warn(warning_index w);
extern call_list_kind find_proper_call_list(setup *s);
extern callarray *assoc(begin_kind key, setup *ss, callarray *spec);
extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index);
extern void clear_people(setup *z);
extern uint32 rotperson(uint32 n, int amount);
extern uint32 rotcw(uint32 n);
extern uint32 rotccw(uint32 n);
extern void clear_person(setup *resultpeople, int resultplace);
extern uint32 copy_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace);
extern uint32 copy_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount);
extern void swap_people(setup *ss, int oneplace, int otherplace);
extern void install_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace);
extern void install_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount);
extern parse_block *process_final_concepts(
   parse_block *cptr,
   long_boolean check_errors,
   final_set *final_concepts);
extern long_boolean fix_n_results(int arity, setup z[]);

/* In SDGETOUT */

extern resolve_indicator resolve_p(setup *s);
extern void write_resolve_text(void);
extern uims_reply full_resolve(search_kind goal);
extern int concepts_in_place(void);
extern int reconcile_command_ok(void);
extern int resolve_command_ok(void);
extern int nice_setup_command_ok(void);
extern void create_resolve_menu_title(search_kind goal, int cur, int max, resolver_display_state state, char *title);
extern void initialize_getout_tables(void);

/* In SDBASIC */

extern void mirror_this(setup *s);
extern void do_stability(uint32 *personp, stability stab, int turning);
extern void check_restriction(setup *ss, assumption_thing restr, unsigned int flags);
extern void basic_move(
   setup *ss,
   int tbonetest,
   long_boolean fudged,
   long_boolean mirror,
   setup *result);

/* In SDMOVES */

extern void canonicalize_rotation(setup *result);
extern void reinstate_rotation(setup *ss, setup *result);

extern long_boolean divide_for_magic(
   setup *ss,
   uint32 flags_to_use,
   uint32 flags_to_check,
   setup *result);

extern void do_call_in_series(
   setup *sss,
   long_boolean roll_transparent,
   long_boolean normalize,
   long_boolean qtfudged);

extern void move(
   setup *ss,
   long_boolean qtfudged,
   setup *result);

/* In SDMISC */

extern void divided_setup_move(
   setup *ss,
   Const map_thing *maps,
   phantest_kind phancontrol,
   long_boolean recompute_id,
   setup *result);

extern void overlapped_setup_move(setup *ss, Const map_thing *maps,
   int m1, int m2, int m3, setup *result);

extern void do_phantom_2x4_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void phantom_2x4_move(
   setup *ss,
   int lineflag,
   phantest_kind phantest,
   Const map_thing *maps,
   setup *result);

extern void distorted_2x2s_move(
   setup *ss,
   concept_descriptor *this_concept,
   setup *result);

extern void distorted_move(
   setup *ss,
   parse_block *parseptr,
   disttest_kind disttest,
   setup *result);

extern void triple_twin_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void do_concept_rigger(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

/* In SDCONCPT */

extern restriction_thing *get_restriction_thing(setup_kind k, assumption_thing t);

extern long_boolean do_big_concept(
   setup *ss,
   setup *result);

/* In SDTAND */

extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,               /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,              /* number, if doing fractional twosome/solid */
   int phantom,               /* normal=0 / phantom=1 / gruesome=2 */
   int tnd_cpl_siam,          /* tandem = 0 / couples = 1 / siamese = 2 / skew = 3
                                 tandem of 3 = 4 / couples of 3 = 5 / tandem of 4 = 6 / couples of 4 = 7
                                 box = 8 / diamond = 9 */
   setup *result);

extern void initialize_tandem_tables(void);

/* In SDCONC */

extern void concentric_move(
   setup *ss,
   setup_command *cmdin,
   setup_command *cmdout,
   calldef_schema analyzer,
   defmodset modifiersin1,
   defmodset modifiersout1,
   setup *result);

extern uint32 get_multiple_parallel_resultflags(setup outer_inners[], int number);

extern void normalize_concentric(
   calldef_schema synthesizer,
   int center_arity,
   setup outer_inners[],   /* outers in position 0, inners follow */
   int outer_elongation,
   setup *result);

extern void merge_setups(setup *ss, merge_action action, setup *result);

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
   long_boolean did_mirror,
   int callflags1);

extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   long_boolean recompute_id);

extern void normalize_setup(setup *ss, normalize_action action);

extern void toplevelmove(void);
