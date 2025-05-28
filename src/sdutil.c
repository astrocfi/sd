/* SD -- square dance caller's helper.

    Copyright (C) 1990-1996  William B. Ackerman.

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

/* This defines the following functions:
   initialize_restr_tables
   get_restriction_thing
   clear_screen
   writestuff
   newline
   doublespace_file
   exit_program
   fail
   fail2
   failp
   specialfail
   get_escape_string
   string_copy
   display_initial_history
   write_history_line
   warn
   find_proper_call_list
   assoc
   find_calldef
   clear_people
   rotperson
   rotcw
   rotccw
   clear_person
   copy_person
   copy_rot
   swap_people
   install_person
   install_rot
   process_final_concepts
   skip_one_concept
   fix_n_results

and the following external variables:
   longjmp_buffer
   longjmp_ptr
   history
   history_allocation
   history_ptr
   written_history_items
   written_history_nopic
   last_magic_diamond
   error_message1
   error_message2
   collision_person1
   collision_person2
   enable_file_writing
   singlespace_mode
   cardinals
   ordinals
   selector_list
   direction_names
   last_direction_kind
   warning_strings
and the following external variable that is declared only in sdui-ttu.h:
   ui_directions
*/

/* For "sprintf" */
#include <stdio.h>
#include <string.h>
#include "sd.h"



/*  This is the line length beyond which we will take pity on
   whatever device has to print the result, and break the text line.
   It is presumably smaller than our internal text capacity. */
# define MAX_PRINT_LENGTH 59

/* These variables are external. */

real_jmp_buf longjmp_buffer;
real_jmp_buf *longjmp_ptr;
configuration *history = (configuration *) 0; /* allocated in sdmain */
int history_allocation = 0; /* How many items are currently allocated in "history". */
int history_ptr;

/* This tells how much of the history text written to the UI is "safe".  If this
   is positive, the history items up to that number, inclusive, have valid
   "text_line" fields, and have had their text written to the UI.  Furthermore,
   the local variable "text_line_count" is >= any of those "text_line" fields,
   and each "text_line" field shows the number of lines of text that were
   written to the UI when that history item was written.  This variable is
   -1 when none of the history is safe or the state of the text in the UI
   is unknown. */
int written_history_items;
/* When written_history_items is positive, this tells how many of the initial lines
   did not have pictures forced on (other than having been drawn as a natural consequence
   of the "draw_pic" flag being on.)  If this number ever becomes greater than
   written_history_items, that's OK.  It just means that none of the lines had
   forced pictures. */
int written_history_nopic;

parse_block *last_magic_diamond;
char error_message1[MAX_ERR_LENGTH];
char error_message2[MAX_ERR_LENGTH];
uint32 collision_person1;
uint32 collision_person2;
long_boolean enable_file_writing;
long_boolean singlespace_mode;

Cstring cardinals[] = {"1", "2", "3", "4", "5", "6", "7", "8", (Cstring) 0};
Cstring ordinals[] = {"1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", (Cstring) 0};

/* BEWARE!!  This list is keyed to the definition of "selector_kind" in sd.h,
   and to the necessary stuff in SDUI. */
selector_item selector_list[] = {
   {"???",          "???",         "???",          "???",         selector_uninitialized},
   {"boys",         "boy",         "BOYS",         "BOY",         selector_girls},
   {"girls",        "girl",        "GIRLS",        "GIRL",        selector_boys},
   {"heads",        "head",        "HEADS",        "HEAD",        selector_sides},
   {"sides",        "side",        "SIDES",        "SIDE",        selector_heads},
   {"head corners", "head corner", "HEAD CORNERS", "HEAD CORNER", selector_sidecorners},
   {"side corners", "side corner", "SIDE CORNERS", "SIDE CORNER", selector_headcorners},
   {"head boys",    "head boy",    "HEAD BOYS",    "HEAD BOY",    selector_uninitialized},
   {"head girls",   "head girl",   "HEAD GIRLS",   "HEAD GIRL",   selector_uninitialized},
   {"side boys",    "side boy",    "SIDE BOYS",    "SIDE BOY",    selector_uninitialized},
   {"side girls",   "side girl",   "SIDE GIRLS",   "SIDE GIRL",   selector_uninitialized},
   {"centers",      "center",      "CENTERS",      "CENTER",      selector_ends},
   {"ends",         "end",         "ENDS",         "END",         selector_centers},
   {"leads",        "lead",        "LEADS",        "LEAD",        selector_trailers},
   {"trailers",     "trailer",     "TRAILERS",     "TRAILER",     selector_leads},
   {"beaus",        "beau",        "BEAUS",        "BEAU",        selector_belles},
   {"belles",       "belle",       "BELLES",       "BELLE",       selector_beaus},
   {"center 2",     "center 2",    "CENTER 2",     "CENTER 2",    selector_outer6},
   {"center 6",     "center 6",    "CENTER 6",     "CENTER 6",    selector_outer2},
   {"outer 2",      "outer 2",     "OUTER 2",      "OUTER 2",     selector_center6},
   {"outer 6",      "outer 6",     "OUTER 6",      "OUTER 6",     selector_center2},
   {"center diamond", "center diamond", "CENTER DIAMOND", "CENTER DIAMOND",    selector_uninitialized},
   {"center 1x4",   "center 1x4",  "CENTER 1X4",   "CENTER 1X4",  selector_uninitialized},
   {"center 1x6",   "center 1x6",  "CENTER 1X6",   "CENTER 1X6",  selector_uninitialized},
   {"center 4",     "center 4",    "CENTER 4",     "CENTER 4",    selector_outerpairs},
   {"outer pairs",  "outer pair",  "OUTER PAIRS",  "OUTER PAIR",  selector_center4},
   {"headliners",   "headliner",   "HEADLINERS",   "HEADLINER",   selector_sideliners},
   {"sideliners",   "sideliner",   "SIDELINERS",   "SIDELINER",   selector_headliners},
   {"near line",    "near line",   "NEAR LINE",    "NEAR LINE",   selector_uninitialized},
   {"far line",     "far line",    "FAR LINE",     "FAR LINE",    selector_uninitialized},
   {"near column",  "near column", "NEAR COLUMN",  "NEAR COLUMN", selector_uninitialized},
   {"far column",   "far column",  "FAR COLUMN",   "FAR COLUMN",  selector_uninitialized},
   {"near box",     "near box",    "NEAR BOX",     "NEAR BOX",    selector_uninitialized},
   {"far box",      "far box",     "FAR BOX",      "FAR BOX",     selector_uninitialized},
   {"everyone",     "everyone",    "EVERYONE",     "EVERYONE",    selector_uninitialized},
   {"no one",       "no one",      "NO ONE",       "NO ONE",      selector_uninitialized},
   {(Cstring) 0,    (Cstring) 0,   (Cstring) 0,    (Cstring) 0,   selector_uninitialized}};


/* BEWARE!!  This list is keyed to the definition of "direction_kind" in sd.h,
   and to the necessary stuff in SDUI. */
/* This array, and the variable "last_direction_kind" below, get manipulated
   at startup in order to remove the "zig-zag" items below A2. */
Cstring direction_names[] = {
   "???",
   "(no direction)",
   "left",
   "right",
   "in",
   "out",
   "zig-zag",
   "zag-zig",
   "zig-zig",
   "zag-zag",
   (Cstring) 0};

int last_direction_kind = direction_zagzag;


/* BEWARE!!  These strings are keyed to the definition of "warning_index" in sd.h . */
/* A "*" as the first character means that this warning precludes acceptance while searching. */
/* A "+" as the first character means that this warning is cleared if a concentric call was done
   and the "suppress_elongation_warnings" flag was on. */
/* A "=" as the first character means that this warning is cleared if it arises during some kind of
   "do your part" call. */
Cstring warning_strings[] = {
   /*  warn__none                */   " Unknown warning????",
   /*  warn__do_your_part        */   "*Do your part.",
   /*  warn__tbonephantom        */   " This is a T-bone phantom setup call.  Everyone will do their own part.",
   /*  warn__awkward_centers     */   "*Awkward for centers.",
   /*  warn__bad_concept_level   */   "*This concept is not allowed at this level.",
   /*  warn__not_funny           */   "*That wasn't funny.",
   /*  warn__hard_funny          */   "*Very difficult funny concept.",
   /*  warn__unusual             */   "*This is an unusual setup for this call.",
   /*  warn__rear_back           */   "*Rear back from the handhold.",
   /*  warn__awful_rear_back     */   "*Rear back from the handhold -- this is very unusual.",
   /*  warn__excess_split        */   "*Split concept seems to be superfluous here.",
   /*  warn__lineconc_perp       */   "+Ends should opt for setup perpendicular to their original line.",
   /*  warn__dmdconc_perp        */   "+Ends should opt for setup perpendicular to their original diamond points.",
   /*  warn__lineconc_par        */   "+Ends should opt for setup parallel to their original line -- concentric rule does not apply.",
   /*  warn__dmdconc_par         */   "+Ends should opt for setup parallel to their original diamond points -- concentric rule does not apply.",
   /*  warn__xclineconc_perpc    */   "+New ends should opt for setup perpendicular to their original (center) line.",
   /*  warn__xcdmdconc_perpc     */   "+New ends should opt for setup perpendicular to their original (center) diamond points.",
   /*  warn__xclineconc_perpe    */   "+New ends should opt for setup perpendicular to the original ends' line.",
   /*  warn__ctrstand_endscpls   */   " Centers work in tandem, ends as couples.",
   /*  warn__ctrscpls_endstand   */   " Centers work as couples, ends in tandem.",
   /*  warn__each2x2             */   "=Each 2x2.",
   /*  warn__each1x4             */   "=Each 1x4.",
   /*  warn__each1x2             */   "=Each 1x2.",
   /*  warn__take_right_hands    */   " Take right hands.",
   /*  warn__ctrs_are_dmd        */   " The centers are the diamond.",
   /*  warn__full_pgram          */   " Completely offset parallelogram.",
   /*  warn__offset_gone         */   " The offset goes away.",
   /*  warn__overlap_gone        */   " The overlap goes away.",
   /*  warn__to_o_spots          */   " Go back to 'O' spots.",
   /*  warn__to_x_spots          */   " Go back to butterfly spots.",
   /*  warn__check_butterfly     */   " Check a butterfly.",
   /*  warn__some_rear_back      */   " Some people rear back.",
   /*  warn__not_tbone_person    */   " Work with the person to whom you are not T-boned.",
   /*  warn__check_c1_phan       */   " Check a 'C1 phantom' setup.",
   /*  warn__check_dmd_qtag      */   " Fudge to a diamond/quarter-tag setup.",
   /*  warn__check_2x4           */   " Check a 2x4 setup.",
   /*  warn__check_4x4           */   "*Check a 4x4 setup at the start of this call.",
   /*  warn__check_pgram         */   " Opt for a parallelogram.",
   /*  warn__dyp_resolve_ok      */   " Do your part.",
   /*  warn__ctrs_stay_in_ctr    */   " Centers stay in the center.",
   /*  warn__check_c1_stars      */   " Check a generalized 'star' setup.",
   /*  warn__bigblock_feet       */   " Bigblock/stagger shapechanger -- go to footprints.",
   /*  warn__adjust_to_feet      */   " Adjust back to footprints.",
   /*  warn__some_touch          */   " Some people step to a wave.",
   /*  warn__split_to_2x4s       */   " Do the call in each 2x4.",
   /*  warn__split_to_2x3s       */   " Do the call in each 2x3.",
   /*  warn__split_to_1x8s       */   " Do the call in each 1x8.",
   /*  warn__split_to_1x6s       */   " Do the call in each 1x6.",
   /*  warn__take_left_hands     */   " Take left hands, since this call is being done mirror.",
   /*  warn__evil_interlocked    */   " Interlocked phantom shape-changers are very evil.",
   /*  warn__split_phan_in_pgram */   " The split phantom setups are directly adjacent to the real people.",
   /*  warn__bad_interlace_match */   "*The interlaced calls have mismatched lengths.",
   /*  warn__not_on_block_spots  */   " Generalized bigblock/stagger -- people are not on block spots.",
   /*  warn__bad_modifier_level  */   "*Use of this modifier on this call is not allowed at this level.",
   /*  warn__did_not_interact    */   "*The setups did not interact with each other.",
   /*  warn__opt_for_normal_cast */   "*If in doubt, assume a normal cast."};




