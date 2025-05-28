/* SD -- square dance caller's helper.

    Copyright (C) 1990-1996  William B. Ackerman.

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
   update_id_bits
   initialize_touch_tables
   touch_or_rear_back
   do_matrix_expansion
   normalize_setup
   toplevelmove
*/

#include "sd.h"

typedef struct {
   Const veryshort source_indices[16];
   Const int size;
   Const setup_kind inner_kind;
   Const setup_kind outer_kind;
   Const int rot;
} expand_thing;

typedef struct grilch {
   Const warning_index warning;
   Const int forbidden_elongation;   /* Low 2 bits = elongation bits to forbid; "4" bit = must set elongation. */
   Const expand_thing *expand_lists;
   Const setup_kind kind;
   Const uint32 live;
   Const uint32 dir;
   Const uint32 dirmask;
   struct grilch *next;
} full_expand_thing;

Private expand_thing exp_1x8_4dm_stuff     = {{12, 13, 15, 14, 4, 5, 7, 6}, 8, s1x8, s4dmd, 0};
Private expand_thing exp_qtg_4dm_stuff     = {{1, 2, 6, 7, 9, 10, 14, 15}, 8, s_qtag, s4dmd, 0};
Private expand_thing exp_3x1d_3d_stuff     = {{9, 10, 11, 1, 3, 4, 5, 7}, 8, s3x1dmd, s3dmd, 0};
Private expand_thing exp_1x2_3d_stuff      = {{11, 5}, 2, s1x2, s3dmd, 0};
Private expand_thing exp_4x4_4dm_stuff_a   = {{0, 1, 2, 14, 3, 5, 4, 7, 8, 9, 10, 6, 11, 13, 12, 15}, 16, nothing, s4dmd, 1};
Private expand_thing exp_4x4_4dm_stuff_b   = {{3, 4, 5, 6, 8, 9, 10, 7, 11, 12, 13, 14, 0, 1, 2, 15}, 16, nothing, s4dmd, 0};
Private expand_thing exp_2x4_2x6_stuff     = {{1, 2, 3, 4, 7, 8, 9, 10}, 8, s2x4, s2x6, 0};
Private expand_thing exp_qtg_3x4_stuff     = {{1, 2, 4, 5, 7, 8, 10, 11}, 8, nothing, s3x4, 0};
Private expand_thing exp_2x6_2x8_stuff     = {{1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14}, 12, s2x6, s2x8, 0};
Private expand_thing exp_1x8_1x12_stuff    = {{2, 3, 5, 4, 8, 9, 11, 10}, 8, nothing, s1x12, 0};
Private expand_thing exp_3x4_3x8_stuff     = {{2, 3, 4, 5, 10, 11, 14, 15, 16, 17, 22, 23}, 12, s3x4, s3x8, 0};
Private expand_thing exp_1x8_3x8_stuff     = {{20, 21, 23, 22, 8, 9, 11, 10}, 8, s1x8, s3x8, 0};
Private expand_thing exp_qtag_bigdmd_stuff = {{10, 1, 2, 3, 4, 7, 8, 9}, 8, s_qtag, sbigdmd, 1};
Private expand_thing exp_bone_bigh_stuff   = {{1, 8, 10, 11, 7, 2, 4, 5}, 8, s_bone, sbigh, 0};
Private expand_thing exp_xwv_bigx_stuff    = {{2, 3, 4, 5, 8, 9, 10, 11}, 8, s_crosswave, sbigx, 0};
Private expand_thing exp_1x3d_bigx_stuff   = {{1, 2, 3, 5, 7, 8, 9, 11}, 8, s1x3dmd, sbigx, 0};
Private expand_thing exp_rig_bigrig_stuff  = {{4, 5, 8, 9, 10, 11, 2, 3}, 8, s_rigger, sbigrig, 0};
Private expand_thing exp_bone_bigbone_stuff= {{1, 4, 8, 9, 7, 10, 2, 3}, 8, s_bone, sbigbone, 0};
Private expand_thing exp_1x10_1x12_stuff   = {{1, 2, 3, 4, 5, 7, 8, 9, 10, 11}, 10, s1x10, s1x12, 0};
Private expand_thing exp_1x12_1x14_stuff   = {{1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13}, 12, s1x12, s1x14, 0};
Private expand_thing exp_1x14_1x16_stuff   = {{1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15}, 14, s1x14, s1x16, 0};
Private expand_thing exp_1x8_1x10_stuff    = {{1, 2, 4, 3, 6, 7, 9, 8}, 8, s1x8, s1x10, 0};
Private expand_thing exp_2x6_4x6_stuff     = {{11, 10, 9, 8, 7, 6, 23, 22, 21, 20, 19, 18}, 12, nothing, s4x6, 0};
Private expand_thing exp_4x4_4x6_stuff_a   = {{4, 7, 22, 8, 13, 14, 15, 21, 16, 19, 10, 20, 1, 2, 3, 9}, 16, nothing, s4x6, 0};
Private expand_thing exp_4x4_4x6_stuff_b   = {{1, 2, 3, 9, 4, 7, 22, 8, 13, 14, 15, 21, 16, 19, 10, 20}, 16, nothing, s4x6, 1};
Private expand_thing exp_2x4_2x8_stuff     = {{2, 3, 4, 5, 10, 11, 12, 13}, 8, nothing, s2x8, 0};
Private expand_thing exp_2x4_4x4_stuff     = {{10, 15, 3, 1, 2, 7, 11, 9}, 8, s2x4, s4x4, 0};
Private expand_thing exp_c1phan_4x4_stuff1 = {{-1, 13, -1, 15, -1, 1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11}, 16, s_c1phan, s4x4, 0};
Private expand_thing exp_c1phan_4x4_stuff2 = {{10, -1, 15, -1, 14, -1, 3, -1, 2, -1, 7, -1, 6, -1, 11, -1}, 16, s_c1phan, s4x4, 0};
Private expand_thing exp_4x4_blob_stuff    = {{3, 4, 8, 5, 9, 10, 14, 11, 15, 16, 20, 17, 21, 22, 2, 23}, 16, nothing, s_bigblob, 0};
Private expand_thing exp_4dmd_3x4_stuff    = {{0, 1, 2, 3, 4, 4, 4, 4, 6, 7, 8, 9}, 12, s4dmd, s3x4, 0};
Private expand_thing exp_4dmd_4x4_stuff    = {{12, 13, 14, 0, 1, 1, 1, 1, 4, 5, 6, 8}, 12, s4dmd, s4x4, 0};



Private void compress_setup(expand_thing *thing, setup *stuff)
{
   setup temp = *stuff;

   stuff->kind = thing->inner_kind;
   gather(stuff, &temp, thing->source_indices, thing->size-1, thing->rot * 011);
   stuff->rotation -= thing->rot;
   canonicalize_rotation(stuff);
}



Private id_bit_table spec_3dmd_ctr1x4[] = {
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4},
   {ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4, ID2_NCTR1X4},
   {ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4},
   {ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4, ID2_CTR1X4}};

Private id_bit_table spec_3dmd_ctr1x6[] = {
   {ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4},
   {ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4},
   {ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4},
   {ID2_CTR1X6 | ID2_NCTR1X4, ID2_CTR1X6 | ID2_NCTR1X4, ID2_CTR1X6 | ID2_NCTR1X4, ID2_CTR1X6 | ID2_NCTR1X4},
   {ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4},
   {ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4},
   {ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4},
   {ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4},
   {ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4, ID2_NCTR1X6 | ID2_NCTR1X4},
   {ID2_CTR1X6 | ID2_NCTR1X4, ID2_CTR1X6 | ID2_NCTR1X4, ID2_CTR1X6 | ID2_NCTR1X4, ID2_CTR1X6 | ID2_NCTR1X4},
   {ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4},
   {ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4, ID2_CTR1X6 | ID2_CTR1X4}};


