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
   special_magic
   special_interlocked
   mark_end_of_list
   marker_decline
   marker_concept_mod
   marker_concept_comment
   main_call_lists
   number_of_calls
   calling_level
   concept_descriptor_table
   concept_fixer_table
   nice_setup_thing_4x4
   nice_setup_thing_3x4
   nice_setup_thing_2x8
   nice_setup_thing_2x6
   nice_setup_thing_1x12
   nice_setup_thing_1x16
   nice_setup_thing_3dmd
   nice_setup_thing_4dmd
   nice_setup_thing_4x6
   phantom_concept_index
   matrix_2x8_concept_index
   cross_concept_index
   magic_concept_index
   intlk_concept_index
   left_concept_index
   grand_concept_index
   general_concept_offset
   general_concept_size
   concept_offset_tables
   concept_size_tables
   concept_menu_strings
*/

#include "sd.h"


/* The number (typically 4), appearing just before the level in the items below, is the "concparseflags" word.
  To save space, it is entered numerically rather than symbolically.  Here are the meanings of the various bits:
      1 - This is a duplicate, and exists only to make menus nicer.  Ignore it
         when scanning in parser.
      2 - If the parse turns out to be ambiguous, don't use this one -- yield to the other one.
      4 - Parse directly.  It directs the parser to allow this concept (and similar concepts)
         and the following call to be typed on one line.  One needs to be very careful
         about avoiding ambiguity when setting this flag. */

#define U CONCPARSE_MENU_DUP
#define Y CONCPARSE_YIELD_IF_AMB
#define D CONCPARSE_PARSE_DIRECT
#define L CONCPARSE_PARSE_L_TYPE
#define F CONCPARSE_PARSE_F_TYPE
#define G CONCPARSE_PARSE_G_TYPE

concept_descriptor special_magic          = {"MAGIC DIAMOND,",       concept_magic,             L+U+D, l_c1, {0, 1}};
concept_descriptor special_interlocked    = {"INTERLOCKED DIAMOND,", concept_interlocked,       L+U+D, l_c1, {0, 1}};

concept_descriptor mark_end_of_list       = {"????",                 marker_end_of_list,            U, l_dontshow};
concept_descriptor marker_decline         = {"decline???",           concept_mod_declined,          U, l_dontshow};
concept_descriptor marker_concept_mod     = {">>MODIFIED BY<<",      concept_another_call_next_mod, U, l_dontshow, {0, 0, 0}};
concept_descriptor marker_concept_comment = {">>COMMENT<<",          concept_comment,               U, l_dontshow};

callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];
int number_of_calls[NUM_CALL_LIST_KINDS];
dance_level calling_level;


/* **** BEWARE!!!!  In addition to the "size" definitions, there are numerous definitions of positions of
    various concepts for use in the "normalize" operation. */

