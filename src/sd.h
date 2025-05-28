/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

#include <stdio.h>
#include <setjmp.h>

#include "basetype.h"
#include "sdui.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "sdcom.h"

#define EXPIRATION_STATE_BITS (RESULTFLAG__YOYO_FINISHED|RESULTFLAG__TWISTED_FINISHED|RESULTFLAG__SPLIT_FINISHED)


/* It should be noted that the CMD_MISC__??? and RESULTFLAG__XXX bits have
   nothing to do with each other.  It is not intended that
   the flags resulting from one call be passed to the next.  In fact, that
   would be incorrect.  The CMD_MISC__??? bits should start at zero at the
   beginning of each call, and accumulate stuff as the call goes deeper into
   recursion.  The RESULTFLAG__??? bits should, in general, be the OR of the
   bits of the components of a compound call, though this may not be so for
   the elongation bits at the bottom of the word. */



#define TEST_HERITBITS(x,y) ((x).her8it & (y))


typedef struct {
   long_boolean reverse_order;
   long_boolean first_call;
   int instant_stop;
   uint32 do_half_of_last_part;
   uint32 do_last_half_of_first_part;
   int highlimit;
   int subcall_index;
   int subcall_incr;
} fraction_info;


/* Meanings of the items in a "setup_command" structure:

   parseptr
      The full parse tree for the concept(s)/call(s) we are trying to do.
      While we traverse the big concepts in this tree, the "callspec" and
      "cmd_final_flags" are typically zero.  They only come into use when
      we reach the end of a subtree of big concepts, at which point we read
      the remaining "small" (or "final") concepts and the call name out of
      the end of the concept parse tree.

   callspec
      The call, after we reach the end of the parse tree.

   cmd_final_flags
      The various "final concept" flags with names INHERITFLAG_??? and FINAL__???.
      The INHERITFLAG_??? bits contain the final concepts like "single" that
      can be inherited from one part of a call definition to another.  The
      FINAL__??? bits contain other miscellaneous final concepts.

   cmd_misc_flags
      Other miscellaneous info controlling the execution of the call,
      with names like CMD_MISC__???.

   cmd_misc2_flags
      Even more miscellaneous info controlling the execution of the call,
      with names like CMD_MISC2__???.

   do_couples_heritflags
      These are "heritable" flags that tell what single/3x3/etc modifiers
      are to be used in determining what to do when CMD_MISC__DO_AS_COUPLES
      is specified.

   cmd_frac_flags
      If nonzero, fractionalization info controlling the execution of the call.
      See the comments in front of "get_fraction_info" in sdmoves.c for details.

   cmd_assume
      Any "assume waves" type command.

   skippable_concept
      For "<so-and-so> work <concept>", this is the concept to be skipped by some people.

   prior_elongation_bits;
      This tells, for a 2x2 setup prior to having a call executed, how that
      2x2 is elongated (due to these people being the outsides) in the east-west
      or north-south direction.  Since 2x2's are always totally canonical, the
      interpretation of the elongation direction is always absolute.  A 1 means
      the elongation is east-west.  A 2 means the elongation is north-south.
      A zero means there was no elongation. */

static const uint32 CMD_FRAC_NULL_VALUE      = 0x00000111UL;
static const uint32 CMD_FRAC_HALF_VALUE      = 0x00000112UL;
static const uint32 CMD_FRAC_LASTHALF_VALUE  = 0x00001211UL;

static const uint32 CMD_FRAC_PART_BIT        = 0x00010000UL;
static const uint32 CMD_FRAC_PART_MASK       = 0x000F0000UL;
static const uint32 CMD_FRAC_REVERSE         = 0x00100000UL;
static const uint32 CMD_FRAC_CODE_MASK       = 0x00E00000UL;    // This is a 3 bit field.

/* Here are the codes that can be inside.  We require that CMD_FRAC_CODE_ONLY be zero.
   We require that the PART_MASK field be nonzero (we use 1-based part numbering)
   when these are in use.  If the PART_MASK field is zero, the code must be zero
   (that is, CMD_FRAC_CODE_ONLY), and this stuff is not in use. */

static const uint32 CMD_FRAC_CODE_ONLY           = 0x00000000UL;
static const uint32 CMD_FRAC_CODE_ONLYREV        = 0x00200000UL;
static const uint32 CMD_FRAC_CODE_FROMTO         = 0x00400000UL;
static const uint32 CMD_FRAC_CODE_FROMTOREV      = 0x00600000UL;
static const uint32 CMD_FRAC_CODE_FROMTOREVREV   = 0x00800000UL;
static const uint32 CMD_FRAC_CODE_FROMTOMOST     = 0x00A00000UL;
static const uint32 CMD_FRAC_CODE_LATEFROMTOREV  = 0x00C00000UL;

