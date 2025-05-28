/* SD -- square dance caller's helper.

    Copyright (C) 1990-2003  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following functions:
   get_multiple_parallel_resultflags
   initialize_sel_tables
   initialize_fix_tables
   conc_tables::initialize
   conc_tables::analyze_this
   conc_tables::synthesize_this
   normalize_concentric
   concentric_move
   merge_table::map_tgl4l
   merge_table::map_tgl4b
   merge_table::map_2234b
   merge_table::initialize
   merge_table::lookup
   merge_setups
   on_your_own_move
   punt_centers_use_concept
   selective_move
   inner_selective_move
*/

#include "sd.h"


#define CONTROVERSIAL_CONC_ELONG 0x200

extern resultflag_rec get_multiple_parallel_resultflags(setup outer_inners[], int number) THROW_DECL
{
   int i;
   resultflag_rec result_flags;
   result_flags.clear_split_info();
   result_flags.misc = 0;

   bool clear_split_fields = false;   // In case we have to clear both fields.

   /* If a call was being done "piecewise" or "random", we demand that both
      calls run out of parts at the same time, and, when that happens, we
      report it to the higher level in the recursion. */

   for (i=0 ; i<number ; i++) {
      if (!(outer_inners[i].result_flags.misc & RESULTFLAG__PARTS_ARE_KNOWN))
         outer_inners[i].result_flags.misc &= ~(RESULTFLAG__DID_LAST_PART|RESULTFLAG__SECONDARY_DONE);

      if (((outer_inners[i].result_flags.misc & outer_inners[0].result_flags.misc) & RESULTFLAG__PARTS_ARE_KNOWN) &&
            ((outer_inners[i].result_flags.misc ^ outer_inners[0].result_flags.misc) & (RESULTFLAG__DID_LAST_PART|RESULTFLAG__SECONDARY_DONE)))
         fail("Two calls must use the same number of fractions.");

      result_flags.misc |= outer_inners[i].result_flags.misc;

      if (i == 0)
         result_flags.copy_split_info(outer_inners[i].result_flags);
      else {
         // For each field (X or Y), if both setups have nonzero values but
         // those values don't match, that field gets cleared.
         if (result_flags.split_info[0] != outer_inners[i].result_flags.split_info[0] &&
             result_flags.split_info[0] != 0 &&
             outer_inners[i].result_flags.split_info[0] != 0)
            clear_split_fields = true;
         if (result_flags.split_info[1] != outer_inners[i].result_flags.split_info[1] &&
             result_flags.split_info[1] != 0 &&
             outer_inners[i].result_flags.split_info[1] != 0)
            clear_split_fields = true;

         result_flags.split_info[0] |= outer_inners[i].result_flags.split_info[0];
         result_flags.split_info[1] |= outer_inners[i].result_flags.split_info[1];
      }
   }

   if (clear_split_fields) result_flags.clear_split_info();

   return result_flags;
}


select::sel_item *select::sel_hash_table[select::NUM_SEL_HASH_BUCKETS];
select::fixer *select::fixer_ptr_table[fx_ENUMLAST];

void select::initialize()
{
   sel_item *selp;
   int i;

   for (i=0 ; i<NUM_SEL_HASH_BUCKETS ; i++) sel_hash_table[i] = (sel_item *) 0;

   for (selp = sel_init_table ; selp->kk != nothing ; selp++) {
      uint32 hash_num = ((selp->thislivemask + (5*selp->kk)) * 25) & (NUM_SEL_HASH_BUCKETS-1);
      selp->next = sel_hash_table[hash_num];
      sel_hash_table[hash_num] = selp;
   }

   fixer *fixp;

   for (i=fx0 ; i<fx_ENUMLAST ; i++) fixer_ptr_table[i] = (fixer *) 0;

   for (fixp = fixer_init_table ; fixp->mykey != fx0 ; fixp++) {
      if (fixer_ptr_table[fixp->mykey])
         gg->fatal_error_exit(1, "Fixer table initialization failed", "dup");
      fixer_ptr_table[fixp->mykey] = fixp;
   }

   for (i=fx0+1 ; i<fx_ENUMLAST ; i++) {
      if (!fixer_ptr_table[i])
         gg->fatal_error_exit(1, "Fixer table initialization failed", "undef");
   }
}


const select::fixer *select::hash_lookup(setup_kind kk, uint32 thislivemask, uint32 key, uint32 arg, const setup *ss)
{
   uint32 hash_num = ((thislivemask + (5*kk)) * 25) & (NUM_SEL_HASH_BUCKETS-1);
   const sel_item *p;
   const fixer *fixp = (const fixer *) 0;

   for (p = sel_hash_table[hash_num] ; p ; p = p->next) {
      if ((p->key & key) && p->kk == kk && p->thislivemask == thislivemask) {
         fixp = fixer_ptr_table[p->fixp];

         // We make an extremely trivial test here to see which way the distortion goes.
         // It will be checked thoroughly later.

         if (p->use_fixp2 >= 0 && ((ss->people[p->use_fixp2].id1 ^ arg) & 1))
            fixp = fixer_ptr_table[p->fixp2];

         return fixp;
      }
   }

   return (const fixer *) 0;
}



conc_tables::cm_thing *conc_tables::conc_hash_synthesize_table[conc_tables::NUM_CONC_HASH_BUCKETS];
conc_tables::cm_thing *conc_tables::conc_hash_analyze_table[conc_tables::NUM_CONC_HASH_BUCKETS];


void conc_tables::initialize()
{
   cm_thing *tabp;
   int i;

   for (i=0 ; i<NUM_CONC_HASH_BUCKETS ; i++) conc_hash_synthesize_table[i] = (cm_thing *) 0;
   for (i=0 ; i<NUM_CONC_HASH_BUCKETS ; i++) conc_hash_analyze_table[i] = (cm_thing *) 0;

   for (tabp = conc_init_table ; tabp->bigsetup != nothing ; tabp++) {

      // For synthesize.

      if ((tabp->elongrotallow & 0x100) == 0) {
         uint32 hash_num = ((tabp->outsetup + (5*(tabp->insetup + 5*tabp->getout_schema))) * 25) & (NUM_CONC_HASH_BUCKETS-1);
         tabp->next_synthesize = conc_hash_synthesize_table[hash_num];
         conc_hash_synthesize_table[hash_num] = tabp;
      }

      // For analyze.

      if ((tabp->elongrotallow & 0x200) == 0) {
         uint32 hash_num = ((tabp->bigsetup + (5*tabp->lyzer)) * 25) & (NUM_CONC_HASH_BUCKETS-1);

         tabp->next_analyze = conc_hash_analyze_table[hash_num];
         conc_hash_analyze_table[hash_num] = tabp;
      }

      tabp->used_mask_analyze = 0;
      tabp->used_mask_synthesize = 0;

      int mapsize = (attr::klimit(tabp->insetup)+1)*tabp->center_arity +
         (attr::klimit(tabp->outsetup)+1);

      for (i=0 ; i<mapsize ; i++) {
         if (tabp->maps[i] >= 0) {
            tabp->used_mask_analyze |= 1 << tabp->maps[i];
            tabp->used_mask_synthesize |= 1 << i;
         }
      }
   }
}


static void fix_missing_centers(
   setup *inners,
   setup *outers,
   setup_kind kki,
   setup_kind kko,
   int center_arity,
   bool enforce_kk) THROW_DECL
{
   int i;

   /* Fix up nonexistent centers, in a rather inept way. */

   for (i=0 ; i<center_arity ; i++) {
      if (inners[i].kind == nothing) {
         inners[i].kind = kki;
         inners[i].rotation = 0;
         inners[i].result_flags = outers->result_flags;
         clear_people(&inners[i]);
      }
      else if (inners[i].kind != kki && enforce_kk)
         fail("Don't recognize concentric ending setup.");
   }

   if (outers->kind != kko && enforce_kk)
      fail("Don't recognize concentric ending setup.");
}


bool conc_tables::analyze_this(
   setup *ss,
   setup *inners,
   setup *outers,
   int *center_arity_p,
   int & mapelong,
   int & inner_rot,
   int & outer_rot,
   calldef_schema analyzer)
{
   uint32 hash_num = ((ss->kind + (5*analyzer)) * 25) &
      (conc_tables::NUM_CONC_HASH_BUCKETS-1);

   const conc_tables::cm_thing *lmap_ptr;

   for (lmap_ptr = conc_tables::conc_hash_analyze_table[hash_num] ;
        lmap_ptr ;
        lmap_ptr = lmap_ptr->next_analyze) {
      if (lmap_ptr->bigsetup == ss->kind &&
          lmap_ptr->lyzer == analyzer) {

         for (int k=0; k<=attr::slimit(ss); k++) {
            if (ss->people[k].id1 && !(lmap_ptr->used_mask_analyze & (1<<k)))
               goto not_this_one;
         }

         int inlim = attr::klimit(lmap_ptr->insetup)+1;
         int outlim = attr::klimit(lmap_ptr->outsetup)+1;

         *center_arity_p = lmap_ptr->center_arity;
         outers->kind = lmap_ptr->outsetup;
         outers->rotation = (-lmap_ptr->outer_rot) & 3;

         gather(outers, ss, &lmap_ptr->maps[inlim*lmap_ptr->center_arity],
                outlim-1, lmap_ptr->outer_rot * 011);

         for (int m=0; m<lmap_ptr->center_arity; m++) {
            uint32 rr = lmap_ptr->inner_rot;

            // Need to flip alternating triangles upside down.
            if (lmap_ptr->insetup == s_trngl && (m&1)) rr ^= 2;

            clear_people(&inners[m]);
            inners[m].kind = lmap_ptr->insetup;
            inners[m].rotation = (0-rr) & 3;
            gather(&inners[m], ss, &lmap_ptr->maps[m*inlim], inlim-1, rr * 011);
         }

         mapelong = lmap_ptr->mapelong;
         inner_rot = lmap_ptr->inner_rot;
         outer_rot = lmap_ptr->outer_rot;

         return true;
      }
   not_this_one: ;
   }

   return false;
}


bool conc_tables::synthesize_this(
   setup *inners,
   setup *outers,
   int center_arity,
   uint32 orig_elong_is_controversial,
   int i,
   uint32 matrix_concept,
   int outer_elongation,
   calldef_schema synthesizer,
   setup *result)
{
   int index;

   if (inners[0].kind == s_trngl) {
      // For triangles, we use the "2" bit of the rotation, demanding that it be even.
      if (i&1) fail("Ends can't figure out what spots to finish on.");
      index = (i&2) >> 1;
   }
   else
      index = i&1;

   uint32 allowmask;

   if (outer_elongation == 3)
      allowmask = 0x10 << index;
   else if (outer_elongation <= 0 || outer_elongation > 3)
      allowmask = 5 << index;
   else
      allowmask = 1 << (index + ((((outer_elongation-1) ^ outers->rotation) & 1) << 1));

   if (matrix_concept) allowmask |= 0x40;

   uint32 hash_num =
      ((outers->kind + (5*(inners[0].kind + 5*synthesizer))) * 25) &
      (conc_tables::NUM_CONC_HASH_BUCKETS-1);

   const conc_tables::cm_thing *lmap_ptr;

   int q;
   int inlim, outlim;
   bool reverse_centers_order = false;

   for (lmap_ptr = conc_tables::conc_hash_synthesize_table[hash_num] ;
        lmap_ptr ;
        lmap_ptr = lmap_ptr->next_synthesize) {
      if (lmap_ptr->outsetup == outers->kind &&
          lmap_ptr->insetup == inners[0].kind &&
          lmap_ptr->center_arity == center_arity &&
          lmap_ptr->getout_schema == synthesizer &&
          (lmap_ptr->elongrotallow & allowmask) == 0) {
         // Find out whether inners need to be flipped around.
         q = i + lmap_ptr->inner_rot - lmap_ptr->outer_rot;

         if (q & 1) fail("Sorry, bug 1 in normalize_concentric.");

         if (synthesizer != schema_concentric_others) {   // This test is a crock!!!!!
            if ((outers->rotation + lmap_ptr->outer_rot + outer_elongation-1) & 2)
               reverse_centers_order = true;
         }

         inlim = attr::klimit(lmap_ptr->insetup)+1;
         outlim = attr::klimit(lmap_ptr->outsetup)+1;

         // If inner (that is, outer) setups are triangles, we won't be able to
         // turn them upside-down.  So accept it.  We don't have multiple maps
         // for such things in any case.

         if (inlim & 1) goto gotit;

         int inners_xorstuff = 0;
         if (q & 2) inners_xorstuff = inlim >> 1;

         uint32 synth_mask = 1;

         for (int k=0; k<lmap_ptr->center_arity; k++) {
            int k_reorder = reverse_centers_order ? lmap_ptr->center_arity-k-1 : k;
            for (int j=0; j<inlim; j++) {
               if (inners[k_reorder].people[j^inners_xorstuff].id1 &&
                   !(lmap_ptr->used_mask_synthesize & synth_mask))
                  goto not_this_one;
               synth_mask <<= 1;
            }
         }

         for (int m=0; m<outlim; m++) {
            if (outers->people[m].id1 &&
                !(lmap_ptr->used_mask_synthesize & synth_mask))
               goto not_this_one;
            synth_mask <<= 1;
         }

         goto gotit;
      }
   not_this_one: ;
   }

   return false;

 gotit:

   q = i + lmap_ptr->inner_rot - lmap_ptr->outer_rot;

   if (orig_elong_is_controversial) {
      // See if the table selection depended on knowing the actual elongation.
      if (lmap_ptr->elongrotallow & (5 << index))
         warn(warn_controversial);
   }

   if (reverse_centers_order) {
      if (lmap_ptr->center_arity == 2) {
         setup tt = inners[0];
         inners[0] = inners[1];
         inners[1] = tt;
      }
      else if (lmap_ptr->center_arity == 3) {
         setup tt = inners[0];
         inners[0] = inners[2];
         inners[2] = tt;
      }
   }

   const veryshort *map_indices = lmap_ptr->maps;

   for (int k=0; k<lmap_ptr->center_arity; k++) {
      uint32 rr = lmap_ptr->inner_rot;

      // Need to flip alternating triangles upside down.
      if (lmap_ptr->insetup == s_trngl && (k&1)) rr ^= 2;

      if (q & 2) {
         inners[k].rotation += 2;
         canonicalize_rotation(&inners[k]);
      }

      if (k != 0) {
         if (((inners[k].rotation ^ inners[k-1].rotation ^ lmap_ptr->inner_rot ^ rr) & 3) != 0)
            fail("Sorry, bug 2 in normalize_concentric.");
      }

      install_scatter(result, inlim, map_indices, &inners[k], ((0-rr) & 3) * 011);
      map_indices += inlim;
   }

   install_scatter(result, outlim, map_indices, outers,
                   ((0-lmap_ptr->outer_rot) & 3) * 011);

   result->kind = lmap_ptr->bigsetup;
   result->rotation = outers->rotation + lmap_ptr->outer_rot;
   return true;
}



/* This overwrites its "outer_inners" argument setups. */
extern void normalize_concentric(
   calldef_schema synthesizer,
   int center_arity,
   setup outer_inners[],   /* outers in position 0, inners follow */
   int outer_elongation,
   uint32 matrix_concept,
   setup *result) THROW_DECL
{
   /* If "outer_elongation" < 0, the outsides can't deduce their ending spots on
      the basis of the starting formation.  In this case, it is an error unless
      they go to some setup for which their elongation is obvious, like a 1x4.
      The "CONTROVERSIAL_CONC_ELONG" is similar, but says that the low 2 bits
      are sort of OK, and that a warning needs to be raised. */

   int i, j;
   setup *inners = &outer_inners[1];
   setup *outers = &outer_inners[0];
   calldef_schema table_synthesizer = synthesizer;
   uint32 orig_elong_is_controversial = outer_elongation & CONTROVERSIAL_CONC_ELONG;
   outer_elongation &= ~CONTROVERSIAL_CONC_ELONG;

   clear_people(result);
   result->result_flags = get_multiple_parallel_resultflags(outer_inners, center_arity+1);

   // Only the first setup (centers) counts when check for space invasion.
   if (!(inners->result_flags.misc & RESULTFLAG__INVADED_SPACE))
       result->result_flags.misc &= ~RESULTFLAG__INVADED_SPACE;

   if (outers->kind == s2x3 && (outers->result_flags.misc & RESULTFLAG__DID_SHORT6_2X3)) {
      expand::expand_setup(&s_short6_2x3, outers);
      outer_elongation = (outers->rotation & 1) + 1;
   }

   if (inners[0].kind == nothing && outers->kind == nothing) {
      result->kind = nothing;
      return;
   }

   compute_rotation_again:

   i = (inners[0].rotation - outers->rotation) & 3;

   if (synthesizer == schema_in_out_triple_squash) {

      // Do special stuff to put setups back properly for squashed schema.

      setup *i0p = &inners[0];
      setup *i1p = &inners[1];

      if (i0p->kind == s2x2) {
         // Move people to the closer parts of 2x2 setups.
         if (outer_elongation == 2) {
            if (!(i1p->people[2].id1 | i1p->people[3].id1)) {
               swap_people(i1p, 0, 3);
               swap_people(i1p, 1, 2);
            }
            if (!(i0p->people[0].id1 | i0p->people[1].id1)) {
               swap_people(i0p, 0, 3);
               swap_people(i0p, 1, 2);
            }
         }
         else if (outer_elongation == 1) {
            if (!(i1p->people[0].id1 | i1p->people[3].id1)) {
               swap_people(i1p, 0, 1);
               swap_people(i1p, 3, 2);
            }
            if (!(i0p->people[2].id1 | i0p->people[1].id1)) {
               swap_people(i0p, 0, 1);
               swap_people(i0p, 3, 2);
            }
         }

         center_arity = 2;
         table_synthesizer = schema_in_out_triple;
      }
      else if (i0p->kind == s1x4) {
         // Move people to the closer parts of 1x4 setups.
         if (i0p->rotation == 1 && outer_elongation == 2) {
            if (!(i1p->people[2].id1 | i1p->people[3].id1)) {
               swap_people(i1p, 0, 3);
               swap_people(i1p, 1, 2);
            }
            else if (!(i1p->people[2].id1)) {
               swap_people(i1p, 2, 3);
               swap_people(i1p, 1, 3);
               swap_people(i1p, 1, 0);
               warn(warn__compress_carefully);
            }
            if (!(i0p->people[0].id1 | i0p->people[1].id1)) {
               swap_people(i0p, 0, 3);
               swap_people(i0p, 1, 2);
            }
            else if (!(i0p->people[0].id1)) {
               swap_people(i0p, 1, 0);
               swap_people(i0p, 1, 3);
               swap_people(i0p, 2, 3);
               warn(warn__compress_carefully);
            }
         }
         else if (i0p->rotation == 0 && outer_elongation == 1) {
            if (!(i0p->people[2].id1 | i0p->people[3].id1)) {
               swap_people(i0p, 0, 3);
               swap_people(i0p, 1, 2);
            }
            else if (!(i0p->people[2].id1)) {
               swap_people(i0p, 2, 3);
               swap_people(i0p, 1, 3);
               swap_people(i0p, 1, 0);
               warn(warn__compress_carefully);
            }
            if (!(i1p->people[0].id1 | i1p->people[1].id1)) {
               swap_people(i1p, 0, 3);
               swap_people(i1p, 1, 2);
            }
            else if (!(i1p->people[0].id1)) {
               swap_people(i1p, 1, 0);
               swap_people(i1p, 1, 3);
               swap_people(i1p, 2, 3);
               warn(warn__compress_carefully);
            }
         }

         center_arity = 2;
         table_synthesizer = schema_in_out_triple;
      }
      else if (i0p->kind == s1x2 && i0p->rotation == 0 && outer_elongation == 2) {
         setup temp = *outers;
         const veryshort v1[] = {3, 2};
         const veryshort v2[] = {0, 1};
         scatter(outers, i0p, v1, 1, 0);
         scatter(outers, i1p, v2, 1, 0);
         outers->rotation = 0;
         outers->kind = s2x2;
         *i0p = temp;
         i = (i0p->rotation - outers->rotation) & 3;
         center_arity = 1;
      }
      else if (i0p->kind == s1x2 && i0p->rotation == 1 && outer_elongation == 1) {
         setup temp = *outers;
         const veryshort v1[] = {0, 3};
         const veryshort v2[] = {1, 2};
         scatter(outers, i0p, v1, 1, 0);
         scatter(outers, i1p, v2, 1, 0);
         outers->rotation = 0;
         outers->kind = s2x2;
         *i0p = temp;
         i = (i0p->rotation - outers->rotation) & 3;
         center_arity = 1;
      }
      else if (i0p->kind == s1x2 && i0p->rotation == 0 && outer_elongation == 1) {
         setup temp = *outers;
         const veryshort v1[] = {0, 1};
         const veryshort v2[] = {3, 2};
         scatter(outers, i0p, v1, 1, 0);
         scatter(outers, i1p, v2, 1, 0);
         outers->rotation = 0;
         outers->kind = s1x4;
         *i0p = temp;
         // Compute the rotation again.
         i = (i0p->rotation - outers->rotation) & 3;
         center_arity = 1;
      }
      else if (i0p->kind == s1x2 && i0p->rotation == 1 && outer_elongation == 2) {
         setup temp = *outers;
         const veryshort v1[] = {3, 2};
         const veryshort v2[] = {0, 1};
         scatter(outers, i0p, v1, 1, 0);
         scatter(outers, i1p, v2, 1, 0);
         outers->rotation = 1;
         outers->kind = s1x4;
         *i0p = temp;
         // Compute the rotation again.
         i = (i0p->rotation - outers->rotation) & 3;
         center_arity = 1;
      }
      else if (i0p->kind == sdmd) {
         table_synthesizer = schema_in_out_triple;
      }
      else if (i0p->kind == nothing) {
         if (outers->kind == sdmd) {
            clear_people(i0p);
            i0p->kind = s2x2;
            i0p->rotation = 0;
            *i1p = *i0p;
            center_arity = 2;
            table_synthesizer = schema_in_out_triple;
            // Compute the rotation again.
            i = (i0p->rotation - outers->rotation) & 3;
         }
         else {
            *i0p = *outers;
            outers->kind = nothing;
            outers->rotation = 0;
            // Compute the rotation again.
            i = (i0p->rotation - outers->rotation) & 3;
            center_arity = 1;
         }
      }
      else
         fail("Can't figure out what to do.");
   }
   else if (synthesizer == schema_sgl_in_out_triple_squash) {

      // Do special stuff to put setups back properly for squashed schema.

      setup *i0p = &inners[0];
      setup *i1p = &inners[1];

      if (i0p->kind == s1x2) {
         // Move people to the closer parts of 1x2 setups.

         if (i0p->rotation == 1 && outer_elongation == 2) {
            if (!(i0p->people[0].id1))
               swap_people(i0p, 0, 1);
            if (!(i1p->people[1].id1))
               swap_people(i1p, 0, 1);
         }
         else if (i0p->rotation == 0 && outer_elongation == 1) {
            if (!(i0p->people[1].id1))
               swap_people(i0p, 0, 1);
            if (!(i1p->people[0].id1))
               swap_people(i1p, 0, 1);
         }

         center_arity = 2;
         table_synthesizer = schema_sgl_in_out_triple;
      }
      else
         fail("Can't figure out what to do.");
   }
   else if (synthesizer == schema_3x3_in_out_triple_squash) {

      // Do special stuff to put setups back properly for squashed schema.

      setup *i0p = &inners[0];
      setup *i1p = &inners[1];

      if (i0p->kind == s2x3) {
         // Move people to the closer parts of 2x3 setups.

         if (i0p->rotation == 0 && outer_elongation == 2) {
            if (!(i0p->people[0].id1 | i0p->people[1].id1 | i0p->people[2].id1)) {
               swap_people(i0p, 0, 5);
               swap_people(i0p, 1, 4);
               swap_people(i0p, 2, 3);
            }
            if (!(i1p->people[3].id1 | i1p->people[4].id1 | i1p->people[5].id1)) {
               swap_people(i1p, 0, 5);
               swap_people(i1p, 1, 4);
               swap_people(i1p, 2, 3);
            }
         }
      }
      else if (i0p->kind == nothing && outer_elongation == 2) {
         i0p->kind = s2x3;
         i0p->rotation = 0;
         clear_people(i0p);
         i1p->kind = s2x3;
         i1p->rotation = 0;
         clear_people(i1p);
         // Compute the rotation again.
         i = (i0p->rotation - outers->rotation) & 3;
      }
      else
         fail("Can't figure out what to do.");

      center_arity = 2;
      table_synthesizer = schema_3x3_in_out_triple;
   }
   else if (synthesizer == schema_4x4_in_out_triple_squash) {

      // Do special stuff to put setups back properly for squashed schema.

      setup *i0p = &inners[0];
      setup *i1p = &inners[1];

      if (i0p->kind == s2x4) {
         // Move people to the closer parts of 2x4 setups.

         if (i0p->rotation == 0 && outer_elongation == 2) {
            if (!(i0p->people[0].id1 | i0p->people[1].id1 |
                  i0p->people[2].id1 | i0p->people[3].id1)) {
               swap_people(i0p, 0, 7);
               swap_people(i0p, 1, 6);
               swap_people(i0p, 2, 5);
               swap_people(i0p, 3, 4);
            }
            if (!(i1p->people[4].id1 | i1p->people[5].id1 |
                  i1p->people[6].id1 | i1p->people[7].id1)) {
               swap_people(i1p, 0, 7);
               swap_people(i1p, 1, 6);
               swap_people(i1p, 2, 5);
               swap_people(i1p, 3, 4);
            }
         }
      }
      else if (i0p->kind == nothing && outer_elongation == 2) {
         i0p->kind = s2x4;
         i0p->rotation = 0;
         clear_people(i0p);
         i1p->kind = s2x4;
         i1p->rotation = 0;
         clear_people(i1p);
      }
      else
         fail("Can't figure out what to do.");

      center_arity = 2;
      table_synthesizer = schema_4x4_in_out_triple;
      // Compute the rotation again.
      i = (inners[0].rotation - outers->rotation) & 3;
   }

   if (table_synthesizer == schema_conc_o) {
      if (outers->kind != s4x4)
         fail("Outsides are not on 'O' spots.");

      if (!(outers->people[5].id1 | outers->people[6].id1 |
            outers->people[13].id1 | outers->people[14].id1))
         outer_elongation = 1;
      else if (!(outers->people[1].id1 | outers->people[2].id1 |
                 outers->people[9].id1 | outers->people[10].id1))
         outer_elongation = 2;
      else
         outer_elongation = 3;
   }

   switch (synthesizer) {
   case schema_rev_checkpoint:
      // Fix up nonexistent centers or ends, in a rather inept way.
      if (inners[0].kind == nothing) {
         inners[0].kind = outers->kind;
         inners[0].rotation = outers->rotation;
         inners[0].result_flags = outers->result_flags;
         clear_people(&inners[0]);
         i = 0;
      }
      else if (outers->kind == nothing) {
         outers->kind = inners[0].kind;
         outers->rotation = inners[0].rotation;
         outers->result_flags = inners[0].result_flags;
         clear_people(outers);
         i = 0;
      }
      break;
   case schema_ckpt_star:
      // There are a few cases of centers or ends being phantoms, in which
      // we nevertheless know what to do, since we know that the setup should
      // be some kind of "winged star".
      if (inners[0].kind == nothing && outers->kind == s1x4) {
         inners[0].kind = s_star;
         inners[0].rotation = 0;
         inners[0].result_flags = outers->result_flags;
         clear_people(&inners[0]);
         goto compute_rotation_again;
      }
      else if (inners[0].kind == sdmd && outers->kind == nothing) {
         // The test case for this is: RWV:intlkphanbox relay top;splitphanbox flip reaction.
         outers->kind = s1x4;
         outers->rotation = inners[0].rotation;
         outers->result_flags = inners[0].result_flags;
         clear_people(outers);
         goto compute_rotation_again;
      }
      break;
   case schema_conc_star:
      // There are a few cases of centers or ends being phantoms, in which
      // we nevertheless know what to do, since we know that the setup should
      // be some kind of "winged star".
      if (outers->kind == nothing && inners[0].kind == s1x4) {
         outers->kind = s_star;
         outers->rotation = 0;
         outers->result_flags = inners[0].result_flags;
         clear_people(outers);
         goto compute_rotation_again;
      }
      else if (outers->kind == nothing && inners[0].kind == s_star) {
         outers->kind = s1x4;
         outers->rotation = outer_elongation-1;
         outers->result_flags = inners[0].result_flags;
         clear_people(outers);
         goto compute_rotation_again;
      }
      else if (outers->kind == s1x4 && inners[0].kind == nothing) {
         inners[0].kind = s_star;
         inners[0].rotation = 0;
         inners[0].result_flags = outers->result_flags;
         clear_people(&inners[0]);
         goto compute_rotation_again;
      }

      switch (outers->kind) {
         case s1x4:
            switch (inners[0].kind) {
               case sdmd:
                  // Just turn it into a star.
                  inners[0].kind = s_star;
                  canonicalize_rotation(&inners[0]);  // Need to do this; it has 4-way symmetry now.
                  goto compute_rotation_again;
               case s1x4:
                  /* In certain phantom cases, what should have been a diamond
                     around the outside, resulting from a 1/2 circulate, will be
                     a line with the two centers missing, since the basic_move
                     routine gives preference to a line when it is ambiguous.
                     If this happens, we have to turn it back into a diamond. */
                  if (!(outers->people[1].id1 | outers->people[3].id1)) {
                     outers->kind = sdmd;  /* That's all that it takes to fix it. */
                     goto compute_rotation_again;
                  }
                  // Or the two ends missing.
                  else if (!(outers->people[0].id1 | outers->people[2].id1)) {
                     expand::compress_setup(&s_1x4_dmd, outers);
                     goto compute_rotation_again;
                  }
                  break;
            }
            break;
      }

      break;
   case schema_conc_star12:
      if (center_arity == 2) {
         fix_missing_centers(inners, outers, s_star, s1x4, center_arity, true);
      }

      break;
   case schema_conc_star16:
      if (center_arity == 3) {
         fix_missing_centers(inners, outers, s_star, s1x4, center_arity, true);
      }

      break;
   case schema_conc_12:
      if (center_arity == 2) {
         fix_missing_centers(inners, outers, s1x6, s2x3, center_arity, true);
      }
      table_synthesizer = schema_concentric;

      break;
   case schema_conc_16:
      if (center_arity == 3) {
         fix_missing_centers(inners, outers, s1x8, s2x4, center_arity, true);
      }
      table_synthesizer = schema_concentric;

      break;
   case schema_conc_bar12:
      if (center_arity == 2)
         fix_missing_centers(inners, outers, s_star, s2x3, center_arity, true);

      break;
   case schema_conc_bar16:
      if (center_arity == 3)
         fix_missing_centers(inners, outers, s_star, s2x3, center_arity, true);

      break;
   case schema_concentric_others:
      if (center_arity == 2) {
         fix_missing_centers(inners, outers, s1x2, s2x2, center_arity, false);
      }
      else if (center_arity == 3) {
         fix_missing_centers(inners, outers, s1x2, s1x2, center_arity, false);
      }
      else
         table_synthesizer = schema_concentric;

      break;
   case schema_grand_single_concentric:
      if (center_arity == 3) {
         fix_missing_centers(inners, outers, s1x2, s1x2, center_arity, true);
      }

      break;
   default:
      // Fix up nonexistent centers or ends, in a rather inept way.
      if (inners[0].kind == nothing) {
         if (table_synthesizer == schema_conc_o) {
            inners[0].kind = s2x2;
            inners[0].rotation = 0;
         }
         else {
            inners[0].kind = outers->kind;
            inners[0].rotation = outers->rotation;
         }

         inners[0].result_flags = outers->result_flags;
         clear_people(&inners[0]);
         for (int j=1 ; j<center_arity ; j++) inners[j] = inners[0];
         i = 0;
      }
      else if (outers->kind == nothing) {
         if (table_synthesizer == schema_conc_o) {
            outers->kind = s4x4;
            outers->rotation = 0;
            outer_elongation = 3;
         }
         else {
            outers->kind = inners[0].kind;
            outers->rotation = inners[0].rotation;
         }
         outers->result_flags = inners[0].result_flags;
         clear_people(outers);
         i = 0;
      }

      if (table_synthesizer != schema_in_out_triple &&
          table_synthesizer != schema_sgl_in_out_triple &&
          table_synthesizer != schema_3x3_in_out_triple &&
          table_synthesizer != schema_4x4_in_out_triple &&
          table_synthesizer != schema_in_out_quad &&
          table_synthesizer != schema_in_out_12mquad &&
          table_synthesizer != schema_concentric_big2_6 &&
          table_synthesizer != schema_concentric_6_2_tgl &&
          table_synthesizer != schema_intlk_vertical_6 &&
          table_synthesizer != schema_intlk_lateral_6 &&
          table_synthesizer != schema_conc_o) {
         // Nonexistent center or ends have been taken care of.
         // Now figure out how to put the setups together.

         switch (outers->kind) {
         case sbigdmd:
            switch (inners[0].kind) {
            case s1x2:
               if ((outers->people[3].id1 | outers->people[9].id1)) goto anomalize_it;
               break;
            }
            break;
         case s4x4:
            switch (inners[0].kind) {
            case s2x2:
               table_synthesizer = schema_conc_o;
               goto compute_rotation_again;
            }
            break;
         }

         table_synthesizer = schema_concentric;   // They are all in the hash table this way.
      }

      break;
   }

   if (!conc_tables::synthesize_this(
         inners,
         outers,
         center_arity,
         orig_elong_is_controversial,
         i,
         matrix_concept,
         outer_elongation,
         table_synthesizer,
         result))
      goto anomalize_it;

   if (table_synthesizer == schema_conc_o)
      normalize_setup(result, simple_normalize, false);
   if (table_synthesizer == schema_sgl_in_out_triple)
      normalize_setup(result, normalize_to_4, false);

   canonicalize_rotation(result);
   return;

 anomalize_it:            // Failed, just leave it as it is.

   switch (synthesizer) {
   case schema_rev_checkpoint:
      fail("Sorry, can't figure out this reverse checkpoint result.");
   case schema_single_concentric:
      fail("Can't figure out this single concentric result.");
   case schema_grand_single_concentric:
      fail("Can't figure out this grand single concentric result.");
   case schema_concentric:
      break;
   default:
      fail("Can't figure out this concentric result.");
   }

   if (outer_elongation <= 0 || outer_elongation > 2)
      fail("Ends can't figure out what spots to finish on.");

   result->kind = s_normal_concentric;
   result->inner.skind = inners[0].kind;
   result->inner.srotation = inners[0].rotation;
   result->outer.skind = outers->kind;
   result->outer.srotation = outers->rotation;
   result->concsetup_outer_elongation = outer_elongation;
   if (outers->rotation & 1) result->concsetup_outer_elongation ^= 3;
   for (j=0; j<(MAX_PEOPLE/2); j++) {
      (void) copy_person(result, j, &inners[0], j);
      (void) copy_person(result, j+(MAX_PEOPLE/2), outers, j);
   }
   canonicalize_rotation(result);
   clear_result_flags(result);
}


