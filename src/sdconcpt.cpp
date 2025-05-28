// SD -- square dance caller's helper.
//
//    Copyright (C) 1990-2004  William B. Ackerman.
//
//    This file is part of "Sd".
//
//    Sd is free software; you can redistribute it and/or modify it
//    under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Sd is distributed in the hope that it will be useful, but WITHOUT
//    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
//    License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Sd; if not, write to the Free Software Foundation, Inc.,
//    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    This is for version 36.

/* This defines the following functions:
   do_big_concept
   stable_move

and the following external variables:
   global_tbonetest
   global_livemask
   global_selectmask
   global_tboneselect
   concept_table
*/

// For sprintf.
#include <stdio.h>

#include "sd.h"

#define WEIRD_BUT_DO_IT_ANYWAY

#define FRACS(code,n,k) (code|((n)*CMD_FRAC_PART_BIT)|((k)*CMD_FRAC_PART2_BIT))

static uint32 orig_tbonetest;
uint32 global_tbonetest;
uint32 global_livemask;
uint32 global_selectmask;
uint32 global_tboneselect;



static void do_concept_expand_some_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // We used to turn on the "FINAL__16_MATRIX" call modifier for 2x8 matrix,
   // but that makes tandem stuff not work (it doesn't like
   // call modifiers preceding it) and 4x4 stuff not work
   // (it wants the matrix expanded, but doesn't want you to say
   // "16 matrix").  So we need to let the CMD_MISC__EXPLICIT_MATRIX
   // bit control the desired effects.
   if (  ss->kind != ((setup_kind) parseptr->concept->arg1) &&
         // "16 matrix of parallel diamonds" needs to accept 4dmd or 4ptpd.
         (ss->kind != s4ptpd || ((setup_kind) parseptr->concept->arg1) != s4dmd))
      fail("Can't make the required matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, false, result);
}


// A remnant of some old stuff.
#define C_const
typedef struct {
   C_const int sizem1;
   C_const setup_kind ikind;
   C_const veryshort map1[8];
   C_const veryshort map2[8];
} phan_map;

static phan_map map_c1_phan   = {7, s2x4,   {0, 2, 7, 5, 8, 10, 15, 13},    {4, 6, 11, 9, 12, 14, 3, 1}};
static phan_map map_pinwheel1 = {7, s2x4,   {10, 15, -1, -1, 2, 7, -1, -1}, {14, 3, -1, -1, 6, 11, -1, -1}};
static phan_map map_pinwheel2 = {7, s2x4,   {-1, -1, 3, 1, -1, -1, 11, 9},  {-1, -1, 7, 5, -1, -1, 15, 13}};
#ifdef WEIRD_BUT_DO_IT_ANYWAY
static phan_map map_pinwheel3 = {7, s2x4,   {0, 1, -1, -1, 6, 7, -1, -1}, {2, 5, -1, -1, 8, 11, -1, -1}};
static phan_map map_pinwheel4 = {7, s2x4,   {-1, -1, 2, 3, -1, -1, 8, 9},  {-1, -1, 5, 7, -1, -1, 11, 1}};
#endif
static phan_map map_o_spots   = {7, s2x4,   {10, -1, -1, 1, 2, -1, -1, 9},  {14, -1, -1, 5, 6, -1, -1, 13}};
static phan_map map_qt_phan   = {7, s_qtag, {-1, -1, 2, 3, -1, -1, 6, 7},   {1, 4, -1, -1, 5, 0, -1, -1}};

static void do_concept_tandem(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // The "arg3" field of the concept descriptor contains bit fields as follows:
   // "100" bit:  this takes a selector
   // "F0" field: (fractional) twosome info --
   //    0=solid
   //    1=twosome
   //    2=solid-frac-twosome
   //    3=twosome-frac-solid
   // "0F" field:
   //    0=normal
   //    2=plain-gruesome
   //    3=gruesome-with-wave-assumption

   if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_TWISTED))
      fail("Improper concept order.");

   // Look for things like "tandem in a 1/4 tag".
   if (parseptr->next && parseptr->next->concept->kind == concept_tandem_in_setup) {
      tandem_key master_key = (tandem_key) parseptr->concept->arg4;

      // Demand that this not be "gruesome" or "anyone are tandem" or "skew" or
      // "triangles are solid" or whatever.

      if ((master_key != tandem_key_cpls && master_key != tandem_key_tand) ||
          (parseptr->concept->arg3 & ~0x0F0) != 0)
         fail("Can do this only with \"as couples\" or \"tandem\".");

      // Find out how the matrix is to be expanded.
      uint32 orig_bits = parseptr->next->concept->arg2;
      if (master_key == tandem_key_tand) {
         switch (orig_bits & CONCPROP__NEED_MASK) {
         case CONCPROP__NEEDK_4DMD:
            orig_bits ^= CONCPROP__NEEDK_TWINQTAG ^ CONCPROP__NEEDK_4DMD;
            break;
         case CONCPROP__NEEDK_DBLX:
            orig_bits ^= CONCPROP__NEEDK_2X8 ^ CONCPROP__NEEDK_DBLX;
            break;
         case CONCPROP__NEEDK_DEEPXWV:
            orig_bits ^= CONCPROP__NEEDK_2X6 ^ CONCPROP__NEEDK_DEEPXWV;
            break;
         case CONCPROP__NEEDK_1X16:
            orig_bits ^= CONCPROP__NEEDK_2X8 ^ CONCPROP__NEEDK_1X16;
            break;
         case CONCPROP__NEEDK_2X8:
            orig_bits ^= CONCPROP__NEEDK_1X16 ^ CONCPROP__NEEDK_2X8;
            break;
         }
      }

      if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX))
         do_matrix_expansion(ss, orig_bits, false);

      // Put in the "VERIFY" bits stating just what type of setup to expect.
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS | parseptr->next->concept->arg1;

      // Skip the "in a whatever" concept.
      ss->cmd.parseptr = parseptr->next->next;
   }

   // The table said this concept was matrix-oblivious.  Now that we have
   // expanded for "in a whatever", we have to take action.

   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

   uint32 mxnflags = ss->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_SINGLE |
                                                            INHERITFLAG_MXNMASK |
                                                            INHERITFLAG_NXNMASK);

   if (parseptr->concept->arg2 == CONCPROP__NEEDK_4DMD ||
       parseptr->concept->arg2 == CONCPROP__NEEDK_4D_4PTPD ||
       parseptr->concept->arg2 == CONCPROP__NEEDK_TWINQTAG)
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

   ss->cmd.cmd_misc_flags |= parseptr->concept->arg1;

   ss->cmd.cmd_final_flags.clear_heritbits(INHERITFLAG_SINGLE |
                                           INHERITFLAG_MXNMASK |
                                           INHERITFLAG_NXNMASK);

   tandem_couples_move(
     ss,
     (parseptr->concept->arg3 & 0x100) ? parseptr->options.who : selector_uninitialized,
     (parseptr->concept->arg3 & 0xF0) >> 4, // (fractional) twosome info
     parseptr->options.number_fields,
     parseptr->concept->arg3 & 0xF,         // normal/phantom/gruesome etc.
     (tandem_key) parseptr->concept->arg4,  // key
     mxnflags,
     false,
     result);
}


static void do_c1_phantom_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   parse_block *next_parseptr;
   final_and_herit_flags junk_concepts;
   setup setup1, setup2;
   setup the_setups[2];
   phan_map *map_ptr = (phan_map *) 0;

   // See if this is a "phantom tandem" (or whatever) by searching ahead,
   // skipping comments of course.  This means we must skip modifiers too,
   // so we check that there weren't any.

   junk_concepts.clear_all_herit_and_final_bits();
   next_parseptr = process_final_concepts(parseptr->next, false, &junk_concepts, true, __FILE__, __LINE__);

   if (next_parseptr->concept->kind == concept_tandem ||
       next_parseptr->concept->kind == concept_frac_tandem) {

      // Find out what kind of tandem call this is.

      uint32 what_we_need = 0;
      uint32 mxnflags = ss->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_SINGLE |
                                                               INHERITFLAG_MXNMASK |
                                                               INHERITFLAG_NXNMASK);

      if (junk_concepts.test_herit_and_final_bits())
         fail("Phantom couples/tandem must not have intervening concepts.");

      // "Phantom tandem" has a higher level than either "phantom" or "tandem".
      if (phantom_tandem_level > calling_level) warn(warn__bad_concept_level);

      switch (next_parseptr->concept->arg4) {
      case tandem_key_siam:
      case tandem_key_skew:
         fail("Phantom not allowed with skew or siamese.");
      case tandem_key_box:
         // We do not expand the matrix.  The caller must say
         // "2x8 matrix", or whatever, to get that effect.
         break;
      case tandem_key_diamond:
         // We do not expand the matrix.  The caller must say
         // "16 matrix or parallel diamonds", or whatever, to get that effect.
         break;
      case tandem_key_tand3:
      case tandem_key_cpls3:
      case tandem_key_siam3:
      case tandem_key_tand4:
      case tandem_key_cpls4:
      case tandem_key_siam4:
         // We do not expand the matrix.  The caller must say
         // "2x8 matrix", or whatever, to get that effect.
         break;
      default:
         // This is plain "phantom tandem", or whatever.  Expand to whatever is in
         // the "arg2" field, or to a 4x4.  The "arg2" check allows the user to say
         // "phantom as couples in a 1/4 tag".  "as couples in a 1/4 tag"
         // would have worked also.
         // But we don't do this if stuff like "1x3" came in.

         if (!mxnflags) {
            what_we_need = next_parseptr->concept->arg2;
            if (what_we_need == 0) what_we_need = CONCPROP__NEEDK_4X4;
         }

         break;
      }

      // Look for things like "tandem in a 1/4 tag".  If so, skip the expansion stuff.

      if (next_parseptr->next && next_parseptr->next->concept->kind == concept_tandem_in_setup) {
         // No matrix expand.
      }
      else {
         if (what_we_need != 0)
            do_matrix_expansion(ss, what_we_need, true);
         ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;
      }

      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;
      ss->cmd.parseptr = next_parseptr->next;
      do_concept_tandem(ss, next_parseptr, result);
      return;
   }

   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;   // We didn't do this before.
   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;    // Or this.

   if (ss->kind == s4x4 && global_livemask == 0x6666) {
      // First, check for everyone on "O" spots.  If so, treat them as though
      // in equivalent C1 phantom spots.
      map_ptr = &map_o_spots;
   }
   else if (ss->kind == s_c1phan)
      // This is a vanilla C1 phantom setup.
      map_ptr = &map_c1_phan;
   else if (ss->kind == s_bone)
      // We allow "phantom" in a bone setup to mean two "dunlap" quarter tags. 
      map_ptr = &map_qt_phan;
#ifdef WEIRD_BUT_DO_IT_ANYWAY
   else if (ss->kind == s3x4) {

      // Check for a 3x4 occupied as a distorted "pinwheel", and treat it as phantoms.

      if (global_livemask == 04747)
         map_ptr = &map_pinwheel3;
      else if (global_livemask == 05656)
         map_ptr = &map_pinwheel4;
   }
#endif
   else if (ss->kind == s4x4) {
      setup temp;

      // Check for a 4x4 occupied as a "pinwheel", and treat it as phantoms.

      if (global_livemask == 0xCCCC) {
         map_ptr = &map_pinwheel1;
         goto use_map;
      }
      else if (global_livemask == 0xAAAA) {
         map_ptr = &map_pinwheel2;
         goto use_map;
      }

      // Next, check for a "phantom turn and deal" sort of thing from stairsteps.
      // Do the call in each line, them remove resulting phantoms carefully.

      if (global_livemask == 0x5C5C || global_livemask == 0xA3A3) {
         // Split into 4 vertical strips.
         divided_setup_move(ss, spcmap_4x4v, phantest_ok, true, result);
      }
      else if (global_livemask == 0xC5C5 || global_livemask == 0x3A3A) {
         // Split into 4 horizontal strips.
         divided_setup_move(ss, MAPCODE(s1x4,4,MPKIND__SPLIT,1), phantest_ok, true, result);
      }
      else
         fail("Inappropriate setup for phantom concept.");

      if (result->kind != s2x8)
         fail("This call is not appropriate for use with phantom concept.");

      temp = *result;
      clear_people(result);
      
      if (!(temp.people[0].id1 | temp.people[7].id1 | temp.people[8].id1 | temp.people[15].id1)) {
         (void) copy_person(result, 0, &temp, 1);
         (void) copy_person(result, 3, &temp, 6);
         (void) copy_person(result, 4, &temp, 9);
         (void) copy_person(result, 7, &temp, 14);
      }
      else if (!(temp.people[1].id1 | temp.people[6].id1 | temp.people[9].id1 | temp.people[14].id1)) {
         (void) copy_person(result, 0, &temp, 0);
         (void) copy_person(result, 3, &temp, 7);
         (void) copy_person(result, 4, &temp, 8);
         (void) copy_person(result, 7, &temp, 15);
      }
      else
         fail("This call is not appropriate for use with phantom concept.");

      if (!(temp.people[2].id1 | temp.people[5].id1 | temp.people[10].id1 | temp.people[13].id1)) {
         (void) copy_person(result, 1, &temp, 3);
         (void) copy_person(result, 2, &temp, 4);
         (void) copy_person(result, 5, &temp, 11);
         (void) copy_person(result, 6, &temp, 12);
      }
      else if (!(temp.people[3].id1 | temp.people[4].id1 | temp.people[11].id1 | temp.people[12].id1)) {
         (void) copy_person(result, 1, &temp, 2);
         (void) copy_person(result, 2, &temp, 5);
         (void) copy_person(result, 5, &temp, 10);
         (void) copy_person(result, 6, &temp, 13);
      }
      else
         fail("This call is not appropriate for use with phantom concept.");

      result->kind = s2x4;
      return;
   }

   use_map:

   if (!map_ptr)
      fail("Inappropriate setup for phantom concept.");

   setup1 = *ss;
   setup2 = *ss;
   
   setup1.kind = map_ptr->ikind;
   setup2.kind = map_ptr->ikind;
   setup1.rotation = ss->rotation;
   setup2.rotation = ss->rotation+1;
   clear_people(&setup1);
   clear_people(&setup2);

   gather(&setup1, ss, map_ptr->map1, map_ptr->sizem1, 0);
   gather(&setup2, ss, map_ptr->map2, map_ptr->sizem1, 033);
   
   normalize_setup(&setup1, simple_normalize, false);
   normalize_setup(&setup2, simple_normalize, false);

   setup1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   setup2.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   move(&setup1, false, &the_setups[0]);
   move(&setup2, false, &the_setups[1]);

   *result = the_setups[1];
   result->result_flags = get_multiple_parallel_resultflags(the_setups, 2);
   merge_setups(&the_setups[0], merge_c1_phantom_real, result);
}



static void do_concept_single_diagonal(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   if (ss->kind != s4x4) fail("Need a 4x4 for this.");

   if (global_livemask != 0x2D2D && global_livemask != 0xD2D2)
      fail("People must be in blocks -- try specifying the people who should do the call.");

   selective_move(ss, parseptr, selective_key_disc_dist, 0,
                  parseptr->concept->arg1,
                  global_livemask & 0x9999, selector_uninitialized, false, result);
}


static void do_concept_double_diagonal(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 tbonetest;
   uint32 map_code;

   if (parseptr->concept->arg2 == 2) {

      // This is "distorted CLW of 6".

      setup ssave = *ss;

      if (ss->kind != sbighrgl) global_livemask = 0;   // Force error.

      if (global_livemask == 0x3CF) { map_code = spcmap_dhrgl1; }
      else if (global_livemask == 0xF3C) { map_code = spcmap_dhrgl2; }
      else fail("Can't find distorted 1x6.");

      if (parseptr->concept->arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;
      else if (parseptr->concept->arg1 == 1)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES;
      else
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS;

      divided_setup_move(ss, map_code, phantest_ok, true, result);

      // I wish this weren't so sleazy, but a new concentricity schema seems excessive.

      if (result->kind != sbighrgl) fail("Can't figure out result setup.");

      (void) copy_person(result, 2, &ssave, 2);
      (void) copy_person(result, 8, &ssave, 8);
   }
   else if (parseptr->concept->arg2) {

      // This is "diagonal CLW's of 3".

      setup ssave = *ss;
      int switcher = (parseptr->concept->arg1 ^ global_tbonetest) & 1;

      if (ss->kind != s4x4 || (global_tbonetest & 011) == 011) global_livemask = 0;   // Force error.

      if (     global_livemask == 0x2D2D)
         map_code = switcher ? spcmap_diag23a : spcmap_diag23b;
      else if (global_livemask == 0xD2D2)
         map_code = switcher ? spcmap_diag23c : spcmap_diag23d;
      else
         fail("There are no diagonal lines or columns of 3 here.");

      if (parseptr->concept->arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      divided_setup_move(ss, map_code, phantest_ok, true, result);

      // I wish this weren't so sleazy, but a new concentricity schema seems excessive.

      if (result->kind != s4x4) fail("Can't figure out result setup.");

      (void) copy_person(result, 0, &ssave, 0);
      (void) copy_person(result, 4, &ssave, 4);
      (void) copy_person(result, 8, &ssave, 8);
      (void) copy_person(result, 12, &ssave, 12);
   }
   else {
      tbonetest = global_tbonetest;

      if (     global_livemask == 0x2A82A8) map_code = spcmap_diag2a;
      else if (global_livemask == 0x505505) map_code = spcmap_diag2b;
      else
         tbonetest = ~0UL;   // Force error.

      if (ss->kind != s4x6) tbonetest = ~0UL;   // Force error.

      if (parseptr->concept->arg1 & 1) {
         if (tbonetest & 010) fail("There are no diagonal lines here.");
      }
      else {
         if (tbonetest & 1) fail("There are no diagonal columns here.");
      }

      if (parseptr->concept->arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      divided_setup_move(ss, map_code, phantest_ok, true, result);
   }
}


static void do_concept_double_offset(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   if (ss->kind != s2x4) fail("Must have a 2x4 setup to do this concept.");

   uint32 topmask, botmask, ctrmask;
   uint32 directions, livemask, map_code;
   get_directions(ss, directions, livemask);  // Get big-endian bit-pair masks.

   if (global_selectmask == (global_livemask & 0xCC)) {
      topmask = 0xF000 & livemask;
      botmask = 0x00F0 & livemask;
      ctrmask = 0x0F0F & livemask;
      map_code = spcmap_dbloff1;
   }
   else if (global_selectmask == (global_livemask & 0x33)) {
      topmask = 0x0F00 & livemask;
      botmask = 0x000F & livemask;
      ctrmask = 0xF0F0 & livemask;
      map_code = spcmap_dbloff2;
   }
   else
      fail("The designated centers are improperly placed.");

   // Check that the concept is correctly named.

   uint32 errmask = 0;

   switch (parseptr->concept->arg1) {
   case 0:
      // Double-offset quarter tag.
      errmask = (directions & ctrmask & 0x5555) |
         ((directions ^ 0xAAAA) & topmask) |
         (directions & botmask);
      break;
   case 1:
      // Double-offset three-quarter tag.
      errmask = (directions & ctrmask & 0x5555) |
         ((directions ^ 0xAAAA) & botmask) |
         (directions & topmask);
      break;
   case 2:
      // Double-offset general quarter tag.
      errmask = directions & livemask & 0x5555;
      break;
   case 3:
      // Double-offset diamonds.
      errmask = (~directions & (topmask | botmask) & 0x5555) |
         (directions & ctrmask & 0x5555);
      break;
   default:
      // Case 4 -- double-offset diamond spots -- anything goes.
      break;
   }

   if (errmask != 0)
      fail("Facing directions are incorrect for this concept.");

   divided_setup_move(ss, map_code, phantest_ok, true, result);
}


static void do_4x4_quad_working(setup *ss, int cstuff, setup *result) THROW_DECL
{
   uint32 masks[8];

   canonicalize_rotation(ss);

   // Initially assign the centers to the upper (masks[1] or masks[2]) group.
   masks[0] = 0xF0; masks[1] = 0xF0; masks[2] = 0xFF;

   // Look at the center 8 people and put each one in the correct group.

   if (cstuff == 8) {
      masks[0] = 0xFC; masks[1] = 0xCC; masks[2] = 0xCF;   // clockwise
   }
   else if (cstuff == 9) {
      masks[0] = 0xF3; masks[1] = 0x33; masks[2] = 0x3F;   // counterclockwise
   }
   else {                        // forward/back/left/right
      if ((ss->people[10].id1 ^ cstuff) & 2) { masks[1] |= 0x01; masks[2] &= ~0x80; };
      if ((ss->people[15].id1 ^ cstuff) & 2) { masks[1] |= 0x02; masks[2] &= ~0x40; };
      if ((ss->people[3].id1  ^ cstuff) & 2) { masks[1] |= 0x04; masks[2] &= ~0x20; };
      if ((ss->people[1].id1  ^ cstuff) & 2) { masks[1] |= 0x08; masks[2] &= ~0x10; };
      if ((ss->people[9].id1  ^ cstuff) & 2) { masks[0] |= 0x01; masks[1] &= ~0x80; };
      if ((ss->people[11].id1 ^ cstuff) & 2) { masks[0] |= 0x02; masks[1] &= ~0x40; };
      if ((ss->people[7].id1  ^ cstuff) & 2) { masks[0] |= 0x04; masks[1] &= ~0x20; };
      if ((ss->people[2].id1  ^ cstuff) & 2) { masks[0] |= 0x08; masks[1] &= ~0x10; };
   }

   overlapped_setup_move(ss, MAPCODE(s2x4,3,MPKIND__OVERLAP,1), masks, result);
}




static void do_concept_multiple_lines_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // This can only be standard for
   // together/apart/clockwise/counterclockwise/toward-the-center,
   // not for forward/back/left/right, because we look at
   // individual facing directions to determine which other
   // line/column the people in the center lines/columns must
   // work in.

   int rotfix = 0;
   setup_kind base_setup = s2x4;
   int base_vert = 1;
   uint32 masks[8];

   // Arg4 = number of C/L/W.

   int cstuff = parseptr->concept->arg1;
   // cstuff =
   // forward (lines) or left (cols)     : 0
   // backward (lines) or right (cols)   : 2
   // clockwise                          : 8
   // counterclockwise                   : 9
   // together (must be end-to-end)      : 10
   // apart (must be end-to-end)         : 11
   // toward the center (quadruple only) : 12

   int linesp = parseptr->concept->arg3;

   // If this was multiple columns, we allow stepping to a wave.  This makes it
   // possible to do interesting cases of turn and weave, when one column
   // is a single 8 chain and another is a single DPT.  But if it was multiple
   // lines, we forbid it.

   if (linesp & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (linesp == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (parseptr->concept->arg4 == 3) {
      // Triple C/L/W working.

      if (cstuff >= 10) {
         if (ss->kind != s1x12) fail("Must have a 1x12 setup for this concept.");

         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
 
         if (linesp & 1) {
            if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         }
         else {
            if (!(global_tbonetest & 1)) fail("There are no columns of 4 here.");
         }
    
         if (cstuff == 10) {     // Working together.
            masks[0] = 0xCF; masks[1] = 0xFC;
         }
         else {                 // Working apart.
            masks[0] = 0x3F; masks[1] = 0xF3;
         }

         base_setup = s1x8;
         base_vert = 0;
      }
      else {
         int i, tbonetest;
 
         if (ss->kind != s3x4) fail("Must have a 3x4 setup for this concept.");
    
         if (cstuff >= 8)
            tbonetest = global_tbonetest;
         else {
            tbonetest = 0;
            for (i=0; i<12; i++) tbonetest |= ss->people[i].id1;
         }
    
         if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
    
         if (linesp & 1) {
            if (tbonetest & 1) fail("There are no lines of 4 here.");
         }
         else {
            if (!(tbonetest & 1)) fail("There are no columns of 4 here.");
         }
    
         // Initially assign the centers to the upper (masks[1]) group.
         masks[0] = 0xF0; masks[1] = 0xFF;
    
         // Look at the center line people and put each one in the correct group.
    
         if (cstuff == 8) {
            masks[0] = 0xFC; masks[1] = 0xCF;   // clockwise
         }
         else if (cstuff == 9) {
            masks[0] = 0xF3; masks[1] = 0x3F;   // counterclockwise
         }
         else {                        // forward/back/left/right
            if ((ss->people[10].id1 ^ cstuff) & 2) { masks[1] &= ~0x80 ; masks[0] |= 0x1; };
            if ((ss->people[11].id1 ^ cstuff) & 2) { masks[1] &= ~0x40 ; masks[0] |= 0x2; };
            if ((ss->people[5].id1  ^ cstuff) & 2) { masks[1] &= ~0x20 ; masks[0] |= 0x4; };
            if ((ss->people[4].id1  ^ cstuff) & 2) { masks[1] &= ~0x10 ; masks[0] |= 0x8; };
         }
      }
   }
   else if (parseptr->concept->arg4 == 4) {
      // Quadruple C/L/W working.

      if (cstuff >= 12) {
         if (ss->kind != s4x4) fail("Must have a 4x4 setup to do this concept.");

         if ((global_tbonetest & 011) == 011) fail("Sorry, can't do this from T-bone setup.");
         rotfix = (global_tbonetest ^ linesp ^ 1) & 1;
         ss->rotation += rotfix;   // Just flip the setup around and recanonicalize.
         canonicalize_rotation(ss);

         masks[0] = 0xF0; masks[1] = 0xFF; masks[2] = 0x0F;
      }
      else if (cstuff >= 10) {
         if (ss->kind != s1x16) fail("Must have a 1x16 setup for this concept.");

         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (linesp & 1) {
            if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         }
         else {
            if (!(global_tbonetest & 1)) fail("There are no columns of 4 here.");
         }

         if (cstuff == 10) {    // Working together end-to-end.
            masks[0] = 0xCF; masks[1] = 0xCC; masks[2] = 0xFC;
         }
         else {                 // Working apart end-to-end.
            masks[0] = 0x3F; masks[1] = 0x33; masks[2] = 0xF3;
         }

         base_setup = s1x8;
         base_vert = 0;
      }
      else {
         int tbonetest;
         setup hpeople = *ss;
         setup vpeople = *ss;

         if (ss->kind != s4x4) fail("Must have a 4x4 setup to do this concept.");

         if (cstuff >= 8) {
            // Clockwise/counterclockwise can use "standard".
            tbonetest = global_tbonetest;
            if ((tbonetest & 011) == 011)  // But we can't have the standard people inconsistent.
               fail("Standard people are inconsistent.");
         }
         else {
            // Others can't use "standard", but can have multiple setups T-bioned to each other.
            tbonetest = 0;
            for (int i=0; i<16; i++) {
               uint32 person = ss->people[i].id1;
               tbonetest |= person;
               if (person & 1)
                  clear_person(&vpeople, i);
               else
                  clear_person(&hpeople, i);
            }
         }

         if ((tbonetest & 011) == 011) {
            setup the_setups[2];

            // We now have nonempty setups in both hpeople and vpeople.

            rotfix = (linesp ^ 1) & 1;
            vpeople.rotation += rotfix;
            do_4x4_quad_working(&vpeople, cstuff, &the_setups[0]);
            the_setups[0].rotation -= rotfix;   // Flip the setup back.

            rotfix = (linesp) & 1;
            hpeople.rotation += rotfix;
            do_4x4_quad_working(&hpeople, cstuff, &the_setups[1]);
            the_setups[1].rotation -= rotfix;   // Flip the setup back.

            *result = the_setups[1];
            result->result_flags = get_multiple_parallel_resultflags(the_setups, 2);
            merge_setups(&the_setups[0], merge_strict_matrix, result);
            warn(warn__tbonephantom);
         }
         else {
            rotfix = (tbonetest ^ linesp ^ 1) & 1;
            ss->rotation += rotfix;
            do_4x4_quad_working(ss, cstuff, result);
            result->rotation -= rotfix;   // Flip the setup back.
         }

         return;
      }
   }
   else if (parseptr->concept->arg4 == 5) {
      // Quintuple C/L/W working.

      int i, tbonetest;

      if (ss->kind != s4x5) fail("Must have a 4x5 setup for this concept.");
 
      if (cstuff >= 8)
         tbonetest = global_tbonetest;
      else {
         tbonetest = 0;
         for (i=0; i<20; i++) tbonetest |= ss->people[i].id1;
      }
 
      if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
 
      if (linesp & 1) {
         if (!(tbonetest & 1)) fail("There are no lines of 4 here.");
      }
      else {
         if (tbonetest & 1) fail("There are no columns of 4 here.");
      }
 
      // Initially assign the center 3 lines to the right (masks[1]/masks[2]/masks[3]) group.
      masks[0] = 0xF0; masks[1] = 0xF0; masks[2] = 0xF0; masks[3] = 0xFF;
 
      // Look at the center 3 lines of people and put each one in the correct group.
 
      if (cstuff == 8) {
         // clockwise
         masks[0] = 0xFC; masks[1] = 0xCC; masks[2] = 0xCC; masks[3] = 0xCF;
      }
      else if (cstuff == 9) {
         // counterclockwise
         masks[0] = 0xF3; masks[1] = 0x33; masks[2] = 0x33; masks[3] = 0x3F;
      }
      else {                        // forward/back/left/right
         if ((ss->people[3].id1  + 3 + cstuff) & 2) { masks[2] |= 0x01; masks[3] &= ~0x80; };
         if ((ss->people[6].id1  + 3 + cstuff) & 2) { masks[2] |= 0x02; masks[3] &= ~0x40; };
         if ((ss->people[18].id1 + 3 + cstuff) & 2) { masks[2] |= 0x04; masks[3] &= ~0x20; };
         if ((ss->people[11].id1 + 3 + cstuff) & 2) { masks[2] |= 0x08; masks[3] &= ~0x10; };
         if ((ss->people[2].id1  + 3 + cstuff) & 2) { masks[1] |= 0x01; masks[2] &= ~0x80; };
         if ((ss->people[7].id1  + 3 + cstuff) & 2) { masks[1] |= 0x02; masks[2] &= ~0x40; };
         if ((ss->people[17].id1 + 3 + cstuff) & 2) { masks[1] |= 0x04; masks[2] &= ~0x20; };
         if ((ss->people[12].id1 + 3 + cstuff) & 2) { masks[1] |= 0x08; masks[2] &= ~0x10; };
         if ((ss->people[1].id1  + 3 + cstuff) & 2) { masks[0] |= 0x01; masks[1] &= ~0x80; };
         if ((ss->people[8].id1  + 3 + cstuff) & 2) { masks[0] |= 0x02; masks[1] &= ~0x40; };
         if ((ss->people[16].id1 + 3 + cstuff) & 2) { masks[0] |= 0x04; masks[1] &= ~0x20; };
         if ((ss->people[13].id1 + 3 + cstuff) & 2) { masks[0] |= 0x08; masks[1] &= ~0x10; };
      }
   }
   else {
      // Sextuple C/L/W working.

      int i;
      uint32 tbonetest = global_tbonetest;

      // Expanding to a 4x6 is tricky.  See the extensive comments in the
      // function "triple_twin_move" in sdistort.c .

      if (cstuff < 8) {
         tbonetest = 0;
         for (i=0; i<24; i++) tbonetest |= ss->people[i].id1;
      }

      if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      tbonetest ^= linesp;

      if (ss->kind == s4x4) {
         expand::expand_setup(
            ((tbonetest & 1) ? &s_4x4_4x6b : &s_4x4_4x6a), ss);
         tbonetest = 0;
      }

      if (ss->kind != s4x6) fail("Must have a 4x6 setup for this concept.");

      if (tbonetest & 1) {
         if (linesp) fail("Can't find the required lines.");
         else fail("Can't find the required columns.");
      }
 
      // Initially assign the center 3 lines to the right
      // (masks[1]/masks[2]/masks[3]/masks[4]) group.
      masks[0] = 0xF0; masks[1] = 0xF0; masks[2] = 0xF0; masks[3] = 0xF0; masks[4] = 0xFF;
 
      // Look at the center 4 lines of people and put each one in the correct group.
 
      if (cstuff == 8) {
         // clockwise
         masks[0] = 0xFC; masks[1] = 0xCC; masks[2] = 0xCC; masks[3] = 0xCC; masks[4] = 0xCF;
      }
      else if (cstuff == 9) {
         // counterclockwise
         masks[0] = 0xF3; masks[1] = 0x33; masks[2] = 0x33; masks[3] = 0x33; masks[4] = 0x3F;
      }
      else {
         // forward/back/left/right
         if ((ss->people[ 4].id1 + 3 + cstuff) & 2) { masks[3] |= 0x01; masks[4] &= ~0x80; };
         if ((ss->people[ 7].id1 + 3 + cstuff) & 2) { masks[3] |= 0x02; masks[4] &= ~0x40; };
         if ((ss->people[22].id1 + 3 + cstuff) & 2) { masks[3] |= 0x04; masks[4] &= ~0x20; };
         if ((ss->people[13].id1 + 3 + cstuff) & 2) { masks[3] |= 0x08; masks[4] &= ~0x10; };
         if ((ss->people[ 3].id1 + 3 + cstuff) & 2) { masks[2] |= 0x01; masks[3] &= ~0x80; };
         if ((ss->people[ 8].id1 + 3 + cstuff) & 2) { masks[2] |= 0x02; masks[3] &= ~0x40; };
         if ((ss->people[21].id1 + 3 + cstuff) & 2) { masks[2] |= 0x04; masks[3] &= ~0x20; };
         if ((ss->people[14].id1 + 3 + cstuff) & 2) { masks[2] |= 0x08; masks[3] &= ~0x10; };
         if ((ss->people[ 2].id1 + 3 + cstuff) & 2) { masks[1] |= 0x01; masks[2] &= ~0x80; };
         if ((ss->people[ 9].id1 + 3 + cstuff) & 2) { masks[1] |= 0x02; masks[2] &= ~0x40; };
         if ((ss->people[20].id1 + 3 + cstuff) & 2) { masks[1] |= 0x04; masks[2] &= ~0x20; };
         if ((ss->people[15].id1 + 3 + cstuff) & 2) { masks[1] |= 0x08; masks[2] &= ~0x10; };
         if ((ss->people[ 1].id1 + 3 + cstuff) & 2) { masks[0] |= 0x01; masks[1] &= ~0x80; };
         if ((ss->people[10].id1 + 3 + cstuff) & 2) { masks[0] |= 0x02; masks[1] &= ~0x40; };
         if ((ss->people[19].id1 + 3 + cstuff) & 2) { masks[0] |= 0x04; masks[1] &= ~0x20; };
         if ((ss->people[16].id1 + 3 + cstuff) & 2) { masks[0] |= 0x08; masks[1] &= ~0x10; };
      }
   }

   overlapped_setup_move(ss, MAPCODE(base_setup,parseptr->concept->arg4-1,MPKIND__OVERLAP,base_vert),
      masks, result);
   result->rotation -= rotfix;   // Flip the setup back.
}


static uint32 get_standard_people(setup *ss, selector_kind who,
                                  uint32 & tbonetest, uint32 & stdtest)
{
   int i, j;
   tbonetest = 0;
   stdtest = 0;
   uint32 livemask = 0;
   selector_kind saved_selector = current_options.who;
   
   current_options.who = who;
   
   for (i=0, j=1; i<=attr::slimit(ss); i++, j<<=1) {
      int p = ss->people[i].id1;
      tbonetest |= p;
      if (p) {
         livemask |= j;
         if (selectp(ss, i)) stdtest |= p;
      }
   }
   
   current_options.who = saved_selector;
   return livemask;
}


static void do_concept_parallelogram(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // First, deal with "parallelogram diamonds".

   if (parseptr->concept->arg1) {
      uint32 map_code;

      if (ss->kind == spgdmdcw)
         map_code = MAPCODE(s_qtag,1,MPKIND__OFFS_R_HALF,1);
      else if (ss->kind == spgdmdccw)
         map_code = MAPCODE(s_qtag,1,MPKIND__OFFS_L_HALF,1);
      else
         fail("Can't find parallelogram diamonds.");

      divided_setup_move(ss, map_code, phantest_ok, true, result);

      // The split-axis bits are gone.  If someone needs them, we have work to do.
      result->result_flags.clear_split_info();
      return;
   }

   // See if it is followed by "split phantom C/L/W" or "split phantom boxes",
   // in which case we do something esoteric.

   const parse_block *next_parseptr;
   final_and_herit_flags junk_concepts;
   junk_concepts.clear_all_herit_and_final_bits();

   next_parseptr = process_final_concepts(parseptr->next, false, &junk_concepts,
                                          true, __FILE__, __LINE__);

   const parse_block *standard_concept = (parse_block *) 0;

   // But skip over "standard"
   if (next_parseptr->concept->kind == concept_standard &&
       junk_concepts.test_herit_and_final_bits() == 0) {
      standard_concept = next_parseptr;
      junk_concepts.clear_all_herit_and_final_bits();
      next_parseptr = process_final_concepts(next_parseptr->next, false, &junk_concepts,
                                             true, __FILE__, __LINE__);
   }

   // We are only interested in a few concepts, and only if there
   // are no intervening modifiers.  Shut off the concept if there
   // are modifiers.

   concept_kind kk = next_parseptr->concept->kind;

   if (junk_concepts.test_herit_and_final_bits() != 0)
      kk = concept_comment;

   mpkind mk, mkbox;

   if (ss->kind == s2x6) {
      if (global_livemask == 07474) {
         mk = MPKIND__OFFS_R_HALF; mkbox = MPKIND__OFFS_R_HALF_SPECIAL; }
      else if (global_livemask == 01717) {
         mk = MPKIND__OFFS_L_HALF; mkbox = MPKIND__OFFS_L_HALF_SPECIAL; }
      else fail("Can't find a parallelogram.");
   }
   else if (ss->kind == s2x5) {
      warn(warn__1_4_pgram);
      if (global_livemask == 0x3DE) {
         mk = MPKIND__OFFS_R_ONEQ; mkbox = MPKIND__OFFS_R_ONEQ_SPECIAL; }
      else if (global_livemask == 0x1EF) {
         mk = MPKIND__OFFS_L_ONEQ; mkbox = MPKIND__OFFS_L_ONEQ_SPECIAL; }
      else fail("Can't find a parallelogram.");
   }
   else if (ss->kind == s2x7) {
      warn(warn__3_4_pgram);
      if (global_livemask == 0x3C78) {
         mk = MPKIND__OFFS_R_THRQ; mkbox = MPKIND__OFFS_R_THRQ_SPECIAL; }
      else if (global_livemask == 0x078F) {
         mk = MPKIND__OFFS_L_THRQ; mkbox = MPKIND__OFFS_L_THRQ_SPECIAL; }
      else fail("Can't find a parallelogram.");
   }
   else if (ss->kind == s2x8) {
      warn(warn__full_pgram);
      if (global_livemask == 0xF0F0) {
         mk = MPKIND__OFFS_R_FULL; mkbox = MPKIND__OFFS_R_FULL_SPECIAL; }
      else if (global_livemask == 0x0F0F) {
         mk = MPKIND__OFFS_L_FULL; mkbox = MPKIND__OFFS_L_FULL_SPECIAL; }
      else fail("Can't find a parallelogram.");
   }
   else if (ss->kind == s4x6 && kk == concept_do_phantom_2x4) {
      // See whether people fit unambiguously
      // into one parallelogram or the other.
      if ((global_livemask & 003600360) == 0 && (global_livemask & 060036003) != 0)
         mk = MPKIND__OFFS_L_HALF;
      else if ((global_livemask & 060036003) == 0 && (global_livemask & 003600360) != 0)
         mk = MPKIND__OFFS_R_HALF;
      else fail("Can't find a parallelogram.");
      warn(warn__pg_hard_to_see);
   }
   else if (ss->kind == s4x5 && kk == concept_do_phantom_2x4) {
      // See whether people fit unambiguously
      // into one parallelogram or the other.
      if ((global_livemask & 0x0C030) == 0 && (global_livemask & 0x80601) != 0)
         mk = MPKIND__OFFS_L_ONEQ;
      else if ((global_livemask & 0x80601) == 0 && (global_livemask & 0x0C030) != 0)
         mk = MPKIND__OFFS_R_ONEQ;
      else fail("Can't find a parallelogram.");
      warn(warn__1_4_pgram);
      warn(warn__pg_hard_to_see);
   }
   else
      fail("Can't do parallelogram concept from this position.");

   phantest_kind phancontrol = phantest_ok;
   uint32 map_code = ~0UL;

   if (kk == concept_do_phantom_2x4 &&
       (ss->kind == s2x6 || ss->kind == s2x5 || ss->kind == s2x7 ||
        ss->kind == s4x5 || ss->kind == s4x6)) {

      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

      if (ss->kind == s2x5) {
         do_matrix_expansion(ss, CONCPROP__NEEDK_4X5, false);
         if (ss->kind != s4x5) fail("Must have a 4x5 setup for this concept.");
      }
      if (ss->kind == s2x6) {
         do_matrix_expansion(ss, CONCPROP__NEEDK_4X6, false);
         if (ss->kind != s4x6) fail("Must have a 4x6 setup for this concept.");
      }

      int linesp = next_parseptr->concept->arg2 & 7;

      if (standard_concept) {
         ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;
         uint32 tbonetest;
         global_livemask = get_standard_people(ss, standard_concept->options.who,
                                               tbonetest, global_tbonetest);

         if (!tbonetest) {
            result->kind = nothing;
            clear_result_flags(result);
            return;
         }
      
         if ((tbonetest & 011) != 011)
            fail("People are not T-boned -- 'standard' is meaningless.");
      
         if (!global_tbonetest)
            fail("No one is standard.");
         if ((global_tbonetest & 011) == 011)
            fail("The standard people are not facing consistently.");
      }

      if (linesp & 1) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      }
      else {
         if (global_tbonetest & 010) fail("There are no columns of 4 here.");
      }

      if (linesp == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      // Look for "parallelogram split phantom C/L/W"
      // or "parallelogram stagger/big block/O/butterfly".

      switch (next_parseptr->concept->arg3) {
      case MPKIND__SPLIT:
         map_code = MAPCODE(s2x4,2,mk,1);
         break;
      case MPKIND__STAG:
         if (mk == MPKIND__OFFS_L_HALF)
            map_code = spcmap_lh_stag;
         else if (mk == MPKIND__OFFS_R_HALF)
            map_code = spcmap_rh_stag;
         else if (mk == MPKIND__OFFS_L_ONEQ)
            map_code = spcmap_lh_stag_1_4;
         else if (mk == MPKIND__OFFS_R_ONEQ)
            map_code = spcmap_rh_stag_1_4;
         break;
      case MPKIND__O_SPOTS:
      case MPKIND__X_SPOTS:
         if (mk == MPKIND__OFFS_L_HALF)
            map_code = spcmap_lh_ox;
         else if (mk == MPKIND__OFFS_R_HALF)
            map_code = spcmap_rh_ox;
         break;
      }

      phancontrol = (phantest_kind) next_parseptr->concept->arg1;
      ss->cmd.parseptr = next_parseptr->next;
   }
   else if (kk == concept_do_phantom_boxes &&
            next_parseptr->concept->arg3 == MPKIND__SPLIT &&
            ss->kind == s2x6) {     // Only allow 50% offset.
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, false);
      if (ss->kind != s2x8) fail("Not in proper setup for this concept.");

      if (standard_concept) fail("Don't use \"standard\" with split phantom boxes.");

      ss->cmd.parseptr = next_parseptr->next;
      map_code = MAPCODE(s2x4,2,mkbox,0);
   }
   else if (kk == concept_multiple_boxes &&
            next_parseptr->concept->arg4 == 3 &&
            next_parseptr->concept->arg5 == MPKIND__SPLIT &&
            ss->kind == s2x6) {     // Only allow 50% offset.
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, false);
      if (ss->kind != s2x8) fail("Not in proper setup for this concept.");

      if (standard_concept) fail("Don't use \"standard\" with triple boxes.");

      ss->cmd.parseptr = next_parseptr->next;
      map_code = MAPCODE(s2x2,3,mkbox,0);
   }
   else
      map_code = MAPCODE(s2x4,1,mk,0);   // Plain parallelogram.

   if (map_code == ~0UL)
      fail("Can't do this concept with parallelogram.");

   divided_setup_move(ss, map_code, phancontrol, true, result);

   // The split-axis bits are gone.  If someone needs them, we have work to do.
   result->result_flags.clear_split_info();
}


static void do_concept_quad_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int cstuff;
   uint32 masks[3];

   if (ss->kind != s2x8) fail("Must have a 2x8 setup to do this concept.");

   cstuff = parseptr->concept->arg1;
   /* cstuff =
      forward          : 0
      left             : 1
      back             : 2
      right            : 3
      together         : 6
      apart            : 7
      clockwise        : 8
      counterclockwise : 9
      toward the center : 12 */

   if (cstuff < 4) {      /* Working forward/back/right/left. */
      int tbonetest = ss->people[2].id1 |
                      ss->people[3].id1 |
                      ss->people[4].id1 |
                      ss->people[5].id1 |
                      ss->people[10].id1 |
                      ss->people[11].id1 |
                      ss->people[12].id1 |
                      ss->people[13].id1;

      if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
      if ((tbonetest & 01) && (cstuff & 1)) fail("Must indicate forward/back.");

      masks[0] = 0xC3 ; masks[1] = 0xC3; masks[2] = 0xFF;
      cstuff <<= 2;

      /* Look at the center 8 people and put each one in the correct group. */

      if (((ss->people[2].id1  + 6) ^ cstuff) & 8) { masks[0] |= 0x04; masks[1] &= ~0x01; }
      if (((ss->people[3].id1  + 6) ^ cstuff) & 8) { masks[0] |= 0x08; masks[1] &= ~0x02; }
      if (((ss->people[4].id1  + 6) ^ cstuff) & 8) { masks[1] |= 0x04; masks[2] &= ~0x01; }
      if (((ss->people[5].id1  + 6) ^ cstuff) & 8) { masks[1] |= 0x08; masks[2] &= ~0x02; }
      if (((ss->people[10].id1 + 6) ^ cstuff) & 8) { masks[1] |= 0x10; masks[2] &= ~0x40; }
      if (((ss->people[11].id1 + 6) ^ cstuff) & 8) { masks[1] |= 0x20; masks[2] &= ~0x80; }
      if (((ss->people[12].id1 + 6) ^ cstuff) & 8) { masks[0] |= 0x10; masks[1] &= ~0x40; }
      if (((ss->people[13].id1 + 6) ^ cstuff) & 8) { masks[0] |= 0x20; masks[1] &= ~0x80; }
   }
   else if (cstuff == 6) {   /* Working together. */
      masks[0] = 0xE7 ; masks[1] = 0x66; masks[2] = 0x7E;
   }
   else if (cstuff == 7) {   /* Working apart. */
      masks[0] = 0xDB ; masks[1] = 0x99; masks[2] = 0xBD;
   }
   else if (cstuff == 8) {   /* Working clockwise. */
      masks[0] = 0xF3 ; masks[1] = 0x33; masks[2] = 0x3F;
   }
   else if (cstuff == 9) {   /* Working counterclockwise. */
      masks[0] = 0xCF ; masks[1] = 0xCC; masks[2] = 0xFC;
   }
   else {                    /* Working toward the center. */
      masks[0] = 0xC3 ; masks[1] = 0xFF; masks[2] = 0x3C;
   }

   overlapped_setup_move(ss, MAPCODE(s2x4,3,MPKIND__OVERLAP,0), masks, result);
}


static void do_concept_triple_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int cstuff;
   uint32 m1, m2;
   uint32 masks[2];

   if (ss->kind != s3dmd) fail("Must have a triple diamond setup to do this concept.");

   cstuff = parseptr->concept->arg1;
   /* cstuff =
      together         : 0
      right            : 1
      left             : 2 */

   if (cstuff == 0) {
      if ((ss->people[1].id1 | ss->people[7].id1) & 010)
         fail("Can't tell where points of center diamond should work.");
   
      m1 = 0xE9; m2 = 0xBF;

      /* Look at the center diamond points and put each one in the correct group. */

      if (ss->people[1].id1 & 2) { m1 |= 0x02; m2 &= ~0x01; }
      if (ss->people[7].id1 & 2) { m1 |= 0x10; m2 &= ~0x20; }
   }
   else {      /* Working right or left. */
      if ((ss->people[1].id1 | ss->people[7].id1 | ss->people[5].id1 | ss->people[11].id1) & 001)
         fail("Can't tell where center 1/4 tag should work.");

      m1 = 0xE1; m2 = 0xFF;
      cstuff &= 2;

      /* Look at the center 1/4 tag and put each one in the correct group. */

      if ((ss->people[1].id1  ^ cstuff) & 2) { m1 |= 0x02; m2 &= ~0x01; }
      if ((ss->people[7].id1  ^ cstuff) & 2) { m1 |= 0x10; m2 &= ~0x20; }
      if ((ss->people[5].id1  ^ cstuff) & 2) { m1 |= 0x04; m2 &= ~0x80; }
      if ((ss->people[11].id1 ^ cstuff) & 2) { m1 |= 0x08; m2 &= ~0x40; }
   }

   masks[0] = m1; masks[1] = m2;
   overlapped_setup_move(ss, MAPCODE(s_qtag,2,MPKIND__OVERLAP,0), masks, result);
}


