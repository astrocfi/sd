/* SD -- square dance caller's helper.

    Copyright (C) 1990-1994  William B. Ackerman.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 1, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    This is for version 31. */

/* This defines the following functions:
   clear_screen
   writestuff
   newline
   doublespace_file
   exit_program
   fail
   fail2
   specialfail
   get_escape_string
   string_copy
   display_initial_history
   write_history_line
   warn
   find_proper_call_list
   assoc
   find_calldef
   clear_people
   rotperson
   rotcw
   rotccw
   clear_person
   copy_person
   copy_rot
   swap_people
   install_person
   install_rot
   process_final_concepts
   fix_n_results

and the following external variables:
   longjmp_buffer
   longjmp_ptr
   history
   history_allocation
   history_ptr
   written_history_items
   written_history_nopic
   last_magic_diamond
   error_message1
   error_message2
   collision_person1
   collision_person2
   enable_file_writing
   cardinals
   ordinals
   selector_names
   direction_names
   warning_strings
*/

/* For "sprintf" */
#include <stdio.h>
#include <string.h>
#include "sd.h"



/*  This is the line length beyond which we will take pity on
   whatever device has to print the result, and break the text line.
   It is presumably smaller than our internal text capacity. */
# define MAX_PRINT_LENGTH 59

/* These variables are external. */

real_jmp_buf longjmp_buffer;
real_jmp_buf *longjmp_ptr;
configuration *history = (configuration *) 0; /* allocated in sdmain */
int history_allocation = 0; /* How many items are currently allocated in "history". */
int history_ptr;

/* This tells how much of the history text written to the UI is "safe".  If this
   is positive, the history items up to that number, inclusive, have valid
   "text_line" fields, and have had their text written to the UI.  Furthermore,
   the local variable "text_line_count" is >= any of those "text_line" fields,
   and each "text_line" field shows the number of lines of text that were
   written to the UI when that history item was written.  This variable is
   -1 when none of the history is safe or the state of the text in the UI
   is unknown. */
int written_history_items;
/* When written_history_items is positive, this tells how many of the initial lines
   did not have pictures forced on (other than having been drawn as a natural consequence
   of the "draw_pic" flag being on.)  If this number ever becomes greater than
   written_history_items, that's OK.  It just means that none of the lines had
   forced pictures. */
int written_history_nopic;

parse_block *last_magic_diamond;
char error_message1[MAX_ERR_LENGTH];
char error_message2[MAX_ERR_LENGTH];
uint32 collision_person1;
uint32 collision_person2;
long_boolean enable_file_writing;

char *cardinals[] = {"1", "2", "3", "4", "5", "6", "7", "8", (char *) 0};
char *ordinals[] = {"1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", (char *) 0};

/* BEWARE!!  These two lists are keyed to the definition of "selector_kind" in sd.h,
   and to the necessary stuff in SDUI. */
/* This one is external, so that the user interface can use it for popup text. */
char *selector_names[] = {
   "???",
   "boys",
   "girls",
   "heads",
   "sides",
   "head corners",
   "side corners",
   "head boys",
   "head girls",
   "side boys",
   "side girls",
   "centers",
   "ends",
   "leads",
   "trailers",
   "beaus",
   "belles",
   "center 2",
   "center 6",
   "outer 2",
   "outer 6",
   "center 4",
   "outer pairs",
   "headliners",
   "sideliners",
   "near line",
   "far line",
   "near column",
   "far column",
   "near box",
   "far box",
   "everyone",
   "no one",
   NULL};         /* The X11 interface uses this when making the popup text. */

Private char *selector_singular[] = {
   "???",
   "boy",
   "girl",
   "head",
   "side",
   "head corner",
   "side corner",
   "head boy",
   "head girl",
   "side boy",
   "side girl",
   "center",
   "end",
   "lead",
   "trailer",
   "beau",
   "belle",
   "center 2",
   "center 6",
   "outer 2",
   "outer 6",
   "center 4",
   "outer pair",
   "headliner",
   "sideliner",
   "near line",
   "far line",
   "near column",
   "far column",
   "near box",
   "far box",
   "everyone",
   "no one"};

/* BEWARE!!  This list is keyed to the definition of "direction_kind" in sd.h,
   and to the necessary stuff in SDUI. */
/* This one is external, so that the user interface can use it for popup text. */
char *direction_names[] = {
   "???",
   "left",
   "right",
   "in",
   "out",
   "zig-zag",
   "zag-zig",
   "zig-zig",
   "zag-zag",
   "(no direction)",
   NULL};         /* The X11 interface uses this when making the popup text. */


/* BEWARE!!  These strings are keyed to the definition of "warning_index" in sd.h . */
/* A "*" as the first character means that this warning precludes acceptance while searching. */
/* A "+" as the first character means that this warning is cleared if a concentric call was done
   and the "suppress_elongation_warnings" flag was on. */
/* A "=" as the first character means that this warning is cleared if it arises during some kind of
   "do your part" call. */
char *warning_strings[] = {
   /*  warn__none                */   " Unknown warning????",
   /*  warn__do_your_part        */   "*Do your part.",
   /*  warn__tbonephantom        */   " This is a T-bone phantom setup call.  Everyone will do their own part.",
   /*  warn__awkward_centers     */   "*Awkward for centers.",
   /*  warn__bad_concept_level   */   "*This concept is not allowed at this level.",
   /*  warn__not_funny           */   "*That wasn't funny.",
   /*  warn__hard_funny          */   "*Very difficult funny concept.",
   /*  warn__unusual             */   "*This is an unusual setup for this call.",
   /*  warn__rear_back           */   "*Rear back from the handhold.",
   /*  warn__awful_rear_back     */   "*Rear back from the handhold -- this is very unusual.",
   /*  warn__excess_split        */   "*Split concept seems to be superfluous here.",
   /*  warn__lineconc_perp       */   "+Ends should opt for setup perpendicular to their original line.",
   /*  warn__dmdconc_perp        */   "+Ends should opt for setup perpendicular to their original diamond points.",
   /*  warn__lineconc_par        */   "+Ends should opt for setup parallel to their original line -- concentric rule does not apply.",
   /*  warn__dmdconc_par         */   "+Ends should opt for setup parallel to their original diamond points -- concentric rule does not apply.",
   /*  warn__xclineconc_perp     */   "+New ends should opt for setup perpendicular to their original (center) line.",
   /*  warn__xcdmdconc_perp      */   "+New ends should opt for setup perpendicular to their original (center) diamond points.",
   /*  warn__ctrstand_endscpls   */   " Centers work in tandem, ends as couples.",
   /*  warn__ctrscpls_endstand   */   " Centers work as couples, ends in tandem.",
   /*  warn__each2x2             */   "=Each 2x2.",
   /*  warn__each1x4             */   "=Each 1x4.",
   /*  warn__each1x2             */   "=Each 1x2.",
   /*  warn__take_right_hands    */   " Take right hands.",
   /*  warn__ctrs_are_dmd        */   " The centers are the diamond.",
   /*  warn__full_pgram          */   " Completely offset parallelogram.",
   /*  warn__offset_gone         */   " The offset goes away.",
   /*  warn__overlap_gone        */   " The overlap goes away.",
   /*  warn__to_o_spots          */   " Go back to 'O' spots.",
   /*  warn__to_x_spots          */   " Go back to butterfly spots.",
   /*  warn__some_rear_back      */   " Some people rear back.",
   /*  warn__not_tbone_person    */   " Work with the person to whom you are not T-boned.",
   /*  warn__check_c1_phan       */   " Check a 'C1 phantom' setup.",
   /*  warn__check_dmd_qtag      */   " Fudge to a diamond/quarter-tag setup.",
   /*  warn__check_2x4           */   " Check a 2x4 setup.",
   /*  warn__check_pgram         */   " Opt for a parallelogram.",
   /*  warn__dyp_resolve_ok      */   " Do your part.",
   /*  warn__ctrs_stay_in_ctr    */   " Centers stay in the center.",
   /*  warn__check_c1_stars      */   " Check a generalized 'star' setup.",
   /*  warn__bigblock_feet       */   " Bigblock/stagger shapechanger -- go to footprints.",
   /*  warn__some_touch          */   " Some people step to a wave.",
   /*  warn__split_to_2x4s       */   " Do the call in each 2x4.",
   /*  warn__split_to_2x3s       */   " Do the call in each 2x3.",
   /*  warn__split_to_1x8s       */   " Do the call in each 1x8.",
   /*  warn__split_to_1x6s       */   " Do the call in each 1x6.",
   /*  warn__take_left_hands     */   " Take left hands, since this call is being done mirror.",
   /*  warn__evil_interlocked    */   " Interlocked phantom shape-changers are very evil.",
   /*  warn__split_phan_in_pgram */   " The split phantom setups are directly adjacent to the real people.",
   /*  warn__bad_interlace_match */   "*The interlaced calls have mismatched lengths.",
   /*  warn__not_on_block_spots  */   " Generalized bigblock/stagger -- people are not on block spots.",
   /*  warn__did_not_interact    */   "*The setups did not interact with each other.",
   /*  warn__opt_for_normal_cast */   "*If in doubt, assume a normal cast."};

/* These variables are used by the text-packing stuff. */

Private char *destcurr;
Private char lastchar;
Private char *lastblank;
Private char current_line[MAX_TEXT_LINE_LENGTH];
Private int text_line_count = 0;



