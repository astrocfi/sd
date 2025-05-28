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


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#ifndef SDLIB_API
#define SDLIB_API __declspec(dllimport)
#endif
#else
#define SDLIB_API
#endif

#ifdef __CPLUSPLUS
#define EXPORT /*extern "C"*/ SDLIB_API
#define MAYBECALLBACK CALLBACK
#else
#define EXPORT extern
#define MAYBECALLBACK
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

/* In each case, an integer or enum is deposited into the global variable uims_menu_index.  Its interpretation
   depends on which of the replies above was given.  For some of the replies, it gives the index
   into a menu.  For "ui_start_select" it is a start_select_kind.
   For other replies, it is one of the following constants: */

/* BEWARE!!  This list must track the array "startup_commands" in sdtables.c . */
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
   start_select_init_session_file,
   start_select_change_outfile,
   start_select_change_header_comment
} start_select_kind;
#define NUM_START_SELECT_KINDS (((int) start_select_change_header_comment)+1)

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

/* Values returned by the various popup routines: */
#define POPUP_DECLINE 0
#define POPUP_ACCEPT  1
#define POPUP_ACCEPT_WITH_STRING 2

extern Cstring concept_key_table[];                                 /* in SDSI */
extern Cstring getout_strings[];                                    /* in SDUTIL */
extern Cstring filename_strings[];                                  /* in SDUTIL */

/* In SDGETOUT */

extern void create_resolve_menu_title(
   command_kind goal,
   int cur,
   int max,
   resolver_display_state state,
   char *title);

/* In SDINIT */

extern void build_database(call_list_mode_t call_list_mode);
extern void initialize_menus(call_list_mode_t call_list_mode);

/* In SDSI */

extern void *get_mem(uint32 siz);
extern void *get_mem_gracefully(uint32 siz);
extern void *get_more_mem(void *oldp, uint32 siz);
extern void *get_more_mem_gracefully(void *oldp, uint32 siz);
extern void free_mem(void *ptr);
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

NORETURN1 extern void exit_program(int code) NORETURN2;
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
extern void uims_add_new_line(char the_line[], uint32 drawing_picture);
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
NORETURN1 extern void uims_final_exit(int code) NORETURN2;

extern int use_escapes_for_drawing_people;                          /* in SDUTIL */
extern char *pn1;                                                   /* in SDUTIL */
extern char *pn2;                                                   /* in SDUTIL */
extern char *direc;                                                 /* in SDUTIL */

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
#ifdef OLD_ELIDE_BLANKS_JUNK
extern long_boolean elide_blanks;                                   /* in SDMAIN */
#endif
extern long_boolean retain_after_error;                             /* in SDMAIN */
extern SDLIB_API long_boolean diagnostic_mode;                      /* in SDTOP */
extern SDLIB_API int singing_call_mode;                             /* in SDTOP */
extern SDLIB_API call_conc_option_state current_options;            /* in SDTOP */
extern SDLIB_API long_boolean allowing_all_concepts;                          /* in SDMAIN */
extern SDLIB_API long_boolean using_active_phantoms;                          /* in SDMAIN */

extern int session_index;                                           /* in SDSI */
extern int num_command_commands;                                    /* in SDSI */
extern Cstring *command_commands;                                   /* in SDSI */
extern command_kind *command_command_values;                        /* in SDSI */
extern int number_of_resolve_commands;                              /* in SDSI */
extern Cstring* resolve_command_strings;                            /* in SDSI */
extern resolve_command_kind *resolve_command_values;                /* in SDSI */
extern int no_graphics;                                             /* in SDSI */
extern int no_intensify;                                            /* in SDSI */
extern int reverse_video;                                           /* in SDSI */
extern int pastel_color;                                            /* in SDSI */
extern int no_color;                                                /* in SDSI */
extern int no_sound;                                                /* in SDSI */
extern int random_number;                                           /* in SDSI */
extern int hashed_randoms;                                          /* in SDSI */
extern char *database_filename;                                     /* in SDSI */
extern char *new_outfile_string;                                    /* in SDSI */
extern char *call_list_string;                                      /* in SDSI */

