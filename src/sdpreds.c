/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/* SD -- square dance caller's helper.

    Copyright (C) 1990-1998  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 31. */

/* This defines the following function:
   selectp

and the following external variables:
   selector_used
   number_used
   mandatory_call_used
   pred_table     which is filled with pointers to the predicate functions
   selector_preds
*/

#include "sd.h"

/* These variables are external. */

long_boolean selector_used;
long_boolean number_used;
long_boolean mandatory_call_used;


/* If a real person and a person under test are XOR'ed, the result AND'ed with this constant,
   and the low bits of that result examined, it will tell whether the person under test is real
   and facing in the same direction (result = 0), the opposite direction (result == 2), or whatever. */

#define DIR_MASK (BIT_PERSON | 3)

#define ID1_PERM_ALL_ID (ID1_PERM_HEAD|ID1_PERM_SIDE|ID1_PERM_BOY|ID1_PERM_GIRL)





extern long_boolean selectp(setup *ss, int place)
{
   uint32 permpid1, pid2, p1, p2;
   selector_kind s;

   selector_used = TRUE;

   /* Pull out the cases that do not require the person to be real. */

   switch (current_options.who) {
      case selector_all:
      case selector_everyone:
         return TRUE;
      case selector_none:
         return FALSE;
   }

   permpid1 = ss->people[place].id1;
   pid2 = ss->people[place].id2;

   /* Demand that the subject be real. */

   if (!(permpid1 & BIT_PERSON))
      fail("Can't decide who are selected.");

   switch (current_options.who) {
      case selector_boys:
         if      ((permpid1 & (ID1_PERM_BOY|ID1_PERM_GIRL)) == ID1_PERM_BOY) return TRUE;
         else if ((permpid1 & (ID1_PERM_BOY|ID1_PERM_GIRL)) == ID1_PERM_GIRL) return FALSE;
         break;
      case selector_girls:
         if      ((permpid1 & (ID1_PERM_BOY|ID1_PERM_GIRL)) == ID1_PERM_GIRL) return TRUE;
         else if ((permpid1 & (ID1_PERM_BOY|ID1_PERM_GIRL)) == ID1_PERM_BOY) return FALSE;
         break;
      case selector_heads:
         if      ((permpid1 & (ID1_PERM_HEAD|ID1_PERM_SIDE)) == ID1_PERM_HEAD) return TRUE;
         else if ((permpid1 & (ID1_PERM_HEAD|ID1_PERM_SIDE)) == ID1_PERM_SIDE) return FALSE;
         break;
      case selector_sides:
         if      ((permpid1 & (ID1_PERM_HEAD|ID1_PERM_SIDE)) == ID1_PERM_SIDE) return TRUE;
         else if ((permpid1 & (ID1_PERM_HEAD|ID1_PERM_SIDE)) == ID1_PERM_HEAD) return FALSE;
         break;
      case selector_headcorners:
         if      ((permpid1 & (ID1_PERM_HCOR|ID1_PERM_SCOR)) == ID1_PERM_HCOR) return TRUE;
         else if ((permpid1 & (ID1_PERM_HCOR|ID1_PERM_SCOR)) == ID1_PERM_SCOR) return FALSE;
         break;
      case selector_sidecorners:
         if      ((permpid1 & (ID1_PERM_HCOR|ID1_PERM_SCOR)) == ID1_PERM_SCOR) return TRUE;
         else if ((permpid1 & (ID1_PERM_HCOR|ID1_PERM_SCOR)) == ID1_PERM_HCOR) return FALSE;
         break;
      case selector_headboys:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NHB)) == (ID1_PERM_HEAD|ID1_PERM_BOY)) return TRUE;
         else if ((permpid1 & ID1_PERM_NHB) == ID1_PERM_NHB) return FALSE;
         break;
      case selector_headgirls:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NHG)) == (ID1_PERM_HEAD|ID1_PERM_GIRL)) return TRUE;
         else if ((permpid1 & ID1_PERM_NHG) == ID1_PERM_NHG) return FALSE;
         break;
      case selector_sideboys:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NSB)) == (ID1_PERM_SIDE|ID1_PERM_BOY)) return TRUE;
         else if ((permpid1 & ID1_PERM_NSB) == ID1_PERM_NSB) return FALSE;
         break;
      case selector_sidegirls:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NSG)) == (ID1_PERM_SIDE|ID1_PERM_GIRL)) return TRUE;
         else if ((permpid1 & ID1_PERM_NSG) == ID1_PERM_NSG) return FALSE;
         break;
      case selector_centers:
      case selector_ends:
         p2 = pid2 & (ID2_CENTER|ID2_END);
         if      (p2 == ID2_CENTER) s = selector_centers;
         else if (p2 == ID2_END)    s = selector_ends;
         else break;
         goto eq_return;
      case selector_leads:
      case selector_trailers:
         p2 = pid2 & (ID2_LEAD|ID2_TRAILER);
         if      (p2 == ID2_LEAD)    s = selector_leads;
         else if (p2 == ID2_TRAILER) s = selector_trailers;
         else break;
         goto eq_return;
      case selector_lead_ends:
      case selector_lead_ctrs:
      case selector_trail_ends:
      case selector_trail_ctrs:
         p2 = pid2 & (ID2_LEAD|ID2_TRAILER|ID2_CENTER|ID2_END);
         if      (p2 == (ID2_LEAD|ID2_END))       s = selector_lead_ends;
         else if (p2 == (ID2_LEAD|ID2_CENTER))    s = selector_lead_ctrs;
         else if (p2 == (ID2_TRAILER|ID2_END))    s = selector_trail_ends;
         else if (p2 == (ID2_TRAILER|ID2_CENTER)) s = selector_trail_ctrs;
         else break;
         goto eq_return;
      case selector_end_boys:
      case selector_end_girls:
      case selector_center_boys:
      case selector_center_girls:
         p1 = permpid1 & (ID1_PERM_BOY|ID1_PERM_GIRL);
         p2 = pid2 & (ID2_CENTER|ID2_END);
         if      (p2 == ID2_END && p1 == ID1_PERM_BOY)     s = selector_end_boys;
         else if (p2 == ID2_END && p1 == ID1_PERM_GIRL)    s = selector_end_girls;
         else if (p2 == ID2_CENTER && p1 == ID1_PERM_BOY)  s = selector_center_boys;
         else if (p2 == ID2_CENTER && p1 == ID1_PERM_GIRL) s = selector_center_girls;
         else break;
         goto eq_return;
      case selector_beaus:
      case selector_belles:
         p2 = pid2 & (ID2_BEAU|ID2_BELLE);
         if      (p2 == ID2_BEAU)  s = selector_beaus;
         else if (p2 == ID2_BELLE) s = selector_belles;
         else break;
         goto eq_return;
      case selector_center2:
      case selector_outer6:
         p2 = pid2 & (ID2_CTR2|ID2_OUTR6);
         if      (p2 == ID2_CTR2)  s = selector_center2;
         else if (p2 == ID2_OUTR6) s = selector_outer6;
         else break;
         goto eq_return;
      case selector_verycenters:    /* Gotta fix this stuff - use fall-through variable. */
         p2 = pid2 & (ID2_CTR2|ID2_OUTR6);
         if      (p2 == ID2_CTR2)  s = selector_verycenters;
         else if (p2 == ID2_OUTR6) s = selector_outer6;
         else break;
         goto eq_return;
      case selector_center6:
      case selector_outer2:
         p2 = pid2 & (ID2_CTR6|ID2_OUTR2);
         if      (p2 == ID2_CTR6)  s = selector_center6;
         else if (p2 == ID2_OUTR2) s = selector_outer2;
         else break;
         goto eq_return;
      case selector_veryends:    /* Gotta fix this stuff - use fall-through variable. */
         p2 = pid2 & (ID2_CTR6|ID2_OUTR2);
         if      (p2 == ID2_CTR6)  s = selector_center6;
         else if (p2 == ID2_OUTR2) s = selector_veryends;
         else break;
         goto eq_return;
#ifdef TGL_SELECTORS
      case selector_wvbasetgl:
         break;
      case selector_tndbasetgl:
         break;
      case selector_insidetgl:
         switch (ss->kind) {
            case s_ptpd: case s_qtag: case s_rigger:
               p2 = pid2 & (ID2_CTR6|ID2_OUTR2);
               if      (p2 == ID2_CTR6)  return TRUE;
               else if (p2 == ID2_OUTR2) return FALSE;
               break;
         }
         break;
      case selector_outsidetgl:
         switch (ss->kind) {
            case s_ptpd: case s_qtag: case s_bone: case s_spindle:
               p2 = pid2 & (ID2_CTR2|ID2_OUTR6);
               if      (p2 == ID2_CTR2)  return FALSE;
               else if (p2 == ID2_OUTR6) return TRUE;
               break;
         }
         break;
      case selector_inpttgl:
         break;
      case selector_outpttgl:
         break;
