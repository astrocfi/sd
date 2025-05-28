/* SD -- square dance caller's helper.

    Copyright (C) 1990-1994  William B. Ackerman.

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
   selectp

and the following external variables:
   selector_used
   number_used
   pred_table     which is filled with pointers to the predicate functions
*/

#include "sd.h"

/* These variables are external. */

long_boolean selector_used;
long_boolean number_used;


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
         return TRUE;
      case selector_none:
         return FALSE;
   }

   /* Demand that the subject be real. */

   if (!(ss->people[place].id1 & BIT_PERSON))
      fail("Can't decide who are selected.");

   pid2 = ss->people[place].id2;

   switch (current_selector) {
      case selector_boys:
         if      ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_BOY) return TRUE;
         else if ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_GIRL) return FALSE;
         break;
      case selector_girls:
         if      ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_GIRL) return TRUE;
         else if ((pid2 & (ID2_BOY|ID2_GIRL)) == ID2_BOY) return FALSE;
         break;
      case selector_heads:
         if      ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_HEAD) return TRUE;
         else if ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_SIDE) return FALSE;
         break;
      case selector_sides:
         if      ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_SIDE) return TRUE;
         else if ((pid2 & (ID2_HEAD|ID2_SIDE)) == ID2_HEAD) return FALSE;
         break;
      case selector_headcorners:
         if      ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_HCOR) return TRUE;
         else if ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_SCOR) return FALSE;
         break;
      case selector_sidecorners:
         if      ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_SCOR) return TRUE;
         else if ((pid2 & (ID2_HCOR|ID2_SCOR)) == ID2_HCOR) return FALSE;
         break;
      case selector_headboys:
         if      ((pid2 & (ID2_ALL_ID|ID2_NHB)) == (ID2_HEAD|ID2_BOY)) return TRUE;
         else if ((pid2 & ID2_NHB) == ID2_NHB) return FALSE;
         break;
      case selector_headgirls:
         if      ((pid2 & (ID2_ALL_ID|ID2_NHG)) == (ID2_HEAD|ID2_GIRL)) return TRUE;
         else if ((pid2 & ID2_NHG) == ID2_NHG) return FALSE;
         break;
      case selector_sideboys:
         if      ((pid2 & (ID2_ALL_ID|ID2_NSB)) == (ID2_SIDE|ID2_BOY)) return TRUE;
         else if ((pid2 & ID2_NSB) == ID2_NSB) return FALSE;
         break;
      case selector_sidegirls:
         if      ((pid2 & (ID2_ALL_ID|ID2_NSG)) == (ID2_SIDE|ID2_GIRL)) return TRUE;
         else if ((pid2 & ID2_NSG) == ID2_NSG) return FALSE;
         break;
      case selector_centers:
         if      ((pid2 & (ID2_CENTER|ID2_END)) == ID2_CENTER) return TRUE;
         else if ((pid2 & (ID2_CENTER|ID2_END)) == ID2_END) return FALSE;
         break;
      case selector_ends:
         if      ((pid2 & (ID2_CENTER|ID2_END)) == ID2_END) return TRUE;
         else if ((pid2 & (ID2_CENTER|ID2_END)) == ID2_CENTER) return FALSE;
         break;
      case selector_leads:
         if      ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_LEAD) return TRUE;
         else if ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_TRAILER) return FALSE;
         break;
      case selector_trailers:
         if      ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_TRAILER) return TRUE;
         else if ((pid2 & (ID2_LEAD|ID2_TRAILER)) == ID2_LEAD) return FALSE;
         break;
      case selector_beaus:
         if      ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BEAU) return TRUE;
         else if ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BELLE) return FALSE;
         break;
      case selector_belles:
         if      ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BELLE) return TRUE;
         else if ((pid2 & (ID2_BEAU|ID2_BELLE)) == ID2_BEAU) return FALSE;
         break;
      case selector_center2:
         if      ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_CTR2) return TRUE;
         else if ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_OUTR6) return FALSE;
         break;
      case selector_center6:
         if      ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_CTR6) return TRUE;
         else if ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_OUTR2) return FALSE;
         break;
      case selector_outer2:
         if      ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_OUTR2) return TRUE;
         else if ((pid2 & (ID2_CTR6|ID2_OUTR2)) == ID2_CTR6) return FALSE;
         break;
      case selector_outer6:
         if      ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_OUTR6) return TRUE;
         else if ((pid2 & (ID2_CTR2|ID2_OUTR6)) == ID2_CTR2) return FALSE;
         break;
      case selector_center4:
         if      ((pid2 & (ID2_CTR4|ID2_OUTRPAIRS)) == ID2_CTR4) return TRUE;
         else if ((pid2 & (ID2_CTR4|ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return FALSE;
         break;
      case selector_outerpairs:
         if      ((pid2 & (ID2_CTR4|ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return TRUE;
         else if ((pid2 & (ID2_CTR4|ID2_OUTRPAIRS)) == ID2_CTR4) return FALSE;
         break;
      case selector_headliners:
         if      ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_HEADLINE) return TRUE;
         else if ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_SIDELINE) return FALSE;
         break;
      case selector_sideliners:
         if      ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_SIDELINE) return TRUE;
         else if ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_HEADLINE) return FALSE;
         break;
      case selector_nearline:
         if      (pid2 & ID2_NEARLINE) return TRUE;
         else if (pid2 & (ID2_FARLINE|ID2_FARCOL|ID2_FARBOX)) return FALSE;
         break;
      case selector_farline:
         if      (pid2 & ID2_FARLINE) return TRUE;
         else if (pid2 & (ID2_NEARLINE|ID2_NEARCOL|ID2_NEARBOX)) return FALSE;
         break;
      case selector_nearcolumn:
         if      (pid2 & ID2_NEARCOL) return TRUE;
         else if (pid2 & (ID2_FARLINE|ID2_FARCOL|ID2_FARBOX)) return FALSE;
         break;
      case selector_farcolumn:
         if      (pid2 & ID2_FARCOL) return TRUE;
         else if (pid2 & (ID2_NEARLINE|ID2_NEARCOL|ID2_NEARBOX)) return FALSE;
         break;
      case selector_nearbox:
         if      (pid2 & ID2_NEARBOX) return TRUE;
         else if (pid2 & (ID2_FARLINE|ID2_FARCOL|ID2_FARBOX)) return FALSE;
         break;
      case selector_farbox:
         if      (pid2 & ID2_FARBOX) return TRUE;
         else if (pid2 & (ID2_NEARLINE|ID2_NEARCOL|ID2_NEARBOX)) return FALSE;
         break;
      default:
         fail("ERROR - selector failed to get initialized.");
   }
   
   fail("Can't decide who are selected.");
   /* NOTREACHED */
}



