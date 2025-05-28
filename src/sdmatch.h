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

typedef struct {
    int valid;            /* set to TRUE if a match was found */
    int exact;            /* set to TRUE if an exact match was found */
    uims_reply kind;
    int index;
    selector_kind who;    /* matches <anyone> */
    direction_kind where; /* matches <direction> */
    int number_fields;    /* matches all the number fields */
    int howmanynumbers;   /* tells how many there are */
    int space_ok;         /* space is a legitimate next input character */
} match_result;

typedef void (*show_function)(char *user_input, char *extension, Const match_result *mr);

enum {
    match_startup_commands = -1,
    match_resolve_commands = -2,
    match_selectors = -3,
    match_directions = -4
};

#define NUM_SPECIAL_COMMANDS 2
#define SPECIAL_COMMAND_ALLOW_MODS 0
#define SPECIAL_COMMAND_ALLOW_ALL_CONCEPTS 1

extern void
matcher_initialize(long_boolean show_commands_last);

extern void
matcher_add_call_to_menu(call_list_kind cl, int call_menu_index, char name[]);
    
extern int
match_user_input(char *user_input, int which_commands, match_result *mr,
                 char *extension, show_function sf, int show_verify);
