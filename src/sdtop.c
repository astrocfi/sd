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
   update_id_bits
   touch_or_rear_back
   toplevelmove
*/

#include "sd.h"

typedef struct {
   short source_indices[16];
   int size;
   setup_kind outer_kind;
   int rot;
   } expand_thing;

typedef struct {
   int warning;
   int forbidden_elongation;
   expand_thing *expand_lists;
   } full_expand_thing;

static expand_thing exp_1x8_4dm_stuff   = {{12, 13, 15, 14, 4, 5, 7, 6}, 8, s_4dmd, 0};
static expand_thing exp_qtg_4dm_stuff   = {{1, 2, 6, 7, 9, 10, 14, 15}, 8, s_4dmd, 0};
static expand_thing exp_3x1d_3d_stuff   = {{9, 10, 11, 1, 3, 4, 5, 7}, 8, s_3dmd, 0};
static expand_thing exp_4x4_4dm_stuff_a = {{8, 9, 10, 6, 11, 13, 12, 15, 0, 1, 2, 14, 3, 5, 4, 7}, 16, s_4dmd, 1};
static expand_thing exp_4x4_4dm_stuff_b = {{3, 4, 5, 6, 8, 9, 10, 7, 11, 12, 13, 14, 0, 1, 2, 15}, 16, s_4dmd, 0};
static expand_thing exp_2x4_2x6_stuff   = {{1, 2, 3, 4, 7, 8, 9, 10}, 8, s2x6, 0};
static expand_thing exp_qtg_3x4_stuff   = {{1, 2, 4, 5, 7, 8, 10, 11}, 8, s3x4, 0};
static expand_thing exp_2x6_2x8_stuff   = {{1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14}, 12, s2x8, 0};
static expand_thing exp_2x6_4x6_stuff   = {{11, 10, 9, 8, 7, 6, 23, 22, 21, 20, 19, 18}, 12, s4x6, 0};
static expand_thing exp_2x4_2x8_stuff   = {{2, 3, 4, 5, 10, 11, 12, 13}, 8, s2x8, 0};
static expand_thing exp_2x4_4x4_stuff   = {{10, 15, 3, 1, 2, 7, 11, 9}, 8, s4x4, 0};
static expand_thing exp_4x4_blob_stuff  = {{3, 4, 8, 5, 9, 10, 14, 11, 15, 16, 20, 17, 21, 22, 2, 23}, 16, s_bigblob, 0};

static int bit_table_2x2[][4] = {
   {ID2_LEAD|ID2_BEAU,     ID2_TRAILER|ID2_BEAU,  ID2_TRAILER|ID2_BELLE, ID2_LEAD|ID2_BELLE},
   {ID2_LEAD|ID2_BELLE,    ID2_LEAD|ID2_BEAU,     ID2_TRAILER|ID2_BEAU,  ID2_TRAILER|ID2_BELLE},
   {ID2_TRAILER|ID2_BELLE, ID2_LEAD|ID2_BELLE,    ID2_LEAD|ID2_BEAU,     ID2_TRAILER|ID2_BEAU},
   {ID2_TRAILER|ID2_BEAU,  ID2_TRAILER|ID2_BELLE, ID2_LEAD|ID2_BELLE,    ID2_LEAD|ID2_BEAU}};

