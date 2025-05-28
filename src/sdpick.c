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
   reset_internal_iterators
   do_selector_iteration
   do_direction_iteration
   do_number_iteration
   do_circcer_iteration
   do_tagger_iteration
   do_pick
   pick_concept
   get_resolve_goodness_info
   pick_allow_multiple_items
   start_pick
   end_pick
   forbid_call_with_mandatory_subcall
   allow_random_subcall_pick

and the following external variables:
   search_goal
*/

#include <string.h>
#include "sd.h"

typedef enum {
   pick_starting_first_scan,
   pick_plain_scan_nice_only,  /* Just calls, very picky about quality. */
   pick_concept_nice_only,     /* Concept/call, very picky about quality. */
   pick_plain_accept_all,      /* Just calls, but accept anything. */
   pick_concept_accept_all,    /* Concept/call, accept anything. */
   pick_in_random_search,
   pick_not_in_any_pick_at_all
} pick_type;


typedef struct {
   long_boolean exhaustive_search;
   long_boolean accept_nice_only;
   long_boolean with_concept;
} pick_type_descriptor;

pick_type_descriptor pick_type_table[] = {
   { FALSE, FALSE, FALSE },
   { TRUE,  TRUE,  FALSE },
   { TRUE,  TRUE,  TRUE },
   { TRUE,  FALSE, FALSE },
   { TRUE,  FALSE, TRUE },
   { FALSE, FALSE, FALSE },
   { FALSE, FALSE, FALSE },};

command_kind search_goal;

/* The "pick_concept_***" passes are actually several scans over several
   concepts.  This counter counts them. */
static int concept_scan_index;
static int concept_scan_limit;
static short int *concept_scan_table;

static uint32 selector_iterator = 0;
static uint32 direction_iterator = 0;
static uint32 number_iterator = 0;
static uint32 tagger_iterator = 0;
static uint32 circcer_iterator = 0;
static int resolve_scan_start_point;
static int resolve_scan_current_point;
/* This is only meaningful if interactivity = interactivity_picking. */
static pick_type current_pick_type = pick_not_in_any_pick_at_all;


/* These two direct the generation of random concepts when we are searching.
   We make an attempt to generate somewhat plausible concepts, depending on the
   setup we are in.  If we just generated evenly weighted concepts from the entire
   concept list, we would hardly ever get a legal one. */
static int concept_sublist_sizes[NUM_CALL_LIST_KINDS];
static short int *concept_sublists[NUM_CALL_LIST_KINDS];
/* These two are similar, but contain only "really nice" concepts that
   we are willing to use in exhaustive searches.  Concepts in these
   lists are very "expensive", in that each one causes an exhaustive search
   through all calls (with nearly-exhaustive enumeration of numbers/selectors, etc).
   Also, these concepts will appear in suggested resolves very early on.  So
   we don't want anything the least bit ugly in these lists. */
static int good_concept_sublist_sizes[NUM_CALL_LIST_KINDS];
static short int *good_concept_sublists[NUM_CALL_LIST_KINDS];


/* This fills in concept_sublist_sizes and concept_sublists. */

/*   A "1" means the concept is allowed in this setup

                     lout, lin, tby
                           |
                           |+- 8ch, Lcol, Rcol
          R2fl, Lwv, Rwv -+||
                          |||+- cdpt, dpt, L1x8
        qtag, gcol, L2fl-+||||
                         |||||+- R1x8, any, junk
                         ||||||       */
#define MASK_CTR_2      0600016
#define MASK_QUAD_D     0200016
#define MASK_CPLS       0547462
#define MASK_GOODCPLS   0147460
#define MASK_TAND       0770362
#define MASK_GOODTAND   0170360
#define MASK_GOODCONC   0177774
#define MASK_GOODRMVD   0177760
#define MASK_SIAM       0400002
#define MASK_2X4        0177762


