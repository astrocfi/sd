/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/*
 * sdui.h - header file for interface between sdui-tty.c
 * and the low level character subsystem.
 * Time-stamp: <93/06/25 19:39:06 gildea>
 * Copyright 1993 Stephen Gildea
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Stephen Gildea <gildea@lcs.mit.edu> January 1993
 */
#define NULLCONCEPTPTR (concept_descriptor *) 0
#define NULLCALLSPEC (callspec_block *) 0

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


/* BEWARE!!  This list must track the array "concept_table" in sdconcpt.c . */
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
   concept_quad_boxes,
   concept_quad_boxes_together,
   concept_triple_boxes,
   concept_triple_boxes_together,
   concept_triple_diamonds,
   concept_triple_diamonds_together,
   concept_quad_diamonds,
   concept_quad_diamonds_together,
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
   MPKIND__OFFS_L_HALF,
   MPKIND__OFFS_R_HALF,
   MPKIND__OFFS_L_FULL,
   MPKIND__OFFS_R_FULL,
   MPKIND__OFFS_L_HALF_SPECIAL,
   MPKIND__OFFS_R_HALF_SPECIAL,
   MPKIND__OFFS_L_FULL_SPECIAL,
   MPKIND__OFFS_R_FULL_SPECIAL,
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
   MPKIND_DBLBENTCW,
   MPKIND_DBLBENTCCW,
   MPKIND__SPEC_ONCEREM,
   MPKIND__SPEC_TWICEREM
} mpkind;

typedef struct skrilch {
   Const veryshort maps[40];
   Const mpkind map_kind;
   Const short int warncode;
   Const short int arity;
   Const setup_kind outer_kind;
   Const setup_kind inner_kind;
   Const uint32 rot;
   Const int vert;
   uint32 code;
   struct skrilch *next;
} map_thing;

typedef struct {
   Cstring name;
   Const concept_kind kind;
   Const uint32 concparseflags;   /* See above. */
   Const dance_level level;
   Const struct {
      Const map_thing *maps;
      Const uint32 arg1;
      Const uint32 arg2;
      Const uint32 arg3;
      Const uint32 arg4;
      Const uint32 arg5;
   } value;
   Cstring menu_name;
} concept_descriptor;

/* These are the values returned by "uims_get_command". */

typedef enum {
   ui_special_concept,  /* Not a real return; used only for fictional purposes
                              in the user interface; never appears in the rest of the program. */
   ui_command_select,   /* (normal/resolve) User chose one of the special buttons like "resolve" or "quit". */
   ui_resolve_select,   /* (resolve only) User chose one of the various actions peculiar to resolving. */
   ui_start_select,     /* (startup only) User chose something. This is the only outcome in startup mode. */
   ui_concept_select,   /* (normal only) User selected a concept. */
   ui_call_select       /* (normal only) User selected a call from the current call menu. */
} uims_reply;

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
   selector_cpls4_1
} selector_kind;
#define last_selector_kind ((int) selector_cpls4_1)

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

/* BEWARE!!  There is a static initializer for this, "null_options", in sdmain.c
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

typedef struct {
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
      } def;            /* if schema = schema_by_def */
      struct {
         by_def_item innerdef;
         by_def_item outerdef;
      } conc;           /* if schema = any of the concentric ones. */
   } stuff;
   /* This is the "pretty" name -- "@" escapes have been changed to things like "<N>".
      If there are no escapes, this just points to the stuff below.
      If escapes are present, it points to allocated storage elsewhere.
      Either way, it persists throughout the program. */
   Cstring menu_name;
   /* Dynamically allocated to whatever size is required, will have trailing null.
      This is the name as it appeared in the database, with "@" escapes. */
   char name[4];
} callspec_block;

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
   callspec_block *call_ptr;
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
    special_index_quote_anything = -7
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

