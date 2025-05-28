/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992  William B. Ackerman.

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

    This is version 24.0. */

/* This defines the following functions:
   distorted_2x2s_move
   distorted_move
   triple_twin_move
   do_concept_rigger
   do_concept_callrigger
*/


#include "sd.h"



static int list_10_6_5_4[4] = {8, 6, 5, 4};
static int list_11_13_7_2[4] = {9, 11, 7, 2};
static int list_12_17_3_1[4] = {10, 15, 3, 1};
static int list_14_15_16_0[4] = {12, 13, 14, 0};
static int list_14_12_11_10[4] = {12, 10, 9, 8};
static int list_15_17_13_6[4] = {13, 15, 11, 6};
static int list_16_3_7_5[4] = {14, 3, 7, 5};
static int list_0_1_2_4[4] = {0, 1, 2, 4};


extern void distorted_2x2s_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   /* maps for 4x4 Z's */
   static int map1[16] = {12, 15, 11, 10, 3, 2, 4, 7, 12, 3, 7, 10, 15, 2, 4, 11};
   static int map2[16] = {12, 13, 3, 15, 11, 7, 4, 5, 12, 13, 7, 11, 15, 3, 4, 5};
   static int map3[16] = {14, 1, 2, 3, 10, 11, 6, 9, 11, 1, 2, 6, 10, 14, 3, 9};
   static int map4[16] = {13, 14, 1, 3, 9, 11, 5, 6, 13, 14, 11, 9, 3, 1, 5, 6};
   static int map5[16] = {10, 13, 15, 9, 7, 1, 2, 5, 10, 7, 5, 9, 13, 1, 2, 15};
   static int map6[16] = {13, 14, 15, 10, 7, 2, 5, 6, 13, 14, 2, 7, 10, 15, 5, 6};
   static int map7[16] = {3, 0, 1, 7, 9, 15, 11, 8, 15, 0, 1, 11, 9, 3, 7, 8};
   static int map8[16] = {14, 0, 3, 15, 11, 7, 6, 8, 14, 0, 7, 11, 15, 3, 6, 8};

   /* maps for 3x4 Z's */
   static int mapa[16] = {10, 1, 11, 9, 5, 3, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1};
   static int mapb[16] = {-1, -1, -1, -1, -1, -1, -1, -1, 10, 2, 5, 9, 11, 3, 4, 8};
   static int mapc[16] = {-1, -1, -1, -1, -1, -1, -1, -1, 0, 5, 7, 10, 1, 4, 6, 11};
   static int mapd[16] = {0, 11, 8, 10, 2, 4, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1};

   /* maps for 2x6 Z's */
   static int mape[16] = {0, 1, 9, 10, 3, 4, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1};
   static int mapf[16] = {1, 2, 10, 11, 4, 5, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1};

   /* maps for twin parallelograms */
   static int map_p1[16] = {2, 3, 11, 10, 5, 4, 8, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   static int map_p2[16] = {0, 1, 4, 5, 10, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1};

   /* maps for interlocked boxes/interlocked parallelograms */
   static int map_b1[16] = {1, 3, 4, 11, 10, 5, 7, 9, 1, 3, 5, 10, 11, 4, 7, 9};
   static int map_b2[16] = {0, 2, 5, 10, 11, 4, 6, 8, 0, 2, 4, 11, 10, 5, 6, 8};

   /* maps for jays */

   static int mapj1[24] = {
               7, 2, 4, 5, 0, 1, 3, 6,
               6, 3, 4, 5, 0, 1, 2, 7,
               -1, -1, -1, -1, -1, -1, -1, -1};
   static int mapj2[24] = {
               6, 3, 4, 5, 0, 1, 2, 7,
               7, 2, 4, 5, 0, 1, 3, 6,
               -1, -1, -1, -1, -1, -1, -1, -1};
   static int mapj3[24] = {
               -1, -1, -1, -1, -1, -1, -1, -1,
               7, 2, 4, 5, 0, 1, 3, 6,
               6, 3, 4, 5, 0, 1, 2, 7};
   static int mapj4[24] = {
               -1, -1, -1, -1, -1, -1, -1, -1,
               6, 3, 4, 5, 0, 1, 2, 7,
               7, 2, 4, 5, 0, 1, 3, 6};

   /* maps for facing/back-to-front/back-to-back parallelograms */

   static int mapk1[24] = {
               3, 2, 4, 5, 0, 1, 7, 6,
               6, 7, 4, 5, 0, 1, 2, 3,
               -1, -1, -1, -1, -1, -1, -1, -1};
   static int mapk2[24] = {
               6, 7, 4, 5, 0, 1, 2, 3,
               3, 2, 4, 5, 0, 1, 7, 6,
               -1, -1, -1, -1, -1, -1, -1, -1};
   static int mapk3[24] = {
               -1, -1, -1, -1, -1, -1, -1, -1,
               3, 2, 4, 5, 0, 1, 7, 6,
               6, 7, 4, 5, 0, 1, 2, 3};
   static int mapk4[24] = {
               -1, -1, -1, -1, -1, -1, -1, -1,
               6, 7, 4, 5, 0, 1, 2, 3,
               3, 2, 4, 5, 0, 1, 7, 6};

   int i;
   int table_offset;
   int misc_indicator;
   setup a1, a2;
   setup res1, res2;
   int *map_ptr;

   /* Check for special case of "interlocked parallelogram", which doesn't look like the
      kind of concept we are expecting. */
   
   if (parseptr->concept->kind == concept_do_both_boxes) {
      table_offset = 8;
      misc_indicator = 3;
   }
   else {
      table_offset = parseptr->concept->value.arg2;
      misc_indicator = parseptr->concept->value.arg1;
   }
   
   /* misc_indicator    concept                                                      */
   /*      0         some kind of "Z"                                                */
   /*      1         some kind of "Jay"                                              */
   /*      2       twin parallelograms (the physical setup is like offset lines)     */
   /*      3       interlocked boxes/Pgrams (the physical setup is like Z's)         */
   /*      4         some kind of parallelogram (as from "heads travel thru")        */
   /* Concept is Jay, "Z", or offset parallelogram/interlocked boxes.                */
   
   /* table_offset is 0, 8, or 12.  It selects the appropriate part of the maps. */
   /* For "Z"      :   0 == normal, 8 == interlocked. */
   /* For Jay/Pgram:   0 == normal, 8 == back-to-front, 16 == back-to-back. */
   
   switch (misc_indicator) {
      case 0:
         switch (ss->kind) {   /* The concept is some variety of "Z" */
            case s4x4:
               if (!(ss->people[0].id1 | ss->people[8].id1)) {
                  if (!(ss->people[1].id1 | ss->people[14].id1 | ss->people[6].id1 | ss->people[9].id1)) {
                     if (!(ss->people[5].id1 | ss->people[13].id1))
                         map_ptr = map1;
                     else if (!(ss->people[2].id1 | ss->people[10].id1))
                         map_ptr = map2;
                     else goto lose;
                  }
                  else if (!(ss->people[4].id1 | ss->people[7].id1 | ss->people[12].id1 | ss->people[15].id1)) {
                     if (!(ss->people[5].id1 | ss->people[13].id1))
                         map_ptr = map3;
                     else if (!(ss->people[2].id1 | ss->people[10].id1))
                         map_ptr = map4;
                     else goto lose;
                  }
                  else if (!(ss->people[3].id1 | ss->people[4].id1 | ss->people[11].id1 | ss->people[12].id1)) {
                     if (!(ss->people[6].id1 | ss->people[14].id1))
                         map_ptr = map5;
                     else if (!(ss->people[1].id1 | ss->people[9].id1))
                         map_ptr = map6;
                     else goto lose;
                  }
                  else goto lose;
               }
               else if (!(ss->people[4].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[12].id1 | ss->people[10].id1 | ss->people[13].id1)) {
                  if (!(ss->people[6].id1 | ss->people[14].id1))
                     map_ptr = map7;
                  else if (!(ss->people[1].id1 | ss->people[9].id1))
                     map_ptr = map8;
                  else goto lose;
               }
               else goto lose;
               break;
            case s3x4:
               if (!(ss->people[0].id1 | ss->people[6].id1)) {
                  if (!(ss->people[2].id1 | ss->people[8].id1))
                     map_ptr = mapa;
                  else if (!(ss->people[1].id1 | ss->people[7].id1))
                     map_ptr = mapb;
                  else goto lose;
               }
               else if (!(ss->people[3].id1 | ss->people[9].id1)) {
                  if (!(ss->people[2].id1 | ss->people[8].id1))
                     map_ptr = mapc;
                  else if (!(ss->people[1].id1 | ss->people[7].id1))
                     map_ptr = mapd;
                  else goto lose;
               }
               else goto lose;
               break;
            case s2x6:
               if (!(ss->people[2].id1 | ss->people[5].id1 | ss->people[8].id1 | ss->people[11].id1))
                  map_ptr = mape;
               else if (!(ss->people[0].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[9].id1))
                  map_ptr = mapf;
               else goto lose;
               break;
            default:
               fail("Must have 3x4, 2x6, or 4x4 setup for this concept.");
         }
         break;
      case 1:
         {               
         switch (ss->kind)  /* The concept is some variety of jay */
            {
            case s_qtag:
               if (((ss->people[0].id1&d_mask) == d_south && (ss->people[1].id1&d_mask) == d_south &&
                  (ss->people[4].id1&d_mask) == d_north && (ss->people[5].id1&d_mask) == d_north)
                                 || table_offset == 0)
                  {
                  if ((ss->people[6].id1&d_mask) == d_north && (ss->people[7].id1&d_mask) == d_south &&
                        (ss->people[3].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_south)
                     map_ptr = mapj1;
                  else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[7].id1&d_mask) == d_north &&
                        (ss->people[3].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_north)
                     map_ptr = mapj2;
                  else goto lose;
                  }
               else if ((ss->people[0].id1&d_mask) == d_north && (ss->people[1].id1&d_mask) == d_north &&
                     (ss->people[4].id1&d_mask) == d_south && (ss->people[5].id1&d_mask) == d_south)
                  {
                  if ((ss->people[6].id1&d_mask) == d_north && (ss->people[7].id1&d_mask) == d_south &&
                        (ss->people[3].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_south)
                     map_ptr = mapj3;
                  else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[7].id1&d_mask) == d_north &&
                        (ss->people[3].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_north)
                     map_ptr = mapj4;
                  else goto lose;
                  }
               else goto lose;
               break;
            default:
               fail("Must have quarter-tag setup for this concept.");
            }
         break;
         }
      case 2:
         switch (ss->kind)     /* The concept is twin parallelograms */
            {
            case s3x4:
               if (!(ss->people[0].id1 | ss->people[6].id1 | ss->people[1].id1 | ss->people[7].id1))
                  map_ptr = map_p1;
               else if (!(ss->people[3].id1 | ss->people[9].id1 | ss->people[2].id1 | ss->people[8].id1))
                  map_ptr = map_p2;
               else goto lose;
               break;
            default:
               fail("Must have 3x4 setup for this concept.");
            }
         break;
      case 3:
         switch (ss->kind) {   /* The concept is interlocked boxes or interlocked parallelograms */
            case s3x4:
               if (!(ss->people[0].id1 | ss->people[6].id1 | ss->people[2].id1 | ss->people[8].id1))
                  map_ptr = map_b1;
               else if (!(ss->people[3].id1 | ss->people[9].id1 | ss->people[1].id1 | ss->people[7].id1))
                  map_ptr = map_b2;
               else goto lose;
               break;
            default:
               fail("Must have 3x4 setup for this concept.");
         }
         break;
      case 4:
         switch (ss->kind) {  /* The concept is facing (or back-to-back, or front-to-back) Pgram */
            case s_qtag:
               if ((ss->people[0].id1&d_mask) == d_south && (ss->people[1].id1&d_mask) == d_south &&
                           (ss->people[4].id1&d_mask) == d_north && (ss->people[5].id1&d_mask) == d_north) {
                  if ((ss->people[6].id1&d_mask) == d_north && (ss->people[7].id1&d_mask) == d_north &&
                              (ss->people[3].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_south)
                     map_ptr = mapk1;
                  else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[7].id1&d_mask) == d_south &&
                           (ss->people[3].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_north)
                     map_ptr = mapk2;
                  else goto lose;
               }
               else if ((ss->people[0].id1&d_mask) == d_north && (ss->people[1].id1&d_mask) == d_north &&
                        (ss->people[4].id1&d_mask) == d_south && (ss->people[5].id1&d_mask) == d_south) {
                  if ((ss->people[6].id1&d_mask) == d_north && (ss->people[7].id1&d_mask) == d_north &&
                           (ss->people[3].id1&d_mask) == d_south && (ss->people[2].id1&d_mask) == d_south)
                     map_ptr = mapk3;
                  else if ((ss->people[6].id1&d_mask) == d_south && (ss->people[7].id1&d_mask) == d_south &&
                           (ss->people[3].id1&d_mask) == d_north && (ss->people[2].id1&d_mask) == d_north)
                     map_ptr = mapk4;
                  else goto lose;
               }
               else goto lose;
               break;
            default:
               fail("Must have quarter-line setup for this concept.");
         }
         break;
   }
   
   if (map_ptr[table_offset] < 0) goto lose;
   
   result->kind = ss->kind;
   result->rotation = 0;
   
   for (i=0; i<4; i++) {
      (void) copy_person(&a1, i, ss, map_ptr[i+table_offset]);
      (void) copy_person(&a2, i, ss, map_ptr[i+table_offset+4]);
   }
   
   a1.kind = s2x2;
   a1.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);
   
   a2.kind = s2x2;
   a2.rotation = 0;
   a2.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   move(&a2, parseptr->next, NULLCALLSPEC, 0, FALSE, &res2);
   
   if (res1.kind != s2x2 || res2.kind != s2x2) fail("Can't do shape-changer with this concept.");
   
   for (i=0; i<4; i++) {
      (void) copy_person(result, map_ptr[i+table_offset], &res1, i);
      (void) copy_person(result, map_ptr[i+table_offset+4], &res2, i);
   }
   
   result->setupflags = res1.setupflags | res2.setupflags;
   return;
   
   lose: fail("Can't find the indicated formation.");
}



