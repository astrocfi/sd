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
   setup resv;
   setup hpeople, vpeople;
   int i, r;
   int vflag, hflag;
   int finalsetupflags = 0;
   phantest_kind newphantest = phantest;

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
         (void) copy_person(&hpeople, i, ss, i);
         hflag |= temp;
      }
      else {
         (void) copy_person(&vpeople, i, ss, i);
         vflag |= temp;
      }
   }

   /* For certain types of phantom tests, we turn off the testing that we pass to
      divided_setup_move.  It could give bogus errors, because it only sees the
      headliners or sideliners at any one time. */
   switch (phantest) {
      case phantest_both:           /* This occurs on "phantom bigblock lines", for example,
                                       and is intended to give error "Don't use phantom concept
                                       if you don't mean it." if everyone is in one variety of
                                       bigblock setup.  But, in this procedure, we could have
                                       headliners entirely in one and sideliners in another, so
                                       that divided_setup_move could be misled. */
      case phantest_first_or_both:  /* This occurs on "phantom lines", for example, and is
                                       intended to give the smae error as above if only the center
                                       phantom lines are occupied.  But the headliners might
                                       occupy just the center phantom lines while the sideliners
                                       make full use of the concept, so, once again, we have to
                                       disable it. */
         newphantest = phantest_ok;
         break;
   }

   /* Do the E-W facing people. */

   if (vflag) {
      vpeople.rotation--;
      canonicalize_rotation(&vpeople);
      divided_setup_move(&vpeople, parseptr, NULLCALLSPEC, 0, maps, newphantest, 0, &resv);
      finalsetupflags |= resv.setupflags;
      resv.rotation++;
      canonicalize_rotation(&resv);
   }
   else
      resv.kind = nothing;

   /* Do the N-S facing people. */

   if (hflag) {
      divided_setup_move(&hpeople, parseptr, NULLCALLSPEC, 0, maps, newphantest, 0, result);
      finalsetupflags |= result->setupflags;
   }
   else
      result->kind = nothing;
  
   merge_setups(&resv, merge_strict_matrix, result);
   result->setupflags = finalsetupflags;
   reinstate_rotation(ss, result);
}