static const uint32 CMD_FRAC_PART2_BIT       = 0x01000000UL;
static const uint32 CMD_FRAC_PART2_MASK      = 0x07000000UL;
static const uint32 CMD_FRAC_IMPROPER_BIT    = 0x08000000UL;
static const uint32 CMD_FRAC_BREAKING_UP     = 0x10000000UL;
static const uint32 CMD_FRAC_FORCE_VIS       = 0x20000000UL;
static const uint32 CMD_FRAC_LASTHALF_ALL    = 0x40000000UL;
static const uint32 CMD_FRAC_FIRSTHALF_ALL   = 0x80000000UL;


/* These flags go along for the ride, in some parts of the code (BUT NOT
   THE CALLFLAGSF WORD OF A CALLSPEC!), in the same word as the heritable flags,
   but are not part of the inheritance mechanism.  We use symbols that have been
   graciously provided for us from database.h to tell us what bits may be safely
   used next to the heritable flags. */

#define FINAL__SPLIT                             CFLAGHSPARE_1
static const uint32 FINAL__SPLIT_SQUARE_APPROVED      = CFLAGHSPARE_2;
static const uint32 FINAL__SPLIT_DIXIE_APPROVED       = CFLAGHSPARE_3;
static const uint32 FINAL__MUST_BE_TAG                = CFLAGHSPARE_4;
static const uint32 FINAL__TRIANGLE                   = CFLAGHSPARE_5;
static const uint32 FINAL__LEADTRIANGLE               = CFLAGHSPARE_6;

/* Flags that reside in the "cmd_misc_flags" word of a setup BEFORE a call is executed.

   BEWARE!! These flags co-exist in the cmd_misc_flags word with copies of some
   of the call invocation flags.  The mask for those flags is
   DFM1_CONCENTRICITY_FLAG_MASK.  Those flags, and that mask, are defined
   in database.h .  We must define these flags starting where the concentricity
   flags end.

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
   cause the assumption to be acted upon in each resulting setup.  It is removed immediately
   by "divided_setup_move" after use.

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

   CMD_MISC__DO_AS_COUPLES means that the "couples_unless_single" invocation
   flag is on, and the call may need to be done as couples or whatever.  The
   bits in "do_couples_heritflags" control this.  If INHERITFLAG_SINGLE is on,
   do not do it as couples.  If off, do it as couples.  If various other bits
   are on (e.g. INHERITFLAG_1X3), do the appropriate thing.

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

   CMD_MISC__DOING_ENDS means that this call is directed only to the ends
   of the original setup.  If the call turns out to be an 8-person call with distinct
   centers and ends parts, we may want to just apply the ends part.  This is what
   makes "ends detour" work.
*/

/* Since DFM1_CONCENTRICITY_FLAG_MASK is FF, we start at 100. */

static const uint32 CMD_MISC__EXPLICIT_MIRROR    = 0x00000100UL;
static const uint32 CMD_MISC__MATRIX_CONCEPT     = 0x00000200UL;
/* This is a 4 bit field.  For codes inside same, see "CMD_MISC__VERIFY_WAVES" below. */
static const uint32 CMD_MISC__VERIFY_MASK        = 0x00003C00UL;
static const uint32 CMD_MISC__EXPLICIT_MATRIX    = 0x00004000UL;
static const uint32 CMD_MISC__NO_EXPAND_MATRIX   = 0x00008000UL;
static const uint32 CMD_MISC__DISTORTED          = 0x00010000UL;
static const uint32 CMD_MISC__OFFSET_Z           = 0x00020000UL;
static const uint32 CMD_MISC__SAID_SPLIT         = 0x00040000UL;
static const uint32 CMD_MISC__SAID_TRIANGLE      = 0x00080000UL;
static const uint32 CMD_MISC__PUT_FRAC_ON_FIRST  = 0x00100000UL;
static const uint32 CMD_MISC__DO_AS_COUPLES      = 0x00200000UL;
static const uint32 CMD_MISC__RESTRAIN_CRAZINESS = 0x00400000UL;
static const uint32 CMD_MISC__RESTRAIN_MODIFIERS = 0x00800000UL;
static const uint32 CMD_MISC__NO_CHECK_MOD_LEVEL = 0x01000000UL;
#define             CMD_MISC__MUST_SPLIT_HORIZ     0x02000000UL
#define             CMD_MISC__MUST_SPLIT_VERT      0x04000000UL
static const uint32 CMD_MISC__NO_CHK_ELONG       = 0x08000000UL;
static const uint32 CMD_MISC__PHANTOMS           = 0x10000000UL;
static const uint32 CMD_MISC__NO_STEP_TO_WAVE    = 0x20000000UL;
static const uint32 CMD_MISC__ALREADY_STEPPED    = 0x40000000UL;
static const uint32 CMD_MISC__DOING_ENDS         = 0x80000000UL;

static const uint32 CMD_MISC__MUST_SPLIT_MASK    = (CMD_MISC__MUST_SPLIT_HORIZ|CMD_MISC__MUST_SPLIT_VERT);

/* Here are the encodings that can go into the CMD_MISC__VERIFY_MASK field.
   Zero means no verification. */
