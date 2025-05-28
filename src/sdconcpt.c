/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993, 1994  William B. Ackerman.

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

#include "sd.h"
extern map_thing map_inner_box;

int global_tbonetest;
int global_livemask;
int global_selectmask;
int global_tboneselect;



Private void do_concept_expand_1x12_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s1x12) fail("Can't make a 1x12 matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, FALSE, result);
}


Private void do_concept_expand_1x16_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s1x16) fail("Can't make a 1x16 matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, FALSE, result);
}


Private void do_concept_expand_2x6_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x6) fail("Can't make a 2x6 matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, FALSE, result);
}


Private void do_concept_expand_2x8_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x8) fail("Can't make a 2x8 matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   /* We used to turn on the "FINAL__16_MATRIX" call modifier,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the CMD_MISC__EXPLICIT_MATRIX
      bit control the desired effects. */
   move(ss, FALSE, result);
}


Private void do_concept_expand_3x4_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s3x4) fail("Can't make a 3x4 matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   move(ss, FALSE, result);
}


Private void do_concept_expand_4x4_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s4x4) fail("Can't make a 4x4 matrix out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   /* We used to turn on the "FINAL__16_MATRIX" call modifier,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the CMD_MISC__EXPLICIT_MATRIX
      bit control the desired effects. */
   move(ss, FALSE, result);
}


Private void do_concept_expand_4dm_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s_4dmd) fail("Can't make quadruple diamonds out of this.");
   ss->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
   /* We used to turn on the "FINAL__16_MATRIX" call modifier,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the CMD_MISC__EXPLICIT_MATRIX
      bit control the desired effects. */
   move(ss, FALSE, result);
}


typedef struct {
   int map1[8];
   int map2[8];
} phan_map;

