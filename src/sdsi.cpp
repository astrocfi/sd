/* SD -- square dance caller's helper.

    Copyright (C) 1990-2000  William B. Ackerman.

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

    This is for version 34. */

/* This defines the following functions:
   general_initialize
   generate_random_number
   hash_nonrandom_number
   get_mem
   get_mem_gracefully
   get_more_mem
   get_more_mem_gracefully
   free_mem
   get_date
   open_file
   write_file
   close_file
   print_line
   init_error
   parse_level
   read_from_call_list_file
   write_to_call_list_file
   close_call_list_file
   install_outfile_string
   get_first_session_line
   get_next_session_line
   prepare_to_read_menus
   initialize_misc_lists
   open_session
   process_session_info
   open_call_list_file
   open_accelerator_region
   get_accelerator_line
   close_init_file
   final_exit
   open_database
   read_8_from_database
   read_16_from_database
   close_database
   parse_number

and the following external variables:

   session_index
   random_number
   database_filename
   new_outfile_string
   call_list_string
   call_list_file
*/

/* You should compile this file (and might as well compile all the others
   too) with some indicator symbol defined that tells what language system
   semantics are to be provided.  This program requires at least POSIX.
   A better random number generator mechanism (48 bits) is provided under
   System 5 (_SYS5_SOURCE), OSF (_AES_SOURCE), XOPEN (_XOPEN_SOURCE), or
   some proprietary systems, so you should define such a symbol if your
   system provides those semantics.  If not, you should just turn on
   _POSIX_SOURCE.  Normally, this is done on the compiler invocation line
   (in the Makefile, or whatever) with some incantation like "-D_AES_SOURCE".

   We recommend _AES_SOURCE over _XOPEN_SOURCE, since the latter doesn't
   seem to recognize that 20th century programmers use include files with
   prototypes for library functions in them.  Under OSF/AES, the file
   stdlib.h has the necessary stuff.  Under XOPEN, you are expected to
   copy the prototypes by hand from the printed manual into your program.
   For those who absolutely must use XOPEN, we have done the necessary
   copying below.

   On HP-UX, we recommend turning on _AES_SOURCE for the reason given
   above.  You could also turn on _HPUX_SOURCE to get the routines,
   if you have no aversion to proprietary system semantics.

   On SUN-OS, giving no symbol will get SUN-OS semantics, which gets the
   48-bit routines, if you have no aversion to proprietary system semantics.
   This is what the "defined(sun)" is for.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>  /* for tolower */
#include <time.h>

/* We take pity on those poor souls who are compelled to use XOPEN,
   an otherwise fine standard, that doesn't put prototypes
   into stdlib.h. */

#if defined(_XOPEN_SOURCE)
   /* We have taken the liberty of translating the prototypes
      appearing in the XOPEN manual into ANSI C.  We can't
      imagine why anyone would ever use the prototypes
      in the prehistoric nomenclature. */
#ifdef __cplusplus
extern "C" {
#endif
extern void srand48(long int);
extern long int lrand48(void);
#ifdef __cplusplus
}
#endif
#endif

#if defined(_SYS5_SOURCE) || defined(_XOPEN_SOURCE) || defined(_AES_SOURCE) || defined(sun)
#define HAVE_RAND48
#endif

/* We also take pity on those poor souls who are compelled to use
   compilation systems that claim to be POSIX compliant, but
   aren't really, and do not have unistd.h. */

#if defined(_POSIX_SOURCE) || defined(sun)
#define POSIX_STYLE
#endif

#ifdef POSIX_STYLE
#include <unistd.h>
#endif

/* Or those who are otherwise compelled to use
    troglodyte development environments.
    gcc -traditional doesn't define __STDC__ */

#if __STDC__ || defined(sun)
#include <stdlib.h>
#else

#ifdef __cplusplus
extern "C" {
#endif
extern void free(void *ptr);
extern char *malloc(unsigned int siz);
extern char *realloc(char *oldp, unsigned int siz);
extern void srand48(long int);
extern long int lrand48(void);
#ifdef __cplusplus
}
#endif

#endif

#if defined(WIN32)

#ifdef __cplusplus
extern "C" {
#endif
extern void srand(long int);
extern long int rand(void);
#ifdef __cplusplus
}
#endif

#endif

#if !defined(sun) && (!__STDC__ || defined(MSDOS))
extern char *strerror(int);
#endif

/* Despite all our efforts, some systems just can't be bothered ... */
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

/* I think we have everything now.  Isn't portability fun?  Before people
   started taking these standards as semi-seriously as they do now, it was
   MUCH HARDER to make a program portable than what you just saw. */

#include "sd.h"
#include "paths.h"


/* These variables are external. */


/* Used for controlling the session file.  When index is nonzero,
   the session file is in use, and the final state should be written back
   to it at that line. */

int session_index = 0;        /* If this is nonzero, we have opened a session. */

/* e1 = page up
   e2 = page down
   e3 = end
   e4 = home
   e5 = left arrow
   e6 = up arrow
   e7 = right arrow
   e8 = down arrow
   e13 = insert
   e14 = delete */


int random_number;
char *database_filename = DATABASE_FILENAME;
char *new_outfile_string = (char *) 0;
char *call_list_string = (char *) 0;
FILE *call_list_file;

static FILE *fp;
static FILE *fildes;
static long_boolean file_error;
static char fail_message[MAX_ERR_LENGTH];
static char fail_errstring[MAX_ERR_LENGTH];
static long_boolean outfile_special = FALSE;

static char *get_errstring(void)
{
#if defined(sun) || defined(athena_vax)
   extern int sys_nerr;
   extern char *sys_errlist[];

   if (errno < sys_nerr) return sys_errlist[errno];
   else return "?unknown error?";
#else
   return strerror(errno);
#endif
}