static int bit_table_2x4[][4] = {
   {ID2_LEAD|ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_BEAU|ID2_END,     ID2_TRAILER|ID2_BELLE|ID2_END,    ID2_LEAD|ID2_BELLE|ID2_END},
   {ID2_LEAD|ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_BEAU|ID2_CENTER,  ID2_TRAILER|ID2_BELLE|ID2_CENTER},
   {ID2_LEAD|ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_BEAU|ID2_CENTER,  ID2_TRAILER|ID2_BELLE|ID2_CENTER, ID2_LEAD|ID2_BELLE|ID2_CENTER},
   {ID2_LEAD|ID2_BELLE|ID2_END,       ID2_LEAD|ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_BEAU|ID2_END,     ID2_TRAILER|ID2_BELLE|ID2_END},
   {ID2_TRAILER|ID2_BELLE|ID2_END,    ID2_LEAD|ID2_BELLE|ID2_END,       ID2_LEAD|ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_BEAU|ID2_END},
   {ID2_TRAILER|ID2_BEAU|ID2_CENTER,  ID2_TRAILER|ID2_BELLE|ID2_CENTER, ID2_LEAD|ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_BEAU|ID2_CENTER},
   {ID2_TRAILER|ID2_BELLE|ID2_CENTER, ID2_LEAD|ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_BEAU|ID2_CENTER},
   {ID2_TRAILER|ID2_BEAU|ID2_END,     ID2_TRAILER|ID2_BELLE|ID2_END,    ID2_LEAD|ID2_BELLE|ID2_END,       ID2_LEAD|ID2_BEAU|ID2_END}};

static int bit_table_1x2[][4] = {
   {ID2_BEAU,              ID2_TRAILER,           ID2_BELLE,             ID2_LEAD},
   {ID2_BELLE,             ID2_LEAD,              ID2_BEAU,              ID2_TRAILER}};

static int bit_table_1x4[][4] = {
   {ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_END,      ID2_BELLE|ID2_END,       ID2_LEAD|ID2_END},
   {ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_CENTER,      ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_CENTER},
   {ID2_BELLE|ID2_END,       ID2_LEAD|ID2_END,         ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_END},
   {ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_CENTER,   ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_CENTER}};

static int bit_table_1x8[][4] = {
   {ID2_BEAU|ID2_OUTR6|ID2_OUTR2,   ID2_TRAILER|ID2_OUTR6|ID2_OUTR2, ID2_BELLE|ID2_OUTR6|ID2_OUTR2,  ID2_LEAD|ID2_OUTR6|ID2_OUTR2},
   {ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6,     ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6},
   {ID2_BELLE|ID2_CTR2|ID2_CTR6,    ID2_LEAD|ID2_CTR2|ID2_CTR6,      ID2_BEAU|ID2_CTR2|ID2_CTR6,     ID2_TRAILER|ID2_CTR2|ID2_CTR6},
   {ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6,  ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6},
   {ID2_BELLE|ID2_OUTR6|ID2_OUTR2,  ID2_LEAD|ID2_OUTR6|ID2_OUTR2,    ID2_BEAU|ID2_OUTR6|ID2_OUTR2,   ID2_TRAILER|ID2_OUTR6|ID2_OUTR2},
   {ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6,  ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6},
   {ID2_BEAU|ID2_CTR2|ID2_CTR6,     ID2_TRAILER|ID2_CTR2|ID2_CTR6,   ID2_BELLE|ID2_CTR2|ID2_CTR6,    ID2_LEAD|ID2_CTR2|ID2_CTR6},
   {ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6,     ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6}};

static int bit_table_qtag[][4] = {
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2}};

static int bit_table_hrglass[][4] = {
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2}};

static int bit_table_spindle[][4] = {
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_OUTR2,   ID2_OUTR2,  ID2_OUTR2,  ID2_OUTR2},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_OUTR2,   ID2_OUTR2,  ID2_OUTR2,  ID2_OUTR2}};

#define BITS_TO_CLEAR (ID2_LEAD|ID2_TRAILER|ID2_BEAU|ID2_BELLE|ID2_CENTER|ID2_END|ID2_CTR2|ID2_CTR6|ID2_OUTR2|ID2_OUTR6)
#define UNSYM_BITS_TO_CLEAR (ID2_NEARCOL|ID2_NEARLINE|ID2_NEARBOX|ID2_FARCOL|ID2_FARLINE|ID2_FARBOX)




typedef int bit_table[4];

extern void update_id_bits(setup *ss)