#define             CMD_MISC__VERIFY_WAVES         0x00000400UL
#define             CMD_MISC__VERIFY_2FL           0x00000800UL
#define             CMD_MISC__VERIFY_DMD_LIKE      0x00000C00UL
#define             CMD_MISC__VERIFY_QTAG_LIKE     0x00001000UL
#define             CMD_MISC__VERIFY_1_4_TAG       0x00001400UL
#define             CMD_MISC__VERIFY_3_4_TAG       0x00001800UL
#define             CMD_MISC__VERIFY_REAL_1_4_TAG  0x00001C00UL
#define             CMD_MISC__VERIFY_REAL_3_4_TAG  0x00002000UL
#define             CMD_MISC__VERIFY_REAL_1_4_LINE 0x00002400UL
#define             CMD_MISC__VERIFY_REAL_3_4_LINE 0x00002800UL
#define             CMD_MISC__VERIFY_LINES         0x00002C00UL
#define             CMD_MISC__VERIFY_COLS          0x00003000UL


/* Flags that reside in the "cmd_misc2_flags" word of a setup BEFORE a call is executed. */

/* The following are used for the "<anyone> work <concept>" or "snag the <anyone>" mechanism:

   CMD_MISC2__ANY_WORK or CMD_MISC2__ANY_SNAG is on if there is such an
   operation in place.  We will make the centers or ends (depending on
   CMD_MISC2__ANY_WORK_INVERT) use the next concept (or do 1/2 of the call)
   while the others skip that concept.  The schema, which is one of
   schema_concentric_2_6, schema_concentric_6_2, schema_concentric, or
   schema_single_concentric, is in the low 16 bits.

   CMD_MISC2__ANY_WORK_INVERT is only meaningful if the CMD_MISC2__ANY_WORK is on.
   It says that the ends are doing the concept, instead of the centers. */

/* The following are used for Z's.
   CMD_MISC2__IN_Z_CW and CMD_MISC2__IN_Z_CCW say that the setup is actually
   a 2x3, but the "Z" (or "each Z", or "triple Z's") concept has been given,
   and the setup should probably be turned into a 2x2.  The only exception
   is if the call takes a 2x3 starting setup but not a 2x2 (that is, the call
   is "Z axle").  In that case, the call is done directly in the 2x3, and the
   "Z" distortion is presumed not to have been in place. */

/* The following are used for "mystic" and old "snag".

   CMD_MISC2__MYSTIFY_SPLIT tells "divided_setup_move" to perform selective mirroring
   of the subsidiary setups because a concept like "mystic triple boxes" is in use.
   It is removed immediately by "divided_setup_move" after use.

   CMD_MISC2__MYSTIFY_INVERT is only meaningful when CMD_MISC2__MYSTIFY_SPLIT is on.
   It says that the concept is actually "invert mystic triple boxes" or whatever. */

/*  The following are used for what we call the "center/end" mechanism.  This
    mechanism is used for the "invert" (centers and ends) concept, as well as
    "central", "snag", and "mystic" and inverts thereof.

      CMD_MISC2__CTR_END_KMASK, when nonzero, says that one of the "central", "snag",
      or "mystic" concepts is in use.  They are all closely related.

      CMD_MISC2__CTR_END_INVERT means that the call is being inverted.  It is
      orthogonal to such issues as "snag" and "invert snag".  "Invert snag invert
      strike out" means that the physical ends do only half, and everyone do
      an invert strike out.  That is, the ends hinge but do not step & fold,
      while the centers detour.

      CMD_MISC2__CTR_END_MASK embraces all of the bits of the "center/end" mechanism.
*/

/*     The low 12 bits are used for encoding the schema if
       CMD_MISC2__ANY_WORK or CMD_MISC2__ANY_SNAG is on */

static const uint32 CMD_MISC2__IN_Z_CW           = 0x00001000UL;
static const uint32 CMD_MISC2__IN_Z_CCW          = 0x00002000UL;
static const uint32 CMD_MISC2__IN_AZ_CW          = 0x00004000UL;
static const uint32 CMD_MISC2__IN_AZ_CCW         = 0x00008000UL;
static const uint32 CMD_MISC2__IN_Z_MASK         = 0x0000F000UL;
static const uint32 CMD_MISC2__DID_Z_COMPRESSION = 0x00010000UL;

static const uint32 CMD_MISC2__MYSTIFY_SPLIT     = 0x00020000UL;
static const uint32 CMD_MISC2__MYSTIFY_INVERT    = 0x00040000UL;

static const uint32 CMD_MISC2__ANY_WORK          = 0x00080000UL;
static const uint32 CMD_MISC2__ANY_SNAG          = 0x00100000UL;
static const uint32 CMD_MISC2__ANY_WORK_INVERT   = 0x00200000UL;


/* Here are the inversion bits for the basic operations. */
#define             CMD_MISC2__INVERT_CENTRAL      0x00400000UL
#define             CMD_MISC2__INVERT_SNAG         0x00800000UL
#define             CMD_MISC2__INVERT_MYSTIC       0x01000000UL
/* Here are the basic operations we can do. */
#define             CMD_MISC2__DO_CENTRAL          0x02000000UL
#define             CMD_MISC2__CENTRAL_SNAG        0x04000000UL
#define             CMD_MISC2__CENTRAL_MYSTIC      0x08000000UL
/* This field embraces the above 3 bits. */
#define             CMD_MISC2__CTR_END_KMASK       0x0E000000UL