extern void general_initialize(void)
{
/* Sorry, plain POSIX doesn't have the nice rand48 stuff. */
#ifdef HAVE_RAND48
   srand48((long int) time((time_t *)0));
#else
   srand((unsigned int) time((time_t *)0));
#endif
}


extern int generate_random_number(int modulus)
{
   int j;

/* Sorry, plain POSIX doesn't have the nice rand48 stuff. */
#ifdef HAVE_RAND48
   random_number = (int) lrand48();
#else
   random_number = (int) rand();
#endif

   j = random_number % modulus;
   return j;
}


extern void hash_nonrandom_number(int number)
{
   hashed_randoms = hashed_randoms*1049633+number;
}


extern void *get_mem(uint32 siz)
{
   void *buf;

   buf = get_mem_gracefully(siz);
   if (!buf && siz != 0) {
      char msg [50];
      sprintf(msg, "Can't allocate %d bytes of memory.", (int) siz);
      uims_fatal_error(msg, 0);
      exit_program(2);
   }
   return buf;
}


/* This will not fail catastrophically, but may return nil pointer
   on nonzero request.  Client must check and react accordingly. */
extern void *get_mem_gracefully(uint32 siz)
{
   return malloc(siz);
}


/* An older version of this, and "get_more_mem_gracefully", actually
   called "malloc" or "realloc" depending on whether the incoming
   pointer was nil.  There was a comment pointing out that this
   was because "SunOS 4 realloc doesn't handle NULL".
   Isn't that funny? */

extern void *get_more_mem(void *oldp, uint32 siz)
{
   void *buf;

   buf = get_more_mem_gracefully(oldp, siz);
   if (!buf && siz != 0) {
      char msg [50];
      sprintf(msg, "Can't allocate %d bytes of memory.", (int) siz);
      uims_fatal_error(msg, 0);
      exit_program(2);
   }
   return buf;
}


/* This will not fail catastrophically, but may return nil pointer
   on nonzero request.  Client must check and react accordingly. */
extern void *get_more_mem_gracefully(void *oldp, uint32 siz)
{
   return realloc((char *) oldp, siz);
}


extern void free_mem(void *ptr)
{
   free(ptr);
}


extern void get_date(char dest[])
{
   time_t clocktime;
   char *junk;
   char *dstptr;

   time(&clocktime);
   junk = ctime(&clocktime);
   dstptr = dest;
   string_copy(&dstptr, junk);
   if (dstptr[-1] == '\n') dstptr[-1] = '\0';         /* Stupid UNIX! */
}



