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

    This is for version 27. */

/* This defines the following function:
   selectp

and the following external variables:
   current_selector
   selector_used
   pred_table     which is filled with pointers to the predicate functions
*/

#include "sd.h"

/* These variables are external. */

selector_kind current_selector;
long_boolean selector_used;


/* If a real person and a person under test are XOR'ed, the result AND'ed with this constant,
   and the low bits of that result examined, it will tell whether the person under test is real
   and facing in the same direction (result = 0), the opposite direction (result == 2), or whatever. */

#define DIR_MASK (BIT_PERSON | 3)

#define ID2_ALL_ID (ID2_HEAD|ID2_SIDE|ID2_BOY|ID2_GIRL)





extern long_boolean selectp(setup *ss, int place)
{
   int pid2;

   selector_used = TRUE;

   /* Pull out the cases that do not require the person to be real. */

   switch (current_selector) {
      case selector_all:
         return(TRUE);
      case selector_none:
         return(FALSE);
   }

   /* Demand that the subject be real. */

   if (!(ss->people[place].id1 & BIT_PERSON))
      fail("Can't decide who are selected.");

   pid2 = ss->people[place].id2;

   switch (current_selector) {
      case selector_boys:
         if      ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_BOY) return(TRUE);
         else if ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_GIRL) return(FALSE);
         break;
      case selector_girls:
         if      ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_GIRL) return(TRUE);
         else if ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_BOY) return(FALSE);
         break;
      case selector_heads:
         if      ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_HEAD) return(TRUE);
         else if ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_SIDE) return(FALSE);
         break;
      case selector_sides:
         if      ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_SIDE) return(TRUE);
         else if ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_HEAD) return(FALSE);
         break;
      case selector_headcorners:
         if      ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_HCOR) return(TRUE);
         else if ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_SCOR) return(FALSE);
         break;
      case selector_sidecorners:
         if      ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_SCOR) return(TRUE);
         else if ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_HCOR) return(FALSE);
         break;
      case selector_headboys:
         if      ((pid2 & (ID2_ALL_ID|ID2_NHB)) == (ID2_HEAD|ID2_BOY)) return(TRUE);
         else if ((pid2 & ID2_NHB) == ID2_NHB) return(FALSE);
         break;
      case selector_headgirls:
         if      ((pid2 & (ID2_ALL_ID|ID2_NHG)) == (ID2_HEAD|ID2_GIRL)) return(TRUE);
         else if ((pid2 & ID2_NHG) == ID2_NHG) return(FALSE);
         break;
      case selector_sideboys:
         if      ((pid2 & (ID2_ALL_ID|ID2_NSB)) == (ID2_SIDE|ID2_BOY)) return(TRUE);
         else if ((pid2 & ID2_NSB) == ID2_NSB) return(FALSE);
         break;
      case selector_sidegirls:
         if      ((pid2 & (ID2_ALL_ID|ID2_NSG)) == (ID2_SIDE|ID2_GIRL)) return(TRUE);
         else if ((pid2 & ID2_NSG) == ID2_NSG) return(FALSE);
         break;
      case selector_centers:
         if      ((pid2 & (ID2_CENTER|ID2_END)) == ID2_CENTER) return(TRUE);
         else if ((pid2 & (ID2_CENTER|ID2_END)) == ID2_END) return(FALSE);
         break;
      case selector_ends:
         if      ((pid2 & (ID2_CENTER|ID2_END)) == ID2_END) return(TRUE);
         else if ((pid2 & (ID2_CENTER|ID2_END)) == ID2_CENTER) return(FALSE);
         break;
      case selector_leads:
         if      ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_LEAD) return(TRUE);
         else if ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_TRAILER) return(FALSE);
         break;
      case selector_trailers:
         if      ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_TRAILER) return(TRUE);
         else if ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_LEAD) return(FALSE);
         break;
      case selector_beaux:
         if      ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BEAU) return(TRUE);
         else if ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BELLE) return(FALSE);
         break;
      case selector_belles:
         if      ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BELLE) return(TRUE);
         else if ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BEAU) return(FALSE);
         break;
      case selector_center2:
         if      ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_CTR2) return(TRUE);
         else if ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_OUTR6) return(FALSE);
         break;
      case selector_center6:
         if      ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_CTR6) return(TRUE);
         else if ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_OUTR2) return(FALSE);
         break;
      case selector_outer2:
         if      ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_OUTR2) return(TRUE);
         else if ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_CTR6) return(FALSE);
         break;
      case selector_outer6:
         if      ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_OUTR6) return(TRUE);
         else if ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_CTR2) return(FALSE);
         break;
      case selector_nearline:
         if      ((pid2 & (ID2_NEARLINE|ID2_FARLINE)) == ID2_NEARLINE) return(TRUE);
         else if ((pid2 & (ID2_NEARLINE|ID2_FARLINE)) == ID2_FARLINE) return(FALSE);
         break;
      case selector_farline:
         if      ((pid2 & (ID2_NEARLINE|ID2_FARLINE)) == ID2_FARLINE) return(TRUE);
         else if ((pid2 & (ID2_NEARLINE|ID2_FARLINE)) == ID2_NEARLINE) return(FALSE);
         break;
      case selector_nearcolumn:
         if      ((pid2 & (ID2_NEARCOL|ID2_FARCOL)) == ID2_NEARCOL) return(TRUE);
         else if ((pid2 & (ID2_NEARCOL|ID2_FARCOL)) == ID2_FARCOL) return(FALSE);
         break;
      case selector_farcolumn:
         if      ((pid2 & (ID2_NEARCOL|ID2_FARCOL)) == ID2_FARCOL) return(TRUE);
         else if ((pid2 & (ID2_NEARCOL|ID2_FARCOL)) == ID2_NEARCOL) return(FALSE);
         break;
      case selector_nearbox:
         if      ((pid2 & (ID2_NEARBOX|ID2_FARBOX)) == ID2_NEARBOX) return(TRUE);
         else if ((pid2 & (ID2_NEARBOX|ID2_FARBOX)) == ID2_FARBOX) return(FALSE);
         break;
      case selector_farbox:
         if      ((pid2 & (ID2_NEARBOX|ID2_FARBOX)) == ID2_FARBOX) return(TRUE);
         else if ((pid2 & (ID2_NEARBOX|ID2_FARBOX)) == ID2_NEARBOX) return(FALSE);
         break;
      default:
         fail("ERROR - selector failed to get initialized.");
   }
   
   fail("Can't decide who are selected.");
   /* NOTREACHED */
}