concept_descriptor concept_descriptor_table[] = {

/* phantom concepts */

#define pl__1_size 52
#define pl__1_spl 0
#define pl__1_ipl 1
#define pl__1_pl 2
#define pl__1_pl8 14
#define pl__1_pl6 15
#define pl__1_tl 22
#define pl__1_tlwt 26
#define pl__1_tlwa 27
#define pl__1_tlwf 28
#define pl__1_tlwb 29
#define pl__1_trtl 39
#define pl__1_qlwt 45
#define pl__1_qlwa 46
#define pl__1_qlwf 47
#define pl__1_qlwb 48
   {"SPLIT PHANTOM LINES",                   concept_do_phantom_2x4,            D, l_c3a, {&map_split_f, phantest_impossible, 1, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM LINES",             concept_do_phantom_2x4,            D, l_c4a, {&map_intlk_f, phantest_impossible, 1, MPKIND__INTLK}},
   {"PHANTOM LINES",                         concept_do_phantom_2x4,            D, l_c3, {&map_full_f, phantest_first_or_both, 1, MPKIND__CONCPHAN}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"12 MATRIX SPLIT PHANTOM LINES",         concept_do_phantom_2x3,            D, l_c3x, {&map_3x4_2x3, phantest_impossible, 1}},
   {"12 MATRIX INTERLOCKED PHANTOM LINES",   concept_do_phantom_2x3,            D, l_c3x, {&map_3x4_2x3_intlk, phantest_impossible, 1}},
   {"12 MATRIX PHANTOM LINES",               concept_do_phantom_2x3,            D, l_c3x, {&map_3x4_2x3_conc, phantest_first_or_both, 1}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM LINES OF 6",              concept_triple_twin,               D, l_c3x, {0, 1, 0, 3, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM LINES OF 6",        concept_triple_twin,               D, l_c3x, {0, 1, 0, 3, MPKIND__INTLK}},
   {"PHANTOM LINES OF 6",                    concept_triple_twin,               D, l_c3x, {0, 1, 0, 3, MPKIND__CONCPHAN}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"DIVIDED LINES",                         concept_divided_2x4,               D, l_c4, {&map_hv_2x4_2, phantest_impossible, 1}},
   {"12 MATRIX DIVIDED LINES",               concept_divided_2x3,               D, l_c4, {&map_2x6_2x3, phantest_impossible, 1}},
   {"TWIN PHANTOM TIDAL LINES",              concept_do_phantom_1x8,            D, l_c3, {0, phantest_impossible, TRUE, 1}},
   {"TWIN PHANTOM LINES OF 6",               concept_do_phantom_1x6,            D, l_c3, {0, phantest_impossible, TRUE, 1}},
   {"TRIPLE TIDAL LINES",                    concept_do_phantom_triple_1x8,     D, l_c3, {0, phantest_impossible, TRUE, 1}},
   {"CRAZY PHANTOM LINES",                   concept_phan_crazy,                D, l_c4, {0, 1, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_LINES}},
   {"REVERSE CRAZY PHANTOM LINES",           concept_phan_crazy,                D, l_c4, {0, 8+1, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_LINES}},
   {"@a CRAZY PHANTOM LINES",                concept_frac_phan_crazy,           D, l_c4, {0, 16+1, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_LINES}},
   {"@a REVERSE CRAZY PHANTOM LINES",        concept_frac_phan_crazy,           D, l_c4, {0, 16+8+1, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_LINES}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE LINES",                          concept_triple_lines,              D, l_c2, {0, 1}},
   {"TRIPLE LINES OF 6",                     concept_do_phantom_triple_1x6,     D, l_c3x, {0, phantest_ok, 0, 1}},
   {"CENTER TRIPLE LINE",                    concept_in_out_std,                D, l_c2, {0, 1, CONCPROP__NEEDK_CTR_1X4, 0}},
   {"OUTSIDE TRIPLE LINES",                  concept_in_out_std,                D, l_c2, {0, 8+1, CONCPROP__NEEDK_END_1X4, 0}},
   {"TRIPLE LINES WORKING TOGETHER",         concept_triple_lines_tog_std,      D, l_c4a, {0, 10, 1}},
   {"TRIPLE LINES WORKING APART",            concept_triple_lines_tog_std,      D, l_c4a, {0, 11, 1}},
   {"TRIPLE LINES WORKING FORWARD",          concept_triple_lines_tog,          D, l_c3, {0, 0, 1}},
   {"TRIPLE LINES WORKING BACKWARD",         concept_triple_lines_tog,          D, l_c3, {0, 2, 1}},
   {"TRIPLE LINES WORKING CLOCKWISE",        concept_triple_lines_tog_std,      D, l_c4, {0, 8, 1}},
   {"TRIPLE LINES WORKING COUNTERCLOCKWISE", concept_triple_lines_tog_std,      D, l_c4, {0, 9, 1}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE DIAGONAL LINES",                 concept_triple_diag,               D, l_c4, {0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING FORWARD", concept_triple_diag_together,      D, l_c4, {0, 0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING BACKWARD",concept_triple_diag_together,      D, l_c4, {0, 2, 1}},
   {"TRIPLE TIDAL LINES WORKING FORWARD",    concept_triple_1x8_tog,            D, l_c4, {0, 0, 1}},
   {"TRIPLE TIDAL LINES WORKING BACKWARD",   concept_triple_1x8_tog,            D, l_c4, {0, 2, 1}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE TWIN LINES",                     concept_triple_twin,               D, l_c4a, {0, 1, 0, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"QUADRUPLE LINES",                       concept_quad_lines,                D, l_c4a, {0, 1}},
   {"QUADRUPLE LINES OF 6",                  concept_triple_twin,               D, l_c3x, {0, 1, 0, 2}},
   {"CENTER PHANTOM LINES",                  concept_in_out_std,                D, l_c3,  {0, 16+1, CONCPROP__NEEDK_4X4_1X16, 0}},
   {"OUTSIDE PHANTOM LINES",                 concept_in_out_std,                D, l_c3,  {0, 16+8+1, CONCPROP__NEEDK_4X4_1X16, 0}},
   {"QUADRUPLE LINES WORKING TOGETHER",      concept_quad_lines_tog_std,        D, l_c4a, {0, 10, 1}},
   {"QUADRUPLE LINES WORKING APART",         concept_quad_lines_tog_std,        D, l_c4a, {0, 11, 1}},
   {"QUADRUPLE LINES WORKING FORWARD",       concept_quad_lines_tog,            D, l_c4a, {0, 0, 1}},
   {"QUADRUPLE LINES WORKING BACKWARD",      concept_quad_lines_tog,            D, l_c4a, {0, 2, 1}},
   {"QUADRUPLE LINES WORKING CLOCKWISE",     concept_quad_lines_tog_std,        D, l_c4,  {0, 8, 1}},
   {"QUADRUPLE LINES WORKING COUNTERCLOCKWISE",concept_quad_lines_tog_std,      D, l_c4,  {0, 9, 1}},
   {"QUADRUPLE LINES WORKING TOWARD THE CENTER",concept_quad_lines_tog_std,     D, l_c4,  {0, 12, 1}},
/* -------- column break -------- */
#define pl__2_size 52
#define pl__2_spw 0
#define pl__2_ipw 1
#define pl__2_pw 2
   {"SPLIT PHANTOM WAVES",                   concept_do_phantom_2x4,            D, l_c3a, {&map_split_f, phantest_impossible, 3, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM WAVES",             concept_do_phantom_2x4,            D, l_c4a, {&map_intlk_f, phantest_impossible, 3, MPKIND__INTLK}},
   {"PHANTOM WAVES",                         concept_do_phantom_2x4,            D, l_c3, {&map_full_f, phantest_first_or_both, 3, MPKIND__CONCPHAN}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM WAVES OF 6",              concept_triple_twin,               D, l_c3x, {0, 3, 0, 3, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM WAVES OF 6",        concept_triple_twin,               D, l_c3x, {0, 3, 0, 3, MPKIND__INTLK}},
   {"PHANTOM WAVES OF 6",                    concept_triple_twin,               D, l_c3x, {0, 3, 0, 3, MPKIND__CONCPHAN}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"DIVIDED WAVES",                         concept_divided_2x4,               D, l_c4, {&map_hv_2x4_2, phantest_impossible, 3}},
   {"12 MATRIX DIVIDED WAVES",               concept_divided_2x3,               D, l_c4, {&map_2x6_2x3, phantest_impossible, 3}},
   {"TWIN PHANTOM TIDAL WAVES",              concept_do_phantom_1x8,            D, l_c3, {0, phantest_impossible, TRUE, 3}},
   {"TWIN PHANTOM WAVES OF 6",               concept_do_phantom_1x6,            D, l_c3, {0, phantest_impossible, TRUE, 3}},
   {"TRIPLE TIDAL WAVES",                    concept_do_phantom_triple_1x8,     D, l_c3, {0, phantest_impossible, TRUE, 3}},
   {"CRAZY PHANTOM WAVES",                   concept_phan_crazy,                D, l_c4, {0, 3, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_WAVES}},
   {"REVERSE CRAZY PHANTOM WAVES",           concept_phan_crazy,                D, l_c4, {0, 8+3, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_WAVES}},
   {"@a CRAZY PHANTOM WAVES",                concept_frac_phan_crazy,           D, l_c4, {0, 16+3, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_WAVES}},
   {"@a REVERSE CRAZY PHANTOM WAVES",        concept_frac_phan_crazy,           D, l_c4, {0, 16+8+3, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_WAVES}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE WAVES",                          concept_triple_lines,              D, l_c2, {0, 3}},
   {"TRIPLE WAVES OF 6",                     concept_do_phantom_triple_1x6,     D, l_c3x, {0, phantest_ok, 0, 3}},
   {"CENTER TRIPLE WAVE",                    concept_in_out_std,                D, l_c2, {0, 3, CONCPROP__NEEDK_CTR_1X4, 0}},
   {"OUTSIDE TRIPLE WAVES",                  concept_in_out_std,                D, l_c2, {0, 8+3, CONCPROP__NEEDK_END_1X4, 0}},
   {"TRIPLE WAVES WORKING TOGETHER",         concept_triple_lines_tog_std,      D, l_c4a, {0, 10, 3}},
   {"TRIPLE WAVES WORKING APART",            concept_triple_lines_tog_std,      D, l_c4a, {0, 11, 3}},
   {"TRIPLE WAVES WORKING FORWARD",          concept_triple_lines_tog,          D, l_c3, {0, 0, 3}},
   {"TRIPLE WAVES WORKING BACKWARD",         concept_triple_lines_tog,          D, l_c3, {0, 2, 3}},
   {"TRIPLE WAVES WORKING CLOCKWISE",        concept_triple_lines_tog_std,      D, l_c4, {0, 8, 3}},
   {"TRIPLE WAVES WORKING COUNTERCLOCKWISE", concept_triple_lines_tog_std,      D, l_c4, {0, 9, 3}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE DIAGONAL WAVES",                 concept_triple_diag,               D, l_c4, {0, 3}},
   {"TRIPLE DIAGONAL WAVES WORKING FORWARD", concept_triple_diag_together,      D, l_c4, {0, 0, 3}},
   {"TRIPLE DIAGONAL WAVES WORKING BACKWARD",concept_triple_diag_together,      D, l_c4, {0, 2, 3}},
   {"TRIPLE TIDAL WAVES WORKING FORWARD",    concept_triple_1x8_tog,            D, l_c4, {0, 0, 3}},
   {"TRIPLE TIDAL WAVES WORKING BACKWARD",   concept_triple_1x8_tog,            D, l_c4, {0, 2, 3}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE TWIN WAVES",                     concept_triple_twin,               D, l_c4a, {0, 3, 0, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"QUADRUPLE WAVES",                       concept_quad_lines,                D, l_c4a, {0, 3}},
   {"QUADRUPLE WAVES OF 6",                  concept_triple_twin,               D, l_c3x, {0, 3, 0, 2}},
   {"CENTER PHANTOM WAVES",                  concept_in_out_std,                D, l_c3,  {0, 16+3, CONCPROP__NEEDK_4X4_1X16, 0}},
   {"OUTSIDE PHANTOM WAVES",                 concept_in_out_std,                D, l_c3,  {0, 16+8+3, CONCPROP__NEEDK_4X4_1X16, 0}},
   {"QUADRUPLE WAVES WORKING TOGETHER",      concept_quad_lines_tog_std,        D, l_c4a, {0, 10, 3}},
   {"QUADRUPLE WAVES WORKING APART",         concept_quad_lines_tog_std,        D, l_c4a, {0, 11, 3}},
   {"QUADRUPLE WAVES WORKING FORWARD",       concept_quad_lines_tog,            D, l_c4a, {0, 0, 3}},
   {"QUADRUPLE WAVES WORKING BACKWARD",      concept_quad_lines_tog,            D, l_c4a, {0, 2, 3}},
   {"QUADRUPLE WAVES WORKING CLOCKWISE",     concept_quad_lines_tog_std,        D, l_c4, {0, 8, 3}},
   {"QUADRUPLE WAVES WORKING COUNTERCLOCKWISE",concept_quad_lines_tog_std,      D, l_c4, {0, 9, 3}},
   {"QUADRUPLE WAVES WORKING TOWARD THE CENTER",concept_quad_lines_tog_std,     D, l_c4, {0, 12, 3}},
/* -------- column break -------- */
#define pl__3_size 52
#define pl__3_spc 0
#define pl__3_ipc 1
#define pl__3_pc 2
#define pl__3_pc8 14
#define pl__3_pc6 15
#define pl__3_tc 22
#define pl__3_tcwt 26
#define pl__3_tcwa 27
#define pl__3_tcwr 28
#define pl__3_tcwl 29
#define pl__3_trtc 39
#define pl__3_qcwt 45
#define pl__3_qcwa 46
#define pl__3_qcwr 47
#define pl__3_qcwl 48
   {"SPLIT PHANTOM COLUMNS",                 concept_do_phantom_2x4,            D, l_c3a, {&map_split_f, phantest_impossible, 0, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM COLUMNS",           concept_do_phantom_2x4,            D, l_c4a, {&map_intlk_f, phantest_impossible, 0, MPKIND__INTLK}},
   {"PHANTOM COLUMNS",                       concept_do_phantom_2x4,            D, l_c3, {&map_full_f, phantest_first_or_both, 0, MPKIND__CONCPHAN}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"12 MATRIX SPLIT PHANTOM COLUMNS",       concept_do_phantom_2x3,            D, l_c3x, {&map_3x4_2x3, phantest_impossible, 0}},
   {"12 MATRIX INTERLOCKED PHANTOM COLUMNS", concept_do_phantom_2x3,            D, l_c3x, {&map_3x4_2x3_intlk, phantest_impossible, 0}},
   {"12 MATRIX PHANTOM COLUMNS",             concept_do_phantom_2x3,            D, l_c3x, {&map_3x4_2x3_conc, phantest_first_or_both, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM COLUMNS OF 6",            concept_triple_twin,               D, l_c3x, {0, 0, 0, 3, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM COLUMNS OF 6",      concept_triple_twin,               D, l_c3x, {0, 0, 0, 3, MPKIND__INTLK}},
   {"PHANTOM COLUMNS OF 6",                  concept_triple_twin,               D, l_c3x, {0, 0, 0, 3, MPKIND__CONCPHAN}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"DIVIDED COLUMNS",                       concept_divided_2x4,               D, l_c4, {&map_hv_2x4_2, phantest_impossible, 0}},
   {"12 MATRIX DIVIDED COLUMNS",             concept_divided_2x3,               D, l_c4, {&map_2x6_2x3, phantest_impossible, 0}},
   {"TWIN PHANTOM TIDAL COLUMNS",            concept_do_phantom_1x8,            D, l_c3, {0, phantest_impossible, TRUE, 0}},
   {"TWIN PHANTOM COLUMNS OF 6",             concept_do_phantom_1x6,            D, l_c3, {0, phantest_impossible, TRUE, 0}},
   {"TRIPLE TIDAL COLUMNS",                  concept_do_phantom_triple_1x8,     D, l_c3, {0, phantest_impossible, TRUE, 0}},
   {"CRAZY PHANTOM COLUMNS",                 concept_phan_crazy,                D, l_c4, {0, 0, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_COLS}},
   {"REVERSE CRAZY PHANTOM COLUMNS",         concept_phan_crazy,                D, l_c4, {0, 8+0, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_COLS}},
   {"@a CRAZY PHANTOM COLUMNS",              concept_frac_phan_crazy,           D, l_c4, {0, 16+0, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_COLS}},
   {"@a REVERSE CRAZY PHANTOM COLUMNS",      concept_frac_phan_crazy,           D, l_c4, {0, 16+8+0, CONCPROP__NEEDK_4X4, CMD_MISC__VERIFY_COLS}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE COLUMNS",                        concept_triple_lines,              D, l_c3, {0, 2}},
   {"TRIPLE COLUMNS OF 6",                   concept_do_phantom_triple_1x6,     D, l_c3x, {0, phantest_ok, 0, 0}},
   {"CENTER TRIPLE COLUMN",                  concept_in_out_std,                D, l_c3, {0, 0, CONCPROP__NEEDK_CTR_1X4, 0}},
   {"OUTSIDE TRIPLE COLUMNS",                concept_in_out_std,                D, l_c3, {0, 8+0, CONCPROP__NEEDK_END_1X4, 0}},
   {"TRIPLE COLUMNS WORKING TOGETHER",       concept_triple_lines_tog_std,      D, l_c4a, {0, 10, 0}},
   {"TRIPLE COLUMNS WORKING APART",          concept_triple_lines_tog_std,      D, l_c4a, {0, 11, 0}},
   {"TRIPLE COLUMNS WORKING RIGHT",          concept_triple_lines_tog,          D, l_c4a, {0, 2, 0}},
   {"TRIPLE COLUMNS WORKING LEFT",           concept_triple_lines_tog,          D, l_c4a, {0, 0, 0}},
   {"TRIPLE COLUMNS WORKING CLOCKWISE",      concept_triple_lines_tog_std,      D, l_c4, {0, 8, 0}},
   {"TRIPLE COLUMNS WORKING COUNTERCLOCKWISE",concept_triple_lines_tog_std,     D, l_c4, {0, 9, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE DIAGONAL COLUMNS",               concept_triple_diag,               D, l_c4, {0, 0}},
   {"TRIPLE DIAGONAL COLUMNS WORKING RIGHT", concept_triple_diag_together,      D, l_c4, {0, 3, 0}},
   {"TRIPLE DIAGONAL COLUMNS WORKING LEFT",  concept_triple_diag_together,      D, l_c4, {0, 1, 0}},
   {"TRIPLE TIDAL COLUMNS WORKING RIGHT",    concept_triple_1x8_tog,            D, l_c4, {0, 2, 0}},
   {"TRIPLE TIDAL COLUMNS WORKING LEFT",     concept_triple_1x8_tog,            D, l_c4, {0, 0, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE TWIN COLUMNS",                   concept_triple_twin,               D, l_c4a, {0, 0, 0, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"QUADRUPLE COLUMNS",                     concept_quad_lines,                D, l_c4a, {0, 0}},
   {"QUADRUPLE COLUMNS OF 6",                concept_triple_twin,               D, l_c3x, {0, 0, 0, 2}},
   {"CENTER PHANTOM COLUMNS",                concept_in_out_std,                D, l_c3a, {0, 16+0, CONCPROP__NEEDK_4X4_1X16, 0}},
   {"OUTSIDE PHANTOM COLUMNS",               concept_in_out_std,                D, l_c3a, {0, 16+8+0, CONCPROP__NEEDK_4X4_1X16, 0}},
   {"QUADRUPLE COLUMNS WORKING TOGETHER",    concept_quad_lines_tog_std,        D, l_c4a, {0, 10, 0}},
   {"QUADRUPLE COLUMNS WORKING APART",       concept_quad_lines_tog_std,        D, l_c4a, {0, 11, 0}},
   {"QUADRUPLE COLUMNS WORKING RIGHT",       concept_quad_lines_tog,            D, l_c4a, {0, 2, 0}},
   {"QUADRUPLE COLUMNS WORKING LEFT",        concept_quad_lines_tog,            D, l_c4a, {0, 0, 0}},
   {"QUADRUPLE COLUMNS WORKING CLOCKWISE",   concept_quad_lines_tog_std,        D, l_c4, {0, 8, 0}},
   {"QUADRUPLE COLUMNS WORKING COUNTERCLOCKWISE",concept_quad_lines_tog_std,    D, l_c4, {0, 9, 0}},
   {"QUADRUPLE COLUMNS WORKING TOWARD THE CENTER",concept_quad_lines_tog_std,   D, l_c4, {0, 12, 0}},
/* -------- column break -------- */
#define pb__1_size 42
#define pb__1_spb 0
#define pb__1_ipb 1
#define pb__1_pb 2
#define pb__1_tb 16
#define pb__1_tbwt 20
#define pb__1_tbwa 21
#define pb__1_tbwf 22
#define pb__1_tbwb 23
#define pb__1_tbwr 24
#define pb__1_tbwl 25
#define pb__1_qbwt 33
#define pb__1_qbwa 34
#define pb__1_qbwf 35
#define pb__1_qbwb 36
#define pb__1_qbwr 37
#define pb__1_qbwl 38
   {"SPLIT PHANTOM BOXES",                   concept_do_phantom_boxes,          D, l_c3, {&map_hv_2x4_2, phantest_impossible}},
   {"INTERLOCKED PHANTOM BOXES",             concept_do_phantom_boxes,          D, l_c4, {&map_intlk_phantom_box, phantest_impossible}},
   {"PHANTOM BOXES",                         concept_do_phantom_boxes,          D, l_c4a, {&map_phantom_box, phantest_first_or_both}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"CRAZY PHANTOM BOXES",                   concept_phan_crazy,                D, l_c4, {0, 4, CONCPROP__NEEDK_2X8, 0}},
   {"REVERSE CRAZY PHANTOM BOXES",           concept_phan_crazy,                D, l_c4, {0, 8+4, CONCPROP__NEEDK_2X8, 0}},
   {"@a CRAZY PHANTOM BOXES",                concept_frac_phan_crazy,           D, l_c4, {0, 16+4, CONCPROP__NEEDK_2X8, 0}},
   {"@a REVERSE CRAZY PHANTOM BOXES",        concept_frac_phan_crazy,           D, l_c4, {0, 16+8+4, CONCPROP__NEEDK_2X8, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE BOXES",                          concept_triple_boxes,              D, l_c1, {0, MPKIND__SPLIT}},
   {"CENTER TRIPLE BOX",                     concept_in_out_nostd,              D, l_c1, {0, 4, CONCPROP__NEEDK_CTR_2X2, 0}},
   {"OUTSIDE TRIPLE BOXES",                  concept_in_out_nostd,              D, l_c1, {0, 8+4, CONCPROP__NEEDK_END_2X2, 0}},
   {"CONCENTRIC TRIPLE BOXES",               concept_triple_boxes,              D, l_c4a, {0, MPKIND__CONCPHAN}},
   {"TRIPLE BOXES WORKING TOGETHER",         concept_triple_boxes_together,     D, l_c4a, {0, 6, 0}},
   {"TRIPLE BOXES WORKING APART",            concept_triple_boxes_together,     D, l_c4a, {0, 7, 0}},
   {"TRIPLE BOXES WORKING FORWARD",          concept_triple_boxes_together,     D, l_c4a, {0, 0, 0}},
   {"TRIPLE BOXES WORKING BACKWARD",         concept_triple_boxes_together,     D, l_c4a, {0, 2, 0}},
   {"TRIPLE BOXES WORKING RIGHT",            concept_triple_boxes_together,     D, l_c4a, {0, 3, 0}},
   {"TRIPLE BOXES WORKING LEFT",             concept_triple_boxes_together,     D, l_c4a, {0, 1, 0}},
   {"TRIPLE BOXES WORKING CLOCKWISE",        concept_triple_boxes_together,     D, l_c4, {0, 8, 0}},
   {"TRIPLE BOXES WORKING COUNTERCLOCKWISE", concept_triple_boxes_together,     D, l_c4, {0, 9, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"QUADRUPLE BOXES",                       concept_quad_boxes,                D, l_c4a, {0, MPKIND__SPLIT}},
   {"CENTER PHANTOM BOXES",                  concept_in_out_nostd,              D, l_c4a, {0, 16+4, CONCPROP__NEEDK_2X8, 0}},
   {"OUTSIDE PHANTOM BOXES",                 concept_in_out_nostd,              D, l_c4a, {0, 16+8+4, CONCPROP__NEEDK_2X8, 0}},
   {"CONCENTRIC QUADRUPLE BOXES",            concept_quad_boxes,                D, l_c4a, {0, MPKIND__CONCPHAN}},
   {"QUADRUPLE BOXES WORKING TOGETHER",      concept_quad_boxes_together,       D, l_c4a, {0, 6, 0}},
   {"QUADRUPLE BOXES WORKING APART",         concept_quad_boxes_together,       D, l_c4a, {0, 7, 0}},
   {"QUADRUPLE BOXES WORKING FORWARD",       concept_quad_boxes_together,       D, l_c4a, {0, 0, 0}},
   {"QUADRUPLE BOXES WORKING BACKWARD",      concept_quad_boxes_together,       D, l_c4a, {0, 2, 0}},
   {"QUADRUPLE BOXES WORKING RIGHT",         concept_quad_boxes_together,       D, l_c4a, {0, 3, 0}},
   {"QUADRUPLE BOXES WORKING LEFT",          concept_quad_boxes_together,       D, l_c4a, {0, 1, 0}},
   {"QUADRUPLE BOXES WORKING CLOCKWISE",     concept_quad_boxes_together,       D, l_c4,  {0, 8, 0}},
   {"QUADRUPLE BOXES WORKING COUNTERCLOCKWISE",concept_quad_boxes_together,     D, l_c4,  {0, 9, 0}},
   {"QUADRUPLE BOXES WORKING TOWARD THE CENTER",concept_quad_boxes_together,    D, l_c4,  {0, 12, 0}},
/* -------- column break -------- */
#define pb__2_size 41
#define pb__2_spd 0
#define pb__2_ipd 1
#define pb__2_pd 2
#define pb__2_spds 4
#define pb__2_ipds 5
#define pb__2_pds 6
#define pb__2_td 20
#define pb__2_tdwt 24
#define pb__2_qd 33
#define pb__2_qdwt 39
   {"SPLIT PHANTOM DIAMONDS",                concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, CMD_MISC__VERIFY_DMD_LIKE}},
   {"INTERLOCKED PHANTOM DIAMONDS",          concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, CMD_MISC__VERIFY_DMD_LIKE}},
   {"PHANTOM DIAMONDS",                      concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, CMD_MISC__VERIFY_DMD_LIKE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM DIAMOND SPOTS",           concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, 0}},
   {"INTERLOCKED PHANTOM DIAMOND SPOTS",     concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, 0}},
   {"PHANTOM DIAMOND SPOTS",                 concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"CRAZY PHANTOM DIAMONDS",                concept_phan_crazy,                D, l_c4, {0, 5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"REVERSE CRAZY PHANTOM DIAMONDS",        concept_phan_crazy,                D, l_c4, {0, 8+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"@a CRAZY PHANTOM DIAMONDS",             concept_frac_phan_crazy,           D, l_c4, {0, 16+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"@a REVERSE CRAZY PHANTOM DIAMONDS",     concept_frac_phan_crazy,           D, l_c4, {0, 16+8+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"CRAZY PHANTOM DIAMOND SPOTS",           concept_phan_crazy,                D, l_c4, {0, 5, CONCPROP__NEEDK_4DMD, 0}},
   {"REVERSE CRAZY PHANTOM DIAMOND SPOTS",   concept_phan_crazy,                D, l_c4, {0, 8+5, CONCPROP__NEEDK_4DMD, 0}},
   {"@a CRAZY PHANTOM DIAMOND SPOTS",        concept_frac_phan_crazy,           D, l_c4, {0, 16+5, CONCPROP__NEEDK_4DMD, 0}},
   {"@a REVERSE CRAZY PHANTOM DIAMOND SPOTS",concept_frac_phan_crazy,           D, l_c4, {0, 16+8+5, CONCPROP__NEEDK_4DMD, 0}},
   {"TWIN PHANTOM DIAMONDS",                 concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_TWINDMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"TWIN PHANTOM DIAMOND SPOTS",            concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_4X6, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE DIAMONDS",                       concept_triple_diamonds,           D, l_c3a, {0, 0, CMD_MISC__VERIFY_DMD_LIKE}},
   {"CENTER TRIPLE DIAMOND",                 concept_in_out_nostd,              D, l_c3a, {0, 5, CONCPROP__NEEDK_CTR_DMD, 0}},
   {"OUTSIDE TRIPLE DIAMONDS",               concept_in_out_nostd,              D, l_c3a, {0, 8+5, CONCPROP__NEEDK_END_DMD, 0}},
   {"TRIPLE DIAMOND SPOTS",                  concept_triple_diamonds,           D, l_c3a, {0, 0, 0}},
   {"TRIPLE DIAMONDS WORKING TOGETHER",      concept_triple_diamonds_together,  D, l_c4a, {0, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"QUADRUPLE DIAMONDS",                    concept_quad_diamonds,             D, l_c4a, {0, 0, CMD_MISC__VERIFY_DMD_LIKE}},
   {"CENTER PHANTOM DIAMONDS",               concept_in_out_nostd,              D, l_c4a, {0, 16+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"OUTSIDE PHANTOM DIAMONDS",              concept_in_out_nostd,              D, l_c4a, {0, 16+8+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_DMD_LIKE}},
   {"CENTER PHANTOM DIAMOND SPOTS",          concept_in_out_nostd,              D, l_c4a, {0, 16+5, CONCPROP__NEEDK_4DMD, 0}},
   {"OUTSIDE PHANTOM DIAMOND SPOTS",         concept_in_out_nostd,              D, l_c4a, {0, 16+8+5, CONCPROP__NEEDK_4DMD, 0}},
   {"QUADRUPLE DIAMOND SPOTS",               concept_quad_diamonds,             D, l_c4a, {0, 0, 0}},
   {"QUADRUPLE DIAMONDS WORKING TOGETHER",   concept_quad_diamonds_together,    D, l_c4a, {0, 0, 0}},
   {"QUADRUPLE DIAMONDS WORKING TOWARD THE CENTER",concept_quad_diamonds_together, D, l_c4, {0, 12, 0}},
/* -------- column break -------- */
#define pb__3_size 59
#define pb__3_sp1 0
#define pb__3_ip1 1
#define pb__3_p1 2
#define pb__3_sp3 4
#define pb__3_ip3 5
#define pb__3_p3 6
#define pb__3_spgt 8
#define pb__3_ipgt 9
#define pb__3_pgt 10
   {"SPLIT PHANTOM 1/4 TAGS",                concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, CMD_MISC__VERIFY_1_4_TAG}},
   {"INTERLOCKED PHANTOM 1/4 TAGS",          concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, CMD_MISC__VERIFY_1_4_TAG}},
   {"PHANTOM 1/4 TAGS",                      concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, CMD_MISC__VERIFY_1_4_TAG}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM 3/4 TAGS",                concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, CMD_MISC__VERIFY_3_4_TAG}},
   {"INTERLOCKED PHANTOM 3/4 TAGS",          concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, CMD_MISC__VERIFY_3_4_TAG}},
   {"PHANTOM 3/4 TAGS",                      concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, CMD_MISC__VERIFY_3_4_TAG}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM GENERAL 1/4 TAGS",        concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"INTERLOCKED PHANTOM GENERAL 1/4 TAGS",  concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"PHANTOM GENERAL 1/4 TAGS",              concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, CMD_MISC__VERIFY_QTAG_LIKE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM 1/4 LINES",               concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, CMD_MISC__VERIFY_REAL_1_4_LINE}},
   {"INTERLOCKED PHANTOM 1/4 LINES",         concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, CMD_MISC__VERIFY_REAL_1_4_LINE}},
   {"PHANTOM 1/4 LINES",                     concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, CMD_MISC__VERIFY_REAL_1_4_LINE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"SPLIT PHANTOM 3/4 LINES",               concept_do_phantom_diamonds,       D, l_c3x, {&map_hv_qtg_2, phantest_impossible, CMD_MISC__VERIFY_REAL_3_4_LINE}},
   {"INTERLOCKED PHANTOM 3/4 LINES",         concept_do_phantom_diamonds,       D, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, CMD_MISC__VERIFY_REAL_3_4_LINE}},
   {"PHANTOM 3/4 LINES",                     concept_do_phantom_diamonds,       D, l_c4, {&map_phantom_dmd, phantest_first_or_both, CMD_MISC__VERIFY_REAL_3_4_LINE}},
   {"CRAZY PHANTOM GENERAL 1/4 TAGS",        concept_phan_crazy,                D, l_c4, {0, 5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"REVERSE CRAZY PHANTOM GENERAL 1/4 TAGS",concept_phan_crazy,                D, l_c4, {0, 8+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"@a CRAZY PHANTOM GENERAL 1/4 TAGS",     concept_frac_phan_crazy,           D, l_c4, {0, 16+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"@a REVERSE CRAZY PHANTOM GENERAL 1/4 TAGS",concept_frac_phan_crazy,        D, l_c4, {0, 16+8+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"TWIN PHANTOM 1/4 TAGS",                 concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_TWINQTAG, CMD_MISC__VERIFY_1_4_TAG}},
   {"TWIN PHANTOM 3/4 TAGS",                 concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_TWINQTAG, CMD_MISC__VERIFY_3_4_TAG}},
   {"TWIN PHANTOM 1/4 LINES",                concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_TWINQTAG, CMD_MISC__VERIFY_REAL_1_4_LINE}},
   {"TWIN PHANTOM 3/4 LINES",                concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_TWINQTAG, CMD_MISC__VERIFY_REAL_3_4_LINE}},
   {"TWIN PHANTOM GENERAL 1/4 TAGS",         concept_do_divided_diamonds,       D, l_c3x, {&map_vv_qtg_2, phantest_impossible, CONCPROP__NEEDK_TWINQTAG, CMD_MISC__VERIFY_QTAG_LIKE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE 1/4 TAGS",                       concept_triple_diamonds,           D, l_c3x, {0, 0, CMD_MISC__VERIFY_1_4_TAG}},
   {"TRIPLE 3/4 TAGS",                       concept_triple_diamonds,           D, l_c3x, {0, 0, CMD_MISC__VERIFY_3_4_TAG}},
   {"TRIPLE 1/4 LINES",                      concept_triple_diamonds,           D, l_c3x, {0, 0, CMD_MISC__VERIFY_REAL_1_4_LINE}},
   {"TRIPLE 3/4 LINES",                      concept_triple_diamonds,           D, l_c3x, {0, 0, CMD_MISC__VERIFY_REAL_3_4_LINE}},
   {"TRIPLE GENERAL 1/4 TAGS",               concept_triple_diamonds,           D, l_c3x, {0, 0, CMD_MISC__VERIFY_QTAG_LIKE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"TRIPLE 1/4 TAGS WORKING TOGETHER",      concept_triple_diamonds_together,  D, l_c4a, {0, 0}},
   {"TRIPLE 1/4 LINES WORKING TOGETHER",     concept_triple_diamonds_together,  D, l_c4a, {0, 0}},
   {"TRIPLE 1/4 TAGS WORKING LEFT",          concept_triple_diamonds_together,  D, l_c4a, {0, 2}},
   {"TRIPLE 1/4 LINES WORKING LEFT",         concept_triple_diamonds_together,  D, l_c4a, {0, 2}},
   {"TRIPLE 1/4 TAGS WORKING RIGHT",         concept_triple_diamonds_together,  D, l_c4a, {0, 1}},
   {"TRIPLE 1/4 LINES WORKING RIGHT",        concept_triple_diamonds_together,  D, l_c4a, {0, 1}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"QUADRUPLE 1/4 TAGS",                    concept_quad_diamonds,             D, l_c4a, {0, 0, CMD_MISC__VERIFY_1_4_TAG}},
   {"QUADRUPLE 3/4 TAGS",                    concept_quad_diamonds,             D, l_c4a, {0, 0, CMD_MISC__VERIFY_3_4_TAG}},
   {"QUADRUPLE 1/4 LINES",                   concept_quad_diamonds,             D, l_c4a, {0, 0, CMD_MISC__VERIFY_REAL_1_4_LINE}},
   {"QUADRUPLE 3/4 LINES",                   concept_quad_diamonds,             D, l_c4a, {0, 0, CMD_MISC__VERIFY_REAL_3_4_LINE}},
   {"QUADRUPLE GENERAL 1/4 TAGS",            concept_quad_diamonds,             D, l_c4a, {0, 0, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"CENTER PHANTOM GENERAL 1/4 TAGS",       concept_in_out_nostd,              D, l_c4a, {0, 16+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"OUTSIDE PHANTOM GENERAL 1/4 TAGS",      concept_in_out_nostd,              D, l_c4a, {0, 16+8+5, CONCPROP__NEEDK_4DMD, CMD_MISC__VERIFY_QTAG_LIKE}},
   {"QUADRUPLE 1/4 TAGS WORKING TOGETHER",   concept_quad_diamonds_together,    D, l_c4a, {0, 0}},
   {"QUADRUPLE 1/4 LINES WORKING TOGETHER",  concept_quad_diamonds_together,    D, l_c4a, {0, 0}},
   {"QUADRUPLE 1/4 TAGS WORKING LEFT",       concept_quad_diamonds_together,    D, l_c4a, {0, 2}},
   {"QUADRUPLE 1/4 LINES WORKING LEFT",      concept_quad_diamonds_together,    D, l_c4a, {0, 2}},
   {"QUADRUPLE 1/4 TAGS WORKING RIGHT",      concept_quad_diamonds_together,    D, l_c4a, {0, 1}},
   {"QUADRUPLE 1/4 LINES WORKING RIGHT",     concept_quad_diamonds_together,    D, l_c4a, {0, 1}},
   {"QUADRUPLE 1/4 TAGS WORKING TOWARD THE CENTER",concept_quad_diamonds_together,D, l_c4a, {0, 12}},
   {"QUADRUPLE 1/4 LINES WORKING TOWARD THE CENTER",concept_quad_diamonds_together,D, l_c4a, {0, 12}},

/* tandem concepts */

#define tt__1_size 73
#define tt__1_cpl 2
#define tt__1_tnd 3
   {"PHANTOM",                               concept_c1_phantom,            L+U+D, l_c1},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"AS COUPLES",                            concept_tandem,                    D, l_a1,         {0, 0, 0,                    0x000, 1}},
   {"TANDEM",                                concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 0}},
   {"SIAMESE",                               concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 2}},
   {"COUPLES OF THREE",                      concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 5}},
   {"TANDEMS OF THREE",                      concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 4}},
   {"SIAMESE OF THREE",                      concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 6}},
   {"COUPLES OF FOUR",                       concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 9}},
   {"TANDEMS OF FOUR",                       concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 8}},
   {"SIAMESE OF FOUR",                       concept_tandem,                    D, l_c1,         {0, 0, 0,                    0x000, 10}},
   {"BOXES ARE SOLID",                       concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 16}},
   {"DIAMONDS ARE SOLID",                    concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 17}},
   {"SKEW",                                  concept_tandem,                    D, l_c4a,        {0, 0, 0,                    0x000, 18}},
   {"GRUESOME AS COUPLES",                   concept_gruesome_tandem,           D, l_c4a,        {0, 0, CONCPROP__NEEDK_2X8,  0x002, 1}},
   {"GRUESOME TANDEM",                       concept_gruesome_tandem,           D, l_c4a,        {0, 0, CONCPROP__NEEDK_2X8,  0x002, 0}},
   {"AS COUPLES IN A 1/4 TAG",               concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_4DMD, 0x000, 1}},
   {"AS COUPLES IN A 3/4 TAG",               concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_4DMD, 0x000, 1}},
   {"AS COUPLES IN A 1/4 LINE",              concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_4DMD, 0x000, 1}},
   {"AS COUPLES IN A 3/4 LINE",              concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_4DMD, 0x000, 1}},
   {"TANDEM IN A 1/4 TAG",                   concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x000, 0}},
   {"TANDEM IN A 3/4 TAG",                   concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x000, 0}},
   {"TANDEM IN A 1/4 LINE",                  concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x000, 0}},
   {"TANDEM IN A 3/4 LINE",                  concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x000, 0}},
   {"@6 ARE AS COUPLES",                     concept_some_are_tandem,         F+D, l_a1,         {0, 0, 0,                    0x100, 1}},
   {"@6 ARE TANDEM",                         concept_some_are_tandem,         F+D, l_c1,         {0, 0, 0,                    0x100, 0}},
   {"@6 ARE COUPLES OF THREE",               concept_some_are_tandem,         F+D, l_c1,         {0, 0, 0,                    0x100, 5}},
   {"@6 ARE TANDEMS OF THREE",               concept_some_are_tandem,         F+D, l_c1,         {0, 0, 0,                    0x100, 4}},
   {"INSIDE TRIANGLES ARE SOLID",            concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 22}},
   {"OUTSIDE TRIANGLES ARE SOLID",           concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 23}},
   {"IN POINT TRIANGLES ARE SOLID",          concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 21}},
   {"OUT POINT TRIANGLES ARE SOLID",         concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 20}},
   {"WAVE-BASED TRIANGLES ARE SOLID",        concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 26}},
   {"TANDEM-BASED TRIANGLES ARE SOLID",      concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 27}},
   {"@k-BASED TRIANGLES ARE SOLID",          concept_some_are_tandem,         F+D, l_c2,         {0, 0, 0,                    0x100, 30}},
   {"3X1 TRIANGLES ARE SOLID",               concept_tandem,                  F+D, l_c2,         {0, 0, 0,                    0x000, 31}},
   {"Y's ARE SOLID",                         concept_tandem,                  F+D, l_c4,         {0, 0, 0,                    0x000, 32}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"COUPLES @b TWOSOME",                    concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 1}},
   {"TANDEM @b TWOSOME",                     concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 0}},
   {"SIAMESE @b TWOSOME",                    concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 2}},
   {"COUPLES OF THREE @b THREESOME",         concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 5}},
   {"TANDEMS OF THREE @b THREESOME",         concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 4}},
   {"SIAMESE OF THREE @b THREESOME",         concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 6}},
   {"COUPLES OF FOUR @b FOURSOME",           concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 9}},
   {"TANDEMS OF FOUR @b FOURSOME",           concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 8}},
   {"SIAMESE OF FOUR @b FOURSOME",           concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 10}},
   {"BOXES ARE SOLID @b BOXSOME",            concept_frac_tandem,             F+D, l_c4,         {0, 0, 0,                    0x020, 16}},
   {"DIAMONDS ARE SOLID @b DIAMONDSOME",     concept_frac_tandem,             F+D, l_c4,         {0, 0, 0,                    0x020, 17}},
   {"SKEW @b TWOSOME",                       concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x020, 18}},
   {"GRUESOME AS COUPLES @b TWOSOME",        concept_gruesome_frac_tandem,      D, l_c4,         {0, 0, CONCPROP__NEEDK_2X8,  0x022, 1}},
   {"GRUESOME TANDEM @b TWOSOME",            concept_gruesome_frac_tandem,      D, l_c4,         {0, 0, CONCPROP__NEEDK_2X8,  0x022, 0}},
   {"COUPLES @b TWOSOME IN A 1/4 TAG",       concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_4DMD, 0x020, 1}},
   {"COUPLES @b TWOSOME IN A 3/4 TAG",       concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_4DMD, 0x020, 1}},
   {"COUPLES @b TWOSOME IN A 1/4 LINE",      concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_4DMD, 0x020, 1}},
   {"COUPLES @b TWOSOME IN A 3/4 LINE",      concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_4DMD, 0x020, 1}},
   {"TANDEM @b TWOSOME IN A 1/4 TAG",        concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x020, 0}},
   {"TANDEM @b TWOSOME IN A 3/4 TAG",        concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x020, 0}},
   {"TANDEM @b TWOSOME IN A 1/4 LINE",       concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x020, 0}},
   {"TANDEM @b TWOSOME IN A 3/4 LINE",       concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x020, 0}},
   {"@6 ARE COUPLES @b TWOSOME",             concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x120, 1}},
   {"@6 ARE TANDEM @b TWOSOME",              concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x120, 0}},
   {"@6 ARE COUPLES OF THREE @b THREESOME",  concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x120, 5}},
   {"@6 ARE TANDEMS OF THREE @b THREESOME",  concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x120, 4}},
   {"INSIDE TRIANGLES ARE SOLID @b THREESOME",       concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 22}},
   {"OUTSIDE TRIANGLES ARE SOLID @b THREESOME",      concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 23}},
   {"IN POINT TRIANGLES ARE SOLID @b THREESOME",     concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 21}},
   {"OUT POINT TRIANGLES ARE SOLID @b THREESOME",    concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 20}},
   {"WAVE-BASED TRIANGLES ARE SOLID @b THREESOME",   concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 26}},
   {"TANDEM-BASED TRIANGLES ARE SOLID @b THREESOME", concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 27}},
   {"@k-BASED TRIANGLES ARE SOLID @b THREESOME", concept_some_are_frac_tandem,F+D, l_c4,         {0, 0, 0,                    0x120, 30}},
   {"3X1 TRIANGLES ARE SOLID @b TRIANGLESOME",       concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 31}},
   {"Y's ARE SOLID @b Y-SOME",                       concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x020, 32}},