extern void open_file(void)
{
   int this_file_position;
#if defined(WIN32) || (!defined(MSDOS) && !defined(__i386))
   struct stat statbuf;
#endif
   int i;

   file_error = FALSE;

   /* If this is a "special" file (indicated by ending with a colon),
      we simply open it and write. */

   if (outfile_special) {
      if (!(fildes = fopen(outfile_string, "w"))) {
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
         file_error = TRUE;
         return;
      }

      writestuff("Writing to special device.");
      newline();
      newline();
      return;
   }


/* If not on a DJGPP or PC/GCC, things are fairly simple.  We open the file in
   "append" mode, thankful that we have escaped one of the most monumentally stupid
   pieces of OS system design ever to plague the universe, and only need to
   deal with Un*x, which is merely one of the most monumentally stupid
   pieces of OS system design ever to plague this galaxy.  Or maybe we're running
   on Windows or NT, operating systems that have sometimes been observed to do
   the right thing. */

#if defined(WIN32) || (!defined(MSDOS) && !defined(__i386))

   if (stat(outfile_string, &statbuf))
      this_file_position = 0;   /* File doesn't exist. */
   else
      this_file_position = statbuf.st_size;

   /* First, open it in "create/append/read-write" mode.  This allows
      reading or writing, but always writes at the end of the file --
      seeks will not affect the write position.  Since we want to
      remove control-Z's by seeking to the spot before them, that
      isn't good enough.  But "r" mode won't create the file if it
      didn't exist, and "w" mode won't let us write.  So we just have
      to do this. */

   if (!(fildes = fopen(outfile_string, "ab+"))) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   if ((last_file_position != -1) && (last_file_position != this_file_position)) {
      writestuff("Warning -- file has been modified since last sequence.");
      newline();
      newline();
   }

   /* Now that we know that the file exists, open it in binary read-write mode. */

   (void) fclose(fildes);

   if (!(fildes = fopen(outfile_string, "rb+"))) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   if (this_file_position != 0) {
      /* The file exists, and we have opened it in "rb+" mode.  Look at its end. */

      if (fseek(fildes, -4, SEEK_END)) {
         /* It isn't 4 characters long -- forget it.  But first, position at the end. */
         if (fseek(fildes, 0, SEEK_END)) {
            (void) fclose(fildes);     /* What happened????? */
            (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
            (void) strncpy(fail_message, "seek", MAX_ERR_LENGTH);
            file_error = TRUE;
            return;
         }
         goto really_just_append;
      }

      /* We are now 4 before the end.  Look at those last 4 characters. */

      for (i=0 ; i<4 ; i++) {
         if (fgetc(fildes) == 0x1A) {
            writestuff("Warning -- file contains spurious control character -- removing same.");
            newline();
            newline();
            last_file_position = -1;   /* Suppress the other error. */
            break;
         }
      }

      /* Now seek to the end, or to the point just before the offending character. */

      if (fseek(fildes, i-4, SEEK_END)) {
         (void) fclose(fildes);     /* What happened????? */
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "seek", MAX_ERR_LENGTH);
         file_error = TRUE;
         return;
      }

      goto really_just_append;
   }

#else

   /* We need to find out whether there are garbage characters (e.g. ^Z)
      near the end of the existing file, and remove same.  Such things
      have been known to be placed in files by some programs running on PC's.
      
      Furthermore, some PC print software stops printing when it encounters
      one, so we have to get rid of it.

      But wait!  The OS is so convinced that it knows better than we what
      should be in a file, that, in addition to silently putting in this
      character and making the print software silently ignore everything
      in the file that occurs after it, IT MAKES IT INVISIBLE TO US!!!!!
      WE CAN'T EVEN SEE THE %$%^#%^@&*$%^#!@ CONTROL Z!!!!!!
      That is, the system won't let us see it if we open the file in
      the usual "text" mode.  It knows we couldn't possibly be interested
      in a character whose meaning is so trivial that it does nothing
      more than make it impossible to print a file.

      So we thank our lucky stars that the system is watching out for our
      interests in this way and making life simple and convenient for us,
      and we open the file in "binary" mode.

      The difference between "text" and "binary" mode for an open file is
      sometimes obscure.  It's good to know that we have discovered its
      significance on this system.

      So we open in "rb+" mode, and look around for ^Z characters. */




   /* But first, it is an observed fact that, if we open a file in binary
      mode, and the file gets created because of that, some garbage header
      bytes get written to it.  So, we don't open it in binary mode until
      we have determined that it already exists.  I'm not making this up,
      you know.  It really behaves that badly. */

   if (!(fildes = fopen(outfile_string, "r+"))) {

      /* Failed.  Maybe the file doesn't exist, in which case we open it
         in append mode (which will create it if necessary) and don't worry
         about the garbage characters. */

      if (!(fildes = fopen(outfile_string, "a"))) {
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
         file_error = TRUE;
         return;
      }

      this_file_position = ftell(fildes);
   
      if ((last_file_position != -1) && (last_file_position != this_file_position)) {
         writestuff("Warning -- file has been modified since last sequence.");
         newline();
         newline();
      }

      /* We are positioned at the end, because that's what "a" mode does. */
      goto really_just_append;
   }

   /* Now that we know that the file exists, open it in binary mode. */

   (void) fclose(fildes);

   if (!(fildes = fopen(outfile_string, "rb+"))) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   /* The file exists, and we have opened it in "rb+" mode.  Look at its end. */

   if (fseek(fildes, -4, SEEK_END)) {
      /* It isn't 4 characters long -- forget it.  But first, position at the end. */
      if (fseek(fildes, 0, SEEK_END)) {
         (void) fclose(fildes);     /* What happened????? */
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "seek", MAX_ERR_LENGTH);
         file_error = TRUE;
         return;
      }
      goto just_append;
   }

   /* We are now 4 before the end.  Look at those last 4 characters. */

   for (i=0 ; i<4 ; i++) {
      if (fgetc(fildes) == 0x1A) {
         writestuff("Warning -- file contains spurious control character -- removing same.");
         newline();
         newline();
         last_file_position = -1;   /* Suppress the other error. */
         break;
      }
   }

   /* Now seek to the end, or to the point just before the offending character. */

   if (fseek(fildes, i-4, SEEK_END)) {
      (void) fclose(fildes);     /* What happened????? */
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "seek", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   just_append:

   this_file_position = ftell(fildes);

   if ((last_file_position != -1) && (last_file_position != this_file_position)) {
      writestuff("Warning -- file has been modified since last sequence.");
      newline();
      newline();
   }

   /* But wait!!!!  There's more!!!!  On a PC, we can't write to the stream
      if it was opened in "binary" mode!  Don't ask me why, the standards
      documents clearly say that it is legal.  It is simply an observed
      fact that it doesn't work.

      So we remember our seek position, close the file, reopen it in "text"
      mode (that is, "r+"), and seek back to that spot. */

   (void) fclose(fildes);
   if (!(fildes = fopen(outfile_string, "r+"))) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   if (fseek(fildes, i-4, SEEK_END)) {
      (void) fclose(fildes);     /* What happened????? */
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "seek", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   /* One remaining question.  Will the system allow us to seek in a
      text file to a point that we determined while it was opened in
      binary?  Will it figure out some way to prevent us from backing up
      over that ^Z?  Will it write a ^Z after the seek point?  Will it
      find some creative way to screw us?

      No.  It actually seems to work.  Aren't computers wonderful? */

#endif

   really_just_append:

   if (this_file_position == 0) {
      writestuff("File does not exist, creating it.");
      newline();
      newline();
   }
   else {
      if (last_file_position == -1) {
         writestuff("Appending to existing file.");
         newline();
         newline();
      }

      if ((fwrite("\f", 1, 1, fildes) != 1) || ferror(fildes)) {
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "write formfeed", MAX_ERR_LENGTH);
         file_error = TRUE;      /* Indicate that sequence will not get written. */
         return;
      }
   }
}


extern void write_file(char line[])
{
   uint32 size;

#if defined(WIN32)
   char nl[] = "\r\n";
#define NLSIZE 2
#else
   char nl[] = "\n";
#define NLSIZE 1
#endif

   if (file_error) return;    // Don't keep trying after a failure.

   size = strlen(line);
   if (size != 0) {
      if ((fwrite(line, 1, size, fildes) != size) || ferror(fildes)) {
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
         file_error = TRUE;      /* Indicate that sequence will not get written. */
         return;
      }
   }

   if ((fwrite(nl, 1, NLSIZE, fildes) != NLSIZE) || ferror(fildes)) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
      file_error = TRUE;      /* Indicate that sequence will not get written. */
   }
}