#endif
      case selector_ctrdmd:
         if      ((pid2 & (ID2_CTRDMD|ID2_NCTRDMD)) == ID2_CTRDMD) return TRUE;
         else if ((pid2 & (ID2_CTRDMD|ID2_NCTRDMD)) == ID2_NCTRDMD) return FALSE;
         break;
      case selector_ctr_1x4:
         if      ((pid2 & (ID2_CTR1X4|ID2_NCTR1X4)) == ID2_CTR1X4) return TRUE;
         else if ((pid2 & (ID2_CTR1X4|ID2_NCTR1X4)) == ID2_NCTR1X4) return FALSE;
         break;
      case selector_ctr_1x6:
         if      ((pid2 & (ID2_CTR1X6|ID2_NCTR1X6)) == ID2_CTR1X6) return TRUE;
         else if ((pid2 & (ID2_CTR1X6|ID2_NCTR1X6)) == ID2_NCTR1X6) return FALSE;
         break;
      case selector_outer1x3s:
         if      ((pid2 & (ID2_OUTR1X3|ID2_NOUTR1X3)) == ID2_OUTR1X3) return TRUE;
         else if ((pid2 & (ID2_OUTR1X3|ID2_NOUTR1X3)) == ID2_NOUTR1X3) return FALSE;
         break;
      case selector_center4:
         if      ((pid2 & (ID2_CTR4|ID2_OUTRPAIRS)) == ID2_CTR4) return TRUE;
         else if ((pid2 & (ID2_CTR4|ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return FALSE;
         else if ((pid2 & (ID2_CTR4|ID2_END)) == ID2_CTR4) return TRUE;
         else if ((pid2 & (ID2_CTR4|ID2_END)) == ID2_END) return FALSE;
         else if ((pid2 & (ID2_CTR4|ID2_NCTR1X4)) == ID2_CTR4) return TRUE;
         else if ((pid2 & (ID2_CTR4|ID2_NCTR1X4)) == ID2_NCTR1X4) return FALSE;
         break;
      case selector_outerpairs:
         if      ((pid2 & (ID2_CTR4  |ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return TRUE;
         else if ((pid2 & (ID2_CTR4  |ID2_OUTRPAIRS)) == ID2_CTR4) return FALSE;
         else if ((pid2 & (ID2_CENTER|ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return TRUE;
         else if ((pid2 & (ID2_CENTER|ID2_OUTRPAIRS)) == ID2_CENTER) return FALSE;
         break;
      case selector_headliners:
         if      ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_HEADLINE) return TRUE;
         else if ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_SIDELINE) return FALSE;
         break;
      case selector_sideliners:
         if      ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_SIDELINE) return TRUE;
         else if ((pid2 & (ID2_HEADLINE|ID2_SIDELINE)) == ID2_HEADLINE) return FALSE;
         break;
      case selector_thosefacing:
         if      ((pid2 & (ID2_FACING|ID2_NOTFACING)) == ID2_FACING) return TRUE;
         else if ((pid2 & (ID2_FACING|ID2_NOTFACING)) == ID2_NOTFACING) return FALSE;
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
      case selector_facingfront:
         if      (pid2 & ID2_FACEFRONT) return TRUE;
         else if (pid2 & ID2_FACEBACK) return FALSE;
         break;
      case selector_facingback:
         if      (pid2 & ID2_FACEBACK) return TRUE;
         else if (pid2 & ID2_FACEFRONT) return FALSE;
         break;

      /* For the unsymmetrical selectors, we demand that the person not be virtual
         (e.g. tandem person) or active phantom.  The former requirement could cause
         us to lose a few extremely rare cases, but it isn't worth using up zillions of bits. */

      case selector_boy1:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0000;
         break;
      case selector_girl1:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0100;
         break;
      case selector_cpl1:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK & ~0100) == 0000;
         break;
      case selector_boy2:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0200;
         break;
      case selector_girl2:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0300;
         break;
      case selector_cpl2:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK & ~0100) == 0200;
         break;
      case selector_boy3:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0400;
         break;
      case selector_girl3:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0500;
         break;
      case selector_cpl3:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK & ~0100) == 0400;
         break;
      case selector_boy4:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0600;
         break;
      case selector_girl4:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK) == 0700;
         break;
      case selector_cpl4:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK & ~0100) == 0600;
         break;
      case selector_cpls1_2:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK & ~0300) == 0000;
         break;
      case selector_cpls2_3:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return ((permpid1+0200) & PID_MASK & ~0300) == 0400;
         break;
      case selector_cpls3_4:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return (permpid1 & PID_MASK & ~0300) == 0400;
         break;
      case selector_cpls4_1:
         if (!(permpid1 & (XPID_MASK & ~PID_MASK))) return ((permpid1+0200) & PID_MASK & ~0300) == 0000;
         break;

      default:
         fail("ERROR - selector failed to get initialized.");
   }
   
   fail("Can't decide who are selected.");
   /* NOTREACHED */

   eq_return:

   return (current_options.who == s);
}


static Const long int iden_tab[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
static Const long int dbl_tab01[4] = {0, 1, FALSE, TRUE};
static Const long int dbl_tab03[2] = {0, 3};
static Const long int dbl_tab23[2] = {2, 3};
static Const long int dbl_tab21[4] = {2, 1, TRUE, FALSE};

static Const long int boystuff_no_rh[3]  = {ID1_PERM_BOY,  ID1_PERM_GIRL, 0};
static Const long int girlstuff_no_rh[3] = {ID1_PERM_GIRL, ID1_PERM_BOY,  0};
static Const long int boystuff_rh[3]     = {ID1_PERM_BOY,  ID1_PERM_GIRL, 1};
static Const long int girlstuff_rh[3]    = {ID1_PERM_GIRL, ID1_PERM_BOY,  1};
static Const long int semi_squeeze_tab[8] = {0xD, 0xE, 0x9, 0x9, 0x2, 0xD, 0x2, 0xE};


/* Here are the predicates.  They will get put into the array "pred_table". */

/* ARGSUSED */
Private long_boolean someone_selected(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return selectp(real_people, real_index ^ (*extra_stuff));
}

/* ARGSUSED */
Private long_boolean sum_mod_selected(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int otherindex = (*extra_stuff) - real_index;
   int size = setup_attrs[real_people->kind].setup_limits+1;
   if (otherindex >= size) otherindex -= size;
   return selectp(real_people, otherindex);
}

/* ARGSUSED */
Private long_boolean plus_mod_selected(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int otherindex = real_index + (*extra_stuff);
   int size = setup_attrs[real_people->kind].setup_limits+1;
   if (otherindex >= size) otherindex -= size;
   return selectp(real_people, otherindex);
}

/* ARGSUSED */
Private long_boolean semi_squeezer_select(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int other_index = ((northified_index ^ extra_stuff[northified_index&7]) + real_index - northified_index) & 0xF;

   return      (real_people->people[other_index].id1 & BIT_PERSON) &&
               selectp(real_people, other_index);
}

/* ARGSUSED */
Private long_boolean unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return !selectp(real_people, real_index);
}

/* ARGSUSED */
Private long_boolean select_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return TRUE;

   return      (real_people->people[real_index ^ 1].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
Private long_boolean select_near_select_or_phantom(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return TRUE;

   return      !(real_people->people[real_index ^ 1].id1 & BIT_PERSON) ||
               selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
Private long_boolean select_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return FALSE;

   return      !(real_people->people[real_index ^ 1].id1 & BIT_PERSON) ||
               !selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
Private long_boolean unselect_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_none) return FALSE;

   return      (real_people->people[real_index ^ 1].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
Private long_boolean unselect_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_none) return TRUE;

   return      !(real_people->people[real_index ^ 1].id1 & BIT_PERSON) ||
               !selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
Private long_boolean select_once_rem_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return TRUE;

   if (real_people->kind == s2x4)
      return   (real_people->people[real_index ^ 2].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 2);
   else
      return   (real_people->people[real_index ^ 3].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 3);


}

/* ARGSUSED */
Private long_boolean select_once_rem_from_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return FALSE;

   if (real_people->kind == s2x4)
      return   !(real_people->people[real_index ^ 2].id1 & BIT_PERSON) ||
               !selectp(real_people, real_index ^ 2);
   else
      return   !(real_people->people[real_index ^ 3].id1 & BIT_PERSON) ||
               !selectp(real_people, real_index ^ 3);
}

/* ARGSUSED */
Private long_boolean unselect_once_rem_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_none) return FALSE;

   if (real_people->kind == s2x4)
      return   (real_people->people[real_index ^ 2].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 2);
   else
      return   (real_people->people[real_index ^ 3].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 3);
}

/* ARGSUSED */
Private long_boolean select_and_roll_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return selectp(real_people, real_index) && (real_people->people[real_index].id1 & ROLLBITR) != 0;
}

/* ARGSUSED */
Private long_boolean select_and_roll_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return selectp(real_people, real_index) && (real_people->people[real_index].id1 & ROLLBITL) != 0;
}

/* ARGSUSED */
Private long_boolean always(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return TRUE;
}

/* ARGSUSED */
Private long_boolean x22_cpltest(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int other_index;

   switch (real_people->cmd.cmd_assume.assumption) {
   case cr_wave_only: case cr_miniwaves: return extra_stuff[2];
   case cr_2fl_only: case cr_couples_only: case cr_li_lo: return extra_stuff[3];
   }

   other_index = real_index ^ (((real_direction << 1) & 2) ^ extra_stuff[1]);
   return ((real_people->people[real_index].id1 ^
            real_people->people[other_index].id1) & DIR_MASK) == (uint32) *extra_stuff;
}

