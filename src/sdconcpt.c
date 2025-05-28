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

/* This defines the following function:
   do_big_concept

and the following external variables:
   global_tbonetest
   global_livemask
   global_selectmask
   concept_table
*/

#include "sd.h"

int global_tbonetest;
int global_livemask;
int global_selectmask;



Private void do_concept_expand_2x6_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (ss->kind != s2x6) fail("Can't make a 2x6 matrix out of this.");
   ss->setupflags |= SETUPFLAG__EXPLICIT_MATRIX;
   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
}


Private void do_concept_expand_2x8_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (ss->kind != s2x8) fail("Can't make a 2x8 matrix out of this.");
   ss->setupflags |= SETUPFLAG__EXPLICIT_MATRIX;
   /* We used to turn on the "FINAL__16_MATRIX" call modifier,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the SETUPFLAG__EXPLICIT_MATRIX
      bit control the desired effects. */
   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
}


Private void do_concept_expand_4x4_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (ss->kind != s4x4) fail("Can't make a 4x4 matrix out of this.");
   ss->setupflags |= SETUPFLAG__EXPLICIT_MATRIX;
   /* We used to turn on the "FINAL__16_MATRIX" call modifier,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the SETUPFLAG__EXPLICIT_MATRIX
      bit control the desired effects. */
   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
}


Private void do_concept_expand_4dm_matrix(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (ss->kind != s_4dmd) fail("Can't make quadruple diamonds out of this.");
   ss->setupflags |= SETUPFLAG__EXPLICIT_MATRIX;
   /* We used to turn on the "FINAL__16_MATRIX" call modifier,
      but that makes tandem stuff not work (it doesn't like
      call modifiers preceding it) and 4x4 stuff not work
      (it wants the matrix expanded, but doesn't want you to say
      "16 matrix").  So we need to let the SETUPFLAG__EXPLICIT_MATRIX
      bit control the desired effects. */
   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
}


Private void do_c1_phantom_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   parse_block *next_parseptr;
   final_set junk_concepts;

   if (ss->setupflags & SETUPFLAG__DISTORTED)
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

      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX | SETUPFLAG__PHANTOMS;

      tandem_couples_move(ss, next_parseptr->next, NULLCALLSPEC, 0,
            next_parseptr->concept->value.arg1 ? next_parseptr->selector : selector_uninitialized,
            next_parseptr->concept->value.arg2,    /* normal=FALSE, twosome=TRUE */
            next_parseptr->number,
            1,                                     /* for phantom */
            next_parseptr->concept->value.arg4,    /* tandem=0 couples=1 siamese=2 */
            result);

      return;
   }

   /* We didn't do this before. */
   ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

   if (ss->kind == s_c1phan) {
      setup setup1, setup2, res1;

      setup1 = *ss;
      setup2 = *ss;
      
      setup1.kind = s2x4;
      setup2.kind = s2x4;
      setup1.rotation = ss->rotation;
      setup2.rotation = ss->rotation+1;
      (void) copy_person(&setup1, 0, ss, 0);
      (void) copy_person(&setup1, 1, ss, 2);
      (void) copy_person(&setup1, 2, ss, 7);
      (void) copy_person(&setup1, 3, ss, 5);
      (void) copy_person(&setup1, 4, ss, 8);
      (void) copy_person(&setup1, 5, ss, 10);
      (void) copy_person(&setup1, 6, ss, 15);
      (void) copy_person(&setup1, 7, ss, 13);
      (void) copy_rot(&setup2, 0, ss, 4, 033);
      (void) copy_rot(&setup2, 1, ss, 6, 033);
      (void) copy_rot(&setup2, 2, ss, 11, 033);
      (void) copy_rot(&setup2, 3, ss, 9, 033);
      (void) copy_rot(&setup2, 4, ss, 12, 033);
      (void) copy_rot(&setup2, 5, ss, 14, 033);
      (void) copy_rot(&setup2, 6, ss, 3, 033);
      (void) copy_rot(&setup2, 7, ss, 1, 033);
      
      normalize_setup(&setup1, simple_normalize);
      normalize_setup(&setup2, simple_normalize);
      setup1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      setup2.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      
      move(&setup1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);
      move(&setup2, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
      result->setupflags |= res1.setupflags;
      merge_setups(&res1, result);
   }
   else if (ss->kind == s4x4) {

      /* This must be a "phantom turn and deal" sort of thing from stairsteps.
         Do the call in each line, them remove resulting phantoms carefully. */

      if (global_livemask == 0x5C5C || global_livemask == 0xA3A3) {
         /* Split into 4 vertical strips. */
         divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
            &map_4x4v, phantest_ok, TRUE, result);
      }
      else if (global_livemask == 0xC5C5 || global_livemask == 0x3A3A) {
         /* Split into 4 horizontal strips. */
         divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
            (*map_lists[s1x4][3])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
      }
      else
         fail("Inappropriate setup for phantom concept.");

      if (result->kind == s2x8) {
         setup temp;
         
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
      }
      else
         fail("This call is not appropriate for use with phantom concept.");
   }
   else
      fail("Inappropriate setup for phantom concept.");
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

   tbonetest = global_tbonetest;

   if (!tbonetest) {
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

   for (i=0; i<4; i++) {
      (void) copy_person(&a1, i, ss, map_ptr->map[i]);
      clear_person(ss, map_ptr->map[i]);
   }

   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   a1.kind = s1x4;
   a1.rotation = 0;
   
   update_id_bits(&a1);
   move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);

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

   result->setupflags = res1.setupflags;
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      map_ptr, phantest_ok, TRUE, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      map_ptr, phantest_ok, TRUE, result);
}



Private void do_concept_quad_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   map_thing *maps;
   int rot = (global_tbonetest ^ parseptr->concept->value.arg1 ^ 1) & 1;

   /* If this was quadruple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was quadruple
      lines, we forbid it. */

   if (parseptr->concept->value.arg1 & 1)
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (ss->kind == s4x4) {
      maps = (*map_lists[s1x4][3])[MPKIND__SPLIT][1];
   }
   else if (ss->kind == s1x16) {
      if (rot) {
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
         else                      fail("There are no columns of 4 here.");
      }
      maps = (*map_lists[s1x4][3])[MPKIND__SPLIT][0];
   }
   else
      fail("Must have a 4x4 or 1x16 setup for this concept.");

   ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
   canonicalize_rotation(ss);
   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      maps, phantest_ok, TRUE, result);
   result->rotation -= rot;   /* Flip the setup back. */
}