extern void close_file(void)
{
   struct stat statbuf;
   char foo[MAX_ERR_LENGTH*10];

   if (file_error) goto fail;

   if (fclose(fildes)) goto error;

   if (!outfile_special) {
      if (stat(outfile_string, &statbuf))
         goto error;

      last_file_position = statbuf.st_size;
   }

   return;

 error:

   (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
   (void) strncpy(fail_message, "close", MAX_ERR_LENGTH);

 fail:

   (void) strncpy(foo, "WARNING!!!  Sequence has not been written!  File ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_message, MAX_ERR_LENGTH);
   (void) strncat(foo, " failure on \"", MAX_ERR_LENGTH);
   (void) strncat(foo, outfile_string, MAX_ERR_LENGTH);
   (void) strncat(foo, "\": ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_errstring, MAX_ERR_LENGTH);
   (void) strncat(foo, " -- try \"change output file\" operation.", MAX_ERR_LENGTH);
   specialfail(foo);
}

/* Used only for printing error messages, so writes to stderr */
extern void print_line(Cstring s)
{
   fprintf(stderr, "%s\n", s);
}



extern void init_error(char s[])
{
   char msg [50];
   sprintf (msg, "Error initializing program: %s", s);
   uims_fatal_error(msg, 0);
   exit_program(1);
}



extern long_boolean parse_level(Cstring s, dance_level *levelp)
{
   char first = tolower(s[0]);

   switch (first) {
      case 'm': *levelp = l_mainstream; return TRUE;
      case 'p': *levelp = l_plus; return TRUE;
      case 'a':
         if (s[1] == '1' && !s[2]) *levelp = l_a1;
         else if (s[1] == '2' && !s[2]) *levelp = l_a2;
         else if (s[1] == 'l' && s[2] == 'l' && !s[3]) *levelp = l_dontshow;
         else return FALSE;
         return TRUE;
      case 'c':
         if (s[1] == '3' && (s[2] == 'a' || s[2] == 'A') && !s[3])
            *levelp = l_c3a;
         else if (s[1] == '3' && (s[2] == 'x' || s[2] == 'X') && !s[3])
            *levelp = l_c3x;
         else if (s[1] == '4' && (s[2] == 'a' || s[2] == 'A') && !s[3])
            *levelp = l_c4a;
         else if (s[1] == '4' && (s[2] == 'x' || s[2] == 'X') && !s[3])
            *levelp = l_c4x;
         else {
            if (!s[2]) {
               switch (s[1]) {
                  case '1': *levelp = l_c1; return TRUE;
                  case '2': *levelp = l_c2; return TRUE;
                  case '3': *levelp = l_c3; return TRUE;
                  case '4': *levelp = l_c4; return TRUE;
                  default: return FALSE;
               }
            }
            else return FALSE;
         }
         return TRUE;
      default:
         return FALSE;
   }
}


extern char *read_from_call_list_file(char name[], int n)
{
   return (fgets(name, n, call_list_file));
}


extern void write_to_call_list_file(const char name[])
{
   fputs(name, call_list_file);
   fputs("\n", call_list_file);
}


extern long_boolean close_call_list_file(void)
{
   if (fclose(call_list_file)) {
      uims_fatal_error("Can't close call list file", 0);
      return TRUE;
   }
   else
      return FALSE;
}


static char rewrite_filename_as_star[2] = { '\0' , '\0' };  /* First char could be "*" or "+". */


/* This makes sure that outfile string is a legal filename, and sets up
   "outfile_special" to tell if it is a printing device.
   Returns FALSE if error occurs.  No action taken in that case. */

extern long_boolean install_outfile_string(char newstring[])
{
   char test_string[MAX_FILENAME_LENGTH];
   long_boolean file_is_ok;
   int j;

   rewrite_filename_as_star[0] = '\0';

   /* Clean off leading blanks, and stop after any internal blank. */

   (void) sscanf(newstring, "%s", test_string);

   /* Look for special file string of "*".  If so, generate a new file name. */

   if ((test_string[0] == '*' || test_string[0] == '+') && !test_string[1]) {
      time_t clocktime;
      FILE *filetest;
      char junk[20], junk2[20], t1[20], t2[20], t3[20], t4[20], t5[20];
      char letter[2];
      char *p;

      letter[0] = 'a';
      letter[1] = '\0';
      time(&clocktime);
      (void) sscanf(ctime(&clocktime), "%s %s %s %s %s", t1, t2, t3, t4, t5);

      /* Now t2 = "Jan", t3 = "16", and t5 = "1996". */

      (void) strncpy(junk, t3, 3);
      (void) strncat(junk, t2, 3);
      (void) strncat(junk, &t5[strlen(t5)-2], 2);
      for (p=junk ; *p ; p++) *p = tolower(*p);  /* Month in lower case. */
      (void) strncpy(junk2, junk, 10);    /* This should be "16jan96". */

      for (;;) {
         (void) strncat(junk2, filename_strings[calling_level], 4);

         /* If the given filename is "+", accept it immediately.
            Otherwise, fuss with the generated name until we get a
            nonexistent file. */

         if (test_string[0] == '+' || (filetest = fopen(junk2, "r")) == 0) break;
         (void) fclose(filetest);
         if (letter[0] == 'z'+1) letter[0] = 'A';
         else if (letter[0] == 'Z'+1) return FALSE;
         (void) strncpy(junk2, junk, 10);
         (void) strncat(junk2, letter, 4);     /* Try appending a letter. */
         letter[0]++;
      }

      (void) strncpy(outfile_string, junk2, MAX_FILENAME_LENGTH);
      outfile_special = FALSE;
      last_file_position = -1;
      rewrite_filename_as_star[0] = test_string[0];
      return TRUE;
   }

   /* Now see if we can write to it. */

#ifdef POSIX_STYLE
   /* If the file does not exist, we allow it, even though creation may
      not be possible because of directory permissions.  It is unfortunate
      that there is no feasible way to determine whether a given pathname
      could be opened for writing. */
   if (access(test_string, F_OK) || !access(test_string, W_OK))
      file_is_ok =  TRUE;
   else
      file_is_ok =  FALSE;
#else
   file_is_ok =  TRUE;
#endif

   if (file_is_ok) {
      (void) strncpy(outfile_string, test_string, MAX_FILENAME_LENGTH);
      j = strlen(outfile_string);
      outfile_special = (j>0 && outfile_string[j-1] == ':');
      last_file_position = -1;
      return TRUE;
   }
   else
      return FALSE;
}



static FILE *init_file;
static int session_linenum = 0;

/* 0 for "no session" line, 1 for real ones, 2 for "new session". */
static int session_line_state = 0;

extern long_boolean get_first_session_line(void)
{
   char line[MAX_FILENAME_LENGTH];

   session_line_state = 0;

   /* If we are writing a call list file, that's all we do. */

   if (glob_call_list_mode == call_list_mode_writing ||
       glob_call_list_mode == call_list_mode_writing_full)
      return TRUE;

   /* Or if the file didn't exist, or we are in diagnostic mode. */
   if (!init_file || diagnostic_mode) return TRUE;

   /* Search for the "[Sessions]" indicator. */

   if (fseek(init_file, 0, SEEK_SET))
      return TRUE;

   for (;;) {
      if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) return TRUE;
      if (!strncmp(line, "[Sessions]", 10)) break;
   }

   return FALSE;
}


extern long_boolean get_next_session_line(char *dest)
{
   int j;
   char line[MAX_FILENAME_LENGTH];

   if (session_line_state == 0) {
      session_line_state = 1;
      sprintf(dest, "  0     (no session)");
      return TRUE;
   }
   else if (session_line_state == 2)
      return FALSE;

   if (!fgets(line, MAX_FILENAME_LENGTH, init_file) || line[0] == '\n') {
      session_line_state = 2;
      sprintf(dest, "%3d     (create a new session)", session_linenum+1);
      return TRUE;
   }

   j = strlen(line);
   if (j>0) line[j-1] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */
   sprintf(dest, "%3d  %s", ++session_linenum, line);
   return TRUE;
}


extern void prepare_to_read_menus(void)
{
   uint32 arithtest = 2081607680;

   /* This "if" should never get executed.  We expect compilers to optimize
      it away, and perhaps print a warning about it. */

   /* Test that the constants ROLL_BIT and DBROLL_BIT are in the right
      relationship, with ROLL_BIT >= DBROLL_BIT, that is, the roll bits
      in a person record are to the left of the roll bits in the binary database.
      This is because of expressions "ROLL_BIT/DBROLL_BIT" in sdbasic.cpp to
      align stuff from the binary database into the person record. */

   if (NROLL_BIT < NDBROLL_BIT)
      init_error("constants not consistent -- program has been compiled incorrectly.");
   else if ((508205 << 12) != arithtest)
      init_error("arithmetic is less than 32 bits -- program has been compiled incorrectly.");
   else if (NUM_QUALIFIERS > 125)
      init_error("insufficient qualifier space -- program has been compiled incorrectly.");

   /* We need to take away the "zig-zag" directions if the level is below A2. */

   if (calling_level < zig_zag_level) {
      last_direction_kind = direction_zigzag-1;
      direction_names[direction_zigzag] = (Cstring) 0;
   }

   if (glob_call_list_mode == call_list_mode_none ||
       glob_call_list_mode == call_list_mode_abridging) {
      int i;

      /* Find out how big the command menu needs to be. */

      for (num_command_commands = 0 ;
           command_menu[num_command_commands].command_name ;
           num_command_commands++) ;

      command_commands = (Cstring *) get_mem(sizeof(Cstring) * num_command_commands);
      command_command_values =
         (command_kind *) get_mem(sizeof(command_kind) * num_command_commands);

      for (i = 0 ; i < num_command_commands; i++) {
         command_commands[i] = command_menu[i].command_name;
         command_command_values[i] = command_menu[i].action;
      }

      /* Find out how big the startup menu needs to be. */

      for (num_startup_commands = 0 ;
           startup_menu[num_startup_commands].startup_name ;
           num_startup_commands++) ;

      startup_commands = (Cstring *) get_mem(sizeof(Cstring) * num_startup_commands);
      startup_command_values =
         (start_select_kind *) get_mem(sizeof(start_select_kind) * num_startup_commands);

      for (i = 0 ; i < num_startup_commands; i++) {
         startup_commands[i] = startup_menu[i].startup_name;
         startup_command_values[i] = startup_menu[i].action;
      }

      /* Find out how big the resolve menu needs to be. */

      for (number_of_resolve_commands = 0 ;
           resolve_menu[number_of_resolve_commands].command_name ;
           number_of_resolve_commands++) ;

      resolve_command_strings = (Cstring *) get_mem(sizeof(Cstring) * number_of_resolve_commands);
      resolve_command_values = (resolve_command_kind *)
         get_mem(sizeof(resolve_command_kind) * number_of_resolve_commands);

      for (i = 0 ; i < number_of_resolve_commands; i++) {
         resolve_command_strings[i] = resolve_menu[i].command_name;
         resolve_command_values[i] = resolve_menu[i].action;
      }
   }
}


extern void initialize_misc_lists(void)
{
   int i;
   uint32 ui;

   initialize_sdlib();

   /* Create the tagger menu lists */

   for (i=0 ; i<NUM_TAGGER_CLASSES ; i++) {
      tagger_menu_list[i] = (Cstring *) get_mem((number_of_taggers[i]+1) * sizeof(char *));

      for (ui=0; ui<number_of_taggers[i]; ui++)
         tagger_menu_list[i][ui] = tagger_calls[i][ui]->menu_name;

      tagger_menu_list[i][number_of_taggers[i]] = (Cstring) 0;
   }

   // Create the selector menu list.  It is one item shorter than the enumeration,
   // because we skip the first item in the enumeration.

   selector_menu_list = (Cstring *) get_mem((selector_enum_extent) * sizeof(char *));

   for (i=0; i<selector_enum_extent-1; i++)
      selector_menu_list[i] = selector_list[i+1].name;

   selector_menu_list[selector_enum_extent-1] = (Cstring) 0;

   /* Create the circcer list. */

   circcer_menu_list = (Cstring *) get_mem((number_of_circcers+1) * sizeof(char *));

   for (ui=0; ui<number_of_circcers; ui++)
      circcer_menu_list[ui] = circcer_calls[ui]->menu_name;

   circcer_menu_list[number_of_circcers] = (Cstring) 0;

   initialize_getout_tables();
}



extern long_boolean open_session(int argc, char **argv)
{
   int i, j;
   int argno;
   char line[MAX_FILENAME_LENGTH];
   char **args;
   int nargs = argc;

   /* Copy the arguments, so that we can use "realloc" to grow the list. */

   args = (char **) get_mem(nargs * sizeof(char *));

   (void) memcpy(args, argv, nargs * sizeof(char *));

   /* Read the initialization file, looking for options. */

   init_file = fopen(SESSION_FILENAME, "r");

   if (init_file) {
      int insert_pos = 1;

      /* Search for the "[Options]" indicator. */

      for (;;) {
         if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) goto no_options;
         if (!strncmp(line, "[Options]", 9)) break;
      }

      for (;;) {
         char *lineptr = line;

         if (!fgets(&line[1], MAX_FILENAME_LENGTH, init_file) || line[1] == '\n') break;

         j = strlen(&line[1]);
         if (j>0) line[j] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */
         line[0] = '-';             /* Put a '-' in front of it. */

         for (;;) {
            char token[MAX_FILENAME_LENGTH];
            int newpos;

            /* Break the line into tokens, and insert each as a command-line argument. */

            /* We need to put a blank at the end, so that the "%s %n" spec won't get confused. */

            j = strlen(lineptr);
            if (j > 0 && lineptr[j-1] != ' ') {
               lineptr[j] = ' ';
               lineptr[j+1] = '\0';
            }
            if (sscanf(lineptr, "%s%n ", token, &newpos) != 1) break;

            j = strlen(token)+1;
            nargs++;
            args = (char **) get_more_mem(args, nargs * sizeof(char *));
            for (i=nargs-1 ; i>insert_pos ; i--) args[i] = args[i-1];
            args[insert_pos] = (char *) get_mem(j);
            (void) memcpy(args[insert_pos], token, j);
            insert_pos++;
            lineptr += newpos;
         }
      }

      no_options: ;
   }

   /* This lets the user interface intercept command line arguments that it is interested in. */
   uims_process_command_line(&nargs, &args);

   glob_call_list_mode = call_list_mode_none;
   calling_level = l_nonexistent_concept;    /* Mark it uninitialized. */

   for (argno=1; argno < nargs; argno++) {
      if (args[argno][0] == '-') {

         /* Special flag: must be one of
            -write_list <filename>  -- write out the call list for the
                  indicated level INSTEAD OF running the program
            -write_full_list <filename>  -- write out the call list for the
                  indicated level and all lower levels INSTEAD OF running the program
            -abridge <filename>  -- read in the file, strike all the calls
                  contained therein off the menus, and proceed.
            -diagnostic  -- (this is a hidden flag) suppress display of version info */

         if (strcmp(&args[argno][1], "write_list") == 0) {
            glob_call_list_mode = call_list_mode_writing;
            if (argno+1 < nargs) call_list_string = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "write_full_list") == 0) {
            glob_call_list_mode = call_list_mode_writing_full;
            if (argno+1 < nargs) call_list_string = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "abridge") == 0) {
            glob_call_list_mode = call_list_mode_abridging;
            if (argno+1 < nargs) call_list_string = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "sequence") == 0) {
	     if (argno+1 < nargs) new_outfile_string = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "db") == 0) {
            if (argno+1 < nargs) database_filename = args[argno+1];
         }
         else if (strcmp(&args[argno][1], "sequence_num") == 0) {
            if (argno+1 < nargs) {
               if (sscanf(args[argno+1], "%d", &ui_options.sequence_num_override) != 1)
                  uims_bad_argument("Bad number:", args[argno+1], (char *) 0);
            }
         }
         else if (strcmp(&args[argno][1], "no_intensify") == 0)
            { ui_options.no_intensify = 1; continue; }
         else if (strcmp(&args[argno][1], "reverse_video") == 0)
            { ui_options.reverse_video = 1; continue; }
         else if (strcmp(&args[argno][1], "normal_video") == 0)
            { ui_options.reverse_video = 0; continue; }
         else if (strcmp(&args[argno][1], "pastel_color") == 0)
            { ui_options.pastel_color = 1; continue; }
         else if (strcmp(&args[argno][1], "bold_color") == 0)
            { ui_options.pastel_color = 0; continue; }
         else if (strcmp(&args[argno][1], "no_color") == 0)
            { ui_options.color_scheme = no_color; continue; }
         else if (strcmp(&args[argno][1], "color_by_couple") == 0)
            { ui_options.color_scheme = color_by_couple; continue; }
         else if (strcmp(&args[argno][1], "color_by_couple_rgyb") == 0)
            { ui_options.color_scheme = color_by_couple_rgyb; continue; }
         else if (strcmp(&args[argno][1], "color_by_corner") == 0)
            { ui_options.color_scheme = color_by_corner; continue; }
         else if (strcmp(&args[argno][1], "no_sound") == 0)
            { ui_options.no_sound = 1; continue; }
         else if (strcmp(&args[argno][1], "single_click") == 0)
            { ui_options.accept_single_click = TRUE; continue; }
         else if (strcmp(&args[argno][1], "no_checkers") == 0)
            { ui_options.no_graphics = 1; continue; }
         else if (strcmp(&args[argno][1], "no_graphics") == 0)
            { ui_options.no_graphics = 2; continue; }
         else if (strcmp(&args[argno][1], "diagnostic") == 0)
            { diagnostic_mode = TRUE; continue; }
         else if (strcmp(&args[argno][1], "singlespace") == 0)
            { ui_options.singlespace_mode = TRUE; continue; }
         else if (strcmp(&args[argno][1], "no_warnings") == 0)
            { ui_options.nowarn_mode = TRUE; continue; }
         else if (strcmp(&args[argno][1], "concept_levels") == 0)
            { allowing_all_concepts = TRUE; continue; }
         else if (strcmp(&args[argno][1], "minigrand_getouts") == 0)
            { allowing_minigrand = TRUE; continue; }
         else if (strcmp(&args[argno][1], "active_phantoms") == 0)
            { using_active_phantoms = TRUE; continue; }
         else if (strcmp(&args[argno][1], "discard_after_error") == 0)
            { retain_after_error = FALSE; continue; }
         else if (strcmp(&args[argno][1], "retain_after_error") == 0)
            { retain_after_error = TRUE; continue; }
         else
            uims_bad_argument("Unknown flag:", args[argno], (char *) 0);

         argno++;
         if (argno >= nargs)
            uims_bad_argument("This flag must be followed by a file name:", args[argno-1], (char *) 0);
      }
      else if (!parse_level(args[argno], &calling_level)) {
         uims_bad_argument("Unknown calling level argument:", args[argno],
            "Known calling levels: m, p, a1, a2, c1, c2, c3a, c3, c3x, c4a, c4, or c4x.");
      }
   }

   free(args);

   /* If we have a calling level at this point, fill in the output file name.
      If we do not have a calling level, we will either get it from the session
      file, in which case we will get the file name also, or we will have to query
      the user.  In the latter case, we will do this step again. */

   if (calling_level != l_nonexistent_concept)
      (void) strncat(outfile_string, filename_strings[calling_level], MAX_FILENAME_LENGTH);

   /* At this point, the command-line arguments, and the preferences in the "[Options]"
      section of the initialization file, have been processed.  Some of those things
      may still interact with the start-up procedure.  They are:

         glob_call_list_mode [default = call_list_mode_none]
         calling_level       [default = l_nonexistent_concept]
         new_outfile_string  [default = (char *) 0, this is just a pointer]
         call_list_string    [default = (char *) 0], this is just a pointer]
         database_filename   [default = "sd_calls.dat", this is just a pointer]
   */

   /* Now open the session file and find out what we are doing. */

   return uims_open_session(argc, argv);
}


