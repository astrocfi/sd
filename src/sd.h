// SD -- square dance caller's helper.
//
//    Copyright (C) 1990-2006  William B. Ackerman.
//
//    This file is part of "Sd".
//
//    Sd is free software; you can redistribute it and/or modify it
//    under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    Sd is distributed in the hope that it will be useful, but WITHOUT
//    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
//    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
//    License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Sd; if not, write to the Free Software Foundation, Inc.,
//    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//    This is for version 36.

#include <stdio.h>

// Figure out how to do dll linkage.  If the source file that includes this
// had "SDLIB_EXPORTS" set (which it will if it's a file for sdlib.dll),
// make symbols "export" type.  Otherwise, make them "import" type.
// Unless this isn't for the WIN32 API at all, in which case make the
// "SDLIB_API" symbol do nothing.

#if defined(WIN32)
#if defined(SDLIB_EXPORTS)
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API __declspec(dllimport)
#endif
#else
#define SDLIB_API
#endif


// It seems that proper handling of throw clauses is just too hard to
// implement.  GCC does it, and presumably does an excellent job of
// diagnosing programs that violate throw-correctness.  However, the
// program runs something like 13 times slower.  This is unacceptable.
// So we turn it off for production builds, and use throws in the dumb
// setjmp-like way that everyone else does.  Too bad.  They were a nice
// idea.  Microsoft doesn't even try, and raises a warning (How thoughtful!
// See flaming below.) if we use them.

#define USE_THROW

#if defined USE_THROW

#if defined(_MSC_VER)
// Microsoft can't be bothered to support C++ exception declarations
// properly, but this pragma at least makes the compiler not complain.
// Geez!  If I wanted to use compilers that whine and wail
// piteously about the fact that I'm actually using the language, I'd
// use the HP-UX compilers!  They did a wonderful job of pointing out,
// in warning messages, that function prototypes are an ANSI C feature.
// As though this dangerous fact (that I was using ANSI C, of all things)
// were something that I didn't know and needed to to warned about.
// The HP-UX compilers came into the 20th century in the nick of time.
// It's now the 21st.
#pragma warning(disable: 4290)
#endif

#define THROW_DECL throw(error_flag_type)
#else
#define THROW_DECL
#endif


#include "database.h"

/* We customize the necessary declarations for functions
   that don't return.  Alas, this requires something in front
   and something in back. */

#if defined(__GNUC__)
#define NORETURN1
#define NORETURN2 __attribute__ ((noreturn))
#elif defined(WIN32)
// This declspec only works for VC++ version 6.
#define NORETURN1 /*__declspec(noreturn)*/
#define NORETURN2
#else
#define NORETURN1
#define NORETURN2
#endif

// We used to do some stuff to cater to compiler vendors (e.g. Sun
// Microsystems) that couldn't be bothered to do the "const" attribute
// correctly.
//
// So we used to have a line that said "#define Const const".  But not
// any longer.  If your compiler doesn't handle "const" correctly (or
// any other aspect of ANSI C++, for that matter), that's too bad.
//
// We no longer take pity on broken compilers.


// We would like "veryshort" to be a signed char, but not all
// compilers are fully ANSI compliant.  The IBM AIX compiler, for
// example, considers char to be unsigned.  The switch
// "NO_SIGNED_CHAR" alerts us to that fact.  The configure script has
// checked this for us.
//
// Baloney!  We no longer use a configure script.  We are aware of the
// fact that there is a whole "industry" (config/autoconf/etc.)
// dedicated to the job of figuring out what hideous brokenness any
// given Unix implementation is inflicting on us today, but we have no
// patience for that kind of garbage.  If your compiler or OS can't
// handle this, tough.
//
// We no longer take pity on broken compilers or operating systems.

#ifdef NO_SIGNED_CHAR
typedef short veryshort;
#else
typedef char veryshort;
#endif

/* We would like to think that we will always be able to count on compilers to do the
   right thing with "int" and "long int" and so on.  What we would really like is
   for compilers to be counted on to make "int" at least 32 bits, because we need
   32 bits in many places.  However, some compilers don't, so we have to use
   "long int" or "unsigned long int".  We think that all compilers we deal with
   will do the right thing with that, but, just in case, we use a typedef.

   The type "uint32" must be an unsigned integer of at least 32 bits.
   The type "uint16" must be an unsigned integer of at least 16 bits.

   Note also:  There are many places in the program (not just in database.h and sd.h)
   where the suffix "UL" is put on constants that are intended to be of type "uint32".
   If "uint32" is changed to anything other than "unsigned long int", it may be
   necessary to change all of those. */

typedef unsigned long int uint32;
typedef unsigned short int uint16;
typedef unsigned char uint8;
typedef const char *Cstring;

enum { MAX_PEOPLE = 24 };


enum error_flag_type {
   error_flag_none = 0,      // Must be zero because setjmp returns this.
                             // (Of course, we haven't used setjmp since March, 2000.)
   error_flag_1_line,        // 1-line error message, text is in error_message1.
   error_flag_2_line,        // 2-line error message, text is in error_message1 and
                             // error_message2.
   error_flag_collision,     // collision error, message is that people collided, they are in
                             // collision_person1 and collision_person2.
   error_flag_cant_execute,  // unable-to-execute error, person is in collision_person1,
                             // text is in error_message1.

   // Errors after this can't be restarted by the mechanism that goes on to the
   // call's next definition when a call execution fails.
   // "Error_flag_no_retry" is the indicator for this.

   error_flag_no_retry,      // Like error_flag_1_line, but it is instantly fatal.

   error_flag_user_wants_to_resolve, // User typed "pick random call" while querying
                                     // for a subcall -- do a resolve.

   // Errors after this did not arise from call execution, so we don't
   // show the ending formation.  "Error_flag_wrong_command" is the indicator for this.

   error_flag_wrong_command,     // clicked on something inappropriate in subcall reader.
   error_flag_wrong_resolve_command, // "resolve" or similar command was called
                                     // in inappropriate context, text is in error_message1.
   error_flag_show_stats,        // wants to display stale call statistics.
   error_flag_selector_changed,  // warn that selector was changed during clipboard paste.
   error_flag_formation_changed  // warn that formation changed during clipboard paste.
};



// In several places in this program, we use C-style static aggregate
// initializers.  Specifically, sdtables.cpp and sdctable.cpp are full of them.
// Some of them are absolutely enormous.  The C++ language sort of frowns on
// that style of initialization.  The C++ way would be to use constructors,
// writing the appropriate code to invoke the constructors in the proper way.
// While we have some sympathy with this style in many cases, we prefer the
// C-style direct initializers for most of the big tables.  The reason is that
// the benefits of C++ constructors seem infinitesimal for these tables, and
// they would sacrifice transparency: with the direct initializers, you can
// look at the struct definition in sd.h and look at the initializer in
// sdtables.cpp and know for sure, without worrying about what clever things
// the constructor might do, which constant belongs in which field.
//
// Because we are initializing things in ways that C++ frowns on, the
// C++ compiler punishes us by not letting us declare fields "const".
// This is unfortunate.  We attach great importance to the ability of
// the C/C++ type mechanism to prevent table overwrites.  (In fact, a
// test compilation of a competing program with "const" declarators on
// these tables revealed an astonishing bug.)
//
// For a while, we solved this problem by putting the initializers in
// plain C files (sdtables.c and sdctable.c) and linking with the other
// C++ files.  Not surprisingly, this created difficulties.  In the end,
// it became unworkable, and now all files are C++.
//
// By the way, one of the problems was that the Intel compiler saw what
// we were doing in the C++ files, realized that we could not possibly
// initialize the structures legally, and gave a warning.  What it
// overlooked was that we were linking with C files to do what would
// have been impossible in C++.  For reference, the incantation to
// shut off the warning was:
//     #if defined(WIN32) && defined(__ICL)
//     #pragma warning(disable: 411)
//     #endif
//
// Because what we really want is the protection against overwriting
// that the "const" declaration would provide, we define the initialized
// tables, and the definition of their structure, in classes.  The tables
// themselves are private.  So that is why you see all these classes
// that have no members, but have statically initialized structs.


enum color_scheme_type {
   color_by_gender,
   no_color,
   color_by_couple,
   color_by_corner,
   color_by_couple_rgyb,
   color_by_couple_ygrb
};

class ui_option_type {
 public:
   color_scheme_type color_scheme;
   int force_session;
   int sequence_num_override;
   int no_graphics;       // 1 = "no_checkers"; 2 = "no_graphics"
   bool no_intensify;
   bool reverse_video;    // T = white-on-black; F = black-on-white.
   bool pastel_color;     // T = use pastel red/blue for color by gender.
                          // F = bold colors.  Color by couple or color by corner
                          // are always done with bold colors.
   bool use_magenta;      // These two override the above on a case-by-case basis.
   bool use_cyan;
   bool singlespace_mode;
   bool nowarn_mode;
   bool keep_all_pictures;
   bool accept_single_click;
   bool diagnostic_mode;
   bool no_sound;
   bool tab_changes_focus;

   // This is the line length beyond which we will take pity on
   // whatever device has to print the result, and break the text line.
   // It is presumably smaller than our internal text capacity.
   // It is an observed fact that, with the default font (14 point Courier),
   // one can print 66 characters on 8.5 x 11 inch paper.
   // This is 59 by default.
   int max_print_length;

   // This is for the hidden command-line switch "resolve_test <N>".  Any
   // nonzero argument will seed the random number generator with that value,
   // thereby making all search operations deterministic.  (The random number
   // generator is normally seeded with the clock, of course.)
   //
   // Also, if the number is positive, it makes all search operations fail, and
   // sets the timeout to that many minutes.  This can be used for testing for
   // crashes in the resolve searcher.  Give an argument of 60, for example, and
   // any search command ("resolve", "pick random call", etc.) will generate
   // random solutions for an hour, rejecting them all.  Doing this on multiple
   // processors, with slightly different arguments, will run separate
   // deterministic tests on each processor.
   int resolve_test_minutes;

   int singing_call_mode;

   // This gets set if a user interface (e.g. sdui-tty/sdui-win) wants escape sequences
   // for drawing people, so that it can fill in funny characters, or draw in color.
   // This applies only to calls to add_new_line with a nonzero second argument.
   //
   // 0 means don't use any funny stuff.  The text strings transmitted when drawing
   // setups are completely plain ASCII.
   //
   // 1 means use escapes for the people themselves (13 octal followed by a byte of
   // person identifier followed by a byte of direction) but don't use the special
   // spacing characters.  All spacing and formatting is done with spaces.
   //
   // 2 means use escapes and other special characters.  Whenever the second arg to
   // add_new_line is nonzero, then in addition to the escape sequences for the
   // people themselves, we have an escape sequence for a phantom, and certain
   // characters have special meaning:  5 means space 1/2 of a glyph width, etc.
   // See the definition of newline for details.
   int use_escapes_for_drawing_people;

   // These could get changed if the user requests special naming.  See "alternate_glyphs_1"
   // in the command-line switch parser in sdsi.cpp.
   const char *pn1;       // 1st char (1/2/3/4) of what we use to print person.
   const char *pn2;       // 2nd char (B/G) of what we use to print person.
   const char *direc;     // 3rd char (direction arrow) of what we use to print person.
   const char *stddirec;  // the "standard" directions, for transcript files.
                          // Doesn't get overridden by any options.
   int squeeze_this_newline;  // randomly used by printing stuff.
   int drawing_picture;       // randomly used by printing stuff.

   ui_option_type();      // Constructor is in sdmain.
};


// BEWARE!!  This list must track the array "concept_table" in sdconcpt.cpp .
enum concept_kind {

   // These next few are not concepts.  Their appearance marks the end of a parse tree.

   concept_another_call_next_mod,         // calla modified by callb
   concept_mod_declined,                  // user was queried about modification, and said no.
   marker_end_of_list,                    // normal case

   // This is not a concept.  Its appearance indicates a comment is to be placed here.

   concept_comment,

   // Everything after this is a real concept.

   concept_concentric,
   concept_tandem,
   concept_some_are_tandem,
   concept_frac_tandem,
   concept_some_are_frac_tandem,
   concept_gruesome_tandem,
   concept_gruesome_frac_tandem,
   concept_tandem_in_setup,
   concept_checkerboard,
   concept_sel_checkerboard,
   concept_anchor,
   concept_reverse,
   concept_left,
   concept_grand,
   concept_magic,
   concept_cross,
   concept_single,
   concept_singlefile,
   concept_interlocked,
   concept_yoyo,
   concept_fractal,
   concept_fast,
   concept_straight,
   concept_twisted,
   concept_rewind,
   concept_12_matrix,
   concept_16_matrix,
   concept_revert,
   concept_1x2,
   concept_2x1,
   concept_2x2,
   concept_1x3,
   concept_3x1,
   concept_3x3,
   concept_4x4,
   concept_5x5,
   concept_6x6,
   concept_7x7,
   concept_8x8,
   concept_create_matrix,
   concept_funny,
   concept_randomtrngl,
   concept_selbasedtrngl,
   concept_split,
   concept_each_1x4,
   concept_diamond,
   concept_triangle,
   concept_leadtriangle,
   concept_do_both_boxes,
   concept_once_removed,
   concept_do_phantom_2x2,
   concept_do_phantom_boxes,
   concept_do_phantom_diamonds,
   concept_do_phantom_2x4,
   concept_do_phantom_stag_qtg,
   concept_do_phantom_diag_qtg,
   concept_do_divided_diamonds,
   concept_do_divided_bones,
   concept_distorted,
   concept_single_diagonal,
   concept_double_diagonal,
   concept_parallelogram,
   concept_multiple_lines,
   concept_multiple_lines_tog,
   concept_multiple_lines_tog_std,
   concept_triple_1x8_tog,
   concept_multiple_boxes,
   concept_quad_boxes_together,
   concept_triple_boxes_together,
   concept_multiple_diamonds,
   concept_multiple_formations,
   concept_triple_diamonds_together,
   concept_quad_diamonds_together,
   concept_triangular_boxes,
   concept_in_out_std,
   concept_in_out_nostd,
   concept_triple_diag,
   concept_triple_diag_together,
   concept_triple_twin,
   concept_triple_twin_nomystic,
   concept_misc_distort,
   concept_misc_distort_matrix,
   concept_old_stretch,
   concept_new_stretch,
   concept_assume_waves,
   concept_active_phantoms,
   concept_mirror,
   concept_central,
   concept_snag_mystic,
   concept_crazy,
   concept_frac_crazy,
   concept_dbl_frac_crazy,
   concept_phan_crazy,
   concept_frac_phan_crazy,
   concept_fan,
   concept_c1_phantom,
   concept_grand_working,
   concept_centers_or_ends,
   concept_so_and_so_only,
   concept_some_vs_others,
   concept_same_sex_disconnected,
   concept_stable,
   concept_so_and_so_stable,
   concept_frac_stable,
   concept_so_and_so_frac_stable,
   concept_emulate,
   concept_standard,
   concept_matrix,
   concept_double_offset,
   concept_checkpoint,
   concept_on_your_own,
   concept_trace,
   concept_move_in_and,
   concept_outeracting,
   concept_ferris,
   concept_overlapped_diamond,
   concept_all_8,
   concept_centers_and_ends,
   concept_mini_but_o,
   concept_n_times_const,
   concept_n_times,
   concept_sequential,
   concept_special_sequential,
   concept_special_sequential_num,
   concept_special_sequential_4num,
   concept_meta,
   concept_meta_one_arg,
   concept_meta_two_args,
   concept_so_and_so_begin,
   concept_replace_nth_part,
   concept_replace_last_part,
   concept_interrupt_at_fraction,
   concept_sandwich,
   concept_interlace,
   concept_fractional,
   concept_rigger,
   concept_wing,
   concept_common_spot,
   concept_drag,
   concept_dblbent,
   concept_omit,
   concept_supercall,
   concept_diagnose
};

// These enumerate the "useful" concepts -- concepts that we will automatically
// generate for the "normalize" command or when we see something like
// "switch to an interlocked diamond".  The order of these is not important,
// though they generally follow the order of the concepts in the main table.
//
// The concepts in the main table ("unsealed_concept_descriptor_table") have
// one of these in their "useful" field to register themselves as available
// for service as a "useful" concept.

enum useful_concept_enum {
  UC_none,
  UC_spl,
  UC_ipl,
  UC_pl,
  UC_pl8,
  UC_pl6,
  UC_tl,
  UC_tlwt,
  UC_tlwa,
  UC_tlwf,
  UC_tlwb,
  UC_trtl,
  UC_qlwt,
  UC_qlwa,
  UC_qlwf,
  UC_qlwb,
  UC_spw,
  UC_ipw,
  UC_pw,
  UC_spc,
  UC_ipc,
  UC_pc,
  UC_pc8,
  UC_pc6,
  UC_tc,
  UC_tcwt,
  UC_tcwa,
  UC_tcwr,
  UC_tcwl,
  UC_trtc,
  UC_qcwt,
  UC_qcwa,
  UC_qcwr,
  UC_qcwl,
  UC_spb,
  UC_ipb,
  UC_pb,
  UC_tb,
  UC_tbwt,
  UC_tbwa,
  UC_tbwf,
  UC_tbwb,
  UC_tbwr,
  UC_tbwl,
  UC_qbwt,
  UC_qbwa,
  UC_qbwf,
  UC_qbwb,
  UC_qbwr,
  UC_qbwl,
  UC_spd,
  UC_ipd,
  UC_pd,
  UC_spds,
  UC_ipds,
  UC_pds,
  UC_td,
  UC_tdwt,
  UC_qd,
  UC_qdwt,
  UC_sp1,
  UC_ip1,
  UC_p1,
  UC_sp3,
  UC_ip3,
  UC_p3,
  UC_spgt,
  UC_ipgt,
  UC_pgt,
  UC_cpl,
  UC_tnd,
  UC_cpl2s,
  UC_tnd2s,
  UC_pofl,
  UC_pofc,
  UC_pob,
  UC_magic,
  UC_pibl,
  UC_left,
  UC_cross,
  UC_grand,
  UC_intlk,
  UC_phan,
  UC_3x3,
  UC_4x4,
  UC_2x8matrix,
  UC_extent    // Not an item; indicates extent of the enum.
};

// Some versions of gcc don't like the class "concept", so we change the spelling.
class SDLIB_API conzept {

 public:

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "unsealed_concept_descriptor_table" be a statically initialized array.

   struct concept_descriptor {
      Cstring name;
      concept_kind kind;
      uint32 concparseflags;   // See above.
      dance_level level;
      useful_concept_enum useful;
      uint32 arg1;
      uint32 arg2;
      uint32 arg3;
      uint32 arg4;
      uint32 arg5;
      Cstring menu_name;
   };

   static concept_descriptor centers_concept;
   static concept_descriptor special_magic;
   static concept_descriptor special_interlocked;
   static concept_descriptor mark_end_of_list;
   static concept_descriptor marker_decline;
   static concept_descriptor marker_concept_mod;
   static concept_descriptor marker_concept_comment;
   static concept_descriptor marker_concept_supercall;

   // We want the concept list, as used by the main program, to be
   // constant.  But we can't literally make it constant, because
   // we need to translate the names during initialization, writing
   // over the "menu_name" field.  So the actual table that has the
   // static initializer in sdctable will *not* be constant.  But it
   // will be private.

 private:

   static concept_descriptor unsealed_concept_descriptor_table[];

 public:

   // This does the translation, writes over the "menu_name" fields,
   // and places a const pointer to the thing in "concept_descriptor_table".
   // This is in sdinit.
   static void conzept::translate_concept_names();
};

// BEWARE!!  This list must track the array "selector_list" in sdtables.cpp
enum selector_kind {
   selector_uninitialized,
   selector_boys,
   selector_girls,
   selector_heads,
   selector_sides,
   selector_headcorners,
   selector_sidecorners,
   selector_headboys,
   selector_headgirls,
   selector_sideboys,
   selector_sidegirls,
   selector_centers,
   selector_ends,
   selector_leads,
   selector_trailers,
   selector_lead_ends,
   selector_lead_ctrs,
   selector_trail_ends,
   selector_trail_ctrs,
   selector_end_boys,
   selector_end_girls,
   selector_center_boys,
   selector_center_girls,
   selector_beaus,
   selector_belles,
   selector_center2,
   selector_verycenters,
   selector_center6,
   selector_outer2,
   selector_veryends,
   selector_outer6,
   selector_ctrdmd,
   selector_ctr_1x4,
   selector_ctr_1x6,
   selector_outer1x3s,
   selector_center4,
   selector_center_wave,
   selector_center_line,
   selector_center_col,
   selector_center_box,
   selector_center_wave_of_6,
   selector_center_line_of_6,
   selector_center_col_of_6,
   selector_outerpairs,
   selector_firstone,
   selector_lastone,
   selector_firsttwo,
   selector_lasttwo,
   selector_firstthree,
   selector_lastthree,
   selector_leftmostone,
   selector_rightmostone,
   selector_leftmosttwo,
   selector_rightmosttwo,
   selector_leftmostthree,
   selector_rightmostthree,
#ifdef TGL_SELECTORS
   // Taken out.  Not convinced these are right.  See also sdutil.c, sdpreds.c .
   selector_wvbasetgl,
   selector_tndbasetgl,
   selector_insidetgl,
   selector_outsidetgl,
   selector_inpttgl,
   selector_outpttgl,
#endif
   selector_headliners,
   selector_sideliners,
   selector_thosefacing,
   selector_everyone,
   selector_all,
   selector_none,
   // Start of unsymmetrical selectors.
   unsymm_SELECTOR_START,    selector_nearline = unsymm_SELECTOR_START,
   selector_farline,
   selector_nearcolumn,
   selector_farcolumn,
   selector_nearbox,
   selector_farbox,
   selector_facingfront,
   selector_facingback,
   selector_boy1,
   selector_girl1,
   selector_cpl1,
   selector_boy2,
   selector_girl2,
   selector_cpl2,
   selector_boy3,
   selector_girl3,
   selector_cpl3,
   selector_boy4,
   selector_girl4,
   selector_cpl4,
   selector_cpls1_2,
   selector_cpls2_3,
   selector_cpls3_4,
   selector_cpls4_1,
   // Start of invisible selectors.
   selector_INVISIBLE_START,   selector_mysticbeaus = selector_INVISIBLE_START,
   selector_mysticbelles,
   selector_notctrdmd,
   selector_ENUM_EXTENT   // Not a selector; indicates extent of the enum.
};

// BEWARE!!  This list must track the array "direction_names" in sdutil.cpp .
// Note also that the "zig-zag" items will get disabled below A2.
// The key for this is "direction_zigzag".
enum direction_kind {
   direction_uninitialized,
   direction_no_direction,
   direction_left,
   direction_right,
   direction_in,
   direction_out,
   direction_back,
   direction_zigzag,
   direction_zagzig,
   direction_zigzig,
   direction_zagzag
};

// There are two different contexts in which we deal with collections of
// numbers.  In each case the numbers are packed into 6 bit fields, so they can
// theoretically go up to 63.  We could therefore handle up to 5 numbers, though
// we never do more than 4.
// Helpful hint:  when debugging, display the word in octal.
//
//  (1) The "number_fields" part of a call_conc_option_state.
//         This has the numbers (up to four of them) that the user entered
//         with a call or concept, as in "3/4 crazy" or "cast off 1/2".  Those
//         numbers are packed in right-to-left order.  So, for example, "circle
//         by 1/2 by 3/4" will have 0/0/3/2 in those fields (this call takes its
//         numeric arguments in quarters).  The "do the last 3/5" concept will
//         have 0/0/5/3 in those fields.  The parser will never put a number
//         larger than 36 (NUM_CARDINALS-1) in the "number_fields" word.
//
//  (2) The "fraction" field of a "fraction_command".  This also takes four

//         numbers, though there is no direct correspondence with the other
//         usage described above.  The four numbers are the numerators and
//         denominators of the starting and ending points of the desired part of
//         the call.  The layout is
//         <start den> / <start num> / <end den> / <end num>.  To do the whole
//         call, we use 1/0/1/1, which is the constant NUMBER_FIELDS_1_0_1_1 or
//         CMD_FRAC_NULL_VALUE.

