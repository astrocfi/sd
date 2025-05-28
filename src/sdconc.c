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

    This is for version 28. */

/* This defines the following functions:
   concentric_move
   normalize_concentric
   merge_setups
   on_your_own_move
   so_and_so_only_move
*/

#include "sd.h"


/* BEWARE!!  This list must track the array "bigconctab" . */
/* BEWARE!!  This list must track the array "conc_error_messages" . */
typedef enum {
   analyzer_NORMAL,
   analyzer_CHECKPT,
   analyzer_2X6,
   analyzer_6X2,
   analyzer_STAR12,
   analyzer_STAR16,
   analyzer_SINGLE,
   analyzer_VERTICAL6,
   analyzer_LATERAL6,
   analyzer_DIAMOND_LINE } analyzer_kind;


typedef struct {
   veryshort mapin[8];
   veryshort mapout[8];
   short inlimit;
   short outlimit;
   setup_kind bigsetup;
   setup_kind insetup;
   setup_kind outsetup;
   int bigsize;
   int inner_rot;    /* 1 if inner setup is rotated CCW relative to big setup */
   int outer_rot;    /* 1 if outer setup is rotated CCW relative to big setup */
   int mapelong;
   } cm_thing;

/*                                                                                                          mapelong --------|
                                                                                                           outer_rot -----|  |
                                                                                                    inner_rot ---------|  |  |
                                                              outlimit -----|                          bigsize ----|   |  |  |
                                   mapin           mapout       inlimit -|  |  bigsetup      insetup  outsetup     |   |  |  |   */

Private cm_thing map1x2_1x2 =        {{1, 3},       {0, 2},              2, 2, s1x4,           s_1x2,    s_1x2,    4,  0, 0, 0};
Private cm_thing oddmap1x2_1x2 =     {{3, 1},       {0, 2},              2, 2, sdmd,           s_1x2,    s_1x2,    4,  1, 0, 0};
/* The map "oddmapdmd_dmd", with its loss of elongation information for the outer diamond, is necessary to make the call
   "with confidence" work from the setup formed by having the centers partner tag in left-hand waves.  This means that certain
   Bakerisms invloving concentric diamonds, in which each diamond must remember its own elongation, are not possible.  Too bad. */
Private cm_thing oddmapdmd_dmd =     {{1, 3, 5, 7}, {6, 0, 2, 4},        4, 4, s_crosswave,    sdmd,     sdmd,     8,  0, 1, 9};
Private cm_thing mapdmd_dmd =        {{1, 3, 5, 7}, {0, 2, 4, 6},        4, 4, s_crosswave,    sdmd,     sdmd,     8,  0, 0, 9};
Private cm_thing oddmapdmd_1x4 =     {{1, 2, 5, 6}, {7, 0, 3, 4},        4, 4, s_3x1dmd,       s1x4,     sdmd,     8,  0, 1, 9};
Private cm_thing mapdmd_1x4 =        {{1, 2, 5, 6}, {0, 3, 4, 7},        4, 4, s_3x1dmd,       s1x4,     sdmd,     8,  0, 0, 9};
Private cm_thing oddmap_s_dmd_1x4 =  {{1, 2, 5, 6}, {7, 0, 3, 4},        4, 4, s_wingedstar,   s1x4,     sdmd,     8,  0, 1, 9};
Private cm_thing map_s_dmd_1x4 =     {{1, 2, 5, 6}, {0, 3, 4, 7},        4, 4, s_wingedstar,   s1x4,     sdmd,     8,  0, 0, 9};
Private cm_thing map_cs_1x4_dmd =    {{0, 3, 4, 7}, {1, 2, 5, 6},        4, 4, s_wingedstar,   sdmd,     s1x4,     8,  0, 0, 9};
Private cm_thing oddmap_s_star_1x4 = {{1, 2, 5, 6}, {7, 0, 3, 4},        4, 4, s_wingedstar,   s1x4,     s_star,   8,  0, 1, 9};
Private cm_thing map_s_star_1x4 =    {{1, 2, 5, 6}, {0, 3, 4, 7},        4, 4, s_wingedstar,   s1x4,     s_star,   8,  0, 0, 9};
Private cm_thing oddmap_s_short_1x6 = {{1, 2, 4, 7, 8, 10},
                                             {11, 0, 3, 5, 6, 9},       6, 6, s_wingedstar12, s_1x6,    s_short6, 12, 0, 1, 9};
Private cm_thing map_s_spindle_1x8 = {{1, 2, 6, 5, 9, 10, 14, 13},
                                     {3, 7, 12, 8, 11, 15, 4, 0},       8, 8, s_wingedstar16, s1x8,    s_spindle, 16, 0, 0, 9};
Private cm_thing map_spec_star12 =   {{2, 3, 4, 11}, {0, 1, 6, 7},       4, 4, s_wingedstar12, s_star,   s1x4,     12, 0, 0, 0};
Private cm_thing map_spec_star12v =  {{11, 2, 3, 4}, {0, 1, 6, 7},       4, 4, s_wingedstar12, s_star,   s1x4,     12, 1, 0, 0};
Private cm_thing map_spec_star16 =   {{2, 3, 5, 4}, {0, 1, 8, 9},       4, 4, s_wingedstar16, s_star,   s1x4,     16, 0, 0, 0};
Private cm_thing map_spec_star16v =  {{4, 2, 3, 5}, {0, 1, 8, 9},       4, 4, s_wingedstar16, s_star,   s1x4,     16, 1, 0, 0};
Private cm_thing mapdmd_2x2h =       {{1, 3, 5, 7}, {0, 2, 4, 6},        4, 4, s_galaxy,       s2x2,     sdmd,     8,  0, 0, 9};
Private cm_thing mapdmd_2x2v =       {{7, 1, 3, 5}, {0, 2, 4, 6},        4, 4, s_galaxy,       s2x2,     sdmd,     8,  1, 0, 9};
Private cm_thing map2x3_1x2 =        {{11, 5}, {0, 1, 2, 6, 7, 8},       2, 6, s_3dmd,         s_1x2,    s_2x3,    12, 0, 0, 9};
Private cm_thing map1x4_1x4 =        {{3, 2, 7, 6}, {0, 1, 4, 5},        4, 4, s1x8,           s1x4,     s1x4,     8,  0, 0, 0};
Private cm_thing oddmap1x4_1x4 =     {{6, 7, 2, 3}, {0, 1, 4, 5},        4, 4, s_crosswave,    s1x4,     s1x4,     8,  1, 0, 0};
Private cm_thing map2x3_2x3 = {{8, 11, 1, 2, 5, 7}, {9, 10, 0, 3, 4, 6}, 6, 6, s3x4,           s_2x3,    s_2x3,    12, 1, 1, 1};
Private cm_thing map2x4_2x4 = {{10, 15, 3, 1, 2, 7, 11, 9}, {12, 13, 14, 0, 4, 5, 6, 8}, 8, 8, s4x4, s2x4, s2x4,   16, 0, 0, 1};
Private cm_thing map2x4_2x4v = {{6, 11, 15, 13, 14, 3, 7, 5}, {8, 9, 10, 12, 0, 1, 2, 4}, 8, 8, s4x4, s2x4, s2x4,  16, 1, 1, 1};
Private cm_thing map1x2_bone6 = {{1, 7, 6, 5, 3, 2}, {0, 4},             6, 2, s_ptpd,         s_bone6,  s_1x2,    8,  0, 0, 0};
Private cm_thing map1x6_1x2 = {{2, 6}, {0, 1, 3, 4, 5, 7},               2, 6, s1x8,           s_1x2,    s_1x6,    8,  0, 0, 0};
Private cm_thing mapbone6_1x2 = {{7, 3}, {0, 1, 2, 4, 5, 6},             2, 6, s_bone,         s_1x2,    s_bone6,  8,  0, 0, 0};
Private cm_thing map1x4_1x4_rc = {{0, 2, 4, 6}, {1, 3, 5, 7},            4, 4, s1x8,           s1x4,     s1x4,     8,  0, 0, 0};
Private cm_thing map1x2_bone6_rc = {{0, 1, 3, 4, 5, 7}, {6, 2},          6, 2, s_bone,         s_bone6,  s_1x2,    8,  0, 0, 0};
Private cm_thing map2x2_dmd_rc = {{7, 1, 3, 5}, {0, 2, 4, 6},            4, 4, s_spindle,      sdmd,     s2x2,     8,  0, 0, 0};
Private cm_thing map2x2_1x4_rc = {{0, 2, 4, 6}, {1, 7, 5, 3},            4, 4, s_ptpd,         s1x4,     s2x2,     8,  0, 0, 0};
Private cm_thing oddmap2x2_1x4_rc = {{0, 2, 4, 6}, {3, 1, 7, 5},         4, 4, s_ptpd,         s1x4,     s2x2,     8,  0, 1, 9};
Private cm_thing map1x2_2x3 = {{0, 1, 2, 4, 5, 6}, {7, 3},               6, 2, s_spindle,      s_2x3,    s_1x2,    8,  0, 0, 0};
Private cm_thing map1x2_short6 = {{1, 2, 3, 5, 6, 7}, {0, 4},            6, 2, s_galaxy,       s_short6, s_1x2,    8,  0, 0, 0};
Private cm_thing mapshort6_1x2h = {{5, 1}, {6, 7, 0, 2, 3, 4},           2, 6, s_spindle,      s_1x2,    s_short6, 8,  1, 1, 1};
Private cm_thing mapshort6_1x2v = {{7, 3}, {5, 6, 0, 1, 2, 4},           2, 6, s_hrglass,      s_1x2,    s_short6, 8,  1, 1, 0};
Private cm_thing mapstar_2x2 = {{1, 3, 5, 7}, {0, 2, 4, 6},              4, 4, s_galaxy,       s2x2,     s_star,   8,  0, 0, 0};
Private cm_thing mapstar_star = {{1, 3, 5, 7}, {0, 2, 4, 6},             4, 4, s_thar,         s_star,   s_star,   8,  0, 0, 0};
Private cm_thing map1x2_1x6 = {{1, 3, 2, 5, 7, 6}, {0, 4},               6, 2, s1x8,           s_1x6,    s_1x2,    8,  0, 0, 0};
Private cm_thing oddmap1x2_1x6 = {{0, 1, 2, 4, 5, 6}, {7, 3},            6, 2, s_3x1dmd,       s_1x6,    s_1x2,    12, 0, 1, 0};
Private cm_thing map1x4_2x2 = {{0, 1, 4, 5}, {6, 7, 2, 3},               4, 4, s_rigger,       s2x2,     s1x4,     8,  0, 0, 0};
Private cm_thing oddmap1x4_2x2 = {{5, 0, 1, 4}, {6, 7, 2, 3},            4, 4, s_rigger,       s2x2,     s1x4,     8,  1, 0, 0};
Private cm_thing map1x4_star = {{2, 3, 6, 7}, {0, 1, 4, 5},              4, 4, s_wingedstar,   s_star,   s1x4,     8,  0, 0, 0};
Private cm_thing oddmap1x4_star = {{7, 2, 3, 6}, {0, 1, 4, 5},           4, 4, s_wingedstar,   s_star,   s1x4,     8,  1, 0, 0};
Private cm_thing map2x2_dmd = {{6, 3, 2, 7}, {0, 1, 4, 5},               4, 4, s_hrglass,      sdmd,     s2x2,     8,  0, 0, 1};
Private cm_thing oddmap2x2_dmd = {{6, 3, 2, 7}, {5, 0, 1, 4},            4, 4, s_hrglass,      sdmd,     s2x2,     8,  0, 1, 9};
Private cm_thing map2x2_1x4h = {{6, 7, 2, 3}, {0, 1, 4, 5},              4, 4, s_qtag,         s1x4,     s2x2,     8,  0, 0, 1};
Private cm_thing oddmap2x2_1x4h = {{6, 7, 2, 3}, {5, 0, 1, 4},           4, 4, s_bone,         s1x4,     s2x2,     8,  0, 1, 9};
Private cm_thing mapstar_1x4 =    {{1, 2, 5, 6}, {0, 3, 4, 7},           4, 4, s_3x1dmd,       s1x4,     s_star,   12, 0, 0, 9};
Private cm_thing oddmapstar_1x4 = {{1, 2, 5, 6}, {7, 0, 3, 4},           4, 4, s_3x1dmd,       s1x4,     s_star,   12, 0, 1, 9};
Private cm_thing mapstar_dmd ={{1, 3, 5, 7}, {0, 2, 4, 6},               4, 4, s_crosswave,    sdmd,     s_star,   8,  0, 0, 9};
Private cm_thing oddmapstar_dmd =  {{1, 3, 5, 7}, {6, 0, 2, 4},          4, 4, s_crosswave,    sdmd,     s_star,   8,  0, 1, 9};
Private cm_thing map2x2_1x4v = {{6, 7, 2, 3}, {0, 1, 4, 5},              4, 4, s_bone,         s1x4,     s2x2,     8,  0, 0, 0};
Private cm_thing oddmap2x2_1x4v = {{6, 7, 2, 3}, {5, 0, 1, 4},           4, 4, s_qtag,         s1x4,     s2x2,     8,  0, 1, 9};
Private cm_thing map2x2_2x2v = {{1, 2, 5, 6}, {0, 3, 4, 7},              4, 4, s2x4,           s2x2,     s2x2,     8,  0, 0, 0};
Private cm_thing oddmap2x2_2x2h = {{1, 2, 5, 6}, {7, 0, 3, 4},           4, 4, s2x4,           s2x2,     s2x2,     8,  0, 1, 9};
Private cm_thing oddmap2x2_2x2v = {{6, 1, 2, 5}, {0, 3, 4, 7},           4, 4, s2x4,           s2x2,     s2x2,     8,  1, 0, 9};
Private cm_thing map2x2_2x2h = {{6, 1, 2, 5}, {7, 0, 3, 4},              4, 4, s2x4,           s2x2,     s2x2,     8,  1, 1, 9};
Private cm_thing maplatgal = {{7, 0, 1, 3, 4, 5}, {6, 2},                6, 2, s_galaxy,       s_short6, s_1x2,    8,  1, 1, 0};
Private cm_thing oddmap1x4_dmd = {{7, 2, 3, 6}, {0, 1, 4, 5},            4, 4, s_3x1dmd,       sdmd,     s1x4,     12, 1, 0, 0};
Private cm_thing oddmap1x2_bone6 = {{5, 0, 3, 1, 4, 7}, {6, 2},          6, 2, s_hrglass,      s_bone6,  s_1x2,    8,  1, 0, 0};
Private cm_thing oddmap1x2_2x3 = {{5, 7, 0, 1, 3, 4}, {6, 2},            6, 2, s_qtag,         s_2x3,    s_1x2,    8,  1, 0, 0};
Private cm_thing oddmap1x2_short6 = {{5, 7, 0, 1, 3, 4}, {6, 2},         6, 2, s_rigger,       s_short6, s_1x2,    8,  1, 0, 0};
Private cm_thing oddmap2x3_1x2 = {{11, 5}, {9, 10, 0, 3, 4, 6},          2, 6, s3x4,           s_1x2,    s_2x3,    12, 0, 1, 0};
Private cm_thing oddmapshort6_1x2h = {{2, 6}, {3, 0, 1, 7, 4, 5},        2, 6, s_ptpd,         s_1x2,    s_short6, 8,  0, 1, 1};
Private cm_thing oddmapshort6_1x2v = {{7, 3}, {5, 6, 0, 1, 2, 4},        2, 6, s_qtag,         s_1x2,    s_short6, 8,  0, 1, 0};
Private cm_thing oddmap1x2_short6_rc = {{5, 6, 0, 1, 2, 4}, {7, 3},      6, 2, s_rigger,       s_short6, s_1x2,    8,  1, 0, 0};
Private cm_thing map2x4_2x2 = {{2, 3, 8, 9}, {0, 1, 4, 5, 6, 7, 10, 11}, 4, 8, s2x6,           s2x2,     s2x4,     12, 0, 0, 9};
Private cm_thing oddmap2x4_2x2 = {{9, 2, 3, 8},
                                       {0, 1, 4, 5, 6, 7, 10, 11},      4, 8, s2x6,           s2x2,     s2x4,     12, 1, 0, 9};