/* Here are the predicates.  They will get put into the array "pred_table". */

/* ARGSUSED */
Private long_boolean selected(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return selectp(real_people, real_index);
}

/* ARGSUSED */
Private long_boolean unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return !selectp(real_people, real_index);
}

/* ARGSUSED */
Private long_boolean select_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(selectp(real_people, real_index) && selectp(real_people, real_index ^ 1));
}

/* ARGSUSED */
Private long_boolean select_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(selectp(real_people, real_index) && !selectp(real_people, real_index ^ 1));
}

/* ARGSUSED */
Private long_boolean unselect_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(!selectp(real_people, real_index) && selectp(real_people, real_index ^ 1));
}

/* ARGSUSED */
Private long_boolean unselect_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(!(selectp(real_people, real_index) || selectp(real_people, real_index ^ 1)));
}

/* ARGSUSED */
Private long_boolean once_rem_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return selectp(real_people, real_index ^ 3);
}

/* ARGSUSED */
Private long_boolean conc_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return selectp(real_people, real_index ^ 2);
          
}

/* ARGSUSED */
Private long_boolean select_once_rem_from_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(selectp(real_people, real_index) && !selectp(real_people, real_index ^ 3));
}

/* ARGSUSED */
Private long_boolean unselect_once_rem_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(!selectp(real_people, real_index) && selectp(real_people, real_index ^ 3));
}