/* ARGSUSED */
Private long_boolean x22_facing_test(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int other_index = real_index ^ (((real_direction << 1) & 2) ^ extra_stuff[1]);
   return ((real_people->people[real_index].id1 ^
            real_people->people[other_index].id1) & DIR_MASK) == (uint32) *extra_stuff;
}

/* ARGSUSED */
Private long_boolean cols_someone_in_front(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (real_people->kind == s2x3) {
      if (real_index == 1 || real_index == 4) {
         if (real_people->people[real_index+((northified_index==1) ? 1 : -1)].id1)
            return TRUE;
         else return FALSE;
      }
      else return TRUE;
   }
   else {
      if (northified_index < 3 || northified_index > 4) {
         if (real_people->people[real_index+((northified_index<4) ? 1 : -1)].id1)
            return TRUE;
         else return FALSE;
      }
      else return FALSE;
   }
}

/* ARGSUSED */
Private long_boolean x14_once_rem_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   switch (real_people->cmd.cmd_assume.assumption) {
   case cr_wave_only: case cr_li_lo: case cr_1fl_only: return FALSE;
   case cr_2fl_only: case cr_magic_only: return TRUE;
   }

   switch ((real_people->people[real_index].id1 ^
            real_people->people[real_index ^ 3].id1) & DIR_MASK) {
   case 0:
      return FALSE;
   case 2:
      return TRUE;
   default:
      if (extra_stuff[0] & 2) {
         /* This is "intlk_cast_normal_or_warn".  Don't give the warning if person
            would have known what to do anyway. */
         if (!(real_index & 1))
            warn(warn__opt_for_normal_cast);
         return TRUE;
      }
      else
         return FALSE;
   }
}

/* ARGSUSED */
Private long_boolean x14_once_rem_couple(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   switch (real_people->cmd.cmd_assume.assumption) {
   case cr_wave_only: case cr_li_lo: case cr_1fl_only: return TRUE;
   case cr_2fl_only: case cr_magic_only: return FALSE;
   }

   return ((real_people->people[real_index].id1 ^
            real_people->people[real_index ^ 3].id1) & DIR_MASK) == 0;
}

/* ARGSUSED */
Private long_boolean lines_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
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
Private long_boolean lines_couple(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
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

static Const long int tab_mwv_in_3n1[8]  = {2, 0, 0, 4, 3, 7, 2};
static Const long int tab_cpl_in_3n1[8]  = {0, 2, 0, 4, 3, 7, 2};
static Const long int tab_mwv_out_3n1[8] = {2, 0, 1, 5, 2, 6, 3};
static Const long int tab_cpl_out_3n1[8] = {0, 2, 1, 5, 2, 6, 3};

/* ARGSUSED */
Private long_boolean check_3n1_setup(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 A = extra_stuff[0];
   uint32 B = extra_stuff[1];
   uint32 C = extra_stuff[2];
   uint32 D = extra_stuff[3];
   uint32 E = extra_stuff[4];
   uint32 F = extra_stuff[5];
   uint32 G = extra_stuff[6];

   if (     real_people->cmd.cmd_assume.assumption == cr_wave_only ||
            real_people->cmd.cmd_assume.assumption == cr_2fl_only ||
            ((real_people->people[real_index].id1 ^ real_people->people[real_index ^ 1].id1) & DIR_MASK) != A)
      return FALSE;
   else if (real_people->kind == s2x4) {
      int k = real_index & 2;

      return
         ((real_people->people[C].id1 ^ real_people->people[E].id1) & DIR_MASK) == 0 &&
         ((real_people->people[D].id1 ^ real_people->people[F].id1) & DIR_MASK) == 0 &&
         ((real_people->people[C].id1 ^ real_people->people[D].id1) & DIR_MASK) == 2 &&
         ((real_people->people[k ^ 2].id1 ^ real_people->people[k ^ 3].id1) & DIR_MASK) == B &&
         ((real_people->people[k ^ 6].id1 ^ real_people->people[k ^ 7].id1) & DIR_MASK) == B &&
         ((real_people->people[k ^ 4].id1 ^ real_people->people[k ^ 5].id1) & DIR_MASK) == A;
   }
   else if (real_people->kind == s1x8) {
      int k = real_index & 2;
      int j = k + (k>>1);

      return
         ((real_people->people[C].id1 ^ real_people->people[G].id1) & DIR_MASK) == 0 &&
         ((real_people->people[D].id1 ^ real_people->people[F^1].id1) & DIR_MASK) == 0 &&
         ((real_people->people[C].id1 ^ real_people->people[D].id1) & DIR_MASK) == 2 &&
         ((real_people->people[k ^ 2].id1 ^ real_people->people[k ^ 3].id1) & DIR_MASK) == B &&
         ((real_people->people[j ^ 6].id1 ^ real_people->people[j ^ 7].id1) & DIR_MASK) == B &&
         ((real_people->people[j ^ 4].id1 ^ real_people->people[j ^ 5].id1) & DIR_MASK) == A;
   }
   else {
      return
         ((real_people->people[C].id1 ^ real_people->people[G].id1) & DIR_MASK) == 0 &&
         ((real_people->people[C].id1 ^ real_people->people[(real_index & 2) ^ E].id1) & DIR_MASK) == B;
   }
}

/* the "extra_stuff" argument is:
     0 for "miniwave_side_of_2n1_line"
     3 for "couple_side_of_2n1_line" */
/* ARGSUSED */
Private long_boolean some_side_of_2n1_line(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int k = extra_stuff[0];
   if (real_index == 0 || real_index == 3) k ^= 3;

   return
      ((real_people->people[0].id1 ^ real_people->people[2].id1) & DIR_MASK) == 2      &&
      ((real_people->people[0].id1 ^ real_people->people[5].id1) & DIR_MASK) == 0      &&
      ((real_people->people[2].id1 ^ real_people->people[3].id1) & DIR_MASK) == 0      &&
      ((real_people->people[0].id1 ^ real_people->people[k+1].id1) & DIR_MASK) == 0    &&
      ((real_people->people[2].id1 ^ real_people->people[4-k].id1) & DIR_MASK) == 0;
}

/* the "extra_stuff" argument is:
     0 for "cast_pushy"
     1 for "cast_normal"
     3 for "cast_normal_or_warn" */
/* ARGSUSED */
Private long_boolean cast_normal_or_whatever(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assumption == cr_miniwaves ||
         real_people->cmd.cmd_assume.assump_cast)
      return extra_stuff[0] & 1;
   else if (real_people->cmd.cmd_assume.assumption == cr_couples_only ||
         real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return (extra_stuff[0] & 1) ^ 1;
   else {
      uint32 this_person = real_people->people[real_index].id1;
      uint32 other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;

      switch (((this_person ^ other_person) & DIR_MASK) ^ ((extra_stuff[0] & 1) << 1)) {
      case 0:
         return TRUE;
      case 2:
         return FALSE;
      default:
         if (extra_stuff[0] & 2) {
            /* This is "cast_normal_or_warn".  Don't give the warning if person
               would have known what to do anyway. */
            if (     real_people->kind == s1x2
                     ||
                     (real_index != 1 && real_index != ((real_people->kind == s1x6) ? 4 : 3)))
               warn(warn__opt_for_normal_cast);
            return TRUE;
         }
         else
            return FALSE;
      }
   }
}

/* ARGSUSED */
Private long_boolean columns_tandem(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 this_person;
   int other_index;

   if (real_people->kind != s_qtag) {
      switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: case cr_2fl_only: return extra_stuff[0] ^ 1;
      case cr_magic_only: case cr_li_lo: return extra_stuff[0];
      }
   }
   else if (real_index == 2 || real_index == 6) return FALSE;  /* Wings of qtag always fail. */

   this_person = real_people->people[real_index].id1;
   other_index = real_index ^ 1;

   if (real_people->kind == s1x6 && real_index >= 2)
      other_index = 7 - real_index;
   else if (real_people->kind == s1x3) {
      if (real_index == 2 || !(real_direction & 2))
         other_index = 3 - real_index;
   }
   else if (real_people->kind == s2x3) {
      if (real_index == 2)
         other_index = 1;
      else if (real_index == 3)
         other_index = 4;
      else if (!(real_direction & 2)) {
         if (real_index == 1)
            other_index = 2;
         else if (real_index == 4)
            other_index = 3;
      }
   }
   else if (real_people->kind == s_qtag) {
      other_index = real_index ^ 2;
      if ((real_index & 3) == 0 ||
          (((real_index & 3) == 3) && ((real_direction ^ (real_index >> 1)) & 2)))
         other_index = real_index ^ 7;
   }

   return ((this_person ^ real_people->people[other_index].id1) & DIR_MASK) ==
      ((uint32) extra_stuff[0] << 1);
}