extern void update_id_bits(setup *ss)
{
   int i;
   uint32 livemask, j;
   id_bit_table *ptr;

   for (i=0,j=1,livemask=0 ; i<=setup_attrs[ss->kind].setup_limits ; i++,j<<=1) {
      if (ss->people[i].id1) livemask |= j;
      ss->people[i].id2 &= ~BITS_TO_CLEAR;
   }

   ptr = setup_attrs[ss->kind].id_bit_table_ptr;

   /* Some setups are only recognized for ID bits with certain patterns of population.
       The bit tables make those assumptions, so we have to use the bit tables
       only if those assumptions are satisfied. */

   switch (ss->kind) {
      case s2x6:
         /* **** This isn't really right -- it would allow "outer pairs bingo".
            We really should only allow 2-person calls, unless we say
            "outer triple boxes".  So we're not completely sure what the right thing is. */
         if (livemask != 0x3CFUL && livemask != 0xF3CUL) ptr = (id_bit_table *) 0;
         break;
      case sbigdmd:
         if (livemask != 0xF3CUL && livemask != 0x3CFUL) ptr = (id_bit_table *) 0;
         break;
      case s3x4:

         /* There are two different things we can recognize from here.
            If the setup is populated as an "H", we use a special table
            (*NOT* the usual one picked up from the setup_attrs list)
            that knows about the center 2 and the outer 6 and all that.
            If the setup is populated as offset lines/columns/whatever,
            we use the table from the setup_attrs list, that knows about
            the "outer pairs". */

         if (livemask == 0xE79UL) ptr = id_bit_table_3x4_h;
         else if (livemask != 0xF3CUL && livemask != 0xCF3UL) ptr = (id_bit_table *) 0;
         break;
      case s3dmd:
         /* The standard table requires all points, and centers of center diamond only, occupied.
            But first we look for a few other configurations. */

         if ((livemask & 0xE38UL) == 0xE38UL) ptr = spec_3dmd_ctr1x6;          /* Look for center 1x6 occupied. */
         else if ((livemask & 0xC30UL) == 0xC30UL) ptr = spec_3dmd_ctr1x4;     /* Look for center 1x6 having center 1x4 occupied. */
         else if (livemask != 0x9E7UL) ptr = (id_bit_table *) 0;
         break;
   }

   if (!ptr) return;

   for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++) {
      if (ss->people[i].id1 & BIT_PERSON)
         ss->people[i].id2 |= ptr[i][ss->people[i].id1 & 3];
   }
}


Private expand_thing rear_wave_stuff = {{3, 0, 1, 2}, 4, nothing, s2x2, 0};
Private expand_thing rear_thar_stuff = {{9, 10, 13, 14, 1, 2, 5, 6}, 8, nothing, s4x4, 0};
Private expand_thing rear_ohh_stuff = {{-1, 5, 4, -1, -1, 7, 6, -1, -1, 1, 0, -1, -1, 3, 2, -1}, 16, nothing, s_thar, 0};
Private expand_thing rear_bone_stuff = {{0, 3, 2, 5, 4, 7, 6, 1}, 8, nothing, s2x4, 0};
Private expand_thing rear_bigd_stuff1 = {{-1, -1, 10, 11, 1, 0, -1, -1, 4, 5, 7, 6}, 12, nothing, s3x4, 1};
Private expand_thing rear_bigd_stuff2 = {{8, 9, 10, 11, -1, -1, 2, 3, 4, 5, -1, -1}, 12, nothing, s3x4, 1};
Private expand_thing rear_wing_stuff = {{1, 2, 3, 4, 5, 6, 7, 0}, 8, nothing, s2x4, 0};
Private expand_thing rear_tgl4a_stuff = {{2, 3, 0, 1}, 4, nothing, s2x2, 0};
Private expand_thing rear_tgl4b_stuff = {{2, 3, 1, 0}, 4, nothing, s1x4, 1};
Private expand_thing rear_c1a_stuff = {{0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1}, 16, nothing, s2x4, 0};
Private expand_thing rear_c1b_stuff = {{-1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6}, 16, nothing, s2x4, 0};
Private expand_thing rear_c1c_stuff = {{6, -1, 7, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1}, 16, nothing, s2x4, 1};
Private expand_thing rear_c1d_stuff = {{-1, 7, -1, 6, -1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5}, 16, nothing, s2x4, 1};
Private expand_thing rear_c1e_stuff = {{3, -1, 1, -1, 7, -1, 5, -1, 11, -1, 9, -1, 15, -1, 13, -1}, 16, nothing, s_c1phan, 0};
Private expand_thing rear_c1f_stuff = {{-1, 0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14}, 16, nothing, s_c1phan, 0};
Private expand_thing rear_miniwave_stuff = {{1, 0}, 2, nothing, s1x2, 1};
Private expand_thing rear_2x4_stuff = {{6, 7, 1, 0, 2, 3, 5, 4}, 8, nothing, s2x4, 1};
Private expand_thing rear_col_stuff = {{0, 3, 6, 5, 4, 7, 2, 1}, 8, nothing, s1x8, 0};
Private expand_thing rear_vrbox_stuff = {{1, 0, 3, 2}, 4, nothing, s1x4, 1};
Private expand_thing rear_hrbox_stuff = {{0, 3, 2, 1}, 4, nothing, s1x4, 0};
Private expand_thing rear_gwave_stuff = {{7, 0, 1, 6, 3, 4, 5, 2}, 8, nothing, s2x4, 0};
Private expand_thing rear_qtag_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, nothing, s2x4, 1};
Private expand_thing rear_ptpd_stuff = {{0, 1, 2, 3, 4, 5, 6, 7}, 8, nothing, s1x8, 0};
Private expand_thing rear_sqtag_stuff = {{0, 1, 2, 3}, 4, nothing, s1x4, 0};

Private expand_thing step_1x8_stuff = {{0, 7, 6, 1, 4, 3, 2, 5}, 8, nothing, s2x4, 0};
Private expand_thing step_1x4_side_stuff = {{0, 1, 2, 3}, 4, nothing, sdmd, 0};
Private expand_thing step_1x4_stuff = {{0, 3, 2, 1}, 4, nothing, s2x2, 0};
Private expand_thing step_1x2_stuff = {{0, 1}, 2, nothing, s1x2, 1};
Private expand_thing step_2x2v_stuff = {{1, 2, 3, 0}, 4, nothing, s1x4, 0};
Private expand_thing step_2x2h_stuff = {{0, 1, 2, 3}, 4, nothing, s1x4, 1};
Private expand_thing step_8ch_stuff = {{7, 6, 0, 1, 3, 2, 4, 5}, 8, nothing, s2x4, 1};
Private expand_thing step_li_stuff = {{1, 2, 7, 4, 5, 6, 3, 0}, 8, nothing, s1x8, 0};
Private expand_thing step_bn_stuff = {{0, 7, 2, 1, 4, 3, 6, 5}, 8, nothing, s_bone, 0};
Private expand_thing step_tby_stuff = {{5, 6, 7, 0, 1, 2, 3, 4}, 8, nothing, s_qtag, 1};
Private expand_thing step_2x4_rig_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, nothing, s_rigger, 0};
Private expand_thing step_bone_stuff = {{1, 4, 7, 6, 5, 0, 3, 2}, 8, nothing, s1x8, 0};
Private expand_thing step_bone_rigstuff = {{7, 2, 4, 1, 3, 6, 0, 5}, 8, nothing, s_rigger, 0};
Private expand_thing step_rig_stuff = {{2, 7, 4, 5, 6, 3, 0, 1}, 8, nothing, s1x8, 0};