Private void do_concept_quad_lines_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff, tbonetest;
   int i, m1, m2, m3, linesp;
   map_thing *map_ptr;

   cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8
      apart    : 9 */

   linesp = parseptr->concept->value.arg2;

   tbonetest = 0;
   for (i=0; i<16; i++) tbonetest |= ss->people[i].id1;

   /* If this was quadruple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was quadruple
      lines, we forbid it. */

   if (linesp)
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

   if ((tbonetest & 011) == 011) fail("Sorry, can't do this from T-bone setup.");

   if (cstuff < 4) {         /* Working forward/back/right/left. */
      if (ss->kind != s4x4) fail("Must have a 4x4 setup to do this concept.");

      map_ptr = ((linesp ^ tbonetest) & 1) ? (*map_lists[s2x4][2])[MPKIND__OVERLAP][1] : &map_ov_s2x4_k;

      m1 = 0xF0; m2 = 0xF0; m3 = 0xFF;
   
      /* Look at the center 8 people and put each one in the correct group. */
   
      if ((cstuff + 1 - ss->people[map_ptr->map2[0]].id1) & 2) { m2 |= 0x01; m3 &= ~0x80; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[1]].id1) & 2) { m2 |= 0x02; m3 &= ~0x40; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[2]].id1) & 2) { m2 |= 0x04; m3 &= ~0x20; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[3]].id1) & 2) { m2 |= 0x08; m3 &= ~0x10; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[4]].id1) & 2) { m1 |= 0x08; m2 &= ~0x10; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[5]].id1) & 2) { m1 |= 0x04; m2 &= ~0x20; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[6]].id1) & 2) { m1 |= 0x02; m2 &= ~0x40; };
      if ((cstuff + 1 - ss->people[map_ptr->map2[7]].id1) & 2) { m1 |= 0x01; m2 &= ~0x80; };
   }
   else {                    /* Working together/apart. */
      if (ss->kind != s1x16) fail("Must have a 1x16 setup for this concept.");

      map_ptr = (*map_lists[s1x8][2])[MPKIND__OVERLAP][0];
   
      if (linesp) {
         if (tbonetest & 1) fail("There are no lines of 4 here.");
      }
      else {
         if (!(tbonetest & 1)) fail("There are no columns of 4 here.");
      }

      if (cstuff == 8) {     /* Working together. */
         m1 = 0xCF; m2 = 0xCC; m3 = 0xFC;
      }
      else {                 /* Working apart. */
         m1 = 0x3F; m2 = 0x33; m3 = 0xF3;
      }
   }

   overlapped_setup_move(ss, map_ptr, m1, m2, m3, parseptr->next, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      map_ptr, phantest_ok, TRUE, result);
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
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8
      apart    : 9 */

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
   else if (cstuff == 8) {      /* Working together. */
      m1 = 0xE7 ; m2 = 0x66; m3 = 0x7E;
   }
   else {                       /* Working apart. */
      m1 = 0xDB ; m2 = 0x99; m3 = 0xBD;
   }

   overlapped_setup_move(ss, (*map_lists[s2x4][2])[MPKIND__OVERLAP][0], m1, m2, m3, parseptr->next, result);
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

   overlapped_setup_move(ss, (*map_lists[s_qtag][1])[MPKIND__OVERLAP][0], m1, m2, 0, parseptr->next, result);
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

   overlapped_setup_move(ss, (*map_lists[s_qtag][2])[MPKIND__OVERLAP][0], m1, m2, m3, parseptr->next, result);
}




Private void do_concept_triple_boxes_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   int cstuff;
   int m1, m2;

   cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8
      apart    : 9 */

   if (ss->kind != s2x6) fail("Must have a 2x6 setup to do this concept.");

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
   else if (cstuff == 8) {   /* Working together. */
      m1 = 0xE7; m2 = 0x7E;
   }
   else {                    /* Working apart. */
      m1 = 0xDB; m2 = 0xBD;
   }

   overlapped_setup_move(ss, (*map_lists[s2x4][1])[MPKIND__OVERLAP][0], m1, m2, 0, parseptr->next, result);
}


Private void do_concept_triple_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result)

/* This concept is "standard", which means that it can look at global_tbonetest
   and global_livemask, but may not look at anyone's facing direction other
   than through global_tbonetest. */

{
   map_thing *maps;

   /* If this was triple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was triple
      lines, we forbid it. */

   if (parseptr->concept->value.arg1 & 1)
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

   if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (ss->kind == s3x4) {
      maps = (*map_lists[s1x4][2])[MPKIND__SPLIT][1];
   }
   else if (ss->kind == s1x12) {
      maps = (*map_lists[s1x4][2])[MPKIND__SPLIT][0];
   }
   else
      fail("Must have a 3x4 or 1x12 setup for this concept.");

   if (!((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)) {
      if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      else                      fail("There are no columns of 4 here.");
   }

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      maps, phantest_ok, TRUE, result);
}


Private void do_concept_triple_lines_tog(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int cstuff, tbonetest;
   int i, m1, m2, linesp;
   map_thing *map_ptr;

   cstuff = parseptr->concept->value.arg1;
   /* cstuff =
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8
      apart    : 9 */

   linesp = parseptr->concept->value.arg2;

   tbonetest = 0;
   for (i=0; i<12; i++) tbonetest |= ss->people[i].id1;

   /* If this was triple columns, we allow stepping to a wave.  This makes it
      possible to do interesting cases of turn and weave, when one column
      is a single 8 chain and another is a single DPT.  But if it was triple
      lines, we forbid it. */

   if (linesp)
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

   if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   if (linesp) {
      if (tbonetest & 1) fail("There are no lines of 4 here.");
   }
   else {
      if (!(tbonetest & 1)) fail("There are no columns of 4 here.");
   }

   if (cstuff < 4) {         /* Working forward/back/right/left. */
      if (ss->kind != s3x4) fail("Must have a 3x4 setup for this concept.");
      map_ptr = (*map_lists[s2x4][1])[MPKIND__OVERLAP][1];

      if (linesp) {
         if (cstuff & 1) fail("Must indicate forward/back.");
      }
      else {
         if (!(cstuff & 1)) fail("Must indicate left/right.");
      }

      /* Initially assign the centers to the upper (m2) group. */
      m1 = 0xF0; m2 = 0xFF;

      /* Look at the center line people and put each one in the correct group. */

      if ((ss->people[10].id1 ^ cstuff) & 2) { m2 &= ~0x80 ; m1 |= 0x1; };
      if ((ss->people[11].id1 ^ cstuff) & 2) { m2 &= ~0x40 ; m1 |= 0x2; };
      if ((ss->people[5].id1  ^ cstuff) & 2) { m2 &= ~0x20 ; m1 |= 0x4; };
      if ((ss->people[4].id1  ^ cstuff) & 2) { m2 &= ~0x10 ; m1 |= 0x8; };
   }
   else {                    /* Working together/apart. */
      if (ss->kind != s1x12) fail("Must have a 1x12 setup for this concept.");
      map_ptr = (*map_lists[s1x8][1])[MPKIND__OVERLAP][0];

      if (cstuff == 8) {     /* Working together. */
         m1 = 0xCF; m2 = 0xFC;
      }
      else {                 /* Working apart. */
         m1 = 0x3F; m2 = 0xF3;
      }
   }

   overlapped_setup_move(ss, map_ptr, m1, m2, 0, parseptr->next, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      maps_3diag[q + ((parseptr->concept->value.arg1 ^ global_tbonetest) & 1)],
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

   overlapped_setup_move(ss, map_ptr, m1, m2, 0, parseptr->next, result);
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
      forward  : 0
      left     : 1
      back     : 2
      right    : 3
      together : 8
      apart    : 9 */

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
   else {      /* Working together or apart. */
      if (ss->kind != s1x8) fail("May not specify together/apart here.");

      /* Put each of the center 4 people in the correct group, no need to look. */

      if (cstuff & 1) {
         m1 = 0x7; m2 = 0x5; m3 = 0xD;
      }
      else {
         m1 = 0xB; m2 = 0xA; m3 = 0xE;
      }

      the_map = (*map_lists[s1x4][2])[MPKIND__OVERLAP][0];
   }

   overlapped_setup_move(ss, the_map, m1, m2, m3, parseptr->next, result);
}


Private void do_concept_do_phantom_2x2(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   /* Do "blocks" or "4 phantom interlocked blocks" or "triangular blocks, etc. */

   if (ss->kind != s4x4) fail("Must have a 4x4 setup for this concept.");

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}


Private void do_concept_do_phantom_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (ss->kind != s_4dmd) fail("Must have a quadruple diamond setup for this concept.");

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         (*map_lists[s_1x6][1])[MPKIND__SPLIT][1], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
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

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      (*map_lists[s1x8][1])[MPKIND__SPLIT][1], (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
}



Private void do_concept_once_removed(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   map_thing *the_map;
   if (parseptr->concept->value.arg1) {
      /* concept was "once removed diamonds" */
      if (ss->kind == s_rigger) {
         the_map = (*map_lists[sdmd][1])[MPKIND__REMOVED][0];
      }
      else {
         fail("There are no once removed diamonds here.");
      }
   }
   else {
      /* concept was just "once removed" */
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
            fail("You must select 'once removed diamonds' in this setup.");
         default:
            fail("Can't do 'once removed' from this setup.");
      }
   }

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0, the_map, phantest_ok, TRUE, result);
}


