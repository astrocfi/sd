/* SD -- square dance caller's helper.

    Copyright (C) 1990-2001  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 32. */

/* This defines the following functions:
   write_resolve_text
   full_resolve
   concepts_in_place
   reconcile_command_ok
   resolve_command_ok
   nice_setup_command_ok
   create_resolve_menu_title
   initialize_getout_tables
*/


#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include <stdio.h>
#include "sd.h"


/* This is the number of resolves that we try in response to each
   "find another" command, before reporting failure.  For a long time, it was 5000. */
#define ATTEMPTS_PER_COMMAND 15000


typedef struct {
   configuration stuph[MAX_RESOLVE_SIZE];
   int size;
   int insertion_point;
   int permute1[8];
   int permute2[8];
   int rotchange;
} resolve_rec;



typedef struct {
   int perm[8];
   int accept_extend;
   long_boolean allow_eighth_rotation;
} reconcile_descriptor;



static reconcile_descriptor promperm =  {{1, 0, 6, 7, 5, 4, 2, 3}, 0, FALSE};
static reconcile_descriptor rpromperm = {{0, 1, 7, 6, 4, 5, 3, 2}, 0, FALSE};
static reconcile_descriptor rlgperm =   {{1, 0, 6, 7, 5, 4, 2, 3}, 2, FALSE};
static reconcile_descriptor qtagperm =  {{1, 0, 7, 6, 5, 4, 3, 2}, 0, FALSE};
static reconcile_descriptor homeperm =  {{6, 5, 4, 3, 2, 1, 0, 7}, 0, TRUE};
static reconcile_descriptor sglperm =   {{7, 6, 5, 4, 3, 2, 1, 0}, 0, TRUE};
static reconcile_descriptor crossperm = {{5, 4, 3, 2, 1, 0, 7, 6}, 0, FALSE};
static reconcile_descriptor crossplus = {{5, 4, 3, 2, 1, 0, 7, 6}, 1, FALSE};
static reconcile_descriptor laperm =    {{1, 3, 6, 0, 5, 7, 2, 4}, 2, FALSE};


static configuration *huge_history_save = (configuration *) 0;
static int huge_history_allocation = 0;
static int huge_history_ptr;

static resolve_rec *all_resolves = (resolve_rec *) 0;
static int resolve_allocation = 0;

static int *avoid_list = (int *) 0;
static int avoid_list_size;
static int avoid_list_allocation = 0;
static uint32 perm_array[8];
static setup_kind goal_kind;
static uint32 goal_directions[8];
static reconcile_descriptor *current_reconciler;

/* BEWARE!!  This must be keyed to the enumeration "command_kind" in sd.h . */
static Cstring title_string[] = {
   "Resolve: ",
   "Normalize: ",
   "Standardize: ",
   "Reconcile: ",
   "Pick Random Call: ",
   "Pick Simple Call: ",
   "Pick Concept Call: ",
   "Pick Level Call: ",
   "Pick 8 Person Level Call: ",
   "Create Setup: ",
};

static void display_reconcile_history(int current_depth, int n);



static char *resolve_distances[] = {
   "0",
   "1/8",
   "1/4",
   "3/8",
   "1/2",
   "5/8",
   "3/4",
   "7/8",
   "0"};

/* BEWARE!!  This enum must track the table "resolve_first_parts". */
typedef enum {
   first_part_none,
   first_part_ext,
   first_part_slcl,
   first_part_circ,
   first_part_pthru,
   first_part_trby,
   first_part_xby
} first_part_kind;

/* Beware!!  This table must track the definition of enum "first_part_kind". */
static Cstring resolve_first_parts[] = {
   (Cstring) 0,
   "extend",
   "slip the clutch",
   "circulate",
   "pass thru",
   "trade by",
   "cross by"};

/* BEWARE!!  This enum must track the table "resolve_main_parts". */
typedef enum {
   main_part_none,
   main_part_rlg,
   main_part_la,
   main_part_dixgnd,
   main_part_minigrand,
   main_part_prom,
   main_part_revprom,
   main_part_sglprom,
   main_part_rsglprom,
   main_part_circ,
   main_part_swing
} main_part_kind;

/* Beware!!  This table must track the definition of enum "main_part_kind". */
static Cstring resolve_main_parts[] = {
   "???",
   "right and left grand",
   "left allemande",
   "dixie grand, left allemande",
   "mini-grand",
   "promenade",
   "reverse promenade",
   "single file promenade",
   "reverse single file promenade",
   "circle right",
   "swing and promenade"};


typedef struct {
   int how_bad;  /* 0 means accept all such resolves.
                  Otherwise, this is (2**N)-1, and accepts only one out of 2**N of them. */
   first_part_kind first_part;
   main_part_kind main_part;
} resolve_descriptor;

/* BEWARE!!  This list is keyed to the definition of "resolve_kind" in sd.h . */
static resolve_descriptor resolve_table[] = {
   {3,  first_part_none,  main_part_none},     /* resolve_none */
   {0,  first_part_none,  main_part_rlg},      /* resolve_rlg */
   {0,  first_part_none,  main_part_la},       /* resolve_la */
   {1,  first_part_ext,   main_part_rlg},      /* resolve_ext_rlg */
   {1,  first_part_ext,   main_part_la},       /* resolve_ext_la */
   {3,  first_part_slcl,  main_part_rlg},      /* resolve_slipclutch_rlg */
   {3,  first_part_slcl,  main_part_la},       /* resolve_slipclutch_la */
   {7,  first_part_circ,  main_part_rlg},      /* resolve_circ_rlg */
   {7,  first_part_circ,  main_part_la},       /* resolve_circ_la */
   {7,  first_part_pthru, main_part_rlg},      /* resolve_pth_rlg */
   {7,  first_part_pthru, main_part_la},       /* resolve_pth_la */
   {15, first_part_trby,  main_part_rlg},      /* resolve_tby_rlg */
   {15, first_part_trby,  main_part_la},       /* resolve_tby_la */
   {1,  first_part_xby,   main_part_rlg},      /* resolve_xby_rlg */
   {1,  first_part_xby,   main_part_la},       /* resolve_xby_la */
   {1,  first_part_none,  main_part_dixgnd},   /* resolve_dixie_grand */
   {3,  first_part_none,  main_part_minigrand},/* resolve_minigrand */
   {0,  first_part_none,  main_part_prom},     /* resolve_prom */
   {1,  first_part_none,  main_part_revprom},  /* resolve_revprom */
   {7,  first_part_none,  main_part_sglprom},  /* resolve_sglfileprom */
   {15, first_part_none,  main_part_rsglprom}, /* resolve_revsglfileprom */
   {3,  first_part_none,  main_part_circ}};    /* resolve_circle */


