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
   triangle_move
*/

#include "sd.h"


Private void ctr_end_triangle_move(
   setup *s,
   int indicator,
   setup *result)
{
   calldef_schema schema;

   if (indicator == 2) {
      switch (s->kind) {
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
         default:
            fail("There are no 'inside' triangles.");
      }

      concentric_move(s, &s->cmd, (setup_command *) 0, schema, 0, 0, result);
   }
   else {
      switch (s->kind) {
         case s_ptpd:
         case s_qtag:
         case s_spindle:
         case s_bone:
            schema = schema_concentric_2_6;
            break;
         default:
            fail("There are no 'outside' triangles.");
      }

      concentric_move(s, (setup_command *) 0, &s->cmd, schema, 0, 0, result);
   }
}


typedef struct qwerty {
   int mapqt1[3];   /* First triangle (upright) in quarter-tag. */
   int mapqt2[3];   /* Second triangle (inverted) in quarter-tag. */
   int mapqti[2];   /* Idle people in quarter-tag. */
   int mapcp1[3];   /* First triangle (inverted) in C1 phantom. */
   int mapcp2[3];   /* Second triangle (upright) in C1 phantom. */
   int mapcpi[2];   /* Idle people in C1 phantom. */
   int map241[3];   /* First triangle (inverted) in 2x4. */
   int map242[3];   /* Second triangle (upright) in 2x4. */
   int map24i[2];   /* Idle people in 2x4. */
   setup_kind kind;
   struct qwerty *other;
   } tgl_map;

/* These need to be predeclared so that they can refer to each other. */
Private tgl_map map1b;
Private tgl_map map2b;

/*                 mapqt1     mapqt2   mapqti    mapcp1        mapcp2      mapcpi    map241     map242   map24i   kind     other */
Private tgl_map map1b = {{4, 3, 2}, {0, 7, 6}, {1, 5}, {6,  8, 10}, {14,  0,  2}, {4, 12}, {6, 5, 4}, {2, 1, 0}, {3, 7}, s_c1phan, &map2b};
Private tgl_map map2b = {{5, 6, 7}, {1, 2, 3}, {0, 4}, {3, 15, 13}, {11,  7,  5}, {1,  9}, {7, 6, 5}, {3, 2, 1}, {0, 4}, s_qtag,   &map1b};


