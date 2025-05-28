/* SD -- square dance caller's helper.

    Copyright (C) 1990-1999  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

/* This defines the following functions:
   clear_screen
   writechar
   newline
   writestuff
   doublespace_file
   exit_program
   string_copy
   display_initial_history
   write_history_line
   deposit_call_tree
   find_proper_call_list
   do_throw
   do_subcall_query

and the following external variables:
   longjmp_buffer
   longjmp_ptr
   history_allocation
   journal_file
   the_topcallflags
   there_is_a_call
   enable_file_writing
   singlespace_mode
   nowarn_mode
   accept_single_click
   getout_strings
   filename_strings
   level_threshholds
   higher_acceptable_level
   menu_names
   command_menu
   resolve_menu
   startup_commands
   last_direction_kind
   writechar_block
   use_escapes_for_drawing_people
   pn1
   pn2
   direc
*/

/* For "sprintf" */
#include <stdio.h>
#include <string.h>
#include "sdprog.h"
#include "resource.h"


#define SCREWED_UP_REVERTS



/*  This is the line length beyond which we will take pity on
   whatever device has to print the result, and break the text line.
   It is presumably smaller than our internal text capacity. */

// It is an observed fact that, with the default font (14 point Courier),
// one can print 66 characters on 8.5 x 11 inch paper.

#define MAX_PRINT_LENGTH 59

/* These variables are external. */

real_jmp_buf longjmp_buffer;
real_jmp_buf *longjmp_ptr;

int history_allocation = 0; /* How many items are currently allocated in "history". */
FILE *journal_file = (FILE *) 0;
uint32 the_topcallflags;
long_boolean there_is_a_call;

