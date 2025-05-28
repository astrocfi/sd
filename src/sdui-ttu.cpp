/*
 * sdui-ttu.cpp - helper functions for sdui-tty interface to use the Unix
 * "curses" mechanism.
 * Time-stamp: <93/11/27 11:06:39 gildea>
 * Copyright 1993 Stephen Gildea
 * Copyright 2001 Chris "Fen" Tamanaha	<tamanaha@coyotesys.com>
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Stephen Gildea <gildea@lcs.mit.edu> January 1993
 * Certain editorial comments by William Ackerman <wba@apollo.com> July 1993
 *
 */


// The "NO_CURSES" compile-time switch will shut off all attempts to use the "curses"
// system service, including the inclusion of the "curses.h" include file.
// In this case, the program will ignore the "-no_cursor" run-time switch, and will
// always act as though that switch had been given.


#ifndef NO_CURSES
#include <curses.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>   // We use this stuff if "-no_cursor" was specified.
#include <unistd.h>    //    This too.

#if !defined(NO_IOCTL) && (defined(__linux__) || defined(__CYGWIN__) || defined(__APPLE__))
#include <sys/ioctl.h>
#endif

#include <signal.h>
#include <string.h>
#include "sd.h"


#ifndef NO_CURSES
static bool curses_initialized = false;
#endif

static int current_tty_mode = 0;

struct colorspec {
   int curses_index;
   Cstring vt100_string;
};

static colorspec color_translations[8] = {
   {0, "00"},  // 0 - not used
   {0, "30"},  // 1 - substitute yellow
   {2, "31"},  // 2 - red
   {3, "32"},  // 3 - green
   {4, "33"},  // 4 - yellow
   {5, "34"},  // 5 - blue
   {6, "35"},  // 6 - magenta
   {7, "36"}}; // 7 - cyan

static void csetmode(int mode)             /* 1 means raw, no echo, one character at a time;
                                                0 means normal. */
{
    static cc_t orig_eof = '\004';
    struct termios term;
    int fd;

    if (mode == current_tty_mode) return;

    fd = fileno(stdin);

    (void) tcgetattr(fd, &term);
    if (mode == 1) {
         orig_eof = term.c_cc[VEOF]; /* VMIN may clobber */
	term.c_cc[VMIN] = 1;	/* 1 char at a time */
	term.c_cc[VTIME] = 0;	/* no time limit on input */
	term.c_lflag &= ~(ICANON|ECHO);
    } else {
	term.c_cc[VEOF] = orig_eof;
	term.c_lflag |= ICANON|ECHO;
    }
    (void) tcsetattr(fd, TCSADRAIN, &term);
    current_tty_mode = mode;
}


static void term_handler(int n)
{
   if ( n == SIGQUIT ) {
      abort();
   }
   else {
      session_index = 0;  // Prevent attempts to update session file.
      general_final_exit(n);
   }
}

extern void ttu_set_window_title(const char *string) {}
void iofull::set_pick_string(const char *string) {}


void iofull::display_help()
{
   printf("-lines <n>                  assume this many lines on the screen\n");
   printf("-no_line_delete             do not use the \"line delete\" function for screen management\n");
   printf("-no_cursor                  do not use screen management functions at all\n");
   printf("-journal <filename>         echo input commands to journal file\n");
}

bool iofull::help_manual() { return false; }
bool iofull::help_faq() { return false; }


#ifndef NO_CURSES
static int orig_curses_bkgd;
#endif

