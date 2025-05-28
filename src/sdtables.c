/* SD -- square dance caller's helper.

    Copyright (C) 1990-1997  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 31. */

/* This defines the following external variables:
   getout_strings
   filename_strings
   level_threshholds
   higher_acceptable_level
   menu_names
   id_bit_table_bigdmd_wings
   id_bit_table_bigbone_wings
   id_bit_table_bighrgl_wings
   id_bit_table_bigdhrgl_wings
   id_bit_table_3x4_h
   id_bit_table_3dmd_ctr1x6
   id_bit_table_3dmd_ctr1x4
   map2x4_2x4
   map2x4_2x4v
   mapgnd1x2_1x2
   mapgnd1x2_1x2r
   conc_init_table
   setup_attrs
   begin_sizes
   startinfolist
   map_b6_trngl
   map_s6_trngl
   map_bone_trngl4
   map_rig_trngl4
   map_s8_tgl4
   map_p8_tgl4
   map_spndle_once_rem
   map_1x3dmd_once_rem
   map_phan_trngl4a
   map_phan_trngl4b
   map_lh_zzztgl
   map_rh_zzztgl
   map_2x2v
   map_2x4_magic
   map_qtg_magic
   map_qtg_intlk
   map_qtg_magic_intlk
   map_ptp_magic
   map_ptp_intlk
   map_ptp_magic_intlk
   map_2x4_diagonal
   map_2x4_int_pgram
   map_2x4_trapezoid
   map_3x4_2x3_intlk
   map_3x4_2x3_conc
   map_4x4_ns
   map_4x4_ew
   map_phantom_box
   map_intlk_phantom_box
   map_phantom_dmd
   map_intlk_phantom_dmd
   map_vsplit_f
   map_split_f
   map_intlk_f
   map_full_f
   map_stagger
   map_stairst
   map_ladder
   map_but_o
   map_o_s2x4_3
   map_x_s2x4_3
   map_offset
   map_4x4v
   map_blocks
   map_trglbox
   map_1x8_1x6
   map_rig_1x6
   map_hv_2x4_2
   map_3x4_2x3
   map_4x6_2x4
   map_hv_qtg_2
   map_vv_qtg_2
   map_ov_hrg_1
   map_ov_gal_1
   map_3o_qtag_1
   map_tgl4_1
   map_tgl4_2
   map_2x6_2x3
   map_qtag_2x3
   map_2x3_rmvr
   map_dbloff1
   map_dbloff2
   map_dhrgl1
   map_dhrgl2
   map_trngl_box1
   map_trngl_box2
   map_inner_box
   map_lh_s2x3_3
   map_lh_c1phana
   map_lh_c1phanb
   map_lh_s2x3_2
   map_rh_c1phana
   map_rh_c1phanb
   map_rh_s2x3_3
   map_rh_s2x3_2
   map_lz12
   map_rz12
   map_dmd_1x1
   map_star_1x1
   map_qtag_f0
   map_qtag_f1
   map_qtag_f2
   map_diag2a
   map_diag2b
   map_diag23a
   map_diag23b
   map_diag23c
   map_diag23d
   map_f2x8_4x4
   map_w4x4_4x4
   map_f2x8_2x8
   map_w4x4_2x8
   maps_3diag
   maps_3diagwk
   map_init_table
   split_lists
*/

#include "sd.h"
extern map_thing map_dhrgl1;
extern map_thing map_dhrgl2;
extern id_bit_table id_bit_table_4x4_but[];


/* These are used in setup_coords. */

