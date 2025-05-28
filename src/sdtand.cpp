// -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:3; fill-column:88 -*-

// SD -- square dance caller's helper.
//
//    Copyright (C) 1990-2007  William B. Ackerman.
//
//    This file is part of "Sd".
//
//    Sd is free software; you can redistribute it and/or modify it
//    under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Sd is distributed in the hope that it will be useful, but WITHOUT
//    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
//    License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Sd; if not, write to the Free Software Foundation, Inc.,
//    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    This is for version 37.


// This file contains stuff for tandem and as-couples moves.

/* This defines the following functions:
   tandem_couples_move
   initialize_tandem_tables
*/

#include <string.h>
#include "sd.h"


struct tandrec {
   personrec real_saved_people[8][MAX_PEOPLE];
   int saved_originals[MAX_PEOPLE];
   setup virtual_setup;
   setup virtual_result;
   int vertical_people[MAX_PEOPLE];    /* 1 if original people were near/far; 0 if lateral */
   uint32 single_mask;
   bool no_unit_symmetry;
   bool phantom_pairing_ok;
   int np;
};


struct tm_thing {
   veryshort maps[32];

   // Laterally grouped people in inside numbering.
   // These are pairs; only low bit of each pair is used (except triangles.)

   // These are triads; usually only the middle bit of each triad is used.
   // (That is, the other bits are zero.)
   // For triangles the highest bit is also used.  For "1/8 twosome" things,
   // the lowest bit is used.
   uint32 ilatmask3high;
   uint32 ilatmask3low;

   uint32 olatmask;
   int limit;
   int rot;

   setup_kind insetup;
   setup_kind outsetup;

   // Things below here are not specified in the tables, but are filled in
   // by "initialize_one_table".

   // Relative to insetup numbering, those people that are NOT paired.
   // These are triads; only low bit of each triad is used; others are zero.
   uint32 insinglemaskhigh;
   uint32 insinglemasklow;

   // Relative to outsetup numbering, those people that are NOT paired.
   uint32 outsinglemask;

   uint32 outunusedmask;
   bool map_is_eighth_twosome;
};