/* This sets "outer_elongation" to the absolute elongation of the
   outsides.  If the outsides are in a 2x2, this, along with individual
   facing directions, will permit enforcement of the "lines-to-lines/
   columns-to-columns" rule.  Otherwise, this will permit enforcement
   of the Hodson checkpoint rule.

   There are a few cases in which this result may seem wrong:
      (1) If we have triple diamonds with points in only the center
         diamond (that is, a line of 6 with some points hanging off
         the center 2), and we ask for the center 6/outer 2, the
         lonesome points become the ends, and "outer_elongation"
         reflects their elongation, even though the line of 6 is longer.
         This doesn't seem to affect any checkpoint or concentric cases.
         Perhaps the phrases "center 6" and "outer 2" aren't really
         correct here.
      (2) If we have a quarter tag, and we ask for the center 6/outer 2,
         the center 6 are, of course, a 2x3, and the ends of the line
         are the outer 2.  We set "outer_elongation" to reflect
         the elongation of the outer 2, which may not be what people
         would think.  Once again, this does not arise in any actual
         checkpoint or concentric case.
      (3) If we have an "H", and we ask for the center 2/outer 6, the
         outer 6 are the 2x3, and "outer_elongation" will show
         their elongation, even though that is not the elongation of
         the 3x4 matrix.  Once again, this does not arise in any actual
         checkpoint or concentric case. */

static calldef_schema concentrify(
   setup *ss,
   calldef_schema & analyzer,
   int & crossing,   // This is int (0 or 1), not bool.
   int & inverting,  // This too.
   setup inners[],
   setup *outers,
   int *center_arity_p,
   int *outer_elongation,    /* Set to elongation of original outers, except if
                                center 6 and outer 2, in which case, if centers
                                are a bone6, it shows their elongation. */
   int *xconc_elongation) THROW_DECL    // If cross concentric, set to
                                        // elongation of original ends.
{
   int i;
   calldef_schema analyzer_result = analyzer;

   *outer_elongation = 1;   /*  **** shouldn't these be -1???? */
   *xconc_elongation = 1;
   clear_people(outers);
   clear_people(&inners[0]);

   // It will be helpful to have a mask of where the live people are.

   uint32 livemask = little_endian_live_mask(ss);

   // First, translate the analyzer into a form that encodes only what we need to know.

   crossing = 0;

   switch (analyzer_result) {
   case schema_single_cross_concentric:
      analyzer = schema_single_concentric;
      analyzer_result = schema_single_concentric;
      crossing = 1;
      break;
   case schema_cross_concentric_2_4:
      analyzer_result = schema_concentric_2_4;
      crossing = 1;
      break;
   case schema_cross_concentric_4_2:
      analyzer_result = schema_concentric_4_2;
      crossing = 1;
      break;
   case schema_cross_concentric_2_6_or_2_4:
      analyzer_result = schema_concentric_2_6_or_2_4;
      crossing = 1;
      break;
   case schema_cross_concentric_2_6:
      analyzer_result = schema_concentric_2_6;
      crossing = 1;
      break;
   case schema_cross_concentric_6_2:
      analyzer_result = schema_concentric_6_2;
      crossing = 1;
      break;
   case schema_grand_single_cross_concentric:
      analyzer = schema_grand_single_concentric;
      analyzer_result = schema_grand_single_concentric;
      crossing = 1;
      break;
   case schema_cross_concentric_innermost:
      analyzer = schema_concentric_innermost;
      analyzer_result = schema_concentric_innermost;
      crossing = 1;
      break;
   case schema_cross_concentric_double_innermost:
      analyzer = schema_concentric_double_innermost;
      analyzer_result = schema_concentric_double_innermost;
      crossing = 1;
      break;
   case schema_cross_checkpoint:
      analyzer = schema_checkpoint;
      analyzer_result = schema_checkpoint;
      crossing = 1;
      break;
   case schema_cross_concentric_6p_or_normal:
      analyzer = schema_concentric_6p_or_normal;
      analyzer_result = schema_concentric_6p_or_normal;
      crossing = 1;
      break;
   case schema_cross_concentric_diamonds:
      analyzer = schema_concentric_diamonds;
      analyzer_result = schema_concentric_diamonds;
      crossing = 1;
      break;
   case schema_cross_concentric_zs:
      analyzer = schema_concentric_zs;
      analyzer_result = schema_concentric_zs;
      crossing = 1;
      break;
   case schema_cross_concentric:
      analyzer = schema_concentric;
      analyzer_result = schema_concentric;
      crossing = 1;
      break;
   }

   switch (analyzer_result) {
      // Completely straightforward ones.
   case schema_single_concentric:
   case schema_concentric_4_2:
   case schema_grand_single_concentric:
   case schema_lateral_6:
   case schema_vertical_6:
   case schema_intlk_lateral_6:
   case schema_intlk_vertical_6:
   case schema_checkpoint:
   case schema_conc_12:
   case schema_conc_16:
   case schema_conc_star12:
   case schema_conc_star16:
   case schema_conc_bar:
   case schema_conc_bar12:
   case schema_conc_bar16:
   case schema_3x3_concentric:
   case schema_4x4_lines_concentric:
   case schema_4x4_cols_concentric:
   case schema_in_out_triple:
   case schema_in_out_triple_zcom:
   case schema_in_out_quad:
   case schema_in_out_12mquad:
   case schema_concentric_6_2:
   case schema_concentric_6p:
   case schema_1221_concentric:
   case schema_concentric_others:
   case schema_concentric_6_2_tgl:
   case schema_concentric_diamonds:
   case schema_concentric_2_4:
   case schema_concentric_8_4:
   case schema_concentric_zs:
      break;
   case schema_ckpt_star:
      analyzer_result = schema_checkpoint; break;
   case schema_conc_o:
      if (ss->kind == s_c1phan) do_matrix_expansion(ss, CONCPROP__NEEDK_4X4, false);
      break;
   case schema_concentric_diamond_line:
      if ((ss->kind == s_crosswave && (livemask & 0x88)) ||
          (ss->kind == s1x8 && (livemask & 0xCC)))
         fail("Can't find centers and ends in this formation.");
      break;
   case schema_concentric_6_2_line:
      if (ss->kind != s3x1dmd)
         analyzer_result = schema_concentric_6_2;
      break;
   case schema_concentric_2_6:
      switch (ss->kind) {
      case s3x4:
         if (livemask != 07171)
            fail("Can't find centers and ends in this formation.");
         break;
      case s4dmd:
         warn(warn_big_outer_triangles);
         analyzer = schema_concentric_big2_6;
         analyzer_result = schema_concentric_big2_6;
         if (livemask != 0xC9C9)
            fail("Can't find centers and ends in this formation.");
         break;
      case s_3mdmd:
         warn(warn_big_outer_triangles);
         analyzer = schema_concentric_big2_6;
         analyzer_result = schema_concentric_big2_6;
         // Warning!  Fall through!
      case s3dmd:
         // Warning!  Fell through!
         if (livemask != 04747)
            fail("Can't find centers and ends in this formation.");
         break;
      default:
         break;
      }

      break;
   case schema_concentric_2_6_or_2_4:
      if (attr::slimit(ss) == 5) {
         warn(warn__unusual);
         analyzer_result = schema_concentric_2_4;
      }
      else
         analyzer_result = schema_concentric_2_6;
      break;
   case schema_concentric_innermost:
      if (ss->kind == s_short6) {
         warn(warn__unusual);
         analyzer_result = schema_concentric_4_2;
      }
      else if (ss->kind == s2x4 || ss->kind == s_rigger || ss->kind == s_galaxy)
         analyzer_result = schema_concentric;
      else if (attr::slimit(ss) == 5)
         analyzer_result = schema_concentric_2_4;
      else if (attr::slimit(ss) == 3)
         analyzer_result = schema_single_concentric;
      else
         analyzer_result = schema_concentric_2_6;
      break;
   case schema_concentric_double_innermost:
      if (ss->kind == s_rigger || ss->kind == s_ptpd)
         analyzer_result = schema_concentric_6_2;
      else if (ss->kind == s_galaxy) {
         // This code is duplicated in triangle_move.  Make schemata "tall6" and "short6"
         uint32 tbonetest = ss->people[1].id1 | ss->people[3].id1 |
            ss->people[5].id1 | ss->people[7].id1;

         if ((tbonetest & 011) == 011) fail("Can't find tall 6.");
         else if (tbonetest & 1)
            analyzer_result = schema_lateral_6;
         else
            analyzer_result = schema_vertical_6;
      }
      else if (attr::slimit(ss) == 5)
         analyzer_result = schema_concentric_4_2;
      else
         analyzer_result = schema_concentric;
      break;
   case schema_in_out_triple_squash:
      analyzer_result = schema_in_out_triple; break;
   case schema_sgl_in_out_triple_squash:
      analyzer_result = schema_sgl_in_out_triple; break;
   case schema_3x3_in_out_triple_squash:
      analyzer_result = schema_3x3_in_out_triple; break;
   case schema_4x4_in_out_triple_squash:
      analyzer_result = schema_4x4_in_out_triple; break;
   case schema_concentric:
      if (crossing) {
         if (ss->kind == s4x4 && livemask != 0x9999)
            fail("Can't find centers and ends in this formation.");
      }
      break;
   case schema_rev_checkpoint:
   case schema_conc_star:
   case schema_concentric_to_outer_diamond:
      analyzer_result = schema_concentric;
      if (ss->kind == s4x4 && livemask != 0x9999)
         fail("Can't find centers and ends in this formation.");
      break;
   case schema_concentric_or_diamond_line:
      if (ss->kind == s3x1dmd)
         analyzer_result = schema_concentric_diamond_line;
      else
         analyzer_result = schema_concentric;
      break;
   case schema_concentric_or_6_2_line:
      if (ss->kind == s3x1dmd)
         analyzer_result = schema_concentric_6_2_line;
      else
         analyzer_result = schema_concentric;
      break;
   case schema_concentric_2_4_or_normal:
      if (attr::slimit(ss) == 5)
         analyzer_result = schema_concentric_2_4;
      else if (attr::slimit(ss) == 3)
         analyzer_result = schema_single_concentric;
      else if (ss->kind == s3x4 && (livemask == 0xCF3 || livemask == 0xF3C)) {
         analyzer_result = schema_in_out_triple;
         analyzer = schema_in_out_triple;
         inverting ^= 1;
      }
      else
         analyzer_result = schema_concentric;
      break;
   case schema_concentric_4_2_or_normal:
      if (attr::slimit(ss) == 5)
         analyzer_result = schema_concentric_4_2;
      else if (attr::slimit(ss) == 3)
         analyzer_result = schema_single_concentric;
      else
         analyzer_result = schema_concentric;
      break;
   case schema_concentric_6p_or_normal:
      if (attr::slimit(ss) == 5)
         analyzer_result = schema_concentric_6p;
      else
         analyzer_result = schema_concentric;
      break;
   case schema_concentric_6p_or_sgltogether:
      if (attr::slimit(ss) == 5)
         analyzer_result = schema_concentric_6p;
      else if (ss->kind == s1x8 || ss->kind == s_ptpd ||
               attr::slimit(ss) == 3)
         analyzer_result = schema_single_concentric;
      else
         analyzer_result = schema_concentric;
      break;
   case schema_1331_concentric:
      if (ss->kind == s3x4 && (livemask & 0111) == 0) {
         // Compress to a 1/4 tag.
         ss->kind = s_qtag;
         swap_people(ss, 1, 0);
         swap_people(ss, 2, 1);
         swap_people(ss, 4, 2);
         swap_people(ss, 5, 3);
         swap_people(ss, 7, 4);
         swap_people(ss, 8, 5);
         swap_people(ss, 10, 6);
         swap_people(ss, 11, 7);
      }
      else if (ss->kind != s_qtag && ss->kind != s_spindle && ss->kind != s3x1dmd)
         analyzer_result = schema_1313_concentric;
      break;
   default:
      fail("Internal error: Don't understand this concentricity type.");
   }

   /* Next, deal with the "normal_concentric" special case.
      We need to be careful here.  The setup was not able to be normalized, but
      we are being asked to pick out centers and ends.  There are very few
      non-normal concentric setups for which we can do that correctly.  For example,
      if we have concentric diamonds whose points are along different axes from each
      other, who are the 4 centers of the total setup?  (If the axes of the diamonds
      had been consistent, the setup would have been normalized to crossed lines,
      and we wouldn't be here.)
      If we don't take action here and the setup is normal_concentric, an error will
      be raised, since the "concthing" entries are zero. */

   if (ss->kind == s_normal_concentric || ss->kind == s_dead_concentric) {
      *center_arity_p = 1;

      if (ss->kind == s_dead_concentric) {
         ss->outer.skind = nothing;
         ss->outer.srotation = 0;
      }

      switch (analyzer_result) {
      case schema_conc_bar:
      case schema_concentric_diamond_line:
         if (ss->inner.skind == nothing && ss->outer.skind == s2x3) {
            inners[0].kind = s_star;
            inners[0].rotation = 0;
            outers->kind = s2x3;
            for (i=0; i<(MAX_PEOPLE/2); i++)
               (void) copy_person(outers, i, ss, i+(MAX_PEOPLE/2));
            *outer_elongation = ss->concsetup_outer_elongation;
            if ((ss->outer.srotation & 1) && (*outer_elongation) != 0) *outer_elongation ^= 3;
            goto finish;
         }

         break;
      case schema_concentric_2_6:
         outers->rotation = ss->outer.srotation;
         inners[0].rotation = ss->outer.srotation;   // Yes, this looks wrong, but it isn't.

         if (ss->outer.skind == nothing) {
            inners[0].kind = ss->inner.skind;
            inners[0].rotation = ss->inner.srotation;
            outers->kind = nothing;
            for (i=0; i<MAX_PEOPLE; i++)
               (void) copy_person(&inners[0], i, ss, i);
            *outer_elongation = ss->concsetup_outer_elongation;
            goto finish;
         }
         break;

      case schema_concentric:
         outers->rotation = ss->outer.srotation;
         inners[0].rotation = ss->outer.srotation;   // Yes, this looks wrong, but it isn't.

         if (ss->outer.skind == nothing) {
            inners[0].kind = ss->inner.skind;
            inners[0].rotation = ss->inner.srotation;
            outers->kind = nothing;
            for (i=0; i<MAX_PEOPLE; i++)
               (void) copy_person(&inners[0], i, ss, i);
            *outer_elongation = ss->concsetup_outer_elongation;
            goto finish;
         }
         else if (ss->inner.skind == sdmd && ss->inner.srotation == ss->outer.srotation) {
            inners[0].kind = sdmd;
            outers->kind = ss->outer.skind;
            for (i=0; i<(MAX_PEOPLE/2); i++) {
               (void) copy_person(&inners[0], i, ss, i);
               (void) copy_person(outers, i, ss, i+(MAX_PEOPLE/2));
            }

            /* ***** Claim these two won't happen, because of dhrglass and 1x3dmd! */

            /* We allow a diamond inside a box with wrong elongation
               (if elongation were good, it would be an hourglass.) */
            if (ss->outer.skind == s2x2) {
               *outer_elongation = ss->concsetup_outer_elongation;
               if (ss->outer.srotation & 1) *outer_elongation ^= 3;
               goto finish;
            }
            /* And a diamond inside a line with wrong elongation
               (if elongation were good, it would be a 3x1 diamond.) */
            if (ss->outer.skind == s1x4) {
               *outer_elongation = (ss->outer.srotation & 1) + 1;
               goto finish;
            }
         }
         else if (ss->inner.skind == s1x2 &&
                  ss->outer.skind == s1x6 &&
                  ss->inner.srotation != ss->outer.srotation) {
            static const veryshort map44[4] = {12, 13, 15, 16};

            inners[0].kind = sdmd;
            outers->kind = s1x4;

            if ((ss->inner.srotation - ss->outer.srotation) & 2) {
               (void) copy_rot(&inners[0], 1, ss, 1, 033);
               (void) copy_rot(&inners[0], 3, ss, 0, 033);
            }
            else {
               (void) copy_rot(&inners[0], 1, ss, 0, 011);
               (void) copy_rot(&inners[0], 3, ss, 1, 011);
            }

            (void) copy_person(&inners[0], 0, ss, 14);
            (void) copy_person(&inners[0], 2, ss, 17);
            gather(outers, ss, map44, 3, 0);
            goto finish;
         }
         break;
      }
   }

   // Next, do the 3x4 -> qtag fudging.  Don't ask permission, just do it.

   if (analyzer_result == schema_concentric && ss->kind == s3x4) {
      if ((livemask & 03131UL) != 01111UL) {
         *center_arity_p = 1;
         inners[0].kind = s1x4;
         inners[0].rotation = 0;
         outers->kind = s2x2;
         outers->rotation = 0;
         *outer_elongation = ((~outers->rotation) & 1) + 1;
         (void) copy_person(&inners[0], 0, ss, 10);
         (void) copy_person(&inners[0], 1, ss, 11);
         (void) copy_person(&inners[0], 2, ss, 4);
         (void) copy_person(&inners[0], 3, ss, 5);

         if (!ss->people[0].id1 && ss->people[1].id1)
            (void) copy_person(outers, 0, ss, 1);
         else if (!ss->people[1].id1 && !ss->people[0].id1)
            (void) copy_person(outers, 0, ss, 0);
         else fail("Can't find centers and ends in this formation.");

         if (!ss->people[2].id1 && ss->people[3].id1)
            (void) copy_person(outers, 1, ss, 3);
         else if (!ss->people[3].id1 && ss->people[2].id1)
            (void) copy_person(outers, 1, ss, 2);
         else fail("Can't find centers and ends in this formation.");

         if (!ss->people[6].id1 && ss->people[7].id1)
            (void) copy_person(outers, 2, ss, 7);
         else if (!ss->people[7].id1 && ss->people[6].id1)
            (void) copy_person(outers, 2, ss, 6);
         else fail("Can't find centers and ends in this formation.");

         if (!ss->people[8].id1 && ss->people[9].id1)
            (void) copy_person(outers, 3, ss, 9);
         else if (!ss->people[9].id1 && ss->people[8].id1)
            (void) copy_person(outers, 3, ss, 8);
         else fail("Can't find centers and ends in this formation.");

         goto finish;
      }
   }

   if ((ss->kind == s4x4 || ss->kind == s4dmd) &&
       (analyzer_result == schema_conc_16 ||
        analyzer_result == schema_conc_star16 ||
        analyzer_result == schema_conc_bar16))
      analyzer_result = schema_4x4_cols_concentric;

   if ((ss->kind == s3x4 || ss->kind == s3dmd) &&
       (analyzer_result == schema_conc_12 ||
        analyzer_result == schema_conc_star12 ||
        analyzer_result == schema_conc_bar12))
      analyzer_result = schema_3x3_concentric;

   if (analyzer_result == schema_4x4_cols_concentric ||
       analyzer_result == schema_4x4_lines_concentric) {

      /* There is a minor kludge in the tables.  For "4x4_cols" and "4x4_lines",
         the tables don't know anything about people's facing direction,
         so the two analyzers are used to split vertically or laterally.
         Therefore, we translate, based on facing direction. */

      if (ss->kind == s4x4) {
         uint32 tbone1 =
            ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1 |
            ss->people[3].id1 | ss->people[7].id1 | ss->people[11].id1 | ss->people[15].id1;

         uint32 tbone2 =
            ss->people[5].id1 | ss->people[6].id1 | ss->people[13].id1 | ss->people[14].id1 |
            ss->people[3].id1 | ss->people[7].id1 | ss->people[11].id1 | ss->people[15].id1;

         if (analyzer_result == schema_4x4_cols_concentric) {
            if (!(tbone2 & 011) && (tbone1 & 001))
               analyzer_result = schema_4x4_lines_concentric;
            else if (!(tbone1 & 010) && (tbone2 & 001))
               analyzer_result = schema_4x4_lines_concentric;
            else if (tbone2 & 001)
               fail("Can't do this from T-bone setup.");
         }
         else {
            if (!(tbone1 & 011) && (tbone2 & 001))
               analyzer_result = schema_4x4_cols_concentric;
            else if (!(tbone2 & 010) && (tbone1 & 001))
               analyzer_result = schema_4x4_cols_concentric;
            else if (tbone1 & 001)
               fail("Can't do this from T-bone setup.");
         }
      }
   }

   if (analyzer_result == schema_1331_concentric)
      analyzer_result = schema_concentric_6_2;
   else if (analyzer_result == schema_1313_concentric)
      analyzer_result = schema_concentric_2_6;
   else if (analyzer_result == schema_1221_concentric)
      analyzer_result = schema_concentric_6p;
   else if (analyzer_result == schema_in_out_triple_zcom)
      analyzer_result = schema_in_out_triple;

   {
      int mapelong;
      int inner_rot;
      int outer_rot;

      if (!conc_tables::analyze_this(
            ss,
            inners,
            outers,
            center_arity_p,
            mapelong,
            inner_rot,
            outer_rot,
            analyzer_result)) {
         switch (analyzer_result) {
         case schema_concentric:
         case schema_conc_bar:
            fail("Can't find centers and ends in this formation.");
         case schema_checkpoint:
            fail("Can't find checkpoint people in this formation.");
         case schema_concentric_2_6:
         case schema_concentric_big2_6:
            fail("Can't find 2 centers and 6 ends in this formation.");
         case schema_concentric_2_4:
            fail("Can't find 2 centers and 4 ends in this formation.");
         case schema_concentric_4_2:
            fail("Can't find 4 centers and 2 ends in this formation.");
         case schema_concentric_6_2:
         case schema_concentric_6_2_line:
            fail("Can't find 6 centers and 2 ends in this formation.");
         case schema_concentric_6p:
         case schema_1221_concentric:
            fail("Can't find centers and ends in this 6-person formation.");
         case schema_concentric_6_2_tgl:
            fail("Can't find inside triangles in this formation.");
         case schema_conc_o:
            fail("Can't find outside 'O' spots.");
         case schema_conc_12:
         case schema_conc_star12:
         case schema_conc_bar12:
            fail("Can't find 12 matrix centers and ends in this formation.");
         case schema_conc_16:
         case schema_conc_star16:
         case schema_conc_bar16:
            fail("Can't find 16 matrix centers and ends in this formation.");
         case schema_3x3_concentric:
            fail("Can't find 3x3 centers and ends in this formation.");
         case schema_4x4_lines_concentric:
            fail("Can't find 16 matrix center and end lines in this formation.");
         case schema_4x4_cols_concentric:
            fail("Can't find 16 matrix center and end columns in this formation.");
         case schema_1331_concentric:
         case schema_1313_concentric:
            fail("Can't find 3x1 concentric formation.");
         case schema_single_concentric:
            fail("Can't do single concentric in this formation.");
         case schema_grand_single_concentric:
            fail("Can't do grand single concentric in this formation.");
         case schema_in_out_triple:
         case schema_sgl_in_out_triple:
         case schema_3x3_in_out_triple:
         case schema_4x4_in_out_triple:
            fail("Can't find triple lines/columns/boxes/diamonds in this formation.");
         case schema_in_out_quad:
         case schema_in_out_12mquad:
            fail("Can't find phantom lines/columns/boxes/diamonds in this formation.");
         case schema_concentric_diamonds:
            fail("Can't find concentric diamonds.");
         case schema_concentric_zs:
            fail("Can't find concentric Z's.");
         case schema_concentric_diamond_line:
            fail("Can't find center line and outer diamond.");
         default:
            fail("Wrong formation.");
         }
      }

      /* Set the outer elongation to whatever elongation the outsides really had, as indicated
         by the map. */

      *outer_elongation = mapelong;
      if (outers->rotation & 1) *outer_elongation ^= 3;

      /* If the concept is cross-concentric, we have to set the elongation to what
         the centers (who will, of course, be going to the outside) had.
         If the original centers are in a 2x2, we set it according to the orientation
         of the entire 2x4 they were in, so that they can think about whether they were
         in lines or columns and act accordingly.  If they were not in a 2x4, that is,
         the setup was a wing or galaxy, we set the elongation to -1 to indicate an
         error.  In such a case the centers won't be able to decide whether they were
         in lines or columns. */

      /* The following additional comment used to be present, back when we obeyed a misguided
         notion of what cross concentric means:
         "But if the outsides started in a 1x4, they override the centers' own axis." */

      switch (analyzer) {
      case schema_concentric:
      case schema_concentric_diamonds:
      case schema_concentric_zs:   // This may not be right.
      case schema_concentric_6p_or_normal:
         if (crossing) {
            *xconc_elongation = inner_rot+1;

            switch (ss->kind) {
            case s4x4:
               if (inners[0].kind == s2x2) *xconc_elongation = 3;
               break;
            case s_galaxy:
               *xconc_elongation = -1;    // Can't do this!
               break;
            case s_rigger:
               *xconc_elongation = -1;
               if (outers->kind == s1x4)
                  *xconc_elongation = (outer_rot+1) | CONTROVERSIAL_CONC_ELONG;
               break;
            }
         }
         break;

      case schema_in_out_triple:
      case schema_in_out_triple_squash:
      case schema_sgl_in_out_triple_squash:
      case schema_3x3_in_out_triple_squash:
      case schema_4x4_in_out_triple_squash:
      case schema_sgl_in_out_triple:
      case schema_3x3_in_out_triple:
      case schema_4x4_in_out_triple:
      case schema_in_out_quad:
      case schema_in_out_12mquad:
      case schema_in_out_triple_zcom:
      case schema_conc_o:
         *outer_elongation = mapelong;   // The map defines it completely.
         break;
      case schema_concentric_6_2_tgl:
         if (inners[0].kind == s_bone6)
            *outer_elongation = ((~outers->rotation) & 1) + 1;
         break;
      case schema_concentric_6_2:
         if (inners[0].kind == s_bone6)
            *outer_elongation = (outers->rotation & 1) + 1;
         break;
      }
   }

 finish:

   canonicalize_rotation(outers);
   canonicalize_rotation(&inners[0]);
   if (*center_arity_p >= 2)
      canonicalize_rotation(&inners[1]);
   if (*center_arity_p == 3)
      canonicalize_rotation(&inners[2]);

   return analyzer_result;
}




