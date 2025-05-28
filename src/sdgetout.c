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

    This is for version 29. */

/* This defines the following functions:
   resolve_p
   full_resolve
   concepts_in_place
   reconcile_command_ok
   resolve_command_ok
   nice_setup_command_ok
   create_resolve_menu_title
*/

#include "sd.h"


/* bits that we need to manipulate, segregated by word number in the personrec */
#define ID_BITS_1 000000700
#define ID_BITS_2 017770000


/* maximum number of hashes we remember to avoid duplicates.
   This may be bigger because we remember hashes of things the
   guy threw away, so as not to annoy him too much. */
#define AVOID_LIST_MAX 100

typedef struct {
   configuration stuph[MAX_RESOLVE_SIZE];
   int size;
   int insertion_point;
   int permute1[8];
   int permute2[8];
   int rotchange;
} resolve_rec;

Private configuration *huge_history_save = (configuration *) 0;
Private int huge_history_allocation = 0;
Private int huge_history_ptr;

Private resolve_rec *all_resolves = (resolve_rec *) 0;
Private int resolve_allocation = 0;

Private int avoid_list[AVOID_LIST_MAX];
Private int avoid_list_size;
Private int perm_array[8];
Private setup_kind goal_kind;
Private int goal_directions[8];

Private char *title_string[] = {
   "Anything: ",
   "Nice setup: ",
   "Resolve: ",
   "Reconcile: "};

static void display_reconcile_history(int current_depth, int n);



typedef struct {
   int d35, d32, d13, d71, d10, d76;
   dance_level level_needed;
   resolve_kind k;
   int distance;
} resolve_tester;