static tm_thing maps_isearch_twosome[] = {
   //       maps                                                      ilatmaskH/L    olatmask   limit rot insetup outsetup

   {{7, 6, 4, 5,                     0, 1, 3, 2},                            0,0,     0000,         4, 0,  s1x4,  s2x4},            /* "2x4_4" - see below */
   {{0, 2, 5, 7,                     1, 3, 4, 6},                        0,02222,     0xFF,         4, 0,  s2x2,  s2x4},
   {{2, 5, 7, 0,                     3, 4, 6, 1},                            0,0,     0xFF,         4, 1,  s2x2,  s2x4},

   // This one gets h1p, far box star thru, as couples touch
   {{7, 6, 2, 5,                     0, 1, 3, 4},                        0,00022,     0x3C,         4, 1, s_trngl4, s2x4},
   // This one gets h1p, near box star thru, as couples touch
   {{4, 5, 7, 0,                     3, 2, 6, 1},                        0,00022,     0xC3,         4, 3, s_trngl4, s2x4},

   // This one comes back from far box
   {{0, 2, 4, 6,                     1, 3, 5, 7},                        0,02200,     0xF0,      4, 0, s_trngl4, s_trngl8},



   // Special maps for couples 1/8 twosome stuff.
   // These need further work in the table initializer.
   {{12, 3, 7, 8,                    15, 0, 4, 11},                      0,01313,   0xFFFF,         4, 0,  s2x2,  s4x4},
   {{10, 14, 5, 9,                   13, 1, 2, 6},                       0,03131,   0xFFFF,         4, 0,  s2x2,  s4x4},
   {{12, 14, 7, 9,                   15, 1, 4, 6},                       0,03333,   0xFFFF,         4, 0,  s2x2,  s4x4},
   {{10, 3, 5, 8,                    13, 0, 2, 11},                      0,01111,   0xFFFF,         4, 0,  s2x2,  s4x4},
   {{15, 13, 9, 11,                  1, 3, 7, 5},                        0,01111,   0xFFFF,         4, 0,  s1x4,  s2x8},
   {{0, 2, 6, 4,                     14, 12, 8, 10},                     0,03333,   0xFFFF,         4, 0,  s1x4,  s2x8},
   {{15, 2, 9, 4,                    1, 12, 7, 10},                      0,03131,   0xFFFF,         4, 0,  s1x4,  s2x8},
   {{0, 13, 6, 11,                   14, 3, 8, 5},                       0,01313,   0xFFFF,         4, 0,  s1x4,  s2x8},
   {{0, 2,                           6, 4},                              0,033,       0xFF,         2, 0,  s1x2,  s2x4},
   {{7, 5,                           1, 3},                              0,011,       0xFF,         2, 0,  s1x2,  s2x4},
   {{7, 2,                           1, 4},                              0,031,       0xFF,         2, 0,  s1x2,  s2x4},
   {{0, 5,                           6, 3},                              0,013,       0xFF,         2, 0,  s1x2,  s2x4},


   {{3, 2,                           0, 1},                                  0,0,     0000,         2, 0,  s1x2,  s2x2},
   {{0, 3,                           1, 2},                                  0,0,      0xF,         2, 1,  s1x2,  s2x2},
   {{0, 3,                           1, 2},                                0,022,      0xF,         2, 0,  s1x2,  s1x4},
   {{0,                              1},                                    0,02,     0003,         1, 0,  s1x1,  s1x2},
   {{0,                              1},                                     0,0,     0003,         1, 1,  s1x1,  s1x2},
   {{0, 3, 5, 6,                     1, 2, 4, 7},                        0,02222,     0xFF,         4, 0,  s1x4,  s1x8},
   {{0, 2, 4, 7, 9, 11,              1, 3, 5, 6, 8, 10},               0,0222222,    0xFFF,         6, 0,  s1x6,  s1x12},
   {{0, 2, 6, 4, 9, 11, 15, 13,      1, 3, 7, 5, 8, 10, 14, 12},       0,022222222,  0xFFFF,        8, 0,  s1x8,  s1x16},
   {{15, 14, 12, 13, 8, 9, 11, 10,   0, 1, 3, 2, 7, 6, 4, 5},                0,0,     0000,         8, 0,  s1x8,  s2x8},
   {{11, 10, 9, 6, 7, 8,             0, 1, 2, 5, 4, 3},                      0,0,     0000,         6, 0,  s1x6,  s2x6},

   {{10, 15, 3, 1, 4, 5, 6, 8,       12, 13, 14, 0, 2, 7, 11, 9},            0,0,     0000,         8, 0,  s2x4,  s4x4},
   {{14, 3, 7, 5, 8, 9, 10, 12,      0, 1, 2, 4, 6, 11, 15, 13},             0,0,   0xFFFF,         8, 1,  s2x4,  s4x4},
   {{-2, 15, 3, 1, -2, 5, 11, 9,     -1, 13, -1, -1, -1, 7, -1, -1},         0,0,     0000,         8, 0,  s2x4,  s4x4},
   {{10, 15, 3, -2, 2, 7, 6, -2,     -1, -1, 14, -1, -1, -1, 11, -1},        0,0,     0000,         8, 0,  s2x4,  s4x4},

   // When analyzing, we prefer the 4x6->3x4 formulation.  But we can synthesize
   // from a qtag.
   {{4, 7, 22, 13, 15, 20, 17, 18, 11, 0, 2, 9,
     5, 6, 23, 12, 14, 21, 16, 19, 10, 1, 3, 8},                             0,0, 0xFFFFFF,        12, 1,  s3x4,  s4x6},
   {{7, 22, 15, 20, 18, 11, 2, 9,    6, 23, 14, 21, 19, 10, 3, 8},           0,0, 0xFCCFCC,         8, 1,  s_qtag,s4x6},

   {{11, 10, 9, 8, 7, 6, 12, 13, 14, 15, 16, 17,
                      0, 1, 2, 3, 4, 5, 23, 22, 21, 20, 19, 18},             0,0,     0000,        12, 0,  s2x6,  s4x6},
   {{0, 2, 4, 6, 8, 10, 13, 15, 17, 19, 21, 23,
                      1, 3, 5, 7, 9, 11, 12, 14, 16, 18, 20, 22}, 04,022222222222, 0xFFFFFF,       12, 0,  s2x6,  s2x12},

   // This is for everyone as couples in a 3x4, making virtual columns of 6.
   {{2, 5, 7, 9, 10, 0,              3, 4, 6, 8, 11, 1},                     0,0,   0x0FFF,         6, 1,  s2x3,  s3x4},
   // This is for various people as couples in a 1/4 tag, making virtual columns of 6.
   {{1, 3, 4, 5, 6, 0,               -1, 2, -1, -1, 7, -1},                  0,0,     0xCC,         6, 1,  s2x3,  s_qtag},

   // There is an issue involving the order of the two pairs of items that follow.
   // In the order shown, (3x4 matrix stuff before c1phan), the program will opt
   // for a 3x4 if we say (normal columns; centers trail off)
   // centers are as couples, circulate.  In the other order, it would opt
   // for C1 phantoms.  We believe that having them arranged in 3 definite lines,
   // from which, for example, we could have the very center 2 trade, is better.

   // But we are nevertheless going to try it the other way, by swapping the
   // following two pairs with each other.

   // Next two are for various people as couples in a C1 phantom, making virtual columns of 6.
   {{3, 7, 5, 9, 15, 13,             1, -1, -1, 11, -1, -1},               0,0,     0000,         6, 0,  s2x3,  s_c1phan},
   {{0, 2, 6, 8, 10, 12,             -1, -1, 4, -1, -1, 14},               0,0,     0000,         6, 0,  s2x3,  s_c1phan},

   // Next two are for various people as couples in a 3x4 matrix, making virtual columns of 6.
   {{2, 5, 7, 8, 11, 0,              -1, -1, 6, -1, -1, 1},                0,0,   0x00C3,         6, 1,  s2x3,  s3x4},
   {{2, 5, 7, 9, 11, 1,              3, -1, -1, 8, -1, -1},                0,0,   0x030C,         6, 1,  s2x3,  s3x4},

   {{0, 2, 4, 6, 9, 11, 13, 15,      1, 3, 5, 7, 8, 10, 12, 14},   0,022222222,   0xFFFF,         8, 0,  s2x4,  s2x8},
   {{0, 2, 4, 6, 9, 11, 13, 15, 17, 19, 20, 22,
                      1, 3, 5, 7, 8, 10, 12, 14, 16, 18, 21, 23},  04,022222222222, 0xFFFFFF,    12, 0,  s3x4,  s3x8},
   {{2, 3, 5, 6, 7, 0,               -1, -1, 4, -1, -1, 1},                0,0,     0x33,         6, 1,  s_2x1dmd, s_crosswave},
   {{0, 1, 3, 4, 5, 6,               -1, -1, 2, -1, -1, 7},                0,0,        0,         6, 0,  s_1x2dmd, s_crosswave},
   {{6, 7, 0, 2, 3, 5,               -1, -1, 1, -1, -1, 4},          0,0200200,     0x33,         6, 0,  s_1x2dmd, s_rigger},
   {{0, 1, 3, 4, 6, 7,               -1, 2, -1, -1, 5, -1},          0,0020020,     0x66,         6, 0,  s_1x2dmd, s1x3dmd},
   {{0, 3, 2, 5, 7, 6,               1, -1, -1, 4, -1, -1},                0,0,     0x33,         6, 1,  s_2x1dmd, s_hrglass},
   {{0, 2, 3, 5, 6, 7,               1, -1, -1, 4, -1, -1},          0,0002002,     0x33,         6, 0,  s_2x1dmd, s3x1dmd},
   {{0, 1, 3, 4, 6, 7,               -1, 2, -1, -1, 5, -1},          0,0020020,     0x66,         6, 0,  s_2x1dmd, s3x1dmd},
   {{6, 7, 0, 2, 3, 5,               -1, -1, 1, -1, -1, 4},          0,0200200,     0x33,         6, 0,  s_2x1dmd, s_qtag},
   {{2, 4, 5, 0,                     -1, 3, -1, 1},                        0,0,      033,         4, 1,  sdmd, s_2x1dmd},
   {{0, 2, 4, 5,                     1, -1, 3, -1},                    0,00202,      033,         4, 0,  sdmd, s_1x2dmd},
   // Next one is for centers in tandem in lines, making a virtual bone6.
   {{0, 3, 5, 4, 7, 6,               -1, -1, 2, -1, -1, 1},                0,0,     0000,         6, 0,  s_bone6, s2x4},
   // Missing left half.
   {{-2, 3, 5, 4, -2, -2,            -2, -1, 2, -1, -2, -2},               0,0,     0000,         6, 0,  s_bone6, s2x4},
   // Missing right half.
   {{0, -2, -2, -2, 7, 6,            -1, -2, -2, -2, -1, 1},               0,0,     0000,         6, 0,  s_bone6, s2x4},

   /* Next two are for certain ends in tandem in an H, making a virtual bone6. */
   {{10, 3, 5, 6, 9, 11,             0, -1, -1, 4, -1, -1},                0,0,     0000,         6, 0,  s_bone6, s3x4},
   {{0, 4, 5, 6, 9, 11,              -1, 3, -1, -1, 10, -1},               0,0,     0000,         6, 0,  s_bone6, s3x4},
   /* Next one is for ends in tandem in lines, making a virtual short6. */
   {{2, 4, 5, 6, 7, 1,               -1, 3, -1, -1, 0, -1},          0,0020020,     0000,         6, 1,  s_short6, s2x4},
   /* Next two are for certain center column people in tandem in a 1/4 tag, making a virtual short6. */
   {{3, 2, 4, 5, 6, 0,               1, -1, -1, 7, -1, -1},          0,0002002,     0000,         6, 1,  s_short6, s_qtag},
   {{1, 2, 4, 5, 6, 7,               -1, -1, 3, -1, -1, 0},          0,0200200,     0000,         6, 1,  s_short6, s_qtag},
   /* Next two are for certain center column people in tandem in a spindle, making a virtual short6. */
   {{2, 3, 5, 6, 7, 0,               -1, -1, 4, -1, -1, 1},                0,0,     0x33,      6, 1,  s_short6, s_spindle},
   {{1, 3, 4, 6, 7, 0,               2, -1, -1, 5, -1, -1},                0,0,     0x66,      6, 1,  s_short6, s_spindle},
   /* Next three are for various people in tandem in columns of 8, making virtual columns of 6. */
   {{0, 2, 3, 5, 6, 7,               1, -1, -1, 4, -1, -1},          0,0002002,     0063,         6, 0,  s2x3,  s2x4},
   {{0, 1, 3, 4, 6, 7,               -1, 2, -1, -1, 5, -1},          0,0020020,     0x66,         6, 0,  s2x3,  s2x4},
   {{0, 1, 2, 4, 5, 7,               -1, -1, 3, -1, -1, 6},          0,0200200,     0xCC,         6, 0,  s2x3,  s2x4},
   /* Next three are for various people in tandem in a rigger/ptpd/bone, making a virtual line of 6. */
   {{6, 7, 5, 2, 3, 4,               -1, -1, 0, -1, -1, 1},                0,0,     0000,         6, 0,  s1x6,  s_rigger},
   {{0, 3, 2, 4, 5, 6,               -1, 1, -1, -1, 7, -1},                0,0,     0000,         6, 0,  s1x6,  s_ptpd},
   {{5, 6, 7, 4, 2, 3,               0, -1, -1, 1, -1, -1},                0,0,     0000,         6, 0,  s1x6,  s_bone},
   {{0, 1, 3, 4, 5, 6,               -1, -1, 2, -1, -1, 7},          0,0200200,     0xCC,         6, 0,  s_bone6,s_bone},
   {{0, 2, 5, 7, 9, 11, 12, 14,      1, 3, 4, 6, 8, 10, 13, 15},   0,022222222,   0xFFFF,         8, 0,  s_qtag,s4dmd},
   {{0, 3, 5, 6,                     1, 2, 4, 7},                          0,0,     0xFF,         4, 1,  sdmd,  s_qtag},
   {{0, 7, 2, 4, 5, 6,               -1, 1, -1, -1, 3, -1},                0,0,     0000,         6, 0,  s_2x1dmd, s_galaxy},
   {{2, 1, 4, 6, 7, 0,               -1, 3, -1, -1, 5, -1},                0,0,     0xAA,         6, 1,  s_2x1dmd, s_galaxy},
   {{3, 7, 9, 13,                    1, 5, 11, 15},                    0,02020,   0xA0A0,         4, 0,  s2x2,  s_c1phan},
   {{0, 6, 10, 12,                   2, 4, 8, 14},                     0,00202,   0x0505,         4, 0,  s2x2,  s_c1phan},

   /* these 4 are unsymmetrical */
   {{0, 7, 9, 12,                    2, 5, 11, 14},                    0,00022,   0x00A5,         4, 0,  s2x2,  s_c1phan},
   {{3, 6, 10, 13,                   1, 4, 8, 15},                     0,02200,   0xA500,         4, 0,  s2x2,  s_c1phan},
   {{3, 7, 10, 12,                   1, 5, 8, 14},                     0,00220,   0x05A0,         4, 0,  s2x2,  s_c1phan},
   {{0, 6, 9, 13,                    2, 4, 11, 15},                    0,02002,   0xA005,         4, 0,  s2x2,  s_c1phan},

   // These do C1-phantom-like stuff from fudgy 4x4.
   // They must follow the pair just above.
   {{15, 3, 5, 9,                    13, 1, 7, 11},                    0,02020,   0x0A0A,         4, 0,  s2x2,  s4x4},
   {{10, 3, 7, 6,                    15, 14, 2, 11},                   0,00202,   0x8484,         4, 0,  s2x2,  s4x4},

   // These two do C1-phantom-like stuff from fudgy 3x4.
   {{11, 2, 7, 9,                    1, 3, 5, 8},                      0,02020,    01414,         4, 0,  s2x2,  s3x4},
   {{0, 5, 7, 8,                     1, 2, 6, 11},                     0,00202,    00303,         4, 0,  s2x2,  s3x4},

   {{1, 3, 4, 5, 6, 0,               -1, 2, -1, -1, 7, -1},                0,0,     0xCC,      6, 1,  s_short6, s_rigger},
   {{6, 0, 3, 5,                     7, 1, 2, 4},                      0,02222,     0xFF,         4, 0,  sdmd,  s_rigger},
   /* must be after "2x4_4" */
   {{6, 5, 3, 4,                     7, 0, 2, 1},                      0,00202,     0xCC,         4, 0,  s1x4,  s_rigger},
   {{5, 6, 4, 3,                     0, 7, 1, 2},                      0,02020,     0xCC,         4, 0,  s1x4,  s_bone},
   {{0, 3, 5, 6,                     1, 2, 4, 7},                      0,00202,     0x33,      4, 0,  sdmd,  s_crosswave},
   {{0, 3, 5, 6,                     1, 2, 4, 7},                      0,00202,     0x33,         4, 0,  s_star, s_thar},

   {{0, 3, 5, 6, 9, 10, 12, 15,      1, 2, 4, 7, 8, 11, 13, 14},   0,002020202,   0x3333,         8, 0,  s_ptpd,  sdblxwave},

   {{15, 1, 12, 14, 8, 10, 11, 5,    0, 2, 3, 13, 7, 9, 4, 6},     0,020202020,   0x6666,         8, 0,  s_ptpd,  s2x8},

   {{6, 0, 3, 5,                     7, 1, 2, 4},                      0,02020,     0x33,         4, 0,  s_star, s_alamo},

   // Must be after "2x4_4".
   {{7, 1, 4, 6,                     0, 2, 3, 5},                      0,02020,     0146,         4, 0,  sdmd,  s2x4},
   /* Next one is for so-and-so in tandem in a bone6, making a virtual line of 4. */
   {{4, 5, 3, 2,                     0, -1, 1, -1},                        0,0,     0000,         4, 0,  s1x4,  s_bone6},
   /* Next one is for so-and-so in tandem in a short6, making a virtual line of 4. */
   {{1, 0, 4, 5,                     -1, 2, -1, 3},                        0,0,     0055,         4, 1,  s1x4,  s_short6},
   {{1, 3, 4, 5,                     -1, 2, -1, 0},                    0,02020,     0000,         4, 1,  sdmd,  s_short6},
   {{5, 1, 3, 4,                     0, -1, 2, -1},                        0,0,     0000,         4, 0,  sdmd,  s2x3},

   /* Next three are for so-and-so as couples in a line of 8, making a virtual line of 6. */
   {{0, 1, 3, 4, 5, 6,               -1, -1, 2, -1, -1, 7},          0,0200200,     0xCC,         6, 0,  s1x6,  s1x8},
   {{0, 1, 2, 4, 7, 6,               -1, 3, -1, -1, 5, -1},          0,0020020,     0xAA,         6, 0,  s1x6,  s1x8},
   {{0, 3, 2, 5, 7, 6,               1, -1, -1, 4, -1, -1},          0,0002002,     0x33,         6, 0,  s1x6,  s1x8},
   /* Next two are for so-and-so as couples in a line of 6, making a virtual line of 4. */
   {{0, 1, 3, 5,                     -1, 2, -1, 4},                    0,02020,     0066,         4, 0,  s1x4,  s1x6},
   {{0, 2, 4, 5,                     1, -1, 3, -1},                    0,00202,     0033,         4, 0,  s1x4,  s1x6},

   // Next 4 are for so-and-so in tandem from a column of 6, making a virtual column of
   // 4.  The first two are the real maps, and the other two take care of the
   // reorientation that sometimes happens when coming out of a 2x2.

   {{0, 1, 3, 5,                     -1, 2, -1, 4},                    0,02020,     0066,         4, 0,  s2x2,  s2x3},
   {{0, 2, 4, 5,                     1, -1, 3, -1},                    0,00202,     0033,         4, 0,  s2x2,  s2x3},
   {{1, 3, 5, 0,                     2, -1, 4, -1},                        0,0,     0066,         4, 1,  s2x2,  s2x3},
   {{2, 4, 5, 0,                     -1, 3, -1, 1},                        0,0,     0033,         4, 1,  s2x2,  s2x3},
   // Next 2 are for similar situations, in "nonisotropic triangles".
   // We do not have the 3rd or 4th maps in the class, because they apply only
   // to unwinding, and we never unwind to these setups.  That's why
   // these 2 maps are placed after the 4 preceding ones.
   {{0, 1, 3, 5,                     -1, 2, -1, 4},                    0,02020,     0066,      4, 0,  s2x2,  s_ntrgl6cw},
   {{0, 2, 4, 5,                     1, -1, 3, -1},                    0,00202,     0033,      4, 0,  s2x2,  s_ntrgl6ccw},

   // Next 2 are for similar situations, with all 8 people present.
   {{1, 3, 4, 6, 7, 0,               2, -1, -1, 5, -1, -1},             0,0,     0x66,         6, 1,  s_short6,  s_nxtrglcw},
   {{2, 3, 5, 6, 7, 0,              -1, -1, 4, -1, -1, 1},              0,0,     0x33,         6, 1,  s_short6,  s_nxtrglccw},

   // Need versions of these with phantoms on each half.
   {{0, 1, 3, 4, 5, 6,               -1, -1, 2, -1, -1, 7},         0,0200200,     0xCC,       6, 0,  s_bone6,  s_bone},
   // Missing left half.
   {{-2, 1, 3, 4, -2, -2,            -2, -1, 2, -1, -2, -2},        0,0000200,     0x0C,       6, 0,  s_bone6,  s_bone},
   // Missing right half.
   {{0, -2, -2, -2, 5, 6,            -1, -2, -2, -2, -1, 7},        0,0200000,     0xC0,       6, 0,  s_bone6,  s_bone},

   {{0, 2, 3, 4, 7, 8, 9, 11,        1, -1, -1, 5, 6, -1, -1, 10},
    0,020022002,     06363,      8, 0,  s2x4, s2x6},

   {{0, 2, 4, 7, 9, 11,              1, 3, 5, 6, 8, 10},
    0,0222222,      0x0FFF,     6, 0,  s2x3,  s2x6},
   // The three maps just below must be after the map just above.
   {{-2, 7, 6, -2, 12, 15,           -2, 2, 5, -2, 17, 16},
    0,0200200,      0x18060,    6, 0,  s2x3,  s4x5},
   {{9, 7, -2, 18, 12, -2,           8, 2, -2, 19, 17, -2},
    0,0002002,      0xC0300,    6, 0,  s2x3,  s4x5},
   {{3, 6, 8, 10, 11, 1,             4, 5, 7, 9, 0, 2},
    0,0020020,      03636,      6, 1,  s_short6,  s2x6},

   {{10, 7, 8, 5, 0, 3,              11, 6, 9, 4, 1, 2},
    0,0202202,      00303,      6, 1,  s_short6,  sdeepxwv},

   {{-2, 3, 4, -2, 8, 11,           -2, 2, 5, -2, 9, 10},
    0,0200200,      0xC30,      6, 0,  s2x3,  sbigdmd},
   {{0, 3, -2, 7, 8, -2,           1, 2, -2, 6, 9, -2},
    0,0002002,      0x0C3,      6, 0,  s2x3,  sbigdmd},

   {{2, 0,                           3, 1},                         0,020,        0xC,        2, 1,  s1x2,  s_trngl4},
   {{1, 3,                           0, 2},                         0,02,         0xC,        2, 3,  s1x2,  s_trngl4},
   {{2, 1, 0,                        3, -1, -1},                    0,0,          0xC,        3, 1,  s1x3,  s_trngl4},
   {{0, 1, 3,                        -1, -1, 2},                    0,0,          0xC,        3, 3,  s1x3,  s_trngl4},
   {{0, 3, 2,                        -1, 1, -1},                    0,0,          0,          3, 0,  s1x3,  sdmd},

   {{1, 3, 4, 7, 9, 11,              -1, -1, 5, -1, -1, 10},
    0,0200200,      0xC30,      6, 0,  s_ntrgl6cw,  s2x6},
   {{0, 2, 4, 7, 8, 10,              1, -1, -1, 6, -1, -1},
    0,0002002,      0x0C3,      6, 0,  s_ntrgl6ccw, s2x6},
   {{1, 3, 5, 7, 10, 11,              -1, 4, -1, -1, 9, -1},
    0,0020020,      0x618,      6, 0,  s_ntrgl6cw,  s2x6},
   {{0, 1, 4, 6, 8, 10,              -1, 2, -1, -1, 7, -1},
    0,0020020,      0x186,      6, 0,  s_ntrgl6ccw, s2x6},

   // This map must be very late, after the two that do 2x4->4x4
   // and the one that does 2x4->2x8.
   {{3, 5, 14, 8, 9, 12, 7, 2,      1, 4, 15, 10, 11, 13, 6, 0},
    0,020022002,     0xF0F0,     8, 1,  s2x4,  sdeepbigqtg},

   {{9, 8, 23, 22, 14, 15, 18, 19,       2, 3, 6, 7, 21, 20, 11, 10},
    0,0,          0,          8, 0,  s_rigger, s4x6},

   // These must be at the end.  They can partially restore setups
   // through intermediate setups that are too big.

   {{15, 14, 13, 12, 11, 10, 9, 8, 16, 17, 18, 19, 20, 21, 22, 23,
     0, 1, 2, 3, 4, 5, 6, 7, 31, 30, 29, 28, 27, 26, 25, 24},
    0,0,          0,          16, 0,  s2x8,  shyper4x8a},
   {{0, 2, 4, 6, 8, 10, 12, 14, 17, 19, 21, 23, 25, 27, 29, 31,
     1, 3, 5, 7, 9, 11, 13, 15, 16, 18, 20, 22, 24, 26, 28, 30},
    044444,022222222222, 0xFFFFFFFF, 16, 0,  s2x8,  shyper2x16},
   {{6, 9, 30, 11, 17, 19, 21, 28, 23, 24, 15, 26, 0, 2, 4, 13,
     7, 8, 31, 10, 16, 18, 20, 29, 22, 25, 14, 27, 1, 3, 5, 12},
    044444,022222222222, 0xFFFFFFFF, 16, 0,  s4x4,  shyper4x8a},
   {{17, 19, 21, 28, 23, 24, 15, 26, 0, 2, 4, 13, 6, 9, 30, 11,
     16, 18, 20, 29, 22, 25, 14, 27, 1, 3, 5, 12, 7, 8, 31, 10},
    0,0,          0xFFFFFFFF, 16, 1,  s4x4,  shyper4x8a},

   {{0}, 0,0, 0, 0, 0, nothing, nothing}};