/* This assumes that "sequence_is_resolved" passes. */
SDLIB_API void write_resolve_text(long_boolean doing_file)
{
   resolve_indicator r = history[history_ptr].resolve_flag;
   int distance = r.distance;

   if (history[history_ptr].state.result_flags & RESULTFLAG__PLUSEIGHTH_ROT)
      distance++;

   distance &= 7;

   if (doing_file && !ui_options.singlespace_mode) doublespace_file();

   if (r.kind == resolve_circle) {
      if (distance == 0) {
         if (history[history_ptr].state.result_flags & RESULTFLAG__IMPRECISE_ROT)
            writestuff("approximately ");
         writestuff("at home");
      }
      else {
         writestuff("circle left ");
         if (history[history_ptr].state.result_flags & RESULTFLAG__IMPRECISE_ROT)
            writestuff("approximately ");
         writestuff(resolve_distances[8 - distance]);
         writestuff(" or right ");
         if (history[history_ptr].state.result_flags & RESULTFLAG__IMPRECISE_ROT)
            writestuff("approximately ");
         writestuff(resolve_distances[distance]);
      }
   }
   else {
      int index = (int) r.kind;
      first_part_kind first;
      main_part_kind mainpart;

      first = resolve_table[index].first_part;
      mainpart = resolve_table[index].main_part;

      /* In a singer, "pass thru, allemande left", "trade by, allemande left", or
         "cross by, allemande left" can be just "swing and promenade". */

      if (singing_call_mode != 0 &&
          (index == resolve_pth_la ||
           index == resolve_tby_la ||
           index == resolve_xby_la)) {
         first = first_part_none;
         mainpart = main_part_swing;
      }

      if (first != first_part_none) {
         writestuff(resolve_first_parts[first]);
         if (doing_file) {
            (*the_callback_block.newline_fn)();
            if (!ui_options.singlespace_mode) doublespace_file();
         }
         else
            writestuff(", ");
      }

      if (singing_call_mode != 0 && mainpart == main_part_rlg) {
         mainpart = main_part_swing;
         distance ^= 4;
      }

      writestuff(resolve_main_parts[mainpart]);

      writestuff("  (");
      if (history[history_ptr].state.result_flags & RESULTFLAG__IMPRECISE_ROT)
         writestuff("approximately ");

      if (distance == 0) {
         writestuff("at home)");
      }
      else {
         if (r.kind == resolve_revprom ||
             r.kind == resolve_revsglfileprom)
            writestuff(resolve_distances[8 - distance]);
         else
            writestuff(resolve_distances[distance]);
         writestuff(" promenade)");
      }
   }
}



/* These variables are actually local to inner_search, but they are
   expected to be preserved across the throw, so they must be static. */

static int perm_indices[8];
static int attempt_count, little_count;
static int hashed_random_list[5];
static parse_block *inner_parse_mark, *outer_parse_mark;
static int history_insertion_point;    /* Where our resolve should lie in the history.
                                          This is normally the end of the history, but
                                          may be earlier if doing a reconcile.  We clobber
                                          everything in the given history past this point. */
static int history_save;               /* Where we are inserting calls now.  This moves
                                          forward as we build multiple-call resolves. */