Private resolve_tester test_thar_rlg    = {0173, 0702, 0207, 0167, 0702, 0676, l_mainstream,      resolve_rlg, 2};              /* RLG from thar. */
Private resolve_tester test_thar_la     = {0167, 0076, 0207, 0173, 0076, 0102, l_mainstream,      resolve_la, 5};               /* LA from thar. */
Private resolve_tester test_thar_xbyla  = {0173, 0102, 0207, 0167, 0102, 0076, cross_by_level,    resolve_xby_la, 5};           /* cross-by-LA from thar. */
Private resolve_tester test_thar_xbyrlg = {0167, 0676, 0207, 0173, 0676, 0702, cross_by_level,    resolve_xby_rlg, 2};          /* cross-by-RLG from thar. */
Private resolve_tester test_thar_slc_rg = {0173, 0102, 0207, 0167, 0102, 0076, l_mainstream,      resolve_slipclutch_rlg, 1};   /* slip-the-clutch-RLG from thar. */
Private resolve_tester test_thar_scl_la = {0167, 0676, 0207, 0173, 0676, 0702, l_mainstream,      resolve_slipclutch_la, 6};    /* slip-the-clutch-LA from thar. */
Private resolve_tester test_thar_pr     = {0173, 0700, 0207, 0167, 0700, 0700, l_mainstream,      resolve_prom, 6};             /* promenade from thar. */
Private resolve_tester test_2x4_home    = {0173, 0671, 0207, 0167, 0711, 0671, l_mainstream,      resolve_at_home, 7};          /* "at home" from pseudo-squared-set. */
Private resolve_tester test_wv_la       = {0076, 0676, 0300, 0102, 0076, 0702, l_mainstream,      resolve_la, 6};               /* LA from waves. */
Private resolve_tester test_wv_extla    = {0276, 0076, 0100, 0302, 0676, 0102, l_mainstream,      resolve_ext_la, 7};           /* ext-LA from waves. */
Private resolve_tester test_wv_crcla    = {0476, 0276, 0700, 0502, 0476, 0302, l_mainstream,      resolve_circ_la, 0};          /* circulate-LA from waves. */
Private resolve_tester test_2x4_xby_la  = {0102, 0702, 0300, 0076, 0102, 0676, cross_by_level,    resolve_xby_la, 6};           /* cross-by-LA from waves. */
Private resolve_tester test_wv_rg       = {0302, 0102, 0100, 0276, 0702, 0076, l_mainstream,      resolve_rlg, 3};              /* RLG from waves. */
Private resolve_tester test_wv_extrg    = {0102, 0702, 0300, 0076, 0102, 0676, l_mainstream,      resolve_ext_rlg, 2};          /* extend-RLG from waves. */
Private resolve_tester test_wv_crcrg    = {0702, 0502, 0500, 0676, 0302, 0476, l_mainstream,      resolve_circ_rlg, 1};         /* circulate-RLG from waves. */
Private resolve_tester test_wv_xbyrg    = {0276, 0076, 0100, 0302, 0676, 0102, cross_by_level,    resolve_xby_rlg, 3};          /* cross-by-RLG from waves. */
Private resolve_tester test_2fl_prom    = {0300, 0100, 0102, 0300, 0700, 0100, l_mainstream,      resolve_prom, 7};             /* promenade from 2FL. */
Private resolve_tester test_8ch_rg      = {0202, 0702, 0200, 0176, 0702, 0676, l_mainstream,      resolve_rlg, 3};              /* RLG from 8-chain. */
Private resolve_tester test_8ch_pthrg   = {0202, 0102, 0200, 0176, 0102, 0076, l_mainstream,      resolve_pth_rlg, 2};          /* pass-thru-RLG from 8-chain. */
Private resolve_tester test_tby_rg      = {0176, 0676, 0200, 0202, 0676, 0702, l_mainstream,      resolve_rlg, 3};              /* RLG from trade-by. */
Private resolve_tester test_tby_tbyrg   = {0176, 0476, 0200, 0202, 0476, 0502, l_mainstream,      resolve_tby_rlg, 4};          /* trade-by-RLG from trade-by. */
Private resolve_tester test_tbone_rg    = {0207, 0707, 0167, 0207, 0667, 0707, l_mainstream,      resolve_rlg, 3};              /* RLG from T-bone setup, ends facing. */
Private resolve_tester test_8ch_la      = {0202, 0702, 0200, 0176, 0702, 0676, l_mainstream,      resolve_la, 7};               /* LA from 8-chain. */
Private resolve_tester test_8ch_pthla   = {0202, 0102, 0200, 0176, 0102, 0076, l_mainstream,      resolve_pth_la, 6};           /* pass-thru-LA from 8-chain. */
Private resolve_tester test_tby_la      = {0176, 0676, 0200, 0202, 0676, 0702, l_mainstream,      resolve_la, 7};               /* LA from trade-by. */
Private resolve_tester test_tby_tbyla   = {0176, 0476, 0200, 0202, 0476, 0502, l_mainstream,      resolve_tby_la, 0};           /* trade-by-LA from trade-by. */
Private resolve_tester test_tbone_la    = {0207, 0707, 0167, 0207, 0667, 0707, l_mainstream,      resolve_la, 7};               /* LA from T-bone setup, ends facing. */
Private resolve_tester test_dpt_dix     = {0700, 0600, 0476, 0700, 0600, 0600, dixie_grand_level, resolve_dixie_grand, 2};      /* dixie grand from DPT. */
Private resolve_tester test_qtag_dix    = {0700, 0476, 0502, 0700, 0300, 0502, dixie_grand_level, resolve_dixie_grand, 2};      /* dixie grand from 1/4 tag. */
Private resolve_tester test_3tag_rg     = {0176, 0676, 0200, 0202, 0700, 0702, l_mainstream,      resolve_rlg, 4};              /* RLG from 3/4 tag. */
Private resolve_tester test_dmd_rg      = {0207, 0676, 0173, 0207, 0702, 0702, l_mainstream,      resolve_rlg, 4};              /* RLG from diamonds with points facing each other. */
Private resolve_tester test_3tag_la     = {0300, 0102, 0076, 0300, 0700, 0076, l_mainstream,      resolve_la, 0};               /* LA from 3/4 tag. */
Private resolve_tester test_dmd_la      = {0311, 0102, 0071, 0305, 0702, 0076, l_mainstream,      resolve_la, 0};               /* LA from diamonds with points facing each other. */



