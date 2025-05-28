/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

/* This defines the following function:
   do_big_concept

and the following external variables:
   global_tbonetest
   global_livemask
   global_selectmask
   global_tboneselect
   concept_table
*/

/* For sprintf. */
#include <stdio.h>

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"

#define THIS_IS_TOO_WEIRD


static uint32 orig_tbonetest;
uint32 global_tbonetest;
uint32 global_livemask;
uint32 global_selectmask;
uint32 global_tboneselect;



Private void do_concept_expand_some_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* We used to turn on the "FINAL__16_MATRIX" call modifier for 2x8 matrix,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the CMD_MISC__EXPLICIT_MATRIX
      bit control the desired effects. */
   if (  ss->kind != ((setup_kind) parseptr->concept->value.arg1) &&
         /* "16 matrix of parallel diamonds" needs to accept 4dmd or 4ptpd. */
         (ss->kind != s4ptpd || ((setup_kind) parseptr->concept->value.arg1) != s4dmd))
      fail("Can't make the required matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, FALSE, result);
}


typedef struct {
   C_const int sizem1;
   C_const setup_kind ikind;
   C_const veryshort map1[8];
   C_const veryshort map2[8];
} phan_map;

static phan_map map_c1_phan   = {7, s2x4,   {0, 2, 7, 5, 8, 10, 15, 13},    {4, 6, 11, 9, 12, 14, 3, 1}};
static phan_map map_pinwheel1 = {7, s2x4,   {10, 15, -1, -1, 2, 7, -1, -1}, {14, 3, -1, -1, 6, 11, -1, -1}};
static phan_map map_pinwheel2 = {7, s2x4,   {-1, -1, 3, 1, -1, -1, 11, 9},  {-1, -1, 7, 5, -1, -1, 15, 13}};
#ifdef THIS_IS_TOO_WEIRD
static phan_map map_pinwheel3 = {7, s2x4,   {0, 1, -1, -1, 6, 7, -1, -1}, {2, 5, -1, -1, 8, 11, -1, -1}};
static phan_map map_pinwheel4 = {7, s2x4,   {-1, -1, 2, 3, -1, -1, 8, 9},  {-1, -1, 5, 7, -1, -1, 11, 1}};
#endif
static phan_map map_o_spots   = {7, s2x4,   {10, -1, -1, 1, 2, -1, -1, 9},  {14, -1, -1, 5, 6, -1, -1, 13}};
static phan_map map_qt_phan   = {7, s_qtag, {-1, -1, 2, 3, -1, -1, 6, 7},   {1, 4, -1, -1, 5, 0, -1, -1}};

Private void do_c1_phantom_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   Const parse_block *next_parseptr;
   uint64 junk_concepts;
   setup setup1, setup2;
   setup the_setups[2];
   phan_map *map_ptr = (phan_map *) 0;

   /* See if this is a "phantom tandem" (or whatever) by searching ahead, skipping comments of course.
      This means we must skip modifiers too, so we check that there weren't any. */

   junk_concepts.her8it = 0;
   junk_concepts.final = 0;
   next_parseptr = process_final_concepts(parseptr->next, FALSE, &junk_concepts);

   if (next_parseptr->concept->kind == concept_tandem || next_parseptr->concept->kind == concept_frac_tandem) {
      /* Find out what kind of tandem call this is. */

      uint32 what_we_need = 0;
      uint32 mxnflags = ss->cmd.cmd_final_flags.her8it &
         (INHERITFLAG_SINGLE | INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK);

      if (junk_concepts.her8it || junk_concepts.final)
         fail("Phantom couples/tandem must not have intervening concepts.");

      /* "Phantom tandem" has a higher level than either "phantom" or "tandem". */
      if (phantom_tandem_level > calling_level) warn(warn__bad_concept_level);

      switch (next_parseptr->concept->value.arg4) {
      case tandem_key_siam:
      case tandem_key_skew:
         fail("Phantom not allowed with skew or siamese.");
      case tandem_key_box:
         /* We do not expand the matrix.  The caller must say
            "2x8 matrix", or whatever, to get that effect. */
         break;
      case tandem_key_diamond:
         /* We do not expand the matrix.  The caller must say
            "16 matrix or parallel diamonds", or whatever, to get that effect. */
         break;
      case tandem_key_tand3:
      case tandem_key_cpls3:
      case tandem_key_siam3:
      case tandem_key_tand4:
      case tandem_key_cpls4:
      case tandem_key_siam4:
         /* We do not expand the matrix.  The caller must say
            "2x8 matrix", or whatever, to get that effect. */
         break;
      default:
         /* This is plain "phantom tandem", or whatever.  Expand to whatever is in
            the "arg2" field, or to a 4x4.  The "arg2" check allows the user to say
            "phantom as couples in a 1/4 tag".  "as couples in a 1/4 tag"
            would have worked also.
            But we don't do this if stuff like "1x3" came in. */

         if (!mxnflags) {
            what_we_need = next_parseptr->concept->value.arg2;
            if (what_we_need == 0) what_we_need = CONCPROP__NEEDK_4X4;
         }

         break;
      }

      if (what_we_need != 0)
         do_matrix_expansion(ss, what_we_need, TRUE);

      ss->cmd.cmd_misc_flags |=
         CMD_MISC__NO_EXPAND_MATRIX | CMD_MISC__PHANTOMS | next_parseptr->concept->value.arg1;
      ss->cmd.cmd_final_flags.her8it &=
         ~(INHERITFLAG_SINGLE | INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK);
      ss->cmd.parseptr = next_parseptr->next;

      tandem_couples_move(
            ss,
            (next_parseptr->concept->value.arg3 & 0x100) ? next_parseptr->options.who : selector_uninitialized,
            (next_parseptr->concept->value.arg3 & 0xF0) >> 4, /* (fractional) twosome info */
            next_parseptr->options.number_fields,
            1,                                                /* for phantom */
            (tandem_key) next_parseptr->concept->value.arg4,  /* key */
            mxnflags,
            FALSE,
            result);

      return;
   }

   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;   /* We didn't do this before. */

   if (ss->kind == s4x4 && global_livemask == 0x6666) {
      /* First, check for everyone on "O" spots.  If so, treat them as though
         in equivalent C1 phantom spots. */
      map_ptr = &map_o_spots;
   }
   else if (ss->kind == s_c1phan)
      /* This is a vanilla C1 phantom setup. */
      map_ptr = &map_c1_phan;
   else if (ss->kind == s_bone)
      /* We allow "phantom" in a bone setup to mean two "dunlap" quarter tags. */
      map_ptr = &map_qt_phan;
#ifdef THIS_IS_TOO_WEIRD
   else if (ss->kind == s3x4) {

      /* Check for a 3x4 occupied as a distorted "pinwheel", and treat it as phantoms. */

      if (global_livemask == 04747)
         map_ptr = &map_pinwheel3;
      else if (global_livemask == 05656)
         map_ptr = &map_pinwheel4;
   }
