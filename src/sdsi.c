/* SD -- square dance caller's helper.

    Copyright (C) 1990-1995  William B. Ackerman.

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
   hash_nonrandom_number
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
   open_session
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
extern long_boolean need_new_header_comment;
extern call_list_mode_t call_list_mode;


/* These variables are external. */

int random_number;
int hashed_randoms;
char *database_filename = DATABASE_FILENAME;


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
   return j;
}


extern void hash_nonrandom_number(int number)
{
   hashed_randoms = hashed_randoms*37+number;
}


extern long_boolean generate_random_concept_p(void)
{
   int i = generate_random_number(8) < CONCEPT_PROBABILITY;

   /* Since we are not going to use the random number in a one-to-one way, we run the risk
      of not having hashed_randoms uniquely represent what is happening.  To remedy
      the problem, we hash just the yes-no result of our decision. */

   hash_nonrandom_number(i);
   return i;
}


extern void *get_mem(uint32 siz)
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
extern void *get_mem_gracefully(uint32 siz)
{
   return malloc(siz);
}


extern void *get_more_mem(void *oldp, uint32 siz)
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
extern void *get_more_mem_gracefully(void *oldp, uint32 siz)
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
#if defined(MSDOS)
   int i;
#endif

   file_error = FALSE;

/* If not on a PC, things are fairly simple.  We open the file in "append"
   mode, thankful that we have escaped one of the most monumentally stupid
   pieces of OS system design ever to plague the universe, and only need to
   deal with Un*x, which is merely one of the most monumentally stupid
   pieces of OS system design ever to plague this galaxy. */