Private void open_text_line(void)
{
   destcurr = current_line;
   lastchar = ' ';
   lastblank = (char *) 0;
}


extern void clear_screen(void)
{
   written_history_items = -1;
   text_line_count = 0;
   uims_reduce_line_count(0);
   open_text_line();
}


Private void writechar(char src)
{
   *destcurr = (lastchar = src);
   if (src == ' ' && destcurr != current_line) lastblank = destcurr;

   if (destcurr < &current_line[MAX_PRINT_LENGTH])
      destcurr++;
   else {
      /* Line overflow.  Try to write everything up to the last
         blank, then fill next line with everything since that blank. */

      char save_buffer[MAX_TEXT_LINE_LENGTH];
      char *q = save_buffer;

      if (lastblank) {
         char *p = lastblank+1;
         while (p <= destcurr) *q++ = *p++;
         destcurr = lastblank;
      }
      else {
         /* Must break a word. */
         *q++ = *destcurr;
      }

      *q = '\0';

      newline();            /* End the current buffer and write it out. */
      writestuff("   ");    /* Make a new line, copying saved stuff into it. */
      writestuff(save_buffer);
   }
}


/* Getting blanks into all the right places in the presence of substitions,
   insertions, and elisions is way too complicated to do in the database or
   to test.  For example, consider calls like "@4keep @5busy@1",
   "fascinat@pe@q@ning@o@t", or "spin the pulley@7 but @8".  So we try to
   help by never putting two blanks together, always putting blanks adjacent
   to the outside of brackets, and never putting blanks adjacent to the
   inside of brackets.  This procedure is part of that mechanism. */
Private void write_blank_if_needed(void)
{
   if (lastchar != ' ' && lastchar != '[' && lastchar != '(' && lastchar != '-') writestuff(" ");
}



extern void newline(void)
{
   /* Erase any trailing blanks.  Failure to do so can lead to some
      incredibly obscure bugs when some editors on PC's try to "fix"
      the file, ultimately leading to apparent loss of entire sequences.
      The problem was that the editor took out the trailing blanks
      and moved the rest of the text downward, but didn't reduce the
      file's byte count.  Control Z's were thereby introduced at the
      end of the file.  Subsequent runs of Sd appended new sequences
      to the file, leaving the ^Z's intact.  (Making Sd look for ^Z's
      at the end of a file it is about to append to, and remove same,
      is very hard.)  It turns out that some printing software stops,
      as though it reached the end of the file, when it encounters a
      ^Z, so the appended sequences were effectively lost. */
   while (destcurr != current_line && destcurr[-1] == ' ') destcurr--;

   *destcurr = '\0';

   if (enable_file_writing)
      write_file(current_line);

   text_line_count++;
   uims_add_new_line(current_line);
   open_text_line();
}



extern void writestuff(Const char s[])
{
   Const char *f = s;
   while (*f) writechar(*f++);
}



Private void write_nice_number(char indicator, uint32 num)
{
   char nn;

   switch (indicator) {
      case '9': case 'a': case 'b': case 'B':
         nn = '0' + num;
         if (indicator == '9')
            writechar(nn);
         else if (indicator == 'B') {
            if (num == 1)
               writestuff("quarter");
            else if (num == 2)
               writestuff("half");
            else if (num == 3)
               writestuff("three quarter");
            else if (num == 4)
               writestuff("four quarter");
            else {
               writechar(nn);
               writestuff("/4");
            }
         }
         else if (num == 2)
            writestuff("1/2");
         else if (num == 4 && indicator == 'a')
            writestuff("full");
         else {
            writechar(nn);
            writestuff("/4");
         }
         break;
      case 'u':     /* Need to plug in an ordinal number. */
         writestuff(ordinals[num-1]);
         break;
   }
}


Private void writestuff_with_decorations(parse_block *cptr, Const char *s, long_boolean singular)
{
   int index = cptr->number;
   Const char *f;

   f = s;     /* Argument "s", if non-null, overrides the concept name in the table. */
   if (!f) f = cptr->concept->name;

   while (*f) {
      if (f[0] == '@') {
         switch (f[1]) {
            case 'a': case 'b': case 'B': case 'u': case '9':
               write_nice_number(f[1], index & 0xF);
               f += 2;
               index >>= 4;
               continue;
            case '6': case 'k':
               writestuff(singular ? selector_singular[cptr->selector] : selector_names[cptr->selector]);
               f += 2;
               continue;
         }
      }
      else if (f[0] == '<' && f[1] == 'c' && f[2] == 'o' && f[3] == 'n' && f[4] == 'c' && f[5] == 'e' && f[6] == 'p' && f[7] == 't' && f[8] == '>') {
         f += 9;
         continue;
      }

      writechar(*f++);
   }
}



extern void doublespace_file(void)
{
   write_file("");
}



extern void exit_program(int code)
{
   uims_terminate();
   final_exit(code);
}


extern void nonreturning fail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   longjmp(longjmp_ptr->the_buf, 1);
}


