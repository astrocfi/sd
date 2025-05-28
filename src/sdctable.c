/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993  William B. Ackerman.

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

    This is for version 30. */

/* This defines the following external variables:
   special_magic
   special_interlocked
   mark_end_of_list
   marker_decline
   marker_concept_mod
   marker_concept_modreact
   marker_concept_modtag
   marker_concept_force
   marker_concept_plain
   marker_concept_second
   marker_concept_secondreact
   marker_concept_secondtag
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


concept_descriptor special_magic = {"MAGIC DIAMOND,", concept_magic, l_dontshow, {0, 1}};
concept_descriptor special_interlocked = {"INTERLOCKED DIAMOND,", concept_interlocked, l_dontshow, {0, 1}};


concept_descriptor mark_end_of_list = {"????", marker_end_of_list, l_dontshow};
concept_descriptor marker_decline = {"decline???", concept_mod_declined, l_dontshow};
concept_descriptor marker_concept_mod = {">>MODIFIED BY<<", concept_another_call_next_mod, l_dontshow, {0, 0, 0}};
concept_descriptor marker_concept_modreact = {">>REACTION MODIFIED BY<<", concept_another_call_next_modreact, l_dontshow, {0, 0, 1}};
concept_descriptor marker_concept_modtag = {">>TAG ENDING MODIFIED BY<<", concept_another_call_next_modtag, l_dontshow, {0, 0, 2}};
concept_descriptor marker_concept_force = {">>FORCIBLY MODIFIED BY<<", concept_another_call_next_force, l_dontshow, {0, 0, 0}};
concept_descriptor marker_concept_plain = {">>PLAINLY MODIFIED BY<<", concept_another_call_next_plain, l_dontshow, {0, 0, 0}};
concept_descriptor marker_concept_second = {">>2nd MODIFIED BY<<", concept_another_call_next_2nd, l_dontshow, {0, 1, 0}};
concept_descriptor marker_concept_secondreact = {">>2nd REACTION MODIFIED BY<<", concept_another_call_next_2ndreact, l_dontshow, {0, 1, 1}};
concept_descriptor marker_concept_secondtag = {">>2nd TAG ENDING MODIFIED BY<<", concept_another_call_next_2ndtag, l_dontshow, {0, 1, 2}};

concept_descriptor marker_concept_comment = {">>COMMENT<<", concept_comment, l_dontshow};

callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];
int number_of_calls[NUM_CALL_LIST_KINDS];
dance_level calling_level;


/* **** BEWARE!!!!  In addition to the "size" definitions, there are numerous definitions of positions of
    various concepts for use in the "nice setup" operation. */