warning_index concwarneeetable[] = {warn__lineconc_perp, warn__xclineconc_perpe, warn__lineconc_par};
warning_index concwarn1x4table[] = {warn__lineconc_perp, warn__xclineconc_perpc, warn__lineconc_par};
warning_index concwarndmdtable[] = {warn__dmdconc_perp, warn__xcdmdconc_perpc, warn__dmdconc_par};


static bool fix_empty_outers(
   setup_kind sskind,
   setup_kind & final_outers_start_kind,
   uint32 & localmods1,
   int crossing,
   int begin_outer_elongation,
   int center_arity,
   calldef_schema & analyzer,
   setup_command *cmdin,
   setup_command *cmdout,
   setup *begin_outer,
   setup *result_outer,
   setup *result_inner,
   setup *result)
{
   /* If the schema is one of the special ones, we will know what to do. */
   if (analyzer == schema_conc_star ||
       analyzer == schema_ckpt_star ||
       analyzer == schema_conc_star12 ||
       analyzer == schema_conc_star16) {

      /* This is what makes 12 matrix relay the top work when everyone is
            in the stars. */

      result_outer->kind = s1x4;
      clear_people(result_outer);
      clear_result_flags(result_outer);
      result_outer->rotation = 0;
   }
   else if (analyzer == schema_conc_bar ||
            analyzer == schema_conc_bar12 ||
            analyzer == schema_conc_bar16) {

      // This is what makes 12 matrix quarter the deucey work
      // when everyone is in the stars.

      result_outer->kind = s2x3;
      clear_people(result_outer);
      clear_result_flags(result_outer);
      result_outer->result_flags.misc |= 1;
      result_outer->rotation = 1;
   }
   else if (analyzer == schema_in_out_triple_squash) {
      result_outer->kind = s2x2;
      clear_people(result_outer);
      clear_result_flags(result_outer);
      result_outer->rotation = 0;
   }
   else if (analyzer == schema_concentric_diamond_line) {
      switch (sskind) {
      case s_wingedstar:
      case s_wingedstar12:
      case s_wingedstar16:
      case s_barredstar:
      case s_barredstar12:
      case s_barredstar16:
      case s3x1dmd:
         result_outer->kind = s2x2;
         result_outer->rotation = 0;
         clear_people(result_outer);
         /* Set their "natural" elongation perpendicular to their original diamond.
            The test for this is 1P2P; touch 1/4; column circ; boys truck; split phantom
            lines tag chain thru reaction.  They should finish in outer triple boxes,
            not a 2x4. */
         result_outer->result_flags = result_inner->result_flags;
         result_outer->result_flags.misc &= ~3;
         result_outer->result_flags.misc |= 2;
         break;
      default:
         fail("Can't figure out ending setup for concentric call -- no ends.");
      }
   }
   else {
      uint32 orig_elong_flags = result_outer->result_flags.misc & 3;

      /* We may be in serious trouble -- we have to figure out what setup the ends
         finish in, and they are all phantoms. */

      *result_outer = *begin_outer;          // Restore the original bunch of phantoms.
      clear_result_flags(result_outer);

      /* If setup is 2x2 and a command "force spots" or "force otherway" was given, we can
         honor it, even though there are no people present in the outer setup. */

      if (final_outers_start_kind == s2x2 &&
          result_outer->kind == s2x2 &&
          (localmods1 & (DFM1_CONC_FORCE_SPOTS | DFM1_CONC_FORCE_OTHERWAY))) {
         ;        /* Take no further action. */
      }
      else if (result_outer->kind == s4x4 && analyzer == schema_conc_o) {
         if (sskind == s2x4 &&
             final_outers_start_kind == s4x4 &&
             cmdout &&
             cmdout->cmd_frac_flags == CMD_FRAC_NULL_VALUE &&
             cmdout->callspec == base_calls[base_call_two_o_circs]) {
            result_outer->kind = s2x2;
            analyzer = schema_concentric;
            final_outers_start_kind = s2x2;
            localmods1 |= DFM1_CONC_FORCE_OTHERWAY;
         }

         // Otherwise, take no action.
      }
      else if (final_outers_start_kind == s1x4 &&
               result_outer->kind == s1x4 &&
               (localmods1 & DFM1_CONC_FORCE_SPOTS)) {
         /* If a call starts in a 1x4 and has "force spots" indicated, it must go to a 2x2
            with same elongation. */
         result_outer->kind = s2x2;    /* Take no further action. */
      }
      else if (final_outers_start_kind == s1x4 &&
               !crossing &&
               (orig_elong_flags ^ begin_outer_elongation) == 3 &&
               (localmods1 & DFM1_CONC_CONCENTRIC_RULES)) {
         /* If a call starts in a 1x4 but tries to set the result elongation to the
            opposite of the starting elongation, it must have been trying to go to a 2x2.
            In that case, the "opposite elongation" rule applies. */
         result_outer->kind = s2x2;    /* Take no further action. */
      }
      else if (final_outers_start_kind == s1x2 &&
               result_outer->kind == s1x2 &&
               ((orig_elong_flags+1) & 2)) {
         /* Note that the "desired elongation" is the opposite
            of the rotation, because, for 1x2 calls, the final
            elongation is opposite of what it should be.
            (Why?  So that, if we specify "parallel_conc_end", it
            will be correct.  Why?  Because counter rotate has that
            flag set in order to get the right 2x2 behavior, and
            this way we get right 1x2 behavior also.  Isn't that
            a stupid reason?  Yes.)  (See the documentation
            of the "parallel_conc_end" flag.)  So we do what
            seems to be the wrong thing. */
         result_outer->rotation = orig_elong_flags & 1;
      }
      else {
         // Otherwise, we can save the day only if we can convince
         // ourselves that they did the call "nothing", or a very few
         // other calls.  We make use of the fact that "concentrify"
         // did NOT flush them, so we still know what their starting
         // setup was.  This is what makes split phantom diamonds
         // diamond chain through work from columns far apart, among
         // other things.

         // Make sure these people go to the same spots, and remove possibly misleading info.
         localmods1 |= DFM1_CONC_FORCE_SPOTS;
         localmods1 &= ~(DFM1_CONC_FORCE_LINES | DFM1_CONC_FORCE_COLUMNS | DFM1_CONC_FORCE_OTHERWAY);

         call_with_name *the_call = begin_outer->cmd.callspec;   // Might be null pointer.

         if (the_call && the_call->the_defn.schema == schema_nothing)
            ;        // It's OK, the call was "nothing"
         else if (the_call == base_calls[base_call_trade])
            ;        // It's OK, the call was "trade"
         else if (the_call == base_calls[base_call_circulate] &&
                  cmdout && cmdout->cmd_final_flags.test_heritbit(INHERITFLAG_HALF) &&
                  cmdout->cmd_frac_flags == CMD_FRAC_NULL_VALUE) {
            // The call is "1/2 circulate".  We assume it goes to a diamond.
            // We don't check that the facing directions are such that this is
            // actually true.  Too bad.
            // Well, it actually goes to a star.  It's the best we can do, given that
            // we don't know facing directions.  In practice, that's good enough.
            localmods1 &= ~DFM1_CONC_FORCE_SPOTS;  // We don't want this after all.
            result_outer->kind = s_star;
         }
         else if ((the_call == base_calls[base_base_hinge_and_then_trade] ||
                   the_call == base_calls[base_base_hinge_and_then_trade_for_breaker]) &&
                  ((orig_elong_flags+1) & 2)) {
            result_outer->kind = s1x4;
            result_outer->rotation = (orig_elong_flags & 1) ^ 1;
         }
         else if (center_arity > 1)
            ;        // It's OK.
         else {
            /* We simply have no idea where the outsides should be.  We
               simply contract the setup to a 4-person setup (or whatever),
               throwing away the outsides completely.  If this was an
               "on your own", it may be possible to put things back together.
               This is what makes "1P2P; pass thru; ENDS leads latch on;
               ON YOUR OWN disband & snap the lock" work.  But if we try to glue
               these setups together, "fix_n_results" will raise an error, since
               it won't know whether to leave room for the phantoms. */

            *result = *result_inner;   // This gets all the inner people, and the result_flags.
            result->kind = s_dead_concentric;
            result->inner.skind = result_inner[0].kind;
            result->inner.srotation = result_inner[0].rotation;
            result->rotation = 0;
            /* We remember a vague awareness of where the outside would have been. */
            result->concsetup_outer_elongation = begin_outer_elongation;
            return true;
         }
      }
   }

   // If we are doing something like "triple boxes",
   // we really want the setups to look alike.
   if (cmdin == cmdout && analyzer == schema_in_out_triple) {
      result_outer->kind = result_inner[0].kind;
      result_outer->rotation = result_inner[0].rotation;
      clear_people(result_outer);  // Do we need this?  Maybe the result setup got bigger.
   }

   return false;
}


static bool fix_empty_inners(
   setup_kind orig_inners_start_kind,
   int center_arity,
   calldef_schema analyzer,
   calldef_schema analyzer_result,
   setup *begin_inner,
   setup *result_outer,
   setup *result_inner,
   setup *result)
{
   for (int i=0 ; i<center_arity ; i++) {
      clear_people(&result_inner[i]);    // This is always safe.
      clear_result_flags(&result_inner[i]);
   }

   // If the schema is one of the special ones, we will know what to do.
   if (analyzer == schema_conc_star ||
       analyzer == schema_ckpt_star ||
       analyzer == schema_conc_star12 ||
       analyzer == schema_conc_star16 ||
       analyzer == schema_in_out_triple ||
       analyzer == schema_sgl_in_out_triple_squash ||
       analyzer == schema_sgl_in_out_triple ||
       analyzer == schema_3x3_in_out_triple_squash ||
       analyzer == schema_3x3_in_out_triple ||
       analyzer == schema_4x4_in_out_triple_squash ||
       analyzer == schema_4x4_in_out_triple ||
       analyzer == schema_in_out_quad ||
       analyzer == schema_in_out_12mquad) {
      // Take no action.
   }
   else if (analyzer == schema_in_out_triple_squash && center_arity == 2) {
      result_inner[0].kind = s2x2;
      result_inner[0].rotation = 0;
      result_inner[1].kind = s2x2;
      result_inner[1].rotation = 0;
   }
   else if (analyzer == schema_conc_bar && result_outer->kind == s2x3) {
      // Fix some quarter the deucey stuff.
      switch (orig_inners_start_kind) {
      case s2x2:
         result_inner->kind = s1x4;
         result_inner->rotation = result_outer->rotation;
         break;
      case s_star:
         result_inner->kind = s_star;
         result_inner->rotation = 0;
         break;
      }
   }
   /* If the ends are a 2x2, we just set the missing centers to a 2x2.
      The ends had better know their elongation, of course.  It shouldn't
      matter to the ends whether the phantoms in the center did something
      that leaves the whole setup as diamonds or as a 2x4.  (Some callers
      might think it matters (Hi, Clark!) but it doesn't matter to this program.)
      This is what makes split phantom diamonds diamond chain through work
      from a grand wave. */
   /* Also, if doing "O" stuff, it's easy.  Do the same thing. */
   else if (center_arity == 1 &&
            (result_outer->kind == s2x2 ||
             (result_outer->kind == s4x4 && analyzer == schema_conc_o))) {
      uint32 orig_elong_flags = result_outer->result_flags.misc & 3;

      if (analyzer_result == schema_concentric_6p && ((orig_elong_flags+1) & 2)) {
         result_inner->kind = s1x2;
         result_inner->rotation = orig_elong_flags & 1;
      }
      else {
         result_inner->kind = s2x2;
         result_inner->rotation = 0;
      }
   }
   /* If the ends are a 1x4, we just set the missing centers to a 1x4,
      unless the missing centers did "nothing", in which case they
      retain their shape. */
   else if (result_outer->kind == s1x4 && center_arity == 1) {
      if (begin_inner->cmd.callspec &&
          (begin_inner->cmd.callspec->the_defn.schema == schema_nothing)) {
         /* Restore the original bunch of phantoms. */
         *result_inner = *begin_inner;
         clear_result_flags(result_inner);
      }
      else {
         result_inner->kind =
            (analyzer_result == schema_concentric_6p) ? s1x2 : s1x4;
         result_inner->rotation = result_outer->rotation;
      }
   }
   /* A similar thing, for single concentric. */
   else if (result_outer->kind == s1x2 &&
            (analyzer == schema_single_concentric)) {
      result_inner->kind = s1x2;
      result_inner->rotation = result_outer->rotation;
   }
   /* If the ends are a 1x6, we just set the missing centers to a 1x2,
      so the entire setup is a 1x8.  Maybe the phantoms went the other way,
      so the setup is really a 1x3 diamond, but we don't care.  See the comment
      just above. */
   else if (result_outer->kind == s1x6 && analyzer == schema_concentric_2_6) {
      result_inner->kind = s1x2;
      result_inner->rotation = result_outer->rotation;
   }
   // If the ends are a short6, (presumably the whole setup was a qtag or hrglass),
   // and the missing centers were an empty 1x2, we just restore that 1x2.
   else if (result_outer->kind == s_short6 &&
            analyzer_result == schema_concentric_2_6 &&
            begin_inner->kind == s1x2) {
      *result_inner = *begin_inner;
      clear_result_flags(result_inner);
   }
   // Similarly, for 6-person arrangements.
   else if (result_outer->kind == s_star &&
            analyzer_result == schema_concentric_2_4 &&
            begin_inner->kind == s1x2) {
      *result_inner = *begin_inner;
      clear_result_flags(result_inner);
   }
   else {
      /* The centers are just gone!  It is quite possible that "fix_n_results"
         may be able to repair this damage by copying some info from another setup.
         Missing centers are not as serious as missing ends, because they won't
         lead to indecision about whether to leave space for the phantoms. */

      int j;
      *result = *result_outer;   // This gets the result_flags.
      result->kind = s_normal_concentric;
      result->rotation = 0;
      result->outer.skind = result_outer->kind;
      result->outer.srotation = result_outer->rotation;
      result->inner.skind = nothing;
      result->inner.srotation = 0;
      result->concsetup_outer_elongation = 0;

      for (j=0; j<(MAX_PEOPLE/2); j++)
         (void) copy_person(result, j+(MAX_PEOPLE/2), result_outer, j);
      clear_result_flags(result);

      return true;
   }

   return false;
}


static void inherit_conc_assumptions(
   setup_kind sskind,
   setup_kind beginkind,
   calldef_schema analyzer,
   int really_doing_ends,
   assumption_thing *this_assumption)
{
   if (analyzer == schema_concentric ||
       analyzer == schema_concentric_6p_or_normal ||
       analyzer == schema_concentric_4_2_or_normal ||
       analyzer == schema_concentric_2_4_or_normal) {
      if (sskind == s2x4 && beginkind == s2x2) {
         switch (this_assumption->assumption) {
         case cr_wave_only:
            /* Waves [wv/0/0] or normal columns [wv/1/0] go to normal boxes [wv/0/0]. */
            this_assumption->assump_col = 0;
            goto got_new_assumption;
         case cr_magic_only:
            if (this_assumption->assump_col == 0) {
               /* Inv lines [mag/0/0] go to couples_only [cpl/0/0]. */
               this_assumption->assumption = cr_couples_only;
            }
            else {
               /* Magic cols [mag/1/0] go to normal boxes [wv/0/0]. */
               this_assumption->assumption = cr_wave_only;
               this_assumption->assump_col = 0;
            }
            goto got_new_assumption;
         case cr_2fl_only:
            if (this_assumption->assump_col == 0) {
               /* 2FL [2fl/0/0] go to normal boxes [wv/0/0]. */
               this_assumption->assumption = cr_wave_only;
            }
            else {
               /* DPT/CDPT [2fl/1/x] go to facing/btb boxes [lilo/0/x]. */
               this_assumption->assumption = cr_li_lo;
               this_assumption->assump_col = 0;
            }
            goto got_new_assumption;
         case cr_li_lo:
            if (this_assumption->assump_col == 0) {
               /* facing/btb lines [lilo/0/x] go to facing/btb boxes [lilo/0/x]. */
            }
            else {
               /* 8ch/tby [lilo/1/x] go to facing/btb boxes [lilo/0/y], */
               this_assumption->assumption = cr_li_lo;
               this_assumption->assump_col = 0;
               /* Where calculation of whether facing or back-to-back is complicated. */
               if (!really_doing_ends)
                  this_assumption->assump_both ^= 3;
            }
            goto got_new_assumption;
         case cr_1fl_only:
            if (this_assumption->assump_col == 0) {
               /* 1-faced lines [1fl/0/0] go to couples_only [cpl/0/0]. */
               this_assumption->assumption = cr_couples_only;
               goto got_new_assumption;
            }
            break;
         }
      }
      if (sskind == s1x8 && beginkind == s1x4) {
         switch (this_assumption->assumption) {
         case cr_wave_only:
         case cr_2fl_only:
         case cr_1fl_only:
            goto got_new_assumption;
         }
      }
      else if (sskind == s_qtag &&
               beginkind == s2x2 &&
               this_assumption->assump_col == 0) {
         switch (this_assumption->assumption) {
         case cr_jright:
         case cr_jleft:
         case cr_ijright:
         case cr_ijleft:
            /* 1/4 tag or line [whatever/0/2] go to facing in [lilo/0/1]. */
            this_assumption->assumption = cr_li_lo;
            /* 3/4 tag or line [whatever/0/1] go to facing out [lilo/0/2]. */
            this_assumption->assump_both ^= 3;
            goto got_new_assumption;
         case cr_ctr_miniwaves:
         case cr_ctr_couples:
            /* Either of those special assumptions means that the outsides
               are in a normal box. */
            this_assumption->assumption = cr_wave_only;
            this_assumption->assump_col = 0;
            this_assumption->assump_both = 0;
            goto got_new_assumption;
         }
      }
      else if (sskind == s_qtag && beginkind == s1x4) {
         switch (this_assumption->assumption) {
         case cr_ctr_miniwaves:
            this_assumption->assumption = cr_wave_only;
            this_assumption->assump_col = 0;
            this_assumption->assump_both = 0;
            goto got_new_assumption;
         case cr_ctr_couples:
            this_assumption->assumption = cr_2fl_only;
            this_assumption->assump_col = 0;
            this_assumption->assump_both = 0;
            goto got_new_assumption;
         }
      }
   }
   else if (analyzer == schema_concentric_2_6) {
      if ((sskind == s_qtag || sskind == s_ptpd) &&
          beginkind == s_short6 && really_doing_ends) {
         // We want to preserve "assume diamond" stuff to the outer 6,
         // so 6x2 and 3x2 acey deucey will work.
         // Of course, it will have a different meaning.  "Assume
         // normal diamonds" will mean "this is a wave-based triangle,
         // with consistent direction".  "Assume facing diamonds" will
         // mean "this is a wave-based triangle, with the base in a
         // real mini-wave, but the apex pointing inconsistently".
         goto got_new_assumption;
      }
   }
   else if (analyzer == schema_ckpt_star &&
            really_doing_ends == 1 &&
            sskind == s_spindle &&
            this_assumption->assumption == cr_ckpt_miniwaves) {
      // The box is a real box.  This makes the hinge win on chain reaction.
      this_assumption->assumption = cr_wave_only;
      goto got_new_assumption;
   }
   else if (analyzer == schema_checkpoint &&
            sskind == s1x8 &&
            this_assumption->assumption == cr_wave_only &&
            this_assumption->assump_col == 0) {
      if (really_doing_ends == 0)
         this_assumption->assump_both = 0;   // Handedness changed -- if we weren't lazy, we would do this right.
      goto got_new_assumption;
   }
   else if (analyzer == schema_single_concentric) {
      if (sskind == s1x4 && beginkind == s1x2 &&
          (this_assumption->assumption == cr_2fl_only ||
           this_assumption->assumption == cr_wave_only)) {
         this_assumption->assumption = cr_wave_only;
         goto got_new_assumption;
      }
   }

   this_assumption->assumption = cr_none;

 got_new_assumption: ;
}

