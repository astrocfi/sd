/* SD -- square dance caller's helper.

    Copyright (C) 1990-1997  William B. Ackerman.

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
   triangle_move
*/

#include "sd.h"


typedef struct qwerty {
   Const setup_kind kind;
   Const struct qwerty *other;
   Const veryshort mapqt1[8];   /* In quarter-tag: first triangle (upright), then second triangle (inverted), then idle. */
   Const veryshort mapcp1[8];   /* In C1 phantom: first triangle (inverted), then second triangle (upright), then idle. */
   Const veryshort mapbd1[8];   /* In bigdmd. */
   Const veryshort map241[8];   /* In 2x4. */
   Const veryshort map261[8];   /* In 2x6. */
} tgl_map;

/* These need to be predeclared so that they can refer to each other. */
static Const tgl_map map1b;
static Const tgl_map map2b;
static Const tgl_map map1i;
static Const tgl_map map2i;
static Const tgl_map map1j;
static Const tgl_map map2j;
static Const tgl_map map1k;
static Const tgl_map map2k;

/*                             kind     other
               mapqt1                         mapcp1                         mapbd1                            map241                    map261 */

static Const tgl_map map1b = {s_c1phan, &map2b,
   {4, 3, 2,   0, 7, 6,   1, 5}, {6,  8, 10,   14,  0,  2,   4, 12}, {10, 9, 8,   4, 3, 2,   5, 11}, {6, 5, 4, 2, 1, 0, 3, 7},  {10, 9, 8, 4, 3, 2, 5, 11}};
static Const tgl_map map2b = {s_qtag,   &map1b,                                                                                                         
   {5, 6, 7,   1, 2, 3,   0, 4}, {3, 15, 13,   11,  7,  5,   1,  9}, {1, 2, 3,    7, 8, 9,   0, 6},  {7, 6, 5, 3, 2, 1, 0, 4},  {1, 2, 3, 7, 8, 9, 0, 6}};
/* Interlocked triangles: */                                                                                                                            
static Const tgl_map map1i = {nothing,  &map2i,                                                                                                         
   {0, 0, 0,   0, 0, 0,   0, 0}, {4,  8, 10,   12,  0,  2,   6, 14}, {0, 0, 0,    0, 0, 0,   0, 0},  {0, 0, 0, 0, 0, 0, 0, 0},  {0, 0, 0, 0, 0, 0, 0, 0}};
static Const tgl_map map2i = {nothing,  &map1i,                                                                                                         
   {0, 0, 0,   0, 0, 0,   0, 0}, {1, 15, 13,   9,   7,  5,   3, 11}, {0, 0, 0,    0, 0, 0,   0, 0},  {0, 0, 0, 0, 0, 0, 0, 0},  {0, 0, 0, 0, 0, 0, 0, 0}};
/* Interlocked triangles in quarter-tag: */                                                                                                             
static Const tgl_map map1j = {s_qtag,   &map2j,                                                                                                         
   {4, 7, 2,   0, 3, 6,   1, 5}, {0,  0,  0,    0,  0,  0,   0, -1}, {0, 0, 0,    0, 0, 0,   0, 0},  {0, 0, 0, 0, 0, 0, 0, 0},  {0, 0, 0, 0, 0, 0, 0, 0}};
static Const tgl_map map2j = {s_qtag,   &map1j,                                                                                                         
   {5, 6, 3,   1, 2, 7,   0, 4}, {0,  0,  0,    0,  0,  0,   0, -1}, {0, 0, 0,    0, 0, 0,   0, 0},  {0, 0, 0, 0, 0, 0, 0, 0},  {0, 0, 0, 0, 0, 0, 0, 0}};
/* Interlocked triangles in bigdmd: */                                                                                                                  
static Const tgl_map map1k = {nothing,  &map2k,                                                                                                         
   {0, 0, 0,   0, 0, 0,   0, 0}, {0,  0,  0,    0,  0,  0,   0, -1}, {10, 3, 8,   4, 9, 2,   5, 11}, {0, 0, 0, 0, 0, 0, 0, 0},  {0, 0, 0, 0, 0, 0, 0, 0}};