/* This says the the operator said "invert".  It might later cause
   a "central" to be turned into an "invert central". */
#define             CMD_MISC2__SAID_INVERT         0x10000000UL
/* This mask embraces this whole mechanism, including the "invert" bit. */
#define             CMD_MISC2__CTR_END_MASK        0x1FC00000UL
static const uint32 CMD_MISC2_RESTRAINED_SUPER   = 0x20000000UL;

typedef enum {
   simple_normalize,
   normalize_before_isolated_call,
   normalize_before_isolate_strict,
   normalize_to_6,
   normalize_to_4,
   normalize_to_2,
   normalize_after_triple_squash,
   normalize_before_merge,
   normalize_compress_bigdmd,
   normalize_recenter
} normalize_action;

typedef enum {
   merge_strict_matrix,
   merge_c1_phantom,
   merge_c1_phantom_nowarn,
   merge_c1phan_nocompress,
   merge_without_gaps
} merge_action;

/* We sometimes don't want to use "const" in C++,
   because initializers are screwed up.
   In particular, we need this on any structure type
   that is initialized sometimes in C files (e.g. sdtables.c)
   and sometimes in C++ files (e.g. the structure "expand_thing"
   has initializers in sdmoves.cpp as well.)
*/
#ifdef __cplusplus
#define C_const
#else
#define C_const const
#endif

typedef struct grylch {
   C_const veryshort source_indices[24];
   C_const int size;
   C_const setup_kind inner_kind;
   C_const setup_kind outer_kind;
   C_const int rot;
   C_const uint32 lillivemask;
   C_const uint32 biglivemask;
   C_const warning_index expwarning;
   C_const warning_index norwarning;
   C_const normalize_action action_level;
   C_const uint32 expandconcpropmask;
   struct grylch *next_expand;
   struct grylch *next_compress;
} expand_thing;

typedef struct grilch {
   C_const warning_index warning;
   C_const int forbidden_elongation;   /* Low 2 bits = elongation bits to forbid;
                                        "4" bit = must set elongation.
                                        Also, the "8" bit means to use "gather"
                                        and do this the other way.
                                        Also, the "16" bit means allow only step
                                        to a box, not step to a full wave. */
   C_const expand_thing *expand_lists;
   C_const setup_kind kind;
   C_const uint32 live;
   C_const uint32 dir;
   C_const uint32 dirmask;
   struct grilch *next;
} full_expand_thing;


typedef enum {    /* These control error messages that arise when we divide a setup
                     into subsetups (e.g. phantom lines) and find that one of
                     the setups is empty.  For example, if we are in normal lines
                     and select "phantom lines", it will notice that one of the setups
                     (the outer one) isn't occupied and will give the error message
                     "Don't use phantom concept if you don't mean it." */

   phantest_ok,               // Anything goes
   phantest_impossible,       // Can't happen in symmetric stuff??
   phantest_both,             // Require both setups (partially) occupied
   phantest_only_one,         // Require only one setup occupied, don't care which
   phantest_only_first_one,   // Require only first setup occupied, second empty
   phantest_only_second_one,  // Require only second setup occupied, first empty
   phantest_only_one_pair,    // Require only first 2 (of 4) or only last 2
   phantest_first_or_both,    // Require first setup only, or a mixture
   phantest_ctr_phantom_line, // Special, created when outside phantom setup is empty.
   phantest_2x2_both,
   phantest_2x2_only_two,
   phantest_not_just_centers
} phantest_kind;

typedef enum {
   disttest_t, disttest_nil, disttest_only_two,
   disttest_any, disttest_offset, disttest_z} disttest_kind;

typedef enum {
   chk_none,
   chk_wave,
   chk_groups,
   chk_anti_groups,
   chk_box,
   chk_box_dbl,
   chk_indep_box,
   chk_dmd_qtag,
   chk_qtag,
   chk_qbox,
   chk_peelable,
   chk_spec_directions
} chk_type;


typedef enum {
   restriction_passes,
   restriction_fails,
   restriction_no_item
} restriction_test_result;


typedef struct milch {
   uint32 code;
   const map_thing *the_map;
   struct milch *next;
} mapcoder;

typedef struct {
   uint32 newheritmods;
   uint32 newfinalmods;
   int before;    /* These are indices into concept_descriptor_table. */
   int after;
} concept_fixer_thing;

typedef enum {
   /* Warning!!!!  Order is important!  See all the stupid ways these are used
      in sdconc.c . */
   selective_key_dyp,
   selective_key_own,
   selective_key_plain,
   selective_key_disc_dist,
   selective_key_ignore,
   selective_key_work_concept,
   selective_key_lead_for_a,
   selective_key_work_no_concentric,
   selective_key_snag_anyone,
   selective_key_plain_from_id_bits
} selective_key;

