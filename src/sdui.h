/* SD -- square dance caller's helper.

    Copyright (C) 1990-2000  William B. Ackerman.
    Copyright (C) 1993 Stephen Gildea
    Copyright (C) 1993 Alan Snyder

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 33. */

#ifdef WIN32
#ifndef SDLIB_API
#define SDLIB_API __declspec(dllimport)
#endif
#else
#define SDLIB_API
#endif


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

typedef struct {
   char full_input    [INPUT_TEXTLINE_SIZE+1]; /* the current user input */
   char extension     [INPUT_TEXTLINE_SIZE+1]; /* the extension for the current pattern */
   char extended_input[INPUT_TEXTLINE_SIZE+1]; /* the maximal common extension to the user input */
   int full_input_size;        /* Number of characters in full_input, not counting null. */
   int space_ok;               /* space is a legitimate next input character */
   int yielding_matches;       /* how many matches are yielding */
   match_result result;        /* value of the first or exact matching pattern */
} match_state;

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


/* In SDGETOUT */

SDLIB_API uims_reply full_resolve(void);
SDLIB_API void initialize_getout_tables(void);
SDLIB_API void write_resolve_text(long_boolean doing_file);
SDLIB_API void create_resolve_menu_title(
   command_kind goal,
   int cur,
   int max,
   resolver_display_state state,
   char *title);

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
SDLIB_API void matcher_initialize(void);
SDLIB_API void matcher_setup_call_menu(call_list_kind cl);
SDLIB_API int match_user_input(
   int which_commands,
   long_boolean show,
   long_boolean show_verify);

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

/* In SDSI */

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
extern int parse_number(char junk[]);

extern void unparse_number(int arg, char dest[]);
extern void write_file(char line[]);
extern void fill_in_neglect_percentage(char junk[], int n);

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
extern void uims_choose_font(long_boolean in_startup) /* THROW_DECL */;
extern void uims_print_this(long_boolean in_startup) /* THROW_DECL */;
extern void uims_print_any(long_boolean in_startup);
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

extern int session_index;                                           /* in SDSI */
extern int random_number;                                           /* in SDSI */
extern char *database_filename;                                     /* in SDSI */
extern char *new_outfile_string;                                    /* in SDSI */
extern char *call_list_string;                                      /* in SDSI */
extern FILE *call_list_file;                                        /* in SDSI */


extern SDLIB_API match_state static_ss;
extern SDLIB_API long_boolean verify_has_stopped;

#ifdef __cplusplus
extern "C" {
#endif

extern SDLIB_API selector_item selector_list[];                     /* in SDTABLES */

extern SDLIB_API Cstring warning_strings[];                         /* in SDTABLES */
extern SDLIB_API setup_attr setup_attrs[];                          /* in SDTABLES */
extern SDLIB_API int begin_sizes[];                                 /* in SDTABLES */
extern SDLIB_API startinfo startinfolist[];                         /* in SDTABLES */

extern SDLIB_API concept_descriptor concept_descriptor_table[];     /* in SDCTABLE */
extern SDLIB_API call_with_name **main_call_lists[NUM_CALL_LIST_KINDS]; /* in SDCTABLE */
extern SDLIB_API int number_of_calls[NUM_CALL_LIST_KINDS];          /* in SDCTABLE */
extern SDLIB_API dance_level calling_level;                         /* in SDCTABLE */

extern SDLIB_API nice_setup_info_item nice_setup_info[];            /* in SDCTABLE */
extern SDLIB_API int phantom_concept_index;                         /* in SDCTABLE */
extern SDLIB_API int matrix_2x8_concept_index;                      /* in SDCTABLE */
extern SDLIB_API int cross_concept_index;                           /* in SDCTABLE */
extern SDLIB_API int magic_concept_index;                           /* in SDCTABLE */
extern SDLIB_API int intlk_concept_index;                           /* in SDCTABLE */
extern SDLIB_API int left_concept_index;                            /* in SDCTABLE */
extern int grand_concept_index;                                     /* in SDCTABLE */
extern SDLIB_API int general_concept_offset;                        /* in SDCTABLE */
extern SDLIB_API int general_concept_size;                          /* in SDCTABLE */

extern SDLIB_API concept_descriptor centers_concept;                /* in SDCTABLE */
extern concept_descriptor special_magic;                            /* in SDCTABLE */
extern concept_descriptor special_interlocked;                      /* in SDCTABLE */
extern SDLIB_API concept_descriptor mark_end_of_list;               /* in SDCTABLE */
extern concept_descriptor marker_decline;                           /* in SDCTABLE */
extern SDLIB_API concept_descriptor marker_concept_mod;             /* in SDCTABLE */
extern SDLIB_API concept_descriptor marker_concept_comment;         /* in SDCTABLE */
extern concept_descriptor marker_concept_supercall;                 /* in SDCTABLE */

#ifdef __cplusplus
}
#endif

// In SDMATCH

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