Private void do_concept_old_stretch(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);

   if (result->kind == s2x4) {
      swap_people(result, 1, 2);
      swap_people(result, 5, 6);
   }
   else if (result->kind == s1x8) {
      swap_people(result, 3, 6);
      swap_people(result, 2, 7);
   }
   else
      fail("Old stretch call didn't go to 2x4 or 1x8 setup.");
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

   tempsetup.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   move(&tempsetup, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
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
   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
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

   (void) copy_rot(result, mape[0+offset], ss, mape[1+offset], 022);
   (void) copy_rot(result, mape[1+offset], ss, mape[0+offset], 022);
   (void) copy_rot(result, mape[2+offset], ss, mape[3+offset], 022);
   (void) copy_rot(result, mape[3+offset], ss, mape[2+offset], 022);

   a1.kind = (setup_kind) parseptr->concept->value.arg1;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
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
   move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);

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
   result->setupflags = res1.setupflags;
   reinstate_rotation(ss, result);
}


Private void do_concept_checkpoint(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   if (parseptr->concept->value.arg1) {   /* 0 for normal, 1 for reverse checkpoint. */
      concentric_move(ss, parseptr->next, parseptr->subsidiary_root, NULLCALLSPEC, NULLCALLSPEC, 0, 0, schema_rev_checkpoint, 0, 0, result);
   }
   else {
      /* The "dfm_conc_force_otherway" flag forces Callerlab interpretation:
         If checkpointers go from 2x2 to 2x2, this is clear.
         If checkpointers go from 1x4 to 2x2, "dfm_conc_force_otherway" forces
            the Callerlab rule in preference to the "parallel_concentric_end" property
            on the call. */
      concentric_move(ss, parseptr->subsidiary_root, parseptr->next, NULLCALLSPEC, NULLCALLSPEC, 0, 0, schema_checkpoint, 0, DFM1_CONC_FORCE_OTHERWAY, result);
   }
}



Private void do_concept_sequential(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int finalsetupflags;
   int call_index;
   setup tempsetup;
   setup tttt;
   int current_elongation = ss->setupflags & SETUPFLAG__ELONGATE_MASK;

   finalsetupflags = 0;
   *result = *ss;

   for (call_index=0; call_index<2; call_index++) {
      tttt = *result;
      tttt.setupflags = (ss->setupflags & ~SETUPFLAG__ELONGATE_MASK) | current_elongation;

      if (call_index == 0)
         move(&tttt, parseptr->next, NULLCALLSPEC, 0, FALSE, &tempsetup);
      else
         move(&tttt, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, &tempsetup);

      finalsetupflags |= tempsetup.setupflags;

      if (tempsetup.kind == s2x2) {
         switch (result->kind) {
            case s1x4: case sdmd: case s2x2:
               current_elongation = (((tempsetup.setupflags & RESULTFLAG__ELONGATE_MASK) / RESULTFLAG__ELONGATE_BIT) * SETUPFLAG__ELONGATE_BIT);
               break;

            /* Otherwise (perhaps the setup was a star) we have no idea how to elongate the setup. */

            default:
               current_elongation = 0;
               break;
         }
      }
      else
         current_elongation = 0;

      *result = tempsetup;

      /* Remove outboard phantoms. 
         It used to be that normalize_setup was not called
         here.  It was found that we couldn't do things like, from a suitable offset wave,
         [triple line 1/2 flip] back to a wave, that is, start offset and finish normally.
         So this has been added.  However, there may have been a reason for not normalizing.
         If any problems are found, it may be that a flag needs to be added to seqdef calls
         saying whether to remove outboard phantoms after each part. */

      normalize_setup(result, simple_normalize);
   }

   result->setupflags = (finalsetupflags & ~RESULTFLAG__ELONGATE_MASK) | ((current_elongation / SETUPFLAG__ELONGATE_BIT) * RESULTFLAG__ELONGATE_BIT);
}