Private cm_thing mapdmd_line = {{1, 2, 5, 6}, {0, 3, 4, 7},              4, 4, s_3x1dmd,       s1x4,     sdmd,     8,  0, 0, 0};
Private cm_thing map_s_dmd_line = {{1, 2, 5, 6}, {0, 3, 4, 7},           4, 4, s_wingedstar,   s1x4,     sdmd,     8,  0, 0, 0};
Private cm_thing *concmap1x2_1x2[4]       = {&map1x2_1x2,          &oddmap1x2_1x2,       &map1x2_1x2,      &oddmap1x2_1x2};
Private cm_thing *concmapdmd_dmd[4]       = {&mapdmd_dmd,          &oddmapdmd_dmd,       &mapdmd_dmd,      &oddmapdmd_dmd};
Private cm_thing *concmapdmd_1x4[4]       = {&mapdmd_1x4,          &oddmapdmd_1x4,       &mapdmd_1x4,      &oddmapdmd_1x4};
Private cm_thing *concmap_s_dmd_1x4[4]    = {&map_s_dmd_1x4,       &oddmap_s_dmd_1x4,    &map_s_dmd_1x4,   &oddmap_s_dmd_1x4};
Private cm_thing *concmap_cs_1x4_dmd[4]   = {&map_cs_1x4_dmd,      0,                    &map_cs_1x4_dmd,  0};
Private cm_thing *concmap_s_star_1x4[4]   = {&map_s_star_1x4,      &oddmap_s_star_1x4,   &map_s_star_1x4,  &oddmap_s_star_1x4};
Private cm_thing *concmap_s_short_1x6[4]  = {0,                    &oddmap_s_short_1x6,  0,                0};
Private cm_thing *concmap_s_spindle_1x8[4] = {0,                   0,                    &map_s_spindle_1x8, 0};
Private cm_thing *concmapdmd_2x2[4]       = {&mapdmd_2x2h,         &mapdmd_2x2v,         &mapdmd_2x2h,     &mapdmd_2x2v};
Private cm_thing *concmap2x2_2x2[4]       = {&map2x2_2x2v,         &oddmap2x2_2x2v,      &map2x2_2x2h,     &oddmap2x2_2x2h};
Private cm_thing *concmap2x2_1x4[4]       = {&map2x2_1x4v,         &oddmap2x2_1x4v,      &map2x2_1x4h,     &oddmap2x2_1x4h};
Private cm_thing *concmap2x2_dmd[4]       = {0,                    &oddmap2x2_dmd,       &map2x2_dmd,      0};
Private cm_thing *concmapshort6_1x2[4]    = {&mapshort6_1x2v,      &oddmapshort6_1x2v,   &mapshort6_1x2h,  &oddmapshort6_1x2h};
Private cm_thing *concmap1x2_1x6[4]       = {&map1x2_1x6,          &oddmap1x2_1x6,       &map1x2_1x6,      &oddmap1x2_1x6};
Private cm_thing *concmap1x2_bone6[4]     = {&map1x2_bone6,        &oddmap1x2_bone6,     &map1x2_bone6,    &oddmap1x2_bone6};
Private cm_thing *concmap1x2_2x3[4]       = {&map1x2_2x3,          &oddmap1x2_2x3,       &map1x2_2x3,      &oddmap1x2_2x3};
Private cm_thing *concmap1x2_short6[4]    = {&map1x2_short6,       &oddmap1x2_short6,    &map1x2_short6,   &oddmap1x2_short6};
Private cm_thing *concmap2x4_2x2[4]       = {&map2x4_2x2,          &oddmap2x4_2x2,       &map2x4_2x2,      &oddmap2x4_2x2};
Private cm_thing *concmap2x4_2x4[4]       = {&map2x4_2x4,          0,                    &map2x4_2x4,      0};
Private cm_thing *concmap1x4_2x2[4]       = {&map1x4_2x2,          &oddmap1x4_2x2,       &map1x4_2x2,      &oddmap1x4_2x2};
Private cm_thing *concmap1x4_star[4]      = {&map1x4_star,         &oddmap1x4_star,      &map1x4_star,     &oddmap1x4_star};
Private cm_thing *concmap1x4_dmd[4]       = {0,                    &oddmap1x4_dmd,       0,                &oddmap1x4_dmd};
Private cm_thing *concmap1x4_1x4[4]       = {&map1x4_1x4,          &oddmap1x4_1x4,       &map1x4_1x4,      &oddmap1x4_1x4};
Private cm_thing *concmapstar_2x2[4]      = {&mapstar_2x2,         0,                    &mapstar_2x2,     0};
Private cm_thing *concmapstar_star[4]     = {&mapstar_star,        0,                    &mapstar_star,    0};
Private cm_thing *concmapstar_1x4[4]      = {&mapstar_1x4,         &oddmapstar_1x4,      &mapstar_1x4,     &oddmapstar_1x4};
Private cm_thing *concmapstar_dmd[4]      = {&mapstar_dmd,         &oddmapstar_dmd,      &mapstar_dmd,     &oddmapstar_dmd};
Private cm_thing *concmap2x3_1x2[4]       = {&map2x3_1x2,          &oddmap2x3_1x2,       &map2x3_1x2,      &oddmap2x3_1x2};
Private cm_thing *concmap2x3_2x3[4]       = {&map2x3_2x3,          0,                    &map2x3_2x3,      0};
Private cm_thing *concmapbone6_1x2[4]     = {&mapbone6_1x2,        0,                    &mapbone6_1x2,    0};
Private cm_thing *concmap1x6_1x2[4]       = {&map1x6_1x2,          0,                    &map1x6_1x2,      0};
Private cm_thing *concmap1x4_1x4_rc[4]    = {&map1x4_1x4_rc,       0,                    &map1x4_1x4_rc,   0};
Private cm_thing *concmap2x2_dmd_rc[4]    = {&map2x2_dmd_rc,       0,                    &map2x2_dmd_rc,   0};
Private cm_thing *concmap2x2_1x4_rc[4]    = {&map2x2_1x4_rc,       &oddmap2x2_1x4_rc,    &map2x2_1x4_rc,   &oddmap2x2_1x4_rc};