static long_boolean inner_search(command_kind goal,
                                 resolve_rec *new_resolve,
                                 int insertion_point)
{
   long_boolean retval;
   int i, j;
   setup *ns;
   uint32 directions, p, q;

   history_insertion_point = huge_history_ptr;

   if (goal == command_reconcile) {
      history_insertion_point -= insertion_point;    /* strip away the extra calls */

      goal_kind = history[history_insertion_point].state.kind;
      if (setup_attrs[goal_kind].setup_limits != 7) return(FALSE);
      for (j=0; j<8; j++) goal_directions[j] = history[history_insertion_point].state.people[j].id1 & d_mask;

      for (j=0; j<8; j++) {
         perm_indices[j] = -1;
         for (i=0; i<8; i++)
            if ((history[history_insertion_point].state.people[i].id1 & PID_MASK) == perm_array[j]) perm_indices[j] = i;
         if (perm_indices[j] < 0) return(FALSE);      /* didn't find the person???? */
      }
   }

   history_save = history_insertion_point;

   little_count = 0;
   attempt_count = 0;
   hashed_random_list[0] = 0;

   /* Mark the parse block allocation, so that we throw away the garbage
      created by failing attempts. */
   inner_parse_mark = outer_parse_mark = mark_parse_blocks();

   /* This loop searches through a group of twenty single-call resolves, then a group
      of twenty two-call resolves, then a group of twenty three-call resolves,
      repeatedly.  Any time it finds a resolve in less than the length of the sequence
      it is looking for, it of course accepts it.  Why don't we simply always search
      for three call resolves and accept shorter ones that it might stumble upon?
      Because this might make the program "lazy": it would settle for long resolves
      rather than looking harder for short ones.  We bias it in favor of short
      resolves by making it sometimes search only for short ones, refusing to look
      deeper when an attempt fails.  The searches are in groups of twenty in order
      to save time: once we decide to search for some two-call resolves, we re-use
      the setup left by the same initial call. */

   try_again:

   try {
      /* Throw away garbage from last attempt. */
      release_parse_blocks_to_mark(inner_parse_mark);
      testing_fidelity = FALSE;
      history_ptr = history_save;
      attempt_count++;
      if (attempt_count > ATTEMPTS_PER_COMMAND) {
         /* Too many tries -- too bad. */
         history_ptr = huge_history_ptr;

         /* We shouldn't have to do the following stuff.  The searcher should be written
            such that it doesn't get stuck on a call with any iterator nonzero, because,
            if the iterator is ever set to zero, the current call should continue
            cycling that iterator until it goes back to zero.  However, if there were
            a bug in that code, the consequences would be extremely embarrassing.
            The resolver would just be stuck, repeatedly reporting failure until the
            entire resolve operation is manually aborted.  To be sure that never happens,
            we do this.  This could have the effect of prematurely terminating an
            iteration search, but no one will notice. */

         reset_internal_iterators();
         retval = FALSE;
         goto timeout;
      }

      /* Now clear any concepts if we are not on the first call of the series. */

      if (history_ptr != history_insertion_point || goal == command_reconcile)
         initialize_parse();
      else
         (void) restore_parse_state();

      /* Generate the concepts and call. */

      hashed_randoms = hashed_random_list[history_ptr - history_insertion_point];

      /* Put in a special initial concept if needed to normalize. */

      if (goal == command_normalize && !concepts_in_place()) {
         int k, l, c;

         for (k=0 ; k < NUM_NICE_START_KINDS ; k++) {
            if (nice_setup_info[k].kind == history[history_ptr].state.kind) {
               l = nice_setup_info[k].number_available_now;
               if (l != 0) goto found_k_and_l;
               else goto try_again;  /* This shouldn't happen, because we are screening setups carefully. */
            }
         }

         goto try_again;   /* This shouldn't happen. */

      found_k_and_l:

         c = nice_setup_info[k].array_to_use_now
            [(*the_callback_block.generate_random_number_fn)(l)];

         /* If the concept is a tandem or as couples type, we really want "phantom"
            or "2x8 matrix"" in front of it. */

         if (concept_descriptor_table[c].kind == concept_tandem) {
            if (history[history_ptr].state.kind == s4x4)
               (*the_callback_block.deposit_concept_fn)(&concept_descriptor_table[phantom_concept_index]);
            else
               (*the_callback_block.deposit_concept_fn)(&concept_descriptor_table[matrix_2x8_concept_index]);
         }

         (*the_callback_block.deposit_concept_fn)(&concept_descriptor_table[c]);
      }

      /* Select the call.  Selecting one that says "don't use in resolve" will signal and go to try_again. */
      /* This may, of course, add more concepts. */

      (void) (*the_callback_block.query_for_call_fn)();

      /* Do the call.  An error will signal and go to try_again. */

      toplevelmove();
      finish_toplevelmove();

      /* We don't like certain warnings either. */
      if (warnings_are_unacceptable(goal != command_standardize)) goto try_again;

      /* See if we have already seen this sequence. */

      for (i=0; i<avoid_list_size; i++) {
         if (hashed_randoms == avoid_list[i]) goto try_again;
      }

      /* The call was legal, see if it satisfies our criterion. */

      /* ***** Because of an apparent bug in the C compiler, the "switch" statement that we would
         like to have used doesn't work, so we use an "if". */

      ns = &history[history_ptr+1].state;

      if (goal == command_resolve) {
         resolve_kind r = history[history_ptr+1].resolve_flag.kind;

         /* Here we bias the search against resolves with circulates (which we
            consider to be of lower quality) by only sometimes accepting them.

            As more bits are set in the "how_bad" indicator, we ignore a larger
            fraction of the resolves.  For example, we bias the search VERY HEAVILY
            against reverse single file promenades, accepting only 1 in 16.
         */

         if (r == resolve_none) goto what_a_loss;

         switch (get_resolve_goodness_info()) {
         case resolve_goodness_only_nice:
            /* Only accept things with "how_bad" = 0, that is, RLG, LA, and prom.
               Furthermore, at C2 and above, only accept if promenade distance
               is 0 to 3/8. */
            if (resolve_table[r].how_bad != 0 ||
                (calling_level >= l_c2 &&
                 ((history[history_ptr+1].resolve_flag.distance & 7) > 3)))
               goto what_a_loss;
            break;
         case resolve_goodness_always:
            /* Accept any one-call resolve. */
            break;
         case resolve_goodness_maybe:
            /* Accept resolves randomly.  The probability that we reject a
               resolve increases as the resolve quality goes down. */
            if (resolve_table[r].how_bad & attempt_count) goto what_a_loss;
            break;
         }
      }
      else if (goal == command_normalize) {
         /* We accept any setup with 8 people in it.  This could conceivably give
            somewhat unusual setups like dogbones or riggers, but they might be
            sort of interesting if they arise.  (Actually, it is highly unlikely,
            given the concepts that we use.) */
         if (setup_attrs[ns->kind].setup_limits != 7) goto what_a_loss;
      }
      else if (goal == command_standardize) {
         uint32 tb = 0;
         uint32 tbtb = 0;
         uint32 tbpt = 0;

         for (i=0 ; i<8 ; i++) {
            tb |= ns->people[i].id1;
            tbtb |= ns->people[i].id1 ^ ((i & 2) << 2);
            tbpt |= ns->people[i].id1 ^ (i & 1);
         }

         if (ns->kind == s2x4 || ns->kind == s1x8) {
            if ((tb & 011) == 011) goto what_a_loss;
         }
         else if (ns->kind == s_qtag) {
            if ((tb & 1) != 0 && (tbtb & 010) != 0) goto what_a_loss;
         }
         else if (ns->kind == s_ptpd) {
            if ((tb & 010) != 0 && (tbpt & 1) != 0) goto what_a_loss;
         }
         else
            goto what_a_loss;
      }
      else if (goal == command_reconcile) {
         if (ns->kind != goal_kind) goto what_a_loss;
         for (j=0; j<8; j++)
            if ((ns->people[j].id1 & d_mask) != goal_directions[j]) goto what_a_loss;

         int p0 = ns->people[perm_indices[0]].id1 & PID_MASK;
         int p1 = ns->people[perm_indices[1]].id1 & PID_MASK;
         int p2 = ns->people[perm_indices[2]].id1 & PID_MASK;
         int p3 = ns->people[perm_indices[3]].id1 & PID_MASK;
         int p4 = ns->people[perm_indices[4]].id1 & PID_MASK;
         int p5 = ns->people[perm_indices[5]].id1 & PID_MASK;
         int p6 = ns->people[perm_indices[6]].id1 & PID_MASK;
         int p7 = ns->people[perm_indices[7]].id1 & PID_MASK;

         // Test for absolute sex correctness if required.
         if (!current_reconciler->allow_eighth_rotation && (p0 & 0100)) goto what_a_loss;

         p7 = (p7 - p6) & PID_MASK;
         p6 = (p6 - p5) & PID_MASK;
         p5 = (p5 - p4) & PID_MASK;
         p4 = (p4 - p3) & PID_MASK;
         p3 = (p3 - p2) & PID_MASK;
         p2 = (p2 - p1) & PID_MASK;
         p1 = (p1 - p0) & PID_MASK;

         // Test each sex individually for uniformity of offset around the ring.
         if (p1 != p3 || p3 != p5 || p5 != p7 || p2 != p4 || p4 != p6)
            goto what_a_loss;

         if (((p1 + p2) & PID_MASK) != 0200) // Test for each sex in sequence.
            goto what_a_loss;

         if ((p2 & 0100) == 0)               // Test for alternating sex.
            goto what_a_loss;

         // Test for relative phase of boys and girls.
         // "accept_extend" tells how accurate the placement must be.
         switch (singing_call_mode) {
         case 1: p2 -= 0200; break;
         case 2: p2 -= 0600; break;
         }

         if (((p2 >>= 7) & 3) > current_reconciler->accept_extend)
            goto what_a_loss;
      }
      else if (goal >= command_create_any_lines) {
         directions = 0;
         for (i=0 ; i<8 ; i++) {
            directions <<= 2;
            directions |= ns->people[i].id1 & 3;
         }

         switch (goal) {
         case command_create_any_lines:
            if (ns->kind != s2x4 || (directions & 0x5555) != 0) goto what_a_loss;
            break;
         case command_create_any_col:
            if (ns->kind != s2x4 || (directions & 0x5555) != 0x5555) goto what_a_loss;
            break;
         case command_create_any_qtag:
            if (ns->kind != s_qtag || (directions & 0x5555) != 0) goto what_a_loss;
            break;
         case command_create_any_tidal:
            if (ns->kind != s1x8) goto what_a_loss;
            break;
         case command_create_waves:
            if (ns->kind != s2x4 || (directions != 0x2288 && directions != 0x8822)) goto what_a_loss;
            break;
         case command_create_2fl:
            if (ns->kind != s2x4 || (directions != 0x0AA0 && directions != 0xA00A)) goto what_a_loss;
            break;
         case command_create_inv_lines:
            if (ns->kind != s2x4 || (directions != 0x2882 && directions != 0x8228)) goto what_a_loss;
            break;
         case command_create_3and1_lines:
            p = (directions ^ (directions >> 6)) & 0x202;
            q = ((directions ^ (directions >> 2)) >> 2) & 0x202;
            if (ns->kind != s2x4 || (directions & 0x5555) != 0 || (p | q) == 0 || p == q) goto what_a_loss;
            break;
         case command_create_tidal_wave:
            if (ns->kind != s1x8 || (directions != 0x2882 && directions != 0x8228)) goto what_a_loss;
            break;
         case command_create_col:
            if (ns->kind != s2x4 || (directions != 0x55FF && directions != 0xFF55)) goto what_a_loss;
            break;
         case command_create_magic_col:
            if (ns->kind != s2x4 || (directions != 0x7DD7 && directions != 0xD77D)) goto what_a_loss;
            break;
         case command_create_qtag:
            if (ns->kind != s_qtag || (directions & 0xF0F0) != 0xA000 || ((directions & 0x0F0F) != 0x0802 && (directions & 0x0F0F) != 0x0208)) goto what_a_loss;
            break;
         case command_create_3qtag:
            if (ns->kind != s_qtag || (directions & 0xF0F0) != 0x00A0 || ((directions & 0x0F0F) != 0x0802 && (directions & 0x0F0F) != 0x0208)) goto what_a_loss;
            break;
         case command_create_qline:
            if (ns->kind != s_qtag || (directions & 0xF0F0) != 0xA000 || ((directions & 0x0F0F) != 0x0A00 && (directions & 0x0F0F) != 0x000A)) goto what_a_loss;
            break;
         case command_create_3qline:
            if (ns->kind != s_qtag || (directions & 0xF0F0) != 0x00A0 || ((directions & 0x0F0F) != 0x0A00 && (directions & 0x0F0F) != 0x000A)) goto what_a_loss;
            break;
         case command_create_dmd:
            if (ns->kind != s_qtag || (directions & 0x5555) != 0x5050) goto what_a_loss;
            break;
         case command_create_li:
            if (ns->kind != s2x4 || directions != 0xAA00) goto what_a_loss;
            break;
         case command_create_lo:
            if (ns->kind != s2x4 || directions != 0x00AA) goto what_a_loss;
            break;
         case command_create_dpt:
            if (ns->kind != s2x4 || directions != 0x5FF5) goto what_a_loss;
            break;
         case command_create_cdpt:
            if (ns->kind != s2x4 || directions != 0xF55F) goto what_a_loss;
            break;
         case command_create_tby:
            if (ns->kind != s2x4 || directions != 0xDD77) goto what_a_loss;
            break;
         case command_create_8ch:
            if (ns->kind != s2x4 || directions != 0x77DD) goto what_a_loss;
            break;
         }
      }
      else if (goal == command_8person_level_call) {
         /* We demand that no splitting have taken place along either axis. */
         if (ns->result_flags & RESULTFLAG__SPLIT_AXIS_FIELDMASK) goto what_a_loss;
      }

      /* The call (or sequence thereof) seems to satisfy our criterion.  Just to be
         sure, we have to examine all future calls (for a reconcile -- for other stuff
         there are no future calls), to make sure that, aside from the permutation
         that gets performed, they will be executed the same way. */

      /* But first, we make the dynamic part of the parse state be a copy of what we
         had, since we are repeatedly overwriting existing blocks. */

      /* The solution that we have found consists of the parse blocks hanging off of
         huge_history_ptr+1 ... history_ptr inclusive.  We have to make sure that they will
         be safe forever.  (That is, until we have exited the entire resolve operation.)
         For the most part, this follows from the fact that we will not re-use any
         already-in-use parse blocks.  But the tree hanging off of huge_history_ptr+1
         gets destructively reset to the initial state by restore_parse_state, so we must
         protect it. */

      history[huge_history_ptr+1].command_root = copy_parse_tree(history[huge_history_ptr+1].command_root);

      /* Save the entire resolve, that is, the calls we inserted, and where we inserted them. */

      history_ptr++;
      new_resolve->size = history_ptr - history_insertion_point;

      if (goal == command_reconcile) {
         for (j=0; j<8; j++) {
            new_resolve->permute1[perm_array[j] >> 6] = ns->people[perm_indices[j]].id1 & PID_MASK;
            new_resolve->permute2[perm_array[j] >> 6] = ns->people[perm_indices[j]].id1 & ID1_PERM_ALLBITS;
         }

         new_resolve->rotchange = ns->rotation - history[history_insertion_point].state.rotation;
         new_resolve->insertion_point = insertion_point;
      }
      else {
         new_resolve->insertion_point = 0;
      }

      /* Now test the "fidelity" of the pre-existing calls after the insertion point,
         to be sure they still behave the way we expect, that is, that they move the
         permuted people around in the same way.  (If one of those calls uses a predicate
         like "heads" or "boys" it will likely fail this test until we get around to
         doing something clever.  Oh well.) */

      testing_fidelity = TRUE;

      for (j=0; j<new_resolve->insertion_point; j++) {
         int k;
         configuration this_state;

         /* Copy the whole thing into the history, chiefly to get the call and concepts. */
         written_history_items = -1;

         history[history_ptr+1] = huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point];

      /* Now execute the call again, from the new starting configuration. */
      /* This might signal and go to try_again. */
         toplevelmove();
         finish_toplevelmove();

         this_state = history[history_ptr+1];
         this_state.state.rotation -= new_resolve->rotchange;
         canonicalize_rotation(&this_state.state);

         if (this_state.state.rotation != huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].state.rotation)
            goto try_again;

         if (this_state.warnings !=
             huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].warnings)
            goto try_again;

         for (k=0; k<=setup_attrs[this_state.state.kind].setup_limits; k++) {
            personrec t = huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].state.people[k];

            if (t.id1) {
               if (this_state.state.people[k].id1 !=
                   ((t.id1 & ~(PID_MASK | ID1_PERM_ALLBITS)) | new_resolve->permute1[(t.id1 & PID_MASK) >> 6] | new_resolve->permute2[(t.id1 & PID_MASK) >> 6]))
                  goto try_again;
               if (this_state.state.people[k].id2 != t.id2)
                  goto try_again;
            }
            else {
               if (this_state.state.people[k].id1)
                  goto try_again;
            }
         }

         history_ptr++;
      }

      testing_fidelity = FALSE;

      /* One more check.  If this was a "reconcile", demand that we have an acceptable resolve.
         How could the permutation be acceptable but not lead to an acceptable resolve?  Because,
         if the resolve is "at home", we demand that the promenade distance be zero.  Our
         previous tests were impervious to promenade distance, because it usually doesn't matter.
         But for "at home", resolve_p will only show a resolve if the distance is zero.
         Note that the above comment is obsolete, because we now allow circling a nonzero amount.
         However, it does little harm to leave this test in place, and it might avoid future
         problems if rotation-sensitive resolves are ever re-introduced. */

      if (goal == command_reconcile && history[history_ptr].resolve_flag.kind == resolve_none)
         goto try_again;   /* Sorry. */

      /* We win.  Really save it and exit.  History_ptr has been clobbered. */

      for (j=0; j<MAX_RESOLVE_SIZE; j++)
         new_resolve->stuph[j] = history[j+history_insertion_point+1];

      /* Grow the "avoid_list" array as needed. */

      avoid_list_size++;

      if (avoid_list_allocation <= avoid_list_size) {
         int *t;
         avoid_list_allocation = avoid_list_size+100;
         t = (int *) (*the_callback_block.get_more_mem_gracefully_fn)(avoid_list, avoid_list_allocation * sizeof(int));
         if (!t) specialfail("Not enough memory!");
         avoid_list = t;
      }

      avoid_list[avoid_list_size-1] = hashed_randoms;   /* It's now safe to do this. */

      retval = TRUE;
      goto timeout;

   what_a_loss:

      if (!pick_allow_multiple_items()) goto try_again;

      if (++little_count == 60) {
         /* Revert back to beginning. */
         history_save = history_insertion_point;
         inner_parse_mark = outer_parse_mark;
         little_count = 0;
      }
      else if (little_count == 20 || little_count == 40) {
         /* Save current state as a base for future calls. */

         /* But first, if doing a "normalize" operation, we verify that the setup
            we have arrived at is one from which we know how to do something.  Otherwise,
            there is no point in trying to build on the setup at which we have arrived.
            Also, if the setup has gotten bigger, do not proceed. */

         if (goal == command_normalize) {
            int k;

            if (setup_attrs[ns->kind].setup_limits > setup_attrs[history[history_ptr].state.kind].setup_limits)
               goto try_again;

            for (k=0 ; k < NUM_NICE_START_KINDS ; k++) {
               if (nice_setup_info[k].kind == ns->kind && nice_setup_info[k].number_available_now != 0)
                  goto ok_to_save_this;
            }

            goto try_again;

         ok_to_save_this: ;
         }

         history_save = history_ptr + 1;
         inner_parse_mark = mark_parse_blocks();
         hashed_random_list[history_save - history_insertion_point] = hashed_randoms;
      }
   }
   catch(error_flag_type) {
   }

   goto try_again;

 timeout:

   return retval;
}