extern void ttu_initialize()
{
   // Set the default value if the user hasn't explicitly set something.
   if (sdtty_screen_height <= 0) sdtty_screen_height = 25;

#ifdef NO_IOCTL
   sdtty_no_console = true;
#endif

   // If user doesn't want any console stuff at all, turn off "curses".
   sdtty_no_cursor |= sdtty_no_console;

#ifdef NO_CURSES
   sdtty_no_cursor = true;
#else
   if (!sdtty_no_cursor) {
      initscr();    /* Initialize "curses". */
      curses_initialized = true;
      noecho();     /* Don't echo; we will do it ourselves. */
      cbreak();     /* Give us each keystroke; don't wait for entire line. */
      scrollok(stdscr, 1);    /* Permit screen to scroll when we write beyond the bottom.
                                 Of course, that means we lose a line at the top, but
                                 finite-memory dumb terminals such as VT-100's are like that.
                                 A consequence of this lost line is that "erase_last_n" may
                                 get unhappy and just erase the entire screen. */

      /* Tell the "curses" system whether we desire to use the line insert/delete
         capabilities of the output device.  The documentation says that, under
         some circumstances the use of this capability can be annoying to the user.
         We let the user decide.

         Note that the second argument to "idlok" is defined in the documentation
         as being of type "bool", and must be given as TRUE or FALSE.  (The HP-UX
         documentation specifically says that you must use TRUE or FALSE, not 1 or 0,
         even though an examination of the file /usr/include/curses.h shows these
         constants defined as 1 and 0.)  Apparently the convention, used by
         C programmers throughout the rest of the known universe, that boolean
         conditions are expressed as integers, with zero denoting falsehood and
         any nonzero value denoting truth, wasn't good enough for the designers
         of "curses", so they added a new data type and constants just for us.
         In choosing the names of the type and the constants, they also seem to
         have felt secure in the assumption that *NO ONE ELSE*, either end-user or
         another subsystem designer, would ever conceivably want to use such obscure
         names as "bool", "TRUE", and "FALSE".

         Note that, in the spirit of encapsulation, we use a conditional
         expression to turn our ordinary 1/0 condition into the required
         TRUE/FALSE values.  We have no choice, since we aren't really allowed
         to look at the include file.  We rest assured and confident that the
         compiler will not be confused by this and will generate optimal code.
         Right? */

      idlok(stdscr, sdtty_no_line_delete ? FALSE : TRUE);
      keypad(stdscr, TRUE);

      if (has_colors()) {
         start_color();
         orig_curses_bkgd = getbkgd(stdscr);
         if (ui_options.reverse_video) {
            init_pair(1, COLOR_WHITE, COLOR_BLACK);
            init_pair(2, COLOR_RED, COLOR_BLACK);
            init_pair(3, COLOR_GREEN, COLOR_BLACK);
            init_pair(4, COLOR_YELLOW, COLOR_BLACK);
            init_pair(5, COLOR_BLUE, COLOR_BLACK);
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
            init_pair(7, COLOR_CYAN, COLOR_BLACK);
         }
         else {
            init_pair(1, COLOR_BLACK, COLOR_WHITE);
            init_pair(2, COLOR_RED, COLOR_WHITE);
            init_pair(3, COLOR_GREEN, COLOR_WHITE);
            init_pair(4, COLOR_YELLOW, COLOR_WHITE);
            init_pair(5, COLOR_BLUE, COLOR_WHITE);
            init_pair(6, COLOR_MAGENTA, COLOR_WHITE);
            init_pair(7, COLOR_CYAN, COLOR_WHITE);
         }
         bkgdset(orig_curses_bkgd | A_BOLD | COLOR_PAIR(1));
         color_set(1, 0);
         if (!ui_options.no_intensify) attron(A_BOLD);
      }
   }
#endif
}

void ttu_terminate()
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      if (curses_initialized) {
         if (has_colors()) bkgdset(orig_curses_bkgd);
         endwin();
      }
#endif
   }
   else
      csetmode(0);   // Restore normal input mode.
}

// If not using curses, query the terminal driver for the
//	window size.  The file descriptor for stdout is 1.
//	If the window size cannot be determined, return 24,
//	(or whatever the user specified) as a reasonable guess.
//
static int get_nocurses_term_lines()
{
   if (!sdtty_no_console) {
#ifndef NO_IOCTL
      // If NO_IOCTL is on, we can't compile these lines.
      // But we'll never get here in that case.
      struct winsize w;

      if (ioctl(1, TIOCGWINSZ, &w) >= 0)
         return w.ws_row;
      else
         return sdtty_screen_height-1;
#else
      return sdtty_screen_height-1;
#endif
   }
   else
      return sdtty_screen_height-1;
}

extern int get_lines_for_more()
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      int x, y;
      getmaxyx(stdscr, y, x);		// returns 1 more than desired values
      return y-1;
#else
      return get_nocurses_term_lines();