/* ARGSUSED */
Private long_boolean always(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return TRUE;
}

/* ARGSUSED */
Private long_boolean x22_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person, other_index, other_person;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: return TRUE;
      case cr_2fl_only: case cr_couples_only: return FALSE;
   }

   this_person = real_people->people[real_index].id1;
   other_index = real_index ^ (((real_direction << 1) & 2) ^ 1);
   other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean x22_couple(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person, other_index, other_person;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: return FALSE;
      case cr_2fl_only: case cr_couples_only: return TRUE;
   }

   this_person = real_people->people[real_index].id1;
   other_index = real_index ^ (((real_direction << 1) & 2) ^ 1);
   other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean x22_facing_someone(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_index = real_index ^ (((real_direction << 1) & 2) ^ 3);
   int other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean x22_tandem_with_someone(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_index = real_index ^ (((real_direction << 1) & 2) ^ 3);
   int other_person = real_people->people[other_index].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean x14_once_rem_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 3].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean x14_once_rem_couple(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 3].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean lines_couple(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return FALSE;
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return TRUE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;
      return ((this_person ^ other_person) & DIR_MASK) == 0;
   }
}

/* ARGSUSED */
Private long_boolean columns_tandem(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 1].id1;
   if (real_people->kind == s1x6 && real_index >= 2)
      other_person = real_people->people[7 - real_index].id1;
   return ((this_person ^ other_person) & DIR_MASK) == 0;
}

/* ARGSUSED */
Private long_boolean lines_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return TRUE;
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return FALSE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;
      return ((this_person ^ other_person) & DIR_MASK) == 2;
   }
}

/* ARGSUSED */
Private long_boolean cast_normal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assump_cast)
      return TRUE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;
      return ((this_person ^ other_person) & DIR_MASK) == 2;
   }
}

/* ARGSUSED */
Private long_boolean cast_pushy(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assump_cast)
      return FALSE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;
      return ((this_person ^ other_person) & DIR_MASK) == 0;
   }
}

/* ARGSUSED */
Private long_boolean cast_normal_or_warn(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assump_cast)
      return TRUE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;

      switch ((this_person ^ other_person) & DIR_MASK) {
         case 2:
            return TRUE;
         case 0:
            return FALSE;
         default:
            warn(warn__opt_for_normal_cast);
            return TRUE;
      }
   }
}

/* ARGSUSED */
Private long_boolean columns_antitandem(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 1].id1;
   if (real_people->kind == s1x6 && real_index >= 2)
      other_person = real_people->people[7 - real_index].id1;
   return ((this_person ^ other_person) & DIR_MASK) == 2;
}

/* ARGSUSED */
Private long_boolean same_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 7].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean opp_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 7].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean opp_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 6].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean same_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 6].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean lines_once_rem_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 2].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean lines_once_rem_couple(setup *real_people, int real_index,
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
Private long_boolean x12_beau_or_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only || northified_index == 0)
      return TRUE;
   else if (real_people->cmd.cmd_assume.assumption == cr_couples_only)
      return FALSE;
   else {
      int other_person = real_people->people[real_index ^ 1].id1;
      int direction_diff = other_person ^ real_direction;
      if (!other_person || (direction_diff & 1))
         fail("Need a real, not T-boned, person to work with."); 
      return ((direction_diff & 2) == 2);
   }
}

/* Test for wheel and deal to be done 2FL-style, or beau side of 1FL.  Returns
   false if belle side of 1FL.  Raises an error if wheel and deal can't be done. */