Private void do_glorious_triangles(
   setup *ss,
   setup *a1,
   setup *a2,
   setup *idle,
   int startingrot,
   tgl_map *map_ptr,
   setup *result)
{
   int i, r;
   setup res1, res2;

   a1->cmd = ss->cmd;
   a2->cmd = ss->cmd;
   a1->kind = s_trngl;
   a2->kind = s_trngl;
   a1->rotation = startingrot;
   a2->rotation = startingrot;
   a1->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   a2->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

   move(a1, FALSE, &res1);
   move(a2, FALSE, &res2);

   if (res1.kind != res2.kind || res1.rotation != res2.rotation)
      fail("Improper result from triangle call.");

   res1.rotation &= 3;     /* Just making sure. */

   result->rotation = res1.rotation;

   r = ((-res1.rotation) & 3) * 011;

   if (res1.kind == s_trngl) {
      if (res1.rotation & 2)
         result->kind = map_ptr->other->kind;
      else
         result->kind = map_ptr->kind;
   
      if (res1.rotation == 0) {
         result->kind = s_qtag;
         /* Restore the two people who don't move. */
         (void) copy_person(result, map_ptr->mapqti[0], idle, 0);
         (void) copy_person(result, map_ptr->mapqti[1], idle, 1);
   
         for (i=0; i<3; i++) {
            (void) copy_person(result, map_ptr->mapqt1[i], &res1, i);
            (void) copy_rot(result, map_ptr->mapqt2[i], &res2, i, 022);
         }
      }
      else if (res1.rotation == 2) {
         result->kind = s_c1phan;
         /* Restore the two people who don't move. */
         (void) copy_rot(result, map_ptr->mapcpi[1], idle, 0, r);
         (void) copy_rot(result, map_ptr->mapcpi[0], idle, 1, r);
         
         for (i=0; i<3; i++) {
            (void) copy_person(result, map_ptr->mapcp2[i], &res1, i);
            (void) copy_rot(result, map_ptr->mapcp1[i], &res2, i, 022);
         }
      }
      else {
         if (result->kind == s_c1phan) {
             /* Restore the two people who don't move. */
             (void) copy_rot(result, map_ptr->other->mapcpi[0], idle, 0, r);
             (void) copy_rot(result, map_ptr->other->mapcpi[1], idle, 1, r);
   
             /* Copy the triangles. */
             for (i=0; i<3; i++) {
                (void) copy_person(result, map_ptr->other->mapcp2[i], &res1, i);
                (void) copy_rot(result, map_ptr->other->mapcp1[i], &res2, i, 022);
             }
         }
         else {
             /* Restore the two people who don't move. */
             (void) copy_rot(result, map_ptr->other->mapqti[1], idle, 0, r);
             (void) copy_rot(result, map_ptr->other->mapqti[0], idle, 1, r);
   
             /* Copy the triangles. */
             for (i=0; i<3; i++) {
                (void) copy_person(result, map_ptr->other->mapqt1[i], &res1, i);
                (void) copy_rot(result, map_ptr->other->mapqt2[i], &res2, i, 022);
             }
         }
      }
   }
   else if (res1.kind == s1x3) {
      result->kind = s2x4;

      if (res1.rotation == 0) {
         (void) copy_person(result, map_ptr->map24i[0], idle, 0);
         (void) copy_person(result, map_ptr->map24i[1], idle, 1);
         for (i=0; i<3; i++) {
            (void) copy_person(result, map_ptr->map241[i], &res1, i);
            (void) copy_rot(result, map_ptr->map242[i], &res2, i, 022);
         }
      }
      else {
         if (map_ptr->kind == s_c1phan) {    /* What a crock! */
            (void) copy_rot(result, map_ptr->other->map24i[0], idle, 0, r);
            (void) copy_rot(result, map_ptr->other->map24i[1], idle, 1, r);
            for (i=0; i<3; i++) {
               (void) copy_person(result, map_ptr->other->map242[i], &res1, 2-i);
               (void) copy_rot(result, map_ptr->other->map241[i], &res2, 2-i, 022);
            }
         }
         else {
            (void) copy_rot(result, map_ptr->other->map24i[1], idle, 0, r);
            (void) copy_rot(result, map_ptr->other->map24i[0], idle, 1, r);
            for (i=0; i<3; i++) {
               (void) copy_person(result, map_ptr->other->map241[i], &res1, i);
               (void) copy_rot(result, map_ptr->other->map242[i], &res2, i, 022);
            }
         }
      }
   }
   else
      fail("Improper result from triangle call.");

   if (result->kind == s2x4)
       warn(warn__check_2x4);
   else if (res1.rotation != startingrot) {
      if (result->kind == s_c1phan)
          warn(warn__check_c1_phan);
      else
          warn(warn__check_dmd_qtag);
   }

   result->result_flags = res1.result_flags | res2.result_flags;
}




