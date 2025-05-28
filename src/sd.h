/* SD -- square dance caller's helper.

    Copyright (C) 1990-2000  William B. Ackerman.
    Copyright (C) 1993  Stephen Gildea
    Copyright (C) 1993  Alan Snyder

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

#include <stdio.h>


#ifdef WIN32
#ifndef SDLIB_API
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

//#define USE_THROW

#if defined USE_THROW

#if defined(WIN32) && defined(__cplusplus)
// Microsoft can't be bothered to support C++ exception declarations
// properly, but this pragma at least makes the compiler not complain.
// Geez!  If I wanted to use compilers that whine and wail
// piteously about the fact that I'm actually using the language, I'd
// use the HP-UX compilers!  They did a wonderful job of pointing out,
// in warning messages, that function prototypes are an ANSI C feature.
// As though this dangerous fact (that I was using ANSI C, of all things)
// were something that I didn't know and needed to to warned about.
// The HP-UX compilers came into the 20th century in the nick of time.
#pragma warning(disable: 4290)
#endif

#define THROW_DECL throw(error_flag_type)
#else
#define THROW_DECL
#endif


// The Intel compiler gets upset when it sees structure definitions
// with "const" fields, that are not actually class definitions that
// have constructors for those fields.  It quite rightly recognizes
// that you can't have a class member be "const" if there isn't some
// way to initialize instances, and that this is done in C++ with
// class constructors.  I don't agree with the compiler extending
// this reasoning to plain vanilla structs.  In any case, we
// statically initialize the various structs in the plain C files
// sdtables.c and sdctable.c, to get around the C++ initialization
// restriction.  The Intel compiler doesn't know that we are going
// to do that, so it doesn't understand how these struct definitions
// with "const" fields are going to work.  So we shut off the warning.
#if defined(WIN32) && defined(__ICL) && defined(__cplusplus)
#pragma warning(disable: 411)
#endif


#include "database.h"

#define MAX_PEOPLE 24


typedef struct {
   uint32 her8it;
   uint32 final;
} uint64;


typedef struct {
   int no_graphics;       // 1 = "no_checkers"; 2 = "no_graphics"
   int no_intensify;
   int reverse_video;     // 0 = black-on-white (default); 1 = white-on-black
   int pastel_color;      // 1 = use pastel red/grn for color by gender;
                          // 0 = bold colors.  Color by couple or color by corner
                          // are always done with bold colors.
   int no_color;          // 0 = default (by gender); 1 = none at all;
                          // 2 = by_couple; 3 = by_corner
   int no_sound;
   int sequence_num_override;
   long_boolean singlespace_mode;
   long_boolean nowarn_mode;
   long_boolean accept_single_click;

   /* This gets set if a user interface (e.g. sdui-tty/sdui-win) wants escape sequences
      for drawing people, so that it can fill in funny characters, or draw in color.
      This applies only to calls to uims_add_new_line with a nonzero second argument.

      0 means don't use any funny stuff.  The text strings transmitted when drawing
      setups are completely plain ASCII.

      1 means use escapes for the people themselves (13 octal followed by a byte of
      person identifier followed by a byte of direction) byt don't use the special
      spacing characters.  All spacing and formatting is done with spaces.

      2 means use escapes and other special characters.  Whenever the second arg to
      uims_add_new_line is nonzero, then in addition to the escape sequences for the
      people themselves, we have an escape sequence for a phantom, and certain
      characters have special meaning:  5 means space 1/2 of a glyph width, etc.
      See the definition of newline for details. */
   int use_escapes_for_drawing_people;

   /* These could get changed if the user requests special naming.  See "alternate_glyphs_1"
      in the command-line switch parser in sdsi.c. */
   char *pn1;             // 1st char (1/2/3/4) of what we use to print person.
   char *pn2;             // 2nd char (B/G) of what we use to print person.
   char *direc;           // 3rd char (direction arrow) of what we use to print person.
   int squeeze_this_newline;  // randomly used by printing stuff.
   int drawing_picture;       // randomly used by printing stuff.
} ui_option_type;


/* BEWARE!!  This list must track the array "concept_table" in sdconcpt.cpp . */
typedef enum {

/* These next few are not concepts.  Their appearance marks the end of a parse tree. */

   concept_another_call_next_mod,         /* calla modified by callb */
   concept_mod_declined,                  /* user was queried about modification, and said no. */
   marker_end_of_list,                    /* normal case */

/* This is not a concept.  Its appearance indicates a comment is to be placed here. */

   concept_comment,

/* Everything after this is a real concept. */

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
   concept_do_phantom_1x6,
   concept_do_phantom_triple_1x6,
   concept_do_phantom_1x8,
   concept_do_phantom_triple_1x8,
   concept_do_phantom_2x4,
   concept_do_phantom_stag_qtg,
   concept_do_phantom_diag_qtg,
   concept_do_phantom_2x3,
   concept_divided_2x4,
   concept_divided_2x3,
   concept_do_divided_diamonds,
   concept_distorted,
   concept_single_diagonal,
   concept_double_diagonal,
   concept_parallelogram,
   concept_triple_lines,
   concept_multiple_lines_tog,
   concept_multiple_lines_tog_std,
   concept_triple_1x8_tog,
   concept_quad_lines,
   concept_quad_lines_of_3,
   concept_quad_boxes,
   concept_quad_boxes_together,
   concept_triple_boxes,
   concept_triple_boxes_together,
   concept_triple_diamonds,
   concept_triple_diamonds_together,
   concept_quad_diamonds,
   concept_quad_diamonds_together,
   concept_triangular_boxes,
   concept_in_out_std,
   concept_in_out_nostd,
   concept_triple_diag,
   concept_triple_diag_together,
   concept_triple_twin,
   concept_misc_distort,
   concept_old_stretch,
   concept_new_stretch,
   concept_assume_waves,
   concept_active_phantoms,
   concept_mirror,
   concept_central,
   concept_snag_mystic,
   concept_crazy,
   concept_frac_crazy,
   concept_phan_crazy,
   concept_frac_phan_crazy,
   concept_fan,
   concept_c1_phantom,
   concept_grand_working,
   concept_centers_or_ends,
   concept_so_and_so_only,
   concept_some_vs_others,
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
   concept_outeracting,
   concept_ferris,
   concept_overlapped_diamond,
   concept_all_8,
   concept_centers_and_ends,
   concept_mini_but_o,
   concept_twice,
   concept_n_times,
   concept_sequential,
   concept_special_sequential,
   concept_meta,
   concept_meta_one_arg,
   concept_so_and_so_begin,
   concept_replace_nth_part,
   concept_replace_last_part,
   concept_interrupt_at_fraction,
   concept_sandwich,
   concept_interlace,
   concept_fractional,
   concept_rigger,
   concept_common_spot,
   concept_drag,
   concept_dblbent,
   concept_supercall,
   concept_diagnose
} concept_kind;

typedef enum {
   MPKIND__NONE,
   MPKIND__SPLIT,
   MPKIND__REMOVED,
   MPKIND__TWICE_REMOVED,
   MPKIND__THRICE_REMOVED,
   MPKIND__OVERLAP,
   MPKIND__INTLK,
   MPKIND__CONCPHAN,
   MPKIND__NONISOTROPIC,
   MPKIND__NONISOTROP1,
   MPKIND__OFFS_L_ONEQ,
   MPKIND__OFFS_R_ONEQ,
   MPKIND__OFFS_L_HALF,
   MPKIND__OFFS_R_HALF,
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
   MPKIND__SPEC_TWICEREM
} mpkind;

typedef struct skrilch {
   const veryshort maps[40];
   const setup_kind inner_kind;
   const int arity;
   const mpkind map_kind;
   const int vert;
   const short int warncode;
   const setup_kind outer_kind;
   const uint32 rot;
   const uint32 per_person_rot;
   uint32 code;
   struct skrilch *next;
} map_thing;

typedef struct {
   const setup_kind k;
   const uint32 mask;
   const uint32 test;
   const map_thing *map;
   const int rot;
   const veryshort inactives[9];
} clw3_thing;

typedef struct {
   Cstring name;
   const concept_kind kind;
   const uint32 concparseflags;   /* See above. */
   const dance_level level;
   const struct {
      const map_thing *maps;
      const uint32 arg1;
      const uint32 arg2;
      const uint32 arg3;
      const uint32 arg4;
      const uint32 arg5;
   } value;
   Cstring menu_name;
} concept_descriptor;

/* BEWARE!!  This list must track the array "selector_list" in sdutil.c . */
typedef enum {
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
   selector_outerpairs,
#ifdef TGL_SELECTORS
   /* Taken out.  Not convinced these are right.  See also sdutil.c, sdpreds.c . */
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
   /* Start of unsymmetrical selectors. */
#define unsymm_selector_start ((int) selector_nearline)
   selector_nearline,
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
   selector_enum_extent    // Not a selector; indicates extent of the enum.
} selector_kind;

/* BEWARE!!  This list must track the array "direction_names" in sdutil.c .
   It must also track the DITL "which direction" in *.rsrc in the Macintosh system. */
/* Note also that the "zig-zag" items will get disabled below A2.
   The key for this is "direction_zigzag". */
typedef enum {
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
} direction_kind;

/* BEWARE!!  There is a static initializer for this, "null_options", in sdmain.cpp
   that must be kept up to date. */
typedef struct {
   selector_kind who;        /* selector, if any, used by concept or call */
   direction_kind where;     /* direction, if any, used by concept or call */
   uint32 tagger;            /* tagging call indices, if any, used by call.
                                If nonzero, this is 3 bits for the 0-based tagger class
                                and 5 bits for the 1-based tagger call */
   uint32 circcer;           /* circulating call index, if any, used by call */
   uint32 number_fields;     /* number, if any, used by concept or call */
   int howmanynumbers;       /* tells how many there are */
   int star_turn_option;     /* For calls with "@S" star turn stuff. */
} call_conc_option_state;

typedef struct glork {
   struct glork *next;
   uint32 callarray_flags;
   call_restriction restriction;
   uint16 qualifierstuff;   /* See QUALBIT__??? definitions in database.h */
   uint8 start_setup;       /* Must cast to begin_kind! */
   uint8 end_setup;         /* Must cast to setup_kind! */
   uint8 end_setup_in;      /* Only if end_setup = concentric */  /* Must cast to setup_kind! */
   uint8 end_setup_out;     /* Only if end_setup = concentric */  /* Must cast to setup_kind! */
   union {
      /* Dynamically allocated to whatever size is required. */
      uint16 def[4];     /* only if pred = false */
      struct {                   /* only if pred = true */
         struct predptr_pair_struct *predlist;
         /* Dynamically allocated to whatever size is required. */
         char errmsg[4];
      } prd;
   } stuff;
} callarray;

typedef struct glowk {
   /* This has individual keys for groups of heritable modifiers.  Hence one
      can't say anything like "alternate_definition [3x3 4x4]".  But of course
      one can mix things from different groups. */
   uint32 modifier_seth;
   callarray *callarray_list;
   struct glowk *next;
   dance_level modifier_level;
} calldef_block;

typedef struct {
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
} by_def_item;

typedef struct structcalldefn {
   uint32 callflags1;    /* The CFLAG1_??? flags. */
   uint32 callflagsh;    /* The mask for the heritable flags. */
   /* Within the "callflagsh" field, the various grouped fields
      (e.g. INHERITFLAG_MXNMASK) are uniform -- either all the bits are
      on or they are all off.  A call can only inherit the entire group,
      by saying "mxn_is_inherited". */
   uint32 callflagsf;    /* The ESCAPE_WORD__???  and CFLAGH__??? flags. */
   short int age;
   short int level;
   calldef_schema schema;
   struct structcalldefn *compound_part;
   union {
      struct {
         calldef_block *def_list;
      } arr;            /* if schema = schema_by_array */
      struct {
         uint32 flags;
         uint32 *stuff;
      } matrix;         /* if schema = schema_matrix or schema_partner_matrix */
      struct {
         int howmanyparts;
         by_def_item *defarray;  /* Dynamically allocated, there are "howmanyparts" of them. */
      } seq;            /* if schema = schema_sequential or whatever */
      struct {
         by_def_item innerdef;
         by_def_item outerdef;
      } conc;           /* if schema = any of the concentric ones. */
   } stuff;
} calldefn;

typedef struct {
   calldefn the_defn;
   /* This is the "pretty" name -- "@" escapes have been changed to things like "<N>".
      If there are no escapes, this just points to the stuff below.
      If escapes are present, it points to allocated storage elsewhere.
      Either way, it persists throughout the program. */
   Cstring menu_name;
   /* Dynamically allocated to whatever size is required, will have trailing null.
      This is the name as it appeared in the database, with "@" escapes. */
   char name[4];
} call_with_name;

typedef struct glock {
   concept_descriptor *concept;   /* the concept or end marker */
   call_with_name *call;          /* if this is end mark, gives the call; otherwise unused */
   call_with_name *call_to_print; /* the original call, for printing (sometimes the field
                                     above gets changed temporarily) */
   struct glock *next;            /* next concept, or, if this is end mark,
                                     points to substitution list */
   struct glock *subsidiary_root; /* for concepts that take a second call,
                                     this is its parse root */
   struct glock *gc_ptr;          /* used for reclaiming dead blocks */
   call_conc_option_state options;/* number, selector, direction, etc. */
   short replacement_key;         /* this is the "DFM1_CALL_MOD_MASK" stuff
                                     (shifted down) for a modification block */
   short no_check_call_level;     /* if nonzero, don't check whether this call
                                     is at the level. */
} parse_block;

// For ui_command_select:

// BEWARE!!  The resolve part of this next definition must be keyed
// to the array "title_string" in sdgetout.cpp, and maybe stuff in the UI.
// For example, see "command_menu" in sdmain.cpp.

/* BEWARE!!  The order is slightly significant -- all search-type commands
   are >= command_resolve, and all "create some setup" commands
   are >= command_create_any_lines.  Certain tests are made easier by this. */
typedef enum {
   command_quit,
   command_undo,
   command_erase,
   command_abort,
   command_create_comment,
   command_change_outfile,
   command_change_header,
   command_getout,
   command_cut_to_clipboard,
   command_delete_entire_clipboard,
   command_delete_one_call_from_clipboard,
   command_paste_one_call,
   command_paste_all_calls,
#ifdef NEGLECT
   command_neglect,
#endif
   command_save_pic,
   command_help,
   command_help_manual,
   command_simple_mods,
   command_all_mods,
   command_toggle_conc_levels,
   command_toggle_minigrand,
   command_toggle_act_phan,
   command_toggle_retain_after_error,
   command_toggle_nowarn_mode,
   command_toggle_singleclick_mode,
   command_select_print_font,
   command_print_current,
   command_print_any,
   command_refresh,
   command_resolve,            /* Search commands start here */
   command_normalize,
   command_standardize,
   command_reconcile,
   command_random_call,
   command_simple_call,
   command_concept_call,
   command_level_call,
   command_8person_level_call,
   command_create_any_lines,   /* Create setup commands start here */
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
} command_kind;