SDLIB_API uims_reply full_resolve(void)
{
   int j, k;
   uims_reply reply;
   int current_resolve_index, max_resolve_index;
   long_boolean show_resolve;
   personrec *current_people = history[history_ptr].state.people;
   int current_depth = 0;
   long_boolean find_another_resolve = TRUE;
   resolver_display_state big_state; /* for display to the user */

   /* Allocate or reallocate the huge_history_save save array if needed. */

   if (huge_history_allocation < history_ptr+MAX_RESOLVE_SIZE+2) {
      configuration *t;
      huge_history_allocation = history_ptr+MAX_RESOLVE_SIZE+2;
      /* Increase by 50% beyond what we have now. */
      huge_history_allocation += huge_history_allocation >> 1;
      t = (configuration *)
         (*the_callback_block.get_more_mem_gracefully_fn)(huge_history_save,
                                 huge_history_allocation * sizeof(configuration));
      if (!t) specialfail("Not enough memory!");
      huge_history_save = t;
   }

   /* Do the resolve array. */

   if (all_resolves == 0) {
      resolve_allocation = 10;
      all_resolves = (resolve_rec *) (*the_callback_block.get_mem_gracefully_fn)(resolve_allocation * sizeof(resolve_rec));
      if (!all_resolves) specialfail("Not enough memory!");
   }

   /* Be sure the extra 5 slots in the history array are clean. */

   for (j=0; j<MAX_RESOLVE_SIZE; j++) {
      history[history_ptr+j+2].command_root = (parse_block *) 0;
      history[history_ptr+j+2].centersp = 0;
   }

   /* See if we are in a reasonable position to do the search. */

   switch (search_goal) {
      case command_resolve:
         if (!resolve_command_ok())
            specialfail("Not in acceptable setup for resolve.");
         break;
      case command_standardize:
         if (!resolve_command_ok())
            specialfail("Not in acceptable setup for standardize.");
         break;
      case command_reconcile:
         if (!reconcile_command_ok())
            specialfail("Not in acceptable setup for reconcile, or sequence is too short, or concepts are selected.");

         for (j=0; j<8; j++)
            perm_array[j] = current_people[current_reconciler->perm[j]].id1 & PID_MASK;

         current_depth = 1;
         find_another_resolve = FALSE;       /* We initially don't look for resolves; we wait for the user
                                                to set the depth. */
         break;
      case command_normalize:
         if (!nice_setup_command_ok())
            specialfail("Sorry, can't do this: concepts are already selected, or no applicable concepts are available.");
         break;
   }

   for (j=0; j<=history_ptr+1; j++)
      huge_history_save[j] = history[j];

   huge_history_ptr = history_ptr;
   save_parse_state();

   (void) restore_parse_state();
   current_resolve_index = 0;
   show_resolve = TRUE;
   max_resolve_index = 0;
   avoid_list_size = 0;

   if (search_goal == command_reconcile) show_resolve = FALSE;

   start_pick();   /* This sets interactivity, among other stuff. */

   for (;;) {
      /* We know the history is restored at this point. */
      if (find_another_resolve) {
         /* Put up the resolve title showing that we are searching. */

         (*the_callback_block.uims_update_resolve_menu_fn)
            (search_goal, current_resolve_index, max_resolve_index, resolver_display_searching);

         (void) restore_parse_state();

         if (inner_search(search_goal, &all_resolves[max_resolve_index], current_depth)) {
            /* Search succeeded, save it. */
            max_resolve_index++;
            /* Make it the current one. */
            current_resolve_index = max_resolve_index;

            /* Put up the resolve title showing this resolve,
               but without saying "searching". */
            big_state = resolver_display_ok;
         }
         else {
            /* Put up a resolve title indicating failure. */
            big_state = resolver_display_failed;
         }

         written_history_items = -1;
         history_ptr = huge_history_ptr;

         for (j=0; j<=history_ptr+1; j++)
            history[j] = huge_history_save[j];

         find_another_resolve = FALSE;
      }
      else {
         /* Just display the sequence with the current resolve inserted. */
         /* Put up a neutral resolve title. */
         big_state = resolver_display_ok;
      }

      /* Modify the history to show the current resolve. */
      /* Note that the currrent history has been restored to its saved state. */

      if ((current_resolve_index != 0) && show_resolve) {
         /* Display the current resolve. */
         resolve_rec *this_resolve;
         configuration *this_state;

         this_resolve = &all_resolves[current_resolve_index-1];

         /* Copy the inserted calls. */
         written_history_items = -1;
         for (j=0; j<this_resolve->size; j++)
            history[j+huge_history_ptr+1-this_resolve->insertion_point] = this_resolve->stuph[j];

         /* Copy and repair the calls after the insertion. */
         for (j=0; j<this_resolve->insertion_point; j++) {
            this_state = &history[j+huge_history_ptr+1-this_resolve->insertion_point+this_resolve->size];
            *this_state = huge_history_save[j+huge_history_ptr+1-this_resolve->insertion_point];
            this_state->state.rotation += this_resolve->rotchange;
            canonicalize_rotation(&this_state->state);

            /* Repair this setup by permuting all the people. */

            for (k=0; k<=setup_attrs[this_state->state.kind].setup_limits; k++) {
               personrec t = this_state->state.people[k];

               if (t.id1) {
                  this_state->state.people[k].id1 =
                     (t.id1 & ~(PID_MASK | ID1_PERM_ALLBITS)) | this_resolve->permute1[(t.id1 & PID_MASK) >> 6]
                                                              | this_resolve->permute2[(t.id1 & PID_MASK) >> 6];
                  this_state->state.people[k].id2 = t.id2;
               }
            }

            this_state->resolve_flag = resolve_p(&this_state->state);
         }

         history_ptr = huge_history_ptr + this_resolve->size;

         /* Show the history up to the start of the resolve, forcing a picture on the last item (unless reconciling). */

         display_initial_history(huge_history_ptr-this_resolve->insertion_point,
                                 search_goal != command_reconcile);

         /* Or a dotted line if doing a reconcile. */
         if (search_goal == command_reconcile) {
            writestuff("------------------------------------");
            (*the_callback_block.newline_fn)();
         }

         /* Show the resolve itself, without its last item. */

         for (j=huge_history_ptr-this_resolve->insertion_point+1; j<history_ptr-this_resolve->insertion_point; j++)
            write_history_line(j, (char *) 0, FALSE, file_write_no);

         /* Show the last item of the resolve, with a forced picture. */
         write_history_line(history_ptr-this_resolve->insertion_point,
                            (char *) 0,
                            search_goal != command_reconcile,
                            file_write_no);

         /* Or a dotted line if doing a reconcile. */
         if (search_goal == command_reconcile) {
            writestuff("------------------------------------");
            (*the_callback_block.newline_fn)();
         }

         /* Show whatever comes after the resolve. */
         for (j=history_ptr-this_resolve->insertion_point+1; j<=history_ptr; j++)
            write_history_line(j, (char *) 0, j==history_ptr-this_resolve->insertion_point, file_write_no);
      }
      else if (show_resolve) {
         /* We don't have any resolve to show.  Just draw the usual picture. */
         display_initial_history(huge_history_ptr, 2);
      }
      else {
         /* Don't show any resolve, because we want to display the current
            insertion point. */
         display_reconcile_history(current_depth, huge_history_ptr);
      }

      if (show_resolve && (history[history_ptr].resolve_flag.kind != resolve_none)) {
         (*the_callback_block.newline_fn)();
         writestuff("     resolve is:");
         (*the_callback_block.newline_fn)();
         write_resolve_text(FALSE);
         (*the_callback_block.newline_fn)();
         (*the_callback_block.newline_fn)();
      }

      (*the_callback_block.uims_update_resolve_menu_fn)
         (search_goal, current_resolve_index, max_resolve_index, big_state);

      show_resolve = TRUE;

      for (;;) {          /* We ignore any "undo" or "erase" clicks. */
         reply = (*the_callback_block.uims_get_resolve_command_fn)();
         if ((reply != ui_command_select) || ((uims_menu_index != command_undo) && (uims_menu_index != command_erase))) break;
      }

      if (reply == ui_resolve_select) {
         switch ((resolve_command_kind) uims_menu_index) {
         case resolve_command_find_another:
            if (resolve_allocation <= max_resolve_index) {
               /* Increase allocation if necessary. */
               int new_allocation = resolve_allocation << 1;
               resolve_rec *t = (resolve_rec *)
                  (*the_callback_block.get_more_mem_gracefully_fn)(all_resolves, new_allocation * sizeof(resolve_rec));
               if (!t) break;               /* By not turning on "find_another_resolve",
                                               we will take no action. */
               resolve_allocation = new_allocation;
               all_resolves = t;
            }

            find_another_resolve = TRUE;             /* will get it next time around */
            break;
         case resolve_command_goto_next:
            if (current_resolve_index < max_resolve_index)
               current_resolve_index++;
            break;
         case resolve_command_goto_previous:
            if (current_resolve_index > 1)
               current_resolve_index--;
            break;
         case resolve_command_raise_rec_point:
            if (current_depth < huge_history_ptr-2)
               current_depth++;
            show_resolve = FALSE;
            break;
         case resolve_command_lower_rec_point:
            if (current_depth > 0)
               current_depth--;
            show_resolve = FALSE;
            break;
         case resolve_command_abort:
            written_history_items = -1;
            history_ptr = huge_history_ptr;

            for (j=0; j<=history_ptr+1; j++)
               history[j] = huge_history_save[j];

            goto getout;
         case resolve_command_write_this:
            reply = ui_command_select;
            uims_menu_index = command_getout;
            goto getout;
         default:
            /* Clicked on "accept choice", or something not on this submenu. */
            goto getout;
         }
      }
      else if ((reply == ui_command_select) && (uims_menu_index == command_refresh)) {
         /* fall through to redisplay */
         ;
      }
      else {
         /* Clicked on "accept choice", or something not on this submenu. */
         goto getout;
      }

      /* Restore history for next cycle. */
      written_history_items = -1;
      history_ptr = huge_history_ptr;

      for (j=0; j<=history_ptr+1; j++)
         history[j] = huge_history_save[j];
   }

   getout:

   interactivity = interactivity_normal;
   end_pick();
   return reply;
}