Private void do_concept_twice(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int finalsetupflags;
   int call_index;
   setup tempsetup;
   setup tttt;
   int current_elongation = ss->setupflags & SETUPFLAG__ELONGATE_MASK;

   finalsetupflags = 0;
   *result = *ss;

   for (call_index=0; call_index<2; call_index++) {
      tttt = *result;
      tttt.setupflags = (ss->setupflags & ~SETUPFLAG__ELONGATE_MASK) | current_elongation;

      move(&tttt, parseptr->next, NULLCALLSPEC, 0, FALSE, &tempsetup);

      finalsetupflags |= tempsetup.setupflags;

      if (tempsetup.kind == s2x2) {
         switch (result->kind) {
            case s1x4: case sdmd: case s2x2:
               current_elongation = (((tempsetup.setupflags & RESULTFLAG__ELONGATE_MASK) / RESULTFLAG__ELONGATE_BIT) * SETUPFLAG__ELONGATE_BIT);
               break;

            /* Otherwise (perhaps the setup was a star) we have no idea how to elongate the setup. */

            default:
               current_elongation = 0;
               break;
         }
      }
      else
         current_elongation = 0;

      *result = tempsetup;

      /* Remove outboard phantoms. 
         It used to be that normalize_setup was not called
         here.  It was found that we couldn't do things like, from a suitable offset wave,
         [triple line 1/2 flip] back to a wave, that is, start offset and finish normally.
         So this has been added.  However, there may have been a reason for not normalizing.
         If any problems are found, it may be that a flag needs to be added to seqdef calls
         saying whether to remove outboard phantoms after each part. */

      normalize_setup(result, simple_normalize);
   }

   result->setupflags = (finalsetupflags & ~RESULTFLAG__ELONGATE_MASK) | ((current_elongation / SETUPFLAG__ELONGATE_BIT) * RESULTFLAG__ELONGATE_BIT);
}
Private void do_concept_trace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int r1, r2, r3, r4, rot1, rot2, rot3, rot4;
   int finalsetupflags;
   setup a1, a2, a3, a4, res1, res2, res3, res4, inners, outers;

   if (ss->kind != s_qtag) fail("Must have a 1/4-tag-like setup for trace.");

   clear_people(&a1);
   clear_people(&a2);
   clear_people(&a3);
   clear_people(&a4);
   clear_people(&inners);
   clear_people(&outers);

   finalsetupflags = 0;

   a1.kind = s2x2;
   a1.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   a2.kind = s2x2;
   a2.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   a3.kind = s2x2;
   a3.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   a4.kind = s2x2;
   a4.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;

   if ((ss->people[6].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_south) {
      (void) copy_person(&a1, 2, ss, 7);
      (void) copy_person(&a1, 3, ss, 6);
      a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);
      finalsetupflags |= res1.setupflags;

      (void) copy_person(&a2, 2, ss, 4);
      (void) copy_person(&a2, 3, ss, 5);
      a2.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a2, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, &res2);
      finalsetupflags |= res2.setupflags;

      (void) copy_person(&a3, 0, ss, 3);
      (void) copy_person(&a3, 1, ss, 2);
      a3.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a3, parseptr->next, NULLCALLSPEC, 0, FALSE, &res3);
      finalsetupflags |= res3.setupflags;

      (void) copy_person(&a4, 0, ss, 0);
      (void) copy_person(&a4, 1, ss, 1);
      a4.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a4, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, &res4);
      finalsetupflags |= res4.setupflags;
   }
   else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_north) {
      (void) copy_person(&a1, 0, ss, 0);
      (void) copy_person(&a1, 1, ss, 1);
      a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a1, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, &res1);
      finalsetupflags |= res1.setupflags;

      (void) copy_person(&a2, 0, ss, 6);
      (void) copy_person(&a2, 1, ss, 7);
      a2.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a2, parseptr->next, NULLCALLSPEC, 0, FALSE, &res2);
      finalsetupflags |= res2.setupflags;

      (void) copy_person(&a3, 2, ss, 4);
      (void) copy_person(&a3, 3, ss, 5);
      a3.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a3, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, &res3);
      finalsetupflags |= res3.setupflags;

      (void) copy_person(&a4, 2, ss, 2);
      (void) copy_person(&a4, 3, ss, 3);
      a4.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a4, parseptr->next, NULLCALLSPEC, 0, FALSE, &res4);
      finalsetupflags |= res4.setupflags;
   }
   else
      fail("Can't determine which box people should work in.");

   /* Check that everyone is in a 2x2 or vertically oriented 1x4. */

   if      ((res1.kind != s2x2 && res1.kind != nothing && (res1.kind != s1x4 || (!(res1.rotation&1)))) ||
            (res2.kind != s2x2 && res2.kind != nothing && (res2.kind != s1x4 || (!(res2.rotation&1)))) ||
            (res3.kind != s2x2 && res3.kind != nothing && (res3.kind != s1x4 || (!(res3.rotation&1)))) ||
            (res4.kind != s2x2 && res4.kind != nothing && (res4.kind != s1x4 || (!(res4.rotation&1)))))
      fail("You can't do this.");

   /* Process people going into the center. */

   inners.rotation = 0;
   inners.setupflags = 0;

   if   ((res1.kind == s2x2 && (res1.people[2].id1 | res1.people[3].id1)) ||
         (res2.kind == s2x2 && (res2.people[0].id1 | res2.people[1].id1)) ||
         (res3.kind == s2x2 && (res3.people[0].id1 | res3.people[1].id1)) ||
         (res4.kind == s2x2 && (res4.people[2].id1 | res4.people[3].id1)))
      inners.kind = s1x4;
   else
      inners.kind = nothing;

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
      if (inners.kind != nothing) fail("You can't do this.");
      inners.kind = s2x2;
   }

   if (res1.kind == s2x2) {
      install_person(&inners, 1, &res1, 2);
      install_person(&inners, 0, &res1, 3);
   }
   else {
      install_rot(&inners, 3, &res1, 2^r1, rot1);
      install_rot(&inners, 0, &res1, 3^r1, rot1);
   }

   if (res2.kind == s2x2) {
      install_person(&inners, 0, &res2, 0);
      install_person(&inners, 1, &res2, 1);
   }
   else {
      install_rot(&inners, 0, &res2, 0^r2, rot2);
      install_rot(&inners, 3, &res2, 1^r2, rot2);
   }

   if (res3.kind == s2x2) {
      install_person(&inners, 3, &res3, 0);
      install_person(&inners, 2, &res3, 1);
   }
   else {
      install_rot(&inners, 1, &res3, 0^r3, rot3);
      install_rot(&inners, 2, &res3, 1^r3, rot3);
   }

   if (res4.kind == s2x2) {
      install_person(&inners, 2, &res4, 2);
      install_person(&inners, 3, &res4, 3);
   }
   else {
      install_rot(&inners, 2, &res4, 2^r4, rot4);
      install_rot(&inners, 1, &res4, 3^r4, rot4);
   }

   /* Process people going to the outside. */

   outers.rotation = 0;
   outers.setupflags = 0;

   if   ((res1.kind == s2x2 && (res1.people[0].id1 | res1.people[1].id1)) ||
         (res2.kind == s2x2 && (res2.people[2].id1 | res2.people[3].id1)) ||
         (res3.kind == s2x2 && (res3.people[2].id1 | res3.people[3].id1)) ||
         (res4.kind == s2x2 && (res4.people[0].id1 | res4.people[1].id1)))
      outers.kind = s2x2;
   else
      outers.kind = nothing;

   r1 = res1.rotation & 2;
   r2 = res2.rotation & 2;
   r3 = res3.rotation & 2;
   r4 = res4.rotation & 2;

   if   ((res1.kind == s1x4 && (res1.people[0 ^ r1].id1 | res1.people[1 ^ r1].id1)) ||
         (res2.kind == s1x4 && (res2.people[2 ^ r2].id1 | res2.people[3 ^ r2].id1)) ||
         (res3.kind == s1x4 && (res3.people[2 ^ r3].id1 | res3.people[3 ^ r3].id1)) ||
         (res4.kind == s1x4 && (res4.people[0 ^ r4].id1 | res4.people[1 ^ r4].id1))) {
      if (outers.kind != nothing) fail("You can't do this.");
      outers.kind = s1x4;
      outers.rotation = 1;
   }

   if (res1.kind == s2x2) {
      install_person(&outers, 0, &res1, 0);
      install_person(&outers, 1, &res1, 1);
   }
   else {
      install_rot(&outers, 0, &res1, 0 ^ (res1.rotation&2), ((res1.rotation-1)&3)*011);
      install_rot(&outers, 1, &res1, 1 ^ (res1.rotation&2), ((res1.rotation-1)&3)*011);
   }

   if (res2.kind == s2x2) {
      install_person(&outers, 2, &res2, 2);
      install_person(&outers, 3, &res2, 3);
   }
   else {
      install_rot(&outers, 2, &res2, 2 ^ (res2.rotation&2), ((res2.rotation-1)&3)*011);
      install_rot(&outers, 3, &res2, 3 ^ (res2.rotation&2), ((res2.rotation-1)&3)*011);
   }

   if (res3.kind == s2x2) {
      install_person(&outers, 2, &res3, 2);
      install_person(&outers, 3, &res3, 3);
   }
   else {
      install_rot(&outers, 2, &res3, 2 ^ (res3.rotation&2), ((res3.rotation-1)&3)*011);
      install_rot(&outers, 3, &res3, 3 ^ (res3.rotation&2), ((res3.rotation-1)&3)*011);
   }

   if (res4.kind == s2x2) {
      install_person(&outers, 0, &res4, 0);
      install_person(&outers, 1, &res4, 1);
   }
   else {
      install_rot(&outers, 0, &res4, 0 ^ (res4.rotation&2), ((res4.rotation-1)&3)*011);
      install_rot(&outers, 1, &res4, 1 ^ (res4.rotation&2), ((res4.rotation-1)&3)*011);
   }

   normalize_concentric(schema_concentric, 1, &inners, &outers, outers.rotation ^ 1, result);
   result->setupflags = finalsetupflags;
   reinstate_rotation(ss, result);
}



Private void do_concept_quad_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s2x8) fail("Must have a 2x8 setup for this concept.");

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      (*map_lists[s2x2][3])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
}



Private void do_concept_do_both_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind == s2x4) {
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         parseptr->concept->value.maps, phantest_ok, TRUE, result);
   }
   else if (ss->kind == s3x4 && parseptr->concept->value.arg2) {
      /* distorted_2x2s_move will notice that concept is funny and will do the right thing. */
      distorted_2x2s_move(ss, parseptr, result);
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
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         (*map_lists[s1x4][1])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
   }
   else if (ss->kind == s1x8) {
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         (*map_lists[s1x4][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
   }
   else
      fail("Need a 2x4 or 1x8 setup for this concept.");
}


Private void do_concept_triple_boxes(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind == s2x6) {
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
            (*map_lists[s2x2][2])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
   }
   else
      fail("Need a 2x6 setup to do this concept.");
}