enum {
   BITS_PER_NUMBER_FIELD = 6,
   NUMBER_FIELD_MASK = (1<<BITS_PER_NUMBER_FIELD)-1,
   NUMBER_FIELD_MASK_SECOND_ONE = NUMBER_FIELD_MASK << (BITS_PER_NUMBER_FIELD*2),
   NUMBER_FIELD_MASK_RIGHT_TWO = (1<<(BITS_PER_NUMBER_FIELD*2))-1,
   NUMBER_FIELD_MASK_LEFT_TWO = NUMBER_FIELD_MASK_RIGHT_TWO << (BITS_PER_NUMBER_FIELD*2),
   NUMBER_FIELDS_1_0 = 00100UL,          // A few useful canned values.
   NUMBER_FIELDS_2_1 = 00201UL,
   NUMBER_FIELDS_1_1 = 00101UL,
   NUMBER_FIELDS_0_0_1_1 = 000000101UL,
   NUMBER_FIELDS_1_0_0_0 = 001000000UL,
   NUMBER_FIELDS_1_0_1_1 = 001000101UL,
   NUMBER_FIELDS_1_0_2_1 = 001000201UL,
   NUMBER_FIELDS_1_0_4_0 = 001000400UL,
   NUMBER_FIELDS_2_1_1_1 = 002010101UL,
   NUMBER_FIELDS_2_1_2_1 = 002010201UL,
   NUMBER_FIELDS_4_0_1_1 = 004000101UL
};

/* BEWARE!!  There is a static initializer for this, "null_options", in sdtop.cpp
   that must be kept up to date. */
struct call_conc_option_state {
   selector_kind who;        /* selector, if any, used by concept or call */
   direction_kind where;     /* direction, if any, used by concept or call */
   uint32 tagger;            /* tagging call indices, if any, used by call.
                                If nonzero, this is 3 bits for the 0-based tagger class
                                and 5 bits for the 1-based tagger call */
   uint32 circcer;           /* circulating call index, if any, used by call */
   uint32 number_fields;     /* number, if any, used by concept or call */
   int howmanynumbers;       /* tells how many there are */
   int star_turn_option;     /* For calls with "@S" star turn stuff. */
};

// We need a forward reference; it's defined later in this file.
struct predptr_pair;

struct callarray {
   callarray *next;
   uint32 callarray_flags;
   call_restriction restriction;
   uint16 qualifierstuff;   /* See QUALBIT__??? definitions in database.h */
   uint8 start_setup;       /* Must cast to begin_kind! */
   uint8 end_setup;         /* Must cast to setup_kind! */
   uint8 end_setup_in;      /* Only if end_setup = concentric */  /* Must cast to setup_kind! */
   uint8 end_setup_out;     /* Only if end_setup = concentric */  /* Must cast to setup_kind! */
   union {
      // Dynamically allocated to whatever size is required.
      uint16 def[4];     /* only if pred = false */
      struct {                   /* only if pred = true */
         predptr_pair *predlist;
         // Dynamically allocated to whatever size is required.
         char errmsg[4];
      } prd;
   } stuff;
};

struct calldef_block {
   /* This has individual keys for groups of heritable modifiers.  Hence one
      can't say anything like "alternate_definition [3x3 4x4]".  But of course
      one can mix things from different groups. */
   uint32 modifier_seth;
   callarray *callarray_list;
   calldef_block *next;
   dance_level modifier_level;
};

struct by_def_item {
   short call_id;
   uint32 modifiers1;

   /* The "modifiersh" field of an invocation of a subcall tells what
      incoming modifiers will be passed to the subcall.  What the bits mean
      depends on whether the call's top-level "callflagsh" bit is on.

      If the bit is on in "callflagsh", the corresponding bit in "modifiersh"
      says to pass the modifier, if present, on to the subcall.  If this is
      a group (e.g. INHERITFLAG_MXNMASK), and it is on in "callflagsh", the
      entire field in "modifiersh" must be on or off.  It will be on if the
      subcall involcation is marked "inherit_mxn" or wahatever.

      If the bit is off in "callflagsh", the corresponding bit in "modifiersh"
      says to force the modifier.  In this case groups are treated as individual
      modifiers.  An individual switch like "force_3x3" causes that key to be
      placed in the field of "modifiersh". */

   uint32 modifiersh;
};

struct matrix_def_block {
   matrix_def_block *next;
   uint32 alternate_def_flags;
   dance_level modifier_level;
   uint32 items[2];     // Dynamically allocated to either 2 or 8.
};

struct calldefn {
   uint32 callflags1;    // The CFLAG1_??? flags.
   uint32 callflagsh;    // The mask for the heritable flags.
   // Within the "callflagsh" field, the various grouped fields
   // (e.g. INHERITFLAG_MXNMASK) are uniform -- either all the bits are
   // on or they are all off.  A call can only inherit the entire group,
   // by saying "mxn_is_inherited".
   uint32 callflagsf;    // The ESCAPE_WORD__???  and CFLAGH__??? flags.
   short int age;
   short int level;
   calldef_schema schema;
   calldefn *compound_part;
   union {
      struct {
         calldef_block *def_list;
      } arr;            // if schema = schema_by_array
      struct {
         uint32 matrix_flags;
         matrix_def_block *matrix_def_list;
      } matrix;         // if schema = schema_matrix or schema_partner_matrix
      struct {
         int howmanyparts;
         by_def_item *defarray;  // Dynamically allocated, there are "howmanyparts" of them.
      } seq;                     // if schema = schema_sequential or whatever
      struct {
         by_def_item innerdef;
         by_def_item outerdef;
      } conc;           // if schema = any of the concentric ones.
   } stuff;
};

struct call_with_name {
   calldefn the_defn;
   /* This is the "pretty" name -- "@" escapes have been changed to things like "<N>".
      If there are no escapes, this just points to the stuff below.
      If escapes are present, it points to allocated storage elsewhere.
      Either way, it persists throughout the program. */
   Cstring menu_name;
   /* Dynamically allocated to whatever size is required, will have trailing null.
      This is the name as it appeared in the database, with "@" escapes. */
   char name[4];
};

struct parse_block {
   const conzept::concept_descriptor *concept; // the concept or end marker
   call_with_name *call;          // if this is end mark, gives the call; otherwise unused
   call_with_name *call_to_print; // the original call, for printing (sometimes the field
                                  // above gets changed temporarily)
   parse_block *next;             // next concept, or, if this is end mark,
                                  // points to substitution list
   parse_block *subsidiary_root;  // for concepts that take a second call,
                                  // this is its parse root
   parse_block *gc_ptr;           // used for reclaiming dead blocks
   call_conc_option_state options;// number, selector, direction, etc.
   short replacement_key;         // this is the "DFM1_CALL_MOD_MASK" stuff
                                  // (shifted down) for a modification block
   bool no_check_call_level;      // if true, don't check whether this call
                                  // is at the level

   // We allow static instantiation of these things with just
   // the "concept" field filled in.
   parse_block(const conzept::concept_descriptor *ccc) : concept(ccc) {}
   // Which of course means we need to provide the default constructor too.
   parse_block() {}
};

// For ui_command_select:

// BEWARE!!  The resolve part of this next definition must be keyed
// to the array "title_string" in sdgetout.cpp, and maybe stuff in the UI.
// For example, see "command_menu" in sdmain.cpp.

// BEWARE!!  The order is slightly significant -- all search-type commands
// are >= command_resolve, and all "create some setup" commands
// are >= command_create_any_lines.  Certain tests are made easier by this.
enum command_kind {
   command_quit,
   command_undo,
   command_erase,
   command_abort,
   command_create_comment,
   command_change_outfile,
   command_change_title,
   command_getout,
   command_cut_to_clipboard,
   command_delete_entire_clipboard,
   command_delete_one_call_from_clipboard,
   command_paste_one_call,
   command_paste_all_calls,
   command_save_pic,
   command_help,
   command_help_manual,
   command_help_faq,
   command_simple_mods,
   command_all_mods,
   command_toggle_conc_levels,
   command_toggle_minigrand,
   command_toggle_act_phan,
   command_toggle_retain_after_error,
   command_toggle_nowarn_mode,
   command_toggle_keepallpic_mode,
   command_toggle_singleclick_mode,
   command_toggle_singer,
   command_toggle_singer_backward,
   command_select_print_font,
   command_print_current,
   command_print_any,
   command_refresh,
   command_resolve,            // Search commands start here.
   command_normalize,
   command_standardize,
   command_reconcile,
   command_random_call,
   command_simple_call,
   command_concept_call,
   command_level_call,
   command_8person_level_call,
   command_create_any_lines,   // Create setup commands start here.
   command_create_waves,
   command_create_2fl,
   command_create_li,
   command_create_lo,
   command_create_inv_lines,
   command_create_3and1_lines,
   command_create_any_col,
   command_create_col,
   command_create_magic_col,
   command_create_dpt,
   command_create_cdpt,
   command_create_tby,
   command_create_8ch,
   command_create_any_qtag,
   command_create_qtag,
   command_create_3qtag,
   command_create_qline,
   command_create_3qline,
   command_create_dmd,
   command_create_any_tidal,
   command_create_tidal_wave,
   command_kind_enum_extent    // Not a command kind; indicates extent of the enum.
};


// In each case, an integer or enum is deposited into the global variable
// uims_menu_index.  Its interpretation depends on which of the replies above
// was given.  For some of the replies, it gives the index into a menu.  For
// "ui_start_select" it is a start_select_kind.  For other replies, it is one of
// the following constants:

/* BEWARE!!  This list must track the array "startup_resources" in sdui-x11.c . */
/* BEWARE!!!!!!!!  Lots of implications for "centersp" and all that! */
/* BEWARE!!  If change this next definition, be sure to update the definition of
   "startinfolist" in sdtables.cpp, and also necessary stuff in the user interfaces.
   The latter includes the definition of "start_choices" in sd.dps
   in the Domain/Dialog system, and the corresponding CNTLs in *.rsrc
   in the Macintosh system.  You may also need changes in create_controls() in
   macstuff.c. */
enum start_select_kind {
   start_select_exit,        /* Don't start a sequence; exit from the program. */
   start_select_h1p2p,       /* Start with Heads 1P2P. */
   start_select_s1p2p,       /* Etc. */
   start_select_heads_start,
   start_select_sides_start,
   start_select_as_they_are,
   start_select_toggle_conc,
   start_select_toggle_singlespace,
   start_select_toggle_minigrand,
   start_select_toggle_act,
   start_select_toggle_retain,
   start_select_toggle_nowarn_mode,
   start_select_toggle_keepallpic_mode,
   start_select_toggle_singleclick_mode,
   start_select_toggle_singer,
   start_select_toggle_singer_backward,
   start_select_select_print_font,
   start_select_print_current,
   start_select_print_any,
   start_select_init_session_file,
   start_select_change_to_new_style_filename,
   start_select_change_outfile,
   start_select_change_title,
   start_select_kind_enum_extent    // Not a start_select kind; indicates extent of the enum.
};


/* For ui_resolve_select: */
/* BEWARE!!  This list must track the array "resolve_resources" in sdui-x11.c . */
enum resolve_command_kind {
   resolve_command_abort,
   resolve_command_find_another,
   resolve_command_goto_next,
   resolve_command_goto_previous,
   resolve_command_accept,
   resolve_command_raise_rec_point,
   resolve_command_lower_rec_point,
   resolve_command_write_this,
   resolve_command_kind_enum_extent    // Not a resolve kind; indicates extent of the enum.
};


struct command_list_menu_item {
   Cstring command_name;
   command_kind action;
   int resource_id;
};

struct startup_list_menu_item {
   Cstring startup_name;
   start_select_kind action;
   int resource_id;
};

struct resolve_list_menu_item {
   Cstring command_name;
   resolve_command_kind action;
};


// This defines a person in a setup.
struct personrec {
   uint32 id1;       // Frequently used bits go here.
   uint32 id2;       // Bits used for evaluating predicates.
   uint32 id3;       // The "permanent ID" bits.
};

// Bits that go into the "id1" field.
//
// This field comprises the important part of the state of this person.
// It changes frequently as the person moves around.

enum {
   // These comprise a 3 bit field for roll info.
   // High bit says person moved.
   // Low 2 bits are: 1=R; 2=L; 3=M; 0=roll unknown/unsupported.
   NROLL_MASK       = 0x00700000UL,  // mask of the field
   PERSON_MOVED     = 0x00400000UL,
   ROLL_DIRMASK     = 0x00300000UL,  // the low 2 bits, that control roll direction
   NROLL_BIT        = 0x00100000UL,  // low bit of the field
   ROLL_IS_R        = 0x00100000UL,
   ROLL_IS_L        = 0x00200000UL,
   ROLL_IS_M        = 0x00300000UL,

   // These comprise an 8 bit field for fractional stability info.  STABLE_ENAB
   // means some fractional stable (or fractional twosome) is in effect.  The
   // "R" field has the amount of fraction left to go.  It is initialized to the
   // number given (in eighths) in the fractional stable command, and counts
   // down to zero.  The "V" field is zero while the "R" field still has some
   // turning left.  Once the turning exceeds the specified amount, and the "R"
   // field is zero, the "V" field tells how this person is turned relative the
   // limit.  For example, V=6 says this person is now 2 positions (one
   // quadrant) counter-clockwise from where she was when the limit was reached,
   // and therefore needs to be turned 1 quadrant clockwise at the end of the
   // fractional stable call to compensate.
   //
   // As of version 36.63, all counting is done in eighths, not quarters.
   STABLE_MASK      = 0x000FF000UL,  // mask of the field
   STABLE_ENAB      = 0x00080000UL,  // fractional stability enabled
   STABLE_VBIT      = 0x00010000UL,  // stability "v" field, 3 bits, this is low bit
   STABLE_RBIT      = 0x00001000UL,  // stability "r" field, 4 bits, this is low bit

   BIT_PERSON       = 0x00000800UL,  // live person (just so at least one bit is always set)
   BIT_ACT_PHAN     = 0x00000400UL,  // active phantom (see below, under XPID_MASK)
   BIT_TANDVIRT     = 0x00000200UL,  // virtual person (see below, under XPID_MASK)

   // Person ID.  These bit positions are extremely hard wired into, among other
   // things, the resolver and the printer.
   PID_MASK         = 0x000001C0UL,  // these 3 bits identify actual person

   // Extended person ID.  These 5 bits identify who the person is for the purpose
   // of most manipulations.  0 to 7 are normal live people (the ones who squared up).
   // 8 to 15 are virtual people assembled for tandem or couples.  16 to 31 are
   // active (but nevertheless identifiable) phantoms.  This means that, when
   // BIT_ACT_PHAN is on, it usurps the meaning of BIT_TANDVIRT.
   XPID_MASK        = 0x000007C0UL,

   // Remaining bits:
   // For various historical reasons, we count these in octal.
   //     0x00000030 (060) - these 2 bits must be clear for rotation
   //     0x00000008 (010) - part of rotation (facing north/south)
   //     0x00000004 (004) - bit must be clear for rotation
   //     0x00000002 (002) - part of rotation
   //     0x00000001 (001) - part of rotation (facing east/west)
   d_mask  = BIT_PERSON | 013UL,
   d_north = BIT_PERSON | 010UL,
   d_south = BIT_PERSON | 012UL,
   d_east  = BIT_PERSON | 001UL,
   d_west  = BIT_PERSON | 003UL
};

// Bits that go into the "id2" field.
//
// This field contains information to respond to queries like
// "Is this person a center?" that are used in doing various operations
// like "centers run".  Such operations are usually based on this field,
// rather than on the actual geometry.  This is what makes it possible
// to do things like "bounce the centers".  This field is updated, from
// the geometry, by "update_id_bits", at strategic times, such as at
// the beginning of a call.  It is not updated between the parts of
// "bounce".

enum {
   ID2_HEADLINE   = 0x80000000UL,
   ID2_SIDELINE   = 0x40000000UL,
   ID2_CTR2       = 0x20000000UL,
   ID2_BELLE      = 0x10000000UL,
   ID2_BEAU       = 0x08000000UL,
   ID2_CTR6       = 0x04000000UL,
   ID2_OUTR2      = 0x02000000UL,
   ID2_OUTR6      = 0x01000000UL,
   ID2_TRAILER    = 0x00800000UL,
   ID2_LEAD       = 0x00400000UL,
   ID2_CTRDMD     = 0x00200000UL,
   ID2_NCTRDMD    = 0x00100000UL,
   ID2_CTR1X4     = 0x00080000UL,
   ID2_NCTR1X4    = 0x00040000UL,
   ID2_CTR1X6     = 0x00020000UL,
   ID2_NCTR1X6    = 0x00010000UL,
   ID2_OUTR1X3    = 0x00008000UL,
   ID2_NOUTR1X3   = 0x00004000UL,
   ID2_FACING     = 0x00002000UL,
   ID2_NOTFACING  = 0x00001000UL,
   ID2_CENTER     = 0x00000800UL,
   ID2_END        = 0x00000400UL,
   ID2_NEARCOL    = 0x00000200UL,
   ID2_NEARLINE   = 0x00000100UL,
   ID2_NEARBOX    = 0x00000080UL,
   ID2_FARCOL     = 0x00000040UL,
   ID2_FARLINE    = 0x00000020UL,
   ID2_FARBOX     = 0x00000010UL,
   ID2_CTR4       = 0x00000008UL,
   ID2_OUTRPAIRS  = 0x00000004UL,
   ID2_FACEFRONT  = 0x00000002UL,
   ID2_FACEBACK   = 0x00000001UL,

   // Various useful combinations.

   BITS_TO_CLEAR =
   ID2_LEAD|ID2_TRAILER|ID2_BEAU|ID2_BELLE|
   ID2_FACING|ID2_NOTFACING|ID2_CENTER|ID2_END|
   ID2_CTR2|ID2_CTR6|ID2_OUTR2|ID2_OUTR6|ID2_CTRDMD|ID2_NCTRDMD|
   ID2_CTR1X4|ID2_NCTR1X4|ID2_CTR1X6|ID2_NCTR1X6|
   ID2_OUTR1X3|ID2_NOUTR1X3|ID2_CTR4|ID2_OUTRPAIRS,

   ID2_GLOB_BITS_TO_CLEAR =
   ID2_NEARCOL|ID2_NEARLINE|ID2_NEARBOX|ID2_FARCOL|ID2_FARLINE|ID2_FARBOX|
   ID2_FACEFRONT|ID2_FACEBACK|ID2_HEADLINE|ID2_SIDELINE,

   ID2_LESS_BITS_TO_CLEAR =
   ID2_NEARCOL|ID2_NEARLINE|ID2_NEARBOX|ID2_FARCOL|ID2_FARLINE|ID2_FARBOX
};


// Bits that go into the "id3" field.
//
// This field contains "permanent" information related to the identity
// of this person.  For real people, this never changes.  The reason
// this field is needed is that the bits get cobbled together (by ANDing)
// when combining people for tandem, etc.  For active phantoms, these
// bits are all zero.

enum {
   ID3_PERM_NSG     = 0x00040000UL,  // Not side girl
   ID3_PERM_NSB     = 0x00020000UL,  // Not side boy
   ID3_PERM_NHG     = 0x00010000UL,  // Not head girl
   ID3_PERM_NHB     = 0x00008000UL,  // Not head boy
   ID3_PERM_HCOR    = 0x00004000UL,  // Head corner
   ID3_PERM_SCOR    = 0x00002000UL,  // Side corner
   ID3_PERM_HEAD    = 0x00001000UL,  // Head
   ID3_PERM_SIDE    = 0x00000800UL,  // Side
   ID3_PERM_BOY     = 0x00000400UL,  // Boy
   ID3_PERM_GIRL    = 0x00000200UL,  // Girl
   ID3_PERM_ALLBITS = 0x0007FE00UL,  // We no longer need this, since we own the whole word.

   // These are the standard definitions for the 8 people in the square.

   ID3_B1 = ID3_PERM_NSG|ID3_PERM_NSB|ID3_PERM_NHG|ID3_PERM_HCOR|ID3_PERM_HEAD|ID3_PERM_BOY,
   ID3_G1 = ID3_PERM_NSG|ID3_PERM_NSB|ID3_PERM_NHB|ID3_PERM_SCOR|ID3_PERM_HEAD|ID3_PERM_GIRL,
   ID3_B2 = ID3_PERM_NSG|ID3_PERM_NHG|ID3_PERM_NHB|ID3_PERM_SCOR|ID3_PERM_SIDE|ID3_PERM_BOY,
   ID3_G2 = ID3_PERM_NSB|ID3_PERM_NHG|ID3_PERM_NHB|ID3_PERM_HCOR|ID3_PERM_SIDE|ID3_PERM_GIRL,
   ID3_B3 = ID3_PERM_NSG|ID3_PERM_NSB|ID3_PERM_NHG|ID3_PERM_HCOR|ID3_PERM_HEAD|ID3_PERM_BOY,
   ID3_G3 = ID3_PERM_NSG|ID3_PERM_NSB|ID3_PERM_NHB|ID3_PERM_SCOR|ID3_PERM_HEAD|ID3_PERM_GIRL,
   ID3_B4 = ID3_PERM_NSG|ID3_PERM_NHG|ID3_PERM_NHB|ID3_PERM_SCOR|ID3_PERM_SIDE|ID3_PERM_BOY,
   ID3_G4 = ID3_PERM_NSB|ID3_PERM_NHG|ID3_PERM_NHB|ID3_PERM_HCOR|ID3_PERM_SIDE|ID3_PERM_GIRL
};

// The following items are not actually part of the setup description,
// but are placed here for the convenience of "move" and similar procedures.
// They contain information about the call to be executed in this setup.
// Once the call is complete, that is, when printing the setup or storing it
// in a history array, this stuff is meaningless.

struct assumption_thing {
   unsigned int assump_col:  16;  // Stuff to go with assumption -- col vs. line.
   unsigned int assump_both:  8;  // Stuff to go with assumption -- "handedness" enforcement
                                  // 0/1/2 = either/1st/2nd.
   unsigned int assump_cast:  6;  // Nonzero means there is an "assume normal casts"
                                  // assumption.
   unsigned int assump_live:  1;  // One means to accept only if everyone is live.
   unsigned int assump_negate:1;  // One means to invert the sense of everything.
   call_restriction assumption;   // Any "assume waves" type command.
};

struct small_setup {
   setup_kind skind;
   int srotation;
};



/* These bits are used to allocate flag bits
   that appear in the "callflagsf" word of a top level calldefn block
   and the "cmd_final_flags.final" of a setup with its command block. */

enum {
   // A 3-bit field.
   CFLAGH__TAG_CALL_RQ_MASK        = 0x00000007UL,
   CFLAGH__TAG_CALL_RQ_BIT         = 0x00000001UL,
   CFLAGH__REQUIRES_SELECTOR       = 0x00000008UL,
   CFLAGH__REQUIRES_DIRECTION      = 0x00000010UL,
   CFLAGH__CIRC_CALL_RQ_BIT        = 0x00000020UL,
   CFLAGH__ODD_NUMBER_ONLY         = 0x00000040UL,
   CFLAGH__HAS_AT_ZERO             = 0x00000080UL,
   CFLAGH__HAS_AT_M                = 0x00000100UL,
   CFLAGHSPARE_1                   = 0x00000200UL,
   CFLAGHSPARE_2                   = 0x00000400UL,
   CFLAGHSPARE_3                   = 0x00000800UL,
   CFLAGHSPARE_4                   = 0x00001000UL,
   CFLAGHSPARE_5                   = 0x00002000UL,
   CFLAGHSPARE_6                   = 0x00004000UL
   // We need to leave the top 12 bits free in order to accomodate the "CFLAG2" bits.
};

/* These flags, and "CFLAGH__???" flags, go along for the ride, in the callflagsf
   word of a callspec.  We use symbols that have been graciously
   provided for us from database.h to tell us what bits may be safely used next
   to the heritable flags.  Note that these bits overlap the FINAL__?? bits above.
   These are used in the callflagsf word of a callspec.  The FINAL__?? bits are
   used elsewhere.  They don't mix. */

enum {
   ESCAPE_WORD__LEFT            = CFLAGHSPARE_1,
   ESCAPE_WORD__CROSS           = CFLAGHSPARE_2,
   ESCAPE_WORD__MAGIC           = CFLAGHSPARE_3,
   ESCAPE_WORD__INTLK           = CFLAGHSPARE_4,
   ESCAPE_WORD__GRAND           = CFLAGHSPARE_5
};