/* In each case, an integer or enum is deposited into the global variable uims_menu_index.  Its interpretation
   depends on which of the replies above was given.  For some of the replies, it gives the index
   into a menu.  For "ui_start_select" it is a start_select_kind.
   For other replies, it is one of the following constants: */

/* BEWARE!!  This list must track the array "startup_resources" in sdui-x11.c . */
/* BEWARE!!!!!!!!  Lots of implications for "centersp" and all that! */
/* BEWARE!!  If change this next definition, be sure to update the definition of
   "startinfolist" in sdtables.c, and also necessary stuff in the user interfaces.
   The latter includes the definition of "start_choices" in sd.dps
   in the Domain/Dialog system, and the corresponding CNTLs in *.rsrc
   in the Macintosh system.  You may also need changes in create_controls() in
   macstuff.c. */
typedef enum {
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
   start_select_toggle_singleclick_mode,
   start_select_toggle_singer,
   start_select_toggle_singer_backward,
   start_select_select_print_font,
   start_select_print_current,
   start_select_print_any,
   start_select_init_session_file,
   start_select_change_outfile,
   start_select_change_header_comment,
   start_select_kind_enum_extent    // Not a start select kind; indicates extent of the enum.
} start_select_kind;


/* For ui_resolve_select: */
/* BEWARE!!  This list must track the array "resolve_resources" in sdui-x11.c . */
typedef enum {
   resolve_command_abort,
   resolve_command_find_another,
   resolve_command_goto_next,
   resolve_command_goto_previous,
   resolve_command_accept,
   resolve_command_raise_rec_point,
   resolve_command_lower_rec_point,
   resolve_command_write_this,
   resolve_command_kind_enum_extent    // Not a resolve kind; indicates extent of the enum.
} resolve_command_kind;


typedef struct {
   Cstring command_name;
   command_kind action;
   int resource_id;
} command_list_menu_item;

typedef struct {
   Cstring startup_name;
   start_select_kind action;
   int resource_id;
} startup_list_menu_item;

typedef struct {
   Cstring command_name;
   resolve_command_kind action;
} resolve_list_menu_item;


/* This defines a person in a setup.  Unfortunately, there is too much data
   to fit into 32 bits. */
typedef struct {
   uint32 id1;       /* Frequently used bits go here. */
   uint32 id2;       /* Bits used for evaluating predicates. */
} personrec;

/* Person bits for "id1" field are:
     0x80000000 -
     0x40000000 - not side girl    **** these 10 bits are "permanent" -- they never change in a person
     0x20000000 - not side boy
     0x10000000 - not head girl
     0x08000000 - not head boy
     0x04000000 - head corner
     0x02000000 - side corner
     0x01000000 - head
     0x00800000 - side
     0x00400000 - boy
     0x00200000 - girl             **** end of permanent bits
      4 000 000 - roll direction is CCW
      2 000 000 - roll direction is neutral
      1 000 000 - roll direction is CW
        400 000 - fractional stability enabled
        200 000 - stability "v" field -- 2 bits
        100 000 -     "
         40 000 - stability "r" field -- 3 bits
         20 000 -     "
         10 000 -     "
          4 000 - live person (just so at least one bit is always set)
          2 000 - active phantom (see below, under XPID_MASK)
          1 000 - virtual person (see below, under XPID_MASK)
            700 - these 3 bits identify actual person
             60 - these 2 bits must be clear for rotation
             10 - part of rotation (facing north/south)
              4 - bit must be clear for rotation
              2 - part of rotation
              1 - part of rotation (facing east/west)
*/

#define ID1_PERM_NSG           0x40000000UL
#define ID1_PERM_NSB           0x20000000UL
#define ID1_PERM_NHG           0x10000000UL
#define ID1_PERM_NHB           0x08000000UL
#define ID1_PERM_HCOR          0x04000000UL
#define ID1_PERM_SCOR          0x02000000UL
#define ID1_PERM_HEAD          0x01000000UL
#define ID1_PERM_SIDE          0x00800000UL
#define ID1_PERM_BOY           0x00400000UL
#define ID1_PERM_GIRL          0x00200000UL

#define ID1_PERM_ALLBITS       0x7FE00000UL

/* These are a 3 bit field -- ROLL_BIT tells where its low bit lies. */
#define ROLL_MASK   07000000UL
#define ROLL_BIT    01000000UL
#define ROLLBITL    04000000UL
#define ROLLBITM    02000000UL
#define ROLLBITR    01000000UL

/* These are a 6 bit field. */
#define STABLE_MASK  0770000UL
#define STABLE_ENAB  0400000UL
#define STABLE_VBIT  0100000UL
#define STABLE_RBIT  0010000UL

#define BIT_PERSON   0004000UL
#define BIT_ACT_PHAN 0002000UL
#define BIT_TANDVIRT 0001000UL

/* Person ID.  These bit positions are extremely hard wired into, among other
   things, the resolver and the printer. */
#define PID_MASK  0700UL

/* Extended person ID.  These 5 bits identify who the person is for the purpose
   of most manipulations.  0 to 7 are normal live people (the ones who squared up).
   8 to 15 are virtual people assembled for tandem or couples.  16 to 31 are
   active (but nevertheless identifiable) phantoms.  This means that, when
   BIT_ACT_PHAN is on, it usurps the meaning of BIT_TANDVIRT. */
#define XPID_MASK 03700UL

#define d_mask  04013UL
#define d_north 04010UL
#define d_south 04012UL
#define d_east  04001UL
#define d_west  04003UL


#define ID2_HEADLINE   0x80000000UL
#define ID2_SIDELINE   0x40000000UL
#define ID2_CTR2       0x20000000UL
#define ID2_BELLE      0x10000000UL
#define ID2_BEAU       0x08000000UL
#define ID2_CTR6       0x04000000UL
#define ID2_OUTR2      0x02000000UL
#define ID2_OUTR6      0x01000000UL
#define ID2_TRAILER    0x00800000UL
#define ID2_LEAD       0x00400000UL
#define ID2_CTRDMD     0x00200000UL
#define ID2_NCTRDMD    0x00100000UL
#define ID2_CTR1X4     0x00080000UL
#define ID2_NCTR1X4    0x00040000UL
#define ID2_CTR1X6     0x00020000UL
#define ID2_NCTR1X6    0x00010000UL
#define ID2_OUTR1X3    0x00008000UL
#define ID2_NOUTR1X3   0x00004000UL
#define ID2_FACING     0x00002000UL
#define ID2_NOTFACING  0x00001000UL
#define ID2_CENTER     0x00000800UL
#define ID2_END        0x00000400UL
#define ID2_NEARCOL    0x00000200UL
#define ID2_NEARLINE   0x00000100UL
#define ID2_NEARBOX    0x00000080UL
#define ID2_FARCOL     0x00000040UL
#define ID2_FARLINE    0x00000020UL
#define ID2_FARBOX     0x00000010UL
#define ID2_CTR4       0x00000008UL
#define ID2_OUTRPAIRS  0x00000004UL
#define ID2_FACEFRONT  0x00000002UL
#define ID2_FACEBACK   0x00000001UL


typedef struct gfwzqg {
   Const setup_kind bigsetup;
   Const calldef_schema lyzer;
   Const veryshort maps[24];
   Const short inlimit;
   Const short outlimit;
   Const setup_kind insetup;
   Const setup_kind outsetup;
   Const int bigsize;
   Const int inner_rot;    /* 1 if inner setup is rotated CCW relative to big setup */
   Const int outer_rot;    /* 1 if outer setup is rotated CCW relative to big setup */
   Const int mapelong;
   Const int center_arity;
   Const int elongrotallow;
   Const calldef_schema getout_schema;
   uint32 used_mask_analyze;
   uint32 used_mask_synthesize;
   struct gfwzqg *next_analyze;
   struct gfwzqg *next_synthesize;
} cm_thing;

/* The following items are not actually part of the setup description,
   but are placed here for the convenience of "move" and similar procedures.
   They contain information about the call to be executed in this setup.
   Once the call is complete, that is, when printing the setup or storing it
   in a history array, this stuff is meaningless. */

typedef struct {
   unsigned int assump_col:  16;  /* Stuff to go with assumption -- col vs. line. */
   unsigned int assump_both:  8;  /* Stuff to go with assumption -- "handedness" enforcement --
                                                0/1/2 = either/1st/2nd. */
   unsigned int assump_cast:  6;  /* Nonzero means there is an "assume normal casts" assumption. */
   unsigned int assump_live:  1;  /* One means to accept only if everyone is live. */
   unsigned int assump_negate:1;  /* One means to invert the sense of everything. */
   call_restriction assumption;   /* Any "assume waves" type command. */
} assumption_thing;

typedef struct {
   setup_kind skind;
   int srotation;
} small_setup;

typedef struct {
   parse_block *parseptr;
   call_with_name *callspec;
   uint64 cmd_final_flags;
   uint32 cmd_frac_flags;
   uint32 cmd_misc_flags;
   uint32 cmd_misc2_flags;
   uint32 do_couples_her8itflags;
   assumption_thing cmd_assume;
   uint32 prior_elongation_bits;
   uint32 prior_expire_bits;
   parse_block *restrained_concept;
   uint32 restrained_fraction;
   uint32 restrained_super8flags;
   parse_block *skippable_concept;
} setup_command;


/* Warning!  Do not rearrange these fields without good reason.  There are data
   initializers instantiating these in sdinit.cpp (test_setup_*) and in sdtables.c
   (startinfolist) that will need to be rewritten. */
typedef struct {
   setup_kind kind;
   int rotation;
   setup_command cmd;
   personrec people[MAX_PEOPLE];

   /* The following item is not actually part of the setup description, but contains
      miscellaneous information left by "move" and similar procedures, for the
      convenience of whatever called same. */
   uint32 result_flags;           /* Miscellaneous info, with names like RESULTFLAG__???. */

   /* The following three items are only used if the setup kind is "s_normal_concentric".  Note in particular that
      "outer_elongation" is thus underutilized, and that a lot of complexity goes into storing similar information
      (in two different places!) in the "prior_elongation_bits" and "result_flags" words. */
   small_setup inner;
   small_setup outer;
   int concsetup_outer_elongation;
} setup;

typedef struct {
   // We wish we could put a "throw" clause on this function, but we can't.
   long_boolean (*predfunc) (setup *, int, int, int, const long int *);
   const long int *extra_stuff;
} predicate_descriptor;

typedef struct predptr_pair_struct {
   predicate_descriptor *pred;
   struct predptr_pair_struct *next;
   /* Dynamically allocated to whatever size is required. */
   uint16 arr[4];
} predptr_pair;


#define MAPCODE(setupkind,num,mapkind,rot) ((((int)(setupkind)) << 12) | (((int)(mapkind)) << 4) | (((num)-1) << 1) | (rot))


#define LOOKUP_NONE     0x1
#define LOOKUP_DIST_CLW 0x2
#define LOOKUP_DISC     0x4
#define LOOKUP_IGNORE   0x8
#define LOOKUP_DIST_DMD 0x10
#define LOOKUP_Z        0x20
#define LOOKUP_DIST_BOX 0x40
#define LOOKUP_DIAG_BOX 0x80
#define LOOKUP_DIAG_CLW 0x100
#define LOOKUP_OFFS_CLW 0x200
#define LOOKUP_DBL_BENT 0x400
#define LOOKUP_MINI_B   0x800
#define LOOKUP_MINI_O   0x1000

#define LOOKUP_GEN_MASK (LOOKUP_DIAG_BOX|LOOKUP_DIST_DMD|LOOKUP_Z|LOOKUP_DIST_BOX| \
   LOOKUP_DIAG_CLW|LOOKUP_OFFS_CLW|LOOKUP_DBL_BENT|LOOKUP_MINI_B|LOOKUP_MINI_O)



typedef struct fixerjunk {
   const setup_kind ink;
   const setup_kind outk;
   const int rot;
   const short prior_elong;
   const short numsetups;
   const struct fixerjunk *next1x2;
   const struct fixerjunk *next1x2rot;
   const struct fixerjunk *next1x4;
   const struct fixerjunk *next1x4rot;
   const struct fixerjunk *nextdmd;
   const struct fixerjunk *nextdmdrot;
   const struct fixerjunk *next2x2;
   const struct fixerjunk *next2x2v;
   const veryshort nonrot[24];
} fixer;

typedef struct dirbtek {
   Const uint32 key;
   Const setup_kind kk;
   Const uint32 thislivemask;
   Const fixer *fixp;
   Const fixer *fixp2;
   Const int use_fixp2;
   struct dirbtek *next;
} sel_item;

typedef struct qwerty {
   const setup_kind kind;
   const setup_kind kind1x3;
   const struct qwerty *other;
   const veryshort nointlkshapechange;
   const veryshort switchtgls;
   const veryshort mapqt1[8];   /* In quarter-tag: first triangle (upright),
                                   then second triangle (inverted), then idle. */
   const veryshort mapcp1[8];   /* In C1 phantom: first triangle (inverted),
                                   then second triangle (upright), then idle. */
   const veryshort mapbd1[8];   /* In bigdmd. */
   const veryshort map241[8];   /* In 2x4. */
   const veryshort map261[8];   /* In 2x6. */
} tgl_map;

typedef struct {
   char *name;
   long_boolean into_the_middle;
   setup the_setup;
} startinfo;

typedef struct {
   const setup_kind result_kind;
   const int xfactor;
   const veryshort xca[24];
   const veryshort yca[24];
   const veryshort diagram[64];
} coordrec;

typedef uint32 id_bit_table[4];