extern resolve_indicator resolve_p(setup *s)
{
   resolve_indicator k;
   resolve_tester *testptr;

   switch (s->kind) {
      case s_qtag:
         switch (s->people[5].id1 & 0177) {
            case 0010:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0300: testptr = &test_qtag_dix; goto check_me;
               }
               break;
            case 0012:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0700: testptr = &test_3tag_rg; goto check_me;
               }
               break;
            case 0112:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0700: testptr = &test_3tag_la; goto check_me;
               }
               break;
            case 0001:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0676: testptr = &test_dmd_rg; goto check_me;
               }
               break;
            case 0101:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0676: testptr = &test_dmd_la; goto check_me;
               }
               break;
         }
         break;
      case s2x4:
         switch (s->people[5].id1 & 0177) {
            case 0010:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0676: testptr = &test_wv_rg; goto check_me;
                  case 0076: testptr = &test_wv_extrg; goto check_me;
                  case 0276: testptr = &test_wv_crcrg; goto check_me;
                  case 0700: testptr = &test_2fl_prom; goto check_me;
               }
               break;
            case 0012:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0102: testptr = &test_wv_la; goto check_me;
                  case 0702: testptr = &test_wv_extla; goto check_me;
                  case 0502: testptr = &test_wv_crcla; goto check_me;
               }
               break;
            case 0001:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0676: testptr = &test_8ch_rg; goto check_me;
                  case 0076: testptr = &test_8ch_pthrg; goto check_me;
               }
               break;
            case 0003:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0702: testptr = &test_tby_la; goto check_me;
                  case 0673: testptr = &test_tbone_la; goto check_me;
                  case 0502: testptr = &test_tby_tbyla; goto check_me;
                  case 0600: testptr = &test_dpt_dix; goto check_me;
               }
               break;
            case 0112:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0702: testptr = &test_wv_xbyrg; goto check_me;
               }
               break;
            case 0110:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0076: testptr = &test_2x4_xby_la; goto check_me;
                  case 0705: testptr = &test_2x4_home; goto check_me;
               }
               break;
            case 0101:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0676: testptr = &test_8ch_la; goto check_me;
                  case 0076: testptr = &test_8ch_pthla; goto check_me;
               }
               break;
            case 0103:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0702: testptr = &test_tby_rg; goto check_me;
                  case 0673: testptr = &test_tbone_rg; goto check_me;
                  case 0502: testptr = &test_tby_tbyrg; goto check_me;
               }
               break;
         }
         break;
      case s_crosswave: case s_thar:
         /* This makes use of the fact that the person numbering
            in crossed lines and thars is identical. */

         switch (s->people[5].id1 & 0177) {
            case 0010:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0700: testptr = &test_thar_pr; goto check_me;
                  case 0676: testptr = &test_thar_rlg; goto check_me;
                  case 0076: testptr = &test_thar_slc_rg; goto check_me;
               }
               break;
            case 0012:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0102: testptr = &test_thar_la; goto check_me;
                  case 0702: testptr = &test_thar_scl_la; goto check_me;
               }
               break;
            case 0112:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0702: testptr = &test_thar_xbyrlg; goto check_me;
               }
               break;
            case 0110:
               switch ((s->people[5].id1 - s->people[4].id1) & 0777) {
                  case 0076: testptr = &test_thar_xbyla; goto check_me;
               }
               break;
         }
         break;
   }

   k.kind = resolve_none;
   k.distance = 0;    /* To get around warnings from buggy and confused compilers. */
   return(k);

   check_me:

   if (  ((s->people[3].id1 - s->people[5].id1) & 0777) == testptr->d35 &&
         ((s->people[3].id1 - s->people[2].id1) & 0777) == testptr->d32 &&
         ((s->people[1].id1 - s->people[3].id1) & 0777) == testptr->d13 &&
         ((s->people[7].id1 - s->people[1].id1) & 0777) == testptr->d71 &&
         ((s->people[1].id1 - s->people[0].id1) & 0777) == testptr->d10 &&
         ((s->people[7].id1 - s->people[6].id1) & 0777) == testptr->d76 &&
         calling_level >= testptr->level_needed) {
      k.kind = testptr->k;
      k.distance = ((s->rotation << 1) + (s->people[5].id1 >> 6) + testptr->distance) & 7;

      /* Disallow an "at home" resolve unless promenade distance is zero. */
      if (k.kind != resolve_at_home || k.distance == 0)
         return(k);
   }

   /* Too bad. */

   k.kind = resolve_none;
   k.distance = 0;    /* To get around warnings from buggy and confused compilers. */
   return(k);
}


