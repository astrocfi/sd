/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

#include <stdio.h>
#include <errno.h>

static FILE *ifp, *ofp;
static char *outfilename;
static char *infilename;

int write_char(int c)
{
   fputc(c, ofp);

   if (errno) {
      fprintf(stderr, "Can't write output file.\n");
      perror(outfilename);
      return 1;
   }
   return 0;
}


extern int main(int argc, char *argv[])
{
   int last = -1;
   int restrained_blanks = 0;
   int already_good = 0;
   int fixed_it = 0;
   int fixed_tails = 0;
   int naked_crs_end = 0;
   int naked_crs_middle = 0;

   outfilename = argv[2];
   infilename = argv[1];

   if (argc != 3) {
      printf("Usage: crfix <infile> <outfile>\n");
      printf("  Converts text file from Unix newline format to Windows format.\n");
      return 1;
   }

   if (!(ifp = fopen(infilename, "rb"))) {
      fprintf(stderr, "Can't open input file.\n");
      perror(infilename);
      return 1;
   }

   if (!(ofp = fopen(outfilename, "wb"))) {
      fprintf(stderr, "Can't open output file.\n");
      perror(outfilename);
      return 1;
   }

   for ( ;; ) {
      int c;

      errno = 0;
      if ((c = fgetc(ifp)) == EOF) break;

      if (errno) {
         fprintf(stderr, "Can't read input file.\n");
         perror(infilename);
         goto getout;
      }

      if (c == '\n') {
         if (restrained_blanks > 0) fixed_tails++;
         restrained_blanks = 0;

         if (last == '\r') {
            already_good++;
         }
         else {
            fixed_it++;
            if (write_char('\r')) goto getout;
         }
         if (write_char(c)) goto getout;
      }
      else if (c == '\r') {
         if (restrained_blanks > 0) fixed_tails++;
         restrained_blanks = 0;

         if (last == '\r') {
            naked_crs_end++;
         }
         else {
            if (write_char('\r')) goto getout;
         }
      }
      else {
         if (last == '\r') {
            naked_crs_middle++;
            if (write_char('\n')) goto getout;
         }

         if (c == ' ') {
            restrained_blanks++;
         }
         else {
            while (restrained_blanks > 0) {
               if (write_char(' ')) goto getout;
               restrained_blanks--;
            }

            if (write_char(c)) goto getout;
         }
      }
               
      last = c;
   }

   printf("There were %d lines.\n", already_good + fixed_it);

   if (already_good == 0)
      printf("All were changed to Windows format.\n");
   else if (fixed_it == 0)
      printf("All were already in Windows format.\n");
   else
      printf("%d were changed to Windows format.\n", fixed_it);

   if (fixed_tails > 0) printf("%d lines had trailing blanks removed.\n", fixed_tails);
   if (naked_crs_middle > 0) printf("%d naked return (^M) characters in the middle of a line were turned into ^M^J.\n", naked_crs_middle);
   if (naked_crs_end > 0) printf("%d naked return (^M) characters at the end of a line were removed.\n", naked_crs_end);

 getout:

   if (fclose(ifp)) {
      fprintf(stderr, "Can't close input file.\n");
      perror(infilename);
      return 1;
   }

   if (fclose(ofp)) {
      fprintf(stderr, "Can't close output file.\n");
      perror(outfilename);
      return 1;
   }

   return 0;
}