/* These flags go along for the ride, in some parts of the code (BUT NOT
   THE CALLFLAGSF WORD OF A CALLSPEC!), in the same word as the heritable flags,
   but are not part of the inheritance mechanism.  We use symbols that have been
   graciously provided for us from database.h to tell us what bits may be safely
   used next to the heritable flags. */

enum finalflags {
   FINAL__SPLIT                      = CFLAGHSPARE_1,
   FINAL__SPLIT_SQUARE_APPROVED      = CFLAGHSPARE_2,
   FINAL__SPLIT_DIXIE_APPROVED       = CFLAGHSPARE_3,
   FINAL__MUST_BE_TAG                = CFLAGHSPARE_4,
   FINAL__TRIANGLE                   = CFLAGHSPARE_5,
   FINAL__LEADTRIANGLE               = CFLAGHSPARE_6
};


// Because this is used in static C-style initializers (see, for example,
// "test_setup_1x8" et. el. in sdinit.cpp and "configuration::startinfolist"
// in sdtables.cpp) we apparently can't make constructors for it, though
// we are allowed to make it a class.  And we aren't allowed to make the
// members private.  Note that the static initializers don't attempt
// to initialize this part.  So we just barely get away with this.
class final_and_herit_flags {
 public:
   heritflags herit;
   finalflags final;

   // Stuff for manipulating the "herit" bits.

   // This tests a single bit.  It returns an int rather than a bool
   // in case the client wants to combine the result with other
   // stuff for greater efficiency.
   inline uint32 test_heritbit(heritflags y) { return herit & y; }
   // This tests against a word, presumably containing a mask of 2 or more bits.
   inline uint32 test_heritbits(uint32 y) { return herit & y; }
   // This clears a single bit.
   inline void clear_heritbit(heritflags y) { herit = (heritflags) (herit & ~y); }
   // This clears a mask of bits.  Bits that are ON in the argument
   // get CLEARED in the field.
   inline void clear_heritbits(uint32 y) { herit = (heritflags) (herit & ~y); }
   // This sets a single bit.
   inline void set_heritbit(heritflags y) { herit = (heritflags) (herit | y); }
   // This sets a mask of bits.
   inline void set_heritbits(uint32 y) { herit = (heritflags) (herit | y); }
   // Clear all bits.
   inline void clear_all_heritbits() { herit = (heritflags) 0; }

   // Stuff for manipulating the "final" bits.  Exactly analogous
   // to the "herit" stuff.  See comments above.

   inline uint32 test_finalbit(finalflags y) { return final & y; }
   inline uint32 test_finalbits(uint32 y) { return final & y; }
   inline void clear_finalbit(finalflags y) { final = (finalflags) (final & ~y); }
   inline void clear_finalbits(uint32 y) { final = (finalflags) (final & ~y); }
   inline void set_finalbit(finalflags y) { final = (finalflags) (final | y); }
   inline void set_finalbits(uint32 y) { final = (finalflags) (final | y); }
   inline void clear_all_finalbits() { final = (finalflags) 0; }

   // Clear both herit and final bits.
   inline void clear_all_herit_and_final_bits()
      { herit = (heritflags) 0; final = (finalflags) 0; }

   // Test both fields, presumably for both equal to zero.  This just
   // returns the bitwise OR of both.  The expectation is that the
   // resulting integer will simply be tested for zero/nonzero.
   inline uint32 test_herit_and_final_bits() { return herit | final; }
};


// The following enumeration and struct encode the fraction/parts information
// about a call to be executed.

enum {
   // These refer to the "fraction" field.
   CMD_FRAC_NULL_VALUE      = NUMBER_FIELDS_1_0_1_1,
   CMD_FRAC_HALF_VALUE      = NUMBER_FIELDS_1_0_2_1,
   CMD_FRAC_LASTHALF_VALUE  = NUMBER_FIELDS_2_1_1_1,

   // These refer to the "flags" field.  First, there are two numeric fields,
   // called "n" and "k", that are associated with the codes.  We encode those
   // fields in 6 bits, as usual.
   CMD_FRAC_PART_BIT        = 00001UL,  // This is "n".
   CMD_FRAC_PART_MASK       = 00077UL,
   CMD_FRAC_PART2_BIT       = 00100UL,  // This is "k".
   CMD_FRAC_PART2_MASK      = 07700UL,

   CMD_FRAC_IMPROPER_BIT    = 0x00400000UL,
   CMD_FRAC_THISISLAST      = 0x00800000UL,
   CMD_FRAC_REVERSE         = 0x01000000UL,
   CMD_FRAC_CODE_MASK       = 0x0E000000UL,    // This is a 3 bit field.

   // Here are the codes that can be inside.  We require that CMD_FRAC_CODE_ONLY be zero.
   // We require that the PART_MASK field be nonzero (we use 1-based part numbering)
   // when these are in use.  If the PART_MASK field is zero, the code must be zero
   // (that is, CMD_FRAC_CODE_ONLY), and this stuff is not in use.

   CMD_FRAC_CODE_ONLY           = 0x00000000UL,
   CMD_FRAC_CODE_ONLYREV        = 0x02000000UL,
   CMD_FRAC_CODE_FROMTO         = 0x04000000UL,
   CMD_FRAC_CODE_FROMTOREV      = 0x06000000UL,
   CMD_FRAC_CODE_FROMTOREVREV   = 0x08000000UL,
   CMD_FRAC_CODE_FROMTOMOST     = 0x0A000000UL,
   CMD_FRAC_CODE_LATEFROMTOREV  = 0x0C000000UL,

   CMD_FRAC_BREAKING_UP     = 0x10000000UL,
   CMD_FRAC_FORCE_VIS       = 0x20000000UL,
   CMD_FRAC_LASTHALF_ALL    = 0x40000000UL,
   CMD_FRAC_FIRSTHALF_ALL   = 0x80000000UL
};

// The "flags" word has special information, like "do parts 5 through 2 in
// reverse order".  The "fraction" word has 4 numbers, encoding information like
// "do from 1/3 to 7/8".  The interaction of these things is quite complicated.
// See the comments in front of "get_fraction_info" in sdmoves.cpp for details
// about this.
//
// The default value ("do the whole call") is zero in the flags word and
// CMD_FRAC_NULL_VALUE in the fraction word.  Note that CMD_FRAC_NULL_VALUE is
// not zero.  Under normal circumstances, the fraction word is never zero,
// because it has fraction denominators.  There are a few special situations in
// which zero is stored in the fractions word.  For example, the
// "restrained_fraction" field of a command may have its fraction word zero.
// That means that the restrained fraction mechanism is turned off.

// Helpful macro for assembling the code and its two numeric arguments.
#define FRACS(code,n,k) (code|((n)*CMD_FRAC_PART_BIT)|((k)*CMD_FRAC_PART2_BIT))

struct fraction_command {
   uint32 flags;
   uint32 fraction;  // The fraction info, packed into 4 fields.

   inline void set_to_null()     { flags = 0; fraction = CMD_FRAC_NULL_VALUE; }
   inline void set_to_firsthalf(){ flags = 0; fraction = CMD_FRAC_HALF_VALUE; }
   inline void set_to_lasthalf() { flags = 0; fraction = CMD_FRAC_LASTHALF_VALUE; }
   inline void set_to_null_with_flags(uint32 newflags)
   { flags = newflags; fraction = CMD_FRAC_NULL_VALUE; }
   inline void set_to_firsthalf_with_flags(uint32 newflags)
   { flags = newflags; fraction = CMD_FRAC_HALF_VALUE; }
   inline void set_to_lasthalf_with_flags(uint32 newflags)
   { flags = newflags; fraction = CMD_FRAC_LASTHALF_VALUE; }

   inline bool is_null() { return flags == 0 && fraction == CMD_FRAC_NULL_VALUE; }
   inline bool is_firsthalf() { return flags == 0 && fraction == CMD_FRAC_HALF_VALUE; }
   inline bool is_lasthalf() { return flags == 0 && fraction == CMD_FRAC_LASTHALF_VALUE; }

   inline bool is_null_with_exact_flags(uint32 testflags)
   { return flags == testflags && fraction == CMD_FRAC_NULL_VALUE; }

   inline bool is_null_with_masked_flags(uint32 testmask, uint32 testflags)
   { return (flags & testmask) == testflags && fraction == CMD_FRAC_NULL_VALUE; }
};

struct setup_command {
   parse_block *parseptr;
   call_with_name *callspec;
   final_and_herit_flags cmd_final_flags;
   fraction_command cmd_fraction;
   uint32 cmd_misc_flags;
   uint32 cmd_misc2_flags;
   uint32 cmd_misc3_flags;
   uint32 do_couples_her8itflags;
   assumption_thing cmd_assume;
   uint32 prior_elongation_bits;
   uint32 prior_expire_bits;
   parse_block *restrained_concept;
   parse_block **restrained_final;
   fraction_command restrained_fraction;
   uint32 restrained_super8flags;
   bool restrained_do_as_couples;
   uint32 restrained_super9flags;
   parse_block *skippable_concept;
   uint32 skippable_heritflags;
};


// We would like to have made this a class, with the "split_info" fields
// private, since those fields are somewhat dangerous.  But this is used
// in static aggregate initializers, so we can't.  Of course, what we
// would really like to have done, if the fields were private, is have
// the methods do tricky things by accessing the those fields as a
// single 32-bit integer, somewhat the way older (extremely dangerous)
// versions of the code used to.  But it would get us into endian-ness
// problems, and do-we-really-know-that-uint32-is-twice-as-big-as-uint16
// problems, that are simply not worth it.  Especially on 64-bit machines.

struct resultflag_rec {
   uint16 split_info[2];  // The split stuff.  X in slot 0, Y in slot 1.
   uint32 misc;           // Miscellaneous info, with names like RESULTFLAG__???.

   inline void clear_split_info()
   { split_info[0] = split_info[1] = 0; }

   inline void maximize_split_info()
   { split_info[0] = split_info[1] = ~0; }

   inline void copy_split_info(const resultflag_rec & rhs)
   { split_info[0] = rhs.split_info[0]; split_info[1] = rhs.split_info[1]; }

   inline void swap_split_info_fields()
   {
      uint16 temp = split_info[0];
      split_info[0] = split_info[1];
      split_info[1] = temp;
   }
};

// Warning!  Do not rearrange these fields without good reason.  There are data
// initializers instantiating these in sdinit.cpp (test_setup_*) and in sdtables.cpp
// (startinfolist) that would need to be rewritten.
struct setup {
   setup_kind kind;
   int rotation;
   setup_command cmd;
   personrec people[MAX_PEOPLE];

   // The following item is not actually part of the setup description, but contains
   // miscellaneous information left by "move" and similar procedures, for the
   // convenience of whatever called same.
   resultflag_rec result_flags;     // Miscellaneous info, with names like RESULTFLAG__???.

   // The following three items are only used if the setup kind is "s_normal_concentric".
   // Note in particular that "outer_elongation" is thus underutilized, and that a lot
   // of complexity goes into storing similar information (in two different places!)
   // in the "prior_elongation_bits" and "result_flags" words.
   small_setup inner;
   small_setup outer;
   int concsetup_outer_elongation;

   void clear_people();
   inline void clear_person(int resultplace);
   inline void suppress_roll(int place);
   inline void suppress_all_rolls();
   inline void swap_people(int oneplace, int otherplace);
};


class conc_tables {

 private:

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "conc_init_table" be a statically initialized array.

   struct cm_thing {
      setup_kind bigsetup;
      calldef_schema lyzer;
      veryshort maps[24];
      setup_kind insetup;
      setup_kind outsetup;
      int inner_rot;    // 1 if inner setup is rotated CCW relative to big setup
      int outer_rot;    // 1 if outer setup is rotated CCW relative to big setup
      int mapelong;
      int center_arity;
      int elongrotallow;
      calldef_schema getout_schema;
      uint32 used_mask_analyze;
      uint32 used_mask_synthesize;
      cm_thing *next_analyze;
      cm_thing *next_synthesize;
   };

   // Must be a power of 2.
   enum { NUM_CONC_HASH_BUCKETS = 32 };

   // The big initialization table, in sdtables.
   static cm_thing conc_init_table[];

   // The hash tables, constructed during initialization.  In sdconc.
   static cm_thing *conc_hash_synthesize_table[NUM_CONC_HASH_BUCKETS];
   static cm_thing *conc_hash_analyze_table[NUM_CONC_HASH_BUCKETS];

 public:

   static bool analyze_this(             // In sdconc.
      setup *ss,
      setup *inners,
      setup *outers,
      int *center_arity_p,
      int & mapelong,
      int & inner_rot,
      int & outer_rot,
      calldef_schema analyzer);

   static bool synthesize_this(             // In sdconc.
      setup *inners,
      setup *outers,
      int center_arity,
      uint32 orig_elong_is_controversial,
      int i,
      uint32 matrix_concept,
      int outer_elongation,
      calldef_schema synthesizer,
      setup *result);

   static void initialize();             // In sdconc.
};


struct predicate_descriptor {
   // We wish we could put a "throw" clause on this function, but we can't.
   bool (*predfunc) (setup *, int, int, int, const long int *);
   const long int *extra_stuff;
};

struct predptr_pair {
   predicate_descriptor *pred;
   predptr_pair *next;
   /* Dynamically allocated to whatever size is required. */
   uint16 arr[4];
};


enum {
   LOOKUP_NONE     = 0x1UL,
   LOOKUP_DIST_CLW = 0x2UL,
   LOOKUP_DISC     = 0x4UL,
   LOOKUP_IGNORE   = 0x8UL,
   LOOKUP_DIST_DMD = 0x10UL,
   LOOKUP_Z        = 0x20UL,
   LOOKUP_DIST_BOX = 0x40UL,
   LOOKUP_DIAG_BOX = 0x80UL,
   LOOKUP_STAG_BOX = 0x100UL,
   LOOKUP_TRAPEZOID= 0x200UL,
   LOOKUP_DIAG_CLW = 0x400UL,
   LOOKUP_OFFS_CLW = 0x800UL,
   LOOKUP_STAG_CLW = 0x1000UL,
   LOOKUP_DBL_BENT = 0x2000UL,
   LOOKUP_MINI_B   = 0x4000UL,
   LOOKUP_MINI_O   = 0x8000UL,

   LOOKUP_GEN_MASK = (LOOKUP_DIST_DMD|LOOKUP_Z|LOOKUP_DIST_BOX|LOOKUP_DIAG_BOX|
                      LOOKUP_STAG_BOX|LOOKUP_TRAPEZOID|LOOKUP_DIAG_CLW|
                      LOOKUP_OFFS_CLW|LOOKUP_STAG_CLW|LOOKUP_DBL_BENT|
                      LOOKUP_MINI_B|LOOKUP_MINI_O)
};


class select {

 public:

   enum fixerkey {
      fx0,        // The null table entry.
      fx_foo33a,
      fx_foocc,
      fx_foo33,
      fx_foo5a,
      fx_fooa5,
      fx_foo55,
      fx_fooaa,
      fx_foo11,
      fx_foo22,
      fx_foo44,
      fx_foo88,
      fx_foo77,
      fx_fooEE,
      fx_n1x43,
      fx_n1x4c,
      fx_n1x45,
      fx_n1x4a,
      fx_n1x3a,
      fx_n1x3b,
      fx_f1x6aa,
      fx_f1x8aa,
      fx_f1x855,
      fx_fo6zz,
      fx_foozz,
      fx_fo6zzd,
      fx_foozzd,
      fx_f3x4east,
      fx_f3x4west,
      fx_f3x4left,
      fx_f3x4right,
      fx_f3x4lzz,
      fx_f3x4rzz,
      fx_f4x4nw,
      fx_f4x4ne,
      fx_f4x4wn,
      fx_f4x4en,
      fx_f4x4lzz,
      fx_f4x4rzz,
      fx_f4x4lzza,
      fx_f4x4rzza,
      fx_f2x4tt0,
      fx_f2x4tt1,
      fx_f2x8qq0,
      fx_f2x8qq1,
      fx_f2x8tt0,
      fx_f2x8tt1,
      fx_f2x8tt2,
      fx_f2x8tt3,
      fx_f2x6qq0,
      fx_f2x6qq1,
      fx_f2x6tt0,
      fx_f2x6tt1,
      fx_f2x6tt2,
      fx_f2x6tt3,
      fx_f3x4outer,
      fx_f3dmouter,
      fx_f3ptpdin,
      fx_fpgdmdcw,
      fx_fpgdmdccw,
      fx_f2x6cw,
      fx_f2x6ccw,
      fx_fdhrgl,
      fx_specspindle,
      fx_specfix3x40,
      fx_specfix3x41,
      fx_f525nw,
      fx_f525ne,
      fx_fdpx44,
      fx_fdpxwve,
      fx_f1x12outer,
      fx_f3x4outrd,
      fx_f3dmoutrd,
      fx_fdhrgld,
      fx_f1x12outrd,
      fx_f1x12outre,
      fx_f1x3d6,
      fx_f323,
      fx_f323d,
      fx_f3x1zzd,
      fx_f1x3zzd,
      fx_f3x1yyd,
      fx_f2x1yyd,
      fx_f1x4xv,
      fx_f1x3yyd,
      fx_f1x6aad,
      fx_f1x4qv,
      fx_fo6qqd,
      fx_f21dabd,
      fx_f12dabd,
      fx_f1x6abd,
      fx_f2x1ded,
      fx_f1x6ed,
      fx_f1x8aad,
      fx_fxwv1d,
      fx_fxwv2d,
      fx_fxwv3d,
      fx_fqtgns,
      fx_ftharns,
      fx_ftharew,
      fx_fqtgj1,
      fx_fqtgj2,
      fx_f2x3j1,
      fx_f2x3j2,
      fx_fqtgjj1,
      fx_fqtgjj2,
      fx_f2x3jj1,
      fx_f2x3jj2,
      fx_f2x3jj3,
      fx_fgalcv,
      fx_fgalch,
      fx_fdblbentcw,
      fx_fdblbentccw,
      fx_fcrookedcw,
      fx_fcrookedccw,
      fx_fspindlc,
      fx_fspindlf,
      fx_fspindlg,
      fx_fspindlfd,
      fx_fspindlgd,
      fx_ftgl6cwd,
      fx_ftgl6ccwd,
      fx_ftgl6cld,
      fx_ftgl6ccld,
      fx_ftgl6ccd,
      fx_ftgl6cccd,
      fx_f1x3aad,
      fx_f2x3c,
      fx_f2x3a41,
      fx_f2x3a14,
      fx_box3c,
      fx_box6c,
      fx_box9c,
      fx_boxcc,
      fx_box55,
      fx_boxaa,
      fx_f2x5c,
      fx_f2x5d,
      fx_fd2x5d,
      fx_bghrgla,
      fx_bghrglb,
      fx_shsqtga,
      fx_shsqtgb,
      fx_f4x6a,
      fx_f4x6b,
      fx_f4x6c,
      fx_f4x6d,
      fx_f4x6e,
      fx_f4x6f,
      fx_f1x2aad,
      fx_f2x166d,
      fx_f1x3bbd,
      fx_fhrglassd,
      fx_fspindld,
      fx_fptpzzd,
      fx_f3ptpo6,
      fx_fspindlbd,
      fx_fspindlod,
      fx_d2x4b1,
      fx_d2x4b2,
      fx_d2x4w1,
      fx_d2x4w2,
      fx_d2x4q1,
      fx_d2x4q2,
      fx_d2x4d1,
      fx_d2x4d2,
      fx_d2x4c1,
      fx_d2x4c2,
      fx_d2x4z1,
      fx_d2x4z2,
      fx_d2x4y1,
      fx_d2x4y2,
      fx_d3x4ul1,
      fx_d3x4ul2,
      fx_d3x4ul3,
      fx_d3x4ul4,
      fx_d3x4ul5,
      fx_d3x4ul6,
      fx_d3x4ub1,
      fx_d3x4ub2,
      fx_d3x4ub3,
      fx_d3x4ub4,
      fx_d3x4vl1,
      fx_d3x4vl2,
      fx_d3x4vl3,
      fx_d3x4vl4,
      fx_d3x4vb1,
      fx_d3x4vb2,
      fx_d3x4vb3,
      fx_d3x4vb4,
      fx_d2x4x1,
      fx_d2x4x2,
      fx_dgalw1,
      fx_dgalw2,
      fx_dgalw3,
      fx_dgalw4,
      fx_dgald1,
      fx_dgald2,
      fx_dgald3,
      fx_dgald4,
      fx_ddmd1,
      fx_ddmd2,
      fx_distbone1,
      fx_distbone2,
      fx_distbone5,
      fx_distbone6,
      fx_distpgdmd1,
      fx_distpgdmd2,
      fx_distrig3,
      fx_distrig1,
      fx_distrig4,
      fx_distrig2,
      fx_distrig7,
      fx_distrig5,
      fx_distrig8,
      fx_distrig6,
      fx_distptp1,
      fx_distptp2,
      fx_distptp3,
      fx_distptp4,
      fx_distptp7,
      fx_distptp8,
      fx_disthrg1,
      fx_disthrg2,
      fx_d4x4l1,
      fx_d4x4l2,
      fx_d4x4l3,
      fx_d4x4l4,
      fx_d4x4d1,
      fx_d4x4d2,
      fx_d4x4d3,
      fx_d4x4d4,
      fx_dqtgdbb1,
      fx_dqtgdbb2,
      fx_hrgminb,
      fx_ptpminb,
      fx_rigmino,
      fx_galmino,
      fx_qtgminb,
      fx_fcpl12,
      fx_fcpl23,
      fx_fcpl34,
      fx_fcpl41,
      fx_foo55d,
      fx_fgalctb,
      fx_f3x1ctl,
      fx_f2x2pl,
      fx_f1x4pl,
      fx_fdmdpl,
      fx_f1x2pl,
      fx_f3x1d_2,
      fx_f1x8_88,
      fx_f1x8_22,
      fx_f1x8_11,
      fx_fdqtagzzz,
      fx_f1x8_77_3,
      fx_f1x8_77,
      fx_fdrhgl_bb,
      fx_f3x1d77,
      fx_f1x8_bb,
      fx_f1x8_dd,
      fx_foo99d,
      fx_foo66d,
      fx_f1x8ctr,
      fx_fqtgctr,
      fx_fxwve,
      fx_fboneendd,
      fx_fqtgend,
      fx_f1x6endd,
      fx_f1x2dd,
      fx_fbn6ndd,
      fx_f2x3od,
      fx_fdmded,
      fx_f1x4ed,
      fx_fdrhgl1,
      fx_f1x8endd,
      fx_f1x8endo,
      fx_f1x8lowf,
      fx_f1x8hif,
      fx_fbonectr,
      fx_fbonetgl,
      fx_frigtgl,
      fx_fboneendo,
      fx_frigendd,
      fx_frigctr,
      fx_f2x4ctr,
      fx_f2x4lr,
      fx_f2x4rl,
      fx_f2x4far,
      fx_f2x4near,
      fx_f4dmdiden,
      fx_f2x4pos1,
      fx_f2x4pos2,
      fx_f2x4pos3,
      fx_f2x4pos4,
      fx_f2x4pos5,
      fx_f2x4pos6,
      fx_f2x4pos7,
      fx_f2x4pos8,
      fx_f2x4posa,
      fx_f2x4posb,
      fx_f2x4posc,
      fx_f2x4posd,
      fx_f2x4pose,
      fx_f2x4posf,
      fx_f2x4posp,
      fx_f2x4posq,
      fx_f2x4posr,
      fx_f2x4poss,
      fx_f2x4posy,
      fx_f2x4posz,
      fx_f2x4left,
      fx_f2x4right,
      fx_f2x4dleft,
      fx_f2x4dright,
      fx_f2zzrdsc,
      fx_f2zzrdsd,
      fx_f288rdsd,
      fx_f2yyrdsc,
      fx_f2x5rdsd,
      fx_f2qt1dsc,
      fx_f2qt1dsd,
      fx_f2qt2dsc,
      fx_f2qt2d99,
      fx_f2x6ndsc,
      fx_f2x6ndsd,
      fx_fdmdndx,
      fx_fdmdnd,
      fx_f3x4ndsd,
      fx_f2x8nd99,
      fx_f2x8nda5,
      fx_f4x6ndoo,
      fx_f4x6ndxx,
      fx_f1x8nd96,
      fx_f1x8nd69,
      fx_f1x8nd41,
      fx_f1x8nd82,
      fx_f1x8nd28,
      fx_f1x8nd14,
      fx_f1x10ndsc,
      fx_f1x10ndsd,
      fx_f1x10ndse,
      fx_z1x4u,
      fx_z2x3a,
      fx_z2x3b,
      fx_fgalpts,
      fx_f2x4endd,
      fx_fhrgl1,
      fx_fhrgl2,
      fx_fhrgle,
      fx_fptpdid,
      fx_f2x477,
      fx_f2x4ee,
      fx_f2x4bb,
      fx_f2x4dd,
      fx_fdhrgld1,
      fx_fdhrgld2,
      fx_f2x4endo,
      fx_f4x4endo,
      fx_bar55d,
      fx_fppaad,
      fx_fpp55d,
      fx_ENUM_EXTENT   // Not a fixer; indicates extent of the enum.
   };

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "fixer_init_table" be a statically initialized array.