typedef struct glock {
   concept_descriptor *concept;   /* the concept or end marker */
   callspec_block *call;          /* if this is end mark, gives the call; otherwise unused */
   callspec_block *call_to_print; /* the original call, for printing (sometimes the field
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

/* In each case, an integer or enum is deposited into the global variable uims_menu_index.  Its interpretation
   depends on which of the replies above was given.  For some of the replies, it gives the index
   into a menu.  For "ui_start_select" it is a start_select_kind.
   For other replies, it is one of the following constants: */

/* BEWARE!!  This list must track the array "startup_commands" in sdmatch.c . */
/* BEWARE!!  This list must track the array "startup_resources" in sdui-x11.c . */
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
   start_select_toggle_act,
   start_select_toggle_retain,
   start_select_toggle_nowarn_mode,
   start_select_toggle_singer,
   start_select_toggle_singer_backward,
   start_select_init_session_file,
   start_select_change_outfile,
   start_select_change_header_comment
} start_select_kind;
#define NUM_START_SELECT_KINDS (((int) start_select_change_header_comment)+1)

/* For ui_command_select: */
/* BEWARE!!  This next definition must be keyed to the array "title_string"
   in sdgetout.c, and maybe stuff in the UI.  For example, see "command_commands"
   in sdui-tty.c. */
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
   command_simple_mods,
   command_all_mods,
   command_toggle_conc_levels,
   command_toggle_act_phan,
   command_toggle_retain_after_error,
   command_toggle_nowarn_mode,
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
   command_create_tidal_wave
} command_kind;
#define NUM_COMMAND_KINDS (((int) command_create_tidal_wave)+1)

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
   resolve_command_write_this
} resolve_command_kind;
#define NUM_RESOLVE_COMMAND_KINDS (((int) resolve_command_write_this)+1)

typedef struct {
   Cstring command_name;
   command_kind action;
} command_list_menu_item;

typedef struct {
   Cstring command_name;
   resolve_command_kind action;
} resolve_list_menu_item;

typedef enum {
   file_write_no,
   file_write_double
} file_write_flag;

/* Warning!  Do not move these around without checking carefully.  Values are changed
   by simple incrementing, so order is important. */
typedef enum {
   interactivity_database_init,
   interactivity_no_query_at_all,    /* Used when pasting from clipboard.  All subcalls,
                                        selectors, numbers, etc. must be filled in already.
                                        If not, it is a bug. */
   interactivity_verify,
   interactivity_normal,
   interactivity_starting_first_scan,
   interactivity_in_first_scan,
   interactivity_in_second_scan,
   interactivity_in_random_search
} interactivity_state;

typedef enum {
   resolver_display_ok,
   resolver_display_searching,
   resolver_display_failed
} resolver_display_state;

/* Values returned by the various popup routines: */
#define POPUP_DECLINE 0
#define POPUP_ACCEPT  1
#define POPUP_ACCEPT_WITH_STRING 2

extern Cstring concept_key_table[];                                 /* in SDTABLES */

/* In SDGETOUT */

extern void create_resolve_menu_title(command_kind goal, int cur, int max, resolver_display_state state, char *title);

/* In SDSI */

extern void *get_mem(uint32 siz);
extern void *get_mem_gracefully(uint32 siz);
extern void *get_more_mem(void *oldp, uint32 siz);
extern void *get_more_mem_gracefully(void *oldp, uint32 siz);
extern void free_mem(void *ptr);

/* In SDINIT */

extern void initialize_menus(call_list_mode_t call_list_mode);

/* In SDSI */

extern long_boolean parse_level(Cstring s, dance_level *levelp);
extern char *read_from_call_list_file(char name[], int n);
extern void write_to_call_list_file(Const char name[]);
extern long_boolean close_call_list_file(void);
extern long_boolean install_outfile_string(char newstring[]);
extern long_boolean get_first_session_line(void);
extern long_boolean get_next_session_line(char *dest);
extern void prepare_to_read_menus(void);
extern void initialize_misc_lists(void);
extern long_boolean open_database(char *msg1, char *msg2);
extern uint32 read_8_from_database(void);
extern uint32 read_16_from_database(void);
extern void close_database(void);

/* In SDUTIL */

extern void exit_program(int code) nonreturning;
extern long_boolean deposit_call_tree(modifier_block *anythings, parse_block *save1, int key);
extern uint32 the_topcallflags;
extern long_boolean there_is_a_call;


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
extern void uims_add_new_line(char the_line[]);
extern uims_reply uims_get_startup_command(void);
extern long_boolean uims_get_call_command(uims_reply *reply_p);
extern uims_reply uims_get_resolve_command(void);
extern void uims_begin_search(command_kind goal);
extern void uims_update_resolve_menu(command_kind goal, int cur, int max, resolver_display_state state);
extern int uims_begin_reconcile_history(int currentpoint, int maxpoint);
extern int uims_end_reconcile_history(void);
extern void uims_terminate(void);
extern void uims_database_tick_max(int n);
extern void uims_database_tick(int n);
extern void uims_database_tick_end(void);
extern void uims_database_error(Cstring message, Cstring call_name);
extern void uims_bad_argument(Cstring s1, Cstring s2, Cstring s3);
extern void uims_debug_print(Cstring s);		/* Alan's code only */
extern void uims_fatal_error(Cstring pszLine1, Cstring pszLine2);
extern void uims_final_exit(int code) nonreturning;