#endif
   }
   else
      return get_nocurses_term_lines();
}


#ifndef NO_CURSES
static void getyx_correctly(int *y, int *x)
{
   int localy, localx;

/*******************************************************
    WARNING WARNING WARNING WARNING WARNING WARNING

We are about to call the curses service known as "getyx".
This service VIOLATES REFERENTIAL TRANSPARENCY!!!  That
is, it violates the principle that, if you don't see a
variable on the left-hand side of an assignment, or with
an ampersand in front of it, you KNOW FOR CERTAIN that
that variable isn't modified.  The ability to know, simply
by looking at a program, what variables get clobbered, or
what other side-effects might occur, is, of course, an
extremely important aspect of good programming, as any
programmer with a multi-cellular brain can tell you.

In fact, more generally, it is critically important to
be able to tell, straightforwardly and trivially, what a
program will do simply by looking at it.  This property is
known as the "WYSIWYG" ("what you see is what you get")
property of programming languages.  (This is not to be
confused with the WYSIWYG property of editors or the
WYSIWYG property of text formatters, which are also very
desirable properties but unrelated to the WYSIWYG property
of programming languages.)

A common way that programming languages can fail to obey
the WYSIWYG property is to provide a method for cloaking
the true linguistic text of the program by means of a
text transformation mechanism such as a macro processor.
Real programmers, needless to say, NEVER use macro
processors for this purpose, even if doing so might make
the program more "convenient" to write.  (The purpose
of a program is not to be convenient to write.  The purpose
is to behave correctly, and obeying the WYSIWYG property
is the single most important thing that a program can do
in furtherance of this goal.)

The manner in which the "getyx" service violates the
WYSIWYG property is, not surprisingly, through a macro.
Someone thought that typing two ampersands was too much
trouble, and that he or she could make the use of this
service more "convenient" by making it a macro that clobbers
the variables named in the macro call.

Furthermore, even the convention that macros will always
be in upper case, a convention that most C programmers
adhere to in recognition of the extremely dangerous
nature of macros, isn't adhered to in this case.

Some operating system vendors' manuals actually describe
this situation as though they think it is a good thing.

For example, the HP-UX manual says

     "This is a macro, so no ampersand is necessary"

        [What does "not necessary" mean in the context
        of programming system documentation?  Is it
        optional?  Is it forbidden?  Furthermore, what
        is the significance of the "so" conjunction in
        that sentence?  Does it logically follow from
        the fact that something is a macro that ampersands
        are never necessary?  Am I missing something?]

and the AT&T System V Release 4 manual says

     "Note that all of these interfaces [getyx and
     several others] are macros and that "&" is not
     necessary before the variables y and x"

and the SunOS 4.1 manual says

     "This is implemented as a macro, so no "&" is
     necessary before the variables"

whereas the correct way to document this would have been

     "This is a macro, and, to save you the two seconds
     that it would take to type two ampersands, we have
     violated referential transparency in it.  To further
     confuse you, we have put it in lower case.  After
     you finish rewriting whatever static analysis tools
     you use at your place of employment so that they
     won't be fooled by the apparent right-hand-side
     usage of what might appear to be an uninitialized
     variable, please make good use of whatever time
     remains of the two seconds that we have saved you."

*******************************************************/


   /* WARNING!!!! LOCALY AND LOCALX MODIFIED BY THIS STATEMENT!!!!! */
                                   /* DANGER BELOW!! */
   /* DANGER TO THE RIGHT!! */ getyx(stdscr, localy, localx) /* DANGER TO THE LEFT!! */
                                   /* DANGER ABOVE!! */

   /* Could we have said just "getyx(stdscr, *y, *x)" ????
      Probably, but who knows what's going in inside that macro.
      We'd better play safe. */

   /* By the way, do we need a semicolon here?  (Note that we
      didn't provide one.)  Exactly what form does this macro
      take?  Does it have a semicolon at the end?  Does it use
      a semicolon between its two assignments, or a comma, or
      what?  No fair peeking at the definition!  Does this
      macro invocation yield a value from its assignment?
      Is it a free-standing statement?  What is it? */

   /* The suspense is killing me!  Here comes the semicolon. */
   ;

   *y = localy;
   *x = localx;
}
#endif


