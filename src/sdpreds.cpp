/* SD -- square dance caller's helper.

    Copyright (C) 1990-2002  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following function:
   selectp

and the following external variables:
   selector_used
   direction_used
   number_used
   mandatory_call_used
   pred_table     which is filled with pointers to the predicate functions
   selector_preds
*/

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"


/* These variables are external. */

long_boolean selector_used;
long_boolean direction_used;
long_boolean number_used;
long_boolean mandatory_call_used;


/* If a real person and a person under test are XOR'ed, the result AND'ed with this constant,
   and the low bits of that result examined, it will tell whether the person under test is real
   and facing in the same direction (result = 0), the opposite direction (result == 2), or whatever. */

#define DIR_MASK (BIT_PERSON | 3)

enum {
   ID1_PERM_ALL_ID = ID1_PERM_HEAD|ID1_PERM_SIDE|ID1_PERM_BOY|ID1_PERM_GIRL
};





extern long_boolean selectp(setup *ss, int place) THROW_DECL
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

   // Demand that the subject be real, or that
   // we can evaluate based on position alone.

   if (!(permpid1 & BIT_PERSON)) {
      // We can still do it for some selectors.  However, this violates
      // the rules about using the *original* centers in calls like
      // "rims trade back".  But, if the person isn't real, this is the best
      // we can do.
      //
      // We do this based on the stuff in the attribute tables.  If the
      // setup is larger than 8, we can't -- the tables become very complicated
      // based on the actual occupancy, and we are looking at an unoccupied
      // space.

      if (attr::slimit(ss) < 8) {
         const id_bit_table *ptr = setup_attrs[ss->kind].id_bit_table_ptr;

         if (ptr) {
            switch (current_options.who) {
            case selector_centers:
            case selector_ends:
            case selector_center2:
            case selector_outer6:
            case selector_verycenters:
            case selector_center6:
            case selector_outer2:
            case selector_veryends:
            case selector_ctrdmd:
            case selector_ctr_1x4:
            case selector_ctr_1x6:
            case selector_outer1x3s:
            case selector_center4:
            case selector_outerpairs:
               pid2 = ptr[place][0] &
                  BITS_TO_CLEAR &
                  ~(ID2_FACING|ID2_NOTFACING|ID2_LEAD|ID2_TRAILER|ID2_BEAU|ID2_BELLE);
               goto foo;
            }
         }
      }

      fail("Can't decide who are selected.");
   }

 foo:

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
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NHB)) == (ID1_PERM_HEAD|ID1_PERM_BOY))
            return TRUE;
         else if ((permpid1 & ID1_PERM_NHB) == ID1_PERM_NHB)
            return FALSE;
         break;
      case selector_headgirls:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NHG)) == (ID1_PERM_HEAD|ID1_PERM_GIRL))
            return TRUE;
         else if ((permpid1 & ID1_PERM_NHG) == ID1_PERM_NHG)
            return FALSE;
         break;
      case selector_sideboys:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NSB)) == (ID1_PERM_SIDE|ID1_PERM_BOY))
            return TRUE;
         else if ((permpid1 & ID1_PERM_NSB) == ID1_PERM_NSB)
            return FALSE;
         break;
      case selector_sidegirls:
         if      ((permpid1 & (ID1_PERM_ALL_ID|ID1_PERM_NSG)) == (ID1_PERM_SIDE|ID1_PERM_GIRL))
            return TRUE;
         else if ((permpid1 & ID1_PERM_NSG) == ID1_PERM_NSG)
            return FALSE;
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
         if (ss->kind == s1x6) {
            p2 = pid2 & (ID2_CTR2|ID2_OUTRPAIRS);
            if      (p2 == ID2_CTR2)      s = selector_center2;
            else if (p2 == ID2_OUTRPAIRS) s = selector_outerpairs;
            else break;
         }
         else if (ss->kind == s2x7 || ss->kind == sdblspindle) {
            if (current_options.who == selector_center2)
               return (pid2 & ID2_CTR2);
            else break;
         }
         else {
            p2 = pid2 & (ID2_CTR2|ID2_OUTR6);
            if      (p2 == ID2_CTR2)  s = selector_center2;
            else if (p2 == ID2_OUTR6) s = selector_outer6;
            else break;
         }
         goto eq_return;
      case selector_verycenters:    /* Gotta fix this stuff - use fall-through variable. */
         if (ss->kind == s1x6) {
            p2 = pid2 & (ID2_CTR2|ID2_OUTRPAIRS);
            if      (p2 == ID2_CTR2) return TRUE;
            else if (p2 == ID2_OUTRPAIRS) s = selector_outerpairs;
            else break;
         }
         else if (ss->kind == s2x7) {
            return (pid2 & ID2_CTR2);
         }
         else {
            p2 = pid2 & (ID2_CTR2|ID2_OUTR6);
            if      (p2 == ID2_CTR2) return TRUE;
            else if (p2 == ID2_OUTR6) s = selector_outer6;
            else break;
         }
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
         if (ss->kind == s1x6) {
            if      ((pid2 & (ID2_CTR2  |ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return TRUE;
            else if ((pid2 & (ID2_CTR2  |ID2_OUTRPAIRS)) == ID2_CTR2) return FALSE;
         }
         else {
            if      ((pid2 & (ID2_CTR4  |ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return TRUE;
            else if ((pid2 & (ID2_CTR4  |ID2_OUTRPAIRS)) == ID2_CTR4) return FALSE;
            else if ((pid2 & (ID2_CENTER|ID2_OUTRPAIRS)) == ID2_OUTRPAIRS) return TRUE;
            else if ((pid2 & (ID2_CENTER|ID2_OUTRPAIRS)) == ID2_CENTER) return FALSE;
         }

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


static const long int iden_tab[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
static const long int dbl_tab01[4] = {0, 1, FALSE, TRUE};
static const long int x22tabtand[3] = {3, 0, 1};
static const long int x22tabface[3] = {3, 2, 1};
static const long int x24tabtand[3] = {7, 0, 1};
static const long int x24tabface[3] = {7, 2, 1};
static const long int dbl_tab21[4] = {2, 1, TRUE, FALSE};

static const long int boystuff_no_rh[3]  = {ID1_PERM_BOY,  ID1_PERM_GIRL, 0};
static const long int girlstuff_no_rh[3] = {ID1_PERM_GIRL, ID1_PERM_BOY,  0};
static const long int boystuff_rh[3]     = {ID1_PERM_BOY,  ID1_PERM_GIRL, 1};
static const long int girlstuff_rh[3]    = {ID1_PERM_GIRL, ID1_PERM_BOY,  1};
static const long int semi_squeeze_tab[8] = {0xD, 0xE, 0x9, 0x9, 0x2, 0xD, 0x2, 0xE};


/* Here are the predicates.  They will get put into the array "pred_table". */

/* ARGSUSED */
static long_boolean someone_selected(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   return selectp(real_people, real_index ^ (*extra_stuff));
}

/* ARGSUSED */
static long_boolean sum_mod_selected(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   int otherindex = (*extra_stuff) - real_index;
   int size = attr::slimit(real_people)+1;
   if (otherindex >= size) otherindex -= size;
   return selectp(real_people, otherindex);
}

/* ARGSUSED */
static long_boolean sum_mod_selected_for_12p(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   int otherindex = (*extra_stuff) - real_index;
   if ((real_index % 6) >= 3) otherindex += 4;
   int size = attr::slimit(real_people)+1;
   if (otherindex >= size) otherindex -= size;
   return selectp(real_people, otherindex);
}

/* ARGSUSED */
static long_boolean plus_mod_selected(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   int otherindex = real_index + (*extra_stuff);
   int size = attr::slimit(real_people)+1;
   if (otherindex >= size) otherindex -= size;
   return selectp(real_people, otherindex);
}

/* ARGSUSED */
static long_boolean semi_squeezer_select(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   int other_index = ((northified_index ^ extra_stuff[northified_index&7]) + real_index - northified_index) & 0xF;

   return      (real_people->people[other_index].id1 & BIT_PERSON) &&
               selectp(real_people, other_index);
}

/* ARGSUSED */
static long_boolean unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   return !selectp(real_people, real_index);
}

/* ARGSUSED */
static long_boolean select_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return TRUE;

   return      (real_people->people[real_index ^ 1].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 1);
}

static int base_table[12] = {0, 0, 0, 3, 3, 3, 6, 6, 6, 9, 9, 9};

/* ARGSUSED */
static long_boolean select_near_select_or_phantom(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   int lim, base_person;

   if (!selectp(real_people, real_index))
      return FALSE;
   else if (current_options.who == selector_all || current_options.who == selector_everyone)
      return TRUE;

   if (attr::slimit(real_people) == 11) {
      base_person = base_table[real_index];
      lim = 2;
   }
   if (attr::slimit(real_people) == 15) {
      base_person = real_index & 4;
      lim = 3;
   }
   else {
      return
         !(real_people->people[real_index ^ 1].id1 & BIT_PERSON) ||
         selectp(real_people, real_index ^ 1);
   }

   for ( ; lim >= 0 ; lim--) {
      if ((real_people->people[base_person+lim].id1 & BIT_PERSON) &&
          !selectp(real_people, base_person+lim))
         return FALSE;
   }

   return TRUE;
}

/* ARGSUSED */
static long_boolean select_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return FALSE;

   return      !(real_people->people[real_index ^ 1].id1 & BIT_PERSON) ||
               !selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
static long_boolean unselect_near_select(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   if (selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_none) return FALSE;

   return      (real_people->people[real_index ^ 1].id1 & BIT_PERSON) &&
               selectp(real_people, real_index ^ 1);
}

/* ARGSUSED */
static long_boolean unselect_near_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   int lim, base_person;

   if (selectp(real_people, real_index))
      return FALSE;
   else if (current_options.who == selector_none)
      return TRUE;

   if (attr::slimit(real_people) == 11) {
      base_person = base_table[real_index];
      lim = 2;
   }
   if (attr::slimit(real_people) == 15) {
      base_person = real_index & 4;
      lim = 3;
   }
   else {
      return
         !(real_people->people[real_index ^ 1].id1 & BIT_PERSON) ||
         !selectp(real_people, real_index ^ 1);
   }

   for ( ; lim >= 0 ; lim--) {
      if ((real_people->people[base_person+lim].id1 & BIT_PERSON) &&
          selectp(real_people, base_person+lim))
         return FALSE;
   }

   return TRUE;
}

/* ARGSUSED */
static long_boolean select_once_rem_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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
static long_boolean select_once_rem_from_unselect(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   if (!selectp(real_people, real_index)) return FALSE;
   if (current_options.who == selector_all || current_options.who == selector_everyone)
      return FALSE;

   int other_index;

   if (real_people->kind == s2x6) {
      int q = (real_index >= 6) ? real_index-6 : real_index;
      other_index = real_index + 2;
      if (q >= 4) other_index = real_index - 2;
      else if (q >= 2) {
         // We demand BOTH outer people be (nonexistent or unselected).
         if ((real_people->people[real_index - 2].id1 & BIT_PERSON) &&
             selectp(real_people, real_index - 2))
            return FALSE;
      }
   }
   else if (real_people->kind == s2x4)
      other_index = real_index ^ 2;
   else
      other_index = real_index ^ 3;

   return !(real_people->people[other_index].id1 & BIT_PERSON) ||
          !selectp(real_people, other_index);
}

/* ARGSUSED */
static long_boolean unselect_once_rem_from_select(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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
static long_boolean select_and_roll_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   return selectp(real_people, real_index) && (real_people->people[real_index].id1 & ROLL_DIRMASK) == ROLL_IS_R;
}

/* ARGSUSED */
static long_boolean select_and_roll_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   return selectp(real_people, real_index) && (real_people->people[real_index].id1 & ROLL_DIRMASK) == ROLL_IS_L;
}

/* ARGSUSED */
static long_boolean always(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   return TRUE;
}

/* ARGSUSED */
static long_boolean x22_cpltest(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean facing_test(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int other_index = real_index ^ extra_stuff[(real_direction << 1) & 2];
   return ((real_people->people[real_index].id1 ^
            real_people->people[other_index].id1) & DIR_MASK) == (uint32) extra_stuff[1];
}

/* ARGSUSED */
static long_boolean cols_someone_in_front(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean x14_once_rem_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean x14_once_rem_couple(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   switch (real_people->cmd.cmd_assume.assumption) {
   case cr_wave_only: case cr_li_lo: case cr_1fl_only: return TRUE;
   case cr_2fl_only: case cr_magic_only: return FALSE;
   }

   return ((real_people->people[real_index].id1 ^
            real_people->people[real_index ^ 3].id1) & DIR_MASK) == 0;
}

/* ARGSUSED */
static long_boolean lines_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   switch (real_people->cmd.cmd_assume.assumption) {
   case cr_wave_only: case cr_magic_only:
      return TRUE;
   case cr_2fl_only:
      return FALSE;
   default:
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;
      return ((this_person ^ other_person) & DIR_MASK) == 2;
   }
}

/* ARGSUSED */
static long_boolean lines_couple(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   switch (real_people->cmd.cmd_assume.assumption) {
   case cr_wave_only: case cr_magic_only:
      return FALSE;
   case cr_2fl_only:
      return TRUE;
   default:
      int this_person = real_people->people[real_index].id1;
      int other_person = real_people->people[real_index ^ 1].id1;
      if (real_people->kind == s1x6 && real_index >= 2)
         other_person = real_people->people[7 - real_index].id1;
      return ((this_person ^ other_person) & DIR_MASK) == 0;
   }
}

static const long int tab_mwv_in_3n1[8]  = {2, 0, 0, 4, 3, 7, 2};
static const long int tab_cpl_in_3n1[8]  = {0, 2, 0, 4, 3, 7, 2};
static const long int tab_mwv_out_3n1[8] = {2, 0, 1, 5, 2, 6, 3};
static const long int tab_cpl_out_3n1[8] = {0, 2, 1, 5, 2, 6, 3};

/* ARGSUSED */
static long_boolean check_3n1_setup(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean some_side_of_2n1_line(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean cast_normal_or_whatever(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean columns_tandem(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   uint32 this_person;
   int other_index;

   if (real_people->kind == s_qtag) {
      if (real_index == 2 || real_index == 6) return FALSE;  // Wings of qtag always fail.
   }
   else if (real_people->kind == s_spindle) {
      if (real_index == 3 || real_index == 7) return FALSE; // Or points of spindle.
   }
   else {
      switch (real_people->cmd.cmd_assume.assumption) {
      case cr_wave_only: case cr_2fl_only: return extra_stuff[0] ^ 1;
      case cr_magic_only: case cr_li_lo: return extra_stuff[0];
      }
   }

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
   else if (real_people->kind == s_spindle) {
      switch (real_index) {
      case 0: other_index = 1; break;
      case 1: other_index = (real_direction & 2) ? 0: 2; break;
      case 2: other_index = 1; break;
      case 4: other_index = 5; break;
      case 5: other_index = (real_direction & 2) ? 6: 4; break;
      case 6: other_index = 5; break;
      }
   }

   return ((this_person ^ real_people->people[other_index].id1) & DIR_MASK) ==
      ((uint32) extra_stuff[0] << 1);
}

/* ARGSUSED */
static long_boolean same_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person;
   int other_person;

   if (real_people->kind == s_bone) {
      // This is only valid for the ends; the documentation says so.
      other_person = real_people->people[real_index ^ 5].id1;
   }
   else {
      if (real_people->cmd.cmd_assume.assump_col == 1) {
         switch (real_people->cmd.cmd_assume.assumption) {
         case cr_2fl_only: case cr_li_lo: return TRUE;
         case cr_wave_only: case cr_magic_only: return FALSE;
         }
      }

      other_person = real_people->people[real_index ^ 7].id1;
   }

   this_person = real_people->people[real_index].id1;

   return ((this_person ^ other_person) & DIR_MASK) == 0;
}

/* ARGSUSED */
static long_boolean opp_in_pair(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person;
   int other_person;

   if (real_people->kind == s_bone) {
      // This is only valid for the ends; the documentation says so.
      other_person = real_people->people[real_index ^ 5].id1;
   }
   else {
      if (real_people->cmd.cmd_assume.assump_col == 1) {
         switch (real_people->cmd.cmd_assume.assumption) {
         case cr_wave_only: case cr_magic_only: return TRUE;
         case cr_2fl_only: case cr_li_lo: return FALSE;
         }
      }

      other_person = real_people->people[real_index ^ 7].id1;
   }

   this_person = real_people->people[real_index].id1;

   return ((this_person ^ other_person) & DIR_MASK) == 2;
}

/* ARGSUSED */
static long_boolean opp_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person;
   int other_person;
   int other_index;

   if (real_people->cmd.cmd_assume.assump_col == 1) {
      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_wave_only: case cr_li_lo: return TRUE;
         case cr_2fl_only: case cr_magic_only: return FALSE;
      }
   }

   this_person = real_people->people[real_index].id1;
   other_index = real_index ^ 6;
   if (real_people->kind == s2x3) {
      other_index = 4 >> (((real_index+5) >> 3) << 1);
      if (real_index == 1)
         other_index = 3 + (this_person & 2);
      else if (real_index == 4)
         other_index = 2 - (this_person & 2);
   }
   other_person = real_people->people[other_index].id1;
   return ((this_person ^ other_person) & DIR_MASK) == 2;
}

/* ARGSUSED */
static long_boolean same_in_magic(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person;
   int other_person;
   int other_index;

   if (real_people->cmd.cmd_assume.assump_col == 1) {
      switch (real_people->cmd.cmd_assume.assumption) {
         case cr_2fl_only: case cr_magic_only: return TRUE;
         case cr_wave_only: case cr_li_lo: return FALSE;
      }
   }

   this_person = real_people->people[real_index].id1;
   other_index = real_index ^ 6;
   if (real_people->kind == s2x3) {
      other_index = 4 >> (((real_index+5) >> 3) << 1);
      if (real_index == 1)
         other_index = 3 + (this_person & 2);
      else if (real_index == 4)
         other_index = 2 - (this_person & 2);
   }
   other_person = real_people->people[other_index].id1;
   return ((this_person ^ other_person) & DIR_MASK) == 0;
}

/* ARGSUSED */
static long_boolean once_rem_test(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 2].id1;
   return ((this_person ^ other_person) & DIR_MASK) == extra_stuff[0];
}

/* ARGSUSED */
static long_boolean x12_beau_or_miniwave(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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

static const long int swingleft_1x3dmd[8] = {-1, 0, 1, -1, 5, 6, -1, 3};
static const long int swingleft_1x4[4] = {-1, 0, 3, 1};
static const long int swingleft_deep2x1dmd[10] = {-1, 0, -1, 1, 3, 6, 8, -1, 9, -1};
static const long int swingleft_wqtag[10] = {-1, -1, 3, 4, 9, -1, -1, -1, 7, 8};

static long_boolean can_swing_left(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int t;

   switch (real_people->kind) {
   case s1x4:
      t = swingleft_1x4[northified_index];
      break;
   case s1x3dmd:
      t = swingleft_1x3dmd[northified_index];
      break;
   case swqtag:
      t = swingleft_wqtag[northified_index];
      break;
   case sdeep2x1dmd:
      t = swingleft_deep2x1dmd[northified_index];
      break;
   default:
      return FALSE;
   }

   if (t < 0) return FALSE;
   if (northified_index != real_index) {
      int size = attr::slimit(real_people)+1;
      t -= size >> 1;
      if (t < 0) t += size;
   }

   return ((real_people->people[real_index].id1 ^ real_people->people[t].id1) &
           DIR_MASK) == 2;
}


/* Test for wheel and deal to be done 2FL-style, or beau side of 1FL.  Returns
   false if belle side of 1FL.  Raises a warning if wheel and deal can't be done,
   and opts for L2FL. */
/* ARGSUSED */
static long_boolean x14_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean x16_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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
static long_boolean x18_wheel_and_deal(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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
static long_boolean cycle_and_wheel_1(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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
static long_boolean cycle_and_wheel_2(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   switch (real_people->cmd.cmd_assume.assumption) {
      case cr_1fl_only: case cr_2fl_only: return TRUE;
      case cr_wave_only:  return (northified_index != 2);
      case cr_magic_only: return (northified_index == 2);
   }

   int other_person = real_people->people[(real_index ^ 2) & (~1)].id1;

   /* Here we default to the "non-colliding" version of the call if the
      opposite end of our line doesn't exist. */
   if (!other_person) {
      warn(warn__opt_for_no_collision);
      return (northified_index == 1);
   }

   /* See if he faces the same way as myself. */
   if (((other_person ^ real_people->people[real_index].id1) & 2))
      return FALSE;

   return TRUE;
}

static long_boolean vert1(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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

static long_boolean vert2(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean inner_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean outer_active_lines(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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

static long_boolean judge_is_right_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 0)       /* judge exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 2));         /* we do not have another judge to my left */
}

static long_boolean judge_is_left_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 2)           /* judge exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 0));     /* we do not have another judge to my right */
}

static long_boolean socker_is_right_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) == 2)       /* socker exists to my right */
         &&
      (((real_people->people[f].id1 ^ this_person) & 013) != 0));         /* we do not have another socker to my left */
}

static long_boolean socker_is_left_1x3(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int f = this_person & 2;

   return(
      (((real_people->people[f].id1 ^ this_person) & 013) == 0)           /* socker exists to my left */
         &&
      (((real_people->people[f ^ 2].id1 ^ this_person) & 013) != 2));     /* we do not have another socker to my right */
}

static const long int jr1x4[4]  = {1, 0, 0, 2};
static const long int sl1x4[4]  = {1, 2, 0, 2};
static const long int jl1x4[4]  = {0, 2, 2, 0};
static const long int sr1x4[4]  = {0, 0, 2, 0};

static const long int jr1x6[4]  = {1, 0, 0, 2};
static const long int sl1x6[4]  = {1, 2, 0, 2};
static const long int jl1x6[4]  = {0, 2, 2, 0};
static const long int sr1x6[4]  = {0, 0, 2, 0};

static const long int jr1x8[4]  = {1, 0, 0, 2};
static const long int sl1x8[4]  = {1, 2, 0, 2};
static const long int jl1x8[4]  = {0, 2, 2, 0};
static const long int sr1x8[4]  = {0, 0, 2, 0};

/* ARGSUSED */
static long_boolean judge_check_1x4(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean judge_check_1x6(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean judge_check_1x8(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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

// A remnant.
#define C_const

static C_const veryshort inroll_directions[24] = {
   012, 012, 012, 012, 010, 010, 010, 010,
      3,  3,  3,  3,  7,  7,  7,  7,
      0,  0,  0,  0,  4,  4,  4,  4};

static C_const veryshort magic_inroll_directions[24] = {
   012, 010, 010, 012, 010, 012, 012, 010,
      3,  7,  7,  3,  7,  3,  3,  7,
      0,  4,  4,  0,  4,  0,  0,  4};

static C_const veryshort inroll_directions_2x3[18] = {
   012, 012, 012, 010, 010, 010,
      2,  2,  2,  5,  5,  5,
      0,  0,  0,  3,  3,  3};

static C_const veryshort magic_inroll_directions_2x3[18] = {
   012, 010, 012, 010, 012, 010,
      2,  5,  2,  5,  2,  5,
      0,  3,  0,  3,  0,  3};

static C_const veryshort inroll_directions_2x6[36] = {
   012, 012, 012, 012, 012, 012, 010, 010, 010, 010, 010, 010,
      5,  5,  5,  5,  5,  5, 11, 11, 11, 11, 11, 11,
      0,  0,  0,  0,  0,  0,  6,  6,  6,  6,  6,  6};

static C_const veryshort inroll_directions_2x8[48] = {
   012, 012, 012, 012, 012, 012, 012, 012, 010, 010, 010, 010, 010, 010, 010, 010,
      7,  7,  7,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15,
      0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8,  8,  8,  8,  8};

enum inroll_assume_test {
   ira__no_wave,
   ira__sixes,
   ira__eights,
   ira__gen
};


typedef struct {
   C_const veryshort *directions;
   C_const long int code;
   C_const inroll_assume_test ira;
} inroll_action;


static C_const inroll_action inroller_cw            = {inroll_directions,           0, ira__gen};
static C_const inroll_action outroller_cw           = {inroll_directions,           1, ira__gen};
static C_const inroll_action magic_inroller_cw      = {magic_inroll_directions,     2, ira__gen};
static C_const inroll_action magic_outroller_cw     = {magic_inroll_directions,     3, ira__gen};
static C_const inroll_action inroller_cw_2x3        = {inroll_directions_2x3,       0, ira__no_wave};
static C_const inroll_action outroller_cw_2x3       = {inroll_directions_2x3,       1, ira__no_wave};
static C_const inroll_action magic_inroller_cw_2x3  = {magic_inroll_directions_2x3, 2, ira__no_wave};
static C_const inroll_action magic_outroller_cw_2x3 = {magic_inroll_directions_2x3, 3, ira__no_wave};
static C_const inroll_action inroller_2x6           = {inroll_directions_2x6,       0, ira__sixes};
static C_const inroll_action outroller_2x6          = {inroll_directions_2x6,       1, ira__sixes};
static C_const inroll_action inroller_2x8           = {inroll_directions_2x8,       0, ira__eights};
static C_const inroll_action outroller_2x8          = {inroll_directions_2x8,       1, ira__eights};



/* ARGSUSED */
static long_boolean in_out_roll_select(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   const inroll_action *thing = (const inroll_action *) extra_stuff;
   const veryshort *directions = thing->directions;
   int code = thing->code;

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

   // "Yes_roll_direction" is the facing direction that constitutes what we are
   // looking for (inroller or outroller as the case may be).

   int yes_roll_direction = directions[real_index];
   if (code&1) yes_roll_direction = 022 - yes_roll_direction;
   int no_roll_direction = 022 - yes_roll_direction;

   int size = attr::slimit(real_people)+1;

   int cw_idx  = directions[real_index+size];
   int ccw_idx = directions[real_index+2*size];

   int cw_end =  real_people->people[cw_idx].id1 & 017;
   int ccw_end = real_people->people[ccw_idx].id1 & 017;

   if (
       // cw_end exists and is proper, and we do not have ccw_end proper also
       (cw_end == yes_roll_direction && ccw_end != yes_roll_direction) ||
       // or if ccw_end exists and is improper, and cw_end is a phantom
       (ccw_end == no_roll_direction && cw_end == 0))
      return TRUE;
   else if (
            // ccw_end exists and is proper, and we do not have cw_end proper also
            (ccw_end == yes_roll_direction && cw_end != yes_roll_direction) ||
            // or if cw_end exists and is improper, and ccw_end is a phantom
            (cw_end == no_roll_direction && ccw_end == 0))
      return FALSE;
   else {
      char *errmsg;
      switch (code&3) {
         case 0: errmsg = "Can't find end looking in.";        break;
         case 1: errmsg = "Can't find end looking out.";       break;
         case 2: errmsg = "Can't find magic end looking in.";  break;
         case 3: errmsg = "Can't find magic end looking out."; break;
      }
      fail(errmsg);
      return FALSE;
   }
}


/* ARGSUSED */
static long_boolean outposter_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   uint32 outroll_direction;
   uint32 cw_dir;

   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) != 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) != 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_1fl_only)
      return (northified_index & 4) == 0;
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
static long_boolean outposter_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   uint32 inroll_direction;

   if (real_people->cmd.cmd_assume.assumption == cr_wave_only)
      return ((northified_index ^ (northified_index >> 2)) & 1) == 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_2fl_only)
      return ((northified_index ^ (northified_index >> 1)) & 2) == 0;
   else if (real_people->cmd.cmd_assume.assumption == cr_1fl_only)
      return FALSE;
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
static long_boolean count_cw_people(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int i;
   int count = 0;

   int limit = (attr::slimit(real_people)+1) >> 1;
   if (real_index >= limit) limit <<= 1;

   for (i=real_index+1; i<limit ; i++) {
      if (real_people->people[i].id1) count++;
   }

   return (count == (*extra_stuff));
}


