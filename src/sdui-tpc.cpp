/*
 * sdui-tpc.c - helper functions for Mac tty interface for port to DOS.
 * Time-stamp: <93/07/19 19:38:40 wba>
 * Copyright (c) 1990-1995 Stephen Gildea and William B. Ackerman
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Stephen Gildea <gildea@lcs.mit.edu> February 1993
 *
 * Uses console I/O package with djgpp, a port of gcc to DOS on IBM PC.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef DJGPP
#include "pc.h"
#include "keys.h"
#include "gppconio.h"
#else
#include <termios.h>   /* We use this stuff if "-no_cursor" was specified. */
#include <unistd.h>    /* This too. */
#endif
#include "sd.h"


static char *text_ptr;           /* End of text buffer; where we are packing. */
static char text_buffer[10000];  /* This is *NOT* normally padded with a null.
                                    It only gets padded when we need to display it. */
static int lines_in_buffer;  /* Number of "newline" characters in the buffer. */

#if !defined(DJGPP)
static int current_tty_mode = 0;

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
#endif


void iofull::final_initialize()
{
   /* If no "-no_graphics" switch was not given, and our run-time
      system supports it, switch over to the "pointy triangles"
      for drawing pictures.  This only works on DJGPP. */

#if defined(DJGPP)
   if (ui_options.no_graphics < 2)
      ui_options.direc = "?\020?\021????\036?\037?????";
#endif
}

void iofull::display_help()
{
   printf("-lines <N>                  assume this many lines on the screen\n");
   printf("-no_cursor                  do not use screen management functions at all\n");
   printf("-journal <filename>         echo input commands to journal file\n");
}

extern void ttu_set_window_title(char s[])
{
}

bool iofull::help_manual() { return false; }


extern void ttu_initialize()
{
   // Set the default value if the user hasn't explicitly set something.
   if (sdtty_screen_height <= 0) sdtty_screen_height = 25;

#ifdef DJGPP
   gppconio_init();
#endif

   /* This code uses "no_cursor" rather than "no_console"
      to direct what it does.  So, if "no_console" is on,
      we take appropriate action. */

   sdtty_no_cursor |= sdtty_no_console;

   text_ptr = text_buffer;
   lines_in_buffer = 0;
}

void iofull::terminate()
{
#if !defined(DJGPP)
   csetmode(0);   /* Restore normal input mode. */
#endif
}

extern int get_lines_for_more()
{
   return sdtty_screen_height-1;
}

extern void clear_line()
{
#ifdef DJGPP
   int yp = wherey();
   gotoxy(1, yp);
   clreol();
#else
   printf(" XXX\n");
#endif
}

extern void rubout()
{
   printf("\b \b");
}

extern void erase_last_n(int n)
{
#ifdef DJGPP
   char *p = text_buffer;
   int c = 0;

   if (!sdtty_no_cursor) {
      if (lines_in_buffer > n) {
         lines_in_buffer = lines_in_buffer-n;

         /* Now point p just after the "lines_in_buffer"th newline. */
         while (p != text_ptr) {   /* Just being careful. */
            if (*p++ == '\n') {
               /* Found a "newline", count same. */
               c++;
               if (c == lines_in_buffer)
                  break;      /* We found the new ending spot. */
            }
         }
      }
      else {
         lines_in_buffer = 0;
      }

      /* Now "p" has the new end. */

      text_ptr = p;
      *p = '\0';

      gotoxy(1, 1);
      clrscr();
      printf("%s", text_buffer);
   }
   /* Otherwise, we take no action at all. */
#endif
}

static void pack_in_buffer(char c)
{
   *text_ptr++ = c;
   if (c == '\n') {
      if (lines_in_buffer >= sdtty_screen_height-1) {
         /* We need to throw away a line at the start of the buffer. */
         char *p = text_buffer;
         char *sp = text_buffer;

         while (p != text_ptr) {   /* Just being careful. */
            if (*p++ == '\n') {
               /* Found the new text, copy it down. */
               while (p != text_ptr) *sp++ = *p++;
               text_ptr = sp;
               break;
            }
         }
      }
      else
         lines_in_buffer++;
   }
}

extern void put_line(const char the_line[])
{
   if (!sdtty_no_cursor) {
      const char *p = the_line;
      char c;
      while ((c = *p++))
         pack_in_buffer(c);
   }

   (void) fputs(the_line, stdout);
}

extern void put_char(int c)
{
   if (!sdtty_no_cursor)
      pack_in_buffer((char) c);

   (void) putchar(c);
}


