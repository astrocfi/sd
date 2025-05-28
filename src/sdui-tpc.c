/*
 * sdui-tpc.c - helper functions for Mac tty interface for port to DOS.
 * Time-stamp: <93/07/19 19:38:40 wba>
 * Copyright 1993 Stephen Gildea
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

#include "sdui-ttu.h"
#include <stdio.h>
#include <gppconio.h>

static int screen_height = 25;
static int no_cursor = 0;
static char *text_ptr;           /* End of text buffer; where we are packing. */
static char text_buffer[10000];  /* This is *NOT* normally padded with a null.  It only gets padded
                                             when we need to display it. */
static int lines_in_buffer;  /* Number of "newline" characters in the buffer. */

extern void ttu_process_command_line(int *argcp, char ***argvp)
{
   int i;
   int argno = 1;
   char **argv = *argvp;

   while (argno < (*argcp)) {
      if (strcmp(argv[argno], "-no_cursor") == 0) {
         no_cursor = 1;
      }
      else if (strcmp(argv[argno], "-screensize") == 0 && argno+1 < (*argcp)) {
         screen_height = atoi(argv[argno+1]);
         (*argcp) -= 2;      /* Remove two arguments from the list. */
         for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i+1];
         continue;
      }
      else {
         argno++;
         continue;
      }

      (*argcp)--;      /* Remove this argument from the list. */
      for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i];
   }
}

extern void ttu_initialize(void)
{
   gppconio_init();
   text_ptr = text_buffer;
   lines_in_buffer = 0;
}

extern void ttu_terminate(void)
{
}

extern int get_lines_for_more(void)
{
   return screen_height;
}

extern void clear_line(void)
{
   int yp = wherey();
   gotoxy(1, yp);
   clreol();
}

extern void rubout(void)
{
   printf("\b \b");
}

extern void erase_last_n(int n)
{
   char *p = text_buffer;
   char *sp = text_buffer;   /* "Start" pointer for display. */
   int c = 0;

   if (!no_cursor) {
      if (lines_in_buffer > n) {
         lines_in_buffer = lines_in_buffer-n;

         /* Now point p just after the "lines_in_buffer"th newline,
            and sp 25 (or so) lines back. */
         while (p != text_ptr) {   /* Just being careful. */
            if (*p++ == '\n') {
               /* Found a "newline", count same. */
               c++;
               if (c == lines_in_buffer-screen_height)
                  sp = p;     /* We found the spot 25 lines back. */
               else if (c == lines_in_buffer)
                  break;      /* We found the new ending spot. */
            }
         }
      }
      else {
         lines_in_buffer = 0;
      }

      /* Now "p" has the new end, and "sp" tells where to start display.  That is,
         if the buffer has more than 25 lines in it, "sp" points to the beginning
         of the last 25 lines. */

      text_ptr = p;
      *p = '\0';

      gotoxy(1, 1);
      clrscr();
      printf("%s", sp);
   }
   /* Otherwise, we take no action at all. */
}

extern void put_line(char the_line[])
{
   if (!no_cursor) {
      char *p = the_line;
      char c;
      while (c = *p++) {
         *text_ptr++ = c;
         if (c == '\n') lines_in_buffer++;
      }

      (void) fputs(the_line, stdout);
   }
   else {
      (void) fputs(the_line, stdout);
   }
}

extern void put_char(int c)
{
   if (!no_cursor) {
      *text_ptr++ = (char) c;
      if (c == '\n') lines_in_buffer++;
   }

   (void) putchar(c);
}

extern int get_char(void)
{
    int n;

    do {
        fflush(stdout);
        n = getch();
        if (n > 127) {
    	if (n == 339) /* Delete */
    	    n = '\177';
    	else
    	    n = ' ';
        }
    } while (n == EOF);   /* busy wait (EOF means no character yet) */

    return n;
}

extern void get_string(char *dest)
{
    gets(dest);
}

extern void bell(void)
{
    (void) putchar('\007');
}

extern void initialize_signal_handlers(void)
{
}