typedef Const struct {
   /* This is the size of the setup MINUS ONE. */
   Const int setup_limits;

   /* These "coordrec" items have the fudged coordinates that are used for doing
      press/truck calls.  For some setups, the coordinates of some people are
      deliberately moved away from the obvious precise matrix spots so that
      those people can't press or truck.  For example, the lateral spacing of
      diamond points is not an integer.  If a diamond point does any truck or loop
      call, he/she will not end up on the other diamond point spot (or any other
      spot in the formation), so the call will not be legal.  This enforces our
      view, not shared by all callers (Hi, Clark!) that the diamond points are NOT
      as if the ends of lines of 3, and hence can NOT trade with each other by
      doing a right loop 1. */
   coordrec *setup_coords;

   /* The above table is not suitable for performing mirror inversion because,
      for example, the points of diamonds do not reflect onto each other.  This
      table has unfudged coordinates, in which all the symmetries are observed.
      This is the table that is used for mirror reversal.  Most of the items in
      it are the same as those in the table above. */
   coordrec *nice_setup_coords;

   /* These determine how designators like "side boys" get turned into
      "center 2", so that so-and-so moves can be done with the much
      more powerful concentric mechanism. */

   Const uint32 mask_normal;
   Const uint32 mask_6_2;
   Const uint32 mask_2_6;
   Const uint32 mask_ctr_dmd;

   /* These show the beginning setups that we look for in a by-array call
      definition in order to do a call in this setup. */
   Const begin_kind keytab[2];

   /* In the bounding boxes, we do not fill in the "length" of a diamond, nor
      the "height" of a qtag.  Everyone knows that the number must be 3, but it
      is not really accepted that one can use that in instances where precision
      is required.  That is, one should not make "matrix" calls depend on this
      number.  Witness all the "diamond to quarter tag adjustment" stuff that
      callers worry about, and the ongoing controversy about which way a quarter
      tag setup is elongated, even though everyone knows that it is 4 wide and 3
      deep, and that it is generally recognized, by the mathematically erudite,
      that 4 is greater than 3. */
   Const short int bounding_box[2];

   /* This is true if the setup has 4-way symmetry.  Such setups will always be
      canonicalized so that their rotation field will be zero. */
   Const long_boolean four_way_symmetry;

   /* This is the bit table for filling in the "ID2" bits. */
   id_bit_table *id_bit_table_ptr;

   /* These are the tables that show how to print out the setup. */
   Cstring print_strings[2];
} setup_attr;

typedef struct {
   char *destcurr;
   char lastchar;
   char lastlastchar;
   char *lastblank;
   long_boolean usurping_writechar;
} writechar_block_type;


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

#define NUM_CARDINALS 16


/* This is the number of tagger classes.  It must not be greater than 7,
   because class numbers, in 1-based form, are put into the CFLAGH__TAG_CALL_RQ_MASK
   and CFLAG1_BASE_TAG_CALL fields, and because the tagger class is stored
   (albeit in 0-based form) in the high 3 bits (along with the tagger call number
   in the low 5 bits) in an 8-bit field that is replicated 4 times in the
   "tagger" field of a call_conc_option_state. */
#define NUM_TAGGER_CLASSES 4


// These are the values returned in user_match.match.kind
// by "uims_get_call_command" and similar functions.

typedef enum {
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
} uims_reply;


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

typedef struct glozk {
   uims_reply kind;
   long int index;
   call_conc_option_state call_conc_options;  /* Has numbers, selectors, etc. */
   call_with_name *call_ptr;
   concept_descriptor *concept_ptr;
   struct glozk *packed_next_conc_or_subcall;  /* next concept, or, if this is end mark, points to substitution list */
   struct glozk *packed_secondary_subcall;     /* points to substitution list for secondary subcall */
   struct glozk *gc_ptr;                /* used for reclaiming dead blocks */
} modifier_block;

typedef struct filch {
   long_boolean valid;       /* set to TRUE if a match was found */
   long_boolean exact;       /* set to TRUE if an exact match was found */
   long_boolean indent;      /* This is a subordinate call; indent it in listing. */
   modifier_block match;
   Const struct filch *real_next_subcall;
   Const struct filch *real_secondary_subcall;
   int yield_depth;          /* if nonzero, this yields by that amount */
} match_result;

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

typedef enum {
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
   call_list_gcol, call_list_qtag
} call_list_kind;
#define NUM_CALL_LIST_KINDS (((int) call_list_qtag)+1)

typedef enum {
   call_list_mode_none,
   call_list_mode_writing,
   call_list_mode_writing_full,
   call_list_mode_abridging
} call_list_mode_t;

typedef struct {
   parse_block **concept_write_save_ptr;
   concept_kind save_concept_kind;
} parse_stack_item;

typedef struct {
   parse_stack_item parse_stack[40];
   int parse_stack_index;
   parse_block **concept_write_ptr;
   parse_block **concept_write_base;
   char specialprompt[MAX_TEXT_LINE_LENGTH];
   uint32 topcallflags1;
   call_list_kind call_list_to_use;
   call_list_kind base_call_list_to_use;
} parse_state_type;

typedef struct {
   Cstring name;
   Cstring sing_name;
   Cstring name_uc;
   Cstring sing_name_uc;
   selector_kind opposite;
} selector_item;

typedef enum {
   mode_none,     /* Not a real mode; used only for fictional purposes
                        in the user interface; never appears in the rest of the program. */
   mode_normal,
   mode_startup,
   mode_resolve
} mode_kind;

typedef enum {
    modify_popup_any,
    modify_popup_only_tag,
    modify_popup_only_circ
} modify_popup_kind;

typedef enum {
   file_write_no,
   file_write_double
} file_write_flag;

typedef enum {
   interactivity_database_init,
   interactivity_no_query_at_all,    /* Used when pasting from clipboard.  All subcalls,
                                        selectors, numbers, etc. must be filled in already.
                                        If not, it is a bug. */
   interactivity_verify,
   interactivity_normal,
   interactivity_picking
} interactivity_state;

typedef enum {
   resolver_display_ok,
   resolver_display_searching,
   resolver_display_failed
} resolver_display_state;

typedef enum {
   resolve_goodness_only_nice,
   resolve_goodness_always,
   resolve_goodness_maybe
} resolve_goodness_test;

/* These enumerate the setups from which we can perform a "normalize" search. */
/* This list tracks the array "nice_setup_info". */

typedef enum {
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
   nice_start_4x6
} nice_start_kind;
#define NUM_NICE_START_KINDS (((int) nice_start_4x6)+1)

typedef struct {
   int *full_list;
   int *on_level_list;
   int full_list_size;
} nice_setup_thing;

typedef struct {
   setup_kind kind;
   nice_setup_thing *thing;
   int *array_to_use_now;
   int number_available_now;
} nice_setup_info_item;

/* Values returned by the various popup routines: */
#define POPUP_DECLINE 0
#define POPUP_ACCEPT  1
#define POPUP_ACCEPT_WITH_STRING 2


/* These are the bits that get filled in by "update_id_bits". */
#define BITS_TO_CLEAR (ID2_LEAD|ID2_TRAILER|ID2_BEAU|ID2_BELLE| \
ID2_FACING|ID2_NOTFACING|ID2_CENTER|ID2_END|ID2_CTR2|ID2_CTR6|ID2_OUTR2|ID2_OUTR6| \
ID2_CTRDMD|ID2_NCTRDMD|ID2_CTR1X4|ID2_NCTR1X4|ID2_CTR1X6|ID2_NCTR1X6| \
ID2_OUTR1X3|ID2_NOUTR1X3|ID2_CTR4|ID2_OUTRPAIRS)


/* These are the really global position bits.  They get filled in only at the top level. */
#define GLOB_BITS_TO_CLEAR (ID2_NEARCOL|ID2_NEARLINE|ID2_NEARBOX|ID2_FARCOL|ID2_FARLINE|ID2_FARBOX|ID2_FACEFRONT|ID2_FACEBACK|ID2_HEADLINE|ID2_SIDELINE)





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

   RESULTFLAG__SPLIT_AXIS_MASK has info saying whether the call was split
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
*/

// The two low bits are used for result elongation, so we start with 0x00000004.

// Next 4 are a logical group.
static const uint32 RESULTFLAG__DID_LAST_PART        = 0x00000004UL;
static const uint32 RESULTFLAG__DID_NEXTTOLAST_PART  = 0x00000008UL;
static const uint32 RESULTFLAG__SECONDARY_DONE       = 0x00000010UL;
static const uint32 RESULTFLAG__PARTS_ARE_KNOWN      = 0x00000020UL;
static const uint32 RESULTFLAG__PART_COMPLETION_BITS = 0x0000003CUL;

static const uint32 RESULTFLAG__NEED_DIAMOND         = 0x00000040UL;

// This is a six bit field.
static const uint32 RESULTFLAG__SPLIT_AXIS_FIELDMASK = 0x00001F80UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_XMASK     = 0x00000380UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_XBIT      = 0x00000080UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_YMASK     = 0x00001C00UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_YBIT      = 0x00000400UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_SEPARATION= 3;

static const uint32 RESULTFLAG__ACTIVE_PHANTOMS_ON   = 0x00002000UL;
static const uint32 RESULTFLAG__ACTIVE_PHANTOMS_OFF  = 0x00004000UL;
static const uint32 RESULTFLAG__EXPAND_TO_2X3        = 0x00008000UL;

// Next 4 are a logical group.
static const uint32 RESULTFLAG__YOYO_EXPIRED         = 0x00010000UL;
static const uint32 RESULTFLAG__TWISTED_EXPIRED      = 0x00020000UL;
static const uint32 RESULTFLAG__SPLIT_EXPIRED        = 0x00040000UL;
static const uint32 RESULTFLAG__EXPIRATION_ENAB      = 0x00080000UL;
static const uint32 RESULTFLAG__EXPIRATION_BITS      = 0x000F0000UL;

static const uint32 RESULTFLAG__DID_TGL_EXPANSION    = 0x00100000UL;
static const uint32 RESULTFLAG__VERY_ENDS_ODD        = 0x00200000UL;
static const uint32 RESULTFLAG__VERY_CTRS_ODD        = 0x00400000UL;
// This is a 2 bit field.
static const uint32 RESULTFLAG__DID_Z_COMPRESSMASK   = 0x01800000UL;
static const uint32 RESULTFLAG__DID_Z_COMPRESSBIT    = 0x00800000UL;
static const uint32 RESULTFLAG__IMPRECISE_ROT        = 0x02000000UL;
static const uint32 RESULTFLAG__NO_REEVALUATE        = 0x04000000UL;

typedef struct glonk {
   char txt[MAX_TEXT_LINE_LENGTH];
   struct glonk *nxt;
} comment_block;

/* These bits appear in the "concparseflags" word. */
/* This is a duplicate, and exists only to make menus nicer.
   Ignore it when scanning in parser. */
#define CONCPARSE_MENU_DUP       0x00000001UL
/* If the parse turns out to be ambiguous, don't use this one --
   yield to the other one. */
#define CONCPARSE_YIELD_IF_AMB   0x00000002UL
/* Parse directly.  It directs the parser to allow this concept
   (and similar concepts) and the following call to be typed
   on one line.  One needs to be very careful about avoiding
   ambiguity when setting this flag. */
#define CONCPARSE_PARSE_DIRECT   0x00000004UL
/* These are used by "print_recurse" in sdutil.c to control the printing.
   They govern the placement of commas. */
#define CONCPARSE_PARSE_L_TYPE 0x8
#define CONCPARSE_PARSE_F_TYPE 0x10
#define CONCPARSE_PARSE_G_TYPE 0x20


/* BEWARE!!  This list must track the array "warning_strings" in sdtables.c . */
typedef enum {
   warn__none,
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
   warn__check_3x4,
   warn__check_2x4,
   warn__check_hokey_2x4,
   warn__check_4x4,
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
   warn__split_to_2x4s,
   warn__split_to_2x3s,
   warn__split_to_1x8s,
   warn__split_to_1x6s,
   warn__take_left_hands,
   warn__evil_interlocked,
   warn__split_phan_in_pgram,
   warn__bad_interlace_match,
   warn__not_on_block_spots,
   warn__stupid_phantom_clw,
   warn__bad_modifier_level,
   warn__bad_call_level,
   warn__did_not_interact,
   warn__opt_for_normal_cast,
   warn__opt_for_normal_hinge,
   warn__opt_for_2fl,
   warn__opt_for_no_collision,
   warn_partial_solomon,
   warn_same_z_shear,
   warn__like_linear_action,
   warn__no_z_action,
   warn__phantoms_thinner,
   warn__hokey_jay_shapechanger,
   warn__split_1x6,
   warn_interlocked_to_6,
   warn__offset_hard_to_see,
   warn__pg_hard_to_see,
   warn__colocated_once_rem,
   warn_big_outer_triangles,
   warn_hairy_fraction,
   warn_bad_collision,
   warn__dyp_resolve_ok,
   warn__unusual,
   warn_controversial,
   warn_serious_violation,
   warn_bogus_yoyo_rims_hubs,
   warn_pg_in_2x6,
   warn_real_people_spots,
   warn__tasteless_com_spot,
   warn__tasteless_junk,
   warn__tasteless_slide_thru,
   warn__diagnostic,
   warn__NUM_WARNINGS       // Not a real warning; just used for counting.
} warning_index;

/* BEWARE!!  This list must track the array "resolve_table" in sdgetout.c . */
typedef enum {
   resolve_none,
   resolve_rlg,
   resolve_la,
   resolve_ext_rlg,
   resolve_ext_la,
   resolve_slipclutch_rlg,
   resolve_slipclutch_la,
   resolve_circ_rlg,
   resolve_circ_la,
   resolve_pth_rlg,
   resolve_pth_la,
   resolve_tby_rlg,
   resolve_tby_la,
   resolve_xby_rlg,
   resolve_xby_la,
   resolve_dixie_grand,
   resolve_minigrand,
   resolve_prom,
   resolve_revprom,
   resolve_sglfileprom,
   resolve_revsglfileprom,
   resolve_circle
} resolve_kind;

typedef struct {
   resolve_kind kind;
   int distance;
} resolve_indicator;


/* These bits are used to allocate flag bits
   that appear in the "callflagsf" word of a top level calldefn block
   and the "cmd_final_flags.final" of a setup with its command block. */

/* A 3-bit field. */
#define CFLAGH__TAG_CALL_RQ_MASK          0x00000007UL
#define CFLAGH__TAG_CALL_RQ_BIT           0x00000001UL
#define CFLAGH__REQUIRES_SELECTOR         0x00000008UL
#define CFLAGH__REQUIRES_DIRECTION        0x00000010UL
#define CFLAGH__CIRC_CALL_RQ_BIT          0x00000020UL
#define CFLAGH__ODD_NUMBER_ONLY           0x00000040UL
#define CFLAGHSPARE_1                     0x00000080UL
#define CFLAGHSPARE_2                     0x00000100UL
#define CFLAGHSPARE_3                     0x00000200UL
#define CFLAGHSPARE_4                     0x00000400UL
#define CFLAGHSPARE_5                     0x00000800UL
#define CFLAGHSPARE_6                     0x00001000UL
/* We need to leave the top 8 bits free in order to accomodate the "CFLAG2" bits. */