static long_boolean search_row(
   int n,
   int *x1,
   int *x2,
   int row[8],
   setup *s)

   /* This returns true if the two live people were consecutive, meaning this is a true Z line/column. */

   {
   int i, z, fill_count;
   long_boolean win;
   
   fill_count = 0;
   z = 2;                       /* counts downward as we see consecutive people */
   win = FALSE;
   for (i=0; i<n; i++)
      if (!s->people[row[i]].id1)           /* live person here? */
         z = 2;                 /* no, reset the consecutivity counter */
      else
         {
         --z;                   /* yes, count -- if get two in a row, turn "win" on */
         if (!z) win = TRUE;
         /* then push this index onto the map, check for no more than 2 */
         fill_count++;
         switch (fill_count)
            {
            case 1: *x1 = row[i]; break;
            case 2: *x2 = row[i]; break;
            default: fail("Can't identify distorted line or column.");
            }
         }
   
   /* Now check that the map has exactly 2 people. */
   
   if (fill_count != 2) fail("Can't identify distorted line or column.");
   
   /* And return this. */
   
   return(win);
   }


extern void distorted_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{

/*
   Args from the concept are as follows:
   arg1 =
      0 - user claims this is some kind of columns
      1 - user claims this is some kind of lines
   arg2 =
      disttest_offset - user claims this is offset lines/columns
      disttest_z      - user claims this is Z lines/columns
      disttest_any    - user claims this is distorted lines/columns

   Tbonetest has the OR of all the people, or all the standard people if
      this is "standard", so it is what we look at to interpret the
      lines vs. columns nature of the concept.
*/

   static int list_0_12_11[3] = {0, 10, 9};
   static int list_1_13_10[3] = {1, 11, 8};
   static int list_2_5_7[3] = {2, 5, 7};
   static int list_3_4_6[3] = {3, 4, 6};

   int tbonetest;
   int livemask;
   int the_map[8];
   int linesp;
   long_boolean zlines;
   disttest_kind disttest;
   int i, rot, rotz;
   setup a1;
   setup res1;
   map_thing *map_ptr;

   tbonetest = global_tbonetest;
   livemask = global_livemask;

   linesp = parseptr->concept->value.arg1;
   disttest = (disttest_kind) parseptr->concept->value.arg2;
   zlines = TRUE;

   if (ss->kind == s4x4) {
      int vertical = (linesp ^ tbonetest) & 1;

      /* **** This is all sort of a crock.  Here we are picking out the "winning" method for
        doing distorted/offset/Z lines and columns.  Below, we do it by the old way, which allows
        T-bone setups. */
   
      /* If any people are T-boned, we must invoke the other method and hope for the best.
         ***** We will someday do it right. */
         
      if ((tbonetest & 011) == 011) {
         if (disttest != disttest_offset)
            fail("Sorry, can't apply this concept when people are T-boned.");
   
         phantom_2x4_move(ss, linesp, phantest_only_one, &(map_offset), parseptr->next, result);
         return;
      }
   
      /* Check for special case of offset lines/columns, and do it the elegant way (handling shape-changers) if so. */

      /* Search for the live people.
         Vertical == 0 if must scan sideways for each Y value, looking for exactly 2 people
         Vertical == 1 if must scan vertically for each X value, looking for exactly 2 people.
         If any of the scans returns false, meaning that the 2 people are not adjacent,
         set zlines to false. */
      
      if (vertical) {
         if (livemask == 0xB4B4) { map_ptr = &map_rf_s2x4_r; goto do_divided_call; }
         else if (livemask == 0x4B4B) { map_ptr = &map_lf_s2x4_r; goto do_divided_call; }
         else {
            if (!search_row(4, &the_map[0], &the_map[7], list_14_12_11_10, ss)) zlines = FALSE;
            if (!search_row(4, &the_map[1], &the_map[6], list_15_17_13_6, ss)) zlines = FALSE;
            if (!search_row(4, &the_map[2], &the_map[5], list_16_3_7_5, ss)) zlines = FALSE;
            if (!search_row(4, &the_map[3], &the_map[4], list_0_1_2_4, ss)) zlines = FALSE;
         }
      }
      else {
         if (livemask == 0xB4B4) { map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_L_FULL][1]; goto do_divided_call; }
         else if (livemask == 0x4B4B) { map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_R_FULL][1]; goto do_divided_call; }
         else {
            if (!search_row(4, &the_map[0], &the_map[7], list_10_6_5_4, ss)) zlines = FALSE;
            if (!search_row(4, &the_map[1], &the_map[6], list_11_13_7_2, ss)) zlines = FALSE;
            if (!search_row(4, &the_map[2], &the_map[5], list_12_17_3_1, ss)) zlines = FALSE;
            if (!search_row(4, &the_map[3], &the_map[4], list_14_15_16_0, ss)) zlines = FALSE;
         }
      }
   
      rot = (1 - vertical) * 011;
      result->kind = s4x4;
      rotz = (1 - vertical) * 033;
   }
   else if (ss->kind == s3x4) {
      if (linesp & 1) {
         if (tbonetest & 1) fail("There are no lines of 4 here.");
      }
      else {
         if (tbonetest & 010) fail("There are no columns of 4 here.");
      }

      /* Check for special case of offset lines/columns, and do it the elegant way (handling shape-changers) if so. */
      
      if (livemask == 07474) { map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_L_HALF][1]; goto do_divided_call; }
      else if (livemask == 06363) { map_ptr = (*map_lists[s2x4][0])[MPKIND__OFFS_R_HALF][1]; goto do_divided_call; }

      /* Search for the live people. */
      /* Minor loss!!!  Search row should use symmetry, and refuse to do its job on unsymmetrical stuff.
         Why?  Because, if it turns out that the result of "move" flips the whole thing over,
         so that res1.rot = 2, we just set result->rot = 2 and proceed.  This would violate the
         true shape of the result setup. */
      
      if (!search_row(3, &the_map[0], &the_map[7], list_0_12_11, ss)) zlines = FALSE;
      if (!search_row(3, &the_map[1], &the_map[6], list_1_13_10, ss)) zlines = FALSE;
      if (!search_row(3, &the_map[2], &the_map[5], list_2_5_7, ss)) zlines = FALSE;
      if (!search_row(3, &the_map[3], &the_map[4], list_3_4_6, ss)) zlines = FALSE;
      
      rot = 0;
      result->kind = s3x4;      
      rotz = 0;
   }
   else
      fail("Must have 3x4 or 4x4 setup for this concept.");

   /* Now see if the concept was correctly named. */

   if (zlines) {
      if (disttest != disttest_z)
         fail("You must specify Z lines/columns when in this setup.");
   }
   else {
      switch (disttest) {
         case disttest_z:
            fail("Can't find Z lines/columns, perhaps you mean distorted.");
         case disttest_offset:
            fail("Can't find offset lines/columns, perhaps you mean distorted.");
      }
   }

   for (i=0; i<8; i++) (void) copy_rot(&a1, i, ss, the_map[i], rot);

   a1.kind = s2x4;
   a1.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);

   if (res1.kind != s2x4 || (res1.rotation & 1)) fail("Can only do non-shape-changing 2x4 -> 2x4 calls in Z or distorted setups.");
   result->rotation = res1.rotation;
   for (i=0; i<8; i++) (void) copy_rot(result, the_map[i], &res1, i, rotz);
   result->setupflags = res1.setupflags;

   reinstate_rotation(ss, result);
   return;
   
   do_divided_call:

   if (disttest != disttest_offset)
      fail("You must specify offset lines/columns when in this setup.");

   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
      map_ptr, phantest_ok, TRUE, result);
}