Private expand_thing step_phan1_stuff = {{-1, 7, -1, 6, -1, 1, -1, 0, -1, 3, -1, 2, -1, 5, -1, 4}, 16, nothing, s2x4, 1};
Private expand_thing step_phan2_stuff = {{7, -1, 6, -1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1}, 16, nothing, s2x4, 1};
Private expand_thing step_phan3_stuff = {{0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6, -1}, 16, nothing, s2x4, 0};
Private expand_thing step_phan4_stuff = {{-1, 1, -1, 0, -1, 3, -1, 2, -1, 5, -1, 4, -1, 7, -1, 6}, 16, nothing, s2x4, 0};

Private expand_thing step_bigd_stuff1 = {{0, 1, 3, 2, -1, -1, 6, 7, 9, 8, -1, -1}, 12, nothing, s2x6, 0};
Private expand_thing step_bigd_stuff2 = {{-1, -1, 3, 2, 4, 5, -1, -1, 9, 8, 10, 11}, 12, nothing, s2x6, 0};
Private expand_thing step_tgl4_stuff = {{2, 3, 0, 1}, 4, nothing, s1x4, 1};
Private expand_thing step_dmd_stuff = {{0, 3, 2, 1}, 4, nothing, s1x4, 0};
Private expand_thing step_qtgctr_stuff = {{7, 0, 2, 1, 3, 4, 6, 5}, 8, nothing, s2x4, 1};

Private full_expand_thing step_2x2v_pair     = {warn__none,       2, &step_2x2v_stuff};
Private full_expand_thing step_2x2h_pair     = {warn__none,       1, &step_2x2h_stuff};
Private full_expand_thing step_dmd_pair      = {warn__some_touch, 0, &step_dmd_stuff};
Private full_expand_thing step_qtgctr_pair   = {warn__some_touch, 0, &step_qtgctr_stuff};


static full_expand_thing touch_init_table1[] = {
   {warn__rear_back,       0, &rear_miniwave_stuff, s1x2,      0xFUL,        0x2UL, ~0UL},      /* Rear back from a miniwave to facing people. */
   {warn__some_rear_back,  0, &rear_tgl4a_stuff, s_trngl4,    0xFFUL,       0xDAUL, ~0UL},      /* Rear back from a 4-person triangle to a "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_tgl4a_stuff, s_trngl4,    0xFFUL,       0xDAUL, ~0UL},      /* Rear back from a 4-person triangle to a "split square thru" setup. */
   {warn__awful_rear_back, 0, &rear_tgl4b_stuff, s_trngl4,    0xFFUL,       0x22UL, ~0UL},      /* Rear back from a 4-person triangle to a single 8 chain. */
   {warn__rear_back,       0, &rear_gwave_stuff, s1x8,      0xFFFFUL,     0x2882UL, ~0UL},      /* Rear back from a grand wave to facing lines. */
   {warn__some_rear_back,  0, &rear_bone_stuff, s_bone,     0xFFFFUL,     0x0802UL, 0x0F0FUL},  /* Centers rear back from a "bone" to lines facing or "split square thru" setup. */
   {warn__rear_back,       0, &rear_ohh_stuff, s4x4,    0x3C3C3C3CUL, 0x1C203408UL, ~0UL},      /* Rear back from an alamo wave to crossed single 8-chains. */
   {warn__rear_back,       0, &rear_2x4_stuff, s2x4,        0xFFFFUL,     0x2288UL, ~0UL},      /* Rear back from parallel waves to an 8 chain. */
   {warn__awful_rear_back, 0, &rear_col_stuff,s2x4,         0xFFFFUL,     0x55FFUL, ~0UL},      /* Rear back from columns to end-to-end single 8-chains. */
   {warn__awful_rear_back, 0, &rear_vrbox_stuff,s2x2,         0xFFUL,       0x28UL, ~0UL},      /* Rear back from a right-hand box to a single 8 chain. */
   {warn__awful_rear_back, 0, &rear_hrbox_stuff,s2x2,         0xFFUL,       0x5FUL, ~0UL},
   {warn__some_rear_back,  0, &rear_wing_stuff,s_rigger,    0xFFFFUL,     0x0802UL, 0x0F0FUL},  /* Ends rear back from a "rigger" to lines facing or "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_bigd_stuff1,sbigdmd,  0x0FF0FFUL,   0x0520F8UL, ~0UL},      /* Some people rear back from horrible "T"'s to couples facing or "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_bigd_stuff1,sbigdmd,  0x0FF0FFUL,   0x082028UL, ~0UL},
   {warn__some_rear_back,  0, &rear_bigd_stuff2,sbigdmd,  0xFF0FF0UL,   0x2F0850UL, ~0UL},      /* Some people rear back from horrible "T"'s to couples facing or "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_bigd_stuff2,sbigdmd,  0xFF0FF0UL,   0x280820UL, ~0UL},
   {warn__rear_back,       0, &rear_thar_stuff, s_thar,     0xFFFFUL,     0x278DUL, ~0UL},      /* Rear back from thar to alamo 8-chain. */
   {warn__some_rear_back,  0, &rear_c1a_stuff,s_c1phan, 0xCCCCCCCCUL, 0x884C00C4UL, ~0UL},      /* Check for certain people rearing back from C1 phantoms. */
   {warn__some_rear_back,  0, &rear_c1a_stuff,s_c1phan, 0xCCCCCCCCUL, 0x4C4CC4C4UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1b_stuff,s_c1phan, 0x33333333UL, 0x13223100UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1b_stuff,s_c1phan, 0x33333333UL, 0x13313113UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1c_stuff,s_c1phan, 0xCCCCCCCCUL, 0x08CC8044UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1c_stuff,s_c1phan, 0xCCCCCCCCUL, 0x08808008UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1d_stuff,s_c1phan, 0x33333333UL, 0x11203302UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1d_stuff,s_c1phan, 0x33333333UL, 0x20200202UL, ~0UL},
   {warn__rear_back,       0, &rear_c1e_stuff,s_c1phan, 0xCCCCCCCCUL, 0x084C80C4UL, ~0UL},
   {warn__rear_back,       0, &rear_c1f_stuff,s_c1phan, 0x33333333UL, 0x13203102UL, ~0UL},
   {warn__rear_back,       4, &rear_wave_stuff, s1x4,         0xFFUL,       0x28UL, ~0UL},      /* Rear back from a wave to facing couples. */
   {warn__none,            0, (expand_thing *) 0, nothing}
};

static full_expand_thing touch_init_table2[] = {
   {warn__rear_back,       0, &rear_qtag_stuff, s_qtag,     0xFFFFUL,     0x08A2UL, ~0UL},      /* Have the centers rear back from a 1/4 tag or 3/4 tag. */
   {warn__rear_back,       0, &rear_qtag_stuff, s_qtag,     0xFFFFUL,     0xA802UL, ~0UL},
   {warn__rear_back,       0, &rear_ptpd_stuff, s_ptpd,     0xFFFFUL,     0x5FF5UL, ~0UL},      /* Have the centers rear back from point-to-point 1/4 tags or 3/4 tags. */
   {warn__rear_back,       0, &rear_ptpd_stuff, s_ptpd,     0xFFFFUL,     0xD77DUL, ~0UL},
   {warn__awful_rear_back, 0, &rear_sqtag_stuff, sdmd,        0xFFUL,       0x5FUL, ~0UL},      /* Have the centers rear back from a single 1/4 tag or 3/4 tag. */
   {warn__awful_rear_back, 0, &rear_sqtag_stuff, sdmd,        0xFFUL,       0xD7UL, ~0UL},
   {warn__none,            0, (expand_thing *) 0, nothing}
};