static void do_concept_quad_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int cstuff;
   uint32 m1, m2, m3;
   uint32 masks[3];

   if (ss->kind != s4dmd) fail("Must have a quadruple diamond setup to do this concept.");

   cstuff = parseptr->concept->arg1;
   /* cstuff =
      together          : 0
      right             : 1
      left              : 2
      toward the center : 12 */

   if (cstuff == 12) {      /* Working toward the center. */
      m1 = 0xE1; m2 = 0xFF; m3 = 0x1E;
   }
   else if (cstuff == 0) {  /* Working together. */
      if ((ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1) & 010)
         fail("Can't tell where points of center diamonds should work.");
   
      m1 = 0xE9; m2 = 0xA9; m3 = 0xBF;
   
      /* Look at the center diamond points and put each one in the correct group. */
   
      if (ss->people[1].id1 & 2)  { m1 |= 0x02; m2 &= ~0x01; }
      if (ss->people[2].id1 & 2)  { m2 |= 0x02; m3 &= ~0x01; }
      if (ss->people[9].id1 & 2)  { m2 |= 0x10; m3 &= ~0x20; }
      if (ss->people[10].id1 & 2) { m1 |= 0x10; m2 &= ~0x20; }
   }
   else {      /* Working right or left. */
      if ((    ss->people[1].id1 | ss->people[2].id1 | ss->people[6].id1 | ss->people[7].id1 |
               ss->people[9].id1 | ss->people[10].id1 | ss->people[14].id1 | ss->people[15].id1) & 001)
         fail("Can't tell where center 1/4 tags should work.");

      m1 = 0xE1; m2 = 0xE1; m3 = 0xFF;
      cstuff &= 2;

      /* Look at the center 1/4 tags and put each one in the correct group. */

      if ((ss->people[1].id1  ^ cstuff) & 2) { m1 |= 0x02; m2 &= ~0x01; }
      if ((ss->people[10].id1 ^ cstuff) & 2) { m1 |= 0x10; m2 &= ~0x20; }
      if ((ss->people[15].id1 ^ cstuff) & 2) { m1 |= 0x04; m2 &= ~0x80; }
      if ((ss->people[14].id1 ^ cstuff) & 2) { m1 |= 0x08; m2 &= ~0x40; }
      if ((ss->people[2].id1  ^ cstuff) & 2) { m2 |= 0x02; m3 &= ~0x01; }
      if ((ss->people[9].id1  ^ cstuff) & 2) { m2 |= 0x10; m3 &= ~0x20; }
      if ((ss->people[6].id1  ^ cstuff) & 2) { m2 |= 0x04; m3 &= ~0x80; }
      if ((ss->people[7].id1  ^ cstuff) & 2) { m2 |= 0x08; m3 &= ~0x40; }
   }

   masks[0] = m1; masks[1] = m2; masks[2] = m3;
   overlapped_setup_move(ss, MAPCODE(s_qtag,3,MPKIND__OVERLAP,0), masks, result);
}


static void do_concept_triple_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int cstuff;
   uint32 m1, m2;
   uint32 masks[2];

   if (ss->kind != s2x6) fail("Must have a 2x6 setup to do this concept.");

   cstuff = parseptr->concept->arg1;
   /* cstuff =
      forward          : 0
      left             : 1
      back             : 2
      right            : 3
      together         : 6
      apart            : 7
      clockwise        : 8
      counterclockwise : 9 */

   if (cstuff < 4) {         /* Working forward/back/right/left. */
      int tbonetest = ss->people[2].id1 |
                      ss->people[3].id1 |
                      ss->people[8].id1 |
                      ss->people[9].id1;

      if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
      if ((tbonetest & 01) && (cstuff & 1)) fail("Must indicate forward/back.");

      // Look at the center 4 people and put each one in the correct group.

      m1 = 0xC3; m2 = 0xFF;
      cstuff <<= 2;

      if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m1 |= 0x04 ; m2 &= ~0x01; };
      if (((ss->people[3].id1 + 6) ^ cstuff) & 8) { m1 |= 0x08 ; m2 &= ~0x02; };
      if (((ss->people[8].id1 + 6) ^ cstuff) & 8) { m1 |= 0x10 ; m2 &= ~0x40; };
      if (((ss->people[9].id1 + 6) ^ cstuff) & 8) { m1 |= 0x20 ; m2 &= ~0x80; };
   }
   else if (cstuff == 6) {   // Working together.
      m1 = 0xE7; m2 = 0x7E;
   }
   else if (cstuff == 7) {   // Working apart.
      m1 = 0xDB; m2 = 0xBD;
   }
   else if (cstuff == 8) {   // Working clockwise.
      m1 = 0xF3; m2 = 0x3F;
   }
   else {                    // Working counterclockwise.
      m1 = 0xCF; m2 = 0xFC;
   }

   masks[0] = m1; masks[1] = m2;
   overlapped_setup_move(ss, MAPCODE(s2x4,2,MPKIND__OVERLAP,0), masks, result);
}


static void do_triple_formation(
   setup *ss,
   parse_block *parseptr,
   uint32 code,
   setup *result) THROW_DECL
{
   // Concepts that use this must have:
   // arg5 = a map_kind, typically MPKIND_SPLIT.  It may be MPKIND_CONCPHAN
   //   for "concentric triple boxes".  among other things, this will force
   //   the use of "divided_setup_move" to force spots or whatever.
   // arg3 = assumption stuff to put into misc bits:
   //
   //   CMD_MISC__VERIFY_DMD_LIKE   "diamonds" -- require diamond-like, i.e.
   //      centers in some kind of line, ends are line-like.
   //   CMD_MISC__VERIFY_1_4_TAG    "1/4 tags" -- centers in some kind of line,
   //      ends are a couple looking in (includes 1/4 line, etc.)
   //      If this isn't specific enough for you, use the "ASSUME LEFT 1/4 LINES" concept,
   //      or whatever.
   //   CMD_MISC__VERIFY_3_4_TAG    "3/4 tags" -- centers in some kind of line,
   //      ends are a couple looking out (includes 3/4 line, etc.)
   //      If this isn't specific enough for you, use the "ASSUME LEFT 3/4 LINES" concept,
   //      or whatever.
   //   CMD_MISC__VERIFY_REAL_1_4_LINE  "1/4 lines" -- centers in 2FL.
   //   CMD_MISC__VERIFY_REAL_3_4_LINE  "3/4 lines" -- centers in 2FL.
   //   CMD_MISC__VERIFY_QTAG_LIKE  "general 1/4 tags" -- all facing same orientation --
   //      centers in some kind of line, ends are column-like.
   //   0                           "diamond spots" -- any facing direction is allowed.

   ss->cmd.cmd_misc_flags |= parseptr->concept->arg3;

   // Mystic only allowed with "triple boxes", not "concentric triple boxes".

   if ((ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_MYSTIC) &&
       parseptr->concept->arg5 != MPKIND__SPLIT)
      fail("Mystic not allowed with this concept.");

   // If concept was "matrix triple boxes", we can't use the elegant "concentric" method.
   if (parseptr->concept->arg5 == MPKIND__SPLIT &&
       !(ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT))
      concentric_move(ss, &ss->cmd, &ss->cmd, schema_in_out_triple,
                      0, DFM1_CONC_CONCENTRIC_RULES,
                      true, ~0UL, result);
   else {
      // Gotta do this by hand.  Sigh.
      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_MYSTIC) {
         ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_SPLIT;
         if (ss->cmd.cmd_misc2_flags & CMD_MISC2__INVERT_MYSTIC)
            ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_INVERT;
         ss->cmd.cmd_misc2_flags &= ~(CMD_MISC2__CENTRAL_MYSTIC|CMD_MISC2__INVERT_MYSTIC);
      }

      divided_setup_move(ss, code, phantest_ok, true, result);
   }
}


static void do_concept_multiple_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   // Arg1 = the C/L/W indicator.  For "triple 1x4's" it has the special value 2.
   // Arg2 = the matrix that we need, with CONCPROP__NEED_ARG2_MATRIX.  It has
   //    already been taken care of.
   // Arg3 = 1 for "quadruple C/L/W of 3".  Zero otherwise.
   // Arg4 = the number of items.
   // Arg5 = stuff used by "do_triple_formation", which we might call.
   //    It is MPKIND__SPLIT.  We don't look at it here.

   uint32 code;
   int clw_indicator = parseptr->concept->arg1;
   int rot = 0;

   if (parseptr->concept->arg4 == 4) {
      if (parseptr->concept->arg3) {
         // This is "quadruple C/L/W OF 3".
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (ss->kind == s3x4) {
            if ((clw_indicator ^ global_tbonetest) & 1) {
               if (clw_indicator & 1) fail("There are no lines of 3 here.");
               else                   fail("There are no columns of 3 here.");
            }
   
            code = MAPCODE(s1x3,4,MPKIND__SPLIT,1);
         }
         else if (ss->kind == s1x12) {
            if (!((clw_indicator ^ global_tbonetest) & 1)) {
               if (clw_indicator & 1) fail("There are no lines of 3 here.");
               else                   fail("There are no columns of 3 here.");
            }
   
            code = MAPCODE(s1x3,4,MPKIND__SPLIT,0);
         }
         else
            fail("Must have quadruple 1x3's for this concept.");
      }
      else {
         // This is plain "quadruple C/L/W" (of 4).

         if (ss->kind == s4x4) {
            rot = (global_tbonetest ^ clw_indicator ^ 1) & 1;
            if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

            ss->rotation += rot;   // Just flip the setup around and recanonicalize.
            canonicalize_rotation(ss);
            code = MAPCODE(s1x4,4,MPKIND__SPLIT,1);
         }
         else if (ss->kind == s1x16) {
            if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

            if (!((clw_indicator ^ global_tbonetest) & 1)) {
               if (clw_indicator & 1) fail("There are no lines of 4 here.");
               else                   fail("There are no columns of 4 here.");
            }
   
            code = MAPCODE(s1x4,4,MPKIND__SPLIT,0);
         }
         else if (ss->kind == sbigbigh)
            code = MAPCODE(s1x4,4,MPKIND__NONISOTROPIC,1);
         else if (ss->kind == sbigbigx)
            code = MAPCODE(s1x4,4,MPKIND__NONISOTROPIC,0);
         else
            fail("Must have quadruple 1x4's for this concept.");
      }
   }
   else if (parseptr->concept->arg4 == 3) {
      if (ss->kind == s3x4)
         code = MAPCODE(s1x4,3,MPKIND__SPLIT,1);
      else if (ss->kind == s1x12)
         code = MAPCODE(s1x4,3,MPKIND__SPLIT,0);
      else if (ss->kind == sbigh)
         code = MAPCODE(s1x4,3,MPKIND__NONISOTROPIC,1);
      else if (ss->kind == sbigx)
         code = MAPCODE(s1x4,3,MPKIND__NONISOTROPIC,0);
      else
         fail("Must have triple 1x4's for this concept.");

      // Clw_indicator = 2 is the special case of "triple 1x4's".
      if (clw_indicator != 2 && ss->kind != sbigh && ss->kind != sbigx) {
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
   
         if (!((clw_indicator ^ global_tbonetest) & 1)) {
            if (clw_indicator & 1) fail("There are no lines of 4 here.");
            else                   fail("There are no columns of 4 here.");
         }
      }
   }
   else {
      if (parseptr->concept->arg4 == 5) {
         if (ss->kind == s4x5)
            code = MAPCODE(s1x4,5,MPKIND__SPLIT,1);
         else
            fail("Must have quintuple 1x4's for this concept.");
      }
      else {
         if (ss->kind == s4x6)
            code = MAPCODE(s1x4,6,MPKIND__SPLIT,1);
         else
            fail("Must have sextuple 1x4's for this concept.");
      }

      // Clw_indicator = 2 is the special case of "quintuple 1x4's".
      if (clw_indicator != 2) {
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
   
         if ((clw_indicator ^ global_tbonetest) & 1) {
            if (clw_indicator & 1) fail("There are no lines of 4 here.");
            else                   fail("There are no columns of 4 here.");
         }
      }
   }

   // If this was multiple columns, we allow stepping to a wave.  This makes it
   // possible to do interesting cases of turn and weave, when one column
   // is a single 8 chain and another is a single DPT.  But if it was multiple
   // lines, we forbid it.

   if (clw_indicator & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (clw_indicator == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   // Sigh....  This clearly isn't the right way to do this.
   // How about CMD_MISC__VERIFY_LINES?

   if (parseptr->concept->arg4 == 3) {
      do_triple_formation(ss, parseptr, code, result);
   }
   else {
      divided_setup_move(ss, code, phantest_ok, true, result);
      result->rotation -= rot;   // Flip the setup back.
  }
}




static void do_concept_triple_1x8_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 m1, m2;
   uint32 masks[2];
   int i, tbonetest;

   int cstuff = parseptr->concept->arg1;
   /* cstuff =
      forward (lines) or left (cols)   : 0
      backward (lines) or right (cols) : 2 */

   int linesp = parseptr->concept->arg2;

   if (linesp & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (linesp == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (ss->kind != s3x8) fail("Must have a 3x8 setup for this concept.");

   tbonetest = 0;
   for (i=0; i<24; i++) tbonetest |= ss->people[i].id1;

   if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (linesp & 1) {
      if (tbonetest & 1) fail("There are no lines of 8 here.");
   }
   else {
      if (!(tbonetest & 1)) fail("There are no columns of 8 here.");
   }

   /* Initially assign the centers to the upper (m2) group. */
   m1 = 0xFF00; m2 = 0xFFFF;

   /* Look at the center line people and put each one in the correct group. */

   if ((ss->people[20].id1 ^ cstuff) & 2) { m2 &= ~0x8000 ; m1 |= 0x1; };
   if ((ss->people[21].id1 ^ cstuff) & 2) { m2 &= ~0x4000 ; m1 |= 0x2; };
   if ((ss->people[22].id1 ^ cstuff) & 2) { m2 &= ~0x2000 ; m1 |= 0x4; };
   if ((ss->people[23].id1 ^ cstuff) & 2) { m2 &= ~0x1000 ; m1 |= 0x8; };
   if ((ss->people[11].id1 ^ cstuff) & 2) { m2 &= ~0x800  ; m1 |= 0x10; };
   if ((ss->people[10].id1 ^ cstuff) & 2) { m2 &= ~0x400  ; m1 |= 0x20; };
   if ((ss->people[9].id1  ^ cstuff) & 2) { m2 &= ~0x200  ; m1 |= 0x40; };
   if ((ss->people[8].id1  ^ cstuff) & 2) { m2 &= ~0x100  ; m1 |= 0x80; };

   masks[0] = m1; masks[1] = m2;
   overlapped_setup_move(ss, MAPCODE(s2x8,2,MPKIND__OVERLAP,1),
                         masks, result);
}


static void do_concept_triple_diag(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   int cstuff = parseptr->concept->arg1;
   int q;

   if (ss->kind != s_bigblob) fail("Must have a rather large setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (cstuff == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if ((global_livemask & ~0x56A56A) == 0) q = 0;
   else if ((global_livemask & ~0xA95A95) == 0) q = 2;
   else fail("Can't identify triple diagonal setup.");

   static specmapkind maps_3diag[4]   = {spcmap_blob_1x4c, spcmap_blob_1x4a,
                                         spcmap_blob_1x4d, spcmap_blob_1x4b};

   divided_setup_move(ss, maps_3diag[q + ((cstuff ^ global_tbonetest) & 1)],
                      phantest_ok, true, result);
}


static void do_concept_triple_diag_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int q;
   uint32 m1, m2;
   uint32 masks[2];

   int cstuff = parseptr->concept->arg1;
   /* cstuff =
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8 (doesn't really exist)
      apart    : 9 (doesn't really exist) */

   if (ss->kind != s_bigblob) fail("Must have a rather large setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->arg2 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   /* Initially assign the centers to the right or upper (m2) group. */
   m1 = 0xF0; m2 = 0xFF;

   if ((global_livemask & ~0x56A56A) == 0) q = 0;
   else if ((global_livemask & ~0xA95A95) == 0) q = 2;
   else fail("Can't identify triple diagonal setup.");

   // Look at the center line/column people and put each one in the correct group.

   static specmapkind maps_3diagwk[4] = {spcmap_wblob_1x4a, spcmap_wblob_1x4c,
                                         spcmap_wblob_1x4b, spcmap_wblob_1x4d};

   uint32 map_code = maps_3diagwk[q+((cstuff ^ global_tbonetest) & 1)];
   const map::map_thing *map_ptr = map::get_map_from_code(map_code);

   if ((cstuff + 1 - ss->people[map_ptr->maps[0]].id1) & 2) { m2 &= ~0x80 ; m1 |= 0x1; };
   if ((cstuff + 1 - ss->people[map_ptr->maps[1]].id1) & 2) { m2 &= ~0x40 ; m1 |= 0x2; };
   if ((cstuff + 1 - ss->people[map_ptr->maps[2]].id1) & 2) { m2 &= ~0x20 ; m1 |= 0x4; };
   if ((cstuff + 1 - ss->people[map_ptr->maps[3]].id1) & 2) { m2 &= ~0x10 ; m1 |= 0x8; };

   masks[0] = m1; masks[1] = m2;
   overlapped_setup_move(ss, map_code, masks, result);
}


static void do_concept_grand_working(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int cstuff;
   uint32 tbonetest;
   uint32 m0, m1, m2, m3, m4;
   uint32 masks[8];
   setup_kind kk;
   int arity = 2;

   cstuff = parseptr->concept->arg1;
   /* cstuff =
      forward          : 0
      left             : 1
      back             : 2
      right            : 3
      clockwise        : 8
      counterclockwise : 9
      as centers       : 10
      as ends          : 11 */

   if (ss->kind == s2x4 &&
       ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_12_MATRIX))
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X6, false);
   else if ((ss->kind == s2x4 || ss->kind == s2x6) &&
            ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_16_MATRIX))
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, false);

   if (ss->kind == s2x4) {
      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest =
            ss->people[1].id1 | ss->people[2].id1 |
            ss->people[5].id1 | ss->people[6].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 4 people and put each one in the correct group. */

         m0 = 0x9; m1 = 0x9; m2 = 0xF;
         cstuff <<= 2;

         if (((ss->people[1].id1 + 6) ^ cstuff) & 8) { m0 |= 0x2 ; m1 &= ~0x1; };
         if (((ss->people[6].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x8; };
         if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m1 |= 0x2 ; m2 &= ~0x1; };
         if (((ss->people[5].id1 + 6) ^ cstuff) & 8) { m1 |= 0x4 ; m2 &= ~0x8; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         /* Put each of the center 4 people in the correct group, no need to look. */

         if (cstuff & 1) {
            m0 = 0xB; m1 = 0xA; m2 = 0xE;
         }
         else {
            m0 = 0xD; m1 = 0x5; m2 = 0x7;
         }
      }
      else        /* Working as-ends or as-centers. */
         fail("May not specify as-ends/as-centers here.");

      kk = s2x2;
   }
   else if (ss->kind == s2x6 &&
            ((ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) ||
             (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_12_MATRIX)))) {

      ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_12_MATRIX);

      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest =
            ss->people[1].id1 | ss->people[2].id1 |
            ss->people[3].id1 | ss->people[4].id1 |
            ss->people[7].id1 | ss->people[8].id1 |
            ss->people[9].id1 | ss->people[10].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 8 people and put each one in the correct group. */

         m0 = m1 = m2 = m3 = 0x9; m4 = 0xF;
         cstuff <<= 2;

         if (((ss->people[1].id1  + 6) ^ cstuff) & 8) { m0 |= 0x2 ; m1 &= ~0x1; };
         if (((ss->people[10].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x8; };
         if (((ss->people[2].id1  + 6) ^ cstuff) & 8) { m1 |= 0x2 ; m2 &= ~0x1; };
         if (((ss->people[9].id1  + 6) ^ cstuff) & 8) { m1 |= 0x4 ; m2 &= ~0x8; };
         if (((ss->people[3].id1  + 6) ^ cstuff) & 8) { m2 |= 0x2 ; m3 &= ~0x1; };
         if (((ss->people[8].id1  + 6) ^ cstuff) & 8) { m2 |= 0x4 ; m3 &= ~0x8; };
         if (((ss->people[4].id1  + 6) ^ cstuff) & 8) { m3 |= 0x2 ; m4 &= ~0x1; };
         if (((ss->people[7].id1  + 6) ^ cstuff) & 8) { m3 |= 0x4 ; m4 &= ~0x8; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         /* Put each of the center 4 people in the correct group, no need to look. */

         if (cstuff & 1) {
            m0 = 0xB; m1 = m2 = m3 = 0xA; m4 = 0xE;
         }
         else {
            m0 = 0xD; m1 = m2 = m3 = 0x5; m4 = 0x7;
         }
      }
      else        /* Working as-ends or as-centers. */
         fail("May not specify as-ends/as-centers here.");

      masks[3] = m3; masks[4] = m4;
      kk = s2x2;
      arity = 4;
   }
   else if (ss->kind == s2x8 &&
            ((ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) ||
             (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_16_MATRIX)))) {
      uint32 m5, m6;

      ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_16_MATRIX);

      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest =
            ss->people[1].id1 | ss->people[2].id1 |
            ss->people[3].id1 | ss->people[4].id1 |
            ss->people[5].id1 | ss->people[6].id1 |
            ss->people[9].id1 | ss->people[10].id1 |
            ss->people[11].id1 | ss->people[12].id1 |
            ss->people[13].id1 | ss->people[14].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 8 people and put each one in the correct group. */

         m0 = m1 = m2 = m3 = m4 = m5 = 0x9; m6 = 0xF;
         cstuff <<= 2;

         if (((ss->people[1].id1  + 6) ^ cstuff) & 8) { m0 |= 0x2 ; m1 &= ~0x1; };
         if (((ss->people[14].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x8; };
         if (((ss->people[2].id1  + 6) ^ cstuff) & 8) { m1 |= 0x2 ; m2 &= ~0x1; };
         if (((ss->people[13].id1 + 6) ^ cstuff) & 8) { m1 |= 0x4 ; m2 &= ~0x8; };
         if (((ss->people[3].id1  + 6) ^ cstuff) & 8) { m2 |= 0x2 ; m3 &= ~0x1; };
         if (((ss->people[12].id1 + 6) ^ cstuff) & 8) { m2 |= 0x4 ; m3 &= ~0x8; };
         if (((ss->people[4].id1  + 6) ^ cstuff) & 8) { m3 |= 0x2 ; m4 &= ~0x1; };
         if (((ss->people[11].id1 + 6) ^ cstuff) & 8) { m3 |= 0x4 ; m4 &= ~0x8; };
         if (((ss->people[5].id1  + 6) ^ cstuff) & 8) { m4 |= 0x2 ; m5 &= ~0x1; };
         if (((ss->people[10].id1 + 6) ^ cstuff) & 8) { m4 |= 0x4 ; m5 &= ~0x8; };
         if (((ss->people[6].id1  + 6) ^ cstuff) & 8) { m5 |= 0x2 ; m6 &= ~0x1; };
         if (((ss->people[9].id1  + 6) ^ cstuff) & 8) { m5 |= 0x4 ; m6 &= ~0x8; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         /* Put each of the center 4 people in the correct group, no need to look. */

         if (cstuff & 1) {
            m0 = 0xB; m1 = m2 = m3 = m4 = m5 = 0xA; m6 = 0xE;
         }
         else {
            m0 = 0xD; m1 = m2 = m3 = m4 = m5 = 0x5; m6 = 0x7;
         }
      }
      else        /* Working as-ends or as-centers. */
         fail("May not specify as-ends/as-centers here.");

      masks[3] = m3; masks[4] = m4; masks[5] = m5; masks[6] = m6;
      kk = s2x2;
      arity = 6;
   }
   else if (ss->kind == s2x3) {
      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest = ss->people[1].id1 | ss->people[4].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 2 people and put each one in the correct group. */

         m0 = 0x9; m1 = 0xF;
         cstuff <<= 2;

         if (((ss->people[1].id1 + 6) ^ cstuff) & 8) { m0 |= 0x2 ; m1 &= ~0x1; };
         if (((ss->people[4].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x8; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         m2 = 0;
         if (cstuff & 1) {
            m0 = 0xB; m1 = 0xE;
         }
         else {
            m0 = 0xD; m1 = 0x7;
         }
      }
      else        /* Working as-ends or as-centers. */
         fail("May not specify as-ends/as-centers here.");

      kk = s2x2;
      arity = 1;
   }
   else if (ss->kind == s2x5) {
      /* **** Should actually put in test for explicit matrix or "10 matrix",
         but don't have the latter. */
      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest =
            ss->people[2].id1 | ss->people[7].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 6 people and put each one in the correct group. */

         m0 = m1 = m2 = 0x9; m3 = 0xF;
         cstuff <<= 2;

         if (((ss->people[1].id1 + 6) ^ cstuff) & 8) { m0 |= 0x2 ; m1 &= ~0x1; };
         if (((ss->people[8].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x8; };
         if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m1 |= 0x2 ; m2 &= ~0x1; };
         if (((ss->people[7].id1 + 6) ^ cstuff) & 8) { m1 |= 0x4 ; m2 &= ~0x8; };
         if (((ss->people[3].id1 + 6) ^ cstuff) & 8) { m2 |= 0x2 ; m3 &= ~0x1; };
         if (((ss->people[6].id1 + 6) ^ cstuff) & 8) { m2 |= 0x4 ; m3 &= ~0x8; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         /* Put each of the center 4 people in the correct group, no need to look. */

         if (cstuff & 1) {
            m0 = 0xB; m1 = m2 = 0xA; m3 = 0xE;
         }
         else {
            m0 = 0xD; m1 = m2 = 0x5; m3 = 0x7;
         }
      }
      else        /* Working as-ends or as-centers. */
         fail("May not specify as-ends/as-centers here.");

      masks[3] = m3;
      kk = s2x2;
      arity = 3;
   }
   else if (ss->kind == s1x8) {
      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest = ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 4 people and put each one in the correct group. */

         m0 = 0x3; m1 = 0x3; m2 = 0xF;
         cstuff <<= 2;

         if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x2; };
         if (((ss->people[3].id1 + 6) ^ cstuff) & 8) { m0 |= 0x8 ; m1 &= ~0x1; };
         if (((ss->people[7].id1 + 6) ^ cstuff) & 8) { m1 |= 0x4 ; m2 &= ~0x2; };
         if (((ss->people[6].id1 + 6) ^ cstuff) & 8) { m1 |= 0x8 ; m2 &= ~0x1; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         fail("Must have a 2x3 or 2x4 setup for this concept.");
      }
      else {      /* Working as-ends or as-centers. */
         /* Put each of the center 4 people in the correct group, no need to look. */

         if (cstuff & 1) {
            m0 = 0x7; m1 = 0x5; m2 = 0xD;
         }
         else {
            m0 = 0xB; m1 = 0xA; m2 = 0xE;
         }
      }

      kk = s1x4;
   }
   else if (ss->kind == s1x6) {
      if (cstuff < 4) {      /* Working forward/back/right/left. */
         tbonetest = ss->people[2].id1 | ss->people[5].id1;
         if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
         if ((tbonetest & 001) && (cstuff & 1)) fail("Must indicate forward/back.");

         /* Look at the center 2 people and put each one in the correct group. */

         m0 = 0x3; m1 = 0xF; m2 = 0;
         cstuff <<= 2;

         if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m0 |= 0x8 ; m1 &= ~0x1; };
         if (((ss->people[5].id1 + 6) ^ cstuff) & 8) { m0 |= 0x4 ; m1 &= ~0x2; };
      }
      else if (cstuff < 10) {      /* Working clockwise/counterclockwise. */
         fail("Must have a 2x3 or 2x4 setup for this concept.");
      }
      else {      /* Working as-ends or as-centers. */
         m2 = 0;
         if (cstuff & 1) {
            m0 = 0x7; m1 = 0xD;
         }
         else {
            m0 = 0xB; m1 = 0xE;
         }
      }

      arity = 1;
      kk = s1x4;
   }
   else
      fail("Must have a 2x3, 2x4, 1x6, or 1x8 setup for this concept.");

   masks[0] = m0; masks[1] = m1; masks[2] = m2;
   overlapped_setup_move(ss, MAPCODE(kk,arity+1,MPKIND__OVERLAP,0), masks, result);
}


static void do_concept_do_phantom_2x2(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // Do "blocks" or "4 phantom interlocked blocks", etc.

   if (ss->kind != s4x4) fail("Must have a 4x4 setup for this concept.");
   divided_setup_move(ss, parseptr->concept->arg1,
                      (phantest_kind) parseptr->concept->arg2,
                      true, result);
}


static void do_concept_do_triangular_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 division_map_code = ~0UL;
   phantest_kind phant = (phantest_kind) parseptr->concept->arg1;

   // We only allow it in a 3x4 if it was real, not phantom.  Arg1 tells which.
   if (ss->kind == s3x4 && phant == phantest_2x2_only_two) {
      if (global_livemask == 04747)
         division_map_code = spcmap_trglbox3x4a;
      else if (global_livemask == 05656)
         division_map_code = spcmap_trglbox3x4b;
      else if (global_livemask == 05353)
         division_map_code = spcmap_trglbox3x4c;
      else if (global_livemask == 05555)
         division_map_code = spcmap_trglbox3x4d;
      phant = phantest_ok;
   }
   if (ss->kind == s4x5 && phant == phantest_2x2_only_two) {
      if (global_livemask == 0xB12C4)
         division_map_code = spcmap_trglbox4x5a;
      if (global_livemask == 0x691A4)
         division_map_code = spcmap_trglbox4x5b;
      phant = phantest_ok;
   }
   else if (ss->kind == s4x4)
      division_map_code = spcmap_trglbox4x4;

   if (division_map_code == ~0UL)
      fail("Must have a 3x4, 4x4, or 4x5 setup for this concept.");

   divided_setup_move(ss, division_map_code, phant, true, result);
}