extern Cstring menu_names[];                                        /* in SDTABLES */

extern long_boolean use_escapes_for_drawing_people;                 /* in SDUTIL */
extern char *pn1;                                                   /* in SDUTIL */
extern char *pn2;                                                   /* in SDUTIL */
extern char *direc;                                                 /* in SDUTIL */

extern long_boolean open_session(int argc, char **argv);
extern int process_session_info(Cstring *error_msg);
extern long_boolean open_call_list_file(char filename[]);
extern long_boolean open_accelerator_region(void);
extern long_boolean get_accelerator_line(char line[]);
extern void close_init_file(void);
extern void final_exit(int code) nonreturning;

extern interactivity_state interactivity;                           /* in SDMAIN */
extern long_boolean testing_fidelity;                               /* in SDMAIN */
extern selector_kind selector_for_initialize;                       /* in SDMAIN */
extern int number_for_initialize;                                   /* in SDMAIN */
extern Cstring *tagger_menu_list[NUM_TAGGER_CLASSES];
extern Cstring *selector_menu_list;
extern Cstring *circcer_menu_list;
extern call_conc_option_state verify_options;                       /* in SDMAIN */
extern long_boolean verify_used_number;                             /* in SDMAIN */
extern long_boolean verify_used_selector;                           /* in SDMAIN */
extern int allowing_modifications;                                  /* in SDMAIN */
extern long_boolean allowing_all_concepts;                          /* in SDMAIN */
extern long_boolean using_active_phantoms;                          /* in SDMAIN */
#ifdef OLD_ELIDE_BLANKS_JUNK
extern long_boolean elide_blanks;                                   /* in SDMAIN */
#endif
extern long_boolean retain_after_error;                             /* in SDMAIN */
extern int singing_call_mode;                                       /* in SDMAIN */
extern long_boolean diagnostic_mode;                                /* in SDMAIN */
extern call_conc_option_state current_options;                      /* in SDMAIN */
extern uint32 selector_iterator;                                    /* in SDMAIN */
extern uint32 direction_iterator;                                   /* in SDMAIN */
extern uint32 number_iterator;                                      /* in SDMAIN */
extern uint32 tagger_iterator;                                      /* in SDMAIN */
extern uint32 circcer_iterator;                                     /* in SDMAIN */

extern int session_index;                                           /* in SDSI */
extern int num_command_commands;                                    /* in SDSI */
extern Cstring *command_commands;                                   /* in SDSI */
extern command_kind *command_command_values;                        /* in SDSI */
extern int number_of_resolve_commands;                              /* in SDSI */
extern Cstring* resolve_command_strings;                            /* in SDSI */
extern resolve_command_kind *resolve_command_values;                /* in SDSI */
extern int no_graphics;                                             /* in SDSI */
extern int no_intensify;                                            /* in SDSI */
extern int no_color;                                                /* in SDSI */
extern int no_sound;                                                /* in SDSI */
extern int color_by_couple;                                         /* in SDSI */
extern int random_number;                                           /* in SDSI */
extern int hashed_randoms;                                          /* in SDSI */
extern char *database_filename;                                     /* in SDSI */
extern char *new_outfile_string;                                    /* in SDSI */
extern char *call_list_string;                                      /* in SDSI */

extern command_list_menu_item command_menu[];                       /* in SDTABLES */
extern resolve_list_menu_item resolve_menu[];                       /* in SDTABLES */
extern Cstring startup_commands[];                                  /* in SDTABLES */
extern Cstring getout_strings[];                                    /* in SDTABLES */
extern Cstring filename_strings[];                                  /* in SDTABLES */
extern dance_level level_threshholds[];                             /* in SDTABLES */
extern dance_level higher_acceptable_level[];                       /* in SDTABLES */