static void display_reconcile_history(int current_depth, int n)
{
   int j;

   display_initial_history(n-current_depth, 0);
   if (current_depth > 0) {
      writestuff("------------------------------------");
      (*the_callback_block.newline_fn)();
      for (j=n-current_depth+1; j<=n; j++)
         write_history_line(j, (char *) 0, FALSE, file_write_no);
   }

   (*the_callback_block.uims_add_new_line_fn)("", 0);   // Write a blank line.
}

extern int concepts_in_place(void)
{
   return history[history_ptr+1].command_root != 0;
}


extern int reconcile_command_ok(void)
{
   int k;
   int dirmask = 0;
   personrec *current_people = history[history_ptr].state.people;
   setup_kind current_kind = history[history_ptr].state.kind;
   current_reconciler = (reconcile_descriptor *) 0;

   /* Since we are going to go back 1 call, demand we have at least 3. ***** */
   /* Also, demand no concepts already in place. */
   if ((history_ptr < 3) || concepts_in_place()) return FALSE;

   for (k=0; k<8; k++)
      dirmask = (dirmask << 2) | (current_people[k].id1 & 3);

   if (current_kind == s2x4) {
      if (dirmask == 0xA00A)
         current_reconciler = &promperm;         /* L2FL, looking for promenade. */
      else if (dirmask == 0x0AA0)
         current_reconciler = &rpromperm;        /* R2FL, looking for reverse promenade. */
      else if (dirmask == 0x6BC1)
         current_reconciler = &homeperm;         /* pseudo-squared-set, looking for circle left/right. */
      else if (dirmask == 0xFF55)
         current_reconciler = &sglperm;          /* Lcol, looking for single file promenade. */
      else if (dirmask == 0x55FF)
         current_reconciler = &sglperm;          /* Rcol, looking for reverse single file promenade. */
      else if (dirmask == 0xBC16)
         current_reconciler = &sglperm;          /* L Tbone, looking for single file promenade. */
      else if (dirmask == 0x16BC)
         current_reconciler = &sglperm;          /* R Tbone, looking for reverse single file promenade. */
      else if (dirmask == 0x2288)
         current_reconciler = &rlgperm;          /* Rwave, looking for RLG (with possible extend or circulate). */
      else if (dirmask == 0x8822)
         current_reconciler = &laperm;           /* Lwave, looking for LA (with possible extend or circulate). */
   }
   else if (current_kind == s_qtag) {
      if (dirmask == 0x08A2)
         current_reconciler = &qtagperm;         /* Rqtag, looking for RLG. */
      else if (dirmask == 0x78D2)
         current_reconciler = &qtagperm;         /* diamonds with points facing, looking for RLG. */
   }
   else if (current_kind == s_crosswave || current_kind == s_thar) {
      if (dirmask == 0x278D)
         current_reconciler = &crossplus;        /* crossed waves or thar, looking for RLG, allow slip the clutch. */
      else if (dirmask == 0x8D27)
         current_reconciler = &crossplus;        /* crossed waves or thar, looking for LA, allow slip the clutch. */
      else if (dirmask == 0xAF05)
         current_reconciler = &crossperm;        /* crossed waves or thar, looking for promenade. */
   }

   if (current_reconciler)
      return TRUE;
   else
      return FALSE;
}