concept_descriptor concept_descriptor_table[] = {

/* phantom concepts */

#define pp__1_size 44
#define pp__1_spl 0
#define pp__1_ipl 1
#define pp__1_espl 3
#define pp__1_eipl 4
#define pp__1_pl8 12
#define pp__1_pl6 13
#define pp__1_tl 15
#define pp__1_etlwt 17
#define pp__1_etlwa 18
#define pp__1_tlwf 19
#define pp__1_tlwb 20
#define pp__1_trtl 32
#define pp__1_eqlwt 36
#define pp__1_eqlwa 37
#define pp__1_qlwf 38
#define pp__1_qlwb 39
   {"SPLIT PHANTOM LINES",                   concept_do_phantom_2x4,        l_c3a, {&map_split_f, phantest_impossible, 1, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM LINES",             concept_do_phantom_2x4,        l_c3, {&map_intlk_f, phantest_impossible, 1, MPKIND__INTLK}},
   {"PHANTOM LINES",                         concept_do_phantom_2x4,        l_c3, {&map_full_f, phantest_first_or_both, 1, MPKIND__CONCPHAN}},
   {"END-TO-END SPLIT PHANTOM LINES",        concept_do_phantom_endtoend,   l_c3a, {0, phantest_impossible, 1, MPKIND__SPLIT}},
   {"END-TO-END INTERLOCKED PHANTOM LINES",  concept_do_phantom_endtoend,   l_c3, {0, phantest_impossible, 1, MPKIND__INTLK}},
   {"END-TO-END PHANTOM LINES",              concept_do_phantom_endtoend,   l_c3, {0, phantest_first_or_both, 1, MPKIND__CONCPHAN}},
   {"12 MATRIX SPLIT PHANTOM LINES",         concept_do_phantom_2x3,        l_c3x, {&map_3x4_2x3, phantest_impossible, 1}},
   {"12 MATRIX INTERLOCKED PHANTOM LINES",   concept_do_phantom_2x3,        l_c3x, {&map_3x4_2x3_intlk, phantest_impossible, 1}},
   {"12 MATRIX PHANTOM LINES",               concept_do_phantom_2x3,        l_c3x, {&map_3x4_2x3_conc, phantest_first_or_both, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED LINES",                         concept_divided_2x4,           l_c4, {&map_hv_2x4_2, phantest_impossible, 1}},
   {"12 MATRIX DIVIDED LINES",               concept_divided_2x3,           l_c4, {&map_2x6_2x3, phantest_impossible, 1}},
   {"PHANTOM TIDAL LINES",                   concept_do_phantom_1x8,        l_c3, {0, phantest_impossible, TRUE, 1}},
   {"PHANTOM LINES OF 6",                    concept_do_phantom_1x6,        l_c3, {0, phantest_impossible, TRUE, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE LINES",                          concept_triple_lines,          l_c2, {0, 1}},
   {"END-TO-END TRIPLE LINES",               concept_triple_lines_endtoend, l_c2, {0, 1}},
   {"END-TO-END TRIPLE LINES WORKING TOGETHER",concept_triple_lines_tog_end2end, l_c3, {0, 8, 1}},
   {"END-TO-END TRIPLE LINES WORKING APART", concept_triple_lines_tog_end2end, l_c3, {0, 9, 1}},
   {"TRIPLE LINES WORKING FORWARD",          concept_triple_lines_together, l_c3, {0, 0, 1}},
   {"TRIPLE LINES WORKING BACKWARD",         concept_triple_lines_together, l_c3, {0, 2, 1}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE LINES WORKING CLOCKWISE",        concept_triple_lines_together, l_c4, {0, 8, 1}},
   {"TRIPLE LINES WORKING COUNTERCLOCKWISE", concept_triple_lines_together, l_c4, {0, 9, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL LINES",                 concept_triple_diag,           l_c4, {0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING FORWARD", concept_triple_diag_together,  l_c4, {0, 0, 1}},
   {"TRIPLE DIAGONAL LINES WORKING BACKWARD",concept_triple_diag_together,  l_c4, {0, 2, 1}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN LINES",                     concept_triple_twin,           l_c4a, {&map_4x6_2x4, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE LINES",                       concept_quad_lines,            l_c4a, {0, 1}},
   {"END-TO-END QUADRUPLE LINES",            concept_quad_lines_endtoend,   l_c4a, {0, 1}},
   {"END-TO-END QUADRUPLE LINES WORKING TOGETHER",concept_quad_lines_tog_end2end,l_c4a, {0, 8, 1}},
   {"END-TO-END QUADRUPLE LINES WORKING APART",concept_quad_lines_tog_end2end,l_c4a, {0, 9, 1}},
   {"QUADRUPLE LINES WORKING FORWARD",       concept_quad_lines_together,   l_c4a, {0, 0, 1}},
   {"QUADRUPLE LINES WORKING BACKWARD",      concept_quad_lines_together,   l_c4a, {0, 2, 1}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE LINES WORKING CLOCKWISE",     concept_quad_lines_together,   l_c4, {0, 8, 1}},
   {"QUADRUPLE LINES WORKING COUNTERCLOCKWISE",concept_quad_lines_together, l_c4, {0, 9, 1}},
/* -------- column break -------- */
#define pp__2_size 44
#define pp__2_spc 0
#define pp__2_ipc 1
#define pp__2_espc 3
#define pp__2_eipc 4
#define pp__2_pc8 12
#define pp__2_pc6 13
#define pp__2_tc 15
#define pp__2_etcwt 17
#define pp__2_etcwa 18
#define pp__2_tcwr 21
#define pp__2_tcwl 22
#define pp__2_trtc 32
#define pp__2_eqcwt 36
#define pp__2_eqcwa 37
#define pp__2_qcwr 40
#define pp__2_qcwl 41
   {"SPLIT PHANTOM COLUMNS",                 concept_do_phantom_2x4,        l_c3a, {&map_split_f, phantest_impossible, 0, MPKIND__SPLIT}},
   {"INTERLOCKED PHANTOM COLUMNS",           concept_do_phantom_2x4,        l_c3, {&map_intlk_f, phantest_impossible, 0, MPKIND__INTLK}},
   {"PHANTOM COLUMNS",                       concept_do_phantom_2x4,        l_c3a, {&map_full_f, phantest_first_or_both, 0, MPKIND__CONCPHAN}},
   {"END-TO-END SPLIT PHANTOM COLUMNS",      concept_do_phantom_endtoend,   l_c3a, {0, phantest_impossible, 0, MPKIND__SPLIT}},
   {"END-TO-END INTERLOCKED PHANTOM COLUMNS",concept_do_phantom_endtoend,   l_c3, {0, phantest_impossible, 0, MPKIND__INTLK}},
   {"END-TO-END PHANTOM COLUMNS",            concept_do_phantom_endtoend,   l_c3a, {0, phantest_first_or_both, 0, MPKIND__CONCPHAN}},
   {"12 MATRIX SPLIT PHANTOM COLUMNS",       concept_do_phantom_2x3,        l_c3x, {&map_3x4_2x3, phantest_impossible, 0}},
   {"12 MATRIX INTERLOCKED PHANTOM COLUMNS", concept_do_phantom_2x3,        l_c3x, {&map_3x4_2x3_intlk, phantest_impossible, 0}},
   {"12 MATRIX PHANTOM COLUMNS",             concept_do_phantom_2x3,        l_c3x, {&map_3x4_2x3_conc, phantest_first_or_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED COLUMNS",                       concept_divided_2x4,           l_c4, {&map_hv_2x4_2, phantest_impossible, 0}},
   {"12 MATRIX DIVIDED COLUMNS",             concept_divided_2x3,           l_c4, {&map_2x6_2x3, phantest_impossible, 0}},
   {"PHANTOM TIDAL COLUMNS",                 concept_do_phantom_1x8,        l_c3, {0, phantest_impossible, TRUE, 0}},
   {"PHANTOM COLUMNS OF 6",                  concept_do_phantom_1x6,        l_c3, {0, phantest_impossible, TRUE, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE COLUMNS",                        concept_triple_lines,          l_c3, {0, 0}},
   {"END-TO-END TRIPLE COLUMNS",             concept_triple_lines_endtoend, l_c3, {0, 0}},
   {"END-TO-END TRIPLE COLUMNS WORKING TOGETHER", concept_triple_lines_tog_end2end, l_c3, {0, 8, 0}},
   {"END-TO-END TRIPLE COLUMNS WORKING APART", concept_triple_lines_tog_end2end, l_c3, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE COLUMNS WORKING RIGHT",          concept_triple_lines_together, l_c3, {0, 2, 0}},
   {"TRIPLE COLUMNS WORKING LEFT",           concept_triple_lines_together, l_c3, {0, 0, 0}},
   {"TRIPLE COLUMNS WORKING CLOCKWISE",      concept_triple_lines_together, l_c4, {0, 8, 0}},
   {"TRIPLE COLUMNS WORKING COUNTERCLOCKWISE",concept_triple_lines_together,l_c4, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL COLUMNS",               concept_triple_diag,           l_c4, {0, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAGONAL COLUMNS WORKING RIGHT", concept_triple_diag_together,  l_c4, {0, 3, 0}},
   {"TRIPLE DIAGONAL COLUMNS WORKING LEFT",  concept_triple_diag_together,  l_c4, {0, 1, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE TWIN COLUMNS",                   concept_triple_twin,           l_c4a, {&map_4x6_2x4, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE COLUMNS",                     concept_quad_lines,            l_c4a, {0, 0}},
   {"END-TO-END QUADRUPLE COLUMNS",          concept_quad_lines_endtoend,   l_c4a, {0, 0}},
   {"END-TO-END QUADRUPLE COLUMNS WORKING TOGETHER",concept_quad_lines_tog_end2end,l_c4a, {0, 8, 0}},
   {"END-TO-END QUADRUPLE COLUMNS WORKING APART",concept_quad_lines_tog_end2end,l_c4a, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE COLUMNS WORKING RIGHT",       concept_quad_lines_together,   l_c4a, {0, 2, 0}},
   {"QUADRUPLE COLUMNS WORKING LEFT",        concept_quad_lines_together,   l_c4a, {0, 0, 0}},
   {"QUADRUPLE COLUMNS WORKING CLOCKWISE",   concept_quad_lines_together,   l_c4, {0, 8, 0}},
   {"QUADRUPLE COLUMNS WORKING COUNTERCLOCKWISE",concept_quad_lines_together,l_c4, {0, 9, 0}},
/* -------- column break -------- */
#define pp__3_size 44
#define pp__3_spb 0
#define pp__3_ipb 1
#define pp__3_tb 15
#define pp__3_tbwt 17
#define pp__3_tbwa 18
#define pp__3_tbwf 19
#define pp__3_tbwb 20
#define pp__3_tbwr 21
#define pp__3_tbwl 22
#define pp__3_qbwt 36
#define pp__3_qbwa 37
#define pp__3_qbwf 38
#define pp__3_qbwb 39
#define pp__3_qbwr 40
#define pp__3_qbwl 41
   {"SPLIT PHANTOM BOXES",                   concept_do_phantom_boxes,      l_c3, {&map_hv_2x4_2, phantest_impossible}},
   {"INTERLOCKED PHANTOM BOXES",             concept_do_phantom_boxes,      l_c4, {&map_intlk_phantom_box, phantest_impossible}},
   {"PHANTOM BOXES",                         concept_do_phantom_boxes,      l_c4a, {&map_phantom_box, phantest_first_or_both}},
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
   {"TRIPLE BOXES",                          concept_triple_boxes,          l_c1, {0, MPKIND__SPLIT}},
   {"CONCENTRIC TRIPLE BOXES",               concept_triple_boxes,          l_c1, {0, MPKIND__CONCPHAN}},
   {"TRIPLE BOXES WORKING TOGETHER",         concept_triple_boxes_together, l_c3, {0, 6, 0}},
   {"TRIPLE BOXES WORKING APART",            concept_triple_boxes_together, l_c3, {0, 7, 0}},
   {"TRIPLE BOXES WORKING FORWARD",          concept_triple_boxes_together, l_c3, {0, 0, 0}},
   {"TRIPLE BOXES WORKING BACKWARD",         concept_triple_boxes_together, l_c3, {0, 2, 0}},
   {"TRIPLE BOXES WORKING RIGHT",            concept_triple_boxes_together, l_c3, {0, 3, 0}},
   {"TRIPLE BOXES WORKING LEFT",             concept_triple_boxes_together, l_c3, {0, 1, 0}},
   {"TRIPLE BOXES WORKING CLOCKWISE",        concept_triple_boxes_together, l_c4, {0, 8, 0}},
   {"TRIPLE BOXES WORKING COUNTERCLOCKWISE", concept_triple_boxes_together, l_c4, {0, 9, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE BOXES",                       concept_quad_boxes,            l_c4a, {0, MPKIND__SPLIT}},
   {"CONCENTRIC QUADRUPLE BOXES",            concept_quad_boxes,            l_c4a, {0, MPKIND__CONCPHAN}},
   {"QUADRUPLE BOXES WORKING TOGETHER",      concept_quad_boxes_together,   l_c4a, {0, 6, 0}},
   {"QUADRUPLE BOXES WORKING APART",         concept_quad_boxes_together,   l_c4a, {0, 7, 0}},
   {"QUADRUPLE BOXES WORKING FORWARD",       concept_quad_boxes_together,   l_c4a, {0, 0, 0}},
   {"QUADRUPLE BOXES WORKING BACKWARD",      concept_quad_boxes_together,   l_c4a, {0, 2, 0}},
   {"QUADRUPLE BOXES WORKING RIGHT",         concept_quad_boxes_together,   l_c4a, {0, 3, 0}},
   {"QUADRUPLE BOXES WORKING LEFT",          concept_quad_boxes_together,   l_c4a, {0, 1, 0}},
   {"QUADRUPLE BOXES WORKING CLOCKWISE",     concept_quad_boxes_together,   l_c4,  {0, 8, 0}},
   {"QUADRUPLE BOXES WORKING COUNTERCLOCKWISE",concept_quad_boxes_together, l_c4 , {0, 9, 0}},
/* -------- column break -------- */
#define pp__4_size 37
#define pp__4_spd 0
#define pp__4_ipd 1
#define pp__4_td 15
#define pp__4_tdwt 17
#define pp__4_qd 34
#define pp__4_qdwt 36
   {"SPLIT PHANTOM DIAMONDS",                concept_do_phantom_diamonds,   l_c3x, {&map_hv_qtg_2, phantest_impossible, 0}},
   {"INTERLOCKED PHANTOM DIAMONDS",          concept_do_phantom_diamonds,   l_c4, {&map_intlk_phantom_dmd, phantest_impossible, 0}},
   {"PHANTOM DIAMONDS",                      concept_do_phantom_diamonds,   l_c4, {&map_phantom_dmd, phantest_first_or_both, 0}},
   {"SPLIT PHANTOM 1/4-TAGS",                concept_do_phantom_qtags,      l_c3x, {&map_hv_qtg_2, phantest_impossible, 1}},
   {"INTERLOCKED PHANTOM 1/4-TAGS",          concept_do_phantom_qtags,      l_c4, {&map_intlk_phantom_dmd, phantest_impossible, 1}},
   {"PHANTOM 1/4-TAGS",                      concept_do_phantom_qtags,      l_c4, {&map_phantom_dmd, phantest_first_or_both, 1}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"DIVIDED DIAMONDS",                      concept_do_divided_diamonds,   l_c3x, {&map_vv_qtg_2, phantest_impossible}},
   {"DIVIDED 1/4-TAGS",                      concept_do_divided_qtags,      l_c3x, {&map_vv_qtg_2, phantest_impossible}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAMONDS",                       concept_triple_diamonds,          l_c3a},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIPLE DIAMONDS WORKING TOGETHER",      concept_triple_diamonds_together, l_c3},
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
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE DIAMONDS",                    concept_quad_diamonds,          l_c4a},
         {"", concept_comment, l_nonexistent_concept},
   {"QUADRUPLE DIAMONDS WORKING TOGETHER",   concept_quad_diamonds_together, l_c4a},

/* tandem concepts */

#define tt__1_size 14
#define tt__1_cpl 0
#define tt__1_tnd 1
#define tt__1_grc 12
#define tt__1_grt 13
   {"AS COUPLES",                            concept_tandem,                l_a1,         {0, FALSE, 0, 0, 1}},
   {"TANDEM",                                concept_tandem,                l_c1,         {0, FALSE, 0, 0, 0}},
   {"SIAMESE",                               concept_tandem,                l_c1,         {0, FALSE, 0, 0, 2}},
   {"COUPLES OF THREE",                      concept_tandem,                l_c1,         {0, FALSE, 0, 0, 5}},
   {"TANDEMS OF THREE",                      concept_tandem,                l_c1,         {0, FALSE, 0, 0, 4}},
   {"COUPLES OF FOUR",                       concept_tandem,                l_c1,         {0, FALSE, 0, 0, 7}},
   {"TANDEMS OF FOUR",                       concept_tandem,                l_c1,         {0, FALSE, 0, 0, 6}},
   {"BOXES WORK SOLID",                      concept_tandem,                l_c3,         {0, FALSE, 0, 0, 8}},
   {"DIAMONDS WORK SOLID",                   concept_tandem,                l_c3,         {0, FALSE, 0, 0, 9}},
   {"SKEW",                                  concept_tandem,                l_c4a,        {0, FALSE, 0, 0, 3}},
   {"<ANYONE> ARE AS COUPLES",               concept_some_are_tandem,       l_a1,         {0, TRUE,  0, 0, 1}},
   {"<ANYONE> ARE TANDEM",                   concept_some_are_tandem,       l_c1,         {0, TRUE,  0, 0, 0}},
   {"GRUESOME AS COUPLES",                   concept_gruesome_tandem,       l_c4a,        {0, FALSE, 0, 2, 1}},
   {"GRUESOME TANDEM",                       concept_gruesome_tandem,       l_c4a,        {0, FALSE, 0, 2, 0}},
/* -------- column break -------- */
#define tt__2_size 14
#define tt__2_grct 12
#define tt__2_grtt 13
   {"COUPLES TWOSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 1}},
   {"TANDEM TWOSOME",                        concept_tandem,                l_c3,         {0, FALSE, 1, 0, 0}},
   {"SIAMESE TWOSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 2}},
   {"COUPLES THREESOME",                     concept_tandem,                l_c3,         {0, FALSE, 1, 0, 5}},
   {"TANDEM THREESOME",                      concept_tandem,                l_c3,         {0, FALSE, 1, 0, 4}},
   {"COUPLES FOURSOME",                      concept_tandem,                l_c3,         {0, FALSE, 1, 0, 7}},
   {"TANDEM FOURSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 6}},
   {"BOXSOME",                               concept_tandem,                l_c3,         {0, FALSE, 1, 0, 8}},
   {"DIAMONDSOME",                           concept_tandem,                l_c3,         {0, FALSE, 1, 0, 9}},
   {"SKEWSOME",                              concept_tandem,                l_c4a,        {0, FALSE, 1, 0, 3}},
   {"<ANYONE> ARE COUPLES TWOSOME",          concept_some_are_tandem,       l_c3,         {0, TRUE,  1, 0, 1}},
   {"<ANYONE> ARE TANDEM TWOSOME",           concept_some_are_tandem,       l_c3,         {0, TRUE,  1, 0, 0}},
   {"GRUESOME TWOSOME",                      concept_gruesome_tandem,       l_c4a,        {0, FALSE, 1, 2, 1}},
   {"GRUESOME TANDEM TWOSOME",               concept_gruesome_tandem,       l_c4a,        {0, FALSE, 1, 2, 0}},
/* -------- column break -------- */
#define tt__3_size 14
   {"COUPLES <N/4> TWOSOME",                 concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 1}},
   {"TANDEM <N/4> TWOSOME",                  concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 0}},
   {"SIAMESE <N/4> TWOSOME",                 concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 2}},
   {"COUPLES OF 3 <N/4> THREESOME",          concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 5}},
   {"TANDEMS OF 3 <N/4> THREESOME",          concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 4}},
   {"COUPLES OF 4 <N/4> FOURSOME",           concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 7}},
   {"TANDEMS OF 4 <N/4> FOURSOME",           concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 6}},
   {"BOXES ARE SOLID <N/4> BOXSOME",         concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 8}},
   {"DIAMONDS ARE SOLID <N/4> DIAMONDSOME",  concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 9}},
   {"SKEW <N/4> TWOSOME",                    concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 3}},
   {"<ANYONE> ARE COUPLES <N/4> TWOSOME",    concept_some_are_frac_tandem,  l_c4,         {0, TRUE,  2, 0, 1}},
   {"<ANYONE> ARE TANDEM <N/4> TWOSOME",     concept_some_are_frac_tandem,  l_c4,         {0, TRUE,  2, 0, 0}},
   {"GRUESOME AS COUPLES <N/4> TWOSOME",     concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 2, 2, 1}},
   {"GRUESOME TANDEM <N/4> TWOSOME",         concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 2, 2, 0}},
/* -------- column break -------- */
#define tt__4_size 14
   {"COUPLES TWOSOME <N/4> SOLID",           concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 1}},
   {"TANDEM TWOSOME <N/4> SOLID",            concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 0}},
   {"SIAMESE TWOSOME <N/4> SOLID",           concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 2}},
   {"COUPLES THREESOME <N/4> SOLID",         concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 5}},
   {"TANDEM THREESOME <N/4> SOLID",          concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 4}},
   {"COUPLES FOURSOME <N/4> SOLID",          concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 7}},
   {"TANDEM FOURSOME <N/4> SOLID",           concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 6}},
   {"BOXSOME <N/4> SOLID",                   concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 8}},
   {"DIAMONDSOME <N/4> SOLID",               concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 9}},
   {"SKEWSOME <N/4> SOLID",                  concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 3}},
   {"<ANYONE> ARE COUPLES TWOSOME <N/4> SOLID", concept_some_are_frac_tandem, l_c4,       {0, TRUE,  3, 0, 1}},
   {"<ANYONE> ARE TANDEM TWOSOME <N/4> SOLID",  concept_some_are_frac_tandem, l_c4,       {0, TRUE,  3, 0, 0}},
   {"GRUESOME TWOSOME <N/4> SOLID",          concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 3, 2, 1}},
   {"GRUESOME TANDEM TWOSOME <N/4> SOLID",   concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 3, 2, 0}},