Private restriction_thing wave_2x4      = {4, {0, 2, 5, 7},                {1, 3, 4, 6},                   {0}, {0}, TRUE, chk_wave};            /* check for two parallel consistent waves */
Private restriction_thing jleft_qtag    = {4, {2, 0, 7, 1},                {3, 4, 6, 5},                   {0}, {0}, TRUE, chk_wave};
Private restriction_thing jright_qtag   = {4, {3, 0, 6, 1},                {2, 4, 7, 5},                   {0}, {0}, TRUE, chk_wave};
Private restriction_thing ijleft_qtag   = {4, {2, 0, 3, 1},                {6, 4, 7, 5},                   {0}, {0}, TRUE, chk_wave};
Private restriction_thing ijright_qtag  = {4, {6, 0, 7, 1},                {2, 4, 3, 5},                   {0}, {0}, TRUE, chk_wave};
Private restriction_thing two_faced_2x4 = {4, {0, 1, 6, 7},                {3, 2, 5, 4},                   {0}, {0}, TRUE, chk_wave};            /* check for two parallel consistent two-faced lines */
Private restriction_thing wave_3x4      = {6, {0, 2, 5, 7, 9, 10},         {1, 3, 4, 6, 8, 11},            {0}, {0}, TRUE, chk_wave};            /* check for three parallel consistent waves */
Private restriction_thing two_faced_3x4 = {6, {0, 1, 8, 9, 10, 11},        {2, 3, 4, 5, 6, 7},             {0}, {0}, TRUE, chk_wave};            /* check for three parallel consistent two-faced lines */
Private restriction_thing wave_1x2      = {1, {0},                         {1},                            {0}, {0}, TRUE, chk_wave};            /* check for a miniwave -- note this is NOT OK for assume */
Private restriction_thing wave_1x4      = {2, {0, 3},                      {1, 2},                         {0}, {0}, TRUE, chk_wave};            /* check for a wave */
Private restriction_thing wave_1x6      = {3, {0, 2, 4},                   {1, 3, 5},                      {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 6 */
Private restriction_thing wave_1x8      = {4, {0, 3, 6, 5},                {1, 2, 7, 4},                   {0}, {0}, TRUE, chk_wave};            /* check for grand wave */
Private restriction_thing wave_1x10     = {5, {0, 2, 4, 6, 8},             {1, 3, 5, 7, 9},                {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 10 */
Private restriction_thing wave_1x12     = {6, {0, 2, 4, 7, 9, 11},         {1, 3, 5, 6, 8, 10},            {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 12 */
Private restriction_thing wave_2x3      = {3, {0, 2, 4},                   {1, 3, 5},                      {0}, {0}, FALSE, chk_wave};           /* check for two "waves" -- people are antitandem */
Private restriction_thing wave_2x6      = {6, {0, 2, 4, 7, 9, 11},         {1, 3, 5, 6, 8, 10},            {0}, {0}, TRUE, chk_wave};            /* check for parallel consistent 2x6 waves */
Private restriction_thing wave_1x14     = {7, {0, 2, 4, 6, 8, 10, 12},     {1, 3, 5, 7, 9, 11, 13},        {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 14 */
Private restriction_thing wave_1x16     = {8, {0, 2, 4, 6, 9, 11, 13, 15}, {1, 3, 5, 7, 8, 10, 12, 14},    {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 16 */
Private restriction_thing wave_2x8      = {8, {0, 2, 4, 6, 9, 11, 13, 15}, {1, 3, 5, 7, 8, 10, 12, 14},    {0}, {0}, TRUE, chk_wave};            /* check for parallel 2x8 waves */

Private restriction_thing cwave_2x4     = {4, {0, 1, 2, 3},                {4, 5, 6, 7},                   {0}, {0}, TRUE, chk_wave};            /* check for real columns */
Private restriction_thing cwave_2x3     = {3, {0, 1, 2},                   {3, 4, 5},                      {0}, {0}, TRUE, chk_wave};            /* check for real columns of 6 */
Private restriction_thing cwave_2x6     = {6, {0, 1, 2, 3, 4, 5},          {6, 7, 8, 9, 10, 11},           {0}, {0}, TRUE, chk_wave};            /* check for real 12-matrix columns */
Private restriction_thing cwave_2x8     = {8, {0, 1, 2, 3, 4, 5, 6, 7},    {8, 9, 10, 11, 12, 13, 14, 15}, {0}, {0}, TRUE, chk_wave};            /* check for real 16-matrix columns */
Private restriction_thing cmagic_2x3    = {3, {0, 2, 4},                   {1, 3, 5},                      {0}, {0}, TRUE, chk_wave};            /* check for magic columns */
Private restriction_thing cmagic_2x4    = {4, {0, 3, 5, 6},                {1, 2, 4, 7},                   {0}, {0}, TRUE, chk_wave};            /* check for magic columns */

Private restriction_thing lio_2x4       = {4, {4, 5, 6, 7},                {0, 1, 2, 3},                   {0}, {0}, TRUE, chk_wave};            /* check for lines in or lines out */
Private restriction_thing invert_2x4    = {4, {0, 3, 5, 6},                {1, 2, 4, 7},                   {0}, {0}, TRUE, chk_wave};            /* check for inverted lines or magic columns */
Private restriction_thing invert_1x4    = {2, {0, 2},                      {1, 3},                         {0}, {0}, TRUE, chk_wave};            /* check for single inverted line */
Private restriction_thing invert_2x3    = {3, {0, 2, 4},                   {1, 3, 5},                      {0}, {0}, TRUE, chk_wave};            /* check for magic columns of 3 */

Private restriction_thing peelable_3x2  = {3, {0, 1, 2},                   {3, 4, 5},                      {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x3 column */
Private restriction_thing peelable_4x2  = {4, {0, 1, 2, 3},                {4, 5, 6, 7},                   {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x4 column */
Private restriction_thing peelable_6x2  = {6, {0, 1, 2, 3, 4, 5},          {6, 7, 8, 9, 10, 11},           {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x6 column */
Private restriction_thing peelable_8x2  = {8, {0, 1, 2, 3, 4, 5, 6, 7},    {8, 9, 10, 11, 12, 13, 14, 15}, {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x8 column */

Private restriction_thing all_same_2    = {2, {0, 1},                      {0},                            {0}, {0}, FALSE, chk_1_group};        /* check for a couple */
Private restriction_thing all_same_4    = {4, {0, 1, 2, 3},                {0},                            {0}, {0}, TRUE,  chk_1_group};        /* check for a 1-faced line */
Private restriction_thing all_same_6    = {6, {0, 1, 2, 3, 4, 5},          {0},                            {0}, {0}, FALSE, chk_1_group};        /* check for a 1-faced line */
Private restriction_thing all_same_8    = {8, {0, 1, 2, 3, 4, 5, 6, 7},    {0},                            {0}, {0}, FALSE, chk_1_group};        /* check for all 8 people same way in 2x4 *OR* 1x8. */


Private restriction_thing two_faced_1x6 = {3, {0, 1, 2},                   {3, 4, 5},                      {0}, {0}, FALSE, chk_wave};           /* check for 3x3 two-faced line -- 3 up and 3 down */
Private restriction_thing two_faced_1x8 = {4, {0, 1, 6, 7},                {3, 2, 5, 4},                   {0}, {0}, FALSE, chk_wave};           /* check for grand two-faced line */
Private restriction_thing one_faced_2x3 = {3, {0, 3, 1, 4, 2, 5},                                     {0}, {0}, {0}, FALSE, chk_2_groups};       /* check for parallel one-faced lines of 3 */
Private restriction_thing one_faced_2x4 = {4, {0, 4, 1, 5, 2, 6, 3, 7},                               {0}, {0}, {0}, FALSE, chk_2_groups};       /* check for parallel one-faced lines */
Private restriction_thing cpls_2x4      = {2, {0, 2, 4, 6, 1, 3, 5, 7},                               {0}, {0}, {0}, FALSE, chk_4_groups};       /* check for everyone as a couple */
Private restriction_thing cpls_4x2      = {2, {0, 1, 2, 3, 7, 6, 5, 4},                               {0}, {0}, {0}, FALSE, chk_4_groups};       /* check for everyone as a couple */
Private restriction_thing cpls_1x4      = {2, {0, 2, 1, 3},                                           {0}, {0}, {0}, FALSE, chk_2_groups};       /* check for everyone as a couple */
Private restriction_thing cpls_1x8      = {2, {0, 2, 4, 6, 1, 3, 5, 7},                               {0}, {0}, {0}, FALSE, chk_4_groups};       /* check for everyone as a couple */
Private restriction_thing cpls_2x8      = {2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {0}, {0}, {0}, FALSE, chk_8_groups};       /* check for everyone as a couple */
Private restriction_thing cpls_1x16     = {2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {0}, {0}, {0}, FALSE, chk_8_groups};       /* check for everyone as a couple */
Private restriction_thing cpls_2x6      = {3, {0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11},                 {0}, {0}, {0}, FALSE, chk_4_groups};       /* check for each three people facing same way */
Private restriction_thing cplsof4_2x8   = {4, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15}, {0}, {0}, {0}, FALSE, chk_4_groups};       /* check for each four people facing same way */
Private restriction_thing cpls_1x3      = {3, {0, 1, 2},                                              {0}, {0}, {0}, FALSE, chk_1_group};        /* check for three people facing same way */
Private restriction_thing cpls_3x3_1x6  = {3, {0, 3, 1, 4, 2, 5},                                     {0}, {0}, {0}, FALSE, chk_2_groups};       /* check for each three people facing same way */
Private restriction_thing cpls_4x4_1x8  = {4, {0, 4, 1, 5, 2, 6, 3, 7},                               {0}, {0}, {0}, FALSE, chk_2_groups};       /* check for each four people facing same way */
Private restriction_thing two_faced_4x4_1x8 = {4, {0, 1, 2, 3},            {4, 5, 6, 7},                   {0}, {0}, FALSE, chk_wave};           /* check for 4x4 two-faced line -- 4 up and 4 down */
Private restriction_thing two_faced_1x4 = {2, {0, 1},                      {2, 3},                         {0}, {0}, TRUE,  chk_wave};           /* check for 2-faced lines */
Private restriction_thing two_faced_2x6 = {6, {0, 1, 2, 9, 10, 11},        {3, 4, 5, 6, 7, 8},             {0}, {0}, FALSE, chk_wave};           /* check for parallel consistent 3x3 two-faced lines */
Private restriction_thing two_faced_4x4_2x8 = {8, {0, 1, 2, 3, 12, 13, 14, 15}, {4, 5, 6, 7, 8, 9, 10, 11},{0}, {0}, FALSE, chk_wave};           /* check for parallel consistent 4x4 two-faced lines */

Private restriction_thing box_wave      = {0, {2, 0, 0, 2},                {0, 0, 2, 2},                   {0}, {0}, TRUE,  chk_box};            /* check for a "real" (walk-and-dodge type) box */
Private restriction_thing box_1face     = {0, {2, 2, 2, 2},                {0, 0, 0, 0},                   {0}, {0}, FALSE, chk_box};            /* check for a "one-faced" (reverse-the-pass type) box */
Private restriction_thing box_magic     = {0, {2, 0, 2, 0},                {0, 2, 0, 2},                   {0}, {0}, TRUE,  chk_box};            /* check for a "magic" (split-trade-circulate type) box */
Private restriction_thing s4x4_wave     = {0,   {2, 0, 2, 0, 0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0},
                                                {0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0, 2, 0, 2, 0},          {0}, {0}, FALSE, chk_box};            /* check for 4 waves of consistent handedness and consistent headliner-ness. */
Private restriction_thing s4x4_2fl     = {0,   {2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2},
                                                {0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 0, 2},          {0}, {0}, FALSE, chk_box};            /* check for 4 waves of consistent handedness and consistent headliner-ness. */

Private restriction_thing cwave_qtg     = {2, {2, 7},                      {3, 6},                         {0}, {0}, FALSE, chk_wave};           /* check for wave across the center */
Private restriction_thing wave_qtag     = {2, {2, 7},                      {3, 6},                         {0}, {0}, FALSE, chk_wave};           /* check for wave across the center */
Private restriction_thing two_faced_qtag= {2, {6, 7},                      {2, 3},                         {0}, {0}, FALSE, chk_wave};           /* check for two-faced line across the center */

Private restriction_thing qtag_1        = {4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},                {2, 4, 5}, {2, 0, 1}, FALSE, chk_dmd_qtag};
Private restriction_thing dmd_1         = {4, {0}, {4, 0, 1, 2, 3},                            {1, 0},    {1, 2},    FALSE, chk_dmd_qtag};
Private restriction_thing ptpd_1        = {4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},                {2, 0, 6}, {2, 2, 4}, FALSE, chk_dmd_qtag};
Private restriction_thing qtag_3        = {4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},                {2, 0, 1}, {2, 4, 5}, FALSE, chk_dmd_qtag};
Private restriction_thing dmd_3         = {4, {0}, {4, 0, 1, 2, 3},                            {1, 2},    {1, 0},    FALSE, chk_dmd_qtag};
Private restriction_thing ptpd_3        = {4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},                {2, 2, 4}, {2, 0, 6}, FALSE, chk_dmd_qtag};
Private restriction_thing dmd_q         = {4, {0}, {4, 0, 1, 2, 3},                            {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing qtag_d        = {4, {4, 2, 3, 6, 7}, {4, 0, 1, 4, 5},                {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing dmd_d         = {4, {2, 0, 2}, {2, 1, 3},                            {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing ptpd_d        = {4, {4, 0, 2, 4, 7}, {4, 1, 3, 5, 6},                {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing all_4_ns      = {4, {4, 0, 1, 2, 3}, {0},                            {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing all_4_ew      = {4, {0}, {4, 0, 1, 2, 3},                            {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing all_8_ns      = {4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},                {0},       {0},       FALSE, chk_dmd_qtag};
Private restriction_thing all_8_ew      = {4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},                {0},       {0},       FALSE, chk_dmd_qtag};



/* Must be a power of 2. */
#define NUM_RESTR_HASH_BUCKETS 32

typedef struct grilch {
   setup_kind k;
   call_restriction restr;
   restriction_thing *value;
   struct grilch *next;
} restr_initializer;


static restr_initializer restr_init_table[] = {
   {s1x8, cr_wave_only, &wave_1x8},
   {s1x8, cr_1fl_only, &all_same_8},
   {s1x8, cr_all_facing_same, &all_same_8},
   {s1x8, cr_2fl_only, &two_faced_1x8},
   {s2x4, cr_4x4couples_only, &cpls_4x4_1x8},
   {s1x8, cr_4x4couples_only, &cpls_4x4_1x8},
   {s1x8, cr_4x4_2fl_only, &two_faced_4x4_1x8},
   {s1x8, cr_couples_only, &cpls_1x8},
   {s1x3, cr_3x3couples_only, &cpls_1x3},
   {s1x4, cr_wave_only, &wave_1x4},
   {s1x4, cr_2fl_only, &two_faced_1x4},
   {s1x4, cr_1fl_only, &all_same_4},
   {s1x4, cr_4x4couples_only, &all_same_4},
   {s1x4, cr_all_facing_same, &all_same_4},
   {s1x4, cr_couples_only, &cpls_1x4},
   {s1x4, cr_magic_only, &invert_1x4},
   {s1x4, cr_all_ns, &all_4_ns},
   {s1x4, cr_all_ew, &all_4_ew},
   {s2x4, cr_all_ns, &all_8_ns},
   {s2x4, cr_all_ew, &all_8_ew},
   {s1x8, cr_all_ns, &all_8_ns},
   {s1x8, cr_all_ew, &all_8_ew},
   {s1x6, cr_wave_only, &wave_1x6},
   {s1x6, cr_1fl_only, &all_same_6},
   {s1x6, cr_all_facing_same, &all_same_6},
   {s1x6, cr_3x3_2fl_only, &two_faced_1x6},
   {s1x6, cr_3x3couples_only, &cpls_3x3_1x6},
   {s2x3, cr_3x3couples_only, &cpls_3x3_1x6},
   {s1x10, cr_wave_only, &wave_1x10},
   {s1x12, cr_wave_only, &wave_1x12},
   {s1x14, cr_wave_only, &wave_1x14},
   {s1x16, cr_wave_only, &wave_1x16},
   {s1x16, cr_couples_only, &cpls_1x16},
   {s2x3, cr_all_facing_same, &all_same_6},
   {s2x3, cr_1fl_only, &one_faced_2x3},
   {s2x4, cr_2fl_only, &two_faced_2x4},
   {s2x4, cr_wave_only, &wave_2x4},
   {s2x3, cr_wave_only, &wave_2x3},
   {s2x3, cr_magic_only, &invert_2x3},
   {s2x4, cr_magic_only, &invert_2x4},
   {s2x4, cr_li_lo, &lio_2x4},
   {s2x4, cr_all_facing_same, &all_same_8},
   {s2x4, cr_1fl_only, &one_faced_2x4},
   {s2x4, cr_couples_only, &cpls_2x4},
   {s_qtag, cr_wave_only, &wave_qtag},
   {s_qtag, cr_2fl_only, &two_faced_qtag},
   {s2x8, cr_wave_only, &wave_2x8},
   {s2x8, cr_4x4_2fl_only, &two_faced_4x4_2x8},
   {s1x16, cr_4x4couples_only, &cplsof4_2x8},
   {s2x8, cr_4x4couples_only, &cplsof4_2x8},
   {s2x8, cr_couples_only, &cpls_2x8},
   {s2x6, cr_wave_only, &wave_2x6},
   {s2x6, cr_3x3_2fl_only, &two_faced_2x6},
   {s2x6, cr_3x3couples_only, &cpls_2x6},
   {s1x12, cr_3x3couples_only, &cpls_2x6},
   {s1x2, cr_wave_only, &wave_1x2},
   {s1x2, cr_2fl_only, &all_same_2},
   {s1x2, cr_couples_only, &all_same_2},
   {s3x4, cr_wave_only, &wave_3x4},
   {s3x4, cr_2fl_only, &two_faced_3x4},
   {nothing}
};


static restr_initializer *restr_hash_table[NUM_RESTR_HASH_BUCKETS];


extern void initialize_restr_tables(void)
{
   restr_initializer *tabp;
   for (tabp = restr_init_table ; tabp->k != nothing ; tabp++) {
      uint32 hash_num = ((tabp->k + (5*tabp->restr)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
      tabp->next = restr_hash_table[hash_num];
      restr_hash_table[hash_num] = tabp;
   }
}


extern restriction_thing *get_restriction_thing(setup_kind k, assumption_thing t)
{
   restriction_thing *restr_thing_ptr = (restriction_thing *) 0;

   /* First, use the hash table to search for easy cases. */

   if (t.assump_col == 0) {
      uint32 hash_num = ((k + (5*t.assumption)) * 25) & (NUM_RESTR_HASH_BUCKETS-1);
      restr_initializer *restr_hash_bucket = restr_hash_table[hash_num];
      while (restr_hash_bucket) {
         if (restr_hash_bucket->k == k && restr_hash_bucket->restr == t.assumption)
            return restr_hash_bucket->value;
         restr_hash_bucket = restr_hash_bucket->next;
      }
   }

   switch (k) {
      case s2x2:
         if (t.assumption == cr_wave_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &box_wave;
         else if (t.assumption == cr_all_facing_same && (t.assump_col & 1) == 0)
            restr_thing_ptr = &box_1face;
         else if (t.assumption == cr_2fl_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &box_1face;
         else if (t.assumption == cr_magic_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &box_magic;
         break;
      case s4x4:
         if (t.assumption == cr_wave_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &s4x4_wave;
         if (t.assumption == cr_2fl_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &s4x4_2fl;
         break;
      case s2x3:
         if (t.assumption == cr_wave_only && t.assump_col != 0)
            restr_thing_ptr = &cwave_2x3;
         else if (t.assumption == cr_magic_only && t.assump_col != 0)
            restr_thing_ptr = &cmagic_2x3;
         else if (t.assumption == cr_peelable_box && t.assump_col != 0)
            restr_thing_ptr = &peelable_3x2;
         break;
/*     try it without this
      case s1x4:
         if (t.assumption == cr_2fl_only && t.assump_col != 0)
            restr_thing_ptr = &two_faced_1x4;
*/
      case s2x4:
         if (t.assumption == cr_li_lo && t.assump_col != 0)
            restr_thing_ptr = &wave_2x4;
         else if (t.assumption == cr_2fl_only && t.assump_col != 0)
            restr_thing_ptr = &two_faced_2x4;
         else if (t.assumption == cr_wave_only && t.assump_col != 0)
            restr_thing_ptr = &cwave_2x4;
         else if (t.assumption == cr_magic_only && t.assump_col != 0)
            restr_thing_ptr = &cmagic_2x4;
         else if (t.assumption == cr_peelable_box && t.assump_col != 0)
            restr_thing_ptr = &peelable_4x2;
         else if (t.assumption == cr_couples_only && t.assump_col == 1)
            restr_thing_ptr = &cpls_4x2;
         break;
      case s1x2:
         if (t.assumption == cr_wave_only && t.assump_col == 2)
            restr_thing_ptr = &wave_1x2;
         break;
      case s2x6:
         if (t.assumption == cr_wave_only && t.assump_col != 0)
            restr_thing_ptr = &cwave_2x6;
         else if (t.assumption == cr_peelable_box && t.assump_col != 0)
            restr_thing_ptr = &peelable_6x2;
         break;
      case s2x8:
         if (t.assumption == cr_wave_only && t.assump_col != 0)
            restr_thing_ptr = &cwave_2x8;
         else if (t.assumption == cr_peelable_box && t.assump_col != 0)
            restr_thing_ptr = &peelable_8x2;
         break;
      case s_qtag:
         if (t.assumption == cr_wave_only && t.assump_col == 1)
            restr_thing_ptr = &cwave_qtg;
         else if (t.assumption == cr_jleft)
            restr_thing_ptr = &jleft_qtag;
         else if (t.assumption == cr_jright)
            restr_thing_ptr = &jright_qtag;
         else if (t.assumption == cr_ijleft)
            restr_thing_ptr = &ijleft_qtag;
         else if (t.assumption == cr_ijright)
            restr_thing_ptr = &ijright_qtag;
         else if (t.assumption == cr_diamond_like)
            restr_thing_ptr = &qtag_d;
         else if (t.assumption == cr_qtag_like)
            restr_thing_ptr = &all_8_ns;
         else if (t.assumption == cr_gen_1_4_tag)
            restr_thing_ptr = &qtag_1;
         else if (t.assumption == cr_gen_3_4_tag)
            restr_thing_ptr = &qtag_3;
         break;
      case sdmd:
         if (t.assumption == cr_diamond_like)
            restr_thing_ptr = &dmd_d;
         else if (t.assumption == cr_qtag_like)
            restr_thing_ptr = &dmd_q;
         else if (t.assumption == cr_gen_1_4_tag)
            restr_thing_ptr = &dmd_1;
         else if (t.assumption == cr_gen_3_4_tag)
            restr_thing_ptr = &dmd_3;
         break;
      case s_ptpd:
         if (t.assumption == cr_diamond_like)
            restr_thing_ptr = &ptpd_d;
         else if (t.assumption == cr_qtag_like)
            restr_thing_ptr = &all_8_ew;
         else if (t.assumption == cr_gen_1_4_tag)
            restr_thing_ptr = &ptpd_1;
         else if (t.assumption == cr_gen_3_4_tag)
            restr_thing_ptr = &ptpd_3;
         break;
   }

   return restr_thing_ptr;
}



/* These variables are used by the text-packing stuff. */

static char *destcurr;
static char lastchar;
static char lastlastchar;
static char *lastblank;
static char current_line[MAX_TEXT_LINE_LENGTH];
static int text_line_count = 0;



Private void open_text_line(void)
{
   destcurr = current_line;
   lastchar = ' ';
   lastlastchar = ' ';
   lastblank = (char *) 0;
}


extern void clear_screen(void)
{
   written_history_items = -1;
   text_line_count = 0;
   uims_reduce_line_count(0);
   open_text_line();
}


Private void writechar(char src)
{
   lastlastchar = lastchar;

   *destcurr = (lastchar = src);
   if (src == ' ' && destcurr != current_line) lastblank = destcurr;

   if (destcurr < &current_line[MAX_PRINT_LENGTH])
      destcurr++;
   else {
      /* Line overflow.  Try to write everything up to the last
         blank, then fill next line with everything since that blank. */

      char save_buffer[MAX_TEXT_LINE_LENGTH];
      char *q = save_buffer;

      if (lastblank) {
         char *p = lastblank+1;
         while (p <= destcurr) *q++ = *p++;
         destcurr = lastblank;
      }
      else {
         /* Must break a word. */
         *q++ = *destcurr;
      }

      *q = '\0';

      newline();            /* End the current buffer and write it out. */
      writestuff("   ");    /* Make a new line, copying saved stuff into it. */
      writestuff(save_buffer);
   }
}


/* Getting blanks into all the right places in the presence of substitions,
   insertions, and elisions is way too complicated to do in the database or
   to test.  For example, consider calls like "@4keep @5busy@1",
   "fascinat@pe@q@ning@o@t", or "spin the pulley@7 but @8".  So we try to
   help by never putting two blanks together, always putting blanks adjacent
   to the outside of brackets, and never putting blanks adjacent to the
   inside of brackets.  This procedure is part of that mechanism. */
Private void write_blank_if_needed(void)
{
   if (lastchar != ' ' && lastchar != '[' && lastchar != '(' && lastchar != '-') writestuff(" ");
}



extern void newline(void)
{
   /* Erase any trailing blanks.  Failure to do so can lead to some
      incredibly obscure bugs when some editors on PC's try to "fix"
      the file, ultimately leading to apparent loss of entire sequences.
      The problem was that the editor took out the trailing blanks
      and moved the rest of the text downward, but didn't reduce the
      file's byte count.  Control Z's were thereby introduced at the
      end of the file.  Subsequent runs of Sd appended new sequences
      to the file, leaving the ^Z's intact.  (Making Sd look for ^Z's
      at the end of a file it is about to append to, and remove same,
      is very hard.)  It turns out that some printing software stops,
      as though it reached the end of the file, when it encounters a
      ^Z, so the appended sequences were effectively lost. */
   while (destcurr != current_line && destcurr[-1] == ' ') destcurr--;

   *destcurr = '\0';

   if (enable_file_writing)
      write_file(current_line);

   text_line_count++;
   uims_add_new_line(current_line);
   open_text_line();
}



extern void writestuff(Const char s[])
{
   Const char *f = s;
   while (*f) writechar(*f++);
}



Private void write_nice_number(char indicator, uint32 num)
{
   char nn;

   switch (indicator) {
      case '9': case 'a': case 'b': case 'B':
         nn = '0' + num;
         if (indicator == '9')
            writechar(nn);
         else if (indicator == 'B') {
            if (num == 1)
               writestuff("quarter");
            else if (num == 2)
               writestuff("half");
            else if (num == 3)
               writestuff("three quarter");
            else if (num == 4)
               writestuff("four quarter");
            else {
               writechar(nn);
               writestuff("/4");
            }
         }
         else if (num == 2)
            writestuff("1/2");
         else if (num == 4 && indicator == 'a')
            writestuff("full");
         else {
            writechar(nn);
            writestuff("/4");
         }
         break;
      case 'u':     /* Need to plug in an ordinal number. */
         writestuff(ordinals[num-1]);
         break;
   }
}


Private void writestuff_with_decorations(parse_block *cptr, Const char *s)
{
   int index = cptr->number;
   Const char *f;

   f = s;     /* Argument "s", if non-null, overrides the concept name in the table. */
   if (!f) f = cptr->concept->name;

   while (*f) {
      if (f[0] == '@') {
         switch (f[1]) {
            case 'a': case 'b': case 'B': case 'u': case '9':
               write_nice_number(f[1], index & 0xF);
               f += 2;
               index >>= 4;
               continue;
            case '6':
               writestuff(selector_list[cptr->selector].name_uc);
               f += 2;
               continue;
            case 'k':
               writestuff(selector_list[cptr->selector].sing_name_uc);
               f += 2;
               continue;
         }
      }
      else if (f[0] == '<' && f[1] == 'c' && f[2] == 'o' && f[3] == 'n' && f[4] == 'c' && f[5] == 'e' && f[6] == 'p' && f[7] == 't' && f[8] == '>') {
         f += 9;
         continue;
      }

      writechar(*f++);
   }
}



extern void doublespace_file(void)
{
   write_file("");
}



extern void exit_program(int code)
{
   uims_terminate();
   final_exit(code);
}


extern void nonreturning fail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   longjmp(longjmp_ptr->the_buf, 1);
}


extern void nonreturning fail2(Const char s1[], Const char s2[])
{
   (void) strncpy(error_message1, s1, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   (void) strncpy(error_message2, s2, MAX_ERR_LENGTH);
   error_message2[MAX_ERR_LENGTH-1] = '\0';
   longjmp(longjmp_ptr->the_buf, 2);
}


extern void nonreturning failp(uint32 id1, Const char s[])
{
   collision_person1 = id1;
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   longjmp(longjmp_ptr->the_buf, 6);
}


extern void nonreturning specialfail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   longjmp(longjmp_ptr->the_buf, 4);
}


/* This examines the indicator character after an "@" escape.  If the escape
   is for something that is supposed to appear in the menu as a "<...>"
   wildcard, it returns that string.  If it is an escape that starts a
   substring that would normally be elided, as in "@7 ... @8", it returns
   a non-null pointer to a null character.  If it is an escape that is normally
   simply dropped, it returns a null pointer. */

extern Const char *get_escape_string(char c)
{
   switch (c) {
      case '0': case 'm': case 'N':
         return "<ANYTHING>";
      case '6': case 'k':
         return "<ANYONE>";
      case 'h':
         return "<DIRECTION>";
      case '9':
         return "<N>";
      case 'a': case 'b': case 'B':
         return "<N/4>";
      case 'u':
         return "<Nth>";
      case 'v': case 'w': case 'x': case 'y':
         return "<ATC>";
      case '7': case 'n': case 'j': case 'J': case 'E':
         return "";
      default:
         return (char *) 0;
   }
}


extern void string_copy(char **dest, Cstring src)
{
   Cstring f = src;
   char *t = *dest;

   while ((*t++ = *f++));
   *dest = t-1;
}

/* There are 2 bits that are meaningful in the argument to "print_recurse":
         PRINT_RECURSE_STAR
   This means to print an asterisk for a call that is missing in the
   current type-in state.
         PRINT_RECURSE_CIRC
   This mean that this is a circulate-substitute call, and should have any
   @O ... @P stuff elided from it. */

#define PRINT_RECURSE_STAR 1
#define PRINT_RECURSE_CIRC 2


Private void print_recurse(parse_block *thing, int print_recurse_arg)
{
   parse_block *local_cptr;
   parse_block *next_cptr;
   long_boolean use_left_name = FALSE;
   long_boolean use_cross_name = FALSE;
   long_boolean use_magic_name = FALSE;
   long_boolean use_intlk_name = FALSE;
   long_boolean comma_after_next_concept = FALSE;


   long_boolean did_concept = FALSE;
   long_boolean last_was_t_type = FALSE;
   long_boolean last_was_l_type = FALSE;
   long_boolean did_comma = FALSE;
   long_boolean request_final_space = FALSE;




   local_cptr = thing;

   while (local_cptr) {
      int i;
      concept_kind k;
      concept_descriptor *item;

      item = local_cptr->concept;
      k = item->kind;

      if (k == concept_comment) {
         comment_block *fubb;

         fubb = (comment_block *) local_cptr->call;
         if (request_final_space) writestuff(" ");
         writestuff("{ ");
         writestuff(fubb->txt);
         writestuff(" } ");
         local_cptr = local_cptr->next;
         request_final_space = FALSE;
         last_was_t_type = FALSE;
         last_was_l_type = FALSE;
         comma_after_next_concept = FALSE;
      }
      else if (k > marker_end_of_list) {
         /* This is a concept. */

         long_boolean force = FALSE;
         long_boolean request_comma_after_next_concept = FALSE;

         /* Some concepts look better with a comma after them. */

         if (     k == concept_so_and_so_stable ||
                  k == concept_so_and_so_frac_stable ||
                  k == concept_so_and_so_begin ||
                  k == concept_fractional ||
                  k == concept_twice ||
                  k == concept_n_times ||
                  k == concept_some_are_tandem ||
                  (k == concept_snag_mystic && (item->value.arg1 & CMD_MISC2__CTR_END_INV_CONC)) ||  /* INVERT SNAG or INVERT MYSTIC */
                  k == concept_some_are_frac_tandem ||
                  (    (k == concept_tandem ||           /* The arg4 test picks out the more esoteric */
                        k == concept_frac_tandem) &&     /* things like "<some setup> work solid". */
                     item->value.arg4 >= 10) ||
                  (     k == concept_nth_part &&
                                             /* "SKIP THE <Nth> PART" or "SHIFT <N>" */
                              (item->value.arg1 == 9 || item->value.arg1 == 10)) ||
                  /* The arg1 test picks out "do your part" or "ignore". */
                  (     (k == concept_so_and_so_only || k == concept_some_vs_others)
                                    &&
                        (item->value.arg1 < 2 || item->value.arg1 == 8))) {
            /* This is an "F" type concept. */
            comma_after_next_concept = TRUE;
            last_was_t_type = FALSE;
            force = did_concept && !last_was_l_type;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }
         else if (k == concept_reverse ||
                  k == concept_left ||
                  k == concept_grand ||
                  k == concept_magic ||
                  k == concept_cross ||
                  k == concept_1x2 ||
                  k == concept_2x1 ||
                  k == concept_2x2 ||
                  k == concept_1x3 ||
                  k == concept_3x1 ||
                  k == concept_3x3 ||
                  k == concept_4x4 ||
                  k == concept_single ||
                  k == concept_singlefile ||
                  k == concept_interlocked ||
                  k == concept_standard ||
                  k == concept_fan ||
                  (k == concept_snag_mystic && item->value.arg1 == CMD_MISC2__CTR_END_INVERT) ||    /* INVERT */
                  (k == concept_nth_part && item->value.arg1 == 8) ||
                  (k == concept_so_and_so_only && item->value.arg1 == 11) ||
                  k == concept_c1_phantom ||
                  k == concept_yoyo) {
            /* This is an "L" type concept. */
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else if ((k == concept_meta && (item->value.arg1 <= 3 || item->value.arg1 == 7)) ||
                  k == concept_matrix) {
            /* This is a "leading T/trailing L" type concept. */
            force = last_was_t_type && !last_was_l_type;;
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else {
            /* This is a "T" type concept. */
            comma_after_next_concept |= did_concept;
            force = last_was_t_type && !last_was_l_type;
            last_was_t_type = TRUE;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }

         if (force && !did_comma) writestuff(", ");
         else if (request_final_space) writestuff(" ");

         next_cptr = local_cptr->next;    /* Now it points to the thing after this concept. */

         request_final_space = FALSE;

         if (concept_table[k].concept_prop & CONCPROP__SECOND_CALL) {
            parse_block *subsidiary_ptr = local_cptr->subsidiary_root;

            if (k == concept_centers_and_ends) {
               if (item->value.arg1 == selector_center6)
                  writestuff("CENTER 6 ");
               else if (item->value.arg1 == selector_center2)
                  writestuff("CENTER 2 ");
               else
                  writestuff("CENTERS ");
            }
            else if (k == concept_some_vs_others) {
               if ((i = item->value.arg1) == 1) {
                  writestuff_with_decorations(local_cptr, "DO YOUR PART, @6 ");
               }
               else if (i == 3)
                  writestuff_with_decorations(local_cptr, "OWN THE @6, ");
               else if (i == 5)
                  writestuff_with_decorations(local_cptr, "@6 ");
               else
                  writestuff_with_decorations(local_cptr, "@6 DISCONNECTED ");
            }
            else if (k == concept_sequential) {
               writestuff("(");
            }
            else if (k == concept_replace_nth_part) {
               writestuff("DELAY: ");
               if (!local_cptr->next || !subsidiary_ptr) {
                  if (local_cptr->concept->value.arg1)
                     writestuff("(interrupting after the ");
                  else
                     writestuff("(replacing the ");
                  writestuff(ordinals[local_cptr->number-1]);
                  writestuff(" part) ");
               }
            }
            else {
               writestuff(item->name);
               writestuff(" ");
            }

            print_recurse(local_cptr->next, 0);

            if (!subsidiary_ptr) break;         /* Can happen if echoing incomplete input. */

            did_concept = FALSE;                /* We're starting over. */
            last_was_t_type = FALSE;
            last_was_l_type = FALSE;
            comma_after_next_concept = FALSE;
            request_final_space = TRUE;

            if (k == concept_centers_and_ends) {
               if (item->value.arg2)
                  writestuff(" WHILE THE ENDS CONCENTRIC");
               else
                  writestuff(" WHILE THE ENDS");
            }
            else if (k == concept_some_vs_others && item->value.arg1 != 3) {
               selector_kind opp = selector_list[local_cptr->selector].opposite;
               writestuff(" WHILE THE ");
               writestuff((opp == selector_uninitialized) ? ((Cstring) "OTHERS") : selector_list[opp].name_uc);
            }
            else if (k == concept_on_your_own)
               writestuff(" AND");
            else if (k == concept_interlace)
               writestuff(" WITH");
            else if (k == concept_replace_nth_part) {
               writestuff(" BUT ");
               writestuff_with_decorations(local_cptr, (Const char *) 0);
               writestuff(" WITH A [");
               request_final_space = FALSE;
            }
            else if (k == concept_sequential)
               writestuff(" ;");
            else if (k == concept_special_sequential)
               writestuff(",");
            else
               writestuff(" BY");

            next_cptr = subsidiary_ptr;
         }
         else if (local_cptr && (k == concept_left || k == concept_cross || k == concept_magic || k == concept_interlocked)) {

            /* These concepts want to take special action if there are no following concepts and
               certain escape characters are found in the name of the following call. */

            final_set finaljunk;
            callspec_block *target_call;
            parse_block *tptr;
            
            /* Skip all final concepts, then demand that what remains is a marker (as opposed to a serious
                concept), and that a real call has been entered, and that its name starts with "@g". */
            tptr = process_final_concepts(next_cptr, FALSE, &finaljunk);

            if (tptr) {
               target_call = tptr->call;
   
               if ((tptr->concept->kind <= marker_end_of_list) && target_call && (
                     target_call->callflagsh & (ESCAPE_WORD__LEFT | ESCAPE_WORD__MAGIC | ESCAPE_WORD__CROSS | ESCAPE_WORD__INTLK))) {
                  if (k == concept_left) {
                     /* See if this is a call whose name naturally changes when the "left" concept is used. */
                     if (target_call->callflagsh & ESCAPE_WORD__LEFT) {
                        use_left_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else if (k == concept_magic) {
                     /* See if this is a call that wants the "magic" modifier to be moved inside its name. */
                     if (target_call->callflagsh & ESCAPE_WORD__MAGIC) {
                        use_magic_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else if (k == concept_interlocked) {
                     /* See if this is a call that wants the "interlocked" modifier to be moved inside its name. */
                     if (target_call->callflagsh & ESCAPE_WORD__INTLK) {
                        use_intlk_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else {
                     /* See if this is a call that wants the "cross" modifier to be moved inside its name. */
                     if (target_call->callflagsh & ESCAPE_WORD__CROSS) {
                        use_cross_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
               }
               else {
                  writestuff(item->name);
                  request_final_space = TRUE;
               }
            }
            else {
               writestuff(item->name);
               request_final_space = TRUE;
            }
         }
         else if (k == concept_nth_part && local_cptr->concept->value.arg1 == 8) {
            request_comma_after_next_concept = TRUE;   /* "DO THE <Nth> PART <concept>" */
            writestuff_with_decorations(local_cptr, (Const char *) 0);
         }
         else if ((k == concept_so_and_so_only) && local_cptr->concept->value.arg1 == 11) {
            request_comma_after_next_concept = TRUE;   /* "<ANYONE> WORK <concept>" */
            writestuff_with_decorations(local_cptr, (Const char *) 0);
         }
         else if (k == concept_snag_mystic && item->value.arg1 == CMD_MISC2__CTR_END_INVERT) {
            /* If INVERT is followed by another concept, it must be SNAG or MYSTIC.  Put a comma after it. */
            request_comma_after_next_concept = TRUE;
            writestuff_with_decorations(local_cptr, (Const char *) 0);
            request_final_space = TRUE;
         }
         else if ((k == concept_meta) && (local_cptr->concept->value.arg1 == 3 || local_cptr->concept->value.arg1 == 7)) {
            /* Initially/finally. */
            writestuff_with_decorations(local_cptr, (Const char *) 0);
            request_comma_after_next_concept = TRUE;
            request_final_space = TRUE;
         }
         else {
            writestuff_with_decorations(local_cptr, (Const char *) 0);
            request_final_space = TRUE;
         }

         if (comma_after_next_concept) {
            writestuff(",");
            request_final_space = TRUE;
         }

         did_comma = comma_after_next_concept;
         comma_after_next_concept = request_comma_after_next_concept;
         local_cptr = next_cptr;

         if (k == concept_sequential) {
            if (request_final_space) writestuff(" ");
            print_recurse(local_cptr, PRINT_RECURSE_STAR);
            writestuff(")");
            return;
         }
         else if (k == concept_replace_nth_part) {
            if (request_final_space) writestuff(" ");
            print_recurse(local_cptr, PRINT_RECURSE_STAR);
            writestuff("]");
            return;
         }
      }
      else {
         /* This is a "marker", so it has a call, perhaps with a selector and/or number.
            The call may be null if we are printing a partially entered line.  Beware. */

         parse_block *subsidiary_ptr;
         parse_block *sub1_ptr;
         parse_block *sub2_ptr;
         parse_block *search;
         long_boolean pending_subst1, subst1_in_use, this_is_subst1;
         long_boolean pending_subst2, subst2_in_use, this_is_subst2;

         selector_kind i16junk = local_cptr->selector;
         direction_kind idirjunk = local_cptr->direction;
         uint32 number_list = local_cptr->number;
         callspec_block *localcall = local_cptr->call;
         parse_block *save_cptr = local_cptr;

         subst1_in_use = FALSE;
         subst2_in_use = FALSE;

         if (request_final_space) writestuff(" ");

         if (k == concept_another_call_next_mod) {
            search = save_cptr->next;
            while (search) {
               this_is_subst1 = FALSE;
               this_is_subst2 = FALSE;
               subsidiary_ptr = search->subsidiary_root;
               if (subsidiary_ptr) {
                  switch ((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) {
                     case 1:
                     case 2:
                        this_is_subst1 = TRUE;
                        break;
                     case 5:
                     case 6:
                        this_is_subst2 = TRUE;
                        break;
                  }

                  if (this_is_subst1) {
                     if (subst1_in_use) writestuff("ERROR!!!");
                     subst1_in_use = TRUE;
                     sub1_ptr = subsidiary_ptr;
                  }

                  if (this_is_subst2) {
                     if (subst2_in_use) writestuff("ERROR!!!");
                     subst2_in_use = TRUE;
                     sub2_ptr = subsidiary_ptr;
                  }
               }
               search = search->next;
            }
         }
   
         pending_subst1 = subst1_in_use;
         pending_subst2 = subst2_in_use;

         /* Now "subst_in_use" is on if there is a replacement call that goes in naturally.  During the
            scan of the name, we will try to fit that replacement into the name of the call as directed
            by atsign-escapes.  If we succeed at this, we will clear "pending_subst".
            In addition to all of this, there may be any number of forcible replacements. */

         if (localcall) {      /* Call = NIL means we are echoing input and user hasn't entered call yet. */
            char *np;
            char savec;

            if (enable_file_writing) localcall->age = global_age;
            np = localcall->name;

            while (*np) {
               if (*np == '@') {
                  savec = np[1];

                  switch (savec) {
                     case '6': case 'k':
                        write_blank_if_needed();
                        if (savec == '6')
                           writestuff(selector_list[i16junk].name);
                        else
                           writestuff(selector_list[i16junk].sing_name);
                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the indicator */
                        break;
                     case 'v': case 'w': case 'x': case 'y':
                        write_blank_if_needed();

                        /* Find the base tag call that this is invoking. */

                        search = save_cptr->next;
                        while (search) {
                           subsidiary_ptr = search->subsidiary_root;
                           if (subsidiary_ptr && subsidiary_ptr->call && (subsidiary_ptr->call->callflags1 & CFLAG1_BASE_TAG_CALL_MASK)) {
                              print_recurse(subsidiary_ptr, 0);
                              goto did_tagger;
                           }
                           search = search->next;
                        }

                        /* We didn't find the tagger.  It must not have been entered into the parse tree.
                           See if we can get it from the "tagger" field. */

                        if (save_cptr->tagger > 0)
                           writestuff(tagger_calls[save_cptr->tagger >> 5][(save_cptr->tagger & 0x1F)-1]->menu_name);
                        else
                           writestuff("NO TAGGER???");

                        did_tagger:

                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the indicator */
                        break;
                     case 'N':
                        write_blank_if_needed();

                        /* Find the base circ call that this is invoking. */

                        search = save_cptr->next;
                        while (search) {
                           subsidiary_ptr = search->subsidiary_root;
                           if (subsidiary_ptr && subsidiary_ptr->call && (subsidiary_ptr->call->callflags1 & CFLAG1_BASE_CIRC_CALL)) {
                              print_recurse(subsidiary_ptr, PRINT_RECURSE_CIRC);
                              goto did_circcer;
                           }
                           search = search->next;
                        }

                        /* We didn't find the circcer.  It must not have been entered into the parse tree.
                           See if we can get it from the "circcer" field. */

                        if (save_cptr->circcer > 0)
                           writestuff(circcer_calls[(save_cptr->circcer)-1]->menu_name);
                        else
                           writestuff("NO CIRCCER???");

                        did_circcer:

                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the indicator */
                        break;
                     case 'h':                   /* Need to plug in a direction. */
                        write_blank_if_needed();
                        writestuff(direction_names[idirjunk]);
                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the indicator */
                        break;
                     case '9': case 'a': case 'b': case 'B': case 'u':    /* Need to plug in a number. */
                        write_blank_if_needed();
                        write_nice_number(savec, number_list & 0xF);
                        number_list >>= 4;    /* Get ready for next number. */
                        np += 2;              /* skip the indicator */
                        break;
                     case 'e':
                        if (use_left_name) {
                           np += 2;
                           while (*np != '@') np++;
                           if (lastchar == ']') writestuff(" ");
                           writestuff("left");
                        }
                        np += 2;
                        break;
                     case 'j':
                        if (!use_cross_name) {
                           np += 2;
                           while (*np != '@') np++;
                        }
                        np += 2;
                        break;
                     case 'C':
                        if (use_cross_name) {
                           if (lastchar != ' ' && lastchar != '[') writechar(' ');
                           writestuff("cross");
                        }
                        np += 2;
                        break;
                     case 'J':
                        if (!use_magic_name) {
                           np += 2;
                           while (*np != '@') np++;
                        }
                        np += 2;
                        break;
                     case 'M':
                        if (use_magic_name) {
                           if (lastchar != ' ' && lastchar != '[') writechar(' ');
                           writestuff("magic");
                        }
                        np += 2;
                        break;
                     case 'E':
                        if (!use_intlk_name) {
                           np += 2;
                           while (*np != '@') np++;
                        }
                        np += 2;
                        break;
                     case 'I':
                        if (use_intlk_name) {
                           if (lastchar == 'a' && lastlastchar == ' ')
                              writestuff("n ");
                           else if (lastchar != ' ' && lastchar != '[')
                              writechar(' ');
                           writestuff("interlocked");
                        }
                        np += 2;
                        break;
                     case 'l': case 'L': case 'F': case '8': case 'o':    /* Just skip these -- they end stuff that we could have elided but didn't. */
                        np += 2;
                        break;
                     case 'n': case 'p': case 'r': case 'm': case 't':
                        if (subst2_in_use) {
                           if (savec == 'p' || savec == 'r') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
                        else {
                           if (savec == 'n') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
   
                        if (pending_subst2 && savec != 'p' && savec != 'n') {
                           write_blank_if_needed();
                           writestuff("[");
                           print_recurse(sub2_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
         
                           pending_subst2 = FALSE;
                        }
         
                        np += 2;        /* skip the digit */
                        break;
                     case 'O':
                        if (print_recurse_arg & PRINT_RECURSE_CIRC) {
                           np += 2;
                           while (*np != '@') np++;
                        }

                        np += 2;        /* skip the digit */
                        break;
                     default:
                        if (subst1_in_use) {
                           if (savec == '2' || savec == '4') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
                        else {
                           if (savec == '7') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
         
                        if (pending_subst1 && savec != '4' && savec != '7') {
                           write_blank_if_needed();
                           writestuff("[");
                           print_recurse(sub1_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
         
                           pending_subst1 = FALSE;
                        }
         
                        np += 2;        /* skip the digit */
                        break;
                  }
               }
               else {
                  char c = *np++;

                  if (lastchar == ']' && c != ' ' && c != ']')
                     writestuff(" ");

                  if ((lastchar != ' ' && lastchar != '[') || c != ' ') writechar(c);
               }
            }

            if (lastchar == ']' && *np && *np != ' ' && *np != ']')
               writestuff(" ");
         }
         else if (print_recurse_arg & PRINT_RECURSE_STAR) {
            writestuff("*");
         }

         /* Now if "pending_subst" is still on, we have to do by hand what should have been
            a natural replacement.  In any case, we have to find forcible replacements and
            report them. */
   
         if (k == concept_another_call_next_mod) {
            int first_replace = 0;

            search = save_cptr->next;
            while (search) {
               subsidiary_ptr = search->subsidiary_root;
               /* If we have a subsidiary_ptr, handle the replacement that is indicated.
                  BUT:  if the call shown in the subsidiary_ptr is a base tag call, don't
                  do anything -- such substitutions were already taken care of.
                     BUT:  only check if there is actually a call there. */

               if (subsidiary_ptr &&
                           (!(subsidiary_ptr->call) ||    /* If no call pointer, it isn't a tag base call. */
                           !(subsidiary_ptr->call->callflags1 & (CFLAG1_BASE_TAG_CALL_MASK | CFLAG1_BASE_CIRC_CALL)))) {
                  long_boolean not_turning_star = FALSE;

                  switch ((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) {
                     case 1:
                     case 2:
                     case 3:
                        /* This is a natural replacement.  It may already have been taken care of. */
                        if (pending_subst1 || ((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) == 3) {
                           write_blank_if_needed();
                           if (((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) == 3)
                              writestuff("but [");
                           else
                              writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     case 5:
                     case 6:
                        /* This is a secondary replacement.  It may already have been taken care of. */
                        if (pending_subst2) {
                           write_blank_if_needed();
                           writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     default:
                        /* This is a forced replacement.  Need to check for case of replacing
                           one star turn with another. */
                        localcall = search->call;
                        write_blank_if_needed();
                        if ((first_replace++ == 0) && subsidiary_ptr &&
                              (localcall->callflags1 & CFLAG1_IS_STAR_CALL) &&
                                    ((subsidiary_ptr->concept->kind == marker_end_of_list) ||
                                    subsidiary_ptr->concept->kind == concept_another_call_next_mod) &&
                              subsidiary_ptr->call &&
                                    ((subsidiary_ptr->call->callflags1 & CFLAG1_IS_STAR_CALL) ||
                                    subsidiary_ptr->call->schema == schema_nothing)) {

                           not_turning_star = subsidiary_ptr->call->schema == schema_nothing;

                           if (not_turning_star)
                              writestuff("BUT don't turn the star");
                           else
                              writestuff("BUT [");
                        }
                        else {
                           if (first_replace == 1)
                              writestuff("BUT REPLACE ");
                           else
                              writestuff("AND REPLACE ");
                           writestuff(localcall->menu_name);
                           writestuff(" WITH [");
                        }
   
                        if (!not_turning_star) {
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                  }
               }
               search = search->next;
            }
         }

         break;
      }
   }

   return;
}


/* These static variables are used by printperson. */

static char peoplenames[] = "1234";
static char directions[] = "?>?<????^?V?????";
static char personbuffer[] = " ZZZ";

/* This could get changed if a user interface for sdtty wishes to use pretty graphics characters. */
char *ui_directions = directions;


Private void printperson(unsigned int x)
{
   if (x & BIT_PERSON) {
      int i = (x & 017);

      personbuffer[1] = peoplenames[(x >> 7) & 3];
      personbuffer[2] = (x & 0100) ? 'G' : 'B';

      if (enable_file_writing) {
         personbuffer[3] = directions[i];
      }
      else {
         personbuffer[3] = ui_directions[i];
      }

      writestuff(personbuffer);
   }
   else
      writestuff("  . ");
}

/* These static variables are used by printsetup/print_4_person_setup/do_write. */

static int offs, roti, modulus, personstart;
static setup *printarg;

Private void do_write(Cstring s)
{
   char c;

   int ri = roti * 011;

   for (;;) {
      if (!(c=(*s++))) return;
      else if (c == '@') newline();
      else if (c == ' ') writestuff(" ");
      else if (c >= 'a' && c <= 'x')
         printperson(rotperson(printarg->people[personstart + ((c-'a'-offs) % modulus)].id1, ri));
      else writestuff("?");
   }
}



Private void print_4_person_setup(int ps, small_setup *s, int elong)
{
   Cstring str;

   modulus = setup_attrs[s->skind].setup_limits+1;
   roti = (s->srotation & 3);
   personstart = ps;

   offs = (((roti >> 1) & 1) * (modulus / 2)) - modulus;

   if (s->skind == s2x2) {
      if (elong < 0)
         str = "ab@dc@";
      else if (elong & 1)
         str = "ab@@@dc@";
      else
         str = "a    b@d    c@";
   }
   else
      str = setup_attrs[s->skind].print_strings[roti & 1];

   if (str) {
      newline();
      do_write(str);
   }
   else
      writestuff(" ????");

   newline();
}



Private void printsetup(setup *x)
{
   Cstring str;

   printarg = x;
   modulus = setup_attrs[x->kind].setup_limits+1;
   roti = x->rotation & 3;
   
   newline();

   personstart = 0;

   if (setup_attrs[x->kind].four_way_symmetry) {
      /* still to do???
         s_1x1
         s2x2
         s_star
         s_hyperglass */

      offs = (roti * (modulus / 4)) - modulus;
      str = setup_attrs[x->kind].print_strings[0];
   }
   else {
      offs = ((roti & 2) * (modulus / 4)) - modulus;
      str = setup_attrs[x->kind].print_strings[roti & 1];
   }

   if (str)
      do_write(str);
   else {
      switch (x->kind) {
         case s_qtag:
            if ((x->people[0].id1 & x->people[1].id1 & x->people[4].id1 & x->people[5].id1 & 1) &&
                  (x->people[2].id1 & x->people[3].id1 & x->people[6].id1 & x->people[7].id1 & 010)) {
               /* People are in diamond-like orientation. */
               if (x->rotation & 1)
                  do_write("      g@f        a@      h@@      d@e        b@      c");
               else {
                  do_write("   a     b@@g h d c@@   f     e");
               }
            }
            else {
               /* People are not.  Probably 1/4-tag-like orientation. */
               if (x->rotation & 1)
                  do_write("      g@f  h  a@e  d  b@      c");
               else {
                  do_write("      a  b@@g  h  d  c@@      f  e");
               }
            }
            break;
         case s_normal_concentric:
            writestuff(" centers:");
            newline();
            print_4_person_setup(0, &(x->inner), -1);
            writestuff(" ends:");
            newline();
            print_4_person_setup(12, &(x->outer), x->concsetup_outer_elongation);
            break;
         default:
            writestuff("???? UNKNOWN SETUP ????");
      }
   }

   newline();
   newline();
}


/* Clear the screen and display the initial part of the history.
   This attempts to re-use material already displayed on the screen.
   The "num_pics" argument tells how many of the last history items
   are to have pictures forced, so we can tell exactly what items
   have pictures. */
extern void display_initial_history(int upper_limit, int num_pics)
{
   int j, startpoint, compilerbug;

   /* See if we can re-use some of the safely written history. */
   /* First, cut down overly optimistic estimates. */
   if (written_history_items > upper_limit) written_history_items = upper_limit;
   /* And normalize the "nopic" number. */
   if (written_history_nopic > written_history_items) written_history_nopic = written_history_items;

   /* Check whether pictures are faithful.  If any item in the written history has its
      "picture forced" property (as determined by written_history_nopic)
      different from what we want that property to be (as determined by upper_limit-num_pics),
      and that item didn't have the draw_pic flag on, that item needs to be rewritten.
      We cut written_history_items down to below that item if such is the case. */

   for (j=1; j<=written_history_items; j++) {
      compilerbug = ((int) ((unsigned int) (written_history_nopic-j)) ^
                 ((unsigned int) (upper_limit-num_pics-j)));
      if (compilerbug < 0 && ~history[j].draw_pic) {
         written_history_items = j-1;
         break;
      }
   }

   if (written_history_items > 0) {
      /* We win.  Back up the text line count to the right place, and rewrite the rest. */

      text_line_count = history[written_history_items].text_line;
      uims_reduce_line_count(text_line_count);
      open_text_line();
      startpoint = written_history_items+1;
   }
   else {
      /* We lose, there is nothing we can use. */
      clear_screen();
#ifndef THINK_C			/* Mac interface provides "About Sd" popup instead */
      write_header_stuff(TRUE, 0);
      newline();
      newline();
#endif
      startpoint = 1;
   }

   for (j=startpoint; j<=upper_limit-num_pics; j++) write_history_line(j, (char *) 0, FALSE, file_write_no);

   /* Now write stuff with forced pictures. */

   for (j=upper_limit-num_pics+1; j<=upper_limit; j++) {
      if (j >= startpoint) write_history_line(j, (char *) 0, TRUE, file_write_no);
   }

   written_history_items = upper_limit;   /* This stuff is now safe. */
   written_history_nopic = written_history_items-num_pics;
}



extern void write_history_line(int history_index, Const char *header, long_boolean picture, file_write_flag write_to_file)
{
   int centersp, w, i;
   parse_block *thing;

   if (write_to_file == file_write_double && !singlespace_mode)
      doublespace_file();

   centersp = history[history_index].centersp;

   /* Do not put index numbers into output file -- user may edit it later. */

   if (!enable_file_writing && !diagnostic_mode) {
      i = history_index-whole_sequence_low_lim+1;
      if (i > 0) {
         char indexbuf[10];
         sprintf(indexbuf, "%2d:   ", i);
         writestuff(indexbuf);
      }
   }

   if (centersp != 0) {
      if (startinfolist[centersp].into_the_middle) goto morefinal;
      writestuff(startinfolist[centersp].name);
      goto final;
   }

   thing = history[history_index].command_root;
   
   /* Need to check for the special case of starting a sequence with heads or sides.
      If this is the first line of the history, and we started with heads of sides,
      change the name of this concept from "centers" to the appropriate thing. */

   if (history_index == 2 && thing->concept->kind == concept_centers_or_ends && thing->concept->value.arg1 == selector_centers) {
      centersp = history[1].centersp;
      if (startinfolist[centersp].into_the_middle) {
         writestuff(startinfolist[centersp].name);
         writestuff(" ");
         thing = thing->next;
      }
   }
   
   print_recurse(thing, 0);
   
   final:

   newline();

   morefinal:

   /* Check for warnings to print. */
   /* Do not double space them, even if writing final output. */

   for (w=0 ; w<NUM_WARNINGS ; w++) {
      if (((1 << (w & 0x1f)) & history[history_index].warnings.bits[w>>5]) != 0) {
         writestuff("  Warning:  ");
         writestuff(&warning_strings[w][1]);
         newline();
      }
   }

   if (picture || history[history_index].draw_pic) {
      printsetup(&history[history_index].state);
   }

   /* Record that this history item has been written to the UI. */
   history[history_index].text_line = text_line_count;
}


extern void warn(warning_index w)
{
   if (w != warn__none) history[history_ptr+1].warnings.bits[w>>5] |= 1 << (w & 0x1F);
}


extern call_list_kind find_proper_call_list(setup *s)
{
   if (s->kind == s1x8) {
      if      ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_1x8;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_l1x8;
      else if ((s->people[0].id1 & 015) == 1 &&
               (s->people[1].id1 & 015) == 1 &&
               (s->people[2].id1 & 015) == 1 &&
               (s->people[3].id1 & 015) == 1 &&
               (s->people[4].id1 & 015) == 1 &&
               (s->people[5].id1 & 015) == 1 &&
               (s->people[6].id1 & 015) == 1 &&
               (s->people[7].id1 & 015) == 1)
         return call_list_gcol;
   }
   else if (s->kind == s2x4) {
      if      ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_dpt;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_cdpt;
      else if ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_rcol;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_lcol;
      else if ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_8ch;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_tby;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_lin;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_lout;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_rwv;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_lwv;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_r2fl;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_l2fl;
   }
   else if (s->kind == s_qtag)
      return call_list_qtag;

   return call_list_any;
}


static short spindle1[] = {d_east, d_west, 0, 6, 1, 5, 2, 4, -1};
static short short1[] = {d_north, d_south, 0, 2, 5, 3, -1};
static short dmd1[] = {d_east, d_west, 1, 3, -1};
static short qtag1[] = {d_north, d_south, 3, 2, 6, 7, -1};
static short ptpd1[] = {d_east, d_west, 1, 3, 7, 5, -1};



extern callarray *assoc(begin_kind key, setup *ss, callarray *spec)
{
   callarray *p;

   for (p = spec; p; p = p->next) {
      uint32 i, k, t, u, v, w, mask;
      assumption_thing tt;
      int idx, limit, j;
      uint32 qa0, qa1;
      restriction_thing *rr;

      /* First, we demand that the starting setup be correct.  Also, if a qualifier
         number was specified, it must match. */

      if ((begin_kind) p->start_setup != key) continue;

      /* During initialization, we will be called with a null pointer for ss.
         We need to be careful, and err on the side of acceptance. */

      if (!ss) goto good;

      if (p->qual_num != 0) {
         number_used = TRUE;
         if (p->qual_num != (current_number_fields & 0xF)+1) continue;
      }

      if ((search_qualifier) p->qualifier == sq_none) goto good;

      /* Note that we have to examine setups larger than the setup the
         qualifier is officially defined for.  If a qualifier were defined
         as being legal only on 1x4's (so that, in the database, we only had
         specifications of the sort "setup 1x4 1x4 qualifier wave_only") we could
         still find ourselves here with ss->kind equal to s2x4.  Why?  Because
         the setup could be a 2x4 and the splitter could be trying to decide
         whether to split the setup into parallel 1x4's.  This happens when
         trying to figure out whether to split a 2x4 into 1x4's or 2x2's for
         the call "recycle". */

      k = 0;   /* Many tests will find these values useful. */
      i = 2;
      tt.assump_col = 0;
      tt.assump_both = 0;
      tt.assump_cast = 0;

      switch ((search_qualifier) p->qualifier) {
         case sq_wave_only:                    /* 1x4 or 2x4 - waves; 3x2 or 4x2 - magic columns; 2x2 - real RH or LH box */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_wave_only;

            switch (ss->kind) {
               case s1x4: case s2x4:
                  rr = get_restriction_thing(ss->kind, tt);
                  if (rr) goto check_stuff;
                  goto good;
               case s2x2:
                  u = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;

                  if ((u & 1) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |=  t; i &=  t; }
                     if (!(k & ~i & 2)) goto good;
                  }
                  else if ((u & 010) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                     if (!(k & ~i & 2)) goto good;
                  }

                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_magic_only:                   /* 3x2 or 4x2 - magic column; 2x4 - inverted lines; 1x4 - single inverted line; 2x2 - inverted box */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
               case cr_wave_only:
                  goto bad;
            }

            tt.assumption = cr_magic_only;

            switch (ss->kind) {
               case s1x4: case s2x4:
                  rr = get_restriction_thing(ss->kind, tt);
                  if (rr) goto check_stuff;
                  goto good;
               case s2x2:
                  u = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;

                  if ((u & 1) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[2].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                     if (!(k & ~i & 2)) goto good;
                  }
                  else if ((u & 010) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[2].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                     if (!(k & ~i & 2)) goto good;
                  }

                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_in_or_out:                    /* 2x2 - all facing in or all facing out */
            switch (ss->kind) {
               case s2x2:
                  u = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;

                  if ((u & 1) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                     if (!(k & ~i & 2)) goto good;
                  }
                  else if ((u & 010) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |=  t; i &=  t; }
                     if (!(k & ~i & 2)) goto good;
                  }

                  goto bad;
               case s1x2:
                  if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                  if (!(k & ~i & 2)) goto good;

                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_1fl_only:       /* 1x4/1x6/1x8 - a 1FL, that is, all 4/6/8 facing same; 2x3/2x4 - individual 1FL's */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_1fl_only;
            rr = get_restriction_thing(ss->kind, tt);
            if (rr) goto check_stuff;
            goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_2fl_only:                     /* 1x4 or 2x4 - 2FL; 4x1 - single DPT or single CDPT */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_2fl_only;
            rr = get_restriction_thing(ss->kind, tt);
            if (rr) goto check_stuff;
            goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_3x3couples_only:     /* 1x3/1x6/2x6/1x12 - each group of 3 people are facing the same way */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_3x3couples_only;
            rr = get_restriction_thing(ss->kind, tt);
            if (rr) goto check_stuff;
            goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_4x4couples_only:     /* 1x4/1x8/2x4/2x8/1x16 - each group of 4 people are facing the same way */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_4x4couples_only;
            rr = get_restriction_thing(ss->kind, tt);
            if (rr) goto check_stuff;
            goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_miniwaves:                    /* miniwaves everywhere */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
                  goto bad;
            }

            switch (ss->kind) {
               case s1x2:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s1x4:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s1x8:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[6].id1) & (u = ss->people[7].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[5].id1) & (u = ss->people[4].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s2x4:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[2].id1) & (u = ss->people[3].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[5].id1) & (u = ss->people[4].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[7].id1) & (u = ss->people[6].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  k = 1;
                  i = 2;
                  if ((t = ss->people[0].id1) & (u = ss->people[7].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[1].id1) & (u = ss->people[6].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[2].id1) & (u = ss->people[5].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[4].id1)) { k &= t&u; i &= t^u; }
                  if ((i & 2) && (k & 1)) goto good;
                  goto bad;
               case s2x2:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  k = 1;
                  i = 2;
                  if ((t = ss->people[0].id1) & (u = ss->people[3].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[1].id1) & (u = ss->people[2].id1)) { k &= t&u; i &= t^u; }
                  if ((i & 2) && (k & 1)) goto good;
                  goto bad;
               case sdmd:
                  k = 1;
                  i = 2;
                  if ((t = ss->people[1].id1) & (u = ss->people[3].id1)) { k &= t&u; i &= t^u; }
                  if ((i & 2) && (k & 1)) goto good;
                  goto bad;
               case s_trngl:
                  if ((t = ss->people[1].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s_qtag:
                  if ((t = ss->people[6].id1) & (u = ss->people[7].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_rwave_only:
            if (ss->kind == s1x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x8) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_north))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_west))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_west))
               goto good;
               goto bad;
            }
            else if (ss->kind == sdmd) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_west))
               goto good;
               goto bad;
            }
            else if (ss->kind == s_trngl) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s_qtag) {
               if ((!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_lwave_only:
            if (ss->kind == s1x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x8) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_south))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_east))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_east))
               goto good;
               goto bad;
            }
            else if (ss->kind == sdmd) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_east))
               goto good;
               goto bad;
            }
            if (ss->kind == s_trngl) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s_qtag) {
               if ((!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_ctrwv_end2fl:
            /* Note that this qualifier is kind of strict.  We won't permit the call "with
               confidence" do be done unless everyone can trivially determine which
               part to do. */
            if (ss->kind == s_crosswave) {
               if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                   ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0 &&
                   ((ss->people[2].id1 | ss->people[3].id1) == 0 || ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 2) &&
                   ((ss->people[6].id1 | ss->people[7].id1) == 0 || ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 2))
               goto good;
               goto bad;
            }
            else
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_ctr2fl_endwv:
            /* Note that this qualifier is kind of strict.  We won't permit the call "with
               confidence" do be done unless everyone can trivially determine which
               part to do. */
            if (ss->kind == s_crosswave) {
               if (((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 0 &&
                   ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 0 &&
                   ((ss->people[0].id1 | ss->people[1].id1) == 0 || ((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2) &&
                   ((ss->people[4].id1 | ss->people[5].id1) == 0 || ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2))
               goto good;
               goto bad;
            }
            else
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_true_Z_cw: case sq_true_Z_ccw:
            mask = 0;

            for (i=0, k=1; i<=setup_attrs[ss->kind].setup_limits; i++, k<<=1) {
               if (ss->people[i].id1) mask |= k;
            }

            if (ss->kind == s2x3) {
               /* In this case, we actually check the shear direction of the Z. */
               if (mask == ((((search_qualifier) p->qualifier) == sq_true_Z_cw) ? 066 : 033)) goto good;
               goto bad;
            }

/*  This stuff taken out -- being fussy about exact occupation of the 2x3's is not a good idea.
   It could prevent recognition of unsymmetrical setups.  So we just allow any setup that could be divided into 2x3's.
            else if (ss->kind == s3x4) {
               if (mask == 0xEBA || mask == 0xD75) goto good;
               goto bad;
            }
            else if (ss->kind == s2x6) {
               if (mask == 03333 || mask == 06666) goto good;
               goto bad;
            }
*/
            else
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
         case sq_lateral_cols_empty:
            mask = 0;
            t = 0;

            for (i=0, k=1; i<=setup_attrs[ss->kind].setup_limits; i++, k<<=1) {
               if (ss->people[i].id1) { mask |= k; t |= ss->people[i].id1; }
            }

            if (        ss->kind == s3x4 && (t & 1) == 0 &&
                        (  (mask & 04646) == 0 || (mask & 04532) == 0 || (mask & 03245) == 0 ||
                           (mask & 02525) == 0 || (mask & 03232) == 0 || (mask & 04651) == 0 || (mask & 05146) == 0))
               goto good;
            else if (   ss->kind == s4x4 && (t & 1) == 0 &&
                        ((mask & 0xE8E8) == 0 || (mask & 0xA3A3) == 0 || (mask & 0x5C5C) == 0))
               goto good;
            else if (   ss->kind == s4x4 && (t & 010) == 0 &&
                        ((mask & 0x8E8E) == 0 || (mask & 0x3A3A) == 0 || (mask & 0xC5C5) == 0))
               goto good;
            else if (ss->kind == s4x6 && (t & 010) == 0) goto good;
            else if (ss->kind == s3x8 && (t & 001) == 0) goto good;
            goto bad;
         case sq_1_4_tag:                      /* dmd or qtag - is a 1/4 tag, i.e. points looking in */
            switch (ss->kind) {
               case sdmd:
                  if (   (!(t = ss->people[0].id1 & d_mask) || t == d_east) &&  /* We forgive phantoms up to a point. */
                         (!(u = ss->people[2].id1 & d_mask) || u == d_west) &&
                         (t | u))               /* But require at least one live person to make the setup definitive. */
                     goto good;
                  goto bad;
               case s_qtag:
                  if (   (!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                         (!(u = ss->people[1].id1 & d_mask) || u == d_south) &&
                         (!(v = ss->people[4].id1 & d_mask) || v == d_north) &&
                         (!(w = ss->people[5].id1 & d_mask) || w == d_north) &&
                         (t | u | v | w))
                     goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_3_4_tag:                      /* dmd or qtag - is a 3/4 tag, i.e. points looking out */
            switch (ss->kind) {
               case sdmd:
                  if (   (!(t = ss->people[0].id1 & d_mask) || t == d_west) &&  /* We forgive phantoms up to a point. */
                         (!(u = ss->people[2].id1 & d_mask) || u == d_east) &&
                         (t | u))               /* But require at least one live person to make the setup definitive. */
                     goto good;
                  goto bad;
               case s_qtag:
                  if (   (!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                         (!(u = ss->people[1].id1 & d_mask) || u == d_north) &&
                         (!(v = ss->people[4].id1 & d_mask) || v == d_south) &&
                         (!(w = ss->people[5].id1 & d_mask) || w == d_south) &&
                         (t | u | v | w))
                     goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_dmd_same_pt:                   /* dmd or pdmd - centers would circulate to same point */
            if (((ss->people[1].id1 & 01011) == d_east) &&                        /* faces either east or west */
                (!((ss->people[3].id1 ^ ss->people[1].id1) & d_mask)))   /* and both face same way */
               goto good;
            goto bad;
         case sq_dmd_facing:                    /* dmd or pdmd - diamond is fully occupied and fully facing */
            if ((ss->people[0].id1 & d_mask) == d_north &&
                (ss->people[1].id1 & d_mask) == d_west &&
                (ss->people[2].id1 & d_mask) == d_south &&
                (ss->people[3].id1 & d_mask) == d_east)
               goto good;
            if ((ss->people[0].id1 & d_mask) == d_south &&
                (ss->people[1].id1 & d_mask) == d_east &&
                (ss->people[2].id1 & d_mask) == d_north &&
                (ss->people[3].id1 & d_mask) == d_west)
               goto good;
            goto bad;
         case sq_8_chain:                   /* 4x1/4x2 - setup is (single) 8-chain */
            switch (ss->kind) {
               case s1x4:
                  if (  !((ss->people[0].id1 | ss->people[3].id1) & 2) &&
                        (!ss->people[1].id1 || (ss->people[1].id1 & 2)) &&
                        (!ss->people[2].id1 || (ss->people[2].id1 & 2)))
                     goto good;
                  break;
               case s2x4:
                  if (  !((ss->people[0].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[7].id1) & 2) &&
                        (!ss->people[1].id1 || (ss->people[1].id1 & 2)) &&
                        (!ss->people[3].id1 || (ss->people[3].id1 & 2)) &&
                        (!ss->people[4].id1 || (ss->people[4].id1 & 2)) &&
                        (!ss->people[6].id1 || (ss->people[6].id1 & 2)))
                     goto good;
                  break;
               default:
                  goto bad;
            }
            goto bad;
         case sq_trade_by:                  /* 4x1/4x2 - setup is (single) trade-by */
            switch (ss->kind) {
               case s1x4:
                  if (  !((ss->people[1].id1 | ss->people[2].id1) & 2) &&
                        (!ss->people[0].id1 || (ss->people[0].id1 & 2)) &&
                        (!ss->people[3].id1 || (ss->people[3].id1 & 2)))
                     goto good;
                  break;
               case s2x4:
                  if (  !((ss->people[1].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[6].id1) & 2) &&
                        (!ss->people[0].id1 || (ss->people[0].id1 & 2)) &&
                        (!ss->people[2].id1 || (ss->people[2].id1 & 2)) &&
                        (!ss->people[5].id1 || (ss->people[5].id1 & 2)) &&
                        (!ss->people[7].id1 || (ss->people[7].id1 & 2)))
                     goto good;
                  break;
               default:
                  break;
            }
            goto bad;
         case sq_split_dixie:
            if (ss->cmd.cmd_final_flags & FINAL__SPLIT_DIXIE_APPROVED) goto good;
            goto bad;
         case sq_not_split_dixie:
            if (!(ss->cmd.cmd_final_flags & FINAL__SPLIT_DIXIE_APPROVED)) goto good;
            goto bad;
         case sq_said_tgl:
            if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) goto good;
            goto bad;
         case sq_didnt_say_tgl:
            if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) goto bad;
            goto good;
         case sq_dmd_ctrs_rh:
         case sq_dmd_ctrs_lh:
         case sq_dmd_ctrs_1f:
            {
               search_qualifier kkk;      /* gets set to the qualifier corresponding to what we have. */
               short *p1;
               short d1;
               short d2;
               uint32 z = 0;
               long_boolean b1 = TRUE;
               long_boolean b2 = TRUE;
            
               switch (ss->kind) {
                  case s_spindle:
                     p1 = spindle1;
                     break;
                  case s_short6:
                     p1 = short1;
                     break;
                  case sdmd:
                     p1 = dmd1;
                     break;
                  case s_qtag:
                     p1 = qtag1;
                     break;
                  case s_ptpd:
                     p1 = ptpd1;
                     break;
                  default:
                     goto bad;
               }

               d1 = *(p1++);
               d2 = *(p1++);
            
               while (*p1>=0) {
                  uint32 t1 = ss->people[*(p1++)].id1;
                  uint32 t2 = ss->people[*(p1++)].id1;
                  z |= t1 | t2;
                  if (t1 && (t1 & d_mask)!=d1) b1 = FALSE;
                  if (t2 && (t2 & d_mask)!=d2) b1 = FALSE;
                  if (t1 && (t1 & d_mask)!=d2) b2 = FALSE;
                  if (t2 && (t2 & d_mask)!=d1) b2 = FALSE;
               }
            
               if (z) {
                  if (b1) kkk = sq_dmd_ctrs_rh;
                  else if (b2) kkk = sq_dmd_ctrs_lh;
                  else kkk = sq_dmd_ctrs_1f;
               }
               else
                  goto bad;

               if ((search_qualifier) p->qualifier == kkk) goto good;
            }
            goto bad;
         case sq_ctr_pts_rh:
         case sq_ctr_pts_lh:
            {
               search_qualifier kkk;      /* gets set to the qualifier corresponding to what we have. */
               uint32 t1;
               uint32 t2;
               long_boolean b1 = TRUE;
               long_boolean b2 = TRUE;
            
               switch (ss->kind) {
                  case s_qtag:
                  case s_hrglass:
                     t1 = ss->people[6].id1;
                     t2 = ss->people[2].id1;
                     break;
                  case s_2x1dmd:
                     t1 = ss->people[0].id1;
                     t2 = ss->people[3].id1;
                     break;
                  default:
                     goto bad;
               }

               if (t1 && (t1 & d_mask)!=d_north) b1 = FALSE;
               if (t2 && (t2 & d_mask)!=d_south) b1 = FALSE;
               if (t1 && (t1 & d_mask)!=d_south) b2 = FALSE;
               if (t2 && (t2 & d_mask)!=d_north) b2 = FALSE;

               if (b1 == b2) goto bad;
               kkk = b1 ? sq_ctr_pts_rh : sq_ctr_pts_lh;
               if ((search_qualifier) p->qualifier == kkk) goto good;
            }
            goto bad;
         case sq_all_sel:
            k = 1;     /* K was initialized to zero. */
            /* FALL THROUGH!!!!!! */
         case sq_none_sel:
            /* Now k=0 for "none_sel", and 1 for "all_sel". */
            if (setup_attrs[ss->kind].setup_limits >= 2) goto good;
            j = 1;
            for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) {
               if (selectp(ss, idx)) {
                  if (k==0) j = 0;
               }
               else {
                  if (k!=0) j = 0;
               }
            }
            if (j) goto good;
            goto bad;
         default:
            break;
      }

      goto bad;

      check_stuff:

      switch (rr->check) {
         case chk_wave:
            qa0 = 0; qa1 = 0;

            for (idx=0; idx<rr->size; idx++) {
               if ((t = ss->people[rr->map1[idx]].id1) != 0) { qa0 |=  t; qa1 |= ~t; }
               if ((t = ss->people[rr->map2[idx]].id1) != 0) { qa0 |= ~t; qa1 |=  t; }
            }

            if ((qa0 & qa1 & 2) != 0) goto bad;

            goto good;
         case chk_1_group:
            limit = 1;
            goto check_groups;
         case chk_2_groups:
            limit = 2;
            goto check_groups;
         case chk_4_groups:
            limit = 4;
            goto check_groups;
         case chk_8_groups:
            limit = 8;
            goto check_groups;
         default:
            goto bad;    /* Shouldn't happen. */
      }

      check_groups:

      for (idx=0; idx<limit; idx++) {
         qa0 = 0; qa1 = 0;
   
         for (i=0,j=idx; i<rr->size; i++,j+=limit) {
            if ((t = ss->people[rr->map1[j]].id1) != 0) { qa0 |= t; qa1 |= ~t; }
         }
   
         if (qa0&qa1&2)
            goto bad;
      }
      goto good;

      bad: ;
   }

   good:
   return p;
}


extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index)
{
   unsigned short *calldef_array;
   predptr_pair *predlistptr;
   unsigned int z;

   if (tdef->callarray_flags & CAF__PREDS) {
      predlistptr = tdef->stuff.prd.predlist;
      while (predlistptr != (predptr_pair *) 0) {
         if ((*(predlistptr->pred))(scopy, real_index, real_direction, northified_index)) {
            calldef_array = predlistptr->arr;
            goto got_it;
         }
         predlistptr = predlistptr->next;
      }
      fail(tdef->stuff.prd.errmsg);
   }
   else
      calldef_array = tdef->stuff.def;

got_it:

   z = calldef_array[northified_index];
   if (!z) failp(scopy->people[real_index].id1, "can't execute their part of this call.");

   return z;
}


extern void clear_people(setup *z)
{
   (void) memset(z->people, 0, sizeof(personrec)*MAX_PEOPLE);
}


extern uint32 rotperson(uint32 n, int amount)
{
   if (n == 0) return 0; else return (n + amount) & ~064;
}


extern uint32 rotcw(uint32 n)
{
   if (n == 0) return 0; else return (n + 011) & ~064;
}


extern uint32 rotccw(uint32 n)
{
   if (n == 0) return 0; else return (n + 033) & ~064;
}


extern void clear_person(setup *resultpeople, int resultplace)
{
   resultpeople->people[resultplace].id1 = 0;
   resultpeople->people[resultplace].id2 = 0;
}


extern uint32 copy_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace)
{
   resultpeople->people[resultplace] = sourcepeople->people[sourceplace];
   return resultpeople->people[resultplace].id1;
}


extern uint32 copy_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount)
{
   unsigned int newperson = sourcepeople->people[sourceplace].id1;

   if (newperson) newperson = (newperson + rotamount) & ~064;
   resultpeople->people[resultplace].id2 = sourcepeople->people[sourceplace].id2;
   return resultpeople->people[resultplace].id1 = newperson;
}


extern void swap_people(setup *ss, int oneplace, int otherplace)
{
   personrec temp = ss->people[otherplace];
   ss->people[otherplace] = ss->people[oneplace];
   ss->people[oneplace] = temp;
}


extern void install_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace)
{
   unsigned int newperson = sourcepeople->people[sourceplace].id1;

   if (resultpeople->people[resultplace].id1 == 0)
      resultpeople->people[resultplace] = sourcepeople->people[sourceplace];
   else if (newperson) {
      collision_person1 = resultpeople->people[resultplace].id1;
      collision_person2 = newperson;
      error_message1[0] = '\0';
      error_message2[0] = '\0';
      longjmp(longjmp_ptr->the_buf, 3);
   }
}


extern void install_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount)
{
   unsigned int newperson = sourcepeople->people[sourceplace].id1;

   if (newperson) {
      if (resultplace < 0) fail("This would go into an excessively large matrix.");

      if (resultpeople->people[resultplace].id1 == 0) {
         resultpeople->people[resultplace].id1 = (newperson + rotamount) & ~064;
         resultpeople->people[resultplace].id2 = sourcepeople->people[sourceplace].id2;
      }
      else {
         collision_person1 = resultpeople->people[resultplace].id1;
         collision_person2 = newperson;
         error_message1[0] = '\0';
         error_message2[0] = '\0';
         longjmp(longjmp_ptr->the_buf, 3);
      }
   }
}


#define MXN_BITS (INHERITFLAG_1X2 | INHERITFLAG_2X1 | INHERITFLAG_2X2 | INHERITFLAG_1X3 | INHERITFLAG_3X1 | INHERITFLAG_3X3 | INHERITFLAG_4X4)

/* Take a concept pointer and scan for all "final" concepts,
   returning an updated concept pointer and a mask of all such concepts found.
   "Final" concepts are those that modify the execution of a call but
   do not cause it to be executed in a virtual or distorted setup.
   This has a side-effect that is occasionally used:  When it passes over
   any "magic" or "interlocked" concept, it drops a pointer to where the
   last such occurred into the external variable "last_magic_diamond". */

extern parse_block *process_final_concepts(
   parse_block *cptr,
   long_boolean check_errors,
   final_set *final_concepts)
{
   parse_block *tptr = cptr;

   *final_concepts = 0;

   while (tptr) {
      final_set bit_to_set;
      final_set bit_to_forbid = 0;

      switch (tptr->concept->kind) {
         case concept_comment:
            goto get_next;               /* Need to skip these. */
         case concept_triangle: bit_to_set = FINAL__TRIANGLE; break;
         case concept_magic:
            last_magic_diamond = tptr;
            bit_to_set = INHERITFLAG_MAGIC;
            bit_to_forbid = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
            break;
         case concept_interlocked:
            last_magic_diamond = tptr;
            bit_to_set = INHERITFLAG_INTLK;
            bit_to_forbid = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
            break;
         case concept_grand:
            bit_to_set = INHERITFLAG_GRAND;
            bit_to_forbid = INHERITFLAG_SINGLE;
            break;
         case concept_cross: bit_to_set = INHERITFLAG_CROSS; break;
         case concept_yoyo: bit_to_set = INHERITFLAG_YOYO; break;
         case concept_single:
            bit_to_set = INHERITFLAG_SINGLE;
            bit_to_forbid = INHERITFLAG_SINGLEFILE;
            break;
         case concept_singlefile:
            bit_to_set = INHERITFLAG_SINGLEFILE;
            bit_to_forbid = INHERITFLAG_SINGLE;
            break;
         case concept_1x2:
            bit_to_set = INHERITFLAG_1X2;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_2x1:
            bit_to_set = INHERITFLAG_2X1;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_2x2:
            bit_to_set = INHERITFLAG_2X2;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_1x3:
            bit_to_set = INHERITFLAG_1X3;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_3x1:
            bit_to_set = INHERITFLAG_3X1;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_3x3:
            bit_to_set = INHERITFLAG_3X3;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_4x4:
            bit_to_set = INHERITFLAG_4X4;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_split:
            bit_to_set = FINAL__SPLIT; break;
         case concept_reverse:
            bit_to_set = INHERITFLAG_REVERSE; break;
         case concept_left:
            bit_to_set = INHERITFLAG_LEFT; break;
         case concept_12_matrix:
            if (check_errors && *final_concepts)
               fail("Matrix modifier must appear first.");
            bit_to_set = INHERITFLAG_12_MATRIX;
            break;
         case concept_16_matrix:
            if (check_errors && *final_concepts)
               fail("Matrix modifier must appear first.");
            bit_to_set = INHERITFLAG_16_MATRIX;
            break;
         case concept_diamond:
            bit_to_set = INHERITFLAG_DIAMOND;
            bit_to_forbid = INHERITFLAG_SINGLE;
            break;
         case concept_funny:
            bit_to_set = INHERITFLAG_FUNNY; break;
         default:
            goto exit5;
      }

      if (check_errors) {
         if (tptr->concept->level > calling_level) warn(warn__bad_concept_level);
   
         if (*final_concepts & bit_to_set)
            fail("Redundant call modifier.");
   
         if (*final_concepts & bit_to_forbid)
            fail("Illegal combination or order of call modifiers.");
      }

      *final_concepts |= bit_to_set;

      get_next:

      tptr = tptr->next;
   }

   exit5:

   return tptr;
}


extern parse_block *skip_one_concept(parse_block *incoming)
{
   final_set new_final_concepts;
   parse_block *retval;

   if (incoming->concept->kind == concept_comment)
      fail("Please don't put a comment after a meta-concept.  Sorry.");

   retval = process_final_concepts(incoming, FALSE, &new_final_concepts);

   /* Find out whether the next concept (the one that will be "random" or whatever)
      is a modifier or a "real" concept. */

   if (new_final_concepts) {
      retval = incoming; /* Lots of comment-aversion code being punted
                                          here, but it's just too hairy, and we're
                                          going to change all that stuff anyway. */
   }
   else {
      concept_kind k = retval->concept->kind;

      if (k <= marker_end_of_list || concept_table[k].concept_action == 0)
         fail("Sorry, can't do this with this concept.");

      if ((concept_table[k].concept_prop & CONCPROP__SECOND_CALL) && retval->concept->kind != concept_special_sequential)
         fail("Can't use a concept that takes a second call.");
   }

   return retval;
}


/* Prepare several setups to be assembled into one, by making them all have
   the same kind and rotation. */
extern long_boolean fix_n_results(int arity, setup z[])
{
   int i;
   long_boolean lineflag = FALSE;
   long_boolean miniflag = FALSE;
   setup_kind kk = nothing;
   int rr = -1;

   /* There are 3 things that make this task nontrivial.  First, some setups could
      be "nothing", in which case we turn them into the same type of setup as
      their neighbors, with no people.  Second, some types of "grand working"
      can leave setups confused about whether they are lines or diamonds,
      because only the ends/points are occupied.  We turn those setups into
      whatever matches their more-fully-occupied neighbors.  (If completely in
      doubt, we opt for 1x4's.)  Third, some 1x4's may have been shrunk to 1x2's.
      This can happen in some cases of "triple box patch the so-and-so", which lead
      to triple lines in which the outer lines collapsed from 1x4's to 1x2's while
      the center line is fully occupied.  In this case we repopulate the outer lines
      to 1x4's. */

   for (i=0; i<arity; i++) {
      if (z[i].kind == s_normal_concentric) {
         /* We definitely have a problem.  A common way for this to happen is if a concentric call
            was done and there were no ends, so we don't know what the setup really is.  Example:
            from normal columns, do a split phantom lines hocus-pocus.  Do we leave space for the
            phantoms?  Humans would probably say yes because they know where the phantoms went, but
            this program has no idea in general.  If a call is concentrically executed and the
            outsides are all phantoms, we don't know what the setup is.  Concentric_move signifies
            this by creating a "concentric" setup with "nothing" for the outer setup.  So we raise
            an error that is somewhat descriptive. */
         if (z[i].outer.skind == nothing)
            fail("Can't do this: don't know where the phantoms went.");
         else if (z[i].inner.skind == nothing && z[i].outer.skind == s1x2) {
            /* We can fix this up.  Just turn it into a 1x4 with the ends missing.
            (If a diamond is actually required, that will get fixed up below.)
            The test case for this is split phantom lines cross to a diamond from 2FL. */

            z[i].kind = s1x4;
            z[i].rotation = z[i].outer.srotation;
            copy_person(&z[i], 0, &z[i], 12);
            copy_person(&z[i], 2, &z[i], 13);
            clear_person(&z[i], 1);
            clear_person(&z[i], 3);
         }
         else
            fail("Don't recognize ending setup for this call.");
      }

      if (z[i].kind != nothing) {
         canonicalize_rotation(&z[i]);

         if (z[i].kind == s1x2)
            miniflag = TRUE;
         else if ((z[i].kind == s1x4 || z[i].kind == sdmd) && (z[i].people[1].id1 | z[i].people[3].id1) == 0)
            lineflag = TRUE;
         else {
            if (kk == nothing) kk = z[i].kind;
            if (kk != z[i].kind) goto lose;
         }

         if (rr < 0) rr = z[i].rotation;
         /* If the setups are "trngl4", we allow mismatched rotation --
            the client will take care of it. */
         if (rr != z[i].rotation && z[i].kind != s_trngl4) goto lose;
      }
   }

   if (kk == nothing) {
      if (lineflag) kk = s1x4;
      else if (miniflag) kk = s1x2;
      else return TRUE;
   }
   
   /* If something wasn't sure whether it was points of a diamond or
      ends of a 1x4, that's OK if something else had a clue. */
   if (lineflag && kk != s1x4 && kk != sdmd) goto lose;

   /* If something was a 1x2, that's OK if something else was a 1x4. */
   if (miniflag && kk != s1x4 && kk != s1x2) goto lose;

   for (i=0; i<arity; i++) {
      if (z[i].kind == nothing)
         clear_people(&z[i]);
      else if (z[i].kind == s1x2 && kk == s1x4) {
         /* We have to expand a 1x2 to the center spots of a 1x4. */
         (void) copy_person(&z[i], 3, &z[i], 1);
         clear_person(&z[i], 2);
         (void) copy_person(&z[i], 1, &z[i], 0);
         clear_person(&z[i], 0);
      }

      z[i].kind = kk;
      if (kk != s_trngl4) z[i].rotation = rr;
   }

   return FALSE;

   lose:

   fail("This is a ridiculously inconsistent shape or orientation changer!!");
   /* NOTREACHED */
}