/* This overwrites its "inners" and "outers argument setups. */
extern void normalize_concentric(
   calldef_schema synthesizer,
   int center_arity,
   setup inners[],
   setup *outers,
   int outer_elongation,
   setup *result)
{
   /* If "outer_elongation" < 0, the outsides can't deduce their ending spots on
      the basis of the starting formation.  In this case, it is an error unless
      they go to some setup for which their elongation is obvious, like a 1x4. */

   int i, j, q, rot;
   cm_thing **map_ptr;
   cm_thing *lmap_ptr;

   clear_people(result);

   /* If a call was being done "piecewise" or "random", we demand that both
      calls run out of parts at the same time, and, when that happens, we
      report it to the higher level in the recursion. */

   if ((inners[0].setupflags ^ outers->setupflags) & RESULTFLAG__DID_LAST_PART)
      fail("Centers and ends parts must use the same number of fractions.");

   result->setupflags = inners[0].setupflags | outers->setupflags;

   if (inners[0].kind == nothing && outers->kind == nothing) {
      result->kind = nothing;
      return;
   }

   compute_rotation_again:

   i = (inners[0].rotation - outers->rotation) & 3;

   map_ptr = 0;

   if (center_arity == 2) {
      /* Fix up nonexistent stars, in a rather inept way. */
      if (inners[0].kind == nothing) {
         inners[0].kind = s_star;
         inners[0].rotation = 0;
         inners[0].setupflags = outers->setupflags;
         clear_people(&inners[0]);
      }

      if (inners[1].kind == nothing) {
         inners[1].kind = s_star;
         inners[1].rotation = 0;
         inners[1].setupflags = outers->setupflags;
         clear_people(&inners[1]);
      }

      if (synthesizer != schema_conc_star12 || outers->kind != s1x4 || inners[0].kind != s_star || inners[1].kind != s_star)
         fail("Can't do this with 12 matrix stars.");

      if (i&1)
         lmap_ptr = &map_spec_star12v;
      else
         lmap_ptr = &map_spec_star12;

      goto gotit;
   }
   else if (center_arity == 3) {
      /* Fix up nonexistent stars, in a rather inept way. */
      if (inners[0].kind == nothing) {
         inners[0].kind = s_star;
         inners[0].rotation = 0;
         inners[0].setupflags = outers->setupflags;
         clear_people(&inners[0]);
      }

      if (inners[1].kind == nothing) {
         inners[1].kind = s_star;
         inners[1].rotation = 0;
         inners[1].setupflags = outers->setupflags;
         clear_people(&inners[1]);
      }

      if (inners[2].kind == nothing) {
         inners[2].kind = s_star;
         inners[2].rotation = 0;
         inners[2].setupflags = outers->setupflags;
         clear_people(&inners[2]);
      }

      if (synthesizer != schema_conc_star16 || outers->kind != s1x4 || inners[0].kind != s_star || inners[1].kind != s_star || inners[2].kind != s_star)
         fail("Can't do this with 16 matrix stars.");

      if (i&1)
         lmap_ptr = &map_spec_star16v;
      else
         lmap_ptr = &map_spec_star16;

      goto gotit;
   }

   if (synthesizer == schema_rev_checkpoint) {
      /* Fix up nonexistent centers or ends, in a rather inept way. */
      if (inners[0].kind == nothing) {
         inners[0].kind = outers->kind;
         inners[0].rotation = outers->rotation;
         inners[0].setupflags = outers->setupflags;
         clear_people(&inners[0]);
         i = 0;
      }
      else if (outers->kind == nothing) {
         outers->kind = inners[0].kind;
         outers->rotation = inners[0].rotation;
         outers->setupflags = inners[0].setupflags;
         clear_people(outers);
         i = 0;
      }

      switch (outers->kind) {
         case s2x2:
            switch (inners[0].kind) {
               case s1x4: map_ptr = concmap2x2_1x4_rc; break;
               case sdmd: map_ptr = concmap2x2_dmd_rc; break;
            }
            break;
         case s1x4:
            switch (inners[0].kind) {
               case s1x4: map_ptr = concmap1x4_1x4_rc; break;
               case sdmd:
                  /* This is a diamond inside a 1x4, oriented the bad way.
                     Leave it as a concentric setup. */
                  if (!(i&1)) {
                     result->inner.skind = inners[0].kind;
                     result->inner.srotation = inners[0].rotation;
                     result->outer.skind = outers->kind;
                     result->outer.srotation = outers->rotation;
                     result->outer_elongation = 0;
                     result->kind = s_normal_concentric;
                     if (i == 0) {
                        install_person(result, 0, outers, 1);
                        install_person(result, 2, outers, 3);
                        install_person(result, 1, &inners[0], 1);
                        install_person(result, 3, &inners[0], 3);
                        install_person(result, 12, &inners[0], 0);
                        install_person(result, 14, &inners[0], 2);
                        install_person(result, 13, outers, 0);
                        install_person(result, 15, outers, 2);
                     }
                     else
                        fail("Sorry, code is broken at line 581 of sdconc.c.");

                     canonicalize_rotation(result);
                     return;
                  }
            }
            break;
      }
   }
   else if (synthesizer == schema_ckpt_star) {
      /* There are a few cases of centers or ends being phantoms, in which
         we nevertheless know what to do, since we know that the setup should
         be some kind of "winged star". */
      if (inners[0].kind == nothing && outers->kind == s1x4) {
         inners[0].kind = s_star;
         inners[0].rotation = 0;
         inners[0].setupflags = outers->setupflags;
         clear_people(&inners[0]);
         goto compute_rotation_again;
      }
      else if (inners[0].kind == sdmd && outers->kind == nothing) {
         /* The test case for this is: RWV:intlkphanbox relay top;splitphanbox flip reaction. */
         outers->kind = s1x4;
         outers->rotation = inners[0].rotation;
         outers->setupflags = inners[0].setupflags;
         clear_people(outers);
         goto compute_rotation_again;
      }

      switch (outers->kind) {
         case s1x4:
            switch (inners[0].kind) {
               case sdmd: map_ptr = concmap_cs_1x4_dmd; break;
            }
            break;
      }
   }
   else if (synthesizer == schema_conc_star) {
      /* There are a few cases of centers or ends being phantoms, in which
         we nevertheless know what to do, since we know that the setup should
         be some kind of "winged star". */
      if (outers->kind == nothing && inners[0].kind == s1x4) {
         outers->kind = s_star;
         outers->rotation = 0;
         outers->setupflags = inners[0].setupflags;
         clear_people(outers);
         goto compute_rotation_again;
      }
      else if (outers->kind == nothing && inners[0].kind == s_star) {
         outers->kind = s1x4;
         outers->rotation = outer_elongation;
         outers->setupflags = inners[0].setupflags;
         clear_people(outers);
         goto compute_rotation_again;
      }
      else if (outers->kind == s1x4 && inners[0].kind == nothing) {
         inners[0].kind = s_star;
         inners[0].rotation = 0;
         inners[0].setupflags = outers->setupflags;
         clear_people(&inners[0]);
         goto compute_rotation_again;
      }

      switch (outers->kind) {
         case sdmd:
            switch (inners[0].kind) {
               case s1x4: map_ptr = concmap_s_dmd_1x4; break;
            }
            break;
         case s_star:
            switch (inners[0].kind) {
               case s1x4: map_ptr = concmap_s_star_1x4; break;
            }
            break;
         case s1x4:
            switch (inners[0].kind) {
               case s_star: map_ptr = concmap1x4_star; break;
               case s1x4:
                  /* In certain phantom cases, what should have been a diamond
                     around the outside, resulting from a 1/2 circulate, will be
                     a line with the two centers missing, since the basic_move
                     routine gives preference to a line when it is ambiguous.
                     If this happens, we have to turn it back into a diamond. */
                  if (!(outers->people[1].id1 | outers->people[3].id1)) {
                     outers->kind = sdmd;  /* That's all that it takes to fix it. */
                     goto compute_rotation_again;
                  }
                  /* Or the two ends missing. */
                  else if (!(outers->people[0].id1 | outers->people[2].id1)) {
                     outers->kind = sdmd;
                     outers->rotation = (outers->rotation-1) & 3;
                     (void) copy_rot(outers, 1, outers, 1, 011);
                     (void) copy_rot(outers, 3, outers, 3, 011);
                     canonicalize_rotation(outers);
                     goto compute_rotation_again;
                  }
            }
            break;
      }
   }
   else if (synthesizer == schema_conc_star12) {
      switch (outers->kind) {
         case s_short6:
            switch (inners[0].kind) {
               case s_1x6: map_ptr = concmap_s_short_1x6; break;
            }
            break;
      }
   }
   else if (synthesizer == schema_conc_star16) {
      switch (outers->kind) {
         case s_spindle:
            switch (inners[0].kind) {
               case s1x8: map_ptr = concmap_s_spindle_1x8; break;
            }
            break;
      }
   }
   else {
      /* Fix up nonexistent centers or ends, in a rather inept way. */
      if (inners[0].kind == nothing) {
         inners[0].kind = outers->kind;
         inners[0].rotation = outers->rotation;
         inners[0].setupflags = outers->setupflags;
         clear_people(&inners[0]);
         i = 0;
      }
      else if (outers->kind == nothing) {
         outers->kind = inners[0].kind;
         outers->rotation = inners[0].rotation;
         outers->setupflags = inners[0].setupflags;
         clear_people(outers);
         i = 0;
      }

      /* Nonexistent center or ends have been taken care of.  Now figure out how to put
         the setups together. */

      switch (outers->kind) {
         case s_1x6:
            switch (inners[0].kind) {
               case s_1x2: map_ptr = concmap1x6_1x2; break;
            }
            break;
         case s_bone6:
            switch (inners[0].kind) {
               case s_1x2: map_ptr = concmapbone6_1x2; break;
            }
            break;
         case s_2x3:
            switch (inners[0].kind) {
               case s_1x2: map_ptr = concmap2x3_1x2; break;
               case s_2x3: map_ptr = concmap2x3_2x3; break;
            }
            break;
         case s2x4:
            switch (inners[0].kind) {
               case s2x2: map_ptr = concmap2x4_2x2; break;
               case s2x4: map_ptr = concmap2x4_2x4; break;
            }
            break;
         case s1x4:
            switch (inners[0].kind) {
               case s1x4: map_ptr = concmap1x4_1x4; break;
               case sdmd: map_ptr = concmap1x4_dmd; break;
               case s_star: map_ptr = concmap1x4_star; break;
               case s2x2: map_ptr = concmap1x4_2x2; break;
            }
            break;
         case s_1x2:
            switch (inners[0].kind) {
               case s_1x2:    map_ptr = concmap1x2_1x2; break;
               case s_2x3:    map_ptr = concmap1x2_2x3; break;
               case s_bone6 : map_ptr = concmap1x2_bone6; break;
               case s_short6: map_ptr = concmap1x2_short6; break;
               case s_1x6:    map_ptr = concmap1x2_1x6; break;
            }
            break;
         case s_short6:
            switch (inners[0].kind) {
               case s_1x2: map_ptr = concmapshort6_1x2; break;
            }
            break;
         case s2x2:
            switch (inners[0].kind) {
               case s2x2: map_ptr = concmap2x2_2x2; break;
               case s1x4: map_ptr = concmap2x2_1x4; break;
               case sdmd: map_ptr = concmap2x2_dmd; break;
            }
            break;
         case sdmd:
            switch (inners[0].kind) {
               case sdmd: map_ptr = concmapdmd_dmd; break;
               case s1x4: map_ptr = concmapdmd_1x4; break;
               case s2x2: map_ptr = concmapdmd_2x2; break;
            }
            break;
         case s_star:
            switch (inners[0].kind) {
               case sdmd: map_ptr = concmapstar_dmd; break;
               case s1x4: map_ptr = concmapstar_1x4; break;
               case s_star: map_ptr = concmapstar_star; break;
               case s2x2: map_ptr = concmapstar_2x2; break;
            }
            break;
      }
   }

   if (!map_ptr) goto anomalize_it;

   if (outer_elongation < 0) {
      /* We need to find out whether it would have made a difference
         when picking out the map. */

      if (map_ptr[(i&1)] != map_ptr[(i&1) + 2]) goto elongation_loss;
   }

   lmap_ptr = map_ptr[(i&1) + (((outer_elongation ^ outers->rotation) & 1) << 1)];

gotit:

   if (!lmap_ptr) goto anomalize_it;

   result->kind = lmap_ptr->bigsetup;
   result->rotation = outers->rotation + lmap_ptr->outer_rot;

   rot = ((-lmap_ptr->outer_rot) & 3) * 011;
   for (j=0; j<lmap_ptr->outlimit; j++)
      (void) copy_rot(result, lmap_ptr->mapout[j], outers, j, rot);

   /* Find out whether inners need to be flipped around. */
   q = i + lmap_ptr->inner_rot - lmap_ptr->outer_rot;

   if (q & 1)
      fail("Sorry, there is a bug in normalize_concentric.");

   if (q & 2) {
      inners[0].rotation += 2;
      canonicalize_rotation(&inners[0]);
   }

   rot = ((-lmap_ptr->inner_rot) & 3) * 011;
   for (j=0; j<lmap_ptr->inlimit; j++)
      (void) copy_rot(result, lmap_ptr->mapin[j], &inners[0], j, rot);

   if (lmap_ptr == &map_spec_star12) {
      if (q & 2) {
         inners[1].rotation += 2;
         canonicalize_rotation(&inners[1]);
      }
      (void) copy_rot(result, 10, &inners[1], 0, rot);
      (void) copy_rot(result, 5,  &inners[1], 1, rot);
      (void) copy_rot(result, 8,  &inners[1], 2, rot);
      (void) copy_rot(result, 9,  &inners[1], 3, rot);
   }
   else if (lmap_ptr == &map_spec_star12v) {
      if (q & 2) {
         inners[1].rotation += 2;
         canonicalize_rotation(&inners[1]);
      }
      (void) copy_rot(result, 9,  &inners[1], 0, rot);
      (void) copy_rot(result, 10, &inners[1], 1, rot);
      (void) copy_rot(result, 5,  &inners[1], 2, rot);
      (void) copy_rot(result, 8,  &inners[1], 3, rot);
   }
   else if (lmap_ptr == &map_spec_star16) {
      if (q & 2) {
         inners[1].rotation += 2;
         inners[2].rotation += 2;
         canonicalize_rotation(&inners[1]);
         canonicalize_rotation(&inners[2]);
      }
      (void) copy_rot(result, 6,  &inners[1], 0, rot);
      (void) copy_rot(result, 7,  &inners[1], 1, rot);
      (void) copy_rot(result, 14, &inners[1], 2, rot);
      (void) copy_rot(result, 15, &inners[1], 3, rot);
      (void) copy_rot(result, 13, &inners[2], 0, rot);
      (void) copy_rot(result, 12, &inners[2], 1, rot);
      (void) copy_rot(result, 10, &inners[2], 2, rot);
      (void) copy_rot(result, 11, &inners[2], 3, rot);
   }
   else if (lmap_ptr == &map_spec_star16v) {
      if (q & 2) {
         inners[1].rotation += 2;
         inners[2].rotation += 2;
         canonicalize_rotation(&inners[1]);
         canonicalize_rotation(&inners[2]);
      }
      (void) copy_rot(result, 15, &inners[1], 0, rot);
      (void) copy_rot(result, 6,  &inners[1], 1, rot);
      (void) copy_rot(result, 7,  &inners[1], 2, rot);
      (void) copy_rot(result, 14, &inners[1], 3, rot);
      (void) copy_rot(result, 11, &inners[2], 0, rot);
      (void) copy_rot(result, 13, &inners[2], 1, rot);
      (void) copy_rot(result, 12, &inners[2], 2, rot);
      (void) copy_rot(result, 10, &inners[2], 3, rot);
   }

   canonicalize_rotation(result);
   return;

   anomalize_it:            /* Failed, just leave it as it is. */

   switch (synthesizer) {
      case schema_rev_checkpoint:
         fail("Sorry, can't figure out this reverse checkpoint result.");
      case schema_single_concentric:
      case schema_single_cross_concentric:
         fail("Can't figure out this single concentric result.");
      case schema_conc_star:
      case schema_conc_star12:
      case schema_conc_star16:
         fail("Can't figure out this concentric result.");
   }

   if (outer_elongation < 0) goto elongation_loss;

   result->kind = s_normal_concentric;
   result->inner.skind = inners[0].kind;
   result->inner.srotation = inners[0].rotation;
   result->outer.skind = outers->kind;
   result->outer.srotation = outers->rotation;
   result->outer_elongation = outer_elongation ^ outers->rotation;
   for (j=0; j<12; j++) {
      (void) copy_person(result, j, &inners[0], j);
      (void) copy_person(result, j+12, outers, j);
   }
   canonicalize_rotation(result);
   return;

   elongation_loss:
   fail("Ends can't figure out what spots to finish on.");
}



