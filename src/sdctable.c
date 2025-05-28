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
   general_concept_offset
   general_concept_size
   concept_offset_tables
   concept_size_tables
   concept_menu_strings
*/

#include "sd.h"


concept_descriptor special_magic          = {"MAGIC DIAMOND,",       concept_magic,        TRUE, l_c1, {0, 1}};
concept_descriptor special_interlocked    = {"INTERLOCKED DIAMOND,", concept_interlocked,  TRUE, l_c1, {0, 1}};

concept_descriptor mark_end_of_list       = {"????",                 marker_end_of_list,   TRUE, l_dontshow};
concept_descriptor marker_decline         = {"decline???",           concept_mod_declined, TRUE, l_dontshow};
concept_descriptor marker_concept_mod     = {">>MODIFIED BY<<",      concept_another_call_next_mod, TRUE, l_dontshow, {0, 0, 0}};
concept_descriptor marker_concept_comment = {">>COMMENT<<",          concept_comment,      TRUE, l_dontshow};

callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];
int number_of_calls[NUM_CALL_LIST_KINDS];
dance_level calling_level;


/* **** BEWARE!!!!  In addition to the "size" definitions, there are numerous definitions of positions of
    various concepts for use in the "normalize" operation. */

concept_descriptor concept_descriptor_table[] = {

/* phantom concepts */

#define pl__1_size 34
#define pl__1_spl 0
#define pl__1_ipl 1
#define pl__1_pl8 10
#define pl__1_pl6 11
#define pl__1_tl 13
#define pl__1_tlwt 14
#define pl__1_tlwa 15
#define pl__1_tlwf 16
#define pl__1_tlwb 17
#define pl__1_trtl 25
#define pl__1_qlwt 28
#define pl__1_qlwa 29
#define pl__1_qlwf 30
#define pl__1_qlwb 31
   {"SPLIT PHANTOM LINES",                   concept_do_phantom_2x4,        FALSE, l_c3a, {&map_split_f, phantest_impossible, 1, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM LINES",             concept_do_phantom_2x4,        FALSE, l_c3, {&map_intlk_f, phantest_impossible, 1, MPKIND__INTLK}},
   {"PHANTOM LINES",                         concept_do_phantom_2x4,        FALSE, l_c3, {&map_full_f, phantest_first_or_both, 1, MPKIND__CONCPHAN}},
         {"", concept_comment, l_nonexistent_concept},
   {"12 MATRIX SPLIT PHANTOM LINES",         concept_do_phantom_2x3,        FALSE, l_c3x, {&map_3x4_2x3, phantest_impossible, 1}},
   {"12 MATRIX INTERLOCKED PHANTOM LINES",   concept_do_phantom_2x3,        FALSE, l_c3x, {&map_3x4_2x3_intlk, phantest_impossible, 1}},
   {"12 MATRIX PHANTOM LINES",               concept_do_phantom_2x3,        FALSE, l_c3x, {&map_3x4_2x3_conc, phantest_first_or_both, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED LINES",                         concept_divided_2x4,           FALSE, l_c4, {&map_hv_2x4_2, phantest_impossible, 1}},
   {"12 MATRIX DIVIDED LINES",               concept_divided_2x3,           FALSE, l_c4, {&map_2x6_2x3, phantest_impossible, 1}},
   {"TWO PHANTOM TIDAL LINES",               concept_do_phantom_1x8,        FALSE, l_c3, {0, phantest_impossible, TRUE, 1}},
   {"TWO PHANTOM LINES OF 6",                concept_do_phantom_1x6,        FALSE, l_c3, {0, phantest_impossible, TRUE, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE LINES",                          concept_triple_lines,          FALSE, l_c2, {0, 1}},
   {"TRIPLE LINES WORKING TOGETHER",         concept_triple_lines_tog_std,  FALSE, l_c3, {0, 10, 1}},
   {"TRIPLE LINES WORKING APART",            concept_triple_lines_tog_std,  FALSE, l_c3, {0, 11, 1}},
   {"TRIPLE LINES WORKING FORWARD",          concept_triple_lines_tog,      FALSE, l_c3, {0, 0, 1}},
   {"TRIPLE LINES WORKING BACKWARD",         concept_triple_lines_tog,      FALSE, l_c3, {0, 2, 1}},
   {"TRIPLE LINES WORKING CLOCKWISE",        concept_triple_lines_tog_std,  FALSE, l_c4, {0, 8, 1}},
   {"TRIPLE LINES WORKING COUNTERCLOCKWISE", concept_triple_lines_tog_std,  FALSE, l_c4, {0, 9, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL LINES",                 concept_triple_diag,           FALSE, l_c4, {0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING FORWARD", concept_triple_diag_together,  FALSE, l_c4, {0, 0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING BACKWARD",concept_triple_diag_together,  FALSE, l_c4, {0, 2, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN LINES",                     concept_triple_twin,           FALSE, l_c4a, {&map_4x6_2x4, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE LINES",                       concept_quad_lines,            FALSE, l_c4a, {0, 1}},
   {"QUADRUPLE LINES WORKING TOGETHER",      concept_quad_lines_tog_std,    FALSE, l_c4a, {0, 10, 1}},
   {"QUADRUPLE LINES WORKING APART",         concept_quad_lines_tog_std,    FALSE, l_c4a, {0, 11, 1}},
   {"QUADRUPLE LINES WORKING FORWARD",       concept_quad_lines_tog,        FALSE, l_c4a, {0, 0, 1}},
   {"QUADRUPLE LINES WORKING BACKWARD",      concept_quad_lines_tog,        FALSE, l_c4a, {0, 2, 1}},
   {"QUADRUPLE LINES WORKING CLOCKWISE",     concept_quad_lines_tog_std,    FALSE, l_c4, {0, 8, 1}},
   {"QUADRUPLE LINES WORKING COUNTERCLOCKWISE",concept_quad_lines_tog_std,  FALSE, l_c4, {0, 9, 1}},
/* -------- column break -------- */
#define pl__2_size 34
   {"SPLIT PHANTOM WAVES",                   concept_do_phantom_2x4,        FALSE, l_c3a, {&map_split_f, phantest_impossible, 3, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM WAVES",             concept_do_phantom_2x4,        FALSE, l_c3, {&map_intlk_f, phantest_impossible, 3, MPKIND__INTLK}},
   {"PHANTOM WAVES",                         concept_do_phantom_2x4,        FALSE, l_c3, {&map_full_f, phantest_first_or_both, 3, MPKIND__CONCPHAN}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED WAVES",                         concept_divided_2x4,           FALSE, l_c4, {&map_hv_2x4_2, phantest_impossible, 3}},
   {"12 MATRIX DIVIDED WAVES",               concept_divided_2x3,           FALSE, l_c4, {&map_2x6_2x3, phantest_impossible, 3}},
   {"TWO PHANTOM TIDAL WAVES",               concept_do_phantom_1x8,        FALSE, l_c3, {0, phantest_impossible, TRUE, 3}},
   {"TWO PHANTOM WAVES OF 6",                concept_do_phantom_1x6,        FALSE, l_c3, {0, phantest_impossible, TRUE, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE WAVES",                          concept_triple_lines,          FALSE, l_c2, {0, 3}},
   {"TRIPLE WAVES WORKING TOGETHER",         concept_triple_lines_tog_std,  FALSE, l_c3, {0, 10, 3}},
   {"TRIPLE WAVES WORKING APART",            concept_triple_lines_tog_std,  FALSE, l_c3, {0, 11, 3}},
   {"TRIPLE WAVES WORKING FORWARD",          concept_triple_lines_tog,      FALSE, l_c3, {0, 0, 3}},
   {"TRIPLE WAVES WORKING BACKWARD",         concept_triple_lines_tog,      FALSE, l_c3, {0, 2, 3}},
   {"TRIPLE WAVES WORKING CLOCKWISE",        concept_triple_lines_tog_std,  FALSE, l_c4, {0, 8, 3}},
   {"TRIPLE WAVES WORKING COUNTERCLOCKWISE", concept_triple_lines_tog_std,  FALSE, l_c4, {0, 9, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL WAVES",                 concept_triple_diag,           FALSE, l_c4, {0, 3}},
   {"TRIPLE DIAGONAL WAVES WORKING FORWARD", concept_triple_diag_together,  FALSE, l_c4, {0, 0, 3}},
   {"TRIPLE DIAGONAL WAVES WORKING BACKWARD",concept_triple_diag_together,  FALSE, l_c4, {0, 2, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN WAVES",                     concept_triple_twin,           FALSE, l_c4a, {&map_4x6_2x4, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE WAVES",                       concept_quad_lines,            FALSE, l_c4a, {0, 3}},
   {"QUADRUPLE WAVES WORKING TOGETHER",      concept_quad_lines_tog_std,    FALSE, l_c4a, {0, 10, 3}},
   {"QUADRUPLE WAVES WORKING APART",         concept_quad_lines_tog_std,    FALSE, l_c4a, {0, 11, 3}},
   {"QUADRUPLE WAVES WORKING FORWARD",       concept_quad_lines_tog,        FALSE, l_c4a, {0, 0, 3}},
   {"QUADRUPLE WAVES WORKING BACKWARD",      concept_quad_lines_tog,        FALSE, l_c4a, {0, 2, 3}},
   {"QUADRUPLE WAVES WORKING CLOCKWISE",     concept_quad_lines_tog_std,    FALSE, l_c4, {0, 8, 3}},
   {"QUADRUPLE WAVES WORKING COUNTERCLOCKWISE",concept_quad_lines_tog_std,  FALSE, l_c4, {0, 9, 3}},
/* -------- column break -------- */
#define pl__3_size 34
#define pl__3_spc 0
#define pl__3_ipc 1
#define pl__3_pc8 10
#define pl__3_pc6 11
#define pl__3_tc 13
#define pl__3_tcwt 14
#define pl__3_tcwa 15
#define pl__3_tcwr 16
#define pl__3_tcwl 17
#define pl__3_trtc 25
#define pl__3_qcwt 28
#define pl__3_qcwa 29
#define pl__3_qcwr 30
#define pl__3_qcwl 31
   {"SPLIT PHANTOM COLUMNS",                 concept_do_phantom_2x4,        FALSE, l_c3a, {&map_split_f, phantest_impossible, 0, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM COLUMNS",           concept_do_phantom_2x4,        FALSE, l_c3, {&map_intlk_f, phantest_impossible, 0, MPKIND__INTLK}},
   {"PHANTOM COLUMNS",                       concept_do_phantom_2x4,        FALSE, l_c3a, {&map_full_f, phantest_first_or_both, 0, MPKIND__CONCPHAN}},
         {"", concept_comment, l_nonexistent_concept},
   {"12 MATRIX SPLIT PHANTOM COLUMNS",       concept_do_phantom_2x3,        FALSE, l_c3x, {&map_3x4_2x3, phantest_impossible, 0}},
   {"12 MATRIX INTERLOCKED PHANTOM COLUMNS", concept_do_phantom_2x3,        FALSE, l_c3x, {&map_3x4_2x3_intlk, phantest_impossible, 0}},
   {"12 MATRIX PHANTOM COLUMNS",             concept_do_phantom_2x3,        FALSE, l_c3x, {&map_3x4_2x3_conc, phantest_first_or_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED COLUMNS",                       concept_divided_2x4,           FALSE, l_c4, {&map_hv_2x4_2, phantest_impossible, 0}},
   {"12 MATRIX DIVIDED COLUMNS",             concept_divided_2x3,           FALSE, l_c4, {&map_2x6_2x3, phantest_impossible, 0}},
   {"TWO PHANTOM TIDAL COLUMNS",             concept_do_phantom_1x8,        FALSE, l_c3, {0, phantest_impossible, TRUE, 0}},
   {"TWO PHANTOM COLUMNS OF 6",              concept_do_phantom_1x6,        FALSE, l_c3, {0, phantest_impossible, TRUE, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE COLUMNS",                        concept_triple_lines,          FALSE, l_c3, {0, 0}},
   {"TRIPLE COLUMNS WORKING TOGETHER",       concept_triple_lines_tog_std,  FALSE, l_c3, {0, 10, 0}},
   {"TRIPLE COLUMNS WORKING APART",          concept_triple_lines_tog_std,  FALSE, l_c3, {0, 11, 0}},
   {"TRIPLE COLUMNS WORKING RIGHT",          concept_triple_lines_tog,      FALSE, l_c3, {0, 2, 0}},
   {"TRIPLE COLUMNS WORKING LEFT",           concept_triple_lines_tog,      FALSE, l_c3, {0, 0, 0}},
   {"TRIPLE COLUMNS WORKING CLOCKWISE",      concept_triple_lines_tog_std,  FALSE, l_c4, {0, 8, 0}},
   {"TRIPLE COLUMNS WORKING COUNTERCLOCKWISE",concept_triple_lines_tog_std, FALSE, l_c4, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL COLUMNS",               concept_triple_diag,           FALSE, l_c4, {0, 0}},
   {"TRIPLE DIAGONAL COLUMNS WORKING RIGHT", concept_triple_diag_together,  FALSE, l_c4, {0, 3, 0}},
   {"TRIPLE DIAGONAL COLUMNS WORKING LEFT",  concept_triple_diag_together,  FALSE, l_c4, {0, 1, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN COLUMNS",                   concept_triple_twin,           FALSE, l_c4a, {&map_4x6_2x4, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE COLUMNS",                     concept_quad_lines,            FALSE, l_c4a, {0, 0}},
   {"QUADRUPLE COLUMNS WORKING TOGETHER",    concept_quad_lines_tog_std,    FALSE, l_c4a, {0, 10, 0}},
   {"QUADRUPLE COLUMNS WORKING APART",       concept_quad_lines_tog_std,    FALSE, l_c4a, {0, 11, 0}},
   {"QUADRUPLE COLUMNS WORKING RIGHT",       concept_quad_lines_tog,        FALSE, l_c4a, {0, 2, 0}},
   {"QUADRUPLE COLUMNS WORKING LEFT",        concept_quad_lines_tog,        FALSE, l_c4a, {0, 0, 0}},
   {"QUADRUPLE COLUMNS WORKING CLOCKWISE",   concept_quad_lines_tog_std,    FALSE, l_c4, {0, 8, 0}},
   {"QUADRUPLE COLUMNS WORKING COUNTERCLOCKWISE",concept_quad_lines_tog_std,FALSE, l_c4, {0, 9, 0}},
/* -------- column break -------- */
#define pb__1_size 34
#define pb__1_spb 0
#define pb__1_ipb 1
#define pb__1_tb 11
#define pb__1_tbwt 13
#define pb__1_tbwa 14
#define pb__1_tbwf 15
#define pb__1_tbwb 16
#define pb__1_tbwr 17
#define pb__1_tbwl 18
#define pb__1_qbwt 26
#define pb__1_qbwa 27
#define pb__1_qbwf 28
#define pb__1_qbwb 29
#define pb__1_qbwr 30
#define pb__1_qbwl 31
   {"SPLIT PHANTOM BOXES",                   concept_do_phantom_boxes,      FALSE, l_c3, {&map_hv_2x4_2, phantest_impossible}},
   {"INTERLOCKED PHANTOM BOXES",             concept_do_phantom_boxes,      FALSE, l_c4, {&map_intlk_phantom_box, phantest_impossible}},
   {"PHANTOM BOXES",                         concept_do_phantom_boxes,      FALSE, l_c4a, {&map_phantom_box, phantest_first_or_both}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE BOXES",                          concept_triple_boxes,          FALSE, l_c1, {0, MPKIND__SPLIT}},
   {"CONCENTRIC TRIPLE BOXES",               concept_triple_boxes,          FALSE, l_c4, {0, MPKIND__CONCPHAN}},
   {"TRIPLE BOXES WORKING TOGETHER",         concept_triple_boxes_together, FALSE, l_c3, {0, 6, 0}},
   {"TRIPLE BOXES WORKING APART",            concept_triple_boxes_together, FALSE, l_c3, {0, 7, 0}},
   {"TRIPLE BOXES WORKING FORWARD",          concept_triple_boxes_together, FALSE, l_c3, {0, 0, 0}},
   {"TRIPLE BOXES WORKING BACKWARD",         concept_triple_boxes_together, FALSE, l_c3, {0, 2, 0}},
   {"TRIPLE BOXES WORKING RIGHT",            concept_triple_boxes_together, FALSE, l_c3, {0, 3, 0}},
   {"TRIPLE BOXES WORKING LEFT",             concept_triple_boxes_together, FALSE, l_c3, {0, 1, 0}},
   {"TRIPLE BOXES WORKING CLOCKWISE",        concept_triple_boxes_together, FALSE, l_c4, {0, 8, 0}},
   {"TRIPLE BOXES WORKING COUNTERCLOCKWISE", concept_triple_boxes_together, FALSE, l_c4, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"INNER DUAL BOXES",                      concept_inner_boxes,           FALSE, l_c4},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE BOXES",                       concept_quad_boxes,            FALSE, l_c4a, {0, MPKIND__SPLIT}},
   {"CONCENTRIC QUADRUPLE BOXES",            concept_quad_boxes,            FALSE, l_c4a, {0, MPKIND__CONCPHAN}},
   {"QUADRUPLE BOXES WORKING TOGETHER",      concept_quad_boxes_together,   FALSE, l_c4a, {0, 6, 0}},
   {"QUADRUPLE BOXES WORKING APART",         concept_quad_boxes_together,   FALSE, l_c4a, {0, 7, 0}},
   {"QUADRUPLE BOXES WORKING FORWARD",       concept_quad_boxes_together,   FALSE, l_c4a, {0, 0, 0}},
   {"QUADRUPLE BOXES WORKING BACKWARD",      concept_quad_boxes_together,   FALSE, l_c4a, {0, 2, 0}},
   {"QUADRUPLE BOXES WORKING RIGHT",         concept_quad_boxes_together,   FALSE, l_c4a, {0, 3, 0}},
   {"QUADRUPLE BOXES WORKING LEFT",          concept_quad_boxes_together,   FALSE, l_c4a, {0, 1, 0}},
   {"QUADRUPLE BOXES WORKING CLOCKWISE",     concept_quad_boxes_together,   FALSE, l_c4,  {0, 8, 0}},
   {"QUADRUPLE BOXES WORKING COUNTERCLOCKWISE",concept_quad_boxes_together, FALSE, l_c4 , {0, 9, 0}},
/* -------- column break -------- */
#define pb__2_size 27
#define pb__2_spd 0
#define pb__2_ipd 1
#define pb__2_td 11
#define pb__2_tdwt 13
#define pb__2_qd 24
#define pb__2_qdwt 26
   {"SPLIT PHANTOM DIAMONDS",                concept_do_phantom_diamonds,   FALSE, l_c3x, {&map_hv_qtg_2, phantest_impossible, 0}},
   {"INTERLOCKED PHANTOM DIAMONDS",          concept_do_phantom_diamonds,   FALSE, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, 0}},
   {"PHANTOM DIAMONDS",                      concept_do_phantom_diamonds,   FALSE, l_c4, {&map_phantom_dmd, phantest_first_or_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"SPLIT PHANTOM 1/4-TAGS",                concept_do_phantom_qtags,      FALSE, l_c3x, {&map_hv_qtg_2, phantest_impossible, 1}},
   {"INTERLOCKED PHANTOM 1/4-TAGS",          concept_do_phantom_qtags,      FALSE, l_c4, {&map_intlk_phantom_dmd, phantest_impossible, 1}},
   {"PHANTOM 1/4-TAGS",                      concept_do_phantom_qtags,      FALSE, l_c4, {&map_phantom_dmd, phantest_first_or_both, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TWIN PHANTOM DIAMONDS",                 concept_do_divided_diamonds,   FALSE, l_c3x, {&map_vv_qtg_2, phantest_impossible}},
   {"TWIN PHANTOM 1/4-TAGS",                 concept_do_divided_qtags,      FALSE, l_c3x, {&map_vv_qtg_2, phantest_impossible}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAMONDS",                       concept_triple_diamonds,       FALSE, l_c3a},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAMONDS WORKING TOGETHER",    concept_triple_diamonds_together,FALSE, l_c3},
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
   {"QUADRUPLE DIAMONDS",                    concept_quad_diamonds,         FALSE, l_c4a},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE DIAMONDS WORKING TOGETHER",   concept_quad_diamonds_together,FALSE, l_c4a},

/* tandem concepts */

#define tt__1_size 14
#define tt__1_cpl 0
#define tt__1_tnd 1
#define tt__1_grc 12
#define tt__1_grt 13
   {"AS COUPLES",                            concept_tandem,                FALSE, l_a1,         {0, FALSE, 0, 0, 1}},
   {"TANDEM",                                concept_tandem,                FALSE, l_c1,         {0, FALSE, 0, 0, 0}},
   {"SIAMESE",                               concept_tandem,                FALSE, l_c1,         {0, FALSE, 0, 0, 2}},
   {"COUPLES OF THREE",                      concept_tandem,                FALSE, l_c1,         {0, FALSE, 0, 0, 5}},
   {"TANDEMS OF THREE",                      concept_tandem,                FALSE, l_c1,         {0, FALSE, 0, 0, 4}},
   {"COUPLES OF FOUR",                       concept_tandem,                FALSE, l_c1,         {0, FALSE, 0, 0, 7}},
   {"TANDEMS OF FOUR",                       concept_tandem,                FALSE, l_c1,         {0, FALSE, 0, 0, 6}},
   {"BOXES WORK SOLID",                      concept_tandem,                FALSE, l_c3,         {0, FALSE, 0, 0, 8}},
   {"DIAMONDS WORK SOLID",                   concept_tandem,                FALSE, l_c3,         {0, FALSE, 0, 0, 9}},
   {"SKEW",                                  concept_tandem,                FALSE, l_c4a,        {0, FALSE, 0, 0, 3}},
   {"<ANYONE> ARE AS COUPLES",               concept_some_are_tandem,       FALSE, l_a1,         {0, TRUE,  0, 0, 1}},
   {"<ANYONE> ARE TANDEM",                   concept_some_are_tandem,       FALSE, l_c1,         {0, TRUE,  0, 0, 0}},
   {"GRUESOME AS COUPLES",                   concept_gruesome_tandem,       FALSE, l_c4a,        {0, FALSE, 0, 2, 1}},
   {"GRUESOME TANDEM",                       concept_gruesome_tandem,       FALSE, l_c4a,        {0, FALSE, 0, 2, 0}},
/* -------- column break -------- */
#define tt__2_size 14
#define tt__2_grct 12
#define tt__2_grtt 13
   {"COUPLES TWOSOME",                       concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 1}},
   {"TANDEM TWOSOME",                        concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 0}},
   {"SIAMESE TWOSOME",                       concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 2}},
   {"COUPLES THREESOME",                     concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 5}},
   {"TANDEM THREESOME",                      concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 4}},
   {"COUPLES FOURSOME",                      concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 7}},
   {"TANDEM FOURSOME",                       concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 6}},
   {"BOXSOME",                               concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 8}},
   {"DIAMONDSOME",                           concept_tandem,                FALSE, l_c3,         {0, FALSE, 1, 0, 9}},
   {"SKEWSOME",                              concept_tandem,                FALSE, l_c4a,        {0, FALSE, 1, 0, 3}},
   {"<ANYONE> ARE COUPLES TWOSOME",          concept_some_are_tandem,       FALSE, l_c3,         {0, TRUE,  1, 0, 1}},
   {"<ANYONE> ARE TANDEM TWOSOME",           concept_some_are_tandem,       FALSE, l_c3,         {0, TRUE,  1, 0, 0}},
   {"GRUESOME TWOSOME",                      concept_gruesome_tandem,       FALSE, l_c4a,        {0, FALSE, 1, 2, 1}},
   {"GRUESOME TANDEM TWOSOME",               concept_gruesome_tandem,       FALSE, l_c4a,        {0, FALSE, 1, 2, 0}},
/* -------- column break -------- */
#define tt__3_size 14
   {"COUPLES <N/4> TWOSOME",                 concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 1}},
   {"TANDEM <N/4> TWOSOME",                  concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 0}},
   {"SIAMESE <N/4> TWOSOME",                 concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 2}},
   {"COUPLES OF THREE <N/4> THREESOME",      concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 5}},
   {"TANDEMS OF THREE <N/4> THREESOME",      concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 4}},
   {"COUPLES OF FOUR <N/4> FOURSOME",        concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 7}},
   {"TANDEMS OF FOUR <N/4> FOURSOME",        concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 6}},
   {"BOXES ARE SOLID <N/4> BOXSOME",         concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 8}},
   {"DIAMONDS ARE SOLID <N/4> DIAMONDSOME",  concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 9}},
   {"SKEW <N/4> TWOSOME",                    concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 2, 0, 3}},
   {"<ANYONE> ARE COUPLES <N/4> TWOSOME",    concept_some_are_frac_tandem,  FALSE, l_c4,         {0, TRUE,  2, 0, 1}},
   {"<ANYONE> ARE TANDEM <N/4> TWOSOME",     concept_some_are_frac_tandem,  FALSE, l_c4,         {0, TRUE,  2, 0, 0}},
   {"GRUESOME AS COUPLES <N/4> TWOSOME",     concept_gruesome_frac_tandem,  FALSE, l_c4,         {0, FALSE, 2, 2, 1}},
   {"GRUESOME TANDEM <N/4> TWOSOME",         concept_gruesome_frac_tandem,  FALSE, l_c4,         {0, FALSE, 2, 2, 0}},
/* -------- column break -------- */
#define tt__4_size 14
   {"COUPLES TWOSOME <N/4> SOLID",           concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 1}},
   {"TANDEM TWOSOME <N/4> SOLID",            concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 0}},
   {"SIAMESE TWOSOME <N/4> SOLID",           concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 2}},
   {"COUPLES THREESOME <N/4> SOLID",         concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 5}},
   {"TANDEM THREESOME <N/4> SOLID",          concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 4}},
   {"COUPLES FOURSOME <N/4> SOLID",          concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 7}},
   {"TANDEM FOURSOME <N/4> SOLID",           concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 6}},
   {"BOXSOME <N/4> SOLID",                   concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 8}},
   {"DIAMONDSOME <N/4> SOLID",               concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 9}},
   {"SKEWSOME <N/4> SOLID",                  concept_frac_tandem,           FALSE, l_c4,         {0, FALSE, 3, 0, 3}},
   {"<ANYONE> ARE COUPLES TWOSOME <N/4> SOLID",concept_some_are_frac_tandem,FALSE, l_c4,       {0, TRUE,  3, 0, 1}},
   {"<ANYONE> ARE TANDEM TWOSOME <N/4> SOLID", concept_some_are_frac_tandem,FALSE, l_c4,       {0, TRUE,  3, 0, 0}},
   {"GRUESOME TWOSOME <N/4> SOLID",          concept_gruesome_frac_tandem,  FALSE, l_c4,         {0, FALSE, 3, 2, 1}},
   {"GRUESOME TANDEM TWOSOME <N/4> SOLID",   concept_gruesome_frac_tandem,  FALSE, l_c4,         {0, FALSE, 3, 2, 0}},

/* distorted concepts */

#define dd__1_size 10
   {"STAGGER",                               concept_do_phantom_2x4,        FALSE, l_c2,         {&map_stagger, phantest_only_one, 0, MPKIND__NONE}},
   {"STAIRSTEP COLUMNS",                     concept_do_phantom_2x4,        FALSE, l_c4,         {&map_stairst, phantest_only_one, 0, MPKIND__NONE}},
   {"LADDER COLUMNS",                        concept_do_phantom_2x4,        FALSE, l_c4,         {&map_ladder, phantest_only_one, 0, MPKIND__NONE}},
   {"OFFSET COLUMNS",                        concept_distorted,             FALSE, l_c2,         {0, disttest_offset, 0}},
   {"\"Z\" COLUMNS",                         concept_distorted,             FALSE, l_c3,         {0, disttest_z, 0}},
   {"DISTORTED COLUMNS",                     concept_distorted,             FALSE, l_c3,         {0, disttest_any, 0}},
   {"<ANYONE> IN A DIAGONAL COLUMN",         concept_single_diagonal,       FALSE, l_c3,         {0, 0}},
   {"TWO DIAGONAL COLUMNS",                  concept_double_diagonal,       FALSE, l_c3,         {0, 0}},
   {"\"O\"",                                 concept_do_phantom_2x4,        FALSE, l_c1,         {&map_o_s2x4_3, phantest_only_first_one, 0, MPKIND__NONE}},
   {"BUTTERFLY",                             concept_do_phantom_2x4,        FALSE, l_c1,         {&map_x_s2x4_3, phantest_only_second_one, 0, MPKIND__NONE}},
/* -------- column break -------- */
#define dd__2_size 8
   {"BIGBLOCK",                              concept_do_phantom_2x4,        FALSE, l_c3a,        {&map_stagger, phantest_only_one, 1, MPKIND__NONE}},
   {"STAIRSTEP LINES",                       concept_do_phantom_2x4,        FALSE, l_c4,         {&map_ladder, phantest_only_one, 1, MPKIND__NONE}},
   {"LADDER LINES",                          concept_do_phantom_2x4,        FALSE, l_c4,         {&map_stairst, phantest_only_one, 1, MPKIND__NONE}},
   {"OFFSET LINES",                          concept_distorted,             FALSE, l_c2,         {0, disttest_offset, 1}},
   {"\"Z\" LINES",                           concept_distorted,             FALSE, l_c3,         {0, disttest_z, 1}},
   {"DISTORTED LINES",                       concept_distorted,             FALSE, l_c3,         {0, disttest_any, 1}},
   {"<ANYONE> IN A DIAGONAL LINE",           concept_single_diagonal,       FALSE, l_c3,         {0, 1}},
   {"TWO DIAGONAL LINES",                    concept_double_diagonal,       FALSE, l_c3,         {0, 1}},
/* -------- column break -------- */
#define dd__3_size 9
#define dd__3_pofc 3
#define dd__3_pob 8
   {"PHANTOM STAGGER COLUMNS",               concept_do_phantom_2x4,        FALSE, l_c4,         {&map_stagger, phantest_both, 0, MPKIND__NONE}},
   {"PHANTOM STAIRSTEP COLUMNS",             concept_do_phantom_2x4,        FALSE, l_c4,         {&map_stairst, phantest_both, 0, MPKIND__NONE}},
   {"PHANTOM LADDER COLUMNS",                concept_do_phantom_2x4,        FALSE, l_c4,         {&map_ladder, phantest_both, 0, MPKIND__NONE}},
   {"PHANTOM OFFSET COLUMNS",                concept_do_phantom_2x4,        FALSE, l_c4a,        {&map_offset, phantest_both, 0, MPKIND__NONE}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"PHANTOM BUTTERFLY OR \"O\"",            concept_do_phantom_2x4,        FALSE, l_c4a,        {&map_but_o, phantest_both, 0, MPKIND__NONE}},
/* -------- column break -------- */
#define dd__4_size 4
#define dd__4_pofl 3
   {"PHANTOM BIGBLOCK LINES",                concept_do_phantom_2x4,        FALSE, l_c4,         {&map_stagger, phantest_both, 1, MPKIND__NONE}},
   {"PHANTOM STAIRSTEP LINES",               concept_do_phantom_2x4,        FALSE, l_c4,         {&map_ladder, phantest_both, 1, MPKIND__NONE}},
   {"PHANTOM LADDER LINES",                  concept_do_phantom_2x4,        FALSE, l_c4,         {&map_stairst, phantest_both, 1, MPKIND__NONE}},
   {"PHANTOM OFFSET LINES",                  concept_do_phantom_2x4,        FALSE, l_c4a,        {&map_offset, phantest_both, 1, MPKIND__NONE}},

/* 4-person distorted concepts */

#define d4__1_size 6
   {"SPLIT",                                 concept_split,                 FALSE, l_mainstream},
   {"ONCE REMOVED",                          concept_once_removed,          FALSE, l_c2,         {0, 0}},
   {"ONCE REMOVED DIAMONDS",                 concept_once_removed,          FALSE, l_c3,         {0, 1}},
   {"MAGIC",                                 concept_magic,                 FALSE, l_c1},
   {"DIAGONAL",                              concept_do_both_boxes,         FALSE, l_c3a,        {&map_2x4_diagonal, 97, FALSE}},
   {"TRAPEZOID",                             concept_do_both_boxes,         FALSE, l_c3,         {&map_2x4_trapezoid, 97, FALSE}},
/* -------- column break -------- */
#define d4__2_size 5
   {"INTERLOCKED PARALLELOGRAM",             concept_do_both_boxes,         FALSE, l_c3x,        {&map_2x4_int_pgram, 97, TRUE}},
   {"INTERLOCKED BOXES",                     concept_misc_distort,          FALSE, l_c3x,        {0, 3, 0}},
   {"TWIN PARALLELOGRAMS",                   concept_misc_distort,          FALSE, l_c3x,        {0, 2, 0}},
   {"EACH \"Z\"",                            concept_misc_distort,          FALSE, l_c3a,        {0, 0, 0}},
   {"INTERLOCKED \"Z's\"",                   concept_misc_distort,          FALSE, l_c3a,        {0, 0, 8}},
/* -------- column break -------- */
#define d4__3_size 6
   {"JAY",                                   concept_misc_distort,          FALSE, l_c3a,        {0, 1, 0}},
   {"BACK-TO-FRONT JAY",                     concept_misc_distort,          FALSE, l_c3a,        {0, 1, 8}},
   {"BACK-TO-BACK JAY",                      concept_misc_distort,          FALSE, l_c3a,        {0, 1, 16}},
   {"FACING PARALLELOGRAM",                  concept_misc_distort,          FALSE, l_c3a,        {0, 4, 0}},
   {"BACK-TO-FRONT PARALLELOGRAM",           concept_misc_distort,          FALSE, l_c3a,        {0, 4, 8}},
   {"BACK-TO-BACK PARALLELOGRAM",            concept_misc_distort,          FALSE, l_c3a,        {0, 4, 16}},
/* -------- column break -------- */
#define d4__4_size 4
#define d4__4_pibl 1
   {"IN YOUR BLOCKS",                        concept_do_phantom_2x2,        FALSE, l_c1,         {&map_blocks, phantest_2x2_only_two}},
   {"4 PHANTOM INTERLOCKED BLOCKS",          concept_do_phantom_2x2,        FALSE, l_c4a,        {&map_blocks, phantest_2x2_both}},
   {"TRIANGULAR BOXES",                      concept_do_phantom_2x2,        FALSE, l_c4,         {&map_trglbox, phantest_2x2_only_two}},
   {"4 PHANTOM TRIANGULAR BOXES",            concept_do_phantom_2x2,        FALSE, l_c4,         {&map_trglbox, phantest_2x2_both}},

/* Miscellaneous concepts */

#define mm__1_size 46
#define mm__1_phan 34
   {"LEFT",                                  concept_left,                  FALSE, l_mainstream},
   {"REVERSE",                               concept_reverse,               FALSE, l_mainstream},
   {"CROSS",                                 concept_cross,                 FALSE, l_mainstream},
   {"SINGLE",                                concept_single,                FALSE, l_mainstream},
   {"SINGLE FILE",                           concept_singlefile,            FALSE, l_mainstream},  /* Not C3X?  No, mainstream, for dixie style. */
   {"GRAND",                                 concept_grand,                 FALSE, l_plus},
   {"MIRROR",                                concept_mirror,                FALSE, l_c2},  /* C2 is about where people become aware of taking right hands. */
         {"", concept_comment, l_nonexistent_concept},
   {"TRIANGLE",                              concept_triangle,              FALSE, l_c1},
   {"DIAMOND",                               concept_diamond,               FALSE, l_c3x},
   {"INTERLOCKED",                           concept_interlocked,           FALSE, l_c1},
   {"12 MATRIX",                             concept_12_matrix,             FALSE, l_c3x},
   {"16 MATRIX",                             concept_16_matrix,             FALSE, l_c3x},
   {"FUNNY",                                 concept_funny,                 FALSE, l_c2},
   {"ASSUME WAVES",                          concept_assume_waves,          FALSE, l_c3,        {0, cr_wave_only,  0, 0}},
   {"ASSUME MINIWAVES",                      concept_assume_waves,          FALSE, l_c3,        {0, cr_wave_only,  2, 0}},
   {"ASSUME TWO-FACED LINES",                concept_assume_waves,          FALSE, l_c3,        {0, cr_2fl_only,   0, 0}},
   {"ASSUME ONE-FACED LINES",                concept_assume_waves,          FALSE, l_c3,        {0, cr_1fl_only,   0, 0}},
   {"ASSUME INVERTED LINES",                 concept_assume_waves,          FALSE, l_c3,        {0, cr_magic_only, 0, 0}},
   {"ASSUME INVERTED BOXES",                 concept_assume_waves,          FALSE, l_c3,        {0, cr_magic_only, 2, 0}},
   {"ASSUME NORMAL COLUMNS",                 concept_assume_waves,          FALSE, l_c3,        {0, cr_wave_only,  1, 0}},
   {"ASSUME MAGIC COLUMNS",                  concept_assume_waves,          FALSE, l_c3,        {0, cr_magic_only, 1, 0}},
   {"ASSUME EIGHT CHAIN",                    concept_assume_waves,          FALSE, l_c3,        {0, cr_li_lo,      1, 1}},
   {"ASSUME TRADE BY",                       concept_assume_waves,          FALSE, l_c3,        {0, cr_li_lo,      1, 2}},
   {"ASSUME DPT",                            concept_assume_waves,          FALSE, l_c3,        {0, cr_2fl_only,   1, 1}},
   {"ASSUME CDPT",                           concept_assume_waves,          FALSE, l_c3,        {0, cr_2fl_only,   1, 2}},
   {"ASSUME FACING LINES",                   concept_assume_waves,          FALSE, l_c3,        {0, cr_li_lo,      0, 2}},
   {"ASSUME BACK-TO-BACK LINES",             concept_assume_waves,          FALSE, l_c3,        {0, cr_li_lo,      0, 1}},
   {"ASSUME NORMAL CASTS",                   concept_assume_waves,          FALSE, l_c3,        {0, cr_alwaysfail, 0, 0}},
   {"WITH ACTIVE PHANTOMS",                  concept_active_phantoms,       FALSE, l_c4},
   {"CENTRAL",                               concept_central,               FALSE, l_c3},
   {"FAN",                                   concept_fan_or_yoyo,           FALSE, l_c3,        {0, 0}},
   {"YO-YO",                                 concept_fan_or_yoyo,           FALSE, l_c4,        {0, 1}},
   {"PARALLELOGRAM",                         concept_parallelogram,         FALSE, l_c2},
   {"PHANTOM",                               concept_c1_phantom,            FALSE, l_c1},
   {"MATRIX",                                concept_matrix,                FALSE, l_c4},
   {"<ANYONE> ARE STANDARD IN",              concept_standard,              FALSE, l_c4a,        {0, 0}},
   {"STABLE",                                concept_stable,                FALSE, l_c3a,        {0, FALSE, FALSE}},
   {"<ANYONE> ARE STABLE",                   concept_so_and_so_stable,      FALSE, l_c3a,        {0, TRUE,  FALSE}},
   {"<N/4> STABLE",                          concept_frac_stable,           FALSE, l_c4,         {0, FALSE, TRUE}},
   {"<ANYONE> ARE <N/4> STABLE",             concept_so_and_so_frac_stable, FALSE, l_c4,         {0, TRUE,  TRUE}},
   {"TRACE",                                 concept_trace,                 FALSE, l_c3x},
   {"STRETCH",                               concept_old_stretch,           FALSE, l_c1},
   {"STRETCHED SETUP",                       concept_new_stretch,           FALSE, l_c2},
   {"FERRIS",                                concept_ferris,                FALSE, l_c3x,        {0, 0}},
   {"RELEASE",                               concept_ferris,                FALSE, l_c3a,        {0, 1}},
/* -------- column break -------- */
#define mm__2_size 47
   {"CENTERS",                               concept_centers_or_ends,       FALSE, l_mainstream, {0, 0}},
   {"ENDS",                                  concept_centers_or_ends,       FALSE, l_mainstream, {0, 1}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,      FALSE, l_mainstream, {0, 0}},
   {"CENTER 6",                              concept_centers_or_ends,       FALSE, l_mainstream, {0, 2}},
   {"OUTER 6",                               concept_centers_or_ends,       FALSE, l_mainstream, {0, 5}},
   {"CENTER 2",                              concept_centers_or_ends,       FALSE, l_mainstream, {0, 4}},
   {"OUTER 2",                               concept_centers_or_ends,       FALSE, l_mainstream, {0, 3}},
   {"CENTER 6/OUTER 2",                      concept_centers_and_ends,      FALSE, l_mainstream, {0, 2}},
   {"CENTER 2/OUTER 6",                      concept_centers_and_ends,      FALSE, l_mainstream, {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"CHECKPOINT",                            concept_checkpoint,            FALSE, l_c2,         {0, 0}},
   {"REVERSE CHECKPOINT",                    concept_checkpoint,            FALSE, l_c3,         {0, 1}},
   {"CHECKERBOARD",                          concept_checkerboard,          FALSE, l_c3a,        {0, s1x4}},
   {"CHECKERBOX",                            concept_checkerboard,          FALSE, l_c3a,        {0, s2x2}},
   {"CHECKERDIAMOND",                        concept_checkerboard,          FALSE, l_c3x,        {0, sdmd}},
         {"", concept_comment, l_nonexistent_concept},
   {"<ANYONE> ONLY",                         concept_so_and_so_only,        FALSE, l_mainstream, {0, 4}},
   {"<ANYONE> ONLY (while the others)",      concept_some_vs_others,        FALSE, l_mainstream, {0, 5}},
   {"<ANYONE> DISCONNECTED",                 concept_so_and_so_only,        FALSE, l_c2, {0, 6}},
   {"<ANYONE> DISCONNECTED (while the others)", concept_some_vs_others,     FALSE, l_c2, {0, 7}},
   {"<ANYONE> DO YOUR PART,",                concept_so_and_so_only,        FALSE, l_mainstream, {0, 0}},
   {"<ANYONE> DO YOUR PART, (while the others)", concept_some_vs_others,    FALSE, l_mainstream, {0, 1}},
   {"ON YOUR OWN",                           concept_on_your_own,           FALSE, l_c4a},
   {"OWN THE <ANYONE>",                      concept_some_vs_others,        FALSE, l_c3a,        {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"two calls in succession",               concept_sequential,            FALSE, l_mainstream},
   {"FOLLOW IT BY",                          concept_special_sequential,    FALSE, l_c2,        {0, 0}},
   {"PRECEDE IT BY",                         concept_special_sequential,    FALSE, l_c2,        {0, 1}},
   {"CRAZY",                                 concept_crazy,                 FALSE, l_c2,        {0, 0, FALSE}},
   {"REVERSE CRAZY",                         concept_crazy,                 FALSE, l_c3x,       {0, 1, FALSE}},
   {"<N/4> CRAZY",                           concept_frac_crazy,            FALSE, l_c2,        {0, 0, TRUE}},
   {"<N/4> REVERSE CRAZY",                   concept_frac_crazy,            FALSE, l_c3x,       {0, 1, TRUE}},
   {"DO IT TWICE:",                          concept_twice,                 FALSE, l_mainstream},
   {"RANDOM",                                concept_meta,                  FALSE, l_c3a,        {0, 0}},
   {"REVERSE RANDOM",                        concept_meta,                  FALSE, l_c3x,        {0, 1}},
   {"PIECEWISE",                             concept_meta,                  FALSE, l_c3x,        {0, 2}},
   {"<N>/<N> (fractional)",                  concept_fractional,            FALSE, l_c1,         {0, 0}},
   {"DO THE LAST <N>/<N>:",                  concept_fractional,            FALSE, l_c1,         {0, 1}},
   {"REVERSE ORDER",                         concept_meta,                  FALSE, l_c3x,        {0, 5}},
   {"INTERLACE",                             concept_interlace,             FALSE, l_c3x},
   {"START <concept>",                       concept_meta,                  FALSE, l_c2,         {0, 3}},
   {"FINISH",                                concept_meta,                  FALSE, l_c2,         {0, 4}},
   {"<ANYONE> START",                        concept_so_and_so_begin,       FALSE, l_c2,         {0, 0}},
   {"SKIP THE <Nth> PART",                   concept_nth_part,              FALSE, l_c2,         {0, 1}},
   {"DO THE <Nth> PART <concept>",           concept_nth_part,              FALSE, l_c2,         {0, 0}},
   {"REPLACE THE <Nth> PART",                concept_replace_nth_part,      FALSE, l_c2,         {0, 0}},
   {"INTERRUPT AFTER THE <Nth> PART",        concept_replace_nth_part,      FALSE, l_c2,         {0, 1}},
/* -------- column break -------- */
#define mm__3_size 45
#define mm__3_3x3 39
#define mm__3_4x4 40
   {"IN POINT TRIANGLE",                     concept_randomtrngl,           FALSE, l_c1,         {0, 1}},
   {"OUT POINT TRIANGLE",                    concept_randomtrngl,           FALSE, l_c1,         {0, 0}},
   {"INSIDE TRIANGLES",                      concept_randomtrngl,           FALSE, l_c1,         {0, 2}},
   {"OUTSIDE TRIANGLES",                     concept_randomtrngl,           FALSE, l_c1,         {0, 3}},
   {"TALL 6",                                concept_randomtrngl,           FALSE, l_c3x,        {0, 4}},
   {"SHORT 6",                               concept_randomtrngl,           FALSE, l_c3x,        {0, 5}},
   {"WAVE-BASE TRIANGLES",                   concept_randomtrngl,           FALSE, l_c1,         {0, 6}},
   {"TANDEM-BASE TRIANGLES",                 concept_randomtrngl,           FALSE, l_c1,         {0, 7}},
   {"<ANYONE>-BASED TRIANGLE",               concept_selbasedtrngl,         FALSE, l_c1,         {0, 20}},
         {"", concept_comment, l_nonexistent_concept},
   {"CONCENTRIC",                            concept_concentric,            FALSE, l_c1,         {0, schema_concentric}},
   {"CROSS CONCENTRIC",                      concept_concentric,            FALSE, l_c2,         {0, schema_cross_concentric}},
   {"SINGLE CONCENTRIC",                     concept_single_concentric,     FALSE, l_c4,         {0, schema_single_concentric}},
   {"SINGLE CROSS CONCENTRIC",               concept_single_concentric,     FALSE, l_c4,         {0, schema_single_cross_concentric}},
         {"", concept_comment, l_nonexistent_concept},
   {"GRAND WORKING FORWARD",                 concept_grand_working,         FALSE, l_c3x,        {0, 0}},
   {"GRAND WORKING BACKWARD",                concept_grand_working,         FALSE, l_c3x,        {0, 2}},
   {"GRAND WORKING LEFT",                    concept_grand_working,         FALSE, l_c3x,        {0, 1}},
   {"GRAND WORKING RIGHT",                   concept_grand_working,         FALSE, l_c3x,        {0, 3}},
   {"GRAND WORKING AS CENTERS",              concept_grand_working,         FALSE, l_c3x,        {0, 8}},
   {"GRAND WORKING AS ENDS",                 concept_grand_working,         FALSE, l_c3x,        {0, 9}},
         {"", concept_comment, l_nonexistent_concept},
   {"<ANYONE> ARE CENTERS OF A DOUBLE-OFFSET 1/4-TAG",concept_double_offset,FALSE, l_c4,      {0, 0}},
   {"<ANYONE> ARE CENTERS OF A DOUBLE-OFFSET 3/4-TAG",concept_double_offset,FALSE, l_c4,      {0, 1}},
   {"<ANYONE> ARE CENTERS OF A DOUBLE-OFFSET THING",concept_double_offset,  FALSE, l_c4,      {0, 2}},
   {"<ANYONE> ARE CENTERS OF DOUBLE-OFFSET DIAMONDS",concept_double_offset, FALSE, l_c4,        {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"INRIGGER",                              concept_rigger,                FALSE, l_c3x,        {0, 2}},
   {"OUTRIGGER",                             concept_rigger,                FALSE, l_c3x,        {0, 0}},
   {"LEFTRIGGER",                            concept_rigger,                FALSE, l_c3x,        {0, 1}},
   {"RIGHTRIGGER",                           concept_rigger,                FALSE, l_c3x,        {0, 3}},
   {"BACKRIGGER",                            concept_rigger,                FALSE, l_c3x,        {0, 16}},
   {"FRONTRIGGER",                           concept_rigger,                FALSE, l_c3x,        {0, 18}},
         {"", concept_comment, l_nonexistent_concept},
   {"1X2",                                   concept_1x2,                   FALSE, l_c3},
   {"2X1",                                   concept_2x1,                   FALSE, l_c3},
   {"2X2",                                   concept_2x2,                   FALSE, l_c3a},
   {"1X3",                                   concept_1x3,                   FALSE, l_c2},
   {"3X1",                                   concept_3x1,                   FALSE, l_c2},
   {"3X3",                                   concept_3x3,                   FALSE, l_c3x},
   {"4X4",                                   concept_4x4,                   FALSE, l_c3x},
         {"", concept_comment, l_nonexistent_concept},
   {"ALL 4 COUPLES",                         concept_all_8,                 FALSE, l_a2,         {0, 0}},
   {"ALL 8",                                 concept_all_8,                 FALSE, l_c1,         {0, 1}},
   {"ALL 8 (diamonds)",                      concept_all_8,                 FALSE, l_c1,         {0, 2}},

/* general concepts */

   {"CENTERS",                               concept_centers_or_ends,       TRUE,  l_mainstream, {0, 0}},
   {"ENDS",                                  concept_centers_or_ends,       TRUE,  l_mainstream, {0, 1}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,      TRUE,  l_mainstream, {0, 0}},
   {"LEFT",                                  concept_left,                  TRUE,  l_mainstream},
   {"REVERSE",                               concept_reverse,               TRUE,  l_mainstream},
   {"SPLIT",                                 concept_split,                 TRUE,  l_mainstream},
   {"EACH 1X4",                              concept_each_1x4,              FALSE, l_mainstream},
   {"CROSS",                                 concept_cross,                 TRUE,  l_mainstream},
   {"GRAND",                                 concept_grand,                 TRUE,  l_plus},
   {"MAGIC",                                 concept_magic,                 TRUE,  l_c1},
   {"INTERLOCKED",                           concept_interlocked,           TRUE,  l_c1},
   {"12 MATRIX",                             concept_12_matrix,             TRUE,  l_c3x},
   {"16 MATRIX",                             concept_16_matrix,             TRUE,  l_c3x},
   {"1X12 MATRIX",                           concept_1x12_matrix,           FALSE, l_c3x},
   {"1X16 MATRIX",                           concept_1x16_matrix,           FALSE, l_c3x},
   {"2X6 MATRIX",                            concept_2x6_matrix,            FALSE, l_c3x},
   {"2X8 MATRIX",                            concept_2x8_matrix,            FALSE, l_c3x},
   {"3X4 MATRIX",                            concept_3x4_matrix,            FALSE, l_c3x},
   {"4X4 MATRIX",                            concept_4x4_matrix,            FALSE, l_c3x},
   {"16 MATRIX OF PARALLEL DIAMONDS",        concept_4dmd_matrix,           FALSE, l_c3x},
   {"PHANTOM",                               concept_c1_phantom,            TRUE,  l_c1},
   {"FUNNY",                                 concept_funny,                 TRUE,  l_c2},
   {"SINGLE",                                concept_single,                TRUE,  l_mainstream},
   {"CONCENTRIC",                            concept_concentric,            TRUE,  l_c1,         {0, schema_concentric}},
   {"AS COUPLES",                            concept_tandem,                TRUE,  l_a1,         {0, FALSE, FALSE, 0, 1}},
   {"TANDEM",                                concept_tandem,                TRUE,  l_c1,         {0, FALSE, FALSE, 0, 0}},
   {"DIAGNOSE",                              concept_diagnose,              FALSE, l_mainstream},  /* Only appears if "-diagnostic" given. */
   {"???",                                   marker_end_of_list}};

/* **** BEWARE!!!!  These things are keyed to "concept_descriptor_table"
   (above) and tell how to divide things into the various menus and popups.
   BE CAREFUL!! */

#define pl__tot_size (pl__1_size+pl__2_size+pl__3_size)
#define pb__tot_size (pb__1_size+pb__2_size)
#define tt__tot_size (tt__1_size+tt__2_size+tt__3_size+tt__4_size)
#define dd__tot_size (dd__1_size+dd__2_size+dd__3_size+dd__4_size)
#define d4__tot_size (d4__1_size+d4__2_size+d4__3_size+d4__4_size)

#define pl_1_offset (0)
#define pl_2_offset (0 + pl__1_size)
#define pl_3_offset (0 + pl__1_size + pl__2_size)

#define pb_1_offset (0 + pl__tot_size)
#define pb_2_offset (0 + pl__tot_size + pb__1_size)

#define tt_1_offset (0 + pl__tot_size + pb__tot_size)
#define tt_2_offset (0 + pl__tot_size + pb__tot_size + tt__1_size)
#define tt_3_offset (0 + pl__tot_size + pb__tot_size + tt__1_size + tt__2_size)
#define tt_4_offset (0 + pl__tot_size + pb__tot_size + tt__1_size + tt__2_size + tt__3_size)

#define dd_1_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size)
#define dd_2_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__1_size)
#define dd_3_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__1_size + dd__2_size)
#define dd_4_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__1_size + dd__2_size + dd__3_size)

#define d4_1_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size)
#define d4_2_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__1_size)
#define d4_3_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__1_size + d4__2_size)
#define d4_4_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__1_size + d4__2_size + d4__3_size)

#define mm_1_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size)
#define mm_2_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size)
#define mm_3_offset (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size + mm__2_size)

#define gg_offset   (0 + pl__tot_size + pb__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size + mm__2_size + mm__3_size)


static int nice_setup_concept_4x4[] = {
   pl_1_offset + pl__1_spl,     /* split phantom lines */
   pl_1_offset + pl__1_ipl,     /* interlocked phantom lines */
   pl_1_offset + pl__1_qlwf,    /* quadruple lines working forward */
   pl_1_offset + pl__1_qlwb,    /* quadruple lines working backward */
   pl_3_offset + pl__3_spc,     /* split phantom columns */
   pl_3_offset + pl__3_ipc,     /* interlocked phantom columns */
   pl_3_offset + pl__3_qcwr,    /* quadruple columns working right */
   pl_3_offset + pl__3_qcwl,    /* quadruple columns working left */
   dd_3_offset + dd__3_pob,     /* phantom "O" or butterfly */
   dd_3_offset + dd__3_pofc,    /* phantom offset columns */
   dd_4_offset + dd__4_pofl,    /* phantom offset lines */
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
   pl_1_offset + pl__1_pl8,     /* phantom tidal lines */
   pl_3_offset + pl__3_pc8,     /* phantom tidal columns */
   /* The less likely ones: */
   mm_3_offset + mm__3_4x4,
   tt_1_offset + tt__1_grc,     /* gruesome as couples */
   tt_1_offset + tt__1_grt,     /* gruesome tandem */
   tt_2_offset + tt__2_grct,    /* gruesome (as couples) twosome */
   tt_2_offset + tt__2_grtt,    /* gruesome tandem twosome */
   -1};

static int nice_setup_concept_2x6[] = {
   pb_1_offset + pb__1_tb,      /* triple boxes */
   pb_1_offset + pb__1_tbwf,    /* triple boxes working forward */
   pb_1_offset + pb__1_tbwb,    /* triple boxes working backward */
   pb_1_offset + pb__1_tbwr,    /* triple boxes working right */
   pb_1_offset + pb__1_tbwl,    /* triple boxes working left */
   pb_1_offset + pb__1_tbwt,    /* triple boxes working together */
   pb_1_offset + pb__1_tbwa,    /* triple boxes working apart */
   pl_1_offset + pl__1_pl6,     /* phantom lines of 6 */
   pl_3_offset + pl__3_pc6,     /* phantom columns of 6 */
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


Private int phantoml_sizes[]  = {pl__1_size, pl__2_size, pl__3_size,             -1};
Private int phantomb_sizes[]  = {pb__1_size, pb__2_size,                         -1};
Private int tandem_sizes[]    = {tt__1_size, tt__2_size, tt__3_size, tt__4_size, -1};
Private int distort_sizes[]   = {dd__1_size, dd__2_size, dd__3_size, dd__4_size, -1};
Private int dist4_sizes[]     = {d4__1_size, d4__2_size, d4__3_size, d4__4_size, -1};
Private int misc_c_sizes[]    = {mm__1_size, mm__2_size, mm__3_size,             -1};

Private int phantoml_offsets[] = {pl_1_offset, pl_2_offset, pl_3_offset};
Private int phantomb_offsets[] = {pb_1_offset, pb_2_offset};
Private int tandem_offsets[]   = {tt_1_offset, tt_2_offset, tt_3_offset, tt_4_offset};
Private int distort_offsets[]  = {dd_1_offset, dd_2_offset, dd_3_offset, dd_4_offset};
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
   "PHANTOM BOX/DMD concepts",
   "COUPLES/TANDEM concepts",
   "DISTORTED SETUP concepts",
   "4-PERSON DISTORTED concepts",
   "MISCELLANEOUS concepts",
   0};