/* Here are the predicates.  They will get put into the array "pred_table". */

/* ARGSUSED */
static long_boolean selected(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(selectp(real_people, real_index));
}

/* ARGSUSED */
static long_boolean unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(!selectp(real_people, real_index));
}

/* ARGSUSED */
static long_boolean select_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(selectp(real_people, real_index) && selectp(real_people, real_index ^ 1));
}

/* ARGSUSED */
static long_boolean select_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(selectp(real_people, real_index) && !selectp(real_people, real_index ^ 1));
}

/* ARGSUSED */
static long_boolean unselect_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(!selectp(real_people, real_index) && selectp(real_people, real_index ^ 1));
}

/* ARGSUSED */
static long_boolean unselect_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(!(selectp(real_people, real_index) || selectp(real_people, real_index ^ 1)));
}

/* ARGSUSED */
static long_boolean always(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(TRUE);
}

/* ARGSUSED */
static long_boolean x22_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_index = real_index ^ (((real_direction << 1) & 2) ^ 1);
   int other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean x22_couple(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_index = real_index ^ (((real_direction << 1) & 2) ^ 1);
   int other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
static long_boolean x22_facing_someone(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_index = real_index ^ (((real_direction << 1) & 2) ^ 3);
   int other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean x14_once_rem_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 3].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean x14_once_rem_couple(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 3].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
static long_boolean same_in_quad(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 1].id1;
   if (real_people->kind == s_1x6 && real_index >= 2)
      other_person = real_people->people[7 - real_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
static long_boolean opp_in_quad(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 1].id1;
   if (real_people->kind == s_1x6 && real_index >= 2)
      other_person = real_people->people[7 - real_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean same_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 7].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
static long_boolean opp_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 7].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean opp_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 6].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean same_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 6].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
static long_boolean lines_once_rem_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 2].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
static long_boolean lines_once_rem_couple(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 2].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* Next test is used for figuring out how to hinge. */
/* Legal from 1x2. */
/* Returns TRUE if this person is beau, or else this belle is in a miniwave with
   the other person.  Returns FALSE if this belle is in a couple with the
   other person.  Complains if this belle can't tell. */
