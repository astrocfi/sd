/*
 * sdui-wincon.cpp
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
 * Uses "console application" mechanism of Visual C++.
 */

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "basetype.h"
#include "sdui.h"



static HANDLE consoleStdout, consoleStdin;
static char SavedWindowTitle[201];
static CONSOLE_SCREEN_BUFFER_INFO globalconsoleInfo;
static DWORD oldMode;
static DWORD newMode;


static char *my_pn1;
static char *my_pn2;
static char *my_directions;

extern void ttu_final_option_setup()
{
   my_pn1 = ui_options.pn1;
   my_pn2 = ui_options.pn2;

   /* Install the pointy triangles. */

   if (ui_options.no_graphics < 2)
      ui_options.direc = "?\020?\021????\036?\037?????";

   my_directions = ui_options.direc;

   if (!no_console) ui_options.use_escapes_for_drawing_people = 1;
}

extern void ttu_display_help(void)
{
   printf("-lines <N>                  assume this many lines on the screen\n");
   printf("-no_console                 do not do any fancy screen processing -- use this\n");
   printf("                                 when input or output is redirected\n");
   printf("-no_cursor                  do not use screen management functions\n");
   printf("-journal <filename>         echo input commands to journal file\n");
}


extern void ttu_set_window_title(char s[])
{
   (void) SetConsoleTitle(s);
}


BOOL WINAPI control_c_handler(DWORD ControlInfo)
{
   if (ControlInfo == CTRL_C_EVENT) {
      DWORD numRead;
      INPUT_RECORD inputRecord;

      inputRecord.EventType = KEY_EVENT;
      inputRecord.Event.KeyEvent.bKeyDown = TRUE;
      inputRecord.Event.KeyEvent.wRepeatCount = 1;
      inputRecord.Event.KeyEvent.wVirtualKeyCode = 'C';
      inputRecord.Event.KeyEvent.wVirtualScanCode = 46;
      inputRecord.Event.KeyEvent.uChar.AsciiChar = 'c';
      inputRecord.Event.KeyEvent.dwControlKeyState = LEFT_CTRL_PRESSED;
      WriteConsoleInput(consoleStdin, &inputRecord, 1, &numRead);
      return TRUE;
   }
   else {
      return FALSE;
   }
}


static WORD text_color;



/* These will get "background_color" added in during initialization. */

static WORD couple_colors[9] = {
   FOREGROUND_RED                     | FOREGROUND_INTENSITY,  /* red */
   FOREGROUND_RED                     | FOREGROUND_INTENSITY,  /* red */
   FOREGROUND_GREEN                   | FOREGROUND_INTENSITY,  /* green */
   FOREGROUND_GREEN                   | FOREGROUND_INTENSITY,  /* green */
   FOREGROUND_BLUE                    | FOREGROUND_INTENSITY,  /* blue */
   FOREGROUND_BLUE                    | FOREGROUND_INTENSITY,  /* blue */
   FOREGROUND_RED  | FOREGROUND_GREEN | FOREGROUND_INTENSITY,  /* yellow */
   FOREGROUND_RED  | FOREGROUND_GREEN | FOREGROUND_INTENSITY,  /* yellow */
   FOREGROUND_RED                     | FOREGROUND_INTENSITY}; /* red */

static WORD person_colors[2] = {
   FOREGROUND_BLUE | FOREGROUND_INTENSITY,  /* blue */
   FOREGROUND_RED  | FOREGROUND_INTENSITY}; /* red */

static WORD pastel_person_colors[2] = {
   FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,  /* bletcherous blue */
   FOREGROUND_RED  | FOREGROUND_BLUE  | FOREGROUND_INTENSITY}; /* putrid pink */