/* ARGSUSED */
Private long_boolean same_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person;
   int other_person;

   if (real_people->cmd.cmd_assume.assump_col == 1) {
      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_2fl_only: case cr_li_lo: return TRUE;
         case cr_wave_only: case cr_magic_only: return FALSE;
      }
   }

   this_person = real_people->people[real_index].id1;
   other_person = real_people->people[real_index ^ 7].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean opp_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person;
   int other_person;

   if (real_people->cmd.cmd_assume.assump_col == 1) {
      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_wave_only: case cr_magic_only: return TRUE;
         case cr_2fl_only: case cr_li_lo: return FALSE;
      }
   }

   this_person = real_people->people[real_index].id1;
   other_person = real_people->people[real_index ^ 7].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean opp_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person;
   int other_person;

   if (real_people->cmd.cmd_assume.assump_col == 1) {
      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_wave_only: case cr_li_lo: return TRUE;
         case cr_2fl_only: case cr_magic_only: return FALSE;
      }
   }

   this_person = real_people->people[real_index].id1;
   other_person = real_people->people[real_index ^ 6].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 2);
}

/* ARGSUSED */
Private long_boolean same_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person;
   int other_person;

   if (real_people->cmd.cmd_assume.assump_col == 1) {
      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_2fl_only: case cr_magic_only: return TRUE;
         case cr_wave_only: case cr_li_lo: return FALSE;
      }
   }

   this_person = real_people->people[real_index].id1;
   other_person = real_people->people[real_index ^ 6].id1;
   return(((this_person ^ other_person) & DIR_MASK) == 0);
}

/* ARGSUSED */
Private long_boolean once_rem_test(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 2].id1;
   return ((this_person ^ other_person) & DIR_MASK) == (uint32) extra_stuff[0];
}

/* ARGSUSED */
Private long_boolean x12_beau_or_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assumption == cr_miniwaves ||
         northified_index == 0 ||
         (real_people->kind == s2x2 && northified_index == 3))
      return TRUE;
   else if (real_people->cmd.cmd_assume.assumption == cr_couples_only)
      return FALSE;
   else {
      int other_person = real_people->people[real_index ^ 1 ^ ((real_direction&1) << 1)].id1;
      int direction_diff = other_person ^ real_direction;

      /* This was "1x2_beau_miniwave_or_warn", and the other person is a phantom,
         we give a warning and assume we had a miniwave. */

      if (extra_stuff[0] == 1 && !other_person) {
         warn(warn__opt_for_normal_hinge);
         return TRUE;
      }

      if (extra_stuff[0] == 2 && !other_person) {
         return TRUE;
      }

      if (!other_person || (direction_diff & 1))
         fail("Need a real, not T-boned, person to work with."); 

      if ((direction_diff & 2) == 2)
         return TRUE;

      if (extra_stuff[0] == 2)
         warn(warn__like_linear_action);

      return FALSE;
   }
}

static Const long int swingleft_3x1dmd[8] = {-1, 0, 1, -1, 5, 6, -1, 3};

Private long_boolean can_swing_left(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   switch (real_people->kind) {
      case s1x3dmd:
         {
            int t = swingleft_3x1dmd[northified_index];
            if (t < 0) return FALSE;
            t ^= northified_index ^ real_index;
            return ((real_people->people[real_index].id1 ^ real_people->people[t].id1) & DIR_MASK) == 2;
         }
      default: return FALSE;
   }
}


/* Test for wheel and deal to be done 2FL-style, or beau side of 1FL.  Returns
   false if belle side of 1FL.  Raises a warning if wheel and deal can't be done,
   and opts for L2FL. */
/* ARGSUSED */
Private long_boolean x14_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
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
      if (!other_people) {
         warn(warn__opt_for_2fl);
         return TRUE;
      }

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
   int real_direction, int northified_index, Const long int *extra_stuff)
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
   int real_direction, int northified_index, Const long int *extra_stuff)
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

/* First test for how to do cycle and wheel.  This always passes the extreme beau,
   and always fails the extreme belle (causing the belle to go on to the next test.)
   For centers, it demands an adjacent end (otherwise we wouldn't know whether
   we were cycling or wheeling) and then returns true if that end is an extreme beau. */
/* ARGSUSED */
Private long_boolean cycle_and_wheel_1(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (northified_index == 0)
      return TRUE;
   else if (northified_index == 2)
      return FALSE;
   else {     /* We are a center.  Find our adjacent end. */
      int other_person;

      /* We think it is rather silly to use an "assume" concept to specify
         a symmetric kind of line, and then give a call that doesn't do
         anything interesting with any of the lines that the assumption can
         make, but we have to do this. */

      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_1fl_only:  case cr_2fl_only:   return (northified_index == 1);
         case cr_wave_only: case cr_magic_only: return (northified_index == 3);
      }

      other_person = real_people->people[real_index ^ 1].id1;

      if (!other_person)
         fail("Can't tell how to do this -- no live people.");

      /* See if he is an extreme beau. */
      if (((other_person ^ real_index) & 2))
         return FALSE;

      return TRUE;
   }
}

/* Second test for how to do cycle and wheel.  This finds the far end, and checks
   whether he faces the same way as myself. */
/* ARGSUSED */
Private long_boolean cycle_and_wheel_2(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int other_person;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_1fl_only: case cr_2fl_only: return TRUE;
      case cr_wave_only:  return (northified_index != 2);
      case cr_magic_only: return (northified_index == 2);
   }

   other_person = real_people->people[(real_index ^ 2) & (~1)].id1;

   /* Here we default to the "non-colliding" version of the call if the
      opposite end of our line doesn't exist. */
   if (!other_person)
      return (northified_index == 1);

   /* See if he faces the same way as myself. */
   if (((other_person ^ real_people->people[real_index].id1) & 2))
      return FALSE;

   return TRUE;
}

Private long_boolean vert1(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!(northified_index & 1))
      return TRUE;
   else if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assumption == cr_miniwaves)
      return FALSE;
   else if (real_people->cmd.cmd_assume.assumption == cr_couples_only ||
            real_people->cmd.cmd_assume.assumption == cr_li_lo)
      return TRUE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) | 1)].id1;

      return(((this_person ^ other_person) & DIR_MASK) == 0);
   }
}

Private long_boolean vert2(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if (!(northified_index & 1))
      return FALSE;
   else if (real_people->cmd.cmd_assume.assumption == cr_wave_only ||
         real_people->cmd.cmd_assume.assumption == cr_miniwaves)
      return TRUE;
   else if (real_people->cmd.cmd_assume.assumption == cr_couples_only ||
            real_people->cmd.cmd_assume.assumption == cr_li_lo)
      return FALSE;
   else {
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) | 1)].id1;

      return(((this_person ^ other_person) & DIR_MASK) == 2);
   }
}

/* ARGSUSED */
Private long_boolean inner_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if ((real_index+3) & 2)
      return(northified_index >= 4);     /* I am an end */
   else if (real_people->people[real_index ^ 1].id1)
      return                           /* I am a center, with a live partner */
         (uint32) (012 - ((real_index & 4) >> 1)) ==
         (real_people->people[real_index ^ 1].id1 & 017);
   else if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return(northified_index < 4);
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return(northified_index >= 4);
   else
      return FALSE;
}

/* ARGSUSED */
Private long_boolean outer_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if ((real_index+3) & 2)
      return(northified_index < 4);      /* I am an end */
   else if (real_people->people[real_index ^ 1].id1)
      return                            /* I am a center, with a live partner */
         (uint32) (010 + ((real_index & 4) >> 1)) ==
         (real_people->people[real_index ^ 1].id1 & 017);
   else if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return(northified_index >= 4);
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return(northified_index < 4);
   else
      return FALSE;
}

Private long_boolean judge_is_right_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 0)       /* judge exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 2));         /* we do not have another judge to my left */
}

Private long_boolean judge_is_left_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 2)           /* judge exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 0));     /* we do not have another judge to my right */
}

Private long_boolean socker_is_right_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 2)       /* socker exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 0));         /* we do not have another socker to my left */
}

Private long_boolean socker_is_left_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 0)           /* socker exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 2));     /* we do not have another socker to my right */
}

static Const long int jr1x4[4]  = {1, 0, 0, 2};
static Const long int sl1x4[4]  = {1, 2, 0, 2};
static Const long int jl1x4[4]  = {0, 2, 2, 0};
static Const long int sr1x4[4]  = {0, 0, 2, 0};

static Const long int jr1x6[4]  = {1, 0, 0, 2};
static Const long int sl1x6[4]  = {1, 2, 0, 2};
static Const long int jl1x6[4]  = {0, 2, 2, 0};
static Const long int sr1x6[4]  = {0, 0, 2, 0};

static Const long int jr1x8[4]  = {1, 0, 0, 2};
static Const long int sl1x8[4]  = {1, 2, 0, 2};
static Const long int jl1x8[4]  = {0, 2, 2, 0};
static Const long int sr1x8[4]  = {0, 0, 2, 0};

/* ARGSUSED */
Private long_boolean judge_check_1x4(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 this_person = real_people->people[real_index].id1;
   uint32 f;

   switch (real_people->cmd.cmd_assume.assumption) {
      /* This is an error -- socker/judge can't be unambiguous. */
   case cr_wave_only: case cr_2fl_only: return FALSE;
   case cr_all_facing_same: case cr_1fl_only: case cr_li_lo: return extra_stuff[0];
   case cr_magic_only: return (real_index & 1) != extra_stuff[0];
   }

   f = (this_person & 2) ^ extra_stuff[1];

   return
      /* judge/socker exists in the correct place */
      (((real_people->people[2-f].id1 ^ this_person) & 013) == (uint32) extra_stuff[2])
      &&
      /* we do not have another judge/socker in the wrong place */
      (((real_people->people[f].id1 ^ this_person) & 013) != (uint32) extra_stuff[3]);
}