/* ARGSUSED */
static long_boolean x12_beau_or_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (northified_index == 0)
      return(TRUE);
   else {
      int other_person = real_people->people[real_index ^ 1].id1;
      if (!other_person)
         fail("Need a real person to work with."); 
      return (((other_person & 2) >> 1) == real_index);
   }
}

/* Test for wheel and deal to be done 2FL-style, or beau side of 1FL.  Returns
   false if belle side of 1FL.  Raises an error if wheel and deal can't be done. */
/* ARGSUSED */
static long_boolean x14_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* We assume people have already been checked for coupleness. */

   if (northified_index <= 1)
      /* We are in the beau-side couple -- it's OK. */
      return(TRUE);
   else {
      /* We are in the belle-side couple -- find the two people in the other couple.
         Just "or" them to be sure we get someone.  They are already known
         to be facing consistently if they are both there. */
      int other_people = real_people->people[real_index ^ 2].id1 |
                           real_people->people[real_index ^ 3].id1;

      /* At least one of those people must exist. */
      if (!other_people)
         fail("Can't tell how to do this -- no live people.");

      /* See if they face the same way as myself.  Note that the "2" bit of
         real_index is the complement of my own direction bit. */
      if (((other_people ^ real_index) & 2))
         return(FALSE);   /* This is a 1FL. */

      return(TRUE);       /* This is a 2FL. */
   }
}


/* Test for 3X3 wheel_and_deal to be done 2FL-style, or beau side of 1FL. */
/* ARGSUSED */
static long_boolean x16_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* We assume people have already been checked for coupleness. */

   if (northified_index <= 2)
      /* We are in the beau-side triad -- it's OK. */
      return(TRUE);
   else {
      /* We are in the belle-side triad -- find the three people in the other triad.
         Just "or" them to be sure we get someone.  They are already known
         to be facing consistently if they are all there. */
      int other_side = real_index <= 2 ? 3 : 0;
      int other_people = real_people->people[other_side].id1 |
                           real_people->people[other_side+1].id1 |
                           real_people->people[other_side+2].id1;

      /* At least one of those people must exist. */
      if (!other_people)
         fail("Can't tell how to do this -- no live people.");

      /* See if they face the same way as myself. */
      if (((other_people ^ real_people->people[real_index].id1) & 2))
         return(TRUE);    /* This is a 2FL. */

      return(FALSE);      /* This is a 1FL. */
   }
}


/* Test for 4X4 wheel_and_deal to be done 2FL-style, or beau side of 1FL. */
/* ARGSUSED */
static long_boolean x18_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* We assume people have already been checked for coupleness. */

   if (northified_index <= 3)
      /* We are in the beau-side quad -- it's OK. */
      return(TRUE);
   else {
      /* We are in the belle-side quad -- find the three people in the other quad.
         Just "or" them to be sure we get someone.  They are already known
         to be facing consistently if they are all there. */
      int other_side = (real_index&4) ^ 4;
      int other_people = real_people->people[other_side].id1 |
                           real_people->people[other_side+1].id1 |
                           real_people->people[other_side+2].id1 |
                           real_people->people[other_side+3].id1;

      /* At least one of those people must exist. */
      if (!other_people)
         fail("Can't tell how to do this -- no live people.");

      /* See if they face the same way as myself. */
      if (((other_people ^ real_people->people[real_index].id1) & 2))
         return(TRUE);    /* This is a 2FL. */

      return(FALSE);      /* This is a 1FL. */
   }
}



static long_boolean vert1(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) | 1)].id1;
   if (northified_index & 1)
      return(((this_person ^ other_person) & DIR_MASK) == 0);
   else
      return(TRUE);
}

static long_boolean vert2(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) | 1)].id1;
   if (northified_index & 1)
      return(((this_person ^ other_person) & DIR_MASK) == 2);
   else
      return(FALSE);
}

/* ARGSUSED */
static long_boolean inner_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if ((real_index+3) & 2)
      return(northified_index >= 4);     /* I am an end */
   else
      return(                            /* I am a center */
         (012 - ((real_index & 4) >> 1)) == (real_people->people[real_index ^ 1].id1 & 017));
}