   struct fixer {
      fixerkey mykey;
      setup_kind ink;
      setup_kind outk;
      int rot;
      short prior_elong;
      short numsetups;
      fixerkey next1x2;
      fixerkey next1x2rot;
      fixerkey next1x4;
      fixerkey next1x4rot;
      fixerkey nextdmd;
      fixerkey nextdmdrot;
      fixerkey next2x2;
      fixerkey next2x2v;
      veryshort nonrot[24];
   };

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "sel_init_table" be a statically initialized array.

   struct sel_item {
      uint32 key;
      setup_kind kk;
      uint32 thislivemask;
      fixerkey fixp;
      fixerkey fixp2;
      int use_fixp2;
      sel_item *next;
   };

 private:

   // Must be a power of 2.
   enum { NUM_SEL_HASH_BUCKETS = 32 };

   // The big initialization table, in sdtables.
   static sel_item sel_init_table[];

   // The hash table, constructed during initialization.  In sdconc.
   static sel_item *sel_hash_table[NUM_SEL_HASH_BUCKETS];

   // The big initialization table, in sdtables.
   static fixer fixer_init_table[];

 public:

   // In sdconc.
   static fixer *fixer_ptr_table[fx_ENUM_EXTENT];
   static void initialize();             // In sdconc.

   // In sdconc.
   static const fixer *hash_lookup(setup_kind kk, uint32 thislivemask,
                                   uint32 key, uint32 arg, const setup *ss);
};

class tglmap {

 public:

   enum tglmapkey {
      tgl0,        // The null table entry.
      tglmap1b,
      tglmap2b,
      tglmap1i,
      tglmap2i,
      tglmap1d,
      tglmap2d,
      tglmap1m,
      tglmap2m,
      tglmap1j,
      tglmap2j,
      tglmap1x,
      tglmap2x,
      tglmap1y,
      tglmap2y,
      tglmap1k,
      tglmap2k,
      tglmap2r,
      tgl_ENUM_EXTENT   // Not a key; indicates extent of the enum.
   };

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "init_table", and other items, be statically
   // initialized arrays.

   struct map {
      tglmapkey mykey;
      setup_kind kind;
      setup_kind kind1x3;
      tglmapkey otherkey;
      veryshort nointlkshapechange;
      veryshort switchtgls;
      veryshort mapqt1[8];   // In quarter-tag: first triangle (upright),
                             // then second triangle (inverted), then idle.
      veryshort mapcp1[8];   // In C1 phantom: first triangle (inverted),
                             // then second triangle (upright), then idle.
      veryshort mapbd1[8];   // In bigdmd.
      veryshort map241[8];   // In 2x4.
      veryshort map261[8];   // In 2x6.
   };

 private:

   // The big initialization table, in sdtables.
   static const map init_table[];

   // The pointer table, constructed during initialization.
   static const map *ptrtable[tgl_ENUM_EXTENT];

 public:

   static void initialize();             // In sdistort.

   static void do_glorious_triangles(    // In sdistort.
      setup *ss,
      const tglmapkey *map_ptr_table,
      int indicator,
      setup *result) THROW_DECL;

   // In sdtables.
   static const tglmapkey c1tglmap1[];
   static const tglmapkey c1tglmap2[];
   static const tglmapkey dbqtglmap1[];
   static const tglmapkey dbqtglmap2[];
   static const tglmapkey qttglmap1[];
   static const tglmapkey qttglmap2[];
   static const tglmapkey bdtglmap1[];
   static const tglmapkey bdtglmap2[];
   static const tglmapkey rgtglmap1[];
};


struct startinfo {
   char *name;
   bool into_the_middle;
   setup the_setup;
};


typedef uint32 id_bit_table[4];

struct ctr_end_mask_rec {
   uint32 mask_normal;
   uint32 mask_6_2;
   uint32 mask_2_6;
   uint32 mask_ctr_dmd;
};


// BEWARE!!  This list must track the array "warning_strings" in sdtables.cpp
enum warning_index {
   warn__none,
   warn__really_no_collision,
   warn__do_your_part,
   warn__tbonephantom,
   warn__awkward_centers,
   warn__bad_concept_level,
   warn__not_funny,
   warn__hard_funny,
   warn__rear_back,
   warn__awful_rear_back,
   warn__excess_split,
   warn__lineconc_perp,
   warn__dmdconc_perp,
   warn__lineconc_par,
   warn__dmdconc_par,
   warn__xclineconc_perpc,
   warn__xcdmdconc_perpc,
   warn__xclineconc_perpe,
   warn__ctrstand_endscpls,
   warn__ctrscpls_endstand,
   warn__each2x2,
   warn__each1x4,
   warn__each1x2,
   warn__eachdmd,
   warn__take_right_hands,
   warn__ctrs_are_dmd,
   warn__1_4_pgram,
   warn__full_pgram,
   warn__3_4_pgram,
   warn__offset_gone,
   warn__overlap_gone,
   warn__to_o_spots,
   warn__to_x_spots,
   warn__check_butterfly,
   warn__check_galaxy,
   warn__some_rear_back,
   warn__not_tbone_person,
   warn__check_c1_phan,
   warn__check_dmd_qtag,
   warn__check_quad_dmds,
   warn__may_be_fudgy,
   warn__fudgy_half_offset,
   warn__check_3x4,
   warn__check_2x4,
   warn__check_hokey_2x4,
   warn__check_4x4,
   warn__check_4x6,
   warn__check_hokey_4x4,
   warn__check_4x4_start,
   warn__check_centered_qtag,
   warn__check_pgram,
   warn__ctrs_stay_in_ctr,
   warn__check_c1_stars,
   warn__check_gen_c1_stars,
   warn__bigblock_feet,
   warn__bigblockqtag_feet,
   warn__diagqtag_feet,
   warn__adjust_to_feet,
   warn__some_touch,
   warn__some_touch_evil,
   warn__split_to_2x4s,
   warn__split_to_2x3s,
   warn__split_to_1x8s,
   warn__split_to_1x6s,
   warn__take_left_hands,
   warn__left_half_pass,
   warn__evil_interlocked,
   warn__split_phan_in_pgram,
   warn__bad_interlace_match,
   warn__not_on_block_spots,
   warn__stupid_phantom_clw,
   warn__should_say_Z,
   warn__bad_modifier_level,
   warn__bad_call_level,
   warn__did_not_interact,
   warn__opt_for_normal_cast,
   warn__opt_for_normal_hinge,
   warn__opt_for_2fl,
   warn__opt_for_no_collision,
   warn__opt_for_not_tboned_base,
   warn_partial_solomon,
   warn_same_z_shear,
   warn__like_linear_action,
   warn__phantoms_thinner,
   warn__hokey_jay_shapechanger,
   warn__split_1x6,
   warn_interlocked_to_6,
   warn__offset_hard_to_see,
   warn__pg_hard_to_see,
   warn__phantom_judge,
   warn__colocated_once_rem,
   warn_big_outer_triangles,
   warn_hairy_fraction,
   warn_bad_collision,
   warn_very_bad_collision,
   warn__dyp_resolve_ok,
   warn__unusual,
   warn_controversial,
   warn_serious_violation,
   warn__assume_dpt,
   warn_bogus_yoyo_rims_hubs,
   warn_pg_in_2x6,
   warn_real_people_spots,
   warn__tasteless_com_spot,
   warn__tasteless_junk,
   warn__tasteless_slide_thru,
   warn__compress_carefully,
   warn__diagnostic,
   warn__NUM_WARNINGS       // Not a real warning; just used for counting.
};

struct coordrec {
   setup_kind result_kind;
   int xfactor;
   veryshort xca[24];
   veryshort yca[24];
   veryshort diagram[64];
};


class merge_table {

 public:

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "merge_init_table" be a statically initialized array.

   struct concmerge_thing {
      setup_kind k1;
      setup_kind k2;
      uint32 m1;
      uint32 m2;
      /* This is the mask of things that we will reject.  The low 4 bits are rotations
         that we will reject.  It is ANDed with "1 << r".  R is the rotation of res1,
         after localizing so that res2 has rotation zero.  Hence r=0 if the two setups
         have the same orientation, and, except in the case of things like triangles,
         r=1 if they are orthogonal.  Common values of the low hex digit of "rotmask"
         are therefore:
         E demand same orientation
         D demand orthogonal
         C either way.

         Additionally:
         The "10" bit means that action must be merge_without_gaps.
         The "20" bit means that action must NOT be merge_strict_matrix.
         The "40" bit means only accept it if the setups, prior to cutting down,
         were a 2x4 and a 1x8 that were perpendicular to each other.
         The "80" bit means that action must NOT be merge_c1_phantom_real.
         The "100" bit means that original action must NOT be merge_after_dyp.
         The "200" bit means only accept it if the setups, prior to cutting down,
         were a 2x4 and a ptpd that were perpendicular to each other. */

      unsigned short rotmask;
      // 1 bit - swap setups;
      // 2 bit - change elongation;
      // 4 bit - no take right hands
      // 8 bit - force outer_elong to 3, so people will go to corners of 4x4
      unsigned short swap_setups;
      calldef_schema conc_type;
      setup_kind innerk;
      setup_kind outerk;
      warning_index warning;
      int irot;
      int orot;
      veryshort innermap[16];
      veryshort outermap[24];  // Only one map (bigh+4x6) needs more than 16
      concmerge_thing *next;
   };

 private:

   // Must be a power of 2.
   enum { NUM_MERGE_HASH_BUCKETS = 32 };

   // The big initialization table, in sdtables.
   static concmerge_thing merge_init_table[];

   // The hash table, constructed during initialization.  In sdconc.
   static concmerge_thing *merge_hash_tables[NUM_MERGE_HASH_BUCKETS];

 public:

   static const concmerge_thing map_tgl4l;
   static const concmerge_thing map_tgl4b;
   static const concmerge_thing map_2234b;

   static const concmerge_thing *lookup(setup_kind res1k,
                                        setup_kind res2k,
                                        unsigned int rotreject,
                                        uint32 mask1,
                                        uint32 mask2);

   static void initialize();             // In sdconc.
};

struct setup_attr {
   // This is the size of the setup MINUS ONE.
   int setup_limits;

   // These "coordrec" items have the fudged coordinates that are used for doing
   // press/truck calls.  For some setups, the coordinates of some people are
   // deliberately moved away from the obvious precise matrix spots so that
   // those people can't press or truck.  For example, the lateral spacing of
   // diamond points is not an integer.  If a diamond point does any truck or loop
   // call, he/she will not end up on the other diamond point spot (or any other
   // spot in the formation), so the call will not be legal.  This enforces our
   // view, not shared by all callers (Hi, Clark!) that the diamond points are NOT
   // as if the ends of lines of 3, and hence can NOT trade with each other by
   // doing a right loop 1.
   const coordrec *setup_coords;

   // The above table is not suitable for performing mirror inversion because,
   // for example, the points of diamonds do not reflect onto each other.  This
   // table has unfudged coordinates, in which all the symmetries are observed.
   // This is the table that is used for mirror reversal.  Most of the items in
   // it are the same as those in the table above.
   const coordrec *nice_setup_coords;

   // These determine how designators like "side boys" get turned into
   // "center 2", so that so-and-so moves can be done with the much
   // more powerful concentric mechanism.
   ctr_end_mask_rec setup_conc_masks;

   // These show the beginning setups that we look for in a by-array call
   // definition in order to do a call in this setup.
   begin_kind keytab[2];

   // In the bounding boxes, we do not fill in the "length" of a diamond, nor
   // the "height" of a qtag.  Everyone knows that the number must be 3, but it
   // is not really accepted that one can use that in instances where precision
   // is required.  That is, one should not make "matrix" calls depend on this
   // number.  Witness all the "diamond to quarter tag adjustment" stuff that
   // callers worry about, and the ongoing controversy about which way a quarter
   // tag setup is elongated, even though everyone knows that it is 4 wide and 3
   // deep, and that it is generally recognized, by the mathematically erudite,
   // that 4 is greater than 3.
   short int bounding_box[2];

   // This is true if the setup has 4-way symmetry.  Such setups will always be
   // canonicalized so that their rotation field will be zero.
   bool four_way_symmetry;

   // This is the bit table for filling in the "ID2" bits.
   const id_bit_table *id_bit_table_ptr;

   // These are the tables that show how to print out the setup.
   Cstring print_strings[2];
};


extern const setup_attr setup_attrs[];    // In sdtables.


// These are the "schema_attr" bits.
enum {
   SCA_CENTRALCONC     = 0x00000001UL,
   SCA_CROSS           = 0x00000002UL,
   SCA_COPY_LYZER      = 0x00000004UL,
   SCA_SNAGOK          = 0x00000008UL,
   SCA_DETOUR          = 0x00000010UL,
   SCA_SPLITOK         = 0x00000020UL,
   SCA_INV_SUP_ELWARN  = 0x00000040UL,
   SCA_CONC_REV_ORDER  = 0x00000080UL
};

struct schema_attr {
   uint32 attrs;
   calldef_schema uncrossed;
};


extern const schema_attr schema_attrs[];    // In sdtables.

// This class just exists for the purpose of reading out the setup_limits.
// It had much more glorious plans at one time.

class attr {

 public:

   // Get the "limit" (that's the number of people minus one), given
   // only a setup kind.
   static inline int klimit(setup_kind k) { return setup_attrs[k].setup_limits; }

   // Get the limit for a given setup, reading out its setup kind.
   static inline int slimit(const setup *s) { return setup_attrs[s->kind].setup_limits; }
};

struct writechar_block_type {
   char *destcurr;
   char lastchar;
   char lastlastchar;
   char *lastblank;
   bool usurping_writechar;
};


/* Probability (out of 8) that a concept will be placed on a randomly generated call. */
#define CONCEPT_PROBABILITY 2
/* We use lots more concepts for "standardize", since it is much less likely (though
   by no means impossible) that a plain call will do the job. */
#define STANDARDIZE_CONCEPT_PROBABILITY 6

/* Actually, we don't make a resolve bigger than 3.  This is how much space
   we allocate for things.  Just being careful. */
#define MAX_RESOLVE_SIZE 5


/* The Sd program reads this binary file for its calls database */
#ifndef DATABASE_FILENAME
#define DATABASE_FILENAME "sd_calls.dat"
#endif

/* The source form of the calls database.  The mkcalls program compiles it. */
#ifndef CALLS_FILENAME
#define CALLS_FILENAME "sd_calls.txt"
#endif

/* The output filename prefix.  ".level" is added to the name. */
#ifndef SEQUENCE_FILENAME
#define SEQUENCE_FILENAME "sequence"
#endif

/* The file containing the user's current working sessions. */
#ifndef SESSION_FILENAME
#define SESSION_FILENAME "sd.ini"
#endif

/* The temporary file used when rewriting the above. */
#ifndef SESSION2_FILENAME
#define SESSION2_FILENAME "sd2.ini"
#endif


#define MAX_ERR_LENGTH 200
#define MAX_FILENAME_LENGTH 260
#define INPUT_TEXTLINE_SIZE 300
/* Absolute maximum length we can handle in text operations, including
   writing to file.  If a call gets more complicated than this, stuff
   will simply not be written to the file.  Too bad. */
#define MAX_TEXT_LINE_LENGTH 200


/* Codes for special accelerator keystrokes. */

/* Function keys can be plain, shifted, control, alt, or control-alt. */

#define FKEY 128
#define SFKEY 144
#define CFKEY 160
#define AFKEY 176
#define CAFKEY 192

/* "Enhanced" keys can be plain, shifted, control, alt, or control-alt. */
/* e1 = page up
   e2 = page down
   e3 = end
   e4 = home
   e5 = left arrow
   e6 = up arrow
   e7 = right arrow
   e8 = down arrow
   e13 = insert
   e14 = delete */

#define EKEY 208
#define SEKEY 224
#define CEKEY 240
#define AEKEY 256
#define CAEKEY 272

/* Digits can be control, alt, or control-alt. */

#define CTLDIG 288
#define ALTDIG 298
#define CTLALTDIG 308

/* Numeric keypad can be control, alt, or control-alt. */

#define CTLNKP 318
#define ALTNKP 328
#define CTLALTNKP 338

/* Letters can be control, alt, or control-alt. */

#define CTLLET (348-'A')
#define ALTLET (374-'A')
#define CTLALTLET (400-'A')

#define FCN_KEY_TAB_LOW (FKEY+1)
#define FCN_KEY_TAB_LAST (CTLALTLET+'Z')

// This allows numbers from 0 to 36, inclusive.
enum {
   NUM_CARDINALS = 37
};


/* This is the number of tagger classes.  It must not be greater than 7,
   because class numbers, in 1-based form, are put into the CFLAGH__TAG_CALL_RQ_MASK
   and CFLAG1_BASE_TAG_CALL fields, and because the tagger class is stored
   (albeit in 0-based form) in the high 3 bits (along with the tagger call number
   in the low 5 bits) in an 8-bit field that is replicated 4 times in the
   "tagger" field of a call_conc_option_state. */
#define NUM_TAGGER_CLASSES 4


// These are the values returned in user_match.match.kind
// by "uims_get_call_command" and similar functions.

enum uims_reply {
   ui_special_concept,  // Not a real return; used only for fictional purposes
                        //    in the user interface; never appears in the rest of the program.
   ui_command_select,   // (normal/resolve) User chose one of the special buttons
                        //    like "resolve" or "quit".
   ui_resolve_select,   // (resolve only) User chose one of the various actions
                        //    peculiar to resolving.
   ui_start_select,     // (startup only) User chose something.
                        //    This is the only outcome in startup mode.
   ui_concept_select,   // (normal only) User selected a concept.
   ui_call_select,      // (normal only) User selected a call from the current call menu.
   ui_help_simple,      // (any) user selected "help"
   ui_help_manual       // (any) user selected "help manual"
};


/*
 * A match_result describes the result of matching a string against
 * a set of acceptable commands. A match_result is effectively a
 * sequence of values to be returned to the main program. A sequence
 * is required because the main program asks for information in
 * bits and pieces.  For example, the call "touch 1/4" first returns
 * "touch <N/4>" and then returns "1".  This structure for the
 * interaction between the main program and the UI reflects the
 * design of the original Domain Dialog UI.
 *
 */

struct modifier_block {
   uims_reply kind;
   long int index;
   call_conc_option_state call_conc_options;  /* Has numbers, selectors, etc. */
   call_with_name *call_ptr;
   const conzept::concept_descriptor *concept_ptr;
   modifier_block *packed_next_conc_or_subcall;  /* next concept, or, if this is end mark, points to substitution list */
   modifier_block *packed_secondary_subcall; // points to substitution list for secondary subcall
   modifier_block *gc_ptr;                /* used for reclaiming dead blocks */
};

struct match_result {
   bool valid;               // Set to true if a match was found.
   bool exact;               // Set to true if an exact match was found.
   bool indent;              // This is a subordinate call; indent it in listing.
   modifier_block match;     // The little thing we actually return.
   const match_result *real_next_subcall;
   const match_result *real_secondary_subcall;
   int recursion_depth;      // How deep in "@0" or "@m" things.
   int yield_depth;          // If nonzero, this yields by that amount.
};

struct abbrev_block {
   Cstring key;
   modifier_block value;
   abbrev_block *next;
};

enum {
    special_index_lineup = -1,
    special_index_linedown = -2,
    special_index_pageup = -3,
    special_index_pagedown = -4,
    special_index_deleteline = -5,
    special_index_deleteword = -6,
    special_index_quote_anything = -7,
    special_index_copytext = -8,
    special_index_cuttext = -9,
    special_index_pastetext = -10
};


enum {
    match_startup_commands = -1,
    match_resolve_commands = -2,
    match_selectors = -3,
    match_directions = -4,
    match_taggers = -8,      /* This is the lowest of 4 numbers. */
    match_circcer = -9,
    match_number = -10       /* Only used by sdui-win; sdui-tty just reads it and uses atoi. */
};


/* BEWARE!!  There may be tables in the user interface file keyed to this enumeration.
   In particular, this list must track the array "menu_names" in sdtables.c . */
/* BEWARE!!  This list is keyed to some messy stuff in the procedure "initialize_concept_sublists".
   In particular, there are octal constants like "MASK_CTR_2" that contain bits assigned
   according to these items.  Changing these items is not recommended. */

enum call_list_kind {
   call_list_none, call_list_empty, /* Not real call list kinds; used only for
                                       fictional purposes in the user interface;
				       never appear in the rest of the program. */
   call_list_any,                   /* This is the "universal" call list; used
                                       whenever the setup isn't one of the known ones. */
   call_list_1x8, call_list_l1x8,
   call_list_dpt, call_list_cdpt,
   call_list_rcol, call_list_lcol,
   call_list_8ch, call_list_tby,
   call_list_lin, call_list_lout,
   call_list_rwv, call_list_lwv,
   call_list_r2fl, call_list_l2fl,
   call_list_gcol, call_list_qtag,

   call_list_extent    // Not a start call_list kind; indicates extent of the enum.
};

// BEWARE!!  Order is important.  Various comparisons are made.
enum abridge_mode_t {
   abridge_mode_none,             // Running, no abridgement.
   abridge_mode_deleting_abridge, // Like the above, but user has explicitly
                                  // requested that the abridgement specified for
                                  // the current session be removed.
   abridge_mode_abridging,        // Running, with abridgement list.
   abridge_mode_writing,          // Just writing the list; don't run the program itself.
   abridge_mode_writing_full      // Same, but write all lower lists as well.
};

struct parse_stack_item {
   parse_block **concept_write_save_ptr;
   concept_kind save_concept_kind;
};

struct parse_state_type {
   parse_stack_item parse_stack[40];
   int parse_stack_index;
   parse_block **concept_write_ptr;
   parse_block **concept_write_base;
   char specialprompt[MAX_TEXT_LINE_LENGTH];
   uint32 topcallflags1;
   call_list_kind call_list_to_use;
   call_list_kind base_call_list_to_use;
};

struct selector_item {
   Cstring name;
   Cstring sing_name;
   Cstring name_uc;
   Cstring sing_name_uc;
   selector_kind opposite;
};

enum mode_kind {
   mode_none,     /* Not a real mode; used only for fictional purposes
                        in the user interface; never appears in the rest of the program. */
   mode_normal,
   mode_startup,
   mode_resolve
};

enum file_write_flag {
   file_write_no,
   file_write_double
};

enum interactivity_state {
   interactivity_database_init,
   interactivity_no_query_at_all,    /* Used when pasting from clipboard.  All subcalls,
                                        selectors, numbers, etc. must be filled in already.
                                        If not, it is a bug. */
   interactivity_verify,
   interactivity_normal,
   interactivity_picking
};

enum resolver_display_state {
   resolver_display_ok,
   resolver_display_searching,
   resolver_display_failed
};

enum resolve_goodness_test {
   resolve_goodness_only_nice,
   resolve_goodness_always,
   resolve_goodness_maybe
};

/* These enumerate the setups from which we can perform a "normalize" search. */
/* This list tracks the array "nice_setup_info". */