/* distorted concepts */

#define dd__1_size 10
   {"STAGGER",                               concept_do_phantom_2x4,        l_c2,         {&map_stagger, phantest_only_one, 0}},
   {"STAIRSTEP COLUMNS",                     concept_do_phantom_2x4,        l_c4,         {&map_stairst, phantest_only_one, 0}},
   {"LADDER COLUMNS",                        concept_do_phantom_2x4,        l_c4,         {&map_ladder, phantest_only_one, 0}},
   {"OFFSET COLUMNS",                        concept_distorted,             l_c2,         {0, 0, disttest_offset}},
   {"\"Z\" COLUMNS",                         concept_distorted,             l_c3,         {0, 0, disttest_z}},
   {"DISTORTED COLUMNS",                     concept_distorted,             l_c3,         {0, 0, disttest_any}},
   {"<ANYONE> IN A DIAGONAL COLUMN",         concept_single_diagonal,       l_c3,         {0, 0}},
   {"TWO DIAGONAL COLUMNS",                  concept_double_diagonal,       l_c3,         {0, 0}},
   {"\"O\"",                                 concept_do_phantom_2x4,        l_c1,         {&map_o_s2x4_3, phantest_only_first_one, 0}},
   {"BUTTERFLY",                             concept_do_phantom_2x4,        l_c1,         {&map_x_s2x4_3, phantest_only_second_one, 0}},