extern void initialize_concept_sublists(void)
{
   int concept_index;
   int all_legal_concepts;
   call_list_kind test_call_list_kind;
   concept_kind end_marker = concept_diagnose;

   /* Decide whether we allow the "diagnose" concept, by deciding
      when we will stop the concept list scan. */
   if (diagnostic_mode) end_marker = marker_end_of_list;

   /* First, just count up all the available concepts. */

   for (concept_index=0,all_legal_concepts=0 ; ; concept_index++) {
      concept_descriptor *p = &concept_descriptor_table[concept_index];
      if (p->kind == end_marker) break;
      if ((p->concparseflags & CONCPARSE_MENU_DUP) || p->level > calling_level)
         continue;
      all_legal_concepts++;
   }

   /* Our friends in the UI will need this. */
   general_concept_size = concept_index - general_concept_offset;

   concept_sublists[call_list_any] =
      (short int *) get_mem(all_legal_concepts*sizeof(short int));
   good_concept_sublists[call_list_any] =
      (short int *) get_mem(all_legal_concepts*sizeof(short int));

   /* Make the concept sublists, one per setup.  We do them in downward order,
      with "any setup" last.  This is because we put our results into the
      "any setup" slot while we are working, and then copy them to the
      correct slot for each setup other than "any". */

   for (test_call_list_kind = call_list_qtag;
        test_call_list_kind >= call_list_any;
        test_call_list_kind--) {
      int concepts_in_this_setup, good_concepts_in_this_setup;

      for (concept_index=0,concepts_in_this_setup=0,good_concepts_in_this_setup=0;
           ;
           concept_index++) {
         uint32 setup_mask = ~0;      /* Default is to accept the concept. */
         uint32 good_setup_mask = 0;  /* Default for this is not to. */
         concept_descriptor *p = &concept_descriptor_table[concept_index];
         if (p->kind == end_marker) break;
         if ((p->concparseflags & CONCPARSE_MENU_DUP) || p->level > calling_level)
            continue;

         /* This concept is legal at this level.  See if it is appropriate for this setup.
            If we don't know, the default value of setup_mask will make it legal. */

         switch (p->kind) {
         case concept_centers_or_ends:
         case concept_centers_and_ends:
            switch (p->value.arg1) {
            case selector_center6:
            case selector_outer6:
            case selector_center2:
            case selector_verycenters:
            case selector_outer2:
            case selector_veryends:
               setup_mask = MASK_CTR_2;    /* This is a 6 and 2 type of concept. */
               break;
            default:
               break;
            }

            break;
         case concept_concentric:
            switch (p->value.arg1) {
            case schema_concentric:
            case schema_cross_concentric:
            case schema_single_concentric:
            case schema_single_cross_concentric:
               good_setup_mask = MASK_GOODCONC;
            }

            break;
         case concept_once_removed:
            if (p->value.arg1 == 0) 
               good_setup_mask = MASK_GOODRMVD;

            break;
         case concept_tandem:
         case concept_some_are_tandem:
         case concept_frac_tandem:
         case concept_some_are_frac_tandem:
            switch (p->value.arg4) {
            case tandem_key_tand:
               setup_mask = MASK_TAND;

               /* We allow <anyone> tandem, and we allow twosome.  That's all.
                  We specifically exclude all the "tandem in a 1/4 tag" stuff. */
               if (p->value.arg2 != CONCPROP__NEEDK_TWINQTAG &&
                   ((p->value.arg3 & 0xFF) == 0 || (p->value.arg3 & 0xFF) == 0x10))
                  good_setup_mask = MASK_GOODTAND;

               break;
            case tandem_key_cpls:
               setup_mask = MASK_CPLS;

               /* We allow <anyone> tandem, and we allow twosome.  That's all.
                  We specifically exclude all the "tandem in a 1/4 tag" stuff. */
               if (p->value.arg2 != CONCPROP__NEEDK_TWINQTAG &&
                   ((p->value.arg3 & 0xFF) == 0 || (p->value.arg3 & 0xFF) == 0x10))
                  good_setup_mask = MASK_GOODCPLS;

               break;
            case tandem_key_siam:
               setup_mask = MASK_SIAM; break;
            default:
               setup_mask = 0; break;    /* Don't waste time on the others. */
            }
            break;
         case concept_multiple_lines_tog_std:
         case concept_multiple_lines_tog:
            /* Test for quadruple C/L/W working. */
            if (p->value.arg4 == 4) setup_mask = MASK_2X4;
            break;
         case concept_quad_diamonds:
         case concept_quad_diamonds_together:
         case concept_do_phantom_diamonds:
            setup_mask = MASK_QUAD_D;
            break;
         case concept_do_phantom_2x4:
         case concept_divided_2x4:
         case concept_quad_lines:
            setup_mask = MASK_2X4;          /* Can actually improve on this. */
            break;
         case concept_assume_waves:
            /* We never allow any "assume_waves" concept.  In the early days,
               it was actually dangerous.  It isn't dangerous any more,
               but it's a fairly stupid thing to generate in a search. */
            setup_mask = 0;
            break;
         }

         /* Now we can determine whether this concept is appropriate for this setup. */

         if ((1 << (((int) test_call_list_kind) - ((int) call_list_empty))) & setup_mask)
            concept_sublists[call_list_any][concepts_in_this_setup++] = concept_index;

         /* And we can determine whether this concept is really nice for this setup. */

         if ((1 << (((int) test_call_list_kind) - ((int) call_list_empty))) & good_setup_mask)
            good_concept_sublists[call_list_any][good_concepts_in_this_setup++] = concept_index;
      }

      concept_sublist_sizes[test_call_list_kind] = concepts_in_this_setup;
      good_concept_sublist_sizes[test_call_list_kind] = good_concepts_in_this_setup;

      if (test_call_list_kind != call_list_any) {
         if (concepts_in_this_setup != 0) {
            concept_sublists[test_call_list_kind] =
               (short int *) get_mem(concepts_in_this_setup*sizeof(short int));
            (void) memcpy(concept_sublists[test_call_list_kind],
                          concept_sublists[call_list_any],
                          concepts_in_this_setup*sizeof(short int));
         }
         if (good_concepts_in_this_setup != 0) {
            good_concept_sublists[test_call_list_kind] =
               (short int *) get_mem(good_concepts_in_this_setup*sizeof(short int));
            (void) memcpy(good_concept_sublists[test_call_list_kind],
                          good_concept_sublists[call_list_any],
                          good_concepts_in_this_setup*sizeof(short int));
         }
      }
   }

   /* "Any" is not considered a good setup. */
   good_concept_sublist_sizes[call_list_any] = 0;
}



