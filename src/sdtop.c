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
   update_id_bits
   touch_or_rear_back
   do_matrix_expansion
   normalize_setup
   toplevelmove
*/

#include "sd.h"

typedef struct {
   short source_indices[16];
   int size;
   setup_kind inner_kind;
   setup_kind outer_kind;
   int rot;
   } expand_thing;

typedef struct {
   warning_index warning;
   int forbidden_elongation;
   expand_thing *expand_lists;
   } full_expand_thing;

Private expand_thing exp_1x8_4dm_stuff   = {{12, 13, 15, 14, 4, 5, 7, 6}, 8, s1x8, s_4dmd, 0};
Private expand_thing exp_qtg_4dm_stuff   = {{1, 2, 6, 7, 9, 10, 14, 15}, 8, nothing, s_4dmd, 0};
Private expand_thing exp_3x1d_3d_stuff   = {{9, 10, 11, 1, 3, 4, 5, 7}, 8, s_3x1dmd, s_3dmd, 0};
Private expand_thing exp_4x4_4dm_stuff_a = {{0, 1, 2, 14, 3, 5, 4, 7, 8, 9, 10, 6, 11, 13, 12, 15}, 16, nothing, s_4dmd, 1};
Private expand_thing exp_4x4_4dm_stuff_b = {{3, 4, 5, 6, 8, 9, 10, 7, 11, 12, 13, 14, 0, 1, 2, 15}, 16, nothing, s_4dmd, 0};
Private expand_thing exp_2x4_2x6_stuff   = {{1, 2, 3, 4, 7, 8, 9, 10}, 8, s2x4, s2x6, 0};
Private expand_thing exp_qtg_3x4_stuff   = {{1, 2, 4, 5, 7, 8, 10, 11}, 8, nothing, s3x4, 0};
Private expand_thing exp_2x6_2x8_stuff   = {{1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14}, 12, s2x6, s2x8, 0};
Private expand_thing exp_1x8_1x12_stuff  = {{2, 3, 5, 4, 8, 9, 11, 10}, 8, nothing, s1x12, 0};
Private expand_thing exp_1x10_1x12_stuff = {{1, 2, 3, 4, 5, 7, 8, 9, 10, 11}, 10, s1x10, s1x12, 0};
Private expand_thing exp_1x12_1x14_stuff = {{1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13}, 12, s1x12, s1x14, 0};
Private expand_thing exp_1x14_1x16_stuff = {{1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15}, 14, s1x14, s1x16, 0};
Private expand_thing exp_1x8_1x10_stuff = {{1, 2, 4, 3, 6, 7, 9, 8}, 8, s1x8, s1x10, 0};
Private expand_thing exp_2x6_4x6_stuff   = {{11, 10, 9, 8, 7, 6, 23, 22, 21, 20, 19, 18}, 12, nothing, s4x6, 0};
Private expand_thing exp_2x4_2x8_stuff   = {{2, 3, 4, 5, 10, 11, 12, 13}, 8, nothing, s2x8, 0};
Private expand_thing exp_2x4_4x4_stuff   = {{10, 15, 3, 1, 2, 7, 11, 9}, 8, s2x4, s4x4, 0};
Private expand_thing exp_4x4_blob_stuff  = {{3, 4, 8, 5, 9, 10, 14, 11, 15, 16, 20, 17, 21, 22, 2, 23}, 16, nothing, s_bigblob, 0};



Private void compress_setup(expand_thing *thing, setup *stuff)
{
   int i;
   setup temp = *stuff;

   stuff->kind = thing->inner_kind;
   for (i=0; i<thing->size; i++) {
      (void) copy_person(stuff, i, &temp, thing->source_indices[i]);
   }
}




Private int bit_table_2x2[][4] = {
   {ID2_LEAD|ID2_BEAU,     ID2_TRAILER|ID2_BEAU,  ID2_TRAILER|ID2_BELLE, ID2_LEAD|ID2_BELLE},
   {ID2_LEAD|ID2_BELLE,    ID2_LEAD|ID2_BEAU,     ID2_TRAILER|ID2_BEAU,  ID2_TRAILER|ID2_BELLE},
   {ID2_TRAILER|ID2_BELLE, ID2_LEAD|ID2_BELLE,    ID2_LEAD|ID2_BEAU,     ID2_TRAILER|ID2_BEAU},
   {ID2_TRAILER|ID2_BEAU,  ID2_TRAILER|ID2_BELLE, ID2_LEAD|ID2_BELLE,    ID2_LEAD|ID2_BEAU}};

Private int bit_table_2x4[][4] = {
   {ID2_LEAD|ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_BEAU|ID2_END,     ID2_TRAILER|ID2_BELLE|ID2_END,    ID2_LEAD|ID2_BELLE|ID2_END},
   {ID2_LEAD|ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_BEAU|ID2_CENTER,  ID2_TRAILER|ID2_BELLE|ID2_CENTER},
   {ID2_LEAD|ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_BEAU|ID2_CENTER,  ID2_TRAILER|ID2_BELLE|ID2_CENTER, ID2_LEAD|ID2_BELLE|ID2_CENTER},
   {ID2_LEAD|ID2_BELLE|ID2_END,       ID2_LEAD|ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_BEAU|ID2_END,     ID2_TRAILER|ID2_BELLE|ID2_END},
   {ID2_TRAILER|ID2_BELLE|ID2_END,    ID2_LEAD|ID2_BELLE|ID2_END,       ID2_LEAD|ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_BEAU|ID2_END},
   {ID2_TRAILER|ID2_BEAU|ID2_CENTER,  ID2_TRAILER|ID2_BELLE|ID2_CENTER, ID2_LEAD|ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_BEAU|ID2_CENTER},
   {ID2_TRAILER|ID2_BELLE|ID2_CENTER, ID2_LEAD|ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_BEAU|ID2_CENTER},
   {ID2_TRAILER|ID2_BEAU|ID2_END,     ID2_TRAILER|ID2_BELLE|ID2_END,    ID2_LEAD|ID2_BELLE|ID2_END,       ID2_LEAD|ID2_BEAU|ID2_END}};

Private int bit_table_1x2[][4] = {
   {ID2_BEAU,              ID2_TRAILER,           ID2_BELLE,             ID2_LEAD},
   {ID2_BELLE,             ID2_LEAD,              ID2_BEAU,              ID2_TRAILER}};

Private int bit_table_1x4[][4] = {
   {ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_END,      ID2_BELLE|ID2_END,       ID2_LEAD|ID2_END},
   {ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_CENTER,      ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_CENTER},
   {ID2_BELLE|ID2_END,       ID2_LEAD|ID2_END,         ID2_BEAU|ID2_END,        ID2_TRAILER|ID2_END},
   {ID2_BEAU|ID2_CENTER,     ID2_TRAILER|ID2_CENTER,   ID2_BELLE|ID2_CENTER,    ID2_LEAD|ID2_CENTER}};

