/*
 * sdmatch.h - header file for command matcher
 *
 * Copyright (C) 1993 Alan Snyder
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The author makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" WITHOUT ANY
 * WARRANTY, without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  
 *
 */


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

#define INPUT_TEXTLINE_SIZE 300

typedef struct glozk {
   uims_reply kind;
   int index;
   call_conc_option_state call_conc_options;  /* Has numbers, selectors, etc. */
   callspec_block *call_ptr;
   concept_descriptor *concept_ptr;
   struct glozk *next_conc_or_subcall;  /* next concept, or, if this is end mark, points to substitution list */
   struct glozk *secondary_subcall;     /* points to substitution list for secondary subcall */
   struct glozk *gc_ptr;                /* used for reclaiming dead blocks */
} modifier_block;

typedef struct filch {
   long_boolean valid;       /* set to TRUE if a match was found */
   long_boolean exact;       /* set to TRUE if an exact match was found */
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
   int match_count;            /* the number of matches so far */
   int exact_count;            /* the number of exact matches so far */
   int exact_match;            /* true if an exact match has been found */
   long_boolean showing;       /* we are only showing the matching patterns */
   long_boolean verify;        /* true => verify calls before showing */
   int space_ok;               /* space is a legitimate next input character */
   int yielding_matches;       /* how many matches are yielding */
   int lowest_yield_depth;
   match_result result;        /* value of the first or exact matching pattern */
} match_state;

enum {
    match_startup_commands = -1,
    match_resolve_commands = -2,
    match_resolve_extra_commands = -3,
    match_selectors = -4,
    match_directions = -5,
    match_taggers = -9,      /* This is the lowest of 4 numbers. */
    match_circcer = -10
};


/* These are provided by sdmatch.c */
extern match_state static_ss;

/* This is provided by sdui-XXX.c */
extern long_boolean verify_has_stopped;

/* The following items are PROVIDED by the main program (that is, the user
   interface part of it) and IMPORTED by sdmatch.c */

extern concept_descriptor *twice_concept_ptr;
extern concept_descriptor *two_calls_concept_ptr;
extern int num_command_commands;
extern Cstring command_commands[];
extern int number_of_resolve_commands;
extern Cstring resolve_command_strings[];
extern int num_extra_resolve_commands;
extern Cstring extra_resolve_commands[];


extern void show_match(void);

extern void matcher_initialize(void);

extern void matcher_setup_call_menu(call_list_kind cl);
    
extern int match_user_input(
   int which_commands,
   long_boolean show,
   long_boolean show_verify);