Private void do_concept_centers_or_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   calldef_schema schema;
   int k = parseptr->concept->value.arg1;

   switch (k&6) {
      case 0:
         schema = schema_concentric;
         break;
      case 2:
         schema = schema_concentric_6_2;
         break;
      default:
         schema = schema_concentric_2_6;
         break;
   }

   if (k&1)
      concentric_move(ss, (parse_block *) 0, parseptr->next, NULLCALLSPEC, NULLCALLSPEC, 0, 0,
               schema, 0, 0, result);
   else
      concentric_move(ss, parseptr->next, (parse_block *) 0, NULLCALLSPEC, NULLCALLSPEC, 0, 0,
               schema, 0, 0, result);
}


Private void do_concept_centers_and_ends(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int thing;
   calldef_schema schema;

   thing = parseptr->concept->value.arg1;
   if (thing == 2)
      schema = schema_concentric_6_2;
   else if (thing == 3)
      schema = schema_concentric_2_6;
   else
      schema = schema_concentric;

   concentric_move(ss, parseptr->next, parseptr->subsidiary_root, NULLCALLSPEC, NULLCALLSPEC, 0, 0, schema, 0, 0, result);
}


Private void do_concept_triple_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind != s_3dmd) fail("Must have a triple diamond setup to do this concept.");
   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         (*map_lists[sdmd][2])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
}


Private void do_concept_quad_diamonds(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   if (ss->kind == s_4dmd)
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
            (*map_lists[sdmd][3])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
   else
      fail("Must have a quadruple diamond setup to do this concept.");
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

   divided_setup_move(&temp, parseptr->next, NULLCALLSPEC, 0,
      (*map_lists[s1x4][2])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);

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


Private void do_concept_meta(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   final_set new_final_concepts;
   parse_block *parseptrcopy;
   parse_block *parseptrcopycopy;
   int finalsetupflags;
   unsigned int index;

   if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK)
      fail("Can't stack meta or fractional concepts.");

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

   *result = *ss;
   finalsetupflags = 0;

   if (parseptr->concept->value.arg1 == 3) {
      /* Arg1 = 3 is special: we select the first part with the concept, using a
         fractionalize field of [0 0 1], and then the rest of the call without the
         concept, using a fractionalize field of [1 0 1]. */

      setup tttt;

      tttt = *result;
      /* Set the fractionalize field to [0 0 1].  This will execute the first part of the call. */
      tttt.setupflags = ss->setupflags | SETUPFLAG__FRACTIONALIZE_BIT;

      /* Do the call with the concept. */
      move(&tttt, parseptrcopy, NULLCALLSPEC, 0, FALSE, result);
      finalsetupflags |= result->setupflags;
      normalize_setup(result, simple_normalize);

      tttt = *result;
      /* Set the fractionalize field to [1 0 1].  This will execute the rest of the call. */
      tttt.setupflags = ss->setupflags | (65*SETUPFLAG__FRACTIONALIZE_BIT);

      /* Do the call without the concept. */
      move(&tttt, parseptrcopy->next, NULLCALLSPEC, 0, FALSE, result);
      finalsetupflags |= result->setupflags;
      normalize_setup(result, simple_normalize);
   }
   else {
      /* Otherwise, this is the "random", "reverse random", or "piecewise" concept.
         Repeatedly execute parts of the call, using a fractionalize field of [2 0 index],
         and skipping the concept where required. */
   
      index = 0;

      do {
         setup tttt;
     
         tttt = *result;
         index++;
         /* Set the fractionalize field to [2 0 index]. */
         tttt.setupflags = ss->setupflags | ((128|index)*SETUPFLAG__FRACTIONALIZE_BIT);
         parseptrcopycopy = parseptrcopy;
         /* If concept is "[reverse] random" and this is an even/odd-numbered part,
            as the case may be, skip over the concept. */
         if (((parseptr->concept->value.arg1 & ~1) == 0) &&
               ((index & 1) == parseptr->concept->value.arg1)) {
            /* Need to skip the concept.  Note that we have already forbidden
               concepts that take another call. */
            parseptrcopycopy = parseptrcopycopy->next;
         }
     
         move(&tttt, parseptrcopycopy, NULLCALLSPEC, 0, FALSE, result);
         finalsetupflags |= result->setupflags;
         normalize_setup(result, simple_normalize);
      }
      while (!(result->setupflags & RESULTFLAG__DID_LAST_PART));
   }
  
   result->setupflags = finalsetupflags & ~RESULTFLAG__ELONGATE_MASK;
}


Private void do_concept_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   final_set new_final_concepts;
   parse_block *parseptrcopy;
   setup tttt;
   int finalsetupflags;

   if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK)
      fail("Can't stack meta or fractional concepts.");

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

   *result = *ss;
   finalsetupflags = 0;

   /* Do the initial part, if any. */

   if (parseptr->number > 1) {
      tttt = *result;
      /* Set the fractionalize field to [0 0 parts-to-do-normally]. */
      tttt.setupflags = ss->setupflags | ((parseptr->number-1)*SETUPFLAG__FRACTIONALIZE_BIT);
      /* Skip over the concept. */
      move(&tttt, parseptrcopy->next, NULLCALLSPEC, 0, FALSE, result);
      finalsetupflags |= result->setupflags;
      normalize_setup(result, simple_normalize);
   }

   /* Do the part of the call that needs the concept. */

   tttt = *result;
   /* Set the fractionalize field to [2 0 part-to-do-with-concept]. */
   tttt.setupflags = ss->setupflags | ((128|parseptr->number)*SETUPFLAG__FRACTIONALIZE_BIT);
   /* Do the concept. */
   move(&tttt, parseptrcopy, NULLCALLSPEC, 0, FALSE, result);
   finalsetupflags |= result->setupflags;
   normalize_setup(result, simple_normalize);

   /* Do the final part, if there is more. */

   if (!(result->setupflags & RESULTFLAG__DID_LAST_PART)) {
      tttt = *result;
      /* Set the fractionalize field to [1 0 parts-to-do-normally+1]. */
      tttt.setupflags = ss->setupflags | ((64|parseptr->number)*SETUPFLAG__FRACTIONALIZE_BIT);
      /* Skip over the concept. */
      move(&tttt, parseptrcopy->next, NULLCALLSPEC, 0, FALSE, result);
      finalsetupflags |= result->setupflags;
      normalize_setup(result, simple_normalize);
   }

   result->setupflags = finalsetupflags & ~RESULTFLAG__ELONGATE_MASK;
}