extern void triple_twin_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int tbonetest;
   int i;
   static short source_indices[16]  = {4, 7, 22, 8, 13, 14, 15, 21, 16, 19, 10, 20, 1, 2, 3, 9};

   /* Arg1 = 1 for triple twin columns, 0 for triple twin lines. */

   /* The setup has not necessarily been expanded to a 4x6.  It has only been
      expanded to a 4x4.  Why?  Because the stuff in toplevelmove that does
      the expansion didn't know which way to orient the 4x6.  (It might have
      depended on "standard" information.)  So why is the expansion supposed
      to be done there rather than at the point where the concept is executed?
      It is to prevent monstrosities like "in your split phantom line you
      have a split phantom column".  Expansion takes place only at the top
      level.  When the concept "in your split phantom line" is executed,
      toplevelmove expands the setup to a 4x4, the concept routine splits
      it into 2x4's, and then the second concept is applied without further
      expansion.  Since we now have 2x4's and the you have a split phantom
      column" concept requires a 4x4, it will raise an error.  If the
      expansion were done wherever a concept is performed, this monstrosity
      would by permitted to occur.  So the remaining question is "What
      safety are we sacrificing (or what monstrosities are we permitting)
      by doing the expansion here?"  The answer is that, if there were a
      concept that divided the setup into 4x4's, we could legally do
      something like "in your split phantom 4x4's you have triple twin
      columns".  It would expand each 4x4 into a 4x6 and go the call.
      Horrors.  Fortunately, there are no such concepts.  Of course
      the really right way to do this is to have a setupflag called
      NOEXPAND, and do the expansion when the concept is acted upon.
      Anyway, here goes. */

   tbonetest = global_tbonetest;

   if ((tbonetest & 011) == 011) fail("Can't do this from T-bone setup.");

   tbonetest ^= parseptr->concept->value.arg1;

   if (ss->kind == s4x4) {
      setup stemp;
      stemp = *ss;
      clear_people(ss);

      if (tbonetest & 1) {
         for (i=0; i<16; i++) copy_person(ss, source_indices[i], &stemp, i);
      }
      else {
         for (i=0; i<16; i++) (void) copy_rot(ss, source_indices[i], &stemp, (i+4) & 0xF, 033);
         ss->rotation++;
         tbonetest ^= 1;    /* Fix it. */
      }
   
      ss->kind = s4x6;

   }
   else if (ss->kind != s4x6) fail("Must have a 4x6 setup for this concept.");
   
   if (!(tbonetest & 1)) {
      if (parseptr->concept->value.arg1) fail("Can't find triple twin columns.");
      else fail("Can't find triple twin lines.");
   }
   
   divided_setup_move(ss, parseptr->next, NULLCALLSPEC, 0,
         (*map_lists[s2x4][2])[MPKIND__SPLIT][1], phantest_not_just_centers, TRUE, result);
}