/* ARGSUSED */
Private long_boolean judge_check_1x6(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 this_person = real_people->people[real_index].id1;
   uint32 f;

   switch (real_people->cmd.cmd_assume.assumption) {
      /* This is an error -- socker/judge can't be unambiguous. */
   case cr_wave_only: case cr_3x3_2fl_only: return FALSE;
   case cr_all_facing_same: case cr_1fl_only: return extra_stuff[0];
   }

   f = (this_person & 2) ^ extra_stuff[1];
   f += f>>1;

   return
      /* judge/socker exists in the correct place */
      (((real_people->people[3-f].id1 ^ this_person) & 013) == (uint32) extra_stuff[2])
      &&
      /* we do not have another judge/socker in the wrong place */
      (((real_people->people[f].id1 ^ this_person) & 013) != (uint32) extra_stuff[3]);
}

/* ARGSUSED */
Private long_boolean judge_check_1x8(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 this_person = real_people->people[real_index].id1;
   uint32 f;

   switch (real_people->cmd.cmd_assume.assumption) {
      /* This is an error -- socker/judge can't be unambiguous. */
   case cr_wave_only: case cr_4x4_2fl_only: return FALSE;
   case cr_all_facing_same: case cr_1fl_only: return extra_stuff[0];
   }

   f = ((this_person & 2) ^ extra_stuff[1]) << 1;

   return
      /* judge/socker exists in the correct place */
      (((real_people->people[4-f].id1 ^ this_person) & 013) == (uint32) extra_stuff[2])
      &&
      /* we do not have another judge/socker in the wrong place */
      (((real_people->people[f].id1 ^ this_person) & 013) != (uint32) extra_stuff[3]);
}

static Const veryshort inroll_directions[24] = {
   012, 012, 012, 012, 010, 010, 010, 010,
      3,  3,  3,  3,  7,  7,  7,  7,
      0,  0,  0,  0,  4,  4,  4,  4};
 
static Const veryshort magic_inroll_directions[24] = {
   012, 010, 010, 012, 010, 012, 012, 010,
      3,  7,  7,  3,  7,  3,  3,  7,
      0,  4,  4,  0,  4,  0,  0,  4};
 
static Const veryshort inroll_directions_2x3[18] = {
   012, 012, 012, 010, 010, 010,
      2,  2,  2,  5,  5,  5,
      0,  0,  0,  3,  3,  3};
 
static Const veryshort magic_inroll_directions_2x3[18] = {
   012, 010, 012, 010, 012, 010,
      2,  5,  2,  5,  2,  5,
      0,  3,  0,  3,  0,  3};
 
static Const veryshort inroll_directions_2x6[36] = {
   012, 012, 012, 012, 012, 012, 010, 010, 010, 010, 010, 010,
      5,  5,  5,  5,  5,  5, 11, 11, 11, 11, 11, 11,
      0,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6};
 
static Const veryshort inroll_directions_2x8[48] = {
   012, 012, 012, 012, 012, 012, 012, 012, 010, 010, 010, 010, 010, 010, 010, 010,
      7,  7,  7,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15,
      0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8,  8,  8,  8,  8};

typedef enum {
   ira__no_wave,
   ira__sixes,
   ira__eights,
   ira__gen
} inroll_assume_test;

typedef struct {
   Const veryshort *directions;
   Const long int code;
   Const inroll_assume_test ira;
} inroll_action;

static Const inroll_action inroller_cw            = {inroll_directions,           0, ira__gen};
static Const inroll_action outroller_cw           = {inroll_directions,           1, ira__gen};
static Const inroll_action magic_inroller_cw      = {magic_inroll_directions,     2, ira__gen};
static Const inroll_action magic_outroller_cw     = {magic_inroll_directions,     3, ira__gen};
static Const inroll_action inroller_cw_2x3        = {inroll_directions_2x3,       0, ira__no_wave};
static Const inroll_action outroller_cw_2x3       = {inroll_directions_2x3,       1, ira__no_wave};
static Const inroll_action magic_inroller_cw_2x3  = {magic_inroll_directions_2x3, 2, ira__no_wave};
static Const inroll_action magic_outroller_cw_2x3 = {magic_inroll_directions_2x3, 3, ira__no_wave};
static Const inroll_action inroller_2x6           = {inroll_directions_2x6,       0, ira__sixes};
static Const inroll_action outroller_2x6          = {inroll_directions_2x6,       1, ira__sixes};
static Const inroll_action inroller_2x8           = {inroll_directions_2x8,       0, ira__eights};
static Const inroll_action outroller_2x8          = {inroll_directions_2x8,       1, ira__eights};



/* ARGSUSED */
Private long_boolean in_out_roll_select(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   /* "Yes_roll_direction" is the facing direction that constitutes what we are
      looking for (inroller or outroller as the case may be). */

   Const veryshort *directions;
   Const inroll_action *thing;
   int yes_roll_direction, no_roll_direction;
   int cw_end, ccw_end;
   int cw_idx, ccw_idx;
   int size, code;

   thing = (Const inroll_action *) extra_stuff;

   directions = thing->directions;
   code = thing->code;

   switch (thing->ira) {
      case ira__no_wave:
         if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
            fail("Not legal.");
         break;
      case ira__sixes:
         if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
            return ((northified_index ^ (northified_index / 6) ^ code) & 1) == 0;
         break;
      case ira__eights:
         if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
            return ((northified_index ^ (northified_index >> 3) ^ code) & 1) == 0;
         break;
      case ira__gen:
         if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
            return ((northified_index ^ (northified_index >> 2) ^ code) & 1) == 0;
         if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
            return (((northified_index >> 1) ^ (northified_index >> 2) ^ code) & 1) == 0;
         break;
   }

   yes_roll_direction = directions[real_index];
   if (code&1) yes_roll_direction = 022 - yes_roll_direction;
   no_roll_direction = 022 - yes_roll_direction;

   size = setup_attrs[real_people->kind].setup_limits+1;

   cw_idx  = directions[real_index+size];
   ccw_idx = directions[real_index+2*size];

   cw_end =  real_people->people[cw_idx].id1 & 017;
   ccw_end = real_people->people[ccw_idx].id1 & 017;

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
      char *errmsg;
      switch (code&3) {
         case 0: errmsg = "Can't find end looking in.";        break;
         case 1: errmsg = "Can't find end looking out.";       break;
         case 2: errmsg = "Can't find magic end looking in.";  break;
         case 3: errmsg = "Can't find magic end looking out."; break;
      }
      fail(errmsg);
      /* NOTREACHED */
   }
}


/* ARGSUSED */
Private long_boolean outposter_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 outroll_direction;
   uint32 cw_dir;

   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) != 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) != 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_li_lo &&
            real_people->cmd.cmd_assume.assump_both == 2)
      return TRUE;

   outroll_direction = 010 + ((real_index & 4) >> 1);
   cw_dir = real_people->people[real_index | 3].id1 & 017;

   /* cw_end exists and is looking out */
   if (cw_dir == outroll_direction) return TRUE;
   /* or cw_end is phantom but ccw_end is looking in, so it has to be the phantom */
   else if (cw_dir == 0 && (real_people->people[real_index & 4].id1 & 017) == (022 - outroll_direction)) return TRUE;
   /* we don't know */
   else return FALSE;
}

/* ARGSUSED */
Private long_boolean outposter_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 inroll_direction;

   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) == 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) == 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_li_lo &&
            real_people->cmd.cmd_assume.assump_both == 2)
      return FALSE;

   inroll_direction = 012 - ((real_index & 4) >> 1);

   /* Demand that cw_end be looking in -- otherwise "outposter_is_cw"
      would have accepted it if it were legal. */
   if ((real_people->people[real_index | 3].id1 & 017) != inroll_direction) return FALSE;
   else {
      /* Now if ccw_end is looking out or is a phantom, it's OK. */
      uint32 ccw_dir = real_people->people[real_index & 4].id1 & 017;
      if (ccw_dir == 0 || ccw_dir == 022-inroll_direction) return TRUE;
      return FALSE;
   }
}

/* ARGSUSED */
Private long_boolean count_cw_people(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int i;
   int count = 0;

   int limit = (setup_attrs[real_people->kind].setup_limits+1) >> 1;
   if (real_index >= limit) limit <<= 1;

   for (i=real_index+1; i<limit ; i++) {
      if (real_people->people[i].id1) count++;
   }

   return (count == (*extra_stuff));
}


/* -3 means error, -2 means return FALSE, -1 does not occur, and >= 0 means test that person. */