Private phan_map map_c1_phan = {{0, 2, 7, 5, 8, 10, 15, 13}, {4, 6, 11, 9, 12, 14, 3, 1}};
Private phan_map map_o_spots = {{10, -1, -1, 1, 2, -1, -1, 9}, {14, -1, -1, 5, 6, -1, -1, 13}};


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

   if (ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED)
      fail("Can't specify phantom in virtual or distorted setup.");

   /* See if this is a "phantom tandem" (or whatever) by searching ahead, skipping comments of course.
      This means we must skip modifiers too, so we check that there weren't any. */

   next_parseptr = process_final_concepts(parseptr->next, FALSE, &junk_concepts);

   if (next_parseptr->concept->kind == concept_tandem || next_parseptr->concept->kind == concept_frac_tandem) {
      /* Find out what kind of tandem call this is. */

      unsigned int what_we_need = 0;

      if (junk_concepts != 0)
         fail("Phantom couples/tandem must not have intervening concpets.");

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
         divided_setup_move(ss, (*map_lists[s1x4][3])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
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
   
   setup1.kind = s2x4;
   setup2.kind = s2x4;
   setup1.rotation = ss->rotation;
   setup2.rotation = ss->rotation+1;
   clear_people(&setup1);
   clear_people(&setup2);

   for (i=0 ; i<8 ; i++) {
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




Private map_thing map_diag2a            = {{5, 7, 21, 15, 17, 19, 9, 3},   {0},                            {0}, {0}, MPKIND__NONE,        1,  s4x6,   s2x4,      1, 0};
Private map_thing map_diag2b            = {{2, 8, 22, 12, 14, 20, 10, 0},  {0},                            {0}, {0}, MPKIND__NONE,        1,  s4x6,   s2x4,      1, 0};




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
   int i;
   int rot, tbonetest;
   diag_map *map_ptr;
   setup a1;
   setup res1;

   tbonetest = global_tboneselect;

   if (!global_tbonetest) {
      result->result_flags = 0;
      result->kind = nothing;
      return;
   }

   rot = (tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;

   map_ptr = rot ? &map_diag1a : &map_diag1b;

   if      (global_selectmask == (global_livemask & 0x0909)) map_ptr = map_ptr->other;
   else if (global_selectmask != (global_livemask & 0x9090))
      tbonetest = -1;   /* Force error. */

   if (ss->kind != s4x4) tbonetest = -1;   /* Force error. */

   if ((tbonetest & 011) == 011) {
      if (parseptr->concept->value.arg1 & 1)
         fail("Designated people are not consistently in a diagonal line.");
      else
         fail("Designated people are not consistently in a diagonal column.");
   }

   ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
   canonicalize_rotation(ss);

   a1 = *ss;

   for (i=0; i<4; i++) {
      (void) copy_person(&a1, i, ss, map_ptr->map[i]);
      clear_person(ss, map_ptr->map[i]);
   }

   a1.kind = s1x4;
   a1.rotation = 0;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   update_id_bits(&a1);
   move(&a1, FALSE, &res1);

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
   int tbonetest;
   map_thing *map_ptr;

   tbonetest = global_tbonetest;

   if      (global_livemask == 0x2A82A8) map_ptr = &map_diag2a;
   else if (global_livemask == 0x505505) map_ptr = &map_diag2b;
   else
      tbonetest = -1;   /* Force error. */

   if (ss->kind != s4x6) tbonetest = -1;   /* Force error. */

   if (parseptr->concept->value.arg1 & 1) {
      if (tbonetest & 010) fail("There are no diagonal lines here.");
   }
   else {
      if (tbonetest & 1) fail("There are no diagonal columns here.");
   }

   divided_setup_move(ss, map_ptr, phantest_ok, TRUE, result);
}


Private void do_concept_double_offset(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int top, bot, ctr;
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

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (ss->kind == s4x4) {
      ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
      canonicalize_rotation(ss);
      divided_setup_move(ss, (*map_lists[s1x4][3])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
      result->rotation -= rot;   /* Flip the setup back. */
   }
   else if (ss->kind == s1x16) {
      if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         else                      fail("There are no columns of 4 here.");
      }
   
      divided_setup_move(ss, (*map_lists[s1x4][3])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
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

   if (linesp)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (cstuff >= 10) {
      if (ss->kind != s1x16) fail("Must have a 1x16 setup for this concept.");
   
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
   
      if (linesp) {
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
   
      overlapped_setup_move(ss, (*map_lists[s1x8][2])[MPKIND__OVERLAP][0],
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
   
      overlapped_setup_move(ss, (*map_lists[s2x4][2])[MPKIND__OVERLAP][1],
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
      if (global_livemask == 07474) map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_R_HALF][0];
      else if (global_livemask == 01717) map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_L_HALF][0];
      else fail("Can't find a parallelogram.");
   }
   else if (ss->kind == s2x8) {
      warn(warn__full_pgram);
      if (global_livemask == 0xF0F0) map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_R_FULL][0];
      else if (global_livemask == 0x0F0F) map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_L_FULL][0];
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

   overlapped_setup_move(ss, (*map_lists[s2x4][2])[MPKIND__OVERLAP][0], m1, m2, m3, result);
}


Private void do_concept_triple_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int m1, m2;

   if (ss->kind != s_3dmd) fail("Must have a triple diamond setup to do this concept.");
   if ((ss->people[1].id1 | ss->people[7].id1) & 010) fail("Can't tell where points of center diamond should work.");

   m1 = 0xE9; m2 = 0xBF;

   /* Look at the center diamond points and put each one in the correct group. */

   if (ss->people[1].id1 & 2) { m1 |= 0x02; m2 &= ~0x01; }
   if (ss->people[7].id1 & 2) { m1 |= 0x10; m2 &= ~0x20; }

   overlapped_setup_move(ss, (*map_lists[s_qtag][1])[MPKIND__OVERLAP][0], m1, m2, 0, result);
}


Private void do_concept_quad_diamonds_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int m1, m2, m3;

   if (ss->kind != s_4dmd) fail("Must have a quadruple diamond setup to do this concept.");
   if ((ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1) & 010)
      fail("Can't tell where points of center diamonds should work.");

   m1 = 0xE9; m2 = 0xA9; m3 = 0xBF;

   /* Look at the center diamond points and put each one in the correct group. */

   if (ss->people[1].id1 & 2) { m1 |= 0x02; m2 &= ~0x01; }
   if (ss->people[2].id1 & 2) { m2 |= 0x02; m3 &= ~0x01; }
   if (ss->people[9].id1 & 2) { m2 |= 0x10; m3 &= ~0x20; }
   if (ss->people[10].id1 & 2) { m1 |= 0x10; m2 &= ~0x20; }

   overlapped_setup_move(ss, (*map_lists[s_qtag][2])[MPKIND__OVERLAP][0], m1, m2, m3, result);
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

   overlapped_setup_move(ss, (*map_lists[s2x4][1])[MPKIND__OVERLAP][0], m1, m2, 0, result);
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

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      else                      fail("There are no columns of 4 here.");
   }

   divided_setup_move(ss, (*map_lists[s1x4][2])[MPKIND__SPLIT][mapidx], phantest_ok, TRUE, result);
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

   if (linesp)
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   if (cstuff >= 10) {
      if (ss->kind != s1x12) fail("Must have a 1x12 setup for this concept.");
   
      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");
   
      if (linesp) {
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
   
      overlapped_setup_move(ss, (*map_lists[s1x8][1])[MPKIND__OVERLAP][0],
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
   
      if (linesp) {
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
   
      overlapped_setup_move(ss, (*map_lists[s2x4][1])[MPKIND__OVERLAP][1],
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

   /* Initially assign the centers to the right or upper (m2) group. */
   m1 = 0xF0; m2 = 0xFF;

   if ((global_livemask & ~0x56A56A) == 0) q = 0;
   else if ((global_livemask & ~0xA95A95) == 0) q = 2;
   else fail("Can't identify triple diagonal setup.");

   /* Look at the center line/column people and put each one in the correct group. */

   map_ptr = maps_3diagwk[q+((cstuff ^ global_tbonetest) & 1)];

   if ((cstuff + 1 - ss->people[map_ptr->map1[0]].id1) & 2) { m2 &= ~0x80 ; m1 |= 0x1; };
   if ((cstuff + 1 - ss->people[map_ptr->map1[1]].id1) & 2) { m2 &= ~0x40 ; m1 |= 0x2; };
   if ((cstuff + 1 - ss->people[map_ptr->map1[2]].id1) & 2) { m2 &= ~0x20 ; m1 |= 0x4; };
   if ((cstuff + 1 - ss->people[map_ptr->map1[3]].id1) & 2) { m2 &= ~0x10 ; m1 |= 0x8; };

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

      the_map = (*map_lists[s2x2][2])[MPKIND__OVERLAP][0];
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

      the_map = (*map_lists[s1x4][2])[MPKIND__OVERLAP][0];
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
   if (ss->kind != s_4dmd) fail("Must have a quadruple diamond setup for this concept.");

   if (((ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1 |
         ss->people[8].id1 | ss->people[9].id1 | ss->people[10].id1 | ss->people[11].id1) & 010) != 0 ||
       ((ss->people[4].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[7].id1 |
         ss->people[12].id1 | ss->people[13].id1 | ss->people[14].id1 | ss->people[15].id1) & 001) != 0)
      fail("People are not facing as in diamonds.");
   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_qtags(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s_4dmd) fail("Must have a quadruple quarter-tag setup for this concept.");
   if (global_tbonetest & 1) fail("People are not facing as in general quarter tags, try using \"standard\".");
   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_divided_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s4x6 ||
         (ss->people[0].id1 | ss->people[2].id1 | ss->people[3].id1 | ss->people[5].id1 |
         ss->people[12].id1 | ss->people[14].id1 | ss->people[15].id1 | ss->people[17].id1) != 0)
      fail("Must have a divided diamond setup for this concept.");

   if (((ss->people[6].id1 | ss->people[8].id1 | ss->people[9].id1 | ss->people[11].id1 |
         ss->people[18].id1 | ss->people[20].id1 | ss->people[21].id1 | ss->people[23].id1) & 001) != 0 ||
       ((ss->people[1].id1 | ss->people[4].id1 | ss->people[7].id1 | ss->people[10].id1 |
         ss->people[13].id1 | ss->people[16].id1 | ss->people[19].id1 | ss->people[22].id1) & 010) != 0)
      fail("People are not facing as in diamonds.");

   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_divided_qtags(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s4x6 || (global_livemask & 0x02D02D) != 0)
      fail("Must have a divided quarter-tag setup for this concept.");

   if (global_tbonetest & 010)
      fail("People are not facing as in general quarter tags, try using \"standard\".");

   divided_setup_move(ss, parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   if ((((parseptr->concept->value.arg2 ^ global_tbonetest) & 1) == 0) || ((global_tbonetest & 011) == 011)) {
      if (parseptr->concept->value.arg2 & 1) fail("There are no 12-matrix divided lines here.");
      else                                       fail("There are no 12-matrix divided columns here.");
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

   if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no lines of 6 here.");
      else                      fail("There are no columns of 6 here.");
   }

   divided_setup_move(ss, (*map_lists[s_1x6][1])[MPKIND__SPLIT][1], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_1x8(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (!((parseptr->concept->value.arg3 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no grand lines here.");
      else                      fail("There are no grand columns here.");
   }

   divided_setup_move(ss, (*map_lists[s1x8][1])[MPKIND__SPLIT][1], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_once_removed(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_thing *the_map;

   switch (ss->kind) {
      case s2x4:
         the_map = (*map_lists[s2x2][1])[MPKIND__REMOVED][0];
         break;
      case s1x8:
         the_map = (*map_lists[s1x4][1])[MPKIND__REMOVED][0];
         break;
      case s1x4:
         the_map = (*map_lists[s_1x2][1])[MPKIND__REMOVED][0];
         break;
      case s_rigger:
         the_map = (*map_lists[sdmd][1])[MPKIND__REMOVED][0];
         break;
      case s_bone:    /* This is for you, Clark. */
         the_map = (*map_lists[s_trngl4][1])[MPKIND__REMOVED][1];
         break;
      case s_ptpd:    /* This is for you, Clark. */
         the_map = &map_p8_tgl4;
         break;
      case s_qtag:
         the_map = (*map_lists[sdmd][1])[MPKIND__REMOVED][1];
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
   setup tempsetup;

   tempsetup = *ss;

   if (tempsetup.kind == s2x4) {
      swap_people(&tempsetup, 1, 2);
      swap_people(&tempsetup, 5, 6);
   }
   else if (tempsetup.kind == s1x8) {
      swap_people(&tempsetup, 3, 6);
      swap_people(&tempsetup, 2, 7);
   }
   else
      fail("Stretched setup call didn't start in 2x4 or 1x8 setup.");

   tempsetup.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   move(&tempsetup, FALSE, result);
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
   /* The restrictions mean different things in different setups.  In some setups, they
      mean things that are unsuitable for the "assume" concept.  In some setups they
      take no action at all.  So we must check the setups on a case-by-case basis. */

   if (parseptr->concept->value.arg2) {
      /* This is a "column-like" restriction. */

      if (((call_restriction) parseptr->concept->value.arg1) == cr_wave_only) {
         switch (ss->kind) {     /* "assume normal columns" */
            case s2x4:
               break;
            default:
               fail("This call is not legal from this formation.");
         }
      }
      else if (((call_restriction) parseptr->concept->value.arg1) == cr_magic_only) {
         switch (ss->kind) {     /* "assume magic columns" */
            case s2x4:
               break;
            default:
               fail("This call is not legal from this formation.");
         }
      }

      check_column_restriction(ss, (call_restriction) parseptr->concept->value.arg1, CAF__RESTR_FORBID);
   }
   else {
      /* This is a "line-like" restriction. */

      if (((call_restriction) parseptr->concept->value.arg1) == cr_wave_only) {
         switch (ss->kind) {     /* "assume waves" */
            case s4x4:  case s2x4:  case s3x4:  case s2x8:  case s2x6:
            case s1x8:  case s1x10: case s1x12: case s1x14: case s1x16:
            case s_1x6: case s1x4:
               break;
            default:
               fail("This call is not legal from this formation.");
         }
      }
      else if (((call_restriction) parseptr->concept->value.arg1) == cr_2fl_only) {
         switch (ss->kind) {     /* "assume two-faced lines" */
            case s2x4:  case s3x4:  case s1x8:  case s1x4:
               break;
            default:
               fail("This call is not legal from this formation.");
         }
      }

      check_line_restriction(ss, (call_restriction) parseptr->concept->value.arg1, CAF__RESTR_FORBID);

      if (((call_restriction) parseptr->concept->value.arg1) == cr_wave_only)
         ss->cmd.cmd_misc_flags |= CMD_MISC__ASSUME_WAVES;
   }

   move(ss, FALSE, result);
}

Private void do_concept_central(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (setup_limits[ss->kind] == 7)
      ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT;
   else if (setup_limits[ss->kind] != 3)
      fail("Need a 4 or 8 person setup for this.");

   ss->cmd.cmd_misc_flags |= CMD_MISC__CENTRAL;
   move(ss, FALSE, result);
}


Private void do_concept_crazy(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i;
   int craziness, highlimit;
   setup tempsetup = *ss;
   setup_command cmd = tempsetup.cmd;    /* We will modify these flags, and, in any case, we need
                                             to rematerialize them at each step. */
   unsigned int finalresultflags = 0;

   /* If we didn't do this check, and we had a 1x4, the "do it on each side"
      stuff would just do it without splitting or thinking anything was unusual,
      while the "do it in the center" code would catch it at present, but might
      not in the future if we add the ability of the concentric schema to mean
      pick out the center 2 from a 1x4.  In any case, if we didn't do this
      check, "1/4 reverse crazy bingo" would be legal from a 2x2. */

   if (setup_limits[tempsetup.kind] != 7)
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

   if (cmd.cmd_frac_flags != 0 && (cmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS) == 0) {
      /* The fractions were meant for us, not the subject call. */

      if (cmd.cmd_frac_flags & 0200000) {
         /* New regime. */

         if ((cmd.cmd_frac_flags & 0307777) != 0200111)  /* Demand that only "N" be present. */
            fail("\"crazy\" is not allowed with fractional or reverse-order concepts.");
   
         i = ((cmd.cmd_frac_flags >> 12) & 7) - 1;
   
         if (i < 0)
            i = 0;     /* We aren't doing parts after all. */
         else {
            /* We are taking the part number indication. */
            highlimit = i+1;          /* This will make us do just the selected part. */
            if (highlimit == craziness) finalresultflags |= RESULTFLAG__DID_LAST_PART;
            cmd.cmd_frac_flags = 0;   /* No fractions for subject, we have consumed them. */
         }
      }
      else {
         /* We handle a few things in the old regime, pending switchover to the new regime.
            This is to allow things like "do the 3rd part stable, crazy mix". */

         if ((cmd.cmd_frac_flags & (CMD_FRAC__FRACTIONALIZE_BIT*0770)) == 0) {
            highlimit = (cmd.cmd_frac_flags / CMD_FRAC__FRACTIONALIZE_BIT) & 7;  /* Do first N parts. */
         }
         else if ((cmd.cmd_frac_flags & (CMD_FRAC__FRACTIONALIZE_BIT*0770)) == CMD_FRAC__FRACTIONALIZE_BIT*0100) {
            i = (cmd.cmd_frac_flags / CMD_FRAC__FRACTIONALIZE_BIT) & 7;  /* Do stuff after part N. */
         }
         else
            fail("Sorry, can't do \"crazy\" with this fractionalization concept.");

         cmd.cmd_frac_flags = 0;   /* No fractions for subject, we have consumed them. */
      }
   }

   if (highlimit <= i || highlimit > craziness) fail("Illegal fraction for \"crazy\".");

   cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;  /* Don't need this any more. */

   for ( ; i<highlimit; i++) {
      tempsetup.cmd = cmd;    /* Get a fresh copy of the command. */

      if ((i ^ parseptr->concept->value.arg1) & 1) {
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


Private void do_concept_fan_or_yoyo(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tempsetup;
   unsigned int finalresultflags = 0;
   /* This is a huge amount of kludgy stuff shoveled in from a variety of sources.
      It needs to be cleaned up and thought about. */

   final_set new_final_concepts;
   parse_block *parseptrcopy;
   callspec_block *callspec;

   parseptrcopy = process_final_concepts(parseptr->next, TRUE, &new_final_concepts);

   if (new_final_concepts || parseptrcopy->concept->kind > marker_end_of_list)
      fail("Can't do \"fan\" or \"yoyo\" followed by another concept or modifier.");

   callspec = parseptrcopy->call;

   if (!callspec || !(callspec->callflags1 & CFLAG1_CAN_BE_FAN_OR_YOYO))
      fail("Can't do \"fan\" or \"yoyo\" with this call.");

   if (ss->cmd.cmd_frac_flags)
      fail("This call can't be fractionalized.");

   /* Step to a wave if necessary.  This is actually only needed for the "yoyo" concept.
      The "fan" concept could take care of itself later.  However, we do them both here. */

   if ((!(ss->cmd.cmd_misc_flags & (CMD_MISC__NO_STEP_TO_WAVE))) &&
         (callspec->callflags1 & (CFLAG1_STEP_TO_WAVE))) {
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;  /* Can only do it once. */
      touch_or_rear_back(ss, FALSE, callspec->callflags1);
   }

   tempsetup = *ss;

   /* If this is "yoyo", do an "arm turn 3/4". */
   if (parseptr->concept->value.arg1 != 0) {
      selector_kind saved_selector = current_selector;
      direction_kind saved_direction = current_direction;
      int saved_number_fields = current_number_fields;

      current_selector = parseptrcopy->selector;
      current_direction = parseptrcopy->direction;
      current_number_fields = parseptrcopy->number;

      /* Do "arm turn 3/4". */
      tempsetup.cmd = ss->cmd;
      tempsetup.cmd.prior_elongation_bits = 0;
      tempsetup.cmd.parseptr = parseptrcopy;
      tempsetup.cmd.callspec = base_calls[2];
      move(&tempsetup, FALSE, result);
      finalresultflags |= result->result_flags;
      tempsetup = *result;

      current_selector = saved_selector;
      current_direction = saved_direction;
      current_number_fields = saved_number_fields;
   }

   /* Now just skip the first part of the call. */
   /* Set the fractionalize field to [1 0 1]. */
   tempsetup.cmd = ss->cmd;
   tempsetup.cmd.cmd_frac_flags |= ((64 | 1)*CMD_FRAC__FRACTIONALIZE_BIT);
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
   int directions[8];
   int n, i, rot, howfar, orig_rotation;

   fractional = parseptr->concept->value.arg2;
   everyone = !parseptr->concept->value.arg1;

   howfar = parseptr->number;
   if (fractional && howfar > 4)
      fail("Can't do fractional stable more than 4/4.");

   new_selector = parseptr->selector;
   n = setup_limits[ss->kind];
   if (n < 0) fail("Sorry, can't do stable starting in this setup.");

   for (i=0; i<=n; i++) {           /* Save current facing directions. */
      unsigned int p = ss->people[i].id1;
      if (p & BIT_PERSON) {
         directions[(p >> 6) & 07] = p;
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

   n = setup_limits[result->kind];
   if (n < 0) fail("Sorry, can't do stable going to this setup.");

   saved_selector = current_selector;
   current_selector = new_selector;

   for (i=0; i<=n; i++) {           /* Restore facing directions of selected people. */
      unsigned int p = result->people[i].id1;
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
                        (directions[(p >> 6) & 07] & (d_mask | STABLE_MASK)),
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
   static int mape[16] = {0, 2, 4, 6, 1, 3, 5, 7};
   static int mapl[16] = {7, 1, 3, 5, 0, 6, 4, 2};
   static int mapb[16] = {1, 3, 5, 7, 0, 2, 4, 6};
   static int mapd[16] = {7, 1, 3, 5, 0, 2, 4, 6};

   int i, rot, offset;
   setup a1;
   setup res1;
   int *map_ptr;

   clear_people(result);

   if (ss->kind != s2x4) fail("Must have a 2x4 setup for 'checker' concept.");

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

   /* Move the people who simply trade, filling in their roll info. */

   (void) copy_rot(result, mape[0+offset], ss, mape[1+offset], 022);
   if (result->people[mape[0+offset]].id1) result->people[mape[0+offset]].id1 = (result->people[mape[0+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
   (void) copy_rot(result, mape[1+offset], ss, mape[0+offset], 022);
   if (result->people[mape[1+offset]].id1) result->people[mape[1+offset]].id1 = (result->people[mape[1+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;
   (void) copy_rot(result, mape[2+offset], ss, mape[3+offset], 022);
   if (result->people[mape[2+offset]].id1) result->people[mape[2+offset]].id1 = (result->people[mape[2+offset]].id1 & (~ROLL_MASK)) | ROLLBITL;
   (void) copy_rot(result, mape[3+offset], ss, mape[2+offset], 022);
   if (result->people[mape[3+offset]].id1) result->people[mape[3+offset]].id1 = (result->people[mape[3+offset]].id1 & (~ROLL_MASK)) | ROLLBITR;

   a1 = *ss;

   a1.kind = (setup_kind) parseptr->concept->value.arg1;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a1.rotation = 0;

   switch (a1.kind) {
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

   for (i=0; i<4; i++) (void) copy_person(&a1, i, ss, map_ptr[i+offset]);

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
   setup_command subsid_cmd;
   subsid_cmd = ss->cmd;
   subsid_cmd.parseptr = parseptr->subsidiary_root;

   if (parseptr->concept->value.arg1) {   /* 0 for normal, 1 for reverse checkpoint. */
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
   int call_index;

   if (ss->cmd.cmd_frac_flags != 0)
      fail("Can't stack meta or fractional concepts.");

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   for (call_index=0; call_index<2; call_index++) {
      unsigned int save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
      result->cmd = ss->cmd;      /* The call we wish to execute (we will fix it up shortly). */

      if (call_index != 0) {
         result->cmd.cmd_frac_flags = 0;  /* No fractions to 2nd call. */
         result->cmd.parseptr = parseptr->subsidiary_root;
      }

      result->cmd.prior_elongation_bits = save_elongation;
      do_call_in_series(result, FALSE, TRUE, FALSE);
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
      unsigned int save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
      unsigned int saved_last_flag = 0;
      result->cmd = ss->cmd;      /* The call we wish to execute (we will fix it up shortly). */

      if ((call_index ^ parseptr->concept->value.arg1) != 0) {   /* Maybe do them in reverse order. */
         result->cmd.cmd_frac_flags = 0;  /* No fractions to 2nd call. */
         saved_last_flag = result->result_flags & RESULTFLAG__DID_LAST_PART;
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
   int call_index;

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   for (call_index=0; call_index<2; call_index++) {
      unsigned int save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
      result->cmd = ss->cmd;      /* The call we wish to execute. */
      result->cmd.prior_elongation_bits = save_elongation;
      do_call_in_series(result, FALSE, TRUE, FALSE);
   }
}


Private void do_concept_trace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int r1, r2, r3, r4, rot1, rot2, rot3, rot4;
   unsigned int finalresultflags;
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

   update_id_bits(&a1);
   move(&a1, FALSE, &res1);
   update_id_bits(&a2);
   move(&a2, FALSE, &res2);
   update_id_bits(&a3);
   move(&a3, FALSE, &res3);
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
   divided_setup_move(ss, (*map_lists[s2x2][3])[parseptr->concept->value.arg1][0], phantest_ok, TRUE, result);
}


Private void do_concept_inner_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");
   divided_setup_move(ss, &map_inner_box, phantest_ok, TRUE, result);
   install_person(result, 0, ss, 0);
   install_person(result, 1, ss, 1);
   install_person(result, 6, ss, 6);
   install_person(result, 7, ss, 7);
   install_person(result, 8, ss, 8);
   install_person(result, 9, ss, 9);
   install_person(result, 14, ss, 14);
   install_person(result, 15, ss, 15);
}


Private void do_concept_do_both_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind == s2x4) {
      divided_setup_move(ss, parseptr->concept->value.maps, phantest_ok, TRUE, result);
   }
   else if (ss->kind == s3x4 && parseptr->concept->value.arg2) {
      /* distorted_2x2s_move will notice that concept is funny and will do the right thing. */
      distorted_2x2s_move(ss, parseptr->concept, result);
      reinstate_rotation(ss, result);
   }
   else
      fail("Need a 2x4 setup to do this concept.");
}


Private void do_concept_do_each_1x4(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind == s2x4) {
      divided_setup_move(ss, (*map_lists[s1x4][1])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
   }
   else if (ss->kind == s1x8) {
      divided_setup_move(ss, (*map_lists[s1x4][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
   }
   else
      fail("Need a 2x4 or 1x8 setup for this concept.");
}


Private void do_concept_triple_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x6) fail("Must have a 2x6 setup for this concept.");
   divided_setup_move(ss, (*map_lists[s2x2][2])[parseptr->concept->value.arg1][0], phantest_ok, TRUE, result);
}


Private void do_concept_centers_or_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   calldef_schema schema;
   int k = parseptr->concept->value.arg1;

   switch (k&6) {
      case 2:
         schema = schema_concentric_6_2;
         break;
      case 4:
         schema = schema_concentric_2_6;
         break;
      default:
         if (setup_limits[ss->kind] == 3)
            schema = schema_single_concentric;
         else
            schema = schema_concentric;
         break;
   }

   if (k&1)
      concentric_move(ss, (setup_command *) 0, &ss->cmd,
               schema, 0, 0, result);
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

   switch (parseptr->concept->value.arg1) {
      case 2:
         schema = schema_concentric_6_2;
         break;
      case 3:
         schema = schema_concentric_2_6;
         break;
      default:
         if (setup_limits[ss->kind] == 3)
            schema = schema_single_concentric;
         else
            schema = schema_concentric;
         break;
   }

   subsid_cmd = ss->cmd;
   subsid_cmd.parseptr = parseptr->subsidiary_root;
   concentric_move(ss, &ss->cmd, &subsid_cmd, schema, 0, 0, result);
}


Private void do_concept_triple_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s_3dmd) fail("Must have a triple diamond setup to do this concept.");
   divided_setup_move(ss, (*map_lists[sdmd][2])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
}


Private void do_concept_quad_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s_4dmd) fail("Must have a quadruple diamond setup to do this concept.");
   divided_setup_move(ss, (*map_lists[sdmd][3])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
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

   divided_setup_move(&temp, (*map_lists[s1x4][2])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);

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

      if (
            ss->kind != s4x4 ||
            (  ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1 |
               ss->people[8].id1 | ss->people[11].id1 | ss->people[12].id1 | ss->people[15].id1 != 0) ||
            (ss->people[ 1].id1 != 0 && ((ss->people[ 1].id1 ^ d_west) & d_mask) != 0) ||
            (ss->people[ 2].id1 != 0 && ((ss->people[ 2].id1 ^ d_west) & d_mask) != 0) ||
            (ss->people[ 5].id1 != 0 && ((ss->people[ 5].id1 ^ d_north) & d_mask) != 0) ||
            (ss->people[ 6].id1 != 0 && ((ss->people[ 6].id1 ^ d_north) & d_mask) != 0) ||
            (ss->people[ 9].id1 != 0 && ((ss->people[ 9].id1 ^ d_east) & d_mask) != 0) ||
            (ss->people[10].id1 != 0 && ((ss->people[10].id1 ^ d_east) & d_mask) != 0) ||
            (ss->people[13].id1 != 0 && ((ss->people[13].id1 ^ d_south) & d_mask) != 0) ||
            (ss->people[14].id1 != 0 && ((ss->people[14].id1 ^ d_south) & d_mask) != 0))
         fail("Must be in a squared set.");

      divided_setup_move(ss, (*map_lists[s2x2][1])[MPKIND__ALL_8][0], phantest_ok, TRUE, result);

      if (result->kind == s4x4)
         goto check_col_ending;
   }
   else {

      /* This is "all 8" or "all 8 (diamond)". */

      if (ss->kind == s_thar) {
         /* Either one is legal in a thar. */
         if (key == 1)
            divided_setup_move(ss, (*map_lists[s1x4][1])[MPKIND__ALL_8][0], phantest_ok, TRUE, result);
         else
            divided_setup_move(ss, (*map_lists[sdmd][1])[MPKIND__ALL_8][0], phantest_ok, TRUE, result);

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
         unsigned int t1, t2, tl, tc;

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

         divided_setup_move(ss, (*map_lists[s2x2][1])[MPKIND__ALL_8][0], phantest_ok, TRUE, result);

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
   final_set new_final_concepts;
   parse_block *parseptrcopy;
   parse_block *parseptrcopycopy;
   unsigned int finalresultflags = 0;
   unsigned int index;
   int key = parseptr->concept->value.arg1;
   unsigned int subject_props = 0;
   unsigned int craziness_restraint = 0;

   /* key =
      random <concept>         : 0
      reverse random <concept> : 1
      piecewise <concept>      : 2
      start <concept>          : 3
      finish                   : 4
      reverse order            : 5 */

   if (key == 5) {
      /* This is "reverse order". */

      /* We don't allow stacking with the old regime. */
      if (ss->cmd.cmd_frac_flags & CMD_FRAC__FRACTIONALIZE_MASK)
         fail("Can't stack meta or fractional concepts.");

      if ((ss->cmd.cmd_frac_flags & 0200000) == 0)
         ss->cmd.cmd_frac_flags = 0200111;

      ss->cmd.cmd_frac_flags ^= 0100000;
      move(ss, FALSE, result);
      return;
   }
   else if (key == 4) {
      /* This is "finish".  Do the call after the first part, with the special indicator
         saying that this was invoked with "finish", so that the flag will be checked. */

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");
      /* Set the fractionalize field to [3 0 1]. */
      ss->cmd.cmd_frac_flags |= ((192|1)*CMD_FRAC__FRACTIONALIZE_BIT);
      move(ss, FALSE, result);
      normalize_setup(result, simple_normalize);
      return;
   }

   *result = *ss;

   /* Scan the modifiers, remembering them and their end point.  The reason for this is to
      avoid getting screwed up by a comment, which counts as a modifier.  YUK!!!!!!
      This code used to have the beginnings of stuff to do it really right.  It isn't
      worth it, and isn't worth holding up "random left" for.  In any case, the stupid
      handling of comments will go away soon. */

   if (ss->cmd.parseptr->concept->kind == concept_comment)
      fail("Please don't put a comment after a meta-concept.  Sorry.");

   parseptrcopy = process_final_concepts(ss->cmd.parseptr, FALSE, &new_final_concepts);

   /* Find out whether the next concept (the one that will be "random" or whatever)
      is a modifier or a "real" concept. */

   if (new_final_concepts != 0) {
      parseptrcopy = ss->cmd.parseptr; /* Lots of comment-aversion code being punted
                                          here, but it's just too hairy, and we're
                                          going to change all that stuff anyway. */
   }
   else {
      concept_kind k = parseptrcopy->concept->kind;

      if (k <= marker_end_of_list)
         fail("Sorry, can't do this with this concept.");

      /* It seems to be real.  Examine it in more detail. */

      if (concept_table[k].concept_action == 0)
         fail("Sorry, can't do this with this concept.");

      if (k == concept_crazy || k == concept_frac_crazy)
         craziness_restraint = CMD_MISC__RESTRAIN_CRAZINESS;

      subject_props = concept_table[k].concept_prop;
   }

   if (key == 3) {
      /* Key = 3 is special: we select the first part with the concept, using a
         fractionalize field of [0 0 1], and then the rest of the call without the
         concept, using a fractionalize field of [1 0 1]. */

      setup tttt = *result;

      /* See the comment in "do_concept_nth_part". */

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");

      if (subject_props & CONCPROP__SECOND_CALL)
         fail("Can't use a concept that takes a second call.");

      /* Do the call with the concept. */
      /* Set the fractionalize field to [0 0 1].  This will execute the first part of the call. */
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags |= CMD_FRAC__FRACTIONALIZE_BIT;
      tttt.cmd.cmd_misc_flags |= craziness_restraint;
      tttt.cmd.parseptr = parseptrcopy;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);

      tttt = *result;
      /* Do the call without the concept. */
      /* Set the fractionalize field to [1 0 1].  This will execute the rest of the call. */
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags |= (65*CMD_FRAC__FRACTIONALIZE_BIT);
      tttt.cmd.parseptr = parseptrcopy->next;
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   }
   else {
      /* Otherwise, this is the "random", "reverse random", or "piecewise" concept.
         Repeatedly execute parts of the call, skipping the concept where required. */

      if ((subject_props & CONCPROP__SECOND_CALL) && parseptrcopy->concept->kind != concept_special_sequential)
         fail("Can't use a concept that takes a second call.");

      /* We don't allow anything, under either the old or the new regime. */

      if (ss->cmd.cmd_frac_flags)
         fail("Can't stack meta or fractional concepts.");

      /* Initialize. */
      if ((ss->cmd.cmd_frac_flags & 0200000) == 0)
         ss->cmd.cmd_frac_flags = 0200111;
   
      index = 0;

      do {
         setup tttt = *result;

         index++;
         parseptrcopycopy = parseptrcopy;
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
         tttt.cmd.cmd_frac_flags |= index << 12;
         tttt.cmd.parseptr = parseptrcopycopy;
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }
      while (!(result->result_flags & RESULTFLAG__DID_LAST_PART));
   }

   result->result_flags = finalresultflags & ~3;
}


Private void do_concept_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   final_set new_final_concepts;
   parse_block *parseptrcopy;
   setup tttt;
   unsigned int finalresultflags = 0;
   int key = parseptr->concept->value.arg1;

   /* Where do we draw the line at stacking of fractionalization concepts?
      We draw it right here.  We do not allow concepts that specifically indicate
      part numbers to be mixed with other fractional concepts.  There is simply no
      accepted meaning for things like "do the 3rd part tandem, interlace X with Y"
      or "skip the 2nd part, do the last 3/4 of X".  So we do not allow any
      fractionalization bits, and we use the old regime so that no future mixing
      can take place. */

   if (ss->cmd.cmd_frac_flags)
      fail("Can't stack meta or fractional concepts.");

   *result = *ss;

   /* key =
      do the Nth part <concept> : 0
      skip the Nth part         : 1 */

   if (key == 1) {
      /* Do the initial part, if any. */
   
      if (parseptr->number > 1) {
         tttt = *result;
         /* Set the fractionalize field to [0 0 parts-to-do-first]. */
         tttt.cmd = ss->cmd;
         tttt.cmd.cmd_frac_flags |= ((parseptr->number-1)*CMD_FRAC__FRACTIONALIZE_BIT);
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }
   
      /* Do the final part, if there is more. */
   
      if (!(result->result_flags & RESULTFLAG__DID_LAST_PART)) {
         tttt = *result;
         /* Set the fractionalize field to [1 0 parts-to-do-first+1]. */
         tttt.cmd = ss->cmd;
         tttt.cmd.cmd_frac_flags |= ((64|parseptr->number)*CMD_FRAC__FRACTIONALIZE_BIT);
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }
   }
   else {
      /* Scan the "final" concepts, remembering them and their end point. */
      parseptrcopy = process_final_concepts(parseptr->next, TRUE, &new_final_concepts);
   
      /* These are the concepts that we are interested in. */
   
      if (parseptrcopy->concept->kind <= marker_end_of_list)
         fail("Sorry, can't do this with this concept.");
   
      if (new_final_concepts) fail("Sorry, can't do this with this concept.");
   
      /* Examine the concept.  It must be a real one. */
   
      if (concept_table[parseptrcopy->concept->kind].concept_action == 0)
         fail("Sorry, can't do this with this concept.");
   
      if (concept_table[parseptrcopy->concept->kind].concept_prop & CONCPROP__SECOND_CALL)
         fail("Can't use a concept that takes a second call.");
   
      /* Do the initial part, if any. */
   
      if (parseptr->number > 1) {
         tttt = *result;
         /* Skip over the concept. */
         /* Set the fractionalize field to [0 0 parts-to-do-normally]. */
         tttt.cmd = ss->cmd;
         tttt.cmd.cmd_frac_flags |= ((parseptr->number-1)*CMD_FRAC__FRACTIONALIZE_BIT);
         tttt.cmd.parseptr = parseptrcopy->next;
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }
   
      /* Do the part of the call that needs the concept. */
   
      tttt = *result;
      /* Do the concept. */
      /* Set the fractionalize field to do the Nth part, using the new regime. */
      tttt.cmd = ss->cmd;
      tttt.cmd.parseptr = parseptrcopy;
      tttt.cmd.cmd_frac_flags = 0200111 | (parseptr->number << 12);
      move(&tttt, FALSE, result);
      finalresultflags |= result->result_flags;
      normalize_setup(result, simple_normalize);
   
      /* Do the final part, if there is more. */
   
      if (!(result->result_flags & RESULTFLAG__DID_LAST_PART)) {
         tttt = *result;
         /* Skip over the concept. */
         /* Set the fractionalize field to [1 0 parts-to-do-normally+1]. */
         tttt.cmd = ss->cmd;
         tttt.cmd.parseptr = parseptrcopy->next;
         tttt.cmd.cmd_frac_flags |= ((64|parseptr->number)*CMD_FRAC__FRACTIONALIZE_BIT);
         move(&tttt, FALSE, result);
         finalresultflags |= result->result_flags;
         normalize_setup(result, simple_normalize);
      }
   }

   result->result_flags = finalresultflags & ~3;
}


Private void do_concept_replace_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tttt;
   unsigned int finalresultflags = 0;
   int stopindex;

   *result = *ss;

   /* Do the initial part, if any. */

   if (parseptr->concept->value.arg1)
      stopindex = parseptr->number;      /* Interrupt after Nth part. */
   else
      stopindex = parseptr->number-1;    /* Replace Nth part. */

   if (stopindex > 0) {
      tttt = *result;
      /* Skip over the concept. */
      /* Set the fractionalize field to [0 0 parts-to-do-normally]. */
      tttt.cmd = ss->cmd;
      tttt.cmd.cmd_frac_flags |= ((stopindex)*CMD_FRAC__FRACTIONALIZE_BIT);
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
   tttt.cmd.cmd_frac_flags |= ((64|parseptr->number)*CMD_FRAC__FRACTIONALIZE_BIT);
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
   unsigned int index;
   unsigned int first_doneflag, second_doneflag;
   unsigned int save_elongation;

   first_doneflag = 0;
   second_doneflag = 0;
   index = 0;

   if (ss->cmd.cmd_frac_flags)
      fail("Can't stack meta or fractional concepts.");

   /* Initialize. */
   if ((ss->cmd.cmd_frac_flags & 0200000) == 0)
      ss->cmd.cmd_frac_flags = 0200111;

   *result = *ss;
   result->result_flags = RESULTFLAG__SPLIT_AXIS_MASK;   /* Seed the result. */

   do {
      index++;

      if (first_doneflag == 0) {
         /* Do the indicated part of the first call. */
         save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
         result->cmd = ss->cmd;
         result->cmd.cmd_frac_flags |= index << 12;
         result->cmd.prior_elongation_bits = save_elongation;
         do_call_in_series(result, FALSE, TRUE, FALSE);
         first_doneflag = result->result_flags & RESULTFLAG__DID_LAST_PART;
      }
      else if (second_doneflag == 0)
         warn(warn__bad_interlace_match);

      if (second_doneflag == 0) {
         save_elongation = result->cmd.prior_elongation_bits;   /* Save it temporarily. */
         /* Do the indicated part of the second call. */
         result->cmd = ss->cmd;
         result->cmd.cmd_frac_flags |= index << 12;
         result->cmd.parseptr = parseptr->subsidiary_root;
         result->cmd.prior_elongation_bits = save_elongation;
         do_call_in_series(result, FALSE, TRUE, FALSE);
         second_doneflag = result->result_flags & RESULTFLAG__DID_LAST_PART;
      }
      else if (first_doneflag == 0)
         warn(warn__bad_interlace_match);
   }
   while ((first_doneflag & second_doneflag) == 0);
}



Private int gcd(int a, int b)    /* a <= b */
{
   if (a==0) return b;
   else {
      int rem = b % a;
      if (rem==0) return a;
      else return gcd(rem, a);
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

   int numer, denom;
   int s_numer, s_denom, e_numer, e_denom, divisor;

   numer = parseptr->number;
   denom = numer >> 4;
   numer &= 0xF;

   /* We don't allow stacking with the old regime. */
   if (ss->cmd.cmd_frac_flags & CMD_FRAC__FRACTIONALIZE_MASK)
      fail("Can't stack meta or fractional concepts.");

   if ((ss->cmd.cmd_frac_flags & 0200000) == 0)
      ss->cmd.cmd_frac_flags = 0200111;

   /* Check that user isn't doing something stupid. */
   if (numer <= 0 || numer >= denom)
      fail("Illegal fraction.");

   s_numer = (ss->cmd.cmd_frac_flags & 07000) >> 9;        /* Start point. */
   s_denom = (ss->cmd.cmd_frac_flags & 0700) >> 6;
   e_numer = (ss->cmd.cmd_frac_flags & 070) >> 3;          /* Stop point. */
   e_denom = (ss->cmd.cmd_frac_flags & 07);

   /* Xor the "reverse" bit with the first/last fraction indicator. */
   if ((parseptr->concept->value.arg1 ^ (ss->cmd.cmd_frac_flags >> 15)) & 1) {
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

   if (s_numer > 7 || s_denom > 7 || e_numer > 7 || e_denom > 7)
      fail("Fractions are too complicated.");

   ss->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~07777) | (s_numer<<9) | (s_denom<<6) | (e_numer<<3) | e_denom;
   ss->cmd.parseptr = parseptr->next;
   ss->cmd.callspec = NULLCALLSPEC;
   ss->cmd.cmd_final_flags = 0;
   move(ss, FALSE, result);
}


Private void do_concept_so_and_so_begin(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   unsigned int finalresultflags;
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
   
   if (setup_limits[ss->kind] < 0) fail("Can't identify people in this setup.");
   for (i=0; i<setup_limits[ss->kind]+1; i++) {
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
   /* Set the fractionalize field to [0 0 1].  This will execute the first part of the call. */
   setup1.cmd.cmd_frac_flags |= CMD_FRAC__FRACTIONALIZE_BIT;

   /* The selected people execute the first part of the call. */

   move(&setup1, FALSE, &the_setups[0]);
   the_setups[1] = setup2;

    /* Give the people who didn't move the same result flags as those who did.
      This is imprtant for the "did last part" check. */
   the_setups[1].result_flags = the_setups[0].result_flags;
   the_setups[1].result_flags = get_multiple_parallel_resultflags(the_setups, 2);

   merge_setups(&the_setups[0], merge_c1_phantom, &the_setups[1]);
   finalresultflags = the_setups[1].result_flags;

   normalize_setup(&the_setups[1], simple_normalize);
   the_setups[1].cmd = ss->cmd;   /* Just in case it got messed up, which shouldn't have happened. */

   /* Set the fractionalize field to [1 0 1].  This will execute the rest of the call. */
   the_setups[1].cmd.cmd_frac_flags |= (65*CMD_FRAC__FRACTIONALIZE_BIT);
   move(&the_setups[1], FALSE, result);
   finalresultflags |= result->result_flags;
   normalize_setup(result, simple_normalize);
  
   result->result_flags = finalresultflags & ~3;
}


Private void do_concept_misc_distort(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   distorted_2x2s_move(ss, parseptr->concept, result);
   reinstate_rotation(ss, result);
}


Private void do_concept_concentric(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   concentric_move(ss, &ss->cmd, &ss->cmd,
         (calldef_schema) parseptr->concept->value.arg1, 0, DFM1_CONC_CONCENTRIC_RULES, result);
}


typedef struct {
   setup_kind a;
   int b;
} map_finder;


Private map_finder sc_2x4 = {s1x4, 1};
Private map_finder sc_1x8 = {s1x4, 0};
Private map_finder sc_qtg = {sdmd, 1};
Private map_finder sc_ptp = {sdmd, 0};



Private void do_concept_single_concentric(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   map_finder *mf;

   switch (ss->kind) {
      case s2x4:   mf = &sc_2x4; break;
      case s1x8:   mf = &sc_1x8; break;
      case s_qtag: mf = &sc_qtg; break;
      case s_ptpd: mf = &sc_ptp; break;
      case s1x4: case sdmd:
         concentric_move(ss, &ss->cmd, &ss->cmd,
               (calldef_schema) parseptr->concept->value.arg1, 0, DFM1_CONC_CONCENTRIC_RULES, result);
         return;
      default:
         fail("Can't figure out how to do single concentric here.");
   }

   ss->cmd.parseptr = parseptr;    /* Reset it to execute this same concept again, until it doesn't have to split any more. */
   divided_setup_move(ss, (*map_lists[mf->a][1])[MPKIND__SPLIT][mf->b], phantest_ok, TRUE, result);
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
         parseptr->concept->value.arg4,    /* tandem=0 couples=1 siamese=2 */
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
   parse_block *this_concept_parse_block = ss->cmd.parseptr;
   concept_descriptor *this_concept = this_concept_parse_block->concept;
   concept_kind this_kind = this_concept->kind;
   concept_table_item *this_table_item = &concept_table[this_kind];

   /* We know the following about the incoming setup:
      ss->cmd.parseptr has the stuff, including the concept we are going to try to do.
      ss->cmd.callspec is null.
      ss->cmd.cmd_misc_flags has lots of stuff.
      ss->cmd.cmd_final_flags is nearly null.  However, it may contain FINAL__MUST_BE_TAG
         or FINAL__MUST_BE_SCOOT.  The rest of this file used to just ignore those,
         passing zero for the final commands.   This may be a bug.  In any case, we
         have now preserved even those two flags in cmd_final_flags, so things can
         possibly get better. */

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT) && !(this_table_item->concept_prop & CONCPROP__PERMIT_MATRIX))
      fail("\"Matrix\" concept must be followed by applicable concept.");

   concept_func = this_table_item->concept_action;
   ss->cmd.parseptr = this_concept_parse_block->next;

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
      substandard_concptptr = process_final_concepts(this_concept_parse_block->next, TRUE, &junk_concepts);
   
      /* If we hit "matrix", do a little extra stuff and continue. */

      if (junk_concepts == 0 && substandard_concptptr->concept->kind == concept_matrix) {
         ss->cmd.cmd_misc_flags |= CMD_MISC__MATRIX_CONCEPT;
         substandard_concptptr = process_final_concepts(substandard_concptptr->next, TRUE, &junk_concepts);
      }

      if ((junk_concepts != 0) || (!(concept_table[substandard_concptptr->concept->kind].concept_prop & CONCPROP__STANDARD)))
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

      if (setup_limits[ss->kind] < 0) fail("Can't do this concept in this setup.");
   
      {
         int i, j;
         selector_kind saved_selector = current_selector;
   
         current_selector = this_concept_parse_block->selector;
   
         for (i=0, j=1; i<=setup_limits[ss->kind]; i++, j<<=1) {
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
      canonicalize_rotation(result);
      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_MASK;  /* **** For now. */
      return TRUE;
   }

   if (!(ss->cmd.cmd_misc_flags & CMD_MISC__NO_EXPAND_MATRIX))
      do_matrix_expansion(ss, this_table_item->concept_prop, FALSE);

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
      int i, j;
      long_boolean doing_select;
      selector_kind saved_selector = current_selector;

      if (setup_limits[ss->kind] < 0) fail("Can't do this concept in this setup.");

      global_tbonetest = 0;
      global_livemask = 0;
      global_selectmask = 0;
      global_tboneselect = 0;
      doing_select = this_table_item->concept_prop & CONCPROP__USE_SELECTOR;

      if (doing_select) {
         current_selector = this_concept_parse_block->selector;
      }

      for (i=0, j=1; i<=setup_limits[ss->kind]; i++, j<<=1) {
         int p = ss->people[i].id1;
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
   canonicalize_rotation(result);
   if (!(this_table_item->concept_prop & CONCPROP__SHOW_SPLIT))
      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_MASK;
   return TRUE;
}


#define Standard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX | CONCPROP__STANDARD)
#define Nostandard_matrix_phantom (CONCPROP__SET_PHANTOMS | CONCPROP__PERMIT_MATRIX)


/* Beware!!  This table must be keyed to definition of "concept_kind" in sd.h . */

concept_table_item concept_table[] = {
   /* concept_another_call_next_mod */    {0,                                                                                      0},
   /* concept_mod_declined */             {0,                                                                                      0},
   /* marker_end_of_list */               {0,                                                                                      0},
   /* concept_comment */                  {0,                                                                                      0},
   /* concept_concentric */               {0,                                                                                      do_concept_concentric},
   /* concept_single_concentric */        {CONCPROP__SHOW_SPLIT,                                                                   do_concept_single_concentric},
   /* concept_tandem */                   {CONCPROP__SHOW_SPLIT,                                                                   do_concept_tandem},
   /* concept_gruesome_tandem */          {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__SHOW_SPLIT,                     do_concept_tandem},
   /* concept_some_are_tandem */          {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,                                          do_concept_tandem},
   /* concept_frac_tandem */              {CONCPROP__USE_NUMBER | CONCPROP__SHOW_SPLIT,                                            do_concept_tandem},
   /* concept_gruesome_frac_tandem */     {CONCPROP__USE_NUMBER | CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__SHOW_SPLIT, do_concept_tandem},
   /* concept_some_are_frac_tandem */     {CONCPROP__USE_NUMBER | CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,                   do_concept_tandem},
   /* concept_checkerboard */             {0,                                                                                      do_concept_checkerboard},
   /* concept_reverse */                  {0,                                                                                      0},
   /* concept_left */                     {0,                                                                                      0},
   /* concept_grand */                    {0,                                                                                      0},
   /* concept_magic */                    {0,                                                                                      0},
   /* concept_cross */                    {0,                                                                                      0},
   /* concept_single */                   {0,                                                                                      0},
   /* concept_singlefile */               {0,                                                                                      0},
   /* concept_interlocked */              {0,                                                                                      0},
   /* concept_12_matrix */                {0,                                                                                      0},
   /* concept_16_matrix */                {0,                                                                                      0},
   /* concept_1x2 */                      {0,                                                                                      0},
   /* concept_2x1 */                      {0,                                                                                      0},
   /* concept_2x2 */                      {0,                                                                                      0},
   /* concept_1x3 */                      {0,                                                                                      0},
   /* concept_3x1 */                      {0,                                                                                      0},
   /* concept_3x3 */                      {0,                                                                                      0},
   /* concept_4x4 */                      {0,                                                                                      0},
   /* concept_1x12_matrix */              {CONCPROP__NEED_1X12 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                       do_concept_expand_1x12_matrix},
   /* concept_1x16_matrix */              {CONCPROP__NEED_1X16 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                       do_concept_expand_1x16_matrix},
   /* concept_2x6_matrix */               {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                        do_concept_expand_2x6_matrix},
   /* concept_2x8_matrix */               {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                        do_concept_expand_2x8_matrix},
   /* concept_3x4_matrix */               {CONCPROP__NEED_3X4 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                        do_concept_expand_3x4_matrix},
   /* concept_4x4_matrix */               {CONCPROP__NEED_4X4 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                        do_concept_expand_4x4_matrix},
   /* concept_4dmd_matrix */              {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                       do_concept_expand_4dm_matrix},
   /* concept_funny */                    {0,                                                                                      0},
   /* concept_randomtrngl */              {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 triangle_move},
   /* concept_selbasedtrngl */            {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        triangle_move},
   /* concept_split */                    {0,                                                                                      0},
   /* concept_each_1x4 */                 {CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX,                                            do_concept_do_each_1x4},
   /* concept_diamond */                  {0,                                                                                      0},
   /* concept_triangle */                 {0,                                                                                      0},
   /* concept_do_both_boxes */            {CONCPROP__NO_STEP,                                                                      do_concept_do_both_boxes},
   /* concept_once_removed */             {0,                                                                                      do_concept_once_removed},
   /* concept_do_phantom_2x2 */           {CONCPROP__NEED_4X4 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                        do_concept_do_phantom_2x2},
   /* concept_do_phantom_boxes */         {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_do_phantom_boxes},
   /* concept_do_phantom_diamonds */      {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_do_phantom_diamonds},
   /* concept_do_phantom_qtags */         {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Standard_matrix_phantom,                      do_concept_do_phantom_qtags},
   /* concept_do_phantom_1x6 */           {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_phantom_1x6},
   /* concept_do_phantom_1x8 */           {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_phantom_1x8},
   /* concept_do_phantom_2x4 */           {CONCPROP__NEED_4X4_1X16 | Standard_matrix_phantom,                                      do_phantom_2x4_concept},
   /* concept_do_phantom_2x3 */           {CONCPROP__NEED_3X4 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_phantom_2x3},
   /* concept_divided_2x4 */              {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_divided_2x4},
   /* concept_divided_2x3 */              {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_divided_2x3},
   /* concept_do_divided_diamonds */      {CONCPROP__NEED_4X6 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_do_divided_diamonds},
   /* concept_do_divided_qtags */         {CONCPROP__NEED_4X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       do_concept_do_divided_qtags},
   /* concept_distorted */                {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 distorted_move},
   /* concept_single_diagonal */          {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        do_concept_single_diagonal},
   /* concept_double_diagonal */          {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 do_concept_double_diagonal},
   /* concept_parallelogram */            {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_parallelogram},
   /* concept_triple_lines */             {CONCPROP__NEED_3X4_1X12 | Standard_matrix_phantom,                                      do_concept_triple_lines},
   /* concept_triple_lines_tog */         {CONCPROP__NEED_3X4_1X12 | Nostandard_matrix_phantom,                                    do_concept_triple_lines_tog},
   /* concept_triple_lines_tog_std */     {CONCPROP__NEED_3X4_1X12 | Standard_matrix_phantom,                                      do_concept_triple_lines_tog},
   /* concept_quad_lines */               {CONCPROP__NEED_4X4_1X16 | Standard_matrix_phantom,                                      do_concept_quad_lines},
   /* concept_quad_lines_tog */           {CONCPROP__NEED_4X4_1X16 | Nostandard_matrix_phantom,                                    do_concept_quad_lines_tog},
   /* concept_quad_lines_tog_std */       {CONCPROP__NEED_4X4_1X16 | Standard_matrix_phantom,                                      do_concept_quad_lines_tog},
   /* concept_quad_boxes */               {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_quad_boxes},
   /* concept_quad_boxes_together */      {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_quad_boxes_tog},
   /* concept_triple_boxes */             {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_triple_boxes},
   /* concept_triple_boxes_together */    {CONCPROP__NEED_2X6 | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                     do_concept_triple_boxes_tog},
   /* concept_triple_diamonds */          {CONCPROP__NEED_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_triple_diamonds},
   /* concept_triple_diamonds_together */ {CONCPROP__NEED_3DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_triple_diamonds_tog},
   /* concept_quad_diamonds */            {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_quad_diamonds},
   /* concept_quad_diamonds_together */   {CONCPROP__NEED_4DMD | CONCPROP__NO_STEP | Nostandard_matrix_phantom,                    do_concept_quad_diamonds_tog},
   /* concept_inner_boxes */              {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS,                        do_concept_inner_boxes},
   /* concept_triple_diag */              {CONCPROP__NEED_BLOB | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,  do_concept_triple_diag},
   /* concept_triple_diag_together */     {CONCPROP__NEED_BLOB | CONCPROP__NO_STEP | CONCPROP__SET_PHANTOMS | CONCPROP__GET_MASK,  do_concept_triple_diag_tog},
   /* concept_triple_twin */              {CONCPROP__NEED_4X6 | CONCPROP__NO_STEP | Standard_matrix_phantom,                       triple_twin_move},
   /* concept_misc_distort */             {CONCPROP__NO_STEP,                                                                      do_concept_misc_distort},
   /* concept_old_stretch */              {0/*CONCPROP__NO_STEP*/,                                                                 do_concept_old_stretch},
   /* concept_new_stretch */              {0/*CONCPROP__NO_STEP*/,                                                                 do_concept_new_stretch},
   /* concept_assume_waves */             {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_assume_waves},
   /* concept_mirror */                   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_mirror},
   /* concept_central */                  {CONCPROP__SHOW_SPLIT,                                                                   do_concept_central},
   /* concept_crazy */                    {0,                                                                                      do_concept_crazy},
   /* concept_frac_crazy */               {CONCPROP__USE_NUMBER,                                                                   do_concept_crazy},
   /* concept_fan_or_yoyo */              {0,                                                                                      do_concept_fan_or_yoyo},
   /* concept_c1_phantom */               {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_c1_phantom_move},
   /* concept_grand_working */            {CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX,                                            do_concept_grand_working},
   /* concept_centers_or_ends */          {0,                                                                                      do_concept_centers_or_ends},
   /* concept_so_and_so_only */           {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP,                                             so_and_so_only_move},
   /* concept_some_vs_others */           {CONCPROP__USE_SELECTOR | CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                     so_and_so_only_move},
   /* concept_stable */                   {CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,                                      do_concept_stable},
   /* concept_so_and_so_stable */         {CONCPROP__USE_SELECTOR | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,             do_concept_stable},
   /* concept_frac_stable */              {CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,               do_concept_stable},
   /* concept_so_and_so_frac_stable */    {CONCPROP__USE_SELECTOR | CONCPROP__USE_NUMBER | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT,   do_concept_stable},
   /* concept_standard */                 {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP | CONCPROP__PERMIT_MATRIX,                   do_concept_standard},
   /* concept_matrix */                   {CONCPROP__MATRIX_OBLIVIOUS,                                                             do_concept_matrix},
   /* concept_double_offset */            {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        do_concept_double_offset},
   /* concept_checkpoint */               {CONCPROP__SECOND_CALL,                                                                  do_concept_checkpoint},
   /* concept_on_your_own */              {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              on_your_own_move},
   /* concept_trace */                    {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              do_concept_trace},
   /* concept_ferris */                   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_ferris},
   /* concept_all_8 */                    {0,                                                                                      do_concept_all_8},
   /* concept_centers_and_ends */         {CONCPROP__SECOND_CALL,                                                                  do_concept_centers_and_ends},
   /* concept_twice */                    {CONCPROP__SHOW_SPLIT,                                                                   do_concept_twice},
   /* concept_sequential */               {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                                           do_concept_sequential},
   /* concept_special_sequential */       {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                                           do_concept_special_sequential},
   /* concept_meta */                     {CONCPROP__SHOW_SPLIT,                                                                   do_concept_meta},
   /* concept_so_and_so_begin */          {CONCPROP__USE_SELECTOR | CONCPROP__SHOW_SPLIT,                                          do_concept_so_and_so_begin},
   /* concept_nth_part */                 {CONCPROP__USE_NUMBER | CONCPROP__SHOW_SPLIT,                                            do_concept_nth_part},
   /* concept_replace_nth_part */         {CONCPROP__USE_NUMBER | CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                    do_concept_replace_nth_part},
   /* concept_interlace */                {CONCPROP__SECOND_CALL | CONCPROP__SHOW_SPLIT,                                           do_concept_interlace},
   /* concept_fractional */               {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS | CONCPROP__MATRIX_OBLIVIOUS | CONCPROP__SHOW_SPLIT, do_concept_fractional},
   /* concept_rigger */                   {CONCPROP__NO_STEP,                                                                      do_concept_rigger},
   /* concept_diagnose */                 {0,                                                                                      do_concept_diagnose}};