extern void reset_internal_iterators(void)
{
   selector_iterator = 0;
   direction_iterator = 0;
   number_iterator = 0;
   tagger_iterator = 0;
   circcer_iterator = 0;
}

extern selector_kind do_selector_iteration(long_boolean is_for_call)
{
   static selector_kind selector_iterator_table[] = {
      selector_boys,
      selector_girls,
      selector_centers,
      selector_ends,
      selector_leads,
      selector_trailers,
      selector_beaus,
      selector_belles,
      selector_uninitialized};

   int j;

   if (pick_type_table[current_pick_type].exhaustive_search &&
       is_for_call) {
      j = (int) selector_iterator_table[selector_iterator];

      selector_iterator++;

      /* See if we have exhausted all possible selectors.
         We only look for "boys", "girls", "centers", and "ends" in the first scan. */
      if (selector_iterator_table[selector_iterator] ==
          ((current_pick_type == pick_plain_scan_nice_only) ?
           selector_leads :
           ((calling_level < beau_belle_level) ?
            selector_beaus :
            selector_uninitialized)))
         selector_iterator = 0;
   }
   else if (pick_type_table[current_pick_type].exhaustive_search) {
      /* In a concept during exhaustive search.  That means we are doing
         the "<anyone> are tandem" stuff. */
      j = (int) selector_centers;
   }

   else {
      /* We don't generate unsymmetrical selectors when searching.  It generates
         too many "couple #3 u-turn-back" calls. */
      j = generate_random_number(unsymm_selector_start-1)+1;
   }

   hash_nonrandom_number(j-1);
   return (selector_kind) j;
}


extern direction_kind do_direction_iteration(void)
{
   static direction_kind direction_iterator_table[] = {
      direction_left,
      direction_right,
      direction_in,
      direction_out,
      direction_uninitialized};

   int j;

   if (pick_type_table[current_pick_type].exhaustive_search) {
      j = (int) direction_iterator_table[direction_iterator];

      if (selector_iterator == 0) {
         direction_iterator++;

         /* See if we have exhausted all possible directions.
            We only look for "left" and "right" in the first scan. */
         if (     direction_iterator_table[direction_iterator] == 
                  ((current_pick_type == pick_plain_scan_nice_only) ?
                   direction_in :
                   direction_uninitialized))
            direction_iterator = 0;
      }
   }
   else {
      j = generate_random_number(last_direction_kind)+1;
   }

   hash_nonrandom_number(j-1);
   return (direction_kind) j;
}