extern void ttu_initialize(void)
{
   DWORD numWrite;
   int i;

   (void) GetConsoleTitle(SavedWindowTitle, 200);

   if (no_console) return;

   consoleStdin = GetStdHandle(STD_INPUT_HANDLE);
   consoleStdout = GetStdHandle(STD_OUTPUT_HANDLE);

   if (consoleStdout == consoleStdin) {
      int err = GetLastError();
      fprintf(stderr, "GetStdHandle failed: %d.\n", err);
      ExitProcess(err);
   }

   if (!GetConsoleMode(consoleStdin, &oldMode)) {
      int err = GetLastError();
      if (err == ERROR_INVALID_HANDLE) {
         no_console = 1;
         return;
      }
      fprintf(stderr, "GetConsoleMode failed: %d.\n", err);
      ExitProcess(err);
   }

   newMode = oldMode &
      ~ENABLE_LINE_INPUT &
      ~ENABLE_ECHO_INPUT &
      ~ENABLE_WINDOW_INPUT &
      ~ENABLE_MOUSE_INPUT;

   if (!SetConsoleMode(consoleStdin, newMode)) {
      int err = GetLastError();
      if (err == ERROR_INVALID_HANDLE) {
         no_console = 1;
         return;
      }
      fprintf(stderr, "SetConsoleMode failed: %d.\n", err);
      ExitProcess(err);
   }

   WORD background_color;

   /* Get information about the "buffer", that is, the "pad".  The window itself,
      and all resizing thereof, is irrelevant.  It will take care of itself, the way
      it does in any decent operating system (unlike some that I've had to deal with.) */

   (void) GetConsoleScreenBufferInfo(consoleStdout, &globalconsoleInfo);

   if (ui_options.reverse_video)
      background_color = 0;
   else {
      /* If doing color_by_couple or color_by_corner, make the backround light
         gray instead of white, because yellow doesn't show up well against white. */
      if (ui_options.no_color == 2 || ui_options.no_color == 3 || ui_options.no_intensify)
         background_color = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED;
      else
         background_color = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED |
            BACKGROUND_INTENSITY;
   }

   if (!ui_options.no_intensify) {
      if (ui_options.reverse_video)
         text_color = FOREGROUND_BLUE | FOREGROUND_GREEN |
            FOREGROUND_RED | FOREGROUND_INTENSITY;
      else
         text_color = background_color;
   }
   else {
      if (ui_options.reverse_video)
         text_color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
      else
         text_color = background_color;
   }

   for (i=0 ; i<9 ; i++) couple_colors[i] |= background_color;
   for (i=0 ; i<2 ; i++) person_colors[i] |= background_color;
   for (i=0 ; i<2 ; i++) pastel_person_colors[i] |= background_color;

   if (ui_options.reverse_video) background_color |= FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;

   COORD coord;
   coord.X = 0;
   coord.Y = 0;

   //globalconsoleInfo.dwCursorPosition.X
   //globalconsoleInfo.dwCursorPosition.Y

   (void) FillConsoleOutputAttribute(consoleStdout,
                                     background_color,
                                     globalconsoleInfo.dwSize.X *
                                     (globalconsoleInfo.dwSize.Y-globalconsoleInfo.dwCursorPosition.Y)
                                     -globalconsoleInfo.dwCursorPosition.X,
                                     globalconsoleInfo.dwCursorPosition, &numWrite);

   (void) SetConsoleTextAttribute(consoleStdout, text_color);

   (void) SetConsoleCtrlHandler(&control_c_handler, TRUE);
}


extern void ttu_terminate(void)
{
   if (!no_console) {

      CONSOLE_SCREEN_BUFFER_INFO finalconsoleInfo;

      DWORD numWrite;
      COORD coord;
      coord.X = 0;
      coord.Y = 0;

      // Find out what line we have advanced to.
      (void) GetConsoleScreenBufferInfo(consoleStdout, &finalconsoleInfo);

      // Paint the rest of the screen to the original color.
      (void) FillConsoleOutputAttribute(consoleStdout,
                                        globalconsoleInfo.wAttributes,
                                        finalconsoleInfo.dwSize.X *
                                        (finalconsoleInfo.dwSize.Y-finalconsoleInfo.dwCursorPosition.Y)
                                        -finalconsoleInfo.dwCursorPosition.X,
                                        finalconsoleInfo.dwCursorPosition, &numWrite);


      /*

      (void) FillConsoleOutputAttribute(consoleStdout,
         globalconsoleInfo.wAttributes, globalconsoleInfo.dwSize.X * globalconsoleInfo.dwSize.Y,
         coord, &numWrite);
      */

      (void) SetConsoleTextAttribute(consoleStdout, globalconsoleInfo.wAttributes);
      (void) SetConsoleMode(consoleStdin, oldMode);
      (void) SetConsoleCtrlHandler(&control_c_handler, FALSE);
   }

   (void) SetConsoleTitle(SavedWindowTitle);
}