static tm_thing maps_isearch_threesome[] = {
   //       maps                                                      ilatmaskH/L    olatmask   limit rot insetup outsetup

   {{0,                    1,                    2},                        0,02,        07,      1, 0,  s1x1,  s1x3},
   {{0,                    1,                    2},                         0,0,        07,      1, 1,  s1x1,  s1x3},
   {{0, 5,                 1, 4,                 2, 3},                    0,022,       077,      2, 0,  s1x2,  s1x6},
   {{0, 5,                 1, 4,                 2, 3},                      0,0,       077,      2, 1,  s1x2,  s2x3},
   {{0, 3, 8, 11,     1, 4, 7, 10,     2, 5, 6, 9},                      0,02222,     07777,      4, 0,  s2x2,  s2x6},
   {{3, 8, 11, 0,     4, 7, 10, 1,     5, 6, 9, 2},                          0,0,     07777,      4, 1,  s2x2,  s2x6},

   {{3, 8, 11, 14, 15, 0,   4, 7, 10, 13, 16, 1,   5, 6, 9, 12, 17, 2},      0,0,   0777777,      6, 1,  s2x3,  s3x6},

   // Ones with missing people:
   {{-2, 8, 10, -2, 15, 1,   -2, 7, -1, -2, 16, -1,   -2, 6, -1, -2, 17, -1}, 0,0,  0700700,      6, 1,  s2x3,  s3x6},

   {{4, 8, -2, 13, 15, -2,   -1, 7, -2, -1, 16, -2,   -1, 6, -2, -1, 17, -2}, 0,0,  0700700,      6, 1,  s2x3,  s3x6},

   {{0, 3, 6, 9, 14, 17, 20, 23,          1, 4, 7, 10, 13, 16, 19, 22,
                      2, 5, 8, 11, 12, 15, 18, 21},                  0,022222222, 077777777,      8, 0,  s2x4,  s2x12},
   {{0, 3, 6, 7,           1, -1, 5, -1,         2, -1, 4, -1},          0,00202,      0x77,      4, 0,  sdmd,  s1x3dmd},
   {{3, 6, 7, 0,           -1, 5, -1, 1,         -1, 4, -1, 2},              0,0,      0x77,      4, 1,  sdmd,  s3x1dmd},
   {{7, 0, 3, 6,           -1, 1, -1, 5,         -1, 2, -1, 4},          0,02020,      0x77,      4, 0,  sdmd,  s_spindle},
   {{0, 3, 6, 7,           1, -1, 5, -1,         2, -1, 4, -1},              0,0,      0x77,      4, 1,  sdmd,  s_323},
   {{0, 5, 8, 11,          1, -1, 7, -1,         2, -1, 6, -1},              0,0,      0707,      4, 1,  sdmd,  s3dmd},
   {{0, 5, 8, 9,           1, 4, 7, 10,          2, 3, 6, 11},               0,0,     07777,      4, 1,  sdmd,  s3dmd},
   {{0, 3, 8, 11,          1, 4, 7, 10,          2, 5, 6, 9},            0,02222,     07777,      4, 0,  s1x4,  s1x12},
   {{0, 2, 7, 6,           1, -1, 5, -1,         3, -1, 4, -1},          0,00202,      0xBB,      4, 0,  s1x4,  s1x8},
   {{0, 1, 4, 6,           -1, 3, -1, 7,         -1, 2, -1, 5},          0,02020,      0xEE,      4, 0,  s1x4,  s1x8},
   {{3, 8, 21, 14, 17, 18, 11, 0,         4, 7, 22, 13, 16, 19, 10, 1,
                                          5, 6, 23, 12, 15, 20, 9, 2},       0,0, 0x0FFFFFF,      8, 1,  s2x4,  s4x6},
   {{19, 18, 16, 17, 12, 13, 15, 14,      20, 21, 23, 22, 8, 9, 11, 10,
                                          0, 1, 3, 2, 7, 6, 4, 5},           0,0,      0000,      8, 0,  s1x8,  s3x8},
   {{9, 8, 6, 7,           10, 11, 4, 5,         0, 1, 3, 2},                0,0,      0000,      4, 0,  s1x4,  s3x4},
   {{9, 11, 6, 5,          10, -1, 4, -1,        0, -1, 3, -1},              0,0,      0000,      4, 0,  s1x4,  s3x4},
   {{10, 8, 4, 7,          -1, 11, -1, 5,        -1, 1, -1, 2},              0,0,      0000,      4, 0,  s1x4,  s3x4},
   {{0, 3, 6, 7,           1, -1, 5, -1,         2, -1, 4, -1},          0,00202,      0x77,      4, 0,  s2x2,  s2x4},
   {{0, 1, 4, 7,           -1, 2, -1, 6,         -1, 3, -1, 5},          0,02020,      0xEE,      4, 0,  s2x2,  s2x4},
   {{3, 6, 7, 0,           -1, 5, -1, 1,         -1, 4, -1, 2},              0,0,      0x77,      4, 1,  s2x2,  s2x4},
   {{1, 4, 7, 0,           2, -1, 6, -1,         3, -1, 5, -1},              0,0,      0xEE,      4, 1,  s2x2,  s2x4},
   {{6, 5, 2, 4,           -1, 7, -1, 3,         -1, 0, -1, 1},              0,0,      0000,      4, 0,  s1x4,  s_qtag},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};

static tm_thing maps_isearch_foursome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{0, 6,             1, 7,             3, 5,             2, 4},              0,022,    0x0FF,         2, 0,  s1x2,  s1x8},
   {{0, 7,             1, 6,             2, 5,             3, 4},                0,0,    0x0FF,         2, 1,  s1x2,  s2x4},
   {{0, 4, 11, 15,     1, 5, 10, 14,     2, 6, 9, 13,      3, 7, 8, 12},      0,02222,  0x0FFFF,         4, 0,  s2x2,  s2x8},
   {{4, 11, 15, 0,     5, 10, 14, 1,     6, 9, 13, 2,      7, 8, 12, 3},         0,0,  0x0FFFF,         4, 1,  s2x2,  s2x8},
   {{0, 7, 11, 12,     1, 6, 10, 13,     2, 5, 9, 14,      3, 4, 8, 15},         0,0,  0x0FFFF,         4, 1,  sdmd,  s4dmd},
   {{0, 4, 11, 15,     1, 5, 10, 14,     2, 6, 9, 13,      3, 7, 8, 12},      0,02222,  0x0FFFF,         4, 0,  s1x4,  s1x16},
   {{17, 16, 15, 12, 13, 14,         18, 19, 20, 23, 22, 21,
         11, 10, 9, 6, 7, 8,                 0, 1, 2, 5, 4, 3},                  0,0,     0000,         6, 0,  s1x6,  s4x6},
   {{8, 6, 4, 5,       9, 11, 2, 7,      10, 15, 1, 3,     12, 13, 0, 14},       0,0,     0000,         4, 0,  s1x4,  s4x4},
   {{12, 10, 8, 9,     13, 15, 6, 11,    14, 3, 5, 7,      0, 1, 4, 2},          0,0,   0xFFFF,         4, 1,  s1x4,  s4x4},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};

static tm_thing maps_isearch_sixsome[] = {
/*   maps
           ilatmask olatmask    limit rot            insetup outsetup */
   {{0, 11,   1, 10,   2, 9,    3, 8,    4, 7,    5, 6},
             0,022,    0x0FFF,       2, 0,  s1x2,  s1x12},
   {{0, 11,   1, 10,   2, 9,    3, 8,    4, 7,    5, 6},
               0,0,    0x0FFF,       2, 1,  s1x2,  s2x6},
   {{0, 6, 17, 23,   1, 7, 16, 22,   2, 8, 15, 21,    3, 9, 14, 20,    4, 10, 13, 19,    5, 11, 12, 18},
            0,02222, 0x0FFFFFF,       4, 0,  s2x2,  s2x12},
   {{0, 11, 17, 18,  1, 10, 16, 19,  2, 9, 15, 20,    3, 8, 14, 21,    4, 7, 13, 22,     5, 6, 12, 23},
               0,0, 0x0FFFFFF,       4, 1,  s1x4,  s4x6},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};

static tm_thing maps_isearch_eightsome[] = {
/*   maps
           ilatmask olatmask    limit rot            insetup outsetup */
   {{0, 15,   1, 14,   2, 13,   3, 12,   4, 11,   5, 10,   6, 9,   7, 8},
             0,022,   0x0FFFF,       2, 0,  s1x2,  s1x16},
   {{0, 15,   1, 14,   2, 13,   3, 12,   4, 11,   5, 10,   6, 9,   7, 8},
               0,0,   0x0FFFF,       2, 1,  s1x2,  s2x8},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};

static tm_thing maps_isearch_boxsome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{0,                1,                3,                2},                  0,02,      0xF,       1, 0,  s1x1,  s2x2},
   {{3,                0,                2,                1},                   0,0,        0,       1, 0,  s1x1,  s2x2},
   {{0, 2,             1, 3,             7, 5,             6, 4},              0,022,     0xFF,       2, 0,  s1x2,  s2x4},
   {{7, 5,             0, 2,             6, 4,             1, 3},                0,0,        0,       2, 0,  s1x2,  s2x4},
   {{0, 2, 6, 4,       1, 3, 7, 5,       15, 13, 9, 11,    14, 12, 8, 10},    0,02222,  0xFFFF,       4, 0,  s1x4,  s2x8},
   {{15, 13, 9, 11,    0, 2, 6, 4,       14, 12, 8, 10,    1, 3, 7, 5},          0,0,        0,       4, 0,  s1x4,  s2x8},
   {{12, 14, 7, 9,     13, 0, 2, 11,     10, 3, 5, 8,      15, 1, 4, 6},      0,02222,  0xFFFF,       4, 0,  s2x2,  s4x4},
   {{10, 3, 5, 8,      12, 14, 7, 9,     15, 1, 4, 6,      13, 0, 2, 11},        0,0,        0,       4, 0,  s2x2,  s4x4},


   // Special maps for couples 1/8 twosome stuff.
   // These need further work in the table initializer.
   {{3, 4, 28, 27,        13, 9, 30, 26,   14, 10, 29, 25,   12, 11, 19, 20},      0,03333,    0xFF,  4, 0,  s2x2,  shyper4x8b},
   {{14, 10, 29, 25,      3, 4, 28, 27,    12, 11, 19, 20,   13, 9, 30, 26},       0,01111,    0xFF,  4, 0,  s2x2,  shyper4x8b},
   {{14, 4, 29, 27,       3, 9, 28, 26,    12, 10, 19, 25,   13, 11, 30, 20},      0,03131,    0xFF,  4, 0,  s2x2,  shyper4x8b},
   {{3, 10, 28, 25,       13, 4, 30, 27,   14, 11, 29, 20,   12, 9, 19, 26},       0,01313,    0xFF,  4, 0,  s2x2,  shyper4x8b},
   {{20, 22, 9, 11,       2, 3, 5, 4,      17, 16, 14, 15,   21, 23, 8, 10},       0,01111,    0xFF,  4, 0,  s1x4,  shyper3x8},
   {{2, 3, 5, 4,          21, 23, 8, 10,   20, 22, 9, 11,    17, 16, 14, 15},      0,03333,    0xFF,  4, 0,  s1x4,  shyper3x8},
   {{20, 3, 9, 4,         2, 23, 5, 10,    17, 22, 14, 11,   21, 16, 8, 15},       0,03131,    0xFF,  4, 0,  s1x4,  shyper3x8},
   {{2, 22, 5, 11,        21, 3, 8, 4,     20, 16, 9, 15,    17, 23, 14, 10},      0,01313,    0xFF,  4, 0,  s1x4,  shyper3x8},
   {{6, 3,       0, 1,      5, 4,    7, 2},       0,011,       0xF,  2, 0,  s1x2,  slittlestars},
   {{0, 1,       7, 2,      6, 3,    5, 4},       0,033,       0xF,  2, 0,  s1x2,  slittlestars},
   {{6, 1,       0, 2,      5, 3,    7, 4},       0,031,       0xF,  2, 0,  s1x2,  slittlestars},
   {{0, 3,       7, 1,      6, 4,    5, 2},       0,013,       0xF,  2, 0,  s1x2,  slittlestars},


   {{11, 2, 7, 20,     10, 3, 6, 21,     18, 9, 22, 15,    19, 8, 23, 14},        0,02222,0x0FCCFCC,  4, 0,  sdmd,  s4x6},
   {{18, 9, 22, 15,    11, 2, 7, 20,     19, 8, 23, 14,    10, 3, 6, 21},              0,0,       0,  4, 0,  sdmd,  s4x6},
   {{0, 2, 4, 22, 20, 18,     1, 3, 5, 23, 21, 19,
     11, 9, 7, 13, 15, 17,    10, 8, 6, 12, 14, 16},                             0,0222222,0xFFFFFF,  6, 0,  s2x3,  s4x6},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};


