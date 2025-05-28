/* SD -- square dance caller's helper.

    Copyright (C) 1990-2002  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following external variables:
   c1tglmap1
   c1tglmap2
   dbqtglmap1
   dbqtglmap2
   qttglmap1
   qttglmap2
   bdtglmap1
   bdtglmap2
   rgtglmap1
*/

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"


// These need to be predeclared so that they can refer to each other.
static const tgl_map map2b;
static const tgl_map map2i;
static const tgl_map map2d;
static const tgl_map map2m;
static const tgl_map map2j;
static const tgl_map map2k;
static const tgl_map map2x;
static const tgl_map map2y;


/*
   In C1 phantom: first triangle (inverted),
 then second triangle (upright), then idle.
*/

static const tgl_map map1b = {s_c1phan, s2x4, &map2b, 0, 0,
   {4, 3, 2,   0, 7, 6,   1, 5},
   {6, 8, 10,  14,0, 2,   4, 12},
   {10, 9, 8,  4, 3, 2,   5, 11},
   {6, 5, 4, 2, 1, 0, 3, 7},
   {10, 9, 8, 4, 3, 2, 5, 11}};

static const tgl_map map2b = {s_qtag, s2x4, &map1b, 0, 1,
   {5, 6, 7,   1, 2, 3,   0, 4},
   {3, 15, 13, 11,7, 5,   1,  9},
   {1, 2, 3,   7, 8, 9,   0, 6},
   {7, 6, 5, 3, 2, 1, 0, 4},
   {1, 2, 3, 7, 8, 9, 0, 6}};

static const tgl_map map1i = {nothing, nothing, &map2i, 0, 0,   // Interlocked
   {0, 0, 0,   0, 0, 0,   0, 0},
   {4, 8, 10,  12,0, 2,   6, 14},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static const tgl_map map2i = {nothing, nothing,  &map1i, 0, 0,   // Interlocked
   {0, 0, 0,   0, 0, 0,   0, 0},
   {1, 15, 13, 9, 7, 5,   3, 11},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};




// Maps for deepbigqtg setups.

static const tgl_map map1d = {s2x8, s2x8, &map2d, 0, 0,
   {4, 3, 2,   0, 7, 6,     1, 5},
   {2, 6, 7,   10,14, 15,   0, 8},   // OK
   {10, 9, 8,  4, 3, 2,     5, 11},
   {1, 2, 3,   9, 10, 11,   8, 0},   // OK
   {10, 9, 8, 4, 3, 2, 5, 11}};

static const tgl_map map2d = {s2x8, s2x8, &map1d, 0, 0,
   {4, 3, 2,   0, 7, 6,   1, 5},
   {3, 4, 5,   11,12,13,  1, 9},    // OK
   {10, 9, 8,  4, 3, 2,   5, 11},
   {14,13,12,  6, 5, 4,   7, 15},   // OK
   {10, 9, 8, 4, 3, 2, 5, 11}};

static const tgl_map map1m = {nothing, s2x8, &map2m, 1, 0,   // Interlocked
   {5, 6, 7,   1, 2, 3,   0, 4},
   {0, 6, 7,   8,14,15,  2, 10},   // OK
   {1, 2, 3,   7, 8, 9,   0, 6},
   {7, 6, 5, 3, 2, 1, 0, 4},
   {1, 2, 3, 7, 8, 9, 0, 6}};

static const tgl_map map2m = {nothing, s2x8, &map1m, 1, 0,   // Interlocked
   {5, 6, 7,   1, 2, 3,   0, 4},
   {1, 4, 5,   9,12,13,  3, 11},   // OK
   {1, 2, 3,   7, 8, 9,   0, 6},
   {7, 6, 5, 3, 2, 1, 0, 4},
   {1, 2, 3, 7, 8, 9, 0, 6}};






// Interlocked triangles in quarter-tag:
static const tgl_map map1j = {s_qtag, nothing, &map2j, 1, 1,
   {4, 7, 2,   0, 3, 6,   1, 5},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static const tgl_map map2j = {s_qtag, nothing, &map1j, 1, 1,
   {5, 6, 3,   1, 2, 7,   0, 4},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

// Magic triangles in quarter-tag:
static const tgl_map map1x = {s_qtag, nothing, &map2x, 1, 1,
   {4, 6, 7,   0, 2, 3,   1, 5},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static const tgl_map map2x = {s_qtag, nothing, &map1x, 1, 1,
   {5, 3, 2,   1, 7, 6,   0, 4},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

// Magic interlocked triangles in quarter-tag:
static const tgl_map map1y = {s_qtag, nothing, &map2y, 1, 1,
   {4, 6, 3,   0, 2, 7,   1, 5},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static const tgl_map map2y = {s_qtag, nothing, &map1y, 1, 1,
   {5, 7, 2,   1, 3, 6,   0, 4},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

// Interlocked triangles in bigdmd:
static const tgl_map map1k = {nothing, nothing, &map2k, 1, 0,
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {10, 3, 8,  4, 9, 2,   5, 11},
   {0},
   {0}};

static const tgl_map map2k = {nothing, nothing, &map1k, 1, 0,
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {1, 2, 9,   7, 8, 3,   0, 6},
   {0},
   {0}};

static const tgl_map map2r = {nothing, nothing, &map2r, 1, 0,
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {7, 1, 4,   3, 5, 0,   2, 6},
   {0},
   {0}};

const tgl_map *c1tglmap1[2] = {&map1b, &map1i};
const tgl_map *c1tglmap2[2] = {&map2b, &map2i};

const tgl_map *dbqtglmap1[2] = {&map1d, &map1m};
const tgl_map *dbqtglmap2[2] = {&map2d, &map2m};

const tgl_map *qttglmap1[4] = {&map1b, &map1j, &map1x, &map1y};
const tgl_map *qttglmap2[4] = {&map2b, &map2j, &map2x, &map2y};
const tgl_map *bdtglmap1[2] = {&map1b, &map1k};
const tgl_map *bdtglmap2[2] = {&map2b, &map2k};
const tgl_map *rgtglmap1[2] = {&map2r, &map2r};