/* -------- column break -------- */
#define tt__2_size 73
#define tt__2_cpl2s 2
#define tt__2_tnd2s 3
   {"GRUESOME TWOSOME",                      concept_gruesome_tandem,           D, l_c4a,        {0, 0, CONCPROP__NEEDK_2X8,  0x013, 1}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"COUPLES TWOSOME",                       concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 1}},
   {"TANDEM TWOSOME",                        concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 0}},
   {"SIAMESE TWOSOME",                       concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 2}},
   {"COUPLES THREESOME",                     concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 5}},
   {"TANDEM THREESOME",                      concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 4}},
   {"SIAMESE THREESOME",                     concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 6}},
   {"COUPLES FOURSOME",                      concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 9}},
   {"TANDEM FOURSOME",                       concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 8}},
   {"SIAMESE FOURSOME",                      concept_tandem,                    D, l_c3,         {0, 0, 0,                    0x010, 10}},
   {"BOXSOME",                               concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 16}},
   {"DIAMONDSOME",                           concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 17}},
   {"SKEWSOME",                              concept_tandem,                    D, l_c4a,        {0, 0, 0,                    0x010, 18}},
   {"GRUESOME COUPLES TWOSOME",              concept_gruesome_tandem,           D, l_c4a,        {0, 0, CONCPROP__NEEDK_2X8,  0x012, 1}},
   {"GRUESOME TANDEM TWOSOME",               concept_gruesome_tandem,           D, l_c4a,        {0, 0, CONCPROP__NEEDK_2X8,  0x012, 0}},
   {"COUPLES TWOSOME IN A 1/4 TAG",          concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_4DMD, 0x010, 1}},
   {"COUPLES TWOSOME IN A 3/4 TAG",          concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_4DMD, 0x010, 1}},
   {"COUPLES TWOSOME IN A 1/4 LINE",         concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_4DMD, 0x010, 1}},
   {"COUPLES TWOSOME IN A 3/4 LINE",         concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_4DMD, 0x010, 1}},
   {"TANDEM TWOSOME IN A 1/4 TAG",           concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x010, 0}},
   {"TANDEM TWOSOME IN A 3/4 TAG",           concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x010, 0}},
   {"TANDEM TWOSOME IN A 1/4 LINE",          concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x010, 0}},
   {"TANDEM TWOSOME IN A 3/4 LINE",          concept_tandem,                    D, l_c3,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x010, 0}},
   {"@6 ARE COUPLES TWOSOME",                concept_some_are_tandem,         F+D, l_c3,         {0, 0, 0,                    0x110, 1}},
   {"@6 ARE TANDEM TWOSOME",                 concept_some_are_tandem,         F+D, l_c3,         {0, 0, 0,                    0x110, 0}},
   {"@6 ARE COUPLES THREESOME",              concept_some_are_tandem,         F+D, l_c3,         {0, 0, 0,                    0x110, 5}},
   {"@6 ARE TANDEM THREESOME",               concept_some_are_tandem,         F+D, l_c3,         {0, 0, 0,                    0x110, 4}},
   {"INSIDE TRIANGLES ARE THREESOME",        concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 22}},
   {"OUTSIDE TRIANGLES ARE THREESOME",       concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 23}},
   {"IN POINT TRIANGLES ARE THREESOME",      concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 21}},
   {"OUT POINT TRIANGLES ARE THREESOME",     concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 20}},
   {"WAVE-BASED TRIANGLES ARE THREESOME",    concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 26}},
   {"TANDEM-BASED TRIANGLES ARE THREESOME",  concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 27}},
   {"@k-BASED TRIANGLES ARE THREESOME",      concept_some_are_tandem,         F+D, l_c4a,        {0, 0, 0,                    0x110, 30}},
   {"3X1 TRIANGLES ARE TRIANGLESOME",        concept_tandem,                  F+D, l_c4a,        {0, 0, 0,                    0x010, 31}},
   {"Y-SOME",                                concept_tandem,                  F+D, l_c4,         {0, 0, 0,                    0x010, 32}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"COUPLES TWOSOME @b SOLID",              concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 1}},
   {"TANDEM TWOSOME @b SOLID",               concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 0}},
   {"SIAMESE TWOSOME @b SOLID",              concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 2}},
   {"COUPLES THREESOME @b SOLID",            concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 5}},
   {"TANDEM THREESOME @b SOLID",             concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 4}},
   {"SIAMESE THREESOME @b SOLID",            concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 6}},
   {"COUPLES FOURSOME @b SOLID",             concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 9}},
   {"TANDEM FOURSOME @b SOLID",              concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 8}},
   {"SIAMESE FOURSOME @b SOLID",             concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 10}},
   {"BOXSOME @b SOLID",                      concept_frac_tandem,             F+D, l_c4,         {0, 0, 0,                    0x030, 16}},
   {"DIAMONDSOME @b SOLID",                  concept_frac_tandem,             F+D, l_c4,         {0, 0, 0,                    0x030, 17}},
   {"SKEWSOME @b SOLID",                     concept_frac_tandem,               D, l_c4,         {0, 0, 0,                    0x030, 18}},
   {"GRUESOME TWOSOME @b SOLID",             concept_gruesome_frac_tandem,      D, l_c4,         {0, 0, CONCPROP__NEEDK_2X8,  0x032, 1}},
   {"GRUESOME TANDEM TWOSOME @b SOLID",      concept_gruesome_frac_tandem,      D, l_c4,         {0, 0, CONCPROP__NEEDK_2X8,  0x032, 0}},
   {"COUPLES TWOSOME @b SOLID IN A 1/4 TAG", concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_4DMD, 0x030, 1}},
   {"COUPLES TWOSOME @b SOLID IN A 3/4 TAG", concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_4DMD, 0x030, 1}},
   {"COUPLES TWOSOME @b SOLID IN A 1/4 LINE",concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_4DMD, 0x030, 1}},
   {"COUPLES TWOSOME @b SOLID IN A 3/4 LINE",concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_4DMD, 0x030, 1}},
   {"TANDEM TWOSOME @b SOLID IN A 1/4 TAG",  concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x030, 0}},
   {"TANDEM TWOSOME @b SOLID IN A 3/4 TAG",  concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_TAG,  CONCPROP__NEEDK_TWINQTAG, 0x030, 0}},
   {"TANDEM TWOSOME @b SOLID IN A 1/4 LINE", concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_1_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x030, 0}},
   {"TANDEM TWOSOME @b SOLID IN A 3/4 LINE", concept_frac_tandem,               D, l_c4,         {0, CMD_MISC__VERIFY_REAL_3_4_LINE, CONCPROP__NEEDK_TWINQTAG, 0x030, 0}},
   {"@6 ARE COUPLES TWOSOME @b SOLID",       concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x130, 1}},
   {"@6 ARE TANDEM TWOSOME @b SOLID",        concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x130, 0}},
   {"@6 ARE COUPLES THREESOME @b SOLID",     concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x130, 5}},
   {"@6 ARE TANDEM THREESOME @b SOLID",      concept_some_are_frac_tandem,    F+D, l_c4,         {0, 0, 0,                    0x130, 4}},
   {"INSIDE TRIANGLES ARE THREESOME @b SOLID",       concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 22}},
   {"OUTSIDE TRIANGLES ARE THREESOME @b SOLID",      concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 23}},
   {"IN POINT TRIANGLES ARE THREESOME @b SOLID",     concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 21}},
   {"OUT POINT TRIANGLES ARE THREESOME @b SOLID",    concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 20}},
   {"WAVE-BASED TRIANGLES ARE THREESOME @b SOLID",   concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 26}},
   {"TANDEM-BASED TRIANGLES ARE THREESOME @b SOLID", concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 27}},
   {"@k-BASED TRIANGLES ARE THREESOME @b SOLID", concept_some_are_frac_tandem,F+D, l_c4,         {0, 0, 0,                    0x130, 30}},
   {"3X1 TRIANGLES ARE TRIANGLESOMESOME @b SOLID",   concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 31}},
   {"Y-SOME @b SOLID",                               concept_frac_tandem,     F+D, l_c4,         {0, 0, 0,                    0x030, 32}},