extern int process_session_info(Cstring *error_msg)
{
   int i, j;

   if (session_index == 0)
      return 1;

   if (session_index <= session_linenum) {
      char line[MAX_FILENAME_LENGTH];
      int ccount;
      int num_fields_parsed;
      char junk_name[MAX_FILENAME_LENGTH];
      char filename_string[MAX_FILENAME_LENGTH];
      char session_levelstring[50];

      /* Find the "[Sessions]" indicator again. */

      if (fseek(init_file, 0, SEEK_SET)) {
         *error_msg = "Can't find correct position in session file.";
         return 3;
      }

      for (;;) {
         if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) {
            *error_msg = "Can't find correct indicator in session file.";
            return 3;
         }
         if (!strncmp(line, "[Sessions]", 10)) break;
      }

      /* Skip over the lines before the one we want. */

      for (i=0 ; i<session_index ; i++) {
         if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) break;
      }

      if (i != session_index)
         return 1;

      j = strlen(line);
      if (j>0) line[j-1] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */

      num_fields_parsed = sscanf(
                             line, "%s %s %d %n%s",
                             filename_string, session_levelstring,
                             &sequence_number, &ccount,
                             junk_name);

      if (num_fields_parsed < 3) {
         *error_msg = "Bad format in session file.";
         return 3;
      }

      if (!parse_level(session_levelstring, &calling_level)) {
         *error_msg = "Bad level given in session file.";
         return 3;
      }

      if (num_fields_parsed == 4)
         strncpy(header_comment, &line[ccount], MAX_TEXT_LINE_LENGTH);
      else
         header_comment[0] = 0;

      if (!install_outfile_string(filename_string)) {
         *error_msg = "Bad file name in session file, using default instead.";
         return 2;    /* This return code will not abort the session. */
      }
   }
   else {
      /* We are creating a new session to be appended to the file. */
      sequence_number = 1;
      need_new_header_comment = TRUE;
   }

   return 0;
}