/* ARGSUSED */
static long_boolean outer_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if ((real_index+3) & 2)
      return(northified_index < 4);      /* I am an end */
   else
      return(                            /* I am a center */
         (010 + ((real_index & 4) >> 1)) == (real_people->people[real_index ^ 1].id1 & 017));
}

/* ARGSUSED */
static long_boolean judge_is_right(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;
   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 0)       /* judge exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 2));         /* we do not have another judge to my left */
}

/* ARGSUSED */
static long_boolean judge_is_left(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;
   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 2)           /* judge exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 0));     /* we do not have another judge to my right */
}

/* ARGSUSED */
static long_boolean socker_is_right(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;
   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 2)       /* socker exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 0));         /* we do not have another socker to my left */
}

/* ARGSUSED */
static long_boolean socker_is_left(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;
   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 0)           /* socker exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 2));     /* we do not have another socker to my right */
}

/* ARGSUSED */
static long_boolean inroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int inroll_direction;

   inroll_direction = 012 - ((real_index & 4) >> 1);
   if (  /* cw_end exists and is inrolling */
         ((real_people->people[real_index | 3].id1 & 017) == inroll_direction)
      &&
         /* we do not have ccw_end inrolling also */
         ((real_people->people[real_index & 4].id1 & 017) != inroll_direction))
      return(TRUE);
   else if (
         /* ccw_end exists and is inrolling */
         ((real_people->people[real_index & 4].id1 & 017) == inroll_direction)
      &&
         /* we do not have cw_end inrolling also */
         ((real_people->people[real_index | 3].id1 & 017) != inroll_direction))
      return(FALSE);
   else {
      fail("Can't find end looking in.");
      /* NOTREACHED */
  }
}

/* ARGSUSED */
static long_boolean magic_inroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   static int cw[8] = {3, 7, 7, 3, 7, 3, 3, 7};
   static int cc[8] = {0, 4, 4, 0, 4, 0, 0, 4};
   static int id[8] = {012, 010, 010, 012, 010, 012, 012, 010};
   int inroll_direction;

   inroll_direction = id[real_index];
   if (  /* cw_end exists and is inrolling */
         ((real_people->people[cw[real_index]].id1 & 017) == inroll_direction)
      &&
         /* we do not have ccw_end inrolling also */
         ((real_people->people[cc[real_index]].id1 & 017) != inroll_direction))
      return(TRUE);
   else if (
         /* ccw_end exists and is inrolling */
         ((real_people->people[cc[real_index]].id1 & 017) == inroll_direction)
      &&
         /* we do not have cw_end inrolling also */
         ((real_people->people[cw[real_index]].id1 & 017) != inroll_direction))
      return(FALSE);
   else {
      fail("Can't find end looking in.");
      /* NOTREACHED */
   }
}

/* ARGSUSED */
static long_boolean outroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int outroll_direction;

   outroll_direction = 010 + ((real_index & 4) >> 1);
   if (  /* cw_end exists and is outrolling */
         ((real_people->people[real_index | 3].id1 & 017) == outroll_direction)
      &&
         /* we do not have ccw_end outrolling also */
         ((real_people->people[real_index & 4].id1 & 017) != outroll_direction))
      return(TRUE);
   else if (
         /* ccw_end exists and is outrolling */
         ((real_people->people[real_index & 4].id1 & 017) == outroll_direction)
      &&
         /* we do not have cw_end outrolling also */
         ((real_people->people[real_index | 3].id1 & 017) != outroll_direction))
      return(FALSE);
   else {
      fail("Can't find end looking out.");
      /* NOTREACHED */
   }
}

/* ARGSUSED */
static long_boolean outposter_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* cw_end exists and is looking out */
   return(((real_people->people[real_index | 3].id1 & 017) == (010 + ((real_index & 4) >> 1))));
}

/* ARGSUSED */
static long_boolean outposter_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* ccw_end exists and is looking out */
   return(((real_people->people[real_index & 4].id1 & 017) == (010 + ((real_index & 4) >> 1))));
}