/* distorted concepts */

#define dd__1_size 23
#define dd__1_pofl 22
   {"BIG BLOCK",                             concept_do_phantom_2x4,            D, l_c3a,        {&map_stagger, phantest_only_one, 1, MPKIND__NONE}},
   {"STAIRSTEP LINES",                       concept_do_phantom_2x4,            D, l_c4,         {&map_ladder, phantest_only_one, 1, MPKIND__NONE}},
   {"LADDER LINES",                          concept_do_phantom_2x4,            D, l_c4,         {&map_stairst, phantest_only_one, 1, MPKIND__NONE}},
   {"OFFSET LINES",                          concept_distorted,                 D, l_c2,         {0, disttest_offset, 1}},
   {"Z LINES",                               concept_distorted,                 D, l_c4a,        {0, disttest_z, 1}},
   {"DISTORTED LINES",                       concept_distorted,                 D, l_c3,         {0, disttest_any, 1}},
   {"DIAGONAL LINE",                         concept_single_diagonal,           D, l_c1,         {0, 1}},
   {"@6 IN YOUR DIAGONAL LINE",              concept_single_diagonal_sel,       D, l_c1,         {0, 8+1}},
   {"DIAGONAL LINES",                        concept_double_diagonal,           D, l_c4a,        {0, 1, 0}},
   {"DIAGONAL LINES OF 3",                   concept_double_diagonal,           D, l_c1,         {0, 1, 1}},
   {"DISTORTED TIDAL LINE",                  concept_distorted,                 D, l_c3,         {0, disttest_any, 8+1}},
   {"DISTORTED LINE OF 6",                   concept_double_diagonal,           D, l_c3,         {0, 1, 2}},
   {"DOUBLE BENT TIDAL LINE",                concept_dblbent,                   D, l_c4,         {0, 1}},
   {"@6 IN YOUR DISTORTED LINE",             concept_so_and_so_only,            D, l_c1,         {0, 6, 1}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"PARALLELOGRAM",                         concept_parallelogram,             D, l_c2},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"PHANTOM BIG BLOCK LINES",               concept_do_phantom_2x4,            D, l_c4,         {&map_stagger, phantest_both, 1, MPKIND__NONE}},
   {"PHANTOM STAIRSTEP LINES",               concept_do_phantom_2x4,            D, l_c4,         {&map_ladder, phantest_both, 1, MPKIND__NONE}},
   {"PHANTOM LADDER LINES",                  concept_do_phantom_2x4,            D, l_c4,         {&map_stairst, phantest_both, 1, MPKIND__NONE}},
   {"PHANTOM OFFSET LINES",                  concept_do_phantom_2x4,            D, l_c4a,        {&map_offset, phantest_both, 1, MPKIND__NONE}},