enum nice_start_kind {
   nice_start_4x4,
   nice_start_3x4,
   nice_start_2x8,
   nice_start_2x6,
   nice_start_1x10,
   nice_start_1x12,
   nice_start_1x14,
   nice_start_1x16,
   nice_start_3dmd,
   nice_start_4dmd,
   nice_start_4x6,
   NUM_NICE_START_KINDS   // End mark; not really in the enumeration.
};

struct nice_setup_thing {
   const useful_concept_enum *zzzfull_list;
   useful_concept_enum *zzzon_level_list;
   int full_list_size;
};

struct nice_setup_info_item {
   setup_kind kind;
   nice_setup_thing *thing;
   const useful_concept_enum *array_to_use_now;
   int number_available_now;
};

// Values returned by the various popup routines.
enum popup_return {
   POPUP_DECLINE = 0,
   POPUP_ACCEPT  = 1,
   POPUP_ACCEPT_WITH_STRING = 2
};







/* Flags that reside in the "result_flags" word of a setup AFTER a call is executed.

   The low two bits tell, for a 2x2 setup after
   having a call executed, how that 2x2 is elongated in the east-west or
   north-south direction.  Since 2x2's are always totally canonical, the
   interpretation of the elongation direction is always absolute.  A 1 in this
   field means the elongation is east-west.  A 2 means the elongation is
   north-south.  A zero means the elongation is unknown.  These bits will be set
   whenever we know which elongation is preferred.  This will be the case if the
   setup was an elongated 2x2 prior to the call (because they are the ends), or
   the setup was a 1x4 prior to the call.  So, for example, after any recycle
   from a wave, these bits will be set to indicate that the dancers want to
   elongate the 2x2 perpendicular to their original wave.  These bits will be
   looked at only if the call was directed to the ends (starting from a grand
   wave, for example.)  Furthermore, the bits will be overridden if the
   "concentric" or "checkpoint" concepts were used, or if the ends did this
   as part of a concentrically defined call for which some forcing modifier
   such as "force_columns" was used.  Note that these two bits are in the same
   format as the "cmd.prior_elongation" field of a setup before the call is
   executed.  In many case, that field can be simply copied into the "result_flags"
   field.

   RESULTFLAG__DID_LAST_PART means that, when a sequentially defined call was executed
   with the "cmd_frac_flags" word nonzero, so that just one part was done,
   that part was the last part.  Hence, if we are doing a call with some "piecewise"
   or "random" concept, we do the parts of the call one at a time, with appropriate
   concepts on each part, until it comes back with this flag set.  This is used with
   RESULTFLAG__PARTS_ARE_KNOWN.

   RESULTFLAG__PARTS_ARE_KNOWN means that the bit in RESULTFLAG__DID_LAST_PART is
   valid.  Sometimes we are unable to obtain information about parts, because
   no one is doing the call.  This can legally happen, for example, on "interlocked
   parallelogram leads shakedown while the trailers star thru" from waves.  In each
   virtual 2x2 setup one or the other of the calls is being done by no one.

   RESULTFLAG__EXPAND_TO_2X3 means that a call was executed that takes a four person
   starting setup (typically a line) and yields a 2x3 result setup.  Two of those
   result spots will be phantoms, of course.  When recombining a divided setup in
   which such a call was executed, if the recombination wuold yield a 2x6, and if
   the center 4 spots would be empty, this flag directs divided_setup_move to
   collapse the setup to a 2x4.  This is what makes calls like "pair the line"
   and "step and slide" behave correctly from parallel lines and from a grand line.

   RESULTFLAG__NEED_DIAMOND means that a call has been executed with the "magic" or
   "interlocked" modifier, and it really should be changed to the "magic diamond"
   concept.  Otherwise, we might end up saying "magic diamond single wheel" when
   we should have said "magic diamond, diamond single wheel".

   RESULTFLAG__ACTIVE_PHANTOMS_ON and _OFF tell whether the state of the "active
   phantoms" mode flag was used in this sequence.  If it was read and found to
   be on, RESULTFLAG__ACTIVE_PHANTOMS_ON is set.  If it was read and found to
   be off, RESULTFLAG__ACTIVE_PHANTOMS_OFF is set.  It is, of course, read any
   time an "assume <whatever> concept is invoked.  This information is used to
   tell what kind of annotation to place in the transcript file to tell what
   assumptions were made.  The reason for having both bits is that the state
   of the active phantoms mode can be toggled at arbitrary times, so a single
   card might have both usages.  In that unusual case, we want to so indicate
   on the card.

   RESULTFLAG__DID_SHORT6_2X3 means that we fudged a short6 to a 2x3
   (with permission), and, if we are putting back a concentric formation,
   we may need to act accordingly.

   The "split_info" field has info saying whether the call was split
   vertically (2) or horizontally (1) in "absolute space".  We can't say for
   sure whether the orientation is absolute, since a client may have stripped
   out rotation info.  What we mean is that it is relative to the incoming setup
   including the effect of its rotation field.  So, if the incoming setup was
   a 2x4 with rotation=0, that is, horizontally oriented in "absolute space",
   1 means it was done in 2x2's (split horizontally) and 2 means it was done
   in 1x4's (split vertically).  If the incoming setup was a 2x4 with
   rotation=1, that is, vertically oriented in "absolute space", 1 means it
   was done in 1x4's (split horizontally) and 2 means it was done in 2x2's
   (split vertically).  3 means that the call was a 1 or 2 person call, that
   could be split either way, so we have no information.
*/

// The two low bits are used for result elongation, so we start with 0x00000004.

enum {
   // This is a four bit field.
   RESULTFLAG__PART_COMPLETION_BITS = 0x0000003CUL,
   RESULTFLAG__DID_LAST_PART        = 0x00000004UL,
   RESULTFLAG__DID_NEXTTOLAST_PART  = 0x00000008UL,
   RESULTFLAG__SECONDARY_DONE       = 0x00000010UL,
   RESULTFLAG__PARTS_ARE_KNOWN      = 0x00000020UL,

   RESULTFLAG__NEED_DIAMOND         = 0x00000040UL,
   // Skipped 6 bits here
   RESULTFLAG__ACTIVE_PHANTOMS_ON   = 0x00002000UL,
   RESULTFLAG__ACTIVE_PHANTOMS_OFF  = 0x00004000UL,
   RESULTFLAG__EXPAND_TO_2X3        = 0x00008000UL,

   // This is a four bit field.
   RESULTFLAG__EXPIRATION_BITS      = 0x000F0000UL,
   RESULTFLAG__YOYO_EXPIRED         = 0x00010000UL,
   RESULTFLAG__TWISTED_EXPIRED      = 0x00020000UL,
   RESULTFLAG__SPLIT_EXPIRED        = 0x00040000UL,
   RESULTFLAG__EXPIRATION_ENAB      = 0x00080000UL,

   RESULTFLAG__DID_TGL_EXPANSION    = 0x00100000UL,
   RESULTFLAG__VERY_ENDS_ODD        = 0x00200000UL,
   RESULTFLAG__VERY_CTRS_ODD        = 0x00400000UL,

   // This is a 2 bit field.
   RESULTFLAG__DID_Z_COMPRESSMASK   = 0x01800000UL,
   RESULTFLAG__DID_Z_COMPRESSBIT    = 0x00800000UL,

   RESULTFLAG__NO_REEVALUATE        = 0x02000000UL,
   RESULTFLAG__IMPRECISE_ROT        = 0x04000000UL,
   RESULTFLAG__PLUSEIGHTH_ROT       = 0x08000000UL,
   RESULTFLAG__DID_SHORT6_2X3       = 0x10000000UL,
   RESULTFLAG__FORCE_SPOTS_ALWAYS   = 0x20000000UL,
   RESULTFLAG__INVADED_SPACE        = 0x40000000UL
};


struct comment_block {
   char txt[MAX_TEXT_LINE_LENGTH];
   comment_block *nxt;
};

// These bits appear in the "concparseflags" word.

// If the parse turns out to be ambiguous, don't use this one --
// yield to the other one.
#define CONCPARSE_YIELD_IF_AMB   0x00000002UL
// Parse directly.  It directs the parser to allow this concept
// (and similar concepts) and the following call to be typed
// on one line.  One needs to be very careful about avoiding
// ambiguity when setting this flag.
#define CONCPARSE_PARSE_DIRECT   0x00000004UL
// These are used by "print_recurse" in sdutil.cpp to control the printing.
// They govern the placement of commas.
#define CONCPARSE_PARSE_L_TYPE 0x8
#define CONCPARSE_PARSE_F_TYPE 0x10
#define CONCPARSE_PARSE_G_TYPE 0x20


// We need a forward reference; it's actually defined in sdgetout.
struct resolve_tester;

struct resolve_indicator {

   // Without this next thing, we get this error in Visual C++ Professional,
   // version 5.0, service pack 3:
   //      sdgetout.cpp
   //      C:\wba\sd\sdgetout.cpp(542) : fatal error C1001: INTERNAL COMPILER ERROR
   //        (compiler file 'E:\utc\src\\P2\main.c', line 379)
   //          Please choose the Technical Support command on the Visual C++
   //          Help menu, or open the Technical Support help file for more information
   //
   // Isn't that cool?
   //
   // Of course, if one were to take it out now, the error probably wouldn't
   // happen, because many other things have changed, and the error has a
   // serious phase-of-the-moon dependency.  But I have saved the files
   // that can provoke it.  July 2002.

   float ICantBelieveWhatABunchOfDunderheadsTheyHaveAtMicrosoft;

   // We only look at the "k" field and the 0x40 bit of the distance
   // (which tells us not to accept it in any resolve search.)
   // Note in particular that the low bits of the distance do *not*
   // contain useful information, because it's just a table item --
   // It doesn't know anything about the current setup and the
   // actual person identities.
   const resolve_tester *the_item;
   // Use this instead.
   int distance;
};

/* These flags go into the "concept_prop" field of a "concept_table_item".

   CONCPROP__SECOND_CALL means that the concept takes a second call, so a sublist must
      be created, with a pointer to same just after the concept pointer.

   CONCPROP__USE_SELECTOR means that the concept requires a selector, which must be
      inserted into the concept list just after the concept pointer.

   CONCPROP__NEEDK_4X4   mean that the concept requires the indicated setup, and, at
   CONCPROP__NEEDK_2X8   the top level, the existing setup should be expanded as needed.
      etc....

   CONCPROP__SET_PHANTOMS means that phantoms are in use under this concept, so that,
      when looking for tandems or couples, we shouldn't be disturbed if we
      pair someone with a phantom.  It is what makes "split phantom lines tandem"
      work, so that "split phantom lines phantom tandem" is unnecessary.

   CONCPROP__NO_STEP means that stepping to a wave or rearing back from one is not
      allowed under this concept.

   CONCPROP__GET_MASK means that tbonetest & livemask need to be computed before executing the concept.

   CONCPROP__STANDARD means that the concept can be "standard".

   CONCPROP__USE_NUMBER         Concept takes one number.
   CONCPROP__USE_TWO_NUMBERS    Concept takes two numbers.
   CONCPROP__USE_FOUR_NUMBERS   Etc.

   CONCPROP__SHOW_SPLIT means that the concept prepares the "split_axis" bits properly
      for transmission back to the client.  Normally this is off, and the split axis bits
      will be cleared after execution of the concept.

   CONCPROP__NEED_ARG2_MATRIX means that the "arg2" word of the concept_descriptor
      block contains additional bits of the "CONCPROP__NEEDK_3X8" kind to be sent to
      "do_matrix_expansion".  This is done so that concepts with different matrix
      expansion bits can share the same concept type.
*/



enum {
   CONCPROP__SECOND_CALL     = 0x00000001UL,
   CONCPROP__USE_SELECTOR    = 0x00000002UL,
   CONCPROP__SET_PHANTOMS    = 0x00000004UL,
   CONCPROP__NO_STEP         = 0x00000008UL,

   // This is a five bit field.  CONCPROP__NEED_LOBIT marks its low bit.
   // WARNING!!!  The values in this field are encoded into a bit field
   // for the setup expansion/normalization tables (see the definition
   // of "NEEDMASK".)  It follows that there can't be more than 32 of them.
   CONCPROP__NEED_MASK       = 0x000001F0UL,
   CONCPROP__NEED_LOBIT      = 0x00000010UL,
   CONCPROP__NEEDK_4X4       = 0x00000010UL,
   CONCPROP__NEEDK_2X8       = 0x00000020UL,
   CONCPROP__NEEDK_2X6       = 0x00000030UL,
   CONCPROP__NEEDK_4DMD      = 0x00000040UL,
   CONCPROP__NEEDK_BLOB      = 0x00000050UL,
   CONCPROP__NEEDK_4X6       = 0x00000060UL,
   CONCPROP__NEEDK_3X8       = 0x00000070UL,
   CONCPROP__NEEDK_3DMD      = 0x00000080UL,
   CONCPROP__NEEDK_1X10      = 0x00000090UL,
   CONCPROP__NEEDK_1X12      = 0x000000A0UL,
   CONCPROP__NEEDK_3X4       = 0x000000B0UL,
   CONCPROP__NEEDK_1X16      = 0x000000C0UL,
   CONCPROP__NEEDK_QUAD_1X4  = 0x000000D0UL,
   CONCPROP__NEEDK_TWINDMD   = 0x000000E0UL,
   CONCPROP__NEEDK_TWINQTAG  = 0x000000F0UL,
   CONCPROP__NEEDK_CTR_DMD   = 0x00000100UL,
   CONCPROP__NEEDK_END_DMD   = 0x00000110UL,
   CONCPROP__NEEDK_TRIPLE_1X4= 0x00000120UL,
   CONCPROP__NEEDK_CTR_1X4   = 0x00000130UL,
   CONCPROP__NEEDK_END_1X4   = 0x00000140UL,
   CONCPROP__NEEDK_CTR_2X2   = 0x00000150UL,
   CONCPROP__NEEDK_END_2X2   = 0x00000160UL,
   CONCPROP__NEEDK_3X4_D3X4  = 0x00000170UL,
   CONCPROP__NEEDK_3X6       = 0x00000180UL,
   CONCPROP__NEEDK_4D_4PTPD  = 0x00000190UL,
   CONCPROP__NEEDK_4X5       = 0x000001A0UL,
   CONCPROP__NEEDK_2X10      = 0x000001B0UL,
   CONCPROP__NEEDK_2X12      = 0x000001C0UL,
   CONCPROP__NEEDK_DBLX      = 0x000001D0UL,
   CONCPROP__NEEDK_DEEPXWV   = 0x000001E0UL,
   CONCPROP__NEEDK_QUAD_1X3  = 0x000001F0UL,

   CONCPROP__NEED_ARG2_MATRIX= 0x00000200UL,
   /* spare:                   0x00000400UL, */
   /* spare:                   0x00000800UL, */
   /* spare:                   0x00010000UL, */
   /* spare:                   0x00020000UL, */
   /* spare:                   0x00040000UL, */
   CONCPROP__IS_META         = 0x00080000UL,
   CONCPROP__GET_MASK        = 0x00100000UL,
   CONCPROP__STANDARD        = 0x00200000UL,
   CONCPROP__USE_NUMBER      = 0x00400000UL,
   CONCPROP__USE_TWO_NUMBERS = 0x00800000UL,
   CONCPROP__USE_FOUR_NUMBERS= 0x01000000UL,
   CONCPROP__MATRIX_OBLIVIOUS= 0x02000000UL,
   CONCPROP__PERMIT_MATRIX   = 0x04000000UL,
   CONCPROP__SHOW_SPLIT      = 0x08000000UL,
   CONCPROP__PERMIT_MYSTIC   = 0x10000000UL,
   CONCPROP__PERMIT_REVERSE  = 0x20000000UL,
   CONCPROP__PERMIT_MODIFIERS= 0x40000000UL
};


class warning_info {
 public:

   // Zero-arg constructor, clears the words.
   warning_info()
      { for (int i=0 ; i<WARNING_WORDS ; i++) bits[i] = 0; }

   bool operator != (const warning_info & rhs) const
      {
         for (int i=0 ; i<WARNING_WORDS ; i++) { if ((bits[i] != rhs.bits[i])) return true; }
         return false;
      }

   bool operator == (const warning_info & rhs) const
      {
         for (int i=0 ; i<WARNING_WORDS ; i++) { if ((bits[i] != rhs.bits[i])) return false; }
         return true;
      }

   void setbit(warning_index i)
      { bits[i>>5] |= 1 << (i & 0x1F); }

   void clearbit(warning_index i)
      { bits[i>>5] &= ~(1 << (i & 0x1F)); }

   bool testbit(warning_index i) const
      { return (bits[i>>5] & (1 << (i & 0x1F))) != 0; }

   bool testmultiple(const warning_info & rhs) const
      {
         for (int i=0 ; i<WARNING_WORDS ; i++) { if ((bits[i] & rhs.bits[i])) return true; }
         return false;
      }

   void setmultiple(const warning_info & rhs)
      { for (int i=0 ; i<WARNING_WORDS ; i++) bits[i] |= rhs.bits[i]; }

   void clearmultiple(const warning_info & rhs)
      { for (int i=0 ; i<WARNING_WORDS ; i++) bits[i] &= ~rhs.bits[i]; }

 private:
   enum { WARNING_WORDS = (warn__NUM_WARNINGS+31)>>5 };

   uint32 bits[WARNING_WORDS];
};

// A "configuration" is a state in the evolving sequence.
// There is a global array of these, making up the sequence itself.
// It is in the static array "history", running from 1 or 2 up to "history_ptr".

class configuration {
 public:
   parse_block *command_root;
   setup state;
   bool state_is_valid;
   bool draw_pic;
   int text_line;          // How many lines of text existed after this item was written,
                           // only meaningful if "written_history_items" is >= this index.

   static const resolve_tester *configuration::null_resolve_ptr;    /* in SDTOP */

 private:
   resolve_indicator resolve_flag;
   warning_info warnings;

   // This is the index into the "startinfolist".  It is only nonzero for history[1].
   // It shows how the sequence starts.
   int startinfoindex;

   // This constant table has useful info pertaining to the various nonzero
   // values that might be in "startinfoindex", e.g. sides start or heads 1P2P.
   static startinfo startinfolist[];                  // in SDTABLES

 public:

   // The sequence being written is in a global array "history".  The item
   // indexed by "history_ptr" is the "current" configuration, and the next higher
   // one is the "next" configuration.  Each configuration has:
   //   "state" -- a formation.  (While doing a call, the "cmd" part of the
   //              state will have the call do be done *from* that formation,
   //              which will be initialized from the *next higher* "command_root".)
   //   "command_root" -- the parse tree for the call that got *to* that formation.
   //   "warnings" -- the warnings, if any, that were raised by that call.
   //   "resolve_flag" -- the resolve info, if any, for that formation.  That is,
   //              the resolve *before* the call was executed.
   //
   // While we are working a call, the current configuration (indexed by
   //   "history_ptr", and given by "current_config") has the formation before
   //   doing the call.  Its "cmd" part has the actual call.  The next
   //   configuration (given by "next_config") has the call in its "command_root",
   //   and receives the result formation, the warnings that the call may have
   //   raised, and the resolve for that result.

   static configuration *history;                     // in SDTOP
   static int history_ptr;                            // in SDTOP
   static int whole_sequence_low_lim;                 // in SDTOP

   inline static configuration & current_config() { return history[history_ptr]; }
   inline static configuration & next_config() { return history[history_ptr+1]; }

   inline static int concepts_in_place()
      { return next_config().command_root != 0; }

   inline void init_centersp_specific() { startinfoindex = 0; }
   inline static void initialize_history(int c) {
      history_ptr = 1;
      history[1].startinfoindex = c;
      history[1].draw_pic = false;
      history[1].state_is_valid = false;
      whole_sequence_low_lim = (startinfolist[c].into_the_middle) ? 2 : 1;
   }
   inline bool nontrivial_startinfo_specific() { return startinfoindex != 0; }
   inline startinfo *get_startinfo_specific() { return &startinfolist[startinfoindex]; }
   inline void init_resolve() { resolve_flag.the_item = null_resolve_ptr; }
   void calculate_resolve();                          // in SDGETOUT
   inline static resolve_indicator & current_resolve() { return current_config().resolve_flag; }
   inline static resolve_indicator & next_resolve() { return next_config().resolve_flag; }
   static bool sequence_is_resolved();                // in SDGETOUT

   inline void restore_warnings_specific(const warning_info & rhs)
      { warnings = rhs; }
   inline void init_warnings_specific()
      { warnings = warning_info(); }
   inline void clear_one_warning_specific(warning_index i)
      { warnings.clearbit(i); }
   inline bool test_one_warning_specific(warning_index i) const { return warnings.testbit(i); }

   inline bool warnings_are_different(const configuration & rhs) const
      { return warnings != rhs.warnings; }

   inline static warning_info save_warnings()
      { return next_config().warnings; }
   inline static void restore_warnings(const warning_info & rhs)
      { next_config().warnings = rhs; }
   inline static void init_warnings()
      { next_config().warnings = warning_info(); }
   inline static void set_one_warning(warning_index i)
      { next_config().warnings.setbit(i); }
   inline static void clear_one_warning(warning_index i)
      { next_config().clear_one_warning_specific(i); }
   inline static void set_multiple_warnings(const warning_info & rhs)
      { next_config().warnings.setmultiple(rhs); }
   inline static void clear_multiple_warnings(const warning_info & rhs)
      { next_config().warnings.clearmultiple(rhs); }
   inline static bool test_multiple_warnings(const warning_info & rhs)
      { return next_config().warnings.testmultiple(rhs); }
};


struct concept_table_item{
   uint32 concept_prop;      /* Takes bits of the form CONCPROP__??? */
   // We wish we could put a "throw" clause on this function, but we can't.
   void (*concept_action)(setup *, parse_block *, setup *);
};


/* It should be noted that the CMD_MISC__??? and RESULTFLAG__XXX bits have
   nothing to do with each other.  It is not intended that
   the flags resulting from one call be passed to the next.  In fact, that
   would be incorrect.  The CMD_MISC__??? bits should start at zero at the
   beginning of each call, and accumulate stuff as the call goes deeper into
   recursion.  The RESULTFLAG__??? bits should, in general, be the OR of the
   bits of the components of a compound call, though this may not be so for
   the elongation bits at the bottom of the word. */



/* Meanings of the items in a "setup_command" structure:

   parseptr
      The full parse tree for the concept(s)/call(s) we are trying to do.
      While we traverse the big concepts in this tree, the "callspec" and
      "cmd_final_flags" are typically zero.  They only come into use when
      we reach the end of a subtree of big concepts, at which point we read
      the remaining "small" (or "final") concepts and the call name out of
      the end of the concept parse tree.

   callspec
      The call, after we reach the end of the parse tree.

   cmd_final_flags
      The various "final concept" flags with names INHERITFLAG_??? and FINAL__???.
      The INHERITFLAG_??? bits contain the final concepts like "single" that
      can be inherited from one part of a call definition to another.  The
      FINAL__??? bits contain other miscellaneous final concepts.

   cmd_misc_flags
      Other miscellaneous info controlling the execution of the call,
      with names like CMD_MISC__???.

   cmd_misc2_flags
      Even more miscellaneous info controlling the execution of the call,
      with names like CMD_MISC2__???.

   do_couples_heritflags
      These are "heritable" flags that tell what single/3x3/etc modifiers
      are to be used in determining what to do when CMD_MISC__DO_AS_COUPLES
      is specified.

   cmd_frac_flags
      If nonzero, fractionalization info controlling the execution of the call.
      See the comments in front of "get_fraction_info" in sdmoves.cpp for details.

   cmd_assume
      Any "assume waves" type command.

   skippable_concept
      For "<so-and-so> work <concept>", this is the concept to be skipped by some people.

   skippable_heritflags
      As above, when it's a simple modifier.

   prior_elongation_bits;
      This tells, for a 2x2 setup prior to having a call executed, how that
      2x2 is elongated (due to these people being the outsides) in the east-west
      or north-south direction.  Since 2x2's are always totally canonical, the
      interpretation of the elongation direction is always absolute.  A 1 means
      the elongation is east-west.  A 2 means the elongation is north-south.
      A zero means there was no elongation. */