{
   int i, j;
   bit_table *ptr;

   for (i=0; i<MAX_PEOPLE; i++) ss->people[i].id2 &= ~BITS_TO_CLEAR;

   switch (ss->kind) {
      case s_1x2:
         ptr = bit_table_1x2; break;
      case s1x4:
         ptr = bit_table_1x4; break;
      case s2x2:
         ptr = bit_table_2x2; break;
      case s2x4:
         ptr = bit_table_2x4; break;
      case s1x8:
         ptr = bit_table_1x8; break;
      case s_qtag:
         ptr = bit_table_qtag; break;
      case s_hrglass:
         ptr = bit_table_hrglass; break;
      case s_spindle:
         ptr = bit_table_spindle; break;
      default:
         return;
   }

   j = setup_limits[ss->kind];
   for (i=0; i<=j; i++) {
      if (ss->people[i].id1 & BIT_PERSON)
         ss->people[i].id2 |= ptr[i][ss->people[i].id1 & 3];
   }
}


static expand_thing rear_wave_stuff = {{3, 0, 1, 2}, 4, s2x2, 0};
static expand_thing rear_bone_stuff = {{0, 3, 2, 5, 4, 7, 6, 1}, 8, s2x4, 0};
static expand_thing rear_wing_stuff = {{1, 2, 3, 4, 5, 6, 7, 0}, 8, s2x4, 0};
static expand_thing rear_c1a_stuff = {{0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1}, 16, s2x4, 0};
static expand_thing rear_c1b_stuff = {{-1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6}, 16, s2x4, 0};
static expand_thing rear_c1c_stuff = {{6, -1, 7, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1}, 16, s2x4, 1};
static expand_thing rear_c1d_stuff = {{-1, 7, -1, 6, -1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5}, 16, s2x4, 1};
static expand_thing rear_miniwave_stuff = {{1, 0}, 2, s_1x2, 1};
static expand_thing rear_2x4_stuff = {{6, 7, 1, 0, 2, 3, 5, 4}, 8, s2x4, 1};
static expand_thing rear_col_stuff = {{0, 3, 6, 5, 4, 7, 2, 1}, 8, s1x8, 0};
static expand_thing rear_vrbox_stuff = {{1, 0, 3, 2}, 4, s1x4, 1};
static expand_thing rear_hrbox_stuff = {{0, 3, 2, 1}, 4, s1x4, 0};
static expand_thing rear_gwave_stuff = {{7, 0, 1, 6, 3, 4, 5, 2}, 8, s2x4, 0};
static expand_thing rear_qtag_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, s2x4, 1};
static expand_thing rear_sqtag_stuff = {{0, 1, 2, 3}, 4, s1x4, 0};

static full_expand_thing rear_wave_pair     = {warn__rear_back,       0, &rear_wave_stuff};
static full_expand_thing rear_bone_pair     = {warn__some_rear_back,  0, &rear_bone_stuff};
static full_expand_thing rear_wing_pair     = {warn__some_rear_back,  0, &rear_wing_stuff};
static full_expand_thing rear_c1a_pair      = {warn__some_rear_back,  0, &rear_c1a_stuff};
static full_expand_thing rear_c1b_pair      = {warn__some_rear_back,  0, &rear_c1b_stuff};
static full_expand_thing rear_c1c_pair      = {warn__some_rear_back,  0, &rear_c1c_stuff};
static full_expand_thing rear_c1d_pair      = {warn__some_rear_back,  0, &rear_c1d_stuff};
static full_expand_thing rear_miniwave_pair = {warn__rear_back,       0, &rear_miniwave_stuff};
static full_expand_thing rear_2x4_pair      = {warn__rear_back,       0, &rear_2x4_stuff};
static full_expand_thing rear_col_pair      = {warn__awful_rear_back, 0, &rear_col_stuff};
static full_expand_thing rear_vrbox_pair    = {warn__awful_rear_back, 0, &rear_vrbox_stuff};
static full_expand_thing rear_hrbox_pair    = {warn__awful_rear_back, 0, &rear_hrbox_stuff};
static full_expand_thing rear_gwave_pair    = {warn__rear_back,       0, &rear_gwave_stuff};
static full_expand_thing rear_qtag_pair     = {warn__rear_back,       0, &rear_qtag_stuff};
static full_expand_thing rear_sqtag_pair    = {warn__awful_rear_back, 0, &rear_sqtag_stuff};