Private void do_concept_replace_nth_part(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup tttt;
   int finalsetupflags;
   int stopindex;

   *result = *ss;
   finalsetupflags = 0;

   /* Do the initial part, if any. */

   if (parseptr->concept->value.arg1)
      stopindex = parseptr->number;      /* Interrupt after Nth part. */
   else
      stopindex = parseptr->number-1;    /* Replace Nth part. */

   if (stopindex > 0) {
      tttt = *result;
      /* Set the fractionalize field to [0 0 parts-to-do-normally]. */
      tttt.setupflags = ss->setupflags | ((stopindex)*SETUPFLAG__FRACTIONALIZE_BIT);
      /* Skip over the concept. */
      move(&tttt, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
      finalsetupflags |= result->setupflags;
      normalize_setup(result, simple_normalize);
   }

   /* Do the interruption/replacement call. */

   tttt = *result;
   tttt.setupflags = ss->setupflags;
   update_id_bits(&tttt);           /* So you can interrupt with "leads run", etc. */
   move(&tttt, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, result);
   finalsetupflags |= result->setupflags;
   normalize_setup(result, simple_normalize);

   /* Do the final part, if there is more. */

   tttt = *result;
   /* Set the fractionalize field to [1 0 parts-to-do-normally+1]. */
   tttt.setupflags = ss->setupflags | ((64|parseptr->number)*SETUPFLAG__FRACTIONALIZE_BIT);
   /* Skip over the concept. */
   move(&tttt, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
   finalsetupflags |= result->setupflags;
   normalize_setup(result, simple_normalize);

   result->setupflags = finalsetupflags & ~RESULTFLAG__ELONGATE_MASK;
}


Private void do_concept_interlace(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int finalsetupflags;
   unsigned int index;
   long_boolean first_active, second_active;

   finalsetupflags = 0;
   first_active = TRUE;
   second_active = TRUE;
   index = 0;

   if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK)
      fail("Can't stack meta or fractional concepts.");

   *result = *ss;

   do {
      setup tttt;

      index++;

      if (first_active) {
         tttt = *result;
         /* Set the fractionalize field to [2 0 index]. */
         tttt.setupflags = ss->setupflags | ((128|index)*SETUPFLAG__FRACTIONALIZE_BIT);
     
         /* Do the indicated part of the first call. */
         move(&tttt, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
         finalsetupflags |= result->setupflags;
         normalize_setup(result, simple_normalize);
         first_active = !(result->setupflags & RESULTFLAG__DID_LAST_PART);
      }

      if (second_active) {
         tttt = *result;
         /* Set the fractionalize field to [2 0 index]. */
         tttt.setupflags = ss->setupflags | ((128|index)*SETUPFLAG__FRACTIONALIZE_BIT);
     
         /* Do the indicated part of the second call. */
         move(&tttt, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, result);
         finalsetupflags |= result->setupflags;
         normalize_setup(result, simple_normalize);
         second_active = !(result->setupflags & RESULTFLAG__DID_LAST_PART);
      }
   }
   while (first_active || second_active);
  
   result->setupflags = finalsetupflags & ~RESULTFLAG__ELONGATE_MASK;
}


Private void do_concept_fractional(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* Note: if we ever implement something that omits the first fraction, that
      concept would have to have "CONCPROP__NO_STEP" set in concept_table, and
      things might get ugly. */
   int numer, denom;

   if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK)
      fail("Can't stack meta or fractional concepts.");

   numer = parseptr->number;
   denom = numer >> 16;
   numer &= 0xFFFF;
   /* Set the fractionalize field to [0 denominator numerator]. */
   ss->setupflags |= ((denom*8) | numer)*SETUPFLAG__FRACTIONALIZE_BIT;

   move(ss, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
}


Private void do_concept_so_and_so_begin(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int finalsetupflags;
   selector_kind saved_selector;
   int i;
   setup setup1, setup2, res1;

   if (ss->setupflags & SETUPFLAG__FRACTIONALIZE_MASK)
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
   setup1.setupflags = ss->setupflags | SETUPFLAG__FRACTIONALIZE_BIT;

   /* The selected people execute the first part of the call. */

   move(&setup1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);
   setup2.setupflags = res1.setupflags;
   merge_setups(&res1, &setup2);
   finalsetupflags = setup2.setupflags;
   normalize_setup(&setup2, simple_normalize);

   /* Set the fractionalize field to [1 0 1].  This will execute the rest of the call. */
   setup2.setupflags = ss->setupflags | (65*SETUPFLAG__FRACTIONALIZE_BIT);

   /* Everyone executes the second part of the call. */
   move(&setup2, parseptr->next, NULLCALLSPEC, 0, FALSE, result);
   finalsetupflags |= result->setupflags;
   normalize_setup(result, simple_normalize);
  
   result->setupflags = finalsetupflags & ~RESULTFLAG__ELONGATE_MASK;
}


Private void do_concept_misc_distort(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   distorted_2x2s_move(ss, parseptr, result);
   reinstate_rotation(ss, result);
}


Private void do_concept_do_phantom_2x4(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{

   /* This concept is "standard", which means that it can look at global_tbonetest
      and global_livemask, but may not look at anyone's facing direction other
      than through global_tbonetest. */

   if (ss->kind != s4x4) fail("Need a 4x4 setup to do this concept.");

   /* If not T-boned, we can do something glorious. */

   if ((global_tbonetest & 011) != 011) {
      int rot = (global_tbonetest ^ parseptr->concept->value.arg2 ^ 1) & 1;
      ss->rotation += rot;   /* Just flip the setup around and recanonicalize. */
      canonicalize_rotation(ss);
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0, parseptr->concept->value.maps,
            (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
      result->rotation -= rot;   /* Flip the setup back. */
      return;
   }

   /* People are T-boned!  This is messy. */

   phantom_2x4_move(
      ss,
      parseptr->concept->value.arg2,
      (phantest_kind) parseptr->concept->value.arg1,
      parseptr->concept->value.maps,
      parseptr->next,
      result);
}


Private void do_concept_do_phantom_lines(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   /* This concept is "standard", which means that it can look at global_tbonetest
      and global_livemask, but may not look at anyone's facing direction other
      than through global_tbonetest. */

   if (ss->kind == s4x4) {
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;
      do_concept_do_phantom_2x4(ss, parseptr, result);
   }
   else if (ss->kind == s1x16) {
      int linesp = parseptr->concept->value.arg2;

      if ((global_tbonetest & 011) == 011) fail("Can't do this from T-bone setup, try using \"standard\".");

      /* If this was phantom columns, we allow stepping to a wave.  This makes it
         possible to do interesting cases of turn and weave, when one column
         is a single 8 chain and another is a single DPT.  But if it was phantom
         lines, we forbid it. */
   
      if (linesp) {
         ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;
         if (global_tbonetest & 1) fail("There are no lines of 4 here.");
      }
      else {
         if (global_tbonetest & 010) fail("There are no columns of 4 here.");
      }
      
      divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
            (*map_lists[s1x8][1])[parseptr->concept->value.arg3][0],
            (phantest_kind) parseptr->concept->value.arg1, TRUE, result);
   }
   else
      fail("Need a 4x4 or 1x16 setup to do this concept.");
}


Private void do_concept_concentric(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   concentric_move(ss, parseptr->next, parseptr->next, NULLCALLSPEC, NULLCALLSPEC, 0, 0,
         (calldef_schema) parseptr->concept->value.arg1, 0, DFM1_CONC_CONCENTRIC_RULES, result);
}


Private void do_concept_single_concentric(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   switch (ss->kind) {
      case s2x4:
         divided_setup_move(ss, parseptr, NULLCALLSPEC, 0, (*map_lists[s1x4][1])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
         break;
      case s1x8:
         divided_setup_move(ss, parseptr, NULLCALLSPEC, 0, (*map_lists[s1x4][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
         break;
      case s_qtag:
         divided_setup_move(ss, parseptr, NULLCALLSPEC, 0, (*map_lists[sdmd][1])[MPKIND__SPLIT][1], phantest_ok, TRUE, result);
         break;
      case s_ptpd:
         divided_setup_move(ss, parseptr, NULLCALLSPEC, 0, (*map_lists[sdmd][1])[MPKIND__SPLIT][0], phantest_ok, TRUE, result);
         break;
      case s1x4: case sdmd:
         concentric_move(ss, parseptr->next, parseptr->next, NULLCALLSPEC, NULLCALLSPEC, 0, 0,
               (calldef_schema) parseptr->concept->value.arg1, 0, DFM1_CONC_CONCENTRIC_RULES, result);
         break;
      default:
         fail("Can't figure out how to do single concentric here.");
   }
}


Private void do_concept_tandem(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   tandem_couples_move(ss, parseptr->next, NULLCALLSPEC, 0,
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
   fail("Huh????.");
}


extern long_boolean do_big_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   void (*concept_func)(setup *, parse_block *, setup *);

   concept_func = concept_table[parseptr->concept->kind].concept_action;

   if (concept_func == 0) return(FALSE);

   if (parseptr->concept->level > calling_level) warn(warn__bad_concept_level);

   if (concept_table[parseptr->concept->kind].concept_prop & CONCPROP__SET_PHANTOMS)
      ss->setupflags |= SETUPFLAG__PHANTOMS;

   if (concept_table[parseptr->concept->kind].concept_prop & CONCPROP__NO_STEP)
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

   /* "Standard" is special -- process it now. */

   if (parseptr->concept->kind == concept_standard) {
      parse_block *parseptr_realconcept;
      final_set junk_concepts;
      int tbonetest = 0;
      int stdtest = 0;
      int livemask = 0;

      /* Skip to the phantom-line (or whatever) concept by going over the "standard" and skipping comments. */
      parseptr_realconcept = process_final_concepts(parseptr->next, TRUE, &junk_concepts);
   
      if ((junk_concepts != 0) || (!(concept_table[parseptr_realconcept->concept->kind].concept_prop & CONCPROP__STANDARD)))
         fail("This concept must be used with some offset/distorted/phantom concept.");

      /* We don't think stepping to a wave is ever a good idea if standard is used.  Most calls that
         permit standard (CONCPROP__STANDARD is on) forbid it anyway (CONCPROP__NO_STEP is on also),
         but a few (e.g. concept_triple_lines) permit standard but don't necessarily forbid stepping
         to a wave.  This is so that interesting cases of triple columns turn and weave will work.
         However, we think that this should be disallowed if "so-and-so are standard in triple lines"
         is given.  At least, that is the theory behind this next line of code. */
      ss->setupflags |= SETUPFLAG__NO_STEP_TO_WAVE;

      if (concept_table[parseptr_realconcept->concept->kind].concept_prop & CONCPROP__SET_PHANTOMS)
         ss->setupflags |= SETUPFLAG__PHANTOMS;

      if (!(ss->setupflags & SETUPFLAG__NO_EXPAND_MATRIX))
         do_matrix_expansion(ss, concept_table[parseptr_realconcept->concept->kind].concept_prop, FALSE);

      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

      if (setup_limits[ss->kind] < 0) fail("Can't do this concept in this setup.");
   
      {
         int i, j;
         selector_kind saved_selector = current_selector;
   
         current_selector = parseptr->selector;
   
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
         result->kind = nothing;
         return(TRUE);
      }
   
      if ((tbonetest & 011) != 011) fail("People are not T-boned -- 'standard' is meaningless.");
   
      if (!stdtest) fail("No one is standard.");
      if ((stdtest & 011) == 011) fail("The standard people are not facing consistently.");
   
      global_tbonetest = stdtest;
      global_livemask = livemask;

      (concept_table[parseptr_realconcept->concept->kind].concept_action)(ss, parseptr_realconcept, result);
      canonicalize_rotation(result);
      return(TRUE);
   }

   if (!(ss->setupflags & SETUPFLAG__NO_EXPAND_MATRIX))
      do_matrix_expansion(ss, concept_table[parseptr->concept->kind].concept_prop, FALSE);

   /* We can no longer do any matrix expansion, unless this is "phantom" and "tandem", in which case we continue to allow it. */
   if (parseptr->concept->kind != concept_c1_phantom)
      ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

   /* See if this concept can be invoked with "standard".  If so, it wants
      tbonetest and livemask computed, and expects the former to indicate
      only the standard people. */

   if (concept_table[parseptr->concept->kind].concept_prop & (CONCPROP__STANDARD | CONCPROP__GET_MASK)) {
      int i, j;
      long_boolean doing_select;
      selector_kind saved_selector = current_selector;

      if (setup_limits[ss->kind] < 0) fail("Can't do this concept in this setup.");

      global_tbonetest = 0;
      global_livemask = 0;
      global_selectmask = 0;
      doing_select = concept_table[parseptr->concept->kind].concept_prop & CONCPROP__USE_SELECTOR;

      if (doing_select) {
         current_selector = parseptr->selector;
      }

      for (i=0, j=1; i<=setup_limits[ss->kind]; i++, j<<=1) {
         int p = ss->people[i].id1;
         global_tbonetest |= p;
         if (p) {
            global_livemask |= j;
            if (doing_select && selectp(ss, i)) global_selectmask |= j;
         }
      }

      current_selector = saved_selector;

      if (!global_tbonetest) {
         result->kind = nothing;
         return(TRUE);
      }
   }

   (*concept_func)(ss, parseptr, result);
   canonicalize_rotation(result);
   return(TRUE);
}


/* Beware!!  This table must be keyed to definition of "concept_kind" in sd.h . */

concept_table_item concept_table[] = {
   {0,                                                                                      0},                               /* concept_another_call_next_mod */
   {0,                                                                                      0},                               /* concept_another_call_next_modreact */
   {0,                                                                                      0},                               /* concept_another_call_next_modtag */
   {0,                                                                                      0},                               /* concept_another_call_next_force */
   {0,                                                                                      0},                               /* concept_mod_declined */
   {0,                                                                                      0},                               /* marker_end_of_list */
   {0,                                                                                      0},                               /* concept_comment */
   {0,                                                                                      do_concept_concentric},           /* concept_concentric */
   {0,                                                                                      do_concept_single_concentric},    /* concept_single_concentric */
   {0,                                                                                      do_concept_tandem},               /* concept_tandem */
   {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS,                                            do_concept_tandem},               /* concept_gruesome_tandem */
   {CONCPROP__USE_SELECTOR,                                                                 do_concept_tandem},               /* concept_some_are_tandem */
   {CONCPROP__USE_NUMBER,                                                                   do_concept_tandem},               /* concept_frac_tandem */
   {CONCPROP__USE_NUMBER | CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS,                     do_concept_tandem},               /* concept_gruesome_frac_tandem */
   {CONCPROP__USE_NUMBER | CONCPROP__USE_SELECTOR,                                          do_concept_tandem},               /* concept_some_are_frac_tandem */
   {0,                                                                                      do_concept_checkerboard},         /* concept_checkerboard */
   {0,                                                                                      0},                               /* concept_reverse */
   {0,                                                                                      0},                               /* concept_left */
   {0,                                                                                      0},                               /* concept_grand */
   {0,                                                                                      0},                               /* concept_magic */
   {0,                                                                                      0},                               /* concept_cross */
   {0,                                                                                      0},                               /* concept_single */
   {0,                                                                                      0},                               /* concept_interlocked */
   {0,                                                                                      0},                               /* concept_12_matrix */
   {0,                                                                                      0},                               /* concept_16_matrix */
   {0,                                                                                      0},                               /* concept_1x2 */
   {0,                                                                                      0},                               /* concept_2x1 */
   {0,                                                                                      0},                               /* concept_2x2 */
   {0,                                                                                      0},                               /* concept_1x3 */
   {0,                                                                                      0},                               /* concept_3x1 */
   {0,                                                                                      0},                               /* concept_3x3 */
   {0,                                                                                      0},                               /* concept_4x4 */
   {CONCPROP__NEED_2X6 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_expand_2x6_matrix},    /* concept_2x6_matrix */
   {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_expand_2x8_matrix},    /* concept_2x8_matrix */
   {CONCPROP__NEED_4X4 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_expand_4x4_matrix},    /* concept_4x4_matrix */
   {CONCPROP__NEED_4DMD | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                       do_concept_expand_4dm_matrix},    /* concept_4dmd_matrix */
   {0,                                                                                      0},                               /* concept_funny */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 triangle_move},                   /* concept_randomtrngl */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        triangle_move},                   /* concept_selbasedtrngl */
   {0,                                                                                      0},                               /* concept_split */
   {CONCPROP__NO_STEP,                                                                      do_concept_do_each_1x4},          /* concept_each_1x4 */
   {0,                                                                                      0},                               /* concept_diamond */
   {0,                                                                                      0},                               /* concept_triangle */
   {CONCPROP__NO_STEP,                                                                      do_concept_do_both_boxes},        /* concept_do_both_boxes */
   {0,                                                                                      do_concept_once_removed},         /* concept_once_removed */
   {CONCPROP__NEED_4X4 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_do_phantom_2x2},       /* concept_do_phantom_2x2 */
   {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_do_phantom_boxes},     /* concept_do_phantom_boxes */
   {CONCPROP__NEED_4DMD | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                       do_concept_do_phantom_diamonds},  /* concept_do_phantom_diamonds */
   {CONCPROP__NEED_2X6 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   do_concept_do_phantom_1x6},       /* concept_do_phantom_1x6 */
   {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   do_concept_do_phantom_1x8},       /* concept_do_phantom_1x8 */
   {CONCPROP__NEED_4X4 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   do_concept_do_phantom_2x4},       /* concept_do_phantom_2x4 */
   {CONCPROP__NEED_4X4_1X16 | CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,                  do_concept_do_phantom_lines},     /* concept_do_phantom_lines */
   {CONCPROP__NEED_3X4 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   do_concept_do_phantom_2x3},       /* concept_do_phantom_2x3 */
   {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   do_concept_divided_2x4},          /* concept_divided_2x4 */
   {CONCPROP__NEED_2X6 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   do_concept_divided_2x3},          /* concept_divided_2x3 */
   {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 distorted_move},                  /* concept_distorted */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        do_concept_single_diagonal},      /* concept_single_diagonal */
   {CONCPROP__NO_STEP | CONCPROP__STANDARD,                                                 do_concept_double_diagonal},      /* concept_double_diagonal */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_parallelogram},        /* concept_parallelogram */
   {CONCPROP__NEED_3X4_1X12 | CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,                  do_concept_triple_lines},         /* concept_triple_lines */
   {CONCPROP__NEED_3X4_1X12 | CONCPROP__SET_PHANTOMS,                                       do_concept_triple_lines_tog},     /* concept_triple_lines_together */
   {CONCPROP__NEED_4X4_1X16 | CONCPROP__SET_PHANTOMS | CONCPROP__STANDARD,                  do_concept_quad_lines},           /* concept_quad_lines */
   {CONCPROP__NEED_4X4_1X16 | CONCPROP__SET_PHANTOMS,                                       do_concept_quad_lines_tog},       /* concept_quad_lines_together */
   {CONCPROP__NEED_2X8 | CONCPROP__NO_STEP,                                                 do_concept_quad_boxes},           /* concept_quad_boxes */
   {CONCPROP__NEED_2X8 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_quad_boxes_tog},       /* concept_quad_boxes_together */
   {CONCPROP__NEED_2X6 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_triple_boxes},         /* concept_triple_boxes */
   {CONCPROP__NEED_2X6 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                        do_concept_triple_boxes_tog},     /* concept_triple_boxes_together */
   {CONCPROP__NEED_3DMD | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                       do_concept_triple_diamonds},      /* concept_triple_diamonds */
   {CONCPROP__NEED_3DMD | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                       do_concept_triple_diamonds_tog},  /* concept_triple_diamonds_together */
   {CONCPROP__NEED_4DMD | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                       do_concept_quad_diamonds},        /* concept_quad_diamonds */
   {CONCPROP__NEED_4DMD | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP,                       do_concept_quad_diamonds_tog},    /* concept_quad_diamonds_together */
   {CONCPROP__NEED_BLOB | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,  do_concept_triple_diag},          /* concept_triple_diag */
   {CONCPROP__NEED_BLOB | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__GET_MASK,  do_concept_triple_diag_tog},      /* concept_triple_diag_together */
   {CONCPROP__NEED_4X6 | CONCPROP__SET_PHANTOMS | CONCPROP__NO_STEP | CONCPROP__STANDARD,   triple_twin_move},                /* concept_triple_twin */
   {CONCPROP__NO_STEP,                                                                      do_concept_misc_distort},         /* concept_misc_distort */
   {CONCPROP__NO_STEP,                                                                      do_concept_old_stretch},          /* concept_old_stretch */
   {CONCPROP__NO_STEP,                                                                      do_concept_new_stretch},          /* concept_new_stretch */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_c1_phantom_move},              /* concept_c1_phantom */
   {CONCPROP__NO_STEP,                                                                      do_concept_grand_working},        /* concept_grand_working */
   {0,                                                                                      do_concept_centers_or_ends},      /* concept_centers_or_ends */
   {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP,                                             so_and_so_only_move},             /* concept_so_and_so_only */
   {CONCPROP__USE_SELECTOR | CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                     so_and_so_only_move},             /* concept_some_vs_others */
   {0,                                                                                      do_concept_stable},               /* concept_stable */
   {CONCPROP__USE_SELECTOR,                                                                 do_concept_stable},               /* concept_so_and_so_stable */
   {CONCPROP__USE_NUMBER,                                                                   do_concept_stable},               /* concept_frac_stable */
   {CONCPROP__USE_SELECTOR | CONCPROP__USE_NUMBER,                                          do_concept_stable},               /* concept_so_and_so_frac_stable */
   {CONCPROP__USE_SELECTOR | CONCPROP__NO_STEP,                                             do_concept_standard},             /* concept_standard */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK | CONCPROP__USE_SELECTOR,                        do_concept_double_offset},        /* concept_double_offset */
   {CONCPROP__SECOND_CALL,                                                                  do_concept_checkpoint},           /* concept_checkpoint */
   {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              on_your_own_move},                /* concept_on_your_own */
   {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              do_concept_trace},                /* concept_trace */
   {CONCPROP__NO_STEP | CONCPROP__GET_MASK,                                                 do_concept_ferris},               /* concept_ferris */
   {CONCPROP__SECOND_CALL,                                                                  do_concept_centers_and_ends},     /* concept_centers_and_ends */
   {0,                                                                                      do_concept_twice},                /* concept_twice */
   {CONCPROP__SECOND_CALL,                                                                  do_concept_sequential},           /* concept_sequential */
   {0,                                                                                      do_concept_meta},                 /* concept_meta */
   {CONCPROP__USE_SELECTOR,                                                                 do_concept_so_and_so_begin},      /* concept_so_and_so_begin */
   {CONCPROP__USE_NUMBER,                                                                   do_concept_nth_part},             /* concept_nth_part */
   {CONCPROP__USE_NUMBER | CONCPROP__SECOND_CALL,                                           do_concept_replace_nth_part},     /* concept_replace_nth_part */
   {CONCPROP__SECOND_CALL,                                                                  do_concept_interlace},            /* concept_interlace */
   {CONCPROP__USE_NUMBER | CONCPROP__USE_TWO_NUMBERS,                                       do_concept_fractional},           /* concept_fractional */
   {CONCPROP__NO_STEP,                                                                      do_concept_rigger},               /* concept_rigger */
   {CONCPROP__NO_STEP,                                                                      do_concept_slider},               /* concept_slider */
   {CONCPROP__SECOND_CALL | CONCPROP__NO_STEP,                                              do_concept_callrigger}};          /* concept_callrigger */