extern void do_number_iteration(int howmanynumbers,
                                uint32 odd_number_only,
                                long_boolean allow_iteration,
                                uint32 *number_list)
{
   int i;

   *number_list = 0;

   for (i=0 ; i<howmanynumbers ; i++) {
      uint32 this_num;

      if (allow_iteration &&
          pick_type_table[current_pick_type].exhaustive_search) {
         this_num = ((number_iterator >> (i*2)) & 3) + 1;
      }
      else if (odd_number_only)
         this_num = (generate_random_number(2)<<1)+1;
      else
         this_num = generate_random_number(4)+1;

      hash_nonrandom_number(this_num-1);

      *number_list |= (this_num << (i*4));
   }

   if (pick_type_table[current_pick_type].exhaustive_search &&
       (selector_iterator | direction_iterator) == 0) {
      number_iterator++;
      /* If the call requires it, or if doing the first scan and the call
         takes multiple numbers, we iterate over odd numbers only.  The
         reason for the latter clause is that we don't want to wast a lot of
         time enumerating 256 combinations of i-j-k-l quarter the deucey.
         We will get to them on the second scan in any case. */
      if (odd_number_only ||
          (howmanynumbers >= 2 && current_pick_type == pick_plain_scan_nice_only)) {
         while (number_iterator & 0x55555555)
            number_iterator += number_iterator & ~(number_iterator-1);
      }

      /* See if we have exhausted all possible numbers. */
      if (number_iterator >> (howmanynumbers*2)) number_iterator = 0;
   }
}


extern void do_circcer_iteration(uint32 *circcp)
{
   if (pick_type_table[current_pick_type].exhaustive_search) {
      *circcp = circcer_iterator+1;

      if ((selector_iterator | direction_iterator | number_iterator | tagger_iterator) == 0) {
         circcer_iterator++;

         /* See if we have exhausted all possible circcers. */
         if (circcer_iterator == number_of_circcers)
            circcer_iterator = 0;
      }
   }
   else
      *circcp = generate_random_number(number_of_circcers)+1;

   hash_nonrandom_number(*circcp - 1);
}


extern long_boolean do_tagger_iteration(uint32 tagclass,
                                        uint32 *tagg,
                                        uint32 numtaggers,
                                        callspec_block **tagtable)
{
   uint32 tag;

   if (pick_type_table[current_pick_type].exhaustive_search) {
      tag = tagger_iterator;

         /* But we don't allow any call that takes a another tag call (e.g. "revert"),
            or any other iteratable modifier.  It's just too complicated to iterate
            over the space of all calls when things like this happen.
            We also reject any that are marked not to be used in resolve. */

      while (tag < numtaggers &&
             ((tagtable[tag]->callflags1 &
               (CFLAG1_DONT_USE_IN_RESOLVE|
                CFLAG1_NUMBER_MASK)) ||
              (tagtable[tag]->callflagsf &
               (CFLAGH__TAG_CALL_RQ_MASK|
                CFLAGH__CIRC_CALL_RQ_BIT|
                CFLAGH__REQUIRES_SELECTOR|
                CFLAGH__REQUIRES_DIRECTION))))
         tag++;

      if (tag == numtaggers && tagger_iterator == 0)
         return TRUE;  /* There simply are no acceptable taggers. */

      if ((selector_iterator | direction_iterator | number_iterator) == 0) {
         tagger_iterator = tag+1;

         while (tagger_iterator < numtaggers &&
                ((tagtable[tagger_iterator]->callflags1 &
                  (CFLAG1_DONT_USE_IN_RESOLVE|
                   CFLAG1_NUMBER_MASK)) ||
                 (tagtable[tagger_iterator]->callflagsf &
                  (CFLAGH__TAG_CALL_RQ_MASK|
                   CFLAGH__CIRC_CALL_RQ_BIT|
                   CFLAGH__REQUIRES_SELECTOR|
                   CFLAGH__REQUIRES_DIRECTION))))
            tagger_iterator++;

         /* See if we have exhausted all possible taggers. */
         if (tagger_iterator == numtaggers)
            tagger_iterator = 0;
      }
   }
   else {
      tag = generate_random_number(numtaggers);
   }

   hash_nonrandom_number(tag);

   /* We don't generate "dont_use_in_resolve" taggers in any random search. */
   if (tagtable[tag]->callflags1 & CFLAG1_DONT_USE_IN_RESOLVE)
      fail("This shouldn't get printed.");

   *tagg = (tagclass << 5) | (tag+1);
   return FALSE;
}