/* These flags, and "CFLAGH__???" flags, go along for the ride, in the callflagsf
   word of a callspec.  We use symbols that have been graciously
   provided for us from database.h to tell us what bits may be safely used next
   to the heritable flags.  Note that these bits overlap the FINAL__?? bits above.
   These are used in the callflagsf word of a callspec.  The FINAL__?? bits are
   used elsewhere.  They don't mix. */

#define ESCAPE_WORD__LEFT                 CFLAGHSPARE_1
#define ESCAPE_WORD__CROSS                CFLAGHSPARE_2
#define ESCAPE_WORD__MAGIC                CFLAGHSPARE_3
#define ESCAPE_WORD__INTLK                CFLAGHSPARE_4
#define ESCAPE_WORD__GRAND                CFLAGHSPARE_5

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

   CONCPROP__USE_NUMBER         If a concept takes one number, only CONCPROP__USE_NUMBER is set.
   CONCPROP__USE_TWO_NUMBERS    If it takes two numbers both bits are set.

   CONCPROP__SHOW_SPLIT means that the concept prepares the "split_axis" bits properly
      for transmission back to the client.  Normally this is off, and the split axis bits
      will be cleared after execution of the concept.

   CONCPROP__NEED_ARG2_MATRIX means that the "arg2" word of the concept_descriptor
      block contains additional bits of the "CONCPROP__NEEDK_3X8" kind to be sent to
      "do_matrix_expansion".  This is done so that concepts with different matrix
      expansion bits can share the same concept type.
*/



#define CONCPROP__SECOND_CALL      0x00000001UL
#define CONCPROP__USE_SELECTOR     0x00000002UL
#define CONCPROP__SET_PHANTOMS     0x00000004UL
#define CONCPROP__NO_STEP          0x00000008UL

/* This is a five bit field.  CONCPROP__NEED_LOBIT marks its low bit.
   WARNING!!!  The values in this field are encoded into a bit field
   for the setup expansion/normalization tables.  It follows that
   there can't be more than 32 of them. */
#define CONCPROP__NEED_LOBIT       0x00000010UL
/* A bit mask for the "NEEDK" bits. */
#define CONCPROP__NEED_MASK        0x000001F0UL

#define CONCPROP__NEEDK_4X4        0x00000010UL
#define CONCPROP__NEEDK_2X8        0x00000020UL
#define CONCPROP__NEEDK_2X6        0x00000030UL
#define CONCPROP__NEEDK_4DMD       0x00000040UL
#define CONCPROP__NEEDK_BLOB       0x00000050UL
#define CONCPROP__NEEDK_4X6        0x00000060UL
#define CONCPROP__NEEDK_3X8        0x00000070UL
#define CONCPROP__NEEDK_3DMD       0x00000080UL
#define CONCPROP__NEEDK_1X10       0x00000090UL
#define CONCPROP__NEEDK_1X12       0x000000A0UL
#define CONCPROP__NEEDK_3X4        0x000000B0UL
#define CONCPROP__NEEDK_1X16       0x000000C0UL
#define CONCPROP__NEEDK_QUAD_1X4   0x000000D0UL
#define CONCPROP__NEEDK_TWINDMD    0x000000E0UL
#define CONCPROP__NEEDK_TWINQTAG   0x000000F0UL
#define CONCPROP__NEEDK_CTR_DMD    0x00000100UL
#define CONCPROP__NEEDK_END_DMD    0x00000110UL
#define CONCPROP__NEEDK_TRIPLE_1X4 0x00000120UL
#define CONCPROP__NEEDK_CTR_1X4    0x00000130UL
#define CONCPROP__NEEDK_END_1X4    0x00000140UL
#define CONCPROP__NEEDK_CTR_2X2    0x00000150UL
#define CONCPROP__NEEDK_END_2X2    0x00000160UL
#define CONCPROP__NEEDK_3X4_D3X4   0x00000170UL
#define CONCPROP__NEEDK_3X6        0x00000180UL
#define CONCPROP__NEEDK_4D_4PTPD   0x00000190UL
#define CONCPROP__NEEDK_4X5        0x000001A0UL
#define CONCPROP__NEEDK_2X12       0x000001B0UL
#define CONCPROP__NEEDK_DBLX       0x000001C0UL
#define CONCPROP__NEEDK_DEEPXWV    0x000001D0UL
#define CONCPROP__NEEDK_QUAD_1X3   0x000001E0UL

#define CONCPROP__NEED_ARG2_MATRIX 0x00000200UL                                   
/* spare:                          0x00000400UL */
/* spare:                          0x00000800UL */
/* spare:                          0x00010000UL */
/* spare:                          0x00020000UL */
/* spare:                          0x00040000UL */
/* spare:                          0x00080000UL */
/* spare:                          0x00100000UL */
#define CONCPROP__GET_MASK         0x00200000UL
#define CONCPROP__STANDARD         0x00400000UL
#define CONCPROP__USE_NUMBER       0x00800000UL
#define CONCPROP__USE_TWO_NUMBERS  0x01000000UL
#define CONCPROP__MATRIX_OBLIVIOUS 0x02000000UL
#define CONCPROP__PERMIT_MATRIX    0x04000000UL
#define CONCPROP__SHOW_SPLIT       0x08000000UL
#define CONCPROP__PERMIT_MYSTIC    0x10000000UL
#define CONCPROP__PERMIT_REVERSE   0x20000000UL
#define CONCPROP__PERMIT_MODIFIERS 0x40000000UL

/* This allows 96 warnings. */
/* BEWARE!!  If this is changed, this initializers for things like "no_search_warnings"
   in sdmain.cpp will need to be updated. */
#define WARNING_WORDS 3

typedef struct {
   uint32 bits[WARNING_WORDS];
} warning_info;

typedef struct {           /* This record is one state in the evolving sequence. */
   parse_block *command_root;
   setup state;
   resolve_indicator resolve_flag;
   long_boolean draw_pic;
   warning_info warnings;
   int centersp;           /* only nonzero for history[1] */
   int text_line;          /* how many lines of text existed after this item was written,
                              only meaningful if "written_history_items" is >= this index */
} configuration;

typedef struct {
   uint32 concept_prop;      /* Takes bits of the form CONCPROP__??? */
   // We wish we could put a "throw" clause on this function, but we can't.
   void (*concept_action)(setup *, parse_block *, setup *);
} concept_table_item;

typedef enum {
   error_flag_none = 0,          /* Must be zero because setjmp returns this. */
   error_flag_1_line,            /* 1-line error message, text is in error_message1. */
   error_flag_2_line,            /* 2-line error message, text is in error_message1 and
                                    error_message2. */
   error_flag_no_retry,          /* Like error_flag_1_line, but it is instantly fatal. */
   error_flag_collision,         /* collision error, message is that people collided, they are in
                                    collision_person1 and collision_person2. */
   error_flag_cant_execute,      /* unable-to-execute error, person is in collision_person1,
                                    text is in error_message1. */

   // Errors after this did not arise from call execution, so we don't
   // show the ending formation.  "Error_flag_wrong_command" is the indicator for this.

   error_flag_wrong_command,     /* clicked on something inappropriate in subcall reader. */
   error_flag_wrong_resolve_command, /* "resolve" or similar command was called
                                         in inappropriate context, text is in error_message1. */
   error_flag_show_stats,        /* wants to display stale call statistics. */
   error_flag_selector_changed,  /* warn that selector was changed during clipboard paste. */
   error_flag_formation_changed  /* warn that formation changed during clipboard paste. */
} error_flag_type;

#define zig_zag_level l_a2
#define cross_by_level l_c1
#define dixie_grand_level l_plus
#define extend_34_level l_plus
#define phantom_tandem_level l_c4a
#define intlk_triangle_level l_c2
#define beau_belle_level l_a2


/* It should be noted that the CMD_MISC__??? and RESULTFLAG__XXX bits have
   nothing to do with each other.  It is not intended that
   the flags resulting from one call be passed to the next.  In fact, that
   would be incorrect.  The CMD_MISC__??? bits should start at zero at the
   beginning of each call, and accumulate stuff as the call goes deeper into
   recursion.  The RESULTFLAG__??? bits should, in general, be the OR of the
   bits of the components of a compound call, though this may not be so for
   the elongation bits at the bottom of the word. */



#define TEST_HERITBITS(x,y) ((x).her8it & (y))


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
      See the comments in front of "get_fraction_info" in sdmoves.c for details.

   cmd_assume
      Any "assume waves" type command.

   skippable_concept
      For "<so-and-so> work <concept>", this is the concept to be skipped by some people.

   prior_elongation_bits;
      This tells, for a 2x2 setup prior to having a call executed, how that
      2x2 is elongated (due to these people being the outsides) in the east-west
      or north-south direction.  Since 2x2's are always totally canonical, the
      interpretation of the elongation direction is always absolute.  A 1 means
      the elongation is east-west.  A 2 means the elongation is north-south.
      A zero means there was no elongation. */

static const uint32 CMD_FRAC_NULL_VALUE      = 0x00000111UL;
static const uint32 CMD_FRAC_HALF_VALUE      = 0x00000112UL;
static const uint32 CMD_FRAC_LASTHALF_VALUE  = 0x00001211UL;

static const uint32 CMD_FRAC_PART_BIT        = 0x00010000UL;
static const uint32 CMD_FRAC_PART_MASK       = 0x00070000UL;
static const uint32 CMD_FRAC_THISISLAST      = 0x00080000UL;
static const uint32 CMD_FRAC_REVERSE         = 0x00100000UL;
static const uint32 CMD_FRAC_CODE_MASK       = 0x00E00000UL;    // This is a 3 bit field.

/* Here are the codes that can be inside.  We require that CMD_FRAC_CODE_ONLY be zero.
   We require that the PART_MASK field be nonzero (we use 1-based part numbering)
   when these are in use.  If the PART_MASK field is zero, the code must be zero
   (that is, CMD_FRAC_CODE_ONLY), and this stuff is not in use. */

static const uint32 CMD_FRAC_CODE_ONLY           = 0x00000000UL;
static const uint32 CMD_FRAC_CODE_ONLYREV        = 0x00200000UL;
static const uint32 CMD_FRAC_CODE_FROMTO         = 0x00400000UL;
static const uint32 CMD_FRAC_CODE_FROMTOREV      = 0x00600000UL;
static const uint32 CMD_FRAC_CODE_FROMTOREVREV   = 0x00800000UL;
static const uint32 CMD_FRAC_CODE_FROMTOMOST     = 0x00A00000UL;
static const uint32 CMD_FRAC_CODE_LATEFROMTOREV  = 0x00C00000UL;

static const uint32 CMD_FRAC_PART2_BIT       = 0x01000000UL;
static const uint32 CMD_FRAC_PART2_MASK      = 0x07000000UL;
static const uint32 CMD_FRAC_IMPROPER_BIT    = 0x08000000UL;
static const uint32 CMD_FRAC_BREAKING_UP     = 0x10000000UL;
static const uint32 CMD_FRAC_FORCE_VIS       = 0x20000000UL;
static const uint32 CMD_FRAC_LASTHALF_ALL    = 0x40000000UL;
static const uint32 CMD_FRAC_FIRSTHALF_ALL   = 0x80000000UL;


/* These flags go along for the ride, in some parts of the code (BUT NOT
   THE CALLFLAGSF WORD OF A CALLSPEC!), in the same word as the heritable flags,
   but are not part of the inheritance mechanism.  We use symbols that have been
   graciously provided for us from database.h to tell us what bits may be safely
   used next to the heritable flags. */

#define FINAL__SPLIT                             CFLAGHSPARE_1
static const uint32 FINAL__SPLIT_SQUARE_APPROVED      = CFLAGHSPARE_2;
static const uint32 FINAL__SPLIT_DIXIE_APPROVED       = CFLAGHSPARE_3;
static const uint32 FINAL__MUST_BE_TAG                = CFLAGHSPARE_4;
static const uint32 FINAL__TRIANGLE                   = CFLAGHSPARE_5;
static const uint32 FINAL__LEADTRIANGLE               = CFLAGHSPARE_6;

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

   CMD_MISC__NO_EXPAND_MATRIX means that we are at a level of recursion that no longer permits us to do the
   implicit expansion of the matrix (e.g. add outboard phantoms to turn a 2x4 into a 2x6
   if the concept "triple box" is used) that some concepts perform at the top level.

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

/* Since DFM1_CONCENTRICITY_FLAG_MASK is FF, we start at 100. */

static const uint32 CMD_MISC__EXPLICIT_MIRROR    = 0x00000100UL;
static const uint32 CMD_MISC__MATRIX_CONCEPT     = 0x00000200UL;
/* This is a 4 bit field.  For codes inside same, see "CMD_MISC__VERIFY_WAVES" below. */
static const uint32 CMD_MISC__VERIFY_MASK        = 0x00003C00UL;
static const uint32 CMD_MISC__EXPLICIT_MATRIX    = 0x00004000UL;
static const uint32 CMD_MISC__NO_EXPAND_MATRIX   = 0x00008000UL;
static const uint32 CMD_MISC__DISTORTED          = 0x00010000UL;
static const uint32 CMD_MISC__OFFSET_Z           = 0x00020000UL;
static const uint32 CMD_MISC__SAID_SPLIT         = 0x00040000UL;
static const uint32 CMD_MISC__SAID_TRIANGLE      = 0x00080000UL;
static const uint32 CMD_MISC__PUT_FRAC_ON_FIRST  = 0x00100000UL;
static const uint32 CMD_MISC__DO_AS_COUPLES      = 0x00200000UL;
static const uint32 CMD_MISC__RESTRAIN_CRAZINESS = 0x00400000UL;
static const uint32 CMD_MISC__RESTRAIN_MODIFIERS = 0x00800000UL;
static const uint32 CMD_MISC__NO_CHECK_MOD_LEVEL = 0x01000000UL;
#define             CMD_MISC__MUST_SPLIT_HORIZ     0x02000000UL
#define             CMD_MISC__MUST_SPLIT_VERT      0x04000000UL
static const uint32 CMD_MISC__NO_CHK_ELONG       = 0x08000000UL;
static const uint32 CMD_MISC__PHANTOMS           = 0x10000000UL;
static const uint32 CMD_MISC__NO_STEP_TO_WAVE    = 0x20000000UL;
static const uint32 CMD_MISC__ALREADY_STEPPED    = 0x40000000UL;
static const uint32 CMD_MISC__DOING_ENDS         = 0x80000000UL;

static const uint32 CMD_MISC__MUST_SPLIT_MASK    = (CMD_MISC__MUST_SPLIT_HORIZ|CMD_MISC__MUST_SPLIT_VERT);

/* Here are the encodings that can go into the CMD_MISC__VERIFY_MASK field.
   Zero means no verification. */
