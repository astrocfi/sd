/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

#include <stdio.h>
#include <setjmp.h>

#include "basetype.h"
#include "sdui.h"


#ifdef __cplusplus
extern "C" {
#endif

#include "sdcom.h"

/* Actually, we don't make a resolve bigger than 3.  This is how much space
   we allocate for things.  Just being careful. */
#define MAX_RESOLVE_SIZE 5

/* Probability (out of 8) that a concept will be placed on a randomly generated call. */
#define CONCEPT_PROBABILITY 2
/* We use lots more concepts for "standardize", since it is much less likely (though
   by no means impossible) that a plain call will do the job. */
#define STANDARDIZE_CONCEPT_PROBABILITY 6


typedef struct flonk {
   char txt[80];
   struct flonk *nxt;
} outfile_block;


typedef struct {           /* This is done to preserve the encapsulation of type "jmp_buf".                  */
   jmp_buf the_buf;        /*   We are going to use pointers to these things.  If we simply used             */
} real_jmp_buf;            /*   pointers to jmp_buf, the semantics would not be transparent if jmp_buf       */
                           /*   were defined as an array.  The semantics of pointers to arrays are different */
                           /*   from those of pointers to other types, and are not transparent.              */
                           /*   In particular, we would need to look inside the include file to see what     */
                           /*   underlying type jmp_buf is an array of, which would violate the principle    */
                           /*   of type encapsulation.                                                       */


/* VARIABLES */

extern real_jmp_buf longjmp_buffer;                                 /* in SDUTIL */
extern real_jmp_buf *longjmp_ptr;                                   /* in SDUTIL */
extern int history_allocation;                                      /* in SDUTIL */
extern dance_level level_threshholds[];                             /* in SDUTIL */
extern dance_level higher_acceptable_level[];                       /* in SDUTIL */
extern writechar_block_type writechar_block;                        /* in SDUTIL */


extern FILE *call_list_file;

extern command_kind search_goal;                                    /* in SDPICK */






/* In SDMAIN */

extern char *sd_version_string(void);
extern parse_block *mark_parse_blocks(void);
extern void release_parse_blocks_to_mark(parse_block *mark_point);
extern void initialize_parse(void);
extern parse_block *copy_parse_tree(parse_block *original_tree);
extern void save_parse_state(void);
extern long_boolean restore_parse_state(void);
extern long_boolean query_for_call(void);
extern void write_header_stuff(long_boolean with_ui_version, uint32 act_phan_flags);
extern long_boolean sequence_is_resolved(void);

/* In SDPICK */

extern void reset_internal_iterators(void);
extern selector_kind do_selector_iteration(long_boolean allow_iteration);
extern direction_kind do_direction_iteration(void);
extern void do_number_iteration(int howmanynumbers,
                                uint32 odd_number_only,
                                long_boolean allow_iteration,
                                uint32 *number_list);
extern void do_circcer_iteration(uint32 *circcp);
extern long_boolean do_tagger_iteration(uint32 tagclass,
                                        uint32 *tagg,
                                        uint32 numtaggers,
                                        callspec_block **tagtable);
extern callspec_block *do_pick(void);
extern concept_descriptor *pick_concept(long_boolean already_have_concept_in_place);
extern resolve_goodness_test get_resolve_goodness_info(void);
extern long_boolean pick_allow_multiple_items(void);
extern void start_pick(void);
extern void end_pick(void);
extern long_boolean forbid_call_with_mandatory_subcall(void);
extern long_boolean allow_random_subcall_pick(void);

/* In SDSI */

extern void general_initialize(void);
extern int generate_random_number(int modulus);
extern void hash_nonrandom_number(int number);
extern void get_date(char dest[]);
extern void unparse_number(int arg, char dest[]);
extern void open_file(void);
extern void write_file(char line[]);
extern void close_file(void);
extern void print_line(Cstring s);
extern void print_id_error(int n);
NORETURN1 extern void init_error(char s[]) NORETURN2;
extern void add_resolve_indices(char junk[], int cur, int max);
extern void fill_in_neglect_percentage(char junk[], int n);
extern int parse_number(char junk[]);

/* In SDUTIL */

extern void clear_screen(void);
extern void writechar(char src);
extern void newline(void);
extern void writestuff(Const char *s);
extern void doublespace_file(void);
extern void string_copy(char **dest, Cstring src);
extern void display_initial_history(int upper_limit, int num_pics);
extern void write_history_line(int history_index, Const char *header, long_boolean picture, file_write_flag write_to_file);
extern call_list_kind find_proper_call_list(setup *s);
NORETURN1 extern void do_throw(error_flag_type f) NORETURN2;
extern long_boolean do_subcall_query(
   int snumber,
   parse_block *parseptr,
   parse_block **newsearch,
   long_boolean this_is_tagger,
   long_boolean this_is_tagger_circcer,
   callspec_block *orig_call);

/* In SDGETOUT */

extern void write_resolve_text(long_boolean doing_file);
extern uims_reply full_resolve(void);
extern int concepts_in_place(void);
extern int reconcile_command_ok(void);
extern int resolve_command_ok(void);
extern int nice_setup_command_ok(void);
extern void initialize_getout_tables(void);

#ifdef __cplusplus
}
#endif
