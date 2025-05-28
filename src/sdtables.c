/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993, 1994  William B. Ackerman.

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

/* This defines the following external variables:
   getout_strings
   filename_strings
   menu_names
   setup_attrs
   setup_limits
   begin_sizes
   startinfolist
   map_b6_trngl
   map_s6_trngl
   map_bone_trngl4
   map_rig_trngl4
   map_s8_tgl4
   map_p8_tgl4
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
   map_hv_2x4_2
   map_3x4_2x3
   map_4x6_2x4
   map_hv_qtg_2
   map_vv_qtg_2
   map_tgl4_1
   map_tgl4_2
   map_2x6_2x3
   map_dbloff1
   map_dbloff2
   map_trngl_box1
   map_trngl_box2
   map_lh_s2x3_3
   map_lh_s2x3_2
   map_rh_s2x3_3
   map_rh_s2x3_2
   map_dmd_1x1
   map_star_1x1
   map_qtag_f0
   map_qtag_f1
   map_qtag_f2
   maps_3diag
   maps_3diagwk
   map_lists
*/

#include "sd.h"
extern map_thing map_tgl4_1;
extern map_thing map_tgl4_2;



/* These are used in setup_coords. */

Private coordrec thing1x1 = {s_1x1, 3,
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

Private coordrec thing1x2 = {s_1x2, 3,
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

Private coordrec thing1x3 = {s_1x3, 3,
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

Private coordrec thing1x6 = {s_1x6, 3,
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

Private coordrec thing2x3 = {s_2x3, 3,
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
   {  0,   0,   7,   3,   0,   0,  -7,  -3}, {
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

Private coordrec thingbone = {s_bone, 3,
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

Private coordrec thingqtag = {s_qtag, 3,       /* "fudged" coordinates */
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

Private coordrec nicethingqtag = {s_qtag, 3,   /* "true" coordinates */
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

Private coordrec thingptpd = {s_ptpd, 3,       /* "fudged" coordinates */
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

Private coordrec thing3x1dmd = {s_3x1dmd, 3,   /* used for both -- symmetric and safe for press/truck */
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

Private coordrec thing3dmd = {s_3dmd, 3,   /* used for both -- symmetric and safe for press/truck */
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

Private coordrec thing4dmd = {s_4dmd, 3,   /* used for both -- symmetric and safe for press/truck */
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

Private coordrec nicethingdmd = {sdmd, 3,          /* "true" coordinates -- there is no "fudged" version */
   { -4,   0,   4,   0},
   {  0,   4,   0,  -4}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  1, -1, -1, -1,
      -1, -1, -1,  0, -1,  2, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
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

Private coordrec nicethingphan = {s_c1phan, 3,   /* "true" coordinates */
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
   {-10,  -7,  -2,  -7,   6,   9,   6,   1,  11,   6,   3,   6,  -5, -10,  -5,  -2},
   {  6,   9,   6,   1,  11,   6,   3,   6,  -5, -10,  -5,  -2, -10,  -7,  -2,  -7}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  1, -1, -1,  4, -1, -1,
      -1,  0, -1,  2,  7, -1,  5, -1,
      -1, -1,  3, -1, -1,  6, -1, -1,
      -1, -1, 14, -1, -1, 11, -1, -1,
      -1, 13, -1, 15, 10, -1,  8, -1,
      -1, -1, 12, -1, -1,  9, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Private coordrec thingglass = {s_hrglass, 3,
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



/* BEWARE!!  This list is keyed to the definition of "level" in sd.h . */
char *getout_strings[] = {
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
   "all",
   ""};

/* BEWARE!!  This list is keyed to the definition of "level" in sd.h . */
char *filename_strings[] = {
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
   ".all",
   ""};

/* BEWARE!!  This list is keyed to the definition of "call_list_kind" in sd.h . */
char *menu_names[] = {
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



/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
int setup_limits[] = {
   -1,         /* nothing */
   0,          /* s_1x1 */
   1,          /* s_1x2 */
   2,          /* s_1x3 */
   3,          /* s2x2 */
   3,          /* sdmd */
   3,          /* s_star */
   2,          /* s_trngl */
   3,          /* s_trngl4 */
   5,          /* s_bone6 */
   5,          /* s_short6 */
   7,          /* s_qtag */
   7,          /* s_bone */
   7,          /* s_rigger */
   7,          /* s_spindle */
   7,          /* s_hrglass */
   11,         /* s_hyperglass */
   7,          /* s_crosswave */
   3,          /* s1x4 */
   7,          /* s1x8 */
   7,          /* s2x4 */
   5,          /* s_2x3 */
   5,          /* s_1x6 */
   11,         /* s3x4 */
   11,         /* s2x6 */
   15,         /* s2x8 */
   15,         /* s4x4 */
   11,         /* s_x1x6 */
   9,          /* s1x10 */
   11,         /* s1x12 */
   13,         /* s1x14 */
   15,         /* s1x16 */
   15,         /* s_c1phan */
   23,         /* s_bigblob */
   7,          /* s_ptpd */
   7,          /* s_3x1dmd */
   11,         /* s_3dmd */
   15,         /* s_4dmd */
   7,          /* s_wingedstar */
   11,         /* s_wingedstar12 */
   15,         /* s_wingedstar16 */
   7,          /* s_galaxy */
   23,         /* s4x6 */
   7,          /* s_thar */
   31,         /* s_x4dmd */   /* Danger!!  We can never store these people!! */
   63,         /* s_8x8 */     /* Danger!!  We can never store these people!! */
   -1};        /* s_normal_concentric */



/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
setup_attr setup_attrs[] = {
   {-1, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* nothing */
   { 0, &thing1x1,      &thing1x1,        {b_1x1,       b_1x1},        { 1, 1},  TRUE},   /* s_1x1 */
   { 1, &thing1x2,      &thing1x2,        {b_1x2,       b_2x1},        { 2, 1}, FALSE},   /* s_1x2 */
   { 2, &thing1x3,      &thing1x3,        {b_1x3,       b_3x1},        { 3, 1}, FALSE},   /* s_1x3 */
   { 3, &thing2x2,      &thing2x2,        {b_2x2,       b_2x2},        { 2, 2},  TRUE},   /* s2x2 */
   { 3, (coordrec *) 0, &nicethingdmd,    {b_dmd,       b_pmd},        { 0, 2}, FALSE},   /* sdmd */
   { 3, (coordrec *) 0, (coordrec *) 0,   {b_star,      b_star},       { 0, 0},  TRUE},   /* s_star */
   { 2, (coordrec *) 0, (coordrec *) 0,   {b_trngl,     b_ptrngl},     { 0, 0}, FALSE},   /* s_trngl */
   { 3, (coordrec *) 0, (coordrec *) 0,   {b_trngl4,    b_ptrngl4},    { 0, 0}, FALSE},   /* s_trngl4 */
   { 5, (coordrec *) 0, (coordrec *) 0,   {b_bone6,     b_pbone6},     { 0, 0}, FALSE},   /* s_bone6 */
   { 5, (coordrec *) 0, &nicethingshort6, {b_short6,    b_pshort6},    { 0, 0}, FALSE},   /* s_short6 */
   { 7, &thingqtag,     &nicethingqtag,   {b_qtag,      b_pqtag},      { 4, 0}, FALSE},   /* s_qtag */
   { 7, &thingbone,     &thingbone,       {b_bone,      b_pbone},      { 0, 0}, FALSE},   /* s_bone */
   { 7, &thingrigger,   &thingrigger,     {b_rigger,    b_prigger},    { 0, 0}, FALSE},   /* s_rigger */
   { 7, &thingspindle,  &thingspindle,    {b_spindle,   b_pspindle},   { 0, 0}, FALSE},   /* s_spindle */
   { 7, &thingglass,    &thingglass,      {b_hrglass,   b_phrglass},   { 0, 0}, FALSE},   /* s_hrglass */
   {11, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0},  TRUE},   /* s_hyperglass */
   { 7, &thingxwv,      &thingxwv,        {b_crosswave, b_pcrosswave}, { 0, 0}, FALSE},   /* s_crosswave */
   { 3, &thing1x4,      &thing1x4,        {b_1x4,       b_4x1},        { 4, 1}, FALSE},   /* s1x4 */
   { 7, &thing1x8,      &thing1x8,        {b_1x8,       b_8x1},        { 8, 1}, FALSE},   /* s1x8 */
   { 7, &thing2x4,      &thing2x4,        {b_2x4,       b_4x2},        { 4, 2}, FALSE},   /* s2x4 */
   { 5, &thing2x3,      &thing2x3,        {b_2x3,       b_3x2},        { 3, 2}, FALSE},   /* s_2x3 */
   { 5, &thing1x6,      &thing1x6,        {b_1x6,       b_6x1},        { 6, 1}, FALSE},   /* s_1x6 */
   {11, &thing3x4,      &thing3x4,        {b_3x4,       b_4x3},        { 4, 3}, FALSE},   /* s3x4 */
   {11, &thing2x6,      &thing2x6,        {b_2x6,       b_6x2},        { 6, 2}, FALSE},   /* s2x6 */
   {15, &thing2x8,      &thing2x8,        {b_2x8,       b_8x2},        { 8, 2}, FALSE},   /* s2x8 */
   {15, &thing4x4,      &thing4x4,        {b_4x4,       b_4x4},        { 4, 4},  TRUE},   /* s4x4 */
   {11, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* s_x1x6 */
   { 9, &thing1x10,     &thing1x10,       {b_1x10,      b_10x1},       {10, 1}, FALSE},   /* s_1x10 */
   {11, &thing1x12,     &thing1x12,       {b_1x12,      b_12x1},       {12, 1}, FALSE},   /* s_1x12 */
   {13, &thing1x14,     &thing1x14,       {b_1x14,      b_14x1},       {14, 1}, FALSE},   /* s_1x14 */
   {15, &thing1x16,     &thing1x16,       {b_1x16,      b_16x1},       {16, 1}, FALSE},   /* s_1x16 */
   {15, &thingphan,     &nicethingphan,   {b_c1phan,    b_c1phan},     { 0, 0},  TRUE},   /* s_c1phan */
   {23, &thingblob,     &thingblob,       {b_nothing,   b_nothing},    { 0, 0},  TRUE},   /* s_bigblob */
   { 7, &thingptpd,     &nicethingptpd,   {b_ptpd,      b_pptpd},      { 0, 0}, FALSE},   /* s_ptpd */
   { 7, &thing3x1dmd,   &thing3x1dmd,     {b_3x1dmd,    b_p3x1dmd},    { 0, 0}, FALSE},   /* s_3x1dmd */
   {11, &thing3dmd,     &thing3dmd,       {b_3dmd,      b_p3dmd},      { 0, 0}, FALSE},   /* s_3dmd */
   {15, &thing4dmd,     &thing4dmd,       {b_4dmd,      b_p4dmd},      { 0, 0}, FALSE},   /* s_4dmd */
   { 7, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* s_wingedstar */
   {11, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* s_wingedstar12 */
   {15, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* s_wingedstar16 */
   { 7, &thinggal,      &thinggal,        {b_galaxy,    b_galaxy},     { 0, 0},  TRUE},   /* s_galaxy */
   {23, &thing4x6,      &thing4x6,        {b_nothing,   b_nothing},    { 6, 4}, FALSE},   /* s4x6 */
   { 7, &thingthar,     &thingthar,       {b_thar,      b_thar},       { 0, 0},  TRUE},   /* s_thar */
   {31, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* s_x4dmd */
   {63, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE},   /* s_8x8 */
   {-1, (coordrec *) 0, (coordrec *) 0,   {b_nothing,   b_nothing},    { 0, 0}, FALSE}};  /* s_normal_concentric */




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
   6,          /* b_1x6 */
   6,          /* b_6x1 */
   12,         /* b_3x4 */
   12,         /* b_4x3 */
   12,         /* b_2x6 */
   12,         /* b_6x2 */
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
   24,         /* b_4x6 */
   24,         /* b_6x4 */
   8,          /* b_thar */
   8,          /* b_ptpd */
   8,          /* b_pptpd */
   8,          /* b_3x1dmd */
   8,          /* b_p3x1dmd */
   12,         /* b_3dmd */
   12,         /* b_p3dmd */
   16,         /* b_4dmd */
   16};        /* b_p4dmd */


#define B1A 0000
#define G1A 0100
#define B2A 0200
#define G2A 0300
#define B3A 0400
#define G3A 0500
#define B4A 0600
#define G4A 0700

#define B1B (ID2_NSG|ID2_NSB|ID2_NHG|ID2_HCOR|ID2_HEAD|ID2_BOY)
#define G1B (ID2_NSG|ID2_NSB|ID2_NHB|ID2_SCOR|ID2_HEAD|ID2_GIRL)
#define B2B (ID2_NSG|ID2_NHG|ID2_NHB|ID2_SCOR|ID2_SIDE|ID2_BOY)
#define G2B (ID2_NSB|ID2_NHG|ID2_NHB|ID2_HCOR|ID2_SIDE|ID2_GIRL)
#define B3B B1B
#define G3B G1B
#define B4B B2B
#define G4B G2B

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
         {{G2A|d_south,G2B}, {B2A|d_south,B2B}, {G1A|d_south,G1B}, {B1A|d_south,B1B},
            {G4A|d_north,G4B}, {B4A|d_north,B4B}, {G3A|d_north,G3B}, {B3A|d_north,B3B}},
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
         {{G3A|d_south,G3B}, {B3A|d_south,B3B}, {G2A|d_south,G2B}, {B2A|d_south,B2B},
            {G1A|d_north,G1B}, {B1A|d_north,B1B}, {G4A|d_north,G4B}, {B4A|d_north,B4B}},
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
         {{B4A|d_east,B4B}, {G3A|d_south,G3B}, {B3A|d_south,B3B}, {G2A|d_west,G2B},
            {B2A|d_west,B2B}, {G1A|d_north,G1B}, {B1A|d_north,B1B}, {G4A|d_east,G4B}},
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
         {{B3A|d_east,B3B}, {G2A|d_south,G2B}, {B2A|d_south,B2B}, {G1A|d_west,G1B},
            {B1A|d_west,B1B}, {G4A|d_north,G4B}, {B4A|d_north,B4B}, {G3A|d_east,G3B}},
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
         {{0,0}, {G2A|d_west,G2B}, {B2A|d_west,B2B}, {0,0}, {0,0}, {G1A|d_north,G1B},
            {B1A|d_north,B1B}, {0,0}, {0,0}, {G4A|d_east,G4B}, {B4A|d_east,B4B}, {0,0},
            {0,0}, {G3A|d_south,G3B}, {B3A|d_south,B3B}, {0,0}},
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
         {{B4A|d_south,B4B}, {G3A|d_west,G3B}, {B3A|d_east,B3B}, {G2A|d_south,G2B},
            {B2A|d_south,B2B}, {G1A|d_west,G1B}, {B1A|d_east,B1B}, {G4A|d_south,G4B}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   }
#endif
};




/*                                                                                                          setups are stacked on top of each other --------------------|
                                                                                        each setup is rotated clockwise before being concatenated -----------------|    |
                                                                                              (rot = 2 for second setup upside-down, 1-fold symmetry)              |    |
                                                                                                 (rot = 3 for 1st setup ccw, 2nd cw, 1-fold symmetry)              |    |
                                                                                                                                                                   V    V
                                             map1                             map2                        map3 map4   map_kind          arity  outer   inner      rot  vert */

        map_thing map_b6_trngl          = {{5, 4, 0},                      {2, 1, 3},                      {0}, {0}, MPKIND__SPLIT,       2,  s_bone6, s_trngl,  0x207, 0};
        map_thing map_s6_trngl          = {{4, 5, 3},                      {1, 2, 0},                      {0}, {0}, MPKIND__SPLIT,       2,  s_short6, s_trngl, 0x108, 1};
        map_thing map_bone_trngl4       = {{7, 6, 5, 0},                   {3, 2, 1, 4},                   {0}, {0}, MPKIND__SPLIT,       2,  s_bone, s_trngl4,  0x207, 0};
        map_thing map_rig_trngl4        = {{6, 7, 0, 5},                   {2, 3, 4, 1},                   {0}, {0}, MPKIND__SPLIT,       2,  s_rigger,s_trngl4, 0x10D, 0};
        map_thing map_s8_tgl4           = {{2, 7, 5, 0},                   {6, 3, 1, 4},                   {0}, {0}, MPKIND__REMOVED,     2,  s_bone, s_trngl4,  0x207, 0};
        map_thing map_p8_tgl4           = {{0, 2, 7, 5},                   {4, 6, 3, 1},                   {0}, {0}, MPKIND__REMOVED,     2,  s_ptpd, s_trngl4,  0x10D, 0};
        map_thing map_2x2v              = {{0, 3},                         {1, 2},                         {0}, {0}, MPKIND__SPLIT,       2,  s2x2,   s_1x2,     0x005, 0};
        map_thing map_2x4_magic         = {{0, 6, 3, 5},                   {7, 1, 4, 2},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s1x4,      0x000, 0};
        map_thing map_qtg_magic         = {{0, 2, 5, 3},                   {1, 7, 4, 6},                   {0}, {0}, MPKIND__NONE,        2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_qtg_intlk         = {{0, 3, 5, 6},                   {1, 2, 4, 7},                   {0}, {0}, MPKIND__NONE,        2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_qtg_magic_intlk   = {{0, 2, 5, 7},                   {1, 3, 4, 6},                   {0}, {0}, MPKIND__NONE,        2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_ptp_magic         = {{6, 1, 4, 3},                   {0, 7, 2, 5},                   {0}, {0}, MPKIND__NONE,        2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_ptp_intlk         = {{0, 1, 6, 3},                   {2, 7, 4, 5},                   {0}, {0}, MPKIND__NONE,        2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_ptp_magic_intlk   = {{2, 1, 4, 3},                   {0, 7, 6, 5},                   {0}, {0}, MPKIND__NONE,        2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_2x4_diagonal      = {{2, 3, 6, 7},                   {0, 1, 4, 5},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_2x4_int_pgram     = {{1, 3, 5, 7},                   {0, 2, 4, 6},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_2x4_trapezoid     = {{1, 2, 4, 7},                   {0, 3, 5, 6},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_3x4_2x3_intlk     = {{2, 5, 7, 9, 10, 0},            {3, 4, 6, 8, 11, 1},            {0}, {0}, MPKIND__NONE,        2,  s3x4,   s_2x3,     0x005, 0};
        map_thing map_3x4_2x3_conc      = {{3, 4, 6, 9, 10, 0},            {2, 5, 7, 8, 11, 1},            {0}, {0}, MPKIND__NONE,        2,  s3x4,   s_2x3,     0x005, 0};
        map_thing map_4x4_ns            = {{12, 13, 0, 14},                {8, 6, 4, 5},                   {0}, {0}, MPKIND__NONE,        2,  s4x4,   s1x4,      0x000, 1};
        map_thing map_4x4_ew            = {{0, 1, 4, 2},                   {12, 10, 8, 9},                 {0}, {0}, MPKIND__NONE,        2,  s4x4,   s1x4,      0x005, 1};
        map_thing map_phantom_box       = {{0, 1, 6, 7, 8, 9, 14, 15},     {2, 3, 4, 5, 10, 11, 12, 13},   {0}, {0}, MPKIND__CONCPHAN,    2,  s2x8,   s2x4,      0x000, 0};
        map_thing map_intlk_phantom_box = {{0, 1, 4, 5, 10, 11, 14, 15},   {2, 3, 6, 7, 8, 9, 12, 13},     {0}, {0}, MPKIND__INTLK,       2,  s2x8,   s2x4,      0x000, 0};
        map_thing map_phantom_dmd       = {{0, 3, 4, 5, 8, 11, 12, 13},    {1, 2, 6, 7, 9, 10, 14, 15},    {0}, {0}, MPKIND__CONCPHAN,    2,  s_4dmd, s_qtag,    0x000, 0};
        map_thing map_intlk_phantom_dmd = {{0, 2, 6, 7, 9, 11, 12, 13},    {1, 3, 4, 5, 8, 10, 14, 15},    {0}, {0}, MPKIND__INTLK,       2,  s_4dmd, s_qtag,    0x000, 0};
        map_thing map_stagger           = {{10, 13, 3, 0, 2, 5, 11, 8},    {12, 15, 14, 1, 4, 7, 6, 9},    {0}, {0}, MPKIND__STAG,        2,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_staggerv          = {{12, 15, 14, 1, 4, 7, 6, 9},    {10, 13, 3, 0, 2, 5, 11, 8},    {0}, {0}, MPKIND__STAG,        2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_stairst           = {{9, 13, 7, 0, 1, 5, 15, 8},     {12, 11, 14, 2, 4, 3, 6, 10},   {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_ladder            = {{10, 15, 14, 0, 2, 7, 6, 8},    {12, 13, 3, 1, 4, 5, 11, 9},    {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_offset            = {{9, 11, 14, 0, 1, 3, 6, 8},     {12, 13, 7, 2, 4, 5, 15, 10},   {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_but_o             = {{10, 13, 14, 1, 2, 5, 6, 9},    {12, 15, 3, 0, 4, 7, 11, 8},    {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_o_s2x4_3          = {{10, 13, 14, 1, 2, 5, 6, 9},    {12, 15, 3, 0, 4, 7, 11, 8},    {0}, {0}, MPKIND__O_SPOTS,     2,  s4x4,   s2x4,      0x000, 0};
        map_thing map_x_s2x4_3          = {{10, 13, 14, 1, 2, 5, 6, 9},    {12, 15, 3, 0, 4, 7, 11, 8},    {0}, {0}, MPKIND__X_SPOTS,     2,  s4x4,   s2x4,      0x000, 0};
Private map_thing map_o_s2x4_2          = {{14, 1, 2, 5, 6, 9, 10, 13},    {0, 3, 7, 4, 8, 11, 15, 12},    {0}, {0}, MPKIND__O_SPOTS,     2,  s4x4,   s2x4,      0x005, 0};
Private map_thing map_x_s2x4_2          = {{14, 1, 2, 5, 6, 9, 10, 13},    {0, 3, 7, 4, 8, 11, 15, 12},    {0}, {0}, MPKIND__X_SPOTS,     2,  s4x4,   s2x4,      0x005, 0};
        map_thing map_4x4v     = {{12, 10, 8, 9},       {13, 15, 6, 11},      {14, 3, 5, 7},      {0, 1, 4, 2},      MPKIND__SPLIT,       4,  s4x4,   s1x4,      0x055, 0};
        map_thing map_blocks   = {{12, 14, 7, 9},       {13, 0, 2, 11},       {15, 1, 4, 6},      {10, 3, 5, 8},     MPKIND__NONE,        4,  s4x4,   s2x2,      0x000, 0};
        map_thing map_trglbox  = {{12, 14, 15, 9},      {13, 0, 2, 3},        {7, 1, 4, 6},       {10, 11, 5, 8},    MPKIND__NONE,        4,  s4x4,   s2x2,      0x000, 0};
Private map_thing map_4x4_1x4  = {{8, 6, 4, 5},         {9, 11, 2, 7},        {10, 15, 1, 3},     {12, 13, 0, 14},   MPKIND__SPLIT,       4,  s4x4,   s1x4,      0x000, 1};
Private map_thing map_1x16_1x4 = {{0, 1, 3, 2},         {4, 5, 7, 6},         {15, 14, 12, 13},   {11, 10, 8, 9},    MPKIND__SPLIT,       4,  s1x16,  s1x4,      0x000, 0};
Private map_thing map_2x8_2x2  = {{0, 1, 14, 15},       {2, 3, 12, 13},       {4, 5, 10, 11},     {6, 7, 8, 9},      MPKIND__SPLIT,       4,  s2x8,   s2x2,      0x000, 0};
Private map_thing map_conc_qb  = {{0, 7, 8, 15},        {1, 6, 9, 14},        {2, 5, 10, 13},     {3, 4, 11, 12},    MPKIND__CONCPHAN,    4,  s2x8,   s2x2,      0x000, 0};
Private map_thing map_4dmd_dmd = {{0, 13, 11, 12},      {1, 15, 10, 14},      {2, 6, 9, 7},       {3, 4, 8, 5},      MPKIND__SPLIT,       4,  s_4dmd, sdmd,      0x055, 0};
Private map_thing map_2x6_1x6           = {{11, 10, 9, 6, 7, 8},           {0, 1, 2, 5, 4, 3},             {0}, {0}, MPKIND__SPLIT,       2,  s2x6,   s_1x6,     0x000, 1};
Private map_thing map_2x8_1x8           = {{15, 14, 12, 13, 8, 9, 11, 10}, {0, 1, 3, 2, 7, 6, 4, 5},       {0}, {0}, MPKIND__SPLIT,       2,  s2x8,   s1x8,      0x000, 1};
Private map_thing map_1x12_1x6          = {{0, 1, 2, 5, 4, 3},             {11, 10, 9, 6, 7, 8},           {0}, {0}, MPKIND__SPLIT,       2,  s1x12,  s_1x6,     0x000, 0};
Private map_thing map_1x16_1x8          = {{0, 1, 3, 2, 7, 6, 4, 5},       {15, 14, 12, 13, 8, 9, 11, 10}, {0}, {0}, MPKIND__SPLIT,       2,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_intlk_phan_grand  = {{0, 1, 3, 2, 12, 13, 15, 14},   {4, 5, 7, 6, 8, 9, 11, 10},     {0}, {0}, MPKIND__INTLK,       2,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_conc_phan_grand   = {{0, 1, 3, 2, 8, 9, 11, 10},     {4, 5, 7, 6, 12, 13, 15, 14},   {0}, {0}, MPKIND__CONCPHAN,    2,  s1x16,  s1x8,      0x000, 0};
        map_thing map_hv_2x4_2          = {{0, 1, 2, 3, 12, 13, 14, 15},   {4, 5, 6, 7, 8, 9, 10, 11},     {0}, {0}, MPKIND__SPLIT,       2,  s2x8,   s2x4,      0x000, 0};
        map_thing map_3x4_2x3           = {{1, 11, 8, 9, 10, 0},           {3, 4, 6, 7, 5, 2},             {0}, {0}, MPKIND__SPLIT,       2,  s3x4,   s_2x3,     0x005, 0};
        map_thing map_split_f           = {{9, 11, 7, 2, 4, 5, 6, 8},      {12, 13, 14, 0, 1, 3, 15, 10},  {0}, {0}, MPKIND__SPLIT,       2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_intlk_f           = {{10, 15, 3, 1, 4, 5, 6, 8},     {12, 13, 14, 0, 2, 7, 11, 9},   {0}, {0}, MPKIND__INTLK,       2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_full_f            = {{12, 13, 14, 0, 4, 5, 6, 8},    {10, 15, 3, 1, 2, 7, 11, 9},    {0}, {0}, MPKIND__CONCPHAN,    2,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_2x4_2x2           = {{0, 1, 6, 7},                   {2, 3, 4, 5},                   {0}, {0}, MPKIND__SPLIT,       2,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_2x3_1x3           = {{5, 4, 3},                      {0, 1, 2},                      {0}, {0}, MPKIND__SPLIT,       2,  s_2x3,  s_1x3,     0x000, 1};
Private map_thing map_2x4_1x4           = {{7, 6, 4, 5},                   {0, 1, 3, 2},                   {0}, {0}, MPKIND__SPLIT,       2,  s2x4,   s1x4,      0x000, 1};
Private map_thing map_qtg_dmd           = {{0, 7, 5, 6},                   {1, 2, 4, 3},                   {0}, {0}, MPKIND__SPLIT,       2,  s_qtag, sdmd,      0x005, 0};
Private map_thing map_ptp_dmd           = {{0, 1, 2, 3},                   {6, 7, 4, 5},                   {0}, {0}, MPKIND__SPLIT,       2,  s_ptpd, sdmd,      0x000, 0};
Private map_thing map_3dmd_dmd = {{0, 10, 8, 9},                  {1, 5, 7, 11},                 {2, 3, 6, 4},  {0}, MPKIND__SPLIT,       3,  s_3dmd, sdmd,      0x015, 0};
        map_thing map_4x6_2x4  = {{1, 10, 19, 16, 17, 18, 11, 0}, {3, 8, 21, 14, 15, 20, 9, 2},
                                                                  {5, 6, 23, 12, 13, 22, 7, 4},                 {0}, MPKIND__SPLIT,       3,  s4x6,   s2x4,      0x015, 0};
        map_thing map_hv_qtg_2          = {{0, 1, 15, 14, 10, 11, 12, 13}, {2, 3, 4, 5, 8, 9, 7, 6},       {0}, {0}, MPKIND__SPLIT,       2,  s_4dmd, s_qtag,    0x000, 0};
        map_thing map_vv_qtg_2          = {{9, 20, 16, 19, 18, 11, 1, 10}, {6, 23, 13, 22, 21, 8, 4, 7},   {0}, {0}, MPKIND__SPLIT,       2,  s4x6, s_qtag,      0x005, 0};
Private map_thing map_1x6_1x3  = {{0, 1, 2},                          {5, 4, 3},                           {0}, {0}, MPKIND__SPLIT,       2,  s_1x6,  s_1x3,     0x000, 0};
Private map_thing map_1x8_1x4  = {{0, 1, 2, 3},                       {6, 7, 4, 5},                        {0}, {0}, MPKIND__SPLIT,       2,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_1x12_1x4 = {{0, 1, 3, 2},                   {4, 5, 10, 11},                {9, 8, 6, 7},  {0}, MPKIND__SPLIT,       3,  s1x12,  s1x4,      0x000, 0};
Private map_thing map_1x2_1x1           = {{0},                            {1},                            {0}, {0}, MPKIND__SPLIT,       2,  s_1x2,  s_1x1,     0x000, 0};
Private map_thing map_1x4_1x2           = {{0, 1},                         {3, 2},                         {0}, {0}, MPKIND__SPLIT,       2,  s1x4,   s_1x2,     0x000, 0};
        map_thing map_tgl4_1            = {{1, 0},                         {2, 3},                         {0}, {0}, MPKIND__NONE,        2,  s_trngl4,s_1x2,    0x001, 1};
        map_thing map_tgl4_2            = {{3, 2},                         {1, 0},                         {0}, {0}, MPKIND__NONE,        2,  s_trngl4,s_1x2,    0x006, 1};
Private map_thing map_2x6_2x2 = {{0, 1, 10, 11},                  {2, 3, 8, 9},                  {4, 5, 6, 7},  {0}, MPKIND__SPLIT,       3,  s2x6,   s2x2,      0x000, 0};
Private map_thing map_2x6_2x2r = {{1, 10, 11, 0},                 {3, 8, 9, 2},                  {5, 6, 7, 4},  {0}, MPKIND__SPLIT,       3,  s2x6,   s2x2,      0x015, 0};
Private map_thing map_conc_tb = {{0, 5, 6, 11},                   {1, 4, 7, 10},                 {2, 3, 8, 9},  {0}, MPKIND__CONCPHAN,    3,  s2x6,   s2x2,      0x000, 0};
Private map_thing map_conc_tbr = {{5, 6, 11, 0},                  {4, 7, 10, 1},                 {3, 8, 9, 2},  {0}, MPKIND__CONCPHAN,    3,  s2x6,   s2x2,      0x015, 0};
Private map_thing map_3x4_1x4 = {{9, 8, 6, 7},                    {10, 11, 4, 5},                {0, 1, 3, 2},  {0}, MPKIND__SPLIT,       3,  s3x4,   s1x4,      0x000, 1};
Private map_thing map_2x3_1x2 = {{0, 5},                          {1, 4},                        {2, 3},        {0}, MPKIND__SPLIT,       3,  s_2x3,  s_1x2,     0x015, 0};
Private map_thing map_1x6_1x2 = {{0, 1},                          {2, 5},                        {4, 3},        {0}, MPKIND__SPLIT,       3,  s_1x6,  s_1x2,     0x000, 0};
Private map_thing map_2x2h = {{3, 2},                             {0, 1},                        {0},           {0}, MPKIND__SPLIT,       2,  s2x2,   s_1x2,     0x000, 1};
Private map_thing map_2x4_2x2r          = {{1, 6, 7, 0},                   {3, 4, 5, 2},                   {0}, {0}, MPKIND__SPLIT,       2,  s2x4,   s2x2,      0x005, 0};
        map_thing map_2x6_2x3   = {{0, 1, 2, 9, 10, 11},              {3, 4, 5, 6, 7, 8},                  {0}, {0}, MPKIND__SPLIT,       2,  s2x6,   s_2x3,     0x000, 0};
Private map_thing map_1x2_rmv   = {{0, 3},                            {1, 2},                              {0}, {0}, MPKIND__REMOVED,     2,  s1x4,   s_1x2,     0x000, 0};
Private map_thing map_1x2_rmvr  = {{0, 3},                            {1, 2},                              {0}, {0}, MPKIND__REMOVED,     2,  s2x2,   s_1x2,     0x005, 0};
Private map_thing map_2x2_rmv   = {{0, 2, 5, 7},                      {1, 3, 4, 6},                        {0}, {0}, MPKIND__REMOVED,     2,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_1x4_rmv   = {{0, 3, 5, 6},                      {1, 2, 4, 7},                        {0}, {0}, MPKIND__REMOVED,     2,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_1x4_rmvr  = {{7, 6, 4, 5},                      {0, 1, 3, 2},                        {0}, {0}, MPKIND__REMOVED,     2,  s2x4,   s1x4,      0x000, 1};
Private map_thing map_dmd_rmv   = {{6, 0, 3, 5},                      {7, 1, 2, 4},                        {0}, {0}, MPKIND__REMOVED,     2,  s_rigger, sdmd,    0x000, 0};
Private map_thing map_dmd_rmvr  = {{0, 3, 5, 6},                      {1, 2, 4, 7},                        {0}, {0}, MPKIND__REMOVED,     2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_dbloff1   = {{0, 1, 3, 2, 4, 5, 7, 6},      {0},                           {0},           {0}, MPKIND__NONE,        1,  s2x4,   s_qtag,    0x000, 0};
        map_thing map_dbloff2   = {{2, 3, 4, 5, 6, 7, 0, 1},      {0},                           {0},           {0}, MPKIND__NONE,        1,  s2x4,   s_qtag,    0x000, 0};
Private map_thing map_ov_1x4_2  = {{0, 1, 2, 3},                  {3, 2, 7, 6},                 {6, 7, 4, 5},   {0}, MPKIND__OVERLAP,     3,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_ov_1x4_3  = {{9, 8, 6, 7},                  {10, 11, 4, 5},               {0, 1, 3, 2},   {1}, MPKIND__OVERLAP,     3,  s3x4,   s1x4,      0x000, 0};
Private map_thing map_ov_s2x2_1         = {{0, 1, 4, 5},                   {1, 2, 3, 4},                   {0}, {0}, MPKIND__OVERLAP,     2,  s_2x3,   s2x2,     0x000, 0};
Private map_thing map_ov_s2x2r_1        = {{1, 4, 5, 0},                   {2, 3, 4, 1},                   {0}, {0}, MPKIND__OVERLAP,     2,  s_2x3,   s2x2,     0x005, 0};
Private map_thing map_ov_s2x2_2 = {{0, 1, 6, 7},                  {1, 2, 5, 6},                 {2, 3, 4, 5},   {0}, MPKIND__OVERLAP,     3,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_ov_2x3_3  = {{1, 11, 8, 9, 10, 0},      {2, 5, 7, 8, 11, 1},     {3, 4, 6, 7, 5, 2},      {0}, MPKIND__OVERLAP,     3,  s3x4,   s_2x3,     0x015, 0};
Private map_thing map_ov_2x4_0  = {{0, 1, 2, 3, 8, 9, 10, 11},        {2, 3, 4, 5, 6, 7, 8, 9},            {0}, {0}, MPKIND__OVERLAP,     2,  s2x6,   s2x4,      0x000, 0};
Private map_thing map_ov_2x4_1  = {{10, 11, 5, 4, 6, 7, 8, 9},        {0, 1, 2, 3, 4, 5, 11, 10},          {0}, {0}, MPKIND__OVERLAP,     2,  s3x4,   s2x4,      0x000, 1};
Private map_thing map_ov_2x4_2  = {{0, 1, 2, 3, 12, 13, 14, 15},           {2, 3, 4, 5, 10, 11, 12, 13},
                                   {4, 5, 6, 7, 8, 9, 10, 11},        {0},                                           MPKIND__OVERLAP,     3,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_ov_2x4_3  = {{9, 11, 7, 2, 4, 5, 6, 8},         {10, 15, 3, 1, 2, 7, 11, 9},
                                   {12, 13, 14, 0, 1, 3, 15, 10},     {0},                                           MPKIND__OVERLAP,     3,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_ov_s1x8_0 = {{0, 1, 3, 2, 10, 11, 4, 5},        {4, 5, 10, 11, 6, 7, 9, 8},          {0}, {0}, MPKIND__OVERLAP,     2,  s1x12,  s1x8,      0x000, 0};
Private map_thing map_ov_s1x8_1 = {{0, 1, 3, 2, 7, 6, 4, 5},          {4, 5, 7, 6, 12, 13, 15, 14},
                                   {15, 14, 12, 13, 8, 9, 11, 10},    {0},                                           MPKIND__OVERLAP,     3,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_ov_qtag_0 = {{0, 1, 5, 11, 7, 8, 9, 10},        {1, 2, 3, 4, 6, 7, 11, 5},           {0}, {0}, MPKIND__OVERLAP,     2,  s_3dmd, s_qtag,    0x000, 0};
Private map_thing map_ov_qtag_2 = {{0, 1, 15, 14, 10, 11, 12, 13},    {1, 2, 6, 7, 9, 10, 14, 15},
                                   {2, 3, 4, 5, 8, 9, 7, 6},          {0},                                           MPKIND__OVERLAP,     3,  s_4dmd, s_qtag,    0x000, 0};

/* Maps for turning triangles into boxes for the "triangle" concept. */
/* Nonzero item in map3[1] for arity 1 map means do not reassemble. */
        map_thing map_trngl_box1        = {{1, 2, -1, 0},                  {0},                         {0, 1}, {0}, MPKIND__NONE,        1,  s_trngl, s2x2,     0x000, 0};
        map_thing map_trngl_box2        = {{1, 2, 0, -1},                  {0},                         {0, 1}, {0}, MPKIND__NONE,        1,  s_trngl, s2x2,     0x000, 0};

/* For maps with arity 1, a nonzero item in map3[0] means that the "offset goes away" warning should be given in certain cases. */
/* For maps with arity 3, a nonzero item in map4[0] means that the "overlap goes away" warning should be given in certain cases. */

Private map_thing map_lh_s2x4_2         = {{2, 3, 4, 5, 6, 7, 8, 9},     {18, 19, 20, 21, 14, 15, 16, 17}, {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s4x6,   s2x4,      0x000, 0};
Private map_thing map_lh_s1x4_3         = {{9, 8, 6, 7},                   {0, 1, 3, 2},                   {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s2x6,   s1x4,      0x000, 1};
Private map_thing map_lh_s2x2_2         = {{10, 11, 8, 9},                 {2, 3, 4, 5},                   {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s3x4,   s2x2,      0x000, 0};
Private map_thing map_lh_s2x2_3         = {{11, 8, 9, 10},                 {3, 4, 5, 2},                   {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s3x4,   s2x2,      0x005, 0};
Private map_thing map_lh_s2x4_0         = {{0, 1, 2, 3, 6, 7, 8, 9},       {0},                            {0}, {0}, MPKIND__OFFS_L_HALF, 1,  s2x6,   s2x4,      0x000, 0};
Private map_thing map_lh_s2x4_1         = {{10, 11, 2, 3, 4, 5, 8, 9},     {0},                            {0}, {0}, MPKIND__OFFS_L_HALF, 1,  s3x4,   s2x4,      0x000, 1};
Private map_thing map_lh_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_L_HALF, 1,  s1x8,   s1x8,      0x000, 0};
        map_thing map_lh_s2x3_3         = {{1, 2, 4, 5, 7, 3},             {13, 15, 11, 9, 10, 12},        {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s4x4,   s_2x3,     0x005, 1};
        map_thing map_lh_s2x3_2         = {{9, 11, 7, 5, 6, 8},            {13, 14, 0, 1, 3, 15},          {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s4x4,   s_2x3,     0x000, 0};

Private map_thing map_rh_s2x4_2         = {{0, 1, 2, 3, 8, 9, 10, 11},   {20, 21, 22, 23, 12, 13, 14, 15}, {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s4x6,   s2x4,      0x000, 0};
Private map_thing map_rh_s1x4_3         = {{11, 10, 8, 9},                 {2, 3, 5, 4},                   {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s2x6,   s1x4,      0x000, 1};
Private map_thing map_rh_s2x2_2         = {{0, 1, 11, 10},                 {5, 4, 6, 7},                   {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s3x4,   s2x2,      0x000, 0};
Private map_thing map_rh_s2x2_3         = {{1, 11, 10, 0},                 {4, 6, 7, 5},                   {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s3x4,   s2x2,      0x005, 0};
Private map_thing map_rh_s2x4_0         = {{2, 3, 4, 5, 8, 9, 10, 11},     {0},                            {0}, {0}, MPKIND__OFFS_R_HALF, 1,  s2x6,   s2x4,      0x000, 0};
Private map_thing map_rh_s2x4_1         = {{0, 1, 5, 4, 6, 7, 11, 10},     {0},                            {0}, {0}, MPKIND__OFFS_R_HALF, 1,  s3x4,   s2x4,      0x000, 1};
Private map_thing map_rh_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_R_HALF, 1,  s1x8,   s1x8,      0x000, 0};
        map_thing map_rh_s2x3_3         = {{15, 11, 6, 8, 9, 10},          {0, 1, 2, 7, 3, 14},            {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s4x4,   s_2x3,     0x005, 1};
        map_thing map_rh_s2x3_2         = {{12, 13, 14, 3, 15, 10},        {11, 7, 2, 4, 5, 6},            {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s4x4,   s_2x3,     0x000, 0};

Private map_thing map_lf_s1x4_3         = {{11, 10, 8, 9},                 {0, 1, 3, 2},                   {0}, {0}, MPKIND__OFFS_L_FULL, 2,  s2x8,   s1x4,      0x000, 1};
Private map_thing map_lf_s2x2_2         = {{9, 11, 6, 8},                  {14, 0, 1, 3},                  {0}, {0}, MPKIND__OFFS_L_FULL, 2,  s4x4,   s2x2,      0x000, 0};
Private map_thing map_lf_s2x2_3         = {{11, 6, 8, 9},                  {0, 1, 3, 14},                  {0}, {0}, MPKIND__OFFS_L_FULL, 2,  s4x4,   s2x2,      0x005, 0};
Private map_thing map_lf_s2x4_0         = {{0, 1, 2, 3, 8, 9, 10, 11},     {0},                            {0}, {0}, MPKIND__OFFS_L_FULL, 1,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_lf_s2x4_1         = {{13, 15, 2, 4, 5, 7, 10, 12},   {0},                            {0}, {0}, MPKIND__OFFS_L_FULL, 1,  s4x4,   s2x4,      0x001, 0};
Private map_thing map_lf_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_L_FULL, 1,  s1x8,   s1x8,      0x000, 0};

Private map_thing map_rf_s1x4_3         = {{15, 14, 12, 13},               {4, 5, 7, 6},                   {0}, {0}, MPKIND__OFFS_R_FULL, 2,  s2x8,   s1x4,      0x000, 1};
Private map_thing map_rf_s2x2_2         = {{12, 13, 15, 10},               {7, 2, 4, 5},                   {0}, {0}, MPKIND__OFFS_R_FULL, 2,  s4x4,   s2x2,      0x000, 0};
Private map_thing map_rf_s2x2_3         = {{13, 15, 10, 12},               {2, 4, 5, 7},                   {0}, {0}, MPKIND__OFFS_R_FULL, 2,  s4x4,   s2x2,      0x005, 0};
Private map_thing map_rf_s2x4_0         = {{4, 5, 6, 7, 12, 13, 14, 15},   {0},                            {0}, {0}, MPKIND__OFFS_R_FULL, 1,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_rf_s2x4_1         = {{0, 1, 11, 6, 8, 9, 3, 14},     {0},                            {0}, {0}, MPKIND__OFFS_R_FULL, 1,  s4x4,   s2x4,      0x001, 0};
Private map_thing map_rf_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_R_FULL, 1,  s1x8,   s1x8,      0x000, 0};

Private map_thing map_blob_1x4a = {{13, 10, 6, 8},                {15, 17, 3, 5},            {18, 20, 1, 22},   {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 0x000, 1};
Private map_thing map_blob_1x4b = {{19, 16, 12, 14},              {21, 23, 9, 11},           {0, 2, 7, 4},      {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 0x000, 1};
Private map_thing map_blob_1x4c = {{1, 22, 18, 20},               {3, 5, 15, 17},            {6, 8, 13, 10},    {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 0x015, 0};
Private map_thing map_blob_1x4d = {{19, 16, 12, 14},              {21, 23, 9, 11},           {0, 2, 7, 4},      {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 0x015, 0};
Private map_thing map_wblob_1x4a        = {{15, 17, 5, 3, 6, 8, 10, 13},   {18, 20, 22, 1, 3, 5, 17, 15},  {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 0x000, 1};
Private map_thing map_wblob_1x4b        = {{21, 23, 11, 9, 12, 14, 16, 19},{0, 2, 4, 7, 9, 11, 23, 21},    {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 0x000, 1};
Private map_thing map_wblob_1x4c        = {{3, 5, 17, 15, 18, 20, 22, 1},  {6, 8, 10, 13, 15, 17, 5, 3},   {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 0x005, 0};
Private map_thing map_wblob_1x4d        = {{21, 23, 11, 9, 12, 14, 16, 19},{0, 2, 4, 7, 9, 11, 23, 21},    {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 0x005, 0};

        map_thing map_dmd_1x1  = {{0},                  {1},                  {2},                {3},               MPKIND__NONE,        4,  sdmd,        s_1x1,0x000, 0};
        map_thing map_star_1x1 = {{0},                  {1},                  {2},                {3},               MPKIND__NONE,        4,  s_star,      s_1x1,0x000, 0};

        map_thing map_qtag_f0           = {{7, 3},                         {0},                            {0}, {0}, MPKIND__NONE,        1,  s_qtag,      s_1x2,0x000, 0};
        map_thing map_qtag_f1           = {{1, 3},                         {7, 5},                         {0}, {0}, MPKIND__NONE,        2,  s_qtag,      s_1x2,0x005, 0};
        map_thing map_qtag_f2           = {{3, 4},                         {0, 7},                         {0}, {0}, MPKIND__NONE,        2,  s_qtag,      s_1x2,0x005, 0};

Private map_thing map_4quads1  = {{0, 2},               {4, 6},               {10, 8},            {14, 12},          MPKIND__4_QUADRANTS, 4,  s_c1phan,    s_1x2,0x044, 0};
Private map_thing map_4quads2  = {{1, 3},               {7, 5},               {11, 9},            {13, 15},          MPKIND__4_QUADRANTS, 4,  s_c1phan,    s_1x2,0x011, 0};
Private map_thing map_4edges1  = {{13, 14},             {1, 2},               {6, 5},             {10, 9},           MPKIND__4_EDGES,     4,  s4x4,        s_1x2,0x044, 0};
Private map_thing map_4edges2  = {{2, 3},               {5, 4},               {7, 6},             {0, 1},            MPKIND__4_EDGES,     4,  s_thar,      s_1x2,0x011, 0};

Private map_thing map_all_8_1  = {{0, 1, 4, 5},         {2, 3, 6, 7},         {0},                {0},               MPKIND__ALL_8,       2,  s_thar,      s1x4, 0x004, 0};
Private map_thing map_all_8_2  = {{2, 3, 6, 7},         {0, 1, 4, 5},         {0},                {0},               MPKIND__ALL_8,       2,  s_thar,      s1x4, 0x001, 0};
Private map_thing map_all_8_d1 = {{0, 3, 4, 7},         {2, 5, 6, 1},         {0},                {0},               MPKIND__ALL_8,       2,  s_thar,      sdmd, 0x004, 0};
Private map_thing map_all_8_d2 = {{2, 5, 6, 1},         {0, 3, 4, 7},         {0},                {0},               MPKIND__ALL_8,       2,  s_thar,      sdmd, 0x001, 0};



Private map_thing map_all_8_b1 = {{13, 14, 5, 6},       {1, 2, 9, 10},        {0},                {0},               MPKIND__ALL_8,       2,  s4x4,       s2x2, 0x004, 0};
Private map_thing map_all_8_b2 = {{1, 2, 9, 10},        {13, 14, 5, 6},       {0},                {0},               MPKIND__ALL_8,       2,  s4x4,       s2x2, 0x001, 0};





Private map_thing map_dmd1     = {{0, 2},               {1, 3},               {0},                {0},               MPKIND__DMD_STUFF,   2,  sdmd,        s_1x2,0x004, 0};
Private map_thing map_dmd2     = {{1, 3},               {0, 2},               {0},                {0},               MPKIND__DMD_STUFF,   2,  sdmd,        s_1x2,0x001, 0};

        map_thing *maps_3diag[4] = {&map_blob_1x4c, &map_blob_1x4a, &map_blob_1x4d, &map_blob_1x4b};
        map_thing *maps_3diagwk[4] = {&map_wblob_1x4a, &map_wblob_1x4c, &map_wblob_1x4b, &map_wblob_1x4d};

/* Beware!!  These items must be keyed to definition of "mpkind" in sd.h as follows:
                    split
                    |        removed
                    |        |        overlap
                    |        |        |        intlk
                    |        |        |        |        concphan
                    |        |        |        |        |        l_half
                    |        |        |        |        |        |        r_half
                    |        |        |        |        |        |        |        l_full
                    |        |        |        |        |        |        |        |        r_full
                    |        |        |        |        |        |        |        |        |        o_spots
                    |        |        |        |        |        |        |        |        |        |        x_spots
                    |        |        |        |        |        |        |        |        |        |        |        4quads
                    |        |        |        |        |        |        |        |        |        |        |        |        4edges
                    |        |        |        |        |        |        |        |        |        |        |        |        |        all_8
                    |        |        |        |        |        |        |        |        |        |        |        |        |        |        diamond  
                    |        |        |        |        |        |        |        |        |        |        |        |        |        |        |        stag
                    |        |        |        |        |        |        |        |        |        |        |        |        |        |        |        |
                    |        |        |        |        |        |        |        |        |        |        |        |        |        |        |        | */
Private map_hunk mm_1x8_1 = {{0, 0},
                    {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_lh_s1x8_0, 0},
                                                                          {&map_rh_s1x8_0, 0},
                                                                                   {&map_lf_s1x8_0, 0},
                                                                                            {&map_rf_s1x8_0, 0},
                                                                                                     {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_2x4_1 = {{0, 0},
                    {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_lh_s2x4_0, &map_lh_s2x4_1},
                                                                          {&map_rh_s2x4_0, &map_rh_s2x4_1},
                                                                                   {&map_lf_s2x4_0, &map_lf_s2x4_1},
                                                                                            {&map_rf_s2x4_0, &map_rf_s2x4_1},
                                                                                                     {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x1_2 = {{0, 0},
                    {&map_1x2_1x1, &map_1x2_1x1},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x2_2 = {{0, 0},
                    {&map_1x4_1x2, &map_2x2h},
                             {&map_1x2_rmv, &map_1x2_rmvr},
                                      {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_dmd1, &map_dmd2},
                                                                                                                                                           {0, 0}};

Private map_hunk mm_2x2_2 = {{0, 0},
                    {&map_2x4_2x2, &map_2x4_2x2r},
                             {&map_2x2_rmv, 0},
                                      {&map_ov_s2x2_1, &map_ov_s2x2r_1},
                                               {0, 0},  {0, 0},  {&map_lh_s2x2_2, &map_lh_s2x2_3},
                                                                          {&map_rh_s2x2_2, &map_rh_s2x2_3},
                                                                                   {&map_lf_s2x2_2, &map_lf_s2x2_3},
                                                                                            {&map_rf_s2x2_2, &map_rf_s2x2_3},
                                                                                                     {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_all_8_b1, &map_all_8_b2},
                                                                                                                                                  {0, 0},  {0, 0}};

Private map_hunk mm_dmd_2 = {{0, 0},
                    {&map_ptp_dmd, &map_qtg_dmd},
                             {&map_dmd_rmv, &map_dmd_rmvr},
                                      {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_all_8_d1, &map_all_8_d2},
                                                                                                                                                  {0, 0},  {0, 0}};

Private map_hunk mm_tgl_2 = {{0, 0},
                    {0, &map_s6_trngl},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_tg4_2 = {{0, 0},
                    {&map_rig_trngl4, 0},
                             {0, &map_s8_tgl4},
                                      {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_qtg_2 = {{0, 0},
                    {&map_hv_qtg_2, &map_vv_qtg_2},
                             {0, 0},  {&map_ov_qtag_0, 0},
                                               {&map_intlk_phantom_dmd, 0},
                                                        {&map_phantom_dmd, 0},
                                                                 {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x3_2 = {{0, 0},
                    {&map_1x6_1x3, &map_2x3_1x3},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x4_2 = {{0, 0},
                    {&map_1x8_1x4, &map_2x4_1x4},
                             {&map_1x4_rmv, &map_1x4_rmvr},
                                      {0, 0},  {0, 0},  {0, 0},  {0, &map_lh_s1x4_3},
                                                                          {0, &map_rh_s1x4_3},
                                                                                   {0, &map_lf_s1x4_3},
                                                                                            {0, &map_rf_s1x4_3},
                                                                                                     {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_all_8_1, &map_all_8_2},
                                                                                                                                                  {0, 0},  {0, 0}};

Private map_hunk mm_1x8_2 = {{0, 0},
                    {&map_1x16_1x8, &map_2x8_1x8},
                             {0, 0},  {&map_ov_s1x8_0, 0},
                                               {&map_intlk_phan_grand, 0},
                                                        {&map_conc_phan_grand, 0},
                                                                 {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x8_3 = {{0, 0},
                    {0, 0},  {0, 0},  {&map_ov_s1x8_1, 0},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_2x4_2 = {{0, 0},
                    {&map_hv_2x4_2, &map_split_f},
                             {0, 0},  {&map_ov_2x4_0, &map_ov_2x4_1},
                                               {&map_intlk_phantom_box, &map_intlk_f},
                                                        {&map_phantom_box, &map_full_f},
                                                                 {&map_lh_s2x4_2, 0},
                                                                          {&map_rh_s2x4_2, 0},
                                                                                   {0, 0},  {0, 0},  {&map_o_s2x4_3, &map_o_s2x4_2},
                                                                                                              {&map_x_s2x4_3, &map_x_s2x4_2},
                                                                                                                       {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_staggerv, 0}};

Private map_hunk mm_2x3_2 = {{0, 0},
                    {&map_2x6_2x3, &map_3x4_2x3},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_2x3_3 = {{0, 0},
                    {0, 0},  {0, 0},  {0, &map_ov_2x3_3},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x6_2 = {{0, 0},
                    {&map_1x12_1x6, &map_2x6_1x6},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x2_3 = {{0, 0},
                    {&map_1x6_1x2, &map_2x3_1x2},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_2x2_3 = {{0, 0},
                    {&map_2x6_2x2, &map_2x6_2x2r},
                             {0, 0},  {&map_ov_s2x2_2, 0},
                                               {0, 0},  {&map_conc_tb, &map_conc_tbr},
                                                                 {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_dmd_3 = {{0, 0},
                    {0, &map_3dmd_dmd},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_qtg_3 = {{0, 0},
                    {0, 0},  {0, 0},  {&map_ov_qtag_2, 0},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x4_3 = {{0, 0},
                    {&map_1x12_1x4, &map_3x4_1x4},
                             {0, 0},  {&map_ov_1x4_2, &map_ov_1x4_3},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_2x4_3 = {{0, 0},
                    {0, &map_4x6_2x4},
                             {0, 0},  {&map_ov_2x4_2, &map_ov_2x4_3},
                                      {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x2_4 = {{0, 0},
                    {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_4quads1, &map_4quads2},
                                                                                                                                {&map_4edges1, &map_4edges2},
                                                                                                                                         {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_2x2_4 = {{0, 0},
                    {&map_2x8_2x2, 0},
                             {0, 0},  {0, 0},  {0, 0},  {&map_conc_qb, 0},
                                                                 {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_dmd_4 = {{0, 0},
                    {0, &map_4dmd_dmd},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

Private map_hunk mm_1x4_4 = {{0, 0},
                    {&map_1x16_1x4, &map_4x4_1x4},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};



/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h. */
map_hunk *map_lists[][4] = {
   {0,         0,         0,         0},          /* nothing */
   {0,         &mm_1x1_2, 0,         0},          /* s_1x1 */
   {0,         &mm_1x2_2, &mm_1x2_3, &mm_1x2_4},  /* s_1x2 */
   {0,         &mm_1x3_2, 0,         0},          /* s_1x3 */
   {0,         &mm_2x2_2, &mm_2x2_3, &mm_2x2_4},  /* s2x2 */
   {0,         &mm_dmd_2, &mm_dmd_3, &mm_dmd_4},  /* sdmd */
   {0,         0,         0,         0},          /* s_star */
   {0,         &mm_tgl_2, 0,         0},          /* s_trngl */
   {0,         &mm_tg4_2, 0,         0},          /* s_trngl4 */
   {0,         0,         0,         0},          /* s_bone6 */
   {0,         0,         0,         0},          /* s_short6 */
   {0,         &mm_qtg_2, &mm_qtg_3, 0},          /* s_qtag */
   {0,         0,         0,         0},          /* s_bone */
   {0,         0,         0,         0},          /* s_rigger */
   {0,         0,         0,         0},          /* s_spindle */
   {0,         0,         0,         0},          /* s_hrglass */
   {0,         0,         0,         0},          /* s_hyperglass */
   {0,         0,         0,         0},          /* s_crosswave */
   {0,         &mm_1x4_2, &mm_1x4_3, &mm_1x4_4},  /* s1x4 */
   {&mm_1x8_1, &mm_1x8_2, &mm_1x8_3, 0},          /* s1x8 */
   {&mm_2x4_1, &mm_2x4_2, &mm_2x4_3, 0},          /* s2x4 */
   {0,         &mm_2x3_2, &mm_2x3_3, 0},          /* s_2x3 */
   {0,         &mm_1x6_2, 0,         0},          /* s_1x6 */
   {0,         0,         0,         0},          /* s3x4 */
   {0,         0,         0,         0},          /* s2x6 */
   {0,         0,         0,         0},          /* s2x8 */
   {0,         0,         0,         0},          /* s4x4 */
   {0,         0,         0,         0},          /* s_x1x6 */
   {0,         0,         0,         0},          /* s1x10 */
   {0,         0,         0,         0},          /* s1x12 */
   {0,         0,         0,         0},          /* s1x14 */
   {0,         0,         0,         0},          /* s1x16 */
   {0,         0,         0,         0},          /* s_c1phan */
   {0,         0,         0,         0},          /* s_bigblob */
   {0,         0,         0,         0},          /* s_ptpd */
   {0,         0,         0,         0},          /* s_3x1dmd */
   {0,         0,         0,         0},          /* s_3dmd */
   {0,         0,         0,         0},          /* s_4dmd */
   {0,         0,         0,         0},          /* s_wingedstar */
   {0,         0,         0,         0},          /* s_wingedstar12 */
   {0,         0,         0,         0},          /* s_wingedstar16 */
   {0,         0,         0,         0},          /* s_galaxy */
   {0,         0,         0,         0},          /* s4x6 */
   {0,         0,         0,         0},          /* s_thar */
   {0,         0,         0,         0},          /* s_x4dmd */
   {0,         0,         0,         0},          /* s_8x8 */
   {0,         0,         0,         0}};         /* s_normal_concentric */
