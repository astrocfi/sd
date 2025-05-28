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

    This is for version 28. */

/* This defines the following external variables:
   special_magic
   special_interlocked
   mark_end_of_list
   marker_decline
   marker_concept_mod
   marker_concept_modreact
   marker_concept_modtag
   marker_concept_force
   marker_concept_comment
   main_call_lists
   number_of_calls
   calling_level
   concept_descriptor_table
   nice_setup_concept
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

#define tt__1 14
   {"AS COUPLES",                            concept_tandem,                l_a1,         {0, FALSE, 0, 0, 1}},
   {"TANDEM",                                concept_tandem,                l_c1,         {0, FALSE, 0, 0, 0}},
   {"SIAMESE",                               concept_tandem,                l_c2,         {0, FALSE, 0, 0, 2}},
   {"COUPLES OF THREE",                      concept_tandem,                l_c1,         {0, FALSE, 0, 0, 5}},
   {"TANDEMS OF THREE",                      concept_tandem,                l_c1,         {0, FALSE, 0, 0, 4}},
   {"COUPLES OF FOUR",                       concept_tandem,                l_c1,         {0, FALSE, 0, 0, 7}},
   {"TANDEMS OF FOUR",                       concept_tandem,                l_c1,         {0, FALSE, 0, 0, 6}},
   {"BOXES WORK SOLID",                      concept_tandem,                l_c3,         {0, FALSE, 0, 0, 8}},
   {"DIAMONDS WORK SOLID",                   concept_tandem,                l_c3,         {0, FALSE, 0, 0, 9}},
   {"SKEW",                                  concept_tandem,                l_c2,         {0, FALSE, 0, 0, 3}},
   {"SO-AND-SO AS COUPLES",                  concept_some_are_tandem,       l_a1,         {0, TRUE,  0, 0, 1}},
   {"SO-AND-SO TANDEM",                      concept_some_are_tandem,       l_c1,         {0, TRUE,  0, 0, 0}},
   {"GRUESOME AS COUPLES",                   concept_gruesome_tandem,       l_c4a,        {0, FALSE, 0, 2, 1}},
   {"GRUESOME TANDEM",                       concept_gruesome_tandem,       l_c4a,        {0, FALSE, 0, 2, 0}},
/* -------- column break -------- */
#define tt__2 14
   {"COUPLES TWOSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 1}},
   {"TANDEM TWOSOME",                        concept_tandem,                l_c3,         {0, FALSE, 1, 0, 0}},
   {"SIAMESE TWOSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 2}},
   {"COUPLES THREESOME",                     concept_tandem,                l_c3,         {0, FALSE, 1, 0, 5}},
   {"TANDEM THREESOME",                      concept_tandem,                l_c3,         {0, FALSE, 1, 0, 4}},
   {"COUPLES FOURSOME",                      concept_tandem,                l_c3,         {0, FALSE, 1, 0, 7}},
   {"TANDEM FOURSOME",                       concept_tandem,                l_c3,         {0, FALSE, 1, 0, 6}},
   {"BOXSOME",                               concept_tandem,                l_c3,         {0, FALSE, 1, 0, 8}},
   {"DIAMONDSOME",                           concept_tandem,                l_c3,         {0, FALSE, 1, 0, 9}},
   {"SKEWSOME",                              concept_tandem,                l_c2,         {0, FALSE, 1, 0, 3}},
   {"SO-AND-SO COUPLES TWOSOME",             concept_some_are_tandem,       l_c3,         {0, TRUE,  1, 0, 1}},
   {"SO-AND-SO TANDEM TWOSOME",              concept_some_are_tandem,       l_c3,         {0, TRUE,  1, 0, 0}},
   {"GRUESOME TWOSOME",                      concept_gruesome_tandem,       l_c4a,        {0, FALSE, 1, 2, 1}},
   {"GRUESOME TANDEM TWOSOME",               concept_gruesome_tandem,       l_c4a,        {0, FALSE, 1, 2, 0}},
/* -------- column break -------- */
#define tt__3 14
   {"COUPLES N/4 TWOSOME",                   concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 1}},
   {"TANDEM N/4 TWOSOME",                    concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 0}},
   {"SIAMESE N/4 TWOSOME",                   concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 2}},
   {"COUPLES OF 3 N/4 THREESOME",            concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 5}},
   {"TANDEMS OF 3 N/4 THREESOME",            concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 4}},
   {"COUPLES OF 4 N/4 FOURSOME",             concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 7}},
   {"TANDEMS OF 4 N/4 FOURSOME",             concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 6}},
   {"BOXES ARE SOLID N/4 BOXSOME",           concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 8}},
   {"DIAMONDS ARE SOLID N/4 DIAMONDSOME",    concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 9}},
   {"SKEW N/4 TWOSOME",                      concept_frac_tandem,           l_c4,         {0, FALSE, 2, 0, 3}},
   {"SO-AND-SO COUPLES N/4 TWOSOME",         concept_some_are_frac_tandem,  l_c4,         {0, TRUE,  2, 0, 1}},
   {"SO-AND-SO TANDEM N/4 TWOSOME",          concept_some_are_frac_tandem,  l_c4,         {0, TRUE,  2, 0, 0}},
   {"GRUESOME AS COUPLES N/4 TWOSOME",       concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 2, 2, 1}},
   {"GRUESOME TANDEM N/4 TWOSOME",           concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 2, 2, 0}},