/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
/* BEWARE!!  The horizontal structure is keyed to the enumeration "analyzer_kind" :
   normal      checkpt                   2x6                 6x2             star12            star16             single       vertical6          lateral6     diamond_line */
                                                                                                                     
Private cm_thing *bigconctab[][10] = {
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* nothing */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_1x1 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_1x2 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_1x3 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s2x2 */
   {0,              0,                    0,                  0,                 0,                0,                &oddmap1x2_1x2, 0,                0,          0},               /* sdmd */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_star */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_trngl */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_bone6 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_short6 */
   {&map2x2_1x4h,   0,                    &oddmapshort6_1x2v, &oddmap1x2_2x3,    0,                0,                0,              0,                0,          0},               /* s_qtag */
   {&map2x2_1x4v,   &map1x2_bone6_rc,     &mapbone6_1x2,      0,                 0,                0,                0,              0,                0,          0},               /* s_bone */
   {&map1x4_2x2,    &oddmap1x2_short6_rc, 0,                  &oddmap1x2_short6, 0,                0,                0,              0,                0,          0},               /* s_rigger */
   {0,              &map2x2_dmd_rc,       &mapshort6_1x2h,    &map1x2_2x3,       0,                0,                0,              0,                0,          0},               /* s_spindle */
   {&map2x2_dmd,    0,                    &mapshort6_1x2v,    0,                 0,                0,                0,              &oddmap1x2_bone6, 0,          0},               /* s_hrglass */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_hyperglass */
   {&oddmap1x4_1x4, 0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_crosswave */
   {0,              0,                    0,                  0,                 0,                0,                &map1x2_1x2,    0,                0,          0},               /* s1x4 */
   {&map1x4_1x4,    &map1x4_1x4_rc,       &map1x6_1x2,        &map1x2_1x6,       0,                0,                0,              0,                0,          0},               /* s1x8 */
   {&map2x2_2x2v,   0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s2x4 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_2x3 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_1x6 */
   {0,              0,                    &oddmap2x3_1x2,     0,                 &map2x3_2x3,      0,                0,              0,                0,          0},               /* s3x4 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s2x6 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s2x8 */
   {0,              0,                    0,                  0,                 0,                &map2x4_2x4v,     0,              0,                0,          0},               /* s4x4 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_x1x6 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s1x10 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s1x12 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s1x14 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s1x16 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_c1phan */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_bigblob */
   {0,              &map2x2_1x4_rc,       &oddmapshort6_1x2h, &map1x2_bone6,     0,                0,                0,              0,                0,          0},               /* s_ptpd */
   {&oddmap1x4_dmd, 0,                    0,                  &oddmap1x2_1x6,    0,                0,                0,              0,                0,          &mapdmd_line},    /* s_3x1dmd */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_3dmd */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_4dmd */
   {&map1x4_star,   0,                    0,                  0,                 0,                0,                0,              0,                0,          &map_s_dmd_line}, /* s_wingedstar */
   {0,              0,                    0,                  0,                 &map_spec_star12, 0,                0,              0,                0,          &oddmap_s_short_1x6},/* s_wingedstar12 */
   {0,              0,                    0,                  0,                 0,                &map_spec_star16, 0,              0,                0,          &map_s_spindle_1x8},/* s_wingedstar16 */
   {&mapstar_2x2,   0,                    0,                  0,                 0,                0,                0,              &map1x2_short6,   &maplatgal, 0},               /* s_galaxy */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s4x6 */
   {&mapstar_star,  0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_thar */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_x4dmd */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0},               /* s_8x8 */
   {0,              0,                    0,                  0,                 0,                0,                0,              0,                0,          0}};              /* s_normal_concentric */





/* BEWARE!!  This is keyed to the enumeration "analyzer_kind". */
Private char *conc_error_messages[] = {
   "Can't find centers and ends in this formation.",                   /* analyzer_NORMAL */
   "Can't find checkpoint people in this formation.",                  /* analyzer_CHECKPT */
   "Can't find 2 centers and 6 ends in this formation.",               /* analyzer_2X6 */
   "Can't find 6 centers and 2 ends in this formation.",               /* analyzer_6X2 */
   "Can't find 12 matrix centers and ends in this formation.",         /* analyzer_STAR12 */
   "Can't find 16 matrix centers and ends in this formation.",         /* analyzer_STAR16 */
   "Can't find single concentric centers and ends in this formation.", /* analyzer_SINGLE */
   "Wrong formation.",                                                 /* analyzer_VERTICAL6 */
   "Wrong formation.",                                                 /* analyzer_LATERAL6 */
   "Can't find center line and outer diamond."                         /* analyzer_DIAMOND_LINE */
};




/* This sets "outer_elongation" to the absolute elongation of the
   outsides.  If the outsides are in a 2x2, this, along with individual
   facing directions, will permit enforcement of the "lines-to-lines/
   columns-to-columns" rule.  Otherwise, this will permit enforcement
   of the Hodson checkpoint rule.
   
   There are a few cases in which this result may seem wrong:
      (1) If we have triple diamonds with points in only the center
         diamond (that is, a line of 6 with some points hanging off
         the center 2), and we ask for the center 6/outer 2, the
         lonesome points become the ends, and "outer_elongation"
         reflects their elongation, even though the line of 6 is longer.
         This doesn't seem to affect any checkpoint or concentric cases.
         Perhaps the phrases "center 6" and "outer 2" aren't really
         correct here.
      (2) If we have a quarter tag, and we ask for the center 6/outer 2,
         the center 6 are, of course, a 2x3, and the ends of the line
         are the outer 2.  We set "outer_elongation" to reflect
         the elongation of the outer 2, which may not be what people
         would think.  Once again, this does not arise in any actual
         checkpoint or concentric case.
      (3) If we have an "H", and we ask for the center 2/outer 6, the
         outer 6 are the 2x3, and "outer_elongation" will show
         their elongation, even though that is not the elongation of
         the 3x4 matrix.  Once again, this does not arise in any actual
         checkpoint or concentric case. */

Private void concentrify(
   setup *ss,
   calldef_schema analyzer,
   setup inners[],
   setup *outers,
   int *center_arity,
   int *outer_elongation,    /* Set to elongation of original outers. */
   int *xconc_elongation)    /* If cross concentric, set to elongation of original ends. */

{
   int i, rot, analyzer_index;
   cm_thing *lmap_ptr;   

   clear_people(outers);
   clear_people(&inners[0]);

   outers->setupflags = ss->setupflags;
   inners[0].setupflags = ss->setupflags;

   *center_arity = 1;

   /* First, translate the analyzer into a form that encodes only what we need to know. */

   switch (analyzer) {
      case schema_lateral_6:
         analyzer_index = analyzer_LATERAL6; break;
      case schema_vertical_6:
         analyzer_index = analyzer_VERTICAL6; break;
      case schema_checkpoint:
      case schema_ckpt_star:
         analyzer_index = analyzer_CHECKPT; break;
      case schema_single_concentric:
      case schema_single_cross_concentric:
         analyzer_index = analyzer_SINGLE; break;
      case schema_conc_star12:
         analyzer_index = analyzer_STAR12; break;
      case schema_conc_star16:
         analyzer_index = analyzer_STAR16; break;
      case schema_concentric_diamond_line:
         analyzer_index = analyzer_DIAMOND_LINE; break;
      case schema_concentric_6_2:
         analyzer_index = analyzer_6X2; break;
      case schema_concentric_2_6:
         if (ss->kind == s3x4 && ((ss->people[1].id1 | ss->people[2].id1 | ss->people[7].id1 | ss->people[8].id1) ||
                     (!(ss->people[0].id1 & ss->people[3].id1 & ss->people[4].id1 & ss->people[5].id1 &
                     ss->people[6].id1 & ss->people[9].id1 & ss->people[10].id1 & ss->people[11].id1))))
            fail("Can't find centers and ends in this formation.");
         analyzer_index = analyzer_2X6;
         break;
      case schema_rev_checkpoint:
      case schema_concentric:
      case schema_conc_star:
      case schema_cross_concentric:
         analyzer_index = analyzer_NORMAL; break;
      case schema_any_concentric:
         analyzer_index = setup_limits[ss->kind] == 3 ? analyzer_SINGLE : analyzer_NORMAL; break;
      case schema_maybe_single_concentric:
         fail("Can't figure out whether concentric is single -- this shouldn't happen.");
      case schema_maybe_matrix_conc_star:
         fail("Can't figure out whether concentric is 12 or 16 matrix -- this shouldn't happen.");
      default:
         fail("Don't understand this concentricity type???.");
   }

   /* Next, deal with the "normal_concentric" special case.
      We need to be careful here.  The setup was not able to be normalized, but
      we are being asked to pick out centers and ends.  There are very few
      non-normal concentric setups for which we can do that correctly.  For example,
      if we have concentric diamonds whose points are along different axes from each
      other, who are the 4 centers of the total setup?  (If the axes of the diamonds
      had been consistent, the setup would have been normalized to crossed lines,
      and we wouldn't be here.)
      If we don't take action here and the setup is normal_concentric, an error will
      be raised, since the "bigconctab" entries are zero. */

   if (ss->kind == s_normal_concentric) {
      outers->rotation = ss->outer.srotation;
      inners[0].rotation = ss->outer.srotation;   /* Yes, this looks wrong, but it isn't. */

      switch (analyzer_index) {
         case analyzer_DIAMOND_LINE:
            if (ss->inner.skind == sdmd && ss->inner.srotation == ss->outer.srotation) {
               inners[0].kind = s1x4;
               outers->kind = sdmd;
               (void) copy_person(&inners[0], 0, ss, 13);
               (void) copy_person(&inners[0], 1, ss, 0);
               (void) copy_person(&inners[0], 2, ss, 15);
               (void) copy_person(&inners[0], 3, ss, 2);
               (void) copy_person(outers, 0, ss, 12);
               (void) copy_person(outers, 1, ss, 1);
               (void) copy_person(outers, 2, ss, 14);
               (void) copy_person(outers, 3, ss, 3);
               if (ss->outer.skind == s1x4) {
                  *outer_elongation = ss->outer.srotation & 1;
                  goto finish;
               }
            }
            break;
         case analyzer_NORMAL:
            if (ss->inner.skind == sdmd && ss->inner.srotation == ss->outer.srotation) {
               inners[0].kind = sdmd;
               outers->kind = ss->outer.skind;
               for (i=0; i<4; i++) {
                  (void) copy_person(&inners[0], i, ss, i);
                  (void) copy_person(outers, i, ss, i+12);
               }

               /* We allow a diamond inside a box with wrong elongation (if elongation were good, it would be an hourglass.) */
               if (ss->outer.skind == s2x2) {
                  *outer_elongation = (ss->outer.srotation ^ ss->outer_elongation) & 1;
                  goto finish;
               }
               /* And a diamond inside a line with wrong elongation (if elongation were good, it would be a 3x1 diamond.) */
               if (ss->outer.skind == s1x4) {
                  *outer_elongation = ss->outer.srotation & 1;
                  goto finish;
               }
            }
            else if (ss->inner.skind == s_1x2 && ss->outer.skind == s_1x6 && ss->inner.srotation != ss->outer.srotation) {
               inners[0].kind = sdmd;
               outers->kind = s1x4;

               if ((ss->inner.srotation - ss->outer.srotation) & 2) {
                  (void) copy_rot(&inners[0], 1, ss, 1, 033);
                  (void) copy_rot(&inners[0], 3, ss, 0, 033);
               }
               else {
                  (void) copy_rot(&inners[0], 1, ss, 0, 011);
                  (void) copy_rot(&inners[0], 3, ss, 1, 011);
               }

               (void) copy_person(&inners[0], 0, ss, 14);
               (void) copy_person(&inners[0], 2, ss, 17);
               (void) copy_person(outers, 0, ss, 12);
               (void) copy_person(outers, 1, ss, 13);
               (void) copy_person(outers, 2, ss, 15);
               (void) copy_person(outers, 3, ss, 16);

               goto finish;
            }
            break;
      }
   }

   /* Next, do the 3x4 -> qtag fudging.  Don't ask permission, just do it. **** maybe that isn't right for Z calls. */

   if (analyzer_index == analyzer_NORMAL && ss->kind == s3x4) {
      inners[0].kind = s1x4;
      inners[0].rotation = ss->rotation;
      outers->kind = s2x2;
      outers->rotation = ss->rotation;
      *outer_elongation = (outers->rotation^1) & 1;
      (void) copy_person(&inners[0], 0, ss, 10);
      (void) copy_person(&inners[0], 1, ss, 11);
      (void) copy_person(&inners[0], 2, ss, 4);
      (void) copy_person(&inners[0], 3, ss, 5);

      if (!ss->people[0].id1 && ss->people[1].id1)
         (void) copy_person(outers, 0, ss, 1);
      else if (!ss->people[1].id1 && !ss->people[0].id1)
         (void) copy_person(outers, 0, ss, 0);
      else fail("Can't find centers and ends in this formation.");

      if (!ss->people[2].id1 && ss->people[3].id1)
         (void) copy_person(outers, 1, ss, 3);
      else if (!ss->people[3].id1 && ss->people[2].id1)
         (void) copy_person(outers, 1, ss, 2);
      else fail("Can't find centers and ends in this formation.");

      if (!ss->people[6].id1 && ss->people[7].id1)
         (void) copy_person(outers, 2, ss, 7);
      else if (!ss->people[7].id1 && ss->people[6].id1)
         (void) copy_person(outers, 2, ss, 6);
      else fail("Can't find centers and ends in this formation.");

      if (!ss->people[8].id1 && ss->people[9].id1)
         (void) copy_person(outers, 3, ss, 9);
      else if (!ss->people[9].id1 && ss->people[8].id1)
         (void) copy_person(outers, 3, ss, 8);
      else fail("Can't find centers and ends in this formation.");
      goto finish;
   }

   lmap_ptr = bigconctab[ss->kind][analyzer_index];
   if (!lmap_ptr) fail(conc_error_messages[analyzer_index]);

   if (lmap_ptr == &map2x4_2x4v) {
      /* See if people were facing laterally, and use the other map if so. */
      for (i=0; i<16; i++) {
         if (ss->people[i].id1 &1) { lmap_ptr = &map2x4_2x4; break; }
      }
   }


   inners[0].kind = lmap_ptr->insetup;
   inners[0].rotation = ss->rotation;
   outers->kind = lmap_ptr->outsetup;
   outers->rotation = ss->rotation;

   rot = 0;

   if (lmap_ptr->outer_rot) {
      outers->rotation--;
      rot = 011;
   }

   for (i=0; i<lmap_ptr->outlimit; i++) (void) copy_rot(outers, i, ss, lmap_ptr->mapout[i], rot);

   rot = 0;

   if (lmap_ptr->inner_rot) {
      inners[0].rotation--;
      rot = 011;
   }

   for (i=0; i<lmap_ptr->inlimit; i++) (void) copy_rot(&inners[0], i, ss, lmap_ptr->mapin[i], rot);

   if (lmap_ptr == &map_spec_star12) {
      *center_arity = 2;
      clear_people(&inners[1]);
      inners[1].setupflags = ss->setupflags;
      inners[1].kind = lmap_ptr->insetup;
      inners[1].rotation = ss->rotation;
      (void) copy_person(&inners[1], 0, ss, 10);
      (void) copy_person(&inners[1], 1, ss, 5);
      (void) copy_person(&inners[1], 2, ss, 8);
      (void) copy_person(&inners[1], 3, ss, 9);
   }
   else if (lmap_ptr == &map_spec_star16) {
      *center_arity = 3;
      clear_people(&inners[1]);
      clear_people(&inners[2]);
      inners[1].setupflags = ss->setupflags;
      inners[2].setupflags = ss->setupflags;
      inners[1].kind = lmap_ptr->insetup;
      inners[2].kind = lmap_ptr->insetup;
      inners[1].rotation = ss->rotation;
      inners[2].rotation = ss->rotation;
      (void) copy_person(&inners[1], 0, ss, 6);
      (void) copy_person(&inners[1], 1, ss, 7);
      (void) copy_person(&inners[1], 2, ss, 14);
      (void) copy_person(&inners[1], 3, ss, 15);
      (void) copy_person(&inners[2], 0, ss, 13);
      (void) copy_person(&inners[2], 1, ss, 12);
      (void) copy_person(&inners[2], 2, ss, 10);
      (void) copy_person(&inners[2], 3, ss, 11);
   }

   /* Set the outer elongation to whatever elongation the outsides really had, as indicated
      by the map. */

   *outer_elongation = (lmap_ptr->mapelong + outers->rotation) & 1;

   /* If the concept is cross-concentric, we have to set the elongation to what
         the centers (who will, of course, be going to the outside) had.
      If the original centers are in a 2x2, we set it according to the orientation
         of the entire 2x4 they were in, so that they can think about whether they were
         in lines or columns and act accordingly.  If they were not in a 2x4, that is,
         the setup was a wing or galaxy, we set the elongation to -1 to indicate an
         error.  In such a case the centers won't be able to decide whether they were
         in lines or columns. */

   if (analyzer == schema_cross_concentric) {
      *xconc_elongation = *outer_elongation;
      switch (ss->kind) {
         case s_galaxy:
         case s_rigger:
         case s_3dmd:
         case s_3x1dmd:
            *xconc_elongation = -1;    /* Can't do this! */
            break;
         case s_crosswave:
         case s_qtag:
         case s_hrglass:
         case s3x4:
            *xconc_elongation ^= 1;
            break;
      }
   }

   finish:

   canonicalize_rotation(outers);
   canonicalize_rotation(&inners[0]);
   if (*center_arity >= 2)
      canonicalize_rotation(&inners[1]);
   if (*center_arity == 3)
      canonicalize_rotation(&inners[2]);
}




int concwarn1x4table[] = {warn__xclineconc_perp, warn__lineconc_perp, warn__lineconc_par};
int concwarndmdtable[] = {warn__xcdmdconc_perp, warn__dmdconc_perp, warn__dmdconc_par};



extern void concentric_move(
   setup *ss,
   parse_block *parsein,
   parse_block *parseout,
   callspec_block *callspecin,
   callspec_block *callspecout,
   final_set final_conceptsin,
   final_set final_conceptsout,
   calldef_schema analyzer,
   defmodset modifiersin1,
   defmodset modifiersout1,
   setup *result)
{
   defmodset localmods1, localmodsin1, localmodsout1;
   setup begin_inner[3];
   setup begin_outer;
   int begin_outer_elongation;
   int begin_xconc_elongation;
   int final_elongation;
   int center_arity;
   setup result_inner[3];
   setup result_outer;
   int i, k;

   setup_kind orig_inners_start_kind;    /* The original info about the people who STARTED on the inside. */
   int orig_inners_start_dirs;           /* We don't need rotation, since we will only use this if 2x2. */
   int orig_inners_start_directions[8];

   setup_kind orig_outers_start_kind;    /* The original info about the people who STARTED on the outside. */
   int orig_outers_start_dirs;           /* We don't need rotation, since we will only use this if 2x2. */
   int orig_outers_start_directions[8];

   setup_kind final_outers_start_kind;   /* The original info about the people who will FINISH on the outside. */
   int *final_outers_start_directions;

   int final_outers_finish_dirs;         /* The final info about the people who FINISHED on the outside. */
   int final_outers_finish_directions[8];

   /* It is clearly too late to expand the matrix -- that can't be what is wanted. */
   ss->setupflags |= SETUPFLAG__NO_EXPAND_MATRIX;

   for (i=0; i<8; i++) {
      orig_inners_start_directions[i] =
      orig_outers_start_directions[i] =
      final_outers_finish_directions[i] = 0;
   }

   localmodsin1 = modifiersin1;
   localmodsout1 = modifiersout1;

   concentrify(ss, analyzer, begin_inner, &begin_outer, &center_arity, &begin_outer_elongation, &begin_xconc_elongation);

   /* Get initial info for the original ends. */
   orig_outers_start_dirs = 0;
   for (i=0; i<=setup_limits[begin_outer.kind]; i++) {
      int q = begin_outer.people[i].id1;
      orig_outers_start_dirs |= q;
      orig_outers_start_directions[(q >> 6) & 07] = q;
   }
   orig_outers_start_kind = begin_outer.kind;

   /* Get initial info for the original centers. */
   orig_inners_start_dirs = 0;
   for (i=0; i<=setup_limits[begin_inner[0].kind]; i++) {
      int q = begin_inner[0].people[i].id1;
      orig_inners_start_dirs |= q;
      orig_inners_start_directions[(q >> 6) & 07] = q;
   }
   orig_inners_start_kind = begin_inner[0].kind;

   if ((analyzer == schema_cross_concentric) || (analyzer == schema_single_cross_concentric)) {
      setup temptemp = begin_inner[0];
      begin_inner[0] = begin_outer;
      begin_outer = temptemp;

      final_outers_start_kind = orig_inners_start_kind;
      final_outers_start_directions = orig_inners_start_directions;
   }
   else {
      final_outers_start_kind = orig_outers_start_kind;
      final_outers_start_directions = orig_outers_start_directions;
   }

   begin_inner[0].setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   begin_inner[1].setupflags = ss->setupflags | SETUPFLAG__DISTORTED;
   begin_inner[2].setupflags = ss->setupflags | SETUPFLAG__DISTORTED;

   /* If the call turns out to be "detour", this will make it do just the ends part. */
   begin_outer.setupflags = ss->setupflags | SETUPFLAG__DISTORTED | SETUPFLAG__DOING_ENDS;

   /* There are two special pieces of information we now have that will help us decide where to
      put the outsides.  "Orig_outers_kind" tells what setup the outsides were originally in,
      and "begin_outer_elongation" is odd if the outsides were oriented vertically.
      "begin_outer_elongation" refers to absolute orientation, that is, "our" view of the
      setups, taking all rotations into account.  "Final_outers_start_dir" gives the individual
      orientations (absolute) of the people who are finishing on the outside.  Later, we will compute
      "final_outers_finish_dirs", telling how the individual people were oriented.  How we use all this
      information depends on many things that we will attend to below. */

   /* Giving one of the concept descriptor pointers as nil indicates that we don't want those people to do anything. */

   if (parsein) {
      for (k=0; k<center_arity; k++) {
         update_id_bits(&begin_inner[k]);
         move(&begin_inner[k], parsein, callspecin, final_conceptsin, FALSE, &result_inner[k]);
      }
   }
   else {
      for (k=0; k<center_arity; k++) {
         result_inner[k] = begin_inner[k];
         result_inner[k].setupflags = 0;
         /* Strip out the roll bits -- people who didn't move can't roll. */
         if (setup_limits[result_inner[k].kind] >= 0) {
            for (i=0; i<=setup_limits[result_inner[k].kind]; i++) {
               if (result_inner[k].people[i].id1) result_inner[k].people[i].id1 = (result_inner[k].people[i].id1 & (~ROLL_MASK)) | ROLLBITM;
            }
         }
      }
   }

   if (parseout) {
      setup begin_outer_temp;

      /* If the ends' starting setup is a 2x2, and we did not say "concentric" (indicated by
         the "concentric rules" flag being off), we mark the setup as elongated.  If the call
         turns out to be a 2-person call, the elongation will be checked against the pairings
         of people, and an error will be given if it isn't right.  This is what makes "cy-kick"
         illegal from diamonds, and "ends hinge" illegal from waves.  The reason this is turned
         off when the "concentric" concept is given is so that "concentric hinge" from waves,
         obnoxious as it may be, will be legal.
      We also turn it off if this is reverse checkpoint.  In that case, the ends know exactly
         where they should go.  This is what makes "reverse checkpoint recycle by star thru"
         work from a DPT setup. */

      if (begin_outer.kind == s2x2 && analyzer != schema_rev_checkpoint &&
            !(begin_outer_elongation & ~1)) {      /* We demand elongation be 0 or 1. */
         begin_outer.setupflags |= ((begin_outer_elongation+1) * SETUPFLAG__ELONGATE_BIT);

         /* If "demand lines" or "demand columns" has been given, we suppress elongation
            checking.  In that case,
            the database author knows what elongation is required and is taking responsibility
            for it.  This is what makes "scamper" and "divvy up" work. */

         if ((DFM1_CONC_CONCENTRIC_RULES | DFM1_CONC_DEMAND_LINES | DFM1_CONC_DEMAND_COLUMNS) & modifiersout1)
            begin_outer.setupflags |= SETUPFLAG__NO_CHK_ELONG;
      }

      begin_outer_temp = begin_outer;   /* Defend against clobbering. */

      update_id_bits(&begin_outer_temp);
      move(&begin_outer_temp, parseout, callspecout, final_conceptsout, FALSE, &result_outer);
   }
   else {
      result_outer = begin_outer;
      result_outer.setupflags = 0;
      localmodsout1 |= DFM1_CONC_FORCE_SPOTS;      /* Make sure these people go to the same spots. */
      /* Strip out the roll bits -- people who didn't move can't roll. */
      if (setup_limits[result_outer.kind] >= 0) {
         for (i=0; i<=setup_limits[result_outer.kind]; i++) {
            if (result_outer.people[i].id1) result_outer.people[i].id1 = (result_outer.people[i].id1 & (~ROLL_MASK)) | ROLLBITM;
         }
      }
   }

   /* If the call was something like "ends detour", the concentricity info was left in the
      setupflags during the execution of the call, so we have to pick it up to make sure
      that the necessary "demand" and "force" bits are honored. */
   localmodsout1 |= (begin_outer.setupflags & DFM1_CONCENTRICITY_FLAG_MASK);

   /* Check whether the necessary "demand" conditions are met.  First, set "localmods1"
      to the demand info for the call that the original ends did.  Where this comes from
      depends on whether the schema is cross concentric. */

   if ((analyzer == schema_cross_concentric) || (analyzer == schema_single_cross_concentric)) {
      localmods1 = localmodsin1;      /* Yes!  "In" describes the call for the original ends. */
   }
   else {
      localmods1 = localmodsout1;
   }

   if ((DFM1_CONC_DEMAND_LINES & localmods1) && (orig_outers_start_kind == s2x2)) {
      /* We make use of the fact that the setup, being a 2x2, is canonicalized. */
      if ((begin_outer_elongation < 0) ||
            (orig_outers_start_dirs & (1 << 3*(begin_outer_elongation & 1))))
         fail("Outsides must be as if in lines at start of this call.");
   }
   
   if ((DFM1_CONC_DEMAND_COLUMNS & localmods1) && (orig_outers_start_kind == s2x2)) {
      if ((begin_outer_elongation < 0) ||
            (orig_outers_start_dirs & (8 >> 3*(begin_outer_elongation & 1))))
         fail("Outsides must be as if in columns at start of this call.");
   }

   /* Now check whether there are any demands on the original centers.  The interpretation
      of "lines" and "columns" is slightly different in this case.  We apply the test only if
      the centers are in a 2x2, but we don't care about the outsides' setup, as long as it
      has a measurable elongation.  If the outsides are also in a 2x2, so that the whole setup
      is a 2x4, these tests will do just what they say -- they will check whether the centers
      believe they are in lines or columns.  However, if the outsides are in a 1x4, so the
      overall setup is a "rigger", we simply test the outsides' elongation.  In such a case
      "demand lines" means "demand outsides lateral to me". */

   if ((analyzer == schema_cross_concentric) || (analyzer == schema_single_cross_concentric)) {
      localmods1 = localmodsout1;
   }
   else {
      localmods1 = localmodsin1;
   }

   if ((DFM1_CONC_DEMAND_LINES & localmods1) && (orig_inners_start_kind == s2x2)) {
      if ((begin_outer_elongation < 0) ||
            (orig_inners_start_dirs & (1 << 3*(begin_outer_elongation & 1))))
         fail("Centers must be as if in lines at start of this call.");
   }
   
   if ((DFM1_CONC_DEMAND_COLUMNS & localmods1) && (orig_inners_start_kind == s2x2)) {
      if ((begin_outer_elongation < 0) ||
            (orig_inners_start_dirs & (8 >> 3*(begin_outer_elongation & 1))))
         fail("Centers must be as if in columns at start of this call.");
   }

   localmods1 = localmodsout1;

   final_outers_finish_dirs = 0;
   for (i=0; i<=setup_limits[result_outer.kind]; i++) {
      int q = result_outer.people[i].id1;
      final_outers_finish_dirs |= q;
      final_outers_finish_directions[(q >> 6) & 07] = q;
   }

   /* Now final_outers_finish_dirs tells whether outer peoples' orientations changed.
      This is only meaningful if outer setup is 2x2.  Note that, if the setups
      are 2x2's, canonicalization sets their rotation to zero, so the
      tbonetest quantities refer to absolute orientation. */
   
   /* The time has come to compute the elongation of the outsides in the final setup.
      This gets complicated if the outsides' final setup is a 2x2.  Among the
      procedures we could use are:
         (1) if the call is "checkpoint", go to spots with opposite elongation
            from the original outsides' elongation.  This is the "Hodson checkpoint
            rule", named after the caller who first used a consistent, methodical,
            and universal rule for the checkpoint concept.
         (2) if the call is "concentric", use the Hodson rule if the original setup
            was a 1x4 or diamond, or the "lines-to-lines, columns-to-columns" rule
            if the original setup was a 2x2.
         (3) if we have various definition flags, such as "force_lines" or
            "force_otherway", obey them.
      We will use information from several sources in carrying out these rules.
      The concentric concept will signify itself by turning on the "lines_lines"
      flag.  The checkpoint concept will signify itself by turning on the
      "force_otherway" flag.  The "parallel_conc_end" flag in the outsides' setup
      indicates that, if "concentric" or "checkpoint" are NOT being used, the call
      wants the outsides to maintain the same elongation as they had at the beginning.
      This is what makes "ends hinge" and "ends recycle" do their respective
      right things when called from a grand wave. */

   /* Default: the ends just keep their original elongation.  This will often
      mean that they stay on their spots. */

   if ((analyzer == schema_cross_concentric) || (analyzer == schema_single_cross_concentric))
      final_elongation = begin_xconc_elongation;
   else
      final_elongation = begin_outer_elongation;

   /* Note: final_elongation might be -1 now, meaning that the people on the outside
      cannot determine their elongation from the original setup.  Unless their
      final setup is one that does not require knowing the value of final_elongation,
      it is an error. */

   if (result_outer.kind == nothing) {
      if (result_inner[0].kind == nothing) {
         result->kind = nothing;    /* If everyone is a phantom, it's simple. */
         return;
      }

      /* If the schema is one of the special ones, we will know what to do. */
      if (  analyzer == schema_conc_star ||
            analyzer == schema_ckpt_star ||
            analyzer == schema_conc_star12 ||
            analyzer == schema_conc_star16) {

         /* This is what makes 12 matrix relay the top work when everyone is
            in the stars. */

         result_outer.kind = s1x4;
         clear_people(&result_outer);
         result_outer.setupflags = 0;
         result_outer.rotation = ss->rotation;
      }
      else if (analyzer == schema_concentric_diamond_line) {
         if (ss->kind == s_wingedstar || ss->kind == s_wingedstar12 || ss->kind == s_wingedstar16) {
            result_outer.kind = s2x2;
            result_outer.rotation = 0;
            clear_people(&result_outer);
            /* Set their "natural" elongation perpendicular to their original diamond.
               The test for this is 1P2P; touch 1/4; column circ; boys truck; split phantom
               lines tag chain thru reaction.  They should finish in outer triple boxes,
               not a 2x4. */
            result_outer.setupflags = (result_inner[0].setupflags & ~RESULTFLAG__ELONGATE_MASK) |
                  (((~ss->rotation & 1) + 1) * RESULTFLAG__ELONGATE_BIT);
         }
         else
            goto no_end_err;
      }
      else {
         /* We may be in serious trouble -- we have to figure out what setup the ends
            finish in, and they are all phantoms.  We can save the day only if we
            can convince ourselves that they did the call "nothing".  We make use
            of the fact that "concentrify" did NOT flush them, so we still know
            what their starting setup was.
         This is what makes split phantom diamonds diamond chain through work
            from columns far apart. */
   
         result_outer = begin_outer;               /* Restore the original bunch of phantoms. */
         /* Make sure these people go to the same spots, and remove possibly misleading info. */
         localmods1 |= DFM1_CONC_FORCE_SPOTS;
         localmods1 &= ~(DFM1_CONC_FORCE_LINES | DFM1_CONC_FORCE_COLUMNS | DFM1_CONC_FORCE_OTHERWAY);
   
         if (parseout && callspecout && (callspecout->schema == schema_nothing))
            ;        /* It's OK. */
         else if (center_arity > 1)
            ;        /* It's OK. */
         else {
            /* We simply have no idea where the outsides should be.  We
               simply contract the setup to a 4-person setup (or whatever),
               throwing away the outsides completely.  If this was an
               "on your own", it may be possible to put things back together.
               This is what makes "1P2P; pass thru; ENDS leads latch on;
               ON YOUR OWN disband & snap the lock" work.  But if we try to glue
               these setups together, "fix_n_results" will raise an error, since
               it won't know whether to leave room for the phantoms. */

            *result = result_inner[0];   /* This gets all the inner people. */
            result->kind = s_normal_concentric;
            result->inner.skind = result_inner[0].kind;
            result->inner.srotation = result_inner[0].rotation;
            result->outer.skind = nothing;
            result->outer.srotation = 0;
            result->outer_elongation = 0;
            canonicalize_rotation(result);
            return;
         }
      }
   }
   else if (result_inner[0].kind == nothing) {
      /* If the schema is one of the special ones, we will know what to do. */
      if (  analyzer == schema_conc_star ||
            analyzer == schema_ckpt_star ||
            analyzer == schema_conc_star12 ||
            analyzer == schema_conc_star16) {
                  ;        /* Take no action. */
      }
      /* If the ends are a 2x2, we just set the missing centers to a 2x2.
         The ends had better know their elongation, of course.  It shouldn't
         matter to the ends whether the phantoms in the center did something
         that leaves the whole setup as diamonds or as a 2x4.  (Some callers
         might think it matters (Hi, Clark!) but it doesn't matter to this program.)
         This is what makes split phantom diamonds diamond chain through work
         from a grand wave. */
      else if (result_outer.kind == s2x2 && center_arity == 1) {
         result_inner[0].kind = s2x2;
         clear_people(&result_inner[0]);
         result_inner[0].setupflags = 0;
         result_inner[0].rotation = 0;
      }
      /* If the ends are a 1x4, we just set the missing centers to a 1x4,
         so the entire setup is a 1x8.  Maybe the phantoms went to a 2x2,
         so the setup is really a rigger, but we don't care.  See the comment
         just above.  This is what makes "1P2P; pass thru; ENDS leads latch on;
         ON YOUR OWN disband & snap the lock" work. */
      else if (result_outer.kind == s1x4 && center_arity == 1) {
         result_inner[0].kind = s2x2;
         clear_people(&result_inner[0]);
         result_inner[0].setupflags = 0;
         result_inner[0].rotation = result_outer.rotation;
      }
      else {
         /* The centers are just gone!  It is quite possible that "fix_n_results"
            may be able to repair this damage by copying some info from another setup.
            Missing centers are not as serious as missing ends, because they won't
            lead to indecision about whether to leave space for the phantoms. */

         int j;
         result->kind = s_normal_concentric;
         result->outer.skind = result_outer.kind;
         result->outer.srotation = result_outer.rotation;
         result->inner.skind = nothing;
         result->inner.srotation = 0;
         result->outer_elongation = 0;

         for (j=0; j<12; j++) (void) copy_person(result, j+12, &result_outer, j);
         canonicalize_rotation(result);
         return;
      }
   }

/* ***** We used to have this:
   if (result_outer.kind == s_short6) {
      switch (final_outers_start_kind) {
         case s_bone6:
            final_elongation ^= 1;     Natural elongation is wrong way!!
            break;
      }
   }
which is wrong.  The fact that short6 has a funny definition is taken care of in
normalize_concentric.  This code was making "outer 6 convert the triangle" fail
from a bone (heads left swing thru, side girl turn back).
***** */

   /* At this point, "final_elongation" actually has the INITIAL elongation of the
      people who finished on the outside.  That is, if they went from a wave or diamond
      to a 2x2, it has the elongation of their initial wave or diamond points.
      
      The elongation bits in their setup tells how they "naturally" wanted to end,
      based on the call they did, how it got divided up, whether it had the "parallel_conc_end"
      flag on, etc.
      
      We will use both pieces of information to figure out how to elongate the outsides at
      the conclusion of the call.  For example, if the word "concentric" was not spoken,
      we will just use their "natural" elongation from the setup.  This is what makes
      "ends hinge" work from a grand wave.  If the word "concentric" was spoken, their
      natural elongation is discarded, and we will set them perpendicular to their
      original 1x4 or diamond, using the value in "final_elongation"  If invocation
      flags like "force lines" or "force columns" are present, we will use those.
      
      When we are done, our final judgement will be put back into the variable
      "final_elongation". */

   if (result_outer.kind == s2x2) {
      int *concwarntable = (final_outers_start_kind == s1x4) ? concwarn1x4table : concwarndmdtable;

      switch (final_outers_start_kind) {
         case s1x4: case sdmd:

            /* Outers' call has gone from a 1x4 or diamond to a 2x2.  The rules are:
               (1) The "force_columns" or "force_lines" flag in the invocation takes precedence
                  over anything else.
               (2) If the "concentric rules" flag is on (that flag is a euphemism for "the
                  concentric or checkpoint concept is explicitly in use here"), we set the
                  elongation perpendicular to the original 1x4 or diamond.
               (3) If the "force_otherway" invocation flag is on, meaning the database
                  really wants us to, we set the elongation perpendicular to the original
                  1x4 or diamond.  *** Actually, it appears that this flag is meaningless
                  for 1x4/dmd -> 2x2 calls, and the "par_conc_end" does this function
               (4) Otherwise, we set the elongation to the natural elongation that the people
                  went to.  This uses the result of the "par_conc_end" flag for 1x4/dmd -> 2x2
                  calls, or the manner in which the setup was divided for calls that were put
                  together from 2-person calls, or whatever.  (For 1x4->2x2 calls, the "par_conc_end"
                  flag means the call prefers the SAME elongation in the resulting 2x2.  The default,
                  absent this flag, is to change the elongation.  In any case, the result of all that
                  has been encoded into the elongation of the 2x2 setup that the people went to;
                  we just have to obey. */

            if (DFM1_CONC_FORCE_LINES & localmods1) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in lines - they are T-boned.");
               final_elongation = final_outers_finish_dirs & 1;
            }
            else if (DFM1_CONC_FORCE_COLUMNS & localmods1) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in columns - they are T-boned.");
               final_elongation = (final_outers_finish_dirs+1) & 1;
            }
            else if ((DFM1_CONC_CONCENTRIC_RULES | DFM1_CONC_FORCE_OTHERWAY) & localmods1) {
               if (analyzer == schema_cross_concentric)
                  warn(concwarntable[0]);
               else
                  warn(concwarntable[1]);

               final_elongation ^= 1;
            }
            else {
               /* Get the elongation from the result setup, if possible. */
               unsigned long int newelong = ((result_outer.setupflags & RESULTFLAG__ELONGATE_MASK) / RESULTFLAG__ELONGATE_BIT) - 1;

               if (result_outer.setupflags & RESULTFLAG__ELONGATE_MASK) {
                  if (final_elongation == newelong) {
                     warn(concwarntable[2]);
                  }
                  else {
                     if (analyzer == schema_cross_concentric)
                        warn(concwarntable[0]);
                     else
                        warn(concwarntable[1]);
                  }
               }

               final_elongation = newelong;
            }

            break;
         case s2x2:
            /* If call went from 2x2 to 2x2, the rules are:
               First, check for "force_columns" or "force_lines" in the invocation.  This is not
                  a property of the call that we did, but of the way its parent (or the concept) invoked it.
               Second, check for "force_spots" or "force_otherway" in the invocation.  This is not
                  a property of the call that we did, but of the way its parent (or the concept) invoked it.
               Third, check for "lines_lines" in the invocation.  This is not
                  a property of the call that we did, but of the way its parent (or the concept) invoked it.
                  If the concept was "concentric", it will be on, of course.
               Finally, check the elongation bits in the result flags left over from the call.  These tell
                  whether to work to spots, or antispots, or whatever, based what the call was, and whether
                  it, or various sequential parts of it, had the "parallel_conc_end" flag on.
               If there are no elongation bits, we simply don't know waht to do.

               Note that the "ends do thus-and-so" concept does NOT set the lines_lines flag in the
                  invocation, so we work to spots unless the call says "parallel_conc_end".  Counter-rotate,
                  for example, says "parallel_conc_end", so it works to antispots. */

            if (DFM1_CONC_FORCE_LINES & localmods1) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in lines - they are T-boned.");
               final_elongation = final_outers_finish_dirs & 1;
            }
            else if (DFM1_CONC_FORCE_COLUMNS & localmods1) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in columns - they are T-boned.");
               final_elongation = (final_outers_finish_dirs+1) & 1;
            }
            else if (DFM1_CONC_FORCE_OTHERWAY & localmods1)
               final_elongation ^= 1;
            else if (DFM1_CONC_FORCE_SPOTS & localmods1)
               ;           /* It's OK the way it is. */
            else if (DFM1_CONC_CONCENTRIC_RULES & localmods1) {       /* do "lines-to-lines / columns-to-columns" */
               int new_elongation = -1;

               for (i=0; i<8; i++) {
                  if (final_outers_finish_directions[i]) {
                     int t = (final_outers_start_directions[i] ^ final_outers_finish_directions[i] ^ final_elongation) & 1;
                     if (t != new_elongation) {
                        if (new_elongation >= 0)
                           fail("Sorry, outsides would have to go to a 'pinwheel', can't handle that.");
                        new_elongation = t;
                     }
                  }
               }

               /* The warning "warn__ends_work_to_spots" is now obsolete. */

               final_elongation = new_elongation;
            }
            else
               final_elongation = ((result_outer.setupflags & RESULTFLAG__ELONGATE_MASK) / RESULTFLAG__ELONGATE_BIT) - 1;

            break;
         default:
            fail("Don't recognize starting setup.");
      }
   }

   /* Now lossage in "final_elongation" may have been repaired.  If it is still
      negative, there may be trouble ahead. */

   normalize_concentric(analyzer, center_arity, result_inner, &result_outer, final_elongation, result);
   return;

   no_end_err:
   fail("Can't figure out ending setup for concentric call -- no ends.");
}