extern void do_concept_rigger(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   /* First half is for wing; second half is for 1/4-tag. */
   /* A huge coincidence is at work here -- the two halves of the maps are the same. */
   static int map1[16] = {0, 1, 3, 2, 4, 5, 7, 6, 0, 1, 3, 2, 4, 5, 7, 6};
   static int map2[16] = {6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5};

   int rstuff, i, indicator;
   setup a1;
   setup res1;
   int *map_ptr;

   rstuff = parseptr->concept->value.arg1;
   /* rstuff =
      outrigger   : 0
      leftrigger  : 1
      inrigger    : 2
      rightrigger : 3 */

   if (ss->kind != s_rigger) fail("Must have a 'rigger' setup to do this concept.");

   if (!(ss->people[2].id1 & ss->people[6].id1 & BIT_PERSON))
      fail("Can't tell which way 'rigger' people are facing.");

   if (((ss->people[2].id1 ^ ss->people[6].id1) & 3) != 2)
      fail("'Rigger' people are not facing consistently!");

   indicator = (ss->people[6].id1 ^ rstuff) & 3;

   if (indicator & 1)
      fail("'Rigger' direction is inappropriate.");

   if (indicator)
       map_ptr = map1;
   else
       map_ptr = map2;

   for (i=0; i<8; i++) (void) copy_person(&a1, i, ss, map_ptr[i]);
   
   a1.kind = s2x4;
   a1.rotation = 0;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);
   
   if (res1.kind != s2x4) fail("Can only do 2x4 -> 2x4 calls.");

   if ((res1.rotation) & 1) {
      result->rotation = res1.rotation;
      for (i=0; i<8; i++) (void) copy_person(result, map_ptr[i+8], &res1, i);
      result->kind = s_qtag;
   }
   else {
      result->rotation = res1.rotation;
      for (i=0; i<8; i++) (void) copy_person(result, map_ptr[i], &res1, i);
      result->kind = s_rigger;
   }
   
   result->setupflags = res1.setupflags;
   reinstate_rotation(ss, result);
}