#define             CMD_MISC__VERIFY_WAVES         0x00000400UL
#define             CMD_MISC__VERIFY_2FL           0x00000800UL
#define             CMD_MISC__VERIFY_DMD_LIKE      0x00000C00UL
#define             CMD_MISC__VERIFY_QTAG_LIKE     0x00001000UL
#define             CMD_MISC__VERIFY_1_4_TAG       0x00001400UL
#define             CMD_MISC__VERIFY_3_4_TAG       0x00001800UL
#define             CMD_MISC__VERIFY_REAL_1_4_TAG  0x00001C00UL
#define             CMD_MISC__VERIFY_REAL_3_4_TAG  0x00002000UL
#define             CMD_MISC__VERIFY_REAL_1_4_LINE 0x00002400UL
#define             CMD_MISC__VERIFY_REAL_3_4_LINE 0x00002800UL
#define             CMD_MISC__VERIFY_LINES         0x00002C00UL
#define             CMD_MISC__VERIFY_COLS          0x00003000UL
#define             CMD_MISC__VERIFY_TALL6         0x00003400UL


/* Flags that reside in the "cmd_misc2_flags" word of a setup BEFORE a call is executed. */

/* The following are used for the "<anyone> work <concept>" or "snag the <anyone>" mechanism:

   CMD_MISC2__ANY_WORK or CMD_MISC2__ANY_SNAG is on if there is such an
   operation in place.  We will make the centers or ends (depending on
   CMD_MISC2__ANY_WORK_INVERT) use the next concept (or do 1/2 of the call)
   while the others skip that concept.  The schema, which is one of
   schema_concentric_2_6, schema_concentric_6_2, schema_concentric, or
   schema_single_concentric, is in the low 16 bits.

   CMD_MISC2__ANY_WORK_INVERT is only meaningful if the CMD_MISC2__ANY_WORK is on.
   It says that the ends are doing the concept, instead of the centers. */

/* The following are used for Z's.
   CMD_MISC2__IN_Z_CW and CMD_MISC2__IN_Z_CCW say that the setup is actually
   a 2x3, but the "Z" (or "each Z", or "triple Z's") concept has been given,
   and the setup should probably be turned into a 2x2.  The only exception
   is if the call takes a 2x3 starting setup but not a 2x2 (that is, the call
   is "Z axle").  In that case, the call is done directly in the 2x3, and the
   "Z" distortion is presumed not to have been in place. */

/* The following are used for "mystic" and old "snag".

   CMD_MISC2__MYSTIFY_SPLIT tells "divided_setup_move" to perform selective mirroring
   of the subsidiary setups because a concept like "mystic triple boxes" is in use.
   It is removed immediately by "divided_setup_move" after use.

   CMD_MISC2__MYSTIFY_INVERT is only meaningful when CMD_MISC2__MYSTIFY_SPLIT is on.
   It says that the concept is actually "invert mystic triple boxes" or whatever. */

/*  The following are used for what we call the "center/end" mechanism.  This
    mechanism is used for the "invert" (centers and ends) concept, as well as
    "central", "snag", and "mystic" and inverts thereof.

      CMD_MISC2__CTR_END_KMASK, when nonzero, says that one of the "central", "snag",
      or "mystic" concepts is in use.  They are all closely related.

      CMD_MISC2__CTR_END_INVERT means that the call is being inverted.  It is
      orthogonal to such issues as "snag" and "invert snag".  "Invert snag invert
      strike out" means that the physical ends do only half, and everyone do
      an invert strike out.  That is, the ends hinge but do not step & fold,
      while the centers detour.

      CMD_MISC2__CTR_END_MASK embraces all of the bits of the "center/end" mechanism.
*/

/*     The low 12 bits are used for encoding the schema if
       CMD_MISC2__ANY_WORK or CMD_MISC2__ANY_SNAG is on */

static const uint32 CMD_MISC2__IN_Z_CW           = 0x00001000UL;
static const uint32 CMD_MISC2__IN_Z_CCW          = 0x00002000UL;
static const uint32 CMD_MISC2__IN_AZ_CW          = 0x00004000UL;
static const uint32 CMD_MISC2__IN_AZ_CCW         = 0x00008000UL;
static const uint32 CMD_MISC2__IN_Z_MASK         = 0x0000F000UL;
static const uint32 CMD_MISC2_RESTRAINED_SUPER   = 0x00010000UL;

static const uint32 CMD_MISC2__MYSTIFY_SPLIT     = 0x00020000UL;
static const uint32 CMD_MISC2__MYSTIFY_INVERT    = 0x00040000UL;

static const uint32 CMD_MISC2__ANY_WORK          = 0x00080000UL;
static const uint32 CMD_MISC2__ANY_SNAG          = 0x00100000UL;
static const uint32 CMD_MISC2__ANY_WORK_INVERT   = 0x00200000UL;


/* Here are the inversion bits for the basic operations. */
#define             CMD_MISC2__INVERT_CENTRAL      0x00400000UL
#define             CMD_MISC2__INVERT_SNAG         0x00800000UL
#define             CMD_MISC2__INVERT_MYSTIC       0x01000000UL
/* Here are the basic operations we can do. */
#define             CMD_MISC2__DO_CENTRAL          0x02000000UL
#define             CMD_MISC2__CENTRAL_SNAG        0x04000000UL
#define             CMD_MISC2__CENTRAL_MYSTIC      0x08000000UL
/* This field embraces the above 3 bits. */
#define             CMD_MISC2__CTR_END_KMASK       0x0E000000UL

/* This says the the operator said "invert".  It might later cause
   a "central" to be turned into an "invert central". */
#define             CMD_MISC2__SAID_INVERT         0x10000000UL
/* This mask embraces this whole mechanism, including the "invert" bit. */
#define             CMD_MISC2__CTR_END_MASK        0x1FC00000UL
// This is a 2 bit field.
static const uint32 CMD_MISC2__DID_Z_COMPRESSMASK= 0x60000000UL;
static const uint32 CMD_MISC2__DID_Z_COMPRESSBIT = 0x20000000UL;

typedef enum {
   simple_normalize,
   normalize_before_isolated_call,
   normalize_before_isolate_not_too_strict,
   normalize_to_6,
   normalize_to_4,
   normalize_to_2,
   normalize_after_triple_squash,
   normalize_after_disconnected,
   normalize_before_merge,
   normalize_strict_matrix,
   normalize_compress_bigdmd,
   normalize_recenter
} normalize_action;

typedef enum {
   merge_strict_matrix,
   merge_c1_phantom,
   merge_c1_phantom_real,
   merge_c1phan_nocompress,
   merge_without_gaps
} merge_action;

/* We sometimes don't want to use "const" in C++,
   because initializers are screwed up.
   In particular, we need this on any structure type
   that is initialized sometimes in C files (e.g. sdtables.c)
   and sometimes in C++ files (e.g. the structure "expand_thing"
   has initializers in sdmoves.cpp as well.)
*/
#ifdef __cplusplus
#define C_const
#else
#define C_const const
#endif

typedef struct grylch {
   C_const veryshort source_indices[24];
   C_const int size;
   C_const setup_kind inner_kind;
   C_const setup_kind outer_kind;
   C_const int rot;
   C_const uint32 lillivemask;
   C_const uint32 biglivemask;
   C_const warning_index expwarning;
   C_const warning_index norwarning;
   C_const normalize_action action_level;
   C_const uint32 expandconcpropmask;
   struct grylch *next_expand;
   struct grylch *next_compress;
} expand_thing;

typedef struct grilch {
   C_const warning_index warning;
   C_const int forbidden_elongation;   /* Low 2 bits = elongation bits to forbid;
                                        "4" bit = must set elongation.
                                        Also, the "8" bit means to use "gather"
                                        and do this the other way.
                                        Also, the "16" bit means allow only step
                                        to a box, not step to a full wave. */
   C_const expand_thing *expand_lists;
   C_const setup_kind kind;
   C_const uint32 live;
   C_const uint32 dir;
   C_const uint32 dirmask;
   struct grilch *next;
} full_expand_thing;


typedef enum {    /* These control error messages that arise when we divide a setup
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
} phantest_kind;

typedef enum {
   disttest_t, disttest_nil, disttest_only_two,
   disttest_any, disttest_offset, disttest_z} disttest_kind;

enum chk_type {
   chk_none,
   chk_wave,
   chk_groups,
   chk_anti_groups,
   chk_box,
   chk_box_dbl,
   chk_indep_box,
   chk_star,
   chk_dmd_qtag,
   chk_qtag,
   chk_qbox,
   chk_peelable,
   chk_spec_directions,
   chk_sex
};


typedef enum {
   restriction_passes,
   restriction_fails,
   restriction_bad_level,
   restriction_no_item
} restriction_test_result;


typedef struct milch {
   uint32 code;
   const map_thing *the_map;
   struct milch *next;
} mapcoder;

typedef struct {
   uint32 newheritmods;
   uint32 newfinalmods;
   int before;    /* These are indices into concept_descriptor_table. */
   int after;
} concept_fixer_thing;

typedef enum {
   /* Warning!!!!  Order is important!  See all the stupid ways these are used
      in sdconc.c . */
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
} selective_key;

typedef enum {
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
} tandem_key;

enum meta_key_kind {
   meta_key_random,
   meta_key_rev_random,   // Must follow meta_key_random.
   meta_key_piecewise,
   meta_key_initially,
   meta_key_finish,
   meta_key_revorder,
   meta_key_like_a,
   meta_key_finally,
   meta_key_nth_part_work,
   meta_key_skip_nth_part,
   meta_key_shift_n,
   meta_key_shifty,
   meta_key_echo,
   meta_key_rev_echo,   // Must follow meta_key_echo.
   meta_key_shift_half,
   meta_key_shift_n_half
};

enum revert_weirdness_type {
   weirdness_off,
   weirdness_flatten_from_3,
   weirdness_otherstuff
};



extern int session_index;                                           /* in SDSI */
extern int random_number;                                           /* in SDSI */
extern char *database_filename;                                     /* in SDSI */
extern char *new_outfile_string;                                    /* in SDSI */
extern char *call_list_string;                                      /* in SDSI */
extern FILE *call_list_file;                                        /* in SDSI */

extern SDLIB_API long_boolean showing_has_stopped;                  /* in SDMATCH */
extern SDLIB_API match_result GLOB_match;                           /* in SDMATCH */
extern SDLIB_API int GLOB_space_ok;                                 /* in SDMATCH */
extern SDLIB_API int GLOB_yielding_matches;                         /* in SDMATCH */
extern SDLIB_API char GLOB_full_input    [];                        /* in SDMATCH */
extern SDLIB_API char GLOB_extension     [];                        /* in SDMATCH */
extern SDLIB_API char GLOB_extended_input[];                        /* in SDMATCH */
extern SDLIB_API int GLOB_full_input_size;                          /* in SDMATCH */

extern SDLIB_API int *concept_list;        /* indices of all concepts */
extern SDLIB_API int concept_list_length;
extern SDLIB_API int *level_concept_list; /* indices of concepts valid at current level */
extern SDLIB_API int level_concept_list_length;