/* ARGSUSED */
Private long_boolean check_tbone(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   long int z = extra_stuff[(real_index<<2) + real_direction];

   if (z == -2)
      return FALSE;
   else if (z >= 0) {
      uint32 zz = real_people->people[z].id1;
      if (zz & BIT_PERSON)
         return (zz ^ real_people->people[real_index].id1) & 1;
      else if (real_people->kind == s_short6) {
         switch (real_people->cmd.cmd_assume.assumption) {
            case cr_diamond_like: return TRUE;
            case cr_qtag_like:
            case cr_gen_1_4_tag:
            case cr_gen_3_4_tag: return FALSE;
            case cr_jleft:
            case cr_jright:
            case cr_ijleft:
            case cr_ijright: return real_people->cmd.cmd_assume.assump_col != 0;
         }
      }
   }
   fail("Can't determine where to go or which way to face.");
   /* NOTREACHED */
}

static Const long int trnglspot_tboned_tab[12] = {-3,  2, -3,  1,    -2, -2,  0, 0,    -2,  0,  0, -2};
static Const long int six2spot_tboned_tab[24]  = {-2, -2, -2, -2,    -3,  2, -3, 0,    -2, -2, -2, -2,    -2, -2, -2, -2,    -3, 3, -3, 5,    -2, -2, -2, -2};
static Const long int mag62spot_tboned_tab[24] = {-3, -2, -3, -2,    -3,  3, -3, 5,    -3, -2, -3, -2,    -3, -2, -3, -2,    -3, 2, -3, 0,    -3, -2, -3, -2};

/* ARGSUSED */
Private long_boolean nextinttrnglspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   static Const long int bb[24] = {-2, -2, 4, 4,     -3, 3, -3, 5,     -2, 4, 4, -2,     1, 1, -2, -2,     -3, 2, -3, 0,     1, -2, -2, 1};
   static Const long int cc[24] = {2, 2, -2, -2,      5, -2, -2, 5,    0, -3, 4, -3,     -2, -2, 5, 5,     -2, 2, 2, -2,     1, -3, 3, -3};

   return check_tbone(real_people, real_index, real_direction, northified_index,
         (real_people->kind == s_short6) ? bb : cc);
}

/* ARGSUSED */
Private long_boolean next_galaxyspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   static Const long int aa[32] = {1, -3, 7, -3,     2, 2, 0, 0,     -3, 3, -3, 1,     2, 4, 4, 2,    3, -3, 5, -3,     4, 4, 6, 6,    -3, 5, -3, 7,      0, 6, 6, 0};

   /* We always return true for centers.  That way
      the centers can reverse flip a galaxy even if the
      next point does not exist.  Maybe this isn't a good way
      to do it, and we need another predicate.  Sigh. */

   if (real_index & 1) return TRUE;

   return check_tbone(real_people, real_index, real_direction, northified_index, aa);
}

/* ARGSUSED */
Private long_boolean column_double_down(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return
      (northified_index < 3)              /* unless #1 in column, it's easy */
         ||
      (northified_index > 4)
         ||
      /* if #1, my adjacent end must exist and face in */
      ((uint32) ((((real_index + 2) & 4) >> 1) + 1) == (real_people->people[real_index ^ 7].id1 & 017));
}



/* ARGSUSED */
Private long_boolean apex_test(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   uint32 unmoving_end = (real_people->people[0].id1 & (ROLLBITR|ROLLBITL)) ? 2 : 0;
   uint32 status;

   if ((real_people->people[0].id1 &
        real_people->people[1].id1 &
        real_people->people[2].id1 & BIT_PERSON) == 0)
      fail("Can't do this with phantoms.");

   status = (real_people->people[1].id1 ^ real_people->people[unmoving_end].id1) & 2;

   if ((uint32) real_index == unmoving_end ||
       (real_index == 1 && (real_people->people[1].id1 & 2) != unmoving_end))
      status |= 1;

   return (*extra_stuff & ~status) == 0;
}



/* ARGSUSED */
Private long_boolean boygirlp(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   /* If this is a slide thru from a miniwave that is not right-handed, raise a warning. */
   if (extra_stuff[2] && northified_index != 0)
      warn(warn__tasteless_slide_thru);

   return (real_people->people[real_index].id1 & extra_stuff[0]) != 0;
}

/* ARGSUSED */
Private long_boolean roll_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return (real_people->people[real_index].id1 & ROLLBITR) != 0;
}

/* ARGSUSED */
Private long_boolean roll_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return (real_people->people[real_index].id1 & ROLLBITL) != 0;
}

/* ARGSUSED */
Private long_boolean x12_facing_other_sex(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 1].id1;
   return((this_person & extra_stuff[0]) && (other_person & extra_stuff[1]));
}

/* ARGSUSED */
Private long_boolean x22_facing_other_sex(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) ^ 3)].id1;
   return (this_person & extra_stuff[0]) && (other_person & extra_stuff[1]);
}



/* ARGSUSED */
Private long_boolean directionp(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   return current_options.where == (direction_kind) extra_stuff;
}



/* ARGSUSED */
Private long_boolean dmd_ctrs_rh(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   Const restriction_thing *p1;
   Const veryshort *p;
   assumption_thing tt;
   veryshort d1, d2;
   uint32 z = 0;
   long_boolean b1 = TRUE;
   long_boolean b2 = TRUE;

   tt.assumption = cr_dmd_ctrs_mwv;
   tt.assump_col = 0;

   p1 = get_restriction_thing(real_people->kind, tt);
   if (!p1) return FALSE;

   p = p1->map1;
   d1 = p1->map2[0];
   d2 = p1->map2[1];

   while (*p>=0) {
      uint32 t1 = real_people->people[*(p++)].id1;
      uint32 t2 = real_people->people[*(p++)].id1;
      z |= t1 | t2;
      if (t1 && (t1 & 3)!=(uint32) d1) b1 = FALSE;
      if (t2 && (t2 & 3)!=(uint32) d2) b1 = FALSE;
      if (t1 && (t1 & 3)!=(uint32) d2) b2 = FALSE;
      if (t2 && (t2 & 3)!=(uint32) d1) b2 = FALSE;
   }

   if (z) {
      if (b1)      { return !(*extra_stuff); }
      else if (b2) { return  (*extra_stuff); }
   }

   fail("Can't determine handedness.");
   /* NOTREACHED */
}

/* ARGSUSED */
Private long_boolean trngl_pt_rh(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   if ((real_people->people[0].id1 & d_mask) == d_west)
      return TRUE;
   else if ((real_people->people[0].id1 & d_mask) == d_east)
      return FALSE;

   fail("Can't determine handedness of triangle point.");
   /* NOTREACHED */
}

typedef struct {
   Const long int ctr_action;    /* -1 for TRUE, -2 for FALSE, else required direction xor. */
   Const long int end_action;
   Const long int bbbbb;
} simple_qtag_action;

typedef struct {
   Const simple_qtag_action if_jleft;
   Const simple_qtag_action if_jright;
   Const simple_qtag_action if_ijleft;
   Const simple_qtag_action if_ijright;
   Const simple_qtag_action if_tag;
   Const simple_qtag_action if_line;
   Const simple_qtag_action none;
} full_qtag_action;

static full_qtag_action q_tag_front_action = {
   {-1,   1,  0},            /* if_jleft    */
   {-1,   0,  1},            /* if_jright   */
   {-2,  -1, 99},            /* if_ijleft   */
   {-2,  -1, 99},            /* if_ijright  */
   {-1,   0, 98},            /* if_tag    */
   {-2,  -1, 99},            /* if_line   */
   { 2, 010,  2}};           /* none        */

static full_qtag_action q_tag_back_action = {
   {-1,   0,  1},            /* if_jleft    */
   {-1,   1,  0},            /* if_jright   */
   {-2,  -1, 99},            /* if_ijleft   */
   {-2,  -1, 99},            /* if_ijright  */
   {-1,   2, 98},            /* if_tag    */
   {-2,  -1, 99},            /* if_line   */
   { 2, 012,  2}};           /* none        */

static full_qtag_action q_line_front_action = {
   {-2,  -1, 99},            /* if_jleft    */
   {-2,  -1, 99},            /* if_jright   */
   {-1,   1,  0},            /* if_ijleft   */
   {-1,   0,  1},            /* if_ijright  */
   {-2,  -1, 99},            /* if_tag    */
   {-1,   0, 97},            /* if_line   */
   { 0, 010,  0}};           /* none        */

static full_qtag_action q_line_back_action = {
   {-2,  -1, 99},            /* if_jleft    */
   {-2,  -1, 99},            /* if_jright   */
   {-1,   0,  1},            /* if_ijleft   */
   {-1,   1,  0},            /* if_ijright  */
   {-2,  -1, 99},            /* if_tag    */
   {-1,   2, 97},            /* if_line   */
   { 0, 012,  0}};           /* none        */