extern int resolve_command_ok(void)
{
   return setup_attrs[history[history_ptr].state.kind].setup_limits == 7;
}

extern int nice_setup_command_ok(void)
{
   int i, k;
   long_boolean setup_ok = FALSE;
   setup_kind current_kind = history[history_ptr].state.kind;

   /* Decide which arrays we will use, depending on the current setting of the "allow all concepts" flag,
      and see if we are in one of the known setups and there are concepts available for that setup. */

   for (k=0 ; k < NUM_NICE_START_KINDS ; k++) {
      /* Select the correct concept array. */
      nice_setup_info[k].array_to_use_now = (allowing_all_concepts) ? nice_setup_info[k].thing->full_list : nice_setup_info[k].thing->on_level_list;

      /* Note how many concepts are in it.  If there are zero in some of them, we may still be able to proceed,
         but we must have concepts available for the current setup. */

      for (i=0 ; ; i++) {
         if (nice_setup_info[k].array_to_use_now[i] == -1) break;
      }

      nice_setup_info[k].number_available_now = i;

      if (nice_setup_info[k].kind == current_kind && nice_setup_info[k].number_available_now != 0) setup_ok = TRUE;
   }

   return setup_ok || concepts_in_place();
}

/*
 * Create a string representing the search state.  Search_goal indicates which user command
 * is being performed.  If there is no current solution,
 * then M and N are both 0.  If there is a current
 * solution, the M is the solution index (minimum value 1) and N is the maximum
 * solution index (N>0).  State indicates whether a search is in progress or not, and
 * if not, whether the most recent search failed.
 */