static tm_thing maps_isearch_dmdsome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{0,                1,                3,                2},                   0,02,      0xF,         1, 0,  s1x1,  sdmd},
   {{0,                1,                3,                2},                   0,0,      0xF,         1, 1,  s1x1,  sdmd},
   {{0, 6,             1, 7,             3, 5,             2, 4},              0,022,     0xFF,         2, 0,  s1x2,  s_ptpd},
   {{5, 4,             6, 3,             7, 2,             0, 1},                0,0,        0,         2, 0,  s1x2,  s_qtag},
   {{11, 10, 8, 9,     12, 14, 5, 7,     13, 15, 4, 6,     0, 1, 3, 2},          0,0,        0,         4, 0,  s1x4,  s4dmd},
   {{12, 14, 5, 7,     0, 1, 3, 2,       11, 10, 8, 9,     13, 15, 4, 6},     0,02222,   0xFFFF,         4, 0,  s1x4,  s4ptpd},
   {{-2, 4, -2, 5,     -2, 3, -2, 6,     -2, 2, -2, 7,     -2, 1, -2, 0},     0,02222,        0,         4, 1,  sdmd,  s_qtag},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};


static tm_thing maps_isearch_tglsome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{6, 0, 2, 4,       -1, 7, -1, 3,     -1, 5, -1, 1},                       0,06020,     0xBB,         4, 0,  s1x4,  s_rigger},
   {{1, 2, 5, 6,       0, -1, 4, -1,     3, -1, 7, -1},                       0,00602,     0xBB,         4, 0,  s1x4,  s_ptpd},
   {{0, 3, 4, 7,       -1, 2, -1, 6,     -1, 1, -1, 5},                       0,02060,     0xEE,         4, 0,  s1x4,  s_ptpd},

   {{7, 1, 3, 5,       0, -1, 4, -1,     6, -1, 2, -1},                       0,00400,        0,         4, 0,  s2x2,  s_qtag},
   {{0, 2, 4, 6,       -1, 1, -1, 5,     -1, 3, -1, 7},                       0,04000,        0,         4, 0,  s2x2,  s_qtag},

   {{1, 3, 5, 7,       -1, 4, -1, 0,     -1, 2, -1, 6},                       0,02060,        0,         4, 1,  s2x2,  s_qtag},
   {{2, 4, 6, 0,       1, -1, 5, -1,     3, -1, 7, -1},                       0,00602,        0,         4, 1,  s2x2,  s_qtag},

   {{5, 7, 1, 3,       6, -1, 2, -1,     0, -1, 4, -1},                       0,00206,     0x77,         4, 0,  s1x4,  s_bone},
   {{0, 1, 4, 5,       7, -1, 3, -1,     6, -1, 2, -1},                       0,00602,     0xDD,         4, 0,  sdmd,  s_spindle},
   {{5, 3, 1, 7,       6, -1, 2, -1,     0, -1, 4, -1},                       0,00206,     0x77,         4, 0,  sdmd,  s_dhrglass},
   {{6, 0, 2, 4,       -1, 3, -1, 7,     -1, 1, -1, 5},                       0,00040,        0,         4, 0,  sdmd,  s_hrglass},
   {{0, 3, 4, 7,       6, -1, 2, -1,     5, -1, 1, -1},                       0,00602,     0x77,         4, 0,  sdmd,  s_hrglass},

   // These four need to show a "fudgy" warning.
   // Well, the last two are being changed to be not fudgy.
   {{6, 1, 2, 5,       0, -1, 4, -1,     7, -1, 3, -1},                       0,00400,        0,         4, 0,  sdmd,  s2x4},
   {{0, 2, 4, 6,       7, -1, 3, -1,     1, -1, 5, -1},                       0,00004,        0,         4, 0,  sdmd,  s2x4},
   /*  old stuff
   {{7, 0, 3, 4,       -1, 6, -1, 2,     -1, 1, -1, 5},                       0x08,        0,         4, 0,  s1x4,  s_spindle},
   {{7, 5, 3, 1,       -1, 0, -1, 4,     -1, 6, -1, 2},                       0x80,        0,         4, 0,  s1x4,  s_spindle},
   */
   // New stuff.
   {{7, 0, 3, 4,       -1, 6, -1, 2,     -1, 1, -1, 5},                       0,00040,        0,         4, 0,  s1x4,  s_nxtrglccw},
   {{7, 5, 3, 1,       -1, 0, -1, 4,     -1, 6, -1, 2},                       0,04000,        0,         4, 0,  s1x4,  s_nxtrglcw},



   {{0, 3, 4, 7,       -1, 2, -1, 6,     -1, 1, -1, 5},                       0,04000,        0,         4, 0,  sdmd,  s_galaxy},
   {{2, 5, 6, 1,       -1, 4, -1, 0,     -1, 3, -1, 7},                       0,00040,     0xBB,         4, 1,  sdmd,  s_galaxy},

   {{1, 7, 9, 15,      -1, 11, -1, 3,    -1, 5, -1, 13},                      0,00040,   0x0000,         4, 0,  s2x2,  s_c1phan},
   {{0, 4, 8, 12,      -1, 2, -1, 10,    -1, 6, -1, 14},                      0,06020,   0x5454,         4, 0,  s2x2,  s_c1phan},
   {{3, 5, 11, 13,     7, -1, 15, -1,    1, -1, 9, -1},                       0,00206,   0x8A8A,         4, 0,  s2x2,  s_c1phan},
   {{0, 4, 8, 12,      14, -1, 6, -1,    2, -1, 10, -1},                      0,00004,   0x0000,         4, 0,  s2x2,  s_c1phan},
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};


static tm_thing maps_isearch_3x1tglsome[] = {
/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{9, 3,             10, 4,            0, 6,             11, 5},             0,026,    07171,         2, 0,  s1x2,  s3x4},
   {{0, 4,             7, 3,             5, 1,             6, 2},              0,062,     0xFF,         2, 0,  s1x2,  s_qtag},
   {{9, 3,             10, 4,            11, 5,            1, 7},              0,040,     0000,         2, 0,  s1x2,  s2x6},
   {{0, 6,             1, 7,             2, 8,             10, 4},             0,004,     0000,         2, 0,  s1x2,  s2x6},

   {{9, 6, 21, 18,     10, 7, 22, 19,    11, 8, 23, 20,    1, 4, 13, 16},      0,00044,    0000,         4, 0,  s2x2,  s4x6},
   {{0, 3, 12, 15,     1, 4, 13, 16,     2, 5, 14, 17,     10, 7, 22, 19},     0,04400,    0000,         4, 0,  s2x2,  s4x6},

   {{19, 3, 7, 15,     20, 23, 8, 11,    0, 16, 12, 4,     21, 22, 9, 10},     0,02662,   0xF99F99,      4, 0,  s1x4,  s3x8},
   {{1, 17, 13, 5,     21, 22, 9, 10,    18, 2, 6, 14,     20, 23, 8, 11},     0,06226,   0xF66F66,      4, 0,  s1x4,  s3x8},

   {{0, 18, 12, 6,     1, 19, 13, 7,     2, 20, 14, 8,     22, 4, 10, 16},     0,00440,   0000,          4, 0,  s1x4,  s2x12},
   {{21, 3, 9, 15,     22, 4, 10, 16,    23, 5, 11, 17,    1, 19, 13, 7},      0,04004,   0000,          4, 0,  s1x4,  s2x12},

   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};

static tm_thing maps_isearch_ysome[] = {
/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{5, 1,             0, 4,             6, 2,             7, 3},              0,026,     0xFF,         2, 0,  s1x2,  s_bone},
   {{0, 4,             5, 1,             7, 3,             6, 2},              0,062,     0xFF,         2, 0,  s1x2,  s_rigger},
   {{15, 7,            13, 5,            3, 11,            1, 9},              0,040,     0000,         2, 0,  s1x2,  s_c1phan},
   {{0, 8,             2, 10,            14, 6,            12, 4},             0,004,     0000,         2, 0,  s1x2,  s_c1phan},
/*   Taken out -- we rotate the setup instead.
   {{3, 11,            1, 9,             7, 15,            5, 13},             0x8,   0xAAAA,         2, 1,  s1x2,  s_c1phan},
   {{4, 12,            6, 14,            2, 10,            0, 8},              0x2,   0x5555,         2, 1,  s1x2,  s_c1phan},
*/
   {{0}, 0,0, 0, 0, 0,  nothing, nothing}};

struct siamese_item {
   setup_kind testkind;
   uint32 testval;
   uint32 fixup;   // High bit means phantom pairing is OK.
   warning_index warning;
};

siamese_item siamese_table_of_2[] = {
   {s2x4,        0x00FF0000UL, 0x99UL,   warn__ctrstand_endscpls},
   {s2x4,        0x00990066UL, 0x99UL,   warn__ctrstand_endscpls},
   {s2x4,        0x000000FFUL, 0x66UL,   warn__ctrscpls_endstand},
   {s2x4,        0x00660099UL, 0x66UL,   warn__ctrscpls_endstand},
   {s2x4,        0x00F0000FUL, 0x0FUL,   warn__none},  // unsymm
   {s2x4,        0x000F00F0UL, 0xF0UL,   warn__none},  // unsymm
   {s1x8,        0x00CC0033UL, 0x33UL,   warn__ctrstand_endscpls},
   {s1x8,        0x003300CCUL, 0xCCUL,   warn__ctrscpls_endstand},
   {s2x4,        0x003300CCUL, 0xCCUL,   warn__none},
   {s2x4,        0x00CC0033UL, 0x33UL,   warn__none},
   {s_trngl4,    0x000F0000UL, 0x03UL,   warn__none},
   {s_trngl4,    0x0000000FUL, 0x0CUL,   warn__none},
   {s_c1phan,    0x0000AAAAUL, 0xA0A0UL, warn__none},
   {s_c1phan,    0x00005555UL, 0x0505UL, warn__none},
   {s_c1phan,    0xAAAA0000UL, 0x0A0AUL, warn__none},
   {s_c1phan,    0x55550000UL, 0x5050UL, warn__none},
   {s_c1phan,    0x00005AA5UL, 0x00A5UL, warn__none},  /* These 8 are unsymmetrical. */
   {s_c1phan,    0x0000A55AUL, 0xA500UL, warn__none},
   {s_c1phan,    0x000055AAUL, 0x05A0UL, warn__none},
   {s_c1phan,    0x0000AA55UL, 0xA005UL, warn__none},
   {s_c1phan,    0x5AA50000UL, 0x5A00UL, warn__none},
   {s_c1phan,    0xA55A0000UL, 0x005AUL, warn__none},
   {s_c1phan,    0x55AA0000UL, 0x500AUL, warn__none},
   {s_c1phan,    0xAA550000UL, 0x0A50UL, warn__none},

   {s4x4,        0x0000AAAAUL, 0x0A0AUL, warn__none},
   {s4x4,        0x0000CCCCUL, 0x8484UL, warn__none},
   {s4x4,        0xAAAA0000UL, 0xA0A0UL, warn__none},
   {s4x4,        0xCCCC0000UL, 0x4848UL, warn__none},

   {s4x5,        0x000090E4UL, 0x18060UL, warn__none},
   {s4x5,        0x00001384UL, 0xC0300UL, warn__none},
   {s4x5,        0x90E40000UL, 0x21084UL, warn__none},
   {s4x5,        0x13840000UL, 0x21084UL, warn__none},

   {sbigdmd,     0x00000F3CUL, 0x18060UL, warn__none},
   {sbigdmd,     0x000003CFUL, 0xC0300UL, warn__none},
   {sbigdmd,     0x0F3C0000UL, 0x0030CUL, warn__none},
   // ************************** THIS IS THE ONE ************************
   {sbigdmd,     0x03CF0000UL, 0x0030CUL, warn__none},
   // ************************** THIS IS THE ONE ************************

   {s4x4,        0x30304141UL, 0x80004141UL, warn__none},
   {s4x4,        0x41413030UL, 0x80003030UL, warn__none},
   {s4x4,        0x03031414UL, 0x80000303UL, warn__none},
   {s4x4,        0x14140303UL, 0x80001414UL, warn__none},

   {s3x4,        0x09E70000UL, 0x0924UL, warn__none},
   {s3x4,        0x000009E7UL, 0x00C3UL, warn__none},
   {s3x4,        0x0BAE0000UL, 0x08A2UL, warn__none},
   {s3x4,        0x00000BAEUL, 0x030CUL, warn__none},

   {s_qtag,      0x003300CCUL, 0xCCUL,   warn__ctrscpls_endstand},
   {s_qtag,      0x00CC0033UL, 0x33UL,   warn__ctrstand_endscpls},
   {s4dmd,       0x0F0FF0F0UL, 0xF0F0UL, warn__ctrscpls_endstand},
   {s4dmd,       0xF0F00F0FUL, 0x0F0FUL, warn__ctrstand_endscpls},
   {s_rigger,    0x00FF0000UL, 0x33UL,   warn__ctrscpls_endstand},
   {s_rigger,    0x00CC0033UL, 0x33UL,   warn__ctrscpls_endstand},
   {s_rigger,    0x000000FFUL, 0xCCUL,   warn__ctrstand_endscpls},
   {s_rigger,    0x003300CCUL, 0xCCUL,   warn__ctrstand_endscpls},
   {s_bone,      0x00FF0000UL, 0x33UL,   warn__ctrstand_endscpls},
   {s_bone,      0x000000FFUL, 0xCCUL,   warn__ctrscpls_endstand},
   {s_crosswave, 0x00FF0000UL, 0xCCUL,   warn__ctrscpls_endstand},
   {s_crosswave, 0x000000FFUL, 0x33UL,   warn__ctrstand_endscpls},
   {sdeepbigqtg, 0xFFFF0000UL, 0x0F0FUL, warn__none},
   {sdeepbigqtg, 0x0000FFFFUL, 0xF0F0UL, warn__none},
   {sdeepbigqtg, 0x3535CACAUL, 0xC5C5UL, warn__none},
   {sdeepbigqtg, 0xCACA3535UL, 0x3A3AUL, warn__none},
   {sdeepbigqtg, 0xC5C53A3AUL, 0x3535UL, warn__none},
   {sdeepbigqtg, 0x3A3AC5C5UL, 0xCACAUL, warn__none},
   {nothing,     0,            0,        warn__none}};