/* ARGSUSED */
static long_boolean nexttrnglspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int z;
   static int aa[4][3] = {
      {-3, -2, -2}, {2, -2, 0}, {-3, 0, 0}, {1, 0, -2}};
   int this_person = real_people->people[real_index].id1;

   z = aa[real_direction][real_index];
   if (z < -2)
      fail("Can't determine where to go or which way to face.");
   else if (z == -2)
      return(FALSE);
   else {
      z = real_people->people[z].id1;
      if (z & BIT_PERSON)
         return((z ^ this_person) & 1);
      else {
         fail("Can't determine where to go or which way to face.");
	 /* NOTREACHED */
      }
   }
}

/* ARGSUSED */
static long_boolean next62spot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int z;
   static int aa[4][6] = {
      {-2, -3, -2, -2, -3, -2}, {-2, 2, -2, -2, 3, -2}, {-2, -3, -2, -2, -3, -2}, {-2, 0, -2, -2, 5, -2}};
   int this_person = real_people->people[real_index].id1;

   z = aa[real_direction][real_index];
   if (z < -2)
      fail("Can't determine where to go or which way to face.");
   else if (z == -2)
      return(FALSE);
   else {
      z = real_people->people[z].id1;
      if (z & BIT_PERSON)
         return((z ^ this_person) & 1);
      else {
         fail("Can't determine where to go or which way to face.");
	 /* NOTREACHED */
     }
   }
}

/* ARGSUSED */
static long_boolean next_magic62spot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int z;
   static int aa[4][6] = {
      {-3, -3, -3, -3, -3, -3}, {-2, 3, -2, -2, 2, -2}, {-3, -3, -3, -3, -3, -3}, {-2, 5, -2, -2, 0, -2}};
   int this_person = real_people->people[real_index].id1;

   z = aa[real_direction][real_index];
   if (z < -2)
      fail("Can't determine where to go or which way to face.");
   else if (z == -2)
      return(FALSE);
   else {
      z = real_people->people[z].id1;
      if (z & BIT_PERSON)
         return((z ^ this_person) & 1);
      else {
         fail("Can't determine where to go or which way to face."); 
         /* NOTREACHED */
      }
   }
}

/* ARGSUSED */
static long_boolean next_galaxyspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int z;
   static int aa[4][8] = {
      { 1,  2, -3,  2,  3,  4, -3,  0},
      {-3,  2,  3,  4, -3,  4,  5,  6},
      { 7,  0, -3,  4,  5,  6, -3,  6},
      {-3,  0,  1,  2, -3,  6,  7,  0}};
   int this_person = real_people->people[real_index].id1;

   /* We always return true for centers.  That way
      the centers can reverse flip a galaxy even if the
      next point does not exist.  Maybe this isn't a good way
      to do it, and we need another predicate.  Sigh. */
   if (real_index & 1) return(TRUE);

   z = aa[real_direction][real_index];
   if (z >= -2) {
      z = real_people->people[z].id1;
      if (z & BIT_PERSON) return((z ^ this_person) & 1);
   }
   fail("Can't determine where to go or which way to face.");
   /* NOTREACHED */
}

/* ARGSUSED */
static long_boolean column_double_down(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(
      (northified_index < 3)              /* unless #1 in column, it's easy */
         ||
      (northified_index > 4)
         ||
      /* if #1, my adjacent end must exist and face in */
      (((((real_index + 2) & 4) ^ 1) + 1) == (real_people->people[real_index ^ 7].id1 & 017)));
}

/* ARGSUSED */
static long_boolean boyp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id2 & ID2_BOY) != 0);
}

/* ARGSUSED */
static long_boolean girlp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id2 & ID2_GIRL) != 0);
}

/* ARGSUSED */
static long_boolean roll_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id1 & ROLLBITR) != 0);
}

/* ARGSUSED */
static long_boolean roll_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id1 & ROLLBITL) != 0);
}

/* ARGSUSED */
static long_boolean x12_boy_facing_girl(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id2;
   int other_person = real_people->people[real_index ^ 1].id2;
   return((this_person & ID2_BOY) && (other_person & ID2_GIRL));
}

/* ARGSUSED */
static long_boolean x12_girl_facing_boy(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id2;
   int other_person = real_people->people[real_index ^ 1].id2;
   return((this_person & ID2_GIRL) && (other_person & ID2_BOY));
}