static const expand::thing fix_cw  = {{1, 2, 4, 5}, 4, s2x2, s2x3, 0};
static const expand::thing fix_ccw = {{0, 1, 3, 4}, 4, s2x2, s2x3, 0};

extern void concentric_move(
   setup *ss,
   setup_command *cmdin,
   setup_command *cmdout,
   calldef_schema analyzer,
   uint32 modifiersin1,
   uint32 modifiersout1,
   bool recompute_id,
   uint32 specialoffsetmapcode,
   setup *result) THROW_DECL
{
   setup begin_inner[3];
   setup begin_outer;
   int begin_outer_elongation;
   int begin_xconc_elongation;
   int center_arity;
   uint32 rotstate, pointclip;
   calldef_schema analyzer_result;
   int rotate_back = 0;
   setup result_inner[3];
   setup result_outer;
   setup outer_inners[4];
   int i, k, klast;
   int crossing;       // This is an int (0 or 1) rather than a bool,
                       // because we will index with it.

   // The original info about the people who STARTED on the inside.
   setup_kind orig_inners_start_kind;
   int orig_inners_start_rot;
   uint32 orig_inners_start_dirs;
   uint32 orig_inners_start_directions[32];

   // The original info about the people who STARTED on the outside.
   setup_kind orig_outers_start_kind;
   int orig_outers_start_rot;
   uint32 orig_outers_start_dirs;
   uint32 orig_outers_start_directions[32];

   // The original info about the people who will FINISH on the outside.
   setup_kind final_outers_start_kind;
   int final_outers_start_rot;
   uint32 *final_outers_start_directions;

   // The original info about the people who will FINISH on the inside.
   setup_kind final_inners_start_kind;

   // The final info about the people who FINISHED on the outside.
   int final_outers_finish_dirs;
   uint32 final_outers_finish_directions[32];
   uint32 ccmask, eemask;

   const call_conc_option_state save_state = current_options;
   uint32 save_cmd_misc2_flags = ss->cmd.cmd_misc2_flags;
   parse_block *save_skippable = ss->cmd.skippable_concept;
   const uint32 scnxn =
      ss->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_NXNMASK | INHERITFLAG_MXNMASK);

   ss->cmd.cmd_misc2_flags &= ~(0xFFF | CMD_MISC2__ANY_WORK_INVERT |
                                CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG);
   ss->cmd.skippable_concept = (parse_block *) 0;

   // It is clearly too late to expand the matrix -- that can't be what is wanted.
   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

   // But, if we thought we weren't sure enough of where people were to allow stepping
   // to a wave, we are once again sure.
   ss->cmd.cmd_misc_flags &= ~CMD_MISC__NO_STEP_TO_WAVE;

   if (ss->kind == s_qtag &&
       analyzer != schema_concentric_6_2 &&
       (ss->cmd.cmd_final_flags.test_heritbit(INHERITFLAG_12_MATRIX) ||
        scnxn == INHERITFLAGMXNK_1X3 ||
        scnxn == INHERITFLAGMXNK_3X1 ||
        scnxn == INHERITFLAGNXNK_3X3))
      do_matrix_expansion(ss, CONCPROP__NEEDK_3X4, true);

   for (i=0; i<32; i++) {
      orig_inners_start_directions[i] =
      orig_outers_start_directions[i] =
      final_outers_finish_directions[i] = 0;
   }

   if (save_cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {
      if (save_cmd_misc2_flags & CMD_MISC2__CENTRAL_SNAG) {
         if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
            fail("Can't do fractional \"snag\".");
      }

      if (!(save_cmd_misc2_flags & CMD_MISC2__DO_CENTRAL))
         ss->cmd.cmd_misc2_flags &= ~CMD_MISC2__CTR_END_MASK;
   }

   if (analyzer == schema_single_concentric_together) {
      if (ss->kind == s1x8 || ss->kind == s_ptpd || attr::slimit(ss) == 3)
         analyzer = schema_single_concentric;
      else if (ss->kind == s_bone6)
         analyzer = schema_concentric_6p;
      else
         analyzer = schema_concentric;
   }

   begin_inner[0].cmd = ss->cmd;
   begin_inner[1].cmd = ss->cmd;
   begin_inner[2].cmd = ss->cmd;
   begin_outer.cmd = ss->cmd;

   int inverting = 0;

   // This reads and writes to "analyzer" and "inverting", and writes to "crossing".
   analyzer_result = concentrify(ss, analyzer, crossing, inverting,
                                 begin_inner, &begin_outer, &center_arity,
                                 &begin_outer_elongation, &begin_xconc_elongation);

   // But reverse them if doing "invert".
   if (save_cmd_misc2_flags & CMD_MISC2__SAID_INVERT)
      inverting ^= 1;

   uint32 localmodsin1 = modifiersin1;
   uint32 localmodsout1 = modifiersout1;

   if (inverting) {
      localmodsin1 = modifiersout1;
      localmodsout1 = modifiersin1;
   }

   // Get initial info for the original ends.
   orig_outers_start_dirs = 0;
   for (i=0, k=1, eemask=0;
        i<=attr::klimit(begin_outer.kind);
        i++, k<<=1) {
      uint32 q = begin_outer.people[i].id1;
      orig_outers_start_dirs |= q;
      if (q) {
         eemask |= k;
         orig_outers_start_directions[(q >> 6) & 037] = q;
      }
   }
   orig_outers_start_kind = begin_outer.kind;
   orig_outers_start_rot = begin_outer.rotation;

   // Get initial info for the original centers.
   orig_inners_start_dirs = 0;
   for (i=0, k=1, ccmask=0;
        i<=attr::klimit(begin_inner[0].kind);
        i++, k<<=1) {
      uint32 q = begin_inner[0].people[i].id1;
      orig_inners_start_dirs |= q;
      if (q) {
         ccmask |= k;
         orig_inners_start_directions[(q >> 6) & 037] = q;
      }
   }
   orig_inners_start_kind = begin_inner[0].kind;
   orig_inners_start_rot = begin_inner[0].rotation;

   if (crossing) {
      setup temptemp = begin_inner[0];
      begin_inner[0] = begin_outer;
      begin_outer = temptemp;

      if (analyzer == schema_grand_single_concentric && center_arity == 3) {
         temptemp = begin_inner[2];
         begin_inner[2] = begin_inner[1];
         begin_inner[1] = temptemp;
      }

      uint32 ttt = ccmask;
      ccmask = eemask;
      eemask = ttt;
      final_outers_start_kind = orig_inners_start_kind;
      final_outers_start_rot = orig_inners_start_rot;
      final_outers_start_directions = orig_inners_start_directions;
      final_inners_start_kind = orig_outers_start_kind;
   }
   else {
      final_outers_start_kind = orig_outers_start_kind;
      final_outers_start_rot = orig_outers_start_rot;
      final_outers_start_directions = orig_outers_start_directions;
      final_inners_start_kind = orig_inners_start_kind;
   }

   // If the call turns out to be "detour", this will make it do just the ends part.

   if (analyzer != schema_in_out_triple_squash &&
       analyzer != schema_in_out_triple &&
       analyzer != schema_sgl_in_out_triple_squash &&
       analyzer != schema_3x3_in_out_triple_squash &&
       analyzer != schema_4x4_in_out_triple_squash &&
       analyzer != schema_sgl_in_out_triple &&
       analyzer != schema_3x3_in_out_triple &&
       analyzer != schema_4x4_in_out_triple &&
       analyzer != schema_in_out_quad &&
       analyzer != schema_in_out_12mquad)
      begin_outer.cmd.cmd_misc_flags |= CMD_MISC__DOING_ENDS;

   // There are two special pieces of information we now have that will help us decide
   // where to put the outsides.  "Orig_outers_start_kind" tells what setup the outsides
   // were originally in, and "begin_outer_elongation" tells how the outsides were
   // oriented (1=horiz, 2=vert).  "begin_outer_elongation" refers to absolute orientation,
   // that is, "our" view of the setups, taking all rotations into account.
   // "final_outers_start_directions" gives the individual orientations (absolute)
   // of the people who are finishing on the outside.  Later, we will compute
   // "final_outers_finish_dirs", telling how the individual people were oriented.
   // How we use all this information depends on many things that we will attend to below.

   // Giving one of the concept descriptor pointers as nil indicates that
   // we don't want those people to do anything.

   // We will now do the parts, controlled by the counter k.  There are multiple
   // instances of doing the centers' parts, indicated by k = 0, 1, ..... center_arity-1.
   // (Normally, center_arity=1, so we have just k=0).  There is just one instance
   // of doing the ends' part, indicated by k not in [0..center_arity-1].

   // Now this is made complicated by the fact that we sometimes want to do things
   // in a different order.  Usually, we want to do the centers first and the ends
   // later, but, when the schema is triple lines, we want to do the ends first and
   // the centers later.  This has to do with the order in which we query the user
   // for "slant <anything> and <anything>".  So, in the normal case, we let k run from
   // 0 to center_arity, inclusive, with the final value for the ends.  When the schema
   // is triple lines, we let k run from zero to center_arity-1, with zero for the ends.

   k = 0;
   klast = center_arity+1;

   if (analyzer == schema_in_out_triple_squash || analyzer == schema_in_out_triple ||
       analyzer == schema_sgl_in_out_triple_squash ||
       analyzer == schema_3x3_in_out_triple_squash ||
       analyzer == schema_4x4_in_out_triple_squash ||
       analyzer == schema_sgl_in_out_triple ||
       analyzer == schema_3x3_in_out_triple ||
       analyzer == schema_4x4_in_out_triple ||
       analyzer == schema_in_out_quad || analyzer == schema_in_out_12mquad) {
      k = -1;
      klast = center_arity;

      if (save_cmd_misc2_flags & CMD_MISC2__CENTRAL_MYSTIC) {
         // Need to switch this, because we are reversing
         // who we think are "centers" and "ends".
         save_cmd_misc2_flags ^= CMD_MISC2__INVERT_MYSTIC;
      }
   }

   // We only allow the "matrix" concept for a few schemata, and only
   // if "cmdin" (which contains the *outsides'* command) is null.
   // That is we allow it only for stuff like "center phantom columns".

   uint32 matrix_concept = ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT;
   begin_outer.cmd.cmd_misc_flags &= ~CMD_MISC__MATRIX_CONCEPT;

   if (cmdin || k == 0) {
      if (matrix_concept)
         fail("\"Matrix\" concept must be followed by applicable concept.");
   }

   for (; k<klast; k++) {
      int doing_ends = (k<0) || (k==center_arity);
      setup *begin_ptr = doing_ends ? &begin_outer : &begin_inner[k];
      setup *result_ptr = doing_ends ? &result_outer : &result_inner[k];
      uint32 modifiers1 = doing_ends ? localmodsout1 : localmodsin1;
      setup_command *cmdptr = (doing_ends ^ inverting) ? cmdout : cmdin;
      uint32 ctr_use_flag = doing_ends ?
         (CMD_MISC2__ANY_WORK|CMD_MISC2__ANY_WORK_INVERT) :
         CMD_MISC2__ANY_WORK;

      current_options = save_state;

      // We allow "quick so-and-so shove off"!

      if (analyzer != schema_in_out_triple_squash &&
          analyzer != schema_in_out_triple &&
          analyzer != schema_in_out_quad &&
          analyzer != schema_in_out_12mquad &&
          (analyzer != schema_concentric || doing_ends || center_arity != 1))
         begin_ptr->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;

      if (cmdptr) {
         uint32 mystictest;
         bool demand_no_z_stuff = false;
         begin_ptr->cmd.parseptr = cmdptr->parseptr;
         begin_ptr->cmd.callspec = cmdptr->callspec;
         begin_ptr->cmd.cmd_final_flags = cmdptr->cmd_final_flags;
         begin_ptr->cmd.cmd_frac_flags = cmdptr->cmd_frac_flags;

         // If doing something under a "3x1" (or "1x3") concentric schema,
         // put the "3x3" flag into the 6-person call, whichever call that is,
         // and "single" into the other one.
         if (analyzer == schema_1331_concentric) {
            if (attr::slimit(begin_ptr) == 5) {
               begin_ptr->cmd.cmd_final_flags.clear_heritbit(INHERITFLAG_NXNMASK);
               begin_ptr->cmd.cmd_final_flags.set_heritbit(INHERITFLAGNXNK_3X3);
            }
            else
               begin_ptr->cmd.cmd_final_flags.set_heritbit(INHERITFLAG_SINGLE);
         }
         else if (analyzer == schema_1221_concentric) {
            if (attr::slimit(begin_ptr) == 1)
               begin_ptr->cmd.cmd_final_flags.set_heritbit(INHERITFLAG_SINGLE);
         }

         // Check for operating on a Z.

         if (begin_ptr->kind == s2x3 &&
             ((!doing_ends && analyzer == schema_concentric) ||
              analyzer == schema_concentric_zs)) {
            uint32 mask = doing_ends ? eemask : ccmask;
            if (mask == 066) {
               begin_ptr->cmd.cmd_misc2_flags |= CMD_MISC2__IN_Z_CW;
               demand_no_z_stuff = (analyzer == schema_concentric);
            }
            else if (mask == 033) {
               begin_ptr->cmd.cmd_misc2_flags |= CMD_MISC2__IN_Z_CCW;
               demand_no_z_stuff = (analyzer == schema_concentric);
            }
         }

         if (doing_ends) {
            // If the operation isn't being done with "DFM1_CONC_CONCENTRIC_RULES"
            // (that is, this is some implicit operation), we allow the user
            // to give the explicit "concentric" concept.  We respond to that concept
            // simply by turning on "DFM1_CONC_CONCENTRIC_RULES".  This makes
            // things like "shuttle [concentric turn to a line]" work.

            if (!inverting && !begin_ptr->cmd.callspec &&
                !(localmodsout1 & DFM1_CONC_CONCENTRIC_RULES)) {
               final_and_herit_flags junk_concepts;
               junk_concepts.clear_all_herit_and_final_bits();
               const parse_block *next_parseptr;

               next_parseptr = process_final_concepts(begin_ptr->cmd.parseptr,
                                                      false, &junk_concepts, true, __FILE__, __LINE__);

               if (junk_concepts.test_herit_and_final_bits() == 0 &&
                   next_parseptr->concept->kind == concept_concentric) {
                  localmodsout1 |= DFM1_CONC_CONCENTRIC_RULES;
                  begin_ptr->cmd.parseptr = next_parseptr->next;
               }
            }

            // If the ends' starting setup is a 2x2, and we did not say
            // "concentric" (indicated by the DFM1_CONC_CONCENTRIC_RULES
            // flag being off), we mark the setup as elongated.  If the
            // call turns out to be a 2-person call, the elongation will be
            // checked against the pairings of people, and an error will be
            // given if it isn't right.  This is what makes "cy-kick"
            // illegal from diamonds, and "ends hinge" illegal from waves.
            // The reason this is turned off when the "concentric" concept
            // is given is so that "concentric hinge" from waves, obnoxious
            // as it may be, will be legal.

            // We also turn it off if this is reverse checkpoint.  In that
            // case, the ends know exactly where they should go.  This is
            // what makes "reverse checkpoint recycle by star thru" work
            // from a DPT setup.

            if (analyzer != schema_in_out_triple_squash &&
                analyzer != schema_in_out_triple &&
                analyzer != schema_in_out_triple_zcom &&
                analyzer != schema_sgl_in_out_triple_squash &&
                analyzer != schema_3x3_in_out_triple_squash &&
                analyzer != schema_4x4_in_out_triple_squash &&
                analyzer != schema_sgl_in_out_triple &&
                analyzer != schema_3x3_in_out_triple &&
                analyzer != schema_4x4_in_out_triple &&
                analyzer != schema_in_out_quad &&
                analyzer != schema_in_out_12mquad &&
                analyzer != schema_conc_o &&
                analyzer != schema_rev_checkpoint) {
               if ((begin_ptr->kind == s2x2 ||
                    begin_ptr->kind == s2x3 ||
                    begin_ptr->kind == s_short6) &&
                   begin_outer_elongation > 0) {      // We demand elongation be 1 or more.
                  begin_ptr->cmd.prior_elongation_bits = begin_outer_elongation;

                  // If "demand lines" or "demand columns" has been given, we suppress
                  // elongation checking.  In that case, the database author knows what
                  // elongation is required and is taking responsibility for it.
                  // This is what makes "scamper" and "divvy up" work.
                  // We also do this if the concept is cross concentric.
                  // In that case the people doing the "ends" call actually did it in
                  // the center (the way they were taught in C2 class) before moving
                  // to the outside.

                  if (((DFM1_CONC_CONCENTRIC_RULES | DFM1_CONC_DEMAND_LINES |
                        DFM1_CONC_DEMAND_COLUMNS) & localmodsout1) ||
                      crossing ||
                      analyzer == schema_checkpoint)
                     begin_ptr->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;
               }
               else if (begin_ptr->kind == s1x4 || begin_ptr->kind == s1x6) {
                  // Indicate that these people are working around the outside.
                  begin_ptr->cmd.prior_elongation_bits = 0x40;

                  if ((DFM1_CONC_CONCENTRIC_RULES & localmodsout1) ||
                      crossing ||
                      analyzer == schema_checkpoint)
                     begin_ptr->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;

                  // Do some more stuff.  This makes "fan the gating [recycle]" work.
                  if ((DFM1_SUPPRESS_ELONGATION_WARNINGS & localmodsout1) &&
                      analyzer_result == schema_concentric)
                     begin_ptr->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;
               }
            }
         }
         else {
            if ((begin_ptr->kind == s2x2 || begin_ptr->kind == s_short6) &&
                cmdout &&
                begin_outer_elongation > 0) {
               begin_ptr->cmd.prior_elongation_bits = begin_outer_elongation << 8;
            }
         }

         process_number_insertion(modifiers1);

         if (recompute_id && !(save_cmd_misc2_flags & CMD_MISC2__CTR_END_KMASK))
            update_id_bits(begin_ptr);

         // Inherit certain assumptions to the child setups.  This is EXTREMELY incomplete.

         if (begin_ptr->cmd.cmd_assume.assumption != cr_none)
            inherit_conc_assumptions(ss->kind, begin_ptr->kind,
                                     analyzer, (doing_ends ^ crossing),
                                     &begin_ptr->cmd.cmd_assume);

         // This call to "move" will fill in good stuff (viz. the DFM1_CONCENTRICITY_FLAG_MASK)
         // into begin_ptr->cmd.cmd_misc_flags, which we will use below to do various
         // "force_lines", "demand_columns", etc. things.

         if (doing_ends) {
            // If cross concentric, we are looking for plain "mystic"
            mystictest = crossing ? CMD_MISC2__CENTRAL_MYSTIC :
               (CMD_MISC2__CENTRAL_MYSTIC | CMD_MISC2__INVERT_MYSTIC);

            // Handle "invert snag" for ends.
            if ((save_cmd_misc2_flags & (CMD_MISC2__CENTRAL_SNAG | CMD_MISC2__INVERT_SNAG)) ==
                (CMD_MISC2__CENTRAL_SNAG | CMD_MISC2__INVERT_SNAG)) {
               if (mystictest == CMD_MISC2__CENTRAL_MYSTIC)
                  fail("Can't do \"central/snag/mystic\" with this call.");
               begin_ptr->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
            }
            else if ((save_cmd_misc2_flags & (CMD_MISC2__ANY_SNAG | CMD_MISC2__ANY_WORK_INVERT))
                     == (CMD_MISC2__ANY_SNAG | CMD_MISC2__ANY_WORK_INVERT)) {
               begin_ptr->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
            }

            // This makes it not normalize the setup between parts -- the 4x4 stays around.
            if (analyzer == schema_conc_o)
               begin_ptr->cmd.cmd_misc_flags |= CMD_MISC__EXPLICIT_MATRIX;
         }
         else {
            // If cross concentric, we are looking for "invert mystic".

            mystictest = crossing ?
               (CMD_MISC2__CENTRAL_MYSTIC | CMD_MISC2__INVERT_MYSTIC) :
               CMD_MISC2__CENTRAL_MYSTIC;

            // Handle "snag" for centers.
            if ((save_cmd_misc2_flags & (CMD_MISC2__CENTRAL_SNAG | CMD_MISC2__INVERT_SNAG))
                == CMD_MISC2__CENTRAL_SNAG) {
               if (mystictest == (CMD_MISC2__CENTRAL_MYSTIC | CMD_MISC2__INVERT_MYSTIC))
                  fail("Can't do \"central/snag/mystic\" with this call.");
               begin_ptr->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
            }
            else if ((save_cmd_misc2_flags & (CMD_MISC2__ANY_SNAG | CMD_MISC2__ANY_WORK_INVERT))
                     == CMD_MISC2__ANY_SNAG) {
               begin_ptr->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
            }
         }

         // Handle "invert mystic" for ends, or "mystic" for centers.
         // Also handle the "mystify_split" stuff.

         bool we_are_mirroring =
            (save_cmd_misc2_flags & (CMD_MISC2__CENTRAL_MYSTIC | CMD_MISC2__INVERT_MYSTIC)) ==
            mystictest;

         if (we_are_mirroring) {
            mirror_this(begin_ptr);
            begin_ptr->cmd.cmd_misc_flags ^= CMD_MISC__EXPLICIT_MIRROR;
         }

         uint32 orig_z_bits = begin_ptr->cmd.cmd_misc2_flags;

         if ((save_cmd_misc2_flags &
              (CMD_MISC2__ANY_WORK|CMD_MISC2__ANY_WORK_INVERT)) == ctr_use_flag) {

            if (!save_skippable)
               fail("Internal error in centers/ends work, please report this.");

            if (!begin_ptr->cmd.callspec)
               fail("No callspec, centers/ends!!!!!!");

            // We are going to alter the list structure while executing
            // the subject call, and then restore same when finished.

            parse_block *z1 = save_skippable;
            while (z1->concept->kind > marker_end_of_list) z1 = z1->next;

            call_with_name *savecall = z1->call;
            call_with_name *savecall_to_print = z1->call_to_print;
            bool savelevelcheck = z1->no_check_call_level;
            parse_block *savebeginparse = begin_ptr->cmd.parseptr;

            z1->call = begin_ptr->cmd.callspec;
            z1->call_to_print = begin_ptr->cmd.callspec;
            z1->no_check_call_level = true;
            begin_ptr->cmd.callspec = (call_with_name *) 0;
            begin_ptr->cmd.parseptr = save_skippable;

            try {
               impose_assumption_and_move(begin_ptr, result_ptr);
            }
            catch(error_flag_type foo) {
               // An error occurred.  We need to resore stuff.
               begin_ptr->cmd.callspec = z1->call;
               begin_ptr->cmd.parseptr = savebeginparse;
               z1->call = savecall;
               z1->call_to_print = savecall_to_print;
               z1->no_check_call_level = savelevelcheck;
               throw(foo);
            }

            // Restore.
            begin_ptr->cmd.callspec = z1->call;
            begin_ptr->cmd.parseptr = savebeginparse;
            z1->call = savecall;
            z1->call_to_print = savecall_to_print;
            z1->no_check_call_level = savelevelcheck;
         }
         else {
            if (specialoffsetmapcode != ~0UL) {
               divided_setup_move(begin_ptr, specialoffsetmapcode,
                                  phantest_only_one, true, result_ptr);
            }
            else
               impose_assumption_and_move(begin_ptr, result_ptr);
         }

         if (demand_no_z_stuff && (result_ptr->result_flags.misc & RESULTFLAG__DID_Z_COMPRESSMASK))
            fail("Can't do this call from a \"Z\".");

         remove_tgl_distortion(result_ptr);

         if (we_are_mirroring)
            mirror_this(result_ptr);

         current_options = save_state;

         if (analyzer == schema_in_out_triple_zcom || analyzer == schema_concentric_zs) {
            if (analyzer == schema_in_out_triple_zcom) orig_z_bits = ss->cmd.cmd_misc2_flags;

            uint32 z_compress_direction =
               (result_ptr->result_flags.misc & RESULTFLAG__DID_Z_COMPRESSMASK) /
               RESULTFLAG__DID_Z_COMPRESSBIT;
            if (z_compress_direction) {
               if (result_ptr->kind == s2x2) {
                  const expand::thing *fixp;
                  uint32 rotfix = z_compress_direction-1;   // This is 0 for E-W or 1 for N-S
                  result_ptr->rotation -= rotfix;
                  canonicalize_rotation(result_ptr);

                  if (orig_z_bits & CMD_MISC2__IN_Z_CW)
                     fixp = &fix_cw;
                  else if (orig_z_bits & CMD_MISC2__IN_Z_CCW)
                     fixp = &fix_ccw;
                  else if (orig_z_bits & CMD_MISC2__IN_AZ_CW)
                     fixp = doing_ends ? &fix_cw : &fix_ccw;
                  else if (orig_z_bits & CMD_MISC2__IN_AZ_CCW)
                     fixp = doing_ends ? &fix_ccw : &fix_cw;
                  else
                     fail("Internal error: Can't figure out how to unwind anisotropic Z's.");

                  expand::expand_setup(fixp, result_ptr);
                  result_ptr->rotation += rotfix;
               }
               else
                  fail("Can't do this shape-changer in a 'Z'.");
            }

            if (analyzer == schema_in_out_triple_zcom) analyzer = schema_in_out_triple;
         }
      }
      else {
         begin_ptr->cmd.callspec = (call_with_name *) 0;
         *result_ptr = *begin_ptr;
         clear_result_flags(result_ptr);

         if (doing_ends) {
            if (begin_outer_elongation <= 0 || begin_outer_elongation > 2) {
               // Outer people have unknown elongation and aren't moving.  Not good.
            }
            else {
               result_ptr->result_flags.misc = begin_outer_elongation;
            }

            // Make sure these people go to the same spots.
            localmodsout1 |= DFM1_CONC_FORCE_SPOTS;
         }

         // Strip out the roll bits -- people who didn't move can't roll.
         if (attr::slimit(result_ptr) >= 0) {
            for (i=0; i<=attr::slimit(result_ptr); i++) {
               if (result_ptr->people[i].id1)
                  result_ptr->people[i].id1 =
                     (result_ptr->people[i].id1 & (~NROLL_MASK)) | ROLL_IS_M;
            }
         }
      }

      if (analyzer ==  schema_concentric_to_outer_diamond &&
          doing_ends &&
          result_ptr->kind != sdmd) {
         if (result_ptr->kind != s1x4 || (result_ptr->people[1].id1 | result_ptr->people[3].id1))
            fail("Can't make a diamond out of this.");
         result_ptr->kind = sdmd;
      }
   }

   // Now, if some command (centers or ends) didn't exist, we pick up the needed result flags
   // from the other part.
   // Grab the "did_last_part" flags from the call that was actually done.

   if (inverting) {
      if (!cmdin)
         result_outer.result_flags.misc |= result_inner[0].result_flags.misc &
            (RESULTFLAG__DID_LAST_PART|
             RESULTFLAG__SECONDARY_DONE|
             RESULTFLAG__PARTS_ARE_KNOWN);

      if (!cmdout) {
         for (k=0; k<center_arity; k++)
            result_inner[k].result_flags.misc |= result_outer.result_flags.misc &
               (RESULTFLAG__DID_LAST_PART|
                RESULTFLAG__SECONDARY_DONE|
                RESULTFLAG__PARTS_ARE_KNOWN);
      }
   }
   else {
      if (!cmdout)
         result_outer.result_flags.misc |= result_inner[0].result_flags.misc & (RESULTFLAG__DID_LAST_PART|RESULTFLAG__SECONDARY_DONE|RESULTFLAG__PARTS_ARE_KNOWN);

      if (!cmdin) {
         for (k=0; k<center_arity; k++)
            result_inner[k].result_flags.misc |= result_outer.result_flags.misc & (RESULTFLAG__DID_LAST_PART|RESULTFLAG__SECONDARY_DONE|RESULTFLAG__PARTS_ARE_KNOWN);
      }
   }

   int final_elongation = crossing ? begin_xconc_elongation : begin_outer_elongation;

   // Note: final_elongation might be -1 now, meaning that the people on the outside
   // cannot determine their elongation from the original setup.  Unless their
   // final setup is one that does not require knowing the value of final_elongation,
   // it is an error.
   // It might also have the "CONTROVERSIAL_CONC_ELONG" bit set, meaning that we should
   // raise a warning if we use it.

   // At this point, "final_elongation" actually has the INITIAL elongation of the
   // people who finished on the outside.  That is, if they went from a wave or diamond
   // to a 2x2, it has the elongation of their initial wave or diamond points.

   // Exception: if the schema was conc_6_2 or conc_6_2_tri, and the centers are in a bone6,
   // "final_elongation" has the elongation of that bone6.

   // The elongation bits in their setup tells how they "naturally" wanted to end,
   // based on the call they did, how it got divided up, whether it had the "parallel_conc_end"
   // flag on, etc.

   if (analyzer == schema_in_out_triple_squash ||
       analyzer == schema_in_out_triple ||
       analyzer == schema_sgl_in_out_triple_squash ||
       analyzer == schema_3x3_in_out_triple_squash ||
       analyzer == schema_4x4_in_out_triple_squash ||
       analyzer == schema_sgl_in_out_triple ||
       analyzer == schema_3x3_in_out_triple ||
       analyzer == schema_4x4_in_out_triple ||
       analyzer == schema_in_out_quad ||
       analyzer == schema_in_out_12mquad ||
       analyzer == schema_concentric_others) {
      if (fix_n_results(center_arity, -1, result_inner, rotstate, pointclip)) {
         result_inner[0].kind = nothing;
      }
      else if (!(rotstate & 0xF03)) fail("Sorry, can't do this orientation changer.");

      // Try to turn inhomogeneous diamond/wave combinations into all diamonds,
      // if the waves are missing their centers or ends.  If the resulting diamonds
      // are nonisotropic (elongated differently), that's OK.
      if (analyzer == schema_in_out_triple && ((final_elongation+1) & 2) != 0) {
         if (result_outer.kind == sdmd && result_inner[0].kind == s1x4) {
            if (!(result_inner[0].people[0].id1 | result_inner[0].people[2].id1 |
                  result_inner[1].people[0].id1 | result_inner[1].people[2].id1)) {
               expand::compress_setup(&s_1x4_dmd, &result_inner[0]);
               expand::compress_setup(&s_1x4_dmd, &result_inner[1]);
            }
            else if (!(result_inner[0].people[1].id1 | result_inner[0].people[3].id1 |
                       result_inner[1].people[1].id1 | result_inner[1].people[3].id1)) {
               result_inner[0].kind = sdmd;
               result_inner[1].kind = sdmd;
            }
         }
         else if (result_outer.kind == s1x4 && result_inner[0].kind == sdmd) {
            if (!(result_outer.people[1].id1 | result_outer.people[3].id1)) {
               result_outer.kind = sdmd;
            }
            else if (!(result_outer.people[0].id1 | result_outer.people[2].id1) &&
                     ((result_outer.rotation ^ final_elongation) & 1)) {
               // Occurs in t00t@1462.
               // The case we avoid with the rotation check above is bi02t@1045.
               expand::compress_setup(&s_1x4_dmd, &result_outer);
            }
            else if (!(result_inner[0].people[0].id1 | result_inner[0].people[2].id1 |
                       result_inner[1].people[0].id1 | result_inner[1].people[2].id1) &&
                     ((result_inner[0].rotation ^ final_elongation) & 1)) {
               expand::expand_setup(&s_1x4_dmd, &result_inner[0]);
               expand::expand_setup(&s_1x4_dmd, &result_inner[1]);
            }
         }
      }
   }

   // If the call was something like "ends detour", the concentricity info was left in the
   // cmd_misc_flags during the execution of the call, so we have to pick it up to make sure
   // that the necessary "demand" and "force" bits are honored.
   localmodsout1 |= (begin_outer.cmd.cmd_misc_flags & DFM1_CONCENTRICITY_FLAG_MASK);

   // If the outsides did "emulate", they stay on the same spots no matter what
   // anyone says.
   if (result_outer.result_flags.misc & RESULTFLAG__FORCE_SPOTS_ALWAYS) {
      localmodsout1 &= ~DFM1_CONCENTRICITY_FLAG_MASK;
      localmodsout1 |= DFM1_CONC_FORCE_SPOTS;
   }

   // Check whether the necessary "demand" conditions are met.  First, set "localmods1"
   // to the demand info for the call that the original ends did.  Where this comes from
   // depends on whether the schema is cross concentric.

   uint32 localmods1 = crossing ? localmodsin1 : localmodsout1;

   if (orig_outers_start_kind == s2x2) {
      if (localmods1 & DFM1_CONC_DEMAND_LINES) {
         // We make use of the fact that the setup, being a 2x2, is canonicalized.
         if (begin_outer_elongation <= 0 || begin_outer_elongation > 2 ||
             (orig_outers_start_dirs & (1 << 3*(begin_outer_elongation - 1))))
            fail("Outsides must be as if in lines at start of this call.");
      }

      if (localmods1 & DFM1_CONC_DEMAND_COLUMNS) {
         if (begin_outer_elongation <= 0 || begin_outer_elongation > 2 ||
             (orig_outers_start_dirs & (8 >> 3*(begin_outer_elongation - 1))))
            fail("Outsides must be as if in columns at start of this call.");
      }
   }

   // Now check whether there are any demands on the original centers.  The interpretation
   // of "lines" and "columns" is slightly different in this case.  We apply the test only if
   // the centers are in a 2x2, but we don't care about the outsides' setup, as long as it
   // has a measurable elongation.  If the outsides are also in a 2x2, so that the whole setup
   // is a 2x4, these tests will do just what they say -- they will check whether the centers
   // believe they are in lines or columns.  However, if the outsides are in a 1x4, so the
   // overall setup is a "rigger", we simply test the outsides' elongation.  In such a case
   // "demand lines" means "demand outsides lateral to me".

   // But we don't do this if we are inverting the centers and ends.

   if (!inverting) {
      uint32 mymods = crossing ? localmodsout1 : localmodsin1;

      if (orig_inners_start_kind == s2x2) {
         if (mymods & DFM1_CONC_DEMAND_LINES) {
            if (begin_outer_elongation <= 0 || begin_outer_elongation > 2 ||
                (orig_inners_start_dirs & (1 << 3*(begin_outer_elongation - 1))))
               fail("Centers must be as if in lines at start of this call.");
         }

         if (mymods & DFM1_CONC_DEMAND_COLUMNS) {
            if (begin_outer_elongation <= 0 || begin_outer_elongation > 2 ||
                (orig_inners_start_dirs & (8 >> 3*(begin_outer_elongation - 1))))
               fail("Centers must be as if in columns at start of this call.");
         }
      }
   }

   localmods1 = localmodsout1;

   final_outers_finish_dirs = 0;
   for (i=0; i<=attr::slimit(&result_outer); i++) {
      int q = result_outer.people[i].id1;
      final_outers_finish_dirs |= q;
      if (q) final_outers_finish_directions[(q >> 6) & 037] = q;
   }

   /* Now final_outers_finish_dirs tells whether outer peoples' orientations changed.
      This is only meaningful if outer setup is 2x2.  Note that, if the setups
      are 2x2's, canonicalization sets their rotation to zero, so the
      tbonetest quantities refer to absolute orientation. */

   // Deal with empty setups.

   if (result_outer.kind == nothing) {
      if (result_inner[0].kind == nothing) {
         result->kind = nothing;    // If everyone is a phantom, it's simple.
         clear_result_flags(result);
         return;
      }

      if (fix_empty_outers(ss->kind, final_outers_start_kind, localmods1,
                           crossing, begin_outer_elongation, center_arity,
                           analyzer, cmdin, cmdout, &begin_outer, &result_outer,
                           result_inner, result))
         goto getout;
   }
   else if (result_inner[0].kind == nothing) {
      if (fix_empty_inners(orig_inners_start_kind, center_arity,
                           analyzer, analyzer_result, &begin_inner[0],
                           &result_outer, result_inner, result))
         goto getout;
   }

   // The time has come to compute the elongation of the outsides in the final setup.
   // This gets complicated if the outsides' final setup is a 2x2.  Among the
   // procedures we could use are:
   //    (1) if the call is "checkpoint", go to spots with opposite elongation
   //    // from the original outsides' elongation.  This is the "Hodson checkpoint
   //    // rule", named after the caller who first used a consistent, methodical,
   //    // and universal rule for the checkpoint concept.
   //    (2) if the call is "concentric", use the Hodson rule if the original setup
   //    // was a 1x4 or diamond, or the "lines-to-lines, columns-to-columns" rule
   //    // if the original setup was a 2x2.
   //    (3) if we have various definition flags, such as "force_lines" or
   //    // "force_otherway", obey them.
   // We will use information from several sources in carrying out these rules.
   // The concentric concept will signify itself by turning on the "lines_lines"
   // flag.  The checkpoint concept will signify itself by turning on the
   // "force_otherway" flag.  The "parallel_conc_end" flag in the outsides' setup
   // indicates that, if "concentric" or "checkpoint" are NOT being used, the call
   // wants the outsides to maintain the same elongation as they had at the beginning.
   // This is what makes "ends hinge" and "ends recycle" do their respective
   // right things when called from a grand wave.

   // Default: the ends just keep their original elongation.  This will often
   // mean that they stay on their spots.

   // We will use both pieces of information to figure out how to elongate the outsides at
   // the conclusion of the call.  For example, if the word "concentric" was not spoken,
   // we will just use their "natural" elongation from the setup.  This is what makes
   // "ends hinge" work from a grand wave.  If the word "concentric" was spoken, their
   // natural elongation is discarded, and we will set them perpendicular to their
   // original 1x4 or diamond, using the value in "final_elongation"  If invocation
   // flags like "force lines" or "force columns" are present, we will use those.

   // When we are done, our final judgement will be put back into the variable
   // "final_elongation".

   if (analyzer != schema_in_out_triple &&
       analyzer != schema_in_out_triple_squash &&
       analyzer != schema_sgl_in_out_triple_squash &&
       analyzer != schema_3x3_in_out_triple_squash &&
       analyzer != schema_4x4_in_out_triple_squash &&
       analyzer != schema_sgl_in_out_triple &&
       analyzer != schema_3x3_in_out_triple &&
       analyzer != schema_4x4_in_out_triple &&
       analyzer != schema_in_out_quad &&
       analyzer != schema_in_out_12mquad &&
       analyzer != schema_conc_o &&
       analyzer != schema_conc_bar12 &&
       analyzer != schema_conc_bar16) {
      if (result_outer.kind == s2x2 || result_outer.kind == s2x3 || result_outer.kind == sdeep2x1dmd ||
          result_outer.kind == s2x4 || result_outer.kind == s_short6) {
         warning_index *concwarntable;

         if (final_outers_start_kind == s1x4) {
            // Watch for special case of cross concentric that some people may not agree with.
            if (orig_outers_start_kind == s1x4 &&
                ((begin_inner[0].rotation ^ begin_outer.rotation) & 1))
               concwarntable = concwarneeetable;
            else
               concwarntable = concwarn1x4table;
         }
         else
            concwarntable = concwarndmdtable;

         switch (final_outers_start_kind) {
         case s1x4: case sdmd:

            // Outers' call has gone from a 1x4 or diamond to a 2x2.  The rules are:
            // (1) The "force_columns" or "force_lines" flag in the invocation takes
            //    precedence over anything else.
            // (2) If the "concentric rules" flag is on (that flag is a euphemism for "the
            //    concentric or checkpoint concept is explicitly in use here"), we set the
            //    elongation perpendicular to the original 1x4 or diamond.
            // (3) If the "force_otherway" invocation flag is on, meaning the database
            //    really wants us to, we set the elongation perpendicular to the original
            //    1x4 or diamond.
            // (4) If the "force_spots" invocation flag is on, meaning the database
            //    really wants us to, we set the elongation parallel to the original
            //    1x4 or diamond.
            // (5) Otherwise, we set the elongation to the natural elongation that the
            //    people went to.  This uses the result of the "par_conc_end" flag for
            //    1x4/dmd -> 2x2 calls, or the manner in which the setup was divided
            //    for calls that were put together from 2-person calls, or whatever.
            //    (For 1x4->2x2 calls, the "par_conc_end" flag means the call prefers
            //    the SAME elongation in the resulting 2x2.)  The default, absent
            //    this flag, is to change the elongation.  In any case, the result
            //    of all that has been encoded into the elongation of the 2x2 setup
            //    that the people went to; we just have to obey.

            if ((DFM1_CONC_FORCE_LINES & localmods1) && final_outers_finish_dirs) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in lines - they are T-boned.");
               final_elongation = (final_outers_finish_dirs & 1) + 1;
            }
            else if ((DFM1_CONC_FORCE_COLUMNS & localmods1) && final_outers_finish_dirs) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in columns - they are T-boned.");
               final_elongation = ((~final_outers_finish_dirs) & 1) + 1;
            }
            else if (DFM1_CONC_CONCENTRIC_RULES & localmods1) {
               warn(concwarntable[crossing]);
               final_elongation ^= 3;
            }
            else if (DFM1_CONC_FORCE_OTHERWAY & localmods1) {
               // But we don't obey this flag unless we did the whole call.
               if (begin_outer.cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE)
                  final_elongation ^= 3;
            }
            else if (DFM1_CONC_FORCE_SPOTS & localmods1) {
               // It's OK the way it is.
            }
            else {
               // Get the elongation from the result setup, if possible.

               int newelong = result_outer.result_flags.misc & 3;

               if (newelong && !(DFM1_SUPPRESS_ELONGATION_WARNINGS & localmods1)) {
                  if ((final_elongation & (~CONTROVERSIAL_CONC_ELONG)) == newelong)
                     warn(concwarntable[2]);
                  else
                     warn(concwarntable[crossing]);
               }

               final_elongation = newelong;
            }

            break;

         case s_short6: case s_bone6: case s_spindle: case s1x6:
         case s2x4: case s2x3: case sdeep2x1dmd:
         case s_ntrgl6cw: case s_ntrgl6ccw: case s_ntrglcw: case s_ntrglccw:
         case s_nptrglcw: case s_nptrglccw: case s_nxtrglcw: case s_nxtrglccw:

            // In these cases we honor the "concentric rules" and "force_????" flags.

            // Otherwise, if the call did not split the setup, we take the elongation info
            // that it generates -- this is what makes "circulate" and "counter rotate"
            // behave differently, as they must.  But if the call split the setup into
            // parts, its own elongation info can't be meaningful, so we use our own
            // notion of how to elongate.  We also use our notion if the call didn't
            // provide one.

            if ((DFM1_CONC_CONCENTRIC_RULES | DFM1_CONC_FORCE_OTHERWAY) & localmods1) {
               if (((final_elongation+1) & 2) != 0)
                  final_elongation ^= 3;
            }
            else if (DFM1_CONC_FORCE_SPOTS & localmods1) {
               // It's OK the way it is.
            }
            else if ((result_outer.result_flags.misc & 3) != 0)
               final_elongation = (result_outer.result_flags.misc & 3);

            break;

         case s2x2:

            // If call went from 2x2 to 2x2, the rules are:
            //
            // First, check for "force_columns" or "force_lines" in the invocation.
            // This is not a property of the call that we did, but of the way its parent
            // (or the concept) invoked it.
            //
            // Second, check for "force_spots" or "force_otherway" in the invocation.
            // This is not a property of the call that we did, but of the way its parent
            // (or the concept) invoked it.
            //
            // Third, check for "lines_lines" in the invocation.  This is not a property
            // of the call that we did, but of the way its parent (or the concept) invoked it.
            // If the concept was "concentric", it will be on, of course.
            //
            // Finally, check the elongation bits in the result flags left over
            // from the call.  These tell whether to work to spots, or antispots,
            // or whatever, based what the call was, and whether it, or various
            // sequential parts of it, had the "parallel_conc_end" flag on.
            // If there are no elongation bits, we simply don't know what to do.
            //
            // Note that the "ends do thus-and-so" concept does NOT set the
            // lines_lines flag in the invocation, so we work to spots unless
            // the call says "parallel_conc_end".  Counter-rotate, for example,
            // says "parallel_conc_end", so it works to antispots.

            if ((DFM1_CONC_FORCE_LINES & localmods1) && final_outers_finish_dirs) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in lines - they are T-boned.");
               final_elongation = (final_outers_finish_dirs & 1) + 1;
            }
            else if ((DFM1_CONC_FORCE_COLUMNS & localmods1) && final_outers_finish_dirs) {
               if ((final_outers_finish_dirs & 011) == 011)
                  fail("Can't force ends to be as in columns - they are T-boned.");
               final_elongation = ((~final_outers_finish_dirs) & 1) + 1;
            }
            else if (DFM1_CONC_FORCE_OTHERWAY & localmods1) {
               if (((final_elongation-1) & (~1)) == 0) {

                  if (DFM1_ONLY_FORCE_ELONG_IF_EMPTY & localmods1) {
                     // If the setup is nonempty, and the setup seems to know
                     // what elongation it should have, pre-undo it.
                     if (final_outers_finish_dirs != 0 &&
                         result_outer.kind == s2x2 &&
                         (result_outer.result_flags.misc & 3) != 0 &&
                         ((final_elongation ^ result_outer.result_flags.misc) & 3) == 0) {
                        final_elongation ^= 3;
                     }
                  }
                  final_elongation ^= 3;
               }

            }
            else if (DFM1_CONC_FORCE_SPOTS & localmods1) {
               // It's OK the way it is.
            }
            else if (DFM1_CONC_CONCENTRIC_RULES & localmods1) {
               // Do "lines-to-lines / columns-to-columns".
               int new_elongation = -1;

               if (final_elongation < 0)
                  fail("People who finish on the outside can't tell whether they started in line-like or column-like orientation.");

               // If they are butterfly points, leave them there.
               if ((final_elongation & ~CONTROVERSIAL_CONC_ELONG) <= 2) {
                  // Otherwise, search among all possible people,
                  // including virtuals and phantoms.
                  for (i=0; i<32; i++) {
                     if (final_outers_finish_directions[i]) {
                        int t = ((final_outers_start_directions[i] ^
                                  final_outers_finish_directions[i] ^
                                  (final_elongation-1)) & 1) + 1;
                        if (t != new_elongation) {
                           if (new_elongation >= 0)
                              fail("Sorry, outsides would have to go to a 'pinwheel', can't handle that.");
                           new_elongation = t;
                        }
                     }
                  }

                  // Preserve the "controversial" bit.
                  final_elongation &= CONTROVERSIAL_CONC_ELONG;
                  final_elongation |= new_elongation;
               }
            }
            else
               final_elongation = (result_outer.result_flags.misc & 3);

            break;
         default:
            if (analyzer != schema_concentric_diamond_line &&
                analyzer != schema_conc_star &&
                analyzer != schema_ckpt_star &&
                analyzer != schema_conc_star12 &&
                analyzer != schema_conc_star16 &&
                analyzer != schema_concentric_others)
               fail("Don't recognize starting setup.");
         }
      }
   }

   // In general, if we sandwich a 1x4 between two parallel 1x4's,
   // with all 3 in parallel, we want a 3x4.  It may happen that
   // the outer setups are actually occupied only in the corners
   // ("butterfly spots"), and the center 1x4 is perpendicular to
   // the outer ones.  In that case, we still want a 3x4, and we
   // have to reformulate the outer 1x4's to go the other way
   // in order to get this result.  Doing this is necessary to get
   // consistent behavior if, in a butterfly, we say "center triple
   // box hinge" vs. "center triple box peel and trail".  (Note that
   // saying "center triple box" requested more precise positioning
   // than if we just said "centers" or "center 4".)  Cf. test t48t.

   if (analyzer == schema_in_out_triple &&
       center_arity == 2 &&
       cmdout &&
       result_inner[0].kind == s1x4 &&
       result_outer.kind == s1x4 &&
       begin_outer.kind == s2x2 &&
       (result_inner[0].rotation ^ result_outer.rotation) == 1 &&
       final_elongation == (1 << (result_outer.rotation & 1)) &&
       !(result_inner[0].people[1].id1 | result_inner[0].people[3].id1 |
         result_inner[1].people[1].id1 | result_inner[1].people[3].id1)) {
      swap_people(&result_inner[1], 0, 1);
      (void) copy_rot(&result_inner[1], 0, &result_inner[1], 2, 033);
      (void) copy_rot(&result_inner[1], 2, &result_inner[0], 2, 033);
      (void) copy_rot(&result_inner[0], 2, &result_inner[0], 0, 033);
      (void) copy_rot(&result_inner[0], 0, &result_inner[1], 1, 033);
      clear_person(&result_inner[1], 1);
      result_outer.rotation--;
      rotate_back++;
   }

   // Now lossage in "final_elongation" may have been repaired.  If it is still
   // negative, there may be trouble ahead.

   outer_inners[0] = result_outer;
   outer_inners[1] = result_inner[0];
   outer_inners[2] = result_inner[1];
   outer_inners[3] = result_inner[2];

   normalize_concentric(analyzer, center_arity, outer_inners,
                        final_elongation, matrix_concept, result);

   getout:

   // Tentatively clear the splitting info.

   result->result_flags.clear_split_info();

   if (analyzer == schema_concentric && center_arity == 1) {
      // Set the result fields to the minimum of the result fields of the
      // two components.  Start by setting to the outers, then bring in the inners.

      result->result_flags.copy_split_info(result_outer.result_flags);
      minimize_splitting_info(result, result_inner[0].result_flags);
   }

   result->rotation += rotate_back;
   reinstate_rotation(ss, result);
}