/* These variables are actually local to inner_search, but they are
   expected to be preserved across the longjmp, so they must be static. */

Private int perm_indices[8];
Private int attempt_count, little_count;
Private int hashed_random_list[5];
Private parse_block *inner_parse_mark, *outer_parse_mark;
Private int history_insertion_point;    /* Where our resolve should lie in the history.
                                          This is normally the end of the history, but
                                          may be earlier if doing a reconcile.  We clobber
                                          everything in the given history past this point. */
Private int history_save;               /* Where we are inserting calls now.  This moves
                                          forward as we build multiple-call resolves. */
       

Private long_boolean inner_search(search_kind goal, resolve_rec *new_resolve, long_boolean accept_extend, int insertion_point)
{
   long_boolean retval;
   int i, j;
   setup *ns;
   real_jmp_buf my_longjmp_buffer;

   history_insertion_point = huge_history_ptr;

   if (goal == search_reconcile) {
      history_insertion_point -= insertion_point;    /* strip away the extra calls */

      goal_kind = history[history_insertion_point].state.kind;
      if (setup_limits[goal_kind] != 7) return(FALSE);
      for (j=0; j<8; j++) goal_directions[j] = history[history_insertion_point].state.people[j].id1 & d_mask;

      for (j=0; j<8; j++) {
         perm_indices[j] = -1;
         for (i=0; i<8; i++)
            if ((history[history_insertion_point].state.people[i].id1 & 0700) == perm_array[j]) perm_indices[j] = i;
         if (perm_indices[j] < 0) return(FALSE);      /* didn't find the person???? */
      }
   }

   history_save = history_insertion_point;

   little_count = 0;
   attempt_count = 0;
   hashed_random_list[0] = 0;
   not_interactive = TRUE;
   /* Mark the parse block allocation, so that we throw away the garbage
      created by failing attempts. */
   inner_parse_mark = outer_parse_mark = mark_parse_blocks();
   
   /* Create a special error handler.  Any time a call fails, we will get back here. */
   
   longjmp_ptr = &my_longjmp_buffer;          /* point the global pointer at it. */
   setjmp(my_longjmp_buffer.the_buf);
   
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

   /* Throw away garbage from last attempt. */
   release_parse_blocks_to_mark(inner_parse_mark);
   testing_fidelity = FALSE;
   history_ptr = history_save;
   attempt_count++;
   if (attempt_count > 5000) {
      /* Too many tries -- too bad. */
      history_ptr = huge_history_ptr;
      retval = FALSE;
      goto timeout;
   }

   /* Now clear any concepts if we are not on the first call of the series. */
   
   if (history_ptr != history_insertion_point || goal == search_reconcile)
      initialize_parse();
   else
      (void) restore_parse_state();
   
   /* Generate the concepts and call. */
   
   hashed_randoms = hashed_random_list[history_ptr - history_insertion_point];

   /* Put in a special initial concept if needed to make a nice setup. */

   switch (goal) {
      case search_nice_setup:
         i = generate_random_number(4);
         /* The table "nice_setup_concept" is defined in the interface file,
            containing the 4 concept indices that we want. */
         deposit_concept(&concept_descriptor_table[nice_setup_concept[i]]);
         break;
   }
   
   /* Select the call.  Selecting one that says "don't use in resolve" will signal and go to try_again. */
   /* This may, of course, add more concepts. */
   
   history[history_ptr+1].warnings.bits[0] = 0;
   history[history_ptr+1].warnings.bits[1] = 0;
   (void) query_for_call();
   
   /* Do the call.  An error will signal and go to try_again. */
   
   toplevelmove();

   if (history[history_ptr+1].warnings.bits[0] & Warnings_That_Preclude_Searching) goto try_again;      /* We don't like certain warnings either. */
   
   /* See if we have already seen this sequence. */
   
   for (i=0; i<avoid_list_size; i++)
      if (hashed_randoms == avoid_list[i]) goto try_again;
   
   /* The call was legal, see if it satisfies our criterion. */
   
   /* ***** Because of an apparent bug in the C compiler, the "switch" statement that we would
      like to have used doesn't work, so we use an "if". */

   ns = &history[history_ptr+1].state;

   if (goal == search_resolve) {
      /* Here we bias the search against resolves with circulates (which we
         consider to be of lower quality) by only sometimes accepting them. */
      if (history[history_ptr+1].resolve_flag.kind == resolve_none ||
         ((attempt_count & 1)
            && history[history_ptr+1].resolve_flag.kind != resolve_rlg
            && history[history_ptr+1].resolve_flag.kind != resolve_ext_rlg
            && history[history_ptr+1].resolve_flag.kind != resolve_la
            && history[history_ptr+1].resolve_flag.kind != resolve_xby_la
            && history[history_ptr+1].resolve_flag.kind != resolve_dixie_grand
            && history[history_ptr+1].resolve_flag.kind != resolve_prom))
         goto what_a_loss;
   }
   else if (goal == search_nice_setup) {
      if (ns->kind != s2x4) goto what_a_loss;
   }
   else if (goal == search_reconcile) {
      if (ns->kind != goal_kind) goto what_a_loss;
      for (j=0; j<8; j++)
         if ((ns->people[j].id1 & d_mask) != goal_directions[j]) goto what_a_loss;

         {        /* Need some local temporaries -- ugly in C, impossible in Pascal! */
         int p0 = ns->people[perm_indices[0]].id1 & 0700;
         int p1 = ns->people[perm_indices[1]].id1 & 0700;
         int p2 = ns->people[perm_indices[2]].id1 & 0700;
         int p3 = ns->people[perm_indices[3]].id1 & 0700;
         int p4 = ns->people[perm_indices[4]].id1 & 0700;
         int p5 = ns->people[perm_indices[5]].id1 & 0700;
         int p6 = ns->people[perm_indices[6]].id1 & 0700;
         int p7 = ns->people[perm_indices[7]].id1 & 0700;

         /* This person must be a boy. */
         if (p0 & 0100) goto what_a_loss;

         p7 = (p7 - p6) & 0700;
         p6 = (p6 - p5) & 0700;
         p5 = (p5 - p4) & 0700;
         p4 = (p4 - p3) & 0700;
         p3 = (p3 - p2) & 0700;
         p2 = (p2 - p1) & 0700;
         p1 = (p1 - p0) & 0700;

         /* By checking that the other 7 people are all in ascending sequence from the first, we know that they
            are alternating boys and girls, with partners, in ascending couple number. */
         if (  (p1 == 0100) && (p2 == 0100) &&
               (p3 == 0100) && (p4 == 0100) &&
               (p5 == 0100) && (p6 == 0100) &&
               (p7 == 0100))
            ;
         else if (accept_extend &&           /* check for off by 1 ==> extend */
               (p1 == 0700) && (p2 == 0300) &&
               (p3 == 0700) && (p4 == 0300) &&
               (p5 == 0700) && (p6 == 0300) &&
               (p7 == 0700))
            ;
         else if (accept_extend &&           /* check for off by 2 ==> circulate */
               (p1 == 0500) && (p2 == 0500) &&
               (p3 == 0500) && (p4 == 0500) &&
               (p5 == 0500) && (p6 == 0500) &&
               (p7 == 0500))
            ;
         else
            goto what_a_loss;
         }
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

   if (goal == search_reconcile) {
      for (j=0; j<8; j++) {
         new_resolve->permute1[perm_array[j] >> 6] = ns->people[perm_indices[j]].id1 & ID_BITS_1;
         new_resolve->permute2[perm_array[j] >> 6] = ns->people[perm_indices[j]].id2 & ID_BITS_2;
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
      history[history_ptr+1].warnings.bits[0] = 0;
      history[history_ptr+1].warnings.bits[1] = 0;
      toplevelmove();

      this_state = history[history_ptr+1];
      this_state.state.rotation -= new_resolve->rotchange;
      canonicalize_rotation(&this_state.state);

      if (this_state.state.rotation != huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].state.rotation)
         goto try_again;
      if (this_state.warnings.bits[0] != huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].warnings.bits[0])
         goto try_again;
      if (this_state.warnings.bits[1] != huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].warnings.bits[1])
         goto try_again;

      for (k=0; k<=setup_limits[this_state.state.kind]; k++) {
         personrec t = huge_history_save[j+huge_history_ptr+1-new_resolve->insertion_point].state.people[k];

         if (t.id1) {
            if (this_state.state.people[k].id1 !=
                  ((t.id1 & (~ID_BITS_1)) | new_resolve->permute1[(t.id1 & 0700) >> 6]))
               goto try_again;
            if (this_state.state.people[k].id2 !=
                  ((t.id2 & (~ID_BITS_2)) | new_resolve->permute2[(t.id1 & 0700) >> 6]))
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
      But for "at home", resolve_p will only show a resolve if the distance is zero. */

   if (goal == search_reconcile && history[history_ptr].resolve_flag.kind == resolve_none)
      goto try_again;   /* Sorry. */

   /* We win.  Really save it and exit.  History_ptr has been clobbered. */

   for (j=0; j<MAX_RESOLVE_SIZE; j++)
      new_resolve->stuph[j] = history[j+history_insertion_point+1];
   
   if (avoid_list_size < AVOID_LIST_MAX)
      avoid_list[avoid_list_size++] = hashed_randoms;

   retval = TRUE;
   goto timeout;
   
   what_a_loss:
   
   if (++little_count == 60) {
      /* Revert back to beginning. */
      history_save = history_insertion_point;
      inner_parse_mark = outer_parse_mark;
      little_count = 0;
   }
   else if (little_count == 20 || little_count == 40) {
      /* Save current state as a base for future calls. */
      history_save = history_ptr + 1;
      inner_parse_mark = mark_parse_blocks();
      hashed_random_list[history_save - history_insertion_point] = hashed_randoms;
   }

   goto try_again;
   
   timeout:

   /* Restore the global error handler. */

   longjmp_ptr = &longjmp_buffer;
   not_interactive = FALSE;
   return(retval);
}


extern uims_reply full_resolve(search_kind goal)
{
   int j, k;
   uims_reply reply;
   int current_resolve_index, max_resolve_index;
   long_boolean show_resolve;
   personrec *current_people = history[history_ptr].state.people;
   long_boolean accept_extend = FALSE;
   int current_depth = 0;
   long_boolean find_another_resolve = TRUE;
   int *perm_map;
   resolver_display_state state; /* for display to the user */

   /* Allocate or reallocate the huge_history_save save array if needed. */

   if (huge_history_allocation < history_ptr+MAX_RESOLVE_SIZE+2) {
      configuration *t;
      huge_history_allocation = (history_ptr+MAX_RESOLVE_SIZE+2) << 1;   /* Twice what we actually need now. */
      t = (configuration *) get_more_mem_gracefully(huge_history_save, huge_history_allocation * sizeof(configuration));
      if (!t) specialfail("Not enough memory!");
      huge_history_save = t;
   }

   /* Do the resolve array. */

   if (all_resolves == 0) {
      resolve_allocation = 10;
      all_resolves = (resolve_rec *) get_mem_gracefully(resolve_allocation * sizeof(resolve_rec));
      if (!all_resolves) specialfail("Not enough memory!");
   }

   /* Be sure the extra 5 slots in the history array are clean. */

   for (j=0; j<MAX_RESOLVE_SIZE; j++) {
      history[history_ptr+j+2].command_root = (parse_block *) 0;
      history[history_ptr+j+2].centersp = 0;
   }

   /* See if we are in a reasonable position to do the search. */

   switch (goal) {
      case search_resolve:
         if (!resolve_command_ok())
            specialfail("Not in acceptable setup for resolve.");
         break;
      case search_reconcile:
         if (!reconcile_command_ok(&perm_map, &accept_extend))
            specialfail("Not in acceptable setup for reconcile, or sequence is too short, or concepts are selected.");

         for (j=0; j<8; j++)
            perm_array[j] = current_people[perm_map[j]].id1 & 0700;

         current_depth = 1;
         find_another_resolve = FALSE;       /* We initially don't look for resolves; we wait for the user
                                                to set the depth. */
         break;
      case search_anything:
         break;
      case search_nice_setup:
         if (!nice_setup_command_ok())
            specialfail("Sorry, can only do this in 4x4 setup with no concepts selected.");
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

   uims_begin_search(goal);
   if (goal == search_reconcile)
      display_reconcile_history(current_depth, huge_history_ptr);

   for (;;) {
      /* We know the history is restored at this point. */
      if (find_another_resolve) {
         /* Put up the resolve title showing that we are searching. */
         uims_update_resolve_menu(goal, current_resolve_index, max_resolve_index, resolver_display_searching);

         (void) restore_parse_state();
   
         if (inner_search(goal, &all_resolves[max_resolve_index], accept_extend, current_depth)) {
            /* Search succeeded, save it. */
            max_resolve_index++;
            /* Make it the current one. */
            current_resolve_index = max_resolve_index;

            /* Put up the resolve title showing this resolve,
               but without saying "searching". */
            state = resolver_display_ok;      
         }
         else {
            /* Display the sequence with the current resolve inserted. */
            /* Put up a resolve title indicating failure. */
            state = resolver_display_failed;
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
         state = resolver_display_ok;
      }

      uims_update_resolve_menu(goal, current_resolve_index, max_resolve_index, state);

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

            for (k=0; k<=setup_limits[this_state->state.kind]; k++) {
               personrec t = this_state->state.people[k];

               if (t.id1) {
                  this_state->state.people[k].id1 = 
                     (t.id1 & (~ID_BITS_1)) | this_resolve->permute1[(t.id1 & 0700) >> 6];
                  this_state->state.people[k].id2 = 
                     (t.id2 & (~ID_BITS_2)) | this_resolve->permute2[(t.id1 & 0700) >> 6];
               }
            }
            
            this_state->resolve_flag = resolve_p(&this_state->state);
         }

         history_ptr = huge_history_ptr + this_resolve->size;

         /* Show the history up to the start of the resolve, forcing a picture on the last item. */

         display_initial_history(huge_history_ptr-this_resolve->insertion_point, 1);

         /* Show the resolve itself, without its last item. */

         for (j=huge_history_ptr-this_resolve->insertion_point+1; j<history_ptr-this_resolve->insertion_point; j++)
            write_history_line(j, (char *) 0, FALSE, file_write_no);

         /* Show the last item of the resolve, with a forced picture. */
         write_history_line(history_ptr-this_resolve->insertion_point, (char *) 0, TRUE, file_write_no);

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
         newline();
         writestuff("     resolve is:");
         newline();
         writestuff(resolve_names[history[history_ptr].resolve_flag.kind]);
         if (history[history_ptr].resolve_flag.kind != resolve_at_home ||
               (history[history_ptr].resolve_flag.distance & 7) != 0)
            writestuff(resolve_distances[history[history_ptr].resolve_flag.distance & 7]);
         newline();
      }

      show_resolve = TRUE;

      for (;;) {          /* We ignore any "undo" clicks. */
         reply = uims_get_command(mode_resolve, call_list_any, FALSE);
         if ((reply != ui_command_select) || (uims_menu_index != command_undo)) break;
      }
   
      if (reply == ui_resolve_select) {
         switch ((resolve_command_kind) uims_menu_index) {
            case resolve_command_find_another:
               if (resolve_allocation <= max_resolve_index) {   /* Increase allocation if necessary. */
                  resolve_rec *t;
                  resolve_allocation <<= 1;
                  t = (resolve_rec *) get_more_mem_gracefully(all_resolves, resolve_allocation * sizeof(resolve_rec));
                  if (!t) break;   /* By not turning on "find_another_resolve", we will take no action. */
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

               return(reply);
            default:
               /* Clicked on "accept choice", or something not on this submenu. */
               return(reply);
         }
      }
      else if ((reply == ui_command_select) && (uims_menu_index == command_refresh)) {
         /* fall through to redisplay */
         ;
      }
      else {
         /* Clicked on "accept choice", or something not on this submenu. */
         return(reply);
      }

      /* Restore history for next cycle. */
      written_history_items = -1;
      history_ptr = huge_history_ptr;

      for (j=0; j<=history_ptr+1; j++)
         history[j] = huge_history_save[j];
   }
}

static void display_reconcile_history(int current_depth, int n)
{
   int j;

   /*
    * The UI might display the reconcile history in a different window
    * than the normal sequence.  In that case, our remembered history
    * must be discarded.
    */

   if (uims_begin_reconcile_history(current_depth, n-2))
       written_history_items = -1;
   display_initial_history(n-current_depth, 0);
   if (current_depth > 0) {
      writestuff("------------------------------------");
      newline();
      for (j=n-current_depth+1; j<=n; j++)
         write_history_line(j, (char *) 0, FALSE, file_write_no);
   }
   if (uims_end_reconcile_history())
       written_history_items = -1;
}

extern int concepts_in_place(void)
{
   return history[history_ptr+1].command_root != 0;
}


Private int promperm[8] = {1, 0, 6, 7, 5, 4, 2, 3};
Private int qtagperm[8] = {1, 0, 7, 6, 5, 4, 3, 2};
Private int homeperm[8] = {6, 5, 4, 3, 2, 1, 0, 7};
Private int crossperm[8] = {5, 4, 3, 2, 1, 0, 7, 6};
Private int laperm[8] = {1, 3, 6, 0, 5, 7, 2, 4};

/* If this returns TRUE, it drops useful stuff into the places that its arguments
   point to.  If you don't want that, point the arguments at dummies. */
extern int reconcile_command_ok(int **permutation_map_p, int *accept_extend_p)
{
   int k;
   int dirmask = 0;
   personrec *current_people = history[history_ptr].state.people;
   setup_kind current_kind = history[history_ptr].state.kind;

   /* Since we are going to go back 1 call, demand we have at least 3. ***** */
   /* Also, demand no concepts already in place. */
   if ((history_ptr < 3) || concepts_in_place()) return FALSE;

   for (k=0; k<8; k++)
      dirmask = (dirmask << 2) | (current_people[k].id1 & 3);

   *accept_extend_p = FALSE;

   if (current_kind == s2x4 && dirmask == 0xA00A)
      *permutation_map_p = promperm;            /* L2FL, looking for promenade. */
   else if (current_kind == s_qtag && dirmask == 0x08A2)
      *permutation_map_p = qtagperm;            /* RQTAG, looking for RLG. */
   else if (current_kind == s2x4 && dirmask == 0x6BC1)
      *permutation_map_p = homeperm;            /* pseudo-squared-set, looking for at home. */
   else if (current_kind == s_qtag && dirmask == 0x78D2)
      *permutation_map_p = qtagperm;            /* diamonds with points facing, looking for RLG. */
   else if ((current_kind == s_crosswave || current_kind == s_thar) && (dirmask == 0x278D || dirmask == 0xAF05))
      *permutation_map_p = crossperm;            /* crossed waves or thar, looking for RLG or promenade. */
   else if (current_kind == s2x4 && dirmask == 0x2288) {
      /* Rwave, looking for RLG, we turn on "accept_extend" to tell it
         to measure couple number only approximately. */
      *accept_extend_p = TRUE;
      *permutation_map_p = promperm;
   }
   else if (current_kind == s2x4 && dirmask == 0x8822) {
      /* Lwave, looking for LA, we turn on "accept_extend" to tell it
         to measure couple number only approximately. */
      *accept_extend_p = TRUE;
      *permutation_map_p = laperm;
   }
   else
      return FALSE;

   return TRUE;
}

extern int resolve_command_ok(void)
{
   setup_kind current_kind = history[history_ptr].state.kind;
   return current_kind == s2x4 ||
          current_kind == s1x8 ||
          current_kind == s_qtag;
}

extern int nice_setup_command_ok(void)
{
   setup_kind current_kind = history[history_ptr].state.kind;
   return current_kind == s4x4 && !concepts_in_place();
}

/*
 * Create a string representing the search state.  Goal indicates which user command
 * is being performed.  If there is no current solution,
 * then M and N are both 0.  If there is a current
 * solution, the M is the solution index (minimum value 1) and N is the maximum
 * solution index (N>0).  State indicates whether a search is in progress or not, and
 * if not, whether the most recent search failed.
 */

extern void create_resolve_menu_title(search_kind goal, int cur, int max, resolver_display_state state, char *title)
{
   char junk[MAX_TEXT_LINE_LENGTH];
   char *titleptr;

   titleptr = title;
   string_copy(&titleptr, title_string[goal]);
   if (max > 0) {
      add_resolve_indices(junk, cur, max);
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
