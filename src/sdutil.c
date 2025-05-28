/* SD -- square dance caller's helper.

    Copyright (C) 1990-1997  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 31. */

/* This defines the following functions:
   initialize_restr_tables
   get_restriction_thing
   clear_screen
   newline
   writestuff
   unparse_call_name
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
   get_rh_test
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
   scatter
   gather
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
#define SCREWED_UP_REVERTS



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
   {"lead ends",    "lead end",    "LEAD ENDS",    "LEAD END",    selector_uninitialized},
   {"lead centers", "lead center", "LEAD CENTERS", "LEAD CENTER", selector_uninitialized},
   {"trailing ends","trailing end","TRAILING ENDS","TRAILING END",selector_uninitialized},
   {"trailing centers","trailing center","TRAILING CENTERS","TRAILING CENTER",selector_uninitialized},
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
   {"everyone",     "everyone",    "EVERYONE",     "EVERYONE",    selector_uninitialized},
   {"no one",       "no one",      "NO ONE",       "NO ONE",      selector_uninitialized},
   /* Start of unsymmetrical selectors. */
   {"near line",    "near line",   "NEAR LINE",    "NEAR LINE",   selector_uninitialized},
   {"far line",     "far line",    "FAR LINE",     "FAR LINE",    selector_uninitialized},
   {"near column",  "near column", "NEAR COLUMN",  "NEAR COLUMN", selector_uninitialized},
   {"far column",   "far column",  "FAR COLUMN",   "FAR COLUMN",  selector_uninitialized},
   {"near box",     "near box",    "NEAR BOX",     "NEAR BOX",    selector_uninitialized},
   {"far box",      "far box",     "FAR BOX",      "FAR BOX",     selector_uninitialized},

   {"those facing the caller", "those facing the caller",
   "THOSE FACING THE CALLER", "THOSE FACING THE CALLER",         selector_uninitialized},

   {"those facing away from the caller", "those facing away from the caller",
   "THOSE FACING AWAY FROM THE CALLER", "THOSE FACING AWAY FROM THE CALLER",
                                                                  selector_uninitialized},

   {"#1 boy",       "#1 boy",      "#1 BOY",       "#1 BOY",      selector_uninitialized},
   {"#1 girl",      "#1 girl",     "#1 GIRL",      "#1 GIRL",     selector_uninitialized},
   {"#1 couple",    "#1 couple",   "#1 COUPLE",    "#1 COUPLE",   selector_uninitialized},
   {"#2 boy",       "#2 boy",      "#2 BOY",       "#2 BOY",      selector_uninitialized},
   {"#2 girl",      "#2 girl",     "#2 GIRL",      "#2 GIRL",     selector_uninitialized},
   {"#2 couple",    "#2 couple",   "#2 COUPLE",    "#2 COUPLE",   selector_uninitialized},
   {"#3 boy",       "#3 boy",      "#3 BOY",       "#3 BOY",      selector_uninitialized},
   {"#3 girl",      "#3 girl",     "#3 GIRL",      "#3 GIRL",     selector_uninitialized},
   {"#3 couple",    "#3 couple",   "#3 COUPLE",    "#3 COUPLE",   selector_uninitialized},
   {"#4 boy",       "#4 boy",      "#4 BOY",       "#4 BOY",      selector_uninitialized},
   {"#4 girl",      "#4 girl",     "#4 GIRL",      "#4 GIRL",     selector_uninitialized},
   {"#4 couple",    "#4 couple",   "#4 COUPLE",    "#4 COUPLE",   selector_uninitialized},
   {"couples 1 and 2", "couple 1 and 2", "COUPLES 1 AND 2", "COUPLE 1 AND 2", selector_uninitialized},
   {"couples 2 and 3", "couple 2 and 3", "COUPLES 2 AND 3", "COUPLE 2 AND 3", selector_uninitialized},
   {"couples 3 and 4", "couple 3 and 4", "COUPLES 3 AND 4", "COUPLE 3 AND 4", selector_uninitialized},
   {"couples 1 and 4", "couple 1 and 4", "COUPLES 1 AND 4", "COUPLE 1 AND 4", selector_uninitialized},
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
   /*  warn__rear_back           */   "*Rear back from the handhold.",
   /*  warn__awful_rear_back     */   "*Rear back from the handhold -- this is very unusual.",
   /*  warn__excess_split        */   "*Split concept seems to be superfluous here.",
   /*  warn__lineconc_perp       */   "+Ends should opt for setup perpendicular to their original line.",
   /*  warn__dmdconc_perp        */   "+Ends should opt for setup perpendicular to their original diamond points.",
   /*  warn__lineconc_par        */   "+Ends should opt for setup parallel to their original line -- concentric rule does not apply.",
   /*  warn__dmdconc_par         */   "+Ends should opt for setup parallel to their original diamond points -- concentric rule does not apply.",
   /*  warn__xclineconc_perpc    */   "+New ends should opt for setup perpendicular to their original (center) line.",
   /*  warn__xcdmdconc_perpc     */   "+New ends should opt for setup perpendicular to their original (center) diamond points.",
   /*  warn__xclineconc_perpe    */   "+New ends should opt for setup perpendicular to their original (center) line.  Beware:  This may be controversial.",
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
   /*  warn__check_galaxy        */   " Check a galaxy.",
   /*  warn__some_rear_back      */   " Some people rear back.",
   /*  warn__not_tbone_person    */   " Work with the person to whom you are not T-boned.",
   /*  warn__check_c1_phan       */   " Check a 'C1 phantom' setup.",
   /*  warn__check_dmd_qtag      */   " Fudge to a diamond/quarter-tag setup.",
   /*  warn__check_2x4           */   " Check a 2x4 setup.",
   /*  warn__check_4x4           */   "*Check a 4x4 setup.",
   /*  warn__check_hokey_4x4     */   "*Check a center box and outer lines/columns.",
   /*  warn__check_4x4_start     */   "*Check a 4x4 setup at the start of this call.",
   /*  warn__check_pgram         */   " Opt for a parallelogram.",
   /*  warn__ctrs_stay_in_ctr    */   " Centers stay in the center.",
   /*  warn__check_c1_stars      */   " Check 'stars'.",
   /*  warn__check_gen_c1_stars  */   " Check a generalized 'star' setup.",
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
   /*  warn__bad_call_level      */   "*This call is not really legal at this level.",
   /*  warn__did_not_interact    */   "*The setups did not interact with each other.",
   /*  warn__opt_for_normal_cast */   "*If in doubt, assume a normal cast.",
   /*  warn__opt_for_normal_hinge */  "*If in doubt, assume a normal hinge.",
   /*  warn__split_1x6           */   "*Do the call in each 1x3 setup.",
   /*  warn__colocated_once_rem  */   " The once-removed setups have the same center.",
   /*  warn_bad_collision        */   "*This collision may be controversial.",
   /*  warn__dyp_resolve_ok      */   " Do your part.",
   /*  warn__unusual             */   "*This is an unusual setup for this call.",
   /*  warn_controversial        */   "*This may be controversial.",
   /*  warn_serious_violation    */   "*This appears to be a serious violation of the definition.",
   /*  warn__tasteless_com_spot  */   "*Common-spot people have left hands -- this may be controversial.",
   /*  warn__tasteless_slide_thru*/   "*Slide thru from left-handed miniwave is very controversial."};



static restriction_thing wave_2x4      = {8, {0, 1, 2, 3, 5, 4, 7, 6},             {0},                   {0}, {0}, TRUE, chk_wave};            /* check for two parallel consistent waves */
static restriction_thing jleft_qtag    = {8, {2, 3, 0, 4, 7, 6, 1, 5},             {0},                   {0}, {0}, TRUE, chk_wave};
static restriction_thing jright_qtag   = {8, {3, 2, 0, 4, 6, 7, 1, 5},             {0},                   {0}, {0}, TRUE, chk_wave};
static restriction_thing jright_dmd    = {4, {0, 2, 1, 3},                         {0},                   {0}, {0}, TRUE, chk_wave};
static restriction_thing jright_ptpd   = {8, {0, 2, 1, 3, 6, 4, 7, 5},             {0},                   {0}, {0}, TRUE, chk_wave};
static restriction_thing jleft_4dmd    = {16, {4, 5, 0, 8, 6, 7, 1, 9, 15, 14, 2, 10, 13, 12, 3, 11}, {0},{0}, {0}, TRUE, chk_wave};
static restriction_thing jright_4dmd   = {16, {5, 4, 0, 8, 7, 6, 1, 9, 14, 15, 2, 10, 12, 13, 3, 11}, {0},{0}, {0}, TRUE, chk_wave};

static restriction_thing ijleft_qtag   = {8, {2, 6, 0, 4, 3, 7, 1, 5},             {0},                   {0}, {0}, TRUE, chk_wave};
static restriction_thing ijright_qtag  = {8, {6, 2, 0, 4, 7, 3, 1, 5},             {0},                   {0}, {0}, TRUE, chk_wave};

