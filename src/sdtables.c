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

/* This defines the following external variables:
   general_concept_size
   general_concept_offset
   concept_offset_tables
   concept_size_tables
   concept_menu_strings
   main_call_lists
   number_of_calls
   calling_level
   getout_strings
   filename_strings
   resolve_names
   resolve_distances
   menu_names
   keytab
   setup_coords
   nice_setup_coords
   setup_limits
   begin_sizes
   startinfolist
   map_b6_trngl
   map_s6_trngl
   map_2x2v
   map_4x4v
   map_2x4_magic
   map_qtg_magic
   map_qtg_intlk
   map_qtg_magic_intlk
   map_ptp_magic
   map_ptp_intlk
   map_ptp_magic_intlk
   map_4x4_ns
   map_4x4_ew
   map_offset
   map_ov_s2x4_k
   map_dbloff1
   map_dbloff2
   map_trngl_box1
   map_trngl_box2
   map_lh_s2x3_3
   map_lh_s2x3_2
   map_rh_s2x3_3
   map_rh_s2x3_2
   map_lf_s2x4_r
   map_rf_s2x4_r
   map_dmd_1x1
   maps_3diag
   maps_3diagwk
   map_lists
   special_magic
   special_interlocked
   mark_end_of_list
   marker_decline
   marker_concept_mod
   marker_concept_modreact
   marker_concept_modtag
   marker_concept_force
   marker_concept_comment
   concept_descriptor_table
   nice_setup_concept
*/

#include "sd.h"



/* These are used in setup_coords. */