#endif
   else if (ss->kind == s4x4) {
      setup temp;

      /* Check for a 4x4 occupied as a "pinwheel", and treat it as phantoms. */

      if (global_livemask == 0xCCCC) {
         map_ptr = &map_pinwheel1;
         goto use_map;
      }
      else if (global_livemask == 0xAAAA) {
         map_ptr = &map_pinwheel2;
         goto use_map;
      }

      /* Next, check for a "phantom turn and deal" sort of thing from stairsteps.
         Do the call in each line, them remove resulting phantoms carefully. */

      if (global_livemask == 0x5C5C || global_livemask == 0xA3A3) {
         /* Split into 4 vertical strips. */
         divided_setup_move(ss, &map_4x4v, phantest_ok, TRUE, result);
      }
      else if (global_livemask == 0xC5C5 || global_livemask == 0x3A3A) {
         /* Split into 4 horizontal strips. */
         new_divided_setup_move(ss, MAPCODE(s1x4,4,MPKIND__SPLIT,1), phantest_ok, TRUE, result);
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
   
   normalize_setup(&setup1, simple_normalize);
   normalize_setup(&setup2, simple_normalize);

   setup1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   setup2.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   move(&setup1, FALSE, &the_setups[0]);
   move(&setup2, FALSE, &the_setups[1]);

   *result = the_setups[1];
   result->result_flags = get_multiple_parallel_resultflags(the_setups, 2);
   merge_setups(&the_setups[0], merge_c1_phantom_nowarn, result);
}



Private void do_concept_single_diagonal(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (parseptr->concept->value.arg1 & 8) {
      /* Concept identified specific people. */
      selective_move(ss, parseptr, selective_key_disc_dist, 0,
                     16+(parseptr->concept->value.arg1 & 7),
                     0, parseptr->options.who, FALSE, result);
      return;
   }

   /* The concept didn't say who are on the diagonal--deduce what the mask would have been,
      based on the full population of the 4x4, which must be blocks. */

   if (ss->kind != s4x4) fail("Need a 4x4 for this.");

   if (global_livemask != 0x2D2D && global_livemask != 0xD2D2)
      fail("People must be in blocks -- try specifying the people who should do the call.");

   selective_move(ss, parseptr, selective_key_disc_dist, 0,
                  16+(parseptr->concept->value.arg1 & 7),
                  global_livemask & 0x9999, selector_uninitialized, FALSE, result);
}


Private void do_concept_double_diagonal(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 tbonetest;
   map_thing *map_ptr;

   if (parseptr->concept->value.arg2 == 2) {

      /* This is "distorted CLW of 6". */

      setup ssave = *ss;

      if (ss->kind != sbighrgl) global_livemask = 0;   /* Force error. */

      if (global_livemask == 0x3CF) { map_ptr = &map_dhrgl1; }
      else if (global_livemask == 0xF3C) { map_ptr = &map_dhrgl2; }
      else fail("Can't find distorted 1x6.");

      if (parseptr->concept->value.arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;
      else if (parseptr->concept->value.arg1 == 1)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES;
      else
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS;

      divided_setup_move(ss, map_ptr, phantest_ok, TRUE, result);

      /* I wish this weren't so sleazy, but a new concentricity schema seems excessive. */

      if (result->kind != sbighrgl) fail("Can't figure out result setup.");

      (void) copy_person(result, 2, &ssave, 2);
      (void) copy_person(result, 8, &ssave, 8);
   }
   else if (parseptr->concept->value.arg2) {

      /* This is "diagonal CLW's of 3". */

      setup ssave = *ss;
      int switcher = (parseptr->concept->value.arg1 ^ global_tbonetest) & 1;

      if (ss->kind != s4x4 || (global_tbonetest & 011) == 011) global_livemask = 0;   /* Force error. */

      if      (global_livemask == 0x2D2D) map_ptr = switcher ? &map_diag23a : &map_diag23b;
      else if (global_livemask == 0xD2D2) map_ptr = switcher ? &map_diag23c : &map_diag23d;
      else
         fail("There are no diagonal lines or columns of 3 here.");

      if (parseptr->concept->value.arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      divided_setup_move(ss, map_ptr, phantest_ok, TRUE, result);

      /* I wish this weren't so sleazy, but a new concentricity schema seems excessive. */

      if (result->kind != s4x4) fail("Can't figure out result setup.");

      (void) copy_person(result, 0, &ssave, 0);
      (void) copy_person(result, 4, &ssave, 4);
      (void) copy_person(result, 8, &ssave, 8);
      (void) copy_person(result, 12, &ssave, 12);
   }
   else {
      tbonetest = global_tbonetest;

      if      (global_livemask == 0x2A82A8) map_ptr = &map_diag2a;
      else if (global_livemask == 0x505505) map_ptr = &map_diag2b;
      else
         tbonetest = ~0UL;   /* Force error. */

      if (ss->kind != s4x6) tbonetest = ~0UL;   /* Force error. */

      if (parseptr->concept->value.arg1 & 1) {
         if (tbonetest & 010) fail("There are no diagonal lines here.");
      }
      else {
         if (tbonetest & 1) fail("There are no diagonal columns here.");
      }

      if (parseptr->concept->value.arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      divided_setup_move(ss, map_ptr, phantest_ok, TRUE, result);
   }
}


Private void do_concept_double_offset(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 top, bot, ctr;
   map_thing *map_ptr;

   if (ss->kind != s2x4) fail("Must have a 2x4 setup to do this concept.");

   if (global_selectmask == (global_livemask & 0xCC)) {
      top = ss->people[0].id1 | ss->people[1].id1;
      bot = ss->people[4].id1 | ss->people[5].id1;
      ctr = ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1;
      map_ptr = &map_dbloff1;
   }
   else if (global_selectmask == (global_livemask & 0x33)) {
      top = ss->people[2].id1 | ss->people[3].id1;
      bot = ss->people[6].id1 | ss->people[7].id1;
      ctr = ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1;
      map_ptr = &map_dbloff2;
   }
   else
      fail("The designated centers are improperly placed.");

   /* Check that the concept is correctly named. */

   switch (parseptr->concept->value.arg1) {
      case 0:
         /* Double-offset quarter tag */
         /* **** need to check people more carefully -- need "signature". */
         /* Need both tops = 012, both bots = 010. */
         if ((ctr & 1) != 0)
            fail("Facing directions are incorrect for this concept.");
         break;
      case 1:
         /* Double-offset three-quarter tag */
         /* **** need to check people more carefully -- need "signature". */
         /* Need both tops = 010, both bots = 012. */
         if ((ctr & 1) != 0)
            fail("Facing directions are incorrect for this concept.");
         break;
      case 2:
         /* Anything goes */
         break;
      case 3:
         /* Double-offset diamonds */
         if ((((top | bot) & 010) != 0) || ((ctr & 1) != 0))
            fail("Facing directions are incorrect for this concept.");
         break;
   }

   divided_setup_move(ss, map_ptr, phantest_ok, TRUE, result);
}


Private void do_concept_quad_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 the_map;
   int arg1 = parseptr->concept->value.arg1;
   int rot = 0;

   if (ss->kind == s4x4) {
      rot = (global_tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
      canonicalize_rotation(ss);
      the_map = MAPCODE(s1x4,4,MPKIND__SPLIT,1);
   }
   else if (ss->kind == s1x16) {
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if (!((arg1 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         else                      fail("There are no columns of 4 here.");
      }
   
      the_map = MAPCODE(s1x4,4,MPKIND__SPLIT,0);
   }
   else if (ss->kind == sbigbigh)
      the_map = MAPCODE(s1x4,4,MPKIND__NONISOTROPIC,1);
   else if (ss->kind == sbigbigx)
      the_map = MAPCODE(s1x4,4,MPKIND__NONISOTROPIC,0);
   else
      fail("Must have quadruple 1x4's for this concept.");

   /* If this was quadruple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was quadruple
      lines, we forbid it. */

   if (arg1 & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (arg1 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   new_divided_setup_move(ss, the_map, phantest_ok, TRUE, result);
   result->rotation -= rot;   /* Flip the setup back. */
}


void do_4x4_quad_working(setup *ss, int cstuff, setup *result)
{
   uint32 masks[8];

   canonicalize_rotation(ss);

   /* Initially assign the centers to the upper (masks[1] or masks[2]) group. */
   masks[0] = 0xF0; masks[1] = 0xF0; masks[2] = 0xFF;

   /* Look at the center 8 people and put each one in the correct group. */

   if (cstuff == 8) {
      masks[0] = 0xFC; masks[1] = 0xCC; masks[2] = 0xCF;   /* clockwise */
   }
   else if (cstuff == 9) {
      masks[0] = 0xF3; masks[1] = 0x33; masks[2] = 0x3F;   /* counterclockwise */
   }
   else {                        /* forward/back/left/right */
      if ((ss->people[10].id1 ^ cstuff) & 2) { masks[1] |= 0x01; masks[2] &= ~0x80; };
      if ((ss->people[15].id1 ^ cstuff) & 2) { masks[1] |= 0x02; masks[2] &= ~0x40; };
      if ((ss->people[3].id1  ^ cstuff) & 2) { masks[1] |= 0x04; masks[2] &= ~0x20; };
      if ((ss->people[1].id1  ^ cstuff) & 2) { masks[1] |= 0x08; masks[2] &= ~0x10; };
      if ((ss->people[9].id1  ^ cstuff) & 2) { masks[0] |= 0x01; masks[1] &= ~0x80; };
      if ((ss->people[11].id1 ^ cstuff) & 2) { masks[0] |= 0x02; masks[1] &= ~0x40; };
      if ((ss->people[7].id1  ^ cstuff) & 2) { masks[0] |= 0x04; masks[1] &= ~0x20; };
      if ((ss->people[2].id1  ^ cstuff) & 2) { masks[0] |= 0x08; masks[1] &= ~0x10; };
   }

   new_overlapped_setup_move(ss, MAPCODE(s2x4,3,MPKIND__OVERLAP,1), masks, result);
}




Private void do_concept_multiple_lines_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This can only be standard for together/apart/clockwise/counterclockwise/toward-the-center,
      not for forward/back/left/right, because we look at individual facing directions to
      determine which other line/column the people in the center lines/columns must work in. */

   int rotfix = 0;
   setup_kind base_setup = s2x4;
   int base_vert = 1;
   uint32 masks[8];

   /* arg4 = number of C/L/W. */

   int cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward (lines) or left (cols)     : 0
      backward (lines) or right (cols)   : 2
      clockwise                          : 8
      counterclockwise                   : 9
      together (must be end-to-end)      : 10
      apart (must be end-to-end)         : 11
      toward the center (quadruple only) : 12 */

   int linesp = parseptr->concept->value.arg3;

   /* If this was multiple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was multiple
      lines, we forbid it. */

   if (linesp & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (linesp == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (parseptr->concept->value.arg4 == 3) {
      /* Triple C/L/W working. */

      if (cstuff >= 10) {
         if (ss->kind != s1x12) fail("Must have a 1x12 setup for this concept.");

         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
 
         if (linesp & 1) {
            if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         }
         else {
            if (!(global_tbonetest & 1)) fail("There are no columns of 4 here.");
         }
    
         if (cstuff == 10) {     /* Working together. */
            masks[0] = 0xCF; masks[1] = 0xFC;
         }
         else {                 /* Working apart. */
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
    
         /* Initially assign the centers to the upper (masks[1]) group. */
         masks[0] = 0xF0; masks[1] = 0xFF;
    
         /* Look at the center line people and put each one in the correct group. */
    
         if (cstuff == 8) {
            masks[0] = 0xFC; masks[1] = 0xCF;   /* clockwise */
         }
         else if (cstuff == 9) {
            masks[0] = 0xF3; masks[1] = 0x3F;   /* counterclockwise */
         }
         else {                        /* forward/back/left/right */
            if ((ss->people[10].id1 ^ cstuff) & 2) { masks[1] &= ~0x80 ; masks[0] |= 0x1; };
            if ((ss->people[11].id1 ^ cstuff) & 2) { masks[1] &= ~0x40 ; masks[0] |= 0x2; };
            if ((ss->people[5].id1  ^ cstuff) & 2) { masks[1] &= ~0x20 ; masks[0] |= 0x4; };
            if ((ss->people[4].id1  ^ cstuff) & 2) { masks[1] &= ~0x10 ; masks[0] |= 0x8; };
         }
      }
   }
   else if (parseptr->concept->value.arg4 == 4) {
      /* Quadruple C/L/W working. */

      if (cstuff >= 12) {
         if (ss->kind != s4x4) fail("Must have a 4x4 setup to do this concept.");

         if ((global_tbonetest & 011) == 011) fail("Sorry, can't do this from T-bone setup.");
         rotfix = (global_tbonetest ^ linesp ^ 1) & 1;
         ss->rotation += rotfix;   /* Just flip the setup around and recanonicalize. */
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

         if (cstuff == 10) {    /* Working together end-to-end. */
            masks[0] = 0xCF; masks[1] = 0xCC; masks[2] = 0xFC;
         }
         else {                 /* Working apart end-to-end. */
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
            the_setups[0].rotation -= rotfix;   /* Flip the setup back. */

            rotfix = (linesp) & 1;
            hpeople.rotation += rotfix;
            do_4x4_quad_working(&hpeople, cstuff, &the_setups[1]);
            the_setups[1].rotation -= rotfix;   /* Flip the setup back. */

            *result = the_setups[1];
            result->result_flags = get_multiple_parallel_resultflags(the_setups, 2);
            merge_setups(&the_setups[0], merge_strict_matrix, result);
            warn(warn__tbonephantom);
         }
         else {
            rotfix = (tbonetest ^ linesp ^ 1) & 1;
            ss->rotation += rotfix;
            do_4x4_quad_working(ss, cstuff, result);
            result->rotation -= rotfix;   /* Flip the setup back. */
         }

         return;
      }
   }
   else if (parseptr->concept->value.arg4 == 5) {
      /* Quintuple C/L/W working. */

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
 
      /* Initially assign the center 3 lines to the right (masks[1]/masks[2]/masks[3]) group. */
      masks[0] = 0xF0; masks[1] = 0xF0; masks[2] = 0xF0; masks[3] = 0xFF;
 
      /* Look at the center 3 lines of people and put each one in the correct group. */
 
      if (cstuff == 8) {
         masks[0] = 0xFC; masks[1] = 0xCC; masks[2] = 0xCC; masks[3] = 0xCF;   /* clockwise */
      }
      else if (cstuff == 9) {
         masks[0] = 0xF3; masks[1] = 0x33; masks[2] = 0x33; masks[3] = 0x3F;   /* counterclockwise */
      }
      else {                        /* forward/back/left/right */
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
      /* Sextuple C/L/W working. */

      int i;
      uint32 tbonetest = global_tbonetest;

      /* Expanding to a 4x6 is tricky.  See the extensive comments in the
         function "triple_twin_move" in sdistort.c . */

      if (cstuff < 8) {
         tbonetest = 0;
         for (i=0; i<24; i++) tbonetest |= ss->people[i].id1;
      }

      if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      tbonetest ^= linesp;

      if (ss->kind == s4x4) {
         expand_setup(
            ((tbonetest & 1) ? &exp_4x4_4x6_stuff_b : &exp_4x4_4x6_stuff_a), ss);
         tbonetest = 0;
      }

      if (ss->kind != s4x6) fail("Must have a 4x6 setup for this concept.");

      if (tbonetest & 1) {
         if (linesp) fail("Can't find the required lines.");
         else fail("Can't find the required columns.");
      }
 
      /* Initially assign the center 3 lines to the right (masks[1]/masks[2]/masks[3]/masks[4]) group. */
      masks[0] = 0xF0; masks[1] = 0xF0; masks[2] = 0xF0; masks[3] = 0xF0; masks[4] = 0xFF;
 
      /* Look at the center 4 lines of people and put each one in the correct group. */
 
      if (cstuff == 8) {
         masks[0] = 0xFC; masks[1] = 0xCC; masks[2] = 0xCC; masks[3] = 0xCC; masks[4] = 0xCF;   /* clockwise */
      }
      else if (cstuff == 9) {
         masks[0] = 0xF3; masks[1] = 0x33; masks[2] = 0x33; masks[3] = 0x33; masks[4] = 0x3F;   /* counterclockwise */
      }
      else {                        /* forward/back/left/right */
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

   new_overlapped_setup_move(ss, MAPCODE(base_setup,parseptr->concept->value.arg4-1,MPKIND__OVERLAP,base_vert),
      masks, result);
   result->rotation -= rotfix;   /* Flip the setup back. */
}


Private void do_concept_parallelogram(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   mpkind mk, mkbox;
   Const parse_block *next_parseptr;
   uint64 junk_concepts;
   uint32 map_code;
   Const parse_block *standard_concept = (parse_block *) 0;

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
   else
      fail("Can't do parallelogram concept from this position.");

   /* See if it is followed by "split phantom C/L/W" or "split phantom boxes",
      in which case we do something esoteric. */

   junk_concepts.her8it = 0;
   junk_concepts.final = 0;
   next_parseptr = process_final_concepts(parseptr->next, FALSE, &junk_concepts);

   /* But skip over "standard" */
   if (next_parseptr->concept->kind == concept_standard && junk_concepts.her8it == 0 && junk_concepts.final == 0) {
      standard_concept = next_parseptr;
      junk_concepts.her8it = 0;
      junk_concepts.final = 0;
      next_parseptr = process_final_concepts(next_parseptr->next, FALSE, &junk_concepts);
   }

   if (next_parseptr->concept->kind == concept_do_phantom_2x4 &&
       (ss->kind == s2x6 || ss->kind == s2x5 || ss->kind == s2x7) &&
       junk_concepts.her8it == 0 &&
       junk_concepts.final == 0 &&
       next_parseptr->concept->value.arg3 == MPKIND__SPLIT) {
      int linesp = next_parseptr->concept->value.arg2 & 7;

      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;
      if (ss->kind == s2x5) {
         do_matrix_expansion(ss, CONCPROP__NEEDK_4X5, FALSE);
         if (ss->kind != s4x5) fail("Must have a 4x5 setup for this concept.");
      }
      if (ss->kind == s2x6) {
         do_matrix_expansion(ss, CONCPROP__NEEDK_4X6, FALSE);
         if (ss->kind != s4x6) fail("Must have a 4x6 setup for this concept.");
      }

      if (standard_concept) {
         int tbonetest = 0;
         int stdtest = 0;
         int livemask = 0;
         ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;
         int i, j;
         selector_kind saved_selector = current_options.who;

         current_options.who = standard_concept->options.who;
      
         for (i=0, j=1; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
            int p = ss->people[i].id1;
            tbonetest |= p;
            if (p) {
               livemask |= j;
               if (selectp(ss, i)) stdtest |= p;
            }
         }
      
         current_options.who = saved_selector;

         if (!tbonetest) {
            result->result_flags = 0;
            result->kind = nothing;
            return;
         }
      
         if ((tbonetest & 011) != 011)
            fail("People are not T-boned -- 'standard' is meaningless.");
      
         if (!stdtest) fail("No one is standard.");
         if ((stdtest & 011) == 011) fail("The standard people are not facing consistently.");
      
         global_tbonetest = stdtest;
         global_livemask = livemask;
      }

      if (linesp & 1) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      }
      else {
         if (global_tbonetest & 010) fail("There are no columns of 4 here.");
      }

      if (linesp == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      ss->cmd.parseptr = next_parseptr->next;
      map_code = MAPCODE(s2x4,2,mk,1);
   }
   else if (next_parseptr->concept->kind == concept_do_phantom_boxes &&
            ss->kind == s2x6 &&     /* Only allow 50% offset. */
            junk_concepts.her8it == 0 &&
            junk_concepts.final == 0 &&
            next_parseptr->concept->value.arg3 == MPKIND__SPLIT) {
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, FALSE);
      if (ss->kind != s2x8) fail("Not in proper setup for this concept.");

      if (standard_concept) fail("Don't use \"standard\" with split phantom boxes.");

      ss->cmd.parseptr = next_parseptr->next;
      map_code = MAPCODE(s2x4,2,mkbox,0);
   }
   else
      map_code = MAPCODE(s2x4,1,mk,0);

   new_divided_setup_move(ss, map_code, phantest_ok, TRUE, result);

   /* The split-axis bits are gone.  If someone needs them, we have work to do. */
   result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
}


Private void do_concept_quad_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff;
   uint32 masks[3];

   if (ss->kind != s2x8) fail("Must have a 2x8 setup to do this concept.");

   cstuff = parseptr->concept->value.arg1;
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

   new_overlapped_setup_move(ss, MAPCODE(s2x4,3,MPKIND__OVERLAP,0), masks, result);
}


Private void do_concept_triple_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff;
   uint32 m1, m2;
   uint32 masks[2];

   if (ss->kind != s3dmd) fail("Must have a triple diamond setup to do this concept.");

   cstuff = parseptr->concept->value.arg1;
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
   new_overlapped_setup_move(ss, MAPCODE(s_qtag,2,MPKIND__OVERLAP,0), masks, result);
}


Private void do_concept_quad_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff;
   uint32 m1, m2, m3;
   uint32 masks[3];

   if (ss->kind != s4dmd) fail("Must have a quadruple diamond setup to do this concept.");

   cstuff = parseptr->concept->value.arg1;
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
   new_overlapped_setup_move(ss, MAPCODE(s_qtag,3,MPKIND__OVERLAP,0), masks, result);
}


Private void do_concept_triple_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff;
   uint32 m1, m2;
   uint32 masks[2];

   if (ss->kind != s2x6) fail("Must have a 2x6 setup to do this concept.");

   cstuff = parseptr->concept->value.arg1;
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

      /* Look at the center 4 people and put each one in the correct group. */

      m1 = 0xC3; m2 = 0xFF;
      cstuff <<= 2;

      if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m1 |= 0x04 ; m2 &= ~0x01; };
      if (((ss->people[3].id1 + 6) ^ cstuff) & 8) { m1 |= 0x08 ; m2 &= ~0x02; };
      if (((ss->people[8].id1 + 6) ^ cstuff) & 8) { m1 |= 0x10 ; m2 &= ~0x40; };
      if (((ss->people[9].id1 + 6) ^ cstuff) & 8) { m1 |= 0x20 ; m2 &= ~0x80; };
   }
   else if (cstuff == 6) {   /* Working together. */
      m1 = 0xE7; m2 = 0x7E;
   }
   else if (cstuff == 7) {   /* Working apart. */
      m1 = 0xDB; m2 = 0xBD;
   }
   else if (cstuff == 8) {   /* Working clockwise. */
      m1 = 0xF3; m2 = 0x3F;
   }
   else {                    /* Working counterclockwise. */
      m1 = 0xCF; m2 = 0xFC;
   }

   masks[0] = m1; masks[1] = m2;
   new_overlapped_setup_move(ss, MAPCODE(s2x4,2,MPKIND__OVERLAP,0), masks, result);
}


Private void do_concept_triple_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 the_map;
   int arg1 = parseptr->concept->value.arg1;

   if (ss->kind == s3x4)
      the_map = MAPCODE(s1x4,3,MPKIND__SPLIT,1);
   else if (ss->kind == s1x12)
      the_map = MAPCODE(s1x4,3,MPKIND__SPLIT,0);
   else if (ss->kind == sbigh)
      the_map = MAPCODE(s1x4,3,MPKIND__NONISOTROPIC,1);
   else if (ss->kind == sbigx)
      the_map = MAPCODE(s1x4,3,MPKIND__NONISOTROPIC,0);
   else
      fail("Must have triple 1x4's for this concept.");

   /* If this was triple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was triple
      lines, we forbid it. */

   if (arg1 & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (arg1 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   /* Sigh....  This clearly isn't the right way to do this.
      How about CMD_MISC__VERIFY_LINES? */

   if (arg1 != 0 && ss->kind != sbigh && ss->kind != sbigx) {
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
   
      if (!((arg1 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         else                      fail("There are no columns of 4 here.");
      }
   }

   new_divided_setup_move(ss, the_map, phantest_ok, TRUE, result);
}




Private void do_concept_triple_1x8_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 m1, m2;
   uint32 masks[2];
   int i, tbonetest;

   int cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward (lines) or left (cols)   : 0
      backward (lines) or right (cols) : 2 */

   int linesp = parseptr->concept->value.arg2;

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
   new_overlapped_setup_move(ss, MAPCODE(s2x8,2,MPKIND__OVERLAP,1),
      masks, result);
}


Private void do_concept_triple_diag(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   int q;

   if (ss->kind != s_bigblob) fail("Must have a rather large setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->value.arg1 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if ((global_livemask & ~0x56A56A) == 0) q = 0;
   else if ((global_livemask & ~0xA95A95) == 0) q = 2;
   else fail("Can't identify triple diagonal setup.");

   divided_setup_move(ss, maps_3diag[q + ((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)],
      phantest_ok, TRUE, result);
}


Private void do_concept_triple_diag_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff, q;
   uint32 m1, m2;
   uint32 masks[2];
   map_thing *map_ptr;

   cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8 (doesn't really exist)
      apart    : 9 (doesn't really exist) */

   if (ss->kind != s_bigblob) fail("Must have a rather large setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->value.arg2 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   /* Initially assign the centers to the right or upper (m2) group. */
   m1 = 0xF0; m2 = 0xFF;

   if ((global_livemask & ~0x56A56A) == 0) q = 0;
   else if ((global_livemask & ~0xA95A95) == 0) q = 2;
   else fail("Can't identify triple diagonal setup.");

   /* Look at the center line/column people and put each one in the correct group. */

   map_ptr = maps_3diagwk[q+((cstuff ^ global_tbonetest) & 1)];

   if ((cstuff + 1 - ss->people[map_ptr->maps[0]].id1) & 2) { m2 &= ~0x80 ; m1 |= 0x1; };
   if ((cstuff + 1 - ss->people[map_ptr->maps[1]].id1) & 2) { m2 &= ~0x40 ; m1 |= 0x2; };
   if ((cstuff + 1 - ss->people[map_ptr->maps[2]].id1) & 2) { m2 &= ~0x20 ; m1 |= 0x4; };
   if ((cstuff + 1 - ss->people[map_ptr->maps[3]].id1) & 2) { m2 &= ~0x10 ; m1 |= 0x8; };

   masks[0] = m1; masks[1] = m2;
   overlapped_setup_move(ss, map_ptr, masks, result);
}


Private void do_concept_grand_working(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff;
   uint32 tbonetest;
   uint32 m0, m1, m2, m3, m4;
   uint32 masks[8];
   setup_kind kk;
   int arity = 2;

   cstuff = parseptr->concept->value.arg1;
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
       ss->cmd.cmd_final_flags.her8it & INHERITFLAG_12_MATRIX)
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X6, FALSE);
   else if ((ss->kind == s2x4 || ss->kind == s2x6) &&
            ss->cmd.cmd_final_flags.her8it & INHERITFLAG_16_MATRIX)
      do_matrix_expansion(ss, CONCPROP__NEEDK_2X8, FALSE);

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
             (ss->cmd.cmd_final_flags.her8it & INHERITFLAG_12_MATRIX))) {

      ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_12_MATRIX;

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
             (ss->cmd.cmd_final_flags.her8it & INHERITFLAG_16_MATRIX))) {
      uint32 m5, m6;

      ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_16_MATRIX;

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
   new_overlapped_setup_move(ss, MAPCODE(kk,arity+1,MPKIND__OVERLAP,0), masks, result);
}


Private void do_concept_do_phantom_2x2(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* Do "blocks" or "4 phantom interlocked blocks" or "triangular blocks, etc. */

   if (ss->kind != s4x4) fail("Must have a 4x4 setup for this concept.");
   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");
   new_divided_setup_move(ss, MAPCODE(s2x4,2,parseptr->concept->value.arg3,0), (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 code;

   /* The values of arg2 are:
      CMD_MISC__VERIFY_DMD_LIKE   "diamonds" -- require diamond-like, i.e. centers in some kind of line, ends are line-like.
      CMD_MISC__VERIFY_1_4_TAG    "1/4 tags" -- centers in some kind of line, ends are a couple looking in (includes 1/4 line, etc.)
                                      If this isn't specific enough for you, use the "ASSUME LEFT 1/4 LINES" concept, or whatever.
      CMD_MISC__VERIFY_3_4_TAG    "3/4 tags" -- centers in some kind of line, ends are a couple looking out (includes 3/4 line, etc.)
                                      If this isn't specific enough for you, use the "ASSUME LEFT 3/4 LINES" concept, or whatever.
      CMD_MISC__VERIFY_QTAG_LIKE  "general 1/4 tags" -- all facing same orientation -- centers in some kind of line, ends are column-like.
      0                           "diamond spots" -- any facing direction is allowed. */

   if (ss->kind == s4dmd)
      code = MAPCODE(s_qtag,2,parseptr->concept->value.arg3,0);
   else if (ss->kind == s4ptpd)
      code = MAPCODE(s_ptpd,2,parseptr->concept->value.arg3,0);
   else
      fail("Must have a quadruple diamond/quarter-tag setup for this concept.");

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   new_divided_setup_move(ss, code, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_divided_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* See "do_concept_do_phantom_diamonds" for meaning of arg2. */

   if (ss->kind != s4x6 || (global_livemask & 0x02D02D) != 0)
      fail("Must have a divided diamond or 1/4 tag setup for this concept.");

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg3;
   new_divided_setup_move(ss, MAPCODE(s_qtag,2,MPKIND__SPLIT,1), (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_distorted(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   distorted_move(ss, parseptr, (disttest_kind) parseptr->concept->value.arg1, result);
}


Private void do_concept_dblbent(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{

/*
   Args from the concept are as follows:
   arg1 =
      0 - user claims this is some kind of columns
      1 - user claims this is some kind of lines
      3 - user claims this is waves
*/

   uint32 map_code = 0;

   if (global_livemask == 0xCF3) {
      ss->people[0].id1 = rotccw(ss->people[0].id1);
      ss->people[1].id1 = rotccw(ss->people[1].id1);
      ss->people[6].id1 = rotccw(ss->people[6].id1);
      ss->people[7].id1 = rotccw(ss->people[7].id1);
      map_code = MAPCODE(s1x8,1,MPKIND_DBLBENTCW,0);
   }
   else if (global_livemask == 0xF3C) {
      ss->people[2].id1 = rotcw(ss->people[2].id1);
      ss->people[3].id1 = rotcw(ss->people[3].id1);
      ss->people[8].id1 = rotcw(ss->people[8].id1);
      ss->people[9].id1 = rotcw(ss->people[9].id1);
      map_code = MAPCODE(s1x8,1,MPKIND_DBLBENTCCW,0);
   }

   if (ss->kind != sbigh || map_code == 0) fail("Must have double-bent setup for this concept.");

   switch (parseptr->concept->value.arg1 & 7) {
      case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
      case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
      case 3: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES; break;
   }

   new_divided_setup_move(ss, map_code, phantest_ok, TRUE, result);

   if (result->kind == sbigh) {
      if (global_livemask == 0xCF3) {
         result->people[0].id1 = rotcw(result->people[0].id1);
         result->people[1].id1 = rotcw(result->people[1].id1);
         result->people[6].id1 = rotcw(result->people[6].id1);
         result->people[7].id1 = rotcw(result->people[7].id1);
      }
      else {
         result->people[2].id1 = rotccw(result->people[2].id1);
         result->people[3].id1 = rotccw(result->people[3].id1);
         result->people[8].id1 = rotccw(result->people[8].id1);
         result->people[9].id1 = rotccw(result->people[9].id1);
      }
   }
   else if (result->kind == s4x6) {
      if (global_livemask == 0xCF3) {
         result->people[0].id1 = rotcw(result->people[0].id1);
         result->people[1].id1 = rotcw(result->people[1].id1);
         result->people[12].id1 = rotcw(result->people[12].id1);
         result->people[13].id1 = rotcw(result->people[13].id1);
      }
      else {
         result->people[4].id1 = rotccw(result->people[4].id1);
         result->people[5].id1 = rotccw(result->people[5].id1);
         result->people[16].id1 = rotccw(result->people[16].id1);
         result->people[17].id1 = rotccw(result->people[17].id1);
      }
   }
}


Private void do_concept_do_phantom_2x3(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s3x4) fail("Must have a 3x4 setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (((parseptr->concept->value.arg2 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no 12-matrix columns here.");
      else                      fail("There are no 12-matrix lines here.");
   }

   new_divided_setup_move(ss, MAPCODE(s2x3,2,parseptr->concept->value.arg3,1), (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_divided_2x4(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");

   if (parseptr->concept->value.arg2 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if ((((parseptr->concept->value.arg2 ^ global_tbonetest) & 1) == 0) || ((global_tbonetest & 011) == 011)) {
      if (parseptr->concept->value.arg2 & 1) fail("There are no divided lines here.");
      else                                   fail("There are no divided columns here.");
   }

   new_divided_setup_move(ss, MAPCODE(s2x4,2,MPKIND__SPLIT,0), (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_divided_2x3(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s2x6) fail("Must have a 2x6 setup for this concept.");

   if (parseptr->concept->value.arg2 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if ((((parseptr->concept->value.arg2 ^ global_tbonetest) & 1) == 0) || ((global_tbonetest & 011) == 011)) {
      if (parseptr->concept->value.arg2 & 1) fail("There are no 12-matrix divided lines here.");
      else                                   fail("There are no 12-matrix divided columns here.");
   }

   new_divided_setup_move(ss, MAPCODE(s2x3,2,MPKIND__SPLIT,0), (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_1x6(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 map_code;

   if (ss->kind == s2x6)
      map_code = MAPCODE(s1x6,2,MPKIND__SPLIT,1);
   else if (ss->kind == s1x12)
      map_code = MAPCODE(s1x6,2,MPKIND__SPLIT,0);
   else
      fail("Must have a 2x6 or 1x12 setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->value.arg3 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no lines of 6 here.");
      else                      fail("There are no columns of 6 here.");
   }

   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   new_divided_setup_move(ss, map_code,
                          (phantest_kind) parseptr->concept->value.arg1,
                          TRUE, result);
}


Private void do_concept_do_phantom_1x8(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 map_code;

   if (ss->kind == s2x8) {
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if (parseptr->concept->value.arg3 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no grand lines here.");
         else                      fail("There are no grand columns here.");
      }

      map_code = MAPCODE(s1x8,2,MPKIND__SPLIT,1);
   }
   else if (ss->kind == s1x16) {
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if (parseptr->concept->value.arg3 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no grand lines here.");
         else                      fail("There are no grand columns here.");
      }

      map_code = MAPCODE(s1x8,2,MPKIND__SPLIT,0);
   }
   else
      fail("Must have a 2x8 or 1x16 setup for this concept.");

   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   new_divided_setup_move(ss, map_code,
                          (phantest_kind) parseptr->concept->value.arg1,
                          TRUE, result);
}


Private void do_concept_do_phantom_triple_1x6(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 map_code;

   if (ss->kind != s3x6) fail("Must have a 3x6 setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->value.arg3 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no grand lines here.");
      else                      fail("There are no grand columns here.");
   }

   map_code = MAPCODE(s1x6,3,MPKIND__SPLIT,1);

   new_divided_setup_move(ss, map_code, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_triple_1x8(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   uint32 map_code;

   if (ss->kind != s3x8) fail("Must have a 3x8 setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->value.arg3 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no grand lines here.");
      else                      fail("There are no grand columns here.");
   }

   map_code = MAPCODE(s1x8,3,MPKIND__SPLIT,1);

   new_divided_setup_move(ss, map_code, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_once_removed(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_thing *division_maps;
   uint32 the_map = ~0UL;

   // We allow "3x1" and the like only with plain "once removed".
   if (parseptr->concept->value.arg1 &&
       ss->cmd.cmd_final_flags.her8it & (INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK))
      fail("Illegal modifier before a concept.");

   if (parseptr->concept->value.arg1 == 2) {
      switch (ss->kind) {
      case s1x6:
         the_map = MAPCODE(s1x2,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s1x12:
         the_map = MAPCODE(s1x4,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s2x6:
         the_map = MAPCODE(s2x2,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s2x12:
         the_map = MAPCODE(s2x4,3,MPKIND__TWICE_REMOVED,0);
         break;
      case s_ptpd:
         the_map = MAPCODE(s2x2,3,MPKIND__SPEC_TWICEREM,0);
         break;
      case s_bone:      /* Figure this out -- it is special. */
      case s_rigger:    /* Figure this out -- it is special. */
      default:
         fail("Can't do 'twice removed' from this setup.");
      }
   }
   else if (parseptr->concept->value.arg1 == 3) {
      switch (ss->kind) {
      case s1x8:
         the_map = MAPCODE(s1x2,4,MPKIND__THRICE_REMOVED,0);
         break;
      case s1x16:
         the_map = MAPCODE(s1x4,4,MPKIND__THRICE_REMOVED,0);
         break;
      case s2x8:
         the_map = MAPCODE(s2x2,4,MPKIND__THRICE_REMOVED,0);
         break;
      default:
         fail("Can't do 'thrice removed' from this setup.");
      }
   }
   else {

      // We allow "3x1" or "1x3".  That's all.

      switch (ss->cmd.cmd_final_flags.her8it & (INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK)) {
      case 0:
         break;
      case INHERITFLAGMXNK_1X3:
      case INHERITFLAGMXNK_3X1:
         warn(warn__tasteless_junk);
         // We allow "12 matrix", but do not require it.  We have no
         // idea whether it should be required.
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_12_MATRIX;
         switch (ss->kind) {
         case s3x4:
            division_maps = &map_2x3_rmvr;
            goto doit;
         case s_qtag:
            division_maps = &map_2x3_rmvs;
            goto doit;
         default:
            fail("Can't do this concept in this formation.");
         }
         break;
      default:
         fail("Illegal modifier before a concept.");
      }

      if (parseptr->concept->value.arg1) {
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
         the_map = MAPCODE(s2x2,2,MPKIND__REMOVED,0);
         break;
      case s2x6:
         the_map = MAPCODE(s2x3,2,MPKIND__REMOVED,0);
         break;
      case s2x8:
         the_map = MAPCODE(s2x4,2,MPKIND__REMOVED,0);
         break;
      case s1x8:
         the_map = MAPCODE(s1x4,2,MPKIND__REMOVED,0);
         break;
      case s1x6:
         the_map = MAPCODE(s1x3,2,MPKIND__REMOVED,0);
         break;
      case s1x4:
         the_map = MAPCODE(s1x2,2,MPKIND__REMOVED,0);
         break;
      case s_rigger:
         the_map = MAPCODE(sdmd,2,MPKIND__REMOVED,0);
         break;
      case s_bone:    /* This is for you, Clark. */
         the_map = MAPCODE(s_trngl4,2,MPKIND__REMOVED,1);
         break;
      case s_ptpd:    /* This is for you, Clark. */
         /* It's too bad we can't use the "MAPCODE" mechanism here.  Maybe can fix,
            since the stuff in "innards" is fussing with these anyway. */
         division_maps = &map_p8_tgl4;
         break;
      case s_spindle:
         division_maps = &map_spndle_once_rem;
         break;
      case s1x3dmd:
         division_maps = &map_1x3dmd_once_rem;
         break;
      case s_qtag:
         the_map = MAPCODE(sdmd,2,MPKIND__REMOVED,1);
         break;
      default:
         fail("Can't do 'once removed' from this setup.");
      }
   }

 doit:

   if (the_map == ~0UL)
      divided_setup_move(ss, division_maps, phantest_ok, TRUE, result);
   else
      new_divided_setup_move(ss, the_map, phantest_ok, TRUE, result);
}



Private void do_concept_diagnose(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   char junk[200];

   move(ss, FALSE, result);

   sprintf(junk, "Command flags: 0x%08X, result flags: 0x%08X.",
         (unsigned int) ss->cmd.cmd_misc_flags, (unsigned int) result->result_flags);
   fail(junk);
}


Private void do_concept_old_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 mxnbits = ss->cmd.cmd_final_flags.her8it & INHERITFLAG_MXNMASK;
   long_boolean mxnstuff = mxnbits == INHERITFLAGMXNK_1X3 || mxnbits == INHERITFLAGMXNK_3X1;

   move(ss, FALSE, result);

   /* We don't check if this was 3x1 -- too complicated for now. */

   if (!mxnstuff &&
       !(((((result->rotation & 1) * 7) + 1) * RESULTFLAG__SPLIT_AXIS_XMASK) &
         result->result_flags))
      fail("Stretch call was not a 4 person call divided along stretching axis.");

   if (mxnstuff && (result->kind == s1x8)) {
      /* This is a bit sleazy. */

      if (((result->people[0].id1 ^ result->people[1].id1) & 2) ||
               (result->result_flags & RESULTFLAG__VERY_ENDS_ODD)) {
         swap_people(result, 1, 6);
         swap_people(result, 2, 5);
         swap_people(result, 3, 7);
      }
      else if (((result->people[2].id1 ^ result->people[3].id1) & 2) ||
               (result->result_flags & RESULTFLAG__VERY_CTRS_ODD)) {
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
      else
         fail("Stretch call didn't go to a legal setup.");
   }
}


Private void do_concept_new_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 maps;
   setup tempsetup = *ss;
   int linesp = parseptr->concept->value.arg1;

   /* linesp =
      16 : any setup
      1  : line
      3  : wave
      4  : column
      18 : box
      19 : diamond */

   if (     (linesp == 18 && tempsetup.kind != s2x4) ||
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
   else if (tempsetup.kind == s_rigger) {
      swap_people(&tempsetup, 0, 1);
      swap_people(&tempsetup, 4, 5);
      maps = MAPCODE(s_trngl4,2,MPKIND__SPLIT,1);
   }
   else
      fail("Stretched setup call didn't start in appropriate setup.");

   new_divided_setup_move(&tempsetup, maps, phantest_ok, TRUE, result);
}


Private void do_concept_mirror(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This is treated differently from "left" or "reverse" in that the code in sdmoves.c
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
   move(ss, FALSE, result);
   mirror_this(result);
}






Private void do_concept_assume_waves(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   assumption_thing t;
   assumption_thing *e = &ss->cmd.cmd_assume;   /* Existing assumption. */
   long_boolean no_phan_error = FALSE;
   setup sss;
   int i;

   /* "Assume normal casts" is special. */

   if (parseptr->concept->value.arg1 == cr_alwaysfail) {
      if (e->assump_cast)
         fail("Redundant or conflicting assumptions.");
      e->assump_cast = 1;
      move(ss, FALSE, result);
      return;
   }

   /* Check that there actually are some phantoms needing to be filled in.
      If user specifically said there are phantoms, we believe it. */

   if (!(ss->cmd.cmd_misc_flags & CMD_MISC__PHANTOMS)) {
      no_phan_error = TRUE;
      for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++) {
         if (!ss->people[i].id1) no_phan_error = FALSE;
      }
   }

   /* We wish it were possible to encode this entire word neatly in the concept
      table, but, unfortunately, the way C struct and union initializers work
      makes it impossible. */

   t.assumption = (call_restriction) parseptr->concept->value.arg1;
   t.assump_col = parseptr->concept->value.arg2;
   t.assump_both = parseptr->concept->value.arg3;
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
      uint32 u;
      int idx;

      switch (ss->kind) {
      case s2x4:
      case s2x6:
      case s2x8:
      case s2x3:
         for (idx=0,u=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++)
            u |= ss->people[idx].id1;

         if (!(u&010)) t.assump_col++;
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
         case s2x4: goto check_it;
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

   no_phan_error = FALSE;    /* There really were phantoms after all. */

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




Private void do_concept_active_phantoms(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->cmd.cmd_assume.assump_cast)
      fail("Don't use \"active phantoms\" with \"assume normal casts\".");

   if (fill_active_phantoms_and_move(ss, result))
      fail("This assumption is not specific enough to fill in active phantoms.");
}


Private void do_concept_central(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (parseptr->concept->value.arg1 == CMD_MISC2__SAID_INVERT) {
      /* If this is "invert", just flip the bit.  They can stack, of course. */
      ss->cmd.cmd_misc2_flags ^= CMD_MISC2__SAID_INVERT;
   }
   else {
      uint32 this_concept = parseptr->concept->value.arg1;

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

      if (parseptr->concept->value.arg1 & CMD_MISC2__DO_CENTRAL) {

         /* We are having a problem here.  It seems that we need to refrain
            from forcing the split if 4x4 was given.  The test is [gwv]
            finally 4x4 central stampede. */

         if (setup_attrs[ss->kind].setup_limits == 7 &&
             ((ss->cmd.cmd_final_flags.her8it & INHERITFLAG_NXNMASK) != INHERITFLAGNXNK_4X4)) {
            if (ss->rotation & 1)
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_VERT;
            else
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;
         }
         /*   and it looks as though we had a problem here too.
              else if (setup_attrs[ss->kind].setup_limits != 3)
              fail("Need a 4 or 8 person setup for this.");
         */
      }

      ss->cmd.cmd_misc2_flags |= this_concept;
   }

   move(ss, FALSE, result);
}


Private void do_concept_crazy(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i;
   int craziness, highlimit;
   setup tempsetup;
   setup_command cmd;

   uint32 finalresultflags = 0;
   int reverseness = parseptr->concept->value.arg1;
   int this_part;

   tempsetup = *ss;

   if (TEST_HERITBITS(tempsetup.cmd.cmd_final_flags,INHERITFLAG_REVERSE)) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      if (parseptr->concept->value.arg2) fail("Don't put 'reverse' in front of the fraction.");
      reverseness = 1;
   }

   tempsetup.cmd.cmd_final_flags.her8it &= ~INHERITFLAG_REVERSE;
   // We don't allow other flags, like "cross", but we do allow "MxN".
   if ((tempsetup.cmd.cmd_final_flags.her8it & ~(INHERITFLAG_MXNMASK|INHERITFLAG_NXNMASK)) |
       tempsetup.cmd.cmd_final_flags.final)
      fail("Illegal modifier before \"crazy\".");

   cmd = tempsetup.cmd;    /* We will modify these flags, and, in any case, we need
                              to rematerialize them at each step. */

   /* If we didn't do this check, and we had a 1x4, the "do it on each side"
      stuff would just do it without splitting or thinking anything was unusual,
      while the "do it in the center" code would catch it at present, but might
      not in the future if we add the ability of the concentric schema to mean
      pick out the center 2 from a 1x4.  In any case, if we didn't do this
      check, "1/4 reverse crazy bingo" would be legal from a 2x2. */

   if (setup_attrs[tempsetup.kind].setup_limits < 7)
      fail("Need an 8-person setup for this.");

   if (parseptr->concept->value.arg2)
      craziness = parseptr->options.number_fields;
   else
      craziness = 4;

   i = 0;   /* The start point. */
   highlimit = craziness;   /* The end point. */

   this_part = ((cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT);

   if (this_part > 0) {
      switch (cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) {
      case CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE:
         /* Request is to do just part this_part. */
         i = this_part-1;
         highlimit = this_part;
         finalresultflags |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (highlimit == craziness) finalresultflags |= RESULTFLAG__DID_LAST_PART;
         break;
      case CMD_FRAC_CODE_FROMTO | CMD_FRAC_NULL_VALUE:
         /* Request is to do everything up through part this_part. */
         highlimit = this_part;
         break;
      case CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE:
         /* Request is to do everything strictly after part this_part-1. */
         i = this_part-1;
         break;
      default:
         fail("\"crazy\" is not allowed after this concept.");
      }
   }

   if (highlimit <= i || highlimit > craziness) fail("Illegal fraction for \"crazy\".");

   if ((cmd.cmd_frac_flags & 0xFFFF) != CMD_FRAC_NULL_VALUE)
      fail("\"crazy\" is not allowed with fractional concepts.");

   if (cmd.cmd_frac_flags & CMD_FRAC_REVERSE)
      reverseness ^= (craziness ^ 1);    /* That's all it takes! */

   /* No fractions for subject, we have consumed them. */
   cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;

   /* Lift the craziness restraint from before -- we are about to pull things apart. */
   cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;

   for ( ; i<highlimit; i++) {
      tempsetup.cmd = cmd;    /* Get a fresh copy of the command. */

      update_id_bits(&tempsetup);

      if ((i ^ reverseness) & 1) {
         /* Do it in the center. */
         selective_move(&tempsetup, parseptr, selective_key_plain,
                        0, 0, 0, selector_center4, FALSE, result);
      }
      else {
         /* Do it on each side. */
         if (tempsetup.rotation & 1)
            tempsetup.cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_VERT;
         else
            tempsetup.cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;
         move(&tempsetup, FALSE, result);
      }

      finalresultflags |= result->result_flags;

      tempsetup = *result;
   }

   result->result_flags = finalresultflags;
}


static void do_concept_phan_crazy(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i, craziness;
   setup tempsetup;
   setup_command cmd;
   setup_kind kk;
   uint32 mapcode;
   int rot;

   uint32 finalresultflags = 0;
   int reverseness = (parseptr->concept->value.arg1 >> 3) & 1;

   if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_REVERSE)) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      reverseness = 1;
   }
   ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_REVERSE;
   /* We don't allow other flags, like "cross". */
   if (ss->cmd.cmd_final_flags.her8it | ss->cmd.cmd_final_flags.final)
      fail("Illegal modifier before \"crazy\".");

   tempsetup = *ss;

   cmd = tempsetup.cmd;    /* We will modify these flags, and, in any case, we need
                              to rematerialize them at each step. */

   if (parseptr->concept->value.arg1 & 16)
      craziness = parseptr->options.number_fields;
   else
      craziness = 4;

   /* Turn on "verify waves" or whatever, for the first time only. */
   tempsetup.cmd.cmd_misc_flags |= parseptr->concept->value.arg3;

   /* Decide which way we are going to divide, once only. */

   if ((parseptr->concept->value.arg1 & 7) < 4) {
      rot = (global_tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;
      if ((global_tbonetest & 011) == 011) fail("People are T-boned -- try using 'standard'.");

      /* We have now processed any "standard" information to determine how to split the setup.
         If this was done nontrivially, we now have to shut off the test that we will do in the
         divided setup -- that test would fail.  Also, we do not allow "waves", only "lines" or
         "columns". */

      if ((orig_tbonetest & 011) == 011) {
         tempsetup.cmd.cmd_misc_flags &= ~CMD_MISC__VERIFY_MASK;
         if ((parseptr->concept->value.arg1 & 7) == 3)
            fail("Don't use 'crazy waves' with standard; use 'crazy lines'.");
      }

      kk = s4x4;
      mapcode = MAPCODE(s2x4,2,MPKIND__SPLIT,1);
   }
   else if ((parseptr->concept->value.arg1 & 7) == 4) {
      rot = tempsetup.rotation & 1;
      kk = s2x8;
      mapcode = MAPCODE(s2x4,2,MPKIND__SPLIT,0);
   }
   else {
      rot = tempsetup.rotation & 1;
      kk = s4dmd;
      mapcode = MAPCODE(s_qtag,2,MPKIND__SPLIT,0);
   }

   tempsetup.rotation += rot;   /* Just flip the setup around and recanonicalize. */
   canonicalize_rotation(&tempsetup);

   for (i=0 ; i<craziness; i++) {
      // Check the validity of the setup each time.
      if (tempsetup.kind != kk || tempsetup.rotation != 0)
         fail("Can't do crazy phantom or offset in this setup.");

      if ((i ^ reverseness) & 1) {        // Do it in the center.
         if (parseptr->concept->value.arg1 & 64) { // Crazy offset C/L/W.
            setup save = tempsetup;   // Save the outer people.
            clear_person(&save, 10);
            clear_person(&save, 15);
            clear_person(&save, 3);
            clear_person(&save, 1);
            clear_person(&save, 2);
            clear_person(&save, 7);
            clear_person(&save, 11);
            clear_person(&save, 9);
            divided_setup_move(&tempsetup, &map_crazy_offset2,
                               phantest_only_one, TRUE, result);
            // The outer people have now been dropped from result.
            merge_setups(&save, merge_strict_matrix, result);
         }
         else
            concentric_move(&tempsetup, (setup_command *) 0, &tempsetup.cmd,
                            schema_in_out_quad, 0, 0, TRUE, result);
      }
      else {                              // Do it on each side.
         if (parseptr->concept->value.arg1 & 64)   // Crazy offset C/L/W.
            divided_setup_move(&tempsetup, &map_crazy_offset1,
                               phantest_only_one_pair, TRUE, result);
         else
            new_divided_setup_move(&tempsetup, mapcode, phantest_ok, TRUE, result);
      }

      finalresultflags |= result->result_flags;
      tempsetup = *result;
      tempsetup.cmd = cmd;    /* Get a fresh copy of the command for next time. */
   }

   result->rotation -= rot;   /* Flip the setup back. */
   /* The split-axis bits are gone.  If someone needs them, we have work to do. */
   result->result_flags = finalresultflags & ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
}



static void do_concept_fan(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tempsetup;
   uint32 finalresultflags = 0;
   /* This is a huge amount of kludgy stuff shoveled in from a variety of sources.
      It needs to be cleaned up and thought about. */

   uint64 new_final_concepts;
   Const parse_block *parseptrcopy;
   callspec_block *callspec;

   new_final_concepts.her8it = 0;
   new_final_concepts.final = 0;
   parseptrcopy = process_final_concepts(parseptr->next, TRUE, &new_final_concepts);

   if (new_final_concepts.her8it || new_final_concepts.final || parseptrcopy->concept->kind > marker_end_of_list)
      fail("Can't do \"fan\" followed by another concept or modifier.");

   callspec = parseptrcopy->call;

   if (!callspec || !(callspec->callflagsf & CFLAG2_CAN_BE_FAN))
      fail("Can't do \"fan\" with this call.");

   /* Step to a wave if necessary.  This is actually only needed for the "yoyo" concept.
      The "fan" concept could take care of itself later.  However, we do them both here. */

   if (     !(ss->cmd.cmd_misc_flags & (CMD_MISC__NO_STEP_TO_WAVE | CMD_MISC__ALREADY_STEPPED)) &&
            (callspec->callflags1 & CFLAG1_STEP_REAR_MASK) == CFLAG1_STEP_TO_WAVE) {
      ss->cmd.cmd_misc_flags |= CMD_MISC__ALREADY_STEPPED;  /* Can only do it once. */
      touch_or_rear_back(ss, FALSE, callspec->callflags1);
   }

   tempsetup = *ss;

   /* Normally, set the fractionalize field to start with the second part.
      But if we have been requested to do a specific part number of "fan <call>",
      just add one to the part number and do the call. */

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
   move(&tempsetup, FALSE, result);
   finalresultflags |= result->result_flags;
   result->result_flags = finalresultflags & ~3;
}



Private void do_concept_stable(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 directions[32];
   long_boolean selected[32];
   setup_kind kk;
   int n, i, rot, howfar, orig_rotation;

   long_boolean fractional = parseptr->concept->value.arg2;
   long_boolean everyone = !parseptr->concept->value.arg1;
   selector_kind saved_selector = current_options.who;

   current_options.who = parseptr->options.who;

   howfar = parseptr->options.number_fields;
   if (fractional && howfar > 4)
      fail("Can't do fractional stable more than 4/4.");

   n = setup_attrs[ss->kind].setup_limits;
   if (n < 0) fail("Sorry, can't do stable starting in this setup.");

   for (i=0; i<=n; i++) {           /* Save current facing directions. */
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

   orig_rotation = ss->rotation;
   move(ss, FALSE, result);
   rot = ((orig_rotation - result->rotation) & 3) * 011;

   kk = result->kind;

   if (kk == s_dead_concentric)
      kk = result->inner.skind;

   n = setup_attrs[kk].setup_limits;
   if (n < 0) fail("Sorry, can't do stable going to this setup.");

   for (i=0; i<=n; i++) {           /* Restore facing directions of selected people. */
      uint32 p = result->people[i].id1;
      if (p & BIT_PERSON) {
         if (selected[(p >> 6) & 037]) {
            if (fractional) {
               if (!(p & STABLE_ENAB))
                  fail("fractional stable not supported for this call.");
               result->people[i].id1 =
                  rotperson(p, ((0 - ((p & (STABLE_VBIT*3)) / STABLE_VBIT)) & 3) * 011);
            }
            else {
               result->people[i].id1 = rotperson(
                     (p & ~(d_mask | STABLE_MASK)) |
                        (directions[(p >> 6) & 037] & (d_mask | STABLE_MASK)),
                     rot);
            }
         }
         if (fractional)
            result->people[i].id1 &= ~STABLE_MASK;
      }
   }
}


Private void do_concept_emulate(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i, j, n, m, rot;
   setup a1;
   setup res1;

   a1 = *ss;

   move(&a1, FALSE, &res1);

   *result = res1;      /* Get all the other fields. */

   result->kind = ss->kind;
   result->rotation = ss->rotation;

   n = setup_attrs[ss->kind].setup_limits;
   m = setup_attrs[res1.kind].setup_limits;
   if (n < 0 || m < 0) fail("Sorry, can't do emulate in this setup.");

   rot = ((res1.rotation-result->rotation) & 3) * 011;

   for (i=0; i<=n; i++) {
      uint32 p;

      result->people[i] = ss->people[i];

      p = ss->people[i].id1;

      if (p & BIT_PERSON) {
         for (j=0; j<=m; j++) {
            uint32 q = res1.people[j].id1;
            if (((q ^ p) & XPID_MASK) == 0) {
               result->people[i].id1 &= ~077;
               result->people[i].id1 |= (rotperson(q, rot) & 077);
               goto did_it;
            }
         }
         fail("Lost someone else during emulate call.");
         did_it: ;
      }
   }
}


Private void do_concept_checkerboard(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* arg1 = setup (1x4/2x2/dmd)
      arg2 = 0 : checkersetup
             1 : shadow setup
             2 : orbitsetup
             3 : twin orbitsetup
             add 8 for selected people (checker only) */

   static Const veryshort mape[20] = {0, 2, 4, 6, 1, 3, 5, 7, 0, 1, 4, 5, 2, 3, 6, 7, 0, 3, 4, 7};
   static Const veryshort mapl[16] = {7, 1, 3, 5, 0, 6, 4, 2, 7, 6, 3, 2, 0, 1, 4, 5};
   static Const veryshort mapb[20] = {1, 3, 5, 7, 0, 2, 4, 6, 2, 3, 6, 7, 0, 1, 4, 5, 1, 2, 5, 6};
   static Const veryshort mapd[16] = {7, 1, 3, 5, 0, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1, -1};
   static Const veryshort wave_tester[8] = {011, 012, 011, 012, 012, 011, 012, 011};
   static Const veryshort twof_tester[8] = {011, 011, 012, 012, 012, 012, 011, 011};

   int i, rot;
   int offset = 0;
   uint32 t;
   setup a1;
   setup res1;
   Const veryshort *map_ptr;
   setup_kind kn = (setup_kind) parseptr->concept->value.arg1;

   clear_people(result);

   if (parseptr->concept->value.arg2 == 1) {
      /* This is "shadow <setup>" */

      setup_command subsid_cmd;

      if (     (kn != s2x2 || ss->kind != s2x4) &&
               (kn != s1x4 || (ss->kind != s_qtag && ss->kind != s_bone)) &&
               (kn != sdmd || (ss->kind != s_hrglass && ss->kind != s_dhrglass)))
         fail("Not in correct setup for 'shadow line/box/diamond' concept.");

      subsid_cmd = ss->cmd;
      subsid_cmd.parseptr = (parse_block *) 0;
      subsid_cmd.callspec = base_calls[base_call_ends_shadow];
      subsid_cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
      subsid_cmd.cmd_final_flags.her8it = 0;
      subsid_cmd.cmd_final_flags.final = 0;
      concentric_move(ss, &ss->cmd, &subsid_cmd, schema_concentric, 0,
                      DFM1_CONC_DEMAND_LINES | DFM1_CONC_FORCE_COLUMNS, TRUE, result);
      return;
   }

   /* This is "checker/orbit <setup>" */

   if (ss->kind != s2x4) fail("Must have a 2x4 setup for 'checker' concept.");

   if (parseptr->concept->value.arg2 & 8) {
      /* This is "so-and-so preferred for the trade, checkerboard". */
      if      (global_selectmask == 0x55)
         offset = 0;
      else if (global_selectmask == 0xAA)
         offset = 4;
      else if (global_selectmask == 0x33)
         offset = 8;
      else if (global_selectmask == 0xCC)
         offset = 12;
      else if (global_selectmask == 0x99)
         offset = 16;
      else fail("Can't select these people.");
   }
   else {
      uint32 directions = 0;
      uint32 wave_test = 0;
      uint32 twof_test = 0;

      for (i=0; i<8; i++) {
         uint32 p = ss->people[i].id1 & d_mask;

         directions = directions<<4;
         if (p == d_north) { directions |= 0x8; wave_test |= wave_tester[i]; twof_test |= twof_tester[i]; }
         if (p != d_north) directions |= 0x4;
         if (p == d_south) { directions |= 0x2; wave_test |= ~wave_tester[i]; twof_test |= ~twof_tester[i]; }
         if (p != d_south) directions |= 0x1;
      }

      if      ((directions & 0x84842121) == 0x84842121)
         offset = 0;
      else if ((directions & 0x48481212) == 0x48481212)
         offset = 4;
      else if ((directions & 0x88442211) == 0x88442211)
         offset = 8;
      else if ((directions & 0x44881122) == 0x44881122)
         offset = 12;
      else if ((directions & 0x84482112) == 0x84482112)
         offset = 16;
      else if (ss->cmd.cmd_assume.assumption == cr_wave_only &&
               ss->cmd.cmd_assume.assump_col == 0 &&
               ss->cmd.cmd_assume.assump_negate == 0) {
         if (wave_test != 0 && !(wave_test & 2))
            offset = 0;
         else if (wave_test != 0 && !(wave_test & 1))
            offset = 4;
         else
            fail("Can't identify checkerboard people.");
      }
      else
         fail("Can't identify checkerboard people.");
   }

   if (offset == 16 && kn != s2x2)
      fail("Can't identify checkerboard people.");

   if (      ((t = ss->people[mape[0+offset]].id1) && (t & d_mask) != d_north) ||
             ((t = ss->people[mape[1+offset]].id1) && (t & d_mask) != d_north) ||
             ((t = ss->people[mape[2+offset]].id1) && (t & d_mask) != d_south) ||
             ((t = ss->people[mape[3+offset]].id1) && (t & d_mask) != d_south))
         fail("Selected people are not facing out.");

   if ((parseptr->concept->value.arg2 & 7) == 2) {
      /* orbitboard */
      if (offset > 4) fail("Can't find orbiting people.");
      /* Move the people who simply orbit, filling in their roll info. */

      (void) copy_rot(result, mape[2+offset], ss, mape[1+offset], 0);
      if (result->people[mape[2+offset]].id1) result->people[mape[2+offset]].id1 = (result->people[mape[2+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[1+offset], ss, mape[0+offset], 022);
      if (result->people[mape[1+offset]].id1) result->people[mape[1+offset]].id1 = (result->people[mape[1+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[0+offset], ss, mape[3+offset], 0);
      if (result->people[mape[0+offset]].id1) result->people[mape[0+offset]].id1 = (result->people[mape[0+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[3+offset], ss, mape[2+offset], 022);
      if (result->people[mape[3+offset]].id1) result->people[mape[3+offset]].id1 = (result->people[mape[3+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
   }
   else if ((parseptr->concept->value.arg2 & 7) == 3) {
      /* twin orbitboard */
      if (offset > 4) fail("Can't find orbiting people.");
      /* Move the people who simply orbit, filling in their roll info. */

      (void) copy_rot(result, mape[2+offset], ss, mape[1+offset], 0);
      if (result->people[mape[2+offset]].id1) result->people[mape[2+offset]].id1 = (result->people[mape[2+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[3+offset], ss, mape[0+offset], 0);
      if (result->people[mape[3+offset]].id1) result->people[mape[3+offset]].id1 = (result->people[mape[3+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
      (void) copy_rot(result, mape[0+offset], ss, mape[3+offset], 0);
      if (result->people[mape[0+offset]].id1) result->people[mape[0+offset]].id1 = (result->people[mape[0+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[1+offset], ss, mape[2+offset], 0);
      if (result->people[mape[1+offset]].id1) result->people[mape[1+offset]].id1 = (result->people[mape[1+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
   }
   else {
      /* checkerboard */
      /* Move the people who simply trade, filling in their roll info. */

      (void) copy_rot(result, mape[0+offset], ss, mape[1+offset], 022);
      if (result->people[mape[0+offset]].id1) result->people[mape[0+offset]].id1 = (result->people[mape[0+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
      (void) copy_rot(result, mape[1+offset], ss, mape[0+offset], 022);
      if (result->people[mape[1+offset]].id1) result->people[mape[1+offset]].id1 = (result->people[mape[1+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[2+offset], ss, mape[3+offset], 022);
      if (result->people[mape[2+offset]].id1) result->people[mape[2+offset]].id1 = (result->people[mape[2+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
      (void) copy_rot(result, mape[3+offset], ss, mape[2+offset], 022);
      if (result->people[mape[3+offset]].id1) result->people[mape[3+offset]].id1 = (result->people[mape[3+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
   }

   a1 = *ss;

   switch (kn) {
      case s1x4:
         map_ptr = mapl;
         break;
      case s2x2:
         map_ptr = mapb;
         break;
      case sdmd:
         map_ptr = mapd;
         break;
   }

   map_ptr += offset;
   if (*map_ptr < 0) fail("Can't select these people.");

   for (i=0; i<4; i++) (void) copy_person(&a1, i, ss, map_ptr[i]);

   a1.kind = kn;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a1.rotation = 0;
   a1.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a1);
   move(&a1, FALSE, &res1);

   /* Look at the rotation coming out of the move.  If the setup is 1x4, we require it to be
      even (checkerboard lockit not allowed).  Otherwise, allow any rotation.  This means
      we allow diamonds that are oriented rather peculiarly! */

   if ((res1.rotation != 0) && res1.kind == s1x4)
      fail("'Checker' call went to 1x4 setup oriented wrong way.");

   rot = res1.rotation * 011;

   if (offset == 16 && res1.kind != s2x2)
      fail("Don't recognize ending setup after 'checker' call.");

   switch (res1.kind) {
      case s1x4:
         map_ptr = mapl;
         break;
      case s2x2:
         map_ptr = mapb;
         break;
      case sdmd:
         map_ptr = mapd;
         break;
      default:
         fail("Don't recognize ending setup after 'checker' call.");
   }

   map_ptr += offset;
   if (*map_ptr < 0) fail("Can't do this.");

   for (i=0; i<4; i++) (void) copy_rot(result, map_ptr[i], &res1, (i-res1.rotation)&3, rot);

   result->kind = s2x4;
   result->rotation = 0;
   result->result_flags = res1.result_flags;
   reinstate_rotation(ss, result);
}


Private void do_concept_checkpoint(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int reverseness = parseptr->concept->value.arg1;
   setup_command subsid_cmd;

   if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_REVERSE)) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      reverseness = 1;
   }

   ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_REVERSE;
   /* We don't allow other flags, like "cross". */
   if (ss->cmd.cmd_final_flags.her8it | ss->cmd.cmd_final_flags.final)
      fail("Illegal modifier before \"checkpoint\".");

   subsid_cmd = ss->cmd;
   subsid_cmd.parseptr = parseptr->subsidiary_root;

   /* The "dfm_conc_force_otherway" flag forces Callerlab interpretation:
      If checkpointers go from 2x2 to 2x2, this is clear.
      If checkpointers go from 1x4 to 2x2, "dfm_conc_force_otherway" forces
         the Callerlab rule in preference to the "parallel_concentric_end" property
         on the call. */

   if (reverseness)
      concentric_move(ss, &ss->cmd, &subsid_cmd, schema_rev_checkpoint, 0, 0, TRUE, result);
   else
      concentric_move(ss, &subsid_cmd, &ss->cmd, schema_checkpoint, 0, DFM1_CONC_FORCE_OTHERWAY, TRUE, result);
}



typedef struct {
   fraction_info fracs;
   int repetitions;
} repetitionrec;



Private long_boolean do_call_under_repetition(repetitionrec *yyy, setup *ss, setup *result)
{
   uint32 save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
   uint32 save_expire = result->cmd.prior_expire_bits;           /* Save it temporarily. */

   if (yyy->fracs.subcall_index*yyy->fracs.subcall_incr >= yyy->fracs.highlimit)
      return TRUE;

   if (yyy->fracs.subcall_index >= yyy->repetitions)
      fail("The indicated part number doesn't exist.");

   result->cmd = ss->cmd;      /* The call we wish to execute. */

   if (yyy->fracs.do_half_of_last_part != 0 &&
       yyy->fracs.subcall_index+1 == yyy->fracs.highlimit)
      result->cmd.cmd_frac_flags = yyy->fracs.do_half_of_last_part;
   else if (result->cmd.restrained_fraction) {
      result->cmd.cmd_frac_flags = result->cmd.restrained_fraction;
      result->cmd.restrained_fraction = 0;
   }
   else
      result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;  /* No fractions to constituent call. */

   /* We don't supply these; they get filled in by the call. */
   result->cmd.cmd_misc_flags &= ~DFM1_CONCENTRICITY_FLAG_MASK;
   if (!yyy->fracs.first_call) {
      result->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;
      result->cmd.cmd_assume.assumption = cr_none;
   }

   result->cmd.prior_elongation_bits = save_elongation;
   result->cmd.prior_expire_bits = save_expire;
   if (!(result->result_flags & RESULTFLAG__NO_REEVALUATE))
      update_id_bits(result);
   return FALSE;
}


static void do_call_in_series_simple(setup *result)
{
   do_call_in_series(result, FALSE, FALSE, TRUE, FALSE);
}


Private void do_concept_sequential(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   repetitionrec yyy;
   int instant_stop;

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE) {
      get_fraction_info(ss->cmd.cmd_frac_flags, 3*CFLAG1_VISIBLE_FRACTION_BIT, 2, &yyy.fracs);
      if (yyy.fracs.reverse_order) yyy.fracs.highlimit = 1-yyy.fracs.highlimit;
      instant_stop = yyy.fracs.instant_stop;
   }
   else {     /* No fractions. */
      yyy.fracs.reverse_order = FALSE;
      instant_stop = 99;
      yyy.fracs.do_half_of_last_part = 0;
      yyy.fracs.do_last_half_of_first_part = 0;
      yyy.fracs.highlimit = 2;
      yyy.fracs.subcall_index = 0;
      yyy.fracs.subcall_incr = 1;
   }

   yyy.repetitions = 2;
   yyy.fracs.first_call = yyy.fracs.reverse_order ? FALSE : TRUE;

   prepare_for_call_in_series(result, ss);

   for (;;) {
      if (do_call_under_repetition(&yyy, ss, result)) break;

      if (yyy.fracs.subcall_index != 0) result->cmd.parseptr = parseptr->subsidiary_root;
      do_call_in_series_simple(result);
      yyy.fracs.subcall_index += yyy.fracs.subcall_incr;
      yyy.fracs.first_call = FALSE;

      /* If we are being asked to do just one part of a call (from cmd_frac_flags),
         exit now.  Also, see if we just did the last part. */

      if (instant_stop != 99) {
         /* Check whether we honored the last possible request.  That is,
            whether we did the last part of the call in forward order, or
            the first part in reverse order. */
         result->result_flags |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (instant_stop >= yyy.fracs.highlimit)
            result->result_flags |= RESULTFLAG__DID_LAST_PART;
         break;
      }
   }
}



Private void do_concept_special_sequential(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (parseptr->concept->value.arg1 == 2) {
      /* This is "start with <call>", which is the same as "replace the 1st part". */

      setup tttt;
      uint32 finalresultflags = 0;

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      /* Do the special first part. */

      tttt = *ss;
      tttt.cmd = ss->cmd;
      if (!(tttt.result_flags & RESULTFLAG__NO_REEVALUATE))
         update_id_bits(&tttt);           /* So you can use "leads run", etc. */
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);

      /* Do the rest of the original call, if there is more. */

      tttt = *result;
      /* Skip over the concept. */
      tttt.cmd = ss->cmd;
      tttt.cmd.parseptr = parseptr->subsidiary_root;
      tttt.cmd.cmd_frac_flags =
         CMD_FRAC_CODE_FROMTOREV |
         (CMD_FRAC_PART_BIT*2) |
         CMD_FRAC_NULL_VALUE;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);

      result->result_flags = finalresultflags & ~3;
   }
   else {
      int call_index;

      /* We allow fractionalization commands only for the special case of "piecewise"
         or "random", in which case we will apply them to the first call only. */

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE && !(ss->cmd.cmd_misc_flags & CMD_MISC__PUT_FRAC_ON_FIRST))
         fail("Can't stack meta or fractional concepts.");

      prepare_for_call_in_series(result, ss);

      for (call_index=0; call_index<2; call_index++) {
         uint32 save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
         uint32 save_expire = result->cmd.prior_expire_bits;           /* Save it temporarily. */
         uint32 saved_last_flag = 0;
         result->cmd = ss->cmd;      /* The call we wish to execute (we will fix it up shortly). */

         if ((call_index ^ parseptr->concept->value.arg1) != 0) {   /* Maybe do them in reverse order. */
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;  /* No fractions to 2nd call. */
            saved_last_flag = result->result_flags & (RESULTFLAG__DID_LAST_PART|RESULTFLAG__PARTS_ARE_KNOWN);
         }
         else {
            result->cmd.parseptr = parseptr->subsidiary_root;
         }

         result->cmd.prior_elongation_bits = save_elongation;
         result->cmd.prior_expire_bits = save_expire;
         do_call_in_series_simple(result);
         result->result_flags |= saved_last_flag;
      }
   }
}



Private void do_concept_twice(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This includes "twice", "thrice", and "N times", which means it can either take a hard-wired
      number or a number from the current options.
      arg1 = 0 :  number of repetitions is hardwired and is in arg2.
      arg1 = 1 :  number of repetitions was specified by user. */

   repetitionrec yyy;
   int instant_stop;
   setup offline_split_info;

   /* We do not accumulate the splitting information as the calls progress.
      Instead, we accumulate it into an offline setup, and put the final
      splitting info into the result at the end. */

   prepare_for_call_in_series(&offline_split_info, ss);

   if (parseptr->concept->value.arg1)
      yyy.repetitions = parseptr->options.number_fields;
   else
      yyy.repetitions = parseptr->concept->value.arg2;

   /* If fractions come in but the craziness is restrained, just pass the fractions on.
      This is what makes "random twice mix" work.  The "random" concept wants to reach through
      the "twice" concept and do the numbered parts of the mix, not the numbered parts of the
      twiceness.  But if the craziness is unrestrained, which is the usual case, we act on
      the fractions.  This makes "interlace twice this with twice that" work. */

   get_fraction_info(ss->cmd.cmd_frac_flags,
                     3*CFLAG1_VISIBLE_FRACTION_BIT,
                     yyy.repetitions,
                     &yyy.fracs);

   if (yyy.fracs.reverse_order) yyy.fracs.highlimit = yyy.repetitions-yyy.fracs.highlimit-1;
   instant_stop = yyy.fracs.instant_stop;

   yyy.fracs.first_call = yyy.fracs.reverse_order ? FALSE : TRUE;

   *result = *ss;
   result->result_flags = 0;

   for (;;) {
      if (do_call_under_repetition(&yyy, ss, result)) break;

      /* Do *NOT* try to maintain consistent splitting across repetitions when doing "twice". */
      result->result_flags |= RESULTFLAG__SPLIT_AXIS_FIELDMASK;

      /* We do *not* remember the yoyo/twisted expiration stuff. */
      result->result_flags &= ~EXPIRATION_STATE_BITS;

      do_call_in_series_simple(result);

      /* Record the minimum in each direction. */
      minimize_splitting_info(&offline_split_info, result->result_flags);
      yyy.fracs.subcall_index += yyy.fracs.subcall_incr;
      yyy.fracs.first_call = FALSE;

      /* If we are being asked to do just one part of a call (from cmd_frac_flags),
         exit now.  Also, see if we just did the last part. */

      if (instant_stop != 99) {
         /* Check whether we honored the last possible request.  That is,
            whether we did the last part of the call in forward order, or
            the first part in reverse order. */
         result->result_flags |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (instant_stop >= yyy.fracs.highlimit)
            result->result_flags |= RESULTFLAG__DID_LAST_PART;
         break;
      }
   }

   result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
   result->result_flags |= offline_split_info.result_flags;
}


Private void do_concept_trace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i, r[4], rot[4];
   uint32 finalresultflags;
   setup a[4], res[4];
   setup outer_inners[2];
   Const veryshort *tracearray;

   static Const veryshort tracearray1[16] =
   {-1, -1, 7, 6, -1, -1, 4, 5, 3, 2, -1, -1, 0, 1, -1, -1};
   static Const veryshort tracearray2[16] =
   {0, 1, -1, -1, 6, 7, -1, -1, -1, -1, 4, 5, -1, -1, 2, 3};

   if (ss->kind != s_qtag) fail("Must have a 1/4-tag-like setup for trace.");

   ss->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   for (i=0 ; i<4 ; i++) {
      a[i] = *ss;
      clear_people(&a[i]);
      a[i].kind = s2x2;
      a[i].rotation = 0;
   }

   if ((ss->people[6].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_south) {
      a[1].cmd.parseptr = parseptr->subsidiary_root;
      a[3].cmd.parseptr = parseptr->subsidiary_root;
      tracearray = tracearray1;
   }
   else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_north) {
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
      move(&a[i], FALSE, &res[i]);
      tracearray += 4;
   }

   finalresultflags = get_multiple_parallel_resultflags(res, 4);
   clear_people(&outer_inners[1]);
   clear_people(&outer_inners[0]);

   /* Check that everyone is in a 2x2 or vertically oriented 1x4. */

   for (i=0 ; i<4 ; i++) {
      if ((res[i].kind != s2x2 && res[i].kind != nothing && (res[i].kind != s1x4 || (!(res[i].rotation&1)))))
         fail("You can't do this.");
   }

   /* Process people going into the center. */

   outer_inners[1].rotation = 0;
   outer_inners[1].result_flags = 0;

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

   /* Process people going to the outside. */

   outer_inners[0].rotation = 0;
   outer_inners[0].result_flags = 0;

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

   normalize_concentric(schema_concentric, 1, outer_inners, ((~outer_inners[0].rotation) & 1) + 1, result);
   result->result_flags = finalresultflags;
   reinstate_rotation(ss, result);
}


Private void do_concept_outeracting(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup temp;

   static Const veryshort mapo1[8] = {7, 0, 2, 5, 3, 4, 6, 1};
   static Const veryshort mapo2[8] = {7, 0, 1, 6, 3, 4, 5, 2};

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

   new_divided_setup_move(&temp, MAPCODE(s2x2,2,MPKIND__SPLIT,0), phantest_ok, TRUE, result);
}


Private void do_concept_quad_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");
   new_divided_setup_move(ss, MAPCODE(s2x2,4,parseptr->concept->value.arg1,0), phantest_ok, TRUE, result);
}


Private void do_concept_inner_outer(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 livemask, misc2_zflag;
   int i;
   calldef_schema sch;
   int rot = 0;

   switch (parseptr->concept->value.arg1 & 0x70) {
   case 0:      /* triple CLW */
   case 0x20:   /* triple twin CLW */
   case 0x30:   /* triple twin CLW of 3 */
      sch = schema_in_out_triple;
      break;
   case 0x10:
      sch = schema_in_out_quad;
      break;
   case 0x40:
      sch = schema_in_out_12mquad;
      break;
   }

   switch (parseptr->concept->value.arg1) {
   case 0: case 1: case 3:
      /* Center triple line/wave/column. */
      switch (ss->kind) {
      case s3x4: case s1x12:
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
            if (global_tbonetest & 1) fail("There are no triple lines here.");
            else                      fail("There are no triple columns here.");
         }
         goto ready;
      case sbigh: case sbigx: case sbigdmd: case sbigbone:
         switch (parseptr->concept->value.arg1 & 7) {
         case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
         case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
         }
         goto ready;
      }

      fail("Need a triple line/column setup for this.");
   case 8+0: case 8+1: case 8+3:
      /* Outside triple lines/waves/columns. */
      switch (ss->kind) {
      case s3x4: case s1x12:
         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
            if (global_tbonetest & 1) fail("There are no triple lines here.");
            else                      fail("There are no triple columns here.");
         }
         goto ready;
      case sbigh: case sbigx: case sbigrig:
         switch (parseptr->concept->value.arg1 & 7) {
         case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
         case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
         }
         goto ready;
      }

      fail("Need a triple line/column setup for this.");
   case 32+0: case 32+1: case 32+3:
   case 32+8+0: case 32+8+1: case 32+8+3:
      /* Center/outside triple twin lines/waves/columns. */
      if (ss->kind != s4x6) fail("Need a 4x6 setup for this.");

      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if ((parseptr->concept->value.arg1 ^ global_tbonetest) & 1) {
         if (global_tbonetest & 1) fail("There are no triple twin lines here.");
         else                      fail("There are no triple twin columns here.");
      }

      goto ready;
   case 48+0: case 48+1: case 48+3:
   case 48+8+0: case 48+8+1: case 48+8+3:
      /* Center/outside triple twin lines/waves/columns of 3. */
      if (ss->kind != s3x6) fail("Need a 4x6 setup for this.");

      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if ((parseptr->concept->value.arg1 ^ global_tbonetest) & 1) {
         if (global_tbonetest & 1) fail("There are no triple twin lines of 3 here.");
         else                      fail("There are no triple twin columns of 3here.");
      }

      goto ready;
   case 16+0: case 16+1: case 16+3:
   case 16+8+0: case 16+8+1: case 16+8+3:
      /* Center or outside phantom lines/waves/columns. */

      if (ss->kind == s4x4) {
         uint32 tbone = global_tbonetest;
         /* If everyone is consistent (or "standard" was used to make it appear so),
            it's easy. */
         if ((tbone & 011) == 011) {
            /* If not, we need to look carefully. */

            tbone =
               ss->people[1].id1 | ss->people[2].id1 |
               ss->people[5].id1 | ss->people[6].id1 |
               ss->people[9].id1 | ss->people[10].id1 |
               ss->people[13].id1 | ss->people[14].id1;

            if (parseptr->concept->value.arg1 & 8) {
               /* This is outside phantom C/L/W. */
               tbone |=
                  ss->people[0].id1 | ss->people[4].id1 |
                  ss->people[8].id1 | ss->people[12].id1;
            }
            else {
               /* This is center phantom C/L/W. */
               tbone |=
                  ss->people[3].id1 | ss->people[7].id1 |
                  ss->people[11].id1 | ss->people[15].id1;
            }
            if ((tbone & 011) == 011) fail("Can't do this from T-bone setup.");
         }

         rot = (tbone ^ parseptr->concept->value.arg1 ^ 1) & 1;

         ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
         canonicalize_rotation(ss);
      }
      else if (ss->kind == s1x16 || ss->kind == sbigbigh || ss->kind == sbigbigx) {
         /* Shouldn't we do this for 4x4 also?  It seems that one ought to say
            "standard in outside phantom 1x4's" rather than "... in lines"
            if they are T-boned.  There is something in t34t that would break, however. */
         switch (parseptr->concept->value.arg1 & 7) {
         case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
         case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
         }
      }
      else
         fail("Need quadruple 1x4's for this.");

      break;
   case 64+0: case 64+1:
   case 64+8+0: case 64+8+1:
      /* Center or outside phantom lines/columns (no wave). */

      if (ss->kind == s3x4 || ss->kind == s_d3x4 || ss->kind == s3dmd) {
         switch (parseptr->concept->value.arg1 & 7) {
         case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
         case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
         }
      }
      else
         fail("Need quadruple 1x3's for this.");

      break;
   case 4:
      /* Center triple box. */
      switch (ss->kind) {
      case s2x6: case sbigrig:
         goto ready;
      case s4x4:
         if (ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1) {
            /* The outer lines/columns are to the left and right.  That's not canonical.
               We want them at top and bottom.  So we flip the setup. */
            rot = 1;
            ss->rotation++;
            canonicalize_rotation(ss);
         }

         /* Now the people had better be clear from the side centers. */

         if (!(ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1))
            goto ready;
      }

      fail("Need center triple box for this.");
   case 8+4:
      /* Outside triple boxes. */
      switch (ss->kind) {
      case s2x6: case sbigbone: case sbigdmd:
      case sbighrgl: case sbigdhrgl:
         goto ready;
      }

      fail("Need outer triple boxes for this.");
   case 16+4:
   case 16+8+4:
      /* Center or outside quadruple boxes. */
      if (ss->kind != s2x8) fail("Need a 2x8 setup for this.");
      goto ready;
   case 5:
      /* Center triple diamond. */
      switch (ss->kind) {
      case s3dmd: case s3ptpd: case s_3mdmd: case s_3mptpd: case s3x1dmd: case s1x3dmd:
      case s_hrglass: case s_dhrglass: case sbighrgl: case sbigdhrgl:
         goto ready;
      }

      fail("Need center triple diamond for this.");
   case 8+5:
      /* Outside triple diamonds. */
      switch (ss->kind) {
      case s3dmd: case s3ptpd: case s_3mdmd: case s_3mptpd: case s3x1dmd: case s1x3dmd:
         goto ready;
      }

      fail("Need outer triple diamonds for this.");
   case 16+5:
   case 16+8+5:
      /* Center or outside quadruple diamonds. */
      if (ss->kind != s4dmd &&
          ss->kind != s4ptpd &&
          ss->kind != s_4mptpd &&
          ss->kind != s_4mdmd) fail("Need quadruple diamonds for this.");
      ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg3;
      break;
   case 6:
   case 8+6:
      /* Center/outside triple Z's. */

      sch = schema_in_out_triple_zcom;

      for (i=0,livemask=0 ; i<=setup_attrs[ss->kind].setup_limits ; i++) {
         livemask <<= 1;
         if (ss->people[i].id1) livemask |= 1;
      }

      switch (ss->kind) {
      case s3x6:
         if ((livemask & 0630630) == 0) {
            /* Of course, this is kind of stupid.  Why would you say "outer
               triple Z's" if the outer Z's weren't recognizeable, and you
               simply wanted them to be imputed from the center Z? */
            if (parseptr->concept->value.arg1 & 8)
               warn(warn_same_z_shear);  /* Outer Z's are ambiguous --
                                            make them look like inner ones. */
         }

         /* Demand that the center Z be solidly filled. */

         switch (livemask & 0141141) {
         case 0101101:   /* Center Z is CW. */
            if ((livemask & 0210210) == 0) {
               misc2_zflag = CMD_MISC2__IN_Z_CW;
               goto do_real_z_stuff;
            }
            else if ((livemask & 0420420) == 0) {
               misc2_zflag = CMD_MISC2__IN_AZ_CW;   /* Z's are anisotropic. */
               goto do_real_z_stuff;
            }

            break;
         case 0041041:   /* Center Z is CCW. */
            if ((livemask & 0420420) == 0) {
               misc2_zflag = CMD_MISC2__IN_Z_CCW;
               goto do_real_z_stuff;
            }
            else if ((livemask & 0210210) == 0) {
               misc2_zflag = CMD_MISC2__IN_AZ_CCW;   /* Z's are anisotropic. */
               goto do_real_z_stuff;
            }

            break;
         }

         fail("Can't find the indicated formation.");
      default:
         fail("Must have 3x6 for this concept.");
      }
      break;
   }

 ready:

   if ((parseptr->concept->value.arg1 & 7) == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (parseptr->concept->value.arg1 & 8)
      concentric_move(ss, &ss->cmd, (setup_command *) 0, sch, 0, 0, TRUE, result);
   else
      concentric_move(ss, (setup_command *) 0, &ss->cmd, sch, 0, 0, TRUE, result);

   result->rotation -= rot;   /* Flip the setup back. */
   return;

 do_real_z_stuff:

   ss->cmd.cmd_misc2_flags |= misc2_zflag;
   goto ready;
}


Private void do_concept_do_both_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind == s2x4)
      divided_setup_move(ss, parseptr->concept->value.maps, phantest_ok, TRUE, result);
   else if (ss->kind == s3x4 && parseptr->concept->value.arg2)
      /* distorted_2x2s_move will notice that concept is funny and will do the right thing. */
      distorted_2x2s_move(ss, parseptr, result);
   else
      fail("Need a 2x4 setup to do this concept.");
}


Private void do_concept_do_each_1x4(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{

   map_thing *division_maps;
   uint32 map_code = ~0UL;
   int arg1 = parseptr->concept->value.arg1;
   int arg2 = parseptr->concept->value.arg2;

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
               division_maps = &map_d1x10;
               goto split_big;
            }
            break;
      }

      fail("Need a 2x4 or 1x8 setup for this concept.");
   }

   split_small:

   (void) do_simple_split(ss, (arg2 != 2), result);
   return;

   split_big:

   if (map_code == ~0UL)
      divided_setup_move(ss, division_maps, phantest_ok, TRUE, result);
   else
      new_divided_setup_move(ss, map_code, phantest_ok, TRUE, result);
}


Private void do_concept_triple_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x6) fail("Must have a 2x6 setup for this concept.");

   if ((ss->cmd.cmd_misc2_flags & CMD_MISC2__MYSTIFY_SPLIT) && parseptr->concept->value.arg1 != MPKIND__SPLIT)
      fail("Mystic not allowed with this concept.");

   new_divided_setup_move(ss, MAPCODE(s2x2,3,parseptr->concept->value.arg1,0), phantest_ok, TRUE, result);
}


Private void do_concept_centers_or_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   selective_move(ss, parseptr, selective_key_plain, 0, 0, 0,
                  (selector_kind) parseptr->concept->value.arg1,
                  parseptr->concept->value.arg2, result);
}



Private void do_concept_centers_and_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   selective_move(ss, parseptr, selective_key_plain, 1, 0, 0,
                  (selector_kind) parseptr->concept->value.arg1,
                  parseptr->concept->value.arg2, result);
}



Private void so_and_so_only_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   selective_move(ss, parseptr, (selective_key) parseptr->concept->value.arg1, parseptr->concept->value.arg2, parseptr->concept->value.arg3, 0, parseptr->options.who, FALSE, result);
}



Private void do_concept_triple_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 code;

   /* See "do_concept_do_phantom_diamonds" for meaning of arg2. */

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

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   new_divided_setup_move(ss, code, phantest_ok, TRUE, result);
}


Private void do_concept_quad_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 code;

   /* See "do_concept_do_phantom_diamonds" for meaning of arg2. */

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

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   new_divided_setup_move(ss, code, phantest_ok, TRUE, result);
}


Private void do_concept_ferris(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup temp;

   static Const veryshort mapr1[8] = {1, 4, 6, 5, 7, 10, 0, 11};
   static Const veryshort mapr2[8] = {10, 2, 3, 5, 4, 8, 9, 11};
   static Const veryshort mapf1[8] = {0, 1, 5, 4, 6, 7, 11, 10};
   static Const veryshort mapf2[8] = {10, 11, 2, 3, 4, 5, 8, 9};

   temp = *ss;
   temp.kind = s3x4;
   clear_people(&temp);

   if (parseptr->concept->value.arg1) {
      /* This is "release". */

      if ((ss->kind != s_qtag) || ((global_tbonetest & 1) != 0))
         fail("Must have quarter-tag to do this concept.");

      if (((
               (ss->people[1].id1 ^ d_south) |
               (ss->people[2].id1 ^ d_south) |
               (ss->people[5].id1 ^ d_north) |
               (ss->people[6].id1 ^ d_north)
            ) & d_mask) == 0) {
         scatter(&temp, ss, mapr1, 7, 0);
      }
      else if (((
               (ss->people[0].id1 ^ d_south) |
               (ss->people[2].id1 ^ d_north) |
               (ss->people[4].id1 ^ d_north) |
               (ss->people[6].id1 ^ d_south)
            ) & d_mask) == 0) {
         scatter(&temp, ss, mapr2, 7, 0);
      }
      else
         fail("Incorrect facing directions.");
   }
   else {
      /* This is "ferris". */

      if ((ss->kind != s2x4) || ((global_tbonetest & 1) != 0))
         fail("Must have lines to do this concept.");

      if (((
               (ss->people[0].id1 ^ d_north) |
               (ss->people[1].id1 ^ d_north) |
               (ss->people[2].id1 ^ d_south) |
               (ss->people[3].id1 ^ d_south) |
               (ss->people[4].id1 ^ d_south) |
               (ss->people[5].id1 ^ d_south) |
               (ss->people[6].id1 ^ d_north) |
               (ss->people[7].id1 ^ d_north)
            ) & d_mask) == 0) {
         scatter(&temp, ss, mapf1, 7, 0);
      }
      else if (((
               (ss->people[0].id1 ^ d_south) |
               (ss->people[1].id1 ^ d_south) |
               (ss->people[2].id1 ^ d_north) |
               (ss->people[3].id1 ^ d_north) |
               (ss->people[4].id1 ^ d_north) |
               (ss->people[5].id1 ^ d_north) |
               (ss->people[6].id1 ^ d_south) |
               (ss->people[7].id1 ^ d_south)
            ) & d_mask) == 0) {
         scatter(&temp, ss, mapf2, 7, 0);
      }
      else
         fail("Incorrect facing directions.");
   }

   new_divided_setup_move(&temp, MAPCODE(s1x4,3,MPKIND__SPLIT,1), phantest_ok, TRUE, result);

   /* Squash phantoms if they are properly placed. */

   if (result->kind == s2x6) {
      if (!(result->people[0].id1 | result->people[5].id1 | result->people[6].id1 | result->people[11].id1)) {
         temp = *result;
         clear_people(result);
         (void) copy_person(result, 0, &temp, 1);
         (void) copy_person(result, 3, &temp, 4);
         (void) copy_person(result, 4, &temp, 7);
         (void) copy_person(result, 7, &temp, 10);
      }
      else if (!(result->people[1].id1 | result->people[4].id1 | result->people[7].id1 | result->people[10].id1)) {
         temp = *result;
         clear_people(result);
         (void) copy_person(result, 0, &temp, 0);
         (void) copy_person(result, 3, &temp, 5);
         (void) copy_person(result, 4, &temp, 6);
         (void) copy_person(result, 7, &temp, 11);
      }
      else
         return;

      (void) copy_person(result, 1, &temp, 2);
      (void) copy_person(result, 2, &temp, 3);
      (void) copy_person(result, 5, &temp, 8);
      (void) copy_person(result, 6, &temp, 9);

      result->kind = s2x4;
   }
}


Private void do_concept_overlapped_diamond(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 mapcode;
   expand_thing *scatterlist;
   static expand_thing list1x4    = {{0, 1, 4, 5}, 4, s1x4, s_thar, 0};
   static expand_thing list1x4rot = {{2, 3, 6, 7}, 4, s1x4, s_thar, 3};
   static expand_thing listdmd    = {{0, 3, 4, 7}, 4, sdmd, s_thar, 0};
   static expand_thing listdmdrot = {{2, 5, 6, 1}, 4, sdmd, s_thar, 3};

   /* Split an 8 person setup. */
   if (setup_attrs[ss->kind].setup_limits == 7) {
      /* Reset it to execute this same concept again, until it doesn't have to split any more. */
      ss->cmd.parseptr = parseptr;
      if (do_simple_split(ss, TRUE, result))
         fail("Not in correct setup for overlapped diamond/line concept.");
      return;
   }

   switch (ss->kind) {
   case s1x4:
      if (parseptr->concept->value.arg1 & 1)
         fail("Must be in a diamond.");

      scatterlist = &list1x4;
      mapcode = MAPCODE(sdmd,2,MPKIND__DMD_STUFF,0);
      break;
   case sdmd:
      if (!(parseptr->concept->value.arg1 & 1))
         fail("Must be in a line.");
      if (parseptr->concept->value.arg1 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      scatterlist = &listdmd;
      mapcode = MAPCODE(s1x4,2,MPKIND__DMD_STUFF,0);
      break;
   default:
      fail("Not in correct setup for overlapped diamond/line concept.");
   }

   expand_setup(scatterlist, ss);
   new_divided_setup_move(ss, mapcode, phantest_ok, TRUE, result);

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

   compress_setup(scatterlist, result);
}



Private void do_concept_all_8(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   static Const veryshort expander[8] = {10, 13, 14, 1, 2, 5, 6, 9};

   int key = parseptr->concept->value.arg1;

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

      new_divided_setup_move(ss, MAPCODE(s2x2,2,MPKIND__ALL_8,0), phantest_ok, TRUE, result);
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
         /* Either one is legal in a thar. */
         if (key == 1)
            new_divided_setup_move(ss, MAPCODE(s1x4,2,MPKIND__ALL_8,0), phantest_ok, TRUE, result);
         else
            new_divided_setup_move(ss, MAPCODE(sdmd,2,MPKIND__ALL_8,0), phantest_ok, TRUE, result);

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
         new_divided_setup_move(ss, MAPCODE(s1x4,2,MPKIND__ALL_8,0), phantest_ok, TRUE, result);
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

#ifdef BULLSHIT
         if (  (tl & 010) &&
               (  (ss->people[ 1].id1 != 0 && ((ss->people[ 1].id1 ^ d_south) & d_mask) != 0) ||
                  (ss->people[ 2].id1 != 0 && ((ss->people[ 2].id1 ^ d_north) & d_mask) != 0) ||
                  (ss->people[ 9].id1 != 0 && ((ss->people[ 9].id1 ^ d_north) & d_mask) != 0) ||
                  (ss->people[10].id1 != 0 && ((ss->people[10].id1 ^ d_south) & d_mask) != 0) ||
                  (ss->people[ 5].id1 != 0 && ((ss->people[ 5].id1 ^ d_west) & d_mask) != 0) ||
                  (ss->people[ 6].id1 != 0 && ((ss->people[ 6].id1 ^ d_east) & d_mask) != 0) ||
                  (ss->people[13].id1 != 0 && ((ss->people[13].id1 ^ d_east) & d_mask) != 0) ||
                  (ss->people[14].id1 != 0 && ((ss->people[14].id1 ^ d_west) & d_mask) != 0)))
            fail("Must not be directly back-to-back.");
#endif

         new_divided_setup_move(ss, MAPCODE(s2x2,2,MPKIND__ALL_8,0), phantest_ok, TRUE, result);
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






Private void do_concept_meta(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   parse_block *parseptr_skip;
   parse_block fudgyblock;
   setup_command nocmd, yescmd;
   uint32 expirations_to_clear = 0;
   meta_key_kind key = (meta_key_kind) parseptr->concept->value.arg1;

   prepare_for_call_in_series(result, ss);

   if ((key == meta_key_random || key == meta_key_echo) &&
       (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_REVERSE))) {
      /* "reverse" and "random"  ==>  "reverse random" */
      key = (meta_key_kind) ((int) key+1);
      ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_REVERSE;
   }

   /* Now demand that no flags remain. */
   if (ss->cmd.cmd_final_flags.final)
      fail("Illegal modifier for this concept.");

   nocmd = ss->cmd;
   yescmd = ss->cmd;

   if (key != meta_key_initially && key != meta_key_finally &&
       key != meta_key_piecewise && key != meta_key_nth_part_work &&
       key != meta_key_echo && key != meta_key_rev_echo)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;   /* We didn't do this before. */

   if (key == meta_key_finish) {

      /* This is "finish".  Do the call after the first part, with the special indicator
         saying that this was invoked with "finish", so that the flag will be checked. */

      if (ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE)
         ss->cmd.cmd_frac_flags =
            CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) ==
               ( CMD_FRAC_REVERSE |                                            CMD_FRAC_NULL_VALUE))
         ss->cmd.cmd_frac_flags += CMD_FRAC_CODE_FROMTOREV + CMD_FRAC_PART_BIT*1 + CMD_FRAC_PART2_BIT*1;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK      | 0xFFFF)) ==
               (                0 | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE))
         /* If we are already doing just parts N and later, just bump N. */
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK      | 0xFFFF)) ==
               ( CMD_FRAC_REVERSE | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE))
         /* If we are already doing just parts N and later while reversed, just bump K. */
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART2_BIT;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK   | 0xFFFF)) ==
               (                    CMD_FRAC_CODE_FROMTO   | CMD_FRAC_NULL_VALUE))
         ss->cmd.cmd_frac_flags +=
            CMD_FRAC_PART_BIT+CMD_FRAC_PART2_BIT;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
               (                    CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE))
         /* If we are already doing just part N only, just bump N. */
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK    | 0xFFFF)) ==
               (                    CMD_FRAC_CODE_ONLYREV | CMD_FRAC_NULL_VALUE))
         /* If we are already doing just part N only in reverse order, do nothing. */
         ;
      else if ((ss->cmd.cmd_frac_flags &
                (CMD_FRAC_BREAKING_UP | CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK      | CMD_FRAC_PART_MASK | 0xFFFF)) ==
               ( CMD_FRAC_BREAKING_UP |                    CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_PART_BIT  | CMD_FRAC_NULL_VALUE))
         /* If we are already doing up to some part, just "finish" that. */
         ss->cmd.cmd_frac_flags += CMD_FRAC_PART_BIT;
      else
         fail("Can't stack meta or fractional concepts.");

      move(ss, FALSE, result);
      normalize_setup(result, simple_normalize);
      return;
   }
   else if (key == meta_key_revorder) {
      /* This is "reverse order". */

      ss->cmd.cmd_frac_flags ^= CMD_FRAC_REVERSE;
      move(ss, FALSE, result);
      return;
   }
   else if (key == meta_key_like_a) {
      /* This is "like a".  Do the last part of the call. */

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      ss->cmd.cmd_frac_flags = CMD_FRAC_REVERSE | CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE;
      move(ss, FALSE, result);
      normalize_setup(result, simple_normalize);
      return;
   }

   if (key != meta_key_skip_nth_part &&
       key != meta_key_shift_n && key != meta_key_shifty &&
       key != meta_key_shift_half && key != meta_key_shift_n_half) {
      concept_kind k;
      uint32 need_to_restrain;
      parse_block *parseptrcopy;

      /* Scan the modifiers, remembering them and their end point.  The reason for this is to
         avoid getting screwed up by a comment, which counts as a modifier.  YUK!!!!!!
         This code used to have the beginnings of stuff to do it really right.  It isn't
         worth it, and isn't worth holding up "random left" for.  In any case, the stupid
         handling of comments will go away soon. */
   
      parseptrcopy = really_skip_one_concept(parseptr->next, &k,
                                             &need_to_restrain, &parseptr_skip);
      yescmd.parseptr = parseptrcopy;

      // If the skipped concept is "twisted" or "yoyo", get ready to clear
      // the expiration bit for same, if we do it "piecewise" or whatever.

      if (k == concept_yoyo)
         expirations_to_clear = RESULTFLAG__YOYO_FINISHED;
      if (k == concept_twisted)
         expirations_to_clear = RESULTFLAG__TWISTED_FINISHED;

      if (concept_table[k].concept_prop & CONCPROP__SECOND_CALL)
         nocmd.parseptr = parseptrcopy->subsidiary_root;
      else
         nocmd.parseptr = parseptr_skip;

      if ((need_to_restrain & 2) ||
          ((need_to_restrain & 1) &&
           (key != meta_key_rev_echo && key != meta_key_echo))) {
         yescmd.cmd_misc_flags |= CMD_MISC__RESTRAIN_CRAZINESS;
         yescmd.restrained_concept = &fudgyblock;
         yescmd.parseptr = parseptr_skip;
         fudgyblock = *parseptrcopy;
      }
   }

   switch (key) {
      uint32 shiftynum;
      uint32 frac_flags;
      uint32 save_elongation;
      uint32 save_expire;
      uint32 index;
      uint32 shortenhighlim;
      uint32 code_to_use_for_only;
      long_boolean doing_just_one;

   case meta_key_skip_nth_part:

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");
   
      /* Do the initial part, if any. */

      if (parseptr->options.number_fields > 1) {
         /* Set the fractionalize field to do the first few parts of the call. */
         result->cmd.cmd_frac_flags =
            ((parseptr->options.number_fields-1) * CMD_FRAC_PART_BIT) |
            CMD_FRAC_NULL_VALUE | CMD_FRAC_CODE_FROMTO;

         do_call_in_series_simple(result);
         result->cmd = ss->cmd;
      }

      /* Do the final part. */
      result->cmd.cmd_frac_flags =
         ((parseptr->options.number_fields+1) * CMD_FRAC_PART_BIT) |
         CMD_FRAC_NULL_VALUE | CMD_FRAC_CODE_FROMTOREV;
      goto do_less;

   case meta_key_shift_n:
   case meta_key_shifty:
   case meta_key_shift_half:
   case meta_key_shift_n_half:

      /* Some form of shift <N>. */

      shiftynum = 1;

      if (key == meta_key_shift_n || key == meta_key_shift_n_half) {
         shiftynum = parseptr->options.number_fields;
         if (key == meta_key_shift_n_half) shiftynum++;
      }

      if (key == meta_key_shift_half || key == meta_key_shift_n_half) {
         if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
            fail("Fractional shift doesn't have parts.");

         /* Do the last (shifted) part. */

         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP |
            CMD_FRAC_CODE_LATEFROMTOREV |
            (shiftynum * CMD_FRAC_PART_BIT) |
            CMD_FRAC_NULL_VALUE;

         do_call_in_series_simple(result);
         result->cmd = ss->cmd;

         /* Do the initial part up to the shift point. */

         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOMOST |
            (shiftynum * CMD_FRAC_PART_BIT) | CMD_FRAC_NULL_VALUE;
      }
      else {         // Not fractional shift.
         if (ss->cmd.cmd_frac_flags == (CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE)) {
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
               CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_REVERSE |
               (shiftynum * CMD_FRAC_PART2_BIT) | CMD_FRAC_PART_BIT | CMD_FRAC_NULL_VALUE;
         }
         else if ((ss->cmd.cmd_frac_flags & ~(CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK)) ==
                  (CMD_FRAC_BREAKING_UP |
                   CMD_FRAC_CODE_FROMTOREV |
                   CMD_FRAC_NULL_VALUE)) {
            result->cmd.cmd_frac_flags =
               CMD_FRAC_BREAKING_UP |
               CMD_FRAC_CODE_FROMTOREV |
               ((shiftynum * CMD_FRAC_PART_BIT) +
                (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK)) |
               CMD_FRAC_NULL_VALUE;

            uint32 kfield = (ss->cmd.cmd_frac_flags & CMD_FRAC_PART2_MASK) / CMD_FRAC_PART2_BIT;

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
               CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTO |
               ((shiftynum - kfield) * CMD_FRAC_PART_BIT) |
               CMD_FRAC_NULL_VALUE;
         }
         else if ((ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
                  (CMD_FRAC_BREAKING_UP |
                   CMD_FRAC_CODE_ONLYREV |
                   CMD_FRAC_NULL_VALUE)) {
            if ((shiftynum * CMD_FRAC_PART_BIT) <
                (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK))
               fail("Can't stack these meta or fractional concepts.");
            result->cmd.cmd_frac_flags =
               CMD_FRAC_BREAKING_UP |
               CMD_FRAC_CODE_ONLY |
               (((shiftynum+1) * CMD_FRAC_PART_BIT) -
                (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK)) |
               CMD_FRAC_NULL_VALUE;
         }
         else if ((ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
                  (CMD_FRAC_BREAKING_UP |
                   CMD_FRAC_CODE_ONLY |
                   CMD_FRAC_NULL_VALUE)) {
            /*
            if ((shiftynum * CMD_FRAC_PART_BIT) <
                (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK))
               fail("Can't stack these meta or fractional concepts.");
            */
            result->cmd.cmd_frac_flags =
               CMD_FRAC_BREAKING_UP |
               CMD_FRAC_CODE_ONLY |
               ((shiftynum * CMD_FRAC_PART_BIT) +
                (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK)) |
               CMD_FRAC_NULL_VALUE;
         }
         else if (ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE) {
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

      if (allowing_all_concepts | 1) {
         if (!(yescmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS)) {
            if (result->cmd.cmd_frac_flags == (CMD_FRAC_HALF_VALUE)) {
               result->cmd = yescmd;
               goto do_less;
            }
            else if (result->cmd.cmd_frac_flags == (CMD_FRAC_LASTHALF_VALUE)) {
               result->cmd = nocmd;
               goto do_less;
            }
            else if (result->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
               fail("Can't stack meta or fractional concepts this way.");
         }
      }

      /* Do the call with the concept. */
      result->cmd = yescmd;
      do_call_in_series_simple(result);

      /* And then again without it. */
      result->cmd = nocmd;

      if (!(result->result_flags & RESULTFLAG__NO_REEVALUATE))
         update_id_bits(result);

      /* Assumptions don't carry through. */
      result->cmd.cmd_assume.assumption = cr_none;
      goto do_less;

   case meta_key_rev_echo:

      /* Do the call without the concept. */
      result->cmd = nocmd;
      do_call_in_series_simple(result);

      /* And then again with it. */
      result->cmd = yescmd;

      /* Assumptions don't carry through. */
      result->cmd.cmd_assume.assumption = cr_none;
      goto do_less;

   case meta_key_nth_part_work:

      /* This is "do the Nth part <concept>". */

      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      /* Do the initial part, if any, without the concept. */

      if (parseptr->options.number_fields > 1) {
         result->cmd = nocmd;
         /* Set the fractionalize field to do the first few parts of the call. */
         result->cmd.cmd_frac_flags =
            ((parseptr->options.number_fields-1) * CMD_FRAC_PART_BIT) |
            CMD_FRAC_CODE_FROMTO | CMD_FRAC_NULL_VALUE;
         do_call_in_series_simple(result);
      }

      /* Do the part of the call that needs the concept. */

      result->cmd = yescmd;
      result->cmd.cmd_frac_flags =
         (parseptr->options.number_fields * CMD_FRAC_PART_BIT) |
         CMD_FRAC_NULL_VALUE | CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY;

      do_call_in_series_simple(result);

      if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
         fail("Can't have 'no one' do a call.");

      /* Do the final part, if there is more. */
   
      if (!(result->result_flags & RESULTFLAG__DID_LAST_PART)) {
         result->cmd = ss->cmd;
         result->cmd.parseptr = parseptr_skip;      /* Skip over the concept. */
         result->cmd.cmd_frac_flags =
            ((parseptr->options.number_fields+1) * CMD_FRAC_PART_BIT) |
            CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE;
         goto do_less;
      }

      goto get_out;

   case meta_key_initially:

      /* This is "initially": we select the first part with the concept,
         and then the rest of the call without the concept. */

      if (ss->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY |
                                     CMD_FRAC_PART_BIT | CMD_FRAC_NULL_VALUE)) {
         /* We are being asked to do just the first part, because of another
            "initially".  Just pass it through. */

         result->cmd = yescmd;
      }
      else if (
               /* Being asked to do all but the first part. */
               (ss->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
                                          CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE)) ||
               /* Being asked to do some specific part other than the first. */
               ((ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
                (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE) &&
                (ss->cmd.cmd_frac_flags &  CMD_FRAC_PART_MASK) >= (CMD_FRAC_PART_BIT*2)) ||
               /* Being asked to do only the last part -- it's a safe bet that
                  that isn't the first part. */
               ss->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
                                          CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         /* In any case, just pass it through. */
         result->cmd.parseptr = parseptr_skip;
      }
      else if (ss->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP |
                                          CMD_FRAC_CODE_FROMTOREV |
                                          CMD_FRAC_PART_BIT*1 |
                                          CMD_FRAC_PART2_BIT*1 |
                                          CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do all but the last part.  Do the first part with the concept,
            then all but first and last without it. */

         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP |
            CMD_FRAC_CODE_ONLY |
            CMD_FRAC_PART_BIT*1 |
            CMD_FRAC_NULL_VALUE;

         do_call_in_series_simple(result);
         result->cmd = nocmd;
         /* Assumptions don't carry through. */
         result->cmd.cmd_assume.assumption = cr_none;
         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP |
            CMD_FRAC_CODE_FROMTOREV |
            CMD_FRAC_PART_BIT*2 |
            CMD_FRAC_PART2_BIT*1 |
            CMD_FRAC_NULL_VALUE;
      }
      else if (ss->cmd.cmd_frac_flags ==
               (CMD_FRAC_BREAKING_UP |
                CMD_FRAC_CODE_FROMTOREV |
                CMD_FRAC_PART_BIT*2 |
                CMD_FRAC_PART2_BIT*1 |
                CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do just the inner parts, presumably because of an
            "initially" and "finally".  Just pass it through. */
         result->cmd = nocmd;
      }
      else if ((ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
               (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTO | CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do an initial subset that includes the first part.
            Do the first part, then do the rest of the subset. */

         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP |
            CMD_FRAC_CODE_ONLY |
            CMD_FRAC_PART_BIT*1 |
            CMD_FRAC_NULL_VALUE;

         /* The first part, with the concept. */
         do_call_in_series_simple(result);
         result->cmd = nocmd;
         /* Assumptions don't carry through. */
         result->cmd.cmd_assume.assumption = cr_none;
         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP |
            CMD_FRAC_CODE_FROMTO |
            (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) |    /* Incoming end point. */
            CMD_FRAC_PART2_BIT |                 /* Skip one at start. */
            CMD_FRAC_NULL_VALUE;
      }
      else if (ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE) {
         /* Do the first part with the concept. */
         result->cmd = yescmd;
         result->cmd.cmd_frac_flags =
            CMD_FRAC_BREAKING_UP |
            CMD_FRAC_CODE_ONLY |
            CMD_FRAC_PART_BIT*1 |
            CMD_FRAC_NULL_VALUE;

         do_call_in_series_simple(result);

         /* And the rest of the call without it. */

         result->cmd = nocmd;
         result->cmd.cmd_assume.assumption = cr_none;  /* Assumptions don't carry through. */
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
            CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE;
      }
      else
         fail("Can't stack meta or fractional concepts.");

      goto do_less;

   case meta_key_finally:

      /* This is "finally": we select all but the last part without the concept,
         and then the last part with the concept. */

      if (result->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
                                         CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do just the last part, because of another "finally".
            Just pass it through. */

         result->cmd = yescmd;
      }
      else if (result->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
                                              CMD_FRAC_PART_BIT*1 | CMD_FRAC_PART2_BIT*1 |
                                              CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do all but the last part, because of another "finally".
            Just pass it through. */

         result->cmd = nocmd;
      }
      else if (result->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY |
                                              CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do just the first part, presumably because of an "initially".
            Just pass it through. */

         result->cmd = nocmd;
      }
      else if (result->cmd.cmd_frac_flags ==
               (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
                CMD_FRAC_PART_BIT*2 | CMD_FRAC_PART2_BIT*1 | CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do just the inner parts, presumably because of
            an "initially" and "finally".  Just pass it through. */

         result->cmd = nocmd;
      }
      else if (ss->cmd.cmd_frac_flags == (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
                                          CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE)) {

         /* We are being asked to do all but the first part, presumably because of
            an "initially".  Do all but first and last normally, then do last
            with the concept. */

         /* Change it to do all but first and last. */

         result->cmd = nocmd;
         result->cmd.cmd_frac_flags += CMD_FRAC_PART2_BIT;
         do_call_in_series_simple(result);
         result->cmd = yescmd;
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
            CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE;
         /* Assumptions don't carry through. */
         result->cmd.cmd_assume.assumption = cr_none;
      }
      else if (ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE) {

         /* Do the call without the concept. */
         /* Set the fractionalize field to execute all but the last part of the call. */

         result->cmd = nocmd;
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
            CMD_FRAC_PART_BIT*1 | CMD_FRAC_PART2_BIT*1 | CMD_FRAC_NULL_VALUE;
         do_call_in_series_simple(result);
         result->cmd = ss->cmd;
         /* Do the call with the concept. */
         /* Set the fractionalize field to execute the last part of the call. */
         result->cmd = yescmd;
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV |
            CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE;
         /* Assumptions don't carry through. */
         result->cmd.cmd_assume.assumption = cr_none;
      }
      else
         fail("Can't stack meta or fractional concepts.");

      goto do_less;

   default:

      /* Otherwise, this is the "random", "reverse random", or "piecewise" concept.
         Repeatedly execute parts of the call, skipping the concept where required. */

      index = 0;
      shortenhighlim = 0;
      doing_just_one = FALSE;
      code_to_use_for_only = CMD_FRAC_CODE_ONLY;

      /* We allow picking a specific part, and we allow "finishing" from a specific
         part, but we allow nothing else. */

      if ((ss->cmd.cmd_frac_flags &
           (CMD_FRAC_BREAKING_UP | CMD_FRAC_IMPROPER_BIT |
            CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) ==
          (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY | CMD_FRAC_NULL_VALUE)) {
         index = ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT) - 1;
         doing_just_one = TRUE;
      }
      else if ((ss->cmd.cmd_frac_flags &
           (CMD_FRAC_BREAKING_UP | CMD_FRAC_IMPROPER_BIT |
            CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | CMD_FRAC_PART2_MASK | 0xFFFF)) ==
          (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLYREV | CMD_FRAC_NULL_VALUE)) {
         index = ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT) - 1;
         doing_just_one = TRUE;
         code_to_use_for_only = CMD_FRAC_CODE_ONLYREV;
         fail("Sorry, can't do this.");
      }
      else if ((ss->cmd.cmd_frac_flags &
           (CMD_FRAC_BREAKING_UP | CMD_FRAC_IMPROPER_BIT |
            CMD_FRAC_REVERSE | CMD_FRAC_CODE_MASK | 0xFFFF)) ==
          (CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_NULL_VALUE)) {
         index = ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT) - 1;
         shortenhighlim = ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART2_MASK));
      }
      else if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't stack meta or fractional concepts.");

      frac_flags = ss->cmd.cmd_frac_flags &
         ~(CMD_FRAC_CODE_MASK|CMD_FRAC_PART_MASK|CMD_FRAC_PART2_MASK);

      do {
         /* Here is where we make use of actual numerical assignments. */
         uint32 revrand = key-meta_key_random;

         index++;
         save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
         save_expire = result->cmd.prior_expire_bits;           /* Save it temporarily. */
         result->cmd = ss->cmd;
         result->cmd.prior_elongation_bits = save_elongation;
         result->cmd.prior_expire_bits = save_expire;

         /* If concept is "[reverse] random" and this is an even/odd-numbered part,
            as the case may be, skip over the concept. */
         if (((revrand & ~1) == 0) && ((index & 1) == revrand)) {
            /* But how do we skip the concept?  If it an ordinary single-call concept,
               it's easy.  But, if the concept takes a second call (the only legal case
               of this being "concept_special_sequential") we use its second subject call
               instead of the first.  This is part of the peculiar behavior of this
               particular combination. */

            result->cmd = nocmd;
         }
         else {
            result->cmd = yescmd;
            result->result_flags &= ~expirations_to_clear;
         }

         /* Set the fractionalize field to do the indicated part.
            The CMD_MISC__PUT_FRAC_ON_FIRST bit tells the "special_sequential" concept
            (if that is the subject concept) that fractions are allowed, and they
            are to be applied to the first call only. */
         result->cmd.cmd_misc_flags |= CMD_MISC__PUT_FRAC_ON_FIRST;
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | code_to_use_for_only |
            (index * CMD_FRAC_PART_BIT) | frac_flags | shortenhighlim;

         /* If the call that we are doing has the RESULTFLAG__NO_REEVALUATE flag
            on (meaning we don't re-evaluate under *any* circumstances, particularly
            circumstances like these), we do not re-evaluate the ID bits. */
         if (!(result->result_flags & RESULTFLAG__NO_REEVALUATE))
            update_id_bits(result);

         result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;

         do_call_in_series(result, TRUE, FALSE, TRUE, FALSE);
         if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");

         if (result->result_flags & RESULTFLAG__SECONDARY_DONE) {
            index = 0;
            frac_flags |= CMD_FRAC_IMPROPER_BIT;
         }

         result->result_flags &= ~RESULTFLAG__SECONDARY_DONE;  /* **** need this? */
         /* We will pass the RESULTFLAG__DID_LAST_PART bit back to our client. */
         if (doing_just_one) break;
      }
      while (!(result->result_flags & RESULTFLAG__DID_LAST_PART));

      goto get_out;
   }

   goto get_out;

 do_less:

   do_call_in_series_simple(result);

 get_out: ;
}


Private void do_concept_replace_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tttt;
   uint32 finalresultflags = 0;
   int stopindex;
   uint32 newfracs;
   uint32 frac_key;

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
      fail("Can't stack meta or fractional concepts.");

   *result = *ss;

   /* Do the initial part, if any, without the concept. */

   /* Values of arg1 are:
      0 - replace last part
      1 - interrupt before last part
      2 - interrupt after given fraction
      3 - "sandwich" - interrupt after 1/2
      8 - replace Nth part
      9 = interrupt after Nth part */

   switch (parseptr->concept->value.arg1) {
   case 9:
      stopindex = parseptr->options.number_fields;      /* Interrupt after Nth part. */
      frac_key = CMD_FRAC_CODE_FROMTO;
      break;
   case 8:
      stopindex = parseptr->options.number_fields-1;    /* Replace Nth part. */
      frac_key = CMD_FRAC_CODE_FROMTO;
      break;
   default:
      stopindex = 1;     /* Interrupt/replace last part. */
      frac_key = CMD_FRAC_CODE_FROMTOREV;
      break;
   }

   if (stopindex > 0) {
      long_boolean bjunk;
      int numer;

      tttt = *result;
      tttt.cmd = ss->cmd;

      if (parseptr->concept->value.arg1 == 2 ||
          parseptr->concept->value.arg1 == 3) {
         numer = (parseptr->concept->value.arg1 == 2) ?
            parseptr->options.number_fields :      /* "Interrupt after M/N". */
            0x21;                                  /* "Sandwich". */

         tttt.cmd.cmd_frac_flags =
            process_new_fractions(numer & 0xF, (numer >> 4) & 0xF,
                                  ss->cmd.cmd_frac_flags,
                                  0, FALSE, &bjunk);
      }
      else {
         if (frac_key == CMD_FRAC_CODE_FROMTOREV) {
            tttt.cmd.cmd_frac_flags =
               CMD_FRAC_NULL_VALUE | frac_key | CMD_FRAC_PART_BIT | CMD_FRAC_PART2_BIT;
         }
         else {
            tttt.cmd.cmd_frac_flags =
               CMD_FRAC_NULL_VALUE | frac_key | (stopindex * CMD_FRAC_PART_BIT);
         }
      }

      newfracs = tttt.cmd.cmd_frac_flags & 0xFF;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }

   /* Do the interruption/replacement call. */

   tttt = *result;
   tttt.cmd = ss->cmd;
   tttt.cmd.parseptr = parseptr->subsidiary_root;
   if (!(result->result_flags & RESULTFLAG__NO_REEVALUATE))
      update_id_bits(&tttt);           /* So you can interrupt with "leads run", etc. */
   tttt.cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;
   move(&tttt, FALSE, result);
   finalresultflags |= result->result_flags;
   normalize_setup(result, simple_normalize);

   /* Do the final part, if there is more. */

   switch (parseptr->concept->value.arg1) {
   case 0:
      goto nolastpart;
   case 1:
      frac_key = CMD_FRAC_REVERSE | CMD_FRAC_CODE_ONLY | 
         CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE;
      break;
   case 2: case 3:
      frac_key = (newfracs<<8) | 0x0011;
      break;
   default:
      frac_key = CMD_FRAC_CODE_FROMTOREV |
         ((parseptr->options.number_fields+1) * CMD_FRAC_PART_BIT) | CMD_FRAC_NULL_VALUE;
      break;
   }

   tttt = *result;
   /* Skip over the concept. */
   tttt.cmd = ss->cmd;
   tttt.cmd.cmd_frac_flags = frac_key;
   /* We continue the expiration state of the call being interrupted. */
   tttt.cmd.prior_expire_bits |= finalresultflags & EXPIRATION_STATE_BITS;
   move(&tttt, FALSE, result);
   finalresultflags |= result->result_flags;
   normalize_setup(result, simple_normalize);

   nolastpart:

   result->result_flags = finalresultflags & ~3;
}


static void do_concept_interlace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* We don't allow any other fractional stuff. */

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
      fail("Can't stack meta or fractional concepts.");

   uint32 first_doneflag = 0;
   uint32 second_doneflag = 0;
   uint32 indexa = 0;
   uint32 indexb = 0;

   prepare_for_call_in_series(result, ss);
   uint32 a_frac_flags = ss->cmd.cmd_frac_flags;
   uint32 b_frac_flags = ss->cmd.cmd_frac_flags;

   do {
      indexa++;

      if (!(first_doneflag & RESULTFLAG__DID_LAST_PART)) {
         /* Do the indicated part of the first call. */
         uint32 save_elongation = result->cmd.prior_elongation_bits;
         uint32 save_expire = result->cmd.prior_expire_bits;
         result->cmd = ss->cmd;
         result->cmd.prior_elongation_bits = save_elongation;
         result->cmd.prior_expire_bits = save_expire;
         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY |
            (indexa * CMD_FRAC_PART_BIT) | a_frac_flags;
         if (!(result->result_flags & RESULTFLAG__NO_REEVALUATE))
            update_id_bits(result);
         result->cmd.cmd_misc_flags &= ~CMD_MISC__NO_EXPAND_MATRIX;
         do_call_in_series(result, TRUE, FALSE, TRUE, FALSE);

         if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");

         if (result->result_flags & RESULTFLAG__SECONDARY_DONE) {
            indexa = 0;
            a_frac_flags |= CMD_FRAC_IMPROPER_BIT;
         }

         first_doneflag = result->result_flags;
         result->result_flags &= ~RESULTFLAG__SECONDARY_DONE;  /* **** need this? */
      }
      else if (!(second_doneflag & RESULTFLAG__DID_LAST_PART))
         warn(warn__bad_interlace_match);

      indexb++;

      if (!(second_doneflag & RESULTFLAG__DID_LAST_PART)) {
         /* Do the indicated part of the second call. */
         uint32 save_elongation = result->cmd.prior_elongation_bits;
         uint32 save_expire = result->cmd.prior_expire_bits;
         result->cmd = ss->cmd;
         result->cmd.prior_elongation_bits = save_elongation;
         result->cmd.prior_expire_bits = save_expire;
         result->cmd.parseptr = parseptr->subsidiary_root;

         // If other part has run out, and we aren't doing an improper fraction,
         // just finish this.
         if ((first_doneflag & RESULTFLAG__DID_LAST_PART) &&
             !(second_doneflag & RESULTFLAG__SECONDARY_DONE)) {
            result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_FROMTOREV |
               (indexb * CMD_FRAC_PART_BIT) | b_frac_flags;
            do_call_in_series(result, TRUE, FALSE, TRUE, FALSE);
            return;
         }

         result->cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_CODE_ONLY |
            (indexb * CMD_FRAC_PART_BIT) | b_frac_flags;

         do_call_in_series(result, TRUE, FALSE, TRUE, FALSE);

         if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");

         if (result->result_flags & RESULTFLAG__SECONDARY_DONE) {
            indexb = 0;
            b_frac_flags |= CMD_FRAC_IMPROPER_BIT;
         }

         second_doneflag = result->result_flags;
         result->result_flags &= ~RESULTFLAG__SECONDARY_DONE;  /* **** need this? */
      }
      else if (!(first_doneflag & RESULTFLAG__DID_LAST_PART))
         warn(warn__bad_interlace_match);
   }
   while ((first_doneflag & second_doneflag & RESULTFLAG__DID_LAST_PART) == 0);
}


Private void do_concept_fractional(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* Note: if we ever implement something that omits the first fraction, that
      concept would have to have "CONCPROP__NO_STEP" set in concept_table, and
      things might get ugly.
   Actually, we have implemented exactly that -- "do last fraction", and we
      have not set CONCPROP__NO_STEP in the concept table.  The user is responsible
      for the consequences of using this. */

   /* The meaning of arg1 is as follows:
      0 - "M/N" - do first part
      1 - "DO THE LAST M/N"
      2 - "1-M/N" do the whole call and then some. */

   int numer, denom;
   uint32 new_fracs;
   uint32 incoming_fracs = ss->cmd.cmd_frac_flags;
   long_boolean improper;

   numer = parseptr->options.number_fields;
   denom = numer >> 4;
   numer &= 0xF;

   /* Check that user isn't doing something stupid. */
   if (numer <= 0 || numer >= denom)
      fail("Illegal fraction.");

   /* If being asked to do "1-M/N", make the fraction improper. */
   if (parseptr->concept->value.arg1 == 2) numer += denom;

   new_fracs = process_new_fractions(numer,
                                     denom,
                                     incoming_fracs,
                                     parseptr->concept->value.arg1,
                                     parseptr->concept->value.arg1 == 2,
                                     &improper);

   if (improper) {
      /* Do the whole call first, then part of it again. */

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"invert/central/snag/mystic\" followed by this concept or modifier.");

      if (     (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) != 0 &&
               ((ss->cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK|CMD_FRAC_BREAKING_UP)) == (CMD_FRAC_CODE_ONLY|CMD_FRAC_BREAKING_UP))) {

         /* We are being asked to do just one part of "1-3/5 swing the fractions". */

         ss->cmd.cmd_frac_flags &= ~0xFFFF;  /* Will pick up the low 16 bits shortly. */

         if ((ss->cmd.cmd_frac_flags & CMD_FRAC_IMPROPER_BIT) == 0) {
            /* First time around -- Do the indicated part of a whole swing the fractions. */

            ss->cmd.cmd_frac_flags |= CMD_FRAC_NULL_VALUE;
            move(ss, FALSE, result);
            /* And, if it reported that it did the last part, change it to
               "secondary_done" to indicate that we're not finished. */
            if (result->result_flags & RESULTFLAG__DID_LAST_PART) {
               result->result_flags &= ~RESULTFLAG__DID_LAST_PART;
               result->result_flags |= RESULTFLAG__SECONDARY_DONE;
            }
         }
         else {
            /* Second time around -- just do the fractional part. */

            ss->cmd.cmd_frac_flags |= new_fracs;
            move(ss, FALSE, result);
         }
      }
      else {
         uint32 save_elongation;
         uint32 save_expire;

         prepare_for_call_in_series(result, ss);
         result->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | CMD_FRAC_NULL_VALUE;
         do_call_in_series(result, FALSE, FALSE,
            !(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX),
            FALSE);

         save_elongation = result->cmd.prior_elongation_bits;
         save_expire = result->cmd.prior_expire_bits;
         result->cmd = ss->cmd;      /* The call we wish to execute. */
         result->cmd.prior_elongation_bits = save_elongation;
         result->cmd.prior_expire_bits = save_expire;
         result->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | new_fracs;

         if ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) >= (CMD_FRAC_PART_BIT*2) &&
                  ((ss->cmd.cmd_frac_flags & (CMD_FRAC_CODE_MASK|CMD_FRAC_BREAKING_UP)) ==
                   (CMD_FRAC_CODE_FROMTOREV|CMD_FRAC_BREAKING_UP))) {
   
            /* We are being asked to do everything beyond some part of
               "1-3/5 swing the fractions". */
            result->cmd.cmd_frac_flags &= 0xFFFF;   /* Do the whole 3/5. */
         }

         do_call_in_series(result, FALSE, FALSE,
            !(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX),
            FALSE);
      }
   }
   else {
      ss->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | new_fracs;
      move(ss, FALSE, result);
   }
}


static void do_concept_so_and_so_begin(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 finalresultflags;
   selector_kind saved_selector;
   int i;
   setup setup1, setup2;
   setup the_setups[2];

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
      fail("Can't stack meta or fractional concepts.");

   saved_selector = current_options.who;
   current_options.who = parseptr->options.who;

   setup1 = *ss;              /* designees */
   setup2 = *ss;              /* non-designees */
   
   if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't identify people in this setup.");
   for (i=0; i<setup_attrs[ss->kind].setup_limits+1; i++) {
      if (ss->people[i].id1) {
         if (selectp(ss, i))
            clear_person(&setup2, i);
         else
            clear_person(&setup1, i);
      }
   }
   
   current_options.who = saved_selector;
   
   normalize_setup(&setup1, normalize_before_isolated_call);
   normalize_setup(&setup2, normalize_before_isolated_call);
   /* Set the fractionalize field to execute the first part of the call. */
   setup1.cmd.cmd_frac_flags = CMD_FRAC_CODE_FROMTO | CMD_FRAC_PART_BIT*1 | CMD_FRAC_NULL_VALUE;

   /* The selected people execute the first part of the call. */

   move(&setup1, FALSE, &the_setups[0]);
   the_setups[1] = setup2;

    /* Give the people who didn't move the same result flags as those who did.
      This is important for the "did last part" check. */
   the_setups[1].result_flags = the_setups[0].result_flags;
   the_setups[1].result_flags = get_multiple_parallel_resultflags(the_setups, 2);

   merge_setups(&the_setups[0], merge_c1_phantom, &the_setups[1]);
   finalresultflags = the_setups[1].result_flags;

   normalize_setup(&the_setups[1], simple_normalize);
   the_setups[1].cmd = ss->cmd;   /* Just in case it got messed up, which shouldn't have happened. */

   /* Set the fractionalize field to execute the rest of the call. */
   the_setups[1].cmd.cmd_frac_flags = CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE;
   move(&the_setups[1], FALSE, result);
   finalresultflags |= result->result_flags;
   normalize_setup(result, simple_normalize);
  
   result->result_flags = finalresultflags & ~3;
}


typedef struct {
   setup_kind a;
   int b;
} map_finder;


Private map_finder sc_2x4 = {s1x4, 1};
Private map_finder sc_1x8 = {s1x4, 0};
Private map_finder sc_qtg = {sdmd, 1};
Private map_finder sc_ptp = {sdmd, 0};



Private void do_concept_concentric(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_finder *mf;
   calldef_schema schema = (calldef_schema) parseptr->concept->value.arg1;

   if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_CROSS)) {
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
      default:
         fail("Redundant 'CROSS' modifiers.");
      }

      if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_SINGLE)) {
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

      if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_GRAND)) {
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

      ss->cmd.cmd_final_flags.her8it &=
         ~(INHERITFLAG_CROSS | INHERITFLAG_SINGLE | INHERITFLAG_GRAND);

      /* We don't allow other flags, like "left", though we do allow "half". */

      if ((ss->cmd.cmd_final_flags.her8it & ~(INHERITFLAG_HALF | INHERITFLAG_LASTHALF)) |
          ss->cmd.cmd_final_flags.final)
         fail("Illegal modifier before \"concentric\".");
   }

   switch (schema) {
      case schema_single_concentric:
      case schema_single_cross_concentric:
         switch (ss->kind) {
            case s2x4:   mf = &sc_2x4; break;
            case s1x8:   mf = &sc_1x8; break;
            case s_qtag: mf = &sc_qtg; break;
            case s_ptpd: mf = &sc_ptp; break;
            case s1x4: case sdmd:
               concentric_move(ss, &ss->cmd, &ss->cmd,
                     schema, 0, DFM1_CONC_CONCENTRIC_RULES, TRUE, result);
               return;
            default:
               fail("Can't figure out how to do single concentric here.");
         }
      
         ss->cmd.parseptr = parseptr;    /* Reset it to execute this same concept again, until it doesn't have to split any more. */
         new_divided_setup_move(ss, MAPCODE(mf->a,2,MPKIND__SPLIT,mf->b), phantest_ok, TRUE, result);
         break;
      default:
         concentric_move(ss, &ss->cmd, &ss->cmd,
               schema, 0, DFM1_CONC_CONCENTRIC_RULES, TRUE, result);
         /* 8-person concentric operations do not show the split. */
         result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
         break;
   }
}



Private void do_concept_tandem(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* The "arg3" field of the concept descriptor contains bit fields as follows:
      "100" bit:  this takes a selector
      "F0" field: (fractional) twosome info --
         0=solid
         1=twosome
         2=solid-frac-twosome
         3=twosome-frac-solid
      "0F" field:
         0=normal
         2=plain-gruesome
         3=gruesome-with-wave-assumption */

   uint32 mxnflags = ss->cmd.cmd_final_flags.her8it &
      (INHERITFLAG_SINGLE | INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK);

   if (parseptr->concept->value.arg2 == CONCPROP__NEEDK_4DMD ||
       parseptr->concept->value.arg2 == CONCPROP__NEEDK_4D_4PTPD ||
       parseptr->concept->value.arg2 == CONCPROP__NEEDK_TWINQTAG)
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg1;
   ss->cmd.cmd_final_flags.her8it &=
      ~(INHERITFLAG_SINGLE | INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK);

   tandem_couples_move(
     ss,
     (parseptr->concept->value.arg3 & 0x100) ? parseptr->options.who : selector_uninitialized,
     (parseptr->concept->value.arg3 & 0xF0) >> 4, /* (fractional) twosome info */
     parseptr->options.number_fields,
     parseptr->concept->value.arg3 & 0xF,         /* normal/phantom/gruesome etc. */
     (tandem_key) parseptr->concept->value.arg4,  /* key */
     mxnflags,
     FALSE,
     result);
}


/* ARGSUSED */
Private void do_concept_standard(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This should never be called this way -- "standard" is treated specially.
      But we do need a nonzero entry in the dispatch table. */
   fail("Huh? -- standard concept out of place.");
}


/* ARGSUSED */
Private void do_concept_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   ss->cmd.cmd_misc_flags |= CMD_MISC__MATRIX_CONCEPT;
   /* The above flag will raise an error in all contexts except those few to which
      this concept may be applied. */
   move(ss, FALSE, result);
}


extern long_boolean do_big_concept(
   setup *ss,
   setup *result)
{
   void (*concept_func)(setup *, parse_block *, setup *);
   parse_block *orig_concept_parse_block = ss->cmd.parseptr;
   parse_block *this_concept_parse_block = orig_concept_parse_block;
   concept_descriptor *this_concept = this_concept_parse_block->concept;
   concept_kind this_kind = this_concept->kind;
   concept_table_item *this_table_item = &concept_table[this_kind];
   parse_block artificial_parse_block;
   uint32 extraheritmods;
   uint32 extrafinalmods;

   /* Most concepts simply have no understanding of, or tolerance for, modifiers
      like "interlocked" in front of them.  So, in the general case, we check for
      those modifiers and raise an error if any are on.  Hence, most of the concept
      procedures don't check for these modifiers, we guarantee that they are off.
      (There are other modifiers bits than those listed here.  They are never
      tolerated under any circumstances by any concept, and have already been
      checked.)

      Now there are two contexts in which we allow modifiers:

         Some concepts (e.g. [reverse] crazy) are explictly coded to allow,
         and check for, modifier bits.  Those are marked with the
         "CONCPROP__PERMIT_REVERSE" bit (misnamed -- this bit enables all of
         the bits shown, not just reverse).  For those concepts, we don't do
         the check, and the concept code is responsible for being sure that
         everything is legal.

         Some concepts (e.g. [interlocked] phantom lines) desire to allow the
         user to type a modifier on one line and then enter another concept,
         and have us figure out what concept was really chosen.  To handle
         these, we look through a table of such pairs. */

   /* If "CONCPROP__PERMIT_REVERSE" is on, let anything pass. */

   extraheritmods = ss->cmd.cmd_final_flags.her8it & (INHERITFLAG_REVERSE|INHERITFLAG_LEFT|INHERITFLAG_GRAND|INHERITFLAG_CROSS|INHERITFLAG_SINGLE|INHERITFLAG_INTLK);
   extrafinalmods = ss->cmd.cmd_final_flags.final & FINAL__SPLIT;

   if ((extraheritmods | extrafinalmods) && !(this_table_item->concept_prop & CONCPROP__PERMIT_REVERSE)) {
      /* This can only be legal if we find a translation in the table. */

      concept_fixer_thing *p;

      for (p=concept_fixer_table ; p->newheritmods | p->newfinalmods ; p++) {
         if (p->newheritmods == extraheritmods && p->newfinalmods == extrafinalmods && &concept_descriptor_table[p->before] == this_concept) {
            this_concept = &concept_descriptor_table[p->after];
            artificial_parse_block = *this_concept_parse_block;
            artificial_parse_block.concept = this_concept;
            this_concept_parse_block = &artificial_parse_block;
            this_kind = this_concept->kind;
            this_table_item = &concept_table[this_kind];
            ss->cmd.cmd_final_flags.her8it &= ~extraheritmods;   /* Take out those mods. */
            ss->cmd.cmd_final_flags.final &= ~extrafinalmods;
            goto found_new_concept;
         }
      }

      fail("Illegal modifier before a concept.");

      found_new_concept: ;
   }

   /* Take care of combinations like "mystic triple waves". */

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_MYSTIC) {
      if (this_table_item->concept_prop & CONCPROP__PERMIT_MYSTIC) {

         /* This is "mystic" with something like "triple waves". */
         /* Turn on the good bits. */

         ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_SPLIT;
         if (ss->cmd.cmd_misc2_flags & CMD_MISC2__INVERT_MYSTIC)
            ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_INVERT;

         /* And turn off the old ones. */

         ss->cmd.cmd_misc2_flags &= ~(CMD_MISC2__CENTRAL_MYSTIC|CMD_MISC2__INVERT_MYSTIC);
      }
   }

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {

      /* If we have an "invert", "central", "mystic" or "snag" concept in place,
         we have to check whether the current concept can deal with it. */

      /* The following concepts are always acceptable with invert/snag/etc in place. */

      if (this_kind == concept_snag_mystic ||
          this_kind == concept_central ||
          this_kind == concept_fractional ||
          this_kind == concept_concentric ||
          this_kind == concept_some_vs_others ||
          (this_kind == concept_meta &&
           (this_concept->value.arg1 == meta_key_finish ||
            this_concept->value.arg1 == meta_key_revorder)))
         goto this_is_ok;

      /* Otherwise, if "central" is selected, it must be one of the following ones. */

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__DO_CENTRAL) {
         if (this_kind != concept_fractional &&
             this_kind != concept_fan &&
             (this_kind != concept_meta ||
              (this_concept->value.arg1 != meta_key_like_a &&
               this_concept->value.arg1 != meta_key_shifty)) &&
             (this_kind != concept_meta_one_arg ||
              (this_concept->value.arg1 != meta_key_skip_nth_part &&
               this_concept->value.arg1 != meta_key_shift_n)))
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

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT) && !(this_table_item->concept_prop & CONCPROP__PERMIT_MATRIX))
      fail("\"Matrix\" concept must be followed by applicable concept.");

   concept_func = this_table_item->concept_action;
   ss->cmd.parseptr = ss->cmd.parseptr->next;

   /* When we invoke one of the functions, we will have:
      2ndarg = the concept we are doing
      1starg->cmd.parseptr = the next concept after that */

   if (concept_func == 0) return FALSE;

   if (this_concept->level > calling_level) warn(warn__bad_concept_level);

   clear_people(result);

   if (this_table_item->concept_prop & CONCPROP__SET_PHANTOMS)
      ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

   if (this_table_item->concept_prop & CONCPROP__NO_STEP)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   /* "Standard" is special -- process it now. */

   if (this_kind == concept_standard) {
      parse_block *substandard_concptptr;
      uint64 junk_concepts;
      int tbonetest = 0;
      int stdtest = 0;
      int livemask = 0;

      /* Skip to the phantom-line (or whatever) concept by going over the "standard" and skipping comments. */

      junk_concepts.her8it = 0;
      junk_concepts.final = 0;
      substandard_concptptr = process_final_concepts(orig_concept_parse_block->next, TRUE, &junk_concepts);
   
      /* If we hit "matrix", do a little extra stuff and continue. */

      if (junk_concepts.her8it == 0 &&
          junk_concepts.final == 0 &&
substandard_concptptr->concept->kind == concept_matrix) {
         ss->cmd.cmd_misc_flags |= CMD_MISC__MATRIX_CONCEPT;
         substandard_concptptr = process_final_concepts(substandard_concptptr->next, TRUE, &junk_concepts);
      }

      if (junk_concepts.her8it != 0 ||
          junk_concepts.final != 0 ||
          (!(concept_table[substandard_concptptr->concept->kind].concept_prop & CONCPROP__STANDARD)))
         fail("This concept must be used with some offset/distorted/phantom concept.");

      /* We don't think stepping to a wave is ever a good idea if standard is used.  Most calls that
         permit standard (CONCPROP__STANDARD is on) forbid it anyway (CONCPROP__NO_STEP is on also),
         but a few (e.g. concept_triple_lines) permit standard but don't necessarily forbid stepping
         to a wave.  This is so that interesting cases of triple columns turn and weave will work.
         However, we think that this should be disallowed if "so-and-so are standard in triple lines"
         is given.  At least, that is the theory behind this next line of code. */
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

      if (concept_table[substandard_concptptr->concept->kind].concept_prop & CONCPROP__SET_PHANTOMS)
         ss->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

      if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX)) {
         uint32 prop_bits = concept_table[substandard_concptptr->concept->kind].concept_prop;
         /* If the "arg2_matrix" bit is on, pick up additional matrix descriptor bits from the arg2 word. */
         if (prop_bits & CONCPROP__NEED_ARG2_MATRIX) prop_bits |= substandard_concptptr->concept->value.arg2;
         do_matrix_expansion(ss, prop_bits, FALSE);
      }

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

      if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do this concept in this setup.");
   
      {
         int i, j;
         selector_kind saved_selector = current_options.who;
   
         current_options.who = this_concept_parse_block->options.who;
   
         for (i=0, j=1; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
            int p = ss->people[i].id1;
            tbonetest |= p;
            if (p) {
               livemask |= j;
               if (selectp(ss, i)) stdtest |= p;
            }
         }
   
         current_options.who = saved_selector;
      }
   
      if (!tbonetest) {
         result->result_flags = 0;
         result->kind = nothing;
         return TRUE;
      }
   
      if ((tbonetest & 011) != 011) fail("People are not T-boned -- 'standard' is meaningless.");
   
      if (!stdtest) fail("No one is standard.");
      if ((stdtest & 011) == 011) fail("The standard people are not facing consistently.");
   
      global_tbonetest = stdtest;
      global_livemask = livemask;
      orig_tbonetest = tbonetest;

      ss->cmd.parseptr = substandard_concptptr->next;
      (concept_table[substandard_concptptr->concept->kind].concept_action)(ss, substandard_concptptr, result);
      /* Beware -- result is not necessarily canonicalized. */
      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;  /* **** For now. */
      return TRUE;
   }

   if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX)) {
      uint32 prop_bits = this_table_item->concept_prop;
      /* If the "arg2_matrix" bit is on, pick up additional matrix descriptor bits from the arg2 word. */
      if (prop_bits & CONCPROP__NEED_ARG2_MATRIX) prop_bits |= this_concept->value.arg2;
      do_matrix_expansion(ss, prop_bits, FALSE);
   }

   /* We can no longer do any matrix expansion, unless this is "phantom" and "tandem", in which case we continue to allow it.
      The code for the "C1 phantom" concept will check whether it is being used with some tandem-like concept, and expand to
      the matrix that it really wants if so, or set the NO_EXPAND_MATRIX flag if not.
   We also don't set the flag if this concept was "fractional" or stable or fractional stable.  Those
      concepts can be effectively oblivious to matrix expansion.  The tests for these are (from lines out)
      "stable 2x8 matrix roll em" and "1/4 (fractional) split phantom boxes split the difference". */

   if (!(this_table_item->concept_prop & CONCPROP__MATRIX_OBLIVIOUS))
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

   /* See if this concept can be invoked with "standard".  If so, it wants
      tbonetest and livemask computed, and expects the former to indicate
      only the standard people. */

   if (this_table_item->concept_prop & (CONCPROP__STANDARD | CONCPROP__GET_MASK)) {
      int i;
      uint32 j;
      long_boolean doing_select;
      selector_kind saved_selector = current_options.who;

      if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do this concept in this setup.");

      global_tbonetest = 0;
      global_livemask = 0;
      global_selectmask = 0;
      global_tboneselect = 0;
      doing_select = this_table_item->concept_prop & CONCPROP__USE_SELECTOR;

      if (doing_select) {
         current_options.who = this_concept_parse_block->options.who;
      }

      for (i=0, j=1; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
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
         result->result_flags = 0;
         result->kind = nothing;
         return TRUE;
      }
   }

   (*concept_func)(ss, this_concept_parse_block, result);
   /* Beware -- result is not necessarily canonicalized. */
   if (!(this_table_item->concept_prop & CONCPROP__SHOW_SPLIT))
      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
   return TRUE;
}


#define Standard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX | CONCPROP__STANDARD)
#define Nostandard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX)
#define Nostep_phantom (CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS)


/* Beware!!  This table must be keyed to definition of "concept_kind" in sd.h . */

concept_table_item concept_table[] = {
   {0, 0},                                /* concept_another_call_next_mod */
   /* concept_mod_declined */             {0, 0},
   /* marker_end_of_list */               {0, 0},
   /* concept_comment */                  {0, 0},
   /* concept_concentric */               {CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,                                        do_concept_concentric},
   /* concept_tandem */                   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT,                                      do_concept_tandem},
   /* concept_some_are_tandem */          {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT | CONCPROP__USE_SELECTOR,             do_concept_tandem},
   /* concept_frac_tandem */              {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT | CONCPROP__USE_NUMBER,               do_concept_tandem},
   /* concept_some_are_frac_tandem */     {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT | CONCPROP__USE_NUMBER | CONCPROP__USE_SELECTOR, do_concept_tandem},
   /* concept_gruesome_tandem */          {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT | CONCPROP__SET_PHANTOMS,             do_concept_tandem},
   /* concept_gruesome_frac_tandem */     {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__SHOW_SPLIT | CONCPROP__SET_PHANTOMS | CONCPROP__USE_NUMBER, do_concept_tandem},
   /* concept_checkerboard */             {0,                                                                                      do_concept_checkerboard},
   /* concept_sel_checkerboard */         {CONCPROP__USE_SELECTOR | CONCPROP__GET_MASK,                                            do_concept_checkerboard},
   /* concept_anchor */                   {CONCPROP__USE_SELECTOR | CONCPROP__GET_MASK,                                            anchor_someone_and_move},
   /* concept_reverse */                  {0, 0},
   /* concept_left */                     {0, 0},
   /* concept_grand */                    {0, 0},
   /* concept_magic */                    {0, 0},
   /* concept_cross */                    {0, 0},
   /* concept_single */                   {0, 0},
   /* concept_singlefile */               {0, 0},
   /* concept_interlocked */              {0, 0},
   /* concept_yoyo */                     {0, 0},
   /* concept_fractal */                  {0, 0},
   /* concept_fast */                     {0, 0},
   /* concept_straight */                 {0, 0},
   /* concept_twisted */                  {0, 0},
   /* concept_12_matrix */                {0, 0},
   /* concept_16_matrix */                {0, 0},
   /* concept_revert */                   {0, 0},
   /* concept_1x2 */                      {0, 0},
   /* concept_2x1 */                      {0, 0},
   /* concept_2x2 */                      {0, 0},
   /* concept_1x3 */                      {0, 0},
   /* concept_3x1 */                      {0, 0},
   /* concept_3x3 */                      {0, 0},
   /* concept_4x4 */                      {0, 0},
   /* concept_5x5 */                      {0, 0},
   /* concept_6x6 */                      {0, 0},
   /* concept_7x7 */                      {0, 0},
   /* concept_8x8 */                      {0, 0},
   {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom,
    do_concept_expand_some_matrix},                                               /* concept_create_matrix */
   /* concept_funny */                    {0,                                                                                      0},
   /* concept_randomtrngl */              {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_REVERSE,                      triangle_move},
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_REVERSE | CONCPROP__USE_SELECTOR,
    triangle_move},                                         /* concept_selbasedtrngl */
   {0, 0},                                                  /* concept_split */
   {CONCPROP__NO_STEP | CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_MATRIX | CONCPROP__GET_MASK,
    do_concept_do_each_1x4},                                /* concept_each_1x4 */
   {0, 0},                                                  /* concept_diamond */
   {0, 0},                                                  /* concept_triangle */
   {0, 0},                                                  /* concept_leadtriangle */
   {0, do_concept_do_both_boxes},                           /* concept_do_both_boxes */
   {CONCPROP__SHOW_SPLIT, do_concept_once_removed},         /* concept_once_removed */
   {CONCPROP__NEEDK_4X4 | Nostep_phantom,
    do_concept_do_phantom_2x2},                             /* concept_do_phantom_2x2 */
   {CONCPROP__NEEDK_2X8 | Nostandard_matrix_phantom,
    do_concept_do_phantom_boxes},                           /* concept_do_phantom_boxes */
   /* concept_do_phantom_diamonds */      {CONCPROP__NEEDK_4D_4PTPD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,               do_concept_do_phantom_diamonds},
   /* concept_do_phantom_1x6 */           {CONCPROP__NEEDK_2X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                      do_concept_do_phantom_1x6},
   {CONCPROP__NEEDK_3X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,
    do_concept_do_phantom_triple_1x6},                      /* concept_do_phantom_triple_1x6 */
   {CONCPROP__NEEDK_2X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,
    do_concept_do_phantom_1x8},                             /* concept_do_phantom_1x8 */
   /* concept_do_phantom_triple_1x8 */    {CONCPROP__NEEDK_3X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,                      do_concept_do_phantom_triple_1x8},
   /* concept_do_phantom_2x4 */           {CONCPROP__NEEDK_QUAD_1X4 | Standard_matrix_phantom | CONCPROP__PERMIT_MYSTIC,           do_phantom_2x4_concept},
   /* concept_do_phantom_stag_qtg */      {CONCPROP__NEEDK_4X4 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_phantom_stag_qtg_concept},
   /* concept_do_phantom_diag_qtg */      {CONCPROP__NEEDK_4X4 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_phantom_diag_qtg_concept},
   /* concept_do_phantom_2x3 */           {CONCPROP__NEEDK_3X4 | CONCPROP__NO_STEP | Standard_matrix_phantom,                      do_concept_do_phantom_2x3},
   /* concept_divided_2x4 */              {CONCPROP__NEEDK_2X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,                      do_concept_divided_2x4},
   /* concept_divided_2x3 */              {CONCPROP__NEEDK_2X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                      do_concept_divided_2x3},
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__NO_STEP | CONCPROP__GET_MASK | Nostandard_matrix_phantom,
    do_concept_do_divided_diamonds},                                              /* concept_do_divided_diamonds */
   /* concept_distorted */                {CONCPROP__STANDARD,                                                                     do_concept_distorted},
   /* concept_single_diagonal */          {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_single_diagonal},
   /* concept_double_diagonal */          {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 do_concept_double_diagonal},
   /* concept_parallelogram */            {CONCPROP__GET_MASK,                                                                     do_concept_parallelogram},
   {CONCPROP__NEEDK_TRIPLE_1X4 | Standard_matrix_phantom | CONCPROP__PERMIT_MYSTIC,
    do_concept_triple_lines},                               /* concept_triple_lines */
   {CONCPROP__NEED_ARG2_MATRIX | Nostandard_matrix_phantom,
    do_concept_multiple_lines_tog},                         /* concept_multiple_lines_tog */
   {CONCPROP__NEED_ARG2_MATRIX | Standard_matrix_phantom,
    do_concept_multiple_lines_tog},                         /* concept_multiple_lines_tog_std */
   {CONCPROP__NEEDK_3X8 | Nostandard_matrix_phantom,
    do_concept_triple_1x8_tog},                             /* concept_triple_1x8_tog */
   {CONCPROP__NEEDK_QUAD_1X4 | Standard_matrix_phantom,
    do_concept_quad_lines},                                 /* concept_quad_lines */
   /* concept_quad_boxes */               {CONCPROP__NEEDK_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_quad_boxes},
   /* concept_quad_boxes_together */      {CONCPROP__NEEDK_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_quad_boxes_tog},
   /* concept_triple_boxes */             {CONCPROP__NEEDK_2X6 | Nostandard_matrix_phantom | CONCPROP__PERMIT_MYSTIC,              do_concept_triple_boxes},
   /* concept_triple_boxes_together */    {CONCPROP__NEEDK_2X6 | Nostandard_matrix_phantom,                                        do_concept_triple_boxes_tog},
   /* concept_triple_diamonds */          {CONCPROP__NEEDK_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                   do_concept_triple_diamonds},
   /* concept_triple_diamonds_together */ {CONCPROP__NEEDK_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                   do_concept_triple_diamonds_tog},
   /* concept_quad_diamonds */            {CONCPROP__NEEDK_4D_4PTPD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,               do_concept_quad_diamonds},
   /* concept_quad_diamonds_together */   {CONCPROP__NEEDK_4D_4PTPD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,               do_concept_quad_diamonds_tog},
   {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__STANDARD | Nostep_phantom,
    do_concept_inner_outer},                                /* concept_in_out_std */
   {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom,
    do_concept_inner_outer},                                /* concept_in_out_nostd */
   /* concept_triple_diag */              {CONCPROP__NEEDK_BLOB | Nostep_phantom | CONCPROP__STANDARD,                             do_concept_triple_diag},
   /* concept_triple_diag_together */     {CONCPROP__NEEDK_BLOB | Nostep_phantom | CONCPROP__GET_MASK,                             do_concept_triple_diag_tog},
   /* concept_triple_twin */              {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__NO_STEP | Standard_matrix_phantom | CONCPROP__PERMIT_MYSTIC, triple_twin_move},
   /* concept_misc_distort */             {CONCPROP__NEED_ARG2_MATRIX,                                                             distorted_2x2s_move},
   /* concept_old_stretch */              {0/*CONCPROP__NO_STEP*/,                                                                 do_concept_old_stretch},
   /* concept_new_stretch */              {CONCPROP__GET_MASK/*CONCPROP__NO_STEP*/,                                                do_concept_new_stretch},
   /* concept_assume_waves */             {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_assume_waves},
   {0, do_concept_active_phantoms},                         /* concept_active_phantoms */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_mirror},                                     /* concept_mirror */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_central},                                    /* concept_central */
   /* concept_snag_mystic */              {CONCPROP__MATRIX_OBLIVIOUS,                                                             do_concept_central},
   /* concept_crazy */                    {CONCPROP__PERMIT_REVERSE,                                                               do_concept_crazy},
   /* concept_frac_crazy */               {CONCPROP__USE_NUMBER | CONCPROP__PERMIT_REVERSE,                                        do_concept_crazy},
   {CONCPROP__PERMIT_REVERSE | CONCPROP__NEED_ARG2_MATRIX |
    CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,
    do_concept_phan_crazy},                                 /* concept_phan_crazy */
   {CONCPROP__PERMIT_REVERSE | CONCPROP__NEED_ARG2_MATRIX | CONCPROP__USE_NUMBER |
    CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,
    do_concept_phan_crazy},                                 /* concept_frac_phan_crazy */
   {0, do_concept_fan},                                     /* concept_fan */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__NO_STEP | CONCPROP__GET_MASK,
    do_c1_phantom_move},                                    /* concept_c1_phantom */
   {CONCPROP__PERMIT_MATRIX | CONCPROP__SET_PHANTOMS,
    do_concept_grand_working},                              /* concept_grand_working */
   {0, do_concept_centers_or_ends},                         /* concept_centers_or_ends */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_SELECTOR,
    so_and_so_only_move},                                   /* concept_so_and_so_only */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_SELECTOR | CONCPROP__SECOND_CALL,
    so_and_so_only_move},                                   /* concept_some_vs_others */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     /* concept_stable */
   {CONCPROP__USE_SELECTOR | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     /* concept_so_and_so_stable */
   {CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     /* concept_frac_stable */
   {CONCPROP__USE_SELECTOR | CONCPROP__USE_NUMBER |
    CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_stable},                                     /* concept_so_and_so_frac_stable */
   {CONCPROP__MATRIX_OBLIVIOUS,
    do_concept_emulate},                                    /* concept_emulate */
   {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX,
    do_concept_standard},                                   /* concept_standard */
   {CONCPROP__MATRIX_OBLIVIOUS, do_concept_matrix},         /* concept_matrix */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,
    do_concept_double_offset},                              /* concept_double_offset */
   {CONCPROP__SECOND_CALL | CONCPROP__PERMIT_REVERSE,
    do_concept_checkpoint},                                 /* concept_checkpoint */
   {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,
    on_your_own_move},                                      /* concept_on_your_own */
   {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,
    do_concept_trace},                                      /* concept_trace */
   {CONCPROP__NO_STEP,
    do_concept_outeracting},                                /* concept_outeracting */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,
    do_concept_ferris},                                     /* concept_ferris */
   {CONCPROP__NO_STEP, do_concept_overlapped_diamond},      /* concept_overlapped_diamond */
   {0, do_concept_all_8},                                   /* concept_all_8 */
   {CONCPROP__SECOND_CALL, do_concept_centers_and_ends},    /* concept_centers_and_ends */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_twice},                                      /* concept_twice */
   {CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_twice},                                      /* concept_n_times */
   {CONCPROP__SECOND_CALL | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_sequential},                                 /* concept_sequential */
   {CONCPROP__SECOND_CALL | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_special_sequential},                         /* concept_special_sequential */
   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,
    do_concept_meta},                                       /* concept_meta */
   {CONCPROP__USE_NUMBER | CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,
    do_concept_meta},                                       /* concept_meta_one_arg */
   {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,
    do_concept_so_and_so_begin},                            /* concept_so_and_so_begin */
   {CONCPROP__USE_NUMBER | CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           /* concept_replace_nth_part */
   {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           /* concept_replace_last_part */
   {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS |
    CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           /* concept_interrupt_at_fraction */
   {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_replace_nth_part},                           /* concept_sandwich */
   {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,
    do_concept_interlace},                                  /* concept_interlace */
   {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS |
    CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,
    do_concept_fractional},                                 /* concept_fractional */
   {CONCPROP__NO_STEP, do_concept_rigger},                  /* concept_rigger */
   {CONCPROP__NO_STEP, common_spot_move},                   /* concept_common_spot */
   {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,
    drag_someone_and_move},                                       /* concept_drag */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,
    do_concept_dblbent},                                    /* concept_dblbent */
   {0, 0},                                                  /* concept_supercall */
   {0, do_concept_diagnose}};                               /* concept_diagnose */