extern int get_lines_for_more(void)
{
   return screen_height;
}

extern void clear_line(void)
{
   if (!no_console) {
      DWORD numWrite;
      CONSOLE_SCREEN_BUFFER_INFO myconsoleInfo;

      (void) GetConsoleScreenBufferInfo(consoleStdout, &myconsoleInfo);
      myconsoleInfo.dwCursorPosition.X = 0;
      (void) SetConsoleCursorPosition(consoleStdout, myconsoleInfo.dwCursorPosition);
      (void) FillConsoleOutputCharacter(consoleStdout, ' ', myconsoleInfo.dwSize.X,
                                        myconsoleInfo.dwCursorPosition,
                                        &numWrite);
   }
   else
      printf(" XXX\n");
}

extern void rubout(void)
{
   if (!no_console) {
      DWORD numWrite;
      CONSOLE_SCREEN_BUFFER_INFO myconsoleInfo;

      (void) GetConsoleScreenBufferInfo(consoleStdout, &myconsoleInfo);
      myconsoleInfo.dwCursorPosition.X--;
      (void) SetConsoleCursorPosition(consoleStdout, myconsoleInfo.dwCursorPosition);
      (void) FillConsoleOutputCharacter(consoleStdout, ' ', 1,
                                        myconsoleInfo.dwCursorPosition,
                                        &numWrite);
   }
   else
      printf("\b \b");
}

extern void erase_last_n(int n)
{
   if (!no_console && !no_cursor) {
      DWORD numWrite;
      CONSOLE_SCREEN_BUFFER_INFO myconsoleInfo;
      int delta = n;

      (void) GetConsoleScreenBufferInfo(consoleStdout, &myconsoleInfo);

      if (myconsoleInfo.dwCursorPosition.Y < delta)
         delta = myconsoleInfo.dwCursorPosition.Y;

      myconsoleInfo.dwCursorPosition.X = 0;
      myconsoleInfo.dwCursorPosition.Y -= delta;

      (void) SetConsoleCursorPosition(consoleStdout, myconsoleInfo.dwCursorPosition);
      (void) FillConsoleOutputCharacter(consoleStdout, ' ', myconsoleInfo.dwSize.X * delta,
                                        myconsoleInfo.dwCursorPosition,
                                        &numWrite);
   }
}

extern void put_line(const char the_line[])
{
   if (!no_console) {
      char c;

      /* We need to watch for escape characters denoting people
         to be printed in a particularly pretty way. */

      while ((c = *the_line++)) {
         if (c == '\013') {
            int c1 = *the_line++;
            int c2 = *the_line++;
            WORD color;

            put_char(' ');

            if (ui_options.no_color != 1) {
               if (ui_options.no_color == 2)
                  color = couple_colors[c1 & 7];
               else if (ui_options.no_color == 3)
                  color = couple_colors[(c1 & 7) + 1];
               else
                  color = (ui_options.pastel_color ? pastel_person_colors : person_colors)[c1 & 1];

               (void) SetConsoleTextAttribute(consoleStdout, color);
            }

            put_char(my_pn1[c1 & 7]);
            put_char(my_pn2[c1 & 7]);
            put_char(my_directions[c2 & 017]);

            /* Set back to plain "white". */

            if (ui_options.no_color != 1)
               (void) SetConsoleTextAttribute(consoleStdout, text_color);
         }
         else
            put_char(c);
      }
   }
   else {
      (void) fputs(the_line, stdout);
   }
}