static coordrec thing1x1 = {s_1x1, 3,
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

static coordrec thing1x2 = {s_1x2, 3,
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

static coordrec thing1x3 = {s_1x3, 3,
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

static coordrec thing1x4 = {s1x4, 3,
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

static coordrec thing1x6 = {s_1x6, 3,
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

static coordrec thing1x8 = {s1x8, 3,
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

static coordrec thing1x10 = {s1x10, 4,
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

static coordrec thing1x12 = {s1x12, 4,
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

static coordrec thing1x14 = {s1x14, 4,
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

static coordrec thing1x16 = {s1x16, 4,
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

static coordrec thing2x2 = {s2x2, 3,
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

static coordrec thing2x3 = {s_2x3, 3,
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

static coordrec thing2x4 = {s2x4, 3,
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

static coordrec thing2x6 = {s2x6, 3,
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

static coordrec thing2x8 = {s2x8, 3,
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

static coordrec thing3x4 = {s3x4, 3,
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

static coordrec thinggal = {s_galaxy, 3,
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

static coordrec thingrigger = {s_rigger, 3,
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

static coordrec thingspindle = {s_spindle, 3,
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

static coordrec thing3x1dmd = {s_3x1dmd, 3,   /* used for both -- symmetric and safe for press/truck */
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

static coordrec thing3dmd = {s_3dmd, 3,   /* used for both -- symmetric and safe for press/truck */
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

static coordrec thing4dmd = {s_4dmd, 3,   /* used for both -- symmetric and safe for press/truck */
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

static coordrec nicethingdmd = {sdmd, 3,          /* "true" coordinates -- there is no "fudged" version */
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

static coordrec nicethingshort6 = {s_short6, 3,   /* "true" coordinates -- there is no "fudged" version */
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

static coordrec nicethingphan = {s_c1phan, 3,   /* "true" coordinates -- there is no "fudged" version */
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

static coordrec thingglass = {s_hrglass, 3,
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

static coordrec thing4x4 = {s4x4, 3,
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

static coordrec thing4x6 = {s4x6, 3,
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

static coordrec thingblob = {s_bigblob, 3,
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
   "sequence.MS",
   "sequence.Plus",
   "sequence.A1",
   "sequence.A2",
   "sequence.C1",
   "sequence.C2",
   "sequence.C3A",
   "sequence.C3",
   "sequence.C3X",
   "sequence.C4A",
   "sequence.C4",
   "sequence.all",
   ""};

/* BEWARE!!  This list is keyed to the definition of "resolve_kind" in sd.h . */
char *resolve_names[] = {
   "???",
   "right and left grand",
   "left allemande",
   "extend, right and left grand",
   "extend, left allemande",
   "circulate, right and left grand",
   "circulate, left allemande",
   "pass thru, right and left grand",
   "pass thru, left allemande",
   "trade by, right and left grand",
   "trade by, left allemande",
   "cross by, right and left grand",
   "cross by, left allemande",
   "dixie grand, left allemande",
   "promenade"};

char *resolve_distances[] = {
   "  (At home)",
   "  (1/8 promenade)",
   "  (1/4 promenade)",
   "  (3/8 promenade)",
   "  (1/2 promenade)",
   "  (5/8 promenade)",
   "  (3/4 promenade)",
   "  (7/8 promenade)"};

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
begin_kind keytab[][2] = {
   {b_nothing,       b_nothing},        /* nothing */
   {b_1x1,           b_1x1},            /* s_1x1 */
   {b_1x2,           b_2x1},            /* s_1x2 */
   {b_1x3,           b_3x1},            /* s_1x3 */
   {b_2x2,           b_2x2},            /* s2x2 */
   {b_dmd,           b_pmd},            /* sdmd */
   {b_star,          b_star},           /* s_star */
   {b_trngl,         b_ptrngl},         /* s_trngl */
   {b_bone6,         b_pbone6},         /* s_bone6 */
   {b_short6,        b_pshort6},        /* s_short6 */
   {b_qtag,          b_pqtag},          /* s_qtag */
   {b_nothing,       b_nothing},        /* s_bone */
   {b_rigger,        b_prigger},        /* s_rigger */
   {b_spindle,       b_pspindle},       /* s_spindle */
   {b_hrglass,       b_phrglass},       /* s_hrglass */
   {b_nothing,       b_nothing},        /* s_hyperglass */
   {b_crosswave,     b_pcrosswave},     /* s_crosswave */
   {b_1x4,           b_4x1},            /* s1x4 */
   {b_1x8,           b_8x1},            /* s1x8 */
   {b_2x4,           b_4x2},            /* s2x4 */
   {b_2x3,           b_3x2},            /* s_2x3 */
   {b_1x6,           b_6x1},            /* s_1x6 */
   {b_3x4,           b_4x3},            /* s3x4 */
   {b_2x6,           b_6x2},            /* s2x6 */
   {b_2x8,           b_8x2},            /* s2x8 */
   {b_4x4,           b_4x4},            /* s4x4 */
   {b_1x10,          b_10x1},           /* b_1x10 */
   {b_1x12,          b_12x1},           /* b_1x12 */
   {b_1x14,          b_14x1},           /* b_1x14 */
   {b_1x16,          b_16x1},           /* b_1x16 */
   {b_c1phan,        b_c1phan},         /* s_c1phan */
   {b_nothing,       b_nothing},        /* s_bigblob */
   {b_ptpd,          b_pptpd},          /* s_ptpd */
   {b_3x1dmd,        b_p3x1dmd},        /* s_3x1dmd */
   {b_3dmd,          b_p3dmd},          /* s_3dmd */
   {b_4dmd,          b_p4dmd},          /* s_4dmd */
   {b_nothing,       b_nothing},        /* s_concstar */
   {b_nothing,       b_nothing},        /* s_concstar12 */
   {b_nothing,       b_nothing},        /* s_concstar16 */
   {b_galaxy,        b_galaxy},         /* s_galaxy */
   {b_nothing,       b_nothing},        /* s4x6 */
   {b_nothing,       b_nothing},        /* s_thar */
   {b_nothing,       b_nothing}};       /* s_normal_concentric */


/* These "coordrec" items have the fudged coordinates that are used for doing
   press/truck calls.  For some setups, the coordinates of some people are
   deliberately moved away from the obvious precise matrix spots so that
   those people can't press or truck.  For example, the lateral spacing of
   diamond points is not an integer.  If a diamond point does any truck or loop
   call, he/she will not end up on the other diamond point spot (or any other
   spot in the formation), so the call will not be legal.  This enforces our
   view, not shared by all callers (Hi, Clark!) that the diamond points are NOT
   as if the ends of lines of 3, and hence can NOT trade with each other by
   doing a right loop 1. */

/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
coordrec *setup_coords[] = {
   0,                  /* nothing */
   &thing1x1,          /* s_1x1 */
   &thing1x2,          /* s_1x2 */
   &thing1x3,          /* s_1x3 */
   &thing2x2,          /* s2x2 */
   0,                  /* sdmd */      /* nothing here -- press and truck not done here */
   0,                  /* s_star */
   0,                  /* s_trngl */
   0,                  /* s_bone6 */
   0,                  /* s_short6 */  /* nothing here -- press and truck not done here */
   &thingqtag,         /* s_qtag */
   &thingbone,         /* s_bone */
   &thingrigger,       /* s_rigger */
   &thingspindle,      /* s_spindle */
   &thingglass,        /* s_hrglass */
   0,                  /* s_hyperglass */
   0,                  /* s_crosswave */
   &thing1x4,          /* s1x4 */
   &thing1x8,          /* s1x8 */
   &thing2x4,          /* s2x4 */
   &thing2x3,          /* s_2x3 */
   &thing1x6,          /* s_1x6 */
   &thing3x4,          /* s3x4 */
   &thing2x6,          /* s2x6 */
   &thing2x8,          /* s2x8 */
   &thing4x4,          /* s4x4 */
   &thing1x10,         /* s1x10 */
   &thing1x12,         /* s1x12 */
   &thing1x14,         /* s1x14 */
   &thing1x16,         /* s1x16 */
   0,                  /* s_c1phan */
   &thingblob,         /* s_bigblob */
   0,                  /* s_ptpd */
   &thing3x1dmd,       /* s_3x1dmd */
   &thing3dmd,         /* s_3dmd */
   &thing4dmd,         /* s_4dmd */
   0,                  /* s_concstar */
   0,                  /* s_concstar12 */
   0,                  /* s_concstar16 */
   &thinggal,          /* s_galaxy */
   &thing4x6,          /* s4x6 */
   0,                  /* s_thar */
   0};                 /* s_normal_concentric */


/* The above table is not suitable for performing mirror inversion because,
   for example, the points of diamonds do not reflect onto each other.  This
   table has unfudged coordinates, in which all the symmetries are observed.
   This is the table that is used for mirror reversal.  Most of the items in
   it are the same as those in the table above. */

/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
coordrec *nice_setup_coords[] = {
   0,                  /* nothing */
   &thing1x1,          /* s_1x1 */
   &thing1x2,          /* s_1x2 */
   &thing1x3,          /* s_1x3 */
   &thing2x2,          /* s2x2 */
   &nicethingdmd,      /* sdmd */
   0,                  /* s_star */
   0,                  /* s_trngl */
   0,                  /* s_bone6 */
   &nicethingshort6,   /* s_short6 */
   &nicethingqtag,     /* s_qtag */
   &thingbone,         /* s_bone */
   &thingrigger,       /* s_rigger */
   &thingspindle,      /* s_spindle */
   &thingglass,        /* s_hrglass */
   0,                  /* s_hyperglass */
   0,                  /* s_crosswave */
   &thing1x4,          /* s1x4 */
   &thing1x8,          /* s1x8 */
   &thing2x4,          /* s2x4 */
   &thing2x3,          /* s_2x3 */
   &thing1x6,          /* s_1x6 */
   &thing3x4,          /* s3x4 */
   &thing2x6,          /* s2x6 */
   &thing2x8,          /* s2x8 */
   &thing4x4,          /* s4x4 */
   &thing1x10,         /* s1x10 */
   &thing1x12,         /* s1x12 */
   &thing1x14,         /* s1x14 */
   &thing1x16,         /* s1x16 */
   &nicethingphan,     /* s_c1phan */
   &thingblob,         /* s_bigblob */
   0,                  /* s_ptpd */
   &thing3x1dmd,       /* s_3x1dmd */
   &thing3dmd,         /* s_3dmd */
   &thing4dmd,         /* s_4dmd */
   0,                  /* s_concstar */
   0,                  /* s_concstar12 */
   0,                  /* s_concstar16 */
   &thinggal,          /* s_galaxy */
   &thing4x6,          /* s4x6 */
   0,                  /* s_thar */
   0};                 /* s_normal_concentric */


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
   7,          /* s_concstar */
   11,         /* s_concstar12 */
   15,         /* s_concstar16 */
   7,          /* s_galaxy */
   23,         /* s4x6 */
   7,          /* s_thar */
   -1};        /* s_normal_concentric */

/* BEWARE!!  This list is keyed to the definition of "begin_kind" in SD.H. */
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
      FALSE,
      {
         nothing,
         0,
         {{0,0}},
         1
      }
   },
   {
      "Heads 1P2P",
      FALSE,                         /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* setupflags */
         {{G2A|d_south,G2B}, {B2A|d_south,B2B}, {G1A|d_south,G1B}, {B1A|d_south,B1B},
            {G4A|d_north,G4B}, {B4A|d_north,B4B}, {G3A|d_north,G3B}, {B3A|d_north,B3B}},
         1                           /* rotation */
      }
   },
   {
      "Sides 1P2P",
      FALSE,                         /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* setupflags */
         {{G3A|d_south,G3B}, {B3A|d_south,B3B}, {G2A|d_south,G2B}, {B2A|d_south,B2B},
            {G1A|d_north,G1B}, {B1A|d_north,B1B}, {G4A|d_north,G4B}, {B4A|d_north,B4B}},
         0                           /* rotation */
      }
   },
   {
      "HEADS",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* setupflags */
         {{B4A|d_east,B4B}, {G3A|d_south,G3B}, {B3A|d_south,B3B}, {G2A|d_west,G2B},
            {B2A|d_west,B2B}, {G1A|d_north,G1B}, {B1A|d_north,B1B}, {G4A|d_east,G4B}},
         0                           /* rotation */
      }
   },
   {
      "SIDES",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* setupflags */
         {{B3A|d_east,B3B}, {G2A|d_south,G2B}, {B2A|d_south,B2B}, {G1A|d_west,G1B},
            {B1A|d_west,B1B}, {G4A|d_north,G4B}, {B4A|d_north,B4B}, {G3A|d_east,G3B}},
         1                           /* rotation */
      }
   },
   {
      "From squared set",
      FALSE,                         /* into_the_middle */
      {
         s4x4,                       /* kind */
         0,                          /* setupflags */
         {{0,0}, {G2A|d_west,G2B}, {B2A|d_west,B2B}, {0,0}, {0,0}, {G1A|d_north,G1B},
            {B1A|d_north,B1B}, {0,0}, {0,0}, {G4A|d_east,G4B}, {B4A|d_east,B4B}, {0,0},
            {0,0}, {G3A|d_south,G3B}, {B3A|d_south,B3B}, {0,0}},
         0                           /* rotation */
      }
   }
#ifdef ripnsnortstuff
   {
      "1/4 stable, couple #3 rip 'n' snort, then HEADS",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* setupflags */
         {{B4A|d_south,B4B}, {G3A|d_west,G3B}, {B3A|d_east,B3B}, {G2A|d_south,G2B},
            {B2A|d_south,B2B}, {G1A|d_west,G1B}, {B1A|d_east,B1B}, {G4A|d_south,G4B}},
         0                           /* rotation */
      }
   }
#endif
};




/*                                                                                                         setups are stacked on top of each other -----------------|
                                                                                       each setup is rotated clockwise before being concatenated ---------------|   |
                                                                                             (rot = 2 for second setup upside-down, 1-fold symmetry)            |   |
                                                                                                (rot = 3 for 1st setup ccw, 2nd cw, 1-fold symmetry)            |   |
                                                                                                                                                                V   V
                                            map1                             map2                        map3 map4   map_kind          arity  outer   inner    rot vert */

       map_thing map_b6_trngl          = {{5, 4, 0},                      {2, 1, 3},                      {0}, {0}, MPKIND__SPLIT,       2,  s_bone6, s_trngl,  3, 0};
       map_thing map_s6_trngl          = {{4, 5, 3},                      {1, 2, 0},                      {0}, {0}, MPKIND__SPLIT,       2,  s_short6, s_trngl, 2, 1};
       map_thing map_2x2v              = {{0, 3},                         {1, 2},                         {0}, {0}, MPKIND__SPLIT,       2,  s2x2,   s_1x2,     1, 0};
       map_thing map_2x4_magic         = {{0, 6, 3, 5},                   {7, 1, 4, 2},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s1x4,      0, 0};
       map_thing map_qtg_magic         = {{0, 2, 5, 3},                   {1, 7, 4, 6},                   {0}, {0}, MPKIND__NONE,        2,  s_qtag, sdmd,      1, 0};
       map_thing map_qtg_intlk         = {{0, 3, 5, 6},                   {1, 2, 4, 7},                   {0}, {0}, MPKIND__NONE,        2,  s_qtag, sdmd,      1, 0};
       map_thing map_qtg_magic_intlk   = {{0, 2, 5, 7},                   {1, 3, 4, 6},                   {0}, {0}, MPKIND__NONE,        2,  s_qtag, sdmd,      1, 0};
       map_thing map_ptp_magic         = {{6, 1, 4, 3},                   {0, 7, 2, 5},                   {0}, {0}, MPKIND__NONE,        2,  s_ptpd, sdmd,      0, 0};
       map_thing map_ptp_intlk         = {{0, 1, 6, 3},                   {2, 7, 4, 5},                   {0}, {0}, MPKIND__NONE,        2,  s_ptpd, sdmd,      0, 0};
       map_thing map_ptp_magic_intlk   = {{2, 1, 4, 3},                   {0, 7, 6, 5},                   {0}, {0}, MPKIND__NONE,        2,  s_ptpd, sdmd,      0, 0};
static map_thing map_2x4_diagonal      = {{2, 3, 6, 7},                   {0, 1, 4, 5},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s2x2,      0, 0};
static map_thing map_2x4_int_pgram     = {{1, 3, 5, 7},                   {0, 2, 4, 6},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s2x2,      0, 0};
static map_thing map_2x4_trapezoid     = {{1, 2, 4, 7},                   {0, 3, 5, 6},                   {0}, {0}, MPKIND__NONE,        2,  s2x4,   s2x2,      0, 0};
static map_thing map_3x4_2x3_intlk     = {{2, 5, 7, 9, 10, 0},            {3, 4, 6, 8, 11, 1},            {0}, {0}, MPKIND__NONE,        2,  s3x4,   s_2x3,     1, 0};
static map_thing map_3x4_2x3_conc      = {{3, 4, 6, 9, 10, 0},            {2, 5, 7, 8, 11, 1},            {0}, {0}, MPKIND__NONE,        2,  s3x4,   s_2x3,     1, 0};
       map_thing map_4x4_ns            = {{12, 13, 0, 14},                {8, 6, 4, 5},                   {0}, {0}, MPKIND__NONE,        2,  s4x4,   s1x4,      0, 1};
       map_thing map_4x4_ew            = {{0, 1, 4, 2},                   {12, 10, 8, 9},                 {0}, {0}, MPKIND__NONE,        2,  s4x4,   s1x4,      1, 1};
static map_thing map_phantom_box       = {{0, 1, 6, 7, 8, 9, 14, 15},     {2, 3, 4, 5, 10, 11, 12, 13},   {0}, {0}, MPKIND__NONE,        2,  s2x8,   s2x4,      0, 0};
static map_thing map_intlk_phantom_box = {{0, 1, 4, 5, 10, 11, 14, 15},   {2, 3, 6, 7, 8, 9, 12, 13},     {0}, {0}, MPKIND__INTLK,       2,  s2x8,   s2x4,      0, 0};
static map_thing map_phantom_dmd       = {{0, 3, 4, 5, 8, 11, 12, 13},    {1, 2, 6, 7, 9, 10, 14, 15},    {0}, {0}, MPKIND__NONE,        2,  s_4dmd, s_qtag,    0, 0};
static map_thing map_intlk_phantom_dmd = {{0, 2, 6, 7, 9, 11, 12, 13},    {1, 3, 4, 5, 8, 10, 14, 15},    {0}, {0}, MPKIND__INTLK,       2,  s_4dmd, s_qtag,    0, 0};
static map_thing map_split_f           = {{13, 15, 11, 6, 8, 9, 10, 12},  {0, 1, 2, 4, 5, 7, 3, 14},      {0}, {0}, MPKIND__SPLIT,       2,  s4x4,   s2x4,      1, 0};
static map_thing map_intlk_f           = {{14, 3, 7, 5, 8, 9, 10, 12},    {0, 1, 2, 4, 6, 11, 15, 13},    {0}, {0}, MPKIND__INTLK,       2,  s4x4,   s2x4,      1, 0};
static map_thing map_full_f            = {{0, 1, 2, 4, 8, 9, 10, 12},     {14, 3, 7, 5, 6, 11, 15, 13},   {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      1, 0};
static map_thing map_stagger           = {{14, 1, 7, 4, 6, 9, 15, 12},    {0, 3, 2, 5, 8, 11, 10, 13},    {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      1, 0};
static map_thing map_stairst           = {{13, 1, 11, 4, 5, 9, 3, 12},    {0, 15, 2, 6, 8, 7, 10, 14},    {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      1, 0};
static map_thing map_ladder            = {{14, 3, 2, 4, 6, 11, 10, 12},   {0, 1, 7, 5, 8, 9, 15, 13},     {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      1, 0};
static map_thing map_but_o             = {{14, 1, 2, 5, 6, 9, 10, 13},    {0, 3, 7, 4, 8, 11, 15, 12},    {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      1, 0};
static map_thing map_o_s2x4_3          = {{14, 1, 2, 5, 6, 9, 10, 13},    {0, 3, 7, 4, 8, 11, 15, 12},    {0}, {0}, MPKIND__O_SPOTS,     2,  s4x4,   s2x4,      1, 0};
static map_thing map_x_s2x4_3          = {{14, 1, 2, 5, 6, 9, 10, 13},    {0, 3, 7, 4, 8, 11, 15, 12},    {0}, {0}, MPKIND__X_SPOTS,     2,  s4x4,   s2x4,      1, 0};
static map_thing map_o_s2x4_2          = {{10, 13, 14, 1, 2, 5, 6, 9},    {12, 15, 3, 0, 4, 7, 11, 8},    {0}, {0}, MPKIND__O_SPOTS,     2,  s4x4,   s2x4,      0, 0};
static map_thing map_x_s2x4_2          = {{10, 13, 14, 1, 2, 5, 6, 9},    {12, 15, 3, 0, 4, 7, 11, 8},    {0}, {0}, MPKIND__X_SPOTS,     2,  s4x4,   s2x4,      0, 0};
       map_thing map_offset            = {{13, 15, 2, 4, 5, 7, 10, 12},   {0, 1, 11, 6, 8, 9, 3, 14},     {0}, {0}, MPKIND__NONE,        2,  s4x4,   s2x4,      1, 0};
       map_thing map_4x4v     = {{12, 10, 8, 9},       {13, 15, 6, 11},      {14, 3, 5, 7},      {0, 1, 4, 2},      MPKIND__SPLIT,       4,  s4x4,   s1x4,      1, 0};
static map_thing map_blocks   = {{12, 14, 7, 9},       {13, 0, 2, 11},       {15, 1, 4, 6},      {10, 3, 5, 8},     MPKIND__NONE,        4,  s4x4,   s2x2,      0, 0};
static map_thing map_trglbox  = {{12, 14, 15, 9},      {13, 0, 2, 3},        {7, 1, 4, 6},       {10, 11, 5, 8},    MPKIND__NONE,        4,  s4x4,   s2x2,      0, 0};
static map_thing map_4x4_1x4  = {{8, 6, 4, 5},         {9, 11, 2, 7},        {10, 15, 1, 3},     {12, 13, 0, 14},   MPKIND__SPLIT,       4,  s4x4,   s1x4,      0, 1};
static map_thing map_1x16_1x4 = {{0, 1, 3, 2},         {4, 5, 7, 6},         {15, 14, 12, 13},   {11, 10, 8, 9},    MPKIND__SPLIT,       4,  s1x16,  s1x4,      0, 0};
static map_thing map_2x8_2x2  = {{0, 1, 14, 15},       {2, 3, 12, 13},       {4, 5, 10, 11},     {6, 7, 8, 9},      MPKIND__SPLIT,       4,  s2x8,   s2x2,      0, 0};
static map_thing map_4dmd_dmd = {{0, 13, 11, 12},      {1, 15, 10, 14},      {2, 6, 9, 7},       {3, 4, 8, 5},      MPKIND__SPLIT,       4,  s_4dmd, sdmd,      1, 0};
static map_thing map_2x6_1x6   = {{11, 10, 9, 6, 7, 8},              {0, 1, 2, 5, 4, 3},                  {0}, {0}, MPKIND__SPLIT,       2,  s2x6,   s_1x6,     0, 1};
static map_thing map_2x8_1x8   = {{15, 14, 12, 13, 8, 9, 11, 10},    {0, 1, 3, 2, 7, 6, 4, 5},            {0}, {0}, MPKIND__SPLIT,       2,  s2x8,   s1x8,      0, 1};
static map_thing map_1x16_1x8  = {{0, 1, 3, 2, 7, 6, 4, 5},          {15, 14, 12, 13, 8, 9, 11, 10},      {0}, {0}, MPKIND__SPLIT,       2,  s1x16,  s1x8,      0, 0};
static map_thing map_hv_2x4_2  = {{0, 1, 2, 3, 12, 13, 14, 15},      {4, 5, 6, 7, 8, 9, 10, 11},          {0}, {0}, MPKIND__SPLIT,       2,  s2x8,   s2x4,      0, 0};
static map_thing map_3x4_2x3   = {{1, 11, 8, 9, 10, 0},              {3, 4, 6, 7, 5, 2},                  {0}, {0}, MPKIND__SPLIT,       2,  s3x4,   s_2x3,     1, 0};
static map_thing map_4x4_2x4v  = {{9, 11, 7, 2, 4, 5, 6, 8},         {12, 13, 14, 0, 1, 3, 15, 10},       {0}, {0}, MPKIND__SPLIT,       2,  s4x4,   s2x4,      0, 1};
static map_thing map_4x4_2x4intv       = {{10, 15, 3, 1, 4, 5, 6, 8},     {12, 13, 14, 0, 2, 7, 11, 9},   {0}, {0}, MPKIND__SPLIT,       2,  s4x4,   s2x4,      0, 1};
static map_thing map_2x4_2x2   = {{0, 1, 6, 7},                      {2, 3, 4, 5},                        {0}, {0}, MPKIND__SPLIT,       2,  s2x4,   s2x2,      0, 0};
static map_thing map_2x3_1x3   = {{5, 4, 3},                         {0, 1, 2},                           {0}, {0}, MPKIND__SPLIT,       2,  s_2x3,  s_1x3,     0, 1};
static map_thing map_2x4_1x4   = {{7, 6, 4, 5},                      {0, 1, 3, 2},                        {0}, {0}, MPKIND__SPLIT,       2,  s2x4,   s1x4,      0, 1};
static map_thing map_qtg_dmd   = {{0, 7, 5, 6},                      {1, 2, 4, 3},                        {0}, {0}, MPKIND__SPLIT,       2,  s_qtag, sdmd,      1, 0};
static map_thing map_ptp_dmd   = {{0, 1, 2, 3},                      {6, 7, 4, 5},                        {0}, {0}, MPKIND__SPLIT,       2,  s_ptpd, sdmd,      0, 0};
static map_thing map_3dmd_dmd = {{0, 10, 8, 9},                  {1, 5, 7, 11},                 {2, 3, 6, 4},  {0}, MPKIND__SPLIT,       3,  s_3dmd, sdmd,      1, 0};
static map_thing map_4x6_2x4  = {{1, 10, 19, 16, 17, 18, 11, 0}, {3, 8, 21, 14, 15, 20, 9, 2},
                                                                 {5, 6, 23, 12, 13, 22, 7, 4},                 {0}, MPKIND__SPLIT,       3,  s4x6,   s2x4,      1, 0};
static map_thing map_hv_qtg_2  = {{0, 1, 15, 14, 10, 11, 12, 13}, {2, 3, 4, 5, 8, 9, 7, 6},               {0}, {0}, MPKIND__SPLIT,       2,  s_4dmd, s_qtag,    0, 0};
static map_thing map_1x6_1x3   = {{0, 1, 2},                         {5, 4, 3},                           {0}, {0}, MPKIND__SPLIT,       2,  s_1x6,  s_1x3,     0, 0};
static map_thing map_1x8_1x4   = {{0, 1, 2, 3},                      {6, 7, 4, 5},                        {0}, {0}, MPKIND__SPLIT,       2,  s1x8,   s1x4,      0, 0};
static map_thing map_1x12_1x4 = {{0, 1, 3, 2},                   {4, 5, 10, 11},                {9, 8, 6, 7},  {0}, MPKIND__SPLIT,       3,  s1x12,  s1x4,      0, 0};
static map_thing map_1x2_1x1 =  {{0},                            {1},                           {0},           {0}, MPKIND__SPLIT,       2,  s_1x2,  s_1x1,     0, 0};
static map_thing map_1x4_1x2 = {{0, 1},                          {3, 2},                        {0},           {0}, MPKIND__SPLIT,       2,  s1x4,   s_1x2,     0, 0};
static map_thing map_2x6_2x2 = {{0, 1, 10, 11},                  {2, 3, 8, 9},                  {4, 5, 6, 7},  {0}, MPKIND__SPLIT,       3,  s2x6,   s2x2,      0, 0};
static map_thing map_3x4_1x4 = {{9, 8, 6, 7},                    {10, 11, 4, 5},                {0, 1, 3, 2},  {0}, MPKIND__SPLIT,       3,  s3x4,   s1x4,      0, 1};
static map_thing map_2x3_1x2 = {{0, 5},                          {1, 4},                        {2, 3},        {0}, MPKIND__SPLIT,       3,  s_2x3,  s_1x2,     1, 0};
static map_thing map_1x6_1x2 = {{0, 1},                          {2, 5},                        {4, 3},        {0}, MPKIND__SPLIT,       3,  s_1x6,  s_1x2,     0, 0};
static map_thing map_2x2h = {{3, 2},                             {0, 1},                        {0},           {0}, MPKIND__SPLIT,       2,  s2x2,   s_1x2,     0, 1};
static map_thing map_2x4_2x2r = {{1, 6, 7, 0},                   {3, 4, 5, 2},                  {0},           {0}, MPKIND__SPLIT,       2,  s2x4,   s2x2,      1, 0};
static map_thing map_2x6_2x2r = {{1, 10, 11, 0},                 {3, 8, 9, 2},                  {5, 6, 7, 4},  {0}, MPKIND__SPLIT,       3,  s2x6,   s2x2,      1, 0};
static map_thing map_2x6_2x3   = {{0, 1, 2, 9, 10, 11},              {3, 4, 5, 6, 7, 8},                  {0}, {0}, MPKIND__SPLIT,       2,  s2x6,   s_2x3,     0, 0};
static map_thing map_1x2_rmv   = {{0, 3},                            {1, 2},                              {0}, {0}, MPKIND__REMOVED,     2,  s1x4,   s_1x2,     0, 0};
static map_thing map_1x2_rmvr  = {{0, 3},                            {1, 2},                              {0}, {0}, MPKIND__REMOVED,     2,  s2x2,   s_1x2,     1, 0};
static map_thing map_2x2_rmv   = {{0, 2, 5, 7},                      {1, 3, 4, 6},                        {0}, {0}, MPKIND__REMOVED,     2,  s2x4,   s2x2,      0, 0};
static map_thing map_1x4_rmv   = {{0, 3, 5, 6},                      {1, 2, 4, 7},                        {0}, {0}, MPKIND__REMOVED,     2,  s1x8,   s1x4,      0, 0};
static map_thing map_1x4_rmvr  = {{7, 6, 4, 5},                      {0, 1, 3, 2},                        {0}, {0}, MPKIND__REMOVED,     2,  s2x4,   s1x4,      0, 1};
static map_thing map_dmd_rmv   = {{6, 0, 3, 5},                      {7, 1, 2, 4},                        {0}, {0}, MPKIND__REMOVED,     2,  s_rigger, sdmd,    0, 0};
static map_thing map_dmd_rmvr  = {{0, 3, 5, 6},                      {1, 2, 4, 7},                        {0}, {0}, MPKIND__REMOVED,     2,  s_qtag, sdmd,      1, 0};
       map_thing map_ov_s2x4_k = {{13, 15, 11, 6, 8, 9, 10, 12}, {14, 3, 7, 5, 6, 11, 15, 13},
                                  {0, 1, 2, 4, 5, 7, 3, 14},     {0},                                               MPKIND__OVERLAP,     3,  s4x4,   s2x4,      1, 0};
       map_thing map_dbloff1   = {{0, 1, 3, 2, 4, 5, 7, 6},      {0},                           {0},           {0}, MPKIND__NONE,        1,  s2x4,   s_qtag,    0, 0};
       map_thing map_dbloff2   = {{2, 3, 4, 5, 6, 7, 0, 1},      {0},                           {0},           {0}, MPKIND__NONE,        1,  s2x4,   s_qtag,    0, 0};
static map_thing map_ov_1x4_2  = {{0, 1, 2, 3},                  {3, 2, 7, 6},                 {6, 7, 4, 5},   {0}, MPKIND__OVERLAP,     3,  s1x8,   s1x4,      0, 0};
static map_thing map_ov_1x4_3  = {{9, 8, 6, 7},                  {10, 11, 4, 5},               {0, 1, 3, 2},   {1}, MPKIND__OVERLAP,     3,  s3x4,   s1x4,      0, 0};
static map_thing map_ov_s2x2_2 = {{0, 1, 6, 7},                  {1, 2, 5, 6},                 {2, 3, 4, 5},   {0}, MPKIND__OVERLAP,     3,  s2x4,   s2x2,      0, 0};
static map_thing map_ov_2x3_3  = {{1, 11, 8, 9, 10, 0},      {2, 5, 7, 8, 11, 1},     {3, 4, 6, 7, 5, 2},      {0}, MPKIND__OVERLAP,     3,  s3x4,   s_2x3,     1, 0};
static map_thing map_ov_2x4_0  = {{0, 1, 2, 3, 8, 9, 10, 11},        {2, 3, 4, 5, 6, 7, 8, 9},            {0}, {0}, MPKIND__OVERLAP,     2,  s2x6,   s2x4,      0, 0};
static map_thing map_ov_2x4_1  = {{10, 11, 5, 4, 6, 7, 8, 9},        {0, 1, 2, 3, 4, 5, 11, 10},          {0}, {0}, MPKIND__OVERLAP,     2,  s3x4,   s2x4,      0, 1};
static map_thing map_ov_2x4_2  = {{0, 1, 2, 3, 12, 13, 14, 15},           {2, 3, 4, 5, 10, 11, 12, 13},
                                  {4, 5, 6, 7, 8, 9, 10, 11},        {0},                                           MPKIND__OVERLAP,     3,  s2x8,   s2x4,      0, 0};
static map_thing map_ov_2x4_3  = {{9, 11, 7, 2, 4, 5, 6, 8},         {10, 15, 3, 1, 2, 7, 11, 9},
                                  {12, 13, 14, 0, 1, 3, 15, 10},     {0},                                           MPKIND__OVERLAP,     3,  s4x4,   s2x4,      0, 1};
static map_thing map_ov_s1x8_0 = {{0, 1, 3, 2, 10, 11, 4, 5},        {4, 5, 10, 11, 6, 7, 9, 8},          {0}, {0}, MPKIND__OVERLAP,     2,  s1x12,  s1x8,      0, 0};
static map_thing map_ov_s1x8_1 = {{0, 1, 3, 2, 7, 6, 4, 5},          {4, 5, 7, 6, 12, 13, 15, 14},
                                  {15, 14, 12, 13, 8, 9, 11, 10},    {0},                                           MPKIND__OVERLAP,     3,  s1x16,  s1x8,      0, 0};
static map_thing map_ov_qtag_0 = {{0, 1, 5, 11, 7, 8, 9, 10},        {1, 2, 3, 4, 6, 7, 11, 5},           {0}, {0}, MPKIND__OVERLAP,     2,  s_3dmd, s_qtag,    0, 0};
static map_thing map_ov_qtag_2 = {{0, 1, 15, 14, 10, 11, 12, 13},    {1, 2, 6, 7, 9, 10, 14, 15},
                                  {2, 3, 4, 5, 8, 9, 7, 6},          {0},                                           MPKIND__OVERLAP,     3,  s_4dmd, s_qtag,    0, 0};

/* Maps for turning triangles into boxes for the "triangle" concept. */
/* Nonzero item in map3[1] for arity 1 map means do not reassemble. */
       map_thing map_trngl_box1        = {{1, 2, -1, 0},                  {0},                         {0, 1}, {0}, MPKIND__NONE,        1,  s_trngl, s2x2,     0, 0};
       map_thing map_trngl_box2        = {{1, 2, 0, -1},                  {0},                         {0, 1}, {0}, MPKIND__NONE,        1,  s_trngl, s2x2,     0, 0};

/* For maps with arity 1, a nonzero item in map3[0] means that the "offset goes away" warning should be given in certain cases. */
/* For maps with arity 3, a nonzero item in map4[0] means that the "overlap goes away" warning should be given in certain cases. */

static map_thing map_lh_s1x4_3         = {{9, 8, 6, 7},                   {0, 1, 3, 2},                   {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s2x6,   s1x4,      0, 1};
static map_thing map_lh_s2x2_2         = {{10, 11, 8, 9},                 {2, 3, 4, 5},                   {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s3x4,   s2x2,      0, 0};
static map_thing map_lh_s2x2_3         = {{11, 8, 9, 10},                 {3, 4, 5, 2},                   {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s3x4,   s2x2,      1, 0};
static map_thing map_lh_s2x4_0         = {{0, 1, 2, 3, 6, 7, 8, 9},       {0},                            {0}, {0}, MPKIND__OFFS_L_HALF, 1,  s2x6,   s2x4,      0, 0};
static map_thing map_lh_s2x4_1         = {{10, 11, 2, 3, 4, 5, 8, 9},     {0},                            {0}, {0}, MPKIND__OFFS_L_HALF, 1,  s3x4,   s2x4,      0, 1};
static map_thing map_lh_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_L_HALF, 1,  s1x8,   s1x8,      0, 0};
       map_thing map_lh_s2x3_3         = {{1, 2, 4, 5, 7, 3},             {13, 15, 11, 9, 10, 12},        {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s4x4,   s_2x3,     1, 1};
       map_thing map_lh_s2x3_2         = {{9, 11, 7, 5, 6, 8},            {13, 14, 0, 1, 3, 15},          {0}, {0}, MPKIND__OFFS_L_HALF, 2,  s4x4,   s_2x3,     0, 0};

static map_thing map_rh_s1x4_3         = {{11, 10, 8, 9},                 {2, 3, 5, 4},                   {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s2x6,   s1x4,      0, 1};
static map_thing map_rh_s2x2_2         = {{0, 1, 11, 10},                 {5, 4, 6, 7},                   {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s3x4,   s2x2,      0, 0};
static map_thing map_rh_s2x2_3         = {{1, 11, 10, 0},                 {4, 6, 7, 5},                   {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s3x4,   s2x2,      1, 0};
static map_thing map_rh_s2x4_0         = {{2, 3, 4, 5, 8, 9, 10, 11},     {0},                            {0}, {0}, MPKIND__OFFS_R_HALF, 1,  s2x6,   s2x4,      0, 0};
static map_thing map_rh_s2x4_1         = {{0, 1, 5, 4, 6, 7, 11, 10},     {0},                            {0}, {0}, MPKIND__OFFS_R_HALF, 1,  s3x4,   s2x4,      0, 1};
static map_thing map_rh_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_R_HALF, 1,  s1x8,   s1x8,      0, 0};
       map_thing map_rh_s2x3_3         = {{15, 11, 6, 8, 9, 10},          {0, 1, 2, 7, 3, 14},            {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s4x4,   s_2x3,     1, 1};
       map_thing map_rh_s2x3_2         = {{12, 13, 14, 3, 15, 10},        {11, 7, 2, 4, 5, 6},            {0}, {0}, MPKIND__OFFS_R_HALF, 2,  s4x4,   s_2x3,     0, 0};

static map_thing map_lf_s1x4_3         = {{11, 10, 8, 9},                 {0, 1, 3, 2},                   {0}, {0}, MPKIND__OFFS_L_FULL, 2,  s2x8,   s1x4,      0, 1};
static map_thing map_lf_s2x2_2         = {{9, 11, 6, 8},                  {14, 0, 1, 3},                  {0}, {0}, MPKIND__OFFS_L_FULL, 2,  s4x4,   s2x2,      0, 0};
static map_thing map_lf_s2x2_3         = {{11, 6, 8, 9},                  {0, 1, 3, 14},                  {0}, {0}, MPKIND__OFFS_L_FULL, 2,  s4x4,   s2x2,      1, 0};
static map_thing map_lf_s2x4_0         = {{0, 1, 2, 3, 8, 9, 10, 11},     {0},                            {0}, {0}, MPKIND__OFFS_L_FULL, 1,  s2x8,   s2x4,      0, 0};
static map_thing map_lf_s2x4_1         = {{13, 15, 2, 4, 5, 7, 10, 12},   {0},                            {0}, {0}, MPKIND__OFFS_L_FULL, 1,  s4x4,   s2x4,      1, 0};
static map_thing map_lf_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_L_FULL, 1,  s1x8,   s1x8,      0, 0};

static map_thing map_rf_s1x4_3         = {{15, 14, 12, 13},               {4, 5, 7, 6},                   {0}, {0}, MPKIND__OFFS_R_FULL, 2,  s2x8,   s1x4,      0, 1};
static map_thing map_rf_s2x2_2         = {{12, 13, 15, 10},               {7, 2, 4, 5},                   {0}, {0}, MPKIND__OFFS_R_FULL, 2,  s4x4,   s2x2,      0, 0};
static map_thing map_rf_s2x2_3         = {{13, 15, 10, 12},               {2, 4, 5, 7},                   {0}, {0}, MPKIND__OFFS_R_FULL, 2,  s4x4,   s2x2,      1, 0};
static map_thing map_rf_s2x4_0         = {{4, 5, 6, 7, 12, 13, 14, 15},   {0},                            {0}, {0}, MPKIND__OFFS_R_FULL, 1,  s2x8,   s2x4,      0, 0};
static map_thing map_rf_s2x4_1         = {{0, 1, 11, 6, 8, 9, 3, 14},     {0},                            {0}, {0}, MPKIND__OFFS_R_FULL, 1,  s4x4,   s2x4,      1, 0};
static map_thing map_rf_s1x8_0         = {{0, 1, 2, 3, 4, 5, 6, 7},       {0},                            {1}, {0}, MPKIND__OFFS_R_FULL, 1,  s1x8,   s1x8,      0, 0};
       map_thing map_lf_s2x4_r         = {{9, 11, 14, 0, 1, 3, 6, 8},     {0},                            {0}, {0}, MPKIND__OFFS_L_FULL, 1,  s4x4,   s2x4,      0, 1};
       map_thing map_rf_s2x4_r         = {{12, 13, 7, 2, 4, 5, 15, 10},   {0},                            {0}, {0}, MPKIND__OFFS_R_FULL, 1,  s4x4,   s2x4,      0, 1};

static map_thing map_blob_1x4a = {{13, 10, 6, 8},                {15, 17, 3, 5},            {18, 20, 1, 22},   {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 0, 1};
static map_thing map_blob_1x4b = {{19, 16, 12, 14},              {21, 23, 9, 11},           {0, 2, 7, 4},      {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 0, 1};
static map_thing map_blob_1x4c = {{1, 22, 18, 20},               {3, 5, 15, 17},            {6, 8, 13, 10},    {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 1, 0};
static map_thing map_blob_1x4d = {{19, 16, 12, 14},              {21, 23, 9, 11},           {0, 2, 7, 4},      {0}, MPKIND__NONE,        3,  s_bigblob,   s1x4, 1, 0};
static map_thing map_wblob_1x4a        = {{15, 17, 5, 3, 6, 8, 10, 13},   {18, 20, 22, 1, 3, 5, 17, 15},  {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 0, 1};
static map_thing map_wblob_1x4b        = {{21, 23, 11, 9, 12, 14, 16, 19},{0, 2, 4, 7, 9, 11, 23, 21},    {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 0, 1};
static map_thing map_wblob_1x4c        = {{3, 5, 17, 15, 18, 20, 22, 1},  {6, 8, 10, 13, 15, 17, 5, 3},   {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 1, 0};
static map_thing map_wblob_1x4d        = {{21, 23, 11, 9, 12, 14, 16, 19},{0, 2, 4, 7, 9, 11, 23, 21},    {0}, {0}, MPKIND__NONE,        2,  s_bigblob,   s2x4, 1, 0};

       map_thing map_dmd_1x1  = {{0},                  {1},                  {2},                {3},               MPKIND__NONE,        4,  sdmd,  s_1x1,      0, 0};

static map_thing map_4quads1  = {{0, 2},               {4, 6},               {10, 8},            {14, 12},          MPKIND__4_QUADRANTS, 4,  s_c1phan,    s_1x2,0, 0};
static map_thing map_4quads2  = {{1, 3},               {7, 5},               {11, 9},            {13, 15},          MPKIND__4_QUADRANTS, 4,  s_c1phan,    s_1x2,1, 0};

static map_thing map_dmd1     = {{0, 2},               {1, 3},               {0},                {0},               MPKIND__DMD_STUFF,   2,  sdmd,        s_1x2,0, 0};
static map_thing map_dmd2     = {{1, 3},               {0, 2},               {0},                {0},               MPKIND__DMD_STUFF,   2,  sdmd,        s_1x2,1, 0};

       map_thing *maps_3diag[4] = {&map_blob_1x4c, &map_blob_1x4a, &map_blob_1x4d, &map_blob_1x4b};
       map_thing *maps_3diagwk[4] = {&map_wblob_1x4a, &map_wblob_1x4c, &map_wblob_1x4b, &map_wblob_1x4d};
       
/* Beware!!  These items must be keyed to definition of "mpkind" in sd.h as follows:
                    split
                    |        removed
                    |        |        overlap
                    |        |        |        intlk
                    |        |        |        |        l_half
                    |        |        |        |        |        r_half
                    |        |        |        |        |        |        l_full
                    |        |        |        |        |        |        |        r_full
                    |        |        |        |        |        |        |        |        o_spots
                    |        |        |        |        |        |        |        |        |        x_spots
                    |        |        |        |        |        |        |        |        |        |        4quads
                    |        |        |        |        |        |        |        |        |        |        |        diamond
                    |        |        |        |        |        |        |        |        |        |        |        |
                    |        |        |        |        |        |        |        |        |        |        |        | */
static map_hunk mm_1x8_1 = {{0, 0},
                    {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_lh_s1x8_0, 0},
                                                                 {&map_rh_s1x8_0, 0},
                                                                          {&map_lf_s1x8_0, 0},
                                                                                   {&map_rf_s1x8_0, 0},
                                                                                            {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_2x4_1 = {{0, 0},
                    {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_lh_s2x4_0, &map_lh_s2x4_1},
                                                                 {&map_rh_s2x4_0, &map_rh_s2x4_1},
                                                                          {&map_lf_s2x4_0, &map_lf_s2x4_1},
                                                                                   {&map_rf_s2x4_0, &map_rf_s2x4_1},
                                                                                            {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x1_2 = {{0, 0},
                    {&map_1x2_1x1, &map_1x2_1x1},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x2_2 = {{0, 0},
                    {&map_1x4_1x2, &map_2x2h},
                             {&map_1x2_rmv, &map_1x2_rmvr},
                                      {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_dmd1, &map_dmd2}};
       
static map_hunk mm_2x2_2 = {{0, 0},
                    {&map_2x4_2x2, &map_2x4_2x2r},
                             {&map_2x2_rmv, 0},
                                      {0, 0},  {0, 0},  {&map_lh_s2x2_2, &map_lh_s2x2_3},
                                                                 {&map_rh_s2x2_2, &map_rh_s2x2_3},
                                                                          {&map_lf_s2x2_2, &map_lf_s2x2_3},
                                                                                   {&map_rf_s2x2_2, &map_rf_s2x2_3},
                                                                                            {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_dmd_2 = {{0, 0},
                    {&map_ptp_dmd, &map_qtg_dmd},
                             {&map_dmd_rmv, &map_dmd_rmvr},
                                      {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_tgl_2 = {{0, 0},
                    {0, &map_s6_trngl},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_qtg_2 = {{0, 0},
                    {&map_hv_qtg_2, 0},
                             {0, 0},  {&map_ov_qtag_0, 0},
                                               {&map_intlk_phantom_dmd, 0},
                                                        {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x3_2 = {{0, 0},
                    {&map_1x6_1x3, &map_2x3_1x3},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x4_2 = {{0, 0},
                    {&map_1x8_1x4, &map_2x4_1x4},
                             {&map_1x4_rmv, &map_1x4_rmvr},
                                      {0, 0},  {0, 0},  {0, &map_lh_s1x4_3},
                                                                 {0, &map_rh_s1x4_3},
                                                                          {0, &map_lf_s1x4_3},
                                                                                   {0, &map_rf_s1x4_3},
                                                                                            {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x8_2 = {{0, 0},
                    {&map_1x16_1x8, &map_2x8_1x8},
                             {0, 0},  {&map_ov_s1x8_0, 0},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

static map_hunk mm_1x8_3 = {{0, 0},
                    {0, 0},  {0, 0},  {&map_ov_s1x8_1, 0},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

static map_hunk mm_2x4_2 = {{0, 0},
                    {&map_hv_2x4_2, &map_4x4_2x4v},
                             {0, 0},  {&map_ov_2x4_0, &map_ov_2x4_1},
                                               {&map_intlk_phantom_box, &map_4x4_2x4intv},
                                                        {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_o_s2x4_2, &map_o_s2x4_3},
                                                                                                     {&map_x_s2x4_2, &map_x_s2x4_3},
                                                                                                              {0, 0},  {0, 0}};
       
static map_hunk mm_2x3_2 = {{0, 0},
                    {&map_2x6_2x3, &map_3x4_2x3},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

static map_hunk mm_2x3_3 = {{0, 0},
                    {0, 0},  {0, 0},  {0, &map_ov_2x3_3},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};

static map_hunk mm_1x6_2 = {{0, 0},
                    {0, &map_2x6_1x6},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x2_3 = {{0, 0},
                    {&map_1x6_1x2, &map_2x3_1x2},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_2x2_3 = {{0, 0},
                    {&map_2x6_2x2, &map_2x6_2x2r},
                             {0, 0},  {&map_ov_s2x2_2, 0},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_dmd_3 = {{0, 0},
                    {0, &map_3dmd_dmd},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_qtg_3 = {{0, 0},
                    {0, 0},  {0, 0},  {&map_ov_qtag_2, 0},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x4_3 = {{0, 0},
                    {&map_1x12_1x4, &map_3x4_1x4},
                             {0, 0},  {&map_ov_1x4_2, &map_ov_1x4_3},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_2x4_3 = {{0, 0},
                    {0, &map_4x6_2x4},
                             {0, 0},  {&map_ov_2x4_2, &map_ov_2x4_3},
                                               {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x2_4 = {{0, 0},
                    {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {&map_4quads1, &map_4quads2},
                                                                                                                       {0, 0}};
       
static map_hunk mm_2x2_4 = {{0, 0},
                    {&map_2x8_2x2, 0},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_dmd_4 = {{0, 0},
                    {0, &map_4dmd_dmd},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};
       
static map_hunk mm_1x4_4 = {{0, 0},
                    {&map_1x16_1x4, &map_4x4_1x4},
                             {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0},  {0, 0}};



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
   {0,         0,         0,         0},          /* s_concstar */
   {0,         0,         0,         0},          /* s_concstar12 */
   {0,         0,         0,         0},          /* s_concstar16 */
   {0,         0,         0,         0},          /* s_galaxy */
   {0,         0,         0,         0},          /* s4x6 */
   {0,         0,         0,         0},          /* s_thar */
   {0,         0,         0,         0}};         /* s_normal_concentric */



concept_descriptor special_magic = {"MAGIC DIAMOND,", concept_magic, l_dontshow, {0, 1}};
concept_descriptor special_interlocked = {"INTERLOCKED DIAMOND,", concept_interlocked, l_dontshow, {0, 1}};


concept_descriptor mark_end_of_list = {"????", marker_end_of_list, l_dontshow};
concept_descriptor marker_decline = {"decline???", concept_mod_declined, l_dontshow};
concept_descriptor marker_concept_mod = {">>MODIFIED BY<<", concept_another_call_next_mod, l_dontshow};
concept_descriptor marker_concept_modreact = {">>REACTION MODIFIED BY<<", concept_another_call_next_modreact, l_dontshow};
concept_descriptor marker_concept_modtag = {">>TAG ENDING MODIFIED BY<<", concept_another_call_next_modtag, l_dontshow};
concept_descriptor marker_concept_force = {">>FORCIBLY MODIFIED BY<<", concept_another_call_next_force, l_dontshow};
concept_descriptor marker_concept_comment = {"{{COMMENT, l_dontshow}, l_dontshow}", concept_comment, l_dontshow};

callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];
int number_of_calls[NUM_CALL_LIST_KINDS];
level calling_level;


/* **** BEWARE!!!!  Four concepts must be at positions indicated in "nice_setup_concept" (below) */

concept_descriptor concept_descriptor_table[] = {

/* phantom concepts */

#define pp__1 35
   {"SPLIT PHANTOM LINES",                   concept_do_phantom_2x4,        l_c3a, {&(map_split_f), phantest_impossible, 1}},
   {"INTERLOCKED PHANTOM LINES",             concept_do_phantom_2x4,        l_c3, {&(map_intlk_f), phantest_impossible, 1}},
   {"PHANTOM LINES",                         concept_do_phantom_2x4,        l_c3a, {&(map_full_f), phantest_first_or_both, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"12 MATRIX SPLIT PHANTOM LINES",         concept_do_phantom_2x3,        l_c3a, {&(map_3x4_2x3), phantest_impossible, 1}},
   {"12 MATRIX INTERLOCKED PHANTOM LINES",   concept_do_phantom_2x3,        l_c3, {&(map_3x4_2x3_intlk), phantest_impossible, 1}},
   {"12 MATRIX PHANTOM LINES",               concept_do_phantom_2x3,        l_c3a, {&(map_3x4_2x3_conc), phantest_first_or_both, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED LINES",                         concept_divided_2x4,           l_c4, {&(map_hv_2x4_2), phantest_impossible, 1}},
   {"12 MATRIX DIVIDED LINES",               concept_divided_2x3,           l_c4, {&(map_2x6_2x3), phantest_impossible, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TWO PHANTOM LINES OF 8",                concept_do_phantom_1x8,        l_c3, {0, phantest_impossible, TRUE, 1}},
   {"TWO PHANTOM LINES OF 6",                concept_do_phantom_1x6,        l_c3, {0, phantest_impossible, TRUE, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE LINES",                          concept_triple_lines,          l_c2, {0, 1}},
   {"TRIPLE LINES WORKING TOGETHER",         concept_triple_lines_together, l_c3x, {0, 8, 1}},
   {"TRIPLE LINES WORKING APART",            concept_triple_lines_together, l_c3x, {0, 9, 1}},
   {"TRIPLE LINES WORKING FORWARD",          concept_triple_lines_together, l_c3x, {0, 0, 1}},
   {"TRIPLE LINES WORKING BACKWARD",         concept_triple_lines_together, l_c3x, {0, 2, 1}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL LINES",                 concept_triple_diag,           l_c4, {0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING FORWARD", concept_triple_diag_together,  l_c4, {0, 0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING BACKWARD",concept_triple_diag_together,  l_c4, {0, 2, 1}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN LINES",                     concept_triple_twin,           l_c4a, {&(map_4x6_2x4), 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE LINES",                       concept_quad_lines,            l_c4a, {0, 1}},
   {"QUADRUPLE LINES WORKING TOGETHER",      concept_quad_lines_together,   l_c4a, {0, 8, 1}},
   {"QUADRUPLE LINES WORKING APART",         concept_quad_lines_together,   l_c4a, {0, 9, 1}},
   {"QUADRUPLE LINES WORKING FORWARD",       concept_quad_lines_together,   l_c4a, {0, 0, 1}},
   {"QUADRUPLE LINES WORKING BACKWARD",      concept_quad_lines_together,   l_c4a, {0, 2, 1}},
/* -------- column break -------- */
#define pp__2 37
   {"SPLIT PHANTOM COLUMNS",                 concept_do_phantom_2x4,        l_c3a, {&(map_split_f), phantest_impossible, 0}},
   {"INTERLOCKED PHANTOM COLUMNS",           concept_do_phantom_2x4,        l_c3, {&(map_intlk_f), phantest_impossible, 0}},
   {"PHANTOM COLUMNS",                       concept_do_phantom_2x4,        l_c3a, {&(map_full_f), phantest_first_or_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"12 MATRIX SPLIT PHANTOM COLUMNS",       concept_do_phantom_2x3,        l_c3a, {&(map_3x4_2x3), phantest_impossible, 0}},
   {"12 MATRIX INTERLOCKED PHANTOM COLUMNS", concept_do_phantom_2x3,        l_c3, {&(map_3x4_2x3_intlk), phantest_impossible, 0}},
   {"12 MATRIX PHANTOM COLUMNS",             concept_do_phantom_2x3,        l_c3a, {&(map_3x4_2x3_conc), phantest_first_or_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED COLUMNS",                       concept_divided_2x4,           l_c4, {&(map_hv_2x4_2), phantest_impossible, 0}},
   {"12 MATRIX DIVIDED COLUMNS",             concept_divided_2x3,           l_c4, {&(map_2x6_2x3), phantest_impossible, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TWO PHANTOM COLUMNS OF 8",              concept_do_phantom_1x8,        l_c3, {0, phantest_impossible, TRUE, 0}},
   {"TWO PHANTOM COLUMNS OF 6",              concept_do_phantom_1x6,        l_c3, {0, phantest_impossible, TRUE, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE COLUMNS",                        concept_triple_lines,          l_c3x, {0, 0}},
   {"TRIPLE COLUMNS WORKING TOGETHER",       concept_triple_lines_together, l_c3x, {0, 8, 0}},
   {"TRIPLE COLUMNS WORKING APART",          concept_triple_lines_together, l_c3x, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE COLUMNS WORKING RIGHT",          concept_triple_lines_together, l_c3x, {0, 3, 0}},
   {"TRIPLE COLUMNS WORKING LEFT",           concept_triple_lines_together, l_c3x, {0, 1, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL COLUMNS",               concept_triple_diag,           l_c4, {0, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL COLUMNS WORKING RIGHT", concept_triple_diag_together,  l_c4, {0, 3, 0}},
   {"TRIPLE DIAGONAL COLUMNS WORKING LEFT",  concept_triple_diag_together,  l_c4, {0, 1, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN COLUMNS",                   concept_triple_twin,           l_c4a, {&(map_4x6_2x4), 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE COLUMNS",                     concept_quad_lines,            l_c4a, {0, 0}},
   {"QUADRUPLE COLUMNS WORKING TOGETHER",    concept_quad_lines_together,   l_c4a, {0, 8, 0}},
   {"QUADRUPLE COLUMNS WORKING APART",       concept_quad_lines_together,   l_c4a, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE COLUMNS WORKING RIGHT",       concept_quad_lines_together,   l_c4a, {0, 3, 0}},
   {"QUADRUPLE COLUMNS WORKING LEFT",        concept_quad_lines_together,   l_c4a, {0, 1, 0}},
/* -------- column break -------- */
#define pp__3 37
   {"SPLIT PHANTOM BOXES",                   concept_do_phantom_boxes,      l_c4a, {&(map_hv_2x4_2), phantest_impossible}},
   {"INTERLOCKED PHANTOM BOXES",             concept_do_phantom_boxes,      l_c4, {&(map_intlk_phantom_box), phantest_impossible}},
   {"PHANTOM BOXES",                         concept_do_phantom_boxes,      l_c4a, {&(map_phantom_box), phantest_first_or_both}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE BOXES",                          concept_triple_boxes,          l_c2},
   {"TRIPLE BOXES WORKING TOGETHER",         concept_triple_boxes_together, l_c3x, {0, 8, 0}},
   {"TRIPLE BOXES WORKING APART",            concept_triple_boxes_together, l_c3x, {0, 9, 0}},
   {"TRIPLE BOXES WORKING FORWARD",          concept_triple_boxes_together, l_c3x, {0, 0, 0}},
   {"TRIPLE BOXES WORKING BACKWARD",         concept_triple_boxes_together, l_c3x, {0, 2, 0}},
   {"TRIPLE BOXES WORKING RIGHT",            concept_triple_boxes_together, l_c3x, {0, 3, 0}},
   {"TRIPLE BOXES WORKING LEFT",             concept_triple_boxes_together, l_c3x, {0, 1, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE BOXES",                       concept_quad_boxes,            l_c4a},
   {"QUADRUPLE BOXES WORKING TOGETHER",      concept_quad_boxes_together,   l_c4a, {0, 8, 0}},
   {"QUADRUPLE BOXES WORKING APART",         concept_quad_boxes_together,   l_c4a, {0, 9, 0}},
   {"QUADRUPLE BOXES WORKING FORWARD",       concept_quad_boxes_together,   l_c4a, {0, 0, 0}},
   {"QUADRUPLE BOXES WORKING BACKWARD",      concept_quad_boxes_together,   l_c4a, {0, 2, 0}},
   {"QUADRUPLE BOXES WORKING RIGHT",         concept_quad_boxes_together,   l_c4a, {0, 3, 0}},
   {"QUADRUPLE BOXES WORKING LEFT",          concept_quad_boxes_together,   l_c4a, {0, 1, 0}},
/* -------- column break -------- */
#define pp__4 32
   {"SPLIT PHANTOM DIAMONDS",                concept_do_phantom_diamonds,   l_c3x, {&(map_hv_qtg_2), phantest_impossible}},
   {"INTERLOCKED PHANTOM DIAMONDS",          concept_do_phantom_diamonds,   l_c4, {&(map_intlk_phantom_dmd), phantest_impossible}},
   {"PHANTOM DIAMONDS",                      concept_do_phantom_diamonds,   l_c4, {&(map_phantom_dmd), phantest_first_or_both}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAMONDS",                       concept_triple_diamonds,          l_c3a},
   {"TRIPLE DIAMONDS WORKING TOGETHER",      concept_triple_diamonds_together, l_c3x},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE DIAMONDS",                    concept_quad_diamonds,          l_c4a},
   {"QUADRUPLE DIAMONDS WORKING TOGETHER",   concept_quad_diamonds_together, l_c4a},

/* tandem concepts */

#define tt__1 10
   {"AS COUPLES",                            concept_tandem,                l_a1,         {0, FALSE, 0, 0, 1}},
   {"TANDEM",                                concept_tandem,                l_c1,         {0, FALSE, 0, 0, 0}},
   {"SIAMESE",                               concept_tandem,                l_c2,         {0, FALSE, 0, 0, 2}},
   {"SKEW",                                  concept_tandem,                l_c2,         {0, FALSE, 0, 0, 3}},
   {"SO-AND-SO AS COUPLES",                  concept_some_are_tandem,       l_a1,         {0, TRUE,  0, 0, 1}},
   {"SO-AND-SO TANDEM",                      concept_some_are_tandem,       l_c1,         {0, TRUE,  0, 0, 0}},
   {"PHANTOM AS COUPLES",                    concept_phantom_tandem,        l_c4,         {0, FALSE, 0, 1, 1}},
   {"PHANTOM TANDEM",                        concept_phantom_tandem,        l_c4a,        {0, FALSE, 0, 1, 0}},
   {"GRUESOME AS COUPLES",                   concept_gruesome_tandem,       l_c4a,        {0, FALSE, 0, 2, 1}},
   {"GRUESOME TANDEM",                       concept_gruesome_tandem,       l_c4a,        {0, FALSE, 0, 2, 0}},
/* -------- column break -------- */
#define tt__2 10
   {"COUPLES TWOSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 1}},
   {"TANDEM TWOSOME",                        concept_tandem,                l_c3,         {0, FALSE, 1, 0, 0}},
   {"SIAMESE TWOSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 2}},
   {"SKEWSOME",                              concept_tandem,                l_c2,         {0, FALSE, 1, 0, 3}},
   {"SO-AND-SO COUPLES TWOSOME",             concept_some_are_tandem,       l_c3,         {0, TRUE,  1, 0, 1}},
   {"SO-AND-SO TANDEM TWOSOME",              concept_some_are_tandem,       l_c3,         {0, TRUE,  1, 0, 0}},
   {"PHANTOM COUPLES TWOSOME",               concept_phantom_tandem,        l_c3,         {0, FALSE, 1, 1, 1}},
   {"PHANTOM TANDEM TWOSOME",                concept_phantom_tandem,        l_c3,         {0, FALSE, 1, 1, 0}},
   {"GRUESOME TWOSOME",                      concept_gruesome_tandem,       l_c4a,        {0, FALSE, 1, 2, 1}},
   {"GRUESOME TANDEM TWOSOME",               concept_gruesome_tandem,       l_c4a,        {0, FALSE, 1, 2, 0}},
/* -------- column break -------- */
#define tt__3 10
   {"COUPLES N/4 TWOSOME",                   concept_frac_tandem,           l_a1,         {0, FALSE, 2, 0, 1}},
   {"TANDEM N/4 TWOSOME",                    concept_frac_tandem,           l_c1,         {0, FALSE, 2, 0, 0}},
   {"SIAMESE N/4 TWOSOME",                   concept_frac_tandem,           l_c2,         {0, FALSE, 2, 0, 2}},
   {"SKEW N/4 TWOSOME",                      concept_frac_tandem,           l_c2,         {0, FALSE, 2, 0, 3}},
   {"SO-AND-SO COUPLES N/4 TWOSOME",         concept_some_are_frac_tandem,  l_a1,         {0, TRUE,  2, 0, 1}},
   {"SO-AND-SO TANDEM N/4 TWOSOME",          concept_some_are_frac_tandem,  l_c1,         {0, TRUE,  2, 0, 0}},
   {"PHANTOM AS COUPLES N/4 TWOSOME",        concept_phantom_frac_tandem,   l_c4,         {0, FALSE, 2, 1, 1}},
   {"PHANTOM TANDEM N/4 TWOSOME",            concept_phantom_frac_tandem,   l_c4a,        {0, FALSE, 2, 1, 0}},
   {"GRUESOME AS COUPLES N/4 TWOSOME",       concept_gruesome_frac_tandem,  l_c4a,        {0, FALSE, 2, 2, 1}},
   {"GRUESOME TANDEM N/4 TWOSOME",           concept_gruesome_frac_tandem,  l_c4a,        {0, FALSE, 2, 2, 0}},
/* -------- column break -------- */
#define tt__4 10
   {"COUPLES TWOSOME N/4 SOLID",              concept_frac_tandem,          l_c3,         {0, FALSE, 3, 0, 1}},
   {"TANDEM TWOSOME N/4 SOLID",               concept_frac_tandem,          l_c3,         {0, FALSE, 3, 0, 0}},
   {"SIAMESE TWOSOME N/4 SOLID",              concept_frac_tandem,          l_c3,         {0, FALSE, 3, 0, 2}},
   {"SKEWSOME N/4 SOLID",                     concept_frac_tandem,          l_c2,         {0, FALSE, 3, 0, 3}},
   {"SO-AND-SO COUPLES TWOSOME N/4 SOLID",    concept_some_are_frac_tandem, l_c3,         {0, TRUE,  3, 0, 1}},
   {"SO-AND-SO TANDEM TWOSOME N/4 SOLID",     concept_some_are_frac_tandem, l_c3,         {0, TRUE,  3, 0, 0}},
   {"PHANTOM COUPLES TWOSOME N/4 SOLID",      concept_phantom_frac_tandem,  l_c3,         {0, FALSE, 3, 1, 1}},
   {"PHANTOM TANDEM TWOSOME N/4 SOLID",       concept_phantom_frac_tandem,  l_c3,         {0, FALSE, 3, 1, 0}},
   {"GRUESOME TWOSOME N/4 SOLID",             concept_gruesome_frac_tandem, l_c4a,        {0, FALSE, 3, 2, 1}},
   {"GRUESOME TANDEM TWOSOME N/4 SOLID",      concept_gruesome_frac_tandem, l_c4a,        {0, FALSE, 3, 2, 0}},

/* distorted concepts */

#define dd__1 10
   {"STAGGER",                               concept_do_phantom_2x4,        l_c2,         {&(map_stagger), phantest_only_one, 0}},
   {"STAIRSTEP COLUMNS",                     concept_do_phantom_2x4,        l_c4,         {&(map_stairst), phantest_only_one, 0}},
   {"LADDER COLUMNS",                        concept_do_phantom_2x4,        l_c4,         {&(map_ladder), phantest_only_one, 0}},
   {"OFFSET COLUMNS",                        concept_distorted,             l_c2,         {0, 0, disttest_offset}},
   {"\"Z\" COLUMNS",                         concept_distorted,             l_c3,         {0, 0, disttest_z}},
   {"DISTORTED COLUMNS",                     concept_distorted,             l_c3,         {0, 0, disttest_any}},
   {"DIAGONAL COLUMN",                       concept_single_diagonal,       l_c3,         {0, 0}},
   {"TWO DIAGONAL COLUMNS",                  concept_double_diagonal,       l_c3,         {0, 0}},
   {"\"O\"",                                 concept_do_phantom_2x4,        l_c1,         {&(map_o_s2x4_3), phantest_only_first_one, 0}},
   {"BUTTERFLY",                             concept_do_phantom_2x4,        l_c1,         {&(map_x_s2x4_3), phantest_only_second_one, 0}},
/* -------- column break -------- */
#define dd__2 8
   {"BIGBLOCK",                              concept_do_phantom_2x4,        l_c3a,        {&(map_stagger), phantest_only_one, 1}},
   {"STAIRSTEP LINES",                       concept_do_phantom_2x4,        l_c4,         {&(map_ladder), phantest_only_one, 1}},
   {"LADDER LINES",                          concept_do_phantom_2x4,        l_c4,         {&(map_stairst), phantest_only_one, 1}},
   {"OFFSET LINES",                          concept_distorted,             l_c2,         {0, 1, disttest_offset}},
   {"\"Z\" LINES",                           concept_distorted,             l_c3,         {0, 1, disttest_z}},
   {"DISTORTED LINES",                       concept_distorted,             l_c3,         {0, 1, disttest_any}},
   {"DIAGONAL LINE",                         concept_single_diagonal,       l_c3,         {0, 1}},
   {"TWO DIAGONAL LINES",                    concept_double_diagonal,       l_c3,         {0, 1}},
/* -------- column break -------- */
#define dd__3 9
   {"PHANTOM STAGGER COLUMNS",               concept_do_phantom_2x4,        l_c4,         {&(map_stagger), phantest_both, 0}},
   {"PHANTOM STAIRSTEP COLUMNS",             concept_do_phantom_2x4,        l_c4,         {&(map_stairst), phantest_both, 0}},
   {"PHANTOM LADDER COLUMNS",                concept_do_phantom_2x4,        l_c4,         {&(map_ladder), phantest_both, 0}},
   {"PHANTOM OFFSET COLUMNS",                concept_do_phantom_2x4,        l_c4a,        {&(map_offset), phantest_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"PHANTOM BUTTERFLY OR \"O\"",            concept_do_phantom_2x4,        l_c4a,        {&(map_but_o), phantest_both, 0}},
/* -------- column break -------- */
#define dd__4 4
   {"PHANTOM BIGBLOCK LINES",                concept_do_phantom_2x4,        l_c4,         {&(map_stagger), phantest_both, 1}},
   {"PHANTOM STAIRSTEP LINES",               concept_do_phantom_2x4,        l_c4,         {&(map_ladder), phantest_both, 1}},
   {"PHANTOM LADDER LINES",                  concept_do_phantom_2x4,        l_c4,         {&(map_stairst), phantest_both, 1}},
   {"PHANTOM OFFSET LINES",                  concept_do_phantom_2x4,        l_c4a,        {&(map_offset), phantest_both, 1}},

/* 4-person distorted concepts */

#define d4__1 6
   {"SPLIT",                                 concept_split,                 l_mainstream},
   {"ONCE REMOVED",                          concept_once_removed,          l_c2,         {0, 0}},
   {"ONCE REMOVED DIAMONDS",                 concept_once_removed,          l_c3,         {0, 1}},
   {"MAGIC",                                 concept_magic,                 l_c1},
   {"DIAGONAL",                              concept_do_both_boxes,         l_c3a,        {&(map_2x4_diagonal), 97, FALSE}},
   {"TRAPEZOID",                             concept_do_both_boxes,         l_c3a,        {&(map_2x4_trapezoid), 97, FALSE}},
/* -------- column break -------- */
#define d4__2 5
   {"INTERLOCKED PARALLELOGRAM",             concept_do_both_boxes,         l_c3x,        {&(map_2x4_int_pgram), 97, TRUE}},
   {"INTERLOCKED BOXES",                     concept_misc_distort,          l_c3x,        {0, 3, 0}},
   {"TWIN PARALLELOGRAMS",                   concept_misc_distort,          l_c3x,        {0, 2, 0}},
   {"EACH \"Z\"",                            concept_misc_distort,          l_c3a,        {0, 0, 0}},
   {"INTERLOCKED \"Z's\"",                   concept_misc_distort,          l_c3a,        {0, 0, 8}},
/* -------- column break -------- */
#define d4__3 6
   {"JAY",                                   concept_misc_distort,          l_c3a,        {0, 1, 0}},
   {"BACK-TO-FRONT JAY",                     concept_misc_distort,          l_c3a,        {0, 1, 8}},
   {"BACK-TO-BACK JAY",                      concept_misc_distort,          l_c3a,        {0, 1, 16}},
   {"FACING PARALLELOGRAM",                  concept_misc_distort,          l_c3x,        {0, 4, 0}},
   {"BACK-TO-FRONT PARALLELOGRAM",           concept_misc_distort,          l_c3x,        {0, 4, 8}},
   {"BACK-TO-BACK PARALLELOGRAM",            concept_misc_distort,          l_c3x,        {0, 4, 16}},
/* -------- column break -------- */
#define d4__4 4
   {"IN YOUR BLOCKS",                        concept_do_phantom_2x2,        l_c1,         {&(map_blocks), phantest_2x2_only_two}},
   {"4 PHANTOM INTERLOCKED BLOCKS",          concept_do_phantom_2x2,        l_c4a,        {&(map_blocks), phantest_2x2_both}},
   {"TRIANGULAR BOXES",                      concept_do_phantom_2x2,        l_c4,         {&(map_trglbox), phantest_2x2_only_two}},
   {"4 PHANTOM TRIANGULAR BOXES",            concept_do_phantom_2x2,        l_c4,         {&(map_trglbox), phantest_2x2_both}},

/* Miscellaneous concepts */

#define mm__1 11
   {"LEFT",                                  concept_left,                  l_mainstream},
   {"REVERSE",                               concept_reverse,               l_mainstream},
   {"CROSS",                                 concept_cross,                 l_mainstream},
   {"SINGLE",                                concept_single,                l_mainstream},
   {"GRAND",                                 concept_grand,                 l_plus},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIANGLE",                              concept_triangle,              l_c1},
   {"DIAMOND",                               concept_diamond,               l_c3a},
   {"INTERLOCKED",                           concept_interlocked,           l_c1},
   {"12 MATRIX",                             concept_12_matrix,             l_c3a},
   {"FUNNY",                                 concept_funny,                 l_c2},
/* -------- column break -------- */
#define mm__2 12
   {"PARALLELOGRAM",                         concept_parallelogram,         l_c2},
   {"PHANTOM",                               concept_c1_phantom,            l_c1},
   {"SO-AND-SO ARE STANDARD",                concept_standard,              l_c4a,        {0, 0}},
   {"STABLE",                                concept_stable,                l_c3a,        {0, FALSE, FALSE}},
   {"SO-AND-SO ARE STABLE",                  concept_so_and_so_stable,      l_c3a,        {0, TRUE,  FALSE}},
   {"N/4 STABLE",                            concept_frac_stable,           l_c4,         {0, FALSE, TRUE}},
   {"SO-AND-SO ARE N/4 STABLE",              concept_so_and_so_frac_stable, l_c4,         {0, TRUE,  TRUE}},
   {"TRACE",                                 concept_trace,                 l_c3x},
   {"(old) STRETCH",                         concept_old_stretch,           l_c1},
   {"STRETCHED SETUP",                       concept_new_stretch,           l_c2},
   {"FERRIS",                                concept_ferris,                l_c3x,        {0, 0}},
   {"RELEASE",                               concept_ferris,                l_c3a,        {0, 1}},
/* -------- column break -------- */
#define mm__3 33
   {"CENTERS",                               concept_centers_or_ends,       l_mainstream, {0, 0}},
   {"ENDS",                                  concept_centers_or_ends,       l_mainstream, {0, 1}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,      l_mainstream, {0, 0}},
   {"CENTER SIX",                            concept_centers_or_ends,       l_mainstream, {0, 2}},
   {"OUTER SIX",                             concept_centers_or_ends,       l_mainstream, {0, 5}},
   {"CENTER TWO",                            concept_centers_or_ends,       l_mainstream, {0, 4}},
   {"OUTER TWO",                             concept_centers_or_ends,       l_mainstream, {0, 3}},
   {"CENTER 6/OUTER 2",                      concept_centers_and_ends,      l_mainstream, {0, 2}},
   {"CENTER 2/OUTER 6",                      concept_centers_and_ends,      l_mainstream, {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"CHECKPOINT",                            concept_checkpoint,            l_c2,         {0, 0}},
   {"REVERSE CHECKPOINT",                    concept_checkpoint,            l_c4,         {0, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"CHECKERBOARD",                          concept_checkerboard,          l_c3a,        {0, s1x4}},
   {"CHECKERBOX",                            concept_checkerboard,          l_c3a,        {0, s2x2}},
   {"CHECKERDIAMOND",                        concept_checkerboard,          l_c3a,        {0, sdmd}},
         {"", concept_comment, l_nonexistent_concept},
   {"two calls in succession",               concept_sequential,            l_mainstream},
         {"", concept_comment, l_nonexistent_concept},
   {"ON YOUR OWN",                           concept_on_your_own,           l_c3x},
   {"SO-AND-SO ONLY",                        concept_so_and_so_only,        l_mainstream, {0, 0}},
   {"OWN THE SO-AND-SO",                     concept_some_vs_others,        l_c3x,        {0, 1}},
   {"SO-AND-SO START",                       concept_so_and_so_begin,       l_c2,         {0, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"RANDOM",                                concept_meta,                  l_c3x,        {0, 0}},
   {"REVERSE RANDOM",                        concept_meta,                  l_c3x,        {0, 1}},
   {"PIECEWISE",                             concept_meta,                  l_c3x,        {0, 2}},
   {"FRACTIONAL",                            concept_fractional,            l_c2,         {0, 0}},
   {"INTERLACE",                             concept_interlace,             l_c3x},
   {"START <concept>",                       concept_meta,                  l_c2,         {0, 3}},
   {"DO THE Nth PART <concept>",             concept_nth_part,              l_c2,         {0, 0}},
   {"REPLACE THE Nth PART OF X WITH Y",      concept_replace_nth_part,      l_c2,         {0, 0}},
   {"INTERRUPT X AFTER THE Nth PART WITH Y", concept_replace_nth_part,      l_c2,         {0, 1}},
/* -------- column break -------- */
#define mm__4 32
   {"IN POINT TRIANGLE",                     concept_randomtrngl,           l_c1,         {0, 1}},
   {"OUT POINT TRIANGLE",                    concept_randomtrngl,           l_c1,         {0, 0}},
   {"INSIDE TRIANGLES",                      concept_randomtrngl,           l_c1,         {0, 2}},
   {"OUTSIDE TRIANGLES",                     concept_randomtrngl,           l_c1,         {0, 3}},
   {"TALL 6",                                concept_randomtrngl,           l_c1,         {0, 4}},
   {"SHORT 6",                               concept_randomtrngl,           l_c1,         {0, 5}},
   {"WAVE-BASE TRIANGLES",                   concept_randomtrngl,           l_c1,         {0, 6}},
   {"TANDEM-BASE TRIANGLES",                 concept_randomtrngl,           l_c1,         {0, 7}},
   {"SO-AND-SO-BASED TRIANGLE",              concept_selbasedtrngl,         l_c1,         {0, 20}},
         {"", concept_comment, l_nonexistent_concept},
   {"CONCENTRIC",                            concept_concentric,            l_c1,         {0, schema_concentric}},
   {"CROSS CONCENTRIC",                      concept_concentric,            l_c2,         {0, schema_cross_concentric}},
   {"SINGLE CONCENTRIC",                     concept_single_concentric,     l_c4,         {0, schema_single_concentric}},
   {"SINGLE CROSS CONCENTRIC",               concept_single_concentric,     l_c4,         {0, schema_single_cross_concentric}},
         {"", concept_comment, l_nonexistent_concept},
   {"GRAND WORKING FORWARD",                 concept_grand_working,         l_c3x,        {0, 0}},
   {"GRAND WORKING BACKWARD",                concept_grand_working,         l_c3x,        {0, 2}},
   {"GRAND WORKING LEFT",                    concept_grand_working,         l_c3x,        {0, 1}},
   {"GRAND WORKING RIGHT",                   concept_grand_working,         l_c3x,        {0, 3}},
   {"GRAND WORKING TOGETHER",                concept_grand_working,         l_c3x,        {0, 8}},
   {"GRAND WORKING APART",                   concept_grand_working,         l_c3x,        {0, 9}},
         {"", concept_comment, l_nonexistent_concept},
   {"DOUBLE-OFFSET 1/4-TAG",                 concept_double_offset,         l_c4,         {0, 0}},
   {"DOUBLE-OFFSET 3/4-TAG",                 concept_double_offset,         l_c4,         {0, 1}},
   {"DOUBLE-OFFSET THING",                   concept_double_offset,         l_c4,         {0, 2}},
   {"DOUBLE-OFFSET DIAMONDS",                concept_double_offset,         l_c4,         {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"INRIGGER",                              concept_rigger,                l_c3x,        {0, 2}},
   {"OUTRIGGER",                             concept_rigger,                l_c3x,        {0, 0}},
   {"LEFTRIGGER",                            concept_rigger,                l_c3x,        {0, 1}},
   {"RIGHTRIGGER",                           concept_rigger,                l_c3x,        {0, 3}},
   {"[call]-RIGGER",                         concept_callrigger,            l_c3x},

/* general concepts */

   {"CENTERS",                               concept_centers_or_ends,       l_mainstream, {0, 0}},
   {"ENDS",                                  concept_centers_or_ends,       l_mainstream, {0, 1}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,      l_mainstream, {0, 0}},
   {"LEFT",                                  concept_left,                  l_mainstream},
   {"REVERSE",                               concept_reverse,               l_mainstream},
   {"SPLIT",                                 concept_split,                 l_mainstream},
   {"CROSS",                                 concept_cross,                 l_mainstream},
   {"GRAND",                                 concept_grand,                 l_plus},
   {"MAGIC",                                 concept_magic,                 l_c1},
   {"INTERLOCKED",                           concept_interlocked,           l_c1},
   {"12 MATRIX",                             concept_12_matrix,             l_c3a},
   {"FUNNY",                                 concept_funny,                 l_c2},
   {"SINGLE",                                concept_single,                l_mainstream},
   {"CONCENTRIC",                            concept_concentric,            l_c1,         {0, schema_concentric}},
   {"AS COUPLES",                            concept_tandem,                l_a1,         {0, FALSE, FALSE, 0, 1}},
   {"TANDEM",                                concept_tandem,                l_c1,         {0, FALSE, FALSE, 0, 0}},
   {"???",                                   marker_end_of_list}};

/* **** BEWARE!!!!  These are indices into "concept_descriptor_table"
   (just above) for SPLIT PHANTOM LINES, SPLIT PHANTOM COLUMNS,
   INTERLOCKED PHANTOM LINES, and INTERLOCKED PHANTOM COLUMNS.
   We use the fact that they are the first two items in the
   first two columns. */

int nice_setup_concept[] = {0, pp__1, 1, pp__1+1};

/* **** BEWARE!!!!  These things are keyed to "concept_descriptor_table"
   (above) and tell how to divide things into the various menus and popups.
   BE CAREFUL!! */

#define pp__z (pp__1+pp__2+pp__3+pp__4)
#define tt__z (tt__1+tt__2+tt__3+tt__4)
#define dd__z (dd__1+dd__2+dd__3+dd__4)
#define d4__z (d4__1+d4__2+d4__3+d4__4)

static int phantom_sizes[]       = {pp__1, pp__2, pp__3, pp__4, -1};
static int tandem_sizes[]        = {tt__1, tt__2, tt__3, tt__4, -1};
static int distort_sizes[]       = {dd__1, dd__2, dd__3, dd__4, -1};
static int dist4_sizes[]         = {d4__1, d4__2, d4__3, d4__4, -1};
static int misc_c_sizes[]        = {mm__1, mm__2, mm__3, mm__4, -1};

static int phantom_offsets[] = {0,
                                0 + pp__1,
                                0 + pp__1 + pp__2,
                                0 + pp__1 + pp__2 + pp__3};

static int tandem_offsets[]  = {0 + pp__z,
                                0 + pp__z + tt__1,
                                0 + pp__z + tt__1 + tt__2,
                                0 + pp__z + tt__1 + tt__2 + tt__3};

static int distort_offsets[] = {0 + pp__z + tt__z,
                                0 + pp__z + tt__z + dd__1,
                                0 + pp__z + tt__z + dd__1 + dd__2,
                                0 + pp__z + tt__z + dd__1 + dd__2 + dd__3};

static int dist4_offsets[]   = {0 + pp__z + tt__z + dd__z,
                                0 + pp__z + tt__z + dd__z + d4__1,
                                0 + pp__z + tt__z + dd__z + d4__1 + d4__2,
                                0 + pp__z + tt__z + dd__z + d4__1 + d4__2 + d4__3};

static int misc_c_offsets[]  = {0 + pp__z + tt__z + dd__z + d4__z,
                                0 + pp__z + tt__z + dd__z + d4__z + mm__1,
                                0 + pp__z + tt__z + dd__z + d4__z + mm__1 + mm__2,
                                0 + pp__z + tt__z + dd__z + d4__z + mm__1 + mm__2 + mm__3};
int general_concept_offset   =  0 + pp__z + tt__z + dd__z + d4__z + mm__1 + mm__2 + mm__3 + mm__4;

int general_concept_size;           /* Gets filled in during initialization. */

int *concept_offset_tables[] = {
   phantom_offsets,
   tandem_offsets,
   distort_offsets,
   dist4_offsets,
   misc_c_offsets,
   0};

int *concept_size_tables[] = {
   phantom_sizes,
   tandem_sizes,
   distort_sizes,
   dist4_sizes,
   misc_c_sizes,
   0};

char *concept_menu_strings[] = {
   "PHANTOM SETUP concepts",
   "COUPLES/TANDEM concepts",
   "DISTORTED SETUP concepts",
   "4-PERSON DISTORTED concepts",
   "MISCELLANEOUS concepts",
   0};