SDLIB_API void create_resolve_menu_title(
   command_kind goal,
   int cur, int max,
   resolver_display_state state,
   char *title)
{
   char junk[MAX_TEXT_LINE_LENGTH];
   char *titleptr = title;
   if (goal > command_create_any_lines) goal = command_create_any_lines;

   string_copy(&titleptr, title_string[goal-command_resolve]);

   if (max > 0) {
      sprintf(junk, "%d out of %d", cur, max);
      string_copy(&titleptr, junk);
   }
   switch (state) {
      case resolver_display_ok:
         break;
      case resolver_display_searching:
         if (max > 0) string_copy(&titleptr, " ");
         string_copy(&titleptr, "searching ...");
         break;
      case resolver_display_failed:
         if (max > 0) string_copy(&titleptr, " ");
         string_copy(&titleptr, "failed");
         break;
   }
}


SDLIB_API void initialize_getout_tables(void)
{
   int i, j, k;

   for (k=0 ; k < NUM_NICE_START_KINDS ; k++) {
      nice_setup_thing *nice = nice_setup_info[k].thing;

      /* Create the "on level" lists if not already created.
         Since we re-use some stuff (e.g. 1x10 and 1x12 both use
         the 1x12 things), it might not be necessary. */

      if (!nice->on_level_list) {
         nice->on_level_list = (int *) (*the_callback_block.get_mem_fn)(nice->full_list_size);

         /* Copy those concepts that are on the level. */
         for (i=0,j=0 ; ; i++) {
            if (nice->full_list[i] == -1) break;
            if (concept_descriptor_table[nice->full_list[i]].level <= calling_level)
               nice->on_level_list[j++] = nice->full_list[i];
         }

         /* Put in the end mark. */
         nice->on_level_list[j] = -1;
      }
   }
}