extern void put_char(int c)
{
   DWORD junk;
   char cc = c;

   if (!no_console) {
      if (c == '\n') {
         CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

         /* Get information about the "buffer", that is, the "pad".  The window itself,
            and all resizing thereof, is irrelevant.  It will take care of itself, the way
            it does in any decent operating system (unlike some that I've had to deal with.) */

         (void) GetConsoleScreenBufferInfo(consoleStdout, &consoleInfo);
         consoleInfo.dwCursorPosition.X = 0;   /* Cursor position within the pad. */

         if (consoleInfo.dwSize.Y-1 != consoleInfo.dwCursorPosition.Y)
            consoleInfo.dwCursorPosition.Y++;
         else {

            /* We have run off the end of the pad. */

            SMALL_RECT scrollRect;    /* The portion of the pad that we are
                                         going to scroll in (all of it.) */
            COORD coord;              /* How we are going to move it (up one position.) */
            CHAR_INFO consoleFill;    /* What we are going to put into vacated area
                                         (blanks, with same color as everything else.) */

            scrollRect.Top = 0;
            scrollRect.Left = 0;
            scrollRect.Bottom = consoleInfo.dwSize.Y-1;
            scrollRect.Right = consoleInfo.dwSize.X-1;
            coord.X = 0;
            coord.Y = -1;
            consoleFill.Attributes = consoleInfo.wAttributes;
            consoleFill.Char.AsciiChar = ' ';
            (void) ScrollConsoleScreenBuffer(consoleStdout, &scrollRect, 0, coord, &consoleFill);
         }

         (void) SetConsoleCursorPosition(consoleStdout, consoleInfo.dwCursorPosition);
      }
      else
         WriteFile(consoleStdout, &cc, 1, &junk, (LPOVERLAPPED) 0);
   }
   else {
      (void) putchar(c);
   }
}