/* -------- column break -------- */
#define dd__2_size 8
   {"BIGBLOCK",                              concept_do_phantom_2x4,        l_c3a,        {&map_stagger, phantest_only_one, 1}},
   {"STAIRSTEP LINES",                       concept_do_phantom_2x4,        l_c4,         {&map_ladder, phantest_only_one, 1}},
   {"LADDER LINES",                          concept_do_phantom_2x4,        l_c4,         {&map_stairst, phantest_only_one, 1}},
   {"OFFSET LINES",                          concept_distorted,             l_c2,         {0, 1, disttest_offset}},
   {"\"Z\" LINES",                           concept_distorted,             l_c3,         {0, 1, disttest_z}},
   {"DISTORTED LINES",                       concept_distorted,             l_c3,         {0, 1, disttest_any}},
   {"<ANYONE> IN A DIAGONAL LINE",           concept_single_diagonal,       l_c3,         {0, 1}},
   {"TWO DIAGONAL LINES",                    concept_double_diagonal,       l_c3,         {0, 1}},
/* -------- column break -------- */
#define dd__3_size 9
#define dd__3_pofc 3
#define dd__3_pob 8
   {"PHANTOM STAGGER COLUMNS",               concept_do_phantom_2x4,        l_c4,         {&map_stagger, phantest_both, 0}},
   {"PHANTOM STAIRSTEP COLUMNS",             concept_do_phantom_2x4,        l_c4,         {&map_stairst, phantest_both, 0}},
   {"PHANTOM LADDER COLUMNS",                concept_do_phantom_2x4,        l_c4,         {&map_ladder, phantest_both, 0}},
   {"PHANTOM OFFSET COLUMNS",                concept_do_phantom_2x4,        l_c4a,        {&map_offset, phantest_both, 0}},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
         {"", concept_comment, l_nonexistent_concept},
   {"PHANTOM BUTTERFLY OR \"O\"",            concept_do_phantom_2x4,        l_c4a,        {&map_but_o, phantest_both, 0}},