/* -------- column break -------- */
#define tt__4 14
   {"COUPLES TWOSOME N/4 SOLID",             concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 1}},
   {"TANDEM TWOSOME N/4 SOLID",              concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 0}},
   {"SIAMESE TWOSOME N/4 SOLID",             concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 2}},
   {"COUPLES THREESOME N/4 SOLID",           concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 5}},
   {"TANDEM THREESOME N/4 SOLID",            concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 4}},
   {"COUPLES FOURSOME N/4 SOLID",            concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 7}},
   {"TANDEM FOURSOME N/4 SOLID",             concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 6}},
   {"BOXSOME N/4 SOLID",                     concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 8}},
   {"DIAMONDSOME N/4 SOLID",                 concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 9}},
   {"SKEWSOME N/4 SOLID",                    concept_frac_tandem,           l_c4,         {0, FALSE, 3, 0, 3}},
   {"SO-AND-SO COUPLES TWOSOME N/4 SOLID",   concept_some_are_frac_tandem,  l_c4,         {0, TRUE,  3, 0, 1}},
   {"SO-AND-SO TANDEM TWOSOME N/4 SOLID",    concept_some_are_frac_tandem,  l_c4,         {0, TRUE,  3, 0, 0}},
   {"GRUESOME TWOSOME N/4 SOLID",            concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 3, 2, 1}},
   {"GRUESOME TANDEM TWOSOME N/4 SOLID",     concept_gruesome_frac_tandem,  l_c4,         {0, FALSE, 3, 2, 0}},

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

#define mm__1 12
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
   {"16 MATRIX",                             concept_16_matrix,             l_c3a},
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
   {"STRETCH",                               concept_old_stretch,           l_c1},
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
#define mm__4 37
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
   {"CROSS",                                 concept_cross,                 l_mainstream},
   {"GRAND",                                 concept_grand,                 l_plus},
   {"MAGIC",                                 concept_magic,                 l_c1},
   {"INTERLOCKED",                           concept_interlocked,           l_c1},
   {"12 MATRIX",                             concept_12_matrix,             l_c3a},
   {"16 MATRIX",                             concept_16_matrix,             l_c3a},
   {"2X6 MATRIX",                            concept_2x6_matrix,            l_c3a},
   {"2X8 MATRIX",                            concept_2x8_matrix,            l_c3a},
   {"4X4 MATRIX",                            concept_4x4_matrix,            l_c3a},
   {"16 MATRIX OF PARALLEL DIAMONDS",        concept_4dmd_matrix,           l_c3a},
   {"PHANTOM",                               concept_c1_phantom,            l_c1},
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