Private coordrec thing1x1 = {s1x1, 3,
   {  0},
   {  0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  0, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x2 = {s1x2, 3,
   { -2,   2},
   {  0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x3 = {s1x3, 3,
   { -4,   0,   4},
   {  0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x4 = {s1x4, 3,
   { -6,  -2,   6,   2},
   {  0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x6 = {s1x6, 3,
   {-10,  -6,  -2,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  5,  4,  3, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x8 = {s1x8, 3,
   {-14, -10,  -2,  -6,  14,  10,   2,   6},
   {  0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  3,  2,  6,  7,  5,  4,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x10 = {s1x10, 4,
   {-18, -14, -10,  -6,  -2,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  0,
       1,  2,  3,  4,  9,  8,  7,  6,
       5, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x12 = {s1x12, 4,
   {-22, -18, -14, -10,  -6,  -2,  22,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1,  0,  1,
       2,  3,  4,  5, 11, 10,  9,  8,
       7,  6, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x14 = {s1x14, 4,
   {-26, -22, -18, -14, -10,  -6,  -2,  26,  22,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1,  0,  1,  2,
       3,  4,  5,  6, 13, 12, 11, 10,
       9,  8,  7, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x16 = {s1x16, 4,
   {-30, -26, -22, -18, -14, -10,  -6,  -2,  30,  26,  22,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  0,  1,  2,  3,
       4,  5,  6,  7, 15, 14, 13, 12,
      11, 10,  9,  8, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingdmd = {sdmd, 3,
   { -5,   0,   4,   0},
   {  0,   2,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec nicethingdmd = {sdmd, 3,
   { -5,   0,   5,   0},
   {  0,   2,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing2x2 = {s2x2, 3,
   { -2,   2,   2,  -2},
   {  2,   2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, -1,  3,  2, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing2x3 = {s2x3, 3,
   { -4,   0,   4,   4,   0,  -4},
   {  2,   2,   2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1,  5,  4,  3, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing2x4 = {s2x4, 3,
   { -6,  -2,   2,   6,   6,   2,  -2,  -6},
   {  2,   2,   2,   2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3, -1, -1,
      -1, -1,  7,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing2x5 = {s2x5, 3,
   { -8,  -4,   0,   4,   8,   8,   4,   0,  -4,  -8},
   {  2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1,  9,  8,  7,  6,  5, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing2x6 = {s2x6, 3,
   {-10,  -6,  -2,   2,   6,  10,  10,   6,   2,  -2,  -6,  -10},
   {  2,   2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5, -1,
      -1, 11, 10,  9,  8,  7,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing2x8 = {s2x8, 3,
   {-14, -10,  -6,  -2,   2,   6,  10,  14,  14,  10,   6,   2,  -2,  -6, -10, -14},
   {  2,   2,   2,   2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  4,  5,  6,  7,
      15, 14, 13, 12, 11, 10,  9,  8,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing3x6 = {s3x6, 3,
   {-10,  -6,  -2,   2,   6,  10,  10,   6,   2,  10,   6,   2,  -2,  -6, -10, -10,  -6,  -2},
   {  4,   4,   4,   4,   4,   4,   0,   0,   0,  -4,  -4,  -4,  -4,  -4,  -4,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5, -1,
      -1, 15, 16, 17,  8,  7,  6, -1,
      -1, 14, 13, 12, 11, 10,  9, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing3x8 = {s3x8, 3,
   {-14, -10,  -6,  -2,   2,   6,  10,  14,  14,  10,   6,   2,  14,  10,   6,   2,  -2,  -6, -10, -14, -14, -10,  -6,  -2},
   {  4,   4,   4,   4,   4,   4,   4,   4,   0,   0,   0,   0,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  4,  5,  6,  7,
      20, 21, 22, 23, 11, 10,  9,  8,
      19, 18, 17, 16, 15, 14, 13, 12,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing3x4 = {s3x4, 3,
   { -6,  -2,   2,   6,   6,   2,   6,   2,  -2,  -6,  -6,  -2},
   {  4,   4,   4,   4,   0,   0,  -4,  -4,  -4,  -4,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3, -1, -1,
      -1, -1, 10, 11,  5,  4, -1, -1,
      -1, -1,  9,  8,  7,  6, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thinggal = {s_galaxy, 3,
   { -7,  -2,   0,   2,   7,   2,   0,  -2},
   {  0,   2,   7,   2,   0,  -2,  -7,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1,  0,  1,  3,  4, -1, -1,
      -1, -1, -1,  7,  5, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingthar = {s_thar, 3,
   { -9,  -5,   0,   0,   9,   5,   0,   0},
   {  0,   0,   9,   5,   0,   0,  -9,  -5}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1,  0,  1, -1, -1,  5,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingxwv = {s_crosswave, 3,
   { -9,  -5,   0,   0,   9,   5,   0,   0},
   {  0,   0,   6,   2,   0,   0,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1,  0,  1, -1,  3,  5,  4, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingrigger = {s_rigger, 3,
   { -2,   2,  10,   6,   2,  -2, -10,  -6},
   {  2,   2,   0,   0,  -2,  -2,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  6,  7,  0,  1,  3,  2, -1,
      -1, -1, -1,  5,  4, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingspindle = {s_spindle, 3,
   { -4,   0,   4,   8,   4,   0,  -4,  -8},
   {  2,   2,   2,   0,  -2,  -2,  -2,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  7,  0,  1,  2,  3, -1,
      -1, -1, -1,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbone = {s_bone, 3,
   {-10,  10,   6,   2,  10, -10,  -6,  -2},
   {  2,   2,   0,   0,  -2,  -2,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  6,  7,  3,  2,  1, -1,
      -1,  5, -1, -1, -1, -1,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingqtag = {s_qtag, 3,       /* "fudged" coordinates */
   { -4,   5,   6,   2,   4,  -5,  -6,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0, -1,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicethingqtag = {s_qtag, 3,   /* "true" coordinates */
   { -5,   5,   6,   2,   5,  -5,  -6,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingptpd = {s_ptpd, 3,       /* "fudged" coordinates */
   { -9,  -6,  -2,  -6,   9,   6,   2,   6},
   {  1,   1,   0,  -3,  -1,  -1,   0,   3}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  6,  7, -1, -1,
      -1, -1,  3, -1, -1,  5,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec nicethingptpd = {s_ptpd, 3,       /* "true" coordinates */
   {-10,  -6,  -2,  -6,  10,   6,   2,   6},
   {  0,   2,   0,  -2,   0,  -2,   0,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  6,  7,  4, -1,
      -1, -1,  3, -1, -1,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingbigdmd = {sbigdmd, 3,   /* used for both -- symmetric and safe for press/truck */
   { -9,  -5,   0,   0,   5,   9,   9,   5,   0,   0,  -5,  -9},
   {  2,   2,   6,   2,   2,   2,  -2,  -2,  -6,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1,  0,  1, -1,  3,  4,  5, -1,
      -1, 11, 10, -1,  9,  7,  6, -1,
      -1, -1, -1, -1,  8, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing1x3dmd = {s1x3dmd, 3,   /* used for both -- symmetric and safe for press/truck */
   {-13,  -9,  -5,   0,  13,   9,   5,   0},
   {  0,   0,   0,   2,   0,   0,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2, -1,  3,  6,  5,  4,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing3x1dmd = {s3x1dmd, 3,   /* used for both -- symmetric and safe for press/truck */
   {-10,  -6,  -2,   0,  10,   6,   2,   0},
   {  0,   0,   0,   5,   0,   0,   0,  -5}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1,  0,  1,  2,  6,  5,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing3dmd = {s3dmd, 3,   /* used for both -- symmetric and safe for press/truck */
   { -7,   0,   7,  10,   6,   2,   7,   0,  -7, -10,  -6,  -2},
   {  5,   5,   5,   0,   0,   0,  -5,  -5,  -5,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2, -1, -1,
      -1,  9, 10, 11,  5,  4,  3, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  8, -1,  7,  6, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing4dmd = {s4dmd, 3,   /* used for both -- symmetric and safe for press/truck */
   {-11,  -5,   5,  11,  14,  10,   6,   2,  11,   5,  -5, -11, -14, -10,  -6,  -2},
   {  5,   5,   5,   5,   0,   0,   0,   0,  -5,  -5,  -5,  -5,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1, -1, -1,  2,  3, -1,
      12, 13, 14, 15,  7,  6,  5,  4,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, 11, 10, -1, -1,  9,  8, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec nicethingshort6 = {s_short6, 3,   /* "true" coordinates -- there is no "fudged" version */
   { -2,   0,   2,   2,   0,  -2},
   {  2,   6,   2,  -2,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  1, -1, -1, -1,
      -1, -1, -1,  0,  2, -1, -1, -1,
      -1, -1, -1,  5,  3, -1, -1, -1,
      -1, -1, -1, -1,  4, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec nicethingphan = {s_c1phan, 3,   /* "true" coordinates for mirroring --
                                   actually, the truck distances are greatly exaggerated. */
   {-10,  -6,  -2,  -6,   6,  10,   6,   2,  10,   6,   2,   6,  -6, -10,  -6,  -2},
   {  6,  10,   6,   2,  10,   6,   2,   6,  -6, -10,  -6,  -2, -10,  -6,  -2,  -6}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  1, -1, -1,  4, -1, -1,
      -1,  0, -1,  2,  7, -1,  5, -1,
      -1, -1,  3, -1, -1,  6, -1, -1,
      -1, -1, 14, -1, -1, 11, -1, -1,
      -1, 13, -1, 15, 10, -1,  8, -1,
      -1, -1, 12, -1, -1,  9, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingphan = {s_c1phan, 3,   /* "fudged" coordinates -- only truck within each 1x2 is allowed */
   { -6,  -3,  -2,  -3,   4,  15,   4,  11,   7,   4,   3,   4,  -5, -14,  -5, -10},
   {  4,  13,   4,   9,   7,   4,   3,   4,  -5, -14,  -5, -10,  -6,  -7,  -2,  -7}, {
      -1, -1, -1,  1, -1, -1, -1, -1,
      -1, -1, -1,  3, -1, -1, -1, -1,
      -1, -1,  0,  2, -1,  4,  7,  5,
      -1, -1, -1, -1, -1,  6, -1, -1,
      -1, -1, 14, -1, -1, -1, -1, -1,
      13, 15, 12, -1, 10,  8, -1, -1,
      -1, -1, -1, -1, -1, 11, -1, -1,
      -1, -1, -1, -1, -1,  9, -1, -1}};

Private coordrec nicethingglass = {s_hrglass, 3,   /* "true" coordinates for mirroring --
                                   actually, the truck distances are greatly exaggerated. */
   { -5,   5,   5,   0,   5,  -5,  -5,   0},
   {  5,   5,   0,   2,  -5,  -5,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1,  1, -1, -1,
      -1, -1,  6, -1,  3,  2, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingglass = {s_hrglass, 3,   /* "fudged" coordinates -- only truck by center 2 is allowed */
   { -4,   5,   5,   0,   4,  -5,  -5,   0},
   {  5,   5,   0,   2,  -5,  -5,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0, -1,  1, -1, -1,
      -1, -1,  6, -1,  3,  2, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingdglass = {s_dhrglass, 3,   /* used for both -- symmetric and safe for press/truck */
   {-10,  10,   5,   0,  10, -10,  -5,   0},
   {  2,   2,   0,   2,  -2,  -2,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  6, -1,  3,  2,  1, -1,
      -1,  5, -1, -1,  7, -1,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing4x4 = {s4x4, 3,
   {  6,   6,   6,   2,   6,   2,  -2,   2,  -6,  -6,  -6,  -2,  -6,  -2,   2,  -2},
   {  6,   2,  -2,   2,  -6,  -6,  -6,  -2,  -6,  -2,   2,  -2,   6,   6,   6,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, 12, 13, 14,  0, -1, -1,
      -1, -1, 10, 15,  3,  1, -1, -1,
      -1, -1,  9, 11,  7,  2, -1, -1,
      -1, -1,  8,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing4x5 = {s4x5, 3,
   { -8,  -4,   0,   4,   8,   8,   4,   0,  -4,  -8,   8,   4,   0,  -4,  -8,  -8,  -4,   0,   4,   8},
   {  6,   6,   6,   6,   6,   2,   2,   2,   2,   2,  -6,  -6,  -6,  -6,  -6,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1,  9,  8,  7,  6,  5, -1,
      -1, -1, 15, 16, 17, 18, 19, -1,
      -1, -1, 14, 13, 12, 11, 10, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing4x6 = {s4x6, 3,
   {-10,  -6,  -2,   2,   6,  10,  10,   6,   2,  -2,  -6, -10,  10,   6,   2,  -2,  -6, -10, -10,  -6,  -2,   2,   6,  10},
   {  6,   6,   6,   6,   6,   6,   2,   2,   2,   2,   2,   2,  -6,  -6,  -6,  -6,  -6,  -6,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5, -1,
      -1, 11, 10,  9,  8,  7,  6, -1,
      -1, 18, 19, 20, 21, 22, 23, -1,
      -1, 17, 16, 15, 14, 13, 12, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thing3oqtg = {s3oqtg, 3,
   { -8,   0,   8,  12,   8,   4,   0,  12,   8,   4,   8,   0,  -8, -12,  -8,  -4,   0, -12,  -8,  -4},
   {  6,   6,   6,   2,   2,   2,   2,  -2,  -2,  -2,  -6,  -6,  -6,  -2,  -2,  -2,  -2,   2,   2,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1, -1,  2, -1,
      -1, 17, 18, 19,  6,  5,  4,  3,
      -1, 13, 14, 15, 16,  9,  8,  7,
      -1, -1, 12, -1, 11, -1, 10, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingbigh = {sbigh, 3,
   { -9,  -9,  -9,  -9,  -6,  -2,   9,   9,   9,   9,   6,   2},
   {  6,   2,  -2,  -6,   0,   0,  -6,  -2,   2,   6,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0, -1, -1, -1, -1,  9, -1,
      -1,  1,  4,  5, 11, 10,  8, -1,
      -1,  2, -1, -1, -1, -1,  7, -1,
      -1,  3, -1, -1, -1, -1,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingbigbone = {sbigbone, 3,
   {-14, -10,  -6,  -2,  10,  14,  14,  10,   6,   2, -10, -14},
   {  2,   2,   0,   0,   2,   2,  -2,  -2,   0,   0,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  9,  8,  4,  5,
      11, 10, -1, -1, -1, -1,  7,  6,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingbigx = {sbigx, 4,
   {-17, -13,  -9,  -5,   0,   0,  17,  13,   9,   5,   0,   0},
   {  0,   0,   0,   0,   6,   2,   0,   0,   0,   0,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  4, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  0,
       1,  2,  3, -1,  5,  9,  8,  7,
       6, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, 11, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, 10, -1, -1, -1}};

Private coordrec thingblob = {s_bigblob, 3,
   { -2,   2,   2,   6,   6,   2,  10,  10,   6,   6,   2,   2,   2,  -2,  -2,  -6,  -6,  -2, -10, -10,  -6,  -6,  -2,  -2},
   { 10,  10,   6,   6,   2,   2,   2,  -2,  -2,  -6,  -6,  -2, -10, -10,  -6,  -6,  -2,  -2,  -2,   2,   2,   6,   6,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, 21, 22,  2,  3, -1, -1,
      -1, 19, 20, 23,  5,  4,  6, -1,
      -1, 18, 16, 17, 11,  8,  7, -1,
      -1, -1, 15, 14, 10,  9, -1, -1,
      -1, -1, -1, 13, 12, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};



/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
Cstring getout_strings[] = {
   "Mainstream",
   "Plus",
   "A1",
   "A2",
   "C1",
   "C2",
   "C3A",
   "C3",
   "C3X",
   "C4A",
   "C4",
   "C4X",
   "all",
   ""};

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
Cstring filename_strings[] = {
   ".MS",
   ".Plus",
   ".A1",
   ".A2",
   ".C1",
   ".C2",
   ".C3A",
   ".C3",
   ".C3X",
   ".C4A",
   ".C4",
   ".C4X",
   ".all",
   ""};

/* This list tells what level calls will be accepted for the "pick level call"
   operation.  When doing a "pick level call, we don't actually require calls
   to be exactly on the indicated level, as long as it's plausibly close. */
/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
dance_level level_threshholds[] = {
   l_mainstream,
   l_plus,
   l_a1,
   l_a1,      /* If a2 is given, an a1 call is OK. */
   l_c1,
   l_c2,
   l_c3a,
   l_c3a,     /* If c3 is given, a c3a call is OK. */
   l_c3a,     /* If c3x is given, a c3a call is OK. */
   l_c3x,     /* If c4a is given, a c3x call is OK. */
   l_c3x,     /* If c4 is given, a c3x call is OK. */
   l_c3x,     /* If c4x is given, a c3x call is OK. */
   l_dontshow,
   l_nonexistent_concept};

/* This list tells what level calls will be put in the menu and hence made available.
   In some cases, we make calls available that are higher than the requested level.
   When we use such a call, a warning is printed. */

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
dance_level higher_acceptable_level[] = {
   l_mainstream,
   l_plus,
   l_a1,
   l_a2,
   l_c1,
   l_c2,
   l_c3a,
   l_c3x,     /* If c3 is given, we allow c3x. */
   l_c3x,
   l_c4a,
   l_c4x,     /* If c4 is given, we allow c4x. */
   l_c4x,
   l_dontshow,
   l_nonexistent_concept};

/* BEWARE!!  This list is keyed to the definition of "call_list_kind" in sd.h . */
Cstring menu_names[] = {
   "???",
   "???",
   "(any setup)",
   "grand wave",
   "left grand wave",
   "dpt",
   "cdpt",
   "column",
   "left column",
   "8 chain",
   "trade by",
   "facing lines",
   "lines-out",
   "waves",
   "left waves",
   "2-faced lines",
   "left 2-faced lines",
   "grand column",
   "diamond/qtag"};


#define WESTBIT(otherbits) { ID2_BEAU | otherbits, ID2_TRAILER | otherbits, ID2_BELLE | otherbits, ID2_LEAD | otherbits }
#define EASTBIT(otherbits) { ID2_BELLE | otherbits, ID2_LEAD | otherbits, ID2_BEAU | otherbits, ID2_TRAILER | otherbits }
#define NORTHBIT(otherbits) { ID2_LEAD | otherbits, ID2_BEAU | otherbits, ID2_TRAILER | otherbits, ID2_BELLE | otherbits }
#define SOUTHBIT(otherbits) { ID2_TRAILER | otherbits, ID2_BELLE | otherbits, ID2_LEAD | otherbits, ID2_BEAU | otherbits }
#define NOBIT(otherbits) { otherbits, otherbits, otherbits, otherbits }
#define NWBITS(otherbits) { ID2_LEAD   |ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BELLE | otherbits, ID2_LEAD   |ID2_BELLE | otherbits }
#define SWBITS(otherbits) { ID2_TRAILER|ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BELLE | otherbits, ID2_LEAD   |ID2_BELLE | otherbits, ID2_LEAD   |ID2_BEAU  | otherbits }
#define SEBITS(otherbits) { ID2_TRAILER|ID2_BELLE | otherbits, ID2_LEAD   |ID2_BELLE | otherbits, ID2_LEAD   |ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BEAU  | otherbits }
#define NEBITS(otherbits) { ID2_LEAD   |ID2_BELLE | otherbits, ID2_LEAD   |ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BELLE | otherbits }


Private id_bit_table id_bit_table_1x2[] = {
   WESTBIT(0),
   EASTBIT(0)};

Private id_bit_table id_bit_table_2x2[] = {
   NWBITS(0),
   NEBITS(0),
   SEBITS(0),
   SWBITS(0)};

Private id_bit_table id_bit_table_2x4[] = {
   NWBITS(ID2_END),
   NEBITS(ID2_CENTER|ID2_CTR4),
   NWBITS(ID2_CENTER|ID2_CTR4),
   NEBITS(ID2_END),
   SEBITS(ID2_END),
   SWBITS(ID2_CENTER|ID2_CTR4),
   SEBITS(ID2_CENTER|ID2_CTR4),
   SWBITS(ID2_END)};

Private id_bit_table id_bit_table_2x6[] = {
   NWBITS(ID2_OUTRPAIRS),
   NEBITS(ID2_OUTRPAIRS),
   NWBITS(ID2_CTR4),
   NEBITS(ID2_CTR4),
   NWBITS(ID2_OUTRPAIRS),
   NEBITS(ID2_OUTRPAIRS),
   SEBITS(ID2_OUTRPAIRS),
   SWBITS(ID2_OUTRPAIRS),
   SEBITS(ID2_CTR4),
   SWBITS(ID2_CTR4),
   SEBITS(ID2_OUTRPAIRS),
   SWBITS(ID2_OUTRPAIRS)};

Private id_bit_table id_bit_table_1x4[] = {
   WESTBIT(ID2_END),
   EASTBIT(ID2_CENTER),
   EASTBIT(ID2_END),
   WESTBIT(ID2_CENTER)};

Private id_bit_table id_bit_table_dmd[] = {
   NOBIT(ID2_END),
   NORTHBIT(ID2_CENTER),
   NOBIT(ID2_END),
   SOUTHBIT(ID2_CENTER)};

Private id_bit_table id_bit_table_bone6[] = {
   NORTHBIT(0),
   NORTHBIT(0),
   /* We mark triangle points as trailers, in case we need to identify them in a 2x2 box,
       as in "leads start wheel the ocean". */
   {ID2_BEAU|ID2_TRAILER,     ID2_TRAILER,             ID2_BELLE|ID2_TRAILER,  ID2_LEAD},
   SOUTHBIT(0),
   SOUTHBIT(0),
   {ID2_BELLE|ID2_TRAILER,    ID2_LEAD,                ID2_BEAU|ID2_TRAILER,   ID2_TRAILER}};

Private id_bit_table id_bit_table_1x8[] = {
   WESTBIT(ID2_OUTR6| ID2_OUTR2| ID2_NCTR1X6| ID2_NCTR1X4| ID2_OUTRPAIRS),
   EASTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_NCTR1X4| ID2_OUTRPAIRS),
   EASTBIT(ID2_CTR2 | ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4),
   WESTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4),
   EASTBIT(ID2_OUTR6| ID2_OUTR2| ID2_NCTR1X6| ID2_NCTR1X4| ID2_OUTRPAIRS),
   WESTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_NCTR1X4| ID2_OUTRPAIRS),
   WESTBIT(ID2_CTR2 | ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4),
   EASTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4)};

Private id_bit_table id_bit_table_qtag[] = {
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR2 | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_CTR6  | ID2_CTR2),
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR2 | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_CTR6  | ID2_CTR2)};

Private id_bit_table id_bit_table_ptpd[] = {
   {ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2,           ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2},
   {ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU,   ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE},
   {ID2_CTR2|ID2_CTR6|ID2_BEAU,     ID2_CTR2|ID2_CTR6|ID2_TRAILER, ID2_CTR2|ID2_CTR6|ID2_BELLE,    ID2_CTR2|ID2_CTR6|ID2_LEAD},
   {ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE,  ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU},
   {ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2,           ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2},
   {ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE,  ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU},
   {ID2_CTR2|ID2_CTR6|ID2_BELLE,    ID2_CTR2|ID2_CTR6|ID2_LEAD,    ID2_CTR2|ID2_CTR6|ID2_BEAU,     ID2_CTR2|ID2_CTR6|ID2_TRAILER},
   {ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU,   ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE}};

Private id_bit_table id_bit_table_crosswave[] = {
   WESTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_NCTRDMD| ID2_NCTR1X4),
   EASTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_CTRDMD | ID2_NCTR1X4),
   NORTHBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR1X4),
   SOUTHBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2 |ID2_CTRDMD | ID2_CTR1X4),
   EASTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_NCTRDMD| ID2_NCTR1X4),
   WESTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_CTRDMD | ID2_NCTR1X4),
   SOUTHBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR1X4),
   NORTHBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2 |ID2_CTRDMD | ID2_CTR1X4)};

Private id_bit_table id_bit_table_gal[] = {
   NOBIT(ID2_END),
   NWBITS(ID2_CENTER|ID2_CTR4),
   NOBIT(ID2_END),
   NEBITS(ID2_CENTER|ID2_CTR4),
   NOBIT(ID2_END),
   SEBITS(ID2_CENTER|ID2_CTR4),
   NOBIT(ID2_END),
   SWBITS(ID2_CENTER|ID2_CTR4)};

Private id_bit_table id_bit_table_hrglass[] = {
   NOBIT(ID2_END   |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_END   |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_CENTER|ID2_OUTR6|ID2_CTRDMD | ID2_OUTR2),
   NOBIT(ID2_CENTER|ID2_CTR2 |ID2_CTRDMD | ID2_CTR6),
   NOBIT(ID2_END   |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_END   |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_CENTER|ID2_OUTR6|ID2_CTRDMD | ID2_OUTR2),
   NOBIT(ID2_CENTER|ID2_CTR2 |ID2_CTRDMD | ID2_CTR6)};

Private id_bit_table id_bit_table_dhrglass[] = {
   NORTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6),
   NORTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6),
   NOBIT(   ID2_CENTER|ID2_CTRDMD | ID2_OUTR6),
   NORTHBIT(ID2_CENTER|ID2_CTRDMD | ID2_CTR2),
   SOUTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6),
   SOUTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6),
   NOBIT(   ID2_CENTER|ID2_CTRDMD | ID2_OUTR6),
   SOUTHBIT(ID2_CENTER|ID2_CTRDMD | ID2_CTR2)};

Private id_bit_table id_bit_table_bigdmd[] = {  
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6)};

id_bit_table id_bit_table_bigdmd_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6)};

Private id_bit_table id_bit_table_bigbone[] = {  
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6)};

id_bit_table id_bit_table_bigbone_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6)};

Private id_bit_table id_bit_table_bighrgl[] = {  
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6)};

id_bit_table id_bit_table_bighrgl_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6)};

Private id_bit_table id_bit_table_bigdhrgl[] = {  
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6)};

id_bit_table id_bit_table_bigdhrgl_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6)};

/* This is the "standard" bit table for a 3x4.  It is used when the population
   is offset lines.  It recognizes "outer pairs". */
Private id_bit_table id_bit_table_3x4[] = {
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_CTR4 | ID2_CTR1X4)};

/* This is the "H" bit table for a 3x4.  It is used when the population
   is an "H".  It recognizes center 2/outer 6, and a center 1x4.
   Note that this table is external. */
id_bit_table id_bit_table_3x4_h[] = {
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_CTR1X4),
   NOBIT(ID2_CTR2  | ID2_CTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_CTR1X4),
   NOBIT(ID2_CTR2  | ID2_CTR1X4)};

id_bit_table id_bit_table_4x4_but[] = {
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4),
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4),
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4),
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4)};

Private id_bit_table id_bit_table_spindle[] = {
   NORTHBIT(ID2_CTR6 |ID2_OUTR6),
   NORTHBIT(ID2_CTR6 |ID2_CTR2),
   NORTHBIT(ID2_CTR6 |ID2_OUTR6),
   NOBIT   (ID2_OUTR2|ID2_OUTR6),
   SOUTHBIT(ID2_CTR6 |ID2_OUTR6),
   SOUTHBIT(ID2_CTR6 |ID2_CTR2),
   SOUTHBIT(ID2_CTR6 |ID2_OUTR6),
   NOBIT   (ID2_OUTR2|ID2_OUTR6)};

Private id_bit_table id_bit_table_rigger[] = {
   NWBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   NEBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   EASTBIT(ID2_OUTR2|ID2_END   |ID2_OUTRPAIRS),
   WESTBIT(ID2_CTR6 |ID2_END   |ID2_OUTRPAIRS),
   SEBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   SWBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   WESTBIT(ID2_OUTR2|ID2_END   |ID2_OUTRPAIRS),
   EASTBIT(ID2_CTR6 |ID2_END   |ID2_OUTRPAIRS)};

Private id_bit_table id_bit_table_1x3dmd[] = {
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_OUTR2| ID2_NCTRDMD),
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_CTR6|  ID2_NCTRDMD),
   NOBIT(   ID2_CENTER| ID2_CTR4|      ID2_OUTR6| ID2_CTR6|  ID2_CTRDMD),
   NORTHBIT(ID2_CENTER| ID2_CTR4|      ID2_CTR2|  ID2_CTR6|  ID2_CTRDMD),
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_OUTR2| ID2_NCTRDMD),
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_CTR6|  ID2_NCTRDMD),
   NOBIT(   ID2_CENTER| ID2_CTR4|      ID2_OUTR6| ID2_CTR6|  ID2_CTRDMD),
   SOUTHBIT(ID2_CENTER| ID2_CTR4|      ID2_CTR2|  ID2_CTR6|  ID2_CTRDMD)};

Private id_bit_table id_bit_table_3x1dmd[] = {
   WESTBIT(ID2_OUTR2| ID2_OUTR6| ID2_OUTRPAIRS| ID2_END|    ID2_CTR1X6|  ID2_NCTRDMD | ID2_NCTR1X4),
   EASTBIT(ID2_CTR6|  ID2_OUTR6| ID2_OUTRPAIRS| ID2_END|    ID2_CTR1X6|  ID2_NCTRDMD | ID2_CTR1X4),
   WESTBIT(ID2_CTR6|  ID2_CTR2|  ID2_CTR4|      ID2_CENTER| ID2_CTR1X6|  ID2_CTRDMD  | ID2_CTR1X4),
   NOBIT(  ID2_CTR6|  ID2_OUTR6| ID2_CTR4|      ID2_CENTER| ID2_NCTR1X6| ID2_CTRDMD  | ID2_NCTR1X4),
   EASTBIT(ID2_OUTR2| ID2_OUTR6| ID2_OUTRPAIRS| ID2_END|    ID2_CTR1X6|  ID2_NCTRDMD | ID2_NCTR1X4),
   WESTBIT(ID2_CTR6|  ID2_OUTR6| ID2_OUTRPAIRS| ID2_END|    ID2_CTR1X6|  ID2_NCTRDMD | ID2_CTR1X4),
   EASTBIT(ID2_CTR6|  ID2_CTR2|  ID2_CTR4|      ID2_CENTER| ID2_CTR1X6|  ID2_CTRDMD  | ID2_CTR1X4),
   NOBIT(  ID2_CTR6|  ID2_OUTR6| ID2_CTR4|      ID2_CENTER| ID2_NCTR1X6| ID2_CTRDMD  | ID2_NCTR1X4)};

/* If center diamond is fully occupied and outer diamonds have only points, use this. */
Private id_bit_table id_bit_table_3dmd[] = {
   NOBIT(ID2_OUTR6| ID2_NCTRDMD),
   NOBIT(ID2_OUTR6| ID2_CTRDMD),
   NOBIT(ID2_OUTR6| ID2_NCTRDMD),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(ID2_CTR2|  ID2_CTRDMD),
   NOBIT(ID2_OUTR6| ID2_NCTRDMD),
   NOBIT(ID2_OUTR6| ID2_CTRDMD),
   NOBIT(ID2_OUTR6| ID2_NCTRDMD),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(ID2_CTR2|  ID2_CTRDMD)};

/* If center 1x6 is fully occupied, use this.  It is external. */
id_bit_table id_bit_table_3dmd_ctr1x6[] = {
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4)};

/* If only center 1x4 is fully occupied, use this.  It is external. */
id_bit_table id_bit_table_3dmd_ctr1x4[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_CTR1X4)};

Private id_bit_table id_bit_table_bone[] = {
   NORTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   NORTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   EASTBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6),
   WESTBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2),
   SOUTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   SOUTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   WESTBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6),
   EASTBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2)};



/*                                                                                                           center_arity -------|
                                                                                                             mapelong --------|  |
                                                                                                            outer_rot -----|  |  |
                                                                                                     inner_rot ---------|  |  |  |
                                                               outlimit -----|                          bigsize ----|   |  |  |  |
                                          maps                   inlimit -|  |  bigsetup      insetup  outsetup     |   |  |  |  |   */
Private cm_thing map_3linel   =      {{0, 1, 3, 2, 9, 8, 6, 7,
                                                      4, 5, 10, 11},      4, 4, s1x12,          s1x4,     s1x4,     12, 0, 0, 0, 2};
Private cm_thing map_3lineh   =      {{3, 2, 0, 1, 6, 7, 9, 8,
                                                      4, 5, 10, 11},      4, 4, sbigh,          s1x4,     s1x4,     12, 1, 0, 0, 2};
Private cm_thing map_3linex   =      {{0, 1, 3, 2, 9, 8, 6, 7,
                                                      10, 11, 4, 5},      4, 4, sbigx,          s1x4,     s1x4,     12, 0, 1, 0, 2};
Private cm_thing map_3line    =      {{9, 8, 6, 7, 0, 1, 3, 2,
                                                      10, 11, 4, 5},      4, 4, s3x4,           s1x4,     s1x4,     12, 0, 0, 1, 2};



Private cm_thing map_3db      =      {{11, 0, 1, 10, 7, 4, 5, 6,
                                                      8, 9, 2, 3},        4, 4, sbighrgl,       s2x2,     sdmd,     12, 1, 1, 0, 2};
Private cm_thing map_3dbr     =      {{0, 1, 10, 11, 4, 5, 6, 7,
                                                      8, 9, 2, 3},        4, 4, sbighrgl,       s2x2,     sdmd,     12, 0, 1, 0, 2};

Private cm_thing map_3dbv     =      {{11, 0, 1, 10, 7, 4, 5, 6,
                                                      9, 2, 3, 8},        4, 4, sbigdhrgl,      s2x2,     sdmd,     12, 1, 0, 0, 2};
Private cm_thing map_3dbrv    =      {{0, 1, 10, 11, 4, 5, 6, 7,
                                                      9, 2, 3, 8},        4, 4, sbigdhrgl,      s2x2,     sdmd,     12, 0, 0, 0, 2};

Private cm_thing map_3lb      =      {{11, 0, 1, 10, 7, 4, 5, 6,
                                                      8, 9, 2, 3},        4, 4, sbigdmd,        s2x2,     s1x4,     12, 1, 1, 0, 2};
Private cm_thing map_3lbr     =      {{0, 1, 10, 11, 4, 5, 6, 7,
                                                      8, 9, 2, 3},        4, 4, sbigdmd,        s2x2,     s1x4,     12, 0, 1, 1, 2};



Private cm_thing map_3boxb    =      {{0, 1, 10, 11, 4, 5, 6, 7,
                                                      2, 3, 8, 9},        4, 4, sbigbone,       s2x2,     s1x4,     12, 0, 0, 0, 2};
Private cm_thing map_3boxbr   =      {{11, 0, 1, 10, 7, 4, 5, 6,
                                                      2, 3, 8, 9},        4, 4, sbigbone,       s2x2,     s1x4,     12, 1, 0, 0, 2};
Private cm_thing map_3dmdl0   =      {{-1, 0, -1, 1, -1, 5, -1, 4,
                                                      6, 7, 2, 3},        4, 4, s_crosswave,    sdmd,     s1x4,     12, 1, 1, 0, 2};
Private cm_thing map_3dmdl1   =      {{-1, 0, -1, 1, -1, 5, -1, 4,
                                                      3, 2, 7, 6},        4, 4, s1x8,           sdmd,     s1x4,     12, 1, 0, 0, 2};
Private cm_thing map_3dmdb0   =      {{-1, 6, -1, 7, -1, 3, -1, 2,
                                                      0, 1, 4, 5},        4, 4, s_rigger,       sdmd,     s2x2,     12, 1, 0, 0, 2};
Private cm_thing map_3dmd0    =      {{8, 9, 0, 10, 6, 4, 2, 3,
                                                      7, 11, 1, 5},       4, 4, s3dmd,          sdmd,     sdmd,     12, 1, 1, 0, 2};
Private cm_thing map_3ldl     =      {{9, 8, 6, 7, 0, 1, 3, 2,
                                                      10, -1, 4, -1},     4, 4, s3x4,           s1x4,     sdmd,     12, 0, 0, 1, 2};
Private cm_thing map_hrgl33v  =      {{5, 4, -1, -1, -1, -1, 1, 0,
                                                      6, 3, 2, 7},        4, 4, s_hrglass,      s2x2,     sdmd,     12, 0, 0, 1, 2};
Private cm_thing map_dhrgl33  =      {{-1, -1, 0, 5, 4, 1, -1, -1,
                                                      6, 3, 2, 7},        4, 4, s_dhrglass,     s2x2,     sdmd,     12, 1, 0, 0, 2};
Private cm_thing map_1x3dmd0  =      {{-1, 0, -1, 1, -1, 5, -1, 4,
                                                      2, 3, 6, 7},        4, 4, s1x3dmd,        sdmd,     sdmd,     12, 1, 0, 0, 2};
Private cm_thing map_4line    =      {{8, 6, 4, 5, 12, 13, 0, 14,
                                           10, 15, 3, 1, 2, 7, 11, 9},    4, 8, s4x4,           s1x4,     s2x4,     16, 0, 0, 1, 2};
Private cm_thing map_4linev   =       {{17, 18, 0, 11, 12, 23, 5, 6,
                                           10, 9, 8, 7, 22, 21, 20, 19},  4, 8, s4x6,           s1x4,     s2x4,     24, 1, 0, 0, 2};
Private cm_thing map_3box     =      {{0, 1, 10, 11, 4, 5, 6, 7,
                                                      2, 3, 8, 9},        4, 4, s2x6,           s2x2,     s2x2,     12, 0, 0, 0, 2};
Private cm_thing map_3boxv    =      {{11, 0, 1, 10, 7, 4, 5, 6,
                                                      9, 2, 3, 8},        4, 4, s2x6,           s2x2,     s2x2,     12, 1, 1, 1, 2};
Private cm_thing map_3boxr    =      {{0, 1, 3, 2, 9, 8, 6, 7,
                                                      4, 5, 10, 11},      4, 4, sbigrig,        s1x4,     s2x2,     12, 0, 0, 0, 2};
Private cm_thing map_3box4    =      {{8, 6, 4, 5, 12, 13, 0, 14,
                                                      11, 15, 3, 7},      4, 4, s4x4,           s1x4,     s2x2,     12, 0, 1, 1, 2};
Private cm_thing map_3boxrv   =      {{8, 6, 4, 5, 12, 13, 0, 14,
                                                      15, 3, 7, 11},      4, 4, s4x4,           s1x4,     s2x2,     12, 0, 0, 1, 2};
Private cm_thing map_3box4v   =      {{0, 1, 3, 2, 9, 8, 6, 7,
                                                      11, 4, 5, 10},      4, 4, sbigrig,        s1x4,     s2x2,     12, 0, 1, 0, 2};
Private cm_thing map_4box     =      {{0, 1, 14, 15, 6, 7, 8, 9,
                                           2, 3, 4, 5, 10, 11, 12, 13},   4, 8, s2x8,           s2x2,     s2x4,     16, 0, 0, 0, 2};
Private cm_thing map_4boxv    =      {{18, 11, 10, 19, 22, 7, 6, 23,
                                           15, 20, 9, 2, 3, 8, 21, 24},   4, 8, s4x6,           s2x2,     s2x4,     16, 1, 1, 0, 2};
Private cm_thing map_4dmd0    =      {{11, 12, 0, 13, 8, 5, 3, 4,
                                           1, 2, 6, 7, 9, 10, 14, 15},    4, 8, s4dmd,          sdmd,     s_qtag,   16, 1, 0, 0, 2};
Private cm_thing map_4dmd3    =      {{11, -1, 0, -1, 8, -1, 3, -1,
                                           1, 2, 6, 7, 9, 10, 14, 15},    4, 8, s4dmd,          s1x4,     s_qtag,   16, 1, 0, 0, 2};
Private cm_thing map_4dmdz    =      {{11, 12, 0, 13, 8, 5, 3, 4,
                                           10, -1, -1, 1, 2, -1, -1, 9},  4, 8, s4dmd,          sdmd,     s2x4,     16, 1, 1, 0, 2};
Private cm_thing map1x4_boxes =      {{6, 7, 2, 3,
                                             -1, 0, 1, -1, -1, 4, 5, -1}, 4, 8, s_bone,         s1x4,     s2x4,      8, 0, 0, 0, 1};
Private cm_thing map1x4_2x4 =        {{10, 11, 4, 5,
                                               0, 1, 2, 3, 6, 7, 8, 9},   4, 8, s3x4,           s1x4,     s2x4,     12, 0, 0, 0, 1};
Private cm_thing oddmap1x4_2x4 =     {{8, 9, 2, 3,
                                               0, 1, 4, 5, 6, 7, 10, 11}, 4, 8, sbigdmd,        s1x4,     s2x4,     12, 1, 0, 0, 1};
Private cm_thing map1x2_1x2 =        {{1, 3,          0, 2},              2, 2, s1x4,           s1x2,     s1x2,     4,  0, 0, 0, 1};
Private cm_thing oddmap1x2_1x2 =     {{3, 1,          0, 2},              2, 2, sdmd,           s1x2,     s1x2,     4,  1, 0, 0, 1};
/* The map "oddmapdmd_dmd", with its loss of elongation information for the outer diamond, is necessary to make the call
   "with confidence" work from the setup formed by having the centers partner tag in left-hand waves.  This means that certain
   Bakerisms involving concentric diamonds, in which each diamond must remember its own elongation, are not possible.  Too bad. */
Private cm_thing oddmapdmd_dmd =     {{1, 3, 5, 7,    6, 0, 2, 4},        4, 4, s_crosswave,    sdmd,     sdmd,     8,  0, 1, 9, 1};
Private cm_thing mapdmd_dmd =        {{1, 3, 5, 7,    0, 2, 4, 6},        4, 4, s_crosswave,    sdmd,     sdmd,     8,  0, 0, 9, 1};
Private cm_thing oddmapdmd_1x4 =     {{1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s3x1dmd,        s1x4,     sdmd,     8,  0, 1, 9, 1};
Private cm_thing mapdmd_1x4 =        {{1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s3x1dmd,        s1x4,     sdmd,     8,  0, 0, 9, 1};



Private cm_thing map2x3_1x4 =        {{8, 9, 3, 4,    0, 1, 2, 5, 6, 7},  4, 6, s_barredstar,   s1x4,     s2x3,    10,  0, 0, 1, 1};
Private cm_thing map2x4_1x6 = {{11, 12, 13, 4, 5, 6,  0, 1, 2, 3, 7, 8, 9, 10},  6, 8, s_barredstar12, s1x6,     s2x4,    14,  0, 0, 1, 1};
Private cm_thing map2x5_1x8 = {{14, 15, 17, 16, 5, 6, 8, 7,  0, 1, 2, 3, 4, 9, 10, 11, 12, 13},  8,10, s_barredstar16, s1x8,     s2x5,    18,  0, 0, 1, 1};



Private cm_thing oddmap_s_dmd_1x4 =  {{1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s_wingedstar,   s1x4,     sdmd,     8,  0, 1, 9, 1};
Private cm_thing map_s_dmd_1x4 =     {{1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s_wingedstar,   s1x4,     sdmd,     8,  0, 0, 9, 1};
Private cm_thing map_cs_1x4_dmd =    {{0, 3, 4, 7,    1, 2, 5, 6},        4, 4, s_wingedstar,   sdmd,     s1x4,     8,  0, 0, 9, 1};
Private cm_thing oddmap_s_star_1x4 = {{1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s_wingedstar,   s1x4,     s_star,   8,  0, 1, 9, 1};
Private cm_thing map_s_star_1x4 =    {{1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s_wingedstar,   s1x4,     s_star,   8,  0, 0, 9, 1};
Private cm_thing oddmap_s_short_1x6 = {{1, 2, 4, 7, 8, 10,
                                                11, 0, 3, 5, 6, 9},       6, 6, s_wingedstar12, s1x6,     s_short6, 12, 0, 1, 9, 1};
Private cm_thing map_s_spindle_1x8 = {{1, 2, 6, 5, 9, 10, 14, 13,
                                        3, 7, 12, 8, 11, 15, 4, 0},       8, 8, s_wingedstar16, s1x8,    s_spindle, 16, 0, 0, 9, 1};

/*                                                                                                           center_arity -------|
                                                                                                             mapelong --------|  |
                                                                                                            outer_rot -----|  |  |
                                                                                                     inner_rot ---------|  |  |  |
                                                               outlimit -----|                          bigsize ----|   |  |  |  |
                                          maps                   inlimit -|  |  bigsetup      insetup  outsetup     |   |  |  |  |   */
Private cm_thing map_spec_star12 =   {{2, 3, 4, 11, 10, 5, 8, 9,
                                                       0, 1, 6, 7},       4, 4, s_wingedstar12, s_star,   s1x4,     12, 0, 0, 0, 2};
Private cm_thing map_spec_star12v =  {{11, 2, 3, 4, 9, 10, 5, 8,
                                                       0, 1, 6, 7},       4, 4, s_wingedstar12, s_star,   s1x4,     12, 1, 0, 0, 2};
Private cm_thing map_spec_star16 =   {{2, 3, 5, 4, 6, 7, 14, 15, 13, 12, 10, 11,
                                                      0, 1, 8, 9},        4, 4, s_wingedstar16, s_star,   s1x4,     16, 0, 0, 0, 3};
Private cm_thing map_spec_star16v =  {{4, 2, 3, 5, 15, 6, 7, 14, 11, 13, 12, 10,
                                                      0, 1, 8, 9},        4, 4, s_wingedstar16, s_star,   s1x4,     16, 1, 0, 0, 3};

Private cm_thing map_spec_bar12 ={{12, 1, 13, 9, 6, 2, 5, 8,
                                                10, 11, 0, 3, 4, 7},      4, 6, s_barredstar12, s_star,   s2x3,    14,  0, 1, 1, 2};
Private cm_thing map_spec_bar16 ={{15, 1, 16, 12, 17, 2, 8, 11, 7, 3, 6, 10,
                                                13, 14, 0, 4, 5, 9},      4, 6, s_barredstar16, s_star,   s2x3,    18,  0, 1, 1, 3};





Private cm_thing mapdmd_2x2h =       {{1, 3, 5, 7,    0, 2, 4, 6},        4, 4, s_galaxy,       s2x2,     sdmd,     8,  0, 0, 9, 1};
Private cm_thing mapdmd_2x2v =       {{7, 1, 3, 5,    0, 2, 4, 6},        4, 4, s_galaxy,       s2x2,     sdmd,     8,  1, 0, 9, 1};
Private cm_thing map2x3_1x2 =        {{11, 5,    0, 1, 2, 6, 7, 8},       2, 6, s3dmd,          s1x2,     s2x3,     12, 0, 0, 9, 1};
Private cm_thing map1x4_1x4 =        {{3, 2, 7, 6,    0, 1, 4, 5},        4, 4, s1x8,           s1x4,     s1x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x4_1x4 =     {{6, 7, 2, 3,    0, 1, 4, 5},        4, 4, s_crosswave,    s1x4,     s1x4,     8,  1, 0, 0, 1};
Private cm_thing oddmap1x4_1x2 =     {{0, 1, 3, 4,    5, 2},              4, 2, s_2x1dmd,       s1x4,     s1x2,     6,  0, 1, 0, 1};

Private cm_thing map1x2_2x2 =        {{5, 2,    0, 1, 3, 4},              2, 4, s_bone6,        s1x2,     s2x2,     6,  0, 0, 0, 1};
Private cm_thing map1x2_2x2v =       {{4, 1,    5, 0, 2, 3},              2, 4, s2x3,           s1x2,     s2x2,     6,  1, 1, 1, 1};
Private cm_thing oddmap1x2_2x2 =     {{5, 2,    4, 0, 1, 3},              2, 4, s_bone6,        s1x2,     s2x2,     6,  0, 1, 1, 1};
Private cm_thing oddmap1x2_2x2v =    {{4, 1,    0, 2, 3, 5},              2, 4, s2x3,           s1x2,     s2x2,     6,  1, 0, 0, 1};

Private cm_thing map2x3_2x3 = {{8, 11, 1, 2, 5, 7,    9, 10, 0, 3, 4, 6}, 6, 6, s3x4,           s2x3,     s2x3,     12, 1, 1, 1, 1};
        cm_thing map2x4_2x4 = {{10, 15, 3, 1, 2, 7, 11, 9,    12, 13, 14, 0, 4, 5, 6, 8}, 8, 8, s4x4, s2x4, s2x4,   16, 0, 0, 1, 1};
        cm_thing map2x4_2x4v = {{6, 11, 15, 13, 14, 3, 7, 5,    8, 9, 10, 12, 0, 1, 2, 4}, 8, 8, s4x4, s2x4, s2x4,  16, 1, 1, 1, 1};
Private cm_thing map1x2_bone6 = {{1, 7, 6, 5, 3, 2,    0, 4},             6, 2, s_ptpd,         s_bone6,  s1x2,     8,  0, 0, 0, 1};
Private cm_thing map1x2_bone6_in = {{0, 1, 3, 4, 5, 7,    6, 2},          6, 2, s_qtag,         s_bone6,  s1x2,     8,  0, 0, 0, 1};
Private cm_thing map1x6_1x2 = {{2, 6,    0, 1, 3, 4, 5, 7},               2, 6, s1x8,           s1x2,     s1x6,     8,  0, 0, 0, 1};
Private cm_thing mapbone6_1x2 = {{7, 3,    0, 1, 2, 4, 5, 6},             2, 6, s_bone,         s1x2,     s_bone6,  8,  0, 0, 0, 1};
Private cm_thing map1x4_1x4_rc = {{0, 2, 4, 6,    1, 3, 5, 7},            4, 4, s1x8,           s1x4,     s1x4,     8,  0, 0, 0, 1};
Private cm_thing map1x4_dmd_rc = {{0, 3, 4, 7,    1, 2, 5, 6},            4, 4, s1x3dmd,        sdmd,     s1x4,     8,  0, 0, 0, 1};
Private cm_thing map1x2_bone6_rc = {{0, 1, 3, 4, 5, 7,    6, 2},          6, 2, s_bone,         s_bone6,  s1x2,     8,  0, 0, 0, 1};
Private cm_thing map1x2_2x3_rc =   {{0, 3, 1, 4, 7, 5,    6, 2},          6, 2, s_dhrglass,     s2x3,     s1x2,     8,  0, 0, 0, 1};
Private cm_thing map2x2_dmd_rc = {{7, 1, 3, 5,    0, 2, 4, 6},            4, 4, s_spindle,      sdmd,     s2x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap2x2_dmd_rc = {{7, 1, 3, 5,    6, 0, 2, 4},         4, 4, s_spindle,      sdmd,     s2x2,     8,  0, 1, 0, 1};
Private cm_thing map2x2_1x4_rc = {{0, 2, 4, 6,    1, 7, 5, 3},            4, 4, s_ptpd,         s1x4,     s2x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap2x2_1x4_rc = {{0, 2, 4, 6,    3, 1, 7, 5},         4, 4, s_ptpd,         s1x4,     s2x2,     8,  0, 1, 9, 1};
Private cm_thing map1x2_2x3 = {{0, 1, 2, 4, 5, 6,    7, 3},               6, 2, s_spindle,      s2x3,     s1x2,     8,  0, 0, 0, 1};
Private cm_thing map1x2_short6 = {{1, 2, 3, 5, 6, 7,    0, 4},            6, 2, s_galaxy,       s_short6, s1x2,     8,  0, 0, 0, 1};
Private cm_thing map1x2_intgl = {{0, 1, 3, 4, 5, 7,    6, 2},             6, 2, s_qtag,         s_bone6,  s1x2,     8,  0, 0, 0, 1};
Private cm_thing mapshort6_1x2h = {{5, 1,    6, 7, 0, 2, 3, 4},           2, 6, s_spindle,      s1x2,     s_short6, 8,  1, 1, 1, 1};
Private cm_thing mapshort6_1x2v = {{7, 3,    5, 6, 0, 1, 2, 4},           2, 6, s_hrglass,      s1x2,     s_short6, 8,  1, 1, 0, 1};
Private cm_thing mapbone6_1x2v =  {{7, 3,    0, 1, 2, 4, 5, 6},           2, 6, s_dhrglass,     s1x2,     s_bone6,  8,  1, 0, 0, 1};
Private cm_thing mapstar_2x2 = {{1, 3, 5, 7,    0, 2, 4, 6},              4, 4, s_galaxy,       s2x2,     s_star,   8,  0, 0, 0, 1};
Private cm_thing mapstar_star = {{1, 3, 5, 7,    0, 2, 4, 6},             4, 4, s_thar,         s_star,   s_star,   8,  0, 0, 0, 1};
Private cm_thing map1x2_1x6 = {{1, 3, 2, 5, 7, 6,    0, 4},               6, 2, s1x8,           s1x6,     s1x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x2_1x6 = {{0, 1, 2, 4, 5, 6,    7, 3},            6, 2, s3x1dmd,        s1x6,     s1x2,     8,  0, 1, 0, 1};

        cm_thing mapgnd1x2_1x2 = {{1, 3, 2, 6, 7, 5,    0, 4},            2, 2, s1x8,           s1x2,     s1x2,     8,  0, 0, 0, 3};
        cm_thing mapgnd1x2_1x2r = {{6, 0, 5, 1, 4, 2,    7, 3},           2, 2, s_spindle,      s1x2,     s1x2,     8,  1, 0, 0, 3};
Private cm_thing mapqtag_other = {{6, 7, 3, 2,    0, 1, 4, 5},            2, 4, s_qtag,         s1x2,     s2x2,     8,  0, 0, 1, 2};
        cm_thing map1x8_other  = {{1, 3, 2, 6, 7, 5,    0, 4},            2, 2, s1x8,           s1x2,     s1x2,     8,  0, 0, 0, 3};
        cm_thing mapspin_other = {{6, 0, 5, 1, 4, 2,    7, 3},            2, 2, s_spindle,      s1x2,     s1x2,     8,  1, 0, 0, 3};
Private cm_thing map2x4_other  = {{1, 2, 6, 5,    0, 3, 4, 7},            2, 4, s2x4,           s1x2,     s2x2,     8,  0, 0, 0, 2};
        cm_thing specialmapqtag_other = {{6, 7, 3, 2,      5, 0, 1, 4},   2, 4, s_qtag,         s1x2,     s2x2,     8,  0, 1, 0, 2};
        cm_thing specialmap2x4_other  = {{6, 5, 1, 2,      7, 0, 3, 4},   2, 4, s2x4,           s1x2,     s2x2,     8,  0, 1, 0, 2};

Private cm_thing map1x4_2x2 = {{0, 1, 4, 5,    6, 7, 2, 3},               4, 4, s_rigger,       s2x2,     s1x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x4_2x2 = {{5, 0, 1, 4,    6, 7, 2, 3},            4, 4, s_rigger,       s2x2,     s1x4,     8,  1, 0, 0, 1};
Private cm_thing map1x4_star = {{2, 3, 6, 7,    0, 1, 4, 5},              4, 4, s_wingedstar,   s_star,   s1x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x4_star = {{7, 2, 3, 6,    0, 1, 4, 5},           4, 4, s_wingedstar,   s_star,   s1x4,     8,  1, 0, 0, 1};

/*                                                                                                           center_arity -------|
                                                                                                             mapelong --------|  |
                                                                                                            outer_rot -----|  |  |
                                                                                                     inner_rot ---------|  |  |  |
                                                               outlimit -----|                          bigsize ----|   |  |  |  |
                                          maps                   inlimit -|  |  bigsetup      insetup  outsetup     |   |  |  |  |   */
Private cm_thing map2x3_star = {{9, 1, 4, 6,    7, 8, 0, 2, 3, 5},        4, 6, s_barredstar,   s_star,   s2x3,    10,  0, 1, 1, 1};
Private cm_thing oddmap2x3_star = {{6, 9, 1, 4,    7, 8, 0, 2, 3, 5},     4, 6, s_barredstar,   s_star,   s2x3,    10,  1, 1, 1, 1};
Private cm_thing map2x2_dmd = {{6, 3, 2, 7,    0, 1, 4, 5},               4, 4, s_hrglass,      sdmd,     s2x2,     8,  0, 0, 1, 1};
Private cm_thing map2x2_dmdv = {{6, 3, 2, 7,    0, 1, 4, 5},              4, 4, s_dhrglass,     sdmd,     s2x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap2x2_dmd = {{6, 3, 2, 7,    5, 0, 1, 4},            4, 4, s_hrglass,      sdmd,     s2x2,     8,  0, 1, 9, 1};
Private cm_thing oddmap2x2_dmdv = {{6, 3, 2, 7,    5, 0, 1, 4},           4, 4, s_dhrglass,     sdmd,     s2x2,     8,  0, 1, 9, 1};
Private cm_thing map2x2_1x4h = {{6, 7, 2, 3,    0, 1, 4, 5},              4, 4, s_qtag,         s1x4,     s2x2,     8,  0, 0, 1, 1};
Private cm_thing oddmap2x2_1x4h = {{6, 7, 2, 3,    5, 0, 1, 4},           4, 4, s_bone,         s1x4,     s2x2,     8,  0, 1, 9, 1};
Private cm_thing mapstar_1x4 =    {{1, 2, 5, 6,    0, 3, 4, 7},           4, 4, s3x1dmd,        s1x4,     s_star,   12, 0, 0, 9, 1};
Private cm_thing oddmapstar_1x4 = {{1, 2, 5, 6,    7, 0, 3, 4},           4, 4, s3x1dmd,        s1x4,     s_star,   12, 0, 1, 9, 1};
Private cm_thing mapstar_dmd ={{1, 3, 5, 7,    0, 2, 4, 6},               4, 4, s_crosswave,    sdmd,     s_star,   8,  0, 0, 9, 1};
Private cm_thing oddmapstar_dmd =  {{1, 3, 5, 7,    6, 0, 2, 4},          4, 4, s_crosswave,    sdmd,     s_star,   8,  0, 1, 9, 1};
Private cm_thing map2x2_1x4v = {{6, 7, 2, 3,    0, 1, 4, 5},              4, 4, s_bone,         s1x4,     s2x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap2x2_1x4v = {{6, 7, 2, 3,    5, 0, 1, 4},           4, 4, s_qtag,         s1x4,     s2x2,     8,  0, 1, 9, 1};



Private cm_thing map_o_1x4   = {{6, 7, 2, 3,    -1, 1, 4, -1, -1, -1, -1, -1,
                                          -1, 5, 0, -1, -1, -1, -1, -1},  4,16, s_bone,         s1x4,     s4x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap_o_1x4   = {{6, 7, 2, 3,    -1, -1, -1, -1, -1, 4, 5, -1,
                                          -1, -1, -1, -1, -1, 0, 1, -1},  4,16, s_qtag,         s1x4,     s4x4,     8,  0, 0, 0, 1};
Private cm_thing map_o_1x4v  = {{6, 7, 2, 3,    -1, -1, -1, -1, -1, 1, 4, -1,
                                          -1, -1, -1, -1, -1, 5, 0, -1},  4,16, s_bone,         s1x4,     s4x4,     8,  0, 1, 0, 1};
Private cm_thing oddmap_o_1x4v  = {{6, 7, 2, 3,    -1, 0, 1, -1, -1, -1, -1, -1,
                                          -1, 4, 5, -1, -1, -1, -1, -1},  4,16, s_qtag,         s1x4,     s4x4,     8,  0, 1, 0, 1};


Private cm_thing map_o_dmd   = {{6, 3, 2, 7,    -1, 1, 4, -1, -1, -1, -1, -1,
                                          -1, 5, 0, -1, -1, -1, -1, -1},  4,16, s_dhrglass,     sdmd,     s4x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap_o_dmd   = {{6, 3, 2, 7,    -1, -1, -1, -1, -1, 4, 5, -1,
                                          -1, -1, -1, -1, -1, 0, 1, -1},  4,16, s_hrglass,      sdmd,     s4x4,     8,  0, 0, 0, 1};
Private cm_thing map_o_dmdv  = {{6, 3, 2, 7,    -1, -1, -1, -1, -1, 1, 4, -1,
                                          -1, -1, -1, -1, -1, 5, 0, -1},  4,16, s_dhrglass,     sdmd,     s4x4,     8,  0, 1, 0, 1};
Private cm_thing oddmap_o_dmdv  = {{6, 3, 2, 7,    -1, 0, 1, -1, -1, -1, -1, -1,
                                          -1, 4, 5, -1, -1, -1, -1, -1},  4,16, s_hrglass,      sdmd,     s4x4,     8,  0, 1, 0, 1};

Private cm_thing map_o_2x2      = {{1, 2, 5, 6,    -1, 3, 4, -1, -1, -1, -1, -1,
                                          -1, 7, 0, -1, -1, -1, -1, -1},  4,16, s2x4,           s2x2,     s4x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap_o_2x2   = {{6, 1, 2, 5,    -1, -1, -1, -1, -1, 3, 4, -1,
                                          -1, -1, -1, -1, -1, 7, 0, -1},  4,16, s2x4,           s2x2,     s4x4,     8,  1, 1, 0, 1};

Private cm_thing map_4x4_2x2    = {{15, 3, 7, 11,    -1, 1, 2, -1, -1, 5, 6, -1,
                                          -1, 9, 10, -1, -1, 13, 14, -1}, 4,16, s4x4,           s2x2,     s4x4,     16, 0, 0, 0, 1};

Private cm_thing map2x2_12m  = {{1, 2, 9, 10, 3, 4, 7, 8,    0, 5, 6, 11},4, 4, s2x6,           s2x2,     s2x2,    12,  0, 0, 0, 2};
Private cm_thing map2x2_16m  = {{1, 2, 13, 14, 3, 4, 11, 12, 5, 6, 9, 10,    0, 7, 8, 15},4, 4, s2x8,s2x2,s2x2,    16,  0, 0, 0, 3};


Private cm_thing oddmap2x2_2x2h = {{1, 2, 5, 6,    7, 0, 3, 4},           4, 4, s2x4,           s2x2,     s2x2,     8,  0, 1, 9, 1};
Private cm_thing oddmap2x2_2x2v = {{6, 1, 2, 5,    0, 3, 4, 7},           4, 4, s2x4,           s2x2,     s2x2,     8,  1, 0, 9, 1};
Private cm_thing map2x2_2x2h = {{6, 1, 2, 5,    7, 0, 3, 4},              4, 4, s2x4,           s2x2,     s2x2,     8,  1, 1, 9, 1};
Private cm_thing map2x2_2x2v = {{1, 2, 5, 6,    0, 3, 4, 7},              4, 4, s2x4,           s2x2,     s2x2,     8,  0, 0, 0, 1};
Private cm_thing maplatgal = {{7, 0, 1, 3, 4, 5,    6, 2},                6, 2, s_galaxy,       s_short6, s1x2,     8,  1, 1, 0, 1};
Private cm_thing map1x4_dmd = {{2, 3, 6, 7,    0, 1, 4, 5},               4, 4, s1x3dmd,        sdmd,     s1x4,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x4_dmd = {{7, 2, 3, 6,    0, 1, 4, 5},            4, 4, s3x1dmd,        sdmd,     s1x4,     8,  1, 0, 0, 1};

Private cm_thing map1x2_1x4    = {{2, 5,    0, 1, 3, 4},                  2, 4, s1x6,           s1x2,     s1x4,     6,  0, 0, 0, 1};
Private cm_thing oddmap1x2_1x4 = {{5, 2,    0, 1, 3, 4},                  2, 4, s_1x2dmd,       s1x2,     s1x4,     6,  1, 0, 0, 1};

Private cm_thing oddmap1x6_1x2 = {{7, 3,    0, 1, 2, 4, 5, 6},            2, 6, s1x3dmd,        s1x2,     s1x6,     8,  1, 0, 0, 1};
Private cm_thing map1x2_2x1dmd = {{1, 2, 3, 5, 6, 7,    0, 4},            6, 2, s3x1dmd,        s_2x1dmd, s1x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x2_2x1dmd = {{6, 7, 1, 2, 3, 5,    0, 4},         6, 2, s_crosswave,    s_2x1dmd, s1x2,     8,  1, 0, 0, 1};
Private cm_thing map1x2_1x2dmd =    {{1, 2, 3, 5, 6, 7,    0, 4},         6, 2, s1x3dmd,        s_1x2dmd, s1x2,     8,  0, 0, 0, 1};
Private cm_thing oddmap1x2_1x2dmd = {{0, 1, 3, 4, 5, 7,    6, 2},         6, 2, s_crosswave,    s_1x2dmd, s1x2,     8,  0, 1, 0, 1};
Private cm_thing map2x1dmd_1x2 = {{2, 6,    0, 1, 3, 4, 5, 7},            2, 6, s3x1dmd,        s1x2,     s_2x1dmd, 8,  0, 0, 0, 1};
Private cm_thing oddmap2x1dmd_1x2 = {{7, 3,    0, 1, 2, 4, 5, 6},         2, 6, s_crosswave,    s1x2,     s_2x1dmd, 8,  1, 0, 0, 1};
Private cm_thing oddmap1x2_bone6 = {{5, 0, 3, 1, 4, 7,    6, 2},          6, 2, s_hrglass,      s_bone6,  s1x2,     8,  1, 0, 0, 1};

Private cm_thing map_intlk_hrglass = {{7, 0, 1, 3, 4, 5,    6, 2},        3, 2, s_hrglass,      s_trngl,  s1x2,     8,  0, 0, 0, 2};
Private cm_thing mapintlkvgal =      {{2, 5, 7, 6, 1, 3,    0, 4},        3, 2, s_galaxy,       s_trngl,  s1x2,     8,  2, 0, 0, 2};
Private cm_thing mapintlklgal =      {{0, 3, 5, 4, 7, 1,    6, 2},        3, 2, s_galaxy,       s_trngl,  s1x2,     8,  3, 1, 0, 2};

Private cm_thing oddmap1x2_2x3 = {{5, 7, 0, 1, 3, 4,    6, 2},            6, 2, s_qtag,         s2x3,     s1x2,     8,  1, 0, 0, 1};
Private cm_thing oddmap1x2_short6 = {{5, 7, 0, 1, 3, 4,    6, 2},         6, 2, s_rigger,       s_short6, s1x2,     8,  1, 0, 0, 1};
Private cm_thing oddmap2x3_1x2 = {{11, 5,    9, 10, 0, 3, 4, 6},          2, 6, s3x4,           s1x2,     s2x3,     12, 0, 1, 0, 1};
Private cm_thing oddmapshort6_1x2h = {{2, 6,    3, 0, 1, 7, 4, 5},        2, 6, s_ptpd,         s1x2,     s_short6, 8,  0, 1, 1, 1};
Private cm_thing oddmapshort6_1x2v = {{7, 3,    5, 6, 0, 1, 2, 4},        2, 6, s_qtag,         s1x2,     s_short6, 8,  0, 1, 0, 1};
Private cm_thing oddmapbigdmd_1x2v = {{9, 3,
                            0, 1, 2, -1, 4, 5, 6, 7, 8, -1, 10, 11},      2,12, sbigdmd,        s1x2,     sbigdmd, 12,  1, 0, 1, 1};
Private cm_thing mapbigdmd_1x2v = {{9, 3,
                            0, 1, 2, -1, 4, 5, 6, 7, 8, -1, 10, 11},      2,12, sbighrgl,       s1x2,     sbigdmd, 12,  0, 0, 1, 1};
Private cm_thing oddmap1x2_short6_rc = {{5, 6, 0, 1, 2, 4,    7, 3},      6, 2, s_rigger,       s_short6, s1x2,     8,  1, 0, 0, 1};
Private cm_thing map2x4_2x2 = {{2, 3, 8, 9,
                                          0, 1, 4, 5, 6, 7, 10, 11},      4, 8, s2x6,           s2x2,     s2x4,     12, 0, 0, 9, 1};
/*                                                                                                           center_arity -------|
                                                                                                             mapelong --------|  |
                                                                                                            outer_rot -----|  |  |
                                                                                                     inner_rot ---------|  |  |  |
                                                               outlimit -----|                          bigsize ----|   |  |  |  |
                                          maps                   inlimit -|  |  bigsetup      insetup  outsetup     |   |  |  |  |   */
Private cm_thing oddmap2x4_2x2 = {{9, 2, 3, 8,
                                          0, 1, 4, 5, 6, 7, 10, 11},      4, 8, s2x6,           s2x2,     s2x4,     12, 1, 0, 9, 1};
Private cm_thing map2x4_2x2v = {{15, 3, 7, 11,
                                          12, 13, 14, 0, 4, 5, 6, 8},     4, 8, s4x4,           s2x2,     s2x4,     16, 0, 0, 9, 1};
Private cm_thing oddmap2x4_2x2v = {{11, 15, 3, 7,
                                          12, 13, 14, 0, 4, 5, 6, 8},     4, 8, s4x4,           s2x2,     s2x4,     16, 1, 0, 9, 1};
Private cm_thing map1x8_2x2 = {{4, 5, 10, 11,
                                          0, 1, 3, 2, 6, 7, 9, 8},        4, 8, sbigrig,        s2x2,     s1x8,     12, 0, 0, 9, 1};
Private cm_thing oddmap1x8_2x2 = {{11, 4, 5, 10,
                                          0, 1, 3, 2, 6, 7, 9, 8},        4, 8, sbigrig,        s2x2,     s1x8,     12, 1, 0, 9, 1};
Private cm_thing map1x3dmd_line = {{1, 2, 5, 6,    0, 3, 4, 7},           4, 4, s1x3dmd,        s1x4,     sdmd,     8,  0, 0, 0, 1};
Private cm_thing mapdmd_line = {{1, 2, 5, 6,    0, 3, 4, 7},              4, 4, s3x1dmd,        s1x4,     sdmd,     8,  0, 0, 0, 1};
/* This one allows "finish" when the center "line" is actually a diamond whose centers are empty.
   This can happen because of the preference given for lines over diamonds at the conclusion of certain calls. */
Private cm_thing mappts_line = {{1, -1, 5, -1,    0, 2, 4, 6},            4, 4, s_crosswave,    s1x4,     sdmd,     8,  0, 0, 0, 1};
Private cm_thing map_s_dmd_line = {{1, 2, 5, 6,    0, 3, 4, 7},           4, 4, s_wingedstar,   s1x4,     sdmd,     8,  0, 0, 0, 1};




conc_initializer conc_init_table[] = {
/* outerk        innerk          conc_type     center_arity    maps */
   {s2x2,        s1x4,     schema_rev_checkpoint,    1, {&map2x2_1x4_rc,  &oddmap2x2_1x4_rc,   &map2x2_1x4_rc,   &oddmap2x2_1x4_rc}},
   {s2x2,        sdmd,     schema_rev_checkpoint,    1, {&map2x2_dmd_rc,  &oddmap2x2_dmd_rc,   &map2x2_dmd_rc,   &oddmap2x2_dmd_rc}},
   {s1x4,        s1x4,     schema_rev_checkpoint,    1, {&map1x4_1x4_rc,  0,                   &map1x4_1x4_rc,   0}},
   {s1x4,        sdmd,     schema_rev_checkpoint,    1, {&map1x4_dmd_rc,  0,                   &map1x4_dmd_rc,   0}},
   {s1x4,        sdmd,     schema_ckpt_star,         1, {&map_cs_1x4_dmd, 0,                   &map_cs_1x4_dmd,  0}},
   {sdmd,        s1x4,     schema_conc_star,         1, {&map_s_dmd_1x4,  &oddmap_s_dmd_1x4,   &map_s_dmd_1x4,   &oddmap_s_dmd_1x4}},
   {s_star,      s1x4,     schema_conc_star,         1, {&map_s_star_1x4, &oddmap_s_star_1x4,  &map_s_star_1x4,  &oddmap_s_star_1x4}},
   {s1x4,        s_star,   schema_conc_star,         1, {&map1x4_star,    &oddmap1x4_star,     &map1x4_star,     &oddmap1x4_star}},
   {s_short6,    s1x6,     schema_conc_star12,       1, {0,               &oddmap_s_short_1x6, 0,                0}},
   {s_spindle,   s1x8,     schema_conc_star16,       1, {0,               0,                   &map_s_spindle_1x8, 0}},
   {s1x4,        s_star,   schema_conc_star12,       2, {&map_spec_star12,&map_spec_star12v,   &map_spec_star12, &map_spec_star12v}},
   {s1x4,        s_star,   schema_conc_star16,       3, {&map_spec_star16,&map_spec_star16v,   &map_spec_star16, &map_spec_star16v}},

   {s2x3,        s1x4,     schema_concentric,        1, {0,               0,                   &map2x3_1x4,      0}},
   {s2x3,        s_star,   schema_concentric,        1, {0,               0,                   &oddmap2x3_star,  &map2x3_star}},

   {s2x4,        s1x6,     schema_conc_bar12,        1, {0,               0,                   &map2x4_1x6,      0}},
   {s2x5,        s1x8,     schema_conc_bar16,        1, {0,               0,                   &map2x5_1x8,      0}},
   {s2x3,        s_star,   schema_conc_bar12,        2, {0,               0,                   0,                &map_spec_bar12}},
   {s2x3,        s_star,   schema_conc_bar16,        3, {0,               0,                   0,                &map_spec_bar16}},
   {s1x2,      s1x2, schema_grand_single_concentric, 3, {&mapgnd1x2_1x2,  &mapgnd1x2_1x2r,     &mapgnd1x2_1x2,   &mapgnd1x2_1x2r}},
   {s1x2,        s1x2,     schema_concentric_others, 3, {&map1x8_other,   &mapspin_other,      &map1x8_other,    &mapspin_other}},
   {s2x2,        s1x2,     schema_concentric_others, 2, {0,               &specialmapqtag_other, 0,              &specialmap2x4_other}},
   {s1x4,        s1x4,     schema_in_out_triple,     2, {&map_3linel,     &map_3lineh,         &map_3line,       &map_3linex}},
   {sdmd,        sdmd,     schema_in_out_triple,     2, {0,               &map_1x3dmd0,        &map_3dmd0,       &map_1x3dmd0}},
   {sdmd,        s1x4,     schema_in_out_triple,     2, {0,               0,                   &map_3ldl,        0}},
   {s1x4,        sdmd,     schema_in_out_triple,     2, {0,               &map_3dmdl1,         &map_3dmdl0,      &map_3dmdl1}},
   {s2x2,        sdmd,     schema_in_out_triple,     2, {0,               &map_3dmdb0,         0,                &map_3dmdb0}},
   {s2x2,        s2x2,     schema_in_out_triple,     2, {&map_3box,       0,                   &map_3boxv,       0}},
   {s1x4,        s2x2,     schema_in_out_triple,     2, {&map_3boxb,      &map_3boxbr,         &map_3lb,         &map_3lbr}},
   {sdmd,        s2x2,     schema_in_out_triple,     2, {&map_3dbrv,      &map_3dbv,           &map_3db,         &map_3dbr}},
   {s2x2,        s1x4,     schema_in_out_triple,     2, {&map_3boxr,      &map_3box4,          &map_3boxrv,      &map_3box4v}},
   {s2x4,        s1x4,     schema_in_out_quad,       2, {0,               &map_4linev,         &map_4line,       0}},
   {s2x4,        s2x2,     schema_in_out_quad,       2, {&map_4box,       0,                   &map_4boxv,       0}},
   {s_qtag,      sdmd,     schema_in_out_quad,       2, {0,               &map_4dmd0,          0,                0}},
   {s2x4,        sdmd,     schema_in_out_quad,       2, {0,               0,                   &map_4dmdz,       0}},
   {s_qtag,      s1x4,     schema_in_out_quad,       2, {0,               &map_4dmd3,          0,                0}},
   {s1x2,        s_trngl,  schema_intlk_vertical_6,  2, {&map_intlk_hrglass, &mapintlkvgal,    0,                0}},
   {s1x2,        s_trngl,  schema_intlk_lateral_6,   2, {&mapintlklgal,   0,                   0,                0}},
   {s4x4,        s2x2,     schema_conc_o,            1, {&map_o_2x2,      0,                   &oddmap_o_2x2,    0}},
   {s4x4,        s1x4,     schema_conc_o,            1, {&map_o_1x4,      &oddmap_o_1x4v,      &oddmap_o_1x4,    &map_o_1x4v}},
   {s4x4,        sdmd,     schema_conc_o,            1, {&map_o_dmd,      &oddmap_o_dmdv,      &oddmap_o_dmd,    &map_o_dmdv}},
   {s1x6,        s1x2,     schema_concentric,        1, {&map1x6_1x2,     &oddmap1x6_1x2,      &map1x6_1x2,      &oddmap1x6_1x2}},
   {s_bone6,     s1x2,     schema_concentric,        1, {&mapbone6_1x2,   &mapbone6_1x2v,      &mapbone6_1x2,    &mapbone6_1x2v}},
   {s2x3,        s1x2,     schema_concentric,        1, {&map2x3_1x2,     &oddmap2x3_1x2,      &map2x3_1x2,      &oddmap2x3_1x2}},
   {s2x3,        s2x3,     schema_concentric,        1, {&map2x3_2x3,     0,                   &map2x3_2x3,      0}},
   {s2x4,        s1x4,     schema_concentric,        1, {&map1x4_boxes,   &oddmap1x4_2x4,      &map1x4_2x4,      0}},
   {s2x4,        s2x2,     schema_concentric,        1, {&map2x4_2x2,     &oddmap2x4_2x2,      &map2x4_2x2v,     &oddmap2x4_2x2v}},
   {s1x8,        s2x2,     schema_concentric,        1, {&map1x8_2x2,     &oddmap1x8_2x2,      &map1x8_2x2,      &oddmap1x8_2x2}},
   {s2x4,        s2x4,     schema_concentric,        1, {&map2x4_2x4,     0,                   &map2x4_2x4,      0}},
   {s1x4,        s1x4,     schema_concentric,        1, {&map1x4_1x4,     &oddmap1x4_1x4,      &map1x4_1x4,      &oddmap1x4_1x4}},
   {s1x2,        s1x4,     schema_concentric,        1, {0,               &oddmap1x4_1x2,      0,                &oddmap1x4_1x2}},
   {s2x2,        s1x2,     schema_concentric,        1, {&map1x2_2x2,     &oddmap1x2_2x2v,     &map1x2_2x2v,     &oddmap1x2_2x2}},

/* outerk        innerk          conc_type     center_arity    maps */
   {s1x4,        s1x2,     schema_concentric,        1, {&map1x2_1x4,     &oddmap1x2_1x4,      &map1x2_1x4,      &oddmap1x2_1x4}},
   {s1x4,        sdmd,     schema_concentric,        1, {&map1x4_dmd,     &oddmap1x4_dmd,      &map1x4_dmd,      &oddmap1x4_dmd}},
   {s1x4,        s_star,   schema_concentric,        1, {&map1x4_star,    &oddmap1x4_star,     &map1x4_star,     &oddmap1x4_star}},
   {s1x4,        s2x2,     schema_concentric,        1, {&map1x4_2x2,     &oddmap1x4_2x2,      &map1x4_2x2,      &oddmap1x4_2x2}},
   {s1x2,        s1x2,     schema_concentric,        1, {&map1x2_1x2,     &oddmap1x2_1x2,      &map1x2_1x2,      &oddmap1x2_1x2}},
   {s1x2,        s2x3,     schema_concentric,        1, {&map1x2_2x3,     &oddmap1x2_2x3,      &map1x2_2x3,      &oddmap1x2_2x3}},
   {s1x2,        s_bone6,  schema_concentric,        1, {&map1x2_bone6,   &oddmap1x2_bone6,    &map1x2_bone6,    &oddmap1x2_bone6}},
   {s1x2,        s_short6, schema_concentric,        1, {&map1x2_short6,  &oddmap1x2_short6,   &map1x2_short6,   &oddmap1x2_short6}},
   {s1x2,        s_bone6,  schema_concentric_6_2_tgl,1, {&map1x2_bone6_in,0,                   &map1x2_bone6_in, 0}},
   {s1x2,        s1x6,     schema_concentric,        1, {&map1x2_1x6,     &oddmap1x2_1x6,      &map1x2_1x6,      &oddmap1x2_1x6}},
   {s_short6,    s1x2,     schema_concentric,        1, {&mapshort6_1x2v, &oddmapshort6_1x2v,  &mapshort6_1x2h,  &oddmapshort6_1x2h}},
   {sbigdmd,     s1x2,     schema_concentric,        1, {&mapbigdmd_1x2v, &oddmapbigdmd_1x2v,  0,                0}},
   {s1x2,        s_2x1dmd, schema_concentric,        1, {&map1x2_2x1dmd,  &oddmap1x2_2x1dmd,   &map1x2_2x1dmd,   &oddmap1x2_2x1dmd}},
   {s1x2,        s_1x2dmd, schema_concentric,        1, {&map1x2_1x2dmd,  &oddmap1x2_1x2dmd,   &map1x2_1x2dmd,   &oddmap1x2_1x2dmd}},
   {s_2x1dmd,    s1x2,     schema_concentric,        1, {&map2x1dmd_1x2,  &oddmap2x1dmd_1x2,   &map2x1dmd_1x2,   &oddmap2x1dmd_1x2}},
   {s2x2,        s2x2,     schema_concentric,        1, {&map2x2_2x2v,    &oddmap2x2_2x2v,     &map2x2_2x2h,     &oddmap2x2_2x2h}},
   {s2x2,        s1x4,     schema_concentric,        1, {&map2x2_1x4v,    &oddmap2x2_1x4v,     &map2x2_1x4h,     &oddmap2x2_1x4h}},
   {s2x2,        sdmd,     schema_concentric,        1, {&map2x2_dmdv,    &oddmap2x2_dmd,      &map2x2_dmd,      &oddmap2x2_dmdv}},
   {sdmd,        sdmd,     schema_concentric,        1, {&mapdmd_dmd,     &oddmapdmd_dmd,      &mapdmd_dmd,      &oddmapdmd_dmd}},
   {sdmd,        s1x4,     schema_concentric,        1, {&mapdmd_1x4,     &oddmapdmd_1x4,      &mapdmd_1x4,      &oddmapdmd_1x4}},
   {sdmd,        s2x2,     schema_concentric,        1, {&mapdmd_2x2h,    &mapdmd_2x2v,        &mapdmd_2x2h,     &mapdmd_2x2v}},
   {s_star,      sdmd,     schema_concentric,        1, {&mapstar_dmd,    &oddmapstar_dmd,     &mapstar_dmd,     &oddmapstar_dmd}},
   {s_star,      s1x4,     schema_concentric,        1, {&mapstar_1x4,    &oddmapstar_1x4,     &mapstar_1x4,     &oddmapstar_1x4}},
   {s_star,      s_star,   schema_concentric,        1, {&mapstar_star,   0,                   &mapstar_star,    0}},
   {s_star,      s2x2,     schema_concentric,        1, {&mapstar_2x2,    0,                   &mapstar_2x2,     0}},
   {nothing}};


/* BEWARE!!  These lists are keyed to the enumeration "analyzer_kind" :

    analyzer_NORMAL
    |    analyzer_CHECKPT
    |    |    analyzer_2X6
    |    |    |    analyzer_6X2
    |    |    |    |    analyzer_4X2
    |    |    |    |    |    analyzer_6X2_TGL
    |    |    |    |    |    |    analyzer_O
    |    |    |    |    |    |    |    analyzer_BAR
    |    |    |    |    |    |    |    |    analyzer_BAR12
    |    |    |    |    |    |    |    |    |    analyzer_BAR16
    |    |    |    |    |    |    |    |    |    |    analyzer_STAR12
    |    |    |    |    |    |    |    |    |    |    |    analyzer_STAR16
    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_SINGLE
    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_GRANDSINGLE
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_TRIPLE_LINE
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_QUAD_LINE
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_VERTICAL6
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_LATERAL6
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_INTLK_VERTICAL6
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_INTLK_LATERAL6
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_OTHERS
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_CONC_DIAMONDS
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_DIAMOND_LINE
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    analyzer_CTR_DMD
    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    | */

Private cm_hunk concthing_1x4 = {0x5, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map1x2_1x2,
                                                                     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_dmd = {0x5, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &oddmap1x2_1x2,
                                                                     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_2x1dmd = {0, 0, 0, 0,
   {0,   0,   0,   0,   &oddmap1x4_1x2,
                             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_2x3 = {0, 0, 0, 0,
   {0,   0,   0,   0,   &oddmap1x2_2x2v,
                             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bone6 = {0, 0, 0, 0,
   {0,   0,   0,   0,   &map1x2_2x2,
                             0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_qtag = {0x33, 0xDD, 0x11, 0,
   {&map2x2_1x4h,
         0,   &oddmapshort6_1x2v,
                   &oddmap1x2_2x3,
                        0,   &map1x2_intgl,
                                  &oddmap_o_1x4,
                                       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &mapqtag_other,
                                                                                                             0,   0,   0}};

Private cm_hunk concthing_bone = {0x33, 0, 0x11, 0,
   {&map2x2_1x4v,
         &map1x2_bone6_rc,
              &mapbone6_1x2,
                   0,   0,   0,   &map_o_1x4,
                                       0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_rigger = {0xCC, 0xDD, 0, 0,
   {&map1x4_2x2,
         &oddmap1x2_short6_rc,
              0,   &oddmap1x2_short6,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_spindle = {0, 0xEE, 0x44, 0,
   {0,   &map2x2_dmd_rc,
              &mapshort6_1x2h,
                   &map1x2_2x3,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   &mapgnd1x2_1x2r,
                                                                          0,   0,   0,   0,   0,   0,   &mapspin_other,
                                                                                                             0,   0,   0}};

Private cm_hunk concthing_hrglass = {0x33, 0xDD, 0x11, 0,
   {&map2x2_dmd,
         0,   &mapshort6_1x2v,
                   &oddmap1x2_bone6,
                        0,   0,   &oddmap_o_dmd,
                                       0,   0,   0,   0,   0,   0,   0,   &map_hrgl33v,
                                                                               0,   &oddmap1x2_bone6,
                                                                                         0,   &map_intlk_hrglass,
                                                                                                   0,   0,   0,   0,   0}};

Private cm_hunk concthing_dhrglass = {0x33, 0, 0x11, 0,
   {&map2x2_dmdv,
         &map1x2_2x3_rc,
              &mapbone6_1x2v,
                   0,   0,   0,   &map_o_dmd,
                                       0,   0,   0,   0,   0,   0,   0,   &map_dhrgl33,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_xwave = {0x33, 0x77, 0x11, 0x55,
   {&oddmap1x4_1x4,
         0,   &oddmap2x1dmd_1x2,
                   &oddmap1x2_2x1dmd,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &mapdmd_dmd,
                                                                                                                  &mappts_line,
                                                                                                                       0}};

Private cm_hunk concthing_1x8 = {0x33, 0x77, 0x22, 0,
   {&map1x4_1x4,
         &map1x4_1x4_rc,
              &map1x6_1x2,
                   &map1x2_1x6,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   &mapgnd1x2_1x2,    
                                                                          0,   0,   0,   0,   0,   0,   &map1x8_other,
                                                                                                             0,   0,   0}};

Private cm_hunk concthing_2x4 = {0x66, 0, 0, 0,
   {&map2x2_2x2v,
         0,   0,   0,   0,   0,   &map_o_2x2,
                                       &map2x2_2x2v,
                                            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map2x4_other,
                                                                                                             0,   0,   0}};

Private cm_hunk concthing_3x4 = {0, 0, 0x041, 0,
   {0,   0,   &oddmap2x3_1x2,
                   0,   0,   0,   0,   0,   &map2x3_2x3,
                                                 0,   &map2x3_2x3,
                                                           0,   0,   0,   &map_3line,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_3dmd = {0, 0, 0x041, 0,    /* This one may only be used when the outer two diamonds have no centers,
                                                         that is, it is a 1x2 between 1x3's.  There is code to check for this. */
   {0,   0,   &map2x3_1x2,
                   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3dmd0,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_4dmd = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_4dmd0,
                                                                                    0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_1x12 = {0, 0, 0x041, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3linel,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bigx = {0, 0, 0x041, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3linex,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bigh = {0, 0, 0x041, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3lineh,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bigbone = {0, 0, 0x104, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3boxb,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bigrig = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3boxr,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bigdmd = {0, 0, 0x104, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3lb,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bighrgl = {0, 0, 0x104, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3dbr,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_bigdhrgl = {0, 0, 0x104, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_3dbrv,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_2x6 = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   &map2x2_12m,
                                                 0,   0,   0,   0,   0,   &map_3box,
                                                                               0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_2x8 = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   &map2x2_16m,
                                                      0,   0,   0,   0,   0,   &map_4box,     
                                                                                    0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_4x4 = {0, 0, 0, 0,

   {0,   0,   0,   0,   0,   0,   &map_4x4_2x2,
                                       0,   0,   &map2x4_2x4v,
                                                      0,   &map2x4_2x4v,
                                                                0,   0,   &map_3boxrv,
                                                                               &map_4line,
                                                                                    0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_ptpd = {0, 0x77, 0x22, 0,
   {0,   &map2x2_1x4_rc,
              &oddmapshort6_1x2h,
                   &map1x2_bone6,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};

Private cm_hunk concthing_1x3dmd = {0x33, 0x77, 0x11, 0,
   {&map1x4_dmd,
         &map1x4_dmd_rc,
              &oddmap1x6_1x2,
                   &map1x2_1x2dmd,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_1x3dmd0,
                                                                               0,   0,   0,   0,   0,   0,   0,   &map1x3dmd_line,
                                                                                                                       0}};

Private cm_hunk concthing_3x1dmd = {0x33, 0xEE, 0, 0,
   {&oddmap1x4_dmd,
         0,   &map2x1dmd_1x2,
                   &oddmap1x2_1x6,
                        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &mapdmd_line,
                                                                                                                       0}};

Private cm_hunk concthing_wstar = {0x33, 0, 0, 0,
   {&map1x4_star,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_s_dmd_line,
                                                                                                                       0}};

Private cm_hunk concthing_wstar12 = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_spec_star12,
                                                           0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &oddmap_s_short_1x6,
                                                                                                                       0}};

Private cm_hunk concthing_wstar16 = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_spec_star16,
                                                                0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_s_spindle_1x8,
                                                                                                                       0}};

Private cm_hunk concthing_bstar = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   &map2x3_star,
                                            0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map2x3_1x4,
                                                                                                                       0}};
Private cm_hunk concthing_bstar12 = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   &map_spec_bar12,
                                                 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map2x4_1x6,
                                                                                                                       0}};
Private cm_hunk concthing_bstar16 = {0, 0, 0, 0,
   {0,   0,   0,   0,   0,   0,   0,   0,   0,   &map_spec_bar16,
                                                      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map2x5_1x8,
                                                                                                                       0}};


Private cm_hunk concthing_gal = {0x55, 0, 0, 0,
   {&mapstar_2x2,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   &map1x2_short6,
                                                                                         &maplatgal,
                                                                                              &mapintlkvgal,
                                                                                                   &mapintlklgal,
                                                                                                        0,   0,   0,   0}};

Private cm_hunk concthing_thar = {0x55, 0, 0, 0,
   {&mapstar_star,
         0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}};




/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
setup_attr setup_attrs[] = {
   /* nothing */
      {-1,                          /* setup_limits */
      (coordrec *) 0,               /* setup_coords */
      (coordrec *) 0,               /* nice_setup_coords */
      (cm_hunk *) 0,                /* conctab */
      {b_nothing,   b_nothing},     /* keytab */
      { 0, 0},                      /* bounding_box */
      FALSE,                        /* four_way_symmetry */
      (id_bit_table *) 0,           /* id_bit_table_ptr */
      {  (Cstring) 0,               /* print_strings */
         (Cstring) 0}},
   /* s1x1 */
      { 0,
      &thing1x1,
      &thing1x1,
      (cm_hunk *) 0,
      {b_1x1,       b_1x1},
      { 1, 1},
      TRUE,
      (id_bit_table *) 0,
      {  "a@",
         (Cstring) 0}},
   /* s1x2 */
      { 1,
      &thing1x2,
      &thing1x2,
      (cm_hunk *) 0,
      {b_1x2,       b_2x1},
      { 2, 1},
      FALSE,
      id_bit_table_1x2,
      {  "a  b@",
         "a@b@"}},
   /* s1x3 */
      { 2,
      &thing1x3,
      &thing1x3,
      (cm_hunk *) 0,
      {b_1x3,       b_3x1},
      { 3, 1},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s2x2 */
      { 3,
      &thing2x2,
      &thing2x2,
      (cm_hunk *) 0,
      {b_2x2,       b_2x2},
      { 2, 2},
      TRUE,
      id_bit_table_2x2,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s1x4 */
      { 3,
      &thing1x4,
      &thing1x4,
      &concthing_1x4,
      {b_1x4,       b_4x1},
      { 4, 1},
      FALSE,
      id_bit_table_1x4,
      {  "a  b  d  c@",
         "a@b@d@c@"}},
   /* sdmd */
      { 3,
      &thingdmd,
      &nicethingdmd,
      &concthing_dmd,
      {b_dmd,       b_pmd},
      { 0, 2},
      FALSE,
      id_bit_table_dmd,
      {  "     b@a      c@     d@",
         "   a@@d  b@@   c@"}},
   /* s_star */
      { 3,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_star,      b_star},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "   b@a  c@   d@",
         (Cstring) 0}},
   /* s_trngl */
      { 2,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_trngl,     b_ptrngl},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_trngl4 */
      { 3,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_trngl4,    b_ptrngl4},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_bone6 */
      { 5,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_bone6,
      {b_bone6,     b_pbone6},
      { 0, 0},
      FALSE,
      id_bit_table_bone6,
      {  "a        b@    fc@e        d@",
         "ea@  f@  c@db@"}},
   /* s_short6 */
      { 5,
      (coordrec *) 0,
      &nicethingshort6,
      (cm_hunk *) 0,
      {b_short6,    b_pshort6},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "   b@a  c@f  d@   e@",
         "   fa@e      b@   dc@"}},
   /* s1x6 */
      { 5,
      &thing1x6,
      &thing1x6,
      (cm_hunk *) 0,
      {b_1x6,       b_6x1},
      { 6, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  f  e  d@",
         "a@b@c@f@e@d@"}},
   /* s2x3 */
      { 5,
      &thing2x3,
      &thing2x3,
      &concthing_2x3,
      {b_2x3,       b_3x2},
      { 3, 2},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c@f  e  d@",
         "f  a@e  b@d  c@"}},
   /* s_1x2dmd */
      { 5,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_1x2dmd,    b_p1x2dmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "           c@a  b      e  d@           f@",
         "   a@@   b@@f  c@@   e@@   d@"}},
   /* s_2x1dmd */
      { 5,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_2x1dmd,
      {b_2x1dmd,    b_p2x1dmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "         c@@a  b  e  d@@         f",
         "      a@@      b@f        c@      e@@      d"}},
   /* s_qtag */
      { 7,
      &thingqtag,
      &nicethingqtag,
      &concthing_qtag,
      {b_qtag,      b_pqtag},
      { 4, 0},
      FALSE,
      id_bit_table_qtag,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_bone */
      { 7,
      &thingbone,
      &thingbone,
      &concthing_bone,
      {b_bone,      b_pbone},
      { 0, 0},
      FALSE,
      id_bit_table_bone,
      {  "a                   b@    g h d c@f                   e",
         "fa@  g@  h@  d@  c@eb"}},
   /* s1x8 */
      { 7,
      &thing1x8,
      &thing1x8,
      &concthing_1x8,
      {b_1x8,       b_8x1},
      { 8, 1},
      FALSE,
      id_bit_table_1x8,
      {  "a b d c g h f e",
         "a@b@d@c@g@h@f@e"}},
   /* slittlestars */
      {7,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s1x3dmd */
      { 7,
      &thing1x3dmd,
      &thing1x3dmd,
      &concthing_1x3dmd,
      {b_1x3dmd,    b_p1x3dmd},
      { 0, 0},
      FALSE,
      id_bit_table_1x3dmd,
      {  "               d@a b c      g f e@               h",
         "   a@@   b@@   c@@h  d@@   g@@   f@@   e@"}},
   /* s3x1dmd */
      { 7,
      &thing3x1dmd,
      &thing3x1dmd,
      &concthing_3x1dmd,
      {b_3x1dmd,    b_p3x1dmd},
      { 0, 0},
      FALSE,
      id_bit_table_3x1dmd,
      {  "             d@@a b c g f e@@             h",
         "      a@@      b@@      c@h        d@      g@@      f@@      e"}},
   /* s_spindle */
      { 7,
      &thingspindle,
      &thingspindle,
      &concthing_spindle,
      {b_spindle,   b_pspindle},
      { 0, 0},
      FALSE,
      id_bit_table_spindle,
      {  "    a b c@h              d@    g f e",
         "  h@ga@fb@ec@  d"}},
   /* s_hrglass */
      { 7,
      &thingglass,
      &nicethingglass,
      &concthing_hrglass,
      {b_hrglass,   b_phrglass},
      { 0, 0},
      FALSE,
      id_bit_table_hrglass,
      {  "   a  b@      d@g        c@      h@   f  e",
         "     g@f      a@   hd@e      b@     c"}},
   /* s_dhrglass */
      { 7,
      &thingdglass,
      &thingdglass,
      &concthing_dhrglass,
      {b_dhrglass,  b_pdhrglass},
      { 0, 0},
      FALSE,
      id_bit_table_dhrglass,
      {  "a      d      b@     g      c@f      h      e",
         "f  a@@   g@@h  d@@   c@@e  b"}},
   /* s_hyperglass */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_crosswave */
      { 7,
      &thingxwv,
      &thingxwv,
      &concthing_xwave,
      {b_crosswave, b_pcrosswave},
      { 0, 0},
      FALSE,
      id_bit_table_crosswave,
      {  "          c@          d@ab        fe@          h@          g",
         "      a@      b@@ghdc@@      f@      e"}},
   /* s2x4 */
      { 7,
      &thing2x4,
      &thing2x4,
      &concthing_2x4,
      {b_2x4,       b_4x2},
      { 4, 2},
      FALSE,
      id_bit_table_2x4,
      {  "a  b  c  d@@h  g  f  e",
         "h  a@@g  b@@f  c@@e  d"}},
   /* s2x5 */
      {9,
      &thing2x5,
      &thing2x5,
      (cm_hunk *) 0,
      {b_2x5,       b_5x2},
      { 5, 2},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e@@j  i  h  g  f",
         "j  a@@i  b@@h  c@@g  d@@f  e"}},
   /* s_rigger */
      { 7,
      &thingrigger,
      &thingrigger,
      &concthing_rigger,
      {b_rigger,    b_prigger},
      { 0, 0},
      FALSE,
      id_bit_table_rigger,
      {  "        a b@gh         dc@        f e",
         "  g@  h@fa@eb@  d@  c"}},
   /* s3x4 */
      {11,
      &thing3x4,
      &thing3x4,
      &concthing_3x4,
      {b_3x4,       b_4x3},
      { 4, 3},
      FALSE,
      id_bit_table_3x4,
      {  "a  b  c  d@@k  l  f  e@@j  i  h  g",
         "j  k  a@@i  l  b@@h  f  c@@g  e  d"}},
   /* s2x6 */
      {11,
      &thing2x6,
      &thing2x6,
      &concthing_2x6,
      {b_2x6,       b_6x2},
      { 6, 2},
      FALSE,
      id_bit_table_2x6,
      {  "a  b  c  d  e  f@@l  k  j  i  h  g",
         "l  a@@k  b@@j  c@@i  d@@h  e@@g  f"}},
   /* d3x4 */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_d3x4,   b_d4x3},
      { 5, 3},
      FALSE,
      (id_bit_table *) 0,
      {  "a              e@   b  c  d@l              f@   j  i  h@k              g",
         "k  l  a@@   j  b@@   i  c@@   h  d@@g  f  e"}},
   /* s1p5x8 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h@p  o  n  m  l  k  j  i",
         "pa@@ob@@nc@@md@@le@@kf@@jg@@ih"}},
   /* s2x8 */
      {15,
      &thing2x8,
      &thing2x8,
      &concthing_2x8,
      {b_2x8,       b_8x2},
      { 8, 2},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h@@p  o  n  m  l  k  j  i",
         "p  a@@o  b@@n  c@@m  d@@l  e@@k  f@@j  g@@i  h"}},
   /* s4x4 */
      {15,
      &thing4x4,
      &thing4x4,
      &concthing_4x4,
      {b_4x4,       b_4x4},
      { 4, 4},
      TRUE,
      (id_bit_table *) 0,
      {  "m  n  o  a@@k  p  d  b@@j  l  h  c@@i  g  f  e",
         (Cstring) 0}},
   /* sx1x6 */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s1x10 */
      { 9,
      &thing1x10,
      &thing1x10,
      (cm_hunk *) 0,
      {b_1x10,      b_10x1},
      {10, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "a b c d e j i h g f",
         "a@b@c@d@e@j@i@h@g@f"}},
   /* s1x12 */
      {11,
      &thing1x12,
      &thing1x12,
      &concthing_1x12,
      {b_1x12,      b_12x1},
      {12, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "a b c d e f l k j i h g",
         "a@b@c@d@e@f@l@k@j@i@h@g"}},
   /* s1x14 */
      {13,
      &thing1x14,
      &thing1x14,
      (cm_hunk *) 0,
      {b_1x14,      b_14x1},
      {14, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "abcdefgnmlkjih",
         "a@b@c@d@e@f@g@n@m@l@k@j@i@h"}},
   /* s1x16 */
      {15,
      &thing1x16,
      &thing1x16,
      (cm_hunk *) 0,
      {b_1x16,      b_16x1},
      {16, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "abcdefghponmlkji",
         "a@b@c@d@e@f@g@h@p@o@n@m@l@k@j@i"}},
   /* s_c1phan */
      {15,
      &thingphan,
      &nicethingphan,
      (cm_hunk *) 0,
      {b_c1phan,    b_c1phan},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "   b        e@a  c  h  f@   d        g@@   o        l@n  p  k  i@   m        j",
         (Cstring) 0}},
   /* s_bigblob */
      {23,
      &thingblob,
      &thingblob,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "            a  b@@      v  w  c  d@@t  u  x  f  e  g@@s  q  r  l  i  h@@      p  o  k  j@@            n  m",
         (Cstring) 0}},
   /* s_ptpd */
      { 7,
      &thingptpd,
      &nicethingptpd,
      &concthing_ptpd,
      {b_ptpd,      b_pptpd},
      { 0, 0},
      FALSE,
      id_bit_table_ptpd,
      {  "    b           h@a    c   g    e@    d           f",
         "  a@@db@@  c@@  g@@fh@@  e"}},
   /* s3dmd */
      {11,
      &thing3dmd,
      &thing3dmd,
      &concthing_3dmd,
      {b_3dmd,      b_p3dmd},
      { 0, 0},
      FALSE,
      id_bit_table_3dmd,
      {  "   a      b      c@@j k l f e d@@   i      h      g",
         "      j@i        a@      k@@      l@h        b@      f@@      e@g        c@      d"}},
   /* s4dmd */
      {15,
      &thing4dmd,
      &thing4dmd,
      &concthing_4dmd,
      {b_4dmd,      b_p4dmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "   a      b      c      d@@m n o p h g f e@@   l      k      j      i",
         "      m@l        a@      n@@      o@k        b@      p@@      h@j        c@      g@@      f@i        d@      e"}},
   /* s_wingedstar */
      { 7,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_wstar,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "             d@a b c  g f e@             h",
         "   a@@   b@@   c@h  d@   g@@   f@@   e"}},
   /* s_wingedstar12 */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_wstar12,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "             d       f@a b c  e k  i h g@             l       j",
         "   a@@   b@@   c@l  d@   e@   k@j  f@   i@@   h@@   g"}},
   /* s_wingedstar16 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_wstar16,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "             d       h       m@a b c  f g  o n  k j i@             e       p       l",
         "   a@@   b@@   c@e  d@   f@   g@p  h@   o@   n@l  m@   k@@   j@@   i"}},
   /* s_barredstar */
      { 9,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_bstar,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  " a          c@        b@i j  e d@        g@ h          f",
         "h  i  a@@      j@   g  b@      e@@f  d  c"}},
   /* s_barredstar12 */
      {13,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_bstar12,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  " a                     d@        b       c@l m  n g  f e@        j       i@ k                     h",
         "k  l  a@      m@   j  b@      n@@      g@   i  c@      f@@h  e  d"}},
   /* s_barredstar16 */
      {17,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_bstar16,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  " a                                e@        b       c       d@o p  q r  i h  g f@        m       l       k@ n                                j",
         "n  o  a@      p@   m  b@      q@      r@   l  c@      i@@      h@   k  d@      g@@j  f  e"}},
   /* s_galaxy */
      { 7,
      &thinggal,
      &thinggal,
      &concthing_gal,
      {b_galaxy,    b_galaxy},
      { 0, 0},
      TRUE,
      id_bit_table_gal,
      {  "     c@   bd@a      e@   hf@     g",
         (Cstring) 0}},
   /* s3x6 */
      {17,
      &thing3x6,
      &thing3x6,
      (cm_hunk *) 0,
      {b_3x6,       b_6x3},
      { 6, 3},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f@@p  q  r  i  h  g@@o  n  m  l  k  j",
         "o  p  a@@n  q  b@@m  r  c@@l  i  d@@k  h  e@@j  g  f"}},
   /* s3x8 */
      {23,
      &thing3x8,
      &thing3x8,
      (cm_hunk *) 0,
      {b_3x8,       b_8x3},
      { 8, 3},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h@@u  v  w  x  l  k  j  i@@t  s  r  q  p  o  n  m",
         "t  u  a@@s  v  b@@r  w  c@@q  x  d@@p  l  e@@o  k  f@@n  j  g@@m  i  h"}},
   /* s4x5 */
      {19,
      &thing4x5,
      &thing4x5,
      (cm_hunk *) 0,
      {b_4x5,       b_5x4},
      { 5, 4},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e@@j  i  h  g  f@@p  q  r  s  t@@o  n  m  l  k",
         "o  p  j  a@@n  q  i  b@@m  r  h  c@@l  s  g  d@@k  t  f  e"}},
   /* s4x6 */
      {23,
      &thing4x6,
      &thing4x6,
      (cm_hunk *) 0,
      {b_4x6,       b_6x4},
      { 6, 4},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f@@l  k  j  i  h  g@@s  t  u  v  w  x@@r  q  p  o  n  m",
         "r  s  l  a@@q  t  k  b@@p  u  j  c@@o  v  i  d@@n  w  h  e@@m  x  g  f"}},
   /* s3oqtg */
      {19,
      &thing3oqtg,
      &thing3oqtg,
      (cm_hunk *) 0,
      {b_3oqtg,     b_p3oqtg},
      { 7, 4},
      FALSE,
      (id_bit_table *) 0,
      {  "      a        b        c@@r  s  t  g  f  e  d@@n  o  p  q  j  i  h@@      m        l        k",
         "      n  r@@m  o  s  a@@      p  t@@l  q  b  g@@      j  f@@k  i  e  c@@      h  d"}},
   /* s_thar */
      { 7,
      &thingthar,
      &thingthar,
      &concthing_thar,
      {b_thar,      b_thar},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "      c@      d@abfe@      h@      g",
         (Cstring) 0}},
   /* s_alamo */
      { 7,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_alamo,     b_alamo},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "   ab@h   c@g   d@   fe",
         (Cstring) 0}},
   /* sx4dmd */
      {31,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s8x8 */
      {63,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* sx1x16 */
      {31,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* sfat2x8 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 8, 4},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* swide4x4 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 8, 4},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* sbigh */
      {11,
      &thingbigh,
      &thingbigh,
      &concthing_bigh,
      {b_bigh,      b_pbigh},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "a                   j@b                   i@    e f l k@c                   h@d                   g",
         "dcba@      e@      f@      l@      k@ghij"}},
   /* sbigx */
      {11,
      &thingbigx,
      &thingbigx,
      &concthing_bigx,
      {b_bigx,      b_pbigx},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "                e@                f@abcd    jihg@                l@                k",
         "      a@      b@      c@      d@klfe@      j@      i@      h@      g"}},
   /* sbigrig */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      &concthing_bigrig,
      {b_bigrig,    b_pbigrig},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "                e f@abcd         jihg@                l k",
         "  a@  b@  c@  d@le@kf@  j@  i@  h@  g"}},
   /* sbighrgl */
      {11,
      (coordrec *) 0,   /* FIX */
      (coordrec *) 0,   /* FIX */
      &concthing_bighrgl,
      {b_bighrgl,   b_pbighrgl},
      { 0, 0},
      FALSE,
      id_bit_table_bighrgl,
      {  "          c@a b      e f@        jd@l k      h g@          i",
         "   l  a@@   k  b@      j@i        c@      d@   h  e@@   g  f"}},
   /* sbigdhrgl */
      {11,
      (coordrec *) 0,   /* FIX */
      (coordrec *) 0,   /* FIX */
      &concthing_bigdhrgl,
      {b_bigdhrgl,   b_pbigdhrgl},
      { 0, 0},
      FALSE,
      id_bit_table_bigdhrgl,
      {  "a  b      c      e  f@           j      d@l  k      i      h  g",
         "l  a@@k  b@@   j@@i  c@@   d@@h  e@@g  f"}},
   /* sbigbone */
      {11,
      &thingbigbone,
      &thingbigbone,
      &concthing_bigbone,
      {b_bigbone,   b_pbigbone},
      { 0, 0},
      FALSE,
      id_bit_table_bigbone,
      {  "a  b                   e  f@          c d j i@l  k                   h  g",
         "la@kb@  c@  d@  j@  i@he@gf"}},
   /* sbigdmd */
      {11,
      &thingbigdmd,
      &thingbigdmd,
      &concthing_bigdmd,
      {b_bigdmd,    b_pbigdmd},
      { 0, 0},
      FALSE,
      id_bit_table_bigdmd,
      {  "          c@a b        e f@          d@@          j@l k        h g@          i",
         "   l      a@   k      b@i j d c@   h      e@   g      f"}},
   /* s_dead_concentric */
      {-1,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_normal_concentric */
      {-1,
      (coordrec *) 0,
      (coordrec *) 0,
      (cm_hunk *) 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}}};


/* BEWARE!!  This list is keyed to the definition of "begin_kind" in database.h . */
/*   It must also match the similar table in the database-building program. */
int begin_sizes[] = {
   0,          /* b_nothing */
   1,          /* b_1x1 */
   2,          /* b_1x2 */
   2,          /* b_2x1 */
   3,          /* b_1x3 */
   3,          /* b_3x1 */
   4,          /* b_2x2 */
   4,          /* b_dmd */
   4,          /* b_pmd */
   4,          /* b_star */
   6,          /* b_trngl */
   6,          /* b_ptrngl */
   8,          /* b_trngl4 */
   8,          /* b_ptrngl4 */
   6,          /* b_bone6 */
   6,          /* b_pbone6 */
   6,          /* b_short6 */
   6,          /* b_pshort6 */
   6,          /* b_1x2dmd */
   6,          /* b_p1x2dmd */
   6,          /* b_2x1dmd */
   6,          /* b_p2x1dmd */
   8,          /* b_qtag */
   8,          /* b_pqtag */
   8,          /* b_bone */
   8,          /* b_pbone */
   8,          /* b_rigger */
   8,          /* b_prigger */
   8,          /* b_spindle */
   8,          /* b_pspindle */
   8,          /* b_hrglass */
   8,          /* b_phrglass */
   8,          /* b_dhrglass */
   8,          /* b_pdhrglass */
   8,          /* b_crosswave */
   8,          /* b_pcrosswave */
   4,          /* b_1x4 */
   4,          /* b_4x1 */
   8,          /* b_1x8 */
   8,          /* b_8x1 */
   8,          /* b_2x4 */
   8,          /* b_4x2 */
   6,          /* b_2x3 */
   6,          /* b_3x2 */
  10,          /* b_2x5 */
  10,          /* b_5x2 */
   6,          /* b_1x6 */
   6,          /* b_6x1 */
   12,         /* b_3x4 */
   12,         /* b_4x3 */
   12,         /* b_2x6 */
   12,         /* b_6x2 */
   12,         /* b_d3x4 */
   12,         /* b_d4x3 */
   16,         /* b_2x8 */
   16,         /* b_8x2 */
   16,         /* b_4x4 */
   10,         /* b_1x10 */
   10,         /* b_10x1 */
   12,         /* b_1x12 */
   12,         /* b_12x1 */
   14,         /* b_1x14 */
   14,         /* b_14x1 */
   16,         /* b_1x16 */
   16,         /* b_16x1 */
   16,         /* b_c1phan */
   8,          /* b_galaxy */
   18,         /* b_3x6 */
   18,         /* b_6x3 */
   24,         /* b_3x8 */
   24,         /* b_8x3 */
   20,         /* b_4x5 */
   20,         /* b_5x4 */
   24,         /* b_4x6 */
   24,         /* b_6x4 */
   20,         /* b_3oqtg */
   20,         /* b_p3oqtg */
   8,          /* b_thar */
   8,          /* b_alamo */
   8,          /* b_ptpd */
   8,          /* b_pptpd */
   8,          /* b_1x3dmd */
   8,          /* b_p1x3dmd */
   8,          /* b_3x1dmd */
   8,          /* b_p3x1dmd */
   12,         /* b_3dmd */
   12,         /* b_p3dmd */
   16,         /* b_4dmd */
   16,         /* b_p4dmd */
   12,         /* b_bigh */
   12,         /* b_pbigh */
   12,         /* b_bigx */
   12,         /* b_pbigx */
   12,         /* b_bigrig */
   12,         /* b_pbigrig */
   12,         /* b_bighrgl */
   12,         /* b_pbighrgl */
   12,         /* b_bigdhrgl */
   12,         /* b_pbigdhrgl */
   12,         /* b_bigbone */
   12,         /* b_pbigbone */
   12,         /* b_bigdmd */
   12};        /* b_pbigdmd */


/* The following 8 definitions are taken verbatim from sdinit.c . */
#define B1A (0000 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_HCOR|ID1_PERM_HEAD|ID1_PERM_BOY)
#define G1A (0100 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_HEAD|ID1_PERM_GIRL)
#define B2A (0200 | ID1_PERM_NSG|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_SIDE|ID1_PERM_BOY)
#define G2A (0300 | ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_HCOR|ID1_PERM_SIDE|ID1_PERM_GIRL)
#define B3A (0400 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_HCOR|ID1_PERM_HEAD|ID1_PERM_BOY)
#define G3A (0500 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_HEAD|ID1_PERM_GIRL)
#define B4A (0600 | ID1_PERM_NSG|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_SIDE|ID1_PERM_BOY)
#define G4A (0700 | ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_HCOR|ID1_PERM_SIDE|ID1_PERM_GIRL)

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h. */
startinfo startinfolist[] = {
   {
      "???",      /* A non-existent setup. */
      FALSE,                         /* into_the_middle */
      {
         nothing,                    /* kind */
         1,                          /* rotation */
         {0},                        /* cmd */
         {{0,0}},                    /* people */
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "Heads 1P2P",
      FALSE,                         /* into_the_middle */
      {
         s2x4,                       /* kind */
         1,                          /* rotation */
         {0},                        /* cmd */
         {{G2A|d_south,0}, {B2A|d_south,0}, {G1A|d_south,0}, {B1A|d_south,0},
            {G4A|d_north,0}, {B4A|d_north,0}, {G3A|d_north,0}, {B3A|d_north,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "Sides 1P2P",
      FALSE,                         /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{G3A|d_south,0}, {B3A|d_south,0}, {G2A|d_south,0}, {B2A|d_south,0},
            {G1A|d_north,0}, {B1A|d_north,0}, {G4A|d_north,0}, {B4A|d_north,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "HEADS",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{B4A|d_east,0}, {G3A|d_south,0}, {B3A|d_south,0}, {G2A|d_west,0},
            {B2A|d_west,0}, {G1A|d_north,0}, {B1A|d_north,0}, {G4A|d_east,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "SIDES",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         1,                          /* rotation */
         {0},                        /* cmd */
         {{B3A|d_east,0}, {G2A|d_south,0}, {B2A|d_south,0}, {G1A|d_west,0},
            {B1A|d_west,0}, {G4A|d_north,0}, {B4A|d_north,0}, {G3A|d_east,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "From squared set",
      FALSE,                         /* into_the_middle */
      {
         s4x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{0,0}, {G2A|d_west,0}, {B2A|d_west,0}, {0,0}, {0,0}, {G1A|d_north,0},
            {B1A|d_north,0}, {0,0}, {0,0}, {G4A|d_east,0}, {B4A|d_east,0}, {0,0},
            {0,0}, {G3A|d_south,0}, {B3A|d_south,0}, {0,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   }
#ifdef RIPNSNORT
   {
      "1/4 stable, couple #3 rip 'n' snort, then HEADS",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{B4A|d_south,0}, {G3A|d_west,0}, {B3A|d_east,0}, {G2A|d_south,0},
            {B2A|d_south,0}, {G1A|d_west,0}, {B1A|d_east,0}, {G4A|d_south,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   }
#endif
};



/* Nonzero item in map3[1] for arity 1 map means do not reassemble. */
/* For maps with arity 1, a nonzero item in map3[0] means that the "offset goes away" warning should be given in certain cases. */
/* For maps with arity 3, a value of 3 after the last map means that the "overlap goes away" warning should be given in certain cases. */

/*                                                                                                          setups are stacked on top of each other ----------------------|
                                                                                        each setup is rotated clockwise before being concatenated -------------------|    |
                                                                                                                                                                     V    V
                                             map1                             map2                        map3 map4   map_kind     warncode  arity outer   inner    rot  vert */

        map_thing map_b6_trngl          = {{5, 4, 0,                          2, 1, 3},                              MPKIND__SPLIT,       0, 2,  s_bone6, s_trngl,  0x207, 0};
        map_thing map_s6_trngl          = {{4, 5, 3,                          1, 2, 0},                              MPKIND__SPLIT,       0, 2,  s_short6, s_trngl, 0x108, 1};
        map_thing map_bone_trngl4       = {{7, 6, 5, 0,                       3, 2, 1, 4},                           MPKIND__SPLIT,       0, 2,  s_bone, s_trngl4,  0x207, 0};
        map_thing map_rig_trngl4        = {{6, 7, 0, 5,                       2, 3, 4, 1},                           MPKIND__SPLIT,       0, 2,  s_rigger,s_trngl4, 0x10D, 0};
        map_thing map_s8_tgl4           = {{2, 7, 5, 0,                       6, 3, 1, 4},                           MPKIND__REMOVED,     0, 2,  s_bone, s_trngl4,  0x207, 0};
        map_thing map_p8_tgl4           = {{0, 2, 7, 5,                       4, 6, 3, 1},                           MPKIND__REMOVED,     0, 2,  s_ptpd, s_trngl4,  0x10D, 0};
        map_thing map_spndle_once_rem   = {{7, 1, 3, 5,                       0, 2, 4, 6, (int) s2x2},               MPKIND__SPEC_ONCEREM,0, 2,  s_spindle, sdmd,   0x000, 0};
        map_thing map_1x3dmd_once_rem   = {{0, 2, 4, 6,                       1, 3, 5, 7, (int) sdmd},               MPKIND__SPEC_ONCEREM,0, 2,  s1x3dmd, s1x4,     0x000, 0};
        map_thing map_phan_trngl4a      = {{9, 11, 7, 5,                      1, 3, 15, 13},                         MPKIND__SPLIT,       0, 2,  s_c1phan,s_trngl4, 0x208, 0};
        map_thing map_phan_trngl4b      = {{12, 14, 0, 2,                     4, 6, 8, 10},                          MPKIND__SPLIT,       0, 2,  s_c1phan,s_trngl4, 0x108, 0};
        map_thing map_lh_zzztgl         = {{11, 7, 2, 4,                      3, 15, 10, 12},                        MPKIND__OFFS_L_HALF, 4, 2,  s4x4,   s_trngl4,  0x00D, 1};
        map_thing map_rh_zzztgl         = {{7, 11, 8, 9,                      15, 3, 0, 1},                          MPKIND__OFFS_R_HALF, 4, 2,  s4x4,   s_trngl4,  0x007, 1};
        map_thing map_2x2v              = {{0, 3,                             1, 2},                                 MPKIND__SPLIT,       0, 2,  s2x2,   s1x2,      0x005, 0};
        map_thing map_2x4_magic         = {{0, 6, 3, 5,                       7, 1, 4, 2},                           MPKIND__NONE,        0, 2,  s2x4,   s1x4,      0x000, 0};
        map_thing map_qtg_magic         = {{0, 2, 5, 3,                       1, 7, 4, 6},                           MPKIND__NONE,        0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_qtg_intlk         = {{0, 3, 5, 6,                       1, 2, 4, 7},                           MPKIND__NONE,        0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_qtg_magic_intlk   = {{0, 2, 5, 7,                       1, 3, 4, 6},                           MPKIND__NONE,        0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_ptp_magic         = {{6, 1, 4, 3,                       0, 7, 2, 5},                           MPKIND__NONE,        0, 2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_ptp_intlk         = {{0, 1, 6, 3,                       2, 7, 4, 5},                           MPKIND__NONE,        0, 2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_ptp_magic_intlk   = {{2, 1, 4, 3,                       0, 7, 6, 5},                           MPKIND__NONE,        0, 2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_2x4_diagonal      = {{2, 3, 6, 7,                       0, 1, 4, 5},                           MPKIND__NONE,        0, 2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_2x4_int_pgram     = {{1, 3, 5, 7,                       0, 2, 4, 6},                           MPKIND__NONE,        0, 2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_2x4_trapezoid     = {{1, 2, 4, 7,                       0, 3, 5, 6},                           MPKIND__NONE,        0, 2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_3x4_2x3_intlk     = {{2, 5, 7, 9, 10, 0,                3, 4, 6, 8, 11, 1},                    MPKIND__NONE,        0, 2,  s3x4,   s2x3,      0x005, 0};
        map_thing map_3x4_2x3_conc      = {{3, 4, 6, 9, 10, 0,                2, 5, 7, 8, 11, 1},                    MPKIND__NONE,        0, 2,  s3x4,   s2x3,      0x005, 0};
        map_thing map_4x4_ns            = {{12, 13, 0, 14,                    8, 6, 4, 5},                           MPKIND__NONE,        0, 2,  s4x4,   s1x4,      0x000, 1};
        map_thing map_4x4_ew            = {{0, 1, 4, 2,                       12, 10, 8, 9},                         MPKIND__NONE,        0, 2,  s4x4,   s1x4,      0x005, 1};
        map_thing map_phantom_box       = {{0, 1, 6, 7, 8, 9, 14, 15,         2, 3, 4, 5, 10, 11, 12, 13},           MPKIND__CONCPHAN,    0, 2,  s2x8,   s2x4,      0x000, 0};
        map_thing map_intlk_phantom_box = {{0, 1, 4, 5, 10, 11, 14, 15,       2, 3, 6, 7, 8, 9, 12, 13},             MPKIND__INTLK,       0, 2,  s2x8,   s2x4,      0x000, 0};
        map_thing map_phantom_dmd       = {{0, 3, 4, 5, 8, 11, 12, 13,        1, 2, 6, 7, 9, 10, 14, 15},            MPKIND__CONCPHAN,    0, 2,  s4dmd,  s_qtag,    0x000, 0};
        map_thing map_intlk_phantom_dmd = {{0, 2, 6, 7, 9, 11, 12, 13,        1, 3, 4, 5, 8, 10, 14, 15},            MPKIND__INTLK,       0, 2,  s4dmd,  s_qtag,    0x000, 0};
        map_thing map_stagger           = {{10, 13, 3, 0, 2, 5, 11, 8,        12, 15, 14, 1, 4, 7, 6, 9},            MPKIND__STAG,        0, 2,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_staggerv          = {{12, 15, 14, 1, 4, 7, 6, 9,        10, 13, 3, 0, 2, 5, 11, 8},            MPKIND__STAG,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_stairst           = {{9, 13, 7, 0, 1, 5, 15, 8,         12, 11, 14, 2, 4, 3, 6, 10},           MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_ladder            = {{10, 15, 14, 0, 2, 7, 6, 8,        12, 13, 3, 1, 4, 5, 11, 9},            MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_offset            = {{9, 11, 14, 0, 1, 3, 6, 8,         12, 13, 7, 2, 4, 5, 15, 10},           MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_but_o             = {{10, 13, 14, 1, 2, 5, 6, 9,        12, 15, 3, 0, 4, 7, 11, 8},            MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_o_s2x4_3          = {{10, 13, 14, 1, 2, 5, 6, 9,        12, 15, 3, 0, 4, 7, 11, 8},            MPKIND__O_SPOTS,     0, 2,  s4x4,   s2x4,      0x000, 0};
        map_thing map_x_s2x4_3          = {{10, 13, 14, 1, 2, 5, 6, 9,        12, 15, 3, 0, 4, 7, 11, 8},            MPKIND__X_SPOTS,     0, 2,  s4x4,   s2x4,      0x000, 0};
Private map_thing map_o_s2x4_2          = {{14, 1, 2, 5, 6, 9, 10, 13,        0, 3, 7, 4, 8, 11, 15, 12},            MPKIND__O_SPOTS,     0, 2,  s4x4,   s2x4,      0x005, 0};
Private map_thing map_x_s2x4_2          = {{14, 1, 2, 5, 6, 9, 10, 13,        0, 3, 7, 4, 8, 11, 15, 12},            MPKIND__X_SPOTS,     0, 2,  s4x4,   s2x4,      0x005, 0};
        map_thing map_4x4v     = {{12, 10, 8, 9,         13, 15, 6, 11,        14, 3, 5, 7,      0, 1, 4, 2},        MPKIND__SPLIT,       0, 4,  s4x4,   s1x4,      0x055, 0};
        map_thing map_blocks   = {{12, 14, 7, 9,         13, 0, 2, 11,         15, 1, 4, 6,      10, 3, 5, 8},       MPKIND__NONE,        0, 4,  s4x4,   s2x2,      0x000, 0};
        map_thing map_trglbox  = {{12, 14, 15, 9,        13, 0, 2, 3,          7, 1, 4, 6,       10, 11, 5, 8},      MPKIND__NONE,        0, 4,  s4x4,   s2x2,      0x000, 0};
Private map_thing map_4x4_1x4  = {{8, 6, 4, 5,           9, 11, 2, 7,          10, 15, 1, 3,     12, 13, 0, 14},     MPKIND__SPLIT,       0, 4,  s4x4,   s1x4,      0x000, 1};
Private map_thing map_1x16_1x4 = {{0, 1, 3, 2,           4, 5, 7, 6,           15, 14, 12, 13,   11, 10, 8, 9},      MPKIND__SPLIT,       0, 4,  s1x16,  s1x4,      0x000, 0};
Private map_thing map_2x8_2x2  = {{0, 1, 14, 15,         2, 3, 12, 13,         4, 5, 10, 11,     6, 7, 8, 9},        MPKIND__SPLIT,       0, 4,  s2x8,   s2x2,      0x000, 0};
Private map_thing map_conc_qb  = {{0, 7, 8, 15,          1, 6, 9, 14,          2, 5, 10, 13,     3, 4, 11, 12},      MPKIND__CONCPHAN,    0, 4,  s2x8,   s2x2,      0x000, 0};
Private map_thing map_4dmd_dmd = {{0, 13, 11, 12,        1, 15, 10, 14,        2, 6, 9, 7,       3, 4, 8, 5},        MPKIND__SPLIT,       0, 4,  s4dmd,  sdmd,      0x055, 0};
Private map_thing map_2x6_1x6           = {{11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 2,  s2x6,   s1x6,      0x000, 1};
Private map_thing map_3x6_1x6           = {{14, 13, 12, 9, 10, 11,            15, 16, 17, 6, 7, 8,
                                                                              0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 3,  s3x6,   s1x6,      0x000, 1};
Private map_thing map_2x8_1x8           = {{15, 14, 12, 13, 8, 9, 11, 10,     0, 1, 3, 2, 7, 6, 4, 5},               MPKIND__SPLIT,       0, 2,  s2x8,   s1x8,      0x000, 1};
Private map_thing map_3x8_1x8           = {{19, 18, 16, 17, 12, 13, 15, 14,   20, 21, 23, 22, 8, 9, 11, 10,
                                                                              0, 1, 3, 2, 7, 6, 4, 5},               MPKIND__SPLIT,       0, 3,  s3x8,   s1x8,      0x000, 1};
Private map_thing map_1x12_1x6          = {{0, 1, 2, 5, 4, 3,                 11, 10, 9, 6, 7, 8},                   MPKIND__SPLIT,       0, 2,  s1x12,  s1x6,      0x000, 0};
/* Special maps for putting back end-to-end 1x6's, or 1x2 diamonds, resulting from collisions at one end. */
        map_thing map_1x8_1x6           = {{0, 1, 3, -1, -1, 2,               -1, -1, 6, 4, 5, 7},                   MPKIND__NONE,        0, 2,  s1x8,   s1x6,      0x000, 0};
        map_thing map_rig_1x6           = {{6, 7, 0, -1, -1, 5,               -1, -1, 1, 2, 3, 4},                   MPKIND__NONE,        0, 2,  s_rigger,s_1x2dmd, 0x000, 0};
Private map_thing map_1x16_1x8          = {{0, 1, 3, 2, 7, 6, 4, 5,           15, 14, 12, 13, 8, 9, 11, 10},         MPKIND__SPLIT,       0, 2,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_intlk_phan_grand  = {{0, 1, 3, 2, 12, 13, 15, 14,       4, 5, 7, 6, 8, 9, 11, 10},             MPKIND__INTLK,       0, 2,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_conc_phan_grand   = {{0, 1, 3, 2, 8, 9, 11, 10,         4, 5, 7, 6, 12, 13, 15, 14},           MPKIND__CONCPHAN,    0, 2,  s1x16,  s1x8,      0x000, 0};
        map_thing map_hv_2x4_2          = {{0, 1, 2, 3, 12, 13, 14, 15,       4, 5, 6, 7, 8, 9, 10, 11},             MPKIND__SPLIT,       0, 2,  s2x8,   s2x4,      0x000, 0};
        map_thing map_3x4_2x3           = {{1, 11, 8, 9, 10, 0,               3, 4, 6, 7, 5, 2},                     MPKIND__SPLIT,       0, 2,  s3x4,   s2x3,      0x005, 0};
        map_thing map_vsplit_f          = {{13, 15, 11, 6, 8, 9, 10, 12,      0, 1, 2, 4, 5, 7, 3, 14},              MPKIND__SPLIT,       0, 2,  s4x4,   s2x4,      0x005, 0};
        map_thing map_split_f           = {{9, 11, 7, 2, 4, 5, 6, 8,          12, 13, 14, 0, 1, 3, 15, 10},          MPKIND__SPLIT,       0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_intlk_f           = {{10, 15, 3, 1, 4, 5, 6, 8,         12, 13, 14, 0, 2, 7, 11, 9},           MPKIND__INTLK,       0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_full_f            = {{12, 13, 14, 0, 4, 5, 6, 8,        10, 15, 3, 1, 2, 7, 11, 9},            MPKIND__CONCPHAN,    0, 2,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_2x4_2x2           = {{0, 1, 6, 7,                       2, 3, 4, 5},                           MPKIND__SPLIT,       0, 2,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_2x3_1x3           = {{5, 4, 3,                          0, 1, 2},                              MPKIND__SPLIT,       0, 2,  s2x3,   s1x3,      0x000, 1};
Private map_thing map_bigd_12d          = {{11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 2,  sbigdmd,s_1x2dmd,  0x000, 1};
Private map_thing map_bone_12d          = {{-1, -1, 0, 7, 6, 5,               3, 2, 1, -1, -1, 4},                   MPKIND__SPLIT,       0, 2,  s_bone, s_1x2dmd,  0x000, 0};
Private map_thing map_2x4_1x4           = {{7, 6, 4, 5,                       0, 1, 3, 2},                           MPKIND__SPLIT,       0, 2,  s2x4,   s1x4,      0x000, 1};
Private map_thing map_qtg_dmd           = {{0, 7, 5, 6,                       1, 2, 4, 3},                           MPKIND__SPLIT,       0, 2,  s_qtag, sdmd,      0x005, 0};
Private map_thing map_ptp_dmd           = {{0, 1, 2, 3,                       6, 7, 4, 5},                           MPKIND__SPLIT,       0, 2,  s_ptpd, sdmd,      0x000, 0};
Private map_thing map_3x8_3x4           = {{0, 1, 2, 3, 23, 22, 16, 17, 18, 19, 20, 21,
                                                                   4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 11, 10},        MPKIND__SPLIT,       0, 2,  s3x8,   s3x4,      0x000, 0};
Private map_thing map_4x6_3x4           = {{2, 9, 20, 15, 16, 19, 17, 18, 11, 0, 1, 10,
                                                                   5, 6, 23, 12, 13, 22, 14, 21, 8, 3, 4, 7},        MPKIND__SPLIT,       0, 2,  s4x6,   s3x4,      0x005, 0};
Private map_thing map_3x6_3x4           = {{1, 2, 3, 4, 11, 23, 15, 16, 17, 18, 21, 22,
                                                                   3, 4, 5, 6, 9, 10, 13, 14, 15, 16, 23, 11},       MPKIND__OVERLAP,     0, 2,  s3x8,   s3x4,      0x000, 0};
Private map_thing map_4x6_2x6           = {{18, 19, 20, 21, 22, 23, 12, 13, 14, 15, 16, 17,
                                                                   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},            MPKIND__SPLIT,       0, 2,  s4x6,   s2x6,      0x000, 1};
Private map_thing map_3dmd_dmd = {{0, 10, 8, 9,                    1, 5, 7, 11,                   2, 3, 6, 4},       MPKIND__SPLIT,       0, 3,  s3dmd,  sdmd,      0x015, 0};
        map_thing map_4x6_2x4  = {{1, 10, 19, 16, 17, 18, 11, 0,   3, 8, 21, 14, 15, 20, 9, 2,
                                                                   5, 6, 23, 12, 13, 22, 7, 4},                      MPKIND__SPLIT,       0, 3,  s4x6,   s2x4,      0x015, 0};
        map_thing map_hv_qtg_2          = {{0, 1, 15, 14, 10, 11, 12, 13,     2, 3, 4, 5, 8, 9, 7, 6},               MPKIND__SPLIT,       0, 2,  s4dmd,  s_qtag,    0x000, 0};
        map_thing map_vv_qtg_2          = {{9, 20, 16, 19, 18, 11, 1, 10,     6, 23, 13, 22, 21, 8, 4, 7},           MPKIND__SPLIT,       0, 2,  s4x6,   s_qtag,    0x005, 0};
Private map_thing map_spin_3x4          = {{1, 11, 8, -1, 9, 10, 0, -1,       3, 4, 6, -1, 7, 5, 2, -1},             MPKIND__SPLIT,       0, 2,  s3x4,   s_spindle, 0x005, 0};
Private map_thing map_hrgl_ptp          = {{-1, -1, 2, 1, -1, -1, 0, 3,       -1, -1, 4, 7, -1, -1, 6, 5},           MPKIND__SPLIT,       0, 2,  s_ptpd, s_hrglass, 0x000, 0};
        map_thing map_ov_hrg_1          = {{-1, -1, 5, 7, -1, -1, 0, 6,       -1, -1, 4, 2, -1, -1, 1, 3},           MPKIND__OVERLAP,     0, 2,  s_qtag, s_hrglass, 0x005, 0};
        map_thing map_ov_gal_1          = {{-1, 0, -1, 1, -1, 6, -1, 7,       -1, 2, -1, 3, -1, 4, -1, 5},           MPKIND__OVERLAP,     0, 2,  s2x4,   s_galaxy,  0x000, 0};


        map_thing map_3o_qtag_1         = {{19, 15, 12, 14, 13, 17, 0, 18,    5, 9, 11, 16, 15, 19, 1, 6,
                                                                              3, 7, 10, 8, 9, 5, 2, 4},              MPKIND__OVERLAP,     0, 3,  s3oqtg, s_qtag,    0x015, 0};


Private map_thing map_1x6_1x3           = {{0, 1, 2,                          5, 4, 3},                              MPKIND__SPLIT,       0, 2,  s1x6,   s1x3,      0x000, 0};
Private map_thing map_1x8_1x4           = {{0, 1, 2, 3,                       6, 7, 4, 5},                           MPKIND__SPLIT,       0, 2,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_1x12_1x4          = {{0, 1, 3, 2,            4, 5, 10, 11,          9, 8, 6, 7},               MPKIND__SPLIT,       0, 3,  s1x12,  s1x4,      0x000, 0};
Private map_thing map_1x2_1x1           = {{0,                                1},                                    MPKIND__SPLIT,       0, 2,  s1x2,   s1x1,      0x000, 0};
Private map_thing map_1x4_1x2           = {{0, 1,                             3, 2},                                 MPKIND__SPLIT,       0, 2,  s1x4,   s1x2,      0x000, 0};
        map_thing map_tgl4_1            = {{1, 0,                             2, 3},                                 MPKIND__NONE,        0, 2,  s_trngl4,s1x2,     0x001, 1};
        map_thing map_tgl4_2            = {{3, 2,                             1, 0},                                 MPKIND__NONE,        0, 2,  s_trngl4,s1x2,     0x006, 1};
Private map_thing map_3x6_2x3           = {{1, 16, 13, 14, 15, 0,  3, 8, 11, 12, 17, 2,   5, 6, 9, 10, 7, 4},        MPKIND__SPLIT,       0, 3,  s3x6,   s2x3,      0x015, 0};
Private map_thing map_2x6_2x2           = {{0, 1, 10, 11,          2, 3, 8, 9,            4, 5, 6, 7},               MPKIND__SPLIT,       0, 3,  s2x6,   s2x2,      0x000, 0};
Private map_thing map_2x6_2x2r          = {{1, 10, 11, 0,          3, 8, 9, 2,            5, 6, 7, 4},               MPKIND__SPLIT,       0, 3,  s2x6,   s2x2,      0x015, 0};
Private map_thing map_conc_tb           = {{0, 5, 6, 11,           1, 4, 7, 10,           2, 3, 8, 9},               MPKIND__CONCPHAN,    0, 3,  s2x6,   s2x2,      0x000, 0};
Private map_thing map_conc_tbr          = {{5, 6, 11, 0,           4, 7, 10, 1,           3, 8, 9, 2},               MPKIND__CONCPHAN,    0, 3,  s2x6,   s2x2,      0x015, 0};
Private map_thing map_3x4_1x4           = {{9, 8, 6, 7,            10, 11, 4, 5,          0, 1, 3, 2},               MPKIND__SPLIT,       0, 3,  s3x4,   s1x4,      0x000, 1};
Private map_thing map_2x3_1x2           = {{0, 5,                  1, 4,                  2, 3},                     MPKIND__SPLIT,       0, 3,  s2x3,   s1x2,      0x015, 0};
Private map_thing map_1x6_1x2           = {{0, 1,                  2, 5,                  4, 3},                     MPKIND__SPLIT,       0, 3,  s1x6,   s1x2,      0x000, 0};
Private map_thing map_2x2h              = {{3, 2,                             0, 1},                                 MPKIND__SPLIT,       0, 2,  s2x2,   s1x2,      0x000, 1};
Private map_thing map_2x4_2x2r          = {{1, 6, 7, 0,                       3, 4, 5, 2},                           MPKIND__SPLIT,       0, 2,  s2x4,   s2x2,      0x005, 0};
        map_thing map_2x6_2x3           = {{0, 1, 2, 9, 10, 11,               3, 4, 5, 6, 7, 8},                     MPKIND__SPLIT,       0, 2,  s2x6,   s2x3,      0x000, 0};
        map_thing map_qtag_2x3          = {{0, 7, 5, -1, 6, -1,               -1, 2, -1, 4, 3, 1},                   MPKIND__SPLIT,       0, 2,  s_qtag, s2x3,      0x005, 0};
Private map_thing map_1x2_rmv           = {{0, 3,                             1, 2},                                 MPKIND__REMOVED,     0, 2,  s1x4,   s1x2,      0x000, 0};
Private map_thing map_1x2_rmvr          = {{0, 3,                             1, 2},                                 MPKIND__REMOVED,     0, 2,  s2x2,   s1x2,      0x005, 0};
Private map_thing map_2x2_rmv           = {{0, 2, 5, 7,                       1, 3, 4, 6},                           MPKIND__REMOVED,     0, 2,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_1x3_rmv           = {{0, 2, 4,                          1, 5, 3},                              MPKIND__REMOVED,     0, 2,  s1x6,   s1x3,      0x000, 0};
Private map_thing map_1x3_rmvr          = {{5, 4, 3,                          0, 1, 2},                              MPKIND__REMOVED,     0, 2,  s2x3,   s1x3,      0x000, 1};
Private map_thing map_1x4_rmv           = {{0, 3, 5, 6,                       1, 2, 4, 7},                           MPKIND__REMOVED,     0, 2,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_1x4_rmvr          = {{7, 6, 4, 5,                       0, 1, 3, 2},                           MPKIND__REMOVED,     0, 2,  s2x4,   s1x4,      0x000, 1};
Private map_thing map_1x6_rmv           = {{0, 2, 4, 7, 9, 11,                1, 3, 5, 6, 8, 10},                    MPKIND__REMOVED,     0, 2,  s1x12,  s1x6,      0x000, 0};
Private map_thing map_1x6_rmvr          = {{11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__REMOVED,     0, 2,  s2x6,   s1x6,      0x000, 1};
Private map_thing map_2x4_rmv           = {{0, 2, 4, 6, 9, 11, 13, 15,        1, 3, 5, 7, 8, 10, 12, 14},            MPKIND__REMOVED,     0, 2,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_2x4_rmvr          = {{10, 15, 3, 1, 4, 5, 6, 8,         12, 13, 14, 0, 2, 7, 11, 9},           MPKIND__REMOVED,     0, 2,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_2x3_rmv           = {{0, 2, 4, 7, 9, 11,                1, 3, 5, 6, 8, 10},                    MPKIND__REMOVED,     0, 2,  s2x6,   s2x3,      0x000, 0};
        map_thing map_2x3_rmvr          = {{2, 5, 7, 9, 10, 0,                3, 4, 6, 8, 11, 1},                    MPKIND__REMOVED,     0, 2,  s3x4,   s2x3,      0x005, 0};
Private map_thing map_dmd_rmv           = {{6, 0, 3, 5,                       7, 1, 2, 4},                           MPKIND__REMOVED,     0, 2,  s_rigger, sdmd,    0x000, 0};
Private map_thing map_dmd_rmvr          = {{0, 3, 5, 6,                       1, 2, 4, 7},                           MPKIND__REMOVED,     0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_dbloff1           = {{0, 1, 3, 2, 4, 5, 7, 6},                                                 MPKIND__NONE,        0, 1,  s2x4,   s_qtag,    0x000, 0};
        map_thing map_dbloff2           = {{2, 3, 4, 5, 6, 7, 0, 1},                                                 MPKIND__NONE,        0, 1,  s2x4,   s_qtag,    0x000, 0};
        map_thing map_dhrgl1            = {{0, 1, 9, 6, 7, 3},                                                       MPKIND__NONE,        0, 1,  sbighrgl,s1x6,     0x000, 0};
        map_thing map_dhrgl2            = {{11, 10, 9, 5, 4, 3},                                                     MPKIND__NONE,        0, 1,  sbighrgl,s1x6,     0x000, 0};
Private map_thing map_ov_1x4_1          = {{0, 1, 5, 2,                     2, 5, 3, 4},                             MPKIND__OVERLAP,     0, 2,  s1x6,   s1x4,      0x000, 0};
Private map_thing map_ov_1x4_2          = {{0, 1, 2, 3,            3, 2, 7, 6,            6, 7, 4, 5},               MPKIND__OVERLAP,     0, 3,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_ov_1x4_3          = {{9, 8, 6, 7,            10, 11, 4, 5,          0, 1, 3, 2},               MPKIND__OVERLAP,     3, 3,  s3x4,   s1x4,      0x000, 0};
Private map_thing map_ov_s2x2_1         = {{0, 1, 4, 5,                     1, 2, 3, 4},                             MPKIND__OVERLAP,     0, 2,  s2x3,   s2x2,      0x000, 0};
Private map_thing map_ov_s2x2r_1        = {{1, 4, 5, 0,                     2, 3, 4, 1},                             MPKIND__OVERLAP,     0, 2,  s2x3,   s2x2,      0x005, 0};
Private map_thing map_ov_s2x2_2         = {{0, 1, 6, 7,            1, 2, 5, 6,            2, 3, 4, 5},               MPKIND__OVERLAP,     0, 3,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_ov_s2x2_3         = {{0, 1, 8, 9,       1, 2, 7, 8,       2, 3, 6, 7,      3, 4, 5, 6},        MPKIND__OVERLAP,     0, 4,  s2x5,   s2x2,      0x000, 0};
Private map_thing map_ov_2x3_3          = {{1, 11, 8, 9, 10, 0,    2, 5, 7, 8, 11, 1,     3, 4, 6, 7, 5, 2},         MPKIND__OVERLAP,     0, 3,  s3x4,   s2x3,      0x015, 0};
Private map_thing map_ov_2x4_0          = {{0, 1, 2, 3, 8, 9, 10, 11,         2, 3, 4, 5, 6, 7, 8, 9},               MPKIND__OVERLAP,     0, 2,  s2x6,   s2x4,      0x000, 0};
Private map_thing map_ov_2x4_1          = {{10, 11, 5, 4, 6, 7, 8, 9,         0, 1, 2, 3, 4, 5, 11, 10},             MPKIND__OVERLAP,     0, 2,  s3x4,   s2x4,      0x000, 1};
Private map_thing map_ov_2x4_2          = {{0, 1, 2, 3, 12, 13, 14, 15,       2, 3, 4, 5, 10, 11, 12, 13,
                                                                    4, 5, 6, 7, 8, 9, 10, 11},                       MPKIND__OVERLAP,     0, 3,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_ov_2x4_3          = {{9, 11, 7, 2, 4, 5, 6, 8,          10, 15, 3, 1, 2, 7, 11, 9,
                                                                      12, 13, 14, 0, 1, 3, 15, 10},                  MPKIND__OVERLAP,     0, 3,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_ov_2x8_1          = {{20, 21, 22, 23, 11, 10, 9, 8, 12, 13, 14, 15, 16, 17, 18, 19,
                                                  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 23, 22, 21, 20},             MPKIND__OVERLAP,     0, 2,  s3x8,   s2x8,      0x000, 1};
Private map_thing map_ov_s1x8_0         = {{0, 1, 3, 2, 10, 11, 4, 5,         4, 5, 10, 11, 6, 7, 9, 8},             MPKIND__OVERLAP,     0, 2,  s1x12,  s1x8,      0x000, 0};
Private map_thing map_ov_s1x8_1         = {{0, 1, 3, 2, 7, 6, 4, 5,           4, 5, 7, 6, 12, 13, 15, 14,
                                                                   15, 14, 12, 13, 8, 9, 11, 10},                    MPKIND__OVERLAP,     0, 3,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_ov_qtag_0         = {{0, 1, 5, 11, 7, 8, 9, 10,         1, 2, 3, 4, 6, 7, 11, 5},              MPKIND__OVERLAP,     0, 2,  s3dmd,  s_qtag,    0x000, 0};
Private map_thing map_ov_qtag_2         = {{0, 1, 15, 14, 10, 11, 12, 13,     1, 2, 6, 7, 9, 10, 14, 15,
                                                                   2, 3, 4, 5, 8, 9, 7, 6},                          MPKIND__OVERLAP,     0, 3,  s4dmd,  s_qtag,    0x000, 0};

/* Maps for turning triangles into boxes for the "triangle" concept. */

        map_thing map_trngl_box1        = {{1, 2, -1, 0},                                                            MPKIND__NONE,        1, 1,  s_trngl, s2x2,     0x000, 0};
        map_thing map_trngl_box2        = {{1, 2, 0, -1},                                                            MPKIND__NONE,        1, 1,  s_trngl, s2x2,     0x000, 0};

        map_thing map_inner_box         = {{2, 3, 4, 5, 10, 11, 12, 13},                                             MPKIND__NONE,        0, 1,  s2x8,   s2x4,      0x000, 0};

Private map_thing map_lh_s2x3_a         = {{15, 16, 17, 12, 13, 14,          3, 4, 5, 6, 7, 8},                      MPKIND__OFFS_L_HALF, 0, 2,  s3x6,   s2x3,      0x000, 0};
Private map_thing map_lh_s2x4_2         = {{20, 21, 22, 23, 16, 17, 18, 19,   4, 5, 6, 7, 8, 9, 10, 11},             MPKIND__OFFS_L_HALF, 0, 2,  s3x8,   s2x4,      0x000, 0};
Private map_thing map_lh_s2x4_3         = {{20, 21, 22, 23, 12, 13, 14, 15,   0, 1, 2, 3, 8, 9, 10, 11},             MPKIND__OFFS_L_HALF, 0, 2,  s4x6,   s2x4,      0x000, 1};
Private map_thing map_lh_s1x4_2         = {{15, 14, 12, 13,                   4, 5, 7, 6},                           MPKIND__OFFS_L_HALF, 0, 2,  s1p5x8, s1x4,      0x000, 0};
Private map_thing map_lh_s1x4_3         = {{9, 8, 6, 7,                       0, 1, 3, 2},                           MPKIND__OFFS_L_HALF, 0, 2,  s2x6,   s1x4,      0x000, 1};
Private map_thing map_lh_s2x2_2         = {{10, 11, 8, 9,                     2, 3, 4, 5},                           MPKIND__OFFS_L_HALF, 0, 2,  s3x4,   s2x2,      0x000, 0};
Private map_thing map_lh_s2x2_3         = {{11, 8, 9, 10,                     3, 4, 5, 2},                           MPKIND__OFFS_L_HALF, 0, 2,  s3x4,   s2x2,      0x005, 0};

Private map_thing map_rh_bigd           = {{6, 7, 8, 9,                       0, 1, 2, 3},                           MPKIND__OFFS_L_HALF, 0, 2,  sbigdmd,s_trngl4,  0x107, 1};
        map_thing map_lh_c1phana        = {{0, -1, 1, -1, 2, -1, 3, -1, 6, -1, 7, -1, 8, -1, 9, -1},                 MPKIND__OFFS_L_HALF, 0, 1,  sbigdmd, s_c1phan, 0x000, 0};
        map_thing map_lh_c1phanb        = {{-1, 12, -1, 10, -1, 3, -1, 15, -1, 4, -1, 2, -1, 11, -1, 7},             MPKIND__OFFS_L_HALF, 4, 1,  s4x4,   s_c1phan,  0x000, 0};

Private map_thing map_lh_s2x4_0         = {{0, 1, 2, 3, 6, 7, 8, 9},                                                 MPKIND__OFFS_L_HALF, 0, 1,  s2x6,   s2x4,      0x000, 0};
Private map_thing map_lh_s2x4_1         = {{10, 11, 2, 3, 4, 5, 8, 9},                                               MPKIND__OFFS_L_HALF, 0, 1,  s3x4,   s2x4,      0x000, 1};
Private map_thing map_lh_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_L_HALF, 2, 1,  s1x8,   s1x8,      0x000, 0};
        map_thing map_lh_s2x3_3         = {{1, 2, 4, 5, 7, 3,                 13, 15, 11, 9, 10, 12},                MPKIND__OFFS_L_HALF, 0, 2,  s4x4,   s2x3,      0x005, 1};
        map_thing map_lh_s2x3_2         = {{9, 11, 7, 5, 6, 8,                13, 14, 0, 1, 3, 15},                  MPKIND__OFFS_L_HALF, 0, 2,  s4x4,   s2x3,      0x000, 0};
Private map_thing map_rh_s2x3_a         = {{0, 1, 2, 17, 16, 15,              8, 7, 6, 9, 10, 11},                   MPKIND__OFFS_R_HALF, 0, 2,  s3x6,   s2x3,      0x000, 0};
Private map_thing map_rh_s2x4_2         = {{0, 1, 2, 3, 23, 22, 21, 20,       11, 10, 9, 8, 12, 13, 14, 15},         MPKIND__OFFS_R_HALF, 0, 2,  s3x8,   s2x4,      0x000, 0};
Private map_thing map_rh_s2x4_3         = {{18, 19, 20, 21, 14, 15, 16, 17,   2, 3, 4, 5, 6, 7, 8, 9},               MPKIND__OFFS_R_HALF, 0, 2,  s4x6,   s2x4,      0x000, 1};
Private map_thing map_rh_s1x4_2         = {{0, 1, 3, 2,                       11, 10, 8, 9},                         MPKIND__OFFS_R_HALF, 0, 2,  s1p5x8, s1x4,      0x000, 0};
Private map_thing map_rh_s1x4_3         = {{11, 10, 8, 9,                     2, 3, 5, 4},                           MPKIND__OFFS_R_HALF, 0, 2,  s2x6,   s1x4,      0x000, 1};
Private map_thing map_rh_s2x2_2         = {{0, 1, 11, 10,                     5, 4, 6, 7},                           MPKIND__OFFS_R_HALF, 0, 2,  s3x4,   s2x2,      0x000, 0};
Private map_thing map_rh_s2x2_3         = {{1, 11, 10, 0,                     4, 6, 7, 5},                           MPKIND__OFFS_R_HALF, 0, 2,  s3x4,   s2x2,      0x005, 0};

Private map_thing map_lh_bigd           = {{11, 10, 9, 8,                     5, 4, 3, 2},                           MPKIND__OFFS_R_HALF, 0, 2,  sbigdmd,s_trngl4,  0x10D, 1};
        map_thing map_rh_c1phana        = {{-1, 2, -1, 3, -1, 5, -1, 4, -1, 8, -1, 9, -1, 11, -1, 10},               MPKIND__OFFS_R_HALF, 0, 1,  sbigdmd, s_c1phan, 0x000, 0};
        map_thing map_rh_c1phanb        = {{15, -1, 3, -1, 0, -1, 1, -1, 7, -1, 11, -1, 8, -1, 9, -1},               MPKIND__OFFS_R_HALF, 4, 1,  s4x4,   s_c1phan,  0x000, 0};

Private map_thing map_rh_s2x4_0         = {{2, 3, 4, 5, 8, 9, 10, 11},                                               MPKIND__OFFS_R_HALF, 0, 1,  s2x6,   s2x4,      0x000, 0};
Private map_thing map_rh_s2x4_1         = {{0, 1, 5, 4, 6, 7, 11, 10},                                               MPKIND__OFFS_R_HALF, 0, 1,  s3x4,   s2x4,      0x000, 1};
Private map_thing map_rh_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_R_HALF, 2, 1,  s1x8,   s1x8,      0x000, 0};
        map_thing map_rh_s2x3_3         = {{15, 11, 6, 8, 9, 10,              0, 1, 2, 7, 3, 14},                    MPKIND__OFFS_R_HALF, 0, 2,  s4x4,   s2x3,      0x005, 1};
        map_thing map_rh_s2x3_2         = {{12, 13, 14, 3, 15, 10,            11, 7, 2, 4, 5, 6},                    MPKIND__OFFS_R_HALF, 0, 2,  s4x4,   s2x3,      0x000, 0};

Private map_thing map_lf_s1x4_2         = {{15, 14, 12, 13,                   4, 5, 7, 6},                           MPKIND__OFFS_L_FULL, 0, 2,  s2x8,   s1x4,      0x000, 0};
Private map_thing map_lf_s1x4_3         = {{11, 10, 8, 9,                     0, 1, 3, 2},                           MPKIND__OFFS_L_FULL, 0, 2,  s2x8,   s1x4,      0x000, 1};
Private map_thing map_lf_s2x2_2         = {{9, 11, 6, 8,                      14, 0, 1, 3},                          MPKIND__OFFS_L_FULL, 0, 2,  s4x4,   s2x2,      0x000, 0};
Private map_thing map_lf_s2x2_3         = {{11, 6, 8, 9,                      0, 1, 3, 14},                          MPKIND__OFFS_L_FULL, 0, 2,  s4x4,   s2x2,      0x005, 0};
Private map_thing map_lf_s2x4_0         = {{0, 1, 2, 3, 8, 9, 10, 11},                                               MPKIND__OFFS_L_FULL, 0, 1,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_lf_s2x4_1         = {{13, 15, 2, 4, 5, 7, 10, 12},                                             MPKIND__OFFS_L_FULL, 0, 1,  s4x4,   s2x4,      0x001, 0};
Private map_thing map_lf_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_L_FULL, 2, 1,  s1x8,   s1x8,      0x000, 0};

Private map_thing map_rf_s1x4_2         = {{0, 1, 3, 2,                       11, 10, 8, 9},                         MPKIND__OFFS_R_FULL, 0, 2,  s2x8,   s1x4,      0x000, 0};
Private map_thing map_rf_s1x4_3         = {{15, 14, 12, 13,                   4, 5, 7, 6},                           MPKIND__OFFS_R_FULL, 0, 2,  s2x8,   s1x4,      0x000, 1};
Private map_thing map_rf_s2x2_2         = {{12, 13, 15, 10,                   7, 2, 4, 5},                           MPKIND__OFFS_R_FULL, 0, 2,  s4x4,   s2x2,      0x000, 0};
Private map_thing map_rf_s2x2_3         = {{13, 15, 10, 12,                   2, 4, 5, 7},                           MPKIND__OFFS_R_FULL, 0, 2,  s4x4,   s2x2,      0x005, 0};
Private map_thing map_rf_s2x4_0         = {{4, 5, 6, 7, 12, 13, 14, 15},                                             MPKIND__OFFS_R_FULL, 0, 1,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_rf_s2x4_1         = {{0, 1, 11, 6, 8, 9, 3, 14},                                               MPKIND__OFFS_R_FULL, 0, 1,  s4x4,   s2x4,      0x001, 0};
Private map_thing map_rf_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_R_FULL, 2, 1,  s1x8,   s1x8,      0x000, 0};

Private map_thing map_blob_1x4a         = {{13, 10, 6, 8,          15, 17, 3, 5,          18, 20, 1, 22},            MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x000, 1};
Private map_thing map_blob_1x4b         = {{19, 16, 12, 14,        21, 23, 9, 11,         0, 2, 7, 4},               MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x000, 1};
Private map_thing map_blob_1x4c         = {{1, 22, 18, 20,         3, 5, 15, 17,          6, 8, 13, 10},             MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x015, 0};
Private map_thing map_blob_1x4d         = {{19, 16, 12, 14,        21, 23, 9, 11,         0, 2, 7, 4},               MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x015, 0};
Private map_thing map_wblob_1x4a        = {{15, 17, 5, 3, 6, 8, 10, 13,       18, 20, 22, 1, 3, 5, 17, 15},          MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x000, 1};
Private map_thing map_wblob_1x4b        = {{21, 23, 11, 9, 12, 14, 16, 19,    0, 2, 4, 7, 9, 11, 23, 21},            MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x000, 1};
Private map_thing map_wblob_1x4c        = {{3, 5, 17, 15, 18, 20, 22, 1,      6, 8, 10, 13, 15, 17, 5, 3},           MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x005, 0};
Private map_thing map_wblob_1x4d        = {{21, 23, 11, 9, 12, 14, 16, 19,    0, 2, 4, 7, 9, 11, 23, 21},            MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x005, 0};



        map_thing map_lz12     = {{10, 9,                1, 11,                5, 7,                 3, 4},          MPKIND__NONE,        0, 4,  s3x4,   s1x2,      0x055, 0};
        map_thing map_rz12     = {{0, 10,                11, 8,                2, 5,                 4, 6},          MPKIND__NONE,        0, 4,  s3x4,   s1x2,      0x055, 0};

        map_thing map_dmd_1x1  = {{0,                    1,                    2,                  3},               MPKIND__NONE,        0, 4,  sdmd,        s1x1, 0x000, 0};
        map_thing map_star_1x1 = {{0,                    1,                    2,                  3},               MPKIND__NONE,        0, 4,  s_star,      s1x1, 0x000, 0};

        map_thing map_qtag_f0           = {{7, 3,                           0, 0},                                   MPKIND__NONE,        0, 1,  s_qtag,      s1x2, 0x000, 0};
        map_thing map_qtag_f1           = {{1, 3,                           7, 5},                                   MPKIND__NONE,        0, 2,  s_qtag,      s1x2, 0x005, 0};
        map_thing map_qtag_f2           = {{3, 4,                           0, 7},                                   MPKIND__NONE,        0, 2,  s_qtag,      s1x2, 0x005, 0};

        map_thing map_diag2a            = {{5, 7, 21, 15, 17, 19, 9, 3},                                             MPKIND__NONE,        0, 1,  s4x6,        s2x4,      1, 0};
        map_thing map_diag2b            = {{2, 8, 22, 12, 14, 20, 10, 0},                                            MPKIND__NONE,        0, 1,  s4x6,        s2x4,      1, 0};
        map_thing map_diag23a           = {{13, 3, 2, 5, 11, 10},                                                    MPKIND__NONE,        0, 1,  s4x4,        s2x3,      0, 0};
        map_thing map_diag23b           = {{13, 3, 2, 5, 11, 10},                                                    MPKIND__NONE,        0, 1,  s4x4,        s2x3,      1, 0};
        map_thing map_diag23c           = {{9, 15, 14, 1, 7, 6},                                                     MPKIND__NONE,        0, 1,  s4x4,        s2x3,      0, 0};
        map_thing map_diag23d           = {{1, 7, 6, 9, 15, 14},                                                     MPKIND__NONE,        0, 1,  s4x4,        s2x3,      1, 0};


        map_thing map_f2x8_4x4          = {{-1, 3, 12, 2, -1, -1, -1, 13, -1, 15, 0, 14, -1, -1, -1, 1,
                                            -1, 7,  8, 6, -1, -1, -1,  9, -1, 11, 4, 10, -1, -1, -1, 5},             MPKIND__SPLIT,       0, 2,  sfat2x8,  s4x4,    0x000, 0};
        map_thing map_w4x4_4x4          = {{13, 15, 11, 10, 6, 8, -1, 9, -1, -1, -1, -1, -1, -1, 12, -1,
                                            -1, -1, -1, -1, -1, -1, 4, -1, 5, 7, 3, 2, 14, 0, -1, 1},                MPKIND__SPLIT,       0, 2,  swide4x4, s4x4,    0x000, 0};
        map_thing map_f2x8_2x8          = {{15, 14, 13, 12, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1
                                            -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, 3, 2, 1, 0},                 MPKIND__SPLIT,       0, 2,  sfat2x8,  s2x8,    0x000, 1};
        map_thing map_w4x4_2x8          = {{-1, -1, 9, 11, 7, 2, -1, -1, -1, -1,  4, 5, 6, 8, -1, -1,
                                            -1, -1, 12, 13, 14, 0, -1, -1, -1, -1, 1, 3, 15, 10, -1, -1},            MPKIND__SPLIT,       0, 2,  swide4x4, s2x8,    0x000, 1};

Private map_thing map_4quads1  = {{0, 2,                 4, 6,                 10, 8,              14, 12},          MPKIND__4_QUADRANTS, 0, 4,  s_c1phan,    s1x2, 0x044, 0};
Private map_thing map_4quads2  = {{1, 3,                 7, 5,                 11, 9,              13, 15},          MPKIND__4_QUADRANTS, 0, 4,  s_c1phan,    s1x2, 0x011, 0};
Private map_thing map_4edges1  = {{13, 14,               1, 2,                 6, 5,               10, 9},           MPKIND__4_EDGES,     0, 4,  s4x4,        s1x2, 0x044, 0};
Private map_thing map_4edges2  = {{2, 3,                 5, 4,                 7, 6,               0, 1},            MPKIND__4_EDGES,     0, 4,  s_thar,      s1x2, 0x011, 0};

Private map_thing map_dblbentcw  = {{0, 1, 5, 4, 6, 7, 11, 10},                                                      MPKIND_DBLBENTCW,    0, 1,  sbigh,       s1x8, 0x000, 0};
Private map_thing map_dblbentccw = {{3, 2, 5, 4, 9, 8, 11, 10},                                                      MPKIND_DBLBENTCCW,   0, 1,  sbigh,       s1x8, 0x000, 0};

Private map_thing map_dblbent2x4cw  = {{1, 9, 8, 12, 13, 21, 20, 0},                                                 MPKIND_DBLBENTCW,    0, 1,  s4x6,        s2x4, 0x000, 0};
Private map_thing map_dblbent2x4ccw = {{17, 9, 8, 4, 5, 21, 20, 16},                                                 MPKIND_DBLBENTCCW,   0, 1,  s4x6,        s2x4, 0x000, 0};

Private map_thing map_all_8_1  = {{0, 1, 4, 5,           2, 3, 6, 7},                                                MPKIND__ALL_8,       0, 2,  s_thar,      s1x4, 0x004, 0};
Private map_thing map_all_8_2  = {{2, 3, 6, 7,           0, 1, 4, 5},                                                MPKIND__ALL_8,       0, 2,  s_thar,      s1x4, 0x001, 0};
Private map_thing map_all_8_d1 = {{0, 3, 4, 7,           2, 5, 6, 1},                                                MPKIND__ALL_8,       0, 2,  s_thar,      sdmd, 0x004, 0};
Private map_thing map_all_8_d2 = {{2, 5, 6, 1,           0, 3, 4, 7},                                                MPKIND__ALL_8,       0, 2,  s_thar,      sdmd, 0x001, 0};

Private map_thing map_all_8_b1 = {{13, 14, 5, 6,         1, 2, 9, 10},                                               MPKIND__ALL_8,       0, 2,  s4x4,       s2x2, 0x004, 0};
Private map_thing map_all_8_b2 = {{1, 2, 9, 10,          13, 14, 5, 6},                                              MPKIND__ALL_8,       0, 2,  s4x4,       s2x2, 0x001, 0};

Private map_thing map_dmd1     = {{0, 2,                 1, 3},                                                      MPKIND__DMD_STUFF,   0, 2,  sdmd,        s1x2, 0x004, 0};
Private map_thing map_dmd2     = {{1, 3,                 0, 2},                                                      MPKIND__DMD_STUFF,   0, 2,  sdmd,        s1x2, 0x001, 0};

Private map_thing mapovdmd1    = {{0, 3, 4, 7, 2, 5, 6, 1},                                                          MPKIND__DMD_STUFF,   0, 2,  s_thar,      sdmd, 0x104, 0};
Private map_thing mapovdmd2    = {{0, 3, 4, 7, 2, 5, 6, 1},                                                          MPKIND__DMD_STUFF,   0, 2,  s_thar,      sdmd, 0x004, 0};
Private map_thing mapovline1   = {{0, 1, 4, 5, 2, 3, 6, 7},                                                          MPKIND__DMD_STUFF,   0, 2,  s_thar,      s1x4, 0x104, 0};
Private map_thing mapovline2   = {{0, 1, 4, 5, 2, 3, 6, 7},                                                          MPKIND__DMD_STUFF,   0, 2,  s_thar,      s1x4, 0x004, 0};
Private map_thing mapovbox1    = {{0, 1, 2, 3, 1, 2, 3, 0},                                                          MPKIND__DMD_STUFF,   0, 2,  s2x2,        s2x2, 0x104, 0};

        map_thing *maps_3diag[4] = {&map_blob_1x4c, &map_blob_1x4a, &map_blob_1x4d, &map_blob_1x4b};
        map_thing *maps_3diagwk[4] = {&map_wblob_1x4a, &map_wblob_1x4c, &map_wblob_1x4b, &map_wblob_1x4d};


mapcoder map_init_table[] = {
   {MAPCODE(s1x8,1,MPKIND_DBLBENTCW,    0), &map_dblbentcw},
   {MAPCODE(s1x8,1,MPKIND_DBLBENTCCW,   0), &map_dblbentccw},
   {MAPCODE(s2x4,1,MPKIND_DBLBENTCW,    0), &map_dblbent2x4cw},
   {MAPCODE(s2x4,1,MPKIND_DBLBENTCCW,   0), &map_dblbent2x4ccw},
   {MAPCODE(s2x4,2,MPKIND__STAG,        0), &map_staggerv},
   {MAPCODE(sdmd,2,MPKIND__DMD_STUFF,   0), &mapovdmd1},
   {MAPCODE(sdmd,2,MPKIND__DMD_STUFF,   1), &mapovdmd2},
   {MAPCODE(s1x2,2,MPKIND__DMD_STUFF,   0), &map_dmd1},
   {MAPCODE(s1x2,2,MPKIND__DMD_STUFF,   1), &map_dmd2},
   {MAPCODE(s2x2,2,MPKIND__DMD_STUFF,   0), &mapovbox1},
   {MAPCODE(s1x4,2,MPKIND__DMD_STUFF,   0), &mapovline1},
   {MAPCODE(s1x4,2,MPKIND__DMD_STUFF,   1), &mapovline2},
   {MAPCODE(s2x2,2,MPKIND__ALL_8,       0), &map_all_8_b1},
   {MAPCODE(s2x2,2,MPKIND__ALL_8,       1), &map_all_8_b2},
   {MAPCODE(sdmd,2,MPKIND__ALL_8,       0), &map_all_8_d1},
   {MAPCODE(sdmd,2,MPKIND__ALL_8,       1), &map_all_8_d2},
   {MAPCODE(s1x4,2,MPKIND__ALL_8,       0), &map_all_8_1},
   {MAPCODE(s1x4,2,MPKIND__ALL_8,       1), &map_all_8_2},
   {MAPCODE(s1x2,4,MPKIND__4_EDGES,     0), &map_4edges1},
   {MAPCODE(s1x2,4,MPKIND__4_EDGES,     1), &map_4edges2},
   {MAPCODE(s1x2,4,MPKIND__4_QUADRANTS, 0), &map_4quads1},
   {MAPCODE(s1x2,4,MPKIND__4_QUADRANTS, 1), &map_4quads2},
   {MAPCODE(s2x4,2,MPKIND__X_SPOTS,     0), &map_x_s2x4_3},
   {MAPCODE(s2x4,2,MPKIND__X_SPOTS,     1), &map_x_s2x4_2},
   {MAPCODE(s2x4,2,MPKIND__O_SPOTS,     0), &map_o_s2x4_3},
   {MAPCODE(s2x4,2,MPKIND__O_SPOTS,     1), &map_o_s2x4_2},

   {MAPCODE(s1x8,1,MPKIND__OFFS_R_FULL, 0), &map_rf_s1x8_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_R_FULL, 0), &map_rf_s2x4_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_R_FULL, 1), &map_rf_s2x4_1},
   {MAPCODE(s2x2,2,MPKIND__OFFS_R_FULL, 0), &map_rf_s2x2_2},
   {MAPCODE(s2x2,2,MPKIND__OFFS_R_FULL, 1), &map_rf_s2x2_3},
   {MAPCODE(s1x4,2,MPKIND__OFFS_R_FULL, 0), &map_rf_s1x4_2},
   {MAPCODE(s1x4,2,MPKIND__OFFS_R_FULL, 1), &map_rf_s1x4_3},
   {MAPCODE(s1x8,1,MPKIND__OFFS_L_FULL, 0), &map_lf_s1x8_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_L_FULL, 0), &map_lf_s2x4_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_L_FULL, 1), &map_lf_s2x4_1},
   {MAPCODE(s2x2,2,MPKIND__OFFS_L_FULL, 0), &map_lf_s2x2_2},
   {MAPCODE(s2x2,2,MPKIND__OFFS_L_FULL, 1), &map_lf_s2x2_3},
   {MAPCODE(s1x4,2,MPKIND__OFFS_L_FULL, 0), &map_lf_s1x4_2},
   {MAPCODE(s1x4,2,MPKIND__OFFS_L_FULL, 1), &map_lf_s1x4_3},

   {MAPCODE(s1x8,1,MPKIND__OFFS_R_HALF, 0), &map_rh_s1x8_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_R_HALF, 0), &map_rh_s2x4_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_R_HALF, 1), &map_rh_s2x4_1},
   {MAPCODE(s2x2,2,MPKIND__OFFS_R_HALF, 0), &map_rh_s2x2_2},
   {MAPCODE(s2x2,2,MPKIND__OFFS_R_HALF, 1), &map_rh_s2x2_3},
   {MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF, 0), &map_rh_s1x4_2},
   {MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF, 1), &map_rh_s1x4_3},
   {MAPCODE(s_trngl4,2,MPKIND__OFFS_R_HALF, 1), &map_lh_bigd},
   {MAPCODE(s2x3,2,MPKIND__OFFS_R_HALF, 0), &map_rh_s2x3_a},
   {MAPCODE(s2x4,2,MPKIND__OFFS_R_HALF, 0), &map_rh_s2x4_2},
   {MAPCODE(s2x4,2,MPKIND__OFFS_R_HALF, 1), &map_rh_s2x4_3},

   {MAPCODE(s1x8,1,MPKIND__OFFS_L_HALF, 0), &map_lh_s1x8_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_L_HALF, 0), &map_lh_s2x4_0},
   {MAPCODE(s2x4,1,MPKIND__OFFS_L_HALF, 1), &map_lh_s2x4_1},
   {MAPCODE(s2x2,2,MPKIND__OFFS_L_HALF, 0), &map_lh_s2x2_2},
   {MAPCODE(s2x2,2,MPKIND__OFFS_L_HALF, 1), &map_lh_s2x2_3},
   {MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF, 0), &map_lh_s1x4_2},
   {MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF, 1), &map_lh_s1x4_3},
   {MAPCODE(s_trngl4,2,MPKIND__OFFS_L_HALF, 1), &map_rh_bigd},
   {MAPCODE(s2x3,2,MPKIND__OFFS_L_HALF, 0), &map_lh_s2x3_a},
   {MAPCODE(s2x4,2,MPKIND__OFFS_L_HALF, 0), &map_lh_s2x4_2},
   {MAPCODE(s2x4,2,MPKIND__OFFS_L_HALF, 1), &map_lh_s2x4_3},

   {MAPCODE(s_qtag,2,MPKIND__CONCPHAN,  0), &map_phantom_dmd},
   {MAPCODE(s1x8,2,MPKIND__CONCPHAN,    0), &map_conc_phan_grand},
   {MAPCODE(s2x4,2,MPKIND__CONCPHAN,    0), &map_phantom_box},
   {MAPCODE(s2x4,2,MPKIND__CONCPHAN,    1), &map_full_f},
   {MAPCODE(s2x2,3,MPKIND__CONCPHAN,    0), &map_conc_tb},
   {MAPCODE(s2x2,3,MPKIND__CONCPHAN,    1), &map_conc_tbr},
   {MAPCODE(s2x2,4,MPKIND__CONCPHAN,    0), &map_conc_qb},

   {MAPCODE(s2x4,2,MPKIND__INTLK,       0), &map_intlk_phantom_box},
   {MAPCODE(s2x4,2,MPKIND__INTLK,       1), &map_intlk_f},
   {MAPCODE(s1x8,2,MPKIND__INTLK,       0), &map_intlk_phan_grand},
   {MAPCODE(s_qtag,2,MPKIND__INTLK,     0), &map_intlk_phantom_dmd},

   {MAPCODE(s2x2,2,MPKIND__OVERLAP,     0), &map_ov_s2x2_1},
   {MAPCODE(s2x2,2,MPKIND__OVERLAP,     1), &map_ov_s2x2r_1},
   {MAPCODE(s3x4,2,MPKIND__OVERLAP,     0), &map_3x6_3x4},
   {MAPCODE(s_qtag,2,MPKIND__OVERLAP,   0), &map_ov_qtag_0},
   {MAPCODE(s2x8,2,MPKIND__OVERLAP,     1), &map_ov_2x8_1},
   {MAPCODE(s1x8,2,MPKIND__OVERLAP,     0), &map_ov_s1x8_0},
   {MAPCODE(s1x8,3,MPKIND__OVERLAP,     0), &map_ov_s1x8_1},
   {MAPCODE(s2x4,2,MPKIND__OVERLAP,     0), &map_ov_2x4_0},
   {MAPCODE(s2x4,2,MPKIND__OVERLAP,     1), &map_ov_2x4_1},
   {MAPCODE(s2x3,3,MPKIND__OVERLAP,     1), &map_ov_2x3_3},
   {MAPCODE(s2x2,3,MPKIND__OVERLAP,     0), &map_ov_s2x2_2},
   {MAPCODE(s_qtag,3,MPKIND__OVERLAP,   0), &map_ov_qtag_2},
   {MAPCODE(s1x4,2,MPKIND__OVERLAP,     0), &map_ov_1x4_1},
   {MAPCODE(s1x4,3,MPKIND__OVERLAP,     0), &map_ov_1x4_2},
   {MAPCODE(s1x4,3,MPKIND__OVERLAP,     1), &map_ov_1x4_3},
   {MAPCODE(s2x4,3,MPKIND__OVERLAP,     0), &map_ov_2x4_2},
   {MAPCODE(s2x4,3,MPKIND__OVERLAP,     1), &map_ov_2x4_3},
   {MAPCODE(s2x2,4,MPKIND__OVERLAP,     0), &map_ov_s2x2_3},

   {MAPCODE(s1x2,2,MPKIND__REMOVED,     0), &map_1x2_rmv},
   {MAPCODE(s1x2,2,MPKIND__REMOVED,     1), &map_1x2_rmvr},
   {MAPCODE(s2x2,2,MPKIND__REMOVED,     0), &map_2x2_rmv},
   {MAPCODE(s1x3,2,MPKIND__REMOVED,     0), &map_1x3_rmv},
   {MAPCODE(s1x3,2,MPKIND__REMOVED,     1), &map_1x3_rmvr},
   {MAPCODE(s1x4,2,MPKIND__REMOVED,     0), &map_1x4_rmv},
   {MAPCODE(s1x4,2,MPKIND__REMOVED,     1), &map_1x4_rmvr},
   {MAPCODE(sdmd,2,MPKIND__REMOVED,     0), &map_dmd_rmv},
   {MAPCODE(sdmd,2,MPKIND__REMOVED,     1), &map_dmd_rmvr},
   {MAPCODE(s1x6,2,MPKIND__REMOVED,     0), &map_1x6_rmv},
   {MAPCODE(s1x6,2,MPKIND__REMOVED,     1), &map_1x6_rmvr},
   {MAPCODE(s_trngl4,2,MPKIND__REMOVED, 1), &map_s8_tgl4},
   {MAPCODE(s2x3,2,MPKIND__REMOVED,     0), &map_2x3_rmv},
   {MAPCODE(s2x4,2,MPKIND__REMOVED,     0), &map_2x4_rmv},
   {MAPCODE(s2x4,2,MPKIND__REMOVED,     1), &map_2x4_rmvr},

   {~0, (Const map_thing *) 0}};


/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h. */
map_thing *split_lists[][6] = {
   {0,             0,             0,             0,             0,             0},          /* nothing */
   {&map_1x2_1x1,  &map_1x2_1x1,  0,             0,             0,             0},          /* s1x1 */
   {&map_1x4_1x2,  &map_2x2h,     &map_1x6_1x2,  &map_2x3_1x2,  0,             0},          /* s1x2 */
   {&map_1x6_1x3,  &map_2x3_1x3,  0,             0,             0,             0},          /* s1x3 */
   {&map_2x4_2x2,  &map_2x4_2x2r, &map_2x6_2x2,  &map_2x6_2x2r, &map_2x8_2x2,  0},          /* s2x2 */
   {&map_1x8_1x4,  &map_2x4_1x4,  &map_1x12_1x4, &map_3x4_1x4,  &map_1x16_1x4, &map_4x4_1x4},  /* s1x4 */
   {&map_ptp_dmd,  &map_qtg_dmd,  0,             &map_3dmd_dmd, 0,             &map_4dmd_dmd},  /* sdmd */
   {0,             0,             0,             0,             0,             0},          /* s_star */
   {0,             &map_s6_trngl, 0,             0,             0,             0},          /* s_trngl */
   {&map_phan_trngl4a, &map_rig_trngl4, 0,       0,             0,             0},          /* s_trngl4 */
   {0,             0,             0,             0,             0,             0},          /* s_bone6 */
   {0,             0,             0,             0,             0,             0},          /* s_short6 */
   {&map_1x12_1x6, &map_2x6_1x6,  0,  &map_3x6_1x6,             0,             0},          /* s1x6 */
   {&map_2x6_2x3,  &map_3x4_2x3,  0,  &map_3x6_2x3,             0,             0},          /* s2x3 */
   {&map_bone_12d, &map_bigd_12d, 0,             0,             0,             0},          /* s_1x2dmd */
   {0,             0,             0,             0,             0,             0},          /* s_2x1dmd */
   {&map_hv_qtg_2, &map_vv_qtg_2, 0,             0,             0,             0},          /* s_qtag */
   {0,             0,             0,             0,             0,             0},          /* s_bone */
   {&map_1x16_1x8, &map_2x8_1x8,  &map_3x8_1x8,  0,             0,             0},          /* s1x8 */
   {0,             0,             0,             0,             0,             0},          /* slittlestars */
   {0,             0,             0,             0,             0,             0},          /* s1x3dmd */
   {0,             0,             0,             0,             0,             0},          /* s3x1dmd */
   {0,             &map_spin_3x4, 0,             0,             0,             0},          /* s_spindle */
   {&map_hrgl_ptp, 0,             0,             0,             0,             0},          /* s_hrglass */
   {0,             0,             0,             0,             0,             0},          /* s_dhrglass */
   {0,             0,             0,             0,             0,             0},          /* s_hyperglass */
   {0,             0,             0,             0,             0,             0},          /* s_crosswave */
   {&map_hv_2x4_2, &map_split_f,  0,             &map_4x6_2x4,  0,             0},          /* s2x4 */
   {0,             0,             0,             0,             0,             0},          /* s2x5 */
   {0,             0,             0,             0,             0,             0},          /* s_rigger */
   {&map_3x8_3x4,  &map_4x6_3x4,  0,             0,             0,             0},          /* s3x4 */
   {0,             &map_4x6_2x6,  0,             0,             0,             0}};         /* s2x6 */