const merge_table::concmerge_thing merge_table::map_tgl4l  = {
   nothing, nothing, 0, 0, 0, 0x0, schema_by_array, s1x4, nothing,
   warn__none, 0, 0, {0, 1, -1, -1}, {0}};
const merge_table::concmerge_thing merge_table::map_tgl4b  = {
   nothing, nothing, 0, 0, 0, 0x0, schema_by_array, s2x2, nothing,
   warn__none, 0, 0, {-1, -1, 2, 3}, {0}};
const merge_table::concmerge_thing merge_table::map_2234b  = {
   nothing, nothing, 0, 0, 0, 0x0, schema_matrix, s4x4, nothing,
   warn__none, 0, 0, {15, 3, 7, 11}, {12, 13, 14, 0, -1, -1, 4, 5, 6, 8, -1, -1}};

merge_table::concmerge_thing *merge_table::merge_hash_tables[merge_table::NUM_MERGE_HASH_BUCKETS];


void merge_table::initialize()
{
   concmerge_thing *tabp;
   int i;

   for (i=0 ; i<NUM_MERGE_HASH_BUCKETS ; i++) merge_hash_tables[i] = (concmerge_thing *) 0;

   for (tabp = merge_init_table ; tabp->k1 != nothing ; tabp++) {
      uint32 hash_num = ((tabp->k1 + (5*tabp->k2)) * 25) & (NUM_MERGE_HASH_BUCKETS-1);

      tabp->next = merge_hash_tables[hash_num];
      merge_hash_tables[hash_num] = tabp;
   }
}