/* When we get here, resolve_scan_current_point has the *last* call that we
   did.  If the iterators are nonzero, we will just repeat that call.
   Otherwise, we will advance it to the next and use that call. */

extern concept_descriptor *pick_concept(long_boolean already_have_concept_in_place)
{
   long_boolean do_concept = FALSE;

   if (interactivity != interactivity_picking)
      return (concept_descriptor *) 0;

   if (current_pick_type == pick_starting_first_scan) {

         /* Generate the random starting point for the scan, so it won't be identical
         each time we resolve from this position.  This is the only time that we use
         the random number generator during the initial scans.  Note that this random
         number will NOT be hashed.  But don't use a random number if in diagnostic mode. */

      /* Note also that this starting point for the call scan lies within
         the available calls for the setup we are in.  Some scans will be
         across the available calls for "any" setup, because we will be
         doing concepts.  But "any" setup always allows more calls, so we are safe. */

      resolve_scan_start_point =
         (diagnostic_mode) ?
         0 :
         generate_random_number(number_of_calls[parse_state.call_list_to_use]);
      resolve_scan_current_point = resolve_scan_start_point-1;
      current_pick_type = pick_plain_scan_nice_only;
      reset_internal_iterators();
   }
   else if (pick_type_table[current_pick_type].exhaustive_search) {
      if (already_have_concept_in_place)
         return (concept_descriptor *) 0;

      if ((selector_iterator | direction_iterator | number_iterator |
           tagger_iterator | circcer_iterator) == 0) {
         if (resolve_scan_current_point == resolve_scan_start_point) {

            /* Done with this scan.  Advance to the next thing to do. */

            if (pick_type_table[current_pick_type].with_concept) {
               /* Currently doing a scan with concepts.  Go to the next
                  concept.  If run out, go to next scan. */
               concept_scan_index++;
               if (concept_scan_index < concept_scan_limit)
                  goto foobar;
            }

            /* Now we are really doing the next major scan type. */

            current_pick_type++;

            concept_scan_index = 0;  /* If this is a concept scan, we will need these. */
            concept_scan_limit = good_concept_sublist_sizes[parse_state.call_list_to_use];
            concept_scan_table = good_concept_sublists[parse_state.call_list_to_use];

            /* Now, if we are in a scan that involves concepts, check whether
               there are any concepts to use.  (There might not be --
               we severely restrict the number of available concepts in these scans.) */

            if (search_goal != command_resolve ||
                good_concept_sublist_sizes[parse_state.call_list_to_use] == 0) {
               /* Can't go into concept scans, so skip over any of same. */
               while (pick_type_table[current_pick_type].with_concept)
                  current_pick_type++;
            }
         foobar: ;
         }

         resolve_scan_current_point--;   /* Might go to -1.  Do_pick will fix same. */
      }
   }
   else if (current_pick_type == pick_in_random_search) {
      switch (search_goal) {
      case command_concept_call:
         if (!already_have_concept_in_place)
            { do_concept = TRUE; break; }
         /* FALL THROUGH!!!!! */
      case command_resolve:
      case command_random_call:
      case command_standardize:
         do_concept = generate_random_number(8) <
            ((search_goal == command_standardize) ?
             STANDARDIZE_CONCEPT_PROBABILITY : CONCEPT_PROBABILITY);

         /* Since we are not going to use the random number in a one-to-one way,
            we run the risk of not having hashed_randoms uniquely represent
            what is happening.  To remedy the problem, we hash just the yes-no
            result of our decision. */

         hash_nonrandom_number((int) do_concept);
         break;
      }
   }

   if (do_concept) {
      int j = concept_sublist_sizes[parse_state.call_list_to_use];

      if (j != 0) {    /* If no concepts are available (perhaps some clown has
                          selected "pick concept call" at mainstream) we don't
                          insert a concept. */
         j = generate_random_number(j);

         uims_menu_index = concept_sublists[parse_state.call_list_to_use][j];
         return &concept_descriptor_table[uims_menu_index];
      }
   }
   else if (pick_type_table[current_pick_type].with_concept &&
            !already_have_concept_in_place) {
      uims_menu_index = concept_scan_table[concept_scan_index];
      return &concept_descriptor_table[uims_menu_index];
   }

   return (concept_descriptor *) 0;
}