siamese_item siamese_table_of_3[] = {
   {s2x6,        0x01C70E38UL, 0xE38UL,  warn__none},
   {s2x6,        0x0E3801C7UL, 0x1C7UL,  warn__none},
   {nothing,     0,            0,        warn__none}};

siamese_item siamese_table_of_4[] = {
   {s2x8,        0x0F0FF0F0UL, 0xF0F0UL, warn__none},
   {s2x8,        0xF0F00F0FUL, 0x0F0FUL, warn__none},
   {nothing,     0,            0,        warn__none}};

static void initialize_one_table(tm_thing *map_start, int np)
{
   tm_thing *map_search;

   for (map_search = map_start; map_search->outsetup != nothing; map_search++) {
      int i, j;
      map_search->insinglemaskhigh = 0;
      map_search->insinglemasklow = 0;
      map_search->outsinglemask = 0;
      map_search->map_is_eighth_twosome = false;

      uint32 osidemask = 0;
      // All 1's for people in outer setup.
      map_search->outunusedmask = (1 << (attr::klimit(map_search->outsetup)+1))-1;

      for (i=0; i<map_search->limit; i++) {
         if (map_search->maps[i] == -2) continue;

         map_search->outunusedmask &= ~(1 << map_search->maps[i]);

         if (map_search->maps[i+map_search->limit] < 0) {
            int bitpos = i*3;
            if (bitpos >= 32)
               map_search->insinglemaskhigh |= 1 << (bitpos-32);
            else
               map_search->insinglemasklow |= 1 << bitpos;

            map_search->outsinglemask |= 1 << map_search->maps[i];
         }
         else {
            for (j=1 ; j<np ; j++)
               map_search->outunusedmask &= ~(1 << map_search->maps[i+(map_search->limit*j)]);

            // ******** here is where we check for twosome map.

            int lowbitpos = i*3;
            uint32 ilatlowbit = (lowbitpos >= 32) ?
               map_search->ilatmask3high >> (lowbitpos-32) :
               map_search->ilatmask3low >> lowbitpos;

            // Once the "map_is_eighth_twosome" bit is set, "osidemask",
            // and the "map_search->olatmask" field, are undefined.
            if (ilatlowbit & 1)
               map_search->map_is_eighth_twosome = true;

            int midbitpos = lowbitpos+1;
            uint32 ilatmidbit = (midbitpos >= 32) ?
               map_search->ilatmask3high >> (midbitpos-32) :
               map_search->ilatmask3low >> midbitpos;

            if ((ilatmidbit ^ map_search->rot) & 1) {
               for (j=0 ; j<np ; j++)
                  osidemask |= 1 << map_search->maps[i+(map_search->limit*j)];
            }
         }
      }

      if (map_search->limit != attr::klimit(map_search->insetup)+1)
         gg->fatal_error_exit(1, "Tandem table initialization failed", "limit wrong");

      if (!map_search->map_is_eighth_twosome) {
         if (map_search->olatmask != osidemask)
            gg->fatal_error_exit(1, "Tandem table initialization failed", "smask");
      }
   }
}


extern void initialize_tandem_tables()
{
   initialize_one_table(maps_isearch_twosome, 2);
   initialize_one_table(maps_isearch_threesome, 3);
   initialize_one_table(maps_isearch_foursome, 4);
   initialize_one_table(maps_isearch_sixsome, 6);
   initialize_one_table(maps_isearch_eightsome, 8);
   initialize_one_table(maps_isearch_boxsome, 4);
   initialize_one_table(maps_isearch_dmdsome, 4);
   initialize_one_table(maps_isearch_tglsome, 3);
   initialize_one_table(maps_isearch_3x1tglsome, 4);
   initialize_one_table(maps_isearch_ysome, 4);
}