long_boolean enable_file_writing;
long_boolean singlespace_mode;
long_boolean nowarn_mode;
long_boolean accept_single_click;

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
Cstring getout_strings[] = {
   "Mainstream",
   "Plus",
   "A1",
   "A2",
   "C1",
   "C2",
   "C3A",
   "C3",
   "C3X",
   "C4A",
   "C4",
   "C4X",
   "all",
   ""};

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
Cstring filename_strings[] = {
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

/* This list tells what level calls will be accepted for the "pick level call"
   operation.  When doing a "pick level call, we don't actually require calls
   to be exactly on the indicated level, as long as it's plausibly close. */
/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
dance_level level_threshholds[] = {
   l_mainstream,
   l_plus,
   l_a1,
   l_a1,      /* If a2 is given, an a1 call is OK. */
   l_c1,
   l_c2,
   l_c3a,
   l_c3a,     /* If c3 is given, a c3a call is OK. */
   l_c3a,     /* If c3x is given, a c3a call is OK. */
   l_c3x,     /* If c4a is given, a c3x call is OK. */
   l_c3x,     /* If c4 is given, a c3x call is OK. */
   l_c3x,     /* If c4x is given, a c3x call is OK. */
   l_dontshow,
   l_nonexistent_concept};

/* This list tells what level calls will be put in the menu and hence made available.
   In some cases, we make calls available that are higher than the requested level.
   When we use such a call, a warning is printed. */

/* BEWARE!!  This list is keyed to the definition of "dance_level" in database.h . */
dance_level higher_acceptable_level[] = {
   l_mainstream,
   l_plus,
   l_a1,
   l_a2,
   l_c1,
   l_c2,
   l_c3a,
   l_c3x,     /* If c3 is given, we allow c3x. */
   l_c3x,
   l_c4a,
   l_c4x,     /* If c4 is given, we allow c4x. */
   l_c4x,
   l_dontshow,
   l_nonexistent_concept};


/* BEWARE!!  This list is keyed to the definition of "call_list_kind" in sd.h . */
Cstring menu_names[] = {
   "???",
   "???",
   "(any setup)",
   "tidal wave",
   "left tidal wave",
   "dpt",
   "cdpt",
   "columns",
   "left columns",
   "8 chain",
   "trade by",
   "facing lines",
   "lines-out",
   "waves",
   "left waves",
   "2-faced lines",
   "left 2-faced lines",
   "tidal column",
   "diamond/qtag"};

command_list_menu_item command_menu[] = {
   {"exit the program",               command_quit, ID_FILE_EXIT},
   {"quit the program",               command_quit, -1},
   {"simple modifications",           command_simple_mods, -1},
   {"allow modifications",            command_all_mods, -1},
   {"toggle concept levels",          command_toggle_conc_levels, ID_COMMAND_TOGGLE_CONC},
   {"toggle active phantoms",         command_toggle_act_phan, ID_COMMAND_TOGGLE_PHAN},
   {"toggle retain after error",      command_toggle_retain_after_error, -1},
   {"toggle nowarn mode",             command_toggle_nowarn_mode, -1},
   {"toggle singleclick mode",        command_toggle_singleclick_mode, -1},
   {"choose font for printing",       command_select_print_font, ID_FILE_CHOOSE_FONT},
   {"print current file",             command_print_current, ID_FILE_PRINTTHIS},
   {"print any file",                 command_print_any, ID_FILE_PRINTFILE},
   {"undo last call",                 command_undo, ID_COMMAND_UNDO},
   {"discard entered concepts",       command_erase, ID_COMMAND_DISCARD_CONCEPT},
   {"abort this sequence",            command_abort, ID_COMMAND_ABORTTHISSEQUENCE},
   {"insert a comment",               command_create_comment, ID_COMMAND_COMMENT},
   {"change output file",             command_change_outfile, ID_COMMAND_CH_OUTFILE},
   {"change title",                   command_change_header, ID_COMMAND_CH_TITLE},
   {"write this sequence",            command_getout, -1},
   {"end this sequence",              command_getout, ID_COMMAND_ENDTHISSEQUENCE},
   {"cut to clipboard",               command_cut_to_clipboard, -1},
   {"clipboard cut",                  command_cut_to_clipboard, ID_COMMAND_CLIPBOARD_CUT},
   {"delete entire clipboard",        command_delete_entire_clipboard, -1},
   {"clipboard delete all",           command_delete_entire_clipboard, ID_COMMAND_CLIPBOARD_DEL_ALL},
   {"delete one call from clipboard", command_delete_one_call_from_clipboard, -1},
   {"clipboard delete one",           command_delete_one_call_from_clipboard, ID_COMMAND_CLIPBOARD_DEL_ONE},
   {"paste one call",                 command_paste_one_call, -1},
   {"clipboard paste one",            command_paste_one_call, ID_COMMAND_CLIPBOARD_PASTE_ONE},
   {"paste all calls",                command_paste_all_calls, ID_COMMAND_CLIPBOARD_PASTE_ALL},
   {"clipboard paste all",            command_paste_all_calls, -1},
   {"keep picture",                   command_save_pic, ID_COMMAND_KEEP_PICTURE},
   {"refresh display",                command_refresh, -1},
   {"resolve",                        command_resolve, ID_COMMAND_RESOLVE},
   {"normalize",                      command_normalize, ID_COMMAND_NORMALIZE},
   {"standardize",                    command_standardize, ID_COMMAND_STANDARDIZE},
   {"reconcile",                      command_reconcile, ID_COMMAND_RECONCILE},
   {"pick random call",               command_random_call, ID_COMMAND_PICK_RANDOM},
   {"pick simple call",               command_simple_call, ID_COMMAND_PICK_SIMPLE},
   {"pick concept call",              command_concept_call, ID_COMMAND_PICK_CONCEPT},
   {"pick level call",                command_level_call, ID_COMMAND_PICK_LEVEL},
   {"pick 8 person level call",       command_8person_level_call, ID_COMMAND_PICK_8P_LEVEL},
   {"create waves",                   command_create_waves, ID_COMMAND_CREATE_WAVES},
   {"create 2fl",                     command_create_2fl, ID_COMMAND_CREATE_2FL},
   {"create lines in",                command_create_li, ID_COMMAND_CREATE_LINESIN},
   {"create lines out",               command_create_lo, ID_COMMAND_CREATE_LINESOUT},
   {"create inverted lines",          command_create_inv_lines, ID_COMMAND_CREATE_INVLINES},
   {"create 3x1 lines",               command_create_3and1_lines, ID_COMMAND_CREATE_3N1LINES},
   {"create any lines",               command_create_any_lines, ID_COMMAND_CREATE_ANYLINES},
   {"create columns",                 command_create_col, ID_COMMAND_CREATE_COLUMNS},
   {"create magic columns",           command_create_magic_col, ID_COMMAND_CREATE_MAGCOL},
   {"create dpt",                     command_create_dpt, ID_COMMAND_CREATE_DPT},
   {"create cdpt",                    command_create_cdpt, ID_COMMAND_CREATE_CDPT},
   {"create 8 chain",                 command_create_8ch, ID_COMMAND_CREATE_8CH},
   {"create trade by",                command_create_tby, ID_COMMAND_CREATE_TRBY},
   {"create any columns",             command_create_any_col, ID_COMMAND_CREATE_ANYCOLS},
   {"create tidal wave",              command_create_tidal_wave, ID_COMMAND_CREATE_GWV},
   {"create any tidal setup",         command_create_any_tidal, ID_COMMAND_CREATE_ANY_TIDAL},
   {"create diamonds",                command_create_dmd, ID_COMMAND_CREATE_DMD},
   {"create 1/4 tag",                 command_create_qtag, ID_COMMAND_CREATE_QTAG},
   {"create 3/4 tag",                 command_create_3qtag, ID_COMMAND_CREATE_3QTAG},
   {"create 1/4 line",                command_create_qline, ID_COMMAND_CREATE_QLINE},
   {"create 3/4 line",                command_create_3qline, ID_COMMAND_CREATE_3QLINE},
   {"create any 1/4 tag",             command_create_any_qtag, ID_COMMAND_CREATE_ANY_QTAG},
   {(Cstring) 0}};

resolve_list_menu_item resolve_menu[] = {
   {"abort the search",       resolve_command_abort},
   {"exit the search",        resolve_command_abort},
   {"quit the search",        resolve_command_abort},
   {"undo the search",        resolve_command_abort},
   {"find another",           resolve_command_find_another},
   {"next",                   resolve_command_goto_next},
   {"previous",               resolve_command_goto_previous},
   {"accept current choice",  resolve_command_accept},
   {"raise reconcile point",  resolve_command_raise_rec_point},
   {"lower reconcile point",  resolve_command_lower_rec_point},
   {"write this sequence",    resolve_command_write_this},
   {(Cstring) 0}};

startup_list_menu_item startup_menu[] = {
   {"exit from the program",       start_select_exit, ID_FILE_EXIT},
   {"heads start",                 start_select_heads_start, -1},
   {"sides start",                 start_select_sides_start, -1},
   {"heads 1p2p",                  start_select_h1p2p, -1},
   {"sides 1p2p",                  start_select_s1p2p, -1},
   {"just as they are",            start_select_as_they_are, -1},
   {"toggle concept levels",       start_select_toggle_conc, ID_COMMAND_TOGGLE_CONC},
   {"toggle active phantoms",      start_select_toggle_act, ID_COMMAND_TOGGLE_PHAN},
   {"toggle retain after error",   start_select_toggle_retain, -1},
   {"toggle nowarn mode",          start_select_toggle_nowarn_mode, -1},
   {"toggle singleclick mode",     start_select_toggle_singleclick_mode, -1},
   {"toggle singing call",         start_select_toggle_singer, -1},
   {"toggle reverse singing call", start_select_toggle_singer_backward, -1},
   {"choose font for printing",    start_select_select_print_font, ID_FILE_CHOOSE_FONT},
   {"print current file",          start_select_print_current, ID_FILE_PRINTTHIS},
   {"print any file",              start_select_print_any, ID_FILE_PRINTFILE},
   {"initialize session file",     start_select_init_session_file, -1},
   {"change output file",          start_select_change_outfile, ID_COMMAND_CH_OUTFILE},
   {"change title",                start_select_change_header_comment, ID_COMMAND_CH_TITLE},
   {(Cstring) 0}};



int last_direction_kind = direction_zagzag;


writechar_block_type writechar_block;
static char current_line[MAX_TEXT_LINE_LENGTH];
static int text_line_count = 0;

static int drawing_picture = 0;
static int squeeze_this_newline = 0;



Private void open_text_line(void)
{
   writechar_block.destcurr = current_line;
   writechar_block.lastchar = ' ';
   writechar_block.lastlastchar = ' ';
   writechar_block.lastblank = (char *) 0;
}


extern void clear_screen(void)
{
   written_history_items = -1;
   text_line_count = 0;
   uims_reduce_line_count(0);
   open_text_line();
}


extern void writechar(char src)
{
   writechar_block.lastlastchar = writechar_block.lastchar;

   *writechar_block.destcurr = (writechar_block.lastchar = src);
   if (src == ' ' && writechar_block.destcurr != current_line)
      writechar_block.lastblank = writechar_block.destcurr;

   /* If drawing a picture, don't do automatic line breaks. */

   if (writechar_block.destcurr < &current_line[MAX_PRINT_LENGTH] || writechar_block.usurping_writechar || drawing_picture)
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
   while (writechar_block.destcurr != current_line && writechar_block.destcurr[-1] == ' ')
      writechar_block.destcurr--;

   *writechar_block.destcurr = '\0';

   /* There will be no special "5" or "6" characters in pictures (drawing_picture&1) if:

      Enable_file_writing is on (we don't write the special stuff to a file,
         of course, and we don't even write it to the transcript when showing the
         final card)

      Use_escapes_for_drawing_people is 0 or 1 (so we don't do it for Sdtty under
         DJGPP or GCC (0), or for Sdtty under Windows (1))

      No_graphics != 0 (so we only do it if showing full checkers in Sd) */


   if (enable_file_writing)
      write_file(current_line);

   text_line_count++;
   uims_add_new_line(current_line,
                     enable_file_writing ? 0 : (drawing_picture | (squeeze_this_newline << 1)));
   open_text_line();
}



extern void writestuff(Const char *s)
{
   while (*s) writechar(*s++);
}



extern void doublespace_file(void)
{
   write_file("");
}



extern void exit_program(int code)
{
   if (journal_file) (void) fclose(journal_file);
   uims_terminate();
   final_exit(code);
}



extern void string_copy(char **dest, Cstring src)
{
   Cstring f = src;
   char *t = *dest;

   while ((*t++ = *f++));
   *dest = t-1;
}



/* These static variables are used by printperson. */

static char peoplenames1[] = "11223344";
static char peoplenames2[] = "BGBGBGBG";
static char directions[] = "?>?<????^?V?????";

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

int use_escapes_for_drawing_people = 0;

/* These could get changed if the user requests special naming.  See "alternate_glyphs_1"
   in the command-line switch parser in sdsi.c. */
char *pn1 = peoplenames1;
char *pn2 = peoplenames2;
char *direc = directions;


Private void printperson(uint32 x)
{
   if (x & BIT_PERSON) {
      if (enable_file_writing || use_escapes_for_drawing_people == 0) {
         /* We never write anything other than standard ASCII to the transcript file. */
         writechar(' ');
         writechar(pn1[(x >> 6) & 7]);
         writechar(pn2[(x >> 6) & 7]);
         if (enable_file_writing)
            writechar(directions[x & 017]);
         else
            writechar(direc[x & 017]);
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
      if (enable_file_writing || use_escapes_for_drawing_people <= 1)
         writestuff("  . ");
      else
         writechar('\014');  /* If we have full ("checker") escape sequences, use this. */
   }
}

/* These static variables are used by printsetup/print_4_person_setup/do_write. */

static int offs, roti, modulus, personstart;
static setup *printarg;

Private void do_write(Cstring s)
{
   char c;

   int ri = roti * 011;

   for (;;) {
      if (!(c=(*s++))) return;
      else if (c == '@') {
         if (*s == '7') {
            s++;
            squeeze_this_newline = 1;
            newline();
            squeeze_this_newline = 0;
         }
         else {
            newline();
         }
      }
      else if (c >= 'a' && c <= 'x')
         printperson(rotperson(printarg->people[personstart + ((c-'a'-offs)%modulus)].id1, ri));
      else {
         /* We need to do the mundane translation of "5" and "6" if the result
            isn't going to be used by something that uses same. */
         if (enable_file_writing || use_escapes_for_drawing_people <= 1 || ui_options.no_graphics != 0) {
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



Private void print_4_person_setup(int ps, small_setup *s, int elong)
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



Private void printsetup(setup *x)
{
   Cstring str;

   drawing_picture = 1;
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
         case s_qtag:
            if ((x->people[0].id1 & x->people[1].id1 &
                 x->people[4].id1 & x->people[5].id1 & 1) &&
                (x->people[2].id1 & x->people[3].id1 &
                 x->people[6].id1 & x->people[7].id1 & 010)) {
               /* People are in diamond-like orientation. */
               if (x->rotation & 1)
                  do_write("6  g@7f  6  a@76  h@@6  d@7e  6  b@76  c");
               else
                  do_write("5 a6 b@@g h d c@@5 f6 e");
            }
            else {
               /* People are not.  Probably 1/4-tag-like orientation. */
               if (x->rotation & 1)
                  do_write("6  g@f  h  a@e  d  b@6  c");
               else
                  do_write("6  a  b@@g  h  d  c@@6  f  e");
            }
            break;
         case s_c1phan:
            /* Look for nice "classic C1 phantom" occupations, and  draw
               tighter diagram, if using checkers, if so. */
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
            drawing_picture = 0;
            writestuff(" centers only:");
            newline();
            drawing_picture = 1;
            print_4_person_setup(0, &(x->inner), -1);
            break;
         case s_normal_concentric:
            drawing_picture = 0;
            writestuff(" centers:");
            newline();
            drawing_picture = 1;
            print_4_person_setup(0, &(x->inner), -1);
            drawing_picture = 0;
            writestuff(" ends:");
            newline();
            drawing_picture = 1;
            print_4_person_setup(12, &(x->outer), x->concsetup_outer_elongation);
            break;
         default:
            drawing_picture = 0;
            writestuff("???? UNKNOWN SETUP ????");
            newline();
            drawing_picture = 1;
      }
   }

   newline();
   drawing_picture = 0;
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
      write_header_stuff(TRUE, 0);
      newline();
      newline();
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
   int centersp, w, i;
   parse_block *thing;
   configuration *this_item = &history[history_index];

   if (write_to_file == file_write_double && !singlespace_mode)
      doublespace_file();

   centersp = this_item->centersp;

   /* Do not put index numbers into output file -- user may edit it later. */

   if (!enable_file_writing && !diagnostic_mode) {
      i = history_index-whole_sequence_low_lim+1;
      if (i > 0) {
         char indexbuf[10];
         sprintf(indexbuf, "%2d:   ", i);
         writestuff(indexbuf);
      }
   }

   if (centersp != 0) {
      if (startinfolist[centersp].into_the_middle) goto morefinal;
      writestuff(startinfolist[centersp].name);
      goto final;
   }

   thing = this_item->command_root;
   
   /* Need to check for the special case of starting a sequence with heads or sides.
      If this is the first line of the history, and we started with heads of sides,
      change the name of this concept from "centers" to the appropriate thing. */

   if (history_index == 2 && thing->concept->kind == concept_centers_or_ends && thing->concept->value.arg1 == selector_centers) {
      centersp = history[1].centersp;
      if (startinfolist[centersp].into_the_middle) {
         writestuff(startinfolist[centersp].name);
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

   /* First, don't print both "bad concept level" and "bad modifier level". */

   if ((1 << (warn__bad_concept_level & 0x1F)) & this_item->warnings.bits[warn__bad_concept_level>>5])
      this_item->warnings.bits[warn__bad_modifier_level>>5] &= ~(1 << (warn__bad_modifier_level & 0x1F));

   /* Or "opt for parallelogram" and "each 1x4". */

   if ((1 << (warn__check_pgram & 0x1F)) & this_item->warnings.bits[warn__check_pgram>>5])
      this_item->warnings.bits[warn__each1x4>>5] &= ~(1 << (warn__each1x4 & 0x1F));

   /* Or "each 1x6" and "each 1x3". */

   if ((1 << (warn__split_1x6 & 0x1F)) & this_item->warnings.bits[warn__split_1x6>>5])
      this_item->warnings.bits[warn__split_to_1x6s>>5] &= ~(1 << (warn__split_to_1x6s & 0x1F));

   if (!nowarn_mode) {
      for (w=0 ; w<NUM_WARNINGS ; w++) {
         if ((1 << (w & 0x1F)) & this_item->warnings.bits[w>>5]) {
            writestuff("  Warning:  ");
            writestuff(&warning_strings[w][1]);
            newline();
         }
      }
   }

   if (picture || this_item->draw_pic)
      printsetup(&this_item->state);

   /* Record that this history item has been written to the UI. */
   this_item->text_line = text_line_count;
}




/* This stuff is duplicated in verify_call in sdmatch.c . */
extern long_boolean deposit_call_tree(modifier_block *anythings, parse_block *save1, int key)
{
   /* First, if we have already deposited a call, and we see more stuff, it must be
      concepts or calls for an "anything" subcall. */

   if (save1) {
      parse_block *tt = get_parse_block();
      /* Run to the end of any already-deposited things.  This could happen if the
         call takes a tagger -- it could have a search chain before we even see it. */
      while (save1->next) save1 = save1->next;
      save1->next = tt;
      save1->concept = &marker_concept_mod;
      tt->concept = &marker_concept_mod;
      tt->call = base_calls[(key == 6) ? base_call_null_second: base_call_null];
      tt->call_to_print = tt->call;
      tt->replacement_key = key;
      parse_state.concept_write_ptr = &tt->subsidiary_root;
   }

   save1 = (parse_block *) 0;
   user_match.match.call_conc_options = anythings->call_conc_options;

   if (anythings->kind == ui_call_select) {
      if (deposit_call(anythings->call_ptr, &anythings->call_conc_options)) return TRUE;
      save1 = *parse_state.concept_write_ptr;
      if (!there_is_a_call) the_topcallflags = parse_state.topcallflags1;
      there_is_a_call = TRUE;
   }
   else if (anythings->kind == ui_concept_select) {
      if (deposit_concept(anythings->concept_ptr)) return TRUE;
   }
   else return TRUE;   /* Huh? */

   if (anythings->packed_next_conc_or_subcall) {
      /* key for "mandatory_anycall" */
      if (deposit_call_tree(anythings->packed_next_conc_or_subcall, save1, 2)) return TRUE;
   }

   if (anythings->packed_secondary_subcall) {
      /* key for "mandatory_secondary_call" */
      if (deposit_call_tree(anythings->packed_secondary_subcall, save1, 6)) return TRUE;
   }

   return FALSE;
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

extern void do_throw(error_flag_type f)
{
   longjmp(longjmp_ptr->the_buf, f);
}


extern long_boolean do_subcall_query(
   int snumber,
   parse_block *parseptr,
   parse_block **newsearch,
   long_boolean this_is_tagger,
   long_boolean this_is_tagger_circcer,
   callspec_block *orig_call)
{
   char tempstring_text[MAX_TEXT_LINE_LENGTH];

   /* Note whether we are using any mandatory substitutions, so that the menu
      initialization will always accept this call. */

   if (snumber == 2 || snumber == 6) {
      /* In some types of pick operations, the picker simply doesn't know how
         to choose a mandatory subcall.  In that case, the call requiring the
         mandatory subcall (e.g. "wheel and <anything>") is simply rejected. */
      if (forbid_call_with_mandatory_subcall())
         fail("Mandatory subcall fail.");
      mandatory_call_used = TRUE;
   }

   /* Now we know that the list doesn't say anything about this call.  Perhaps we should
      query the user for a replacement and add something to the list.  First, decide whether
      we should consider doing so.  If we are initializing the database, the answer is
      always "no", even for calls that require a replacement call, such as
      "clover and anything".  This means that, for the purposes of database initialization,
      "clover and anything" is tested as "clover and nothing", since "nothing" is the subcall
      that appears in the database. */

   /* Also, when doing pick operations, the picker might not want to do a random pick.
      It might just want to leave the default call ("clover and [nothing]") in place.
      So we ask the picker. */

   /* Of course, if we are testing the fidelity of later calls during a reconcile
      operation, we DO NOT EVER add any modifiers to the list, even if the user
      clicked on "allow modification" before clicking on "reconcile".  It is perfectly
      legal to click on "allow modification" before clicking on "reconcile".  It means
      we want modifications (chosen by random number generator, since we won't be
      interactive) for the calls that we randomly choose, but not for the later calls
      that we test for fidelity. */

   if (!(interactivity == interactivity_normal ||
         allow_random_subcall_pick()) ||
       testing_fidelity)
      return TRUE;

   /* When we are searching for resolves and the like, the situation is different.  In this case,
      the interactivity state is set for a search.  We do perform mandatory
      modifications, so we will generate things like "clover and shakedown".  Of course, no
      querying actually takes place.  Instead, get_subcall just uses the random number generator.
      Therefore, whether resolving or in normal interactive mode, we are guided by the
      call modifier flags and the "allowing_modifications" global variable. */

   /* Depending on what type of substitution is involved and what the "allowing modifications"
      level is, we may choose not to query about this subcall, but just return the default. */

   switch (snumber) {
      case 1:   /* or_anycall */
      case 3:   /* allow_plain_mod */
      case 5:   /* or_secondary_call */
         if (!allowing_modifications) return TRUE;
         break;
      case 4:   /* allow_forced_mod */
         if (allowing_modifications <= 1) return TRUE;
         break;
   }

   /* At this point, we know we should query the user about this call. */

   /* Set ourselves up for modification by making the null modification list
      if necessary.  ***** Someday this null list will always be present. */

   if (parseptr->concept->kind == marker_end_of_list)
      parseptr->concept = &marker_concept_mod;

   /* Create a reference on the list.  "search" points to the null item at the end. */

   tempstring_text[0] = '\0';           /* Null string, just to be safe. */

   /* If doing a tagger, just get the call. */

   if (snumber == 0 && this_is_tagger_circcer)
      ;

   /* If the replacement is mandatory, or we are not interactive,
      don't present the popup.  Just get the replacement call. */

   else if (interactivity != interactivity_normal)
      ;
   else if (snumber == 2)
      (void) sprintf (tempstring_text, "SUBSIDIARY CALL");
   else if (snumber == 6)
      (void) sprintf (tempstring_text, "SECOND SUBSIDIARY CALL");
   else {

      /* Need to present the popup to the operator and find out whether modification is desired. */

      modify_popup_kind kind;
      char pretty_call_name[MAX_TEXT_LINE_LENGTH];

      /* Star turn calls can have funny names like "nobox". */

      unparse_call_name(
         (orig_call->callflags1 & CFLAG1_IS_STAR_CALL) ?
         "turn the star @b" : orig_call->name,
         pretty_call_name, &current_options);

      if (this_is_tagger) kind = modify_popup_only_tag;
      else if (this_is_tagger_circcer) kind = modify_popup_only_circ;
      else kind = modify_popup_any;

      if (uims_do_modifier_popup(pretty_call_name, kind)) {
         /* User accepted the modification.
            Set up the prompt and get the concepts and call. */

         (void) sprintf (tempstring_text, "REPLACEMENT FOR THE %s", pretty_call_name);
      }
      else {
         /* User declined the modification.  Create a null entry so that we don't query again. */
         *newsearch = get_parse_block();
         (*newsearch)->concept = &marker_concept_mod;
         (*newsearch)->options = current_options;
         (*newsearch)->replacement_key = snumber;
         (*newsearch)->call = orig_call;
         (*newsearch)->call_to_print = orig_call;
         return TRUE;
      }
   }

   *newsearch = get_parse_block();
   (*newsearch)->concept = &marker_concept_mod;
   (*newsearch)->options = current_options;
   (*newsearch)->replacement_key = snumber;
   (*newsearch)->call = orig_call;
   (*newsearch)->call_to_print = orig_call;

   /* Set stuff up for reading subcall and its concepts. */

   /* Create a new parse block, point concept_write_ptr at its contents. */
   /* Create the new root at the start of the subsidiary list. */

   parse_state.concept_write_base = &(*newsearch)->subsidiary_root;
   parse_state.concept_write_ptr = parse_state.concept_write_base;

   parse_state.parse_stack_index = 0;
   parse_state.call_list_to_use = call_list_any;
   (void) strncpy(parse_state.specialprompt, tempstring_text, MAX_TEXT_LINE_LENGTH);

   /* Search for special case of "must_be_tag_call" with no other modification bits.
      That means it is a new-style tagging call. */

   if (snumber == 0 && this_is_tagger_circcer) {
      longjmp(longjmp_ptr->the_buf, 5);
   }
   else {
      if (query_for_call())
         longjmp(longjmp_ptr->the_buf, 5);     /* User clicked on something unusual like "exit" or "undo". */
   }

   return FALSE;
}