static Const tgl_map map2k = {nothing,  &map1k,                                                                                                         
   {0, 0, 0,   0, 0, 0,   0, 0}, {0,  0,  0,    0,  0,  0,   0, -1}, {1, 2, 9,    7, 8, 3,   0, 6},  {0, 0, 0, 0, 0, 0, 0, 0},  {0, 0, 0, 0, 0, 0, 0, 0}};


Private void do_glorious_triangles(
   setup *ss,
   Const tgl_map *map_ptr,
   setup *result)
{
   int i, r, startingrot;
   setup a1, a2;
   setup idle;
   setup res[2];
   Const veryshort *mapnums;

   if (ss->kind == s_c1phan) {
      mapnums = map_ptr->mapcp1;
      startingrot = 2;
   }
   else if (ss->kind == sbigdmd) {
      mapnums = map_ptr->mapbd1;
      startingrot = 1;
   }
   else {   /* s_qtag */
      mapnums = map_ptr->mapqt1;
      startingrot = 0;
   }

   r = ((-startingrot) & 3) * 011;
   gather(&a1, ss, mapnums, 2, r);
   gather(&a2, ss, &mapnums[3], 2, r^022);

   /* Save the two people who don't move. */
   (void) copy_person(&idle, 0, ss, mapnums[6]);
   (void) copy_person(&idle, 1, ss, mapnums[7]);

   a1.cmd = ss->cmd;
   a2.cmd = ss->cmd;
   a1.kind = s_trngl;
   a2.kind = s_trngl;
   a1.rotation = 0;
   a2.rotation = 0;
   a1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a2.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   move(&a1, FALSE, &res[0]);
   move(&a2, FALSE, &res[1]);

   if (fix_n_results(2, res)) {
      result->kind = nothing;
      result->result_flags = 0;
      return;
   }

   result->result_flags = get_multiple_parallel_resultflags(res, 2);

   /* Check for non-shape-or-orientation-changing result. */

   if (res[0].kind == s_trngl && res[0].rotation == 0) {
      result->kind = ss->kind;
      result->rotation = 0;
      /* Restore the two people who don't move. */
      (void) copy_person(result, mapnums[6], &idle, 0);   
      (void) copy_person(result, mapnums[7], &idle, 1);
      r = startingrot * 011;
      scatter(result, &res[0], mapnums, 2, r);
      scatter(result, &res[1], &mapnums[3], 2, r^022);
      return;
   }

   res[0].rotation += startingrot;
   res[1].rotation += startingrot;
   canonicalize_rotation(&res[0]);
   canonicalize_rotation(&res[1]);

   result->rotation = res[0].rotation;

   r = ((-res[0].rotation) & 3) * 011;

   if (res[0].rotation & 2)
      result->kind = map_ptr->other->kind;
   else
      result->kind = map_ptr->kind;

   if (res[0].kind == s_trngl) {
      /* We know that res[0].rotation != startingrot */
      if (startingrot == 1) fail("Sorry, can't do this.");

      if (res[0].rotation == 0) {
         if (result->kind == nothing) fail("Can't do shape-changer in interlocked triangles.");
         result->kind = s_qtag;
         /* Restore the two people who don't move. */
         (void) copy_person(result, map_ptr->mapqt1[6], &idle, 0);
         (void) copy_person(result, map_ptr->mapqt1[7], &idle, 1);
         scatter(result, &res[0], map_ptr->mapqt1, 2, 0);
         scatter(result, &res[1], &map_ptr->mapqt1[3], 2, 022);
      }
      else if (res[0].rotation == 2) {
         if (map_ptr->mapcp1[7] < 0)
            fail("Can't do shape-changer in interlocked triangles.");

         result->kind = s_c1phan;
         /* Restore the two people who don't move. */
         (void) copy_rot(result, map_ptr->mapcp1[7], &idle, 0, r);
         (void) copy_rot(result, map_ptr->mapcp1[6], &idle, 1, r);
         scatter(result, &res[0], &map_ptr->mapcp1[3], 2, 0);
         scatter(result, &res[1], map_ptr->mapcp1, 2, 022);
      }
      else {
         if (result->kind == nothing || map_ptr->mapcp1[7] < 0)
            fail("Can't do shape-changer in interlocked triangles.");

         map_ptr = map_ptr->other;

         if (result->kind == s_c1phan) {
            /* Restore the two people who don't move. */
            (void) copy_rot(result, map_ptr->mapcp1[6], &idle, 0, r);
            (void) copy_rot(result, map_ptr->mapcp1[7], &idle, 1, r);
   
             /* Copy the triangles. */
            scatter(result, &res[1], map_ptr->mapcp1, 2, 022);
            scatter(result, &res[0], &map_ptr->mapcp1[3], 2, 0);
         }
         else {
            /* Restore the two people who don't move. */
            (void) copy_rot(result, map_ptr->mapqt1[7], &idle, 0, r);
            (void) copy_rot(result, map_ptr->mapqt1[6], &idle, 1, r);

            /* Copy the triangles. */
            scatter(result, &res[0], map_ptr->mapqt1, 2, 0);
            scatter(result, &res[1], &map_ptr->mapqt1[3], 2, 022);
         }
      }
   }
   else if (res[0].kind == s1x3) {
      if (result->kind == nothing || map_ptr->mapcp1[7] < 0)
         fail("Can't do shape-changer in interlocked triangles.");

      result->kind = s2x4;

      if (res[0].rotation == 0) {
         if (startingrot == 1) {
            mapnums = map_ptr->map261;
            result->kind = s2x6;
         }
         else {
            mapnums = map_ptr->map241;
         }

         (void) copy_person(result, mapnums[6], &idle, 0);
         (void) copy_person(result, mapnums[7], &idle, 1);
         scatter(result, &res[0], mapnums, 2, 0);
         scatter(result, &res[1], &mapnums[3], 2, 022);
      }
      else {
         if (startingrot == 1)
            fail("Can't do this shape-changer.");

         map_ptr = map_ptr->other;

         if (map_ptr->kind == s_qtag) {    /* What a crock! */
            (void) copy_rot(result, map_ptr->map241[6], &idle, 0, r);
            (void) copy_rot(result, map_ptr->map241[7], &idle, 1, r);

            for (i=0; i<3; i++) {
               (void) copy_person(result, map_ptr->map241[i+3], &res[0], 2-i);
               (void) copy_rot(result, map_ptr->map241[i], &res[1], 2-i, 022);
            }
         }
         else {
            (void) copy_rot(result, map_ptr->map241[7], &idle, 0, r);
            (void) copy_rot(result, map_ptr->map241[6], &idle, 1, r);
            scatter(result, &res[0], map_ptr->map241, 2, 0);
            scatter(result, &res[1], &map_ptr->map241[3], 2, 022);
         }
      }
   }
   else
      fail("Improper result from triangle call.");

   if (result->kind == s2x4)
       warn(warn__check_2x4);
   else if (result->kind == s2x6)
       warn(warn__check_pgram);
   else if (res[0].rotation != startingrot) {
      if (result->kind == s_c1phan)
          warn(warn__check_c1_phan);
      else
          warn(warn__check_dmd_qtag);
   }
}