/* -------- column break -------- */
#define dd__2_size 23
   {"BIG BLOCK WAVES",                       concept_do_phantom_2x4,            D, l_c3a,        {&map_stagger, phantest_only_one, 3, MPKIND__NONE}},
   {"STAIRSTEP WAVES",                       concept_do_phantom_2x4,            D, l_c4,         {&map_ladder, phantest_only_one, 3, MPKIND__NONE}},
   {"LADDER WAVES",                          concept_do_phantom_2x4,            D, l_c4,         {&map_stairst, phantest_only_one, 3, MPKIND__NONE}},
   {"OFFSET WAVES",                          concept_distorted,                 D, l_c2,         {0, disttest_offset, 3}},
   {"Z WAVES",                               concept_distorted,                 D, l_c4a,        {0, disttest_z, 3}},
   {"DISTORTED WAVES",                       concept_distorted,                 D, l_c3,         {0, disttest_any, 3}},
   {"DIAGONAL WAVE",                         concept_single_diagonal,           D, l_c1,         {0, 3}},
   {"@6 IN YOUR DIAGONAL WAVE",              concept_single_diagonal_sel,       D, l_c1,         {0, 8+3}},
   {"DIAGONAL WAVES",                        concept_double_diagonal,           D, l_c4a,        {0, 3, 0}},
   {"DIAGONAL WAVES OF 3",                   concept_double_diagonal,           D, l_c1,         {0, 3, 1}},
   {"DISTORTED TIDAL WAVE",                  concept_distorted,                 D, l_c3,         {0, disttest_any, 8+3}},
   {"DISTORTED WAVE OF 6",                   concept_double_diagonal,           D, l_c3,         {0, 3, 2}},
   {"DOUBLE BENT TIDAL WAVE",                concept_dblbent,                   D, l_c4,         {0, 3}},
   {"@6 IN YOUR DISTORTED WAVE",             concept_so_and_so_only,            D, l_c1,         {0, 6, 3}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"@6 IN YOUR DISTORTED BOX",              concept_so_and_so_only,            D, l_c1,         {0, 6, 4}},
   {"@6 IN YOUR DISTORTED DIAMOND",          concept_so_and_so_only,            D, l_c1,         {0, 6, 5}},
         {"", concept_comment, 0, l_nonexistent_concept},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"PHANTOM BIG BLOCK WAVES",               concept_do_phantom_2x4,            D, l_c4,         {&map_stagger, phantest_both, 3, MPKIND__NONE}},
   {"PHANTOM STAIRSTEP WAVES",               concept_do_phantom_2x4,            D, l_c4,         {&map_ladder, phantest_both, 3, MPKIND__NONE}},
   {"PHANTOM LADDER WAVES",                  concept_do_phantom_2x4,            D, l_c4,         {&map_stairst, phantest_both, 3, MPKIND__NONE}},
   {"PHANTOM OFFSET WAVES",                  concept_do_phantom_2x4,            D, l_c4a,        {&map_offset, phantest_both, 3, MPKIND__NONE}},