/* ARGSUSED */
Private long_boolean x14_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* We assume people have already been checked for coupleness. */

   if (northified_index <= 1)
      /* We are in the beau-side couple -- it's OK. */
      return TRUE;
   else {
      /* We are in the belle-side couple.  First, see if an assumption is guiding us. */

      int other_people;

      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_2fl_only: return TRUE;
         case cr_1fl_only: return FALSE;
      }

      /* Find the two people in the other couple.
         Just "or" them to be sure we get someone.  They are already known
         to be facing consistently if they are both there. */
      other_people = real_people->people[real_index ^ 2].id1 |
                     real_people->people[real_index ^ 3].id1;

      /* At least one of those people must exist. */
      if (!other_people)
         fail("Can't tell how to do this -- no live people.");

      /* See if they face the same way as myself.  Note that the "2" bit of
         real_index is the complement of my own direction bit. */
      if (((other_people ^ real_index) & 2))
         return FALSE;   /* This is a 1FL. */

      return TRUE;       /* This is a 2FL. */
   }
}


/* Test for 3X3 wheel_and_deal to be done 2FL-style, or beau side of 1FL. */
/* ARGSUSED */
Private long_boolean x16_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* We assume people have already been checked for coupleness. */

   if (northified_index <= 2 || northified_index >= 9)
      /* We are in the beau-side triad -- it's OK. */
      return(TRUE);
   else {
      /* We are in the belle-side triad -- find the three people in the other triad.
         Just "or" them to be sure we get someone.  They are already known
         to be facing consistently if they are all there. */
      int other_side = ((real_index / 3) ^ 1) * 3;
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
Private long_boolean x18_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   /* We assume people have already been checked for coupleness. */

   if (northified_index <= 3 || northified_index >= 12)
      /* We are in the beau-side quad -- it's OK. */
      return(TRUE);
   else {
      /* We are in the belle-side quad -- find the four people in the other quad.
         Just "or" them to be sure we get someone.  They are already known
         to be facing consistently if they are all there. */
      int other_side = (real_index & 014) ^ 4;
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



Private long_boolean vert1(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) | 1)].id1;
   if (northified_index & 1)
      return(((this_person ^ other_person) & DIR_MASK) == 0);
   else
      return(TRUE);
}

Private long_boolean vert2(setup *real_people, int real_index,
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
Private long_boolean inner_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if ((real_index+3) & 2)
      return(northified_index >= 4);     /* I am an end */
   else if (real_people->people[real_index ^ 1].id1)
      return(                            /* I am a center, with a live partner */
         (012 - ((real_index & 4) >> 1)) == (real_people->people[real_index ^ 1].id1 & 017));
   else if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return(northified_index < 4);
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return(northified_index >= 4);
   else
      return FALSE;
}

/* ARGSUSED */
Private long_boolean outer_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if ((real_index+3) & 2)
      return(northified_index < 4);      /* I am an end */
   else if (real_people->people[real_index ^ 1].id1)
      return(                            /* I am a center, with a live partner */
         (010 + ((real_index & 4) >> 1)) == (real_people->people[real_index ^ 1].id1 & 017));
   else if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return(northified_index >= 4);
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return(northified_index < 4);
   else
      return FALSE;
}

/* ARGSUSED */
Private long_boolean judge_is_right(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: case cr_2fl_only: return FALSE;  /* This is an error -- socker/judge can't be unambiguous. */
      case cr_all_facing_same: case cr_1fl_only: case cr_li_lo: return TRUE;
      case cr_magic_only: return (real_index & 1) == 0;
   }

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 0)       /* judge exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 2));         /* we do not have another judge to my left */
}

/* ARGSUSED */
Private long_boolean judge_is_left(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: case cr_2fl_only: return FALSE;  /* This is an error -- socker/judge can't be unambiguous. */
      case cr_all_facing_same: case cr_1fl_only: case cr_li_lo: return FALSE;
      case cr_magic_only: return (real_index & 1) != 0;
   }

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 2)           /* judge exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 0));     /* we do not have another judge to my right */
}

/* ARGSUSED */
Private long_boolean socker_is_right(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: case cr_2fl_only: return FALSE;  /* This is an error -- socker/judge can't be unambiguous. */
      case cr_all_facing_same: case cr_1fl_only: case cr_li_lo: return FALSE;
      case cr_magic_only: return (real_index & 1) != 0;
   }

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 2)       /* socker exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 0));         /* we do not have another socker to my left */
}