static expand_thing step_1x8_stuff = {{0, 7, 6, 1, 4, 3, 2, 5}, 8, s2x4, 0};
static expand_thing step_1x4_side_stuff = {{0, 1, 2, 3}, 4, sdmd, 0};
static expand_thing step_1x4_stuff = {{0, 3, 2, 1}, 4, s2x2, 0};
static expand_thing step_1x2_stuff = {{0, 1}, 2, s_1x2, 1};
static expand_thing step_2x2v_stuff = {{1, 2, 3, 0}, 4, s1x4, 0};
static expand_thing step_2x2h_stuff = {{0, 1, 2, 3}, 4, s1x4, 1};
static expand_thing step_8ch_stuff = {{7, 6, 0, 1, 3, 2, 4, 5}, 8, s2x4, 1};
static expand_thing step_li_stuff = {{1, 2, 7, 4, 5, 6, 3, 0}, 8, s1x8, 0};
static expand_thing step_tby_stuff = {{5, 6, 7, 0, 1, 2, 3, 4}, 8, s_qtag, 1};

static full_expand_thing step_1x8_pair =      {0, 0, &step_1x8_stuff};
static full_expand_thing step_1x4_side_pair = {0, 0, &step_1x4_side_stuff};
static full_expand_thing step_1x4_pair =      {0, 0, &step_1x4_stuff};
static full_expand_thing step_2x2v_pair =     {0, 2*SETUPFLAG__ELONGATE_BIT, &step_2x2v_stuff};
static full_expand_thing step_2x2h_pair =     {0, 1*SETUPFLAG__ELONGATE_BIT, &step_2x2h_stuff};
static full_expand_thing step_8ch_pair =      {0, 0, &step_8ch_stuff};
static full_expand_thing step_li_pair =       {0, 0, &step_li_stuff};
static full_expand_thing step_tby_pair =      {0, 0, &step_tby_stuff};
static full_expand_thing step_1x2_pair =      {0, 0, &step_1x2_stuff};



extern void touch_or_rear_back(
   setup *scopy,
   int callflags)