typedef enum {
   tandem_key_tand = 0,
   tandem_key_cpls = 1,
   tandem_key_siam = 2,
   tandem_key_tand3 = 4,
   tandem_key_cpls3 = 5,
   tandem_key_siam3 = 6,
   tandem_key_tand4 = 8,
   tandem_key_cpls4 = 9,
   tandem_key_siam4 = 10,
   tandem_key_box = 16,
   tandem_key_diamond = 17,
   tandem_key_skew = 18,
   tandem_key_outpoint_tgls = 20,
   tandem_key_inpoint_tgls = 21,
   tandem_key_inside_tgls = 22,
   tandem_key_outside_tgls = 23,
   tandem_key_wave_tgls = 26,
   tandem_key_tand_tgls = 27,
   tandem_key_anyone_tgls = 30,
   tandem_key_3x1tgls = 31,
   tandem_key_ys = 32
} tandem_key;

typedef enum {
   meta_key_random,
   meta_key_rev_random,   /* Must follow meta_key_random. */
   meta_key_piecewise,
   meta_key_initially,
   meta_key_finish,
   meta_key_revorder,
   meta_key_like_a,
   meta_key_finally,
   meta_key_nth_part_work,
   meta_key_skip_nth_part,
   meta_key_shift_n,
   meta_key_shifty,
   meta_key_echo,
   meta_key_rev_echo,   /* Must follow meta_key_echo. */
   meta_key_shift_half,
   meta_key_shift_n_half
} meta_key_kind;


/* VARIABLES */

extern parse_block *last_magic_diamond;                             /* in SDTOP */
extern uint32 global_tbonetest;                                     /* in SDCONCPT */
extern uint32 global_livemask;                                      /* in SDCONCPT */
extern uint32 global_selectmask;                                    /* in SDCONCPT */
extern uint32 global_tboneselect;                                   /* in SDCONCPT */
extern concept_fixer_thing concept_fixer_table[];                   /* in SDCTABLE */

extern int *concept_offset_tables[];                                /* in SDCTABLE */
extern int *concept_size_tables[];                                  /* in SDCTABLE */
extern Cstring concept_menu_strings[];                              /* in SDCTABLE */