const merge_table::concmerge_thing *merge_table::lookup(setup_kind res1k,
                                                        setup_kind res2k,
                                                        unsigned int rotreject,
                                                        uint32 mask1,
                                                        uint32 mask2)
{
   uint32 hash_num = ((res1k + (5*res2k)) * 25) & (NUM_MERGE_HASH_BUCKETS-1);

   const concmerge_thing *result;

   for (result = merge_hash_tables[hash_num] ;
        result ;
        result = result->next) {
      if (res1k == result->k1 &&
          res2k == result->k2 &&
          (!(rotreject & result->rotmask)) &&
          (mask1 & result->m1) == 0 &&
          (mask2 & result->m2) == 0)
         return result;
   }
   return (const concmerge_thing *) 0;
}


// This overwrites its first argument setup.
extern void merge_setups(setup *ss, merge_action action, setup *result) THROW_DECL
{
   int i, j, r, rot;
   setup res2copy;
   setup outer_inners[2];
   setup *res1, *res2;
   uint32 rotmaskreject;
   const merge_table::concmerge_thing *the_map;
   uint32 mask1, mask2;
   int reinstatement_rotation;
   bool rose_from_dead = false;
   bool perp_2x4_1x8 = false;
   bool perp_2x4_ptp = false;
   normalize_action na = normalize_before_merge;

   res2copy = *result;
   res1 = ss;
   res2 = &res2copy;

   if (res1->kind == s_dead_concentric || res2->kind == s_dead_concentric)
      rose_from_dead = true;

   merge_action orig_action = action;

   if (action == merge_after_dyp) {
      action = merge_c1_phantom;
      na = normalize_before_isolate_not_too_strict;
   }
   else if (action == merge_c1_phantom_real)
      na = normalize_before_isolate_not_too_strict;
   else if (action == merge_strict_matrix)
      na = normalize_strict_matrix;
   else if (action == merge_without_gaps)
      na = normalize_after_disconnected;

   // If either incoming setup is big, opt for a 4x4 rather than C1 phantoms.
   // The test for this is, from a squared set, everyone phantom column wheel thru.
   // We want a 4x4.

   if ((res1->rotation ^ res2->rotation) & 1) {
      if ((res1->kind == s2x4 && res2->kind == s1x8) ||
          (res1->kind == s1x8 && res2->kind == s2x4))
         perp_2x4_1x8 = true;
      else if ((res1->kind == s2x4 && res2->kind == s_ptpd) ||
          (res1->kind == s_ptpd && res2->kind == s2x4))
         perp_2x4_ptp = true;  // Test for this is T-boned right wing follow to a diamond.
   }

   canonicalize_rotation(res1);    // Do we really need to do this before normalize_setup?
   normalize_setup(res1, na, res2->kind == nothing);
   canonicalize_rotation(res1);    // We definitely need to do it now --
                                   // a 2x2 might have been created.

 tryagain:

   canonicalize_rotation(res2);
   normalize_setup(res2, na, res1->kind == nothing);
   canonicalize_rotation(res2);

   // Canonicalize the setups according to their kind.  This is a bit sleazy, since
   // the enumeration order of setup kinds is supposed to be insignificant.  We depend in
   // general on small setups being before larger ones.  In particular, we seem to require:
   //    s2x2 < s2x4
   //    s2x2 < s2x6
   //    s2x2 < s1x8
   //    s1x4 < s1x8
   //    s1x4 < s2x4
   //    s2x4 < s_c1phan
   //    s2x4 < s2x6
   // You get the idea.

   if (res2->kind < res1->kind) {
      setup *temp = res2;
      res2 = res1;
      res1 = temp;
   }

   // If one of the setups was a "concentric" setup in which there are no ends,
   // we can still handle it.

   if ((res2->kind == s_normal_concentric && res2->outer.skind == nothing) ||
       res2->kind == s_dead_concentric) {
      res2->kind = res2->inner.skind;
      res2->rotation += res2->inner.srotation;
      goto tryagain;    // Need to recanonicalize setup order.
   }

   if (rose_from_dead && res1->kind == s1x4 && res2->kind == s4x4) {
      for (i=0, j=1, mask2 = 0; i<16; i++, j<<=1) {
         if (res2->people[i].id1) mask2 |= j;
      }

      if (res1->rotation == 0 && (mask2 & 0x8E8E) == 0) {
         expand::expand_setup(&s_4x4_4dmb, res2);
         goto tryagain;
      }
      else if (res1->rotation == 1 && (mask2 & 0xE8E8) == 0) {
         expand::expand_setup(&s_4x4_4dma, res2);
         goto tryagain;
      }
   }

   // If one of the setups was a "concentric" setup in which there are no centers,
   // merge concentrically.

   if (res2->kind == s_normal_concentric &&
       res2->inner.skind == nothing &&
       action != merge_strict_matrix) {
      res2->kind = res2->outer.skind;
      res2->rotation += res2->outer.srotation;
      canonicalize_rotation(res2);
      for (i=0 ; i<(MAX_PEOPLE/2) ; i++) swap_people(res2, i, i+(MAX_PEOPLE/2));
      int outer_elong = res2->concsetup_outer_elongation;
      outer_inners[0] = *res2;
      outer_inners[1] = *res1;
      normalize_concentric(schema_concentric, 1, outer_inners, outer_elong, 0, result);
      reinstatement_rotation = 0;
      goto final_getout;
   }

   reinstatement_rotation = res2->rotation;
   res1->rotation -= res2->rotation;
   res2->rotation = 0;
   result->rotation = 0;
   canonicalize_rotation(res1);
   canonicalize_rotation(res2);
   mask1 = little_endian_live_mask(res1);
   mask2 = little_endian_live_mask(res2);

   r = res1->rotation & 3;
   rot = r * 011;

   if (res1->kind == nothing) {
      *result = *res2;
      goto final_getout;
   }

   rotmaskreject = (1<<r);
   if (action != merge_without_gaps) rotmaskreject |= 0x10;
   if (action == merge_strict_matrix) rotmaskreject |= 0x20;
   if (!perp_2x4_1x8) rotmaskreject |= 0x40;
   if (!perp_2x4_ptp) rotmaskreject |= 0x200;
   if (action == merge_c1_phantom_real) rotmaskreject |= 0x80;
   if (orig_action == merge_after_dyp) rotmaskreject |= 0x100;

   the_map = merge_table::lookup(res1->kind,
                                 res2->kind,
                                 rotmaskreject,
                                 mask1,
                                 mask2);

   if (the_map) goto merge_concentric;

   if (res1->kind == s2x4 && res2->kind == s2x4 && (r&1)) {
      bool going_to_stars;
      bool going_to_o;
      bool go_to_4x4_anyway;
      bool conflict_at_4x4;
      bool action_suggests_4x4;

      conflict_at_4x4 = (
         (res2->people[1].id1 & res1->people[6].id1) |
         (res2->people[2].id1 & res1->people[1].id1) |
         (res2->people[5].id1 & res1->people[2].id1) |
         (res2->people[6].id1 & res1->people[5].id1)) != 0;

      going_to_stars =
         ((mask1 == 0x33) && (mask2 == 0xCC)) ||
         ((mask1 == 0xCC) && (mask2 == 0x33));
      going_to_o = ((mask1 | mask2) & 0x66) == 0;
      go_to_4x4_anyway = (mask1 == 0x99) || (mask2 == 0x99);

      // This stuff affects tests t33, t35, and vg06.  The point is that,
      // if we have two groups do calls in distorted setups out of a 4x4,
      // and each group goes to what amounts to a 1x4 with a shear in the
      // middle, then we want either stars or a C1 phantom, as needed.
      // but if the two sets of people are more random (e.g. the messy
      // couple up in vg06), we want a pinwheel (or whatever) on a 4x4.
      // Of course, if the action is merge_strict_matrix, we always go to
      // a 4x4 if possible.

      action_suggests_4x4 =
         action == merge_strict_matrix ||
         (action == merge_without_gaps &&
          ((mask1 != 0x33 && mask1 != 0xCC) || (mask2 != 0x33 && mask2 != 0xCC)));

      if ((action_suggests_4x4 && !going_to_stars && !conflict_at_4x4) ||
          go_to_4x4_anyway || going_to_o) {
         static const veryshort matrixmap1[8] = {14, 3, 7, 5, 6, 11, 15, 13};
         static const veryshort matrixmap2[8] = {10, 15, 3, 1, 2, 7, 11, 9};
         // Make this an instance of expand_setup.
         result->kind = s4x4;
         clear_people(result);
         scatter(result, res1, matrixmap1, 7, 011);
         install_scatter(result, 8, matrixmap2, res2, 0);
      }
      else {
         static const veryshort phanmap1[8] = {4, 6, 11, 9, 12, 14, 3, 1};
         static const veryshort phanmap2[8] = {0, 2, 7, 5, 8, 10, 15, 13};
         uint32 t1 = mask2 & 0x33;
         uint32 t2 = mask2 & 0xCC;
         uint32 t3 = mask1 & 0x33;
         uint32 t4 = mask1 & 0xCC;

         result->kind = s_c1phan;
         clear_people(result);
         scatter(result, res1, phanmap1, 7, 011);
         scatter(result, res2, phanmap2, 7, 0);

         // See if we have a "classical" C1 phantom setup, and give the appropriate warning.
         if (action != merge_c1_phantom_real) {
            if ((t1 | t3) == 0 || (t2 | t4) == 0)
               warn(warn__check_c1_phan);
            else if ((t1 | t4) == 0 || (t2 | t3) == 0)
               warn(warn__check_c1_stars);
            else
               warn(warn__check_gen_c1_stars);
         }
      }
      goto final_getout;
   }
   else if (res2->kind == s_trngl4 && res1->kind == s_trngl4 && r == 2 &&
            (mask1 & 0xC) == 0 && (mask2 & 0xC) == 0) {
      (void) copy_rot(res2, 0, res2, 0, 011);
      (void) copy_rot(res2, 1, res2, 1, 011);
      res2->rotation = 3;
      the_map = &merge_table::map_tgl4l;
      r = 1;
      goto merge_concentric;
   }
   else if (res2->kind == s_trngl4 && res1->kind == s_trngl4 && r == 2 &&
            (mask1 & 0x3) == 0 && (mask2 & 0x3) == 0) {
      (void) swap_people(res1, 0, 2);
      (void) swap_people(res1, 1, 3);
      res1->kind = s2x2;
      canonicalize_rotation(res1);
      (void) swap_people(res2, 0, 2);
      (void) swap_people(res2, 1, 3);
      the_map = &merge_table::map_tgl4b;
      r = 0;
      goto merge_concentric;
   }
   else if (res2->kind == s3x4 && res1->kind == s2x2 && ((mask2 & 06060) == 0)) {
      the_map = &merge_table::map_2234b;
      warn((mask2 & 06666) ? warn__check_4x4 : warn__check_butterfly);
      goto merge_concentric;
   }

   // The only remaining hope is that the setups match and we can blindly combine them.
   // Our 180 degree rotation wouldn't work for triangles.

   brute_force_merge(res1, res2, action >= merge_c1_phantom, result);
   goto final_getout;

 merge_concentric:

   if (the_map->swap_setups & 1) {
      setup *temp = res2;
      res2 = res1;
      res1 = temp;
      r = -r;
   }

   rot = (r & 3) * 011;

   int outer_elongation;

   if (the_map->swap_setups & 8)
      outer_elongation = 3;
   else
      outer_elongation = ((res2->rotation ^ (the_map->swap_setups >> 1)) & 1) + 1;

   warn(the_map->warning);

   if (the_map->conc_type == schema_nothing) {
      *result = *res2;
      goto merge_merge;
   }
   else if (the_map->conc_type == schema_by_array) {
      *result = *res2;
      result->kind = the_map->innerk;
      canonicalize_rotation(result);
      goto merge_merge;
   }
   else if (the_map->conc_type == schema_matrix) {
      *result = *res2;
      result->rotation += the_map->orot;
      result->kind = the_map->innerk;
      clear_people(result);
      scatter(result, res2, the_map->outermap, attr::slimit(res2),
              ((-the_map->orot) & 3) * 011);
      r -= the_map->orot;
      rot = (r & 3) * 011;
      goto merge_merge;
   }

   rot = 0;
   res2->kind = the_map->outerk;
   if (the_map->orot) {
      res2->rotation++;
      rot = 033;
   }
   outer_inners[0] = *res2;
   clear_people(&outer_inners[0]);
   gather(&outer_inners[0], res2, the_map->outermap, attr::slimit(res2), rot);
   canonicalize_rotation(&outer_inners[0]);

   rot = 0;
   res1->kind = the_map->innerk;
   if (the_map->irot) {
      res1->rotation++;
      rot = 033;
   }
   outer_inners[1] = *res1;
   clear_people(&outer_inners[1]);
   gather(&outer_inners[1], res1, the_map->innermap, attr::slimit(res1), rot);
   canonicalize_rotation(&outer_inners[1]);
   normalize_concentric(the_map->conc_type, 1, outer_inners, outer_elongation, 0, result);
   goto final_getout;

   merge_merge:

   {
      collision_collector CC(action >= merge_c1_phantom && !(the_map->swap_setups & 4));
      CC.note_prefilled_result(result);

      for (i=0; i<=attr::slimit(res1); i++) {
         if (res1->people[i].id1)
            CC.install_with_collision(result, the_map->innermap[i], res1, i, rot);
      }

      CC.fix_possible_collision(result);
   }

 final_getout:

   result->rotation += reinstatement_rotation;
   canonicalize_rotation(result);
}


extern void on_your_own_move(
   setup *ss,
   parse_block *parseptr,
   setup *result) THROW_DECL
{
   setup setup1, setup2, res1;
   setup outer_inners[2];

   if (ss->kind != s2x4) fail("Must have 2x4 setup for 'on your own'.");

   warning_info saved_warnings = configuration::save_warnings();

   setup1 = *ss;              /* Get outers only. */
   clear_person(&setup1, 1);
   clear_person(&setup1, 2);
   clear_person(&setup1, 5);
   clear_person(&setup1, 6);
   setup1.cmd = ss->cmd;
   setup1.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED | CMD_MISC__PHANTOMS;
   move(&setup1, false, &res1);

   setup2 = *ss;              /* Get inners only. */
   clear_person(&setup2, 0);
   clear_person(&setup2, 3);
   clear_person(&setup2, 4);
   clear_person(&setup2, 7);
   setup1.cmd = ss->cmd;
   setup2.cmd.cmd_misc_flags |= CMD_MISC__DISTORTED | CMD_MISC__PHANTOMS;
   setup2.cmd.parseptr = parseptr->subsidiary_root;
   move(&setup2, false, result);

   outer_inners[0] = res1;
   outer_inners[1] = *result;

   result->result_flags = get_multiple_parallel_resultflags(outer_inners, 2);
   merge_setups(&res1, merge_strict_matrix, result);

   // Shut off "superfluous phantom setups" warnings.

   configuration::clear_multiple_warnings(useless_phan_clw_warnings);
   configuration::set_multiple_warnings(saved_warnings);
}


// This places an assumption into a command structure, if the setup
// is fully occupied and supports that assumption.
void infer_assumption(setup *ss)
{
   int sizem1 = attr::slimit(ss);

   if (ss->cmd.cmd_assume.assumption == cr_none && sizem1 >= 0) {
      uint32 directions, livemask;
      get_directions(ss, directions, livemask);
      if (livemask == (uint32) (1<<((sizem1+1)<<1))-1) {
         assumption_thing tt;
         tt.assump_col = 0;
         tt.assump_both = 0;
         tt.assump_cast = 0;
         tt.assump_live = 0;
         tt.assump_negate = 0;

         if (ss->kind == s_qtag) {
            switch (directions) {
            case 0x58F2:
               tt.assump_both = 1;
               tt.assump_col = 4;
               tt.assumption = cr_jright;
               break;
            case 0x52F8:
               tt.assump_both = 1;
               tt.assump_col = 4;
               tt.assumption = cr_jleft;
               break;
            case 0xF852:
               tt.assump_both = 2;
               tt.assump_col = 4;
               tt.assumption = cr_jleft;
               break;
            case 0xF258:
               tt.assump_both = 2;
               tt.assump_col = 4;
               tt.assumption = cr_jright;
               break;
            case 0x5AF0:
               tt.assump_both = 1;
               tt.assump_col = 4;
               tt.assumption = cr_ijright;
               break;
            case 0x50FA:
               tt.assump_both = 1;
               tt.assump_col = 4;
               tt.assumption = cr_ijleft;
               break;
            case 0xFA50:
               tt.assump_both = 2;
               tt.assump_col = 4;
               tt.assumption = cr_ijleft;
               break;
            case 0xF05A:
               tt.assump_both = 2;
               tt.assump_col = 4;
               tt.assumption = cr_ijright;
               break;
            case 0x08A2:
               tt.assump_both = 1;
               tt.assumption = cr_jright;
               break;
            case 0x02A8:
               tt.assump_both = 1;
               tt.assumption = cr_jleft;
               break;
            case 0xA802:
               tt.assump_both = 2;
               tt.assumption = cr_jleft;
               break;
            case 0xA208:
               tt.assump_both = 2;
               tt.assumption = cr_jright;
               break;
            case 0x0AA0:
               tt.assump_both = 1;
               tt.assumption = cr_ijright;
               break;
            case 0x00AA:
               tt.assump_both = 1;
               tt.assumption = cr_ijleft;
               break;
            case 0xAA00:
               tt.assump_both = 2;
               tt.assumption = cr_ijleft;
               break;
            case 0xA00A:
               tt.assump_both = 2;
               tt.assumption = cr_ijright;
               break;
            default:
               return;
            }

            ss->cmd.cmd_assume = tt;
         }
      }
   }
}



/* We know that the setup has well-defined size, and that the conctable masks are good. */
extern void punt_centers_use_concept(setup *ss, setup *result) THROW_DECL
{
   int i, setupcount;
   setup the_setups[2], the_results[2];
   int sizem1 = attr::slimit(ss);
   uint32 cmd2word = ss->cmd.cmd_misc2_flags;
   int crossconc = (cmd2word & CMD_MISC2__ANY_WORK_INVERT) ? 1 : 0;
   bool doing_yoyo = false;
   bool doing_do_last_frac = false;
   uint32 numer;
   uint32 denom;
   parse_block *parseptrcopy;

   remove_z_distortion(ss);

   // Clear the stuff out of the cmd_misc2_flags word.

   ss->cmd.cmd_misc2_flags &=
      ~(0xFFF |
        CMD_MISC2__ANY_WORK_INVERT |
        CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG |
        CMD_MISC2__SAID_INVERT | CMD_MISC2__CENTRAL_SNAG | CMD_MISC2__INVERT_SNAG);

   // Before we separate the two groups from each other, it may be helpful
   // to infer an assumption from the whole setup.  That way, each group
   // will have a recollection of what the facing directions of the other group
   // were, which will lead to more realistic execution.  This helps test lg01t.

   infer_assumption(ss);

   the_setups[0] = *ss;              // designees
   the_setups[1] = *ss;              // non-designees

   uint32 ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_normal;

   if (cmd2word & (CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG)) {
      switch ((calldef_schema) (cmd2word & 0xFFF)) {
      case schema_concentric_2_6:
         ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_2_6;
         break;
      case schema_concentric_6_2:
         ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_6_2;
         break;
      case schema_concentric_diamonds:
         ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_ctr_dmd;
         break;
      }
   }

   for (i=sizem1; i>=0; i--) {
      clear_person(&the_setups[(ssmask & 1) ^ crossconc], i);
      ssmask >>= 1;
   }

   normalize_setup(&the_setups[0], normalize_before_isolated_call, false);
   normalize_setup(&the_setups[1], normalize_before_isolated_call, false);
   warning_info saved_warnings = configuration::save_warnings();

   /* Check for "someone work yoyo".  If call is sequential and yoyo is consumed by
      first part, then just do this stuff on the first part.  After that, merge
      the setups and finish the call normally. */

   if ((cmd2word & CMD_MISC2__ANY_WORK) &&
       ss->cmd.parseptr->concept->kind == concept_yoyo &&
       ss->cmd.parseptr->next->concept->kind == marker_end_of_list &&
       ss->cmd.parseptr->next->call->the_defn.schema == schema_sequential &&
       (ss->cmd.parseptr->next->call->the_defn.callflagsh & INHERITFLAG_YOYO) &&
       (ss->cmd.parseptr->next->call->the_defn.stuff.seq.defarray[0].modifiersh &
        INHERITFLAG_YOYO) &&
       ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE) {
      doing_yoyo = true;
      ss->cmd.cmd_frac_flags =
         CMD_FRAC_BREAKING_UP | CMD_FRAC_FORCE_VIS |
         CMD_FRAC_PART_BIT | CMD_FRAC_NULL_VALUE;
   }
   else if ((cmd2word & CMD_MISC2__ANY_WORK) &&
            ss->cmd.parseptr->concept->kind == concept_fractional &&
            ss->cmd.parseptr->concept->arg1 == 1 &&
            ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE) {
      uint32 fraction_to_finish = ss->cmd.parseptr->options.number_fields;
      numer = fraction_to_finish & 0xF;
      denom = (fraction_to_finish >> 4) & 0xF;
      doing_do_last_frac = true;
   }

   for (setupcount=0; setupcount<2; setupcount++) {
      setup *this_one = &the_setups[setupcount];
      this_one->cmd = ss->cmd;

      if (doing_do_last_frac) {
         if (setupcount == 0) {
            the_results[0] = *this_one;
            continue;                                 /* Designees do nothing. */
         }
         else
            /* Non-designees do first part only. */
            this_one->cmd.cmd_frac_flags =
               process_new_fractions(denom - numer, denom,
                                     this_one->cmd.cmd_frac_flags, 0);
      }

      this_one->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

      if (setupcount == 1 && (cmd2word & CMD_MISC2__ANY_WORK)) {
         parse_block *kstuff;
         uint32 njunk;
         parse_block **foop;

         parseptrcopy = really_skip_one_concept(ss->cmd.parseptr, kstuff, njunk, &foop);
         this_one->cmd.parseptr = *foop;

         if (kstuff->concept->kind == concept_supercall)
            fail("A concept is required.");
      }
      else if (setupcount == 0 &&
               ((cmd2word & CMD_MISC2__ANY_SNAG) ||
                (cmd2word &
                 (CMD_MISC2__SAID_INVERT|CMD_MISC2__CENTRAL_SNAG|CMD_MISC2__INVERT_SNAG)) ==
                CMD_MISC2__CENTRAL_SNAG)) {
         if (this_one->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE)
            this_one->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
         else if (this_one->cmd.cmd_frac_flags == (CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE))
            this_one->cmd.cmd_frac_flags = CMD_FRAC_LASTHALF_VALUE;
         else
            this_one->cmd.cmd_frac_flags |= CMD_FRAC_FIRSTHALF_ALL;
      }
      else if (setupcount == 1 &&
               (cmd2word &
                (CMD_MISC2__SAID_INVERT|CMD_MISC2__CENTRAL_SNAG|CMD_MISC2__INVERT_SNAG)) ==
               (CMD_MISC2__CENTRAL_SNAG|CMD_MISC2__INVERT_SNAG)) {
         if (this_one->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE)
            this_one->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
         else if (this_one->cmd.cmd_frac_flags == (CMD_FRAC_REVERSE | CMD_FRAC_NULL_VALUE))
            this_one->cmd.cmd_frac_flags = CMD_FRAC_LASTHALF_VALUE;
         else
            this_one->cmd.cmd_frac_flags |= CMD_FRAC_FIRSTHALF_ALL;
      }

      move(this_one, false, &the_results[setupcount]);
   }

   // Shut off "each 1x4" types of warnings -- they will arise spuriously
   // while the people do the calls in isolation.
   configuration::clear_multiple_warnings(dyp_each_warnings);
   configuration::set_multiple_warnings(saved_warnings);

   *result = the_results[0];

   if (!doing_do_last_frac) {
      if (!(result->result_flags.misc & RESULTFLAG__EXPAND_TO_2X3) &&
          (the_results[1].result_flags.misc & RESULTFLAG__EXPAND_TO_2X3) &&
          result->rotation == the_results[1].rotation) {
         if (result->kind == s2x4 &&
             the_results[1].kind == s2x4 &&
             ((result->people[1].id1 & the_results[1].people[1].id1) |
              (result->people[2].id1 & the_results[1].people[2].id1) |
              (result->people[5].id1 & the_results[1].people[5].id1) |
              (result->people[6].id1 & the_results[1].people[6].id1))) {
            /* We are doing something like "snag pair the line".  The centers
            didn't know which way the ends went, so they punted, moved into
            the center, and cleared the bit to say they really didn't know
            that this was right.  We now have the information, and it isn't
            good -- there is a collision.  We need to move the_results[0] outward. */
            swap_people(result, 0, 1);
            swap_people(result, 3, 2);
            swap_people(result, 4, 5);
            swap_people(result, 7, 6);
         }
         else if (result->kind == s2x4 &&
                  the_results[1].kind == s2x6 &&
                  ((result->people[1].id1 & the_results[1].people[2].id1) |
                   (result->people[2].id1 & the_results[1].people[3].id1) |
                   (result->people[5].id1 & the_results[1].people[8].id1) |
                   (result->people[6].id1 & the_results[1].people[9].id1))) {
            swap_people(result, 0, 1);
            swap_people(result, 3, 2);
            swap_people(result, 4, 5);
            swap_people(result, 7, 6);
         }
         else if (result->kind == s1x6 &&
                  the_results[1].kind == s2x4 &&
                  (((result->people[0].id1 | result->people[1].id1) &
                    (the_results[1].people[0].id1 | the_results[1].people[7].id1)) |
                   ((result->people[3].id1 | result->people[4].id1) &
                    (the_results[1].people[3].id1 | the_results[1].people[4].id1)))) {
            swap_people(result, 1, 2);
            swap_people(result, 0, 1);
            swap_people(result, 4, 5);
            swap_people(result, 3, 4);
         }
      }

      result->result_flags = get_multiple_parallel_resultflags(the_results, 2);
   }

   merge_setups(&the_results[1], merge_c1_phantom, result);

   if (doing_yoyo) {
      the_setups[0] = *result;
      the_setups[0].cmd = ss->cmd;    /* Restore original command stuff (though we clobbered fractionalization info). */
      the_setups[0].cmd.cmd_assume.assumption = cr_none;  /* Assumptions don't carry through. */
      the_setups[0].cmd.cmd_frac_flags = CMD_FRAC_BREAKING_UP | CMD_FRAC_FORCE_VIS | CMD_FRAC_CODE_FROMTOREV | (CMD_FRAC_PART_BIT*2) | CMD_FRAC_NULL_VALUE;
      the_setups[0].cmd.parseptr = parseptrcopy->next;      /* Skip over the concept. */
      uint32 finalresultflagsmisc = the_setups[0].result_flags.misc;
      move(&the_setups[0], false, result);
      finalresultflagsmisc |= result->result_flags.misc;
      normalize_setup(result, simple_normalize, false);
      result->result_flags.misc = finalresultflagsmisc & ~3;
   }
   else if (doing_do_last_frac) {
      the_setups[0] = *result;
      the_setups[0].cmd = ss->cmd;    /* Restore original command stuff. */
      the_setups[0].cmd.cmd_assume.assumption = cr_none;  // Assumptions don't carry through.
      the_setups[0].cmd.cmd_frac_flags =
         process_new_fractions(numer, denom,
                               the_setups[0].cmd.cmd_frac_flags, 1);
      the_setups[0].cmd.parseptr = parseptrcopy->next;    // Skip over the concept.
      uint32 finalresultflagsmisc = the_setups[0].result_flags.misc;
      move(&the_setups[0], false, result);
      finalresultflagsmisc |= result->result_flags.misc;
      normalize_setup(result, simple_normalize, false);
      result->result_flags.misc = finalresultflagsmisc & ~3;
   }
}