/* ARGSUSED */
static long_boolean x22_boy_facing_girl(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id2;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) ^ 3)].id2;
   return((this_person & ID2_BOY) && (other_person & ID2_GIRL));
}

/* ARGSUSED */
static long_boolean x22_girl_facing_boy(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) ^ 3)].id2;
   int this_person = real_people->people[real_index].id2;
   return((this_person & ID2_GIRL) && (other_person & ID2_BOY));
}

/* ARGSUSED */
static long_boolean dmd_ctrs_rh(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->people[1].id1 | real_people->people[3].id1) {
      if (     (!real_people->people[1].id1 || (real_people->people[1].id1 & d_mask)==d_east) &&
               (!real_people->people[3].id1 || (real_people->people[3].id1 & d_mask)==d_west) )
         return(TRUE);
      else if ((!real_people->people[1].id1 || (real_people->people[1].id1 & d_mask)==d_west) &&
               (!real_people->people[3].id1 || (real_people->people[3].id1 & d_mask)==d_east) )
         return(FALSE);
   }

   fail("Can't determine handedness of centers.");
   /* NOTREACHED */
}

/* ARGSUSED */
static long_boolean trngl_pt_rh(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if ((real_people->people[0].id1 & d_mask)==d_west)
      return(TRUE);
   else if ((real_people->people[0].id1 & d_mask)==d_east)
      return(FALSE);

   fail("Can't determine handedness of triangle point.");
   /* NOTREACHED */
}

/* ARGSUSED */
static long_boolean q_tag_front(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_index & 2)
      /* I am in the center line, see if I have a miniwave. */
      return(((real_people->people[real_index].id1 ^ real_people->people[real_index ^ 1].id1) & DIR_MASK) == 2);
   else {
      /* I am on the outside; find the end of the center line nearest me. */
      int z;
      if (real_index & 1) z = real_index ^ 3; else z = real_index ^ 6;
      return(((real_people->people[z].id1 & 017) == (010 + (real_index >> 1))) &&
            (((real_people->people[z].id1 ^ real_people->people[z ^ 1].id1) & DIR_MASK) == 2));
   }
}

/* ARGSUSED */
static long_boolean q_tag_back(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_index & 2)
      /* I am in the center line, see if I have a miniwave. */
      return(((real_people->people[real_index].id1 ^ real_people->people[real_index ^ 1].id1) & DIR_MASK) == 2);
   else {
      /* I am on the outside; find the end of the center line nearest me. */
      int z;
      if (real_index & 1) z = real_index ^ 3; else z = real_index ^ 6;
      return(((real_people->people[z].id1 & 017) == (012 - (real_index >> 1))) &&
            (((real_people->people[z].id1 ^ real_people->people[z ^ 1].id1) & DIR_MASK) == 2));
   }
}

/* ARGSUSED */
static long_boolean q_line_front(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_index & 2)
      /* I am in the center line, see if I have a couple. */
      return(((real_people->people[real_index].id1 ^ real_people->people[real_index ^ 1].id1) & DIR_MASK) == 0);
   else {
      /* I am on the outside; find the end of the center line nearest me. */
      int z;
      if (real_index & 1) z = real_index ^ 3; else z = real_index ^ 6;
      return(((real_people->people[z].id1 & 017) == (010 + (real_index >> 1))) &&
            (((real_people->people[z].id1 ^ real_people->people[z ^ 1].id1) & DIR_MASK) == 0));
   }
}

/* ARGSUSED */
static long_boolean q_line_back(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_index & 2)
      /* I am in the center line, see if I have a couple. */
      return(((real_people->people[real_index].id1 ^ real_people->people[real_index ^ 1].id1) & DIR_MASK) == 0);
   else {
      /* I am on the outside; find the end of the center line nearest me. */
      int z;
      if (real_index & 1) z = real_index ^ 3; else z = real_index ^ 6;
      return(((real_people->people[z].id1 & 017) == (012 - (real_index >> 1))) &&
            (((real_people->people[z].id1 ^ real_people->people[z ^ 1].id1) & DIR_MASK) == 0));
   }
}

/* BEWARE!!  This list must track the array "predtab" in the database maker. */

/* The first 6 of these (the constant to use is SELECTOR_PREDS) take a predicate.
   Any call that uses one of these predicates in its definition will cause a
   popup to appear asking "who?". */

