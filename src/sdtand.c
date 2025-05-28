/* SD -- square dance caller's helper.

    Copyright (C) 1990-1994  William B. Ackerman.

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
   setup virtual_setup;
   setup virtual_result;
   int vertical_people[MAX_PEOPLE];    /* 1 if original people were near/far; 0 if lateral */
   int twosomep[MAX_PEOPLE];           /* 0: solid / 1: twosome / 2: solid-to-twosome / 3: twosome-to-solid */
   unsigned int single_mask;
   int np;
} tandrec;

typedef struct {
   int map1[8];
   int map2[8];
   int map3[8];
   int map4[8];
   uint32 sidewaysmask;       /* lateral pairs in inside numbering -- only alternate bits used! */
   uint32 outsidemask;
   int limit;
   int rot;
   uint32 insinglemask;       /* relative to insetup numbering, those people that are NOT paired -- only alternate bits used! */
   uint32 outsinglemask;      /* relative to outsetup numbering, those people that are NOT paired */
   uint32 outunusedmask;
   setup_kind insetup;
   setup_kind outsetup;
} tm_thing;


Private tm_thing maps_isearch_twosome[] = {

/*         map1                              map2                map3  map4   sidemask outsidemask limit rot            insetup outsetup            old name */
   {{7, 6, 4, 5},                   {0, 1, 3, 2},                 {0}, {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s2x4},            /* "2x4_4" - see below */
   {{0, 2, 5, 7},                   {1, 3, 4, 6},                 {0}, {0},   0x55,     0xFF,         4, 0,  0,  0, 0,  s2x2,  s2x4},
   {{2, 5, 7, 0},                   {3, 4, 6, 1},                 {0}, {0},      0,     0xFF,         4, 1,  0,  0, 0,  s2x2,  s2x4},
   {{3, 2},                         {0, 1},                       {0}, {0},      0,     0000,         2, 0,  0,  0, 0,  s1x2,  s2x2},
   {{0, 3},                         {1, 2},                       {0}, {0},      0,     0017,         2, 1,  0,  0, 0,  s1x2,  s2x2},
   {{0, 3},                         {1, 2},                       {0}, {0},    0x5,     0017,         2, 0,  0,  0, 0,  s1x2,  s1x4},
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
/*    Can't do this yet because maps can have only 8 items each.
   {{11, 10, 9, 8, 7, 6, 12, 13, 14, 15, 16, 17},
                     {0, 1, 2, 3, 4, 5, 23, 22, 21, 20, 19, 18},  {0}, {0},      0,     0000,        12, 0,  0,  0, 0,  s2x6,  s4x6},
*/
   {{2, 5, 7, 9, 10, 0},            {3, 4, 6, 8, 11, 1},          {0}, {0},      0,   0x0FFF,         6, 1,  0,  0, 0,  s2x3,  s3x4},
   {{0, 2, 4, 6, 9, 11, 13, 15},    {1, 3, 5, 7, 8, 10, 12, 14},  {0}, {0}, 0x5555,   0xFFFF,         8, 0,  0,  0, 0,  s2x4,  s2x8},
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
   /* Next three are for various people in tandem in columns of 8, making virtual columns of 6. */
   {{0, 2, 3, 5, 6, 7},             {1, -1, -1, 4, -1, -1},       {0}, {0},  0x041,     0063,         6, 0,  0,  0, 0,  s2x3,  s2x4},
   {{0, 1, 3, 4, 6, 7},             {-1, 2, -1, -1, 5, -1},       {0}, {0},  0x104,     0146,         6, 0,  0,  0, 0,  s2x3,  s2x4},
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
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                 {0}, {0},      0,     0377,         4, 1,  0,  0, 0,  sdmd,  s_qtag},

   {{3, 7, 9, 13},                  {1, 5, 11, 15},               {0}, {0},   0x44,   0xA0A0,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0, 6, 10, 12},                 {2, 4, 8, 14},                {0}, {0},   0x11,   0x0505,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},

   {{0, 7, 9, 12},                  {2, 5, 11, 14},               {0}, {0},   0x05,   0x00A5,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},     /* these 4 are unsymmetrical */
   {{3, 6, 10, 13},                 {1, 4, 8, 15},                {0}, {0},   0x50,   0xA500,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{3, 7, 10, 12},                 {1, 5, 8, 14},                {0}, {0},   0x14,   0x05A0,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},
   {{0, 6, 9, 13},                  {2, 4, 11, 15},               {0}, {0},   0x41,   0xA005,         4, 0,  0,  0, 0,  s2x2,  s_c1phan},

   {{15, 3, 5, 9},                  {13, 1, 7, 11},               {0}, {0},   0x44,   0x0A0A,         4, 0,  0,  0, 0,  s2x2,  s4x4},         /* These do C1-phantom-like stuff from fudgy 4x4 */
   {{10, 3, 7, 6},                  {15, 14, 2, 11},              {0}, {0},   0x11,   0x8484,         4, 0,  0,  0, 0,  s2x2,  s4x4},         /* They must follow the pair just above. */
   {{6, 0, 3, 5},                   {7, 1, 2, 4},                 {0}, {0},   0x55,     0377,         4, 0,  0,  0, 0,  sdmd,  s_rigger},
   {{6, 5, 3, 4},                   {7, 0, 2, 1},                 {0}, {0},   0x11,     0xCC,         4, 0,  0,  0, 0,  s1x4,  s_rigger},     /* must be after "2x4_4" */
   {{5, 6, 4, 3},                   {0, 7, 1, 2},                 {0}, {0},   0x44,     0xCC,         4, 0,  0,  0, 0,  s1x4,  s_bone},
   {{0, 3, 5, 6},                   {1, 2, 4, 7},                 {0}, {0},   0x11,     0063,         4, 0,  0,  0, 0,  sdmd,  s_crosswave},  /* crosswave*/  
   {{7, 1, 4, 6},                   {0, 2, 3, 5},                 {0}, {0},   0x44,     0146,         4, 0,  0,  0, 0,  sdmd,  s2x4},         /* must be after "2x4_4" */
   /* Next one is for so-and-so in tandem in a bone6, making a virtual line of 4. */
   {{4, 5, 3, 2},                   {0, -1, 1, -1},               {0}, {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s_bone6},
   /* Next one is for so-and-so in tandem in a short6, making a virtual line of 4. */
   {{1, 0, 4, 5},                   {-1, 2, -1, 3},               {0}, {0},      0,     0055,         4, 1,  0,  0, 0,  s1x4,  s_short6},
   /* Next three are for so-and-so as couples in a line of 8, making a virtual line of 6. */
   {{0, 1, 3, 4, 5, 6},             {-1, -1, 2, -1, -1, 7},       {0}, {0},  0x410,     0xCC,         6, 0,  0,  0, 0,  s1x6,  s1x8},
   {{0, 1, 2, 4, 7, 6},             {-1, 3, -1, -1, 5, -1},       {0}, {0},  0x104,     0252,         6, 0,  0,  0, 0,  s1x6,  s1x8},
   {{0, 3, 2, 5, 7, 6},             {1, -1, -1, 4, -1, -1},       {0}, {0},  0x041,     0063,         6, 0,  0,  0, 0,  s1x6,  s1x8},
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

/*   map1                  map2                  map3                 map4    sidemask outsidemask limit rot            insetup outsetup */
   {{0, 5},               {1, 4},               {2, 3},                {0},    0x5,      077,         2, 0,  0,  0, 0,  s1x2,  s1x6},
   {{0, 5},               {1, 4},               {2, 3},                {0},      0,      077,         2, 1,  0,  0, 0,  s1x2,  s2x3},
   {{0, 3, 8, 11},        {1, 4, 7, 10},        {2, 5, 6, 9},          {0},   0x55,    07777,         4, 0,  0,  0, 0,  s2x2,  s2x6},
   {{3, 8, 11, 0},        {4, 7, 10, 1},        {5, 6, 9, 2},          {0},      0,    07777,         4, 1,  0,  0, 0,  s2x2,  s2x6},
   {{0, 3, 8, 11},        {1, 4, 7, 10},        {2, 5, 6, 9},          {0},    0x55,   07777,         4, 0,  0,  0, 0,  s1x4,  s1x12},
   {{3, 8, 21, 14, 17, 18, 11, 0},       {4, 7, 22, 13, 16, 19, 10, 1},
                                         {5, 6, 23, 12, 15, 20, 9, 2}, {0},      0,0x0FFFFFF,         8, 1,  0,  0, 0,  s2x4,  s4x6},
   {{9, 8, 6, 7},         {10, 11, 4, 5},       {0, 1, 3, 2},          {0},      0,     0000,         4, 0,  0,  0, 0,  s1x4,  s3x4},
   {{0},                            {0},                          {0}, {0},      0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};

Private tm_thing maps_isearch_foursome[] = {

/*   map1              map2              map3              map4               sidemask outsidemask limit rot            insetup outsetup */
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

/*   map1              map2              map3              map4               sidemask outsidemask limit rot            insetup outsetup */
   {{0, 2},           {1, 3},           {7, 5},           {6, 4},              0x5,    0x0FF,         2, 0,  0,  0, 0,  s1x2,  s2x4},
   {{7, 5},           {0, 2},           {6, 4},           {1, 3},                0,        0,         2, 0,  0,  0, 0,  s1x2,  s2x4},
   {{0, 2, 6, 4},     {1, 3, 7, 5},     {15, 13, 9, 11},  {14, 12, 8, 10},    0x55,  0x0FFFF,         4, 0,  0,  0, 0,  s1x4,  s2x8},
   {{15, 13, 9, 11},  {0, 2, 6, 4},     {14, 12, 8, 10},  {1, 3, 7, 5},          0,        0,         4, 0,  0,  0, 0,  s1x4,  s2x8},
   {{12, 14, 7, 9},   {13, 0, 2, 11},   {10, 3, 5, 8},    {15, 1, 4, 6},      0x55,  0x0FFFF,         4, 0,  0,  0, 0,  s2x2,  s4x4},
   {{10, 3, 5, 8},    {12, 14, 7, 9},   {15, 1, 4, 6},   {13, 0, 2, 11},         0,        0,         4, 0,  0,  0, 0,  s2x2,  s4x4},
   {{0},              {0},              {0},              {0},                   0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};

Private tm_thing maps_isearch_dmdsome[] = {

/*   map1              map2              map3              map4               sidemask outsidemask limit rot            insetup outsetup */
   {{0, 6},           {1, 7},           {3, 5},           {2, 4},              0x5,    0x0FF,         2, 0,  0,  0, 0,  s1x2,  s_ptpd},
   {{5, 4},           {6, 3},           {7, 2},           {0, 1},                0,        0,         2, 0,  0,  0, 0,  s1x2,  s_qtag},
   {{11, 10, 8, 9},   {12, 14, 5, 7},   {13, 15, 4, 6},   {0, 1, 3, 2},          0,        0,         4, 0,  0,  0, 0,  s1x4,  s4dmd},
   {{0},              {0},              {0},              {0},                   0,     0000,         0, 0,  0,  0, 0,  nothing,  nothing}};


typedef struct {
   setup_kind testkind;
   uint32 testval;
   uint32 fixup;
   warning_index warning;
} siamese_item;

siamese_item siamese_table[] = {
   {s2x4,        0x00FF0000, 0x99,   warn__ctrstand_endscpls},
   {s2x4,        0x00990066, 0x99,   warn__ctrstand_endscpls},
   {s2x4,        0x000000FF, 0x66,   warn__ctrscpls_endstand},
   {s2x4,        0x00660099, 0x66,   warn__ctrscpls_endstand},
   {s2x4,        0x003300CC, 0xCC,   warn__none},
   {s2x4,        0x00CC0033, 0x33,   warn__none},
   {s_trngl4,    0x000F0000, 0x03,   warn__none},
   {s_trngl4,    0x0000000F, 0x0C,   warn__none},
   {s_c1phan,    0x0000AAAA, 0xA0A0, warn__none},
   {s_c1phan,    0x00005555, 0x0505, warn__none},
   {s_c1phan,    0xAAAA0000, 0x0A0A, warn__none},
   {s_c1phan,    0x55550000, 0x5050, warn__none},
   {s_c1phan,    0x00005AA5, 0x00A5, warn__none},  /* These 8 are unsymmetrical. */
   {s_c1phan,    0x0000A55A, 0xA500, warn__none},
   {s_c1phan,    0x000055AA, 0x05A0, warn__none},
   {s_c1phan,    0x0000AA55, 0xA005, warn__none},
   {s_c1phan,    0x5AA50000, 0x5A00, warn__none},
   {s_c1phan,    0xA55A0000, 0x005A, warn__none},
   {s_c1phan,    0x55AA0000, 0x500A, warn__none},
   {s_c1phan,    0xAA550000, 0x0A50, warn__none},
   {s4x4,        0x0000AAAA, 0x0A0A, warn__none},
   {s4x4,        0x0000CCCC, 0x8484, warn__none},
   {s4x4,        0xAAAA0000, 0xA0A0, warn__none},
   {s4x4,        0xCCCC0000, 0x4848, warn__none},
   {s_qtag,      0x003300CC, 0xCC,   warn__ctrscpls_endstand},
   {s_qtag,      0x00CC0033, 0x33,   warn__ctrstand_endscpls},
   {s_rigger,    0x00FF0000, 0x33,   warn__ctrscpls_endstand},
   {s_rigger,    0x00CC0033, 0x33,   warn__ctrscpls_endstand},
   {s_rigger,    0x000000FF, 0xCC,   warn__ctrstand_endscpls},
   {s_rigger,    0x003300CC, 0xCC,   warn__ctrstand_endscpls},
   {s_bone,      0x00FF0000, 0x33,   warn__ctrstand_endscpls},
   {s_bone,      0x000000FF, 0xCC,   warn__ctrscpls_endstand},
   {s_crosswave, 0x00FF0000, 0xCC,   warn__ctrscpls_endstand},
   {s_crosswave, 0x000000FF, 0x33,   warn__ctrstand_endscpls},
   {nothing,     0,          0,      warn__none}};


Private void initialize_one_table(tm_thing *map_start, int np)
{
   tm_thing *map_search;

   for (map_search = map_start; map_search->outsetup != nothing; map_search++) {
      int i;
      unsigned int m;
      unsigned int imask = 0;
      unsigned int omask = 0;
      unsigned int osidemask = 0;

      /* All 1's for people in outer setup. */
      unsigned int alloutmask = (1 << (setup_attrs[map_search->outsetup].setup_limits+1))-1;

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

            if (((map_search->sidewaysmask >> (i*2)) ^ map_search->rot) & 1) {
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
      if (map_search->limit != setup_attrs[map_search->insetup].setup_limits+1) fail("Tandem table initialization failed: limit wrong.");
      if (map_search->limit > 8) fail("Tandem table initialization failed: limit too big.");
      if (map_search->outsidemask != osidemask) fail("Tandem table initialization failed: Smask.");
   }
}


extern void initialize_tandem_tables(void)
{
   initialize_one_table(maps_isearch_twosome, 2);
   initialize_one_table(maps_isearch_threesome, 3);
   initialize_one_table(maps_isearch_foursome, 4);
   initialize_one_table(maps_isearch_boxsome, 4);
   initialize_one_table(maps_isearch_dmdsome, 4);
}


Private void unpack_us(
   tm_thing *map_ptr,
   unsigned int orbitmask,
   tandrec *tandstuff,
   setup *result)
{
   int i;
   unsigned int m, o, r;

   result->kind = map_ptr->outsetup;
   result->rotation = tandstuff->virtual_result.rotation - map_ptr->rot;
   result->result_flags = tandstuff->virtual_result.result_flags;
   r = map_ptr->rot*011;

   for (i=0, m=map_ptr->insinglemask, o=orbitmask; i<map_ptr->limit; i++, m>>=2, o>>=2) {
      int z = rotperson(tandstuff->virtual_result.people[i].id1, r);

      if (m & 1) {
         /* Unpack single person. */
         if (z != 0) {
            personrec f;
            int ii = (z & 0700) >> 6;
            f = tandstuff->real_front_people[ii];
            if (f.id1) f.id1 = (f.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            result->people[map_ptr->map1[i]] = f;
         }
      }
      else {
         /* Unpack tandem/couples person. */
         personrec f, b, b2, b3;

         if (z != 0) {
            int ii = (z >> 6) & 7;
            f = tandstuff->real_front_people[ii];
            b = tandstuff->real_back_people[ii];
            b2 = tandstuff->real_second_people[ii];
            b3 = tandstuff->real_third_people[ii];

            if (f.id1) f.id1 = (f.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b.id1) b.id1 = (b.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b2.id1) b2.id1 = (b2.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));
            if (b3.id1) b3.id1 = (b3.id1 & ~(ROLL_MASK|STABLE_MASK|077)) | (z & (ROLL_MASK|STABLE_MASK|013));

            if ((o + (map_ptr->rot&1) + 1) & 2) {
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
   int tnd_cpl_siam,
   tandrec *tandstuff)
{
   int i;
   int lat;
   unsigned int m, sgl;

   tandstuff->virtual_setup.rotation = map_ptr->rot & 1;
   tandstuff->virtual_setup.kind = map_ptr->insetup;

   for (i=0, m=map_ptr->sidewaysmask, sgl=map_ptr->insinglemask; i<map_ptr->limit; i++, m>>=2, sgl>>=2) {
      personrec f, b, b2, b3;

      personrec *ptr = &tandstuff->virtual_setup.people[i];

      f = s[map_ptr->map1[i]];

      lat = (m ^ map_ptr->rot) & 1;

      if (sgl & 1) {
         ptr->id1 = (f.id1 & ~0700) | (i << 6) | BIT_TANDVIRT;
         ptr->id2 = f.id2;
         b.id1 = 0xFFFFFFFF;
      }
      else {
         unsigned int u1;

         b = s[map_ptr->map2[i]];
         b2 = b;
         b3 = b;
         if (tandstuff->np >= 3) b2 = s[map_ptr->map3[i]];
         if (tandstuff->np >= 4) b3 = s[map_ptr->map4[i]];

         if (tnd_cpl_siam == 3) {
            /* If this is skew/skewsome, we require people paired in the appropriate way.
               This means [f, b3] must match, [b, b2] must match, and [f, b] must not match. */

            if ((((f.id1 ^ b3.id1) | (b.id1 ^ b2.id1) | (~(f.id1 ^ b.id1))) & BIT_PERSON))
               fail("Can't find skew people.");
         }
         else {
            /* If this is not skew/skewsome, we forbid a live person grouped with a phantom unless
               some phantom concept was used (either something like "phantom tandem" or some other
               phantom concept such as "split phantom lines"). */

            if (!(tandstuff->virtual_setup.cmd.cmd_misc_flags & CMD_MISC__PHANTOMS)) {

/* In fact, we forbid ANY PERSON AT ALL to be a phantom, even if paired with another phantom. */

               if (!(f.id1 & b.id1 & b2.id1 & b3.id1 & BIT_PERSON))
                  fail("Use \"phantom\" concept in front of this concept.");


/*
               if ((((f.id1 ^ b.id1) | (f.id1 ^ b2.id1) | (f.id1 ^ b3.id1)) & BIT_PERSON))
                  fail("Use \"phantom\" concept in front of this concept.");
*/


            }
         }

         u1 = f.id1 | b.id1 | b2.id1 | b3.id1;

         if (u1) {
            unsigned int vp1, vp2;
         
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

            ptr->id1 = (vp1 & ~0700) | (i << 6) | BIT_TANDVIRT;
            ptr->id2 = vp2;

            if (twosome >= 2)
               ptr->id1 |= STABLE_ENAB | (STABLE_RBIT * fraction);
         }
         else {
            ptr->id1 = 0;
            ptr->id2 = 0;
         }
      
         tandstuff->vertical_people[i] = lat ^ 1;   /* 1 if original people were near/far; 0 if lateral */
         tandstuff->twosomep[i] = twosome;
      }

      if (map_ptr->rot & 1)   /* Compensate for above rotation. */
         (void) copy_rot(&tandstuff->virtual_setup, i, &tandstuff->virtual_setup, i, 033);

      tandstuff->real_front_people[i] = f;
      tandstuff->real_back_people[i] = b;
      tandstuff->real_second_people[i] = b2;
      tandstuff->real_third_people[i] = b3;
   }
}




extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,               /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,              /* number, if doing fractional twosome/solid */
   int phantom,               /* normal=0 / phantom=1 / gruesome=2 */
   int tnd_cpl_siam,          /* tandem = 0 / couples = 1 / siamese = 2 / skew = 3
                                 tandem of 3 = 4 / couples of 3 = 5 / tandem of 4 = 6 / couples of 4 = 7
                                 box = 8 / diamond = 9 */
   setup *result)
{
   selector_kind saved_selector;
   tandrec tandstuff;
   tm_thing *map;
   tm_thing *map_search;
   uint32 nsmask, ewmask, allmask;
   int i, np;
   uint32 jbit;
   uint32 hmask;
   uint32 orbitmask;
   uint32 sglmask;
   uint32 livemask;
   setup saved_originals;
   long_boolean fractional = FALSE;
   tm_thing *our_map_table;

   tandstuff.single_mask = 0;
   clear_people(result);

   if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do tandem/couples concept from this position.");

   /* We use the phantom indicator to forbid an already-distorted setup.
      The act of forgiving phantom pairing is based on the setting of the
      CMD_MISC__PHANTOMS bit in the incoming setup, not on the phantom indicator. */

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED) && (phantom != 0))
      fail("Can't specify phantom tandem/couples in virtual or distorted setup.");

   /* Find out who is selected, if this is a "so-and-so are tandem". */
   saved_selector = current_selector;
   if (selector != selector_uninitialized)
      current_selector = selector;

   nsmask = 0;
   ewmask = 0;
   allmask = 0;

   for (i=0, jbit=1; i<=setup_attrs[ss->kind].setup_limits; i++, jbit<<=1) {
      unsigned int p = ss->people[i].id1;
      if (p) {
         allmask |= jbit;
         if ((selector != selector_uninitialized) && !selectp(ss, i))
            tandstuff.single_mask |= jbit;
         else {
            if (p & 1)
               ewmask |= jbit;
            else
               nsmask |= jbit;
         }
      }
   }
   
   current_selector = saved_selector;

   if (!allmask) {
      result->result_flags = 0;
      result->kind = nothing;
      return;
   }

   if (twosome >= 2) fractional = TRUE;

   if (fractional && fraction > 4)
      fail("Can't do fractional twosome more than 4/4.");

   if (tnd_cpl_siam == 9) {
      np = 4;
      our_map_table = maps_isearch_dmdsome;
   }
   else if (tnd_cpl_siam == 8 || tnd_cpl_siam == 3) {
      np = 4;
      our_map_table = maps_isearch_boxsome;
   }
   else if (tnd_cpl_siam >= 6) {
      np = 4;
      our_map_table = maps_isearch_foursome;
   }
   else if (tnd_cpl_siam >= 4) {
      np = 3;
      our_map_table = maps_isearch_threesome;
   }
   else {
      np = 2;
      our_map_table = maps_isearch_twosome;
   }

   if (tnd_cpl_siam == 2) {
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
   else if (tnd_cpl_siam == 3) {
      ewmask = allmask;
      nsmask = 0;
   }
   else if (tnd_cpl_siam == 8) {
      ewmask = allmask;
      nsmask = 0;
   }
   else if (tnd_cpl_siam == 9) {
      if (ss->kind == s_ptpd) {
         ewmask = allmask;
         nsmask = 0;
      }
      else {
         ewmask = 0;
         nsmask = allmask;
      }
   }
   else if (tnd_cpl_siam & 1) {
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
            (!(ewmask & (~map_search->outsidemask))) &&
            (!(nsmask & map_search->outsidemask))) {
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
               ss->kind != s3dmd && ss->kind != s4dmd)
         fail("Can't do couples or tandem concepts in this setup.");
   }
   else if (phantom == 2) {
      if (ss->kind != s2x8 || map->insetup != s2x4)
         fail("Can't do gruesome concept in this setup.");
   }

   tandstuff.np = np;
   tandstuff.virtual_setup.cmd = ss->cmd;
   tandstuff.virtual_setup.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
   pack_us(ss->people, map, fraction, twosome, tnd_cpl_siam, &tandstuff);
   update_id_bits(&tandstuff.virtual_setup);
   saved_originals = tandstuff.virtual_setup;    /* Move will clobber the incoming setup.  This bug caused
                                                    embarrassment at an ATA dance, April 3, 1993. */
   move(&tandstuff.virtual_setup, FALSE, &tandstuff.virtual_result);

   if (setup_attrs[tandstuff.virtual_result.kind].setup_limits < 0)
      fail("Don't recognize ending position from this tandem or as couples call.");

   sglmask = 0;     /* Bits appear here in pairs!  Only low bit of each pair is used. */
   livemask = 0;    /* Bits appear here in pairs!  Only low bit of each pair is used. */
   orbitmask = 0;   /* Bits appear here in pairs! */

   /* Compute orbitmask, livemask, and sglmask.
      Since we are synthesizing bit masks, we scan in reverse order to make things easier. */

   for (i=setup_attrs[tandstuff.virtual_result.kind].setup_limits; i>=0; i--) {
      int p = tandstuff.virtual_result.people[i].id1;
      sglmask <<= 2;
      livemask <<= 2;
      orbitmask <<= 2;

      if (p) {
         int vpi = (p >> 6) & 7;
         livemask |= 1;

         if (tandstuff.real_back_people[vpi].id1 == 0xFFFFFFFF) {
            sglmask |= 1;
         }
         else {
            unsigned int orbit;

            if (fractional) {
               if (!(p & STABLE_ENAB))
                  fail("fractional twosome not supported for this call.");
            }

            orbit = p - saved_originals.rotation +
                  tandstuff.virtual_result.rotation - saved_originals.people[vpi].id1;

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

   /* Pick out only low bits for map search, and only bits of live paired people. */
   hmask = (~orbitmask) & livemask & ~sglmask & 0x55555555;

   map_search = our_map_table;
   while (map_search->outsetup != nothing) {
      if ((map_search->insetup == tandstuff.virtual_result.kind) &&
            (map_search->insinglemask == sglmask) &&
            ((map_search->sidewaysmask & livemask) == hmask)) {
         unpack_us(map_search, orbitmask, &tandstuff, result);
         reinstate_rotation(ss, result);
         return;
      }
      map_search++;
   }

   fail("Don't recognize ending position from this tandem or as couples call.");
}