{
   int i, j;
   setup stemp;
   full_expand_thing *tptr;
   expand_thing *zptr;
   int directions, livemask;

   directions = 0;
   livemask = 0;
   tptr = 0;

   for (i=0, j=1; i<=setup_limits[scopy->kind]; i++, j<<=1) {
      int p = scopy->people[i].id1;
      directions = (directions<<2) | (p&3);
      if (p) livemask |= j;
   }

   if (callflags & cflag__rear_back_from_r_wave) {
      if (scopy->kind == s1x4 && (livemask == 0xF) && (directions == 0x28)) {
         tptr = &rear_wave_pair;          /* Rear back from a wave to facing couples. */
      }
      else if (scopy->kind == s_1x2 && (livemask == 0x3) && (directions == 0x2)) {
         tptr = &rear_miniwave_pair;      /* Rear back from a miniwave to facing people. */
      }
      else if (scopy->kind == s2x4) {
         if ((livemask == 0xFF) && (directions == 0x2288)) {
            tptr = &rear_2x4_pair;        /* Rear back from parallel waves to an 8 chain. */
         }
         else if ((livemask == 0xFF) && (directions == 0x55FF)) {
            tptr = &rear_col_pair;        /* Rear back from columns to end-to-end single 8-chains. */
         }
      }
      else if (scopy->kind == s2x2) {
         if ((livemask == 0xF) && (directions == 0x28)) {
            tptr = &rear_vrbox_pair;      /* Rear back from a right-hand box to a single 8 chain. */
         }
         else if ((livemask == 0xF) && (directions == 0x5F)) {
            tptr = &rear_hrbox_pair;      /* Rear back from a right-hand box to a single 8 chain. */
         }
      }
      else if (scopy->kind == s1x8 && (livemask == 0xFF) && (directions == 0x2882)) {
         tptr = &rear_gwave_pair;         /* Rear back from a grand wave to facing lines. */
      }
      else if (scopy->kind == s_bone && livemask == 0xFF && ((directions == 0xA802) || (directions == 0x78D2))) {
         /* Centers rear back from a "bone" to lines facing or "split square thru" setup. */
         tptr = &rear_bone_pair;
      }
      else if (scopy->kind == s_rigger && livemask == 0xFF && ((directions == 0xA802) || (directions == 0xD872))) {
         /* Ends rear back from a "wing" to lines facing or "split square thru" setup. */
         tptr = &rear_wing_pair;
      }
      else if (scopy->kind == s_c1phan) {
         /* Check for certain people rearing back from C1 phantoms. */
         if ((livemask == 0x5555) && ((directions == 0x884C00C4) || (directions == 0x4C4CC4C4))) {
            tptr = &rear_c1a_pair;
         }
         else if ((livemask == 0xAAAA) && ((directions == 0x13223100) || (directions == 0x13313113))) {
            tptr = &rear_c1b_pair;
         }
         else if ((livemask == 0x5555) && ((directions == 0x08CC8044) || (directions == 0x08808008))) {
            tptr = &rear_c1c_pair;
         }
         else if ((livemask == 0xAAAA) && ((directions == 0x11203302) || (directions == 0x20200202))) {
            tptr = &rear_c1d_pair;
         }
      }
   }
   else if (callflags & cflag__rear_back_from_qtag) {
      if (scopy->kind == s_qtag && livemask == 0xFF && ((directions == 0x08A2) || (directions == 0xA802))) {
         tptr = &rear_qtag_pair;         /* Have the centers rear back from a 1/4 tag or 3/4 tag. */
      }
      else if (scopy->kind == sdmd && livemask == 0xF && ((directions == 0x5F) || (directions == 0xD7))) {
         tptr = &rear_sqtag_pair;         /* Have the centers rear back from a single 1/4 tag or 3/4 tag. */
      }
   }
   else {      /* We know that (callflags & cflag__step_to_wave) is true here. */
      switch (scopy->kind) {
         case s2x2:
            if ((livemask == 0xF) && (directions == 0x7D)) {
               tptr = &step_2x2h_pair;
            }
            else if ((livemask == 0xF) && (directions == 0xA0)) {
               tptr = &step_2x2v_pair;
            }
            break;
         case s2x4:
            if ((livemask == 0xFF) && (directions == 0x77DD)) {
               tptr = &step_8ch_pair;         /* Check for stepping to parallel waves from an 8 chain. */
            }
            else if ((livemask == 0xFF) && (directions == 0xAA00)) {
               tptr = &step_li_pair;          /* Check for stepping to a grand wave from lines facing. */
            }
            else if ((livemask == 0xFF) && ((directions == 0xDD77) || (directions == 0x5FF5))) {
               tptr = &step_tby_pair;         /* Check for stepping to a 1/4 tag or 3/4 tag from a DPT or trade-by. */
            }
            break;
         case s_1x2:
            if ((livemask == 0x3) && (directions == 0x7)) {
               tptr = &step_1x2_pair;         /* Check for stepping to a miniwave from people facing. */
            }
            break;
         case s1x4:
            /* Check for stepping to a box from a 1x4 single 8 chain -- we allow some phantoms.  This is what makes
               triple columns turn and weave legal in certain interesting cases. */
            if ((livemask == 0xF) && (directions == 0x7D))
               tptr = &step_1x4_pair;
            else if ((livemask == 0x3) && (directions == 0x70))
               tptr = &step_1x4_pair;
            else if ((livemask == 0xC) && (directions == 0x0D))
               tptr = &step_1x4_pair;
            /* Check for stepping to a single 1/4 tag or 3/4 tag from a single-file DPT or trade-by --
               we allow some phantoms, as above. */
            else if ((livemask == 0xF) && ((directions == 0xD7) || (directions == 0x5F)))
               tptr = &step_1x4_side_pair;
            else if ((livemask == 0xA) && (directions == 0x13))
               tptr = &step_1x4_side_pair;
            break;
         case s1x8:
            if ((livemask == 0xFF) && (directions == 0x7DD7)) {
               tptr = &step_1x8_pair;         /* Check for stepping to a column from a 1x8 single 8 chain. */
            }
            break;
      }
   }

   if (!tptr) return;
   if (tptr->warning) warn(tptr->warning);
   if (scopy->setupflags & tptr->forbidden_elongation)
      fail("People are too far away to work with each other on this call.");
   zptr = tptr->expand_lists;

   scopy->setupflags |= SETUPFLAG__DISTORTED;

   stemp = *scopy;
   clear_people(scopy);

   if (zptr->rot) {
      for (i=0; i<zptr->size; i++) {
         int idx = zptr->source_indices[i];
         if (idx < 0) {
            if (stemp.people[i].id1) fail("Don't understand this setup at all.");
         }
         else {
            (void) copy_rot(scopy, idx, &stemp, i, 033);
         }
      }
      scopy->rotation++;
   }
   else {
      for (i=0; i<zptr->size; i++) {
         int idx = zptr->source_indices[i];
         if (idx < 0) {
            if (stemp.people[i].id1) fail("Don't understand this setup at all.");
         }
         else {
            (void) copy_person(scopy, idx, &stemp, i);
         }
      }
   }

   scopy->kind = zptr->outer_kind;
   canonicalize_rotation(scopy);
}