#if defined(DJGPP)
static short int altletter_translate[] = {
   ALTLET+'Q',     /* 110 */
   ALTLET+'W',
   ALTLET+'E',
   ALTLET+'R',
   ALTLET+'T',
   ALTLET+'Y',
   ALTLET+'U',
   ALTLET+'I',
   ALTLET+'O',
   ALTLET+'P',
   ' ',
   ' ',
   ' ',
   ' ',
   ALTLET+'A',
   ALTLET+'S',
   ALTLET+'D',     /* 120 */
   ALTLET+'F',
   ALTLET+'G',
   ALTLET+'H',
   ALTLET+'J',
   ALTLET+'K',
   ALTLET+'L',
   ' ',
   ' ',
   ' ',
   ' ',
   ' ',
   ALTLET+'Z',
   ALTLET+'X',
   ALTLET+'C',
   ALTLET+'V',
   ALTLET+'B',     /* 130 */
   ALTLET+'N',
   ALTLET+'M',
   ' ',
   ' ',
   ' ',
   ' ',
   ' ',
   ' ',
   ' ',
   ' ',
   FKEY+1,
   FKEY+2,
   FKEY+3,
   FKEY+4,
   FKEY+5,
   FKEY+6,     /* 140 */
   FKEY+7,
   FKEY+8,
   FKEY+9,
   FKEY+10,
   ' ',
   ' ',
   EKEY+4, /* K_Home */
   EKEY+6, /* K_Up */
   EKEY+1, /* K_PageUp */
   ' ',
   EKEY+5, /* K_Left */
   ' ',
   EKEY+7, /* K_Right */
   ' ',
   EKEY+3, /* K_End */
   EKEY+8, /* K_Down   150 */
   EKEY+2, /* K_PageDown */
   EKEY+13,/* K_Insert */
   EKEY+14,/* K_Delete */
   SFKEY+1,
   SFKEY+2,
   SFKEY+3,
   SFKEY+4,
   SFKEY+5,
   SFKEY+6,
   SFKEY+7,
   SFKEY+8,
   SFKEY+9,
   SFKEY+10,
   CFKEY+1,
   CFKEY+2,
   CFKEY+3,  /* 160 */
   CFKEY+4,
   CFKEY+5,
   CFKEY+6,
   CFKEY+7,
   CFKEY+8,
   CFKEY+9,
   CFKEY+10,
   AFKEY+1,
   AFKEY+2,
   AFKEY+3,
   AFKEY+4,
   AFKEY+5,
   AFKEY+6,
   AFKEY+7,
   AFKEY+8,
   AFKEY+9,  /* 170 */
   AFKEY+10};
#endif


extern int get_char()
#if defined(DJGPP)
{
   int n;

   if (diagnostic_mode) return getchar();

   do {
      fflush(stdout);
      n = getxkey();
   } while (n == EOF);   /* busy wait (EOF means no character yet) */

   if (n == 0) {     /* Escape for function keys on some keyboards. */
      do {           /* Get the indicator character. */
         fflush(stdout);
         n = getxkey();
      } while (n == EOF);

      n += 0x100;
      goto translate_special;
   }
   else if (n >= 128)
      goto translate_special;

   if (n >= 'A'-0100 && n <= 'Z'-0100) {
      if (n != '\b' && n != '\r' &&
          n != '\n' && n != '\t')
         n += CTLLET+0100;
   }

   return n;

 translate_special:

   if (n == K_Delete)           /* Delete */
      n = '\177';
   else if (n >= K_Alt_Q && n <= K_Alt_F10)
      n = altletter_translate[n-K_Alt_Q];
   else if (n >= K_F11 && n <= K_F12)
      n += FKEY+11-K_F11;                             /* F11..F12 */
   else if (n >= K_Shift_F11 && n <= K_Shift_F12)
      n += SFKEY+11-K_Shift_F11;                      /* shift F11..F12 */
   else if (n >= K_Control_F11 && n <= K_Control_F12)
      n += CFKEY+11-K_Control_F11;                    /* control F11..F12 */
   else if (n >= K_Alt_F11 && n <= K_Alt_F12)
      n += AFKEY+11-K_Alt_F11;                        /* alt F11..F12 */
   else if (n == K_EPageUp)   n = EKEY+1;
   else if (n == K_EPageDown) n = EKEY+2;
   else if (n == K_EEnd)      n = EKEY+3;
   else if (n == K_EHome)     n = EKEY+4;
   else if (n == K_ELeft)     n = EKEY+5;
   else if (n == K_EUp)       n = EKEY+6;
   else if (n == K_ERight)    n = EKEY+7;
   else if (n == K_EDown)     n = EKEY+8;
   else if (n == K_EInsert)   n = EKEY+13;
   else if (n == K_EDelete)   n = EKEY+14;
   else n = ' ';

   return n;
}
#else
{
   int c;

   csetmode(1);         /* Raw, no echo, single-character mode. */

   for ( ;; ) {
      c = getchar();
      if (c != '\r') break;
   }

   if (c >= 'A'-0100 && c <= 'Z'-0100) {
      if (c != '\b' && c != '\r' &&
          c != '\n' && c != '\t')
         c += CTLLET+0100;
   }

   return c;
}
#endif

extern void get_string(char *dest, int max)
{
   int size;

#if !defined(DJGPP)
   csetmode(0);         /* Regular full-line mode with system echo. */
#endif
   (void) fgets(dest, max, stdin);
   size = strlen(dest);

   while (size > 0 && (dest[size-1] == '\n' || dest[size-1] == '\r'))
      dest[--size] = '\000';

   (void) fputs(dest, stdout);
   (void) putchar('\n');
}

extern void ttu_bell()
{
   (void) putchar('\007');
}