/* Flags that reside in the "cmd_misc_flags" word of a setup BEFORE a call is executed.

   BEWARE!! These flags co-exist in the cmd_misc_flags word with copies of some
   of the call invocation flags.  The mask for those flags is
   DFM1_CONCENTRICITY_FLAG_MASK.  Those flags, and that mask, are defined
   in database.h .  We must define these flags starting where the concentricity
   flags end.

   CMD_MISC__EXPLICIT_MIRROR means that the setup has been mirrored by the "mirror"
   concept, separately from anything done by "left" or "reverse".  Such a mirroring
   does NOT cause the "take right hands" stuff to compensate by going to left hands,
   so it has the effect of making the people physically take left hands.

   CMD_MISC__MATRIX_CONCEPT means that the "matrix" concept has been given, and we have
   to be very careful about what we can do.  Specifically, this will make everything illegal
   except calls to suitable "split phantom" concepts.

   CMD_MISC__VERIFY_WAVES means that, before doing the call, we have to act as though the
   "assume waves" concept had been given.  That is, we have to verify that the setup is in fact
   waves, and we have to set the assumption stuff, so that a "with active phantoms" concept
   can be handled.  This flag is used only through "divided_setup_move", to propagate
   information from a concept like "split phantom waves" through the division, and then
   cause the assumption to be acted upon in each resulting setup.  It is removed immediately
   by "divided_setup_move" after use.

   CMD_MISC__EXPLICIT_MATRIX means that the caller said "4x4 matrix" or "2x6 matrix" or whatever,
   so we got to this matrix explicitly.  This enables natural splitting of the setup, e.g. form
   a parallelogram, "2x6 matrix 1x2 checkmate" is legal -- the 2x6 gets divided naturally
   into 2x3's.

   CMD_MISC__NO_EXPAND_MATRIX means that we are at a level of recursion that
   no longer permits us to do the implicit expansion of the matrix
   (e.g. add outboard phantoms to turn a 2x4 into a 2x6 if the concept
   "triple box" is used) that some concepts perform at the top level.

   CMD_MISC__DISTORTED means that we are at a level of recursion in which some
   distorted-setup concept has been used.  When this is set, "matrix" (a.k.a.
   "space invader") calls, such as press and truck, are not permitted.  Sorry, Clark.

   CMD_MISC__OFFSET_Z means that we are doing a 2x3 call (the only known case
   of this is "Z axle") that came from Z's picked out of a 4x4 setup.  The map
   that we are using has a "50% offset" map schema, so that, if the call goes
   into 1x4's (which Z axle does) it will put the results into a parallelogram,
   in accordance with customary usage.  The problem is that, if the call goes
   into a setup other than a 1x4 oriented in the appropriate way, using the
   50% offset schema will lead to the wrong result.  For example, if a call
   were defined that went from Z's into 2x2's, the program would attempt to
   bizarrely offset boxes.  (Yes, I know that Z axle is the only call in the
   database that can cause Z's to be picked out of a 4x4.  The program is just
   trying to be very careful.)  So, when this flag is on and a shape-changer
   occurs, the program insists on a parallelogram result.

   CMD_MISC__SAID_SPLIT means that we are at a level of recursion in which
   the "split" concept has been used.  When this is on, it is not permissible
   to do a 4-person call that has not 8-person definition, and hence would have
   been done the same way without the "split" concept.  This prevents superfluous
   things like "split pass thru".

   CMD_MISC__SAID_TRIANGLE means that we are at a level of recursion in which
   the word "triangle" has been uttered, so it is OK to do things like "triangle
   peel and trail" without giving the explicit "triangle" concept again.  This
   makes it possible to say things like "tandem-based triangles peel and trail".

   CMD_MISC__DO_AS_COUPLES means that the "couples_unless_single" invocation
   flag is on, and the call may need to be done as couples or whatever.  The
   bits in "do_couples_heritflags" control this.  If INHERITFLAG_SINGLE is on,
   do not do it as couples.  If off, do it as couples.  If various other bits
   are on (e.g. INHERITFLAG_1X3), do the appropriate thing.

   CMD_MISC__NO_CHK_ELONG means that the elongation of the incoming setup is for
   informational purposes only (to tell where people should finish) and should not
   be used for raising error messages.  It suppresses the error that would be
   raised if we have people in a facing 2x2 star thru when they are far from the
   ones they are facing.

   CMD_MISC__PHANTOMS means that we are at a level of recursion in which some phantom
   concept has been used, or when "on your own" or "so-and-so do your part" concepts
   are in use.  It indicates that we shouldn't be surprised if there are phantoms
   in the setup.  When this is set, the "tandem" or "as couples" concepts will
   forgive phantoms wherever they might appear, so that "split phantom lines tandem
   <call>" is legal.  Otherwise, we would have to say "split phantom lines phantom
   tandem <call>.  This flag also allows "basic_move" to be more creative about
   inferring the starting position for a call.  If I know that I am doing a
   "so-and-so do your part" operation, it may well be that, even though the setup
   is, say, point-to-point diamonds, the centers of the diamonds are none of my
   business (I am a point), and I want to think of the setup as a grand wave,
   so I can do my part of a grand mix.  The "CMD_MISC__PHANTOMS" flag makes it
   possible to do a grand mix from point-to-point diamonds in which the centers
   are absent.

   CMD_MISC__NO_STEP_TO_WAVE means that we are at a level of recursion that
   no longer permits us to do the implicit step to a wave or rear back from one
   that some calls permit at the top level.

   CMD_MISC__DOING_ENDS means that this call is directed only to the ends
   of the original setup.  If the call turns out to be an 8-person call with distinct
   centers and ends parts, we may want to just apply the ends part.  This is what
   makes "ends detour" work.
*/

// Since DFM1_CONCENTRICITY_FLAG_MASK is FF, we start at 100 hex.

enum {
   CMD_MISC__EXPLICIT_MIRROR      = 0x00000100UL,
   CMD_MISC__MATRIX_CONCEPT       = 0x00000200UL,
   // This is a 4 bit field.
   CMD_MISC__VERIFY_MASK          = 0x00003C00UL,
   // Here are the encodings that can go into same.
   CMD_MISC__VERIFY_WAVES         = 0x00000400UL,
   CMD_MISC__VERIFY_2FL           = 0x00000800UL,
   CMD_MISC__VERIFY_DMD_LIKE      = 0x00000C00UL,
   CMD_MISC__VERIFY_QTAG_LIKE     = 0x00001000UL,
   CMD_MISC__VERIFY_1_4_TAG       = 0x00001400UL,
   CMD_MISC__VERIFY_3_4_TAG       = 0x00001800UL,
   CMD_MISC__VERIFY_REAL_1_4_TAG  = 0x00001C00UL,
   CMD_MISC__VERIFY_REAL_3_4_TAG  = 0x00002000UL,
   CMD_MISC__VERIFY_REAL_1_4_LINE = 0x00002400UL,
   CMD_MISC__VERIFY_REAL_3_4_LINE = 0x00002800UL,
   CMD_MISC__VERIFY_LINES         = 0x00002C00UL,
   CMD_MISC__VERIFY_COLS          = 0x00003000UL,
   CMD_MISC__VERIFY_TALL6         = 0x00003400UL,

   CMD_MISC__EXPLICIT_MATRIX      = 0x00004000UL,

   CMD_MISC__NO_EXPAND_1          = 0x00008000UL,  // Allow only one triple box expansion.
   CMD_MISC__NO_EXPAND_2          = 0x00010000UL,  // Allow only one split phantom C/L/W expansion.
   CMD_MISC__NO_EXPAND_AT_ALL     = 0x00020000UL,  // Positively no expansion.
   CMD_MISC__NO_EXPAND_MATRIX = CMD_MISC__NO_EXPAND_1 | CMD_MISC__NO_EXPAND_2 | CMD_MISC__NO_EXPAND_AT_ALL,
   CMD_MISC__DISTORTED            = 0x00040000UL,
   CMD_MISC__OFFSET_Z             = 0x00080000UL,
   CMD_MISC__SAID_SPLIT           = 0x00100000UL,
   CMD_MISC__SAID_TRIANGLE        = 0x00200000UL,
   CMD_MISC__DO_AS_COUPLES        = 0x00400000UL,

   // spare:                        0x00800000UL,

   CMD_MISC__NO_CHECK_MOD_LEVEL   = 0x01000000UL,
   CMD_MISC__MUST_SPLIT_HORIZ     = 0x02000000UL,
   CMD_MISC__MUST_SPLIT_VERT      = 0x04000000UL,
   CMD_MISC__NO_CHK_ELONG         = 0x08000000UL,
   CMD_MISC__PHANTOMS             = 0x10000000UL,
   CMD_MISC__NO_STEP_TO_WAVE      = 0x20000000UL,
   CMD_MISC__ALREADY_STEPPED      = 0x40000000UL,
   CMD_MISC__DOING_ENDS           = 0x80000000UL,

   CMD_MISC__MUST_SPLIT_MASK      = (CMD_MISC__MUST_SPLIT_HORIZ|CMD_MISC__MUST_SPLIT_VERT)
};


// Flags that reside in the "cmd_misc3_flags" word of a setup BEFORE a call is executed.
enum {
   CMD_MISC3__PUT_FRAC_ON_FIRST    = 0x00000002UL,
   CMD_MISC3__RESTRAIN_CRAZINESS   = 0x00000004UL,
   CMD_MISC3__RESTRAIN_MODIFIERS   = 0x00000008UL,
};


// Flags that reside in the "cmd_misc2_flags" word of a setup BEFORE a call is executed.

// The following are used for the "<anyone> work <concept>" or "snag the <anyone>" mechanism:
//    CMD_MISC2__ANY_WORK or CMD_MISC2__ANY_SNAG is on if there is such an
//    operation in place.  We will make the centers or ends (depending on
//    CMD_MISC2__ANY_WORK_INVERT) use the next concept (or do 1/2 of the call)
//    while the others skip that concept.  The schema, which is one of
//    schema_concentric_2_6, schema_concentric_6_2, schema_concentric, or
//    schema_single_concentric, is in the low 16 bits.
//
//    CMD_MISC2__ANY_WORK_INVERT is only meaningful if the CMD_MISC2__ANY_WORK is on.
//    It says that the ends are doing the concept, instead of the centers.

// The following are used for Z's.
//    CMD_MISC2__IN_Z_CW and CMD_MISC2__IN_Z_CCW say that the setup is actually
//    a 2x3, but the "Z" (or "each Z", or "triple Z's") concept has been given,
//    and the setup should probably be turned into a 2x2.  The only exception
//    is if the call takes a 2x3 starting setup but not a 2x2 (that is, the call
//    is "Z axle").  In that case, the call is done directly in the 2x3, and the
//    "Z" distortion is presumed not to have been in place.

// The following are used for "mystic".
//
//    CMD_MISC2__MYSTIFY_SPLIT tells "divided_setup_move" to perform selective mirroring
//    of the subsidiary setups because a concept like "mystic triple boxes" is in use.
//    It is removed immediately by "divided_setup_move" after use.
//
//    CMD_MISC2__MYSTIFY_INVERT is only meaningful when CMD_MISC2__MYSTIFY_SPLIT is on.
//    It says that the concept is actually "invert mystic triple boxes" or whatever.

//  The following are used for what we call the "center/end" mechanism.  This
//    mechanism is used for the "invert" (centers and ends) concept, as well as
//    "central", "snag", and "mystic" and inverts thereof.
//
//    CMD_MISC2__CTR_END_KMASK, when nonzero, says that one of the "central", "snag",
//    or "mystic" concepts is in use.  They are all closely related.
//
//    CMD_MISC2__CTR_END_INVERT means that the call is being inverted.  It is
//    orthogonal to such issues as "snag" and "invert snag".  "Invert snag invert
//    strike out" means that the physical ends do only half, and everyone do
//    an invert strike out.  That is, the ends hinge but do not step & fold,
//    while the centers detour.
//
//    CMD_MISC2__CTR_END_MASK embraces all of the bits of the "center/end" mechanism.

// The low 12 bits are used for encoding the schema if
// CMD_MISC2__ANY_WORK or CMD_MISC2__ANY_SNAG is on.

enum {
   CMD_MISC2__IN_Z_CW           = 0x00001000UL,
   CMD_MISC2__IN_Z_CCW          = 0x00002000UL,
   CMD_MISC2__IN_AZ_CW          = 0x00004000UL,
   CMD_MISC2__IN_AZ_CCW         = 0x00008000UL,
   CMD_MISC2__IN_Z_MASK         = 0x0000F000UL,
   CMD_MISC2_RESTRAINED_SUPER   = 0x00010000UL,

   CMD_MISC2__MYSTIFY_SPLIT     = 0x00020000UL,
   CMD_MISC2__MYSTIFY_INVERT    = 0x00040000UL,

   CMD_MISC2__ANY_WORK          = 0x00080000UL,
   CMD_MISC2__ANY_SNAG          = 0x00100000UL,
   CMD_MISC2__ANY_WORK_INVERT   = 0x00200000UL,

   // Here are the inversion bits for the basic operations.
   CMD_MISC2__INVERT_CENTRAL    = 0x00400000UL,
   CMD_MISC2__INVERT_SNAG       = 0x00800000UL,
   CMD_MISC2__INVERT_MYSTIC     = 0x01000000UL,

   // Here are the basic operations we can do.
   CMD_MISC2__DO_CENTRAL        = 0x02000000UL,
   CMD_MISC2__CENTRAL_SNAG      = 0x04000000UL,
   CMD_MISC2__CENTRAL_MYSTIC    = 0x08000000UL,
   // This field embraces the above 3 bits.
   CMD_MISC2__CTR_END_KMASK     = 0x0E000000UL,

   // This says the the operator said "invert".  It might later cause
   // a "central" to be turned into an "invert central".
   CMD_MISC2__SAID_INVERT       = 0x10000000UL,

   // This mask embraces this whole mechanism, including the "invert" bit.
   CMD_MISC2__CTR_END_MASK      = 0x1FC00000UL,

   // This is a 2 bit field.
   CMD_MISC2__DID_Z_COMPRESSMASK= 0x60000000UL,
   CMD_MISC2__DID_Z_COMPRESSBIT = 0x20000000UL,
   CMD_MISC2__DO_NOT_EXECUTE    = 0x80000000UL
};

enum normalize_action {
   simple_normalize,
   normalize_after_exchange_boxes,
   normalize_before_isolated_call,
   normalize_before_isolate_not_too_strict,
   normalize_after_triple_squash,    // ***** this used to be after normalize_to_2.
   normalize_to_6,
   normalize_to_4,
   normalize_to_2,
   normalize_after_disconnected,
   normalize_before_merge,
   normalize_strict_matrix,
   normalize_compress_bigdmd,
   normalize_recenter,
   normalize_never
};

// Beware!  There are >= tests lying around, so order is important.
// In particular, sdconc (search for "brute_force_merge" has such tests.
enum merge_action {
   merge_strict_matrix,
   merge_strict_matrix_but_colliding_merge,
   merge_c1_phantom,
   merge_c1_phantom_real,
   merge_after_dyp,
   merge_without_gaps
};


class expand {

 public:

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "init_table", and many other items, be statically
   // initialized arrays.

   struct thing {
      veryshort source_indices[24];
      int size;
      setup_kind inner_kind;
      setup_kind outer_kind;
      int rot;
      uint32 lillivemask;
      uint32 biglivemask;
      warning_index expwarning;
      warning_index norwarning;
      normalize_action action_level;
      uint32 expandconcpropmask;
      thing *next_expand;
      thing *next_compress;
   };

   static void initialize();

   static void compress_setup(const thing *thing, setup *stuff) THROW_DECL;

   static void expand_setup(const thing *thing, setup *stuff) THROW_DECL;

   static bool compress_from_hash_table(setup *ss,
                                        normalize_action action,
                                        uint32 livemask,
                                        bool noqtagcompress) THROW_DECL;

   static bool expand_from_hash_table(setup *ss,
                                      uint32 needpropbits,
                                      uint32 livemask) THROW_DECL;

 private:

   // Must be a power of 2.
   enum { NUM_EXPAND_HASH_BUCKETS = 32 };

   static thing *expand_hash_table[NUM_EXPAND_HASH_BUCKETS];
   static thing *compress_hash_table[NUM_EXPAND_HASH_BUCKETS];

   static thing init_table[];
};

class full_expand {

 public:

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make a number of items be statically initialized arrays.

   struct thing {
      warning_index warning;
      // Low 2 bits = elongation bits to forbid;
      // "4" bit = must set elongation.
      // Also, the "8" bit means to use "gather" and do this the other way.
      // Also, the "16" bit means allow only step to a box, not step to a full wave.
      // Also, the "32" bit means to give "some people touch" warning if this is
      //     a "step_to_wave_4_people" call.
      // Also, the "64" bit means this is an item relating to having just the centers
      // touch to a miniwave, and we may need to give the "warn__some_touch_evil"
      // warning.
      // Also, the "128" bit means this is an item that lets you do a swing thru
      // from the partially occupied 2x4 that would result from everyone doing a
      // 1/2 press back from a right-hand wave.  We don't allow the touch to a wave
      // if the user gave a phantom concept.
      int forbidden_elongation;
      expand::thing *expand_lists;
      setup_kind kind;
      uint32 live;
      uint32 dir;
      uint32 dirmask;
      thing *next;
   };

   static void initialize_touch_tables();
   static thing *search_table_1(setup_kind kind, uint32 livemask, uint32 directions);
   static thing *search_table_2(setup_kind kind, uint32 livemask, uint32 directions);
   static thing *search_table_3(setup_kind kind,
                                uint32 livemask, uint32 directions, uint32 touchflags);

 private:

   // Must be a power of 2.
   enum { NUM_TOUCH_HASH_BUCKETS = 32 };

   static thing *touch_hash_table1[NUM_TOUCH_HASH_BUCKETS];
   static thing *touch_hash_table2[NUM_TOUCH_HASH_BUCKETS];
   static thing *touch_hash_table3[NUM_TOUCH_HASH_BUCKETS];
};


enum phantest_kind {
   /* These control error messages that arise when we divide a setup
      into subsetups (e.g. phantom lines) and find that one of
      the setups is empty.  For example, if we are in normal lines
      and select "phantom lines", it will notice that one of the setups
      (the outer one) isn't occupied and will give the error message
      "Don't use phantom concept if you don't mean it." */

   phantest_ok,               // Anything goes
   phantest_impossible,       // Can't happen in symmetric stuff??
   phantest_both,             // Require both setups (partially) occupied
   phantest_only_one,         // Require only one setup occupied, don't care which
   phantest_only_first_one,   // Require only first setup occupied, second empty
   phantest_only_second_one,  // Require only second setup occupied, first empty
   phantest_only_one_pair,    // Require only first 2 (of 4) or only last 2
   phantest_first_or_both,    // Require first setup only, or a mixture
   phantest_ctr_phantom_line, // Special, created when outside phantom setup is empty.
   phantest_2x2_both,
   phantest_2x2_only_two,
   phantest_not_just_centers
};

enum disttest_kind {
   disttest_t, disttest_nil, disttest_only_two,
   disttest_any, disttest_offset, disttest_z};


enum restriction_test_result {
   restriction_passes,
   restriction_fails,
   restriction_bad_level,
   restriction_no_item
};

struct concept_fixer_thing {
   uint32 newheritmods;
   uint32 newfinalmods;
   useful_concept_enum before;
   useful_concept_enum after;
};

enum selective_key {
   // Warning!!!!  Order is important!  See all the stupid ways these are used
   // in sdconc.cpp.
   selective_key_dyp,
   selective_key_own,
   selective_key_plain_no_live_subsets,  // there is an
                                         // "indicator < selective_key_plain_no_live_subsets"
   selective_key_plain,       // there is an "indicator >= selective_key_plain"
   selective_key_disc_dist,   // there is an "indicator < selective_key_disc_dist"
                              // and an "indicator >= selective_key_disc_dist"
   selective_key_ignore,
   selective_key_work_concept,
   selective_key_lead_for_a,
   selective_key_work_no_concentric,
   selective_key_snag_anyone,
   selective_key_plain_from_id_bits,
   selective_key_mini_but_o
};

enum tandem_key {
   tandem_key_tand = 0,
   tandem_key_cpls = 1,
   tandem_key_siam = 2,
   tandem_key_tand3 = 4,
   tandem_key_cpls3 = 5,
   tandem_key_siam3 = 6,
   tandem_key_tand4 = 8,
   tandem_key_cpls4 = 9,
   tandem_key_siam4 = 10,
   tandem_key_box = 16,
   tandem_key_diamond = 17,
   tandem_key_skew = 18,
   tandem_key_outpoint_tgls = 20,
   tandem_key_inpoint_tgls = 21,
   tandem_key_inside_tgls = 22,
   tandem_key_outside_tgls = 23,
   tandem_key_wave_tgls = 26,
   tandem_key_tand_tgls = 27,
   tandem_key_anyone_tgls = 30,
   tandem_key_3x1tgls = 31,
   tandem_key_ys = 32
};

enum meta_key_kind {
   meta_key_random,
   meta_key_rev_random,   // Must follow meta_key_random.
   meta_key_piecewise,
   meta_key_initially,
   meta_key_finish,
   meta_key_revorder,
   meta_key_like_a,
   meta_key_finally,
   meta_key_initially_and_finally,
   meta_key_nth_part_work,
   meta_key_first_frac_work,
   meta_key_skip_nth_part,
   meta_key_shift_n,
   meta_key_echo,
   meta_key_rev_echo,   // Must follow meta_key_echo.
   meta_key_shift_half
};


// These are the "meta_key_props" bits.
enum mkprop {
   MKP_RESTRAIN_1    = 0x00000001UL,
   MKP_RESTRAIN_2    = 0x00000002UL,
   MKP_COMMA_NEXT    = 0x00000004UL
};

extern const uint32 meta_key_props[];    // In sdtables.

enum revert_weirdness_type {
   weirdness_off,
   weirdness_flatten_from_3,
   weirdness_otherstuff
};


enum split_command_kind {
   split_command_none,
   split_command_1x4,
   split_command_1x8,
   split_command_2x3,
};



extern SDLIB_API int session_index;                           // in SDSI
extern SDLIB_API bool rewrite_with_new_style_filename;        // in SDSI
extern int random_number;                                     // in SDSI
extern SDLIB_API char *database_filename;                     // in SDSI
extern SDLIB_API char *new_outfile_string;                    // in SDSI
extern SDLIB_API char abridge_filename[MAX_TEXT_LINE_LENGTH]; // in SDSI

extern SDLIB_API bool showing_has_stopped;                    // in SDMATCH
extern SDLIB_API match_result GLOB_match;                     // in SDMATCH
extern SDLIB_API bool GLOB_space_ok;                          // in SDMATCH
extern SDLIB_API int GLOB_yielding_matches;                   // in SDMATCH
extern SDLIB_API char GLOB_user_input[];                      // in SDMATCH
extern SDLIB_API char GLOB_full_extension[];                  // in SDMATCH
extern SDLIB_API char GLOB_echo_stuff[];                      // in SDMATCH
extern SDLIB_API int GLOB_user_input_size;                    // in SDMATCH

extern SDLIB_API short int *concept_list;        /* indices of all concepts */
extern SDLIB_API int concept_list_length;
extern SDLIB_API short int *level_concept_list; /* indices of concepts valid at current level */
extern SDLIB_API int level_concept_list_length;