static full_expand_thing touch_init_table3[] = {
   {warn__some_touch, 0, &step_phan1_stuff,   s_c1phan, 0x33333333UL, 0x13313113UL, ~0UL},
   {warn__some_touch, 0, &step_phan2_stuff,   s_c1phan, 0xCCCCCCCCUL, 0x4C4CC4C4UL, ~0UL},
   {warn__some_touch, 0, &step_phan3_stuff,   s_c1phan, 0xCCCCCCCCUL, 0x08808008UL, ~0UL},
   {warn__some_touch, 0, &step_phan4_stuff,   s_c1phan, 0x33333333UL, 0x20200202UL, ~0UL},
   {warn__some_touch, 0, &step_bigd_stuff1,   sbigdmd,    0xFF0FF0UL,   0x280820UL, ~0UL},      /* Some people touch from horrible "T"'s. */
   {warn__some_touch, 0, &step_bigd_stuff2,   sbigdmd,    0x0FF0FFUL,   0x082028UL, ~0UL},
   {warn__none,       0, &step_li_stuff,      s2x4,         0xFFFFUL,     0xAA00UL, ~0UL},      /* Check for stepping to a grand wave from lines facing. */
   {warn__none,       0, &step_tby_stuff,     s2x4,         0xFFFFUL,     0x5D75UL, 0x7D7DUL},  /* Check for stepping to some kind of 1/4 tag from a DPT or trade-by or whatever. */
   {warn__none,       0, &step_bn_stuff,      s2x4,         0xFFFFUL,     0x6941UL, 0x7D7DUL},  /* Check for stepping to a bone from a squared set or whatever. */
   {warn__some_touch, 0, &step_2x4_rig_stuff, s2x4,         0xFFFFUL,     0x963CUL, ~0UL},      /* Check for stepping to rigger from suitable T-bone. */
   {warn__some_touch, 0, &step_2x4_rig_stuff, s2x4,         0xC3C3UL,     0x8200UL, ~0UL},      /* Same, with missing people. */
   {warn__some_touch, 0, &step_2x4_rig_stuff, s2x4,         0x3C3CUL,     0x143CUL, ~0UL},      /* Same, with missing people. */
   {warn__some_touch, 0, &step_tgl4_stuff,    s_trngl4,       0xFFUL,       0xD7UL, ~0UL},
   {warn__some_touch, 0, &step_bone_stuff,    s_bone,       0xFFFFUL,     0xA802UL, 0xFFFFUL},  /* Ends touch from a "bone" to a grand wave. */
   {warn__none,       0, &step_bone_rigstuff, s_bone,       0xFFFFUL,     0xAD07UL, 0xFFFFUL},  /* All touch from a "bone" to a rigger. */
   {warn__none,       0, &step_bone_rigstuff, s_bone,       0xF0F0UL,     0xAD07UL, 0xF0F0UL},  /* Same, with missing people. */
   {warn__none,       0, &step_bone_rigstuff, s_bone,       0x0F0FUL,     0xAD07UL, 0x0F0FUL},  /* Same, with missing people. */
   {warn__some_touch, 0, &step_rig_stuff,     s_rigger,     0xFFFFUL,     0xA802UL, 0xFFFFUL},  /* Centers touch from a "rigger" to a grand wave. */
   {warn__some_touch, 0, &step_rig_stuff,     s_rigger,     0xF0F0UL,     0xA802UL, 0xF0F0UL},  /* Same, with missing people. */
   {warn__some_touch, 0, &step_rig_stuff,     s_rigger,     0x0F0FUL,     0xA802UL, 0x0F0FUL},  /* Same, with missing people. */
   {warn__none,       0, &step_1x2_stuff,     s1x2,            0xFUL,        0x7UL, 0xFUL},     /* Check for stepping to a miniwave from people facing. */
   {warn__none,       0, &step_1x4_stuff,     s1x4,           0xFFUL,       0x7DUL, 0xFFUL},    /* Check for stepping to a box from a 1x4 single 8 chain -- */
   {warn__none,       0, &step_1x4_stuff,     s1x4,           0xF0UL,       0x7DUL, 0xF0UL},    /*    we allow some phantoms.  This is what makes */
   {warn__none,       0, &step_1x4_stuff,     s1x4,           0x0FUL,       0x7DUL, 0x0FUL},    /*    triple columns turn and weave legal in certain interesting cases. */
   {warn__none,       0, &step_1x4_side_stuff, s1x4,          0xFFUL,       0xD7UL, 0xFFUL},    /* Check for stepping to a single 1/4 tag or 3/4 tag */
   {warn__none,       0, &step_1x4_side_stuff, s1x4,          0xFFUL,       0x5FUL, 0xFFUL},    /*    from a single-file DPT or trade-by -- */
   {warn__none,       0, &step_1x4_side_stuff, s1x4,          0x33UL,       0x13UL, 0x33UL},    /*    we allow some phantoms, as above. */
   {warn__none,       0, &step_1x8_stuff,      s1x8,        0xFFFFUL,     0x7DD7UL, 0xFFFFUL},  /* Check for stepping to a column from a 1x8 single 8 chain. */
   {warn__none,       0, &step_1x8_stuff,      s1x8,        0xF0F0UL,     0x7DD7UL, 0xF0F0UL},
   {warn__none,       0, &step_1x8_stuff,      s1x8,        0x0F0FUL,     0x7DD7UL, 0x0F0FUL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,        0xFFFFUL,     0x77DDUL, 0xFFFFUL},  /* Check for stepping to parallel waves from an 8 chain. */
   {warn__none,       0, &step_8ch_stuff,      s2x4,        0xF0F0UL,     0x77DDUL, 0xF0F0UL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,        0x0F0FUL,     0x77DDUL, 0x0F0FUL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,        0x0FF0UL,     0x77DDUL, 0x0FF0UL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,        0xF00FUL,     0x77DDUL, 0xF00FUL},
   {warn__none,       0, (expand_thing *) 0, nothing}
};


/* Must be a power of 2. */
#define NUM_TOUCH_HASH_BUCKETS 32

static full_expand_thing *touch_hash_table1[NUM_TOUCH_HASH_BUCKETS];
static full_expand_thing *touch_hash_table2[NUM_TOUCH_HASH_BUCKETS];
static full_expand_thing *touch_hash_table3[NUM_TOUCH_HASH_BUCKETS];