/* -------- column break -------- */
#define dd__3_size 23
#define dd__3_pob 17
#define dd__3_pofc 22
   {"STAGGER",                               concept_do_phantom_2x4,            D, l_c2,         {&map_stagger, phantest_only_one, 0, MPKIND__NONE}},
   {"STAIRSTEP COLUMNS",                     concept_do_phantom_2x4,            D, l_c4,         {&map_stairst, phantest_only_one, 0, MPKIND__NONE}},
   {"LADDER COLUMNS",                        concept_do_phantom_2x4,            D, l_c4,         {&map_ladder, phantest_only_one, 0, MPKIND__NONE}},
   {"OFFSET COLUMNS",                        concept_distorted,                 D, l_c2,         {0, disttest_offset, 0}},
   {"Z COLUMNS",                             concept_distorted,                 D, l_c4a,        {0, disttest_z, 0}},
   {"DISTORTED COLUMNS",                     concept_distorted,                 D, l_c3,         {0, disttest_any, 0}},
   {"DIAGONAL COLUMN",                       concept_single_diagonal,           D, l_c1,         {0, 2}},
   {"@6 IN YOUR DIAGONAL COLUMN",            concept_single_diagonal_sel,       D, l_c1,         {0, 8+2}},
   {"DIAGONAL COLUMNS",                      concept_double_diagonal,           D, l_c4a,        {0, 0, 0}},
   {"DIAGONAL COLUMNS OF 3",                 concept_double_diagonal,           D, l_c1,         {0, 0, 1}},
   {"DISTORTED TIDAL COLUMN",                concept_distorted,                 D, l_c3,         {0, disttest_any, 8+0}},
   {"DISTORTED COLUMN OF 6",                 concept_double_diagonal,           D, l_c3,         {0, 0, 2}},
   {"DOUBLE BENT TIDAL COLUMN",              concept_dblbent,                   D, l_c4,         {0, 0}},
   {"@6 IN YOUR DISTORTED COLUMN",           concept_so_and_so_only,            D, l_c1,         {0, 6, 2}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"O",                                     concept_do_phantom_2x4,            D, l_c1,         {&map_o_s2x4_3, phantest_only_first_one, 0, MPKIND__NONE}},
   {"BUTTERFLY",                             concept_do_phantom_2x4,            D, l_c1,         {&map_x_s2x4_3, phantest_only_second_one, 0, MPKIND__NONE}},
   {"PHANTOM BUTTERFLY OR O",                concept_do_phantom_2x4,            D, l_c4a,        {&map_but_o, phantest_both, 0, MPKIND__NONE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"PHANTOM STAGGER COLUMNS",               concept_do_phantom_2x4,            D, l_c4,         {&map_stagger, phantest_both, 0, MPKIND__NONE}},
   {"PHANTOM STAIRSTEP COLUMNS",             concept_do_phantom_2x4,            D, l_c4,         {&map_stairst, phantest_both, 0, MPKIND__NONE}},
   {"PHANTOM LADDER COLUMNS",                concept_do_phantom_2x4,            D, l_c4,         {&map_ladder, phantest_both, 0, MPKIND__NONE}},
   {"PHANTOM OFFSET COLUMNS",                concept_do_phantom_2x4,            D, l_c4a,        {&map_offset, phantest_both, 0, MPKIND__NONE}},

/* 4-person distorted concepts */

#define d4__1_size 9
#define d4__1_magic 3
   {"SPLIT",                                 concept_split,                     D, l_mainstream},
   {"ONCE REMOVED",                          concept_once_removed,              D, l_c2,         {0, 0}},
   {"ONCE REMOVED DIAMONDS",                 concept_once_removed,              D, l_c3a,        {0, 1}},
   {"MAGIC",                                 concept_magic,                   L+D, l_c1},
   {"DIAGONAL BOX",                          concept_do_both_boxes,             D, l_c3,         {&map_2x4_diagonal, 97, FALSE}},
   {"TRAPEZOID",                             concept_do_both_boxes,             D, l_c3,         {&map_2x4_trapezoid, 97, FALSE}},
   {"OVERLAPPED DIAMONDS",                   concept_overlapped_diamond,        D, l_c4,         {0, 0}},
   {"OVERLAPPED LINES",                      concept_overlapped_diamond,        D, l_c4,         {0, 1}},
   {"OVERLAPPED WAVES",                      concept_overlapped_diamond,        D, l_c4,         {0, 3}},
/* -------- column break -------- */
#define d4__2_size 6
   {"INTERLOCKED PARALLELOGRAM",             concept_do_both_boxes,             D, l_c3x,        {&map_2x4_int_pgram, 97, TRUE}},
   {"INTERLOCKED BOXES",                     concept_misc_distort,              D, l_c3x,        {0, 3, 0}},
   {"TWIN PARALLELOGRAMS",                   concept_misc_distort,              D, l_c3x,        {0, 2, 0}},
   {"Z",                                     concept_misc_distort,              D, l_c3,         {0, 0, 0, 1}},
   {"EACH Z",                                concept_misc_distort,              D, l_c3,         {0, 0, 0, 2}},
   {"INTERLOCKED Z's",                       concept_misc_distort,              D, l_c4a,        {0, 0, 8, 2}},
/* -------- column break -------- */
#define d4__3_size 10
   {"JAY",                                   concept_misc_distort,              D, l_c3a,        {0, 1, 0,  0x0000}},
   {"BACK-TO-FRONT JAY",                     concept_misc_distort,              D, l_c3a,        {0, 1, 8,  0x0000}},
   {"BACK-TO-BACK JAY",                      concept_misc_distort,              D, l_c3a,        {0, 1, 16, 0x0000}},
   {"FRONT JAY",                             concept_misc_distort,              D, l_c4a,        {0, 1, 0,  0x0000}},
   {"BACK JAY",                              concept_misc_distort,              D, l_c4a,        {0, 1, 0,  0xAAAA}},
   {"LEFT JAY",                              concept_misc_distort,              D, l_c4a,        {0, 1, 0,  0x5555}},
   {"RIGHT JAY",                             concept_misc_distort,              D, l_c4a,        {0, 1, 0,  0xFFFF}},
   {"FACING PARALLELOGRAM",                  concept_misc_distort,              D, l_c3a,        {0, 4, 0}},
   {"BACK-TO-FRONT PARALLELOGRAM",           concept_misc_distort,              D, l_c3a,        {0, 4, 8}},
   {"BACK-TO-BACK PARALLELOGRAM",            concept_misc_distort,              D, l_c3a,        {0, 4, 16}},
/* -------- column break -------- */
#define d4__4_size 7
#define d4__4_pibl 2
   {"BLOCKS",                                concept_do_phantom_2x2,            D, l_c1,         {&map_blocks, phantest_2x2_only_two}},
   {"IN YOUR BLOCKS",                        concept_do_phantom_2x2,            D, l_c1,         {&map_blocks, phantest_2x2_only_two}},
   {"4 PHANTOM INTERLOCKED BLOCKS",          concept_do_phantom_2x2,            D, l_c4a,        {&map_blocks, phantest_2x2_both}},
   {"TRIANGULAR BOXES",                      concept_do_phantom_2x2,            D, l_c4,         {&map_trglbox, phantest_2x2_only_two}},
   {"4 PHANTOM TRIANGULAR BOXES",            concept_do_phantom_2x2,            D, l_c4,         {&map_trglbox, phantest_2x2_both}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"DISTORTED BLOCKS",                      concept_misc_distort,              D, l_c4a,        {0, 5, 0}},

/* Miscellaneous concepts */

#define mm__1_size 78
#define mm__1_left 0
#define mm__1_cross 2
#define mm__1_grand 5
#define mm__1_intlk 9
#define mm__1_phan 12
   {"LEFT",                                  concept_left,                    L+D, l_mainstream},
   {"REVERSE",                               concept_reverse,                 L+D, l_mainstream},
   {"CROSS",                                 concept_cross,                   L+D, l_mainstream},
   {"SINGLE",                                concept_single,                  L+D, l_mainstream},
   {"SINGLE FILE",                           concept_singlefile,              L+D, l_mainstream},  /* Not C3?  No, mainstream, for dixie style. */
   {"GRAND",                                 concept_grand,                   L+D, l_plus},
   {"MIRROR",                                concept_mirror,                    D, l_c3},
   {"TRIANGLE",                              concept_triangle,                  D, l_c1},
   {"DIAMOND",                               concept_diamond,                   D, l_c3x},
   {"INTERLOCKED",                           concept_interlocked,             L+D, l_c1},
   {"12 MATRIX",                             concept_12_matrix,                 D, l_c3x},
   {"16 MATRIX",                             concept_16_matrix,                 D, l_c3x},
   {"PHANTOM",                               concept_c1_phantom,              L+D, l_c1},
   {"FUNNY",                                 concept_funny,                     D, l_c2},
   {"MATRIX",                                concept_matrix,                  G+D, l_c4},
   {"ASSUME WAVES",                          concept_assume_waves,              D, l_c3,         {0, cr_wave_only,  0, 0}},
   {"ASSUME MINIWAVES",                      concept_assume_waves,              D, l_c3,         {0, cr_miniwaves,  0, 0}},
   {"ASSUME COUPLES",                        concept_assume_waves,              D, l_c3,         {0, cr_couples_only,0, 0}},
   {"ASSUME TWO-FACED LINES",                concept_assume_waves,              D, l_c3,         {0, cr_2fl_only,   0, 0}},
   {"ASSUME ONE-FACED LINES",                concept_assume_waves,              D, l_c3,         {0, cr_1fl_only,   0, 0}},
   {"ASSUME INVERTED LINES",                 concept_assume_waves,              D, l_c3,         {0, cr_magic_only, 0, 0}},
   {"ASSUME NORMAL BOXES",                   concept_assume_waves,              D, l_c3,         {0, cr_wave_only,  2, 0}},
   {"ASSUME INVERTED BOXES",                 concept_assume_waves,              D, l_c3,         {0, cr_magic_only, 2, 0}},
   {"ASSUME NORMAL COLUMNS",                 concept_assume_waves,              D, l_c3,         {0, cr_wave_only,  1, 0}},
   {"ASSUME MAGIC COLUMNS",                  concept_assume_waves,              D, l_c3,         {0, cr_magic_only, 1, 0}},
   {"ASSUME EIGHT CHAIN",                    concept_assume_waves,              D, l_c3,         {0, cr_li_lo,      1, 1}},
   {"ASSUME TRADE BY",                       concept_assume_waves,              D, l_c3,         {0, cr_li_lo,      1, 2}},
   {"ASSUME DPT",                            concept_assume_waves,              D, l_c3,         {0, cr_2fl_only,   1, 1}},
   {"ASSUME CDPT",                           concept_assume_waves,              D, l_c3,         {0, cr_2fl_only,   1, 2}},
   {"ASSUME FACING LINES",                   concept_assume_waves,              D, l_c3,         {0, cr_li_lo,      0, 1}},
   {"ASSUME BACK-TO-BACK LINES",             concept_assume_waves,              D, l_c3,         {0, cr_li_lo,      0, 2}},
   {"ASSUME GENERAL DIAMONDS",               concept_assume_waves,              D, l_c3,         {0, cr_diamond_like,0, 0}},
   {"ASSUME GENERAL 1/4 TAGS",               concept_assume_waves,              D, l_c3,         {0, cr_qtag_like,  0, 0}},
   {"ASSUME 1/4 TAGS",                       concept_assume_waves,              D, l_c3,         {0, cr_real_1_4_tag, 0, 0}},
   {"ASSUME RIGHT 1/4 TAGS",                 concept_assume_waves,              D, l_c3,         {0, cr_jleft,      0, 2}},
   {"ASSUME LEFT 1/4 TAGS",                  concept_assume_waves,              D, l_c3,         {0, cr_jright,     0, 2}},
   {"ASSUME 3/4 TAGS",                       concept_assume_waves,              D, l_c3,         {0, cr_real_3_4_tag, 0, 0}},
   {"ASSUME LEFT 3/4 TAGS",                  concept_assume_waves,              D, l_c3,         {0, cr_jleft,      0, 1}},
   {"ASSUME RIGHT 3/4 TAGS",                 concept_assume_waves,              D, l_c3,         {0, cr_jright,     0, 1}},
   {"ASSUME 1/4 LINES",                      concept_assume_waves,              D, l_c3,         {0, cr_real_1_4_line, 0, 0}},
   {"ASSUME RIGHT 1/4 LINES",                concept_assume_waves,              D, l_c3,         {0, cr_ijleft,     0, 2}},
   {"ASSUME LEFT 1/4 LINES",                 concept_assume_waves,              D, l_c3,         {0, cr_ijright,    0, 2}},
   {"ASSUME 3/4 LINES",                      concept_assume_waves,              D, l_c3,         {0, cr_real_3_4_line, 0, 0}},
   {"ASSUME LEFT 3/4 LINES",                 concept_assume_waves,              D, l_c3,         {0, cr_ijleft,     0, 1}},
   {"ASSUME RIGHT 3/4 LINES",                concept_assume_waves,              D, l_c3,         {0, cr_ijright,    0, 1}},
   {"ASSUME NORMAL DIAMONDS",                concept_assume_waves,              D, l_c3,         {0, cr_jright,     4, 0}},
   {"ASSUME FACING DIAMONDS",                concept_assume_waves,              D, l_c3,         {0, cr_jleft,      4, 0}},
   {"ASSUME NORMAL INTERLOCKED DIAMONDS",    concept_assume_waves,              D, l_c3,         {0, cr_ijright,    4, 0}},
   {"ASSUME FACING INTERLOCKED DIAMONDS",    concept_assume_waves,              D, l_c3,         {0, cr_ijleft,     4, 0}},
   {"ASSUME NORMAL CASTS",                   concept_assume_waves,              D, l_c3,         {0, cr_alwaysfail, 0, 0}},
   {"WITH ACTIVE PHANTOMS",                  concept_active_phantoms,           D, l_c3},
   {"INVERT",                                concept_snag_mystic,             L+D, l_c4,         {0, CMD_MISC2__CTR_END_INVERT}},
   {"INVERT CENTRAL",                        concept_central,                   D, l_c4,         {0, CMD_MISC2__CENTRAL_PLAIN | CMD_MISC2__CTR_END_INV_CONC}},
   {"CENTRAL",                               concept_central,                   D, l_c3,         {0, CMD_MISC2__CENTRAL_PLAIN}},
   {"SNAG",                                  concept_snag_mystic,               D, l_c4,         {0, CMD_MISC2__CENTRAL_SNAG}},
   {"MYSTIC",                                concept_snag_mystic,               D, l_c4,         {0, CMD_MISC2__CENTRAL_MYSTIC}},
   {"INVERT SNAG",                           concept_snag_mystic,             F+D, l_c4,         {0, CMD_MISC2__CENTRAL_SNAG | CMD_MISC2__CTR_END_INV_CONC}},
   {"INVERT MYSTIC",                         concept_snag_mystic,             F+D, l_c4,         {0, CMD_MISC2__CENTRAL_MYSTIC | CMD_MISC2__CTR_END_INV_CONC}},
   {"FAN",                                   concept_fan,                     L+D, l_c3},
   {"YOYO",                                  concept_yoyo,                    L+D, l_c4},
   {"STRAIGHT",                              concept_straight,                L+D, l_c4},
   {"TWISTED",                               concept_twisted,                 L+D, l_c4},
   {"@6 ARE STANDARD IN",                    concept_standard,                L+D, l_c4a,        {0, 0}},
   {"STABLE",                                concept_stable,                    D, l_c3a,        {0, FALSE, FALSE}},
   {"@6 ARE STABLE",                         concept_so_and_so_stable,        F+D, l_c3a,        {0, TRUE,  FALSE}},
   {"@b STABLE",                             concept_frac_stable,               D, l_c4,         {0, FALSE, TRUE}},
   {"@6 ARE @b STABLE",                      concept_so_and_so_frac_stable,   F+D, l_c4,         {0, TRUE,  TRUE}},
   {"EMULATE",                               concept_emulate,                   D, l_c4},
   {"TRACE",                                 concept_trace,                     0, l_c3x},
   {"STRETCH",                               concept_old_stretch,               D, l_c1},
   {"STRETCHED SETUP",                       concept_new_stretch,               D, l_c2,         {0, 16}},
   {"STRETCHED BOX",                         concept_new_stretch,               D, l_c2,         {0, 18}},
   {"STRETCHED LINE",                        concept_new_stretch,               D, l_c2,         {0, 1}},
   {"STRETCHED WAVE",                        concept_new_stretch,               D, l_c2,         {0, 3}},
   {"STRETCHED COLUMN",                      concept_new_stretch,               D, l_c2,         {0, 4}},
   {"STRETCHED DIAMOND",                     concept_new_stretch,               D, l_c3a,        {0, 19}},
   {"FERRIS",                                concept_ferris,                    D, l_c3x,        {0, 0}},
   {"RELEASE",                               concept_ferris,                    D, l_c3a,        {0, 1}},
/* -------- column break -------- */
#define mm__2_size 78
   {"CENTERS",                               concept_centers_or_ends,           D, l_mainstream, {0, selector_centers, FALSE}},
   {"ENDS",                                  concept_centers_or_ends,           D, l_mainstream, {0, selector_ends,    FALSE}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,          0, l_mainstream, {0, selector_centers, FALSE}},
   {"CENTER 6",                              concept_centers_or_ends,           D, l_mainstream, {0, selector_center6, FALSE}},
   {"OUTER 6",                               concept_centers_or_ends,           D, l_mainstream, {0, selector_outer6,  FALSE}},
   {"CENTER 2",                              concept_centers_or_ends,           D, l_mainstream, {0, selector_center2, FALSE}},
   {"OUTER 2",                               concept_centers_or_ends,           D, l_mainstream, {0, selector_outer2,  FALSE}},
   {"CENTER 6/OUTER 2",                      concept_centers_and_ends,          0, l_mainstream, {0, selector_center6, FALSE}},
   {"CENTER 2/OUTER 6",                      concept_centers_and_ends,          0, l_mainstream, {0, selector_center2, FALSE}},
   {"ENDS CONCENTRIC",                       concept_centers_or_ends,           D, l_c1,         {0, selector_ends,    TRUE}},
   {"OUTER 2 CONCENTRIC",                    concept_centers_or_ends,           D, l_c1,         {0, selector_outer2,  TRUE}},
   {"OUTER 6 CONCENTRIC",                    concept_centers_or_ends,           D, l_c1,         {0, selector_outer6,  TRUE}},
   {"CENTERS AND ENDS CONCENTRIC",           concept_centers_and_ends,          0, l_c1,         {0, selector_centers, TRUE}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"CHECKPOINT",                            concept_checkpoint,                0, l_c2,         {0, 0}},
   {"REVERSE CHECKPOINT",                    concept_checkpoint,                0, l_c3,         {0, 1}},
   {"CHECKERBOARD",                          concept_checkerboard,              D, l_c3a,        {0, s1x4, 0}},
   {"CHECKERBOX",                            concept_checkerboard,              D, l_c3a,        {0, s2x2, 0}},
   {"CHECKERDIAMOND",                        concept_checkerboard,              D, l_c4a,        {0, sdmd, 0}},
   {"@6 PREFERRED FOR TRADE, CHECKERBOARD",  concept_sel_checkerboard,          D, l_c3a,        {0, s1x4, 8}},
   {"@6 PREFERRED FOR TRADE, CHECKERBOX",    concept_sel_checkerboard,          D, l_c3a,        {0, s2x2, 8}},
   {"@6 PREFERRED FOR TRADE, CHECKERDIAMOND",concept_sel_checkerboard,          D, l_c4a,        {0, sdmd, 8}},
   {"ORBITBOARD",                            concept_checkerboard,              D, l_c4,         {0, s1x4, 2}},
   {"ORBITBOX",                              concept_checkerboard,              D, l_c4,         {0, s2x2, 2}},
   {"ORBITDIAMOND",                          concept_checkerboard,              D, l_c4,         {0, sdmd, 2}},
   {"TWIN ORBITBOARD",                       concept_checkerboard,              D, l_c4,         {0, s1x4, 3}},
   {"TWIN ORBITBOX",                         concept_checkerboard,              D, l_c4,         {0, s2x2, 3}},
   {"TWIN ORBITDIAMOND",                     concept_checkerboard,              D, l_c4,         {0, sdmd, 3}},
   {"SHADOW LINE",                           concept_checkerboard,              D, l_c4a,        {0, s1x4, 1}},
   {"SHADOW BOX",                            concept_checkerboard,              D, l_c4a,        {0, s2x2, 1}},
   {"SHADOW DIAMOND",                        concept_checkerboard,              D, l_c4a,        {0, sdmd, 1}},
   {"ANCHOR THE @6",                         concept_anchor,                  F+D, l_c4},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"@6",                                    concept_so_and_so_only,          Y+D, l_mainstream, {0, 4, 0}},
   {"@6 (while the others)",                 concept_some_vs_others,            0, l_mainstream, {0, 5, 0}},
   {"@6 DISCONNECTED",                       concept_so_and_so_only,            D, l_c2,         {0, 6, 0}},
   {"@6 DISCONNECTED (while the others)",    concept_some_vs_others,            0, l_c2,         {0, 7, 0}},
   {"@6 DO YOUR PART",                       concept_so_and_so_only,          F+D, l_mainstream, {0, 0, 0}},
   {"@6 DO YOUR PART (while the others)",    concept_some_vs_others,            F, l_mainstream, {0, 1, 0}},
   {"ON YOUR OWN",                           concept_on_your_own,               0, l_c4a},
   {"OWN THE @6",                            concept_some_vs_others,            F, l_c3a,        {0, 3, 0}},
   {"IGNORE THE @6",                         concept_so_and_so_only,          F+D, l_c1,         {0, 8, 0}},
   {"@6 WORK",                               concept_so_and_so_only,          L+D, l_c2,         {0, 11, 0}},
   {"@6 LEAD FOR A",                         concept_so_and_so_only,            D, l_mainstream, {0, 12, 0}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"two calls in succession",               concept_sequential,                0, l_mainstream},
   {"FOLLOW IT BY",                          concept_special_sequential,        0, l_c2,         {0, 0}},
   {"PRECEDE IT BY",                         concept_special_sequential,        0, l_c2,         {0, 1}},
   {"CRAZY",                                 concept_crazy,                     D, l_c2,         {0, 0, FALSE}},
   {"REVERSE CRAZY",                         concept_crazy,                     D, l_c3x,        {0, 1, FALSE}},
   {"@a CRAZY",                              concept_frac_crazy,                D, l_c2,         {0, 0, TRUE}},
   {"@a REVERSE CRAZY",                      concept_frac_crazy,                D, l_c3x,        {0, 1, TRUE}},
   {"RANDOM",                                concept_meta,                    G+D, l_c3,         {0, 0}},
   {"REVERSE RANDOM",                        concept_meta,                    G+D, l_c3x,        {0, 1}},
   {"PIECEWISE",                             concept_meta,                    G+D, l_c3x,        {0, 2}},
   {"@9/@9",                                 concept_fractional,              F+D, l_mainstream, {0, 0}},
   {"1-@9/@9",                               concept_fractional,              F+D, l_mainstream, {0, 2}},
   {"TWICE",                                 concept_twice,                   F+D, l_mainstream, {0, 0, 2}},
   {"@9 TIMES",                              concept_n_times,                 F+D, l_mainstream, {0, 1}},
   {"DO THE LAST @9/@9",                     concept_fractional,              F+D, l_c1,         {0, 1}},
   {"REVERSE ORDER",                         concept_meta,                      D, l_mainstream, {0, 5}},
   {"INTERLACE",                             concept_interlace,                 0, l_c3x},
   {"INITIALLY",                             concept_meta,                    G+D, l_c3a,        {0, 3}},
   {"FINALLY",                               concept_meta,                    G+D, l_c4,         {0, 7}},
   {"FINISH",                                concept_meta,                      D, l_c2,         {0, 4}},
   {"LIKE A",                                concept_meta,                      D, l_c2,         {0, 6}},
   {"SHIFTY",                                concept_meta,                      D, l_c4,         {0, 11}},
   {"SHIFT @9",                              concept_nth_part,                F+D, l_c4,         {0, 10}},
   {"@6 START",                              concept_so_and_so_begin,         F+D, l_c1,         {0, 0}},
   {"SKIP THE @u PART",                      concept_nth_part,                F+D, l_c1,         {0, 9}},
   {"DO THE @u PART",                        concept_nth_part,                L+D, l_c1,         {0, 8}},
   {"REPLACE THE @u PART",                   concept_replace_nth_part,          0, l_c1,         {0, 8}},
   {"REPLACE THE LAST PART",                 concept_replace_last_part,         0, l_c1,         {0, 0}},
   {"START WITH",                            concept_special_sequential,        0, l_c2,         {0, 2}},
   {"INTERRUPT AFTER THE @u PART",           concept_replace_nth_part,          0, l_c1,         {0, 9}},
   {"INTERRUPT BEFORE THE LAST PART",        concept_replace_last_part,         0, l_c1,         {0, 1}},
   {"INTERRUPT AFTER @9/@9",                 concept_interrupt_at_fraction,     0, l_c1,         {0, 2}},
   {"SANDWICH",                              concept_sandwich,                  0, l_c3,         {0, 3}},