/* -3 means error, -2 means return FALSE, -1 does not occur, and >= 0 means test that person. */

/* ARGSUSED */
static long_boolean check_tbone(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
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
         case cr_diamond_like:
            return TRUE;
         case cr_qtag_like:
            return FALSE;
         case cr_jleft:
         case cr_jright:
         case cr_ijleft:
         case cr_ijright:
            return real_people->cmd.cmd_assume.assump_col != 0;
         }
      }
      else if (real_people->kind == s_trngl) {
         switch (real_people->cmd.cmd_assume.assumption) {
         case cr_jleft: case cr_jright:
            // If "assume normal diamonds" or "assume facing diamonds" is present,
            // the spot is t-boned for person zero.
            if (real_people->cmd.cmd_assume.assump_col == 4)
               return real_index==0;
            break;
         case cr_tall6:
            return real_index==0;
         default:
            if (real_index==0) {
               // Try looking at the other base person!
               uint32 zz = real_people->people[z^3].id1;
               if (zz & BIT_PERSON) {
                  warn(warn__opt_for_not_tboned_base);
                  return (zz ^ real_people->people[real_index].id1) & 1;
               }
            }
         }
      }
   }
   fail("Can't determine where to go or which way to face.");
   return FALSE;
}

static const long int trnglspot_tboned_tab[12] = {-3,  2, -3,  1,    -2, -2,  0, 0,    -2,  0,  0, -2};
static const long int six2spot_tboned_tab[24]  = {-2, -2, -2, -2,    -3,  2, -3, 0,    -2, -2, -2, -2,    -2, -2, -2, -2,    -3, 3, -3, 5,    -2, -2, -2, -2};
static const long int mag62spot_tboned_tab[24] = {-3, -2, -3, -2,    -3,  3, -3, 5,    -3, -2, -3, -2,    -3, -2, -3, -2,    -3, 2, -3, 0,    -3, -2, -3, -2};