extern callspec_block *do_pick(void)
{
   int i;
   uint32 rejectflag;
   callspec_block *result;

   if (pick_type_table[current_pick_type].exhaustive_search) {
      if (resolve_scan_current_point < 0)
         resolve_scan_current_point = number_of_calls[parse_state.call_list_to_use]-1;
      i = resolve_scan_current_point;
   }
   else        /* In random search. */
      i = generate_random_number(number_of_calls[parse_state.call_list_to_use]);

   /* Fix up the "hashed randoms" stuff as though we had generated this number
         through the random number generator. */

   hash_nonrandom_number(i);
   result = main_call_lists[parse_state.call_list_to_use][i];

   /* Why don't we just call the random number generator again if the call is inappropriate?
      Wouldn't that be much faster?  There are two reasons:  First, we would need to take
      special precautions against an infinite loop.  Second, and more importantly, if we
      just called the random number generator again, it would screw up the hash numbers,
      which would make the uniquefication fail, so we could see the same thing twice. */
   
   if ((search_goal == command_level_call || search_goal == command_8person_level_call) &&
       ((dance_level) result->level) < level_threshholds[calling_level])
      fail("Level reject.");

   rejectflag = pick_type_table[current_pick_type].exhaustive_search ?
      (CFLAG1_DONT_USE_IN_RESOLVE|CFLAG1_DONT_USE_IN_NICE_RESOLVE) :
      CFLAG1_DONT_USE_IN_RESOLVE;

   if (result->callflags1 & rejectflag) fail("This shouldn't get printed.");
   return result;
}


extern resolve_goodness_test get_resolve_goodness_info(void)
{
   if (interactivity == interactivity_picking) {
      if (pick_type_table[current_pick_type].accept_nice_only)
         /* "Nice" exhaustive scans:  Only accept things with "how_bad" = 0,
            that is, RLG, LA, and prom.  And only if the promenade is short
            (if at C2 or above).  And only if one call long. */
         return resolve_goodness_only_nice;
      else if (current_pick_type != pick_in_random_search)
         /* Other exhaustive scans: accept any one-call resolve. */
         return resolve_goodness_always;
   }

   /* Anything else: Accept resolves randomly.  Do this during the random search. */

   return resolve_goodness_maybe;
}


extern long_boolean pick_allow_multiple_items(void)
{
   if (pick_type_table[current_pick_type].exhaustive_search)
      return FALSE;
   else
      return TRUE;
}


extern void start_pick(void)
{
   /* Following a suggestion of Eric Brosius, we initially scan the entire database once,
      looking for one-call resolves, before we start the complex search.  This way, we
      will never show a multiple-call resolve if a single-call one exists.  Of course,
      it's not really that simple -- if a call takes a number, direction, or person,
      we will only use one canned value for it, so we could miss a single call resolve
      on this first pass if that call involves an interesting number, etc. */

   interactivity = interactivity_picking;

   if (search_goal == command_resolve ||
       search_goal == command_reconcile ||
       search_goal == command_normalize ||
       search_goal == command_standardize ||
       search_goal >= command_create_any_lines)
      current_pick_type = pick_starting_first_scan;
   else
      current_pick_type = pick_in_random_search;
}


extern void end_pick(void)
{
   current_pick_type = pick_not_in_any_pick_at_all;
}


/* When doing a pick, this predicate says that any call that takes a
   mandatory subcall is simply rejected. */
extern long_boolean forbid_call_with_mandatory_subcall(void)
{
   if (pick_type_table[current_pick_type].exhaustive_search)
      return TRUE;
   else
      return FALSE;
}

/* When we are doing the special scans in the resolver, we don't geneerate
   random subcalls -- we just leave the default call in place.  Only when in the random search
   do we generate random subcalls. */
extern long_boolean allow_random_subcall_pick(void)
{
   if (current_pick_type == pick_in_random_search)
      return TRUE;
   else
      return FALSE;
}