static void unpack_us(
   tm_thing *map_ptr,
   uint32 orbitmask3high,
   uint32 orbitmask3low,
   tandrec *tandstuff,
   setup *result) THROW_DECL
{
   int i, j;
   uint32 sglhigh, sgllow, ohigh, olow, r;

   r = map_ptr->rot*011;

   // The result of the unpacking goes to an enormous "hyper" array.
   // This array may be bigger than the maximum allowed value of 24.
   // We will then reduce it to the actual setup.

   personrec hyperarray[32];
   memset(hyperarray, 0, sizeof(personrec)*32);
   uint32 hyperarrayoccupation = 0;

   for (i=0,
           sglhigh=map_ptr->insinglemaskhigh,
           sgllow=map_ptr->insinglemasklow,
           ohigh=orbitmask3high,
           olow=orbitmask3low;
        i<map_ptr->limit;
        i++,
           sgllow >>= 3,
           sgllow |= sglhigh << 29,
           sglhigh >>= 3,
           olow >>= 3,
           olow |= ohigh << 29,
           ohigh >>= 3) {
      uint32 z = rotperson(tandstuff->virtual_result.people[i].id1, r);
      if (z != 0) {
         int ii = (z >> 6) & 7;

         // ****** Here is where things get tricky!

         bool invert_order =
            (((olow>>1) + (map_ptr->rot&1) + 1) & 2) && !tandstuff->no_unit_symmetry;

         // Figure out whether we are unpacking a single person or multiple people.
         int howmanytounpack = 1;

         if (!(sgllow & 1)) {
            howmanytounpack = tandstuff->np;
            if (map_ptr->maps[i+map_ptr->limit] < 0)
               fail("This would go to an impossible setup.");
         }

         personrec fb[8];

         for (j=0 ; j<howmanytounpack ; j++) {
            fb[j] = tandstuff->real_saved_people[j][ii];
            if (fb[j].id1) fb[j].id1 =
                              (fb[j].id1 & ~(NROLL_MASK|STABLE_ALL_MASK|077)) |
                              (z & (NROLL_MASK|STABLE_ALL_MASK|013));
         }

         for (j=0 ; j<howmanytounpack ; j++) {
            int unpack_order = invert_order ? howmanytounpack-j-1 : j;
            if (fb[unpack_order].id1 != 0) {
               int hyperarrayplace = map_ptr->maps[i+(map_ptr->limit*j)];
               // This could only arise in pathological uses of the 1/8 skewsome
               // maps.  (And probably not even in that case.)
               if (hyperarray[hyperarrayplace].id1)
                  fail("Can't do couples or tandem concepts in this setup.");
               hyperarray[hyperarrayplace] = fb[unpack_order];
               hyperarrayoccupation |= 1<< hyperarrayplace;
            }
         }
      }
   }

   static const veryshort fixer_4x8a_4x4[24] = {
      5, 10, 29, 11, 18, 19, 20, 28,
      21, 26, 13, 27, 2, 3, 4, 12,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_4x8a_2x8[24] = {
      15, 14, 13, 12, 11, 10, 9, 8,
      31, 30, 29, 28, 27, 26, 25, 24,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_4x8b_2x4h[24] = {
      14, 13, 10, 9, 30, 29, 26, 25,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_4x8b_2x4v[24] = {
      4, 11, 28, 19, 20, 27, 12, 3,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_2x4_3x8[24] = {
      2, 3, 4, 5, 14, 15, 16, 17,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_1x8_3x8[24] = {
      20, 21, 23, 22, 8, 9, 11, 10,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_2x8_2x16[24] = {
      4, 5, 6, 7, 8, 9, 10, 11,
      20, 21, 22, 23, 24, 25, 26, 27,
      -1, -1, -1, -1, -1, -1, -1, -1};

   static const veryshort fixer_2x12_2x24[24] = {
      2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
      18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

   static const veryshort lilstar1[8] = {6, 7, 1, 4, 0, 0, 0, 0};
   static const veryshort lilstar2[8] = {2, 3, 5, 0, 0, 0, 0, 0};
   static const veryshort lilstar3[8] = {0, 1, 4, 5, 0, 0, 0, 0};
   static const veryshort lilstar4[8] = {6, 7, 2, 3, 0, 0, 0, 0};

   result->kind = map_ptr->outsetup;
   result->rotation = tandstuff->virtual_result.rotation - map_ptr->rot;
   result->result_flags = tandstuff->virtual_result.result_flags;

   const veryshort *my_huge_map = (const veryshort *) 0;
   int rot = 0;

   if (result->kind == shyper4x8a) {
      if ((hyperarrayoccupation & 0xC3C3C3C3) == 0) {
         result->kind = s4x4;
         my_huge_map = fixer_4x8a_4x4;
      }
      else if ((hyperarrayoccupation & 0x00FF00FF) == 0) {
         result->kind = s2x8;
         my_huge_map = fixer_4x8a_2x8;
      } 
   }
   else if (result->kind == shyper4x8b) {
      if ((hyperarrayoccupation & 0x99FF99FF) == 0) {
         result->kind = s2x4;
         my_huge_map = fixer_4x8b_2x4h;
      }
      else if ((hyperarrayoccupation & 0xE7E7E7E7) == 0) {
         result->kind = s2x4;
         my_huge_map = fixer_4x8b_2x4v;
         result->rotation++;
         rot = 033;
      }
   }
   else if (result->kind == shyper3x8) {
      if ((hyperarrayoccupation & 0xFC3FC3) == 0) {
         result->kind = s2x4;
         my_huge_map = fixer_2x4_3x8;
      }
      else if ((hyperarrayoccupation & 0x0FF0FF) == 0) {
         result->kind = s1x8;
         my_huge_map = fixer_1x8_3x8;
      }
   }
   else if (result->kind == shyper2x16) {
      if ((hyperarrayoccupation & 0xF00FF00F) == 0) {
         result->kind = s2x8;
         my_huge_map = fixer_2x8_2x16;
      }
      else if ((hyperarrayoccupation & 0xC003C003) == 0) {
         result->kind = s2x12;
         my_huge_map = fixer_2x12_2x24;
      }
   }
   else if (result->kind == slittlestars) {
      if ((hyperarrayoccupation & 0xCC) == 0) {
         result->kind = s2x2;
         my_huge_map = lilstar3;
      }
      else if ((hyperarrayoccupation & 0x33) == 0) {
         result->kind = s1x4;
         my_huge_map = lilstar4;
      }
      else if ((hyperarrayoccupation & 0x2D) == 0) {
         result->kind = s_trngl4;
         my_huge_map = lilstar1;
         result->rotation--;
         rot = 011;
      }
      else if ((hyperarrayoccupation & 0xD2) == 0) {
         result->kind = s_trngl4;
         my_huge_map = lilstar2;
         result->rotation++;
         rot = 033;
      }
   }
   else
      my_huge_map = identity24;

   if (!my_huge_map)
      fail("This would go to an impossible setup.");

   for (i=0; i<MAX_PEOPLE; i++) {
      if (my_huge_map[i] >= 0)
         result->people[i] = hyperarray[my_huge_map[i]];
         result->people[i].id1 = rotperson(hyperarray[my_huge_map[i]].id1, rot);
   }
}



// Lat = 0 means the people we collapsed, relative to the incoming geometry,
// were one behind the other.  Lat = 1 means they were lateral.  "Incoming
// geometry" does not include the incoming rotation field, since we ignore it.
// We are not responsible for the rotation field of the incoming setup.

// The canonical storage of the real people, while we are doing the virtual
// call, is as follows:
//    Real_saved_people[0] gets person on left (lat=1) near person (lat=0).
//    Real_saved_people[last] gets person on right (lat=1) or far person (lat=0). */

// This returns true if it found people facing the wrong way.  This can happen
// if we are trying siamese and we shouldn't be.

static bool pack_us(
   personrec *s,
   tm_thing *map_ptr,
   int fraction,    // This is in eighths.
   bool fraction_in_use,
   bool dynamic_in_use,
   int key,
   tandrec *tandstuff) THROW_DECL
{
   int i, j;
   uint32 mhigh, mlow, sglhigh, sgllow;
   int virt_index = -1;

   tandstuff->virtual_setup.clear_people();
   tandstuff->virtual_setup.rotation = map_ptr->rot & 3;
   tandstuff->virtual_setup.kind = map_ptr->insetup;

   for (i=0,
           mhigh=map_ptr->ilatmask3high,
           mlow=map_ptr->ilatmask3low,
           sglhigh=map_ptr->insinglemaskhigh,
           sgllow=map_ptr->insinglemasklow;
        i<map_ptr->limit;
        i++,
           sgllow >>= 3,
           sgllow |= sglhigh << 29,
           mlow >>= 3,
           mlow |= mhigh << 29) {

      // Process a virtual person, put together from some number (1 or tandstuff->np)
      // of people working together.

      personrec fb[8];    // Will receive the people being assembled.
      personrec *ptr = &tandstuff->virtual_setup.people[i];
      uint32 vp1, vp2, vp3;
      // We know (hopefully) that this map doesn't have the "1/8 twosome" stuff, so the bits will be AB0.
      // This will get an error, of course.  Where m was ......AB,
      // mlow is .....AB0.  Or maybe AB1, in which case this map must not be used.

      // Just in case...
      if (mlow & 1) return true;

      int vert = (1 + map_ptr->rot + (mlow >> 1)) & 3;

      fb[0].id1 = 0;
      fb[0].id2 = 0;
      fb[0].id3 = 0;
      int thingy = map_ptr->maps[i];
      if (thingy >= 0) fb[0] = s[thingy];

      if (!tandstuff->no_unit_symmetry) vert &= 1;

      if (sgllow & 1) {
         // This person is not paired, as in "boys are tandem" when we are a girl.
         vp1 = fb[0].id1;
         vp2 = fb[0].id2;
         vp3 = fb[0].id3;
         fb[1].id1 = ~0UL;
      }
      else {
         // This person is paired; pick up the other real people.
         uint32 orpeople1 = fb[0].id1;
         uint32 andpeople1 = fb[0].id1;

         for (j=1 ; j<tandstuff->np ; j++) {
            fb[j].id1 = 0;
            fb[j].id2 = 0;
            int thingy2 = map_ptr->maps[i+(map_ptr->limit*j)];
            if (thingy2 >= 0) fb[j] = s[thingy2];
            orpeople1 |= fb[j].id1;
            andpeople1 &= fb[j].id1;
         }

         if (key == tandem_key_skew) {
            // If this is skew/skewsome, we require people paired in the appropriate way.
            // This means [fb[0], fb[3]] must match, [fb[1], fb[2]] must match, and
            // [fb[0], fb[1]] must not match.

            // But, if phantoms are allowed, and nobody is home, we allow it.
            // And we allow empty spots if the reduced setup is larger than 4,
            // since we couldn't possibly fill them.  This allows skewsome Z axle.

            if ((orpeople1 ||
                 !(tandstuff->virtual_setup.cmd.cmd_misc_flags & CMD_MISC__PHANTOMS)) &&
                (attr::slimit(&tandstuff->virtual_setup) < 4 || orpeople1) &&
                (((fb[0].id1 ^ fb[3].id1) |
                  (fb[1].id1 ^ fb[2].id1) |
                  (~(fb[0].id1 ^ fb[1].id1))) & BIT_PERSON))
               fail("Can't find skew people.");
         }
         else if (!(tandstuff->virtual_setup.cmd.cmd_misc_flags & CMD_MISC__PHANTOMS) &&
                  !tandstuff->phantom_pairing_ok) {

            // Otherwise, we usually forbid phantoms unless some phantom concept was used
            // (either something like "phantom tandem" or some other phantom concept such
            // as "split phantom lines").  If we get here, such a concept was not used.
            // We forbid a live person paired with a phantom.  Additionally, we forbid
            // ANY PERSON AT ALL to be a phantom, even if paired with another phantom,
            // except in the special case of a virtual 2x3.

            if (!(andpeople1 & BIT_PERSON)) {
               if (orpeople1 ||
                   (tandstuff->virtual_setup.kind != s2x3 &&
                    key != tandem_key_siam))
                  fail("Use \"phantom\" concept in front of this concept.");
            }
         }

         if (orpeople1) {
            if ((fraction_in_use || dynamic_in_use) && (orpeople1 & STABLE_ALL_MASK))
               fail("Sorry, can't nest fractional stable/twosome.");

            vp1 = ~0UL;
            vp2 = ~0UL;
            vp3 = ~0UL;

            // Create the virtual person.  When both people are present, anding
            // the real peoples' id2 bits gets the right bits.  For example,
            // the virtual person will be a boy and able to do a tandem star thru
            // if both real people were boys.  Remove the identity field (700 bits)
            // from id1 and replace with a virtual person indicator.  Check that
            // direction, roll, and stability parts of id1 are consistent.

            for (j=0 ; j<tandstuff->np ; j++) {
               if (fb[j].id1) {
                  vp1 &= fb[j].id1;
                  vp2 &= fb[j].id2;
                  vp3 &= fb[j].id3;

                  // If they have different fractional stability states,
                  // just clear them -- they can't do it.
                  if ((fb[j].id1 ^ orpeople1) & STABLE_ALL_MASK) vp1 &= ~STABLE_ALL_MASK;
                  // If they have different roll states, just clear them -- they can't roll.
                  if ((fb[j].id1 ^ orpeople1) & NROLL_MASK) vp1 &= ~NROLL_MASK;
                  // Check that all real people face the same way.
                  if ((fb[j].id1 ^ orpeople1) & 077)
                     return true;
               }
            }
         }
         else
            vp1 = 0;
      }

      if (vp1) {

         /* Create a virtual person number.  Only 8 are allowed, because of the tightness
            in the person representation.  That shouldn't be a problem, since each
            virtual person came from a group of real people that had at least one
            live person.  Unless active phantoms have been created, that is. */

         if ((++virt_index) >= 8) fail("Sorry, too many tandem or as couples people.");

         ptr->id1 = (vp1 & ~0700) | (virt_index << 6) | BIT_TANDVIRT;
         ptr->id2 = vp2;
         ptr->id3 = vp3;

         if (!(sgllow & 1)) {
            if (fraction_in_use)
               ptr->id1 |= STABLE_ENAB | (STABLE_RBIT * fraction);
            else if (dynamic_in_use)
               ptr->id1 |= STABLE_ENAB;

            // 1 if original people were near/far; 0 if lateral.
            tandstuff->vertical_people[virt_index] = vert;
         }

         if (map_ptr->rot & 1)   // Compensate for setup rotation.
            ptr->id1 = rotperson(ptr->id1, (-map_ptr->rot & 3) * 011);

         for (j=0 ; j<tandstuff->np ; j++)
            tandstuff->real_saved_people[j][virt_index] = fb[j];

         tandstuff->saved_originals[virt_index] = ptr->id1 + tandstuff->virtual_setup.rotation;
      }
      else {
         ptr->id1 = 0;
         ptr->id2 = 0;
         ptr->id3 = 0;
      }
   }

   return false;
}



extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,           // solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3
                          // also, 4 bit => dynamic, 8 bit => reverse dynamic
   int fraction_fields,   // number fields, if doing fractional twosome/solid
   int phantom,           // normal=0 phantom=1 general-gruesome=2 gruesome-with-wave-check=3
   tandem_key key,
   uint32 mxn_bits,
   bool phantom_pairing_ok,
   setup *result) THROW_DECL
{
   uint32 dynamic = twosome >> 2;
   twosome &= 3;

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__DO_NOT_EXECUTE) {
      result->kind = nothing;
      clear_result_flags(result);
      return;
   }

   selector_kind saved_selector;
   tandrec tandstuff;
   tm_thing *map;
   tm_thing *map_search;
   int i, np;
   uint32 jbit;
   bool fractional = false;
   int fraction_in_eighths = 0;
   bool fraction_eighth_part = false;
   bool dead_conc = false;
   bool dead_xconc = false;
   tm_thing *our_map_table;

   uint32 special_mask = 0;
   tandstuff.single_mask = 0;
   tandstuff.no_unit_symmetry = false;
   tandstuff.phantom_pairing_ok = phantom_pairing_ok;
   result->clear_people();
   remove_z_distortion(ss);

   if (mxn_bits != 0) {
      tandem_key transformed_key = key;

      if (key == tandem_key_tand3 || key == tandem_key_tand4)
         transformed_key = tandem_key_tand;
      else if (key == tandem_key_cpls3 || key == tandem_key_cpls4)
         transformed_key = tandem_key_cpls;
      else if (key == tandem_key_siam3 || key == tandem_key_siam4)
         transformed_key = tandem_key_siam;

      if ((transformed_key & ~1) != 0) fail("Can't do this combination of concepts.");

      switch (mxn_bits) {
      case INHERITFLAGNXNK_3X3:
         np = 3;
         our_map_table = maps_isearch_threesome;
         goto foobarves;
      case INHERITFLAGNXNK_4X4:
         np = 4;
         our_map_table = maps_isearch_foursome;
         goto foobarves;
      case INHERITFLAGNXNK_6X6:
         np = 6;
         our_map_table = maps_isearch_sixsome;
         goto foobarves;
      case INHERITFLAGNXNK_8X8:
         np = 8;
         our_map_table = maps_isearch_eightsome;
         goto foobarves;
      }

      {
         // ****** Maybe all this could be done in terms of "do_1x3_type_expansion".

         uint32 livemaskl, livemaskr, directionsl, directionsr;
         big_endian_get_directions(ss, directionsr, livemaskr, &directionsl, &livemaskl);

         if (mxn_bits == INHERITFLAGMXNK_2X1 || mxn_bits == INHERITFLAGMXNK_1X2) {
            np = 2;
            our_map_table = maps_isearch_twosome;

            if (ss->kind == s2x3 || ss->kind == s1x6) {
               if (transformed_key == tandem_key_tand) directionsr ^= 0x555;

               if (((directionsr ^ 0x0A8) & livemaskr) == 0 ||
                   ((directionsr ^ 0xA02) & livemaskr) == 0)
                  special_mask |= 044;

               if (((directionsr ^ 0x2A0) & livemaskr) == 0 ||
                   ((directionsr ^ 0x80A) & livemaskr) == 0)
                  special_mask |= 011;

               if (mxn_bits == INHERITFLAGMXNK_2X1 && ((directionsr ^ 0x02A) & livemaskr) == 0)
                  special_mask |= 011;

               if (mxn_bits == INHERITFLAGMXNK_2X1 && ((directionsr ^ 0xA80) & livemaskr) == 0)
                  special_mask |= 044;

               if (mxn_bits == INHERITFLAGMXNK_1X2 && ((directionsr ^ 0x02A) & livemaskr) == 0)
                  special_mask |= 044;

               if (mxn_bits == INHERITFLAGMXNK_1X2 && ((directionsr ^ 0xA80) & livemaskr) == 0)
                  special_mask |= 011;

               if (special_mask != 011 && special_mask != 044) special_mask = 0;
            }
         }
         else if (mxn_bits == INHERITFLAGMXNK_3X1 || mxn_bits == INHERITFLAGMXNK_1X3) {
            np = 3;
            our_map_table = maps_isearch_threesome;
            if (transformed_key == tandem_key_tand) directionsr ^= 0x555555;

            if (ss->kind == s2x4) {
               if (((directionsr ^ 0x02A8) & livemaskr) == 0 ||
                   ((directionsr ^ 0xA802) & livemaskr) == 0)
                  special_mask |= 0x88;

               if (((directionsr ^ 0x2A80) & livemaskr) == 0 ||
                   ((directionsr ^ 0x802A) & livemaskr) == 0)
                  special_mask |= 0x11;

               if (mxn_bits == INHERITFLAGMXNK_3X1 && ((directionsr ^ 0x00AA) & livemaskr) == 0)
                  special_mask |= 0x11;

               if (mxn_bits == INHERITFLAGMXNK_3X1 && ((directionsr ^ 0xAA00) & livemaskr) == 0)
                  special_mask |= 0x88;

               if (mxn_bits == INHERITFLAGMXNK_1X3 && ((directionsr ^ 0x00AA) & livemaskr) == 0)
                  special_mask |= 0x88;

               if (mxn_bits == INHERITFLAGMXNK_1X3 && ((directionsr ^ 0xAA00) & livemaskr) == 0)
                  special_mask |= 0x11;

               if (special_mask != 0x11 && special_mask != 0x88) special_mask = 0;
            }
            else if (ss->kind == s3x4 && livemaskr == 0xC3FC3F) {
               // Don't look at facing directions; there's only one way it can be.
               special_mask = 0x820;
            }
            else if (ss->kind == s3x4 && livemaskr == 0x3CF3CF) {
               special_mask = 0x410;
            }
            else if (ss->kind == s3x6 && livemaskl == 0x3 && livemaskr == 0x00FCC03F) {
               special_mask = 0022022;
            }
            else if (ss->kind == s3x6 && livemaskl == 0x0 && livemaskr == 0x0CFC033F) {
               special_mask = 0022022;
            }
            else if (ss->kind == s3x6 && livemaskl == 0xF && livemaskr == 0xC033F00C) {
               special_mask = 0220220;
            }
            else if (ss->kind == s3x6 && livemaskl == 0x0 && livemaskr == 0x3F300FCC) {
               special_mask = 0202202;
            }
            else if (ss->kind == s3dmd && livemaskr == 0xFC3FC3) {
               special_mask = 0x820;
            }
            else if (ss->kind == s_qtag) {
               special_mask = 0x44;
            }
            else if (ss->kind == s1x8) {
               if (((directionsr ^ 0x08A2) & livemaskr) == 0 ||
                   ((directionsr ^ 0xA208) & livemaskr) == 0)
                  special_mask |= 0x44;

               if (((directionsr ^ 0x2A80) & livemaskr) == 0 ||
                   ((directionsr ^ 0x802A) & livemaskr) == 0)
                  special_mask |= 0x11;

               if (mxn_bits == INHERITFLAGMXNK_3X1 && ((directionsr ^ 0x00AA) & livemaskr) == 0)
                  special_mask |= 0x11;

               if (mxn_bits == INHERITFLAGMXNK_3X1 && ((directionsr ^ 0xAA00) & livemaskr) == 0)
                  special_mask |= 0x44;

               if (mxn_bits == INHERITFLAGMXNK_1X3 && ((directionsr ^ 0x00AA) & livemaskr) == 0)
                  special_mask |= 0x44;

               if (mxn_bits == INHERITFLAGMXNK_1X3 && ((directionsr ^ 0xAA00) & livemaskr) == 0)
                  special_mask |= 0x11;

               if (special_mask != 0x11 && special_mask != 0x44) special_mask = 0;
            }
            else if (ss->kind == s3x1dmd) {
               if (((directionsr ^ 0x00A8) & 0xFCFC & livemaskr) == 0 ||
                   ((directionsr ^ 0xA800) & 0xFCFC & livemaskr) == 0)
                  special_mask |= 0x88;
            }
         }
         else
            fail("Can't do this combination of concepts.");

         if (!special_mask) fail("Can't find 3x3/3x1/2x1 people.");
      }

      foobarves:

      // This will make it look like "as couples" or "tandem", as needed,
      // for swapping masks, but won't trip the assumption transformation stuff.
      // It will also turn "threesome" into "twosome", as required by the rest of the code.
      key = (tandem_key) (transformed_key | 64);
   }
   else if (key == tandem_key_ys) {
      np = 4;
      our_map_table = maps_isearch_ysome;
      tandstuff.no_unit_symmetry = true;
   }
   else if (key == tandem_key_3x1tgls) {
      np = 4;
      our_map_table = maps_isearch_3x1tglsome;
      tandstuff.no_unit_symmetry = true;
   }
   else if (key >= tandem_key_outpoint_tgls) {
      np = 3;
      our_map_table = maps_isearch_tglsome;
      tandstuff.no_unit_symmetry = true;
      if (key == tandem_key_outside_tgls)
         selector = selector_outer6;
      else if (key == tandem_key_inside_tgls)
         selector = selector_center6;
      else if (key == tandem_key_wave_tgls || key == tandem_key_tand_tgls) {
         if (ss->kind == s_hrglass) {
            uint32 tbonetest =
               ss->people[0].id1 | ss->people[1].id1 |
               ss->people[4].id1 | ss->people[5].id1;

            if ((tbonetest & 011) == 011 || ((key ^ tbonetest) & 1))
               fail("Can't find the indicated triangles.");

            special_mask = 0x44;   // The triangles have to be these.
         }
         else if (ss->kind == s_bone) {
            uint32 tbonetest =
               ss->people[0].id1 | ss->people[1].id1 |
               ss->people[4].id1 | ss->people[5].id1;

            if ((tbonetest & 011) == 011 || !((key ^ tbonetest) & 1))
               fail("Can't find the indicated triangles.");

            special_mask = 0x88;   // The triangles have to be these.
         }
         else if (ss->kind == s_rigger) {
            uint32 tbonetest =
               ss->people[0].id1 | ss->people[1].id1 |
               ss->people[4].id1 | ss->people[5].id1;

            if ((tbonetest & 011) == 011 || !((key ^ tbonetest) & 1))
               fail("Can't find the indicated triangles.");

            special_mask = 0x44;   // The triangles have to be these.
         }
         else if (ss->kind == s_dhrglass) {
            uint32 tbonetest =
               ss->people[0].id1 | ss->people[1].id1 |
               ss->people[4].id1 | ss->people[5].id1;

            if ((tbonetest & 011) == 011 || !((key ^ tbonetest) & 1))
               fail("Can't find the indicated triangles.");

            special_mask = 0x88;   // The triangles have to be these.
         }
         else if (ss->kind == s_galaxy) {
            uint32 tbonetest =
               ss->people[1].id1 | ss->people[3].id1 |
               ss->people[5].id1 | ss->people[7].id1;

            if ((tbonetest & 011) == 011)
               fail("Can't find the indicated triangles.");

            if ((key ^ tbonetest) & 1)
               special_mask = 0x44;
            else
               special_mask = 0x11;
         }
         else if (ss->kind == s_c1phan) {
            uint32 tbonetest = 0;
            int t = key;

            for (i=0; i<16; i++) tbonetest |= ss->people[i].id1;

            if ((tbonetest & 010) == 0) t ^= 1;
            else if ((tbonetest & 1) != 0)
               fail("Can't find the indicated triangles.");

            if (t&1) special_mask = 0x2121;
            else     special_mask = 0x1212;
         }
         else
            fail("Can't find these triangles.");
      }
      else if ((key == tandem_key_outpoint_tgls || key == tandem_key_inpoint_tgls) &&
               ss->kind == s_qtag) {
         if (key == tandem_key_inpoint_tgls) {
            if (     (ss->people[0].id1 & d_mask) == d_east &&
                     (ss->people[1].id1 & d_mask) != d_west &&
                     (ss->people[4].id1 & d_mask) == d_west &&
                     (ss->people[5].id1 & d_mask) != d_east)
               special_mask = 0x22;
            else if ((ss->people[0].id1 & d_mask) != d_east &&
                     (ss->people[1].id1 & d_mask) == d_west &&
                     (ss->people[4].id1 & d_mask) != d_west &&
                     (ss->people[5].id1 & d_mask) == d_east)
               special_mask = 0x11;
         }
         else {
            if (     (ss->people[0].id1 & d_mask) != d_west &&
                     (ss->people[1].id1 & d_mask) == d_east &&
                     (ss->people[4].id1 & d_mask) != d_east &&
                     (ss->people[5].id1 & d_mask) == d_west)
               special_mask = 0x11;
            else if ((ss->people[0].id1 & d_mask) == d_west &&
                     (ss->people[1].id1 & d_mask) != d_east &&
                     (ss->people[4].id1 & d_mask) == d_east &&
                     (ss->people[5].id1 & d_mask) != d_west)
               special_mask = 0x22;
         }

         if (special_mask == 0)
            fail("Can't find designated point.");
      }
      else if (key != tandem_key_anyone_tgls || ss->kind != s_c1phan)
         // For <anyone>-based triangles in C1-phantom,
         // we just use whatever selector was given.
         fail("Can't find these triangles.");
   }
   else if (key == tandem_key_diamond) {
      np = 4;
      our_map_table = maps_isearch_dmdsome;
   }
   else if (key == tandem_key_box || key == tandem_key_skew) {
      np = 4;
      our_map_table = maps_isearch_boxsome;
   }
   else if (key >= tandem_key_tand4) {
      np = 4;
      our_map_table = maps_isearch_foursome;
   }
   else if (key >= tandem_key_tand3) {
      np = 3;
      our_map_table = maps_isearch_threesome;
   }
   else {
      np = 2;
      our_map_table = maps_isearch_twosome;
   }

   if (attr::slimit(ss) < 0)
      fail("Can't do tandem/couples concept from this position.");

   // We use the phantom indicator to forbid an already-distorted setup.
   // The act of forgiving phantom pairing is based on the setting of the
   // CMD_MISC__PHANTOMS bit in the incoming setup, not on the phantom indicator.

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED) && (phantom != 0))
      fail("Can't specify phantom tandem/couples in virtual or distorted setup.");

   // Find out who is selected, if this is a "so-and-so are tandem".
   saved_selector = current_options.who;

   if (selector != selector_uninitialized)
      current_options.who = selector;

   uint32 nsmask = 0;
   uint32 ewmask = 0;
   uint32 allmask = 0;

   for (i=0, jbit=1; i<=attr::slimit(ss); i++, jbit<<=1) {
      uint32 p = ss->people[i].id1;
      if (p) {
         allmask |= jbit;
         // We allow a "special" mask to override the selector.
         // We also tell "selectp" that "some" is a legal selector.
         if ((selector != selector_uninitialized && !selectp(ss, i, np)) ||
             (jbit & special_mask) != 0)
            tandstuff.single_mask |= jbit;
         else {
            if (p & 1)
               ewmask |= jbit;
            else
               nsmask |= jbit;
         }
      }
   }

   current_options.who = saved_selector;

   if (!allmask) {
      result->kind = nothing;
      clear_result_flags(result);
      return;
   }

   if (twosome >= 2) {
      fractional = true;

      int num = (fraction_fields & NUMBER_FIELD_MASK) << 2;
      int fractional_twosome_part_den = fraction_fields >> BITS_PER_NUMBER_FIELD;

      fraction_in_eighths =
         num / fractional_twosome_part_den;
      int fractional_twosome_part_num =
         num - fraction_in_eighths * fractional_twosome_part_den;

      int fracgcd = gcd(fractional_twosome_part_num, fractional_twosome_part_den);
      fractional_twosome_part_num /= fracgcd;
      fractional_twosome_part_den /= fracgcd;

      fraction_in_eighths <<= 1;  // Put it into eighths temporarily.

      if (fractional_twosome_part_num != 0) {
         if (fractional_twosome_part_num != 1 || fractional_twosome_part_den != 2)
            fail("Fraction is too complicated.");
         fraction_in_eighths++;
         fraction_eighth_part = true;
      }

      if (fraction_in_eighths > 8)
         fail("Can't do fractional twosome more than 4/4.");
   }

   uint32 siamese_fixup(0);
   warning_index siamese_warning = warn__none;
   siamese_item *ptr;
   bool doing_siamese = false;
   uint32 saveew, savens;

   if (key == tandem_key_box || key == tandem_key_skew) {
      ewmask = allmask;
      nsmask = 0;
   }
   else if (key == tandem_key_diamond) {
      if (ss->kind == s_ptpd || ss->kind == s3ptpd || ss->kind == s4ptpd || ss->kind == sdmd) {
         ewmask = allmask;
         nsmask = 0;
      }
      else {
         ewmask = 0;
         nsmask = allmask;
      }
   }
   else if (tandstuff.no_unit_symmetry) {
      if (key == tandem_key_anyone_tgls) {
         /* This was <anyone>-based triangles.  The setup must have been a C1-phantom.
            The current mask shows just the base.  Expand it to include the apex. */
         ewmask |= nsmask;     /* Get the base bits regardless of facing direction. */
         if (allmask == 0xAAAA) {
            tandstuff.single_mask &= 0x7777;
            if (ewmask == 0x0A0A) {
               ewmask |= 0x8888;
               nsmask = 0;
            }
            else if (ewmask == 0xA0A0) {
               ewmask |= 0x8888;
               nsmask = ewmask;
               ewmask = 0;
            }
            else
               fail("Can't find these triangles.");
         }
         else if (allmask == 0x5555) {
            tandstuff.single_mask &= 0xBBBB;
            if (ewmask == 0x0505) {
               ewmask |= 0x4444;
               nsmask = ewmask;
               ewmask = 0;
            }
            else if (ewmask == 0x5050) {
               ewmask |= 0x4444;
               nsmask = 0;
            }
            else
               fail("Can't find these triangles.");
         }
         else
            fail("Can't find these triangles.");
      }
      else if (key == tandem_key_3x1tgls) {
         if (ss->kind == s2x6 || ss->kind == s4x6 || ss->kind == s2x12) {
            ewmask = 0;
            nsmask = allmask;
         }
         else {
            ewmask = allmask;
            nsmask = 0;
         }
      }
      else if (key == tandem_key_ys) {
         if (ss->kind == s_c1phan) {
            if (!((ss->people[0].id1 ^ ss->people[1].id1 ^ ss->people[4].id1 ^ ss->people[5].id1) & 2)) {
               ss->rotation++;
               canonicalize_rotation(ss);
               ss->rotation--;   /* Since we won't look at the ss->rotation again until the end,
                                    this will have the effect of rotating it back.
                                    Note that allmask, if it is acceptable at all, is still correct
                                    for the new rotation. */
            }
            ewmask = 0;
            nsmask = allmask;
         }
         else {
            ewmask = allmask;
            nsmask = 0;
         }
      }
      else if (ss->kind == s_galaxy) {
         if (special_mask == 0x44) {
            ewmask = allmask;
            nsmask = 0;
         }
         else if (special_mask == 0x11) {
            ewmask = 0;
            nsmask = allmask;
         }
         else
            fail("Can't find these triangles.");
      }
      else if ((special_mask == 0x44 && ss->kind != s_rigger) ||
               special_mask == 0x1212 ||
               special_mask == 0x22 ||
               special_mask == 0x11) {
         ewmask = 0;
         nsmask = allmask;
      }
      else if (special_mask == 0x2121) {
         ewmask = allmask;
         nsmask = 0;
      }
      else if (ss->kind == s_c1phan)
         fail("Sorry, don't know who is lateral.");
      else {
         ewmask = allmask;
         nsmask = 0;
      }
   }
   else if (key & 2) {

      // Siamese.  Usually, we will insist on a true siamese separation.
      // But, if we fail, it may be because we are doing things "random
      // siamese" or whatever.  In that case, we will allow a pure
      // couples or tandem separation.

      doing_siamese = true;
      saveew = ewmask;
      savens = nsmask;

      if (key == tandem_key_siam3)
         ptr = siamese_table_of_3;
      else if (key == tandem_key_siam4)
         ptr = siamese_table_of_4;
      else
         ptr = siamese_table_of_2;

      uint32 A = allmask & 0xFFFF;
      uint32 AA = (A << 16) | A;
      uint32 EN = ((ewmask & 0xFFFF) << 16) | (nsmask & 0xFFFF);

      for (; ptr->testkind != nothing; ptr++) {
         if (ptr->testkind == ss->kind && ((EN ^ ptr->testval) & AA) == 0) {
            siamese_warning = ptr->warning;
            siamese_fixup = ptr->fixup & 0xFFFFFF;
            goto try_siamese;   // We seem to have a match.  However, it still might be wrong.
         }
      }

      goto siamese_failed;
   }
   else if (key & 1) {
      // Couples -- swap masks.  Tandem -- do nothing.
      uint32 temp = ewmask;
      ewmask = nsmask;
      nsmask = temp;
   }

 try_this:

   // If this is "tandem in a tall 6", be sure we get the map that assumes the ends
   // of the 2x6 are oriented differently.
   if ((ss->cmd.cmd_misc_flags & CMD_MISC__VERIFY_MASK) == CMD_MISC__VERIFY_TALL6) {
      if (ss->kind == s2x6) {
         ewmask |= 03636;
         nsmask |= 04141;
      }
      else if (ss->kind == sdeepxwv) {
         ewmask |= 00303;
         nsmask |= 07474;
      }
   }

   /* Now ewmask and nsmask have the info about who is paired with whom. */
   ewmask &= ~tandstuff.single_mask;         /* Clear out unpaired people. */
   nsmask &= ~tandstuff.single_mask;

   map_search = our_map_table;
   while (map_search->outsetup != nothing) {
      if ((map_search->outsetup == ss->kind) &&
          !map_search->map_is_eighth_twosome &&
            map_search->outsinglemask == tandstuff.single_mask &&
            (!(allmask & map_search->outunusedmask)) &&
            (!(ewmask & (~map_search->olatmask))) &&
            (!(nsmask & map_search->olatmask))) {
         map = map_search;
         goto fooy;
      }
      map_search++;
   }

   if (!doing_siamese)
      fail("Can't do this tandem or couples call in this setup or with these people selected.");

   goto siamese_failed;

 try_siamese:

   ewmask ^= (siamese_fixup & allmask);
   nsmask ^= (siamese_fixup & allmask);
   if (ptr->fixup & 0x80000000UL) tandstuff.phantom_pairing_ok = true;
   goto try_this;

 fooy:

   // We also use the subtle aspects of the phantom indicator to tell what kind
   // of setup we allow, and whether pairings must be parallel to the long axis.

   if (phantom == 1) {
      if (ss->kind != s2x8 && ss->kind != s4x4 && ss->kind != s3x4 && ss->kind != s2x6 &&
          ss->kind != s4x6 && ss->kind != s1x12 && ss->kind != s2x12 && ss->kind != s1x16 &&
          ss->kind != s3dmd && ss->kind != s_323 &&
          ss->kind != s4dmd && ss->kind != s3x8)
         fail("Can't do couples or tandem concepts in this setup.");
   }
   else if (phantom >= 2) {
      if (ss->kind != s2x8 || map->insetup != s2x4)
         fail("Can't do gruesome concept in this setup.");
   }

   tandstuff.np = np;
   tandstuff.virtual_setup.cmd = ss->cmd;
   tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_none;

   /* There are a small number of assumptions that we can transform. */

   if (ss->cmd.cmd_assume.assump_col == 4) {
      if (ss->cmd.cmd_assume.assumption == cr_ijright)
         tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_jright;
      else if (ss->cmd.cmd_assume.assumption == cr_ijleft)
         tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_jleft;
   }

   if (ss->cmd.cmd_assume.assumption == cr_jright ||
       ss->cmd.cmd_assume.assumption == cr_jleft)
      fail("Couples or tandem concept is inconsistent with phantom facing direction.");

   if (key == tandem_key_cpls) {
      if (ss->cmd.cmd_assume.assumption == cr_wave_only)
         fail("Couples or tandem concept is inconsistent with phantom facing direction.");
      else if (ss->cmd.cmd_assume.assumption == cr_li_lo) {
         if (ss->kind == s2x2 || ss->kind == s2x4)
            tandstuff.virtual_setup.cmd.cmd_assume.assumption = ss->cmd.cmd_assume.assumption;
      }
      else if (ss->cmd.cmd_assume.assumption == cr_1fl_only) {
         if (ss->kind == s1x4 || ss->kind == s2x4)
            tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_couples_only;
      }
      else if (ss->cmd.cmd_assume.assumption == cr_2fl_only) {
         if (ss->kind == s1x4 || ss->kind == s2x4 || ss->kind == s1x8)
            tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_wave_only;
      }
   }

   if (phantom == 3) tandstuff.virtual_setup.cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

   // Give it the fraction in quarters, rounded down if an eighth part was specified.
   if (!pack_us(ss->people, map, fraction_in_eighths, fractional, dynamic != 0, key, &tandstuff))
      goto got_good_separation;

   // Or failure to pack people properly may just be because we are taking
   // "siamese" too seriously.

   if (!doing_siamese)
      fail("People not facing same way for tandem or as couples.");

 siamese_failed:

   // We allow pointless siamese if this is a part of a call that is being done
   // "piecewise" or "random" or whatever.
   if (!(ss->cmd.cmd_fraction.flags & CMD_FRAC_BREAKING_UP))
      fail("Can't do Siamese in this setup.");

   nsmask = savens;
   ewmask = saveew;

   if (key & 2) {
      key = (tandem_key) (key & ~2);    // Turn off siamese -- it's now effectively as couples
      key = (tandem_key) (key | 1);
      uint32 temp(ewmask);
      ewmask = nsmask;
      nsmask = temp;
   }
   else if (key & 1)
      key = (tandem_key) (key & ~1);    // Try tandem
   else
      fail("Can't do this tandem or couples call in this setup or with these people selected.");

   goto try_this;

 got_good_separation:

   warn(siamese_warning);

   if (tandstuff.virtual_setup.kind == s_ntrgl6cw ||
       tandstuff.virtual_setup.kind == s_ntrgl6ccw)
      tandstuff.virtual_setup.cmd.cmd_misc_flags |= CMD_MISC__SAID_TRIANGLE;

   update_id_bits(&tandstuff.virtual_setup);
   impose_assumption_and_move(&tandstuff.virtual_setup, &tandstuff.virtual_result);
   remove_mxn_spreading(&tandstuff.virtual_result);
   remove_tgl_distortion(&tandstuff.virtual_result);

   // If this is a concentric setup with dead ends or centers, we can still handle it.
   // We have to remember to put dead folks back when we are done, in order
   // to make gluing illegal.

   if (tandstuff.virtual_result.kind == s_dead_concentric) {
      if (fraction_eighth_part) fail("Fraction is too complicated.");
      dead_conc = true;
      tandstuff.virtual_result.kind = tandstuff.virtual_result.inner.skind;
      tandstuff.virtual_result.rotation += tandstuff.virtual_result.inner.srotation;
   }
   else if (tandstuff.virtual_result.kind == s_normal_concentric &&
            tandstuff.virtual_result.inner.skind == nothing &&
            tandstuff.virtual_result.outer.skind == s1x2) {
      if (fraction_eighth_part) fail("Fraction is too complicated.");
      dead_xconc = true;
      tandstuff.virtual_result.kind = tandstuff.virtual_result.outer.skind;
      tandstuff.virtual_result.rotation += tandstuff.virtual_result.outer.srotation;
   }

   if (attr::slimit(&tandstuff.virtual_result) < 0)
      fail("Don't recognize ending position from this tandem or as couples call.");

   // Bits appear in this 64-bit item in triples!  All 3 bits are the same.
   uint32 sglmask3high = 0;
   uint32 sglmask3low = 0;

   // Bits appear in this 64-bit item in triples!  All 3 bits are the same.
   uint32 livemask3high = 0;
   uint32 livemask3low = 0;

   // Bits appear in this 64-bit item in triples!  The 3 bits give the orbit in eighths.
   uint32 orbitmask3high = 0;
   uint32 orbitmask3low = 0;

   // Compute orbitmask3, livemask3, and sglmask3.
   // Since we are synthesizing bit masks, we scan in reverse order to make things easier.
   // The resultant masks will be little-endian.

   for (i=attr::slimit(&tandstuff.virtual_result); i>=0; i--) {
      sglmask3high <<= 3;
      sglmask3high |= sglmask3low >> 29;
      sglmask3low <<= 3;

      livemask3high <<= 3;
      livemask3high |= livemask3low >> 29;
      livemask3low <<= 3;

      orbitmask3high <<= 3;
      orbitmask3high |= orbitmask3low >> 29;
      orbitmask3low <<= 3;

      uint32 p = tandstuff.virtual_result.people[i].id1;

      if (p) {
         int vpi = (p >> 6) & 7;
         livemask3low |= 7;

         if (tandstuff.real_saved_people[1][vpi].id1 == ~0UL) {
            sglmask3low |= 7;
         }
         else {
            if (fractional || dynamic != 0) {
               if (!(p & STABLE_ENAB))
                  fail("fractional twosome not supported for this call.");
            }

            int orbit_in_eighths =
               (p + tandstuff.virtual_result.rotation - tandstuff.saved_originals[vpi]) << 1;

            uint32 stable_stuff_in_eighths = ((p & STABLE_VRMASK) / STABLE_VRBIT) - ((p & STABLE_VLMASK) / STABLE_VLBIT);

            if (twosome == 3) {
               // This is "twosome to solid" -- they orbit by whatever the excess is after the stability expires.
               orbit_in_eighths = 0;
               if (fractional) {
                  if (dynamic == 1)
                     orbit_in_eighths += (p & STABLE_VRMASK) / STABLE_VRBIT;
                  else if (dynamic == 2)
                     orbit_in_eighths -= (p & STABLE_VLMASK) / STABLE_VLBIT;
                  else
                     orbit_in_eighths = stable_stuff_in_eighths;
               }
            }
            else if (twosome == 2) {
               // This is "solid to twosome" -- they orbit by whatever happened before the stability expired.
               if (fractional) {
                  if (dynamic == 1)
                     orbit_in_eighths += (p & STABLE_VLMASK) / STABLE_VLBIT;
                  else if (dynamic == 2)
                     orbit_in_eighths -= (p & STABLE_VRMASK) / STABLE_VRBIT;
                  else
                     orbit_in_eighths -= stable_stuff_in_eighths;
               }
            }
            else if (twosome == 1) {
               // This is "twosome" -- they don't orbit (unless "dynamic" is on).
               orbit_in_eighths = 0;
               if (dynamic == 1)
                  orbit_in_eighths += (p & STABLE_VRMASK) / STABLE_VRBIT;
               else if (dynamic == 2)
                  orbit_in_eighths -= (p & STABLE_VLMASK) / STABLE_VLBIT;
            }

            orbitmask3low |=
               (orbit_in_eighths - ((tandstuff.virtual_result.rotation + tandstuff.vertical_people[vpi]) << 1)) & 7;
         }

         if (fractional || dynamic != 0)
            tandstuff.virtual_result.people[i].id1 &= ~STABLE_ALL_MASK;
      }
   }

   uint32 orbitcomhigh = orbitmask3high ^ 0155555UL;
   uint32 orbitcomlow = orbitmask3low ^ 026666666666UL;
   uint32 hmask3high = orbitcomhigh & livemask3high & ~sglmask3high;
   uint32 hmask3low = orbitcomlow & livemask3low & ~sglmask3low;

   // Bits appear here in triples!  Only low bit of each triple is used.
   uint32 sglmaskhigh = sglmask3high & 022222UL;
   uint32 sglmasklow = sglmask3low & 011111111111UL;

   // Bits appear here in triples!  Only low two bits of each triple are used.
   uint32 livemaskhigh = livemask3high & 066666UL;
   uint32 livemasklow = livemask3low & 033333333333UL;

   // Pick out only low two bits for map search, and only bits of live paired people.
   uint32 hmaskhigh = hmask3high & 066666UL;
   uint32 hmasklow = hmask3low & 033333333333UL;

   if (tandstuff.no_unit_symmetry) {
      livemaskhigh = livemask3high;
      livemasklow = livemask3low;
      hmaskhigh = hmask3high;
      hmasklow = hmask3low;
   }

   map_search = our_map_table;
   // We don't accept the special 1/8-twosome maps from the "maps_isearch_boxsome"
   // table unless we are doing "skew".  If we are doing "box" (the only other
   // possibility) the mapes are not used.
   while (map_search->outsetup != nothing) {
      if ((map_search->insetup == tandstuff.virtual_result.kind) &&
          (!map_search->map_is_eighth_twosome || key != tandem_key_box) &&
          map_search->insinglemaskhigh == sglmaskhigh &&
          map_search->insinglemasklow == sglmasklow &&
          (map_search->ilatmask3high & livemaskhigh) == hmaskhigh &&
          (map_search->ilatmask3low & livemasklow) == hmasklow) {
         unpack_us(map_search, orbitmask3high, orbitmask3low, &tandstuff, result);
         canonicalize_rotation(result);
         reinstate_rotation(ss, result);

         // When we fudge wrongly-oriented triangles to a 2x4, we need
         // to say something.
         if (our_map_table == maps_isearch_tglsome && result->kind == s2x4)
            warn(warn__check_hokey_2x4);

         if (dead_conc) {
            result->inner.skind = result->kind;
            result->inner.srotation = result->rotation;
            result->rotation = 0;
            result->kind = s_dead_concentric;
         }
         else if (dead_xconc) {
            result->outer.skind = result->kind;
            result->outer.srotation = result->rotation;
            result->concsetup_outer_elongation = tandstuff.virtual_result.outer.srotation+1;
            result->rotation = 0;
            result->inner.skind = nothing;
            result->kind = s_normal_concentric;
            for (i=0 ; i<12 ; i++) result->swap_people(i, i+12);
         }
         else if (ss->kind == s1x4 && result->kind == s2x2) {
            result->result_flags.misc &= ~3;
            result->result_flags.misc |= (ss->rotation & 1) + 1;
         }

         return;
      }
      map_search++;
   }

   fail("Don't recognize ending position from this tandem or as couples call.");
}