/* -------- column break -------- */
#define dd__4_size 4
#define dd__4_pofl 3
   {"PHANTOM BIGBLOCK LINES",                concept_do_phantom_2x4,        l_c4,         {&map_stagger, phantest_both, 1}},
   {"PHANTOM STAIRSTEP LINES",               concept_do_phantom_2x4,        l_c4,         {&map_ladder, phantest_both, 1}},
   {"PHANTOM LADDER LINES",                  concept_do_phantom_2x4,        l_c4,         {&map_stairst, phantest_both, 1}},
   {"PHANTOM OFFSET LINES",                  concept_do_phantom_2x4,        l_c4a,        {&map_offset, phantest_both, 1}},

/* 4-person distorted concepts */

#define d4__1_size 6
   {"SPLIT",                                 concept_split,                 l_mainstream},
   {"ONCE REMOVED",                          concept_once_removed,          l_c2,         {0, 0}},
   {"ONCE REMOVED DIAMONDS",                 concept_once_removed,          l_c3,         {0, 1}},
   {"MAGIC",                                 concept_magic,                 l_c1},
   {"DIAGONAL",                              concept_do_both_boxes,         l_c3a,        {&map_2x4_diagonal, 97, FALSE}},
   {"TRAPEZOID",                             concept_do_both_boxes,         l_c3,         {&map_2x4_trapezoid, 97, FALSE}},
/* -------- column break -------- */
#define d4__2_size 5
   {"INTERLOCKED PARALLELOGRAM",             concept_do_both_boxes,         l_c3x,        {&map_2x4_int_pgram, 97, TRUE}},
   {"INTERLOCKED BOXES",                     concept_misc_distort,          l_c3x,        {0, 3, 0}},
   {"TWIN PARALLELOGRAMS",                   concept_misc_distort,          l_c3x,        {0, 2, 0}},
   {"EACH \"Z\"",                            concept_misc_distort,          l_c3a,        {0, 0, 0}},
   {"INTERLOCKED \"Z's\"",                   concept_misc_distort,          l_c3a,        {0, 0, 8}},
/* -------- column break -------- */
#define d4__3_size 6
   {"JAY",                                   concept_misc_distort,          l_c3a,        {0, 1, 0}},
   {"BACK-TO-FRONT JAY",                     concept_misc_distort,          l_c3a,        {0, 1, 8}},
   {"BACK-TO-BACK JAY",                      concept_misc_distort,          l_c3a,        {0, 1, 16}},
   {"FACING PARALLELOGRAM",                  concept_misc_distort,          l_c3x,        {0, 4, 0}},
   {"BACK-TO-FRONT PARALLELOGRAM",           concept_misc_distort,          l_c3x,        {0, 4, 8}},
   {"BACK-TO-BACK PARALLELOGRAM",            concept_misc_distort,          l_c3x,        {0, 4, 16}},
/* -------- column break -------- */
#define d4__4_size 4
#define d4__4_pibl 1
   {"IN YOUR BLOCKS",                        concept_do_phantom_2x2,        l_c1,         {&map_blocks, phantest_2x2_only_two}},
   {"4 PHANTOM INTERLOCKED BLOCKS",          concept_do_phantom_2x2,        l_c4a,        {&map_blocks, phantest_2x2_both}},
   {"TRIANGULAR BOXES",                      concept_do_phantom_2x2,        l_c4,         {&map_trglbox, phantest_2x2_only_two}},
   {"4 PHANTOM TRIANGULAR BOXES",            concept_do_phantom_2x2,        l_c4,         {&map_trglbox, phantest_2x2_both}},

/* Miscellaneous concepts */

