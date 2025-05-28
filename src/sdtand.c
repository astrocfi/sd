/* SD -- square dance caller's helper.

    Copyright (C) 1990-1997  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 31. */

/* This file contains stuff for tandem and as-couples moves. */

/* This defines the following functions:
   tandem_couples_move
   initialize_tandem_tables
*/

#include "sd.h"

typedef struct {
   personrec real_front_people[MAX_PEOPLE];
   personrec real_back_people[MAX_PEOPLE];
   personrec real_second_people[MAX_PEOPLE];
   personrec real_third_people[MAX_PEOPLE];
   int saved_originals[MAX_PEOPLE];
   setup virtual_setup;
   setup virtual_result;
   int vertical_people[MAX_PEOPLE];    /* 1 if original people were near/far; 0 if lateral */
   uint32 single_mask;
   long_boolean no_unit_symmetry;
   int np;
} tandrec;

typedef struct {
   int map1[12];
   int map2[12];
   int map3[12];
   int map4[12];
   uint32 ilatmask;           /* lateral pairs in inside numbering -- only "1" bits used! (except triangles) */
   uint32 olatmask;
   int limit;
   int rot;
   uint32 insinglemask;       /* relative to insetup numbering, those people that are NOT paired -- only alternate bits used! */
   uint32 outsinglemask;      /* relative to outsetup numbering, those people that are NOT paired */
   uint32 outunusedmask;
   setup_kind insetup;
   setup_kind outsetup;
} tm_thing;