extern void clear_line()
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      int y, x;

      getyx_correctly(&y, &x);
      move(y, 0);
      clrtoeol();
      refresh();
#endif
   }
   else {
      // We may be on a printing terminal, so we can't erase anything.  Just
      // print "XXX" at the end of the line that we want to erase, and start
      // a new line.  This will happen if the user types "C-U", or after any
      // "--More--" line.  We can't make the "--More--" line go away completely,
      // leaving a seamless transcript.  This is the best we can do.
      printf(" XXX\n");
   }
}

extern void rubout()
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      int y, x;
      getyx_correctly(&y, &x);
      if (x > 0) move(y, x-1);
      clrtoeol();
      refresh();
#endif
   }
   else {
      printf("\b \b");    // We hope that this works.
   }
}

extern void erase_last_n(int n)
{
#ifndef NO_CURSES
   if (!sdtty_no_cursor) {
      int y, x;

      getyx_correctly(&y, &x);
      if (y >= n)
         move(y-n, 0);
      else
         /* We would have to erase more than is on the screen.
            This normally means that stuff scrolled off the top and
            got lost. Just home the cursor and clear the whole screen.
            User will get updated text only.  If that isn't
            satisfactory, user can ask for a refresh.  Of course, if
            the sequence is too long to fit on the screen, stuff will
            scroll off the top during the refresh, but there's only
            so much we can do. */
         move(0, 0);

      clrtobot();
      refresh();
   }
#endif
}

extern void put_line(const char the_line[])
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      char c;
      while ((c = *the_line++)) {
         if (c == '\013') {
            int personidx = (*the_line++) & 7;
            int persondir = (*the_line++) & 0xF;

            addch(' ');

            if (ui_options.color_scheme != no_color)
               color_set(color_translations[color_index_list[personidx]].curses_index, 0);

            addch(ui_options.pn1[personidx]);
            addch(ui_options.pn2[personidx]);
            addch(ui_options.direc[persondir]);

            if (ui_options.color_scheme != no_color)
               color_set(1, 0);
         }
         else {
            addch(c);
         }
      }

      refresh();
#endif
   }
   else {
      // Curses is not in use.  But we still might be trying to use colors.

      if (sdtty_no_console) {
         // No funny stuff at all.
         // By leaving "use_escapes_for_drawing_people" at zero, we know
         // that the line will be nothing but ASCII text.
         fputs(the_line, stdout);
      }
      else {
         // We need to watch for escape characters
         // denoting people to be printed in color.

         char c;
         while ((c = *the_line++)) {
            if (c == '\013') {
               int personidx = (*the_line++) & 7;
               int persondir = (*the_line++) & 0xF;

               put_char(' ');

               if (ui_options.color_scheme != no_color) {
                  fputs("\033[1;", stdout);
                  fputs(color_translations[color_index_list[personidx]].vt100_string, stdout);

                  if (ui_options.reverse_video)
                     fputs(";40m", stdout);
                  else
                     fputs(";47m", stdout);
               }

               put_char(ui_options.pn1[personidx]);
               put_char(ui_options.pn2[personidx]);
               put_char(ui_options.direc[persondir]);

               if (ui_options.color_scheme != no_color) {
                  if (ui_options.reverse_video)
                     (void) fputs("\033[0;37;40m", stdout);
                  else
                     (void) fputs("\033[0;30;47m", stdout);
               }
            }
            else
               put_char(c);
         }
      }
   }
}

extern void put_char(int c)
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      addch(c);
      refresh();
#endif
   }
   else {
      putchar(c);
   }
}