/* -------- column break -------- */
#define mm__3_size 65
#define mm__3_3x3 59
#define mm__3_4x4 60
   {"INSIDE TRIANGLES",                      concept_randomtrngl,               D, l_c1,         {0, 2}},
   {"OUTSIDE TRIANGLES",                     concept_randomtrngl,               D, l_c1,         {0, 3}},
   {"IN POINT TRIANGLES",                    concept_randomtrngl,               D, l_c1,         {0, 5}},
   {"OUT POINT TRIANGLES",                   concept_randomtrngl,               D, l_c1,         {0, 4}},
   {"TALL 6",                                concept_randomtrngl,               D, l_c3x,        {0, 0}},
   {"SHORT 6",                               concept_randomtrngl,               D, l_c3x,        {0, 1}},
   {"WAVE-BASED TRIANGLES",                  concept_randomtrngl,               D, l_c1,         {0, 6}},
   {"TANDEM-BASED TRIANGLES",                concept_randomtrngl,               D, l_c1,         {0, 7}},
   {"@k-BASED TRIANGLES",                    concept_selbasedtrngl,             D, l_c1,         {0, 20}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"CONCENTRIC",                            concept_concentric,                D, l_c1,         {0, schema_concentric}},
   {"CROSS CONCENTRIC",                      concept_concentric,                D, l_c2,         {0, schema_cross_concentric}},
   {"SINGLE CONCENTRIC",                     concept_concentric,                D, l_c4,         {0, schema_single_concentric}},
   {"SINGLE CROSS CONCENTRIC",               concept_concentric,                D, l_c4,         {0, schema_single_cross_concentric}},
   {"GRAND SINGLE CONCENTRIC",               concept_concentric,                D, l_c4,         {0, schema_grand_single_concentric}},
   {"GRAND SINGLE CROSS CONCENTRIC",         concept_concentric,                D, l_c4,         {0, schema_grand_single_cross_concentric}},
   {"CONCENTRIC DIAMONDS",                   concept_concentric,                D, l_c1,         {0, schema_concentric_diamonds}},
   {"CROSS CONCENTRIC DIAMONDS",             concept_concentric,                D, l_c2,         {0, schema_cross_concentric_diamonds}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"GRAND WORKING FORWARD",                 concept_grand_working,             D, l_c4a,        {0, 0}},
   {"GRAND WORKING BACKWARD",                concept_grand_working,             D, l_c4a,        {0, 2}},
   {"GRAND WORKING LEFT",                    concept_grand_working,             D, l_c4a,        {0, 1}},
   {"GRAND WORKING RIGHT",                   concept_grand_working,             D, l_c4a,        {0, 3}},
   {"GRAND WORKING AS CENTERS",              concept_grand_working,             D, l_c4a,        {0, 10}},
   {"GRAND WORKING AS ENDS",                 concept_grand_working,             D, l_c4a,        {0, 11}},
   {"GRAND WORKING CLOCKWISE",               concept_grand_working,             D, l_c4a,        {0, 8}},
   {"GRAND WORKING COUNTERCLOCKWISE",        concept_grand_working,             D, l_c4a,        {0, 9}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"@6 ARE CENTERS OF A DOUBLE-OFFSET 1/4 TAG",concept_double_offset,          D, l_c4,         {0, 0}},
   {"@6 ARE CENTERS OF A DOUBLE-OFFSET 3/4 TAG",concept_double_offset,          D, l_c4,         {0, 1}},
   {"@6 ARE CENTERS OF DOUBLE-OFFSET DIAMONDS",concept_double_offset,           D, l_c4,         {0, 3}},
   {"@6 ARE CENTERS OF DOUBLE-OFFSET DIAMOND SPOTS",concept_double_offset,      D, l_c4,         {0, 2}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"INRIGGER",                              concept_rigger,                    D, l_c3x,        {0, 2}},
   {"OUTRIGGER",                             concept_rigger,                    D, l_c3x,        {0, 0}},
   {"LEFTRIGGER",                            concept_rigger,                    D, l_c3x,        {0, 1}},
   {"RIGHTRIGGER",                           concept_rigger,                    D, l_c3x,        {0, 3}},
   {"BACKRIGGER",                            concept_rigger,                    D, l_c3x,        {0, 16}},
   {"FRONTRIGGER",                           concept_rigger,                    D, l_c3x,        {0, 18}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"COMMON POINT GALAXY",                   concept_common_spot,               D, l_c4,         {0, 1,    0}},
   {"COMMON SPOT COLUMNS",                   concept_common_spot,               D, l_c4,         {0, 2,    0}},
   {"COMMON POINT DIAMONDS",                 concept_common_spot,               D, l_c4,         {0, 4,    0}},
   {"COMMON SPOT DIAMONDS",                  concept_common_spot,               D, l_c4,         {0, 4,    0}},
   {"COMMON POINT HOURGLASS",                concept_common_spot,               D, l_c4,         {0, 0x80, 0}},
   {"COMMON SPOT HOURGLASS",                 concept_common_spot,               D, l_c4,         {0, 0x80, 0}},
   {"COMMON END LINES",                      concept_common_spot,               D, l_c4,         {0, 0x10, 0}},
   {"COMMON END WAVES",                      concept_common_spot,               D, l_c4,         {0, 0x10, CMD_MISC__VERIFY_WAVES}},
   {"COMMON CENTER LINES",                   concept_common_spot,               D, l_c4,         {0, 0x20, 0}},
   {"COMMON CENTER WAVES",                   concept_common_spot,               D, l_c4,         {0, 0x20, CMD_MISC__VERIFY_WAVES}},
   {"COMMON SPOT LINES",                     concept_common_spot,               D, l_c4,         {0, 0x78, 0}},
   {"COMMON SPOT WAVES",                     concept_common_spot,               D, l_c4,         {0, 0x70, CMD_MISC__VERIFY_WAVES}},
   {"COMMON SPOT TWO-FACED LINES",           concept_common_spot,               D, l_c4,         {0, 8,    CMD_MISC__VERIFY_2FL}},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"1X2",                                   concept_1x2,                     L+D, l_c3},
   {"2X1",                                   concept_2x1,                     L+D, l_c3},
   {"2X2",                                   concept_2x2,                     L+D, l_c3a},
   {"1X3",                                   concept_1x3,                     L+D, l_c2},
   {"3X1",                                   concept_3x1,                     L+D, l_c2},
   {"3X3",                                   concept_3x3,                     L+D, l_c3x},
   {"4X4",                                   concept_4x4,                     L+D, l_c3x},
         {"", concept_comment, 0, l_nonexistent_concept},
   {"ALL 4 COUPLES",                         concept_all_8,                     D, l_a2,         {0, 0}},
   {"ALL 8",                                 concept_all_8,                     D, l_a2,         {0, 1}},
   {"ALL 8 (diamonds)",                      concept_all_8,                     D, l_a2,         {0, 2}},

/* general concepts */

#define gg_2x8_matrix 23
   {"CENTERS",                               concept_centers_or_ends,         U+D, l_mainstream, {0, selector_centers, FALSE}},
   {"ENDS",                                  concept_centers_or_ends,         U+D, l_mainstream, {0, selector_ends, FALSE}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,        U+D, l_mainstream, {0, selector_centers, FALSE}},
   {"LEFT",                                  concept_left,                  L+U+D, l_mainstream},
   {"REVERSE",                               concept_reverse,               L+U+D, l_mainstream},
   {"SPLIT",                                 concept_split,                   U+D, l_mainstream},
   {"TRIPLE 1X4s",                           concept_triple_lines,              D, l_c3, {0, 0}},
   {"EACH 1X4",                              concept_each_1x4,                  D, l_mainstream, {0, 0, 0}},
   {"EACH LINE",                             concept_each_1x4,                  D, l_mainstream, {0, 1, 0}},
   {"EACH COLUMN",                           concept_each_1x4,                  D, l_mainstream, {0, 2, 0}},
   {"EACH WAVE",                             concept_each_1x4,                  D, l_mainstream, {0, 3, 0}},
   {"EACH BOX",                              concept_each_1x4,                  D, l_mainstream, {0, 0, 2}},
   {"EACH DIAMOND",                          concept_each_1x4,                  D, l_mainstream, {0, 0, 1}},
   {"CROSS",                                 concept_cross,                 L+U+D, l_mainstream},
   {"GRAND",                                 concept_grand,                 L+U+D, l_plus},
   {"MAGIC",                                 concept_magic,                 L+U+D, l_c1},
   {"SINGLE",                                concept_single,                L+U+D, l_mainstream},
   {"INTERLOCKED",                           concept_interlocked,           L+U+D, l_c1},
   {"12 MATRIX",                             concept_12_matrix,               U+D, l_c3x},
   {"16 MATRIX",                             concept_16_matrix,               U+D, l_c3x},
   {"1X12 MATRIX",                           concept_create_matrix,             D, l_c3x,        {0, s1x12, CONCPROP__NEEDK_1X12}},
   {"1X16 MATRIX",                           concept_create_matrix,             D, l_c3x,        {0, s1x16, CONCPROP__NEEDK_1X16}},
   {"2X6 MATRIX",                            concept_create_matrix,             D, l_c3x,        {0, s2x6,  CONCPROP__NEEDK_2X6}},
   {"2X8 MATRIX",                            concept_create_matrix,             D, l_c3x,        {0, s2x8,  CONCPROP__NEEDK_2X8}},
   {"3X4 MATRIX",                            concept_create_matrix,             D, l_c3x,        {0, s3x4,  CONCPROP__NEEDK_3X4}},
   {"4X4 MATRIX",                            concept_create_matrix,             D, l_c3x,        {0, s4x4,  CONCPROP__NEEDK_4X4}},
   {"3X8 MATRIX",                            concept_create_matrix,             D, l_c3x,        {0, s3x8,  CONCPROP__NEEDK_3X8}},
   {"4X6 MATRIX",                            concept_create_matrix,             D, l_c3x,        {0, s4x6,  CONCPROP__NEEDK_4X6}},
   {"16 MATRIX OF PARALLEL DIAMONDS",        concept_create_matrix,             D, l_c3x,        {0, s4dmd, CONCPROP__NEEDK_4DMD}},
   {"PHANTOM",                               concept_c1_phantom,            L+U+D, l_c1},
   {"FUNNY",                                 concept_funny,                   U+D, l_c2},
   {"CONCENTRIC",                            concept_concentric,              U+D, l_c1,         {0, schema_concentric}},
   {"AS COUPLES",                            concept_tandem,                  U+D, l_a1,         {0, FALSE, FALSE, 0, 1}},
   {"TANDEM",                                concept_tandem,                  U+D, l_c1,         {0, FALSE, FALSE, 0, 0}},
   {"DIAGNOSE",                              concept_diagnose,                  D, l_mainstream},  /* Only appears if "-diagnostic" given. */
   {"???",                                   marker_end_of_list}};

/* **** BEWARE!!!!  These things are keyed to "concept_descriptor_table"
   (above) and tell how to divide things into the various menus and popups.
   BE CAREFUL!! */

#define pl__tot_size (pl__1_size+pl__2_size+pl__3_size)
#define pb__tot_size (pb__1_size+pb__2_size+pb__3_size)
#define tt__tot_size (tt__1_size+tt__2_size)
#define dd__tot_size (dd__1_size+dd__2_size+dd__3_size)
#define d4__tot_size (d4__1_size+d4__2_size+d4__3_size+d4__4_size)

#define pl_1_offset (0)
#define pl_2_offset (0 + pl__1_size)
#define pl_3_offset (0 + pl__1_size + pl__2_size)

#define pb_1_offset (0 + pl__tot_size)
#define pb_2_offset (0 + pl__tot_size + pb__1_size)
#define pb_3_offset (0 + pl__tot_size + pb__1_size + pb__2_size)

#define tt_1_offset (0 + pl__tot_size + pb__tot_size)
#define tt_2_offset (0 + pl__tot_size + pb__tot_size + tt__1_size)

#define dd_1_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size)
#define dd_2_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__1_size)
#define dd_3_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__1_size + dd__2_size)