extern void nonreturning fail2(Const char s1[], Const char s2[])
{
   (void) strncpy(error_message1, s1, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   (void) strncpy(error_message2, s2, MAX_ERR_LENGTH);
   error_message2[MAX_ERR_LENGTH-1] = '\0';
   longjmp(longjmp_ptr->the_buf, 2);
}


extern void nonreturning specialfail(Const char s[])
{
   (void) strncpy(error_message1, s, MAX_ERR_LENGTH);
   error_message1[MAX_ERR_LENGTH-1] = '\0';
   error_message2[0] = '\0';
   longjmp(longjmp_ptr->the_buf, 4);
}


/* This examines the indicator character after an "@" escape.  If the escape
   is for something that is supposed to appear in the menu as a "<...>"
   wildcard, it returns that string.  If it is an escape that starts a
   substring that would normally be elided, as in "@7 ... @8", it returns
   a non-null pointer to a null character.  If it is an escape that is normally
   simply dropped, it returns a null pointer. */

extern Const char *get_escape_string(char c)
{
   switch (c) {
      case '0': case 'm':
         return "<ANYTHING>";
      case '6': case 'k':
         return "<ANYONE>";
      case 'h':
         return "<DIRECTION>";
      case '9':
         return "<N>";
      case 'a': case 'b': case 'B':
         return "<N/4>";
      case 'u':
         return "<Nth>";
      case '7': case 'n': case 'j':
         return "";
      default:
         return (char *) 0;
   }
}


extern void string_copy(char **dest, char src[])
{
   char *f = src;
   char *t = *dest;

   while (*t++ = *f++);
   *dest = t-1;
}

/* There are 2 (and only 2) bits that are meaningful in the argument to "print_recurse":
         PRINT_RECURSE_STAR
         DFM1_MUST_BE_TAG_CALL
   The first of these means to print an asterisk for a call that is missing in the
   current type-in state.  The other two mean that we are printing a subcall, and
   that the indicated invocation flag is on.  These control the elision of stuff
   like "1/2", so that we will say "[vertical tag] back to a wave" instead of
   "[vertical 1/2 tag] back to a wave". */

/* We define this one to be some bit that won't conflict with the other two.
   The simplest way to do that is to use some other "DFM1" bit.  We pick on poor
   defenseless "DFM1_CONC_DEMAND_LINES" because it is so far away from home in this file. */
#define PRINT_RECURSE_STAR DFM1_CONC_DEMAND_LINES


Private void print_recurse(parse_block *thing, int print_recurse_arg)
{
   parse_block *static_cptr;
   parse_block *next_cptr;
   long_boolean use_left_name = FALSE;
   long_boolean use_cross_name = FALSE;
   long_boolean comma_after_next_concept = FALSE;

   static_cptr = thing;

   while (static_cptr) {
      int i;
      concept_kind k;
      concept_descriptor *item;

      item = static_cptr->concept;
      k = item->kind;

      if (k == concept_comment) {
         comment_block *fubb;

         fubb = (comment_block *) static_cptr->call;
         writestuff("{ ");
         writestuff(fubb->txt);
         writestuff(" } ");
         static_cptr = static_cptr->next;
      }
      else if (k > marker_end_of_list) {
         /* This is a concept. */

         long_boolean request_final_space = FALSE;
         long_boolean request_comma_after_next_concept = FALSE;
         concept_kind kk = k;
         parse_block *cc = static_cptr;

         /* We turn off the funny name elision if concepts are present,
            except for "cross" and "left", since we want "[cross flip] your neighbor"
            and "[LEFT tag the star] your neighbor" to work. */

         for (;;) {
            if (kk <= marker_end_of_list) {
               break;
            }
            else if (kk != concept_left && kk != concept_cross) {
               print_recurse_arg &= ~DFM1_MUST_BE_TAG_CALL;
               break;
            }
            cc = cc->next;
            if (!cc) break;
            kk = cc->concept->kind;
         }

         next_cptr = static_cptr->next;    /* Now it points to the thing after this concept. */

         if (concept_table[k].concept_prop & CONCPROP__SECOND_CALL) {
            parse_block *subsidiary_ptr = static_cptr->subsidiary_root;

            if (k == concept_centers_and_ends) {
               if ((i = item->value.arg1) == 2)
                  writestuff("CENTER 6 ");
               else if (i)
                  writestuff("CENTER 2 ");
               else
                  writestuff("CENTERS ");
            }
            else if (k == concept_some_vs_others) {
               if ((i = item->value.arg1) == 1)
                  writestuff_with_decorations(static_cptr, "@6 DO YOUR PART, ", FALSE);
               else if (i == 3)
                  writestuff_with_decorations(static_cptr, "OWN THE @6, ", FALSE);
               else if (i == 5)
                  writestuff_with_decorations(static_cptr, "@6, ", FALSE);
               else
                  writestuff_with_decorations(static_cptr, "@6 DISCONNECTED ", FALSE);
            }
            else if (k == concept_sequential) {
               writestuff("(");
            }
            else if (k == concept_replace_nth_part) {
               writestuff("DELAY: ");
               if (!static_cptr->next || !subsidiary_ptr) {
                  if (static_cptr->concept->value.arg1)
                     writestuff("(interrupting after the ");
                  else
                     writestuff("(replacing the ");
                  writestuff(ordinals[static_cptr->number-1]);
                  writestuff(" part) ");
               }
            }
            else {
               writestuff(item->name);
               writestuff(" ");
            }

            print_recurse(static_cptr->next, 0);

            if (!subsidiary_ptr) break;         /* Can happen if echoing incomplete input. */

            request_final_space = TRUE;

            if (k == concept_centers_and_ends)
               writestuff(" WHILE THE ENDS");
            else if (k == concept_some_vs_others && item->value.arg1 != 3)
               writestuff(" WHILE THE OTHERS");
            else if (k == concept_on_your_own)
               writestuff(" AND");
            else if (k == concept_interlace)
               writestuff(" WITH");
            else if (k == concept_replace_nth_part) {
               writestuff(" BUT ");
               writestuff_with_decorations(static_cptr, (Const char *) 0, FALSE);
               writestuff(" WITH A [");
               request_final_space = FALSE;
            }
            else if (k == concept_sequential)
               writestuff(" ;");
            else if (k == concept_special_sequential)
               writestuff(",");
            else
               writestuff(" BY");

            next_cptr = subsidiary_ptr;
         }
         else if (k == concept_selbasedtrngl) {
            writestuff_with_decorations(static_cptr, (Const char *) 0, TRUE);
            request_final_space = TRUE;
         }
         else if (k == concept_so_and_so_stable || k == concept_so_and_so_frac_stable ||
                  k == concept_so_and_so_begin || k == concept_some_are_frac_tandem ||
                  k == concept_some_are_tandem) {
            writestuff_with_decorations(static_cptr, (Const char *) 0, FALSE);
            writestuff(",");
            request_final_space = TRUE;
         }
         else if (static_cptr && (k == concept_left || k == concept_cross)) {

            /* These concepts want to take special action if there are no following concepts and
               certain escape characters are found in the name of the following call. */

            final_set finaljunk;
            callspec_block *target_call;
            parse_block *tptr;
            
            /* Skip all final concepts, then demand that what remains is a marker (as opposed to a serious
                concept), and that a real call has been entered, and that its name starts with "@g". */
            tptr = process_final_concepts(next_cptr, FALSE, &finaljunk);

            if (tptr) {
               target_call = tptr->call;
   
               if ((tptr->concept->kind <= marker_end_of_list) && target_call && (target_call->name[0] == '@')) {
                  if (k == concept_left) {
                     /* See if this is a call whose name naturally changes when the "left" concept is used. */
                     if (target_call->name[1] == 'g') {
                        use_left_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
                  else {
                     /* See if this is a call that wants the "cross" modifier to be moved inside its name. */
                     if (target_call->name[1] == 'i') {
                        use_cross_name = TRUE;
                     }
                     else {
                        writestuff(item->name);
                        request_final_space = TRUE;
                     }
                  }
               }
               else {
                  writestuff(item->name);
                  request_final_space = TRUE;
               }
            }
            else {
               writestuff(item->name);
               request_final_space = TRUE;
            }
         }
         else if (k == concept_nth_part) {
            if (static_cptr->concept->value.arg1 == 1) {
               comma_after_next_concept = TRUE;           /* "SKIP THE <Nth> PART" */
               request_final_space = TRUE;
            }
            else
               request_comma_after_next_concept = TRUE;   /* "DO THE <Nth> PART <concept>" */

            writestuff_with_decorations(static_cptr, (Const char *) 0, FALSE);
         }
         else if ((k == concept_meta) && static_cptr->concept->value.arg1 == 3) {
            writestuff("START");
            request_comma_after_next_concept = TRUE;
            request_final_space = TRUE;
         }
         else {
            writestuff_with_decorations(static_cptr, (Const char *) 0, FALSE);
            request_final_space = TRUE;
         }

         if (k == concept_fractional || k == concept_twice)
            comma_after_next_concept = TRUE;

         static_cptr = next_cptr;

         if (comma_after_next_concept)
            writestuff(", ");
         else if (request_final_space)
            writestuff(" ");

         comma_after_next_concept = request_comma_after_next_concept;

         if (k == concept_sequential) {
            print_recurse(static_cptr, PRINT_RECURSE_STAR);
            writestuff(")");
            return;
         }
         else if (k == concept_replace_nth_part) {
            print_recurse(static_cptr, PRINT_RECURSE_STAR);
            writestuff("]");
            return;
         }
      }
      else {
         /* This is a "marker", so it has a call, perhaps with a selector and/or number.
            The call may be null if we are printing a partially entered line.  Beware. */

         parse_block *subsidiary_ptr;
         parse_block *sub1_ptr;
         parse_block *sub2_ptr;
         parse_block *search;
         long_boolean pending_subst1, subst1_in_use, this_is_subst1;
         long_boolean pending_subst2, subst2_in_use, this_is_subst2;

         selector_kind i16junk = static_cptr->selector;
         int tagjunk = static_cptr->tagger;
         direction_kind idirjunk = static_cptr->direction;
         uint32 number_list = static_cptr->number;
         unsigned int next_recurse1_arg = 0;
         unsigned int next_recurse2_arg = 0;
         callspec_block *localcall = static_cptr->call;
         parse_block *save_cptr = static_cptr;

         subst1_in_use = FALSE;
         subst2_in_use = FALSE;

         if (k == concept_another_call_next_mod) {
            search = save_cptr->next;
            while (search) {
               this_is_subst1 = FALSE;
               this_is_subst2 = FALSE;
               subsidiary_ptr = search->subsidiary_root;
               if (subsidiary_ptr) {
                  switch ((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) {
                     case 1:
                     case 2:
                        /* Pick up the MUST_BE_TAG bit.  This might also
                           inadvertently pick up the PRINT_RECURSE_STAR bit, but it
                           doesn't matter, since we are going to set that bit anyway. */
                        next_recurse1_arg = search->number;
                        this_is_subst1 = TRUE;
                        break;
                     case 5:
                     case 6:
                        /* See above. */
                        next_recurse2_arg = search->number;
                        this_is_subst2 = TRUE;
                        break;
                  }

                  if (this_is_subst1) {
                     if (subst1_in_use) writestuff("ERROR!!!");
                     subst1_in_use = TRUE;
                     sub1_ptr = subsidiary_ptr;
                  }

                  if (this_is_subst2) {
                     if (subst2_in_use) writestuff("ERROR!!!");
                     subst2_in_use = TRUE;
                     sub2_ptr = subsidiary_ptr;
                  }
               }
               search = search->next;
            }
         }
   
         pending_subst1 = subst1_in_use;
         pending_subst2 = subst2_in_use;

         /* Now "subst_in_use" is on if there is a replacement call that goes in naturally.  During the
            scan of the name, we will try to fit that replacement into the name of the call as directed
            by atsign-escapes.  If we succeed at this, we will clear "pending_subst".
            In addition to all of this, there may be any number of forcible replacements. */

         if (localcall) {      /* Call = NIL means we are echoing input and user hasn't entered call yet. */
            char *np;
            char savec;

            if (enable_file_writing) localcall->age = global_age;
            np = localcall->name;

            /* Fix up elision of stuff.  If a call with "is_tag_call" on invokes a call
               with "must_be_tag_call", we do not use the "must_be_tag_call" flag to cause
               elision of "1/2" in things like "1/2 flip".  Instead, we simply pass on the
               state of the "must_be_tag_call" that we inherited.  This makes "revert the [1/2 tag]"
               or "[reflected [1/2 flip]] stimulate" not elide the "1/2", while
               "revert the [flip] your neighbor" does elide it. */

            if (localcall->callflags1 & CFLAG1_IS_TAG_CALL) {
               if (!(print_recurse_arg & DFM1_MUST_BE_TAG_CALL)) {
                  next_recurse1_arg &= ~DFM1_MUST_BE_TAG_CALL;
                  next_recurse2_arg &= ~DFM1_MUST_BE_TAG_CALL;
               }
            }

            /* Skip any "@g" or "@i" marker (we already acted on it.) */
            if ((*np == '@') && ((np[1] == 'g') || (np[1] == 'i'))) np += 2;

            while (*np) {
               if (*np == '@') {
                  savec = np[1];

                  switch (savec) {
                     case '6': case 'k':
                        write_blank_if_needed();
                        if (savec == '6')
                           writestuff(selector_names[i16junk]);
                        else
                           writestuff(selector_singular[i16junk]);
                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the digit */
                        break;
                     case 'h':                   /* Need to plug in a direction. */
                        write_blank_if_needed();
                        writestuff(direction_names[idirjunk]);
                        if (np[2] && np[2] != ' ' && np[2] != ']')
                           writestuff(" ");
                        np += 2;       /* skip the indicator */
                        break;
                     case '9': case 'a': case 'b': case 'B': case 'u':    /* Need to plug in a number. */
                        write_blank_if_needed();
                        write_nice_number(savec, number_list & 0xF);
                        number_list >>= 4;    /* Get ready for next number. */
                        np += 2;              /* skip the indicator */
                        break;
                     case 'e':
                        if (use_left_name) {
                           np += 2;
                           while (*np != '@') np++;
                           if (lastchar == ']') writestuff(" ");
                           writestuff("left");
                        }
                        np += 2;
                        break;
                     case 'j':
                        if (!use_cross_name) {
                           np += 2;
                           while (*np != '@') np++;
                        }
                        np += 2;
                        break;
                     case 'l': case '8': case 'o':    /* Just skip these -- they end stuff that we could have elided but didn't. */
                        np += 2;
                        break;
                     case 'c':
                        if (print_recurse_arg & DFM1_MUST_BE_TAG_CALL) {
                           np += 2;
                           while (*np != '@') np++;
                        }
                        else {
                           if ((lastchar == ' ' || lastchar == '[') && (np[2] == ' ')) np++;
                        }
                        np += 2;
                        break;
                     case 'n': case 'p': case 'r': case 'm': case 't':
                        if (subst2_in_use) {
                           if (savec == 'p' || savec == 'r') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
                        else {
                           if (savec == 'n') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
   
                        if (pending_subst2 && savec != 'p' && savec != 'n') {
                           write_blank_if_needed();
                           writestuff("[");
                           print_recurse(sub2_ptr, next_recurse2_arg | PRINT_RECURSE_STAR);
                           writestuff("]");
         
                           pending_subst2 = FALSE;
                        }
         
                        np += 2;        /* skip the digit */
                        break;
                     default:
                        if (subst1_in_use) {
                           if (savec == '2' || savec == '4') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
                        else {
                           if (savec == '7') {
                              np += 2;
                              while (*np != '@') np++;
                           }
                        }
         
                        if (pending_subst1 && savec != '4' && savec != '7') {
                           write_blank_if_needed();
                           writestuff("[");
                           print_recurse(sub1_ptr, next_recurse1_arg | PRINT_RECURSE_STAR);
                           writestuff("]");
         
                           pending_subst1 = FALSE;
                        }
         
                        np += 2;        /* skip the digit */
                        break;
                  }
               }
               else {
                  char c = *np++;

                  if (lastchar == ']' && c != ' ' && c != ']')
                     writestuff(" ");

                  if ((lastchar != ' ' && lastchar != '[') || c != ' ') writechar(c);
               }
            }

            if (lastchar == ']' && *np && *np != ' ' && *np != ']')
               writestuff(" ");
         }
         else if (print_recurse_arg & PRINT_RECURSE_STAR) {
            writestuff("*");
         }

         /* Now if "pending_subst" is still on, we have to do by hand what should have been
            a natural replacement.  In any case, we have to find forcible replacements and
            report them. */
   
         if (k == concept_another_call_next_mod) {
            int first_replace = 0;

            search = save_cptr->next;
            while (search) {
               subsidiary_ptr = search->subsidiary_root;
               if (subsidiary_ptr) {
                  switch ((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) {
                     case 1:
                     case 2:
                     case 3:
                        /* This is a natural replacement.  It may already have been taken care of. */
                        if (pending_subst1 || ((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) == 3) {
                           write_blank_if_needed();
                           if (((search->number & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT) == 3)
                              writestuff("but [");
                           else
                              writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     case 5:
                     case 6:
                        /* This is a secondary replacement.  It may already have been taken care of. */
                        if (pending_subst2) {
                           write_blank_if_needed();
                           writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     default:
                        /* This is a forced replacement.  Need to check for case of replacing
                           one star turn with another. */
                        localcall = search->call;
                        write_blank_if_needed();
                        if ((!(first_replace++)) && subsidiary_ptr &&
                              (localcall->callflags1 & CFLAG1_IS_STAR_CALL) &&
                                    ((subsidiary_ptr->concept->kind == marker_end_of_list) ||
                                    (subsidiary_ptr->concept->kind == concept_another_call_next_mod)) &&
                              (subsidiary_ptr->call) &&
                              (subsidiary_ptr->call->callflags1 & CFLAG1_IS_STAR_CALL)) {
                           writestuff("BUT [");
                        }
                        else {
                           if (first_replace == 1)
                              writestuff("BUT REPLACE ");
                           else
                              writestuff("AND REPLACE ");
                           writestuff(localcall->name);
                           writestuff(" WITH [");
                        }
   
                        print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                        writestuff("]");
                        break;
                  }
               }
               search = search->next;
            }
         }

         break;
      }
   }

   return;
}


/* These static variables are used by printperson. */

Private char peoplenames[] = "1234";
Private char directions[] = "B?B>B?B<B?B?B?B?B^B?BVB?B?B?B?B?G?G>G?G<G?G?G?G?G^G?GVG?G?G?G?G?";
Private char personbuffer[] = " ZZZ";

Private void printperson(unsigned int x)
{
   int i;

   if (x & BIT_PERSON) {
      i = 2 * (x & 017);
      if ((x & 0100) != 0) i = i+32;
      personbuffer[1] = peoplenames[(x >> 7) & 3];
      personbuffer[2] = directions[i];
      personbuffer[3] = directions[i+1];
      writestuff(personbuffer);
   }
   else
      writestuff("  . ");
}

/* These static variables are used by printsetup/print_4_person_setup/do_write. */

Private int offs, roti, modulus, personstart;
Private setup *printarg;

Private void do_write(char s[])
{
   char c;

   int ri = roti * 011;

   for (;;) {
      if (!(c=(*s++))) return;
      else if (c == '@') newline();
      else if (c == ' ') writestuff(" ");
      else if (c >= 'a' && c <= 'x')
         printperson(rotperson(printarg->people[personstart + ((c-'a'-offs) % modulus)].id1, ri));
      else writestuff("?");
   }
}


/* BEWARE!!  This list is keyed to the definition of "setup_kind" in database.h . */
static char *printing_tables[][2] = {
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* nothing */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s1x1 */
   {"a  b@",                                                                  "a@b@"},                                                                                                           /* s1x2 */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s1x3 */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s2x2 */
   {"a  b  d  c@",                                                            "a@b@d@c@"},                                                                                                       /* s1x4 */
   {"     b@a      c@     d@",                                                "   a@@d  b@@   c@"},                                                                                              /* sdmd */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_star */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_trngl */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_trngl4 */
   {"a        b@    fc@e        d@",                                          "ea@  f@  c@db@"},                                                                                                 /* s_bone6 */
   {"   b@a  c@f  d@   e@",                                                   "   fa@e      b@   dc@"},                                                                                          /* s_short6 */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_qtag */
   {"a                   b@    g h d c@f                   e",                "fa@  g@  h@  d@  c@eb"},                                                                                          /* s_bone */
   {"        a b@gh         dc@        f e",                                  "  g@  h@fa@eb@  d@  c"},                                                                                          /* s_rigger */
   {"    a b c@h              d@    g f e",                                   "  h@ga@fb@ec@  d"},                                                                                               /* s_spindle */
   {"   a  b@      d@g        c@      h@   f  e",                             "     g@f      a@   hd@e      b@     c"},                                                                          /* s_hrglass */
   {"a      d      b@     g      c@f      h      e",                          "f  a@@   g@@h  d@@   c@@e  b"},                                                                                  /* s_dhrglass */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_hyperglass */
   {"          c@          d@ab        fe@          h@          g",           "      a@      b@@ghdc@@      f@      e"},                                                                         /* s_crosswave */
   {"a b d c g h f e",                                                        "a@b@d@c@g@h@f@e"},                                                                                                /* s1x8 */
   {"a  b  c  d@@h  g  f  e",                                                 "h  a@@g  b@@f  c@@e  d"},                                                                                         /* s2x4 */
   {"a  b  c@f  e  d@",                                                       "f  a@e  b@d  c@"},                                                                                                /* s2x3 */
   {"a  b  c  f  e  d@",                                                      "a@b@c@f@e@d@"},                                                                                                   /* s1x6 */
   {"a  b  c  d@@k  l  f  e@@j  i  h  g",                                     "j  k  a@@i  l  b@@h  f  c@@g  e  d"},                                                                             /* s3x4 */
   {"a  b  c  d  e  f@@l  k  j  i  h  g",                                     "l  a@@k  b@@j  c@@i  d@@h  e@@g  f"},                                                                             /* s2x6 */
   {"a  b  c  d  e  f  g  h@@p  o  n  m  l  k  j  i",                         "p  a@@o  b@@n  c@@m  d@@l  e@@k  f@@j  g@@i  h"},                                                                 /* s2x8 */
   {"m  n  o  a@@k  p  d  b@@j  l  h  c@@i  g  f  e",                         (char *) 0},                                                                                                       /* s4x4 */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* sx1x6 */
   {"a b c d e j i h g f",                                                    "a@b@c@d@e@j@i@h@g@f"},                                                                                            /* s1x10 */
   {"a b c d e f l k j i h g",                                                "a@b@c@d@e@f@l@k@j@i@h@g"},                                                                                        /* s1x12 */
   {"abcdefgnmlkjih",                                                         "a@b@c@d@e@f@g@n@m@l@k@j@i@h"},                                                                                    /* s1x14 */
   {"abcdefghponmlkji",                                                       "a@b@c@d@e@f@g@h@p@o@n@m@l@k@j@i"},                                                                                /* s1x16 */
   {"   b        e@a  c  h  f@   d        g@@   o        l@n  p  k  i@   m        j",                                       (char *) 0},                                                         /* s_c1phan */
   {"            a  b@@      v  w  c  d@@t  u  x  f  e  g@@s  q  r  l  i  h@@      p  o  k  j@@            n  m",           (char *) 0},                                                         /* s_bigblob */
   {"    b           h@a    c   g    e@    d           f",                    "  a@@db@@  c@@  g@@fh@@  e"},                                                                                     /* s_ptpd */
   {"             d@@a b c g f e@@             h",                            "      a@@      b@@      c@h        d@      g@@      f@@      e"},                                                 /* s3x1dmd */
   {"   a      b      c@@j k l f e d@@   i      h      g",                    "      j@i        a@      k@@      l@h        b@      f@@      e@g        c@      d"},                             /* s3dmd */
   {"   a      b      c      d@@m n o p h g f e@@   l      k      j      i",  "      m@l        a@      n@@      o@k        b@      p@@      h@j        c@      g@@      f@i        d@      e"}, /* s4dmd */
   {"             d@a b c  g f e@             h",                             "   a@@   b@@   c@h  d@   g@@   f@@   e"},                                                                         /* s_wingedstar */
   {"             d       f@a b c  e k  i h g@             l       j",        "   a@@   b@@   c@l  d@   e@   k@j  f@   i@@   h@@   g"},                                                          /* s_wingedstar12 */
   {"             d       h       m@a b c  f g  o n  k j i@             e       p       l",  "   a@@   b@@   c@e  d@   f@   g@p  h@   o@   n@l  m@   k@@   j@@   i"},                            /* s_wingedstar16 */
   {"     c@   bd@a      e@   hf@     g",                                     (char *) 0},                                                                                                       /* s_galaxy */
   {"a  b  c  d  e  f@@l  k  j  i  h  g@@s  t  u  v  w  x@@r  q  p  o  n  m", "r  s  l  a@@q  t  k  b@@p  u  j  c@@o  v  i  d@@n  w  h  e@@m  x  g  f"},                                         /* s4x6 */
   {"      c@      d@abfe@      h@      g",                                   (char *) 0},                                                                                                       /* s_thar */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_x4dmd */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* s_8x8 */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* sfat2x8 */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* swide4x4 */
   {"   b      e@   c      f@a j d g@   l      i@   k      h",                "          a@k l        c b@          j@@          d@h i        f e@          g"},                                 /* sbigdmd */
   {(char *) 0,                                                               (char *) 0},                                                                                                       /* sminirigger */
   {(char *) 0,                                                               (char *) 0}};                                                                                                      /* s_normal_concentric */


Private void print_4_person_setup(int ps, small_setup *s, int elong)
{
   modulus = setup_attrs[s->skind].setup_limits+1;
   roti = (s->srotation & 3);
   personstart = ps;

   offs = (((roti >> 1) & 1) * (modulus / 2)) - modulus;

   switch (s->skind) {
      case s2x2:
         newline();
         if (elong < 0)
            do_write("ab@dc@");
         else if (elong & 1)
            do_write("ab@@@dc@");
         else
            do_write("a    b@d    c@");
         break;
      case s_star:
         newline();
         do_write("   b@a  c@   d@");
         break;
      case s1x4: case sdmd: case s2x4: case s2x3: case s1x6: case s_short6: case s_bone6: case s1x2:
         newline();
         do_write(printing_tables[s->skind][roti & 1]);
         break;
      default:
         writestuff(" ????");
   }
   newline();
}



Private void printsetup(setup *x)
{
   char *str;

   printarg = x;
   modulus = setup_attrs[x->kind].setup_limits+1;
   roti = x->rotation & 3;
   
   newline();

   personstart = 0;

   if (setup_attrs[x->kind].four_way_symmetry) {
      /* still to do???
         s_1x1
         s2x2
         s_star
         s_hyperglass */

      offs = (roti * (modulus / 4)) - modulus;
      str = printing_tables[x->kind][0];
   }
   else {
      offs = ((roti & 2) * (modulus / 4)) - modulus;
      str = printing_tables[x->kind][roti & 1];
   }

   if (str)
      do_write(str);
   else {
      switch (x->kind) {
         case s_qtag:
            if ((x->people[0].id1 & x->people[1].id1 & x->people[4].id1 & x->people[5].id1 & 1) &&
                  (x->people[2].id1 & x->people[3].id1 & x->people[6].id1 & x->people[7].id1 & 010)) {
               /* People are in diamond-like orientation. */
               if (x->rotation & 1)
                  do_write("      g@f        a@      h@@      d@e        b@      c");
               else {
                  do_write("   a     b@@g h d c@@   f     e");
               }
            }
            else {
               /* People are not.  Probably 1/4-tag-like orientation. */
               if (x->rotation & 1)
                  do_write("      g@f  h  a@e  d  b@      c");
               else {
                  do_write("      a  b@@g  h  d  c@@      f  e");
               }
            }
            break;
         case s_normal_concentric:
            writestuff(" centers:");
            newline();
            print_4_person_setup(0, &(x->inner), -1);
            writestuff(" ends:");
            newline();
            print_4_person_setup(12, &(x->outer), x->outer_elongation);
            break;
         default:
            writestuff("???? UNKNOWN SETUP ????");
      }
   }

   newline();
   newline();
}


/* Clear the screen and display the initial part of the history.
   This attempts to re-use material already displayed on the screen.
   The "num_pics" argument tells how many of the last history items
   are to have pictures forced, so we can tell exactly what items
   have pictures. */
extern void display_initial_history(int upper_limit, int num_pics)
{
   int j, startpoint, compilerbug;

   /* See if we can re-use some of the safely written history. */
   /* First, cut down overly optimistic estimates. */
   if (written_history_items > upper_limit) written_history_items = upper_limit;
   /* And normalize the "nopic" number. */
   if (written_history_nopic > written_history_items) written_history_nopic = written_history_items;

   /* Check whether pictures are faithful.  If any item in the written history has its
      "picture forced" property (as determined by written_history_nopic)
      different from what we want that property to be (as determined by upper_limit-num_pics),
      and that item didn't have the draw_pic flag on, that item needs to be rewritten.
      We cut written_history_items down to below that item if such is the case. */

   for (j=1; j<=written_history_items; j++) {
      compilerbug = ((int) ((unsigned int) (written_history_nopic-j)) ^
                 ((unsigned int) (upper_limit-num_pics-j)));
      if (compilerbug < 0 && ~history[j].draw_pic) {
         written_history_items = j-1;
         break;
      }
   }

   if (written_history_items > 0) {
      /* We win.  Back up the text line count to the right place, and rewrite the rest. */

      text_line_count = history[written_history_items].text_line;
      uims_reduce_line_count(text_line_count);
      open_text_line();
      startpoint = written_history_items+1;
   }
   else {
      /* We lose, there is nothing we can use. */
      clear_screen();
#ifndef THINK_C			/* Mac interface provides "About Sd" popup instead */
      write_header_stuff(TRUE, 0);
      newline();
#endif
      startpoint = 1;
   }

   for (j=startpoint; j<=upper_limit-num_pics; j++) write_history_line(j, (char *) 0, FALSE, file_write_no);

   /* Now write stuff with forced pictures. */

   for (j=upper_limit-num_pics+1; j<=upper_limit; j++) {
      if (j >= startpoint) write_history_line(j, (char *) 0, TRUE, file_write_no);
   }

   written_history_items = upper_limit;   /* This stuff is now safe. */
   written_history_nopic = written_history_items-num_pics;
}



extern void write_history_line(int history_index, Const char *header, long_boolean picture, file_write_flag write_to_file)
{
   int index, w, i;
   parse_block *thing;

   if (write_to_file == file_write_double)
      doublespace_file();

   index = history[history_index].centersp;

   /* Do not put index numbers into output file -- user may edit it later. */

   if (!enable_file_writing) {

#ifndef OLD_WAY
      i = history_index-whole_sequence_low_lim+1;
      if (header) writestuff(header);   /* Be sure we get it for partially entered concepts -- this line will go away. */
      else if (i > 0 && !diagnostic_mode) {   /* For now, don't do this if diagnostic, until we decide whether it is permanent. */
         char indexbuf[200];
         sprintf(indexbuf, "%2d:   ", i);
         writestuff(indexbuf);
      }
#else
      if (header) writestuff(header);
#endif

   }

   if (index != 0) {
      if (startinfolist[index].into_the_middle) goto morefinal;
      writestuff(startinfolist[index].name);
      goto final;
   }

   thing = history[history_index].command_root;
   
   /* Need to check for the special case of starting a sequence with heads or sides.
      If this is the first line of the history, and we started with heads of sides,
      change the name of this concept from "centers" to the appropriate thing. */

   if (history_index == 2 && thing->concept->kind == concept_centers_or_ends && thing->concept->value.arg1 == 0) {
      index = history[1].centersp;
      if (startinfolist[index].into_the_middle) {
         writestuff(startinfolist[index].name);
         writestuff(" ");
         thing = thing->next;
      }
   }
   
   print_recurse(thing, 0);
   
   final:

   newline();

   morefinal:

   /* Check for warnings to print. */
   /* Do not double space them, even if writing final output. */

   for (w=0 ; w<NUM_WARNINGS ; w++) {
      if (((1 << (w & 0x1f)) & history[history_index].warnings.bits[w>>5]) != 0) {
         writestuff("  Warning:  ");
         writestuff(&warning_strings[w][1]);
         newline();
      }
   }

   if (picture || history[history_index].draw_pic) {
      printsetup(&history[history_index].state);
   }

   /* Record that this history item has been written to the UI. */
   history[history_index].text_line = text_line_count;
}


extern void warn(warning_index w)
{
   if (w != warn__none) history[history_ptr+1].warnings.bits[w>>5] |= 1 << (w & 0x1F);
}


extern call_list_kind find_proper_call_list(setup *s)
{
   if (s->kind == s1x8) {
      if      ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_1x8;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_l1x8;
      else if ((s->people[0].id1 & 015) == 1 &&
               (s->people[1].id1 & 015) == 1 &&
               (s->people[2].id1 & 015) == 1 &&
               (s->people[3].id1 & 015) == 1 &&
               (s->people[4].id1 & 015) == 1 &&
               (s->people[5].id1 & 015) == 1 &&
               (s->people[6].id1 & 015) == 1 &&
               (s->people[7].id1 & 015) == 1)
         return call_list_gcol;
   }
   else if (s->kind == s2x4) {
      if      ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_dpt;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_cdpt;
      else if ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_rcol;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_lcol;
      else if ((s->people[0].id1 & 017) == 1 &&
               (s->people[1].id1 & 017) == 3 &&
               (s->people[2].id1 & 017) == 1 &&
               (s->people[3].id1 & 017) == 3 &&
               (s->people[4].id1 & 017) == 3 &&
               (s->people[5].id1 & 017) == 1 &&
               (s->people[6].id1 & 017) == 3 &&
               (s->people[7].id1 & 017) == 1)
         return call_list_8ch;
      else if ((s->people[0].id1 & 017) == 3 &&
               (s->people[1].id1 & 017) == 1 &&
               (s->people[2].id1 & 017) == 3 &&
               (s->people[3].id1 & 017) == 1 &&
               (s->people[4].id1 & 017) == 1 &&
               (s->people[5].id1 & 017) == 3 &&
               (s->people[6].id1 & 017) == 1 &&
               (s->people[7].id1 & 017) == 3)
         return call_list_tby;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_lin;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_lout;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_rwv;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_lwv;
      else if ((s->people[0].id1 & 017) == 010 &&
               (s->people[1].id1 & 017) == 010 &&
               (s->people[2].id1 & 017) == 012 &&
               (s->people[3].id1 & 017) == 012 &&
               (s->people[4].id1 & 017) == 012 &&
               (s->people[5].id1 & 017) == 012 &&
               (s->people[6].id1 & 017) == 010 &&
               (s->people[7].id1 & 017) == 010)
         return call_list_r2fl;
      else if ((s->people[0].id1 & 017) == 012 &&
               (s->people[1].id1 & 017) == 012 &&
               (s->people[2].id1 & 017) == 010 &&
               (s->people[3].id1 & 017) == 010 &&
               (s->people[4].id1 & 017) == 010 &&
               (s->people[5].id1 & 017) == 010 &&
               (s->people[6].id1 & 017) == 012 &&
               (s->people[7].id1 & 017) == 012)
         return call_list_l2fl;
   }
   else if (s->kind == s_qtag)
      return call_list_qtag;

   return call_list_any;
}


extern callarray *assoc(begin_kind key, setup *ss, callarray *spec)
{
   callarray *p;
   int t, u, i, k, mask;

   for (p = spec; p; p = p->next) {

      /* First, we demand that the starting setup be correct.  Also, if a qualifier
         number was specified, it must match. */

      if ((begin_kind) p->start_setup != key) continue;

      /* During initialization, we will be called with a null pointer for ss.
         We need to be careful, and err on the side of acceptance. */

      if (!ss) goto good;

      if (p->qual_num != 0) {
         number_used = TRUE;
         if (p->qual_num != (current_number_fields & 0xF)+1) continue;
      }

      if ((search_qualifier) p->qualifier == sq_none) goto good;

      /* Note that we have to examine setups larger than the setup the
         qualifier is officially defined for.  If a qualifier were defined
         as being legal only on 1x4's (so that, in the database, we only had
         specifications of the sort "setup 1x4 1x4 qualifier wave_only") we could
         still find ourselves here with ss->kind equal to s2x4.  Why?  Because
         the setup could be a 2x4 and the splitter could be trying to decide
         whether to split the setup into parallel 1x4's.  This happens when
         trying to figure out whether to split a 2x4 into 1x4's or 2x2's for
         the call "recycle". */

      k = 0;   /* Many tests will find these values useful. */
      i = 2;

      switch ((search_qualifier) p->qualifier) {
         case sq_wave_only:                    /* 1x4 or 2x4 - waves; 2x2 - real RH or LH box */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
               case cr_magic_only:
                  goto bad;
            }

            switch (ss->kind) {
               case s1x4:
                  if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[3].id1) != 0) { k |=  t; i &=  t; }
                  if (!(k & ~i & 2)) goto good;
                  goto bad;
               case s2x2:
                  u = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;

                  if ((u & 1) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |=  t; i &=  t; }
                     if (!(k & ~i & 2)) goto good;
                  }
                  else if ((u & 010) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                     if (!(k & ~i & 2)) goto good;
                  }

                  goto bad;
               case s2x4:
                  if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[2].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[4].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[5].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[6].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[7].id1) != 0) { k |=  t; i &=  t; }
                  if (!(k & ~i & 2)) goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_in_or_out:                    /* 2x2 - all facing in or all facing out */
            switch (ss->kind) {
               case s2x2:
                  u = ss->people[0].id1 | ss->people[1].id1 | ss->people[2].id1 | ss->people[3].id1;

                  if ((u & 1) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                     if (!(k & ~i & 2)) goto good;
                  }
                  else if ((u & 010) == 0) {
                     if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                     if ((t = ss->people[1].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                     if ((t = ss->people[3].id1) != 0) { k |=  t; i &=  t; }
                     if (!(k & ~i & 2)) goto good;
                  }

                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_2fl_only:                     /* 1x4 or 2x4 - 2FL; 4x1 - single DPT or single CDPT */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_wave_only:
               case cr_magic_only:
                  goto bad;
            }

            switch (ss->kind) {
               case s1x4:
                  if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[1].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                  if (!(k & ~i & 2)) goto good;
                  goto bad;
               case s2x4:
                  if ((t = ss->people[0].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[1].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[2].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[3].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[4].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[5].id1) != 0) { k |= ~t; i &= ~t; }
                  if ((t = ss->people[6].id1) != 0) { k |=  t; i &=  t; }
                  if ((t = ss->people[7].id1) != 0) { k |=  t; i &=  t; }
                  if (!(k & ~i & 2)) goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_miniwaves:                    /* miniwaves everywhere */
            switch (ss->cmd.cmd_assume.assumption) {
               case cr_1fl_only:
               case cr_2fl_only:
                  goto bad;
            }

            switch (ss->kind) {
               case s1x2:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s1x4:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s1x8:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[6].id1) & (u = ss->people[7].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[5].id1) & (u = ss->people[4].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s2x4:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[2].id1) & (u = ss->people[3].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[5].id1) & (u = ss->people[4].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[7].id1) & (u = ss->people[6].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  k = 1;
                  i = 2;
                  if ((t = ss->people[0].id1) & (u = ss->people[7].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[1].id1) & (u = ss->people[6].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[2].id1) & (u = ss->people[5].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[4].id1)) { k &= t&u; i &= t^u; }
                  if ((i & 2) && (k & 1)) goto good;
                  goto bad;
               case s2x2:
                  if ((t = ss->people[0].id1) & (u = ss->people[1].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  k = 1;
                  i = 2;
                  if ((t = ss->people[0].id1) & (u = ss->people[3].id1)) { k &= t&u; i &= t^u; }
                  if ((t = ss->people[1].id1) & (u = ss->people[2].id1)) { k &= t&u; i &= t^u; }
                  if ((i & 2) && (k & 1)) goto good;
                  goto bad;
               case sdmd:
                  k = 1;
                  i = 2;
                  if ((t = ss->people[1].id1) & (u = ss->people[3].id1)) { k &= t&u; i &= t^u; }
                  if ((i & 2) && (k & 1)) goto good;
                  goto bad;
               case s_trngl:
                  if ((t = ss->people[1].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               case s_qtag:
                  if ((t = ss->people[6].id1) & (u = ss->people[7].id1)) { k |= t|u; i &= t^u; }
                  if ((t = ss->people[3].id1) & (u = ss->people[2].id1)) { k |= t|u; i &= t^u; }
                  if ((i & 2) && !(k & 1)) goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_rwave_only:
            if (ss->kind == s1x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x8) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_north))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_west))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_west))
               goto good;
               goto bad;
            }
            else if (ss->kind == sdmd) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_west))
               goto good;
               goto bad;
            }
            else if (ss->kind == s_trngl) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else if (ss->kind == s_qtag) {
               if ((!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_south))
               goto good;
               goto bad;
            }
            else {
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_lwave_only:
            if (ss->kind == s1x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s1x8) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x4) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_south))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[4].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[5].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_east))
               goto good;
               goto bad;
            }
            else if (ss->kind == s2x2) {
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south))
               goto good;
               if ((!(t = ss->people[0].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[1].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_east) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_east))
               goto good;
               goto bad;
            }
            else if (ss->kind == sdmd) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_west) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_east))
               goto good;
               goto bad;
            }
            if (ss->kind == s_trngl) {
               if ((!(t = ss->people[1].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[2].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else if (ss->kind == s_qtag) {
               if ((!(t = ss->people[2].id1 & d_mask) || t == d_north) &&
                   (!(t = ss->people[3].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[6].id1 & d_mask) || t == d_south) &&
                   (!(t = ss->people[7].id1 & d_mask) || t == d_north))
               goto good;
               goto bad;
            }
            else {
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_ctrwv_end2fl:
            /* Note that this qualifier is kind of strict.  We won't permit the call "with
               confidence" do be done unless everyone can trivially determine which
               part to do. */
            if (ss->kind == s_crosswave) {
               if (((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 0 &&
                   ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 0 &&
                   ((ss->people[2].id1 | ss->people[3].id1) == 0 || ((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 2) &&
                   ((ss->people[6].id1 | ss->people[7].id1) == 0 || ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 2))
               goto good;
               goto bad;
            }
            else {
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_ctr2fl_endwv:
            /* Note that this qualifier is kind of strict.  We won't permit the call "with
               confidence" do be done unless everyone can trivially determine which
               part to do. */
            if (ss->kind == s_crosswave) {
               if (((ss->people[2].id1 ^ ss->people[3].id1) & d_mask) == 0 &&
                   ((ss->people[6].id1 ^ ss->people[7].id1) & d_mask) == 0 &&
                   ((ss->people[0].id1 | ss->people[1].id1) == 0 || ((ss->people[0].id1 ^ ss->people[1].id1) & d_mask) == 2) &&
                   ((ss->people[4].id1 | ss->people[5].id1) == 0 || ((ss->people[4].id1 ^ ss->people[5].id1) & d_mask) == 2))
               goto good;
               goto bad;
            }
            else {
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_true_Z:                    /* 2x3, 3x4, or 2x6 - real Z spots occupied, so can do Z axle */
            mask = 0;

            for (i=0, k=1; i<=setup_attrs[ss->kind].setup_limits; i++, k<<=1) {
               if (ss->people[i].id1) mask |= k;
            }

            if (ss->kind == s2x3) {
               if (mask == 033 || mask == 066) goto good;
               goto bad;
            }
            else if (ss->kind == s3x4) {
               if (mask == 0xEBA || mask == 0xD75) goto good;
               goto bad;
            }
            else if (ss->kind == s2x6) {
               if (mask == 03333 || mask == 06666) goto good;
               goto bad;
            }
            else {
               goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_3_4_tag:                      /* dmd or qtag - is a 3/4 tag, i.e. points looking out */
            switch (ss->kind) {
               case sdmd:
                  if ((!(t = ss->people[0].id1 & d_mask) || t == d_west) &&
                      (!(t = ss->people[2].id1 & d_mask) || t == d_east))
                  goto good;
                  goto bad;
               case s_qtag:
                  if ((!(t = ss->people[0].id1 & d_mask) || t == d_north) &&
                      (!(t = ss->people[1].id1 & d_mask) || t == d_north) &&
                      (!(t = ss->people[4].id1 & d_mask) || t == d_south) &&
                      (!(t = ss->people[5].id1 & d_mask) || t == d_south))
                  goto good;
                  goto bad;
               default:
                  goto good;                 /* We don't understand the setup -- we'd better accept it. */
            }
         case sq_dmd_same_pt:                   /* dmd or pdmd - centers would circulate to same point */
            if (((ss->people[1].id1 & 01011) == d_east) &&                        /* faces either east or west */
                (!((ss->people[3].id1 ^ ss->people[1].id1) & d_mask)))   /* and both face same way */
               goto good;
            goto bad;
         case sq_dmd_facing:                    /* dmd or pdmd - diamond is fully occupied and fully facing */
            if ((ss->people[0].id1 & d_mask) == d_north &&
                (ss->people[1].id1 & d_mask) == d_west &&
                (ss->people[2].id1 & d_mask) == d_south &&
                (ss->people[3].id1 & d_mask) == d_east)
               goto good;
            if ((ss->people[0].id1 & d_mask) == d_south &&
                (ss->people[1].id1 & d_mask) == d_east &&
                (ss->people[2].id1 & d_mask) == d_north &&
                (ss->people[3].id1 & d_mask) == d_west)
               goto good;
            goto bad;
         case sq_8_chain:                   /* 4x1/4x2 - setup is (single) 8-chain */
            switch (ss->kind) {
               case s1x4:
                  if (  !((ss->people[0].id1 | ss->people[3].id1) & 2) &&
                        (!ss->people[1].id1 || (ss->people[1].id1 & 2)) &&
                        (!ss->people[2].id1 || (ss->people[2].id1 & 2)))
                     goto good;
                  break;
               case s2x4:
                  if (  !((ss->people[0].id1 | ss->people[2].id1 | ss->people[5].id1 | ss->people[7].id1) & 2) &&
                        (!ss->people[1].id1 || (ss->people[1].id1 & 2)) &&
                        (!ss->people[3].id1 || (ss->people[3].id1 & 2)) &&
                        (!ss->people[4].id1 || (ss->people[4].id1 & 2)) &&
                        (!ss->people[6].id1 || (ss->people[6].id1 & 2)))
                     goto good;
                  break;
            }
            goto bad;
         case sq_trade_by:                  /* 4x1/4x2 - setup is (single) trade-by */
            switch (ss->kind) {
               case s1x4:
                  if (  !((ss->people[1].id1 | ss->people[2].id1) & 2) &&
                        (!ss->people[0].id1 || (ss->people[0].id1 & 2)) &&
                        (!ss->people[3].id1 || (ss->people[3].id1 & 2)))
                     goto good;
                  break;
               case s2x4:
                  if (  !((ss->people[1].id1 | ss->people[3].id1 | ss->people[4].id1 | ss->people[6].id1) & 2) &&
                        (!ss->people[0].id1 || (ss->people[0].id1 & 2)) &&
                        (!ss->people[2].id1 || (ss->people[2].id1 & 2)) &&
                        (!ss->people[5].id1 || (ss->people[5].id1 & 2)) &&
                        (!ss->people[7].id1 || (ss->people[7].id1 & 2)))
                     goto good;
                  break;
            }
            goto bad;
         case sq_split_dixie:
            if (ss->cmd.cmd_final_flags & FINAL__SPLIT_DIXIE_APPROVED) goto good;
            goto bad;
         case sq_not_split_dixie:
            if (!(ss->cmd.cmd_final_flags & FINAL__SPLIT_DIXIE_APPROVED)) goto good;
            goto bad;
      }

      bad: ;
   }

   good:
   return p;
}


extern uint32 find_calldef(
   callarray *tdef,
   setup *scopy,
   int real_index,
   int real_direction,
   int northified_index)
{
   unsigned short *calldef_array;
   predptr_pair *predlistptr;
   unsigned int z;

   if (tdef->callarray_flags & CAF__PREDS) {
      predlistptr = tdef->stuff.prd.predlist;
      while (predlistptr != (predptr_pair *) 0) {
         if ((*(predlistptr->pred))(scopy, real_index, real_direction, northified_index)) {
            calldef_array = predlistptr->arr;
            goto got_it;
         }
         predlistptr = predlistptr->next;
      }
      fail(tdef->stuff.prd.errmsg);
   }
   else
      calldef_array = tdef->stuff.def;

got_it:

   z = calldef_array[northified_index];
   if (!z) {
      collision_person1 = scopy->people[real_index].id1;
      error_message1[0] = '\0';
      error_message2[0] = '\0';
      longjmp(longjmp_ptr->the_buf, 6);
   }

   return z;
}


extern void clear_people(setup *z)
{
   (void) memset(z->people, 0, sizeof(personrec)*MAX_PEOPLE);
}


extern uint32 rotperson(uint32 n, int amount)
{
   if (n == 0) return 0; else return (n + amount) & ~064;
}


extern uint32 rotcw(uint32 n)
{
   if (n == 0) return 0; else return (n + 011) & ~064;
}


extern uint32 rotccw(uint32 n)
{
   if (n == 0) return 0; else return (n + 033) & ~064;
}


extern void clear_person(setup *resultpeople, int resultplace)
{
   resultpeople->people[resultplace].id1 = 0;
   resultpeople->people[resultplace].id2 = 0;
}


extern uint32 copy_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace)
{
   resultpeople->people[resultplace] = sourcepeople->people[sourceplace];
   return resultpeople->people[resultplace].id1;
}


extern uint32 copy_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount)
{
   unsigned int newperson = sourcepeople->people[sourceplace].id1;

   if (newperson) newperson = (newperson + rotamount) & ~064;
   resultpeople->people[resultplace].id2 = sourcepeople->people[sourceplace].id2;
   return resultpeople->people[resultplace].id1 = newperson;
}


extern void swap_people(setup *ss, int oneplace, int otherplace)
{
   personrec temp = ss->people[otherplace];
   ss->people[otherplace] = ss->people[oneplace];
   ss->people[oneplace] = temp;
}


extern void install_person(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace)
{
   unsigned int newperson = sourcepeople->people[sourceplace].id1;

   if (resultpeople->people[resultplace].id1 == 0)
      resultpeople->people[resultplace] = sourcepeople->people[sourceplace];
   else if (newperson) {
      collision_person1 = resultpeople->people[resultplace].id1;
      collision_person2 = newperson;
      error_message1[0] = '\0';
      error_message2[0] = '\0';
      longjmp(longjmp_ptr->the_buf, 3);
   }
}


extern void install_rot(setup *resultpeople, int resultplace, setup *sourcepeople, int sourceplace, int rotamount)
{
   unsigned int newperson = sourcepeople->people[sourceplace].id1;

   if (newperson) {
      if (resultpeople->people[resultplace].id1 == 0) {
         resultpeople->people[resultplace].id1 = (newperson + rotamount) & ~064;
         resultpeople->people[resultplace].id2 = sourcepeople->people[sourceplace].id2;
      }
      else {
         collision_person1 = resultpeople->people[resultplace].id1;
         collision_person2 = newperson;
         error_message1[0] = '\0';
         error_message2[0] = '\0';
         longjmp(longjmp_ptr->the_buf, 3);
      }
   }
}


#define MXN_BITS (INHERITFLAG_1X2 | INHERITFLAG_2X1 | INHERITFLAG_2X2 | INHERITFLAG_1X3 | INHERITFLAG_3X1 | INHERITFLAG_3X3 | INHERITFLAG_4X4)

/* Take a concept pointer and scan for all "final" concepts,
   returning an updated concept pointer and a mask of all such concepts found.
   "Final" concepts are those that modify the execution of a call but
   do not cause it to be executed in a virtual or distorted setup.
   This has a side-effect that is occasionally used:  When it passes over
   any "magic" or "interlocked" concept, it drops a pointer to where the
   last such occurred into the external variable "last_magic_diamond". */

extern parse_block *process_final_concepts(
   parse_block *cptr,
   long_boolean check_errors,
   final_set *final_concepts)
{
   parse_block *tptr = cptr;

   *final_concepts = 0;

   while (tptr) {
      final_set bit_to_set = 0;
      final_set bit_to_forbid = 0;

      switch (tptr->concept->kind) {
         case concept_comment:
            goto get_next;               /* Need to skip these. */
         case concept_triangle:
            bit_to_set = FINAL__TRIANGLE; break;
         case concept_magic:
            last_magic_diamond = tptr;
            bit_to_set = INHERITFLAG_MAGIC;
            bit_to_forbid = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
            break;
         case concept_interlocked:
            last_magic_diamond = tptr;
            bit_to_set = INHERITFLAG_INTLK;
            bit_to_forbid = INHERITFLAG_SINGLE | INHERITFLAG_DIAMOND;
            break;
         case concept_grand:
            bit_to_set = INHERITFLAG_GRAND;
            bit_to_forbid = INHERITFLAG_SINGLE;
            break;
         case concept_cross: bit_to_set = INHERITFLAG_CROSS; break;
         case concept_single:
            bit_to_set = INHERITFLAG_SINGLE;
            bit_to_forbid = INHERITFLAG_SINGLEFILE;
            break;
         case concept_singlefile:
            bit_to_set = INHERITFLAG_SINGLEFILE;
            bit_to_forbid = INHERITFLAG_SINGLE;
            break;
         case concept_1x2:
            bit_to_set = INHERITFLAG_1X2;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_2x1:
            bit_to_set = INHERITFLAG_2X1;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_2x2:
            bit_to_set = INHERITFLAG_2X2;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_1x3:
            bit_to_set = INHERITFLAG_1X3;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_3x1:
            bit_to_set = INHERITFLAG_3X1;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_3x3:
            bit_to_set = INHERITFLAG_3X3;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_4x4:
            bit_to_set = INHERITFLAG_4X4;
            bit_to_forbid = MXN_BITS;
            break;
         case concept_split:
            bit_to_set = FINAL__SPLIT; break;
         case concept_reverse:
            bit_to_set = INHERITFLAG_REVERSE; break;
         case concept_left:
            bit_to_set = INHERITFLAG_LEFT; break;
         case concept_12_matrix:
            if (check_errors && *final_concepts)
               fail("Matrix modifier must appear first.");
            bit_to_set = INHERITFLAG_12_MATRIX;
            break;
         case concept_16_matrix:
            if (check_errors && *final_concepts)
               fail("Matrix modifier must appear first.");
            bit_to_set = INHERITFLAG_16_MATRIX;
            break;
         case concept_diamond:
            bit_to_set = INHERITFLAG_DIAMOND;
            bit_to_forbid = INHERITFLAG_SINGLE;
            break;
         case concept_funny:
            bit_to_set = INHERITFLAG_FUNNY; break;
         default:
            goto exit5;
      }

      if (check_errors) {
         if (tptr->concept->level > calling_level) warn(warn__bad_concept_level);
   
         if (*final_concepts & bit_to_set)
            fail("Redundant call modifier.");
   
         if (*final_concepts & bit_to_forbid)
            fail("Illegal combination or order of call modifiers.");
      }

      *final_concepts |= bit_to_set;

      get_next:

      tptr = tptr->next;
   }

   exit5:

   return tptr;
}


/* Prepare several setups to be assembled into one, by making them all have
   the same kind and rotation. */
extern long_boolean fix_n_results(int arity, setup z[])
{
   int i;
   long_boolean lineflag = FALSE;
   long_boolean miniflag = FALSE;
   setup_kind kk = nothing;
   int rr = -1;

   /* There are 3 things that make this task nontrivial.  First, some setups could
      be "nothing", in which case we turn them into the same type of setup as
      their neighbors, with no people.  Second, some types of "grand working"
      can leave setups confused about whether they are lines or diamonds,
      because only the ends/points are occupied.  We turn those setups into
      whatever matches their more-fully-occupied neighbors.  (If completely in
      doubt, we opt for 1x4's.)  Third, some 1x4's may have been shrunk to 1x2's.
      This can happen in some cases of "triple box patch the so-and-so", which lead
      to triple lines in which the outer lines collapsed from 1x4's to 1x2's while
      the center line is fully occupied.  In this case we repopulate the outer lines
      to 1x4's. */

   for (i=0; i<arity; i++) {
      if (z[i].kind == s_normal_concentric) {
         /* We definitely have a problem.  A common way for this to happen is if a concentric call
            was done and there were no ends, so we don't know what the setup really is.  Example:
            from normal columns, do a split phantom lines hocus-pocus.  Do we leave space for the
            phantoms?  Humans would probably say yes because they know where the phantoms went, but
            this program has no idea in general.  If a call is concentrically executed and the
            outsides are all phantoms, we don't know what the setup is.  Concentric_move signifies
            this by creating a "concentric" setup with "nothing" for the outer setup.  So we raise
            an error that is somewhat descriptive. */
         if (z[i].outer.skind == nothing)
            fail("Can't do this: don't know where the phantoms went.");
         else if (z[i].inner.skind == nothing && z[i].outer.skind == s1x2) {
            /* We can fix this up.  Just turn it into a 1x4 with the ends missing.
            (If a diamond is actually required, that will get fixed up below.)
            The test case for this is split phantom lines cross to a diamond from 2FL. */

            z[i].kind = s1x4;
            z[i].rotation = z[i].outer.srotation;
            copy_person(&z[i], 0, &z[i], 12);
            copy_person(&z[i], 2, &z[i], 13);
            clear_person(&z[i], 1);
            clear_person(&z[i], 3);
         }
         else
            fail("Don't recognize ending setup for this call.");
      }

      if (z[i].kind != nothing) {
         canonicalize_rotation(&z[i]);

         if (z[i].kind == s1x2)
            miniflag = TRUE;
         else if ((z[i].kind == s1x4 || z[i].kind == sdmd) && (z[i].people[1].id1 | z[i].people[3].id1) == 0)
            lineflag = TRUE;
         else {
            if (kk == nothing) kk = z[i].kind;
            if (kk != z[i].kind) goto lose;
         }

         if (rr < 0) rr = z[i].rotation;
         /* If the setups are "trngl4", we allow mismatched rotation --
            the client will take care of it. */
         if (rr != z[i].rotation && z[i].kind != s_trngl4) goto lose;
      }
   }

   if (kk == nothing) {
      if (lineflag) kk = s1x4;
      else if (miniflag) kk = s1x2;
      else return TRUE;
   }
   
   /* If something wasn't sure whether it was points of a diamond or
      ends of a 1x4, that's OK if something else had a clue. */
   if (lineflag && kk != s1x4 && kk != sdmd) goto lose;

   /* If something was a 1x2, that's OK if something else was a 1x4. */
   if (miniflag && kk != s1x4 && kk != s1x2) goto lose;

   for (i=0; i<arity; i++) {
      if (z[i].kind == nothing)
         clear_people(&z[i]);
      else if (z[i].kind == s1x2 && kk == s1x4) {
         /* We have to expand a 1x2 to the center spots of a 1x4. */
         (void) copy_person(&z[i], 3, &z[i], 1);
         clear_person(&z[i], 2);
         (void) copy_person(&z[i], 1, &z[i], 0);
         clear_person(&z[i], 0);
      }

      z[i].kind = kk;
      if (kk != s_trngl4) z[i].rotation = rr;
   }

   return FALSE;

   lose:

   fail("This is a ridiculously inconsistent shape or orientation changer!!");
   /* NOTREACHED */
}