static restriction_thing two_faced_2x4 = {8, {0, 3, 1, 2, 6, 5, 7, 4},             {0},                   {0}, {0}, TRUE, chk_wave};            /* check for two parallel consistent two-faced lines */
static restriction_thing wave_3x4      = {12, {0, 1, 2, 3, 5, 4, 7, 6, 9, 8, 10, 11},    {0},             {0}, {0}, TRUE, chk_wave};            /* check for three parallel consistent waves */
static restriction_thing two_faced_3x4 = {12, {0, 2, 1, 3, 8, 4, 9, 5, 10, 6, 11, 7},    {0},             {0}, {0}, TRUE, chk_wave};            /* check for three parallel consistent two-faced lines */
static restriction_thing wave_1x2      = {2, {0, 1},                      {0},                            {0}, {0}, TRUE, chk_wave};            /* check for a miniwave -- note this is NOT OK for assume */
static restriction_thing wave_1x3      = {3, {0, 1, 2},                      {0},                         {0}, {0}, TRUE, chk_wave};            /* check for a wave */
static restriction_thing wave_1x4      = {4, {0, 1, 3, 2},                   {0},                         {0}, {0}, TRUE, chk_wave};            /* check for a wave */
static restriction_thing wave_1x6      = {6, {0, 1, 2, 5, 4, 3},                {0},                      {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 6 */
static restriction_thing wave_1x8      = {8, {0, 1, 3, 2, 6, 7, 5, 4},             {0},                   {0}, {0}, TRUE, chk_wave};            /* check for grand wave */
static restriction_thing wave_1x10     = {10, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},         {0},                {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 10 */
static restriction_thing wave_1x12     = {12, {0, 1, 2, 3, 4, 5, 7, 6, 9, 8, 11, 10},     {0},            {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 12 */
static restriction_thing wave_2x3      = {6, {0, 1, 2, 3, 4, 5},                {0},                      {0}, {0}, FALSE, chk_wave};           /* check for two "waves" -- people are antitandem */
static restriction_thing wave_2x6      = {12, {0, 1, 2, 3, 4, 5, 7, 6, 9, 8, 11, 10},                {0}, {0}, {0}, TRUE, chk_wave};            /* check for parallel consistent 2x6 waves */
static restriction_thing wave_1x14     = {14, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},        {0}, {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 14 */
static restriction_thing wave_1x16     = {16, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14},{0}, {0}, {0}, TRUE, chk_wave};            /* check for grand wave of 16 */
static restriction_thing wave_2x8      = {16, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8, 11, 10, 13, 12, 15, 14},{0}, {0}, {0}, TRUE, chk_wave};            /* check for parallel 2x8 waves */

static restriction_thing wave_dmd      = {2, {1, 3},                         {0},                         {0}, {0}, TRUE, chk_wave};            /* check for miniwave in center of diamond */
static restriction_thing wave_ptpd     = {4, {1, 3, 7, 5},                   {0},                         {0}, {0}, TRUE, chk_wave};            /* check for miniwaves in center of each diamond */
static restriction_thing wave_tgl      = {2, {1, 2},                         {0},                         {0}, {0}, TRUE, chk_wave};            /* check for miniwave as base of triangle */

static restriction_thing wave_thar     = {8, {0, 1, 2, 3, 5, 4, 7, 6},           /* NOTE THE 4 --> */{4}, {0}, {0}, TRUE,  chk_wave};           /* check for consistent wavy thar */
static restriction_thing thar_1fl      = {8, {0, 3, 1, 2, 6, 5, 7, 4},                               {0}, {0}, {0}, FALSE, chk_wave};           /* check for consistent 1-faced thar */

static restriction_thing cwave_2x4     = {8, {0, 4, 1, 5, 2, 6, 3, 7},                               {0}, {0}, {0}, TRUE, chk_wave};            /* check for real columns */
static restriction_thing cwave_2x3     = {6, {0, 3, 1, 4, 2, 5},                                     {0}, {0}, {0}, TRUE, chk_wave};            /* check for real columns of 6 */
static restriction_thing cwave_2x6     = {12, {0, 6, 1, 7, 2, 8, 3, 9, 4, 10, 5, 11},                {0}, {0}, {0}, TRUE, chk_wave};            /* check for real 12-matrix columns */
static restriction_thing cwave_2x8     = {16, {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15},{0}, {0}, {0}, TRUE, chk_wave};            /* check for real 16-matrix columns */
static restriction_thing cmagic_2x3    = {6, {0, 1, 2, 3, 4, 5},                {0},                      {0}, {0}, TRUE, chk_wave};            /* check for magic columns of 3 */
static restriction_thing cmagic_2x4    = {8, {0, 1, 3, 2, 5, 4, 6, 7},             {0},                   {0}, {0}, TRUE, chk_wave};            /* check for magic columns */

static restriction_thing lio_2x4       = {8, {4, 0, 5, 1, 6, 2, 7, 3},             {0},                   {0}, {0}, TRUE, chk_wave};            /* check for lines in or lines out */
static restriction_thing invert_1x4    = {4, {0, 1, 2, 3},                   {0},                         {0}, {0}, TRUE, chk_wave};            /* check for single inverted line */

static restriction_thing peelable_3x2  = {3, {0, 1, 2},                   {3, 4, 5},                      {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x3 column */
static restriction_thing peelable_4x2  = {4, {0, 1, 2, 3},                {4, 5, 6, 7},                   {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x4 column */
static restriction_thing peelable_6x2  = {6, {0, 1, 2, 3, 4, 5},          {6, 7, 8, 9, 10, 11},           {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x6 column */
static restriction_thing peelable_8x2  = {8, {0, 1, 2, 3, 4, 5, 6, 7},    {8, 9, 10, 11, 12, 13, 14, 15}, {0}, {0}, FALSE, chk_peelable};       /* check for a "peelable" 2x8 column */

static restriction_thing all_same_2    = {2, {0, 1},                                                 {1}, {0}, {0}, TRUE,  chk_groups};         /* check for a couple */
static restriction_thing all_same_4    = {4, {0, 1, 2, 3},                                           {1}, {0}, {0}, TRUE,  chk_groups};         /* check for a 1-faced line */
static restriction_thing all_same_6    = {6, {0, 1, 2, 3, 4, 5},                                     {1}, {0}, {0}, TRUE,  chk_groups};         /* check for a 1-faced line */
static restriction_thing all_same_8    = {8, {0, 1, 2, 3, 4, 5, 6, 7},                               {1}, {0}, {0}, TRUE,  chk_groups};         /* check for all 8 people same way in 2x4 *OR* 1x8. */
static restriction_thing all_diff_2    = {2, {0, 1},                                                 {1}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for a miniwave */


static restriction_thing two_faced_1x6 = {6, {0, 3, 1, 4, 2, 5},                                     {0}, {0}, {0}, FALSE, chk_wave};           /* check for 3x3 two-faced line -- 3 up and 3 down */
static restriction_thing two_faced_1x8 = {8, {0, 3, 1, 2, 6, 5, 7, 4},                               {0}, {0}, {0}, FALSE, chk_wave};           /* check for grand two-faced line */
static restriction_thing one_faced_2x3 = {3, {0, 3, 1, 4, 2, 5},                                     {2}, {0}, {0}, TRUE,  chk_groups};         /* check for parallel one-faced lines of 3 */
static restriction_thing one_faced_2x4 = {4, {0, 4, 1, 5, 2, 6, 3, 7},                               {2}, {0}, {0}, TRUE,  chk_groups};         /* check for parallel one-faced lines */
static restriction_thing cpls_2x4      = {2, {0, 2, 4, 6, 1, 3, 5, 7},                               {4}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_4x2      = {2, {0, 1, 2, 3, 7, 6, 5, 4},                               {4}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_1x4      = {2, {0, 2, 1, 3},                                           {2}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_1x8      = {2, {0, 2, 4, 6, 1, 3, 5, 7},                               {4}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_2x8      = {2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_8x2      = {2, {0, 1, 2, 3, 4, 5, 6, 7, 15, 14, 13, 12, 11, 10, 9, 8}, {8}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_1x16     = {2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, TRUE,  chk_groups};         /* check for everyone as a couple */
static restriction_thing cpls_2x6      = {3, {0, 3, 6, 9, 1, 4, 7, 10, 2, 5, 8, 11},                 {4}, {0}, {0}, TRUE,  chk_groups};         /* check for each three people facing same way */
static restriction_thing cpls_6x2      = {3, {0, 1, 2, 3, 4, 5, 11, 10, 9, 8, 7, 6},                 {4}, {0}, {0}, TRUE,  chk_groups};         /* check for each three people facing same way */
static restriction_thing cplsof4_2x8   = {4, {0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15}, {4}, {0}, {0}, TRUE,  chk_groups};         /* check for each four people facing same way */
static restriction_thing cpls_1x3      = {3, {0, 1, 2},                                              {1}, {0}, {0}, TRUE,  chk_groups};         /* check for three people facing same way */
static restriction_thing cpls_3x3_1x6  = {3, {0, 3, 1, 4, 2, 5},                                     {2}, {0}, {0}, TRUE,  chk_groups};         /* check for each three people facing same way */
static restriction_thing cpls_4x4_1x8  = {4, {0, 4, 1, 5, 2, 6, 3, 7},                               {2}, {0}, {0}, TRUE,  chk_groups};         /* check for each four people facing same way */
static restriction_thing two_faced_4x4_1x8 = {8, {0, 4, 1, 5, 2, 6, 3, 7},                           {0}, {0}, {0}, FALSE, chk_wave};           /* check for 4x4 two-faced line -- 4 up and 4 down */
static restriction_thing two_faced_1x4 = {4, {0, 2, 1, 3},                                           {0}, {0}, {0}, TRUE,  chk_wave};           /* check for 2-faced lines */
static restriction_thing two_faced_2x6 = {12, {0, 3, 1, 4, 2, 5, 9, 6, 10, 7, 11, 8},                {0}, {0}, {0}, FALSE, chk_wave};           /* check for parallel consistent 3x3 two-faced lines */
static restriction_thing two_faced_4x4_2x8 = {16, {0, 4, 1, 5, 2, 6, 3, 7, 12, 8, 13, 9, 14, 10, 15, 11},{0},{0},{0},FALSE,chk_wave};           /* check for parallel consistent 4x4 two-faced lines */
static restriction_thing mnwv_2x4      = {2, {0, 2, 4, 6, 1, 3, 5, 7},                               {4}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for everyone in a miniwave */
static restriction_thing mnwv_4x2      = {2, {0, 1, 2, 3, 7, 6, 5, 4},                               {4}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for everyone in a miniwave */
static restriction_thing mnwv_2x8      = {2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, FALSE, chk_anti_groups};    /* check for everyone in a miniwave */
static restriction_thing mnwv_1x16     = {2, {0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15}, {8}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for everyone in a miniwave */
static restriction_thing mnwv_1x4      = {2, {0, 2, 1, 3},                                           {2}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for everyone in a miniwave */
static restriction_thing mnwv_1x6      = {2, {0, 2, 4, 1, 5, 3},                                     {3}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for everyone in a miniwave */
static restriction_thing mnwv_1x8      = {2, {0, 2, 4, 6, 1, 3, 5, 7},                               {4}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for everyone in a miniwave */

static restriction_thing box_wave      = {0, {2, 0, 0, 2},                {0, 0, 2, 2},                   {0}, {0}, TRUE,  chk_box};            /* check for a "real" (walk-and-dodge type) box */
static restriction_thing box_1face     = {0, {2, 2, 2, 2},                {0, 0, 0, 0},                   {0}, {0}, TRUE,  chk_box};            /* check for a "one-faced" (reverse-the-pass type) box */
static restriction_thing box_in_or_out = {0, {0, 0, 2, 2},                {0, 2, 2, 0},                   {0}, {0}, TRUE,  chk_box};            /* check for facing couples or back-to-back couples */
static restriction_thing box_magic     = {0, {2, 0, 2, 0},                {0, 2, 0, 2},                   {0}, {0}, TRUE,  chk_box};            /* check for a "magic" (split-trade-circulate type) box */
static restriction_thing s4x4_wave     = {0,   {0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0, 2, 0, 2, 0},
                                               {2, 0, 2, 0, 0, 0, 0, 2, 0, 2, 0, 2, 2, 2, 2, 0},          {0}, {0}, TRUE,  chk_box};            /* check for 4 waves of consistent handedness and consistent headliner-ness. */

static restriction_thing s4x4_2fl     = {0,    {0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2, 2, 2, 0, 2},  
                                               {2, 2, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 2, 2, 2, 2},          {0}, {0}, TRUE,  chk_box};            /* check for 4 waves of consistent handedness and consistent headliner-ness. */

static restriction_thing cwave_qtg     = {4, {2, 3, 7, 6},                                           {0}, {0}, {0}, FALSE, chk_wave};           /* check for wave across the center */
static restriction_thing wave_qtag     = {4, {6, 7, 3, 2},                                           {0}, {0}, {0}, TRUE,  chk_wave};           /* check for wave across the center */
static restriction_thing miniwave_qtg  = {2, {6, 2, 7, 3},                                           {2}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for center people in miniwaves */
static restriction_thing miniwave_ptpd = {2, {1, 7, 3, 5},                                           {2}, {0}, {0}, TRUE,  chk_anti_groups};    /* check for miniwaves in center of each diamond */

static restriction_thing two_faced_qtag= {4, {6, 2, 7, 3},                                           {0}, {0}, {0}, FALSE, chk_wave};           /* check for two-faced line across the center */

static restriction_thing qtag_1        = {4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},                {2, 4, 5}, {2, 0, 1}, FALSE, chk_dmd_qtag};
static restriction_thing dmd4_1        = {4, {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0}, {4, 8, 9, 10, 11}, {4, 0, 1, 2, 3}, FALSE, chk_dmd_qtag};

static restriction_thing dmd_1         = {4, {0}, {4, 0, 1, 2, 3},                            {1, 0},    {1, 2},    FALSE, chk_dmd_qtag};
static restriction_thing ptpd_1        = {4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},                {2, 0, 6}, {2, 2, 4}, FALSE, chk_dmd_qtag};
static restriction_thing qtag_3        = {4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},                {2, 0, 1}, {2, 4, 5}, FALSE, chk_dmd_qtag};

static restriction_thing dmd4_3        = {4, {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0}, {4, 0, 1, 2, 3}, {4, 8, 9, 10, 11}, FALSE, chk_dmd_qtag};

static restriction_thing dmd_3         = {4, {0}, {4, 0, 1, 2, 3},                            {1, 2},    {1, 0},    FALSE, chk_dmd_qtag};
static restriction_thing ptpd_3        = {4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},                {2, 2, 4}, {2, 0, 6}, FALSE, chk_dmd_qtag};
static restriction_thing dmd_q         = {4, {0}, {4, 0, 1, 2, 3},                            {0},       {0},       FALSE, chk_dmd_qtag};
static restriction_thing qtag_d        = {4, {4, 2, 3, 6, 7}, {4, 0, 1, 4, 5},                {0},       {0},       FALSE, chk_dmd_qtag};

static restriction_thing dmd4_d        = {8, {8, 4, 5, 6, 7, 12, 13, 14, 15}, {8, 0, 1, 2, 3, 8, 9, 10, 11}, {0},       {0},       FALSE, chk_dmd_qtag};

static restriction_thing dmd_d         = {4, {2, 0, 2}, {2, 1, 3},                            {0},       {0},       FALSE, chk_dmd_qtag};
static restriction_thing ptpd_d        = {4, {4, 0, 2, 4, 6}, {4, 1, 3, 5, 7},                {0},       {0},       FALSE, chk_dmd_qtag};
static restriction_thing all_4_ns      = {4, {4, 0, 1, 2, 3}, {0},                            {0},       {0},       FALSE, chk_dmd_qtag};
static restriction_thing all_4_ew      = {4, {0}, {4, 0, 1, 2, 3},                            {0},       {0},       FALSE, chk_dmd_qtag};
static restriction_thing all_8_ns      = {4, {8, 0, 1, 2, 3, 4, 5, 6, 7}, {0},                {0},       {0},       FALSE, chk_dmd_qtag};
static restriction_thing all_16_ns     = {4, {16, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}, {0}, {0}, {0}, FALSE, chk_dmd_qtag};
static restriction_thing all_8_ew      = {4, {0}, {8, 0, 1, 2, 3, 4, 5, 6, 7},                {0},       {0},       FALSE, chk_dmd_qtag};

static restriction_thing r1qt          = {4, {6, 7, 3, 2},             {4, 4, 0, 5, 1},                   {0}, {0}, TRUE, chk_qtag};
static restriction_thing r3qt          = {4, {6, 7, 3, 2},             {4, 0, 4, 1, 5},                   {0}, {0}, TRUE, chk_qtag};
static restriction_thing r1ql          = {4, {6, 3, 7, 2},             {4, 4, 0, 5, 1},                   {0}, {0}, TRUE, chk_qtag};
static restriction_thing r3ql          = {4, {6, 3, 7, 2},             {4, 0, 4, 1, 5},                   {0}, {0}, TRUE, chk_qtag};

static restriction_thing check_4x1_8ch = {4, {0, 1, 3, 2},                                           {0}, {0}, {0}, FALSE, chk_wave};

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
   {s1x8, cr_miniwaves, &mnwv_1x8},
   {s1x3, cr_1fl_only, &cpls_1x3},
   {s1x3, cr_3x3couples_only, &cpls_1x3},
   {s1x3, cr_wave_only, &wave_1x3},
   {s1x4, cr_wave_only, &wave_1x4},
   {s1x4, cr_2fl_only, &two_faced_1x4},
   {s1x4, cr_1fl_only, &all_same_4},
   {s1x4, cr_4x4couples_only, &all_same_4},
   {s1x4, cr_all_facing_same, &all_same_4},
   {s1x4, cr_couples_only, &cpls_1x4},
   {s1x4, cr_miniwaves, &mnwv_1x4},
   {s1x4, cr_magic_only, &invert_1x4},
   {s1x4, cr_all_ns, &all_4_ns},
   {s1x4, cr_all_ew, &all_4_ew},
   {s2x4, cr_all_ns, &all_8_ns},
   {s2x4, cr_all_ew, &all_8_ew},
   {s1x8, cr_all_ns, &all_8_ns},
   {s1x8, cr_all_ew, &all_8_ew},
   {s1x6, cr_miniwaves, &mnwv_1x6},
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
   {s1x16, cr_miniwaves, &mnwv_1x16},
   {s2x3, cr_all_facing_same, &all_same_6},
   {s2x3, cr_1fl_only, &one_faced_2x3},
   {s2x4, cr_2fl_only, &two_faced_2x4},
   {s2x4, cr_wave_only, &wave_2x4},
   {s2x3, cr_wave_only, &wave_2x3},
   {s2x3, cr_magic_only, &cmagic_2x3},
   {s2x4, cr_magic_only, &cmagic_2x4},
   {s2x4, cr_li_lo, &lio_2x4},
   {s2x4, cr_all_facing_same, &all_same_8},
   {s2x4, cr_1fl_only, &one_faced_2x4},
   {s2x4, cr_couples_only, &cpls_2x4},
   {s2x4, cr_miniwaves, &mnwv_2x4},
   {s_qtag, cr_wave_only, &wave_qtag},
   {s_qtag, cr_miniwaves, &miniwave_qtg},
   {s_qtag, cr_2fl_only, &two_faced_qtag},
   {s2x8, cr_wave_only, &wave_2x8},
   {s2x8, cr_4x4_2fl_only, &two_faced_4x4_2x8},
   {s1x16, cr_4x4couples_only, &cplsof4_2x8},
   {s2x8, cr_4x4couples_only, &cplsof4_2x8},
   {s2x8, cr_couples_only, &cpls_2x8},
   {s2x8, cr_miniwaves, &mnwv_2x8},
   {s2x6, cr_wave_only, &wave_2x6},
   {s2x6, cr_3x3_2fl_only, &two_faced_2x6},
   {s2x6, cr_3x3couples_only, &cpls_2x6},
   {s1x12, cr_3x3couples_only, &cpls_2x6},
   {s1x2, cr_wave_only, &wave_1x2},
   {s1x2, cr_2fl_only, &all_same_2},
   {s1x2, cr_couples_only, &all_same_2},
   {s1x2, cr_all_facing_same, &all_same_2},
   {s1x2, cr_miniwaves, &all_diff_2},
   {s3x4, cr_wave_only, &wave_3x4},
   {s3x4, cr_2fl_only, &two_faced_3x4},
   {s_thar, cr_wave_only, &wave_thar},
   {s_thar, cr_1fl_only, &thar_1fl},
   {s_qtag, cr_real_1_4_tag, &r1qt},
   {s_qtag, cr_real_3_4_tag, &r3qt},
   {s_qtag, cr_real_1_4_line, &r1ql},
   {s_qtag, cr_real_3_4_line, &r3ql},
   {nothing}
};


static restr_initializer *restr_hash_table[NUM_RESTR_HASH_BUCKETS];


extern void initialize_restr_tables(void)
{
   restr_initializer *tabp;
   int i;

   for (i=0 ; i<NUM_RESTR_HASH_BUCKETS ; i++)
      restr_hash_table[i] = (restr_initializer *) 0;

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
   else if (t.assump_col == 1) {
      switch (k) {
         case s2x3:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &cwave_2x3;
            else if (t.assumption == cr_magic_only)
               restr_thing_ptr = &cmagic_2x3;
            else if (t.assumption == cr_peelable_box)
               restr_thing_ptr = &peelable_3x2;
            else if (t.assumption == cr_all_facing_same)
               restr_thing_ptr = &all_same_6;
            break;
         case s1x4:
            if (t.assumption == cr_all_facing_same)
               restr_thing_ptr = &all_same_4;
            else if (t.assumption == cr_2fl_only)
               restr_thing_ptr = &two_faced_1x4;
            break;
         case s1x6:
            if (t.assumption == cr_all_facing_same)
               restr_thing_ptr = &all_same_6;
            break;
         case s1x8:
            if (t.assumption == cr_all_facing_same)
               restr_thing_ptr = &all_same_8;
            break;
         case s2x4:
            if (t.assumption == cr_li_lo)
               restr_thing_ptr = &wave_2x4;
            else if (t.assumption == cr_2fl_only)
               restr_thing_ptr = &two_faced_2x4;
            else if (t.assumption == cr_wave_only)
               restr_thing_ptr = &cwave_2x4;
            else if (t.assumption == cr_magic_only)
               restr_thing_ptr = &cmagic_2x4;
            else if (t.assumption == cr_peelable_box)
               restr_thing_ptr = &peelable_4x2;
            else if (t.assumption == cr_couples_only)
               restr_thing_ptr = &cpls_4x2;
            else if (t.assumption == cr_miniwaves)
               restr_thing_ptr = &mnwv_4x2;
            else if (t.assumption == cr_all_facing_same)
               restr_thing_ptr = &all_same_8;
            break;
         case s1x2:
            if (t.assumption == cr_all_facing_same)
               restr_thing_ptr = &all_same_2;
            break;
         case s2x6:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &cwave_2x6;
            else if (t.assumption == cr_peelable_box)
               restr_thing_ptr = &peelable_6x2;
            else if (t.assumption == cr_couples_only)
               restr_thing_ptr = &cpls_6x2;
            break;
         case s2x8:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &cwave_2x8;
            else if (t.assumption == cr_peelable_box)
               restr_thing_ptr = &peelable_8x2;
            else if (t.assumption == cr_couples_only)
               restr_thing_ptr = &cpls_8x2;
            break;
         case s_qtag:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &cwave_qtg;
            break;
         case s_trngl:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &wave_tgl;   /* isn't this bogus?  It checks for TANDEM-BASE. */
            break;
         case sdmd:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &wave_dmd;
            else if (t.assumption == cr_miniwaves)
               restr_thing_ptr = &wave_dmd;
            break;
         case s_ptpd:
            if (t.assumption == cr_wave_only)
               restr_thing_ptr = &wave_ptpd;
            else if (t.assumption == cr_miniwaves)
               restr_thing_ptr = &miniwave_ptpd;
            break;
      }

      if (restr_thing_ptr) return restr_thing_ptr;
   }

   switch (k) {
      case s2x2:
            switch (t.assumption) {
               case cr_wave_only:
                  restr_thing_ptr = &box_wave;
                  break;
               case cr_all_facing_same: case cr_2fl_only:
                  restr_thing_ptr = &box_1face;
                  break;
               case cr_trailers_only: case cr_leads_only: case cr_li_lo:
                  restr_thing_ptr = &box_in_or_out;
                  break;
               case cr_magic_only:
                  restr_thing_ptr = &box_magic;
                  break;
            }
         break;
      case s4x4:
         if (t.assumption == cr_wave_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &s4x4_wave;
         if (t.assumption == cr_2fl_only && (t.assump_col & 1) == 0)
            restr_thing_ptr = &s4x4_2fl;
         break;
      case s2x4:
         if (t.assumption == cr_wave_only && t.assump_col == 2)
            restr_thing_ptr = &mnwv_2x4;  /* WRONG!!!  This is "assume normal boxes" while in gen'lized lines, but will accept any miniwaves. */
         else if (t.assumption == cr_magic_only && (t.assump_col & 2) == 2)
            /* "assume inverted boxes", people are in general lines (col=2) or cols (col=3) */
            restr_thing_ptr = &cmagic_2x4;   /* This is wrong, need something that allows real mag col or unsymm congruent inv boxes.
                                                      This is a "check mixed groups" type of thing!!!! */
         break;
      case s1x2:
         if (t.assumption == cr_wave_only && t.assump_col == 2)
            restr_thing_ptr = &wave_1x2;
         break;
      case s_qtag:
         if (t.assumption == cr_jleft)
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
      case s4dmd:
         if (t.assumption == cr_jleft)
            restr_thing_ptr = &jleft_4dmd;
         else if (t.assumption == cr_jright)
            restr_thing_ptr = &jright_4dmd;
         else if (t.assumption == cr_diamond_like)
            restr_thing_ptr = &dmd4_d;
         else if (t.assumption == cr_qtag_like)
            restr_thing_ptr = &all_16_ns;
         else if (t.assumption == cr_gen_1_4_tag)
            restr_thing_ptr = &dmd4_1;
         else if (t.assumption == cr_gen_3_4_tag)
            restr_thing_ptr = &dmd4_3;
         break;
      case s_trngl:
         if (t.assumption == cr_miniwaves && t.assump_col == 0)
            restr_thing_ptr = &wave_tgl;
         else if (t.assumption == cr_wave_only && t.assump_col == 0)
            restr_thing_ptr = &wave_tgl;
         break;
      case sdmd:
         if (t.assumption == cr_jright)
            restr_thing_ptr = &jright_dmd;
         else if (t.assumption == cr_diamond_like)
            restr_thing_ptr = &dmd_d;
         else if (t.assumption == cr_qtag_like)
            restr_thing_ptr = &dmd_q;
         else if (t.assumption == cr_gen_1_4_tag)
            restr_thing_ptr = &dmd_1;
         else if (t.assumption == cr_gen_3_4_tag)
            restr_thing_ptr = &dmd_3;
         break;
      case s_ptpd:
         if (t.assumption == cr_jright)
            restr_thing_ptr = &jright_ptpd;
         else if (t.assumption == cr_diamond_like)
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

static long_boolean usurping_writechar = FALSE;




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

   if (destcurr < &current_line[MAX_PRINT_LENGTH] || usurping_writechar)
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
   num &= 0xF;

   switch (indicator) {
      case '9': case 'a': case 'b': case 'B': case 'D':
         nn = '0' + num;
         if (indicator == '9')
            writechar(nn);
         else if (indicator == 'B' || indicator == 'D') {
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
   uint32 index = cptr->options.number_fields;
   Const char *f;

   f = s;     /* Argument "s", if non-null, overrides the concept name in the table. */
   if (!f) f = cptr->concept->name;

   while (*f) {
      if (f[0] == '@') {
         switch (f[1]) {
            case 'a': case 'b': case 'B': case 'D': case 'u': case '9':
               write_nice_number(f[1], index);
               f += 2;
               index >>= 4;
               continue;
            case '6':
               writestuff(selector_list[cptr->options.who].name_uc);
               f += 2;
               continue;
            case 'k':
               writestuff(selector_list[cptr->options.who].sing_name_uc);
               f += 2;
               continue;
         }
      }

      writechar(*f++);
   }
}



Private void print_call_name(callspec_block *call, parse_block *pb)
{
   if (     (call->callflags1 & CFLAG1_NUMBER_MASK) &&
            (pb->options.number_fields & 0xF)) {
      writestuff_with_decorations(pb, call->name);
   }
   else {
      writestuff(call->menu_name);
   }
}



extern void unparse_call_name(callspec_block *call, char *s, parse_block *pb)
{
   char saved_lastchar = lastchar;
   char saved_lastlastchar = lastlastchar;
   char *saved_lastblank = lastblank;
   char *saved_destcurr = destcurr;
   destcurr = s;
   usurping_writechar = TRUE;

   print_call_name(call, pb);
   writechar('\0');

   lastchar = saved_lastchar;
   lastlastchar = saved_lastlastchar;
   lastblank = saved_lastblank;
   destcurr = saved_destcurr;
   usurping_writechar = FALSE;
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


extern void fail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   longjmp(longjmp_ptr->the_buf, 1);
}


extern void fail2(Const char s1[], Const char s2[])
{
   (void) strncpy(error_message1, s1, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   (void) strncpy(error_message2, s2, MAX_ERR_LENGTH);
   error_message2[MAX_ERR_LENGTH-1] = '\0';
   longjmp(longjmp_ptr->the_buf, 2);
}


extern void failp(uint32 id1, Const char s[])
{
   collision_person1 = id1;
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   longjmp(longjmp_ptr->the_buf, 6);
}


extern void specialfail(Const char s[])
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
      case '0': case 'm':
         return "<ANYTHING>";
      case 'N':
         return "<ANYCIRC>";
      case '6': case 'k':
         return "<ANYONE>";
      case 'h':
         return "<DIRECTION>";
      case '9':
         return "<N>";
      case 'a': case 'b': case 'B': case 'D':
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

         if (item->concparseflags & CONCPARSE_PARSE_F_TYPE) {
            /* This is an "F" type concept. */
            comma_after_next_concept = TRUE;
            last_was_t_type = FALSE;
            force = did_concept && !last_was_l_type;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }
         else if (item->concparseflags & CONCPARSE_PARSE_L_TYPE) {
            /* This is an "L" type concept. */
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else if (item->concparseflags & CONCPARSE_PARSE_G_TYPE) {
            /* This is a "leading T/trailing L" type concept, also known as a "G" concept. */
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
                  writestuff(ordinals[local_cptr->options.number_fields-1]);
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
               selector_kind opp = selector_list[local_cptr->options.who].opposite;
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
            else if (k == concept_special_sequential) {
               if (item->value.arg1 == 2)
                  writestuff(" :");   /* This is "start with". */
               else
                  writestuff(",");
            }
            else
               writestuff(" BY");

            next_cptr = subsidiary_ptr;
         }
         else if (local_cptr && (k == concept_left || k == concept_cross || k == concept_magic || k == concept_interlocked)) {

            /* These concepts want to take special action if there are no following concepts and
               certain escape characters are found in the name of the following call. */

            uint64 finaljunk;
            callspec_block *target_call;
            parse_block *tptr;
            
            /* Skip all final concepts, then demand that what remains is a marker (as opposed to a serious
                concept), and that a real call has been entered, and that its name starts with "@g". */
            tptr = process_final_concepts(next_cptr, FALSE, &finaljunk);

            if (tptr) {
               target_call = tptr->call;
   
               if ((tptr->concept->kind <= marker_end_of_list) && target_call && (
                     target_call->callflagsf & (ESCAPE_WORD__LEFT | ESCAPE_WORD__MAGIC | ESCAPE_WORD__CROSS | ESCAPE_WORD__INTLK))) {
                  if (k == concept_left) {
                     /* See if this is a call whose name naturally changes when the "left" concept is used. */
                     if (target_call->callflagsf & ESCAPE_WORD__LEFT) {
                        use_left_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else if (k == concept_magic) {
                     /* See if this is a call that wants the "magic" modifier to be moved inside its name. */
                     if (target_call->callflagsf & ESCAPE_WORD__MAGIC) {
                        use_magic_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else if (k == concept_interlocked) {
                     /* See if this is a call that wants the "interlocked" modifier to be moved inside its name. */
                     if (target_call->callflagsf & ESCAPE_WORD__INTLK) {
                        use_intlk_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else {
                     /* See if this is a call that wants the "cross" modifier to be moved inside its name. */
                     if (target_call->callflagsf & ESCAPE_WORD__CROSS) {
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
         else {
            if (     (k == concept_nth_part && local_cptr->concept->value.arg1 == 8) ||      /* "DO THE <Nth> PART" */
                     (k == concept_so_and_so_only && local_cptr->concept->value.arg1 == 11) ||    /* "<ANYONE> WORK" */
                     (k == concept_snag_mystic && item->value.arg1 == CMD_MISC2__CTR_END_INVERT) ||        /* If INVERT
                                                               is followed by another concept, it must be SNAG or MYSTIC. */
                     (k == concept_meta && (local_cptr->concept->value.arg1 == 3 || local_cptr->concept->value.arg1 == 7))) {   /* Initially/finally. */
               request_comma_after_next_concept = TRUE;     /* These concepts require a comma after the following concept. */
            }

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

         parse_block *sub1_ptr;
         parse_block *sub2_ptr;
         parse_block *search;
         long_boolean pending_subst1, pending_subst2;

         selector_kind i16junk = local_cptr->options.who;
         direction_kind idirjunk = local_cptr->options.where;
         uint32 number_list = local_cptr->options.number_fields;
         callspec_block *localcall = local_cptr->call;
         parse_block *save_cptr = local_cptr;

         long_boolean subst1_in_use = FALSE;
         long_boolean subst2_in_use = FALSE;

         if (request_final_space) writestuff(" ");

         if (k == concept_another_call_next_mod) {
            search = save_cptr->next;
            while (search) {
               parse_block *subsidiary_ptr = search->subsidiary_root;
               long_boolean this_is_subst1 = FALSE;
               long_boolean this_is_subst2 = FALSE;
               if (subsidiary_ptr) {
                  switch (search->replacement_key) {
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
#ifdef SCREWED_UP_REVERTS
                        search = save_cptr->next;
                        while (search) {
                           parse_block *subsidiary_ptr = search->subsidiary_root;
                           if (subsidiary_ptr && subsidiary_ptr->call && (subsidiary_ptr->call->callflags1 & CFLAG1_BASE_TAG_CALL_MASK)) {
                              parse_block *search2;
                              parse_block *subsidiary_ptr2 = subsidiary_ptr;

                              for (;;) {
                                 parse_block *search3;
                                 parse_block *subsidiary_ptr3;

                                 for (search3 = subsidiary_ptr2->next ; search3 ; search3 = search3->next) {
                                    subsidiary_ptr3 = search3->subsidiary_root;

                                    if (subsidiary_ptr3 && subsidiary_ptr3->call && (subsidiary_ptr3->call->callflags1 & CFLAG1_BASE_TAG_CALL_MASK)) {
                                       if (subsidiary_ptr3->concept->kind == concept_another_call_next_mod)
                                          goto search_again;
                                       else if (subsidiary_ptr2 == subsidiary_ptr) {
                                          print_recurse(subsidiary_ptr, 0);
                                          goto did_tagger;
                                       }
                                       else {
                                          search2->subsidiary_root = subsidiary_ptr3;
                                          search3->subsidiary_root = subsidiary_ptr;
                                          print_recurse(subsidiary_ptr2, 0);
                                          search2->subsidiary_root = subsidiary_ptr2;
                                          search3->subsidiary_root = subsidiary_ptr3;
                                          goto did_tagger;
                                       }
                                    }
                                 }

                                 break;    /* Can't seem to find anything. */

                                 search_again:
                                 search2 = search3;
                                 subsidiary_ptr2 = subsidiary_ptr3;
                              }

                              print_recurse(subsidiary_ptr, 0);
                              goto did_tagger;
                           }
                           search = search->next;
                        }

                        /* We didn't find the tagger.  It must not have been entered into the parse tree.
                           See if we can get it from the "tagger" field. */

                        if (save_cptr->options.tagger > 0)
                           writestuff(tagger_calls[save_cptr->options.tagger >> 5][(save_cptr->options.tagger & 0x1F)-1]->menu_name);
                        else
                           writestuff("NO TAGGER???");

                        did_tagger:
#else
                        search = save_cptr->next;
                        while (search) {
                           parse_block *subsidiary_ptr = search->subsidiary_root;
                           if (subsidiary_ptr && subsidiary_ptr->call && (subsidiary_ptr->call->callflags1 & CFLAG1_BASE_TAG_CALL_MASK)) {
                              print_recurse(subsidiary_ptr, 0);
                              goto did_tagger;
                           }
                           search = search->next;
                        }

                        /* We didn't find the tagger.  It must not have been entered into the parse tree.
                           See if we can get it from the "tagger" field. */

                        if (save_cptr->options.tagger > 0)
                           writestuff(tagger_calls[save_cptr->options.tagger >> 5][(save_cptr->options.tagger & 0x1F)-1]->menu_name);
                        else
                           writestuff("NO TAGGER???");

                        did_tagger:
#endif
                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the indicator */
                        break;
                     case 'N':
                        write_blank_if_needed();

                        /* Find the base circ call that this is invoking. */

                        search = save_cptr->next;
                        while (search) {
                           parse_block *subsidiary_ptr = search->subsidiary_root;
                           if (subsidiary_ptr && subsidiary_ptr->call && (subsidiary_ptr->call->callflags1 & CFLAG1_BASE_CIRC_CALL)) {
                              print_recurse(subsidiary_ptr, PRINT_RECURSE_CIRC);
                              goto did_circcer;
                           }
                           search = search->next;
                        }

                        /* We didn't find the circcer.  It must not have been entered into the parse tree.
                           See if we can get it from the "circcer" field. */

                        if (save_cptr->options.circcer > 0)
                           writestuff(circcer_calls[(save_cptr->options.circcer)-1]->menu_name);
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
                     case '9': case 'a': case 'b': case 'B': case 'D': case 'u':    /* Need to plug in a number. */
                        write_blank_if_needed();
                        write_nice_number(savec, number_list);
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

            for (search = save_cptr->next ; search ; search = search->next) {
               callspec_block *cc;
               parse_block *subsidiary_ptr = search->subsidiary_root;

               /* If we have a subsidiary_ptr, handle the replacement that is indicated.
                  BUT:  if the call shown in the subsidiary_ptr is a base tag call, don't
                  do anything -- such substitutions were already taken care of.
                     BUT:  only check if there is actually a call there. */

               if (!subsidiary_ptr) continue;
               cc = subsidiary_ptr->call;

               if (     !cc ||    /* If no call pointer, it isn't a tag base call. */
                           (
                              !(cc->callflags1 & (CFLAG1_BASE_TAG_CALL_MASK)) &&
                                 (
                                    !(cc->callflags1 & (CFLAG1_BASE_CIRC_CALL)) ||
                                    search->call != base_calls[BASE_CALL_CIRCCER]
                                 )
                           )) {


                  callspec_block *replaced_call = search->call;

                  /* Need to check for case of replacing one star turn with another. */

                  if ((first_replace == 0) &&
                        (replaced_call->callflags1 & CFLAG1_IS_STAR_CALL) &&
                              ((subsidiary_ptr->concept->kind == marker_end_of_list) ||
                              subsidiary_ptr->concept->kind == concept_another_call_next_mod) &&
                              cc &&
                              ((cc->callflags1 & CFLAG1_IS_STAR_CALL) ||
                              cc->schema == schema_nothing)) {
                     first_replace++;
                     write_blank_if_needed();

                     if (cc->schema == schema_nothing)
                        writestuff("BUT don't turn the star");
                     else {
                        writestuff("BUT [");
                        print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                        writestuff("]");
                     }
                  }
                  else {
                     switch (search->replacement_key) {
                        case 1:
                        case 2:
                        case 3:
                           /* This is a natural replacement.  It may already have been taken care of. */
                           if (pending_subst1 || search->replacement_key == 3) {
                              write_blank_if_needed();
                              if (search->replacement_key == 3)
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
                           /* This is a forced replacement. */
                           write_blank_if_needed();
                           first_replace++;
                           if (first_replace == 1)
                              writestuff("BUT REPLACE ");
                           else
                              writestuff("AND REPLACE ");

                           print_call_name(replaced_call, search);
                           writestuff(" WITH [");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                           break;
                     }
                  }
               }
            }
         }

         break;
      }
   }

   return;
}


/* These static variables are used by printperson. */

static char peoplenames1[] = "11223344";
static char peoplenames2[] = "BGBGBGBG";
static char alt1_names1[] = "        ";
static char alt1_names2[] = "1P2R3O4C";
static char directions[] = "?>?<????^?V?????";

/* This could get changed if a user interface for Sdtty wishes to use pretty graphics characters. */
char *ui_directions = directions;

/* These could get changed if the user requests special naming. */
static char *pn1 = peoplenames1;
static char *pn2 = peoplenames2;


Private void printperson(unsigned int x)
{
   if (x & BIT_PERSON) {
      static char personbuffer[] = " ZZZ";
      personbuffer[1] = pn1[(x >> 6) & 7];
      personbuffer[2] = pn2[(x >> 6) & 7];
      /* But we never write anything other than standard ASCII to the transcript file. */
      personbuffer[3] = (enable_file_writing ? directions : ui_directions)[x & 017];
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
   configuration *this_item = &history[history_index];

   if (write_to_file == file_write_double && !singlespace_mode)
      doublespace_file();

   centersp = this_item->centersp;

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

   thing = this_item->command_root;
   
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

   /* First, don't print both "bad concept level" and "bad modifier level". */

   if ((1 << (warn__bad_concept_level & 0x1F)) & this_item->warnings.bits[warn__bad_concept_level>>5])
      this_item->warnings.bits[warn__bad_modifier_level>>5] &= ~(1 << (warn__bad_modifier_level & 0x1F));

   /* Or "opt for parallelogram" and "each 1x4". */

   if ((1 << (warn__check_pgram & 0x1F)) & this_item->warnings.bits[warn__check_pgram>>5])
      this_item->warnings.bits[warn__each1x4>>5] &= ~(1 << (warn__each1x4 & 0x1F));

   /* Or "each 1x6" and "each 1x3". */

   if ((1 << (warn__split_1x6 & 0x1F)) & this_item->warnings.bits[warn__split_1x6>>5])
      this_item->warnings.bits[warn__split_to_1x6s>>5] &= ~(1 << (warn__split_to_1x6s & 0x1F));

   for (w=0 ; w<NUM_WARNINGS ; w++) {
      if ((1 << (w & 0x1F)) & this_item->warnings.bits[w>>5]) {
         writestuff("  Warning:  ");
         writestuff(&warning_strings[w][1]);
         newline();
      }
   }

   if (picture || this_item->draw_pic) {
      if (alternate_person_glyphs == 1) {
         pn1 = alt1_names1;
         pn2 = alt1_names2;
      }

      printsetup(&this_item->state);
   }

   /* Record that this history item has been written to the UI. */
   this_item->text_line = text_line_count;
}


extern void warn(warning_index w)
{
   if (w == warn__none) return;

   /* If this is an "each 1x4" type of warning, and we already have
      such a warning, don't enter the new one.  The first one takes precedence. */

   if (dyp_each_warnings.bits[w>>5] & (1 << (w & 0x1F))) {
      int i;

      for (i=0 ; i<WARNING_WORDS ; i++) {
         if (dyp_each_warnings.bits[i] & history[history_ptr+1].warnings.bits[i])
            return;
      }
   }

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


static Const long int spindle1[] = {d_east, d_west, 0, 6, 1, 5, 2, 4, -1};
static Const long int short1[] = {d_north, d_south, 0, 2, 5, 3, -1};
static Const long int dmd1[] = {d_east, d_west, 1, 3, -1};
static Const long int line1[] = {d_north, d_south, 1, 3, -1};
static Const long int lines1[] = {d_north, d_south, 1, 2, 6, 5, -1};
static Const long int qtag1[] = {d_north, d_south, 3, 2, 6, 7, -1};
static Const long int ptpd1[] = {d_east, d_west, 1, 3, 7, 5, -1};

            

extern Const long int *get_rh_test(setup_kind kind)
{
   switch (kind) {
      case s_spindle: return spindle1;
      case s_short6:  return short1;
      case sdmd:      return dmd1;
      case s1x4:      return line1;
      case s2x4:      return lines1;
      case s_qtag:    return qtag1;
      case s_ptpd:    return ptpd1;
      default:        return (Const long int *) 0;
   }
}



extern long_boolean verify_restriction(
   setup *ss,
   restriction_thing *rr,
   assumption_thing tt,
   long_boolean instantiate_phantoms,
   long_boolean *failed_to_instantiate)
{
   int idx, limit, i, j, k;
   uint32 t;
   uint32 qa0, qa1, qa2, qa3;
   uint32 qaa[4], qab[4];
   uint32 pdir, qdir, pdirodd, qdirodd;
   uint32 dirtest[2];
   int phantom_count = 0;

   dirtest[0] = 0;
   dirtest[1] = 0;

   *failed_to_instantiate = TRUE;

   switch (rr->check) {
      case chk_wave:
         qaa[0] = tt.assump_both;
         qaa[1] = tt.assump_both << 1;
         qab[0] = 0;
         qab[2] = 0;

         for (idx=0; idx<rr->size; idx++) {
            if ((t = ss->people[rr->map1[idx]].id1) != 0) { qaa[idx&1] |=  t; qaa[(idx&1)^1] |= t^2; }
         }

         if ((qaa[0] & qaa[1] & 2) != 0)
            goto bad;

         if (rr->ok_for_assume) {
            for (idx=0; idx<rr->size; idx++)
               qab[idx&2] |= ss->people[rr->map1[idx]].id1;

            if ((tt.assump_col | rr->map2[0]) & 4) {
               qab[2] >>= 3;
            }
            else if (tt.assump_col == 1) {
               qab[0] >>= 3;
               qab[2] >>= 3;
            }

            if ((qab[0]|qab[2]) & 1) goto bad;

            if (instantiate_phantoms) {
               *failed_to_instantiate = FALSE;

               if (qaa[0] == 0) fail("Need live person to determine handedness.");

               if ((tt.assump_col | rr->map2[0]) & 4) {
                  if ((qaa[0] & 2) == 0) { pdir = d_north; qdir = d_south; }
                  else                   { pdir = d_south; qdir = d_north; }
                  pdirodd = rotcw(pdir); qdirodd = rotcw(qdir);
               }
               else if (tt.assump_col == 1) {
                  if ((qaa[0] & 2) == 0) { pdir = d_east; qdir = d_west; }
                  else                   { pdir = d_west; qdir = d_east; }
                  pdirodd = pdir; qdirodd = qdir;
               }
               else {
                  if ((qaa[0] & 2) == 0) { pdir = d_north; qdir = d_south; }
                  else                   { pdir = d_south; qdir = d_north; }
                  pdirodd = pdir; qdirodd = qdir;
               }

               for (i=0; i<rr->size; i++) {
                  int p = rr->map1[i];
   
                  if (!ss->people[p].id1) {
                     if (phantom_count >= 16) fail("Too many phantoms.");
   
                     ss->people[p].id1 =           (i&1) ?
                                          ((i&2) ? qdirodd : qdir) :
                                          ((i&2) ? pdirodd : pdir);
   
                     ss->people[p].id1 |= BIT_ACT_PHAN | ((phantom_count++) << 6);
                     ss->people[p].id2 = 0;
                  }
                  else if (ss->people[p].id1 & BIT_ACT_PHAN)
                     fail("Active phantoms may only be used once.");
               }
            }
         }

         goto good;
      case chk_box:
         qa0 = (tt.assump_both << 1) & 2;
         qa1 = tt.assump_both & 2;
         qa2 = qa1;
         qa3 = qa0;

         for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) {
            if ((t = ss->people[idx].id1) != 0) {
               qa0 |= t^rr->map1[idx]^0;
               qa1 |= t^rr->map1[idx]^2;
               qa2 |= t^rr->map2[idx]^1;
               qa3 |= t^rr->map2[idx]^3;
            }
         }

         if ((qa1&3) && (qa0&3) && (qa3&3) && (qa2&3)) goto bad;

         if (rr->ok_for_assume) {
            if (instantiate_phantoms) {
               if (!(qa0 & BIT_PERSON))
                  fail("Need live person to determine handedness.");

               for (i=0 ; i<=setup_attrs[ss->kind].setup_limits ; i++) {
                  if (!ss->people[i].id1) {
                     if (phantom_count >= 16)
                        fail("Too many phantoms.");
      
                     pdir = (qa0&1) ?
                           (d_east ^ ((rr->map2[i] ^ qa2) & 2)) :
                           (d_north ^ ((rr->map1[i] ^ qa0) & 2));
      
                     ss->people[i].id1 = pdir | BIT_ACT_PHAN | ((phantom_count++) << 6);
                     ss->people[i].id2 = 0;
                  }
                  else if (ss->people[i].id1 & BIT_ACT_PHAN)
                     fail("Active phantoms may only be used once.");
               }

               *failed_to_instantiate = FALSE;
            }
         }

         goto good;
      case chk_groups:
         limit = rr->map2[0];
   
         for (idx=0; idx<limit; idx++) {
            qa0 = 0; qa1 = 0;

            for (i=0,j=idx; i<rr->size; i++,j+=limit) {
               if ((t = ss->people[rr->map1[j]].id1) != 0) { qa0 |= t; qa1 |= t^2; }
            }

            if ((qa0 & qa1 & 2) != 0) goto bad;

            if (rr->ok_for_assume) {
               if (tt.assump_col == 1) {
                  if ((qa0 & 2) == 0) { pdir = d_east; }
                  else                { pdir = d_west; }

                  qa0 >>= 3;
                  qa1 >>= 3;
               }
               else {
                  if ((qa0 & 2) == 0) { pdir = d_north; }
                  else                { pdir = d_south; }
               }

               if ((qa0|qa1)&1) goto bad;

               if (instantiate_phantoms) {
                  if (qa0 == 0) fail("Need live person to determine handedness.");

                  for (i=0,k=0 ; k<rr->size ; i+=limit,k++) {
                     int p = rr->map1[idx+i];
   
                     personrec *pq = &ss->people[p];
                     t = pq->id1;
      
                     if (!t) {
                        if (phantom_count >= 16) fail("Too many phantoms.");
                        pq->id1 = pdir | BIT_ACT_PHAN | ((phantom_count++) << 6);
                        pq->id2 = 0;
                     }
                     else if (t & BIT_ACT_PHAN)
                        fail("Active phantoms may only be used once.");
                  }

                  *failed_to_instantiate = FALSE;
               }
            }
         }
         goto good;
      case chk_anti_groups:
         limit = rr->map2[0];

         for (idx=0; idx<limit; idx++) {
            qa0 = 0; qa1 = 0;

            if ((t = ss->people[rr->map1[idx]].id1) != 0)       { qa0 |= t;   qa1 |= t^2; }
            if ((t = ss->people[rr->map1[idx+limit]].id1) != 0) { qa0 |= t^2; qa1 |= t;   }

            if ((qa0 & qa1 & 2) != 0) goto bad;

            if (rr->ok_for_assume) {
               if (tt.assump_col == 1) {
                  if ((qa0 & 2) == 0) { pdir = d_east; qdir = d_west; }
                  else                { pdir = d_west; qdir = d_east; }

                  qa0 >>= 3;
                  qa1 >>= 3;
               }
               else {
                  if ((qa0 & 2) == 0) { pdir = d_north; qdir = d_south; }
                  else                { pdir = d_south; qdir = d_north; }
               }

               if ((qa0|qa1)&1) goto bad;

               if (instantiate_phantoms) {
                  if (qa0 == 0)
                     fail("Need live person to determine handedness.");

                  for (i=0 ; i<=limit ; i += limit) {
                     int p = rr->map1[idx+i];
                     personrec *pq = &ss->people[p];
                     t = pq->id1;
      
                     if (!t) {
                        if (phantom_count >= 16) fail("Too many phantoms.");
                        pq->id1 = (i ? qdir : pdir) | BIT_ACT_PHAN | ((phantom_count++) << 6);
                        pq->id2 = 0;
                     }
                     else if (t & BIT_ACT_PHAN)
                        fail("Active phantoms may only be used once.");
                  }

                  *failed_to_instantiate = FALSE;
               }
            }
         }
         goto good;
      case chk_peelable:
         qa0 = 3; qa1 = 3;
         qa2 = 3; qa3 = 3;

         for (j=0; j<rr->size; j++) {
            if ((t = ss->people[rr->map1[j]].id1) != 0)  { qa0 &= t; qa1 &= t^2; }
            if ((t = ss->people[rr->map2[j]].id1) != 0)  { qa2 &= t; qa3 &= t^2; }
         }

         if ((((~qa0)&3) == 0 || ((~qa1)&3) == 0) && (((~qa2)&3) == 0 || ((~qa3)&3) == 0))
            goto good;

         goto bad;
      case chk_dmd_qtag:
         qa1 = 0;
         qa0 = 0;

         for (idx=0; idx<rr->map1[0]; idx++)
            qa1 |= ss->people[rr->map1[idx+1]].id1;

         for (idx=0; idx<rr->map2[0]; idx++)
            qa0 |= ss->people[rr->map2[idx+1]].id1;

         for (idx=0; idx<rr->map3[0]; idx++) {
            if ((t = ss->people[rr->map3[idx+1]].id1) != 0 && (t & 2) != 0)
               goto bad;
         }

         for (idx=0; idx<rr->map4[0]; idx++) {
            if ((t = ss->people[rr->map4[idx+1]].id1) != 0 && (t & 2) != 2)
               goto bad;
         }

         if ((qa1 & 001) != 0 || (qa0 & 010) != 0)
            goto bad;

         goto good;
      case chk_qtag:
         qaa[0] = tt.assump_both;
         qaa[1] = tt.assump_both << 1;

         for (idx=0; idx<rr->map2[0]; idx++) {
            if ((t = ss->people[rr->map2[idx+1]].id1) != 0 && ((t ^ (idx << 1)) & 2) != 0)
               goto bad;
         }

         for (idx=0; idx<rr->size; idx++) {
            if ((t = ss->people[rr->map1[idx]].id1) != 0) { qaa[idx&1] |=  t; qaa[(idx&1)^1] |= t^2; }
         }

         if ((qaa[0] & qaa[1] & 2) != 0)
            goto bad;

         for (idx=0 ; idx<8 ; idx++) { if (ss->people[idx].id1 & 1) goto bad; }

         goto good;
      default:
         goto bad;    /* Shouldn't happen. */
   }

   good: return TRUE;

   bad: return FALSE;
}


extern callarray *assoc(begin_kind key, setup *ss, callarray *spec)
{
   callarray *p;
   long_boolean booljunk;

   for (p = spec; p; p = p->next) {
      uint32 i, k, t, u, v, w, mask;
      assumption_thing tt;
      int idx, j;
      restriction_thing *rr;

      /* First, we demand that the starting setup be correct.  Also, if a qualifier
         number was specified, it must match. */

      if ((begin_kind) p->start_setup != key) continue;

      /* During initialization, we will be called with a null pointer for ss.
         We need to be careful, and err on the side of acceptance. */

      if (!ss) goto good;

      if ((p->qual_num & 0xF) != 0) {
         number_used = TRUE;
         if ((p->qual_num & 0xF) != (current_options.number_fields & 0xF)+1) continue;
      }

      /* The "16" bit of the "qual_num" field means we require people
         to be not T-boned. */

      if ((p->qual_num & 0x10) != 0) {
         u = 0;

         for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++)
            u |= ss->people[i].id1;

         if ((u & 011) == 011) continue;
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
            goto do_wave_stuff;
         case sq_rwave_only:
            tt.assump_both = 1;   /* To get right-hand only. */
            goto do_wave_stuff;
         case sq_lwave_only:
            tt.assump_both = 2;   /* To get left-hand only. */
            goto do_wave_stuff;
         case sq_magic_only:                   /* 3x2 or 4x2 - magic column; 2x4 - inverted lines; 1x4 - single inverted line; 2x2 - inverted box */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
               case cr_wave_only:
                  goto bad;
            }

            tt.assumption = cr_magic_only;

            switch (ss->kind) {
               case s1x4: case s2x4: case s2x2:
                  goto check_tt;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_facing_in:
            tt.assump_both = 1;   /* To get facing-in only. */
            rr = &box_in_or_out;
            goto check_stuff;
         case sq_facing_out:
            tt.assump_both = 2;   /* To get facing-out only. */
            rr = &box_in_or_out;
            goto check_stuff;
         case sq_in_or_out:                    /* 2x2 - all facing in or all facing out */
            switch (ss->kind) {
               case s2x2:
                  rr = &box_in_or_out;
                  goto check_stuff;
               case s1x2:
                  if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                  if (!(k & ~i & 2)) goto good;

                  goto bad;
               default:
                  goto good;           /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_1fl_only:       /* 1x3/1x4/1x6/1x8 - a 1FL, that is, all 3/4/6/8 facing same; 2x3/2x4 - individual 1FL's */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_1fl_only;
            goto check_tt;
         case sq_2fl_only:             /* 1x4 or 2x4 - 2FL; 4x1 - single DPT or single CDPT */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only: case cr_wave_only: case cr_magic_only: goto bad;
            }

            tt.assumption = cr_2fl_only;
            goto fix_col_line_stuff;
         case sq_couples_only:         /* 1x2/1x4/1x8/2x2/2x4 lines, or 2x4 columns - people are in genuine couples, not miniwaves */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_miniwaves:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_couples_only;
            goto fix_col_line_stuff;
         case sq_3x3couples_only:      /* 1x3/1x6/2x3/2x6/1x12 - each group of 3 people are facing the same way */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_3x3couples_only;
            goto check_tt;
         case sq_4x4couples_only:      /* 1x4/1x8/2x4/2x8/1x16 - each group of 4 people are facing the same way */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
                  goto good;
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            tt.assumption = cr_4x4couples_only;
            goto check_tt;
         case sq_miniwaves:                    /* miniwaves everywhere */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
                  goto bad;
            }

            tt.assumption = cr_miniwaves;

            switch (ss->kind) {
               case s1x2: case s1x4: case s1x6: case s1x8: case s1x16: case s2x4:
               case sdmd: case s_trngl: case s_qtag: case s_ptpd:
                  goto fix_col_line_stuff;
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
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
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
            else if (   ss->kind == s3x6 && (t & 1) == 0 &&
                        ((mask & 0222222) == 0))
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
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_jleft: case cr_jright: case cr_ijleft: case cr_ijright:
                  if (tt.assump_both == 2) goto good;
            }

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
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_jleft: case cr_jright: case cr_ijleft: case cr_ijright:
                  if (tt.assump_both == 1) goto good;
            }

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
         case sq_trade_by:                  /* 4x1/4x2 - setup is (single) trade-by */
            tt.assumption = cr_li_lo;
            tt.assump_col = 1;
            tt.assump_both = (((search_qualifier) p->qualifier) == sq_trade_by) ? 2 : 1;

            switch (ss->kind) {
               case s1x4:
                  rr = &check_4x1_8ch;
                  goto check_stuff;
               case s2x4:
                  goto check_tt;
               default:
                  goto bad;
            }
         case sq_split_dixie:
            if (ss->cmd.cmd_final_flags.final & FINAL__SPLIT_DIXIE_APPROVED) goto good;
            goto bad;
         case sq_not_split_dixie:
            if (!(ss->cmd.cmd_final_flags.final & FINAL__SPLIT_DIXIE_APPROVED)) goto good;
            goto bad;
         case sq_said_tgl:
            if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) goto good;
            goto bad;
         case sq_didnt_say_tgl:
            if (ss->cmd.cmd_misc_flags & CMD_MISC__SAID_TRIANGLE) goto bad;
            goto good;
         case sq_occupied_as_h:
            if (ss->kind != s3x4 || (ss->people[1].id1 | ss->people[2].id1 | ss->people[7].id1 | ss->people[8].id1)) goto bad;
            goto good;
         case sq_occupied_as_qtag:
            if (ss->kind != s3x4 || (ss->people[0].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[9].id1)) goto bad;
            goto good;
         case sq_occupied_as_3x1tgl:
            if (ss->kind == s_qtag) goto good;
            if (ss->kind == s3x4 && !(ss->people[1].id1 | ss->people[2].id1 | ss->people[7].id1 | ss->people[8].id1)) goto good;
            if (ss->kind == s3x4 && !(ss->people[0].id1 | ss->people[3].id1 | ss->people[6].id1 | ss->people[9].id1)) goto good;
            if (ss->kind == s2x6 && !(ss->people[0].id1 | ss->people[2].id1 | ss->people[6].id1 | ss->people[8].id1)) goto good;
            if (ss->kind == s2x6 && !(ss->people[3].id1 | ss->people[5].id1 | ss->people[9].id1 | ss->people[11].id1)) goto good;
            if (ss->kind == s2x3 && !(ss->people[0].id1 | ss->people[2].id1)) goto good;
            if (ss->kind == s2x3 && !(ss->people[3].id1 | ss->people[5].id1)) goto good;
            goto bad;
         case sq_dmd_ctrs_rh:
         case sq_dmd_ctrs_lh:
         case sq_dmd_ctrs_1f:
            {
               search_qualifier kkk;      /* gets set to the qualifier corresponding to what we have. */
               Const long int *p1;
               long int d1;
               long int d2;
               uint32 z = 0;
               long_boolean b1 = TRUE;
               long_boolean b2 = TRUE;

               p1 = get_rh_test(ss->kind);
               if (!p1) goto bad;

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
         case sq_ends_looking_out:
            if (ss->kind != s2x4) goto bad;
            if ((t = ss->people[0].id1) && (t & d_mask) != d_north) goto bad;
            if ((t = ss->people[3].id1) && (t & d_mask) != d_north) goto bad;
            if ((t = ss->people[4].id1) && (t & d_mask) != d_south) goto bad;
            if ((t = ss->people[7].id1) && (t & d_mask) != d_south) goto bad;
            goto good;
         case sq_ripple_centers:
            k ^= (0xA82 ^ 0x144);
            /* FALL THROUGH!!!!!! */
         case sq_ripple_one_end:
            k ^= (0x144 ^ 0x555);
            /* FALL THROUGH!!!!!! */
         case sq_ripple_both_ends:
            if (ss->kind != s1x4) goto good;
            k ^= 0x555;
            mask = 0;

            for (i=0, w=1; i<=setup_attrs[ss->kind].setup_limits; i++, w<<=1) {
               if (selectp(ss, i)) mask |= w;
            }

            if (mask == (k & 0xF) || mask == ((k>>4) & 0xF) || mask == ((k>>8) & 0xF)) goto good;
            goto bad;
         case sq_people_1_and_5_real:
            if (ss->people[1].id1 & ss->people[5].id1) goto good;
            goto bad;
         case sq_ctrs_sel:
            k = 0xFF;     /* K was initialized to zero. */
            /* FALL THROUGH!!!!!! */
         case sq_ends_sel:
            /* Now k=0 for "ends_sel", and 0xFF for "ctrs_sel". */
            switch (ss->kind) {
               case s2x4: k ^= 0x99; break;
               case s_qtag: k ^= 0x33; break;
               default: goto bad;
            }
            j = 1;
            for (idx=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++,k>>=1) {
               if (selectp(ss, idx)) {
                  if (!(k&1)) j = 0;
               }
               else {
                  if (k&1) j = 0;
               }
            }
            if (j) goto good;
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

      do_wave_stuff:

      switch (ss->cmd.cmd_assume.assumption) {
         case cr_1fl_only: case cr_2fl_only: case cr_magic_only: goto bad;
         case cr_ijright: case cr_ijleft: goto bad;
         case cr_wave_only: case cr_jright:
            if (ss->cmd.cmd_assume.assump_both == 2 && tt.assump_both == 1) goto bad;
            if (ss->cmd.cmd_assume.assump_both == 1 && tt.assump_both == 2) goto bad;
            break;
         case cr_jleft:
            if (ss->cmd.cmd_assume.assump_both == 2 && tt.assump_both == 2) goto bad;
            if (ss->cmd.cmd_assume.assump_both == 1 && tt.assump_both == 1) goto bad;
            break;
      }

      tt.assumption = cr_wave_only;

      switch (ss->kind) {
         case s3x4:         /* This only handles lines; not columns -- we couldn't have "wavy" columns that were symmetric. */
         case s_trngl:
         case s_qtag:
            goto check_tt;
         case sdmd:
         case s_ptpd:
            tt.assump_col = 1;
            goto check_tt;
      }

      fix_col_line_stuff:

      switch (ss->kind) {
         case s1x3:
            if (tt.assump_both) goto bad;   /* We can't check a 1x3 for right-or-left-handedness. */
            /* FALL THROUGH!!! */
         case s1x2: case s1x6: case s1x8: case s1x10:
         case s1x12: case s1x14: case s1x16:
         case s2x2: case s4x4: case s_thar: case s_qtag: case s_trngl:
            /* FELL THROUGH!!! */
            goto check_tt;
         case sdmd: case s_ptpd:
            tt.assump_col = 1;
            goto check_tt;
         case s1x4:  /* Note that 1x6, 1x8, etc should be here also.  This will
                           make "cr_2fl_only" and such things work in 4x1. */
         case s2x4:
         case s2x6:
         case s2x8:
         case s2x3:
            for (idx=0,u=0 ; idx<=setup_attrs[ss->kind].setup_limits ; idx++) u |= ss->people[idx].id1;
            if (!(u&010)) tt.assump_col = 1;
            else if (tt.assump_both && ss->kind == s2x3) goto bad;   /* We can't check 2x3 lines for right-or-left-handedness. */
            goto check_tt;
         default:
                  /* ****** Try to change this (and other things) to "goto bad". */
            goto good;                 /* We don't understand the setup -- we'd better accept it. */
      }

      check_tt:

      rr = get_restriction_thing(ss->kind, tt);
      if (!rr) goto good;                 /* We don't understand the setup -- we'd better accept it. */

      check_stuff:

      if (verify_restriction(ss, rr, tt, FALSE, &booljunk)) goto good;

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
         if ((*(predlistptr->pred->predfunc))(scopy, real_index, real_direction, northified_index, predlistptr->pred->extra_stuff)) {
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


extern void scatter(setup *resultpeople, setup *sourcepeople, Const veryshort *resultplace, int countminus1, int rotamount)
{
   int k, idx;
   for (k=0; k<=countminus1; k++) {
      idx = resultplace[k];

      if (idx < 0) {
         /* This could happen in "touch_or_rear_back". */
         if (sourcepeople->people[k].id1) fail("Don't understand this setup at all.");
      }
      else
         (void) copy_rot(resultpeople, idx, sourcepeople, k, rotamount);
   }
}


extern void gather(setup *resultpeople, setup *sourcepeople, Const veryshort *resultplace, int countminus1, int rotamount)
{
   int k, idx;
   for (k=0; k<=countminus1; k++) {
      idx = resultplace[k];

      if (idx >= 0)
         (void) copy_rot(resultpeople, k, sourcepeople, idx, rotamount);
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
   uint64 *final_concepts)
{
   parse_block *tptr = cptr;

   final_concepts->herit = 0;
   final_concepts->final = 0;

   while (tptr) {
      uint64 bit_to_set;
      uint64 bit_to_forbid;
      bit_to_set.herit = 0;
      bit_to_set.final = 0;
      bit_to_forbid.herit = 0;
      bit_to_forbid.final = 0;

      switch (tptr->concept->kind) {
         case concept_comment:
            goto get_next;               /* Need to skip these. */
         case concept_triangle: bit_to_set.final = FINAL__TRIANGLE; break;
         case concept_magic:
            last_magic_diamond = tptr;
            bit_to_set.herit = INHERITFLAG_MAGIC;
            bit_to_forbid.herit = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
            break;
         case concept_interlocked:
            last_magic_diamond = tptr;
            bit_to_set.herit = INHERITFLAG_INTLK;
            bit_to_forbid.herit = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
            break;
         case concept_grand:
            bit_to_set.herit = INHERITFLAG_GRAND;
            bit_to_forbid.herit = INHERITFLAG_SINGLE;
            break;
         case concept_cross: bit_to_set.herit = INHERITFLAG_CROSS; break;
         case concept_yoyo: bit_to_set.herit = INHERITFLAG_YOYO; break;
         case concept_straight: bit_to_set.herit = INHERITFLAG_STRAIGHT; break;
         case concept_twisted: bit_to_set.herit = INHERITFLAG_TWISTED; break;
         case concept_single:
            bit_to_set.herit = INHERITFLAG_SINGLE;
            bit_to_forbid.herit = INHERITFLAG_SINGLEFILE;
            break;
         case concept_singlefile:
            bit_to_set.herit = INHERITFLAG_SINGLEFILE;
            bit_to_forbid.herit = INHERITFLAG_SINGLE;
            break;
         case concept_1x2:
            bit_to_set.herit = INHERITFLAG_1X2;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_2x1:
            bit_to_set.herit = INHERITFLAG_2X1;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_2x2:
            bit_to_set.herit = INHERITFLAG_2X2;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_1x3:
            bit_to_set.herit = INHERITFLAG_1X3;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_3x1:
            bit_to_set.herit = INHERITFLAG_3X1;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_3x3:
            bit_to_set.herit = INHERITFLAG_3X3;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_4x4:
            bit_to_set.herit = INHERITFLAG_4X4;
            bit_to_forbid.herit = MXN_BITS;
            break;
         case concept_split:
            bit_to_set.final = FINAL__SPLIT; break;
         case concept_reverse:
            bit_to_set.herit = INHERITFLAG_REVERSE; break;
         case concept_left:
            bit_to_set.herit = INHERITFLAG_LEFT; break;
         case concept_12_matrix:
            if (check_errors && (final_concepts->herit | final_concepts->final))
               fail("Matrix modifier must appear first.");
            bit_to_set.herit = INHERITFLAG_12_MATRIX;
            break;
         case concept_16_matrix:
            if (check_errors && (final_concepts->herit | final_concepts->final))
               fail("Matrix modifier must appear first.");
            bit_to_set.herit = INHERITFLAG_16_MATRIX;
            break;
         case concept_diamond:
            bit_to_set.herit = INHERITFLAG_DIAMOND;
            bit_to_forbid.herit = INHERITFLAG_SINGLE;
            break;
         case concept_funny:
            bit_to_set.herit = INHERITFLAG_FUNNY; break;
         default:
            goto exit5;
      }

      if (check_errors) {
         if (tptr->concept->level > calling_level) warn(warn__bad_concept_level);
   
         if ((final_concepts->herit & bit_to_set.herit) || (final_concepts->final & bit_to_set.final))
            fail("Redundant call modifier.");
   
         if ((final_concepts->herit & bit_to_forbid.herit) || (final_concepts->final & bit_to_forbid.final))
            fail("Illegal combination or order of call modifiers.");
      }

      final_concepts->herit |= bit_to_set.herit;
      final_concepts->final |= bit_to_set.final;

      get_next:

      tptr = tptr->next;
   }

   exit5:

   return tptr;
}


extern parse_block *skip_one_concept(parse_block *incoming)
{
   uint64 new_final_concepts;
   parse_block *retval;

   if (incoming->concept->kind == concept_comment)
      fail("Please don't put a comment after a meta-concept.  Sorry.");

   retval = process_final_concepts(incoming, FALSE, &new_final_concepts);

   /* Find out whether the next concept (the one that will be "random" or whatever)
      is a modifier or a "real" concept. */

   if (new_final_concepts.herit | new_final_concepts.final) {
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
            continue;      /* Defer this until later; we may be able to figure something out. */
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

            if (kk != z[i].kind) {
               /* We may have a minor problem -- a 2x4 with just the ends present might want to be a qtag instead. */
               if (     ((kk == s2x4 && z[i].kind == s_qtag) || (kk == s_qtag && z[i].kind == s2x4)) &&
                        ((rr ^ z[i].rotation) & 1)) {
                  if (kk == s2x4) { kk = s_qtag ; rr ^= 1; }
                  continue;
               }
               else
                  goto lose;
            }
         }

         if (rr < 0) rr = z[i].rotation;
         /* If the setups are "trngl" or "trngl4", we allow mismatched rotation --
            the client will take care of it. */
         if (rr != z[i].rotation && z[i].kind != s_trngl && z[i].kind != s_trngl4) goto lose;
      }
   }

   if (kk == nothing) {
      if (lineflag) kk = s1x4;
      else if (miniflag) kk = s1x2;
   }

   /* Now deal with any setups that we may have deferred. */

   for (i=0; i<arity; i++) {
      if (z[i].kind == s_normal_concentric && z[i].outer.skind == nothing) {
         if (z[i].inner.skind == s2x2 && kk == s2x4) {
            /* Turn the 2x2 into a 2x4.  Need to make it have same rotation as the others;
               that is, rotation = rr.  (We know that rr has something in it by now.) */
            z[i].inner.srotation -= rr;
            canonicalize_rotation(&z[i]);
            z[i].kind = kk;
            z[i].rotation = rr;
            swap_people(&z[i], 3, 6);
            swap_people(&z[i], 2, 5);
            swap_people(&z[i], 2, 1);
            swap_people(&z[i], 1, 0);
            clear_person(&z[i], 4);
            clear_person(&z[i], 7);
            clear_person(&z[i], 0);
            clear_person(&z[i], 3);
            canonicalize_rotation(&z[i]);
         }
         else if (z[i].inner.skind == s1x4 && kk == s_qtag && z[i].inner.srotation == rr) {
            /* Turn the 1x4 into a qtag. */
            z[i].kind = kk;
            z[i].rotation = rr;
            swap_people(&z[i], 0, 6);
            swap_people(&z[i], 1, 7);
            clear_person(&z[i], 0);
            clear_person(&z[i], 1);
            clear_person(&z[i], 4);
            clear_person(&z[i], 5);
            canonicalize_rotation(&z[i]);
         }
         else
            fail("Can't do this: don't know where the phantoms went.");
      }
      else if (z[i].kind == s2x4 && kk == s_qtag) {
         /* Turn the 2x4 into a qtag. */
         if (z[i].people[1].id1 | z[i].people[2].id1 | z[i].people[5].id1 | z[i].people[6].id1) goto lose;

         z[i].kind = kk;
         z[i].rotation++;
         (void) copy_rot(&z[i], 5, &z[i], 0, 033);
         (void) copy_rot(&z[i], 0, &z[i], 3, 033);
         (void) copy_rot(&z[i], 1, &z[i], 4, 033);
         (void) copy_rot(&z[i], 4, &z[i], 7, 033);
         clear_person(&z[i], 3);
         clear_person(&z[i], 7);
         canonicalize_rotation(&z[i]);
      }
   }
  
   if (kk == nothing) return TRUE;

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
      if (kk != s_trngl && kk != s_trngl4) z[i].rotation = rr;
   }

   return FALSE;

   lose:

   fail("This is a ridiculously inconsistent shape or orientation changer!!");
   /* NOTREACHED */
}
