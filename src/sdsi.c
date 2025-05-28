/* SD -- square dance caller's helper.

    Copyright (C) 1990, 1991, 1992, 1993  William B. Ackerman.

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

    This is for version 30. */

/* This defines the following functions:
   general_initialize
   generate_random_number
   generate_random_concept_p
   get_mem
   get_mem_gracefully
   get_more_mem
   get_more_mem_gracefully
   free_mem
   get_date
   open_file
   probe_file
   write_file
   close_file
   print_line
   print_id_error
   init_error
   add_resolve_indices
   final_exit
   open_database
   read_8_from_database
   read_16_from_database
   close_database
   fill_in_neglect_percentage
   parse_number
   open_call_list_file
   read_from_call_list_file
   write_to_call_list_file
   close_call_list_file

and the following external variables:
   random_number
   hashed_randoms
   database_filename
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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

/* We take pity on those poor souls who are compelled to use XOPEN,
   an otherwise fine standard, that doesn't put prototypes
   into stdlib.h. */

#if defined(_XOPEN_SOURCE)
   /* We have taken the liberty of translating the prototypes
      appearing in the XOPEN manual into ANSI C.  We can't
      imagine why anyone would ever use the prototypes
      in the prehistoric nomenclature. */
extern void srand48(long int);
extern long int lrand48(void);
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
extern void free(void *ptr);
extern char *malloc(unsigned int siz);
extern char *realloc(char *oldp, unsigned int siz);
extern void exit(int code);
extern void srand48(long int);
extern long int lrand48(void);
#endif

#if !defined(sun) && (!__STDC__ || defined(MSDOS))
extern char *strerror(int);
#endif

/* I think we have everything now.  Isn't portability fun?  Before people
   started taking these standards as semi-seriously as they do now, it was
   MUCH HARDER to make a program portable than what you just saw. */

#include "sd.h"
#include "paths.h"


/* These variables are external. */

int random_number;
int hashed_randoms;


Private int last_hashed_randoms;
Private FILE *fp;
Private FILE *fildes;
Private long_boolean file_error;
Private char fail_message[MAX_ERR_LENGTH];
Private char fail_errstring[MAX_ERR_LENGTH];

Private char *get_errstring(void)
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
   last_hashed_randoms = hashed_randoms;       /* save in case we need to undo it */
   hashed_randoms = hashed_randoms*37+j;
   return(j);
}


extern long_boolean generate_random_concept_p(void)
{
   int i = generate_random_number(8);

   /* Since we are not going to use the random number in a one-to-one way, we run the risk
      of not having hashed_randoms uniquely represent what is happening.  To remedy
      the problem, we undo the transformation that was just made to hashed_randoms,
      and redo it with just the information we intend to use. */

   hashed_randoms = last_hashed_randoms*37;       /* Now just need to add something. */

   if (i < CONCEPT_PROBABILITY) {
      hashed_randoms++;          /* This should do the trick. */
      return (TRUE);
   }
   else {
      return (FALSE);
   }
}


extern void *get_mem(unsigned int siz)
{
   void *buf;

   buf = get_mem_gracefully(siz);
   if (!buf && siz != 0) {
      fprintf(stderr, "Can't allocate %d bytes of memory.\n", siz);
      exit_program(2);
   }
   return buf;
}


/* This will not fail catastrophically, but may return nil pointer
   on nonzero request.  Client must check and react accordingly. */
extern void *get_mem_gracefully(unsigned int siz)
{
   return malloc(siz);
}


extern void *get_more_mem(void *oldp, unsigned int siz)
{
   void *buf;

   if (!oldp)
      return get_mem(siz);	/* SunOS 4 realloc doesn't handle NULL */
   buf = get_more_mem_gracefully(oldp, siz);
   if (!buf && siz != 0) {
      fprintf(stderr, "Can't allocate %d bytes of memory.\n", siz);
      exit_program(2);
   }
   return buf;
}


/* This will not fail catastrophically, but may return nil pointer
   on nonzero request.  Client must check and react accordingly. */
