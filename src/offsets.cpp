// Many concepts invoke concept_distorted.
// These 3 are the only ones that use DISTORTKEY_OFFSCLW_SINGULAR.
// Many things use disttest_offset.

   {"OFFSET LINE",                           concept_distorted,               D, l_c2,
    UC_none, disttest_offset, 0, 0, DISTORTKEY_OFFSCLW_SINGULAR*16+1},
   {"OFFSET WAVE",                           concept_distorted,               D, l_c2,
    UC_none, disttest_offset, 0, 0, DISTORTKEY_OFFSCLW_SINGULAR*16+3},
   {"OFFSET COLUMN",                         concept_distorted,               D, l_c2,
    UC_none, disttest_offset, 0, 0, DISTORTKEY_OFFSCLW_SINGULAR*16+0},

      OFFSET BOX, with 4, and OFFSET 1X4, with 6, are now in.

 "OFFSET BOX", is just a re-imagining of OFFSET LINE.



All of the above are done in an appropriately populated 2x4.

   OFFSET BOX can also be done in a 2x3, where it has the same meaning as "Z".


DISTORTKEY_OFFSCLW_SINGULAR is sensed at istort\3105, 3110, and 3128.