/* This procedure does wave-base, tandem-base, and so-and-so-base. */
Private void wv_tand_base_move(
   setup *s,
   int indicator,
   setup *result)
{
   uint32 tbonetest;
   int t;
   calldef_schema schema;
   Const tgl_map *map_ptr;

   switch (s->kind) {
      case s_galaxy:
         if ((indicator & 076) != 6)   /* Only "tandem-base" and "wave-base" are allowed here. */
            fail("Can't find the indicated triangles.");

         tbonetest = s->people[1].id1 | s->people[3].id1 | s->people[5].id1 | s->people[7].id1;

         if ((tbonetest & 011) == 011)
            fail("Can't find the indicated triangles.");
         else if ((indicator ^ tbonetest) & 1)
            schema = (indicator & 0100) ? schema_intlk_lateral_6 : schema_lateral_6;
         else
            schema = (indicator & 0100) ? schema_intlk_vertical_6 : schema_vertical_6;
   
         /* For galaxies, the schema is now in terms of the absolute orientation. */
         /* We know that the original setup rotation was canonicalized. */
         break;
      case s_hrglass:
         if ((indicator & 076) != 6)   /* Only "tandem-base" and "wave-base" are allowed here. */
            fail("Can't find the indicated triangles.");

         tbonetest = s->people[0].id1 | s->people[1].id1 | s->people[4].id1 | s->people[5].id1;

         if ((tbonetest & 011) == 011 || ((indicator ^ tbonetest) & 1))
            fail("Can't find the indicated triangles.");
   
         schema = (indicator & 0100) ? schema_intlk_vertical_6 : schema_vertical_6;
         break;
      case s_c1phan:
         if ((indicator&63) == 20) {
            t = 0;
            if (global_selectmask == (global_livemask & 0x5A5A))
               t = 1;
            else if (global_selectmask != (global_livemask & 0xA5A5))
               fail("Can't find the indicated triangles.");
         }
         else {
            t = indicator & 1;
            if ((global_tbonetest & 010) == 0) t ^= 1;
            else if ((global_tbonetest & 1) != 0)
               fail("Can't find the indicated triangles.");
         }

         /* Now t is 0 to select the triangles whose bases are horizontally
            aligned, and 1 for the vertically aligned bases. */

         s->rotation += t;   /* Just flip the setup around and recanonicalize. */
         canonicalize_rotation(s);

         if ((global_livemask & 0xAAAA) == 0)
            map_ptr = (indicator & 0100) ? &map1i : &map1b;
         else if ((global_livemask & 0x5555) == 0)
            map_ptr = (indicator & 0100) ? &map2i : &map2b;
         else
            fail("Can't find the indicated triangles.");

         do_glorious_triangles(s, map_ptr, result);
         result->rotation -= t;   /* Flip the setup back. */
         reinstate_rotation(s, result);
         return;
      default:
         fail("Can't do this concept in this setup.");
   }

   concentric_move(s, &s->cmd, (setup_command *) 0, schema, 0, 0, TRUE, result);
}


extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   uint32 tbonetest;
   Const tgl_map *map_ptr;
   calldef_schema schema;
   int indicator = parseptr->concept->value.arg1;
   int indicator_base;

/* indicator = 0 - tall 6
               1 - short 6
               2 - inside
               3 - outside
               4 - out point
               5 - in point
               6 - wave-base
               7 - tandem-base
               20 - <anyone>-base
   Add 100 octal if interlocked triangles. */

   if (indicator >= 2 && indicator <= 21 && (ss->cmd.cmd_final_flags.herit & INHERITFLAG_INTLK)) {
      indicator |= 0100;     /* Interlocked triangles. */
      ss->cmd.cmd_final_flags.herit &= ~INHERITFLAG_INTLK;
   }

   if (ss->cmd.cmd_final_flags.herit | ss->cmd.cmd_final_flags.final)   /* Now demand that no flags remain. */
      fail("Illegal modifier for this concept.");

   indicator_base = indicator & 077;

   if ((indicator & 0100) && calling_level < intlk_triangle_level) {
      if (allowing_all_concepts)
         warn(warn__bad_concept_level);
      else
         fail("This concept modifier is not allowed at this level.");
   }

   if (indicator <= 1) {
      /* Indicator = 0 for tall 6, 1 for short 6 */

      if (ss->kind == s_galaxy) {
         /* We know the setup rotation is canonicalized. */
         tbonetest = ss->people[1].id1 | ss->people[3].id1 | ss->people[5].id1 | ss->people[7].id1;

         if ((tbonetest & 011) == 011) fail("Can't find tall/short 6.");
         else if ((indicator ^ tbonetest) & 1)
            schema = schema_lateral_6;
         else
            schema = schema_vertical_6;
      }
      else
         fail("Must have galaxy for this concept.");

      /* For galaxies, the schema is now in terms of the absolute orientation. */

      concentric_move(ss, &ss->cmd, (setup_command *) 0, schema, 0, 0, TRUE, result);
   }
   else {
      /* Set this so we can do "peel and trail" without saying "triangle" again. */
      ss->cmd.cmd_misc_flags |= CMD_MISC__SAID_TRIANGLE;

      if (indicator_base >= 6) {
         /* Indicator = 6 for wave-base, 7 for tandem-base, 20 for <anyone>-base. */
         wv_tand_base_move(ss, indicator, result);
      }
      else if (indicator_base >= 4) {
         /* Indicator = 5 for in point, 4 for out point */
   
         if (ss->kind != s_qtag) fail("Must have diamonds.");
   
         if (indicator_base == 5) {
            if (
                  (ss->people[0].id1 & d_mask) == d_east &&
                  (ss->people[1].id1 & d_mask) != d_west &&
                  (ss->people[4].id1 & d_mask) == d_west &&
                  (ss->people[5].id1 & d_mask) != d_east)
               map_ptr = (indicator & 0100) ? &map1j : &map1b;
            else if (
                  (ss->people[0].id1 & d_mask) != d_east &&
                  (ss->people[1].id1 & d_mask) == d_west &&
                  (ss->people[4].id1 & d_mask) != d_west &&
                  (ss->people[5].id1 & d_mask) == d_east)
               map_ptr = (indicator & 0100) ? &map2j : &map2b;
            else
               fail("Can't find designated point.");
         }
         else {
            if (
                  (ss->people[0].id1 & d_mask) == d_west &&
                  (ss->people[1].id1 & d_mask) != d_east &&
                  (ss->people[4].id1 & d_mask) == d_east &&
                  (ss->people[5].id1 & d_mask) != d_west)
               map_ptr = (indicator & 0100) ? &map1j : &map1b;
            else if (
                  (ss->people[0].id1 & d_mask) != d_west &&
                  (ss->people[1].id1 & d_mask) == d_east &&
                  (ss->people[4].id1 & d_mask) != d_east &&
                  (ss->people[5].id1 & d_mask) == d_west)
               map_ptr = (indicator & 0100) ? &map2j : &map2b;
            else
               fail("Can't find designated point.");
         }
   
         do_glorious_triangles(ss, map_ptr, result);
         reinstate_rotation(ss, result);
      }
      else {
         /* Indicator = 2 for inside, 3 for outside */
   
         /* This is the only case that allows interlocked. */

         if (indicator_base == 2 && ss->kind == sbigdmd) {
            if (global_livemask == 07474)
               map_ptr = (indicator & 0100) ? &map1k : &map1b;
            else if (global_livemask == 01717)
               map_ptr = (indicator & 0100) ? &map2k : &map2b;
            else
               fail("Can't find the indicated triangles.");

            do_glorious_triangles(ss, map_ptr, result);
            reinstate_rotation(ss, result);

            return;
         }
         else if (indicator & 0100)
            fail("Illegal modifier for this concept.");

         if (indicator_base == 2) {
            switch (ss->kind) {
               case s_hrglass:
                  schema = schema_vertical_6;   /* This is the schema for picking out
                                                   the triangles in an hourglass. */
                  break;
               case s_rigger:
               case s_ptpd:
                  schema = schema_concentric_6_2;
                  break;
               case s_qtag:
                  schema = schema_concentric_6_2_tgl;
                  break;
               case sbigdmd:
               default:
                  fail("There are no 'inside' triangles.");
            }
   
            concentric_move(ss, &ss->cmd, (setup_command *) 0, schema, 0, 0, TRUE, result);
         }
         else {
            switch (ss->kind) {
               case s_ptpd:
               case s_qtag:
               case s_spindle:
               case s_bone:
                  schema = schema_concentric_2_6;
                  break;
               default:
                  fail("There are no 'outside' triangles.");
            }
   
            concentric_move(ss, (setup_command *) 0, &ss->cmd, schema, 0, 0, TRUE, result);
         }
      }
   }
}