/* ARGSUSED */
Private long_boolean q_tag_check(setup *real_people, int real_index,
   int real_direction, int northified_index, Const long int *extra_stuff)
{
   full_qtag_action *bigactionp = (full_qtag_action *) extra_stuff;
   simple_qtag_action *actionp;
   int both = real_people->cmd.cmd_assume.assump_both;

   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_jleft:         actionp = (simple_qtag_action *) &(bigactionp->if_jleft);          break;
      case cr_jright:        actionp = (simple_qtag_action *) &(bigactionp->if_jright);         break;
      case cr_ijleft:        actionp = (simple_qtag_action *) &(bigactionp->if_ijleft);         break;
      case cr_ijright:       actionp = (simple_qtag_action *) &(bigactionp->if_ijright);        break;
      case cr_real_1_4_tag:  actionp = (simple_qtag_action *) &(bigactionp->if_tag);  both = 2; break;
      case cr_real_3_4_tag:  actionp = (simple_qtag_action *) &(bigactionp->if_tag);  both = 1; break;
      case cr_real_1_4_line: actionp = (simple_qtag_action *) &(bigactionp->if_line); both = 2; break;
      case cr_real_3_4_line: actionp = (simple_qtag_action *) &(bigactionp->if_line); both = 1; break;
      default:               actionp = (simple_qtag_action *) &(bigactionp->none);              break;
   }

   if (real_index & 2) {
      /* I am in the center line. */
      if (actionp->ctr_action == -1) return TRUE;
      else if (actionp->ctr_action == -2) return FALSE;
      /* This line is executed if there is no assumption.  It attempts to determine whether the physical setup
         is a wave or a 2FL by checking just the subject and his partner.  Of course, a more thorough check
         would be a nice idea. */
      else return ((real_people->people[real_index].id1 ^ real_people->people[real_index ^ 1].id1) & DIR_MASK) == (uint32) actionp->ctr_action;
   }
   else {
      /* I am on the outside; find the end of the center line nearest me. */

      if (actionp->end_action < 0) return FALSE;
      else if (actionp->end_action <= 2) {
         if (real_people->cmd.cmd_assume.assump_col == 4)
            return ((((real_index+3) >> 1) ^ real_people->people[real_index].id1) & 2) == ((actionp->end_action == 0) ? 0UL : 2UL);

         if (actionp->bbbbb == 98) {
            uint32 t;

            both = 0;

            if ((t = real_people->people[2].id1))
               both |= actionp->end_action ^ t;

            if ((t = real_people->people[3].id1))
               both |= actionp->end_action ^ t ^ 2;

            if ((t = real_people->people[6].id1))
               both |= actionp->end_action ^ t ^ 2;

            if ((t = real_people->people[7].id1))
               both |= actionp->end_action ^ t;

            if (!both) return FALSE;
            both >>= 1;
         }
         else if (actionp->bbbbb == 97) {
            uint32 t;

            both = 0;

            if ((t = real_people->people[2].id1))
               both |= actionp->end_action ^ t;

            if ((t = real_people->people[3].id1))
               both |= actionp->end_action ^ t;

            if ((t = real_people->people[6].id1))
               both |= actionp->end_action ^ t ^ 2;

            if ((t = real_people->people[7].id1))
               both |= actionp->end_action ^ t ^ 2;

            if (!both) return FALSE;
            both >>= 1;
         }
         else {
            if (actionp->bbbbb == 0) both ^= 1;
         }

         return ((real_index ^ both) & 1) != 0;
      }
      else {
         int z;
         if (real_index & 1) z = real_index ^ 3; else z = real_index ^ 6;

            /* Demand that the indicated line end or diamond point be facing toward
               or way from this person, as required by the front/back nature of
               the predicate.

               Also, demand that the line in the center consist of miniwaves or couples,
               as required by the wave/line nature of the predicate.  But if the setup
               is an hourglass, we waive the second test.  Hence "q_tag_front" and
               "q_line_front" are indistinguishable in an hourglass. */

         return
            ((real_people->people[z].id1 & 017) == ((uint32) actionp->end_action ^ (real_index >> 1)))
                                       &&
               (real_people->kind == s_hrglass
                                 ||
               (((real_people->people[z].id1 ^ real_people->people[z ^ 1].id1) & DIR_MASK) == (uint32) actionp->bbbbb));
      }
   }
}


/* BEWARE!!  This list must track the array "predtab" in dbcomp.c . */

/* The first several of these take a predicate.
   Any call that uses one of these predicates will have its "need_a_selector"
   flag set during initialization.  We set the variable "selector_preds" to contain
   the number of such predicates. */