/* This procedure does wave-base, tandem-base, and so-and-so-base. */
Private void wv_tand_base_move(
   setup *s,
   int indicator,
   setup *result)
{
   int i, t, tbonetest;
   calldef_schema schema;
   setup a1, a2;
   setup idle;
   tgl_map *map_ptr;

   switch (s->kind) {
      case s_galaxy:
         /* We allow so-and-so-base triangles only in C1 phantoms. */
         if (indicator == 20)
            fail("Can't find the indicated triangles.");
         tbonetest = s->people[1].id1 | s->people[3].id1 | s->people[5].id1 | s->people[7].id1;
         if ((tbonetest & 011) == 011)
            fail("Can't find the indicated triangles.");
         else if ((indicator ^ tbonetest) & 1)
            schema = schema_lateral_6;
         else
            schema = schema_vertical_6;
   
         /* For galaxies, the schema is now in terms of the absolute orientation. */
         /* We know that the original setup rotation was canonicalized. */
         break;
      case s_hrglass:
         /* We allow so-and-so-base triangles only in C1 phantoms. */
         if (indicator == 20)
            fail("Can't find the indicated triangles.");
         tbonetest = s->people[0].id1 | s->people[1].id1 | s->people[4].id1 | s->people[5].id1;
         if ((tbonetest & 011) == 011 || ((indicator ^ tbonetest) & 1))
            fail("Can't find the indicated triangles.");
   
         schema = schema_vertical_6;   /* This is the schema for picking out
                                          the triangles in an hourglass. */
         break;
      case s_c1phan:
         if (indicator == 20) {
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

         if ((global_livemask & 0xAAAA) == 0) {
            map_ptr = &map1b;
         }
         else if ((global_livemask & 0x5555) == 0) {
            map_ptr = &map2b;
         }
         else
            fail("Can't find the indicated triangles.");

         for (i=0; i<3; i++) {
            (void) copy_rot(&a1, i, s, map_ptr->mapcp1[i], 022);
            (void) copy_person(&a2, i, s, map_ptr->mapcp2[i]);
         }
      
         /* Save the two people who don't move. */
         (void) copy_person(&idle, 0, s, map_ptr->mapcpi[0]);
         (void) copy_person(&idle, 1, s, map_ptr->mapcpi[1]);
      
         do_glorious_triangles(s, &a1, &a2, &idle, 2, map_ptr, result);
         result->rotation -= t;   /* Flip the setup back. */
         reinstate_rotation(s, result);
         return;
      default:
         fail("Can't do this concept in this setup.");
   }

   concentric_move(s, &s->cmd, (setup_command *) 0, schema, 0, 0, result);
}


Private void tall_short_6_move(
   setup *s,
   int indicator,
   setup *result)
{
   int tbonetest;
   calldef_schema schema;

   if (s->kind == s_galaxy) {
      /* We know the setup rotation is canonicalized. */
      tbonetest = s->people[1].id1 | s->people[3].id1 | s->people[5].id1 | s->people[7].id1;

      if ((tbonetest & 011) == 011) fail("Can't find tall/short 6.");
      else if ((indicator ^ tbonetest) & 1)
         schema = schema_lateral_6;
      else
         schema = schema_vertical_6;
   }
   else
      fail("Must have galaxy for this concept.");

   /* For galaxies, the schema is now in terms of the absolute orientation. */

   concentric_move(s, &s->cmd, (setup_command *) 0, schema, 0, 0, result);
}


extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   int i;
   setup a1, a2;
   setup idle;
   tgl_map *map_ptr;
   int indicator = parseptr->concept->value.arg1;

/* indicator = 0 - out point
               1 - in point
               2 - inside
               3 - outside
               4 - tall 6
               5 - short 6
               6 - wave-base
               7 - tandem-base
               20 - so-and-so-base */

   if (indicator >= 6) {
      /* Set this so we can do "peel and trail" without saying "triangle" again. */
      ss->cmd.cmd_misc_flags |= CMD_MISC__SAID_TRIANGLE;
      wv_tand_base_move(ss, indicator, result);
      return;
   }
   else if (indicator >= 4) {
      tall_short_6_move(ss, indicator, result);
      return;
   }
   else if (indicator >= 2) {
      /* Set this so we can do "peel and trail" without saying "triangle" again. */
      ss->cmd.cmd_misc_flags |= CMD_MISC__SAID_TRIANGLE;
      ctr_end_triangle_move(ss, indicator, result);
      return;
   }

   /* arg1 = 1 for in point, 0 for out point */

   /* Set this so we can do "peel and trail" without saying "triangle" again. */
   ss->cmd.cmd_misc_flags |= CMD_MISC__SAID_TRIANGLE;

   if (ss->kind != s_qtag) fail("Must have diamonds.");

   if (indicator) {
      if (
            (ss->people[0].id1 & d_mask) == d_east &&
            (ss->people[1].id1 & d_mask) != d_west &&
            (ss->people[4].id1 & d_mask) == d_west &&
            (ss->people[5].id1 & d_mask) != d_east)
         map_ptr = &map1b;
      else if (
            (ss->people[0].id1 & d_mask) != d_east &&
            (ss->people[1].id1 & d_mask) == d_west &&
            (ss->people[4].id1 & d_mask) != d_west &&
            (ss->people[5].id1 & d_mask) == d_east)
         map_ptr = &map2b;
      else
         fail("Can't find designated point.");
   }
   else {
      if (
            (ss->people[0].id1 & d_mask) == d_west &&
            (ss->people[1].id1 & d_mask) != d_east &&
            (ss->people[4].id1 & d_mask) == d_east &&
            (ss->people[5].id1 & d_mask) != d_west)
         map_ptr = &map1b;
      else if (
            (ss->people[0].id1 & d_mask) != d_west &&
            (ss->people[1].id1 & d_mask) == d_east &&
            (ss->people[4].id1 & d_mask) != d_east &&
            (ss->people[5].id1 & d_mask) == d_west)
         map_ptr = &map2b;
      else
         fail("Can't find designated point.");
   }

   for (i=0; i<3; i++) {
      (void) copy_person(&a1, i, ss, map_ptr->mapqt1[i]);
      (void) copy_rot(&a2, i, ss, map_ptr->mapqt2[i], 022);
   }

   /* Save the two people who don't move. */
   (void) copy_person(&idle, 0, ss, map_ptr->mapqti[0]);
   (void) copy_person(&idle, 1, ss, map_ptr->mapqti[1]);

   do_glorious_triangles(ss, &a1, &a2, &idle, 0, map_ptr, result);
   reinstate_rotation(ss, result);
}
