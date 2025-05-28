/* SD -- square dance caller's helper.

    Copyright (C) 1990-2002  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following functions:
   get_escape_string
   write_history_line
   unparse_call_name
   print_recurse
   clear_screen
   write_header_stuff
   writechar
   newline
   open_text_line
   doublespace_file
   writestuff
   mark_parse_blocks
   release_parse_blocks_to_mark
   copy_parse_tree
   get_parse_block
   reset_parse_tree
   save_parse_state
   restore_parse_state
   string_copy
   display_initial_history
   initialize_parse
   run_program
and the following external variables:
   global_error_flag
   global_reply
   global_age
   global_leave_missing_calls_blank
   clipboard
   clipboard_size
   wrote_a_sequence
   retain_after_error
   outfile_string
   header_comment
   need_new_header_comment
   sequence_number
   starting_sequence_number
   old_filename_strings
   new_filename_strings
   filename_strings
   concept_key_table
*/


#include <string.h>

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"


// This is the line length beyond which we will take pity on
// whatever device has to print the result, and break the text line.
// It is presumably smaller than our internal text capacity.

// It is an observed fact that, with the default font (14 point Courier),
// one can print 66 characters on 8.5 x 11 inch paper.

#define MAX_PRINT_LENGTH 59


// External variables.
error_flag_type global_error_flag;
uims_reply global_reply;
int global_age;
bool global_leave_missing_calls_blank;
configuration *clipboard = (configuration *) 0;
int clipboard_size = 0;
long_boolean wrote_a_sequence = FALSE;
bool retain_after_error = false;
char outfile_string[MAX_FILENAME_LENGTH] = SEQUENCE_FILENAME;
char header_comment[MAX_TEXT_LINE_LENGTH];
long_boolean need_new_header_comment = FALSE;
int sequence_number = -1;
int starting_sequence_number;

// Under DJGPP, the default is always old-style filenames, because
// the underlying system (DOS or Windows 3.1) presumably can only
// handle "8.3" failenames.  Even under Windows NT, the emulation
// seems to handle only 8.3 filenames.  Under Windows 2000, it
// seems to handle long filenames, but is totally broken in other
// respects (compilation with Cygwin crashed in ntvdm.)  The bug
// has been reported to Microsoft, and, of course, never fixed.

#if defined(DJGPP)
Cstring *filename_strings = old_filename_strings;
#else
Cstring *filename_strings = old_filename_strings; // ******** For now
#endif

// BEWARE!!  These lists are keyed to the definition of "dance_level" in database.h
Cstring old_filename_strings[] = {
   ".MS",
   ".Plus",
   ".A1",
   ".A2",
   ".C1",
   ".C2",
   ".C3A",
   ".C3",
   ".C3X",
   ".C4A",
   ".C4",
   ".C4X",
   ".all",
   ".all",
   ""};

Cstring new_filename_strings[] = {
   "_MS.txt",
   "_Plus.txt",
   "_A1.txt",
   "_A2.txt",
   "_C1.txt",
   "_C2.txt",
   "_C3A.txt",
   "_C3.txt",
   "_C3X.txt",
   "_C4A.txt",
   "_C4.txt",
   "_C4X.txt",
   "_all.txt",
   "_all.txt",
   ""};

Cstring concept_key_table[] = {
   /* These are the standard bindings. */
   "cu     deleteline",
   "cx     deleteword",
   "e6     lineup",
   "e8     linedown",
   "e1     pageup",
   "e2     pagedown",
   "+f1    heads start",
   "+sf1   sides start",
   "+cf1   just as they are",
   "f2     two calls in succession",
   "sf2    twice",
   "f3     pick random call",
   "sf3    pick concept call",
   "cf3    pick simple call",
   "f4     resolve",
   "e4     resolve",            // home
   "sf4    reconcile",
   "cf4    normalize",
   "f5     refresh display",
   "sf5    keep picture",
   "cf5    insert a comment",
   "e13    insert a comment",   // insert
   "f6     simple modifications",
   "sf6    allow modifications",
   "cf6    centers",
   "f7     toggle concept levels",
   "+f7    toggle concept levels",
   "sf7    toggle active phantoms",
   "+sf7   toggle active phantoms",
   "f8     quoteanything",
   "sf8    cut to clipboard",
   "cf8    paste one call",
   "f9     undo last call",
   "+f9    exit from the program",
   "*f9    abort the search",
   "sf9    undo last call",
   "+sf9   exit from the program",
   "*sf9   abort the search",
   "f10    write this sequence",
   "*f10   write this sequence",
   "*e3    write this sequence",    // end
   "sf10   change output file",
   "+sf10  change output file",
   "f11    pick level call",
   "sf11   pick 8 person level call",
   "cf11   standardize",
   "*f12   find another",
   "*sf12  accept current choice",
   "*cf12  previous",
   "*af12  next",
   "*se6   raise reconcile point",  // shift up arrow
   "*e5    previous",               // left arrow
   "*e7    find another",           // right arrow
   "*se8   lower reconcile point",  // shift down arrow
   (char *) 0};


static Cstring sessions_init_table[] = {
   "[Options]",
   "",
   "[Sessions]",
   "+                    C1               1      Sample",
   "",
   "[Accelerators]",
   (char *) 0};




/* These variables are are global to this file. */

static long_boolean reply_pending;
static int clipboard_allocation = 0;


/* Getting blanks into all the right places in the presence of substitions,
   insertions, and elisions is way too complicated to do in the database, or
   to test.  For example, consider calls like "@4keep @5busy@1",
   "fascinat@pe@q@ning@o@t", or "spin the pulley@7 but @8".  So we try to
   help by never putting two blanks together, always putting blanks adjacent
   to the outside of brackets, and never putting blanks adjacent to the
   inside of brackets.  This procedure is part of that mechanism. */
static void write_blank_if_needed(void)
{
   if (writechar_block.lastchar != ' ' &&
       writechar_block.lastchar != '[' &&
       writechar_block.lastchar != '(' &&
       writechar_block.lastchar != '-') writechar(' ');
}

/* This examines the indicator character after an "@" escape.  If the escape
   is for something that is supposed to appear in the menu as a "<...>"
   wildcard, it returns that string.  If it is an escape that starts a
   substring that would normally be elided, as in "@7 ... @8", it returns
   a non-null pointer to a null character.  If it is an escape that is normally
   simply dropped, it returns a null pointer. */

SDLIB_API const char *get_escape_string(char c)
{
   switch (c) {
   case '0': case 'm': case 'T':
      return "<ANYTHING>";
   case 'N':
      return "<ANYCIRC>";
   case '6': case 'k':
      return "<ANYONE>";
   case 'h':
      return "<DIRECTION>";
   case '9':
      return "<N>";
   case 'a': case 'b': case 'B': case 'D':
      return "<N/4>";
   case 'u':
      return "<Nth>";
   case 'v': case 'w': case 'x': case 'y':
      return "<ATC>";
   case '7': case 'n': case 'j': case 'J': case 'E': case 'Q':
      return "";
   default:
      return (char *) 0;
   }
}


static void write_nice_number(char indicator, int num)
{
   if (num < 0) {
      writestuff(get_escape_string(indicator));
   }
   else {
      switch (indicator) {
      case '9': case 'a': case 'b': case 'B': case 'D':
         if (indicator == '9')
            writestuff(cardinals[num]);
         else if (indicator == 'B' || indicator == 'D') {
            if (num == 1)
               writestuff("quarter");
            else if (num == 2)
               writestuff("half");
            else if (num == 3)
               writestuff("three quarter");
            else if (num == 4)
               writestuff("four quarter");
            else {
               writestuff(cardinals[num]);
               writestuff("/4");
            }
         }
         else if (num == 2)
            writestuff("1/2");
         else if (num == 4 && indicator == 'a')
            writestuff("full");
         else {
            writestuff(cardinals[num]);
            writestuff("/4");
         }
         break;
      case 'u':     /* Need to plug in an ordinal number. */
         writestuff(ordinals[num]);
         break;
      }
   }
}




static void writestuff_with_decorations(call_conc_option_state *cptr, Cstring f)
{
   uint32 index = cptr->number_fields;
   int howmany = cptr->howmanynumbers;

   while (f[0]) {
      if (f[0] == '@') {
         switch (f[1]) {
         case 'a': case 'b': case 'B': case 'D': case 'u': case '9':
            // DJGPP has a problem with this, need convert to int.
            write_nice_number(f[1], (howmany <= 0) ? -1 : (int) (index & 0xF));
            index >>= 4;
            howmany--;
            break;
         case '6':
            writestuff(selector_list[cptr->who].name_uc);
            break;
         case 'k':
            writestuff(selector_list[cptr->who].sing_name_uc);
            break;
         default:   /**** maybe we should really do what "translate_menu_name"
                          does, using call to "get_escape_string". */
            break;
         }
         
         f += 2;
      }
      else
         writechar(*f++);
   }
}


static void printperson(uint32 x)
{
   if (x & BIT_PERSON) {
      if (enable_file_writing || ui_options.use_escapes_for_drawing_people == 0) {
         /* We never write anything other than standard ASCII to the transcript file. */
         writechar(' ');
         writechar(ui_options.pn1[(x >> 6) & 7]);
         writechar(ui_options.pn2[(x >> 6) & 7]);
         if (enable_file_writing)
            writechar(ui_options.stddirec[x & 017]);
         else
            writechar(ui_options.direc[x & 017]);
      }
      else {
         /* Write an escape sequence, so that the user interface can display
            the person in color. */
         writechar('\013');
         writechar((char) (((x >> 6) & 7) | 040));
         writechar((char) ((x & 017) | 040));
      }
   }
   else {
      if (enable_file_writing || ui_options.use_escapes_for_drawing_people <= 1)
         writestuff("  . ");
      else
         writechar('\014');  /* If we have full ("checker") escape sequences, use this. */
   }
}


/* These static variables are used by printsetup/print_4_person_setup/do_write. */

static int offs, roti, modulus, personstart;
static setup *printarg;

static void do_write(Cstring s)
{
   char c;

   int ri = roti * 011;

   for (;;) {
      if (!(c=(*s++))) return;
      else if (c == '@') {
         if (*s == '7') {
            s++;
            ui_options.squeeze_this_newline = 1;
            newline();
            ui_options.squeeze_this_newline = 0;
         }
         else
            newline();
      }
      else if (c >= 'a' && c <= 'x')
         printperson(rotperson(printarg->people[personstart + ((c-'a'-offs)%modulus)].id1, ri));
      else {
         /* We need to do the mundane translation of "5" and "6" if the result
            isn't going to be used by something that uses same. */
         if (enable_file_writing ||
             ui_options.use_escapes_for_drawing_people <= 1 ||
             ui_options.no_graphics != 0) {
            if (c == '6')
               writestuff("    ");    /* space equivalent to 1 full glyph. */
            else if (c == '9')
               writestuff("   ");     /* space equivalent to 3/4 glyph. */
            else if (c == '5')
               writestuff("  ");      /* space equivalent to 1/2 glyph. */
            else if (c == '8')
               writestuff(" ");       /* Like 5, but one space less if doing ASCII.
                                         (Exactly same as 5 if doing checkers. */
            else
               writechar(c);
         }
         else
            writechar(c);
      }
   }
}