predicate_descriptor pred_table[] = {
      {someone_selected,               &iden_tab[0]},            /* "select" */
      {unselect,                     (Const long int *) 0},      /* "unselect" */
      {select_near_select,           (Const long int *) 0},      /* "select_near_select" */
      {select_near_select_or_phantom,(Const long int *) 0},      /* "select_near_select_or_phantom" */
      {select_near_unselect,         (Const long int *) 0},      /* "select_near_unselect" */
      {unselect_near_select,         (Const long int *) 0},      /* "unselect_near_select" */
      {unselect_near_unselect,       (Const long int *) 0},      /* "unselect_near_unselect" */
      {select_once_rem_from_select,  (Const long int *) 0},      /* "select_once_rem_from_select" */
      {someone_selected,               &iden_tab[2]},            /* "conc_from_select" */
      {someone_selected,               &iden_tab[3]},            /* "other_spindle_cw_select" */
      {someone_selected,               &iden_tab[4]},            /* "grand_conc_from_select" */
      {someone_selected,               &iden_tab[5]},            /* "other_diamond_point_select" */
      {someone_selected,               &iden_tab[6]},            /* "other_spindle_ckpt_select" */
      {someone_selected,               &iden_tab[7]},            /* "pair_person_select" */
      {sum_mod_selected,               &iden_tab[5]},            /* "person_select_sum5" */
      {sum_mod_selected,               &iden_tab[8]},            /* "person_select_sum8" */
      {sum_mod_selected,              &iden_tab[11]},            /* "person_select_sum11" */
      {sum_mod_selected,              &iden_tab[15]},            /* "person_select_sum15" */
      {plus_mod_selected,             &iden_tab[4]},            /* "person_select_plus4" */
      {plus_mod_selected,             &iden_tab[8]},            /* "person_select_plus8" */
      {plus_mod_selected,             &iden_tab[12]},           /* "person_select_plus12" */
      {semi_squeezer_select,       semi_squeeze_tab},            /* "semi_squeezer_select" */
      {select_once_rem_from_unselect,(Const long int *) 0},      /* "select_once_rem_from_unselect" */
      {unselect_once_rem_from_select,(Const long int *) 0},      /* "unselect_once_rem_from_select" */
      {select_and_roll_is_cw,        (Const long int *) 0},      /* "select_and_roll_is_cw" */
      {select_and_roll_is_ccw,       (Const long int *) 0},      /* "select_and_roll_is_ccw" */
/* End of predicates that force use of selector. */
#define PREDS_BEFORE_THIS_POINT 23
      {always,                       (Const long int *) 0},      /* "always" */
      {x22_cpltest,                    dbl_tab21},               /* "x22_miniwave" */
      {x22_cpltest,                    dbl_tab01},               /* "x22_couple" */
      {x22_facing_test,                dbl_tab23},               /* "x22_facing_someone" */
      {x22_facing_test,                dbl_tab03},               /* "x22_tandem_with_someone" */
      {cols_someone_in_front,        (Const long int *) 0},      /* "columns_someone_in_front" */
      {x14_once_rem_miniwave,         &iden_tab[1]},             /* "x14_once_rem_miniwave" */
      {x14_once_rem_couple,          (Const long int *) 0},      /* "x14_once_rem_couple" */
      {lines_miniwave,               (Const long int *) 0},      /* "lines_miniwave" */
      {lines_couple,                 (Const long int *) 0},      /* "lines_couple" */
      {check_3n1_setup,              tab_mwv_in_3n1},            /* "miniwave_side_of_in_3n1_line" */
      {check_3n1_setup,              tab_cpl_in_3n1},            /* "couple_side_of_in_3n1_line" */
      {check_3n1_setup,             tab_mwv_out_3n1},            /* "miniwave_side_of_out_3n1_line" */
      {check_3n1_setup,             tab_cpl_out_3n1},            /* "couple_side_of_out_3n1_line" */
      {check_3n1_setup,              tab_mwv_in_3n1},            /* "antitandem_side_of_in_3n1_col" */
      {check_3n1_setup,              tab_cpl_in_3n1},            /* "tandem_side_of_in_3n1_col" */
      {check_3n1_setup,             tab_mwv_out_3n1},            /* "antitandem_side_of_out_3n1_col" */
      {check_3n1_setup,             tab_cpl_out_3n1},            /* "tandem_side_of_out_3n1_col" */
      {some_side_of_2n1_line,          &iden_tab[0]},            /* "miniwave_side_of_2n1_line" */
      {some_side_of_2n1_line,          &iden_tab[3]},            /* "couple_side_of_2n1_line" */
      {some_side_of_2n1_line,          &iden_tab[0]},            /* "antitandem_side_of_2n1_col" */
      {some_side_of_2n1_line,          &iden_tab[3]},            /* "tandem_side_of_2n1_col" */
      {cast_normal_or_whatever,        &iden_tab[1]},            /* "cast_normal" */
      {cast_normal_or_whatever,        &iden_tab[0]},            /* "cast_pushy" */
      {cast_normal_or_whatever,        &iden_tab[3]},            /* "cast_normal_or_warn" */

      {x14_once_rem_miniwave,          &iden_tab[3]},            /* "intlk_cast_normal_or_warn" */

      {opp_in_magic,                 (Const long int *) 0},      /* "lines_magic_miniwave" */
      {same_in_magic,                (Const long int *) 0},      /* "lines_magic_couple" */
      {once_rem_test,                  &iden_tab[2]},            /* "lines_once_rem_miniwave" */
      {once_rem_test,                  &iden_tab[0]},            /* "lines_once_rem_couple" */
      {same_in_pair,                 (Const long int *) 0},      /* "lines_tandem" */
      {opp_in_pair,                  (Const long int *) 0},      /* "lines_antitandem" */
      {columns_tandem,                 &iden_tab[0]},            /* "columns_tandem" */
      {columns_tandem,                 &iden_tab[1]},            /* "columns_antitandem" */
      {same_in_magic,                (Const long int *) 0},      /* "columns_magic_tandem" */
      {opp_in_magic,                 (Const long int *) 0},      /* "columns_magic_antitandem" */
      {once_rem_test,                  &iden_tab[0]},            /* "columns_once_rem_tandem" */
      {once_rem_test,                  &iden_tab[2]},            /* "columns_once_rem_antitandem" */
      {same_in_pair,                 (Const long int *) 0},      /* "columns_couple" */
      {opp_in_pair,                  (Const long int *) 0},      /* "columns_miniwave" */
      {x12_beau_or_miniwave,           &iden_tab[0]},            /* "1x2_beau_or_miniwave" */
      {x12_beau_or_miniwave,           &iden_tab[1]},            /* "1x2_beau_miniwave_or_warn" */
      {x12_beau_or_miniwave,           &iden_tab[2]},            /* "1x2_beau_miniwave_for_breaker" */
      {can_swing_left,               (Const long int *) 0},      /* "can_swing_left" */
      {x14_wheel_and_deal,           (Const long int *) 0},      /* "1x4_wheel_and_deal" */
      {x16_wheel_and_deal,           (Const long int *) 0},      /* "1x6_wheel_and_deal" */
      {x18_wheel_and_deal,           (Const long int *) 0},      /* "1x8_wheel_and_deal" */
      {cycle_and_wheel_1,            (Const long int *) 0},      /* "cycle_and_wheel_1" */
      {cycle_and_wheel_2,            (Const long int *) 0},      /* "cycle_and_wheel_2" */
      {vert1,                        (Const long int *) 0},      /* "vert1" */
      {vert2,                        (Const long int *) 0},      /* "vert2" */
      {inner_active_lines,           (Const long int *) 0},      /* "inner_active_lines" */
      {outer_active_lines,           (Const long int *) 0},      /* "outer_active_lines" */
      {judge_check_1x4,                       jr1x4},            /* "judge_is_right" */
      {judge_check_1x4,                       jl1x4},            /* "judge_is_left" */
      {judge_check_1x4,                       sr1x4},            /* "socker_is_right" */
      {judge_check_1x4,                       sl1x4},            /* "socker_is_left" */
      {judge_is_right_1x3,           (Const long int *) 0},      /* "judge_is_right_1x3" */
      {judge_is_left_1x3,            (Const long int *) 0},      /* "judge_is_left_1x3" */
      {socker_is_right_1x3,          (Const long int *) 0},      /* "socker_is_right_1x3" */
      {socker_is_left_1x3,           (Const long int *) 0},      /* "socker_is_left_1x3" */
      {judge_check_1x6,                       jr1x6},            /* "judge_is_right_1x6" */
      {judge_check_1x6,                       jl1x6},            /* "judge_is_left_1x6" */
      {judge_check_1x6,                       sr1x6},            /* "socker_is_right_1x6" */
      {judge_check_1x6,                       sl1x6},            /* "socker_is_left_1x6" */
      {judge_check_1x8,                       jr1x8},            /* "judge_is_right_1x8" */
      {judge_check_1x8,                       jl1x8},            /* "judge_is_left_1x8" */
      {judge_check_1x8,                       sr1x8},            /* "socker_is_right_1x8" */
      {judge_check_1x8,                       sl1x8},            /* "socker_is_left_1x8" */
      {in_out_roll_select, (Const long int *) &inroller_cw},     /* "inroller_is_cw" */
      {in_out_roll_select, (Const long int *) &magic_inroller_cw}, /* "magic_inroller_is_cw" */
      {in_out_roll_select, (Const long int *) &outroller_cw},    /* "outroller_is_cw" */
      {in_out_roll_select, (Const long int *) &magic_outroller_cw}, /* "magic_outroller_is_cw" */
      {in_out_roll_select, (Const long int *) &inroller_cw_2x3}, /* "inroller_is_cw_2x3" */
      {in_out_roll_select, (Const long int *) &magic_inroller_cw_2x3}, /* "magic_inroller_is_cw_2x3" */
      {in_out_roll_select, (Const long int *) &outroller_cw_2x3},/* "outroller_is_cw_2x3" */
      {in_out_roll_select, (Const long int *) &magic_outroller_cw_2x3}, /* "magic_outroller_is_cw_2x3" */
      {in_out_roll_select, (Const long int *) &inroller_2x6},   /* "inroller_is_cw_2x6" */
      {in_out_roll_select, (Const long int *) &outroller_2x6},  /* "outroller_is_cw_2x6" */
      {in_out_roll_select, (Const long int *) &inroller_2x8},   /* "inroller_is_cw_2x8" */
      {in_out_roll_select, (Const long int *) &outroller_2x8},  /* "outroller_is_cw_2x8" */
      {outposter_is_cw,              (Const long int *) 0},      /* "outposter_is_cw" */
      {outposter_is_ccw,             (Const long int *) 0},      /* "outposter_is_ccw" */
      {count_cw_people,                &iden_tab[0]},            /* "zero_cw_people" */
      {count_cw_people,                &iden_tab[1]},            /* "one_cw_person" */
      {count_cw_people,                &iden_tab[2]},            /* "two_cw_people" */
      {count_cw_people,                &iden_tab[3]},            /* "three_cw_people" */
      {check_tbone,            trnglspot_tboned_tab},            /* "nexttrnglspot_is_tboned" */
      {nextinttrnglspot_is_tboned,   (Const long int *) 0},    /* "nextinttrnglspot_is_tboned" */
      {check_tbone,             six2spot_tboned_tab},            /* "next62spot_is_tboned" */
      {check_tbone,            mag62spot_tboned_tab},          /* "next_magic62spot_is_tboned" */
      {next_galaxyspot_is_tboned,    (Const long int *) 0},     /* "next_galaxyspot_is_tboned" */
      {column_double_down,           (Const long int *) 0},      /* "column_double_down" */
      {apex_test,                      &iden_tab[1]},            /* "apex_test_1" */
      {apex_test,                      &iden_tab[2]},            /* "apex_test_2" */
      {apex_test,                      &iden_tab[3]},            /* "apex_test_3" */
      {boygirlp,                     boystuff_no_rh},            /* "boyp" */
      {boygirlp,                    girlstuff_no_rh},            /* "girlp" */
      {boygirlp,                        boystuff_rh},            /* "boyp_rh_slide_thru" */
      {boygirlp,                       girlstuff_rh},            /* "girlp_rh_slide_thru" */
      {roll_is_cw,                   (Const long int *) 0},      /* "roll_is_cw" */
      {roll_is_ccw,                  (Const long int *) 0},      /* "roll_is_ccw" */
      {x12_facing_other_sex,          boystuff_no_rh},           /* "x12_boy_facing_girl" */
      {x12_facing_other_sex,         girlstuff_no_rh},           /* "x12_girl_facing_boy" */
      {x22_facing_other_sex,          boystuff_no_rh},           /* "x22_boy_facing_girl" */
      {x22_facing_other_sex,         girlstuff_no_rh},           /* "x22_girl_facing_boy" */
      {directionp,   (Const long int *) direction_left},         /* "leftp" */
      {directionp,   (Const long int *) direction_right},        /* "rightp" */
      {directionp,   (Const long int *) direction_in},           /* "inp" */
      {directionp,   (Const long int *) direction_out},          /* "outp" */
      {directionp,   (Const long int *) direction_back},         /* "backp" */
      {directionp,   (Const long int *) direction_zigzag},       /* "zigzagp" */
      {directionp,   (Const long int *) direction_zagzig},       /* "zagzigp" */
      {directionp,   (Const long int *) direction_zigzig},       /* "zigzigp" */
      {directionp,   (Const long int *) direction_zagzag},       /* "zagzagp" */
      {directionp,   (Const long int *) direction_no_direction}, /* "no_dir_p" */
      {dmd_ctrs_rh,                    &iden_tab[0]},            /* "dmd_ctrs_rh" */
      {dmd_ctrs_rh,                    &iden_tab[1]},            /* "dmd_ctrs_lh" */
      {trngl_pt_rh,                  (Const long int *) 0},      /* "trngl_pt_rh" */
      {q_tag_check, (Const long int *) &q_tag_front_action},     /* "q_tag_front" */
      {q_tag_check, (Const long int *) &q_tag_back_action},      /* "q_tag_back" */
      {q_tag_check, (Const long int *) &q_line_front_action},    /* "q_line_front" */
      {q_tag_check, (Const long int *) &q_line_back_action}};    /* "q_line_back" */

int selector_preds = PREDS_BEFORE_THIS_POINT;