#define mm__1_size 37
#define mm__1_3x3 35
#define mm__1_4x4 36
#define mm__1_phan 18
   {"LEFT",                                  concept_left,                  l_mainstream},
   {"REVERSE",                               concept_reverse,               l_mainstream},
   {"CROSS",                                 concept_cross,                 l_mainstream},
   {"SINGLE",                                concept_single,                l_mainstream},
   {"SINGLE FILE",                           concept_singlefile,            l_c3x},
   {"GRAND",                                 concept_grand,                 l_plus},
         {"", concept_comment, l_nonexistent_concept},
   {"TRIANGLE",                              concept_triangle,              l_c1},
   {"DIAMOND",                               concept_diamond,               l_c3x},
   {"INTERLOCKED",                           concept_interlocked,           l_c1},
   {"12 MATRIX",                             concept_12_matrix,             l_c3x},
   {"16 MATRIX",                             concept_16_matrix,             l_c3x},
   {"FUNNY",                                 concept_funny,                 l_c2},
   {"ASSUME WAVES",                          concept_assume_waves,          l_c3},
   {"CENTRAL",                               concept_central,               l_c3x},
   {"FAN",                                   concept_fan_or_yoyo,           l_c3,        {0, 0}},
   {"YO-YO",                                 concept_fan_or_yoyo,           l_c4,        {0, 1}},
   {"PARALLELOGRAM",                         concept_parallelogram,         l_c2},
   {"PHANTOM",                               concept_c1_phantom,            l_c1},
   {"<ANYONE> ARE STANDARD IN",              concept_standard,              l_c4a,        {0, 0}},
   {"STABLE",                                concept_stable,                l_c3a,        {0, FALSE, FALSE}},
   {"<ANYONE> ARE STABLE",                   concept_so_and_so_stable,      l_c3a,        {0, TRUE,  FALSE}},
   {"<N/4> STABLE",                          concept_frac_stable,           l_c4,         {0, FALSE, TRUE}},
   {"<ANYONE> ARE <N/4> STABLE",             concept_so_and_so_frac_stable, l_c4,         {0, TRUE,  TRUE}},
   {"TRACE",                                 concept_trace,                 l_c3x},
   {"STRETCH",                               concept_old_stretch,           l_c1},
   {"STRETCHED SETUP",                       concept_new_stretch,           l_c2},
   {"FERRIS",                                concept_ferris,                l_c3x,        {0, 0}},
   {"RELEASE",                               concept_ferris,                l_c3a,        {0, 1}},
         {"", concept_comment, l_nonexistent_concept},
   {"1X2",                                   concept_1x2,                   l_c3},
   {"2X1",                                   concept_2x1,                   l_c3},
   {"2X2",                                   concept_2x2,                   l_c3a},
   {"1X3",                                   concept_1x3,                   l_c2},
   {"3X1",                                   concept_3x1,                   l_c2},
   {"3X3",                                   concept_3x3,                   l_c3x},
   {"4X4",                                   concept_4x4,                   l_c3x},
/* -------- column break -------- */
#define mm__2_size 38
   {"CENTERS",                               concept_centers_or_ends,       l_mainstream, {0, 0}},
   {"ENDS",                                  concept_centers_or_ends,       l_mainstream, {0, 1}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,      l_mainstream, {0, 0}},
   {"CENTER 6",                              concept_centers_or_ends,       l_mainstream, {0, 2}},
   {"OUTER 6",                               concept_centers_or_ends,       l_mainstream, {0, 5}},
   {"CENTER 2",                              concept_centers_or_ends,       l_mainstream, {0, 4}},
   {"OUTER 2",                               concept_centers_or_ends,       l_mainstream, {0, 3}},
   {"CENTER 6/OUTER 2",                      concept_centers_and_ends,      l_mainstream, {0, 2}},
   {"CENTER 2/OUTER 6",                      concept_centers_and_ends,      l_mainstream, {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"CHECKPOINT",                            concept_checkpoint,            l_c2,         {0, 0}},
   {"REVERSE CHECKPOINT",                    concept_checkpoint,            l_c3,         {0, 1}},
   {"CHECKERBOARD",                          concept_checkerboard,          l_c3a,        {0, s1x4}},
   {"CHECKERBOX",                            concept_checkerboard,          l_c3a,        {0, s2x2}},
   {"CHECKERDIAMOND",                        concept_checkerboard,          l_c3x,        {0, sdmd}},
         {"", concept_comment, l_nonexistent_concept},
   {"ON YOUR OWN",                           concept_on_your_own,           l_c4a},
   {"<ANYONE> DO YOUR PART,",                concept_so_and_so_only,        l_mainstream, {0, 0}},
   {"OWN THE <ANYONE>",                      concept_some_vs_others,        l_c3a,        {0, 1}},
   {"<ANYONE> START",                        concept_so_and_so_begin,       l_c2,         {0, 0}},
         {"", concept_comment, l_nonexistent_concept},
   {"two calls in succession",               concept_sequential,            l_mainstream},
   {"<N/4> CRAZY",                           concept_crazy,                 l_c2,        {0, 0}},
   {"<N/4> REVERSE CRAZY",                   concept_crazy,                 l_c3x,       {0, 1}},
   {"DO IT TWICE:",                          concept_twice,                 l_mainstream},
   {"RANDOM",                                concept_meta,                  l_c3a,        {0, 0}},
   {"REVERSE RANDOM",                        concept_meta,                  l_c3x,        {0, 1}},
   {"PIECEWISE",                             concept_meta,                  l_c3x,        {0, 2}},
   {"FRACTIONAL",                            concept_fractional,            l_c2,         {0, 0}},
   {"LAST FRACTION",                         concept_fractional,            l_c2,         {0, 1}},
   {"REVERSE ORDER",                         concept_meta,                  l_c3x,        {0, 5}},
   {"INTERLACE",                             concept_interlace,             l_c3x},
   {"START <concept>",                       concept_meta,                  l_c2,         {0, 3}},
   {"FINISH",                                concept_meta,                  l_c2,        {0, 4}},
   {"SKIP THE Nth PART",                     concept_nth_part,              l_c2,         {0, 1}},
   {"DO THE Nth PART <concept>",             concept_nth_part,              l_c2,         {0, 0}},
   {"REPLACE THE Nth PART OF X WITH Y",      concept_replace_nth_part,      l_c2,         {0, 0}},
   {"INTERRUPT X AFTER THE Nth PART WITH Y", concept_replace_nth_part,      l_c2,         {0, 1}},
/* -------- column break -------- */
#define mm__3_size 37
   {"IN POINT TRIANGLE",                     concept_randomtrngl,           l_c1,         {0, 1}},
   {"OUT POINT TRIANGLE",                    concept_randomtrngl,           l_c1,         {0, 0}},
   {"INSIDE TRIANGLES",                      concept_randomtrngl,           l_c1,         {0, 2}},
   {"OUTSIDE TRIANGLES",                     concept_randomtrngl,           l_c1,         {0, 3}},
   {"TALL 6",                                concept_randomtrngl,           l_c3x,        {0, 4}},
   {"SHORT 6",                               concept_randomtrngl,           l_c3x,        {0, 5}},
   {"WAVE-BASE TRIANGLES",                   concept_randomtrngl,           l_c1,         {0, 6}},
   {"TANDEM-BASE TRIANGLES",                 concept_randomtrngl,           l_c1,         {0, 7}},
   {"<ANYONE>-BASED TRIANGLE",               concept_selbasedtrngl,         l_c1,         {0, 20}},
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
   {"GRAND WORKING AS CENTERS",              concept_grand_working,         l_c3x,        {0, 8}},
   {"GRAND WORKING AS ENDS",                 concept_grand_working,         l_c3x,        {0, 9}},
         {"", concept_comment, l_nonexistent_concept},
   {"<ANYONE> ARE CENTERS OF A DOUBLE-OFFSET 1/4-TAG",  concept_double_offset, l_c4,      {0, 0}},
   {"<ANYONE> ARE CENTERS OF A DOUBLE-OFFSET 3/4-TAG",  concept_double_offset, l_c4,      {0, 1}},
   {"<ANYONE> ARE CENTERS OF A DOUBLE-OFFSET THING",    concept_double_offset, l_c4,      {0, 2}},
   {"<ANYONE> ARE CENTERS OF DOUBLE-OFFSET DIAMONDS", concept_double_offset, l_c4,        {0, 3}},
         {"", concept_comment, l_nonexistent_concept},
   {"INRIGGER",                              concept_rigger,                l_c3x,        {0, 2}},
   {"OUTRIGGER",                             concept_rigger,                l_c3x,        {0, 0}},
   {"LEFTRIGGER",                            concept_rigger,                l_c3x,        {0, 1}},
   {"RIGHTRIGGER",                           concept_rigger,                l_c3x,        {0, 3}},
   {"BACKRIGGER",                            concept_rigger,                l_c3x,        {0, 16}},
   {"FRONTRIGGER",                           concept_rigger,                l_c3x,        {0, 18}},
   {"LEFTSLIDER",                            concept_slider,                l_c3x,        {0, 3}},
   {"RIGHTSLIDER",                           concept_slider,                l_c3x,        {0, 1}},
   {"BACKSLIDER",                            concept_slider,                l_c3x,        {0, 0}},
   {"FRONTSLIDER",                           concept_slider,                l_c3x,        {0, 2}},
/*
   {"[call]-RIGGER",                         concept_callrigger,            l_c3x},
*/

/* general concepts */

   {"CENTERS",                               concept_centers_or_ends,       l_mainstream, {0, 0}},
   {"ENDS",                                  concept_centers_or_ends,       l_mainstream, {0, 1}},
   {"CENTERS AND ENDS",                      concept_centers_and_ends,      l_mainstream, {0, 0}},
   {"LEFT",                                  concept_left,                  l_mainstream},
   {"REVERSE",                               concept_reverse,               l_mainstream},
   {"SPLIT",                                 concept_split,                 l_mainstream},
   {"EACH 1X4",                              concept_each_1x4,              l_mainstream},
   {"CROSS",                                 concept_cross,                 l_mainstream},
   {"GRAND",                                 concept_grand,                 l_plus},
   {"MAGIC",                                 concept_magic,                 l_c1},
   {"INTERLOCKED",                           concept_interlocked,           l_c1},
   {"12 MATRIX",                             concept_12_matrix,             l_c3x},
   {"16 MATRIX",                             concept_16_matrix,             l_c3x},
   {"2X6 MATRIX",                            concept_2x6_matrix,            l_c3x},
   {"2X8 MATRIX",                            concept_2x8_matrix,            l_c3x},
   {"3X4 MATRIX",                            concept_3x4_matrix,            l_c3x},
   {"4X4 MATRIX",                            concept_4x4_matrix,            l_c3x},
   {"16 MATRIX OF PARALLEL DIAMONDS",        concept_4dmd_matrix,           l_c3x},
   {"PHANTOM",                               concept_c1_phantom,            l_c1},
   {"FUNNY",                                 concept_funny,                 l_c2},
   {"SINGLE",                                concept_single,                l_mainstream},
   {"CONCENTRIC",                            concept_concentric,            l_c1,         {0, schema_concentric}},
   {"AS COUPLES",                            concept_tandem,                l_a1,         {0, FALSE, FALSE, 0, 1}},
   {"TANDEM",                                concept_tandem,                l_c1,         {0, FALSE, FALSE, 0, 0}},
   {"???",                                   marker_end_of_list}};