typedef struct {
   int map_a1[4];
   int map_a2[4];
   setup_kind start_kind;
   setup_kind end_kind;
   int outrot;
   int parity;
   int bigmap1[8];
   int bigmap2[8];
   } rig_thing;

static rig_thing rig_wing = {{3, 2, -1, -1}, {-1, -1, 7, 6}, s1x4, s2x2, 0, 0, {0, 1, 8, 9, 4, 5, 14, 15}, {12, 13, 0, 1, 10, 11, 4, 5}};
static rig_thing rig_2x4 =  {{3, -1, -1, 4}, {-1, 0, 7, -1}, s2x2, s2x2, 0, 0, {1, 2, 8, 9, 5, 6, 14, 15}, {12, 13, 1, 2, 10, 11, 5, 6}};
static rig_thing rig_xwv =  {{5, 4, -1, -1}, {-1, -1, 1, 0}, s1x4, s1x4, 1, 0, {8, 9, 7, 6, 14, 15, 3, 2}, {2, 3, 11, 10, 6, 7, 13, 12}};
static rig_thing rig_qtag=  {{-1, -1, 1, 0}, {5, 4, -1, -1}, s2x2, s1x4, 0, 1, {8, 9, 3, 2, 14, 15, 7, 6}, {6, 7, 11, 10, 2, 3, 13, 12}};