static void print_4_person_setup(int ps, small_setup *s, int elong)
{
   Cstring str;

   modulus = setup_attrs[s->skind].setup_limits+1;
   roti = (s->srotation & 3);
   personstart = ps;

   offs = (((roti >> 1) & 1) * (modulus / 2)) - modulus;

   if (s->skind == s2x2) {
      if (elong < 0)
         str = "ab@dc@";
      else if (elong == 1)
         str = "a6b@d6c@";
      else if (elong == 2)
         str = "ab@@@dc@";
      else
         str = "a6b@@@d6c@";
   }
   else
      str = setup_attrs[s->skind].print_strings[roti & 1];

   if (str) {
      newline();
      do_write(str);
   }
   else
      writestuff(" ????");

   newline();
}



static void printsetup(setup *x)
{
   Cstring str;

   ui_options.drawing_picture = 1;
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
      str = setup_attrs[x->kind].print_strings[0];
   }
   else {
      offs = ((roti & 2) * (modulus / 4)) - modulus;
      str = setup_attrs[x->kind].print_strings[roti & 1];
   }

   if (str)
      do_write(str);
   else {
      switch (x->kind) {
         /* For a while, we took out the table entry, and used this.
            This was done so that the dots wouldn't be printed
            in a squared set.  We have changed it back.
      case s4x4:
         // Look for squared-set formation, and, if so, don't draw
         // the dots for the phantoms.
         if (!(x->people[0].id1 | x->people[3].id1 |
               x->people[4].id1 | x->people[7].id1 |
               x->people[8].id1 | x->people[11].id1 |
               x->people[12].id1 | x->people[15].id1))
            do_write("6  n  o@@k  6  6  b@@j  6  6  c@@6  g  f");
         else
            do_write("m  n  o  a@@k  p  d  b@@j  l  h  c@@i  g  f  e");

         break;
         */
      case s_qtag:
         if ((x->people[0].id1 & x->people[1].id1 &
              x->people[4].id1 & x->people[5].id1 & 1) &&
             (x->people[2].id1 & x->people[3].id1 &
              x->people[6].id1 & x->people[7].id1 & 010)) {
            // People are in diamond-like orientation.
            if (x->rotation & 1)
               do_write("6  g@7f  6  a@76  h@@6  d@7e  6  b@76  c");
            else
               do_write("5 a6 b@@g h d c@@5 f6 e");
         }
         else {
            // People are not.  Probably 1/4-tag-like orientation.
            if (x->rotation & 1)
               do_write("6  g@f  h  a@e  d  b@6  c");
            else
               do_write("6  a  b@@g  h  d  c@@6  f  e");
         }
         break;
      case s_c1phan:
         // Look for nice "classic C1 phantom" occupations, and,
         // if so, draw tighter diagram.
         if (!(x->people[0].id1 | x->people[2].id1 |
               x->people[4].id1 | x->people[6].id1 |
               x->people[8].id1 | x->people[10].id1 |
               x->people[12].id1 | x->people[14].id1))
            str = "8  b@786       h  f@78  d@7@868         l@7n  p@7868         j";
         else if (!(x->people[1].id1 | x->people[3].id1 |
                    x->people[5].id1 | x->people[7].id1 |
                    x->people[9].id1 | x->people[11].id1 |
                    x->people[13].id1 | x->people[15].id1))
            str = "868         e@7a  c@7868         g@7@8  o@786       k  i@78  m";
         else
            str = "58b66e@a88c  h88f@58d66g@@58o66l@n88p  k88i@58m66j";

         do_write(str);
         break;
      case s_dead_concentric:
         ui_options.drawing_picture = 0;
         writestuff(" centers only:");
         newline();
         ui_options.drawing_picture = 1;
         print_4_person_setup(0, &(x->inner), -1);
         break;
      case s_normal_concentric:
         ui_options.drawing_picture = 0;
         writestuff(" centers:");
         newline();
         ui_options.drawing_picture = 1;
         print_4_person_setup(0, &(x->inner), -1);
         ui_options.drawing_picture = 0;
         writestuff(" ends:");
         newline();
         ui_options.drawing_picture = 1;
         print_4_person_setup(12, &(x->outer), x->concsetup_outer_elongation);
         break;
      default:
         ui_options.drawing_picture = 0;
         writestuff("???? UNKNOWN SETUP ????");
         newline();
         ui_options.drawing_picture = 1;
      }
   }

   newline();
   ui_options.drawing_picture = 0;
   newline();
}