extern SDLIB_API selector_item selector_list[];                               /* in SDTABLES */

extern Const call_conc_option_state null_options;                   /* in SDMAIN */
extern SDLIB_API concept_descriptor concept_descriptor_table[];               /* in SDCTABLE */
extern SDLIB_API callspec_block **main_call_lists[NUM_CALL_LIST_KINDS];       /* in SDCTABLE */
extern SDLIB_API int number_of_calls[NUM_CALL_LIST_KINDS];                    /* in SDCTABLE */
extern SDLIB_API dance_level calling_level;                                   /* in SDCTABLE */

extern long_boolean enable_file_writing;                            /* in SDUTIL */
extern long_boolean singlespace_mode;                               /* in SDUTIL */
extern long_boolean nowarn_mode;                                    /* in SDUTIL */
extern long_boolean accept_single_click;                            /* in SDUTIL */
extern Cstring menu_names[];                                        /* in SDUTIL */
extern command_list_menu_item command_menu[];                       /* in SDUTIL */
extern resolve_list_menu_item resolve_menu[];                       /* in SDUTIL */
extern Cstring startup_commands[];                                  /* in SDUTIL */
extern SDLIB_API Cstring cardinals[];                                         /* in SDUTIL */
extern SDLIB_API Cstring ordinals[];                                          /* in SDUTIL */
extern SDLIB_API Cstring direction_names[];                                   /* in SDUTIL */
extern int last_direction_kind;                                     /* in SDUTIL */

extern int *concept_list;        /* indices of all concepts */
extern int concept_list_length;
extern int *level_concept_list; /* indices of concepts valid at current level */
extern int level_concept_list_length;

extern modifier_block *fcn_key_table_normal[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern modifier_block *fcn_key_table_start[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern modifier_block *fcn_key_table_resolve[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
extern match_result user_match;

extern SDLIB_API concept_descriptor centers_concept;                          /* in SDCTABLE */
extern concept_descriptor special_magic;                            /* in SDCTABLE */
extern concept_descriptor special_interlocked;                      /* in SDCTABLE */
extern SDLIB_API concept_descriptor mark_end_of_list;                         /* in SDCTABLE */
extern concept_descriptor marker_decline;                           /* in SDCTABLE */
extern SDLIB_API concept_descriptor marker_concept_mod;                       /* in SDCTABLE */
extern SDLIB_API concept_descriptor marker_concept_comment;                   /* in SDCTABLE */
extern concept_descriptor marker_concept_supercall;                 /* in SDCTABLE */

extern int abs_max_calls;                                           /* in SDMAIN */
extern int max_base_calls;                                          /* in SDMAIN */
extern SDLIB_API callspec_block **base_calls;                       /* in SDTOP */
extern uint32 number_of_taggers[NUM_TAGGER_CLASSES];                /* in SDMAIN */
extern SDLIB_API callspec_block **tagger_calls[NUM_TAGGER_CLASSES];           /* in SDMAIN */
extern uint32 number_of_circcers;                                   /* in SDMAIN */
extern SDLIB_API callspec_block **circcer_calls;                              /* in SDMAIN */
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


/* in SDSI */

extern long_boolean open_session(int argc, char **argv);
extern int process_session_info(Cstring *error_msg);
extern long_boolean open_call_list_file(char filename[]);
extern long_boolean open_accelerator_region(void);
extern long_boolean get_accelerator_line(char line[]);
extern void close_init_file(void);
NORETURN1 extern void final_exit(int code) NORETURN2;


/* in SDMAIN */

extern long_boolean deposit_call(
   callspec_block *call,
   Const call_conc_option_state *options);
extern long_boolean deposit_concept(concept_descriptor *conc);
extern int MAYBECALLBACK sdmain(int argc, char *argv[]);
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

#ifdef __cplusplus
}
#endif