extern long_boolean open_call_list_file(char filename[])
{
   char msg [50];

   call_list_file = fopen(filename,
      (glob_call_list_mode == call_list_mode_abridging) ? "r" : "w");

   if (!call_list_file) {
      sprintf (msg, "Can't open call list file: %s", filename);
      uims_fatal_error (msg, 0);
      return TRUE;
   }
   else
      return FALSE;
}


extern long_boolean open_accelerator_region(void)
{
   char line[MAX_FILENAME_LENGTH];

   if (!init_file) return FALSE;

   if (fseek(init_file, 0, SEEK_SET))
      return FALSE;

   /* Search for the "[Accelerators]" indicator. */

   for (;;) {
      if (!fgets(line, MAX_FILENAME_LENGTH, init_file)) return FALSE;
      if (!strncmp(line, "[Accelerators]", 14)) return TRUE;
   }
}

extern long_boolean get_accelerator_line(char line[])
{
   if (!init_file) return FALSE;

   for ( ;; ) {
      int j;

      if (!fgets(line, MAX_FILENAME_LENGTH, init_file) || line[0] == '\n') return FALSE;

      j = strlen(line);
      if (j>0) line[j-1] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */

      if (line[0] != '#') return TRUE;
   }
}


extern void close_init_file(void)
{
   if (init_file) (void) fclose(init_file);
}