SDLIB_API void write_history_line(int history_index,
                                  bool picture,
                                  bool leave_missing_calls_blank,
                                  file_write_flag write_to_file,
                                  const char *header)
{
   global_leave_missing_calls_blank = leave_missing_calls_blank;

   int w, i;
   parse_block *thing;
   configuration *this_item = &configuration::history[history_index];

   if (write_to_file == file_write_double && !ui_options.singlespace_mode)
      doublespace_file();

   // Do not put index numbers into output file -- user may edit it later.

   if (!enable_file_writing && !diagnostic_mode) {
      i = history_index-configuration::whole_sequence_low_lim+1;
      if (i > 0) {
         char indexbuf[10];
         sprintf(indexbuf, "%2d:   ", i);
         writestuff(indexbuf);
      }
   }

   if (this_item->nontrivial_startinfo_specific()) {
      if (this_item->get_startinfo_specific()->into_the_middle) goto morefinal;
      writestuff(this_item->get_startinfo_specific()->name);
      goto final;
   }

   thing = this_item->command_root;
   
   /* Need to check for the special case of starting a sequence with heads or sides.
      If this is the first line of the history, and we started with heads of sides,
      change the name of this concept from "centers" to the appropriate thing. */

   if (history_index == 2 &&
       thing->concept->kind == concept_centers_or_ends &&
       thing->concept->value.arg1 == selector_centers) {
      if (configuration::history[1].get_startinfo_specific()->into_the_middle) {
         writestuff(configuration::history[1].get_startinfo_specific()->name);
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

   // First, don't print both "bad concept level" and "bad modifier level".

   if (this_item->test_one_warning_specific(warn__bad_concept_level))
      this_item->clear_one_warning_specific(warn__bad_modifier_level);

   // Or "opt for parallelogram" and "each 1x4".

   if (this_item->test_one_warning_specific(warn__check_pgram))
      this_item->clear_one_warning_specific(warn__each1x4);

   // Or "each 1x6" and "each 1x3".

   if (this_item->test_one_warning_specific(warn__split_1x6))
      this_item->clear_one_warning_specific(warn__split_to_1x6s);

   if (!ui_options.nowarn_mode) {
      for (w=0 ; w<warn__NUM_WARNINGS ; w++) {
         if (this_item->test_one_warning_specific((warning_index) w)) {
            writestuff("  Warning:  ");
            writestuff(&warning_strings[w][1]);
            newline();
         }
      }
   }

   if (picture || this_item->draw_pic) {
      printsetup(&this_item->state);

      if (this_item->state.result_flags & RESULTFLAG__PLUSEIGHTH_ROT) {
         writestuff("  Note:  Actual setup is 45 degrees clockwise from diagram above.");
         newline();
      }
   }

   // Record that this history item has been written to the UI.
   this_item->text_line = text_line_count;
}





SDLIB_API void unparse_call_name(Cstring name, char *s, call_conc_option_state *options)
{
   writechar_block_type saved_writeblock = writechar_block;
   writechar_block.destcurr = s;
   writechar_block.usurping_writechar = TRUE;

   writestuff_with_decorations(options, name);
   writechar('\0');

   writechar_block = saved_writeblock;
   writechar_block.usurping_writechar = FALSE;
}


/* There are 2 bits that are meaningful in the argument to "print_recurse":
         PRINT_RECURSE_STAR
   This means to print an asterisk for a call that is missing in the
   current type-in state.
         PRINT_RECURSE_CIRC
   This means that this is a circulate-substitute call, and should have any
   @O ... @P stuff elided from it. */

#define PRINT_RECURSE_STAR 1
#define PRINT_RECURSE_CIRC 2


SDLIB_API void print_recurse(parse_block *thing, int print_recurse_arg)
{
   parse_block *local_cptr;
   parse_block *next_cptr;
   long_boolean use_left_name = FALSE;
   long_boolean use_cross_name = FALSE;
   long_boolean use_magic_name = FALSE;
   long_boolean use_grand_name = FALSE;
   long_boolean use_intlk_name = FALSE;
   long_boolean allow_deferred_concept = TRUE;
   parse_block *deferred_concept = (parse_block *) 0;
   int deferred_concept_paren = 0;
   int comma_after_next_concept = 0;    /* 1 for comma, 2 for the word "all". */
   int did_comma = 0;                   /* Same as comma_after_next_concept. */
   long_boolean did_concept = FALSE;
   long_boolean last_was_t_type = FALSE;
   long_boolean last_was_l_type = FALSE;
   long_boolean request_final_space = FALSE;

   local_cptr = thing;

   while (local_cptr) {
      concept_kind k;
      const concept_descriptor *item;

      item = local_cptr->concept;
      k = item->kind;

      if (k == concept_comment) {
         comment_block *fubb;

         fubb = (comment_block *) local_cptr->call_to_print;
         if (request_final_space) writestuff(" ");
         writestuff("{ ");
         writestuff(fubb->txt);
         writestuff(" } ");
         local_cptr = local_cptr->next;
         request_final_space = FALSE;
         last_was_t_type = FALSE;
         last_was_l_type = FALSE;
         comma_after_next_concept = 0;
      }
      else if (k > marker_end_of_list) {
         /* This is a concept. */

         long_boolean force = FALSE;
         int request_comma_after_next_concept = 0;           /* Same as comma_after_next_concept. */

         /* Some concepts look better with a comma after them. */

         if (item->concparseflags & CONCPARSE_PARSE_F_TYPE) {
            /* This is an "F" type concept. */
            comma_after_next_concept = 1;
            last_was_t_type = FALSE;
            force = did_concept && !last_was_l_type;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }
         else if (item->concparseflags & CONCPARSE_PARSE_L_TYPE) {
            /* This is an "L" type concept. */
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else if (item->concparseflags & CONCPARSE_PARSE_G_TYPE) {
            /* This is a "leading T/trailing L" type concept, also known as a "G" concept. */
            force = last_was_t_type && !last_was_l_type;;
            last_was_t_type = FALSE;
            last_was_l_type = TRUE;
         }
         else {
            /* This is a "T" type concept. */
            if (did_concept && k != concept_tandem_in_setup) comma_after_next_concept = 1;
            force = last_was_t_type && !last_was_l_type;
            last_was_t_type = TRUE;
            last_was_l_type = FALSE;
            did_concept = TRUE;
         }

         // We never put a comma before things like "in a 1/4 tag".
         if (force && did_comma == 0 && k != concept_tandem_in_setup) writestuff(", ");
         else if (request_final_space) writestuff(" ");

         next_cptr = local_cptr->next;    /* Now it points to the thing after this concept. */

         request_final_space = FALSE;

         if (concept_table[k].concept_prop & CONCPROP__SECOND_CALL) {
            parse_block *subsidiary_ptr = local_cptr->subsidiary_root;

            if (k == concept_centers_and_ends) {
               if (item->value.arg1 == selector_center6 ||
                   item->value.arg1 == selector_center2 ||
                   item->value.arg1 == selector_verycenters)
                  writestuff(selector_list[item->value.arg1].name_uc);
               else
                  writestuff(selector_list[selector_centers].name_uc);

               writestuff(" ");
            }
            else if (k == concept_some_vs_others) {
               selective_key sk = (selective_key) item->value.arg1;

               if (sk == selective_key_dyp)
                  writestuff_with_decorations(&local_cptr->options, "DO YOUR PART, @6 ");
               else if (sk == selective_key_own)
                  writestuff_with_decorations(&local_cptr->options, "OWN THE @6, ");
               else if (sk == selective_key_plain)
                  writestuff_with_decorations(&local_cptr->options, "@6 ");
               else
                  writestuff_with_decorations(&local_cptr->options, "@6 DISCONNECTED ");
            }
            else if (k == concept_sequential) {
               writestuff("(");
            }
            else if (k == concept_special_sequential_num) {
               writestuff("USE ");

               // If stuff hasn't been completely entered, show the number.
               if (!local_cptr->next || !subsidiary_ptr)
                  writestuff_with_decorations(&local_cptr->options, "(for @u part) ");
            }
            else if (k == concept_replace_nth_part ||
                     k == concept_replace_last_part ||
                     k == concept_interrupt_at_fraction) {
               if (local_cptr->next && subsidiary_ptr) {
                  writestuff("DELAY: ");
               }
               else {
                  switch (local_cptr->concept->value.arg1) {
                  case 0:
                     writestuff("replace the last part of ");
                     if (!local_cptr->next) writestuff("this call:");
                     break;
                  case 1:
                     writestuff("interrupt before the last part of ");
                     if (!local_cptr->next) writestuff("this call:");
                     break;
                  case 2:
                     if (!local_cptr->next)
                        writestuff_with_decorations(&local_cptr->options,
                                                    "interrupt this call after @9/@9:");
                     else
                        writestuff("interrrupt ");
                     break;
                  case 8:
                     writestuff_with_decorations(&local_cptr->options,
                                                 "replace the @u part of ");
                     if (!local_cptr->next) writestuff("this call:");
                     break;
                  case 9:
                     writestuff_with_decorations(&local_cptr->options,
                                                 "interrupting after the @u part of ");
                     if (!local_cptr->next) writestuff("this call:");
                     break;
                  }
               }
            }
            else {
               writestuff(item->name);
               writestuff(" ");
            }

            print_recurse(local_cptr->next, 0);

            // If echoing incomplete input, we can't show the second call.
            // But we may want to show something useful.
            if (!subsidiary_ptr) {
               if (local_cptr->next) {
                  if (k == concept_replace_nth_part ||
                      k == concept_replace_last_part ||
                      k == concept_interrupt_at_fraction) {
                     switch (local_cptr->concept->value.arg1) {
                     case 0: case 1: case 8: case 9:
                        writestuff(" with this call:");
                        break;
                     case 2:
                        writestuff_with_decorations(&local_cptr->options,
                                                    " after @9/@9 with this call:");
                        break;
                     }
                  }
               }
               break;
            }

            did_concept = FALSE;                /* We're starting over. */
            last_was_t_type = FALSE;
            last_was_l_type = FALSE;
            comma_after_next_concept = 0;
            request_final_space = TRUE;

            if (k == concept_centers_and_ends) {
               if (item->value.arg2)
                  writestuff(" WHILE THE ENDS CONCENTRIC");
               else
                  writestuff(" WHILE THE ENDS");
            }
            else if (k == concept_some_vs_others &&
                     (selective_key) item->value.arg1 != selective_key_own) {
               selector_kind opp = selector_list[local_cptr->options.who].opposite;
               writestuff(" WHILE THE ");
               writestuff((opp == selector_uninitialized) ?
                          ((Cstring) "OTHERS") :
                          selector_list[opp].name_uc);
            }
            else if (k == concept_on_your_own)
               writestuff(" AND");
            else if (k == concept_interlace)
               writestuff(" WITH");
            else if (k == concept_sandwich)
               writestuff(" AROUND");
            else if (k == concept_special_sequential_num) {
               writestuff_with_decorations(&local_cptr->options, " FOR THE @u PART: ");
               request_final_space = FALSE;
            }
            else if (k == concept_replace_nth_part ||
                     k == concept_replace_last_part ||
                     k == concept_interrupt_at_fraction) {
               writestuff(" BUT ");
               writestuff_with_decorations(&local_cptr->options, local_cptr->concept->name);
               writestuff(" WITH A [");
               request_final_space = FALSE;
            }
            else if (k == concept_sequential)
               writestuff(" ;");
            else if (k == concept_special_sequential) {
               if (item->value.arg1 == 2)
                  writestuff(" :");   // This is "start with".
               else if (item->value.arg1 == 4)
                  writestuff(" IN");
               else
                  writestuff(",");
            }
            else
               writestuff(" BY");

            // Note that we leave "allow_deferred_concept" on.  This means that
            // if we say "twice" immediately inside the second clause
            // of an interlace, it will get the special processing.
            // The first clause will get the special processing by virtue
            // of the recursion.

            next_cptr = subsidiary_ptr;

            // Setting this means that, if the second argument uses "twice",
            // we will put it in parens.  This is needed to disambiguate
            // this situation from the use of "twice" before the entire
            // "interlace".
            if (deferred_concept_paren == 0) deferred_concept_paren = 2;
         }
         else {
            const call_with_name *target_call = (call_with_name *) 0;
            const parse_block *tptr;

            // Look for special concepts that, in conjunction with calls that have
            // certain escape codes in them, get deferred and inserted into the call name.

            if (local_cptr && (k == concept_left ||
                               k == concept_cross ||
                               k == concept_magic ||
                               k == concept_grand ||
                               k == concept_interlocked)) {
   
               /* These concepts want to take special action if there are no following
                  concepts and certain escape characters are found in the name of
                  the following call. */
   
               finalflags junk_concepts;

               junk_concepts.her8it = 0;
               junk_concepts.final = 0;
               
               /* Skip all final concepts, then demand that what remains is a marker
                  (as opposed to a serious concept), and that a real call
                  has been entered, and that its name starts with "@g". */
               tptr = process_final_concepts(next_cptr, FALSE, &junk_concepts);
   
               if (tptr && tptr->concept->kind <= marker_end_of_list) target_call = tptr->call_to_print;
            }

            if (target_call &&
                k == concept_left &&
                (target_call->the_defn.callflagsf & ESCAPE_WORD__LEFT)) {
               use_left_name = TRUE;
            }
            else if (target_call &&
                     k == concept_magic &&
                     (target_call->the_defn.callflagsf & ESCAPE_WORD__MAGIC)) {
               use_magic_name = TRUE;
            }
            else if (target_call &&
                     k == concept_grand &&
                     (target_call->the_defn.callflagsf & ESCAPE_WORD__GRAND)) {
               use_grand_name = TRUE;
            }
            else if (target_call &&
                     k == concept_interlocked &&
                     (target_call->the_defn.callflagsf & ESCAPE_WORD__INTLK)) {
               use_intlk_name = TRUE;
            }
            else if (target_call &&
                     k == concept_cross &&
                     (target_call->the_defn.callflagsf & ESCAPE_WORD__CROSS)) {
               use_cross_name = TRUE;
            }
            else if (allow_deferred_concept &&
                     next_cptr &&
                     (k == concept_n_times_const ||
                      k == concept_n_times ||
                      (k == concept_fractional && item->value.arg1 == 2))) {
               deferred_concept = local_cptr;
               comma_after_next_concept = 0;
               did_concept = FALSE;

               // If there is another concept, we need parens.
               if (next_cptr->concept->kind > marker_end_of_list) deferred_concept_paren |= 1;

               if (deferred_concept_paren == 3) writestuff("(");
               if (deferred_concept_paren) writestuff("(");
            }
            else {
               if ((k == concept_meta_one_arg &&
                    item->value.arg1 == meta_key_nth_part_work) ||
                   (k == concept_meta_two_args &&
                    item->value.arg1 == meta_key_first_frac_work) ||
                   (k == concept_snag_mystic && item->value.arg1 == CMD_MISC2__SAID_INVERT) ||
                   (k == concept_meta &&
                    (item->value.arg1 == meta_key_initially ||
                     item->value.arg1 == meta_key_finally ||
                     item->value.arg1 == meta_key_initially_and_finally))) {
                  /* This is "DO THE <Nth> PART",
                     or INVERT followed by another concept, which must be SNAG or MYSTIC,
                     or INITIALLY/FINALLY.
                     These concepts require a comma after the following concept. */
                  request_comma_after_next_concept = 1;
               }
               else if (k == concept_so_and_so_only &&
                        ((selective_key) item->value.arg1) == selective_key_work_concept) {
                  /* "<ANYONE> WORK" */
                  /* This concept requires the word "all" after the following concept. */
                  request_comma_after_next_concept = 2;
               }

               writestuff_with_decorations(&local_cptr->options, local_cptr->concept->name);
               request_final_space = TRUE;
            }

            /* For some concepts, we still permit the "defer" stuff.  But don't do it
               if others are doing the call, because that would lead to
               "<anyone> work 1-1/2, swing thru" turning into
               "<anyone> work swing thru 1-1/2". */

            if ((k != concept_so_and_so_only || item->value.arg2) &&
                k != concept_centers_or_ends &&
                k != concept_c1_phantom &&
                k != concept_tandem)
               allow_deferred_concept = FALSE;
         }

         if (comma_after_next_concept == 2) {
            writestuff(", ALL");
            request_final_space = TRUE;
         }
         else if (comma_after_next_concept == 1) {
            writestuff(",");
            request_final_space = TRUE;
         }

         did_comma = comma_after_next_concept;

         if (comma_after_next_concept == 3)
            comma_after_next_concept = 2;
         else
            comma_after_next_concept = request_comma_after_next_concept;

         if (comma_after_next_concept == 2 && next_cptr) {
            concept_kind kjunk;
            uint32 njunk;

            if (check_for_concept_group(next_cptr, &kjunk, &njunk))
               comma_after_next_concept = 3;    // Will try again later.
         }

         local_cptr = next_cptr;

         if (k == concept_sequential) {
            if (request_final_space) writestuff(" ");
            print_recurse(local_cptr, PRINT_RECURSE_STAR);
            writestuff(")");
            return;
         }
         else if (k == concept_replace_nth_part ||
                  k == concept_replace_last_part ||
                  k == concept_interrupt_at_fraction) {
            if (request_final_space) writestuff(" ");
            print_recurse(local_cptr, PRINT_RECURSE_STAR);
            writestuff("]");
            return;
         }
      }
      else {
         /* This is a "marker", so it has a call, perhaps with a selector and/or number.
            The call may be null if we are printing a partially entered line.  Beware. */

         parse_block *sub1_ptr;
         parse_block *sub2_ptr;
         parse_block *search;
         bool pending_subst1, pending_subst2;

         selector_kind i16junk = local_cptr->options.who;
         direction_kind idirjunk = local_cptr->options.where;
         uint32 number_list = local_cptr->options.number_fields;
         call_with_name *localcall = local_cptr->call_to_print;
         parse_block *save_cptr = local_cptr;

         bool subst1_in_use = false;
         bool subst2_in_use = false;

         if (request_final_space) writestuff(" ");

         if (k == concept_another_call_next_mod) {
            search = save_cptr->next;
            while (search) {
               parse_block *subsidiary_ptr = search->subsidiary_root;
               long_boolean this_is_subst1 = FALSE;
               long_boolean this_is_subst2 = FALSE;
               if (subsidiary_ptr) {
                  switch (search->replacement_key) {
                     case DFM1_CALL_MOD_ANYCALL/DFM1_CALL_MOD_BIT:
                     case DFM1_CALL_MOD_MAND_ANYCALL/DFM1_CALL_MOD_BIT:
                        this_is_subst1 = TRUE;
                        break;
                     case DFM1_CALL_MOD_OR_SECONDARY/DFM1_CALL_MOD_BIT:
                     case DFM1_CALL_MOD_MAND_SECONDARY/DFM1_CALL_MOD_BIT:
                        this_is_subst2 = TRUE;
                        break;
                  }

                  if (this_is_subst1) {
                     if (subst1_in_use) writestuff("ERROR!!!");
                     subst1_in_use = true;
                     sub1_ptr = subsidiary_ptr;
                  }

                  if (this_is_subst2) {
                     if (subst2_in_use) writestuff("ERROR!!!");
                     subst2_in_use = true;
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

         /* Call = NIL means we are echoing input and user hasn't entered call yet. */

         if (localcall) {
            char *np = localcall->name;

            if (enable_file_writing) localcall->the_defn.age = global_age;

            while (*np) {
               char c = *np++;

               if (c == '@') {
                  char savec = *np++;

                  switch (savec) {
                  case '6': case 'k':
                     write_blank_if_needed();
                     if (savec == '6')
                        writestuff(selector_list[i16junk].name);
                     else
                        writestuff(selector_list[i16junk].sing_name);
                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case 'v': case 'w': case 'x': case 'y':
                     write_blank_if_needed();
                     /* Find the base tag call that this is invoking. */

                     search = save_cptr->next;
                     while (search) {
                        parse_block *subsidiary_ptr = search->subsidiary_root;
                        if (subsidiary_ptr &&
                            subsidiary_ptr->call_to_print &&
                            (subsidiary_ptr->call_to_print->the_defn.callflags1 &
                             CFLAG1_BASE_TAG_CALL_MASK)) {
                           print_recurse(subsidiary_ptr, 0);
                           goto did_tagger;
                        }
                        search = search->next;
                     }

                     /* We didn't find the tagger.  It must not have been entered into
                           the parse tree.  See if we can get it from the "tagger" field. */

                     if (save_cptr->options.tagger != 0)
                        writestuff(tagger_calls
                                   [save_cptr->options.tagger >> 5]
                                   [(save_cptr->options.tagger & 0x1F)-1]->menu_name);
                     else
                        writestuff("NO TAGGER???");

                  did_tagger:

                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case 'N':
                     write_blank_if_needed();

                     /* Find the base circ call that this is invoking. */

                     search = save_cptr->next;
                     while (search) {
                        parse_block *subsidiary_ptr = search->subsidiary_root;
                        if (subsidiary_ptr &&
                            subsidiary_ptr->call_to_print &&
                            (subsidiary_ptr->call_to_print->the_defn.callflags1 &
                             CFLAG1_BASE_CIRC_CALL)) {
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_CIRC);
                           goto did_circcer;
                        }
                        search = search->next;
                     }

                     /* We didn't find the circcer.  It must not have been entered into the parse tree.
                           See if we can get it from the "circcer" field. */

                     if (save_cptr->options.circcer > 0)
                        writestuff(circcer_calls[(save_cptr->options.circcer)-1]->menu_name);
                     else
                        writestuff("NO CIRCCER???");

                  did_circcer:

                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case 'h':                   /* Need to plug in a direction. */
                     write_blank_if_needed();
                     writestuff(direction_names[idirjunk]);
                     if (np[0] && np[0] != ' ' && np[0] != ']')
                        writestuff(" ");
                     break;
                  case '9': case 'a': case 'b': case 'B': case 'D': case 'u':
                     /* Need to plug in a number. */
                     write_blank_if_needed();
                     write_nice_number(savec, number_list & 0xF);
                     number_list >>= 4;    /* Get ready for next number. */
                     break;
                  case 'e':
                     if (use_left_name) {
                        while (*np != '@') np++;
                        if (writechar_block.lastchar == ']') writestuff(" ");
                        writestuff("left");
                        np += 2;
                     }
                     break;
                  case 'j':
                     if (!use_cross_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'C':
                     if (use_cross_name) {
                        write_blank_if_needed();
                        writestuff("cross");
                     }
                     break;
                  case 'S':                   /* Look for star turn replacement. */
                     if (save_cptr->options.star_turn_option < 0) {
                        writestuff(", don't turn the star");
                     }
                     else if (save_cptr->options.star_turn_option != 0) {
                        writestuff(", turn the star ");
                        write_nice_number('b', save_cptr->options.star_turn_option & 0xF);
                     }
                     break;
                  case 'Q':
                     if (!use_grand_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'G':
                     if (use_grand_name) {
                        if (writechar_block.lastchar != ' ' &&
                            writechar_block.lastchar != '[') writechar(' ');
                        writestuff("grand");
                     }
                     break;
                  case 'J':
                     if (!use_magic_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'M':
                     if (use_magic_name) {
                        if (writechar_block.lastchar != ' ' &&
                            writechar_block.lastchar != '[') writechar(' ');
                        writestuff("magic");
                     }
                     break;
                  case 'E':
                     if (!use_intlk_name) {
                        while (*np != '@') np++;
                        np += 2;
                     }
                     break;
                  case 'I':
                     if (use_intlk_name) {
                        if (writechar_block.lastchar == 'a' &&
                            writechar_block.lastlastchar == ' ')
                           writestuff("n ");
                        else if (writechar_block.lastchar != ' ' &&
                                 writechar_block.lastchar != '[')
                           writechar(' ');
                        writestuff("interlocked");
                     }
                     break;
                  case 'l': case 'L': case 'R': case 'F': case '8': case 'o':
                     // Just skip these -- they end stuff that we could have
                     // elided but didn't.
                     break;
                  case 'n': case 'p': case 'r': case 'm': case 't':
                     if (subst2_in_use) {
                        if (savec == 'p' || savec == 'r') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
                     else {
                        if (savec == 'n') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
   
                     if (savec != 'p' && savec != 'n') {
                        if (pending_subst2) {
                           write_blank_if_needed();
                           writestuff("[");
                           print_recurse(sub2_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                           pending_subst2 = false;
                        }
                        else if (savec == 'm' && !global_leave_missing_calls_blank) {
                           write_blank_if_needed();
                           writestuff("[???]");
                        }
                     }
         
                     break;
                  case 'O':
                     if (print_recurse_arg & PRINT_RECURSE_CIRC) {
                        while (*np != '@') np++;
                        np += 2;
                     }

                     break;
                  default:
                     if (subst1_in_use) {
                        if (savec == '2' || savec == '4') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
                     else {
                        if (savec == '7') {
                           while (*np != '@') np++;
                           np += 2;
                        }
                     }
         
                     if (savec != '4' && savec != '7') {
                        if (pending_subst1) {
                           write_blank_if_needed();
                           writestuff("[");
                           print_recurse(sub1_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                           if (savec == 'T') writestuff(" er's");
                           pending_subst1 = false;
                        }
                        else if (savec == '0' && !global_leave_missing_calls_blank) {
                           write_blank_if_needed();
                           writestuff("[???]");
                        }
                     }
         
                     break;
                  }
               }
               else {
                  if (writechar_block.lastchar == ']' && c != ' ' && c != ']')
                     writestuff(" ");

                  if ((writechar_block.lastchar != ' ' && writechar_block.lastchar != '[') || c != ' ') writechar(c);
               }
            }

            if (writechar_block.lastchar == ']' && *np && *np != ' ' && *np != ']')
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

            for (search = save_cptr->next ; search ; search = search->next) {
               const call_with_name *cc;
               parse_block *subsidiary_ptr = search->subsidiary_root;

               /* If we have a subsidiary_ptr, handle the replacement that is indicated.
                  BUT:  if the call shown in the subsidiary_ptr is a base tag call, don't
                  do anything -- such substitutions were already taken care of.
                     BUT:  only check if there is actually a call there. */

               if (!subsidiary_ptr) continue;
               cc = subsidiary_ptr->call_to_print;

               if (!cc ||    /* If no call pointer, it isn't a tag base call. */
                   (!(cc->the_defn.callflags1 & CFLAG1_BASE_TAG_CALL_MASK) &&
                    (!(cc->the_defn.callflags1 & CFLAG1_BASE_CIRC_CALL) ||
                     search->call_to_print != base_calls[base_call_circcer]))) {
                  call_with_name *replaced_call = search->call_to_print;

                  /* Need to check for case of replacing one star turn with another. */

                  if ((first_replace == 0) &&
                      (replaced_call->the_defn.callflags1 & CFLAG1_IS_STAR_CALL) &&
                      ((subsidiary_ptr->concept->kind == marker_end_of_list) ||
                       subsidiary_ptr->concept->kind == concept_another_call_next_mod) &&
                      cc &&
                      ((cc->the_defn.callflags1 & CFLAG1_IS_STAR_CALL) ||
                       cc->the_defn.schema == schema_nothing)) {
                     first_replace++;

                     if (cc->the_defn.schema == schema_nothing)
                        writestuff(", don't turn the star");
                     else {
                        writestuff(", ");
                        print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                     }
                  }
                  else {
                     switch (search->replacement_key) {
                     case DFM1_CALL_MOD_ANYCALL/DFM1_CALL_MOD_BIT:
                     case DFM1_CALL_MOD_MAND_ANYCALL/DFM1_CALL_MOD_BIT:
                     case DFM1_CALL_MOD_ALLOW_PLAIN_MOD/DFM1_CALL_MOD_BIT:
                        // This is a natural replacement.
                        // It may already have been taken care of.
                        if (pending_subst1 ||
                            search->replacement_key ==
                            DFM1_CALL_MOD_ALLOW_PLAIN_MOD/DFM1_CALL_MOD_BIT) {
                           write_blank_if_needed();
                           if (search->replacement_key ==
                               DFM1_CALL_MOD_ALLOW_PLAIN_MOD/DFM1_CALL_MOD_BIT)
                              writestuff("but [");
                           else
                              writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     case DFM1_CALL_MOD_OR_SECONDARY/DFM1_CALL_MOD_BIT:
                     case DFM1_CALL_MOD_MAND_SECONDARY/DFM1_CALL_MOD_BIT:
                        // This is a secondary replacement.
                        // It may already have been taken care of.
                        if (pending_subst2) {
                           write_blank_if_needed();
                           writestuff("[modification: ");
                           print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                           writestuff("]");
                        }
                        break;
                     default:
                        /* This is a forced replacement. */
                        write_blank_if_needed();
                        first_replace++;
                        if (first_replace == 1)
                           writestuff("BUT REPLACE ");
                        else
                           writestuff("AND REPLACE ");

                        /* Star turn calls can have funny names like "nobox". */

                        writestuff_with_decorations(
                           &search->options,
                           (replaced_call->the_defn.callflags1 & CFLAG1_IS_STAR_CALL) ?
                           "turn the star @b" : replaced_call->name);

                        writestuff(" WITH [");
                        print_recurse(subsidiary_ptr, PRINT_RECURSE_STAR);
                        writestuff("]");
                        break;
                     }
                  }
               }
            }
         }

         break;
      }
   }

   if (deferred_concept) {
      if (deferred_concept_paren & 1) writestuff(/*(*/")");
      writestuff(" ");

      if (deferred_concept->concept->kind == concept_n_times_const &&
          deferred_concept->concept->value.arg2 == 3)
         writestuff("3 TIMES");
      else
         writestuff_with_decorations(&deferred_concept->options, deferred_concept->concept->name);
      if (deferred_concept_paren & 2) writestuff(/*(*/")");
   }

   return;
}

SDLIB_API void clear_screen(void)
{
   written_history_items = -1;
   text_line_count = 0;
   gg->reduce_line_count(0);
   open_text_line();
}

SDLIB_API void write_header_stuff(long_boolean with_ui_version, uint32 act_phan_flags)
{
   if (!diagnostic_mode) {
      /* log creation version info */
      if (with_ui_version) {     /* This is the "pretty" form that we display while running. */
         writestuff("Sd ");
         writestuff(sd_version_string());
         writestuff(" : db");
         writestuff(database_version);
         writestuff(" : ui");
         writestuff(gg->version_string());
      }
      else {                     /* This is the "compact" form that goes into the file. */
         writestuff("Sd");
         writestuff(sd_version_string());
         writestuff(":db");
         writestuff(database_version);
      }
   }

   if (act_phan_flags & RESULTFLAG__ACTIVE_PHANTOMS_ON) {
      if (act_phan_flags & RESULTFLAG__ACTIVE_PHANTOMS_OFF)
         writestuff(" (AP-)");
      else
         writestuff(" (AP)");
   }

   writestuff("     ");

   /* log level info */
   writestuff(getout_strings[calling_level]);

   if (glob_call_list_mode == call_list_mode_abridging)
      writestuff(" (abridged)");
}


static char current_line[MAX_TEXT_LINE_LENGTH];

extern void writechar(char src)
{
   // Don't write two consecutive commas.
   if (src == ',' && src == writechar_block.lastchar) return;

   writechar_block.lastlastchar = writechar_block.lastchar;

   *writechar_block.destcurr = (writechar_block.lastchar = src);
   if (src == ' ' && writechar_block.destcurr != current_line)
      writechar_block.lastblank = writechar_block.destcurr;

   /* If drawing a picture, don't do automatic line breaks. */

   if (writechar_block.destcurr < &current_line[MAX_PRINT_LENGTH] || writechar_block.usurping_writechar || ui_options.drawing_picture)
      writechar_block.destcurr++;
   else {
      /* Line overflow.  Try to write everything up to the last
         blank, then fill next line with everything since that blank. */

      char save_buffer[MAX_TEXT_LINE_LENGTH];
      char *q = save_buffer;

      if (writechar_block.lastblank) {
         char *p = writechar_block.lastblank+1;
         while (p <= writechar_block.destcurr) *q++ = *p++;
         writechar_block.destcurr = writechar_block.lastblank;
      }
      else {
         /* Must break a word. */
         *q++ = *writechar_block.destcurr;
      }

      *q = '\0';

      newline();            /* End the current buffer and write it out. */
      writestuff("   ");    /* Make a new line, copying saved stuff into it. */
      writestuff(save_buffer);
   }
}

SDLIB_API void newline(void)
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
   while (writechar_block.destcurr != current_line && writechar_block.destcurr[-1] == ' ')
      writechar_block.destcurr--;

   *writechar_block.destcurr = '\0';

   /* There will be no special "5" or "6" characters in pictures (ui_options.drawing_picture&1) if:

      Enable_file_writing is on (we don't write the special stuff to a file,
         of course, and we don't even write it to the transcript when showing the
         final card)

      Use_escapes_for_drawing_people is 0 or 1 (so we don't do it for Sdtty under
         DJGPP or GCC (0), or for Sdtty under Windows (1))

      No_graphics != 0 (so we only do it if showing full checkers in Sd) */


   if (enable_file_writing)
      write_file(current_line);

   text_line_count++;
   gg->add_new_line(current_line,
      enable_file_writing ? 0 : (ui_options.drawing_picture | (ui_options.squeeze_this_newline << 1)));
   open_text_line();
}





extern void open_text_line(void)
{
   writechar_block.destcurr = current_line;
   writechar_block.lastchar = ' ';
   writechar_block.lastlastchar = ' ';
   writechar_block.lastblank = (char *) 0;
}


SDLIB_API void doublespace_file(void)
{
   write_file("");
}


SDLIB_API void writestuff(const char *s)
{
   while (*s) writechar(*s++);
}



static parse_block *parse_active_list = (parse_block *) 0;
static parse_block *parse_inactive_list = (parse_block *) 0;

extern parse_block *mark_parse_blocks(void)
{
   return parse_active_list;
}


extern void release_parse_blocks_to_mark(parse_block *mark_point)
{
   while (parse_active_list && parse_active_list != mark_point) {
      parse_block *item = parse_active_list;

      parse_active_list = item->gc_ptr;
      item->gc_ptr = parse_inactive_list;
      parse_inactive_list = item;

      /* Clear pointers so we will notice if it gets erroneously re-used. */
      item->concept = &mark_end_of_list;
      item->call = base_calls[1];
      item->call_to_print = item->call;
      item->subsidiary_root = (parse_block *) 0;
      item->next = (parse_block *) 0;
   }
}


extern parse_block *copy_parse_tree(parse_block *original_tree)
{
   parse_block *new_item, *new_root;

   if (!original_tree) return NULL;

   new_item = get_parse_block();
   new_root = new_item;

   for (;;) {
      new_item->concept = original_tree->concept;
      new_item->call = original_tree->call;
      new_item->call_to_print = original_tree->call_to_print;
      new_item->options = original_tree->options;
      new_item->replacement_key = original_tree->replacement_key;
      new_item->no_check_call_level = original_tree->no_check_call_level;

      if (original_tree->subsidiary_root)
         new_item->subsidiary_root = copy_parse_tree(original_tree->subsidiary_root);

      if (!original_tree->next) break;

      new_item->next = get_parse_block();
      new_item = new_item->next;
      original_tree = original_tree->next;
   }

   return new_root;
}


parse_block *get_parse_block(void)
{
   parse_block *item;

   if (parse_inactive_list) {
      item = parse_inactive_list;
      parse_inactive_list = item->gc_ptr;
   }
   else {
      item = (parse_block *) get_mem(sizeof(parse_block));
   }

   item->gc_ptr = parse_active_list;
   parse_active_list = item;

   item->concept = (concept_descriptor *) 0;
   item->call = (call_with_name *) 0;
   item->call_to_print = (call_with_name *) 0;
   item->options = null_options;
   item->replacement_key = 0;
   item->no_check_call_level = 0;
   item->subsidiary_root = (parse_block *) 0;
   item->next = (parse_block *) 0;

   return item;
}


/* Stuff for saving parse state while we resolve. */

static parse_state_type saved_parse_state;
static parse_block *saved_command_root;

extern void reset_parse_tree(parse_block *original_tree, parse_block *final_head)
{
   parse_block *new_item, *old_item;

   new_item = final_head;
   old_item = original_tree;
   for (;;) {
      new_item->concept = old_item->concept;
      new_item->call = old_item->call;
      new_item->call_to_print = old_item->call_to_print;
      new_item->options = old_item->options;

      /* Chop off branches that don't belong. */

      if (!old_item->subsidiary_root)
         new_item->subsidiary_root = (parse_block *) 0;
      else
         reset_parse_tree(old_item->subsidiary_root, new_item->subsidiary_root);

      if (!old_item->next) {
         new_item->next = 0;
         break;
      }

      new_item = new_item->next;
      old_item = old_item->next;
   }
}


/* Save the entire parse stack, and make a copy of the dynamic blocks that
   comprise the parse state. */
extern void save_parse_state(void)
{
   saved_parse_state = parse_state;
   saved_command_root = copy_parse_tree(configuration::next_config().command_root);
}


/* Restore the parse state.  We write directly over the original dynamic blocks
   of the current parse state (making use of the fact that the alterations that
   could have happened will add to the tree but never delete anything.)  This way,
   after we have saved and restored things, they are all in their original,
   locations, so that the pointers in the parse stack will still be valid. */
extern long_boolean restore_parse_state(void)
{
   parse_state = saved_parse_state;

   if (saved_command_root)
      reset_parse_tree(saved_command_root, configuration::next_config().command_root);
   else
      configuration::next_config().command_root = 0;

   return FALSE;
}


SDLIB_API void string_copy(char **dest, Cstring src)
{
   Cstring f = src;
   char *t = *dest;

   while ((*t++ = *f++));
   *dest = t-1;
}



/* Clear the screen and display the initial part of the history.
   This attempts to re-use material already displayed on the screen.
   The "num_pics" argument tells how many of the last history items
   are to have pictures forced, so we can tell exactly what items
   have pictures. */
void display_initial_history(int upper_limit, int num_pics)
{
   int j, startpoint;

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
      int compilerbug = ((int) ((unsigned int) (written_history_nopic-j)) ^
                 ((unsigned int) (upper_limit-num_pics-j)));
      if (compilerbug < 0 && ~configuration::history[j].draw_pic) {
         written_history_items = j-1;
         break;
      }
   }

   if (written_history_items > 0) {
      /* We win.  Back up the text line count to the right place, and rewrite the rest. */

      text_line_count = configuration::history[written_history_items].text_line;
      gg->reduce_line_count(text_line_count);
      open_text_line();
      startpoint = written_history_items+1;
   }
   else {
      /* We lose, there is nothing we can use. */
      clear_screen();
      write_header_stuff(TRUE, 0);
      newline();
      newline();
      startpoint = 1;
   }

   for (j=startpoint; j<=upper_limit-num_pics; j++)
      write_history_line(j, false, false, file_write_no);

   // Now write stuff with forced pictures.

   for (j=upper_limit-num_pics+1; j<=upper_limit; j++) {
      if (j >= startpoint) write_history_line(j, true, false, file_write_no);
   }

   written_history_items = upper_limit;   /* This stuff is now safe. */
   written_history_nopic = written_history_items-num_pics;
}



extern void initialize_parse(void)
{
   parse_state.concept_write_base = &configuration::next_config().command_root;
   parse_state.concept_write_ptr = parse_state.concept_write_base;
   *(parse_state.concept_write_ptr) = (parse_block *) 0;
   parse_state.parse_stack_index = 0;
   parse_state.base_call_list_to_use = find_proper_call_list(&configuration::current_config().state);
   parse_state.call_list_to_use = parse_state.base_call_list_to_use;
   configuration::next_config().init_centersp_specific();
   configuration::init_warnings();
   configuration::next_config().draw_pic = FALSE;

   if (written_history_items > configuration::history_ptr)
      written_history_items = configuration::history_ptr;

   parse_state.specialprompt[0] = '\0';
   parse_state.topcallflags1 = 0;
}




static void do_change_outfile(long_boolean signal)
{
   char newfile_string[MAX_FILENAME_LENGTH];

   if (gg->do_outfile_popup(newfile_string)) {
      if (newfile_string[0]) {
         char confirm_message[MAX_FILENAME_LENGTH+25];
         char *final_message;

         if (install_outfile_string(newfile_string)) {
            (void) strncpy(confirm_message, "Output file changed to \"", 25);
            (void) strncat(confirm_message, outfile_string, MAX_FILENAME_LENGTH);
            (void) strncat(confirm_message, "\"", 2);
            final_message = confirm_message;
         }
         else
            final_message = "No write access to that file, no action taken.";

         if (signal) {
            specialfail(final_message);
         }
         else {
            writestuff(final_message);
            newline();
         }
      }
   }
}



// Returns TRUE if it successfully backed up one parse block.
static long_boolean backup_one_item(void)
{

   // User wants to undo a call.  The concept parse list is not set up
   // for easy backup, so we search forward from the beginning.

   parse_block **this_ptr = parse_state.concept_write_base;
   if (!this_ptr || !*this_ptr) return FALSE;

   if ((configuration::history_ptr == 1) && configuration::history[1].get_startinfo_specific()->into_the_middle)
      this_ptr = &((*this_ptr)->next);

   for (;;) {
      parse_block **last_ptr;

      if (!*this_ptr) break;
      last_ptr = this_ptr;
      this_ptr = &((*this_ptr)->next);

      if (this_ptr == parse_state.concept_write_ptr) {
         parse_state.concept_write_ptr = last_ptr;

         /* See whether we need to destroy a frame in the parse stack. */
         if (parse_state.parse_stack_index != 0 &&
               parse_state.parse_stack[parse_state.parse_stack_index-1].concept_write_save_ptr == last_ptr)
            parse_state.parse_stack_index--;

         *last_ptr = (parse_block *) 0;
         return TRUE;
      }

      if ((*last_ptr)->concept->kind <= marker_end_of_list) break;
   }

   /* We did not find our place. */

   return FALSE;
}


// Returns TRUE if sequence was written.
static long_boolean write_sequence_to_file(void) THROW_DECL
{
   int getout_ind;
   char date[MAX_TEXT_LINE_LENGTH];
   char second_header[MAX_TEXT_LINE_LENGTH];
   char seqstring[20];
   int j;

   /* Put up the getout popup to see if the user wants to enter a header string. */

   getout_ind = gg->do_getout_popup(second_header);

   /* Some user interfaces (those with icons) may have an icon to abort the
      sequence, rather than just decline the comment.  Such an action comes
      back as "POPUP_DECLINE". */

   if (getout_ind == POPUP_DECLINE)
      return FALSE;    /* User didn't want to end this sequence after all. */
   else if (getout_ind != POPUP_ACCEPT_WITH_STRING) second_header[0] = '\0';

   /* Open the file and write it. */

   clear_screen();
   open_file();
   enable_file_writing = TRUE;
   doublespace_file();
   get_date(date);
   writestuff(date);
   writestuff("     ");
   write_header_stuff(FALSE, configuration::current_config().state.result_flags);
   newline();

   if (!configuration::sequence_is_resolved()) {
      writestuff("             NOT RESOLVED");
      newline();
   }

   if (singing_call_mode != 0) {
      writestuff(
         (singing_call_mode == 2) ?
         "             Singing call reverse progression" :
         "             Singing call progression");
      newline();
   }

   /* Write header comment, if it exists. */

   if (header_comment[0]) {
      writestuff("             ");
      writestuff(header_comment);
   }

   if (sequence_number >= 0) {
      (void) sprintf(seqstring, "%d", sequence_number);
      writestuff("   ");
      writestuff(seqstring);
   }

   /* Write secondary header comment, if it exists. */

   if (second_header[0]) {
      writestuff("       ");
      writestuff(second_header);
   }

   if (header_comment[0] || second_header[0] || sequence_number >= 0) newline();

   newline();

   if (sequence_number >= 0) sequence_number++;

   for (j=configuration::whole_sequence_low_lim; j<=configuration::history_ptr; j++)
      write_history_line(j, false, false, file_write_double);

   // Echo the concepts entered so far.

   if (parse_state.concept_write_ptr != &configuration::next_config().command_root) {
      write_history_line(configuration::history_ptr+1, false, false, file_write_double);
   }

   if (configuration::sequence_is_resolved())
      write_resolve_text(TRUE);

   newline();
   enable_file_writing = FALSE;
   newline();

   close_file();     /* This will signal a "specialfail" if a file error occurs. */

   writestuff("Sequence");

   if (sequence_number >= 0) {
      writestuff(" ");
      writestuff(seqstring);
   }

   writestuff(" written to \"");
   writestuff(outfile_string);
   writestuff("\".");
   newline();

   wrote_a_sequence = TRUE;
   global_age++;
   return TRUE;
}


static uint32 id_fixer_array[16] = {
   07777525252, 07777454545, 07777313131, 07777262626,
   07777522525, 07777453232, 07777314646, 07777265151,
   07777255225, 07777324532, 07777463146, 07777512651,
   07777252552, 07777323245, 07777464631, 07777515126};
  

static selector_kind translate_selector_permutation1(uint32 x)
{
   switch (x & 077) {
   case 01: return selector_sidecorners;
   case 02: return selector_headcorners;
   case 04: return selector_girls;
   case 010: return selector_boys;
   case 020: return selector_sides;
   case 040: return selector_heads;
   default: return selector_uninitialized;
   }
}


static selector_kind translate_selector_permutation2(uint32 x)
{
   switch (x & 07) {
   case 04: return selector_headboys;
   case 05: return selector_headgirls;
   case 06: return selector_sideboys;
   case 07: return selector_sidegirls;
   default: return selector_uninitialized;
   }
}


/* Returned value with "2" bit on means error occurred and counld not translate.
   Selectors are messed up.  Should only occur if in unsymmetrical formation
   in which it can't figure out what is going on.
   Or if we get "end boys" or something like that, that we can't handle yet.
   Otherwise, "1" bit says at
   least one selector changed.  Zero means nothing changed. */

static uint32 translate_selector_fields(parse_block *xx, uint32 mask)
{
   selector_kind z;
   uint32 retval = 0;

   for ( ; xx ; xx=xx->next) {
      switch (xx->options.who) {
      case selector_heads:
         z = translate_selector_permutation1(mask >> 13);
         break;
      case selector_sides:
         z = translate_selector_permutation1(mask >> 13);
         z = selector_list[z].opposite;
         break;
      case selector_boys:
         z = translate_selector_permutation1(mask >> 7);
         break;
      case selector_girls:
         z = translate_selector_permutation1(mask >> 7);
         z = selector_list[z].opposite;
         break;
      case selector_headcorners:
         z = translate_selector_permutation1(mask >> 1);
         break;
      case selector_sidecorners:
         z = translate_selector_permutation1(mask >> 1);
         z = selector_list[z].opposite;
         break;

      case selector_end_boys:
         if (((mask >> 7) & 077) == 010) z = selector_end_boys;
         else if (((mask >> 7) & 077) == 04) z = selector_end_girls;
         else z = selector_uninitialized;
         break;
      case selector_end_girls:
         if (((mask >> 7) & 077) == 010) z = selector_end_girls;
         else if (((mask >> 7) & 077) == 04) z = selector_end_boys;
         else z = selector_uninitialized;
         break;
      case selector_center_boys:
         if (((mask >> 7) & 077) == 010) z = selector_center_boys;
         else if (((mask >> 7) & 077) == 04) z = selector_center_girls;
         else z = selector_uninitialized;
         break;
      case selector_center_girls:
         if (((mask >> 7) & 077) == 010) z = selector_center_girls;
         else if (((mask >> 7) & 077) == 04) z = selector_center_boys;
         else z = selector_uninitialized;
         break;

      case selector_headliners:
         if (mask & 1) z = selector_sideliners;
         break;
      case selector_sideliners:
         if (mask & 1) z = selector_headliners;
         break;
      case selector_headboys:
         z = translate_selector_permutation2(mask >> 19);
         break;
      case selector_headgirls:
         z = translate_selector_permutation2(mask >> 22);
         break;
      case selector_sideboys:
         z = translate_selector_permutation2(mask >> 25);
         break;
      case selector_sidegirls:
         z = translate_selector_permutation2(mask >> 28);
         break;
      default: goto nofix;
      }

      if (z == selector_uninitialized) retval = 2;   /* Raise error. */
      if (z != xx->options.who) retval |= 1;         /* Note that we changed something. */
      xx->options.who = z;

   nofix:

      retval |= translate_selector_fields(xx->subsidiary_root, mask);
   }

   return retval;
}



void run_program()
{
   int i;

   global_age = 1;
   global_error_flag = (error_flag_type) 0;
   interactivity = interactivity_normal;
   clear_screen();

   if (!diagnostic_mode) {
      writestuff("SD -- square dance caller's helper.");
      newline();
      newline();
      writestuff("Copyright (c) 1990-2002 William B. Ackerman");
      newline();
      writestuff("   and Stephen Gildea.");
      newline();
      writestuff("Copyright (c) 1992-1993 Alan Snyder");
      newline();
      writestuff("Copyright (c) 1995, Robert E. Cays");
      newline();
      writestuff("Copyright (c) 1996, Charles Petzold");
      newline();
      writestuff("Copyright (c) 2001-2002  Chris \"Fen\" Tamanaha");
      newline();
      writestuff("SD comes with ABSOLUTELY NO WARRANTY;");
      newline();
      writestuff("   for details see the license.");
      newline();
      writestuff("This is free software, and you are");
      newline();
      writestuff("   welcome to redistribute it.");
      newline();
      newline();
      writestuff("At any time that you don't know what you can type,");
      newline();
      writestuff("type a question mark (?).  The program will show you all");
      newline();
      writestuff("legal choices.");
      newline();
      newline();

      FILE *session = fopen(SESSION_FILENAME, "r");
      if (session) {
         (void) fclose(session);
      }
      else {
         writestuff("You do not have a session control file.  If you want to create one, give the command \"initialize session file\".");
         newline();
         newline();
      }
   }

   // If anything in the "try" block throws an exception, we will get here
   // with error_flag nonzero.

 got_an_exception:

   // Enter, or re-enter, the big try block.

   try {
      if (global_error_flag) {
         // The call we were trying to do has failed.  Abort it and display the error message.
   
         if (interactivity == interactivity_database_init ||
             interactivity == interactivity_verify)
            gg->fatal_error_exit(1, "Unknown error context", error_message1);

         // If this is a real call execution error, save the call that caused it.

         if (global_error_flag < error_flag_wrong_command) {
            configuration::history[0] = configuration::next_config();     // So failing call will get printed.
            // But copy the parse tree, since we are going to clip it.
            configuration::history[0].command_root = copy_parse_tree(configuration::history[0].command_root);
            // But without any warnings we may have collected.
            configuration::history[0].init_warnings_specific();
         }
         if (global_error_flag == error_flag_wrong_command) {
            /* Special signal -- user clicked on special thing while trying to get subcall. */
            if ((global_reply == ui_command_select) &&
                ((uims_menu_index == command_quit) ||
                 (uims_menu_index == command_undo) ||
                 (uims_menu_index == command_cut_to_clipboard) ||
                 (uims_menu_index == command_delete_entire_clipboard) ||
                 (uims_menu_index == command_delete_one_call_from_clipboard) ||
                 (uims_menu_index == command_paste_one_call) ||
                 (uims_menu_index == command_paste_all_calls) ||
                 (uims_menu_index == command_erase) ||
                 (uims_menu_index == command_abort)))
               reply_pending = TRUE;
            goto start_with_pending_reply;
         }
   
         /* Try to remove the call from the current parse tree, but leave everything else
            in place.  This will fail if the parse tree, or our place on it, is too
            complicated.  Also, we do not do it if in diagnostic mode, or if the user
            did not specify "retain_after_error", or if the special "heads into the middle and ..."
            operation is in place. */

         if (!diagnostic_mode && 
             retain_after_error &&
             ((configuration::history_ptr != 1) || !configuration::history[1].get_startinfo_specific()->into_the_middle) &&
             backup_one_item()) {
            reply_pending = FALSE;
            // Take out warnings that arose from the failed call,
            // since we aren't going to do that call.
            configuration::init_warnings();
            goto simple_restart;
         }
         goto start_cycle;      /* Failed, reinitialize the whole line. */
      }

   show_banner:

      writestuff("Version ");
      write_header_stuff(TRUE, 0);
      newline();
      writestuff("Output file is \"");
      writestuff(outfile_string);
      writestuff("\"");
      newline();

      if (need_new_header_comment) {
         (void) gg->do_header_popup(header_comment);
         need_new_header_comment = FALSE;
      }
   
   new_sequence:
   
      // Here to start a fresh sequence.  If first time, or if we got here
      // by clicking on "abort", the screen has been cleared.  Otherwise,
      // it shows the last sequence that we wrote.

      // Replace all the parse blocks left from the last sequence.
      // But if we have stuff in the clipboard, we save everything.

      if (clipboard_size == 0) release_parse_blocks_to_mark((parse_block *) 0);
   
      /* Update the console window title. */

      {
         char numstuff[50];
         char title[MAX_TEXT_LINE_LENGTH];

         if (sequence_number >= 0)
            (void) sprintf(numstuff, " (%d:%d)", starting_sequence_number, sequence_number);
         else
            numstuff[0] = '\0';

         if (header_comment[0])
            (void) sprintf(title, "%s  %s%s",
                           &old_filename_strings[calling_level][1], header_comment, numstuff);
         else
            (void) sprintf(title, "%s%s",
                           &old_filename_strings[calling_level][1], numstuff);

         gg->set_window_title(title);
      }

      /* Query for the starting setup. */

      global_reply = gg->get_startup_command();

      if (global_reply == ui_command_select && uims_menu_index == command_quit) goto normal_exit;
      if (global_reply != ui_start_select) goto normal_exit;           /* Huh? */

      switch (uims_menu_index) {
      case start_select_toggle_conc:
         allowing_all_concepts = !allowing_all_concepts;
         goto new_sequence;
      case start_select_toggle_singlespace:
         ui_options.singlespace_mode = !ui_options.singlespace_mode;
         goto new_sequence;
      case start_select_toggle_minigrand:
         allowing_minigrand = !allowing_minigrand;
         goto new_sequence;
      case start_select_toggle_act:
         using_active_phantoms = !using_active_phantoms;
         goto new_sequence;
      case start_select_toggle_retain:
         retain_after_error = !retain_after_error;
         goto new_sequence;
      case start_select_toggle_nowarn_mode:
         ui_options.nowarn_mode = !ui_options.nowarn_mode;
         goto new_sequence;
      case start_select_toggle_singleclick_mode:
         ui_options.accept_single_click = !ui_options.accept_single_click;
         goto new_sequence;
      case start_select_toggle_singer:
         if (singing_call_mode != 0)
            singing_call_mode = 0;    /* Turn it off. */
         else
            singing_call_mode = 1;    /* 1 for forward progression, 2 for backward. */
         goto new_sequence;
      case start_select_toggle_singer_backward:
         if (singing_call_mode != 0)
            singing_call_mode = 0;    /* Turn it off. */
         else
            singing_call_mode = 2;
         goto new_sequence;
      case start_select_select_print_font:
         if (!gg->choose_font()) {
            writestuff("Printing is not supported in this program.");
            newline();
         }
         goto new_sequence;
      case start_select_print_current:
         if (!gg->print_this()) {
            writestuff("Printing is not supported in this program.");
            newline();
         }
         goto new_sequence;
      case start_select_print_any:
         if (!gg->print_any()) {
            writestuff("Printing is not supported in this program.");
            newline();
         }
         goto new_sequence;
      case start_select_init_session_file:
         {
            Cstring *q;
            FILE *session = fopen(SESSION_FILENAME, "r");

            if (session) {
               (void) fclose(session);
               if (gg->do_session_init_popup() != POPUP_ACCEPT) {
                  writestuff("No action has been taken.");
                  newline();
                  goto new_sequence;
               }
               else if (!rename(SESSION_FILENAME, SESSION2_FILENAME)) {
                  writestuff("File '" SESSION_FILENAME "' has been saved as '"
                             SESSION2_FILENAME "'.");
                  newline();
               }
            }

            if (!(session = fopen(SESSION_FILENAME, "w"))) {
               writestuff("Failed to create '" SESSION_FILENAME "'.");
               newline();
               goto new_sequence;
            }

            for (q = sessions_init_table ; *q ; q++) {
               if (fputs(*q, session) == EOF) goto copy_failed;
               if (fputs("\n", session) == EOF) goto copy_failed;
            }

            for (q = concept_key_table ; *q ; q++) {
               if (fputs(*q, session) == EOF) goto copy_failed;
               if (fputs("\n", session) == EOF) goto copy_failed;
            }

            if (fputs("\n", session) == EOF) goto copy_failed;
            (void) fclose(session);
            writestuff("The file has been initialized, and will take effect the next time the program is started.");
            newline();
            writestuff("Exit and restart the program if you want to use it now.");
            newline();
            goto new_sequence;

         copy_failed:

            writestuff("Failed to create '" SESSION_FILENAME "'");
            newline();
            (void) fclose(session);
            goto new_sequence;
         }
      case start_select_change_outfile:
         do_change_outfile(FALSE);
         goto new_sequence;
      case start_select_change_header_comment:
         (void) gg->do_header_popup(header_comment);
         need_new_header_comment = FALSE;
         goto new_sequence;
      case start_select_exit:
         goto normal_exit;
      }
   
      // We now know that uims_menu_index is in the range 1 to 5, that is,
      // start_select_h1p2p ... start_select_as_they_are.  We will put
      // that into the startinfo stuff in the history.

      configuration::initialize_history(uims_menu_index);   // Clear the position history.
      configuration::history[1].init_warnings_specific();
      configuration::history[1].init_resolve();
      // Put the people into their starting position.
      configuration::history[1].state = configuration::history[1].get_startinfo_specific()->the_setup;

      written_history_items = -1;

      global_error_flag = (error_flag_type) 0;
   
      /* Come here to read a bunch of concepts and a call and add an item to the history. */
   
   start_cycle:

      reply_pending = FALSE;

   start_with_pending_reply:

      allowing_modifications = 0;

      /* See if we need to increase the size of the history array.
         We must have history_allocation at least equal to history_ptr+2,
         so that history items [0..history_ptr+1] will exist.
         We also need to allow for MAX_RESOLVE_SIZE extra items, so that the
         resolver can work.  Why don't we just increase the allocation
         at the start of the resolver if we are too close?  We tried that once.
         The resolver uses the current parse state, so we can do "TANDEM <resolve>".
         This means that things like "parse_state.concept_write_base", which point
         into the history array, must remain valid.  So the resolver can't reallocate
         the history array.  There is only one place where it is safe to reallocate,
         and that is right here.  Note that we are about to call "initialize_parse",
         which destroys any lingering pointers into the history array. */

      if (history_allocation < configuration::history_ptr+MAX_RESOLVE_SIZE+2) {
         configuration * t;
         history_allocation <<= 1;
         t = (configuration *)
            get_more_mem_gracefully(configuration::history, history_allocation * sizeof(configuration));
         if (!t) {
            /* Couldn't get memory; we are in serious trouble. */
            history_allocation >>= 1;
            /* Bring history_ptr down to safe size.  This will have the effect of
               throwing away the last call, or part or all of the last resolve. */
            configuration::history_ptr = history_allocation-MAX_RESOLVE_SIZE-2;
            specialfail("Not enough memory!");
         }
         configuration::history = t;
      }

      initialize_parse();

      /* Check for first call given to heads or sides only. */
   
      if ((configuration::history_ptr == 1) && configuration::history[1].get_startinfo_specific()->into_the_middle)
         deposit_concept(&centers_concept);
   
      /* Come here to get a concept or call or whatever from the user. */
   
      /* Display the menu and make a choice!!!! */
   
   simple_restart:

      if ((!reply_pending) && (!query_for_call())) {
         // User specified a call (and perhaps concepts too).

         // The call to toplevelmove may make a call to "fail", which will get caught
         // by the cleanup handler above, reset history_ptr, and go to start_cycle
         // with the error message displayed. */

         toplevelmove();
         finish_toplevelmove();
         configuration::history_ptr++;         // Call successfully completed; save it.
         goto start_cycle;
      }
   
      /* If get here, query_for_call exitted without completing its parse, because the operator
         selected something like "quit", "undo", or "resolve", or because we have such a command
         already pending. */

      reply_pending = FALSE;

      if (global_reply == ui_command_select) {
         switch ((command_kind) uims_menu_index) {
         case command_quit:
            if (gg->do_abort_popup() != POPUP_ACCEPT)
               goto simple_restart;
            goto normal_exit;
         case command_abort:
            if (gg->do_abort_popup() != POPUP_ACCEPT)
               goto simple_restart;
            clear_screen();
            goto show_banner;
         case command_cut_to_clipboard:
            while (backup_one_item()) ;   /* Repeatedly remove any parse blocks that we have. */
            initialize_parse();

            if (configuration::history_ptr <= 1 ||
                (configuration::history_ptr == 2 && configuration::history[1].get_startinfo_specific()->into_the_middle))
               specialfail("Can't cut past this point.");

            clipboard_size++;

            if (clipboard_allocation < clipboard_size) {
               configuration *t;
               clipboard_allocation = clipboard_size;
               /* Increase by 50% beyond what we have now. */
               clipboard_allocation += clipboard_allocation >> 1;
               t = (configuration *)
                  get_more_mem_gracefully(clipboard,
                                          clipboard_allocation * sizeof(configuration));
               if (!t) specialfail("Not enough memory!");
               clipboard = t;
            }

            clipboard[clipboard_size-1] = configuration::history[configuration::history_ptr-1];
            clipboard[clipboard_size-1].command_root = configuration::current_config().command_root;
            configuration::history_ptr--;
            goto start_cycle;
         case command_delete_entire_clipboard:
            if (clipboard_size != 0) {
               if (gg->do_delete_clipboard_popup() != POPUP_ACCEPT)
                  goto simple_restart;
            }

            clipboard_size = 0;
            goto simple_restart;
         case command_delete_one_call_from_clipboard:
            if (clipboard_size != 0) clipboard_size--;
            goto simple_restart;
         case command_paste_one_call:
         case command_paste_all_calls:
            if (clipboard_size == 0) specialfail("The clipboard is empty.");

            while (backup_one_item()) ;   /* Repeatedly remove any parse blocks that we have. */
            initialize_parse();

            if (configuration::history_ptr >= 1 &&
                (configuration::history_ptr >= 2 || !configuration::history[1].get_startinfo_specific()->into_the_middle)) {
               uint32 status = 0;

               while (clipboard_size != 0) {
                  uint32 directions1, directions2, livemask1, livemask2;
                  parse_block *saved_root;
                  setup *old = &configuration::current_config().state;
                  setup *nuu = &clipboard[clipboard_size-1].state;
                  uint32 mask = 0777777;

                  configuration::next_config() = clipboard[clipboard_size-1];

                  /* Save the entire parse tree, in case it gets damaged
                     by an aborted selector replacement. */

                  saved_root = copy_parse_tree(configuration::next_config().command_root);

                  /* If the setup, population, and facing directions don't match, the
                     call execution is problematical.  We don't translate selectors.
                     The operator is responsible for what happens. */

                  if (nuu->kind != old->kind) {
                     status |= 4;
                     goto doitanyway;
                  }

                  directions1 = 0;
                  directions2 = 0;
                  livemask1 = 0;
                  livemask2 = 0;

                  /* Find out whether the formations agree, and gather the information
                     that we need to translate the selectors. */

                  for (i=0; i<=setup_attrs[old->kind].setup_limits; i++) {
                     uint32 p = old->people[i].id1;
                     uint32 q = nuu->people[i].id1;
                     uint32 oldmask = mask;
                     uint32 a = (q >> 6) & 3;
                     uint32 b = (p >> 6) & 3;

                     livemask1 <<= 1;
                     livemask2 <<= 1;
                     if (p) livemask1 |= 1;
                     if (q) livemask2 |= 1;
                     directions1 = (directions1<<2) | (p&3);
                     directions2 = (directions2<<2) | (q&3);

                     mask |= (b|4) << (a*3 + 18);
                     oldmask ^= mask;     /* The bits that changed. */
                     /* Demand that, if anything changed at all, some new field got
                        set.  This has the effect of demanding that existing fields
                        never change, and that only new fields are created or existing
                        fields are rewritten with their original data. */
                     if (oldmask != 0 && (mask & 04444000000) == 0)
                        mask |= 07777000000;  /* Raise error. */
                     mask &= id_fixer_array[(a<<2) | b];
                  }

                  if (directions1 != directions2 || livemask1 != livemask2) {
                     status |= 4;
                     goto doitanyway;
                  }

                  /* Everything matches.  Translate the selectors. */

                  /* If error happened, be sure everyone knows about it. */
                  if ((mask & 07777000000) == 07777000000) mask &= ~07777000000;

                  status |=
                     translate_selector_fields(configuration::next_config().command_root,
                                               (mask << 1) | ((nuu->rotation ^ old->rotation) & 1));

                  if (status & 2) {
                     reset_parse_tree(saved_root, configuration::next_config().command_root);
                     specialfail("Sorry, can't fix person identifier.  "
                                 "You can give the command 'delete one call from clipboard' "
                                 "to remove this call.");
                  }

               doitanyway:

                  interactivity = interactivity_no_query_at_all;
                  testing_fidelity = TRUE;

                  // Create a temporary error handler.

                  try {
                     toplevelmove();
                     finish_toplevelmove();
                     configuration::history_ptr++;
                  }
                  catch(error_flag_type) {
                     // The call failed.
                     interactivity = interactivity_normal;
                     testing_fidelity = FALSE;
                     reset_parse_tree(saved_root, configuration::next_config().command_root);
                     specialfail("The pasted call has failed.  "
                                 "You can give the command 'delete one call from clipboard' "
                                 "to remove it.");
                  }

                  interactivity = interactivity_normal;
                  testing_fidelity = FALSE;
                  clipboard_size--;
                  if ((command_kind) uims_menu_index == command_paste_one_call) break;
               }

               if (status & 4) global_error_flag = error_flag_formation_changed;
               else if (status & 1) global_error_flag = error_flag_selector_changed;
               else global_error_flag = (error_flag_type) 0;
            }

            goto start_cycle;
         case command_undo:
            if (backup_one_item()) {
               /* We succeeded in backing up by one concept.  Continue from that point.
                  But if we backed all the way to the beginning, reset the call menu list. */

               if (parse_state.concept_write_base == parse_state.concept_write_ptr &&
                   parse_state.parse_stack_index == 0)
                  parse_state.call_list_to_use = parse_state.base_call_list_to_use;

               reply_pending = FALSE;
               goto simple_restart;
            }
            else if (parse_state.concept_write_base != parse_state.concept_write_ptr ||
                     parse_state.concept_write_base != &configuration::next_config().command_root) {
               /* Failed to back up, but some concept exists.  This must have been inside
                  a "checkpoint" or similar complex thing.  Just throw it all away,
                  but do not delete any completed calls. */
               reply_pending = FALSE;
               goto start_cycle;
            }
            else {
               // There were no concepts entered.  Throw away the entire preceding line.
               if (configuration::history_ptr > 1) {
                  configuration::history_ptr--;
                  // Going to start_cycle will make sure written_history_items
                  // does not exceed history_ptr.
                  goto start_cycle;
               }
               else {
                  // User type "undo" at the start of the sequence.
                  // Just abort the sequence.  Don't even ask for permission --
                  // there is nothing of value.
                  clear_screen();
                  goto show_banner;
               }
            }
         case command_erase:
            reply_pending = FALSE;
            goto start_cycle;
         case command_save_pic:
            configuration::current_config().draw_pic = TRUE;
            /* We have to back up to BEFORE the item we just changed. */
            if (written_history_items > configuration::history_ptr-1)
               written_history_items = configuration::history_ptr-1;
            goto simple_restart;
         case command_help:
            {
               char help_string[MAX_ERR_LENGTH];
               const char *prefix;
               int current_length;

               switch (parse_state.call_list_to_use) {
               case call_list_lin:
                  prefix = "You are in facing lines."
                     "  Try typing something like 'pass thru' and pressing Enter.";
                  break;
               case call_list_lout:
                  prefix = "You are in back-to-back lines."
                     "  Try typing something like 'wheel and deal' and pressing Enter.";
                  break;
               case call_list_8ch:
                  prefix = "You are in an 8-chain."
                     "  Try typing something like 'pass thru' and pressing Enter.";
                  break;
               case call_list_tby:
                  prefix = "You are in a trade-by."
                     "  Try typing something like 'trade by' and pressing Enter.";
                  break;
               case call_list_rcol: case call_list_lcol:
                  prefix = "You are in columns."
                     "  Try typing something like 'column circulate' and pressing Enter.";
                  break;
               case call_list_rwv: case call_list_lwv:
                  prefix = "You are in waves."
                     "  Try typing something like 'swing thru' and pressing Enter.";
                  break;
               case call_list_r2fl: case call_list_l2fl:
                  prefix = "You are in 2-faced lines."
                     "  Try typing something like 'ferris wheel' and pressing Enter.";
                  break;
               case call_list_dpt:
                  prefix = "You are in a starting DPT."
                     "  Try typing something like 'double pass thru' and pressing Enter.";
                  break;
               case call_list_cdpt:
                  prefix = "You are in a completed DPT."
                     "  Try typing something like 'cloverleaf' and pressing Enter.";
                  break;
               default:
                  prefix = "Type a call and press Enter.";
                  break;
               }

               (void) strncpy(help_string, prefix, MAX_ERR_LENGTH);
               help_string[MAX_ERR_LENGTH-1] = '\0';
               current_length = strlen(help_string);

               if (configuration::sequence_is_resolved()) {
                  (void) strncpy(&help_string[current_length],
                                 "  You may also write out this finished sequence "
                                 "by typing 'write this sequence'.",
                                 MAX_ERR_LENGTH-current_length);
               }
               else {
                  (void) strncpy(&help_string[current_length],
                                 "  You may also type 'resolve'.",
                                 MAX_ERR_LENGTH-current_length);
               }

               help_string[MAX_ERR_LENGTH-1] = '\0';
               specialfail(help_string);
            }
         case command_change_outfile:
            do_change_outfile(TRUE);
            goto start_cycle;
         case command_change_header:
            {
               char newhead_string[MAX_TEXT_LINE_LENGTH];
         
               if (gg->do_header_popup(newhead_string)) {
                  (void) strncpy(header_comment, newhead_string, MAX_TEXT_LINE_LENGTH);

                  if (newhead_string[0]) {
                     char confirm_message[MAX_TEXT_LINE_LENGTH+25];
                     (void) strncpy(confirm_message, "Header comment changed to \"", 28);
                     (void) strncat(confirm_message, header_comment, MAX_TEXT_LINE_LENGTH);
                     (void) strncat(confirm_message, "\"", 2);
                     specialfail(confirm_message);
                  }
                  else {
                     specialfail("Header comment deleted");
                  }
               }
               goto start_cycle;
            }
         case command_getout:
            /* Check that it is really resolved. */

            if (!configuration::sequence_is_resolved()) {
               if (gg->do_write_anyway_popup() != POPUP_ACCEPT)
                  specialfail("This sequence is not resolved.");
               configuration::current_config().draw_pic = TRUE;
            }

            if (!write_sequence_to_file())
               goto start_cycle; /* user cancelled action */
            goto new_sequence;
         case command_select_print_font:
            if (!gg->choose_font())
               specialfail("Printing is not supported in this program.");
            goto start_cycle;
         case command_print_current:
            if (!gg->print_this())
               specialfail("Printing is not supported in this program.");
            goto start_cycle;
         case command_print_any:
            if (!gg->print_any())
               specialfail("Printing is not supported in this program.");
            goto start_cycle;
         case command_help_manual:
            if (!gg->help_manual())
               specialfail("Manual browsing is not supported in this program.");
            goto start_cycle;
         default:     /* Should be some kind of search command. */
            /* If it wasn't, we have a serious problem. */
            if (((command_kind) uims_menu_index) < command_resolve) goto normal_exit;
            search_goal = (command_kind) uims_menu_index;
            global_reply = full_resolve();

            /* If full_resolve refused to operate (for example, we clicked on "reconcile"
               when in an hourglass), it returned "ui_search_accept", which will cause
               us simply to go to start_cycle. */

            /* If user clicked on something random, treat it as though he clicked on "accept"
               followed by whatever it was.  This includes "quit", "abort", "end this sequence",
            and any further search commands.  So a search command (e.g. "pick random call")
            will cause the last search result to be accepted, and begin another search on top
            of that result. */

            if (global_reply == ui_command_select) {
               switch ((command_kind) uims_menu_index) {
               case command_quit:
               case command_abort:
               case command_getout:
                  break;
               default:
                  if (((command_kind) uims_menu_index) < command_resolve) goto start_cycle;
                  break;
               }

               allowing_modifications = 0;
               configuration::next_config().draw_pic = FALSE;
               parse_state.concept_write_base = &configuration::next_config().command_root;
               parse_state.concept_write_ptr = parse_state.concept_write_base;
               *parse_state.concept_write_ptr = (parse_block *) 0;
               reply_pending = TRUE;
               /* Going to start_with_pending_reply will make sure written_history_items does not exceed history_ptr. */
               goto start_with_pending_reply;
            }

            goto start_cycle;
         }
      }

      goto normal_exit;
   }      
   catch(error_flag_type sss) {
      global_error_flag = sss;
      goto got_an_exception;
   }

 normal_exit: ;
}