extern int get_char(void)
{
   DWORD numRead;
   INPUT_RECORD inputRecord;

   if (no_console)
      return getchar();    /* A "stdio" call. */

   for ( ;; ) {
      ReadConsoleInput(consoleStdin, &inputRecord, 1, &numRead);

      if (inputRecord.EventType == KEY_EVENT && inputRecord.Event.KeyEvent.bKeyDown) {
         DWORD ctlbits = inputRecord.Event.KeyEvent.dwControlKeyState & (~NUMLOCK_ON | SCROLLLOCK_ON);
         unsigned int c = inputRecord.Event.KeyEvent.uChar.AsciiChar;
         int key = inputRecord.Event.KeyEvent.wVirtualKeyCode;
         int npdigit;

         /* Canonicalize the control bits -- we don't distinguish
               between left ALT and right ALT, for example. */

         if (ctlbits & CAPSLOCK_ON) ctlbits |= SHIFT_PRESSED;
         if (ctlbits & RIGHT_CTRL_PRESSED) ctlbits |= LEFT_CTRL_PRESSED;
         if (ctlbits & RIGHT_ALT_PRESSED) ctlbits |= LEFT_ALT_PRESSED;
         ctlbits &= ~(CAPSLOCK_ON | RIGHT_CTRL_PRESSED | RIGHT_ALT_PRESSED);

         if (key >= 'A' && key <= 'Z') {    /* Letter key was pressed. */
            ctlbits &= ~SHIFT_PRESSED;
            if (ctlbits == 0) return c;
            else if (ctlbits == LEFT_CTRL_PRESSED)
               return CTLLET + key;
            else if (ctlbits == LEFT_ALT_PRESSED)
               return ALTLET + key;
            else if (ctlbits == (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED))
               return CTLALTLET + key;
            else
               continue;
         }
         else if (key >= '0' && key <= '9') {       /* Digit key. */
            ctlbits &= ~SHIFT_PRESSED;
            if (ctlbits == 0) return c;
            else if (ctlbits == LEFT_CTRL_PRESSED)
               return key-'0'+CTLDIG;
            else if (ctlbits == LEFT_ALT_PRESSED)
               return key-'0'+ALTDIG;
            else if (ctlbits == (LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED))
               return key-'0'+CTLALTDIG;
            else
               continue;   /* Don't know what it is. */
         }
         else if (key == 0x10 || key == 0x11 || key == 0x12 || key == 0x14)
            continue;      /* Just an actual shift/control/alt key. */
         else if (key >= 0x70 && key <= 0x7B) {    /* Function key. */
            if (ctlbits == 0)                      return key-0x6F+FKEY;
            else if (ctlbits == SHIFT_PRESSED)     return key-0x6F+SFKEY;
            else if (ctlbits == LEFT_CTRL_PRESSED) return key-0x6F+CFKEY;
            else if (ctlbits == LEFT_ALT_PRESSED)  return key-0x6F+AFKEY;
            else if (ctlbits == (LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED)) return key-0x6F+CAFKEY;
            else
               continue;   /* Don't know what it is. */
         }
         else if ((ctlbits & ENHANCED_KEY) && key >= 0x20 && key <= 0x20+15) {
            /* "Enhanced" key. */
            if (ctlbits == ENHANCED_KEY)                          return key-0x20+EKEY;
            else if (ctlbits == (ENHANCED_KEY|SHIFT_PRESSED))     return key-0x20+SEKEY;
            else if (ctlbits == (ENHANCED_KEY|LEFT_CTRL_PRESSED)) return key-0x20+CEKEY;
            else if (ctlbits == (ENHANCED_KEY|LEFT_ALT_PRESSED))  return key-0x20+AEKEY;
            else if (ctlbits == (ENHANCED_KEY|LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED)) return key-0x20+CAEKEY;
            else
               continue;   /* Don't know what it is. */
         }
         else if ((key >= 0xBA && key <= 0xBF) ||     /* Random other keys ... */
                  (key == 0x20) ||                    /* Space. */
                  (key == 0x09) ||                    /* Tab. */
                  (key == 0x08) ||                    /* Backspace. */
                  (key == 0x0D) ||                    /* Enter (CR). */
                  (key == 0x1B) ||                    /* Escape. */
                  (key == 0xC0) ||                    /* Random other keys ... */
                  (key >= 0xDB && key <= 0xDE)) {     /* Random other keys ... */
            ctlbits &= ~SHIFT_PRESSED;
            if (ctlbits == 0) return c;
            else continue;
         }
         else if (key == 0x23) npdigit = '1';    /* Numeric keypad -- */
         else if (key == 0x28) npdigit = '2';    /* just treat as normal digits. */
         else if (key == 0x22) npdigit = '3';
         else if (key == 0x25) npdigit = '4';
         else if (key == 0x0C) npdigit = '5';
         else if (key == 0x27) npdigit = '6';
         else if (key == 0x24) npdigit = '7';
         else if (key == 0x26) npdigit = '8';
         else if (key == 0x21) npdigit = '9';
         else if (key == 0x2D) npdigit = '0';
         else if (ctlbits == 0 && isprint(c))
            return c;
         else
            continue;

         /* If we get here, this is a numeric keypad press. */

         ctlbits &= ~SHIFT_PRESSED;
         if (ctlbits == 0) return npdigit;
         else if (ctlbits == LEFT_CTRL_PRESSED)
            return npdigit-'0'+CTLNKP;
         else if (ctlbits == LEFT_ALT_PRESSED)
            return npdigit-'0'+ALTNKP;
         else if (ctlbits == (LEFT_CTRL_PRESSED|LEFT_ALT_PRESSED))
            return npdigit-'0'+CTLALTNKP;
         else
            continue;   /* Don't know what it is. */
      }
   }
}

extern void get_string(char *dest, int max)
{
   int size = 0;

   for ( ;; ) {
      int key = get_char();

      if (key == (CTLLET + 'U')) {
         size = 0;
         clear_line();
         continue;
      }
      else if (key > 0x7F) continue;    /* Ignore special function keys and such. */
      else if (key == '\r' || key == '\n') break;
      else if ((key == '\b') || (key == 0x7F)) {
         if (size > 0) {
            size--;
            rubout();
         }
         continue;
      }

      if (key > 128) continue;   /* This was a function key press. */

      put_char(key);
      dest[size++] = key;
   }

   dest[size] = '\0';
   put_line("\n");
}

extern void ttu_bell(void)
{
   (void) MessageBeep(MB_ICONQUESTION);
}


extern void initialize_signal_handlers(void)
{
}