extern int screen_height;                                           /* in SDUI-TTY */
extern int no_cursor;                                               /* in SDUI-TTY */
extern int no_console;                                              /* in SDUI-TTY */
extern int no_line_delete;                                          /* in SDUI-TTY */


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
static const uint32 RESULTFLAG__DID_LAST_PART        = 0x00000004UL;
static const uint32 RESULTFLAG__PARTS_ARE_KNOWN      = 0x00000008UL;
static const uint32 RESULTFLAG__EXPAND_TO_2X3        = 0x00000010UL;
static const uint32 RESULTFLAG__NEED_DIAMOND         = 0x00000020UL;
static const uint32 RESULTFLAG__IMPRECISE_ROT        = 0x00000040UL;
// This is a six bit field.
static const uint32 RESULTFLAG__SPLIT_AXIS_FIELDMASK = 0x00001F80UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_XMASK     = 0x00000380UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_XBIT      = 0x00000080UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_YMASK     = 0x00001C00UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_YBIT      = 0x00000400UL;
static const uint32 RESULTFLAG__SPLIT_AXIS_SEPARATION= 3;

static const uint32 RESULTFLAG__ACTIVE_PHANTOMS_ON   = 0x00002000UL;
static const uint32 RESULTFLAG__ACTIVE_PHANTOMS_OFF  = 0x00004000UL;
static const uint32 RESULTFLAG__SECONDARY_DONE       = 0x00008000UL;
static const uint32 RESULTFLAG__YOYO_FINISHED        = 0x00010000UL;
static const uint32 RESULTFLAG__TWISTED_FINISHED     = 0x00020000UL;
static const uint32 RESULTFLAG__SPLIT_FINISHED       = 0x00040000UL;
static const uint32 RESULTFLAG__NO_REEVALUATE        = 0x00080000UL;
static const uint32 RESULTFLAG__DID_Z_COMPRESSION    = 0x00100000UL;
static const uint32 RESULTFLAG__VERY_ENDS_ODD        = 0x00200000UL;
static const uint32 RESULTFLAG__VERY_CTRS_ODD        = 0x00400000UL;
static const uint32 RESULTFLAG__DID_TGL_EXPANSION    = 0x00800000UL;

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
   warn__tasteless_slide_thru  /* If this ceases to be last, look 2 lines below! */
} warning_index;
#define NUM_WARNINGS (((int) warn__tasteless_slide_thru)+1)

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
   in sdmain.c will need to be updated. */
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
   void (*uims_choose_font_fn)(long_boolean in_startup) /* THROW_DECL */;
   void (*uims_print_this_fn)(long_boolean in_startup) /* THROW_DECL */;
   void (*uims_print_any_fn)(long_boolean in_startup);
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


/* This file is used by some plain C files for data initialization.
   The plain C compiler won't like the "throw" declarations.
   The files that do data initialization don't need function prototypes anyway.
*/

#ifdef __cplusplus

/* in SDSI */

// ***** This next one actually wants NORETURN stuff.
extern void init_error(char s[]);
extern long_boolean open_session(int argc, char **argv);
extern int process_session_info(Cstring *error_msg);
extern long_boolean open_call_list_file(char filename[]);
extern long_boolean open_accelerator_region(void);
extern long_boolean get_accelerator_line(char line[]);
extern void close_init_file(void);
NORETURN1 extern void final_exit(int code) NORETURN2;

/* in SDUTIL */

SDLIB_API void run_program();


/* in SDMAIN */

extern long_boolean sequence_is_resolved(void);
extern long_boolean deposit_call(call_with_name *call, const call_conc_option_state *options);
extern long_boolean deposit_concept(concept_descriptor *conc);
extern int sdmain(int argc, char *argv[]);
/*NORETURN1*/ extern void exit_program(int code) /*NORETURN2*/;

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

/* In SDTOP */

SDLIB_API void initialize_sdlib(void);
NORETURN1 SDLIB_API void fail(const char s[]) THROW_DECL NORETURN2;
NORETURN1 SDLIB_API void fail_no_retry(const char s[]) THROW_DECL NORETURN2;
NORETURN1 SDLIB_API void specialfail(Const char s[]) THROW_DECL NORETURN2;

inline uint32 rotperson(uint32 n, int amount)
{ if (n == 0) return 0; else return (n + amount) & ~064; }

extern resolve_indicator resolve_p(setup *s);
extern long_boolean warnings_are_unacceptable(long_boolean strict);
SDLIB_API void toplevelmove(void) THROW_DECL;
SDLIB_API void finish_toplevelmove(void) THROW_DECL;
SDLIB_API long_boolean deposit_call_tree(modifier_block *anythings, parse_block *save1, int key);
SDLIB_API Const char *get_escape_string(char c);
SDLIB_API void write_history_line(int history_index, const char *header,
                                  long_boolean picture, file_write_flag write_to_file);
SDLIB_API void unparse_call_name(Cstring name, char *s, call_conc_option_state *options);
SDLIB_API void print_recurse(parse_block *thing, int print_recurse_arg);
SDLIB_API void clear_screen(void);
SDLIB_API void write_header_stuff(long_boolean with_ui_version, uint32 act_phan_flags);
SDLIB_API parse_block *get_parse_block(void);
extern void writechar(char src);
SDLIB_API void newline(void);
SDLIB_API void writestuff(const char *s);
SDLIB_API void doublespace_file(void);
SDLIB_API void string_copy(char **dest, Cstring src);
SDLIB_API void display_initial_history(int upper_limit, int num_pics);
SDLIB_API void print_id_error(int n);



// More stuff!


/* VARIABLES */

/* In SDMAIN */

extern char *sd_version_string(void);
extern long_boolean query_for_call(void);

#endif