/* This overwrites its first argument setup. */
extern void merge_setups(setup *ss, setup *result)
{
   int i, r, rot, offs, limit, lim1, limhalf;
   setup res2copy;
   setup *res1, *res2;

   res2copy = *result;
   res1 = ss;
   res2 = &res2copy;

   canonicalize_rotation(res1);    /* Do we really need to do this before normalize_setup? */
   normalize_setup(res1, normalize_before_merge);
   canonicalize_rotation(res1);    /* We definitely need to do it now -- a 2x2 might have been created. */

   tryagain:

   canonicalize_rotation(res2);
   normalize_setup(res2, normalize_before_merge);
   canonicalize_rotation(res2);

   /* Canonicalize the setups according to their kind.  This is a bit sleazy, since
      the enumeration order of setup kinds is supposed to be insignificant.  We depend in
      general on small setups being before larger ones.  In particular, we seem to require:
         s2x2 < s2x4
         s2x2 < s2x6
         s2x2 < s1x8
         s1x4 < s1x8
         s1x4 < s2x4
         s2x4 < s_c1phan
         s2x4 < s2x6
      You get the idea. */

   if (res2->kind < res1->kind) {
      setup *temp = res2;
      res2 = res1;
      res1 = temp;
   }

   /* If one of the setups was a "concentric" setup in which there are no ends, we can still handle it. */

   if (res2->kind == s_normal_concentric && res2->outer.skind == nothing) {
      res2->kind = res2->inner.skind;
      res2->rotation = res2->inner.srotation;
      goto tryagain;    /* Need to recanonicalize setup order. */
   }

   result->rotation = res2->rotation;

   r = (res1->rotation - res2->rotation) & 3;
   rot = r * 011;
   
   if (res1->kind == nothing) {
      *result = *res2;
      return;
   }
   else if ((res1->kind == s2x4) && (res2->kind == s2x4) && (r&1)) {
      offs = r * 2;
      if ((((res1->people[0].id1 & res1->people[1].id1 & res1->people[4].id1 & res1->people[5].id1) |
               (res1->people[2].id1 & res1->people[3].id1 & res1->people[6].id1 & res1->people[7].id1)) & BIT_PERSON) &&
         (((res2->people[0].id1 & res2->people[1].id1 & res2->people[4].id1 & res2->people[5].id1) |
               (res2->people[2].id1 & res2->people[3].id1 & res2->people[6].id1 & res2->people[7].id1)) & BIT_PERSON)) {
         result->kind = s_c1phan;
         (void) copy_person(result, 10, res2, 5);
         (void) copy_person(result, 5, res2, 3);
         (void) copy_person(result, 13, res2, 7);
         (void) copy_person(result, 7, res2, 2);
         (void) copy_person(result, 2, res2, 1);
         (void) copy_person(result, 8, res2, 4);
         (void) copy_person(result, 15, res2, 6);
         clear_person(result, 1);
         clear_person(result, 3);
         clear_person(result, 4);
         clear_person(result, 6);
         clear_person(result, 9);
         clear_person(result, 11);
         clear_person(result, 12);
         clear_person(result, 14);
         install_rot(result, 11, res1, 0^offs, rot);
         install_rot(result, 9, res1, 1^offs, rot);
         install_rot(result, 4, res1, 2^offs, rot);
         install_rot(result, 6, res1, 3^offs, rot);
         install_rot(result, 3, res1, 4^offs, rot);
         install_rot(result, 1, res1, 5^offs, rot);
         install_rot(result, 12, res1, 6^offs, rot);
         install_rot(result, 14, res1, 7^offs, rot);
      }
      else {
         result->kind = s4x4;
         clear_person(result, 12);
         clear_person(result, 13);
         clear_person(result, 14);
         clear_person(result, 0);
         clear_person(result, 4);
         clear_person(result, 5);
         clear_person(result, 6);
         clear_person(result, 8);
         (void) copy_person(result, 10, res2, 0);
         (void) copy_person(result, 15, res2, 1);
         (void) copy_person(result, 3, res2, 2);
         (void) copy_person(result, 1, res2, 3);
         (void) copy_person(result, 2, res2, 4);
         (void) copy_person(result, 7, res2, 5);
         (void) copy_person(result, 11, res2, 6);
         (void) copy_person(result, 9, res2, 7);

         install_rot(result, 7, res1, 0^offs, rot);
         install_rot(result, 5, res1, 1^offs, rot);
         install_rot(result, 14, res1, 2^offs, rot);
         install_rot(result, 3, res1, 3^offs, rot);
         install_rot(result, 15, res1, 4^offs, rot);
         install_rot(result, 13, res1, 5^offs, rot);
         install_rot(result, 6, res1, 6^offs, rot);
         install_rot(result, 11, res1, 7^offs, rot);
      }
      return;
   }
   else if (res2->kind == s_c1phan && res1->kind == s2x4) {
      result->kind = s_c1phan;
      for (i=0; i<16; i++)
         (void) copy_person(result, i, res2, i);

      result->rotation -= r;
      canonicalize_rotation(result);

      install_person(result, 0, res1, 0);
      install_person(result, 2, res1, 1);
      install_person(result, 7, res1, 2);
      install_person(result, 5, res1, 3);
      install_person(result, 8, res1, 4);
      install_person(result, 10, res1, 5);
      install_person(result, 15, res1, 6);
      install_person(result, 13, res1, 7);

      result->rotation += r;
      canonicalize_rotation(result);
      return;
   }
   else if (res2->kind == s_qtag && res1->kind == s_1x2 &&
            (!(res2->people[3].id1 | res2->people[7].id1))) {
      res2->kind = s_short6;
      res2->rotation++;
      (void) copy_rot(res2, 3, res2, 5, 033);         /* careful -- order is important */
      (void) copy_rot(res2, 5, res2, 0, 033);
      (void) copy_rot(res2, 0, res2, 1, 033);
      (void) copy_rot(res2, 1, res2, 2, 033);
      (void) copy_rot(res2, 2, res2, 4, 033);
      (void) copy_rot(res2, 4, res2, 6, 033);
      normalize_concentric(schema_concentric_2_6, 1, res1, res2, res2->rotation, result);
      return;
   }
   else if (res2->kind == s2x4 && res1->kind == s2x2) {
      result->kind = s2x4;
      for (i=0; i<8; i++)
         (void) copy_person(result, i, res2, i);

      res1->rotation += r;
      canonicalize_rotation(res1);

      install_person(result, 1, res1, 0);
      install_person(result, 2, res1, 1);
      install_person(result, 5, res1, 2);
      install_person(result, 6, res1, 3);

      canonicalize_rotation(result);
      return;
   }
   else if (res2->kind == s4x4 && res1->kind == s2x4) {
      *result = *res2;
      result->rotation -= r;
      canonicalize_rotation(result);

      install_person(result, 10, res1, 0);
      install_person(result, 15, res1, 1);
      install_person(result, 3,  res1, 2);
      install_person(result, 1,  res1, 3);
      install_person(result, 2,  res1, 4);
      install_person(result, 7,  res1, 5);
      install_person(result, 11, res1, 6);
      install_person(result, 9,  res1, 7);

      result->rotation += r;
      canonicalize_rotation(result);
      return;
   }
   else if (res2->kind == s2x6 && res1->kind == s2x2) {
      result->kind = s2x6;
      for (i=0; i<12; i++)
         (void) copy_person(result, i, res2, i);

      res1->rotation += r;
      canonicalize_rotation(res1);

      install_person(result, 2, res1, 0);
      install_person(result, 3, res1, 1);
      install_person(result, 8, res1, 2);
      install_person(result, 9, res1, 3);

      canonicalize_rotation(result);
      return;
   }
   else if (res2->kind == s1x8 && res1->kind == s2x2 &&
            (!(res2->people[2].id1 | res2->people[3].id1 | res2->people[6].id1 | res2->people[7].id1))) {
      res2->kind = s1x4;
      (void) copy_person(res2, 2, res2, 4);
      (void) copy_person(res2, 3, res2, 5);
      normalize_concentric(schema_concentric, 1, res1, res2, 0, result);
      return;
   }
   else if (res1->kind == s_rigger &&
            (!(res1->people[0].id1 | res1->people[1].id1 | res1->people[4].id1 | res1->people[5].id1))) {
      res1->kind = s1x4;
      (void) copy_person(res1, 0, res1, 6);
      (void) copy_person(res1, 1, res1, 7);
      normalize_concentric(schema_concentric, 1, res2, res1, 0, result);
      return;
   }
   else if (res2->kind == s1x8 && res1->kind == s1x4 &&
            (!(res2->people[2].id1 | res2->people[3].id1 | res2->people[6].id1 | res2->people[7].id1))) {
      res2->kind = s1x4;
      (void) copy_person(res2, 2, res2, 4);
      (void) copy_person(res2, 3, res2, 5);
      normalize_concentric(schema_concentric, 1, res1, res2, 0, result);
      return;
   }
   else if (res2->kind == s2x4 && res1->kind == s1x4 &&
            (!(res2->people[1].id1 | res2->people[2].id1 | res2->people[5].id1 | res2->people[6].id1))) {
      int outer_elongation = res2->rotation & 1;
      res2->kind = s2x2;
      (void) copy_person(res2, 1, res2, 3);
      (void) copy_person(res2, 2, res2, 4);
      (void) copy_person(res2, 3, res2, 7);
      canonicalize_rotation(res2);
      normalize_concentric(schema_concentric, 1, res1, res2, outer_elongation, result);
      return;
   }
   else if (res2->kind == s_ptpd && res1->kind == s1x8 && r == 0 &&
            (!(res1->people[1].id1 | res1->people[3].id1 | res1->people[5].id1 | res1->people[7].id1))) {
      *result = *res2;

      install_person(result, 0, res1, 0);
      install_person(result, 2, res1, 2);
      install_person(result, 4, res1, 4);
      install_person(result, 6, res1, 6);
      return;
   }
   else if (res2->kind == s_ptpd && res1->kind == s1x8 && r == 0 &&
            (!(res2->people[1].id1 | res2->people[3].id1 | res2->people[5].id1 | res2->people[7].id1))) {
      *result = *res1;

      install_person(result, 0, res2, 0);
      install_person(result, 2, res2, 2);
      install_person(result, 4, res2, 4);
      install_person(result, 6, res2, 6);
      return;
   }
   else if (res2->kind == s2x6 && res1->kind == s2x4 && r == 0) {
      /* Because of canonicalization, we know that r = 0 unless
         they are 90 degrees from each other. */
      *result = *res2;

      install_person(result, 1, res1, 0);
      install_person(result, 2, res1, 1);
      install_person(result, 3, res1, 2);
      install_person(result, 4, res1, 3);
      install_person(result, 7, res1, 4);
      install_person(result, 8, res1, 5);
      install_person(result, 9, res1, 6);
      install_person(result, 10, res1, 7);
      return;
   }
   
   /* This is sleazy. */

   limit = setup_limits[res1->kind];
   lim1 = limit+1;
   limhalf = lim1 >> 1;

   /* The only remaining hope is that the setups match and we can blindly combine them.  We require
      lim1 even because our 180 degree rotation wouldn't work for triangles. */

   if (res1->kind != res2->kind || r & 1 || limit < 0 || lim1 & 1)
      fail("Can't figure out result setup.");
   
   *result = *res2;

   if (r) {
      for (i=0; i<limit+1; i++)
         install_rot(result, i, res1, (i+limhalf) % lim1, rot);
   }
   else {
      for (i=0; i<limit+1; i++)
         install_person(result, i, res1, i);
   }
}