/* ARGSUSED */
/* We don't use this procedure!!!!
static long_boolean nexttrnglspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   return check_tbone(real_people, real_index, real_direction,
                      northified_index, trnglspot_tboned_tab);
}
*/

/* ARGSUSED */
static long_boolean nextinttrnglspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   static const long int bb[24] = {-2, -2, 4, 4,     -3, 3, -3, 5,     -2, 4, 4, -2,     1, 1, -2, -2,     -3, 2, -3, 0,     1, -2, -2, 1};
   static const long int cc[24] = {2, 2, -2, -2,      5, -2, -2, 5,    0, -3, 4, -3,     -2, -2, 5, 5,     -2, 2, 2, -2,     1, -3, 3, -3};

   return check_tbone(real_people, real_index, real_direction, northified_index,
         (real_people->kind == s_short6) ? bb : cc);
}

/* ARGSUSED */
static long_boolean next_galaxyspot_is_tboned(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   static const long int aa[32] = {1, -3, 7, -3,     2, 2, 0, 0,     -3, 3, -3, 1,     2, 4, 4, 2,    3, -3, 5, -3,     4, 4, 6, 6,    -3, 5, -3, 7,      0, 6, 6, 0};

   /* We always return true for centers.  That way
      the centers can reverse flip a galaxy even if the
      next point does not exist.  Maybe this isn't a good way
      to do it, and we need another predicate.  Sigh. */

   if (real_index & 1) return TRUE;

   return check_tbone(real_people, real_index, real_direction, northified_index, aa);
}