/* ARGSUSED */
Private long_boolean socker_is_left(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: case cr_2fl_only: return FALSE;  /* This is an error -- socker/judge can't be unambiguous. */
      case cr_all_facing_same: case cr_1fl_only: case cr_li_lo: return TRUE;
      case cr_magic_only: return (real_index & 1) == 0;
   }

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 0)           /* socker exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 2));     /* we do not have another socker to my right */
}

Private long_boolean judge_is_right_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 0)       /* judge exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 2));         /* we do not have another judge to my left */
}

Private long_boolean judge_is_left_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 2)           /* judge exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 0));     /* we do not have another judge to my right */
}

Private long_boolean socker_is_right_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 2)       /* socker exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 0));         /* we do not have another socker to my left */
}

Private long_boolean socker_is_left_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 0)           /* socker exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 2));     /* we do not have another socker to my right */
}

/* Helper function for in/out roll circulate stuff.  "Yes_roll_direction" is the facing direction
   that constitutes what we are looking for (inroller or outroller as the case may be). */
Private long_boolean in_out_roll_select(int yes_roll_direction, int cw_end, int ccw_end, char *errmsg)
{
   int no_roll_direction = 022 - yes_roll_direction;

   if (  /* cw_end exists and is proper, and we do not have ccw_end proper also */
         (cw_end == yes_roll_direction && ccw_end != yes_roll_direction) ||
         /* or if ccw_end exists and is improper, and cw_end is a phantom */
         (ccw_end == no_roll_direction && cw_end == 0))
      return(TRUE);
   else if (
         /* ccw_end exists and is proper, and we do not have cw_end proper also */
         (ccw_end == yes_roll_direction && cw_end != yes_roll_direction) ||
         /* or if cw_end exists and is improper, and ccw_end is a phantom */
         (cw_end == no_roll_direction && ccw_end == 0))
      return(FALSE);
   else {
      fail(errmsg);
      /* NOTREACHED */
   }
}

static int magic_cw_idx[8] =      {3,  7,  7,  3,  7,  3,  3,  7};
static int magic_ccw_idx[8] =     {0,  4,  4,  0,  4,  0,  0,  4};
static int magic_cw_idx_2x3[8] =  {2,  5,  2,  5,  2,  5};
static int magic_ccw_idx_2x3[8] = {0,  3,  0,  3,  0,  3};
static int cw_idx_2x3[6] =        {2,  2,  2,  5,  5,  5};
static int ccw_idx_2x3[6] =       {0,  0,  0,  3,  3,  3};
static int cw_idx_2x6[12] =       {5,  5,  5,  5,  5,  5, 11, 11, 11, 11, 11, 11};
static int ccw_idx_2x6[12] =      {0,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6};
static int cw_idx_2x8[16] =       {7,  7,  7,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15};
static int ccw_idx_2x8[16] =      {0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8,  8,  8,  8,  8};

static int magic_inroll_directions[8] =     {012, 010, 010, 012, 010, 012, 012, 010};
static int inroll_directions_2x3[6] =       {012, 012, 012, 010, 010, 010};
static int inroll_directions_2x6[12] =      {012, 012, 012, 012, 012, 012, 010, 010, 010, 010, 010, 010};
static int inroll_directions_2x8[16] =      {012, 012, 012, 012, 012, 012, 012, 012, 010, 010, 010, 010, 010, 010, 010, 010};
static int magic_inroll_directions_2x3[6] = {012, 010, 012, 010, 012, 010};


/* ARGSUSED */
Private long_boolean inroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) == 0;
   if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) == 0;

   return in_out_roll_select(
      012 - ((real_index & 4) >> 1),
      real_people->people[real_index | 3].id1 & 017,
      real_people->people[real_index & 4].id1 & 017,
      "Can't find end looking in.");
}

/* ARGSUSED */
Private long_boolean magic_inroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) == 0;
   if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) == 0;

   return in_out_roll_select(
      magic_inroll_directions[real_index],
      real_people->people[magic_cw_idx[real_index]].id1 & 017,
      real_people->people[magic_ccw_idx[real_index]].id1 & 017,
      "Can't find magic end looking in.");
}

