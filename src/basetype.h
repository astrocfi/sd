/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

#include "database.h"


#define NULLCONCEPTPTR (concept_descriptor *) 0
#define NULLCALLSPEC (callspec_block *) 0

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
} ui_option_type;


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

/* For ui_command_select: */
/* BEWARE!!  This next definition must be keyed to the array "title_string"
   in sdgetout.cpp, and maybe stuff in the UI.  For example, see "command_menu"
   in sdutil.cpp. */
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
   command_create_tidal_wave
} command_kind;
#define NUM_COMMAND_KINDS (((int) command_create_tidal_wave)+1)

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
   start_select_change_header_comment
} start_select_kind;
#define NUM_START_SELECT_KINDS (((int) start_select_change_header_comment)+1)

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
   callspec_block *callspec;
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
   initializers instantiating these in sdinit.c (test_setup_*) and in sdtables.c
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
   long_boolean (*predfunc) (setup *, int, int, int, Const long int *);
   Const long int *extra_stuff;
} predicate_descriptor;

typedef struct predptr_pair_struct {
   predicate_descriptor *pred;
   struct predptr_pair_struct *next;
   /* Dynamically allocated to whatever size is required. */
   uint16 arr[4];
} predptr_pair;


#define MAPCODE(setupkind,num,mapkind,rot) ((((int)(setupkind)) << 10) | (((int)(mapkind)) << 4) | (((num)-1) << 1) | (rot))


#define LOOKUP_NONE     0x1
#define LOOKUP_DIST_DMD 0x2
#define LOOKUP_DIST_BOX 0x4
#define LOOKUP_DIST_CLW 0x8
#define LOOKUP_DIAG_CLW 0x10
#define LOOKUP_OFFS_CLW 0x20
#define LOOKUP_DISC     0x40
#define LOOKUP_IGNORE   0x80
#define LOOKUP_Z        0x100


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
   Const setup_kind kind;
   Const struct qwerty *other;
   Const veryshort mapqt1[8];   /* In quarter-tag: first triangle (upright),
                                   then second triangle (inverted), then idle. */
   Const veryshort mapcp1[8];   /* In C1 phantom: first triangle (inverted),
                                   then second triangle (upright), then idle. */
   Const veryshort mapbd1[8];   /* In bigdmd. */
   Const veryshort map241[8];   /* In 2x4. */
   Const veryshort map261[8];   /* In 2x6. */
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