/* **** BEWARE!!!!  These things are keyed to "concept_descriptor_table"
   (above) and tell how to divide things into the various menus and popups.
   BE CAREFUL!! */

#define pp__tot_size (pp__1_size+pp__2_size+pp__3_size+pp__4_size)
#define tt__tot_size (tt__1_size+tt__2_size+tt__3_size+tt__4_size)
#define dd__tot_size (dd__1_size+dd__2_size+dd__3_size+dd__4_size)
#define d4__tot_size (d4__1_size+d4__2_size+d4__3_size+d4__4_size)

#define pp_1_offset (0)
#define pp_2_offset (0 + pp__1_size)
#define pp_3_offset (0 + pp__1_size + pp__2_size)
#define pp_4_offset (0 + pp__1_size + pp__2_size + pp__3_size)

#define tt_1_offset (0 + pp__tot_size)
#define tt_2_offset (0 + pp__tot_size + tt__1_size)
#define tt_3_offset (0 + pp__tot_size + tt__1_size + tt__2_size)
#define tt_4_offset (0 + pp__tot_size + tt__1_size + tt__2_size + tt__3_size)

#define dd_1_offset (0 + pp__tot_size + tt__tot_size)
#define dd_2_offset (0 + pp__tot_size + tt__tot_size + dd__1_size)
#define dd_3_offset (0 + pp__tot_size + tt__tot_size + dd__1_size + dd__2_size)
#define dd_4_offset (0 + pp__tot_size + tt__tot_size + dd__1_size + dd__2_size + dd__3_size)

#define d4_1_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size)
#define d4_2_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__1_size)
#define d4_3_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__1_size + d4__2_size)
#define d4_4_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__1_size + d4__2_size + d4__3_size)

#define mm_1_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__tot_size)
#define mm_2_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size)
#define mm_3_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size + mm__2_size)

#define gg_offset (0 + pp__tot_size + tt__tot_size + dd__tot_size + d4__tot_size + mm__1_size + mm__2_size + mm__3_size)


static int nice_setup_concept_4x4[] = {
   pp_1_offset + pp__1_spl,     /* split phantom lines */
   pp_1_offset + pp__1_ipl,     /* interlocked phantom lines */
   pp_1_offset + pp__1_qlwf,    /* quadruple lines working forward */
   pp_1_offset + pp__1_qlwb,    /* quadruple lines working backward */
   pp_2_offset + pp__2_spc,     /* split phantom columns */
   pp_2_offset + pp__2_ipc,     /* interlocked phantom columns */
   pp_2_offset + pp__2_qcwr,    /* quadruple columns working right */
   pp_2_offset + pp__2_qcwl,    /* quadruple columns working left */
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
   mm_1_offset + mm__1_4x4,     /* 4x4 */
   -1};

