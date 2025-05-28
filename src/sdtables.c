/* SD -- square dance caller's helper.

    Copyright (C) 1990-2000  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following external variables:
   selector_list
   warning_strings
   comp_qtag_2x4_stuff
   exp_2x3_qtg_stuff
   exp_4x4_4x6_stuff_a
   exp_4x4_4x6_stuff_b
   exp_4x4_4dm_stuff_a
   exp_4x4_4dm_stuff_b
   exp_c1phan_4x4_stuff1
   exp_c1phan_4x4_stuff2
   * Try to do something sensible with these. *
   rear_1x2_pair
   rear_2x2_pair
   rear_bone_pair
   step_8ch_pair
   step_qtag_pair
   step_2x2h_pair
   step_2x2v_pair
   step_spindle_pair
   step_dmd_pair
   step_tgl_pair
   step_ptpd_pair
   step_qtgctr_pair
   exp_dmd_323_stuff
   exp_1x2_dmd_stuff
   exp_qtg_3x4_stuff
   exp_1x2_hrgl_stuff
   exp_dmd_hrgl_stuff
   exp_dmd_hrgl_disc_stuff
   touch_init_table1
   touch_init_table2
   touch_init_table3
   expand_init_table
   tgl3_0
   tgl3_1
   tgl4_0
   tgl4_1
   squeezethingglass
   squeezethinggal
   squeezethingqtag
   squeezething4dmd
   squeezefinalglass
   truck_to_ptpd
   truck_to_deepxwv
   press_4dmd_4x4
   press_4dmd_qtag1
   press_4dmd_qtag2
   press_qtag_4dmd1
   press_qtag_4dmd2
   acc_crosswave
   id_bit_table_2x6_pg
   id_bit_table_bigdmd_wings
   id_bit_table_bigbone_wings
   id_bit_table_bighrgl_wings
   id_bit_table_bigdhrgl_wings
   id_bit_table_3x4_offset
   id_bit_table_3x4_h
   id_bit_table_3x4_ctr6
   id_bit_table_butterfly
   id_bit_table_525_nw
   id_bit_table_525_ne
   id_bit_table_343_outr
   id_bit_table_343_innr
   id_bit_table_545_outr
   id_bit_table_545_innr
   id_bit_table_3dmd_in_out
   id_bit_table_3dmd_ctr1x6
   id_bit_table_3dmd_ctr1x4
   id_bit_table_4dmd_cc_ee
   id_bit_table_3ptpd
   id_bit_table_3x6_with_1x6
   conc_init_table
   fdhrgl
   f323
   f2x4far
   f2x4near
   f4dmdiden
   fixmumble
   fixfrotz
   fixwhuzzis
   fixgizmo
   c1tglmap1
   c1tglmap2
   dbqtglmap1
   dbqtglmap2
   qttglmap1
   qttglmap2
   bdtglmap1
   bdtglmap2
   rgtglmap1
   sel_init_table
   setup_attrs
   begin_sizes
   startinfolist
   map_p8_tgl4
   map_spndle_once_rem
   map_1x3dmd_once_rem
   map_lh_zzztgl
   map_rh_zzztgl
   map_2x2v
   map_2x4_magic
   map_qtg_magic
   map_qtg_intlk
   map_qtg_magic_intlk
   map_ptp_magic
   map_ptp_intlk
   map_ptp_magic_intlk
   map_2x4_diagonal
   map_2x4_int_pgram
   map_2x4_trapezoid
   map_4x4_ns
   map_4x4_ew
   map_vsplit_f
   map_stairst
   map_ladder
   map_crazy_offset1
   map_crazy_offset2
   map_but_o
   map_4x4v
   map_blocks
   map_trglbox
   map_trglbox12a
   map_trglbox12b
   map_2x3_0134
   map_2x3_1245
   map_1x8_1x6
   map_rig_1x6
   map_4x6_2x4
   map_ov_hrg_1
   map_ov_gal_1
   map_3o_qtag_1
   map_tgl4_1
   map_tgl4_2
   map_qtag_2x3
   map_2x3_rmvr
   map_2x3_rmvs
   map_dbloff1
   map_dbloff2
   map_dhrgl1
   map_dhrgl2
   map_dbgbn1
   map_dbgbn2
   map_off1x81
   map_off1x82
   map_dqtag1
   map_dqtag2
   map_dqtag3
   map_dqtag4
   clw3_table
   map_trngl_box1
   map_trngl_box2
   map_inner_box
   map_lh_c1phana
   map_lh_c1phanb
   map_rh_c1phana
   map_rh_c1phanb
   map_lh_s2x3_3
   map_lh_s2x3_2
   map_rh_s2x3_3
   map_rh_s2x3_2
   map_lh_s2x3_7
   map_rh_s2x3_7
   map_d1x10
   map_lz12
   map_rz12
   map_tgl451
   map_tgl452
   map_dmd_1x1
   map_star_1x1
   map_qtag_f0
   map_qtag_f1
   map_qtag_f2
   map_diag2a
   map_diag2b
   map_diag23a
   map_diag23b
   map_diag23c
   map_diag23d
   map_f2x8_4x4
   map_w4x4_4x4
   map_f2x8_2x8
   map_w4x4_2x8
   map_emergency1
   map_emergency2
   maps_3diag
   maps_3diagwk
   map_init_table2
   split_lists
*/

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"


// BEWARE!!  This list is keyed to the definition of "selector_kind" in basetype.h .
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
   {"end boys",     "end boy",     "END BOYS",     "END BOY",     selector_uninitialized},
   {"end girls",    "end girl",    "END GIRLS",    "END GIRL",    selector_uninitialized},
   {"center boys",  "center boy",  "CENTER BOYS",  "CENTER BOY",  selector_uninitialized},
   {"center girls", "center girl", "CENTER GIRLS", "CENTER GIRL", selector_uninitialized},
   {"beaus",        "beau",        "BEAUS",        "BEAU",        selector_belles},
   {"belles",       "belle",       "BELLES",       "BELLE",       selector_beaus},
   {"center 2",     "center 2",    "CENTER 2",     "CENTER 2",    selector_outer6},
   {"very centers", "very center", "VERY CENTERS", "VERY CENTER", selector_outer6},
   {"center 6",     "center 6",    "CENTER 6",     "CENTER 6",    selector_outer2},
   {"outer 2",      "outer 2",     "OUTER 2",      "OUTER 2",     selector_center6},
   {"very ends",    "very end",    "VERY ENDS",    "VERY END",    selector_center6},
   {"outer 6",      "outer 6",     "OUTER 6",      "OUTER 6",     selector_center2},
   {"center diamond", "center diamond", "CENTER DIAMOND", "CENTER DIAMOND",    selector_uninitialized},
   {"center 1x4",   "center 1x4",  "CENTER 1X4",   "CENTER 1X4",  selector_uninitialized},
   {"center 1x6",   "center 1x6",  "CENTER 1X6",   "CENTER 1X6",  selector_uninitialized},
   {"outer 1x3s",   "outer 1x3s",  "OUTER 1X3s",   "OUTER 1X3s",  selector_uninitialized},
   {"center 4",     "center 4",    "CENTER 4",     "CENTER 4",    selector_outerpairs},
   {"outer pairs",  "outer pair",  "OUTER PAIRS",  "OUTER PAIR",  selector_center4},
#ifdef TGL_SELECTORS
   /* Taken out.  Not convinced these are right.  See also sdutil.c, sdpreds.c . */
   {"wave-based triangles",   "wave-based triangle",   "WAVE-BASED TRIANGLES",   "WAVE-BASED TRIANGLE",   selector_uninitialized},
   {"tandem-based triangles", "tandem-based triangle", "TANDEM-BASED TRIANGLES", "TANDEM-BASED TRIANGLE", selector_uninitialized},
   {"inside triangles",       "inside triangle",       "INSIDE TRIANGLES",       "INSIDE TRIANGLE",       selector_uninitialized},
   {"outside triangles",      "outside triangle",      "OUTSIDE TRIANGLES",      "OUTSIDE TRIANGLE",      selector_uninitialized},
   {"in point triangles",     "in point triangle",     "IN POINT TRIANGLES",     "IN POINT TRIANGLE",     selector_uninitialized},
   {"out point triangles",    "out point triangle",    "OUT POINT TRIANGLES",    "OUT POINT TRIANGLE",    selector_uninitialized},
#endif
   {"headliners",   "headliner",   "HEADLINERS",   "HEADLINER",   selector_sideliners},
   {"sideliners",   "sideliner",   "SIDELINERS",   "SIDELINER",   selector_headliners},
   {"those facing", "those facing","THOSE FACING", "THOSE FACING",selector_uninitialized},
   {"everyone",     "everyone",    "EVERYONE",     "EVERYONE",    selector_uninitialized},
   {"all",          "all",         "ALL",          "ALL",         selector_uninitialized},
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

/* BEWARE!!  These strings are keyed to the definition of "warning_index" in sdui.h . */
/* A "*" as the first character means that this warning precludes acceptance while searching. */
/* A "+" as the first character means that this warning is cleared if a
   concentric call was done and the "suppress_elongation_warnings" flag was on. */
/* A "=" as the first character means that this warning is cleared if it arises
   during some kind of "do your part" call. */
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
   /*  warn__1_4_pgram           */   " 25% offset parallelogram.",
   /*  warn__full_pgram          */   " Completely offset parallelogram.",
   /*  warn__3_4_pgram           */   " 75% offset parallelogram.",
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
   /*  warn__check_quad_dmds     */   " Fudge to quadruple diamonds.",
   /*  warn__check_3x4           */   " Check a 3x4 setup.",
   /*  warn__check_2x4           */   " Check a 2x4 setup.",
   /*  warn__check_hokey_2x4     */   "*Fudge to a 2x4 -- the dancers might not like this.",
   /*  warn__check_4x4           */   "*Check a 4x4 setup.",
   /*  warn__check_hokey_4x4     */   "*Check a center box and outer lines/columns.",
   /*  warn__check_4x4_start     */   "*Check a 4x4 setup at the start of this call.",
   /*  warn__check_centered_qtag */   "*Each 8-person twin 1/4 tag is centered, with the outsides directly adjacent.",
   /*  warn__check_pgram         */   " Opt for a parallelogram.",
   /*  warn__ctrs_stay_in_ctr    */   " Centers stay in the center.",
   /*  warn__check_c1_stars      */   " Check 'stars'.",
   /*  warn__check_gen_c1_stars  */   " Check a generalized 'star' setup.",
   /*  warn__bigblock_feet       */   " Bigblock/stagger shapechanger -- go to footprints.",
   /*  warn__bigblockqtag_feet   */   " Adjust to bigblock/stagger 1/4 tag footprints.",
   /*  warn__diagqtag_feet       */   " Adjust to diagonal 1/4 tag footprints on other diagonal.",
   /*  warn__adjust_to_feet      */   " Adjust back to footprints.",
   /*  warn__some_touch          */   " Some people step to a wave.",
   /*  warn__split_to_2x4s       */   "=Do the call in each 2x4.",
   /*  warn__split_to_2x3s       */   "=Do the call in each 2x3.",
   /*  warn__split_to_1x8s       */   "=Do the call in each 1x8.",
   /*  warn__split_to_1x6s       */   "=Do the call in each 1x6.",
   /*  warn__take_left_hands     */   " Take left hands, since this call is being done mirror.",
   /*  warn__evil_interlocked    */   " Interlocked phantom shape-changers are very evil.",
   /*  warn__split_phan_in_pgram */   " The split phantom setups are directly adjacent to the real people.",
   /*  warn__bad_interlace_match */   "*The interlaced calls have mismatched lengths.",
   /*  warn__not_on_block_spots  */   " Generalized bigblock/stagger -- people are not on block spots.",
   /*  warn__stupid_phantom_clw  */   "#This use of phantom setups seems superfluous.",
   /*  warn__bad_modifier_level  */   "*Use of this modifier on this call is not allowed at this level.",
   /*  warn__bad_call_level      */   "*This call is not really legal at this level.",
   /*  warn__did_not_interact    */   "*The setups did not interact with each other.",
   /*  warn__opt_for_normal_cast */   "*If in doubt, assume a normal cast.",
   /*  warn__opt_for_normal_hinge*/   "*If in doubt, assume a normal hinge.",
   /*  warn__opt_for_2fl         */   "*If in doubt, assume a two-faced line.",
   /*  warn__opt_for_no_collision*/   "*If in doubt, assume no collision.",
   /*  warn_partial_solomon      */   "*For the center line or column, the offset goes away.",
   /*  warn_same_z_shear         */   "*Make the outside Z's have the same shear as the center one.",
   /*  warn__like_linear_action  */   "*Ends start like a linear action -- this may be controversial.",
   /*  warn__no_z_action         */   "*The 'Z' concept was not actually used.",
   /*  warn__phantoms_thinner    */   "*Phantoms may have gotten thinner -- go to outer triple boxes.",
   /*  warn__hokey_jay_shapechanger */"*This shapechanger in a jay may be controversial.",
   /*  warn__split_1x6           */   "=Do the call in each 1x3 setup.",
   /*  warn_interlocked_to_6     */   "*This went from 4 interlocked groups to 6.",
   /*  warn__offset_hard_to_see  */   "*The offset setup is hard to see.",
   /*  warn__pg_hard_to_see      */   "*The parallelogram setup is hard to see.",
   /*  warn__colocated_once_rem  */   " The once-removed setups have the same center.",
   /*  warn_big_outer_triangles  */   "*The outside triangles are very large.",
   /*  warn_hairy_fraction       */   " Fraction is very complicated.",
   /*  warn_bad_collision        */   "*This collision may be controversial.",
   /*  warn__dyp_resolve_ok      */   " Do your part.",
   /*  warn__unusual             */   "*This is an unusual setup for this call.",
   /*  warn_controversial        */   "*This may be controversial.",
   /*  warn_serious_violation    */   "*This appears to be a serious violation of the definition.",
   /*  warn_bogus_yoyo_rims_hubs */   "*Using incorrect definition of rims/hubs trade.",
   /*  warn_pg_in_2x6            */   "*Offset the resulting 2x6 by 50%, or 3 positions.",
   /*  warn_real_people_spots    */   "*The distorted setup is formed by the real people.",
   /*  warn__tasteless_com_spot  */   "*Not all common-spot people had right hands.",
   /*  warn__tasteless_junk      */   "*The algorithmic nondeterminism of this usage is truly extraordinary.",
   /*  warn__tasteless_slide_thru*/   "*Slide thru from left-handed miniwave may be controversial."};



/* comp_qtag_2x4_stuff is duplicated in the big table. */
        expand_thing comp_qtag_2x4_stuff   = {
           {5, -1, -1, 0, 1, -1, -1, 4}, 8, s2x4, s_qtag, 1};
/* exp_2x3_qtg_stuff is duplicated in the big table. */
        expand_thing exp_2x3_qtg_stuff     = {
           {5, 7, 0, 1, 3, 4}, 6, s2x3, s_qtag, 1};
        expand_thing exp_4x4_4x6_stuff_a   = {
           {4, 7, 22, 8, 13, 14, 15, 21, 16, 19, 10, 20, 1, 2, 3, 9}, 16, s4x4, s4x6, 0};
        expand_thing exp_4x4_4x6_stuff_b   = {
           {1, 2, 3, 9, 4, 7, 22, 8, 13, 14, 15, 21, 16, 19, 10, 20}, 16, s4x4, s4x6, 1};
        expand_thing exp_4x4_4dm_stuff_a   = {
           {0, 1, 2, -1, 3, -1, -1, -1, 8, 9, 10, -1, 11, -1, -1, -1}, 16, nothing, s4dmd, 1};
        expand_thing exp_4x4_4dm_stuff_b   = {
           {3, -1, -1, -1, 8, 9, 10, -1, 11, -1, -1, -1, 0, 1, 2, -1}, 16, nothing, s4dmd, 0};
        expand_thing exp_c1phan_4x4_stuff1 = {
           {-1, 13, -1, 15, -1, 1, -1, 3, -1, 5, -1, 7, -1, 9, -1, 11}, 16, s_c1phan, s4x4, 0};
        expand_thing exp_c1phan_4x4_stuff2 = {
           {10, -1, 15, -1, 14, -1, 3, -1, 2, -1, 7, -1, 6, -1, 11, -1}, 16, s_c1phan, s4x4, 0};
       expand_thing exp_dmd_323_stuff     = {{5, 7, 1, 3}, 4, sdmd, s_323, 1};
/* exp_1x2_dmd_stuff is duplicated in the big table. */
       expand_thing exp_1x2_dmd_stuff     = {{3, 1}, 2, s1x2, sdmd, 1};
/* exp_qtg_3x4_stuff is duplicated in the big table. */
       expand_thing exp_qtg_3x4_stuff     = {{1, 2, 4, 5, 7, 8, 10, 11}, 8, s_qtag, s3x4, 0};
       expand_thing exp_1x2_hrgl_stuff     = {{7, 3}, 2, s1x2, s_hrglass, 1};
       expand_thing exp_dmd_hrgl_stuff     = {{6, 3, 2, 7}, 4, sdmd, s_hrglass, 0};
       expand_thing exp_dmd_hrgl_disc_stuff = {{6, -1, 3, 2, -1, 7}, 6, s_1x2dmd, s_hrglass, 0};

static expand_thing rear_thar_stuff = {{9, 10, 13, 14, 1, 2, 5, 6}, 8, s_thar, s4x4, 0};
static expand_thing rear_ohh_stuff = {{-1, 5, 4, -1, -1, 7, 6, -1, -1, 1, 0, -1, -1, 3, 2, -1}, 16, nothing, s_thar, 0};
static expand_thing rear_bigd_stuff1 = {{-1, -1, 10, 11, 1, 0, -1, -1, 4, 5, 7, 6}, 12, nothing, s3x4, 1};
static expand_thing rear_bigd_stuff2 = {{8, 9, 10, 11, -1, -1, 2, 3, 4, 5, -1, -1}, 12, nothing, s3x4, 1};
static expand_thing rear_bone_stuffa = {{0, 5, 7, 6, 4, 1, 3, 2}, 8, s1x8, s_bone, 0};
static expand_thing rear_bone_stuffb = {{0, 3, 2, 5, 4, 7, 6, 1}, 8, s_bone, s2x4, 0};
static expand_thing rear_bone_stuffc = {{6, 3, 1, 4, 2, 7, 5, 0}, 8, s_bone, s_rigger, 0};
static expand_thing rear_rig_stuffa = {{1, 2, 3, 4, 5, 6, 7, 0}, 8, s_rigger, s2x4, 0};
static expand_thing rear_rig_stuffb = {{3, 6, 4, 5, 7, 2, 0, 1}, 8, s_rigger, s1x8, 0};
static expand_thing rear_rig_stuffc = {{6, 3, 1, 4, 2, 7, 5, 0}, 8, s_rigger, s_bone, 0};
static expand_thing rear_funnydmd   = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, s_qtag, s2x4, 1};
static expand_thing rear_tgl4a_stuff = {{2, 3, 0, 1}, 4, nothing, s2x2, 0};
static expand_thing rear_tgl4b_stuff = {{2, 3, 1, 0}, 4, nothing, s1x4, 1};

static expand_thing rear_c1a_stuff = {{0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1, 6, -1, 7, -1}, 16, s_c1phan, s2x4, 0};
static expand_thing rear_44a_stuff = {{-1, -1, 4, 3, -1, -1, 6, 5, -1, -1, 0, 7, -1, -1, 2, 1}, 16, s4x4, s2x4, 0};
static expand_thing rear_c1b_stuff = {{-1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6}, 16, s_c1phan, s2x4, 0};
static expand_thing rear_44b_stuff = {{-1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6, -1, 0, -1, 1}, 16, s4x4, s2x4, 0};
static expand_thing rear_c1c_stuff = {{6, -1, 7, -1, 0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, -1}, 16, s_c1phan, s2x4, 1};
static expand_thing rear_44c_stuff = {{-1, -1, 2, 1, -1, -1, 4, 3, -1, -1, 6, 5, -1, -1, 0, 7}, 16, s4x4, s2x4, 1};
static expand_thing rear_c1d_stuff = {{-1, 7, -1, 6, -1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5}, 16, s_c1phan, s2x4, 1};
static expand_thing rear_44d_stuff = {{-1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6}, 16, s4x4, s2x4, 1};

static expand_thing rear_c1e_stuff = {{3, -1, 1, -1, 7, -1, 5, -1, 11, -1, 9, -1, 15, -1, 13, -1}, 16, s_c1phan, s_c1phan, 0};
static expand_thing rear_c1f_stuff = {{-1, 0, -1, 2, -1, 4, -1, 6, -1, 8, -1, 10, -1, 12, -1, 14}, 16, s_c1phan, s_c1phan, 0};

static expand_thing rear_vrbox_stuff = {{1, 0, 3, 2}, 4, nothing, s1x4, 1};
static expand_thing rear_hrbox_stuff = {{0, 3, 2, 1}, 4, nothing, s1x4, 0};
static expand_thing rear_qtag_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, nothing, s2x4, 1};
static expand_thing rear_ptpd_stuff = {{0, 1, 2, 3, 4, 5, 6, 7}, 8, nothing, s1x8, 0};
static expand_thing rear_sqtag_stuff = {{0, 1, 2, 3}, 4, nothing, s1x4, 0};
static expand_thing rear_twistqtag_stuff = {{0, 3, 2, 1}, 4, nothing, sdmd, 0};
static expand_thing rear_twist2x4c_stuff = {{5, 7, 6, 0, 1, 3, 2, 4}, 8, nothing, s_qtag, 1};
static expand_thing rear_3x1qtag_stuff = {{4, 5, 3, 6, 0, 1, 7, 2}, 8, nothing, s_qtag, 1};
static expand_thing rear_1x3qtag_stuff = {{9, 10, 0, 11, 3, 4, 6, 5}, 8, nothing, s3x4, 1};

static expand_thing step_8by_stuff = {{6, 7, 0, 1, 2, 3, 4, 5}, 8, s_thar, s_qtag, 0};
static expand_thing step_sqs_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, s_thar, s2x4, 0};
static expand_thing step_1x8_stuff = {{0, 7, 6, 1, 4, 3, 2, 5}, 8, s1x8, s2x4, 0};
static expand_thing step_qbox_stuff = {{0, 3, 5, 2, 4, 7, 1, 6}, 8, s_bone, s2x4, 0};
static expand_thing rear_3n1a_stuff = {{3, 1, 7, 5, 11, 9, 15, 13}, 8, s2x4, s_c1phan, 0};
static expand_thing rear_3n1b_stuff = {{0, 2, 6, 4, 8, 10, 14, 12}, 8, s2x4, s_c1phan, 0};
static expand_thing rear_3n1c_stuff = {{6, 0, 1, 3, 2, 4, 5, 7}, 8, s2x4, s_rigger, 0};

static expand_thing step_1x4_side_stuff = {{0, 1, 2, 3}, 4, nothing, sdmd, 0};
static expand_thing step_1x4_stuff = {{0, 3, 2, 1}, 4, nothing, s2x2, 0};
static expand_thing step_1x2_stuff = {{0, 1}, 2, s1x2, s1x2, 1};
static expand_thing step_offs1_stuff = {{-1, -1, 0, 1, 3, 2, -1, -1, 6, 7, 9, 8}, 12, s3x4, s2x6, 1};
static expand_thing step_offs2_stuff = {{11, 10, -1, -1, 3, 2, 5, 4, -1, -1, 9, 8}, 12, s3x4, s2x6, 1};
static expand_thing step_2x2v_stuff = {{1, 2, 3, 0}, 4, s2x2, s1x4, 0};
static expand_thing step_2x2h_stuff = {{0, 1, 2, 3}, 4, nothing, s1x4, 1};
static expand_thing step_8ch_stuff = {{7, 6, 0, 1, 3, 2, 4, 5}, 8, s2x4, s2x4, 1};
static expand_thing step_li_stuff = {{1, 2, 7, 4, 5, 6, 3, 0}, 8, s2x4, s1x8, 0};
static expand_thing step_li6_stuff = {{1, 5, 3, 4, 2, 0}, 6, s2x3, s1x6, 0};
static expand_thing step_spindle_stuff = {{3, 6, 5, 4, 7, 2, 1, 0}, 8, s_spindle, s1x8, 0};
static expand_thing step_bn_stuff = {{0, 7, 2, 1, 4, 3, 6, 5}, 8, nothing, s_bone, 0};
static expand_thing step_bn23_stuff = {{0, 2, 1, 3, 5, 4}, 6, nothing, s_bone6, 0};
static expand_thing step_24bn_stuff = {{0, 3, 5, 2, 4, 7, 1, 6}, 8, nothing, s2x4, 0};
static expand_thing step_23bn_stuff = {{0, 2, 4, 3, 5, 1}, 6, nothing, s2x3, 0};
static expand_thing step_tby_stuff = {{5, 6, 7, 0, 1, 2, 3, 4}, 8, nothing, s_qtag, 1};
static expand_thing step_2x4_rig_stuff = {{7, 0, 1, 2, 3, 4, 5, 6}, 8, nothing, s_rigger, 0};
static expand_thing step_bone_stuff = {{1, 4, 7, 6, 5, 0, 3, 2}, 8, s_bone, s1x8, 0};
static expand_thing step_bone_rigstuff = {{7, 2, 4, 1, 3, 6, 0, 5}, 8, s_bone, s_rigger, 0};
static expand_thing step_rig_stuff = {{2, 7, 4, 5, 6, 3, 0, 1}, 8, nothing, s1x8, 0};

static expand_thing step_phan1_stuff = {{-1, 7, -1, 6, -1, 1, -1, 0, -1, 3, -1, 2, -1, 5, -1, 4},
                                        16, nothing, s2x4, 1};
static expand_thing step_phan2_stuff = {{7, -1, 6, -1, 0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1},
                                        16, nothing, s2x4, 1};
static expand_thing step_phan3_stuff = {{0, -1, 1, -1, 3, -1, 2, -1, 4, -1, 5, -1, 7, -1, 6, -1},
                                        16, nothing, s2x4, 0};
static expand_thing step_phan4_stuff = {{-1, 1, -1, 0, -1, 3, -1, 2, -1, 5, -1, 4, -1, 7, -1, 6},
                                        16, nothing, s2x4, 0};
static expand_thing step_bigd_stuff1 = {{0, 1, 3, 2, -1, -1, 6, 7, 9, 8, -1, -1},
                                        12, nothing, s2x6, 0};
static expand_thing step_bigd_stuff2 = {{-1, -1, 3, 2, 4, 5, -1, -1, 9, 8, 10, 11},
                                        12, nothing, s2x6, 0};
static expand_thing step_tgl4_stuffa = {{2, 3, 0, 1}, 4, nothing, s1x4, 1};
static expand_thing step_tgl4_stuffb = {{3, 2, 0, 1}, 4, nothing, s2x2, 0};
static expand_thing step_dmd_stuff   = {{0, 3, 2, 1}, 4, nothing, s1x4, 0};
static expand_thing step_tgl_stuff   = {{2, 0, 1}, 3, nothing, s1x3, 1};
static expand_thing step_ptpd_stuff  = {{0, 3, 2, 1, 4, 7, 6, 5}, 8, nothing, s1x8, 0};
static expand_thing step_qtgctr_stuff = {{7, 0, 2, 1, 3, 4, 6, 5}, 8, nothing, s2x4, 1};

full_expand_thing rear_1x2_pair      = {warn__rear_back,  8, &step_1x2_stuff};
full_expand_thing rear_2x2_pair      = {warn__rear_back,  8, &step_2x2v_stuff};
full_expand_thing rear_bone_pair     = {warn__some_rear_back, 0, &rear_bone_stuffb};
full_expand_thing step_8ch_pair      = {warn__none,       0, &step_8ch_stuff};
full_expand_thing step_qtag_pair     = {warn__none,       0, &step_tby_stuff};
full_expand_thing step_2x2h_pair     = {warn__none,    16+1, &step_2x2h_stuff};
full_expand_thing step_2x2v_pair     = {warn__none,    16+2, &step_2x2v_stuff};
full_expand_thing step_spindle_pair  = {warn__some_touch, 0, &step_spindle_stuff};
full_expand_thing step_dmd_pair      = {warn__some_touch, 0, &step_dmd_stuff};
full_expand_thing step_tgl_pair      = {warn__some_touch, 0, &step_tgl_stuff};
full_expand_thing step_ptpd_pair     = {warn__some_touch, 0, &step_ptpd_stuff};

full_expand_thing step_qtgctr_pair   = {warn__some_touch, 0, &step_qtgctr_stuff};

full_expand_thing touch_init_table1[] = {
   {warn__rear_back,       8, &step_1x2_stuff,   s1x2,         0xFUL,        0x2UL, ~0UL},      /* Rear back from a miniwave to facing people. */
   {warn__some_rear_back,  0, &rear_tgl4a_stuff, s_trngl4,    0xFFUL,       0xDAUL, ~0UL},      /* Rear back from a 4-person triangle to a "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_tgl4a_stuff, s_trngl4,    0xFFUL,       0xD2UL, ~0UL},      /* Two similar ones with miniwave base for funny square thru. */
   {warn__some_rear_back,  0, &rear_tgl4a_stuff, s_trngl4,    0xFFUL,       0xD8UL, ~0UL},         /* (The base couldn't want to rear back -- result would be stupid.) */
   {warn__awful_rear_back, 0, &rear_tgl4b_stuff, s_trngl4,    0xFFUL,       0x22UL, ~0UL},      /* Rear back from a 4-person triangle to a single 8 chain. */
   {warn__rear_back,       8, &step_li_stuff,    s1x8,      0xFFFFUL,     0x2882UL, ~0UL},      /* Rear back from a grand wave to facing lines. */
   {warn__some_rear_back,  8, &rear_bone_stuffa, s_bone,    0xFFFFUL,     0x55F5UL, 0xF5F5UL},  /* Ends rear back from a "bone" to grand 8-chain or whatever. */
   {warn__some_rear_back,  0, &rear_bone_stuffb, s_bone,    0xFFFFUL,     0x0802UL, 0x0F0FUL},  /* Centers rear back from a "bone" to lines facing or "split square thru" setup. */
   {warn__rear_back,       0, &rear_bone_stuffc, s_bone,    0xFFFFUL,     0x58F2UL, 0xFFFFUL},  /* All rear back from a "bone" to a "rigger". */
   {warn__rear_back,       0, &rear_ohh_stuff, s4x4,    0x3C3C3C3CUL, 0x1C203408UL, ~0UL},      /* Rear back from an alamo wave to crossed single 8-chains. */
   {warn__rear_back,       8, &step_8ch_stuff, s2x4,        0xFFFFUL,     0x2288UL, ~0UL},      /* Rear back from parallel waves to an 8 chain. */
   {warn__awful_rear_back, 8, &step_1x8_stuff, s2x4,        0xFFFFUL,     0x55FFUL, ~0UL},      /* Rear back from columns to end-to-end single 8-chains. */
   {warn__some_rear_back,  8, &step_qbox_stuff, s2x4,       0xFFFFUL,     0x57FDUL, ~0UL},      /* Centers rear back from 1/4-box to triangles. */

   // Some people rear back from 3&1 column to rigger.
   {warn__some_rear_back,  0, &rear_3n1c_stuff, s2x4,       0xFFFFUL,     0x5DF7UL, ~0UL},

   // Some people rear back from 3&1 line to triangles.
   {warn__some_rear_back,  0, &rear_3n1a_stuff, s2x4,       0xFFFFUL,     0x2A80UL, ~0UL},
   // Some people rear back from 3&1 line to triangles.
   {warn__some_rear_back,  0, &rear_3n1b_stuff, s2x4,       0xFFFFUL,     0xA208UL, ~0UL},
   /* Rear back from a right-hand box to a single 8 chain. */
   {warn__awful_rear_back, 0, &rear_vrbox_stuff, s2x2,        0xFFUL,       0x28UL, ~0UL},
   {warn__awful_rear_back, 0, &rear_hrbox_stuff, s2x2,        0xFFUL,       0x5FUL, ~0UL},

   /* Centers rear back from appropriate "diamonds" to T-boned pairs facing. */
   {warn__some_rear_back,  0, &rear_funnydmd,  s_qtag,    0xFFFFUL,     0x78D2UL, ~0UL},

   /* Ends rear back from a "rigger" to lines facing or "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_rig_stuffa,s_rigger,    0xFFFFUL,     0x0802UL, 0x0F0FUL},
   /* Centers rear back from a "rigger" to grand 8-chain or whatever. */
   {warn__some_rear_back,  0, &rear_rig_stuffb,s_rigger,    0xFFFFUL,     0x55F5UL, 0xF5F5UL},
   /* All rear back from a "rigger" to a "bone". */
   {warn__rear_back,       0, &rear_rig_stuffc,s_rigger,    0xFFFFUL,     0x58F2UL, 0xFFFFUL},
   /* Some people rear back from horrible "T"'s to couples facing or "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_bigd_stuff1,sbigdmd,  0x0FF0FFUL,   0x0520F8UL, ~0UL},
   {warn__some_rear_back,  0, &rear_bigd_stuff1,sbigdmd,  0x0FF0FFUL,   0x082028UL, ~0UL},
   {warn__some_rear_back,  0, &rear_bigd_stuff2,sbigdmd,  0xFF0FF0UL,   0x2F0850UL, ~0UL},      /* Some people rear back from horrible "T"'s to couples facing or "split square thru" setup. */
   {warn__some_rear_back,  0, &rear_bigd_stuff2,sbigdmd,  0xFF0FF0UL,   0x280820UL, ~0UL},
   {warn__rear_back,       0, &rear_thar_stuff, s_thar,     0xFFFFUL,     0x278DUL, ~0UL},      /* Rear back from thar to alamo 8-chain. */

   {warn__some_rear_back,  0, &rear_c1a_stuff,s_c1phan, 0xCCCCCCCCUL, 0x884C00C4UL, ~0UL},      /* Check for certain people rearing back from C1 phantoms. */
   {warn__some_rear_back,  0, &rear_c1a_stuff,s_c1phan, 0xCCCCCCCCUL, 0x4C4CC4C4UL, ~0UL},
   {warn__some_rear_back,  0, &rear_44a_stuff, s4x4,    0x0F0F0F0FUL, 0x030C0906UL, ~0UL},      /* Or from equivalent pinwheel. */
   {warn__some_rear_back,  0, &rear_44a_stuff, s4x4,    0x0F0F0F0FUL, 0x0F0D0507UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1b_stuff,s_c1phan, 0x33333333UL, 0x13223100UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1b_stuff,s_c1phan, 0x33333333UL, 0x13313113UL, ~0UL},
   {warn__some_rear_back,  0, &rear_44b_stuff, s4x4,    0x33333333UL, 0x22310013UL, ~0UL},
   {warn__some_rear_back,  0, &rear_44b_stuff, s4x4,    0x33333333UL, 0x31311313UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1c_stuff,s_c1phan, 0xCCCCCCCCUL, 0x08CC8044UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1c_stuff,s_c1phan, 0xCCCCCCCCUL, 0x08808008UL, ~0UL},
   {warn__some_rear_back,  0, &rear_44c_stuff, s4x4,    0x0F0F0F0FUL, 0x0B04010EUL, ~0UL},
   {warn__some_rear_back,  0, &rear_44c_stuff, s4x4,    0x0F0F0F0FUL, 0x0800020AUL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1d_stuff,s_c1phan, 0x33333333UL, 0x11203302UL, ~0UL},
   {warn__some_rear_back,  0, &rear_c1d_stuff,s_c1phan, 0x33333333UL, 0x20200202UL, ~0UL},
   {warn__some_rear_back,  0, &rear_44d_stuff, s4x4,    0x33333333UL, 0x20330211UL, ~0UL},
   {warn__some_rear_back,  0, &rear_44d_stuff, s4x4,    0x33333333UL, 0x20020220UL, ~0UL},

   {warn__rear_back,       0, &rear_c1e_stuff,s_c1phan, 0xCCCCCCCCUL, 0x084C80C4UL, ~0UL},
   {warn__rear_back,       0, &rear_c1f_stuff,s_c1phan, 0x33333333UL, 0x13203102UL, ~0UL},
   {warn__rear_back,     4+8, &step_2x2v_stuff, s1x4,         0xFFUL,       0x28UL, ~0UL},      /* Rear back from a wave to facing couples. */
   {warn__none,            0, (expand_thing *) 0, nothing}
};

full_expand_thing touch_init_table2[] = {
   /* Have the centers rear back from a 1/4 tag or 3/4 tag. */
   {warn__rear_back,       0, &rear_qtag_stuff, s_qtag,     0xFFFFUL,     0x08A2UL, ~0UL},
   {warn__rear_back,       0, &rear_qtag_stuff, s_qtag,     0xFFFFUL,     0xA802UL, ~0UL},
   /* Have the centers rear back from point-to-point 1/4 tags or 3/4 tags. */
   {warn__rear_back,       0, &rear_ptpd_stuff, s_ptpd,     0xFFFFUL,     0x5FF5UL, ~0UL},
   {warn__rear_back,       0, &rear_ptpd_stuff, s_ptpd,     0xFFFFUL,     0xD77DUL, ~0UL},
   /* Have the centers rear back from a single 1/4 tag or 3/4 tag. */
   {warn__awful_rear_back, 0, &rear_sqtag_stuff, sdmd,        0xFFUL,       0x5FUL, ~0UL},
   {warn__awful_rear_back, 0, &rear_sqtag_stuff, sdmd,        0xFFUL,       0xD7UL, ~0UL},
   /* As above, but centers are "twisted". */
   {warn__awful_rear_back, 0, &rear_twistqtag_stuff, s1x4,    0xFFUL,       0x4EUL, ~0UL},
   {warn__awful_rear_back, 0, &rear_twist2x4c_stuff, s2x4,  0xFFFFUL,     0x0820UL, 0x3C3CUL},
   {warn__rear_back,       0, &rear_3x1qtag_stuff, s3x1dmd, 0xFFFFUL,     0x2088UL, 0xFCFCUL},
   {warn__rear_back,       0, &rear_1x3qtag_stuff,   s_323, 0xFFFFUL,     0x0200UL, 0x0303UL},

   {warn__none,            0, (expand_thing *) 0, nothing}
};

full_expand_thing touch_init_table3[] = {
   {warn__some_touch, 0, &step_phan1_stuff,   s_c1phan, 0x33333333UL, 0x13313113UL, ~0UL},
   {warn__some_touch, 0, &step_phan2_stuff,   s_c1phan, 0xCCCCCCCCUL, 0x4C4CC4C4UL, ~0UL},
   {warn__some_touch, 0, &step_phan3_stuff,   s_c1phan, 0xCCCCCCCCUL, 0x08808008UL, ~0UL},
   {warn__some_touch, 0, &step_phan4_stuff,   s_c1phan, 0x33333333UL, 0x20200202UL, ~0UL},

   /* Some people touch from horrible "T"'s. */
   {warn__some_touch, 0, &step_bigd_stuff1,   sbigdmd,    0xFF0FF0UL,   0x280820UL, ~0UL},

   {warn__some_touch, 0, &step_bigd_stuff2,   sbigdmd,    0x0FF0FFUL,   0x082028UL, ~0UL},

   /* Check for stepping to a grand wave from lines facing. */
   {warn__none,      16, &step_li_stuff,      s2x4,         0xFFFFUL,     0xAA00UL, ~0UL},
   {warn__none,      16, &step_li6_stuff,     s2x3,          0xFFFUL,      0xA80UL, ~0UL},
   {warn__none,      16, &step_spindle_stuff, s_spindle,    0xFFFFUL,     0xA802UL, ~0UL},

   /* Same, with missing people. */
   {warn__none,      16, &step_li_stuff,      s2x4,         0xC3C3UL,     0x8200UL, ~0UL},
   {warn__none,      16, &step_li_stuff,      s2x4,         0x3C3CUL,     0x2800UL, ~0UL},

   /* Check for stepping to a bone from a squared set or whatever. */
   {warn__none,      16, &step_bn_stuff,      s2x4,         0xFFFFUL,     0x6941UL, 0x7D7DUL},

   /* Check for stepping to a bone6 from a 2x3. */
   {warn__none,       0, &step_bn23_stuff,    s2x3,          07777UL,      03121UL,  03535UL},

   /* Check for centers stepping to a column from a bone. */
   {warn__none,       0, &step_24bn_stuff,    s_bone,       0xFFFFUL,     0x5D57UL, 0x5F5FUL},

   /* Check for centers stepping to a column of 6 from a bone6. */
   {warn__none,       0, &step_23bn_stuff,    s_bone6,       07777UL,      02725UL,  02727UL},

   /* Check for stepping to rigger from suitable T-bone. */
   {warn__some_touch,16, &step_2x4_rig_stuff, s2x4,       0xFFFFUL,     0x963CUL, ~0UL},
   {warn__none,       0, &step_offs1_stuff,   s3x4,     0x0FF0FFUL,   0x07D0D7UL, ~0UL},
   {warn__none,       0, &step_offs2_stuff,   s3x4,     0xF0FF0FUL,   0x70DD07UL, ~0UL},

   /* Triangle base, who are facing, touch. */
   {warn__some_touch, 0, &step_tgl4_stuffa,   s_trngl4,     0xFFUL,       0xD7UL, ~0UL},
   {warn__some_touch, 0, &step_tgl4_stuffa,   s_trngl4,     0xF0UL,       0xD0UL, ~0UL}, /* Same, with missing people. */
   {warn__some_touch, 0, &step_tgl4_stuffa,   s_trngl4,     0x0FUL,       0x07UL, ~0UL}, /* Same, with missing people. */

   /* Triangle apexes, who are facing, touch. */
   {warn__some_touch, 0, &step_tgl4_stuffb,   s_trngl4,     0xFFUL,       0x22UL, ~0UL},
   {warn__some_touch, 0, &step_tgl4_stuffb,   s_trngl4,     0xF0UL,       0x20UL, ~0UL}, /* Same, with missing people. */
   {warn__some_touch, 0, &step_tgl4_stuffb,   s_trngl4,     0x0FUL,       0x02UL, ~0UL}, /* Same, with missing people. */

   /* Ends touch from a "bone" to a grand wave. */
   {warn__some_touch, 0, &step_bone_stuff,    s_bone,     0xFFFFUL,     0xA802UL, 0xFFFFUL},
   {warn__some_touch, 0, &step_bone_stuff,    s_bone,     0xFFFFUL,     0xA208UL, 0xFFFFUL},

   /* All touch from a "bone" to a rigger. */
   {warn__none,       0, &step_bone_rigstuff, s_bone,     0xFFFFUL,     0xAD07UL, 0xFFFFUL},
   {warn__none,       0, &step_bone_rigstuff, s_bone,     0xF0F0UL,     0xAD07UL, 0xF0F0UL}, /* Same, with missing people. */
   {warn__none,       0, &step_bone_rigstuff, s_bone,     0x0F0FUL,     0xAD07UL, 0x0F0FUL}, /* Same, with missing people. */

   /* Centers touch from a "rigger" to a grand wave. */
   {warn__some_touch, 0, &step_rig_stuff,     s_rigger,   0xFFFFUL,     0xA802UL, 0xFFFFUL},
   {warn__some_touch, 0, &step_rig_stuff,     s_rigger,   0xF0F0UL,     0xA802UL, 0xF0F0UL}, /* Same, with missing people. */
   {warn__some_touch, 0, &step_rig_stuff,     s_rigger,   0x0F0FUL,     0xA802UL, 0x0F0FUL}, /* Same, with missing people. */

   /* Check for stepping to a miniwave from people facing. */
   {warn__none,       0, &step_1x2_stuff,     s1x2,          0xFUL,        0x7UL, 0xFUL},

   /* Check for stepping to a box from a 1x4 single 8 chain -- we allow some phantoms.
      This is what makes triple columns turn and weave legal in certain interesting cases. */
   {warn__none,       0, &step_1x4_stuff,     s1x4,         0xFFUL,       0x7DUL, 0xFFUL},
   {warn__none,       0, &step_1x4_stuff,     s1x4,         0xF0UL,       0x7DUL, 0xF0UL},
   {warn__none,       0, &step_1x4_stuff,     s1x4,         0x0FUL,       0x7DUL, 0x0FUL},

   /* Check for stepping to a single 1/4 tag or 3/4 tag from a single-file DPT or trade-by --
      we allow some phantoms, as above. */
   {warn__none,       0, &step_1x4_side_stuff, s1x4,        0xFFUL,       0xD7UL, 0xFFUL},
   {warn__none,       0, &step_1x4_side_stuff, s1x4,        0xFFUL,       0x5FUL, 0xFFUL},
   {warn__none,       0, &step_1x4_side_stuff, s1x4,        0x33UL,       0x13UL, 0x33UL},

   /* Check for stepping to a column from a 1x8 single 8 chain. */
   {warn__none,       0, &step_1x8_stuff,      s1x8,      0xFFFFUL,     0x7DD7UL, 0xFFFFUL},
   {warn__none,       0, &step_1x8_stuff,      s1x8,      0xF0F0UL,     0x7DD7UL, 0xF0F0UL}, /* Same, with missing people. */
   {warn__none,       0, &step_1x8_stuff,      s1x8,      0x0F0FUL,     0x7DD7UL, 0x0F0FUL}, /* Same, with missing people. */

   /* Check for stepping to parallel waves from an 8 chain. */
   {warn__none,       0, &step_8ch_stuff,      s2x4,      0xFFFFUL,     0x77DDUL, 0xFFFFUL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,      0xF0F0UL,     0x77DDUL, 0xF0F0UL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,      0x0F0FUL,     0x77DDUL, 0x0F0FUL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,      0x0FF0UL,     0x77DDUL, 0x0FF0UL},
   {warn__none,       0, &step_8ch_stuff,      s2x4,      0xF00FUL,     0x77DDUL, 0xF00FUL},

   // Ends touch from diamonds to thar.
   {warn__none,       8, &step_8by_stuff,      s_qtag,    0xFFFFUL,     0x78D2UL, ~0UL},

   /* Touch from alamo 8-chain to thar. */
   {warn__none,       8, &rear_thar_stuff,     s4x4,  0x3C3C3C3CUL, 0x2034081CUL, ~0UL},

   /* Touch from "squared set" 2x4 to thar. */
   {warn__none,       8, &step_sqs_stuff,      s2x4,      0xFFFFUL,     0x9E34UL, ~0UL},

   {warn__none,       0, (expand_thing *) 0, nothing}
};

expand_thing expand_init_table[] = {

   /* Unsymmetrical recenter items. */

   {{10, 15, 3, 1, 2, 7, 4, 5, 6, 8, 9, 11},
    12, s3x4, s4x4, 0, 0UL, 0x7001,
    warn__none, warn__none, normalize_recenter, 0},

   {{12, 13, 14, 0, 1, 3, 2, 7, 11, 9, 10, 15},
    12, s3x4, s4x4, 0, 0UL, 0x0170,
    warn__none, warn__none, normalize_recenter, 0},

   {{6, 11, 15, 13, 14, 3, 0, 1, 2, 4, 5, 7},
    12, s3x4, s4x4, 1, 0UL, 0x1700,
    warn__none, warn__none, normalize_recenter, 0},

   {{8, 9, 10, 12, 13, 15, 14, 3, 7, 5, 6, 11},
    12, s3x4, s4x4, 1, 0UL, 0x0017,
    warn__none, warn__none, normalize_recenter, 0},

   {{10, 11, 5, 4, 6, 7, 8, 9},
    8, s2x4, s3x4, 0, 0UL, 0x00F,
    warn__none, warn__none, normalize_recenter, 0},

   {{0, 1, 2, 3, 4, 5, 11, 10},
    8, s2x4, s3x4, 0, 0UL, 0x3C0,
    warn__none, warn__none, normalize_recenter, 0},

   {{2, 3, 4, 5, 6, 7, 8, 9},
    8, s2x4, s2x6, 0, 0UL, 0xC03,
    warn__none, warn__none, normalize_recenter, 0},

   {{0, 1, 2, 3, 8, 9, 10, 11},
    8, s2x4, s2x6, 0, 0UL, 0x0F0,
    warn__none, warn__none, normalize_recenter, 0},

   {{2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13},
    12, s2x6, s2x8, 0, 0UL, 0xC003,
    warn__none, warn__none, normalize_recenter, 0},

   {{0, 1, 2, 3, 4, 5, 10, 11, 12, 13, 14, 15},
    12, s2x6, s2x8, 0, 0UL, 0x03C0,
    warn__none, warn__none, normalize_recenter, 0},

   {{15, 14, 12, 13, 8, 9, 11, 10},
    8, s1x8, s2x8, 0, 0UL, 0x00FF,
    warn__none, warn__none, normalize_recenter, 0},

   {{0, 1, 3, 2, 7, 6, 4, 5},
    8, s1x8, s2x8, 0, 0UL, 0xFF00,
    warn__none, warn__none, normalize_recenter, 0},

   /* This makes it possible to do "own the <points>, trade by flip the diamond"
      from point-to-point diamonds. */
   {{0, 1, 2, 3, 4, 5, 6, 7},
    8, s1x8, s_ptpd, 0, 0UL, 0xAA,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   /* These next 4 must be in this order. */
   {{1, 2, 3, 5, 6, 7},
    6, s_2x1dmd, s3x1dmd, 0, 0UL, 0x11,
    warn__none, warn__none, normalize_before_isolated_call, 0},
   // the real exp_1x4_3x1d_stuff
   {{1, 2, 5, 6},
    4, s1x4,     s3x1dmd, 0, 0UL, 0x99,
    warn__none, warn__none, normalize_before_isolated_call, 0},
   {{7, 2, 3, 6},
    4, sdmd,     s3x1dmd, 1, 0UL, 0x33,
    warn__none, warn__none, normalize_before_isolated_call, 0},
   {{2, 6},
    2, s1x2,     s3x1dmd, 0, 0UL, 0xBB,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{10, 3},
    2, s1x2,     s2x7, 1, 0UL, 0x3BF7,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    12, s2x6, sbigdhrgl, 0, 0UL, 01414,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   /* If only the "wings" are present, turn it into a 2x6. */
   {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    12, s2x6, sbigbone, 0, 0UL, 01414,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    12, s2x6, sbighrgl, 0, 0UL, 01414,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   /* If only the center diamond is present, turn it into a 3dmd.
            If only the "wings" are present, turn it into a 2x6. */
   {{-1, 2, -1, -1, -1, 3, -1, 8, -1, -1, -1, 9},
    12, s3dmd, sbighrgl, 0, 0UL, 06363,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
    12, s2x6, sbigdmd, 0, 0UL, 01414,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   /* If only the center 1x4 is present, turn it into a 3x4.
      If only the "wings" are present, turn it into a 2x6. */
   {{-1, -1, -1, -1, 2, 3, -1, -1, -1, -1, 8, 9},
    12, s3x4, sbigdmd, 1, 0UL, 06363,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{0, -1, -1, 1, 3, -1, -1, 4},
    8, s2x4, s_bone6, 0, 0UL, 044,
    warn__none, warn__none, normalize_before_isolate_strict, 0},

   {{0, -1, -1, 1, 4, -1, -1, 5},
    8, s2x4, s_bone, 0, 0UL, 0xCC,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{-1, -1, 7, 6, -1, -1, 3, 2},
    8, s1x8, s_bone, 0, 0UL, 0x33,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{-1, 0, 1, -1, -1, 4, 5, -1},
    8, s2x4, s_rigger, 0, 0UL, 0xCC,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   /* This makes it possible to do "ends explode" from a rigger. */
   // These two must be in this order.

   {{6, 7, -1, -1, 2, 3, -1, -1},
    8, s1x8, s_rigger, 0, 0UL, 0x33,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{6, 7, -1, 2, 3, -1},
    8, s1x6, s_rigger, 0, 0UL, 0x33,
    warn__none, warn__none, normalize_before_isolate_strict, 0},

   /* This makes it possible to do "own the <points>, trade by flip the diamond" from
      a single diamond. */
   /* We do NOT compress to a 1x2 -- see comment above. */
   {{0, 1, 2, 3},
    4, s1x4, sdmd, 0, 0UL, 0xA,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   // **** Have downgraded these next two, so that points of hourglass can do
   // disconnected peel to diamond or 1/2 circulate.

   {{0, -1, -1, 1, 4, -1, -1, 5},
    8, s2x4, s_dhrglass, 0, 0UL, 0xCC,
    warn__none, warn__none, normalize_before_isolate_strict, 0},
   {{5, -1, -1, 0, 1, -1, -1, 4},
    8, s2x4, s_hrglass, 1, 0UL, 0xCC,
    warn__none, warn__none, normalize_before_isolate_strict, 0},

   /* This makes it possible to do "own the <points>, trade by flip the diamond" from
      normal diamonds. */
   /* We do NOT compress to a 2x2 -- doing so might permit people to
      work with each other across the set when they shouldn't, as in
      "heads pass the ocean; heads recycle while the sides star thru". */
   // comp_qtag_2x4_stuff
   {{5, -1, -1, 0, 1, -1, -1, 4},
    8, s2x4, s_qtag, 1, 0UL, 0xCC,
    warn__none, warn__none, normalize_before_isolated_call, 0},

   {{5, 1},
    2, s1x2, s_spindle, 1, 0UL, 0xDD,
    warn__none, warn__none, normalize_to_2, 0},

   // exp_1x2_dmd_stuff
   {{3, 1},
    2, s1x2, sdmd, 1, 0UL, 0x5,
    warn__none, warn__none, normalize_to_2, 0},

   {{1, 3},
    2, s1x2, s1x4, 0, 0UL, 0x5,
    warn__none, warn__none, normalize_to_2, 0},

   {{4, 1},
    2, s1x2, s2x3, 1, 0UL, 055,
    warn__none, warn__none, normalize_to_2, 0},

   // exp_2x3_qtg_stuff
   {{5, 7, 0, 1, 3, 4},
    6, s2x3, s_qtag, 1, 0UL, 0x44,
    warn__none, warn__none, normalize_to_6, 0},

   {{10, 11, 4, 5},
     4, s1x4, s3x4, 0, 0UL, 01717,
    warn__none, warn__none, normalize_to_4, 0},

   {{5, 1, 2, 4},
    4, sdmd, s_2x1dmd, 1, 0UL, 011,
    warn__none, warn__none, normalize_to_4, 0},

   {{1, 2, 4, 5},
    4, sdmd, s_1x2dmd, 0, 0UL, 011,
    warn__none, warn__none, normalize_to_4, 0},

   {{0, 1, 3, 4},
    4, s1x4, s_2x1dmd, 0, 0UL, 044,
    warn__none, warn__none, normalize_to_4, 0},

   {{1, 3, 5, 7},
    4, s2x2, s_galaxy, 0, 0UL, 0x55,
    warn__none, warn__none, normalize_to_4, 0},

   {{6, 7, 2, 3},
    4, s1x4, s_crosswave, 1, 0UL, 0x33,
    warn__none, warn__none, normalize_to_4, 0},

   {{7, 3},
    2, s1x2, s_crosswave, 1, 0UL, 0x77,
    warn__none, warn__none, normalize_to_2, 0},

   {{0, 1, 4, 5},
    4, s2x2, s_rigger, 0, 0UL, 0xCC,
    warn__none, warn__none, normalize_to_4, 0},

   {{6, 7, 2, 3},
    4, s1x4, s_bone, 0, 0UL, 0x33,
    warn__none, warn__none, normalize_to_4, 0},

   {{1, 2, 4, 5},
    4, s1x4, s1x6, 0, 0UL, 011,
    warn__none, warn__none, normalize_to_4, 0},

   {{3, 2, 7, 6},
    4, s1x4, s1x8, 0, 0UL, 0x33,
    warn__none, warn__none, normalize_to_4, 0},

   {{2, 6},
    2, s1x2, s1x8, 0, 0UL, 0xBB,
    warn__none, warn__none, normalize_to_2, 0},

   {{1, 2, 5, 6},
    4, s2x2, s2x4, 0, 0UL, 0x99,
    warn__none, warn__none, normalize_to_4, 0},

   {{1, 3, 2, 5, 7, 6},
    6, s1x6, s1x8, 0, 0UL, 0x11,
    warn__none, warn__none, normalize_to_6, 0},

   {{1, 2, 3, 5, 6, 7},
    6, s_1x2dmd, s1x3dmd, 0, 0UL, 0x11,
    warn__none, warn__none, normalize_to_6, 0},

   {{0, 1, 2, 4, 5, 6},
    6, s1x6, s3x1dmd, 0, 0UL, 0x88,
    warn__none, warn__none, normalize_to_6, 0},

   {{6, 7, 2, 3},
    4, s1x4, s_qtag, 0, 0UL, 0x33,
    warn__none, warn__none, normalize_to_4, 0},

   {{7, 3},
    2, s1x2, s_qtag, 0, 0UL, 0x77,
    warn__none, warn__none, normalize_to_2, 0},

   {{3, 1},
    2, s1x2, s_star, 1, 0UL, 0x5,
    warn__none, warn__none, normalize_to_2, 0},
   {{0, 2},
    2, s1x2, s_star, 0, 0UL, 0xA,
    warn__none, warn__none, normalize_to_2, 0},

   {{9, 11, 13, 6, 2, 0, 1, 3, 5, 14, 10, 8},
    12, s2x6, sdeepbigqtg, 1, 0UL, 0x9090,
    warn__none, warn__phantoms_thinner, simple_normalize, 0},

   {{2, 3, 7, 6, 5, 4, 10, 11, 15, 14, 13, 12},
    12, sbigh, sdblxwave, 0, 0UL, 0x0303,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_DBLX)},

   {{10, -1,-1, 1, 2, -1, -1, 9},
    8, s2x4, s4dmd, 1, 0x66, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},

   {{0, 1, -1, -1, 2, 3, 5, 6, 7, 8, 9, -1, -1, 10, 11, 13, 14, 15},
    18, s3x6, s4dmd, 0, 0014014, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},

   {{1, -1, -1, 2, 6, 7, 9, -1, -1, 10, 14, 15},
    12, s3x4, s4dmd, 0, 00606, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},

   {{0, 1, 2, 3, -1, -1, 8, 9, 10, 11, -1, -1},
    12, s3x4, s4dmd, 0, 06060, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},

   {{-1, 2, 3, -1, -1, 6, 7, 8, -1, 11, 12, -1, -1, 15, 16, 17},
    16, s4dmd, s3x6, 0, 0x1919, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X6)},

   {{13, 14, 1, -1, -1, 10, 5, 6, 9, -1, -1, 2},
    12, sdeepqtg, s4x4, 0, 03030, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4)},

   {{-1, -1, 13, 14, 1, -1, -1, 10, -1, -1, 5, 6, 9, -1, -1, 2},
    16, sdeepbigqtg, s4x4, 0, 0x6363, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4)},

   {{10, 13, -1, 15, 14, 1, 3, -1, 2, 5, -1, 7, 6, 9, 11, -1},
    16, s_c1phan, s4x4, 0, 0x8484, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                                         NEEDMASK(CONCPROP__NEEDK_4X6) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},
   {{10, 13, 15, -1, 14, 1, -1, 3, 2, 5, 7, -1, 6, 9, -1, 11},
    16, s_c1phan, s4x4, 0, 0x4848, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                                         NEEDMASK(CONCPROP__NEEDK_4X6) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},
   {{10, 13, 15, -1, 14, 1, 3, -1, 2, 5, 7, -1, 6, 9, 11, -1},
    16, s_c1phan, s4x4, 0, 0x8888, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                                         NEEDMASK(CONCPROP__NEEDK_4X6) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},

   {{10, 13, -1, 15, 14, 1, -1, 3, 2, 5, -1, 7, 6, 9, -1, 11},
    16, s_c1phan, s4x4, 0, 0x4444, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                                         NEEDMASK(CONCPROP__NEEDK_4X6) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},


   {{12, 13, 14, 0, -1, -1, -1, -1, 4, 5, 6, 8, -1, -1, -1, -1},
    16, s4dmd, s4x4, 0, 0xF0F0, ~0UL,
    warn__check_4x4_start, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                                         NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                                         NEEDMASK(CONCPROP__NEEDK_4X6) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                                         NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},

   {{0, 1, 2, 3, -1, -1, 4, 5, 6, 7, 8, 9, -1, -1, 10, 11},
    16, s4dmd, s3x4, 0, 0x3030, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X4) |
                                              NEEDMASK(CONCPROP__NEEDK_3X4_D3X4) |
                                              NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4)},

   {{0, 1, 4, 5, -1, 6, 7, 8, 9, 10, 13, 14, -1, 15, 16, 17},
    16, s4dmd, s3x6, 0, 0x1010, ~0UL,
    warn__check_centered_qtag, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X6)},
   {{1, 2, 5, 6, 8, 9, 10, 11, 13, 14, 17, 18, 20, 21, 22, 23},
    16, s4dmd, s3x8, 0, 0UL, ~0UL,
    warn__check_centered_qtag, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X8)},

   {{1, 2, 3, 5, 7, 8, 9, 11},
    8, s_spindle, s_d3x4, 0, 0UL, 02121,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X4_D3X4)},
   {{1, 2, 3, 7, 8, 9},
    6, s2x3, s_d3x4, 0, 0UL, 06161,
    warn__none, warn__none, simple_normalize, 0},

   {{2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15},
    12, sdeepqtg, sdeepbigqtg, 0, 0, 0x0303,
    warn__none, warn__none, simple_normalize, 0},

   {{8, 11, 1, 2, 5, 7},
    6, s2x3, s3x4, 1, 0UL, 03131,
    warn__none, warn__none, simple_normalize, 0},
   {{12, 14, 3, 1, 4, 6, 11, 9},
    8, s2x4, s_c1phan, 1, 0UL, 0xA5A5,
    warn__none, warn__none, simple_normalize, 0},
   {{0, 2, 7, 5, 8, 10, 15, 13},
    8, s2x4, s_c1phan, 0, 0UL, 0x5A5A,
    warn__none, warn__none, simple_normalize, 0},
   {{6, 11, 15, 13, 14, 3, 7, 5},
    8, s2x4, s4x4, 1, 0UL, 0x1717,
    warn__none, warn__none, simple_normalize, 0},

   {{10, 15, 3, 1, 2, 7, 11, 9},
    8, s2x4, s4x4, 0, 0UL, 0x7171,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                              NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                              NEEDMASK(CONCPROP__NEEDK_4X6) |
                                              NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                              NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},

   // exp_qtg_3x4_stuff
   /* ***** This is a kludge to make threesome work!!!! */
   {{1, 2, 4, 5, 7, 8, 10, 11},
    8, s_qtag, s3x4, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X4) |
                                              NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_BLOB) |
                                              NEEDMASK(CONCPROP__NEEDK_4X6) |
                                              NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                              NEEDMASK(CONCPROP__NEEDK_TWINQTAG) |
                                              NEEDMASK(CONCPROP__NEEDK_3X4) |
                                              NEEDMASK(CONCPROP__NEEDK_3X4_D3X4) |
                                              NEEDMASK(CONCPROP__NEEDK_3X8) |
                                              NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4)},
   {{0, 1, 2, 4, 5, 6, 7, 9},
    8, s_323, s_343, 0, 0UL, 0x108,
    warn__none, warn__none, simple_normalize, 0},
   {{1, 2, 3, 5, 7, 8, 9, 11},
    8, s_323, s_525, 0, 0UL, 02121,
    warn__none, warn__none, simple_normalize, 0},
   {{1, 2, 3, 5, 6, 8, 9, 10, 12, 13},
    10, s_343, s_545, 0, 0UL, 0x891,
    warn__none, warn__none, simple_normalize, 0},
   {{0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 13},
    12, s_525, s_545, 0, 0UL, 0x1020,
    warn__none, warn__none, simple_normalize, 0},
   {{0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 13},
    12, s_525, sh545, 0, 0UL, 0x1020,
    warn__none, warn__none, simple_normalize, 0},
   {{0, -1, 1, -1, 2, 4, 5, 6, 19, 18, 10, -1, 11, -1, 12, 14, 15, 16, 9, 8},
    20, s4x5, s3oqtg, 0, 0UL, 0x22088,
    warn__none, warn__none, simple_normalize, 0},
   {{1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 13, 14, 15, 16, 17, 18},
    16, s2x8, s2x10, 0, 0UL, 0x80601,
    warn__none, warn__none, simple_normalize, 0},
   {{-1, 15, 16, 20, 21, -1, 0, 22, 23, 17, 14, 13, -1, 3, 4, 8, 9, -1, 12, 10, 11, 5, 2, 1},
    24, s4x6, s_bigblob, 0, 0UL, 0x0C00C0,
    warn__none, warn__none, simple_normalize, 0},
   {{-1, 21, 22, 2, 3, -1, 6, 4, 5, 23, 20, 19, -1, 9, 10, 14, 15, -1, 18, 16, 17, 11, 8, 7},
    24, s4x6, s_bigblob, 0, 0UL, 0x003003,
    warn__none, warn__none, simple_normalize, 0},
   {{7, 6, 5, 4, 15, 14, 13, 12},
    8, s2x4, sdeepbigqtg, 0, 0UL, 0xF0F,
    warn__none, warn__none, simple_normalize, 0},
   {{5, 4, 3, 2, 11, 10, 9, 8},
    8, s2x4, sdeepxwv, 1, 0UL, 0303,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_DEEPXWV)},
   {{1, 2, 3, 4, 5, 6, 7, 9, 10, 11, 12, 13, 14, 15},
    14, s1x14, s1x16, 0, 0UL, 0x101,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_1X16)},
   {{1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13},
    12, s1x12, s1x14, 0, 0UL, 0x81,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_1X16)},
   {{0, 1, 2, 3, 5, 14, 8, 9, 10, 11, 13, 6},
    12, sbigrig, sbigbigx, 0, 0UL, 0x9090,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_QUAD_1X4)},
   {{3, 10, 6, 7, 9, 4, 0, 1},
    8, s_rigger, sdeepxwv, 0, 0UL, 04444,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_QUAD_1X4)},

   {{2, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15},
    12, sdeepxwv, sbigbigx, 0, 0UL, 0x303,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_QUAD_1X4)},

   // order of these next 2 items must be as shown: must be 3x4, 1x6!!!!
   {{15, 16, 17, 6, 7, 8},
    6,  s1x6, s3x6, 0, 0UL, 0077077,
    warn__none, warn__none, simple_normalize, 0},
   {{1, 2, 3, 4, 7, 8, 10, 11, 12, 13, 16, 17},
    12, s3x4, s3x6, 0, 0UL, 0141141,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X6)},

   {{12, 13, 15, 14, 4, 5, 7, 6},
    8, s1x8, s4dmd, 0, 0UL, 0x0F0F,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},
   {{1, 2, 5, 6, 7, 9, 10, 13, 14, 15},
    10, swqtag, s4dmd, 0, 0UL, 0x1919,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},
   {{1, 2, 6, 7, 9, 10, 14, 15},
    8, s_qtag, s4dmd, 0, 0UL, 0x3939,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD) |
                                              NEEDMASK(CONCPROP__NEEDK_4DMD)},
   {{0, 1, 3, 4, 5, 6, 8, 9},
    8, s_qtag, swqtag, 0, 0UL, 0x084,
    warn__none, warn__none, simple_normalize, 0},

   {{0, 1, 3, 4, 5, 6, 8, 9},
    8, s2x4, sdeep2x1dmd, 0, 0UL, 0x084,
    warn__none, warn__none, simple_normalize, 0},
   {{5, 4, 1, 3, 2, 11, 10, 7, 9, 8},
    10, sdeep2x1dmd, sdeepxwv, 1, 0UL, 00101,
    warn__none, warn__none, simple_normalize, 0},

   {{11, 10, 9, 8, 7, 6, 23, 22, 21, 20, 19, 18},
    12, s2x6, s4x6, 0, 0UL, 0x03F03F,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X6) |
                                              NEEDMASK(CONCPROP__NEEDK_TWINDMD) |
                                              NEEDMASK(CONCPROP__NEEDK_TWINQTAG)},

   // order of these next 3 items must be as shown: must be 1x8, 3x6, 3x4!!!!
   {{20, 21, 23, 22, 8, 9, 11, 10},
    8, s1x8, s3x8, 0, 0UL, 0x0FF0FF,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X8)},
   {{1, 2, 3, 4, 5, 6, 9, 10, 11, 13, 14, 15, 16, 17, 18, 21, 22, 23},
    18, s3x6, s3x8, 0, 0UL, 0x181181,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X8)},
   {{2, 3, 4, 5, 10, 11, 14, 15, 16, 17, 22, 23},
    12, s3x4, s3x8, 0, 0UL, 0x3C33C3,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3X8)},

   {{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22},
    20, s2x10, s2x12, 0, 0UL, 0x801801,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_2X12)},
   {{2, 3, 4, 5, 6, 7, 8, 9, 14, 15, 16, 17, 18, 19, 20, 21},
    16, s2x8, s2x12, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_2X12)},
   {{3, 4, 5, 6, 7, 8, 15, 16, 17, 18, 19, 20},
    12, s2x6, s2x12, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_2X12)},
   {{4, 5, 6, 7, 16, 17, 18, 19},
    8, s2x4, s2x12, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_2X12)},
   {{14, 1, 15, 10, 6, 9, 7, 2},
    8, s_ptpd, s4ptpd,0, 0UL, 0x3939,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4D_4PTPD)},
   {{1, 2, 4, 3, 6, 7, 9, 8},
    8, s1x8, s1x10, 0, 0UL, 0x21,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_1X10)},
   {{1, 8, 10, 11, 7, 2, 4, 5},
    8, s_bone, sbigh, 0, 0UL, 01111,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_END_1X4)},
   {{1, 2, 3, 5, 7, 8, 9, 11},
    8, s1x3dmd, sbigx, 0, 0UL, 02121,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_CTR_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_END_1X4)},
   {{2, 3, 4, 5, 8, 9, 10, 11},
    8, s_crosswave, sbigx, 0, 0UL, 0303,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_CTR_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_END_1X4)},
   {{1, 2, 3, 4, 5, 7, 8, 9, 10, 11},
    10, s1x10, s1x12, 0, 0UL, 0101,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_CTR_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_END_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_QUAD_1X3) |
                                              NEEDMASK(CONCPROP__NEEDK_1X12) |
                                              NEEDMASK(CONCPROP__NEEDK_1X16)},
   {{2, 3, 5, 4, 8, 9, 11, 10},
    8, s1x8,    s1x12, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_CTR_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_TRIPLE_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_END_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_1X12) |
                                              NEEDMASK(CONCPROP__NEEDK_QUAD_1X3) |
                                              NEEDMASK(CONCPROP__NEEDK_QUAD_1X4) |
                                              NEEDMASK(CONCPROP__NEEDK_1X16)},
   {{1, 4, 3, 2, 7, 10, 9, 8},
    8, s_dhrglass,sbigdhrgl,0, 0UL, 04141,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_END_2X2)},
   {{10, 1, 2, 9, 4, 7, 8, 3},
    8, s_hrglass, sbighrgl, 1, 0UL, 04141,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_END_2X2)},
   {{1, 4, 8, 9, 7, 10, 2, 3},
    8, s_bone, sbigbone, 0, 0UL, 04141,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_END_2X2) |
                                              NEEDMASK(CONCPROP__NEEDK_CTR_1X4)},
   {{10, 1, 2, 3, 4, 7, 8, 9},
    8, s_qtag, sbigdmd, 1, 0UL, 04141,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_END_2X2) |
                                              NEEDMASK(CONCPROP__NEEDK_CTR_1X4)},
   {{4, 5, 8, 9, 10, 11, 2, 3},
    8, s_rigger,sbigrig,0, 0UL, 0303,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_CTR_2X2) |
                                              NEEDMASK(CONCPROP__NEEDK_END_1X4)},
   {{1, 2, 3, 4, 7, 8, 9, 10},
    8, s2x4, s2x6, 0, 0UL, 04141,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_2X6) |
                                              NEEDMASK(CONCPROP__NEEDK_CTR_2X2) | 
                                              NEEDMASK(CONCPROP__NEEDK_END_2X2)},
   {{11, 5},
    2, s1x2, s3dmd, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3DMD)},
   {{0, 1, 2, 4, 5, 6, 7, 8, 10, 11},
    10, s_343, s3dmd, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3DMD)},
   {{0, 1, 2, 5, 6, 7, 8, 11},
    8, s_323, s3dmd, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3DMD)},
   {{9, 10, 11, 1, 3, 4, 5, 7},
    8, s1x3dmd, s_3mdmd, 0, 0UL, 0505,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_3DMD)},
   {{9, 10, 11, 1, 3, 4, 5, 7},
    8,  s3x1dmd, s3dmd,  0, 0UL, 0505,
    warn__none, warn__none, simple_normalize,   NEEDMASK(CONCPROP__NEEDK_END_DMD) |
                                                NEEDMASK(CONCPROP__NEEDK_CTR_DMD) |
                                                NEEDMASK(CONCPROP__NEEDK_3DMD)},
   {{1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14},
    12, s2x6,    s2x8,   0, 0UL, 0x8181,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_2X8)},
   {{2, 3, 4, 5, 10, 11, 12, 13},
    8,  s2x4,    s2x8,   0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize,   NEEDMASK(CONCPROP__NEEDK_2X8)},
   {{3, 4, 8, 5, 9, 10, 14, 11, 15, 16, 20, 17, 21, 22, 2, 23},
    16, s4x4, s_bigblob, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize,   NEEDMASK(CONCPROP__NEEDK_BLOB)},

   {{9, 8, 2, 7, 6, 5, 19, 18, 12, 17, 16, 15},
    12, sbigdmd, s4x5, 0, 0UL, ~0UL,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X5)},

   {{9, 8, 7, 6, 5, 19, 18, 17, 16, 15},
    10, s2x5, s4x5, 0, 0UL, 0x07C1F,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X5)},
   {{13, 16, 8, 1, 2, 7, 3, 6, 18, 11, 12, 17},
    12, s3x4, s4x5, 1, 0UL, 0x8C631,
    warn__none, warn__none, simple_normalize, NEEDMASK(CONCPROP__NEEDK_4X5)},

   {{0}, 0, nothing, nothing}};


/* These are used in mirror_this, for handling difficult triangles. */

Const coordrec tgl3_0 = {s_trngl, 3,
   {  0,  -2,   2},
   {  0,   4,   4}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  1,  2, -1, -1, -1,
      -1, -1, -1, -1,  0, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Const coordrec tgl3_1 = {s_trngl, 3,
   {  0,   4,   4},
   {  0,   2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  0,  1, -1, -1,
      -1, -1, -1, -1, -1,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Const coordrec tgl4_0 = {s_trngl4, 3,
   {  0,   0,  -2,   2},
   { -4,   0,   4,   4}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  2,  3, -1, -1, -1,
      -1, -1, -1, -1,  1, -1, -1, -1,
      -1, -1, -1, -1,  0, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

Const coordrec tgl4_1 = {s_trngl4, 3,
   { -4,   0,   4,   4},
   {  0,   0,   2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1, -1, -1,  3, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

/* These are used in sdmoves to fudge matrix calls. */

const coordrec squeezethingglass = {s_hrglass, 3,
   { -4,   4,   8,   0,   4,  -4,  -8,   0},
   {  6,   6,   0,   2,  -6,  -6,   0,  -2}, {0}};

const coordrec squeezethinggal = {s_galaxy, 3,
   { -6,  -2,   0,   2,   6,   2,   0,  -2},
   {  0,   2,   6,   2,   0,  -2,  -6,  -2}, {0}};

const coordrec squeezethingqtag = {s_qtag, 3,
   { -2,   2,   6,   2,   2,  -2,  -6,  -2},
   {  4,   4,   0,   0,  -4,  -4,   0,   0}, {0}};

const coordrec squeezething4dmd = {s4dmd, 3,
   {-10,  -5,   5,  10,  14,  10,   6,   2,  10,   5,  -5, -10, -14, -10,  -6,  -2},
   {  6,   6,   6,   6,   0,   0,   0,   0,  -6,  -6,  -6,  -6,   0,   0,   0,   0}, {0}};

const coordrec squeezefinalglass = {s_hrglass, 3,
   { -2,   2,   6,   0,   2,  -2,  -6,   0},
   {  6,   6,   0,   2,  -6,  -6,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1,  6, -1,  3, -1,  2, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1,  5,  4, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec truck_to_ptpd = {s_ptpd, 3,
   {-10,  -6,  -2,  -6,  10,   6,   2,   6},
   {  0,   2,   0,  -2,   0,  -2,   0,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  6,  7,  4, -1,
      -1, -1,  3, -1, -1,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec truck_to_deepxwv = {sdeepxwv, 4,
   {-11,  -7,  -2,  -2,  -2,  -2,  11,   7,   2,   2,   2,   2},
   {  0,   0,   6,   2,  -2,  -6,   0,   0,  -6,  -2,   2,   6}, {
      -1, -1, -1, -1, -1, -1, -1,  2, 11, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1,  0,  1,  3, 10,  7,  6, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  4,  9, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  5,  8, -1, -1, -1, -1, -1, -1, -1}};

const coordrec press_4dmd_4x4 = {s4x4, 3,
   { 11,   9,   9,   1,  11,   5,  -5,   1, -11,  -9,  -9,  -1, -11,  -5,   5,  -1},
   {  7,   1,  -1,   1,  -7,  -7,  -7,  -1,  -7,  -1,   1,  -1,   7,   7,   7,   1}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, 12, 13, -1, -1, 14,  0, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  8,  6, -1, -1,  5,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec press_4dmd_qtag1 = {s_qtag, 3,
   { -7,  -1,   6,   2,   7,   1,  -6,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1, -1, -1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec press_4dmd_qtag2 = {s_qtag, 3,
   {  1,   7,   6,   2,  -1,   -7,  -6,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  0,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5,  4, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec press_qtag_4dmd1 = {s4dmd, 3,
   {-11,  -5,   0,   9,  14,  10,   6,   2,  11,   5,   0,  -9, -14, -10,  -6,  -2},
   {  5,   5,   5,   5,   0,   0,   0,   0,  -5,  -5,  -5,  -5,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1, -1,  2, -1,  3, -1,
      12, 13, 14, 15,  7,  6,  5,  4,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, 11, -1, -1, 10,  9,  8, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec press_qtag_4dmd2 = {s4dmd, 3,
   { -8,   1,   5,  11,  14,  10,   6,   2,   8,  -1,  -5, -11, -14, -10,  -6,  -2},
   {  5,   5,   5,   5,   0,   0,   0,   0,  -5,  -5,  -5,  -5,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2,  3, -1,
      12, 13, 14, 15,  7,  6,  5,  4,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, 11, 10,  9, -1, -1,  8, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

const coordrec acc_crosswave = {s_crosswave, 3,
   { -8,  -4,   0,   0,   8,   4,   0,   0},
   {  0,   0,   6,   2,   0,   0,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1,  0,  1,  3,  5,  4, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

/* These are used in setup_coords. */

static coordrec thing1x1 = {s1x1, 3,
   {  0},
   {  0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  0, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x2 = {s1x2, 3,
   { -2,   2},
   {  0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x3 = {s1x3, 3,
   { -4,   0,   4},
   {  0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x4 = {s1x4, 3,
   { -6,  -2,   6,   2},
   {  0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x6 = {s1x6, 3,
   {-10,  -6,  -2,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  5,  4,  3, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x8 = {s1x8, 3,
   {-14, -10,  -2,  -6,  14,  10,   2,   6},
   {  0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  3,  2,  6,  7,  5,  4,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x10 = {s1x10, 4,
   {-18, -14, -10,  -6,  -2,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2,  3,  4,  9,  8,  7,  6,  5, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x12 = {s1x12, 4,
   {-22, -18, -14, -10,  -6,  -2,  22,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4,  5, 11, 10,  9,  8,  7,  6, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x14 = {s1x14, 4,
   {-26, -22, -18, -14, -10,  -6,  -2,  26,  22,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5,  6, 13, 12, 11, 10,  9,  8,  7, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x16 = {s1x16, 4,
   {-30, -26, -22, -18, -14, -10,  -6,  -2,  30,  26,  22,  18,  14,  10,   6,   2},
   {  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  4,  5,  6,  7, 15, 14, 13, 12, 11, 10,  9,  8,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingdmd = {sdmd, 3,
   { -5,   0,   4,   0},
   {  0,   2,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicethingdmd = {sdmd, 3,
   { -5,   0,   5,   0},
   {  0,   2,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x2 = {s2x2, 3,
   { -2,   2,   2,  -2},
   {  2,   2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, -1,  3,  2, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x3 = {s2x3, 3,
   { -4,   0,   4,   4,   0,  -4},
   {  2,   2,   2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1,  5,  4,  3, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x4 = {s2x4, 3,
   { -6,  -2,   2,   6,   6,   2,  -2,  -6},
   {  2,   2,   2,   2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3, -1, -1,
      -1, -1,  7,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x5 = {s2x5, 3,
   { -8,  -4,   0,   4,   8,   8,   4,   0,  -4,  -8},
   {  2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1,  9,  8,  7,  6,  5, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x6 = {s2x6, 3,
   {-10,  -6,  -2,   2,   6,  10,  10,   6,   2,  -2,  -6,  -10},
   {  2,   2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5, -1,
      -1, 11, 10,  9,  8,  7,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x7 = {s2x7, 3,
   { -12, -8,  -4,   0,   4,   8,  12,  12,   8,   4,   0,  -4,  -8, -12},
   {  2,   2,   2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5,  6,
      -1, 13, 12, 11, 10,  9,  8,  7,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x8 = {s2x8, 3,
   {-14, -10,  -6,  -2,   2,   6,  10,  14,  14,  10,   6,   2,  -2,  -6, -10, -14},
   {  2,   2,   2,   2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  4,  5,  6,  7,
      15, 14, 13, 12, 11, 10,  9,  8,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x10 = {s2x10, 4,
   {-18, -14, -10,  -6,  -2,   2,   6,  10,  14,  18,  18,  14,  10,   6,   2,  -2,  -6, -10, -14, -18},
   {  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1,
      -1, -1, -1, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3x6 = {s3x6, 3,
   {-10,  -6,  -2,   2,   6,  10,  10,   6,   2,  10,   6,   2,  -2,  -6, -10, -10,  -6,  -2},
   {  4,   4,   4,   4,   4,   4,   0,   0,   0,  -4,  -4,  -4,  -4,  -4,  -4,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5, -1,
      -1, 15, 16, 17,  8,  7,  6, -1,
      -1, 14, 13, 12, 11, 10,  9, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3x8 = {s3x8, 3,
   {-14, -10,  -6,  -2,   2,   6,  10,  14,  14,  10,   6,   2,  14,  10,   6,   2,  -2,  -6, -10, -14, -14, -10,  -6,  -2},
   {  4,   4,   4,   4,   4,   4,   4,   4,   0,   0,   0,   0,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  4,  5,  6,  7,
      20, 21, 22, 23, 11, 10,  9,  8,
      19, 18, 17, 16, 15, 14, 13, 12,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3x4 = {s3x4, 3,
   { -6,  -2,   2,   6,   6,   2,   6,   2,  -2,  -6,  -6,  -2},
   {  4,   4,   4,   4,   0,   0,  -4,  -4,  -4,  -4,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3, -1, -1,
      -1, -1, 10, 11,  5,  4, -1, -1,
      -1, -1,  9,  8,  7,  6, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing_d3x4 = {s_d3x4, 3,
   { -8,  -4,   0,   4,   8,   8,   8,   4,   0,  -4,  -8,  -8},
   {  4,   2,   2,   2,   4,   0,  -4,  -2,  -2,  -2,  -4,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1, -1,  4, -1,
      -1, -1, 11,  1,  2,  3,  5, -1,
      -1, -1, 10,  9,  8,  7,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing_323 = {s_323, 3,
   { -4,   0,   4,   2,   4,   0,  -4,  -2},
   {  4,   4,   4,   0,  -4,  -4,  -4,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1, -1,  7,  3, -1, -1, -1,
      -1, -1, -1,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing_343 = {s_343, 3,
   { -4,   0,   4,   6,   2,   4,   0,  -4,  -6,  -2},
   {  4,   4,   4,   0,   0,  -4,  -4,  -4,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2, -1, -1,
      -1, -1,  8,  9,  4,  3, -1, -1,
      -1, -1, -1,  7,  6,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing_525 = {s_525, 3,
   { -8,  -4,   0,   4,   8,   2,   8,   4,   0,  -4,  -8,  -2},
   {  4,   4,   4,   4,   4,   0,  -4,  -4,  -4,  -4,  -4,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1, -1, 11,  5, -1, -1, -1,
      -1, -1, 10,  9,  8,  7,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing_545 = {s_545, 3,
   { -8,  -4,   0,   4,   8,   6,   2,   8,   4,   0,  -4,  -8,  -6,  -2},
   {  4,   4,   4,   4,   4,   0,   0,  -4,  -4,  -4,  -4,  -4,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1, 12, 13,  6,  5, -1, -1,
      -1, -1, 11, 10,  9,  8,  7, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing_h545 = {sh545, 3,
   { -8,  -4,   0,   4,   8,   8,   2,   8,   4,   0,  -4,  -8,  -8,  -2},
   {  4,   4,   4,   4,   4,   0,   0,  -4,  -4,  -4,  -4,  -4,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1, 12, 13,  6, -1,  5, -1,
      -1, -1, 11, 10,  9,  8,  7, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thinggal = {s_galaxy, 3,
   { -7,  -2,   0,   2,   7,   2,   0,  -2},
   {  0,   2,   7,   2,   0,  -2,  -7,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1,  0,  1,  3,  4, -1, -1,
      -1, -1, -1,  7,  5, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingthar = {s_thar, 3,
   { -9,  -5,   0,   0,   9,   5,   0,   0},
   {  0,   0,   9,   5,   0,   0,  -9,  -5}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1,  0,  1, -1, -1,  5,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingxwv = {s_crosswave, 3,
   { -9,  -5,   0,   0,   9,   5,   0,   0},
   {  0,   0,   6,   2,   0,   0,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1,  0,  1, -1,  3,  5,  4, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingrigger = {s_rigger, 3,
   { -2,   2,  10,   6,   2,  -2, -10,  -6},
   {  2,   2,   0,   0,  -2,  -2,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  6,  7,  0,  1,  3,  2, -1,
      -1, -1, -1,  5,  4, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingd2x5 = {sd2x5, 3,
   { -6,  -6,  -2,  -2,  -2,   6,   6,   2,   2,   2},
   {  2,  -2,   4,   0,  -4,  -2,   2,  -4,   0,   4}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  2,  9, -1, -1, -1,
      -1, -1,  0,  3,  8,  6, -1, -1,
      -1, -1,  1,  4,  7,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingspindle = {s_spindle, 3,
   { -4,   0,   4,   8,   4,   0,  -4,  -8},
   {  2,   2,   2,   0,  -2,  -2,  -2,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  7,  0,  1,  2,  3, -1,
      -1, -1, -1,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbone = {s_bone, 3,
   {-10,  10,   6,   2,  10, -10,  -6,  -2},
   {  2,   2,   0,   0,  -2,  -2,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  6,  7,  3,  2,  1, -1,
      -1,  5, -1, -1, -1, -1,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbone6 = {s_bone6, 3,
   { -6,   6,   2,   6,  -6,  -2},
   {  2,   2,   0,  -2,  -2,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  5,  2,  1, -1, -1,
      -1, -1,  4, -1, -1,  3, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingqtag = {s_qtag, 3,       /* "fudged" coordinates */
   { -4,   5,   6,   2,   4,  -5,  -6,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0, -1,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicethingqtag = {s_qtag, 3,   /* "true" coordinates */
   { -5,   5,   6,   2,   5,  -5,  -6,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingwqtag = {swqtag, 3,       // "fudged" coordinates
   { -4,   5,  10,   6,   2,   4,  -5, -10,  -6,  -2},
   {  5,   5,   0,   0,   0,  -5,  -5,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0, -1,  1, -1, -1,
      -1,  7,  8,  9,  4,  3,  2, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  6, -1, -1,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicethingwqtag = {swqtag, 3,   // "true" coordinates
   { -5,   5,  10,   6,   2,   5,  -5, -10,  -6,  -2},
   {  5,   5,   0,   0,   0,  -5,  -5,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1,  1, -1, -1,
      -1,  7,  8,  9,  4,  3,  2, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  6, -1, -1,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2stars = {s_2stars, 3,       /* "fudged" coordinates */
   { -4,   5,   7,   2,   4,  -5,  -7,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0, -1,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicething2stars = {s_2stars, 3,   /* "true" coordinates */
   { -5,   5,   7,   2,   5,  -5,  -7,  -2},
   {  5,   5,   0,   0,  -5,  -5,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1,  1, -1, -1,
      -1, -1,  6,  7,  3,  2, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingptpd = {s_ptpd, 3,       /* "fudged" coordinates */
   { -9,  -5,  -2,  -5,   9,   5,   2,   5},
   {  0,   2,   0,  -2,   0,  -2,   0,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  6,  7,  4, -1,
      -1, -1,  3, -1, -1,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicethingptpd = {s_ptpd, 3,       /* "true" coordinates */
   {-10,  -6,  -2,  -6,  10,   6,   2,   6},
   {  0,   2,   0,  -2,   0,  -2,   0,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  6,  7,  4, -1,
      -1, -1,  3, -1, -1,  5, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbigdmd = {sbigdmd, 3,   /* used for both --
                                                symmetric and safe for press/truck */
   { -9,  -5,   0,   0,   5,   9,   9,   5,   0,   0,  -5,  -9},
   {  2,   2,   6,   2,   2,   2,  -2,  -2,  -6,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1,  0,  1, -1,  3,  4,  5, -1,
      -1, 11, 10, -1,  9,  7,  6, -1,
      -1, -1, -1, -1,  8, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing1x3dmd = {s1x3dmd, 3,   /* used for both --
                                                symmetric and safe for press/truck */
   {-13,  -9,  -5,   0,  13,   9,   5,   0},
   {  0,   0,   0,   2,   0,   0,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2, -1,  3,  6,  5,  4,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3x1dmd = {s3x1dmd, 3,   /* used for both --
                                                symmetric and safe for press/truck */
   {-10,  -6,  -2,   0,  10,   6,   2,   0},
   {  0,   0,   0,   5,   0,   0,   0,  -5}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  3, -1, -1, -1,
      -1,  0,  1,  2,  6,  5,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing2x1dmd = {s_2x1dmd, 3,   // used for both --
                                              // symmetric and safe for press/truck
   { -6,  -2,   0,   6,   2,   0},
   {  0,   0,   5,   0,   0,  -5}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1,  0,  1,  4,  3, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  5, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingdeep2x1dmd = {sdeep2x1dmd, 3,   // used for both --
                                                     // symmetric and safe for press/truck
   { -6,  -2,   0,   2,   6,   6,   2,   0,  -2,  -6},
   {  2,   2,   7,   2,   2,  -2,  -2,  -7,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  2, -1, -1, -1,
      -1, -1,  0,  1,  3,  4, -1, -1,
      -1, -1,  9,  8,  6,  5, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3dmd = {s3dmd, 3,   /* used for both --
                                            symmetric and safe for press/truck */
   { -7,   0,   7,  10,   6,   2,   7,   0,  -7, -10,  -6,  -2},
   {  5,   5,   5,   0,   0,   0,  -5,  -5,  -5,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1,  2, -1, -1,
      -1,  9, 10, 11,  5,  4,  3, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  8, -1,  7,  6, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing4dmd = {s4dmd, 3,   /* used for both --
                                            symmetric and safe for press/truck */
   {-11,  -5,   5,  11,  14,  10,   6,   2,  11,   5,  -5, -11, -14, -10,  -6,  -2},
   {  5,   5,   5,   5,   0,   0,   0,   0,  -5,  -5,  -5,  -5,   0,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1, -1, -1,  2,  3, -1,
      12, 13, 14, 15,  7,  6,  5,  4,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, 11, 10, -1, -1,  9,  8, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3mdmd = {s_3mdmd, 3,   /* used for both --
                                               symmetric and safe for press/truck */
   {-11,   0,  11,  13,   9,   5,  11,   0, -11, -13,  -9,  -5},
   {  5,   2,   5,   0,   0,   0,  -5,  -2,  -5,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0, -1, -1, -1, -1,  2, -1,
       9, 10, 11, -1,  1,  5,  4,  3,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1,  8, -1, -1, -1, -1,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3mptpd = {s_3mptpd, 3,   /* used for both --
                                                 symmetric and safe for press/truck */
   {-11,   0,  11,  14,   6,   2,  11,   0, -11, -14,  -6,  -2},
   {  2,   5,   2,   0,   0,   0,  -2,  -5,  -2,   0,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  1,  2, -1, -1,
       9,  0, 10, 11,  5,  4,  2,  3,
      -1,  8, -1, -1, -1, -1,  6, -1,
      -1, -1, -1, -1,  7,  6, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingshort6 = {s_short6, 3,   /* used for both --
                                                 symmetric and safe for press/truck */
   { -2,   0,   2,   2,   0,  -2},
   {  2,   6,   2,  -2,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1,  1, -1, -1, -1,
      -1, -1, -1,  0,  2, -1, -1, -1,
      -1, -1, -1,  5,  3, -1, -1, -1,
      -1, -1, -1, -1,  4, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec nicethingphan = {s_c1phan, 3,   /* "true" coordinates for mirroring --
                                   actually, the truck distances are greatly exaggerated. */
   {-10,  -6,  -2,  -6,   6,  10,   6,   2,  10,   6,   2,   6,  -6, -10,  -6,  -2},
   {  6,  10,   6,   2,  10,   6,   2,   6,  -6, -10,  -6,  -2, -10,  -6,  -2,  -6}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  1, -1, -1,  4, -1, -1,
      -1,  0, -1,  2,  7, -1,  5, -1,
      -1, -1,  3, -1, -1,  6, -1, -1,
      -1, -1, 14, -1, -1, 11, -1, -1,
      -1, 13, -1, 15, 10, -1,  8, -1,
      -1, -1, 12, -1, -1,  9, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingphan = {s_c1phan, 3,   /* "fudged" coordinates -- only truck within each 1x2 is allowed */
   { -6,  -3,  -2,  -3,   4,  15,   4,  11,   7,   4,   3,   4,  -5, -14,  -5, -10},
   {  4,  13,   4,   9,   7,   4,   3,   4,  -5, -14,  -5, -10,  -6,  -7,  -2,  -7}, {
      -1, -1, -1,  1, -1, -1, -1, -1,
      -1, -1, -1,  3, -1, -1, -1, -1,
      -1, -1,  0,  2, -1,  4,  7,  5,
      -1, -1, -1, -1, -1,  6, -1, -1,
      -1, -1, 14, -1, -1, -1, -1, -1,
      13, 15, 12, -1, 10,  8, -1, -1,
      -1, -1, -1, -1, -1, 11, -1, -1,
      -1, -1, -1, -1, -1,  9, -1, -1}};

static coordrec nicethingglass = {s_hrglass, 3,   /* "true" coordinates for mirroring --
                                   actually, the truck distances are greatly exaggerated. */
   { -5,   5,   5,   0,   5,  -5,  -5,   0},
   {  5,   5,   0,   2,  -5,  -5,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1, -1,  1, -1, -1,
      -1, -1,  6, -1,  3,  2, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingglass = {s_hrglass, 3,   /* "fudged" coordinates -- only truck by center 2 is allowed */
   { -4,   5,   5,   0,   4,  -5,  -5,   0},
   {  5,   5,   0,   2,  -5,  -5,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0, -1,  1, -1, -1,
      -1, -1,  6, -1,  3,  2, -1, -1,
      -1, -1, -1, -1,  7, -1, -1, -1,
      -1, -1,  5, -1, -1,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingdglass = {s_dhrglass, 3,   /* used for both -- symmetric and safe for press/truck */
   {-10,  10,   5,   0,  10, -10,  -5,   0},
   {  2,   2,   0,   2,  -2,  -2,   0,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  6, -1,  3,  2,  1, -1,
      -1,  5, -1, -1,  7, -1,  4, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing4x4 = {s4x4, 3,
   {  6,   6,   6,   2,   6,   2,  -2,   2,  -6,  -6,  -6,  -2,  -6,  -2,   2,  -2},
   {  6,   2,  -2,   2,  -6,  -6,  -6,  -2,  -6,  -2,   2,  -2,   6,   6,   6,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, 12, 13, 14,  0, -1, -1,
      -1, -1, 10, 15,  3,  1, -1, -1,
      -1, -1,  9, 11,  7,  2, -1, -1,
      -1, -1,  8,  6,  5,  4, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing4x5 = {s4x5, 3,
   { -8,  -4,   0,   4,   8,   8,   4,   0,  -4,  -8,   8,   4,   0,  -4,  -8,  -8,  -4,   0,   4,   8},
   {  6,   6,   6,   6,   6,   2,   2,   2,   2,   2,  -6,  -6,  -6,  -6,  -6,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0,  1,  2,  3,  4, -1,
      -1, -1,  9,  8,  7,  6,  5, -1,
      -1, -1, 15, 16, 17, 18, 19, -1,
      -1, -1, 14, 13, 12, 11, 10, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing4x6 = {s4x6, 3,
   {-10,  -6,  -2,   2,   6,  10,  10,   6,   2,  -2,  -6, -10,  10,   6,   2,  -2,  -6, -10, -10,  -6,  -2,   2,   6,  10},
   {  6,   6,   6,   6,   6,   6,   2,   2,   2,   2,   2,   2,  -6,  -6,  -6,  -6,  -6,  -6,  -2,  -2,  -2,  -2,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0,  1,  2,  3,  4,  5, -1,
      -1, 11, 10,  9,  8,  7,  6, -1,
      -1, 18, 19, 20, 21, 22, 23, -1,
      -1, 17, 16, 15, 14, 13, 12, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thing3oqtg = {s3oqtg, 3,
   { -8,   0,   8,  12,   8,   4,   0,  12,   8,   4,   8,   0,  -8, -12,  -8,  -4,   0, -12,  -8,  -4},
   {  6,   6,   6,   2,   2,   2,   2,  -2,  -2,  -2,  -6,  -6,  -6,  -2,  -2,  -2,  -2,   2,   2,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1,  0, -1,  1, -1,  2, -1,
      -1, 17, 18, 19,  6,  5,  4,  3,
      -1, 13, 14, 15, 16,  9,  8,  7,
      -1, -1, 12, -1, 11, -1, 10, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbigh = {sbigh, 3,
   { -9,  -9,  -9,  -9,  -6,  -2,   9,   9,   9,   9,   6,   2},
   {  6,   2,  -2,  -6,   0,   0,  -6,  -2,   2,   6,   0,   0}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1,  0, -1, -1, -1, -1,  9, -1,
      -1,  1,  4,  5, 11, 10,  8, -1,
      -1,  2, -1, -1, -1, -1,  7, -1,
      -1,  3, -1, -1, -1, -1,  6, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbigbigx = {sbigbigx, 4,
   {-19, -15, -11,  -7,  -2,  -2,  -2,  -2,  19,  15,  11,   7,   2,   2,   2,   2},
   {  0,   0,   0,   0,   6,   2,  -2,  -6,   0,   0,   0,   0,  -6,  -2,   2,   6}, {
      -1, -1, -1, -1, -1, -1, -1,  4, 15, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2,  3,  5, 14, 11, 10,  9,  8, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  6, 13, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  7, 12, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbigbigh = {sbigbigh, 4,
   {-17, -17, -17, -17, -14, -10,  -6,  -2,  17,  17,  17,  17,  14,  10,   6,   2},
   {  6,   2,  -2,  -6,   0,   0,   0,   0,  -6,  -2,   2,   6,   0,   0,   0,   0}, {
      -1, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1, -1,
      -1, -1, -1,  1,  4,  5,  6,  7, 15, 14, 13, 12, 10, -1, -1, -1,
      -1, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, -1,  9, -1, -1, -1,
      -1, -1, -1,  3, -1, -1, -1, -1, -1, -1, -1, -1,  8, -1, -1, -1}};


static coordrec thingbigbone = {sbigbone, 3,
   {-14, -10,  -6,  -2,  10,  14,  14,  10,   6,   2, -10, -14},
   {  2,   2,   0,   0,   2,   2,  -2,  -2,   0,   0,  -2,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
       0,  1,  2,  3,  9,  8,  4,  5,
      11, 10, -1, -1, -1, -1,  7,  6,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingbigx = {sbigx, 4,
   {-17, -13,  -9,  -5,   0,   0,  17,  13,   9,   5,   0,   0},
   {  0,   0,   0,   0,   6,   2,   0,   0,   0,   0,  -6,  -2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1,  2,  3, -1,  5,  9,  8,  7,  6, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, 11, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingdeepxwv = {sdeepxwv, 4,
   {-10,  -6,  -2,  -2,  -2,  -2,  10,   6,   2,   2,   2,   2},
   {  0,   0,   6,   2,  -2,  -6,   0,   0,  -6,  -2,   2,   6}, {
      -1, -1, -1, -1, -1, -1, -1,  2, 11, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1,  0,  1,  3, 10,  7,  6, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  4,  9, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1,  5,  8, -1, -1, -1, -1, -1, -1, -1}};

static coordrec thingblob = {s_bigblob, 3,
   { -2,   2,   2,   6,   6,   2,  10,  10,   6,   6,   2,   2,   2,  -2,  -2,  -6,  -6,  -2, -10, -10,  -6,  -6,  -2,  -2},
   { 10,  10,   6,   6,   2,   2,   2,  -2,  -2,  -6,  -6,  -2, -10, -10,  -6,  -6,  -2,  -2,  -2,   2,   2,   6,   6,   2}, {
      -1, -1, -1, -1, -1, -1, -1, -1,
      -1, -1, -1,  0,  1, -1, -1, -1,
      -1, -1, 21, 22,  2,  3, -1, -1,
      -1, 19, 20, 23,  5,  4,  6, -1,
      -1, 18, 16, 17, 11,  8,  7, -1,
      -1, -1, 15, 14, 10,  9, -1, -1,
      -1, -1, -1, 13, 12, -1, -1, -1,
      -1, -1, -1, -1, -1, -1, -1, -1}};



#define NOBIT(otherbits) { otherbits, otherbits, otherbits, otherbits }

#define WESTBIT(otherbits) { ID2_BEAU | otherbits, ID2_TRAILER | otherbits, ID2_BELLE | otherbits, ID2_LEAD | otherbits }

#define EASTBIT(otherbits) { ID2_BELLE | otherbits, ID2_LEAD | otherbits, ID2_BEAU | otherbits, ID2_TRAILER | otherbits }

#define NORTHBIT(otherbits) { ID2_LEAD | otherbits, ID2_BEAU | otherbits, ID2_TRAILER | otherbits, ID2_BELLE | otherbits }

#define SOUTHBIT(otherbits) { ID2_TRAILER | otherbits, ID2_BELLE | otherbits, ID2_LEAD | otherbits, ID2_BEAU | otherbits }

#define NWBITS(otherbits) { ID2_LEAD   |ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BELLE | otherbits, ID2_LEAD   |ID2_BELLE | otherbits }

#define SWBITS(otherbits) { ID2_TRAILER|ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BELLE | otherbits, ID2_LEAD   |ID2_BELLE | otherbits, ID2_LEAD   |ID2_BEAU  | otherbits }

#define SEBITS(otherbits) { ID2_TRAILER|ID2_BELLE | otherbits, ID2_LEAD   |ID2_BELLE | otherbits, ID2_LEAD   |ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BEAU  | otherbits }

#define NEBITS(otherbits) { ID2_LEAD   |ID2_BELLE | otherbits, ID2_LEAD   |ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BEAU  | otherbits, ID2_TRAILER|ID2_BELLE | otherbits }



static id_bit_table id_bit_table_1x2[] = {
   WESTBIT(0),
   EASTBIT(0)};

static id_bit_table id_bit_table_2x2[] = {
   NWBITS(0),
   NEBITS(0),
   SEBITS(0),
   SWBITS(0)};

static id_bit_table id_bit_table_2x4[] = {
   NWBITS(ID2_END),
   NEBITS(ID2_CENTER|ID2_CTR4),
   NWBITS(ID2_CENTER|ID2_CTR4),
   NEBITS(ID2_END),
   SEBITS(ID2_END),
   SWBITS(ID2_CENTER|ID2_CTR4),
   SEBITS(ID2_CENTER|ID2_CTR4),
   SWBITS(ID2_END)};

static id_bit_table id_bit_table_2x5[] = {
   NORTHBIT(0),
   NORTHBIT(0),
   NORTHBIT(0),
   NORTHBIT(0),
   NORTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0)};

static id_bit_table id_bit_table_2x7[] = {
   NORTHBIT(0),
   NORTHBIT(0),
   NORTHBIT(0),
   NORTHBIT(ID2_CTR2),
   NORTHBIT(0),
   NORTHBIT(0),
   NORTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(ID2_CTR2),
   SOUTHBIT(0),
   SOUTHBIT(0),
   SOUTHBIT(0)};


/* This table is accepted in any circumstances. */
/* Note that the people marked "ID2_END" won't actually identify
   themselves as ends.  That is just to make "center 4" work. */
static id_bit_table id_bit_table_2x6[] = {  
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_CTR4),
   NORTHBIT(ID2_CTR4),
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_END),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_CTR4),
   SOUTHBIT(ID2_CTR4),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_END)};

/* This table is only accepted if the population is a parallelogram. */
id_bit_table id_bit_table_2x6_pg[] = {
   NWBITS(ID2_OUTRPAIRS),
   NEBITS(ID2_OUTRPAIRS),
   NWBITS(ID2_CTR4),
   NEBITS(ID2_CTR4),
   NWBITS(ID2_OUTRPAIRS),
   NEBITS(ID2_OUTRPAIRS),
   SEBITS(ID2_OUTRPAIRS),
   SWBITS(ID2_OUTRPAIRS),
   SEBITS(ID2_CTR4),
   SWBITS(ID2_CTR4),
   SEBITS(ID2_OUTRPAIRS),
   SWBITS(ID2_OUTRPAIRS)};

static id_bit_table id_bit_table_2x8[] = {  
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_CTR4),
   NORTHBIT(ID2_CTR4),
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_END),
   NORTHBIT(ID2_END),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_CTR4),
   SOUTHBIT(ID2_CTR4),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_END),
   SOUTHBIT(ID2_END)};

static id_bit_table id_bit_table_1x4[] = {
   WESTBIT(ID2_END),
   EASTBIT(ID2_CENTER),
   EASTBIT(ID2_END),
   WESTBIT(ID2_CENTER)};

static id_bit_table id_bit_table_dmd[] = {
   NOBIT(ID2_END),
   NORTHBIT(ID2_CENTER),
   NOBIT(ID2_END),
   SOUTHBIT(ID2_CENTER)};

static id_bit_table id_bit_table_bone6[] = {
   NORTHBIT(0),
   NORTHBIT(0),
   /* We mark triangle points as trailers, in case we need to identify them in a 2x2 box,
       as in "leads start wheel the ocean". */
   {ID2_BEAU|ID2_TRAILER,     ID2_TRAILER,             ID2_BELLE|ID2_TRAILER,  ID2_LEAD},
   SOUTHBIT(0),
   SOUTHBIT(0),
   {ID2_BELLE|ID2_TRAILER,    ID2_LEAD,                ID2_BEAU|ID2_TRAILER,   ID2_TRAILER}};

static id_bit_table id_bit_table_short6[] = {
   NWBITS(ID2_CENTER),
   NOBIT(ID2_END),
   NEBITS(ID2_CENTER),
   SEBITS(ID2_CENTER),
   NOBIT(ID2_END),
   SWBITS(ID2_CENTER)};

static id_bit_table id_bit_table_1x6[] = {
   NOBIT(ID2_OUTRPAIRS| ID2_OUTR2| ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS| ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_CTR2     | ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_OUTRPAIRS| ID2_OUTR2| ID2_NCTR1X4),
   NOBIT(ID2_OUTRPAIRS| ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_CTR2     | ID2_CTR4 | ID2_CTR1X4)};

static id_bit_table id_bit_table_1x8[] = {
   WESTBIT(ID2_OUTR6| ID2_OUTR2| ID2_NCTR1X6| ID2_NCTR1X4| ID2_OUTRPAIRS),
   EASTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_NCTR1X4| ID2_OUTRPAIRS),
   EASTBIT(ID2_CTR2 | ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4),
   WESTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4),
   EASTBIT(ID2_OUTR6| ID2_OUTR2| ID2_NCTR1X6| ID2_NCTR1X4| ID2_OUTRPAIRS),
   WESTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_NCTR1X4| ID2_OUTRPAIRS),
   WESTBIT(ID2_CTR2 | ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4),
   EASTBIT(ID2_OUTR6| ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4 | ID2_CTR4)};

/* Used only if center 6 are contiguous, then a gap, then isolated people. */
static id_bit_table id_bit_table_1x10[] = {
   NOBIT(ID2_OUTR2| ID2_NCTR1X6| ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(ID2_CTR6 | ID2_CTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4  | ID2_CTR4),
   NOBIT(ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4  | ID2_CTR4),
   NOBIT(ID2_OUTR2| ID2_NCTR1X6| ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(ID2_CTR6 | ID2_CTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4  | ID2_CTR4),
   NOBIT(ID2_CTR6 | ID2_CTR1X6 | ID2_CTR1X4  | ID2_CTR4)};

static id_bit_table id_bit_table_qtag[] = {
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR2 | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_CTR6  | ID2_CTR2),
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_END    | ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_CTR6  | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR2 | ID2_OUTR6),
   NOBIT(ID2_CENTER | ID2_CTR4      | ID2_CTR1X4  | ID2_CTR6  | ID2_CTR2)};

static id_bit_table id_bit_table_2stars[] = {
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_CTR2),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_CTR2)};

static id_bit_table id_bit_table_ptpd[] = {
   {ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2,           ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2},
   {ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU,   ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE},
   {ID2_CTR2|ID2_CTR6|ID2_BEAU,     ID2_CTR2|ID2_CTR6|ID2_TRAILER, ID2_CTR2|ID2_CTR6|ID2_BELLE,    ID2_CTR2|ID2_CTR6|ID2_LEAD},
   {ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE,  ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU},
   {ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2,           ID2_OUTR6|ID2_OUTR2,            ID2_OUTR6|ID2_OUTR2},
   {ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE,  ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU},
   {ID2_CTR2|ID2_CTR6|ID2_BELLE,    ID2_CTR2|ID2_CTR6|ID2_LEAD,    ID2_CTR2|ID2_CTR6|ID2_BEAU,     ID2_CTR2|ID2_CTR6|ID2_TRAILER},
   {ID2_OUTR6|ID2_CTR6|ID2_LEAD,    ID2_OUTR6|ID2_CTR6|ID2_BEAU,   ID2_OUTR6|ID2_CTR6|ID2_TRAILER, ID2_OUTR6|ID2_CTR6|ID2_BELLE}};

static id_bit_table id_bit_table_crosswave[] = {
   WESTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_OUTR2|ID2_NCTRDMD| ID2_NCTR1X4),
   EASTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_CTR6 |ID2_CTRDMD | ID2_NCTR1X4),
   NORTHBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6|ID2_CTR6 |ID2_NCTRDMD| ID2_CTR1X4),
   SOUTHBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2 |ID2_CTR6 |ID2_CTRDMD | ID2_CTR1X4),
   EASTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_OUTR2|ID2_NCTRDMD| ID2_NCTR1X4),
   WESTBIT( ID2_END   |ID2_OUTRPAIRS|ID2_OUTR6|ID2_CTR6 |ID2_CTRDMD | ID2_NCTR1X4),
   SOUTHBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6|ID2_CTR6 |ID2_NCTRDMD| ID2_CTR1X4),
   NORTHBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2 |ID2_CTR6 |ID2_CTRDMD | ID2_CTR1X4)};

static id_bit_table id_bit_table_gal[] = {
   NOBIT(ID2_END),
   NWBITS(ID2_CENTER|ID2_CTR4),
   NOBIT(ID2_END),
   NEBITS(ID2_CENTER|ID2_CTR4),
   NOBIT(ID2_END),
   SEBITS(ID2_CENTER|ID2_CTR4),
   NOBIT(ID2_END),
   SWBITS(ID2_CENTER|ID2_CTR4)};

static id_bit_table id_bit_table_hrglass[] = {
   NOBIT(ID2_END            |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_END            |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_CENTER|ID2_CTR4|ID2_OUTR6|ID2_CTRDMD | ID2_OUTR2),
   NOBIT(ID2_CENTER|ID2_CTR4|ID2_CTR2 |ID2_CTRDMD | ID2_CTR6),
   NOBIT(ID2_END            |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_END            |ID2_OUTR6|ID2_NCTRDMD| ID2_CTR6),
   NOBIT(ID2_CENTER|ID2_CTR4|ID2_OUTR6|ID2_CTRDMD | ID2_OUTR2),
   NOBIT(ID2_CENTER|ID2_CTR4|ID2_CTR2 |ID2_CTRDMD | ID2_CTR6)};

static id_bit_table id_bit_table_dhrglass[] = {
   NORTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   NORTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   NOBIT(   ID2_CENTER|ID2_CTRDMD | ID2_OUTR6|ID2_NOUTR1X3),
   NORTHBIT(ID2_CENTER|ID2_CTRDMD | ID2_CTR2 |ID2_OUTR1X3),
   SOUTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   SOUTHBIT(ID2_END   |ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   NOBIT(   ID2_CENTER|ID2_CTRDMD | ID2_OUTR6|ID2_NOUTR1X3),
   SOUTHBIT(ID2_CENTER|ID2_CTRDMD | ID2_CTR2 |ID2_OUTR1X3)};

static id_bit_table id_bit_table_323[] = {
   NOBIT(  ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   NOBIT(  ID2_CTRDMD | ID2_OUTR6|ID2_OUTR1X3),
   NOBIT(  ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   EASTBIT(ID2_CTRDMD | ID2_CTR2 |ID2_NOUTR1X3),
   NOBIT(  ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   NOBIT(  ID2_CTRDMD | ID2_OUTR6|ID2_OUTR1X3),
   NOBIT(  ID2_NCTRDMD| ID2_OUTR6|ID2_OUTR1X3),
   WESTBIT(ID2_CTRDMD | ID2_CTR2 |ID2_NOUTR1X3)};

/* If the population is "common spot diamonds", the richer table below is used instead. */
static id_bit_table id_bit_table_bigdmd[] = {  
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6)};

/* This table is only accepted if the population is "common spot diamonds". */
id_bit_table id_bit_table_bigdmd_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6)};

static id_bit_table id_bit_table_bigptpd[] = {  
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_CTR2),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_CTR2),
   NOBIT(ID2_OUTR6),
   NOBIT(ID2_OUTR6)};

/* If the population is a distorted tidal line (2-4-2), the richer table below is used instead. */
static id_bit_table id_bit_table_bigbone[] = {  
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_NCTR1X4 | ID2_OUTR6)};

/* This table is only accepted if the population is a distorted tidal line (2-4-2). */
id_bit_table id_bit_table_bigbone_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTR1X4  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTR1X4 | ID2_OUTR6)};

/* If the population is a "common spot hourglass", the richer table below is used instead. */
static id_bit_table id_bit_table_bighrgl[] = {  
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6)};

/* This table is only accepted if the population is a "common spot hourglass". */
id_bit_table id_bit_table_bighrgl_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6)};

/* If the population is a "common spot distorted hourglass", the richer table below is used instead. */
static id_bit_table id_bit_table_bigdhrgl[] = {  
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_NCTRDMD | ID2_OUTR6)};

/* This table is only accepted if the population is a "common spot distorted hourglass". */
id_bit_table id_bit_table_bigdhrgl_wings[] = {  
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_CTR2),
   NOBIT(ID2_CTR4      | ID2_CTRDMD  | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6),
   NOBIT(ID2_OUTRPAIRS | ID2_NCTRDMD | ID2_OUTR6)};

/* If the population of a 3x4 is an "H", use this. */

id_bit_table id_bit_table_3x4_h[] = {
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4|ID2_OUTR1X3),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4|ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_CTR1X4 |ID2_OUTR1X3),
   NOBIT(ID2_CTR2  | ID2_CTR1X4 |ID2_NOUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4|ID2_OUTR1X3),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4|ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_CTR1X4 |ID2_OUTR1X3),
   NOBIT(ID2_CTR2  | ID2_CTR1X4 |ID2_NOUTR1X3)};

/* Else if the center 2x3 of a 3x4 is full, use this. */
id_bit_table id_bit_table_3x4_ctr6[] = {
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_CTR2  | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_CTR2  | ID2_CTR6)};

/* Else if the population is offset C/L/W, use this. */
id_bit_table id_bit_table_3x4_offset[] = {
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_CTR4      | ID2_CTR1X4),
   NOBIT(ID2_CTR2  | ID2_CTR4      | ID2_CTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_OUTRPAIRS | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_CTR4      | ID2_CTR1X4),
   NOBIT(ID2_CTR2  | ID2_CTR4      | ID2_CTR1X4)};

/* If all else fails, use this. */
static id_bit_table id_bit_table_3x4[] = {
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_CTR1X4),
   NOBIT(ID2_CTR2  | ID2_CTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_NCTR1X4),
   NOBIT(ID2_OUTR6 | ID2_CTR1X4),
   NOBIT(ID2_CTR2  | ID2_CTR1X4)};

/* This is only used if the center 2x3 is full. */
static id_bit_table id_bit_table_d3x4[] = {
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_CTR2  | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_CTR6),
   NOBIT(ID2_OUTR6 | ID2_OUTR2),
   NOBIT(ID2_CTR2  | ID2_CTR6)};

// Use only if center 1x4 is full.
id_bit_table id_bit_table_bigh[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_CTR1X4)};

/* If the population is a butterfly, this table is used. */
id_bit_table id_bit_table_butterfly[] = {
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4),
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4),
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4),
   NOBIT(ID2_END),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4)};

/* Otherwise, this table is used for 4x4's. */
/* The use of "ID2_NCTR1X4" is a crock, of course. */
id_bit_table id_bit_table_4x4[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR4)};

id_bit_table id_bit_table_525_nw[] = {
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_CTRDMD  | ID2_OUTR1X3),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CTR2  | ID2_CTRDMD  | ID2_NOUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_CTRDMD  | ID2_OUTR1X3),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CTR2  | ID2_CTRDMD  | ID2_NOUTR1X3)};

id_bit_table id_bit_table_525_ne[] = {
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_OUTR6 | ID2_CTRDMD  | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_CTR2  | ID2_CTRDMD  | ID2_NOUTR1X3),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_OUTR6 | ID2_CTRDMD  | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_OUTR6 | ID2_NCTRDMD | ID2_OUTR1X3),
   NOBIT(ID2_CTR2  | ID2_CTRDMD  | ID2_NOUTR1X3)};

id_bit_table id_bit_table_343_outr[] = {
   NOBIT(ID2_OUTR1X3),
   NOBIT(ID2_OUTR1X3),
   NOBIT(ID2_OUTR1X3),
   NOBIT(ID2_NOUTR1X3),
   NOBIT(ID2_NOUTR1X3),
   NOBIT(ID2_OUTR1X3),
   NOBIT(ID2_OUTR1X3),
   NOBIT(ID2_OUTR1X3),
   NOBIT(ID2_NOUTR1X3),
   NOBIT(ID2_NOUTR1X3)};

id_bit_table id_bit_table_343_innr[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4)};

id_bit_table id_bit_table_545_outr[] = {
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_NOUTR1X3|ID2_CTR1X4),
   NOBIT(ID2_NOUTR1X3|ID2_CTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_OUTR1X3 |ID2_NCTR1X4),
   NOBIT(ID2_NOUTR1X3|ID2_CTR1X4),
   NOBIT(ID2_NOUTR1X3|ID2_CTR1X4)};

id_bit_table id_bit_table_545_innr[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4)};

id_bit_table id_bit_table_3dmd_in_out[] = {
   NOBIT(ID2_END | ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(ID2_END | ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_CENTER | ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_END | ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(ID2_END | ID2_NCTR1X4),
   NOBIT(0),
   NOBIT(ID2_CENTER | ID2_CTR4 | ID2_CTR1X4),
   NOBIT(ID2_CENTER | ID2_CTR4 | ID2_CTR1X4)};

static id_bit_table id_bit_table_spindle[] = {
   NORTHBIT(ID2_CTR6 |ID2_OUTR6),
   NORTHBIT(ID2_CTR6 |ID2_CTR2),
   NORTHBIT(ID2_CTR6 |ID2_OUTR6),
   NOBIT   (ID2_OUTR2|ID2_OUTR6),
   SOUTHBIT(ID2_CTR6 |ID2_OUTR6),
   SOUTHBIT(ID2_CTR6 |ID2_CTR2),
   SOUTHBIT(ID2_CTR6 |ID2_OUTR6),
   NOBIT   (ID2_OUTR2|ID2_OUTR6)};

/* This is only accepted if the outer pairs are occupied. */
static id_bit_table id_bit_table_deepxwv[] = {
   NOBIT(ID2_OUTRPAIRS),
   NOBIT(ID2_OUTRPAIRS),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_OUTRPAIRS),
   NOBIT(ID2_OUTRPAIRS),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_CENTER),
   NOBIT(ID2_CENTER)};

static id_bit_table id_bit_table_rigger[] = {
   NWBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   NEBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   EASTBIT(ID2_OUTR2|ID2_END   |ID2_OUTRPAIRS),
   WESTBIT(ID2_CTR6 |ID2_END   |ID2_OUTRPAIRS),
   SEBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   SWBITS( ID2_CTR6 |ID2_CENTER|ID2_CTR4),
   WESTBIT(ID2_OUTR2|ID2_END   |ID2_OUTRPAIRS),
   EASTBIT(ID2_CTR6 |ID2_END   |ID2_OUTRPAIRS)};

static id_bit_table id_bit_table_1x3dmd[] = {
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_OUTR2| ID2_NCTRDMD),
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_CTR6|  ID2_NCTRDMD),
   NOBIT(   ID2_CENTER| ID2_CTR4|      ID2_OUTR6| ID2_CTR6|  ID2_CTRDMD),
   NORTHBIT(ID2_CENTER| ID2_CTR4|      ID2_CTR2|  ID2_CTR6|  ID2_CTRDMD),
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_OUTR2| ID2_NCTRDMD),
   NOBIT(   ID2_END|    ID2_OUTRPAIRS| ID2_OUTR6| ID2_CTR6|  ID2_NCTRDMD),
   NOBIT(   ID2_CENTER| ID2_CTR4|      ID2_OUTR6| ID2_CTR6|  ID2_CTRDMD),
   SOUTHBIT(ID2_CENTER| ID2_CTR4|      ID2_CTR2|  ID2_CTR6|  ID2_CTRDMD)};

static id_bit_table id_bit_table_2x1dmd[] = {
   WESTBIT(ID2_NCTRDMD | ID2_CTR1X4),
   EASTBIT(ID2_CTRDMD  | ID2_CTR1X4),
   NOBIT(  ID2_CTRDMD  | ID2_NCTR1X4),
   EASTBIT(ID2_NCTRDMD | ID2_CTR1X4),
   WESTBIT(ID2_CTRDMD  | ID2_CTR1X4),
   NOBIT(  ID2_CTRDMD  | ID2_NCTR1X4)};

static id_bit_table id_bit_table_3x1dmd[] = {
   WESTBIT(ID2_OUTR2| ID2_OUTR6| ID2_OUTRPAIRS| ID2_END| ID2_CTR1X6|  ID2_NCTRDMD | ID2_NCTR1X4),
   EASTBIT(ID2_CTR6|  ID2_OUTR6| ID2_OUTRPAIRS| ID2_END| ID2_CTR1X6|  ID2_NCTRDMD | ID2_CTR1X4),
   WESTBIT(ID2_CTR6|  ID2_CTR2|  ID2_CTR4|   ID2_CENTER| ID2_CTR1X6|  ID2_CTRDMD  | ID2_CTR1X4),
   NOBIT(  ID2_CTR6|  ID2_OUTR6| ID2_CTR4|   ID2_CENTER| ID2_NCTR1X6| ID2_CTRDMD  | ID2_NCTR1X4),
   EASTBIT(ID2_OUTR2| ID2_OUTR6| ID2_OUTRPAIRS| ID2_END| ID2_CTR1X6|  ID2_NCTRDMD | ID2_NCTR1X4),
   WESTBIT(ID2_CTR6|  ID2_OUTR6| ID2_OUTRPAIRS| ID2_END| ID2_CTR1X6|  ID2_NCTRDMD | ID2_CTR1X4),
   EASTBIT(ID2_CTR6|  ID2_CTR2|  ID2_CTR4|   ID2_CENTER| ID2_CTR1X6|  ID2_CTRDMD  | ID2_CTR1X4),
   NOBIT(  ID2_CTR6|  ID2_OUTR6| ID2_CTR4|   ID2_CENTER| ID2_NCTR1X6| ID2_CTRDMD  | ID2_NCTR1X4)};

/* If center diamond is fully occupied and outer diamonds have only points, use this. */
static id_bit_table id_bit_table_3dmd[] = {
   NOBIT(ID2_OUTR6| ID2_NCTRDMD|ID2_OUTR1X3),
   NOBIT(ID2_OUTR6| ID2_CTRDMD |ID2_OUTR1X3),
   NOBIT(ID2_OUTR6| ID2_NCTRDMD|ID2_OUTR1X3),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(ID2_CTR2|  ID2_CTRDMD |ID2_NOUTR1X3),
   NOBIT(ID2_OUTR6| ID2_NCTRDMD|ID2_OUTR1X3),
   NOBIT(ID2_OUTR6| ID2_CTRDMD |ID2_OUTR1X3),
   NOBIT(ID2_OUTR6| ID2_NCTRDMD|ID2_OUTR1X3),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(           ID2_NCTRDMD),
   NOBIT(ID2_CTR2|  ID2_CTRDMD |ID2_NOUTR1X3)};

/* If center 1x6 is fully occupied, use this.  It is external. */
id_bit_table id_bit_table_3dmd_ctr1x6[] = {
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4)};

/* If only center 1x4 is fully occupied, use this.  It is external. */
id_bit_table id_bit_table_3dmd_ctr1x4[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4),
   NOBIT(ID2_CTR1X4)};

// If center 1x4 is occupied, use this.
static id_bit_table id_bit_table_4dmd[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4|ID2_CTR4),
   NOBIT(ID2_CTR1X4|ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4|ID2_CTR4),
   NOBIT(ID2_CTR1X4|ID2_CTR4)};

// If center diamond has only centers and outer diamonds have only points, use this.
id_bit_table id_bit_table_4dmd_cc_ee[] = {
   NOBIT(ID2_END|ID2_NCTR1X4|ID2_OUTRPAIRS),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_END|ID2_NCTR1X4|ID2_OUTRPAIRS),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER|ID2_CTR1X4|ID2_CTR4),
   NOBIT(ID2_CENTER|ID2_CTR1X4|ID2_CTR4),
   NOBIT(ID2_END|ID2_NCTR1X4|ID2_OUTRPAIRS),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_END|ID2_NCTR1X4|ID2_OUTRPAIRS),
   NOBIT(0),
   NOBIT(0),
   NOBIT(ID2_CENTER|ID2_CTR1X4|ID2_CTR4),
   NOBIT(ID2_CENTER|ID2_CTR1X4|ID2_CTR4)};

/* If center diamond of triple point-to-point diamonds is fully occupied,
   along with inboard points of other diamonds, this lets us do "triple trade".
   It is external. */
id_bit_table id_bit_table_3ptpd[] = {
   NORTHBIT(ID2_OUTR2| ID2_NCTRDMD),
   NORTHBIT(ID2_CTR6|  ID2_CTRDMD),
   NORTHBIT(ID2_OUTR2| ID2_NCTRDMD),
   NOBIT   (ID2_OUTR2| ID2_NCTRDMD),
   EASTBIT (ID2_CTR6|  ID2_NCTRDMD),
   WESTBIT (ID2_CTR6|  ID2_CTRDMD),
   SOUTHBIT(ID2_OUTR2| ID2_NCTRDMD),
   SOUTHBIT(ID2_CTR6|  ID2_CTRDMD),
   SOUTHBIT(ID2_OUTR2| ID2_NCTRDMD),
   NOBIT   (ID2_OUTR2| ID2_NCTRDMD),
   WESTBIT (ID2_CTR6|  ID2_NCTRDMD),
   EASTBIT (ID2_CTR6|  ID2_CTRDMD)};

static id_bit_table id_bit_table_bone[] = {
   NORTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   NORTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   EASTBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6),
   WESTBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2),
   SOUTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   SOUTHBIT(ID2_END  |ID2_OUTRPAIRS|ID2_OUTR6),
   WESTBIT(ID2_CENTER|ID2_CTR4     |ID2_OUTR6),
   EASTBIT(ID2_CENTER|ID2_CTR4     |ID2_CTR2)};

// Use this only if center 1x6 is fully occupied.
id_bit_table id_bit_table_3x6_with_1x6[] = {
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X6 | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_NCTR1X4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X6  | ID2_CTR1X4 | ID2_CTR4)};

// Use this one if center 1x4 only.
static id_bit_table id_bit_table_3x6[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4)};

static id_bit_table id_bit_table_4x5[] = {
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_CTR1X4 | ID2_CTR4),
   NOBIT(ID2_NCTR1X4),
   NOBIT(ID2_NCTR1X4)};


#define XXX schema_nothing
#define Z schema_concentric
#define DL schema_concentric_diamond_line
#define CD schema_concentric_diamonds
#define OT schema_concentric_others
#define IL schema_intlk_lateral_6
#define IV schema_intlk_vertical_6
#define L6 schema_lateral_6
#define V6 schema_vertical_6
#define QL schema_in_out_quad
#define Q2 schema_in_out_12mquad
#define TL schema_in_out_triple
#define GS schema_grand_single_concentric
#define SI schema_single_concentric
#define S6 schema_conc_star16
#define S2 schema_conc_star12
#define B6 schema_conc_bar16
#define B2 schema_conc_bar12
#define G3 schema_3x3_concentric
#define C4 schema_4x4_cols_concentric
#define L4 schema_4x4_lines_concentric
#define BA schema_conc_bar
#define OO schema_conc_o
#define T6 schema_concentric_6_2_tgl
#define X6P schema_concentric_6p
#define X62 schema_concentric_6_2
#define X26 schema_concentric_2_6
#define B26 schema_concentric_big2_6
#define X24 schema_concentric_2_4
#define X42 schema_concentric_4_2
#define CK schema_checkpoint





/*                                                                            center_arity -------|
                                                                              mapelong --------|  |
                                                                             outer_rot -----|  |  |
                                                                      inner_rot ---------|  |  |  |
                                                outlimit -----|          bigsize ----|   |  |  |  |
     bigsetup    lyzer     maps                   inlimit -|  |  insetup  outsetup   |   |  |  |  |   */

cm_thing conc_init_table[] = {
   {s_ptpd,         Z, {0, 2, 4, 6,    3, 1, 7, 5},        4, 4, s1x4,     s2x2,     8,  0, 1, 9, 1,  0x2F5, schema_rev_checkpoint},
   {s_spindle,      Z, {7, 1, 3, 5,    6, 0, 2, 4},        4, 4, sdmd,     s2x2,     8,  0, 1, 1, 1,  0x2F5, schema_rev_checkpoint},
   {s_wingedstar,   Z, {0, 3, 4, 7,    1, 2, 5, 6},        4, 4, sdmd,     s1x4,     8,  0, 0, 9, 1,  0x2FA, schema_ckpt_star},
   {s_wingedstar,   Z, {1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s1x4,     sdmd,     8,  0, 1, 9, 1,  0x2F5, schema_conc_star},
   {s_wingedstar,   Z, {1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s1x4,     sdmd,     8,  0, 0, 9, 1,  0x2FA, schema_conc_star},
   {s_wingedstar,   Z, {1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s1x4,     s_star,   8,  0, 1, 9, 1,  0x2F5, schema_conc_star},
   {s_wingedstar,   Z, {1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s1x4,     s_star,   8,  0, 0, 9, 1,  0x2FA, schema_conc_star},
   {s_wingedstar12, Z, {11, 2, 3, 4, 9, 10, 5, 8,
                           0, 1, 6, 7},                    4, 4, s_star,   s1x4,     12, 1, 0, 1, 2,  0x2F5, schema_conc_star12},
   {s_wingedstar16, Z, {4, 2, 3, 5, 15, 6, 7, 14, 11, 13, 12, 10,
                           0, 1, 8, 9},                    4, 4, s_star,   s1x4,     16, 1, 0, 1, 3,  0x2F5, schema_conc_star16},
   {s_barredstar,   Z, {6, 9, 1, 4,    7, 8, 0, 2, 3, 5},  4, 6, s_star,   s2x3,     10, 1, 1, 2, 1,  0x2FB, schema_concentric},
   {s2x4,           Z, {6, 5, 1, 2,      7, 0, 3, 4},      2, 4, s1x2,     s2x2,     8,  0, 1, 1, 2,  0x2F7, schema_concentric_others},
   {s_qtag,         Z, {6, 7, 3, 2,      5, 0, 1, 4},      2, 4, s1x2,     s2x2,     8,  0, 1, 1, 2,  0x2FD, schema_concentric_others},
   {s2x6,           Z, {11, 0, 1, 10, 7, 4, 5, 6,
                                       9, 2, 3, 8},        4, 4, s2x2,     s2x2,     12, 1, 1, 2, 2,  0x2FB, TL},
   {sbigbone,       Z, {11, 0, 1, 10, 7, 4, 5, 6,
                                       2, 3, 8, 9},        4, 4, s2x2,     s1x4,     12, 1, 0, 1, 2,  0x2FD, TL},


   {sbigdmd,        Z, {0, 1, 10, 11, 4, 5, 6, 7,       8, 9, 2, 3},
             4, 4, s2x2,     s1x4,     12, 0, 1, 2, 2,  0x2F7, TL},



   {sbigdhrgl,      Z, {11, 0, 1, 10, 7, 4, 5, 6,       9, 2, 3, 8},
             4, 4, s2x2,     sdmd,     12, 1, 0, 1, 2,  0x2FD, TL},
   {sbighrgl,       Z, {11, 0, 1, 10, 7, 4, 5, 6,       8, 9, 2, 3},
             4, 4, s2x2,     sdmd,     12, 1, 1, 1, 2,  0x2FB, TL},
   {sbigrig,        Z, {0, 1, 3, 2, 9, 8, 6, 7,         11, 4, 5, 10},
             4, 4, s1x4,     s2x2,     12, 0, 1, 1, 2,  0x2F7, TL},
   {s4x4,           Z, {8, 6, 4, 5, 12, 13, 0, 14,      11, 15, 3, 7},
             4, 4, s1x4,     s2x2,     12, 0, 1, 2, 2,  0x2FD, TL},
   {s3x4,           Z, {9, 8, 6, 7, 0, 1, 3, 2,         10, -1, 4, -1},
             4, 4, s1x4,     sdmd,     12, 0, 0, 2, 2,  0x2FB, TL},

   {s1x3dmd,       TL, {-1, 0, -1, 1, -1, 5, -1, 4,     2, 3, 6, 7},
             4, 4, sdmd,     sdmd,     12, 1, 0, 1, 2,  0x1FD, XXX},
   {s_3mdmd,        Z, {8, -1, 0, -1, 6, -1, 2, -1,     11, 1, 5, 7},
             4, 4, s1x4,     sdmd,     12, 1, 0, 1, 2,  0x2FD, TL},
   {s_3mdmd,        TL, {8, 9, 0, 10, 6, 4, 2, 3,       11, 1, 5, 7},
             4, 4, sdmd,     sdmd,     12, 1, 0, 1, 2,  0x0FD, TL},
   {s_4mdmd,        QL, {11, 12, 0, 13, 8, 5, 3, 4,     14, 1, 15, 10, 6, 9, 7, 2},
             4, 8, sdmd,     s_ptpd,   16, 1, 0, 1, 2,  0x0FD, QL},

   {s_3mptpd,       Z, {9, -1, 10, -1, 4, -1, 3, -1,    7, 11, 1, 5},
             4, 4, s1x4,     sdmd,     12, 0, 1, 1, 2,  0x2F7, TL},
   {s_3mptpd,       TL, {9, 0, 10, 8, 4, 2, 3, 6,       7, 11, 1, 5},
             4, 4, sdmd,     sdmd,     12, 0, 1, 1, 2,  0x0F7, TL},
   {s_4mptpd,       QL, {12, 0, 13, 11, 5, 3, 4, 8,     1, 2, 6, 7, 9, 10, 14, 15},
             4, 8, sdmd,     s_qtag,   16, 0, 0, 1, 2,  0x0FE, QL},

   {s3dmd,          Z, {8, -1, 0, -1, 6, -1, 2, -1,     7, 11, 1, 5},
             4, 4, s1x4,     sdmd,     12, 1, 1, 1, 2,  0x2FB, TL},
   {s3dmd,          TL, {8, 9, 0, 10, 6, 4, 2, 3,       7, 11, 1, 5},
             4, 4, sdmd,     sdmd,     12, 1, 1, 1, 2,  0x0FB, TL},
   {s4dmd,          QL, {11, 12, 0, 13, 8, 5, 3, 4,     1, 2, 6, 7, 9, 10, 14, 15},
             4, 8, sdmd,     s_qtag,   16, 1, 0, 1, 2,  0x0FD, QL},
   /* This one must not be used for analysis ("going in").  The reason is that we go in
      with stuff separated left-to-right.  But this is top-to-bottom.  Wasn't the
      elongation supposed to take care of all this???? */
   {s3x8,            Z, {17, 16, 14, 15, 2, 3, 5, 4,    20, 21, 23, 22, 8, 9, 11, 10},
             4, 8, s1x4,     s1x8,     24, 0, 0, 2, 2,  0x2FB, QL},


   {s3x8,           Z, {19, 20, 0, 1, 21, 18, 13, 9, 6, 7, 8, 12,     22, 23, 10, 11},
             6, 4, s2x3,     s1x4,     24, 1, 0, 1, 2,  0x2FD, TL},
   /* No!  This conflicts with the next one.  Replace the missing center z peel the top
      in t46t when this gets fixed.
   {s_343,          Z, {7, 6, 5, -1, -1, -1, -1, -1, -1, 2, 1, 0,    8, 9, 3, 4},
             6, 4, s2x3,     s1x4,     10, 0, 0, 2, 2,  0x2FB, TL},
   */
   {s_crosswave,    Z, {-1, 0, -1, -1, 1, -1, -1, 5, -1, -1, 4, -1,       6, 7, 2, 3},
             6, 4, s2x3,     s1x4,      8, 1, 1, 1, 2,  0x2FB, TL},

   {s3ptpd,         Z, {9, -1, 10, -1, 4, -1, 3, -1,    11, 1, 5, 7},
             4, 4, s1x4,     sdmd,     12, 0, 0, 1, 2,  0x2FE, TL},

   {s3ptpd,         TL, {9, 0, 10, 8, 4, 2, 3, 6,       11, 1, 5, 7},
             4, 4, sdmd,     sdmd,     12, 0, 0, 1, 2,  0x0FE, TL},
   {s4ptpd,         QL, {12, 0, 13, 11, 5, 3, 4, 8,     14, 1, 15, 10, 6, 9, 7, 2},
             4, 8, sdmd,     s_ptpd,   16, 0, 0, 1, 2,  0x0FE, QL},

   {s_crosswave,    Z, {-1, 0, -1, 1, -1, 5, -1, 4,     6, 7, 2, 3},
             4, 4, sdmd,     s1x4,     12, 1, 1, 1, 2,  0x2FB, TL},
   {s1x8,           Z, {-1, 0, -1, 1, -1, 5, -1, 4,     3, 2, 7, 6},
             4, 4, sdmd,     s1x4,     12, 1, 0, 1, 2,  0x2F5, TL},
   {s_rigger,       Z, {-1, 6, -1, 7, -1, 3, -1, 2,     0, 1, 4, 5},
             4, 4, sdmd,     s2x2,     12, 1, 0, 1, 2,  0x2F5, TL},
   {s_rigger,       Z, {-1, 6, -1, -1, 7, -1, -1, 3, -1, -1, 2, -1,       0, 1, 4, 5},
             6, 4, s2x3,     s2x2,     12, 1, 0, 1, 2,  0x2FD, TL},
   {s4x6,           Z, {17, 18, 0, 11, 12, 23, 5, 6,    10, 9, 8, 7, 22, 21, 20, 19},
             4, 8, s1x4,     s2x4,     24, 1, 0, 1, 2,  0x2FD, QL},
   {sbigrig,        Z, {0, 1, 3, 2, 9, 8, 6, 7,         -1, 11, 4, -1, -1, 5, 10, -1},
             4, 8, s1x4,     s2x4,     12, 0, 1, 1, 2,  0x2F7, QL},
   {s4x6,           Z, {18, 11, 10, 19, 22, 7, 6, 23,
                            15, 20, 9, 2, 3, 8, 21, 24},   4, 8, s2x2,     s2x4,     16, 1, 1, 1, 2,  0x2FB, QL},
   {s2x4,           Z, {6, 1, 2, 5,    -1, -1, -1, -1, -1, 3, 4, -1,
                           -1, -1, -1, -1, -1, 7, 0, -1},  4,16, s2x2,     s4x4,     8,  1, 1, 1, 1,  0x2FB, schema_conc_o},
   {s_qtag,         Z, {6, 7, 2, 3,    -1, 0, 1, -1, -1, -1, -1, -1,
                           -1, 4, 5, -1, -1, -1, -1, -1},  4,16, s1x4,     s4x4,     8,  0, 1, 1, 1,  0x2FD, schema_conc_o},
   {s_bone,         Z, {6, 7, 2, 3,    -1, -1, -1, -1, -1, 1, 4, -1,
                           -1, -1, -1, -1, -1, 5, 0, -1},  4,16, s1x4,     s4x4,     8,  0, 1, 1, 1,  0x2F7, schema_conc_o},
   {s_hrglass,      Z, {6, 3, 2, 7,    -1, 0, 1, -1, -1, -1, -1, -1,
                           -1, 4, 5, -1, -1, -1, -1, -1},  4,16, sdmd,     s4x4,     8,  0, 1, 1, 1,  0x2FD, schema_conc_o},
   {s_dhrglass,     Z, {6, 3, 2, 7,    -1, -1, -1, -1, -1, 1, 4, -1,
                           -1, -1, -1, -1, -1, 5, 0, -1},  4,16, sdmd,     s4x4,     8,  0, 1, 1, 1,  0x2F7, schema_conc_o},
   {s_bone,         Z, {6, 7, 2, 3,
                              -1, 0, 1, -1, -1, 4, 5, -1}, 4, 8, s1x4,     s2x4,      8, 0, 0, 1, 1,  0x2FE, schema_concentric},
   {sbigdmd,        Z, {8, 9, 2, 3,
                                0, 1, 4, 5, 6, 7, 10, 11}, 4, 8, s1x4,     s2x4,     12, 1, 0, 1, 1,  0x2FD, schema_concentric},


   {s3x4,           Z, {10, 11, 4, 5,
                                0, 1, 2, 3, 6, 7, 8, 9},   4, 8, s1x4,     s2x4,     12, 0, 0, 1, 1,  0x2FB, schema_concentric},

   {s3x6,           Z, {15, 16, 17, 6, 7, 8,     0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14},
             6, 12, s1x6,    s2x6,     18, 0, 0, 1, 1,  0x2FB, schema_concentric},


   {s4x4,           Z, {11, 15, 3, 7,    12, 13, 14, 0, 4, 5, 6, 8},
             4, 8, s2x2,     s2x4,     16, 1, 0, 2, 1,  0x2F7, schema_concentric},
   {s4x4,           Z, {15, 3, 7, 11,    12, 13, 14, 0, 4, 5, 6, 8},
             4, 8, s2x2,     s2x4,     16, 0, 0, 2, 1,  0x2FB, schema_concentric},

   {s2x6,           Z, {9, 2, 3, 8,      0, 1, 4, 5, 6, 7, 10, 11},
             4, 8, s2x2,     s2x4,     12, 1, 0, 9, 1,  0x2FD, schema_concentric},
   {s2x6,           Z, {2, 3, 8, 9,      0, 1, 4, 5, 6, 7, 10, 11},
             4, 8, s2x2,     s2x4,     12, 0, 0, 9, 1,  0x2FE, schema_concentric},
   {s3dmd,         G3, {9, 10, 11, 3, 4, 5,          0, 1, 2, 6, 7, 8},
             6, 6, s1x6,     s2x3,     12, 0, 0, 2, 1,  0x0FB, schema_concentric},
   {s3x4,           Z, {11, 5,           0, 1, 2, 3, 6, 7, 8, 9},
             2, 8, s1x2,     s2x4,     12, 0, 0, 2, 1,  0x0FB, schema_concentric},

   {s3x8,          G3, {21, 22, 23, 9, 10, 11,       19, 20, 0, 7, 8, 12},
             6, 6, s1x6,     s2x3,     24, 0, 1, 1, 1,  0x0F7, schema_concentric},

   {s4dmd,         C4, {12, 13, 15, 14, 4, 5, 7, 6,      0, 1, 2, 3, 8, 9, 10, 11},
             8, 8, s1x8,     s2x4,     16, 0, 0, 2, 1,  0x0FB, schema_concentric},
   {s4dmd,         L4, {12, 13, 15, 14, 4, 5, 7, 6,      0, 1, 2, 3, 8, 9, 10, 11},
             8, 8, s1x8,     s2x4,     16, 0, 0, 2, 1,  0x100, XXX},


   {sbigrig,        Z, {11, 4, 5, 10,
                           0, 1, 3, 2, 6, 7, 9, 8},        4, 8, s2x2,     s1x8,     12, 1, 0, 9, 1,  0x2F5, schema_concentric},
   {sbigrig,        Z, {4, 5, 10, 11,
                           0, 1, 3, 2, 6, 7, 9, 8},        4, 8, s2x2,     s1x8,     12, 0, 0, 9, 1,  0x2FA, schema_concentric},
   /*  These were used in a misguided attempt to make "triple cast" work.
   {s2x6,           Z, {1, 2, 3, 4, 7, 8, 9, 10,
                                        11, 0, 5, 6},      8, 4, s2x4,     s2x2,     12, 0, 1, 9, 1,  0x2F7, schema_concentric},
   {s2x6,           Z, {1, 2, 3, 4, 7, 8, 9, 10,
                                        0, 5, 6, 11},      8, 4, s2x4,     s2x2,     12, 0, 0, 9, 1,  0x2FD, schema_concentric},
   */

   {s_bone6,        Z, {5, 2,    4, 0, 1, 3},              2, 4, s1x2,     s2x2,     6,  0, 1, 2, 1,  0x2F7, schema_concentric},
   {s_bone6,      X6P, {5, 2,    0, 1, 3, 4},              2, 4, s1x2,     s2x2,     6,  0, 0, 1, 1,  0x0FE, schema_concentric},
   {s_bone6,      X24, {5, 2,    0, 1, 3, 4},              2, 4, s1x2,     s2x2,     6,  0, 0, 1, 1,  0x0FE, schema_concentric},

   {s2x3,           Z, {4, 1,    5, 0, 2, 3},              2, 4, s1x2,     s2x2,     6,  1, 1, 2, 1,  0x0FB, schema_concentric},
   {s_1x2dmd,     X6P, {5, 2,    0, 1, 3, 4},              2, 4, s1x2,     s1x4,     6,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_1x2dmd,     X24, {5, 2,    0, 1, 3, 4},              2, 4, s1x2,     s1x4,     6,  1, 0, 1, 1,  0x0F5, schema_concentric},

   {s1x6,           Z, {2, 5,    0, 1, 3, 4},              2, 4, s1x2,     s1x4,     6,  0, 0, 1, 1,  0x2FA, schema_concentric},
   {s1x6,         X42, {1, 2, 4, 5,    0, 3},              4, 2, s1x4,     s1x2,     6,  0, 0, 1, 1,  0x0FA, schema_concentric},

   /* these two are duplicate. */
   {s_wingedstar,   Z, {7, 2, 3, 6,    0, 1, 4, 5},        4, 4, s_star,   s1x4,     8,  1, 0, 1, 1,  0x2F5, schema_conc_star},
   {s_wingedstar,   Z, {7, 2, 3, 6,    0, 1, 4, 5},        4, 4, s_star,   s1x4,     8,  1, 0, 1, 1,  0x2F5, schema_concentric},

   {s_rigger,       Z, {5, 0, 1, 4,    6, 7, 2, 3},
             4, 4, s2x2,     s1x4,     8,  1, 0, 1, 1,  0x2F5, schema_concentric},
   {s_qtag,         Z, {0, 1, 3, 4, 5, 7,    6, 2},        6, 2, s_bone6,  s1x2,     8,  0, 0, 1, 1,  0x2FA, schema_concentric_6_2_tgl},
   {sbigdmd,        Z, {9, 3,
             0, 1, 2, -1, 4, 5, 6, 7, 8, -1, 10, 11},      2,12, s1x2,     sbigdmd, 12,  1, 0, 2, 1,  0x2FD, schema_concentric},
   {sbighrgl,       Z, {9, 3,
             0, 1, 2, -1, 4, 5, 6, 7, 8, -1, 10, 11},      2,12, s1x2,     sbigdmd, 12,  0, 0, 2, 1,  0x2FE, schema_concentric},
   {s3x1dmd,        Z, {1, 2, 3, 5, 6, 7,    0, 4},        6, 2, s_2x1dmd, s1x2,     8,  0, 0, 1, 1,  0x2FA, schema_concentric},
   {s_crosswave,    Z, {0, 1, 3, 4, 5, 7,    6, 2},        6, 2, s_1x2dmd, s1x2,     8,  0, 1, 1, 1,  0x2F5, schema_concentric},
   {s2x4,           Z, {1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s2x2,     s2x2,     8,  0, 1, 9, 1,  0x2F7, schema_concentric},
   {s2x4,           Z, {6, 1, 2, 5,    0, 3, 4, 7},        4, 4, s2x2,     s2x2,     8,  1, 0, 9, 1,  0x2FD, schema_concentric},
   {s2x4,           Z, {6, 1, 2, 5,    7, 0, 3, 4},        4, 4, s2x2,     s2x2,     8,  1, 1, 9, 1,  0x2FB, schema_concentric},
   {s_bone,         Z, {6, 7, 2, 3,    5, 0, 1, 4},        4, 4, s1x4,     s2x2,     8,  0, 1, 9, 1,  0x2F7, schema_concentric},
   {s_qtag,         Z, {6, 7, 2, 3,    5, 0, 1, 4},        4, 4, s1x4,     s2x2,     8,  0, 1, 9, 1,  0x2FD, schema_concentric},
   {s_dhrglass,     Z, {6, 3, 2, 7,    5, 0, 1, 4},        4, 4, sdmd,     s2x2,     8,  0, 1, 9, 1,  0x2F7, schema_concentric},
   {s_hrglass,      Z, {6, 3, 2, 7,    5, 0, 1, 4},        4, 4, sdmd,     s2x2,     8,  0, 1, 9, 1,  0x2FD, schema_concentric},

   {s_crosswave,   CD, {1, 3, 5, 7,    0, 2, 4, 6},        4, 4, sdmd,     sdmd,     8,  0, 0, 9, 1,  0x0FA, schema_concentric},
   /* This next map, with its loss of elongation information for the outer diamond, is necessary to make the call
      "with confidence" work from the setup formed by having the centers partner tag in left-hand waves.  This means that certain
      Bakerisms involving concentric diamonds, in which each diamond must remember its own elongation, are not possible.  Too bad. */
   {s_crosswave,    Z, {1, 3, 5, 7,    6, 0, 2, 4},        4, 4, sdmd,     sdmd,     8,  0, 1, 9, 1,  0x2F5, schema_concentric},
   {s3x1dmd,        Z, {1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s1x4,     sdmd,     8,  0, 1, 9, 1,  0x2F5, schema_concentric},
   {s3x1dmd,       DL, {1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s1x4,     sdmd,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_galaxy,       Z, {7, 1, 3, 5,    0, 2, 4, 6},        4, 4, s2x2,     sdmd,     8,  1, 0, 9, 1,  0x2F5, schema_concentric},
   {s_galaxy,       Z, {1, 3, 5, 7,    0, 2, 4, 6},        4, 4, s2x2,     sdmd,     8,  0, 0, 9, 1,  0x2FA, schema_concentric},
   {s_crosswave,    Z, {1, 3, 5, 7,    6, 0, 2, 4},        4, 4, sdmd,     s_star,   8,  0, 1, 9, 1,  0x2F5, schema_concentric},
   {s_crosswave,    Z, {1, 3, 5, 7,    0, 2, 4, 6},        4, 4, sdmd,     s_star,   8,  0, 0, 9, 1,  0x2FA, schema_concentric},
   {s3x1dmd,        Z, {1, 2, 5, 6,    7, 0, 3, 4},        4, 4, s1x4,     s_star,   12, 0, 1, 9, 1,  0x2F5, schema_concentric},
   {s3x1dmd,        Z, {1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s1x4,     s_star,   12, 0, 0, 1, 1,  0x2FA, schema_concentric},

   {s3dmd,          Z, {10, 11, 4, 5,    0, 2, 6, 8},      4, 4, s1x4,     s2x2,    12,  0, 0, 3, 1,  0x0EF, schema_concentric},
   {s3dmd,          Z, {10, 11, 4, 5,    8, 0, 2, 6},      4, 4, s1x4,     s2x2,    12,  0, 1, 3, 1,  0x2DF, schema_concentric},
   {s4x4,           Z, {15, 3, 7, 11,    12, 0, 4, 8},     4, 4, s2x2,     s2x2,    16,  0, 0, 3, 1,  0x0CF, schema_concentric},

   {s1x3dmd,       DL, {1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s1x4,     sdmd,     8,  0, 0, 1, 1,  0x100, schema_nothing},
   {s_crosswave,   DL, {1, -1, 5, -1,    0, 2, 4, 6},      4, 4, s1x4,     sdmd,     8,  0, 0, 1, 1,  0x100, schema_nothing},
   {s1x8,          DL, {1, -1, 5, -1,    0, -1, 4, -1},    4, 4, s1x4,     sdmd,     8,  0, 0, 1, 1,  0x100, schema_nothing},
   /* This one allows "finish" when the center "line" is actually a diamond whose centers are empty.
      This can happen because of the preference given for lines over diamonds at the conclusion of certain calls. */
                                                                                 /* used to be 1, and still is, 'cuz it has to be! */
   {s_wingedstar,  DL, {1, 2, 5, 6,    0, 3, 4, 7},        4, 4, s1x4,     sdmd,     8,  0, 0, 1, 1,  0x100, schema_nothing},
   {s_wingedstar12,DL, {1, 2, 4, 7, 8, 10,
                           11, 0, 3, 5, 6, 9},             6, 6, s1x6,     s_short6, 12, 0, 1, 1, 1,  0x0F5, schema_conc_star12},
   {s_wingedstar16,DL, {1, 2, 6, 5, 9, 10, 14, 13,
                           3, 7, 12, 8, 11, 15, 4, 0},     8, 8, s1x8,    s_spindle, 16, 0, 0, 1, 1,  0x0FA, schema_conc_star16},


   {s_barredstar12,DL, {11, 12, 13, 4, 5, 6,
                           0, 1, 2, 3, 7, 8, 9, 10},       6, 8, s1x6,     s2x4,    14,  0, 0, 2, 1,  0x0FA, schema_conc_bar12},
   {s_barredstar16,DL, {14, 15, 17, 16, 5, 6, 8, 7,
                      0, 1, 2, 3, 4, 9, 10, 11, 12, 13},   8,10, s1x8,     s2x5,    18,  0, 0, 2, 1,  0x0FA, schema_conc_bar16},
   {s_barredstar,  DL, {8, 9, 3, 4,
                           0, 1, 2, 5, 6, 7},              4, 6, s1x4,     s2x3,    10,  0, 0, 2, 1,  0x0FB, schema_concentric},
   {s_qtag,        OT, {6, 7, 3, 2,    0, 1, 4, 5},        2, 4, s1x2,     s2x2,     8,  0, 0, 2, 2,  0x100, schema_nothing},
   {s1x8,          OT, {1, 3, 2, 6, 7, 5,    0, 4},        2, 2, s1x2,     s1x2,     8,  0, 0, 1, 3,  0x0FA, schema_concentric_others},
   {s1x6,          OT, {1, 2, 5, 4,    0, 3},              2, 2, s1x2,     s1x2,     6,  0, 0, 1, 2,  0x0FA, schema_concentric_others},

   {s_spindle,      Z, {6, 0, 5, 1, 4, 2,    7, 3},        2, 2, s1x2,     s1x2,     8,  1, 0, 1, 3,  0x2F5, schema_concentric_others},
   {s_spindle,     OT, {0, 1, 2, 4, 5, 6,    7, 3},        6, 2, s2x3,     s1x2,     8,  0, 0, 1, 1,  0x100, schema_nothing},

   /*
   {s_short6,      OT, {0, 2, 5, 3,    4, 1},              2, 2, s1x2,     s1x2,     6,  0, 1, 2, 2,  0x100, schema_nothing},
   */

   {s_short6,     X6P, {0, 2, 3, 5,    4, 1},              4, 2, s2x2,     s1x2,     6,  0, 1, 2, 1,  0x100, schema_nothing},

   {s_short6,      OT, {0, 2, 3, 5,    4, 1},              4, 2, s2x2,     s1x2,     6,  0, 1, 2, 1,  0x0F5, schema_concentric},
   {s_short6,       Z, {5, 0, 2, 3,    4, 1},              4, 2, s2x2,     s1x2,     6,  1, 1, 2, 1,  0x0FA, schema_concentric},
   {s_short6,       Z, {5, 3, 0, 2,    4, 1},              2, 2, s1x2,     s1x2,     6,  0, 1, 2, 2,  0x2F5, schema_concentric_others},

   {s2x4,          OT, {1, 2, 6, 5,    0, 3, 4, 7},
             2, 4, s1x2,     s2x2,     8,  0, 0, 1, 2,  0x100, schema_nothing},
   {s_galaxy,      IL, {0, 3, 5, 4, 7, 1,    6, 2},
             3, 2, s_trngl,  s1x2,     8,  3, 1, 1, 2,  0x0FE, schema_intlk_lateral_6},
   {s_hrglass,     IV, {7, 0, 1, 3, 4, 5,    6, 2},
             3, 2, s_trngl,  s1x2,     8,  0, 0, 1, 2,  0x0FE, schema_intlk_vertical_6},
   {s_galaxy,      IV, {2, 5, 7, 6, 1, 3,    0, 4},
             3, 2, s_trngl,  s1x2,     8,  2, 0, 1, 2,  0x0FD, schema_intlk_vertical_6},
   {s_galaxy,      L6, {7, 0, 1, 3, 4, 5,    6, 2},
             6, 2, s_short6, s1x2,     8,  1, 1, 1, 1,  0x100, schema_nothing},
   {s_galaxy,      V6, {1, 2, 3, 5, 6, 7,    0, 4},
             6, 2, s_short6, s1x2,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   /* This next one is duplicate!  It has to be! */
   {s_hrglass,     V6, {5, 0, 3, 1, 4, 7,    6, 2},
             6, 2, s_bone6,  s1x2,     8,  1, 0, 1, 1,  0x100, schema_nothing},


   {s2x8,          QL, {0, 1, 14, 15, 6, 7, 8, 9,       2, 3, 4, 5, 10, 11, 12, 13},
             4, 8, s2x2,     s2x4,     16, 0, 0, 1, 2,  0x0FE, QL},
   {s4x4,          QL, {8, 6, 4, 5, 12, 13, 0, 14,      10, 15, 3, 1, 2, 7, 11, 9},
             4, 8, s1x4,     s2x4,     16, 0, 0, 2, 2,  0x0FB, QL},

   {s3x4,          Q2, {9, 10, 0, 6, 4, 3,              8, 11, 1, 2, 5, 7},
             3, 6, s1x3,     s2x3,     12, 1, 1, 1, 2,  0x0FB, Q2},
   {s_d3x4,        Q2, {10, 11, 0, 4, 5, 6,             1, 2, 3, 7, 8, 9},
             3, 6, s1x3,     s2x3,     12, 1, 0, 1, 2,  0x0FD, Q2},
   {s3dmd,         Q2, {8, 7, 6, 0, 1, 2,               9, 10, 11, 3, 4, 5},
             3, 6, s1x3,     s1x6,     12, 0, 0, 2, 2,  0x0FB, Q2},


   {s_hrglass,     TL, {5, 4, -1, -1, -1, -1, 1, 0,     6, 3, 2, 7},
             4, 4, s2x2,     sdmd,     12, 0, 0, 2, 2,  0x100, XXX},
   {s_dhrglass,    TL, {-1, -1, 0, 5, 4, 1, -1, -1,     6, 3, 2, 7},
             4, 4, s2x2,     sdmd,     12, 1, 0, 1, 2,  0x100, XXX},
   {s3x4,          TL, {9, 8, 6, 7, 0, 1, 3, 2,         10, 11, 4, 5},
             4, 4, s1x4,     s1x4,     12, 0, 0, 2, 2,  0x0FB, TL},
   {s_qtag,         Z, {5, 4, 0, 1,                     6, 7, 2, 3},
             2, 4, s1x2,     s1x4,      8, 0, 0, 2, 2,  0x2FB, TL},

   {s4dmd,          Z, {11, 12, 0, 13, 8, 5, 3, 4,      10, -1, -1, 1, 2, -1, -1, 9},
             4, 8, sdmd,     s2x4,     16, 1, 1, 1, 2,  0x2FB, QL},
   {s4dmd,          Z, {11, -1, 0, -1, 8, -1, 3, -1,    1, 2, 6, 7, 9, 10, 14, 15},
             4, 8, s1x4,     s_qtag,   16, 1, 0, 1, 2,  0x2FD, QL},
   {swqtag,         Z, {9, 4,     0, 1, 2, 3, 5, 6, 7, 8},
             2, 8, s1x2, s_rigger,    16,  0, 0, 1, 1,  0x2FB, schema_concentric},
   {sd2x5,          Z, {4, 3, 2, 9, 8, 7,     0, 6, 5, 1},
    6, 4, s2x3, s2x2,                 10,  1, 0, 1, 1,  0x2FD, schema_concentric},
   {s2x5,           Z, {1, 2, 3, 6, 7, 8,     9, 0, 4, 5},
    6, 4, s2x3, s2x2,                 10,  0, 1, 0, 1,  0x2F7, schema_concentric},

   {s1x16,         QL, {0, 1, 3, 2, 11, 10, 8, 9,         4, 5, 7, 6, 12, 13, 15, 14},
             4, 8, s1x4,     s1x8,     16, 0, 0, 1, 2,  0x0FE, QL},
   {sbigbigh,      QL, {3, 2, 0, 1, 8, 9, 11, 10,         4, 5, 7, 6, 12, 13, 15, 14},
             4, 8, s1x4,     s1x8,     16, 1, 0, 1, 2,  0x0FD, QL},
   {sbigbigx,      QL, {0, 1, 3, 2, 11, 10, 8, 9,         7, 6, 5, 4, 15, 14, 13, 12},
             4, 8, s1x4,     s2x4,     16, 0, 1, 1, 2,  0x0F7, QL},

   {s1x12,         TL, {0, 1, 3, 2, 9, 8, 6, 7,         4, 5, 10, 11},
             4, 4, s1x4,     s1x4,     12, 0, 0, 1, 2,  0x0FE, TL},
   {sbigx,         TL, {0, 1, 3, 2, 9, 8, 6, 7,         10, 11, 4, 5},
             4, 4, s1x4,     s1x4,     12, 0, 1, 1, 2,  0x0F7, TL},
   {sbigh,         TL, {3, 2, 0, 1, 6, 7, 9, 8,         4, 5, 10, 11},
             4, 4, s1x4,     s1x4,     12, 1, 0, 1, 2,  0x0FD, TL},
   {sbigbone,      TL, {0, 1, 10, 11, 4, 5, 6, 7,       2, 3, 8, 9},
             4, 4, s2x2,     s1x4,     12, 0, 0, 1, 2,  0x0FE, TL},
   {sbigrig,       TL, {0, 1, 3, 2, 9, 8, 6, 7,         4, 5, 10, 11},
             4, 4, s1x4,     s2x2,     12, 0, 0, 1, 2,  0x0FE, TL},
   {sbigdmd,       TL, {11, 0, 1, 10, 7, 4, 5, 6,       8, 9, 2, 3},
             4, 4, s2x2,     s1x4,     12, 1, 1, 1, 2,  0x0FB, TL},
   {sbighrgl,      TL, {0, 1, 10, 11, 4, 5, 6, 7,       8, 9, 2, 3},
             4, 4, s2x2,     sdmd,     12, 0, 1, 1, 2,  0x0F7, TL},
   {sbigdhrgl,     TL, {0, 1, 10, 11, 4, 5, 6, 7,       9, 2, 3, 8},
             4, 4, s2x2,     sdmd,     12, 0, 0, 1, 2,  0x0FE, TL},
   {s2x6,          TL, {0, 1, 10, 11, 4, 5, 6, 7,       2, 3, 8, 9},
             4, 4, s2x2,     s2x2,     12, 0, 0, 1, 2,  0x0FE, TL},



   {s4x6,          TL, {17, 18, 11, 0, 1, 10, 19, 16, 13, 22, 7, 4, 5, 6, 23, 12,
                           15, 20, 9, 2, 3, 8, 21, 14},
             8, 8, s2x4,     s2x4,     24, 1, 1, 1, 2,  0x0FB, TL},
   {s3x6,          TL, {14, 15, 0, 1, 16, 13, 10, 7, 4, 5, 6, 9,     12, 17, 2, 3, 8, 11},
             6, 6, s2x3,     s2x3,     18, 1, 1, 1, 2,  0x0FB, TL},
   {s2x5,          TL, {0, 4, 5, 9,     1, 2, 3, 6, 7, 8},
             4, 6, s2x2,     s2x3,     10, 0, 0, 1, 1,  0x100, XXX},
   {sd2x5,         TL, {0, 6, 5, 1,     4, 3, 2, 9, 8, 7},
             4, 6, s2x2,     s2x3,     10, 0, 1, 1, 1,  0x100, XXX},

   {s_qtag,       XXX, {5, 0, 1, 4,     6, 7, 2, 3},
             4, 4, s2x2,     s1x4,      8, 1, 0, 2, 1,  0x2F7, TL},
   {s_bone,       XXX, {0, 1, 4, 5,     6, 7, 2, 3},
             4, 4, s2x2,     s1x4,      8, 0, 0, 1, 1,  0x2FE, TL},






   {s4x4,          TL, {8, 6, 4, 5, 12, 13, 0, 14,
                                       15, 3, 7, 11},
             4, 4, s1x4,     s2x2,     12, 0, 0, 2, 2,  0x0FB, TL},
   {s1x8,          GS, {2, 6, 3, 7, 1, 5,    0, 4},
             2, 2, s1x2,     s1x2,     8,  0, 0, 1, 3,  0x0FA, schema_grand_single_concentric},
   {s_spindle,     GS, {5, 1, 6, 0, 4, 2,    7, 3},
             2, 2, s1x2,     s1x2,     8,  1, 0, 1, 3,  0x0F5, schema_grand_single_concentric},
   {s1x6,          GS, {2, 5, 1, 4,    0, 3},
             2, 2, s1x2,     s1x2,     6,  0, 0, 1, 2,  0x0FA, schema_grand_single_concentric},
   {s1x4,          SI, {1, 3,          0, 2},
             2, 2, s1x2,     s1x2,     4,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {sdmd,          SI, {3, 1,          0, 2},
             2, 2, s1x2,     s1x2,     4,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_wingedstar16,S6, {2, 3, 5, 4, 6, 7, 14, 15, 13, 12, 10, 11,
                           0, 1, 8, 9},
             4, 4, s_star,   s1x4,     16, 0, 0, 1, 3,  0x0FA, schema_conc_star16},

   {s4x4,          L4, {10, 15, 3, 1, 2, 7, 11, 9,      12, 13, 14, 0, 4, 5, 6, 8},
             8, 8, s2x4,     s2x4,     16, 0, 0, 2, 1,  0x0FB, schema_concentric},
   {s4x4,          C4, {6, 11, 15, 13, 14, 3, 7, 5,     8, 9, 10, 12, 0, 1, 2, 4},
             8, 8, s2x4,     s2x4,     16, 1, 1, 2, 1,  0x100, XXX},
   {s4x6,          L4, {10, 9, 8, 7, 22, 21, 20, 19,    17, 18, 11, 0, 5, 6, 23, 12},
             /* This used to be 2FD, need this for 4x4 countershake from blocks */
             8, 8, s2x4,     s2x4,     16, 0, 1, 2, 1,  0x0F7, schema_concentric},
   {s4x6,          C4, {10, 9, 8, 7, 22, 21, 20, 19,    17, 18, 11, 0, 5, 6, 23, 12},
             8, 8, s2x4,     s2x4,     16, 0, 1, 2, 1,  0x100, XXX},
   {s_wingedstar12,S2, {2, 3, 4, 11, 10, 5, 8, 9,       0, 1, 6, 7},
             4, 4, s_star,   s1x4,     12, 0, 0, 1, 2,  0x0FA, schema_conc_star12},
   {s3x4,          G3, {8, 11, 1, 2, 5, 7,              9, 10, 0, 3, 4, 6},
    /* elong used to be 1, changed to 2 to make 12 matrix relay the diamond work.
                                                 V */
             6, 6, s2x3,     s2x3,     12, 1, 1, 2, 1,  0x0FA, schema_concentric},
   {s_d3x4,        G3, {1, 2, 3, 7, 8, 9,               10, 11, 0, 4, 5, 6},
             6, 6, s2x3,     s2x3,     12, 0, 1, 2, 1,  0x0F5, schema_concentric},

   {s_barredstar16,B6, {15, 1, 16, 12, 17, 2, 8, 11, 7, 3, 6, 10,
                           13, 14, 0, 4, 5, 9},
             4, 6, s_star,   s2x3,    18,  0, 1, 2, 3,  0x0F5, schema_conc_bar16},
   {s2x8,          B6, {1, 2, 13, 14, 3, 4, 11, 12, 5, 6, 9, 10,
                           0, 7, 8, 15},
             4, 4, s2x2,     s2x2,    16,  0, 0, 1, 3,  0x100, XXX},
   {s_barredstar12,B2, {12, 1, 13, 9, 6, 2, 5, 8,       10, 11, 0, 3, 4, 7},
             4, 6, s_star,   s2x3,    14,  0, 1, 2, 2,  0x0F5, schema_conc_bar12},
   {s2x6,          B2, {1, 2, 9, 10, 3, 4, 7, 8,        0, 5, 6, 11},
             4, 4, s2x2,     s2x2,    12,  0, 0, 1, 2,  0x100, XXX},
   {s_barredstar,  BA, {9, 1, 4, 6,                     7, 8, 0, 2, 3, 5},
             4, 6, s_star,   s2x3,    10,  0, 1, 2, 1,  0x0F7, schema_concentric},


   /* these 2 are sort of duplicates */
   {s2x4,          BA, {1, 2, 5, 6,    0, 3, 4, 7},
             4, 4, s2x2,     s2x2,     8,  0, 0, 1, 1,  0x100, XXX},
   {s2x4,           Z, {1, 2, 5, 6,    0, 3, 4, 7},
             4, 4, s2x2,     s2x2,     8,  0, 0, 1, 1,  0x0FE, schema_concentric},

   {s_qtag,        OO, {6, 7, 2, 3,    -1, -1, -1, -1, -1, 4, 5, -1,
                           -1, -1, -1, -1, -1, 0, 1, -1},
             4,16, s1x4,     s4x4,     8,  0, 0, 1, 1,  0x0FB, schema_conc_o},
   {s4x4,          OO, {15, 3, 7, 11,    -1, 1, 2, -1, -1, 5, 6, -1,
                           -1, 9, 10, -1, -1, 13, 14, -1},
             4,16, s2x2,     s4x4,     16, 0, 0, 1, 1,  0x0CF, schema_conc_o},
   {s2x4,          OO, {1, 2, 5, 6,    -1, 3, 4, -1, -1, -1, -1, -1,
                           -1, 7, 0, -1, -1, -1, -1, -1},
             4,16, s2x2,     s4x4,     8,  0, 0, 1, 1,  0x0FE, schema_conc_o},
   {s_hrglass,     OO, {6, 3, 2, 7,    -1, -1, -1, -1, -1, 4, 5, -1,
                           -1, -1, -1, -1, -1, 0, 1, -1},
             4,16, sdmd,     s4x4,     8,  0, 0, 1, 1,  0x0FB, schema_conc_o},
   {s_dhrglass,    OO, {6, 3, 2, 7,    -1, 1, 4, -1, -1, -1, -1, -1,
                           -1, 5, 0, -1, -1, -1, -1, -1},
             4,16, sdmd,     s4x4,     8,  0, 0, 1, 1,  0x0FE, schema_conc_o},
   {s_bone,        OO, {6, 7, 2, 3,    -1, 1, 4, -1, -1, -1, -1, -1,
                           -1, 5, 0, -1, -1, -1, -1, -1},
             4,16, s1x4,     s4x4,     8,  0, 0, 1, 1,  0x0FE, schema_conc_o},
   {s_qtag,        T6, {0, 1, 3, 4, 5, 7,    6, 2},
             6, 2, s_bone6,  s1x2,     8,  0, 0, 1, 1,  0x100, XXX},

   {s2x3,         X6P, {4, 1,    0, 2, 3, 5},
             2, 4, s1x2,     s2x2,     6,  1, 0, 1, 1,  0x0FD, schema_concentric},
   {s2x3,         X24, {4, 1,    0, 2, 3, 5},
             2, 4, s1x2,     s2x2,     6,  1, 0, 1, 1,  0x0FD, schema_concentric},
   {s_2x1dmd,     X6P, {0, 1, 3, 4,    5, 2},
             4, 2, s1x4,     s1x2,     6,  0, 1, 1, 1,  0x0F5, schema_concentric},
   {s3x1dmd,      X62, {0, 1, 2, 4, 5, 6,    7, 3},
             6, 2, s1x6,     s1x2,     8,  0, 1, 1, 1,  0x0F5, schema_concentric},
   {s_crosswave,  X62, {6, 7, 1, 2, 3, 5,    0, 4},
             6, 2, s_2x1dmd, s1x2,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s1x3dmd,      X62, {1, 2, 3, 5, 6, 7,    0, 4},
             6, 2, s_1x2dmd, s1x2,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   /* This next one is duplicated!!!!  It has to be!!!  I wish I knew where. */
   {s_hrglass,    X62, {5, 0, 3, 1, 4, 7,    6, 2},
             6, 2, s_bone6,  s1x2,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_ptpd,       X62, {1, 7, 6, 5, 3, 2,    0, 4},
             6, 2, s_bone6,  s1x2,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s1x8,         X62, {1, 3, 2, 5, 7, 6,    0, 4},
             6, 2, s1x6,     s1x2,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_rigger,     X62, {5, 7, 0, 1, 3, 4,    6, 2},
             6, 2, s_short6, s1x2,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_qtag,       X62, {5, 7, 0, 1, 3, 4,    6, 2},
             6, 2, s2x3,     s1x2,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_spindle,    X62, {0, 1, 2, 4, 5, 6,    7, 3},
             6, 2, s2x3,     s1x2,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_crosswave,  X26, {7, 3,    0, 1, 2, 4, 5, 6},
             2, 6, s1x2,     s_2x1dmd, 8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s3x1dmd,      X26, {2, 6,    0, 1, 3, 4, 5, 7},
             2, 6, s1x2,     s_2x1dmd, 8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s1x3dmd,      X26, {7, 3,    0, 1, 2, 4, 5, 6},
             2, 6, s1x2,     s1x6,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_ptpd,       X26, {2, 6,    3, 0, 1, 7, 4, 5},
             2, 6, s1x2,     s_short6, 8,  0, 1, 2, 1,  0x0F7, schema_concentric},
   {s3x4,         X26, {11, 5,    9, 10, 0, 3, 4, 6},
             2, 6, s1x2,     s2x3,     12, 0, 1, 2, 1,  0x0F5, schema_concentric},
   {s1x8,         X26, {2, 6,    0, 1, 3, 4, 5, 7},
             2, 6, s1x2,     s1x6,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_dhrglass,   X26, {7, 3,    0, 1, 2, 4, 5, 6},
             2, 6, s1x2,     s_bone6,  8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s_3mdmd,      B26, {7, 1,    0, 2, 5, 6, 8, 11},
             2, 6, s1x2,     s_bone6, 12,  1, 0, 1, 1,  0x0FD, B26},
   {s4dmd,        B26, {15, 7,   0, 3, 6, 8, 11, 14},
             2, 6, s1x2,     s_bone6, 16,  0, 0, 1, 1,  0x0FE, B26},
   {s_bone,       X26, {7, 3,    0, 1, 2, 4, 5, 6},
             2, 6, s1x2,     s_bone6,  8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_qtag,       X26, {7, 3,    5, 6, 0, 1, 2, 4},
             2, 6, s1x2,     s_short6, 8,  0, 1, 1, 1,  0x0FD, schema_concentric},
   {s_hrglass,    X26, {7, 3,    5, 6, 0, 1, 2, 4},
             2, 6, s1x2,     s_short6, 8,  1, 1, 1, 1,  0x0FE, schema_concentric},
   {s_spindle,    X26, {5, 1,    6, 7, 0, 2, 3, 4},
             2, 6, s1x2,     s_short6, 8,  1, 1, 2, 1,  0x0FB, schema_concentric},
   {s3dmd,        X26, {11, 5,    0, 1, 2, 6, 7, 8},
             2, 6, s1x2,     s2x3,     12, 0, 0, 9, 1,  0x100, XXX},
   {s_323,        X26, {7, 3,     0, 1, 2, 4, 5, 6},
             2, 6, s1x2,     s2x3,      8, 0, 0, 2, 1,  0x0FA, schema_concentric},
   {s1x3dmd,       CK, {0, 3, 4, 7,    1, 2, 5, 6},
             4, 4, sdmd,     s1x4,     8,  0, 0, 1, 1,  0x0FA, schema_rev_checkpoint},
   {s_ptpd,        CK, {0, 2, 4, 6,    1, 7, 5, 3},
             4, 4, s1x4,     s2x2,     8,  0, 0, 1, 1,  0x0FA, schema_rev_checkpoint},
   {s1x8,          CK, {0, 2, 4, 6,    1, 3, 5, 7},
             4, 4, s1x4,     s1x4,     8,  0, 0, 1, 1,  0x0FA, schema_rev_checkpoint},
   {s_dhrglass,    CK, {0, 3, 1, 4, 7, 5,    6, 2},
             6, 2, s2x3,     s1x2,     8,  0, 0, 1, 1,  0x100, XXX},
   {s_spindle,     CK, {7, 1, 3, 5,    0, 2, 4, 6},
             4, 4, sdmd,     s2x2,     8,  0, 0, 1, 1,  0x0FA, schema_rev_checkpoint},
   {s_rigger,      CK, {5, 6, 0, 1, 2, 4,    7, 3},
             6, 2, s_short6, s1x2,     8,  1, 0, 1, 1,  0x100, XXX},
   {s_bone,        CK, {0, 1, 3, 4, 5, 7,    6, 2},
             6, 2, s_bone6,  s1x2,     8,  0, 0, 1, 1,  0x100, XXX},
   {s_thar,         Z, {1, 3, 5, 7,    0, 2, 4, 6},
             4, 4, s_star,   s_star,   8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_galaxy,       Z, {1, 3, 5, 7,    0, 2, 4, 6},
             4, 4, s2x2,     s_star,   8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   /* Next two are duplicate */
   {s_wingedstar,   Z, {2, 3, 6, 7,    0, 1, 4, 5},
             4, 4, s_star,   s1x4,     8,  0, 0, 1, 1,  0x2FA, schema_conc_star},
   {s_wingedstar,   Z, {2, 3, 6, 7,    0, 1, 4, 5},
             4, 4, s_star,   s1x4,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s3x1dmd,        Z, {7, 2, 3, 6,    0, 1, 4, 5},
             4, 4, sdmd,     s1x4,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s1x3dmd,        Z, {2, 3, 6, 7,    0, 1, 4, 5},
             4, 4, sdmd,     s1x4,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_crosswave,    Z, {6, 7, 2, 3,    0, 1, 4, 5},
             4, 4, s1x4,     s1x4,     8,  1, 0, 1, 1,  0x0F5, schema_concentric},
   {s1x8,           Z, {3, 2, 7, 6,    0, 1, 4, 5},
             4, 4, s1x4,     s1x4,     8,  0, 0, 1, 1,  0x0FA, schema_concentric},
   {s_dhrglass,     Z, {6, 3, 2, 7,    0, 1, 4, 5},
             4, 4, sdmd,     s2x2,     8,  0, 0, 1, 1,  0x0FE, schema_concentric},
   {s_hrglass,      Z, {6, 3, 2, 7,    0, 1, 4, 5},
             4, 4, sdmd,     s2x2,     8,  0, 0, 2, 1,  0x0FB, schema_concentric},
   {s_rigger,       Z, {0, 1, 4, 5,    6, 7, 2, 3},
             4, 4, s2x2,     s1x4,     8,  0, 0, 1, 1,  0x0EA, schema_concentric},
   {s_bone,         Z, {6, 7, 2, 3,    0, 1, 4, 5},
             4, 4, s1x4,     s2x2,     8,  0, 0, 1, 1,  0x0FE, schema_concentric},
   {s_qtag,         Z, {6, 7, 2, 3,    0, 1, 4, 5},
             4, 4, s1x4,     s2x2,     8,  0, 0, 2, 1,  0x0FB, schema_concentric},

   {nothing,        Z, {0},                                0, 0, nothing,  nothing},
};



static Const fixer f1x8aa;
static Const fixer f1x8ctr;
static Const fixer foozz;
static Const fixer fo6zz;
static Const fixer fqtgend;
static Const fixer f1x6aad;
static Const fixer f1x8aad;
static Const fixer foo66d;
static Const fixer foo55d;
static Const fixer f2x4endd;
static Const fixer fgalch;
static Const fixer fspindld;
static Const fixer fspindlbd;
static Const fixer f1x8endd;
static Const fixer bar55d;
static Const fixer f1x12outer;
static Const fixer f3x4rzz;
static Const fixer f3x4lzz;
static Const fixer f4x4rzz;
static Const fixer f4x4lzz;
static Const fixer f3dmoutrd;
static Const fixer fspindlod;
static Const fixer f323d;
static Const fixer fppaad;
static Const fixer fpp55d;
static Const fixer f1x3aad;
static Const fixer f1x2aad;
static Const fixer fboneendo;
static Const fixer distrig1;
static Const fixer distrig2;
static Const fixer distrig5;
static Const fixer distrig6;
static Const fixer distptp7;
static Const fixer distptp8;
static Const fixer dgald1;
static Const fixer dgald2;
static Const fixer dgald3;
static Const fixer dgald4;
static Const fixer d2x4x1;
static Const fixer d2x4c1;
static Const fixer d2x4x2;
static Const fixer d2x4c2;
static Const fixer d4x4l1;
static Const fixer d4x4l2;
static Const fixer d4x4l3;
static Const fixer d4x4l4;
static Const fixer d4x4d1;
static Const fixer d4x4d2;
static Const fixer d4x4d3;
static Const fixer d4x4d4;
static Const fixer box6c;
static Const fixer box9c;
static Const fixer fdrhgl_bb;
static Const fixer frigctr;
static Const fixer f2x4right;
static Const fixer f2x4left;
static Const fixer d3x4ub1;
static Const fixer d3x4ub2;
static Const fixer d3x4ub3;
static Const fixer d3x4ub4;
static Const fixer d3x4vb1;
static Const fixer d3x4vb2;
static Const fixer d3x4vb3;
static Const fixer d3x4vb4;


/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer foo33a    = {s1x2, s2x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {5, 4, 0, 1}};
static Const fixer foocc     = {s1x2, s2x4,        0, 0, 2,       &foocc,     &foo33a,    0,          0, 0,          0,    0,          0,          {2, 3, 7, 6}};
static Const fixer foo33     = {s1x2, s2x4,        0, 0, 2,       &foo33,     &foocc,     0,          0, 0,          0,    0,          0,          {0, 1, 5, 4}};
static Const fixer foo5a     = {s1x2, s2x4,        1, 0, 2,       &foo5a,     0,          0,          0, 0,          0,    0,          0,          {1, 6, 3, 4}};
static Const fixer fooa5     = {s1x2, s2x4,        1, 0, 2,       &fooa5,     0,          0,          0, 0,          0,    0,          0,          {0, 7, 2, 5}};
static Const fixer foo55     = {s1x1, s2x4,        0, 0, 4,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 2, 4, 6}};
static Const fixer fooaa     = {s1x1, s2x4,        0, 0, 4,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 3, 5, 7}};

static Const fixer foo11     = {s1x1, s2x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 4}};
static Const fixer foo22     = {s1x1, s2x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 5}};
static Const fixer foo44     = {s1x1, s2x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {2, 6}};
static Const fixer foo88     = {s1x1, s2x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {3, 7}};

static Const fixer n1x43     = {s1x2, s1x4,        0, 0, 1,       &n1x43,     &box9c,     0,          0, 0,          0,    0,          0,          {0, 1}};
static Const fixer n1x4c     = {s1x2, s1x4,        0, 0, 1,       &n1x4c,     &box6c,     0,          0, 0,          0,    0,          0,          {3, 2}};
static Const fixer n1x45     = {s1x1, s1x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 2}};
static Const fixer n1x4a     = {s1x2, s1x4,        0, 0, 1,       &n1x4a,     0,          0,          0, 0,          0,    0,          0,          {1, 3}};
static Const fixer n1x3a     = {s1x2, s1x3,        0, 0, 1,       &n1x3a,     0,          0,          0, 0,          0,    0,          0,          {0, 1}};
static Const fixer n1x3b     = {s1x2, s1x3,        0, 0, 1,       &n1x3b,     0,          0,          0, 0,          0,    0,          0,          {1, 2}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer f1x6aa    = {s1x2, s1x6,        0, 0, 2,       &f1x6aa,    &fo6zz,     0,          0, 0,          0,    0,          0,          {0, 1, 4, 3}};
static Const fixer f1x8aa    = {s1x2, s1x8,        0, 0, 2,       &f1x8aa,    &foozz,     0,          0, 0,          0,    0,          0,          {1, 3, 7, 5}};
static Const fixer f1x855    = {s1x1, s1x8,        0, 0, 4,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 2, 4, 6}};
static Const fixer fo6zz     = {s1x2, s_bone6,     1, 0, 2,       &fo6zz,     &f1x6aa,    0,          0, 0,          0,    0,          0,          {0, 4, 1, 3}};
static Const fixer foozz     = {s1x2, s_ptpd,      1, 0, 2,       &foozz,     &f1x8aa,    0,          0, 0,          0,    0,          0,          {1, 3, 7, 5}};
static Const fixer fo6zzd    = {s2x2, s_bone6,     0, 1, 1,       0,          0,          &f1x6aad,   0, 0,          0,    &fo6zzd,    0,          {0, 1, 3, 4}};
static Const fixer foozzd    = {s2x2, s_ptpd,      0, 1, 1,       0,          0,          &f1x8aad,   0, 0,          0,    &foozzd,    &fqtgend,   {1, 7, 5, 3}};

static Const fixer f3x4east  = {s1x2, s3x4,        1, 0, 2,       0,         &f3x4east,   0,          0, 0,          0,    0,          0,          {10, 9, 3, 4}};
static Const fixer f3x4west  = {s1x2, s3x4,        1, 0, 2,       0,         &f3x4west,   0,          0, 0,          0,    0,          0,          {0, 10, 4, 6}};
static Const fixer f3x4left  = {s1x2, s3x4,        0, 0, 2,       &f3x4left,  &f3x4rzz,   0,          0, 0,          0,    0,          0,          {0, 1, 7, 6}};
static Const fixer f3x4right = {s1x2, s3x4,        0, 0, 0x100+2, &f3x4right, &f3x4lzz,   0,          0, 0,          0,    0,          0,          {2, 3, 9, 8}};
static Const fixer f3x4lzz   = {s1x2, s2x6,        0, 0, 2,       &f3x4lzz,   &f3x4right, 0,          0, 0,          0,    0,          0,          {0, 1, 7, 6}};
static Const fixer f3x4rzz   = {s1x2, s2x6,        0, 0, 0x100+2, &f3x4rzz,   &f3x4left,  0,          0, 0,          0,    0,          0,          {4, 5, 11, 10}};
static Const fixer f4x4left  = {s1x2, s4x4,        0, 0, 2,       &f4x4left,  &f4x4rzz,   0,          0, 0,          0,    0,          0,          {12, 13, 5, 4}};
static Const fixer f4x4right = {s1x2, s4x4,        0, 0, 0x100+2, &f4x4right, &f4x4lzz,   0,          0, 0,          0,    0,          0,          {14, 0, 8, 6}};
static Const fixer f4x4lzz   = {s1x2, s2x8,        0, 0, 2,       &f4x4lzz,   &f4x4right, 0,          0, 0,          0,    0,          0,          {0, 1, 9, 8}};
static Const fixer f4x4rzz   = {s1x2, s2x8,        0, 0, 0x100+2, &f4x4rzz,   &f4x4left,  0,          0, 0,          0,    0,          0,          {6, 7, 15, 14}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer f3x4outer = {s1x3, s3x4,        1, 0, 2,       &f3x4outer, 0,          0,          0, 0,          0,    0,          0,          {0, 10, 9, 3, 4, 6}};
static Const fixer f3dmouter = {s1x3, s3dmd,       0, 0, 2,       &f3dmouter, 0,          0,          0, 0,          0,    0,          0,          {8, 7, 6, 0, 1, 2}};
       Const fixer fdhrgl    = {s_trngl,s_dhrglass,3, 0, 2,       &f323,      0,          0,          0, 0,          0,    0,          0,          {6, 5, 0, 2, 1, 4}};
static Const fixer f525nw    = {s1x3, s_525,       0, 0, 2,       &f525nw,    0,          0,          0, 0,          0,    0,          0,          {8, 7, 6, 0, 1, 2}};
static Const fixer f525ne    = {s1x3, s_525,       0, 0, 2,       &f525ne,    0,          0,          0, 0,          0,    0,          0,          {10, 9, 8, 2, 3, 4}};

static Const fixer fdpx44    = {s1x2, s4x4,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {10, 9, 1, 2}};
static Const fixer fdpxwve   = {s1x2, sdeepxwv,    0, 0, 2,       &fdpxwve,   &fdpx44,    0,          0, 0,          0,    0,          0,          {0, 1, 7, 6}};

static Const fixer f1x12outer= {s1x3, s1x12,       0, 0, 2,       &f1x12outer,&f3x4outer, 0,          0, 0,          0,    0,          0,          {0, 1, 2, 8, 7, 6}};

static Const fixer f3x4outrd = {s2x3, s3x4,        1, 0, 1,       0,          0,          0,          0, 0,          0,    &f3x4outrd, &f3dmoutrd, {3, 4, 6, 9, 10, 0}};
static Const fixer f3dmoutrd = {s2x3, s3dmd,       0, 0, 1,       0,          0,          0,          0, 0,          0,    &f3dmoutrd, &f3x4outrd, {0, 1, 2, 6, 7, 8}};
static Const fixer fdhrgld   = {s_bone6,s_dhrglass,0, 0, 1,       0,          &fdhrgld,   0,          0, 0,          &fspindlod,0,     &f323d,     {0, 1, 2, 4, 5, 6}};

static Const fixer f1x12outrd= {s1x6, s1x12,       0, 0, 1,       0,          0,          &f1x12outrd,0, 0,          0,    0,          0,          {0, 1, 2, 6, 7, 8}};
static Const fixer f1x12outre= {s1x8, s1x12,       0, 0, 1,       0,          0,          &f1x12outre,0, 0,          0,    0,          0,          {0, 1, 5, 4, 6, 7, 11, 10}};


/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer f1x3d6    =  {s1x3, s1x3dmd,    0, 0, 2,       &f1x3d6,    &f323,      0,          0, 0,          0,    0,          0,          {0, 1, 2, 6, 5, 4}};
       Const fixer f323    =   {s1x3, s_323,       0, 0, 2,       &f323,      &f1x3d6,    0,          0, 0,          0,    0,          0,          {6, 5, 4, 0, 1, 2}};

static Const fixer f323d   =   {s2x3, s_323,       0, 0, 1,       0,          0,          0,          0, 0,          0,    &f323d,     &f3x4outrd, {0, 1, 2, 4, 5, 6}};
static Const fixer f3x1zzd   = {sdmd, s3x1dmd,     0, 0, 1,       0,          0,          &f1x8endd,  0, &f3x1zzd,   0,    0,          0,          {0, 3, 4, 7}};
static Const fixer f1x3zzd   = {sdmd, s1x3dmd,     0, 0, 1,       0,          0,          &foo55d,    0, &f1x3zzd,   0,    0,          0,          {0, 3, 4, 7}};
static Const fixer f3x1yyd   = {sdmd, s3x1dmd,     0, 0, 1,       0,          0,          &f1x8aad,   0, &f3x1yyd,   0,    0,          0,          {1, 3, 5, 7}};
static Const fixer f2x1yyd   = {s_star,s_2x1dmd,   0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 2, 3, 5}};
static Const fixer f1x4xv    = {s1x4, s_crosswave, 1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {2, 3, 6, 7}};
static Const fixer f1x3yyd   = {sdmd, s1x3dmd,     0, 0, 1,       0,          0,          &foo66d,    &f1x4xv, &f1x3yyd, 0, &frigctr,  &frigctr,   {1, 3, 5, 7}};
static Const fixer f1x6aad   = {s1x4, s1x6,        0, 0, 1,       0,          0,          &f1x6aad,   0, &f3x1zzd,   0,    &fo6zzd,    &fo6zzd,    {0, 1, 3, 4}};
static Const fixer f1x8aad   = {s1x4, s1x8,        0, 0, 1,       0,          0,          &f1x8aad,   0, &f3x1zzd,   0,    &foozzd,    &foozzd,    {1, 3, 5, 7}};
static Const fixer fxwv1d    = {sdmd, s_crosswave, 0, 0, 1,       0,          0,          0,          0, &fxwv1d,    0,    0,          0,          {0, 2, 4, 6}};
static Const fixer fxwv2d    = {sdmd, s_crosswave, 0, 0, 1,       0,          0,          0,          0, &fxwv2d,    0,    0,          0,          {0, 3, 4, 7}};
static Const fixer fxwv3d    = {s_star,s_crosswave,0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 2, 5, 6}};

static Const fixer fqtgns    = {s1x2, s_qtag,      0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 5, 4}};
static Const fixer ftharns   = {s1x2, s_thar,      1, 0, 2,       &ftharns,   &fqtgns,    0,          0, 0,          0,    0,          0,          {2, 3, 7, 6}};
static Const fixer ftharew   = {s1x2, s_thar,      0, 0, 2,       &ftharew,   &fqtgns,    0,          0, 0,          0,    0,          0,          {0, 1, 5, 4}};

static Const fixer fqtgj1    = {s1x2, s_qtag,      1, 0, 2,       &fqtgj1,    0,          0,          0, 0,          0,    0,          0,          {1, 3, 7, 5}};
static Const fixer fqtgj2    = {s1x2, s_qtag,      1, 0, 2,       &fqtgj2,    0,          0,          0, 0,          0,    0,          0,          {0, 7, 3, 4}};
static Const fixer f2x3j1    = {s1x2, s2x3,        0, 0, 2,       &f2x3j1,    0,          0,          0, 0,          0,    0,          0,          {0, 1, 4, 3}};
static Const fixer f2x3j2    = {s1x2, s2x3,        0, 0, 2,       &f2x3j2,    0,          0,          0, 0,          0,    0,          0,          {1, 2, 5, 4}};
static Const fixer fqtgjj1   = {s2x2, s_qtag,      0, 0, 1,       0,          0,          0,          0, 0,          0,    &fqtgjj1,   &fqtgjj1,   {7, 1, 3, 5}};
static Const fixer fqtgjj2   = {s2x2, s_qtag,      0, 0, 1,       0,          0,          0,          0, 0,          0,    &fqtgjj2,   &fqtgjj2,   {0, 3, 4, 7}};
static Const fixer fgalcv    = {s1x2, s_galaxy,    1, 0, 1,       &fgalcv,    &fgalch,    0,          0, 0,          0,    0,          0,          {2, 6}};
static Const fixer fgalch    = {s1x2, s_galaxy,    0, 0, 1,       &fgalch,    &fgalcv,    0,          0, 0,          0,    0,          0,          {0, 4}};
static Const fixer fspindlc  = {s1x2, s_spindle,   1, 0, 2,       &fspindlc,  &f1x3aad,   0,          0, 0,          0,    0,          0,          {0, 6, 2, 4}};
static Const fixer fspindlf  = {s1x2, s_spindle,   0, 0, 2,       &fspindlf,  0,          0,          0, 0,          0,    0,          0,          {1, 2, 6, 5}};
static Const fixer fspindlg  = {s1x2, s_spindle,   0, 0, 2,       &fspindlg,  0,          0,          0, 0,          0,    0,          0,          {0, 1, 5, 4}};
static Const fixer fspindlfd = {s2x2, s_spindle,   0, 0, 1,       0,          0,          0,          0, 0,          0,    &fspindlfd, 0,          {1, 2, 5, 6}};
static Const fixer fspindlgd = {s2x2, s_spindle,   0, 0, 1,       0,          0,          0,          0, 0,          0,    &fspindlgd, 0,          {0, 1, 4, 5}};
static Const fixer f1x3aad   = {s1x2, s1x3dmd,     0, 0, 2,       &f1x3aad,   &fspindlc,  0,          0, 0,          0,    0,          0,          {1, 2, 6, 5}};
static Const fixer f2x3c     = {s1x2, s2x3,        1, 0, 2,       &f2x3c,     &f1x2aad,   0,          0, 0,          0,    0,          0,          {0, 5, 2, 3}};
static Const fixer f2x3a41   = {s1x2, s2x3,        1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 5}};
static Const fixer f2x3a14   = {s1x2, s2x3,        1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {2, 3}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer box3c     = {s1x2, s2x2,        0, 0, 1,       &box3c,     0,          0,          0, 0,          0,    0,          0,          {0, 1}};
static Const fixer box6c     = {s1x2, s2x2,        1, 0, 1,       &box6c,     0,          0,          0, 0,          0,    0,          0,          {1, 2}};
static Const fixer box9c     = {s1x2, s2x2,        1, 0, 1,       &box9c,     0,          0,          0, 0,          0,    0,          0,          {0, 3}};
static Const fixer boxcc     = {s1x2, s2x2,        0, 0, 1,       &boxcc,     0,          0,          0, 0,          0,    0,          0,          {3, 2}};
static Const fixer box55     = {s1x1, s2x2,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 2}};
static Const fixer boxaa     = {s1x1, s2x2,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 3}};
static Const fixer f2x5c     = {s1x2, s2x5,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {3, 4, 9, 8}};
static Const fixer f2x5d     = {s1x2, s2x5,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 6, 5}};
static Const fixer fd2x5d    = {s2x3, sd2x5,        1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {9, 8, 7, 4, 3, 2}};

static Const fixer f4x6a     = {s1x2, s4x6,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {13, 12, 0, 1}};
static Const fixer f4x6b     = {s1x2, s4x6,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {22, 23, 11, 10}};
static Const fixer f4x6c     = {s1x2, s4x6,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {7, 6, 18, 19}};
static Const fixer f4x6d     = {s1x2, s4x6,        0, 0, 2,       0,          0,          0,          0, 0,          0,    0,          0,          {4, 5, 17, 16}};

static Const fixer f1x2aad   = {s1x2, s_1x2dmd,    0, 0, 2,       &f1x2aad,   &f2x3c,     0,          0, 0,          0,    0,          0,          {0, 1, 4, 3}};

static const fixer f2x166d   = {sdmd, s_2x1dmd,    0, 0, 1,       0,          0,          0,          0, &f2x166d,   0,    0,          0,          {2, 4, 5, 1}};

static Const fixer f1x3bbd   = {s1x4, s1x3dmd,     0, 0, 1,       0,          0,          &f1x3bbd,   0, 0,          0,    &fspindld,  &fspindld,  {1, 2, 5, 6}};
static Const fixer fhrglassd = {s2x2, s_hrglass,   0, 2, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 4, 5}};
static Const fixer fspindld  = {s2x2, s_spindle,   0, 1, 1,       0,          0,          &f1x3bbd,   0, 0,          0,    &fspindld,  &fhrglassd, {0, 2, 4, 6}};
static Const fixer fptpzzd   = {s1x4, s_ptpd,      0, 0, 1,       0,          0,          &fptpzzd,   0, &fspindlbd, 0,    0,          0,          {0, 2, 4, 6}};
static Const fixer f3ptpo6   = {s_1x2dmd, s3ptpd,  0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {10, 11, 1, 4, 5, 7}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer fspindlbd = {sdmd, s_spindle,   0, 0, 1,       0,          0,          &fptpzzd,   0, &fspindlbd, 0,    0,          0,          {7, 1, 3, 5}};


/* This map is broken!!!!!! */
static Const fixer fspindlo  = {s_trngl,s_spindle, 1, 0, 2,       &f323,      0,          0,          0, &fspindlo,  &fspindlo, 0,     0,          {7, 0, 6, 3, 4, 2}};


static Const fixer fspindlod = {s_short6,s_spindle,1, 0, 1,       0,          &fdhrgld,   0,          0, 0,          &fspindlod, &f323d, 0,        {2, 3, 4, 6, 7, 0}};
static Const fixer d2x4b1    = {s2x2, s4x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {13, 7, 5, 15}};
static Const fixer d2x4b2    = {s2x2, s4x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {11, 14, 3, 6}};
static Const fixer d2x4w1    = {s1x4, s2x4,        0, 0, 1,       0,          0,          &d2x4w1,    0, 0,          0,    &d2x4b1,    &d2x4b1,    {0, 1, 4, 5}};
static Const fixer d2x4w2    = {s1x4, s2x4,        0, 0, 1,       0,          0,          &d2x4w2,    0, 0,          0,    &d2x4b2,    &d2x4b2,    {7, 6, 3, 2}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer d2x4d1    = {sdmd, s2x4,        0, 0, 1,       0,          0,          &d2x4x1,    0, &d2x4d1,    0,    &d2x4c1,    &d2x4c1,    {7, 1, 3, 5}};
static Const fixer d2x4d2    = {sdmd, s2x4,        0, 0, 1,       0,          0,          &d2x4x2,    0, &d2x4d2,    0,    &d2x4c2,    &d2x4c2,    {0, 2, 4, 6}};
static Const fixer d2x4c1    = {s2x2, s2x4,        0, 0, 1,       0,          0,          &d2x4x1,    0, &d2x4d1,    0,    &d2x4c1,    &d2x4c1,    {1, 3, 5, 7}};
static Const fixer d2x4c2    = {s2x2, s2x4,        0, 0, 1,       0,          0,          &d2x4x2,    0, &d2x4d2,    0,    &d2x4c2,    &d2x4c2,    {0, 2, 4, 6}};
static Const fixer d2x4z1    = {s2x2, s4x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {9, 11, 1, 3}};
static Const fixer d2x4z2    = {s2x2, s4x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {10, 15, 2, 7}};
static Const fixer d2x4y1    = {s2x2, s2x4,        0, 0, 1,       0,          0,          &d2x4w1,    &d2x4z1, 0,    0,    &d2x4y1,    &d2x4y1,    {0, 1, 4, 5}};
static Const fixer d2x4y2    = {s2x2, s2x4,        0, 0, 1,       0,          0,          &d2x4w2,    &d2x4z2, 0,    0,    0,          0,          {2, 3, 6, 7}};


static Const fixer d3x4ul1   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4ub1,   0,          {10, 8, 6, 5}};
static Const fixer d3x4ul2   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4ub2,   0,          {9, 11, 4, 7}};
static Const fixer d3x4ul3   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4ub3,   0,          {10, 1, 3, 5}};
static Const fixer d3x4ul4   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4ub4,   0,          {0, 11, 4, 2}};

static Const fixer d3x4ub1   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4ul1,    0,      0,    0,    0,          0,          {10, 5, 6, 8}};
static Const fixer d3x4ub2   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4ul2,    0,      0,    0,    0,          0,          {11, 4, 7, 9}};
static Const fixer d3x4ub3   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4ul3,    0,      0,    0,    0,          0,          {1, 3, 5, 10}};
static Const fixer d3x4ub4   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4ul4,    0,      0,    0,    0,          0,          {0, 2, 4, 11}};

static Const fixer d3x4vl1   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4vb1,   0,          {10, 11, 6, 7}};
static Const fixer d3x4vl2   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4vb2,   0,          {9, 8, 4, 5}};
static Const fixer d3x4vl3   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4vb3,   0,          {10, 11, 3, 2}};
static Const fixer d3x4vl4   = {s1x4, s3x4,        0, 0, 1,       0,          0,          0,           0,      0,    0,    &d3x4vb4,   0,          {0, 1, 4, 5}};

static Const fixer d3x4vb1   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4vl1,    0,      0,    0,    0,          0,          {10, 11, 6, 7}};
static Const fixer d3x4vb2   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4vl2,    0,      0,    0,    0,          0,          {5, 4, 8, 9}};
static Const fixer d3x4vb3   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4vl3,    0,      0,    0,    0,          0,          {2, 3, 11, 10}};
static Const fixer d3x4vb4   = {s2x2, s3x4,        0, 0, 1,       0,          0,          &d3x4vl4,    0,      0,    0,    0,          0,          {0, 1, 4, 5}};

static Const fixer d2x4x1    = {s1x4, s2x4,        0, 0, 1,       0,          0,          &d2x4x1,    0, &d2x4d1,    0,    &d2x4c1,    &d2x4c1,    {7, 1, 3, 5}};
static Const fixer d2x4x2    = {s1x4, s2x4,        0, 0, 1,       0,          0,          &d2x4x2,    0, &d2x4d2,    0,    &d2x4c2,    &d2x4c2,    {0, 6, 4, 2}};
static Const fixer dgalw1    = {s1x4, s_galaxy,    1, 0, 1,       0,          0,          &dgalw1,    0, &dgald1,    0,    0,          0,          {2, 1, 6, 5}};
static Const fixer dgalw2    = {s1x4, s_galaxy,    1, 0, 1,       0,          0,          &dgalw2,    0, &dgald2,    0,    0,          0,          {2, 3, 6, 7}};
static Const fixer dgalw3    = {s1x4, s_galaxy,    0, 0, 1,       0,          0,          &dgalw3,    0, &dgald3,    0,    0,          0,          {0, 1, 4, 5}};
static Const fixer dgalw4    = {s1x4, s_galaxy,    0, 0, 1,       0,          0,          &dgalw4,    0, &dgald4,    0,    0,          0,          {0, 7, 4, 3}};
static Const fixer dgald1    = {sdmd, s_galaxy,    1, 0, 1,       0,          0,          &dgalw1,    0, &dgald1,    0,    0,          0,          {2, 5, 6, 1}};
static Const fixer dgald2    = {sdmd, s_galaxy,    1, 0, 1,       0,          0,          &dgalw2,    0, &dgald2,    0,    0,          0,          {2, 3, 6, 7}};
static Const fixer dgald3    = {sdmd, s_galaxy,    0, 0, 1,       0,          0,          &dgalw3,    0, &dgald3,    0,    0,          0,          {0, 1, 4, 5}};
static Const fixer dgald4    = {sdmd, s_galaxy,    0, 0, 1,       0,          0,          &dgalw4,    0, &dgald4,    0,    0,          0,          {0, 3, 4, 7}};
static Const fixer ddmd1     = {sdmd, s_qtag,      0, 0, 1,       0,          0,          0,          0, &ddmd1,     0,    0,          0,          {6, 1, 2, 5}};
static Const fixer ddmd2     = {sdmd, s_qtag,      0, 0, 1,       0,          0,          0,          0, &ddmd2,     0,    0,          0,          {6, 0, 2, 4}};
static Const fixer distbone1 = {s1x4, s_bone,      0, 0, 1,       0,          0,          &distbone1, 0, 0,          0,    0,          0,          {0, 6, 4, 2}};
static Const fixer distbone2 = {s1x4, s_bone,      0, 0, 1,       0,          0,          &distbone2, 0, 0,          0,    0,          0,          {0, 7, 4, 3}};
static Const fixer distbone5 = {s1x4, s_bone,      0, 0, 1,       0,          0,          &distbone5, 0, 0,          0,    0,          0,          {5, 6, 1, 2}};
static Const fixer distbone6 = {s1x4, s_bone,      0, 0, 1,       0,          0,          &distbone6, 0, 0,          0,    0,          0,          {5, 7, 1, 3}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer distrig3  = {sdmd, s_rigger,    0, 0, 1,       0,          0,          &distrig1,  0, &distrig3,  0,    0,          0,          {7, 0, 3, 4}};
static Const fixer distrig1  = {s1x4, s_rigger,    0, 0, 1,       0,          0,          &distrig1,  0, &distrig3,  0,    0,          0,          {7, 0, 3, 4}};
static Const fixer distrig4  = {sdmd, s_rigger,    0, 0, 1,       0,          0,          &distrig2,  0, &distrig4,  0,    0,          0,          {6, 1, 2, 5}};
static Const fixer distrig2  = {s1x4, s_rigger,    0, 0, 1,       0,          0,          &distrig2,  0, &distrig4,  0,    0,          0,          {6, 5, 2, 1}};
static Const fixer distrig7  = {sdmd, s_rigger,    0, 0, 1,       0,          0,          &distrig5,  0, &distrig7,  0,    0,          0,          {6, 0, 2, 4}};
static Const fixer distrig5  = {s1x4, s_rigger,    0, 0, 1,       0,          0,          &distrig5,  0, &distrig7,  0,    0,          0,          {6, 0, 2, 4}};
static Const fixer distrig8  = {sdmd, s_rigger,    0, 0, 1,       0,          0,          &distrig6,  0, &distrig8,  0,    0,          0,          {7, 1, 3, 5}};
static Const fixer distrig6  = {s1x4, s_rigger,    0, 0, 1,       0,          0,          &distrig6,  0, &distrig8,  0,    0,          0,          {7, 5, 3, 1}};


static Const fixer distptp1  = {s1x4, s_ptpd,      0, 0, 1,       0,          0,          &distptp1,  0, 0,          0,    0,          0,          {3, 2, 7, 6}};
static Const fixer distptp2  = {s1x4, s_ptpd,      0, 0, 1,       0,          0,          &distptp2,  0, 0,          0,    0,          0,          {1, 2, 5, 6}};
static Const fixer distptp3  = {s1x4, s_ptpd,      0, 0, 1,       0,          0,          &distptp3,  0, &distptp7,  0,    0,          0,          {0, 1, 4, 5}};
static Const fixer distptp4  = {s1x4, s_ptpd,      0, 0, 1,       0,          0,          &distptp4,  0, &distptp8,  0,    0,          0,          {0, 3, 4, 7}};
static Const fixer distptp7  = {sdmd, s_ptpd,      0, 0, 1,       0,          0,          &distptp3,  0, &distptp7,  0,    0,          0,          {0, 1, 4, 5}};
static Const fixer distptp8  = {sdmd, s_ptpd,      0, 0, 1,       0,          0,          &distptp4,  0, &distptp8,  0,    0,          0,          {0, 7, 4, 3}};


static Const fixer disthrg1  = {s1x4, s_hrglass,   1, 0, 1,       0,          0,          &disthrg1,  0, 0,          0,    0,          0,          {1, 3, 5, 7}};
static Const fixer disthrg2  = {s1x4, s_hrglass,   1, 0, 1,       0,          0,          &disthrg2,  0, 0,          0,    0,          0,          {0, 3, 4, 7}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer d4x4l1    = {s1x4, s4x4,        1, 0, 1,       0,          0,          &d4x4l1, &d4x4l4, &d4x4d1, &d4x4d4, 0,       0,          {0, 3, 8, 11}};
static Const fixer d4x4l2    = {s1x4, s4x4,        0, 0, 1,       0,          0,          &d4x4l2, &d4x4l3, &d4x4d2, &d4x4d3, 0,       0,          {8, 11, 0, 3}};
static Const fixer d4x4l3    = {s1x4, s4x4,        1, 0, 1,       0,          0,          &d4x4l3, &d4x4l2, &d4x4d3, &d4x4d2, 0,       0,          {12, 15, 4, 7}};
static Const fixer d4x4l4    = {s1x4, s4x4,        0, 0, 1,       0,          0,          &d4x4l4, &d4x4l1, &d4x4d4, &d4x4d1, 0,       0,          {12, 15, 4, 7}};

static Const fixer d4x4d1    = {sdmd, s4x4,        1, 0, 1,       0,          0,          &d4x4l1, &d4x4l4, &d4x4d1, &d4x4d4, 0,       0,          {0, 7, 8, 15}};
static Const fixer d4x4d2    = {sdmd, s4x4,        0, 0, 1,       0,          0,          &d4x4l2, &d4x4l3, &d4x4d2, &d4x4d3, 0,       0,          {8, 15, 0, 7}};
static Const fixer d4x4d3    = {sdmd, s4x4,        1, 0, 1,       0,          0,          &d4x4l3, &d4x4l2, &d4x4d3, &d4x4d2, 0,       0,          {12, 3, 4, 11}};
static Const fixer d4x4d4    = {sdmd, s4x4,        0, 0, 1,       0,          0,          &d4x4l4, &d4x4l1, &d4x4d4, &d4x4d1, 0,       0,          {12, 3, 4, 11}};

static Const fixer fcpl12    = {s2x2, s4x4,     0x14, 0, 1,       0,          0,          0,          0, 0,          0,    &fcpl12,    0,          {1, 2, 5, 6}};
static Const fixer fcpl23    = {s2x2, s4x4,    0x3C0, 0, 1,       0,          0,          0,          0, 0,          0,    &fcpl23,    0,          {13, 14, 1, 2}};
static Const fixer fcpl34    = {s2x2, s4x4,    0x140, 0, 1,       0,          0,          0,          0, 0,          0,    &fcpl34,    0,          {13, 14, 9, 10}};
static Const fixer fcpl41    = {s2x2, s4x4,     0x3C, 0, 1,       0,          0,          0,          0, 0,          0,    &fcpl41,    0,          {9, 10, 5, 6}};

static Const fixer foo55d    = {s1x4, s1x8,        0, 0, 1,       0,          0,          &foo55d,    0, &f1x3zzd,   0,    &bar55d,    &bar55d,    {0, 2, 4, 6}};
static Const fixer fgalctb   = {s2x2, s_galaxy,    0, 0, 1,       0,          0,          0,          0, 0,          0,    &fgalctb,   &fgalctb,   {1, 3, 5, 7}};
static Const fixer f3x1ctl   = {s1x4, s3x1dmd,     0, 0, 1,       0,          0,          &f3x1ctl,   0, 0,          0,    &fgalctb,   &fgalctb,   {1, 2, 5, 6}};
static Const fixer f2x2pl    = {s2x2, s2x2,        0, 0, 1,       0,          0,          0,          0, 0,          0,    &f2x2pl,    &f2x2pl,    {0, 1, 2, 3}};
static Const fixer f1x4pl    = {s1x4, s1x4,        0, 0, 1,       0,          0,          &f1x4pl,    &f1x4pl, 0,    0,    &f2x2pl,    &f2x2pl,    {0, 1, 2, 3}};

static Const fixer f3x1d_2   = {s1x2, s3x1dmd,     1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {3, 7}};
static Const fixer f1x8_88   = {s1x2, s1x8,        0, 0, 1,       &f1x8_88,   &f3x1d_2,   0,          0, 0,          0,    0,          0,          {3, 7}};
static Const fixer f1x8_22   = {s1x2, s1x8,        0, 0, 1,       &f1x8_22,   &f3x1d_2,   0,          0, 0,          0,    0,          0,          {1, 5}};
static Const fixer f1x8_11   = {s1x2, s1x8,        0, 0, 1,       &f1x8_11,   &f3x1d_2,   0,          0, 0,          0,    0,          0,          {0, 4}};


/* Used for get-out only. */
static Const fixer fdqtagzzz = {s2x3, s_qtag,      1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0, {1, 3, 4, 5, 7, 0}};


static Const fixer f1x8_77_3 = {s1x2, s1x8,        0, 0, 3,       &f1x8_77_3, 0,          0,          0, 0,          0,    0,          0,          {0, 1, 2, 6, 5, 4}};
static Const fixer f1x8_77   = {s1x6, s1x8,        0, 0, 1,       0,          0,          &f1x8_77,   0, 0,          0,    &fdrhgl_bb, 0,          {0, 1, 2, 4, 5, 6}};
static Const fixer fdrhgl_bb = {s2x3, s_dhrglass,  0, 1, 1,       0,          0,          &f1x8_77,   0, 0,          0,    &fdrhgl_bb, &fdqtagzzz, {0, 3, 1, 4, 7, 5}};
static Const fixer f1x8_bb   = {s1x6, s1x8,        0, 0, 1,       0,          0,          &f1x8_bb,   0, 0,          0,    0,          0,          {0, 1, 3, 4, 5, 7}};
static Const fixer f1x8_dd   = {s1x6, s1x8,        0, 0, 1,       0,          0,          &f1x8_dd,   0, 0,          0,    0,          0,          {0, 3, 2, 4, 7, 6}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer foo99d    = {s1x4, s1x8,        0, 0, 1,       0,          0,          &foo99d,    0, 0,          0,    &f2x4endd,  &f2x4endd,  {0, 3, 4, 7}};
static Const fixer foo66d    = {s1x4, s1x8,        0, 0, 1,       0,          0,          &foo66d,    &f1x4xv, &f1x3yyd, 0, &bar55d,   &bar55d,    {1, 2, 5, 6}};
static Const fixer f1x8ctr   = {s1x4, s1x8,        0, 0, 1,       0,          0,          &f1x8ctr,   0, 0,          0,    &bar55d,    &bar55d,    {3, 2, 7, 6}};
static Const fixer fqtgctr   = {s1x4, s_qtag,      0, 0, 1,       0,          0,          &fqtgctr,   0, 0,          0,    &bar55d,    &bar55d,    {6, 7, 2, 3}};
static Const fixer fxwve     = {s1x4, s_crosswave, 0, 0, 1,       0,          0,          &fxwve,     &f1x8endd, 0,  0,    &f2x4endd,  &f2x4endd,  {0, 1, 4, 5}};
static Const fixer fboneendd = {s2x2, s_bone,      0, 1, 1,       0,          0,          &f1x8endd,  0, 0,          0,    &fboneendd, &fqtgend,   {0, 1, 4, 5}};
static Const fixer fqtgend   = {s2x2, s_qtag,      0, 2, 1,       0,          0,          &f1x8endd,  0, 0,          0,    &fqtgend,   &fboneendd, {0, 1, 4, 5}};
static Const fixer fdrhgl1   = {s2x2, s_dhrglass,  0, 1, 1,       0,          0,          0,          0, 0,          0,    &fdrhgl1,   &fqtgend,   {0, 1, 4, 5}};
static Const fixer f1x8endd  = {s1x4, s1x8,        0, 0, 1,       0,          0,          &f1x8endd,  &fxwve, 0,     0,    &f2x4endd,  &f2x4endd,  {0, 1, 4, 5}};
static Const fixer f1x8endo  = {s1x2, s1x8,        0, 0, 2,       &f1x8endo,  &fboneendo, 0,          0, 0,          0,    0,          0,          {0, 1, 5, 4}};

static Const fixer f1x8lowf  = {s1x4, s1x8,        0, 0, 1,       0,          0,          &f1x8lowf, &f2x4far, 0,    0,    &f2x4left,  &f2x4left,  {0, 1, 2, 3}};
static Const fixer f1x8hif   = {s1x4, s1x8,        0, 0, 1,       0,          0,          &f1x8hif,  &f2x4near, 0,   0,    &f2x4right, &f2x4right, {6, 7, 4, 5}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer fbonectr  = {s1x4, s_bone,      0, 0, 1,       0,          0,          &fbonectr,  0, 0,          0,    &bar55d,    &bar55d,    {6, 7, 2, 3}};
static Const fixer fbonetgl  = {s_bone6, s_bone,   0, 0, 1,       0,          0,          &f1x8_77,   0, 0,          0,    0,          0,          {0, 1, 3, 4, 5, 7}};
static Const fixer frigtgl   = {s_short6, s_rigger,1, 0, 1,       0,          0,          0,          &f1x8_77, 0,   0,    0,          0,          {1, 2, 4, 5, 6, 0}};
static Const fixer fboneendo = {s1x2, s_bone,      1, 0, 2,       &fboneendo, &f1x8endo,  0,          0, 0,          0,    0,          0,          {0, 5, 1, 4}};
static Const fixer frigendd  = {s1x4, s_rigger,    0, 0, 1,       0,          0,          &frigendd,  0, 0,          0,    &f2x4endd,  &f2x4endd,  {6, 7, 2, 3}};
static Const fixer frigctr   = {s2x2, s_rigger,    0, 0, 1,       0,          0,          &f1x8ctr,   0, 0,          0,    &frigctr,   &frigctr,   {0, 1, 4, 5}};
static Const fixer f2x4ctr   = {s2x2, s2x4,        0, 0, 1,       0,          0,          &fbonectr,  0, 0,          0,    &f2x4ctr,   &f2x4ctr,   {1, 2, 5, 6}};


static Const fixer f2x4lr    = {s2x2, s2x4,        1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {3, 4, 5, 2}};
static Const fixer f2x4rl    = {s2x2, s2x4,        1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 6, 7, 0}};


/* These 26 are unsymmetrical. */
       Const fixer f2x4far   = {s1x4, s2x4,        0, 0, 1,       0,          0,          &f2x4far, &f1x8hif, 0,     0,    &f2x4rl,   &f2x4rl,     {0, 1, 3, 2}};
       Const fixer f2x4near  = {s1x4, s2x4,        0, 0, 1,       0,          0,          &f2x4near, &f1x8lowf, 0,   0,    &f2x4lr,   &f2x4lr,     {7, 6, 4, 5}};

       Const fixer f4dmdiden = {s4dmd, s4dmd,      0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer f2x4pos1  = {s1x2, s2x4,        1, 0, 1,       &f2x4pos1,  0,         0,           0, 0,          0,    0,          0,          {0, 7}};
static Const fixer f2x4pos2  = {s1x2, s2x4,        1, 0, 1,       &f2x4pos2,  0,         0,           0, 0,          0,    0,          0,          {1, 6}};
static Const fixer f2x4pos3  = {s1x2, s2x4,        1, 0, 1,       &f2x4pos3,  0,         0,           0, 0,          0,    0,          0,          {2, 5}};
static Const fixer f2x4pos4  = {s1x2, s2x4,        1, 0, 1,       &f2x4pos4,  0,         0,           0, 0,          0,    0,          0,          {3, 4}};
static Const fixer f2x4pos5  = {s1x2, s2x4,        1, 0, 3,       &f2x4pos5,  0,         0,           0, 0,          0,    0,          0,          {1, 6, 2, 5, 3, 4}};
static Const fixer f2x4pos6  = {s1x2, s2x4,        1, 0, 3,       &f2x4pos6,  0,         0,           0, 0,          0,    0,          0,          {0, 7, 2, 5, 3, 4}};
static Const fixer f2x4pos7  = {s1x2, s2x4,        1, 0, 3,       &f2x4pos7,  0,         0,           0, 0,          0,    0,          0,          {0, 7, 1, 6, 3, 4}};
static Const fixer f2x4pos8  = {s1x2, s2x4,        1, 0, 3,       &f2x4pos8,  0,         0,           0, 0,          0,    0,          0,          {0, 7, 1, 6, 2, 5}};
static Const fixer f2x4posa  = {s1x2, s2x4,        0, 0, 1,       &f2x4posa,  0,         0,           0, 0,          0,    0,          0,          {0, 1}};
static Const fixer f2x4posb  = {s1x2, s2x4,        0, 0, 1,       &f2x4posb,  0,         0,           0, 0,          0,    0,          0,          {1, 2}};
static Const fixer f2x4posc  = {s1x2, s2x4,        0, 0, 1,       &f2x4posc,  0,         0,           0, 0,          0,    0,          0,          {2, 3}};
static Const fixer f2x4posd  = {s1x2, s2x4,        0, 0, 1,       &f2x4posd,  0,         0,           0, 0,          0,    0,          0,          {5, 4}};
static Const fixer f2x4pose  = {s1x2, s2x4,        0, 0, 1,       &f2x4pose,  0,         0,           0, 0,          0,    0,          0,          {6, 5}};
static Const fixer f2x4posf  = {s1x2, s2x4,        0, 0, 1,       &f2x4posf,  0,         0,           0, 0,          0,    0,          0,          {7, 6}};
static Const fixer f2x4posp  = {s1x2, s2x4,        0, 0, 2,       &f2x4posp,  0,         0,           0, 0,          0,    0,          0,          {0, 1, 6, 5}};
static Const fixer f2x4posq  = {s1x2, s2x4,        0, 0, 2,       &f2x4posq,  0,         0,           0, 0,          0,    0,          0,          {2, 3, 6, 5}};
static Const fixer f2x4posr  = {s1x2, s2x4,        0, 0, 2,       &f2x4posr,  0,         0,           0, 0,          0,    0,          0,          {1, 2, 7, 6}};
static Const fixer f2x4poss  = {s1x2, s2x4,        0, 0, 2,       &f2x4poss,  0,         0,           0, 0,          0,    0,          0,          {1, 2, 5, 4}};
static Const fixer f2x4posy  = {s1x2, s2x4,        0, 0, 3,       &f2x4posy,  0,         0,           0, 0,          0,    0,          0,          {0, 1, 2, 3, 6, 5}};
static Const fixer f2x4posz  = {s1x2, s2x4,        0, 0, 3,       &f2x4posz,  0,         0,           0, 0,          0,    0,          0,          {1, 2, 7, 6, 5, 4}};

static Const fixer f2x4left  = {s2x2, s2x4,        0, 0, 1,       0,          0,     &f1x8lowf, &f2x4far, 0,         0,    &f2x4left,  &f2x4left,  {0, 1, 6, 7}};
static Const fixer f2x4right = {s2x2, s2x4,        0, 0, 1,       0,          0,     &f1x8hif,  &f2x4near, 0,        0,    &f2x4right, &f2x4right, {2, 3, 4, 5}};
static Const fixer f2x4dleft = {s2x2, s2x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    &f2x4dleft, &f2x4dleft, {0, 2, 5, 7}};
static Const fixer f2x4dright= {s2x2, s2x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    &f2x4dright,&f2x4dright,{1, 3, 4, 6}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer f2zzrdsc =  {s2x4, s2x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 2, 3, 4, 5, 6, 7}};
static Const fixer f2zzrdsd =  {s2x4, s4x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {12, 13, 14, 0, 4, 5, 6, 8}};
static Const fixer f288rdsd =  {s1x8, s1x10,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 3, 2, 5, 6, 8, 7}};
static Const fixer f2yyrdsc =  {s1x8, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 2, 3, 4, 5, 6, 7}};
static Const fixer f2x5rdsd =  {s2x4, s2x5,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 1, 3, 4, 5, 6, 8, 9}};
static Const fixer f2qt1dsc =  {s_qtag,s3x6,       0, 0, 1,       0,          0,          0,          0, &f2qt1dsc,  0,    0,          0,          {2, 3, 6, 8, 11, 12, 15, 17}};
static Const fixer f2qt1dsd =  {s_qtag,s3x6,       0, 0, 1,       0,          0,          0,          0, &f2qt1dsd,  0,    0,          0,          {1, 4, 6, 7, 10, 13, 15, 16}};
static Const fixer f2qt2dsc =  {s_qtag,s3x8,       0, 0, 1,       0,          0,          0,          0, &f2qt2dsc,  0,    0,          0,          {3, 4, 8, 11, 15, 16, 20, 23}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static const fixer f2x6ndsc =  {s2x4, s2x6,        0, 0, 1,       0,          0,          0,      &f2yyrdsc,&f2qt1dsc,0,   0,          &f2zzrdsc,  {0, 2, 3, 5, 6, 8, 9, 11}};
static const fixer f2x6ndsd =  {s2x4, s2x6,        0, 0, 1,       0,          0,          &f288rdsd,   0, &f2qt1dsd,  0,   0,          &f2zzrdsd,  {0, 1, 4, 5, 6, 7, 10, 11}};

static const fixer fdmdndx = {s3x4, s3x6,          0, 0, 1,       0,          0,          0,           0, 0,          0,   0,               0,     {0, -1, -1, 5, 6, 8, 9, -1, -1, 14, 15, 17}};
static const fixer fdmdnd =  {s3x4, s4dmd,         0, 0, 1,       0,          0,          0,           0, 0,          0,   &fdmdndx,        0,     {0, -1, -1, 3, 5, 7, 8, -1, -1, 11, 13, 15}};

static const fixer f3x4ndsd =  {s2x4, s3x4,        0, 0, 1,       0,          0,          0,           0, 0,          0,   0,          &f2x5rdsd,  {0, 1, 2, 3, 6, 7, 8, 9}};
static const fixer f2x8ndsc =  {s2x4, s2x8,        0, 0, 1,       0,          0,          0,      &f2yyrdsc,&f2qt2dsc,0,   0,          &f2zzrdsc,  {0, 3, 4, 7, 8, 11, 12, 15}};

static Const fixer f1x8nd96 =  {s1x4, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 2, 4, 7}};
static Const fixer f1x8nd69 =  {s1x4, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 3, 5, 6}};
static Const fixer f1x8nd41 =  {s1x2, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 6}};
static Const fixer f1x8nd82 =  {s1x2, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 7}};
static Const fixer f1x8nd28 =  {s1x2, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {3, 5}};
static Const fixer f1x8nd14 =  {s1x2, s1x8,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {2, 4}};
static Const fixer f1x10ndsc =  {s1x8, s1x10,      0, 0, 1,       0,          0,          0,          &f2yyrdsc, 0,  0,    &f2zzrdsc,  &f2zzrdsc,  {0, 1, 4, 3, 5, 6, 9, 8}};
static Const fixer f1x10ndsd =  {s1x8, s1x10,      0, 0, 1,       0,          0,          0,          &f2yyrdsc, 0,  0,    &f2zzrdsc,  &f2zzrdsc,  {0, 2, 4, 3, 5, 7, 9, 8}};
static Const fixer f1x10ndse =  {s1x8, s1x10,      0, 0, 1,       0,          0,          0,          &f2yyrdsc, 0,  0,    &f2x6ndsc,  &f2zzrdsc,  {0, 1, 4, 2, 5, 6, 9, 7}};

/*                              ink   outk       rot  el numsetup 1x2         1x2rot      1x4    1x4rot dmd         dmdrot 2x2         2x2v             nonrot  */

static Const fixer z1x4u     = {s1x4, s1x4,        0, 0, 1,       0,          0,          &z1x4u,     0, 0,          0,    0,          0,          {0, 1, 2, 3}};

static Const fixer z2x3a     = {s2x3, s2x3,        0, 0, 1,       0,          0,          &z1x4u, &z1x4u, 0,         0,    &z2x3a,     0,          {0, 1, 2, 3, 4, 5}};
static Const fixer z2x3b     = {s2x3, s2x3,        0, 0, 1,       0,          0,          &z1x4u, &z1x4u, 0,         0,    &z2x3b,     0,          {0, 1, 2, 3, 4, 5}};

static Const fixer f2x4endd  = {s2x2, s2x4,        0, 1, 1,       0,          0,          &frigendd,  &frigendd, 0,  0,    &f2x4endd,  &fqtgend,   {0, 3, 4, 7}};



static Const fixer fhrgl1    = {sdmd, s_2x1dmd,   1, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {2, 3, 5, 0}};
static Const fixer fhrgl2    = {sdmd, s_2x1dmd,   0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {0, 2, 3, 5}};


static Const fixer fhrgle    = {s2x2, s_hrglass,   0, 2, 1,       0,          0,          &frigendd,  &frigendd, &fhrgl1, &fhrgl2, &fhrgle, &fqtgend, {0, 1, 4, 5}};


static Const fixer f2x477    = {s2x3, s2x4,        0, 1, 1,       0,          0,          0,          0, 0,          0,    &f2x477,    0,          {0, 1, 2, 4, 5, 6}};
static Const fixer f2x4ee    = {s2x3, s2x4,        0, 1, 1,       0,          0,          0,          0, 0,          0,    &f2x4ee,    0,          {1, 2, 3, 5, 6, 7}};
static Const fixer f2x4bb    = {s2x3, s2x4,        0, 1, 1,       0,          0,          0,          0, 0,          0,    &f2x4bb,    0,          {0, 1, 3, 4, 5, 7}};
static Const fixer f2x4dd    = {s2x3, s2x4,        0, 1, 1,       0,          0,          0,          0, 0,          0,    &f2x4dd,    0,          {0, 2, 3, 4, 6, 7}};
static Const fixer fdhrgld1  = {s1x2, s_dhrglass,  0, 1, 2,       &fdhrgld1,  0,          0,          0, 0,          0,    0,          0,          {0, 3, 7, 4}};
static Const fixer fdhrgld2  = {s1x2, s_dhrglass,  0, 1, 2,       &fdhrgld2,  0,          0,          0, 0,          0,    0,          0,          {3, 1, 5, 7}};
static Const fixer f2x4endo  = {s1x2, s2x4,        1, 0, 2,       &f2x4endo,  &f1x8endo,  0,          0, 0,          0,    0,          0,          {0, 7, 3, 4}};
static Const fixer bar55d    = {s2x2, s2x4,        0, 0, 1,       0,          0,          0,          0, 0,          0,    0,          0,          {1, 2, 5, 6}};
static Const fixer fppaad    = {s1x2, s2x4,        0, 0, 2,       &fppaad,    0,          0,          0, 0,          0,    0,          0,          {1, 3, 7, 5}};
static Const fixer fpp55d    = {s1x2, s2x4,        0, 0, 2,       &fpp55d,    0,          0,          0, 0,          0,    0,          0,          {0, 2, 6, 4}};

       const fixer fixmumble = {s1x3, s_spindle,     0x0001, 0, 2, &fixmumble, 0,         0,          0, 0,          0,    0,          0,          {3, 4, 2, 7, 0, 6}};

       const fixer fixfrotz  = {s1x3, s_dhrglass,    0x2A01, 0, 2, &fixfrotz, 0,          0,          0, 0,          0,    0,          0,          {2, 1, 4, 6, 5, 0}};

       const fixer fixwhuzzis= {s1x3, s_spindle,     0x2A01, 0, 2, &fixwhuzzis,0,         0,          0, 0,          0,    0,          0,          {7, 0, 6, 3, 4, 2}};

       const fixer fixgizmo  = {s_trngl,s_dhrglass,  0x2A03, 0, 2, &fixgizmo, 0,          0,          0, 0,          0,    0,          0,          {6, 5, 0, 2, 1, 4}};

sel_item sel_init_table[] = {
   {LOOKUP_Z,                  s2x3,        066,    &z2x3a,      (fixer *) 0, -1},
   {LOOKUP_Z,                  s2x3,        033,    &z2x3b,      (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_rigger,    0x99,   &distrig3,   (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_rigger,    0x66,   &distrig4,   (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_rigger,    0x55,   &distrig7,   (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_rigger,    0xAA,   &distrig8,   (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_ptpd,      0x33,   &distptp7,   (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_ptpd,      0x99,   &distptp8,   (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_galaxy,    0x66,   &dgald1,     (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_galaxy,    0xCC,   &dgald2,     (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_galaxy,    0x33,   &dgald3,     (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_galaxy,    0x99,   &dgald4,     (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s2x4,        0xAA,   &d2x4d1,     (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s2x4,        0x55,   &d2x4d2,     (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s4x4,        0x8181, &d4x4d1,     &d4x4d2,     0},
   {LOOKUP_DIST_DMD,           s4x4,        0x1818, &d4x4d3,     &d4x4d4,     4},
   {LOOKUP_DIST_DMD,           s_qtag,      0x66,   &ddmd1,      (fixer *) 0, -1},
   {LOOKUP_DIST_DMD,           s_qtag,      0x55,   &ddmd2,      (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s2x4,        0xAA,   &d2x4c1,     (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s2x4,        0x55,   &d2x4c2,     (fixer *) 0, -1},
   {LOOKUP_DIST_BOX|LOOKUP_DIAG_BOX, s2x4,  0x33,   &d2x4y1,     (fixer *) 0, -1},
   {LOOKUP_DIST_BOX|LOOKUP_DIAG_BOX, s2x4,  0xCC,   &d2x4y2,     (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0x560,   &d3x4ub1,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0xA90,   &d3x4ub2,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0x42A,   &d3x4ub3,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0x815,   &d3x4ub4,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0xCC0,   &d3x4vb1,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0x330,   &d3x4vb2,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0xC0C,   &d3x4vb3,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s3x4,       0x033,   &d3x4vb4,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s_qtag,      0xAA,   &fqtgjj1,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s_qtag,      0x99,   &fqtgjj2,    (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s_spindle,   0x66,   &fspindlfd,  (fixer *) 0, -1},
   {LOOKUP_DIST_BOX,           s_spindle,   0x33,   &fspindlgd,  (fixer *) 0, -1},
   {LOOKUP_DIAG_CLW|LOOKUP_DIST_CLW, s4x4,  0x0909, &d4x4l1,     &d4x4l2,     0},
   {LOOKUP_DIAG_CLW|LOOKUP_DIST_CLW, s4x4,  0x9090, &d4x4l3,     &d4x4l4,     4},
   {LOOKUP_DIST_CLW,           s_rigger,    0x99,   &distrig1,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_rigger,    0x66,   &distrig2,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_rigger,    0x55,   &distrig5,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_rigger,    0xAA,   &distrig6,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_ptpd,      0xCC,   &distptp1,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_ptpd,      0x66,   &distptp2,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_ptpd,      0x33,   &distptp3,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_ptpd,      0x99,   &distptp4,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_bone,      0x55,   &distbone1,  (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_bone,      0x99,   &distbone2,  (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_bone,      0x66,   &distbone5,  (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_bone,      0xAA,   &distbone6,  (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_hrglass,   0xAA,   &disthrg1,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_hrglass,   0x99,   &disthrg2,   (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_galaxy,    0x66,   &dgalw1,     (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_galaxy,    0xCC,   &dgalw2,     (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_galaxy,    0x33,   &dgalw3,     (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s_galaxy,    0x99,   &dgalw4,     (fixer *) 0, -1},
   {LOOKUP_OFFS_CLW|LOOKUP_DIST_CLW, s2x4,  0x33,   &d2x4w1,     (fixer *) 0, -1},
   {LOOKUP_OFFS_CLW|LOOKUP_DIST_CLW, s2x4,  0xCC,   &d2x4w2,     (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s2x4,        0xAA,   &d2x4x1,     (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s2x4,        0x55,   &d2x4x2,     (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s3x4,       0x560,   &d3x4ul1,    (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s3x4,       0xA90,   &d3x4ul2,    (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s3x4,       0x42A,   &d3x4ul3,    (fixer *) 0, -1},
   {LOOKUP_DIST_CLW,           s3x4,       0x815,   &d3x4ul4,    (fixer *) 0, -1},
   {LOOKUP_OFFS_CLW|LOOKUP_DIST_CLW, s3x4, 0xCC0,   &d3x4vl1,    (fixer *) 0, -1},
   {LOOKUP_OFFS_CLW|LOOKUP_DIST_CLW, s3x4, 0x330,   &d3x4vl2,    (fixer *) 0, -1},
   {LOOKUP_OFFS_CLW|LOOKUP_DIST_CLW, s3x4, 0xC0C,   &d3x4vl3,    (fixer *) 0, -1},
   {LOOKUP_OFFS_CLW|LOOKUP_DIST_CLW, s3x4, 0x033,   &d3x4vl4,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x6,         033,   &f1x6aad,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0xAA,   &f1x8aad,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x55,   &foo55d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x99,   &foo99d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x66,   &foo66d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x33,   &f1x8endd,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x88,   &f1x8_88,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x22,   &f1x8_22,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x11,   &f1x8_11,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0x77,   &f1x8_77,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0xBB,   &f1x8_bb,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x8,        0xDD,   &f1x8_dd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s3x4,       03131,   &f3x4outrd,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s3dmd,      00707,   &f3dmoutrd,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_dhrglass,  0x77,   &fdhrgld,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x12,      00707,   &f1x12outrd, (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x12,      06363,   &f1x12outre, (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_323,       0x77,   &f323d,      (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_bone,      0x33,   &fboneendd,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_bone,      0xBB,   &fbonetgl,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_rigger,    0x77,   &frigtgl,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_qtag,      0x33,   &fqtgend,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_bone6,      033,   &fo6zzd,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_ptpd,      0xAA,   &foozzd,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_ptpd,      0x55,   &fptpzzd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_dhrglass,  0x33,   &fdrhgl1,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s3x1dmd,     0x99,   &f3x1zzd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s3x1dmd,     0xAA,   &f3x1yyd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_2x1dmd,     055,   &f2x1yyd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x3dmd,     0x99,   &f1x3zzd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x3dmd,     0xAA,   &f1x3yyd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s1x3dmd,     0x66,   &f1x3bbd,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_crosswave, 0x55,   &fxwv1d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_crosswave, 0x99,   &fxwv2d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_crosswave, 0x66,   &fxwv3d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_crosswave, 0x33,   &fxwve,      (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_spindle,   0x55,   &fspindld,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_spindle,   0xAA,   &fspindlbd,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_spindle,   0xDD,   &fspindlod,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_dhrglass,  0xBB,   &fdrhgl_bb,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0xAA,   &fppaad,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0x55,   &fpp55d,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0xA5,   &f2x4dleft,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0x5A,   &f2x4dright, (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0x99,   &f2x4endd,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_hrglass,   0x33,   &fhrgle,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_dhrglass,  0x99,   &fdhrgld1,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s_dhrglass,  0xAA,   &fdhrgld2,   (fixer *) 0, -1},
   {LOOKUP_IGNORE,             s2x4,        0x77,   &f2x477,     (fixer *) 0, -1},  /* 2x4 with 2 corners ignored.  These 2 are *NOT* disconnected. */
   {LOOKUP_IGNORE,             s2x4,        0xEE,   &f2x4ee,     (fixer *) 0, -1},  /* 2x4 with 2 centers ignored.  These 2 *ARE* disconnected (or ignored). */
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0xBB,   &f2x4bb,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE, s2x4,        0xDD,   &f2x4dd,     (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s1x8,     0xCC, &f1x8ctr,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s_qtag,   0xCC, &fqtgctr,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s_bone,   0xCC, &fbonectr,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s_rigger, 0x33, &frigctr,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s3x1dmd,  0x66, &f3x1ctl,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s1x4,     0x0F, &f1x4pl,    (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x66, &f2x4ctr,   (fixer *) 0, -1},

   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x0F, &f2x4far,   (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xF0, &f2x4near,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x81, &f2x4pos1,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x42, &f2x4pos2,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x24, &f2x4pos3,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x18, &f2x4pos4,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x7E, &f2x4pos5,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xBD, &f2x4pos6,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xDB, &f2x4pos7,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xE7, &f2x4pos8,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x03, &f2x4posa,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x06, &f2x4posb,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x0C, &f2x4posc,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x30, &f2x4posd,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x60, &f2x4pose,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xC0, &f2x4posf,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x63, &f2x4posp,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x6C, &f2x4posq,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xC6, &f2x4posr,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x36, &f2x4poss,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x6F, &f2x4posy,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xF6, &f2x4posz,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0xC3, &f2x4left,  (fixer *) 0, -1},
   {LOOKUP_DISC|LOOKUP_IGNORE|LOOKUP_NONE, s2x4,     0x3C, &f2x4right, (fixer *) 0, -1},
   {LOOKUP_DISC,                           s2x6,    05555, &f2x6ndsc,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s2x6,    06363, &f2x6ndsd,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s3x4,    01717, &f3x4ndsd,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s2x8,   0x9999, &f2x8ndsc,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s1x8,     0x96, &f1x8nd96,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s1x8,     0x69, &f1x8nd69,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s1x8,     0x41, &f1x8nd41,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s1x8,     0x82, &f1x8nd82,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s1x8,     0x28, &f1x8nd28,  (fixer *) 0, -1},
   {LOOKUP_DISC,                           s1x8,     0x14, &f1x8nd14,  (fixer *) 0, -1},
   {LOOKUP_DISC,                          s1x10,    0x37B, &f1x10ndsc, (fixer *) 0, -1},
   {LOOKUP_DISC,                          s1x10,    0x3BD, &f1x10ndsd, (fixer *) 0, -1},
   {LOOKUP_DISC,                          s1x10,    0x2F7, &f1x10ndse, (fixer *) 0, -1},
   {LOOKUP_DISC,                          s4dmd,   0xA9A9, &fdmdnd,    (fixer *) 0, -1},
   {LOOKUP_DISC,                           s3x6,  0x2C361, &fdmdndx,   (fixer *) 0, -1},
   {LOOKUP_NONE,                           s4x4,   0x0066, &fcpl12,     (fixer *) 0, -1},
   {LOOKUP_NONE,                           s4x4,   0x6006, &fcpl23,     (fixer *) 0, -1},
   {LOOKUP_NONE,                           s4x4,   0x6600, &fcpl34,     (fixer *) 0, -1},
   {LOOKUP_NONE,                           s4x4,   0x0660, &fcpl41,     (fixer *) 0, -1},
   {LOOKUP_NONE,                           s_thar, 0x00CC, &ftharns,    (fixer *) 0, -1},
   {LOOKUP_NONE,                           s_thar, 0x0033, &ftharew,    (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x33,   &foo33,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x5A,   &foo5a,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0xA5,   &fooa5,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x55,   &foo55,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0xAA,   &fooaa,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x88,   &foo88,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x44,   &foo44,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x22,   &foo22,      (fixer *) 0, -1},
   {LOOKUP_NONE,                           s2x4,   0x11,   &foo11,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x4,        0x3,    &n1x43,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x4,        0xC,    &n1x4c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x4,        0x5,    &n1x45,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x4,        0xA,    &n1x4a,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x3,        0x3,    &n1x3a,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x3,        0x6,    &n1x3b,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x4,        0xCC,   &foocc,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x4,        0x99,   &f2x4endo,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x6,         033,   &f1x6aa,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x8,        0xAA,   &f1x8aa,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x8,        0x55,   &f1x855,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s3x4,        03131,  &f3x4outer,  (fixer *) 0, -1},
   {LOOKUP_NONE,               s3dmd,       00707,  &f3dmouter,  (fixer *) 0, -1},
   {LOOKUP_NONE,               s_dhrglass,   0x77,  &fdhrgl,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s_525,       00707,  &f525nw,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s_525,       03434,  &f525ne,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s_323,        0x77,  &f323,       (fixer *) 0, -1},
   {LOOKUP_NONE,               sdeepxwv,    00303,  &fdpxwve,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x3dmd,      0x77,  &f1x3d6,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x12,       00707,  &f1x12outer, (fixer *) 0, -1},
   {LOOKUP_NONE,               s3x4,        00303,  &f3x4left,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s3x4,        01414,  &f3x4right,  (fixer *) 0, -1},
   {LOOKUP_NONE,               s3x4,        03030,  &f3x4east,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s3x4,        02121,  &f3x4west,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s4x4,       0x3030,  &f4x4left,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s4x4,       0x4141,  &f4x4right,  (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x6,        0x0C3,  &f3x4lzz,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x6,        0xC30,  &f3x4rzz,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x8,       0x0303,  &f4x4lzz,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x8,       0xC0C0,  &f4x4rzz,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x8,        0x33,   &f1x8endo,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x8,        0x77,   &f1x8_77_3,  (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x8,        0x0F,   &f1x8lowf,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x8,        0xF0,   &f1x8hif,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s_bone,      0x33,   &fboneendo,  (fixer *) 0, -1},
   {LOOKUP_NONE,               s_ptpd,      0xAA,   &foozz,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s_spindle,   0x55,   &fspindlc,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s_spindle,   0x66,   &fspindlf,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s_spindle,   0x33,   &fspindlg,   (fixer *) 0, -1},
   {LOOKUP_NONE,               s_galaxy,    0x44,   &fgalcv,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s_galaxy,    0x11,   &fgalch,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s1x3dmd,     0x66,   &f1x3aad,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s_2x1dmd,    066,    &f2x166d,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s_1x2dmd,    033,    &f1x2aad,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s3ptpd,      06262,  &f3ptpo6,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x3,        055,    &f2x3c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x3,        041,    &f2x3a41,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x3,        014,    &f2x3a14,    (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x5,        0x318,  &f2x5c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x5,        0x063,  &f2x5d,      (fixer *) 0, -1},
   {LOOKUP_NONE,               sd2x5,       0x18C,  &fd2x5d,     (fixer *) 0, -1},
   {LOOKUP_NONE,               sd2x5,       0x318,  &fd2x5d,     (fixer *) 0, -1},
   {LOOKUP_NONE,               sd2x5,       0x198,  &fd2x5d,     (fixer *) 0, -1},
   {LOOKUP_NONE,               sd2x5,       0x30C,  &fd2x5d,     (fixer *) 0, -1},
   {LOOKUP_NONE,               sd2x5,       0x294,  &fd2x5d,     (fixer *) 0, -1},

   // Of course, we need lots more entries for 4x6!
   {LOOKUP_NONE,               s4x6,      0x003003,  &f4x6a,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s4x6,      0xC00C00,  &f4x6b,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s4x6,      0x0C00C0,  &f4x6c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s4x6,      0x030030,  &f4x6d,      (fixer *) 0, -1},

   {LOOKUP_NONE,               s2x2,        0x3,    &box3c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x2,        0x6,    &box6c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x2,        0x9,    &box9c,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x2,        0xC,    &boxcc,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x2,        0x5,    &box55,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x2,        0xA,    &boxaa,      (fixer *) 0, -1},
   {LOOKUP_NONE,               s_qtag,      0xAA,   &fqtgj1,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s_qtag,      0x99,   &fqtgj2,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x3,         033,   &f2x3j1,     (fixer *) 0, -1},
   {LOOKUP_NONE,               s2x3,         066,   &f2x3j2,     (fixer *) 0, -1},
   {LOOKUP_NONE,               nothing}};


/* These need to be predeclared so that they can refer to each other. */
static Const tgl_map map2b;
static Const tgl_map map2i;
static Const tgl_map map2d;
static Const tgl_map map2m;
static Const tgl_map map2j;
static Const tgl_map map2k;
static Const tgl_map map2x;
static Const tgl_map map2y;


/*
   In C1 phantom: first triangle (inverted),
 then second triangle (upright), then idle.
*/

static Const tgl_map map1b = {s_c1phan, s2x4, &map2b, 0, 0,
   {4, 3, 2,   0, 7, 6,   1, 5},
   {6, 8, 10,  14,0, 2,   4, 12},
   {10, 9, 8,  4, 3, 2,   5, 11},
   {6, 5, 4, 2, 1, 0, 3, 7},
   {10, 9, 8, 4, 3, 2, 5, 11}};

static Const tgl_map map2b = {s_qtag, s2x4, &map1b, 0, 1,
   {5, 6, 7,   1, 2, 3,   0, 4},
   {3, 15, 13, 11,7, 5,   1,  9},
   {1, 2, 3,   7, 8, 9,   0, 6},
   {7, 6, 5, 3, 2, 1, 0, 4},
   {1, 2, 3, 7, 8, 9, 0, 6}};

static Const tgl_map map1i = {nothing, nothing, &map2i, 0, 0,   // Interlocked
   {0, 0, 0,   0, 0, 0,   0, 0},
   {4, 8, 10,  12,0, 2,   6, 14},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static Const tgl_map map2i = {nothing, nothing,  &map1i, 0, 0,   // Interlocked
   {0, 0, 0,   0, 0, 0,   0, 0},
   {1, 15, 13, 9, 7, 5,   3, 11},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};




// Maps for deepbigqtg setups.

static Const tgl_map map1d = {s2x8, s2x8, &map2d, 0, 0,
   {4, 3, 2,   0, 7, 6,     1, 5},
   {2, 6, 7,   10,14, 15,   0, 8},   // OK
   {10, 9, 8,  4, 3, 2,     5, 11},
   {1, 2, 3,   9, 10, 11,   8, 0},   // OK
   {10, 9, 8, 4, 3, 2, 5, 11}};

static Const tgl_map map2d = {s2x8, s2x8, &map1d, 0, 0,
   {4, 3, 2,   0, 7, 6,   1, 5},
   {3, 4, 5,   11,12,13,  1, 9},    // OK
   {10, 9, 8,  4, 3, 2,   5, 11},
   {14,13,12,  6, 5, 4,   7, 15},   // OK
   {10, 9, 8, 4, 3, 2, 5, 11}};

static Const tgl_map map1m = {nothing, s2x8, &map2m, 1, 0,   // Interlocked
   {5, 6, 7,   1, 2, 3,   0, 4},
   {0, 6, 7,   8,14,15,  2, 10},   // OK
   {1, 2, 3,   7, 8, 9,   0, 6},
   {7, 6, 5, 3, 2, 1, 0, 4},
   {1, 2, 3, 7, 8, 9, 0, 6}};

static Const tgl_map map2m = {nothing, s2x8, &map1m, 1, 0,   // Interlocked
   {5, 6, 7,   1, 2, 3,   0, 4},
   {1, 4, 5,   9,12,13,  3, 11},   // OK
   {1, 2, 3,   7, 8, 9,   0, 6},
   {7, 6, 5, 3, 2, 1, 0, 4},
   {1, 2, 3, 7, 8, 9, 0, 6}};






// Interlocked triangles in quarter-tag:
static Const tgl_map map1j = {s_qtag, nothing, &map2j, 1, 1,
   {4, 7, 2,   0, 3, 6,   1, 5},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static Const tgl_map map2j = {s_qtag, nothing, &map1j, 1, 1,
   {5, 6, 3,   1, 2, 7,   0, 4},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

// Magic triangles in quarter-tag:
static Const tgl_map map1x = {s_qtag, nothing, &map2x, 1, 1,
   {4, 6, 7,   0, 2, 3,   1, 5},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static Const tgl_map map2x = {s_qtag, nothing, &map1x, 1, 1,
   {5, 3, 2,   1, 7, 6,   0, 4},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

// Magic interlocked triangles in quarter-tag:
static Const tgl_map map1y = {s_qtag, nothing, &map2y, 1, 1,
   {4, 6, 3,   0, 2, 7,   1, 5},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

static Const tgl_map map2y = {s_qtag, nothing, &map1y, 1, 1,
   {5, 7, 2,   1, 3, 6,   0, 4},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0},
   {0, 0, 0, 0, 0, 0, 0, 0}};

// Interlocked triangles in bigdmd:
static Const tgl_map map1k = {nothing, nothing, &map2k, 1, 0,
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {10, 3, 8,  4, 9, 2,   5, 11},
   {0},
   {0}};

static Const tgl_map map2k = {nothing, nothing, &map1k, 1, 0,
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {1, 2, 9,   7, 8, 3,   0, 6},
   {0},
   {0}};

static Const tgl_map map2r = {nothing, nothing, &map2r, 1, 0,
   {0, 0, 0,   0, 0, 0,   0, 0},
   {0, 0, 0,   0, 0, 0,   0, 0},
   {7, 1, 4,   3, 5, 0,   2, 6},
   {0},
   {0}};

       Const tgl_map *c1tglmap1[2] = {&map1b, &map1i};
       Const tgl_map *c1tglmap2[2] = {&map2b, &map2i};

       Const tgl_map *dbqtglmap1[2] = {&map1d, &map1m};
       Const tgl_map *dbqtglmap2[2] = {&map2d, &map2m};

       Const tgl_map *qttglmap1[4] = {&map1b, &map1j, &map1x, &map1y};
       Const tgl_map *qttglmap2[4] = {&map2b, &map2j, &map2x, &map2y};
       Const tgl_map *bdtglmap1[2] = {&map1b, &map1k};
       Const tgl_map *bdtglmap2[2] = {&map2b, &map2k};
       Const tgl_map *rgtglmap1[2] = {&map2r, &map2r};


/* In the print_strings tables below, characters have the following meanings:
   a-z  draw a person, this will be 4 characters wide in ASCII
   6    blank person space, whether in ASCII (4 blanks) or checkers
   5    1/2 person space (2 blanks if in ASCII)
   9    3/4 person space (3 blanks if in ASCII)
   8    1/2 person space with checkers, but only one blank (not 2) if ASCII
   @7   only 1/2 line feed if using checkers, must be careful not to overlap
*/

/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
setup_attr setup_attrs[] = {
   /* nothing */
   {-1,                           /* setup_limits */
    (coordrec *) 0,               /* setup_coords */
    (coordrec *) 0,               /* nice_setup_coords */
    0, 0, 0, 0,                   /* concentric masks:
                                     mask_normal,
                                     mask_6_2,
                                     mask_2_6, and
                                     mask_ctr_dmd */
    {b_nothing,   b_nothing},     /* keytab */
    { 0, 0},                      /* bounding_box */
    FALSE,                        /* four_way_symmetry */
    (id_bit_table *) 0,           /* id_bit_table_ptr */
    {  (Cstring) 0,               /* print_strings */
       (Cstring) 0}},
   /* s1x1 */
   { 0,
     &thing1x1,
     &thing1x1,
     0, 0, 0, 0,
     {b_1x1,       b_1x1},
     { 1, 1},
     TRUE,
     (id_bit_table *) 0,
     {  "a@",
        (Cstring) 0}},
   /* s1x2 */
   { 1,
     &thing1x2,
      &thing1x2,
      0, 0, 0, 0,
      {b_1x2,       b_2x1},
      { 2, 1},
      FALSE,
      id_bit_table_1x2,
      {  "a  b@",
         "a@b@"}},
   /* s1x3 */
      { 2,
      &thing1x3,
      &thing1x3,
      0, 0, 0, 0,
      {b_1x3,       b_3x1},
      { 3, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c@",
         "a@b@c@"}},
   /* s2x2 */
      { 3,
      &thing2x2,
      &thing2x2,
      0, 0, 0, 0,
      {b_2x2,       b_2x2},
      { 2, 2},
      TRUE,
      id_bit_table_2x2,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s1x4 */
      { 3,
      &thing1x4,
      &thing1x4,
      0x5, 0, 0, 0,
      {b_1x4,       b_4x1},
      { 4, 1},
      FALSE,
      id_bit_table_1x4,
      {  "a  b  d  c@",
         "a@b@d@c@"}},
   /* sdmd */
      { 3,
      &thingdmd,
      &nicethingdmd,
      0x5, 0, 0, 0,
      {b_dmd,       b_pmd},
      { 0, 2},
      FALSE,
      id_bit_table_dmd,
      {  "6 b@7a 6 c@76 d@",
         " 5a@@ db@@ 5c@"}},
   /* s_star */
      { 3,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_star,      b_star},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "5 b@a  c@5 d@",
         (Cstring) 0}},
   /* s_trngl */
      { 2,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_trngl,     b_ptrngl},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_trngl4 */
      { 3,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_trngl4,    b_ptrngl4},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_bone6 */
      { 5,
      &thingbone6,
      &thingbone6,
      0, 0, 0, 0,
      {b_bone6,     b_pbone6},
      { 0, 0},
      FALSE,
      id_bit_table_bone6,
      {  "a6 6b@76f c@7e6 6d@",
         "ea@5f@5c@db@"}},
   /* s_short6 */
      { 5,
      &thingshort6,
      &thingshort6,
      0, 0, 0, 0,
      {b_short6,    b_pshort6},
      { 0, 0},
      FALSE,
      id_bit_table_short6,
      {  "5 b@a  c@f  d@5 e@",
         "5 fa@e 6 b@5 dc@"}},
   /* s1x6 */
      { 5,
      &thing1x6,
      &thing1x6,
      0, 0, 0, 0,
      {b_1x6,       b_6x1},
      { 6, 1},
      FALSE,
      id_bit_table_1x6,
      {  "a  b  c  f  e  d@",
         "a@b@c@f@e@d@"}},
   /* s2x3 */
      { 5,
      &thing2x3,
      &thing2x3,
      0, 0, 0, 0,
      {b_2x3,       b_3x2},
      { 3, 2},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c@f  e  d@",
         "f  a@e  b@d  c@"}},
   /* s_1x2dmd */
      { 5,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_1x2dmd,    b_p1x2dmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6  6 c@7a  b 6 e  d@76  6 f@",
         "5 a@@5 b@@f  c@@5 e@@5 d@"}},
   /* s_2x1dmd */
      { 5,
      &thing2x1dmd,
      &thing2x1dmd,
      0, 0, 0, 0,
      {b_2x1dmd,    b_p2x1dmd},
      { 0, 0},
      FALSE,
      id_bit_table_2x1dmd,
      {  "6  5 c@@a  b  e  d@@6  5 f",
         "6  a@@6  b@7f  6  c@76  e@@6  d"}},
   /* s_qtag */
      { 7,
      &thingqtag,
      &nicethingqtag,
      0x33, 0xDD, 0x11, 0,
      {b_qtag,      b_pqtag},
      { 4, 0},
      FALSE,
      id_bit_table_qtag,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_bone */
      { 7,
      &thingbone,
      &thingbone,
      0x33, 0, 0x11, 0,
      {b_bone,      b_pbone},
      { 0, 0},
      FALSE,
      id_bit_table_bone,
      {  "a6 6 6 6b@76g h d c@7f6 6 6 6e",
         "fa@5g@5h@5d@5c@eb"}},
   /* s1x8 */
      { 7,
      &thing1x8,
      &thing1x8,
      0x33, 0x77, 0x22, 0,
      {b_1x8,       b_8x1},
      { 8, 1},
      FALSE,
      id_bit_table_1x8,
      {  "a b d c g h f e",
         "a@b@d@c@g@h@f@e"}},
   /* slittlestars */
      {7,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_2stars */
      {7,
       &thing2stars,
       &nicething2stars,
       /*
         0x33, 0xDD, 0x11, 0,
       */
      0, 0, 0, 0,
      {b_2stars,   b_p2stars},
      { 0, 0},
      FALSE,
      id_bit_table_2stars,
      {  "  8a6 b@g8hd8c@  8f6 e",
         "9g@f8a@9h@@9d@e8b@9c"}},
   /* s1x3dmd */
      { 7,
      &thing1x3dmd,
      &thing1x3dmd,
      0x33, 0x77, 0x11, 0,
      {b_1x3dmd,    b_p1x3dmd},
      { 0, 0},
      FALSE,
      id_bit_table_1x3dmd,
      {  "6 6 6 d@7a b c 6 g f e@76 6 6 h",
         " 5a@@ 5b@@ 5c@@ hd@@ 5g@@ 5f@@ 5e@"}},
   /* s3x1dmd */
      { 7,
      &thing3x1dmd,
      &thing3x1dmd,
      0x33, 0xEE, 0, 0,
      {b_3x1dmd,    b_p3x1dmd},
      { 0, 0},
      FALSE,
      id_bit_table_3x1dmd,
        {  "6 6 9d@@a b c g f e@@6 6 9h",  /* not quite symmetrical, unfortunately. */
         "6  a@@6  b@@6  c@7h  6  d@76  g@@6  f@@6  e"}},
   /* s_spindle */
      { 7,
      &thingspindle,
      &thingspindle,
      0, 0xEE, 0x44, 0,
      {b_spindle,   b_pspindle},
      { 0, 0},
      FALSE,
      id_bit_table_spindle,
      {  "6a b c@7h6 6 6d@76g f e",
         "5h@ga@fb@ec@5d"}},
   /* s_hrglass */
      { 7,
      &thingglass,
      &nicethingglass,
      0x33, 0xDD, 0x11, 0,
      {b_hrglass,   b_phrglass},
      { 0, 0},
      FALSE,
      id_bit_table_hrglass,
      {  "9a5b@56d@7g66c@756h@9f5e",
         "6 g@7f 6 a@5 hd@e 6 b@76 c"}},
   /* s_dhrglass */
      { 7,
      &thingdglass,
      &thingdglass,
      0x33, 0, 0x11, 0,
      {b_dhrglass,  b_pdhrglass},
      { 0, 0},
      FALSE,
      id_bit_table_dhrglass,
      {  "a 6 d 6 b@6 g 6 c@f 6 h 6 e",
         "f  a@@5 g@@h  d@@5 c@@e  b"}},
   /* s_hyperglass */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_crosswave */
      { 7,
      &thingxwv,
      &thingxwv,
      0x33, 0x77, 0x11, 0x55,
      {b_crosswave, b_pcrosswave},
      { 0, 0},
      FALSE,
      id_bit_table_crosswave,
      {  "66  c@66  d@7ab  6  fe@766  h@66  g",
         "65a@65b@@ghdc@@65f@65e"}},
   /* s2x4 */
      { 7,
      &thing2x4,
      &thing2x4,
      0x66, 0, 0, 0,
      {b_2x4,       b_4x2},
      { 4, 2},
      FALSE,
      id_bit_table_2x4,
      {  "a  b  c  d@@h  g  f  e",
         "h  a@@g  b@@f  c@@e  d"}},
   /* s2x5 */
      {9,
      &thing2x5,
      &thing2x5,
      0, 0, 0, 0,
      {b_2x5,       b_5x2},
      { 5, 2},
      FALSE,
      id_bit_table_2x5,
      {  "a  b  c  d  e@@j  i  h  g  f",
         "j  a@@i  b@@h  c@@g  d@@f  e"}},
   //========================================
   /* sd2x5 */
      {9,
      &thingd2x5,
      &thingd2x5,
      0, 0, 0, 0,
      {b_d2x5,       b_5x2},
      { 4, 3},
      FALSE,
      (id_bit_table *) 0,
      {  "58c  j@7a6 6g@758d  i@7b6 6f@758e  h",
         "6  b  a@@5 e  d  c@@5 h  i  j@@6  f  g"}},
   /* swqtag */
      {9,
      &thingwqtag,
      &nicethingwqtag,
      0, 0, 0, 0,
      {b_wqtag,       b_pwqtag},
      { 6, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6 5 a6 b@@h i j e d c@@6 5 g6 f",
         "6  h@@6  i@7g  6  a@76  j@@6  e@7f  6  b@76  d@@6  c"}},
   /* sdeep2x1dmd */
      {9,
      &thingdeep2x1dmd,
      &thingdeep2x1dmd,
      0, 0, 0, 0,
      {b_deep2x1dmd,       b_pdeep2x1dmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6  5 c@@a  b  d  e@@j  i  g  f@@6  5 h",
         "6  j a@@6  i b@7h  6 6  c@76  g d@@6  f e"}},
   /* swhrglass */
      {9,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_whrglass,       b_pwhrglass},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "9a5b@56d@7g66c@756h@9f5e",
         "6 g@7f 6 a@5 hd@e 6 b@76 c"}},

   //========================================
   /* s_rigger */
      { 7,
      &thingrigger,
      &thingrigger,
      0xCC, 0xDD, 0, 0,
      {b_rigger,    b_prigger},
      { 0, 0},
      FALSE,
      id_bit_table_rigger,
      {  "66a b@7gh6 6dc@766f e",
         "5g@5h@fa@eb@5d@5c"}},
   /* s3x4 */
      {11,
      &thing3x4,
      &thing3x4,
      0, 0, 0x041, 0,         /* Only used if occupied as "H" */
      {b_3x4,       b_4x3},
      { 4, 3},
      FALSE,
      id_bit_table_3x4,
      {  "a  b  c  d@@k  l  f  e@@j  i  h  g",
         "j  k  a@@i  l  b@@h  f  c@@g  e  d"}},
   /* s2x6 */
      {11,
      &thing2x6,
      &thing2x6,
      0, 0, 0, 0,
      {b_2x6,       b_6x2},
      { 6, 2},
      FALSE,
      id_bit_table_2x6,
      {  "a  b  c  d  e  f@@l  k  j  i  h  g",
         "l  a@@k  b@@j  c@@i  d@@h  e@@g  f"}},
   /* s2x7 */
      {13,
      &thing2x7,
      &thing2x7,
      0, 0, 0, 0,
      {b_2x7,       b_7x2},
      { 7, 2},
      FALSE,
      id_bit_table_2x7,
      {  "a  b  c  d  e  f  g@@n  m  l  k  j  i  h",
         "n  a@@m  b@@l  c@@k  d@@j  e@@i  f@@h  g"}},
   /* d3x4 */
      {11,
      &thing_d3x4,
      &thing_d3x4,
      0, 0, 0, 0,
      {b_d3x4,   b_d4x3},
      { 5, 3},
      FALSE,
      id_bit_table_d3x4,
      {  "a6 6 6e@758b  c  d@7l6 6 6f@758j  i  h@7k6 6 6g",
         "k  l  a@@5 j  b@@5 i  c@@5 h  d@@g  f  e"}},
   /* s1p5x8 */
      {15,
      (coordrec *) 0,   // Code in anchor_someone_and_move requires that this
      (coordrec *) 0,   // setup look a lot like a 2x8.
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h@p  o  n  m  l  k  j  i",
         "pa@@ob@@nc@@md@@le@@kf@@jg@@ih"}},
   /* s2x8 */
      {15,
      &thing2x8,
      &thing2x8,
      0, 0, 0, 0,
      {b_2x8,       b_8x2},
      { 8, 2},
      FALSE,
      id_bit_table_2x8,
      {  "a  b  c  d  e  f  g  h@@p  o  n  m  l  k  j  i",
         "p  a@@o  b@@n  c@@m  d@@l  e@@k  f@@j  g@@i  h"}},
   /* s4x4 */
      {15,
      &thing4x4,
      &thing4x4,
      0x1111, 0, 0, 0,        /* Only used if occupied as butterfly */
      {b_4x4,       b_4x4},
      { 4, 4},
      TRUE,
      id_bit_table_4x4,
      {  "m  n  o  a@@k  p  d  b@@j  l  h  c@@i  g  f  e",
         (Cstring) 0}},
   /* s1x10 */
      { 9,
      &thing1x10,
      &thing1x10,
      0, 0, 0, 0,
      {b_1x10,      b_10x1},
      {10, 1},
      FALSE,
      id_bit_table_1x10,
      {  "a b c d e j i h g f",
         "a@b@c@d@e@j@i@h@g@f"}},
   /* s1x12 */
      {11,
      &thing1x12,
      &thing1x12,
      0, 0, 0x041, 0,
      {b_1x12,      b_12x1},
      {12, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "a b c d e f l k j i h g",
         "a@b@c@d@e@f@l@k@j@i@h@g"}},
   /* s1x14 */
      {13,
      &thing1x14,
      &thing1x14,
      0, 0, 0, 0,
      {b_1x14,      b_14x1},
      {14, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "abcdefgnmlkjih",
         "a@b@c@d@e@f@g@n@m@l@k@j@i@h"}},
   /* s1x16 */
      {15,
      &thing1x16,
      &thing1x16,
      0, 0, 0, 0,
      {b_1x16,      b_16x1},
      {16, 1},
      FALSE,
      (id_bit_table *) 0,
      {  "abcdefghponmlkji",
         "a@b@c@d@e@f@g@h@p@o@n@m@l@k@j@i"}},
   /* s_c1phan */
      {15,
      &thingphan,
      &nicethingphan,
      0, 0, 0, 0,
      {b_c1phan,    b_c1phan},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_hyperbone */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_bigblob */
      {23,
      &thingblob,
      &thingblob,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "6  6  a  b@@6  v  w  c  d@@t  u  x  f  e  g@@s  q  r  l  i  h@@6  p  o  k  j@@6  6  n  m",
         (Cstring) 0}},
   /* s_ptpd */
      { 7,
      &thingptpd,
      &nicethingptpd,
      0, 0x77, 0x22, 0,
      {b_ptpd,      b_pptpd},
      { 0, 0},
      FALSE,
      id_bit_table_ptpd,
      {  "6b6   6h@7a6c   g6e@76d6   6f",
         "5a@@db@@5c@@5g@@fh@@5e"}},
   /* s3dmd */
      {11,
      &thing3dmd,
      &thing3dmd,
      0, 0, 0x041, 0,        /* Only used if occupied as center 1x2 and outer 1x3's */
      {b_3dmd,      b_p3dmd},
      { 0, 0},
      FALSE,
      id_bit_table_3dmd,
      {  "5 a 6 b 6 c@@j k l f e d@@5 i 6 h 6 g",
         "6  j@7i  6  a@76  k@@6  l@7h  6  b@76  f@@6  e@7g  6  c@76  d"}},
   /* s4dmd */
      {15,
      &thing4dmd,
      &thing4dmd,
      0, 0, 0, 0,
      {b_4dmd,      b_p4dmd},
      { 0, 0},
      FALSE,
      id_bit_table_4dmd,
      {  "5 a 6 b 6 c 6 d@@m n o p h g f e@@5 l 6 k 6 j 6 i",
         "6  m@7l  6  a@76  n@@6  o@7k  6  b@76  p@@6  h@7j  6  c@76  g@@6  f@7i  6  d@76  e"}},
   /* s3ptpd */
      { 11,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_3ptpd,      b_p3ptpd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6a696b696c@7j6k9l6f9e6d@76i696h696g",
         "5j@@ia@@5k@@5l@@hb@@5f@@5e@@gc@@5d"}},
   /* s4ptpd */
      { 15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_4ptpd,      b_p4ptpd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6a696b696c696d@7m6n9o6p9h6g9f6e@76l696k696j696i",
         "5m@@la@@5n@@5o@@kb@@5p@@5h@@jc@@5g@@5f@@id@@5e"}},
   /* s_wingedstar */
      { 7,
      (coordrec *) 0,
      (coordrec *) 0,
      0x33, 0, 0, 0,
      {b_wingedstar, b_pwingedstar},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "665   d@a b c  g f e@665   h",
         "9a@@9b@@9c@h5d@9g@@9f@@9e"}},
   /* s_wingedstar12 */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "665   d6   f@a b c  e k  i h g@665   l6   j",
         "9a@@9b@@9c@l5d@9e@9k@j5f@9i@@9h@@9g"}},
   /* s_wingedstar16 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "665   d6   h6   m@a b c  f g  o n  k j i@665   e6   p6   l",
         "9a@@9b@@9c@e5d@9f@9g@p5h@9o@9n@l5m@9k@@9j@@9i"}},
   /* s_barredstar */
      { 9,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  " a 66 c@ 65 b@i j  e d@ 65 g@ h 66 f",
         "h  i  a@@6  j@5 g  b@6  e@@f  d  c"}},
   /* s_barredstar12 */
      {13,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  " a6  6 6  6d@6 5 b 6  c@l m  n g  f e@6 5 j 6  i@ k6  6 6  6h",
         "k  l  a@6  m@5 j  b@6  n@@6  g@5 i  c@6  f@@h  e  d"}},
   /* s_barredstar16 */
      {17,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  " a6  6 6  6 6  6e@6 5 b 6  c 6  d@o p  q r  i h  g f@6 5 m 6  l 6  k@ n6  6 6  6 6  6j",
         "n  o  a@6  p@5 m  b@6  q@6  r@5 l  c@6  i@@6  h@5 k  d@6  g@@j  f  e"}},
   /* s_galaxy */
      { 7,
      &thinggal,
      &thinggal,
      0x55, 0, 0, 0,
      {b_galaxy,    b_galaxy},
      { 0, 0},
      TRUE,
      id_bit_table_gal,
      {  "68c@58bd@7a688e@758hf@68g",
         (Cstring) 0}},
   /* s3x6 */
      {17,
      &thing3x6,
      &thing3x6,
      0, 0, 0, 0,
      {b_3x6,       b_6x3},
      { 6, 3},
      FALSE,
      id_bit_table_3x6,
      {  "a  b  c  d  e  f@@p  q  r  i  h  g@@o  n  m  l  k  j",
         "o  p  a@@n  q  b@@m  r  c@@l  i  d@@k  h  e@@j  g  f"}},
   /* s3x8 */
      {23,
      &thing3x8,
      &thing3x8,
      0, 0, 0, 0,
      {b_3x8,       b_8x3},
      { 8, 3},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h@@u  v  w  x  l  k  j  i@@t  s  r  q  p  o  n  m",
         "t  u  a@@s  v  b@@r  w  c@@q  x  d@@p  l  e@@o  k  f@@n  j  g@@m  i  h"}},
   /* s4x5 */
      {19,
      &thing4x5,
      &thing4x5,
      0, 0, 0, 0,
      {b_4x5,       b_5x4},
      { 5, 4},
      FALSE,
      id_bit_table_4x5,
      {  "a  b  c  d  e@@j  i  h  g  f@@p  q  r  s  t@@o  n  m  l  k",
         "o  p  j  a@@n  q  i  b@@m  r  h  c@@l  s  g  d@@k  t  f  e"}},
   /* s4x6 */
      {23,
      &thing4x6,
      &thing4x6,
      0, 0, 0, 0,
      {b_4x6,       b_6x4},
      { 6, 4},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f@@l  k  j  i  h  g@@s  t  u  v  w  x@@r  q  p  o  n  m",
         "r  s  l  a@@q  t  k  b@@p  u  j  c@@o  v  i  d@@n  w  h  e@@m  x  g  f"}},
   /* s2x10 */
      {19,
      &thing2x10,
      &thing2x10,
      0, 0, 0, 0,
      {b_2x10,      b_10x2},
      { 10, 2},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h  i  j@@t  s  r  q  p  o  n  m  l  k",
         "t  a@@s  b@@r  c@@q  d@@p  e@@o  f@@n  g@@m  h@@l  i@@k  j"}},
   /* s2x12 */
      {23,
/*
      &thing2x12,
      &thing2x12,
*/
      (coordrec *) 0,
      (coordrec *) 0,


      0, 0, 0, 0,
      {b_2x12,      b_12x2},
      { 12, 2},
      FALSE,
      (id_bit_table *) 0,
      {  "a  b  c  d  e  f  g  h  i  j  k  l@@x  w  v  u  t  s  r  q  p  o  n  m",
         "x  a@@w  b@@v  c@@u  d@@t  e@@s  f@@r  g@@q  h@@p  i@@o  j@@n  k@@m  l"}},
   /* sdeepqtg */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_deepqtg,      b_pdeepqtg},
      { 4, 4},
      FALSE,
      (id_bit_table *) 0,
      {  "5 a  6  b@f  e  d  c@@i  j  k  l@5 h  6  g",
         "6i  f@7h6  6a@76j  e@@6k  d@7g6  6b@76l  c"}},
   /* sdeepbigqtg */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_deepbigqtg,      b_pdeepbigqtg},
      { 4, 6},
      FALSE,
      (id_bit_table *) 0,
      {  "5 a  6  b@5 c  6  d@h  g  f  e@@m  n  o  p@5 l  6  k@5 j  6  i",
         "6 6m  h@7j l6  6c a@76 6n  g@@6 6o  f@7i k6  6d b@6 6p  e"}},
   /* sdeepxwv */
      {11,
      &thingdeepxwv,
      &thingdeepxwv,
      0, 0, 0, 0,
      {b_deepxwv,      b_pdeepxwv},
      { 6, 4},
      FALSE,
      id_bit_table_deepxwv,
      {  "66c l@@66d k@7ab6 6hg@766e j@@66f i",
         "65a@65b@fedc@ijkl@65h@65g@"}},
   /* s3oqtg */
      {19,
      &thing3oqtg,
      &thing3oqtg,
      0, 0, 0, 0,
      {b_3oqtg,     b_p3oqtg},
      { 7, 4},
      FALSE,
      (id_bit_table *) 0,
      {  "6  a  6  b  6  c@@r  s  t  g  f  e  d@@n  o  p  q  j  i  h@@6  m  6  l  6  k",
         "6  n  r@@m  o  s  a@@6  p  t@@l  q  b  g@@6  j  f@@k  i  e  c@@6  h  d"}},
   /* s_thar */
      { 7,
      &thingthar,
      &thingthar,
      0x55, 0, 0, 0,
      {b_thar,      b_thar},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "66c@66d@ab6fe@66h@66g",
         (Cstring) 0}},
   /* s_alamo */
      { 7,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_alamo,     b_alamo},
      { 0, 0},
      TRUE,
      (id_bit_table *) 0,
      {  "6ab@h66c@g66d@6fe",
         (Cstring) 0}},
   /* sx4dmd */
      {31,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s8x8 */
      {63,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* sx1x16 */
      {31,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* sfat2x8 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 8, 4},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* swide4x4 */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 8, 4},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_323 */
      { 7,
      &thing_323,
      &thing_323,
      0, 0, 0x11, 0,
      {b_323,      b_p323},
      { 0, 0},
      FALSE,
      id_bit_table_323,
      {  "   a  b  c@@   5 h  d@@   g  f  e",
         "g6a@76h@7f6b@76d@7e6c"}},
   /* s_343 */
      { 9,
      &thing_343,
      &thing_343,
      0, 0, 0, 0,
      {b_343,      b_p343},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "   5 a  b  c@@   i  j  e  d@@   5 h  g  f",
         "6i@7h6a@76j@7g6b@76e@7f6c@76d"}},
   /* s_525 */
      { 11,
      &thing_525,
      &thing_525,
      0, 0, 0, 0,
      {b_525,      b_p525},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "   a  b  c  d  e@@   6  5 l  f@@   k  j  i  h  g",
         "k6a@@j6b@76l@7i6c@76f@7h6d@@g6e"}},
   /* s_545 */
      { 13,
      &thing_545,
      &thing_545,
      0, 0, 0, 0,
      {b_545,      b_p545},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "   a  b  c  d  e@@   5 m  n  g  f@@   l  k  j  i  h",
         "l6a@76m@7k6b@76n@7j6c@76g@7i6d@76f@7h6e"}},
   /* sh545 */
      { 13,
      &thing_h545,
      &thing_h545,
      0, 0, 0, 0,
      {bh545,      bhp545},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "   a  b  c  d  e@@   m  5 n  g 5  f@@   l  k  j  i  h",
         "lma@@k6b@76n@7j6c@76g@7i6d@@hfe"}},
   /* s_3mdmd */
      { 11,
      &thing3mdmd,
      &thing3mdmd,
      0, 0, 0, 0,
      {b_3mdmd,      b_p3mdmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "5 a6666     c@7666   b@7j k l6  f e d@7666   h@75 i6666     g",
         "6  j@7i6    a@76  k@@6  l@@5  hb@@6  f@@6  e@7g6    c@76  d"}},
   /* s_3mptpd */
      { 11,
      &thing3mptpd,
      &thing3mptpd,
      0, 0, 0, 0,
      {b_3mptpd,      b_p3mptpd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6665    b@76 a6666     c@7j6  k l f e6  d@76 i6666     g@76665    h",
         "6  j@@5  ia@@6  k@@6  l@7h6    b@76  f@@6  e@@5  gc@@6  d"}},
   /* s_4mdmd */
      { 15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_4mdmd,      b_p4mdmd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "5 a6666666       d@7666   b66   c@7m n o 6 p h 6 g f e@7666   k66   j@75 l6666666       i",
         "6  m@7l  6  a@76  n@@6  o@@5  kb@@6  p@@6  h@@5  jc@@6  g@@6  f@7i  6  d@76  e"}},
   /* s_4mptpd */
      { 15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_4mptpd,      b_p4mptpd},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6 6 6 5 b 6 c@76 a 6 6 6 6 6 6 d@7m 6 n o p h g f 6 e@76 l 6 6 6 6 6 6 i@76 6 6 5 k 6 j",
         "6  m@@5  la@@6  n@@6  o@7k  6  b@76  p@@6  h@7j  6  c@76  g@@6  f@@5  id@@6  e"}},
   /* sbigh */
      {11,
      &thingbigh,
      &thingbigh,
      0, 0, 0x041, 0,
      {b_bigh,      b_pbigh},
      { 0, 0},
      FALSE,
      id_bit_table_bigh,
      {  "a6666   j@b6666   i@76e f l k@7c6666   h@d6666   g",
         "dcba@65e@65f@65l@65k@ghij"}},
   /* sbigx */
      {11,
      &thingbigx,
      &thingbigx,
      0, 0, 0x041, 0,
      {b_bigx,      b_pbigx},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6666e@6666f@7abcd6jihg@76666l@6666k",
         "65a@65b@65c@65d@klfe@65j@65i@65h@65g"}},
   /* sbigbigh */
      {15,
      &thingbigbigh,
      &thingbigbigh,
      0, 0, 0, 0,
      {b_bigbigh,      b_pbigbigh},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "a66666666l@b66666666k@76efghponm@7c66666666j@d66666666i",
         "dcba@65e@65f@65g@65h@65p@65o@65n@65m@ijkl"}},
   /* sbigbigx */
      {15,
      &thingbigbigx,
      &thingbigbigx,
      0, 0, 0, 0,
      {b_bigbigx,      b_pbigbigx},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6666e p@6666f o@abcd6 6lkji@76666g n@6666h m",
         "65a@65b@65c@65d@hgfe@mnop@65l@65k@65j@65i"}},
   /* sbigrig */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_bigrig,    b_pbigrig},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6666e f@7abcd6 6jihg@76666l k",
         "5a@5b@5c@5d@le@kf@5j@5i@5h@5g"}},
   /* sbighrgl */
      {11,
      (coordrec *) 0,   /* FIX */
      (coordrec *) 0,   /* FIX */
      0, 0, 0x104, 0,
      {b_bighrgl,   b_pbighrgl},
      { 0, 0},
      FALSE,
      id_bit_table_bighrgl,
      {  "6 6 c@7a b 6 e f@6 5 jd@l k 6 h g@76 6 i",
         "9l5a@@9k5b@56j@7i66c@756d@9h5e@@9g5f"}},
   /* sbigdhrgl */
      {11,
      (coordrec *) 0,   /* FIX */
      (coordrec *) 0,   /* FIX */
      0, 0, 0x104, 0,
      {b_bigdhrgl,   b_pbigdhrgl},
      { 0, 0},
      FALSE,
      id_bit_table_bigdhrgl,
      {  "a  b 6 c 6 e  f@6  6 j 6 d@l  k 6 i 6 h  g",
         "l  a@@k  b@@5 j@@i  c@@5 d@@h  e@@g  f"}},
   /* sbigbone */
      {11,
      &thingbigbone,
      &thingbigbone,
      0, 0, 0x104, 0,
      {b_bigbone,   b_pbigbone},
      { 0, 0},
      FALSE,
      id_bit_table_bigbone,
      {  "a  b6666   e  f@766  c d j i@7l  k6666   h  g",
         "la@kb@5c@5d@5j@5i@he@gf"}},
   /* sbigdmd */
      {11,
      &thingbigdmd,
      &thingbigdmd,
      0, 0, 0x104, 0,
      {b_bigdmd, b_pbigdmd},
      {0, 0},
      FALSE,
      id_bit_table_bigdmd,
      {  "6 6  c@7a b  6  e f@76 6  d@@6 6  j@7l k  6  h g@76 6  i",
         "5 l6  a@5 k6  b@i j d c@5 h6  e@5 g6  f"}},
   /* sbigptpd */
      {11,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0x104, 0,
      {b_bigptpd, b_pbigptpd},
      {0, 0},
      FALSE,
      id_bit_table_bigptpd,
      {  "85f 8858 g@85e 8858 h@7c88d j88i@785b 8858 k@85 a 8858 l",
         "65c@@abef@@65d@@65j@@lkhg@@65i"}},
   /* sdblxwave */
      {15,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_dblxwave, b_pdblxwave},
      {0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  "6 6c6 6 6 6o@6 6d6 6 6 6p@7a b6f e m n6j i@76 6h6 6 6 6l@6 6g6 6 6 6k",
         "65a@65b@ghdc@65f@65e@65m@65n@klpo@65j@65i"}},
   /* s_dead_concentric */
      {-1,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}},
   /* s_normal_concentric */
      {-1,
      (coordrec *) 0,
      (coordrec *) 0,
      0, 0, 0, 0,
      {b_nothing,   b_nothing},
      { 0, 0},
      FALSE,
      (id_bit_table *) 0,
      {  (Cstring) 0,
         (Cstring) 0}}};


/* BEWARE!!  This list is keyed to the definition of "begin_kind" in database.h . */
/*   It must also match the similar table in the mkcalls.c . */
int begin_sizes[] = {
   0,          /* b_nothing */
   1,          /* b_1x1 */
   2,          /* b_1x2 */
   2,          /* b_2x1 */
   3,          /* b_1x3 */
   3,          /* b_3x1 */
   4,          /* b_2x2 */
   4,          /* b_dmd */
   4,          /* b_pmd */
   4,          /* b_star */
   6,          /* b_trngl */
   6,          /* b_ptrngl */
   8,          /* b_trngl4 */
   8,          /* b_ptrngl4 */
   6,          /* b_bone6 */
   6,          /* b_pbone6 */
   6,          /* b_short6 */
   6,          /* b_pshort6 */
   6,          /* b_1x2dmd */
   6,          /* b_p1x2dmd */
   6,          /* b_2x1dmd */
   6,          /* b_p2x1dmd */
   8,          /* b_qtag */
   8,          /* b_pqtag */
   8,          /* b_bone */
   8,          /* b_pbone */
   8,          /* b_rigger */
   8,          /* b_prigger */
   8,          /* b_2stars */
   8,          /* b_p2stars */
   8,          /* b_spindle */
   8,          /* b_pspindle */
   8,          /* b_hrglass */
   8,          /* b_phrglass */
   8,          /* b_dhrglass */
   8,          /* b_pdhrglass */
   8,          /* b_crosswave */
   8,          /* b_pcrosswave */
   4,          /* b_1x4 */
   4,          /* b_4x1 */
   8,          /* b_1x8 */
   8,          /* b_8x1 */
   8,          /* b_2x4 */
   8,          /* b_4x2 */
   6,          /* b_2x3 */
   6,          /* b_3x2 */
  10,          /* b_2x5 */
  10,          /* b_5x2 */
  10,          /* b_d2x5 */
  10,          /* b_d5x2 */
  10,          /* b_wqtag */
  10,          /* b_pwqtag */
  10,          /* b_deep2x1dmd */
  10,          /* b_pdeep2x1dmd */
  10,          /* b_whrglass */
  10,          /* b_pwhrglass */
   6,          /* b_1x6 */
   6,          /* b_6x1 */
   12,         /* b_3x4 */
   12,         /* b_4x3 */
   12,         /* b_2x6 */
   12,         /* b_6x2 */
   14,         /* b_2x7 */
   14,         /* b_7x2 */
   12,         /* b_d3x4 */
   12,         /* b_d4x3 */
   16,         /* b_2x8 */
   16,         /* b_8x2 */
   16,         /* b_4x4 */
   10,         /* b_1x10 */
   10,         /* b_10x1 */
   12,         /* b_1x12 */
   12,         /* b_12x1 */
   14,         /* b_1x14 */
   14,         /* b_14x1 */
   16,         /* b_1x16 */
   16,         /* b_16x1 */
   16,         /* b_c1phan */
   8,          /* b_galaxy */
   18,         /* b_3x6 */
   18,         /* b_6x3 */
   24,         /* b_3x8 */
   24,         /* b_8x3 */
   20,         /* b_4x5 */
   20,         /* b_5x4 */
   24,         /* b_4x6 */
   24,         /* b_6x4 */
   20,         /* b_2x10 */
   20,         /* b_10x2 */
   24,         /* b_2x12 */
   24,         /* b_12x2 */
   12,         /* b_deepqtg */
   12,         /* b_pdeepqtg */
   16,         /* b_deepbigqtg */
   16,         /* b_pdeepbigqtg */
   12,         /* b_deepxwv */
   12,         /* b_pdeepxwv */
   20,         /* b_3oqtg */
   20,         /* b_p3oqtg */
   8,          /* b_thar */
   8,          /* b_alamo */
   8,          /* b_ptpd */
   8,          /* b_pptpd */
   8,          /* b_1x3dmd */
   8,          /* b_p1x3dmd */
   8,          /* b_3x1dmd */
   8,          /* b_p3x1dmd */
   12,         /* b_3dmd */
   12,         /* b_p3dmd */
   16,         /* b_4dmd */
   16,         /* b_p4dmd */
   12,         /* b_3ptpd */
   12,         /* b_p3ptpd */
   16,         /* b_4ptpd */
   16,         /* b_p4ptpd */

   8,          /* b_wingedstar */
   8,          /* b_pwingedstar */

   8,          /* b_323 */
   8,          /* b_p323 */
   10,         /* b_343 */
   10,         /* b_p343 */
   12,         /* b_525 */
   12,         /* b_p525 */
   14,         /* b_545 */
   14,         /* b_p545 */
   14,         /* bh545 */
   14,         /* bhp545 */
   12,         /* b_3mdmd */
   12,         /* b_p3mdmd */
   12,         /* b_3mptpd */
   12,         /* b_p3mptpd */
   16,         /* b_4mdmd */
   16,         /* b_p4mdmd */
   16,         /* b_4mptpd */
   16,         /* b_p4mptpd */
   12,         /* b_bigh */
   12,         /* b_pbigh */
   12,         /* b_bigx */
   12,         /* b_pbigx */
   16,         /* b_bigbigh */
   16,         /* b_pbigbigh */
   16,         /* b_bigbigx */
   16,         /* b_pbigbigx */
   12,         /* b_bigrig */
   12,         /* b_pbigrig */
   12,         /* b_bighrgl */
   12,         /* b_pbighrgl */
   12,         /* b_bigdhrgl */
   12,         /* b_pbigdhrgl */
   12,         /* b_bigbone */
   12,         /* b_pbigbone */
   12,         /* b_bigdmd */
   12,         /* b_pbigdmd */
   12,         /* b_bigptpd */
   12,         /* b_pbigptpd */
   16,         /* b_dblxwave */
   16};        /* b_pdblxwave */


/* The following 8 definitions are taken verbatim from sdinit.c . */
#define B1A (0000 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_HCOR|ID1_PERM_HEAD|ID1_PERM_BOY)
#define G1A (0100 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_HEAD|ID1_PERM_GIRL)
#define B2A (0200 | ID1_PERM_NSG|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_SIDE|ID1_PERM_BOY)
#define G2A (0300 | ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_HCOR|ID1_PERM_SIDE|ID1_PERM_GIRL)
#define B3A (0400 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_HCOR|ID1_PERM_HEAD|ID1_PERM_BOY)
#define G3A (0500 | ID1_PERM_NSG|ID1_PERM_NSB|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_HEAD|ID1_PERM_GIRL)
#define B4A (0600 | ID1_PERM_NSG|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_SCOR|ID1_PERM_SIDE|ID1_PERM_BOY)
#define G4A (0700 | ID1_PERM_NSB|ID1_PERM_NHG|ID1_PERM_NHB|ID1_PERM_HCOR|ID1_PERM_SIDE|ID1_PERM_GIRL)

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h. */
startinfo startinfolist[] = {
   {
      "???",      /* A non-existent setup. */
      FALSE,                         /* into_the_middle */
      {
         nothing,                    /* kind */
         1,                          /* rotation */
         {0},                        /* cmd */
         {{0,0}},                    /* people */
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "Heads 1P2P",
      FALSE,                         /* into_the_middle */
      {
         s2x4,                       /* kind */
         1,                          /* rotation */
         {0},                        /* cmd */
         {{G2A|d_south,0}, {B2A|d_south,0}, {G1A|d_south,0}, {B1A|d_south,0},
            {G4A|d_north,0}, {B4A|d_north,0}, {G3A|d_north,0}, {B3A|d_north,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "Sides 1P2P",
      FALSE,                         /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{G3A|d_south,0}, {B3A|d_south,0}, {G2A|d_south,0}, {B2A|d_south,0},
            {G1A|d_north,0}, {B1A|d_north,0}, {G4A|d_north,0}, {B4A|d_north,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "HEADS",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{B4A|d_east,0}, {G3A|d_south,0}, {B3A|d_south,0}, {G2A|d_west,0},
            {B2A|d_west,0}, {G1A|d_north,0}, {B1A|d_north,0}, {G4A|d_east,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "SIDES",
      TRUE,                          /* into_the_middle */
      {
         s2x4,                       /* kind */
         1,                          /* rotation */
         {0},                        /* cmd */
         {{B3A|d_east,0}, {G2A|d_south,0}, {B2A|d_south,0}, {G1A|d_west,0},
            {B1A|d_west,0}, {G4A|d_north,0}, {B4A|d_north,0}, {G3A|d_east,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   },
   {
      "From squared set",
      FALSE,                         /* into_the_middle */
      {
         s4x4,                       /* kind */
         0,                          /* rotation */
         {0},                        /* cmd */
         {{0,0}, {G2A|d_west,0}, {B2A|d_west,0}, {0,0}, {0,0}, {G1A|d_north,0},
            {B1A|d_north,0}, {0,0}, {0,0}, {G4A|d_east,0}, {B4A|d_east,0}, {0,0},
            {0,0}, {G3A|d_south,0}, {B3A|d_south,0}, {0,0}},
         0                           /* result_flags (imprecise_rotation is off) */
      }
   }
};



/* Nonzero item in map3[1] for arity 1 map means do not reassemble. */
/* For maps with arity 1, a nonzero item in map3[0] means that the "offset goes away" warning should be given in certain cases. */
/* For maps with arity 3, a value of 3 after the last map means that the "overlap goes away" warning should be given in certain cases. */

/*                                                                                                          setups are stacked on top of each other ----------------------|
                                                                                        each setup is rotated clockwise before being concatenated -------------------|    |
                                                                                                                                                                     V    V
                                             map1                             map2                        map3 map4   map_kind     warncode  arity outer   inner    rot  vert */

Private map_thing map_rig_trngl4        = {{6, 7, 0, 5,                       2, 3, 4, 1},                           MPKIND__SPLIT,       0, 2,  s_rigger,s_trngl4, 0x10D, 0};

Private map_thing map_s6_trngl          = {{4, 5, 3,                          1, 2, 0},                              MPKIND__SPLIT,       0, 2,  s_short6, s_trngl, 0x108, 1, MAPCODE(s_trngl,2,MPKIND__SPLIT,1)};


Private map_thing map_phan_trngl4b      = {{12, 14, 0, 2,                     4, 6, 8, 10},                          MPKIND__SPLIT,       0, 2,  s_c1phan,s_trngl4, 0x108, 0, MAPCODE(s_trngl4,2,MPKIND__SPLIT,0)};




        map_thing map_p8_tgl4           = {{0, 2, 7, 5,                       4, 6, 3, 1},                           MPKIND__REMOVED,     0, 2,  s_ptpd, s_trngl4,  0x10D, 0};
        map_thing map_spndle_once_rem   = {{7, 1, 3, 5,                       0, 2, 4, 6, (int) s2x2},               MPKIND__SPEC_ONCEREM,0, 2,  s_spindle, sdmd,   0x000, 0};
        map_thing map_1x3dmd_once_rem   = {{0, 2, 4, 6,                       1, 3, 5, 7, (int) sdmd},               MPKIND__SPEC_ONCEREM,0, 2,  s1x3dmd, s1x4,     0x000, 0};
        map_thing map_lh_zzztgl         = {{11, 7, 2, 4,                      3, 15, 10, 12},                        MPKIND__OFFS_L_HALF, 4, 2,  s4x4,   s_trngl4,  0x00D, 1};
        map_thing map_rh_zzztgl         = {{7, 11, 8, 9,                      15, 3, 0, 1},                          MPKIND__OFFS_R_HALF, 4, 2,  s4x4,   s_trngl4,  0x007, 1};
        map_thing map_2x2v              = {{0, 3,                             1, 2},                                 MPKIND__SPLIT,       0, 2,  s2x2,   s1x2,      0x005, 0};
        map_thing map_2x4_magic         = {{0, 6, 3, 5,                       7, 1, 4, 2},                           MPKIND__NONE,        0, 2,  s2x4,   s1x4,      0x000, 0};
        map_thing map_qtg_magic         = {{0, 2, 5, 3,                       1, 7, 4, 6},                           MPKIND__NONE,        0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_qtg_intlk         = {{0, 3, 5, 6,                       1, 2, 4, 7},                           MPKIND__NONE,        0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_qtg_magic_intlk   = {{0, 2, 5, 7,                       1, 3, 4, 6},                           MPKIND__NONE,        0, 2,  s_qtag, sdmd,      0x005, 0};
        map_thing map_ptp_magic         = {{6, 1, 4, 3,                       0, 7, 2, 5},                           MPKIND__NONE,        0, 2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_ptp_intlk         = {{0, 1, 6, 3,                       2, 7, 4, 5},                           MPKIND__NONE,        0, 2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_ptp_magic_intlk   = {{2, 1, 4, 3,                       0, 7, 6, 5},                           MPKIND__NONE,        0, 2,  s_ptpd, sdmd,      0x000, 0};
        map_thing map_2x4_diagonal      = {{2, 3, 6, 7,                       0, 1, 4, 5},                           MPKIND__NONE,        0, 2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_2x4_int_pgram     = {{1, 3, 5, 7,                       0, 2, 4, 6},                           MPKIND__NONE,        0, 2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_2x4_trapezoid     = {{1, 2, 4, 7,                       0, 3, 5, 6},                           MPKIND__NONE,        0, 2,  s2x4,   s2x2,      0x000, 0};
        map_thing map_4x4_ns            = {{12, 13, 0, 14,                    8, 6, 4, 5},                           MPKIND__NONE,        0, 2,  s4x4,   s1x4,      0x000, 1};
        map_thing map_4x4_ew            = {{0, 1, 4, 2,                       12, 10, 8, 9},                         MPKIND__NONE,        0, 2,  s4x4,   s1x4,      0x005, 1};
        map_thing map_stairst           = {{9, 13, 7, 0, 1, 5, 15, 8,         12, 11, 14, 2, 4, 3, 6, 10},           MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_ladder            = {{10, 15, 14, 0, 2, 7, 6, 8,        12, 13, 3, 1, 4, 5, 11, 9},            MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_crazy_offset1     = {{10, 15, 0, 14,    8, 6, 2, 7,    12, 13, 1, 3,    9, 11, 4, 5},          MPKIND__NONE,        0, 4,  s4x4,   s1x4,      0x000, 1};
        map_thing map_crazy_offset2     = {{10, 15, 2, 7,        9, 11, 1, 3},                                       MPKIND__NONE,        0, 2,  s4x4,   s1x4,      0x000, 1};
        map_thing map_but_o             = {{10, 13, 14, 1, 2, 5, 6, 9,        12, 15, 3, 0, 4, 7, 11, 8},            MPKIND__NONE,        0, 2,  s4x4,   s2x4,      0x000, 1};
        map_thing map_4x4v     = {{12, 10, 8, 9,         13, 15, 6, 11,       14, 3, 5, 7,      0, 1, 4, 2},         MPKIND__SPLIT,       0, 4,  s4x4,   s1x4,      0x055, 0};
        map_thing map_blocks   = {{12, 14, 7, 9,         13, 0, 2, 11,        15, 1, 4, 6,      10, 3, 5, 8},        MPKIND__NONE,        0, 4,  s4x4,   s2x2,      0x000, 0};

        map_thing map_trglbox  = {{12, 14, 15, 9,        13, 0, 2, 3,         7, 1, 4, 6,       10, 11, 5, 8},       MPKIND__NONE,        0, 4,  s4x4,   s2x2,      0x000, 0};
        map_thing map_trglbox12a = {{0, 2, 7, 11,        1, 5, 6, 8},                                                MPKIND__NONE,        0, 2,  s3x4,   s2x2,      0x000, 0};
        map_thing map_trglbox12b = {{11, 2, 7, 9,        1, 3, 5, 8},                                                MPKIND__NONE,        0, 2,  s3x4,   s2x2,      0x000, 0};

        map_thing map_2x3_0134  = {{4, 3,                0, 1},                                                      MPKIND__NONE,        0, 2,  s2x3,   s1x2,      0x000, 1};
        map_thing map_2x3_1245  = {{5, 4,                1, 2},                                                      MPKIND__NONE,        0, 2,  s2x3,   s1x2,      0x000, 1};

Private map_thing map_4x4_1x4  = {{8, 6, 4, 5,           9, 11, 2, 7,         10, 15, 1, 3,     12, 13, 0, 14},      MPKIND__SPLIT,       0, 4,  s4x4,   s1x4,      0x000, 1};
Private map_thing map_1x16_1x4 = {{0, 1, 3, 2,           4, 5, 7, 6,          15, 14, 12, 13,   11, 10, 8, 9},       MPKIND__SPLIT,       0, 4,  s1x16,  s1x4,      0x000, 0};
Private map_thing map_1x12_1x3 = {{0, 1, 2,              3, 4, 5,             11, 10, 9,        8, 7, 6},            MPKIND__SPLIT,       0, 4,  s1x12,  s1x3,      0x000, 0};
Private map_thing map_3x4_1x3  = {{0, 10, 9,             1, 11, 8,            2, 5, 7,          3, 4, 6},            MPKIND__SPLIT,       0, 4,  s3x4,   s1x3,      0x055, 0};
Private map_thing map_2x8_2x2  = {{0, 1, 14, 15,         2, 3, 12, 13,        4, 5, 10, 11,     6, 7, 8, 9},         MPKIND__SPLIT,       0, 4,  s2x8,   s2x2,      0x000, 0};
Private map_thing map_4dmd_dmd          = {{0, 13, 11, 12,     1, 15, 10, 14,     2, 6, 9, 7,     3, 4, 8, 5},       MPKIND__SPLIT,       0, 4,  s4dmd,  sdmd,      0x055, 0};
Private map_thing map_4ptp_dmd          = {{12, 0, 13, 11,     14, 1, 15, 10,     7, 2, 6, 9,     5, 3, 4, 8},       MPKIND__SPLIT,       0, 4,  s4ptpd, sdmd,      0x000, 0};
Private map_thing map_2x6_1x6           = {{11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 2,  s2x6,   s1x6,      0x000, 1};
Private map_thing map_3x6_1x6           = {{14, 13, 12, 9, 10, 11,            15, 16, 17, 6, 7, 8,
                                                                              0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 3,  s3x6,   s1x6,      0x000, 1};
Private map_thing map_4x6_1x6           = {{17, 16, 15, 12, 13, 14,           18, 19, 20, 23, 22, 21,
                                            11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 4,  s4x6,   s1x6,      0x000, 1};
Private map_thing map_2x8_1x8           = {{15, 14, 12, 13, 8, 9, 11, 10,     0, 1, 3, 2, 7, 6, 4, 5},               MPKIND__SPLIT,       0, 2,  s2x8,   s1x8,      0x000, 1};
Private map_thing map_3x8_1x8           = {{19, 18, 16, 17, 12, 13, 15, 14,   20, 21, 23, 22, 8, 9, 11, 10,
        0, 1, 3, 2, 7, 6, 4, 5},               MPKIND__SPLIT,       0, 3,  s3x8,   s1x8,      0x000, 1};
Private map_thing map_1x12_1x6          = {{0, 1, 2, 5, 4, 3,                 11, 10, 9, 6, 7, 8},                   MPKIND__SPLIT,       0, 2,  s1x12,  s1x6,      0x000, 0};
/* Special maps for putting back end-to-end 1x6's, or 1x2 diamonds, resulting from collisions at one end. */
        map_thing map_1x8_1x6           = {{0, 1, 3, -1, -1, 2,               -1, -1, 6, 4, 5, 7},                   MPKIND__NONE,        0, 2,  s1x8,   s1x6,      0x000, 0};
        map_thing map_rig_1x6           = {{6, 7, 0, -1, -1, 5,               -1, -1, 1, 2, 3, 4},                   MPKIND__NONE,        0, 2,  s_rigger,s_1x2dmd, 0x000, 0};
Private map_thing map_1x16_1x8          = {{0, 1, 3, 2, 7, 6, 4, 5,           15, 14, 12, 13, 8, 9, 11, 10},         MPKIND__SPLIT,       0, 2,  s1x16,  s1x8,      0x000, 0};
Private map_thing map_hv_2x4_2          = {{0, 1, 2, 3, 12, 13, 14, 15,       4, 5, 6, 7, 8, 9, 10, 11},             MPKIND__SPLIT,       0, 2,  s2x8,   s2x4,      0x000, 0};
Private map_thing map_3x4_2x3           = {{1, 11, 8, 9, 10, 0,               3, 4, 6, 7, 5, 2},                     MPKIND__SPLIT,       0, 2,  s3x4,   s2x3,      0x005, 0};
        map_thing map_vsplit_f          = {{13, 15, 11, 6, 8, 9, 10, 12,      0, 1, 2, 4, 5, 7, 3, 14},              MPKIND__SPLIT,       0, 2,  s4x4,   s2x4,      0x005, 0};
Private map_thing map_split_f           = {{9, 11, 7, 2, 4, 5, 6, 8,          12, 13, 14, 0, 1, 3, 15, 10},          MPKIND__SPLIT,       0, 2,  s4x4,   s2x4,      0x000, 1};
Private map_thing map_2x4_2x2           = {{0, 1, 6, 7,                       2, 3, 4, 5},                           MPKIND__SPLIT,       0, 2,  s2x4,   s2x2,      0x000, 0};
Private map_thing map_2x3_1x3           = {{5, 4, 3,                          0, 1, 2},                              MPKIND__SPLIT,       0, 2,  s2x3,   s1x3,      0x000, 1};
Private map_thing map_bigd_12d          = {{11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__SPLIT,       0, 2,  sbigdmd,s_1x2dmd,  0x000, 1};
Private map_thing map_bone_12d          = {{-1, -1, 0, 7, 6, 5,               3, 2, 1, -1, -1, 4},                   MPKIND__SPLIT,       0, 2,  s_bone, s_1x2dmd,  0x000, 0};
Private map_thing map_bone_rig          = {{-1, 0, 7, 5, -1, 6,               1, -1, 2, -1, 4, 3},                   MPKIND__SPLIT,       0, 2,  s_rigger,s_bone6,  0x000, 0};
Private map_thing map_2x4_1x4           = {{7, 6, 4, 5,                       0, 1, 3, 2},                           MPKIND__SPLIT,       0, 2,  s2x4,   s1x4,      0x000, 1};
Private map_thing map_qtg_dmd           = {{0, 7, 5, 6,                       1, 2, 4, 3},                           MPKIND__SPLIT,       0, 2,  s_qtag, sdmd,      0x005, 0};
Private map_thing map_ptp_dmd           = {{0, 1, 2, 3,                       6, 7, 4, 5},                           MPKIND__SPLIT,       0, 2,  s_ptpd, sdmd,      0x000, 0};
Private map_thing map_2stars            = {{6, 0, 7, 5,                       3, 1, 2, 4},                           MPKIND__SPLIT,       0, 2,  s_2stars, s_star,  0x000, 0};

Private map_thing map_3x8_3x4           = {{0, 1, 2, 3, 23, 22, 16, 17, 18, 19, 20, 21,
                                                                   4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 11, 10},        MPKIND__SPLIT,       0, 2,  s3x8,   s3x4,      0x000, 0};
Private map_thing map_4x6_3x4           = {{2, 9, 20, 15, 16, 19, 17, 18, 11, 0, 1, 10,
                                                                   5, 6, 23, 12, 13, 22, 14, 21, 8, 3, 4, 7},        MPKIND__SPLIT,       0, 2,  s4x6,   s3x4,      0x005, 0};
Private map_thing map_par_rig           = {{6, 13, 9, 11, 12, 7, 0, 2,
                                            4, 15, 8, 10, 14, 5, 1, 3},                                              MPKIND__SPLIT,       0, 2,  sdeepbigqtg, s_rigger,      0x005, 0};
Private map_thing map_4x6_2x6           = {{18, 19, 20, 21, 22, 23, 12, 13, 14, 15, 16, 17,
                                                                   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},            MPKIND__SPLIT,       0, 2,  s4x6,   s2x6,      0x000, 1};

Private map_thing map_2x12_2x6          = {{0, 1, 2, 3, 4, 5, 18, 19, 20, 21, 22, 23,
                                                                   6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17},      MPKIND__SPLIT,       0, 2,  s2x12,   s2x6,      0x000, 0};



Private map_thing map_3dmd_dmd          = {{0, 10, 8, 9,           1, 5, 7, 11,           2, 3, 6, 4},               MPKIND__SPLIT,       0, 3,  s3dmd,  sdmd,      0x015, 0};
Private map_thing map_3ptp_dmd          = {{9, 0, 10, 8,           11, 1, 5, 7,           4, 2, 3, 6},               MPKIND__SPLIT,       0, 3,  s3ptpd, sdmd,      0x000, 0};

        map_thing map_4x6_2x4  = {{1, 10, 19, 16, 17, 18, 11, 0,   3, 8, 21, 14, 15, 20, 9, 2,
                                                                   5, 6, 23, 12, 13, 22, 7, 4},                      MPKIND__SPLIT,       0, 3,  s4x6,   s2x4,      0x015, 0};

Private map_thing map_2x12_2x4  = {{0, 1, 2, 3, 20, 21, 22, 23,    4, 5, 6, 7, 16, 17, 18, 19,
                                                                   8, 9, 10, 11, 12, 13, 14, 15},                    MPKIND__SPLIT,       0, 3,  s2x12,  s2x4,      0x000, 0};

Private map_thing map_hv_qtg_2          = {{0, 1, 15, 14, 10, 11, 12, 13,     2, 3, 4, 5, 8, 9, 7, 6},               MPKIND__SPLIT,       0, 2,  s4dmd,  s_qtag,    0x000, 0};
Private map_thing map_vv_qtg_2          = {{9, 20, 16, 19, 18, 11, 1, 10,     6, 23, 13, 22, 21, 8, 4, 7},           MPKIND__SPLIT,       0, 2,  s4x6,   s_qtag,    0x005, 0};
Private map_thing map_spin_3x4          = {{1, 11, 8, -1, 9, 10, 0, -1,       3, 4, 6, -1, 7, 5, 2, -1},             MPKIND__SPLIT,       0, 2,  s3x4,   s_spindle, 0x005, 0};
static map_thing map_hrgl_ptp           = {{-1, -1, 2, 1, -1, -1, 0, 3,       -1, -1, 4, 7, -1, -1, 6, 5},           MPKIND__SPLIT,       0, 2,  s_ptpd, s_hrglass, 0x000, 0};
static map_thing map_xwv_bone           = {{-1, -1, -1, 0, 7, 6, -1, 5,       3, 2, -1, 1, -1, -1, -1, 4},           MPKIND__SPLIT,       0, 2,  s_bone, s_crosswave, 0x000, 0};

        map_thing map_ov_hrg_1          = {{-1, -1, 5, 7, -1, -1, 0, 6,       -1, -1, 4, 2, -1, -1, 1, 3},           MPKIND__OVERLAP,     0, 2,  s_qtag, s_hrglass, 0x005, 0};
        map_thing map_ov_gal_1          = {{-1, 0, -1, 1, -1, 6, -1, 7,       -1, 2, -1, 3, -1, 4, -1, 5},           MPKIND__OVERLAP,     0, 2,  s2x4,   s_galaxy,  0x000, 0};


        map_thing map_3o_qtag_1         = {{19, 15, 12, 14, 13, 17, 0, 18,    5, 9, 11, 16, 15, 19, 1, 6,
                                                                              3, 7, 10, 8, 9, 5, 2, 4},              MPKIND__OVERLAP,     0, 3,  s3oqtg, s_qtag,    0x015, 0};


Private map_thing map_1x6_1x3           = {{0, 1, 2,                          5, 4, 3},                              MPKIND__SPLIT,       0, 2,  s1x6,   s1x3,      0x000, 0};
Private map_thing map_1x8_1x4           = {{0, 1, 2, 3,                       6, 7, 4, 5},                           MPKIND__SPLIT,       0, 2,  s1x8,   s1x4,      0x000, 0};
Private map_thing map_1x12_1x4          = {{0, 1, 3, 2,            4, 5, 10, 11,          9, 8, 6, 7},               MPKIND__SPLIT,       0, 3,  s1x12,  s1x4,      0x000, 0};
Private map_thing map_1x2_1x1           = {{0,                                1},                                    MPKIND__SPLIT,       0, 2,  s1x2,   s1x1,      0x000, 0};
Private map_thing map_1x3_1x1           = {{0,                     1,                     2},                        MPKIND__SPLIT,       0, 3,  s1x3,   s1x1,      0x000, 0};
Private map_thing map_1x4_1x2           = {{0, 1,                             3, 2},                                 MPKIND__SPLIT,       0, 2,  s1x4,   s1x2,      0x000, 0};
        map_thing map_tgl4_1            = {{1, 0,                             2, 3},                                 MPKIND__NONE,        0, 2,  s_trngl4,s1x2,     0x001, 1};
        map_thing map_tgl4_2            = {{3, 2,                             1, 0},                                 MPKIND__NONE,        0, 2,  s_trngl4,s1x2,     0x006, 1};
Private map_thing map_3x6_2x3           = {{1, 16, 13, 14, 15, 0,  3, 8, 11, 12, 17, 2,   5, 6, 9, 10, 7, 4},        MPKIND__SPLIT,       0, 3,  s3x6,   s2x3,      0x015, 0};
Private map_thing map_2x6_2x2           = {{0, 1, 10, 11,          2, 3, 8, 9,            4, 5, 6, 7},               MPKIND__SPLIT,       0, 3,  s2x6,   s2x2,      0x000, 0};
Private map_thing map_2x6_2x2r          = {{1, 10, 11, 0,          3, 8, 9, 2,            5, 6, 7, 4},               MPKIND__SPLIT,       0, 3,  s2x6,   s2x2,      0x015, 0};
Private map_thing map_3x4_1x4           = {{9, 8, 6, 7,            10, 11, 4, 5,          0, 1, 3, 2},               MPKIND__SPLIT,       0, 3,  s3x4,   s1x4,      0x000, 1};
Private map_thing map_2x3_1x2           = {{0, 5,                  1, 4,                  2, 3},                     MPKIND__SPLIT,       0, 3,  s2x3,   s1x2,      0x015, 0};
Private map_thing map_1x6_1x2           = {{0, 1,                  2, 5,                  4, 3},                     MPKIND__SPLIT,       0, 3,  s1x6,   s1x2,      0x000, 0};
Private map_thing map_2x2h              = {{3, 2,                             0, 1},                                 MPKIND__SPLIT,       0, 2,  s2x2,   s1x2,      0x000, 1};
Private map_thing map_2x4_2x2r          = {{1, 6, 7, 0,                       3, 4, 5, 2},                           MPKIND__SPLIT,       0, 2,  s2x4,   s2x2,      0x005, 0};
Private map_thing map_2x6_2x3           = {{0, 1, 2, 9, 10, 11,               3, 4, 5, 6, 7, 8},                     MPKIND__SPLIT,       0, 2,  s2x6,   s2x3,      0x000, 0};



Private map_thing map_short6_2 =
   {{5, 0, 4, 9, 7, 8,                 3, 1, 2, 11, 6, 10},                   MPKIND__SPLIT,       0, 2, sdeepqtg,s_short6,  0x000, 0, MAPCODE(s_short6,2,MPKIND__SPLIT,   0)};


        map_thing map_qtag_2x3          = {{0, 7, 5, -1, 6, -1,               -1, 2, -1, 4, 3, 1},                   MPKIND__SPLIT,       0, 2,  s_qtag, s2x3,      0x005, 0};
        map_thing map_2x3_rmvr          = {{2, 5, 7, 9, 10, 0,                3, 4, 6, 8, 11, 1},                    MPKIND__REMOVED,     0, 2,  s3x4,   s2x3,      0x005, 0};
        map_thing map_2x3_rmvs          = {{1, 3, 4, -1, 6, -1,               -1, 2, -1, 5, 7, 0},                   MPKIND__REMOVED,     0, 2,  s_qtag, s2x3,      0x005, 0};
        map_thing map_dbloff1           = {{0, 1, 3, 2, 4, 5, 7, 6},                                                 MPKIND__NONE,        0, 1,  s2x4,   s_qtag,    0x000, 0};
        map_thing map_dbloff2           = {{2, 3, 4, 5, 6, 7, 0, 1},                                                 MPKIND__NONE,        0, 1,  s2x4,   s_qtag,    0x000, 0};
        map_thing map_dhrgl1            = {{0, 1, 9, 6, 7, 3},                                                       MPKIND__NONE,        0, 1,  sbighrgl,s1x6,     0x000, 0};
        map_thing map_dhrgl2            = {{11, 10, 9, 5, 4, 3},                                                     MPKIND__NONE,        0, 1,  sbighrgl,s1x6,     0x000, 0};


        map_thing map_dbgbn1            = {{0, 1, 3, 2, 6, 7, 9, 8},                                                 MPKIND__NONE,        0, 1,  sbigbone,s1x8,     0x000, 0};
        map_thing map_dbgbn2            = {{11, 10, 3, 2, 5, 4, 9, 8},                                               MPKIND__NONE,        0, 1,  sbigbone,s1x8,     0x000, 0};
        map_thing map_off1x81           = {{0, 1, 3, 2, 8, 9, 11, 10},                                               MPKIND__NONE,        0, 1,  s2x8,    s1x8,     0x000, 0};
        map_thing map_off1x82           = {{15, 14, 12, 13, 7, 6, 4, 5},                                             MPKIND__NONE,        0, 1,  s2x8,    s1x8,     0x000, 0};
        map_thing map_dqtag1            = {{0, 1, 4, 5, 6, 7, 10, 11},                                               MPKIND__NONE,        0, 1,  s3dmd,   s_qtag,   0x000, 0};
        map_thing map_dqtag2            = {{1, 2, 4, 5, 7, 8, 10, 11},                                               MPKIND__NONE,        0, 1,  s3dmd,   s_qtag,   0x000, 0};
        map_thing map_dqtag3            = {{13, 14, 2, 3, 5, 6, 10, 11},                                             MPKIND__NONE,        0, 1,  s4x4,   s_qtag,   0x000, 0};
        map_thing map_dqtag4            = {{13, 14, 1, 7, 5, 6, 9, 15},                                              MPKIND__NONE,        0, 1,  s4x4,   s_qtag,   0x000, 0};

// Maps for finding C/L/W's of 3.

static map_thing map3ri            = {{9, 11, 7, 15, 3, 1},                                                     MPKIND__NONE,        0, 2,  s4x4,   s1x3,   0x000, 0};
static map_thing map3li            = {{11, 7, 2, 10, 15, 3},                                                    MPKIND__NONE,        0, 2,  s4x4,   s1x3,   0x000, 0};
static map_thing map3ro            = {{6, 5, 4, 12, 13, 14},                                                    MPKIND__NONE,        0, 2,  s4x4,   s1x3,   0x000, 0};
static map_thing map3lo            = {{8, 6, 5, 13, 14, 0},                                                     MPKIND__NONE,        0, 2,  s4x4,   s1x3,   0x000, 0};
static map_thing map328a           = {{0, 1, 2, 10, 9, 8},                                                      MPKIND__NONE,        0, 2,  s2x8,   s1x3,   0x000, 0};
static map_thing map328b           = {{5, 6, 7, 15, 14, 13},                                                    MPKIND__NONE,        0, 2,  s2x8,   s1x3,   0x000, 0};
static map_thing map328c           = {{1, 2, 3, 11, 10, 9},                                                     MPKIND__NONE,        0, 2,  s2x8,   s1x3,   0x000, 0};
static map_thing map328d           = {{4, 5, 6, 14, 13, 12},                                                    MPKIND__NONE,        0, 2,  s2x8,   s1x3,   0x000, 0};
static map_thing map328e           = {{2, 3, 4, 12, 11, 10},                                                    MPKIND__NONE,        0, 2,  s2x8,   s1x3,   0x000, 0};
static map_thing map328f           = {{3, 4, 5, 13, 12, 11},                                                    MPKIND__NONE,        0, 2,  s2x8,   s1x3,   0x000, 0};
static map_thing map31x3d          = {{0, 1, 2, 6, 5, 4},                                                       MPKIND__NONE,        0, 2, s1x3dmd, s1x3,   0x000, 0};
static map_thing map3lqtg          = {{1, 3, 4, 0, 7, 5},                                                       MPKIND__NONE,        0, 2,  s_qtag, s1x3,   0x005, 0};



       clw3_thing clw3_table[] = {
         {s4x4, 0x8E8E, 0x8A8A, &map3ri,  0, {12, 13, 14, 0, 8, 6,  5, 4, -1}},
         {s4x4, 0x8E8E, 0x8C8C, &map3li,  0, {12, 13, 14, 0, 8, 6,  5, 4, -1}},
         {s4x4, 0x7171, 0x7070, &map3ro,  0, {10, 15,  3, 1, 9, 11, 7, 2, -1}},
         {s4x4, 0x7171, 0x6161, &map3lo,  0, {10, 15,  3, 1, 9, 11, 7, 2, -1}},
         {s4x4, 0xE8E8, 0xA8A8, &map3ri,  1, {12, 13, 14, 0, 8, 6,  5, 4, -1}},
         {s4x4, 0xE8E8, 0xC8C8, &map3li,  1, {12, 13, 14, 0, 8, 6,  5, 4, -1}},
         {s4x4, 0x1717, 0x0707, &map3ro,  1, {10, 15,  3, 1, 9, 11, 7, 2, -1}},
         {s4x4, 0x1717, 0x1616, &map3lo,  1, {10, 15,  3, 1, 9, 11, 7, 2, -1}},
         {s2x8, 0x0F0F, 0x0707, &map328a, 0, {4, 5, 6, 7, 12, 13, 14, 15, -1}},
         {s2x8, 0xF0F0, 0xE0E0, &map328b, 0, {0, 1, 2, 3, 8, 9, 10, 11, -1}},
         {s2x8, 0x1F1F, 0x0E0E, &map328c, 0, {5, 6, 7, 13, 14, 15, -1}},
         {s2x8, 0xF8F8, 0x7070, &map328d, 0, {0, 1, 2, 8, 9, 10, -1}},
         {s2x8, 0x3E3E, 0x1C1C, &map328e, 0, {0, 6, 7, 8, 14, 15, -1}},
         {s2x8, 0x7C7C, 0x3838, &map328f, 0, {0, 1, 7, 8, 9, 15, -1}},
         {s1x3dmd, 0x77,  0x77, &map31x3d,0, {3, 7, -1}},
         {s_qtag, 0xBB,   0xBB, &map3lqtg,0, {2, 6, -1}},
         {0, 0}};

// Maps for turning triangles into boxes for the "triangle" concept.

        map_thing map_trngl_box1        = {{1, 2, -1, 0},                                                            MPKIND__NONE,        1, 1,  s_trngl, s2x2,     0x000, 0};
        map_thing map_trngl_box2        = {{1, 2, 0, -1},                                                            MPKIND__NONE,        1, 1,  s_trngl, s2x2,     0x000, 0};

        map_thing map_inner_box         = {{2, 3, 4, 5, 10, 11, 12, 13},                                             MPKIND__NONE,        0, 1,  s2x8,   s2x4,      0x000, 0};


        map_thing map_lh_c1phana        = {{0, -1, 1, -1, 2, -1, 3, -1, 6, -1, 7, -1, 8, -1, 9, -1},                 MPKIND__OFFS_L_HALF, 0, 1,  sbigdmd, s_c1phan, 0x000, 0};
        map_thing map_lh_c1phanb        = {{-1, 12, -1, 10, -1, 3, -1, 15, -1, 4, -1, 2, -1, 11, -1, 7},             MPKIND__OFFS_L_HALF, 4, 1,  s4x4,   s_c1phan,  0x000, 0};

        map_thing map_rh_c1phana        = {{-1, 2, -1, 3, -1, 5, -1, 4, -1, 8, -1, 9, -1, 11, -1, 10},               MPKIND__OFFS_R_HALF, 0, 1,  sbigdmd, s_c1phan, 0x000, 0};
        map_thing map_rh_c1phanb        = {{15, -1, 3, -1, 0, -1, 1, -1, 7, -1, 11, -1, 8, -1, 9, -1},               MPKIND__OFFS_R_HALF, 4, 1,  s4x4,   s_c1phan,  0x000, 0};

        map_thing map_lh_s2x3_3         = {{1, 2, 4, 5, 7, 3,                 13, 15, 11, 9, 10, 12},                MPKIND__OFFS_L_HALF, 0, 2,  s4x4,   s2x3,      0x005, 1};
        map_thing map_lh_s2x3_2         = {{9, 11, 7, 5, 6, 8,                13, 14, 0, 1, 3, 15},                  MPKIND__OFFS_L_HALF, 0, 2,  s4x4,   s2x3,      0x000, 0};

        map_thing map_rh_s2x3_3         = {{15, 11, 6, 8, 9, 10,              0, 1, 2, 7, 3, 14},                    MPKIND__OFFS_R_HALF, 0, 2,  s4x4,   s2x3,      0x005, 1};
        map_thing map_rh_s2x3_2         = {{12, 13, 14, 3, 15, 10,            11, 7, 2, 4, 5, 6},                    MPKIND__OFFS_R_HALF, 0, 2,  s4x4,   s2x3,      0x000, 0};

        map_thing map_lh_s2x3_7         = {{6, 23, 12, 13, 22, 7,              1, 10, 19, 18, 11, 0},                MPKIND__OFFS_L_FULL, 0, 2,  s4x6,   s2x3,      0x005, 1};
        map_thing map_rh_s2x3_7         = {{10, 19, 16, 17, 18, 11,            5, 6, 23, 22, 7, 4},                  MPKIND__OFFS_R_FULL, 0, 2,  s4x6,   s2x3,      0x005, 1};

Private map_thing map_blob_1x4a         = {{13, 10, 6, 8,          15, 17, 3, 5,          18, 20, 1, 22},            MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x000, 1};
Private map_thing map_blob_1x4b         = {{19, 16, 12, 14,        21, 23, 9, 11,         0, 2, 7, 4},               MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x000, 1};
Private map_thing map_blob_1x4c         = {{1, 22, 18, 20,         3, 5, 15, 17,          6, 8, 13, 10},             MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x015, 0};
Private map_thing map_blob_1x4d         = {{19, 16, 12, 14,        21, 23, 9, 11,         0, 2, 7, 4},               MPKIND__NONE,        0, 3,  s_bigblob,   s1x4, 0x015, 0};
Private map_thing map_wblob_1x4a        = {{15, 17, 5, 3, 6, 8, 10, 13,       18, 20, 22, 1, 3, 5, 17, 15},          MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x000, 1};
Private map_thing map_wblob_1x4b        = {{21, 23, 11, 9, 12, 14, 16, 19,    0, 2, 4, 7, 9, 11, 23, 21},            MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x000, 1};
Private map_thing map_wblob_1x4c        = {{3, 5, 17, 15, 18, 20, 22, 1,      6, 8, 10, 13, 15, 17, 5, 3},           MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x005, 0};
Private map_thing map_wblob_1x4d        = {{21, 23, 11, 9, 12, 14, 16, 19,    0, 2, 4, 7, 9, 11, 23, 21},            MPKIND__NONE,        0, 2,  s_bigblob,   s2x4, 0x005, 0};

        map_thing map_d1x10    = {{0, 1, 3, 2,    8, 7, 5, 6},                                                       MPKIND__NONE,        0, 2,  s1x10,   s1x4,      0x000, 0};

        map_thing map_lz12     = {{10, 9,                1, 11,                5, 7,                 3, 4},          MPKIND__NONE,        0, 4,  s3x4,   s1x2,      0x055, 0};
        map_thing map_rz12     = {{0, 10,                11, 8,                2, 5,                 4, 6},          MPKIND__NONE,        0, 4,  s3x4,   s1x2,      0x055, 0};

        map_thing map_tgl451   = {{15, 16, 17, 12, 13, 14,                   2, 3, 4, 5, 6, 7},                      MPKIND__NONE,        0, 2,  s4x5,   s2x3,      0x000, 0};

        map_thing map_tgl452   = {{17, 18, 19, 10, 11, 12,                   0, 1, 2, 7, 8, 9},                      MPKIND__NONE,        0, 2,  s4x5,   s2x3,      0x000, 0};

        map_thing map_dmd_1x1  = {{0,                    1,                    2,                  3},               MPKIND__NONE,        0, 4,  sdmd,        s1x1, 0x000, 0};
        map_thing map_star_1x1 = {{0,                    1,                    2,                  3},               MPKIND__NONE,        0, 4,  s_star,      s1x1, 0x000, 0};

        map_thing map_qtag_f0           = {{7, 3,                           0, 0},                                   MPKIND__NONE,        0, 1,  s_qtag,      s1x2, 0x000, 0};
        map_thing map_qtag_f1           = {{1, 3,                           7, 5},                                   MPKIND__NONE,        0, 2,  s_qtag,      s1x2, 0x005, 0};
        map_thing map_qtag_f2           = {{3, 4,                           0, 7},                                   MPKIND__NONE,        0, 2,  s_qtag,      s1x2, 0x005, 0};

        map_thing map_diag2a            = {{5, 7, 21, 15, 17, 19, 9, 3},                                             MPKIND__NONE,        0, 1,  s4x6,        s2x4,      1, 0};
        map_thing map_diag2b            = {{2, 8, 22, 12, 14, 20, 10, 0},                                            MPKIND__NONE,        0, 1,  s4x6,        s2x4,      1, 0};
        map_thing map_diag23a           = {{13, 3, 2, 5, 11, 10},                                                    MPKIND__NONE,        0, 1,  s4x4,        s2x3,      0, 0};
        map_thing map_diag23b           = {{13, 3, 2, 5, 11, 10},                                                    MPKIND__NONE,        0, 1,  s4x4,        s2x3,      1, 0};
        map_thing map_diag23c           = {{9, 15, 14, 1, 7, 6},                                                     MPKIND__NONE,        0, 1,  s4x4,        s2x3,      0, 0};
        map_thing map_diag23d           = {{1, 7, 6, 9, 15, 14},                                                     MPKIND__NONE,        0, 1,  s4x4,        s2x3,      1, 0};


        map_thing map_f2x8_4x4          = {{-1, 3, 12, 2, -1, -1, -1, 13, -1, 15, 0, 14, -1, -1, -1, 1,
                                            -1, 7,  8, 6, -1, -1, -1,  9, -1, 11, 4, 10, -1, -1, -1, 5},             MPKIND__SPLIT,       0, 2,  sfat2x8,  s4x4,    0x000, 0};
        map_thing map_w4x4_4x4          = {{13, 15, 11, 10, 6, 8, -1, 9, -1, -1, -1, -1, -1, -1, 12, -1,
                                            -1, -1, -1, -1, -1, -1, 4, -1, 5, 7, 3, 2, 14, 0, -1, 1},                MPKIND__SPLIT,       0, 2,  swide4x4, s4x4,    0x000, 0};
        map_thing map_f2x8_2x8          = {{15, 14, 13, 12, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1
                                            -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, 3, 2, 1, 0},                 MPKIND__SPLIT,       0, 2,  sfat2x8,  s2x8,    0x000, 1};
        map_thing map_w4x4_2x8          = {{-1, -1, 9, 11, 7, 2, -1, -1, -1, -1,  4, 5, 6, 8, -1, -1,
                                            -1, -1, 12, 13, 14, 0, -1, -1, -1, -1, 1, 3, 15, 10, -1, -1},            MPKIND__SPLIT,       0, 2,  swide4x4, s2x8,    0x000, 1};

       map_thing map_emergency1        = {{17, 18, 19, -1, -1, 10, 11, 12,   -1, 0, 1, 2, 7, 8, 9, -1},             MPKIND__OFFS_L_THRQ, 0, 2,  s4x5,   s2x4,      0x000, 1};

       map_thing map_emergency2        = {{-1, 15, 16, 17, 12, 13, 14, -1,   2, 3, 4, -1, -1, 5, 6, 7},             MPKIND__OFFS_R_THRQ, 0, 2,  s4x5,   s2x4,      0x000, 1};


        map_thing *maps_3diag[4] = {&map_blob_1x4c, &map_blob_1x4a, &map_blob_1x4d, &map_blob_1x4b};
        map_thing *maps_3diagwk[4] = {&map_wblob_1x4a, &map_wblob_1x4c, &map_wblob_1x4b, &map_wblob_1x4d};


map_thing map_init_table2[] = {
   {{0, 1, 5, 4, 6, 7, 11, 10},                                               MPKIND_DBLBENTCW,    0, 1,  sbigh,       s1x8, 0x000, 0, MAPCODE(s1x8,1,MPKIND_DBLBENTCW,    0)},
   {{3, 2, 5, 4, 9, 8, 11, 10},                                               MPKIND_DBLBENTCCW,   0, 1,  sbigh,       s1x8, 0x000, 0, MAPCODE(s1x8,1,MPKIND_DBLBENTCCW,   0)},
   {{1, 9, 8, 12, 13, 21, 20, 0},                                             MPKIND_DBLBENTCW,    0, 1,  s4x6,        s2x4, 0x000, 0, MAPCODE(s2x4,1,MPKIND_DBLBENTCW,    0)},
   {{17, 9, 8, 4, 5, 21, 20, 16},                                             MPKIND_DBLBENTCCW,   0, 1,  s4x6,        s2x4, 0x000, 0, MAPCODE(s2x4,1,MPKIND_DBLBENTCCW,   0)},

   {{0, 1, 3, 2,            4, 5, 10, 11,          9, 8, 6, 7},               MPKIND__NONISOTROPIC,0, 3,  sbigh,       s1x4, 0x011, 0, MAPCODE(s1x4,3,MPKIND__NONISOTROPIC,1)},
   {{0, 1, 3, 2,            4, 5, 10, 11,          9, 8, 6, 7},               MPKIND__NONISOTROPIC,0, 3,  sbigx,       s1x4, 0x004, 0, MAPCODE(s1x4,3,MPKIND__NONISOTROPIC,0)},
   {{0, 10, 8, 9,           11, 1, 5, 7,           2, 3, 6, 4},               MPKIND__NONISOTROPIC,0, 3,  s_3mdmd,     sdmd, 0x011, 0, MAPCODE(sdmd,3,MPKIND__NONISOTROPIC,1)},
   {{9, 0, 10, 8,           1, 5, 7, 11,           4, 2, 3, 6},               MPKIND__NONISOTROPIC,0, 3,  s_3mptpd,    sdmd, 0x004, 0, MAPCODE(sdmd,3,MPKIND__NONISOTROPIC,0)},

   {{0, 1, 3, 2,        4, 5, 7, 6,        15, 14, 12, 13,  11, 10, 8, 9},    MPKIND__NONISOTROPIC,0, 4,  sbigbigh,    s1x4, 0x041, 0, MAPCODE(s1x4,4,MPKIND__NONISOTROPIC,1)},
   {{0, 1, 3, 2,        4, 5, 7, 6,        15, 14, 12, 13,  11, 10, 8, 9},    MPKIND__NONISOTROPIC,0, 4,  sbigbigx,    s1x4, 0x014, 0, MAPCODE(s1x4,4,MPKIND__NONISOTROPIC,0)},
   {{0, 13, 11, 12,     14, 1, 15, 10,     7, 2, 6, 9,      3, 4, 8, 5},      MPKIND__NONISOTROPIC,0, 4,  s_4mdmd,     sdmd, 0x041, 0, MAPCODE(sdmd,4,MPKIND__NONISOTROPIC,1)},
   {{12, 0, 13, 11,     1, 15, 10, 14,     2, 6, 9, 7,      5, 3, 4, 8},      MPKIND__NONISOTROPIC,0, 4,  s_4mptpd,    sdmd, 0x014, 0, MAPCODE(sdmd,4,MPKIND__NONISOTROPIC,0)},


   {{5, 4, 0,                          2, 1, 3},                              MPKIND__SPLIT,       0, 2,  s_bone6, s_trngl,  0x207, 0, MAPCODE(s_trngl,2,MPKIND__NONISOTROP1,1)},
   {{7, 6, 5, 0,                       3, 2, 1, 4},                           MPKIND__SPLIT,       0, 2,  s_bone, s_trngl4,  0x207, 0, MAPCODE(s_trngl4,2,MPKIND__NONISOTROP1,1)},

   {{1, 3, 15, 13,                     9, 11, 7, 5},                          MPKIND__SPLIT,       0, 2,  s_c1phan,s_trngl4, 0x202, 0, MAPCODE(s_trngl4,2,MPKIND__NONISOTROP1,0)},





   {{12, 15, 14, 1, 4, 7, 6, 9,        10, 13, 3, 0, 2, 5, 11, 8},            MPKIND__STAG,        0, 2,  s4x4,        s2x4, 0x000, 1, MAPCODE(s2x4,2,MPKIND__STAG,        0)},
   {{10, 13, 3, 0, 2, 5, 11, 8,        12, 15, 14, 1, 4, 7, 6, 9},            MPKIND__STAG,        0, 2,  s4x4,        s2x4, 0x000, 1, MAPCODE(s2x4,2,MPKIND__STAG,        1)},
   {{12, 14, 1, 7, 4, 6, 9, 15,        13, 0, 2, 3, 5, 8, 10, 11},            MPKIND__STAG,        0, 2,  s4x4,      s_qtag, 0x000, 1, MAPCODE(s_qtag,2,MPKIND__STAG,      0)},
   {{13, 0, 2, 3, 5, 8, 10, 11,        12, 14, 1, 7, 4, 6, 9, 15},            MPKIND__STAG,        0, 2,  s4x4,      s_qtag, 0x000, 1, MAPCODE(s_qtag,2,MPKIND__STAG,      1)},

   {{14, 1, 4, 7, 6, 9, 12, 15,        2, 5, 8, 11, 10, 13, 0, 3},            MPKIND__DIAGQTAG,    0, 2,  s4x4,      s_qtag, 0x000, 1, MAPCODE(s_qtag,2,MPKIND__DIAGQTAG,  0)},
   {{14, 1, 4, 7, 6, 9, 12, 15,        10, 13, 0, 3, 2, 5, 8, 11},            MPKIND__DIAGQTAG,    0, 2,  s4x4,      s_qtag, 0x000, 1, MAPCODE(s_qtag,2,MPKIND__DIAGQTAG,  1)},

   {{0, 3, 4, 7, 2, 5, 6, 1},                                                 MPKIND__DMD_STUFF,   0, 2,  s_thar,      sdmd, 0x104, 0, MAPCODE(sdmd,2,MPKIND__DMD_STUFF,   0)},
   {{0, 3, 4, 7, 2, 5, 6, 1},                                                 MPKIND__DMD_STUFF,   0, 2,  s_thar,      sdmd, 0x004, 0, MAPCODE(sdmd,2,MPKIND__DMD_STUFF,   1)},
   {{0, 2,                 1, 3},                                             MPKIND__DMD_STUFF,   0, 2,  sdmd,        s1x2, 0x004, 0, MAPCODE(s1x2,2,MPKIND__DMD_STUFF,   0)},
   {{1, 3,                 0, 2},                                             MPKIND__DMD_STUFF,   0, 2,  sdmd,        s1x2, 0x001, 0, MAPCODE(s1x2,2,MPKIND__DMD_STUFF,   1)},
   {{0, 1, 2, 3, 1, 2, 3, 0},                                                 MPKIND__DMD_STUFF,   0, 2,  s2x2,        s2x2, 0x104, 0, MAPCODE(s2x2,2,MPKIND__DMD_STUFF,   0)},
   {{0, 1, 4, 5, 2, 3, 6, 7},                                                 MPKIND__DMD_STUFF,   0, 2,  s_thar,      s1x4, 0x104, 0, MAPCODE(s1x4,2,MPKIND__DMD_STUFF,   0)},
   {{0, 1, 4, 5, 2, 3, 6, 7},                                                 MPKIND__DMD_STUFF,   0, 2,  s_thar,      s1x4, 0x004, 0, MAPCODE(s1x4,2,MPKIND__DMD_STUFF,   1)},
   {{13, 14, 5, 6,         1, 2, 9, 10},                                      MPKIND__ALL_8,       0, 2,  s4x4,        s2x2, 0x004, 0, MAPCODE(s2x2,2,MPKIND__ALL_8,       0)},
   {{1, 2, 9, 10,          13, 14, 5, 6},                                     MPKIND__ALL_8,       0, 2,  s4x4,        s2x2, 0x001, 0, MAPCODE(s2x2,2,MPKIND__ALL_8,       1)},
   {{0, 3, 4, 7,           2, 5, 6, 1},                                       MPKIND__ALL_8,       0, 2,  s_thar,      sdmd, 0x004, 0, MAPCODE(sdmd,2,MPKIND__ALL_8,       0)},
   {{2, 5, 6, 1,           0, 3, 4, 7},                                       MPKIND__ALL_8,       0, 2,  s_thar,      sdmd, 0x001, 0, MAPCODE(sdmd,2,MPKIND__ALL_8,       1)},
   {{0, 1, 4, 5,           2, 3, 6, 7},                                       MPKIND__ALL_8,       0, 2,  s_thar,      s1x4, 0x004, 0, MAPCODE(s1x4,2,MPKIND__ALL_8,       0)},
   {{2, 3, 6, 7,           0, 1, 4, 5},                                       MPKIND__ALL_8,       0, 2,  s_thar,      s1x4, 0x001, 0, MAPCODE(s1x4,2,MPKIND__ALL_8,       1)},

   {{13, 14,               1, 2,                 6, 5,               10, 9},  MPKIND__4_EDGES,     0, 4,  s4x4,        s1x2, 0x044, 0, MAPCODE(s1x2,4,MPKIND__4_EDGES,     0)},
   {{2, 3,                 5, 4,                 7, 6,               0, 1},   MPKIND__4_EDGES,     0, 4,  s_thar,      s1x2, 0x011, 0, MAPCODE(s1x2,4,MPKIND__4_EDGES,     1)},
   {{0, 2,                 4, 6,                 10, 8,              14, 12}, MPKIND__4_QUADRANTS, 0, 4,  s_c1phan,    s1x2, 0x044, 0, MAPCODE(s1x2,4,MPKIND__4_QUADRANTS, 0)},
   {{1, 3,                 7, 5,                 11, 9,              13, 15}, MPKIND__4_QUADRANTS, 0, 4,  s_c1phan,    s1x2, 0x011, 0, MAPCODE(s1x2,4,MPKIND__4_QUADRANTS, 1)},
   {{0, 1, 2, 3,        7, 4, 5, 6,        10, 11, 8, 9,    13, 14, 15, 12},  MPKIND__4_QUADRANTS, 0, 4,  s_c1phan,  s_star, 0x000, 0, MAPCODE(s_star,4,MPKIND__4_QUADRANTS, 0)},

   {{-1, 0, 1, 2, 11, 12, 13, 14,      3, 4, 5, 6, -1, 8, 9, 10},             MPKIND__OFFS_L_HALF_SPECIAL, 0, 2,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__OFFS_L_HALF_SPECIAL, 0)},
   {{1, 2, 3, 4, 13, 14, 15, -1,       5, 6, 7, -1, 9, 10, 11, 12},           MPKIND__OFFS_R_HALF_SPECIAL, 0, 2,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__OFFS_R_HALF_SPECIAL, 0)},

   {{1, 8, 17, 12, 13, 16, 9, 0,       3, 6, 19, 10, 11, 18, 7, 2},           MPKIND__OFFS_L_HALF_SPECIAL, 0, 2,  s4x5,   s2x4,      0x005, 0, MAPCODE(s2x4,2,MPKIND__OFFS_L_HALF_SPECIAL, 1)},
   {{2, 7, 16, 13, 14, 15, 8, 1,       4, 5, 18, 11, 12, 17, 6, 3},           MPKIND__OFFS_R_HALF_SPECIAL, 0, 2,  s4x5,   s2x4,      0x005, 0, MAPCODE(s2x4,2,MPKIND__OFFS_R_HALF_SPECIAL, 1)},

   {{0, 1, 2, 3, 6, 7, 8, 9},                                                 MPKIND__OFFS_L_HALF, 0, 1,  s2x6,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_L_HALF, 0)},
   {{2, 3, 4, 5, 8, 9, 10, 11},                                               MPKIND__OFFS_R_HALF, 0, 1,  s2x6,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_R_HALF, 0)},

   {{0, 1, 2, 3, 5, 6, 7, 8},                                                 MPKIND__OFFS_L_ONEQ, 0, 1,  s2x5,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_L_ONEQ, 0)},
   {{1, 2, 3, 4, 6, 7, 8, 9},                                                 MPKIND__OFFS_R_ONEQ, 0, 1,  s2x5,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_R_ONEQ, 0)},

   {{0, 1, 2, 3, 7, 8, 9, 10},                                                MPKIND__OFFS_L_THRQ, 0, 1,  s2x7,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_L_THRQ, 0)},
   {{3, 4, 5, 6, 10, 11, 12, 13},                                             MPKIND__OFFS_R_THRQ, 0, 1,  s2x7,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_R_THRQ, 0)},

   {{0, 1, 2, 3, 8, 9, 10, 11},                                               MPKIND__OFFS_L_FULL, 0, 1,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_L_FULL, 0)},
   {{4, 5, 6, 7, 12, 13, 14, 15},                                             MPKIND__OFFS_R_FULL, 0, 1,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,1,MPKIND__OFFS_R_FULL, 0)},

   {{0, 1, 2, 3, 7, 8, 9, 10, 11, 12, 16, 17},                                MPKIND__OFFS_L_HALF, 7, 1,  s3x6,   s3x4,      0x000, 0, MAPCODE(s3x4,1,MPKIND__OFFS_L_HALF, 0)},
  {{2, 3, 4, 5, 7, 8, 11, 12, 13, 14, 16, 17},                                MPKIND__OFFS_R_HALF, 7, 1,  s3x6,   s3x4,      0x000, 0, MAPCODE(s3x4,1,MPKIND__OFFS_R_HALF, 0)},
   {{1, 2, 7, 8, 10, 11, 16, 17},                                             MPKIND__OFFS_L_HALF, 7, 1,  s3x6,   s_qtag,    0x000, 0, MAPCODE(s_qtag,1,MPKIND__OFFS_L_HALF, 0)},
   {{3, 4, 7, 8, 12, 13, 16, 17},                                             MPKIND__OFFS_R_HALF, 7, 1,  s3x6,   s_qtag,    0x000, 0, MAPCODE(s_qtag,1,MPKIND__OFFS_R_HALF, 0)},

   {{-1, -1, 0, 1, 2, 3, -1, -1, 5, 6, 7, 8},                                 MPKIND__OFFS_L_HALF, 5, 1,  s2x5,   s2x6,      0x000, 0, MAPCODE(s2x6,1,MPKIND__OFFS_L_HALF, 0)},
   {{1, 2, 3, 4, -1, -1, 6, 7, 8, 9, -1, -1},                                 MPKIND__OFFS_R_HALF, 5, 1,  s2x5,   s2x6,      0x000, 0, MAPCODE(s2x6,1,MPKIND__OFFS_R_HALF, 0)},

   {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_L_HALF, 2, 1,  s1x8,   s1x8,      0x000, 0, MAPCODE(s1x8,1,MPKIND__OFFS_L_HALF, 0)},
   {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_R_HALF, 2, 1,  s1x8,   s1x8,      0x000, 0, MAPCODE(s1x8,1,MPKIND__OFFS_R_HALF, 0)},
   {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_L_FULL, 2, 1,  s1x8,   s1x8,      0x000, 0, MAPCODE(s1x8,1,MPKIND__OFFS_L_FULL, 0)},
   {{0, 1, 2, 3, 4, 5, 6, 7},                                                 MPKIND__OFFS_R_FULL, 2, 1,  s1x8,   s1x8,      0x000, 0, MAPCODE(s1x8,1,MPKIND__OFFS_R_FULL, 0)},

   {{10, 11, 2, 3, 4, 5, 8, 9},                                               MPKIND__OFFS_L_HALF, 0, 1,  s3x4,   s2x4,      0x000, 1, MAPCODE(s2x4,1,MPKIND__OFFS_L_HALF, 1)},
   {{0, 1, 5, 4, 6, 7, 11, 10},                                               MPKIND__OFFS_R_HALF, 0, 1,  s3x4,   s2x4,      0x000, 1, MAPCODE(s2x4,1,MPKIND__OFFS_R_HALF, 1)},

   {{15, 16, 17, 3, 4, 5, 6, 7, 8, 12, 13, 14},                               MPKIND__OFFS_L_HALF, 0, 1,  s3x6,   s2x6,      0x000, 1, MAPCODE(s2x6,1,MPKIND__OFFS_L_HALF, 1)},
   {{0, 1, 2, 8, 7, 6, 9, 10, 11, 17, 16, 15},                                MPKIND__OFFS_R_HALF, 0, 1,  s3x6,   s2x6,      0x000, 1, MAPCODE(s2x6,1,MPKIND__OFFS_R_HALF, 1)},
   {{13, 15, 2, 4, 5, 7, 10, 12},                                             MPKIND__OFFS_L_FULL, 0, 1,  s4x4,   s2x4,      0x001, 0, MAPCODE(s2x4,1,MPKIND__OFFS_L_FULL, 1)},
   {{0, 1, 11, 6, 8, 9, 3, 14},                                               MPKIND__OFFS_R_FULL, 0, 1,  s4x4,   s2x4,      0x001, 0, MAPCODE(s2x4,1,MPKIND__OFFS_R_FULL, 1)},

   {{9, 11, 14, 0, 1, 3, 6, 8,         12, 13, 7, 2, 4, 5, 15, 10},           MPKIND__OFFS_BOTH_FULL, 0, 2, s4x4, s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_BOTH_FULL, 1)},


   {{0, 1, 2, 3, 8, 9, 10, 11,         4, 5, 6, 7, 12, 13, 14, 15},           MPKIND__OFFS_BOTH_FULL, 0, 2, s2x8, s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__OFFS_BOTH_FULL, 0)},


   {{10, 11, 8, 9,                     2, 3, 4, 5},                           MPKIND__OFFS_L_HALF, 0, 2,  s3x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__OFFS_L_HALF, 0)},
   {{0, 1, 11, 10,                     5, 4, 6, 7},                           MPKIND__OFFS_R_HALF, 0, 2,  s3x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__OFFS_R_HALF, 0)},
   {{9, 11, 6, 8,                      14, 0, 1, 3},                          MPKIND__OFFS_L_FULL, 0, 2,  s4x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__OFFS_L_FULL, 0)},
   {{12, 13, 15, 10,                   7, 2, 4, 5},                           MPKIND__OFFS_R_FULL, 0, 2,  s4x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__OFFS_R_FULL, 0)},

   {{11, 8, 9, 10,                     3, 4, 5, 2},                           MPKIND__OFFS_L_HALF, 0, 2,  s3x4,   s2x2,      0x005, 0, MAPCODE(s2x2,2,MPKIND__OFFS_L_HALF, 1)},
   {{1, 11, 10, 0,                     4, 6, 7, 5},                           MPKIND__OFFS_R_HALF, 0, 2,  s3x4,   s2x2,      0x005, 0, MAPCODE(s2x2,2,MPKIND__OFFS_R_HALF, 1)},
   {{11, 6, 8, 9,                      0, 1, 3, 14},                          MPKIND__OFFS_L_FULL, 0, 2,  s4x4,   s2x2,      0x005, 0, MAPCODE(s2x2,2,MPKIND__OFFS_L_FULL, 1)},
   {{13, 15, 10, 12,                   2, 4, 5, 7},                           MPKIND__OFFS_R_FULL, 0, 2,  s4x4,   s2x2,      0x005, 0, MAPCODE(s2x2,2,MPKIND__OFFS_R_FULL, 1)},

   {{15, 14, 12, 13,                   4, 5, 7, 6},                           MPKIND__OFFS_L_HALF, 0, 2,  s1p5x8, s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF, 0)},
   {{0, 1, 3, 2,                       11, 10, 8, 9},                         MPKIND__OFFS_R_HALF, 0, 2,  s1p5x8, s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF, 0)},
   {{15, 14, 12, 13,                   4, 5, 7, 6},                           MPKIND__OFFS_L_FULL, 0, 2,  s2x8,   s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__OFFS_L_FULL, 0)},
   {{0, 1, 3, 2,                       11, 10, 8, 9},                         MPKIND__OFFS_R_FULL, 0, 2,  s2x8,   s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__OFFS_R_FULL, 0)},



   {{0, 1, 4, 5},                                                             MPKIND__OFFS_R_FULL, 0, 1,  s2x4,   s1x4,      0x000, 1, MAPCODE(s1x4,1,MPKIND__OFFS_L_FULL, 1)},
   {{7, 6, 3, 2},                                                             MPKIND__OFFS_L_FULL, 0, 1,  s2x4,   s1x4,      0x000, 1, MAPCODE(s1x4,1,MPKIND__OFFS_R_FULL, 1)},

   {{0, 1, 4, 5},                                                             MPKIND__OFFS_R_FULL, 0, 1,  s2x4,   s2x2,      0x000, 0, MAPCODE(s2x2,1,MPKIND__OFFS_L_FULL, 0)},
   {{2, 3, 6, 7},                                                             MPKIND__OFFS_R_FULL, 0, 1,  s2x4,   s2x2,      0x000, 0, MAPCODE(s2x2,1,MPKIND__OFFS_R_FULL, 0)},



   {{9, 8, 6, 7,                       0, 1, 3, 2},                           MPKIND__OFFS_L_HALF, 0, 2,  s2x6,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF, 1)},
   {{11, 10, 8, 9,                     2, 3, 5, 4},                           MPKIND__OFFS_R_HALF, 0, 2,  s2x6,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF, 1)},

   {{8, 7, 5, 6,                       0, 1, 3, 2},                           MPKIND__OFFS_L_ONEQ, 0, 2,  s2x5,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_L_ONEQ, 1)},
   {{9, 8, 6, 7,                       1, 2, 4, 3},                           MPKIND__OFFS_R_ONEQ, 0, 2,  s2x5,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_R_ONEQ, 1)},

   {{10, 9, 7, 8,                      0, 1, 3, 2},                           MPKIND__OFFS_L_THRQ, 0, 2,  s2x7,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_L_THRQ, 1)},
   {{13, 12, 10, 11,                   3, 4, 6, 5},                           MPKIND__OFFS_R_THRQ, 0, 2,  s2x7,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_R_THRQ, 1)},

   {{11, 10, 8, 9,                     0, 1, 3, 2},                           MPKIND__OFFS_L_FULL, 0, 2,  s2x8,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_L_FULL, 1)},
   {{15, 14, 12, 13,                   4, 5, 7, 6},                           MPKIND__OFFS_R_FULL, 0, 2,  s2x8,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__OFFS_R_FULL, 1)},

   {{6, 7, 8, 9,                       0, 1, 2, 3},                           MPKIND__OFFS_L_HALF, 0, 2,  sbigdmd,s_trngl4,  0x107, 1, MAPCODE(s_trngl4,2,MPKIND__OFFS_L_HALF, 1)},
   {{11, 10, 9, 8,                     5, 4, 3, 2},                           MPKIND__OFFS_R_HALF, 0, 2,  sbigdmd,s_trngl4,  0x10D, 1, MAPCODE(s_trngl4,2,MPKIND__OFFS_R_HALF, 1)},

   {{15, 16, 17, 12, 13, 14,           3, 4, 5, 6, 7, 8},                     MPKIND__OFFS_L_HALF, 0, 2,  s3x6,   s2x3,      0x000, 0, MAPCODE(s2x3,2,MPKIND__OFFS_L_HALF, 0)},
   {{0, 1, 2, 17, 16, 15,              8, 7, 6, 9, 10, 11},                   MPKIND__OFFS_R_HALF, 0, 2,  s3x6,   s2x3,      0x000, 0, MAPCODE(s2x3,2,MPKIND__OFFS_R_HALF, 0)},
   {{20, 21, 22, 23, 16, 17, 18, 19,   4, 5, 6, 7, 8, 9, 10, 11},             MPKIND__OFFS_L_HALF, 0, 2,  s3x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__OFFS_L_HALF, 0)},
   {{0, 1, 2, 3, 23, 22, 21, 20,       11, 10, 9, 8, 12, 13, 14, 15},         MPKIND__OFFS_R_HALF, 0, 2,  s3x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__OFFS_R_HALF, 0)},

   {{16, 17, 18, 19, 10, 11, 12, 13,   0, 1, 2, 3, 6, 7, 8, 9},               MPKIND__OFFS_L_ONEQ, 0, 2,  s4x5,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_L_ONEQ, 1)},
   {{15, 16, 17, 18, 11, 12, 13, 14,   1, 2, 3, 4, 5, 6, 7, 8},               MPKIND__OFFS_R_ONEQ, 0, 2,  s4x5,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_R_ONEQ, 1)},

   {{17, 16, 14, 15, 10, 11, 13, 12,   0, 1, 3, 2, 7, 6, 4, 5},               MPKIND__OFFS_L_ONEQ, 0, 2,  s2x10,  s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__OFFS_L_ONEQ, 1)},
   {{19, 18, 16, 17, 12, 13, 15, 14,   2, 3, 5, 4, 9, 8, 6, 7},               MPKIND__OFFS_R_ONEQ, 0, 2,  s2x10,  s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__OFFS_R_ONEQ, 1)},

   {{10, 9, 8, 7, -1, -1, -1, -1,      -1, -1, -1, -1, 3, 2, 1, 0},           MPKIND__OFFS_L_THRQ, 0, 2,  s2x7,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_L_THRQ, 1)},
   {{13, 12, 11, 10, -1, -1, -1, -1,   -1, -1, -1, -1, 6, 5, 4, 3},           MPKIND__OFFS_R_THRQ, 0, 2,  s2x7,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_R_THRQ, 1)},

   {{18, 17, 15, 16, -1, 12, 14, 13,   -1, 0, 2, 1, 6, 5, 3, 4},              MPKIND__OFFS_L_THRQ, 0, 2,  s2x12,  s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__OFFS_L_THRQ, 1)},
   {{-1, 23, 21, 22, 17, 18, 20, 19,   5, 6, 8, 7, -1, 11, 9, 10},            MPKIND__OFFS_R_THRQ, 0, 2,  s2x12,  s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__OFFS_R_THRQ, 1)},

   {{20, 21, 22, 23, 12, 13, 14, 15,   0, 1, 2, 3, 8, 9, 10, 11},             MPKIND__OFFS_L_HALF, 0, 2,  s4x6,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_L_HALF, 1)},
   {{18, 19, 20, 21, 14, 15, 16, 17,   2, 3, 4, 5, 6, 7, 8, 9},               MPKIND__OFFS_R_HALF, 0, 2,  s4x6,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OFFS_R_HALF, 1)},

   {{13, 12, 10, 11, -1, -1, 9, 8,     -1, -1, 1, 0, 5, 4, 2, 3},             MPKIND__OFFS_L_HALF, 0, 2,  s2x8,   s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__OFFS_L_HALF, 1)},
   {{-1, -1, 14, 15, 10, 11, 13, 12,   2, 3, 5, 4, -1, -1, 6, 7},             MPKIND__OFFS_R_HALF, 0, 2,  s2x8,   s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__OFFS_R_HALF, 1)},

   {{10, 1, 11, 9,                     5, 3, 4, 7},                           MPKIND__LILZCCW,     0, 2,  s3x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__LILZCCW,0)},
   {{0, 11, 8, 10,                     2, 4, 6, 5},                           MPKIND__LILZCW,      0, 2,  s3x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__LILZCW,0)},
   {{15, 1, 16, 14,     17, 3, 8, 12,     7, 5, 6, 10},                       MPKIND__LILZCCW,     0, 3,  s3x6,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__LILZCCW,0)},
   {{0, 16, 13, 15,     2, 8, 11, 17,     4, 6, 9, 7},                        MPKIND__LILZCW,      0, 3,  s3x6,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__LILZCW,0)},
   {{0, 16, 13, 15,     17, 3, 8, 12,     4, 6, 9, 7},                        MPKIND__LILAZCCW,    0, 3,  s3x6,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__LILAZCCW,0)},
   {{15, 1, 16, 14,     2, 8, 11, 17,     7, 5, 6, 10},                       MPKIND__LILAZCW,     0, 3,  s3x6,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__LILAZCW,0)},
   {{1, 11, 8, 9, 10, 0,               3, 4, 6, 7, 5, 2},                     MPKIND__LILZCOM,     0, 2,  s3x4,   s2x3,      0x005, 0, MAPCODE(s2x3,2,MPKIND__LILZCOM,1)},
   {{1, 16, 13, 14, 15, 0,  3, 8, 11, 12, 17, 2,   5, 6, 9, 10, 7, 4},        MPKIND__LILZCOM,     0, 3,  s3x6,   s2x3,      0x015, 0, MAPCODE(s2x3,3,MPKIND__LILZCOM,1)},

   {{0, 1, 6, 7, 8, 9, 14, 15,         2, 3, 4, 5, 10, 11, 12, 13},           MPKIND__CONCPHAN,    0, 2,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__CONCPHAN,    0)},
   {{0, 1, 4, 5, 10, 11, 14, 15,       2, 3, 6, 7, 8, 9, 12, 13},             MPKIND__INTLK,       0, 2,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__INTLK,       0)},
   {{12, 13, 14, 0, 4, 5, 6, 8,        10, 15, 3, 1, 2, 7, 11, 9},            MPKIND__CONCPHAN,    0, 2,  s4x4,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__CONCPHAN,    1)},
   {{10, 15, 3, 1, 4, 5, 6, 8,         12, 13, 14, 0, 2, 7, 11, 9},           MPKIND__INTLK,       0, 2,  s4x4,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__INTLK,       1)},
   {{0, 1, 3, 2, 8, 9, 11, 10,         4, 5, 7, 6, 12, 13, 15, 14},           MPKIND__CONCPHAN,    0, 2,  s1x16,  s1x8,      0x000, 0, MAPCODE(s1x8,2,MPKIND__CONCPHAN,    0)},
   {{0, 1, 3, 2, 12, 13, 15, 14,       4, 5, 7, 6, 8, 9, 11, 10},             MPKIND__INTLK,       0, 2,  s1x16,  s1x8,      0x000, 0, MAPCODE(s1x8,2,MPKIND__INTLK,       0)},
   {{0, 3, 4, 5, 8, 11, 12, 13,        1, 2, 6, 7, 9, 10, 14, 15},            MPKIND__CONCPHAN,    0, 2,  s4dmd,  s_qtag,    0x000, 0, MAPCODE(s_qtag,2,MPKIND__CONCPHAN,  0)},
   {{0, 2, 6, 7, 9, 11, 12, 13,        1, 3, 4, 5, 8, 10, 14, 15},            MPKIND__INTLK,       0, 2,  s4dmd,  s_qtag,    0x000, 0, MAPCODE(s_qtag,2,MPKIND__INTLK,     0)},
   {{12, 0, 13, 11, 4, 8, 5, 3,        14, 1, 15, 10, 6, 9, 7, 2},            MPKIND__CONCPHAN,    0, 2,  s4ptpd, s_ptpd,    0x000, 0, MAPCODE(s_ptpd,2,MPKIND__CONCPHAN,  0)},
   {{12, 0, 13, 11, 6, 9, 7, 2,        14, 1, 15, 10, 4, 8, 5, 3},            MPKIND__INTLK,       0, 2,  s4ptpd, s_ptpd,    0x000, 0, MAPCODE(s_ptpd,2,MPKIND__INTLK,     0)},
   {{12, 0, 13, 11, 15, 10, 14, 1,     7, 2, 6, 9, 4, 8, 5, 3},               MPKIND__SPLIT,       0, 2,  s4ptpd, s_ptpd,    0x000, 0, MAPCODE(s_ptpd,2,MPKIND__SPLIT,     0)},
   {{0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17,
                            11, 10, 9, 8, 7, 6, 23, 22, 21, 20, 19, 18},      MPKIND__CONCPHAN,    0, 2,  s4x6,   s2x6,      0x000, 1, MAPCODE(s2x6,2,MPKIND__CONCPHAN,    1)},
   {{11, 10, 9, 8, 7, 6, 12, 13, 14, 15, 16, 17,
                            0, 1, 2, 3, 4, 5, 23, 22, 21, 20, 19, 18},        MPKIND__INTLK,       0, 2,  s4x6,   s2x6,      0x000, 1, MAPCODE(s2x6,2,MPKIND__INTLK,       1)},
   {{3, 4, 6, 9, 10, 0,                2, 5, 7, 8, 11, 1},                    MPKIND__CONCPHAN,    0, 2,  s3x4,   s2x3,      0x005, 0, MAPCODE(s2x3,2,MPKIND__CONCPHAN,    1)},
   {{2, 5, 7, 9, 10, 0,                3, 4, 6, 8, 11, 1},                    MPKIND__INTLK,       0, 2,  s3x4,   s2x3,      0x005, 0, MAPCODE(s2x3,2,MPKIND__INTLK,       1)},

   {{0, 5, 6, 11,           1, 4, 7, 10,           2, 3, 8, 9},               MPKIND__CONCPHAN,    0, 3,  s2x6,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__CONCPHAN,    0)},
   {{5, 6, 11, 0,           4, 7, 10, 1,           3, 8, 9, 2},               MPKIND__CONCPHAN,    0, 3,  s2x6,   s2x2,      0x015, 0, MAPCODE(s2x2,3,MPKIND__CONCPHAN,    1)},
   {{0, 7, 8, 15,     1, 6, 9, 14,     2, 5, 10, 13,     3, 4, 11, 12},       MPKIND__CONCPHAN,    0, 4,  s2x8,   s2x2,      0x000, 0, MAPCODE(s2x2,4,MPKIND__CONCPHAN,    0)},

   {{10, 13, 14, 1, 2, 5, 6, 9,        12, 15, 3, 0, 4, 7, 11, 8},            MPKIND__O_SPOTS,     0, 2,  s4x4,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__O_SPOTS,     1)},
   {{10, 13, 14, 1, 2, 5, 6, 9,        12, 15, 3, 0, 4, 7, 11, 8},            MPKIND__X_SPOTS,     0, 2,  s4x4,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__X_SPOTS,     1)},

   {{1, 2, 3, 4, 11, 23, 15, 16, 17, 18, 21, 22,
                            3, 4, 5, 6, 9, 10, 13, 14, 15, 16, 23, 11},       MPKIND__OVERLAP,     0, 2,  s3x8,   s3x4,      0x000, 0, MAPCODE(s3x4,2,MPKIND__OVERLAP,     0)},
   {{0, 1, 5, 11, 7, 8, 9, 10,         1, 2, 3, 4, 6, 7, 11, 5},              MPKIND__OVERLAP,     0, 2,  s3dmd,  s_qtag,    0x000, 0, MAPCODE(s_qtag,2,MPKIND__OVERLAP,   0)},
   {{9, 0, 10, 8, 5, 7, 11, 1,         11, 1, 5, 7, 3, 6, 4, 2},              MPKIND__OVERLAP,     0, 2,  s3ptpd, s_ptpd,    0x000, 0, MAPCODE(s_ptpd,2,MPKIND__OVERLAP,   0)},
   {{20, 21, 22, 23, 11, 10, 9, 8, 12, 13, 14, 15, 16, 17, 18, 19,
           0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 23, 22, 21, 20},             MPKIND__OVERLAP,     0, 2,  s3x8,   s2x8,      0x000, 1, MAPCODE(s2x8,2,MPKIND__OVERLAP,     1)},
   {{0, 1, 3, 2, 10, 11, 4, 5,         4, 5, 10, 11, 6, 7, 9, 8},             MPKIND__OVERLAP,     0, 2,  s1x12,  s1x8,      0x000, 0, MAPCODE(s1x8,2,MPKIND__OVERLAP,     0)},
   {{0, 1, 3, 2, 7, 6, 4, 5,           4, 5, 7, 6, 12, 13, 15, 14,
                            15, 14, 12, 13, 8, 9, 11, 10},                    MPKIND__OVERLAP,     0, 3,  s1x16,  s1x8,      0x000, 0, MAPCODE(s1x8,3,MPKIND__OVERLAP,     0)},
   {{0, 1, 2, 3, 8, 9, 10, 11,         2, 3, 4, 5, 6, 7, 8, 9},               MPKIND__OVERLAP,     0, 2,  s2x6,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__OVERLAP,     0)},
   {{10, 11, 5, 4, 6, 7, 8, 9,         0, 1, 2, 3, 4, 5, 11, 10},             MPKIND__OVERLAP,     0, 2,  s3x4,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__OVERLAP,     1)},
   {{1, 11, 8, 9, 10, 0,    2, 5, 7, 8, 11, 1,     3, 4, 6, 7, 5, 2},         MPKIND__OVERLAP,     0, 3,  s3x4,   s2x3,      0x015, 0, MAPCODE(s2x3,3,MPKIND__OVERLAP,     1)},

   /* Overlapped 2x2's. */
   {{0, 1, 4, 5,                       1, 2, 3, 4},                           MPKIND__OVERLAP,     0, 2,  s2x3,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__OVERLAP,     0)},
   {{1, 4, 5, 0,                       2, 3, 4, 1},                           MPKIND__OVERLAP,     0, 2,  s2x3,   s2x2,      0x005, 0, MAPCODE(s2x2,2,MPKIND__OVERLAP,     1)},
   {{0, 1, 6, 7,            1, 2, 5, 6,            2, 3, 4, 5},               MPKIND__OVERLAP,     0, 3,  s2x4,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__OVERLAP,     0)},
   {{0, 1, 8, 9,       1, 2, 7, 8,       2, 3, 6, 7,      3, 4, 5, 6},        MPKIND__OVERLAP,     0, 4,  s2x5,   s2x2,      0x000, 0, MAPCODE(s2x2,4,MPKIND__OVERLAP,     0)},
   {{0, 1, 10, 11,  1, 2, 9, 10,  2, 3, 8, 9,  3, 4, 7, 8,  4, 5, 6, 7},      MPKIND__OVERLAP,     0, 5,  s2x6,   s2x2,      0x000, 0, MAPCODE(s2x2,5,MPKIND__OVERLAP,     0)},
   {{0, 1, 14, 15,  1, 2, 13, 14,  2, 3, 12, 13,  3, 4, 11, 12,  4, 5, 10, 11,
     5, 6, 9, 10,  6, 7, 8, 9},                                               MPKIND__OVERLAP,     0, 7,  s2x8,   s2x2,      0x000, 0, MAPCODE(s2x2,7,MPKIND__OVERLAP,     0)},

   {{0, 1, 15, 14, 10, 11, 12, 13,     1, 2, 6, 7, 9, 10, 14, 15,
                            2, 3, 4, 5, 8, 9, 7, 6},                          MPKIND__OVERLAP,     0, 3,  s4dmd,  s_qtag,    0x000, 0, MAPCODE(s_qtag,3,MPKIND__OVERLAP,   0)},
   {{12, 0, 13, 11, 15, 10, 14, 1,     14, 1, 15, 10, 6, 9, 7, 2,
                            7, 2, 6, 9, 4, 8, 5, 3},                          MPKIND__OVERLAP,     0, 3,  s4ptpd, s_ptpd,    0x000, 0, MAPCODE(s_ptpd,3,MPKIND__OVERLAP,   0)},

   /* Overlapped 1x4's. */
   {{0, 1, 5, 2,                     2, 5, 3, 4},                             MPKIND__OVERLAP,     0, 2,  s1x6,   s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__OVERLAP,     0)},
   {{7, 6, 4, 5,                     0, 1, 3, 2},                             MPKIND__OVERLAP,     3, 2,  s2x4,   s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__OVERLAP,     1)},
   {{0, 1, 2, 3,            3, 2, 7, 6,            6, 7, 4, 5},               MPKIND__OVERLAP,     0, 3,  s1x8,   s1x4,      0x000, 0, MAPCODE(s1x4,3,MPKIND__OVERLAP,     0)},
   {{9, 8, 6, 7,            10, 11, 4, 5,          0, 1, 3, 2},               MPKIND__OVERLAP,     3, 3,  s3x4,   s1x4,      0x000, 0, MAPCODE(s1x4,3,MPKIND__OVERLAP,     1)},
   {{0, 1, 3, 2,     2, 3, 9, 4,    4, 9, 7, 8,    8, 7, 5, 6},               MPKIND__OVERLAP,     0, 4,  s1x10,  s1x4,      0x000, 0, MAPCODE(s1x4,4,MPKIND__OVERLAP,     0)},
   {{8, 6, 4, 5,   9, 11, 2, 7,   10, 15, 1, 3,   12, 13, 0, 14},             MPKIND__OVERLAP,     3, 4,  s4x4,   s1x4,      0x000, 0, MAPCODE(s1x4,4,MPKIND__OVERLAP,     1)},
   {{0, 1, 3, 2,  2, 3, 5, 4,  4, 5, 10, 11,  11, 10, 8, 9,  9, 8, 6, 7},     MPKIND__OVERLAP,     0, 5,  s1x12,  s1x4,      0x000, 0, MAPCODE(s1x4,5,MPKIND__OVERLAP,     0)},
   {{0, 9, 14, 15, 1, 8, 13, 16, 2, 7, 12, 17, 3, 6, 11, 18, 4, 5, 10, 19},   MPKIND__OVERLAP,     3, 5,  s4x5,   s1x4,      0x155, 0, MAPCODE(s1x4,5,MPKIND__OVERLAP,     1)},


   {{7, 0, -1, 6,           -1, 1, -1, 5,          -1, 2, 3, 4},              MPKIND__OVERLAP,     0, 3,s_spindle,sdmd,      0x000, 0, MAPCODE(sdmd,3,MPKIND__OVERLAP,     0)},
   {{0, 1, 2, 3, 12, 13, 14, 15,       2, 3, 4, 5, 10, 11, 12, 13,
                             4, 5, 6, 7, 8, 9, 10, 11},                       MPKIND__OVERLAP,     0, 3,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,3,MPKIND__OVERLAP,     0)},
   {{9, 11, 7, 2, 4, 5, 6, 8,          10, 15, 3, 1, 2, 7, 11, 9,
                               12, 13, 14, 0, 1, 3, 15, 10},                  MPKIND__OVERLAP,     0, 3,  s4x4,   s2x4,      0x000, 1, MAPCODE(s2x4,3,MPKIND__OVERLAP,     1)},
   {{1, 8, 16, 13, 14, 15, 9, 0,       2, 7, 17, 12, 13, 16, 8, 1,
        3, 6, 18, 11, 12, 17, 7, 2,      4, 5, 19, 10, 11, 18, 6, 3},         MPKIND__OVERLAP,     0, 4,  s4x5,   s2x4,      0x055, 0, MAPCODE(s2x4,4,MPKIND__OVERLAP,     1)},
   {{1, 10, 19, 16, 17, 18, 11, 0,       2, 9, 20, 15, 16, 19, 10, 1,   3, 8, 21, 14, 15, 20, 9, 2,
        4, 7, 22, 13, 14, 21, 8, 3,      5, 6, 23, 12, 13, 22, 7, 4},         MPKIND__OVERLAP,     0, 5,  s4x6,   s2x4,      0x155, 0, MAPCODE(s2x4,5,MPKIND__OVERLAP,     1)},

   {{0, 3,                             1, 2},                                 MPKIND__REMOVED,     0, 2,  s1x4,   s1x2,      0x000, 0, MAPCODE(s1x2,2,MPKIND__REMOVED,     0)},
   {{0, 5,                  1, 4,                  2, 3},                     MPKIND__TWICE_REMOVED,0,3,  s1x6,   s1x2,      0x000, 0, MAPCODE(s1x2,3,MPKIND__TWICE_REMOVED,0)},
   {{0, 6,              1, 7,              3, 5,            2, 4},            MPKIND__THRICE_REMOVED,0,4, s1x8,   s1x2,      0x000, 0, MAPCODE(s1x2,4,MPKIND__THRICE_REMOVED,0)},

   {{0, 3,                             1, 2},                                 MPKIND__REMOVED,     0, 2,  s2x2,   s1x2,      0x005, 0, MAPCODE(s1x2,2,MPKIND__REMOVED,     1)},
   {{0, 5,                  1, 4,                  2, 3},                     MPKIND__TWICE_REMOVED,0,3,  s2x3,   s1x2,      0x015, 0, MAPCODE(s1x2,3,MPKIND__TWICE_REMOVED,1)},
   {{0, 7,              1, 6,              2, 5,            3, 4},            MPKIND__THRICE_REMOVED,0,4, s2x4,   s1x2,      0x055, 0, MAPCODE(s1x2,4,MPKIND__THRICE_REMOVED,1)},

   {{0, 2, 5, 7,                       1, 3, 4, 6},                           MPKIND__REMOVED,     0, 2,  s2x4,   s2x2,      0x000, 0, MAPCODE(s2x2,2,MPKIND__REMOVED,     0)},
   {{0, 3, 8, 11,           1, 4, 7, 10,           2, 5, 6, 9},               MPKIND__TWICE_REMOVED,0,3,  s2x6,   s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__TWICE_REMOVED,0)},
   {{0, 4, 11, 15,      1, 5, 10, 14,      2, 6, 9, 13,     3, 7, 8, 12},     MPKIND__THRICE_REMOVED,0,4, s2x8,   s2x2,      0x000, 0, MAPCODE(s2x2,4,MPKIND__THRICE_REMOVED,0)},

   {{0, 2, 4,                          1, 5, 3},                              MPKIND__REMOVED,     0, 2,  s1x6,   s1x3,      0x000, 0, MAPCODE(s1x3,2,MPKIND__REMOVED,0)},
   {{5, 4, 3,                          0, 1, 2},                              MPKIND__REMOVED,     0, 2,  s2x3,   s1x3,      0x000, 1, MAPCODE(s1x3,2,MPKIND__REMOVED,1)},
   {{0, 2, 4, 7, 9, 11,                1, 3, 5, 6, 8, 10},                    MPKIND__REMOVED,     0, 2,  s2x6,   s2x3,      0x000, 0, MAPCODE(s2x3,2,MPKIND__REMOVED,0)},

   {{0, 3, 5, 6,                       1, 2, 4, 7},                           MPKIND__REMOVED,     0, 2,  s1x8,   s1x4,      0x000, 0, MAPCODE(s1x4,2,MPKIND__REMOVED,0)},
   {{0, 3, 8, 11,           1, 4, 7, 10,           2, 5, 6, 9},               MPKIND__TWICE_REMOVED,0,3,  s1x12,  s1x4,      0x000, 0, MAPCODE(s1x4,3,MPKIND__TWICE_REMOVED,0)},
   {{0, 4, 11, 15,      1, 5, 10, 14,      2, 6, 9, 13,     3, 7, 8, 12},     MPKIND__THRICE_REMOVED,0,4, s1x16,  s1x4,      0x000, 0, MAPCODE(s1x4,4,MPKIND__THRICE_REMOVED,0)},

   {{7, 6, 4, 5,                       0, 1, 3, 2},                           MPKIND__REMOVED,     0, 2,  s2x4,   s1x4,      0x000, 1, MAPCODE(s1x4,2,MPKIND__REMOVED,1)},

   {{0, 2, 6, 4, 9, 11, 15, 13,        1, 3, 7, 5, 8, 10, 14, 12},            MPKIND__REMOVED,     0, 2,  s1x16,  s1x8,      0x000, 0, MAPCODE(s1x8,2,MPKIND__REMOVED,0)},

   {{15, 14, 12, 13, 8, 9, 11, 10,     0, 1, 3, 2, 7, 6, 4, 5},               MPKIND__REMOVED,     0, 2,  s2x8,   s1x8,      0x000, 1, MAPCODE(s1x8,2,MPKIND__REMOVED,1)},
   {{19, 18, 16, 17, 12, 13, 15, 14,   20, 21, 23, 22, 8, 9, 11, 10,
              0, 1, 3, 2, 7, 6, 4, 5},                                        MPKIND__TWICE_REMOVED,0,3,  s3x8,   s1x8,      0x000, 1, MAPCODE(s1x8,3,MPKIND__TWICE_REMOVED,1)},


   {{6, 0, 3, 5,                       7, 1, 2, 4},                           MPKIND__REMOVED,     0, 2,  s_rigger, sdmd,    0x000, 0, MAPCODE(sdmd,2,MPKIND__REMOVED,     0)},
   {{0, 3, 5, 6,                       1, 2, 4, 7},                           MPKIND__REMOVED,     0, 2,  s_qtag, sdmd,      0x005, 0, MAPCODE(sdmd,2,MPKIND__REMOVED,     1)},
   {{0, 2, 4, 7, 9, 11,                1, 3, 5, 6, 8, 10},                    MPKIND__REMOVED,     0, 2,  s1x12,  s1x6,      0x000, 0, MAPCODE(s1x6,2,MPKIND__REMOVED,     0)},
   {{11, 10, 9, 6, 7, 8,               0, 1, 2, 5, 4, 3},                     MPKIND__REMOVED,     0, 2,  s2x6,   s1x6,      0x000, 1, MAPCODE(s1x6,2,MPKIND__REMOVED,     1)},

   {{0, 2, 4, 6, 9, 11, 13, 15,        1, 3, 5, 7, 8, 10, 12, 14},            MPKIND__REMOVED,     0, 2,  s2x8,   s2x4,      0x000, 0, MAPCODE(s2x4,2,MPKIND__REMOVED,0)},
   {{0, 3, 6, 9, 14, 17, 20, 23,      1, 4, 7, 10, 13, 16, 19, 22,
              2, 5, 8, 11, 12, 15, 18, 21},                                   MPKIND__TWICE_REMOVED,0,3,  s2x12,  s2x4,      0x000, 0, MAPCODE(s2x4,3,MPKIND__TWICE_REMOVED,0)},
   {{10, 15, 3, 1, 4, 5, 6, 8,         12, 13, 14, 0, 2, 7, 11, 9},           MPKIND__REMOVED,     0, 2,  s4x4,   s2x4,      0x000, 1, MAPCODE(s2x4,2,MPKIND__REMOVED,1)},
   {{3, 8, 21, 14, 17, 18, 11, 0,     4, 7, 22, 13, 16, 19, 10, 1,
              5, 6, 23, 12, 15, 20, 9,  2},                                   MPKIND__TWICE_REMOVED,0,3,  s4x6,   s2x4,      0x015, 1, MAPCODE(s2x4,3,MPKIND__TWICE_REMOVED,1)},

   {{4, 7, 22, 13, 15, 20, 17, 18, 11, 0, 2, 9,
     5, 6, 23, 12, 14, 21, 16, 19, 10, 1, 3, 8},                              MPKIND__REMOVED,     0, 2,  s4x6,   s3x4,      0x005, 0, MAPCODE(s3x4,2,MPKIND__REMOVED,1)},
   {{7, 22, 15, 20, 18, 11, 2, 9,
     6, 23, 14, 21, 19, 10, 3, 8},                                            MPKIND__REMOVED,     0, 2,  s4x6, s_qtag,      0x005, 0, MAPCODE(s_qtag,2,MPKIND__REMOVED,1)},

   {{1, 7, 5, 3,                 (veryshort) s1x2, 0, 6, 2, 4},               MPKIND__SPEC_TWICEREM,0,3,  s_ptpd, s2x2,      0x000, 0, MAPCODE(s2x2,3,MPKIND__SPEC_TWICEREM,0)},
   {{6, 7, 2, 3,                 (veryshort) s1x2, 5, 4, 0, 1},               MPKIND__SPEC_TWICEREM,0,3,  s_qtag, s1x4,      0x000, 1, MAPCODE(s1x4,3,MPKIND__SPEC_TWICEREM,1)},

   {{2, 7, 5, 0,                       6, 3, 1, 4},                           MPKIND__REMOVED,     0, 2,  s_bone, s_trngl4,  0x207, 0, MAPCODE(s_trngl4,2,MPKIND__REMOVED, 1)},
   {{2, 4, 0,                          5, 1, 3},                              MPKIND__REMOVED,     0, 2,  s_bone6, s_trngl,  0x207, 0, MAPCODE(s_trngl,2,MPKIND__REMOVED, 1)},

   {{0, 1,     2, 3,        4, 9,      8, 7,       6, 5},                     MPKIND__SPLIT,       0, 5,  s1x10,  s1x2,      0x000, 0, MAPCODE(s1x2,5,MPKIND__SPLIT,       0)},
   {{0, 1,     2, 3,        4, 5,      11, 10,     9, 8,     7, 6},           MPKIND__SPLIT,       0, 6,  s1x12,  s1x2,      0x000, 0, MAPCODE(s1x2,6,MPKIND__SPLIT,       0)},
   {{0, 9,     1, 8,        2, 7,      3, 6,       4, 5},                     MPKIND__SPLIT,       0, 5,  s2x5,   s1x2,      0x155, 0, MAPCODE(s1x2,5,MPKIND__SPLIT,       1)},
   {{0, 11,    1, 10,       2, 9,      3, 8,       4, 7,     5, 6},           MPKIND__SPLIT,       0, 6,  s2x6,   s1x2,      0x555, 0, MAPCODE(s1x2,6,MPKIND__SPLIT,       1)},
   {{0, 9, 14, 15,          1, 8, 13, 16,          2, 7, 12, 17,
                            3, 6, 11, 18,          4, 5, 10, 19},             MPKIND__SPLIT,       0, 5,  s4x5,   s1x4,      0x155, 0, MAPCODE(s1x4,5,MPKIND__SPLIT,       1)},

   {{0}, MPKIND__NONE, 0, 0,  nothing}};



/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h. */
map_thing *split_lists[][6] = {
   {0,             0,             0,             0,             0,             0},          /* nothing */
   {&map_1x2_1x1,  &map_1x2_1x1,  &map_1x3_1x1,  &map_1x3_1x1,  0,             0},          /* s1x1 */
   {&map_1x4_1x2,  &map_2x2h,     &map_1x6_1x2,  &map_2x3_1x2,  0,             0},          /* s1x2 */
   {&map_1x6_1x3,  &map_2x3_1x3,  0,             0,             &map_1x12_1x3, &map_3x4_1x3}, /* s1x3 */
   {&map_2x4_2x2,  &map_2x4_2x2r, &map_2x6_2x2,  &map_2x6_2x2r, &map_2x8_2x2,  0},          /* s2x2 */
   {&map_1x8_1x4,  &map_2x4_1x4,  &map_1x12_1x4, &map_3x4_1x4,  &map_1x16_1x4, &map_4x4_1x4},  /* s1x4 */
   {&map_ptp_dmd,  &map_qtg_dmd,  &map_3ptp_dmd, &map_3dmd_dmd, &map_4ptp_dmd, &map_4dmd_dmd},  /* sdmd */
   {&map_2stars,   0,             0,             0,             0,             0},          /* s_star */
   {0,             &map_s6_trngl, 0,             0,             0,             0},          /* s_trngl */
   {&map_phan_trngl4b, &map_rig_trngl4, 0,       0,             0,             0},          /* s_trngl4 */
   {&map_bone_rig, 0,             0,             0,             0,             0},          /* s_bone6 */
   {&map_short6_2, 0,             0,             0,             0,             0},          /* s_short6 */
   {&map_1x12_1x6, &map_2x6_1x6,  0,             &map_3x6_1x6,  0,             &map_4x6_1x6}, /* s1x6 */
   {&map_2x6_2x3,  &map_3x4_2x3,  0,             &map_3x6_2x3,  0,             0},          /* s2x3 */
   {&map_bone_12d, &map_bigd_12d, 0,             0,             0,             0},          /* s_1x2dmd */
   {0,             0,             0,             0,             0,             0},          /* s_2x1dmd */
   {&map_hv_qtg_2, &map_vv_qtg_2, 0,             0,             0,             0},          /* s_qtag */
   {0,             0,             0,             0,             0,             0},          /* s_bone */
   {&map_1x16_1x8, &map_2x8_1x8,  0,             &map_3x8_1x8,  0,             0},          /* s1x8 */
   {0,             0,             0,             0,             0,             0},          /* slittlestars */
   {0,             0,             0,             0,             0,             0},          /* s_2stars */
   {0,             0,             0,             0,             0,             0},          /* s1x3dmd */
   {0,             0,             0,             0,             0,             0},          /* s3x1dmd */
   {0,             &map_spin_3x4, 0,             0,             0,             0},          /* s_spindle */
   {&map_hrgl_ptp, 0,             0,             0,             0,             0},          /* s_hrglass */
   {0,             0,             0,             0,             0,             0},          /* s_dhrglass */
   {0,             0,             0,             0,             0,             0},          /* s_hyperglass */
   {&map_xwv_bone, 0,             0,             0,             0,             0},          /* s_crosswave */
   {&map_hv_2x4_2, &map_split_f,  &map_2x12_2x4, &map_4x6_2x4,  0,             0},          /* s2x4 */
   {0,             0,             0,             0,             0,             0},          /* s2x5 */
   {0,             0,             0,             0,             0,             0},          /* sd2x5 */
   {0,             0,             0,             0,             0,             0},          /* swqtag */
   {0,             0,             0,             0,             0,             0},          /* sdeep2x1dmd */
   {0,             0,             0,             0,             0,             0},          /* swhrglass */
   {0,             &map_par_rig,  0,             0,             0,             0},          /* s_rigger */
   {&map_3x8_3x4,  &map_4x6_3x4,  0,             0,             0,             0},          /* s3x4 */
   {&map_2x12_2x6, &map_4x6_2x6,  0,             0,             0,             0}};         /* s2x6 */