/* This does the various types of "so-and-so do this, perhaps while the others do that"
   concepts. */

/* indicator =
   selective_key_dyp                   -  <> do your part
   selective_key_dyp + others          -  <> do your part while the others ....
   selective_key_own + others          -  own the <>, .... by ....
   selective_key_plain                 -  <>
   selective_key_plain + others        -  <> while the others ....
   selective_key_disc_dist             -  <> disconnected   or   <> distorted
   selective_key_disc_dist + others    -  <> disconnected .... while the others ....
   selective_key_ignore                -  <> ignored
   selective_key_work_concept + others -  <> work <concept> (the others do the call,
                                           but without the concept)
   selective_key_lead_for_a            -  <> lead for a .... (as in cast a shadow
                                           from a promenade)
   selective_key_work_no_concentric    -  like selective_key_work_concept, but don't
                                           use concentric_move
   selective_key_snag_anyone + others  -  <> work 1/2, the others do the whole call
                                           (i.e. snag the <>)
   selective_key_plain_from_id_bits    -  like selective_key_plain, but don't use concentric_move


   arg2 = 0 - not doing distorted setup, this is the usual case, people work in the
                       actual setup that they have
   arg2 nonzero is used ONLY when indicator = selective_key_disc_dist:
      1/17/33 - distorted line -- used only with indicator = selective_key_disc_dist
                (16 bit means user said "diagonal", 32 bit means user said "offset")
      2/18/34 - distorted column (same as above)
      3/19/35 - distorted wave   (same as above)
      4       - distorted box
      5       - distorted diamond
      6       - Z */


extern void selective_move(
   setup *ss,
   parse_block *parseptr,
   selective_key indicator,
   int others,  // -1 - only selectees do the call, others can still roll
                //  0 - only selectees do the call, others can't roll
                //  1 - both sets
   uint32 arg2,
   uint32 override_selector,
   selector_kind selector_to_use,
   bool concentric_rules,
   setup *result) THROW_DECL
{
   setup_command cmd1thing, cmd2thing;
   setup_command *cmd2ptr;

   cmd1thing = ss->cmd;
   cmd2thing = ss->cmd;

   if (indicator == selective_key_work_concept) {
      // This is "<anyone> work <concept>".  Pick out the concept to be skipped
      // by the unselected people.

      parse_block *kstuff;
      uint32 njunk;
      parse_block **foop;

      (void) really_skip_one_concept(parseptr->next, kstuff, njunk, &foop);
      cmd2thing.parseptr = *foop;

      concept_kind k = kstuff->concept->kind;

      if (k == concept_supercall)
         fail("A concept is required.");

      // If this is a concept like "split phantom diamonds", we want to
      // refrain from the "centers" optimization.  That is, we don't want
      // to do this in terms of concentric_move.  Change the indicator
      // to indicate this.

      if (concept_table[k].concept_prop & CONCPROP__SET_PHANTOMS)
         indicator = selective_key_work_no_concentric;

      // Check for special case of "<anyone> work tandem", and change it to
      // "<anyone> are tandem".

      if ((kstuff->concept->kind == concept_tandem ||
           kstuff->concept->kind == concept_frac_tandem) &&
          kstuff->concept->arg1 == 0 &&
          kstuff->concept->arg2 == 0 &&
          (kstuff->concept->arg3 & ~0xF0) == 0 &&
          (kstuff->concept->arg4 == tandem_key_cpls ||
           kstuff->concept->arg4 == tandem_key_tand ||
           kstuff->concept->arg4 == tandem_key_cpls3 ||
           kstuff->concept->arg4 == tandem_key_tand3) &&
          ss->cmd.cmd_final_flags.test_heritbits(INHERITFLAG_SINGLE |
                                                 INHERITFLAG_MXNMASK |
                                                 INHERITFLAG_NXNMASK |
                                                 INHERITFLAG_TWISTED) == 0) {
         ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;
         ss->cmd.parseptr = cmd2thing.parseptr;  // Skip the tandem concept.
         tandem_couples_move(ss, 
                             parseptr->options.who,
                             kstuff->concept->arg3 >> 4,
                             kstuff->options.number_fields,
                             0,
                             (tandem_key) kstuff->concept->arg4,
                             0,
                             false,
                             result);
         return;
      }
      else if ((kstuff->concept->kind == concept_stable ||
                kstuff->concept->kind == concept_frac_stable) &&
               kstuff->concept->arg1 == 0) {
         ss->cmd.parseptr = cmd2thing.parseptr;  // Skip the stable concept.
         stable_move(ss,
                     kstuff->concept->arg2 != 0,
                     false,
                     kstuff->options.number_fields,
                     parseptr->options.who,
                     result);
         return;
      }
   }
   else if (indicator != selective_key_snag_anyone)
      cmd2thing.parseptr = parseptr->subsidiary_root;

   if (others > 0)
      cmd2ptr = &cmd2thing;
   else
      cmd2ptr = (setup_command *) 0;

   inner_selective_move(
      ss,
      &cmd1thing,
      cmd2ptr,
      indicator,
      others,
      arg2,
      false,
      override_selector,
      selector_to_use,
      0,
      (concentric_rules ? DFM1_CONC_CONCENTRIC_RULES : 0),
      result);
}


extern void inner_selective_move(
   setup *ss,
   setup_command *cmd1,
   setup_command *cmd2,
   selective_key indicator,
   int others,  // -1 - only selectees do the call, others can still roll
                //  0 - only selectees do the call, others can't roll
                //  1 - both sets
   uint32 arg2,
   bool demand_both_setups_live,
   uint32 override_selector,
   selector_kind selector_to_use,
   uint32 modsa1,
   uint32 modsb1,
   setup *result) THROW_DECL
{
   selector_kind saved_selector;
   int i, k;
   int setupcount;
   bool crossconc;
   uint32 livemask[2];
   uint32 j;
   uint32 rotstate, pointclip;
   warning_info saved_warnings;
   calldef_schema schema;
   setup the_setups[2], the_results[2];
   uint32 ssmask, llmask;
   int sizem1 = attr::slimit(ss);
   selective_key orig_indicator = indicator;
   normalize_action action = normalize_before_isolated_call;
   bool force_matrix_merge = false;

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__DO_NOT_EXECUTE) {
      clear_result_flags(result);
      result->kind = nothing;
      return;
   }

   if (indicator == selective_key_plain ||
       indicator == selective_key_plain_from_id_bits ||
       indicator == selective_key_plain_no_live_subsets) {
      action = normalize_before_merge;
      if (others <= 0 && sizem1 == 3) {
         switch (selector_to_use) {
         case selector_center2:
         case selector_verycenters:
            selector_to_use = selector_centers;
            break;
         case selector_center4:
         case selector_ctrdmd:
         case selector_ctr_1x4:
         case selector_center_wave:
         case selector_center_line:
         case selector_center_col:
         case selector_center_box:
            selector_to_use = selector_everyone;
            break;
         }
      }
   }

   saved_selector = current_options.who;
   current_options.who = selector_to_use;

   the_setups[0] = *ss;              /* designees */
   the_setups[1] = *ss;              /* non-designees */

   if (sizem1 < 0) fail("Can't identify people in this setup.");

   for (i=0, ssmask=0, llmask=0, livemask[0] = 0, livemask[1] = 0, j=1;
        i<=sizem1;
        i++, j<<=1) {
      ssmask <<= 1;
      llmask <<= 1;
      if (ss->people[i].id1) {
         int q = 0;

         /* We allow the designators "centers" and "ends" while in a 1x8, which
            would otherwise not be allowed.  The reason we don't allow it in
            general is that people would carelessly say "centers kickoff" in a 1x8
            when they should realy say "each 1x4, centers kickoff".  But we assume
            that they will not misuse the term here. */

         if (ss->kind == s1x8 && current_options.who == selector_centers) {
            if (i&2) q = 1;
         }
         else if (ss->kind == s1x8 && current_options.who == selector_ends) {
            if (!(i&2)) q = 1;
         }
         else if (override_selector) {
            if (override_selector & 0x80000000UL) {
               /* It is a headliner/sideliner mask. */
               if (override_selector & 011 & ss->people[i].id1) q = 1;
            }
            else {
               /* It is a little-endian mask of the people to select. */
               if (override_selector & j) q = 1;
            }
         }
         else if (selectp(ss, i))
            q = 1;

         // Indicator selective_key_ignore is like selective_key_disc_dist, but inverted.
         if (orig_indicator == selective_key_ignore) q ^= 1;

         ssmask |= q;
         llmask |= 1;
         clear_person(&the_setups[q], i);
         livemask[q^1] |= j;
      }
   }

   current_options.who = saved_selector;

   if (demand_both_setups_live && (livemask[0] == 0 ||livemask[1] == 0))
      fail("Can't do this call in this formation.");

   // If this is "ignored", we invert the selector
   // and change the operation to "disconnected".

   if (orig_indicator == selective_key_ignore) {
      selector_to_use = selector_list[selector_to_use].opposite;
      indicator = selective_key_disc_dist;
   }

   if (indicator == selective_key_disc_dist)
      action = normalize_to_2;

   // If the call is a "space-invader", and we are simply doing it
   // under a selector, and the call takes no further selector, and
   // "others" is <=0, that means the user simply said, for example,
   // "boys" and "press ahead" as two seperate actions, rather than
   // using the single call "boys press ahead".  In that case, we
   // simply do whatever "boys press ahead" would have done -- we have
   // the designated (or non-ignored) people do their part in a strict
   // matrix.  We don't do any of the clever stuff that this procedure
   // generally tries to do.  But if "others" is >0, things are more
   // complicated, and the designees have to interact with the
   // non-designees, so we don't take this shortcut.

   if (others <= 0 &&
       cmd1->parseptr &&
       cmd1->parseptr->concept &&
       cmd1->parseptr->concept->kind == marker_end_of_list &&
       cmd1->parseptr->call &&
       cmd1->parseptr->call->the_defn.schema == schema_matrix &&
       !(cmd1->parseptr->call->the_defn.callflagsf & CFLAGH__REQUIRES_SELECTOR) &&
       (orig_indicator == selective_key_ignore ||
        orig_indicator == selective_key_plain ||
        orig_indicator == selective_key_dyp)) {
      indicator = selective_key_dyp;
      action = normalize_strict_matrix;
      force_matrix_merge = true;
   }

   if (orig_indicator == selective_key_lead_for_a) {
      // This is "so-and-so lead for a cast a shadow".

      uint32 dirmask;
      const veryshort *map_prom;
      static const veryshort map_prom_1[16] =
      {6, 7, 1, 0, 2, 3, 5, 4, 011, 011, 022, 022, 011, 011, 022, 022};
      static const veryshort map_prom_2[16] =
      {4, 5, 7, 6, 0, 1, 3, 2, 022, 022, 033, 033, 022, 022, 033, 033};
      static const veryshort map_prom_3[16] =
      {0, 1, 3, 2, 4, 5, 7, 6, 000, 000, 011, 011, 000, 000, 011, 011};
      static const veryshort map_prom_4[16] =
      {6, 7, 1, 0, 2, 3, 5, 4, 011, 011, 022, 022, 011, 011, 022, 022};

      static const veryshort map_phan_1[16] =
      {13, 15, 3, 1, 5, 7, 11, 9, 000, 000, 011, 011, 000, 000, 011, 011};
      static const veryshort map_phan_2[16] =
      {9, 11, 15, 13, 1, 3, 7, 5, 011, 011, 022, 022, 011, 011, 022, 022};
      static const veryshort map_phan_3[16] =
      {8, 10, 14, 12, 0, 2, 6, 4, 022, 022, 033, 033, 022, 022, 033, 033};
      static const veryshort map_phan_4[16] =
      {12, 14, 2, 0, 4, 6, 10, 8, 011, 011, 022, 022, 011, 011, 022, 022};

      static const veryshort map_4x4_1[16] =
      {9, 11, 7, 5, 9, 11, 15, 13, 000, 000, 011, 011, 000, 000, 011, 011};
      static const veryshort map_4x4_2[16] =
      {13, 15, 11, 9, 5, 7, 11, 9, 011, 011, 022, 022, 011, 011, 022, 022};
      static const veryshort map_4x4_3[16] =
      {2, 7, 11, 6, 10, 15, 3, 14, 022, 022, 033, 033, 022, 022, 033, 033};
      static const veryshort map_4x4_4[16] =
      {6, 11, 15, 10, 14, 3, 7, 2, 011, 011, 022, 022, 011, 011, 022, 022};

      the_setups[0] = *ss;        /* Use this all over again. */
      clear_people(&the_setups[0]);
      the_setups[0].kind = s2x4;

      for (i=0, dirmask=0; i<=attr::slimit(ss); i++) {
         dirmask <<= 2;
         dirmask |= ss->people[i].id1 & 3;
      }

      if (ss->kind == s_crosswave || ss->kind == s_thar) {
         if (ssmask == 0xCC) {
            if (dirmask == 0xAF05) {
               map_prom = map_prom_1;
               goto got_map;
            }
            else if (dirmask == 0x05AF) {
               map_prom = map_prom_2;
               goto got_map;
            }
         }
         else if (ssmask == 0x33) {
            if (dirmask == 0xAF05) {
               map_prom = map_prom_3;
               the_setups[0].rotation++;
               goto got_map;
            }
            else if (dirmask == 0x05AF) {
               map_prom = map_prom_4;
               the_setups[0].rotation++;
               goto got_map;
            }
         }
      }
      else if (ss->kind == s_c1phan) {
         if (llmask == 0x5555) {
            if (ssmask == 0x5050) {
               if (dirmask == 0x33001122UL) {
                  map_prom = map_phan_1;
                  the_setups[0].rotation++;
                  goto got_map;
               }
               else if (dirmask == 0x11223300UL) {
                  map_prom = map_phan_2;
                  goto got_map;
               }
            }
            else if (ssmask == 0x0505) {
               if (dirmask == 0x33001122UL) {
                  map_prom = map_phan_2;
                  goto got_map;
               }
               else if (dirmask == 0x11223300UL) {
                  map_prom = map_phan_1;
                  the_setups[0].rotation++;
                  goto got_map;
               }
            }
         }
         else if (llmask == 0xAAAA) {
            if (ssmask == 0x0A0A) {
               if (dirmask == 0x88CC0044UL) {
                  map_prom = map_phan_3;
                  goto got_map;
               }
               else if (dirmask == 0x004488CCUL) {
                  map_prom = map_phan_4;
                  the_setups[0].rotation++;
                  goto got_map;
               }
            }
            else if (ssmask == 0xA0A0) {
               if (dirmask == 0x88CC0044UL) {
                  map_prom = map_phan_4;
                  the_setups[0].rotation++;
                  goto got_map;
               }
               else if (dirmask == 0x004488CCUL) {
                  map_prom = map_phan_3;
                  goto got_map;
               }
            }
         }
      }
      else if (ss->kind == s4x4) {
         if (llmask == 0x5555) {
            if (ssmask == 0x0505) {
               if (dirmask == 0x00112233UL) {
                  map_prom = map_4x4_1;
                  the_setups[0].rotation++;
                  goto got_map;
               }
               else if (dirmask == 0x22330011UL) {
                  map_prom = map_4x4_2;
                  goto got_map;
               }
            }
            else if (ssmask == 0x5050) {
               if (dirmask == 0x00112233UL) {
                  map_prom = map_4x4_2;
                  goto got_map;
               }
               else if (dirmask == 0x22330011UL) {
                  map_prom = map_4x4_1;
                  the_setups[0].rotation++;
                  goto got_map;
               }
            }
         }
         else if (llmask == 0x3333) {
            if (ssmask == 0x1212) {
               if (dirmask == 0x0304090EUL) {
                  map_prom = map_4x4_3;
                  goto got_map;
               }
               else if (dirmask == 0x090E0304UL) {
                  map_prom = map_4x4_4;
                  the_setups[0].rotation++;
                  goto got_map;
               }
            }
            else if (ssmask == 0x2121) {
               if (dirmask == 0x0304090EUL) {
                  map_prom = map_4x4_4;
                  the_setups[0].rotation++;
                  goto got_map;
               }
               else if (dirmask == 0x090E0304UL) {
                  map_prom = map_4x4_3;
                  goto got_map;
               }
            }
         }
      }

      fail("Can't do this in this setup or with these people designated.");

      got_map:

      for (i=0 ; i<8 ; i++)
         (void) copy_rot(&the_setups[0], i, ss, map_prom[i], map_prom[i+8]);

      update_id_bits(&the_setups[0]);
      move(&the_setups[0], false, result);
      result->result_flags.misc |= RESULTFLAG__IMPRECISE_ROT;
      return;
   }

   uint32 mask = ~(~0 << (sizem1+1));
   const ctr_end_mask_rec *ctr_end_masks_to_use = &dead_masks;

   switch (ss->kind) {
   case s3x4:
      if (llmask == 04747) {
         ctr_end_masks_to_use = &masks_for_3x4;
         mask = llmask;
      }
      break;
   case s3dmd:
      if (llmask == 07171) {
         ctr_end_masks_to_use = &masks_for_3dmd_ctr2;
         mask = llmask;
      }
      else if (llmask == 05353) {
         ctr_end_masks_to_use = &masks_for_3dmd_ctr4;
         mask = llmask;
      }
      break;
   case sbigh:
      if (llmask == 04747) {
         ctr_end_masks_to_use = &masks_for_bigh_ctr4;
         mask = llmask;
      }
      break;
   case s4x4:
      if (llmask == 0x9999) {
         ctr_end_masks_to_use = &masks_for_4x4;
         mask = llmask;
      }
      break;
   default:
      ctr_end_masks_to_use = &setup_attrs[ss->kind].setup_conc_masks;
   }

   /* See if the user requested "centers" (or the equivalent people under some other
      designation), and just do it with the concentric_move stuff if so.
      The concentric_move stuff is much more sophisticated about a lot of things than
      what we would otherwise do. */

   if (force_matrix_merge) {
   }
   else if (orig_indicator == selective_key_plain_from_id_bits) {
      // With this indicator, we don't require an exact match.
      if (sizem1 == 3)
         schema = schema_single_concentric;
      else
         schema = schema_concentric;

      if (ctr_end_masks_to_use != &dead_masks &&
          ssmask != 0 &&
          setup_attrs[ss->kind].setup_conc_masks.mask_normal != 0) {
         if ((ssmask & ~setup_attrs[ss->kind].setup_conc_masks.mask_normal) == 0)
            goto do_concentric_ctrs;
         else if ((ssmask & ~(mask-setup_attrs[ss->kind].setup_conc_masks.mask_normal)) == 0)
            goto do_concentric_ends;
      }
   }
   else if (orig_indicator == selective_key_plain ||
            orig_indicator == selective_key_plain_no_live_subsets ||
            orig_indicator == selective_key_ignore ||
            orig_indicator == selective_key_work_concept ||
            orig_indicator == selective_key_snag_anyone) {
      if (sizem1 == 3) {
         schema = schema_single_concentric;
      }
      else {
         if (ss->kind == s3x4) {             /* **** BUG  what a crock -- fix this right. */
            if (llmask == 03333) {         // Qtag occupation.
               schema = schema_concentric_6_2;
               if (selector_to_use == selector_center6)
                  goto do_concentric_ctrs;
               if (selector_to_use == selector_outer2 || selector_to_use == selector_veryends)
                  goto do_concentric_ends;
            }
            else if (llmask == 04747) {    // "H" occupation.
               schema = schema_concentric_2_6;
               if (selector_to_use == selector_center2 ||
                   selector_to_use == selector_verycenters)
                  goto do_concentric_ctrs;
               if (selector_to_use == selector_outer6)
                  goto do_concentric_ends;
            }
         }
         else if (ss->kind == s_d3x4) {    /* **** BUG  what a crock -- fix this right. */
            if (llmask == 03535) {    // Spindle occupation.
               schema = schema_concentric_6_2;
               if (selector_to_use == selector_center6)
                  goto do_concentric_ctrs;
               if (selector_to_use == selector_outer2 || selector_to_use == selector_veryends)
                  goto do_concentric_ends;
            }
         }
         else if (ss->kind == swqtag) {
            // Check for center 6 in "hollow" occupation.
            // Turn it into a "mini O", which we know how to handle.
            if (llmask == 0x3DE &&
                ssmask == 0x35A &&
                orig_indicator == selective_key_plain &&
                selector_to_use == selector_center6) {
                  action = normalize_before_isolated_call;
                  indicator = selective_key_mini_but_o;
                  arg2 = LOOKUP_MINI_O;
                  goto back_here;
            }
         }
         else if (ss->kind == sd2x5) {
            if (llmask == 0x37B || llmask == 0x3DE) {
               schema = schema_concentric;
               if (ssmask == 0x063 || ssmask == 0x0C6)
                  goto do_concentric_ctrs;
               else if (ssmask == 0x318)
                  goto do_concentric_ends;
            }
         }
         else if (ss->kind == s2x5) {
            if (llmask == 0x2F7 || llmask == 0x3BD) {
               schema = schema_concentric;
               if (ssmask == 0x18C || ssmask == 0x0C6)
                  goto do_concentric_ctrs;
               else if (ssmask == 0x231)
                  goto do_concentric_ends;
            }
         }
         else if (ss->kind != s2x7) {    // Default action, but not if 2x7.
            schema = schema_concentric_6_2;
            if (selector_to_use == selector_center6)
               goto do_concentric_ctrs;
            if (selector_to_use == selector_outer2 || selector_to_use == selector_veryends)
               goto do_concentric_ends;
            schema = schema_concentric_2_6;
            if (selector_to_use == selector_center2 || selector_to_use == selector_verycenters)
               goto do_concentric_ctrs;
            if (selector_to_use == selector_outer6)
               goto do_concentric_ends;
            schema = schema_concentric_diamond_line;
            if (ss->kind == s3x1dmd && selector_to_use == selector_ctr_1x4)
               goto do_concentric_ctrs;
         }

         schema = schema_concentric;
      }

      if (selector_to_use == selector_centers) goto do_concentric_ctrs;
      if (selector_to_use == selector_ends) goto do_concentric_ends;

      // This stuff is needed, with the livemask test, for rf01t and rd01t.

      if (others <= 0 && livemask[1] != 0) {
         switch (selector_to_use) {
         case selector_center2:
         case selector_verycenters:
            schema = schema_select_ctr2;
            action = normalize_to_2;
            goto back_here;
         case selector_center4:
         case selector_ctr_1x4:
         case selector_center_wave:
         case selector_center_line:
         case selector_center_col:
         case selector_center_box:
            schema = schema_select_ctr4;
            action = normalize_to_4;
            goto back_here;
         case selector_center6:
         case selector_ctr_1x6:
            schema = schema_select_ctr6;
            action = normalize_to_6;
            goto back_here;
         }
      }

      if (ctr_end_masks_to_use->mask_normal) {
         if (ssmask == ctr_end_masks_to_use->mask_normal) goto do_concentric_ctrs;
         else if (ssmask == mask - ctr_end_masks_to_use->mask_normal) goto do_concentric_ends;
      }

      if (ctr_end_masks_to_use->mask_6_2) {
         schema = schema_concentric_6_2;
         if (ssmask == ctr_end_masks_to_use->mask_6_2) goto do_concentric_ctrs;
         else if (ssmask == mask - ctr_end_masks_to_use->mask_6_2) goto do_concentric_ends;
      }

      // We don't do this if the selector is "outer 1x3's", because that
      // designates the 1x3's separately, not as all 6 people.
      if (ctr_end_masks_to_use->mask_2_6 && selector_to_use != selector_outer1x3s) {
         schema = schema_concentric_2_6;
         if (ssmask == ctr_end_masks_to_use->mask_2_6) goto do_concentric_ctrs;
         else if (ssmask == mask - ctr_end_masks_to_use->mask_2_6) goto do_concentric_ends;
      }

      if (ctr_end_masks_to_use->mask_ctr_dmd) {
         schema = schema_concentric_diamonds;
         if (ssmask == ctr_end_masks_to_use->mask_ctr_dmd) goto do_concentric_ctrs;
      }

      if (ss->kind == s3x1dmd) {
         schema = schema_concentric_6_2_line;
         if (ssmask == 0xEE) goto do_concentric_ctrs;
         else if (ssmask == 0x11) goto do_concentric_ends;
      }

      schema = schema_lateral_6;
      if (ss->kind == s_galaxy && ssmask == 0xDD) goto do_concentric_ctrs;
      schema = schema_vertical_6;
      if (ss->kind == s_galaxy && ssmask == 0x77) goto do_concentric_ctrs;
   }
   else if (orig_indicator == selective_key_disc_dist) {
      uint32 mask = ~(~0 << (sizem1+1));

      if (setup_attrs[ss->kind].setup_conc_masks.mask_ctr_dmd) {
         schema = schema_concentric_diamonds;
         if (ssmask == mask-setup_attrs[ss->kind].setup_conc_masks.mask_ctr_dmd)
            goto do_concentric_ends;
      }
      else if (ss->kind == s_galaxy && setup_attrs[ss->kind].setup_conc_masks.mask_normal) {
         schema = schema_concentric;
         if (ssmask == mask-setup_attrs[ss->kind].setup_conc_masks.mask_normal)
            goto do_concentric_ends;
      }
   }