/* ARGSUSED */
Private long_boolean outroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) != 0;
   if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) != 0;

   return in_out_roll_select(
      010 + ((real_index & 4) >> 1),
      real_people->people[real_index | 3].id1 & 017,
      real_people->people[real_index & 4].id1 & 017,
      "Can't find end looking out.");
}

/* ARGSUSED */
Private long_boolean magic_outroller_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) != 0;
   if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) != 0;

   return in_out_roll_select(
      022 - magic_inroll_directions[real_index],
      real_people->people[magic_cw_idx[real_index]].id1 & 017,
      real_people->people[magic_ccw_idx[real_index]].id1 & 017,
      "Can't find magic end looking out.");
}

/* ARGSUSED */
Private long_boolean inroller_is_cw_2x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      fail("Not legal.");

   return in_out_roll_select(
      inroll_directions_2x3[real_index],
      real_people->people[cw_idx_2x3[real_index]].id1 & 017,
      real_people->people[ccw_idx_2x3[real_index]].id1 & 017,
      "Can't find end looking in.");
}

/* ARGSUSED */
Private long_boolean magic_inroller_is_cw_2x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      fail("Not legal.");

   return in_out_roll_select(
      magic_inroll_directions_2x3[real_index],
      real_people->people[magic_cw_idx_2x3[real_index]].id1 & 017,
      real_people->people[magic_ccw_idx_2x3[real_index]].id1 & 017,
      "Can't find magic end looking in.");
}

/* ARGSUSED */
Private long_boolean outroller_is_cw_2x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      fail("Not legal.");

   return in_out_roll_select(
      022 - inroll_directions_2x3[real_index],
      real_people->people[cw_idx_2x3[real_index]].id1 & 017,
      real_people->people[ccw_idx_2x3[real_index]].id1 & 017,
      "Can't find end looking out.");
}

/* ARGSUSED */
Private long_boolean magic_outroller_is_cw_2x3(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      fail("Not legal.");

   return in_out_roll_select(
      022 - magic_inroll_directions_2x3[real_index],
      real_people->people[magic_cw_idx_2x3[real_index]].id1 & 017,
      real_people->people[magic_ccw_idx_2x3[real_index]].id1 & 017,
      "Can't find magic end looking out.");
}

/* ARGSUSED */
Private long_boolean inroller_is_cw_2x6(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index / 6)) & 1) == 0;

   return in_out_roll_select(
      inroll_directions_2x6[real_index],
      real_people->people[cw_idx_2x6[real_index]].id1 & 017,
      real_people->people[ccw_idx_2x6[real_index]].id1 & 017,
      "Can't find end looking in.");
}

/* ARGSUSED */
Private long_boolean outroller_is_cw_2x6(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index / 6)) & 1) != 0;

   return in_out_roll_select(
      022 - inroll_directions_2x6[real_index],
      real_people->people[cw_idx_2x6[real_index]].id1 & 017,
      real_people->people[ccw_idx_2x6[real_index]].id1 & 017,
      "Can't find end looking out.");
}

/* ARGSUSED */
Private long_boolean inroller_is_cw_2x8(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 3)) & 1) == 0;

   return in_out_roll_select(
      inroll_directions_2x8[real_index],
      real_people->people[cw_idx_2x8[real_index]].id1 & 017,
      real_people->people[ccw_idx_2x8[real_index]].id1 & 017,
      "Can't find end looking in.");
}

/* ARGSUSED */
Private long_boolean outroller_is_cw_2x8(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 3)) & 1) != 0;

   return in_out_roll_select(
      022 - inroll_directions_2x8[real_index],
      real_people->people[cw_idx_2x8[real_index]].id1 & 017,
      real_people->people[ccw_idx_2x8[real_index]].id1 & 017,
      "Can't find end looking out.");
}