Private int bit_table_1x8[][4] = {
   {ID2_BEAU|ID2_OUTR6|ID2_OUTR2,   ID2_TRAILER|ID2_OUTR6|ID2_OUTR2, ID2_BELLE|ID2_OUTR6|ID2_OUTR2,  ID2_LEAD|ID2_OUTR6|ID2_OUTR2},
   {ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6,     ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6},
   {ID2_BELLE|ID2_CTR2|ID2_CTR6,    ID2_LEAD|ID2_CTR2|ID2_CTR6,      ID2_BEAU|ID2_CTR2|ID2_CTR6,     ID2_TRAILER|ID2_CTR2|ID2_CTR6},
   {ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6,  ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6},
   {ID2_BELLE|ID2_OUTR6|ID2_OUTR2,  ID2_LEAD|ID2_OUTR6|ID2_OUTR2,    ID2_BEAU|ID2_OUTR6|ID2_OUTR2,   ID2_TRAILER|ID2_OUTR6|ID2_OUTR2},
   {ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6,  ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6},
   {ID2_BEAU|ID2_CTR2|ID2_CTR6,     ID2_TRAILER|ID2_CTR2|ID2_CTR6,   ID2_BELLE|ID2_CTR2|ID2_CTR6,    ID2_LEAD|ID2_CTR2|ID2_CTR6},
   {ID2_BELLE|ID2_OUTR6|ID2_CTR6,   ID2_LEAD|ID2_OUTR6|ID2_CTR6,     ID2_BEAU|ID2_OUTR6|ID2_CTR6,    ID2_TRAILER|ID2_OUTR6|ID2_CTR6}};

Private int bit_table_qtag[][4] = {
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2}};

Private int bit_table_hrglass[][4] = {
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6,    ID2_END|ID2_OUTR6},
   {ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6, ID2_CENTER|ID2_OUTR6},
   {ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2,  ID2_CENTER|ID2_CTR2}};

Private int bit_table_spindle[][4] = {
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_OUTR2,   ID2_OUTR2,  ID2_OUTR2,  ID2_OUTR2},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_CTR6,    ID2_CTR6,   ID2_CTR6,   ID2_CTR6},
   {ID2_OUTR2,   ID2_OUTR2,  ID2_OUTR2,  ID2_OUTR2}};

Private int bit_table_rigger[][4] = {
   {ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BEAU,     ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BEAU,  ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BELLE, ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BELLE},
   {ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BELLE,    ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BEAU,     ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BEAU,  ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BELLE},
   {ID2_OUTR2|ID2_END|ID2_BELLE,               ID2_OUTR2|ID2_END|ID2_LEAD,                ID2_OUTR2|ID2_END|ID2_BEAU,                ID2_OUTR2|ID2_END|ID2_TRAILER},
   {ID2_CTR6|ID2_END|ID2_BEAU,                 ID2_CTR6|ID2_END|ID2_TRAILER,              ID2_CTR6|ID2_END|ID2_BELLE,                ID2_CTR6|ID2_END|ID2_LEAD},
   {ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BELLE, ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BELLE,    ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BEAU,     ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BEAU},
   {ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BEAU,  ID2_CTR6|ID2_CENTER|ID2_TRAILER|ID2_BELLE, ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BELLE,    ID2_CTR6|ID2_CENTER|ID2_LEAD|ID2_BEAU},
   {ID2_OUTR2|ID2_END|ID2_BEAU,                ID2_OUTR2|ID2_END|ID2_TRAILER,             ID2_OUTR2|ID2_END|ID2_BELLE,               ID2_OUTR2|ID2_END|ID2_LEAD},
   {ID2_CTR6|ID2_END|ID2_BELLE,                ID2_CTR6|ID2_END|ID2_LEAD,                 ID2_CTR6|ID2_END|ID2_BEAU,                 ID2_CTR6|ID2_END|ID2_TRAILER}};

Private int bit_table_bone[][4] = {
   {ID2_END|ID2_OUTR6|ID2_LEAD,     ID2_END|ID2_OUTR6|ID2_BEAU,       ID2_END|ID2_OUTR6|ID2_TRAILER,  ID2_END|ID2_OUTR6|ID2_BELLE},
   {ID2_END|ID2_OUTR6|ID2_LEAD,     ID2_END|ID2_OUTR6|ID2_BEAU,       ID2_END|ID2_OUTR6|ID2_TRAILER,  ID2_END|ID2_OUTR6|ID2_BELLE},
   {ID2_CENTER|ID2_OUTR6|ID2_BELLE, ID2_CENTER|ID2_OUTR6|ID2_LEAD,    ID2_CENTER|ID2_OUTR6|ID2_BEAU,  ID2_CENTER|ID2_OUTR6|ID2_TRAILER},
   {ID2_CENTER|ID2_CTR2|ID2_BEAU,   ID2_CENTER|ID2_CTR2|ID2_TRAILER,  ID2_CENTER|ID2_CTR2|ID2_BELLE,  ID2_CENTER|ID2_CTR2|ID2_LEAD},
   {ID2_END|ID2_OUTR6|ID2_TRAILER,  ID2_END|ID2_OUTR6|ID2_BELLE,      ID2_END|ID2_OUTR6|ID2_LEAD,     ID2_END|ID2_OUTR6|ID2_BEAU},
   {ID2_END|ID2_OUTR6|ID2_TRAILER,  ID2_END|ID2_OUTR6|ID2_BELLE,      ID2_END|ID2_OUTR6|ID2_LEAD,     ID2_END|ID2_OUTR6|ID2_BEAU},
   {ID2_CENTER|ID2_OUTR6|ID2_BEAU,  ID2_CENTER|ID2_OUTR6|ID2_TRAILER, ID2_CENTER|ID2_OUTR6|ID2_BELLE, ID2_CENTER|ID2_OUTR6|ID2_LEAD},
   {ID2_CENTER|ID2_CTR2|ID2_BELLE,  ID2_CENTER|ID2_CTR2|ID2_LEAD,     ID2_CENTER|ID2_CTR2|ID2_BEAU,   ID2_CENTER|ID2_CTR2|ID2_TRAILER}};


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
      case s_rigger:
         ptr = bit_table_rigger; break;
      case s_bone:
         ptr = bit_table_bone; break;
      default:
         return;
   }

   j = setup_limits[ss->kind];
   for (i=0; i<=j; i++) {
      if (ss->people[i].id1 & BIT_PERSON)
         ss->people[i].id2 |= ptr[i][ss->people[i].id1 & 3];
   }
}