extern SDLIB_API modifier_block *fcn_key_table_normal[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern SDLIB_API modifier_block *fcn_key_table_start[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern SDLIB_API modifier_block *fcn_key_table_resolve[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern SDLIB_API abbrev_block *abbrev_table_normal;
extern SDLIB_API abbrev_block *abbrev_table_start;
extern SDLIB_API abbrev_block *abbrev_table_resolve;

extern SDLIB_API match_result user_match;


extern SDLIB_API command_kind search_goal;                          /* in SDPICK */

extern SDLIB_API Cstring menu_names[];                              /* in SDMAIN */
extern SDLIB_API command_list_menu_item command_menu[];             /* in SDMAIN */
extern SDLIB_API resolve_list_menu_item resolve_menu[];             /* in SDMAIN */
extern SDLIB_API startup_list_menu_item startup_menu[];             /* in SDMAIN */
extern int last_file_position;                                      /* in SDMAIN */
extern SDLIB_API char *sd_version_string();                         /* In SDMAIN */
extern SDLIB_API bool query_for_call();                             /* In SDMAIN */

extern int sdtty_screen_height;                                     /* in SDUI-TTY */
extern bool sdtty_no_cursor;                                        /* in SDUI-TTY */
extern bool sdtty_no_console;                                       /* in SDUI-TTY */
extern bool sdtty_no_line_delete;                                   /* in SDUI-TTY */


// During initialization, the main program makes a number of callbacks
// to the user interface stuff, through procedure "init_step".
// This is done to do things like put up and take down dialog boxes,
// change the status bar, and manipulate the progress bar.
// The first argument is one of these keys.  They are listed in the
// order in which they occur.  A few of them take a second argument.

enum init_callback_state {
   get_session_info,   // Query user about the session.
   final_level_query,  // Maybe query the user for the level --
                       // didn't get it from the command line or the session.
   init_database1,     // Got level, about to open database.
   init_database2,     // Starting the big database scan to create menus.
   calibrate_tick,     // Takes arg, calibrate the progress bar.
   do_tick,            // Takes arg, called repeatedly to advance the progress bar.
   tick_end,           // End the progress bar.
   do_accelerator      // Starting the processing of accelerator keys.
};


class iobase {
 public:
   virtual int do_abort_popup() = 0;
   virtual uims_reply get_startup_command() = 0;
   virtual void set_window_title(char s[]) = 0;
   virtual void add_new_line(const char the_line[], uint32 drawing_picture) = 0;
   virtual void no_erase_before_n(int n) = 0;
   virtual void reduce_line_count(int n) = 0;
   virtual void update_resolve_menu(command_kind goal, int cur, int max,
                                    resolver_display_state state) = 0;
   virtual void show_match() = 0;
   virtual char *version_string() = 0;
   virtual uims_reply get_resolve_command() = 0;
   virtual bool choose_font() = 0;
   virtual bool print_this() = 0;
   virtual bool print_any() = 0;
   virtual bool help_manual() = 0;
   virtual bool help_faq() = 0;
   virtual popup_return do_outfile_popup(char dest[]) = 0;
   virtual popup_return do_header_popup(char dest[]) = 0;
   virtual popup_return do_getout_popup(char dest[]) = 0;
   virtual void fatal_error_exit(int code, Cstring s1=0, Cstring s2=0) = 0;
   virtual void serious_error_print(Cstring s1) = 0;
   virtual void create_menu(call_list_kind cl) = 0;
   virtual int do_selector_popup() = 0;
   virtual int do_direction_popup() = 0;
   virtual int do_circcer_popup() = 0;
   virtual int do_tagger_popup(int tagger_class) = 0;
   virtual int yesnoconfirm(char *title, char *line1, char *line2, bool excl, bool info) = 0;
   virtual popup_return do_comment_popup(char dest[]) = 0;
   virtual uint32 get_number_fields(int nnumbers, bool odd_number_only, bool forbid_zero) = 0;
   virtual bool get_call_command(uims_reply *reply_p) = 0;
   virtual void set_pick_string(const char *string) = 0;
   virtual void display_help() = 0;
   virtual void terminate(int code) = 0;
   virtual void process_command_line(int *argcp, char ***argvp) = 0;
   virtual void bad_argument(Cstring s1, Cstring s2, Cstring s3) = 0;
   virtual void final_initialize() = 0;
   virtual bool init_step(init_callback_state s, int n) = 0;
};

class iofull : public iobase {
 public:
   int do_abort_popup();
   uims_reply get_startup_command();
   void set_window_title(char s[]);
   void add_new_line(const char the_line[], uint32 drawing_picture);
   void no_erase_before_n(int n);
   void reduce_line_count(int n);
   void update_resolve_menu(command_kind goal, int cur, int max,
                            resolver_display_state state);
   void show_match();
   char *version_string();
   uims_reply get_resolve_command();
   bool choose_font();
   bool print_this();
   bool print_any();
   bool help_manual();
   bool help_faq();
   popup_return do_outfile_popup(char dest[]);
   popup_return do_header_popup(char dest[]);
   popup_return do_getout_popup(char dest[]);
   void fatal_error_exit(int code, Cstring s1=0, Cstring s2=0);
   void serious_error_print(Cstring s1);
   void create_menu(call_list_kind cl);
   int do_selector_popup();
   int do_direction_popup();
   int do_circcer_popup();
   int do_tagger_popup(int tagger_class);
   int yesnoconfirm(char *title, char *line1, char *line2, bool excl, bool info);
   void set_pick_string(const char *string);
   popup_return do_comment_popup(char dest[]);
   uint32 get_number_fields(int nnumbers, bool odd_number_only, bool forbid_zero);
   bool get_call_command(uims_reply *reply_p);
   void display_help();
   void terminate(int code);
   void process_command_line(int *argcp, char ***argvp);
   void bad_argument(Cstring s1, Cstring s2, Cstring s3);
   void final_initialize();
   bool init_step(init_callback_state s, int n);
};


/* VARIABLES */

extern SDLIB_API iobase *gg;                                        /* in SDTOP */
extern SDLIB_API int text_line_count;                               /* in SDTOP */
extern SDLIB_API char error_message1[MAX_ERR_LENGTH];               /* in SDTOP */
extern SDLIB_API char error_message2[MAX_ERR_LENGTH];               /* in SDTOP */
extern SDLIB_API uint32 collision_person1;                          /* in SDTOP */
extern SDLIB_API uint32 collision_person2;                          /* in SDTOP */
extern SDLIB_API int history_allocation;                            /* in SDTOP */
extern SDLIB_API int written_history_items;                         /* in SDTOP */
extern SDLIB_API int no_erase_before_this;                          /* in SDTOP */
extern SDLIB_API int written_history_nopic;                         /* in SDTOP */
extern SDLIB_API dance_level higher_acceptable_level[];             /* in SDTOP */
extern SDLIB_API uint32 the_topcallflags;                           /* in SDTOP */
extern SDLIB_API bool there_is_a_call;                              /* in SDTOP */

extern SDLIB_API call_with_name **base_calls;                       /* in SDTOP */
extern SDLIB_API ui_option_type ui_options;                         /* in SDTOP */
extern SDLIB_API bool enable_file_writing;                          /* in SDTOP */
extern SDLIB_API Cstring cardinals[NUM_CARDINALS+1];                /* in SDTOP */
extern SDLIB_API Cstring ordinals[NUM_CARDINALS+1];                 /* in SDTOP */
extern SDLIB_API Cstring direction_names[];                         /* in SDTOP */
extern SDLIB_API Cstring getout_strings[];                          /* in SDTOP */
extern SDLIB_API writechar_block_type writechar_block;              /* in SDTOP */
extern SDLIB_API int num_command_commands;                          /* in SDTOP */
extern SDLIB_API Cstring *command_commands;                         /* in SDTOP */
extern SDLIB_API command_kind *command_command_values;              /* in SDTOP */
extern SDLIB_API int num_startup_commands;                          /* in SDTOP */
extern SDLIB_API Cstring *startup_commands;                         /* in SDTOP */
extern SDLIB_API start_select_kind *startup_command_values;         /* in SDTOP */
extern SDLIB_API int number_of_resolve_commands;                    /* in SDTOP */
extern SDLIB_API Cstring* resolve_command_strings;                  /* in SDTOP */
extern SDLIB_API resolve_command_kind *resolve_command_values;      /* in SDTOP */
extern SDLIB_API abridge_mode_t glob_abridge_mode;                  /* in SDTOP */
extern SDLIB_API int abs_max_calls;                                 /* in SDTOP */
extern SDLIB_API int max_base_calls;                                /* in SDTOP */
extern SDLIB_API Cstring *tagger_menu_list[NUM_TAGGER_CLASSES];     /* in SDTOP */
extern SDLIB_API Cstring *selector_menu_list;                       /* in SDTOP */
extern SDLIB_API Cstring *circcer_menu_list;                        /* in SDTOP */


extern SDLIB_API call_with_name **tagger_calls[NUM_TAGGER_CLASSES]; /* in SDTOP */
extern SDLIB_API call_with_name **circcer_calls;                    /* in SDTOP */
extern SDLIB_API uint32 number_of_taggers[NUM_TAGGER_CLASSES];      /* in SDTOP */
extern SDLIB_API uint32 number_of_circcers;                         /* in SDTOP */
extern SDLIB_API parse_state_type parse_state;                      /* in SDTOP */
extern SDLIB_API call_conc_option_state current_options;            /* in SDTOP */
extern SDLIB_API bool allowing_all_concepts;                        /* in SDTOP */
extern SDLIB_API bool allowing_minigrand;                           /* in SDTOP */
extern SDLIB_API bool using_active_phantoms;                        /* in SDTOP */
extern SDLIB_API const call_conc_option_state null_options;         /* in SDTOP */
extern SDLIB_API call_conc_option_state verify_options;             /* in SDTOP */
extern SDLIB_API bool verify_used_number;                           /* in SDTOP */
extern SDLIB_API bool verify_used_direction;                        /* in SDTOP */
extern SDLIB_API bool verify_used_selector;                         /* in SDTOP */
extern SDLIB_API int uims_menu_index;                               /* in SDTOP */
extern SDLIB_API int last_direction_kind;                           /* in SDTOP */
extern SDLIB_API interactivity_state interactivity;                 /* in SDTOP */
extern SDLIB_API char database_version[81];                         /* in SDTOP */
extern SDLIB_API bool testing_fidelity;                             /* in SDTOP */
extern SDLIB_API dance_level level_threshholds[];                   /* in SDTOP */
extern SDLIB_API int allowing_modifications;                        /* in SDTOP */
extern SDLIB_API int hashed_randoms;                                /* in SDTOP */

extern int useful_concept_indices[UC_extent];                       /* in SDINIT */
extern selector_kind selector_for_initialize;                       /* in SDINIT */
extern direction_kind direction_for_initialize;                     /* in SDINIT */
extern int number_for_initialize;                                   /* in SDINIT */
extern SDLIB_API int *color_index_list;                             /* in SDINIT */

extern SDLIB_API error_flag_type global_error_flag;                 /* in SDUTIL */
extern SDLIB_API bool global_cache_failed_flag;                     /* in SDUTIL */
extern SDLIB_API int global_cache_miss_reason[3];                   /* in SDUTIL */
extern SDLIB_API uims_reply global_reply;                           /* in SDUTIL */
extern SDLIB_API int global_age;                                    /* in SDUTIL */
extern configuration *clipboard;                                    /* in SDUTIL */
extern int clipboard_size;                                          /* in SDUTIL */
extern SDLIB_API bool wrote_a_sequence;                             /* in SDUTIL */
extern bool retain_after_error;                                     /* in SDUTIL */
extern SDLIB_API char outfile_string[MAX_FILENAME_LENGTH];          /* in SDUTIL */
extern SDLIB_API char header_comment[MAX_TEXT_LINE_LENGTH];         /* in SDUTIL */
extern SDLIB_API bool creating_new_session;                         /* in SDUTIL */
extern SDLIB_API int sequence_number;                               /* in SDUTIL */
extern SDLIB_API int starting_sequence_number;                      /* in SDUTIL */
extern SDLIB_API const Cstring old_filename_strings[];              /* in SDUTIL */
extern SDLIB_API const Cstring new_filename_strings[];              /* in SDUTIL */
extern SDLIB_API const Cstring *filename_strings;                   /* in SDUTIL */
extern SDLIB_API const Cstring concept_key_table[];                 /* in SDUTIL */

extern SDLIB_API const concept_table_item concept_table[];          /* in SDCONCPT */
extern uint32 global_tbonetest;                                     /* in SDCONCPT */
extern uint32 global_livemask;                                      /* in SDCONCPT */
extern uint32 global_selectmask;                                    /* in SDCONCPT */
extern uint32 global_tboneselect;                                   /* in SDCONCPT */

extern parse_block *last_magic_diamond;                             /* in SDTOP */
extern warning_info no_search_warnings;                             /* in SDTOP */
extern warning_info conc_elong_warnings;                            /* in SDTOP */
extern warning_info dyp_each_warnings;                              /* in SDTOP */
extern warning_info useless_phan_clw_warnings;                      /* in SDTOP */
extern int concept_sublist_sizes[call_list_extent];                 /* in SDTOP */
extern short int *concept_sublists[call_list_extent];               /* in SDTOP */
extern int good_concept_sublist_sizes[call_list_extent];            /* in SDTOP */
extern short int *good_concept_sublists[call_list_extent];          /* in SDTOP */

extern bool selector_used;                                          /* in SDPREDS */
extern bool direction_used;                                         /* in SDPREDS */
extern bool number_used;                                            /* in SDPREDS */
extern bool mandatory_call_used;                                    /* in SDPREDS */
extern predicate_descriptor pred_table[];                           /* in SDPREDS */
extern int selector_preds;                                          /* in SDPREDS */


extern const ctr_end_mask_rec dead_masks;                 /* in SDTABLES */
extern const ctr_end_mask_rec masks_for_3x4;              /* in SDTABLES */
extern const ctr_end_mask_rec masks_for_3dmd_ctr2;        /* in SDTABLES */
extern const ctr_end_mask_rec masks_for_3dmd_ctr4;        /* in SDTABLES */
extern const ctr_end_mask_rec masks_for_bigh_ctr4;        /* in SDTABLES */
extern const ctr_end_mask_rec masks_for_4x4;              /* in SDTABLES */
extern int begin_sizes[];                                 /* in SDTABLES */

extern const coordrec tgl3_0;
extern const coordrec tgl3_1;
extern const coordrec tgl4_0;
extern const coordrec tgl4_1;
extern const coordrec squeezethingglass;
extern const coordrec squeezethinggal;
extern const coordrec squeezethingqtag;
extern const coordrec squeezething4dmd;
extern const coordrec squeezefinalglass;
extern const coordrec truck_to_ptpd;
extern const coordrec truck_to_deepxwv;
extern const coordrec press_4dmd_4x4;
extern const coordrec press_4dmd_qtag1;
extern const coordrec press_4dmd_qtag2;
extern const coordrec press_qtag_4dmd1;
extern const coordrec press_qtag_4dmd2;
extern const coordrec acc_crosswave;

extern id_bit_table id_bit_table_2x5_z[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_2x6_pg[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_bigdmd_wings[];                    /* in SDTABLES */
extern id_bit_table id_bit_table_bigbone_wings[];                   /* in SDTABLES */
extern id_bit_table id_bit_table_bighrgl_wings[];                   /* in SDTABLES */
extern id_bit_table id_bit_table_bigdhrgl_wings[];                  /* in SDTABLES */
extern id_bit_table id_bit_table_bigh_dblbent[];                    /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_h[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_ctr6[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_offset[];                      /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_corners[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_zs[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_butterfly[];                       /* in SDTABLES */
extern id_bit_table id_bit_table_4x4_outer_pairs[];                 /* in SDTABLES */
extern id_bit_table id_bit_table_525_nw[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_525_ne[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_343_outr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_343_innr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_545_outr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_545_innr[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_3dmd_in_out[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_3dmd_ctr1x6[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_3dmd_ctr1x4[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_4dmd_cc_ee[];                      /* in SDTABLES */
extern id_bit_table id_bit_table_3ptpd[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_wqtag_hollow[];                    /* in SDTABLES */
extern id_bit_table id_bit_table_3x6_with_1x6[];                    /* in SDTABLES */
extern const expand::thing s_qtg_2x4;
extern const expand::thing s_4x4_4x6a;
extern const expand::thing s_4x4_4x6b;
extern const expand::thing s_4x4_4dma;
extern const expand::thing s_4x4_4dmb;
extern const expand::thing s_c1phan_4x4a;
extern const expand::thing s_c1phan_4x4b;
extern const expand::thing s_1x4_dmd;
extern const expand::thing s_qtg_3x4;
extern const expand::thing s_short6_2x3;
extern const expand::thing s_bigrig_dblbone;
extern const expand::thing s_bigbone_dblrig;
extern const veryshort identity24[24];
extern full_expand::thing rear_1x2_pair;
extern full_expand::thing rear_2x2_pair;
extern full_expand::thing rear_bone_pair;
extern full_expand::thing step_8ch_pair;
extern full_expand::thing step_qtag_pair;
extern full_expand::thing step_2x2h_pair;
extern full_expand::thing step_2x2v_pair;
extern full_expand::thing step_spindle_pair;
extern full_expand::thing step_dmd_pair;
extern full_expand::thing step_tgl_pair;
extern full_expand::thing step_ptpd_pair;
extern full_expand::thing step_qtgctr_pair;

extern full_expand::thing touch_init_table1[];
extern full_expand::thing touch_init_table2[];
extern full_expand::thing touch_init_table3[];


inline uint32 get_meta_key_props(const conzept::concept_descriptor *this_concept)
{
   if (concept_table[this_concept->kind].concept_prop & CONCPROP__IS_META)
      return meta_key_props[this_concept->arg1];
   else
      return 0;
}


#define NEEDMASK(K) (1<<(((uint32) (K))/((uint32) CONCPROP__NEED_LOBIT)))

enum mpkind {
   MPKIND__NONE,
   MPKIND__SPLIT,
   MPKIND__SPLIT_OTHERWAY_TOO,
   MPKIND__REMOVED,
   MPKIND__TWICE_REMOVED,
   MPKIND__THRICE_REMOVED,
   MPKIND__OVERLAP,
   MPKIND__OVERLAP14,
   MPKIND__OVERLAP34,
   MPKIND__SPEC_MATRIX_OVERLAP,
   MPKIND__INTLK,
   MPKIND__CONCPHAN,
   MPKIND__INTLKDMD,
   MPKIND__MAGICDMD,
   MPKIND__MAGICINTLKDMD,
   MPKIND__NONISOTROPIC,
   MPKIND__NONISOTROP1,
   MPKIND__NONISOTROP2,
   MPKIND__NONISOTROPREM,
   MPKIND__OFFS_L_ONEQ,
   MPKIND__OFFS_R_ONEQ,
   MPKIND__OFFS_L_HALF,
   MPKIND__OFFS_R_HALF,
   MPKIND__OFFS_L_HALF_NONISO,
   MPKIND__OFFS_R_HALF_NONISO,
   MPKIND__OFFS_L_THRQ,
   MPKIND__OFFS_R_THRQ,
   MPKIND__OFFS_L_FULL,
   MPKIND__OFFS_R_FULL,
   MPKIND__OFFS_L_ONEQ_SPECIAL,
   MPKIND__OFFS_R_ONEQ_SPECIAL,
   MPKIND__OFFS_L_HALF_SPECIAL,
   MPKIND__OFFS_R_HALF_SPECIAL,
   MPKIND__OFFS_L_THRQ_SPECIAL,
   MPKIND__OFFS_R_THRQ_SPECIAL,
   MPKIND__OFFS_L_FULL_SPECIAL,
   MPKIND__OFFS_R_FULL_SPECIAL,
   MPKIND__OFFS_BOTH_FULL,
   MPKIND__OFFS_BOTH_SINGLEV,
   MPKIND__OFFS_BOTH_SINGLEH,
   MPKIND__LILZCCW,
   MPKIND__LILZCW,
   MPKIND__LILAZCCW,
   MPKIND__LILAZCW,
   MPKIND__LILZCOM,
   MPKIND__O_SPOTS,
   MPKIND__X_SPOTS,
   MPKIND__4_QUADRANTS,
   MPKIND__4_EDGES,
   MPKIND__ALL_8,
   MPKIND__DMD_STUFF,
   MPKIND__STAG,
   MPKIND__DIAGQTAG,
   MPKIND__DIAGQTAG4X6,
   MPKIND__BENT0CW,
   MPKIND__BENT0CCW,
   MPKIND__BENT1CW,
   MPKIND__BENT1CCW,
   MPKIND__BENT2CW,
   MPKIND__BENT2CCW,
   MPKIND__BENT3CW,
   MPKIND__BENT3CCW,
   MPKIND__BENT4CW,
   MPKIND__BENT4CCW,
   MPKIND__BENT5CW,
   MPKIND__BENT5CCW,
   MPKIND__BENT6CW,
   MPKIND__BENT6CCW,
   MPKIND__BENT7CW,
   MPKIND__BENT7CCW,
   MPKIND__SPEC_ONCEREM,
   MPKIND__SPEC_TWICEREM,
   NUM_PLAINMAP_KINDS   // End mark; not really in the enumeration.
};

// See sdtables.cpp (search for "map_thing") for extensive discussion
// of how these maps, and their code numbers, are handled.

class map {

 public:

   // We make this a struct inside the class, rather than having its
   // fields just comprise the class itself (note that there are no
   // fields in this class, and it is never instantiated) so that
   // we can make "map_init_table" be a statically initialized array.

   struct map_thing {
      veryshort maps[48];  // Darn it!  40 would be enough for all maps but one.
      setup_kind inner_kind;
      int arity;
      mpkind map_kind;
      int vert;
      short int warncode;
      setup_kind outer_kind;
      uint32 rot;
      uint32 per_person_rot;
      uint32 code;
      map_thing *next;
   };

   static void initialize();
   static const map_thing *get_map_from_code(uint32 map_encoding);

 private:

   // Must be a power of 2.
   enum { NUM_MAP_HASH_BUCKETS = 128 };

   static map_thing *map_hash_table2[NUM_MAP_HASH_BUCKETS];

   // The big initialization tables, in sdtables.
   static const map_thing spec_map_table[];
   static map_thing map_init_table[];
};

enum specmapkind {
   spcmap_stairst,
   spcmap_ladder,
   spcmap_but_o,
   spcmap_blocks,
   spcmap_2x4_diagonal,
   spcmap_2x4_int_pgram,
   spcmap_2x4_trapezoid,
   spcmap_trngl_box1,
   spcmap_trngl_box2,
   spcmap_inner_box,
   spcmap_lh_stag,
   spcmap_rh_stag,
   spcmap_lh_stag_1_4,
   spcmap_rh_stag_1_4,
   spcmap_lh_ox,
   spcmap_rh_ox,
   spcmap_lh_c1phana,
   spcmap_rh_c1phana,
   spcmap_lh_c1phanb,
   spcmap_rh_c1phanb,
   spcmap_lh_s2x3_3,
   spcmap_rh_s2x3_3,
   spcmap_lh_s2x3_2,
   spcmap_rh_s2x3_2,
   spcmap_lh_s2x3_7,
   spcmap_rh_s2x3_7,
   spcmap_d1x10,
   spcmap_lz12,
   spcmap_rz12,
   spcmap_tgl451,
   spcmap_tgl452,
   spcmap_dmd_1x1,
   spcmap_star_1x1,
   spcmap_qtag_f0,
   spcmap_qtag_f1,
   spcmap_qtag_f2,
   spcmap_diag2a,
   spcmap_diag2b,
   spcmap_diag23a,
   spcmap_diag23b,
   spcmap_diag23c,
   spcmap_diag23d,
   spcmap_f2x8_4x4,
   spcmap_f2x8_4x4h,
   spcmap_w4x4_4x4,
   spcmap_w4x4_4x4h,
   spcmap_f2x8_2x8,
   spcmap_f2x8_2x8h,
   spcmap_w4x4_2x8,
   spcmap_emergency1,
   spcmap_emergency2,
   spcmap_fix_triple_turnstyle,
   spcmap_2x2v,
   spcmap_2x4_magic,
   spcmap_ptp_magic,
   spcmap_ptp_intlk,
   spcmap_ptp_magic_intlk,
   spcmap_4x4_ns,
   spcmap_4x4_ew,
   spcmap_4x4_spec0,
   spcmap_4x4_spec1,
   spcmap_4x4_spec2,
   spcmap_4x4_spec3,
   spcmap_4x4_spec4,
   spcmap_4x4_spec5,
   spcmap_4x4v,
   spcmap_4x4_1x1,
   spcmap_trglbox3x4a,
   spcmap_trglbox3x4b,
   spcmap_trglbox3x4c,
   spcmap_trglbox3x4d,
   spcmap_trglbox4x4,
   spcmap_trglbox4x5a,
   spcmap_trglbox4x5b,
   spcmap_2x3_1234,
   spcmap_2x3_0145,
   spcmap_1x8_1x6,
   spcmap_rig_1x6,
   spcmap_qtag_2x3,
   spcmap_dbloff1,
   spcmap_dbloff2,
   spcmap_dhrgl1,
   spcmap_dhrgl2,
   spcmap_dbgbn1,
   spcmap_dbgbn2,
   spcmap_off1x81,
   spcmap_off1x82,
   spcmap_dqtag1,
   spcmap_dqtag2,
   spcmap_dqtag3,
   spcmap_dqtag4,
   spcmap_dqtag5,
   spcmap_dqtag6,
   spcmap_stw3a,
   spcmap_stw3b,
   spcmap_3ri,
   spcmap_3li,
   spcmap_3ro,
   spcmap_3lo,
   spcmap_328a,
   spcmap_328b,
   spcmap_328c,
   spcmap_328d,
   spcmap_328e,
   spcmap_328f,
   spcmap_31x3d,
   spcmap_34x6a,
   spcmap_34x6b,
   spcmap_3lqtg,
   spcmap_blob_1x4a,
   spcmap_blob_1x4b,
   spcmap_blob_1x4c,
   spcmap_blob_1x4d,
   spcmap_wblob_1x4a,
   spcmap_wblob_1x4b,
   spcmap_wblob_1x4c,
   spcmap_wblob_1x4d,
   spcmap_bigbone_cw,
   spcmap_bigbone_ccw,
   NUM_SPECMAP_KINDS   // End mark; not really in the enumeration.
};

#define MAPCODE(setupkind,num,mapkind,vert) ((((int)(setupkind)) << 12) | (((int)(mapkind)) << 4) | (((num)-1) << 1) | (vert) | 0x80000000)


struct clw3_thing {
   setup_kind k;
   uint32 mask;
   uint32 test;
   uint32 map_code;
   int rot;
   veryshort inactives[9];
};


/* in SDCTABLE */

extern SDLIB_API const conzept::concept_descriptor *concept_descriptor_table;
extern SDLIB_API call_with_name **main_call_lists[call_list_extent];
extern SDLIB_API int number_of_calls[call_list_extent];
extern SDLIB_API dance_level calling_level;

extern nice_setup_info_item nice_setup_info[];

extern const concept_fixer_thing concept_fixer_table[];

extern selector_item selector_list[];                               /* in SDTABLES */
extern Cstring warning_strings[];                                   /* in SDTABLES */

extern const clw3_thing clw3_table[];                               /* in SDTABLES */


/* In SDPREDS */

extern bool selectp(setup *ss, int place) THROW_DECL;

/* In SDGETOUT */

SDLIB_API void write_resolve_text(bool doing_file);
SDLIB_API uims_reply full_resolve();
extern int concepts_in_place();
extern int resolve_command_ok();
extern int nice_setup_command_ok();
SDLIB_API void create_resolve_menu_title(
   command_kind goal,
   int cur,
   int max,
   resolver_display_state state,
   char *title);
void initialize_getout_tables();


/* In SDBASIC */

class collision_collector {
public:

   // Simple constructor, takes argument saying whether collisions will be legal.
   collision_collector(bool allow):
      allow_collisions(allow),
      collision_mask(0),
      callflags1(CFLAG1_TAKE_RIGHT_HANDS),  // Default is that collisions are legal.
      assume_ptr((assumption_thing *) 0),
      m_force_mirror_warn(false),
      m_doing_half_override(false),
      cmd_misc_flags(0),
      m_collision_appears_illegal(1),  // Halfway between "appears_illegal"
                                       // and not -- use table item.
      result_mask(0)
   {}

   // Glorious constructor, takes all sorts of stuff.
   collision_collector(bool mirror, setup_command *cmd, const calldefn *callspec):
      allow_collisions(true),
      collision_mask(0),
      callflags1(callspec->callflags1),
      assume_ptr(&cmd->cmd_assume),
      m_force_mirror_warn(mirror),
      m_doing_half_override(false),
      cmd_misc_flags(cmd->cmd_misc_flags),
      m_collision_appears_illegal(0),  // May change to 2 as call progresses.
      result_mask(0)
      {
         // If doing half of a call, and doing it left,
         // and there is a "collision", make them come to left hands.
         if (mirror && cmd->cmd_final_flags.test_heritbit(INHERITFLAG_HALF)) {
            m_force_mirror_warn = false;
            m_doing_half_override = true;
         }
      }

   void note_prefilled_result(setup *result)
      {
         for (int i=0; i<=attr::slimit(result); i++)
            if (result->people[i].id1) result_mask |= (1 << i);
      }

   void install_with_collision(
      setup *result, int resultplace,
      const setup *sourcepeople, int sourceplace,
      int rot) THROW_DECL;

   void fix_possible_collision(setup *result) THROW_DECL;

private:
   const bool allow_collisions;
   int collision_index;
   uint32 collision_mask;
   uint32 callflags1;
   assumption_thing *assume_ptr;
   bool m_force_mirror_warn;
   bool m_doing_half_override;
   uint32 cmd_misc_flags;
   int m_collision_appears_illegal;
   uint32 result_mask;
};

extern void mirror_this(setup *s) THROW_DECL;

extern void do_stability(uint32 *personp,
                         int field,
                         int turning,
                         bool mirror) THROW_DECL;

extern bool check_restriction(
   setup *ss,
   assumption_thing restr,
   bool instantiate_phantoms,
   uint32 flags) THROW_DECL;

extern void basic_move(
   setup *ss,
   calldefn *the_calldefn,
   int tbonetest,
   bool fudged,
   bool mirror,
   setup *result) THROW_DECL;

/* In SDMOVES */

extern void canonicalize_rotation(setup *result) THROW_DECL;

extern void reinstate_rotation(setup *ss, setup *result) THROW_DECL;

extern bool divide_for_magic(
   setup *ss,
   uint32 heritflags_to_check,
   setup *result) THROW_DECL;

extern bool do_simple_split(
   setup *ss,
   split_command_kind split_command,
   setup *result) THROW_DECL;

extern uint32 do_call_in_series(
   setup *sss,
   bool dont_enforce_consistent_split,
   bool roll_transparent,
   bool normalize,
   bool qtfudged) THROW_DECL;

extern void brute_force_merge(const setup *res1, const setup *res2,
                              merge_action action, setup *result) THROW_DECL;

extern void drag_someone_and_move(setup *ss, parse_block *parseptr, setup *result) THROW_DECL;

extern void anchor_someone_and_move(setup *ss, parse_block *parseptr, setup *result) THROW_DECL;

extern void process_number_insertion(uint32 mod_word);

extern bool get_real_subcall(
   parse_block *parseptr,
   const by_def_item *item,
   const setup_command *cmd_in,
   const calldefn *parent_call,
   bool forbid_flip,
   uint32 extra_heritmask_bits,
   setup_command *cmd_out) THROW_DECL;

extern int gcd(int a, int b);

// These control inversion of the start and end args.  That is, the position
// fraction F is turned into 1-F.
enum fraction_invert_flags {
   FRAC_INVERT_NONE = 0,
   FRAC_INVERT_START = 1,
   FRAC_INVERT_END = 2
};

extern uint32 process_stupendously_new_fractions(int start, int end,
                                                 fraction_invert_flags invert_flags,
                                                 const fraction_command & incoming_fracs,
                                                 bool make_improper = false,
                                                 bool *improper_p = 0) THROW_DECL;

extern bool fill_active_phantoms_and_move(setup *ss, setup *result) THROW_DECL;

extern void move_perhaps_with_active_phantoms(setup *ss, setup *result) THROW_DECL;

extern void impose_assumption_and_move(setup *ss, setup *result) THROW_DECL;

extern void move(
   setup *ss,
   bool qtfudged,
   setup *result) THROW_DECL;

/* In SDISTORT */

extern void prepare_for_call_in_series(setup *result, setup *ss);

extern void minimize_splitting_info(setup *ss, const resultflag_rec & other_split_info);

extern void remove_z_distortion(setup *ss) THROW_DECL;

extern void remove_tgl_distortion(setup *ss) THROW_DECL;

extern void divided_setup_move(
   setup *ss,
   uint32 map_encoding,
   phantest_kind phancontrol,
   bool recompute_id,
   setup *result,
   unsigned int noexpand_bits_to_set = CMD_MISC__NO_EXPAND_1 | CMD_MISC__NO_EXPAND_2) THROW_DECL;

extern void overlapped_setup_move(
   setup *ss,
   uint32 map_encoding,
   uint32 *masks, setup *result) THROW_DECL;

extern void do_phantom_2x4_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void do_phantom_stag_qtg_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void do_phantom_diag_qtg_concept(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void distorted_2x2s_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void distorted_move(
   setup *ss,
   parse_block *parseptr,
   disttest_kind disttest,
   setup *result) THROW_DECL;

extern void triple_twin_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void do_concept_rigger(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void do_concept_wing(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void initialize_commonspot_tables();

extern void common_spot_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void do_glorious_triangles(
   setup *ss,
   const tglmap::tglmapkey *map_ptr_table,
   int indicator,
   setup *result) THROW_DECL;

extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

/* In SDCONCPT */

extern bool do_big_concept(
   setup *ss,
   setup *result) THROW_DECL;

void stable_move(
   setup *ss,
   bool fractional,
   bool everyone,
   int howfar,
   selector_kind who,
   setup *result) THROW_DECL;

/* In SDTAND */

extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,           // solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3
   int fraction_fields,   // number fields, if doing fractional twosome/solid
   int phantom,           // normal=0 phantom=1 general-gruesome=2 gruesome-with-wave-check=3
   tandem_key key,
   uint32 mxn_bits,
   bool phantom_pairing_ok,
   setup *result) THROW_DECL;

extern void initialize_tandem_tables();

/* In SDCONC */

extern void concentric_move(
   setup *ss,
   setup_command *cmdin,
   setup_command *cmdout,
   calldef_schema analyzer,
   uint32 modifiersin1,
   uint32 modifiersout1,
   bool recompute_id,
   uint32 specialoffsetmapcode,
   setup *result) THROW_DECL;

extern resultflag_rec get_multiple_parallel_resultflags(setup outer_inners[], int number) THROW_DECL;

extern void initialize_fix_tables();

extern void normalize_concentric(
   calldef_schema synthesizer,
   int center_arity,
   setup outer_inners[],   /* outers in position 0, inners follow */
   int outer_elongation,
   uint32 matrix_concept,
   setup *result) THROW_DECL;

extern void merge_setups(setup *ss, merge_action action, setup *result) THROW_DECL;

extern void on_your_own_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void punt_centers_use_concept(setup *ss, setup *result) THROW_DECL;

extern void selective_move(
   setup *ss,
   parse_block *parseptr,
   selective_key indicator,
   int others,  // -1 - only selectees do the call, others can still roll
                //  0 - only selectees do the call, others can't roll
                //  1 - both sets
   uint32 arg2,
   uint32 override_selector,
   selector_kind selector_to_use,
   bool concentric_rules,
   setup *result) THROW_DECL;

extern void inner_selective_move(
   setup *ss,
   setup_command *cmd1,
   setup_command *cmd2,
   selective_key indicator,
   int others,  // -1 - only selectees do the call, others can still roll
                //  0 - only selectees do the call, others can't roll
                //  1 - both sets
   uint32 arg2,
   bool demand_both_setups_live,
   uint32 override_selector,
   selector_kind selector_to_use,
   uint32 modsa1,
   uint32 modsb1,
   setup *result) THROW_DECL;

/* In SDTOP */

extern void update_id_bits(setup *ss);

// This gets a ===> BIG-ENDIAN <=== mask of people's facing directions.
// Each person occupies 2 bits in the resultant masks.  The "livemask"
// bits are both on if the person is live.
extern void get_directions(
   setup *ss,
   uint32 & directions,
   uint32 & livemask);

extern void touch_or_rear_back(
   setup *scopy,
   bool did_mirror,
   int callflags1) THROW_DECL;

extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   bool recompute_id) THROW_DECL;

void initialize_sdlib();

extern void crash_print(const char *filename, int linenum) THROW_DECL;

struct skipped_concept_info {
   parse_block *old_retval;
   parse_block *skipped_concept;
   uint32 need_to_restrain;   // 1=(if not doing echo), 2=(yes, always)
   uint32 heritflag;
   parse_block *concept_with_root;
   parse_block *result_of_skip;
   parse_block **root_of_result_of_skip;
};

extern bool check_for_concept_group(
   parse_block *parseptrcopy,
   skipped_concept_info & retstuff,
   bool want_result_root) THROW_DECL;

NORETURN1 void fail(const char s[]) THROW_DECL NORETURN2;

NORETURN1 void fail_no_retry(const char s[]) THROW_DECL NORETURN2;

NORETURN1 extern void fail2(const char s1[], const char s2[]) THROW_DECL NORETURN2;

NORETURN1 extern void failp(uint32 id1, const char s[]) THROW_DECL NORETURN2;

NORETURN1 void specialfail(const char s[]) THROW_DECL NORETURN2;

extern void warn(warning_index w);

extern restriction_test_result verify_restriction(
   setup *ss,
   assumption_thing tt,
   bool instantiate_phantoms,
   bool *failed_to_instantiate) THROW_DECL;

extern callarray *assoc(begin_kind key, setup *ss, callarray *spec) THROW_DECL;

extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index) THROW_DECL;

extern void clear_result_flags(setup *z);

inline uint32 rotperson(uint32 n, int amount)
{ if (n == 0) return 0; else return (n + amount) & ~064; }

inline uint32 rotcw(uint32 n)
{ if (n == 0) return 0; else return (n + 011) & ~064; }

inline uint32 rotccw(uint32 n)
{ if (n == 0) return 0; else return (n + 033) & ~064; }


inline uint32 little_endian_live_mask(const setup *ss)
{
   int i;
   uint32 j, result;
   for (i=0, j=1, result = 0; i<=attr::slimit(ss); i++, j<<=1) {
      if (ss->people[i].id1) result |= j;
   }
   return result;
}


inline uint32 or_all_people(const setup *ss)
{
   uint32 result = 0;

   for (int i=0 ; i<=attr::slimit(ss) ; i++)
      result |= ss->people[i].id1;

   return result;
}


inline void setup::clear_person(int place)
{
   people[place].id1 = 0;
   people[place].id2 = 0;
   people[place].id3 = 0;
}

inline void setup::suppress_roll(int place)
{
   if (people[place].id1)
      people[place].id1 = (people[place].id1 & (~NROLL_MASK)) | ROLL_IS_M;
}

inline void setup::suppress_all_rolls()
{
   // If we can't determine the setup size, it will be -1,
   // and the loop below will take no action.
   for (int k=0; k<=attr::klimit(kind); k++)
      suppress_roll(k);
}


inline void setup::swap_people(int oneplace, int otherplace)
{
   personrec temp = people[otherplace];
   people[otherplace] = people[oneplace];
   people[oneplace] = temp;
}

extern uint32 copy_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace);

extern uint32 copy_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount);

extern void install_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace);

extern void install_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount) THROW_DECL;

extern void scatter(setup *resultpeople, const setup *sourcepeople,
                    const veryshort *resultplace, int countminus1, int rotamount) THROW_DECL;

extern void gather(setup *resultpeople, const setup *sourcepeople,
                   const veryshort *resultplace, int countminus1, int rotamount);

extern void install_scatter(setup *resultpeople, int num, const veryshort *placelist,
                            const setup *sourcepeople, int rot) THROW_DECL;

extern parse_block *process_final_concepts(
   parse_block *cptr,
   bool check_errors,
   final_and_herit_flags *final_concepts,
   bool forbid_unfinished_parse,
   bool only_one) THROW_DECL;

extern void really_skip_one_concept(
   parse_block *incoming,
   skipped_concept_info & retstuff) THROW_DECL;

extern bool fix_n_results(int arity,
                          int goal,
                          bool reorder_setups_2_and_3,
                          setup z[],
                          uint32 & rotstates,
                          uint32 & pointclip,
                          uint32 fudgystupidrot) THROW_DECL;

extern bool warnings_are_unacceptable(bool strict);

extern void normalize_setup(setup *ss, normalize_action action, bool noqtagcompress)
     THROW_DECL;

void check_concept_parse_tree(parse_block *conceptptr, bool strict) THROW_DECL;

bool check_for_centers_concept(uint32 callflags1_to_examine,
                               parse_block *parse_scan,
                               setup_command *the_cmd) THROW_DECL;

void toplevelmove() THROW_DECL;

void finish_toplevelmove() THROW_DECL;

SDLIB_API bool deposit_call_tree(modifier_block *anythings, parse_block *save1, int key);

extern bool do_subcall_query(
   int snumber,
   parse_block *parseptr,
   parse_block **newsearch,
   bool this_is_tagger,
   bool this_is_tagger_circcer,
   call_with_name *orig_call);

extern call_list_kind find_proper_call_list(setup *s);

class fraction_info {
 public:
   fraction_info(int n) :
      m_reverse_order(false),
      m_instant_stop(99),  // If not 99, says to stop instantly after doing one part,
                         // and to report (in RESULTFLAG__PART_COMPLETION_BITS bit)
                         // whether that part was the last part.
      m_do_half_of_last_part(0),
      m_do_last_half_of_first_part(0),
      m_highlimit(n),
      m_start_point(0),
      m_end_point(n-1),
      m_fetch_index(0),
      m_client_index(0),
      m_fetch_total(n),
      m_client_total(n),
      m_subcall_incr(1)
      {}

   // This one is in sdmoves.cpp
   void get_fraction_info(fraction_command frac_stuff,
                          uint32 callflags1,
                          revert_weirdness_type doing_weird_revert) THROW_DECL;

   // This one is in sdmoves.cpp
   uint32 get_fracs_for_this_part();

   // This one is in sdmoves.cpp
   bool query_instant_stop(uint32 & result_flag_wordmisc) const;

   void demand_this_part_exists()  const THROW_DECL
      {
         if (m_fetch_index >= m_fetch_total || m_fetch_index < 0)
            fail("The indicated part number doesn't exist.");
      }

   void fudge_client_total(int delta)
      {
         m_client_total += delta;
         m_highlimit = m_client_total;
         m_end_point = m_highlimit-1;
      }

   bool not_yet_in_active_section()
      {
         if (m_reverse_order) {
            if (m_client_index > m_start_point) return true;
         }
         else {
            if (m_client_index < m_start_point) return true;
         }
         return false;
      }

   bool ran_off_active_section()
      {
         if (m_reverse_order) {
            if (m_client_index < m_end_point) return true;
         }
         else {
            if (m_client_index > m_end_point) return true;
         }
         return false;
      }

   inline bool this_starts_at_beginning()
      { return
           m_start_point == 0 &&
           !m_do_last_half_of_first_part &&
           !m_reverse_order;
      }

 public:
   bool m_reverse_order;
   int m_instant_stop;
   bool m_first_call;
   uint32 m_do_half_of_last_part;
   uint32 m_do_last_half_of_first_part;
   int m_highlimit;
 private:
   int m_start_point;
   int m_end_point;
 public:
   int m_fetch_index;
   int m_client_index;
   int m_fetch_total;
   int m_client_total;
   int m_subcall_incr;
};


/* In SDUTIL */

extern void FuckingThingToTryToKeepTheFuckingStupidMicrosoftCompilerFromScrewingUp();

const char *get_escape_string(char c);
void write_history_line(int history_index,
                        bool picture,
                        bool leave_missing_calls_blank,
                        file_write_flag write_to_file,
                        const char *header = 0);
void unparse_call_name(Cstring name, char *s, call_conc_option_state *options);
void print_recurse(parse_block *thing, int print_recurse_arg);
void clear_screen();
extern void writechar(char src);
SDLIB_API void newline();
void doublespace_file();
SDLIB_API void writestuff(const char *s);
extern parse_block *mark_parse_blocks();
extern void release_parse_blocks_to_mark(parse_block *mark_point);
extern parse_block *copy_parse_tree(parse_block *original_tree);
parse_block *get_parse_block();
extern void reset_parse_tree(parse_block *original_tree, parse_block *final_head);
extern void save_parse_state();
extern void restore_parse_state();
void string_copy(char **dest, Cstring src);
void display_initial_history(int upper_limit, int num_pics);
extern void initialize_parse();
void run_program();

/* In SDINIT */


SDLIB_API bool parse_level(Cstring s);
void start_sel_dir_num_iterator();
bool iterate_over_sel_dir_num(
   bool enable_selector_iteration,
   bool enable_direction_iteration,
   bool enable_number_iteration);
bool install_outfile_string(const char newstring[]);
SDLIB_API bool get_first_session_line();
SDLIB_API bool get_next_session_line(char *dest);
void prepare_to_read_menus();
SDLIB_API int process_session_info(Cstring *error_msg);
void close_init_file();
SDLIB_API void general_final_exit(int code);
extern bool open_session(int argc, char **argv);

/* In SDMATCH */

void do_accelerator_spec(Cstring inputline, bool is_accelerator);
SDLIB_API bool process_accel_or_abbrev(modifier_block & mb, char linebuff[]);
SDLIB_API void erase_matcher_input();
SDLIB_API int delete_matcher_word();
void matcher_initialize();
SDLIB_API void matcher_setup_call_menu(call_list_kind cl);
SDLIB_API int match_user_input(
   int which_commands,
   bool show,
   bool show_verify,
   bool only_want_extension);

/* In SDPICK */

bool in_exhaustive_search();
void reset_internal_iterators();
selector_kind do_selector_iteration(bool allow_iteration);
direction_kind do_direction_iteration();
void do_number_iteration(int howmanynumbers,
                         uint32 odd_number_only,
                         bool allow_iteration,
                         uint32 *number_list);
bool do_tagger_iteration(uint32 tagclass,
                         uint32 *tagg,
                         uint32 numtaggers,
                         call_with_name **tagtable);
void do_circcer_iteration(uint32 *circcp);
const conzept::concept_descriptor *pick_concept(bool already_have_concept_in_place);
call_with_name *do_pick();
resolve_goodness_test get_resolve_goodness_info();
bool pick_allow_multiple_items();
void start_pick();
void end_pick();
bool forbid_call_with_mandatory_subcall();
bool allow_random_subcall_pick();

/* In SDUI */

// Change the title bar (or whatever it's called) on the window.
extern void ttu_set_window_title(const char *string);

// Initialize this package.
extern void ttu_initialize();

// The opposite.
extern void ttu_terminate();

// Get number of lines to use for "more" processing.  This number is
// not used for any other purpose -- the rest of the program is not concerned
// with the "screen" size.

extern int get_lines_for_more();

// Return true for 'tty-like' devices which don't require 'more' processing;
// i.e. they have an unlimited scrollback buffer.
extern bool ttu_unlimited_scrollback();

// Clear the current line, leave cursor at left edge.
extern void clear_line();

// Backspace the cursor and clear the rest of the line, presumably
// erasing the last character.
extern void rubout();

// Move cursor up "n" lines and then clear rest of screen.
extern void erase_last_n(int n);

// Write a line.  The text may or may not have a newline at the end.
// This may or may not be after a prompt and/or echoed user input.
extern void put_line(const char the_line[]);

// Write a single character on the current output line.
extern void put_char(int c);

/* Get one character from input, no echo, no waiting for <newline>.
   Return large number for function keys and alt alphabetics:
      128+N for plain function key (F1 = 129)
      144+N for shifted
      160+N for control
      176+N for alt function key
      192..217 for ctl letter (ctl-A = 192)
      218..243 for alt letter (alt-A = 218)
      244..269 for ctl-alt letter (ctl-alt-A = 244) */

extern int get_char();

/* Get string from input, up to <newline>, with echoing and editing.
   Return it without the final <newline>. */
extern void get_string(char *dest, int max);

/* Ring the bell, or whatever. */
extern void ttu_bell();

extern void refresh_input();

/* in SDSI */

extern void general_initialize();
SDLIB_API int generate_random_number(int modulus);
SDLIB_API void hash_nonrandom_number(int number);
SDLIB_API void *get_mem(uint32 siz);
SDLIB_API void *get_more_mem(void *oldp, uint32 siz);
SDLIB_API void get_date(char dest[]);
extern char *get_errstring();
SDLIB_API void open_file();
SDLIB_API void close_file();
SDLIB_API void write_file(char line[]);

/* in SDMAIN */

SDLIB_API bool deposit_call(call_with_name *call, const call_conc_option_state *options);
SDLIB_API bool deposit_concept(const conzept::concept_descriptor *conc);
SDLIB_API int sdmain(int argc, char *argv[]);