#if !defined(MSDOS)

   if (!(fildes = fopen(outfile_string, "a"))) {
      (void) strncpy(fail_errstring, get_errstring(), MAX_ERR_LENGTH);
      (void) strncpy(fail_message, "open", MAX_ERR_LENGTH);
      file_error = TRUE;
      return;
   }

   /* We are positioned at the end, because that's what "a" mode does. */

   this_file_position = ftell(fildes);

   if ((last_file_position != -1) && (last_file_position != this_file_position)) {
      writestuff("Warning -- file has been modified since last sequence.");
      newline();
      newline();
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
      mode (that is "r+"), and seek back to that spot. */

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

   really_just_append:

#endif

   if (this_file_position == 0) {
      writestuff("File does not exist, creating it.");
      newline();
      newline();
   }
   else {
      char formfeed = '\f';

      if (last_file_position == -1) {
         writestuff("Appending to existing file.");
         newline();
         newline();
      }

      if ((fwrite(&formfeed, 1, 1, fildes) != 1) || ferror(fildes)) {
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
extern void print_line(Cstring s)
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


Private long_boolean parse_level(Cstring s, dance_level *levelp)
{
   char first = tolower(s[0]);

   switch (first) {
      case 'm': *levelp = l_mainstream; break;
      case 'p': *levelp = l_plus; break;
      case 'a':
         if (s[1] == '1' && !s[2]) *levelp = l_a1;
         else if (s[1] == '2' && !s[2]) *levelp = l_a2;
         else if (s[1] == 'l' && s[2] == 'l' && !s[3]) *levelp = l_dontshow;
         else
            return FALSE;
         break;
      case 'c':
         if (s[1] == '3' && (s[2] == 'a' || s[2] == 'A') && !s[3])
            *levelp = l_c3a;
         else if (s[1] == '3' && (s[2] == 'x' || s[2] == 'X') && !s[3])
            *levelp = l_c3x;
         else if (s[1] == '4' && (s[2] == 'a' || s[2] == 'A') && !s[3])
            *levelp = l_c4a;
         else if (!s[2]) {
            switch (s[1]) {
               case '1': *levelp = l_c1; break;
               case '2': *levelp = l_c2; break;
               case '3': *levelp = l_c3; break;
               case '4': *levelp = l_c4; break;
               default: return FALSE;
            }
         }
         break;
      default:
         return FALSE;
   }

   return TRUE;
}



/* Stuff used for controlling the session file.  When index is nonzero,
   the session file is in use, and the final state should be written back
   to it at that line. */

static int session_index = 0;        /* If this is nonzero, we have opened a session. */


extern long_boolean open_session(int argc, char **argv)
{
   int session_linenum = 0;
   int session_position;
   int i, j;
   int argno;
   FILE *session;
   char line[MAX_FILENAME_LENGTH];
   char **args;
   int nargs = argc;

   /* Copy the arguments, so that we can use "realloc" to grow the list. */

   args = (char **) get_mem(nargs * sizeof(char *));

   (void) memcpy(args, argv, nargs * sizeof(char *));

   /* Read the initialization file, looking for options. */

   session = fopen(SESSION_FILENAME, "r");

   if (session) {
      /* Search for the "[Options]" indicator. */

      for (;;) {
         if (!fgets(line, MAX_FILENAME_LENGTH, session)) goto no_options;
         if (!strncmp(line, "[Options]", 9)) break;
      }

      for (;;) {
         if (!fgets(line, MAX_FILENAME_LENGTH, session) || line[0] == '\n') break;
         nargs++;
         args = (char **) get_more_mem(args, nargs * sizeof(char *));
         for (i=nargs-1 ; i>1 ; i--) args[i] = args[i-1];
         j = strlen(line);
         if (j>0) line[j-1] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */
         args[1] = (char *) get_mem(j+2);
         args[1][0] = '-';    /* Put a '-' in front of it. */
         (void) memcpy(&args[1][1], line, j+1);
      }

      no_options: ;
   }

   /* This lets the user interface intercept command line arguments that it is interested in. */
   uims_process_command_line(&nargs, &args);

   call_list_mode = call_list_mode_none;
   calling_level = l_mainstream;    /* The default. */

   for (argno=1; argno < nargs; argno++) {
      if (args[argno][0] == '-') {
         call_list_mode_t this_mode_maybe = call_list_mode_none;

         /* Special flag: must be one of
            -write_list <filename>  -- write out the call list for the
                  indicated level INSTEAD OF running the program
            -write_full_list <filename>  -- write out the call list for the
                  indicated level and all lower levels INSTEAD OF running the program
            -abridge <filename>  -- read in the file, strike all the calls
                  contained therein off the menus, and proceed.
            -diagnostic  -- (this is a hidden flag) suppress display of version info */

         if (strcmp(&args[argno][1], "write_list") == 0)
            this_mode_maybe = call_list_mode_writing;
         else if (strcmp(&args[argno][1], "write_full_list") == 0)
            this_mode_maybe = call_list_mode_writing_full;
         else if (strcmp(&args[argno][1], "abridge") == 0)
            this_mode_maybe = call_list_mode_abridging;
         else if (strcmp(&args[argno][1], "diagnostic") == 0)
            { diagnostic_mode = TRUE; continue; }
         else if (strcmp(&args[argno][1], "singlespace") == 0)
            { singlespace_mode = TRUE; continue; }
         else if (strcmp(&args[argno][1], "discard_after_error") == 0)
            { erase_after_error = TRUE; continue; }
         else if (strcmp(&args[argno][1], "active_phantoms") == 0)
            { using_active_phantoms = TRUE; continue; }
         else if (strcmp(&args[argno][1], "sequence") == 0) {
	     if (argno+1 < nargs)
		 strncpy(outfile_string, args[argno+1], MAX_FILENAME_LENGTH);
	 }
         else if (strcmp(&args[argno][1], "db") == 0) {
	     if (argno+1 < nargs)
		 database_filename = args[argno+1];
	 }
         else
            uims_bad_argument("Unknown flag:", args[argno], (char *) 0);

         /* At this point, if "this_mode_maybe" is not null, we have to deal with some
            kind of "write_list" or "abridge" operation.  If not, we have already processed
            the file name, and all that remains is to check its existence and then skip it. */

         argno++;
         if (argno >= nargs)
            uims_bad_argument("This flag must be followed by a file name:", args[argno-1], NULL);

         if (this_mode_maybe != call_list_mode_none) {
            call_list_mode = this_mode_maybe;
            if (open_call_list_file(call_list_mode, args[argno]))
               exit_program(1);
         }
      }
      else if (!parse_level(args[argno], &calling_level)) {
         uims_bad_argument("Unknown calling level argument:", args[argno],
            "Known calling levels: m, p, a1, a2, c1, c2, c3a, c3, c3x, c4a, or c4.");
      }
   }

   /* initialize outfile_string to calling-level-specific default outfile */

   (void) strncat(outfile_string, filename_strings[calling_level], MAX_FILENAME_LENGTH);

   /* If we are writing a call list file, that's all we do. */
   if (call_list_mode == call_list_mode_writing || call_list_mode == call_list_mode_writing_full)
      goto no_session;

   /* Or if the file didn't exist, or we are in diagnostic mode. */
   if (!session || diagnostic_mode) goto no_session;

   if (fseek(session, 0, SEEK_SET))
      goto no_session;

   if (!(session = fopen(SESSION_FILENAME, "r")))
      goto no_session;

   /* Search for the "[Sessions]" indicator. */

   for (;;) {
      if (!fgets(line, MAX_FILENAME_LENGTH, session)) goto no_session;
      if (!strncmp(line, "[Sessions]", 10)) break;
   }

   session_position = ftell(session);

   printf("Do you want to use one of the following sessions?\n\n");
   printf("  0     (no session)\n");

   for (;;) {
      if (!fgets(line, MAX_FILENAME_LENGTH, session) || line[0] == '\n') break;
      printf("%3d  %s", ++session_linenum, line);
   }

   printf("%3d     (create a new session)\n\n", session_linenum+1);

   printf("Enter the number of the desired session:  ");
   if (!gets(line))
      goto no_session;

   if (!sscanf(line, "%d", &session_index) || session_index == 0)
      goto no_session;

   if (session_index < 0) {
      (void) fclose(session);
      return TRUE;    /* Exit the program immediately. */
   }

   if (session_index <= session_linenum) {
      if (!fseek(session, session_position, SEEK_SET)) {
         for (i=0 ; i<session_index ; i++)
            if (!fgets(line, MAX_FILENAME_LENGTH, session)) break;

         if (i == session_index) {
            int ccount;
            char session_levelstring[50];

            if (sscanf(line, "%s %s %d %n", outfile_string, session_levelstring, &sequence_number, &ccount) != 3) {
               printf("Bad format in session file.\n");
               goto no_session;
            }

            #if defined(MSDOS)
               /* What a total crock!!!!!!  The "%n" scan specifier doesn't work. */
               ccount = strlen(line);
               if (ccount > 45) ccount = 45;
            #endif

            if (!parse_level(session_levelstring, &calling_level)) {
               printf("Bad level given in session file.\n");
               goto no_session;
            }

            strncpy(header_comment, &line[ccount], MAX_TEXT_LINE_LENGTH);
            j = strlen(header_comment);
            if (j>0) header_comment[j-1] = '\0';   /* Strip off the <NEWLINE> -- we don't want it. */
         }
      }
   }
   else {
      sequence_number = 1;       /* We are creating a new session to be appended to the file. */
      need_new_header_comment = TRUE;
   }

   goto really_do_it;

   no_session:

   session_index = 0;

   really_do_it:

   if (session) (void) fclose(session);

   return FALSE;
}



extern void final_exit(int code)
{
   if (session_index != 0) {
      char line[MAX_FILENAME_LENGTH];
      FILE *rfile;
      FILE *wfile;
      int i;

      if (rename(SESSION_FILENAME, SESSION2_FILENAME)) {
         printf("Failed to save file '" SESSION_FILENAME "' in '" SESSION2_FILENAME "'\n");
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
                  if (!fputs(line, wfile)) goto copy_failed;
                  if (!strncmp(line, "[Sessions]", 10)) break;
               }

               for (i=0 ; ; i++) {
                  if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) break;
                  if (line[0] == '\n') { more_stuff = TRUE; break; }

                  if (i == session_index-1) {
                     if (fprintf(wfile, "%20-s %11-s %6d      %s\n", outfile_string, getout_strings[calling_level], sequence_number, header_comment) < 0)
                        goto copy_failed;
                  }
                  else if (i == -session_index-1) {
                  }
                  else {
                     if (!fputs(line, wfile)) goto copy_failed;
                  }
               }

               if (i < session_index) {
                  /* User has requested a line number larger than the file.  Append a new line. */
                  if (fprintf(wfile, "%20-s %11-s %6d      %s\n", outfile_string, getout_strings[calling_level], sequence_number, header_comment) < 0)
                     goto copy_failed;
               }

               if (more_stuff) {
                  if (!fputs("\n", wfile)) goto copy_failed;
                  for (;;) {
                     if (!fgets(line, MAX_FILENAME_LENGTH, rfile)) break;
                     if (!fputs(line, wfile)) goto copy_failed;
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

   exit(code);
}

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


extern void write_to_call_list_file(Const char name[])
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