extern void on_your_own_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   setup setup1, setup2, res1;

   if (ss->kind != s2x4) fail("Must have 2x4 setup for 'on your own'.");
   
   setup1 = *ss;              /* Get outers only. */
   clear_person(&setup1, 1);
   clear_person(&setup1, 2);
   clear_person(&setup1, 5);
   clear_person(&setup1, 6);
   setup1.setupflags = ss->setupflags | SETUPFLAG__DISTORTED | SETUPFLAG__PHANTOMS;
   move(&setup1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);

   setup2 = *ss;              /* Get inners only. */
   clear_person(&setup2, 0);
   clear_person(&setup2, 3);
   clear_person(&setup2, 4);
   clear_person(&setup2, 7);
   setup2.setupflags = ss->setupflags | SETUPFLAG__DISTORTED | SETUPFLAG__PHANTOMS;
   move(&setup2, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, result);
   result->setupflags |= res1.setupflags;
   
   merge_setups(&res1, result);
}



extern void so_and_so_only_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)

{
   selector_kind saved_selector;
   int i;
   long_boolean others;
   setup setup1, setup2, res1;

   saved_selector = current_selector;
   others = parseptr->concept->value.arg1;
   current_selector = parseptr->selector;

   if (current_selector == selector_all || current_selector == selector_none)
      fail("Can't have 'everyone' or 'no one' do a call.");

   setup1 = *ss;              /* designees */
   setup2 = *ss;              /* non-designees */
   
   if (setup_limits[ss->kind] < 0) fail("Can't identify people in this setup.");
   for (i=0; i<setup_limits[ss->kind]+1; i++) {
      if (ss->people[i].id1) {
         if (selectp(ss, i))
            clear_person(&setup2, i);
         else
            clear_person(&setup1, i);
      }
   }
   
   current_selector = saved_selector;
   
   normalize_setup(&setup1, normalize_before_isolated_call);
   normalize_setup(&setup2, normalize_before_isolated_call);
   setup1.setupflags = ss->setupflags | SETUPFLAG__PHANTOMS;
   setup2.setupflags = ss->setupflags | SETUPFLAG__PHANTOMS;
   
   move(&setup1, parseptr->next, NULLCALLSPEC, 0, FALSE, &res1);
   
   if (others) {
      move(&setup2, parseptr->subsidiary_root, NULLCALLSPEC, 0, FALSE, result);

      if ((res1.setupflags ^ result->setupflags) & RESULTFLAG__DID_LAST_PART)
         fail("Two calls must use the same number of fractions.");
   
      result->setupflags |= res1.setupflags;
   }
   else {
      *result = setup2;
      result->setupflags = res1.setupflags;
   }

   merge_setups(&res1, result);
}