extern SDLIB_API modifier_block *fcn_key_table_normal[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern SDLIB_API modifier_block *fcn_key_table_start[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern SDLIB_API modifier_block *fcn_key_table_resolve[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern SDLIB_API match_result user_match;


extern SDLIB_API command_kind search_goal;                          /* in SDPICK */

extern FILE *journal_file;                                          /* in SDMAIN */
extern Cstring menu_names[];                                        /* in SDMAIN */
extern command_list_menu_item command_menu[];                       /* in SDMAIN */
extern resolve_list_menu_item resolve_menu[];                       /* in SDMAIN */
extern startup_list_menu_item startup_menu[];                       /* in SDMAIN */
extern int last_file_position;                                      /* in SDMAIN */
extern char *sd_version_string(void);                               /* In SDMAIN */
extern long_boolean query_for_call(void);                           /* In SDMAIN */

extern int screen_height;                                           /* in SDUI-TTY */
extern int no_cursor;                                               /* in SDUI-TTY */
extern int no_console;                                              /* in SDUI-TTY */
extern int no_line_delete;                                          /* in SDUI-TTY */


// This file is used by some plain C files for data initialization.
// The plain C compiler won't like the "throw" declarations or the
// "bool" datatype.  The files that do data initialization don't
// need function prototypes anyway.

#ifdef __cplusplus

typedef struct {
   void * (*get_mem_fn)(uint32 siz);
   void * (*get_more_mem_fn)(void *oldp, uint32 siz);
   void * (*get_mem_gracefully_fn)(uint32 siz);
   void * (*get_more_mem_gracefully_fn)(void *oldp, uint32 siz);
   void (*free_mem_fn)(void *ptr);
   void (*get_date_fn)(char dest[]);
   void (*open_file_fn)(void);
   void (*close_file_fn)(void);
   void (*uims_database_error_fn)(Cstring message, Cstring call_name);
   parse_block * (*get_parse_block_fn)(void);
   void (*newline_fn)(void);
   void (*write_file_fn)(char line[]);
   long_boolean (*install_outfile_string_fn)(char newstring[]);
   int (*uims_do_abort_popup_fn)(void);
   int (*uims_do_session_init_popup_fn)(void);
   uims_reply (*uims_get_startup_command_fn)(void);
   void (*uims_set_window_title_fn)(char s[]);
   void (*uims_add_new_line_fn)(char the_line[], uint32 drawing_picture);
   void (*uims_reduce_line_count_fn)(int n);
   void (*uims_update_resolve_menu_fn)(command_kind goal,
                                       int cur, int max,
                                       resolver_display_state state);
   void (*show_match_fn)(void);
   char * (*uims_version_string_fn)(void);
   char * (*sd_version_string_fn)(void);
   uims_reply (*uims_get_resolve_command_fn)(void);
   long_boolean (*query_for_call_fn)(void);
   bool (*uims_choose_font_fn)();
   bool (*uims_print_this_fn)();
   bool (*uims_print_any_fn)();
   bool (*uims_help_manual_fn)();
   int (*uims_do_outfile_popup_fn)(char dest[]);
   int (*uims_do_header_popup_fn)(char dest[]);
   int (*uims_do_getout_popup_fn)(char dest[]);
   int (*uims_do_write_anyway_popup_fn)(void);
   int (*uims_do_delete_clipboard_popup_fn)(void);
   void (*init_error_fn)(char s[]);
   void (*uims_fatal_error_fn)(Cstring pszLine1, Cstring pszLine2);
   void (*uims_database_tick_fn)(int n);
   void (*uims_database_tick_max_fn)(int n);
   void (*uims_database_tick_end_fn)(void);
   void (*uims_create_menu_fn)(call_list_kind cl);
   long_boolean (*open_database_fn)(char *msg1, char *msg2);
   uint32 (*read_8_from_database_fn)(void);
   uint32 (*read_16_from_database_fn)(void);
   void (*close_database_fn)(void);
   long_boolean (*open_call_list_file_fn)(char filename[]);
   char * (*read_from_call_list_file_fn)(char name[], int n);
   void (*write_to_call_list_file_fn)(Const char name[]);
   long_boolean (*close_call_list_file_fn)(void);
   long_boolean (*sequence_is_resolved_fn)(void);
   long_boolean (*deposit_call_fn)(call_with_name *call, const call_conc_option_state *options);
   long_boolean (*deposit_concept_fn)(concept_descriptor *conc);
   int (*uims_do_modifier_popup_fn)(Cstring callname, modify_popup_kind kind);
   void (*create_resolve_menu_title_fn)(
      command_kind goal,
      int cur, int max,
      resolver_display_state state,
      char *title);
   void (*exit_program_fn)(int code);
   int (*generate_random_number_fn)(int modulus);
   void (*hash_nonrandom_number_fn)(int number);
   int text_line_count;
} callbackstuff;

/* VARIABLES */

extern SDLIB_API callbackstuff the_callback_block;                  /* in SDTOP */
extern SDLIB_API char error_message1[MAX_ERR_LENGTH];               /* in SDTOP */
extern SDLIB_API char error_message2[MAX_ERR_LENGTH];               /* in SDTOP */
extern SDLIB_API uint32 collision_person1;                          /* in SDTOP */
extern SDLIB_API uint32 collision_person2;                          /* in SDTOP */
extern SDLIB_API configuration *history;                            /* in SDTOP */
extern SDLIB_API int history_ptr;                                   /* in SDTOP */
extern SDLIB_API int history_allocation;                            /* in SDTOP */
extern SDLIB_API int written_history_items;                         /* in SDTOP */
extern SDLIB_API int written_history_nopic;                         /* in SDTOP */
extern SDLIB_API dance_level higher_acceptable_level[];             /* in SDTOP */
extern SDLIB_API uint32 the_topcallflags;                           /* in SDTOP */
extern SDLIB_API long_boolean there_is_a_call;                      /* in SDTOP */

extern SDLIB_API call_with_name **base_calls;                       /* in SDTOP */
extern SDLIB_API ui_option_type ui_options;                         /* in SDTOP */
extern SDLIB_API int whole_sequence_low_lim;                        /* in SDTOP */
extern SDLIB_API long_boolean enable_file_writing;                  /* in SDTOP */
extern SDLIB_API Cstring cardinals[];                               /* in SDTOP */
extern SDLIB_API Cstring ordinals[];                                /* in SDTOP */
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
extern SDLIB_API call_list_mode_t glob_call_list_mode;              /* in SDTOP */
extern SDLIB_API int abs_max_calls;                                 /* in SDTOP */
extern SDLIB_API int max_base_calls;                                /* in SDTOP */
extern SDLIB_API Cstring *tagger_menu_list[NUM_TAGGER_CLASSES];     /* in SDTOP */
extern SDLIB_API Cstring *selector_menu_list;                       /* in SDTOP */
extern SDLIB_API Cstring *circcer_menu_list;                        /* in SDTOP */


extern SDLIB_API call_with_name **tagger_calls[NUM_TAGGER_CLASSES]; /* in SDTOP */
extern SDLIB_API call_with_name **circcer_calls;                    /* in SDTOP */
extern SDLIB_API uint32 number_of_taggers[NUM_TAGGER_CLASSES];      /* in SDTOP */
extern SDLIB_API uint32 number_of_circcers;                         /* in SDTOP */
extern SDLIB_API long_boolean diagnostic_mode;                      /* in SDTOP */
extern SDLIB_API int singing_call_mode;                             /* in SDTOP */
extern SDLIB_API parse_state_type parse_state;                      /* in SDTOP */
extern SDLIB_API call_conc_option_state current_options;            /* in SDTOP */
extern SDLIB_API long_boolean allowing_all_concepts;                /* in SDTOP */
extern SDLIB_API long_boolean allowing_minigrand;                   /* in SDTOP */
extern SDLIB_API long_boolean using_active_phantoms;                /* in SDTOP */
extern SDLIB_API const call_conc_option_state null_options;         /* in SDTOP */
extern SDLIB_API call_conc_option_state verify_options;             /* in SDTOP */
extern SDLIB_API long_boolean verify_used_number;                   /* in SDTOP */
extern SDLIB_API long_boolean verify_used_direction;                /* in SDTOP */
extern SDLIB_API long_boolean verify_used_selector;                 /* in SDTOP */
extern SDLIB_API int uims_menu_index;                               /* in SDTOP */
extern SDLIB_API int last_direction_kind;                           /* in SDTOP */
extern SDLIB_API interactivity_state interactivity;                 /* in SDTOP */
extern SDLIB_API char database_version[81];                         /* in SDTOP */
extern SDLIB_API long_boolean testing_fidelity;                     /* in SDTOP */
extern SDLIB_API dance_level level_threshholds[];                   /* in SDTOP */
extern SDLIB_API int allowing_modifications;                        /* in SDTOP */
extern SDLIB_API int hashed_randoms;                                /* in SDTOP */

extern SDLIB_API selector_kind selector_for_initialize;             /* in SDINIT */
extern SDLIB_API direction_kind direction_for_initialize;           /* in SDINIT */
extern SDLIB_API int number_for_initialize;                         /* in SDINIT */

extern SDLIB_API error_flag_type global_error_flag;                 /* in SDUTIL */
extern SDLIB_API uims_reply global_reply;                           /* in SDUTIL */
extern SDLIB_API int global_age;                                    /* in SDUTIL */
extern SDLIB_API configuration *clipboard;                          /* in SDUTIL */
extern SDLIB_API int clipboard_size;                                /* in SDUTIL */
extern SDLIB_API long_boolean wrote_a_sequence;                     /* in SDUTIL */
extern SDLIB_API long_boolean retain_after_error;                   /* in SDUTIL */
extern SDLIB_API char outfile_string[];                             /* in SDUTIL */
extern SDLIB_API char header_comment[];                             /* in SDUTIL */
extern SDLIB_API long_boolean need_new_header_comment;              /* in SDUTIL */
extern SDLIB_API int sequence_number;                               /* in SDUTIL */
extern SDLIB_API int starting_sequence_number;                      /* in SDUTIL */
extern SDLIB_API Cstring filename_strings[];                        /* in SDUTIL */
extern SDLIB_API Cstring concept_key_table[];                       /* in SDUTIL */

extern SDLIB_API concept_table_item concept_table[];                /* in SDCONCPT */


extern uint32 global_tbonetest;                                     /* in SDCONCPT */
extern uint32 global_livemask;                                      /* in SDCONCPT */
extern uint32 global_selectmask;                                    /* in SDCONCPT */
extern uint32 global_tboneselect;                                   /* in SDCONCPT */

extern parse_block *last_magic_diamond;                             /* in SDTOP */
extern warning_info no_search_warnings;                             /* in SDTOP */
extern warning_info conc_elong_warnings;                            /* in SDTOP */
extern warning_info dyp_each_warnings;                              /* in SDTOP */
extern warning_info useless_phan_clw_warnings;                      /* in SDTOP */
extern int concept_sublist_sizes[NUM_CALL_LIST_KINDS];              /* in SDTOP */
extern short int *concept_sublists[NUM_CALL_LIST_KINDS];            /* in SDTOP */
extern int good_concept_sublist_sizes[NUM_CALL_LIST_KINDS];         /* in SDTOP */
extern short int *good_concept_sublists[NUM_CALL_LIST_KINDS];       /* in SDTOP */

extern long_boolean selector_used;                                  /* in SDPREDS */
extern long_boolean direction_used;                                 /* in SDPREDS */
extern long_boolean number_used;                                    /* in SDPREDS */
extern long_boolean mandatory_call_used;                            /* in SDPREDS */
extern predicate_descriptor pred_table[];                           /* in SDPREDS */
extern int selector_preds;                                          /* in SDPREDS */

#endif

#ifdef __cplusplus
extern "C" {
#endif

extern SDLIB_API selector_item selector_list[];                     /* in SDTABLES */

extern SDLIB_API Cstring warning_strings[];                         /* in SDTABLES */
extern SDLIB_API setup_attr setup_attrs[];                          /* in SDTABLES */
extern SDLIB_API int begin_sizes[];                                 /* in SDTABLES */
extern SDLIB_API startinfo startinfolist[];                         /* in SDTABLES */

extern id_bit_table id_bit_table_2x5_z[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_2x6_pg[];                          /* in SDTABLES */
extern id_bit_table id_bit_table_bigdmd_wings[];                    /* in SDTABLES */
extern id_bit_table id_bit_table_bigbone_wings[];                   /* in SDTABLES */
extern id_bit_table id_bit_table_bighrgl_wings[];                   /* in SDTABLES */
extern id_bit_table id_bit_table_bigdhrgl_wings[];                  /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_h[];                           /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_ctr6[];                        /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_offset[];                      /* in SDTABLES */
extern id_bit_table id_bit_table_3x4_corners[];                     /* in SDTABLES */
extern id_bit_table id_bit_table_butterfly[];                       /* in SDTABLES */
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
extern id_bit_table id_bit_table_3x6_with_1x6[];                    /* in SDTABLES */
extern cm_thing conc_init_table[];                                  /* in SDTABLES */
extern const fixer fdhrgl;                                          /* in SDTABLES */
extern const fixer f323;                                            /* in SDTABLES */
extern const fixer f2x4far;                                         /* in SDTABLES */
extern const fixer f2x4near;                                        /* in SDTABLES */
extern const fixer f4dmdiden;                                       /* in SDTABLES */
extern const fixer fixmumble;                                       /* in SDTABLES */
extern const fixer fixfrotz;                                        /* in SDTABLES */
extern const fixer fixwhuzzis;                                      /* in SDTABLES */
extern const fixer fixgizmo;                                        /* in SDTABLES */

extern full_expand_thing rear_1x2_pair;
extern full_expand_thing rear_2x2_pair;
extern full_expand_thing rear_bone_pair;
extern full_expand_thing step_8ch_pair;
extern full_expand_thing step_qtag_pair;
extern full_expand_thing step_2x2h_pair;
extern full_expand_thing step_2x2v_pair;
extern full_expand_thing step_spindle_pair;
extern full_expand_thing step_dmd_pair;
extern full_expand_thing step_tgl_pair;
extern full_expand_thing step_ptpd_pair;
extern full_expand_thing step_qtgctr_pair;

extern full_expand_thing touch_init_table1[];
extern full_expand_thing touch_init_table2[];
extern full_expand_thing touch_init_table3[];
#define NEEDMASK(K) (1<<((K)/(CONCPROP__NEED_LOBIT)))
extern expand_thing expand_init_table[];

extern const expand_thing comp_qtag_2x4_stuff;                      /* in SDTABLES */
extern const expand_thing exp_2x3_qtg_stuff;                        /* in SDTABLES */
extern const expand_thing exp_2x2_2x4_stuff;                        /* in SDTABLES */
extern const expand_thing exp_4x4_4x6_stuff_a;                      /* in SDTABLES */
extern const expand_thing exp_4x4_4x6_stuff_b;                      /* in SDTABLES */
extern const expand_thing exp_4x4_4dm_stuff_a;                      /* in SDTABLES */
extern const expand_thing exp_4x4_4dm_stuff_b;                      /* in SDTABLES */
extern const expand_thing exp_c1phan_4x4_stuff1;                    /* in SDTABLES */
extern const expand_thing exp_c1phan_4x4_stuff2;                    /* in SDTABLES */
extern const expand_thing exp_dmd_323_stuff;
extern const expand_thing exp_1x4_dmd_stuff;
extern const expand_thing exp_2x4_qtg_stuff;
extern const expand_thing exp_1x2_dmd_stuff;
extern const expand_thing exp_qtg_3x4_stuff;
extern const expand_thing exp_1x2_hrgl_stuff;
extern const expand_thing exp_dmd_hrgl_stuff;
extern const expand_thing exp_dmd_hrgl_disc_stuff;

extern const coordrec tgl3_0;                                       /* in SDTABLES */
extern const coordrec tgl3_1;                                       /* in SDTABLES */
extern const coordrec tgl4_0;                                       /* in SDTABLES */
extern const coordrec tgl4_1;                                       /* in SDTABLES */
extern const coordrec squeezethingglass;                            /* in SDTABLES */
extern const coordrec squeezethinggal;                              /* in SDTABLES */
extern const coordrec squeezethingqtag;                             /* in SDTABLES */
extern const coordrec squeezething4dmd;                             /* in SDTABLES */
extern const coordrec squeezefinalglass;                            /* in SDTABLES */
extern const coordrec truck_to_ptpd;                                /* in SDTABLES */
extern const coordrec truck_to_deepxwv;                             /* in SDTABLES */
extern const coordrec press_4dmd_4x4;                               /* in SDTABLES */
extern const coordrec press_4dmd_qtag1;                             /* in SDTABLES */
extern const coordrec press_4dmd_qtag2;                             /* in SDTABLES */
extern const coordrec press_qtag_4dmd1;                             /* in SDTABLES */
extern const coordrec press_qtag_4dmd2;                             /* in SDTABLES */
extern const coordrec acc_crosswave;                                /* in SDTABLES */
extern const tgl_map *c1tglmap1[];                                  /* in SDTABLES */
extern const tgl_map *c1tglmap2[];                                  /* in SDTABLES */
extern const tgl_map *dbqtglmap1[];                                 /* in SDTABLES */
extern const tgl_map *dbqtglmap2[];                                 /* in SDTABLES */
extern const tgl_map *qttglmap1[];                                  /* in SDTABLES */
extern const tgl_map *qttglmap2[];                                  /* in SDTABLES */
extern const tgl_map *bdtglmap1[];                                  /* in SDTABLES */
extern const tgl_map *bdtglmap2[];                                  /* in SDTABLES */
extern const tgl_map *rgtglmap1[];                                  /* in SDTABLES */
extern sel_item sel_init_table[];                                   /* in SDTABLES */
extern map_thing map_p8_tgl4;                                       /* in SDTABLES */
extern map_thing map_spndle_once_rem;                               /* in SDTABLES */
extern map_thing map_1x3dmd_once_rem;                               /* in SDTABLES */
extern map_thing map_lh_zzztgl;                                     /* in SDTABLES */
extern map_thing map_rh_zzztgl;                                     /* in SDTABLES */
extern map_thing map_2x2v;                                          /* in SDTABLES */
extern map_thing map_2x4_magic;                                     /* in SDTABLES */
extern map_thing map_qtg_magic;                                     /* in SDTABLES */
extern map_thing map_qtg_intlk;                                     /* in SDTABLES */
extern map_thing map_qtg_magic_intlk;                               /* in SDTABLES */
extern map_thing map_ptp_magic;                                     /* in SDTABLES */
extern map_thing map_ptp_intlk;                                     /* in SDTABLES */
extern map_thing map_ptp_magic_intlk;                               /* in SDTABLES */
extern map_thing map_2x4_diagonal;                                  /* in SDTABLES */
extern map_thing map_2x4_int_pgram;                                 /* in SDTABLES */
extern map_thing map_2x4_trapezoid;                                 /* in SDTABLES */
extern map_thing map_4x4_ns;                                        /* in SDTABLES */
extern map_thing map_4x4_ew;                                        /* in SDTABLES */
extern map_thing map_vsplit_f;                                      /* in SDTABLES */
extern map_thing map_stairst;                                       /* in SDTABLES */
extern map_thing map_ladder;                                        /* in SDTABLES */
extern map_thing map_but_o;                                         /* in SDTABLES */
extern map_thing map_4x4v;                                          /* in SDTABLES */
extern map_thing map_blocks;                                        /* in SDTABLES */
extern map_thing map_trglbox;                                       /* in SDTABLES */
extern map_thing map_trglbox12a;                                    /* in SDTABLES */
extern map_thing map_trglbox12b;                                    /* in SDTABLES */
extern map_thing map_2x3_0134;                                      /* in SDTABLES */
extern map_thing map_2x3_1245;                                      /* in SDTABLES */
extern map_thing map_2x3_1234;                                      /* in SDTABLES */
extern map_thing map_2x3_0145;                                      /* in SDTABLES */
extern map_thing map_1x8_1x6;                                       /* in SDTABLES */
extern map_thing map_rig_1x6;                                       /* in SDTABLES */
extern map_thing map_3mdmd_3d;                                      /* in SDTABLES */
extern map_thing map_ov_hrg_1;                                      /* in SDTABLES */
extern map_thing map_ov_gal_1;                                      /* in SDTABLES */
extern map_thing map_3o_qtag_1;                                     /* in SDTABLES */
extern map_thing map_tgl4_1;                                        /* in SDTABLES */
extern map_thing map_tgl4_2;                                        /* in SDTABLES */
extern map_thing map_qtag_2x3;                                      /* in SDTABLES */
extern map_thing map_2x3_rmvr;                                      /* in SDTABLES */
extern map_thing map_2x3_rmvs;                                      /* in SDTABLES */
extern map_thing map_dbloff1;                                       /* in SDTABLES */
extern map_thing map_dbloff2;                                       /* in SDTABLES */
extern map_thing map_dhrgl1;                                        /* in SDTABLES */
extern map_thing map_dhrgl2;                                        /* in SDTABLES */
extern map_thing map_dbgbn1;                                        /* in SDTABLES */
extern map_thing map_dbgbn2;                                        /* in SDTABLES */
extern map_thing map_off1x81;                                       /* in SDTABLES */
extern map_thing map_off1x82;                                       /* in SDTABLES */
extern map_thing map_dqtag1;                                        /* in SDTABLES */
extern map_thing map_dqtag2;                                        /* in SDTABLES */
extern map_thing map_dqtag3;                                        /* in SDTABLES */
extern map_thing map_dqtag4;                                        /* in SDTABLES */
extern map_thing map_stw3a;                                         /* in SDTABLES */
extern map_thing map_stw3b;                                         /* in SDTABLES */
extern map_thing map_stw3c;                                         /* in SDTABLES */
extern map_thing map_stw3d;                                         /* in SDTABLES */
extern clw3_thing clw3_table[];                                     /* in SDTABLES */
extern map_thing map_trngl_box1;                                    /* in SDTABLES */
extern map_thing map_trngl_box2;                                    /* in SDTABLES */
extern map_thing map_inner_box;                                     /* in SDTABLES */
extern map_thing map_lh_c1phana;                                    /* in SDTABLES */
extern map_thing map_lh_c1phanb;                                    /* in SDTABLES */
extern map_thing map_rh_c1phana;                                    /* in SDTABLES */
extern map_thing map_rh_c1phanb;                                    /* in SDTABLES */
extern map_thing map_lh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_lh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_3;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_2;                                     /* in SDTABLES */
extern map_thing map_lh_s2x3_7;                                     /* in SDTABLES */
extern map_thing map_rh_s2x3_7;                                     /* in SDTABLES */
extern map_thing map_d1x10;                                         /* in SDTABLES */
extern map_thing map_lz12;                                          /* in SDTABLES */
extern map_thing map_rz12;                                          /* in SDTABLES */
extern map_thing map_tgl451;                                        /* in SDTABLES */
extern map_thing map_tgl452;                                        /* in SDTABLES */
extern map_thing map_dmd_1x1;                                       /* in SDTABLES */
extern map_thing map_star_1x1;                                      /* in SDTABLES */
extern map_thing map_qtag_f0;                                       /* in SDTABLES */
extern map_thing map_qtag_f1;                                       /* in SDTABLES */
extern map_thing map_qtag_f2;                                       /* in SDTABLES */
extern map_thing map_diag2a;                                        /* in SDTABLES */
extern map_thing map_diag2b;                                        /* in SDTABLES */
extern map_thing map_diag23a;                                       /* in SDTABLES */
extern map_thing map_diag23b;                                       /* in SDTABLES */
extern map_thing map_diag23c;                                       /* in SDTABLES */
extern map_thing map_diag23d;                                       /* in SDTABLES */
extern map_thing map_f2x8_4x4;                                      /* in SDTABLES */
extern map_thing map_w4x4_4x4;                                      /* in SDTABLES */
extern map_thing map_f2x8_2x8;                                      /* in SDTABLES */
extern map_thing map_w4x4_2x8;                                      /* in SDTABLES */
extern map_thing map_emergency1;                                    /* in SDTABLES */
extern map_thing map_emergency2;                                    /* in SDTABLES */
extern map_thing *maps_3diag[4];                                    /* in SDTABLES */
extern map_thing *maps_3diagwk[4];                                  /* in SDTABLES */
extern map_thing map_init_table2[];                                 /* in SDTABLES */

/* in SDCTABLE */

extern SDLIB_API concept_descriptor concept_descriptor_table[];
extern SDLIB_API call_with_name **main_call_lists[NUM_CALL_LIST_KINDS];
extern SDLIB_API int number_of_calls[NUM_CALL_LIST_KINDS];
extern SDLIB_API dance_level calling_level;

extern SDLIB_API nice_setup_info_item nice_setup_info[];
extern SDLIB_API int phantom_concept_index;
extern SDLIB_API int matrix_2x8_concept_index;
extern SDLIB_API int cross_concept_index;
extern SDLIB_API int magic_concept_index;
extern SDLIB_API int intlk_concept_index;
extern SDLIB_API int left_concept_index;
extern SDLIB_API int grand_concept_index;
extern SDLIB_API int general_concept_offset;
extern SDLIB_API int general_concept_size;

extern SDLIB_API concept_descriptor centers_concept;
extern concept_descriptor special_magic;
extern concept_descriptor special_interlocked;
extern SDLIB_API concept_descriptor mark_end_of_list;
extern concept_descriptor marker_decline;
extern SDLIB_API concept_descriptor marker_concept_mod;
extern SDLIB_API concept_descriptor marker_concept_comment;
extern concept_descriptor marker_concept_supercall;

extern concept_fixer_thing concept_fixer_table[];

extern int *concept_offset_tables[];
extern int *concept_size_tables[];
extern Cstring concept_menu_strings[];

#ifdef __cplusplus
}
#endif



// This file is used by some plain C files for data initialization.
// The plain C compiler won't like classes, "throw" declarations or the
// "bool" datatype.  The files that do data initialization don't
// need function prototypes anyway.


#ifdef __cplusplus

/* In SDPREDS */

extern long_boolean selectp(setup *ss, int place) THROW_DECL;

/* In SDGETOUT */

SDLIB_API void write_resolve_text(long_boolean doing_file);
SDLIB_API uims_reply full_resolve(void);
extern int concepts_in_place(void);
extern int reconcile_command_ok(void);
extern int resolve_command_ok(void);
extern int nice_setup_command_ok(void);
SDLIB_API void create_resolve_menu_title(
   command_kind goal,
   int cur,
   int max,
   resolver_display_state state,
   char *title);
SDLIB_API void initialize_getout_tables(void);


/* In SDBASIC */

class collision_collector {
public:

   // Simple constructor, takes argument saying whether collisions will be legal.
   collision_collector(bool allow):
      allow_collisions(allow),
      collision_mask(0),
      callflags1(CFLAG1_TAKE_RIGHT_HANDS),  // Default is that collisions are legal.
      assume_ptr((assumption_thing *) 0),
      force_mirror_warn(false),
      cmd_misc_flags(0),
      collision_appears_illegal(1),    // Halfway between "appears_illegal"
                                       // and not -- use table item.
      result_mask(0)
   {}

   // Glorious constructor, takes all sorts of stuff.
   collision_collector(bool mirror, setup_command *cmd, const calldefn *callspec):
      allow_collisions(true),
      collision_mask(0),
      callflags1(callspec->callflags1),
      assume_ptr(&cmd->cmd_assume),
      force_mirror_warn(mirror),
      cmd_misc_flags(cmd->cmd_misc_flags),
      collision_appears_illegal(0),    // May change to 2 as call progresses.
      result_mask(0)
      {}

   void note_prefilled_result(setup *result)
      {
         for (int i=0; i<=setup_attrs[result->kind].setup_limits; i++)
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
   bool force_mirror_warn;
   uint32 cmd_misc_flags;
   int collision_appears_illegal;
   uint32 result_mask;
};


extern void mirror_this(setup *s) THROW_DECL;

extern void do_stability(uint32 *personp, stability stab, int turning) THROW_DECL;

extern long_boolean check_restriction(
   setup *ss,
   assumption_thing restr,
   long_boolean instantiate_phantoms,
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

extern long_boolean divide_for_magic(
   setup *ss,
   uint32 heritflags_to_check,
   setup *result) THROW_DECL;

extern long_boolean do_simple_split(
   setup *ss,
   uint32 prefer_1x4,   /* 1 means prefer 1x4, 2 means this is 1x8 and do not recompute id. */
   setup *result) THROW_DECL;

extern void do_call_in_series(
   setup *sss,
   long_boolean dont_enforce_consistent_split,
   long_boolean roll_transparent,
   long_boolean normalize,
   bool qtfudged) THROW_DECL;

extern void brute_force_merge(const setup *res1, const setup *res2,
                              bool allow_collisions, setup *result) THROW_DECL;

extern void drag_someone_and_move(setup *ss, parse_block *parseptr, setup *result) THROW_DECL;

extern void anchor_someone_and_move(setup *ss, parse_block *parseptr, setup *result) THROW_DECL;

extern void process_number_insertion(uint32 mod_word);

extern uint32 process_new_fractions(
   int numer,
   int denom,
   uint32 incoming_fracs,
   uint32 reverse_orderbit,   /* Low bit on mean treat as if we mean "do the last M/N". */
   long_boolean allow_improper,
   long_boolean *improper_p) THROW_DECL;

extern long_boolean fill_active_phantoms_and_move(setup *ss, setup *result) THROW_DECL;

extern void move_perhaps_with_active_phantoms(setup *ss, setup *result) THROW_DECL;

extern void impose_assumption_and_move(setup *ss, setup *result) THROW_DECL;

extern void move(
   setup *ss,
   bool qtfudged,
   setup *result) THROW_DECL;

/* In SDISTORT */

extern void prepare_for_call_in_series(setup *result, setup *ss);

extern void minimize_splitting_info(setup *ss, uint32 other_info);

extern void initialize_map_tables(void);

extern void remove_z_distortion(setup *ss) THROW_DECL;

extern void remove_tgl_distortion(setup *ss) THROW_DECL;

extern const map_thing *get_map_from_code(uint32 map_encoding);

extern void new_divided_setup_move(
   setup *ss,
   uint32 map_encoding,
   phantest_kind phancontrol,
   long_boolean recompute_id,
   setup *result) THROW_DECL;

extern void divided_setup_move(
   setup *ss,
   const map_thing *maps,
   phantest_kind phancontrol,
   long_boolean recompute_id,
   setup *result) THROW_DECL;

extern void new_overlapped_setup_move(setup *ss, uint32 map_encoding,
   uint32 *masks, setup *result) THROW_DECL;

extern void overlapped_setup_move(setup *ss, const map_thing *maps,
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

extern void phantom_2x4_move(
   setup *ss,
   int lineflag,
   phantest_kind phantest,
   const map_thing *maps,
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

extern void common_spot_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

extern void triangle_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL;

/* In SDCONCPT */

extern long_boolean do_big_concept(
   setup *ss,
   setup *result) THROW_DECL;

/* In SDTAND */

extern void tandem_couples_move(
   setup *ss,
   selector_kind selector,
   int twosome,           /* solid=0 / twosome=1 / solid-to-twosome=2 / twosome-to-solid=3 */
   int fraction,          /* number, if doing fractional twosome/solid */
   int phantom,           /* normal=0 phantom=1 general-gruesome=2 gruesome-with-wave-check=3 */
   tandem_key key,
   uint32 mxn_bits,
   long_boolean phantom_pairing_ok,
   setup *result) THROW_DECL;

extern void initialize_tandem_tables(void);

/* In SDCONC */

extern void concentric_move(
   setup *ss,
   setup_command *cmdin,
   setup_command *cmdout,
   calldef_schema analyzer,
   uint32 modifiersin1,
   uint32 modifiersout1,
   long_boolean recompute_id,
   uint32 specialoffsetmapcode,
   setup *result) THROW_DECL;

extern uint32 get_multiple_parallel_resultflags(setup outer_inners[], int number) THROW_DECL;

extern void initialize_conc_tables(void);
extern void initialize_sel_tables(void);

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
   long_boolean others,
   int arg2,
   uint32 override_selector,
   selector_kind selector_to_use,
   long_boolean concentric_rules,
   setup *result) THROW_DECL;

extern void inner_selective_move(
   setup *ss,
   setup_command *cmd1,
   setup_command *cmd2,
   selective_key indicator,
   long_boolean others,
   int arg2,
   uint32 override_selector,
   selector_kind selector_to_use,
   uint32 modsa1,
   uint32 modsb1,
   setup *result) THROW_DECL;

/* In SDTOP */

extern void compress_setup(const expand_thing *thing, setup *stuff) THROW_DECL;

extern void expand_setup(const expand_thing *thing, setup *stuff) THROW_DECL;

extern void update_id_bits(setup *ss);

extern void touch_or_rear_back(
   setup *scopy,
   bool did_mirror,
   int callflags1) THROW_DECL;

extern void do_matrix_expansion(
   setup *ss,
   uint32 concprops,
   long_boolean recompute_id) THROW_DECL;

SDLIB_API void initialize_sdlib(void);

extern long_boolean check_for_concept_group(
   const parse_block *parseptrcopy,
   long_boolean want_all_that_other_stuff,
   concept_kind *k_p,
   uint32 *need_to_restrain_p,   /* 1=(if not doing echo), 2=(yes, always) */
   parse_block **parseptr_skip_p) THROW_DECL;

NORETURN1 SDLIB_API void fail(const char s[]) THROW_DECL NORETURN2;

NORETURN1 SDLIB_API void fail_no_retry(const char s[]) THROW_DECL NORETURN2;

NORETURN1 extern void fail2(const char s1[], const char s2[]) THROW_DECL NORETURN2;

NORETURN1 extern void failp(uint32 id1, const char s[]) THROW_DECL NORETURN2;

NORETURN1 SDLIB_API void specialfail(Const char s[]) THROW_DECL NORETURN2;

extern void warn(warning_index w);

extern restriction_test_result verify_restriction(
   setup *ss,
   assumption_thing tt,
   long_boolean instantiate_phantoms,
   long_boolean *failed_to_instantiate) THROW_DECL;

extern callarray *assoc(begin_kind key, setup *ss, callarray *spec) THROW_DECL;

extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index) THROW_DECL;

extern void clear_people(setup *z);

inline uint32 rotperson(uint32 n, int amount)
{ if (n == 0) return 0; else return (n + amount) & ~064; }

inline uint32 rotcw(uint32 n)
{ if (n == 0) return 0; else return (n + 011) & ~064; }

inline uint32 rotccw(uint32 n)
{ if (n == 0) return 0; else return (n + 033) & ~064; }

inline void clear_person(setup *resultpeople, int resultplace)
{
   resultpeople->people[resultplace].id1 = 0;
   resultpeople->people[resultplace].id2 = 0;
}

extern uint32 copy_person(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace);

extern uint32 copy_rot(setup *resultpeople, int resultplace, const setup *sourcepeople, int sourceplace, int rotamount);

extern void swap_people(setup *ss, int oneplace, int otherplace);

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
   long_boolean check_errors,
   uint64 *final_concepts) THROW_DECL;

extern parse_block *really_skip_one_concept(
   parse_block *incoming,
   concept_kind *k_p,
   uint32 *need_to_restrain_p,   /* 1=(if not doing echo), 2=(yes, always) */
   parse_block **parseptr_skip_p) THROW_DECL;

extern long_boolean fix_n_results(int arity, setup_kind goal, setup z[], uint32 *rotstatep)
   THROW_DECL;

extern resolve_indicator resolve_p(setup *s);

extern long_boolean warnings_are_unacceptable(long_boolean strict);

extern void normalize_setup(setup *ss, normalize_action action) THROW_DECL;

SDLIB_API void toplevelmove(void) THROW_DECL;

SDLIB_API void finish_toplevelmove(void) THROW_DECL;

SDLIB_API long_boolean deposit_call_tree(modifier_block *anythings, parse_block *save1, int key);

extern long_boolean do_subcall_query(
   int snumber,
   parse_block *parseptr,
   parse_block **newsearch,
   long_boolean this_is_tagger,
   long_boolean this_is_tagger_circcer,
   call_with_name *orig_call);

SDLIB_API void print_id_error(int n);

extern call_list_kind find_proper_call_list(setup *s);

class fraction_info {
 public:
   fraction_info(int n) :
      reverse_order(FALSE),
      instant_stop(99),  // If not 99, says to stop instantly after doing one part,
                         // and to report (in RESULTFLAG__PART_COMPLETION_BITS bit)
                         // whether that part was the last part.
      do_half_of_last_part(0),
      do_last_half_of_first_part(0),
      highlimit(n),
      start_point(0),
      end_point(n-1),
      fetch_index(0),
      client_index(0),
      fetch_total(n),
      client_total(n),
      subcall_incr(1)
      {}

   void demand_this_part_exists() THROW_DECL
      {
         if (fetch_index >= fetch_total || fetch_index < 0)
            fail("The indicated part number doesn't exist.");
      }

   void get_fraction_info(uint32 frac_flags,
                          uint32 callflags1,
                          revert_weirdness_type doing_weird_revert) THROW_DECL;

   uint32 get_fracs_for_this_part();

   bool query_instant_stop(uint32 & result_flag_word);

   void fudge_client_total(int delta)
      {
         client_total += delta;
         highlimit = client_total;
         end_point = highlimit-1;
      }

   bool not_yet_in_active_section()
      {
         if (reverse_order) {
            if (client_index > start_point) return true;
         }
         else {
            if (client_index < start_point) return true;
         }
         return false;
      }

   bool ran_off_active_section()
      {
         if (reverse_order) {
            if (client_index < end_point) return true;
         }
         else {
            if (client_index > end_point) return true;
         }
         return false;
      }

   inline bool this_starts_at_beginning()
      { return
           start_point == 0 &&
           !do_last_half_of_first_part &&
           !reverse_order;
      }

 public:
   long_boolean reverse_order;
   int instant_stop;
   long_boolean first_call;
   uint32 do_half_of_last_part;
   uint32 do_last_half_of_first_part;
   int highlimit;
 private:
   int start_point;
   int end_point;
 public:
   int fetch_index;
   int client_index;
   int fetch_total;
   int client_total;
   int subcall_incr;
};


/* In SDUTIL */

SDLIB_API Const char *get_escape_string(char c);
SDLIB_API void write_history_line(int history_index, const char *header,
                                  long_boolean picture, file_write_flag write_to_file);
SDLIB_API void unparse_call_name(Cstring name, char *s, call_conc_option_state *options);
SDLIB_API void print_recurse(parse_block *thing, int print_recurse_arg);
SDLIB_API void clear_screen(void);
SDLIB_API void write_header_stuff(long_boolean with_ui_version, uint32 act_phan_flags);
extern void writechar(char src);
SDLIB_API void newline(void);
extern void open_text_line(void);
SDLIB_API void doublespace_file(void);
SDLIB_API void writestuff(const char *s);
extern parse_block *mark_parse_blocks(void);
extern void release_parse_blocks_to_mark(parse_block *mark_point);
extern parse_block *copy_parse_tree(parse_block *original_tree);
SDLIB_API parse_block *get_parse_block(void);
extern void reset_parse_tree(parse_block *original_tree, parse_block *final_head);
extern void save_parse_state(void);
extern long_boolean restore_parse_state(void);
SDLIB_API void string_copy(char **dest, Cstring src);
SDLIB_API void display_initial_history(int upper_limit, int num_pics);
extern void initialize_parse(void);
SDLIB_API void run_program();

/* In SDINIT */

SDLIB_API void start_sel_dir_num_iterator();
SDLIB_API long_boolean iterate_over_sel_dir_num(
   long_boolean enable_selector_iteration,
   long_boolean enable_direction_iteration,
   long_boolean enable_number_iteration);
SDLIB_API void build_database(call_list_mode_t call_list_mode);
SDLIB_API void initialize_menus(call_list_mode_t call_list_mode);

/* In SDMATCH */

SDLIB_API void do_accelerator_spec(Cstring qq);
SDLIB_API void erase_matcher_input();
SDLIB_API int delete_matcher_word();
SDLIB_API void matcher_initialize();
SDLIB_API void matcher_setup_call_menu(call_list_kind cl);
SDLIB_API int match_user_input(
   int which_commands,
   long_boolean show,
   long_boolean show_verify,
   long_boolean only_want_extension);

/* In SDPICK */

SDLIB_API void reset_internal_iterators(void);
SDLIB_API selector_kind do_selector_iteration(long_boolean allow_iteration);
SDLIB_API direction_kind do_direction_iteration(void);
SDLIB_API void do_number_iteration(int howmanynumbers,
                                   uint32 odd_number_only,
                                   long_boolean allow_iteration,
                                   uint32 *number_list);
SDLIB_API void do_circcer_iteration(uint32 *circcp);
SDLIB_API long_boolean do_tagger_iteration(uint32 tagclass,
                                           uint32 *tagg,
                                           uint32 numtaggers,
                                           call_with_name **tagtable);
SDLIB_API call_with_name *do_pick(void);
SDLIB_API concept_descriptor *pick_concept(long_boolean already_have_concept_in_place);
SDLIB_API resolve_goodness_test get_resolve_goodness_info(void);
SDLIB_API long_boolean pick_allow_multiple_items(void);
SDLIB_API void start_pick(void);
SDLIB_API void end_pick(void);
SDLIB_API long_boolean forbid_call_with_mandatory_subcall(void);
SDLIB_API long_boolean allow_random_subcall_pick(void);

/* In SDUI */

extern char *uims_version_string(void);
extern void uims_process_command_line(int *argcp, char ***argvp);
extern void uims_display_help(void);
extern void uims_display_ui_intro_text(void);
extern void uims_create_menu(call_list_kind cl);
extern void uims_postinitialize(void);
extern long_boolean uims_open_session(int argc, char **argv);
extern void uims_set_window_title(char s[]);
extern void uims_bell(void);
extern int uims_do_comment_popup(char dest[]);
extern bool uims_choose_font();
extern bool uims_print_this();
extern bool uims_print_any();
extern bool uims_help_manual();
extern int uims_do_outfile_popup(char dest[]);
extern int uims_do_header_popup(char dest[]);
extern int uims_do_getout_popup(char dest[]);
extern int uims_do_write_anyway_popup(void);
extern int uims_do_delete_clipboard_popup(void);
extern int uims_do_abort_popup(void);
extern int uims_do_session_init_popup(void);
extern int uims_do_neglect_popup(char dest[]);
extern int uims_do_selector_popup(void);
extern int uims_do_direction_popup(void);
extern int uims_do_circcer_popup(void);
extern int uims_do_tagger_popup(int tagger_class);
extern int uims_do_modifier_popup(Cstring callname, modify_popup_kind kind);
extern uint32 uims_get_number_fields(int nnumbers, long_boolean forbid_zero);
extern void uims_reduce_line_count(int n);
extern void uims_add_new_line(char the_line[], uint32 drawing_picture);
extern void show_match(void);
extern uims_reply uims_get_startup_command(void);
extern long_boolean uims_get_call_command(uims_reply *reply_p);
extern uims_reply uims_get_resolve_command(void);
extern void uims_update_resolve_menu(command_kind goal, int cur, int max, resolver_display_state state);
extern void uims_terminate(void);
extern void uims_database_tick_max(int n);
extern void uims_database_tick(int n);
extern void uims_database_tick_end(void);
extern void uims_database_error(Cstring message, Cstring call_name);
extern void uims_bad_argument(Cstring s1, Cstring s2, Cstring s3);
extern void uims_debug_print(Cstring s);		/* Alan's code only */
extern void uims_fatal_error(Cstring pszLine1, Cstring pszLine2);
NORETURN1 extern void uims_final_exit(int code) NORETURN2;

extern void ttu_final_option_setup();

/* Print the help message appropriate for the switches that we support. */
extern void ttu_display_help(void);

/* Change the title bar (or whatever it's called) on the window. */
extern void ttu_set_window_title(char s[]);

/* Initialize this package. */
extern void ttu_initialize(void);

/* Terminate this package. */
extern void ttu_terminate(void);

/* Get number of lines to use for "more" processing.  This number is
   not used for any other purpose -- the rest of the program is not concerned
   with the "screen" size. */

extern int get_lines_for_more(void);

/* Clear the current line, leave cursor at left edge. */
extern void clear_line(void);

/* Backspace the cursor and clear the rest of the line, presumably
   erasing the last character. */
extern void rubout(void);

/* Move cursor up "n" lines and then clear rest of screen. */
extern void erase_last_n(int n);

/* Write a line.  The text may or may not have a newline at the end. */
/* This may or may not be after a prompt and/or echoed user input. */
extern void put_line(const char the_line[]);

/* Write a single character on the current output line. */
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

extern int get_char(void);

/* Get string from input, up to <newline>, with echoing and editing.
   Return it without the final <newline>. */
extern void get_string(char *dest, int max);

/* Ring the bell, or whatever. */
extern void ttu_bell(void);

extern void initialize_signal_handlers(void);
extern void refresh_input(void);

/* in SDSI */

extern void general_initialize(void);
extern int generate_random_number(int modulus);
extern void hash_nonrandom_number(int number);
extern void *get_mem(uint32 siz);
extern void *get_mem_gracefully(uint32 siz);
extern void *get_more_mem(void *oldp, uint32 siz);
extern void *get_more_mem_gracefully(void *oldp, uint32 siz);
extern void free_mem(void *ptr);
extern void get_date(char dest[]);
extern void open_file(void);
extern void close_file(void);
extern void print_line(Cstring s);
// ***** This next one actually wants NORETURN stuff.
extern void init_error(char s[]);
extern long_boolean parse_level(Cstring s, dance_level *levelp);
extern char *read_from_call_list_file(char name[], int n);
extern void write_to_call_list_file(Const char name[]);
extern long_boolean close_call_list_file(void);
extern long_boolean install_outfile_string(char newstring[]);
extern long_boolean get_first_session_line(void);
extern long_boolean get_next_session_line(char *dest);
extern void prepare_to_read_menus(void);
extern void initialize_misc_lists(void);
extern long_boolean open_session(int argc, char **argv);
extern int process_session_info(Cstring *error_msg);
extern long_boolean open_call_list_file(char filename[]);
extern long_boolean open_accelerator_region(void);
extern long_boolean get_accelerator_line(char line[]);
extern void close_init_file(void);
NORETURN1 extern void final_exit(int code) NORETURN2;
extern long_boolean open_database(char *msg1, char *msg2);
extern uint32 read_8_from_database(void);
extern uint32 read_16_from_database(void);
extern void close_database(void);
extern int parse_number(char junk[]);

extern void unparse_number(int arg, char dest[]);
extern void write_file(char line[]);
extern void fill_in_neglect_percentage(char junk[], int n);

/* in SDMAIN */

extern long_boolean sequence_is_resolved(void);
extern long_boolean deposit_call(call_with_name *call, const call_conc_option_state *options);
extern long_boolean deposit_concept(concept_descriptor *conc);
extern int sdmain(int argc, char *argv[]);
/*NORETURN1*/ extern void exit_program(int code) /*NORETURN2*/;

#endif