extern int get_char()
{
   int nc;

   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      int c = getch();      // A "curses" call.

      // Fen - Fixed curses support for function keys.
      //	Sys V and ncurses don't support modifiers
      //	on most function keys.  Curses supports up
      //	to 64 numbered function keys, but 'sd' can
      //	only recognize the first 16.  The "KEY_*"
      //	constants are defined in <curses.h>.
      //
      if ( c < KEY_MIN ) {		// non-function key
         nc = c;
      }
      else if ( c == KEY_DOWN ) {		// down arrow
         nc = EKEY + 8;
      }
      else if ( c == KEY_UP ) {		// up arrow
         nc = EKEY + 6;
      }
      else if ( c == KEY_LEFT ) {		// left arrow
         nc = EKEY + 5;
      }
      else if ( c == KEY_RIGHT ) {		// right arrow
         nc = EKEY + 7;
      }
      else if ( c == KEY_HOME ) {		// home
         nc = EKEY + 4;
      }
      else if ( c == KEY_BACKSPACE ) {	// backspace
         nc = '\b';
      }
      else if ( c > KEY_F0 && c <= KEY_F(16) ) {	// F1 - F16
         nc = FKEY + c - KEY_F0;
      }
      else if ( c == KEY_DC ) {		// delete character
         nc = EKEY + 14;
      }
      else if ( c == KEY_IC ) {		// insert character
         nc = EKEY + 13;
      }
      else if ( c == KEY_NPAGE ) {		// next page
         nc = EKEY + 2;
      }
      else if ( c == KEY_PPAGE ) {		// previous page
         nc = EKEY + 1;
      }
      else if ( c == KEY_END ) {		// end
         nc = EKEY + 3;
      }
      else if ( c == KEY_SDC ) {		// shift delete character
         nc = SEKEY + 14;
      }
      else if ( c == KEY_SHOME ) {		// shift home
         nc = SEKEY + 4;
      }
      else if ( c == KEY_SIC ) {		// shift insert character
         nc = SEKEY + 13;
      }
      else if ( c == KEY_SLEFT ) {		// shift left arrow
         nc = SEKEY + 5;
      }
      else if ( c == KEY_SRIGHT ) {		// shift right arrow
         nc = SEKEY + 7;
      }
      else {				// no match, return 0
         nc = 0;
      }
#endif
   }
   else {
      csetmode(1);         // Raw, no echo, single-character mode.
      nc = getchar();      // A "stdio" call.
   }

   // Turn control characters (other than ones
   // that are real genuine control characters)
   // into our special encoding.

   if (nc >= 'A'-0100 && nc <= 'Z'-0100) {
      if (nc != '\b' && nc != '\r' &&
          nc != '\n' && nc != '\t')
         nc += CTLLET+0100;
   }

   return nc;
}


extern void get_string(char *dest, int max)
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      echo();
      wgetnstr(stdscr, dest, max);
      noecho();
#endif
   }
   else {
      int size;

      csetmode(0);         /* Regular full-line mode with system echo. */
      (void) fgets(dest, max, stdin);
      size = strlen(dest);

      while (size > 0 && (dest[size-1] == '\n' || dest[size-1] == '\r'))
         dest[--size] = '\000';

      (void) fputs(dest, stdout);
      (void) putchar('\n');
   }
}


extern void ttu_bell()
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      beep();
#endif
   }
   else {
      (void) putchar('\007');
   }
}

/*
 * signal handling
 */

/* ARGSUSED */
static void stop_handler(int n)
{
   if (!sdtty_no_cursor) {
#ifndef NO_CURSES
      // If NO_CURSES is on, we can't compile these lines.
      // But we'll never get here in that case.
      if (curses_initialized) {
         def_prog_mode();
         endwin();
      }
#endif
   }
   else {
      csetmode(0);
   }

   signal(SIGTSTP, SIG_DFL);
   raise(SIGTSTP);			// resend the caught SIGTSTP
}

// Need a forward declaration.
void initialize_signal_handlers();

/* ARGSUSED */
static void cont_handler(int n)
{
   initialize_signal_handlers();	// restore signal handlers

#ifndef NO_CURSES
   if (!sdtty_no_cursor && isendwin() ) {
      refresh();
   }
   else {
      refresh_input();
   }
#else
   refresh_input();
#endif
}

void initialize_signal_handlers()
{
    signal(SIGINT,  term_handler);
    signal(SIGQUIT, term_handler);
    signal(SIGTERM, term_handler);
    signal(SIGCONT, cont_handler);
    signal(SIGTSTP, stop_handler);
}

void iofull::final_initialize()
{
   if (!sdtty_no_console)
      ui_options.use_escapes_for_drawing_people = 1;

   initialize_signal_handlers();
}