back_here:

   normalize_setup(&the_setups[0], action, false);
   if (others > 0)
      normalize_setup(&the_setups[1], action, false);

   saved_warnings = configuration::save_warnings();

   // It will be helpful to have a mask of where the live people are.

   for (setupcount=0; setupcount<2; setupcount++) {
      for (i=0, j=1, livemask[setupcount] = 0;
           i<=attr::klimit(the_setups[setupcount].kind);
           i++, j<<=1) {
         if (the_setups[setupcount].people[i].id1) livemask[setupcount] |= j;
      }
   }

   // Iterate 1 or 2 times, depending on whether the "other" people do a call.

   for (setupcount=0; ; setupcount++) {

      // Not clear that this is really right.
      uint32 svd_number_fields = current_options.number_fields;
      int svd_num_numbers = current_options.howmanynumbers;
      uint32 thislivemask = livemask[setupcount];
      uint32 otherlivemask = livemask[setupcount^1];
      setup *this_one = &the_setups[setupcount];
      setup_kind kk = this_one->kind;
      setup_command *cmdp = (setupcount == 1) ? cmd2 : cmd1;

      process_number_insertion((setupcount == 1) ? modsb1 : modsa1);
      this_one->cmd = ss->cmd;
      this_one->cmd.parseptr = cmdp->parseptr;
      this_one->cmd.callspec = cmdp->callspec;
      this_one->cmd.cmd_final_flags = cmdp->cmd_final_flags;

      this_one->cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;

      if (indicator == selective_key_snag_anyone) {
         /* Snag the <anyone>. */
         if (setupcount == 0) {
            if (this_one->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
               fail("Can't do fractional \"snag\".");
            this_one->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
         }

         move(this_one, false, &the_results[setupcount]);
      }
      else if (indicator >= selective_key_plain &&
               indicator != selective_key_work_concept &&
               indicator != selective_key_work_no_concentric) {
         int lilcount;
         int numsetups;
         uint32 key;
         uint32 frot, vrot;
         setup lilsetup[4], lilresult[4];
         int map_scanner;
         bool feet_warning = false;
         bool doing_iden = false;

         // Check for special cases of no one or everyone.
         // For the "everyone" test, we include cases of "ignore" if the setup
         // is larger than 8 people.
         // The general reasoning is this:  Under normal circumstances "ignore"
         // means "disconnected with the other people", and "disconnected" means
         // "close the gaps".  So, under normal circumstances, we do not want to
         // pick out "everyone" here and just have them do the call.  But, if there
         // are more than 8 people, there are already gaps, and we can't close them.
         // So we interpret it as "other people just do the call where you stand"
         // in that case, which is what this code does.
         // The specific reason for this is to make
         // "ignore <anyone>, staggered waves of 3 <call>" work.

         if (indicator < selective_key_disc_dist ||
             orig_indicator == selective_key_ignore ||
             orig_indicator == selective_key_plain_from_id_bits) {
            if (thislivemask == 0) {
               // No one.
               the_results[setupcount].kind = nothing;
               clear_result_flags(&the_results[setupcount]);
               goto done_with_this_one;
            }
            else if (thislivemask == (uint32) ((1 << (attr::klimit(kk)+1)) - 1) ||
                     otherlivemask == 0 ||
                     orig_indicator == selective_key_plain_from_id_bits ||
                     (orig_indicator == selective_key_ignore &&
                      attr::klimit(kk) > 7) ||
                     ((schema == schema_select_ctr2 ||
                       schema == schema_select_ctr4 ||
                       schema == schema_select_ctr6) && others <= 0)) {
               // Everyone.
               update_id_bits(this_one);
               this_one->cmd.cmd_misc_flags &= ~CMD_MISC__VERIFY_MASK;
               switch (selector_to_use) {
               case selector_center_wave:
                  this_one->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;
                  break;
               case selector_center_line:
                  this_one->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_LINES;
                  break;
               case selector_center_col:
                  this_one->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_COLS;
                  break;
               }

               impose_assumption_and_move(this_one, &the_results[setupcount]);
               goto done_with_this_one;
            }
         }

         // By the way, if it is LOOKUP_DIST_DMD, there will be a "7" in the low bits
         // to make the "fixp2" stuff behave correctly.

         if (arg2 & LOOKUP_GEN_MASK)
            key = arg2 & LOOKUP_GEN_MASK;
         else if (arg2 != 0)
            key = LOOKUP_DIST_CLW;
         else if (orig_indicator == selective_key_ignore)
            key = LOOKUP_IGNORE;
         else if (indicator >= selective_key_disc_dist &&
                  indicator != selective_key_plain_from_id_bits)
            key = LOOKUP_DISC;
         else
            key = LOOKUP_NONE;

         const select::fixer *fixp = select::hash_lookup(kk, thislivemask, key, arg2, this_one);

         if (!fixp) {

            // These two have a looser livemask criterion.

            if (key & (LOOKUP_IGNORE|LOOKUP_DISC|LOOKUP_NONE)) {
               if (kk == s2x4 && the_setups[setupcount^1].kind == s2x4) {
                  if ((thislivemask & ~0x0F) == 0 && (otherlivemask & 0x0F) == 0)
                     fixp = select::fixer_ptr_table[select::fx_f2x4far];
                  else if ((thislivemask & ~0xF0) == 0  && (otherlivemask & 0xF0) == 0)
                     fixp = select::fixer_ptr_table[select::fx_f2x4near];
               }
               else if (kk == s4dmd && the_setups[setupcount^1].kind == s1x4) {
                  if ((thislivemask & ~0x0F0F) == 0) {
                     doing_iden = true;
                     fixp = select::fixer_ptr_table[select::fx_f4dmdiden];
                  }
               }
            }

            if (!fixp)
               fail("Can't do this with these people designated.");
         }

         numsetups = fixp->numsetups & 0xFF;
         map_scanner = 0;
         frot = fixp->rot;  // This stays fixed.
         vrot=frot>>2;      // This shifts down.

         for (lilcount=0; lilcount<numsetups; lilcount++) {
            uint32 tbone = 0;
            setup *lilss = &lilsetup[lilcount];
            setup *lilres = &lilresult[lilcount];

            lilss->cmd = this_one->cmd;
            lilss->cmd.prior_elongation_bits = fixp->prior_elong;
            lilss->cmd.prior_expire_bits = 0;
            lilss->cmd.cmd_assume.assumption = cr_none;
            if (indicator == selective_key_disc_dist)
               lilss->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
            lilss->kind = fixp->ink;
            lilss->rotation = 0;
            if (fixp->ink == s_trngl && lilcount == 1)
               lilss->rotation = 2;

            for (k=0;
                 k<=attr::klimit(fixp->ink);
                 k++,map_scanner++,vrot>>=2)
               tbone |= copy_rot(lilss, k, this_one, fixp->nonrot[map_scanner],
                                 011*((0-frot-vrot) & 3));

            // If we are picking out a distorted diamond from a 4x4, we can't tell
            // unambiguously how to do it unless all 4 people are facing in a
            // sensible way, that is, as if in real diamonds.  We did an extremely
            // cursory test to see which map to use, now we have to test whether
            // everyone agrees that the distortion direction was correct,
            // by checking whether they are in fact in real diamonds.

            if (kk == s4x4 && key == LOOKUP_DIST_DMD) {
               if (((lilss->people[0].id1) |
                    (~lilss->people[1].id1) |
                    (lilss->people[2].id1) |
                    (~lilss->people[3].id1)) & 1)
                  fail("Can't determine direction of diamond distortion.");
            }

            if ((arg2&7) == 2 && (tbone & 010) != 0) fail("There are no columns here.");
            if ((arg2&5) == 1 && (tbone & 001) != 0) fail("There are no lines here.");
            if ((arg2&7) == 3) lilss->cmd.cmd_misc_flags |= CMD_MISC__VERIFY_WAVES;

            if (indicator == selective_key_disc_dist)
               lilss->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;

            if (key == LOOKUP_Z) {
               if (thislivemask == 066)
                  lilss->cmd.cmd_misc2_flags |= CMD_MISC2__IN_Z_CW;
               else if (thislivemask == 033)
                  lilss->cmd.cmd_misc2_flags |= CMD_MISC2__IN_Z_CCW;
            }

            update_id_bits(lilss);
            impose_assumption_and_move(lilss, lilres);

            // There are a few cases in which we handle shape-changers in a distorted setup.
            // Print a warning if so.  Of course, it may not be allowed, in which case an
            // error will occur later.  But we won't give the warning if we went to a 4x4,
            // so we just set a flag.  See below.

            if (arg2 != 0 && key != LOOKUP_Z) {
               if (lilss->kind != lilres->kind ||
                   lilss->rotation != lilres->rotation)
                  feet_warning = true;
            }
         }

         if (doing_iden) {
            the_results[setupcount] = lilresult[0];
            goto fooble;
         }

         if (fix_n_results(numsetups, -1, lilresult, rotstate, pointclip)) goto lose;
         if (!(rotstate & 0xF03)) fail("Sorry, can't do this orientation changer.");

         clear_people(&the_results[setupcount]);

         the_results[setupcount].result_flags =
            get_multiple_parallel_resultflags(lilresult, numsetups);
         the_results[setupcount].result_flags.misc &= ~3;

         // If the call just kept a 2x2 in place, and they were the outsides, make
         // sure that the elongation is preserved.

         switch (this_one->kind) {
         case s2x2: case s_short6:
            the_results[setupcount].result_flags.misc |= this_one->cmd.prior_elongation_bits & 3;
            break;
         case s1x2: case s1x4: case sdmd:
            the_results[setupcount].result_flags.misc |= 2 - (this_one->rotation & 1);
            break;
         }

         the_results[setupcount].rotation = 0;

         // Ought to make this a method of "select".

         {
            const select::fixer *nextfixp = (const select::fixer *) 0;

            if (lilresult[0].kind == s2x2 && key == LOOKUP_Z) {
               if (lilresult[0].result_flags.misc & RESULTFLAG__DID_Z_COMPRESSMASK) {
                  expand::expand_setup((thislivemask == 066) ? &fix_cw : &fix_ccw,
                                       &lilresult[0]);
               }
            }

            if (numsetups == 2 && lilresult[0].kind == s_trngl) {
               if (((lilresult[0].rotation ^ lilresult[1].rotation) != 2) ||
                   ((lilresult[0].rotation & ~2) != 0))
                  goto lose;

               if (fixp == select::fixer_ptr_table[select::fx_f323]) {
                  the_results[setupcount].rotation = 3;
                  nextfixp = select::fixer_ptr_table[select::fx_specspindle];
               }
               else if (fixp == select::fixer_ptr_table[select::fx_f3x4outer]) {
                  nextfixp = select::fixer_ptr_table[select::fx_specfix3x40];
               }
               else if (fixp == select::fixer_ptr_table[select::fx_fdhrgl] ||
                        fixp == select::fixer_ptr_table[select::fx_specspindle] ||
                        fixp == select::fixer_ptr_table[select::fx_specfix3x40] ||
                        fixp == select::fixer_ptr_table[select::fx_specfix3x41]) {
                  nextfixp = fixp;
               }
               else
                  goto lose;

               if (lilresult[0].rotation != 0)
                  nextfixp = select::fixer_ptr_table[nextfixp->nextdmdrot];
            }
            else if (lilresult[0].rotation != 0) {
               if (attr::klimit(fixp->ink) == 5) {
                  if (lilresult[0].kind == s1x6 ||
                      (lilresult[0].kind == s1x4 && key == LOOKUP_Z))
                     nextfixp = select::fixer_ptr_table[fixp->next1x4rot];
                  else if (lilresult[0].kind == s2x3)
                     nextfixp = select::fixer_ptr_table[fixp->next2x2v];
                  else if (lilresult[0].kind == s_short6)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmdrot];
                  else if (lilresult[0].kind == s_bone6)
                     nextfixp = select::fixer_ptr_table[fixp->next1x2rot];
               }
               else if (attr::klimit(fixp->ink) == 7) {
                  if (lilresult[0].kind == s1x8)
                     nextfixp = select::fixer_ptr_table[fixp->next1x4rot];
                  else if (lilresult[0].kind == s2x4)
                     nextfixp = select::fixer_ptr_table[fixp->next2x2v];
               }
               else if (attr::klimit(fixp->ink) == 2) {
                  if (lilresult[0].kind == s1x3)
                     nextfixp = select::fixer_ptr_table[fixp->next1x2rot];
                  else if (lilresult[0].kind == s_trngl)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmdrot];
               }
               else if (attr::klimit(fixp->ink) == 11) {
                  if (lilresult[0].kind == s3x4)
                     nextfixp = select::fixer_ptr_table[fixp->next2x2v];
               }
               else {
                  if (lilresult[0].kind == s1x2)
                     nextfixp = select::fixer_ptr_table[fixp->next1x2rot];
                  else if (lilresult[0].kind == s1x4)
                     nextfixp = select::fixer_ptr_table[fixp->next1x4rot];
                  else if (lilresult[0].kind == sdmd)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmdrot];
               }

               if (!nextfixp) goto lose;

               if (((fixp->rot ^ nextfixp->rot) & 3) == 0) {
                  the_results[setupcount].rotation--;

                  if (fixp->numsetups & 0x100) {
                     the_results[setupcount].rotation += 2;

                     for (lilcount=0; lilcount<numsetups; lilcount++) {
                        lilresult[lilcount].rotation += 2;
                        canonicalize_rotation(&lilresult[lilcount]);
                     }
                  }
               }

               if ((nextfixp->rot & 3) == 0) {
                  for (lilcount=0; lilcount<numsetups; lilcount++) {
                     lilresult[lilcount].rotation += 2;
                     canonicalize_rotation(&lilresult[lilcount]);
                  }
               }
            }
            else {
               if (attr::klimit(fixp->ink) == 5) {
                  if (lilresult[0].kind == s1x6)
                     nextfixp = select::fixer_ptr_table[fixp->next1x4];
                  else if (lilresult[0].kind == s2x3)
                     nextfixp = select::fixer_ptr_table[fixp->next2x2];
                  else if (lilresult[0].kind == s_short6)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmd];
                  else if (lilresult[0].kind == s_bone6)
                     nextfixp = select::fixer_ptr_table[fixp->next1x2];
               }
               else if (attr::klimit(fixp->ink) == 7) {
                  if (lilresult[0].kind == s1x8)
                     nextfixp = select::fixer_ptr_table[fixp->next1x4];
                  else if (lilresult[0].kind == s2x4)
                     nextfixp = select::fixer_ptr_table[fixp->next2x2];
                  else if (lilresult[0].kind == s_qtag)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmd];
               }
               else if (attr::klimit(fixp->ink) == 2) {
                  if (lilresult[0].kind == s1x3)
                     nextfixp = select::fixer_ptr_table[fixp->next1x2];
                  else if (lilresult[0].kind == s_trngl)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmd];
               }
               else if (attr::klimit(fixp->ink) == 11) {
                  if (lilresult[0].kind == s3x4)
                     nextfixp = select::fixer_ptr_table[fixp->next2x2];
               }
               else {
                  if (lilresult[0].kind == s1x2)
                     nextfixp = select::fixer_ptr_table[fixp->next1x2];
                  else if (lilresult[0].kind == s1x4)
                     nextfixp = select::fixer_ptr_table[fixp->next1x4];
                  else if (lilresult[0].kind == sdmd)
                     nextfixp = select::fixer_ptr_table[fixp->nextdmd];
                  else if (lilresult[0].kind == s2x2) {
                     // If points are counter rotating in an hourglass, preserve
                     // elongation (by going to a 2x3) so they will go to the outside.
                     // But if they are doing it disconnected, always go to spots.
                     // See Application Note 3.
                     if (indicator != selective_key_disc_dist &&
                         (lilresult[0].result_flags.misc & 3) == 2)
                        nextfixp = select::fixer_ptr_table[fixp->next2x2v];
                     else
                        nextfixp = select::fixer_ptr_table[fixp->next2x2];
                  }
               }

               if (!nextfixp) {
                  if (lilresult[0].kind == fixp->ink)
                     nextfixp = fixp;
                  else goto lose;
               }

               if ((fixp->rot ^ nextfixp->rot) & 1) {
                  if (fixp->rot & 1) {
                     the_results[setupcount].rotation = 3;
                  }
                  else {
                     the_results[setupcount].rotation = 1;
                  }
                  for (lilcount=0; lilcount<numsetups; lilcount++) {
                     lilresult[lilcount].rotation += 2;
                     canonicalize_rotation(&lilresult[lilcount]);
                  }
               }
            }

            fixp = nextfixp;
         }

         the_results[setupcount].kind = fixp->outk;
         map_scanner = 0;
         frot = fixp->rot;  // This stays fixed.
         vrot=frot>>2;      // This shifts down.

         for (lilcount=0; lilcount<numsetups; lilcount++) {
            for (k=0;
                 k<=attr::klimit(lilresult[0].kind);
                 k++,map_scanner++,vrot>>=2)
               (void) copy_rot(&the_results[setupcount], fixp->nonrot[map_scanner],
                               &lilresult[lilcount], k, 011*((frot+vrot) & 3));
         }

         /* We only give the warning if they in fact went to spots.  Some of the maps
            create a result setup of 4x4.  For these maps, the dancers are not actually
            going to spots, but are going back to the quadrants the belong in.
            This is a "put the offset back" type of adjustment.  There don't seem to be
            any generally recognized words that one says to cause this to happen.
            We hope the dancers will know what to do. */

         if (feet_warning && fixp->outk != s4x4) warn(warn__adjust_to_feet);

       fooble:

         reinstate_rotation(this_one, &the_results[setupcount]);
      }
      else {
         uint32 doing_mystic = this_one->cmd.cmd_misc2_flags &
            (CMD_MISC2__CTR_END_MASK & ~CMD_MISC2__SAID_INVERT);
         bool mirror = false;

         this_one->cmd.cmd_misc2_flags &= ~doing_mystic;

         if (setupcount == 1)
            doing_mystic ^= (CMD_MISC2__INVERT_MYSTIC|CMD_MISC2__INVERT_SNAG);

         if ((doing_mystic & (CMD_MISC2__CENTRAL_MYSTIC|CMD_MISC2__INVERT_MYSTIC)) ==
             CMD_MISC2__CENTRAL_MYSTIC) {
            mirror = true;
         }
         else if ((doing_mystic & (CMD_MISC2__CENTRAL_SNAG|CMD_MISC2__INVERT_SNAG)) ==
                  CMD_MISC2__CENTRAL_SNAG) {
            if (this_one->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
               fail("Can't do fractional \"snag\".");
            this_one->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;
         }

         if (mirror) {
            mirror_this(this_one);
            this_one->cmd.cmd_misc_flags ^= CMD_MISC__EXPLICIT_MIRROR;
         }

         move(this_one, false, &the_results[setupcount]);
         if (mirror) mirror_this(&the_results[setupcount]);
      }

   done_with_this_one:

      current_options.number_fields = svd_number_fields;
      current_options.howmanynumbers = svd_num_numbers;

      if (setupcount >= others) break;
   }

   if (others <= 0) {      // The non-designees did nothing.
      the_results[1] = the_setups[1];
      // Give the people who didn't move the same result flags as those who did.
      // This is important for the "did last part" check.
      the_results[1].result_flags = the_results[0].result_flags;
      if (livemask[1] == 0) the_results[1].kind = nothing;

      // Strip out the roll bits -- people who didn't move can't roll.
      if (others == 0 && attr::klimit(the_results[1].kind) >= 0) {
         for (k=0; k<=attr::klimit(the_results[1].kind); k++) {
            if (the_results[1].people[k].id1)
               the_results[1].people[k].id1 =
                  (the_results[1].people[k].id1 & (~NROLL_MASK)) | ROLL_IS_M;
         }
      }
   }

   // Shut off "each 1x4" types of warnings -- they will arise spuriously while
   // the people do the calls in isolation.
   // Also, shut off "superfluous phantom setups" warnings if this was "own the
   // <anyone> or <anyone> do your part".
   configuration::clear_multiple_warnings(dyp_each_warnings);
   if (indicator < selective_key_plain_no_live_subsets)
      configuration::clear_multiple_warnings(useless_phan_clw_warnings);
   configuration::set_multiple_warnings(saved_warnings);

   *result = the_results[1];
   result->result_flags = get_multiple_parallel_resultflags(the_results, 2);

   /* For "own the <anyone>", we use strict matrix spots for the merge.
      Otherwise, we allow a little breathing. */

   {
      merge_action ma = merge_c1_phantom;

      if (force_matrix_merge) {
         // If we are doing an "anyone truck" type of call from
         // a C1 phantom, go back to same.
         if (ss->kind != s_c1phan)
            ma = merge_strict_matrix;
      }
      else if (indicator == selective_key_own ||
               indicator == selective_key_mini_but_o)
         ma = merge_strict_matrix;
      else if (indicator == selective_key_dyp ||
               indicator == selective_key_plain_from_id_bits)
         ma = merge_after_dyp;
      else if (the_results[0].kind == s1x6 &&
               the_results[1].kind == s3x6 &&
               the_results[0].rotation == the_results[1].rotation)
         ma = merge_strict_matrix;
      else if (the_results[0].kind == s3x4 &&
               the_results[1].kind == s3x6 &&
               the_results[0].rotation == the_results[1].rotation)
         ma = merge_strict_matrix;
      else if (indicator == selective_key_disc_dist)
         ma = merge_without_gaps;

      merge_setups(&the_results[0], ma, result);
   }

   return;

 lose: fail("Can't do this call with these people.");

 do_concentric_ends:
   crossconc = true;
   goto do_concentric_something;

 do_concentric_ctrs:
   crossconc = false;

   // If the setup is bigger than 8 people, concentric_move won't be able to handle it.
   // The only hope is that we are just having the center 2 or center 4 do something.
   // Except for 2x5 or d2x5.  In those cases we presume that the centers make a Z,
   // and that concentric_move will handle it.

   if (attr::slimit(ss) > 7 &&
       ss->kind != sd2x5 &&
       ss->kind != s2x5 &&
       indicator == selective_key_plain &&
       others <= 0)
      goto back_here;

 do_concentric_something:

   if (indicator == selective_key_work_concept || indicator == selective_key_snag_anyone)
      goto use_punt_stuff;

   ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

   // We may need to back out of this.  If either call is a space-invader,
   // we do *not* do the normal concentric formulation.  We have everyone do
   // their call in the actual setup, and merge them directly.  The test for this
   // is something like "centers truck while the ends reverse truck".
   saved_warnings = configuration::save_warnings();

   concentric_move(ss,
                   crossconc ? cmd2 : cmd1,
                   crossconc ? cmd1 : cmd2,
                   schema, modsa1, modsb1, true, ~0UL, result);

   if (result->result_flags.misc & RESULTFLAG__INVADED_SPACE) {
      configuration::restore_warnings(saved_warnings);
      goto back_here;
   }

   return;

   use_punt_stuff:

   switch (schema) {
      case schema_concentric_2_6:
         ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_2_6;
         break;
      case schema_concentric_6_2:
         ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_6_2;
         break;
      default:
         ssmask = setup_attrs[ss->kind].setup_conc_masks.mask_normal;
         break;
   }

   if (!ssmask) goto back_here;    /* We don't know how to find centers and ends. */

   if (ss->cmd.cmd_misc2_flags & (CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG))
      fail("Can't stack \"<anyone> work <concept>\" concepts.");

   ss->cmd.cmd_misc2_flags |=
      (indicator == selective_key_snag_anyone) ? CMD_MISC2__ANY_SNAG : CMD_MISC2__ANY_WORK;
   ss->cmd.cmd_misc2_flags &= ~(0xFFF | CMD_MISC2__ANY_WORK_INVERT);
   ss->cmd.cmd_misc2_flags |= (0xFFF & ((int) schema));
   if (crossconc) ss->cmd.cmd_misc2_flags |= CMD_MISC2__ANY_WORK_INVERT;

   /* If we aren't already looking for something,
      check whether to put on a new assumption. */

   if (ss->kind == s2x4 && ss->cmd.cmd_assume.assumption == cr_none) {
      assumption_thing restr;
      bool booljunk;

      restr.assumption = cr_wave_only;
      restr.assump_col = 0;
      restr.assump_both = 0;
      restr.assump_negate = 0;
      // Only do this if all are live -- otherwise we would be imposing
      // a stronger restriction on the whole setup than we ought to.
      restr.assump_live = 1;

      if (verify_restriction(ss, restr, false, &booljunk) == restriction_passes)
         ss->cmd.cmd_assume = restr;
      else {
         restr.assumption = cr_li_lo;
         restr.assump_col = 1;
         restr.assump_both = 1;
         if (verify_restriction(ss, restr, false, &booljunk) == restriction_passes)
         ss->cmd.cmd_assume = restr;
      }
   }

   move(ss, false, result);
}