Private expand_thing rear_wave_stuff = {{3, 0, 1, 2}, 4, nothing, s2x2, 0};
Private expand_thing rear_bone_stuff = {{0, 3, 2, 5, 4, 7, 6, 1}, 8, nothing, s2x4, 0};
Private expand_thing rear_wing_stuff = {{1, 2, 3, 4, 5, 6, 7, 0}, 8, nothing, s2x4, 0};
Private expand_thing rear_c1a_stuff = {{0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1}, 16, nothing, s2x4, 0};
Private expand_thing rear_c1b_stuff = {{-1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6}, 16, nothing, s2x4, 0};
Private expand_thing rear_c1c_stuff = {{6, -1, 7, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1}, 16, nothing, s2x4, 1};
Private expand_thing rear_c1d_stuff = {{-1, 7, -1, 6, -1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5}, 16, nothing, s2x4, 1};
Private expand_thing rear_miniwave_stuff = {{1, 0}, 2, nothing, s_1x2, 1};
Private expand_thing rear_2x4_stuff = {{6, 7, 1, 0, 2, 3, 5, 4}, 8, nothing, s2x4, 1};
Private expand_thing rear_col_stuff = {{0, 3, 6, 5, 4, 7, 2, 1}, 8, nothing, s1x8, 0};
Private expand_thing rear_vrbox_stuff = {{1, 0, 3, 2}, 4, nothing, s1x4, 1};
Private expand_thing rear_hrbox_stuff = {{0, 3, 2, 1}, 4, nothing, s1x4, 0};
Private expand_thing rear_gwave_stuff = {{7, 0, 1, 6, 3, 4, 5, 2}, 8, nothing, s2x4, 0};
Private expand_thing rear_qtag_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, nothing, s2x4, 1};
Private expand_thing rear_sqtag_stuff = {{0, 1, 2, 3}, 4, nothing, s1x4, 0};

Private full_expand_thing rear_wave_pair     = {warn__rear_back,       0, &rear_wave_stuff};
Private full_expand_thing rear_bone_pair     = {warn__some_rear_back,  0, &rear_bone_stuff};
Private full_expand_thing rear_wing_pair     = {warn__some_rear_back,  0, &rear_wing_stuff};
Private full_expand_thing rear_c1a_pair      = {warn__some_rear_back,  0, &rear_c1a_stuff};
Private full_expand_thing rear_c1b_pair      = {warn__some_rear_back,  0, &rear_c1b_stuff};
Private full_expand_thing rear_c1c_pair      = {warn__some_rear_back,  0, &rear_c1c_stuff};
Private full_expand_thing rear_c1d_pair      = {warn__some_rear_back,  0, &rear_c1d_stuff};
Private full_expand_thing rear_miniwave_pair = {warn__rear_back,       0, &rear_miniwave_stuff};
Private full_expand_thing rear_2x4_pair      = {warn__rear_back,       0, &rear_2x4_stuff};
Private full_expand_thing rear_col_pair      = {warn__awful_rear_back, 0, &rear_col_stuff};
Private full_expand_thing rear_vrbox_pair    = {warn__awful_rear_back, 0, &rear_vrbox_stuff};
Private full_expand_thing rear_hrbox_pair    = {warn__awful_rear_back, 0, &rear_hrbox_stuff};
Private full_expand_thing rear_gwave_pair    = {warn__rear_back,       0, &rear_gwave_stuff};
Private full_expand_thing rear_qtag_pair     = {warn__rear_back,       0, &rear_qtag_stuff};
Private full_expand_thing rear_sqtag_pair    = {warn__awful_rear_back, 0, &rear_sqtag_stuff};

Private expand_thing step_1x8_stuff = {{0, 7, 6, 1, 4, 3, 2, 5}, 8, nothing, s2x4, 0};
Private expand_thing step_1x4_side_stuff = {{0, 1, 2, 3}, 4, nothing, sdmd, 0};
Private expand_thing step_1x4_stuff = {{0, 3, 2, 1}, 4, nothing, s2x2, 0};
Private expand_thing step_1x2_stuff = {{0, 1}, 2, nothing, s_1x2, 1};
Private expand_thing step_2x2v_stuff = {{1, 2, 3, 0}, 4, nothing, s1x4, 0};
Private expand_thing step_2x2h_stuff = {{0, 1, 2, 3}, 4, nothing, s1x4, 1};
Private expand_thing step_8ch_stuff = {{7, 6, 0, 1, 3, 2, 4, 5}, 8, nothing, s2x4, 1};
Private expand_thing step_li_stuff = {{1, 2, 7, 4, 5, 6, 3, 0}, 8, nothing, s1x8, 0};
Private expand_thing step_tby_stuff = {{5, 6, 7, 0, 1, 2, 3, 4}, 8, nothing, s_qtag, 1};
Private expand_thing step_bone_stuff = {{1, 4, 7, 6, 5, 0, 3, 2}, 8, nothing, s1x8, 0};
Private expand_thing step_rig_stuff = {{2, 7, 4, 5, 6, 3, 0, 1}, 8, nothing, s1x8, 0};

Private full_expand_thing step_1x8_pair      = {(warning_index) 0, 0, &step_1x8_stuff};
Private full_expand_thing step_1x4_side_pair = {(warning_index) 0, 0, &step_1x4_side_stuff};
Private full_expand_thing step_1x4_pair      = {(warning_index) 0, 0, &step_1x4_stuff};
Private full_expand_thing step_2x2v_pair     = {(warning_index) 0, 2*SETUPFLAG__ELONGATE_BIT, &step_2x2v_stuff};
Private full_expand_thing step_2x2h_pair     = {(warning_index) 0, 1*SETUPFLAG__ELONGATE_BIT, &step_2x2h_stuff};
Private full_expand_thing step_8ch_pair      = {(warning_index) 0, 0, &step_8ch_stuff};
Private full_expand_thing step_li_pair       = {(warning_index) 0, 0, &step_li_stuff};
Private full_expand_thing step_tby_pair      = {(warning_index) 0, 0, &step_tby_stuff};
Private full_expand_thing step_1x2_pair      = {(warning_index) 0, 0, &step_1x2_stuff};
Private full_expand_thing step_bone_pair     = {warn__some_touch,  0, &step_bone_stuff};
Private full_expand_thing step_rig_pair      = {warn__some_touch,  0, &step_rig_stuff};