extern Const call_conc_option_state null_options;                   /* in SDMAIN */
extern concept_descriptor concept_descriptor_table[];               /* in SDCTABLE */
extern callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];       /* in SDCTABLE */
extern int number_of_calls[NUM_CALL_LIST_KINDS];                    /* in SDCTABLE */
extern dance_level calling_level;                                   /* in SDCTABLE */

extern long_boolean enable_file_writing;                            /* in SDUTIL */
extern long_boolean singlespace_mode;                               /* in SDUTIL */
extern long_boolean nowarn_mode;                                    /* in SDUTIL */
extern Cstring cardinals[];                                         /* in SDUTIL */
extern Cstring ordinals[];                                          /* in SDUTIL */
extern selector_item selector_list[];                               /* in SDUTIL */
extern Cstring direction_names[];                                   /* in SDUTIL */
extern int last_direction_kind;                                     /* in SDUTIL */

extern long_boolean deposit_call(callspec_block *call, Const call_conc_option_state *options);
extern long_boolean deposit_concept(concept_descriptor *conc);

extern int *concept_list;        /* indices of all concepts */
extern int concept_list_length;
extern int *level_concept_list; /* indices of concepts valid at current level */
extern int level_concept_list_length;

extern modifier_block *fcn_key_table_normal[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern modifier_block *fcn_key_table_start[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern modifier_block *fcn_key_table_resolve[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern match_result user_match;

extern concept_descriptor special_magic;                            /* in SDCTABLE */
extern concept_descriptor special_interlocked;                      /* in SDCTABLE */
extern concept_descriptor mark_end_of_list;                         /* in SDCTABLE */
extern concept_descriptor marker_decline;                           /* in SDCTABLE */
extern concept_descriptor marker_concept_mod;                       /* in SDCTABLE */extern concept_descriptor marker_concept_comment;                   /* in SDCTABLE */
extern concept_descriptor marker_concept_supercall;                 /* in SDCTABLE */

extern int abs_max_calls;                                           /* in SDMAIN */
extern int max_base_calls;                                          /* in SDMAIN */
extern callspec_block **base_calls;                                 /* in SDMAIN */
extern uint32 number_of_taggers[NUM_TAGGER_CLASSES];                /* in SDMAIN */
extern callspec_block **tagger_calls[NUM_TAGGER_CLASSES];           /* in SDMAIN */
extern uint32 number_of_circcers;                                   /* in SDMAIN */
extern callspec_block **circcer_calls;                              /* in SDMAIN */
extern char outfile_string[];                                       /* in SDMAIN */
extern char header_comment[];                                       /* in SDMAIN */
extern long_boolean need_new_header_comment;                        /* in SDMAIN */
extern call_list_mode_t glob_call_list_mode;                        /* in SDMAIN */
extern int sequence_number;                                         /* in SDMAIN */
extern int starting_sequence_number;                                /* in SDMAIN */
extern int last_file_position;                                      /* in SDMAIN */
extern int global_age;                                              /* in SDMAIN */
extern parse_state_type parse_state;                                /* in SDMAIN */
extern int uims_menu_index;                                         /* in SDMAIN */
extern long_boolean uims_menu_cross;                                /* in SDMAIN */
extern long_boolean uims_menu_magic;                                /* in SDMAIN */
extern long_boolean uims_menu_intlk;                                /* in SDMAIN */
extern long_boolean uims_menu_left;                                 /* in SDMAIN */
extern long_boolean uims_menu_grand;                                /* in SDMAIN */
extern char database_version[81];                                   /* in SDMAIN */
extern int whole_sequence_low_lim;                                  /* in SDMAIN */

extern parse_block *get_parse_block(void);

extern void ttu_final_option_setup(int *use_escapes_for_drawing_people_p,
                                  char *pn1, char *pn2, char **direc_p);

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

extern FILE *journal_file;
extern int screen_height;
extern int no_cursor;
extern int no_console;
extern int no_line_delete;
/* These are duplicates of declarations in sd.h .  We don't want the
   console interface modules to need sd.h . */
extern int no_graphics;
extern int no_intensify;
extern int no_color;
extern int diagnostic_mode;
extern int color_by_couple;


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
extern void get_string(char *dest);

/* Ring the bell, or whatever. */
extern void ttu_bell(void);

extern void initialize_signal_handlers(void);
extern void refresh_input(void);