static int nice_setup_concept_3x4[] = {
   pp_1_offset + pp__1_tl,      /* triple lines */
   pp_2_offset + pp__2_tc,      /* triple columns */
   pp_1_offset + pp__1_tlwf,    /* triple lines working forward */
   pp_1_offset + pp__1_tlwb,    /* triple lines working backward */
   pp_2_offset + pp__2_tcwr,    /* triple columns working right */
   pp_2_offset + pp__2_tcwl,    /* triple columns working left */
   mm_1_offset + mm__1_3x3,     /* 3x3 */
   -1};

static int nice_setup_concept_2x8[] = {
   pp_3_offset + pp__3_spb,     /* split phantom boxes */
   pp_3_offset + pp__3_ipb,     /* interlocked phantom boxes */
   pp_3_offset + pp__3_qbwf,    /* quadruple boxes working forward */
   pp_3_offset + pp__3_qbwb,    /* quadruple boxes working backward */
   pp_3_offset + pp__3_qbwr,    /* quadruple boxes working right */
   pp_3_offset + pp__3_qbwl,    /* quadruple boxes working left */
   pp_3_offset + pp__3_qbwt,    /* quadruple boxes working together */
   pp_3_offset + pp__3_qbwa,    /* quadruple boxes working apart */
   pp_1_offset + pp__1_pl8,     /* phantom tidal lines */
   pp_2_offset + pp__2_pc8,     /* phantom tidal columns */
   /* The less likely ones: */
   mm_1_offset + mm__1_4x4,
   tt_1_offset + tt__1_grc,     /* gruesome as couples */
   tt_1_offset + tt__1_grt,     /* gruesome tandem */
   tt_2_offset + tt__2_grct,    /* gruesome (as couples) twosome */
   tt_2_offset + tt__2_grtt,    /* gruesome tandem twosome */
   -1};

static int nice_setup_concept_2x6[] = {
   pp_3_offset + pp__3_tb,      /* triple boxes */
   pp_3_offset + pp__3_tbwf,    /* triple boxes working forward */
   pp_3_offset + pp__3_tbwb,    /* triple boxes working backward */
   pp_3_offset + pp__3_tbwr,    /* triple boxes working right */
   pp_3_offset + pp__3_tbwl,    /* triple boxes working left */
   pp_3_offset + pp__3_tbwt,    /* triple boxes working together */
   pp_3_offset + pp__3_tbwa,    /* triple boxes working apart */
   pp_1_offset + pp__1_pl6,     /* phantom lines of 6 */
   pp_2_offset + pp__2_pc6,     /* phantom columns of 6 */
   mm_1_offset + mm__1_3x3,     /* 3x3 */
   /* Let's give these extra probability. */
   pp_3_offset + pp__3_tb,      /* triple boxes */
   pp_3_offset + pp__3_tb,      /* triple boxes */
   pp_3_offset + pp__3_tbwt,    /* triple boxes working together */
   pp_3_offset + pp__3_tbwt,    /* triple boxes working together */
   /* These are actually 2x8 concepts, but they could still be helpful. */
   pp_3_offset + pp__3_spb,     /* split phantom boxes */
   pp_3_offset + pp__3_qbwt,    /* quadruple boxes working together */
   pp_1_offset + pp__1_pl8,     /* phantom tidal lines */
   pp_2_offset + pp__2_pc8,     /* phantom tidal columns */
   -1};

static int nice_setup_concept_1x12[] = {
   pp_1_offset + pp__1_etlwt,    /* end-to-end triple lines working together */
   pp_1_offset + pp__1_etlwa,    /* end-to-end triple lines working apart */
   pp_2_offset + pp__2_etcwt,    /* end-to-end triple columns working together */
   pp_2_offset + pp__2_etcwa,    /* end-to-end triple columns working apart */
   -1};

static int nice_setup_concept_1x16[] = {
   pp_1_offset + pp__1_espl,     /* end-to-end split phantom lines */
   pp_1_offset + pp__1_eipl,     /* end-to-end interlocked phantom lines */
   pp_1_offset + pp__1_eqlwt,    /* end-to-end quadruple lines working together */
   pp_1_offset + pp__1_eqlwa,    /* end-to-end quadruple lines working apart */
   pp_2_offset + pp__2_espc,     /* end-to-end split phantom columns */
   pp_2_offset + pp__2_eipc,     /* end-to-end interlocked phantom columns */
   pp_2_offset + pp__2_eqcwt,    /* end-to-end quadruple columns working together */
   pp_2_offset + pp__2_eqcwa,    /* end-to-end quadruple columns working apart */
   -1};

static int nice_setup_concept_3dmd[] = {
   pp_4_offset + pp__4_td,      /* triple diamonds */
   pp_4_offset + pp__4_tdwt,    /* triple diamonds working together */
   -1};

static int nice_setup_concept_4dmd[] = {
   pp_4_offset + pp__4_spd,     /* split phantom diamonds */
   pp_4_offset + pp__4_ipd,     /* interlocked phantom diamonds */
   pp_4_offset + pp__4_qd,      /* quadruple diamonds */
   pp_4_offset + pp__4_qdwt,    /* quadruple diamonds working together */
   -1};

static int nice_setup_concept_4x6[] = {
   pp_1_offset + pp__1_trtl,     /* triple twin lines */
   pp_2_offset + pp__2_trtc,     /* triple twin columns */
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


Private int phantom_sizes[]   = {pp__1_size, pp__2_size, pp__3_size, pp__4_size, -1};
Private int tandem_sizes[]    = {tt__1_size, tt__2_size, tt__3_size, tt__4_size, -1};
Private int distort_sizes[]   = {dd__1_size, dd__2_size, dd__3_size, dd__4_size, -1};
Private int dist4_sizes[]     = {d4__1_size, d4__2_size, d4__3_size, d4__4_size, -1};
Private int misc_c_sizes[]    = {mm__1_size, mm__2_size, mm__3_size,             -1};

Private int phantom_offsets[] = {pp_1_offset, pp_2_offset, pp_3_offset, pp_4_offset};
Private int tandem_offsets[]  = {tt_1_offset, tt_2_offset, tt_3_offset, tt_4_offset};
Private int distort_offsets[] = {dd_1_offset, dd_2_offset, dd_3_offset, dd_4_offset};
Private int dist4_offsets[]   = {d4_1_offset, d4_2_offset, d4_3_offset, d4_4_offset};
Private int misc_c_offsets[]  = {mm_1_offset, mm_2_offset, mm_3_offset};

int general_concept_offset    =  gg_offset;

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
