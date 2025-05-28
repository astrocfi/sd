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
   phantom_2x4_move
*/

#include "sd.h"


/* This handles only the T-boned case.  Other cases are handled elsewhere. */

extern void phantom_2x4_move(
   setup *ss,
   int lineflag,
   phantest_kind phantest,
   map_thing *maps,
   parse_block *parseptr,
   setup *result)

{
   setup a1v, a2v, a1h, a2h;
   setup b1v, b2v, b1h, b2h;
   setup hpeople, vpeople;
   int i, r, m1, m2;
   int v1flag, v2flag, h1flag, h2flag;
   int vflag, hflag;
   veryshort *map_ptr1, *map_ptr2;
   int finalsetupflags;

   warn(warn__tbonephantom);

   vflag = 0;
   hflag = 0;
   hpeople = *ss;
   vpeople = *ss;
   clear_people(&hpeople);
   clear_people(&vpeople);
   
   for (i=0; i<16; i++) {
      int temp = ss->people[i].id1;
      if ((temp ^ lineflag) & 1) {
         (void) copy_rot(&hpeople, (i+4) & 017, ss, i, 011);
         hflag |= temp;
      }
      else {
         (void) copy_person(&vpeople, i, ss, i);
         vflag |= temp;
      }
   }

   v1flag = 0;
   v2flag = 0;
   h1flag = 0;
   h2flag = 0;

   clear_people(result);
   finalsetupflags = 0;

   map_ptr1 = maps->map1;
   map_ptr2 = maps->map2;

   for (i=0; i<8; i++) {
      m1 = map_ptr2[i];
      m2 = map_ptr1[i];
      v1flag |= copy_rot(&a1v, i, &vpeople, m1, 033);
      v2flag |= copy_rot(&a2v, i, &vpeople, m2, 033);
      h1flag |= copy_rot(&a1h, i, &hpeople, m1, 033);
      h2flag |= copy_rot(&a2h, i, &hpeople, m2, 033);
   }
   
   /* ***** this next stuff doesn't really do the right thing.  If people are T-boned
      but no one sees a violation of bigblock among those with the same orientation
      as himself, it won't complain, even though the total setup violates bigblock. */

   if ((vflag && !(v1flag && v2flag)) || (hflag && !(h1flag && h2flag))) {
      /* Only one of the two setups is occupied. */
      switch (phantest) {
         /* **** bug: If T-boned, this message comes out if either orientation of people fails
            to occupy both of their setups.  But it could easily happen that the "phantom lines"
            concept is superfluous for the north-south people but completely meaningful for the
            east-west people. */
         case phantest_both:
            fail("Don't use phantom concept if you don't mean it.");
         case phantest_impossible:
            fail("This is impossible in a symmetric setup!!!!");
         case phantest_first_or_both:
            if (v2flag | h2flag) fail("Don't use phantom concept if you don't mean it.");
            break;
         case phantest_only_first_one:
            if (v2flag | h2flag) fail("Not in correct setup.");
            break;
         case phantest_only_second_one:
            if (v1flag | h1flag) fail("Not in correct setup.");
            break;
      }
   }
   else {
      /* Both of the two setups are occupied. */
      switch (phantest) {
         case phantest_only_one:
         case phantest_only_first_one:
         case phantest_only_second_one:
            fail("Can't find the setup to work in.");
      }
   }
   
   if (v1flag) {
      a1v.kind = s2x4;
      a1v.rotation = 0;
      a1v.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a1v, parseptr, NULLCALLSPEC, 0, FALSE, &b1v);
      finalsetupflags |= b1v.setupflags;
   }
   else
      b1v.kind = nothing;
   
   if (v2flag) {
      a2v.kind = s2x4;
      a2v.rotation = 0;
      a2v.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a2v, parseptr, NULLCALLSPEC, 0, FALSE, &b2v);
      finalsetupflags |= b2v.setupflags;
   }
   else
      b2v.kind = nothing;
   
   if (h1flag) {
      a1h.kind = s2x4;
      a1h.rotation = 0;
      a1h.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a1h, parseptr, NULLCALLSPEC, 0, FALSE, &b1h);
      finalsetupflags |= b1h.setupflags;
   }
   else
      b1h.kind = nothing;
   
   if (h2flag) {
      a2h.kind = s2x4;
      a2h.rotation = 0;
      a2h.setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
      move(&a2h, parseptr, NULLCALLSPEC, 0, FALSE, &b2h);
      finalsetupflags |= b2h.setupflags;
   }
   else
      b2h.kind = nothing;

   
   if (b1v.kind != nothing) {
      if (b1v.kind != s2x4) fail("Can't do this call with phantom C/L/W concept.");
      r = b1v.rotation & 3;
      if (r & 1) fail("Can't do shape changer with phantom C/L/W concept.");
      else if (r == 0)
         for (i=0; i<8; i++) install_rot(result, map_ptr2[i], &b1v, i, 011);
      else
         for (i=0; i<8; i++) install_rot(result, map_ptr2[i], &b1v, (i+4) & 7, 033);
   }
   
   if (b2v.kind != nothing) {
      if (b2v.kind != s2x4) fail("Can't do this call with phantom C/L/W concept.");
      r = b2v.rotation & 3;
      if (r & 1) fail("Can't do shape changer with phantom C/L/W concept.");
      else if (r == 0)
         for (i=0; i<8; i++) install_rot(result, map_ptr1[i], &b2v, i, 011);
      else
         for (i=0; i<8; i++) install_rot(result, map_ptr1[i], &b2v, (i+4) & 7, 033);
   }
   
   if (b1h.kind != nothing) {
      if (b1h.kind != s2x4) fail("Can't do this call with phantom C/L/W concept.");
      r = b1h.rotation & 3;
      if (r & 1) fail("Can't do shape changer with phantom C/L/W concept.");
      else if (r == 0)
         for (i=0; i<8; i++) install_person(result, (map_ptr2[i]+12) & 017, &b1h, i);
      else
         for (i=0; i<8; i++) install_rot(result, (map_ptr2[i]+12) & 017, &b1h, (i+4) & 7, 022);
   }
   
   if (b2h.kind != nothing) {
      if (b2h.kind != s2x4) fail("Can't do this call with phantom C/L/W concept.");
      r = b2h.rotation & 3;
      if (r & 1) fail("Can't do shape changer with phantom C/L/W concept.");
      else if (r == 0)
         for (i=0; i<8; i++) install_person(result, (map_ptr1[i]+12) & 017, &b2h, i);
      else
         for (i=0; i<8; i++) install_rot(result, (map_ptr1[i]+12) & 017, &b2h, (i+4) & 7, 022);
   }
   
   result->setupflags = finalsetupflags;
   result->kind = s4x4;
   result->rotation = 0;
   reinstate_rotation(ss, result);
}