static void do_concept_do_phantom_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");

   divided_setup_move(ss, MAPCODE(s2x4,2,parseptr->concept->arg3,0),
                      (phantest_kind) parseptr->concept->arg1, true, result);
}


static void do_concept_do_phantom_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 code;

   // See "do_triple_formation" for meaning of arg3.

   if (ss->kind == s4dmd)
      code = MAPCODE(s_qtag,2,parseptr->concept->arg3,0);
   else if (ss->kind == s4ptpd)
      code = MAPCODE(s_ptpd,2,parseptr->concept->arg3,0);
   else
      fail("Must have a quadruple diamond/quarter-tag setup for this concept.");

   ss->cmd.cmd_misc_flags |= parseptr->concept->arg2;

   divided_setup_move(ss, code, (phantest_kind) parseptr->concept->arg1, true, result);
}


static void do_concept_do_divided_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // See "do_triple_formation" for meaning of arg3.

   if (ss->kind != s4x6 || (global_livemask & 0x02D02D) != 0)
      fail("Must have a divided diamond or 1/4 tag setup for this concept.");

   ss->cmd.cmd_misc_flags |= parseptr->concept->arg3;

   // If arg4 is nonzero, this is point-to-point diamonds.

   divided_setup_move(ss,
                      parseptr->concept->arg4 ?
                      MAPCODE(s_ptpd,2,MPKIND__SPLIT,1) :
                      MAPCODE(s_qtag,2,MPKIND__SPLIT,1),
                      (phantest_kind) parseptr->concept->arg1, true, result);
}


static void do_concept_do_divided_bones(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   setup tempsetup = *ss;

   if (parseptr->concept->arg2) {
      // Expand, first to a bigbone, and then to a dblrig.
      // Either or both of these may be unnecessary or may fail.

      do_matrix_expansion(&tempsetup, CONCPROP__NEEDK_END_2X2, false);
      if (tempsetup.kind == sbigbone) expand::expand_setup(&s_bigbone_dblrig, &tempsetup);

      divided_setup_move(&tempsetup,
                         MAPCODE(s_rigger,2,MPKIND__SPLIT,0),
                         (phantest_kind) parseptr->concept->arg1, true, result);
   }
   else {
      // Expand, first to a bigrig, and then to a dblbone.
      // Either or both of these may be unnecessary or may fail.

      do_matrix_expansion(&tempsetup, CONCPROP__NEEDK_END_1X4, false);
      if (tempsetup.kind == sbigrig) expand::expand_setup(&s_bigrig_dblbone, &tempsetup);

      divided_setup_move(&tempsetup,
                         MAPCODE(s_bone,2,MPKIND__SPLIT,0),
                         (phantest_kind) parseptr->concept->arg1, true, result);
   }
}


static void do_concept_distorted(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   distorted_move(ss, parseptr, (disttest_kind) parseptr->concept->arg1, result);
}


static void do_concept_dblbent(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{

/*
   Args from the concept are as follows:
   arg1 =
      0 - user claims this is some kind of columns
      1 - user claims this is some kind of lines
      3 - user claims this is waves
   8 bit -- this is double bent tidal
   16 bit -- this is bent box
*/

   uint32 map_code = 0;
   uint32 arg1 = parseptr->concept->arg1;
   setup otherfolks = *ss;
   setup *otherfolksptr = (setup *) 0;


   if (arg1 & 16) {
      //Bent boxes.
      if (ss->kind == s3x6) {
         if (global_livemask == 0x2170B)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT0CW,0);
         else if (global_livemask == 0x26934)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT0CCW,0);
         else if (global_livemask == 0x0B259)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT1CW,0);
         else if (global_livemask == 0x0CC66)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT1CCW,0);
      }
      else if (ss->kind == s4x6) {
         if (global_livemask == 0x981981)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT2CW,0);
         else if (global_livemask == 0x660660)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT2CCW,0);
         else if (global_livemask == 0xD08D08)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT3CW,0);
         else if (global_livemask == 0x2C42C4)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT3CCW,0);
         else if (global_livemask == 0x303303)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT4CW,0);
         else if (global_livemask == 0x330330)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT4CCW,0);
         else if (global_livemask == 0x858858)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT5CW,0);
         else if (global_livemask == 0x846846)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT5CCW,0);
      }
      else if (ss->kind == sbigh) {
         if (global_livemask == 0xCF3)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT6CW,0);
         else if (global_livemask == 0xF3C)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT6CCW,0);
      }
      else if (ss->kind == sdeepxwv) {
         if (global_livemask == 0xCF3)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT7CW,0);
         else if (global_livemask == 0x3CF)
            map_code = MAPCODE(s2x2,2,MPKIND__BENT7CCW,0);
      }
   }
   else if (!(arg1 & 8)) {
      //Bent C/L/W's.
      if (ss->kind == s3x6) {
         if (global_livemask == 0x2170B)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT2CW,0);
         else if (global_livemask == 0x26934)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT2CCW,0);
         else if (global_livemask == 0x0B259)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT3CW,0);
         else if (global_livemask == 0x0CC66)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT3CCW,0);
      }
      else if (ss->kind == s4x6) {
         if (global_livemask == 0x981981)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT0CW,0);
         else if (global_livemask == 0x660660)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT0CCW,0);
         else if (global_livemask == 0xD08D08)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT1CW,0);
         else if (global_livemask == 0x2C42C4)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT1CCW,0);
         else if (global_livemask == 0x303303)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT6CW,0);
         else if (global_livemask == 0x330330)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT6CCW,0);
         else if (global_livemask == 0x858858)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT7CW,0);
         else if (global_livemask == 0x846846)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT7CCW,0);
      }
      else if (ss->kind == sbigh) {
         if (global_livemask == 0xCF3)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT4CW,0);
         else if (global_livemask == 0xF3C)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT4CCW,0);
      }
      else if (ss->kind == sdeepxwv) {
         if (global_livemask == 0xCF3)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT5CW,0);
         else if (global_livemask == 0x3CF)
            map_code = MAPCODE(s1x4,2,MPKIND__BENT5CCW,0);
      }
   }
   else {
      // Double bent tidal C/L/W.
      if (ss->kind == sbigh) {
         if (global_livemask == 0xCF3)
            map_code = MAPCODE(s1x8,1,MPKIND__BENT4CW,0);
         else if (global_livemask == 0xF3C)
            map_code = MAPCODE(s1x8,1,MPKIND__BENT4CCW,0);
      }
      else if (ss->kind == sbigptpd) {
         // We need to save a few people.  Maybe the "selective_move"
         // mechanism, with "override_selector", would be a better way.
         otherfolksptr = &otherfolks;

         if (global_livemask == 0xF3C)
            map_code = MAPCODE(s1x6,1,MPKIND__BENT0CW,0);
         else if (global_livemask == 0x3CF)
            map_code = MAPCODE(s1x6,1,MPKIND__BENT0CCW,0);
      }
   }

   if (!map_code)
      fail("Can't do this concept in this formation.");

   switch (arg1 & 7) {
   case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
   case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
   case 3: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES; break;
   }

   divided_setup_move(ss, map_code, phantest_ok, true, result);

   if (otherfolksptr) {
      if (otherfolksptr->kind != result->kind ||
          otherfolksptr->rotation != result->rotation)
         fail("Can't do this concept in this formation.");
      (void) install_person(result, 2, otherfolksptr, 2);
      (void) install_person(result, 8, otherfolksptr, 8);
   }
}


static void do_concept_omit(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // We need to go through the motions of doing the call, at least
   // through any sequential definition, so that the number of parts
   // will be known.
   ss->cmd.cmd_misc2_flags |= CMD_MISC2__DO_NOT_EXECUTE;
   move(ss, false, result);
   // But that left the result as "nothing".  So now we
   // copy the incoming setup to the result, so that we will actually
   // have done the null call.  But of course we save our hard-won
   // result flags.
   resultflag_rec save_flags = result->result_flags;
   *result = *ss;
   result->result_flags = save_flags;
}


static void do_concept_once_removed(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 map_code = ~0UL;

   // We allow "3x1" and the like only with plain "once removed".
   if (parseptr->concept->arg1 &&
       ss->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK))
      fail("Illegal modifier before a concept.");

   if (parseptr->concept->arg1 == 2) {
      switch (ss->kind) {
      case s1x6:
         map_code = MAPCODE(s1x2,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s1x12:
         map_code = MAPCODE(s1x4,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s2x6:
         map_code = MAPCODE(s2x2,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s2x12:
         map_code = MAPCODE(s2x4,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s_ptpd:
         map_code = MAPCODE(s2x2,3,MPKIND__SPEC_TWICEREM,0);
         break;
      case s_bone:      /* Figure this out -- it is special. */
      case s_rigger:    /* Figure this out -- it is special. */
      default:
         fail("Can't do 'twice removed' from this setup.");
      }
   }
   else if (parseptr->concept->arg1 == 3) {
      switch (ss->kind) {
      case s1x8:
         map_code = MAPCODE(s1x2,4,MPKIND__THRICE_REMOVED,0);
         break;
      case s1x16:
         map_code = MAPCODE(s1x4,4,MPKIND__THRICE_REMOVED,0);
         break;
      case s2x8:
         map_code = MAPCODE(s2x2,4,MPKIND__THRICE_REMOVED,0);
         break;
      default:
         fail("Can't do 'thrice removed' from this setup.");
      }
   }
   else {

      // We allow "3x1" or "1x3".  That's all.
      // Well, we also allow "3x3" etc.

      switch (ss->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK)) {
      case 0:
         break;
      case INHERITFLAGMXNK_1X3:
      case INHERITFLAGMXNK_3X1:
         warn(warn__tasteless_junk);
         // We allow "12 matrix", but do not require it.  We have no
         // idea whether it should be required.
         ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_12_MATRIX);
         switch (ss->kind) {
         case s3x4:
            map_code = spcmap_2x3_rmvr;
            goto doit;
         case s_qtag:
            map_code = spcmap_2x3_rmvs;
            goto doit;
         default:
            fail("Can't do this concept in this formation.");
         }
         break;
      case INHERITFLAGNXNK_3X3:
         ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_12_MATRIX);
         switch (ss->kind) {
         case s1x12:
            map_code = MAPCODE(s1x6,2,MPKIND__REMOVED, 0);
            goto doit;
         default:
            fail("Can't do this concept in this formation.");
         }
         break;
      case INHERITFLAGNXNK_4X4:
         ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_16_MATRIX);
         switch (ss->kind) {
         case s1x16:
            map_code = MAPCODE(s1x8,2,MPKIND__REMOVED, 0);
            goto doit;
         default:
            fail("Can't do this concept in this formation.");
         }
         break;
      default:
         fail("Illegal modifier before a concept.");
      }

      if (parseptr->concept->arg1) {
         /* If this is the "once removed diamonds" concept, we only allow diamonds. */
         if (ss->kind != s_qtag && ss->kind != s_rigger)
            fail("There are no once removed diamonds here.");
      }
      else {

         /* If this is just the "once removed" concept, we do NOT allow the splitting of a
            quarter-tag into diamonds -- although there is only one splitting axis than
            will work, it is not generally accepted usage. */
         if (ss->kind == s_qtag)
            fail("You must use the \"once removed diamonds\" concept here.");
      }

      switch (ss->kind) {
      case s2x4:
         map_code = MAPCODE(s2x2,2,MPKIND__REMOVED,0);
         break;
      case s2x6:
         map_code = MAPCODE(s2x3,2,MPKIND__REMOVED,0);
         break;
      case s2x8:
         map_code = MAPCODE(s2x4,2,MPKIND__REMOVED,0);
         break;
      case s1x8:
         map_code = MAPCODE(s1x4,2,MPKIND__REMOVED,0);
         break;
      case s1x12:
         map_code = MAPCODE(s1x6,2,MPKIND__REMOVED,0);
         break;
      case s1x16:
         map_code = MAPCODE(s1x8,2,MPKIND__REMOVED,0);
         break;
      case s1x6:
         map_code = MAPCODE(s1x3,2,MPKIND__REMOVED,0);
         break;
      case s1x4:
         map_code = MAPCODE(s1x2,2,MPKIND__REMOVED,0);
         break;
      case s_rigger:
         map_code = MAPCODE(sdmd,2,MPKIND__REMOVED,0);
         break;
      case s_bone:
         map_code = MAPCODE(s_trngl4,2,MPKIND__REMOVED,1);
         break;
      case s_bone6:
         map_code = MAPCODE(s_trngl,2,MPKIND__REMOVED,1);
         break;
      case s_ptpd:
         /* It's too bad we can't use the "MAPCODE" mechanism here.  Maybe can fix,
            since the stuff in "innards" is fussing with these anyway. */
         map_code = spcmap_p8_tgl4;
         break;
      case s_spindle:
         map_code = spcmap_spndle_once_rem;
         break;
      case s1x3dmd:
         map_code = spcmap_1x3dmd_once_rem;
         break;
      case s_qtag:
         map_code = MAPCODE(sdmd,2,MPKIND__REMOVED,1);
         break;
      default:
         fail("Can't do 'once removed' from this setup.");
      }
   }

 doit:

   divided_setup_move(ss, map_code, phantest_ok, true, result);
}



static void do_concept_diagnose(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   char junk[200];

   move(ss, false, result);

   sprintf(junk, "Command flags: 0x%08X, result flags misc/split: 0x%08X/%d/%d.",
           (unsigned int) ss->cmd.cmd_misc_flags,
           (unsigned int) result->result_flags.misc,
           (unsigned int) result->result_flags.split_info[0],
           (unsigned int) result->result_flags.split_info[1]);
   fail(junk);
}


static void do_concept_old_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 mxnbits = ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_MXNMASK);
   bool mxnstuff = mxnbits == INHERITFLAGMXNK_1X3 || mxnbits == INHERITFLAGMXNK_3X1;

   move(ss, false, result);

   // We don't check if this was 3x1 -- too complicated for now.

   uint32 field_to_check = (result->rotation & 1);

   if (!mxnstuff && result->result_flags.split_info[field_to_check] == 0)
      fail("Stretch call was not a 4 person call divided along stretching axis.");

   if (mxnstuff && (result->kind == s1x8)) {
      /* This is a bit sleazy. */

      if (((result->people[0].id1 ^ result->people[1].id1) & 2) ||
               (result->result_flags.misc & RESULTFLAG__VERY_ENDS_ODD)) {
         swap_people(result, 1, 6);
         swap_people(result, 2, 5);
         swap_people(result, 3, 7);
      }
      else if (((result->people[2].id1 ^ result->people[3].id1) & 2) ||
               (result->result_flags.misc & RESULTFLAG__VERY_CTRS_ODD)) {
         swap_people(result, 2, 6);
      }
      else {
         fail("Sorry, can't figure this out.");
      }
   }
   else {
      if (mxnstuff)
         fail("1x3 or 3x1 Stretch call not permitted here.");

      if (result->kind == s2x4) {
         swap_people(result, 1, 2);
         swap_people(result, 5, 6);
      }
      else if (result->kind == s1x8) {
         swap_people(result, 3, 6);
         swap_people(result, 2, 7);
      }
      else if (result->kind == s_qtag) {
         swap_people(result, 3, 7);
      }
      else if (result->kind == s_ptpd) {
         swap_people(result, 2, 6);
      }
      else if (result->kind == s1x4) {
         swap_people(result, 1, 3);
      }
      else
         fail("Stretch call didn't go to a legal setup.");
   }
}


static void do_concept_new_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 maps;
   setup tempsetup = *ss;
   int linesp = parseptr->concept->arg1;

   // linesp =
   //  16 : any setup
   //  1  : line
   //  3  : wave
   //  4  : column
   //  18 : box
   //  19 : diamond
   //  20 : just "stretched", to be used with triangles.

   if ((linesp == 18 && tempsetup.kind != s2x4) ||
       (linesp == 19 && tempsetup.kind != s_qtag && tempsetup.kind != s_ptpd) ||
       (!(linesp&16) && tempsetup.kind != s1x8))
      fail("Not in correct formation for this concept.");

   if (!(linesp & 16)) {
      if (linesp & 1) {
         if (global_tbonetest & 1) fail("There is no line of 8 here.");
      }
      else {
         if (global_tbonetest & 010) fail("There is no column of 8 here.");
      }
   }

   if (linesp == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   tempsetup.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   if (linesp == 20) {
      // This was just "stretched".  Demand that the next concept
      // be some kind of triangle designation.
      // Search ahead, skipping comments of course.  This means we
      // must skip modifiers too, so we check that there weren't any.

      final_and_herit_flags junk_concepts;
      junk_concepts.clear_all_herit_and_final_bits();
      parse_block *next_parseptr = process_final_concepts(parseptr->next, false,
                                                          &junk_concepts, true, __FILE__, __LINE__);

      if ((next_parseptr->concept->kind == concept_randomtrngl ||
          next_parseptr->concept->kind == concept_selbasedtrngl) &&
          junk_concepts.test_herit_and_final_bits() == 0) {
         if (tempsetup.kind == s_hrglass) {
            swap_people(&tempsetup, 3, 7);
         }
         else if (tempsetup.kind == s_ptpd &&
                  next_parseptr->concept->kind == concept_randomtrngl &&
                  next_parseptr->concept->arg1 == 2) {
            // We require "inside triangles".
            swap_people(&tempsetup, 2, 6);
         }
         else
            fail("Stretched setup call didn't start in appropriate setup.");
      }
      else if (next_parseptr->concept->kind == concept_do_phantom_2x4 &&
               next_parseptr->concept->arg3 == MPKIND__SPLIT &&
               junk_concepts.test_herit_and_final_bits() == 0) {
         if (tempsetup.kind == s4x4 && ((global_tbonetest & 011) != 011)) {
            if ((global_tbonetest ^ next_parseptr->concept->arg2) & 1) {
               swap_people(&tempsetup, 1, 2);
               swap_people(&tempsetup, 3, 7);
               swap_people(&tempsetup, 15, 11);
               swap_people(&tempsetup, 10, 9);
            }
            else {
               swap_people(&tempsetup, 13, 14);
               swap_people(&tempsetup, 15, 3);
               swap_people(&tempsetup, 11, 7);
               swap_people(&tempsetup, 6, 5);
            }
         }
         else
            fail("Stretched setup call didn't start in appropriate setup.");
      }
      else
         fail("\"Stretched\" concept must be followed by triangle or split phantom concept.");

      move(&tempsetup, false, result);
      return;
   }

   if (tempsetup.kind == s2x4) {
      swap_people(&tempsetup, 1, 2);
      swap_people(&tempsetup, 5, 6);
      maps = MAPCODE(s2x2,2,MPKIND__SPLIT,0);
   }
   else if (tempsetup.kind == s1x8) {
      swap_people(&tempsetup, 3, 6);
      swap_people(&tempsetup, 2, 7);
      maps = MAPCODE(s1x4,2,MPKIND__SPLIT,0);
   }
   else if (tempsetup.kind == s_qtag) {
      swap_people(&tempsetup, 3, 7);
      maps = MAPCODE(sdmd,2,MPKIND__SPLIT,1);
   }
   else if (tempsetup.kind == s_ptpd) {
      swap_people(&tempsetup, 2, 6);
      maps = MAPCODE(sdmd,2,MPKIND__SPLIT,0);
   }
   else if (tempsetup.kind == s_bone6) {
      swap_people(&tempsetup, 2, 5);
      maps = MAPCODE(s_trngl,2,MPKIND__NONISOTROP1,1);
   }
   else if (tempsetup.kind == s_rigger) {
      swap_people(&tempsetup, 0, 1);
      swap_people(&tempsetup, 4, 5);
      maps = MAPCODE(s_trngl4,2,MPKIND__SPLIT,1);
   }
   else
      fail("Stretched setup call didn't start in appropriate setup.");

   divided_setup_move(&tempsetup, maps, phantest_ok, true, result);
}


static void do_concept_mirror(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   /* This is treated differently from "left" or "reverse" in that the code in sdmoves.cpp
      that handles the latter communicates it to the "take right hands" and "rear back
      from right hands" stuff, and we do not.  This means that people will physically
      take left hands when they meet.  This seems to be the consensus of what "mirror"
      means, as distinguished from "left" or "reverse".

      We may also need to be careful about unsymmetrical selectors.  It happens that
      there are no selectors that are sensitive to left-right position ("box to my
      left peel off"), and that unsymmetrical selectors are loaded at the start of
      the entire call, when absolute orientation is manifest, so we luck out.
      That may change in the future.  We have to be careful never to load unsymmetrical
      selectors unless we are in absolute orientation space. */

   mirror_this(ss);
   /* We xor it of course -- these can be nested! */
   ss->cmd.cmd_misc_flags ^= CMD_MISC__EXPLICIT_MIRROR;
   move(ss, false, result);
   mirror_this(result);
}