static int write_back_session_line(FILE *wfile)
{
   char *filename = rewrite_filename_as_star[0] ? rewrite_filename_as_star : outfile_string;

   if (header_comment[0])
      return
         fprintf(wfile, "%-20s %-11s %6d      %s\n",
                 filename,
                 getout_strings[calling_level],
                 sequence_number,
                 header_comment);
   else
      return
         fprintf(wfile, "%-20s %-11s %6d\n",
                 filename,
                 getout_strings[calling_level],
                 sequence_number);
}


extern void final_exit(int code)
{
   if (session_index != 0) {
      char line[MAX_FILENAME_LENGTH];
      FILE *rfile;
      FILE *wfile;
      int i;

      remove(SESSION2_FILENAME);

      if (rename(SESSION_FILENAME, SESSION2_FILENAME)) {
         printf("Failed to save file '" SESSION_FILENAME "' in '" SESSION2_FILENAME "'\n");
         printf("%s\n", get_errstring());
      }
      else {
         if (!(rfile = fopen(SESSION2_FILENAME, "r"))) {
            printf("Failed to open '" SESSION2_FILENAME "'\n");
         }
         else {
            if (!(wfile = fopen(SESSION_FILENAME, "w"))) {
               printf("Failed to open '" SESSION_FILENAME "'\n");
            }
            else {
               long_boolean more_stuff = FALSE;

               /* Search for the "[Sessions]" indicator. */

               for (;;) {
                  if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) goto copy_done;
                  if (fputs(line, wfile) == EOF) goto copy_failed;
                  if (!strncmp(line, "[Sessions]", 10)) break;
               }

               for (i=0 ; ; i++) {
                  if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) break;
                  if (line[0] == '\n') { more_stuff = TRUE; break; }

                  if (i == session_index-1) {
                     if (write_back_session_line(wfile) < 0)
                        goto copy_failed;
                  }
                  else if (i == -session_index-1) {
                  }
                  else {
                     if (fputs(line, wfile) == EOF) goto copy_failed;
                  }
               }

               if (i < session_index) {
                  /* User has requested a line number larger than the file.  Append a new line. */
                  if (write_back_session_line(wfile) < 0)
                     goto copy_failed;
               }

               if (more_stuff) {
                  if (fputs("\n", wfile) == EOF) goto copy_failed;
                  for (;;) {
                     if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) break;
                     if (fputs(line, wfile) == EOF) goto copy_failed;
                  }
               }

               goto copy_done;

               copy_failed:

               printf("Failed to write to '" SESSION_FILENAME "'\n");

               copy_done:

               (void) fclose(wfile);
            }
            (void) fclose(rfile);
         }
      }
   }

   uims_final_exit(code);
}