extern void touch_or_rear_back(
   setup *scopy,
   long_boolean did_mirror,
   int callflags1)
{
   int i, j;
   setup stemp;
   full_expand_thing *tptr;
   expand_thing *zptr;
   int directions, livemask;

   if (setup_limits[scopy->kind] < 0) return;          /* We don't understand absurd setups. */

   directions = 0;
   livemask = 0;
   tptr = 0;

   for (i=0; i<=setup_limits[scopy->kind]; i++) {
      int p = scopy->people[i].id1;
      directions = (directions<<2) | (p&3);
      if (p) livemask = (livemask<<2) | 3;
   }

   if (callflags1 & CFLAG1_REAR_BACK_FROM_R_WAVE) {
      if (scopy->kind == s1x4 && (livemask == 0xFF) && (directions == 0x28)) {
         tptr = &rear_wave_pair;          /* Rear back from a wave to facing couples. */
      }
      else if (scopy->kind == s_1x2 && (livemask == 0xF) && (directions == 0x2)) {
         tptr = &rear_miniwave_pair;      /* Rear back from a miniwave to facing people. */
      }
      else if (scopy->kind == s2x4) {
         if ((livemask == 0xFFFF) && (directions == 0x2288)) {
            tptr = &rear_2x4_pair;        /* Rear back from parallel waves to an 8 chain. */
         }
         else if ((livemask == 0xFFFF) && (directions == 0x55FF)) {
            tptr = &rear_col_pair;        /* Rear back from columns to end-to-end single 8-chains. */
         }
      }
      else if (scopy->kind == s2x2) {
         if ((livemask == 0xFF) && (directions == 0x28)) {
            tptr = &rear_vrbox_pair;      /* Rear back from a right-hand box to a single 8 chain. */
         }
         else if ((livemask == 0xFF) && (directions == 0x5F)) {
            tptr = &rear_hrbox_pair;      /* Rear back from a right-hand box to a single 8 chain. */
         }
      }
      else if (scopy->kind == s1x8 && (livemask == 0xFFFF) && (directions == 0x2882)) {
         tptr = &rear_gwave_pair;         /* Rear back from a grand wave to facing lines. */
      }
      else if (scopy->kind == s_bone && livemask == 0xFFFF && ((directions == 0xA802) || (directions == 0x78D2))) {
         /* Centers rear back from a "bone" to lines facing or "split square thru" setup. */
         tptr = &rear_bone_pair;
      }
      else if (scopy->kind == s_rigger && livemask == 0xFFFF && ((directions == 0xA802) || (directions == 0xD872))) {
         /* Ends rear back from a "wing" to lines facing or "split square thru" setup. */
         tptr = &rear_wing_pair;
      }
      else if (scopy->kind == s_c1phan) {
         /* Check for certain people rearing back from C1 phantoms. */
         if ((livemask == 0xCCCCCCCC) && ((directions == 0x884C00C4) || (directions == 0x4C4CC4C4))) {
            tptr = &rear_c1a_pair;
         }
         else if ((livemask == 0x33333333) && ((directions == 0x13223100) || (directions == 0x13313113))) {
            tptr = &rear_c1b_pair;
         }
         else if ((livemask == 0xCCCCCCCC) && ((directions == 0x08CC8044) || (directions == 0x08808008))) {
            tptr = &rear_c1c_pair;
         }
         else if ((livemask == 0x33333333) && ((directions == 0x11203302) || (directions == 0x20200202))) {
            tptr = &rear_c1d_pair;
         }
      }
   }
   else if (callflags1 & CFLAG1_REAR_BACK_FROM_QTAG) {
      if (scopy->kind == s_qtag && livemask == 0xFFFF && ((directions == 0x08A2) || (directions == 0xA802))) {
         tptr = &rear_qtag_pair;         /* Have the centers rear back from a 1/4 tag or 3/4 tag. */
      }
      else if (scopy->kind == sdmd && livemask == 0xFF && ((directions == 0x5F) || (directions == 0xD7))) {
         tptr = &rear_sqtag_pair;         /* Have the centers rear back from a single 1/4 tag or 3/4 tag. */
      }
   }
   else {

      /* We know that (callflags1 & CFLAG1_STEP_TO_WAVE) is true here.
         It is also possible that CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK is true. */

      switch (scopy->kind) {
         case s2x2:
            if ((livemask == 0xFF) && (directions == 0x7D)) {
               tptr = &step_2x2h_pair;
            }
            else if ((livemask == 0xFF) && (directions == 0xA0)) {
               tptr = &step_2x2v_pair;
            }
            else if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
               if ((directions & livemask) != (0x28 & livemask) && (directions & livemask) != (0x5F & livemask))
                  fail("Setup is not left-handed.");
            }
            break;
         case s2x4:
            if ((livemask == 0xFFFF) && (directions == 0x77DD)) {
               tptr = &step_8ch_pair;         /* Check for stepping to parallel waves from an 8 chain. */
            }
            else if ((livemask == 0xFFFF) && (directions == 0xAA00)) {
               tptr = &step_li_pair;          /* Check for stepping to a grand wave from lines facing. */
            }
            else if ((livemask == 0xFFFF) && ((directions == 0xDD77) || (directions == 0x5FF5))) {
               tptr = &step_tby_pair;         /* Check for stepping to a 1/4 tag or 3/4 tag from a DPT or trade-by. */
            }
            else if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
               if ((directions & livemask) != (0x2288 & livemask) && (directions & livemask) != (0x55FF & livemask))
                  fail("Setup is not left-handed.");
            }
            break;
         case s_bone:
            if (livemask == 0xFFFF && (directions == 0xA802)) {
               /* Ends touch from a "bone" to a grand wave. */
               tptr = &step_bone_pair;
            }
            break;
         case s_rigger:
            if (livemask == 0xFFFF && (directions == 0xA802)) {
               /* Centers touch from a "wing" to a grand wave. */
               tptr = &step_rig_pair;
            }
            break;
         case s_1x2:
            if ((livemask == 0xF) && (directions == 0x7)) {
               tptr = &step_1x2_pair;         /* Check for stepping to a miniwave from people facing. */
            }
            else if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
               if ((directions & livemask) != (0x2 & livemask))
                  fail("Setup is not left-handed.");
            }
            break;
         case s1x4:
            /* Check for stepping to a box from a 1x4 single 8 chain -- we allow some phantoms.  This is what makes
               triple columns turn and weave legal in certain interesting cases. */
            if ((livemask == 0xFF) && (directions == 0x7D))
               tptr = &step_1x4_pair;
            else if ((livemask == 0xF0) && (directions == 0x70))
               tptr = &step_1x4_pair;
            else if ((livemask == 0x0F) && (directions == 0x0D))
               tptr = &step_1x4_pair;
            /* Check for stepping to a single 1/4 tag or 3/4 tag from a single-file DPT or trade-by --
               we allow some phantoms, as above. */
            else if ((livemask == 0xFF) && ((directions == 0xD7) || (directions == 0x5F)))
               tptr = &step_1x4_side_pair;
            else if ((livemask == 0x33) && (directions == 0x13))
               tptr = &step_1x4_side_pair;
            else if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
               if ((directions & livemask) != (0x22 & livemask))
                  fail("Setup is not left-handed.");
            }
            break;
         case s1x8:
            if ((livemask == 0xFFFF) && (directions == 0x7DD7)) {
               tptr = &step_1x8_pair;         /* Check for stepping to a column from a 1x8 single 8 chain. */
            }
            else if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
               if ((directions & livemask) != (0x2882 & livemask))
                  fail("Setup is not left-handed.");
            }
            break;
      }
   }

   if (!tptr) return;
   if (tptr->warning) warn(tptr->warning);
   if ((scopy->setupflags & tptr->forbidden_elongation) && (!(scopy->setupflags & SETUPFLAG__NO_CHK_ELONG)))
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