static void do_concept_assume_waves(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   assumption_thing t;
   assumption_thing *e = &ss->cmd.cmd_assume;   /* Existing assumption. */
   bool no_phan_error = false;
   setup sss;
   int i;

   /* "Assume normal casts" is special. */

   if (parseptr->concept->arg1 == cr_alwaysfail) {
      if (e->assump_cast)
         fail("Redundant or conflicting assumptions.");
      e->assump_cast = 1;
      move(ss, false, result);
      return;
   }

   /* Check that there actually are some phantoms needing to be filled in.
      If user specifically said there are phantoms, we believe it. */

   if (!(ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS)) {
      no_phan_error = true;
      for (i=0; i<=attr::slimit(ss); i++) {
         if (!ss->people[i].id1) no_phan_error = false;
      }
   }

   /* We wish it were possible to encode this entire word neatly in the concept
      table, but, unfortunately, the way C struct and union initializers work
      makes it impossible. */

   t.assumption = (call_restriction) parseptr->concept->arg1;
   t.assump_col = parseptr->concept->arg2;
   t.assump_both = parseptr->concept->arg3;
   t.assump_cast = e->assump_cast;
   t.assump_live = 0;
   t.assump_negate = 0;

   /* Need to check any pre-existing assumption. */

   if (e->assumption == cr_none) ;   /* If no pre-existing assumption, OK. */
   else {     /* We have something, and must check carefully. */
      /* First, an exact match is allowed. */
      if (e->assumption == t.assumption &&
          e->assump_col == t.assump_col &&
          e->assump_both == t.assump_both) ;

      /* We also allow certain tightenings of existing assumptions. */

      else if ((t.assumption == cr_jleft || t.assumption == cr_jright) &&
               ((e->assumption == cr_diamond_like && t.assump_col == 4) ||
                (e->assumption == cr_qtag_like &&
                 t.assump_col == 0 &&
                 (e->assump_both == 0 || e->assump_both == (t.assump_both ^ 3))) ||
                (e->assumption == cr_real_1_4_tag && t.assump_both == 2) ||
                (e->assumption == cr_real_3_4_tag && t.assump_both == 1))) ;
      else if ((t.assumption == cr_ijleft || t.assumption == cr_ijright) &&
               ((e->assumption == cr_diamond_like && t.assump_col == 4) ||
                (e->assumption == cr_qtag_like &&
                 t.assump_col == 0 &&
                 (e->assump_both == 0 || e->assump_both == (t.assump_both ^ 3))) ||
                (e->assumption == cr_real_1_4_line && t.assump_both == 2) ||
                (e->assumption == cr_real_3_4_line && t.assump_both == 1))) ;
      else if ((t.assumption == cr_real_1_4_tag || t.assumption == cr_real_1_4_line) &&
               e->assumption == cr_qtag_like &&
               e->assump_both == 1) ;
      else if ((t.assumption == cr_real_3_4_tag || t.assumption == cr_real_3_4_line) &&
               e->assumption == cr_qtag_like &&
               e->assump_both == 2) ;
      else
         fail("Redundant or conflicting assumptions.");
   }

   if (t.assumption == cr_miniwaves || t.assumption == cr_couples_only ||
       ((t.assumption == cr_magic_only || t.assumption == cr_wave_only) &&
        t.assump_col == 2)) {
      switch (ss->kind) {
      case s2x4:
      case s2x6:
      case s2x8:
      case s2x3:
         if (!(or_all_people(ss) & 010)) t.assump_col++;
         break;
      }
   }

   ss->cmd.cmd_assume = t;

   /* The restrictions mean different things in different setups.  In some setups, they
      mean things that are unsuitable for the "assume" concept.  In some setups they
      take no action at all.  So we must check the setups on a case-by-case basis. */

   if (     t.assumption == cr_jleft  || t.assumption == cr_jright ||
            t.assumption == cr_ijleft || t.assumption == cr_ijright ||
            t.assumption == cr_real_1_4_tag  || t.assumption == cr_real_3_4_tag ||
            t.assumption == cr_real_1_4_line || t.assumption == cr_real_3_4_line) {
      /* These assumptions work independently of the "assump_col" number. */
      goto fudge_diamond_like;
   }
   else if ((t.assump_col & 2) == 2) {
      /* This is a special assumption -- "assume normal boxes", or "assume inverted boxes". */

      if (t.assumption == cr_wave_only || t.assumption == cr_magic_only) {
         switch (ss->kind) {
            case s2x2: goto check_it;
            case s2x4: goto check_it;
         }
      }
   }
   else if (t.assump_col == 1) {
      /* This is a "column-like" assumption. */

      if (t.assumption == cr_wave_only) {
         switch (ss->kind) {     /* "assume normal columns" */
            case s2x4: goto check_it;
         }
      }
      else if (t.assumption == cr_magic_only) {
         switch (ss->kind) {     /* "assume magic columns" */
            case s2x4: goto check_it;
         }
      }
      else if (t.assumption == cr_2fl_only) {
         switch (ss->kind) {     /* "assume DPT/CDPT" */
            case s2x4: goto check_it;
         }
      }
      else if (t.assumption == cr_couples_only || t.assumption == cr_miniwaves) {
         switch (ss->kind) {     /* "assume couples" or "assume miniwaves" */
            case s2x8: case s2x4: case s2x3: case s2x6: goto check_it;
         }
      }
      else if (t.assumption == cr_li_lo) {
         switch (ss->kind) {     /* "assume 8 chain" or "assume trade by" */
            case s2x4: goto check_it;
         }
      }
   }
   else {
      /* This is a "line-like" assumption. */

      switch (t.assumption) {
      case cr_wave_only:
         switch (ss->kind) {     /* "assume waves" */
         case s2x4: case s3x4:  case s2x8:  case s2x6:
         case s1x8: case s1x10: case s1x12: case s1x14:
         case s1x16: case s1x6: case s1x4: goto check_it;
         }
         break;
      case cr_2fl_only:
         switch (ss->kind) {     /* "assume two-faced lines" */
         case s2x4:  case s3x4:  case s1x8:  case s1x4: goto check_it;
         }
         break;
      case cr_couples_only:
      case cr_miniwaves:
         switch (ss->kind) {     /* "assume couples" or "assume miniwaves" */
         case s2x2: case s1x2: case s1x8: case s1x16:
         case s2x8: case s2x4: case s1x4: goto check_it;
         }
         break;
      case cr_magic_only:
         switch (ss->kind) {     /* "assume inverted lines" */
         case s2x4:  case s1x4: goto check_it;
         }
         break;
      case cr_1fl_only:
         switch (ss->kind) {     /* "assume one-faced lines" */
         case s1x4: case s2x4: goto check_it;
         }
         break;
      case cr_li_lo:
         switch (ss->kind) {     /* "assume lines in" or "assume lines out" */
         case s2x3: case s2x4: goto check_it;
         }
         break;
      case cr_diamond_like:
      case cr_qtag_like:
      case cr_pu_qtag_like:
         goto fudge_diamond_like;
      }
   }

   goto bad_assume;

   fudge_diamond_like:

   switch (ss->kind) {
      case s1x4: goto fudge_from_1x4;
      case s3x4: goto fudge_from_3x4;
      case sdmd: case s_qtag: case s_ptpd: case s3dmd: case s4dmd: goto check_it;
   }

   bad_assume:

   fail("This assumption is not legal from this formation.");

   fudge_from_1x4:

   no_phan_error = false;    // There really were phantoms after all.

   (void) copy_person(ss, 6, ss, 0);
   (void) copy_person(ss, 7, ss, 1);
   clear_person(ss, 0);
   clear_person(ss, 1);
   clear_person(ss, 4);
   clear_person(ss, 5);

   ss->kind = s_qtag;

   goto check_it;

   fudge_from_3x4:

   sss = *ss;
   (void) copy_person(ss, 0, &sss, 1);
   (void) copy_person(ss, 1, &sss, 2);
   (void) copy_person(ss, 2, &sss, 4);
   (void) copy_person(ss, 3, &sss, 5);
   (void) copy_person(ss, 4, &sss, 7);
   (void) copy_person(ss, 5, &sss, 8);
   (void) copy_person(ss, 6, &sss, 10);
   (void) copy_person(ss, 7, &sss, 11);

   if (sss.people[0].id1) {
      if (sss.people[1].id1) fail("Can't do this assumption.");
      else (void) copy_person(ss, 0, &sss, 0);
   }
   if (sss.people[3].id1) {
      if (sss.people[2].id1) fail("Can't do this assumption.");
      else (void) copy_person(ss, 1, &sss, 3);
   }
   if (sss.people[6].id1) {
      if (sss.people[7].id1) fail("Can't do this assumption.");
      else (void) copy_person(ss, 4, &sss, 6);
   }
   if (sss.people[9].id1) {
      if (sss.people[8].id1) fail("Can't do this assumption.");
      else (void) copy_person(ss, 5, &sss, 9);
   }

   ss->kind = s_qtag;

   check_it:

   if (no_phan_error) fail("Don't know where the phantoms should be assumed to be.");

   move_perhaps_with_active_phantoms(ss, result);
}




static void do_concept_active_phantoms(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   if (ss->cmd.cmd_assume.assump_cast)
      fail("Don't use \"active phantoms\" with \"assume normal casts\".");

   if (fill_active_phantoms_and_move(ss, result))
      fail("This assumption is not specific enough to fill in active phantoms.");
}


static void do_concept_central(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   if (parseptr->concept->arg1 == CMD_MISC2__SAID_INVERT) {
      /* If this is "invert", just flip the bit.  They can stack, of course. */
      ss->cmd.cmd_misc2_flags ^= CMD_MISC2__SAID_INVERT;
   }
   else {
      uint32 this_concept = parseptr->concept->arg1;

      /* Otherwise, if the "invert" bit was on, we assume that means that the
         user really wanted "invert snag" or whatever. */

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__SAID_INVERT) {
         if (this_concept &
            (CMD_MISC2__INVERT_CENTRAL|CMD_MISC2__INVERT_SNAG|CMD_MISC2__INVERT_MYSTIC))
            fail("You can't invert a concept twice.");
         /* Take out the "invert" bit". */
         ss->cmd.cmd_misc2_flags &= ~CMD_MISC2__SAID_INVERT;
         /* Put in the "this concept is inverted" bit. */
         if (this_concept == CMD_MISC2__DO_CENTRAL)
            this_concept |= CMD_MISC2__INVERT_CENTRAL;
         else if (this_concept == CMD_MISC2__CENTRAL_SNAG)
            this_concept |= CMD_MISC2__INVERT_SNAG;
         else if (this_concept == CMD_MISC2__CENTRAL_MYSTIC)
            this_concept |= CMD_MISC2__INVERT_MYSTIC;
      }

      /* Check that we aren't setting the bit twice. */

      if (ss->cmd.cmd_misc2_flags & this_concept)
         fail("You can't give this concept twice.");

      if (parseptr->concept->arg1 & CMD_MISC2__DO_CENTRAL) {

         // We are having a problem here.  It seems that we need to refrain
         // from forcing the split if 4x4 was given.  The test is [gwv]
         // finally 4x4 central stampede.

         // We do not force the split if the "straight" modifier has been given.
         // In that case the dancers know what they are doing, and that the
         // usual rule that "central" means "do it on each side" is not
         // being followed.  The test is, of course, straight central interlocked little.

         if (attr::slimit(ss) == 7 &&
             ((ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_NXNMASK)) != INHERITFLAGNXNK_4X4) &&
             ((ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_STRAIGHT)) == 0)) {
            ss->cmd.cmd_misc_flags |=
               (ss->rotation & 1) ? CMD_MISC__MUST_SPLIT_VERT : CMD_MISC__MUST_SPLIT_HORIZ;
         }
      }

      ss->cmd.cmd_misc2_flags |= this_concept;
   }

   move(ss, false, result);
}


static void do_concept_crazy(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 finalresultflagsmisc = 0;
   int this_part;

   setup tempsetup = *ss;

   // If we didn't do this check, and we had a 1x4, the "do it on each side"
   // stuff would just do it without splitting or thinking anything was unusual,
   // while the "do it in the center" code would catch it at present, but might
   // not in the future if we add the ability of the concentric schema to mean
   // pick out the center 2 from a 1x4.  In any case, if we didn't do this
   // check, "1/4 reverse crazy bingo" would be legal from a 2x2.

   if (attr::klimit(tempsetup.kind) < 7)
      fail("Need an 8-person setup for this.");

   int reverseness = parseptr->concept->arg1;

   if (tempsetup.cmd.cmd_final_flags.test_heritbit(INHERITFLAG_REVERSE)) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      if (parseptr->concept->arg2) fail("Don't put 'reverse' in front of the fraction.");
      reverseness = 1;
   }

   tempsetup.cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_REVERSE);
   // We don't allow other flags, like "cross", but we do allow "MxN".
   if ((tempsetup.cmd.cmd_final_flags.test_heritbits(~(INHERITFLAG_MXNMASK|
                                                       INHERITFLAG_NXNMASK))) |
       tempsetup.cmd.cmd_final_flags.final)
      fail("Illegal modifier before \"crazy\".");

   // We will modify these flags, and, in any case,
   // we need to rematerialize them at each step.
   setup_command cmd = tempsetup.cmd;

   int craziness = (parseptr->concept->arg2) ? parseptr->options.number_fields : 4;
   uint32 incomingfracs = cmd.cmd_frac_flags & ~CMD_FRAC_THISISLAST;

   if (incomingfracs & CMD_FRAC_REVERSE)
      reverseness ^= (craziness ^ 1);    // That's all it takes!

   int s_numer = (incomingfracs & 0xF000) >> 12;
   int s_denom = (incomingfracs & 0xF00) >> 8;
   int e_numer = (incomingfracs & 0xF0) >> 4;
   int e_denom = (incomingfracs & 0xF);

   int ttt = craziness * s_numer;
   int uuu = craziness * e_numer;
   int i = ttt / s_denom;           // The start point.
   int highlimit = uuu / e_denom;   // The end point.

   if (i*s_denom != ttt || highlimit*e_denom != uuu || i >= highlimit)
      fail("Illegal fraction for \"crazy\".");

   // We have *not* checked for highlimit <= 4.  We allow 5/4 crazy.
   // Use tastefully.

   this_part = (incomingfracs & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT;

   if (this_part > 0) {
      switch (incomingfracs & (CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK)) {
      case CMD_FRAC_CODE_ONLY:
         // Request is to do just part this_part.
         i = this_part-1;
         highlimit = this_part;
         finalresultflagsmisc |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (highlimit == craziness) finalresultflagsmisc |= RESULTFLAG__DID_LAST_PART;
         break;
      case CMD_FRAC_CODE_FROMTO:
         // Request is to do everything up through part this_part.
         highlimit = this_part;
         break;
      case CMD_FRAC_CODE_FROMTOREV:
         // Request is to do everything strictly after part this_part-1.
         i = this_part-1;
         break;
      case CMD_FRAC_CODE_ONLYREV:
         // Request is to do just part this_part, counting from the end.
         i = highlimit-this_part;
         highlimit += 1-this_part;
         finalresultflagsmisc |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (highlimit == craziness) finalresultflagsmisc |= RESULTFLAG__DID_LAST_PART;
         break;
      default:
         // If the "K" field ("part2") is nonzero, maybe we can still do something.
         switch (incomingfracs & CMD_FRAC_CODE_MASK) {
         case CMD_FRAC_CODE_FROMTOREV:
            i = this_part-1;
            highlimit -= (incomingfracs & CMD_FRAC_PART2_MASK) / CMD_FRAC_PART2_BIT;
            break;
         default:
            fail("\"crazy\" is not allowed after this concept.");
         }
      }
   }

   if (highlimit <= i || highlimit > craziness) fail("Illegal fraction for \"crazy\".");

   // No fractions for subject, we have consumed them.
   cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;

   // Lift the craziness restraint from before -- we are about to pull things apart.
   cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;

   for ( ; i<highlimit; i++) {
      tempsetup.cmd = cmd;    // Get a fresh copy of the command.

      update_id_bits(&tempsetup);

      if ((i ^ reverseness) & 1) {
         // Do it in the center.  The -1 for arg 4 makes it
         // preserve roll information for the inactives.
         selective_move(&tempsetup, parseptr, selective_key_plain,
                        -1, 0, 0, selector_center4, false, result);
      }
      else {
         // Do it on each side.
         if (tempsetup.rotation & 1)
            tempsetup.cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_VERT;
         else
            tempsetup.cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;
         move(&tempsetup, false, result);
      }

      finalresultflagsmisc |= result->result_flags.misc;

      tempsetup = *result;
   }

   result->result_flags.misc = finalresultflagsmisc;
}


static void do_concept_phan_crazy(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int i;
   setup tempsetup;
   setup_command cmd;
   setup_kind kk = s4x4;

   int reverseness = (parseptr->concept->arg1 >> 3) & 1;

   if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_REVERSE)) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      reverseness = 1;
   }
   ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_REVERSE);
   // We don't allow other flags, like "cross".
   if (ss->cmd.cmd_final_flags.test_herit_and_final_bits())
      fail("Illegal modifier before \"crazy\".");

   int craziness = (parseptr->concept->arg1 & 16) ?
      parseptr->options.number_fields : 4;

   tempsetup = *ss;

   cmd = tempsetup.cmd;    // We will modify these flags, and, in any case,
                           // we need to rematerialize them at each step.

   uint32 offsetmapcode;
   uint32 specialmapcode = ~0UL;

   phantest_kind phanstuff = phantest_ok;

   // Turn on "verify waves" or whatever, for the first time only.
   tempsetup.cmd.cmd_misc_flags |= parseptr->concept->arg3;

   if (parseptr->concept->arg1 & 64) {
      // This is crazy offset.  A 2x8 setup is reasonable.
      // Unfortunately, there is no concept property flag
      // that calls for a 4x4 or a 2x8.  The flag for this
      // concept just called for a 4x4.  That did no harm, but
      // we have to consider expanding to a 2x8.  If the current
      // setup is not a 4x4, we try to make a 2x8.  If that fails,
      // there will be trouble ahead.
      if (tempsetup.kind != s4x4)
         do_matrix_expansion(&tempsetup, CONCPROP__NEEDK_2X8, false);
   }

   int rot = tempsetup.rotation & 1;
   int spec_conc_rot = 1;

   // Decide which way we are going to divide, once only.

   if ((parseptr->concept->arg1 & 7) < 4) {
      // This is {crazy phantom / crazy offset} C/L/W.  64 bit tells which.

      if ((global_tbonetest & 011) == 011) fail("People are T-boned -- try using 'standard'.");

      if (tempsetup.kind == s4x4) {
         rot = (global_tbonetest ^ parseptr->concept->arg1) & 1;
      }
      else
         kk = s2x8;

      // We have now processed any "standard" information to determine how to split the setup.
      // If this was done nontrivially, we now have to shut off the test that we will do in the
      // divided setup -- that test would fail.  Also, we do not allow "waves", only "lines" or
      // "columns".

      if ((orig_tbonetest & 011) == 011) {
         tempsetup.cmd.cmd_misc_flags &= ~CMD_MISC__VERIFY_MASK;
         if ((parseptr->concept->arg1 & 7) == 3)
            fail("Don't use 'crazy waves' with standard; use 'crazy lines'.");
      }

      if (parseptr->concept->arg1 & 64) {
         // Crazy offset C/L/W.
         phanstuff = phantest_only_one_pair;
         specialmapcode = MAPCODE(s1x4,2,MPKIND__OFFS_BOTH_SINGLEV,1);

         // Look for the case of crazy offset CLW in a 2x8.
         if (tempsetup.kind != s4x4)
            offsetmapcode = MAPCODE(s1x4,4,MPKIND__OFFS_BOTH_SINGLEV,0);
         else
            offsetmapcode = MAPCODE(s1x4,4,MPKIND__OFFS_BOTH_SINGLEH,1);
      }
      else {
         // Crazy phantom C/L/W.  We will use a map that stacks the setups vertically.
         offsetmapcode = MAPCODE(s2x4,2,MPKIND__SPLIT,1);
         spec_conc_rot = 0;   // Undo the effect of this for the center CLW.
         rot++;
      }
   }
   else if ((parseptr->concept->arg1 & 7) == 4) {
      // This is {crazy phantom / crazy diagonal} boxes.  64 bit tells which.

      if (parseptr->concept->arg1 & 64) {   // Crazy diagonal boxes.
         specialmapcode = MAPCODE(s2x2,2,MPKIND__OFFS_BOTH_SINGLEV,0);

         if (tempsetup.kind != s4x4) {
            kk = s2x8;
            offsetmapcode = MAPCODE(s2x2,4,MPKIND__OFFS_BOTH_SINGLEH,0);
         }
         else {
            // We do an incredibly simple test of which way the 4x4 is oriented.
            // A rigorous test of the exact occupation will be made later.
            rot = ((global_livemask == 0x3A3A) || (global_livemask == 0xC5C5)) ?
               1 : 0;
            offsetmapcode = MAPCODE(s2x2,4,MPKIND__OFFS_BOTH_SINGLEV,0);
         }
      }
      else {
         kk = s2x8;
         offsetmapcode = MAPCODE(s2x4,2,MPKIND__SPLIT,0);
      }
   }
   else {
      kk = s4dmd;
      offsetmapcode = MAPCODE(s_qtag,2,MPKIND__SPLIT,0);
   }

   uint32 finalresultflagsmisc = 0;

   // This setup rotation stuff is complicated.  What we do may be
   // different for "each side" and "centers".

   // Flip the setup around and recanonicalize.
   // We have to do it in pieces like this, because of the test
   // of tempsetup.rotation that will happen a few lines below.
   tempsetup.rotation += rot;

   for (i=0 ; i<craziness; i++) {
      int ctrflag = (i ^ reverseness) & 1;
      canonicalize_rotation(&tempsetup);

      // Check the validity of the setup each time for boxes/diamonds,
      // or first time only for C/L/W.
      if ((i==0 || ((parseptr->concept->arg1 & 7) >= 4 &&
                    !(parseptr->concept->arg1 & 64))) &&
          (tempsetup.kind != kk || tempsetup.rotation != 0))
         fail("Can't do crazy phantom or offset in this setup.");

      if (ctrflag) {
         // Do it in the center.
         // Do special check for crazy offset.
         tempsetup.rotation += spec_conc_rot;
         canonicalize_rotation(&tempsetup);
         if ((parseptr->concept->arg1 & 64) &&
             tempsetup.kind != s2x8 && tempsetup.kind != s4x4)
            fail("Can't do crazy offset with this shape-changer.");
         concentric_move(&tempsetup, (setup_command *) 0, &tempsetup.cmd,
                         schema_in_out_quad, 0, 0, true, specialmapcode, result);
         result->rotation -= spec_conc_rot;
      }
      else                              // Do it on each side.
         divided_setup_move(&tempsetup, offsetmapcode, phanstuff, true, result);

      finalresultflagsmisc |= result->result_flags.misc;
      tempsetup = *result;
      tempsetup.cmd = cmd;    // Get a fresh copy of the command for next time.
   }

   // Flip the setup back.  No need to canonicalize.
   result->rotation -= rot;
   // The split-axis bits are gone.  If someone needs them, we have work to do.
   result->result_flags.misc = finalresultflagsmisc;
   result->result_flags.clear_split_info();
}



static void do_concept_fan(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   setup tempsetup;
   uint32 finalresultflagsmisc = 0;
   /* This is a huge amount of kludgy stuff shoveled in from a variety of sources.
      It needs to be cleaned up and thought about. */

   final_and_herit_flags new_final_concepts;
   new_final_concepts.clear_all_herit_and_final_bits();
   const parse_block *parseptrcopy;
   call_with_name *callspec;

   parseptrcopy = process_final_concepts(parseptr->next, true, &new_final_concepts, true, __FILE__, __LINE__);

   if (new_final_concepts.test_herit_and_final_bits() ||
       parseptrcopy->concept->kind > marker_end_of_list)
      fail("Can't do \"fan\" followed by another concept or modifier.");

   callspec = parseptrcopy->call;

   if (!callspec || !(callspec->the_defn.callflagsf & CFLAG2_CAN_BE_FAN))
      fail("Can't do \"fan\" with this call.");

   // Step to a wave if necessary.  This is actually only needed for the "yoyo" concept.
   // The "fan" concept could take care of itself later.  However, we do them both here.

   if (!(ss->cmd.cmd_misc_flags & (CMD_MISC__NO_STEP_TO_WAVE | CMD_MISC__ALREADY_STEPPED)) &&
       (callspec->the_defn.callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_STEP_TO_WAVE) {
      ss->cmd.cmd_misc_flags |= CMD_MISC__ALREADY_STEPPED;  // Can only do it once.
      touch_or_rear_back(ss, false, callspec->the_defn.callflags1);
   }

   tempsetup = *ss;

   // Normally, set the fractionalize field to start with the second part.
   // But if we have been requested to do a specific part number of "fan <call>",
   // just add one to the part number and do the call.

   tempsetup.cmd = ss->cmd;

   if (tempsetup.cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE)
      tempsetup.cmd.cmd_frac_flags =
         CMD_FRAC_CODE_FROMTOREV | (CMD_FRAC_PART_BIT*2) | CMD_FRAC_NULL_VALUE;
   else if ((tempsetup.cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK | CMD_FRAC_REVERSE | 0xFFFF)) ==
       CMD_FRAC_NULL_VALUE)
      tempsetup.cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
   else
      fail("Sorry, this concept can't be fractionalized this way.");

   tempsetup.cmd.prior_elongation_bits = 0;
   tempsetup.cmd.prior_expire_bits = 0;
   move(&tempsetup, false, result);
   finalresultflagsmisc |= result->result_flags.misc;
   result->result_flags.misc = finalresultflagsmisc & ~3;
}

void stable_move(
   setup *ss,
   bool fractional,
   bool everyone,
   int howfar,
   selector_kind who,
   setup *result) THROW_DECL
{
   if (fractional && howfar > 4)
      fail("Can't do fractional stable more than 4/4.");

   selector_kind saved_selector = current_options.who;
   current_options.who = who;

   int n = attr::slimit(ss);
   if (n < 0) fail("Sorry, can't do stable starting in this setup.");

   uint32 directions[32];
   bool selected[32];

   int i;
   for (i=0; i<=n; i++) {           // Save current facing directions.
      uint32 p = ss->people[i].id1;
      if (p & BIT_PERSON) {
         directions[(p >> 6) & 037] = p;
         selected[(p >> 6) & 037] = everyone || selectp(ss, i);
         if (fractional) {
            if (p & STABLE_MASK)
               fail("Sorry, can't nest fractional stable/twosome.");
            ss->people[i].id1 |= STABLE_ENAB | (STABLE_RBIT * howfar);
         }
      }
   }

   current_options.who = saved_selector;

   int orig_rotation = ss->rotation;
   move(ss, false, result);
   int rot = ((orig_rotation - result->rotation) & 3) * 011;

   setup_kind kk = result->kind;

   if (kk == s_dead_concentric)
      kk = result->inner.skind;

   n = attr::klimit(kk);
   if (n < 0) fail("Sorry, can't do stable going to this setup.");

   for (i=0; i<=n; i++) {      // Restore facing directions of selected people.
      uint32 p = result->people[i].id1;
      if (p & BIT_PERSON) {
         if (selected[(p >> 6) & 037]) {
            uint32 stop_amount;
            if (fractional) {
               stop_amount = (p & (STABLE_VBIT*3)) / STABLE_VBIT;

               if (!(p & STABLE_ENAB))
                  fail("fractional stable not supported for this call.");
               p = rotperson(p, ((0 - stop_amount) & 3) * 011);
            }
            else {
               stop_amount = 1;   // So that roll will be turned off.

               p = rotperson(
                  (p & ~(d_mask | STABLE_MASK)) |
                  (directions[(p >> 6) & 037] & (d_mask | STABLE_MASK)),
                  rot);
            }

            // If this was fractional stable, roll is turned off
            // only if the person was actually stopped from turning.
            if (stop_amount) p = (p & ~ROLL_DIRMASK) | ROLL_IS_M;
         }

         if (fractional) p &= ~STABLE_MASK;

         result->people[i].id1 = p;
      }
   }
}


static void do_concept_stable(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   stable_move(ss,
               parseptr->concept->arg2 != 0,
               parseptr->concept->arg1 == 0,
               parseptr->options.number_fields,
               parseptr->options.who,
               result);
}



static void do_concept_emulate(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int i, j, n, m, rot;
   setup res1;

   setup a1 = *ss;

   move(&a1, false, &res1);

   *result = res1;      // Get all the other fields.

   result->result_flags.misc |= RESULTFLAG__FORCE_SPOTS_ALWAYS;
   result->kind = ss->kind;
   result->rotation = ss->rotation;

   n = attr::slimit(ss);
   m = attr::klimit(res1.kind);
   if (n < 0 || m < 0) fail("Sorry, can't do emulate in this setup.");

   rot = ((res1.rotation-result->rotation) & 3) * 011;

   for (i=0; i<=n; i++) {
      uint32 p = copy_person(result, i, ss, i);

      if (p & BIT_PERSON) {
         for (j=0; j<=m; j++) {
            uint32 q = res1.people[j].id1;
            if ((q & BIT_PERSON) && ((q ^ p) & XPID_MASK) == 0) {
               result->people[i].id1 &= ~(NROLL_MASK | STABLE_MASK | 0x3F);
               result->people[i].id1 |= rotperson(q, rot) & (NROLL_MASK | STABLE_MASK | 0x3F);
               goto did_it;
            }
         }
         fail("Lost someone during emulate call.");
         did_it: ;
      }
   }
}