extern id_bit_table id_bit_table_2x6_pg[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_bigdmd_wings[];                    /* in SDTABLES */
extern id_bit_table id_bit_table_bigbone_wings[];                   /* in SDTABLES */
extern id_bit_table id_bit_table_bighrgl_wings[];                   /* in SDTABLES */
extern id_bit_table id_bit_table_bigdhrgl_wings[];                  /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_offset[];                      /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_h[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_ctr6[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_butterfly[];                       /* in SDTABLES */
extern id_bit_table id_bit_table_525_nw[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_525_ne[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_343_outr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_343_innr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_545_outr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_545_innr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_3dmd_in_out[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_3dmd_ctr1x6[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_3dmd_ctr1x4[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_4dmd_cc_ee[];                      /* in SDTABLES */
extern id_bit_table id_bit_table_3ptpd[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_3x6_with_1x6[];                    /* in SDTABLES */
extern cm_thing conc_init_table[];                                  /* in SDTABLES */
extern const fixer fdhrgl;                                          /* in SDTABLES */
extern const fixer f323;                                            /* in SDTABLES */
extern const fixer f2x4far;                                         /* in SDTABLES */
extern const fixer f2x4near;                                        /* in SDTABLES */
extern const fixer f4dmdiden;                                       /* in SDTABLES */
extern const fixer fixmumble;                                       /* in SDTABLES */
extern const fixer fixfrotz;                                        /* in SDTABLES */
extern const fixer fixwhuzzis;                                      /* in SDTABLES */
extern const fixer fixgizmo;                                        /* in SDTABLES */


extern expand_thing comp_qtag_2x4_stuff;                            /* in SDTOP */
extern expand_thing exp_2x3_qtg_stuff;                              /* in SDTOP */
extern expand_thing exp_4x4_4x6_stuff_a;                            /* in SDTOP */
extern expand_thing exp_4x4_4x6_stuff_b;                            /* in SDTOP */
extern expand_thing exp_4x4_4dm_stuff_a;                            /* in SDTOP */
extern expand_thing exp_4x4_4dm_stuff_b;                            /* in SDTOP */
extern expand_thing exp_c1phan_4x4_stuff1;                          /* in SDTOP */
extern expand_thing exp_c1phan_4x4_stuff2;                          /* in SDTOP */

extern expand_thing exp_dmd_323_stuff;
extern expand_thing exp_1x2_dmd_stuff;
extern expand_thing exp_qtg_3x4_stuff;
extern expand_thing exp_1x2_hrgl_stuff;
extern expand_thing exp_dmd_hrgl_stuff;

extern full_expand_thing rear_1x2_pair;
extern full_expand_thing rear_2x2_pair;
extern full_expand_thing rear_bone_pair;
extern full_expand_thing step_8ch_pair;
extern full_expand_thing step_qtag_pair;
extern full_expand_thing step_2x2h_pair;
extern full_expand_thing step_2x2v_pair;
extern full_expand_thing step_spindle_pair;
extern full_expand_thing step_dmd_pair;
extern full_expand_thing step_qtgctr_pair;

extern full_expand_thing touch_init_table1[];
extern full_expand_thing touch_init_table2[];
extern full_expand_thing touch_init_table3[];
#define NEEDMASK(K) (1<<((K)/(CONCPROP__NEED_LOBIT)))
extern expand_thing expand_init_table[];

extern const coordrec tgl3_0;                                       /* in SDTABLES */
extern const coordrec tgl3_1;                                       /* in SDTABLES */
extern const coordrec tgl4_0;                                       /* in SDTABLES */
extern const coordrec tgl4_1;                                       /* in SDTABLES */
extern const coordrec squeezethingglass;                            /* in SDTABLES */
extern const coordrec squeezethinggal;                              /* in SDTABLES */
extern const coordrec squeezethingqtag;                             /* in SDTABLES */
extern const coordrec squeezething4dmd;                             /* in SDTABLES */
extern const coordrec squeezefinalglass;                            /* in SDTABLES */
extern const coordrec press_4dmd_4x4;                               /* in SDTABLES */
extern const coordrec press_4dmd_qtag1;                             /* in SDTABLES */
extern const coordrec press_4dmd_qtag2;                             /* in SDTABLES */
extern const coordrec press_qtag_4dmd1;                             /* in SDTABLES */
extern const coordrec press_qtag_4dmd2;                             /* in SDTABLES */
extern const coordrec acc_crosswave;                                /* in SDTABLES */
extern const tgl_map *c1tglmap1[];                                  /* in SDTABLES */
extern const tgl_map *c1tglmap2[];                                  /* in SDTABLES */
extern const tgl_map *dbqtglmap1[];                                 /* in SDTABLES */
extern const tgl_map *dbqtglmap2[];                                 /* in SDTABLES */
extern const tgl_map *qttglmap1[];                                  /* in SDTABLES */
extern const tgl_map *qttglmap2[];                                  /* in SDTABLES */
extern const tgl_map *bdtglmap1[];                                  /* in SDTABLES */
extern const tgl_map *bdtglmap2[];                                  /* in SDTABLES */
extern const tgl_map *rgtglmap1[];                                  /* in SDTABLES */
extern sel_item sel_init_table[];                                   /* in SDTABLES */
extern map_thing map_p8_tgl4;                                       /* in SDTABLES */
extern map_thing map_spndle_once_rem;                               /* in SDTABLES */
extern map_thing map_1x3dmd_once_rem;                               /* in SDTABLES */
extern map_thing map_lh_zzztgl;                                     /* in SDTABLES */
extern map_thing map_rh_zzztgl;                                     /* in SDTABLES */
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
extern map_thing map_4x4_ns;                                        /* in SDTABLES */
extern map_thing map_4x4_ew;                                        /* in SDTABLES */
extern map_thing map_vsplit_f;                                      /* in SDTABLES */
extern map_thing map_stairst;                                       /* in SDTABLES */
extern map_thing map_ladder;                                        /* in SDTABLES */
extern map_thing map_crazy_offset1;                                 /* in SDTABLES */
extern map_thing map_crazy_offset2;                                 /* in SDTABLES */
extern map_thing map_but_o;                                         /* in SDTABLES */
extern map_thing map_4x4v;                                          /* in SDTABLES */
extern map_thing map_blocks;                                        /* in SDTABLES */
extern map_thing map_trglbox;                                       /* in SDTABLES */
extern map_thing map_2x3_0134;                                      /* in SDTABLES */
extern map_thing map_2x3_1245;                                      /* in SDTABLES */
extern map_thing map_1x8_1x6;                                       /* in SDTABLES */
extern map_thing map_rig_1x6;                                       /* in SDTABLES */
extern map_thing map_3mdmd_3d;                                      /* in SDTABLES */
extern map_thing map_4x6_2x4;                                       /* in SDTABLES */
extern map_thing map_ov_hrg_1;                                      /* in SDTABLES */
extern map_thing map_ov_gal_1;                                      /* in SDTABLES */
extern map_thing map_3o_qtag_1;                                     /* in SDTABLES */
extern map_thing map_tgl4_1;                                        /* in SDTABLES */
extern map_thing map_tgl4_2;                                        /* in SDTABLES */
extern map_thing map_qtag_2x3;                                      /* in SDTABLES */
extern map_thing map_2x3_rmvr;                                      /* in SDTABLES */
extern map_thing map_2x3_rmvs;                                      /* in SDTABLES */
extern map_thing map_dbloff1;                                       /* in SDTABLES */
extern map_thing map_dbloff2;                                       /* in SDTABLES */
extern map_thing map_dhrgl1;                                        /* in SDTABLES */
extern map_thing map_dhrgl2;                                        /* in SDTABLES */
extern map_thing map_dbgbn1;                                        /* in SDTABLES */
extern map_thing map_dbgbn2;                                        /* in SDTABLES */
extern map_thing map_off1x81;                                       /* in SDTABLES */
extern map_thing map_off1x82;                                       /* in SDTABLES */
extern map_thing map_dqtag1;                                        /* in SDTABLES */
extern map_thing map_dqtag2;                                        /* in SDTABLES */
extern map_thing map_dqtag3;                                        /* in SDTABLES */
extern map_thing map_dqtag4;                                        /* in SDTABLES */
extern clw3_thing clw3_table[];                                     /* in SDTABLES */
extern map_thing map_trngl_box1;                                    /* in SDTABLES */
extern map_thing map_trngl_box2;                                    /* in SDTABLES */
extern map_thing map_inner_box;                                     /* in SDTABLES */
extern map_thing map_lh_c1phana;                                    /* in SDTABLES */
extern map_thing map_lh_c1phanb;                                    /* in SDTABLES */
extern map_thing map_lh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_lh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_rh_c1phana;                                    /* in SDTABLES */
extern map_thing map_rh_c1phanb;                                    /* in SDTABLES */
extern map_thing map_rh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_d1x10;                                         /* in SDTABLES */
extern map_thing map_lz12;                                          /* in SDTABLES */
extern map_thing map_rz12;                                          /* in SDTABLES */
extern map_thing map_dmd_1x1;                                       /* in SDTABLES */
extern map_thing map_star_1x1;                                      /* in SDTABLES */
extern map_thing map_qtag_f0;                                       /* in SDTABLES */
extern map_thing map_qtag_f1;                                       /* in SDTABLES */
extern map_thing map_qtag_f2;                                       /* in SDTABLES */
extern map_thing map_diag2a;                                        /* in SDTABLES */
extern map_thing map_diag2b;                                        /* in SDTABLES */
extern map_thing map_diag23a;                                       /* in SDTABLES */
extern map_thing map_diag23b;                                       /* in SDTABLES */
extern map_thing map_diag23c;                                       /* in SDTABLES */
extern map_thing map_diag23d;                                       /* in SDTABLES */
extern map_thing map_f2x8_4x4;                                      /* in SDTABLES */
extern map_thing map_w4x4_4x4;                                      /* in SDTABLES */
extern map_thing map_f2x8_2x8;                                      /* in SDTABLES */
extern map_thing map_w4x4_2x8;                                      /* in SDTABLES */
extern map_thing map_emergency1;                                    /* in SDTABLES */
extern map_thing map_emergency2;                                    /* in SDTABLES */
extern map_thing *maps_3diag[4];                                    /* in SDTABLES */
extern map_thing *maps_3diagwk[4];                                  /* in SDTABLES */
extern mapcoder map_init_table[];                                   /* in SDTABLES */
extern map_thing map_init_table2[];                                 /* in SDTABLES */
extern map_thing *split_lists[][6];                                 /* in SDTABLES */
extern warning_info no_search_warnings;                             /* in SDTOP */
extern warning_info conc_elong_warnings;                            /* in SDTOP */
extern warning_info dyp_each_warnings;                              /* in SDTOP */
extern warning_info useless_phan_clw_warnings;                      /* in SDTOP */




/* In SDPREDS */

extern long_boolean selectp(setup *ss, int place);

/* In SDGETOUT */

extern void write_resolve_text(long_boolean doing_file);
extern uims_reply full_resolve(void);
extern int concepts_in_place(void);
extern int reconcile_command_ok(void);
extern int resolve_command_ok(void);
extern int nice_setup_command_ok(void);
extern void initialize_getout_tables(void);

/* In SDBASIC */

extern void mirror_this(setup *s);

extern void fix_collision(
   uint32 explicit_mirror_flag,
   uint32 collision_mask,
   int collision_index,
   uint32 result_mask,
   int appears_illegal,
   long_boolean mirror,
   assumption_thing *assumption,
   setup *result);

extern void do_stability(uint32 *personp, stability stab, int turning);

extern long_boolean check_restriction(
   setup *ss,
   assumption_thing restr,
   long_boolean instantiate_phantoms,
   uint32 flags);

extern void basic_move(
   setup *ss,
   int tbonetest,
   long_boolean fudged,
   long_boolean mirror,
   setup *result);

/* In SDMOVES */

extern void reinstate_rotation(setup *ss, setup *result);

extern long_boolean divide_for_magic(
   setup *ss,
   uint32 heritflags_to_check,
   setup *result);

extern long_boolean do_simple_split(
   setup *ss,
   uint32 prefer_1x4,   /* 1 means prefer 1x4, 2 means this is 1x8 and do not recompute id. */
   setup *result);

extern void do_call_in_series(
   setup *sss,
   long_boolean dont_enforce_consistent_split,
   long_boolean roll_transparent,
   long_boolean normalize,
   long_boolean qtfudged);


extern void drag_someone_and_move(setup *ss, parse_block *parseptr, setup *result);

extern void anchor_someone_and_move(setup *ss, parse_block *parseptr, setup *result);

extern void process_number_insertion(uint32 mod_word);

extern int gcd(int a, int b);

extern uint32 process_new_fractions(
   int numer,
   int denom,
   uint32 incoming_fracs,
   uint32 reverse_orderbit,   /* Low bit on mean treat as if we mean "do the last M/N". */
   long_boolean allow_improper,
   long_boolean *improper_p);

extern void get_fraction_info(
   uint32 frac_flags,
   uint32 callflags1,
   int total,
   fraction_info *zzz);

extern long_boolean fill_active_phantoms_and_move(setup *ss, setup *result);

extern void move_perhaps_with_active_phantoms(setup *ss, setup *result);

extern void impose_assumption_and_move(setup *ss, setup *result);

extern void move(
   setup *ss,
   long_boolean qtfudged,
   setup *result);

/* In SDISTORT */

extern void prepare_for_call_in_series(setup *result, setup *ss);

extern void minimize_splitting_info(setup *ss, uint32 other_info);

extern void initialize_map_tables(void);

extern void remove_z_distortion(setup *ss);

extern void new_divided_setup_move(
   setup *ss,
   uint32 map_encoding,
   phantest_kind phancontrol,
   long_boolean recompute_id,
   setup *result);

extern void divided_setup_move(
   setup *ss,
   const map_thing *maps,
   phantest_kind phancontrol,
   long_boolean recompute_id,
   setup *result);

extern void new_overlapped_setup_move(setup *ss, uint32 map_encoding,
   uint32 *masks, setup *result);

extern void overlapped_setup_move(setup *ss, const map_thing *maps,
   uint32 *masks, setup *result);

extern void do_phantom_2x4_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void do_phantom_stag_qtg_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void do_phantom_diag_qtg_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void phantom_2x4_move(
   setup *ss,
   int lineflag,
   phantest_kind phantest,
   const map_thing *maps,
   setup *result);

extern void distorted_2x2s_move(
   setup *ss,
   parse_block *parseptr,
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

extern void common_spot_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result);

/* In SDCONCPT */

extern long_boolean do_big_concept(
   setup *ss,
   setup *result);

/* In SDTAND */

extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,           /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,          /* number, if doing fractional twosome/solid */
   int phantom,           /* normal=0 phantom=1 general-gruesome=2 gruesome-with-wave-check=3 */
   tandem_key key,
   uint32 mxn_bits,
   long_boolean phantom_pairing_ok,
   setup *result);

extern void initialize_tandem_tables(void);

/* In SDCONC */

extern void concentric_move(
   setup *ss,
   setup_command *cmdin,
   setup_command *cmdout,
   calldef_schema analyzer,
   uint32 modifiersin1,
   uint32 modifiersout1,
   long_boolean recompute_id,
   setup *result);

extern uint32 get_multiple_parallel_resultflags(setup outer_inners[], int number);

extern void initialize_conc_tables(void);
extern void initialize_sel_tables(void);

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

extern void punt_centers_use_concept(setup *ss, setup *result);

extern void selective_move(
   setup *ss,
   parse_block *parseptr,
   selective_key indicator,
   long_boolean others,
   int arg2,
   uint32 override_selector,
   selector_kind selector_to_use,
   long_boolean concentric_rules,
   setup *result);

extern void inner_selective_move(
   setup *ss,
   setup_command *cmd1,
   setup_command *cmd2,
   selective_key indicator,
   long_boolean others,
   int arg2,
   uint32 override_selector,
   selector_kind selector_to_use,
   uint32 modsa1,
   uint32 modsb1,
   setup *result);

/* In SDTOP */

extern void compress_setup(const expand_thing *thing, setup *stuff);

extern void expand_setup(const expand_thing *thing, setup *stuff);

extern void update_id_bits(setup *ss);

extern void touch_or_rear_back(
   setup *scopy,
   long_boolean did_mirror,
   int callflags1);

extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   long_boolean recompute_id);

NORETURN1 extern void fail2(const char s1[], const char s2[]) NORETURN2;
NORETURN1 extern void failp(uint32 id1, const char s[]) NORETURN2;
extern void warn(warning_index w);

extern restriction_test_result verify_restriction(
   setup *ss,
   assumption_thing tt,
   long_boolean instantiate_phantoms,
   long_boolean *failed_to_instantiate);

extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index);
extern void clear_people(setup *z);
extern uint32 rotcw(uint32 n);
extern uint32 rotccw(uint32 n);
extern void clear_person(setup *resultpeople, int resultplace);
extern uint32 copy_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace);
extern uint32 copy_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount);
extern void swap_people(setup *ss, int oneplace, int otherplace);
extern void install_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace);
extern void install_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount);
extern void scatter(setup *resultpeople, const setup *sourcepeople,
                    const veryshort *resultplace, int countminus1, int rotamount);
extern void gather(setup *resultpeople, const setup *sourcepeople,
                   const veryshort *resultplace, int countminus1, int rotamount);
extern parse_block *really_skip_one_concept(
   parse_block *incoming,
   concept_kind *k_p,
   uint32 *need_to_restrain_p,   /* 1=(if not doing echo), 2=(yes, always) */
   parse_block **parseptr_skip_p);
extern long_boolean fix_n_results(int arity, setup_kind goal, setup z[], uint32 *rotstatep);

extern void normalize_setup(setup *ss, normalize_action action);

#ifdef __cplusplus
}
#endif