/* ARGSUSED */
Private long_boolean outposter_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int outroll_direction = 010 + ((real_index & 4) >> 1);
   unsigned int cw_dir = real_people->people[real_index | 3].id1 & 017;

   /* cw_end exists and is looking out */
   if (cw_dir == outroll_direction) return TRUE;
   /* or cw_end is phantom but ccw_end is looking in, so it has to be the phantom */
   else if (cw_dir == 0 && (real_people->people[real_index & 4].id1 & 017) == (022 - outroll_direction)) return TRUE;
   /* we don't know */
   else return FALSE;
}

/* ARGSUSED */
Private long_boolean outposter_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int inroll_direction = 012 - ((real_index & 4) >> 1);

   /* Demand that cw_end be looking in -- otherwise "outposter_is_cw"
      would have accepted it if it were legal. */
   if ((real_people->people[real_index | 3].id1 & 017) != inroll_direction) return FALSE;
   else {
      /* Now if ccw_end is looking out or is a phantom, it's OK. */
      unsigned int ccw_dir = real_people->people[real_index & 4].id1 & 017;
      if (ccw_dir == 0 || ccw_dir == 022-inroll_direction) return TRUE;
      return FALSE;
   }
}

/* ARGSUSED */
Private long_boolean nexttrnglspot_is_tboned(setup *real_people, int real_index,
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
Private long_boolean next62spot_is_tboned(setup *real_people, int real_index,
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
Private long_boolean next_magic62spot_is_tboned(setup *real_people, int real_index,
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
Private long_boolean next_galaxyspot_is_tboned(setup *real_people, int real_index,
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
Private long_boolean column_double_down(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return(
      (northified_index < 3)              /* unless #1 in column, it's easy */
         ||
      (northified_index > 4)
         ||
      /* if #1, my adjacent end must exist and face in */
      (((((real_index + 2) & 4) >> 1) + 1) == (real_people->people[real_index ^ 7].id1 & 017)));
}

/* ARGSUSED */
Private long_boolean boyp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id2 & ID2_BOY) != 0);
}

/* ARGSUSED */
Private long_boolean girlp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id2 & ID2_GIRL) != 0);
}

/* ARGSUSED */
Private long_boolean roll_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id1 & ROLLBITR) != 0);
}

/* ARGSUSED */
Private long_boolean roll_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return((real_people->people[real_index].id1 & ROLLBITL) != 0);
}

/* ARGSUSED */
Private long_boolean x12_boy_facing_girl(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id2;
   int other_person = real_people->people[real_index ^ 1].id2;
   return((this_person & ID2_BOY) && (other_person & ID2_GIRL));
}

/* ARGSUSED */
Private long_boolean x12_girl_facing_boy(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id2;
   int other_person = real_people->people[real_index ^ 1].id2;
   return((this_person & ID2_GIRL) && (other_person & ID2_BOY));
}

/* ARGSUSED */
Private long_boolean x22_boy_facing_girl(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int this_person = real_people->people[real_index].id2;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) ^ 3)].id2;
   return((this_person & ID2_BOY) && (other_person & ID2_GIRL));
}

/* ARGSUSED */
Private long_boolean x22_girl_facing_boy(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) ^ 3)].id2;
   int this_person = real_people->people[real_index].id2;
   return((this_person & ID2_GIRL) && (other_person & ID2_BOY));
}

/* ARGSUSED */
Private long_boolean leftp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_left;
}

/* ARGSUSED */
Private long_boolean rightp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_right;
}

/* ARGSUSED */
Private long_boolean inp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_in;
}

/* ARGSUSED */
Private long_boolean outp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_out;
}


/* ARGSUSED */
Private long_boolean zigzagp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_zigzag;
}

/* ARGSUSED */
Private long_boolean zagzigp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_zagzig;
}

/* ARGSUSED */
Private long_boolean zigzigp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_zigzig;
}

/* ARGSUSED */
Private long_boolean zagzagp(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_zagzag;
}

/* ARGSUSED */
Private long_boolean no_dir_p(setup *real_people, int real_index,
   int real_direction, int northified_index)
{
   return current_direction == direction_no_direction;
}