long_boolean (*pred_table[])(
   setup *real_people,
   int real_index,
   int real_direction,
   int northified_index) = {
      selected,                        /* "select" */
      unselect,                        /* "unselect" */
      select_near_select,              /* "select_near_select" */
      select_near_unselect,            /* "select_near_unselect" */
      unselect_near_select,            /* "unselect_near_select" */
      unselect_near_unselect,          /* "unselect_near_unselect" */
      always,                          /* "always" */
      x22_miniwave,                    /* "x22_miniwave" */
      x22_couple,                      /* "x22_couple" */
      x22_facing_someone,              /* "x22_facing_someone" */
      x14_once_rem_miniwave,           /* "x14_once_rem_miniwave" */
      x14_once_rem_couple,             /* "x14_once_rem_couple" */
      opp_in_quad,                     /* "lines_miniwave" */
      same_in_quad,                    /* "lines_couple" */
      opp_in_magic,                    /* "lines_magic_miniwave" */
      same_in_magic,                   /* "lines_magic_couple" */
      lines_once_rem_miniwave,         /* "lines_once_rem_miniwave" */
      lines_once_rem_couple,           /* "lines_once_rem_couple" */
      same_in_pair,                    /* "lines_tandem" */
      opp_in_pair,                     /* "lines_antitandem" */
      same_in_quad,                    /* "columns_tandem" */
      opp_in_quad,                     /* "columns_antitandem" */
      same_in_magic,                   /* "columns_magic_tandem" */
      opp_in_magic,                    /* "columns_magic_antitandem" */
      lines_once_rem_couple,           /* "columns_once_rem_tandem" */
      lines_once_rem_miniwave,         /* "columns_once_rem_antitandem" */
      same_in_pair,                    /* "columns_couple" */
      opp_in_pair,                     /* "columns_miniwave" */
      x12_beau_or_miniwave,            /* "1x2_beau_or_miniwave" */
      x14_wheel_and_deal,              /* "1x4_wheel_and_deal" */
      x16_wheel_and_deal,              /* "1x6_wheel_and_deal" */
      x18_wheel_and_deal,              /* "1x8_wheel_and_deal" */
      vert1,                           /* "vert1" */
      vert2,                           /* "vert2" */
      inner_active_lines,              /* "inner_active_lines" */
      outer_active_lines,              /* "outer_active_lines" */
      judge_is_right,                  /* "judge_is_right" */
      judge_is_left,                   /* "judge_is_left" */
      socker_is_right,                 /* "socker_is_right" */
      socker_is_left,                  /* "socker_is_left" */
      inroller_is_cw,                  /* "inroller_is_cw" */
      magic_inroller_is_cw,            /* "magic_inroller_is_cw" */
      outroller_is_cw,                 /* "outroller_is_cw" */
      outposter_is_cw,                 /* "outposter_is_cw" */
      outposter_is_ccw,                /* "outposter_is_ccw" */
      nexttrnglspot_is_tboned,         /* "nexttrnglspot_is_tboned" */
      next62spot_is_tboned,            /* "next62spot_is_tboned" */
      next_magic62spot_is_tboned,      /* "next_magic62spot_is_tboned" */
      next_galaxyspot_is_tboned,       /* "next_galaxyspot_is_tboned" */
      column_double_down,              /* "column_double_down" */
      boyp,                            /* "boyp" */
      girlp,                           /* "girlp" */
      roll_is_cw,                      /* "roll_is_cw" */
      roll_is_ccw,                     /* "roll_is_ccw" */
      x12_boy_facing_girl,             /* "x12_boy_facing_girl" */
      x12_girl_facing_boy,             /* "x12_girl_facing_boy" */
      x22_boy_facing_girl,             /* "x22_boy_facing_girl" */
      x22_girl_facing_boy,             /* "x22_girl_facing_boy" */
      dmd_ctrs_rh,                     /* "dmd_ctrs_rh" */
      trngl_pt_rh,                     /* "trngl_pt_rh" */
      q_tag_front,                     /* "q_tag_front" */
      q_tag_back,                      /* "q_tag_back" */
      q_line_front,                    /* "q_line_front" */
      q_line_back};                    /* "q_line_back" */
