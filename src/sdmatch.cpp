/*
   sdmatch.c - command matching support

    Copyright (C) 1990-2000  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/*
 * Copyright (C) 1993 Alan Snyder
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The author makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" WITHOUT ANY
 * WARRANTY, without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  
 *
 */

/* This file defines the following functions:
   do_accelerator_spec
   erase_matcher_input
   delete_matcher_word
   matcher_initialize
   matcher_setup_call_menu
   match_user_input
and the following external variables:
   concept_list
   concept_list_length
   level_concept_list
   level_concept_list_length
*/

/* #define TIMING */ /* uncomment to display timing information */

#include <string.h> /* for strcpy */
#include <stdio.h>  /* for sprintf */
#include <ctype.h>  /* for tolower */

#ifdef TIMING
#include <time.h>
#endif

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"


modifier_block *fcn_key_table_normal[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
modifier_block *fcn_key_table_start[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
modifier_block *fcn_key_table_resolve[FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1];
match_result user_match;

long_boolean showing_has_stopped;
match_result GLOB_match;
int GLOB_extended_bracket_depth;
int GLOB_space_ok;
int GLOB_yielding_matches;
char GLOB_user_input[INPUT_TEXTLINE_SIZE+1];     // the current user input
char GLOB_full_extension[INPUT_TEXTLINE_SIZE+1];      // the extension for the current pattern
char GLOB_echo_stuff[INPUT_TEXTLINE_SIZE+1]; // the maximal common extension
int GLOB_user_input_size;        // This is always equal to strlen(GLOB_user_input).

/* Must be a power of 2. */
#define NUM_NAME_HASH_BUCKETS 128
#define BRACKET_HASH (NUM_NAME_HASH_BUCKETS+1)

/* These lists have two extra item at the end:
    The first is for calls whose names can't be hashed.
    The second is the bucket that any string starting with left bracket hashes to. */
static short *call_hash_lists[NUM_NAME_HASH_BUCKETS+2];
static short call_hash_list_sizes[NUM_NAME_HASH_BUCKETS+2];
static short *conc_hash_lists[NUM_NAME_HASH_BUCKETS+2];
static short conc_hash_list_sizes[NUM_NAME_HASH_BUCKETS+2];
static short *conclvl_hash_lists[NUM_NAME_HASH_BUCKETS+2];
static short conclvl_hash_list_sizes[NUM_NAME_HASH_BUCKETS+2];

/* These list all the buckets that selectors can go to. */
static short *selector_hash_list;
static short selector_hash_list_size;

/* These list all the buckets that taggers can go to. */
static short *tagger_hash_list;
static short tagger_hash_list_size;

short int *concept_list;        /* indices of all concepts */
int concept_list_length;
short int *level_concept_list;  /* indices of concepts valid at current level */
int level_concept_list_length;


typedef struct pat2_blockstruct {
   Cstring car;
   concept_descriptor *special_concept;
   match_result *folks_to_restore;
   struct pat2_blockstruct *cdr;
   long_boolean demand_a_call;
} pat2_block;

static modifier_block *modifier_active_list = (modifier_block *) 0;
static modifier_block *modifier_inactive_list = (modifier_block *) 0;

static int static_call_menu;              /* The call menu (or special negative command) that we are searching */




static void scan_concepts_and_calls(
   Cstring user,
   Cstring firstchar,
   pat2_block *pat2,
   const match_result **fixme,
   int patxi);

static void match_wildcard(Cstring user, Cstring pat, pat2_block *pat2,
                           int patxi, concept_descriptor *special);


/* This is statically used by the match_wildcard and match_suffix_2 procedures. */

static match_result *current_result;
static match_result active_result;


static int GLOB_only_extension;         // Only want extension, short-circuit the search.
static int GLOB_user_bracket_depth;
static int GLOB_match_count;            /* the number of matches so far */
static int GLOB_exact_count;            /* the number of exact matches so far */
static int GLOB_exact_match;            /* true if an exact match has been found */
static long_boolean GLOB_showing;       /* we are only showing the matching patterns */
static long_boolean GLOB_verify;        /* true => verify calls before showing */
static int GLOB_lowest_yield_depth;



/* The following array must be coordinated with the Sd program */

static Cstring n_4_patterns[] = {
   "0/4",
   "1/4",
   "2/4",
   "3/4",
   "4/4",
   "5/4",
   (Cstring) 0
};



static int translate_keybind_spec(char key_name[])
{
   int key_length;
   int d1, d2, digits;
   long_boolean shift = FALSE;
   long_boolean ctl = FALSE;
   long_boolean alt = FALSE;
   long_boolean ctlalt = FALSE;

   /* Compress hyphens out, and canonicalize to lower case. */
   for (d1=key_length=0 ; key_name[d1] ; d1++) {
      if (key_name[d1] != '-') key_name[key_length++] = tolower(key_name[d1]);
   }

   if (key_length < 2) return -1;

   switch (key_name[0]) {
   case 's': shift = TRUE; break;
   case 'c':   ctl = TRUE; break;
   case 'a':
   case 'm':   alt = TRUE; break;
   }

   switch (key_name[1]) {
   case 'c':   ctlalt = alt; break;
   case 'a':
   case 'm':   ctlalt = ctl; break;
   }

   d2 = key_name[key_length-1] - '0';
   if (d2 >= 0 && d2 <= 9) {
      digits = d2;
      d1 = key_name[key_length-2] - '0';
      if (d1 >= 0 && d1 <= 9) {
         digits += d1*10;
         key_length--;
      }

      if (key_name[key_length-2] == 'f') {
         if (digits < 1 || digits > 12)
            return -1;
         else if (key_length == 2) {
            return FKEY+digits;
         }
         else if (key_length == 3 && shift) {
            return SFKEY+digits;
         }
         else if (key_length == 3 && ctl) {
            return CFKEY+digits;
         }
         else if (key_length == 3 && alt) {
            return AFKEY+digits;
         }
         else if (key_length == 4 && ctlalt) {
            return CAFKEY+digits;
         }
         else {
            return -1;
         }
      }
      if (key_name[key_length-2] == 'n') {
         if (digits > 9 || key_length < 3)
            return -1;
         else if (key_length == 3 && ctl) {
            return CTLNKP+digits;
         }
         else if (key_length == 3 && alt) {
            return ALTNKP+digits;
         }
         else if (key_length == 4 && ctlalt) {
            return CTLALTNKP+digits;
         }
         else {
            return -1;
         }
      }
      if (key_name[key_length-2] == 'e') {
         if (digits > 15)
            return -1;

         if (key_length == 2) {
            return EKEY+digits;
         }
         else if (key_length == 3 && shift) {
            return SEKEY+digits;
         }
         else if (key_length == 3 && ctl) {
            return CEKEY+digits;
         }
         else if (key_length == 3 && alt) {
            return AEKEY+digits;
         }
         else if (key_length == 4 && ctlalt) {
            return CAEKEY+digits;
         }
         else {
            return -1;
         }
      }
      else if (key_length == 2 && ctl) {
         return CTLDIG+digits;
      }
      else if (key_length == 2 && alt) {
         return ALTDIG+digits;
      }
      else if (key_length == 3 && ctlalt) {
         return CTLALTDIG+digits;
      }
      else { 
         return -1;
      }
   }
   else if (key_name[key_length-1] >= 'a' && key_name[key_length-1] <= 'z') {
      if (key_length == 2 && ctl) {
         return CTLLET+key_name[key_length-1]+'A'-'a';
      }
      else if (key_length == 2 && alt) {
         return ALTLET+key_name[key_length-1]+'A'-'a';
      }
      else if (key_length == 3 && ctlalt) {
         return CTLALTLET+key_name[key_length-1]+'A'-'a';
      }
      else {
         return -1;
      }
   }
   else
      return -1;
}


SDLIB_API void do_accelerator_spec(Cstring qq)
{
   char key_name[MAX_FILENAME_LENGTH];
   char junk_name[MAX_FILENAME_LENGTH];
   char errbuf[255];
   modifier_block **table_thing;
   modifier_block *newthing;
   int ccount;
   int matches;
   int menu_type = call_list_any;
   int keybindcode = -1;

   if (!qq[0] || qq[0] == '#') return;   /* This is a blank line or a comment. */

   if (sscanf(qq, "%s %n%s", key_name, &ccount, junk_name) == 2) {
      if (key_name[0] == '+') {
         menu_type = match_startup_commands;
         keybindcode = translate_keybind_spec(&key_name[1]);
      }
      else if (key_name[0] == '*') {
         menu_type = match_resolve_commands;
         keybindcode = translate_keybind_spec(&key_name[1]);
      }
      else
         keybindcode = translate_keybind_spec(key_name);
   }

   if (keybindcode < 0) {
      (*the_callback_block.uims_database_error_fn)("Bad format in key binding.", qq);
      return;
   }

   user_match.match.kind = ui_call_select;

   if (!strcmp(&qq[ccount], "deleteline")) {
      user_match.match.index = special_index_deleteline;
   }
   else if (!strcmp(&qq[ccount], "deleteword")) {
      user_match.match.index = special_index_deleteword;
   }
   else if (!strcmp(&qq[ccount], "copytext")) {
      user_match.match.index = special_index_copytext;
   }
   else if (!strcmp(&qq[ccount], "cuttext")) {
      user_match.match.index = special_index_cuttext;
   }
   else if (!strcmp(&qq[ccount], "pastetext")) {
      user_match.match.index = special_index_pastetext;
   }
   else if (!strcmp(&qq[ccount], "lineup")) {
      user_match.match.index = special_index_lineup;
   }
   else if (!strcmp(&qq[ccount], "linedown")) {
      user_match.match.index = special_index_linedown;
   }
   else if (!strcmp(&qq[ccount], "pageup")) {
      user_match.match.index = special_index_pageup;
   }
   else if (!strcmp(&qq[ccount], "pagedown")) {
      user_match.match.index = special_index_pagedown;
   }
   else if (!strcmp(&qq[ccount], "quoteanything")) {
      user_match.match.index = special_index_quote_anything;
   }
   else {
      strcpy(GLOB_user_input, &qq[ccount]);
      GLOB_user_input_size = strlen(GLOB_user_input);
      matches = match_user_input(menu_type, FALSE, FALSE, FALSE);
      user_match = GLOB_match;

      if ((matches != 1 && matches - GLOB_yielding_matches != 1 && !user_match.exact)) {
         /* Didn't find the target of the key binding.  Below C4X, failure to find
            something could just mean that it was a call off the list.  At C4X, we
            take it seriously.  So the initialization file should always be tested at C4X. */
         if (calling_level >= l_c4x)
            (*the_callback_block.uims_database_error_fn)("Didn't find target of key binding.", qq);

         return;
      }

      if (user_match.match.packed_next_conc_or_subcall ||
          user_match.match.packed_secondary_subcall) {
         (*the_callback_block.uims_database_error_fn)("Target of key binding is too complicated.", qq);
         return;
      }
   }

   if (user_match.match.kind == ui_start_select) {
      table_thing = &fcn_key_table_start[keybindcode-FCN_KEY_TAB_LOW];
   }
   else if (user_match.match.kind == ui_resolve_select) {
      table_thing = &fcn_key_table_resolve[keybindcode-FCN_KEY_TAB_LOW];
   }
   else if (user_match.match.kind == ui_concept_select ||
            user_match.match.kind == ui_call_select ||
            user_match.match.kind == ui_command_select) {
      table_thing = &fcn_key_table_normal[keybindcode-FCN_KEY_TAB_LOW];
   }
   else {
      (*the_callback_block.uims_database_error_fn)("Anomalous key binding.", qq);
      return;
   }

   newthing = (modifier_block *) (*the_callback_block.get_mem_fn)(sizeof(modifier_block));
   *newthing = user_match.match;

   if (*table_thing) {
      sprintf(errbuf, "Redundant key binding.");
      (*the_callback_block.uims_database_error_fn)(errbuf, qq);
      return;
   }

   *table_thing = newthing;
}



/* This returns zero if the name was clearly not able to be hashed
   (contains NUL, comma, atsign, or single quote).  It returns 1
   if it could clearly be hashed (no blanks).  It returns 2 if it
   had blanks, and might be questionable.  Patterns to match
   (call or concept names, etc. can have blanks in them and still
   be hashed.  Strings the user types can't be hashed if they have blanks. */
static int get_hash(Cstring string, int *bucket_p)
{
   char c1 = string[0];
   char c2 = string[1];
   char c3 = string[2];
   int bucket;

   if (c1 == '<' && ((int) c2 & ~32) == 'A' && ((int) c3 & ~32) == 'N') {
      *bucket_p = BRACKET_HASH;
      return 1;
   }
   else if (
               (c1 && c1 != ',' && c1 != '@' && c1 != '\'') &&
               (c2 && c2 != ',' && c2 != '@' && c2 != '\'') &&
               (c3 && c3 != ',' && c3 != '@' && c3 != '\'')) {
      /* We use a hash function that ignores the "32" bit, so it is insensitive to case. */
      bucket = (((((int) c1 & ~32) << 3) + ((int) c2 & ~32)) << 3) + ((int) c3 & ~32);
      bucket += bucket * 20;
      bucket += bucket >> 7;
      bucket &= (NUM_NAME_HASH_BUCKETS-1);
      *bucket_p = bucket;

      if (c1 != ' ' && c2 != ' ' && c3 != ' ') return 1;
      else return 2;
   }
   else
      return 0;
}


static void hash_me(int bucket, int i)
{
   call_hash_list_sizes[bucket]++;
   call_hash_lists[bucket] = (short *)
      (*the_callback_block.get_more_mem_fn)(call_hash_lists[bucket],
                   call_hash_list_sizes[bucket] * sizeof(short));
   call_hash_lists[bucket][call_hash_list_sizes[bucket]-1] = i;
}


SDLIB_API void erase_matcher_input()
{
   GLOB_user_input[0] = '\0';
   GLOB_user_input_size = 0;
}


SDLIB_API int delete_matcher_word()
{
   long_boolean deleted_letter = FALSE;
   int orig_size = GLOB_user_input_size;

   while (GLOB_user_input_size > 0) {
      if (GLOB_user_input[GLOB_user_input_size-1] == ' ') {
         if (deleted_letter) break;
      }
      else
         deleted_letter = TRUE;

      GLOB_user_input_size--;
      GLOB_user_input[GLOB_user_input_size] = '\0';
   }

   return orig_size-GLOB_user_input_size;
}


/* Call MATCHER_INITIALIZE first.
   This function sets up the concept list.  The concepts are found
   in the external array concept_descriptor_table.  For each
   i, the field concept_descriptor_table[i].name has the text that we
   should display for the user.
*/
SDLIB_API void matcher_initialize()
{
   int i, j;
   int concept_number;
   concept_descriptor *p;
   short int *item, *level_item;
   concept_kind end_marker = concept_diagnose;

   /* Decide whether we allow the "diagnose" concept, by deciding
      when we will stop the concept list scan. */
   if (diagnostic_mode) end_marker = marker_end_of_list;

   (void) memset(fcn_key_table_normal, 0,
                 sizeof(modifier_block *) * (FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1));
   (void) memset(fcn_key_table_start, 0,
                 sizeof(modifier_block *) * (FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1));
   (void) memset(fcn_key_table_resolve, 0,
                 sizeof(modifier_block *) * (FCN_KEY_TAB_LAST-FCN_KEY_TAB_LOW+1));

   /* Count the number of concepts to put in the lists. */
   
   concept_list_length = 0;
   level_concept_list_length = 0;
   for (concept_number=0;;concept_number++) {
      p = &concept_descriptor_table[concept_number];
      if (p->kind == end_marker) break;

      if (p->kind == concept_comment || (p->concparseflags & CONCPARSE_MENU_DUP))
         continue;

      concept_list_length++;

      if (p->level <= higher_acceptable_level[calling_level])
         level_concept_list_length++;
   }

   /* create the concept lists */

   concept_list = (short int *)
      (*the_callback_block.get_mem_fn)(sizeof(short int) * concept_list_length);
   level_concept_list = (short int *)
      (*the_callback_block.get_mem_fn)(sizeof(short int) * level_concept_list_length);

   item = concept_list;
   level_item = level_concept_list;
   for (concept_number=0 ; ; concept_number++) {
      p = &concept_descriptor_table[concept_number];
      if (p->kind == end_marker) {
         break;
      }
      if (p->kind == concept_comment || (p->concparseflags & CONCPARSE_MENU_DUP)) {
         continue;
      }

      *item = concept_number;
      item++;

      if (p->level <= higher_acceptable_level[calling_level]) {
         *level_item = concept_number;
         level_item++;
      }
   }

   /* Initialize the hash buckets for call names. */

   {
      int bucket;
      uint32 ku;

      /* First, do the selectors.  Before that, be sure "<anyone>" is hashed. */

      selector_hash_list = (short *) (*the_callback_block.get_mem_fn)(sizeof(short));
      selector_hash_list_size = 1;

      if (!get_hash("<an", &bucket)) {
         (*the_callback_block.uims_database_error_fn)("Can't hash selector base!", (Cstring) 0);
         (*the_callback_block.exit_program_fn)(2);
      }

      selector_hash_list[0] = bucket;

      for (i=1; i<selector_enum_extent; i++) {
         if (!get_hash(selector_list[i].name, &bucket)) {
            char errbuf[255];
            sprintf(errbuf, "Can't hash selector %d - 1!", i);
            (*the_callback_block.uims_database_error_fn)(errbuf, (Cstring) 0);
            (*the_callback_block.exit_program_fn)(2);
         }

         /* See if this bucket is already accounted for. */

         for (j=0; j<selector_hash_list_size; j++) {
            if (selector_hash_list[j] == bucket) goto already_in1;
         }

         selector_hash_list_size++;
         selector_hash_list = (short *)
            (*the_callback_block.get_more_mem_fn)(selector_hash_list, selector_hash_list_size * sizeof(short));
         selector_hash_list[selector_hash_list_size-1] = bucket;

         /* Now do it again for the singular names. */

         already_in1:

         if (!get_hash(selector_list[i].sing_name, &bucket)) {
            char errbuf[255];
            sprintf(errbuf, "Can't hash selector %d - 2!", i);
            (*the_callback_block.uims_database_error_fn)(errbuf, (Cstring) 0);
            (*the_callback_block.exit_program_fn)(2);
         }

         for (j=0; j<selector_hash_list_size; j++) {
            if (selector_hash_list[j] == bucket) goto already_in2;
         }

         selector_hash_list_size++;
         selector_hash_list = (short *)
            (*the_callback_block.get_more_mem_fn)(selector_hash_list, selector_hash_list_size * sizeof(short));
         selector_hash_list[selector_hash_list_size-1] = bucket;

         already_in2: ;
      }

      /* Next, do the taggers.  Before that, be sure "<atc>" is hashed. */

      tagger_hash_list = (short *) (*the_callback_block.get_mem_fn)(sizeof(short));
      tagger_hash_list_size = 1;

      if (!get_hash("<at", &bucket)) {
         (*the_callback_block.uims_database_error_fn)("Can't hash tagger base!", (Cstring) 0);
         (*the_callback_block.exit_program_fn)(2);
      }

      tagger_hash_list[0] = bucket;

      for (i=0; i<NUM_TAGGER_CLASSES; i++) {
         for (ku=0; ku<number_of_taggers[i]; ku++) {
            if (!get_hash(tagger_calls[i][ku]->name, &bucket)) {
               char errbuf[255];
               sprintf(errbuf, "Can't hash tagger %d %d!", i, (int) ku);
               (*the_callback_block.uims_database_error_fn)(errbuf, (Cstring) 0);
               (*the_callback_block.exit_program_fn)(2);
            }
   
            for (j=0; j<tagger_hash_list_size; j++) {
               if (tagger_hash_list[j] == bucket) goto already_in3;
            }
   
            tagger_hash_list_size++;
            tagger_hash_list = (short *)
               (*the_callback_block.get_more_mem_fn)(tagger_hash_list,
                            tagger_hash_list_size * sizeof(short));
            tagger_hash_list[tagger_hash_list_size-1] = bucket;
   
            already_in3: ;
         }
      }

      /* Now do the calls. */

      for (i=0 ; i<NUM_NAME_HASH_BUCKETS+2 ; i++) {
         call_hash_lists[i] = (short *) 0;
         conc_hash_lists[i] = (short *) 0;
         conclvl_hash_lists[i] = (short *) 0;
         call_hash_list_sizes[i] = 0;
         conc_hash_list_sizes[i] = 0;
         conclvl_hash_list_sizes[i] = 0;
      }

      for (i=0; i<number_of_calls[call_list_any]; i++) {
         Cstring name = main_call_lists[call_list_any][i]->name;

         doitagain:

         if (name[0] == '@') {
            if (name[1] == '6' || name[1] == 'k') {
               // This is a call like "<anyone> run".  Put it into every bucket
               // that could match a selector. */
   
               for (j=0 ; j<selector_hash_list_size ; j++)
                  hash_me(selector_hash_list[j], i);

               continue;
            }
            else if (name[1] == 'v' || name[1] == 'w' || name[1] == 'x' || name[1] == 'y') {
               /* This is a call like "<atc> your neighbor".
                  Put it into every bucket that could match a tagger. */
   
               for (j=0 ; j<tagger_hash_list_size ; j++)
                  hash_me(tagger_hash_list[j], i);

               continue;
            }
            else if (name[1] == '0' || name[1] == 'm') {
               // We act as though any string starting with "[" hashes to BRACKET_HASH.
               hash_me(BRACKET_HASH, i);
               continue;
            }
            else if (name[1] == 'e') {
               // If this is "@e", hash it to both "left" and to whatever naturally follows.
               (void) get_hash("left", &bucket);
               hash_me(bucket, i);
               name += 2;
               goto doitagain;
            }
            else if (!get_escape_string(name[1])) {
               // If this escape is something like "@2", as in "@2scoot and plenty",
               // ignore it.  Hash it under "scoot and plenty".
               name += 2;
               goto doitagain;
            }
         }

         if (get_hash(name, &bucket)) {
            hash_me(bucket, i);
            continue;
         }

         /* If we get here, this call needs to be put into the extra bucket at the end,
            and also into EVERY OTHER BUCKET!!!! */
         for (bucket=0 ; bucket < NUM_NAME_HASH_BUCKETS+1 ; bucket++)
            hash_me(bucket, i);
      }

      /* Now do the concepts from the big list. */

      item = concept_list;

      for (i=0; i<concept_list_length; i++,item++) {
         Cstring name = concept_descriptor_table[*item].name;

         if (name[0] == '@' && (name[1] == '6' || name[1] == 'k')) {
            /* This is a call like "<anyone> run".  Put it into every bucket that could match a selector. */

            for (j=0 ; j<selector_hash_list_size ; j++) {
               bucket = selector_hash_list[j];
               conc_hash_list_sizes[bucket]++;
               conc_hash_lists[bucket] = (short *)
                  (*the_callback_block.get_more_mem_fn)(conc_hash_lists[bucket],
                               conc_hash_list_sizes[bucket] * sizeof(short));
               conc_hash_lists[bucket][conc_hash_list_sizes[bucket]-1] = *item;
            }
            continue;
         }
         else if (get_hash(name, &bucket)) {
            conc_hash_list_sizes[bucket]++;
            conc_hash_lists[bucket] = (short *)
               (*the_callback_block.get_more_mem_fn)(conc_hash_lists[bucket],
                            conc_hash_list_sizes[bucket] * sizeof(short));
            conc_hash_lists[bucket][conc_hash_list_sizes[bucket]-1] = *item;
            continue;
         }

         /* If we get here, this concept needs to be put into the extra bucket at the end,
            and also into EVERY OTHER BUCKET!!!! */
         for (bucket=0 ; bucket < NUM_NAME_HASH_BUCKETS+1 ; bucket++) {
            conc_hash_list_sizes[bucket]++;
            conc_hash_lists[bucket] = (short *)
               (*the_callback_block.get_more_mem_fn)(conc_hash_lists[bucket],
                            conc_hash_list_sizes[bucket] * sizeof(short));
            conc_hash_lists[bucket][conc_hash_list_sizes[bucket]-1] = *item;
         }
      }

      /* Now do the "level concepts". */

      item = level_concept_list;

      for (i=0; i<level_concept_list_length; i++,item++) {
         Cstring name = concept_descriptor_table[*item].name;

         if (name[0] == '@' && (name[1] == '6' || name[1] == 'k')) {
            /* This is a call like "<anyone> run".  Put it into every bucket that could match a selector. */

            for (j=0 ; j<selector_hash_list_size ; j++) {
               bucket = selector_hash_list[j];
               conclvl_hash_list_sizes[bucket]++;
               conclvl_hash_lists[bucket] = (short *)
                  (*the_callback_block.get_more_mem_fn)(conclvl_hash_lists[bucket],
                               conclvl_hash_list_sizes[bucket] * sizeof(short));
               conclvl_hash_lists[bucket][conclvl_hash_list_sizes[bucket]-1] = *item;
            }
            continue;
         }
         else if (get_hash(name, &bucket)) {
            conclvl_hash_list_sizes[bucket]++;
            conclvl_hash_lists[bucket] = (short *)
               (*the_callback_block.get_more_mem_fn)(conclvl_hash_lists[bucket],
                            conclvl_hash_list_sizes[bucket] * sizeof(short));
            conclvl_hash_lists[bucket][conclvl_hash_list_sizes[bucket]-1] = *item;
            continue;
         }

         /* If we get here, this concept needs to be put into the extra bucket at the end,
            and also into EVERY OTHER BUCKET!!!! */
         for (bucket=0 ; bucket < NUM_NAME_HASH_BUCKETS+1 ; bucket++) {
            conclvl_hash_list_sizes[bucket]++;
            conclvl_hash_lists[bucket] = (short *)
               (*the_callback_block.get_more_mem_fn)(conclvl_hash_lists[bucket],
                            conclvl_hash_list_sizes[bucket] * sizeof(short));
            conclvl_hash_lists[bucket][conclvl_hash_list_sizes[bucket]-1] = *item;
         }
      }
   }
}



SDLIB_API void matcher_setup_call_menu(call_list_kind cl)
{
}

/*
 * Call Verification
 */



/* These variables are actually local to verify_call, but they are
   expected to be preserved across the throw, so they must be static. */
static parse_block *parse_mark;
static call_list_kind savecl;

/*
 * Return TRUE if the specified call appears to be legal in the
 * current context.
 */

static long_boolean verify_call(void)
{
   // If we are not verifying, we return TRUE immediately,
   // thereby causing the item to be listed.
   if (!GLOB_verify) return TRUE;

   long_boolean resultval = TRUE;

   interactivity = interactivity_verify;   /* so deposit_call doesn't ask user for info */
   warning_info saved_warnings = history[history_ptr+1].warnings;
   int old_history_ptr = history_ptr;

   parse_mark = mark_parse_blocks();
   save_parse_state();
   savecl = parse_state.call_list_to_use;

   start_sel_dir_num_iterator();

 try_another_selector:

   selector_used = FALSE;
   direction_used = FALSE;
   number_used = FALSE;
   mandatory_call_used = FALSE;
   verify_used_number = FALSE;
   verify_used_selector = FALSE;
   verify_used_direction = FALSE;

   // Do the call.  An error will signal and go to failed.

   try {
      long_boolean theres_a_call_in_here = FALSE;
      parse_block *save1 = (parse_block *) 0;
      modifier_block *anythings = &GLOB_match.match;

      (void) restore_parse_state();

      /* This stuff is duplicated in uims_get_call_command in sdui-tty.c . */

      while (anythings) {
         call_conc_option_state save_stuff = GLOB_match.match.call_conc_options;

            /* First, if we have already deposited a call, and we see more stuff, it must be
               concepts or calls for an "anything" subcall. */

         if (save1) {
            parse_block *tt = get_parse_block();
            save1->concept = &marker_concept_mod;
            save1->next = tt;
            tt->concept = &marker_concept_mod;
            tt->call = base_calls[base_call_null];
            tt->call_to_print = tt->call;
            tt->replacement_key = DFM1_CALL_MOD_MAND_ANYCALL/DFM1_CALL_MOD_BIT;
            parse_state.concept_write_ptr = &tt->subsidiary_root;
            save1 = (parse_block *) 0;
         }

         GLOB_match.match.call_conc_options = anythings->call_conc_options;

         if (anythings->kind == ui_call_select) {
            verify_options = anythings->call_conc_options;
            if ((*the_callback_block.deposit_call_fn)(anythings->call_ptr, &anythings->call_conc_options)) goto failed;
            save1 = *parse_state.concept_write_ptr;
            theres_a_call_in_here = TRUE;
         }
         else if (anythings->kind == ui_concept_select) {
            verify_options = anythings->call_conc_options;
            if ((*the_callback_block.deposit_concept_fn)(anythings->concept_ptr)) goto failed;
         }
         else break;   /* Huh? */

         GLOB_match.match.call_conc_options = save_stuff;
         anythings = anythings->packed_next_conc_or_subcall;
      }

      parse_state.call_list_to_use = savecl;         /* deposit_concept screwed it up */

         /* If we didn't see a call, the user is just verifying
            a string of concepts. Accept it. */

      if (!theres_a_call_in_here) goto accept;

      /* If the parse stack is nenempty, a subsidiary call is needed and hasn't been filled in.
            Therefore, the parse tree is incomplete.  We can print such parse trees, but we
            can't execute them.  So we just assume the call works. */

      if (parse_state.parse_stack_index != 0) goto accept;

      toplevelmove();   // This might throw an error.
      goto accept;
   }
   catch(error_flag_type) {
      // A call failed.  If the call had some mandatory substitution, pass it anyway.

      if (mandatory_call_used) goto accept;

      // Or a bad choice of selector or number may be the cause.
      // Try again with a different selector, until we run out of ideas.

      if (iterate_over_sel_dir_num(verify_used_selector,
                                   verify_used_direction,
                                   verify_used_number))
         goto try_another_selector;

      goto failed;
   }

   failed:
      resultval = FALSE;

   accept:

   (void) restore_parse_state();
   release_parse_blocks_to_mark(parse_mark);

   history_ptr = old_history_ptr;
   history[history_ptr+1].warnings = saved_warnings;
   interactivity = interactivity_normal;

   return resultval;
}


static void copy_sublist(Const match_result *outbar, modifier_block *tails)
{
   if (outbar->real_next_subcall) {
      modifier_block *out;
      Const match_result *newoutbar = outbar->real_next_subcall;

      if (modifier_inactive_list) {
         out = modifier_inactive_list;
         modifier_inactive_list = out->gc_ptr;
      }
      else
         out = (modifier_block *) (*the_callback_block.get_mem_fn)(sizeof(modifier_block));

      *out = newoutbar->match;
      out->packed_next_conc_or_subcall = (modifier_block *) 0;
      out->packed_secondary_subcall = (modifier_block *) 0;
      out->gc_ptr = modifier_active_list;
      modifier_active_list = out;
      tails->packed_next_conc_or_subcall = out;
      copy_sublist(newoutbar, out);
   }

   if (outbar->real_secondary_subcall) {
      modifier_block *out;
      Const match_result *newoutbar = outbar->real_secondary_subcall;

      if (modifier_inactive_list) {
         out = modifier_inactive_list;
         modifier_inactive_list = out->gc_ptr;
      }
      else
         out = (modifier_block *) (*the_callback_block.get_mem_fn)(sizeof(modifier_block));

      *out = newoutbar->match;
      out->packed_next_conc_or_subcall = (modifier_block *) 0;
      out->packed_secondary_subcall = (modifier_block *) 0;
      out->gc_ptr = modifier_active_list;
      modifier_active_list = out;
      tails->packed_secondary_subcall = out;
      copy_sublist(newoutbar, out);
   }
}


/*
 * Record a match.  Extension is how the current input would be extended to
 * match the current pattern.  Result is the value of the
 * current pattern.  Special case: if user input is empty, extension is
 * not set.
 */
    
static void record_a_match(void)
{
   int old_yield = GLOB_lowest_yield_depth;
   GLOB_extended_bracket_depth = 0;

   if (!GLOB_showing) {
      char *s1 = GLOB_echo_stuff;
      const char *s2 = GLOB_full_extension;

      if (GLOB_match_count == 0) {
         // This is the first match.  Set GLOB_echo_stuff to the
         // full extension that we have.  Count brackets.
         for ( ; ; s1++,s2++) {
            *s1 = *s2;
            if (!*s1) break;
            else if (*s1 == '[') GLOB_extended_bracket_depth++;
            else if (*s1 == ']') GLOB_extended_bracket_depth--;
         }
      }
      else {
         // Shorten GLOB_echo_stuff to the maximal common prefix.
         // Count brackets.
         for ( ; ; s1++,s2++) {
            if (!*s1) break;
            else if (*s1 != *s2) {
               *s1 = 0;
               break;
            }
            else if (*s1 == '[') GLOB_extended_bracket_depth++;
            else if (*s1 == ']') GLOB_extended_bracket_depth--;
         }
      }
   }

   // Copy if we are doing the "show" operation, whether we are verifying or not.
   if (GLOB_showing) GLOB_match = active_result;

   // Always copy the first match.
   // Also, always copy, and process modifiers, if we are processing the
   // "verify" operation.  Also, copy the first exact match, and any
   // exact match that isn't yielding relative to what we have so far.

   if (GLOB_match_count == 0 ||
       GLOB_verify ||
       (*GLOB_full_extension == '\0' &&
        (GLOB_exact_count == 0 || current_result->yield_depth <= old_yield))) {
      GLOB_match = active_result;
      GLOB_lowest_yield_depth = current_result->yield_depth;

      /* We need to copy the modifiers to reasonably stable storage. */

      copy_sublist(&GLOB_match, &GLOB_match.match);
   }

   if (*GLOB_full_extension == '\0') {
      GLOB_exact_count++;
      GLOB_match.exact = TRUE;
   }

   GLOB_match_count++;

   if (current_result->yield_depth > old_yield)
      GLOB_yielding_matches++;

   if (GLOB_showing) {
      if (verify_call())
         (*the_callback_block.show_match_fn)();
   }
}


/* ************************************************************************

This procedure must obey certain properties.  Be sure that no modifications
to this procedure compromise this, or else fix the stuff in
"scan_concepts_and_calls" that depends on it.

Theorem 1 (pure calls):

   If user != nil
         AND
   pat1[0..2] does not contain NUL, comma, atsign, or apostrophe
         AND
   user[0..2] does not contain NUL or blank
         AND
   user[0..2] does not match pat1[0..2] case insensitively,
         THEN
   this procedure will do nothing.

Theorem 2 (<anyone> calls):

   If user != nil
         AND
   pat1[0..1] = "@6" or "@k",
         AND
   user[0..2] does not contain NUL or blank
         AND
   user[0..2] does not match any selector name
      (note that we have already determined that no selector
      name has NUL, comma, atsign, or apostrophe in the first 3
      characters, and have determined what hash buckets they must
      lie in)
         AND
   user[0..2] does not match "<an" case insensitively,
         THEN
   this procedure will do nothing.

Theorem 3 (<atc> calls):

   If user != nil
         AND
   pat1[0..1] = "@v", "@w", "@x" or "@y",
         AND
   user[0..2] does not contain NUL or blank
         AND
   user[0..2] does not match any tagger name
      (note that we have already determined that no tagger
      name has NUL, comma, atsign, or apostrophe in the first 3
      characters, and have determined what hash buckets they must
      lie in)
         AND
   user[0..2] does not match "<at" case insensitively,
         THEN
   this procedure will do nothing.

Theorem 4 (<anything> calls):

   If user != nil
         AND
   pat1[0..1] = "@0" or "@m",
         AND
   user[0] is not left bracket or NUL
***** under what circumstances is it true even if user[0] = NUL???????
Well, we need GLOB_showing off, and pat2, if non-nil, must have folks_to_restore = nil
then it won't do anything except record bunches of extra partial matches (but no exact ones),
and generally mess around.
         AND
   user[0..2] does not match "<an" case insensitively,
         THEN
   this procedure will do nothing.

Theorem A (prefix mismatch):

   If user != nil AND user[0] is not NUL
         AND
   pat1 = the single character left bracket or blank (this theorem
      could apply to many other characters, but these are the only
      ones that arise)
         AND
   user[0] != pat1[0]
         THEN
   this procedure will do nothing.

Theorem B (prefix match):

   If user != nil AND user[0] is not NUL
         AND
   pat1 = the single character left bracket or blank
         AND
   user[0] = pat1[0]
         AND
   pat2 is not nil
         AND
   pat2->demand_a_call = FALSE   (don't really need this, since it's doing nothing!)
         AND
   pat2->folks_to_restore = nil
         AND
   this procedure would do nothing if called with
      pat2->car in place of pat1 and the remaining characters of user.
         THEN
   this procedure will do nothing.

************************************************************************ */

static void match_suffix_2(Cstring user, Cstring pat1, pat2_block *pat2, int patxi)
{
   concept_descriptor *pat2_concept = (concept_descriptor *) 0;

   if (pat2->special_concept &&
       !(pat2->special_concept->concparseflags & CONCPARSE_PARSE_DIRECT))
      pat2->special_concept = (concept_descriptor *) 0;

   for (;;) {
      if (*pat1 == 0) {
         /* PAT1 has run out, get a string from PAT2 */
         if (pat2_concept) {
            if (user) {
               // We have processed a concept.  Scan for concepts and calls after same.
               current_result->match.concept_ptr = pat2_concept;
               scan_concepts_and_calls(user, " ", pat2,
                                       &current_result->real_next_subcall, patxi);
            }
            pat2 = (pat2_block *) 0;
            pat2_concept = (concept_descriptor *) 0;
         }
         else if (pat2) {
            // We don't allow a closing bracket after a concept.  That is,
            // stuff in brackets must be zero or more concepts PLUS A CALL.

            if (current_result->match.kind != ui_call_select && pat2->demand_a_call)
               return;

            if (pat2->folks_to_restore) {
               // Be sure maximum yield depth gets propagated back.
               pat2->folks_to_restore->yield_depth = current_result->yield_depth;
               current_result = pat2->folks_to_restore;
            }

            pat1 = pat2->car;
            pat2_concept = pat2->special_concept;
            pat2 = pat2->cdr;
            continue;
         }
      }

      if (user && (*user == '\0')) {
         // We have just reached the end of the user input.
         Cstring p = pat1;

         while (p[0] == '@') {
            switch (p[1]) {
               case 'S':
                  GLOB_space_ok = TRUE;
                  // FALL THROUGH!
               case 'M':
               case 'I':
               case 'C':
               case 'G':
               case 'r':
               case 's':
                  // FELL THROUGH!
                  p += 2;
                  continue;
               case 'n':
                  p += 2;
                  while (*p++ != '@');
                  p++;
                  continue;
            }
            break;
         }

         while (p[0] == ',' || p[0] == '\'') p++;

         if (*p == ' ' || *p == '-')
            GLOB_space_ok = TRUE;

         if (!pat2 && *pat1 == '\0') {
            // Exact match.
            GLOB_full_extension[patxi] = '\0';
            record_a_match();
            break;
         }

         // We need to look at the rest of the pattern because
         // if it contains wildcards, then there are multiple matches.
         user = (Cstring) 0;
      }
      else {
         char p = *pat1++;

         // Check for expansion of wildcards.  But if we are just listing
         // the matching commands, there is no point in expanding wildcards
         // that are past the part that matches the user input.

         if (p == '@') {
            match_wildcard(user, pat1, pat2, patxi, pat2_concept);
   
            if (user==0) {
               // User input has run out, just looking for more wildcards.
   
               Cstring ep = get_escape_string(*pat1++);

               if (ep && *ep) {
                  (void) strcpy(&GLOB_full_extension[patxi], ep);
                  patxi += strlen(ep);
               }
               else {
                  if (ep) {
                     while (*pat1++ != '@');
                     pat1++;
                  }

                  // Don't write duplicate blanks.
                  if (*pat1 == ' ' && patxi > 0 && GLOB_full_extension[patxi-1] == ' ') pat1++;
               }
            }
            else {
               char u = *user++;
               Cstring ep = get_escape_string(*pat1++);

               if (u == '<') {
                  if (ep && *ep) {
                     int i;

                     for (i=1; ep[i]; i++) {
                        if (!user[i-1]) {
                           while (ep[i]) { GLOB_full_extension[patxi++] = ep[i] ; i++; }
                           user = 0;
                           goto cont;
                        }
   
                        if (user[i-1] != tolower(ep[i])) return;
                     }
   
                     user += strlen((char *) ep)-1;
                     cont: ;
                  }
                  else
                     break;
               }
               else {
                  if (ep && *ep)
                     break;   // Pattern has "<...>" thing, but user didn't type "<".
                  else {
                     if (ep) {
                        while (*pat1++ != '@');
                        pat1++;
                     }

                     user--;    /* Back up! */

                     // Check for something that would cause the pattern effectively to have
                     // two consecutive blanks, and compress them to one.  This can happen
                     // if the pattern string has lots of confusing '@' escapes.
                     if (*pat1 == ' ' && user[-1] == ' ') pat1++;
                  }
               }
            }
         }
         else {
            if (user==0) {
               // User input has run out, just looking for more wildcards.
   
               if (p) {
                  // There is more pattern.
                  GLOB_full_extension[patxi++] = tolower(p);
               }
               else {
                  // Reached the end of the pattern.
   
                  GLOB_full_extension[patxi] = '\0';
   
                  if (!pat2) {
                     record_a_match();
                     break;
                  }
               }
            }
            else {
               char u = *user++;

               if (u != p && (p > 'Z' || p < 'A' || u != p+'a'-'A')) {
                  // If user said "wave based" instead of "wave-based", just continue.
                  if (p != '-' || u != ' ') {
                     // Also, if user left out apostrophe or comma, just continue.
                     if (p == '\'' || p == ',') user--;
                     else break;
                  }
               }
            }
         }
      }
   }
}


#define SPIFFY_PARSER

#ifdef SPIFFY_PARSER
static const int spiffy_parser = 1;
#else
static const int spiffy_parser = 0;
#endif




static void scan_concepts_and_calls(
   Cstring user,
   Cstring firstchar,
   pat2_block *pat2,
   const match_result **fixme,
   int patxi)
{
   match_result local_result = *current_result;
   match_result *saved_cur_res_ptr = current_result;
   if (firstchar[0] == '[')
      local_result.recursion_depth++;

   *fixme = &local_result;

   int i;
   int bucket;
   int new_depth;
   long_boolean using_hash = FALSE;

   /* We force any call invoked under a concept to yield if it is ambiguous.  This way,
      if the user types "cross roll", preference will be given to the call "cross roll",
      rather than to the modifier "cross" on the call "roll".  This whole thing depends,
      of course, on our belief that the language of square dance calling never contains
      an ambiguous call/concept utterance.  For example, we do not expect anyone to
      invent a call "and turn" that can be used with the "cross" modifier. */
   new_depth = current_result->yield_depth+1;
   local_result.indent = FALSE;
   local_result.real_next_subcall = (match_result *) 0;
   local_result.real_secondary_subcall = (match_result *) 0;

   pat2_block p2b;
   p2b.folks_to_restore = (match_result *) 0;
   p2b.demand_a_call = FALSE;
   p2b.cdr = pat2;

   /* We know that user is never nil when this procedure is called.
      We also know the firstchar is just a single character, and that
      that character is blank or left bracket. */

   /* Given the above, we know that, in the next 2 loops, any iteration cycle will do nothing if:

      user[0] != NUL and user[0] != firstchar[0]

                  OR

      (call_or_concept_name has 1st 3 chars with no comma, quote, atsign, or NUL)
         AND
      (user[1,2,3] are not NUL or blank and don't match call_or_concept_name)

      */

   const match_result *save_stuff1 = (const match_result *) 0;
   const match_result *save_stuff2 = (const match_result *) 0;

   int matches_as_seen_by_me = GLOB_match_count;

   if (user[0] && user[0] != firstchar[0])
      goto getout;

   if (current_result->match.kind == ui_call_select) {
      save_stuff1 = current_result->real_next_subcall;
      save_stuff2 = current_result->real_secondary_subcall;
   }

   // See if we can use the hash mechanism.

   if (user[0] || !GLOB_showing) {
      // We now know that it will do nothing if:
      // (call_or_concept_name has 1st 3 chars with no comma, quote, atsign, or NUL)
      //                         AND
      // (user[1,2,3] are not NUL or blank and don't match call_or_concept_name)

      if (user[1] == '[' || get_hash(&user[1], &bucket) == 1) {
         if (user[1] == '[') bucket = BRACKET_HASH;

         /* Therefore, we should only search for those calls that either
            (1) have comma/quote/atsign/NULL in 1st 3 characters, OR
            (2) match user, that is, match the hash number we just computed.
         */

         using_hash = TRUE;
      }
   }

   // Now figure out how to scan the concepts.

   short int *item;
   int menu_length;

   if (using_hash) {
      if (allowing_all_concepts) {
         item = conc_hash_lists[bucket];
         menu_length = conc_hash_list_sizes[bucket];
      }
      else {
         item = conclvl_hash_lists[bucket];
         menu_length = conclvl_hash_list_sizes[bucket];
      }
   }
   else {
      if (allowing_all_concepts) {
         item = concept_list;
         menu_length = concept_list_length;
      }
      else {
         item = level_concept_list;
         menu_length = level_concept_list_length;
      }
   }

   local_result.match.kind = ui_concept_select;

   for (i = 0; i < menu_length; i++) {
      // Don't waste time after user stops us.
      if (GLOB_showing && showing_has_stopped) break;

      concept_descriptor *this_concept = &concept_descriptor_table[item[i]];
      local_result.match.concept_ptr = this_concept;
      p2b.special_concept = this_concept;
      p2b.car = this_concept->name;
      current_result = &local_result;
      current_result->yield_depth = new_depth;
      local_result.match.call_conc_options = null_options;
      match_suffix_2(user, firstchar, &p2b, patxi);
   }

   // And the calls.

   if (using_hash)
      menu_length = call_hash_list_sizes[bucket];
   else
      menu_length = number_of_calls[call_list_any];

   p2b.special_concept = (concept_descriptor *) 0;
   local_result.match.kind = ui_call_select;

   {
      bool got_aborted_subcall = false;
      modifier_block *got_matched_subcall = (modifier_block *) 0;
      int my_patxi = 0;

      for (i = 0; i < menu_length; i++) {
         // Don't waste time after user stops us.
         if (GLOB_showing && showing_has_stopped) break;

         call_with_name *this_call =
            main_call_lists[call_list_any][using_hash ? call_hash_lists[bucket][i] : i];
         current_result = &local_result;
         current_result->match.call_ptr = this_call;
         matches_as_seen_by_me = GLOB_match_count;

         p2b.car = this_call->name;
         current_result->yield_depth =
            ((this_call->the_defn.callflags1 & CFLAG1_YIELD_IF_AMBIGUOUS) ?
             new_depth+1 : new_depth);
         local_result.match.call_conc_options = null_options;

         if (spiffy_parser && this_call->name[0] == '@' && this_call->name[1] == '0') {
            if (got_aborted_subcall) {
               // We have seen another "@0" call after having seen one whose subcall
               // was incomplete.  There can't possibly be any benefit from parsing
               // further such calls.
               GLOB_match_count++;
               continue;
            }
            else if (got_matched_subcall) {
               p2b.car += 2;    // Skip over the "@0".
               match_suffix_2((Cstring) 0, "", &p2b, patxi+my_patxi);
            }
            else {
               match_suffix_2(user, firstchar, &p2b, patxi);
            }
         }
         else {
            match_suffix_2(user, firstchar, &p2b, patxi);
         }

         // See if any new matches have come up that can allow us to curtail the scan.
         if (spiffy_parser && GLOB_match_count > matches_as_seen_by_me) {
            if (GLOB_only_extension &&
                this_call->name[0] == '@' &&
                this_call->name[1] == '0' &&
                GLOB_user_input[0] == '[') {   // ***** This is probably wrong!!!!
       
               int full_bracket_depth = GLOB_user_bracket_depth + GLOB_extended_bracket_depth;

               if (local_result.recursion_depth < full_bracket_depth) {
                  // The subcall was aborted.  Processing future "@0" calls
                  // can't possibly get nontrivial results, other than to bump
                  // the match count.
                  if (!GLOB_verify && !GLOB_showing &&
                      GLOB_match.match.packed_next_conc_or_subcall &&
                      !GLOB_match.match.packed_secondary_subcall)
                     got_aborted_subcall = true;
               }
               else if (!got_matched_subcall) {
                  // The subcall was completely parsed.
                  // We should take the subcall (if there is one), and simply
                  // plug it in, without parsing it again, for all future "@0" calls.
                  if (!GLOB_verify && !GLOB_showing &&
                      GLOB_match.match.packed_next_conc_or_subcall &&
                      !GLOB_match.match.packed_secondary_subcall) {

                     // Let's try this.  We're only going to look at stuff for which
                     // the user input stopped in the middle of the subcall, for now.
                     if (GLOB_user_bracket_depth-local_result.recursion_depth > 0) {
                        // Find out where, in the full extension, the bracket count
                        // went down to zero.
                        int jj = GLOB_user_bracket_depth-local_result.recursion_depth;

                        for (my_patxi = 0 ; ; my_patxi++) {
                           if (!GLOB_full_extension[my_patxi] || jj == 0) break;
                           else if (GLOB_full_extension[my_patxi] == '[') jj++;
                           else if (GLOB_full_extension[my_patxi] == ']') jj--;
                        }
                        // Now my_patxi tells where the close bracket was.

                        got_matched_subcall = GLOB_match.match.packed_next_conc_or_subcall;
                     }
                  }
               }
            }
         }
      }
   }

 getout:

   current_result = saved_cur_res_ptr;
   current_result->real_next_subcall = save_stuff1;
   current_result->real_secondary_subcall = save_stuff2;
   current_result->indent = FALSE;

   // Clear this stuff -- it points to our local_result.
   *fixme = (match_result *) 0;
}



/*
 * Match_wildcard tests for and handles pattern suffixes that begin with
 * a wildcard such as "<anyone>".  A wildcard is handled only if there is
 * room in the result struct to store the associated value.
 */

static void match_wildcard(
   Cstring user,
   Cstring pat,
   pat2_block *pat2,
   int patxi,
   concept_descriptor *special)
{
   Cstring prefix;
   Cstring *number_table;
   int i;
   uint32 iu;
   pat2_block p2b;
   char crossname[80];
   char *crossptr;
   int save_howmanynumbers;
   uint32 save_number_fields;
   int concidx;
   Cstring pattern;
   char key = *pat++;
   p2b.car = pat;
   p2b.special_concept = special;
   p2b.folks_to_restore = (match_result *) 0;
   p2b.demand_a_call = FALSE;
   p2b.cdr = pat2;

   /* if we are just listing the matching commands, there
      is no point in expanding wildcards that are past the
      part that matches the user input.  That is why we test
      "(user == 0)". */

   if (user) {
      switch (key) {
      case '6': case 'k':
         if (current_result->match.call_conc_options.who == selector_uninitialized) {
            selector_kind save_who = current_result->match.call_conc_options.who;

            for (i=1; i<selector_enum_extent; i++) {
               current_result->match.call_conc_options.who = (selector_kind) i;
               match_suffix_2(user,
                              (key == '6') ? selector_list[i].name : selector_list[i].sing_name,
                              &p2b, patxi);
            }

            current_result->match.call_conc_options.who = save_who;
            return;
         }
         break;
      case '0': case 'm':
         if (*user == '[') {
            pat2_block p3b;

            p3b.car = "]";
            p3b.special_concept = (concept_descriptor *) 0;
            p3b.folks_to_restore = current_result;
            p3b.demand_a_call = TRUE;
            p3b.cdr = &p2b;

            scan_concepts_and_calls(user, "[", &p3b,
                                    ((key == 'm') ?
                                     &current_result->real_secondary_subcall :
                                     &current_result->real_next_subcall),
                                    patxi);
            return;
         }
         break;
      case 'h':
         if (current_result->match.call_conc_options.where == direction_uninitialized) {
            direction_kind save_where = current_result->match.call_conc_options.where;
      
            for (i=1; i<=last_direction_kind; ++i) {
               current_result->match.call_conc_options.where = (direction_kind) i;
               match_suffix_2(user, direction_names[i], &p2b, patxi);
            }
      
            current_result->match.call_conc_options.where = save_where;
            return;
         }
         break;
      case 'v': case 'w': case 'x': case 'y':

         // We don't allow this if we are already doing a tagger.  It won't happen
         // in any case, because we have taken out "revert <atc>" and "reflected <atc>"
         // as taggers.

         if (current_result->match.call_conc_options.tagger == 0) {
            int tagclass = 0;

            if (key == 'w') tagclass = 1;
            else if (key == 'x') tagclass = 2;
            else if (key == 'y') tagclass = 3;

            current_result->match.call_conc_options.tagger = tagclass << 5;
         
            for (iu=0; iu<number_of_taggers[tagclass]; iu++) {
               current_result->match.call_conc_options.tagger++;
               match_suffix_2(user, tagger_calls[tagclass][iu]->name, &p2b, patxi);
            }
      
            current_result->match.call_conc_options.tagger = 0;
            return;
         }
         break;
      case 'N':
         if (current_result->match.call_conc_options.circcer == 0) {
            char circname[80];
            uint32 save_circcer = current_result->match.call_conc_options.circcer;
      
            for (iu=0; iu<number_of_circcers; ++iu) {
               char *fromptr = circcer_calls[iu]->name;
               char *toptr = circname;
               char c;
               do {
                  c = *fromptr++;
                  if (c == '@') {
                     if (*fromptr++ == 'O') {
                        while (*fromptr++ != '@') ;
                        fromptr++;
                     }
                  }
                  else
                     *toptr++ = c;
               } while (c);
         
               current_result->match.call_conc_options.circcer++;
               match_suffix_2(user, circname, &p2b, patxi);
            }
      
            current_result->match.call_conc_options.circcer = save_circcer;
            return;
         }
         break;
      case '9':
         if (*user < '0' || *user > '9') return;
         number_table = cardinals;
         goto do_number_stuff;
      case 'u':
         if (*user < '0' || *user > '9') return;
         number_table = ordinals;
         goto do_number_stuff;
      case 'a': case 'b': case 'B': case 'D':
         if ((*user >= '0' && *user <= '8') ||
             *user == 'q' || *user == 'h' ||
             *user == 't' || *user == 'f') {
            save_howmanynumbers = current_result->match.call_conc_options.howmanynumbers;
            save_number_fields = current_result->match.call_conc_options.number_fields;
      
            current_result->match.call_conc_options.howmanynumbers++;

            for (i=0 ; (prefix = n_4_patterns[i]) ; i++) {
               if (key != 'D' || (i&1) != 0)
                  match_suffix_2(user, prefix, &p2b, patxi);
               current_result->match.call_conc_options.number_fields +=
                  1 << (save_howmanynumbers*4);
            }
         
            /* special case: allow "quarter" for 1/4 */
            current_result->match.call_conc_options.number_fields =
               save_number_fields + (1 << (save_howmanynumbers*4));
            match_suffix_2(user, "quarter", &p2b, patxi);
         
            /* special case: allow "half" or "1/2" for 2/4 */
            if (key != 'D') {
               current_result->match.call_conc_options.number_fields =
                  save_number_fields + (2 << (save_howmanynumbers*4));
               match_suffix_2(user, "half", &p2b, patxi);
               match_suffix_2(user, "1/2", &p2b, patxi);
            }
         
            /* special case: allow "three quarter" for 3/4 */
            current_result->match.call_conc_options.number_fields =
               save_number_fields + (3 << (save_howmanynumbers*4));
            match_suffix_2(user, "three quarter", &p2b, patxi);
         
            /* special case: allow "full" for 4/4 */
            if (key != 'D') {
               current_result->match.call_conc_options.number_fields =
                  save_number_fields + (4 << (save_howmanynumbers*4));
               match_suffix_2(user, "full", &p2b, patxi);
            }
      
            current_result->match.call_conc_options.howmanynumbers = save_howmanynumbers;
            current_result->match.call_conc_options.number_fields = save_number_fields;
            return;
         }
         break;
      }
   }

   // The following escape codes are the ones that we print out
   // even if the user input has run out.

   switch (key) {
   case 'S':
      {
         long_boolean saved_indent = current_result->indent;
         save_number_fields = current_result->match.call_conc_options.star_turn_option;
         current_result->indent = TRUE;
         
         current_result->match.call_conc_options.star_turn_option = -1;
         match_suffix_2(user, ", don't turn the star", &p2b, patxi);
   
         current_result->match.call_conc_options.star_turn_option = 1;
         match_suffix_2(user, ", turn the star 1/4", &p2b, patxi);
         current_result->match.call_conc_options.star_turn_option = 2;
         match_suffix_2(user, ", turn the star 1/2", &p2b, patxi);
         current_result->match.call_conc_options.star_turn_option = 3;
         match_suffix_2(user, ", turn the star 3/4", &p2b, patxi);
         
         current_result->match.call_conc_options.star_turn_option = save_number_fields;
         current_result->indent = saved_indent;
      }
      
      return;
   case 'j':
      crossptr = crossname;
      while ((*crossptr++ = *pat++) != '@');
      pat++;
      crossptr--;
      *crossptr = 0;
      pattern = crossname;
      concidx = cross_concept_index;
      goto do_cross_stuff;
   case 'C':
      pattern = " cross";
      concidx = cross_concept_index;
      goto do_cross_stuff;
   case 'J':
      crossptr = crossname;
      while ((*crossptr++ = *pat++) != '@');
      pat++;
      crossptr--;
      *crossptr = 0;
      pattern = crossname;
      concidx = magic_concept_index;
      goto do_cross_stuff;
   case 'Q':
      crossptr = crossname;
      while ((*crossptr++ = *pat++) != '@');
      pat++;
      crossptr--;
      *crossptr = 0;
      pattern = crossname;
      concidx = grand_concept_index;
      goto do_cross_stuff;
   case 'M':
      concidx = magic_concept_index;
      pattern = " magic";
      goto do_cross_stuff;
   case 'G':
      concidx = grand_concept_index;
      pattern = " grand";
      goto do_cross_stuff;
   case 'E':
      crossptr = crossname;
      while ((*crossptr++ = *pat++) != '@');
      pat++;
      crossptr--;
      *crossptr = 0;
      pattern = crossname;
      concidx = intlk_concept_index;
      goto do_cross_stuff;
   case 'I':
      {
         char *p = GLOB_full_extension;
         int idx = patxi;
         long_boolean fixing_an_a = TRUE;

         for (i=0 ; i<2 ; i++) {
            idx--;
            if (idx < 0) { idx = GLOB_user_input_size-1 ; p = GLOB_user_input; }
            if (p[idx] != "a "[i]) { fixing_an_a = FALSE; break; }
         }

         if (fixing_an_a || (user && user[-1] == 'a' && user[-2] == ' '))
            pattern = "n interlocked";
         else
            pattern = " interlocked";

         concidx = intlk_concept_index;
      }
      goto do_cross_stuff;
   case 'e':
      while (*pat++ != '@');
      pat++;
      pattern = "left";
      concidx = left_concept_index;
      goto do_cross_stuff;
   }

   return;

   do_number_stuff:

   save_howmanynumbers = current_result->match.call_conc_options.howmanynumbers;
   save_number_fields = current_result->match.call_conc_options.number_fields;

   current_result->match.call_conc_options.howmanynumbers++;

   for (i=0 ; (prefix = number_table[i]) ; i++) {
      match_suffix_2(user, prefix, &p2b, patxi);
      current_result->match.call_conc_options.number_fields += 1 << (save_howmanynumbers*4);
   }

   current_result->match.call_conc_options.howmanynumbers = save_howmanynumbers;
   current_result->match.call_conc_options.number_fields = save_number_fields;

   return;

   do_cross_stuff:

   {
      match_result *saved_cross_ptr = current_result;
      match_result saved_cross_result = *current_result;

      current_result->match.kind = ui_concept_select;
      current_result->match.call_conc_options = null_options;
      current_result->match.concept_ptr = &concept_descriptor_table[concidx];
      current_result->real_next_subcall = &saved_cross_result;
      current_result->indent = TRUE;
      p2b.car = pat;

      current_result = &saved_cross_result;
      match_suffix_2(user, pattern, &p2b, patxi);
      current_result = saved_cross_ptr;
      *current_result = saved_cross_result;
   }
}


/*
 * Match_pattern tests the user input against a pattern (pattern)
 * that may contain wildcards (such as "<anyone>").  Pattern matching is
 * case-sensitive.  As currently used, the user input and the pattern
 * are always given in all lower case.
 * If the input is equivalent to a prefix of the pattern,
 * a match is recorded in the search state.
 */
 
static void match_pattern(Cstring pattern)
{
   pat2_block p2b;

   p2b.car = pattern;
   p2b.special_concept = (concept_descriptor *) 0;
   p2b.folks_to_restore = (match_result *) 0;
   p2b.demand_a_call = FALSE;
   p2b.cdr = (pat2_block *) 0;

   match_suffix_2(GLOB_user_input, "", &p2b, 0);
}

static void search_menu(uims_reply kind)
{
   unsigned int i, menu_length;
   Cstring *menu;
   char uch = GLOB_user_input[0];
   bool input_is_null = uch == '\0' && !GLOB_showing;

   current_result = &active_result;

   current_result->recursion_depth = 0;
   current_result->valid = TRUE;
   current_result->exact = FALSE;
   current_result->match.kind = kind;
   current_result->match.call_conc_options = null_options;
   current_result->indent = FALSE;
   current_result->real_next_subcall = (match_result *) 0;
   current_result->real_secondary_subcall = (match_result *) 0;
   current_result->yield_depth = 0;

   if (kind == ui_call_select) {
      menu_length = number_of_calls[static_call_menu];

      if (input_is_null)
         GLOB_match_count += menu_length;
      else {
         int matches_as_seen_by_me;
         bool got_aborted_subcall = false;
         modifier_block *got_matched_subcall = (modifier_block *) 0;
         int my_patxi = 0;

         for (i = 0; i < menu_length; i++) {
            pat2_block p2b;

            // Don't waste time after user stops us.
            if (GLOB_showing && showing_has_stopped) break;

            call_with_name *this_call = main_call_lists[static_call_menu][i];

            // Do a quick check for mismatch on first character.
            // Q: Why do we do it just at the top level?  Shouldn't
            //    we do it at all levels?
            // A: At deeper levels, the hashing mechanism has cut the
            //    list way down, so we don't need it.  But here we don't
            //    have hashing.
            // Q: Why not?
            // A: At the top level, we have many different menus to deal
            //    with, one for each possible starting setup.  Making a hash
            //    table for each of them is unwieldy.  At deeper levels,
            //    there is just the "call_list_any" menu to deal with, and
            //    that one is hashed.
            char pch = this_call->name[0];

            if (!GLOB_showing &&
                uch != pch &&
                (pch > 'Z' || pch < 'A' || uch != pch+'a'-'A') &&
                ((pch != '@') ||
                 (this_call->name[1] == '0' && uch != '[' && uch != '<')))
               continue;

            parse_state.call_list_to_use = (call_list_kind) static_call_menu;
            active_result.match.call_ptr = this_call;
            active_result.yield_depth =
               (this_call->the_defn.callflags1 & CFLAG1_YIELD_IF_AMBIGUOUS) ? 1 : 0;
            matches_as_seen_by_me = GLOB_match_count;

            p2b.car = this_call->name;
            p2b.special_concept = (concept_descriptor *) 0;
            p2b.folks_to_restore = (match_result *) 0;
            p2b.demand_a_call = FALSE;
            p2b.cdr = (pat2_block *) 0;

            if (spiffy_parser && this_call->name[0] == '@' && this_call->name[1] == '0') {
               if (got_aborted_subcall) {
                  // We have seen another "@0" call after having seen one whose subcall
                  // was incomplete.  There can't possibly be any benefit from parsing
                  // further such calls.
                  GLOB_match_count++;
                  continue;
               }
               else if (got_matched_subcall) {
                  p2b.car += 2;    // Skip over the "@0".
                  match_suffix_2((Cstring) 0, "", &p2b, my_patxi);
               }
               else {
                  match_suffix_2(GLOB_user_input, "", &p2b, 0);
               }
            }
            else {
               match_suffix_2(GLOB_user_input, "", &p2b, 0);
            }

            // See if any new matches have come up that can allow us to curtail the scan.
            if (spiffy_parser && GLOB_match_count > matches_as_seen_by_me) {
               if (GLOB_only_extension &&
                   this_call->name[0] == '@' &&
                   this_call->name[1] == '0' &&
                   uch == '[') {
                  int full_bracket_depth = GLOB_user_bracket_depth + GLOB_extended_bracket_depth;
                  if (0 < full_bracket_depth) {
                     // The subcall was aborted.  Processing future "@0" calls
                     // can't possibly get nontrivial results, other than to bump
                     // the match count.
                     if (!GLOB_verify && !GLOB_showing &&
                         GLOB_match.match.packed_next_conc_or_subcall &&
                         !GLOB_match.match.packed_secondary_subcall)
                        got_aborted_subcall = true;
                  }
                  else if (!got_matched_subcall) {
                     // The subcall was completely parsed.
                     // We should take the subcall (if there is one), and simply
                     // plug it in, without parsing it again, for all future "@0" calls.
                     if (!GLOB_verify && !GLOB_showing &&
                         GLOB_match.match.packed_next_conc_or_subcall &&
                         !GLOB_match.match.packed_secondary_subcall) {

                        // Let's try this.  We're only going to look at stuff for which
                        // the user input stopped in the middle of the subcall, for now.
                        if (GLOB_user_bracket_depth > 0) {
                           // Find out where, in the full extension, the bracket count
                           // went down to zero.
                           int jj = GLOB_user_bracket_depth;

                           for (my_patxi = 0 ; ; my_patxi++) {
                              if (!GLOB_full_extension[my_patxi] || jj == 0) break;
                              else if (GLOB_full_extension[my_patxi] == '[') jj++;
                              else if (GLOB_full_extension[my_patxi] == ']') jj--;
                           }
                           // Now my_patxi tells where the close bracket was.

                           got_matched_subcall = GLOB_match.match.packed_next_conc_or_subcall;
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else if (kind == ui_concept_select) {
      short int *item;

      if (allowing_all_concepts) {
         item = concept_list;
         menu_length = concept_list_length;
      }
      else {
         item = level_concept_list;
         menu_length = level_concept_list_length;
      }
   
      if (input_is_null)
         GLOB_match_count += menu_length;
      else {
         for (i = 0; i < menu_length; i++) {
            pat2_block p2b;

            // Don't waste time after user stops us.
            if (GLOB_showing && showing_has_stopped) break;

            concept_descriptor *this_concept = &concept_descriptor_table[item[i]];

            // Another quick check -- there are hundreds of concepts.
            char pch = this_concept->name[0];

            if (uch != pch &&
                (pch > 'Z' || pch < 'A' || uch != pch+'a'-'A') &&
                ((pch != '@') ||
                 (this_concept->name[1] == '0' && uch != '[' && uch != '<')))
               continue;

            parse_state.call_list_to_use = (call_list_kind) static_call_menu;
            active_result.match.concept_ptr = this_concept;
            active_result.yield_depth =
               (this_concept->concparseflags & CONCPARSE_YIELD_IF_AMB) ? 1 : 0;

            p2b.car = this_concept->name;
            p2b.special_concept = this_concept;
            p2b.folks_to_restore = (match_result *) 0;
            p2b.demand_a_call = FALSE;
            p2b.cdr = (pat2_block *) 0;

            match_suffix_2(GLOB_user_input, "", &p2b, 0);
         }
      }
   }
   else if (static_call_menu >= match_taggers &&
            static_call_menu < match_taggers+NUM_TAGGER_CLASSES) {
      int tagclass = static_call_menu - match_taggers;
      active_result.match.call_conc_options.tagger = tagclass << 5;

      if (input_is_null)
         GLOB_match_count += number_of_taggers[tagclass];
      else {
         for (i = 0; i < number_of_taggers[tagclass]; i++) {
            active_result.match.call_conc_options.tagger++;
            match_pattern(tagger_calls[tagclass][i]->name);
         }
      }
   }
   else if (static_call_menu == match_circcer) {
      active_result.match.call_conc_options.circcer = 0;

      if (input_is_null)
         GLOB_match_count += number_of_circcers;
      else {
         for (i = 0; i < number_of_circcers; i++) {
            active_result.match.call_conc_options.circcer++;
            match_pattern(circcer_calls[i]->name);
         }
      }
   }
   else {
      if (kind == ui_command_select) {
         menu = command_commands;
         menu_length = num_command_commands;
      }
      else if (static_call_menu == match_directions) {
         menu = &direction_names[1];
         menu_length = last_direction_kind;
      }
      else if (static_call_menu == match_number) {
         menu = cardinals;
         menu_length = NUM_CARDINALS;
      }
      else if (static_call_menu == match_selectors) {
         menu = selector_menu_list;
         // Menu is shorter than it appears, because we are skipping first item.
         menu_length = selector_enum_extent-1;
      }
      else if (static_call_menu == match_startup_commands) {
         kind = ui_start_select;
         menu = startup_commands;
         menu_length = num_startup_commands;
      }
      else if (static_call_menu == match_resolve_commands) {
         kind = ui_resolve_select;
         menu = resolve_command_strings;
         menu_length = number_of_resolve_commands;
      }

      active_result.match.kind = kind;

      if (input_is_null)
         GLOB_match_count += menu_length;
      else {
         for (i = 0; i < menu_length; i++) {
            active_result.match.index = i;
            active_result.yield_depth = 0;
            match_pattern(menu[i]);
         }
      }
   }
}

    


/*
 * MATCH_USER_INPUT is the basic command matching function.
 *
 * The WHICH_COMMANDS parameter determines which list of commands
 * is matched against. Possibilities include:
 *
 * match_startup_commands - startup mode commands
 * match_resolve_commands - resolve mode commands
 * match_selectors        - selectors (e.g. "boys")
 * match_directions       - directions (e.g. "left")
 * <call menu index>      - normal commands, concepts, and calls
 *                          from specified call menu
 *
 * The USER_INPUT parameter is the user-specified string that is being
 * matched.
 *
 * MATCH_USER_INPUT returns the number of commands that are
 * compatible with the specified user input.  A command is compatible
 * if it is matches the user input as is or the user input could
 * be extended to match the command.
 *
 * If there is at least
 * one compatible command, then the maximal extension to the user
 * input that is compatible with all the compatible commands is returned
 * in EXTENSION; otherwise EXTENSION is cleared.
 * If there is at least one compatible command, then
 * a description of one such command is returned in MR.  EXTENSION
 * and MR can be NULL, if the corresponding information is not needed.
 *
 * MR->VALID is true if and only if there is at least one matching command.
 * MR->EXACT is true if and only if one or more commands exactly matched the
 * input (no extension required).  The remainder of MR is the
 * data to be returned to sd from one or more invocations of
 * user interface functions.
 *
 * The SF parameter is used to display a list of matching
 * commands. If SF is non-zero, then it must be a function pointer
 * that accepts two char * parameters and a match_result * parameter.
 * That function will be invoked
 * for each matching command.  The first parameter is the user input,
 * the second parameter is the extension corresponding to the command.
 * When showing matching commands,
 * wildcards are not expanded unless they are needed to match the
 * user input.  The extension may contain wildcards.  The third
 * parameter is the (volatile) match_result that describes the
 * matching command.  If SHOW_VERIFY is nonzero, then an attempt
 * will be made to verify that a call is possible in the current
 * setup before showing it.
 *
 * When searching for calls and concepts, it also searches for the special
 * commands provided by the "command_list" and "num_commands" arguments.
 * When such a command is found, we return "ui_command_select" as the kind,
 * and the index into that array as the index.
 *
 */

SDLIB_API int match_user_input(
   int which_commands,
   long_boolean show,
   long_boolean show_verify,
   long_boolean only_want_extension)
{
   // Reclaim all old modifier blocks.

   while (modifier_active_list) {
      modifier_block *item = modifier_active_list;
      modifier_active_list = item->gc_ptr;
      item->gc_ptr = modifier_inactive_list;
      modifier_inactive_list = item;
   }

   GLOB_only_extension = only_want_extension;
   GLOB_match_count = 0;
   GLOB_exact_count = 0;
   GLOB_lowest_yield_depth = 999;
   GLOB_exact_match = FALSE;
   GLOB_showing = show;
   GLOB_verify = show_verify;
   GLOB_echo_stuff[0] = 0;   // Needed if no matches or user input is empty.
   GLOB_yielding_matches = 0;
   GLOB_match.valid = FALSE;
   GLOB_match.exact = FALSE;
   GLOB_space_ok = FALSE;
   static_call_menu = which_commands;

   // Count the bracket depth of the user's part of the line.
   GLOB_user_bracket_depth = 0;
   for (const char *p = GLOB_user_input ; *p ; p++) {
      if (*p == '[') GLOB_user_bracket_depth++;
      else if (*p == ']') GLOB_user_bracket_depth--;
   }

   if (static_call_menu >= 0) {
      search_menu(ui_call_select);
      search_menu(ui_concept_select);
      search_menu(ui_command_select);
   }  
   else
      search_menu(ui_special_concept);

#ifdef TIMING
    timer = clock() - timer;
    sprintf(time_buf, "%.2f", (double)timer / CLOCKS_PER_SEC);
    uims_debug_print(time_buf);
#endif

   return GLOB_match_count;
}