/* Top level move routine. */

extern void toplevelmove(void)
{
   setup stemp;
   int i, j, livemask;
   expand_thing *eptr;
   parse_block *conceptptr;
   parse_block *cptr;
   final_set new_final_concepts;
   setup new_setup;

   int concept_read_base = 0;
   setup starting_setup = history[history_ptr].state;
   configuration *newhist = &history[history_ptr+1];
   /* Be sure that the amount of written history that we consider to be safely
      written is less than the item we are about to change. */
   if (written_history_items > history_ptr)
      written_history_items = history_ptr;

   starting_setup.setupflags = 0;

   /* If we are starting a sequence with the "so-and-so into the center and do whatever"
      flag on, and this call is a "sequence starter", take special action. */

   if (startinfolist[history[history_ptr].centersp].into_the_middle) {
   
      /* If the call is a special sequence starter (e.g. spin a pulley) remove the implicit
         "centers" concept and just do it.  The setup in this case will be a squared set
         with so-and-so moved into the middle, which is what the encoding of these calls
         wants. */
   
      if (parse_state.topcallflags & cflag__sequence_starter)
         concept_read_base = 1;
   
      /* If the call is a "split square thru" type of call, we do the same.  We leave the
         "split" concept in place.  Other mechanisms will do the rest. */

      else if ((parse_state.topcallflags & cflag__split_like_square_thru) &&
            ((newhist->command_root->next->concept->kind == concept_split) ||
                  ((newhist->command_root->next->concept->kind == concept_left) &&
                  (newhist->command_root->next->next->concept->kind == concept_split))))
         concept_read_base = 1;

      /* If the call is a "split dixie style" type of call, we do something similar,
         but the transformation we perform on the starting setup is highly peculiar. */
   /* ****** bug: ability to handle "left" concept patched out!  It does it wrong! */
   
      else if ((parse_state.topcallflags & cflag__split_like_dixie_style) &&
               newhist->command_root->next->concept->kind == concept_split &&
               newhist->command_root->next->next->concept->kind == marker_end_of_list) {
         personrec x;

         concept_read_base = 2;
         starting_setup.rotation++;
         x = starting_setup.people[0];
         (void) copy_rot(&starting_setup, 0, &starting_setup, 6, 022);
         (void) copy_person(&starting_setup, 6, &starting_setup, 5);
         (void) copy_rot(&starting_setup, 5, &starting_setup, 7, 033);
         starting_setup.people[7].id1 = rotccw(x.id1);
         starting_setup.people[7].id2 = x.id2;

         x = starting_setup.people[4];
         (void) copy_rot(&starting_setup, 4, &starting_setup, 2, 022);
         (void) copy_person(&starting_setup, 2, &starting_setup, 1);
         (void) copy_rot(&starting_setup, 1, &starting_setup, 3, 033);
         starting_setup.people[3].id1 = rotccw(x.id1);
         starting_setup.people[3].id2 = x.id2;
      }
   }

   if (concept_read_base == 2)
      conceptptr = newhist->command_root->next->next;
   else if (concept_read_base == 1)
      conceptptr = newhist->command_root->next;
   else
      conceptptr = newhist->command_root;

   /* Clear a few things.  We do NOT clear the warnings, because some (namely the
      "concept not allowed at this level" warning) may have already been logged. */
   newhist->centersp = 0;
   newhist->draw_pic = FALSE;
   newhist->resolve_flag.kind = resolve_none;

   /* Set the selector to "uninitialized", so that, if we do a call like "run", we
      will query the user to find out who is selected. */

   current_selector = selector_uninitialized;

   /* This next thing is not really right.  It makes the program refuse to step to a wave     */
   /*   or rear back from one if any concepts are in use.  Actually, we should allow          */
   /*   a few concepts, such as left.  We should only disallow concepts where the change of   */
   /*   shape resulting from the stepping to a wave or rearing back alters the interpretation */
   /*   of the concept.  For example, the evaluation of split phantom lines would be ruined   */
   /*   if an implicit step to a wave occurred.                                               */

   /* We must check for rearing back from a wave or stepping to one.  Do this only if there are
      no virtual-setup concepts.  The way we find out is to pass over
      all the final concepts and demand that the next thing be the end of the concept list.
      If this test is satisfied, just call move. */

   cptr = process_final_concepts(conceptptr, TRUE, &new_final_concepts);

   if (cptr->concept->kind <= marker_end_of_list) {

      /* We make move re-read the final concepts.  ***** Is this necessary? */

      goto do_the_call;
   }

   /* We seem to have a virtual-setup concept.  See if the matrix needs to be expanded. */
/* ****** We should just use the left-over cptr, and demand that the new final mask be zero. */

   cptr = conceptptr;
   while (cptr->concept->kind == concept_comment) cptr = cptr->next;
   if (cptr->concept->kind == concept_standard) cptr = cptr->next;
   while (cptr->concept->kind == concept_comment) cptr = cptr->next;

   tryagain:

   if (concept_table[cptr->concept->kind].concept_prop & (CONCPROP__NEED_4X4 | CONCPROP__NEED_BLOB | CONCPROP__NEED_4X6)) {
      if (starting_setup.kind == s2x4) {
         eptr = &exp_2x4_4x4_stuff; goto expand_me;
      }
   }

   if (concept_table[cptr->concept->kind].concept_prop & (CONCPROP__NEED_BLOB)) {
      if (starting_setup.kind == s4x4) {
         eptr = &exp_4x4_blob_stuff; goto expand_me;
      }
   }
   else if (concept_table[cptr->concept->kind].concept_prop & CONCPROP__NEED_4DMD) {
      switch (starting_setup.kind) {
         case s1x8:
            eptr = &exp_1x8_4dm_stuff; goto expand_me;
         case s_qtag:
            eptr = &exp_qtg_4dm_stuff; goto expand_me;
         case s4x4:
            for (i=0, j=1, livemask=0; i<16; i++, j<<=1) {
               if (starting_setup.people[i].id1) livemask |= j;
            }

            if (livemask == 0x1717) {
               eptr = &exp_4x4_4dm_stuff_a; goto expand_me;
            }
            else if (livemask == 0x7171) {
               eptr = &exp_4x4_4dm_stuff_b; goto expand_me;
            }
      }
   }
   else if (concept_table[cptr->concept->kind].concept_prop & CONCPROP__NEED_3DMD) {
      if (starting_setup.kind == s_3x1dmd) {         /* Need to expand to real triple diamonds. */
         eptr = &exp_3x1d_3d_stuff; goto expand_me;
      }
   }
   else if (concept_table[cptr->concept->kind].concept_prop & CONCPROP__NEED_2X8) {
      switch (starting_setup.kind) {         /* Need to expand to a 2x8. */
         case s2x4:
            eptr = &exp_2x4_2x8_stuff; goto expand_me;
         case s2x6:
            eptr = &exp_2x6_2x8_stuff; goto expand_me;
      }
   }
   else if (concept_table[cptr->concept->kind].concept_prop & CONCPROP__NEED_2X6) {
      if (starting_setup.kind == s2x4) {         /* Need to expand to a 2x6. */
         eptr = &exp_2x4_2x6_stuff; goto expand_me;
      }
   }
   else if (concept_table[cptr->concept->kind].concept_prop & CONCPROP__NEED_4X6) {
      if (starting_setup.kind == s2x6) {         /* Need to expand to a 4x6. */
         eptr = &exp_2x6_4x6_stuff; goto expand_me;
      }
   }
   else {
      switch (cptr->concept->kind) {
         case concept_triple_lines: case concept_triple_lines_together:
            if (starting_setup.kind == s_qtag) {         /* Need to expand to a 3x4. */
               eptr = &exp_qtg_3x4_stuff; goto expand_me;
            }
            break;
      }
   }

   /* If get here, we did NOT see any concept that requires a setup expansion. */

   goto do_the_call;

   expand_me:

   /* If get here, we DID see a concept that requires a setup expansion. */

   stemp = starting_setup;
   clear_people(&starting_setup);
   if (eptr->rot) {
      for (i=0; i<eptr->size; i++) (void) copy_rot(&starting_setup, eptr->source_indices[i], &stemp, i, 033);
      starting_setup.rotation++;
   }
   else {
      for (i=0; i<eptr->size; i++) (void) copy_person(&starting_setup, eptr->source_indices[i], &stemp, i);
   }

   starting_setup.kind = eptr->outer_kind;
   canonicalize_rotation(&starting_setup);

   goto tryagain;

   do_the_call:

   /* Put in identification bits for unsymmetrical stuff, if possible. */

   for (i=0; i<MAX_PEOPLE; i++) starting_setup.people[i].id2 &= ~UNSYM_BITS_TO_CLEAR;

   if (starting_setup.kind == s2x4) {
      int nearbit = 0;
      int farbit = 0;

      if (starting_setup.rotation & 1) {
         nearbit = ID2_NEARBOX;
         farbit = ID2_FARBOX;
      }
      else {
         int tbonetest = 0;
         for (i=0; i<8; i++) tbonetest |= starting_setup.people[i].id1;

         if (!(tbonetest & 1)) {
            nearbit = ID2_NEARLINE;
            farbit = ID2_FARLINE;
         }
         else if (!(tbonetest & 010)) {
            nearbit = ID2_NEARCOL;
            farbit = ID2_FARCOL;
         }
      }

      for (i=0; i<8; i++) {
         if (starting_setup.people[i].id1 & BIT_PERSON)
            starting_setup.people[i].id2 |= ((i + (starting_setup.rotation << 1)) & 4) ? nearbit:farbit;
      }
   }

   /* Put in position-identification bits (leads/trailers/beaux/belles/centers/ends etc.) */

   update_id_bits(&starting_setup);
   move(&starting_setup, conceptptr, NULLCALLSPEC, 0, FALSE, &new_setup);

   /* Remove outboard phantoms from the resulting setup. */

   normalize_setup(&new_setup, simple_normalize);

   newhist->state = new_setup;
   newhist->resolve_flag = resolve_p(&new_setup);
}