extern long_boolean open_database(char *msg1, char *msg2)
{
   int format_version, n, j;

   *msg1 = (char) 0;
   *msg2 = (char) 0;

   /* The "b" in the mode is meaningless and harmless in POSIX.  Some systems,
      however, require it for correct handling of binary data. */
   if (!(fp = fopen(database_filename, "rb"))) {
      strncpy(msg1, "Can't open database file.", 199);
      return TRUE;
   }

   if (read_16_from_database() != DATABASE_MAGIC_NUM) {
      sprintf(msg1,
              "Database file \"%s\" has improper format.", database_filename);
      return TRUE;
   }

   format_version = read_16_from_database();
   if (format_version != DATABASE_FORMAT_VERSION) {
      sprintf(msg1,
              "Database format version (%d) is not the required one (%d)",
              format_version, DATABASE_FORMAT_VERSION);
      strncpy(msg2, "You must recompile the database.", 199);
      return TRUE;
   }

   abs_max_calls = read_16_from_database();
   max_base_calls = read_16_from_database();

   n = read_16_from_database();

   if (n > 80) {
      strncpy(msg1, "Database version string is too long.", 199);
      return TRUE;
   }

   for (j=0; j<n; j++)
      database_version[j] = (unsigned char) read_8_from_database();

   database_version[j] = '\0';
   return FALSE;
}


extern uint32 read_8_from_database(void)
{
   return fgetc(fp) & 0xFF;
}


extern uint32 read_16_from_database(void)
{
   uint32 bar;

   bar = (read_8_from_database() & 0xFF) << 8;
   bar |= read_8_from_database() & 0xFF;
   return bar;
}


extern void close_database(void)
{
   fclose(fp);
}


extern int parse_number(char junk[])
{
   int n;

   if (sscanf(junk, "%d", &n) != 1) return(0);
   return(n);
}


#ifdef NEGLECT
extern void fill_in_neglect_percentage(char junk[], int n)
{
   sprintf(junk, "LEAST RECENTLY USED %d%% OF THE CALLS ARE:", n);
}
#endif
