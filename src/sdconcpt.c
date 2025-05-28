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

/* This defines the following functions:
   impose_assumption_and_move
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

#include "sd.h"


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
   if (ss->kind != ((setup_kind) parseptr->concept->value.arg1))
      fail("Can't make the required matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, FALSE, result);
}


typedef struct {
   int size;
   setup_kind ikind;
   int map1[8];
   int map2[8];
} phan_map;

Private phan_map map_c1_phan = {8, s2x4, {0, 2, 7, 5, 8, 10, 15, 13}, {4, 6, 11, 9, 12, 14, 3, 1}};
Private phan_map map_o_spots = {8, s2x4, {10, -1, -1, 1, 2, -1, -1, 9}, {14, -1, -1, 5, 6, -1, -1, 13}};
Private phan_map map_qt_phan = {8, s_qtag, {-1, -1, 2, 3, -1, -1, 6, 7}, {1, 4, -1, -1, 5, 0, -1, -1}};



Private void do_c1_phantom_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   parse_block *next_parseptr;
   final_set junk_concepts;
   setup setup1, setup2;
   setup the_setups[2];
   int i;
   phan_map *map_ptr;

   /* See if this is a "phantom tandem" (or whatever) by searching ahead, skipping comments of course.
      This means we must skip modifiers too, so we check that there weren't any. */

   next_parseptr = process_final_concepts(parseptr->next, FALSE, &junk_concepts);

   if (next_parseptr->concept->kind == concept_tandem || next_parseptr->concept->kind == concept_frac_tandem) {
      /* Find out what kind of tandem call this is. */

      uint32 what_we_need = 0;

      if (junk_concepts)
         fail("Phantom couples/tandem must not have intervening concpets.");

      /* "Phantom tandem" has a higher level than either "phantom" or "tandem". */
      if (phantom_tandem_level > calling_level) warn(warn__bad_concept_level);

      switch (next_parseptr->concept->value.arg4) {
         case 2: case 3:
            fail("Phantom not allowed with skew or siamese.");
         case 8:     /* Box/boxsome */
            /* We do not expand the matrix.  The caller must say
               "2x8 matrix", or whatever, to get that effect. */
            break;
         case 9:     /* Diamond/diamondsome */
            /* We do not expand the matrix.  The caller must say
               "16 matrix or parallel diamonds", or whatever, to get that effect. */
            break;
         case 4: case 5:   /* Couples/tandems of 3. */
         case 6: case 7:   /* Couples/tandems of 4. */
            /* We do not expand the matrix.  The caller must say
               "2x8 matrix", or whatever, to get that effect. */
            break;
         default:
            /* This is plain "phantom tandem", or whatever.  Expansion to 4x4 is required. */
            what_we_need = CONCPROP__NEED_4X4;
            break;
      }

      if (what_we_need != 0)
         do_matrix_expansion(ss, what_we_need, TRUE);

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX | CMD_MISC__PHANTOMS;
      ss->cmd.parseptr = next_parseptr->next;

      tandem_couples_move(ss,
            next_parseptr->concept->value.arg1 ? next_parseptr->selector : selector_uninitialized,
            next_parseptr->concept->value.arg2,    /* normal=FALSE, twosome=TRUE */
            next_parseptr->number,
            1,                                     /* for phantom */
            next_parseptr->concept->value.arg4,    /* tandem=0 couples=1 siamese=2 */
            result);

      return;
   }

   /* We didn't do this before. */
   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

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
   else if (ss->kind == s4x4) {
      setup temp;

      /* Next, check for a "phantom turn and deal" sort of thing from stairsteps.
         Do the call in each line, them remove resulting phantoms carefully. */

      if (global_livemask == 0x5C5C || global_livemask == 0xA3A3) {
         /* Split into 4 vertical strips. */
         divided_setup_move(ss, &map_4x4v, phantest_ok, TRUE, result);
      }
      else if (global_livemask == 0xC5C5 || global_livemask == 0x3A3A) {
         /* Split into 4 horizontal strips. */
         divided_setup_move(ss, map_lists[s1x4][3]->f[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
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
   else
      fail("Inappropriate setup for phantom concept.");

   setup1 = *ss;
   setup2 = *ss;
   
   setup1.kind = map_ptr->ikind;
   setup2.kind = map_ptr->ikind;
   setup1.rotation = ss->rotation;
   setup2.rotation = ss->rotation+1;
   clear_people(&setup1);
   clear_people(&setup2);

   for (i=0 ; i<map_ptr->size ; i++) {
      if (map_ptr->map1[i] >= 0)
         (void) copy_person(&setup1, i, ss, map_ptr->map1[i]);

      if (map_ptr->map2[i] >= 0)
         (void) copy_rot(&setup2, i, ss, map_ptr->map2[i], 033);
   }
   
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





typedef struct asdfgh {
   int map[4];
   int othermap[4];
   struct asdfgh *other;
} diag_map;

/* These need to be external so that they can refer to each other. */
extern diag_map map_diag1a;
extern diag_map map_diag1b;

diag_map map_diag1a = {{8, 11, 0, 3}, {12, 15, 4, 7}, &map_diag1b};
diag_map map_diag1b = {{12, 15, 4, 7}, {0, 3, 8, 11}, &map_diag1a};


Private void do_concept_single_diagonal(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i, rot;
   setup a1;
   setup res1;
   diag_map *map_ptr = &map_diag1b;
   uint32 tbonetest = 0;

   if (!global_tbonetest) {
      result->result_flags = 0;
      result->kind = nothing;
      return;
   }

   if (ss->kind != s4x4) tbonetest = ~0UL;   /* Force error. */

   if (parseptr->concept->value.arg1 & 8) {    /* Concept identified specific people -- check them. */
      if      (global_selectmask == (global_livemask & 0x0909)) map_ptr = map_ptr->other;
      else if (global_selectmask != (global_livemask & 0x9090)) tbonetest = ~0UL;   /* Force error. */
   }
   else {
      if      (global_livemask == 0x2D2D) map_ptr = map_ptr->other;
      else if (global_livemask != 0xD2D2) tbonetest = ~0UL;   /* Force error. */
   }

   for (i=0; i<4; i++) tbonetest |= ss->people[map_ptr->map[i]].id1;

   /* New tbonetest has all the people on the chosen diagonal. */

   if ((tbonetest & 011) == 011) {
      if (parseptr->concept->value.arg1 & 1)
         fail("Designated people are not consistently in a diagonal line.");
      else
         fail("Designated people are not consistently in a diagonal column.");
   }

   rot = (tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;

   if (rot) map_ptr = map_ptr->other;

   ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
   canonicalize_rotation(ss);

   a1 = *ss;
   a1.kind = s1x4;
   a1.rotation = 0;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   for (i=0; i<4; i++) {
      (void) copy_person(&a1, i, ss, map_ptr->map[i]);
      clear_person(ss, map_ptr->map[i]);
   }

   if ((parseptr->concept->value.arg1 & 7) == 3)
      a1.cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   update_id_bits(&a1);
   impose_assumption_and_move(&a1, &res1);

   *result = *ss;

   if (res1.kind != s1x4)
      fail("The call must go back to a 1x4.");

   /* The rotation is either 0 or 1. */
   if (res1.rotation != 0) {
      for (i=0; i<4; i++) {
         install_rot(result, map_ptr->othermap[i], &res1, i, 011);
      }
   }
   else {
      for (i=0; i<4; i++) {
         install_person(result, map_ptr->map[i], &res1, i);
      }
   }

   result->result_flags = res1.result_flags;
   result->rotation -= rot;   /* Flip the setup back. */
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

   if (parseptr->concept->value.arg2) {
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
   int rot = (global_tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;

   /* If this was quadruple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was quadruple
      lines, we forbid it. */

   if (parseptr->concept->value.arg1 & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (parseptr->concept->value.arg1 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (ss->kind == s4x4) {
      ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
      canonicalize_rotation(ss);
      divided_setup_move(ss, map_lists[s1x4][3]->f[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
      result->rotation -= rot;   /* Flip the setup back. */
   }
   else if (ss->kind == s1x16) {
      if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         else                      fail("There are no columns of 4 here.");
      }
   
      divided_setup_move(ss, map_lists[s1x4][3]->f[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
   }
   else
      fail("Must have a 4x4 or 1x16 setup for this concept.");
}


Private void do_concept_quad_lines_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This can only be standard for together/apart/clockwise/counterclockwise,
      not for forward/back/left/right, because we look at individual facing directions to
      determine which other line/column the people in the center lines/columns must work in. */

   int m1, m2, m3;

   int cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward (lines) or left (cols)   : 0
      backward (lines) or right (cols) : 2
      clockwise                        : 8
      counterclockwise                 : 9
      together (must be end-to-end)    : 10
      apart (must be end-to-end)       : 11 */

   int linesp = parseptr->concept->value.arg2;

   /* If this was quadruple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was quadruple
      lines, we forbid it. */

   if (linesp & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (linesp == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (cstuff >= 10) {
      if (ss->kind != s1x16) fail("Must have a 1x16 setup for this concept.");
   
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
   
      if (linesp & 1) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      }
      else {
         if (!(global_tbonetest & 1)) fail("There are no columns of 4 here.");
      }
   
      if (cstuff == 10) {    /* Working together end-to-end. */
         m1 = 0xCF; m2 = 0xCC; m3 = 0xFC;
      }
      else {                 /* Working apart end-to-end. */
         m1 = 0x3F; m2 = 0x33; m3 = 0xF3;
      }
   
      overlapped_setup_move(ss, map_lists[s1x8][2]->f[MPKIND__OVERLAP][0],
            m1, m2, m3, result);
   }
   else {
      int i, rot, tbonetest;

      if (ss->kind != s4x4) fail("Must have a 4x4 setup to do this concept.");
   
      if (cstuff >= 8)
         tbonetest = global_tbonetest;
      else {
         tbonetest = 0;
         for (i=0; i<16; i++) tbonetest |= ss->people[i].id1;
      }
   
      if ((tbonetest & 011) == 011) fail("Sorry, can't do this from T-bone setup.");
      rot = (tbonetest ^ linesp ^ 1) & 1;
      ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
      canonicalize_rotation(ss);
   
      /* Initially assign the centers to the upper (m2 or m3) group. */
      m1 = 0xF0; m2 = 0xF0; m3 = 0xFF;
   
      /* Look at the center 8 people and put each one in the correct group. */
   
      if (cstuff == 8) {
         m1 = 0xFC; m2 = 0xCC; m3 = 0xCF;   /* clockwise */
      }
      else if (cstuff == 9) {
         m1 = 0xF3; m2 = 0x33; m3 = 0x3F;   /* counterclockwise */
      }
      else {                        /* forward/back/left/right */
         if ((ss->people[10].id1 ^ cstuff) & 2) { m2 |= 0x01; m3 &= ~0x80; };
         if ((ss->people[15].id1 ^ cstuff) & 2) { m2 |= 0x02; m3 &= ~0x40; };
         if ((ss->people[3].id1  ^ cstuff) & 2) { m2 |= 0x04; m3 &= ~0x20; };
         if ((ss->people[1].id1  ^ cstuff) & 2) { m2 |= 0x08; m3 &= ~0x10; };
         if ((ss->people[9].id1  ^ cstuff) & 2) { m1 |= 0x01; m2 &= ~0x80; };
         if ((ss->people[11].id1 ^ cstuff) & 2) { m1 |= 0x02; m2 &= ~0x40; };
         if ((ss->people[7].id1  ^ cstuff) & 2) { m1 |= 0x04; m2 &= ~0x20; };
         if ((ss->people[2].id1  ^ cstuff) & 2) { m1 |= 0x08; m2 &= ~0x10; };
      }
   
      overlapped_setup_move(ss, map_lists[s2x4][2]->f[MPKIND__OVERLAP][1],
         m1, m2, m3, result);
      result->rotation -= rot;   /* Flip the setup back. */
   }
}


Private void do_concept_parallelogram(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_thing *map_ptr;

   if (ss->kind == s2x6) {
      if (global_livemask == 07474) map_ptr = map_lists[s2x4][0]->f[MPKIND__OFFS_R_HALF][0];
      else if (global_livemask == 01717) map_ptr = map_lists[s2x4][0]->f[MPKIND__OFFS_L_HALF][0];
      else fail("Can't find a parallelogram.");
   }
   else if (ss->kind == s2x8) {
      warn(warn__full_pgram);
      if (global_livemask == 0xF0F0) map_ptr = map_lists[s2x4][0]->f[MPKIND__OFFS_R_FULL][0];
      else if (global_livemask == 0x0F0F) map_ptr = map_lists[s2x4][0]->f[MPKIND__OFFS_L_FULL][0];
      else fail("Can't find a parallelogram.");
   }
   else
      fail("Can't do parallelogram concept from this position.");

   divided_setup_move(ss, map_ptr, phantest_ok, TRUE, result);
}


Private void do_concept_quad_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff, m1, m2, m3;

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
      counterclockwise : 9 */

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

      m1 = 0xC3 ; m2 = 0xC3; m3 = 0xFF;
      cstuff <<= 2;

      /* Look at the center 8 people and put each one in the correct group. */

      if (((ss->people[2].id1  + 6) ^ cstuff) & 8) { m1 |= 0x04; m2 &= ~0x01; };
      if (((ss->people[3].id1  + 6) ^ cstuff) & 8) { m1 |= 0x08; m2 &= ~0x02; };
      if (((ss->people[4].id1  + 6) ^ cstuff) & 8) { m2 |= 0x04; m3 &= ~0x01; };
      if (((ss->people[5].id1  + 6) ^ cstuff) & 8) { m2 |= 0x08; m3 &= ~0x02; };
      if (((ss->people[10].id1 + 6) ^ cstuff) & 8) { m2 |= 0x10; m3 &= ~0x40; };
      if (((ss->people[11].id1 + 6) ^ cstuff) & 8) { m2 |= 0x20; m3 &= ~0x80; };
      if (((ss->people[12].id1 + 6) ^ cstuff) & 8) { m1 |= 0x10; m2 &= ~0x40; };
      if (((ss->people[13].id1 + 6) ^ cstuff) & 8) { m1 |= 0x20; m2 &= ~0x80; };
   }
   else if (cstuff == 6) {   /* Working together. */
      m1 = 0xE7 ; m2 = 0x66; m3 = 0x7E;
   }
   else if (cstuff == 7) {   /* Working apart. */
      m1 = 0xDB ; m2 = 0x99; m3 = 0xBD;
   }
   else if (cstuff == 8) {   /* Working clockwise. */
      m1 = 0xF3 ; m2 = 0x33; m3 = 0x3F;
   }
   else {                    /* Working counterclockwise. */
      m1 = 0xCF ; m2 = 0xCC; m3 = 0xFC;
   }

   overlapped_setup_move(ss, map_lists[s2x4][2]->f[MPKIND__OVERLAP][0], m1, m2, m3, result);
}


Private void do_concept_triple_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int m1, m2;

   if (ss->kind != s3dmd) fail("Must have a triple diamond setup to do this concept.");
   if ((ss->people[1].id1 | ss->people[7].id1) & 010) fail("Can't tell where points of center diamond should work.");

   m1 = 0xE9; m2 = 0xBF;

   /* Look at the center diamond points and put each one in the correct group. */

   if (ss->people[1].id1 & 2) { m1 |= 0x02; m2 &= ~0x01; }
   if (ss->people[7].id1 & 2) { m1 |= 0x10; m2 &= ~0x20; }

   overlapped_setup_move(ss, map_lists[s_qtag][1]->f[MPKIND__OVERLAP][0], m1, m2, 0, result);
}


Private void do_concept_quad_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int m1, m2, m3;

   if (ss->kind != s4dmd) fail("Must have a quadruple diamond setup to do this concept.");
   if ((ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1) & 010)
      fail("Can't tell where points of center diamonds should work.");

   m1 = 0xE9; m2 = 0xA9; m3 = 0xBF;

   /* Look at the center diamond points and put each one in the correct group. */

   if (ss->people[1].id1 & 2) { m1 |= 0x02; m2 &= ~0x01; }
   if (ss->people[2].id1 & 2) { m2 |= 0x02; m3 &= ~0x01; }
   if (ss->people[9].id1 & 2) { m2 |= 0x10; m3 &= ~0x20; }
   if (ss->people[10].id1 & 2) { m1 |= 0x10; m2 &= ~0x20; }

   overlapped_setup_move(ss, map_lists[s_qtag][2]->f[MPKIND__OVERLAP][0], m1, m2, m3, result);
}


Private void do_concept_triple_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff, m1, m2;

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

   overlapped_setup_move(ss, map_lists[s2x4][1]->f[MPKIND__OVERLAP][0], m1, m2, 0, result);
}


Private void do_concept_triple_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   int mapidx;

   /* If this was triple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was triple
      lines, we forbid it. */

   if (ss->kind == s3x4)
      mapidx = 1;
   else if (ss->kind == s1x12)
      mapidx = 0;
   else
      fail("Must have a 3x4 or 1x12 setup for this concept.");

   if (parseptr->concept->value.arg1 & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (parseptr->concept->value.arg1 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      else                      fail("There are no columns of 4 here.");
   }

   divided_setup_move(ss, map_lists[s1x4][2]->f[MPKIND__SPLIT][mapidx], phantest_ok, TRUE, result);
}




Private void do_concept_triple_lines_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This can only be standard for together/apart/clockwise/counterclockwise,
      not for forward/back/left/right, because we look at individual facing directions to
      determine which other line/column the people in the center lines/columns must work in. */

   int m1, m2;

   int cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward (lines) or left (cols)   : 0
      backward (lines) or right (cols) : 2
      clockwise                        : 8
      counterclockwise                 : 9
      together (must be end-to-end)    : 10
      apart (must be end-to-end)       : 11 */

   int linesp = parseptr->concept->value.arg2;

   /* If this was triple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was triple
      lines, we forbid it. */

   if (linesp & 1)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (linesp == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

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
         m1 = 0xCF; m2 = 0xFC;
      }
      else {                 /* Working apart. */
         m1 = 0x3F; m2 = 0xF3;
      }
   
      overlapped_setup_move(ss, map_lists[s1x8][1]->f[MPKIND__OVERLAP][0],
            m1, m2, 0, result);
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
   
      /* Initially assign the centers to the upper (m2) group. */
      m1 = 0xF0; m2 = 0xFF;
   
      /* Look at the center line people and put each one in the correct group. */
   
      if (cstuff == 8) {
         m1 = 0xFC; m2 = 0xCF;   /* clockwise */
      }
      else if (cstuff == 9) {
         m1 = 0xF3; m2 = 0x3F;   /* counterclockwise */
      }
      else {                        /* forward/back/left/right */
         if ((ss->people[10].id1 ^ cstuff) & 2) { m2 &= ~0x80 ; m1 |= 0x1; };
         if ((ss->people[11].id1 ^ cstuff) & 2) { m2 &= ~0x40 ; m1 |= 0x2; };
         if ((ss->people[5].id1  ^ cstuff) & 2) { m2 &= ~0x20 ; m1 |= 0x4; };
         if ((ss->people[4].id1  ^ cstuff) & 2) { m2 &= ~0x10 ; m1 |= 0x8; };
      }
   
      overlapped_setup_move(ss, map_lists[s2x4][1]->f[MPKIND__OVERLAP][1],
         m1, m2, 0, result);
   }
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
   int cstuff, m1, m2, q;
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

   overlapped_setup_move(ss, map_ptr, m1, m2, 0, result);
}


Private void do_concept_grand_working(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff, tbonetest;
   int m1, m2, m3;
   map_thing *the_map;

   cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward    : 0
      left       : 1
      back       : 2
      right      : 3
      as centers : 8
      as ends    : 9 */

   if (cstuff < 4) {      /* Working forward/back/right/left. */
      if (ss->kind != s2x4) fail("Must have a 2x4 setup for this concept.");

      tbonetest = ss->people[1].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[6].id1;
      if ((tbonetest & 010) && (!(cstuff & 1))) fail("Must indicate left/right.");
      if ((tbonetest & 01) && (cstuff & 1)) fail("Must indicate forward/back.");

      /* Look at the center 4 people and put each one in the correct group. */

      m1 = 0x9; m2 = 0x9; m3 = 0xF;
      cstuff <<= 2;

      if (((ss->people[1].id1 + 6) ^ cstuff) & 8) { m1 |= 0x2 ; m2 &= ~0x1; };
      if (((ss->people[2].id1 + 6) ^ cstuff) & 8) { m2 |= 0x2 ; m3 &= ~0x1; };
      if (((ss->people[5].id1 + 6) ^ cstuff) & 8) { m2 |= 0x4 ; m3 &= ~0x8; };
      if (((ss->people[6].id1 + 6) ^ cstuff) & 8) { m1 |= 0x4 ; m2 &= ~0x8; };

      the_map = map_lists[s2x2][2]->f[MPKIND__OVERLAP][0];
   }
   else {      /* Working as-ends or as-centers. */
      if (ss->kind != s1x8) fail("May not specify as-ends/as-centers here.");

      /* Put each of the center 4 people in the correct group, no need to look. */

      if (cstuff & 1) {
         m1 = 0x7; m2 = 0x5; m3 = 0xD;
      }
      else {
         m1 = 0xB; m2 = 0xA; m3 = 0xE;
      }

      the_map = map_lists[s1x4][2]->f[MPKIND__OVERLAP][0];
   }

   overlapped_setup_move(ss, the_map, m1, m2, m3, result);
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
   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* The values of arg2 are:
      CMD_MISC__VERIFY_DMD_LIKE   "diamonds" -- require diamond-like, i.e. centers in some kind of line, ends are line-like.
      CMD_MISC__VERIFY_1_4_TAG    "1/4 tags" -- centers in some kind of line, ends are a couple looking in (includes 1/4 line, etc.)
                                      If this isn't specific enough for you, use the "ASSUME LEFT 1/4 LINES" concept, or whatever.
      CMD_MISC__VERIFY_3_4_TAG    "3/4 tags" -- centers in some kind of line, ends are a couple looking out (includes 3/4 line, etc.)
                                      If this isn't specific enough for you, use the "ASSUME LEFT 3/4 LINES" concept, or whatever.
      CMD_MISC__VERIFY_QTAG_LIKE  "general 1/4 tags" -- all facing same orientation -- centers in some kind of line, ends are column-like.
      0                           "diamond spots" -- any facing direction is allowed. */

   if (ss->kind != s4dmd) fail("Must have a quadruple diamond/quarter-tag setup for this concept.");

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_divided_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* See "do_concept_do_phantom_diamonds" for meaning of arg2. */

   if (ss->kind != s4x6 || (global_livemask & 0x02D02D) != 0)
      fail("Must have a divided diamond or 1/4 tag setup for this concept.");

   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_distorted(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   distorted_move(ss, parseptr, (disttest_kind) parseptr->concept->value.arg1, result);
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

   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_1x6(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s2x6) fail("Must have a 2x6 setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (parseptr->concept->value.arg3 == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no lines of 6 here.");
      else                      fail("There are no columns of 6 here.");
   }

   divided_setup_move(ss, map_lists[s1x6][1]->f[MPKIND__SPLIT][1], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_1x8(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind == s2x8) {
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if (parseptr->concept->value.arg3 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no grand lines here.");
         else                      fail("There are no grand columns here.");
      }

      divided_setup_move(ss, map_lists[s1x8][1]->f[MPKIND__SPLIT][1], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
   }
   else if (ss->kind == s1x16) {
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

      if (parseptr->concept->value.arg3 == 3)
         ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

      if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no grand lines here.");
         else                      fail("There are no grand columns here.");
      }

      divided_setup_move(ss, map_lists[s1x8][1]->f[MPKIND__SPLIT][0], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
   }
   else
      fail("Must have a 2x8 or 1x16 setup for this concept.");
}


Private void do_concept_once_removed(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_thing *the_map;

   switch (ss->kind) {
      case s2x4:
         the_map = map_lists[s2x2][1]->f[MPKIND__REMOVED][0];
         break;
      case s1x8:
         the_map = map_lists[s1x4][1]->f[MPKIND__REMOVED][0];
         break;
      case s1x4:
         the_map = map_lists[s1x2][1]->f[MPKIND__REMOVED][0];
         break;
      case s_rigger:
         the_map = map_lists[sdmd][1]->f[MPKIND__REMOVED][0];
         break;
      case s_bone:    /* This is for you, Clark. */
         the_map = map_lists[s_trngl4][1]->f[MPKIND__REMOVED][1];
         break;
      case s_ptpd:    /* This is for you, Clark. */
         the_map = &map_p8_tgl4;
         break;
      case s_qtag:
         the_map = map_lists[sdmd][1]->f[MPKIND__REMOVED][1];
         break;
      default:
         fail("Can't do 'once removed' from this setup.");
   }

   if (parseptr->concept->value.arg1) {
      /* If this is the "once removed diamonds" concept, we only allow diamonds. */
      if (the_map->inner_kind != sdmd)
         fail("There are no once removed diamonds here.");
   }
   else {
      /* If this is just the "once removed" concept, we do NOT allow the splitting of a
         quarter-tag into diamonds -- although there is only one splitting axis than
         will work, it is not generally accepted usage. */
      if (ss->kind == s_qtag)
         fail("You must use the \"once removed diamonds\" concept here.");
   }

   divided_setup_move(ss, the_map, phantest_ok, TRUE, result);
}



Private void do_concept_diagnose(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   char junk[200];

   move(ss, FALSE, result);

   sprintf(junk, "Command flags: 0x%08X, result flags: 0x%08X.",
         ss->cmd.cmd_misc_flags, result->result_flags);
   fail(junk);
}


Private void do_concept_old_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   move(ss, FALSE, result);

   if (!((((result->rotation & 1) + 1) * RESULTFLAG__SPLIT_AXIS_BIT) & result->result_flags))
      fail("Stretch call was not a 4 person call divided along stretching axis.");

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


Private void do_concept_new_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_thing *maps;
   setup tempsetup = *ss;

   if (     (parseptr->concept->value.arg1 == 1 && tempsetup.kind != s2x4) ||
            (parseptr->concept->value.arg1 == 2 && tempsetup.kind != s1x8))
      fail("Not in correct pformation for this concept.");

   tempsetup.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   if (tempsetup.kind == s2x4) {
      swap_people(&tempsetup, 1, 2);
      swap_people(&tempsetup, 5, 6);
      maps = map_lists[s2x2][1]->f[MPKIND__SPLIT][0];
   }
   else if (tempsetup.kind == s1x8) {
      swap_people(&tempsetup, 3, 6);
      swap_people(&tempsetup, 2, 7);
      maps = map_lists[s1x4][1]->f[MPKIND__SPLIT][0];
   }
   else
      fail("Stretched setup call didn't start in 2x4 or 1x8 setup.");

   divided_setup_move(&tempsetup, maps, phantest_ok, TRUE, result);
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



/* This returns TRUE if it can't do it because the assumption isn't specific enough.
   In such a case, the call was not executed.  If the user had said "with active phantoms",
   that is an error.  But if we are only doing this because the automatic active phantoms
   switch is on, we will just ignore it. */

Private long_boolean fill_active_phantoms_and_move(
   setup *ss,
   restriction_thing *restr_thing_ptr,
   setup *result)
{
   int phantom_count = 0;
   int i;
   uint32 pdir, qdir, pdirodd, qdirodd;
   uint32 bothp;
   uint32 dirtest1 = 0;
   uint32 dirtest2 = 0;

   if (restr_thing_ptr && restr_thing_ptr->ok_for_assume) {
      switch (restr_thing_ptr->check) {
         case chk_wave:

            /* Figure out handedness of live people. */
         
            for (i=0; i<restr_thing_ptr->size; i++) {
               int p = restr_thing_ptr->map1[i];
               int q = restr_thing_ptr->map2[i];
         
               if (ss->people[p].id1) { dirtest1 |= ss->people[p].id1;       dirtest2 |= (ss->people[p].id1 ^ 2); }
               if (ss->people[q].id1) { dirtest1 |= (ss->people[q].id1 ^ 2); dirtest2 |= ss->people[q].id1; }
            }
         
            if (dirtest1 == 0)
               fail("Need live person to determine handedness.");
         
            bothp = ss->cmd.cmd_assume.assump_both;
         
            if (ss->cmd.cmd_assume.assump_col & 4) {
               if ((dirtest1 & 077) == 011)
                  { pdir = d_north; qdir = d_south; }
               else if ((dirtest2 & 077) == 011)
                  { pdir = d_south; qdir = d_north; }
               else
                  fail("Live people are not consistent.");

               pdirodd = rotcw(pdir); qdirodd = rotcw(qdir);
            }
            else if (ss->cmd.cmd_assume.assump_col == 1) {
               if ((dirtest1 & 077) == 001 && ((bothp & 2) == 0))
                  { pdir = d_east; qdir = d_west; }
               else if ((dirtest2 & 077) == 001 && ((bothp & 1) == 0))
                  { pdir = d_west; qdir = d_east; }
               else
                  fail("Live people are not consistent.");
               pdirodd = pdir; qdirodd = qdir;
            }
            else {
               if ((dirtest1 & 077) == 010 && ((bothp & 2) == 0))
                  { pdir = d_north; qdir = d_south; }
               else if ((dirtest2 & 077) == 010 && ((bothp & 1) == 0))
                  { pdir = d_south; qdir = d_north; }
               else
                  fail("Live people are not consistent.");
               pdirodd = pdir; qdirodd = qdir;
            }
         
            for (i=0; i<restr_thing_ptr->size; i++) {
               int p = restr_thing_ptr->map1[i];
               int q = restr_thing_ptr->map2[i];
         
               if (!ss->people[p].id1) {
                  if (phantom_count >= 16)
                     fail("Too many phantoms.");
                  ss->people[p].id1 = ((i&1) ? pdirodd : pdir) | BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[p].id2 = 0;
               }
               else if (ss->people[p].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
         
               if (!ss->people[q].id1) {
                  if (phantom_count >= 16)
                     fail("Too many phantoms.");
                  ss->people[q].id1 = ((i&1) ? qdirodd : qdir) | BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[q].id2 = 0;
               }
               else if (ss->people[q].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
            }

            goto finished_filling_in;
         case chk_1_group:
            /* Figure out handedness of live people. */
         
            for (i=0; i<restr_thing_ptr->size; i++) {
               int p = restr_thing_ptr->map1[i];
         
               if (ss->people[p].id1) { dirtest1 |= ss->people[p].id1; dirtest2 |= (ss->people[p].id1 ^ 2); }
            }
         
            if (dirtest1 == 0)
               fail("Need live person to determine handedness.");
         
            if ((dirtest1 & 077) == 010)
               { pdir = d_north; }
            else if ((dirtest2 & 077) == 010)
               { pdir = d_south; }
            else
               fail("Live people are not consistent.");
         
            for (i=0; i<restr_thing_ptr->size; i++) {
               int p = restr_thing_ptr->map1[i];
         
               if (!ss->people[p].id1) {
                  if (phantom_count >= 16)
                     fail("Too many phantoms.");
                  ss->people[p].id1 = pdir | BIT_ACT_PHAN | ((phantom_count++) << 6);
                  ss->people[p].id2 = 0;
               }
               else if (ss->people[p].id1 & BIT_ACT_PHAN)
                  fail("Active phantoms may only be used once.");
            }
            goto finished_filling_in;
         case chk_box:
            {
               uint32 j, k, m, z, t;
      
               j = 0; k = 0; m = 0; z = 0;
      
               for (i=0 ; i<4 ; i++) {
                  if ((t = ss->people[i].id1) != 0) {
                     j |= (t^restr_thing_ptr->map1[i]);
                     k |= (t^restr_thing_ptr->map1[i]^2);
                     m |= (t^restr_thing_ptr->map2[i]^3);
                     z |= (t^restr_thing_ptr->map2[i]^1);
                  }
               }
      
               if ((j | k | m | z) == 0)
                  fail("Need live person to determine handedness.");
      
               if ((k&3) && (j&3) && (m&3) && (z&3))
                  fail("Live people are not consistent.");
      
               for (i=0 ; i<4 ; i++) {
                  if (!ss->people[i].id1) {
                     if (phantom_count >= 16)
                        fail("Too many phantoms.");
      
                     pdir = (j&1) ?
                           (d_east ^ ((restr_thing_ptr->map2[i] ^ z) & 2)) :
                           (d_north ^ ((restr_thing_ptr->map1[i] ^ j) & 2));
      
                     ss->people[i].id1 = pdir | BIT_ACT_PHAN | ((phantom_count++) << 6);
                     ss->people[i].id2 = 0;
                  }
                  else if (ss->people[i].id1 & BIT_ACT_PHAN)
                     fail("Active phantoms may only be used once.");
               }
            }
            goto finished_filling_in;
      }
   }

   return TRUE;   /* We couldn't do it -- the assumption is not specific enough, like "general diamonds". */

   finished_filling_in:

   ss->cmd.cmd_assume.assumption = cr_none;
   move(ss, FALSE, result);

   /* Take out the phantoms. */

   for (i=0; i<=setup_attrs[result->kind].setup_limits; i++) {
      if (result->people[i].id1 & BIT_ACT_PHAN)
         result->people[i].id1 = 0;
   }

   return FALSE;
}



Private void move_perhaps_with_active_phantoms(
   setup *ss,
   restriction_thing *restr_thing_ptr,
   setup *result)
{
   if (using_active_phantoms) {
      if (fill_active_phantoms_and_move(ss, restr_thing_ptr, result)) {
         /* Active phantoms couldn't be used.  Just do the call the way it is.
            This does not count as a use of active phantoms, so don't set the flag. */
         move(ss, FALSE, result);
      }
      else
         result->result_flags |= RESULTFLAG__ACTIVE_PHANTOMS_ON;
   }
   else {
      move(ss, FALSE, result);
      result->result_flags |= RESULTFLAG__ACTIVE_PHANTOMS_OFF;
   }
}




extern void impose_assumption_and_move(setup *ss, setup *result)
{
   if (ss->cmd.cmd_misc_flags & CMD_MISC__VERIFY_MASK) {
      assumption_thing t;

      /* **** actually, we want to allow the case of "assume waves" already in place. */
      if (ss->cmd.cmd_assume.assumption != cr_none)
         fail("Redundant or conflicting assumptions.");

      t.assump_col = 0;
      t.assump_both = 0;
      t.assump_cast = ss->cmd.cmd_assume.assump_cast;

      switch (ss->cmd.cmd_misc_flags & CMD_MISC__VERIFY_MASK) {
         case CMD_MISC__VERIFY_WAVES:     t.assumption = cr_wave_only;    break;
         case CMD_MISC__VERIFY_DMD_LIKE:  t.assumption = cr_diamond_like; break;
         case CMD_MISC__VERIFY_QTAG_LIKE: t.assumption = cr_qtag_like;    break;
         case CMD_MISC__VERIFY_1_4_TAG:   t.assumption = cr_gen_1_4_tag;  break;
         case CMD_MISC__VERIFY_3_4_TAG:   t.assumption = cr_gen_3_4_tag;  break;
         case CMD_MISC__VERIFY_LINES:     t.assumption = cr_all_ns;       break;
         case CMD_MISC__VERIFY_COLS:      t.assumption = cr_all_ew;       break;
         default:
            fail("Unknown assumption verify code.");
      }

      ss->cmd.cmd_assume = t;
      ss->cmd.cmd_misc_flags &= ~CMD_MISC__VERIFY_MASK;
      move_perhaps_with_active_phantoms(ss, check_restriction(ss, t, 99), result);
   }
   else
      move(ss, FALSE, result);
}




Private void do_concept_assume_waves(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   assumption_thing t;
   restriction_thing *restr_thing_ptr;
   long_boolean no_phan_error = FALSE;
   setup sss;
   int i;

   /* "Assume normal casts" is special. */

   if (parseptr->concept->value.arg1 == cr_alwaysfail) {
      if (ss->cmd.cmd_assume.assump_cast)
         fail("Redundant or conflicting assumptions.");
      ss->cmd.cmd_assume.assump_cast = 1;
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
   t.assump_cast = ss->cmd.cmd_assume.assump_cast;

   /* Need to check any pre-existing assumption. */

   if (ss->cmd.cmd_assume.assumption == cr_none) ;   /* If no pre-existing assumption, OK. */
   else {     /* We have something, and must check carefully. */
      /* First, an exact match is allowed. */
      if (         ss->cmd.cmd_assume.assumption == t.assumption &&
                   ss->cmd.cmd_assume.assump_col == t.assump_col &&
                   ss->cmd.cmd_assume.assump_both == t.assump_both) ;

      /* We also allow certain tightenings of existing assumptions. */

      else if (     (t.assumption == cr_jleft ||
                     t.assumption == cr_jright ||
                     t.assumption == cr_ijleft ||
                     t.assumption == cr_ijright)
                                 &&
                    ((ss->cmd.cmd_assume.assumption == cr_diamond_like && t.assump_col == 4) ||
                     (ss->cmd.cmd_assume.assumption == cr_qtag_like && t.assump_col == 0) ||
                     (ss->cmd.cmd_assume.assumption == cr_gen_1_4_tag && t.assump_both == 2) ||
                     (ss->cmd.cmd_assume.assumption == cr_gen_3_4_tag && t.assump_both == 1))) ;
      else
         fail("Redundant or conflicting assumptions.");
   }

   ss->cmd.cmd_assume = t;

   /* The restrictions mean different things in different setups.  In some setups, they
      mean things that are unsuitable for the "assume" concept.  In some setups they
      take no action at all.  So we must check the setups on a case-by-case basis. */

   if (t.assumption == cr_jleft || t.assumption == cr_jright || t.assumption == cr_ijleft || t.assumption == cr_ijright) {
      /* These assumptions work independently of the "assump_col" number. */
      switch (ss->kind) {
         case s1x4: goto fudge_from_1x4;
         case s3x4: goto fudge_from_3x4;
         case s_qtag: goto check_it;
      }
   }
   else if (t.assump_col == 2) {
      /* This is a special assumption -- "assume miniwaves", "assume normal boxes", or "assume inverted boxes". */

      if (t.assumption == cr_wave_only) {
         switch (ss->kind) {     /* "assume miniwaves" or "assume normal boxes" */
            case s1x2: goto check_it;
            case s2x2: goto check_it;
            case s2x4: goto check_it;
         }
      }
      else if (t.assumption == cr_magic_only) {
         switch (ss->kind) {     /* "assume inverted boxes" */
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
      else if (t.assumption == cr_li_lo) {
         switch (ss->kind) {     /* "assume 8 chain" or "assume trade by" */
            case s2x4: goto check_it;
         }
      }
   }
   else {
      /* This is a "line-like" assumption. */

      if (t.assumption == cr_wave_only) {
         switch (ss->kind) {     /* "assume waves" */
            case s2x4: case s3x4:  case s2x8:  case s2x6:
            case s1x8: case s1x10: case s1x12: case s1x14: case s1x16:
            case s1x6: case s1x4: goto check_it;
         }
      }
      else if (t.assumption == cr_2fl_only) {
         switch (ss->kind) {     /* "assume two-faced lines" */
            case s2x4:  case s3x4:  case s1x8:  case s1x4: goto check_it;
         }
      }
      else if (t.assumption == cr_couples_only) {
         switch (ss->kind) {     /* "assume couples" */
            case s2x2:  case s1x2:  case s1x8:  case s1x16:  case s2x8:  case s1x4: goto check_it;
         }
      }
      else if (t.assumption == cr_magic_only) {
         switch (ss->kind) {     /* "assume inverted lines" */
            case s2x4:  case s1x4: goto check_it;
         }
      }
      else if (t.assumption == cr_1fl_only) {
         switch (ss->kind) {     /* "assume one-faced lines" */
            case s1x4: goto check_it;
         }
      }
      else if (t.assumption == cr_li_lo) {
         switch (ss->kind) {     /* "assume lines in" or "assume lines out" */
            case s2x4: goto check_it;
         }
      }
      else if (t.assumption == cr_diamond_like) {
         switch (ss->kind) {
            case s1x4: goto fudge_from_1x4;
            case s3x4: goto fudge_from_3x4;
            case s_qtag: goto check_it;
         }
      }
      else if (t.assumption == cr_qtag_like) {
         switch (ss->kind) {
            case s1x4: goto fudge_from_1x4;
            case s3x4: goto fudge_from_3x4;
            case s_qtag: goto check_it;
         }
      }
   }

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

   restr_thing_ptr = check_restriction(ss, ss->cmd.cmd_assume, 99);

   move_perhaps_with_active_phantoms(ss, restr_thing_ptr, result);
}




Private void do_concept_active_phantoms(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   restriction_thing *restr_thing_ptr;

   if (ss->cmd.cmd_assume.assump_cast)
      fail("Don't use \"active phantoms\" with \"assume normal casts\".");

   restr_thing_ptr = get_restriction_thing(ss->kind, ss->cmd.cmd_assume);

   if (fill_active_phantoms_and_move(ss, restr_thing_ptr, result))
      fail("This assumption is not specific enough to fill in active phantoms.");
}


Private void do_concept_central(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (parseptr->concept->value.arg1 == CMD_MISC2__CTR_END_INVERT) {
      /* If this is "invert", just flip the bit.  They can stack, of course. */
      ss->cmd.cmd_misc2_flags ^= CMD_MISC2__CTR_END_INVERT;
   }
   else {
      uint32 this_concept = parseptr->concept->value.arg1;

      /* Otherwise, if the "invert" bit was on, we assume that means that the
         user really wanted "invert snag" or whatever. */

      if (     (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) == CMD_MISC2__CTR_END_INVERT &&
               !(this_concept & CMD_MISC2__CTR_END_INV_CONC)) {
         ss->cmd.cmd_misc2_flags &= ~CMD_MISC2__CTR_END_INVERT;   /* Take out the "invert" bit". */
         this_concept |= CMD_MISC2__CTR_END_INV_CONC;             /* Put in the "this concept is inverted" bit. */
      }
      else if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Only one of central, snag, and mystic is allowed.");

      if ((parseptr->concept->value.arg1 & CMD_MISC2__CTR_END_KMASK) == CMD_MISC2__CENTRAL_PLAIN) {
         if (setup_attrs[ss->kind].setup_limits == 7)
            ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT;
         else if (setup_attrs[ss->kind].setup_limits != 3)
            fail("Need a 4 or 8 person setup for this.");
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
   uint32 restraint = ss->cmd.cmd_misc_flags;

   ss->cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;
   tempsetup = *ss;

   if (tempsetup.cmd.cmd_final_flags & INHERITFLAG_REVERSE) {
      if (reverseness) fail("Redundant 'REVERSE' modifiers.");
      reverseness = 1;
   }

   tempsetup.cmd.cmd_final_flags &= ~INHERITFLAG_REVERSE;
   if (tempsetup.cmd.cmd_final_flags)   /* We don't allow other flags, like "cross". */
      fail("Illegal modifier before \"crazy\".");

   cmd = tempsetup.cmd;    /* We will modify these flags, and, in any case, we need
                              to rematerialize them at each step. */

   /* If we didn't do this check, and we had a 1x4, the "do it on each side"
      stuff would just do it without splitting or thinking anything was unusual,
      while the "do it in the center" code would catch it at present, but might
      not in the future if we add the ability of the concentric schema to mean
      pick out the center 2 from a 1x4.  In any case, if we didn't do this
      check, "1/4 reverse crazy bingo" would be legal from a 2x2. */

   if (setup_attrs[tempsetup.kind].setup_limits != 7)
      fail("Need an 8-person setup for this.");

   if (parseptr->concept->value.arg2)
      craziness = parseptr->number;
   else
      craziness = 4;

   i = 0;   /* The start point. */
   highlimit = craziness;   /* The end point. */

   /* If fractions come in but the craziness is restrained, just pass the fractions on.
      This is what makes "random crazy mix" work.  The "random" concept wants to reach through
      the crazy concept and do the numbered parts of the mix, not the numbered parts of the
      craziness.  But if the craziness is unrestrained, which is the usual case, we act on
      the fractions.  This makes "interlace crazy this with reverse crazy that" work. */

   if (cmd.cmd_frac_flags && (restraint & CMD_MISC__RESTRAIN_CRAZINESS) == 0) {

      /* The fractions were meant for us, not the subject call. */

      int this_part = ((cmd.cmd_frac_flags >> 16) & 0xF) - 1;

      switch (cmd.cmd_frac_flags & 0xF0FFFF) {
         case 0x00000111:
            /* Request is to do just part this_part+1. */
            if (this_part >= 0) {
               i = this_part;
               /* We are taking the part number indication. */
               highlimit = this_part+1;          /* This will make us do just the selected part. */
               finalresultflags |= RESULTFLAG__PARTS_ARE_KNOWN;
               if (highlimit == craziness) finalresultflags |= RESULTFLAG__DID_LAST_PART;
               cmd.cmd_frac_flags = 0;   /* No fractions for subject, we have consumed them. */
            }
            break;
         case 0x00400111:
            /* Request is to do everything up through part this_part+1. */
            if (this_part >= 0) {
               /* We are taking the part number indication. */
               highlimit = this_part+1;          /* This will make us do up through the selected part. */
               cmd.cmd_frac_flags = 0;   /* No fractions for subject, we have consumed them. */
            }
            break;
         case 0x00200111:
            /* Request is to do everything strictly after part this_part+1. */
            if (this_part >= 0) {
               /* We are taking the part number indication. */
               i = this_part+1;          /* This will make us do after the selected part. */
               cmd.cmd_frac_flags = 0;   /* No fractions for subject, we have consumed them. */
            }
            break;
         default:
            fail("\"crazy\" is not allowed with fractional or reverse-order concepts.");
      }
   }

   if (highlimit <= i || highlimit > craziness) fail("Illegal fraction for \"crazy\".");

   for ( ; i<highlimit; i++) {
      tempsetup.cmd = cmd;    /* Get a fresh copy of the command. */

      if ((i ^ reverseness) & 1) {
         /* Do it in the center. */
         concentric_move(&tempsetup, &tempsetup.cmd, (setup_command *) 0,
                  schema_concentric, 0, 0, result);
      }
      else {
         /* Do it on each side. */
         tempsetup.cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT;
         move(&tempsetup, FALSE, result);
      }

      finalresultflags |= result->result_flags;

      tempsetup = *result;
   }

   result->result_flags = finalresultflags;
}


Private void do_concept_fan(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tempsetup;
   uint32 finalresultflags = 0;
   /* This is a huge amount of kludgy stuff shoveled in from a variety of sources.
      It needs to be cleaned up and thought about. */

   final_set new_final_concepts;
   parse_block *parseptrcopy;
   callspec_block *callspec;

   parseptrcopy = process_final_concepts(parseptr->next, TRUE, &new_final_concepts);

   if (new_final_concepts || parseptrcopy->concept->kind > marker_end_of_list)
      fail("Can't do \"fan\" followed by another concept or modifier.");

   callspec = parseptrcopy->call;

   if (!callspec || !(callspec->callflags1 & CFLAG1_CAN_BE_FAN))
      fail("Can't do \"fan\" with this call.");

   /* Step to a wave if necessary.  This is actually only needed for the "yoyo" concept.
      The "fan" concept could take care of itself later.  However, we do them both here. */

   if ((!(ss->cmd.cmd_misc_flags & (CMD_MISC__NO_STEP_TO_WAVE))) &&
         (callspec->callflags1 & (CFLAG1_STEP_TO_WAVE))) {
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;  /* Can only do it once. */
      touch_or_rear_back(ss, FALSE, callspec->callflags1);
   }

   tempsetup = *ss;

   /* Normally, set the fractionalize field to [1 0 1] to start with the second part.
      But if we have been requested to do a specific part number of "fan <call>",
      just add one to the part number and do the call. */

   tempsetup.cmd = ss->cmd;

   if ((tempsetup.cmd.cmd_frac_flags & 0xF0FFFF) == 0x000111 && tempsetup.cmd.cmd_frac_flags != 0x000111)
      tempsetup.cmd.cmd_frac_flags += 0x010000;
   else if (tempsetup.cmd.cmd_frac_flags)
      fail("Sorry, this concept can't be fractionalized this way.");
   else
      tempsetup.cmd.cmd_frac_flags = 0x210111;

   tempsetup.cmd.prior_elongation_bits = 0;
   move(&tempsetup, FALSE, result);
   finalresultflags |= result->result_flags;
   result->result_flags = finalresultflags & ~3;
}



Private void do_concept_stable(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   selector_kind saved_selector, new_selector;
   long_boolean everyone, fractional;
   uint32 directions[32];
   int n, i, rot, howfar, orig_rotation;

   fractional = parseptr->concept->value.arg2;
   everyone = !parseptr->concept->value.arg1;

   howfar = parseptr->number;
   if (fractional && howfar > 4)
      fail("Can't do fractional stable more than 4/4.");

   new_selector = parseptr->selector;
   n = setup_attrs[ss->kind].setup_limits;
   if (n < 0) fail("Sorry, can't do stable starting in this setup.");

   for (i=0; i<=n; i++) {           /* Save current facing directions. */
      uint32 p = ss->people[i].id1;
      if (p & BIT_PERSON) {
         directions[(p >> 6) & 037] = p;
         if (fractional) {
            if (p & STABLE_MASK)
               fail("Sorry, can't nest fractional stable/twosome.");
            ss->people[i].id1 |= STABLE_ENAB | (STABLE_RBIT * howfar);
         }
      }
   }

   orig_rotation = ss->rotation;
   move(ss, FALSE, result);
   rot = ((orig_rotation - result->rotation) & 3) * 011;

   n = setup_attrs[result->kind].setup_limits;
   if (n < 0) fail("Sorry, can't do stable going to this setup.");

   saved_selector = current_selector;
   current_selector = new_selector;

   for (i=0; i<=n; i++) {           /* Restore facing directions of selected people. */
      uint32 p = result->people[i].id1;
      if (p & BIT_PERSON) {
         if (everyone || selectp(result, i)) {
            if (fractional) {
               if (!(p & STABLE_ENAB))
                  fail("fractional stable not supported for this call.");
               result->people[i].id1 = rotperson(p, ((- ((p & (STABLE_VBIT*3)) / STABLE_VBIT)) & 3) * 011);
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

   current_selector = saved_selector;
}


Private void do_concept_checkerboard(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   static veryshort mape[16] = {0, 2, 4, 6, 1, 3, 5, 7, 0, 1, 4, 5, 2, 3, 6, 7};
   static veryshort mapl[16] = {7, 1, 3, 5, 0, 6, 4, 2, 7, 6, 3, 2, 0, 1, 4, 5};
   static veryshort mapb[16] = {1, 3, 5, 7, 0, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1, -1};
   static veryshort mapd[16] = {7, 1, 3, 5, 0, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1, -1};

   int i, rot, offset;
   setup a1;
   setup res1;
   veryshort *map_ptr;
   setup_kind kn = (setup_kind) parseptr->concept->value.arg1;

   clear_people(result);

   if (parseptr->concept->value.arg2 == 1) {
      /* This is "shadow <setup>" */

      setup_command subsid_cmd;

      if (kn != s2x2) fail("Sorry, can only do 'shadow box'.");

      if (ss->kind != s2x4) fail("Must have a 2x4 setup for 'shadow box' concept.");

      subsid_cmd = ss->cmd;
      subsid_cmd.parseptr = (parse_block *) 0;
      subsid_cmd.callspec = base_calls[BASE_CALL_ENDS_SHADOW];
      concentric_move(ss, &ss->cmd, &subsid_cmd, schema_concentric, 0, DFM1_CONC_DEMAND_LINES | DFM1_CONC_FORCE_COLUMNS, result);
      return;
   }
   else {
      /* This is "checker <setup>" */

      if (ss->kind != s2x4) fail("Must have a 2x4 setup for 'checker' concept.");

      if (parseptr->concept->value.arg2 & 8) {
         /* This is "so-and-so preferred for the trade, checkerboard". */
         if (global_selectmask == 0x55)
            offset = 0;
         else if (global_selectmask == 0xAA)
            offset = 4;
         else if (global_selectmask == 0x33)
            offset = 8;
         else if (global_selectmask == 0xCC)
            offset = 12;
         else fail("Can't select these people.");
      }
      else {
         if ((ss->people[0].id1 & d_mask) == d_north && (ss->people[1].id1 & d_mask) != d_north &&
               (ss->people[2].id1 & d_mask) == d_north && (ss->people[3].id1 & d_mask) != d_north &&
               (ss->people[4].id1 & d_mask) == d_south && (ss->people[5].id1 & d_mask) != d_south &&
               (ss->people[6].id1 & d_mask) == d_south && (ss->people[7].id1 & d_mask) != d_south)
            offset = 0;
         else if ((ss->people[0].id1 & d_mask) != d_north && (ss->people[1].id1 & d_mask) == d_north &&
               (ss->people[2].id1 & d_mask) != d_north && (ss->people[3].id1 & d_mask) == d_north &&
               (ss->people[4].id1 & d_mask) != d_south && (ss->people[5].id1 & d_mask) == d_south &&
               (ss->people[6].id1 & d_mask) != d_south && (ss->people[7].id1 & d_mask) == d_south)
            offset = 4;
         else
            fail("Can't identify checkerboard people.");
      }
   
      /* Move the people who simply trade, filling in their roll info. */
   
      if (      (ss->people[mape[0+offset]].id1 & d_mask) != d_north ||
                (ss->people[mape[1+offset]].id1 & d_mask) != d_north ||
                (ss->people[mape[2+offset]].id1 & d_mask) != d_south ||
                (ss->people[mape[3+offset]].id1 & d_mask) != d_south)
            fail("Selected people are not facing out.");

      (void) copy_rot(result, mape[0+offset], ss, mape[1+offset], 022);
      if (result->people[mape[0+offset]].id1) result->people[mape[0+offset]].id1 = (result->people[mape[0+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
      (void) copy_rot(result, mape[1+offset], ss, mape[0+offset], 022);
      if (result->people[mape[1+offset]].id1) result->people[mape[1+offset]].id1 = (result->people[mape[1+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
      (void) copy_rot(result, mape[2+offset], ss, mape[3+offset], 022);
      if (result->people[mape[2+offset]].id1) result->people[mape[2+offset]].id1 = (result->people[mape[2+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
      (void) copy_rot(result, mape[3+offset], ss, mape[2+offset], 022);
      if (result->people[mape[3+offset]].id1) result->people[mape[3+offset]].id1 = (result->people[mape[3+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;

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

      if (map_ptr[offset] < 0) fail("Can't select these people.");
   
      for (i=0; i<4; i++) (void) copy_person(&a1, i, ss, map_ptr[i+offset]);
   }

   a1.kind = kn;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a1.rotation = 0;
   a1.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a1);
   move(&a1, FALSE, &res1);

   /* Look at the rotation coming out of the move.  If the setup is 1x4, we require it to be
      even (no checkerboard lockit allowed).  Otherwise, allow any rotation.  This means
      we allow diamonds that are oriented rather peculiarly! */

   if ((res1.rotation & 1) && res1.kind == s1x4)
      fail("'Checker' call went to 1x4 setup oriented wrong way.");

   rot = res1.rotation * 011;

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

   for (i=0; i<4; i++) (void) copy_rot(result, map_ptr[i+offset], &res1, (i-res1.rotation)&3, rot);
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
   setup_command subsid_cmd = ss->cmd;
   subsid_cmd.parseptr = parseptr->subsidiary_root;

   if (ss->cmd.cmd_final_flags & ~INHERITFLAG_REVERSE)   /* We don't allow other flags, like "cross". */
      fail("Illegal modifier before \"checkpoint\".");

   if (parseptr->concept->value.arg1 || (ss->cmd.cmd_final_flags & INHERITFLAG_REVERSE)) {   /* 0 for normal, 1 for reverse checkpoint. */
      if (parseptr->concept->value.arg1 && (ss->cmd.cmd_final_flags & INHERITFLAG_REVERSE))
         fail("Redundant 'REVERSE' modifiers.");
      ss->cmd.cmd_final_flags &= ~INHERITFLAG_REVERSE;
      subsid_cmd.cmd_final_flags &= ~INHERITFLAG_REVERSE;
      concentric_move(ss, &ss->cmd, &subsid_cmd, schema_rev_checkpoint, 0, 0, result);
   }
   else {
      /* The "dfm_conc_force_otherway" flag forces Callerlab interpretation:
         If checkpointers go from 2x2 to 2x2, this is clear.
         If checkpointers go from 1x4 to 2x2, "dfm_conc_force_otherway" forces
            the Callerlab rule in preference to the "parallel_concentric_end" property
            on the call. */
      concentric_move(ss, &subsid_cmd, &ss->cmd, schema_checkpoint, 0, DFM1_CONC_FORCE_OTHERWAY, result);
   }
}



Private void do_concept_sequential(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int highlimit;
   int subcall_incr;
   int subcall_index;
   int instant_stop;
   long_boolean first_call;
   long_boolean reverse_order;
   long_boolean do_half_of_last_part;

   if (ss->cmd.cmd_frac_flags) {
      fraction_info zzz = get_fraction_info(ss->cmd.cmd_frac_flags, 3*CFLAG1_VISIBLE_FRACTION_BIT, 2);
      reverse_order = zzz.reverse_order;
      do_half_of_last_part = zzz.do_half_of_last_part;
      highlimit = zzz.highlimit;
      if (reverse_order) highlimit = 1-highlimit;
      subcall_index = zzz.subcall_index;
      subcall_incr = reverse_order ? -1 : 1;
      instant_stop = zzz.instant_stop ? subcall_index*subcall_incr+1 : 99;
   }
   else {     /* No fractions. */
      reverse_order = FALSE;
      instant_stop = 99;
      do_half_of_last_part = FALSE;
      highlimit = 2;
      subcall_index = 0;
      subcall_incr = 1;
   }

   first_call = reverse_order ? FALSE : TRUE;

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   for (;;) {
      uint32 save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */

      if (subcall_index*subcall_incr >= highlimit) break;
      if (subcall_index >= 2) fail("The indicated part number doesn't exist.");

      result->cmd = ss->cmd;      /* The call we wish to execute (we will fix it up shortly). */

      if (do_half_of_last_part && subcall_index+1 == highlimit)
         result->cmd.cmd_frac_flags = 0x000112;
      else
         result->cmd.cmd_frac_flags = 0;  /* No fractions to constituent call. */

      /* We don't supply these; they get filled in by the call. */
      result->cmd.cmd_misc_flags &= ~DFM1_CONCENTRICITY_FLAG_MASK;
      if (!first_call) {
         result->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;
         result->cmd.cmd_assume.assumption = cr_none;
      }

      if (subcall_index != 0) result->cmd.parseptr = parseptr->subsidiary_root;
      result->cmd.prior_elongation_bits = save_elongation;
      update_id_bits(result);
      do_call_in_series(result, FALSE, TRUE, FALSE);
      subcall_index += subcall_incr;
      first_call = FALSE;

      /* If we are being asked to do just one part of a call (from cmd_frac_flags),
         exit now.  Also, see if we just did the last part. */

      if (ss->cmd.cmd_frac_flags && instant_stop != 99) {
         /* Check whether we honored the last possible request.  That is,
            whether we did the last part of the call in forward order, or
            the first part in reverse order. */
         result->result_flags |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (instant_stop >= highlimit)
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
   int call_index;

   /* We allow fractionalization commands only for the special case of "piecewise"
      or "random", in which case we will apply them to the first call only. */

   if (ss->cmd.cmd_frac_flags != 0 && !(ss->cmd.cmd_misc_flags & CMD_MISC__PUT_FRAC_ON_FIRST))
      fail("Can't stack meta or fractional concepts.");

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   for (call_index=0; call_index<2; call_index++) {
      uint32 save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
      uint32 saved_last_flag = 0;
      result->cmd = ss->cmd;      /* The call we wish to execute (we will fix it up shortly). */

      if ((call_index ^ parseptr->concept->value.arg1) != 0) {   /* Maybe do them in reverse order. */
         result->cmd.cmd_frac_flags = 0;  /* No fractions to 2nd call. */
         saved_last_flag = result->result_flags & (RESULTFLAG__DID_LAST_PART|RESULTFLAG__PARTS_ARE_KNOWN);
      }
      else {
         result->cmd.parseptr = parseptr->subsidiary_root;
      }

      result->cmd.prior_elongation_bits = save_elongation;
      do_call_in_series(result, FALSE, TRUE, FALSE);
      result->result_flags |= saved_last_flag;
   }
}




Private void do_concept_twice(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This includes "twice" and "N times", which means it can either take a hard-wired
      number or a number form whatever.
      arg1 = 0 :  number of repetitions is hardwired and is in arg2.
      arg1 = 1 :  number of repetitions was specified by user. */

   int highlimit;
   int repetitions;
   int subcall_incr;
   int subcall_index;
   int instant_stop;
   long_boolean first_call;
   long_boolean reverse_order;
   long_boolean do_half_of_last_part;
   uint32 subject_fractions;
   uint32 restraint = ss->cmd.cmd_misc_flags;

   ss->cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;

   if (parseptr->concept->value.arg1)
      repetitions = parseptr->number;
   else
      repetitions = parseptr->concept->value.arg2;

   /* If fractions come in but the craziness is restrained, just pass the fractions on.
      This is what makes "random twice mix" work.  The "random" concept wants to reach through
      the "twice" concept and do the numbered parts of the mix, not the numbered parts of the
      twiceness.  But if the craziness is unrestrained, which is the usual case, we act on
      the fractions.  This makes "interlace twice this with twice that" work. */

   if (ss->cmd.cmd_frac_flags && (restraint & CMD_MISC__RESTRAIN_CRAZINESS) == 0) {
      /* The fractions were meant for us, not the subject call. */
      fraction_info zzz;

      zzz = get_fraction_info(ss->cmd.cmd_frac_flags, 3*CFLAG1_VISIBLE_FRACTION_BIT, repetitions);
      reverse_order = zzz.reverse_order;
      do_half_of_last_part = zzz.do_half_of_last_part;
      highlimit = zzz.highlimit;
      if (reverse_order) highlimit = repetitions-highlimit-1;
      subcall_index = zzz.subcall_index;
      subcall_incr = reverse_order ? -1 : 1;
      instant_stop = zzz.instant_stop ? subcall_index*subcall_incr+1 : 99;
      subject_fractions = 0;
   }
   else {     /* No fractions, or, at least, no fraction for us. */
      reverse_order = FALSE;
      instant_stop = 99;
      do_half_of_last_part = FALSE;
      highlimit = repetitions;
      subcall_index = 0;
      subcall_incr = 1;
      subject_fractions = ss->cmd.cmd_frac_flags;   /* Pass any fractions through. */
   }

   first_call = reverse_order ? FALSE : TRUE;

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   for (;;) {
      uint32 save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */

      if (subcall_index*subcall_incr >= highlimit) break;
      if (subcall_index >= repetitions) fail("The indicated part number doesn't exist.");

      result->cmd = ss->cmd;      /* The call we wish to execute. */

      if (do_half_of_last_part && subcall_index+1 == highlimit)
         result->cmd.cmd_frac_flags = 0x000112;
      else
         result->cmd.cmd_frac_flags = subject_fractions;  /* Fractions for subject call. */

      /* We don't supply these; they get filled in by the call. */
      result->cmd.cmd_misc_flags &= ~DFM1_CONCENTRICITY_FLAG_MASK;
      if (!first_call) {
         result->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;
         result->cmd.cmd_assume.assumption = cr_none;
      }

      result->cmd.prior_elongation_bits = save_elongation;
      update_id_bits(result);
      do_call_in_series(result, FALSE, TRUE, FALSE);
      subcall_index += subcall_incr;
      first_call = FALSE;

      /* If we are being asked to do just one part of a call (from cmd_frac_flags),
         exit now.  Also, see if we just did the last part. */

      if (ss->cmd.cmd_frac_flags && instant_stop != 99) {
         /* Check whether we honored the last possible request.  That is,
            whether we did the last part of the call in forward order, or
            the first part in reverse order. */
         result->result_flags |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (instant_stop >= highlimit)
            result->result_flags |= RESULTFLAG__DID_LAST_PART;
         break;
      }
   }
}


Private void do_concept_trace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int r1, r2, r3, r4, rot1, rot2, rot3, rot4;
   uint32 finalresultflags;
   setup a1, a2, a3, a4, res1, res2, res3, res4;
   setup outer_inners[2];

   if (ss->kind != s_qtag) fail("Must have a 1/4-tag-like setup for trace.");

   ss->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a1 = *ss;
   a2 = *ss;
   a3 = *ss;
   a4 = *ss;

   clear_people(&a1);
   clear_people(&a2);
   clear_people(&a3);
   clear_people(&a4);
   clear_people(&outer_inners[1]);
   clear_people(&outer_inners[0]);

   a1.kind = s2x2;
   a1.rotation = 0;
   a2.kind = s2x2;
   a2.rotation = 0;
   a3.kind = s2x2;
   a3.rotation = 0;
   a4.kind = s2x2;
   a4.rotation = 0;

   if ((ss->people[6].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_south) {
      a2.cmd.parseptr = parseptr->subsidiary_root;
      a4.cmd.parseptr = parseptr->subsidiary_root;

      (void) copy_person(&a1, 2, ss, 7);
      (void) copy_person(&a1, 3, ss, 6);
      (void) copy_person(&a2, 2, ss, 4);
      (void) copy_person(&a2, 3, ss, 5);
      (void) copy_person(&a3, 0, ss, 3);
      (void) copy_person(&a3, 1, ss, 2);
      (void) copy_person(&a4, 0, ss, 0);
      (void) copy_person(&a4, 1, ss, 1);
   }
   else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_north) {
      a1.cmd.parseptr = parseptr->subsidiary_root;
      a3.cmd.parseptr = parseptr->subsidiary_root;

      (void) copy_person(&a1, 0, ss, 0);
      (void) copy_person(&a1, 1, ss, 1);
      (void) copy_person(&a2, 0, ss, 6);
      (void) copy_person(&a2, 1, ss, 7);
      (void) copy_person(&a3, 2, ss, 4);
      (void) copy_person(&a3, 3, ss, 5);
      (void) copy_person(&a4, 2, ss, 2);
      (void) copy_person(&a4, 3, ss, 3);
   }
   else
      fail("Can't determine which box people should work in.");

   a1.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a1);
   move(&a1, FALSE, &res1);
   a2.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a2);
   move(&a2, FALSE, &res2);
   a3.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a3);
   move(&a3, FALSE, &res3);
   a4.cmd.cmd_assume.assumption = cr_none;
   update_id_bits(&a4);
   move(&a4, FALSE, &res4);

   finalresultflags = res1.result_flags | res2.result_flags | res3.result_flags | res4.result_flags;

   /* Check that everyone is in a 2x2 or vertically oriented 1x4. */

   if      ((res1.kind != s2x2 && res1.kind != nothing && (res1.kind != s1x4 || (!(res1.rotation&1)))) ||
            (res2.kind != s2x2 && res2.kind != nothing && (res2.kind != s1x4 || (!(res2.rotation&1)))) ||
            (res3.kind != s2x2 && res3.kind != nothing && (res3.kind != s1x4 || (!(res3.rotation&1)))) ||
            (res4.kind != s2x2 && res4.kind != nothing && (res4.kind != s1x4 || (!(res4.rotation&1)))))
      fail("You can't do this.");

   /* Process people going into the center. */

   outer_inners[1].rotation = 0;
   outer_inners[1].result_flags = 0;

   if   ((res1.kind == s2x2 && (res1.people[2].id1 | res1.people[3].id1)) ||
         (res2.kind == s2x2 && (res2.people[0].id1 | res2.people[1].id1)) ||
         (res3.kind == s2x2 && (res3.people[0].id1 | res3.people[1].id1)) ||
         (res4.kind == s2x2 && (res4.people[2].id1 | res4.people[3].id1)))
      outer_inners[1].kind = s1x4;
   else
      outer_inners[1].kind = nothing;

   r1 = res1.rotation & 2;
   rot1 = (res1.rotation & 3) * 011;
   r2 = res2.rotation & 2;
   rot2 = (res2.rotation & 3) * 011;
   r3 = res3.rotation & 2;
   rot3 = (res3.rotation & 3) * 011;
   r4 = res4.rotation & 2;
   rot4 = (res4.rotation & 3) * 011;

   if   ((res1.kind == s1x4 && (res1.people[2 ^ r1].id1 | res1.people[3 ^ r1].id1)) ||
         (res2.kind == s1x4 && (res2.people[0 ^ r2].id1 | res2.people[1 ^ r2].id1)) ||
         (res3.kind == s1x4 && (res3.people[0 ^ r3].id1 | res3.people[1 ^ r3].id1)) ||
         (res4.kind == s1x4 && (res4.people[2 ^ r4].id1 | res4.people[3 ^ r4].id1))) {
      if (outer_inners[1].kind != nothing) fail("You can't do this.");
      outer_inners[1].kind = s2x2;
   }

   if (res1.kind == s2x2) {
      install_person(&outer_inners[1], 1, &res1, 2);
      install_person(&outer_inners[1], 0, &res1, 3);
   }
   else {
      install_rot(&outer_inners[1], 3, &res1, 2^r1, rot1);
      install_rot(&outer_inners[1], 0, &res1, 3^r1, rot1);
   }

   if (res2.kind == s2x2) {
      install_person(&outer_inners[1], 0, &res2, 0);
      install_person(&outer_inners[1], 1, &res2, 1);
   }
   else {
      install_rot(&outer_inners[1], 0, &res2, 0^r2, rot2);
      install_rot(&outer_inners[1], 3, &res2, 1^r2, rot2);
   }

   if (res3.kind == s2x2) {
      install_person(&outer_inners[1], 3, &res3, 0);
      install_person(&outer_inners[1], 2, &res3, 1);
   }
   else {
      install_rot(&outer_inners[1], 1, &res3, 0^r3, rot3);
      install_rot(&outer_inners[1], 2, &res3, 1^r3, rot3);
   }

   if (res4.kind == s2x2) {
      install_person(&outer_inners[1], 2, &res4, 2);
      install_person(&outer_inners[1], 3, &res4, 3);
   }
   else {
      install_rot(&outer_inners[1], 2, &res4, 2^r4, rot4);
      install_rot(&outer_inners[1], 1, &res4, 3^r4, rot4);
   }

   /* Process people going to the outside. */

   outer_inners[0].rotation = 0;
   outer_inners[0].result_flags = 0;

   if   ((res1.kind == s2x2 && (res1.people[0].id1 | res1.people[1].id1)) ||
         (res2.kind == s2x2 && (res2.people[2].id1 | res2.people[3].id1)) ||
         (res3.kind == s2x2 && (res3.people[2].id1 | res3.people[3].id1)) ||
         (res4.kind == s2x2 && (res4.people[0].id1 | res4.people[1].id1)))
      outer_inners[0].kind = s2x2;
   else
      outer_inners[0].kind = nothing;

   r1 = res1.rotation & 2;
   r2 = res2.rotation & 2;
   r3 = res3.rotation & 2;
   r4 = res4.rotation & 2;

   if   ((res1.kind == s1x4 && (res1.people[0 ^ r1].id1 | res1.people[1 ^ r1].id1)) ||
         (res2.kind == s1x4 && (res2.people[2 ^ r2].id1 | res2.people[3 ^ r2].id1)) ||
         (res3.kind == s1x4 && (res3.people[2 ^ r3].id1 | res3.people[3 ^ r3].id1)) ||
         (res4.kind == s1x4 && (res4.people[0 ^ r4].id1 | res4.people[1 ^ r4].id1))) {
      if (outer_inners[0].kind != nothing) fail("You can't do this.");
      outer_inners[0].kind = s1x4;
      outer_inners[0].rotation = 1;
   }

   if (res1.kind == s2x2) {
      install_person(&outer_inners[0], 0, &res1, 0);
      install_person(&outer_inners[0], 1, &res1, 1);
   }
   else {
      install_rot(&outer_inners[0], 0, &res1, 0 ^ (res1.rotation&2), ((res1.rotation-1)&3)*011);
      install_rot(&outer_inners[0], 1, &res1, 1 ^ (res1.rotation&2), ((res1.rotation-1)&3)*011);
   }

   if (res2.kind == s2x2) {
      install_person(&outer_inners[0], 2, &res2, 2);
      install_person(&outer_inners[0], 3, &res2, 3);
   }
   else {
      install_rot(&outer_inners[0], 2, &res2, 2 ^ (res2.rotation&2), ((res2.rotation-1)&3)*011);
      install_rot(&outer_inners[0], 3, &res2, 3 ^ (res2.rotation&2), ((res2.rotation-1)&3)*011);
   }

   if (res3.kind == s2x2) {
      install_person(&outer_inners[0], 2, &res3, 2);
      install_person(&outer_inners[0], 3, &res3, 3);
   }
   else {
      install_rot(&outer_inners[0], 2, &res3, 2 ^ (res3.rotation&2), ((res3.rotation-1)&3)*011);
      install_rot(&outer_inners[0], 3, &res3, 3 ^ (res3.rotation&2), ((res3.rotation-1)&3)*011);
   }

   if (res4.kind == s2x2) {
      install_person(&outer_inners[0], 0, &res4, 0);
      install_person(&outer_inners[0], 1, &res4, 1);
   }
   else {
      install_rot(&outer_inners[0], 0, &res4, 0 ^ (res4.rotation&2), ((res4.rotation-1)&3)*011);
      install_rot(&outer_inners[0], 1, &res4, 1 ^ (res4.rotation&2), ((res4.rotation-1)&3)*011);
   }

   normalize_concentric(schema_concentric, 1, outer_inners, outer_inners[0].rotation ^ 1, result);
   result->result_flags = finalresultflags;
   reinstate_rotation(ss, result);
}


Private void do_concept_quad_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");
   divided_setup_move(ss, map_lists[s2x2][3]->f[parseptr->concept->value.arg1][0], phantest_ok, TRUE, result);
}


Private void do_concept_inner_outer(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   calldef_schema sch;
   int rot = 0;

   if (parseptr->concept->value.arg1 & 16)
      sch = schema_in_out_quad;
   else
      sch = schema_in_out_triple;

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
               break;
            case sbigh: case sbigx: case sbigdmd: case sbigbone:
               switch (parseptr->concept->value.arg1 & 7) {
                  case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
                  case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
               }

               break;
            default:
               fail("Need a triple line/column setup for this.");
         }

         break;
      case 8+0: case 8+1: case 8+3:
         /* Outside triple lines/waves/columns. */
         switch (ss->kind) {
            case s3x4: case s1x12:
               if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

               if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
                  if (global_tbonetest & 1) fail("There are no triple lines here.");
                  else                      fail("There are no triple columns here.");
               }
               break;
            case sbigh: case sbigx: case sbigrig:
               switch (parseptr->concept->value.arg1 & 7) {
                  case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
                  case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
               }
               break;
            default:
               fail("Need a triple line/column setup for this.");
         }

         break;
      case 16+0: case 16+1: case 16+3:
      case 16+8+0: case 16+8+1: case 16+8+3:
         if (ss->kind != s4x4 && ss->kind != s1x16) fail("Need a 1x16 or 4x4 setup for this.");

         if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

         if (ss->kind == s4x4) {
            rot = (global_tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;

            ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
            canonicalize_rotation(ss);
         }
         else {
            switch (parseptr->concept->value.arg1 & 7) {
               case 0: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS; break;
               case 1: ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES; break;
            }
         }

         break;
      case 4:
         if (ss->kind == s2x6)
            break;
         if (ss->kind == sbigrig)
            break;
         if (ss->kind == s4x4) {
            if (ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1) {
               /* The outer lines/columns are to the left and right.  That's not canonical.
                  We want them at top and bottom.  So we flip the setup. */
               rot = 1;
               ss->rotation++;
               canonicalize_rotation(ss);
            }

            /* Now the people had better be clear from the side centers. */

            if (!(ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1))
               break;
         }
         fail("Need center triple box for this.");
      case 8+4:
         if (ss->kind == s2x6)
            break;
         if (ss->kind == sbigbone)
            break;
         if (ss->kind == sbigdmd)
            break;
         fail("Need outer triple boxes for this.");
      case 16+4:
      case 16+8+4:
         if (ss->kind != s2x8) fail("Need a 2x8 setup for this.");
         break;
   }

   if ((parseptr->concept->value.arg1 & 7) == 3)
      ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   if (parseptr->concept->value.arg1 & 8)
      concentric_move(ss, &ss->cmd, (setup_command *) 0, sch, 0, 0, result);
   else
      concentric_move(ss, (setup_command *) 0, &ss->cmd, sch, 0, 0, result);

   result->rotation -= rot;   /* Flip the setup back. */
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
   switch (ss->kind) {
      case s2x4: case s1x8:
         if (parseptr->concept->value.arg1 & global_tbonetest & 1)
            fail("People are not in a line or wave.");

         if (parseptr->concept->value.arg1 == 3)
            ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

         (void) do_simple_split(ss, TRUE, result);
         break;
      default:
         fail("Need a 2x4 or 1x8 setup for this concept.");
   }
}


Private void do_concept_triple_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x6) fail("Must have a 2x6 setup for this concept.");

   if ((ss->cmd.cmd_misc2_flags & CMD_MISC2__MYSTIFY_SPLIT) && parseptr->concept->value.arg1 != MPKIND__SPLIT)
      fail("Mystic not allowed with this concept.");

   divided_setup_move(ss, map_lists[s2x2][2]->f[parseptr->concept->value.arg1][0], phantest_ok, TRUE, result);
}


Private void do_concept_centers_or_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   calldef_schema schema;
   int k = parseptr->concept->value.arg1;
   uint32 concbits = 0;

   switch (k&6) {
      case 2:
         schema = schema_concentric_6_2;
         break;
      case 4:
         schema = schema_concentric_2_6;
         break;
      case 6:
         schema = schema_concentric;
         concbits = DFM1_CONC_CONCENTRIC_RULES;
         break;
      default:
         if (setup_attrs[ss->kind].setup_limits == 3)
            schema = schema_single_concentric;
         else
            schema = schema_concentric;
         break;
   }

   if (k&1)
      concentric_move(ss, (setup_command *) 0, &ss->cmd,
               schema, 0, concbits, result);
   else
      concentric_move(ss, &ss->cmd, (setup_command *) 0,
               schema, 0, 0, result);
}


Private void do_concept_centers_and_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   calldef_schema schema;
   setup_command subsid_cmd;
   uint32 concbits = 0;

   switch (parseptr->concept->value.arg1) {
      case 2:
         schema = schema_concentric_6_2;
         break;
      case 3:
         schema = schema_concentric_2_6;
         break;
      case 7:
         schema = schema_concentric;
         concbits = DFM1_CONC_CONCENTRIC_RULES;
         break;
      default:
         if (setup_attrs[ss->kind].setup_limits == 3)
            schema = schema_single_concentric;
         else
            schema = schema_concentric;
         break;
   }

   subsid_cmd = ss->cmd;
   subsid_cmd.parseptr = parseptr->subsidiary_root;
   concentric_move(ss, &ss->cmd, &subsid_cmd, schema, 0, concbits, result);
}


Private void do_concept_triple_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* See "do_concept_do_phantom_diamonds" for meaning of arg2. */

   if (ss->kind != s3dmd) fail("Must have a triple diamond or 1/4 tag setup for this concept.");
   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   divided_setup_move(ss, map_lists[sdmd][2]->f[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
}


Private void do_concept_quad_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* See "do_concept_do_phantom_diamonds" for meaning of arg2. */

   if (ss->kind != s4dmd) fail("Must have a quadruple diamond or 1/4 tag setup for this concept.");
   ss->cmd.cmd_misc_flags |= parseptr->concept->value.arg2;
   divided_setup_move(ss, map_lists[sdmd][3]->f[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
}


Private void do_concept_ferris(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup temp;

   temp = *ss;
   temp.kind = s3x4;
   clear_people(&temp);

   if (parseptr->concept->value.arg1) {
      /* This is "release". */

      if ((ss->kind != s_qtag) || ((global_tbonetest & 01) != 0))
         fail("Must have quarter-tag to do this concept.");

      if (((
               (ss->people[0].id1 ^ d_south) |
               (ss->people[1].id1 ^ d_south) |
               (ss->people[2].id1 ^ d_south) |
               (ss->people[3].id1 ^ d_north) |
               (ss->people[4].id1 ^ d_north) |
               (ss->people[5].id1 ^ d_north) |
               (ss->people[6].id1 ^ d_north) |
               (ss->people[7].id1 ^ d_south)
            ) & d_mask) == 0) {
         (void) copy_person(&temp, 1, ss, 0);
         (void) copy_person(&temp, 4, ss, 1);
         (void) copy_person(&temp, 6, ss, 2);
         (void) copy_person(&temp, 5, ss, 3);
         (void) copy_person(&temp, 7, ss, 4);
         (void) copy_person(&temp, 10, ss, 5);
         (void) copy_person(&temp, 0, ss, 6);
         (void) copy_person(&temp, 11, ss, 7);
      }
      else if (((
               (ss->people[0].id1 ^ d_south) |
               (ss->people[1].id1 ^ d_south) |
               (ss->people[2].id1 ^ d_north) |
               (ss->people[3].id1 ^ d_south) |
               (ss->people[4].id1 ^ d_north) |
               (ss->people[5].id1 ^ d_north) |
               (ss->people[6].id1 ^ d_south) |
               (ss->people[7].id1 ^ d_north)
            ) & d_mask) == 0) {
         (void) copy_person(&temp, 10, ss, 0);
         (void) copy_person(&temp, 2, ss, 1);
         (void) copy_person(&temp, 3, ss, 2);
         (void) copy_person(&temp, 5, ss, 3);
         (void) copy_person(&temp, 4, ss, 4);
         (void) copy_person(&temp, 8, ss, 5);
         (void) copy_person(&temp, 9, ss, 6);
         (void) copy_person(&temp, 11, ss, 7);
      }
      else
         fail("Incorrect facing directions.");
   }
   else {
      /* This is "ferris". */

      if ((ss->kind != s2x4) || ((global_tbonetest & 01) != 0))
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
         (void) copy_person(&temp, 0, ss, 0);
         (void) copy_person(&temp, 1, ss, 1);
         (void) copy_person(&temp, 5, ss, 2);
         (void) copy_person(&temp, 4, ss, 3);
         (void) copy_person(&temp, 6, ss, 4);
         (void) copy_person(&temp, 7, ss, 5);
         (void) copy_person(&temp, 11, ss, 6);
         (void) copy_person(&temp, 10, ss, 7);
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
         (void) copy_person(&temp, 10, ss, 0);
         (void) copy_person(&temp, 11, ss, 1);
         (void) copy_person(&temp, 2, ss, 2);
         (void) copy_person(&temp, 3, ss, 3);
         (void) copy_person(&temp, 4, ss, 4);
         (void) copy_person(&temp, 5, ss, 5);
         (void) copy_person(&temp, 8, ss, 6);
         (void) copy_person(&temp, 9, ss, 7);
      }
      else
         fail("Incorrect facing directions.");
   }

   divided_setup_move(&temp, map_lists[s1x4][2]->f[MPKIND__SPLIT][1], phantest_ok, TRUE, result);

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
   setup temp;
   map_thing *map;

   /* Split an 8 person setup. */
   if (setup_attrs[ss->kind].setup_limits == 7) {
      ss->cmd.parseptr = parseptr;    /* Reset it to execute this same concept again, until it doesn't have to split any more. */
      if (do_simple_split(ss, TRUE, result))
         fail("Not in correct setup for overlapped diamond/line concept.");
      return;
   }

   switch (ss->kind) {
      case s1x4:
         if (parseptr->concept->value.arg1 & 1)
            fail("Must be in a diamond.");

         temp = *ss;
         (void) copy_person(&temp, 4, ss, 2);
         (void) copy_person(&temp, 5, ss, 3);
         clear_person(&temp, 2);
         clear_person(&temp, 3);
         clear_person(&temp, 6);
         clear_person(&temp, 7);
         map = map_lists[sdmd][1]->f[MPKIND__DMD_STUFF][0];
         goto fixup;
      case sdmd:
         if (!(parseptr->concept->value.arg1 & 1))
            fail("Must be in a line.");
         if (parseptr->concept->value.arg1 == 3)
            ss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

         temp = *ss;
         (void) copy_person(&temp, 3, ss, 1);
         (void) copy_person(&temp, 4, ss, 2);
         (void) copy_person(&temp, 7, ss, 3);
         clear_person(&temp, 1);
         clear_person(&temp, 2);
         clear_person(&temp, 5);
         clear_person(&temp, 6);
         map = map_lists[s1x4][1]->f[MPKIND__DMD_STUFF][0];
         goto fixup;
   }

   fail("Not in correct setup for overlapped diamond/line concept.");

   fixup:

   temp.kind = s_thar;
   canonicalize_rotation(&temp);
   divided_setup_move(&temp, map, phantest_ok, TRUE, result);

   if (result->kind == s2x2)
      return;
   else if (result->kind != s_thar)
      fail("Something horrible happened during overlapped diamond call.");

   if ((result->people[2].id1 | result->people[3].id1 | result->people[6].id1 | result->people[7].id1) == 0) {
      result->kind = s1x4;
      (void) copy_person(result, 2, result, 4);
      (void) copy_person(result, 3, result, 5);
   }
   else if ((result->people[0].id1 | result->people[1].id1 | result->people[4].id1 | result->people[5].id1) == 0) {
      result->kind = s1x4;
      result->rotation++;    /* Set it to 1, this is canonical. */
      (void) copy_rot(result, 0, result, 2, 033);
      (void) copy_rot(result, 1, result, 3, 033);
      (void) copy_rot(result, 2, result, 6, 033);
      (void) copy_rot(result, 3, result, 7, 033);
   }
   else if ((result->people[1].id1 | result->people[2].id1 | result->people[5].id1 | result->people[6].id1) == 0) {
      result->kind = sdmd;
      (void) copy_person(result, 1, result, 3);
      (void) copy_person(result, 2, result, 4);
      (void) copy_person(result, 3, result, 7);
   }
   else if ((result->people[0].id1 | result->people[3].id1 | result->people[4].id1 | result->people[7].id1) == 0) {
      result->kind = sdmd;
      result->rotation++;    /* Set it to 1, this is canonical. */
      (void) copy_rot(result, 0, result, 2, 033);
      (void) copy_rot(result, 3, result, 1, 033);
      (void) copy_rot(result, 1, result, 5, 033);
      (void) copy_rot(result, 2, result, 6, 033);
   }
   else
      fail("Can't put the setups back together.");
}



Private void do_concept_all_8(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
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

      divided_setup_move(ss, map_lists[s2x2][1]->f[MPKIND__ALL_8][0], phantest_ok, TRUE, result);

      if (result->kind == s4x4)
         goto check_col_ending;
   }
   else {

      /* This is "all 8" or "all 8 (diamond)". */

      if (ss->kind == s_thar) {
         /* Either one is legal in a thar. */
         if (key == 1)
            divided_setup_move(ss, map_lists[s1x4][1]->f[MPKIND__ALL_8][0], phantest_ok, TRUE, result);
         else
            divided_setup_move(ss, map_lists[sdmd][1]->f[MPKIND__ALL_8][0], phantest_ok, TRUE, result);

         /* The above stuff did an "elongate perpendicular to the long axis of the 1x4 or diamond"
            operation, also known as an "ends' part of concentric" operation.  Some people believe
            (perhaps as part of a confusion between "all 8" and "all 4 couples", or some other mistaken
            notion) that they should always end on column spots.  Now it is true that "all 4 couples"
            always ends on columns spots, but that's because it can only begin on column spots.
            To avoid undue controversy or bogosity, we only allow the call if both criteria are met.
            The "opposite elongation" criterion was already met, so we check for the mistaken
            "end on column spots" criterion.  If someone really wants to hinge from a thar, they can
            just say "hinge". */

         if (result->kind == s4x4) {
            if (     ((result->people[1].id1 | result->people[2].id1 | result->people[9].id1 | result->people[10].id1) & 010) ||
                     ((result->people[14].id1 | result->people[5].id1 | result->people[6].id1 | result->people[13].id1) & 1))
               fail("Ending position is not defined.");
         }
      }
      else if (key == 2)
         fail("Must be in a thar.");   /* Can't do "all 8 (diamonds)" from squared-set spots. */
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
                  ss->people[8].id1 | ss->people[11].id1 | ss->people[12].id1 | ss->people[15].id1 != 0))
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

         divided_setup_move(ss, map_lists[s2x2][1]->f[MPKIND__ALL_8][0], phantest_ok, TRUE, result);

         /* If this ended in a thar, we accept it.  If not, we have the usual lines-to-lines/
            columns-to-columns problem.  We don't know whether to enforce column spots, line spots,
            perpendicular to the lines they had after stepping to a wave (if indeed they did so;
            we don't know), to footprints from before stepping to a wave, or what.  So the only case
            we allow is columns-to-columns. */

         if (result->kind == s4x4) {
            if (tl & 010)
               fail("Ending position is not defined.");
            goto check_col_ending;
         }
         else if (result->kind != s_thar)
            fail("Ending position is not defined.");
      }
      else
         fail("Must be in a thar or squared-set spots.");
   }

   return;


   check_col_ending:

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
      out of each others' way may have left people incorrect. */

   if (     ((result->people[1].id1 | result->people[2].id1 | result->people[9].id1 | result->people[10].id1) & 010) ||
            ((result->people[14].id1 | result->people[5].id1 | result->people[6].id1 | result->people[13].id1) & 1))
      fail("People must end as if on column spots.");
}

Private void do_concept_meta(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   parse_block *parseptrcopy;
   parse_block *parseptr_skip;
   uint32 index;
   setup tttt;
   concept_kind k;
   uint32 finalresultflags = 0;
   int key = parseptr->concept->value.arg1;
   uint32 subject_props = 0;
   uint32 craziness_restraint = 0;

   /* key =
      random <concept>          : 0
      reverse random <concept>  : 1
      piecewise <concept>       : 2
      initially <concept>       : 3
      finish                    : 4
      reverse order             : 5
      like a                    : 6
      finally <concept>         : 7
      do the Nth part <concept> : 8
      skip the Nth part         : 9
      shift N                   : 10
      shifty                    : 11 */

   if (key == 0 && (ss->cmd.cmd_final_flags & INHERITFLAG_REVERSE)) {
      key = 1;     /* "reverse" and "random"  ==>  "reverse random" */
      ss->cmd.cmd_final_flags &= ~INHERITFLAG_REVERSE;
   }

   if (ss->cmd.cmd_final_flags)   /* Now demand that no flags remain. */
      fail("Illegal modifier for this concept.");

   if (key == 4) {
      /* This is "finish".  Do the call after the first part, with the special indicator
         saying that this was invoked with "finish", so that the flag will be checked. */

      if (!ss->cmd.cmd_frac_flags)
         ss->cmd.cmd_frac_flags = 0x000111UL;

      if (ss->cmd.cmd_frac_flags == 0x000111UL)
         ss->cmd.cmd_frac_flags = 0x810111UL;
      else if ((ss->cmd.cmd_frac_flags & 0xF0FFFFUL) == 0x800111UL)
         /* If we are already doing just parts N and later, just bump N. */
         ss->cmd.cmd_frac_flags += 0x010000UL;
      else if ((ss->cmd.cmd_frac_flags & 0xF0FFFFUL) == 0x000111UL)
         /* If we are already doing just parts N only, just bump N. */
         ss->cmd.cmd_frac_flags += 0x010000UL;
      else
         fail("Can't stack meta or fractional concepts.");

      move(ss, FALSE, result);
      normalize_setup(result, simple_normalize);
      return;
   }
   else if (key == 5) {
      /* This is "reverse order". */

      if (!ss->cmd.cmd_frac_flags)
         ss->cmd.cmd_frac_flags = 0x000111UL;

      ss->cmd.cmd_frac_flags ^= 0x100000UL;
      move(ss, FALSE, result);
      return;
   }
   else if (key == 6) {
      /* This is "like a".  Do the last part of the call. */

      if (!ss->cmd.cmd_frac_flags)
         ss->cmd.cmd_frac_flags = 0x000111UL;

      if (ss->cmd.cmd_frac_flags != 0x000111UL)
         fail("Can't stack meta or fractional concepts.");

      ss->cmd.cmd_frac_flags = 0x110111;
      move(ss, FALSE, result);
      normalize_setup(result, simple_normalize);
      return;
   }

   *result = *ss;

   if (key != 9 && key != 10 && key != 11) {
      /* Scan the modifiers, remembering them and their end point.  The reason for this is to
         avoid getting screwed up by a comment, which counts as a modifier.  YUK!!!!!!
         This code used to have the beginnings of stuff to do it really right.  It isn't
         worth it, and isn't worth holding up "random left" for.  In any case, the stupid
         handling of comments will go away soon. */
   
      parseptrcopy = skip_one_concept(ss->cmd.parseptr);
      k = parseptrcopy->concept->kind;
      parseptr_skip = parseptrcopy->next;
   
      if (k == concept_crazy || k == concept_frac_crazy || k == concept_fractional || k == concept_twice || k == concept_n_times)
         craziness_restraint = CMD_MISC__RESTRAIN_CRAZINESS;
   
      subject_props = concept_table[k].concept_prop;
   }

   if (key == 9) {
      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");
   
      /* Do the initial part, if any. */

      if (parseptr->number > 1) {
         tttt = *result;
         /* Set the fractionalize field to do the first few parts of the call. */
         tttt.cmd = ss->cmd;
         tttt.cmd.cmd_frac_flags = ((parseptr->number-1) << 16) | 0x400111;
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }

      /* Do the final part. */
      tttt = *result;
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags = 0x800111 | (parseptr->number << 16);
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }
   else if (key == 10 || key == 11) {      /* shift <N> or shifty */
      int shiftynum;
      shiftynum = (key == 11) ? 1 : parseptr->number;

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");
   
      /* Do the last (shifted) part. */

      tttt = *result;
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags = 0x800111 | (shiftynum << 16);
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);

      /* Do the initial part up to the shift point. */

      tttt = *result;
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags = (shiftynum << 16) | 0x400111;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }
   else if (key == 8) {
      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");

      /* Do the initial part, if any, without the concept. */

      if (parseptr->number > 1) {
         tttt = *result;

         /* Set the fractionalize field to do the first few parts of the call. */
         tttt.cmd = ss->cmd;
         tttt.cmd.cmd_frac_flags = 0x400111 | ((parseptr->number-1) << 16);
         tttt.cmd.parseptr = parseptr_skip;
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }

      /* Do the part of the call that needs the concept. */

      tttt = *result;
      /* Do the call with the concept. */
      /* Set the fractionalize field to do the Nth part. */
      tttt.cmd = ss->cmd;

      tttt.cmd.cmd_frac_flags = 0x000111 | (parseptr->number << 16);
      tttt.cmd.cmd_misc_flags |= craziness_restraint;
      tttt.cmd.parseptr = parseptrcopy;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);

      if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
         fail("Can't have 'no one' do a call.");

      /* Do the final part, if there is more. */
   
      if (!(result->result_flags & RESULTFLAG__DID_LAST_PART)) {
         tttt = *result;
         tttt.cmd = ss->cmd;
         tttt.cmd.cmd_frac_flags = 0x200111 | (parseptr->number << 16);
         tttt.cmd.parseptr = parseptr_skip;      /* Skip over the concept. */
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }
   }
   else if (key == 3) {
      /* Key = 3 is special: we select the first part with the concept,
         and then the rest of the call without the concept. */

      setup tttt = *result;

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");

      /* Do the call with the concept. */
      /* Set the fractionalize field to execute the first part of the call. */
      tttt.cmd = ss->cmd;

      tttt.cmd.cmd_frac_flags = 0x010111;
      tttt.cmd.cmd_misc_flags |= craziness_restraint;
      tttt.cmd.parseptr = parseptrcopy;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
      tttt = *result;
      /* Do the call without the concept. */
      /* Set the fractionalize field to execute the rest of the call. */
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags = 0x810111;
      tttt.cmd.parseptr = parseptr_skip;      /* Skip over the concept. */
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }
   else if (key == 7) {
      /* This is "finally": we select all but the last part without the concept,
         and then the last part with the concept. */

      setup tttt = *result;

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");

      /* Do the call without the concept. */
      /* Set the fractionalize field to execute all but the last part of the call. */
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags = 0xB10111;
      tttt.cmd.parseptr = parseptr_skip;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
      tttt = *result;

      /* Do the call with the concept. */
      /* Set the fractionalize field to execute the last part of the call. */
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags = 0x110111;
      tttt.cmd.cmd_misc_flags |= craziness_restraint;
      tttt.cmd.parseptr = parseptrcopy;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }
   else {
      /* Otherwise, this is the "random", "reverse random", or "piecewise" concept.
         Repeatedly execute parts of the call, skipping the concept where required. */

      /* We don't allow any other fractional stuff. */

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");

      /* Initialize. */
      if (!ss->cmd.cmd_frac_flags)
         ss->cmd.cmd_frac_flags = 0x000111;
   
      index = 0;

      do {
         parse_block *parseptrcopycopy = parseptrcopy;
         setup tttt = *result;

         index++;
         tttt.cmd = ss->cmd;

         /* If concept is "[reverse] random" and this is an even/odd-numbered part,
            as the case may be, skip over the concept. */
         if (((key & ~1) == 0) && ((index & 1) == key)) {
            /* But how do we skip the concept?  If it an ordinary single-call concept,
               it's easy.  But, if the concept takes a second call (the only legal case
               of this being "concept_special_sequential") we use its second subject call
               instead of the first.  This is part of the peculiar behavior of this
               particular combination. */

            if (subject_props & CONCPROP__SECOND_CALL)
               parseptrcopycopy = parseptrcopycopy->subsidiary_root;
            else
               parseptrcopycopy = parseptrcopycopy->next;
         }
         else
            tttt.cmd.cmd_misc_flags |= craziness_restraint;

         /* Set the fractionalize field to do the indicated part.
            The CMD_MISC__PUT_FRAC_ON_FIRST bit tells the "special_sequential" concept
            (if that is the subject concept) that fractions are allowed, and they
            are to be applied to the first call only. */
         tttt.cmd.cmd_misc_flags |= CMD_MISC__PUT_FRAC_ON_FIRST;
         tttt.cmd.cmd_frac_flags |= index << 16;
         tttt.cmd.parseptr = parseptrcopycopy;
         update_id_bits(&tttt);
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
         if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");
      }
      while (!(result->result_flags & RESULTFLAG__DID_LAST_PART));
   }

   result->result_flags = finalresultflags & ~3;
}


Private void do_concept_replace_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tttt;
   uint32 finalresultflags = 0;
   int stopindex;

   if (ss->cmd.cmd_frac_flags)
      fail("Can't stack meta or fractional concepts.");

   *result = *ss;

   /* Do the initial part, if any, without the concept. */

   if (parseptr->concept->value.arg1)
      stopindex = parseptr->number;      /* Interrupt after Nth part. */
   else
      stopindex = parseptr->number-1;    /* Replace Nth part. */

   if (stopindex > 0) {
      tttt = *result;
      /* Set the fractionalize field to [0 0 parts-to-do-normally]. */
      tttt.cmd = ss->cmd;


      tttt.cmd.cmd_frac_flags = 0x400111 | (stopindex << 16);
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }

   /* Do the interruption/replacement call. */

   tttt = *result;
   tttt.cmd = ss->cmd;
   tttt.cmd.parseptr = parseptr->subsidiary_root;
   update_id_bits(&tttt);           /* So you can interrupt with "leads run", etc. */
   move(&tttt, FALSE, result);
   finalresultflags |= result->result_flags;
   normalize_setup(result, simple_normalize);

   /* Do the final part, if there is more. */

   tttt = *result;
   /* Skip over the concept. */
   /* Set the fractionalize field to [1 0 parts-to-do-normally+1]. */
   tttt.cmd = ss->cmd;
   tttt.cmd.cmd_frac_flags = 0x200111 | (parseptr->number << 16);
   move(&tttt, FALSE, result);
   finalresultflags |= result->result_flags;
   normalize_setup(result, simple_normalize);

   result->result_flags = finalresultflags & ~3;
}


Private void do_concept_interlace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 index;
   uint32 first_doneflag, second_doneflag;
   uint32 save_elongation;

   first_doneflag = 0;
   second_doneflag = 0;
   index = 0;

   if (ss->cmd.cmd_frac_flags)
      fail("Can't stack meta or fractional concepts.");

   /* Initialize. */
   if (!ss->cmd.cmd_frac_flags)
      ss->cmd.cmd_frac_flags = 0x000111;

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   do {
      index++;

      if (first_doneflag == 0) {
         /* Do the indicated part of the first call. */
         save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
         result->cmd = ss->cmd;
         result->cmd.cmd_frac_flags |= index << 16;
         result->cmd.prior_elongation_bits = save_elongation;
         do_call_in_series(result, FALSE, TRUE, FALSE);
         if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");
         first_doneflag = result->result_flags & RESULTFLAG__DID_LAST_PART;
      }
      else if (second_doneflag == 0)
         warn(warn__bad_interlace_match);

      if (second_doneflag == 0) {
         save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
         /* Do the indicated part of the second call. */
         result->cmd = ss->cmd;
         result->cmd.cmd_frac_flags |= index << 16;
         result->cmd.parseptr = parseptr->subsidiary_root;
         result->cmd.prior_elongation_bits = save_elongation;
         do_call_in_series(result, FALSE, TRUE, FALSE);
         if (!(result->result_flags & RESULTFLAG__PARTS_ARE_KNOWN))
            fail("Can't have 'no one' do a call.");
         second_doneflag = result->result_flags & RESULTFLAG__DID_LAST_PART;
      }
      else if (first_doneflag == 0)
         warn(warn__bad_interlace_match);
   }
   while ((first_doneflag & second_doneflag) == 0);
}



Private int gcd(int a, int b)    /* a <= b */
{
   for (;;) {
      int rem;
      if (a==0) return b;
      rem = b % a;
      if (rem==0) return a;
      b = a;
      a = rem;
   }
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
   int s_numer, s_denom, e_numer, e_denom, divisor;

   numer = parseptr->number;
   denom = numer >> 4;
   numer &= 0xF;

   if (parseptr->concept->value.arg1 == 2) {

      /* Do the whole call first, then part of it again. */

      *result = *ss;
      result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */
      do_call_in_series(result, FALSE, TRUE, FALSE);
   }

   /* Initialize. */
   if (!ss->cmd.cmd_frac_flags)
      ss->cmd.cmd_frac_flags = 0x000111;

   /* Check that user isn't doing something stupid. */
   if (numer <= 0 || numer >= denom)
      fail("Illegal fraction.");

   s_numer = (ss->cmd.cmd_frac_flags & 0xF000) >> 12;        /* Start point. */
   s_denom = (ss->cmd.cmd_frac_flags & 0xF00) >> 8;
   e_numer = (ss->cmd.cmd_frac_flags & 0xF0) >> 4;          /* Stop point. */
   e_denom = (ss->cmd.cmd_frac_flags & 0xF);

   /* Xor the "reverse" bit with the first/last fraction indicator. */
   if ((parseptr->concept->value.arg1 ^ (ss->cmd.cmd_frac_flags >> 20)) & 1) {
      /* This is "last fraction". */

      s_numer = s_numer*numer + s_denom*(denom-numer);
      e_numer = e_numer*numer + e_denom*(denom-numer);
   }
   else {
      /* This is "fractional". */

      s_numer *= numer;
      e_numer *= numer;
   }

   s_denom *= denom;
   e_denom *= denom;

   if (s_numer < 0 || s_numer >= s_denom || e_numer <= 0 || e_numer > e_denom)
      fail("Illegal fraction.");

   divisor = gcd(s_numer, s_denom);
   s_numer /= divisor;
   s_denom /= divisor;

   divisor = gcd(e_numer, e_denom);
   e_numer /= divisor;
   e_denom /= divisor;

   if (s_numer > 15 || s_denom > 15 || e_numer > 15 || e_denom > 15)
      fail("Fractions are too complicated.");

   ss->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | (s_numer<<12) | (s_denom<<8) | (e_numer<<4) | e_denom;
   ss->cmd.parseptr = parseptr->next;
   ss->cmd.callspec = NULLCALLSPEC;
   ss->cmd.cmd_final_flags = 0;

   if (parseptr->concept->value.arg1 == 2) {
      uint32 save_elongation = result->cmd.prior_elongation_bits;
      result->cmd = ss->cmd;      /* The call we wish to execute. */
      result->cmd.prior_elongation_bits = save_elongation;
      do_call_in_series(result, FALSE, TRUE, FALSE);
   }
   else
      move(ss, FALSE, result);
}


Private void do_concept_so_and_so_begin(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 finalresultflags;
   selector_kind saved_selector;
   int i;
   setup setup1, setup2;
   setup the_setups[2];

   if (ss->cmd.cmd_frac_flags)
      fail("Can't stack meta or fractional concepts.");

   saved_selector = current_selector;
   current_selector = parseptr->selector;

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
   
   current_selector = saved_selector;
   
   normalize_setup(&setup1, normalize_before_isolated_call);
   normalize_setup(&setup2, normalize_before_isolated_call);
   /* Set the fractionalize field to execute the first part of the call. */
   setup1.cmd.cmd_frac_flags = 0x410111;

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
   the_setups[1].cmd.cmd_frac_flags = 0x210111;
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

   if (ss->cmd.cmd_final_flags & INHERITFLAG_CROSS) {
      switch (schema) {
         case schema_concentric:
            schema = schema_cross_concentric;
            break;
         case schema_single_concentric:
            schema = schema_single_cross_concentric;
            break;
         case schema_concentric_diamonds:
            schema = schema_cross_concentric_diamonds;
            break;
         default:
            fail("Redundant 'CROSS' modifiers.");
      }
   }

   if (ss->cmd.cmd_final_flags & INHERITFLAG_SINGLE) {
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

   ss->cmd.cmd_final_flags &= ~(INHERITFLAG_CROSS | INHERITFLAG_SINGLE);

   if (ss->cmd.cmd_final_flags)   /* We don't allow other flags, like "left". */
      fail("Illegal modifier before \"concentric\".");

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
                     schema, 0, DFM1_CONC_CONCENTRIC_RULES, result);
               return;
            default:
               fail("Can't figure out how to do single concentric here.");
         }
      
         ss->cmd.parseptr = parseptr;    /* Reset it to execute this same concept again, until it doesn't have to split any more. */
         divided_setup_move(ss, map_lists[mf->a][1]->f[MPKIND__SPLIT][mf->b], phantest_ok, TRUE, result);
         break;
      default:
         concentric_move(ss, &ss->cmd, &ss->cmd,
               schema, 0, DFM1_CONC_CONCENTRIC_RULES, result);
         /* 8-person concentric operations do not show the split. */
         result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_MASK;
         break;
   }
}



Private void do_concept_tandem(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   tandem_couples_move(ss,
         parseptr->concept->value.arg1 ? parseptr->selector : selector_uninitialized,
         parseptr->concept->value.arg2,    /* normal=FALSE, twosome=TRUE */
         parseptr->number,
         parseptr->concept->value.arg3,    /* normal=0 phantom=1 gruesome=2 */
         parseptr->concept->value.arg4,    /* tandem=0 couples=1 siamese=2, etc. */
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
   uint32 extra_mods;

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

   extra_mods = ss->cmd.cmd_final_flags & (INHERITFLAG_REVERSE|INHERITFLAG_LEFT|FINAL__SPLIT|INHERITFLAG_GRAND|INHERITFLAG_CROSS|INHERITFLAG_SINGLE|INHERITFLAG_INTLK);

   if (extra_mods && !(this_table_item->concept_prop & CONCPROP__PERMIT_REVERSE)) {
      /* This can only be legal if we find a translation in the table. */

      concept_fixer_thing *p;

      for (p=concept_fixer_table ; p->newmods ; p++) {
         if (p->newmods == extra_mods && &concept_descriptor_table[p->before] == this_concept) {
            this_concept = &concept_descriptor_table[p->after];
            artificial_parse_block = *this_concept_parse_block;
            artificial_parse_block.concept = this_concept;
            this_concept_parse_block = &artificial_parse_block;
            this_kind = this_concept->kind;
            this_table_item = &concept_table[this_kind];
            ss->cmd.cmd_final_flags &= ~extra_mods;   /* Take out those mods. */
            goto found_new_concept;
         }
      }

      fail("Illegal modifier before a concept.");

      found_new_concept: ;
   }

   /* We know the following about the incoming setup:
      ss->cmd.parseptr has the stuff, including the concept we are going to try to do.
      ss->cmd.callspec is null.
      ss->cmd.cmd_misc_flags has lots of stuff.
      ss->cmd.cmd_final_flags is nearly null.  However, it may contain FINAL__MUST_BE_TAG.
         The rest of this file used to just ignore those,
         passing zero for the final commands.   This may be a bug.  In any case, we
         have now preserved even those two flags in cmd_final_flags, so things can
         possibly get better. */

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {
      if (     (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_KMASK) == CMD_MISC2__CENTRAL_MYSTIC &&
               (this_table_item->concept_prop & CONCPROP__PERMIT_MYSTIC)) {

         /* This is "mystic" with something like "triple waves". */
         /* Turn on the good bits. */

         ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_SPLIT;
         if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_INV_CONC)
            ss->cmd.cmd_misc2_flags |= CMD_MISC2__MYSTIFY_INVERT;

         /* And turn off the old ones. */

         ss->cmd.cmd_misc2_flags &= ~CMD_MISC2__CTR_END_MASK;
      }
      else if (   (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_KMASK) == CMD_MISC2__CENTRAL_PLAIN &&
                  this_kind == concept_fractional) {
         /* We *DO* allow central and fractional to be stacked in either order. */
         ;
      }
      else if (this_kind != concept_snag_mystic && this_kind != concept_central) {
         /* If it's another invert/snag/central/mystic, we allow it, since the routine will check carefully. */
         fail("Can't do \"invert/central/snag/mystic\" followed by another concept or modifier.");
      }
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
      final_set junk_concepts;
      int tbonetest = 0;
      int stdtest = 0;
      int livemask = 0;

      /* Skip to the phantom-line (or whatever) concept by going over the "standard" and skipping comments. */
      substandard_concptptr = process_final_concepts(orig_concept_parse_block->next, TRUE, &junk_concepts);
   
      /* If we hit "matrix", do a little extra stuff and continue. */

      if (!junk_concepts && substandard_concptptr->concept->kind == concept_matrix) {
         ss->cmd.cmd_misc_flags |= CMD_MISC__MATRIX_CONCEPT;
         substandard_concptptr = process_final_concepts(substandard_concptptr->next, TRUE, &junk_concepts);
      }

      if (junk_concepts || (!(concept_table[substandard_concptptr->concept->kind].concept_prop & CONCPROP__STANDARD)))
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

      if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX))
         do_matrix_expansion(ss, concept_table[substandard_concptptr->concept->kind].concept_prop, FALSE);

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

      if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do this concept in this setup.");
   
      {
         int i, j;
         selector_kind saved_selector = current_selector;
   
         current_selector = this_concept_parse_block->selector;
   
         for (i=0, j=1; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
            int p = ss->people[i].id1;
            tbonetest |= p;
            if (p) {
               livemask |= j;
               if (selectp(ss, i)) stdtest |= p;
            }
         }
   
         current_selector = saved_selector;
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

      ss->cmd.parseptr = substandard_concptptr->next;
      (concept_table[substandard_concptptr->concept->kind].concept_action)(ss, substandard_concptptr, result);
      /* Beware -- result is not necessarily canonicalized. */
      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_MASK;  /* **** For now. */
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

   if (this_kind != concept_c1_phantom &&
            !(this_table_item->concept_prop & CONCPROP__MATRIX_OBLIVIOUS))
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

   /* See if this concept can be invoked with "standard".  If so, it wants
      tbonetest and livemask computed, and expects the former to indicate
      only the standard people. */

   if (this_table_item->concept_prop & (CONCPROP__STANDARD | CONCPROP__GET_MASK)) {
      int i;
      uint32 j;
      long_boolean doing_select;
      selector_kind saved_selector = current_selector;

      if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do this concept in this setup.");

      global_tbonetest = 0;
      global_livemask = 0;
      global_selectmask = 0;
      global_tboneselect = 0;
      doing_select = this_table_item->concept_prop & CONCPROP__USE_SELECTOR;

      if (doing_select) {
         current_selector = this_concept_parse_block->selector;
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

      current_selector = saved_selector;

      if (!global_tbonetest) {
         result->result_flags = 0;
         result->kind = nothing;
         return TRUE;
      }
   }

   (*concept_func)(ss, this_concept_parse_block, result);
   /* Beware -- result is not necessarily canonicalized. */
   if (!(this_table_item->concept_prop & CONCPROP__SHOW_SPLIT))
      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_MASK;
   return TRUE;
}


#define Standard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX | CONCPROP__STANDARD)
#define Nostandard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX)
#define Nostep_phantom (CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS)


/* Beware!!  This table must be keyed to definition of "concept_kind" in sd.h . */

concept_table_item concept_table[] = {
   /* concept_another_call_next_mod */    {0,                                                                                      0},
   /* concept_mod_declined */             {0,                                                                                      0},
   /* marker_end_of_list */               {0,                                                                                      0},
   /* concept_comment */                  {0,                                                                                      0},
   /* concept_concentric */               {CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,                                        do_concept_concentric},
   /* concept_tandem */                   {CONCPROP__SHOW_SPLIT,                                                                   do_concept_tandem},
   /* concept_some_are_tandem */          {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,                                          do_concept_tandem},
   /* concept_frac_tandem */              {CONCPROP__USE_NUMBER | CONCPROP__SHOW_SPLIT,                                            do_concept_tandem},
   /* concept_some_are_frac_tandem */     {CONCPROP__USE_NUMBER | CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,                   do_concept_tandem},
   /* concept_gruesome_tandem */          {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__SHOW_SPLIT,                     do_concept_tandem},
   /* concept_checkerboard */             {0,                                                                                      do_concept_checkerboard},
   /* concept_sel_checkerboard */         {CONCPROP__USE_SELECTOR | CONCPROP__GET_MASK,                                            do_concept_checkerboard},
   /* concept_reverse */                  {0,                                                                                      0},
   /* concept_left */                     {0,                                                                                      0},
   /* concept_grand */                    {0,                                                                                      0},
   /* concept_magic */                    {0,                                                                                      0},
   /* concept_cross */                    {0,                                                                                      0},
   /* concept_single */                   {0,                                                                                      0},
   /* concept_singlefile */               {0,                                                                                      0},
   /* concept_interlocked */              {0,                                                                                      0},
   /* concept_yoyo */                     {0,                                                                                      0},
   /* concept_12_matrix */                {0,                                                                                      0},
   /* concept_16_matrix */                {0,                                                                                      0},
   /* concept_1x2 */                      {0,                                                                                      0},
   /* concept_2x1 */                      {0,                                                                                      0},
   /* concept_2x2 */                      {0,                                                                                      0},
   /* concept_1x3 */                      {0,                                                                                      0},
   /* concept_3x1 */                      {0,                                                                                      0},
   /* concept_3x3 */                      {0,                                                                                      0},
   /* concept_4x4 */                      {0,                                                                                      0},
   /* concept_create_matrix */            {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom,                                            do_concept_expand_some_matrix},
   /* concept_funny */                    {0,                                                                                      0},
   /* concept_randomtrngl */              {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_REVERSE,                      triangle_move},
   /* concept_selbasedtrngl */            {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__PERMIT_REVERSE | CONCPROP__USE_SELECTOR, triangle_move},
   /* concept_split */                    {0,                                                                                      0},
   /* concept_each_1x4 */                 {CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX | CONCPROP__GET_MASK,                       do_concept_do_each_1x4},
   /* concept_diamond */                  {0,                                                                                      0},
   /* concept_triangle */                 {0,                                                                                      0},
   /* concept_do_both_boxes */            {CONCPROP__NO_STEP,                                                                      do_concept_do_both_boxes},
   /* concept_once_removed */             {0,                                                                                      do_concept_once_removed},
   /* concept_do_phantom_2x2 */           {CONCPROP__NEED_4X4 | Nostep_phantom,                                                    do_concept_do_phantom_2x2},
   /* concept_do_phantom_boxes */         {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_do_phantom_boxes},
   /* concept_do_phantom_diamonds */      {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_do_phantom_diamonds},
   /* concept_do_phantom_1x6 */           {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_phantom_1x6},
   /* concept_do_phantom_1x8 */           {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_phantom_1x8},
   /* concept_do_phantom_2x4 */           {CONCPROP__NEED_4X4_1X16 | Standard_matrix_phantom | CONCPROP__PERMIT_MYSTIC,            do_phantom_2x4_concept},
   /* concept_do_phantom_2x3 */           {CONCPROP__NEED_3X4 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_phantom_2x3},
   /* concept_divided_2x4 */              {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_divided_2x4},
   /* concept_divided_2x3 */              {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_divided_2x3},
   /* concept_do_divided_diamonds */      {CONCPROP__NEED_4X6 | CONCPROP__NO_STEP | CONCPROP__GET_MASK | Nostandard_matrix_phantom,do_concept_do_divided_diamonds},
   /* concept_distorted */                {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 do_concept_distorted},
   /* concept_single_diagonal */          {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_single_diagonal},
   /* concept_single_diagonal_sel */      {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        do_concept_single_diagonal},
   /* concept_double_diagonal */          {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 do_concept_double_diagonal},
   /* concept_parallelogram */            {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_parallelogram},
   /* concept_triple_lines */             {CONCPROP__NEED_TRIPLE_1X4 | Standard_matrix_phantom | CONCPROP__PERMIT_MYSTIC,          do_concept_triple_lines},
   /* concept_triple_lines_tog */         {CONCPROP__NEED_TRIPLE_1X4 | Nostandard_matrix_phantom,                                  do_concept_triple_lines_tog},
   /* concept_triple_lines_tog_std */     {CONCPROP__NEED_TRIPLE_1X4 | Standard_matrix_phantom,                                    do_concept_triple_lines_tog},
   /* concept_quad_lines */               {CONCPROP__NEED_4X4_1X16 | Standard_matrix_phantom,                                      do_concept_quad_lines},
   /* concept_quad_lines_tog */           {CONCPROP__NEED_4X4_1X16 | Nostandard_matrix_phantom,                                    do_concept_quad_lines_tog},
   /* concept_quad_lines_tog_std */       {CONCPROP__NEED_4X4_1X16 | Standard_matrix_phantom,                                      do_concept_quad_lines_tog},
   /* concept_quad_boxes */               {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_quad_boxes},
   /* concept_quad_boxes_together */      {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_quad_boxes_tog},
   /* concept_triple_boxes */             {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Nostandard_matrix_phantom | CONCPROP__PERMIT_MYSTIC, do_concept_triple_boxes},
   /* concept_triple_boxes_together */    {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_triple_boxes_tog},
   /* concept_triple_diamonds */          {CONCPROP__NEED_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_triple_diamonds},
   /* concept_triple_diamonds_together */ {CONCPROP__NEED_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_triple_diamonds_tog},
   /* concept_quad_diamonds */            {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_quad_diamonds},
   /* concept_quad_diamonds_together */   {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_quad_diamonds_tog},
   /* concept_in_out_line_3 */            {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__STANDARD | Nostep_phantom,                       do_concept_inner_outer},
   /* concept_in_out_line_4 */            {CONCPROP__NEED_ARG2_MATRIX | CONCPROP__STANDARD | Nostep_phantom,                       do_concept_inner_outer},
   /* concept_in_out_box_3 */             {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom,                                            do_concept_inner_outer},
   /* concept_in_out_box_4 */             {CONCPROP__NEED_ARG2_MATRIX | Nostep_phantom,                                            do_concept_inner_outer},
   /* concept_triple_diag */              {CONCPROP__NEED_BLOB | Nostep_phantom | CONCPROP__STANDARD,                              do_concept_triple_diag},
   /* concept_triple_diag_together */     {CONCPROP__NEED_BLOB | Nostep_phantom | CONCPROP__GET_MASK,                              do_concept_triple_diag_tog},
   /* concept_triple_twin */              {CONCPROP__NEED_4X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       triple_twin_move},
   /* concept_misc_distort */             {CONCPROP__NO_STEP,                                                                      distorted_2x2s_move},
   /* concept_old_stretch */              {0/*CONCPROP__NO_STEP*/,                                                                 do_concept_old_stretch},
   /* concept_new_stretch */              {0/*CONCPROP__NO_STEP*/,                                                                 do_concept_new_stretch},
   /* concept_assume_waves */             {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_assume_waves},
   /* concept_active_phantoms */          {0,                                                                                      do_concept_active_phantoms},
   /* concept_mirror */                   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_mirror},
   /* concept_central */                  {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_central},
   /* concept_snag_mystic */              {CONCPROP__MATRIX_OBLIVIOUS,                                                             do_concept_central},
   /* concept_crazy */                    {CONCPROP__PERMIT_REVERSE,                                                               do_concept_crazy},
   /* concept_frac_crazy */               {CONCPROP__USE_NUMBER | CONCPROP__PERMIT_REVERSE,                                        do_concept_crazy},
   /* concept_fan */                      {0,                                                                                      do_concept_fan},
   /* concept_c1_phantom */               {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_c1_phantom_move},
   /* concept_grand_working */            {CONCPROP__PERMIT_MATRIX | CONCPROP__SET_PHANTOMS,                                       do_concept_grand_working},
   /* concept_centers_or_ends */          {0,                                                                                      do_concept_centers_or_ends},
   /* concept_so_and_so_only */           {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_SELECTOR,                                    so_and_so_only_move},
   /* concept_some_vs_others */           {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__USE_SELECTOR | CONCPROP__SECOND_CALL,            so_and_so_only_move},
   /* concept_stable */                   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_stable},
   /* concept_so_and_so_stable */         {CONCPROP__USE_SELECTOR | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,             do_concept_stable},
   /* concept_frac_stable */              {CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,               do_concept_stable},
   /* concept_so_and_so_frac_stable */    {CONCPROP__USE_SELECTOR | CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,   do_concept_stable},
   /* concept_standard */                 {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX,                   do_concept_standard},
   /* concept_matrix */                   {CONCPROP__MATRIX_OBLIVIOUS,                                                             do_concept_matrix},
   /* concept_double_offset */            {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        do_concept_double_offset},
   /* concept_checkpoint */               {CONCPROP__SECOND_CALL | CONCPROP__PERMIT_REVERSE,                                       do_concept_checkpoint},
   /* concept_on_your_own */              {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              on_your_own_move},
   /* concept_trace */                    {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              do_concept_trace},
   /* concept_ferris */                   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_ferris},
   /* concept_overlapped_diamond */       {CONCPROP__NO_STEP,                                                                      do_concept_overlapped_diamond},
   /* concept_all_8 */                    {0,                                                                                      do_concept_all_8},
   /* concept_centers_and_ends */         {CONCPROP__SECOND_CALL,                                                                  do_concept_centers_and_ends},
   /* concept_twice */                    {CONCPROP__SHOW_SPLIT,                                                                   do_concept_twice},
   /* concept_n_times */                  {CONCPROP__USE_NUMBER | CONCPROP__SHOW_SPLIT,                                            do_concept_twice},
   /* concept_sequential */               {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                                           do_concept_sequential},
   /* concept_special_sequential */       {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                                           do_concept_special_sequential},
   /* concept_meta */                     {CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,                                        do_concept_meta},
   /* concept_so_and_so_begin */          {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,                                          do_concept_so_and_so_begin},
   /* concept_nth_part */                 {CONCPROP__USE_NUMBER | CONCPROP__SHOW_SPLIT | CONCPROP__PERMIT_REVERSE,                 do_concept_meta},
   /* concept_replace_nth_part */         {CONCPROP__USE_NUMBER | CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                    do_concept_replace_nth_part},
   /* concept_interlace */                {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                                           do_concept_interlace},
   /* concept_fractional */               {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT, do_concept_fractional},
   /* concept_rigger */                   {CONCPROP__NO_STEP,                                                                      do_concept_rigger},
   /* concept_common_spot */              {CONCPROP__NO_STEP,                                                                      common_spot_move},
   /* concept_diagnose */                 {0,                                                                                      do_concept_diagnose}};