#define d4_1_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size)
#define d4_2_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__1_size)
#define d4_3_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__1_size + d4__2_size)
#define d4_4_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__1_size + d4__2_size + d4__3_size)

#define mm_1_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size)
#define mm_2_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size)
#define mm_3_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size + mm__2_size)

#define gg_offset   (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size + mm__2_size + mm__3_size)


concept_fixer_thing concept_fixer_table[] = {
   {INHERITFLAG_INTLK, 0, pl_1_offset + pl__1_pl, pl_1_offset + pl__1_ipl},    /* INTERLOCKED + PHANTOM LINES */
   {INHERITFLAG_INTLK, 0, pl_2_offset + pl__2_pw, pl_2_offset + pl__2_ipw},    /* INTERLOCKED + PHANTOM WAVES */
   {INHERITFLAG_INTLK, 0, pl_3_offset + pl__3_pc, pl_3_offset + pl__3_ipc},    /* INTERLOCKED + PHANTOM COLUMNS */
   {INHERITFLAG_INTLK, 0, pb_1_offset + pb__1_pb, pb_1_offset + pb__1_ipb},    /* INTERLOCKED + PHANTOM BOXES */
   {INHERITFLAG_INTLK, 0, pb_2_offset + pb__2_pd, pb_2_offset + pb__2_ipd},    /* INTERLOCKED + PHANTOM DIAMONDS */
   {INHERITFLAG_INTLK, 0, pb_2_offset + pb__2_pds, pb_2_offset + pb__2_ipds},  /* INTERLOCKED + PHANTOM DIAMOND SPOTS */
   {INHERITFLAG_INTLK, 0, pb_3_offset + pb__3_p1, pb_3_offset + pb__3_ip1},    /* INTERLOCKED + PHANTOM 1/4 TAGS */
   {INHERITFLAG_INTLK, 0, pb_3_offset + pb__3_p3, pb_3_offset + pb__3_ip3},    /* INTERLOCKED + PHANTOM 3/4 TAGS */
   {INHERITFLAG_INTLK, 0, pb_3_offset + pb__3_pgt, pb_3_offset + pb__3_ipgt},  /* INTERLOCKED + PHANTOM GENERAL 1/4 TAGS */
   {0,      FINAL__SPLIT, pl_1_offset + pl__1_pl, pl_1_offset + pl__1_spl},    /* SPLIT + PHANTOM LINES */
   {0,      FINAL__SPLIT, pl_2_offset + pl__2_pw, pl_2_offset + pl__2_spw},    /* SPLIT + PHANTOM WAVES */
   {0,      FINAL__SPLIT, pl_3_offset + pl__3_pc, pl_3_offset + pl__3_spc},    /* SPLIT + PHANTOM COLUMNS */
   {0,      FINAL__SPLIT, pb_1_offset + pb__1_pb, pb_1_offset + pb__1_spb},    /* SPLIT + PHANTOM BOXES */
   {0,      FINAL__SPLIT, pb_2_offset + pb__2_pd, pb_2_offset + pb__2_spd},    /* SPLIT + PHANTOM DIAMONDS */
   {0,      FINAL__SPLIT, pb_2_offset + pb__2_pds, pb_2_offset + pb__2_spds},  /* SPLIT + PHANTOM DIAMOND SPOTS */
   {0,      FINAL__SPLIT, pb_3_offset + pb__3_p1, pb_3_offset + pb__3_sp1},    /* SPLIT + PHANTOM 1/4 TAGS */
   {0,      FINAL__SPLIT, pb_3_offset + pb__3_p3, pb_3_offset + pb__3_sp3},    /* SPLIT + PHANTOM 3/4 TAGS */
   {0,      FINAL__SPLIT, pb_3_offset + pb__3_pgt, pb_3_offset + pb__3_spgt},  /* SPLIT + PHANTOM GENERAL 1/4 TAGS */
   {0,      0,            0,                       0}};

static int nice_setup_concept_4x4[] = {
   pl_1_offset + pl__1_spl,     /* split phantom lines */
   pl_1_offset + pl__1_ipl,     /* interlocked phantom lines */
   pl_1_offset + pl__1_qlwf,    /* quadruple lines working forward */
   pl_1_offset + pl__1_qlwb,    /* quadruple lines working backward */
   pl_3_offset + pl__3_spc,     /* split phantom columns */
   pl_3_offset + pl__3_ipc,     /* interlocked phantom columns */
   pl_3_offset + pl__3_qcwr,    /* quadruple columns working right */
   pl_3_offset + pl__3_qcwl,    /* quadruple columns working left */
   dd_1_offset + dd__1_pofl,    /* phantom offset lines */
   dd_3_offset + dd__3_pob,     /* phantom "O" or butterfly */
   dd_3_offset + dd__3_pofc,    /* phantom offset columns */
   d4_4_offset + d4__4_pibl,    /* 4 phantom interlocked blocks */

   /* We suspect that the following concepts are less useful than those above,
      because, unless people are situated in certain ways, they won't lead to
      *any* legal calls, and we're too lazy to look at the setup in more
      detail to decide whether they can be used.  Actually, the task of making
      searches restrict their attention to plausible things is very difficult.
      We accept the fact that most of the things we try are pointless. */

   tt_1_offset + tt__1_cpl,     /* phantom as couples (of course, we will need to put in "phantom") */
   tt_1_offset + tt__1_tnd,     /* phantom tandem */
   mm_3_offset + mm__3_4x4,     /* 4x4 */
   -1};

static int nice_setup_concept_3x4[] = {
   pl_1_offset + pl__1_tl,      /* triple lines */
   pl_3_offset + pl__3_tc,      /* triple columns */
   pl_1_offset + pl__1_tlwf,    /* triple lines working forward */
   pl_1_offset + pl__1_tlwb,    /* triple lines working backward */
   pl_3_offset + pl__3_tcwr,    /* triple columns working right */
   pl_3_offset + pl__3_tcwl,    /* triple columns working left */
   mm_3_offset + mm__3_3x3,     /* 3x3 */
   -1};

static int nice_setup_concept_2x8[] = {
   pb_1_offset + pb__1_spb,     /* split phantom boxes */
   pb_1_offset + pb__1_ipb,     /* interlocked phantom boxes */
   pb_1_offset + pb__1_qbwf,    /* quadruple boxes working forward */
   pb_1_offset + pb__1_qbwb,    /* quadruple boxes working backward */
   pb_1_offset + pb__1_qbwr,    /* quadruple boxes working right */
   pb_1_offset + pb__1_qbwl,    /* quadruple boxes working left */
   pb_1_offset + pb__1_qbwt,    /* quadruple boxes working together */
   pb_1_offset + pb__1_qbwa,    /* quadruple boxes working apart */
   pl_1_offset + pl__1_pl8,     /* twin phantom tidal lines */
   pl_3_offset + pl__3_pc8,     /* twin phantom tidal columns */
   /* The less likely ones: */
   mm_3_offset + mm__3_4x4,
   tt_1_offset + tt__1_cpl,     /* (2x8 matrix) as couples */
   tt_1_offset + tt__1_tnd,     /* (2x8 matrix) tandem */
   tt_2_offset + tt__2_cpl2s,   /* (2x8 matrix) couples twosome */
   tt_2_offset + tt__2_tnd2s,   /* (2x8 matrix) tandem twosome */
   -1};

static int nice_setup_concept_2x6[] = {
   pb_1_offset + pb__1_tb,      /* triple boxes */
   pb_1_offset + pb__1_tbwf,    /* triple boxes working forward */
   pb_1_offset + pb__1_tbwb,    /* triple boxes working backward */
   pb_1_offset + pb__1_tbwr,    /* triple boxes working right */
   pb_1_offset + pb__1_tbwl,    /* triple boxes working left */
   pb_1_offset + pb__1_tbwt,    /* triple boxes working together */
   pb_1_offset + pb__1_tbwa,    /* triple boxes working apart */
   pl_1_offset + pl__1_pl6,     /* twin phantom lines of 6 */
   pl_3_offset + pl__3_pc6,     /* twin phantom columns of 6 */
   mm_3_offset + mm__3_3x3,     /* 3x3 */
   /* Let's give these extra probability. */
   pb_1_offset + pb__1_tb,      /* triple boxes */
   pb_1_offset + pb__1_tb,      /* triple boxes */
   pb_1_offset + pb__1_tbwt,    /* triple boxes working together */
   pb_1_offset + pb__1_tbwt,    /* triple boxes working together */
   /* These are actually 2x8 concepts, but they could still be helpful. */
   pb_1_offset + pb__1_spb,     /* split phantom boxes */
   pb_1_offset + pb__1_qbwt,    /* quadruple boxes working together */
   pl_1_offset + pl__1_pl8,     /* phantom tidal lines */
   pl_3_offset + pl__3_pc8,     /* phantom tidal columns */
   -1};

static int nice_setup_concept_1x12[] = {
   pl_1_offset + pl__1_tlwt,     /* end-to-end triple lines working together */
   pl_1_offset + pl__1_tlwa,     /* end-to-end triple lines working apart */
   pl_3_offset + pl__3_tcwt,     /* end-to-end triple columns working together */
   pl_3_offset + pl__3_tcwa,     /* end-to-end triple columns working apart */
   -1};

static int nice_setup_concept_1x16[] = {
   pl_1_offset + pl__1_spl,      /* end-to-end split phantom lines */
   pl_1_offset + pl__1_ipl,      /* end-to-end interlocked phantom lines */
   pl_1_offset + pl__1_qlwt,     /* end-to-end quadruple lines working together */
   pl_1_offset + pl__1_qlwa,     /* end-to-end quadruple lines working apart */
   pl_3_offset + pl__3_spc,      /* end-to-end split phantom columns */
   pl_3_offset + pl__3_ipc,      /* end-to-end interlocked phantom columns */
   pl_3_offset + pl__3_qcwt,     /* end-to-end quadruple columns working together */
   pl_3_offset + pl__3_qcwa,     /* end-to-end quadruple columns working apart */
   -1};

static int nice_setup_concept_3dmd[] = {
   pb_2_offset + pb__2_td,      /* triple diamonds */
   pb_2_offset + pb__2_tdwt,    /* triple diamonds working together */
   -1};

static int nice_setup_concept_4dmd[] = {
   pb_2_offset + pb__2_spd,     /* split phantom diamonds */
   pb_2_offset + pb__2_ipd,     /* interlocked phantom diamonds */
   pb_2_offset + pb__2_qd,      /* quadruple diamonds */
   pb_2_offset + pb__2_qdwt,    /* quadruple diamonds working together */
   -1};

static int nice_setup_concept_4x6[] = {
   pl_1_offset + pl__1_trtl,     /* triple twin lines */
   pl_3_offset + pl__3_trtc,     /* triple twin columns */
   -1};


nice_setup_thing nice_setup_thing_4x4 = {
   nice_setup_concept_4x4,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_4x4)};

nice_setup_thing nice_setup_thing_3x4 = {
   nice_setup_concept_3x4,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_3x4)};

nice_setup_thing nice_setup_thing_2x8 = {
   nice_setup_concept_2x8,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_2x8)};

nice_setup_thing nice_setup_thing_2x6 = {
   nice_setup_concept_2x6,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_2x6)};

nice_setup_thing nice_setup_thing_1x12 = {
   nice_setup_concept_1x12,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_1x12)};

nice_setup_thing nice_setup_thing_1x16 = {
   nice_setup_concept_1x16,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_1x16)};

nice_setup_thing nice_setup_thing_3dmd = {
   nice_setup_concept_3dmd,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_3dmd)};

nice_setup_thing nice_setup_thing_4dmd = {
   nice_setup_concept_4dmd,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_4dmd)};

nice_setup_thing nice_setup_thing_4x6 = {
   nice_setup_concept_4x6,
   (int *) 0,     /* Will be filled in during initialization. */
   sizeof(nice_setup_concept_4x6)};


int phantom_concept_index = mm_1_offset + mm__1_phan;
int matrix_2x8_concept_index = gg_offset + gg_2x8_matrix;
int cross_concept_index = mm_1_offset + mm__1_cross;
int magic_concept_index = d4_1_offset + d4__1_magic;
int intlk_concept_index = mm_1_offset + mm__1_intlk;
int left_concept_index = mm_1_offset + mm__1_left;
int grand_concept_index = mm_1_offset + mm__1_grand;


Private int phantoml_sizes[]  = {pl__1_size, pl__2_size, pl__3_size,             -1};
Private int phantomb_sizes[]  = {pb__1_size, pb__2_size, pb__3_size,             -1};
Private int tandem_sizes[]    = {tt__1_size, tt__2_size,                         -1};
Private int distort_sizes[]   = {dd__1_size, dd__2_size, dd__3_size,             -1};
Private int dist4_sizes[]     = {d4__1_size, d4__2_size, d4__3_size, d4__4_size, -1};
Private int misc_c_sizes[]    = {mm__1_size, mm__2_size, mm__3_size,             -1};

Private int phantoml_offsets[] = {pl_1_offset, pl_2_offset, pl_3_offset};
Private int phantomb_offsets[] = {pb_1_offset, pb_2_offset, pb_3_offset};
Private int tandem_offsets[]   = {tt_1_offset, tt_2_offset};
Private int distort_offsets[]  = {dd_1_offset, dd_2_offset, dd_3_offset};
Private int dist4_offsets[]    = {d4_1_offset, d4_2_offset, d4_3_offset, d4_4_offset};
Private int misc_c_offsets[]   = {mm_1_offset, mm_2_offset, mm_3_offset};

int general_concept_offset    =  gg_offset;

int general_concept_size;           /* Gets filled in during initialization. */

int *concept_offset_tables[] = {
   phantoml_offsets,
   phantomb_offsets,
   tandem_offsets,
   distort_offsets,
   dist4_offsets,
   misc_c_offsets,
   0};

int *concept_size_tables[] = {
   phantoml_sizes,
   phantomb_sizes,
   tandem_sizes,
   distort_sizes,
   dist4_sizes,
   misc_c_sizes,
   0};

Const char *concept_menu_strings[] = {
   "PHANTOM LINE/COL concepts",
   "PHANTOM BOX/DMD/QTAG concepts",
   "COUPLES/TANDEM concepts",
   "DISTORTED SETUP concepts",
   "4-PERSON DISTORTED concepts",
   "MISCELLANEOUS concepts",
   0};