/* ARGSUSED */
static long_boolean column_double_down(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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
static long_boolean apex_test(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   uint32 unmoving_end = ((real_people->people[0].id1+NROLL_BIT) & (NROLL_BIT*2)) ? 2 : 0;
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
static long_boolean boygirlp(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   /* If this is a slide thru from a miniwave that is not right-handed, raise a warning. */
   if (extra_stuff[2] && northified_index != 0)
      warn(warn__tasteless_slide_thru);

   return (real_people->people[real_index].id1 & extra_stuff[0]) != 0;
}

/* ARGSUSED */
static long_boolean roll_is_cw(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   return (real_people->people[real_index].id1 & ROLL_DIRMASK) == ROLL_IS_R;
}

/* ARGSUSED */
static long_boolean roll_is_ccw(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   return (real_people->people[real_index].id1 & ROLL_DIRMASK) == ROLL_IS_L;
}

/* ARGSUSED */
static long_boolean x12_with_other_sex(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ 1].id1;
   return (this_person & extra_stuff[0]) && (other_person & extra_stuff[1]);
}

/* ARGSUSED */
static long_boolean x22_facing_other_sex(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   int this_person = real_people->people[real_index].id1;
   int other_person = real_people->people[real_index ^ (((real_direction << 1) & 2) ^ 3)].id1;
   return (this_person & extra_stuff[0]) && (other_person & extra_stuff[1]);
}


static direction_kind direction_list[] = {
   direction_left,
   direction_right,
   direction_in,
   direction_out,
   direction_back,
   direction_zigzag,
   direction_zagzig,
   direction_zigzig,
   direction_zagzag,
   direction_no_direction};


/* ARGSUSED */
static long_boolean directionp(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
{
   direction_used = TRUE;
   return current_options.where == direction_list[extra_stuff[0]];
}



/* ARGSUSED */
static long_boolean dmd_ctrs_rh(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   if (real_people->cmd.cmd_assume.assump_col == 0) {
      switch (real_people->cmd.cmd_assume.assumption) {
      case cr_jleft:
         if (real_people->cmd.cmd_assume.assump_both == 2) return !(*extra_stuff);
         else if (real_people->cmd.cmd_assume.assump_both == 1) return (*extra_stuff);
         break;
      case cr_jright:
         if (real_people->cmd.cmd_assume.assump_both == 2) return (*extra_stuff);
         else if (real_people->cmd.cmd_assume.assump_both == 1) return !(*extra_stuff);
         break;
      }
   }

   assumption_thing tt;
   long_boolean booljunk;

   tt.assumption = cr_dmd_ctrs_mwv;
   tt.assump_col = 0;
   tt.assump_both = 1;
   tt.assump_negate = 0;
   tt.assump_live = 0;
   if (verify_restriction(real_people, tt, FALSE, &booljunk) == restriction_passes)
      return !(*extra_stuff);
   tt.assump_both = 2;
   if (verify_restriction(real_people, tt, FALSE, &booljunk) == restriction_passes)
      return (*extra_stuff);

   fail("Can't determine handedness.");
   return FALSE;
}

/* ARGSUSED */
static long_boolean trngl_pt_rh(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff) THROW_DECL
{
   if ((real_people->people[0].id1 & d_mask) == d_west)
      return TRUE;
   else if ((real_people->people[0].id1 & d_mask) == d_east)
      return FALSE;

   fail("Can't determine handedness of triangle point.");
   return FALSE;
}


typedef struct {
   C_const long int ctr_action;    /* -1 for TRUE, -2 for FALSE, else required direction xor. */
   C_const long int end_action;
   C_const long int bbbbb;
} simple_qtag_action;

typedef struct {
   C_const simple_qtag_action if_jleft;
   C_const simple_qtag_action if_jright;
   C_const simple_qtag_action if_ijleft;
   C_const simple_qtag_action if_ijright;
   C_const simple_qtag_action if_tag;
   C_const simple_qtag_action if_line;
   C_const simple_qtag_action none;
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
static long_boolean q_tag_check(setup *real_people, int real_index,
   int real_direction, int northified_index, const long int *extra_stuff)
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


// BEWARE!!  This list must track the array "predtab" in dbcomp.c

/* The first several of these take a predicate.
   Any call that uses one of these predicates will have its "need_a_selector"
   flag set during initialization.  We set the variable "selector_preds" to contain
   the number of such predicates. */

predicate_descriptor pred_table[] = {
      {someone_selected,               &iden_tab[0]},            /* "select" */
      {unselect,                     (const long int *) 0},      /* "unselect" */
      {select_near_select,           (const long int *) 0},      /* "select_near_select" */
      {select_near_select_or_phantom,(const long int *) 0},      /* "select_near_select_or_phantom" */
      {select_near_unselect,         (const long int *) 0},      /* "select_near_unselect" */
      {unselect_near_select,         (const long int *) 0},      /* "unselect_near_select" */
      {unselect_near_unselect,       (const long int *) 0},      /* "unselect_near_unselect" */
      {select_once_rem_from_select,  (const long int *) 0},      /* "select_once_rem_from_select" */
      {someone_selected,               &iden_tab[2]},            /* "conc_from_select" */
      {someone_selected,               &iden_tab[3]},            /* "other_spindle_cw_select" */
      {someone_selected,               &iden_tab[4]},            /* "grand_conc_from_select" */
      {someone_selected,               &iden_tab[5]},            /* "other_diamond_point_select" */
      {someone_selected,               &iden_tab[6]},            /* "other_spindle_ckpt_select" */
      {someone_selected,               &iden_tab[7]},            /* "pair_person_select" */
      {sum_mod_selected,               &iden_tab[5]},            /* "person_select_sum5" */
      {sum_mod_selected,               &iden_tab[8]},            /* "person_select_sum8" */
      {sum_mod_selected,              &iden_tab[11]},            /* "person_select_sum11" */
      {sum_mod_selected,              &iden_tab[13]},            /* "person_select_sum13" */
      {sum_mod_selected,              &iden_tab[15]},            /* "person_select_sum15" */
      {plus_mod_selected,             &iden_tab[4]},             /* "person_select_plus4" */
      {plus_mod_selected,             &iden_tab[6]},             /* "person_select_plus6" */
      {plus_mod_selected,             &iden_tab[8]},             /* "person_select_plus8" */
      {plus_mod_selected,             &iden_tab[12]},            /* "person_select_plus12" */
      {sum_mod_selected_for_12p,      &iden_tab[15]},            /* "person_select12_sum15" */
      {semi_squeezer_select,       semi_squeeze_tab},            /* "semi_squeezer_select" */
      {select_once_rem_from_unselect,(const long int *) 0},      /* "select_once_rem_from_unselect" */
      {unselect_once_rem_from_select,(const long int *) 0},      /* "unselect_once_rem_from_select" */
      {select_and_roll_is_cw,        (const long int *) 0},      /* "select_and_roll_is_cw" */
      {select_and_roll_is_ccw,       (const long int *) 0},      /* "select_and_roll_is_ccw" */
/* End of predicates that force use of selector. */
#define PREDS_BEFORE_THIS_POINT 23
      {always,                       (const long int *) 0},      /* "always" */
      {x22_cpltest,                    dbl_tab21},               /* "x22_miniwave" */
      {x22_cpltest,                    dbl_tab01},               /* "x22_couple" */
      {facing_test,                    x22tabface},              /* "x22_facing_someone" */
      {facing_test,                    x22tabtand},              /* "x22_tandem_with_someone" */
      {facing_test,                    x24tabface},              /* "x24_facing_someone" */
      {facing_test,                    x24tabtand},              /* "x24_tandem_with_someone" */
      {cols_someone_in_front,        (const long int *) 0},      /* "columns_someone_in_front" */
      {x14_once_rem_miniwave,         &iden_tab[1]},             /* "x14_once_rem_miniwave" */
      {x14_once_rem_couple,          (const long int *) 0},      /* "x14_once_rem_couple" */
      {lines_miniwave,               (const long int *) 0},      /* "lines_miniwave" */
      {lines_couple,                 (const long int *) 0},      /* "lines_couple" */
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
      {opp_in_magic,                 (const long int *) 0},      /* "lines_magic_miniwave" */
      {same_in_magic,                (const long int *) 0},      /* "lines_magic_couple" */
      {once_rem_test,                  &iden_tab[2]},            /* "lines_once_rem_miniwave" */
      {once_rem_test,                  &iden_tab[0]},            /* "lines_once_rem_couple" */
      {same_in_pair,                 (const long int *) 0},      /* "lines_tandem" */
      {opp_in_pair,                  (const long int *) 0},      /* "lines_antitandem" */
      {columns_tandem,                 &iden_tab[0]},            /* "columns_tandem" */
      {columns_tandem,                 &iden_tab[1]},            /* "columns_antitandem" */
      {same_in_magic,                (const long int *) 0},      /* "columns_magic_tandem" */
      {opp_in_magic,                 (const long int *) 0},      /* "columns_magic_antitandem" */
      {once_rem_test,                  &iden_tab[0]},            /* "columns_once_rem_tandem" */
      {once_rem_test,                  &iden_tab[2]},            /* "columns_once_rem_antitandem" */
      {same_in_pair,                 (const long int *) 0},      /* "columns_couple" */
      {opp_in_pair,                  (const long int *) 0},      /* "columns_miniwave" */
      {x12_beau_or_miniwave,           &iden_tab[0]},            /* "1x2_beau_or_miniwave" */
      {x12_beau_or_miniwave,           &iden_tab[1]},            /* "1x2_beau_miniwave_or_warn" */
      {x12_beau_or_miniwave,           &iden_tab[2]},            /* "1x2_beau_miniwave_for_breaker" */
      {can_swing_left,               (const long int *) 0},      /* "can_swing_left" */
      {x14_wheel_and_deal,           (const long int *) 0},      /* "1x4_wheel_and_deal" */
      {x16_wheel_and_deal,           (const long int *) 0},      /* "1x6_wheel_and_deal" */
      {x18_wheel_and_deal,           (const long int *) 0},      /* "1x8_wheel_and_deal" */
      {cycle_and_wheel_1,            (const long int *) 0},      /* "cycle_and_wheel_1" */
      {cycle_and_wheel_2,            (const long int *) 0},      /* "cycle_and_wheel_2" */
      {vert1,                        (const long int *) 0},      /* "vert1" */
      {vert2,                        (const long int *) 0},      /* "vert2" */
      {inner_active_lines,           (const long int *) 0},      /* "inner_active_lines" */
      {outer_active_lines,           (const long int *) 0},      /* "outer_active_lines" */
      {judge_check_1x4,                       jr1x4},            /* "judge_is_right" */
      {judge_check_1x4,                       jl1x4},            /* "judge_is_left" */
      {judge_check_1x4,                       sr1x4},            /* "socker_is_right" */
      {judge_check_1x4,                       sl1x4},            /* "socker_is_left" */
      {judge_is_right_1x3,           (const long int *) 0},      /* "judge_is_right_1x3" */
      {judge_is_left_1x3,            (const long int *) 0},      /* "judge_is_left_1x3" */
      {socker_is_right_1x3,          (const long int *) 0},      /* "socker_is_right_1x3" */
      {socker_is_left_1x3,           (const long int *) 0},      /* "socker_is_left_1x3" */
      {judge_check_1x6,                       jr1x6},            /* "judge_is_right_1x6" */
      {judge_check_1x6,                       jl1x6},            /* "judge_is_left_1x6" */
      {judge_check_1x6,                       sr1x6},            /* "socker_is_right_1x6" */
      {judge_check_1x6,                       sl1x6},            /* "socker_is_left_1x6" */
      {judge_check_1x8,                       jr1x8},            /* "judge_is_right_1x8" */
      {judge_check_1x8,                       jl1x8},            /* "judge_is_left_1x8" */
      {judge_check_1x8,                       sr1x8},            /* "socker_is_right_1x8" */
      {judge_check_1x8,                       sl1x8},            /* "socker_is_left_1x8" */
      {in_out_roll_select, (const long int *) &inroller_cw},     /* "inroller_is_cw" */
      {in_out_roll_select, (const long int *) &magic_inroller_cw}, /* "magic_inroller_is_cw" */
      {in_out_roll_select, (const long int *) &outroller_cw},    /* "outroller_is_cw" */
      {in_out_roll_select, (const long int *) &magic_outroller_cw}, /* "magic_outroller_is_cw" */
      {in_out_roll_select, (const long int *) &inroller_cw_2x3}, /* "inroller_is_cw_2x3" */
      {in_out_roll_select, (const long int *) &magic_inroller_cw_2x3}, /* "magic_inroller_is_cw_2x3" */
      {in_out_roll_select, (const long int *) &outroller_cw_2x3},/* "outroller_is_cw_2x3" */
      {in_out_roll_select, (const long int *) &magic_outroller_cw_2x3}, /* "magic_outroller_is_cw_2x3" */
      {in_out_roll_select, (const long int *) &inroller_2x6},   /* "inroller_is_cw_2x6" */
      {in_out_roll_select, (const long int *) &outroller_2x6},  /* "outroller_is_cw_2x6" */
      {in_out_roll_select, (const long int *) &inroller_2x8},   /* "inroller_is_cw_2x8" */
      {in_out_roll_select, (const long int *) &outroller_2x8},  /* "outroller_is_cw_2x8" */
      {outposter_is_cw,              (const long int *) 0},      /* "outposter_is_cw" */
      {outposter_is_ccw,             (const long int *) 0},      /* "outposter_is_ccw" */
      {count_cw_people,                &iden_tab[0]},            /* "zero_cw_people" */
      {count_cw_people,                &iden_tab[1]},            /* "one_cw_person" */
      {count_cw_people,                &iden_tab[2]},            /* "two_cw_people" */
      {count_cw_people,                &iden_tab[3]},            /* "three_cw_people" */
      {check_tbone,            trnglspot_tboned_tab},            // "nexttrnglspot_is_tboned"
      {nextinttrnglspot_is_tboned,   (const long int *) 0},      // "nextinttrnglspot_is_tboned"
      {check_tbone,             six2spot_tboned_tab},            // "next62spot_is_tboned"
      {check_tbone,            mag62spot_tboned_tab},            // "next_magic62spot_is_tboned"
      {next_galaxyspot_is_tboned,    (const long int *) 0},      // "next_galaxyspot_is_tboned"
      {column_double_down,           (const long int *) 0},      // "column_double_down"
      {apex_test,                      &iden_tab[1]},            /* "apex_test_1" */
      {apex_test,                      &iden_tab[2]},            /* "apex_test_2" */
      {apex_test,                      &iden_tab[3]},            /* "apex_test_3" */
      {boygirlp,                     boystuff_no_rh},            /* "boyp" */
      {boygirlp,                    girlstuff_no_rh},            /* "girlp" */
      {boygirlp,                        boystuff_rh},            /* "boyp_rh_slide_thru" */
      {boygirlp,                       girlstuff_rh},            /* "girlp_rh_slide_thru" */
      {roll_is_cw,                   (const long int *) 0},      /* "roll_is_cw" */
      {roll_is_ccw,                  (const long int *) 0},      /* "roll_is_ccw" */
      {x12_with_other_sex,            boystuff_no_rh},           /* "x12_boy_with_girl" */
      {x12_with_other_sex,           girlstuff_no_rh},           /* "x12_girl_with_boy" */
      {x22_facing_other_sex,          boystuff_no_rh},           /* "x22_boy_facing_girl" */
      {x22_facing_other_sex,         girlstuff_no_rh},           /* "x22_girl_facing_boy" */
      {directionp,                     &iden_tab[0]},            /* "leftp" */
      {directionp,                     &iden_tab[1]},            /* "rightp" */
      {directionp,                     &iden_tab[2]},            /* "inp" */
      {directionp,                     &iden_tab[3]},            /* "outp" */
      {directionp,                     &iden_tab[4]},            /* "backp" */
      {directionp,                     &iden_tab[5]},            /* "zigzagp" */
      {directionp,                     &iden_tab[6]},            /* "zagzigp" */
      {directionp,                     &iden_tab[7]},            /* "zigzigp" */
      {directionp,                     &iden_tab[8]},            /* "zagzagp" */
      {directionp,                     &iden_tab[9]},            /* "no_dir_p" */
      {dmd_ctrs_rh,                    &iden_tab[0]},            /* "dmd_ctrs_rh" */
      {dmd_ctrs_rh,                    &iden_tab[1]},            /* "dmd_ctrs_lh" */
      {trngl_pt_rh,                  (const long int *) 0},      /* "trngl_pt_rh" */
      {q_tag_check, (const long int *) &q_tag_front_action},     /* "q_tag_front" */
      {q_tag_check, (const long int *) &q_tag_back_action},      /* "q_tag_back" */
      {q_tag_check, (const long int *) &q_line_front_action},    /* "q_line_front" */
      {q_tag_check, (const long int *) &q_line_back_action}};    /* "q_line_back" */

int selector_preds = PREDS_BEFORE_THIS_POINT;