extern void initialize_touch_tables(void)
{
   full_expand_thing *tabp;
   int i;

   for (i=0 ; i<NUM_TOUCH_HASH_BUCKETS ; i++) {
      touch_hash_table1[i] = (full_expand_thing *) 0;
      touch_hash_table2[i] = (full_expand_thing *) 0;
      touch_hash_table3[i] = (full_expand_thing *) 0;
   }

   for (tabp = touch_init_table1 ; tabp->kind != nothing ; tabp++) {
      uint32 hash_num = ((tabp->kind + (5*tabp->live)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);
      tabp->next = touch_hash_table1[hash_num];
      touch_hash_table1[hash_num] = tabp;
   }

   for (tabp = touch_init_table2 ; tabp->kind != nothing ; tabp++) {
      uint32 hash_num = ((tabp->kind + (5*tabp->live)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);
      tabp->next = touch_hash_table2[hash_num];
      touch_hash_table2[hash_num] = tabp;
   }

   for (tabp = touch_init_table3 ; tabp->kind != nothing ; tabp++) {
      uint32 hash_num = ((tabp->kind + (5*tabp->live)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);
      tabp->next = touch_hash_table3[hash_num];
      touch_hash_table3[hash_num] = tabp;
   }
}




extern void touch_or_rear_back(
   setup *scopy,
   long_boolean did_mirror,
   int callflags1)
{
   int i;
   uint32 directions, livemask;
   setup stemp;
   Const full_expand_thing *tptr;
   Const expand_thing *zptr;

   if (setup_attrs[scopy->kind].setup_limits < 0) return;          /* We don't understand absurd setups. */

   directions = 0;
   livemask = 0;

   for (i=0; i<=setup_attrs[scopy->kind].setup_limits; i++) {
      uint32 p = scopy->people[i].id1;
      directions = (directions<<2) | (p&3);
      livemask <<= 2;
      if (p) livemask |= 3;
   }

   /* Check first for rearing back from a wave. */

   if (callflags1 & CFLAG1_REAR_BACK_FROM_R_WAVE) {
      uint32 hash_num = ((scopy->kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

      for (tptr = touch_hash_table1[hash_num] ; tptr ; tptr = tptr->next) {
         if (     tptr->kind == scopy->kind &&
                  tptr->live == livemask &&
                  ((tptr->dir ^ directions) & tptr->dirmask) == 0) goto found_tptr;
      }
   }

   /* If we didn't find anything, check for rearing back from a qtag. */

   if (callflags1 & CFLAG1_REAR_BACK_FROM_QTAG) {
      uint32 hash_num = ((scopy->kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

      for (tptr = touch_hash_table2[hash_num] ; tptr ; tptr = tptr->next) {
         if (     tptr->kind == scopy->kind &&
                  tptr->live == livemask &&
                  ((tptr->dir ^ directions) & tptr->dirmask) == 0) goto found_tptr;
      }
   }

   /* Finally, try stepping to a wave. */

   if (callflags1 & CFLAG1_STEP_TO_WAVE) {
      uint32 hash_num = ((scopy->kind + (5*livemask)) * 25) & (NUM_TOUCH_HASH_BUCKETS-1);

      for (tptr = touch_hash_table3[hash_num] ; tptr ; tptr = tptr->next) {
         if (     tptr->kind == scopy->kind &&
                  tptr->live == livemask &&
                  ((tptr->dir ^ directions) & tptr->dirmask) == 0) goto found_tptr;
      }

      /* 2x2 setups are special -- we allow any combination at all in livemask. */

      if (scopy->kind == s2x2 && livemask != 0) {
         if (((directions ^ 0x7DUL) & livemask) == 0) {
            tptr = &step_2x2h_pair;
            goto found_tptr;
         }
         else if (((directions ^ 0xA0UL) & livemask) == 0) {
            tptr = &step_2x2v_pair;
            goto found_tptr;
         }
      }

      switch (scopy->kind) {    /* These ones are not allowed for calls like "spin the top" that have "left_means_touch_or_check" on. */
         case sdmd:
            if (  (livemask & 0x33UL) == 0x33UL &&
                  ((directions ^ 0xA0UL) & livemask) == 0 &&
                  !(callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)) {
               tptr = &step_dmd_pair;
               goto found_tptr;
            }
            break;
         case s_qtag:
            if (  (livemask & 0x0F0FUL) == 0x0F0FUL &&
                  ((directions ^ 0xFD57UL) & livemask) == 0 &&
                  !(callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK)) {
               tptr = &step_qtgctr_pair;
               goto found_tptr;
            }
            break;
      }

      /* We didn't find anything at all.  But we still need to raise an error
         if the caller said "left spin the top" when we were in a right-hand wave. */

      if ((callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) && did_mirror) {
         uint32 aa;
         uint32 bb = ~0UL;
         long_boolean other_test = TRUE;

         switch (scopy->kind) {
            case s2x2:
               other_test = (directions & livemask) != (0x5FUL & livemask);
               aa = 0x28UL;
               break;
            case s2x4:
               other_test = (directions & livemask) != (0x55FFUL & livemask);
               aa = 0x2288UL;
               break;
            case s_bone:   aa = 0x58F2UL; break;
            case s_rigger: aa = 0x58F2UL; break;
            case s1x2:     aa = 0x2UL; break;
            case s1x4:     aa = 0x28UL; break;
            case s1x8:     aa = 0x2882UL; break;
            case s_qtag:   aa = 0x0802UL; bb = 0x0F0F; break;
         }
         if ((directions & livemask & bb) != (aa & livemask) && other_test)
            fail("Setup is not left-handed.");
      }
   }

   return;

   found_tptr:

   warn(tptr->warning);

   if ((tptr->forbidden_elongation & 4) && scopy->cmd.cmd_misc_flags & CMD_MISC__DOING_ENDS)
      scopy->cmd.prior_elongation_bits = (scopy->cmd.prior_elongation_bits & (~3)) | ((scopy->rotation+1) & 3);

   if ((scopy->cmd.prior_elongation_bits & tptr->forbidden_elongation & 3) && (!(scopy->cmd.cmd_misc_flags & CMD_MISC__NO_CHK_ELONG)))
      fail("People are too far away to work with each other on this call.");

   zptr = tptr->expand_lists;
   scopy->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   stemp = *scopy;
   clear_people(scopy);
   scatter(scopy, &stemp, zptr->source_indices, zptr->size-1, zptr->rot * 033);
   scopy->rotation += zptr->rot;
   scopy->kind = zptr->outer_kind;
   canonicalize_rotation(scopy);
}




extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   long_boolean recompute_id)
{
   int i;
   uint32 needprops = concprops & CONCPROP__NEED_MASK;
   expand_thing *eptr;
   setup stemp;

   for (;;) {
      if (     needprops == CONCPROP__NEEDK_4X4 ||
               needprops == CONCPROP__NEEDK_4X4_1X16 ||
               needprops == CONCPROP__NEEDK_BLOB ||
               needprops == CONCPROP__NEEDK_4X6 ||
               needprops == CONCPROP__NEEDK_TWINDMD ||
               needprops == CONCPROP__NEEDK_TWINQTAG) {
         if (ss->kind == s2x4) {
            eptr = &exp_2x4_4x4_stuff; goto expand_me;
         }
         else if (ss->kind == s_c1phan) {
            if (!(ss->people[0].id1 | ss->people[2].id1 | ss->people[4].id1 | ss->people[6].id1 |
                     ss->people[8].id1 | ss->people[10].id1 | ss->people[12].id1 | ss->people[14].id1)) {
               eptr = &exp_c1phan_4x4_stuff1; warn(warn__check_4x4); goto expand_me;
            }
            else if (!(ss->people[1].id1 | ss->people[3].id1 | ss->people[5].id1 | ss->people[7].id1 |
                     ss->people[9].id1 | ss->people[11].id1 | ss->people[13].id1 | ss->people[15].id1)) {
               eptr = &exp_c1phan_4x4_stuff2; warn(warn__check_4x4); goto expand_me;
            }
         }
/* ***** This is a kludge to make threesome work!!!! */
         else if (ss->kind == s_qtag) {
            eptr = &exp_qtg_3x4_stuff; goto expand_me;
         }
         else if (ss->kind == s4dmd) {
            if (!(ss->people[4].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[7].id1 |
                     ss->people[12].id1 | ss->people[13].id1 | ss->people[14].id1 | ss->people[15].id1)) {
               eptr = &exp_4dmd_4x4_stuff; warn(warn__check_4x4); goto expand_me;
            }
         }
      }

      if (needprops == CONCPROP__NEEDK_3X4 || needprops == CONCPROP__NEEDK_3X8 || (concprops & CONCPROP__NEED_TRIPLE_1X4)) {
         if (ss->kind == s_qtag) {
            eptr = &exp_qtg_3x4_stuff; goto expand_me;
         }
         else if (ss->kind == s4dmd) {
            if (!(ss->people[4].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[7].id1 |
                     ss->people[12].id1 | ss->people[13].id1 | ss->people[14].id1 | ss->people[15].id1)) {
               eptr = &exp_4dmd_3x4_stuff; goto expand_me;
            }
         }
      }

      if (needprops == CONCPROP__NEEDK_BLOB) {
         if (ss->kind == s4x4) {
            eptr = &exp_4x4_blob_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_4DMD) {
         uint32 livemask, j;

         switch (ss->kind) {
            case s1x8:
               eptr = &exp_1x8_4dm_stuff; goto expand_me;
            case s_qtag:
               eptr = &exp_qtg_4dm_stuff; goto expand_me;
            case s4x4:
               for (i=0, j=1, livemask=0; i<16; i++, j<<=1) {
                  if (ss->people[i].id1) livemask |= j;
               }

               if (livemask == 0x1717UL) {
                  eptr = &exp_4x4_4dm_stuff_a; goto expand_me;
               }
               else if (livemask == 0x7171UL) {
                  eptr = &exp_4x4_4dm_stuff_b; goto expand_me;
               }
         }
      }
      else if (needprops == CONCPROP__NEEDK_3DMD) {
         switch (ss->kind) {                /* Need to expand to real triple diamonds. */
            case s3x1dmd:
               eptr = &exp_3x1d_3d_stuff; goto expand_me;
            case s1x2:
               eptr = &exp_1x2_3d_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_2X8) {
         switch (ss->kind) {
            case s2x4:
               eptr = &exp_2x4_2x8_stuff; goto expand_me;
            case s2x6:
               eptr = &exp_2x6_2x8_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_3X8) {
         switch (ss->kind) {
            case s3x4:
               eptr = &exp_3x4_3x8_stuff; goto expand_me;
            case s1x8:
               eptr = &exp_1x8_3x8_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_2X6) {
         if (ss->kind == s2x4) {
            eptr = &exp_2x4_2x6_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_4X6) {
         if (ss->kind == s2x6) {
            eptr = &exp_2x6_4x6_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_TWINDMD || needprops == CONCPROP__NEEDK_TWINQTAG) {
         if (ss->kind == s2x6) {
            eptr = &exp_2x6_4x6_stuff; goto expand_me;
         }
         else if (ss->kind == s4x4) {
            uint32 ctrs = ss->people[3].id1 | ss->people[7].id1 | ss->people[11].id1 | ss->people[15].id1;

            if (ctrs != 0 && (ctrs & 011) != 011) {
               if (needprops == CONCPROP__NEEDK_TWINQTAG) ctrs ^= 1;
               eptr = (ctrs & 1) ? &exp_4x4_4x6_stuff_b : &exp_4x4_4x6_stuff_a; goto expand_me;
            }
         }
      }
      else if (concprops & CONCPROP__NEED_CTR_2X2) {
         switch (ss->kind) {
            case s2x4:
               eptr = &exp_2x4_2x6_stuff; goto expand_me;
            case s_rigger:
               eptr = &exp_rig_bigrig_stuff; goto expand_me;
         }
      }
      else if (concprops & CONCPROP__NEED_END_2X2) {
         switch (ss->kind) {
            case s2x4:
               eptr = &exp_2x4_2x6_stuff; goto expand_me;
            case s_qtag:
               eptr = &exp_qtag_bigdmd_stuff; goto expand_me;
            case s_bone:
               eptr = &exp_bone_bigbone_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_1X12) {
         switch (ss->kind) {
            case s1x8:
               eptr = &exp_1x8_1x12_stuff; goto expand_me;
            case s1x10:
               eptr = &exp_1x10_1x12_stuff; goto expand_me;
         }
      }
      else if (concprops & (CONCPROP__NEED_TRIPLE_1X4)) {
         switch (ss->kind) {
            case s1x8:
               eptr = &exp_1x8_1x12_stuff; goto expand_me;
            case s1x10:
               eptr = &exp_1x10_1x12_stuff; goto expand_me;
            case s_bone:
               eptr = &exp_bone_bigh_stuff; goto expand_me;
            case s_crosswave:
               eptr = &exp_xwv_bigx_stuff; goto expand_me;
            case s1x3dmd:
               eptr = &exp_1x3d_bigx_stuff; goto expand_me;
         }
      }
      else if (concprops & (CONCPROP__NEED_CTR_1X4)) {
         switch (ss->kind) {
            case s_qtag:
               eptr = &exp_qtag_bigdmd_stuff; goto expand_me;
            case s_bone:
               eptr = &exp_bone_bigbone_stuff; goto expand_me;
            case s1x8:
               eptr = &exp_1x8_1x12_stuff; goto expand_me;
            case s1x10:
               eptr = &exp_1x10_1x12_stuff; goto expand_me;
            case s_crosswave:
               eptr = &exp_xwv_bigx_stuff; goto expand_me;
            case s1x3dmd:
               eptr = &exp_1x3d_bigx_stuff; goto expand_me;
         }
      }
      else if (concprops & (CONCPROP__NEED_END_1X4)) {
         switch (ss->kind) {
            /* **** also create 4x4 */
            case s_rigger:
               eptr = &exp_rig_bigrig_stuff; goto expand_me;
            case s1x8:
               eptr = &exp_1x8_1x12_stuff; goto expand_me;
            case s1x10:
               eptr = &exp_1x10_1x12_stuff; goto expand_me;
            case s_bone:
               eptr = &exp_bone_bigh_stuff; goto expand_me;
            case s_crosswave:
               eptr = &exp_xwv_bigx_stuff; goto expand_me;
            case s1x3dmd:
               eptr = &exp_1x3d_bigx_stuff; goto expand_me;
         }
      }
      else if (needprops == CONCPROP__NEEDK_1X16 || needprops == CONCPROP__NEEDK_4X4_1X16) {
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
      scatter(ss, &stemp, eptr->source_indices, eptr->size-1, eptr->rot * 033);
      ss->rotation += eptr->rot;

      /* Global selectors are disabled if we expanded the matrix. */
      for (i=0; i<MAX_PEOPLE; i++) ss->people[i].id2 &= ~GLOB_BITS_TO_CLEAR;
      ss->kind = eptr->outer_kind;

      canonicalize_rotation(ss);

      /* Put in position-identification bits (leads/trailers/beaus/belles/centers/ends etc.) */
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
   if (!(stuff->people[0].id1 | stuff->people[3].id1 | stuff->people[4].id1 | stuff->people[5].id1 |
            stuff->people[8].id1 | stuff->people[11].id1 | stuff->people[12].id1 | stuff->people[13].id1)) {
      compress_setup(&exp_qtg_4dm_stuff, stuff);
   }
   else if (!(stuff->people[0].id1 | stuff->people[1].id1 | stuff->people[2].id1 | stuff->people[3].id1 |
            stuff->people[8].id1 | stuff->people[9].id1 | stuff->people[10].id1 | stuff->people[11].id1)) {
      compress_setup(&exp_1x8_4dm_stuff, stuff);
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

   if (ss->kind == sfat2x8)
      ss->kind = s2x8;     /* That's all it takes! */

   if (ss->kind == swide4x4)
      ss->kind = s4x4;     /* That's all it takes! */

   if (ss->kind == sbigh) {
      if (!(ss->people[0].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[9].id1)) {
         compress_setup(&exp_bone_bigh_stuff, ss);
      }
   }

   if (ss->kind == sbigx) {
      if (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[6].id1 | ss->people[7].id1)) {
         compress_setup(&exp_xwv_bigx_stuff, ss);
      }
      else if (!(ss->people[0].id1 | ss->people[4].id1 | ss->people[6].id1 | ss->people[10].id1)) {
         compress_setup(&exp_1x3d_bigx_stuff, ss);
      }
   }

   if (ss->kind == sbigrig) {
      if (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[6].id1 | ss->people[7].id1)) {
         compress_setup(&exp_rig_bigrig_stuff, ss);
      }
   }

   if (ss->kind == sbigbone) {
      if (!(ss->people[0].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[11].id1)) {
         compress_setup(&exp_bone_bigbone_stuff, ss);
      }
   }

   if (ss->kind == sbigdmd) {         /* This might leave a qtag, which might be reduced further. */
      if (!(ss->people[0].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[11].id1)) {
         compress_setup(&exp_qtag_bigdmd_stuff, ss);
      }
   }

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
   else if (ss->kind == s3dmd) {
      if (!(ss->people[0].id1 | ss->people[2].id1 | ss->people[6].id1 | ss->people[8].id1)) {
         compress_setup(&exp_3x1d_3d_stuff, ss);
      }
   }
   else if (ss->kind == s4dmd)
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
   else if (ss->kind == s3x8) {
      if (!(   ss->people[0].id1  | ss->people[1].id1  | ss->people[6].id1  | ss->people[7].id1 |
               ss->people[20].id1 | ss->people[21].id1 | ss->people[9].id1  | ss->people[8].id1 |
               ss->people[19].id1 | ss->people[18].id1 | ss->people[13].id1 | ss->people[12].id1)) {
         compress_setup(&exp_3x4_3x8_stuff, ss);
      }
      else if (!(   ss->people[0].id1  | ss->people[1].id1  | ss->people[2].id1  | ss->people[3].id1 |
                    ss->people[4].id1  | ss->people[4].id1  | ss->people[6].id1  | ss->people[7].id1 |
                    ss->people[12].id1 | ss->people[13].id1 | ss->people[14].id1 | ss->people[15].id1 |
                    ss->people[16].id1 | ss->people[17].id1 | ss->people[18].id1 | ss->people[19].id1)) {
         compress_setup(&exp_1x8_3x8_stuff, ss);
      }
   }

   if (ss->kind == s2x6) {
      if (!(ss->people[0].id1 | ss->people[5].id1 | ss->people[6].id1 | ss->people[11].id1)) {
         compress_setup(&exp_2x4_2x6_stuff, ss);
      }
   }

   /* Before a merge, we remove phantoms very aggressively.  This is the highest level. */

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
         if (!(ss->people[0].id1 | ss->people[2].id1)) {   /* See if we can compress further. */
            ss->kind = s1x2;
            (void) copy_person(ss, 0, ss, 1);
            (void) copy_person(ss, 1, ss, 3);
         }
      }
      else if ((ss->kind == s_rigger) && (!(ss->people[2].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[7].id1))) {
         ss->kind = s2x2;
         (void) copy_person(ss, 2, ss, 4);
         (void) copy_person(ss, 3, ss, 5);
      }
      else if ((ss->kind == s_bone) && (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1))) {
         ss->kind = s1x4;
         (void) copy_person(ss, 0, ss, 6);
         (void) copy_person(ss, 1, ss, 7);
      }
      else if ((ss->kind == s_crosswave) && (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[4].id1 | ss->people[5].id1 | ss->people[6].id1))) {
         ss->kind = s1x2;
         ss->rotation++;
         (void) copy_rot(ss, 0, ss, 3, 033);
         (void) copy_rot(ss, 1, ss, 7, 033);
         canonicalize_rotation(ss);
      }
      else if ((ss->kind == s_crosswave) && (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1))) {
         ss->kind = s1x4;
         ss->rotation++;
         (void) copy_rot(ss, 0, ss, 2, 033);
         (void) copy_rot(ss, 1, ss, 3, 033);
         (void) copy_rot(ss, 2, ss, 6, 033);
         (void) copy_rot(ss, 3, ss, 7, 033);
         canonicalize_rotation(ss);
      }
      else if ((ss->kind == s_qtag) && (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1))) {
         ss->kind = s1x4;
         (void) copy_person(ss, 0, ss, 6);
         (void) copy_person(ss, 1, ss, 7);
         (void) copy_person(ss, 2, ss, 2);
         (void) copy_person(ss, 3, ss, 3);
      }
      else if ((ss->kind == s_galaxy) && (!(ss->people[0].id1 | ss->people[2].id1 | ss->people[4].id1 | ss->people[6].id1))) {
         ss->kind = s2x2;
         (void) copy_person(ss, 0, ss, 1);
         (void) copy_person(ss, 1, ss, 3);
         (void) copy_person(ss, 2, ss, 5);
         (void) copy_person(ss, 3, ss, 7);
      }
      else if ((ss->kind == s3x1dmd) && (!(ss->people[0].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[7].id1))) {
         ss->kind = s1x4;
         (void) copy_person(ss, 0, ss, 1);
         (void) copy_person(ss, 1, ss, 2);
         (void) copy_person(ss, 2, ss, 5);
         (void) copy_person(ss, 3, ss, 6);
      }
      else if ((ss->kind == s3x1dmd) && (!(ss->people[3].id1 | ss->people[7].id1))) {
         ss->kind = s1x6;
         (void) copy_person(ss, 3, ss, 4);
         (void) copy_person(ss, 4, ss, 5);
         (void) copy_person(ss, 5, ss, 6);
      }
      else if ((ss->kind == s1x3dmd) && (!(ss->people[0].id1 | ss->people[4].id1))) {
         ss->kind = s_1x2dmd;
         (void) copy_person(ss, 0, ss, 1);
         (void) copy_person(ss, 1, ss, 2);
         (void) copy_person(ss, 2, ss, 3);
         (void) copy_person(ss, 3, ss, 5);
         (void) copy_person(ss, 4, ss, 6);
         (void) copy_person(ss, 5, ss, 7);
      }
   }

   /* If preparing for a "so-and-so only do your part", we remove outboard phantoms
      fairly aggressively.  The level "normalize_before_isolated_call" is used for
         <anyone> start, <call>
         <anyone> do your part, <call>
         own the <anyone>, <call1> by <call2>. */

   if (action >= normalize_before_isolated_call) {
      if (ss->kind == s_qtag || ss->kind == s_hrglass) {

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
               ss->kind = s1x2;
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
      else if (ss->kind == sdmd) {
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
      else if (ss->kind == sbigdmd) {
         /* If only the center 1x4 is present, turn it into a 3x4.  If only the "wings" are
            present, turn it into a 2x6. */
         if (!(   ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1 |
                  ss->people[6].id1 | ss->people[7].id1 | ss->people[10].id1 | ss->people[11].id1)) {
            ss->kind = s3x4;
            ss->rotation++;
            (void) copy_rot(ss, 10, ss, 2, 033);
            (void) copy_rot(ss, 11, ss, 3, 033);
            (void) copy_rot(ss, 4, ss, 8, 033);
            (void) copy_rot(ss, 5, ss, 9, 033);
            clear_person(ss, 2);
            clear_person(ss, 3);
            clear_person(ss, 8);
            clear_person(ss, 9);
            canonicalize_rotation(ss);
         }
         else if (!(ss->people[2].id1 | ss->people[3].id1 | ss->people[8].id1 | ss->people[9].id1)) {
            ss->kind = s2x6;     /* That's all! */
         }
      }
      else if ((ss->kind == s3x1dmd) && (!(ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1))) {
         if (!(ss->people[3].id1 | ss->people[7].id1)) {
            ss->kind = s1x2;
            (void) copy_person(ss, 0, ss, 2);
            (void) copy_person(ss, 1, ss, 6);
         }
         else {
            ss->kind = sdmd;
            ss->rotation++;
            (void) copy_rot(ss, 0, ss, 3, 033);
            (void) copy_rot(ss, 1, ss, 6, 033);
            (void) copy_rot(ss, 3, ss, 2, 033);
            (void) copy_rot(ss, 2, ss, 7, 033);
            canonicalize_rotation(ss);
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

   canonicalize_rotation(ss);
}



/* Top level move routine. */

extern void toplevelmove(void)
{
   int i;
   setup new_setup;

   setup starting_setup = history[history_ptr].state;
   configuration *newhist = &history[history_ptr+1];
   parse_block *conceptptr = newhist->command_root;

   /* Be sure that the amount of written history that we consider to be safely
      written is less than the item we are about to change. */
   if (written_history_items > history_ptr)
      written_history_items = history_ptr;

   starting_setup.cmd.cmd_misc_flags = 0;
   starting_setup.cmd.cmd_misc2_flags = 0;
   starting_setup.cmd.cmd_frac_flags = 0;
   starting_setup.cmd.cmd_assume.assumption = cr_none;
   starting_setup.cmd.cmd_assume.assump_cast = 0;
   starting_setup.cmd.prior_elongation_bits = 0;
   starting_setup.cmd.skippable_concept = (parse_block *) 0;
   newhist->warnings.bits[0] = 0;
   newhist->warnings.bits[1] = 0;

   /* If we are starting a sequence with the "so-and-so into the center and do whatever"
      flag on, and this call is a "sequence starter", take special action. */

   if (startinfolist[history[history_ptr].centersp].into_the_middle) {

      /* If the call is a special sequence starter (e.g. spin a pulley) remove the implicit
         "centers" concept and just do it.  The setup in this case will be a squared set
         with so-and-so moved into the middle, which is what the encoding of these calls
         wants.
      If the call is a "split square thru" or "split dixie style" type of call, and the
         "split" concept has been given, possibly preceded by "left" we do the same.
         We leave the "split" concept in place.  Other mechanisms will do the rest. */

      if (
               (parse_state.topcallflags1 & CFLAG1_SEQUENCE_STARTER) ||
               ((parse_state.topcallflags1 & (CFLAG1_SPLIT_LIKE_SQUARE_THRU | CFLAG1_SPLIT_LIKE_DIXIE_STYLE)) &&
                     ((newhist->command_root->next->concept->kind == concept_split) ||
                           ((newhist->command_root->next->concept->kind == concept_left) &&
                           (newhist->command_root->next->next->concept->kind == concept_split)))))
         conceptptr = conceptptr->next;
   }

   /* Clear a few things.  We do NOT clear the warnings, because some (namely the
      "concept not allowed at this level" warning) may have already been logged. */
   newhist->centersp = 0;
   newhist->draw_pic = FALSE;
   newhist->resolve_flag.kind = resolve_none;

   /* Set the selector to "uninitialized", so that, if we do a call like "run", we
      will query the user to find out who is selected. */

   current_options.who = selector_uninitialized;

   /* Put in identification bits for global/unsymmetrical stuff, if possible. */

   for (i=0; i<MAX_PEOPLE; i++) starting_setup.people[i].id2 &= ~GLOB_BITS_TO_CLEAR;

   if (!(starting_setup.result_flags & RESULTFLAG__IMPRECISE_ROT)) {    /* Can't do it if rotation is not known. */
      if (setup_attrs[starting_setup.kind].setup_limits >= 0) {     /* Put in headliner/sideliner stuff if possible. */
         for (i=0; i<=setup_attrs[starting_setup.kind].setup_limits; i++) {
            if (starting_setup.people[i].id1 & BIT_PERSON) {
               if ((starting_setup.people[i].id1 + starting_setup.rotation) & 1)
                  starting_setup.people[i].id2 |= ID2_SIDELINE;
               else if ((starting_setup.people[i].id1 + starting_setup.rotation) & 2)
                  starting_setup.people[i].id2 |= ID2_HEADLINE|ID2_FACEFRONT;
               else
                  starting_setup.people[i].id2 |= ID2_HEADLINE|ID2_FACEBACK;
            }
         }
      }

      if (starting_setup.kind == s2x4) {
         uint32 nearbit = 0;
         uint32 farbit = 0;

         if (starting_setup.rotation & 1) {
            nearbit = ID2_NEARBOX;
            farbit = ID2_FARBOX;
         }
         else {
            uint32 tbonetest = 0;

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
               starting_setup.people[i].id2 |= ((i + (starting_setup.rotation << 1)) & 4) ? nearbit : farbit;
         }
      }
      else if (starting_setup.kind == s_c1phan) {
         uint32 nearbit = 0;
         uint32 farbit = 0;
         uint32 tbonetest[2];
         uint32 livemask;
         uint32 j;

         tbonetest[0] = 0;
         tbonetest[1] = 0;

         for (i=0, j=1, livemask = 0; i<16; i++, j<<=1) {
            if (starting_setup.people[i].id1) livemask |= j;
            tbonetest[i>>3] |= starting_setup.people[i].id1;
         }

         if (livemask == 0x5AA5UL) {
            nearbit = ID2_NEARBOX;

            if (!(tbonetest[0] & 1)) {
               farbit = ID2_FARLINE;
            }
            else if (!(tbonetest[0] & 010)) {
               farbit = ID2_FARCOL;
            }
         }
         else if (livemask == 0xA55AUL) {
            farbit = ID2_FARBOX;

            if (!(tbonetest[1] & 1)) {
               nearbit = ID2_NEARLINE;
            }
            else if (!(tbonetest[1] & 010)) {
               nearbit = ID2_NEARCOL;
            }
         }

         for (i=0; i<16; i++) {
            if (starting_setup.people[i].id1 & BIT_PERSON)
               starting_setup.people[i].id2 |= (i & 8) ? nearbit : farbit;
         }
      }
   }

   /* Put in position-identification bits (leads/trailers/beaus/belles/centers/ends etc.) */
   update_id_bits(&starting_setup);
   starting_setup.cmd.parseptr = conceptptr;
   starting_setup.cmd.callspec = NULLCALLSPEC;
   starting_setup.cmd.cmd_final_flags = 0;
   move(&starting_setup, FALSE, &new_setup);
   if (new_setup.kind == s1p5x8)
      fail("Can't go into a 50% offset 1x8.");

   /* Remove outboard phantoms from the resulting setup. */

   normalize_setup(&new_setup, simple_normalize);
   for (i=0; i<MAX_PEOPLE; i++) new_setup.people[i].id2 &= ~GLOB_BITS_TO_CLEAR;
   newhist->state = new_setup;
   /* Once rotation is imprecise, it is always imprecise.  Same for the other flags copied here. */
   newhist->state.result_flags |= starting_setup.result_flags &
         (RESULTFLAG__IMPRECISE_ROT|RESULTFLAG__ACTIVE_PHANTOMS_ON|RESULTFLAG__ACTIVE_PHANTOMS_OFF);
   newhist->resolve_flag = resolve_p(&newhist->state);
}