extern void *get_more_mem_gracefully(void *oldp, unsigned int siz)
{
   if (!oldp)
      return malloc(siz);	/* SunOS 4 realloc doesn't handle NULL */
   return realloc(oldp, siz);
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

   file_error = FALSE;

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

   if (this_file_position == 0) {
      writestuff("File does not exist, creating it.");
      newline();
      newline();
   }
   else {
      char junk[1];

      if (last_file_position == -1) {
         writestuff("Appending to existing file.");
         newline();
         newline();
      }

      junk[0] = '\f';      /* Write a formfeed (end-of-page indicator). */
      if ((fwrite(junk, 1, 1, fildes) != 1) || ferror(fildes)) {
         (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
         (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
         file_error = TRUE;
      }
   }
}


extern long_boolean probe_file(char filename[])
{
   /* If the file does not exist, we allow it, even though creation may
      not be possible because of directory permissions.  It is unfortunate
      that there is no feasible way to determine whether a given pathname
      could be opened for writing. */
#ifdef POSIX_STYLE
   if (access(filename, F_OK) || !access(filename, W_OK))
      return (TRUE);
   else
      return (FALSE);
#else
   return (TRUE);
#endif
}


extern void write_file(char line[])
{
   int size;
   char nl = '\n';

   if (file_error) return;    /* Don't keep trying after a failure. */

   size = strlen(line);
   if ((fwrite(line, 1, size, fildes) != size) || ferror(fildes)) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
      file_error = TRUE;      /* Indicate that sequence will not get written. */
      return;
   }

   if ((fwrite(&nl, 1, 1, fildes) != 1) || ferror(fildes)) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "write", MAX_ERR_LENGTH);
      file_error = TRUE;      /* Indicate that sequence will not get written. */
   }
}


extern void close_file(void)
{
   char foo[MAX_ERR_LENGTH*10];

   if (file_error) goto fail;
   last_file_position = ftell(fildes);

   if (!fclose(fildes)) return;

   (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
   (void) strncpy(fail_message, "close", MAX_ERR_LENGTH);

   fail:

   (void) strncpy(foo, "WARNING!!!  Sequence has not been written!  File ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_message, MAX_ERR_LENGTH);
   (void) strncat(foo, " failure on \"", MAX_ERR_LENGTH);
   (void) strncat(foo, outfile_string, MAX_ERR_LENGTH);
   (void) strncat(foo, "\": ", MAX_ERR_LENGTH);
   (void) strncat(foo, fail_errstring, MAX_ERR_LENGTH);
   specialfail(foo);
}

/* Used only for printing error messages, so writes to stderr */
extern void print_line(char s[])
{
   fprintf(stderr, "%s\n", s);
}


extern void print_id_error(int n)
{
   fprintf(stderr, "Call didn't identify self -- %d\n", n);
}


extern void init_error(char s[])
{
   fprintf(stderr, "Error initializing program: %s\n", s);
}


extern void add_resolve_indices(char junk[], int cur, int max)
{
   sprintf(junk, "%d out of %d", cur, max);
}



extern void final_exit(int code)
{
   exit(code);
}

char *database_filename = DATABASE_FILENAME;

extern void open_database(void)
{
   int format_version, n, j;

   /* The "b" in the mode is meaningless and harmless in POSIX.  Some systems,
      however, require it for correct handling of binary data. */
   if (!(fp = fopen(database_filename, "rb"))) {
      fprintf(stderr, "sd: Can't open database file.\n");
      perror(database_filename);
      if (errno == ENOENT)
         fprintf(stderr, "Use the mkcalls program to create the database file.\n");
      exit_program(1);
   }

   if (read_16_from_database() != DATABASE_MAGIC_NUM) {
      fprintf(stderr,
         "Database file \"%s\" has improper format.\n", database_filename);
      exit_program(1);
   }

   format_version = read_16_from_database();
   if (format_version != DATABASE_FORMAT_VERSION) {
      fprintf(stderr,
         "Database format version (%d) is not the required one (%d) -- you must recompile the database.\n",
         format_version, DATABASE_FORMAT_VERSION);
      exit_program(1);
   }

   abs_max_calls = read_16_from_database();
   max_base_calls = read_16_from_database();

   n = read_16_from_database();

   if (n > 80) {
      fprintf(stderr, "Database version string is too long.\n");
      exit_program(1);
   }

   for (j=0; j<n; j++)
      database_version[j] = read_8_from_database();

   database_version[j] = '\0';
}


extern unsigned int read_8_from_database(void)
{
   return fgetc(fp) & 0xFF;
}


extern unsigned int read_16_from_database(void)
{
   unsigned int bar;

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


Private FILE *call_list_file;

extern long_boolean open_call_list_file(call_list_mode_t call_list_mode, char filename[])
{
   if (call_list_mode == call_list_mode_abridging) {
      call_list_file = fopen(filename, "r");
   }
   else {
      call_list_file = fopen(filename, "w");
   }

   if (!call_list_file) {
      printf("Can't open call list file\n");
      perror(filename);
      return TRUE;
   }
   else
      return FALSE;
}


extern char *read_from_call_list_file(char name[], int n)
{
   return (fgets(name, n, call_list_file));
}


extern void write_to_call_list_file(char name[])
{
   fputs(name, call_list_file);
   fputs("\n", call_list_file);
}


extern long_boolean close_call_list_file(void)
{
   if (fclose(call_list_file)) {
      printf("Can't close call list file\n");
      return TRUE;
   }
   else
      return FALSE;
}