extern void do_concept_callrigger(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   /* First half is for wing; second half is for 1/4-tag. */
   /* A huge coincidence is at work here -- the two halves of the maps are the same. */
   static int map1[16] = {0, 1, 3, 2, 4, 5, 7, 6, 0, 1, 3, 2, 4, 5, 7, 6};
   static int map2[16] = {6, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5};

   int i, j, signature;
   setup a1, a2;
   rig_thing *rig_map;
   int *map_ptr;
   setup z[2];

   switch (ss->kind) {
      case s_rigger: rig_map = &rig_wing; break;
      case s2x4: rig_map = &rig_2x4; break;
      case s_crosswave: rig_map = &rig_xwv; break;
      case s_qtag: rig_map = &rig_qtag; break;
      default: fail("Must have a 'rigger' setup to do this concept.");
   }

   clear_people(&a1);
   clear_people(&a2);

   /* Pull out the wings and have them do the call. */

   for (i=0; i<4; i++) {
      if (rig_map->map_a1[i] >= 0)
         (void) copy_person(&a1, i, ss, rig_map->map_a1[i]);

      if (rig_map->map_a2[i] >= 0)
         (void) copy_person(&a2, i, ss, rig_map->map_a2[i]);
   }


   a1.rotation = 0;
   a1.kind = rig_map->start_kind;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;

   a2.rotation = 0;
   a2.kind = rig_map->start_kind;
   a2.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;

   move(&a1, parseptr->next, NULLCALLSPEC, 0, FALSE, &z[0]);
   move(&a2, parseptr->next, NULLCALLSPEC, 0, FALSE, &z[1]);
   result->setupflags = z[0].setupflags | z[1].setupflags;

   if (fix_n_results(2, z))
      fail("There are no wings???");

   if (z[0].kind != rig_map->end_kind) fail("Can't figure out result of rigging call.");

   if (z[0].rotation != rig_map->outrot) fail("Sorry, can't handle this rotation from rigging call.");

   signature = 0;

   for (i=0, j=1; i<=4; i++, j<<=1) {
      if (z[0].people[i].id1) {
         signature |= j;
      }
      if (z[1].people[i].id1) {
         signature |= (j<<4);
      }
   }

   /* Copy the people from the various sources into one place (a2) for easier indexing. */

   for (i=0; i<8; i++) {
      if (rig_map->outrot)
         (void) copy_rot(&a2, i, ss, i, 033);
      else
         (void) copy_person(&a2, i, ss, i);
   }

   for (i=0; i<4; i++) {
      (void) copy_person(&a2, i+8, &z[0], i);
      (void) copy_person(&a2, i+12, &z[1], i);
   }

   if (signature == 0xC3) {
      for (i=0; i<8; i++) (void) copy_person(&a1, i, &a2, rig_map->bigmap1[i]);
      map_ptr = map1;
   }
   else if (signature == 0x3C) {
      for (i=0; i<8; i++) (void) copy_person(&a1, i, &a2, rig_map->bigmap2[i]);
      map_ptr = map2;
   }
   else fail("Can't figure out which way to rig.");

   a1.kind = s2x4;
   a1.rotation = rig_map->outrot;
   a1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   move(&a1, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, &z[0]);
   
   if (z[0].kind != s2x4) fail("Can only do 2x4 -> 2x4 calls.");
   result->rotation = z[0].rotation;

   if ((z[0].rotation ^ rig_map->parity) & 1) {
      for (i=0; i<8; i++) (void) copy_person(result, map_ptr[i+8], &z[0], i);
      result->kind = s_qtag;
   }
   else {
      for (i=0; i<8; i++) (void) copy_person(result, map_ptr[i], &z[0], i);
      result->kind = s_rigger;
   }
   
   result->setupflags |= z[0].setupflags;
   reinstate_rotation(ss, result);
}