/* ARGSUSED */
Private long_boolean dmd_ctrs_rh(setup *real_people, int real_index,
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
Private long_boolean trngl_pt_rh(setup *real_people, int real_index,
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
Private long_boolean q_tag_front(setup *real_people, int real_index,
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
Private long_boolean q_tag_back(setup *real_people, int real_index,
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
Private long_boolean q_line_front(setup *real_people, int real_index,
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
Private long_boolean q_line_back(setup *real_people, int real_index,
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

/* BEWARE!!  This list must track the array "predtab" in dbcomp.c . */

/* The first 10 of these (the constant to use is SELECTOR_PREDS) take a predicate.
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
      once_rem_from_select,            /* "once_rem_from_select" */
      conc_from_select,                /* "conc_from_select" */
      select_once_rem_from_unselect,   /* "select_once_rem_from_unselect" */
      unselect_once_rem_from_select,   /* "unselect_once_rem_from_select" */
      always,                          /* "always" */
      x22_miniwave,                    /* "x22_miniwave" */
      x22_couple,                      /* "x22_couple" */
      x22_facing_someone,              /* "x22_facing_someone" */
      x22_tandem_with_someone,         /* "x22_tandem_with_someone" */
      x14_once_rem_miniwave,           /* "x14_once_rem_miniwave" */
      x14_once_rem_couple,             /* "x14_once_rem_couple" */
      lines_miniwave,                  /* "lines_miniwave" */
      lines_couple,                    /* "lines_couple" */
      cast_normal,                     /* "cast_normal" */
      cast_pushy,                      /* "cast_pushy" */
      cast_normal_or_warn,             /* "cast_normal_or_warn" */
      opp_in_magic,                    /* "lines_magic_miniwave" */
      same_in_magic,                   /* "lines_magic_couple" */
      lines_once_rem_miniwave,         /* "lines_once_rem_miniwave" */
      lines_once_rem_couple,           /* "lines_once_rem_couple" */
      same_in_pair,                    /* "lines_tandem" */
      opp_in_pair,                     /* "lines_antitandem" */
      columns_tandem,                  /* "columns_tandem" */
      columns_antitandem,              /* "columns_antitandem" */
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
      judge_is_right_1x3,              /* "judge_is_right_1x3" */
      judge_is_left_1x3,               /* "judge_is_left_1x3" */
      socker_is_right_1x3,             /* "socker_is_right_1x3" */
      socker_is_left_1x3,              /* "socker_is_left_1x3" */
      inroller_is_cw,                  /* "inroller_is_cw" */
      magic_inroller_is_cw,            /* "magic_inroller_is_cw" */
      outroller_is_cw,                 /* "outroller_is_cw" */
      magic_outroller_is_cw,           /* "magic_outroller_is_cw" */
      inroller_is_cw_2x3,              /* "inroller_is_cw_2x3" */
      magic_inroller_is_cw_2x3,        /* "magic_inroller_is_cw_2x3" */
      outroller_is_cw_2x3,             /* "outroller_is_cw_2x3" */
      magic_outroller_is_cw_2x3,       /* "magic_outroller_is_cw_2x3" */
      inroller_is_cw_2x6,              /* "inroller_is_cw_2x6" */
      outroller_is_cw_2x6,             /* "outroller_is_cw_2x6" */
      inroller_is_cw_2x8,              /* "inroller_is_cw_2x8" */
      outroller_is_cw_2x8,             /* "outroller_is_cw_2x8" */
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
      leftp,                           /* "leftp" */
      rightp,                          /* "rightp" */
      inp,                             /* "inp" */
      outp,                            /* "outp" */
      zigzagp,                         /* "zigzagp" */
      zagzigp,                         /* "zagzigp" */
      zigzigp,                         /* "zigzigp" */
      zagzagp,                         /* "zagzagp" */
      no_dir_p,                        /* "no_dir_p" */
      dmd_ctrs_rh,                     /* "dmd_ctrs_rh" */
      trngl_pt_rh,                     /* "trngl_pt_rh" */
      q_tag_front,                     /* "q_tag_front" */
      q_tag_back,                      /* "q_tag_back" */
      q_line_front,                    /* "q_line_front" */
      q_line_back};                    /* "q_line_back" */