extern void do_matrix_expansion(
   setup *ss,
   unsigned int concprops,
   long_boolean recompute_id)
{
   int i, j;
   expand_thing *eptr;
   setup stemp;

   for (;;) {
      if (concprops & (CONCPROP__NEED_4X4 | CONCPROP__NEED_BLOB | CONCPROP__NEED_4X6)) {
         if (ss->kind == s2x4) {
            eptr = &exp_2x4_4x4_stuff; goto expand_me;
         }
/* ***** This is a kludge to make threesome work!!!! */
         else if (ss->kind == s_qtag) {
            eptr = &exp_qtg_3x4_stuff; goto expand_me;
         }
      }
   
      if (concprops & (CONCPROP__NEED_BLOB)) {
         if (ss->kind == s4x4) {
            eptr = &exp_4x4_blob_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_4DMD) {
         int livemask;
   
         switch (ss->kind) {
            case s1x8:
               eptr = &exp_1x8_4dm_stuff; goto expand_me;
            case s_qtag:
               eptr = &exp_qtg_4dm_stuff; goto expand_me;
            case s4x4:
               for (i=0, j=1, livemask=0; i<16; i++, j<<=1) {
                  if (ss->people[i].id1) livemask |= j;
               }
   
               if (livemask == 0x1717) {
                  eptr = &exp_4x4_4dm_stuff_a; goto expand_me;
               }
               else if (livemask == 0x7171) {
                  eptr = &exp_4x4_4dm_stuff_b; goto expand_me;
               }
         }
      }
      else if (concprops & CONCPROP__NEED_3DMD) {
         if (ss->kind == s_3x1dmd) {         /* Need to expand to real triple diamonds. */
            eptr = &exp_3x1d_3d_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_2X8) {
         switch (ss->kind) {         /* Need to expand to a 2x8. */
            case s2x4:
               eptr = &exp_2x4_2x8_stuff; goto expand_me;
            case s2x6:
               eptr = &exp_2x6_2x8_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_2X6) {
         if (ss->kind == s2x4) {         /* Need to expand to a 2x6. */
            eptr = &exp_2x4_2x6_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_4X6) {
         if (ss->kind == s2x6) {         /* Need to expand to a 4x6. */
            eptr = &exp_2x6_4x6_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_3X4) {
         if (ss->kind == s_qtag) {       /* Need to expand to a 3x4. */
            eptr = &exp_qtg_3x4_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_1X12) {
         switch (ss->kind) {             /* Need to expand to a 1x12. */
            case s1x8:
               eptr = &exp_1x8_1x12_stuff; goto expand_me;
            case s1x10:
               eptr = &exp_1x10_1x12_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_1X16) {
         switch (ss->kind) {             /* Need to expand to a 1x16. */
            case s1x8:
               eptr = &exp_1x8_1x12_stuff; goto expand_me;
            case s1x10:
               eptr = &exp_1x10_1x12_stuff; goto expand_me;
            case s1x12:
               eptr = &exp_1x12_1x14_stuff; goto expand_me;
            case s1x14:
               eptr = &exp_1x14_1x16_stuff; goto expand_me;
         }
      }
   
      /* If get here, we did NOT see any concept that requires a setup expansion. */
   
      return;
   
      expand_me:
   
      /* If get here, we DID see a concept that requires a setup expansion. */
   
      stemp = *ss;
      clear_people(ss);
      if (eptr->rot) {
         for (i=0; i<eptr->size; i++) (void) copy_rot(ss, eptr->source_indices[i], &stemp, i, 033);
         ss->rotation++;
      }
      else {
         for (i=0; i<eptr->size; i++) (void) copy_person(ss, eptr->source_indices[i], &stemp, i);
      }
   
      /* Unsymmetrical selectors are disabled if we expanded the matrix. */
      for (i=0; i<MAX_PEOPLE; i++) ss->people[i].id2 &= ~UNSYM_BITS_TO_CLEAR;
      ss->kind = eptr->outer_kind;

      canonicalize_rotation(ss);
   
      /* Put in position-identification bits (leads/trailers/beaux/belles/centers/ends etc.) */
      if (recompute_id) update_id_bits(ss);
   }
}


Private void normalize_4x4(setup *stuff)
{
   if (!(stuff->people[0].id1 | stuff->people[4].id1 | stuff->people[8].id1 | stuff->people[12].id1)) {
      if (!(stuff->people[5].id1 | stuff->people[6].id1 | stuff->people[13].id1 | stuff->people[14].id1)) {
         compress_setup(&exp_2x4_4x4_stuff, stuff);
      }
      else if (!(stuff->people[1].id1 | stuff->people[2].id1 | stuff->people[9].id1 | stuff->people[10].id1)) {
         stuff->kind = s2x4;
         stuff->rotation++;
         (void) copy_rot(stuff, 1, stuff, 3, 033);     /* careful -- order is important */
         (void) copy_rot(stuff, 2, stuff, 7, 033);
         (void) copy_rot(stuff, 3, stuff, 5, 033);
         (void) copy_rot(stuff, 4, stuff, 6, 033);
         (void) copy_rot(stuff, 0, stuff, 14, 033);
         (void) copy_rot(stuff, 5, stuff, 11, 033);
         (void) copy_rot(stuff, 6, stuff, 15, 033);
         (void) copy_rot(stuff, 7, stuff, 13, 033);
      }
   }
}


Private void normalize_blob(setup *stuff)

{
   if (!(stuff->people[0].id1 | stuff->people[1].id1 | stuff->people[12].id1 | stuff->people[13].id1)) {
      stuff->kind = s4x6;
      (void) copy_person(stuff, 13, stuff, 9);         /* careful -- order is important */
      (void) copy_person(stuff, 9, stuff, 23);
      (void) copy_person(stuff, 23, stuff, 7);
      (void) copy_person(stuff, 7, stuff, 4);
      (void) copy_person(stuff, 4, stuff, 3);
      (void) copy_person(stuff, 3, stuff, 2);
      (void) copy_person(stuff, 2, stuff, 22);
      (void) copy_person(stuff, 22, stuff, 8);
      (void) copy_person(stuff, 8, stuff, 5);
      (void) copy_person(stuff, 1, stuff, 21);
      (void) copy_person(stuff, 21, stuff, 11);
      (void) copy_person(stuff, 11, stuff, 19);
      (void) copy_person(stuff, 19, stuff, 16);
      (void) copy_person(stuff, 16, stuff, 15);
      (void) copy_person(stuff, 15, stuff, 14);
      (void) copy_person(stuff, 14, stuff, 10);
      (void) copy_person(stuff, 10, stuff, 20);
      (void) copy_person(stuff, 20, stuff, 17);

      clear_person(stuff, 5);
      clear_person(stuff, 17);
   }
   else if (!(stuff->people[6].id1 | stuff->people[7].id1 | stuff->people[18].id1 | stuff->people[19].id1)) {
      stuff->kind = s4x6;
      stuff->rotation++;
      (void) copy_rot(stuff, 18, stuff, 0, 033);       /* careful -- order is important */
      (void) copy_rot(stuff, 7, stuff, 10, 033);
      (void) copy_rot(stuff, 10, stuff, 2, 033);
      (void) copy_rot(stuff, 2, stuff, 4, 033);
      (void) copy_rot(stuff, 4, stuff, 9, 033);
      (void) copy_rot(stuff, 9, stuff, 5, 033);
      (void) copy_rot(stuff, 6, stuff, 12, 033);
      (void) copy_rot(stuff, 19, stuff, 22, 033);
      (void) copy_rot(stuff, 22, stuff, 14, 033);
      (void) copy_rot(stuff, 14, stuff, 16, 033);
      (void) copy_rot(stuff, 16, stuff, 21, 033);
      (void) copy_rot(stuff, 21, stuff, 17, 033);

      (void) copy_rot(stuff, 0, stuff, 3, 033);
      (void) copy_rot(stuff, 3, stuff, 8, 033);
      (void) copy_rot(stuff, 8, stuff, 11, 033);
      (void) copy_rot(stuff, 11, stuff, 1, 033);
      (void) copy_person(stuff, 1, stuff, 0);

      (void) copy_rot(stuff, 0, stuff, 23, 033);
      (void) copy_rot(stuff, 23, stuff, 13, 033);
      (void) copy_rot(stuff, 13, stuff, 15, 033);
      (void) copy_rot(stuff, 15, stuff, 20, 033);
      (void) copy_person(stuff, 20, stuff, 0);

      clear_person(stuff, 0);
      clear_person(stuff, 5);
      clear_person(stuff, 12);
      clear_person(stuff, 17);
   }
}




Private void normalize_4x6(setup *stuff)
{
   if (!(stuff->people[0].id1 | stuff->people[11].id1 | stuff->people[18].id1 | stuff->people[17].id1 |
            stuff->people[5].id1 | stuff->people[6].id1 | stuff->people[23].id1 | stuff->people[12].id1)) {
      stuff->kind = s4x4;
      (void) copy_person(stuff, 6, stuff, 15);         /* careful -- order is important */
      (void) copy_person(stuff, 15, stuff, 9);
      (void) copy_person(stuff, 9, stuff, 19);
      (void) copy_person(stuff, 11, stuff, 20);
      (void) copy_person(stuff, 5, stuff, 14);
      (void) copy_person(stuff, 14, stuff, 3);
      (void) copy_person(stuff, 3, stuff, 8);
      (void) copy_person(stuff, 8, stuff, 16);
      (void) copy_person(stuff, 12, stuff, 1);
      (void) copy_person(stuff, 1, stuff, 7);
      (void) copy_person(stuff, 7, stuff, 21);
      (void) copy_person(stuff, 0, stuff, 4);
      (void) copy_person(stuff, 4, stuff, 13);
      (void) copy_person(stuff, 13, stuff, 2);
      (void) copy_person(stuff, 2, stuff, 22);

      canonicalize_rotation(stuff);
   }
   else if (!(stuff->people[0].id1 | stuff->people[1].id1 | stuff->people[2].id1 | stuff->people[3].id1 |
            stuff->people[4].id1 | stuff->people[5].id1 | stuff->people[17].id1 | stuff->people[16].id1 |
            stuff->people[15].id1 | stuff->people[14].id1 | stuff->people[13].id1 | stuff->people[12].id1)) {
      stuff->kind = s2x6;

      (void) copy_person(stuff, 0, stuff, 11);         /* careful -- order is important */
      (void) copy_person(stuff, 11, stuff, 18);
      (void) copy_person(stuff, 1, stuff, 10);
      (void) copy_person(stuff, 10, stuff, 19);
      (void) copy_person(stuff, 2, stuff, 9);
      (void) copy_person(stuff, 9, stuff, 20);
      (void) copy_person(stuff, 3, stuff, 8);
      (void) copy_person(stuff, 8, stuff, 21);
      (void) copy_person(stuff, 4, stuff, 7);
      (void) copy_person(stuff, 7, stuff, 22);
      (void) copy_person(stuff, 5, stuff, 6);
      (void) copy_person(stuff, 6, stuff, 23);
   }
}


Private void normalize_4dmd(setup *stuff)

{
   setup temp;

   if (!(stuff->people[0].id1 | stuff->people[3].id1 | stuff->people[4].id1 | stuff->people[5].id1 |
            stuff->people[8].id1 | stuff->people[11].id1 | stuff->people[12].id1 | stuff->people[13].id1)) {

      /* Danger!  If the people were in side-by-side quarter-tags, turning this into
         a single quarter-tag would require that the outsides slide together.  That
         would be wrong.  The bug show up in cases like
            1P2P; pass the ocean; swing and mix; follow thru; truck twice;
            split phantom twin boxes sets in motion.
         So, if people do not have diamond-like orientation, we go into an "H".
         However, it should be noted that this leads to the following situation:
         From waves, split phantom lines 3/4 flip the line also goes to an "H".
         Is that correct?  We seem to have to put up with it. */

      if (!(((stuff->people[6].id1 | stuff->people[7].id1 | stuff->people[14].id1 | stuff->people[15].id1) & 1) |
            ((stuff->people[1].id1 | stuff->people[2].id1 | stuff->people[9].id1 | stuff->people[10].id1) & 010))) {
         temp = *stuff;

         stuff->kind = s_qtag;
         (void) copy_person(stuff, 0, &temp, 1);
         (void) copy_person(stuff, 1, &temp, 2);
         (void) copy_person(stuff, 2, &temp, 6);
         (void) copy_person(stuff, 3, &temp, 7);
         (void) copy_person(stuff, 4, &temp, 9);
         (void) copy_person(stuff, 5, &temp, 10);
         (void) copy_person(stuff, 6, &temp, 14);
         (void) copy_person(stuff, 7, &temp, 15);
      }
      else {
         temp = *stuff;
         clear_people(stuff);

         stuff->kind = s3x4;
         (void) copy_person(stuff, 0, &temp, 1);
         (void) copy_person(stuff, 3, &temp, 2);
         (void) copy_person(stuff, 4, &temp, 6);
         (void) copy_person(stuff, 5, &temp, 7);
         (void) copy_person(stuff, 6, &temp, 9);
         (void) copy_person(stuff, 9, &temp, 10);
         (void) copy_person(stuff, 10, &temp, 14);
         (void) copy_person(stuff, 11, &temp, 15);
      }
   }
   else if (!(stuff->people[0].id1 | stuff->people[1].id1 | stuff->people[2].id1 | stuff->people[3].id1 |
            stuff->people[8].id1 | stuff->people[9].id1 | stuff->people[10].id1 | stuff->people[11].id1)) {
      compress_setup(&exp_1x8_4dm_stuff, stuff);
   }
   else if (!(stuff->people[4].id1 | stuff->people[5].id1 | stuff->people[6].id1 | stuff->people[7].id1 |
            stuff->people[12].id1 | stuff->people[13].id1 | stuff->people[14].id1 | stuff->people[15].id1)) {
      temp = *stuff;
      clear_people(stuff);

      stuff->kind = s4x4;
      (void) copy_person(stuff, 12, &temp, 0);
      (void) copy_person(stuff, 13, &temp, 1);
      (void) copy_person(stuff, 14, &temp, 2);
      (void) copy_person(stuff, 0, &temp, 3);
      (void) copy_person(stuff, 4, &temp, 8);
      (void) copy_person(stuff, 5, &temp, 9);
      (void) copy_person(stuff, 6, &temp, 10);
      (void) copy_person(stuff, 8, &temp, 11);

      canonicalize_rotation(stuff);
   }
}


/* See if this 3x4 is actually occupied only in spots of a qtag. */

Private void normalize_3x4(setup *stuff)
{
   if ((!stuff->people[0].id1) && (!stuff->people[3].id1) && (!stuff->people[6].id1) && (!stuff->people[9].id1)) {
      stuff->kind = s_qtag;
      (void) copy_person(stuff, 0, stuff, 1);         /* careful -- order is important */
      (void) copy_person(stuff, 1, stuff, 2);
      (void) copy_person(stuff, 2, stuff, 4);
      (void) copy_person(stuff, 3, stuff, 5);
      (void) copy_person(stuff, 4, stuff, 7);
      (void) copy_person(stuff, 5, stuff, 8);
      (void) copy_person(stuff, 6, stuff, 10);
      (void) copy_person(stuff, 7, stuff, 11);
   }
}


/* Check whether "C1 phantom" setup is only occupied in 2x4 spots, and fix it if so. */

Private void normalize_c1_phan(setup *stuff)

{
   if (!(stuff->people[1].id1 | stuff->people[3].id1 | stuff->people[4].id1 | stuff->people[6].id1 |
            stuff->people[9].id1 | stuff->people[11].id1 | stuff->people[12].id1 | stuff->people[14].id1)) {
      stuff->kind = s2x4;
      (void) copy_person(stuff, 1, stuff, 2);                /* careful -- order is important */
      (void) copy_person(stuff, 2, stuff, 7);
      (void) copy_person(stuff, 3, stuff, 5);
      (void) copy_person(stuff, 4, stuff, 8);
      (void) copy_person(stuff, 5, stuff, 10);
      (void) copy_person(stuff, 6, stuff, 15);
      (void) copy_person(stuff, 7, stuff, 13);
   }
   else if (!(stuff->people[0].id1 | stuff->people[2].id1 | stuff->people[5].id1 | stuff->people[7].id1 |
            stuff->people[8].id1 | stuff->people[10].id1 | stuff->people[13].id1 | stuff->people[15].id1)) {
      stuff->kind = s2x4;
      stuff->rotation++;
      (void) copy_rot(stuff, 7, stuff, 1, 033);        /* careful -- order is important */
      (void) copy_rot(stuff, 0, stuff, 4, 033);
      (void) copy_rot(stuff, 2, stuff, 11, 033);
      (void) copy_rot(stuff, 1, stuff, 6, 033);
      (void) copy_rot(stuff, 6, stuff, 3, 033);
      (void) copy_rot(stuff, 3, stuff, 9, 033);
      (void) copy_rot(stuff, 4, stuff, 12, 033);
      (void) copy_rot(stuff, 5, stuff, 14, 033);
   }
}




/* The "action" argument tells how hard we work to remove the outside phantoms.
   When merging the results of "on your own" or "own the so-and-so",
   we set action=normalize_before_merge to work very hard at stripping away
   outside phantoms, so that we can see more readily how to put things together.
   When preparing for an isolated call, that is, "so-and-so do your part, whatever",
   we work at it a little, so we set action=normalize_before_isolated_call.
   For normal usage, we set action=simple_normalize. */
extern void normalize_setup(setup *ss, normalize_action action)
{
   /* Normalize setup by removing outboard phantoms. */

   if (ss->kind == s1x16) {         /* This might leave a 1x14, which might be reduced further. */
      if (!(ss->people[0].id1 | ss->people[8].id1)) {
         compress_setup(&exp_1x14_1x16_stuff, ss);
      }
   }

   if (ss->kind == s1x14) {         /* This might leave a 1x12, which might be reduced further. */
      if (!(ss->people[0].id1 | ss->people[7].id1)) {
         compress_setup(&exp_1x12_1x14_stuff, ss);
      }
   }

   if (ss->kind == s1x12) {         /* This might leave a 1x10, which might be reduced further. */
      if (!(ss->people[0].id1 | ss->people[6].id1)) {
         compress_setup(&exp_1x10_1x12_stuff, ss);
      }
   }

   if (ss->kind == s_bigblob)
      normalize_blob(ss);           /* This might leave a 4x6, which might be reduced further. */

   if (ss->kind == s4x6)
      normalize_4x6(ss);            /* This might leave a 4x4 or 2x6, which might be reduced further. */

   if (ss->kind == s1x10) {
      if (!(ss->people[0].id1 | ss->people[5].id1)) {
         compress_setup(&exp_1x8_1x10_stuff, ss);
      }
   }
   else if (ss->kind == s4x4)
      normalize_4x4(ss);
   else if (ss->kind == s_3dmd) {
      if (!(ss->people[0].id1 | ss->people[2].id1 | ss->people[6].id1 | ss->people[8].id1)) {
         compress_setup(&exp_3x1d_3d_stuff, ss);
      }
   }
   else if (ss->kind == s_4dmd)
      normalize_4dmd(ss);
   else if (ss->kind == s_c1phan)
      normalize_c1_phan(ss);
   else if (ss->kind == s3x4)
      normalize_3x4(ss);
   else if (ss->kind == s2x8) {  /* This might leave a 2x6, which could then be reduced to 2x4, below. */
      if (!(ss->people[7].id1 | ss->people[8].id1 | ss->people[0].id1 | ss->people[15].id1)) {
         compress_setup(&exp_2x6_2x8_stuff, ss);
      }
   }

   if (ss->kind == s2x6) {
      if (!(ss->people[0].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[11].id1)) {
         compress_setup(&exp_2x4_2x6_stuff, ss);
      }
   }

   /* Before a merge, we remove phantoms very aggressively. */

   if (action >= normalize_before_merge) {
      if ((ss->kind == s2x4) && (!(ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1))) {
         /* This reduction is necessary to make "ends only rotate 1/4" work from a DPT, yielding a rigger. */
         ss->kind = s2x2;
         (void) copy_person(ss, 0, ss, 1);
         (void) copy_person(ss, 1, ss, 2);
         (void) copy_person(ss, 2, ss, 5);
         (void) copy_person(ss, 3, ss, 6);
      }
      else if ((ss->kind == s1x8) && (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1))) {
         ss->kind = s1x4;
         (void) copy_person(ss, 0, ss, 3);
         (void) copy_person(ss, 1, ss, 2);
         (void) copy_person(ss, 2, ss, 7);
         (void) copy_person(ss, 3, ss, 6);
      }
      else if ((ss->kind == s_rigger) && (!(ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1))) {
         ss->kind = s2x2;
         (void) copy_person(ss, 2, ss, 4);
         (void) copy_person(ss, 3, ss, 5);
      }
   }

   /* If preparing for a "so-and-so only do your part", we remove outboard phantoms
      more aggressively.  The level "normalize_before_isolated_call" is used for
         <anyone> start, <call>
         <anyone> do your part, <call>
         own the <anyone>, <call1> by <call2>. */

   if (action >= normalize_before_isolated_call) {
      if (ss->kind == s_qtag) {

#ifdef never

/*
We used to do this stuff, with the following comment:
      For example, if we are selecting just the center line of
      a quarter tag, we reduce the setup all the way down to a line.  Normally we
      wouldn't do this, lest we lose phantoms when gluing setups together.
But we have taken it out.  It goes beyond losing phantoms when gluing setups together.
The code you see deleted here would preclude "own the <points>, trade by flip the diamond"
from normal diamonds.  We believe that having the centers of diamonds know that they are
still in diamonds on an "on your own" or "do your part" is exremely important.  In fact,
it's hard to imagine why I thought otherwise.  I can only guess that
"normalize_before_isolated_call" was formerly used for some other operation.

In fact, the comment above: "we remove outboard phantoms more aggressively"
suggestes that.  We are definitely NOT removing outboard phantoms.  We are simply
moving them so as to make the overall setup seem more normal to the remaining live
people.

In any case, let's try it without this.
*/

         if (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1)) {
            /* This makes the centers able to do it. */
            if (!(ss->people[2].id1 | ss->people[6].id1)) {
               ss->kind = s_1x2;
               (void) copy_person(ss, 0, ss, 7);
               (void) copy_person(ss, 1, ss, 3);
            }
            else {
               ss->kind = s1x4;
               (void) copy_person(ss, 0, ss, 6);
               (void) copy_person(ss, 1, ss, 7);
            }
         }
#endif


         if (!(ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1)) {
            /* This makes it possible to do "own the <points>, trade by flip the diamond" from
               normal diamonds. */
            /* We do NOT compress to a 2x2 -- doing so might permit people to
               work with each other across the set when they shouldn't, as in
               "heads pass the ocean; heads recycle while the sides star thru". */
            ss->kind = s2x4;
            ss->rotation++;
            (void) copy_rot(ss, 7, ss, 0, 033);         /* careful -- order is important */
            (void) copy_rot(ss, 3, ss, 4, 033);
            (void) copy_rot(ss, 0, ss, 1, 033);
            (void) copy_rot(ss, 4, ss, 5, 033);
            clear_person(ss, 1);
            clear_person(ss, 5);
            canonicalize_rotation(ss);
         }
      }
      if (ss->kind == sdmd) {
         /* This makes it possible to do "own the <points>, trade by flip the diamond" from
            a single diamond. */
         if (!(ss->people[1].id1 | ss->people[3].id1)) {
            /* We do NOT compress to a 1x2 -- see comment above. */
            ss->kind = s1x4;   /* That's all it takes! */
         }
      }
      else if (ss->kind == s_rigger) {
         /* This makes it possible to do "ends explode" from a rigger. */
         if (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1)) {
            ss->kind = s1x8;
            (void) copy_person(ss, 0, ss, 6);
            (void) copy_person(ss, 1, ss, 7);
            (void) copy_person(ss, 4, ss, 2);
            (void) copy_person(ss, 5, ss, 3);
            clear_person(ss, 2);
            clear_person(ss, 3);
            clear_person(ss, 6);
            clear_person(ss, 7);
         }
      }
      else if (ss->kind == s_ptpd) {
         /* This makes it possible to do "own the <points>, trade by flip the diamond" from
            point-to-point diamonds. */
         if (!(ss->people[1].id1 | ss->people[3].id1 | ss->people[5].id1 | ss->people[7].id1)) {
            ss->kind = s1x8;   /* That's all it takes! */
         }
      }
   }
}



/* Top level move routine. */

extern void toplevelmove(void)
{
   int i;
   parse_block *conceptptr;
   setup new_setup;

   int concept_read_base = 0;
   setup starting_setup = history[history_ptr].state;
   configuration *newhist = &history[history_ptr+1];
   /* Be sure that the amount of written history that we consider to be safely
      written is less than the item we are about to change. */
   if (written_history_items > history_ptr)
      written_history_items = history_ptr;

   starting_setup.setupflags = 0;
   newhist->warnings.bits[0] = 0;
   newhist->warnings.bits[1] = 0;

   /* If we are starting a sequence with the "so-and-so into the center and do whatever"
      flag on, and this call is a "sequence starter", take special action. */

   if (startinfolist[history[history_ptr].centersp].into_the_middle) {
   
      /* If the call is a special sequence starter (e.g. spin a pulley) remove the implicit
         "centers" concept and just do it.  The setup in this case will be a squared set
         with so-and-so moved into the middle, which is what the encoding of these calls
         wants. */
   
      if (parse_state.topcallflags1 & CFLAG1_SEQUENCE_STARTER)
         concept_read_base = 1;
   
      /* If the call is a "split square thru" type of call, we do the same.  We leave the
         "split" concept in place.  Other mechanisms will do the rest. */

      else if ((parse_state.topcallflags1 & CFLAG1_SPLIT_LIKE_SQUARE_THRU) &&
            ((newhist->command_root->next->concept->kind == concept_split) ||
                  ((newhist->command_root->next->concept->kind == concept_left) &&
                  (newhist->command_root->next->next->concept->kind == concept_split))))
         concept_read_base = 1;

      /* If the call is a "split dixie style" type of call, we do something similar,
         but the transformation we perform on the starting setup is highly peculiar. */
   /* ****** bug: ability to handle "left" concept patched out!  It does it wrong! */
   
      else if ((parse_state.topcallflags1 & CFLAG1_SPLIT_LIKE_DIXIE_STYLE) &&
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
   for (i=0; i<MAX_PEOPLE; i++) new_setup.people[i].id2 &= ~UNSYM_BITS_TO_CLEAR;
   newhist->state = new_setup;
   newhist->resolve_flag = resolve_p(&new_setup);
}