static void do_concept_checkerboard(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // arg1 = setup (1x4/2x2/dmd)
   // arg2 = 0 : checkersetup
   //        1 : shadow setup
   //        2 : orbitsetup
   //        3 : twin orbitsetup
   //        4 : left orbitsetup
   //        add 8 for selected people (checker only)

   struct CKBDtabletype {
      setup_kind K;
      uint32 L;   // the actual livemask must not intersect this
                  // (only to pick out two types of C1 phantoms.)
      uint32 M;   // 11 means direction is required (and person must be live);
                  // 00 means direction is forbidden.
      uint32 V;   // what the required/forbidden direction is.
      uint32 dup; // Direction of 1st 2 trading people, typically north
      uint32 ddn; // Direction of last 2 trading people, typically south
      uint32 rot; // People are rotated.
      veryshort mape[4];   // The people who will trade.
      veryshort mapl[4];   // Where the others go, checkerboard.
      veryshort mapb[4];   // Where the others go, checkerbox.
      veryshort mapd[4];   // Where the others go, checkerdiamond.
   };

   static const CKBDtabletype CKBDtable[] = {
      {s2x4, 0,              0xCCCC,     0x00AA,      // outfacers are as if in RWV
       d_north, d_south, 0, {0, 2, 4, 6}, {7, 1, 3, 5}, {1, 3, 5, 7}, {7, 1, 3, 5}},
      {s2x4, 0,              0x3333,     0x00AA,      // outfacers are as if in LWV
       d_north, d_south, 0, {1, 3, 5, 7}, {0, 6, 4, 2}, {0, 2, 4, 6}, {0, 2, 4, 6}},
      {s2x4, 0,              0xF0F0,     0x00AA,      // outfacers are as if in R2FL
       d_north, d_south, 0, {0, 1, 4, 5}, {7, 6, 3, 2}, {2, 3, 6, 7}, {7, 2, 3, 6}},
      {s2x4, 0,              0x0F0F,     0x00AA,      // outfacers are as if in L2FL
       d_north, d_south, 0, {2, 3, 6, 7}, {0, 1, 4, 5}, {0, 1, 4, 5}, {0, 1, 4, 5}},
      {s2x4, 0,              0xC3C3,     0x00AA,      // outfacers are ends
       d_north, d_south, 0, {0, 3, 4, 7}, {-1, -1, -1, -1}, {1, 2, 5, 6}, {-1, -1, -1, -1}},
      {s_c1phan, 0x33333333, 0xCC00CC00, 0x004488CC,  // C1 phantoms, outfacers N/S
       d_north, d_south, 033, {0, 2, 8, 10}, {4, 6, 12, 14}, {4, 6, 12, 14}, {4, 6, 12, 14}},
      {s_c1phan, 0x33333333, 0x00CC00CC, 0x004488CC,  // C1 phantoms, outfacers E/W
       d_east, d_west, 0, {4, 6, 12, 14}, {0, 2, 8, 10}, {0, 2, 8, 10}, {0, 2, 8, 10}},
      {s_c1phan, 0xCCCCCCCC, 0x00330033, 0x33001122,  // other C1 phantoms, N/S
       d_north, d_south, 033, {7, 5, 15, 13}, {1, 3, 9, 11}, {11, 9, 3, 1}, {1, 11, 9, 3}},
      {s_c1phan, 0xCCCCCCCC, 0x33003300, 0x33001122,  // other C1 phantoms, E/W
       d_west, d_east, 0, {3, 1, 11, 9}, {13, 15, 5, 7}, {7, 5, 15, 13}, {13, 7, 5, 15}},
      {nothing}};

   int i;
   int offset = -1;
   setup_kind kn = (setup_kind) parseptr->concept->arg1;

   clear_people(result);

   if (parseptr->concept->arg2 == 1) {
      // This is "shadow <setup>"

      setup_command subsid_cmd;

      if ((kn != s2x2 || ss->kind != s2x4) &&
          (kn != s1x4 || (ss->kind != s_qtag && ss->kind != s_bone)) &&
          (kn != sdmd || (ss->kind != s_hrglass && ss->kind != s_dhrglass)))
         fail("Not in correct setup for 'shadow line/box/diamond' concept.");

      subsid_cmd = ss->cmd;
      subsid_cmd.parseptr = (parse_block *) 0;
      subsid_cmd.callspec = base_calls[base_call_ends_shadow];
      subsid_cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
      subsid_cmd.cmd_final_flags.clear_all_herit_and_final_bits();
      concentric_move(ss, &ss->cmd, &subsid_cmd, schema_concentric, 0,
                      DFM1_CONC_DEMAND_LINES | DFM1_CONC_FORCE_COLUMNS, true, ~0UL, result);
      return;
   }

   // This is "checker/orbit <setup>"

   // First, we allow these to be done in a C1 phantom setup under certain
   // conditions.  If it's the equivalent "pinwheel" 4x4, fix it.

   if (ss->kind == s4x4) {
      uint32 mask = little_endian_live_mask(ss);

      if (mask == 0xAAAA)
         expand::compress_setup(&s_c1phan_4x4a, ss);
      else if (mask == 0xCCCC)
         expand::compress_setup(&s_c1phan_4x4b, ss);
   }

   if (parseptr->concept->arg2 & 8) {
      // This is "so-and-so preferred for the trade, checkerboard".
      if (ss->kind != s2x4) fail("Must have a 2x4 setup for 'checker' concept.");

      if      (global_selectmask == 0x55)
         offset = 0;
      else if (global_selectmask == 0xAA)
         offset = 1;
      else if (global_selectmask == 0x33)
         offset = 2;
      else if (global_selectmask == 0xCC)
         offset = 3;
      else if (global_selectmask == 0x99)
         offset = 4;
      else fail("Can't select these people.");
   }
   else {
      uint32 D, L;
      get_directions(ss, D, L);

      for (i=0; CKBDtable[i].K != nothing ; i++) {
         if (CKBDtable[i].K != ss->kind ||
             CKBDtable[i].L & L) continue;
         uint32 M = CKBDtable[i].M;
         // Find out who is facing out for the trade, by XOR'ing the person's
         // direction (in D) with the required direction from the table
         // (the V field).  The result will be 00 for people facing out.
         // If the spot is unoccupied, set the bits to 11 by OR'ing
         // the complement of L.  That way, such a phantom will be
         // considered not to be facing out for the trade.
         // (That means that people trading must be real.  We will
         // allow exceptions to this below, where we check for
         // "assume waves".)
         uint32 Q = ~L | (CKBDtable[i].V ^ D);

         // Now "M" is 11 for people who must be facing out for the trade,
         // and 00 for the other people, who are therefore forbidden to
         // be facing out.  The first part of the next conditional tests
         // that the M=11 spots all have Q=00, which means that they are
         // live and facing out.  The second part tests that the M=00
         // spots all have Q != 00, which means that they are phantoms
         // or are facing some other way.
         if ((M & Q) == 0 &&
             (((Q & 0x55555555) << 1) | Q | M | 0x55555555) == ~0UL) {
            offset = i;
            break;
         }
      }

      // If we don't have the required outfacers, but a suitable
      // "assume" command was given, that's good enough.

      if (offset < 0 &&
          ss->kind == s2x4 &&
          ss->cmd.cmd_assume.assumption == cr_wave_only &&
          ss->cmd.cmd_assume.assump_col == 0 &&
          ss->cmd.cmd_assume.assump_negate == 0) {
         if (((L & ~D) | 0xAAAAAAAA) != 0xAAAAAAAA) {
            uint32 DR = D ^ 0x8822;
            uint32 DL = D ^ 0x2288;
            if (((DR | ~L | ((DR & 0x55555555) << 1)) & 0xAAAAAAAA) == 0xAAAAAAAA)
               offset = 0;
            else if (((DL | ~L | ((DL & 0x55555555) << 1)) & 0xAAAAAAAA) == 0xAAAAAAAA)
               offset = 1;
         }
      }
   }

   if (offset < 0)
      fail("Can't identify checkerboard people.");

   const CKBDtabletype *CKBDthing = &CKBDtable[offset];

   const veryshort *mapeptr = CKBDthing->mape;

   const veryshort *map_ptr =
      (kn == s1x4) ? CKBDthing->mapl :
      (kn == s2x2) ? CKBDthing->mapb :
      CKBDthing->mapd;

   // If we have inverted lines with the ends facing out,
   // it is only legal with checkerbox.
   if (*map_ptr < 0) fail("Can't identify checkerboard people.");

   uint32 t;

   if (((t = ss->people[mapeptr[0]].id1) && (t & d_mask) != CKBDthing->dup) ||
       ((t = ss->people[mapeptr[1]].id1) && (t & d_mask) != CKBDthing->dup) ||
       ((t = ss->people[mapeptr[2]].id1) && (t & d_mask) != CKBDthing->ddn) ||
       ((t = ss->people[mapeptr[3]].id1) && (t & d_mask) != CKBDthing->ddn))
      fail("Selected people are not facing out.");

   if ((parseptr->concept->arg2 & 7) == 2) {
      // orbitboard
      if (offset > 1) fail("Can't find orbiting people.");

      // Move the people who simply orbit, filling in their roll info.

      copy_rot(result, mapeptr[2], ss, mapeptr[1], 0);
      if (result->people[mapeptr[2]].id1)
         result->people[mapeptr[2]].id1 = 
            (result->people[mapeptr[2]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
      copy_rot(result, mapeptr[1], ss, mapeptr[0], 022);
      if (result->people[mapeptr[1]].id1)
         result->people[mapeptr[1]].id1 =
            (result->people[mapeptr[1]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
      copy_rot(result, mapeptr[0], ss, mapeptr[3], 0);
      if (result->people[mapeptr[0]].id1)
         result->people[mapeptr[0]].id1 =
            (result->people[mapeptr[0]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
      copy_rot(result, mapeptr[3], ss, mapeptr[2], 022);
      if (result->people[mapeptr[3]].id1)
         result->people[mapeptr[3]].id1 =
            (result->people[mapeptr[3]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
   }
   else if ((parseptr->concept->arg2 & 7) == 4) {
      // left orbitboard
      if (offset > 1) fail("Can't find orbiting people.");

      // Move the people who simply orbit, filling in their roll info.

      copy_rot(result, mapeptr[2], ss, mapeptr[3], 022);
      if (result->people[mapeptr[2]].id1)
         result->people[mapeptr[2]].id1 = 
            (result->people[mapeptr[2]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
      copy_rot(result, mapeptr[1], ss, mapeptr[2], 0);
      if (result->people[mapeptr[1]].id1)
         result->people[mapeptr[1]].id1 =
            (result->people[mapeptr[1]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
      copy_rot(result, mapeptr[0], ss, mapeptr[1], 022);
      if (result->people[mapeptr[0]].id1)
         result->people[mapeptr[0]].id1 =
            (result->people[mapeptr[0]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
      copy_rot(result, mapeptr[3], ss, mapeptr[0], 0);
      if (result->people[mapeptr[3]].id1)
         result->people[mapeptr[3]].id1 =
            (result->people[mapeptr[3]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
   }
   else if ((parseptr->concept->arg2 & 7) == 3) {
      // twin orbitboard
      if (offset > 1) fail("Can't find orbiting people.");

      // Move the people who simply orbit, filling in their roll info.

      copy_rot(result, mapeptr[2], ss, mapeptr[1], 0);
      if (result->people[mapeptr[2]].id1)
         result->people[mapeptr[2]].id1 =
            (result->people[mapeptr[2]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
      copy_rot(result, mapeptr[3], ss, mapeptr[0], 0);
      if (result->people[mapeptr[3]].id1)
         result->people[mapeptr[3]].id1 =
            (result->people[mapeptr[3]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
      copy_rot(result, mapeptr[0], ss, mapeptr[3], 0);
      if (result->people[mapeptr[0]].id1)
         result->people[mapeptr[0]].id1 =
            (result->people[mapeptr[0]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
      copy_rot(result, mapeptr[1], ss, mapeptr[2], 0);
      if (result->people[mapeptr[1]].id1)
         result->people[mapeptr[1]].id1 =
            (result->people[mapeptr[1]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
   }
   else {
      // checkerboard/box/diamond

      // Move the people who simply trade, filling in their roll info.

      copy_rot(result, mapeptr[0], ss, mapeptr[1], 022);
      if (result->people[mapeptr[0]].id1)
         result->people[mapeptr[0]].id1 =
            (result->people[mapeptr[0]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
      copy_rot(result, mapeptr[1], ss, mapeptr[0], 022);
      if (result->people[mapeptr[1]].id1)
         result->people[mapeptr[1]].id1 =
            (result->people[mapeptr[1]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
      copy_rot(result, mapeptr[2], ss, mapeptr[3], 022);
      if (result->people[mapeptr[2]].id1)
         result->people[mapeptr[2]].id1 =
            (result->people[mapeptr[2]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_L;
      copy_rot(result, mapeptr[3], ss, mapeptr[2], 022);
      if (result->people[mapeptr[3]].id1)
         result->people[mapeptr[3]].id1 =
            (result->people[mapeptr[3]].id1 & (~NROLL_MASK)) | PERSON_MOVED | ROLL_IS_R;
   }

   setup a1 = *ss;
   for (i=0; i<4; i++) copy_rot(&a1, i, ss, map_ptr[i], CKBDthing->rot);

   a1.kind = kn;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a1.rotation = 0;
   a1.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a1);
   setup res1;
   move(&a1, false, &res1);

   // Look at the rotation coming out of the move.  If the setup is
   // 1x4, we require it to be even (checkerboard lockit not allowed).
   // Otherwise, allow any rotation.  But we give a warning for
   // peculiarly oriented diamonds.
   if (res1.rotation != 0) {
      if (res1.kind == s1x4)
         fail("'Checker' call went to 1x4 setup oriented the wrong way.");
      if (res1.kind == sdmd)
         warn(warn_controversial);
   }

   int rot = ((res1.rotation - CKBDthing->rot) & 3) * 011;

   map_ptr =
      (res1.kind == s1x4) ? CKBDthing->mapl :
      (res1.kind == s2x2) ? CKBDthing->mapb :
      (res1.kind == sdmd) ? CKBDthing->mapd : 0;

   if (!map_ptr) fail("Don't recognize ending setup after 'checker' call.");

   if (*map_ptr < 0) fail("Can't do this.");

   for (i=0; i<4; i++) copy_rot(result, map_ptr[i], &res1, (i-res1.rotation)&3, rot);

   result->kind = ss->kind;
   result->rotation = 0;
   result->result_flags = res1.result_flags;
   reinstate_rotation(ss, result);
}


static void do_concept_checkpoint(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int reverseness = parseptr->concept->arg1;

   if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_REVERSE)) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      reverseness = 1;
   }

   ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_REVERSE);
   // We don't allow other flags, like "cross".
   if (ss->cmd.cmd_final_flags.test_herit_and_final_bits())
      fail("Illegal modifier before \"checkpoint\".");

   setup_command this_cmd = ss->cmd;

   if ((this_cmd.cmd_misc_flags & (CMD_MISC__PUT_FRAC_ON_FIRST|CMD_MISC__RESTRAIN_CRAZINESS)) ==
       CMD_MISC__PUT_FRAC_ON_FIRST) {
      
      // Curried meta-concept, as in "finally checkpoint recycle
      // by 1/4 thru".  Take the fraction info off the first
      // call.  In this example, the 1/4 thru is affected but the
      // recycle is not.
      this_cmd.cmd_misc_flags &= ~CMD_MISC__PUT_FRAC_ON_FIRST;
      this_cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
   }
   else {
      // If not under a meta-concept, we don't allow fractionalization.
      // You can't do 1/2 of the moving of the checkpointers to the outside.
      if (this_cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't do this.");
   }

   setup_command subsid_cmd = ss->cmd;
   subsid_cmd.parseptr = parseptr->subsidiary_root;

   // The "dfm_conc_force_otherway" flag forces Callerlab interpretation:
   // If checkpointers go from 2x2 to 2x2, this is clear.
   // If checkpointers go from 1x4 to 2x2, "dfm_conc_force_otherway" forces
   //    the Callerlab rule in preference to the "parallel_concentric_end" property
   //    on the call.

   if (reverseness)
      concentric_move(ss, &this_cmd, &subsid_cmd, schema_rev_checkpoint,
                      0, 0, true, ~0UL, result);
   else
      concentric_move(ss, &subsid_cmd, &this_cmd, schema_checkpoint,
                      0, DFM1_CONC_FORCE_OTHERWAY, true, ~0UL, result);
}


static void copy_cmd_preserve_elong_and_expire(const setup *ss, setup *result, bool only_elong = false)
{
   uint32 save_elongation = result->cmd.prior_elongation_bits;
   uint32 save_expire = result->cmd.prior_expire_bits;
   result->cmd = ss->cmd;
   result->cmd.prior_elongation_bits = save_elongation;
   if (!only_elong) result->cmd.prior_expire_bits = save_expire;
}


static bool prepare_for_call_under_repetition(
   const fraction_info *yyy,
   int & fetch_number,
   setup *ss,
   setup *result) THROW_DECL
{
   if (yyy->m_reverse_order) {
      if (yyy->m_fetch_index < 0) return true;
   }
   else {
      if (yyy->m_fetch_index >= yyy->m_fetch_total) return true;
   }

   yyy->demand_this_part_exists();

   copy_cmd_preserve_elong_and_expire(ss, result);

   if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
      update_id_bits(result);

   // We don't supply these; they get filled in by the call.
   result->cmd.cmd_misc_flags &= ~DFM1_CONCENTRICITY_FLAG_MASK;
   if (!yyy->m_first_call) {
      result->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;
      result->cmd.cmd_assume.assumption = cr_none;
   }

   // Move this stuff out, after the increment.

   if (yyy->m_do_half_of_last_part != 0 &&
       yyy->m_fetch_index+1 == yyy->m_highlimit)
      result->cmd.cmd_frac_flags = yyy->m_do_half_of_last_part;
   else if (result->cmd.restrained_fraction) {
      result->cmd.cmd_frac_flags = result->cmd.restrained_fraction;
      result->cmd.restrained_fraction = 0;
   }
   else
      result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;  // No fractions to constituent call.

   fetch_number = yyy->m_fetch_index;
   return false;
}


static void do_call_in_series_simple(setup *result) THROW_DECL
{
   do_call_in_series(result, false, false, true, false);
}


static void do_concept_sequential(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   fraction_info zzz(2);

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE) {
      zzz.get_fraction_info(ss->cmd.cmd_frac_flags,
                            3*CFLAG1_VISIBLE_FRACTION_BIT,
                            weirdness_off);
   }

   zzz.m_first_call = !zzz.m_reverse_order;

   prepare_for_call_in_series(result, ss);

   for (;;) {
      int fetch_number;

      if (prepare_for_call_under_repetition(&zzz, fetch_number, ss, result)) break;
      if (zzz.not_yet_in_active_section()) goto go_to_next_cycle;
      if (zzz.ran_off_active_section()) break;

      // The fetch number is 0 or 1.  Depending on which it is, get the proper parse pointer.
      if (fetch_number != 0) result->cmd.parseptr = parseptr->subsidiary_root;

      do_call_in_series_simple(result);
      zzz.m_first_call = false;

      // If we are being asked to do just one part of a call,
      // exit now.  Also, fill in bits in result->result_flags.

      if (zzz.query_instant_stop(result->result_flags.misc)) break;

   go_to_next_cycle:

      // Increment for next cycle.
      zzz.m_fetch_index += zzz.m_subcall_incr;
      zzz.m_client_index += zzz.m_subcall_incr;
   }
}



static void do_concept_special_sequential(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // Values of arg1 are:
   //    0 - follow it by (call)
   //    1 - precede it by (call)
   //    2 - start with (call)
   //    3 - use (call) for <Nth> part
   //    4 - use (call) in
   //    5 - half and half

   if (parseptr->concept->arg1 == 5) {
      // This is "half and half".

      fraction_info zzz(2);

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE) {
         zzz.get_fraction_info(ss->cmd.cmd_frac_flags,
                               3*CFLAG1_VISIBLE_FRACTION_BIT,
                               weirdness_off);
      }

      zzz.m_first_call = !zzz.m_reverse_order;

      prepare_for_call_in_series(result, ss);

      for (;;) {
         int fetch_number;
         uint32 corefracs;

         if (prepare_for_call_under_repetition(&zzz, fetch_number, ss, result)) break;
         if (zzz.not_yet_in_active_section()) goto go_to_next_cycle;
         if (zzz.ran_off_active_section()) break;

         corefracs = zzz.get_fracs_for_this_part();

         // The fetch number is 0 or 1.  Depending on which it is,
         // get the proper parse pointer.
         if (fetch_number == 0) {
            // Doing the first call.  Apply the fraction "first 1/2".
            result->cmd.cmd_frac_flags =
               process_spectacularly_new_fractions(0, 1, 1, 2, corefracs);
         }
         else {
            // Doing the second call.  Apply the fraction "last 1/2".
            result->cmd.cmd_frac_flags =
               process_spectacularly_new_fractions(1, 2, 1, 1, corefracs);
            result->cmd.parseptr = parseptr->subsidiary_root;
         }

         do_call_in_series_simple(result);
         zzz.m_first_call = false;

         // If we are being asked to do just one part of a call,
         // exit now.  Also, fill in bits in result->result_flags.

         if (zzz.query_instant_stop(result->result_flags.misc)) break;

      go_to_next_cycle:

         // Increment for next cycle.
         zzz.m_fetch_index += zzz.m_subcall_incr;
         zzz.m_client_index += zzz.m_subcall_incr;
      }
   }
   else if (parseptr->concept->arg1 == 3) {
      // This is "use (call) for <Nth> part", which is the same as "replace the <Nth> part".

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      prepare_for_call_in_series(result, ss);
      int stopindex = parseptr->options.number_fields;

      // Do the early part, if any of the main call.

      if (stopindex > 1) {
         copy_cmd_preserve_elong_and_expire(ss, result);
         result->cmd.prior_expire_bits |= RESULTFLAG__EXPIRATION_ENAB;
         result->cmd.parseptr = parseptr->subsidiary_root;
         result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_FROMTO,stopindex-1,0) |
            CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         result->cmd.prior_expire_bits |= result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
         do_call_in_series(result, true, false, true, false);
      }

      // Do the replacement call.

      result->cmd = ss->cmd;
      if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
         update_id_bits(result);    // So you can interrupt with "leads run", etc.
      result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;

      // Give this call a clean start with respect to expiration stuff.
      uint32 suspended_expiration_bits = result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
      result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;
      do_call_in_series(result, true, false, true, false);

      // Put back the expiration bits for the resumed call.
      result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;
      result->result_flags.misc |= suspended_expiration_bits|RESULTFLAG__EXPIRATION_ENAB;

      // Do the remainder of the main call, if there is more.

      copy_cmd_preserve_elong_and_expire(ss, result);
      result->cmd.parseptr = parseptr->subsidiary_root;
      result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_FROMTOREV,stopindex+1,0) |
         CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
      result->cmd.prior_expire_bits |= result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
      do_call_in_series(result, true, false, true, false);
   }
   else if (parseptr->concept->arg1 == 2) {
      // This is "start with (call)", which is the same as "replace the 1st part".

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      setup tttt;
      uint32 finalresultflagsmisc = 0;

      // Do the special first part.

      tttt = *ss;
      if (!(tttt.result_flags.misc & RESULTFLAG__NO_REEVALUATE))
         update_id_bits(&tttt);           /* So you can use "leads run", etc. */
      move(&tttt, false, result);
      finalresultflagsmisc |= result->result_flags.misc;
      normalize_setup(result, simple_normalize, false);

      // Do the rest of the original call, if there is more.

      tttt = *result;
      // Skip over the concept.
      tttt.cmd = ss->cmd;
      tttt.cmd.parseptr = parseptr->subsidiary_root;
      tttt.cmd.cmd_frac_flags =
         CMD_FRAC_CODE_FROMTOREV |
         (CMD_FRAC_PART_BIT*2) |
         CMD_FRAC_NULL_VALUE;
      move(&tttt, false, result);
      finalresultflagsmisc |= result->result_flags.misc;
      normalize_setup(result, simple_normalize, false);

      result->result_flags.misc = finalresultflagsmisc & ~3;
   }
   else {
      // This is replace with (4), follow with (0) or precede with (1).

      // We allow fractionalization commands only for the special case of "piecewise"
      // or "random", in which case we will apply them to the first call only.

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE &&
          !(ss->cmd.cmd_misc_flags & CMD_MISC__PUT_FRAC_ON_FIRST))
         fail("Can't stack meta or fractional concepts.");

      prepare_for_call_in_series(result, ss);

      if (parseptr->concept->arg1 == 4) {
         // Replace with this call.

         // We need to do a "test execution" of the thing being replaced, to find out
         // whether we are at the last part.

         copy_cmd_preserve_elong_and_expire(ss, result);
         result->cmd.parseptr = parseptr->subsidiary_root;
         result->cmd.cmd_misc2_flags |= CMD_MISC2__DO_NOT_EXECUTE;
         do_call_in_series_simple(result);
         uint32 saved_last_flagmisc = result->result_flags.misc &
            (RESULTFLAG__DID_LAST_PART|RESULTFLAG__PARTS_ARE_KNOWN);  // The info we want.

         // Now do the actual replacement call.  It gets no fractions.
         prepare_for_call_in_series(result, ss);
         result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
         do_call_in_series_simple(result);
         result->result_flags.misc &= ~RESULTFLAG__PART_COMPLETION_BITS;
         result->result_flags.misc |= saved_last_flagmisc;
      }
      else {
         // Follow (0) or precede (1).

         for (int call_index=0; call_index<2; call_index++) {
            copy_cmd_preserve_elong_and_expire(ss, result);

            if ((call_index ^ parseptr->concept->arg1) != 0) {
               // The interloper call.  It gets no fractions.
               result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
               uint32 saved_last_flagmisc = result->result_flags.misc &
                  (RESULTFLAG__DID_LAST_PART|RESULTFLAG__PARTS_ARE_KNOWN);
               do_call_in_series_simple(result);
               result->result_flags.misc &= ~RESULTFLAG__PART_COMPLETION_BITS;
               result->result_flags.misc |= saved_last_flagmisc;
            }
            else {
               // The base call, or the part of it.
               result->cmd.parseptr = parseptr->subsidiary_root;
               do_call_in_series_simple(result);
            }
         }
      }
   }
}



static void do_concept_n_times(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   /* This includes "twice", "thrice", and "N times", which means it can either take a hard-wired
      number or a number from the current options.
      arg1 = 0 :  number of repetitions is hardwired and is in arg2.
      arg1 = 1 :  number of repetitions was specified by user. */

   // Lift the craziness restraint from before -- we are about to pull things apart.
   ss->cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;

   fraction_info zzz((parseptr->concept->arg1) ?
                     parseptr->options.number_fields :
                     parseptr->concept->arg2);
   setup offline_split_info;

   // We do not accumulate the splitting information as the calls progress.
   // Instead, we accumulate it into an offline setup, and put the final
   // splitting info into the result at the end.

   prepare_for_call_in_series(&offline_split_info, ss);

   // If fractions come in but the craziness is restrained, just pass the fractions on.
   // This is what makes "random twice mix" work.  The "random" concept wants to reach through
   // the "twice" concept and do the numbered parts of the mix, not the numbered parts of the
   // twiceness.  But if the craziness is unrestrained, which is the usual case, we act on
   // the fractions.  This makes "interlace twice this with twice that" work. */

   zzz.get_fraction_info(ss->cmd.cmd_frac_flags,
                         3*CFLAG1_VISIBLE_FRACTION_BIT,
                         weirdness_off);

   zzz.m_first_call = !zzz.m_reverse_order;

   *result = *ss;
   clear_result_flags(result);

   for (;;) {
      int fetch_number;

      if (prepare_for_call_under_repetition(&zzz, fetch_number, ss, result)) break;
      if (zzz.not_yet_in_active_section()) goto go_to_next_cycle;
      if (zzz.ran_off_active_section()) break;

      // Do *NOT* try to maintain consistent splitting across repetitions when doing "twice".
      result->result_flags.maximize_split_info();

      // We do *NOT* remember the yoyo/twisted expiration stuff.
      result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;

      do_call_in_series_simple(result);

      // Record the minimum in each direction.
      minimize_splitting_info(&offline_split_info, result->result_flags);
      zzz.m_first_call = false;

      // If we are being asked to do just one part of a call,
      // exit now.  Also, fill in bits in result->result_flags.

      if (zzz.query_instant_stop(result->result_flags.misc)) break;

   go_to_next_cycle:

      // Increment for next cycle.
      zzz.m_fetch_index += zzz.m_subcall_incr;
      zzz.m_client_index += zzz.m_subcall_incr;
   }

   result->result_flags.misc |= offline_split_info.result_flags.misc;  //******* needed?
   result->result_flags.copy_split_info(offline_split_info.result_flags);
}


static void do_concept_trace(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int interlock = ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_INTLK);

   ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_INTLK);
   // We don't allow other flags, like "cross".
   if (ss->cmd.cmd_final_flags.test_herit_and_final_bits())
      fail("Illegal modifier before \"trace\".");

   int i, r[4], rot[4];
   resultflag_rec finalresultflags;
   setup a[4], res[4];
   setup outer_inners[2];
   const veryshort *tracearray;

   static const veryshort tracearray1[16] =
   {-1, -1, 7, 6, -1, -1, 4, 5, 3, 2, -1, -1, 0, 1, -1, -1};
   static const veryshort tracearray2[16] =
   {0, 1, -1, -1, 6, 7, -1, -1, -1, -1, 4, 5, -1, -1, 2, 3};

   if (ss->kind != s_qtag) fail("Must have a 1/4-tag-like setup for trace.");

   // We handle "interlocked trace" in a rather simple-minded way.
   if (interlock != 0) swap_people(ss, 3, 7);

   ss->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   for (i=0 ; i<4 ; i++) {
      a[i] = *ss;
      clear_people(&a[i]);
      a[i].kind = s2x2;
      a[i].rotation = 0;
   }

   if ((ss->people[6].id1 & d_mask) == d_north && (ss->people[2].id1 & d_mask) == d_south) {
      a[1].cmd.parseptr = parseptr->subsidiary_root;
      a[3].cmd.parseptr = parseptr->subsidiary_root;
      tracearray = tracearray1;
   }
   else if ((ss->people[6].id1 & d_mask) == d_south && (ss->people[2].id1 & d_mask) == d_north) {
      a[0].cmd.parseptr = parseptr->subsidiary_root;
      a[2].cmd.parseptr = parseptr->subsidiary_root;
      tracearray = tracearray2;
   }
   else
      fail("Can't determine which box people should work in.");

   for (i=0 ; i<4 ; i++) {
      a[i].cmd.cmd_assume.assumption = cr_none;
      gather(&a[i], ss, tracearray, 3, 0);
      update_id_bits(&a[i]);
      move(&a[i], false, &res[i]);
      tracearray += 4;
   }

   finalresultflags = get_multiple_parallel_resultflags(res, 4);
   clear_people(&outer_inners[1]);
   clear_people(&outer_inners[0]);

   // Check that everyone is in a 2x2 or vertically oriented 1x4.

   for (i=0 ; i<4 ; i++) {
      if ((res[i].kind != s2x2 && res[i].kind != nothing && (res[i].kind != s1x4 || (!(res[i].rotation&1)))))
         fail("You can't do this.");
   }

   // Process people going into the center.

   outer_inners[1].rotation = 0;
   clear_result_flags(&outer_inners[1]);

   if   ((res[0].kind == s2x2 && (res[0].people[2].id1 | res[0].people[3].id1)) ||
         (res[1].kind == s2x2 && (res[1].people[0].id1 | res[1].people[1].id1)) ||
         (res[2].kind == s2x2 && (res[2].people[0].id1 | res[2].people[1].id1)) ||
         (res[3].kind == s2x2 && (res[3].people[2].id1 | res[3].people[3].id1)))
      outer_inners[1].kind = s1x4;
   else
      outer_inners[1].kind = nothing;

   for (i=0 ; i<4 ; i++) {
      r[i] = res[i].rotation & 2;
      rot[i] = (res[i].rotation & 3) * 011;
   }

   if   ((res[0].kind == s1x4 && (res[0].people[2 ^ r[0]].id1 | res[0].people[3 ^ r[0]].id1)) ||
         (res[1].kind == s1x4 && (res[1].people[0 ^ r[1]].id1 | res[1].people[1 ^ r[1]].id1)) ||
         (res[2].kind == s1x4 && (res[2].people[0 ^ r[2]].id1 | res[2].people[1 ^ r[2]].id1)) ||
         (res[3].kind == s1x4 && (res[3].people[2 ^ r[3]].id1 | res[3].people[3 ^ r[3]].id1))) {
      if (outer_inners[1].kind != nothing) fail("You can't do this.");
      outer_inners[1].kind = s2x2;
   }

   if (res[0].kind == s2x2) {
      install_person(&outer_inners[1], 1, &res[0], 2);
      install_person(&outer_inners[1], 0, &res[0], 3);
   }
   else {
      install_rot(&outer_inners[1], 3, &res[0], 2^r[0], rot[0]);
      install_rot(&outer_inners[1], 0, &res[0], 3^r[0], rot[0]);
   }

   if (res[1].kind == s2x2) {
      install_person(&outer_inners[1], 0, &res[1], 0);
      install_person(&outer_inners[1], 1, &res[1], 1);
   }
   else {
      install_rot(&outer_inners[1], 0, &res[1], 0^r[1], rot[1]);
      install_rot(&outer_inners[1], 3, &res[1], 1^r[1], rot[1]);
   }

   if (res[2].kind == s2x2) {
      install_person(&outer_inners[1], 3, &res[2], 0);
      install_person(&outer_inners[1], 2, &res[2], 1);
   }
   else {
      install_rot(&outer_inners[1], 1, &res[2], 0^r[2], rot[2]);
      install_rot(&outer_inners[1], 2, &res[2], 1^r[2], rot[2]);
   }

   if (res[3].kind == s2x2) {
      install_person(&outer_inners[1], 2, &res[3], 2);
      install_person(&outer_inners[1], 3, &res[3], 3);
   }
   else {
      install_rot(&outer_inners[1], 2, &res[3], 2^r[3], rot[3]);
      install_rot(&outer_inners[1], 1, &res[3], 3^r[3], rot[3]);
   }

   // Process people going to the outside.

   outer_inners[0].rotation = 0;
   clear_result_flags(&outer_inners[0]);

   for (i=0 ; i<4 ; i++) {
      r[i] = res[i].rotation & 2;
   }

   if   ((res[0].kind == s2x2 && (res[0].people[0].id1 | res[0].people[1].id1)) ||
         (res[1].kind == s2x2 && (res[1].people[2].id1 | res[1].people[3].id1)) ||
         (res[2].kind == s2x2 && (res[2].people[2].id1 | res[2].people[3].id1)) ||
         (res[3].kind == s2x2 && (res[3].people[0].id1 | res[3].people[1].id1)))
      outer_inners[0].kind = s2x2;
   else
      outer_inners[0].kind = nothing;

   if   ((res[0].kind == s1x4 && (res[0].people[0 ^ r[0]].id1 | res[0].people[1 ^ r[0]].id1)) ||
         (res[1].kind == s1x4 && (res[1].people[2 ^ r[1]].id1 | res[1].people[3 ^ r[1]].id1)) ||
         (res[2].kind == s1x4 && (res[2].people[2 ^ r[2]].id1 | res[2].people[3 ^ r[2]].id1)) ||
         (res[3].kind == s1x4 && (res[3].people[0 ^ r[3]].id1 | res[3].people[1 ^ r[3]].id1))) {
      if (outer_inners[0].kind != nothing) fail("You can't do this.");
      outer_inners[0].kind = s1x4;
      outer_inners[0].rotation = 1;
   }

   for (i=0 ; i<4 ; i++) {
      int ind = (i+1) & 2;

      if (res[i].kind == s2x2) {
         install_person(&outer_inners[0], ind,     &res[i], ind);
         install_person(&outer_inners[0], ind ^ 1, &res[i], ind ^ 1);
      }
      else {
         install_rot(&outer_inners[0], ind,     &res[i], ind ^ (res[i].rotation&2),     ((res[i].rotation-1)&3)*011);
         install_rot(&outer_inners[0], ind ^ 1, &res[i], ind ^ (res[i].rotation&2) ^ 1, ((res[i].rotation-1)&3)*011);
      }
   }

   normalize_concentric(schema_concentric, 1, outer_inners,
                        ((~outer_inners[0].rotation) & 1) + 1, 0, result);
   result->result_flags = finalresultflags;

   if (interlock != 0) {
      if (result->kind != s_qtag) fail("Can't do this interlocked trace.");
      swap_people(result, 3, 7);
   }

   reinstate_rotation(ss, result);
}


static void do_concept_outeracting(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   setup temp;

   static const veryshort mapo1[8] = {7, 0, 2, 5, 3, 4, 6, 1};
   static const veryshort mapo2[8] = {7, 0, 1, 6, 3, 4, 5, 2};

   temp = *ss;
   temp.kind = s2x4;
   temp.rotation++;
   clear_people(&temp);

   if (ss->kind != s_qtag)
      fail("Must have 1/4 tag to do this concept.");

   if ((((ss->people[2].id1 ^ d_south) | (ss->people[6].id1 ^ d_north)) & d_mask) == 0) {
      scatter(&temp, ss, mapo1, 7, 033);
   }
   else if ((((ss->people[2].id1 ^ d_north) | (ss->people[6].id1 ^ d_south)) & d_mask) == 0) {
      scatter(&temp, ss, mapo2, 7, 033);
   }
   else
      fail("Incorrect facing directions.");

   divided_setup_move(&temp, MAPCODE(s2x2,2,MPKIND__SPLIT,0), phantest_ok, true, result);
}


static void do_concept_multiple_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // Arg2 = the matrix that we need, with CONCPROP__NEED_ARG2_MATRIX.  It has
   //    already been taken care of.
   // Arg4 = the number of items.
   // Arg5 = stuff used by "do_triple_formation", which we might call.
   //    It is MPKIND__SPLIT.  We don't look at it here.

   if (parseptr->concept->arg4 == 3) {
      if (ss->kind != s2x6) fail("Must have a 2x6 setup for this concept.");
      do_triple_formation(ss, parseptr, MAPCODE(s2x2,3,parseptr->concept->arg5,0), result);
   }
   else {
      if (parseptr->concept->arg4 == 4) {
         if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");
      }
      else if (parseptr->concept->arg4 == 5) {
         if (ss->kind != s2x10) fail("Must have a 2x10 setup for this concept.");
      }
      else if (parseptr->concept->arg4 == 6) {
         if (ss->kind != s2x12) fail("Must have a 2x12 setup for this concept.");
      }

      divided_setup_move(ss, MAPCODE(s2x2,parseptr->concept->arg4,MPKIND__SPLIT,0),
                         phantest_ok, true, result);
   }
}


static void do_concept_inner_outer(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 livemask, misc2_zflag;
   int i;
   calldef_schema sch;
   int rot = 0;
   int arg1 = parseptr->concept->arg1;

   switch (arg1 & 0x70) {
   case 0:      // triple CLWBDZ
   case 0x20:   // triple twin CLW
   case 0x30:   // triple twin CLW of 3
   case 0x50:   // triple tidal CLW
      sch = schema_in_out_triple;
      break;
   case 0x10:
      sch = schema_in_out_quad;
      break;
   case 0x40:
      sch = schema_in_out_12mquad;
      break;
   }

   switch (arg1) {
   case 0: case 1: case 3:
      // Center triple line/wave/column.
      switch (ss->kind) {
      case s3x4: case s1x12:
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (!((arg1 ^ global_tbonetest) & 1)) {
            if (global_tbonetest & 1) fail("There are no triple lines here.");
            else                      fail("There are no triple columns here.");
         }
         goto ready;
      case sbigh: case sbigx: case sbigdmd: case sbigbone:
         goto verify_clw;
      }

      fail("Need a triple line/column setup for this.");
   case 8+0: case 8+1: case 8+3:
      // Outside triple lines/waves/columns.
      switch (ss->kind) {
      case s3x4: case s1x12:
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (!((arg1 ^ global_tbonetest) & 1)) {
            if (global_tbonetest & 1) fail("There are no triple lines here.");
            else                      fail("There are no triple columns here.");
         }
         goto ready;
      case sbigh: case sbigx: case sbigrig: case shsqtag: case sbig3x1dmd: case sbig1x3dmd:
         goto verify_clw;
      }

      fail("Need a triple line/column setup for this.");
   case 32+0: case 32+1: case 32+3:
   case 32+8+0: case 32+8+1: case 32+8+3:
      // Center/outside triple twin lines/waves/columns.
      if (ss->kind != s4x6) fail("Need a 4x6 setup for this.");

      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if ((arg1 ^ global_tbonetest) & 1) {
         if (global_tbonetest & 1) fail("There are no triple twin lines here.");
         else                      fail("There are no triple twin columns here.");
      }

      goto ready;
   case 80+0: case 80+1: case 80+3:
      // Center tidal line/wave/column.
      // This concept has the "CONCPROP__NEEDK_3X8" property set,
      // which will fail for quadruple diamonds.
      if (ss->kind != s3x8 && ss->kind != s4dmd)
         fail("Need center tidal setup for this.");
      // Unfortunately, we can't readily test facing direction.
      // Too lazy to do it right.
      goto ready;
   case 48+0: case 48+1: case 48+3:
   case 48+8+0: case 48+8+1: case 48+8+3:
      // Center/outside triple twin lines/waves/columns of 3.
      if (ss->kind != s3x6) fail("Need a 3x6 setup for this.");

      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if ((arg1 ^ global_tbonetest) & 1) {
         if (global_tbonetest & 1) fail("There are no triple twin lines of 3 here.");
         else                      fail("There are no triple twin columns of 3here.");
      }

      goto ready;
   case 16+0: case 16+1: case 16+3:
   case 16+8+0: case 16+8+1: case 16+8+3:
      // Center or outside phantom lines/waves/columns.

      if (ss->kind == s4x4) {
         uint32 tbone = global_tbonetest;
         // If everyone is consistent (or "standard" was used
         // to make it appear so), it's easy.
         if ((tbone & 011) == 011) {
            // If not, we need to look carefully.
            tbone =
               ss->people[1].id1 | ss->people[2].id1 |
               ss->people[5].id1 | ss->people[6].id1 |
               ss->people[9].id1 | ss->people[10].id1 |
               ss->people[13].id1 | ss->people[14].id1;

            if (arg1 & 8) {
               // This is outside phantom C/L/W.
               tbone |=
                  ss->people[0].id1 | ss->people[4].id1 |
                  ss->people[8].id1 | ss->people[12].id1;
            }
            else {
               // This is center phantom C/L/W.
               tbone |=
                  ss->people[3].id1 | ss->people[7].id1 |
                  ss->people[11].id1 | ss->people[15].id1;
            }
            if ((tbone & 011) == 011) fail("Can't do this from T-bone setup.");
         }

         rot = (tbone ^ arg1 ^ 1) & 1;

         ss->rotation += rot;   // Just flip the setup around and recanonicalize.
         canonicalize_rotation(ss);
      }
      else if (ss->kind == s1x16 || ss->kind == sbigbigh || ss->kind == sbigbigx) {
         /* Shouldn't we do this for 4x4 also?  It seems that one ought to say
            "standard in outside phantom 1x4's" rather than "... in lines"
            if they are T-boned.  There is something in t34t that would break, however. */
         goto verify_clw;
      }
      else
         fail("Need quadruple 1x4's for this.");

      break;
   case 64+0: case 64+1:
   case 64+8+0: case 64+8+1:
      // Center or outside phantom lines/columns (no wave).
      if (ss->kind == s3x4 || ss->kind == s_d3x4 || ss->kind == s3dmd)
         goto verify_clw;
      else
         fail("Need quadruple 1x3's for this.");
      break;
   case 4:
      // Center triple box.
      switch (ss->kind) {
      case s2x6: case sbigrig:
         goto ready;
      case s4x4:
         if (ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1) {
            // The outer lines/columns are to the left and right.  That's not canonical.
            // We want them at top and bottom.  So we flip the setup.
            rot = 1;
            ss->rotation++;
            canonicalize_rotation(ss);
         }

         // Now the people had better be clear from the side centers.

         if (!(ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1))
            goto ready;
      }

      fail("Need center triple box for this.");
   case 8+4:
      // Outside triple boxes.
      switch (ss->kind) {
      case s2x6: case sbigbone: case sbigdmd:
      case sbighrgl: case sbigdhrgl:
         goto ready;
      }

      fail("Need outer triple boxes for this.");
   case 16+4:
   case 16+8+4:
      // Center or outside quadruple boxes.
      if (ss->kind != s2x8) fail("Need a 2x8 setup for this.");
      goto ready;
   case 5:
      // Center triple diamond.
      switch (ss->kind) {
      case s3dmd: case s3ptpd: case s_3mdmd: case s_3mptpd: case s3x1dmd: case s1x3dmd:
      case shsqtag: case s_hrglass: case s_dhrglass: case sbighrgl: case sbigdhrgl:
      case sbig3x1dmd: case sbig1x3dmd:
         goto ready;
      }

      fail("Need center triple diamond for this.");
   case 8+5:
      // Outside triple diamonds.
      switch (ss->kind) {
      case s3dmd: case s3ptpd: case s_3mdmd: case s_3mptpd: case s3x1dmd: case s1x3dmd:
         goto ready;
      }

      fail("Need outer triple diamonds for this.");
   case 16+5:
   case 16+8+5:
      // Center or outside quadruple diamonds.
      if (ss->kind != s4dmd &&
          ss->kind != s4ptpd &&
          ss->kind != s_4mptpd &&
          ss->kind != s_4mdmd) fail("Need quadruple diamonds for this.");
      ss->cmd.cmd_misc_flags |= parseptr->concept->arg3;
      break;
   case 6:
   case 8+6:
      // Center/outside triple Z's.

      sch = schema_in_out_triple_zcom;

      for (i=0,livemask=0 ; i<=attr::slimit(ss) ; i++) {
         livemask <<= 1;
         if (ss->people[i].id1) livemask |= 1;
      }

      switch (ss->kind) {
      case s3x6:
         if ((livemask & 0630630) == 0) {
            /* Of course, this is kind of stupid.  Why would you say "outer
               triple Z's" if the outer Z's weren't recognizeable, and you
               simply wanted them to be imputed from the center Z? */
            if (arg1 & 8)
               warn(warn_same_z_shear);  /* Outer Z's are ambiguous --
                                            make them look like inner ones. */
         }

         // Demand that the center Z be solidly filled.

         switch (livemask & 0141141) {
         case 0101101:   // Center Z is CW.
            if ((livemask & 0210210) == 0) {
               misc2_zflag = CMD_MISC2__IN_Z_CW;
               goto do_real_z_stuff;
            }
            else if ((livemask & 0420420) == 0) {
               misc2_zflag = CMD_MISC2__IN_AZ_CW;   // Z's are anisotropic.
               goto do_real_z_stuff;
            }

            break;
         case 0041041:   // Center Z is CCW.
            if ((livemask & 0420420) == 0) {
               misc2_zflag = CMD_MISC2__IN_Z_CCW;
               goto do_real_z_stuff;
            }
            else if ((livemask & 0210210) == 0) {
               misc2_zflag = CMD_MISC2__IN_AZ_CCW;   // Z's are anisotropic.
               goto do_real_z_stuff;
            }

            break;
         }
         break;
      case swqtag:
         if (arg1 & 8) break;   // Can't say "outside triple Z's".
         switch (livemask & 0x339) {
         case 0x231:   // Center Z is CW.
            misc2_zflag = CMD_MISC2__IN_Z_CW;
            goto do_real_z_stuff;
         case 0x129:   // Center Z is CCW.
            misc2_zflag = CMD_MISC2__IN_Z_CCW;
            goto do_real_z_stuff;
         }
         break;
      case s2x5:
         if (arg1 & 8) break;   // Can't say "outside triple Z's".
         switch (livemask) {
         case 0x2F7:
            misc2_zflag = CMD_MISC2__IN_Z_CW;
            goto do_real_z_stuff;
         case 0x3BD:
            misc2_zflag = CMD_MISC2__IN_Z_CCW;
            goto do_real_z_stuff;
         }
         break;
      case s2x7:
         if (arg1 & 8) break;   // Can't say "outside triple Z's".
         switch (livemask & 0x0E1C) {
         case 0x060C:
            misc2_zflag = CMD_MISC2__IN_Z_CW;
            goto do_real_z_stuff;
         case 0x0C18:
            misc2_zflag = CMD_MISC2__IN_Z_CCW;
            goto do_real_z_stuff;
         }
         break;
      case sd2x5:
         if (arg1 & 8) break;   // Can't say "outside triple Z's".
         switch (livemask) {
         case 0x3DE:
            misc2_zflag = CMD_MISC2__IN_Z_CW;
            goto do_real_z_stuff;
         case 0x37B:
            misc2_zflag = CMD_MISC2__IN_Z_CCW;
            goto do_real_z_stuff;
         }
         break;
      }
      fail("Can't find the indicated formation.");
   }

 ready:

   if ((arg1 & 7) == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (arg1 & 8)
      concentric_move(ss, &ss->cmd, (setup_command *) 0, sch, 0, 0, true, ~0UL, result);
   else
      concentric_move(ss, (setup_command *) 0, &ss->cmd, sch, 0, 0, true, ~0UL, result);

   result->rotation -= rot;   // Flip the setup back.
   return;

 verify_clw:

   switch (arg1 & 7) {
   case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
   case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
   }
   goto ready;

 do_real_z_stuff:

   ss->cmd.cmd_misc2_flags |= misc2_zflag;
   goto ready;
}


static void do_concept_do_both_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   if (ss->kind == s2x4)
      divided_setup_move(ss, parseptr->concept->arg1, phantest_ok, true, result);
   else if (ss->kind == s3x4 && parseptr->concept->arg3)
      // distorted_2x2s_move will notice that concept is funny and will do the right thing.
      distorted_2x2s_move(ss, parseptr, result);
   else
      fail("Need a 2x4 setup to do this concept.");
}


static void do_concept_do_each_1x4(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 map_code;
   int arg1 = parseptr->concept->arg1;
   int arg2 = parseptr->concept->arg2;

   if (arg2 == 1) {
      switch (ss->kind) {
         case s_qtag: case s_ptpd: break;
         default: fail("Need diamonds for this concept.");
      }
   }
   else if (arg2 == 2) {
      switch (ss->kind) {
         case s2x4: break;
         case s3x4:
            if (global_livemask == 0xCF3) {
               map_code = MAPCODE(s2x2,2,MPKIND__OFFS_R_HALF, 0);
               goto split_big;
            }
            else if (global_livemask == 0xF3C) {
               map_code = MAPCODE(s2x2,2,MPKIND__OFFS_L_HALF, 0);
               goto split_big;
            }
            fail("Need boxes for this concept.");
            break;
         case s4x4:
            if (global_livemask == 0xB4B4) {
               map_code = MAPCODE(s2x2,2,MPKIND__OFFS_R_FULL, 1);
               goto split_big;
            }
            else if (global_livemask == 0x4B4B) {
               map_code = MAPCODE(s2x2,2,MPKIND__OFFS_L_FULL, 1);
               goto split_big;
            }
         /* !!!!!! FALL THROUGH !!!!!! */
         default: fail("Need boxes for this concept.");
      }
   }
   else {
      if (arg1 != 0 && ((arg1 ^ global_tbonetest) & 1) == 0)
         fail("People are not in the required line, column or wave.");

      if (arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      switch (ss->kind) {
         case s2x4: case s1x8:
            goto split_small;
         /* Really ought to handle 4x4 as well, but then the tbone test above won't do. */
         case s3x4:
            if (global_livemask == 01717) {
               map_code = MAPCODE(s1x4,3,MPKIND__SPLIT,1);
               goto split_big;
            }
            break;
         case s2x6:
            if (global_livemask == 07474) {
               map_code = MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF,1);
               goto split_big;
            }
            else if (global_livemask == 01717) {
               map_code = MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF,1);
               goto split_big;
            }
            break;
         case s2x8:
            if (global_livemask == 0xF0F0) {
               map_code = MAPCODE(s1x4,2,MPKIND__OFFS_R_FULL,1);
               goto split_big;
            }
            else if (global_livemask == 0x0F0F) {
               map_code = MAPCODE(s1x4,2,MPKIND__OFFS_L_FULL,1);
               goto split_big;
            }
            break;
         case s1x10:
            if (global_livemask == 0x1EF) {
               map_code = spcmap_d1x10;
               goto split_big;
            }
            break;
      }

      fail("Need a 2x4 or 1x8 setup for this concept.");
   }

   split_small:

   do_simple_split(ss, (arg2 != 2) ? split_command_1x4 : split_command_none, result);
   return;

   split_big:

   divided_setup_move(ss, map_code, phantest_ok, true, result);
}


static void do_concept_centers_and_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   selective_move(ss, parseptr, selective_key_plain, 1, 0, 0,
                  (selector_kind) parseptr->concept->arg1,
                  parseptr->concept->arg2 != 0, result);
}


static void do_concept_centers_or_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   selective_move(ss, parseptr, selective_key_plain, 0, 0, 0,
                  (selector_kind) parseptr->concept->arg1,
                  parseptr->concept->arg2 != 0, result);
}


static void do_concept_mini_but_o(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   int rot = 0;
   uint32 is_an_o = parseptr->concept->arg1;
   uint32 mask = 0;

   switch (ss->kind) {
   case s_galaxy:
      if (is_an_o) {
         rot = ss->people[1].id1 | ss->people[3].id1 |
            ss->people[5].id1 | ss->people[7].id1;

         if ((rot & 011) == 011) fail("mini-O is ambiguous.");
         rot &= 1;
         ss->rotation += rot;
         canonicalize_rotation(ss);
         mask = 0xBB;
      }

      break;
   case s_hrglass:
      if (!is_an_o) mask = 0xBB;
      break;
   case s_ptpd:
      if (!is_an_o) mask = 0xEE;
      break;
   case s_rigger:
      if (is_an_o) mask = 0xBB;
      break;
   case swqtag:
      // We require the center two missing.
      if (is_an_o && (ss->people[4].id1 | ss->people[9].id1) == 0) mask = 0x16B;
      break;
   case s3x4:
      if (is_an_o) mask = 02626;
      break;
   }

   if (mask == 0) fail("Can't do this concept in this setup.");

   // Adding 2 to the lookup key makes it check for columns.
   selective_move(ss, parseptr, selective_key_mini_but_o, 0,
                  is_an_o ? LOOKUP_MINI_O+2 : LOOKUP_MINI_B+2,
                  mask, selector_none, false, result);

   result->rotation -= rot;   // Flip the setup back.
   return;
}



static void so_and_so_only_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   selective_move(ss, parseptr, (selective_key) parseptr->concept->arg1,
                  parseptr->concept->arg2, parseptr->concept->arg3,
                  0, parseptr->options.who, false, result);
}


static void do_concept_triple_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 code;

   switch (ss->kind) {
   case s3dmd:
      code = MAPCODE(sdmd,3,MPKIND__SPLIT,1); break;
   case s3ptpd:
      code = MAPCODE(sdmd,3,MPKIND__SPLIT,0); break;
   case s_3mdmd:
      code = MAPCODE(sdmd,3,MPKIND__NONISOTROPIC,1); break;
   case s_3mptpd:
      code = MAPCODE(sdmd,3,MPKIND__NONISOTROPIC,0); break;
   default:
      fail("Must have a triple diamond or 1/4 tag setup for this concept.");
   }

   do_triple_formation(ss, parseptr, code, result);
}


static void do_concept_triple_formations(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // Meaning of arg3 is:
   //   0 - triple lines or boxes
   //   1 - triple lines or diamonds
   //   2 - triple boxes or diamonds

   setup tempsetup = *ss;

   // We need to expand certain setups, of course.
   //
   // Why don't we do it the usual way, with "NEEDK" bits like
   // "NEEDK_TRIPLE_1X4_OR_BOX", setting those bits in the concept
   // properties?  Because we would run out of bits!  We have used
   // up too many bits doing other (worthwhile) things.  Unless we
   // change the format of the "NEEDK" mechanism to use 64 bits
   // (which isn't worth it), we have to do these by hand.  It's
   // not actually a serious problem -- the "NEEDK" mechanism does
   // a lot of powerful and useful things elsewhere in the program.
   // We don't need that power and flexibility here.  It's not much
   // work to take care of the individual cases.

   uint32 need_prop = 0;

   switch (parseptr->concept->arg3) {
   case 0:
      switch (tempsetup.kind) {
      case s_rigger:            need_prop = CONCPROP__NEEDK_END_1X4; break;
      case s2x4:                need_prop = CONCPROP__NEEDK_4X4; break;
      case s_bone: case s_qtag: need_prop = CONCPROP__NEEDK_END_2X2; break;
      }

      do_matrix_expansion(&tempsetup, need_prop, false);

      // The 4x4 case is for a box sandwiched between two parallel 1x4's.
      // Concentric_move knows how to deal with this.
      if (tempsetup.kind != sbigbone && tempsetup.kind != sbigrig &&
          tempsetup.kind != sbigdmd && tempsetup.kind != s4x4)
         fail("Can't do this concept in this setup.");
      break;
   case 1:
      switch (tempsetup.kind) {
      case s3x1dmd: case s1x3dmd: case s_hrglass: need_prop = CONCPROP__NEEDK_END_1X4; break;
      }

      do_matrix_expansion(&tempsetup, need_prop, false);

      if (tempsetup.kind != sbig3x1dmd && tempsetup.kind != sbig1x3dmd &&
          tempsetup.kind != shsqtag)
         fail("Can't do this concept in this setup.");
      break;
   default:

      switch (tempsetup.kind) {
      case s_dhrglass: case s_hrglass: need_prop = CONCPROP__NEEDK_END_2X2; break;
      }

      do_matrix_expansion(&tempsetup, need_prop, false);

      if (tempsetup.kind != sbighrgl && tempsetup.kind != sbigdhrgl)
         fail("Can't do this concept in this setup.");
      break;
   }

   do_triple_formation(&tempsetup, parseptr, ~0UL, result);
}


static void do_concept_quad_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 code;

   // See "do_triple_formation" for meaning of arg2.

   switch (ss->kind) {
   case s4dmd:
      code = MAPCODE(sdmd,4,MPKIND__SPLIT,1); break;
   case s4ptpd:
      code = MAPCODE(sdmd,4,MPKIND__SPLIT,0); break;
   case s_4mdmd:
      code = MAPCODE(sdmd,4,MPKIND__NONISOTROPIC,1); break;
   case s_4mptpd:
      code = MAPCODE(sdmd,4,MPKIND__NONISOTROPIC,0); break;
   default:
      fail("Must have a quadruple diamond or 1/4 tag setup for this concept.");
   }

   ss->cmd.cmd_misc_flags |= parseptr->concept->arg2;
   divided_setup_move(ss, code, phantest_ok, true, result);
}


static void do_concept_ferris(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   const expand::thing *map_ptr = (const expand::thing *) 0;

   if (parseptr->concept->arg1) {
      // This is "release".

      static const expand::thing mapr1 = {{10, 2, 3, 5, 4,  8, 9, 11}, 8, s_qtag, s3x4, 0};
      static const expand::thing mapr2 = {{1,  4, 6, 5, 7, 10, 0, 11}, 8, s_qtag, s3x4, 0};
      static const expand::thing mapr3 = {{1, 10, 0, 5, 7,  4, 6, 11}, 8, s_qtag, s3x4, 0};
      static const expand::thing mapr4 = {{4,  2, 9, 5,10,  8, 3, 11}, 8, s_qtag, s3x4, 0};

      if ((ss->kind != s_qtag) || ((global_tbonetest & 1) != 0))
         fail("Must have quarter-tag to do this concept.");

      uint32 whatcanitbe = ~0;
      uint32 t;

      if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_MAGIC)) {
         if ((t = ss->people[0].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x4;
         if ((t = ss->people[4].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x4;
         if ((t = ss->people[2].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x4;
         if ((t = ss->people[6].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x4;
         if ((t = ss->people[1].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x8;
         if ((t = ss->people[5].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x8;
         if ((t = ss->people[2].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x8;
         if ((t = ss->people[6].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x8;
      }
      else {
         if ((t = ss->people[1].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x1;
         if ((t = ss->people[5].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x1;
         if ((t = ss->people[2].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x1;
         if ((t = ss->people[6].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x1;
         if ((t = ss->people[0].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x2;
         if ((t = ss->people[4].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x2;
         if ((t = ss->people[2].id1) && ((t ^ d_north) & d_mask)) whatcanitbe &= 0x2;
         if ((t = ss->people[6].id1) && ((t ^ d_south) & d_mask)) whatcanitbe &= 0x2;
      }

      switch (whatcanitbe) {
      case 0x1: map_ptr = &mapr1; break;
      case 0x2: map_ptr = &mapr2; break;
      case 0x4: map_ptr = &mapr3; break;
      case 0x8: map_ptr = &mapr4; break;
      }

      ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_MAGIC);
   }
   else {
      // This is "ferris".

      static const expand::thing mapf1 = {{0, 1, 5, 4, 6, 7, 11, 10}, 8, s2x4, s3x4, 0};
      static const expand::thing mapf2 = {{10, 11, 2, 3, 4, 5, 8, 9}, 8, s2x4, s3x4, 0};

      if ((ss->kind != s2x4) || ((global_tbonetest & 1) != 0))
         fail("Must have lines to do this concept.");

      bool retvaljunk;
      assumption_thing t;
      t.assumption = cr_2fl_only;
      t.assump_col = 0;
      t.assump_both = 1;    // right 2FL
      t.assump_cast = 0;
      t.assump_live = 0;
      t.assump_negate = 0;

      if (verify_restriction(ss, t, false, &retvaljunk) == restriction_passes)
         map_ptr = &mapf1;
      else {
         t.assump_both = 2;    // left 2FL
         if (verify_restriction(ss, t, false, &retvaljunk) == restriction_passes)
            map_ptr = &mapf2;
      }
   }

   if (!map_ptr) fail("Incorrect facing directions.");

   // Be sure there aren't any modifiers other than "magic release".
   if (ss->cmd.cmd_final_flags.test_herit_and_final_bits())
      fail("Illegal modifier before \"ferris\" or \"release\".");

   expand::expand_setup(map_ptr, ss);
   concentric_move(ss, &ss->cmd, &ss->cmd, schema_in_out_triple_squash, 0, 0, false, ~0UL, result);
}


static void do_concept_overlapped_diamond(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   uint32 mapcode;
   const expand::thing *scatterlist;
   static const expand::thing list1x4    = {{0, 1, 4, 5}, 4, s1x4, s_thar, 0};
   static const expand::thing list1x4rot = {{2, 3, 6, 7}, 4, s1x4, s_thar, 3};
   static const expand::thing listdmd    = {{0, 3, 4, 7}, 4, sdmd, s_thar, 0};
   static const expand::thing listdmdrot = {{2, 5, 6, 1}, 4, sdmd, s_thar, 3};

   // Split an 8 person setup.
   if (attr::slimit(ss) == 7) {
      // Reset it to execute this same concept again, until it doesn't have to split any more.
      ss->cmd.parseptr = parseptr;
      if (do_simple_split(ss, split_command_1x4, result))
         fail("Not in correct setup for overlapped diamond/line concept.");
      return;
   }

   switch (ss->kind) {
   case s1x4:
      if (parseptr->concept->arg1 & 1)
         fail("Must be in a diamond.");

      scatterlist = &list1x4;
      mapcode = MAPCODE(sdmd,2,MPKIND__DMD_STUFF,0);
      break;
   case sdmd:
      if (!(parseptr->concept->arg1 & 1))
         fail("Must be in a line.");
      if (parseptr->concept->arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      scatterlist = &listdmd;
      mapcode = MAPCODE(s1x4,2,MPKIND__DMD_STUFF,0);
      break;
   default:
      fail("Not in correct setup for overlapped diamond/line concept.");
   }

   expand::expand_setup(scatterlist, ss);
   divided_setup_move(ss, mapcode, phantest_ok, true, result);

   if (result->kind == s2x2)
      return;
   else if (result->kind != s_thar)
      fail("Something horrible happened during overlapped diamond call.");

   if ((result->people[2].id1 | result->people[3].id1 |
        result->people[6].id1 | result->people[7].id1) == 0)
      scatterlist = &list1x4;
   else if ((result->people[0].id1 | result->people[1].id1 |
             result->people[4].id1 | result->people[5].id1) == 0)
      scatterlist = &list1x4rot;
   else if ((result->people[1].id1 | result->people[2].id1 |
             result->people[5].id1 | result->people[6].id1) == 0)
      scatterlist = &listdmd;
   else if ((result->people[0].id1 | result->people[3].id1 |
             result->people[4].id1 | result->people[7].id1) == 0)
      scatterlist = &listdmdrot;
   else
      fail("Can't put the setups back together.");

   expand::compress_setup(scatterlist, result);
}



static void do_concept_all_8(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   static const veryshort expander[8] = {10, 13, 14, 1, 2, 5, 6, 9};

   int key = parseptr->concept->arg1;

   /* key =
      all 4 couples    : 0
      all 8            : 1
      all 8 (diamonds) : 2 */

   if (key == 0) {

      /* This is "all 4 couples". */

      if (  ss->kind != s4x4 ||
            (( ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1 |
               ss->people[8].id1 | ss->people[11].id1 | ss->people[12].id1 | ss->people[15].id1) != 0) ||
            (((ss->people[ 1].id1 ^ ss->people[ 2].id1) & d_mask) != 0) ||
            (((ss->people[ 5].id1 ^ ss->people[ 6].id1) & d_mask) != 0) ||
            (((ss->people[ 9].id1 ^ ss->people[10].id1) & d_mask) != 0) ||
            (((ss->people[13].id1 ^ ss->people[14].id1) & d_mask) != 0) ||
            (ss->people[ 1].id1 != 0 && ((ss->people[ 1].id1) & 1) == 0) ||
            (ss->people[ 5].id1 != 0 && ((ss->people[ 5].id1) & 1) != 0) ||
            (ss->people[ 9].id1 != 0 && ((ss->people[ 9].id1) & 1) == 0) ||
            (ss->people[13].id1 != 0 && ((ss->people[13].id1) & 1) != 0))
         fail("Must be in squared set spots.");

      divided_setup_move(ss, MAPCODE(s2x2,2,MPKIND__ALL_8,0), phantest_ok, true, result);
   }
   else {

      /* This is "all 8" or "all 8 (diamond)". */

      /* Turn a 2x4 into a 4x4, if people are facing reasonably.  If the centers are all
         facing directly across the set, it might not be a good idea to allow this. */
      if (ss->kind == s2x4 && key == 1) {
         uint32 tbctrs =
            ss->people[1].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[6].id1;
         uint32 tbends =
            ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1;

         if ((((ss->people[1].id1 ^ d_south) |   /* Don't allow it if centers are */
               (ss->people[2].id1 ^ d_south) |   /* facing directly accross the set. */
               (ss->people[5].id1 ^ d_north) |
               (ss->people[6].id1 ^ d_north)) & d_mask) &&
             (((tbends & 010) == 0 && (tbctrs & 1) == 0) ||    /* Like an alamo line. */
              ((tbends & 1) == 0 && (tbctrs & 010) == 0))) {   /* Like an alamo column. */
            setup temp = *ss;
            ss->kind = s4x4;
            clear_people(ss);
            scatter(ss, &temp, expander, 7, 0);
            canonicalize_rotation(ss);
         }
      }

      if (ss->kind == s_thar) {
         // Either one is legal in a thar.
         if (key == 1)
            divided_setup_move(ss, MAPCODE(s1x4,2,MPKIND__ALL_8,0), phantest_ok, true, result);
         else
            divided_setup_move(ss, MAPCODE(sdmd,2,MPKIND__ALL_8,0), phantest_ok, true, result);

         /* The above stuff did an "elongate perpendicular to the long axis of the 1x4 or diamond"
            operation, also known as an "ends' part of concentric" operation.  Some people believe
            (perhaps as part of a confusion between "all 8" and "all 4 couples", or some other mistaken
            notion) that they should always end on column spots.  Now it is true that "all 4 couples"
            always ends on columns spots, but that's because it can only begin on column spots.
            To avoid undue controversy or bogosity, we only allow the call if both criteria are met.
            The "opposite elongation" criterion was already met, so we check for the mistaken
            "end on column spots" criterion.  If someone really wants to hinge from a thar, they can
            just say "hinge". */
/*  Not any longer.  Always go to columns.
         if (result->kind == s4x4) {
            if (     ((result->people[1].id1 | result->people[2].id1 | result->people[9].id1 | result->people[10].id1) & 010) ||
                     ((result->people[14].id1 | result->people[5].id1 | result->people[6].id1 | result->people[13].id1) & 1))
               fail("Ending position is not defined.");
         }
         return;
*/
      }
      else if (key == 2)
         fail("Must be in a thar.");   /* Can't do "all 8 (diamonds)" from squared-set spots. */
      else if (ss->kind == s_crosswave) {
         ss->kind = s_thar;
         divided_setup_move(ss, MAPCODE(s1x4,2,MPKIND__ALL_8,0), phantest_ok, true, result);
      }
      else if (ss->kind == s4x4) {
         uint32 t1, t2, tl, tc;

         /* This is "all 8" in a squared-set-type of formation.  This concept isn't really formally
            defined here, except for the well-known cases like "all 8 spin the top", in which they
            would step to a wave and then proceed from the resulting thar.  That is, it is known
            to be legal in facing line-like elongation if everyone steps to a wave.  But it is
            also called from column-like elongation, with everyone facing out, for things like
            "all 8 shakedown", so we want to allow that.

            Perhaps, if people are in line-like elongation, we should have a cmd_misc bit saying
            "must step to a wave" along with the bit saying "must not step to a wave".
            To make matters worse, it might be nice to allow only some people to step to a wave
            in a suitable rigger setup.

            Basically, no one knows exactly how this concept is supposed to work in all the
            cases.  This isn't a problem for anyone except those people who have the misfortune
            to try to write a computer program to do this stuff. */

         /* First, it's clearly only legal if on squared-set spots. */

         if ((    ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1 |
                  ss->people[8].id1 | ss->people[11].id1 | ss->people[12].id1 | ss->people[15].id1) != 0)
            fail("Must be on squared-set spots.");

         /* Next, we remember whether people started in line-like or column-like elongation. */

         t1 = ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1;
         t2 = ss->people[14].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[13].id1;
         tl = t1 | (t2 << 3);         /* If (tl & 010)   then people are in line-like orientation. */
         tc = t2 | (t1 << 3);         /* If (tc & 010)   then people are in column-like orientation. */

         /* If people started out line-like, we demand that they be facing in.  That is, we do not allow
            "all quarter away from your partner, then all 8 shakedown", though we do allow "all
            face your partner and all 8 square chain thru to a wave".  We are just being as conservative
            as possible while allowing those cases that are commonly used. */

         divided_setup_move(ss, MAPCODE(s2x2,2,MPKIND__ALL_8,0), phantest_ok, true, result);
      }
      else
         fail("Must be in a thar or squared-set spots.");
   }

   /* If this ended in a thar, we accept it.  If not, we have the usual lines-to-lines/
      columns-to-columns problem.  We don't know whether to enforce column spots, line spots,
      perpendicular to the lines they had after stepping to a wave (if indeed they did so;
      we don't know), to footprints from before stepping to a wave, or what.  So the only case
      we allow is columns-to-columns. */

   if (result->kind == s_thar)
      return;
   else if (result->kind != s4x4)
      fail("Ending position is not defined.");

   /* Now make sure the "columns-to-columns" rule is applied.  (We know everyone started
      in columns.) */

   if ((result->people[1].id1 & 010) || (result->people[14].id1 & 1))
      swap_people(result, 1, 14);
   if ((result->people[2].id1 & 010) || (result->people[5].id1 & 1))
      swap_people(result, 2, 5);
   if ((result->people[9].id1 & 010) || (result->people[6].id1 & 1))
      swap_people(result, 9, 6);
   if ((result->people[10].id1 & 010) || (result->people[13].id1 & 1))
      swap_people(result, 10, 13);

   /* Check that we succeeded.  If the call ended T-boned, our zeal to get people
      OUT of each others' way may have left people incorrect. */

   if (     ((result->people[1].id1 | result->people[2].id1 | result->people[9].id1 | result->people[10].id1) & 010) ||
            ((result->people[14].id1 | result->people[5].id1 | result->people[6].id1 | result->people[13].id1) & 1))
      fail("People must end as if on column spots.");
}



static void do_concept_meta(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   parse_block *result_of_skip;
   parse_block fudgyblock;
   setup_command nocmd, yescmd;
   uint32 expirations_to_clearmisc = 0;
   uint32 finalresultflagsmisc = 0;
   meta_key_kind key = (meta_key_kind) parseptr->concept->arg1;

   prepare_for_call_in_series(result, ss);

   // We hardly ever care about the "thisislast" bit.
   uint32 corefracs = ss->cmd.cmd_frac_flags & ~CMD_FRAC_THISISLAST;
   uint32 nfield = (corefracs & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT;
   uint32 kfield = (corefracs & CMD_FRAC_PART2_MASK) / CMD_FRAC_PART2_BIT;

   if ((key == meta_key_random || key == meta_key_echo) &&
       ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_REVERSE)) {
      /* "reverse" and "random"  ==>  "reverse random" */
      key = (meta_key_kind) ((int) key+1);
      ss->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_REVERSE);
   }

   // Now demand that no flags remain.
   if (ss->cmd.cmd_final_flags.final)
      fail("Illegal modifier for this concept.");

   nocmd = ss->cmd;
   yescmd = ss->cmd;

   // The CMD_MISC__PUT_FRAC_ON_FIRST bit tells the "special_sequential" concept
   // (if that is the subject concept) that fractions are allowed, and they
   // are to be applied to the first call only.
   yescmd.cmd_misc_flags |= CMD_MISC__PUT_FRAC_ON_FIRST;

   if (key != meta_key_initially && key != meta_key_finally &&
       key != meta_key_initially_and_finally &&
       key != meta_key_piecewise && key != meta_key_nth_part_work &&
       key != meta_key_first_frac_work &&
       key != meta_key_echo && key != meta_key_rev_echo)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;   // We didn't do this before.

   if (key == meta_key_finish) {
      if (corefracs == CMD_FRAC_NULL_VALUE)
         ss->cmd.cmd_frac_flags += FRACS(CMD_FRAC_CODE_FROMTOREV,2,0);
      else if ((corefracs &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) ==
               (CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE))
         ss->cmd.cmd_frac_flags += FRACS(CMD_FRAC_CODE_FROMTOREV,1,1);
      else if ((corefracs &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK      | 0xFFFF)) ==
               (                0 | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE))
         // If we are already doing just parts N and later, just bump N.
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
      else if ((corefracs &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
               ( CMD_FRAC_REVERSE | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE))
         // If we are already doing just parts N and later while reversed, just bump K.
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART2_BIT;
      else if ((corefracs &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
               (CMD_FRAC_CODE_FROMTO | CMD_FRAC_NULL_VALUE))
         ss->cmd.cmd_frac_flags +=
            CMD_FRAC_PART_BIT+CMD_FRAC_PART2_BIT;
      else if ((corefracs &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
               (CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE))
         // If we are already doing just part N only, just bump N.
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
      else if ((corefracs &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
               (CMD_FRAC_CODE_ONLYREV | CMD_FRAC_NULL_VALUE))
         // If we are already doing just part N only in reverse order, do nothing.
         ;
      else if ((corefracs &
                (CMD_FRAC_BREAKING_UP | CMD_FRAC_REVERSE |
                 CMD_FRAC_CODE_MASK | CMD_FRAC_PART_MASK | 0xFFFF)) ==
               (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
                CMD_FRAC_PART_BIT  | CMD_FRAC_NULL_VALUE))
         // If we are already doing up to some part, just "finish" that.
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
      else
         fail("Can't stack meta or fractional concepts.");

      // The "CMD_FRAC_THISISLAST" bit, if still set, might not tell the truth at this point.
      ss->cmd.cmd_frac_flags &= ~CMD_FRAC_THISISLAST;
      move(ss, false, result);
      normalize_setup(result, simple_normalize, false);
      return;
   }
   else if (key == meta_key_like_a) {
      // This is "like a".  Do the last part of the call.
      ss->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_ONLYREV,1,0) | CMD_FRAC_NULL_VALUE;

      // We generally don't allow any incoming fraction stuff.  However,
      // there are two cases that we allow.  If the incoming info was just "1/2",
      // we are being asked to do 1/2 of "like a whatever".  The order in which
      // parts and fractions are specified can't handle that correctly.  (If we
      // just set a fraction field of
      //        FRACS(CMD_FRAC_CODE_ONLYREV,1,0) + CMD_FRAC_HALF_VALUE
      // it would have us do the last part whatever the first half of the call is.)
      // We want the first half of the last part.  But we can do it by using
      // the "CMD_FRAC_FIRSTHALF_ALL" flag, which says to apply the "first half"
      // mechanism *outside* of the part-selecting mechanism.

      if (corefracs == CMD_FRAC_HALF_VALUE)
         ss->cmd.cmd_frac_flags |= CMD_FRAC_FIRSTHALF_ALL;
      else if (corefracs == CMD_FRAC_LASTHALF_VALUE)
         ss->cmd.cmd_frac_flags |= CMD_FRAC_LASTHALF_ALL;
      else if (corefracs != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      move(ss, false, result);
      normalize_setup(result, simple_normalize, false);
      return;
   }

   if (key != meta_key_skip_nth_part &&
       key != meta_key_revorder &&
       key != meta_key_shift_n &&
       key != meta_key_shift_half) {

      // Scan the modifiers, remembering them and their end point.  The reason for this is to
      // avoid getting screwed up by a comment, which counts as a modifier.  YUK!!!!!!
      // This code used to have the beginnings of stuff to do it really right.  It isn't
      // worth it, and isn't worth holding up "random left" for.  In any case, the stupid
      // handling of comments will go away soon.

      skipped_concept_info foo;

      really_skip_one_concept(parseptr->next, foo);

      result_of_skip = *foo.root_of_result_of_skip;
      yescmd.parseptr = foo.old_retval;
      nocmd.parseptr = result_of_skip;

      if ((foo.need_to_restrain & 2) ||
          ((foo.need_to_restrain & 1) &&
           (key != meta_key_rev_echo && key != meta_key_echo))) {
         fudgyblock = *foo.old_retval;
         yescmd.restrained_concept = &fudgyblock;
         yescmd.cmd_misc_flags |= CMD_MISC__RESTRAIN_CRAZINESS;
         yescmd.restrained_final = foo.root_of_result_of_skip;
         yescmd.parseptr = result_of_skip;
      }

      // If the skipped concept is "twisted" or "yoyo", get ready to clear
      // the expiration bit for same, if we do it "piecewise" or whatever.

      if (foo.skipped_concept->concept->kind == concept_yoyo)
         expirations_to_clearmisc = RESULTFLAG__YOYO_EXPIRED;
      if (foo.skipped_concept->concept->kind == concept_twisted)
         expirations_to_clearmisc = RESULTFLAG__TWISTED_EXPIRED;
   }

   // Some concepts take a number, which might be wired into the concept ("shifty"),
   // or might be entered explicitly by the user ("shift <N>").

   uint32 shiftynum =
      (concept_table[parseptr->concept->kind].concept_prop & CONCPROP__USE_NUMBER) ?
      parseptr->options.number_fields : parseptr->concept->arg2;

   switch (key) {
      uint32 frac_flags;
      uint32 index;
      uint32 shortenhighlim;
      uint32 code_to_use_for_only;
      bool doing_just_one;

   case meta_key_revorder:
      result->cmd.cmd_frac_flags ^= CMD_FRAC_REVERSE;
      goto do_less;

   case meta_key_skip_nth_part:
      if (corefracs != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");
   
      // Do the initial part, if any.

      if (parseptr->options.number_fields > 1) {
         /* Set the fractionalize field to do the first few parts of the call. */
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_FROMTO,(parseptr->options.number_fields-1),0) |
            CMD_FRAC_NULL_VALUE;

         do_call_in_series_simple(result);
         result->cmd = ss->cmd;
      }

      // Do the final part.
      result->cmd.cmd_frac_flags =
         FRACS(CMD_FRAC_CODE_FROMTOREV,parseptr->options.number_fields+1,0) |
         CMD_FRAC_NULL_VALUE;
      goto do_less;

   case meta_key_shift_n:
   case meta_key_shift_half:

      // Some form of shift <N>.

      if (key == meta_key_shift_half) {
         shiftynum++;

         if (corefracs != CMD_FRAC_NULL_VALUE)
            fail("Fractional shift doesn't have parts.");

         // Do the last (shifted) part.

         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_LATEFROMTOREV,shiftynum,0) |
            CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;

         do_call_in_series_simple(result);
         result->cmd = ss->cmd;

         // Do the initial part up to the shift point.

         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOMOST |
            (shiftynum * CMD_FRAC_PART_BIT) | CMD_FRAC_NULL_VALUE;
      }
      else {         // Not fractional shift.
         uint32 incoming_break = corefracs & CMD_FRAC_BREAKING_UP;

         if (corefracs == (CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE)) {
            result->cmd.cmd_frac_flags =        // We allow "reverse order".
               CMD_FRAC_BREAKING_UP |
               CMD_FRAC_REVERSE |
               CMD_FRAC_CODE_FROMTOREVREV |
               (shiftynum * CMD_FRAC_PART_BIT) |
               CMD_FRAC_NULL_VALUE;
            // Do the last (shifted) part.
            do_call_in_series_simple(result);
            result->cmd = ss->cmd;

            // Do the initial part up to the shift point.
            result->cmd.cmd_frac_flags =
               FRACS(CMD_FRAC_CODE_FROMTOREV,1,shiftynum) |
               CMD_FRAC_BREAKING_UP | CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE;
         }
         else if ((corefracs & ~(CMD_FRAC_BREAKING_UP|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK)) ==
                  (CMD_FRAC_CODE_FROMTOREV |
                   CMD_FRAC_NULL_VALUE)) {
            result->cmd.cmd_frac_flags =
               incoming_break |
               CMD_FRAC_CODE_FROMTOREV |
               ((shiftynum * CMD_FRAC_PART_BIT) +
                (corefracs & CMD_FRAC_PART_MASK)) |
               CMD_FRAC_NULL_VALUE;

            // See whether we will do the early part.
            if (shiftynum == kfield) {
               goto do_less;    // That's all.
            }
            else if (shiftynum <= kfield)
               fail("Can't stack these meta or fractional concepts.");

            do_call_in_series_simple(result);
            result->cmd = ss->cmd;
            // Do the initial part up to the shift point.
            result->cmd.cmd_frac_flags =
               FRACS(CMD_FRAC_CODE_FROMTO,shiftynum-kfield,0) |
               incoming_break | CMD_FRAC_NULL_VALUE;
         }
         else if ((corefracs & ~(CMD_FRAC_BREAKING_UP|CMD_FRAC_PART_MASK)) ==
                  (CMD_FRAC_CODE_ONLYREV | CMD_FRAC_NULL_VALUE)) {
            if (shiftynum < nfield)
               fail("Can't stack these meta or fractional concepts.");
            result->cmd.cmd_frac_flags =
               incoming_break |
               CMD_FRAC_CODE_ONLY |
               ((shiftynum+1-nfield) * CMD_FRAC_PART_BIT) |
               CMD_FRAC_NULL_VALUE;
         }
         else if ((corefracs & ~(CMD_FRAC_PART_MASK | CMD_FRAC_BREAKING_UP)) ==
                  (CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE) &&
                  nfield != 0) {
            result->cmd.cmd_frac_flags =
               incoming_break |
               CMD_FRAC_CODE_ONLY |
               ((shiftynum+nfield) * CMD_FRAC_PART_BIT) |
               CMD_FRAC_NULL_VALUE;
         }
         else if (corefracs == CMD_FRAC_NULL_VALUE) {
            result->cmd.cmd_frac_flags =
               CMD_FRAC_BREAKING_UP |
               CMD_FRAC_CODE_FROMTOREV |
               ((shiftynum+1) * CMD_FRAC_PART_BIT) |
               CMD_FRAC_NULL_VALUE;
            do_call_in_series_simple(result);
            result->cmd = ss->cmd;
            // Do the initial part up to the shift point.
            result->cmd.cmd_frac_flags =
               CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTO |
               (shiftynum * CMD_FRAC_PART_BIT) | CMD_FRAC_NULL_VALUE;
         }
         else
            fail("Can't stack meta or fractional concepts.");
      }

      goto do_less;

   case meta_key_echo:

      if (!(yescmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS)) {
         if ((corefracs &
              (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART_MASK | 0xFFFF)) ==
             (                    CMD_FRAC_CODE_ONLY | CMD_FRAC_PART_BIT | CMD_FRAC_NULL_VALUE)) {
            finalresultflagsmisc |= RESULTFLAG__PARTS_ARE_KNOWN;
            result->cmd = yescmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if ((corefracs &
              (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART_MASK | 0xFFFF)) ==
             (                    CMD_FRAC_CODE_ONLY | CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE)) {
            finalresultflagsmisc |= RESULTFLAG__PARTS_ARE_KNOWN;
            finalresultflagsmisc |= RESULTFLAG__DID_LAST_PART;
            result->cmd = nocmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if ((corefracs &
              (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART_MASK | 0xFFFF)) ==
             (                    CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE)) {
            finalresultflagsmisc |= RESULTFLAG__PARTS_ARE_KNOWN;
            finalresultflagsmisc |= RESULTFLAG__DID_LAST_PART;
            result->cmd = nocmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if (result->cmd.cmd_frac_flags == CMD_FRAC_HALF_VALUE) {
            result->cmd = yescmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if (result->cmd.cmd_frac_flags == CMD_FRAC_LASTHALF_VALUE) {
            result->cmd = nocmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if (result->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
            fail("Can't stack meta or fractional concepts this way.");
      }

      // Do the call with the concept.
      result->cmd = yescmd;
      do_call_in_series_simple(result);

      // And then again without it.
      result->cmd = nocmd;

      if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
         update_id_bits(result);

      // Assumptions don't carry through.
      result->cmd.cmd_assume.assumption = cr_none;
      goto do_less;

   case meta_key_rev_echo:

      if (!(yescmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS)) {
         if (result->cmd.cmd_frac_flags == CMD_FRAC_HALF_VALUE) {
            result->cmd = nocmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if (result->cmd.cmd_frac_flags == CMD_FRAC_LASTHALF_VALUE) {
            result->cmd = yescmd;
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
            goto do_less;
         }
         else if (result->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
            fail("Can't stack meta or fractional concepts this way.");
      }

      // Do the call without the concept.
      result->cmd = nocmd;
      do_call_in_series_simple(result);

      // And then again with it.
      result->cmd = yescmd;

      // Assumptions don't carry through.
      result->cmd.cmd_assume.assumption = cr_none;
      goto do_less;

   case meta_key_first_frac_work:
      {
         // This is "do the first/last/middle M/N <concept>", or "halfway".

         if (corefracs != CMD_FRAC_NULL_VALUE)
            fail("Can't stack meta or fractional concepts.");

         int code = parseptr->concept->arg2;

         int numer = parseptr->options.number_fields;
         int denom = numer >> 4;
         numer &= 0xF;
         // Check that user isn't doing something stupid.
         if (code != 3 && (numer <= 0 || numer >= denom))
            fail("Illegal fraction.");

         uint32 afracs = ~0UL;
         uint32 bfracs = ~0UL;
         uint32 cfracs = ~0UL;

         if (code == 3) {
            // This is "halfway".
            bfracs = process_spectacularly_new_fractions(0, 1, 1, 2, corefracs);
            cfracs = process_spectacularly_new_fractions(1, 2, 1, 1, corefracs);
         }
         else if (code == 2) {
            // This is "middle M/N".
            afracs = process_spectacularly_new_fractions(0, 1,
                                                         denom-numer, denom<<1, corefracs);
            bfracs = process_spectacularly_new_fractions(denom-numer, denom<<1,
                                                         denom+numer, denom<<1, corefracs);
            cfracs = process_spectacularly_new_fractions(denom+numer, denom<<1,
                                                         1, 1, corefracs);
         }
         else if (code) {
            // This is "last M/N".
            afracs = process_spectacularly_new_fractions(0, 1, denom-numer, denom, corefracs);
            bfracs = process_spectacularly_new_fractions(denom-numer, denom, 1, 1, corefracs);
         }
         else {
            // This is "first M/N".
            bfracs = process_spectacularly_new_fractions(0, 1, numer, denom, corefracs);
            cfracs = process_spectacularly_new_fractions(numer, denom, 1, 1, corefracs);
         }

         // Do afracs without.
         if (afracs != ~0UL) {
            result->cmd = nocmd;
            result->cmd.parseptr = result_of_skip;      // Skip over the concept.
            result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | afracs;
            do_call_in_series_simple(result);
         }

         // Do bfracs with.
         if (bfracs != ~0UL) {
            result->cmd = yescmd;
            result->cmd.cmd_frac_flags = (corefracs & ~0xFFFF) |
               CMD_FRAC_BREAKING_UP | bfracs;
            do_call_in_series_simple(result);
         }

         // Do cfracs without.
         if (cfracs != ~0UL) {
            result->cmd = nocmd;
            result->cmd.parseptr = result_of_skip;      // Skip over the concept.
            result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | cfracs;
            do_call_in_series_simple(result);
         }

         goto final_fixup;
      }
   case meta_key_nth_part_work:
      {
      // This is "do the Nth part <concept>".

      if (corefracs != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      // Do the initial part, if any, without the concept.

      if (shiftynum > 1) {
         result->cmd = nocmd;
         // Set the fractionalize field to do the first few parts of the call.
         result->cmd.cmd_frac_flags =
            ((shiftynum-1) * CMD_FRAC_PART_BIT) |
            CMD_FRAC_CODE_FROMTO | CMD_FRAC_NULL_VALUE;
         do_call_in_series_simple(result);
      }

      // Do the part of the call that needs the concept.

      result->cmd = yescmd;
      result->cmd.cmd_frac_flags =
         FRACS(CMD_FRAC_CODE_ONLY,shiftynum,0) |
         CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;

      do_call_in_series_simple(result);

      int did_last = 0;
      if (result->result_flags.misc & RESULTFLAG__PARTS_ARE_KNOWN)
         did_last = result->result_flags.misc & RESULTFLAG__DID_LAST_PART;

      // If the parts were not known, go ahead anyway, and hope for the best.
      // It is very likely that the code below will respond correctly
      // to being told to do the rest of the call even if there isn't
      // any more.  It's only for really pathological stuff like
      // "thirdly use hinge, 3/4 crazy mix" which makes no sense anyway,
      // that it would lose.
      // We need this for "secondly use acey deucey in swing the fractions".

      // Do the final part, if there is more.
      if (!did_last) {
         result->cmd = ss->cmd;
         result->cmd.parseptr = result_of_skip;      // Skip over the concept.
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_FROMTOREV,shiftynum+1,0) | CMD_FRAC_NULL_VALUE;
         goto do_less;
      }

      goto get_out;
      }
   case meta_key_initially:

      // This is "initially": we select the first part with the concept,
      // and then the rest of the call without the concept.

      if (corefracs == (FRACS(CMD_FRAC_CODE_ONLY,1,0) |
                        CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE)) {
         /* We are being asked to do just the first part, because of another
            "initially".  Just pass it through. */

         result->cmd = yescmd;
      }
      else if (
               // Being asked to do all but the first part.
               (corefracs == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
                              CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE)) ||

               // Being asked to do some specific part other than the first.
               ((corefracs & ~CMD_FRAC_PART_MASK) ==
                (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE) &&
                nfield >= 2) ||

               // Being asked to do only the last part -- it's a safe bet that
               // that isn't the first part.
               corefracs == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
                             CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         // In any case, just pass it through.
         result->cmd.parseptr = result_of_skip;
      }
      else if (corefracs == (FRACS(CMD_FRAC_CODE_FROMTOREV,1,1) |
                             CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do all but the last part.  Do the first part
         // with the concept, then all but first and last without it.

         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_ONLY,1,0) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;

         do_call_in_series_simple(result);
         result->cmd = nocmd;
         // Assumptions don't carry through.
         result->cmd.cmd_assume.assumption = cr_none;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_FROMTOREV,2,1) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
      }
      else if (corefracs ==
               (FRACS(CMD_FRAC_CODE_FROMTOREV,2,1) |
                CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do just the inner parts, presumably because of an
         // "initially" and "finally".  Just pass it through.
         result->cmd = nocmd;
      }
      else if ((corefracs & ~CMD_FRAC_PART_MASK) ==
               (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTO | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do an initial subset that includes the first part.
         // Do the first part, then do the rest of the subset.

         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_ONLY,1,0) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;

         // The first part, with the concept.
         do_call_in_series_simple(result);
         result->cmd = nocmd;
         // Assumptions don't carry through.
         result->cmd.cmd_assume.assumption = cr_none;
         // nfield = incoming end point; skip one at start.
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_FROMTO,nfield,1) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
      }
      else if (corefracs == CMD_FRAC_NULL_VALUE) {
         // Do the first part with the concept.
         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_ONLY,1,0) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         result->result_flags.misc |= RESULTFLAG__EXPIRATION_ENAB;
         do_call_in_series_simple(result);

         // And the rest of the call without it.

         result->cmd = nocmd;
         result->cmd.cmd_assume.assumption = cr_none;  /* Assumptions don't carry through. */
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_FROMTOREV,2,0) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
      }
      else
         fail("Can't stack meta or fractional concepts.");

      goto do_less;

   case meta_key_finally:

      // This is "finally": we select all but the last part without the concept,
      // and then the last part with the concept.

      if (corefracs == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
                        CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do just the last part, because of another "finally".
         // Just pass it through.

         result->cmd = yescmd;
      }
      else if (corefracs == (FRACS(CMD_FRAC_CODE_FROMTOREV,1,1) |
                             CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do all but the last part,
         // because of another "finally".  Just pass it through.

         result->cmd = nocmd;
      }
      else if (corefracs == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY |
                             CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do just the first part,
         // presumably because of an "initially".  Just pass it through.

         result->cmd = nocmd;
      }
      else if (corefracs ==
               (FRACS(CMD_FRAC_CODE_FROMTOREV,2,1) |
                CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do just the inner parts, presumably because of
         // an "initially" and "finally".  Just pass it through.

         result->cmd = nocmd;
      }
      else if (corefracs == (FRACS(CMD_FRAC_CODE_FROMTOREV,2,0) |
                             CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do all but the first part, presumably because of
         // an "initially".  Do all but first and last normally, then do last
         // with the concept.

         // Change it to do all but first and last.

         result->cmd = nocmd;
         result->cmd.cmd_frac_flags += CMD_FRAC_PART2_BIT;
         do_call_in_series_simple(result);
         result->cmd = yescmd;
         result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_ONLYREV,1,0) |
            CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         // Assumptions don't carry through.
         result->cmd.cmd_assume.assumption = cr_none;
      }
      else if ((ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
               (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY |
                CMD_FRAC_THISISLAST | CMD_FRAC_NULL_VALUE)) {

         // We are being asked to do some part, and we normally wouldn't know
         // whether it is last, so we would be screwed.  However, we have been
         // specifically told that it is last.  So we just pass it through.

         result->cmd = yescmd;
      }
      else if (corefracs == CMD_FRAC_NULL_VALUE) {

         // Do the call without the concept.
         // Set the fractionalize field to execute all but the last part of the call.

         result->cmd = nocmd;
         result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_FROMTOREV,1,1) |
            CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         do_call_in_series_simple(result);
         // Do the call with the concept.
         // Set the fractionalize field to execute the last part of the call.
         result->cmd = yescmd;
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
            CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE;
         // Assumptions don't carry through.
         result->cmd.cmd_assume.assumption = cr_none;
         result->result_flags.misc |= RESULTFLAG__EXPIRATION_ENAB;
      }
      else
         fail("Can't stack meta or fractional concepts.");

      goto do_less;

   case meta_key_initially_and_finally:

      // This is "initially and finally": we select the first part with the concept,
      // the interior part without the concept,
      // and then the last part with the concept.
      if (corefracs == CMD_FRAC_NULL_VALUE) {   // The only case we care about.

         // Do the first part with the concept,

         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_ONLY,1,0) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         result->result_flags.misc |= RESULTFLAG__EXPIRATION_ENAB;
         do_call_in_series_simple(result);

         // the interior of the call without it,

         result->cmd = nocmd;
         // Assumptions don't carry through.
         result->cmd.cmd_assume.assumption = cr_none;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_FROMTOREV,2,1) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         result->result_flags.misc |= RESULTFLAG__EXPIRATION_ENAB;
         do_call_in_series_simple(result);

         // and the last part with it.  Be sure to reset the "twisted"/"yoyo" expiration.

         result->cmd = yescmd;
         result->result_flags.misc &= ~expirations_to_clearmisc;

         // Assumptions don't carry through.
         result->cmd.cmd_assume.assumption = cr_none;
         result->cmd.cmd_frac_flags =
            FRACS(CMD_FRAC_CODE_ONLYREV,1,0) | CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
         result->result_flags.misc |= RESULTFLAG__EXPIRATION_ENAB;
      }
      else
         fail("Can't stack meta or fractional concepts.");

      goto do_less;

   default:

      // Otherwise, this is the "random", "reverse random", or "piecewise" concept.
      // Repeatedly execute parts of the call, skipping the concept where required.

      index = 0;
      shortenhighlim = 0;
      doing_just_one = false;
      code_to_use_for_only = CMD_FRAC_CODE_ONLY;

      // We allow picking a specific part, and we allow "finishing" from a specific
      // part, but we allow nothing else.

      if ((corefracs &
           (CMD_FRAC_BREAKING_UP | CMD_FRAC_IMPROPER_BIT |
            CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) ==
          (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE)) {
         index = nfield - 1;
         doing_just_one = true;
      }
      else if ((corefracs &
           (CMD_FRAC_BREAKING_UP | CMD_FRAC_IMPROPER_BIT |
            CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) ==
          (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV | CMD_FRAC_NULL_VALUE)) {
         index = nfield - 1;
         doing_just_one = true;
         code_to_use_for_only = CMD_FRAC_CODE_ONLYREV;
         fail("Sorry, can't do this.");
      }
      else if ((corefracs &
           (CMD_FRAC_BREAKING_UP | CMD_FRAC_IMPROPER_BIT |
            CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
          (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE)) {
         index = nfield - 1;
         shortenhighlim = kfield;
      }
      else if (corefracs != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      frac_flags = corefracs &
         ~(CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK);

      uint32 saved_last_flagmisc = 0;

      do {
         // Here is where we make use of actual numerical assignments.
         uint32 revrand = key-meta_key_random;

         index++;
         if (index > 7) fail("Sorry, can't handle this number.");
         copy_cmd_preserve_elong_and_expire(ss, result);

         // If concept is "[reverse] random" and this is an even/odd-numbered part,
         // as the case may be, skip over the concept.
         if (((revrand & ~1) == 0) && ((index & 1) == revrand)) {
            // But how do we skip the concept?  If it an ordinary single-call concept,
            // it's easy.  But, if the concept takes a second call (the only legal case
            // of this being "concept_special_sequential") we use its second subject call
            // instead of the first.  This is part of the peculiar behavior of this
            // particular combination.

            result->cmd = nocmd;
         }
         else {
            result->cmd = yescmd;
            result->result_flags.misc &= ~expirations_to_clearmisc;
         }

         // Set the fractionalize field to do the indicated part.
         result->cmd.cmd_frac_flags = FRACS(code_to_use_for_only,index,shortenhighlim) |
            CMD_FRAC_BREAKING_UP | frac_flags;

         // If the call that we are doing has the RESULTFLAG__NO_REEVALUATE flag
         // on (meaning we don't re-evaluate under *any* circumstances, particularly
         // circumstances like these), we do not re-evaluate the ID bits.
         if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
            update_id_bits(result);

         result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;

         do_call_in_series(result, true, false, true, false);

         if (result->result_flags.misc & RESULTFLAG__SECONDARY_DONE) {
            index = 0;
            frac_flags |= CMD_FRAC_IMPROPER_BIT;
         }

         result->result_flags.misc &= ~RESULTFLAG__SECONDARY_DONE;

         // Now look at the "DID_LAST_PART" bits of the call we just executed,
         // to see whether we should break out.
         if (!(result->result_flags.misc & RESULTFLAG__PARTS_ARE_KNOWN)) {
            // Problem.  The call doesn't know what it did.
            // Perhaps the previous part knew.  This could arise if we
            // do something like "random use flip back in swing the fractions" --
            // various parts aren't actually done, but enough parts are done
            // for us to piece together the required information.

            result->result_flags.misc &= ~RESULTFLAG__PART_COMPLETION_BITS;

            if ((saved_last_flagmisc & (RESULTFLAG__PARTS_ARE_KNOWN|RESULTFLAG__DID_LAST_PART))
                == RESULTFLAG__PARTS_ARE_KNOWN) {
               result->result_flags.misc |= RESULTFLAG__PARTS_ARE_KNOWN;
               if (saved_last_flagmisc & RESULTFLAG__DID_NEXTTOLAST_PART)
                  result->result_flags.misc |= RESULTFLAG__DID_LAST_PART;
            }

            saved_last_flagmisc = 0;   // Need to recharge before can use it again.
         }
         else
            saved_last_flagmisc = result->result_flags.misc;

         if (!(result->result_flags.misc & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");

         // We will pass all those bits back to our client.
         if (doing_just_one) break;
      }
      while (!(result->result_flags.misc & RESULTFLAG__DID_LAST_PART));

      goto get_out;
   }

   goto get_out;

 do_less:

   // This seems to make t50 work.
   result->cmd.prior_expire_bits |= result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
   do_call_in_series_simple(result);
 final_fixup:
   result->result_flags.misc |= finalresultflagsmisc;

 get_out: ;
}


static void do_concept_replace_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   // Values of arg1 are:
   //    0 - replace last part
   //    1 - interrupt before last part
   //    2 - interrupt after given fraction
   //    3 - "sandwich" - interrupt after 1/2
   //    8 - replace Nth part
   //    9 = interrupt after Nth part

   int concept_key = parseptr->concept->arg1;
   uint32 incoming_fracs = ss->cmd.cmd_frac_flags;

   ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;

   // We will modify these flags, and, in any case,
   // we need to rematerialize them at each step.
   setup_command cmd = ss->cmd;

   prepare_for_call_in_series(result, ss);

   uint32 fracs_for_middle_part = CMD_FRAC_NULL_VALUE;

   // Fill in some default values that might be useful.

   uint32 frac_key1 = FRACS(CMD_FRAC_CODE_FROMTOREV,1,1) |
      CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;

   uint32 frac_key2 = FRACS(CMD_FRAC_CODE_FROMTOREV,parseptr->options.number_fields+1,0) |
      CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;

   bool skip_replacement_call = false;

   switch (concept_key) {
   case 0:
      frac_key2 = 0;
      break;
   case 1:
      frac_key2 = FRACS(CMD_FRAC_CODE_ONLY,1,0) |
         CMD_FRAC_BREAKING_UP | CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE;
      break;
   case 2:
      // "Interrupt after M/N".
      frac_key1 = process_spectacularly_new_fractions(0, 1,
                                                      parseptr->options.number_fields & 0xF,
                                                      (parseptr->options.number_fields >> 4) & 0xF,
                                                      ss->cmd.cmd_frac_flags);
      frac_key2 = ((frac_key1 & 0xFF)<<8) | 0x0011;
      break;
   case 3:
      // "Sandwich".
      frac_key1 = process_spectacularly_new_fractions(0, 1, 1, 2, ss->cmd.cmd_frac_flags);
      frac_key2 = ((frac_key1 & 0xFF)<<8) | 0x0011;
      break;
   case 8:
      // Replace Nth part.
      frac_key1 = FRACS(CMD_FRAC_CODE_FROMTO,parseptr->options.number_fields-1,0) |
         CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
      break;
   case 9:
      // Interrupt after Nth part.
      frac_key1 = FRACS(CMD_FRAC_CODE_FROMTO,parseptr->options.number_fields,0) |
         CMD_FRAC_BREAKING_UP | CMD_FRAC_NULL_VALUE;
      break;
   }

   // Handle Curried meta-concept on "sandwich".  "Sandwich" is the only concept
   // we will see that can use a Curried meta-concept.

   if (concept_key == 3 &&
       (cmd.cmd_misc_flags & (CMD_MISC__PUT_FRAC_ON_FIRST|CMD_MISC__RESTRAIN_CRAZINESS)) ==
       CMD_MISC__PUT_FRAC_ON_FIRST) {
      cmd.cmd_misc_flags &= ~CMD_MISC__PUT_FRAC_ON_FIRST;
      fracs_for_middle_part = incoming_fracs;
      incoming_fracs = CMD_FRAC_NULL_VALUE;
   }

   // Handle special fractionalization commands on "sandwich".  These are the
   // only instances of fractionalization that we allow on these concepts.

   if (concept_key == 3 &&
       (incoming_fracs &
        (CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK|0xFFFF)) ==
       (FRACS(CMD_FRAC_CODE_ONLYREV,1,0) | CMD_FRAC_NULL_VALUE)) {
      // Do the last part only.
      frac_key1 = FRACS(CMD_FRAC_CODE_FROMTO,0,0);
      skip_replacement_call = true;
   }
   else if (concept_key == 3 &&
            (incoming_fracs &
             (CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK|0xFFFF)) ==
            (FRACS(CMD_FRAC_CODE_FROMTOREV,1,1) | CMD_FRAC_NULL_VALUE)) {
      // Don't do the last part.
      frac_key2 = 0;
   }
   else if (incoming_fracs != CMD_FRAC_NULL_VALUE)
      fail("Can't stack meta or fractional concepts.");

   // Do the initial part, if any, without the concept.  Use frac_key1.
   // If frac_key1 is CMD_FRAC_CODE_FROMTO with K=0 and N=0, there is no first part.

   if ((frac_key1 & (CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK)) !=
       FRACS(CMD_FRAC_CODE_FROMTO,0,0)) {
      cmd.cmd_frac_flags = frac_key1;

      ss->cmd = cmd;

      // Lift the craziness restraint from before -- we are about to pull things apart.
      // But only if it is "sandwich".
      if (concept_key == 3) {
         if (cmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS) {
            cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;
            cmd.restrained_fraction = cmd.cmd_frac_flags;
            cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
         }
      }

      copy_cmd_preserve_elong_and_expire(ss, result);
      result->cmd.prior_expire_bits |= RESULTFLAG__EXPIRATION_ENAB;
      // This seems to make t50 work.
      result->cmd.prior_expire_bits |=
         result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
      do_call_in_series(result, true, false, true, false);
   }

   // Do the interruption/replacement call.

   if (!skip_replacement_call) {
      cmd.cmd_frac_flags = fracs_for_middle_part;
      ss->cmd = cmd;
      result->cmd = ss->cmd;
      result->cmd.parseptr = parseptr->subsidiary_root;
      if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
         update_id_bits(result);    // So you can interrupt with "leads run", etc.
      result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;

      // Give this call a clean start with respect to expiration stuff.
      uint32 suspended_expiration_bitsmisc =
         result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
      result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;
      do_call_in_series(result, true, false, true, false);

      // Put back the expiration bits for the resumed call.
      result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;
      result->result_flags.misc |= suspended_expiration_bitsmisc|RESULTFLAG__EXPIRATION_ENAB;
   }

   // Do the final part, if there is more.  Use frac_key2.

   if (frac_key2 != 0) {
      cmd.cmd_frac_flags = frac_key2;
      ss->cmd = cmd;





      // Lift the craziness restraint from before -- we are about to pull things apart.
      // But only if it is "sandwich".
      if (concept_key == 3) {
         if (cmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS) {
            cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;
            cmd.restrained_fraction = cmd.cmd_frac_flags;
            cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
         }

         ss->cmd = cmd;
      }



      copy_cmd_preserve_elong_and_expire(ss, result);
      // This seems to make t50 work.
      result->cmd.prior_expire_bits |=
         result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;
      do_call_in_series(result, true, false, true, false);
   }
}


static void do_concept_interlace(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   /* We don't allow any other fractional stuff. */

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
      fail("Can't stack meta or fractional concepts.");

   uint32 first_doneflagmisc = 0;
   uint32 second_doneflagmisc = 0;
   uint32 indexa = 0;
   uint32 indexb = 0;

   prepare_for_call_in_series(result, ss);
   uint32 a_frac_flags = CMD_FRAC_NULL_VALUE;
   uint32 b_frac_flags = CMD_FRAC_NULL_VALUE;
   uint32 calla_expiration_bitsmisc = 0;
   uint32 callb_expiration_bitsmisc = 0;

   // If yoyo coming in, set callb_expiration, so only calla will do it.

   if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_YOYO))
      callb_expiration_bitsmisc |= RESULTFLAG__YOYO_EXPIRED;
   if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_TWISTED))
      callb_expiration_bitsmisc |= RESULTFLAG__TWISTED_EXPIRED;
   if (ss->cmd.cmd_final_flags.test_finalbit(FINAL__SPLIT_SQUARE_APPROVED))
      callb_expiration_bitsmisc |= RESULTFLAG__SPLIT_EXPIRED;

   // Well, actually, what we want to do it not carry the stuff around,
   // that is, not clear and restore.

   do {
      indexa++;

      if (!(first_doneflagmisc & RESULTFLAG__DID_LAST_PART)) {
         // Do the indicated part of the first call.
         copy_cmd_preserve_elong_and_expire(ss, result, true);
         result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;
         result->result_flags.misc |= calla_expiration_bitsmisc & RESULTFLAG__EXPIRATION_BITS;

         // If we know that this part is in fact last, we say so.  That way,
         // if the subject call is an instance of "finally", it will work.

         result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_ONLY,indexa,0) |
            CMD_FRAC_BREAKING_UP | a_frac_flags;

         if (first_doneflagmisc & RESULTFLAG__DID_NEXTTOLAST_PART)
            result->cmd.cmd_frac_flags |= CMD_FRAC_THISISLAST;

         if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
            update_id_bits(result);
         result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;

         // This seems to make t50 work.
         result->cmd.prior_expire_bits |= result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;

         do_call_in_series(result, true, false, true, false);
         calla_expiration_bitsmisc = result->result_flags.misc;

         if (!(result->result_flags.misc & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");

         if (result->result_flags.misc & RESULTFLAG__SECONDARY_DONE) {
            indexa = 0;
            a_frac_flags |= CMD_FRAC_IMPROPER_BIT;
         }

         first_doneflagmisc = result->result_flags.misc;
         result->result_flags.misc &= ~RESULTFLAG__SECONDARY_DONE;  /* **** need this? */
      }
      else if (!(second_doneflagmisc & RESULTFLAG__DID_LAST_PART))
         warn(warn__bad_interlace_match);

      indexb++;

      if (!(second_doneflagmisc & RESULTFLAG__DID_LAST_PART)) {
         // Do the indicated part of the second call.
         copy_cmd_preserve_elong_and_expire(ss, result, true);
         result->cmd.parseptr = parseptr->subsidiary_root;
         result->result_flags.misc &= ~RESULTFLAG__EXPIRATION_BITS;
         result->result_flags.misc |= callb_expiration_bitsmisc & RESULTFLAG__EXPIRATION_BITS;

         // If other part has run out, and we aren't doing an improper fraction,
         // just finish this.
         if ((first_doneflagmisc & RESULTFLAG__DID_LAST_PART) &&
             !(second_doneflagmisc & RESULTFLAG__SECONDARY_DONE)) {
            result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_FROMTOREV,indexb,0) |
               CMD_FRAC_BREAKING_UP | b_frac_flags;

            // This seems to make t50 work.
            result->cmd.prior_expire_bits |= result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;

            do_call_in_series(result, true, false, true, false);
            return;
         }

         result->cmd.cmd_frac_flags = FRACS(CMD_FRAC_CODE_ONLY,indexb,0) |
            CMD_FRAC_BREAKING_UP | b_frac_flags;

         // This seems to make t50 work.
         result->cmd.prior_expire_bits |= result->result_flags.misc & RESULTFLAG__EXPIRATION_BITS;

         do_call_in_series(result, true, false, true, false);
         callb_expiration_bitsmisc = result->result_flags.misc;

         if (!(result->result_flags.misc & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");

         if (result->result_flags.misc & RESULTFLAG__SECONDARY_DONE) {
            indexb = 0;
            b_frac_flags |= CMD_FRAC_IMPROPER_BIT;
         }

         second_doneflagmisc = result->result_flags.misc;
         result->result_flags.misc &= ~RESULTFLAG__SECONDARY_DONE;  /* **** need this? */
      }
      else if (!(first_doneflagmisc & RESULTFLAG__DID_LAST_PART))
         warn(warn__bad_interlace_match);
   }
   while ((first_doneflagmisc & second_doneflagmisc & RESULTFLAG__DID_LAST_PART) == 0);
}


static void do_concept_fractional(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   /* Note: if we ever implement something that omits the first fraction, that
      concept would have to have "CONCPROP__NO_STEP" set in concept_table, and
      things might get ugly.
   Actually, we have implemented exactly that -- "do last fraction", and we
      have not set CONCPROP__NO_STEP in the concept table.  The user is responsible
      for the consequences of using this. */

   // The meaning of arg1 is as follows:
   // 0 - "M/N" - do first part
   // 1 - "DO THE LAST M/N"
   // 2 - "1-M/N" do the whole call and then some.
   // 4 - first half
   // 5 - last half
   // 90 - special

   uint32 ARG4 = ss->cmd.cmd_frac_flags;
   uint32 FOO;

   if (parseptr->concept->arg1 == 90) {
      FOO = ss->cmd.restrained_fraction;
      ss->cmd.restrained_fraction = 0;
   }
   else {
      int nn = (parseptr->concept->arg1 >= 4) ?
         0x21 :               // Canned number fields as though fraction were 1/2.
         parseptr->options.number_fields;

      int dd = nn >> 4;
      nn &= 0xF;
      FOO = (parseptr->concept->arg1 & 1) ?
         ((dd-nn) << 12)+(dd << 8)+0x0011 : (nn << 4)+(dd << 0)+0x0100;
   }

   int cn = (FOO >> 12) & 0xF;
   int cd = (FOO >> 8) & 0xF;
   int dn = (FOO >> 4) & 0xF;
   int dd = (FOO) & 0xF;

   bool improper = false;
   uint32 new_fracs = process_spectacularly_new_fractions(cn, cd, dn, dd,
                                                          ARG4,
                                                          parseptr->concept->arg1 == 2,
                                                          &improper);

   if (improper) {
      // Do the whole call first, then part of it again.

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"invert/central/snag/mystic\" followed by this concept or modifier.");

      if ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) != 0 &&
          ((ss->cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK|CMD_FRAC_BREAKING_UP)) ==
           (CMD_FRAC_CODE_ONLY|CMD_FRAC_BREAKING_UP))) {

         // We are being asked to do just one part of "1-3/5 swing the fractions".

         ss->cmd.cmd_frac_flags &= ~0xFFFF;  // Will pick up the low 16 bits shortly.

         if ((ss->cmd.cmd_frac_flags & CMD_FRAC_IMPROPER_BIT) == 0) {
            // First time around -- Do the indicated part of a whole swing the fractions.

            ss->cmd.cmd_frac_flags |= CMD_FRAC_NULL_VALUE;
            move(ss, false, result);
            // And, if it reported that it did the last part, change it to
            // "secondary_done" to indicate that we're not finished.
            if (result->result_flags.misc & RESULTFLAG__DID_LAST_PART) {
               result->result_flags.misc &= ~RESULTFLAG__DID_LAST_PART;
               result->result_flags.misc |= RESULTFLAG__SECONDARY_DONE;
            }
         }
         else {
            // Second time around -- just do the fractional part.
            ss->cmd.cmd_frac_flags |= new_fracs;
            move(ss, false, result);
         }
      }
      else {
         prepare_for_call_in_series(result, ss);
         result->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | CMD_FRAC_NULL_VALUE;
         do_call_in_series(result, false, false,
            !(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX),
            false);

         copy_cmd_preserve_elong_and_expire(ss, result);
         result->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | new_fracs;

         if ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) >= (CMD_FRAC_PART_BIT*2) &&
                  ((ss->cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK|CMD_FRAC_BREAKING_UP)) ==
                   (CMD_FRAC_CODE_FROMTOREV|CMD_FRAC_BREAKING_UP))) {
   
            // We are being asked to do everything beyond some part of
            // "1-3/5 swing the fractions".
            result->cmd.cmd_frac_flags &= 0xFFFF;   // Do the whole 3/5.
         }

         if (!(result->result_flags.misc & RESULTFLAG__NO_REEVALUATE))
            update_id_bits(result);
         result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;

         do_call_in_series(result, false, false,
            !(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX),
            false);
      }
   }
   else {
      ss->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | new_fracs;
      move(ss, false, result);
   }
}


static void do_concept_so_and_so_begin(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   selector_kind saved_selector;
   int i;
   setup setup1, setup2;
   setup the_setups[2];

   // See if we are being requested to do only stuff that does not
   // include the first part.  If so, we just do it.

   // We check for either part selection indicating the first part isn't involved ...
   if ((((ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) == CMD_FRAC_CODE_ONLY ||
        (ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) == CMD_FRAC_CODE_FROMTOREV) &&
       (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) >= CMD_FRAC_PART_BIT*2)
       ||
       // or fraction selection indicating the same.
       // That is, code="only", part=0, reverse off, and starting frac != 0.
       (((ss->cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_REVERSE))
         == CMD_FRAC_CODE_ONLY)
        && (ss->cmd.cmd_frac_flags & 0xF000) != 0)) {
      move(ss, false, result);
      return;
   }

   // At this point we know that we will do stuff including the first part,
   // which is the part that requires special action.

   saved_selector = current_options.who;
   current_options.who = parseptr->options.who;

   setup1 = *ss;              /* designees */
   setup2 = *ss;              /* non-designees */
   
   if (attr::slimit(ss) < 0) fail("Can't identify people in this setup.");
   for (i=0; i<=attr::slimit(ss); i++) {
      if (ss->people[i].id1) {
         if (selectp(ss, i))
            clear_person(&setup2, i);
         else
            clear_person(&setup1, i);
      }
   }
   
   current_options.who = saved_selector;
   
   normalize_setup(&setup1, normalize_before_isolated_call, false);
   normalize_setup(&setup2, normalize_before_isolated_call, false);

   // We just feed the "reverse" bit and the fractional stuff (low 16 bits)
   // through.  They have no effect on what we are doing.

   uint32 important_bits = ss->cmd.cmd_frac_flags & ~(CMD_FRAC_REVERSE|0xFFFF);
   uint32 passed_bits = ss->cmd.cmd_frac_flags &
      ~(CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK);

   // Set the fractionalize field to execute the first part of the call.

   setup1.cmd.cmd_frac_flags = CMD_FRAC_CODE_ONLY | CMD_FRAC_PART_BIT*1 | passed_bits;

   // The selected people execute the first part of the call.

   move(&setup1, false, &the_setups[0]);
   the_setups[1] = setup2;

   // Give the people who didn't move the same result flags as those who did.
   // This is important for the "did last part" check.
   the_setups[1].result_flags = the_setups[0].result_flags;
   the_setups[1].result_flags = get_multiple_parallel_resultflags(the_setups, 2);

   merge_setups(&the_setups[0], merge_c1_phantom, &the_setups[1]);
   uint32 finalresultflagsmisc = the_setups[1].result_flags.misc;
   normalize_setup(&the_setups[1], simple_normalize, false);

   if ((important_bits & CMD_FRAC_CODE_MASK) != CMD_FRAC_CODE_ONLY)
      fail("Can't stack meta or fractional concepts.");

   // Now we know that the code is "ONLY".  If N=0, this is the null operation;
   // just finish the call.  If N=1, we were doing only the first part; exit now.
   // If N>=2, we already exited.

   if ((important_bits & CMD_FRAC_PART_MASK) == CMD_FRAC_PART_BIT) {
      *result = the_setups[1];
      result->result_flags.misc |= finalresultflagsmisc;
      result->result_flags.misc &= ~3;
      return;
   }

   the_setups[1].cmd = ss->cmd; // Just in case it got messed up, which shouldn't have happened.

   // Set the fractionalize field to execute the rest of the call.
   the_setups[1].cmd.cmd_frac_flags =
      CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_PART_BIT*2 | passed_bits;
   move(&the_setups[1], false, result);
   result->result_flags.misc = finalresultflagsmisc;
   result->result_flags.misc &= ~3;
   normalize_setup(result, simple_normalize, false);
}



static void do_concept_concentric(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   calldef_schema schema = (calldef_schema) parseptr->concept->arg1;

   if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_CROSS)) {
      switch (schema) {
      case schema_concentric:
         schema = schema_cross_concentric;
         break;
      case schema_single_concentric:
         schema = schema_single_cross_concentric;
         break;
      case schema_grand_single_concentric:
         schema = schema_grand_single_cross_concentric;
         break;
      case schema_concentric_diamonds:
         schema = schema_cross_concentric_diamonds;
         break;
      case schema_concentric_zs:
         schema = schema_cross_concentric_zs;
         break;
      default:
         fail("Redundant 'CROSS' modifiers.");
      }

      if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_SINGLE)) {
         switch (schema) {
            case schema_concentric:
               schema = schema_single_concentric;
               break;
            case schema_cross_concentric:
               schema = schema_single_cross_concentric;
               break;
            default:
               fail("Redundant 'SINGLE' modifiers.");
         }
      }

      if (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_GRAND)) {
         switch (schema) {
            case schema_single_concentric:
               schema = schema_grand_single_concentric;
               break;
            case schema_single_cross_concentric:
               schema = schema_grand_single_cross_concentric;
               break;
            default:
               fail("Redundant 'GRAND' modifiers.");
         }
      }

      ss->cmd.cmd_final_flags.clear_heritbits(INHERITFLAG_CROSS |
                                              INHERITFLAG_SINGLE |
                                              INHERITFLAG_GRAND);

      // We don't allow other flags, like "left", though we do allow "half".

      if ((ss->cmd.cmd_final_flags.test_heritbits(~(INHERITFLAG_HALF | INHERITFLAG_LASTHALF))) |
          ss->cmd.cmd_final_flags.final)
         fail("Illegal modifier before \"concentric\".");
   }

   switch (schema) {
      uint32 map_code;
   case schema_single_concentric:
   case schema_single_cross_concentric:
      switch (ss->kind) {
      case s2x4:   map_code = MAPCODE(s1x4,2,MPKIND__SPLIT,1); break;
      case s1x8:   map_code = MAPCODE(s1x4,2,MPKIND__SPLIT,0); break;
      case s_qtag: map_code = MAPCODE(sdmd,2,MPKIND__SPLIT,1); break;
      case s_ptpd: map_code = MAPCODE(sdmd,2,MPKIND__SPLIT,0); break;
      case s1x4: case sdmd:
         concentric_move(ss, &ss->cmd, &ss->cmd, schema, 0,
                         DFM1_CONC_CONCENTRIC_RULES, true, ~0UL, result);
         return;
      default:
         fail("Can't figure out how to do single concentric here.");
      }
      
      // Reset it to execute this same concept again, until it doesn't have to split any more.
      ss->cmd.parseptr = parseptr;
      divided_setup_move(ss, map_code, phantest_ok, true, result);
      break;
   default:
      concentric_move(ss, &ss->cmd, &ss->cmd, schema, 0,
                      DFM1_CONC_CONCENTRIC_RULES, true, ~0UL, result);

      // 8-person concentric operations do not show the split.
      result->result_flags.clear_split_info();
      break;
   }
}



/* ARGSUSED */
static void do_concept_standard(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This should never be called this way -- "standard" is treated specially.
      But we do need a nonzero entry in the dispatch table. */
   fail("Huh? -- standard concept out of place.");
}

static void complain_about_in_setup(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   fail("This concept must be preceded by \"as couples\" or \"tandem\" type of concept.");
}



/* ARGSUSED */
static void do_concept_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   ss->cmd.cmd_misc_flags |= CMD_MISC__MATRIX_CONCEPT;
   // The above flag will raise an error in all contexts except
   // those few to which this concept may be applied.
   move(ss, false, result);
}


extern bool do_big_concept(
   setup *ss,
   setup *result) THROW_DECL
{
   remove_z_distortion(ss);

   void (*concept_func)(setup *, parse_block *, setup *);
   parse_block *orig_concept_parse_block = ss->cmd.parseptr;
   parse_block *this_concept_parse_block = orig_concept_parse_block;
   const conzept::concept_descriptor *this_concept = this_concept_parse_block->concept;
   concept_kind this_kind = this_concept->kind;
   concept_table_item *this_table_item = &concept_table[this_kind];

   // Take care of combinations like "mystic triple waves".

   if (this_table_item->concept_prop & CONCPROP__PERMIT_MYSTIC &&
       ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_MYSTIC) {

      // Turn on the good bits.
      ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_SPLIT;
      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__INVERT_MYSTIC)
         ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_INVERT;

      // And turn off the old ones.
      ss->cmd.cmd_misc2_flags &= ~(CMD_MISC2__CENTRAL_MYSTIC|CMD_MISC2__INVERT_MYSTIC);
   }

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {

      // If we have an "invert", "central", "mystic" or "snag" concept in place,
      // we have to check whether the current concept can deal with it.

      // The following concepts are always acceptable with invert/snag/etc in place.

      if (this_kind == concept_snag_mystic ||
          this_kind == concept_central ||
          this_kind == concept_fractional ||
          this_kind == concept_concentric ||
          this_kind == concept_some_vs_others ||
          (this_kind == concept_meta &&
           (this_concept->arg1 == meta_key_finish ||
            this_concept->arg1 == meta_key_revorder)))
         goto this_is_ok;

      // Otherwise, if "central" is selected, it must be one of the following ones.

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__DO_CENTRAL) {
         if (this_kind != concept_fractional &&
             this_kind != concept_fan &&
             (this_kind != concept_meta ||
              (this_concept->arg1 != meta_key_like_a &&
               this_concept->arg1 != meta_key_shift_n)) &&
             (this_kind != concept_meta_one_arg ||
              (this_concept->arg1 != meta_key_skip_nth_part &&
               this_concept->arg1 != meta_key_shift_n)))
            goto this_is_bad;
      }

      /* But if "snag" is selected, we lose. */

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_SNAG)
         goto this_is_bad;

      goto this_is_ok;

   this_is_bad:

      fail("Can't do \"invert/central/snag/mystic\" followed by this concept or modifier.");

   this_is_ok: ;
   }

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT) &&
       !(this_table_item->concept_prop & CONCPROP__PERMIT_MATRIX))
      fail("\"Matrix\" concept must be followed by applicable concept.");

   concept_func = this_table_item->concept_action;
   ss->cmd.parseptr = ss->cmd.parseptr->next;
   if (!ss->cmd.parseptr)
      fail("Incomplete parse.");

   /* When we invoke one of the functions, we will have:
      2ndarg = the concept we are doing
      1starg->cmd.parseptr = the next concept after that */

   if (concept_func == 0) return false;

   if (this_concept->level > calling_level) warn(warn__bad_concept_level);

   clear_people(result);

   if (this_table_item->concept_prop & CONCPROP__SET_PHANTOMS)
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

   if (this_table_item->concept_prop & CONCPROP__NO_STEP)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   // "Standard" is special -- process it now.

   if (this_kind == concept_standard) {
      parse_block *substandard_concptptr;
      final_and_herit_flags junk_concepts;

      // Skip to the phantom-line (or whatever) concept
      // by going over the "standard" and skipping comments.

      junk_concepts.clear_all_herit_and_final_bits();
      substandard_concptptr = process_final_concepts(orig_concept_parse_block->next,
                                                     true, &junk_concepts,
                                                     true, __FILE__, __LINE__);
   
      // If we hit "matrix", do a little extra stuff and continue.

      if (junk_concepts.test_herit_and_final_bits() == 0 &&
          substandard_concptptr->concept->kind == concept_matrix) {
         ss->cmd.cmd_misc_flags |= CMD_MISC__MATRIX_CONCEPT;
         substandard_concptptr = process_final_concepts(substandard_concptptr->next,
                                                        true, &junk_concepts,
                                                        true, __FILE__, __LINE__);
      }

      if (junk_concepts.test_herit_and_final_bits() != 0 ||
          (!(concept_table[substandard_concptptr->concept->kind].concept_prop & CONCPROP__STANDARD)))
         fail("This concept must be used with some offset/distorted/phantom concept.");

      /* We don't think stepping to a wave is ever a good idea if standard is used.
         Most calls that permit standard (CONCPROP__STANDARD is on) forbid it anyway
         (CONCPROP__NO_STEP is on also), but a few (e.g. concept_triple_lines)
         permit standard but don't necessarily forbid stepping to a wave.
         This is so that interesting cases of triple columns turn and weave will work.
         However, we think that this should be disallowed if "so-and-so are standard
         in triple lines" is given.  At least, that is the theory behind
         this next line of code. */

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

      if (concept_table[substandard_concptptr->concept->kind].concept_prop &
          CONCPROP__SET_PHANTOMS)
         ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

      if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX)) {
         uint32 prop_bits = concept_table[substandard_concptptr->concept->kind].concept_prop;
         // If the "arg2_matrix" bit is on, pick up additional matrix descriptor bits
         // from the arg2 word.
         if (prop_bits & CONCPROP__NEED_ARG2_MATRIX)
            prop_bits |= substandard_concptptr->concept->arg2;
         do_matrix_expansion(ss, prop_bits, false);
      }

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

      if (attr::slimit(ss) < 0) fail("Can't do this concept in this setup.");
   
      uint32 tbonetest;
      uint32 stdtest;
      int livemask = get_standard_people(ss, this_concept_parse_block->options.who,
                                         tbonetest, stdtest);
   
      if (!tbonetest) {
         result->kind = nothing;
         clear_result_flags(result);
         return true;
      }
   
      if ((tbonetest & 011) != 011)
         fail("People are not T-boned -- 'standard' is meaningless.");
   
      if (!stdtest) fail("No one is standard.");
      if ((stdtest & 011) == 011) fail("The standard people are not facing consistently.");
   
      global_tbonetest = stdtest;
      global_livemask = livemask;
      orig_tbonetest = tbonetest;

      ss->cmd.parseptr = substandard_concptptr->next;
      (concept_table[substandard_concptptr->concept->kind].concept_action)
         (ss, substandard_concptptr, result);
      remove_tgl_distortion(result);
      /* Beware -- result is not necessarily canonicalized. */
      result->result_flags.clear_split_info();  /* **** For now. */
      return true;
   }

   if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX)) {
      uint32 prop_bits = this_table_item->concept_prop;
      // If the "arg2_matrix" bit is on, pick up additional
      // matrix descriptor bits from the arg2 word.
      if (prop_bits & CONCPROP__NEED_ARG2_MATRIX) prop_bits |= this_concept->arg2;
      do_matrix_expansion(ss, prop_bits, false);
   }

   // We can no longer do any matrix expansion, unless this is "phantom" and "tandem",
   // in which case we continue to allow it.  The code for the "C1 phantom" concept
   // will check whether it is being used with some tandem-like concept, and expand to
   // the matrix that it really wants if so, or set the NO_EXPAND_MATRIX flag if not.
   // We also don't set the flag if this concept was "fractional" or stable or
   // fractional stable.  Those concepts can be effectively oblivious to matrix expansion.
   // The tests for these are (from lines out) "stable 2x8 matrix roll em" and
   // "1/4 (fractional) split phantom boxes split the difference".

   if (!(this_table_item->concept_prop & CONCPROP__MATRIX_OBLIVIOUS))
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

   /* See if this concept can be invoked with "standard".  If so, it wants
      tbonetest and livemask computed, and expects the former to indicate
      only the standard people. */

   if (this_table_item->concept_prop & (CONCPROP__STANDARD | CONCPROP__GET_MASK)) {
      int i;
      uint32 j;
      bool doing_select;
      selector_kind saved_selector = current_options.who;

      if (attr::slimit(ss) < 0) fail("Can't do this concept in this setup.");

      global_tbonetest = 0;
      global_livemask = 0;
      global_selectmask = 0;
      global_tboneselect = 0;
      doing_select = (this_table_item->concept_prop & CONCPROP__USE_SELECTOR) != 0;

      if (doing_select) {
         current_options.who = this_concept_parse_block->options.who;
      }

      for (i=0, j=1; i<=attr::slimit(ss); i++, j<<=1) {
         uint32 p = ss->people[i].id1;
         global_tbonetest |= p;
         if (p) {
            global_livemask |= j;
            if (doing_select && selectp(ss, i)) {
               global_selectmask |= j; global_tboneselect |= p;
            }
         }
      }

      current_options.who = saved_selector;

      orig_tbonetest = global_tbonetest;

      if (!global_tbonetest) {
         result->kind = nothing;
         clear_result_flags(result);
         return true;
      }
   }

   (*concept_func)(ss, this_concept_parse_block, result);
   remove_tgl_distortion(result);
   /* Beware -- result is not necessarily canonicalized. */
   if (!(this_table_item->concept_prop & CONCPROP__SHOW_SPLIT))
      result->result_flags.clear_split_info();
   return true;
}


#define Standard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX | CONCPROP__STANDARD)
#define Nostandard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX)
#define Nostep_phantom (CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS)


// Beware!!  This table must be keyed to definition of "concept_kind" in sd.h .

concept_table_item concept_table[] = {
   {0, 0},                                                  // concept_another_call_next_mod
   {0, 0},                                                  // concept_mod_declined
   {0, 0},                                                  // marker_end_of_list
   {0, 0},                                                  // concept_comment
   {CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_MODIFIERS,
    do_concept_concentric},                                 // concept_concentric
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_tandem},                                     // concept_tandem
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__MATRIX_OBLIVIOUS |
    CONCPROP__SHOW_SPLIT | CONCPROP__USE_SELECTOR,
    do_concept_tandem},                                     // concept_some_are_tandem
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__MATRIX_OBLIVIOUS |
    CONCPROP__SHOW_SPLIT | CONCPROP__USE_NUMBER,
    do_concept_tandem},                                     // concept_frac_tandem
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__MATRIX_OBLIVIOUS |
    CONCPROP__SHOW_SPLIT | CONCPROP__USE_NUMBER |
    CONCPROP__USE_SELECTOR,
    do_concept_tandem},                                     // concept_some_are_frac_tandem
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__MATRIX_OBLIVIOUS |
    CONCPROP__SHOW_SPLIT | CONCPROP__SET_PHANTOMS,
    do_concept_tandem},                                     // concept_gruesome_tandem
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__MATRIX_OBLIVIOUS |
    CONCPROP__SHOW_SPLIT | CONCPROP__USE_NUMBER |
    CONCPROP__SET_PHANTOMS,
    do_concept_tandem},                                     // concept_gruesome_frac_tandem
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT,
    complain_about_in_setup},                               // concept_tandem_in_setup
   {0, do_concept_checkerboard},                            // concept_checkerboard
   {CONCPROP__USE_SELECTOR | CONCPROP__GET_MASK,
    do_concept_checkerboard},                               // concept_sel_checkerboard  
   {CONCPROP__USE_SELECTOR | CONCPROP__GET_MASK,
    anchor_someone_and_move},                               // concept_anchor
   {0, 0},                                                  // concept_reverse
   {0, 0},                                                  // concept_left
   {0, 0},                                                  // concept_grand
   {0, 0},                                                  // concept_magic
   {0, 0},                                                  // concept_cross
   {0, 0},                                                  // concept_single
   {0, 0},                                                  // concept_singlefile
   {0, 0},                                                  // concept_interlocked
   {0, 0},                                                  // concept_yoyo
   {0, 0},                                                  // concept_fractal
   {0, 0},                                                  // concept_fast
   {0, 0},                                                  // concept_straight
   {0, 0},                                                  // concept_twisted
   {0, 0},                                                  // concept_12_matrix
   {0, 0},                                                  // concept_16_matrix
   {0, 0},                                                  // concept_revert
   {0, 0},                                                  // concept_1x2
   {0, 0},                                                  // concept_2x1
   {0, 0},                                                  // concept_2x2
   {0, 0},                                                  // concept_1x3
   {0, 0},                                                  // concept_3x1
   {0, 0},                                                  // concept_3x3
   {0, 0},                                                  // concept_4x4
   {0, 0},                                                  // concept_5x5
   {0, 0},                                                  // concept_6x6
   {0, 0},                                                  // concept_7x7
   {0, 0},                                                  // concept_8x8
   {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom,
    do_concept_expand_some_matrix},                         // concept_create_matrix
   {0, 0},                                                  // concept_funny
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_MODIFIERS,
    triangle_move},                                         // concept_randomtrngl
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_MODIFIERS | CONCPROP__USE_SELECTOR,
    triangle_move},                                         // concept_selbasedtrngl
   {0, 0},                                                  // concept_split
   {CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_MATRIX | CONCPROP__GET_MASK,
    do_concept_do_each_1x4},                                // concept_each_1x4
   {0, 0},                                                  // concept_diamond
   {0, 0},                                                  // concept_triangle
   {0, 0},                                                  // concept_leadtriangle
   {0, do_concept_do_both_boxes},                           // concept_do_both_boxes
   {CONCPROP__SHOW_SPLIT, do_concept_once_removed},         // concept_once_removed
   {CONCPROP__NEEDK_4X4 | Nostep_phantom,
    do_concept_do_phantom_2x2},                             // concept_do_phantom_2x2
   {CONCPROP__NEEDK_2X8 | Nostandard_matrix_phantom,
    do_concept_do_phantom_boxes},                           // concept_do_phantom_boxes
   {CONCPROP__NEEDK_4D_4PTPD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_do_phantom_diamonds},                        // concept_do_phantom_diamonds
   {CONCPROP__NEEDK_QUAD_1X4 | Standard_matrix_phantom | CONCPROP__PERMIT_MYSTIC,
    do_phantom_2x4_concept},                                // concept_do_phantom_2x4
   {CONCPROP__NEEDK_4X4 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_phantom_stag_qtg_concept},                           // concept_do_phantom_stag_qtg
   {CONCPROP__NEEDK_4X4 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_phantom_diag_qtg_concept},                           // concept_do_phantom_diag_qtg
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__NO_STEP | CONCPROP__GET_MASK |
    Nostandard_matrix_phantom,
    do_concept_do_divided_diamonds},                        // concept_do_divided_diamonds
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK |
    Nostandard_matrix_phantom,
    do_concept_do_divided_bones},                           // concept_do_divided_bones
   {CONCPROP__STANDARD,
    do_concept_distorted},                                  // concept_distorted
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,
    do_concept_single_diagonal},                            // concept_single_diagonal
   {CONCPROP__NO_STEP | CONCPROP__STANDARD,
    do_concept_double_diagonal},                            // concept_double_diagonal
   {CONCPROP__GET_MASK,
    do_concept_parallelogram},                              // concept_parallelogram
   {CONCPROP__NEED_ARG2_MATRIX | Standard_matrix_phantom,
    do_concept_multiple_lines},                             // concept_multiple_lines
   {CONCPROP__NEED_ARG2_MATRIX | Nostandard_matrix_phantom,
    do_concept_multiple_lines_tog},                         // concept_multiple_lines_tog
   {CONCPROP__NEED_ARG2_MATRIX | Standard_matrix_phantom,
    do_concept_multiple_lines_tog},                         // concept_multiple_lines_tog_std
   {CONCPROP__NEEDK_3X8 | Nostandard_matrix_phantom,
    do_concept_triple_1x8_tog},                             // concept_triple_1x8_tog
   {CONCPROP__NEED_ARG2_MATRIX | Nostandard_matrix_phantom,
    do_concept_multiple_boxes},                             // concept_multiple_boxes
   {CONCPROP__NEEDK_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_quad_boxes_tog},                             // concept_quad_boxes_together
   {CONCPROP__NEEDK_2X6 | Nostandard_matrix_phantom,
    do_concept_triple_boxes_tog},                           // concept_triple_boxes_together
   {CONCPROP__NEEDK_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_triple_diamonds},                            // concept_triple_diamonds
   {CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_triple_formations},                          // concept_triple_formations
   {CONCPROP__NEEDK_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_triple_diamonds_tog},                        // concept_triple_diamonds_together
   {CONCPROP__NEEDK_4D_4PTPD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_quad_diamonds},                              // concept_quad_diamonds
   {CONCPROP__NEEDK_4D_4PTPD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,
    do_concept_quad_diamonds_tog},                          // concept_quad_diamonds_together
   {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom | CONCPROP__GET_MASK,
    do_concept_do_triangular_boxes},                        // concept_triangular_boxes
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__STANDARD | CONCPROP__PERMIT_MATRIX | Nostep_phantom,
    do_concept_inner_outer},                                // concept_in_out_std
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__PERMIT_MATRIX | Nostep_phantom,
    do_concept_inner_outer},                                // concept_in_out_nostd
   {CONCPROP__NEEDK_BLOB | Nostep_phantom | CONCPROP__STANDARD,
    do_concept_triple_diag},                                // concept_triple_diag
   {CONCPROP__NEEDK_BLOB | Nostep_phantom | CONCPROP__GET_MASK,
    do_concept_triple_diag_tog},                            // concept_triple_diag_together
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__NO_STEP | Standard_matrix_phantom |
    CONCPROP__PERMIT_MYSTIC,
    triple_twin_move},                                      // concept_triple_twin
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__NO_STEP | Standard_matrix_phantom,
    triple_twin_move},                                      // concept_triple_twin_nomystic
   {CONCPROP__NEED_ARG2_MATRIX, distorted_2x2s_move},       // concept_misc_distort
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__PERMIT_MATRIX,
    distorted_2x2s_move},                                   // concept_misc_distort_matrix
   {0, do_concept_old_stretch},                             // concept_old_stretch
   {CONCPROP__GET_MASK, do_concept_new_stretch},            // concept_new_stretch
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_assume_waves},                               // concept_assume_waves
   {0, do_concept_active_phantoms},                         // concept_active_phantoms
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_mirror},                                     // concept_mirror
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_central},                                    // concept_central
   {CONCPROP__MATRIX_OBLIVIOUS, do_concept_central},        // concept_snag_mystic
   {CONCPROP__PERMIT_MODIFIERS, do_concept_crazy},          // concept_crazy
   {CONCPROP__USE_NUMBER | CONCPROP__PERMIT_MODIFIERS,
    do_concept_crazy},                                      // concept_frac_crazy
   {CONCPROP__PERMIT_MODIFIERS | CONCPROP__NEED_ARG2_MATRIX |
    CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,
    do_concept_phan_crazy},                                 // concept_phan_crazy
   {CONCPROP__PERMIT_MODIFIERS | CONCPROP__NEED_ARG2_MATRIX | CONCPROP__USE_NUMBER |
    CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,
    do_concept_phan_crazy},                                 // concept_frac_phan_crazy
   {0, do_concept_fan},                                     // concept_fan
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__GET_MASK,
    do_c1_phantom_move},                                    // concept_c1_phantom
   {CONCPROP__PERMIT_MATRIX | CONCPROP__SET_PHANTOMS,
    do_concept_grand_working},                              // concept_grand_working
   {0, do_concept_centers_or_ends},                         // concept_centers_or_ends
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_SELECTOR,
    so_and_so_only_move},                                   // concept_so_and_so_only
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_SELECTOR | CONCPROP__SECOND_CALL,
    so_and_so_only_move},                                   // concept_some_vs_others
   {CONCPROP__MATRIX_OBLIVIOUS,
    so_and_so_only_move},                                   // concept_same_sex_disconnected
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     // concept_stable
   {CONCPROP__USE_SELECTOR | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     // concept_so_and_so_stable
   {CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     // concept_frac_stable
   {CONCPROP__USE_SELECTOR | CONCPROP__USE_NUMBER |
    CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     // concept_so_and_so_frac_stable
   {CONCPROP__MATRIX_OBLIVIOUS,
    do_concept_emulate},                                    // concept_emulate
   {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX,
    do_concept_standard},                                   // concept_standard
   {CONCPROP__MATRIX_OBLIVIOUS, do_concept_matrix},         // concept_matrix
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,
    do_concept_double_offset},                              // concept_double_offset
   {CONCPROP__SECOND_CALL | CONCPROP__PERMIT_MODIFIERS,
    do_concept_checkpoint},                                 // concept_checkpoint
   {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,
    on_your_own_move},                                      // concept_on_your_own
   {CONCPROP__SECOND_CALL | CONCPROP__PERMIT_MODIFIERS | CONCPROP__NO_STEP,
    do_concept_trace},                                      // concept_trace
   {CONCPROP__NO_STEP,
    do_concept_outeracting},                                // concept_outeracting
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_MODIFIERS,
    do_concept_ferris},                                     // concept_ferris
   {CONCPROP__NO_STEP, do_concept_overlapped_diamond},      // concept_overlapped_diamond
   {0, do_concept_all_8},                                   // concept_all_8
   {CONCPROP__SECOND_CALL, do_concept_centers_and_ends},    // concept_centers_and_ends
   {0, do_concept_mini_but_o},                              // concept_mini_but_o
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_n_times},                                    // concept_n_times_const
   {CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_n_times},                                    // concept_n_times
   {CONCPROP__SECOND_CALL | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_sequential},                                 // concept_sequential
   {CONCPROP__SECOND_CALL | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_special_sequential},                         // concept_special_sequential
   {CONCPROP__USE_NUMBER | CONCPROP__SECOND_CALL |
    CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_special_sequential},                         // concept_special_sequential_num
   {CONCPROP__MATRIX_OBLIVIOUS |
    CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,
    do_concept_meta},                                       // concept_meta
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_NUMBER |
    CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,
    do_concept_meta},                                       // concept_meta_one_arg
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS |
    CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,
    do_concept_meta},                                       // concept_meta_two_args
   {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,
    do_concept_so_and_so_begin},                            // concept_so_and_so_begin
   {CONCPROP__USE_NUMBER | CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           // concept_replace_nth_part
   {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           // concept_replace_last_part
   {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS |
    CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           // concept_interrupt_at_fraction
   {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           // concept_sandwich
   {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_interlace},                                  // concept_interlace
   {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS |
    CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_fractional},                                 // concept_fractional
   {CONCPROP__NO_STEP, do_concept_rigger},                  // concept_rigger
   {CONCPROP__NO_STEP | CONCPROP__MATRIX_OBLIVIOUS,
    do_concept_wing},                                       // concept_wing
   {CONCPROP__NO_STEP, common_spot_move},                   // concept_common_spot
   {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,
    drag_someone_and_move},                                 // concept_drag
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,
    do_concept_dblbent},                                    // concept_dblbent
   {0, do_concept_omit},                                    // concept_omit
   {0, 0},                                                  // concept_supercall
   {0, do_concept_diagnose}};                               // concept_diagnose