Private tm_thing maps_isearch_twosome[] = {

/*         map1                              map2                map3  map4   ilatmask olatmask    limit rot            insetup outsetup            old name */
   {{7, 6, 4, 5},                   {0, 1, 3, 2},                 {0}, {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s2x4},            /* "2x4_4" - see below */
   {{0, 2, 5, 7},                   {1, 3, 4, 6},                 {0}, {0},   0x55,     0xFF,         4, 0,  0,  0, 0,  s2x2,  s2x4},
   {{2, 5, 7, 0},                   {3, 4, 6, 1},                 {0}, {0},      0,     0xFF,         4, 1,  0,  0, 0,  s2x2,  s2x4},
   {{3, 2},                         {0, 1},                       {0}, {0},      0,     0000,         2, 0,  0,  0, 0,  s1x2,  s2x2},
   {{0, 3},                         {1, 2},                       {0}, {0},      0,      0xF,         2, 1,  0,  0, 0,  s1x2,  s2x2},
   {{0, 3},                         {1, 2},                       {0}, {0},    0x5,      0xF,         2, 0,  0,  0, 0,  s1x2,  s1x4},
   {{0},                            {1},                          {0}, {0},    0x1,     0003,         1, 0,  0,  0, 0,  s1x1,  s1x2},
   {{0},                            {1},                          {0}, {0},      0,     0003,         1, 1,  0,  0, 0,  s1x1,  s1x2},
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                 {0}, {0},   0x55,     0xFF,         4, 0,  0,  0, 0,  s1x4,  s1x8},
   {{0, 2, 4, 7, 9, 11},            {1, 3, 5, 6, 8, 10},          {0}, {0},  0x555,    0xFFF,         6, 0,  0,  0, 0,  s1x6,  s1x12},
   {{0, 2, 6, 4, 9, 11, 15, 13},    {1, 3, 7, 5, 8, 10, 14, 12},  {0}, {0}, 0x5555,   0xFFFF,         8, 0,  0,  0, 0,  s1x8,  s1x16},
   {{15, 14, 12, 13, 8, 9, 11, 10}, {0, 1, 3, 2, 7, 6, 4, 5},     {0}, {0},      0,     0000,         8, 0,  0,  0, 0,  s1x8,  s2x8},
   {{11, 10, 9, 6, 7, 8},           {0, 1, 2, 5, 4, 3},           {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s1x6,  s2x6},
   {{10, 15, 3, 1, 4, 5, 6, 8},     {12, 13, 14, 0, 2, 7, 11, 9}, {0}, {0},      0,     0000,         8, 0,  0,  0, 0,  s2x4,  s4x4},
   {{14, 3, 7, 5, 8, 9, 10, 12},    {0, 1, 2, 4, 6, 11, 15, 13},  {0}, {0},      0,   0xFFFF,         8, 1,  0,  0, 0,  s2x4,  s4x4},
   /* When the map following this one gets fixed and uncommented, this one will have to appear first, of course.  Actually, it's
      trickier than that.  The whole issue of 3x4 vs. qtag operation needs to be straightened out. */
   {{7, 22, 15, 20, 18, 11, 2, 9},  {6, 23, 14, 21, 19, 10, 3, 8},{0}, {0},      0, 0xFCCFCC,         8, 1,  0,  0, 0,  s_qtag,s4x6},
   {{11, 10, 9, 8, 7, 6, 12, 13, 14, 15, 16, 17},
                     {0, 1, 2, 3, 4, 5, 23, 22, 21, 20, 19, 18},  {0}, {0},      0,     0000,        12, 0,  0,  0, 0,  s2x6,  s4x6},
   {{2, 5, 7, 9, 10, 0},            {3, 4, 6, 8, 11, 1},          {0}, {0},      0,   0x0FFF,         6, 1,  0,  0, 0,  s2x3,  s3x4},
   {{2, 5, 7, 8, 11, 0},            {-1, -1, 6, -1, -1, 1},       {0}, {0},      0,   0x00C3,         6, 1,  0,  0, 0,  s2x3,  s3x4},
   {{2, 5, 7, 9, 11, 1},            {3, -1, -1, 8, -1, -1},       {0}, {0},      0,   0x030C,         6, 1,  0,  0, 0,  s2x3,  s3x4},
   {{0, 2, 4, 6, 9, 11, 13, 15},    {1, 3, 5, 7, 8, 10, 12, 14},  {0}, {0}, 0x5555,   0xFFFF,         8, 0,  0,  0, 0,  s2x4,  s2x8},
   {{0, 2, 4, 6, 9, 11, 13, 15, 17, 19, 20, 22},
                     {1, 3, 5, 7, 8, 10, 12, 14, 16, 18, 21, 23}, {0}, {0}, 0x555555, 0xFFFFFF,      12, 0,  0,  0, 0,  s3x4,  s3x8},
   {{2, 3, 5, 6, 7, 0},             {-1, -1, 4, -1, -1, 1},       {0}, {0},      0,     0x33,         6, 1,  0,  0, 0,  s_2x1dmd, s_crosswave},
   {{0, 3, 2, 5, 7, 6},             {1, -1, -1, 4, -1, -1},       {0}, {0},      0,     0x33,         6, 1,  0,  0, 0,  s_2x1dmd, s_hrglass},
   {{0, 2, 3, 5, 6, 7},             {1, -1, -1, 4, -1, -1},       {0}, {0},  0x041,     0x33,         6, 0,  0,  0, 0,  s_2x1dmd, s3x1dmd},
   {{0, 1, 3, 4, 6, 7},             {-1, 2, -1, -1, 5, -1},       {0}, {0},  0x104,     0x66,         6, 0,  0,  0, 0,  s_2x1dmd, s3x1dmd},
   {{6, 7, 0, 2, 3, 5},             {-1, -1, 1, -1, -1, 4},       {0}, {0},  0x410,     0x33,         6, 0,  0,  0, 0,  s_2x1dmd, s_qtag},
   {{2, 4, 5, 0},                   {-1, 3, -1, 1},               {0}, {0},      0,      033,         4, 1,  0,  0, 0,  sdmd, s_2x1dmd},
   {{0, 2, 4, 5},                   {1, -1, 3, -1},               {0}, {0},   0x11,      033,         4, 0,  0,  0, 0,  sdmd, s_1x2dmd},
   /* Next one is for centers in tandem in lines, making a virtual bone6. */
   {{0, 3, 5, 4, 7, 6},             {-1, -1, 2, -1, -1, 1},       {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s_bone6, s2x4},
   /* Next two are for certain ends in tandem in an H, making a virtual bone6. */
   {{10, 3, 5, 6, 9, 11},           {0, -1, -1, 4, -1, -1},       {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s_bone6, s3x4},
   {{0, 4, 5, 6, 9, 11},            {-1, 3, -1, -1, 10, -1},      {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s_bone6, s3x4},
   /* Next one is for ends in tandem in lines, making a virtual short6. */
   {{2, 4, 5, 6, 7, 1},             {-1, 3, -1, -1, 0, -1},       {0}, {0},  0x104,     0000,         6, 1,  0,  0, 0,  s_short6, s2x4},
   /* Next two are for certain center column people in tandem in a 1/4 tag, making a virtual short6. */
   {{3, 2, 4, 5, 6, 0},             {1, -1, -1, 7, -1, -1},       {0}, {0},  0x041,     0000,         6, 1,  0,  0, 0,  s_short6, s_qtag},
   {{1, 2, 4, 5, 6, 7},             {-1, -1, 3, -1, -1, 0},       {0}, {0},  0x410,     0000,         6, 1,  0,  0, 0,  s_short6, s_qtag},



/* new */
   /* Next two are for certain center column people in tandem in a spindle, making a virtual short6. */
   {{2, 3, 5, 6, 7, 0},             {-1, -1, 4, -1, -1, 1},       {0}, {0},      0,     0x33,         6, 1,  0,  0, 0,  s_short6, s_spindle},
   {{1, 3, 4, 6, 7, 0},             {2, -1, -1, 5, -1, -1},       {0}, {0},      0,     0x66,         6, 1,  0,  0, 0,  s_short6, s_spindle},


   /* Next three are for various people in tandem in columns of 8, making virtual columns of 6. */
   {{0, 2, 3, 5, 6, 7},             {1, -1, -1, 4, -1, -1},       {0}, {0},  0x041,     0063,         6, 0,  0,  0, 0,  s2x3,  s2x4},
   {{0, 1, 3, 4, 6, 7},             {-1, 2, -1, -1, 5, -1},       {0}, {0},  0x104,     0x66,         6, 0,  0,  0, 0,  s2x3,  s2x4},
   {{0, 1, 2, 4, 5, 7},             {-1, -1, 3, -1, -1, 6},       {0}, {0},  0x410,     0xCC,         6, 0,  0,  0, 0,  s2x3,  s2x4},
   /* Next three are for various people as couples in a C1 phantom or 1/4 tag, making virtual columns of 6. */
   {{3, 7, 5, 9, 15, 13},           {1, -1, -1, 11, -1, -1},      {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s2x3,  s_c1phan},
   {{1, 3, 4, 5, 6, 0},             {-1, 2, -1, -1, 7, -1},       {0}, {0},      0,     0xCC,         6, 1,  0,  0, 0,  s2x3,  s_qtag},
   {{0, 2, 6, 8, 10, 12},           {-1, -1, 4, -1, -1, 14},      {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s2x3,  s_c1phan},
   /* Next three are for various people in tandem in a rigger or PTP diamonds, making a virtual line of 6. */
   {{6, 7, 5, 2, 3, 4},             {-1, -1, 0, -1, -1, 1},       {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s1x6,  s_rigger},
   {{0, 3, 2, 4, 5, 6},             {-1, 1, -1, -1, 7, -1},       {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s1x6,  s_ptpd},
   {{5, 6, 7, 4, 2, 3},             {0, -1, -1, 1, -1, -1},       {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s1x6,  s_bone},
   {{0, 2, 5, 7, 9, 11, 12, 14},    {1, 3, 4, 6, 8, 10, 13, 15},  {0}, {0}, 0x5555,   0xFFFF,         8, 0,  0,  0, 0,  s_qtag,s4dmd},
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                 {0}, {0},      0,     0xFF,         4, 1,  0,  0, 0,  sdmd,  s_qtag},
   {{0, 7, 2, 4, 5, 6},             {-1, 1, -1, -1, 3, -1},       {0}, {0},      0,     0000,         6, 0,  0,  0, 0,  s_2x1dmd, s_galaxy},
   {{2, 1, 4, 6, 7, 0},             {-1, 3, -1, -1, 5, -1},       {0}, {0},      0,     0xAA,         6, 1,  0,  0, 0,  s_2x1dmd, s_galaxy},
   {{3, 7, 9, 13},                  {1, 5, 11, 15},               {0}, {0},   0x44,   0xA0A0,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0, 6, 10, 12},                 {2, 4, 8, 14},                {0}, {0},   0x11,   0x0505,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},

   {{0, 7, 9, 12},                  {2, 5, 11, 14},               {0}, {0},   0x05,   0x00A5,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},     /* these 4 are unsymmetrical */
   {{3, 6, 10, 13},                 {1, 4, 8, 15},                {0}, {0},   0x50,   0xA500,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{3, 7, 10, 12},                 {1, 5, 8, 14},                {0}, {0},   0x14,   0x05A0,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0, 6, 9, 13},                  {2, 4, 11, 15},               {0}, {0},   0x41,   0xA005,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},

   {{15, 3, 5, 9},                  {13, 1, 7, 11},               {0}, {0},   0x44,   0x0A0A,         4, 0,  0,  0, 0,  s2x2,  s4x4},         /* These do C1-phantom-like stuff from fudgy 4x4 */
   {{10, 3, 7, 6},                  {15, 14, 2, 11},              {0}, {0},   0x11,   0x8484,         4, 0,  0,  0, 0,  s2x2,  s4x4},         /* They must follow the pair just above. */
   {{1, 3, 4, 5, 6, 0},             {-1, 2, -1, -1, 7, -1},       {0}, {0},      0,     0xCC,         6, 1,  0,  0, 0,  s_short6, s_rigger},
   {{6, 0, 3, 5},                   {7, 1, 2, 4},                 {0}, {0},   0x55,     0xFF,         4, 0,  0,  0, 0,  sdmd,  s_rigger},
   {{6, 5, 3, 4},                   {7, 0, 2, 1},                 {0}, {0},   0x11,     0xCC,         4, 0,  0,  0, 0,  s1x4,  s_rigger},     /* must be after "2x4_4" */
   {{5, 6, 4, 3},                   {0, 7, 1, 2},                 {0}, {0},   0x44,     0xCC,         4, 0,  0,  0, 0,  s1x4,  s_bone},
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                 {0}, {0},   0x11,     0063,         4, 0,  0,  0, 0,  sdmd,  s_crosswave},  /* crosswave*/  
   {{7, 1, 4, 6},                   {0, 2, 3, 5},                 {0}, {0},   0x44,     0146,         4, 0,  0,  0, 0,  sdmd,  s2x4},         /* must be after "2x4_4" */
   /* Next one is for so-and-so in tandem in a bone6, making a virtual line of 4. */
   {{4, 5, 3, 2},                   {0, -1, 1, -1},               {0}, {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s_bone6},
   /* Next one is for so-and-so in tandem in a short6, making a virtual line of 4. */
   {{1, 0, 4, 5},                   {-1, 2, -1, 3},               {0}, {0},      0,     0055,         4, 1,  0,  0, 0,  s1x4,  s_short6},
   {{1, 3, 4, 5},                   {-1, 2, -1, 0},               {0}, {0},   0x44,     0000,         4, 1,  0,  0, 0,  sdmd,  s_short6},
   {{5, 1, 3, 4},                   {0, -1, 2, -1},               {0}, {0},      0,     0000,         4, 0,  0,  0, 0,  sdmd,  s2x3},
   /* Next three are for so-and-so as couples in a line of 8, making a virtual line of 6. */
   {{0, 1, 3, 4, 5, 6},             {-1, -1, 2, -1, -1, 7},       {0}, {0},  0x410,     0xCC,         6, 0,  0,  0, 0,  s1x6,  s1x8},
   {{0, 1, 2, 4, 7, 6},             {-1, 3, -1, -1, 5, -1},       {0}, {0},  0x104,     0xAA,         6, 0,  0,  0, 0,  s1x6,  s1x8},
   {{0, 3, 2, 5, 7, 6},             {1, -1, -1, 4, -1, -1},       {0}, {0},  0x041,     0x33,         6, 0,  0,  0, 0,  s1x6,  s1x8},
   /* Next two are for so-and-so as couples in a line of 6, making a virtual line of 4. */
   {{0, 1, 3, 5},                   {-1, 2, -1, 4},               {0}, {0},   0x44,     0066,         4, 0,  0,  0, 0,  s1x4,  s1x6},
   {{0, 2, 4, 5},                   {1, -1, 3, -1},               {0}, {0},   0x11,     0033,         4, 0,  0,  0, 0,  s1x4,  s1x6},
   /* Next 4 are for so-and-so in tandem from a column of 6, making a virtual column of 4.  The first two are the real maps,
      and the other two take care of the reorientation that sometimes happens when coming out of a 2x2. */
   {{0, 1, 3, 5},                   {-1, 2, -1, 4},               {0}, {0},   0x44,     0066,         4, 0,  0,  0, 0,  s2x2,  s2x3},
   {{0, 2, 4, 5},                   {1, -1, 3, -1},               {0}, {0},   0x11,     0033,         4, 0,  0,  0, 0,  s2x2,  s2x3},
   {{1, 3, 5, 0},                   {2, -1, 4, -1},               {0}, {0},      0,     0066,         4, 1,  0,  0, 0,  s2x2,  s2x3},
   {{2, 4, 5, 0},                   {-1, 3, -1, 1},               {0}, {0},      0,     0033,         4, 1,  0,  0, 0,  s2x2,  s2x3},
   {{0, 2, 4, 7, 9, 11},            {1, 3, 5, 6, 8, 10},          {0}, {0},  0x555,   0x0FFF,         6, 0,  0,  0, 0,  s2x3,  s2x6},
   {{2, 0},                         {3, 1},                       {0}, {0},    0x4,      0xC,         2, 1,  0,  0, 0,  s1x2,  s_trngl4},
   {{1, 3},                         {0, 2},                       {0}, {0},    0x1,      0xC,         2, 3,  0,  0, 0,  s1x2,  s_trngl4},
   {{2, 1, 0},                      {3, -1, -1},                  {0}, {0},      0,      0xC,         3, 1,  0,  0, 0,  s1x3,  s_trngl4},
   {{0, 1, 3},                      {-1, -1, 2},                  {0}, {0},      0,      0xC,         3, 3,  0,  0, 0,  s1x3,  s_trngl4},
   {{0, 3, 2},                      {-1, 1, -1},                  {0}, {0},      0,     0000,         3, 0,  0,  0, 0,  s1x3,  sdmd},
   {{0},                            {0},                          {0}, {0},      0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};


Private tm_thing maps_isearch_threesome[] = {

/*   map1                  map2                  map3                 map4    ilatmask olatmask    limit rot            insetup outsetup */
   {{0, 5},               {1, 4},               {2, 3},                {0},    0x5,      077,         2, 0,  0,  0, 0,  s1x2,  s1x6},
   {{0, 5},               {1, 4},               {2, 3},                {0},      0,      077,         2, 1,  0,  0, 0,  s1x2,  s2x3},
   {{0, 3, 8, 11},        {1, 4, 7, 10},        {2, 5, 6, 9},          {0},   0x55,    07777,         4, 0,  0,  0, 0,  s2x2,  s2x6},
   {{3, 8, 11, 0},        {4, 7, 10, 1},        {5, 6, 9, 2},          {0},      0,    07777,         4, 1,  0,  0, 0,  s2x2,  s2x6},
   {{0, 3, 6, 7},         {1, -1, 5, -1},       {2, -1, 4, -1},        {0},   0x11,     0x77,         4, 0,  0,  0, 0,  sdmd,  s1x3dmd},
   {{3, 6, 7, 0},         {-1, 5, -1, 1},       {-1, 4, -1, 2},        {0},      0,     0x77,         4, 1,  0,  0, 0,  sdmd,  s3x1dmd},
   {{7, 0, 3, 6},         {-1, 1, -1, 5},       {-1, 2, -1, 4},        {0},   0x44,     0x77,         4, 0,  0,  0, 0,  sdmd,  s_spindle},
   {{0, 5, 8, 11},        {1, -1, 7, -1},       {2, -1, 6, -1},        {0},      0,     0707,         4, 1,  0,  0, 0,  sdmd,  s3dmd},
   {{0, 3, 8, 11},        {1, 4, 7, 10},        {2, 5, 6, 9},          {0},   0x55,    07777,         4, 0,  0,  0, 0,  s1x4,  s1x12},
   {{0, 2, 7, 6},         {1, -1, 5, -1},       {3, -1, 4, -1},        {0},   0x11,     0xBB,         4, 0,  0,  0, 0,  s1x4,  s1x8},
   {{0, 1, 4, 6},         {-1, 3, -1, 7},       {-1, 2, -1, 5},        {0},   0x44,     0xEE,         4, 0,  0,  0, 0,  s1x4,  s1x8},
   {{3, 8, 21, 14, 17, 18, 11, 0},       {4, 7, 22, 13, 16, 19, 10, 1},
                                         {5, 6, 23, 12, 15, 20, 9, 2}, {0},      0,0x0FFFFFF,         8, 1,  0,  0, 0,  s2x4,  s4x6},
   {{19, 18, 16, 17, 12, 13, 15, 14},    {20, 21, 23, 22, 8, 9, 11, 10},
                                         {0, 1, 3, 2, 7, 6, 4, 5},     {0},      0,     0000,         8, 0,  0,  0, 0,  s1x8,  s3x8},
   {{9, 8, 6, 7},         {10, 11, 4, 5},       {0, 1, 3, 2},          {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s3x4},
   {{9, 11, 6, 5},        {10, -1, 4, -1},      {0, -1, 3, -1},        {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s3x4},
   {{6, 5, 2, 4},         {-1, 7, -1, 3},       {-1, 0, -1, 1},        {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s_qtag},
   {{0},                            {0},                          {0}, {0},      0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};

Private tm_thing maps_isearch_foursome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{0, 6},           {1, 7},           {3, 5},           {2, 4},              0x5,    0x0FF,         2, 0,  0,  0, 0,  s1x2,  s1x8},
   {{0, 7},           {1, 6},           {2, 5},           {3, 4},                0,    0x0FF,         2, 1,  0,  0, 0,  s1x2,  s2x4},
   {{0, 4, 11, 15},   {1, 5, 10, 14},   {2, 6, 9, 13},    {3, 7, 8, 12},      0x55,  0x0FFFF,         4, 0,  0,  0, 0,  s2x2,  s2x8},
   {{4, 11, 15, 0},   {5, 10, 14, 1},   {6, 9, 13, 2},    {7, 8, 12, 3},         0,  0x0FFFF,         4, 1,  0,  0, 0,  s2x2,  s2x8},
   {{0, 4, 11, 15},   {1, 5, 10, 14},   {2, 6, 9, 13},    {3, 7, 8, 12},      0x55,  0x0FFFF,         4, 0,  0,  0, 0,  s1x4,  s1x16},
   {{17, 16, 15, 12, 13, 14},       {18, 19, 20, 23, 22, 21},
        {11, 10, 9, 6, 7, 8},               {0, 1, 2, 5, 4, 3},                  0,     0000,         6, 0,  0,  0, 0,  s1x6,  s4x6},
   {{8, 6, 4, 5},     {9, 11, 2, 7},    {10, 15, 1, 3},   {12, 13, 0, 14},       0,     0000,         4, 0,  0,  0, 0,  s1x4,  s4x4},
   {{12, 10, 8, 9},   {13, 15, 6, 11},  {14, 3, 5, 7},    {0, 1, 4, 2},          0,   0xFFFF,         4, 1,  0,  0, 0,  s1x4,  s4x4},
   {{0},                            {0},                          {0}, {0},      0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};

Private tm_thing maps_isearch_boxsome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{0},              {1},              {3},              {2},                   1,      0xF,         1, 0,  0,  0, 0,  s1x1,  s2x2},
   {{3},              {0},              {2},              {1},                   0,        0,         1, 0,  0,  0, 0,  s1x1,  s2x2},
   {{0, 2},           {1, 3},           {7, 5},           {6, 4},              0x5,     0xFF,         2, 0,  0,  0, 0,  s1x2,  s2x4},
   {{7, 5},           {0, 2},           {6, 4},           {1, 3},                0,        0,         2, 0,  0,  0, 0,  s1x2,  s2x4},
   {{0, 2, 6, 4},     {1, 3, 7, 5},     {15, 13, 9, 11},  {14, 12, 8, 10},    0x55,   0xFFFF,         4, 0,  0,  0, 0,  s1x4,  s2x8},
   {{15, 13, 9, 11},  {0, 2, 6, 4},     {14, 12, 8, 10},  {1, 3, 7, 5},          0,        0,         4, 0,  0,  0, 0,  s1x4,  s2x8},
   {{12, 14, 7, 9},   {13, 0, 2, 11},   {10, 3, 5, 8},    {15, 1, 4, 6},      0x55,   0xFFFF,         4, 0,  0,  0, 0,  s2x2,  s4x4},
   {{10, 3, 5, 8},    {12, 14, 7, 9},   {15, 1, 4, 6},    {13, 0, 2, 11},        0,        0,         4, 0,  0,  0, 0,  s2x2,  s4x4},
   {{11, 2, 7, 20},   {10, 3, 6, 21},   {18, 9, 22, 15},  {19, 8, 23, 14},    0x55,0x0FCCFCC,         4, 0,  0,  0, 0,  sdmd,  s4x6},
   {{18, 9, 22, 15},  {11, 2, 7, 20},   {19, 8, 23, 14},  {10, 3, 6, 21},        0,        0,         4, 0,  0,  0, 0,  sdmd,  s4x6},
   {{0},              {0},              {0},              {0},                   0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};

Private tm_thing maps_isearch_dmdsome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{0},              {1},              {3},              {2},                   1,      0xF,         1, 0,  0,  0, 0,  s1x1,  sdmd},
   {{0},              {1},              {3},              {2},                   0,      0xF,         1, 1,  0,  0, 0,  s1x1,  sdmd},
   {{0, 6},           {1, 7},           {3, 5},           {2, 4},              0x5,     0xFF,         2, 0,  0,  0, 0,  s1x2,  s_ptpd},
   {{5, 4},           {6, 3},           {7, 2},           {0, 1},                0,        0,         2, 0,  0,  0, 0,  s1x2,  s_qtag},
   {{11, 10, 8, 9},   {12, 14, 5, 7},   {13, 15, 4, 6},   {0, 1, 3, 2},          0,        0,         4, 0,  0,  0, 0,  s1x4,  s4dmd},
   {{0},              {0},              {0},              {0},                   0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};


Private tm_thing maps_isearch_tglsome[] = {

/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{6, 0, 2, 4},     {-1, 7, -1, 3},   {-1, 5, -1, 1},   {0},                0xC4,     0xBB,         4, 0,  0,  0, 0,  s1x4,  s_rigger},
   {{1, 2, 5, 6},     {0, -1, 4, -1},   {3, -1, 7, -1},   {0},                0x31,     0xBB,         4, 0,  0,  0, 0,  s1x4,  s_ptpd},
   {{0, 3, 4, 7},     {-1, 2, -1, 6},   {-1, 1, -1, 5},   {0},                0x4C,     0xEE,         4, 0,  0,  0, 0,  s1x4,  s_ptpd},

   {{7, 1, 3, 5},     {0, -1, 4, -1},   {6, -1, 2, -1},   {0},                0x20,        0,         4, 0,  0,  0, 0,  s2x2,  s_qtag},
   {{0, 2, 4, 6},     {-1, 1, -1, 5},   {-1, 3, -1, 7},   {0},                0x80,        0,         4, 0,  0,  0, 0,  s2x2,  s_qtag},

   {{1, 3, 5, 7},     {-1, 4, -1, 0},   {-1, 2, -1, 6},   {0},                0x4C,        0,         4, 1,  0,  0, 0,  s2x2,  s_qtag},
   {{2, 4, 6, 0},     {1, -1, 5, -1},   {3, -1, 7, -1},   {0},                0x31,        0,         4, 1,  0,  0, 0,  s2x2,  s_qtag},

   {{5, 7, 1, 3},     {6, -1, 2, -1},   {0, -1, 4, -1},   {0},                0x13,     0x77,         4, 0,  0,  0, 0,  s1x4,  s_bone},
   {{0, 1, 4, 5},     {7, -1, 3, -1},   {6, -1, 2, -1},   {0},                0x31,     0xDD,         4, 0,  0,  0, 0,  sdmd,  s_spindle},
   {{5, 3, 1, 7},     {6, -1, 2, -1},   {0, -1, 4, -1},   {0},                0x13,     0x77,         4, 0,  0,  0, 0,  sdmd,  s_dhrglass},
   {{6, 0, 2, 4},     {-1, 3, -1, 7},   {-1, 1, -1, 5},   {0},                0x08,        0,         4, 0,  0,  0, 0,  sdmd,  s_hrglass},

   {{0, 3, 4, 7},     {-1, 2, -1, 6},   {-1, 1, -1, 5},   {0},                0x80,        0,         4, 0,  0,  0, 0,  sdmd,  s_galaxy},
   {{2, 5, 6, 1},     {-1, 4, -1, 0},   {-1, 3, -1, 7},   {0},                0x08,     0xBB,         4, 1,  0,  0, 0,  sdmd,  s_galaxy},

   {{1, 7, 9, 15},    {-1, 11, -1, 3},  {-1, 5, -1, 13},  {0},                0x08,   0x0000,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0, 4, 8, 12},    {-1, 2, -1, 10},  {-1, 6, -1, 14},  {0},                0xC4,   0x5454,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{3, 5, 11, 13},   {7, -1, 15, -1},  {1, -1, 9, -1},   {0},                0x13,   0x8A8A,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0, 4, 8, 12},    {14, -1, 6, -1},  {2, -1, 10, -1},  {0},                0x02,   0x0000,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0},              {0},              {0},              {0},                   0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};


Private tm_thing maps_isearch_3x1tglsome[] = {
/*   map1              map2              map3              map4               ilatmask olatmask    limit rot            insetup outsetup */
   {{9, 3},           {10, 4},          {0, 6},           {11, 5},             0x7,    07171,         2, 0,  0,  0, 0,  s1x2,  s3x4},
   {{0, 4},           {7, 3},           {5, 1},           {6, 2},              0xD,     0xFF,         2, 0,  0,  0, 0,  s1x2,  s_qtag},
   {{9, 3},           {10, 4},          {11, 5},          {1, 7},              0x8,     0000,         2, 0,  0,  0, 0,  s1x2,  s2x6},
   {{0, 6},           {1, 7},           {2, 8},           {10, 4},             0x2,     0000,         2, 0,  0,  0, 0,  s1x2,  s2x6},
   {{0},              {0},              {0},              {0},                   0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};


typedef struct {
   setup_kind testkind;
   uint32 testval;
   uint32 fixup;
   warning_index warning;
} siamese_item;

siamese_item siamese_table[] = {
   {s2x4,        0x00FF0000UL, 0x99UL,   warn__ctrstand_endscpls},
   {s2x4,        0x00990066UL, 0x99UL,   warn__ctrstand_endscpls},
   {s2x4,        0x000000FFUL, 0x66UL,   warn__ctrscpls_endstand},
   {s2x4,        0x00660099UL, 0x66UL,   warn__ctrscpls_endstand},
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
   {nothing,     0,            0,        warn__none}};


Private void initialize_one_table(tm_thing *map_start, int np)
{
   tm_thing *map_search;

   for (map_search = map_start; map_search->outsetup != nothing; map_search++) {
      int i;
      uint32 m;
      uint32 imask = 0;
      uint32 omask = 0;
      uint32 osidemask = 0;

      /* All 1's for people in outer setup. */
      uint32 alloutmask = (1 << (setup_attrs[map_search->outsetup].setup_limits+1))-1;

      for (i=0, m=1; i<map_search->limit; i++, m<<=2) {
         alloutmask &= ~(1 << map_search->map1[i]);
         if (map_search->map2[i] < 0) {
            imask |= m;
            omask |= 1 << map_search->map1[i];
         }
         else {
            alloutmask &= ~(1 << map_search->map2[i]);
            if (np >= 3)
               alloutmask &= ~(1 << map_search->map3[i]);
            if (np >= 4)
               alloutmask &= ~(1 << map_search->map4[i]);

            if (((map_search->ilatmask >> (i*2)) ^ map_search->rot) & 1) {
               osidemask |= 1 << map_search->map1[i];
               osidemask |= 1 << map_search->map2[i];
               if (np >= 3)
                  osidemask |= 1 << map_search->map3[i];
               if (np >= 4)
                  osidemask |= 1 << map_search->map4[i];
            }
         }
      }

      map_search->insinglemask = imask;
      map_search->outsinglemask = omask;
      map_search->outunusedmask = alloutmask;

      /* We can't encode the virtual person number in the required 3-bit field if this is > 8. */
      if (map_search->limit != setup_attrs[map_search->insetup].setup_limits+1) uims_database_error("Tandem table initialization failed: limit wrong.\n", (Cstring) 0);
      if (map_search->olatmask != osidemask) uims_database_error("Tandem table initialization failed: Smask.\n", (Cstring) 0);
   }
}


extern void initialize_tandem_tables(void)
{
   initialize_one_table(maps_isearch_twosome, 2);
   initialize_one_table(maps_isearch_threesome, 3);
   initialize_one_table(maps_isearch_foursome, 4);
   initialize_one_table(maps_isearch_boxsome, 4);
   initialize_one_table(maps_isearch_dmdsome, 4);
   initialize_one_table(maps_isearch_tglsome, 3);
   initialize_one_table(maps_isearch_3x1tglsome, 4);
}


Private void unpack_us(
   tm_thing *map_ptr,
   uint32 orbitmask,
   tandrec *tandstuff,
   setup *result)
{
   int i;
   uint32 m, o, r;

   result->kind = map_ptr->outsetup;
   result->rotation = tandstuff->virtual_result.rotation - map_ptr->rot;
   result->result_flags = tandstuff->virtual_result.result_flags;
   r = map_ptr->rot*011;

   for (i=0, m=map_ptr->insinglemask, o=orbitmask; i<map_ptr->limit; i++, m>>=2, o>>=2) {
      uint32 z = rotperson(tandstuff->virtual_result.people[i].id1, r);

      if (z != 0) {
         int ii = (z >> 6) & 7;

         if (m & 1) {
            /* Unpack single person. */

            personrec f = tandstuff->real_front_people[ii];
            if (f.id1) f.id1 = (f.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            result->people[map_ptr->map1[i]] = f;
         }
         else {
            /* Unpack tandem/couples person. */
   
            personrec f = tandstuff->real_front_people[ii];
            personrec b = tandstuff->real_back_people[ii];
            personrec b2 = tandstuff->real_second_people[ii];
            personrec b3 = tandstuff->real_third_people[ii];

            if (f.id1) f.id1 = (f.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b.id1) b.id1 = (b.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b2.id1) b2.id1 = (b2.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b3.id1) b3.id1 = (b3.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));

            if (((o + (map_ptr->rot&1) + 1) & 2) && !tandstuff->no_unit_symmetry) {
               if (tandstuff->np >= 4) {
                  result->people[map_ptr->map1[i]] = b3;
                  result->people[map_ptr->map2[i]] = b2;
                  result->people[map_ptr->map3[i]] = b;
                  result->people[map_ptr->map4[i]] = f;
               }
               else if (tandstuff->np >= 3) {
                  result->people[map_ptr->map1[i]] = b2;
                  result->people[map_ptr->map2[i]] = b;
                  result->people[map_ptr->map3[i]] = f;
               }
               else {
                  result->people[map_ptr->map1[i]] = b;
                  result->people[map_ptr->map2[i]] = f;
               }
            }
            else {
               result->people[map_ptr->map1[i]] = f;
               result->people[map_ptr->map2[i]] = b;
               if (tandstuff->np >= 3) result->people[map_ptr->map3[i]] = b2;
               if (tandstuff->np >= 4) result->people[map_ptr->map4[i]] = b3;
            }
         }
      }
   }

   canonicalize_rotation(result);
}




/* Lat = 0 means the people we collapsed, relative to the incoming geometry, were one
   behind the other.  Lat = 1 means they were lateral.  "Incoming geometry" does
   not include the incoming rotation field, since we ignore it.  We are not responsible
   for the rotation field of the incoming setup.

  The canonical storage of the real people, while we are doing the virtual call, is as follows:

   Real_front_people gets person on left (lat=1) near person (lat=0).
   Real_back_people gets person on right (lat=1) or far person (lat=0). */

Private void pack_us(
   personrec *s,
   tm_thing *map_ptr,
   int fraction,
   long_boolean twosome,
   int key,
   tandrec *tandstuff)
{
   int i;
   uint32 m, sgl;
   int virt_index = -1;

   tandstuff->virtual_setup.rotation = map_ptr->rot & 1;
   tandstuff->virtual_setup.kind = map_ptr->insetup;

   for (i=0, m=map_ptr->ilatmask, sgl=map_ptr->insinglemask; i<map_ptr->limit; i++, m>>=2, sgl>>=2) {
      personrec f, b, b2, b3;
      personrec *ptr = &tandstuff->virtual_setup.people[i];
      uint32 vp1, vp2;
      int vert = (1 + map_ptr->rot + m) & 3;

      f = s[map_ptr->map1[i]];

      if (!tandstuff->no_unit_symmetry) vert &= 1;

      if (sgl & 1) {
         vp1 = f.id1;
         vp2 = f.id2;
         b.id1 = ~0UL;
      }
      else {
         uint32 u1;

         b = s[map_ptr->map2[i]];
         b2 = b;
         b3 = b;
         if (tandstuff->np >= 3) b2 = s[map_ptr->map3[i]];
         if (tandstuff->np >= 4) b3 = s[map_ptr->map4[i]];

         if (key == 3) {
            /* If this is skew/skewsome, we require people paired in the appropriate way.
               This means [f, b3] must match, [b, b2] must match, and [f, b] must not match. */

            if ((((f.id1 ^ b3.id1) | (b.id1 ^ b2.id1) | (~(f.id1 ^ b.id1))) & BIT_PERSON))
               fail("Can't find skew people.");
         }
         else {
            /* If this is not skew/skewsome, we forbid a live person grouped with a phantom unless
               some phantom concept was used (either something like "phantom tandem" or some other
               phantom concept such as "split phantom lines"). */

            /* In fact, we forbid ANY PERSON AT ALL to be a phantom, even if paired with another phantom. */

            if (     !(tandstuff->virtual_setup.cmd.cmd_misc_flags & CMD_MISC__PHANTOMS) &&
                     !(f.id1 & b.id1 & b2.id1 & b3.id1 & BIT_PERSON)) {
               fail("Use \"phantom\" concept in front of this concept.");
            }
         }

         u1 = f.id1 | b.id1 | b2.id1 | b3.id1;

         if (u1) {
            if (twosome >= 2 && (u1 & STABLE_MASK))
               fail("Sorry, can't nest fractional stable/twosome.");

            vp1 = ~0UL;
            vp2 = ~0UL;

            /* Create the virtual person.  When both people are present, anding
               the real peoples' id2 bits gets the right bits.  For example,
               the virtual person will be a boy and able to do a tandem star thru
               if both real people were boys.  Remove the identity field (700 bits)
               from id1 and replace with a virtual person indicator.  Check that
               direction, roll, and stability parts of id1 are consistent. */

            /* Needless to say, this could use some cleaning up. */

            if (f.id1) {
               vp1 &= f.id1;
               vp2 &= f.id2;

               /* If they have different fractional stability states, just clear them -- they can't do it. */
               if ((f.id1 ^ u1) & STABLE_MASK) vp1 &= ~STABLE_MASK;
               /* If they have different roll states, just clear them -- they can't roll. */
               if ((f.id1 ^ u1) & ROLL_MASK) vp1 &= ~ROLL_MASK;
               /* Check that all real people face the same way. */
               if ((f.id1 ^ u1) & 077)
                  fail("People not facing same way for tandem or as couples.");
            }
            if (b.id1) {
               vp1 &= b.id1;
               vp2 &= b.id2;

               /* If they have different fractional stability states, just clear them -- they can't do it. */
               if ((b.id1 ^ u1) & STABLE_MASK) vp1 &= ~STABLE_MASK;
               /* If they have different roll states, just clear them -- they can't roll. */
               if ((b.id1 ^ u1) & ROLL_MASK) vp1 &= ~ROLL_MASK;
               /* Check that all real people face the same way. */
               if ((b.id1 ^ u1) & 077)
                  fail("People not facing same way for tandem or as couples.");
            }
            if (b2.id1) {
               vp1 &= b2.id1;
               vp2 &= b2.id2;

               /* If they have different fractional stability states, just clear them -- they can't do it. */
               if ((b2.id1 ^ u1) & STABLE_MASK) vp1 &= ~STABLE_MASK;
               /* If they have different roll states, just clear them -- they can't roll. */
               if ((b2.id1 ^ u1) & ROLL_MASK) vp1 &= ~ROLL_MASK;
               /* Check that all real people face the same way. */
               if ((b2.id1 ^ u1) & 077)
                  fail("People not facing same way for tandem or as couples.");
            }
            if (b3.id1) {
               vp1 &= b3.id1;
               vp2 &= b3.id2;

               /* If they have different fractional stability states, just clear them -- they can't do it. */
               if ((b3.id1 ^ u1) & STABLE_MASK) vp1 &= ~STABLE_MASK;
               /* If they have different roll states, just clear them -- they can't roll. */
               if ((b3.id1 ^ u1) & ROLL_MASK) vp1 &= ~ROLL_MASK;
               /* Check that all real people face the same way. */
               if ((b3.id1 ^ u1) & 077)
                  fail("People not facing same way for tandem or as couples.");
            }
         }
         else
            vp1 = 0;
      }

      if (vp1) {

         /* Create a virtual person number.  Only 8 are allowed, because of the tightness in the
            person representation.  That shouldn't be a problem, since each virtual person came
            from a group of real people that had at least one live person.  Unless active phantoms
            have been created, that is. */

         if ((++virt_index) >= 8) fail("Sorry, too many tandem or as couples people.");

         ptr->id1 = (vp1 & ~0700) | (virt_index << 6) | BIT_TANDVIRT;
         ptr->id2 = vp2;

         if (!(sgl & 1)) {
            if (twosome >= 2)
               ptr->id1 |= STABLE_ENAB | (STABLE_RBIT * fraction);

            tandstuff->vertical_people[virt_index] = vert;   /* 1 if original people were near/far; 0 if lateral */
         }

         if (map_ptr->rot & 1)   /* Compensate for setup rotation. */
            ptr->id1 = rotccw(ptr->id1);

         tandstuff->real_front_people[virt_index] = f;
         tandstuff->real_back_people[virt_index] = b;
         tandstuff->real_second_people[virt_index] = b2;
         tandstuff->real_third_people[virt_index] = b3;
         tandstuff->saved_originals[virt_index] = ptr->id1 + tandstuff->virtual_setup.rotation;
      }
      else {
         ptr->id1 = 0;
         ptr->id2 = 0;
      }
   }
}




extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,               /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,              /* number, if doing fractional twosome/solid */
   int phantom,               /* normal=0 phantom=1 general-gruesome=2 gruesome-with-wave-check=3 */
   int key,                   /* tandem = 0 / couples = 1 / siamese = 2 / skew = 3
                                 tandem of 3 = 4 / couples of 3 = 5 / tandem of 4 = 6 / couples of 4 = 7
                                 box = 10 / diamond = 11 / out point triangles = 20
                                 in point triangles = 21 / inside triangles = 22 / outside triangles = 23
                                 wave-based triangles triangles = 26 / tandem-based triangles = 27
                                 <anyone>-based triangles = 30 / 3x1 triangles = 31 */
   setup *result)
{
   selector_kind saved_selector;
   tandrec tandstuff;
   tm_thing *map;
   tm_thing *map_search;
   uint32 nsmask, ewmask, allmask, special_mask;
   int i, np;
   uint32 jbit;
   uint32 hmask, hmask2;
   uint32 orbitmask;
   uint32 sglmask, sglmask2;
   uint32 livemask, livemask2;
   long_boolean fractional = FALSE;
   long_boolean dead_conc = FALSE;
   tm_thing *our_map_table;

   special_mask = 0;
   tandstuff.single_mask = 0;
   tandstuff.no_unit_symmetry = FALSE;
   clear_people(result);


   if (key == 31) {
      np = 4;
      our_map_table = maps_isearch_3x1tglsome;
      tandstuff.no_unit_symmetry = TRUE;
   }
   else if (key >= 20) {
      np = 3;
      our_map_table = maps_isearch_tglsome;
      tandstuff.no_unit_symmetry = TRUE;
      if (key == 23)
         selector = selector_outer6;
      else if (key == 22)
         selector = selector_center6;
      else if (key == 26 || key == 27) {
         if (ss->kind == s_hrglass) {
            uint32 tbonetest = ss->people[0].id1 | ss->people[1].id1 | ss->people[4].id1 | ss->people[5].id1;

            if ((tbonetest & 011) == 011 || ((key ^ tbonetest) & 1))
               fail("Can't find the indicated triangles.");

            special_mask = 0x44;   /* The triangles have to be these. */
         }
         else if (ss->kind == s_galaxy) {
            uint32 tbonetest = ss->people[1].id1 | ss->people[3].id1 | ss->people[5].id1 | ss->people[7].id1;

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

            for (i=0; i<=16; i++) tbonetest |= ss->people[i].id1;

            if ((tbonetest & 010) == 0) t ^= 1;
            else if ((tbonetest & 1) != 0)
               fail("Can't find the indicated triangles.");

            if (t&1) special_mask = 0x2121;
            else     special_mask = 0x1212;
         }
         else
            fail("Can't find these triangles.");
      }
      else if ((key == 20 || key == 21) && ss->kind == s_qtag) {
         if (key == 21) {
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
      else if (key != 30 || ss->kind != s_c1phan)
         /* For <anyone>-based triangles in C1-phantom, we just use whatever selector was given. */
         fail("Can't find these triangles.");
   }
   else if (key == 11) {
      np = 4;
      our_map_table = maps_isearch_dmdsome;
   }
   else if (key == 10 || key == 3) {
      np = 4;
      our_map_table = maps_isearch_boxsome;
   }
   else if (key >= 6) {
      np = 4;
      our_map_table = maps_isearch_foursome;
   }
   else if (key >= 4) {
      np = 3;
      our_map_table = maps_isearch_threesome;
   }
   else {
      np = 2;
      our_map_table = maps_isearch_twosome;
   }

   if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do tandem/couples concept from this position.");

   /* We use the phantom indicator to forbid an already-distorted setup.
      The act of forgiving phantom pairing is based on the setting of the
      CMD_MISC__PHANTOMS bit in the incoming setup, not on the phantom indicator. */

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED) && (phantom != 0))
      fail("Can't specify phantom tandem/couples in virtual or distorted setup.");

   /* Find out who is selected, if this is a "so-and-so are tandem". */
   saved_selector = current_options.who;

   if (selector != selector_uninitialized)
      current_options.who = selector;

   nsmask = 0;
   ewmask = 0;
   allmask = 0;

   for (i=0, jbit=1; i<=setup_attrs[ss->kind].setup_limits; i++, jbit<<=1) {
      uint32 p = ss->people[i].id1;
      if (p) {
         allmask |= jbit;
         /* We allow a "special" mask to override the selector. */
         if (     (selector != selector_uninitialized && !selectp(ss, i)) ||
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
      result->result_flags = 0;
      result->kind = nothing;
      return;
   }

   if (twosome >= 2) fractional = TRUE;

   if (fractional && fraction > 4)
      fail("Can't do fractional twosome more than 4/4.");

   if (key == 2) {
      /* Siamese. */
      uint32 j;

      siamese_item *ptr;

      for (ptr = siamese_table; ptr->testkind != nothing; ptr++) {
         if (ptr->testkind == ss->kind && ((((ewmask << 16) | nsmask) ^ ptr->testval) & ((allmask << 16) | allmask)) == 0) {
            warn(ptr->warning);
            j = ptr->fixup;
            goto foox;
         }
      }

      fail("Can't do Siamese in this setup.");

      foox:
      ewmask ^= (j & allmask);
      nsmask ^= (j & allmask);
   }
   else if (key == 10 || key == 3) {
      ewmask = allmask;
      nsmask = 0;
   }
   else if (key == 11) {
      if (ss->kind == s_ptpd || ss->kind == sdmd) {
         ewmask = allmask;
         nsmask = 0;
      }
      else {
         ewmask = 0;
         nsmask = allmask;
      }
   }
   else if (tandstuff.no_unit_symmetry) {
      if (key == 30) {
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
      else if (key == 31) {
         if (ss->kind == s2x6) {
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
      else if (special_mask == 0x44 || special_mask == 0x1212 || special_mask == 0x22 || special_mask == 0x11) {
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
   else if (key & 1) {
      /* Couples -- swap masks.  Tandem -- do nothing. */
      uint32 j = ewmask;
      ewmask = nsmask;
      nsmask = j;
   }

   /* Now ewmask and nsmask have the info about who is paired with whom. */
   ewmask &= ~tandstuff.single_mask;         /* Clear out unpaired people. */
   nsmask &= ~tandstuff.single_mask;

   map_search = our_map_table;
   while (map_search->outsetup != nothing) {
      if ((map_search->outsetup == ss->kind) &&
            (map_search->outsinglemask == tandstuff.single_mask) &&
            (!(allmask & map_search->outunusedmask)) &&
            (!(ewmask & (~map_search->olatmask))) &&
            (!(nsmask & map_search->olatmask))) {
         map = map_search;
         goto fooy;
      }
      map_search++;
   }
   fail("Can't do this tandem or couples call in this setup or with these people selected.");

   fooy:

   /* We also use the subtle aspects of the phantom indicator to tell what kind
      of setup we allow, and whether pairings must be parallel to the long axis. */

   if (phantom == 1) {
      if (ss->kind != s2x8 && ss->kind != s4x4 && ss->kind != s3x4 && ss->kind != s2x6 &&
               ss->kind != s4x6 && ss->kind != s1x12 && ss->kind != s1x16 &&
               ss->kind != s3dmd && ss->kind != s4dmd && ss->kind != s3x8)
         fail("Can't do couples or tandem concepts in this setup.");
   }
   else if (phantom >= 2) {
      if (ss->kind != s2x8 || map->insetup != s2x4)
         fail("Can't do gruesome concept in this setup.");
   }

   tandstuff.np = np;
   tandstuff.virtual_setup.cmd = ss->cmd;

   if (ss->cmd.cmd_assume.assumption == cr_wave_only && key == 1)
      fail("Couples or tandem concept is inconsistent with phantom facing direction.");
   else if (ss->cmd.cmd_assume.assumption == cr_jright || ss->cmd.cmd_assume.assumption == cr_jleft)
      fail("Couples or tandem concept is inconsistent with phantom facing direction.");

   tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_none;

   /* There are a small number of assumptions that we can transform. */

   if (ss->cmd.cmd_assume.assump_col == 4) {
      if (ss->cmd.cmd_assume.assumption == cr_ijright)
         tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_jright;
      else if (ss->cmd.cmd_assume.assumption == cr_ijleft)
         tandstuff.virtual_setup.cmd.cmd_assume.assumption = cr_jleft;
   }

   tandstuff.virtual_setup.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   if (phantom == 3) tandstuff.virtual_setup.cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;
   pack_us(ss->people, map, fraction, twosome, key, &tandstuff);

   update_id_bits(&tandstuff.virtual_setup);

   impose_assumption_and_move(&tandstuff.virtual_setup, &tandstuff.virtual_result);

   /* If this is a concentric setup with dead outsides, we can still handle it.
      We have to remember to put dead outsides back when we are done, in order
      to make gluing illegal. */

   if (tandstuff.virtual_result.kind == s_dead_concentric) {
      dead_conc = TRUE;
      tandstuff.virtual_result.kind = tandstuff.virtual_result.inner.skind;
      tandstuff.virtual_result.rotation = tandstuff.virtual_result.inner.srotation;
   }

   if (setup_attrs[tandstuff.virtual_result.kind].setup_limits < 0)
      fail("Don't recognize ending position from this tandem or as couples call.");

   sglmask2 = 0;    /* Bits appear here in pairs!  Both are duplicates. */
   livemask2 = 0;   /* Bits appear here in pairs!  Both are duplicates. */
   orbitmask = 0;   /* Bits appear here in pairs! */

   /* Compute orbitmask, livemask2, and sglmask2.
      Since we are synthesizing bit masks, we scan in reverse order to make things easier. */

   for (i=setup_attrs[tandstuff.virtual_result.kind].setup_limits; i>=0; i--) {
      uint32 p = tandstuff.virtual_result.people[i].id1;
      sglmask2 <<= 2;
      livemask2 <<= 2;
      orbitmask <<= 2;

      if (p) {
         int vpi = (p >> 6) & 7;
         livemask2 |= 3;

         if (tandstuff.real_back_people[vpi].id1 == ~0UL) {
            sglmask2 |= 3;
         }
         else {
            uint32 orbit;

            if (fractional) {
               if (!(p & STABLE_ENAB))
                  fail("fractional twosome not supported for this call.");
            }

            orbit = p + tandstuff.virtual_result.rotation - tandstuff.saved_originals[vpi];

            if (twosome == 2) {
               orbit -= ((p & (STABLE_VBIT*3)) / STABLE_VBIT);
            }
            else if (twosome == 3) {
               orbit = ((p & (STABLE_VBIT*3)) / STABLE_VBIT);
            }
            else if (twosome == 1) {
               orbit = 0;
            }

            orbitmask |= ((orbit - tandstuff.virtual_result.rotation - tandstuff.vertical_people[vpi]) & 3);
         }

         if (fractional)
            tandstuff.virtual_result.people[i].id1 &= ~STABLE_MASK;
      }
   }

   sglmask = sglmask2 & 0x55555555UL;     /* Bits appear here in pairs!  Only low bit of each pair is used. */
   livemask = livemask2 & 0x55555555UL;   /* Bits appear here in pairs!  Only low bit of each pair is used. */
   hmask2 = ~orbitmask & livemask2 & ~sglmask2;

   /* Pick out only low bits for map search, and only bits of live paired people. */
   hmask = hmask2 & 0x55555555UL;
   if (tandstuff.no_unit_symmetry) {
      hmask = hmask2;
      livemask = livemask2;
   }

   map_search = our_map_table;
   while (map_search->outsetup != nothing) {
      if ((map_search->insetup == tandstuff.virtual_result.kind) &&
            (map_search->insinglemask == sglmask) &&
            ((map_search->ilatmask & livemask) == hmask)) {
         unpack_us(map_search, orbitmask, &tandstuff, result);
         reinstate_rotation(ss, result);

         if (dead_conc) {
            result->inner.skind = result->kind;
            result->inner.srotation = result->rotation;
            result->kind = s_dead_concentric;
         }

         return;
      }
      map_search++;
   }

   fail("Don't recognize ending position from this tandem or as couples call.");
}
