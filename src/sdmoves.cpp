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
   canonicalize_rotation
   reinstate_rotation
   divide_for_magic
   do_simple_split
   do_call_in_series
   drag_someone_and_move
   anchor_someone_and_move
   process_number_insertion
   gcd
   get_fraction_info
   fill_active_phantoms_and_move
   move_perhaps_with_active_phantoms
   impose_assumption_and_move
   move
*/

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#define SDLIB_API __declspec(dllexport)
#else
#define SDLIB_API
#endif

#include "sd.h"


extern void canonicalize_rotation(setup *result)
{
   result->rotation &= 3;

   if (result->kind == s1x1) {
      (void) copy_rot(result, 0, result, 0, (result->rotation) * 011);
      result->rotation = 0;
   }
   else if (result->kind == s_normal_concentric) {
      int i, save_rotation;

      if (     result->inner.skind == s_normal_concentric ||
               result->outer.skind == s_normal_concentric ||
               result->inner.skind == s_dead_concentric ||
               result->outer.skind == s_dead_concentric)
         fail("Recursive concentric?????.");

      save_rotation = result->rotation;

      result->kind = result->inner.skind;
      result->rotation += result->inner.srotation;
      canonicalize_rotation(result);    /* Sorry! */
      result->inner.srotation = result->rotation;

      result->kind = result->outer.skind;
      result->rotation = save_rotation + result->outer.srotation;
      for (i=0 ; i<12 ; i++) swap_people(result, i, i+12);
      canonicalize_rotation(result);    /* Sorrier! */
      for (i=0 ; i<12 ; i++) swap_people(result, i, i+12);
      result->outer.srotation = result->rotation;

      result->kind = s_normal_concentric;
      result->rotation = 0;
   }
   else if (result->kind == s_dead_concentric) {
      if (result->inner.skind == s_normal_concentric || result->inner.skind == s_dead_concentric)
         fail("Recursive concentric?????.");

      result->kind = result->inner.skind;
      result->rotation += result->inner.srotation;
      canonicalize_rotation(result);    /* Sorry! */
      result->inner.srotation = result->rotation;

      result->kind = s_dead_concentric;
      result->rotation = 0;
   }
   else if (setup_attrs[result->kind].four_way_symmetry) {
      /* The setup has 4-way symmetry.  We can canonicalize it so the
         result rotation is zero. */
      int i, rot, rot11, delta, bigd, i0, i1, i2, i3, j0, j1, j2, j3;
      personrec x0, x1, x2, x3;

      rot = result->rotation;
      if (rot == 0) return;
      rot11 = rot * 011;
      bigd = setup_attrs[result->kind].setup_limits + 1;
      delta = bigd >> 2;

      i0 = 1;
      i1 = i0 + delta;
      i2 = i1 + delta;
      i3 = i2 + delta;
      j0 = (rot-4)*delta+1;
      j1 = j0 + delta;
      j2 = j1 + delta;
      j3 = j2 + delta;
      for (i=0; i<delta; i++) {
         if ((--i0) < 0) i0 += bigd;
         if ((--i1) < 0) i1 += bigd;
         if ((--i2) < 0) i2 += bigd;
         if ((--i3) < 0) i3 += bigd;
         if ((--j0) < 0) j0 += bigd;
         if ((--j1) < 0) j1 += bigd;
         if ((--j2) < 0) j2 += bigd;
         if ((--j3) < 0) j3 += bigd;
         x0 = result->people[i0];
         x1 = result->people[i1];
         x2 = result->people[i2];
         x3 = result->people[i3];
         result->people[j0].id1 = rotperson(x0.id1, rot11);
         result->people[j0].id2 = x0.id2;
         result->people[j1].id1 = rotperson(x1.id1, rot11);
         result->people[j1].id2 = x1.id2;
         result->people[j2].id1 = rotperson(x2.id1, rot11);
         result->people[j2].id2 = x2.id2;
         result->people[j3].id1 = rotperson(x3.id1, rot11);
         result->people[j3].id2 = x3.id2;
      }

      result->rotation = 0;
   }
   else if (result->kind == s_trngl4) {
   }
   else if (result->kind == s1x3) {
      if (result->rotation & 2) {

         /* Must turn this setup upside-down. */

         swap_people(result, 0, 2);
         (void) copy_rot(result, 0, result, 0, 022);
         (void) copy_rot(result, 1, result, 1, 022);
         (void) copy_rot(result, 2, result, 2, 022);
      }
      result->rotation &= 1;
   }
   else if (((setup_attrs[result->kind].setup_limits & ~07776) == 1)) {
      /* We have a setup of an even number of people.  We know how to canonicalize
         this.  The resulting rotation should be 0 or 1. */

      if (result->rotation & 2) {

         /* Must turn this setup upside-down. */

         int i, offs;

         offs = (setup_attrs[result->kind].setup_limits+1) >> 1;     /* Half the setup size. */

         for (i=0; i<offs; i++) {
            swap_people(result, i, i+offs);
            (void) copy_rot(result, i, result, i, 022);
            (void) copy_rot(result, i+offs, result, i+offs, 022);
         }
      }
      result->rotation &= 1;
   }
}


extern void reinstate_rotation(setup *ss, setup *result)
{
   int globalrotation;

   switch (ss->kind) {
      case s_dead_concentric:
      case s_normal_concentric:
         globalrotation = 0;
         break;
      default:
         globalrotation = ss->rotation;
   }

   switch (result->kind) {
      case s_normal_concentric:
         result->inner.srotation += globalrotation;
         result->outer.srotation += globalrotation;
         break;
      case s_dead_concentric:
         result->inner.srotation += globalrotation;
         if ((globalrotation & 1) && ((result->concsetup_outer_elongation + 1) & 2))
            result->concsetup_outer_elongation ^= 3;
         break;
      case nothing:
         break;
      default:
         result->rotation += globalrotation;
         break;
   }

   /* If we turned by 90 degress, and the "split axis" bits are 01 or 10,
      we have to to swap those bits. */

   if (globalrotation & 1) {
      uint32 xbits = result->result_flags & RESULTFLAG__SPLIT_AXIS_XMASK;
      uint32 ybits = result->result_flags & RESULTFLAG__SPLIT_AXIS_YMASK;

      result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
      result->result_flags |= xbits << RESULTFLAG__SPLIT_AXIS_SEPARATION;
      result->result_flags |= ybits >> RESULTFLAG__SPLIT_AXIS_SEPARATION;
   }

   canonicalize_rotation(result);
}


extern long_boolean divide_for_magic(
   setup *ss,
   uint32 heritflags_to_check,
   setup *result)
{
   static expand_thing exp27  = {{1, 3, 4, 5, 7, 9, 10, 11}, 8, s2x4, s2x6, 0};
   static expand_thing exp72  = {{0, 1, 2, 4, 6, 7, 8, 10},  8, s2x4, s2x6, 0};
   static expand_thing exp13  = {{1, 3, 5, 7, 9, 11},        6, s2x3, s2x6, 0};
   static expand_thing exp31  = {{0, 2, 4, 6, 8, 10},        6, s2x3, s2x6, 0};
   static expand_thing expl13 = {{1, 3, 5, 7, 9, 11},        6, s1x6, s1x12, 0};
   static expand_thing expl31 = {{0, 2, 4, 6, 8, 10},        6, s1x6, s1x12, 0};
   static expand_thing exp35  = {{1, 2, 3, 5, 7, 8, 9, 11},  8, s2x4, s2x6, 0};
   static expand_thing exp53  = {{0, 2, 4, 5, 6, 8, 10, 11}, 8, s2x4, s2x6, 0};
   static expand_thing exp56  = {{0, 2, 3, 4, 6, 8, 9, 10},  8, s2x4, s2x6, 0};
   static expand_thing exp65  = {{0, 1, 3, 5, 6, 7, 9, 11},  8, s2x4, s2x6, 0};
   static expand_thing expg27 = {{1, 3, 5, 4, 7, 9, 11, 10}, 8, s1x8, s1x12, 0};
   static expand_thing expg72 = {{0, 1, 4, 2, 6, 7, 10, 8},  8, s1x8, s1x12, 0};
   static expand_thing expg35 = {{1, 2, 5, 3, 7, 8, 11, 9},  8, s1x8, s1x12, 0};
   static expand_thing expg56 = {{0, 2, 4, 3, 6, 8, 10, 9},  8, s1x8, s1x12, 0};

   static expand_thing expsp3 = {{1, 2, 3, 5, 7, 8, 9, 11}, 8, s_spindle, s_d3x4, 0};
   static expand_thing exp3d3 = {{10, 11, 0, -1, -1, 2, 4, 5, 6, -1, -1, 8},
                                 12, s3dmd, s_d3x4, 1};
   static expand_thing exp323 = {{10, 11, 0, 2, 4, 5, 6, 8}, 8, s_323, s_d3x4, 1};

   static expand_thing expb45 = {{0, 3, 5, 6, 9, 11},        6, s_bone6, s3x4, 0};
   static expand_thing expb32 = {{8, 10, 1, 2, 4, 7},        6, s_short6, s3x4, 1};
   static expand_thing expl53 = {{0, 2, 4, 5},               4, s1x4, s1x6, 0};
   static expand_thing expl35 = {{1, 2, 3, 5},               4, s1x4, s1x6, 0};
   static expand_thing expl27 = {{1, 5, 3, 4},               4, s1x4, s1x6, 0};
   static expand_thing expl72 = {{0, 1, 4, 2},               4, s1x4, s1x6, 0};

   warning_info saved_warnings;
   int i;
   map_thing *division_maps;
   uint32 resflags = 0;
   uint32 directions;
   uint32 livemask;

   uint32 heritflags_to_use = ss->cmd.cmd_final_flags.her8it;

   switch (ss->kind) {
   case s2x4:
      if (heritflags_to_check == INHERITFLAG_MAGIC) {
         /* "Magic" was specified.  Split it into 1x4's in the appropriate magical way. */
         division_maps = &map_2x4_magic;
         goto divide_us;
      }
      break;
   case s_qtag:
      /* Indicate that we have done a diamond division and the concept name needs to be changed. */
      resflags = RESULTFLAG__NEED_DIAMOND;

      if (heritflags_to_check == INHERITFLAG_MAGIC) {
         division_maps = &map_qtg_magic;
         goto divide_us;
      }
      else if (heritflags_to_check == INHERITFLAG_INTLK) {
         division_maps = &map_qtg_intlk;
         goto divide_us;
      }
      else if (heritflags_to_check == (INHERITFLAG_MAGIC | INHERITFLAG_INTLK)) {
         division_maps = &map_qtg_magic_intlk;
         goto divide_us;
      }
      else if (heritflags_to_check == INHERITFLAGMXNK_3X1 ||
               heritflags_to_check == INHERITFLAGMXNK_1X3) {
         expand_setup(&exp_qtg_3x4_stuff, ss);
         goto do_3x3;
      }
      break;
   case s_ptpd:
      resflags = RESULTFLAG__NEED_DIAMOND;

      if (heritflags_to_check == INHERITFLAG_MAGIC) {
         division_maps = &map_ptp_magic;
         goto divide_us;
      }
      else if (heritflags_to_check == INHERITFLAG_INTLK) {
         division_maps = &map_ptp_intlk;
         goto divide_us;
      }
      else if (heritflags_to_check == (INHERITFLAG_MAGIC | INHERITFLAG_INTLK)) {
         division_maps = &map_ptp_magic_intlk;
         goto divide_us;
      }
      break;
   }

   /* Now check for 1x3 types of stuff. */

   if (heritflags_to_check == INHERITFLAGMXNK_3X1 ||
       heritflags_to_check == INHERITFLAGMXNK_1X3 ||
       heritflags_to_check == INHERITFLAGMXNK_2X1 ||
       heritflags_to_check == INHERITFLAGMXNK_1X2) {
      directions = 0;
      livemask = 0;

      for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++) {
         uint32 p = ss->people[i].id1;
         livemask <<= 1;
         if (p) livemask |= 1;
         directions = (directions<<2) | (p&3);
      }

      if (heritflags_to_check == INHERITFLAGMXNK_3X1 ||
          heritflags_to_check == INHERITFLAGMXNK_1X3) {
         if (ss->kind == s2x4) {
            if (livemask != 0xFF) return FALSE;

            /* These are independent of whether we said "1x3" or "3x1". */
            if (directions == 0x2A80 || directions == 0x802A ||
                directions == 0x7FD5 || directions == 0xD57F) {
               expand_setup(&exp27, ss);
               goto do_3x3;
            }
            else if (directions == 0xA802 || directions == 0x02A8 ||
                     directions == 0xFD57 || directions == 0x57FD) {
               expand_setup(&exp72, ss);
               goto do_3x3;
            }
            else if (directions == 0x208A || directions == 0x8A20 ||
                     directions == 0x75DF || directions == 0xDF75) {
               expand_setup(&exp35, ss);
               goto do_3x3;
            }
            else if (directions == 0xA208 || directions == 0x08A2 ||
                     directions == 0xF75D || directions == 0x5DF7) {
               expand_setup(&exp56, ss);
               goto do_3x3;
            }

            /* These are specific to "1x3" or "3x1". */
            if (directions == 0x55FF || directions == 0x00AA) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_3X1) ? &exp27 : &exp72,
                            ss);
               goto do_3x3;
            }
            else if (directions == 0xFF55 || directions == 0xAA00) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_3X1) ? &exp72 : &exp27,
                            ss);
               goto do_3x3;
            }
         }
         else if (ss->kind == s1x8) {
            if (livemask != 0xFF) return FALSE;

            /* These are independent of whether we said "1x3" or "3x1". */
            if (directions == 0x2A80 || directions == 0x802A ||
                directions == 0x7FD5 || directions == 0xD57F) {
               expand_setup(&expg27, ss);
               goto do_3x3;
            }
            else if (directions == 0xA208 || directions == 0x08A2 ||
                     directions == 0xF75D || directions == 0x5DF7) {
               expand_setup(&expg72, ss);
               goto do_3x3;
            }
            else if (directions == 0x208A || directions == 0x8A20 ||
                     directions == 0x75DF || directions == 0xDF75) {
               expand_setup(&expg35, ss);
               goto do_3x3;
            }
            else if (directions == 0xA802 || directions == 0x02A8 ||
                     directions == 0xFD57 || directions == 0x57FD) {
               expand_setup(&expg56, ss);
               goto do_3x3;
            }

            /* These are specific to "1x3" or "3x1". */
            if (directions == 0x55FF || directions == 0x00AA) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_3X1) ? &expg27 : &expg72,
                            ss);
               goto do_3x3;
            }
            else if (directions == 0xFF55 || directions == 0xAA00) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_3X1) ? &expg72 : &expg27,
                            ss);
               goto do_3x3;
            }
         }
         else if (ss->kind == s3x4) {
            if (livemask == 03333 || livemask == 04747 ||
                livemask == 02753 || livemask == 05327) goto do_3x3;
         }
         else if (ss->kind == s2x6) {
            if (livemask == 02727 || livemask == 07272 ||
                livemask == 02277 || livemask == 07722) goto do_3x3;
         }
         else if (ss->kind == s_spindle) {
            if (livemask == 0xFF) {
               expand_setup(&expsp3, ss);
               goto do_3x3;
            }
         }
         else if (ss->kind == s3dmd) {
            if (livemask == 07171) {
               expand_setup(&exp3d3, ss);
               goto do_3x3;
            }
         }
         else if (ss->kind == s_323) {
            if (livemask == 0xFF) {
               expand_setup(&exp323, ss);
               goto do_3x3;
            }
         }
         else if (ss->kind == s2x3) {
            if (livemask == 027 || livemask == 072) goto do_3x3;
         }
      }
      else if (heritflags_to_check == INHERITFLAGMXNK_2X1 ||
               heritflags_to_check == INHERITFLAGMXNK_1X2) {
         if (ss->kind == s2x3) {
            if (livemask != 077) return FALSE;

            /* These are independent of whether we said "1x2" or "2x1". */
            if (directions == 01240 || directions == 04012 ||
                directions == 03765 || directions == 06537) {
               expand_setup(&exp13, ss);
               goto do_3x3;
            }
            else if (directions == 05002 || directions == 00250 ||
                     directions == 07527 || directions == 02775) {
               expand_setup(&exp31, ss);
               goto do_3x3;
            }
            /* These are specific to "1x2" or "2x1". */
            if (directions == 02577 || directions == 00052) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_2X1) ? &exp13 : &exp31,
                            ss);
               goto do_3x3;
            }
            else if (directions == 07725 || directions == 05200) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_2X1) ? &exp31 : &exp13,
                            ss);
               goto do_3x3;
            }
         }
         else if (ss->kind == s1x6) {
            if (livemask != 077) return FALSE;

            /* These are independent of whether we said "1x2" or "2x1". */
            if (directions == 01240 || directions == 04012 ||
                directions == 03765 || directions == 06537) {
               expand_setup(&expl13, ss);
               goto do_3x3;
            }
            else if (directions == 05002 || directions == 00250 ||
                     directions == 07527 || directions == 02775) {
               expand_setup(&expl31, ss);
               goto do_3x3;
            }
            /* These are specific to "1x2" or "2x1". */
            if (directions == 02577 || directions == 00052) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_2X1) ? &expl13 : &expl31,
                            ss);
               goto do_3x3;
            }
            else if (directions == 07725 || directions == 05200) {
               expand_setup((heritflags_to_check == INHERITFLAGMXNK_2X1) ? &expl31 : &expl13,
                            ss);
               goto do_3x3;
            }
         }
      }
   }

   return FALSE;

 divide_us:

   ss->cmd.cmd_final_flags.her8it = heritflags_to_use & ~heritflags_to_check;
   divided_setup_move(ss, division_maps, phantest_ok, TRUE, result);

   /* Since more concepts follow the magic and/or interlocked stuff, we can't
      allow the concept to be just "magic" etc.  We have to change it to
      "magic diamond, ..."  Otherwise, things could come out sounding like
      "magic diamond as couples quarter right" when we should really be saying
      "magic diamond, diamond as couples quarter right".  Therefore, we are going
      to do something seriously hokey: we are going to change the concept descriptor
      to one whose name has the extra "diamond" word.  We do this by marking the
      setupflags word in the result. */

   result->result_flags |= resflags;
   return TRUE;

 do_3x3:

   ss->cmd.cmd_final_flags.her8it =
      (heritflags_to_use & ~(INHERITFLAG_MXNMASK|INHERITFLAG_NXNMASK)) |
      INHERITFLAGNXNK_3X3;

   if (setup_attrs[ss->kind].setup_limits > 7)
      ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_12_MATRIX;

   saved_warnings = history[history_ptr+1].warnings;
   impose_assumption_and_move(ss, result);

   /* Shut off "each 2x3" types of warnings -- they will arise spuriously while
      the people do the calls in isolation. */
   for (i=0 ; i<WARNING_WORDS ; i++) {
      history[history_ptr+1].warnings.bits[i] &= ~dyp_each_warnings.bits[i];
      history[history_ptr+1].warnings.bits[i] |= saved_warnings.bits[i];
   }

   livemask = 0;

   for (i=0; i<=setup_attrs[result->kind].setup_limits; i++) {
      livemask <<= 1;
      if (result->people[i].id1) livemask |= 1;
   }

   if (result->kind == s2x6) {
      if (livemask == 02525) {
         compress_setup(&exp13, result);
      }
      else if (livemask == 05252) {
         compress_setup(&exp31, result);
      }
      else if (livemask == 02727) {
         compress_setup(&exp27, result);
      }
      else if (livemask == 07272) {
         compress_setup(&exp72, result);
      }
      else if (livemask == 03535) {
         compress_setup(&exp35, result);
      }
      else if (livemask == 05353) {
         compress_setup(&exp53, result);
      }
      else if (livemask == 05656) {
         compress_setup(&exp56, result);
      }
      else if (livemask == 06565) {
         compress_setup(&exp65, result);
      }
   }
   else if (result->kind == s1x12) {
      if (livemask == 02525) {
         compress_setup(&expl13, result);
      }
      else if (livemask == 05252) {
         compress_setup(&expl31, result);
      }
      else if (livemask == 02727) {
         compress_setup(&expg27, result);
         result->result_flags |= RESULTFLAG__VERY_ENDS_ODD;
      }
      else if (livemask == 07272) {
         compress_setup(&expg72, result);
         result->result_flags |= RESULTFLAG__VERY_CTRS_ODD;
      }
      else if (livemask == 03535) {
         compress_setup(&expg35, result);
      }
      else if (livemask == 05656) {
         compress_setup(&expg56, result);
      }
   }
   else if (result->kind == s3x4) {
      if (livemask == 03333) {
         compress_setup(&exp_qtg_3x4_stuff, result);
      }
      else if (livemask == 04545) {
         compress_setup(&expb45, result);
      }
      else if (livemask == 03232) {
         compress_setup(&expb32, result);
      }
   }
   else if (result->kind == s_d3x4) {
      if (livemask == 05353) {
         compress_setup(&exp323, result);
      }
      else if (livemask == 03535) {
         compress_setup(&expsp3, result);
      }
   }
   else if (result->kind == s1x6) {
      if (livemask == 027) {
         compress_setup(&expl27, result);
      }
      else if (livemask == 072) {
         compress_setup(&expl72, result);
      }
      else if (livemask == 035) {
         compress_setup(&expl35, result);
      }
      else if (livemask == 053) {
         compress_setup(&expl53, result);
      }
   }

   return TRUE;
}


extern long_boolean do_simple_split(
   setup *ss,
   uint32 prefer_1x4,   /* 1 means prefer 1x4, 2 means this is 1x8 and do not recompute id. */
   setup *result)
{
   uint32 mapcode;
   long_boolean recompute_id = TRUE;

   ss->cmd.cmd_misc_flags &= ~CMD_MISC__MUST_SPLIT_MASK;

   switch (ss->kind) {
   case s2x4:
      mapcode = prefer_1x4 ?
         MAPCODE(s1x4,2,MPKIND__SPLIT,1) : MAPCODE(s2x2,2,MPKIND__SPLIT,0);
      break;
   case s2x6:
      if (prefer_1x4 == 1) {
         if (ss->people[0].id1 & ss->people[1].id1 &
             ss->people[2].id1 & ss->people[3].id1 &
             ss->people[6].id1 & ss->people[7].id1 &
             ss->people[8].id1 & ss->people[9].id1) {
            mapcode = MAPCODE(s1x4,2,MPKIND__OFFS_L_HALF, 1);
            break;
         }
         else if (ss->people[2].id1 & ss->people[3].id1 &
                  ss->people[4].id1 & ss->people[5].id1 &
                  ss->people[8].id1 & ss->people[9].id1 &
                  ss->people[10].id1 & ss->people[11].id1) {
            mapcode = MAPCODE(s1x4,2,MPKIND__OFFS_R_HALF, 1);
            break;
         }
      }

      fail("Can't figure out how to split this call.");
   case s1x8:
      mapcode = MAPCODE(s1x4,2,MPKIND__SPLIT,0);
      if (prefer_1x4 == 2) recompute_id = FALSE;
      break;
   case s_qtag:
      mapcode = MAPCODE(sdmd,2,MPKIND__SPLIT,1);
      break;
   case s_ptpd:
      mapcode = MAPCODE(sdmd,2,MPKIND__SPLIT,0);
      break;
   default:
      return TRUE;
   }

   new_divided_setup_move(ss, mapcode, phantest_ok, recompute_id, result);
   return FALSE;
}


/* Do a call as part of a series.  The result overwrites the incoming setup.

   This handles the task of "turning around" the setup from the result left by
   one call to "move" to the input required by the next call.

   The result_flags are carried from one part to another through the "result_flags"
   word of the setup, even though (or especially because) that word is undefined
   prior to doing a call.  The client must seed that word with RESULTFLAG__SPLIT_AXIS_MASK
   (or whatever other bits are desired) prior to the beginning of the series, and not
   clobber it between parts.  At the end of the series, it will contain all the required
   result flags for the whole operation, including the final elongation.

   The elongation bits of the setup (in the "cmd.prior_elongation_bits" field)
   must be correct at the start of the series, and must preserved from one call
   to the next.  Since the entire "cmd" field of the setup typically gets changed
   for each call in the series (in order to do different calls) this will typically
   require specific action by the client. */

extern void do_call_in_series(
   setup *sss,
   long_boolean dont_enforce_consistent_split,
   long_boolean roll_transparent,
   long_boolean normalize,
   long_boolean qtfudged)
{
   setup tempsetup;
   setup qqqq = *sss;
   uint32 current_elongation = 0;
   uint32 saved_result_flags = qqqq.result_flags;
   uint32 save_expire;

   qqqq.cmd.prior_expire_bits |= saved_result_flags & (RESULTFLAG__YOYO_FINISHED |
                                                       RESULTFLAG__TWISTED_FINISHED |
                                                       RESULTFLAG__SPLIT_FINISHED);

   /* Check for a concept that will need to be re-evaluated under "twice".
      The test for this is [waves] initially twice initially once removed
      hinge the lock.  We want the "once removed" to be re-evaluated. */

   if (qqqq.cmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS &&
       (qqqq.cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) == CMD_FRAC_CODE_ONLY) {
      if (qqqq.cmd.restrained_concept->concept->kind == concept_twice) {
         qqqq.cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;
         qqqq.cmd.restrained_fraction = qqqq.cmd.cmd_frac_flags;
         qqqq.cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
      }
   }

   /* If we are forcing a split, and an earlier call in the series has responded
      to that split by returning an unequivocal splitting axis (indicated by
      one field being zero and the other nonzero), we continue to split
      along the same axis. */

   if ((qqqq.cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK) &&
       !dont_enforce_consistent_split &&
       (saved_result_flags & RESULTFLAG__SPLIT_AXIS_FIELDMASK) &&    /* at least one field
                                                                        is nonzero */
       (
        !(saved_result_flags & RESULTFLAG__SPLIT_AXIS_XMASK) ||   /* but one field or
                                                                     the other is zero */
        !(saved_result_flags & RESULTFLAG__SPLIT_AXIS_YMASK))) {
      int prefer_1x4;
      uint32 save_split = qqqq.cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK;

      if (saved_result_flags & RESULTFLAG__SPLIT_AXIS_XMASK)
         prefer_1x4 = qqqq.rotation & 1;
      else
         prefer_1x4 = (~qqqq.rotation) & 1;

      if (prefer_1x4 && qqqq.kind != s2x4 && qqqq.kind != s2x6)
         fail("Can't figure out how to split multiple part call.");

      if (do_simple_split(&qqqq, prefer_1x4, &tempsetup))
         fail("Can't figure out how to split this multiple part call.");

      qqqq.cmd.cmd_misc_flags |= save_split;  /* Put it back in. */
   }
   else
      move(&qqqq, qtfudged, &tempsetup);

   if (tempsetup.kind == s2x2) {
      switch (sss->kind) {
         case s1x4: case sdmd: case s2x2:
            current_elongation = tempsetup.result_flags & 3;

            /* If just the ends were doing this, and it had some
               "force_lines" type of directive, honor same. */

            if (qqqq.cmd.cmd_misc_flags & CMD_MISC__DOING_ENDS) {
               if (sss->kind == s2x2 &&
                   (qqqq.cmd.cmd_misc_flags &
                    (DFM1_CONC_DEMAND_LINES | DFM1_CONC_DEMAND_COLUMNS))) {

                  int i;
                  uint32 tb = 0;

                  for (i=0; i<4; i++) tb |= sss->people[i].id1;
                  if ((tb & 011) == 011) fail("Can't figure out where people started.");

                  if (qqqq.cmd.cmd_misc_flags & DFM1_CONC_DEMAND_LINES)
                     tb++;

                  if (current_elongation & ((tb & 1) + 1))
                     fail("Ends aren't starting in required position.");
               }

               if (qqqq.cmd.cmd_misc_flags & (DFM1_CONC_FORCE_LINES | DFM1_CONC_FORCE_COLUMNS)) {
                  int i;
                  uint32 tb = 0;

                  for (i=0; i<4; i++) tb |= tempsetup.people[i].id1;
                  if ((tb & 011) == 011) fail("Can't figure out where people finish.");

                  if (qqqq.cmd.cmd_misc_flags & DFM1_CONC_FORCE_COLUMNS)
                     tb++;

                  current_elongation = (tb & 1) + 1;
               }
               else if (qqqq.cmd.cmd_misc_flags & (DFM1_CONC_FORCE_OTHERWAY)) {
                  if ((sss->cmd.prior_elongation_bits+1) & 2)
                     current_elongation = (sss->cmd.prior_elongation_bits & 3) ^ 3;
               }
               else if (qqqq.cmd.cmd_misc_flags & (DFM1_CONC_FORCE_SPOTS)) {
                  if ((sss->cmd.prior_elongation_bits+1) & 2)
                     current_elongation = sss->cmd.prior_elongation_bits & 3;
               }

               qqqq.cmd.cmd_misc_flags &= ~(DFM1_CONC_FORCE_SPOTS|DFM1_CONC_FORCE_OTHERWAY|
                                            DFM1_CONC_FORCE_LINES | DFM1_CONC_FORCE_COLUMNS|
                                            DFM1_CONC_DEMAND_LINES | DFM1_CONC_DEMAND_COLUMNS);
            }

            break;

         /* Otherwise (perhaps the setup was a star) we have no idea how to elongate the setup. */
         default:
            break;
      }
   }

   /* If this call is "roll transparent", restore roll info from before the call for those people
      that are marked as roll-neutral. */

   if (roll_transparent) {
      /* Can only do this if we understand the setups. */
      if ((setup_attrs[tempsetup.kind].setup_limits >= 0) && (setup_attrs[sss->kind].setup_limits >= 0)) {
         int u, v;

         for (u=0; u<=setup_attrs[tempsetup.kind].setup_limits; u++) {
            if (tempsetup.people[u].id1 & ROLLBITM) {
               /* This person is roll-neutral.  Reinstate his original roll info, by
                  searching for him in the starting setup. */
               tempsetup.people[u].id1 &= ~ROLL_MASK;
               for (v=0; v<=setup_attrs[sss->kind].setup_limits; v++) {
                  if (((tempsetup.people[u].id1 ^ sss->people[v].id1) & XPID_MASK) == 0)
                     tempsetup.people[u].id1 |= (sss->people[v].id1 & ROLL_MASK);
               }
            }
         }
      }
   }

   save_expire = sss->cmd.prior_expire_bits;
   *sss = tempsetup;
   sss->cmd.prior_expire_bits = save_expire;
   sss->cmd.cmd_misc_flags = qqqq.cmd.cmd_misc_flags;   /* But pick these up from the call. */
   sss->cmd.cmd_misc_flags &= ~CMD_MISC__DISTORTED;   // But not this one!

   /* Remove outboard phantoms.
      It used to be that normalize_setup was not called
      here.  It was found that we couldn't do things like, from a suitable offset wave,
      [triple line 1/2 flip] back to a wave, that is, start offset and finish normally.
      So this has been added.  However, there may have been a reason for not normalizing.
      If any problems are found, it may be that a flag needs to be added to seqdef calls
      saying whether to remove outboard phantoms after each part. */

   /* But we *DON'T* remove the phantoms if "explicit matrix" is on, as of version 29.45.
      This is to make "1x12 matrix grand swing thru" work from a simple grand wave.
      The intention is to add outboard phantoms and then do the call in a gigantic way.
      We added two outboard phantoms on each end, then did the swing half, resulting
      in another 1x12 with only the center 8 spots occupied.  If outboard phantoms are
      removed, the effect of the explicit "1x12 matrix" modifier is lost.  The "grand
      1/2 by the left" is done on the 1x8.  So we leave the phantoms in.  Actually,
      making this call operate properly required a database change as well.  We had to
      add 1x12 and 1x16 starting setups for "1/2 by the left or right".  Otherwise,
      on "1x12 matrix grand swing thru", it would just do it in each 1x6.  Also, we
      don't want the warning "Do the call in each 1x6" to be given, though it is
      admittedly better to have the program warn us when it's about to do something
      wrong than to do the wrong thing silently.
      We also don't normalize if a "12 matrix" or "16 matrix" modifier is given, so that
      "12 matrix grand swing thru" will work also, along with "1x12 matrix grand swing thru".
      Only "1x12 matrix" turns on CMD_MISC__EXPLICIT_MATRIX.  Plain "12 matrix will appear
      in the "new_final_concepts" word. */

   if (normalize) normalize_setup(sss, simple_normalize);

   canonicalize_rotation(sss);

   /* To be safe, we should take away the "did last part" bit for the second call,
      but we are fairly sure it won't be on. */

   sss->cmd.prior_elongation_bits = current_elongation;

   /* The computation of the new result flags is complex, since we are trying
      to accumulate results over a series of calls.  To that end, the incoming contents
      of the "result_flags" word are used.   (Normally, this word is undefined
      on input to "move", but "do_call_in_series" takes it is the accumulated
      stuff so far.  It follows that it must be "seeded" at the start of the series.)

      The manipulations that produce the final value differ among the various bits and fields:

         *  The RESULTFLAG__DID_LAST_PART, RESULTFLAG__SECONDARY_DONE, and
                  RESULTFLAG__PARTS_ARE_KNOWN bits:
            Set to the result of the part we just did -- discard incoming values.

         *  The RESULTFLAG__SPLIT_AXIS_MASK bits:
            Set to the AND of the incoming values and what we just got --
            this has the effect of correctly keeping track of the splitting through all
            parts of the call.

         *  The low 2 bits, which have the elongation:
            Set to "current_elongation", which typically has the incoming stuff,
            but it actually a little bit subtle.  Put this in the
            "cmd.prior_elongation_bits" field also.

         *  The other bits:
            Set to the OR of the incoming values and what we just got --
            this is believed to be the correct way to accumulate these bits.

      It follows from this that the correct way to "seed" the result_flags word
      at the start of a series is by initializing it to RESULTFLAG__SPLIT_AXIS_FIELDMASK. */

   sss->result_flags = ((
                  (saved_result_flags &
                   ~(RESULTFLAG__DID_LAST_PART|RESULTFLAG__SECONDARY_DONE|
                     RESULTFLAG__PARTS_ARE_KNOWN)) |
                  tempsetup.result_flags) &
                        ~(3|RESULTFLAG__SPLIT_AXIS_FIELDMASK)) | current_elongation;

   /* We now have all but the SPLIT_AXIS stuff. */

   sss->result_flags |= tempsetup.result_flags & RESULTFLAG__SPLIT_AXIS_FIELDMASK;

   /* Now have tempsetup, need to minimize with saved_result_flags. */
   minimize_splitting_info(sss, saved_result_flags);
}



typedef struct gloop {
   int x;                  /* This person's coordinates, calibrated so that a matrix */
   int y;                  /*   position cooresponds to an increase by 4. */
   int nicex;              /* This person's "nice" coordinates, used for calculating jay walk legality. */
   int nicey;
   uint32 id1;             /* The actual person, for error printing. */
   long_boolean sel;       /* True if this person is selected.  (False if selectors not in use.) */
   long_boolean done;      /* Used for loop control on each pass */
   long_boolean realdone;  /* Used for loop control on each pass */
   uint32 jbits;           /* Bit mask for all possible jaywalkees. */
   int boybit;             /* 1 if boy, 0 if not (might be neither). */
   int girlbit;            /* 1 if girl, 0 if not (might be neither). */
   int dir;                /* This person's initial facing direction, 0 to 3. */
   int deltax;             /* How this person will move, relative to his own facing */
   int deltay;             /*   direction, when call is finally executed. */
   int nearest;            /* Forward distance to nearest jaywalkee. */
   int leftidx;            /* X-increment of leftmost valid jaywalkee. */
   int rightidx;           /* X-increment of rightmost valid jaywalkee. */
   int deltarot;           /* How this person will turn. */
   int roll_stability_info;/* This person's roll & stability info, from call def'n. */
   int orig_source_idx;
   struct gloop *nextse;   /* Points to next person south (dir even) or east (dir odd.) */
   struct gloop *nextnw;   /* Points to next person north (dir even) or west (dir odd.) */
   long_boolean tbstopse;  /* True if nextse/nextnw is zero because the next spot */
   long_boolean tbstopnw;  /*   is occupied by a T-boned person (as opposed to being empty.) */
} matrix_rec;


Private int start_matrix_call(
   const setup *ss,
   matrix_rec matrix_info[],
   uint32 flags,
   setup *people)
{
   int i;
   const coordrec *thingyptr, *nicethingyptr;
   int nump = 0;

   *people = *ss;    /* Get the setup kind, so selectp will be happier. */
   clear_people(people);

   nicethingyptr = setup_attrs[ss->kind].nice_setup_coords;
   thingyptr = setup_attrs[ss->kind].setup_coords;

   if (flags & (MTX_FIND_SQUEEZERS|MTX_FIND_SPREADERS)) {
      thingyptr = nicethingyptr;
      /* Fix up a galaxy or hourglass so that points can squeeze.
         They have funny coordinates so that they can't truck or loop. */
      if (ss->kind == s_hrglass) thingyptr = &squeezethingglass;
      else if (ss->kind == s_galaxy) thingyptr = &squeezethinggal;
      else if (ss->kind == s_qtag) thingyptr = &squeezethingqtag;
      else if (ss->kind == s4dmd) thingyptr = &squeezething4dmd;
   }

   if (!thingyptr || !nicethingyptr) fail("Can't do this in this setup.");
   if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't do this in this setup.");        /* this is actually superfluous */

   for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++) {
      if (ss->people[i].id1) {
         if (nump == 8) fail("?????too many people????");
         (void) copy_person(people, nump, ss, i);

         matrix_info[nump].nicex = nicethingyptr->xca[i];
         matrix_info[nump].nicey = nicethingyptr->yca[i];
         matrix_info[nump].x = thingyptr->xca[i];
         matrix_info[nump].y = thingyptr->yca[i];

         matrix_info[nump].done = FALSE;
         matrix_info[nump].realdone = FALSE;

         if (flags & MTX_USE_SELECTOR)
            matrix_info[nump].sel = selectp(people, nump);
         else
            matrix_info[nump].sel = FALSE;

         matrix_info[nump].id1 = people->people[nump].id1;
         matrix_info[nump].dir = people->people[nump].id1 & 3;

         matrix_info[nump].jbits = 0;

         if (flags & MTX_USE_VEER_DATA) {
            matrix_info[nump].girlbit = (people->people[nump].id1 & ROLLBITL) ? 1 : 0;
            matrix_info[nump].boybit = (people->people[nump].id1 & ROLLBITR) ? 1 : 0;
         }
         else {
            matrix_info[nump].girlbit = (people->people[nump].id1 & ID1_PERM_GIRL) ? 1 : 0;
            matrix_info[nump].boybit = (people->people[nump].id1 & ID1_PERM_BOY) ? 1 : 0;
         }

         matrix_info[nump].nextse = 0;
         matrix_info[nump].nextnw = 0;
         matrix_info[nump].deltax = 0;
         matrix_info[nump].deltay = 0;
         matrix_info[nump].nearest = 100000;
         matrix_info[nump].deltarot = 0;
         matrix_info[nump].roll_stability_info = (ROLLBITM / ROLL_BIT) * DBROLL_BIT;
         matrix_info[nump].orig_source_idx = i;
         matrix_info[nump].tbstopse = FALSE;
         matrix_info[nump].tbstopnw = FALSE;
         nump++;
      }
   }

   return nump;
}



Private void finish_matrix_call(
   matrix_rec matrix_info[],
   int nump,
   long_boolean do_roll_stability,
   setup *people,
   setup *result)
{
   int i, place;
   int xmax, xpar, ymax, ypar, x, y, k, doffset;
   uint32 signature;
   const coordrec *checkptr;

   xmax = xpar = ymax = ypar = signature = 0;

   for (i=0; i<nump; i++) {

      /* If this person's position has low bit on, that means we consider his coordinates
         not sufficiently well-defined that we will allow him to do any pressing or
         trucking.  He is only allowed to turn.  That is, we will require deltax and
         deltay to be zero.  An example of this situation is the points of a galaxy. */

      /* ****** This seems to be too restrictive.  There may have been good reason for doing this
         at one time, but now it makes all press and truck calls illegal in C1 phantoms.  The
         table for C1 phantoms has been carefully chosen to make things legal only within one's
         own miniwave, but it requires odd numbers.  Perhaps we need to double the resolution
         of things in matrix_info[i].x or y, but that should wait until after version 28 is released. */

/* So this is patched out.  The same problem holds for bigdmds.
      if (((matrix_info[i].x | matrix_info[i].y) & 1) && (matrix_info[i].deltax | matrix_info[i].deltay))
         fail("Someone's ending position is not well defined.");
*/

      switch (matrix_info[i].dir) {
         case 0:
            matrix_info[i].x += matrix_info[i].deltax;
            matrix_info[i].y += matrix_info[i].deltay;
            break;
         case 1:
            matrix_info[i].x += matrix_info[i].deltay;
            matrix_info[i].y -= matrix_info[i].deltax;
            break;
         case 2:
            matrix_info[i].x -= matrix_info[i].deltax;
            matrix_info[i].y -= matrix_info[i].deltay;
            break;
         case 3:
            matrix_info[i].x -= matrix_info[i].deltay;
            matrix_info[i].y += matrix_info[i].deltax;
            break;
      }

      x = matrix_info[i].x;
      y = matrix_info[i].y;

      /* Compute new max, parity, and signature info. */

      if ((x < 0) || ((x == 0) && (y < 0))) { x = -x; y = -y; }
      signature |= 1 << ((31000 + 12*x - 11*y) % 31);
      if (y < 0) y = -y;
      /* Now x and y have both had absolute values taken. */
      if (x > xmax) xmax = x;
      if (y > ymax) ymax = y;
      k = x | 4;
      xpar |= (k & (~(k-1)));
      k = y | 4;
      ypar |= (k & (~(k-1)));
   }


   ypar |= (xmax << 20) | (xpar << 16) | (ymax << 4);

   result->rotation = 0;

   if ((ypar == 0x00A20026) && ((signature & (~0x08008404)) == 0)) {
      checkptr = setup_attrs[s_rigger].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00770077) && ((signature & (~0x00418004)) == 0)) {
      checkptr = setup_attrs[s_galaxy].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00840066) && ((signature & (~0x0C000108)) == 0)) {
      /* Fudge this to a galaxy.  The center 2 did a squeeze or spread from a spindle. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x ==  0 && matrix_info[i].y ==  6) { matrix_info[i].y =  7; }
         else if (matrix_info[i].x ==  0 && matrix_info[i].y == -6) { matrix_info[i].y = -7; }
         else if (matrix_info[i].x ==  8 && matrix_info[i].y ==  0) { matrix_info[i].x =  7; }
         else if (matrix_info[i].x == -8 && matrix_info[i].y ==  0) { matrix_info[i].x = -7; }
         else if (   (matrix_info[i].x == 4 || matrix_info[i].x == -4) &&
                     (matrix_info[i].y == 2 || matrix_info[i].y == -2))
            { matrix_info[i].x >>= 1; }
      }

      warn(warn__check_galaxy);
      checkptr = setup_attrs[s_galaxy].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x00930044) && ((signature & (~0x21018800)) == 0))) {
      /* Fudge this to a 3x6.  The centers did a 1/2 truck from point-to-point diamonds. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x == -9)
            { matrix_info[i].x = -10; }
         else if (matrix_info[i].x == 9)
            { matrix_info[i].x = 10; }
         else if (matrix_info[i].x == -5)
            { matrix_info[i].x = -6; }
         else if (matrix_info[i].x == 5)
            { matrix_info[i].x = 6; }
      }

      checkptr = setup_attrs[s3x6].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x00630055) && ((signature & (~0x01420421)) == 0))) {
      /* Fudge this to a qtag.  The points did a 1/2 press ahead from triple diamonds. */

      for (i=0; i<nump; i++) {
         if (matrix_info[i].y != 0) {
            if (matrix_info[i].x == -2)
               { matrix_info[i].x = -5; }
            else if (matrix_info[i].x == 2)
               { matrix_info[i].x = 5; }

            if (matrix_info[i].x == -5 && matrix_info[i].y > 0)
               { matrix_info[i].x = -4; }
            else if (matrix_info[i].x == 5 && matrix_info[i].y < 0)
               { matrix_info[i].x = 4; }
         }
      }

      checkptr = setup_attrs[s_qtag].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x00A20026) && ((signature & (~0x09080002)) == 0))) {
      /* Fudge this to point-to-point diamonds.  The centers did a 1/2 truck from a 3x6. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x == -10)
            { matrix_info[i].x = -9; }
         else if (matrix_info[i].x == 10)
            { matrix_info[i].x = 9; }
         else if (matrix_info[i].x == -6)
            { matrix_info[i].x = -5; }
         else if (matrix_info[i].x == 6)
            { matrix_info[i].x = 5; }
      }

      checkptr = setup_attrs[s_ptpd].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00660066) && ((signature & (~0x08008404)) == 0)) {
      /* Fudge this to a galaxy.  The points got here by pressing and trucking. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x ==  0 && matrix_info[i].y ==  6) { matrix_info[i].y =  7; }
         else if (matrix_info[i].x ==  0 && matrix_info[i].y == -6) { matrix_info[i].y = -7; }
         else if (matrix_info[i].x ==  6 && matrix_info[i].y ==  0) { matrix_info[i].x =  7; }
         else if (matrix_info[i].x == -6 && matrix_info[i].y ==  0) { matrix_info[i].x = -7; }
      }

      /* Don't need to tell them to check a galaxy -- it's pretty obvious. */
      checkptr = setup_attrs[s_galaxy].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00730055 || ypar == 0x00710051) && ((signature & (~0x01008420)) == 0)) {
      /* Fudge this to diamonds.  The trailing points pressed ahead from quadruple diamonds,
         so that only the centers 2 diamonds are now occupied. */
      warn(warn__check_dmd_qtag);
      checkptr = &press_4dmd_qtag1;
      goto doit;
   }
   else if ((ypar == 0x00730055 || ypar == 0x00710051) && ((signature & (~0x21080400)) == 0)) {
      /* Same, other way. */
      warn(warn__check_dmd_qtag);
      checkptr = &press_4dmd_qtag2;
      goto doit;
   }
   else if ((ypar == 0x00970055) && ((signature & (~0x01400480)) == 0)) {
      /* Fudge this to quadruple diamonds.  The points pressed ahead from normal diamonds. */
      warn(warn__check_quad_dmds);
      checkptr = &press_qtag_4dmd1;
      goto doit;
   }
   else if ((ypar == 0x00870055) && ((signature & (~0x09080400)) == 0)) {
      /* Same, other way. */
      warn(warn__check_quad_dmds);
      checkptr = &press_qtag_4dmd2;
      goto doit;
   }
   else if (ypar == 0x00620026 && ((signature & (~0x01080002)) == 0)) {
      // This must precede the "squeezefinalglass" stuff.
      checkptr = setup_attrs[s_bone6].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00B10071) && ((signature & (~0x01806000)) == 0)) {
      /* Fudge this to a 4x4.  People 1/2 pressed ahead from quadruple 3/4 tags. */
      checkptr = &press_4dmd_4x4;
      goto doit;
   }
   else if ((ypar == 0x00260062 || ypar == 0x00660066) && ((signature & (~0x10100600)) == 0)) {
      /* Fudge this to an hourglass.  Six poeple did a squeeze from a galaxy. */
      checkptr = &squeezefinalglass;
      goto doit;
   }
   else if ((ypar == 0x00620026 || ypar == 0x00660066) && ((signature & (~0x09080002)) == 0)) {
      /* Same, on other orientation. */
      checkptr = &squeezefinalglass;
      goto doitrot;
   }
   else if ((ypar == 0x00660026) && ((signature & (~0x00008604)) == 0)) {
      /* Fudge this to a spindle.  Some points did a squeeze from a galaxy. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x ==  6 && matrix_info[i].y ==  0) { matrix_info[i].x =  8; }
         else if (matrix_info[i].x == -6 && matrix_info[i].y ==  0) { matrix_info[i].x = -8; }
         else if (   (matrix_info[i].x == 2 || matrix_info[i].x == -2) &&
                     (matrix_info[i].y == 2 || matrix_info[i].y == -2)) { matrix_info[i].x <<= 1; }
      }

      checkptr = setup_attrs[s_spindle].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00260066) && ((signature & (~0x09008004)) == 0)) {
      /* Same, on other orientation. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x ==  0 && matrix_info[i].y ==  6) { matrix_info[i].y =  8; }
         else if (matrix_info[i].x ==  0 && matrix_info[i].y == -6) { matrix_info[i].y = -8; }
         else if (   (matrix_info[i].x == 2 || matrix_info[i].x == -2) &&
                     (matrix_info[i].y == 2 || matrix_info[i].y == -2)) { matrix_info[i].y <<= 1; }
      }

      checkptr = setup_attrs[s_spindle].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00950062 || ypar == 0x00550062) && ((signature & (~0x091002C0)) == 0)) {
      checkptr = setup_attrs[sbigdmd].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00950095) && ((signature & (~0x22008080)) == 0)) {
      checkptr = setup_attrs[s_thar].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00660084) && ((signature & (~0x01040420)) == 0)) {
      /* This is a "crosswave" on precise matrix spots. */
      checkptr = &acc_crosswave;
      goto doitrot;
   }
   else if ((ypar == 0x00950066) && ((signature & (~0x28008200)) == 0)) {
      checkptr = setup_attrs[s_crosswave].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00A20026) && ((signature & (~0x01040420)) == 0)) {
      checkptr = setup_attrs[s_bone].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00260062) && ((signature & (~0x08008004)) == 0)) {
      checkptr = setup_attrs[s_short6].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00840026) && ((signature & (~0x04000308)) == 0)) {
      checkptr = setup_attrs[s_spindle].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00840046) && ((signature & (~0x04210308)) == 0)) {
      checkptr = setup_attrs[s_d3x4].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00A200A2) && ((signature & (~0x101CC4E6)) == 0)) {
      checkptr = setup_attrs[s_bigblob].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00670055) && ((signature & (~0x01000420)) == 0)) {
      checkptr = setup_attrs[s_qtag].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00620026) && ((signature & (~0x01008404)) == 0)) {
      /* Fudge this to a 1/4 tag.  Someone did a truck-like operation from a rigger,
         for example, after a sets in motion. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x == -2 && matrix_info[i].y ==  2) { matrix_info[i].x = -4; matrix_info[i].y =  5; }
         else if (matrix_info[i].x ==  2 && matrix_info[i].y ==  2) { matrix_info[i].x =  5; matrix_info[i].y =  5; }
         else if (matrix_info[i].x ==  2 && matrix_info[i].y == -2) { matrix_info[i].x =  4; matrix_info[i].y = -5; }
         else if (matrix_info[i].x == -2 && matrix_info[i].y == -2) { matrix_info[i].x = -5; matrix_info[i].y = -5; }
      }

      checkptr = setup_attrs[s_qtag].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00550067) && ((signature & (~0x08410200)) == 0)) {
      checkptr = setup_attrs[s_qtag].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00660055) && ((signature & (~0x01000480)) == 0)) {
      checkptr = setup_attrs[s_2x1dmd].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00D50026) && ((signature & (~0x20008202)) == 0)) {
      checkptr = setup_attrs[s1x3dmd].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00A60055) && ((signature & (~0x09000480)) == 0)) {
      checkptr = setup_attrs[s3x1dmd].setup_coords;
      goto doit;
   }
   /* Depending on how the setup is actually occupied, xmax and ymax may vary.
      ***** We need to work this out carefully.  Maybe parity should be high
      and max low so that range checking will suffice.
      For now, we just mask out max, and look at par. */
   else if (((ypar & 0x00070007) == 0x00070007) && ((signature & (~0x278198CC)) == 0)) {
      checkptr = setup_attrs[s_c1phan].setup_coords;
      goto doit;
   }
   /* If certain far out people are missing, xmax will be different, but we will
       still need to go to a 3dmd. */
   else if (((ypar == 0x00A70055) || (ypar == 0x00770055) || (ypar == 0x00730055)) && ((signature & (~0x29008480)) == 0)) {
      checkptr = setup_attrs[s3dmd].setup_coords;
      goto doit;
   }
   /* If certain far out people are missing, xmax will be different, but we will
       still need to go to a 4dmd. */
   else if (((ypar == 0x00E30055) ||
             (ypar == 0x00B30055) ||
             (ypar == 0x00B10051) ||
             (ypar == 0x00A30055)) &&
            ((signature & (~0x0940A422)) == 0)) {
      checkptr = setup_attrs[s4dmd].setup_coords;
      goto doit;
   }
   /* Similarly. */
   else if (((ypar == 0x00D50057) ||
             (ypar == 0x00B50057)) &&
            ((signature & (~0x20008202)) == 0)) {
      checkptr = setup_attrs[s_3mdmd].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x00B70057) ||
             (ypar == 0x00E70057)) &&
            ((signature & (~0x41022480)) == 0)) {
      checkptr = setup_attrs[s_3mptpd].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x00D50066) || (ypar == 0x01150066)) &&
            ((signature & (~0x28048202)) == 0)) {
      checkptr = setup_attrs[sbigx].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00B30066) && ((signature & (~0x10508104)) == 0)) {
      checkptr = setup_attrs[sdeepxwv].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x00F30066) || (ypar == 0x01130066) || (ypar == 0x01330066)) &&
            ((signature & (~0x12148904)) == 0)) {
      checkptr = setup_attrs[sbigbigx].setup_coords;
      goto doit;
   }
   else if (((ypar == 0x01130066) || (ypar == 0x01130026)) &&
            ((signature & (~0x09406600)) == 0)) {
      checkptr = setup_attrs[sbigbigh].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00550057) && ((signature & (~0x20000620)) == 0)) {
      checkptr = setup_attrs[s_hrglass].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00840066) && ((signature & (~0x00202208)) == 0)) {
      /* Fudge this to an hourglass.  Someone had the checkpointers squeeze or spread
         from a spindle. */

      for (i=0; i<nump; i++) {
         if      (matrix_info[i].x == -4 && matrix_info[i].y ==  6) {                        matrix_info[i].y =  5; }
         else if (matrix_info[i].x ==  4 && matrix_info[i].y ==  6) { matrix_info[i].x =  5; matrix_info[i].y =  5; }
         else if (matrix_info[i].x ==  4 && matrix_info[i].y == -6) {                        matrix_info[i].y = -5; }
         else if (matrix_info[i].x == -4 && matrix_info[i].y == -6) { matrix_info[i].x = -5; matrix_info[i].y = -5; }
         else if (matrix_info[i].x ==  8 && matrix_info[i].y ==  0) { matrix_info[i].x =  5;                        }
         else if (matrix_info[i].x == -8 && matrix_info[i].y ==  0) { matrix_info[i].x = -5;                        }
      }

      checkptr = setup_attrs[s_hrglass].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00A70026) && ((signature & (~0x20040220)) == 0)) {
      checkptr = setup_attrs[s_dhrglass].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00930026) && ((signature & (~0x01108080)) == 0)) {
      checkptr = setup_attrs[s_ptpd].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00620044) && ((signature & (~0x11800C40)) == 0)) {
      checkptr = setup_attrs[s3x4].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00440062) && ((signature & (~0x0C202300)) == 0)) {
      checkptr = setup_attrs[s3x4].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00840022) && ((signature & (~0x06001300)) == 0)) {
      checkptr = setup_attrs[s2x5].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00220084) && ((signature & (~0x21080840)) == 0)) {
      checkptr = setup_attrs[s2x5].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00E20004) && ((signature & (~0x09002400)) == 0)) {
      checkptr = setup_attrs[s1x8].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x000400E2) && ((signature & (~0x08004202)) == 0)) {
      checkptr = setup_attrs[s1x8].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x01220004) && ((signature & (~0x49002400)) == 0)) {
      checkptr = setup_attrs[s1x10].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x01620004) && ((signature & (~0x49012400)) == 0)) {
      checkptr = setup_attrs[s1x12].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x01A20004) && ((signature & (~0x49012404)) == 0)) {
      checkptr = setup_attrs[s1x14].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x01E20004) && ((signature & (~0x49092404)) == 0)) {
      checkptr = setup_attrs[s1x16].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00620022) && ((signature & (~0x00088006)) == 0)) {
      checkptr = setup_attrs[s2x4].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00220062) && ((signature & (~0x10108004)) == 0)) {
      checkptr = setup_attrs[s2x4].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00440022) && ((signature & (~0x04000300)) == 0)) {
      checkptr = setup_attrs[s2x3].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00220044) && ((signature & (~0x01000840)) == 0)) {
      checkptr = setup_attrs[s2x3].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00A20022) && ((signature & (~0x000C8026)) == 0)) {
      checkptr = setup_attrs[s2x6].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x002200A2) && ((signature & (~0x10108484)) == 0)) {
      checkptr = setup_attrs[s2x6].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00C40022) && ((signature & (~0x26001B00)) == 0)) {
      checkptr = setup_attrs[s2x7].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00E20022) && ((signature & (~0x004C8036)) == 0)) {
      checkptr = setup_attrs[s2x8].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x002200E2) && ((signature & (~0x12908484)) == 0)) {
      checkptr = setup_attrs[s2x8].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00A20044) && ((signature & (~0x19804E40)) == 0)) {
      checkptr = setup_attrs[s3x8].setup_coords;   /* Actually a 3x6. */
      goto doit;
   }
   else if ((ypar == 0x00E20044) && ((signature & (~0x1D806E41)) == 0)) {
      checkptr = setup_attrs[s3x8].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00840062) && ((signature & (~0x46203180)) == 0)) {
      checkptr = setup_attrs[s4x5].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00620084) && ((signature & (~0x31888C60)) == 0)) {
      checkptr = setup_attrs[s4x5].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00A20062) && ((signature & (~0x109CC067)) == 0)) {
      checkptr = setup_attrs[s4x6].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x006200A2) && ((signature & (~0x1918C4C6)) == 0)) {
      checkptr = setup_attrs[s4x6].setup_coords;
      goto doitrot;
   }
   else if ((ypar == 0x00C40062) && ((signature & (~0x6E001B80)) == 0)) {
      checkptr = setup_attrs[s3oqtg].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00620062) && ((signature & (~0x1018C046)) == 0)) {
      checkptr = setup_attrs[s4x4].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00930066) && ((signature & (~0x01080C40)) == 0)) {
      checkptr = setup_attrs[sbigh].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00E20026) && ((signature & (~0x01440430)) == 0)) {
      checkptr = setup_attrs[sbigbone].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00460044) && ((signature & (~0x41040010)) == 0)) {
      checkptr = setup_attrs[s_323].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00660044) && ((signature & (~0x41040410)) == 0)) {
      checkptr = setup_attrs[s_343].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00860044) && ((signature & (~0x49250010)) == 0)) {
      checkptr = setup_attrs[s_525].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00860044) && ((signature & (~0x41250410)) == 0)) {
      checkptr = setup_attrs[s_545].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00860044) && ((signature & (~0x41250018)) == 0)) {
      checkptr = setup_attrs[sh545].setup_coords;
      goto doit;
   }
   /* **** These last ones are sort of a crock.  They are designed to make
      matrix calls work in distorted or virtual setups in some circumstances
      (i.e. if no one changes coordinates.)  However, they won't work in the
      presence of unsymmetrical phantoms.  What we really should do is, if
      the setup is virtual/distorted (or maybe the test should be if no one
      moved) just force people back to the same setup they started in. */
   else if ((ypar == 0x00220022) && ((signature & (~0x00008004)) == 0)) {
      checkptr = setup_attrs[s2x2].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00620004) && ((signature & (~0x01000400)) == 0)) {
      checkptr = setup_attrs[s1x4].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00550026) && ((signature & (~0x20020200)) == 0)) {
      checkptr = setup_attrs[sdmd].setup_coords;
      goto doit;
   }
   else if ((ypar == 0x00220004) && ((signature & (~0x01000000)) == 0)) {
      checkptr = setup_attrs[s1x2].setup_coords;
      goto doit;
   }

   fail("Can't handle this result matrix.");

 doit:

   doffset = 32 - (1 << (checkptr->xfactor-1));

   for (i=0; i<nump; i++) {
      people->people[i].id1 =
         rotperson(people->people[i].id1, matrix_info[i].deltarot*011);

      place = checkptr->diagram[doffset -
                               ((matrix_info[i].y >> 2) << checkptr->xfactor) +
                               (matrix_info[i].x >> 2)];
      if (place < 0) fail("Person has moved into a grossly ill-defined location.");

      if ((checkptr->xca[place] != matrix_info[i].x) ||
          (checkptr->yca[place] != matrix_info[i].y))
      fail("Person has moved into a slightly ill-defined location.");

      install_person(result, place, people, i);

      if (do_roll_stability) {
         result->people[place].id1 &= ~ROLL_MASK;
         result->people[place].id1 |=
            ((matrix_info[i].roll_stability_info * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

         if (result->people[place].id1 & STABLE_ENAB)
            do_stability(
               &result->people[place].id1,
               (stability) ((matrix_info[i].roll_stability_info/DBSTAB_BIT) & 0xF),
               matrix_info[i].deltarot);
      }
   }

   result->kind = checkptr->result_kind;
   return;

 doitrot:

   result->rotation = 1;
   doffset = 32 - (1 << (checkptr->xfactor-1));

   for (i=0; i<nump; i++) {
      people->people[i].id1 =
         rotperson(people->people[i].id1, matrix_info[i].deltarot*011);

      place = checkptr->diagram[doffset -
                               ((matrix_info[i].x >> 2) << checkptr->xfactor) +
                               ((-matrix_info[i].y) >> 2)];
      if (place < 0) fail("Person has moved into a grossly ill-defined location.");

      if ((checkptr->xca[place] != -matrix_info[i].y) ||
          (checkptr->yca[place] != matrix_info[i].x))
         fail("Person has moved into a slightly ill-defined location.");

      install_rot(result, place, people, i, 033);

      if (do_roll_stability) {
         result->people[place].id1 &= ~ROLL_MASK;
         result->people[place].id1 |=
            ((matrix_info[i].roll_stability_info * (ROLL_BIT/DBROLL_BIT)) & ROLL_MASK);

         if (result->people[place].id1 & STABLE_ENAB)
            do_stability(
               &result->people[place].id1,
               (stability) ((matrix_info[i].roll_stability_info/DBSTAB_BIT) & 0xF),
               matrix_info[i].deltarot);
      }
   }

   result->kind = checkptr->result_kind;
   return;
}



Private void matrixmove(
   setup *ss,
   Const callspec_block *callspec,
   setup *result)
{
   uint32 datum;
   setup people;
   matrix_rec matrix_info[9];
   int i, nump, alldelta;
   uint32 flags = callspec->stuff.matrix.flags;
   const uint32 *callstuff = callspec->stuff.matrix.stuff;

   alldelta = 0;

   nump = start_matrix_call(ss, matrix_info, flags, &people);

   for (i=0; i<nump; i++) {
      matrix_rec *thisrec = &matrix_info[i];

      if (!(flags & MTX_USE_SELECTOR) || thisrec->sel) {
         /* This is legal if girlbit or boybit is on (in which case we use the appropriate datum)
            or if the two data are identical so the sex doesn't matter. */
         if ((thisrec->girlbit | thisrec->boybit) == 0 &&
             callstuff[0] != callstuff[1]) {
            if (flags & MTX_USE_VEER_DATA)
               fail("Can't determine lateral direction of this person.");
            else
               fail("Can't determine sex of this person.");
         }

         datum = callstuff[thisrec->girlbit];

         thisrec->deltax = (((datum >> 4) & 0x1F) - 16) << 1;
         thisrec->deltay = (((datum >> 16) & 0x1F) - 16) << 1;

         if (flags & MTX_USE_NUMBER) {
            int count = current_options.number_fields & 0xF;
            thisrec->deltax *= count;
            thisrec->deltay *= count;
         }

         thisrec->deltarot = datum & 3;
         thisrec->roll_stability_info = datum;
         alldelta |= thisrec->deltax | thisrec->deltay;
      }
   }

   if (alldelta != 0) {
      if (ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED)
         fail("This call not allowed in distorted or virtual setup.");

      if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK)
         fail("Can't split the setup.");
   }
   else
      /* This call split the setup in every possible way. */
      result->result_flags |= RESULTFLAG__SPLIT_AXIS_FIELDMASK;

   finish_matrix_call(matrix_info, nump, TRUE, &people, result);
   reinstate_rotation(ss, result);

   /* If the call just kept a 2x2 in place, and they were the outsides, make
      sure that the elongation is preserved. */

   switch (ss->kind) {
   case s2x2: case s_short6:
      result->result_flags = ss->cmd.prior_elongation_bits & 3;
      break;
   case s1x2: case s1x4: case sdmd:
      result->result_flags = 2 - (ss->rotation & 1);
      break;
   default:
      result->result_flags = 0;
   }
}



Private void do_part_of_pair(matrix_rec *thisrec, int base, Const uint32 *callstuff)
{
   /* This is legal if girlbit or boybit is on (in which case we use the appropriate datum)
         or if the two data are identical so the sex doesn't matter. */
   if ((thisrec->girlbit | thisrec->boybit) == 0 && callstuff[base] != callstuff[base+1])
      fail("Can't determine sex of this person.");
   uint32 datum = callstuff[base+thisrec->girlbit];
   if (datum == 0) failp(thisrec->id1, "can't do this call.");
   thisrec->deltax = (((datum >> 4) & 0x1F) - 16) << 1;
   thisrec->deltay = (((datum >> 16) & 0x1F) - 16) << 1;
   thisrec->deltarot = datum & 3;
   thisrec->roll_stability_info = datum;
   thisrec->realdone = TRUE;
}


Private void do_pair(
   matrix_rec *ppp,        /* Selected person */
   matrix_rec *qqq,        /* Unselected person */
   Const uint32 *callstuff,
   uint32 flags,
   int flip,
   int filter)             /* 1 to do N/S facers, 0 for E/W facers. */
{
   if (callstuff) {     // Doing normal matrix call.
      if ((!(flags & (MTX_IGNORE_NONSELECTEES | MTX_BOTH_SELECTED_OK))) && qqq->sel)
         fail("Two adjacent selected people.");

      /* We know that either ppp is actually selected, or we are not using selectors. */

      if ((filter ^ ppp->dir) & 1) {
         int base = (ppp->dir & 2) ? 6 : 4;
         if (!(flags & MTX_USE_SELECTOR)) base &= 3;
         do_part_of_pair(ppp, base^flip, callstuff);
      }

      if ((filter ^ qqq->dir) & 1) {
         int base = (qqq->dir & 2) ? 0 : 2;
         if ((flags & MTX_IGNORE_NONSELECTEES) || qqq->sel) base |= 4;
         do_part_of_pair(qqq, base^flip, callstuff);
      }
   }
   else {    // Doing "drag" concept.
      // ppp and qqq are a pair, independent of selection.
      // They may contain a dragger and a draggee.
      if (ppp->sel) {
         if (qqq->sel) fail("Two adjacent people being dragged.");
         ppp->realdone = TRUE;
         ppp->deltax = qqq->x;
         ppp->deltay = qqq->y;
         ppp->deltarot = qqq->orig_source_idx;
         ppp->nearest = qqq->dir;
      }
      else if (qqq->sel) {
         qqq->realdone = TRUE;
         qqq->deltax = ppp->x;
         qqq->deltay = ppp->y;
         qqq->deltarot = ppp->orig_source_idx;
         qqq->nearest = ppp->dir;
      }
   }

   ppp->done = TRUE;
   qqq->done = TRUE;
}



Private void make_matrix_chains(
   matrix_rec matrix_info[],
   int nump,
   long_boolean finding_far_squeezers,
   uint32 flags,
   int filter)                        /* 1 for E/W chains, 0 for N/S chains. */
{
   int i, j;

   /* Find adjacency relationships, and fill in the "se"/"nw" pointers. */

   for (i=0; i<nump; i++) {
      matrix_rec *mi = &matrix_info[i];

      if ((flags & MTX_IGNORE_NONSELECTEES) && (!mi->sel)) continue;

      for (j=0; j<nump; j++) {
         matrix_rec *mj = &matrix_info[j];
         int delx, dely;
         uint32 dirxor;
         int ix, jx, iy, jy;

         if ((flags & MTX_IGNORE_NONSELECTEES) && (!mj->sel)) continue;

         /* Find out if these people are adjacent in the right way. */

         if (flags & MTX_FIND_JAYWALKERS) {
            jx = mj->nicex;
            ix = mi->nicex;
            jy = mj->nicey;
            iy = mi->nicey;
         }
         else if (filter) {
            jx = - mj->y;
            ix = - mi->y;
            jy = - mj->x;
            iy = - mi->x;
         }
         else {
            jx = mj->x;
            ix = mi->x;
            jy = mj->y;
            iy = mi->y;
         }

         delx = ix - jx;
         dely = iy - jy;

         dirxor = mi->dir ^ mj->dir;

         if (flags & MTX_FIND_JAYWALKERS) {
            int jdist = (mi->dir & 1) ? delx : dely;
            if (!(mi->dir & 2)) jdist = -jdist;

            /* Jdist is the forward distance to the jaywalkee.  We demand that it be
               strictly positive, and minimal with respect to all other jaywalkee
               candidates.  We also consider only jaywalkees that are either facing
               back toward us or are facing lateral but are in such a position that
               they can see us. */

            if (jdist > 0 && jdist <= mi->nearest && dirxor != 0) {
/**** that dirxor test isn't really good enough */
               if (jdist < mi->nearest) {
                  /* This jaywalkee is closer than any others that we have seen; drop all others. */
                  mi->jbits = 0;
                  mi->nearest = jdist;  /* This is the new nearest distance. */
               }
               mi->jbits |= (1 << j);
            }
         }
         else {
            if (dely != (finding_far_squeezers ? 12 : 4) || delx != 0) continue;

            /* Now, if filter = 1, person j is just east of person i.
               If filter = 0, person j is just south of person i. */

            if (flags & MTX_TBONE_IS_OK) {
               /* Make the chain independently of facing direction. */
            }
            else {
               if ((mi->dir ^ filter) & 1) {
                  if (dirxor & 1) {
                     if (!(flags & MTX_STOP_AND_WARN_ON_TBONE)) fail("People are T-boned.");
                     mi->tbstopse = TRUE;
                     mj->tbstopnw = TRUE;
                     break;
                  }
                  else {
                     if ((flags & MTX_MUST_FACE_SAME_WAY) && dirxor)
                        fail("Paired people must face the same way.");
                  }
               }
               else
                  break;
            }

            if (mj->nextnw) fail("Adjacent to too many people.");

            mi->nextse = mj;
            mj->nextnw = mi;
            break;
         }
      }
   }
}


Private void process_matrix_chains(
   matrix_rec matrix_info[],
   int nump,
   const uint32 *callstuff,
   uint32 flags,
   int filter)                        /* 1 for E/W chains, 0 for N/S chains. */
{
   long_boolean another_round = TRUE;
   long_boolean picking_end_jaywalkers = FALSE;
   int i, j;

   /* Pick out pairs of people and move them. */

   while (another_round) {
      another_round = FALSE;

      if (flags & MTX_FIND_JAYWALKERS) {
         /* Clear out any bits that aren't bidirectional. */

         for (i=0; i<nump; i++) {
            matrix_rec *mi = &matrix_info[i];
            if ((flags & MTX_IGNORE_NONSELECTEES) && (!mi->sel)) continue;

            for (j=0; j<nump; j++) {
               if (      ((1<<j) & mi->jbits) &&
                        !((1<<i) & matrix_info[j].jbits))
                  mi->jbits &= ~(1<<j);
            }

            /* If selected person has no jaywalkee, it is an error. */
            if (!mi->jbits) failp(mi->id1, "can't find person to jaywalk with.");
         }
      }

      for (i=0; i<nump; i++) {
         matrix_rec *mi = &matrix_info[i];

         if (!mi->done) {
            if (flags & MTX_FIND_JAYWALKERS) {

               if (!picking_end_jaywalkers) {
                  mi->leftidx = -1;
                  mi->rightidx = -1;
               }

               if (mi->jbits) {   /* If no jbits, person wasn't selected. */
                  for (j=0; j<nump; j++) {
                     matrix_rec *mj = &matrix_info[j];

                     if (picking_end_jaywalkers) {
                        /* Look for people who have been identified as extreme jaywalkers with each other. */
                        if (j == mi->leftidx && i == mj->rightidx) {
                           mi->jbits = (1<<j);    /* Separate them from the rest. */
                           mj->jbits = (1<<i);
                           another_round = TRUE;   /* We will move them on a future iteration. */
                        }
                     }
                     else {
                        int jdist;

                        /* Fill in jaywalkee indices with record-holding lateral distance. */

                        if ((1<<j) & mi->jbits) {
                           jdist = (mi->dir & 1) ? mj->nicey : mj->nicex;
                           if ((mi->dir+1) & 2) jdist = -jdist;

                           if (mi->leftidx >= 0) {
                              int temp = (mi->dir & 1) ? matrix_info[mi->leftidx].nicey : matrix_info[mi->leftidx].nicex;
                              if ((mi->dir+1) & 2) temp = -temp;
                              if (jdist < temp) mi->leftidx = j;
                           }
                           else
                              mi->leftidx = j;

                           if (mi->rightidx >= 0) {
                              int temp = (mi->dir & 1) ? matrix_info[mi->rightidx].nicey : matrix_info[mi->rightidx].nicex;
                              if ((mi->dir+1) & 2) temp = -temp;
                              if (jdist > temp) mi->rightidx = j;
                           }
                           else
                              mi->rightidx = j;
                        }

                        /* Look for special case of two people uniquely selecting each other. */
                        if (      (1UL<<j) == mi->jbits &&
                                  (1UL<<i) == mj->jbits) {
                           int delx = mj->x - mi->x;
                           int dely = mj->y - mi->y;
                           int deltarot;

                           uint32 datum = callstuff[mi->girlbit];
                           if (datum == 0) failp(mi->id1, "can't do this call.");

                           another_round = TRUE;

                           if (mi->dir & 2) delx = -delx;
                           if ((mi->dir+1) & 2) dely = -dely;

                           mi->deltax = (((datum >> 4) & 0x1F) - 16) << 1;
                           mi->deltay = (((datum >> 16) & 0x1F) - 16) << 1;
                           mi->deltarot = datum & 3;
                           mi->roll_stability_info = datum;
                           mi->realdone = TRUE;

                           deltarot = mj->dir - mi->dir + 2;

                           if (deltarot) {
                              /* This person went "around the corner" due to facing
                                 direction of other dancer. */
                              if (mi->deltarot ||
                                  (stability) ((mi->roll_stability_info/DBSTAB_BIT) & 0xF) != stb_z) {


                                 /* Call definition also had turning, as in "jay slide thru".
                                    Just erase the stability info. */
                                 mi->roll_stability_info &= ~(DBSTAB_BIT * 0xF);
                              }
                              else {
                                 mi->roll_stability_info &= ~(DBSTAB_BIT * 0xF);
                                 if ((deltarot & 3) == 1)
                                    mi->roll_stability_info |= (DBSTAB_BIT * stb_c);
                                 else
                                    mi->roll_stability_info |= (DBSTAB_BIT * stb_a);
                              }
                           }

                           mi->deltarot += deltarot;
                           mi->deltarot &= 3;

                           if (mi->dir & 1) { mi->deltax += dely; mi->deltay = +delx; }
                           else             { mi->deltax += delx; mi->deltay = +dely; }

                           mi->done = TRUE;
                        }
                        /* Take care of anyone who has unambiguous jaywalkee.  Get that
                           jaywalkee's attention by stripping off all his other bits. */
                        else if ((1UL<<j) == mi->jbits) {
                           mj->jbits = (1<<i);
                           another_round = TRUE;   /* We will move them on a future iteration. */
                        }
                     }
                  }
               }
            }
            else if (mi->nextse) {
               /* This person might be ready to be paired up with someone. */
               if (mi->nextnw)
                  /* This person has neighbors on both sides.  Can't do anything yet. */
                  ;
               else {
                  /* This person has a neighbor on south/east side only. */

                  if (mi->tbstopnw) warn(warn__not_tbone_person);

                  if (   (!(flags & MTX_USE_SELECTOR))  ||  mi->sel   ) {
                     /* Do this pair.  First, chop the pair off from anyone else. */

                     if (mi->nextse->nextse) mi->nextse->nextse->nextnw = 0;
                     mi->nextse->nextse = 0;
                     another_round = TRUE;
                     do_pair(mi, mi->nextse, callstuff, flags, 0, filter);
                  }
               }
            }
            else if (mi->nextnw) {
               /* This person has a neighbor on north/west side only. */

               if (mi->tbstopse) warn(warn__not_tbone_person);

               if (   (!(flags & MTX_USE_SELECTOR))  ||  mi->sel   ) {
                  /* Do this pair.  First, chop the pair off from anyone else. */

                  if (mi->nextnw->nextnw) mi->nextnw->nextnw->nextse = 0;
                  mi->nextnw->nextnw = 0;
                  another_round = TRUE;
                  do_pair(mi, mi->nextnw, callstuff, flags, 2, filter);
               }
            }
            else {
               /* Person is alone.  If this is his lateral axis, mark him done and don't move him. */

               if ((mi->dir ^ filter) & 1) {
                  if (mi->tbstopse || mi->tbstopnw) warn(warn__not_tbone_person);
                  if (   (!(flags & MTX_USE_SELECTOR))  ||  mi->sel   )
                     failp(mi->id1, "has no one to work with.");
                  mi->done = TRUE;
               }
            }
         }
      }

      /* If anything happened, don't pick jaywalkers next time around. */

      if (another_round) picking_end_jaywalkers = FALSE;

      /* If doing a jaywalk, and nothing was processed, the leftest/rightest stuff
         is filled in, and we should check same. */

      if ((flags & MTX_FIND_JAYWALKERS) && !another_round && !picking_end_jaywalkers) {
         picking_end_jaywalkers = TRUE;
         another_round = TRUE;     /* Do another round this way. */
      }
   }
}




Private void partner_matrixmove(
   setup *ss,
   Const callspec_block *callspec,
   setup *result)
{
   uint32 flags = callspec->stuff.matrix.flags;
   const uint32 *callstuff = callspec->stuff.matrix.stuff;
   setup people;
   matrix_rec matrix_info[9];
   int i, nump;

   if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK)
      fail("Can't split the setup.");

   /* We allow stuff like "tandem jay walk". */

   if ((ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED) && !(flags & MTX_FIND_JAYWALKERS))
      fail("This call not allowed in distorted or virtual setup.");

   nump = start_matrix_call(ss, matrix_info, flags, &people);

   /* Make the lateral chains first. */

   make_matrix_chains(matrix_info, nump, FALSE, flags, 1);
   if (flags & MTX_FIND_SQUEEZERS)
      make_matrix_chains(matrix_info, nump, TRUE, flags, 1);
   process_matrix_chains(matrix_info, nump, callstuff, flags, 1);

   /* If jaywalking, don't do it again. */

   if (!(flags & MTX_FIND_JAYWALKERS)){
      /* Now clean off the pointers in preparation for the second pass. */

      for (i=0; i<nump; i++) {
         matrix_info[i].done = FALSE;
         matrix_info[i].nextse = 0;
         matrix_info[i].nextnw = 0;
         matrix_info[i].tbstopse = FALSE;
         matrix_info[i].tbstopnw = FALSE;
      }

      /* Vertical chains next. */

      make_matrix_chains(matrix_info, nump, FALSE, flags, 0);
      if (flags & MTX_FIND_SQUEEZERS)
         make_matrix_chains(matrix_info, nump, TRUE, flags, 0);
      process_matrix_chains(matrix_info, nump, callstuff, flags, 0);
   }

   /* Scan for people who ought to have done something but didn't. */

   for (i=0; i<nump; i++) {
      if (!matrix_info[i].realdone) {
         if ((!(flags & MTX_USE_SELECTOR)) || matrix_info[i].sel)
            failp(matrix_info[i].id1, "could not identify other person to work with.");
      }
   }

   finish_matrix_call(matrix_info, nump, TRUE, &people, result);
   reinstate_rotation(ss, result);
   result->result_flags = 0;
}



extern void drag_someone_and_move(setup *ss, parse_block *parseptr, setup *result)
{
   setup people, second_people;
   matrix_rec matrix_info[9];
   matrix_rec second_matrix_info[9];
   int i;
   long_boolean fudged_start = FALSE;
   uint32 flags = MTX_STOP_AND_WARN_ON_TBONE | MTX_IGNORE_NONSELECTEES;
   selector_kind saved_selector = current_options.who;
   current_options.who = parseptr->options.who;

   setup scopy = *ss;      // Will save rotation of this to the very end.
   scopy.rotation = 0;

   if (scopy.kind == s_qtag) {
      expand_setup(&exp_qtg_3x4_stuff, &scopy);
      fudged_start = TRUE;
   }

   int nump = start_matrix_call(&scopy, matrix_info,
                                MTX_USE_SELECTOR | MTX_STOP_AND_WARN_ON_TBONE, &people);
   current_options.who = saved_selector;

   /* Make the lateral chains first. */

   make_matrix_chains(matrix_info, nump, FALSE, MTX_STOP_AND_WARN_ON_TBONE, 1);
   process_matrix_chains(matrix_info, nump, (uint32 *) 0, flags, 1);

   /* Now clean off the pointers in preparation for the second pass. */

   for (i=0; i<nump; i++) {
      matrix_info[i].done = FALSE;
      matrix_info[i].nextse = 0;
      matrix_info[i].nextnw = 0;
      matrix_info[i].tbstopse = FALSE;
      matrix_info[i].tbstopnw = FALSE;
   }

   /* Vertical chains next. */

   make_matrix_chains(matrix_info, nump, FALSE, MTX_STOP_AND_WARN_ON_TBONE, 0);
   process_matrix_chains(matrix_info, nump, (uint32 *) 0, flags, 0);

   /* Scan for people who ought to have done something but didn't. */

   for (i=0; i<nump; i++) {
      if (matrix_info[i].sel)
         clear_person(&scopy, matrix_info[i].orig_source_idx);
   }

   setup refudged = scopy;
   if (fudged_start)
      compress_setup(&exp_qtg_3x4_stuff, &refudged);

   move(&refudged, FALSE, result);

   // Expand again if it's another qtag.
   if (result->kind == s_qtag)
      expand_setup(&exp_qtg_3x4_stuff, result);

   // Now figure out where the people who moved really are.

   int second_nump = start_matrix_call(result, second_matrix_info,
                                       MTX_STOP_AND_WARN_ON_TBONE, &second_people);

   int final_2nd_nump = second_nump;

   // And scan the dragged people (who aren't in the result setup)
   // to find out how to glue them to the real result people.

   for (i=0; i<nump; i++) {
      if (matrix_info[i].sel) {
         // Get the actual dragger person id1 word.
         uint32 dragger_id = scopy.people[matrix_info[i].deltarot].id1;

         // Find the XY coords of the person's dragger.

         int kk;
         for (kk=0; kk<second_nump; kk++) {
            if (((second_matrix_info[kk].id1 ^ dragger_id) & (PID_MASK|BIT_PERSON)) == 0)
               goto found_dragger;
         }
         fail("Internal error: failed to find dragger coords.");
      found_dragger:
         // Original offset of draggee relative to dragger.
         int origdx = matrix_info[i].x - matrix_info[i].deltax;
         int origdy = matrix_info[i].y - matrix_info[i].deltay;
         // Find out how much the dragger turned while doing the call.
         // The "before" space is scopy and the "after" space is result,
         // so this doesn't necessarily relate to actual turning.
         int dragger_turn = (second_matrix_info[kk].dir - matrix_info[i].nearest) & 3;
         if (dragger_turn & 2) {
            origdx = -origdx;
            origdy = -origdy;
         }
         if (dragger_turn & 1) {
            int temp = origdx;
            origdx = origdy;
            origdy = -temp;
         }
         // Now origdx/dy has offset of draggee from dragger in new space.
         // This is new info for draggee.
         second_people.people[final_2nd_nump].id1 =
            rotperson(people.people[i].id1, dragger_turn*011);
         second_matrix_info[final_2nd_nump] = matrix_info[i];
         second_matrix_info[final_2nd_nump].x = second_matrix_info[kk].x + origdx;
         second_matrix_info[final_2nd_nump++].y = second_matrix_info[kk].y + origdy;
      }
   }

   for (i=0; i<final_2nd_nump; i++) {
      second_matrix_info[i].deltax = 0;
      second_matrix_info[i].deltay = 0;
      second_matrix_info[i].deltarot = 0;
   }

   clear_people(result);

   ss->rotation += result->rotation;    // finish_matrix_call will clear result->rotation.
   finish_matrix_call(second_matrix_info, final_2nd_nump, TRUE, &second_people, result);
   reinstate_rotation(ss, result);
   result->result_flags = 0;
}



extern void anchor_someone_and_move(
   setup *ss,
   parse_block *parseptr,
   setup *result)
{
   setup people;
   matrix_rec before_matrix_info[9];
   matrix_rec after_matrix_info[9];
   int i, j, k, nump, numgroups;
   int deltax[4], deltay[4];
   selector_kind saved_selector = current_options.who;
   setup saved_start_people = *ss;
   int Bindex[4];
   int Aindex[4];

   current_options.who = parseptr->options.who;

   if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK)
      fail("Can't split the setup.");

   if (ss->cmd.cmd_misc_flags & CMD_MISC__DISTORTED)
      fail("This call not allowed in distorted or virtual setup.");

   for (i=0 ; i<4 ; i++) { Bindex[i] = -1; Aindex[i] = -1; }

   ss->rotation = 0;

   if (     ss->kind != s2x4 &&
            ss->kind != s1x8 &&
            ss->kind != s2x6 &&
            ss->kind != s3x4 &&
            ss->kind != s_qtag &&
            ss->kind != s_ptpd)
      fail("Sorry, can't do this in this setup.");
   move(ss, FALSE, result);
   numgroups = 0;

   nump = start_matrix_call(&saved_start_people, before_matrix_info, MTX_USE_SELECTOR, &people);
   current_options.who = saved_selector;

   for (i=0 ; i<nump ; i++) {
      switch (result->result_flags & RESULTFLAG__SPLIT_AXIS_FIELDMASK) {
      case RESULTFLAG__SPLIT_AXIS_XBIT:
         numgroups = 2;
         if (before_matrix_info[i].x < 0) j = 0;
         else j = 1;
         break;
      case RESULTFLAG__SPLIT_AXIS_YBIT:
         numgroups = 2;
         if (before_matrix_info[i].y < 0) j = 0;
         else j = 1;
         break;
      case (RESULTFLAG__SPLIT_AXIS_XBIT + RESULTFLAG__SPLIT_AXIS_YBIT):
         numgroups = 4;
         if (before_matrix_info[i].x < 0) j = 0;
         else j = 2;
         if (before_matrix_info[i].y < 0) j++;
         break;
      case (RESULTFLAG__SPLIT_AXIS_XBIT * 2):
         /* This is split into four groups in a row from left to right. */
         numgroups = 4;
         if      (before_matrix_info[i].x < -setup_attrs[ss->kind].bounding_box[0]) j = 0;
         else if (before_matrix_info[i].x < 0) j = 1;
         else if (before_matrix_info[i].x < setup_attrs[ss->kind].bounding_box[0]) j = 2;
         else j = 3;
         break;
      default:
         fail("Can't 'anchor' someone for an 8-person call.");
      }

      before_matrix_info[i].nearest = j;    /* The "nearest" field tells what anchored group the person is in. */

      if (before_matrix_info[i].sel) {
         if (Bindex[j] >= 0) fail("Need exactly one 'anchored' person in each group.");
         Bindex[j] = i;
      }
   }

   for (k=0 ; k<numgroups ; k++) {
      if (Bindex[k] < 0) fail("Need exactly one 'anchored' person in each group.");
   }

   nump = start_matrix_call(result, after_matrix_info, 0, &people);

   for (i=0 ; i<nump ; i++) {
      if (result->rotation) {
         int t = after_matrix_info[i].x;
         after_matrix_info[i].x = after_matrix_info[i].y;
         after_matrix_info[i].y = -t;
      }

      after_matrix_info[i].deltarot += result->rotation;

      for (k=0 ; k<numgroups ; k++) {
         if (((after_matrix_info[i].id1 ^ before_matrix_info[Bindex[k]].id1) & XPID_MASK) == 0) Aindex[k] = i;
      }
   }

   for (k=0 ; k<numgroups ; k++) {
      if (Aindex[k] < 0) fail("Sorry6.");
   }

   for (k=0 ; k<numgroups ; k++) {
      deltax[k] = before_matrix_info[Bindex[k]].x - after_matrix_info[Aindex[k]].x;
      deltay[k] = before_matrix_info[Bindex[k]].y - after_matrix_info[Aindex[k]].y;
   }

   for (i=0 ; i<nump ; i++) {
      /* Find this person's group. */

      for (k=0 ; k<nump ; k++) {
         if (((after_matrix_info[i].id1 ^ before_matrix_info[k].id1) & XPID_MASK) == 0) {
            j = before_matrix_info[k].nearest;
            goto found_person;
         }
      }
      fail("Can't find where this person went.");

      found_person:

      after_matrix_info[i].deltax = deltax[j];
      after_matrix_info[i].deltay = deltay[j];
      after_matrix_info[i].dir = 0;
   }

   clear_people(result);
   finish_matrix_call(after_matrix_info, nump, FALSE, &people, result);
   reinstate_rotation(&saved_start_people, result);
   result->result_flags = 0;
}


Private void rollmove(
   setup *ss,
   Const callspec_block *callspec,
   setup *result)
{
   int i;

   if (setup_attrs[ss->kind].setup_limits < 0) fail("Can't roll in this setup.");

   result->kind = ss->kind;
   result->rotation = ss->rotation;

   for (i=0; i<=setup_attrs[ss->kind].setup_limits; i++) {
      if (ss->people[i].id1) {
         int rot = 0;
         uint32 st = ((uint32) stb_z)*DBSTAB_BIT;

         if (!(callspec->callflagsf & CFLAGH__REQUIRES_SELECTOR) || selectp(ss, i)) {
            switch (ss->people[i].id1 & ROLL_MASK) {
               case ROLLBITL: rot = 033, st = ((uint32) stb_a)*DBSTAB_BIT; break;
               case ROLLBITM: break;
               case ROLLBITR: rot = 011; st = ((uint32) stb_c)*DBSTAB_BIT; break;
               default: fail("Roll not permitted after previous call.");
            }
         }
         install_rot(result, i, ss, i, rot);

         if (result->people[i].id1 & STABLE_ENAB) {
            do_stability(&result->people[i].id1, (stability) ((st/DBSTAB_BIT) & 0xF), rot);
         }
      }
      else
         clear_person(result, i);
   }
}


Private void do_inheritance(setup_command *cmd, Const callspec_block *parent_call, Const by_def_item *defptr)
{
   /* Strip out those concepts that do not have the "dfm__xxx" flag set saying that
      they are to be inherited to this part of the call.  BUT: the "INHERITFLAG_LEFT"
      flag controls both "INHERITFLAG_REVERSE" and "INHERITFLAG_LEFT", turning the former
      into the latter.  This makes reverse circle by, touch by, and clean sweep work. */

   /* If this subcall has "inherit_reverse" or "inherit_left" given, but the top-level call
      doesn't permit the corresponding flag to be given, we should turn any "reverse" or
      "left" modifier that was given into the other one, and cause that to be inherited.
      This is what turns, for example, part 3 of "*REVERSE* clean sweep" into a "*LEFT*
      1/2 tag". */

   uint32 callflagsh = parent_call->callflagsh;
   uint32 temp_concepts = cmd->cmd_final_flags.her8it;
   uint32 forcing_concepts = defptr->modifiersh & ~callflagsh;

   if (forcing_concepts & (INHERITFLAG_REVERSE | INHERITFLAG_LEFT)) {
      if (cmd->cmd_final_flags.her8it & (INHERITFLAG_REVERSE | INHERITFLAG_LEFT))
         temp_concepts |= (INHERITFLAG_REVERSE | INHERITFLAG_LEFT);
   }

   /* Pass any "inherit" flags.  That is, turn off any that are NOT to be inherited.
      Flags to be inherited are indicated by "modifiersh" and "callflagsh" both on.
      But we don't check "callflagsh", since, if it is off, we will force the bit
      immediately below. */

   /* But "half" and "lasthalf" are ALWAYS inherited.  (They can be forced, too.) */

   temp_concepts &= (~cmd->cmd_final_flags.her8it | defptr->modifiersh | INHERITFLAG_HALF | INHERITFLAG_LASTHALF);

   /* Now turn on any "force" flags.  These are indicated by "modifiersh" on
      and "callflagsh" off. */

   if (temp_concepts & defptr->modifiersh & ~callflagsh & (INHERITFLAG_HALF | INHERITFLAG_LASTHALF))
      fail("Can't do this with this fraction.");   /* "force_half" was used when we already had "half" coming in. */

   if (((INHERITFLAG_REVERSE | INHERITFLAG_LEFT) & callflagsh) == 0)
      /* If neither of the "reverse_means_mirror" or "left_means_mirror" bits is on,
         we allow forcing of left or reverse. */
      temp_concepts |= forcing_concepts;
   else
      /* Otherwise, we only allow the other bits. */
      temp_concepts |= forcing_concepts & ~(INHERITFLAG_REVERSE | INHERITFLAG_LEFT);

   cmd->cmd_final_flags.her8it = temp_concepts;
   cmd->callspec = base_calls[defptr->call_id];
}

extern void process_number_insertion(uint32 mod_word)
{
   if (mod_word & DFM1_NUM_INSERT_MASK) {
      uint32 insertion_num = (mod_word & DFM1_NUM_INSERT_MASK) / DFM1_NUM_INSERT_BIT;

      if ((mod_word & DFM1_FRACTAL_INSERT) && (insertion_num & ~2) == 1)
         insertion_num ^= 2;

      current_options.number_fields <<= 4;
      current_options.number_fields += insertion_num;
      current_options.howmanynumbers++;
   }

   current_options.number_fields >>= ((DFM1_NUM_SHIFT_MASK & mod_word) / DFM1_NUM_SHIFT_BIT) * 4;
   current_options.howmanynumbers -= ((DFM1_NUM_SHIFT_MASK & mod_word) / DFM1_NUM_SHIFT_BIT);
}



Private long_boolean get_real_subcall(
   parse_block *parseptr,
   Const by_def_item *item,
   setup_command *cmd_in,
   Const callspec_block *parent_call,
   setup_command *cmd_out)         /* We fill in just the parseptr, callspec,
                                      cmd_final_flags fields. */
{
   parse_block *search;
   parse_block **newsearch;
   int snumber;
   int item_id = item->call_id;
   /* The flags, heritable and otherwise, with which the parent call was invoked.
      Some of these may be inherited to the subcall. */
   uint64 new_final_concepts = cmd_in->cmd_final_flags;
   uint32 mods1 = item->modifiers1;
   callspec_block *orig_call = base_calls[item_id];
   long_boolean this_is_tagger =
      item_id >= base_call_tagger0 && item_id < base_call_tagger0+NUM_TAGGER_CLASSES;
   long_boolean this_is_tagger_circcer = this_is_tagger || item_id == base_call_circcer;
   call_conc_option_state save_state = current_options;

   if (!(TEST_HERITBITS(new_final_concepts,INHERITFLAG_FRACTAL)))
      mods1 &= ~DFM1_FRACTAL_INSERT;

   process_number_insertion(mods1);

   /* Fill in defaults in case we choose not to get a replacement call. */

   cmd_out->parseptr = parseptr;
   cmd_out->cmd_final_flags = new_final_concepts;

   /* If this context requires a tagging or scoot call, pass that fact on. */
   if (this_is_tagger) cmd_out->cmd_final_flags.final |= FINAL__MUST_BE_TAG;

   do_inheritance(cmd_out, parent_call, item);

   /* Do the substitutions called for by star turn replacements (from "@S" escape codes.) */

   if (current_options.star_turn_option != 0 && orig_call->callflags1 & CFLAG1_IS_STAR_CALL) {
      parse_block *xx = (*the_callback_block.get_parse_block_fn)();
      xx->concept = &marker_concept_mod;
      xx->options = current_options;
      xx->options.star_turn_option = 0;
      xx->replacement_key = 0;
      xx->call = orig_call;
      xx->options.number_fields &= ~0xF;
      xx->no_check_call_level = TRUE;
      xx->call_to_print = xx->call;

      if (current_options.star_turn_option >= 0)
         xx->options.number_fields += current_options.star_turn_option;

      cmd_out->parseptr = xx;
      cmd_out->callspec = (callspec_block *) 0;
      goto ret_true;
   }

   /* If this subcall invocation does not permit modification under any value of the
      "allowing_modifications" switch, we do nothing.  Just return the default.
      We do not search the list.  Hence, such subcalls are always protected
      from being substituted, and, if the same call appears multiple times
      in the derivation tree of a compound call, it will never be replaced.
      What do we mean by that?  Suppose we did a
         "[tally ho but [2/3 recycle]] the difference".
      After we create the table entry saying to change cast off 3/4 into 2/3
      recycle, we wouldn't want the cast off's in the difference getting
      modified.  Actually, that is not the real reason.  The casts are in
      different sublists.  The real reason is that the final part of mixed-up
      square thru is defined as
         conc nullcall [mandatory_anycall] nullcall []
      and we don't want the unmodifiable nullcall that the ends are supposed to
      do getting modified, do we? */

   /* But if this is a tagging call substitution, we most definitely do
      proceed with the search. */

   if (!(mods1 & DFM1_CALL_MOD_MASK) && !this_is_tagger_circcer)
      goto ret_false;

   /* See if we had something from before.  This avoids embarassment if a call is actually
      done multiple times.  We want to query the user just once and use that result everywhere.
      We accomplish this by keeping a subcall list showing what modifications the user
      supplied when we queried. */

   snumber = (mods1 & DFM1_CALL_MOD_MASK) / DFM1_CALL_MOD_BIT;

   /* We ALWAYS search this list, if such exists.  Even if modifications are disabled.
      Why?  Because the reconciler re-executes calls after the point of insertion to test
      their fidelity with the new setup that results from the inserted calls.  If those
      calls have modified subcalls, we will find ourselves here, looking through the list.
      Modifications may have been enabled at the time the call was initially entered, but
      might not be now that we are being called from the reconciler. */

   newsearch = &parseptr->next;

   if (parseptr->concept->kind == concept_another_call_next_mod) {
      while ((search = *newsearch) != (parse_block *) 0) {
         if (  orig_call == search->call ||
               (this_is_tagger && search->call == base_calls[base_call_tagger0])) {
            /* Found a reference to this call. */
            parse_block *subsidiary_ptr = search->subsidiary_root;

            /* If the pointer is nil, we already asked about this call,
               and the reply was no. */
            if (!subsidiary_ptr)
               goto ret_false;
            
            cmd_out->parseptr = subsidiary_ptr;

            if (this_is_tagger_circcer)
               cmd_out->callspec = subsidiary_ptr->call;
            else {
               /* This makes it pick up the substituted call. */
               cmd_out->callspec = (callspec_block *) 0;

               /* Substitutions are normal, and hence we clear the modifier bits,
                  unless this was a "mandatory_anycall" or "mandatory_secondary_call",
                  in which case we assume that the database tells just what bits
                  to inherit. */
               if (snumber == 2 || snumber == 6) {
                  if (search->concept->kind == concept_supercall) {
                     cmd_out->cmd_final_flags.her8it = cmd_in->restrained_super8flags;
                     cmd_out->cmd_misc2_flags &= ~CMD_MISC2_RESTRAINED_SUPER;
                  }
               }
               else {
                  cmd_out->cmd_final_flags.her8it = 0;        /* ****** not right????. */
                  cmd_out->cmd_final_flags.final = 0;        /* ****** not right????. */
               }
            }

            goto ret_true;
         }

         newsearch = &search->next;
      }
   }
   else if (parseptr->concept->kind != marker_end_of_list)
      fail("wrong marker in get_real_subcall???");

   if ((*the_callback_block.do_subcall_query_fn)
       (snumber, parseptr, newsearch, this_is_tagger, this_is_tagger_circcer, orig_call))
      goto ret_false;

   cmd_out->parseptr = (*newsearch)->subsidiary_root;
   cmd_out->callspec = (callspec_block *) 0;    /* We THROW AWAY the alternate call, because we want our user to get it from the concept list. */
   cmd_out->cmd_final_flags.her8it = 0;
   cmd_out->cmd_final_flags.final = 0;

 ret_true:
   current_options = save_state;
   return TRUE;

 ret_false:
   current_options = save_state;
   return FALSE;
}


Private void divide_diamonds(setup *ss, setup *result)
{
   uint32 ii;

   if (ss->kind == s_qtag) ii = MAPCODE(sdmd,2,MPKIND__SPLIT,1);
   else if (ss->kind == s_ptpd) ii = MAPCODE(sdmd,2,MPKIND__SPLIT,0);
   else fail("Must have diamonds for this concept.");

   new_divided_setup_move(ss, ii, phantest_ok, FALSE, result);
}


/* a <= b */
extern int gcd(int a, int b)
{
   for (;;) {
      int rem;
      if (a==0) return b;
      rem = b % a;
      if (rem==0) return a;
      b = a;
      a = rem;
   }
}


/* The fraction stuff is encoded into 6 hexadecimal digits, which we will call
   digits 3 through 8:

      Late-breaking news:  digit 2 is now used.  The bit 0x01000000 means
      "this is 'initially' or 'finally', and I promise to invoke all parts
      of the call, even though I appear at present to be invoking just a
      subset."  It is used to allow 'initially' and 'finally' to be stacked
      with themselves.

      digit 3 - the code (3 bits) and the reversal bit (1 bit)
                  Note that the code*2 is what is visible in this digit.
                  If this digit is odd, the reversal bit is on.  The code
                  indicates what kind of special job we are doing.
      digit 4 - the selected part, called "N".  Think of this as being 1-based,
                  even though the part numbering stuff that comes out of this
                  procedure will be zero-based.  This will always be nonzero
                  when some nontrivial job is being done.  the meaning of the
                  codes has been defined so that no job ever requires N to be zero.
                  The only time N is zero is when we are not doing anything, in
                  which case the code is zero.  Code = N = 0 means no special
                  job is being done.  (Though reversal and nontrivial fractions
                  may still be present.)
      digits 5 and 6 - the numerator and denominator, respectively, of the
                  start point of the call, always in reduced-fraction form.
                  The default is [0,1] meaning to start at 0.0000, that is, the
                  beginning.
      digits 7 and 8 - the numerator and denominator, respectively, of the
                  end point of the call, always in reduced-fraction form.
                  The default is [1,1] meaning to end at 1.0000, that is, the
                  end.

   The default configuration is 000111 hex, meaning do not reverse, go from
   0.0000 to 1.0000, and don't do any special job.  A word of zero means the same
   thing, and the fraction flag word is initialized to 0 if no fractions are being
   used.  This makes it easier to tell whether any fractions are being used.
   The zero is changed to 000111 hex at the start of any fraction-manipulation
   operation.

   The meaning of all this stuff is as follows:

   First, if the reversal bit is on, the target call (or target random crazy
   call, or whatever) has its parts reversed.

   Second, the fraction stuff, determined by the low 4 hexadecimal digits, is
   applied to the possibly reversed call.  This creates the "region".

   Third, the region is manipulated according to the key and the value of N.

   Before describing the key actions, here is how the region is computed.

      Suppose the call has 10 parts: A, B, C, D, E, F, G, H, I, and J, and that
      the low 4 digits of the fraction word are 1535.  This means that the start
      point is 1/5 and the end point is 3/5.  So the first 1/5 of the call (2 parts,
      A and B) are chopped off, and the last 2/5 of the call (4 parts, G, H, I, and J)
      are also chopped off.  The region is C, D, E, and F.  This effect could be
      obtained by saying "do the last 2/3, 3/5, <call>" or "1/2, do the last 4/5, <call>".

      Now suppose that the "reverse" bit is also on.  This reverses the parts BEFORE
      the fractions are computed.  The call is considered to consist of J, I, H, G, F,
      E, D, C, B, and A.  If the low 4 digits are 1535, this chops off J and I at
      the beginning, and D, C, B, and A at the end.  The region is thus H, G, F, and E.
      This could be obtained by saying "do the last 2/3, 3/5, reverse order, <call>",
      or "reverse order, 2/3, do the last 3/5, <call>", or "1/2, do the last 4/5,
      reverse order, <call>", etc.

   What happens next depends on the key and the value of "N", but always applies just
   to the region as though the region were the only thing under consideration.

   If "N" is zero (and the key is zero also, of course), the region is simply
   executed from beginning to end.

   Otherwise, the parts of the region are considered to be numbered in a 1-based
   fashion.  In the case above of the "reverse" bit being on, the low 4 digits being
   1535 hex, and the call having 10 parts, the region was H, G, F, and E.  These
   are numbered:
      H = 1
      G = 2
      F = 3
      E = 4

   Let T be the number of parts in the region.  In this example, T is 4.

   The meanings of the key values are as follows.  These must only be used
      when "N" (the selected part, the stuff in CMD_FRAC_PART_MASK) is nonzero!
      When "N" is zero, the key must be zero (which happens to be CMD_FRAC_CODE_ONLY,
      but it doesn't matter.)

      CMD_FRAC_CODE_ONLY
               Do part N only.  Do 1 part.  For a 5-part call ABCDE:
                  ONLY(1) means do A
                  ONLY(4) means do D

      CMD_FRAC_CODE_ONLYREV
               Do part N from the end.  Do 1 part.  For a 5-part call ABCDE:
                  ONLYREV(1) means do E
                  ONLYREV(4) means do B

      CMD_FRAC_CODE_FROMTO
                  do parts from K+1 up through N, inclusive,
                  that is, K+1, K+2, K+3, .... N.  Relative to the region.
                  K is the "secondary part number".  If K=0, this does N parts:
                  from the beginning of the region up through N, inclusive.                  
                  For a 5-part call ABCDE:
                  FROMTO(K=0,N=1) means do A
                  FROMTO(K=0,N=4) means do ABCD
                  FROMTO(K=1,N=4) means do BCD

      CMD_FRAC_CODE_FROMTOREV
               Do parts from N up through size-K, inclusive.
               For a 5-part call ABCDE:
                  FROMTOREV(K=0,N=1) means do whole thing
                  FROMTOREV(K=0,N=2) means do BCDE
                  FROMTOREV(K=0,N=5) means do E
                  FROMTOREV(K=1,N=1) means do ABCD
                  FROMTOREV(K=1,N=2) means do BCD
                  FROMTOREV(K=3,N=2) means do B
                  FROMTOREV(K=4,N=1) means do A

      CMD_FRAC_CODE_FROMTOREVREV
               Do parts from size-N-1 up through size-K, inclusive.
               Do N-K parts -- the last N parts, but omitting the last K parts.
               For a 5-part call ABCDE:
                  FROMTOREVREV(K=0,N=5) means do whole thing
                  FROMTOREVREV(K=0,N=4) means do BCDE
                  FROMTOREVREV(K=0,N=1) means do E
                  FROMTOREVREV(K=1,N=5) means do ABCD
                  FROMTOREVREV(K=1,N=4) means do BCD
                  FROMTOREVREV(K=3,N=4) means do B
                  FROMTOREVREV(K=4,N=5) means do A

      CMD_FRAC_CODE_FROMTOMOST
                  Same as FROMTO, but leave early from the last part.
                  Do just the first half of it.
                  For a 5-part call ABCDE:
                  FROMTO(K=1,N=4) means do BC and the first half of D

      CMD_FRAC_CODE_LATEFROMTOREV
                  Same as FROMTOREV, but get a late start on the first part.
                  Do just the last half of it.
               For a 5-part call ABCDE:
                  LATEFROMTOREV(K=1,N=2) means do the last half of B, then CD

   The output of this procedure, after digesting the above, is a "fraction_info"
   structure, whose important items are:

      subcall_index - the first part of the target call that we are to execute.
         This is now in ZERO-BASED numbering, as befits the sequential-definition
         array.
      highlimit - if going forward, this is the (zero-based) part JUST AFTER the
         last part that we are to execute.  We will execute (highlimit-subcall_index)
         parts.  If going backward, this is the (zero-based) LAST PART that we will
         execute.  We will execute (subcall_index-highlimit+1) parts.  Note the
         assymetry.  Sorry.

   So, to execute all of a 10 part call in forward order, we will have
      subcall_index = 0    and    highlimit = 10
   To execute it in reverse order, we will have
      subcall_index = 9    and    highlimit = 0

   The "instant_stop" and "do_half_of_last_part" flags do various things.
*/


extern uint32 process_new_fractions(
   int numer,
   int denom,
   uint32 incoming_fracs,
   uint32 reverse_orderbit,   /* Low bit on mean treat as if we mean "do the last M/N". */
   long_boolean allow_improper,
   long_boolean *improper_p)
{
   int s_numer, s_denom, e_numer, e_denom, divisor;

   *improper_p = FALSE;

   s_numer = (incoming_fracs & 0xF000) >> 12;        /* Start point. */
   s_denom = (incoming_fracs & 0xF00) >> 8;
   e_numer = (incoming_fracs & 0xF0) >> 4;          /* Stop point. */
   e_denom = (incoming_fracs & 0xF);

   /* Xor the "reverse" bit with the first/last fraction indicator. */
   if ((reverse_orderbit ^ (incoming_fracs / CMD_FRAC_REVERSE)) & 1) {
      /* This is "last fraction". */
      s_numer = s_numer*numer + s_denom*(denom-numer);
      e_numer = e_numer*numer + e_denom*(denom-numer);
   }
   else {
      /* This is "fractional". */
      s_numer *= numer;
      e_numer *= numer;
   }

   s_denom *= denom;
   e_denom *= denom;

   if (allow_improper && e_numer > e_denom) {
      *improper_p = TRUE;
      e_numer -= e_denom;
   }

   if (s_numer < 0 || s_numer >= s_denom || e_numer <= 0 || e_numer > e_denom)
      fail("Illegal fraction.");

   divisor = gcd(s_numer, s_denom);
   s_numer /= divisor;
   s_denom /= divisor;

   divisor = gcd(e_numer, e_denom);
   e_numer /= divisor;
   e_denom /= divisor;

   if (s_numer > 15 || s_denom > 15 || e_numer > 15 || e_denom > 15)
      fail("Fractions are too complicated.");

   return (s_numer<<12) | (s_denom<<8) | (e_numer<<4) | e_denom;
}


extern void get_fraction_info(
   uint32 frac_flags,
   uint32 callflags1,
   int total,
   fraction_info *zzz)
{
   int e_numer, e_denom, s_numer, s_denom, this_part, divisor, test_num;
   int subcall_index, highlimit;
   uint32 last_half_stuff = 0;
   uint32 first_half_stuff = 0;

   int available_fractions = (callflags1 & CFLAG1_VISIBLE_FRACTION_MASK) / CFLAG1_VISIBLE_FRACTION_BIT;
   if (available_fractions == 3 || (frac_flags & CMD_FRAC_FORCE_VIS)) available_fractions = 1000;     /* 3 means all parts. */


   zzz->reverse_order = FALSE;
   zzz->instant_stop = 0;
   zzz->do_half_of_last_part = 0;
   zzz->do_last_half_of_first_part = 0;

   this_part = (frac_flags & CMD_FRAC_PART_MASK) / CMD_FRAC_PART_BIT;
   s_numer = (frac_flags & 0xF000) >> 12;      /* Start point. */
   s_denom = (frac_flags & 0xF00) >> 8;
   e_numer = (frac_flags & 0xF0) >> 4;         /* End point. */
   e_denom = (frac_flags & 0xF);

   if (s_numer >= s_denom) fail("Fraction must be proper.");
   subcall_index = total * s_numer;
   test_num = subcall_index / s_denom;

   if (test_num*s_denom != subcall_index) {
      divisor = (test_num == 0) ? gcd(subcall_index, s_denom) : gcd(s_denom, subcall_index);
      s_denom /= divisor;
      subcall_index /= divisor;
      last_half_stuff = subcall_index - test_num * s_denom;   /* We will need this if we have
                                                                 to reverse the order. */
      zzz->do_last_half_of_first_part = (last_half_stuff << 12) | (s_denom << 8) | 0x11;
      if (zzz->do_last_half_of_first_part != CMD_FRAC_LASTHALF_VALUE)
         warn(warn_hairy_fraction);
   }

   subcall_index = test_num;

   if (e_numer <= 0) fail("Fraction must be proper.");
   highlimit = total * e_numer;
   test_num = highlimit / e_denom;

   if (test_num*e_denom != highlimit) {
      divisor = (test_num == 0) ? gcd(highlimit, e_denom) : gcd(e_denom, highlimit);
      highlimit /= divisor;
      e_denom /= divisor;
      first_half_stuff = highlimit-e_denom*test_num;
      zzz->do_half_of_last_part = 0x0100 | (first_half_stuff << 4) | e_denom;
      if (zzz->do_half_of_last_part != CMD_FRAC_HALF_VALUE)
         warn(warn_hairy_fraction);
      test_num++;
   }

   highlimit = test_num;

   /* Now subcall_index is the start point, and highlimit is the end point. */

   if (subcall_index >= highlimit || highlimit > total)
      fail("Fraction must be proper.");

   /* Check for "reverse order" */
   if (frac_flags & CMD_FRAC_REVERSE) {
      uint32 orig_last = zzz->do_last_half_of_first_part;
      long_boolean dont_clobber = FALSE;

      zzz->reverse_order = TRUE;
      subcall_index = total-1-subcall_index;
      highlimit = total-highlimit;

      if (zzz->do_half_of_last_part) {
         zzz->do_last_half_of_first_part =
            ((e_denom - first_half_stuff) << 12) | (e_denom << 8) | 0x11;
         zzz->do_half_of_last_part = 0;
         dont_clobber = TRUE;
      }

      if (orig_last) {
         zzz->do_half_of_last_part = 0x0100 | ((s_denom - last_half_stuff) << 4) | s_denom;
         if (!dont_clobber) zzz->do_last_half_of_first_part = 0;
      }
   }

   if (this_part != 0) {
      int highdel;
      uint32 kvalue = ((frac_flags & CMD_FRAC_PART2_MASK) / CMD_FRAC_PART2_BIT);

      /* In addition to everything else, we are picking out a specific part
         of whatever series we have decided upon. */

      if (zzz->do_half_of_last_part | zzz->do_last_half_of_first_part)
         fail("This call can't be fractionalized with this fraction.");

      switch (frac_flags & CMD_FRAC_CODE_MASK) {
      case CMD_FRAC_CODE_ONLY:
         zzz->instant_stop = 1;
         subcall_index += zzz->reverse_order ? (1-this_part) : (this_part-1);
         /* Be sure that enough parts are visible. */
         if (subcall_index >= available_fractions)
            fail("This call can't be fractionalized.");
         if (subcall_index >= total)
            fail("The indicated part number doesn't exist.");

         /* If "K" (the secondary part number) is nonzero,
            shorten highlimit by that amount. */
         if (zzz->reverse_order) {
            highlimit += kvalue;
            if (highlimit > subcall_index)
               fail("The indicated part number doesn't exist.");
         }
         else {
            highlimit -= kvalue;
            if (highlimit <= subcall_index)
               fail("The indicated part number doesn't exist.");
         }

         break;
      case CMD_FRAC_CODE_ONLYREV:
         zzz->instant_stop = 1;
         subcall_index = zzz->reverse_order ? (highlimit-1+this_part) : (highlimit-this_part);
         /* Be sure that enough parts are visible. */
         if (subcall_index >= available_fractions)
            fail("This call can't be fractionalized.");
         if (subcall_index >= total) fail("The indicated part number doesn't exist.");
         break;
      case CMD_FRAC_CODE_FROMTO:

         /* We are doing parts from (K+1) through N. */

         if (zzz->reverse_order) {
            highdel = subcall_index-this_part+1;

            if (highdel < highlimit   )
               fail("This call can't be fractionalized this way.");

            highlimit = highdel;
            subcall_index -= kvalue;

            if (subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (subcall_index > total)
               fail("The indicated part number doesn't exist.");
         }
         else {
            highdel = highlimit-subcall_index-this_part;
            if (highdel < 0)
               fail("This call can't be fractionalized this way.");

            highlimit -= highdel;
            subcall_index += kvalue;

            if (highlimit > available_fractions || subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (highlimit > total || subcall_index > total)
               fail("The indicated part number doesn't exist.");
         }

         break;
      case CMD_FRAC_CODE_FROMTOREV:

         /* We are doing parts from N through (end-K). */

         if (zzz->reverse_order) {
            int lowdel = 1-this_part;

            highlimit += kvalue;
            subcall_index += lowdel;

            /* Be sure that enough parts are visible, and that we are within bounds.
               If lowdel is zero, we weren't cutting the upper limit, so it's
               allowed to be beyond the limit of part visibility. */

            if (subcall_index > total)
               fail("The indicated part number doesn't exist.");

            if ((subcall_index >= available_fractions && lowdel != 0) ||
                highlimit > available_fractions)
               fail("This call can't be fractionalized.");
         }
         else {
            highlimit -= kvalue;
            subcall_index += this_part-1;

            /* Be sure that enough parts are visible, and that we are within bounds.
               If kvalue is zero, we weren't cutting the upper limit, so it's
               allowed to be beyond the limit of part visibility. */

            if ((highlimit > available_fractions && kvalue != 0) ||
                subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (highlimit > total || subcall_index > total)
               fail("The indicated part number doesn't exist.");
         }

         break;
      case CMD_FRAC_CODE_FROMTOREVREV:

         /* We are doing parts from (end-N+1 through (end-K). */

         if (zzz->reverse_order) {
            int highdel = subcall_index+1-highlimit-this_part;

            highlimit += kvalue;
            subcall_index -= highdel;

            /* Be sure that enough parts are visible, and that we are within bounds.
               If lowhdel is zero, we weren't cutting the upper limit, so it's
               allowed to be beyond the limit of part visibility. */

            if (subcall_index > total)
               fail("The indicated part number doesn't exist.");

            if ((subcall_index >= available_fractions && highdel != 0) ||
                highlimit > available_fractions)
               fail("This call can't be fractionalized.");
         }
         else {
            subcall_index = highlimit-this_part;
            highlimit -= kvalue;

            /* Be sure that enough parts are visible, and that we are within bounds.
               If kvalue is zero, we weren't cutting the upper limit, so it's
               allowed to be beyond the limit of part visibility. */

            if ((highlimit > available_fractions && kvalue != 0) ||
                subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (highlimit > total || subcall_index > total)
               fail("The indicated part number doesn't exist.");
         }

         break;
      case CMD_FRAC_CODE_FROMTOMOST:

         /* We are doing parts from (K+1) through N, but only doing half of part N. */

         if (zzz->reverse_order) {
            highdel = (subcall_index-this_part+1);

            if (highdel < highlimit)
               fail("This call can't be fractionalized this way.");

            highlimit = highdel;
            subcall_index -= kvalue;
            zzz->do_last_half_of_first_part = CMD_FRAC_HALF_VALUE;

            if (subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (subcall_index > total)
               fail("The indicated part number doesn't exist.");
            /* Be sure that we actually do the part that we take half of. */
            if (subcall_index < highlimit)
               fail("This call can't be fractionalized this way.");
         }
         else {
            highdel = highlimit-subcall_index-this_part;
            if (highdel < 0)
               fail("This call can't be fractionalized this way.");

            highlimit -= highdel;
            subcall_index += kvalue;
            zzz->do_half_of_last_part = CMD_FRAC_HALF_VALUE;

            if (highlimit > available_fractions || subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (highlimit > total || subcall_index > total)
               fail("The indicated part number doesn't exist.");
            /* Be sure that we actually do the part that we take half of. */
            if (subcall_index >= highlimit)
               fail("This call can't be fractionalized this way.");
         }

         break;
      case CMD_FRAC_CODE_LATEFROMTOREV:

         /* Like FROMTOREV, but get a late start on the first part. */

         if (zzz->reverse_order) {
            int lowdel = 1-this_part;

            highlimit += kvalue;
            subcall_index += lowdel;
            zzz->do_half_of_last_part = CMD_FRAC_LASTHALF_VALUE;

            /* Be sure that enough parts are visible, and that we are within bounds.
               If lowdel is zero, we weren't cutting the upper limit, so it's
               allowed to be beyond the limit of part visibility. */

            if (subcall_index > total)
               fail("The indicated part number doesn't exist.");

            if ((subcall_index >= available_fractions && lowdel != 0) ||
                highlimit > available_fractions)
               fail("This call can't be fractionalized.");
            /* Be sure that we actually do the part that we take half of. */
            if (subcall_index < highlimit)
               fail("This call can't be fractionalized this way.");
         }
         else {
            highlimit -= kvalue;
            subcall_index += this_part-1;
            zzz->do_last_half_of_first_part = CMD_FRAC_LASTHALF_VALUE;

            /* Be sure that enough parts are visible, and that we are within bounds.
               If kvalue is zero, we weren't cutting the upper limit, so it's
               allowed to be beyond the limit of part visibility. */

            if ((highlimit > available_fractions && kvalue != 0) ||
                subcall_index > available_fractions)
               fail("This call can't be fractionalized.");
            if (highlimit > total || subcall_index > total)
               fail("The indicated part number doesn't exist.");
            /* Be sure that we actually do the part that we take half of. */
            if (subcall_index >= highlimit)
               fail("This call can't be fractionalized with this fraction.");
         }
         break;
      default:
         fail("Internal error: bad fraction code.");
      }
   }
   else {
      /* Unless all parts are visible, this is illegal. */
      if (available_fractions != 1000)
         fail("This call can't be fractionalized.");
   }

   if (frac_flags & CMD_FRAC_FIRSTHALF_ALL) {
      int diff = highlimit - subcall_index;

      if (zzz->reverse_order | zzz->do_half_of_last_part | zzz->do_last_half_of_first_part)
         fail("Can't do this fractional \"snag\".");

      if (zzz->instant_stop || diff <= 0 || (diff & 1))
         fail("Can't do this fractional \"snag\".");
      highlimit -= diff >> 1;
   }
   else if (frac_flags & CMD_FRAC_LASTHALF_ALL) {
      int diff = highlimit - subcall_index;

      if (zzz->reverse_order | zzz->do_half_of_last_part | zzz->do_last_half_of_first_part)
         fail("Can't do this fractional \"snag\".");

      if (zzz->instant_stop || diff <= 0 || (diff & 1))
         fail("Can't do this fractional \"snag\".");
      subcall_index += diff >> 1;
   }

   zzz->subcall_index = subcall_index;
   zzz->highlimit = highlimit;
   zzz->subcall_incr = zzz->reverse_order ? -1 : 1;
   if (zzz->instant_stop != 0)
      zzz->instant_stop = zzz->subcall_index*zzz->subcall_incr+1;
   else
      zzz->instant_stop = 99;
}



/* This returns TRUE if it can't do it because the assumption isn't specific enough.
   In such a case, the call was not executed.  If the user had said "with active phantoms",
   that is an error.  But if we are only doing this because the automatic active phantoms
   switch is on, we will just ignore it. */

extern long_boolean fill_active_phantoms_and_move(setup *ss, setup *result)
{
   int i;

   if (check_restriction(ss, ss->cmd.cmd_assume, TRUE, 99))
      return TRUE;   /* We couldn't do it -- the assumption is not specific enough, like "general diamonds". */

   ss->cmd.cmd_assume.assumption = cr_none;
   move(ss, FALSE, result);

   /* Take out the phantoms. */

   for (i=0; i<=setup_attrs[result->kind].setup_limits; i++) {
      if (result->people[i].id1 & BIT_ACT_PHAN)
         result->people[i].id1 = 0;
   }

   return FALSE;
}



extern void move_perhaps_with_active_phantoms(setup *ss, setup *result)
{
   if (using_active_phantoms) {
      if (fill_active_phantoms_and_move(ss, result)) {
         /* Active phantoms couldn't be used.  Just do the call the way it is.
            This does not count as a use of active phantoms, so don't set the flag. */
         move(ss, FALSE, result);
      }
      else
         result->result_flags |= RESULTFLAG__ACTIVE_PHANTOMS_ON;
   }
   else {
      (void) check_restriction(ss, ss->cmd.cmd_assume, FALSE, 99);
      move(ss, FALSE, result);
      result->result_flags |= RESULTFLAG__ACTIVE_PHANTOMS_OFF;
   }
}



extern void impose_assumption_and_move(setup *ss, setup *result)
{
   if (ss->cmd.cmd_misc_flags & CMD_MISC__VERIFY_MASK) {
      assumption_thing t;

      /* **** actually, we want to allow the case of "assume waves" already in place. */
      if (ss->cmd.cmd_assume.assumption != cr_none)
         fail("Redundant or conflicting assumptions.");

      t.assump_col = 0;
      t.assump_both = 0;
      t.assump_cast = ss->cmd.cmd_assume.assump_cast;
      t.assump_live = 0;
      t.assump_negate = 0;

      switch (ss->cmd.cmd_misc_flags & CMD_MISC__VERIFY_MASK) {
      case CMD_MISC__VERIFY_WAVES:         t.assumption = cr_wave_only;     break;
      case CMD_MISC__VERIFY_2FL:           t.assumption = cr_2fl_only;      break;
      case CMD_MISC__VERIFY_DMD_LIKE:      t.assumption = cr_diamond_like;  break;
      case CMD_MISC__VERIFY_QTAG_LIKE:     t.assumption = cr_qtag_like;     break;
      case CMD_MISC__VERIFY_1_4_TAG:
         t.assumption = cr_qtag_like;
         t.assump_both = 1;
         break;
      case CMD_MISC__VERIFY_3_4_TAG:
         t.assumption = cr_qtag_like;
         t.assump_both = 2;
         break;
      case CMD_MISC__VERIFY_REAL_1_4_TAG:  t.assumption = cr_real_1_4_tag;  break;
      case CMD_MISC__VERIFY_REAL_3_4_TAG:  t.assumption = cr_real_3_4_tag;  break;
      case CMD_MISC__VERIFY_REAL_1_4_LINE: t.assumption = cr_real_1_4_line; break;
      case CMD_MISC__VERIFY_REAL_3_4_LINE: t.assumption = cr_real_3_4_line; break;
      case CMD_MISC__VERIFY_LINES:         t.assumption = cr_all_ns;        break;
      case CMD_MISC__VERIFY_COLS:          t.assumption = cr_all_ew;        break;
      default:
         fail("Unknown assumption verify code.");
      }

      ss->cmd.cmd_assume = t;
      ss->cmd.cmd_misc_flags &= ~CMD_MISC__VERIFY_MASK;
      move_perhaps_with_active_phantoms(ss, result);
   }
   else
      move(ss, FALSE, result);
}




Private void do_sequential_call(
   setup *ss,
   Const callspec_block *callspec,
   long_boolean qtfudged,
   long_boolean *mirror_p,
   setup *result)
{
   int fetch_index;
   int dist_index;
   int i;
   uint64 new_final_concepts = ss->cmd.cmd_final_flags;
   int *test_index = &fetch_index;
   parse_block *parseptr = ss->cmd.parseptr;
   uint32 callflags1 = callspec->callflags1;
   fraction_info zzz;
   long_boolean first_call = TRUE;    /* First call in logical definition. */
   long_boolean first_time = TRUE;    /* First thing we are doing, in temporal sequence. */
   call_restriction fix_next_assumption = cr_none;
   int fix_next_assump_col = 0;
   int fix_next_assump_both = 0;
   int realtotal = callspec->stuff.def.howmanyparts;
   int total = realtotal;
   int start_point;    /* Where we start, in the absence of special stuff. */
   int end_point;  /* Where we end, in the absence of special stuff. */
   long_boolean distribute = FALSE;
   /* This tells whether the setup was genuinely elongated when it came in.
      We keep track of pseudo-elongation during the call even when it wasn't,
      but sometimes we really need to know. */
   long_boolean setup_is_elongated =
      (ss->kind == s2x2 || ss->kind == s_short6) && (ss->cmd.prior_elongation_bits & 0x3F) != 0;
   int remembered_2x2_elongation = 0;
   int subpart_count = 0;

   zzz.instant_stop = 99;  /* If not 99, says to stop instantly after doing one part,
                              and to report (in RESULTFLAG__DID_LAST_PART bit)
                              whether that part was the last part. */
   zzz.do_half_of_last_part = 0;
   zzz.do_last_half_of_first_part = 0;

   /* If a restrained concept is in place, it is waiting for the call to be pulled apart
      into its pieces.  That is about to happen.  Turn off the restraint flag.
      That will be the signal to "move" that it should act on the concept. */

   ss->cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_CRAZINESS;

   if (callflags1 & CFLAG1_DISTRIBUTE_REPETITIONS) distribute = TRUE;

   if (distribute) {
      int ii;
      total = 0;
      for (ii=0 ; ii<callspec->stuff.def.howmanyparts ; ii++) {
         by_def_item *this_item = &callspec->stuff.def.defarray[ii];
         uint32 this_mod1 = this_item->modifiers1;

         if ((DFM1_SEQ_REPEAT_N | DFM1_SEQ_REPEAT_NM1 | DFM1_SEQ_REPEAT_N_ALTERNATE) & this_mod1) {
            uint32 local_number_fields = current_options.number_fields;

            total += local_number_fields & 0xF;

            if (this_mod1 & DFM1_SEQ_REPEAT_N_ALTERNATE) ii++;
            if (this_mod1 & DFM1_SEQ_DO_HALF_MORE) {
               total++;
               zzz.do_half_of_last_part = CMD_FRAC_HALF_VALUE;
            }
            if (this_mod1 & DFM1_SEQ_REPEAT_NM1) {
               if ((local_number_fields & 0xF) == 0) fail("Can't give number zero.");
               total--;
            }
         }
         else
            total++;
      }
      test_index = &dist_index;
   }

   /* Check for special behavior of "sequential_with_fraction". */

   if (callspec->schema == schema_sequential_with_fraction) {
      uint32 new_fracs;

      if ((ss->cmd.cmd_frac_flags & 0xFFFF) != CMD_FRAC_NULL_VALUE)
         fail("Fractions have been specified in two places.");

      if (current_options.number_fields == 0 || current_options.number_fields > 4)
         fail("Illegal fraction.");

      if (ss->cmd.cmd_frac_flags & CMD_FRAC_REVERSE) {
         new_fracs = 0x0411 | ((4-current_options.number_fields) << 12);
      }
      else {
         new_fracs = 0x0104 | (current_options.number_fields << 4);
      }

      ss->cmd.cmd_frac_flags = (ss->cmd.cmd_frac_flags & ~0xFFFF) | new_fracs;
   }

   zzz.reverse_order = FALSE;
   zzz.subcall_incr = 1;
   zzz.highlimit = total;
   zzz.subcall_index = 0;

   /* If the "cmd_frac_flags" word is not null, we are being asked to do something special.
      Otherwise, the defaults that we have placed into zzz will be used. */

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE) {
      if ((zzz.do_half_of_last_part | zzz.do_last_half_of_first_part) != 0)
         fail("Sorry, can't fractionalize this.");
      get_fraction_info(ss->cmd.cmd_frac_flags, callflags1, total, &zzz);
      if (zzz.reverse_order) {
         zzz.highlimit = 1-zzz.highlimit;
      }
      if (zzz.reverse_order && zzz.instant_stop == 99) first_call = FALSE;
   }

   start_point = zzz.subcall_index;

   if (new_final_concepts.final & FINAL__SPLIT) {
      if (callflags1 & CFLAG1_SPLIT_LIKE_SQUARE_THRU)
         new_final_concepts.final |= FINAL__SPLIT_SQUARE_APPROVED;
      else if (callflags1 & CFLAG1_SPLIT_LIKE_DIXIE_STYLE)
         new_final_concepts.final |= FINAL__SPLIT_DIXIE_APPROVED;
   }

   if (!first_time && ss->kind != s2x2 && ss->kind != s_short6)
      ss->cmd.prior_elongation_bits = 0;

   /* Did we neglect to do the touch/rear back stuff because fractionalization was enabled?
      If so, now is the time to correct that.  We only do it for the first part, and only if
      doing parts in forward order. */

   /* Test for all this is "random left, swing thru".
      The test cases for this stuff are such things as "left swing thru". */

   if (     !(ss->cmd.cmd_misc_flags & (CMD_MISC__NO_STEP_TO_WAVE | CMD_MISC__ALREADY_STEPPED)) &&
            (start_point == 0) &&
            !zzz.do_last_half_of_first_part &&
            !zzz.reverse_order &&
            (callflags1 & CFLAG1_STEP_REAR_MASK)) {

      if (TEST_HERITBITS(new_final_concepts,INHERITFLAG_LEFT)) {
         if (!*mirror_p) mirror_this(ss);
         *mirror_p = TRUE;
      }

      ss->cmd.cmd_misc_flags |= CMD_MISC__ALREADY_STEPPED;  /* Can only do it once. */
      touch_or_rear_back(ss, *mirror_p, callflags1);
   }

   if (callspec->schema == schema_sequential_with_split_1x8_id && ss->kind == s1x8) {
      for (i=0; i<8; i++) {
         if (ss->people[i].id1) ss->people[i].id2 |= (i&1) ? ID2_CENTER : ID2_END;
      }
   }

   /* See comment earlier about mirroring.  For sequentially or concentrically defined
      calls, the "left" flag does not mean mirror; it is simply passed to subcalls.
      So we must put things into their normal state.  If we did any mirroring, it was
      only to facilitate the action of "touch_or_rear_back". */

   if (*mirror_p) mirror_this(ss);
   *mirror_p = FALSE;

   prepare_for_call_in_series(result, ss);

   /* Iterate over the parts of the call.
      We will let "fetch_index" scan the actual call definition:
         forward - from 0 to realtotal-1 inclusive
         reverse - from realtotal-1 down to 0 inclusive.
      While doing this, we will let "test_index" scan the parts of the
      call as seen by the fracionalization stuff.  If we are not distributing
      parts, "test_index" will be the same as "fetch_index".  Otherwise,
      it will show the distributed subparts. */

   if (zzz.reverse_order) {
      fetch_index = distribute ? realtotal-1 : start_point;
      dist_index = total-1;
      end_point = -zzz.highlimit+1;
   }
   else {
      fetch_index = distribute ? 0 : start_point;
      dist_index = 0;
      end_point = zzz.highlimit-1;
   }

   for (;;) {
      by_def_item *this_item;
      uint32 this_mod1;
      uint32 remember_elongation;
      setup_command foo1, foo2;
      by_def_item *alt_item;
      int use_alternate;
      setup_command foobar;
      setup_kind oldk;
      long_boolean recompute_id = FALSE;
      uint32 saved_number_fields = current_options.number_fields;
      int saved_num_numbers = current_options.howmanynumbers;
      uint32 resultflags_to_put_in = 0;

      ss->cmd.prior_expire_bits |=
         result->result_flags & (RESULTFLAG__YOYO_FINISHED |
                                 RESULTFLAG__TWISTED_FINISHED |
                                 RESULTFLAG__SPLIT_FINISHED);

      /* Now the "index" values (fetch_index and dist_index) contain the
         number of parts we have completed.  That is, they point (in 0-based
         numbering) to what we are about to do.  Also, if "subpart_count" is
         nonzero, it has the number of extra repetitions of what we just did
         that we must perform before we can go on to the next thing. */

      if (subpart_count) {
         subpart_count--;    /* This is now the number of EXTRA repetitions
                                 of this that we will still have to do after
                                 we do the repetition that we are about to do. */

         use_alternate ^= 1;
         dist_index += zzz.subcall_incr;
         goto do_plain_call;
      }

      if (zzz.reverse_order) {
         if (fetch_index < 0) break;
         else if (fetch_index == 0) recompute_id = TRUE;
      }
      else {
         if (fetch_index >= realtotal) break;
      }

      if (fetch_index >= realtotal || fetch_index < 0) fail("The indicated part number doesn't exist.");
      this_item = &callspec->stuff.def.defarray[fetch_index];
      this_mod1 = this_item->modifiers1;

      if ((this_mod1 & DFM1_SEQ_NO_RE_EVALUATE) &&
          !(result->cmd.cmd_misc2_flags & CMD_MISC2_RESTRAINED_SUPER))
         result->result_flags |= RESULTFLAG__NO_REEVALUATE;

      fetch_index += zzz.subcall_incr;
      dist_index += zzz.subcall_incr;

      if (zzz.reverse_order) {
         if (fetch_index >= 0 && (DFM1_SEQ_REPEAT_N_ALTERNATE & callspec->stuff.def.defarray[fetch_index].modifiers1)) {
            alt_item = this_item;
            this_item = &callspec->stuff.def.defarray[fetch_index];
            this_mod1 = this_item->modifiers1;
            fetch_index--;
         }
      }
      else {
         if (DFM1_SEQ_REPEAT_N_ALTERNATE & this_mod1) {
            alt_item = &callspec->stuff.def.defarray[fetch_index];
            fetch_index++;
         }
      }

      /* If we are not distributing, perform the range test now, so we don't
         query the user needlessly about parts of calls that we won't do. */

      if (!distribute) {
         if (zzz.reverse_order) {
            if (*test_index+1 > start_point)
               continue;
            if (*test_index+1 < end_point)
               break;
         }
         else {
            if (*test_index-1 < start_point)
               continue;
            if (*test_index-1 > end_point)
               break;
         }
      }

      /* If an explicit substitution was made, we will recompute the ID bits for the setup.
         Normally, we don't, which is why "patch the <anyone>" works.  The original
         evaluation of the designees is retained after the first part of the call.
         But if the user does something like "circle by 1/4 x [leads run]", we
         want to re-evaluate who the leads are. */

      foobar = ss->cmd;
      foobar.cmd_final_flags = new_final_concepts;

      recompute_id |= get_real_subcall(parseptr, this_item, &foobar, callspec, &foo1);

      /* We allow stepping (or rearing back) again. */
      if (this_mod1 & DFM1_PERMIT_TOUCH_OR_REAR_BACK)
         ss->cmd.cmd_misc_flags &= ~CMD_MISC__ALREADY_STEPPED;

      if (this_mod1 & DFM1_SEQ_REPEAT_N_ALTERNATE)
         (void) get_real_subcall(parseptr, alt_item, &foobar, callspec, &foo2);

      /* We also re-evaluate if the invocation flag "seq_re_evaluate" is on. */

      if (recompute_id || (this_mod1 & DFM1_SEQ_RE_EVALUATE)) update_id_bits(result);

      /* If this subcall invocation involves inserting or shifting the numbers, do so. */

      if (!(TEST_HERITBITS(new_final_concepts,INHERITFLAG_FRACTAL)))
         this_mod1 &= ~DFM1_FRACTAL_INSERT;

      process_number_insertion(this_mod1);

      /* Check for special repetition stuff. */
      if ((DFM1_SEQ_REPEAT_N | DFM1_SEQ_REPEAT_NM1 | DFM1_SEQ_REPEAT_N_ALTERNATE) & this_mod1) {
         int count_to_use = current_options.number_fields & 0xF;

         number_used = TRUE;
         if (this_mod1 & DFM1_SEQ_DO_HALF_MORE) count_to_use++;
         if (this_mod1 & DFM1_SEQ_REPEAT_NM1) count_to_use--;
         if (count_to_use < 0) fail("Can't give number zero.");

         if (zzz.do_half_of_last_part != 0 && !distribute && fetch_index == zzz.highlimit) {
            if (count_to_use & 1) fail("Can't fractionalize this call this way.");
            count_to_use >>= 1;
         }

         use_alternate = zzz.reverse_order && !(count_to_use & 1);
         subpart_count = count_to_use;
         if (subpart_count == 0) goto done_with_big_cycle;
         subpart_count--;
      }

      remember_elongation = result->cmd.prior_elongation_bits;

   do_plain_call:

      /* The index point AFTER what we are about to do (0-based numbering, of course),
         so index-1 (or index+1) point to what we are about to do.  Subpart_count has the
         number of ADDITION repetitions of what we are about to do, after we finish the
         upcoming one. */

      if (zzz.reverse_order) {
         if (*test_index+1 > start_point)
            continue;
         if (*test_index+1 < end_point)
            break;
      }
      else {
         if (*test_index-1 < start_point)
            continue;
         if (*test_index-1 > end_point)
            break;
      }

      result->cmd = ss->cmd;
      result->cmd.prior_elongation_bits = remember_elongation;

      /* We don't supply these; they get filled in by the call. */
      result->cmd.cmd_misc_flags &= ~(DFM1_CONCENTRICITY_FLAG_MASK | CMD_MISC__NO_CHECK_MOD_LEVEL);

      if (this_mod1 & DFM1_NO_CHECK_MOD_LEVEL)
         result->cmd.cmd_misc_flags |= CMD_MISC__NO_CHECK_MOD_LEVEL;

      if (zzz.reverse_order) {
         if (zzz.do_half_of_last_part != 0 && *test_index+1 == start_point)
            result->cmd.cmd_frac_flags = zzz.do_half_of_last_part;
         else if (zzz.do_last_half_of_first_part != 0 && *test_index+1 == end_point)
            result->cmd.cmd_frac_flags = zzz.do_last_half_of_first_part;
         else
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
      }
      else {
         if (zzz.do_half_of_last_part != 0 && *test_index == zzz.highlimit)
            result->cmd.cmd_frac_flags = zzz.do_half_of_last_part;
         else if (zzz.do_last_half_of_first_part != 0 && *test_index-1 == start_point)
            result->cmd.cmd_frac_flags = zzz.do_last_half_of_first_part;
         else
            result->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
      }

      if (this_mod1 & DFM1_FINISH_THIS) {
         if (result->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE)
            result->cmd.cmd_frac_flags = 
               CMD_FRAC_CODE_FROMTOREV | CMD_FRAC_PART_BIT*2 | CMD_FRAC_NULL_VALUE;
         else
            fail("Can't fractionalize this call this way.");
      }

      if (!first_call) {    /* Is this right, or should we be using "first_time" here also? */
         /* Stop checking unless we are really serious. */
         if (!setup_is_elongated)
            result->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;

         result->cmd.cmd_misc2_flags &= ~CMD_MISC2__IN_Z_MASK;
      }

      if (!first_time) {
         result->cmd.cmd_assume.assumption = fix_next_assumption;

         if (fix_next_assumption != cr_none) {
            result->cmd.cmd_assume.assump_col = fix_next_assump_col;
            result->cmd.cmd_assume.assump_both = fix_next_assump_both;
            result->cmd.cmd_assume.assump_cast = 0;
            result->cmd.cmd_assume.assump_live = 0;
            result->cmd.cmd_assume.assump_negate = 0;

            /* If we just put in an "assume 1/4 tag" type of thing, we presumably
               did a "scoot back to a wave" as part of a "scoot reaction".  Now, if
               there were phantoms in the center after the call, the result could
               have gotten changed (by the normalization stuff deep within
               "fix_n_results" or whatever) to a 2x4.  However, if we are doing a
               scoot reaction, we really want the 1/4 tag.  So change it back.
               It happens that code in "divide_the_setup" would do this anyway,
               but we don't like assumptions in place on setups for which they
               are meaningless. */

            if (fix_next_assump_both == 2 &&
                (fix_next_assumption == cr_jleft || fix_next_assumption == cr_jright) &&
                result->kind == s2x4 &&
                (result->people[1].id1 | result->people[2].id1 |
                 result->people[5].id1 | result->people[6].id1) == 0) {
               expand_setup(&comp_qtag_2x4_stuff, result);
            }
         }
      }

      fix_next_assumption = cr_none;
      fix_next_assump_col = 0;
      fix_next_assump_both = 0;

      if ((DFM1_SEQ_REPEAT_N_ALTERNATE & this_mod1) && use_alternate) {
         result->cmd.parseptr = foo2.parseptr;
         result->cmd.callspec = foo2.callspec;
         result->cmd.cmd_final_flags = foo2.cmd_final_flags;

      }
      else {
         result->cmd.parseptr = foo1.parseptr;
         result->cmd.callspec = foo1.callspec;
         result->cmd.cmd_final_flags = foo1.cmd_final_flags;
      }

      oldk = result->kind;

      if (oldk == s2x2 && (result->cmd.prior_elongation_bits & 3) != 0)
         remembered_2x2_elongation = result->cmd.prior_elongation_bits & 3;

      /* We need to manipulate some assumptions -- there are a few cases in
         dancers really do track an awareness of the formation. */

      if (!(TEST_HERITBITS(result->cmd.cmd_final_flags,(INHERITFLAG_HALF | INHERITFLAG_LASTHALF))) &&
          result->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE) {

         if (result->cmd.callspec == base_calls[base_call_chreact_1]) {

            /* If we are starting a chain reaction, and the assumption was some form
               of 1/4 tag or 1/4 line (all of the above indicate such a thing --
               the outsides are a couple looking in), then, whether it was a plain
               chain reaction or a cross chain reaction, the result will have the
               checkpointers in miniwaves.  Pass that assumption on, so that they can hinge. */

            if (     ((    result->cmd.cmd_assume.assumption == cr_jleft ||
                           result->cmd.cmd_assume.assumption == cr_ijleft ||
                           result->cmd.cmd_assume.assumption == cr_jright ||
                           result->cmd.cmd_assume.assumption == cr_ijright) &&
                      result->cmd.cmd_assume.assump_both == 2)
                     ||
                     (result->cmd.cmd_assume.assumption == cr_qtag_like &&
                      result->cmd.cmd_assume.assump_both == 1)
                     ||
                     result->cmd.cmd_assume.assumption == cr_real_1_4_tag
                     ||
                     result->cmd.cmd_assume.assumption == cr_real_1_4_line)
               fix_next_assumption = cr_ckpt_miniwaves;
         }
         else if (result->cmd.callspec == base_calls[base_call_scoottowave]) {
            if (result->kind == s2x4 &&
                !(result->cmd.cmd_final_flags.her8it & INHERITFLAG_YOYO)) {
               if ((result->people[0].id1 & d_mask) == d_north ||
                   (result->people[1].id1 & d_mask) == d_south ||
                   (result->people[2].id1 & d_mask) == d_north ||
                   (result->people[3].id1 & d_mask) == d_south ||
                   (result->people[4].id1 & d_mask) == d_south ||
                   (result->people[5].id1 & d_mask) == d_north ||
                   (result->people[6].id1 & d_mask) == d_south ||
                   (result->people[7].id1 & d_mask) == d_north) {
                  fix_next_assumption = cr_jleft;               
                  fix_next_assump_both = 2;
               }
               else if ((result->people[0].id1 & d_mask) == d_south ||
                        (result->people[1].id1 & d_mask) == d_north ||
                        (result->people[2].id1 & d_mask) == d_south ||
                        (result->people[3].id1 & d_mask) == d_north ||
                        (result->people[4].id1 & d_mask) == d_north ||
                        (result->people[5].id1 & d_mask) == d_south ||
                        (result->people[6].id1 & d_mask) == d_north ||
                        (result->people[7].id1 & d_mask) == d_south) {
                  fix_next_assumption = cr_jright;               
                  fix_next_assump_both = 2;
               }
            }
         }
         else if (result->cmd.callspec == base_calls[base_call_makepass_1]) {

            /* If we are starting a "make a pass", and the assumption was some form
               of 1/4 tag, then we will have a 2-faced line in the center.  Pass that
               assumption on, so that they can cast off 3/4.  If it was a 1/4 line,
               the result will be a wave in the center. */

            if (((result->cmd.cmd_assume.assumption == cr_jleft ||
                  result->cmd.cmd_assume.assumption == cr_jright) &&
                 result->cmd.cmd_assume.assump_both == 2) ||
                result->cmd.cmd_assume.assumption == cr_real_1_4_tag)
               fix_next_assumption = cr_ctr_couples;
            else if (((result->cmd.cmd_assume.assumption == cr_ijleft ||
                       result->cmd.cmd_assume.assumption == cr_ijright) &&
                      result->cmd.cmd_assume.assump_both == 2) ||
                     result->cmd.cmd_assume.assumption == cr_real_1_4_line)
               fix_next_assumption = cr_ctr_miniwaves;
            else if (result->cmd.cmd_assume.assumption == cr_qtag_like &&
                     result->cmd.cmd_assume.assump_both == 1 &&
                     oldk == s_qtag &&
                     (result->people[2].id1 & d_mask & ~2) == d_north &&
                     ((result->people[2].id1 ^ result->people[6].id1) & d_mask) == 2 &&
                     ((result->people[3].id1 ^ result->people[7].id1) & d_mask) == 2) {
               if (((result->people[2].id1 ^ result->people[3].id1) & d_mask) == 0)
                  fix_next_assumption = cr_ctr_miniwaves;
               else if (((result->people[2].id1 ^ result->people[3].id1) & d_mask) == 2)
                  fix_next_assumption = cr_ctr_couples;
            }
         }
         else if (result->cmd.callspec == base_calls[base_call_circulate]) {

            /* If we are doing a circulate in columns, and the assumption was
               "8 chain" or "trade by", change it to the other assumption.
               Similarly for facing lines and back-to-back lines. */

            if (result->cmd.cmd_assume.assumption == cr_li_lo &&
                (result->cmd.cmd_assume.assump_col & (~1)) == 0 &&
                ((result->cmd.cmd_assume.assump_both - 1) & (~1)) == 0) {
               fix_next_assumption = cr_li_lo;
               fix_next_assump_col = result->cmd.cmd_assume.assump_col;
               fix_next_assump_both = result->cmd.cmd_assume.assump_both ^ 3;
            }
         }
         else if (result->cmd.callspec == base_calls[base_call_slither] &&
                  result->cmd.cmd_assume.assump_col == 0 &&
                  result->cmd.cmd_assume.assump_both == 0) {
            switch (result->cmd.cmd_assume.assumption) {
            case cr_2fl_only:
               fix_next_assumption = cr_wave_only;
               break;
            case cr_wave_only:
               fix_next_assumption = cr_2fl_only;
               break;
            case cr_miniwaves:
               fix_next_assumption = cr_couples_only;
               break;
            case cr_couples_only:
               fix_next_assumption = cr_miniwaves;
               break;
            }
         }
         else if (result->cmd.callspec == base_calls[base_call_lockit] &&
                  result->cmd.cmd_assume.assump_col == 0 &&
                  result->cmd.cmd_assume.assump_both == 0) {
            switch (result->cmd.cmd_assume.assumption) {
            case cr_2fl_only:
            case cr_wave_only:
               fix_next_assumption = result->cmd.cmd_assume.assumption;
               break;
            }
         }
         else if (result->cmd.callspec == base_calls[base_call_disband1] &&
                  result->kind == s2x4 &&
                  result->cmd.cmd_assume.assump_col == 1 &&
                  result->cmd.cmd_assume.assump_both == 0) {
            switch (result->cmd.cmd_assume.assumption) {
            case cr_wave_only:
               fix_next_assumption = cr_magic_only;
               fix_next_assump_col = 1;
               break;
            case cr_magic_only:
               fix_next_assumption = cr_wave_only;
               fix_next_assump_col = 1;
               break;
            }
         }
         else if (result->cmd.callspec == base_calls[base_call_check_cross_counter]) {
            /* Just pass everything directly -- this call does nothing. */
            fix_next_assumption = result->cmd.cmd_assume.assumption;
            fix_next_assump_col = result->cmd.cmd_assume.assump_col;
            fix_next_assump_both = result->cmd.cmd_assume.assump_both;
         }
      }

      if (DFM1_CPLS_UNLESS_SINGLE & this_mod1) {
         result->cmd.cmd_misc_flags |= CMD_MISC__DO_AS_COUPLES;
         result->cmd.do_couples_her8itflags = new_final_concepts.her8it;
      }

      if (TEST_HERITBITS(result->cmd.cmd_final_flags,INHERITFLAG_TWISTED)) {
         if (result->cmd.prior_expire_bits & RESULTFLAG__TWISTED_FINISHED)
            result->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_TWISTED;   /* Already did that. */
         resultflags_to_put_in |= RESULTFLAG__TWISTED_FINISHED;
      }

      if (TEST_HERITBITS(result->cmd.cmd_final_flags,INHERITFLAG_YOYO)) {
         if (result->cmd.prior_expire_bits & RESULTFLAG__YOYO_FINISHED)
            result->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_YOYO;   /* Already did that. */
         resultflags_to_put_in |= RESULTFLAG__YOYO_FINISHED;
      }

      if (result->cmd.cmd_final_flags.final & FINAL__SPLIT_SQUARE_APPROVED) {
         if (result->cmd.prior_expire_bits & RESULTFLAG__SPLIT_FINISHED)
            result->cmd.cmd_final_flags.final &= ~FINAL__SPLIT_SQUARE_APPROVED;
         resultflags_to_put_in |= RESULTFLAG__SPLIT_FINISHED;
      }

      do_call_in_series(
         result,
         zzz.reverse_order,
         DFM1_ROLL_TRANSPARENT & this_mod1,
         (!(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX) &&
          !(TEST_HERITBITS(new_final_concepts,(INHERITFLAG_12_MATRIX|INHERITFLAG_16_MATRIX))) &&
          (recompute_id | (this_mod1 & DFM1_SEQ_NORMALIZE))),
         qtfudged);

      result->result_flags |= resultflags_to_put_in;

      if (oldk != s2x2 && result->kind == s2x2 && remembered_2x2_elongation != 0) {
         result->result_flags = (result->result_flags & ~3) | remembered_2x2_elongation;
         result->cmd.prior_elongation_bits = (result->cmd.prior_elongation_bits & ~3) | remembered_2x2_elongation;
      }

      remember_elongation = result->cmd.prior_elongation_bits;

      if (subpart_count && !distribute) continue;

done_with_big_cycle:

      /* We allow expansion on the first part, and then shut it off for later parts.
         This is required for things like 12 matrix grand swing thru from a 1x8 or 1x10. */
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

      /* This really isn't right.  It is done in order to make "ENDS FINISH set back" work.
         The flaw is that, if the sequentially defined call "FINISH set back" had more than one
         part, we would be OR'ing the bits from multiple parts.  What would it mean?  The bits
         we are interested in are the "demand_lines" and "force_lines" stuff.  I guess we should
         take the "demand" bits from the first part and the "force" bits from the last part.  Yuck! */

      /* The claim is that the following code removes the above problem.  The test is "ends 2/3 chisel thru".
         Below, we will pick up the concentricity flags from the last subcall. */

      ss->cmd.cmd_misc_flags |= result->cmd.cmd_misc_flags & ~DFM1_CONCENTRICITY_FLAG_MASK;

      if (DFM1_SEQ_REENABLE_ELONG_CHK & this_mod1)
         ss->cmd.cmd_misc_flags &= ~CMD_MISC__NO_CHK_ELONG;

      current_options.number_fields = saved_number_fields;
      current_options.howmanynumbers = saved_num_numbers;

      qtfudged = FALSE;

      new_final_concepts.final &=
         ~(FINAL__SPLIT | FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED);

      first_call = FALSE;
      first_time = FALSE;

      /* If we are being asked to do just one part of a call (from cmd_frac_flags),
         exit now.  Also, see if we just did the last part. */

      if (zzz.instant_stop != 99) {
         /* Check whether we honored the last possible request.  That is,
            whether we did the last part of the call in forward order, or
            the first part in reverse order. */
         result->result_flags |= RESULTFLAG__PARTS_ARE_KNOWN;
         if (zzz.instant_stop >= zzz.highlimit)
            result->result_flags |= RESULTFLAG__DID_LAST_PART;
         break;
      }
   }

   // If the setup expanded from an 8-person setup to a "bigdmd", and we can
   // compress it back, do so.  This takes care of certain type of "triple diamonds
   // working together exchange the diamonds 1/2" situations.

   if (result->kind == sbigdmd && setup_attrs[ss->kind].setup_limits == 7)
      normalize_setup(result, normalize_compress_bigdmd);

   // Pick up the concentricity command stuff from the last thing we did,
   // but take out the effect of "splitseq".

   ss->cmd.cmd_misc_flags |= result->cmd.cmd_misc_flags;
   ss->cmd.cmd_misc_flags &= ~CMD_MISC__MUST_SPLIT_MASK;
}


// Check for a schema that we weren't sure about, and fix it up, using the specified modifiers.
Private calldef_schema fixup_conc_schema(Const callspec_block *callspec, setup *ss)
{
   calldef_schema the_schema = callspec->schema;
   uint32 herit_concepts = ss->cmd.cmd_final_flags.her8it;

   switch (the_schema) {
   case schema_maybe_single_concentric:
      return (herit_concepts & INHERITFLAG_SINGLE) ?
         schema_single_concentric : schema_concentric;
   case schema_maybe_single_cross_concentric:
      return (herit_concepts & INHERITFLAG_SINGLE) ?
         schema_single_cross_concentric : schema_cross_concentric;
   case schema_maybe_grand_single_concentric:
      if (herit_concepts & INHERITFLAG_GRAND) {
         if (herit_concepts & INHERITFLAG_SINGLE)
            return schema_grand_single_concentric;
         else
            fail("You must not use \"grand\" without \"single\".");
      }
      else {
         return (herit_concepts & INHERITFLAG_SINGLE) ?
            schema_single_concentric : schema_concentric;
      }
   case schema_maybe_grand_single_cross_concentric:
      if (herit_concepts & INHERITFLAG_GRAND) {
         if (herit_concepts & INHERITFLAG_SINGLE)
            return schema_grand_single_cross_concentric;
         else
            fail("You must not use \"grand\" without \"single\".");
      }
      else {
         if (herit_concepts & INHERITFLAG_SINGLE)
            return schema_single_cross_concentric;
         else
            return schema_cross_concentric;
      }
   case schema_maybe_special_single_concentric:
      /* "Single" has the usual meaning for this one.  But "grand single"
         turns it into a "special concentric", which has the centers working
         in three pairs. */

      if (herit_concepts & INHERITFLAG_SINGLE) {
         if (herit_concepts & (INHERITFLAG_GRAND | INHERITFLAG_NXNMASK))
            return schema_concentric_others;
         else
            return schema_single_concentric;
      }
      else {
         if ((herit_concepts & (INHERITFLAG_GRAND | INHERITFLAG_NXNMASK)) == INHERITFLAG_GRAND)
            fail("You must not use \"grand\" without \"single\" or \"nxn\".");
         else if ((herit_concepts & (INHERITFLAG_GRAND | INHERITFLAG_NXNMASK)) == 0)
            return schema_concentric;
         else if ((herit_concepts &
                   (INHERITFLAG_NXNMASK | INHERITFLAG_12_MATRIX | INHERITFLAG_16_MATRIX)) ==
                  (INHERITFLAGNXNK_4X4 | INHERITFLAG_16_MATRIX))
            return schema_4x4_lines_concentric;
         else if ((herit_concepts &
                   (INHERITFLAG_NXNMASK | INHERITFLAG_12_MATRIX | INHERITFLAG_16_MATRIX)) ==
                  (INHERITFLAGNXNK_3X3 | INHERITFLAG_12_MATRIX))
            return schema_3x3_concentric;
      }
   case schema_maybe_nxn_lines_concentric:
      switch (herit_concepts &
              (INHERITFLAG_SINGLE | INHERITFLAG_NXNMASK | INHERITFLAG_MXNMASK)) {
      case INHERITFLAG_SINGLE:
         return schema_single_concentric;
      case INHERITFLAGNXNK_3X3:
         return schema_3x3_concentric;
      case INHERITFLAGNXNK_4X4:
         return schema_4x4_lines_concentric;
      case 0:
         return schema_concentric;
      }
   case schema_maybe_nxn_cols_concentric:
      switch (herit_concepts &
              (INHERITFLAG_SINGLE | INHERITFLAG_NXNMASK | INHERITFLAG_MXNMASK)) {
      case INHERITFLAG_SINGLE:
         return schema_single_concentric;
      case INHERITFLAGNXNK_3X3:
         return schema_3x3_concentric;
      case INHERITFLAGNXNK_4X4:
         return schema_4x4_cols_concentric;
      case 0:
         return schema_concentric;
      }
   case schema_maybe_nxn_1331_lines_concentric:
      switch (herit_concepts &
              (INHERITFLAG_SINGLE | INHERITFLAG_NXNMASK | INHERITFLAG_MXNMASK)) {
      case INHERITFLAG_SINGLE:
         return schema_single_concentric;
      case INHERITFLAGNXNK_3X3:
         return schema_3x3_concentric;
      case INHERITFLAGNXNK_4X4:
         return schema_4x4_lines_concentric;
      case INHERITFLAGMXNK_1X3:
      case INHERITFLAGMXNK_3X1:
         return schema_1331_concentric;
      case 0:
         return schema_concentric;
      }
   case schema_maybe_nxn_1331_cols_concentric:
      switch (herit_concepts &
              (INHERITFLAG_SINGLE | INHERITFLAG_NXNMASK | INHERITFLAG_MXNMASK)) {
      case INHERITFLAG_SINGLE:
         return schema_single_concentric;
      case INHERITFLAGNXNK_3X3:
         return schema_3x3_concentric;
      case INHERITFLAGNXNK_4X4:
         return schema_4x4_cols_concentric;
      case INHERITFLAGMXNK_1X3:
      case INHERITFLAGMXNK_3X1:
         return schema_1331_concentric;
      case INHERITFLAGMXNK_1X2:
      case INHERITFLAGMXNK_2X1:
         return schema_concentric_6p_or_normal;
      case 0:
         return schema_concentric;
      }
   case schema_maybe_matrix_single_concentric_together:
      if (herit_concepts & INHERITFLAG_12_MATRIX)
         return schema_conc_12;
      else if (herit_concepts & INHERITFLAG_16_MATRIX)
         return schema_conc_16;
      else if (herit_concepts & INHERITFLAG_GRAND)
         return schema_concentric_others;
      else
         return schema_single_concentric_together;
   case schema_maybe_matrix_conc:
      if (herit_concepts & INHERITFLAG_12_MATRIX)
         return schema_conc_12;
      else if (herit_concepts & INHERITFLAG_16_MATRIX)
         return schema_conc_16;
      else
         return schema_concentric;
   case schema_maybe_matrix_conc_star:
      if (herit_concepts & INHERITFLAG_12_MATRIX)
         return schema_conc_star12;
      else if (herit_concepts & INHERITFLAG_16_MATRIX)
         return schema_conc_star16;
      else
         return schema_conc_star;
   case schema_maybe_matrix_conc_bar:
      if (herit_concepts & INHERITFLAG_12_MATRIX)
         return schema_conc_bar12;
      else if (herit_concepts & INHERITFLAG_16_MATRIX)
         return schema_conc_bar16;
      else
         return schema_conc_bar;
   default:
      return the_schema;
   }

   fail("Can't use this combination of modifiers.");
}


/* This leaves the split axis result bits in absolute orientation. */

Private void move_with_real_call(
   setup *ss,
   long_boolean qtfudged,
   setup *result)
{
   uint32 tbonetest;
   uint32 imprecise_rotation_result_flag = 0;
   uint32 unaccepted_flags;
   calldef_schema the_schema;
   setup_command foo1;
   setup_command foo2;
   long_boolean mirror;
   long_boolean did_4x4_expansion = FALSE;
   uint32 force_split = 0;      /* 1 means force split,
                                   2 means this is 1x8 and do not recompute id. */
   selector_kind special_selector = selector_none;
   uint32 special_modifiers = 0;
   selective_key special_indicator = selective_key_plain;
   callspec_block *callspec = ss->cmd.callspec;
   uint32 callflags1 = callspec->callflags1;

   clear_people(result);
   result->result_flags = 0;   /* In case we bail out. */

   /* We have a genuine call.  Presumably all serious concepts have been disposed of
      (that is, nothing interesting will be found in parseptr -- it might be
      useful to check that someday) and we just have the callspec and the final
      concepts. */

   if (ss->cmd.cmd_misc_flags & CMD_MISC__RESTRAIN_MODIFIERS) {
      ss->cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_MODIFIERS;
      ss->cmd.cmd_final_flags.her8it |= ss->cmd.restrained_super8flags;
   }

   if (ss->kind == nothing) {
      if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
         fail("Can't fractionalize a call if no one is doing it.");

      result->kind = nothing;
      return;
   }

   if ((callspec->callflagsf & CFLAG2_YOYO_FRACTAL_NUM)) {
      if ((TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_FRACTAL))) {
         if ((current_options.number_fields & 0xD) == 1)
            current_options.number_fields ^= 2;
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_FRACTAL;
      }
      else if ((TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_YOYO))) {
         if ((current_options.number_fields & 0xF) == 2)
            current_options.number_fields++;
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_YOYO;
      }
   }


/* ***** Beware!!!!  There are still some checks for
   if (ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT)
that probably need to be put in. */

   the_schema = fixup_conc_schema(callspec, ss);

   /* Check for "central" concept and its ilk, and pick up correct definition. */

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX | CMD_MISC__DISTORTED;

      /* If we invert centers and ends parts, we don't raise errors
         for bad elongation if "suppress_elongation_warnings" was set for the centers part.
         This allows horrible "ends trade" on "invert acey deucey", for example,
         since "acey deucey" has that flag set for the trade that the centers do. */

      if ((ss->cmd.cmd_misc2_flags & CMD_MISC2__SAID_INVERT) &&
          (the_schema == schema_concentric ||
           the_schema == schema_concentric_6p ||
           the_schema == schema_concentric_6p_or_normal ||
           the_schema == schema_conc_o) &&
          (DFM1_SUPPRESS_ELONGATION_WARNINGS & callspec->stuff.conc.innerdef.modifiers1))
         ss->cmd.cmd_misc_flags |= CMD_MISC__NO_CHK_ELONG;

      /* We shut off the "doing ends" stuff.  If we say "ends detour" we mean "ends do the
         ends part of detour".  But if we say "ends central detour" we mean
         "ends do the *centers* part of detour". */
      ss->cmd.cmd_misc_flags &= ~CMD_MISC__DOING_ENDS;

      /* Now we demand that, if a concept was given, the call had the appropriate flag
         set saying that the concept is legal and will be inherited to the children.
         Unless it is defined by array. */

      if (the_schema != schema_by_array &&
          (ss->cmd.cmd_final_flags.her8it &
           (~(callspec->callflagsh|INHERITFLAG_HALF|INHERITFLAG_LASTHALF))))
         fail("Can't do this call with this concept.");

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__DO_CENTRAL) {
         /* If it is sequential, we just pass it through.  Otherwise, we handle it here. */
         if (the_schema != schema_sequential &&
             the_schema != schema_sequential_with_fraction &&
             the_schema != schema_sequential_with_split_1x8_id) {
            Const by_def_item *defptr;
            uint32 inv_bits;

            switch (the_schema) {
            case schema_concentric:
            case schema_single_concentric:
            case schema_single_concentric_together:
            case schema_conc_o:
            case schema_concentric_6p:
            case schema_concentric_6p_or_normal:
            case schema_concentric_6p_or_sgltogether:
            case schema_cross_concentric_6p_or_normal:
               /* Normally, we get the centers' part of the definition.  But if the user said
                  either "invert central" (the concept that means to get the ends' part)
                  or "central invert" (the concept that says to get the centers' part
                  of the inverted call) we get the ends' part.  If BOTH inversion bits are on,
                  the user said "invert central invert", meaning to get the ends' part of the
                  inverted call, so we just get the centers' part as usual. */

               inv_bits = ss->cmd.cmd_misc2_flags &
                  (CMD_MISC2__INVERT_CENTRAL | CMD_MISC2__SAID_INVERT);

               if (inv_bits == CMD_MISC2__INVERT_CENTRAL ||
                   inv_bits == CMD_MISC2__SAID_INVERT)
                  defptr = &callspec->stuff.conc.outerdef;
               else
                  defptr = &callspec->stuff.conc.innerdef;

               if (ss->cmd.cmd_final_flags.final &
                   ~(FINAL__SPLIT | FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED))
                  fail("This concept not allowed here.");

               do_inheritance(&ss->cmd, callspec, defptr);
               callspec = ss->cmd.callspec;
               callflags1 = callspec->callflags1;
               the_schema = fixup_conc_schema(callspec, ss);

               process_number_insertion(defptr->modifiers1);

               switch (defptr->modifiers1 & DFM1_CALL_MOD_MASK) {
               case DFM1_CALL_MOD_BIT*2:
               case DFM1_CALL_MOD_BIT*6:
                  fail("You can't select that part of the call.");
               }

               if (callspec == base_calls[base_call_null_second])
                  fail("You can't select that part of the call.");

               break;
            case schema_select_ctr2:
            case schema_select_ctr4:
               /* Just leave the definition in place.  We will split the 8-person
                  setup into two 4-person setups, and then pick out the center 2 from them. */
               force_split = 1;
               break;
            default:
               fail("Can't do \"central\" with this call.");
            }

            /* We are done. */
            ss->cmd.cmd_misc2_flags &=
               ~(CMD_MISC2__DO_CENTRAL | CMD_MISC2__INVERT_CENTRAL | CMD_MISC2__SAID_INVERT);
         }
      }
      else {
         /* Snag or mystic, or maybe just plain invert, was on. */
         if (ss->cmd.cmd_final_flags.final) fail("This concept not allowed here.");
      }
   }

   if (callflags1 & CFLAG1_IMPRECISE_ROTATION)
      imprecise_rotation_result_flag = RESULTFLAG__IMPRECISE_ROT;

   /* We of course don't allow "mystic" or "snag" for things that are
      *CROSS* concentrically defined.  That will be taken care of later, in concentric_move. */

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK) {
      switch (the_schema) {
         case schema_sequential:
         case schema_sequential_with_fraction:
         case schema_sequential_with_split_1x8_id:
         case schema_concentric:
         case schema_concentric_2_6:
         case schema_conc_o:
         case schema_select_ctr2:
         case schema_select_ctr4:
         case schema_select_ctr6:
         case schema_single_concentric:
         case schema_single_concentric_together:
         case schema_select_original_rims:
         case schema_select_original_hubs:
         case schema_cross_concentric:
         case schema_single_cross_concentric:
         case schema_concentric_or_diamond_line:
         case schema_concentric_6p:
         case schema_concentric_6p_or_sgltogether:
         case schema_concentric_6p_or_normal:
         case schema_cross_concentric_6p_or_normal:
         case schema_by_array:
            break;
         default:
            fail("Can't do \"central/snag/mystic\" with this call.");
      }
   }

   /* Do some quick error checking for visible fractions.  For now, any flag is acceptable.  Later, we will
      distinguish among the various flags. */

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE) {
      switch (the_schema) {
         case schema_by_array:
            /* We allow the fractions "1/2" and "last 1/2" to be given.
               Basic_move will handle them. */
            if (ss->cmd.cmd_frac_flags == CMD_FRAC_HALF_VALUE) {
               ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
               ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_HALF;
            }
            else if (ss->cmd.cmd_frac_flags == CMD_FRAC_LASTHALF_VALUE) {
               ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
               ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_LASTHALF;
            }
            else
               fail("This call can't be fractionalized this way.");

            break;
         case schema_nothing: case schema_matrix: case schema_partner_matrix: case schema_roll:
            fail("This call can't be fractionalized.");
            break;
         case schema_sequential:
         case schema_sequential_with_fraction:
         case schema_split_sequential:
         case schema_sequential_with_split_1x8_id:
            if (!(callflags1 & CFLAG1_VISIBLE_FRACTION_MASK) && !(ss->cmd.cmd_frac_flags & CMD_FRAC_FORCE_VIS))
               fail("This call can't be fractionalized.");
            break;
         default:

            /* Must be some form of concentric.  We allow visible fractions, and take no action in that case.
               This means that any fractions will be sent to constituent calls. */

            if (!(callflags1 & CFLAG1_VISIBLE_FRACTION_MASK)) {

               /* Otherwise, we allow the fraction "1/2" to be given, if the top-level heritablilty
                  flag allows it.  We turn the fraction into a "final concept". */

               if (ss->cmd.cmd_frac_flags == CMD_FRAC_HALF_VALUE) {
                  ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
                  ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_HALF;
               }
               else if (ss->cmd.cmd_frac_flags == CMD_FRAC_LASTHALF_VALUE) {
                  ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
                  ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_LASTHALF;
               }
               else {
                  fail("This call can't be fractionalized this way.");
               }
            }

            break;
      }
   }

   /* If the "diamond" concept has been given and the call doesn't want it, we do
      the "diamond single wheel" variety. */

   if (INHERITFLAG_DIAMOND & ss->cmd.cmd_final_flags.her8it & (~callspec->callflagsh))  {
      /* If the call is sequentially or concentrically defined, the top level flag is required
         before the diamond concept can be inherited.  Since that flag is off, it is an error. */
      if (the_schema != schema_by_array)
         fail("Can't do this call with the \"diamond\" concept.");

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"invert/central/snag/mystic\" with the \"diamond\" concept.");

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

      if (ss->kind == sdmd) {
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_DIAMOND;
         new_divided_setup_move(ss, MAPCODE(s1x2,2,MPKIND__DMD_STUFF,0), phantest_ok, TRUE, result);
         return;
      }
      else {
         /* If in a qtag or point-to-points, perhaps we ought to divide
            into single diamonds and try again.   BUT: if "magic" or "interlocked"
            is also present, we don't.  We let basic_move deal with
            it.  It will come back here after it has done what it needs to. */

         if (!(TEST_HERITBITS(ss->cmd.cmd_final_flags,(INHERITFLAG_MAGIC|INHERITFLAG_INTLK)))) {
            /* Divide into diamonds and try again.  Note that we do not clear the concept. */
            divide_diamonds(ss, result);
            return;
         }
      }
   }

   mirror = FALSE;

   /* It may be appropriate to step to a wave or rear back from one.
      This is only legal if the flag forbidding same is off.
      Furthermore, if certain modifiers have been given, we don't allow it. */

   if (TEST_HERITBITS(ss->cmd.cmd_final_flags,(INHERITFLAG_MAGIC | INHERITFLAG_INTLK | INHERITFLAG_12_MATRIX | INHERITFLAG_16_MATRIX | INHERITFLAG_FUNNY)))
      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_STEP_TO_WAVE;

   /* But, alas, if fractionalization is on, we can't do it yet, because we don't
      know whether we are starting at the beginning.  In the case of fractionalization,
      we will do it later.  We also can't do it yet if we are going
      to split the setup for "central" or "crazy", or if we are doing the call "mystic". */

   if ((!(ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_MYSTIC) ||
        the_schema != schema_by_array) &&
       (callflags1 & (CFLAG1_STEP_REAR_MASK | CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK))) {
      if (ss->cmd.cmd_frac_flags == CMD_FRAC_NULL_VALUE ||
          (ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
          (CMD_FRAC_NULL_VALUE | CMD_FRAC_CODE_FROMTO)) {

         if (!(ss->cmd.cmd_misc_flags & (CMD_MISC__NO_STEP_TO_WAVE |
                                         CMD_MISC__ALREADY_STEPPED |
                                         CMD_MISC__MUST_SPLIT_MASK))) {
            if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_LEFT)) {
               mirror_this(ss);
               mirror = TRUE;
            }
      
            ss->cmd.cmd_misc_flags |= CMD_MISC__ALREADY_STEPPED;  /* Can only do it once. */
            touch_or_rear_back(ss, mirror, callflags1);
      
            /* But, if the "left_means_touch_or_check" flag is set, we only wanted the "left"
               flag for the purpose of what "touch_or_rear_back" just did.  So, in that case,
               we turn off the "left" flag and set things back to normal. */
      
            if (callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) {
               if (mirror) mirror_this(ss);
               mirror = FALSE;
            }
         }
   
         /* Actually, turning off the "left" flag is more global than that. */
   
         if (callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) {
            ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_LEFT;
         }
      }
      else if ((ss->cmd.cmd_frac_flags & ~CMD_FRAC_PART_MASK) ==
               (CMD_FRAC_NULL_VALUE | CMD_FRAC_CODE_FROMTOREV) &&
               (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) >= (CMD_FRAC_PART_BIT*2)) {
         /* If we're doing the rest of the call, just turn all that stuff off. */
         if (callflags1 & CFLAG1_LEFT_MEANS_TOUCH_OR_CHECK) {
            ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_LEFT;
         }
      }
   }

   /* Check for a call whose schema is single (cross) concentric.
      If so, be sure the setup is divided into 1x4's or diamonds.
      But don't do it if something like "magic" is still unprocessed. */

   if ((ss->cmd.cmd_final_flags.her8it &
        (~(callspec->callflagsh|INHERITFLAG_HALF|INHERITFLAG_LASTHALF))) == 0) {
      switch (the_schema) {
      case schema_single_concentric:
      case schema_single_cross_concentric:
         force_split = 1;
         break;
      case schema_single_concentric_together:
      case schema_concentric_6p_or_sgltogether:
         switch (ss->kind) {
         case s1x8: case s_ptpd:
            force_split = 1;
            break;
         }
         break;
      case schema_select_original_rims:
      case schema_select_original_hubs:
         switch (ss->kind) {
         case s1x8: case s_ptpd:
            force_split = 2;     /* What a crock!!!!! ****** */
            break;
         }
         break;
      }
   }

   if (force_split)
      if (!do_simple_split(ss, force_split, result)) return;

   /* At this point, we may have mirrored the setup and, of course, left the switch "mirror"
      on.  We did it only as needed for the [touch / rear back / check] stuff.  What we
      did doesn't actually count.  In particular, if the call is defined concentrically
      or sequentially, mirroring the setup in response to "left" is *NOT* the right thing
      to do.  The right thing is to pass the "left" flag to all subparts that have the
      "inherit_left" invocation flag, and letting events take their course.  So we allow
      the "INHERITFLAG_LEFT" bit to remain in "cmd_final_flags", because it is still important
      to know whether we have been invoked with the "left" modifier. */

   /* Check for special case of ends doing a call like "detour" which specifically
      allows just the ends part to be done.  If the call was "central", this flag will be turned off. */

   if (ss->cmd.cmd_misc_flags & CMD_MISC__DOING_ENDS) {
      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"invert/central/snag/mystic\" with a call for the ends only.");

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;
      if ((the_schema == schema_concentric ||
           the_schema == schema_rev_checkpoint ||
           the_schema == schema_concentric_6p ||
           the_schema == schema_concentric_6p_or_normal ||
           the_schema == schema_conc_o) &&
            (DFM1_ENDSCANDO & callspec->stuff.conc.outerdef.modifiers1)) {

         /* Copy the concentricity flags from the call definition into the setup.  All the fuss
            in database.h about concentricity flags co-existing with setupflags refers to this moment. */
         ss->cmd.cmd_misc_flags |= (callspec->stuff.conc.outerdef.modifiers1 & DFM1_CONCENTRICITY_FLAG_MASK);

         if (the_schema == schema_conc_o) {
            did_4x4_expansion = TRUE;
            if (ss->kind != s2x2)
               fail("Can't find outside 'O' spots.");
            if (ss->cmd.prior_elongation_bits == 1) {
               setup temp = *ss;
               ss->kind = s4x4;
               clear_people(ss);
               (void) copy_person(ss, 10, &temp, 0);
               (void) copy_person(ss, 1, &temp, 1);
               (void) copy_person(ss, 2, &temp, 2);
               (void) copy_person(ss, 9, &temp, 3);
            }
            else if (ss->cmd.prior_elongation_bits == 2) {
               setup temp = *ss;
               ss->kind = s4x4;
               clear_people(ss);
               (void) copy_person(ss, 13, &temp, 0);
               (void) copy_person(ss, 14, &temp, 1);
               (void) copy_person(ss, 5, &temp, 2);
               (void) copy_person(ss, 6, &temp, 3);
            }
            else
               fail("Can't find outside 'O' spots.");
         }

         do_inheritance(&ss->cmd, callspec, &callspec->stuff.conc.outerdef);
         callspec = ss->cmd.callspec;
         callflags1 = callspec->callflags1;
         the_schema = fixup_conc_schema(callspec, ss);
      }
   }

   /* ******** We did this before, but maybe that was too early!!!!  Need to do it again
         after pulling out the "doing ends" stuff. */


   /* Do some quick error checking for visible fractions.  For now, any flag is acceptable.  Later, we will
      distinguish among the various flags. */

   if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE) {
      switch (the_schema) {
         case schema_by_array:
            /* We allow the fractions "1/2" and "last 1/2" to be given.
               Basic_move will handle them. */
            if (ss->cmd.cmd_frac_flags == CMD_FRAC_HALF_VALUE) {
               ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
               ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_HALF;
            }
            else if (ss->cmd.cmd_frac_flags == CMD_FRAC_LASTHALF_VALUE) {
               ss->cmd.cmd_frac_flags = CMD_FRAC_NULL_VALUE;
               ss->cmd.cmd_final_flags.her8it |= INHERITFLAG_LASTHALF;
            }
            else
               fail("This call can't be fractionalized this way.");

            break;
      }
   }

   /* ******** */



   /* Enforce the restriction that only tagging calls are allowed in certain contexts. */

   if (ss->cmd.cmd_final_flags.final & FINAL__MUST_BE_TAG) {
      if (!(callflags1 & CFLAG1_BASE_TAG_CALL_MASK))
         fail("Only a tagging call is allowed here.");
   }

   ss->cmd.cmd_final_flags.final &= ~FINAL__MUST_BE_TAG;

   /* If the "split" concept has been given and this call uses that concept for a special
      meaning (split square thru, split dixie style), set the special flag to determine that
      action, and remove the split concept.  Why remove it?  So that "heads split catch grand
      mix 3" will work.  If we are doing a "split catch", we don't really want to split the
      setup into 2x2's that are isolated from each other, or else the "grand mix" won't work. */

   if (ss->cmd.cmd_final_flags.final & FINAL__SPLIT) {
      long_boolean starting = TRUE;

      ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;

      /* Check for doing "split square thru" or "split dixie style" stuff.
         But don't propagate the stuff if we aren't doing the first part of the call. */

      if ((ss->cmd.cmd_frac_flags & 0xFF00) != 0x0100 ||
          ((ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) == CMD_FRAC_CODE_FROMTOREV &&
           (ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) > CMD_FRAC_PART_BIT))
         starting = FALSE;     /* We aren't doing the first part. */

      if (callflags1 & CFLAG1_SPLIT_LIKE_SQUARE_THRU) {
         if (starting) ss->cmd.cmd_final_flags.final |= FINAL__SPLIT_SQUARE_APPROVED;
         ss->cmd.cmd_final_flags.final &= ~FINAL__SPLIT;

         if (current_options.howmanynumbers != 0 && (current_options.number_fields & 0xF) <= 1)
            fail("Can't split square thru 1.");
      }
      else if (callflags1 & CFLAG1_SPLIT_LIKE_DIXIE_STYLE) {
         if (starting) ss->cmd.cmd_final_flags.final |= FINAL__SPLIT_DIXIE_APPROVED;
         ss->cmd.cmd_final_flags.final &= ~FINAL__SPLIT;
      }

      if (ss->kind == s4x4) {
         /* The entire rest of the program expects split calls to be done in
            a C1 phantom setup rather than a 4x4. */

         int i, j;
         uint32 mask = 0;

         for (i=0,j=1 ; i<16 ; i++,j<<=1) {
            if (ss->people[i].id1) mask |= j;
         }

         if (mask == 0xAAAA || mask == 0xCCCC) {
            expand_thing *t = (mask & 2) ? &exp_c1phan_4x4_stuff1 : &exp_c1phan_4x4_stuff2;
            compress_setup(t, ss);
         }
      }
   }

   /* NOTE: We may have mirror-reflected the setup.  "Mirror" is true if so.  We may need to undo this. */

   /* If this is the "split sequential" schema and we have not already done so,
      cause splitting to take place. */

   if (the_schema == schema_split_sequential) {
      if (setup_attrs[ss->kind].setup_limits == 7) {
         if ((ss->cmd.cmd_final_flags.her8it & INHERITFLAG_NXNMASK) != INHERITFLAGNXNK_3X3 &&
             (ss->cmd.cmd_final_flags.her8it & INHERITFLAG_NXNMASK) != INHERITFLAGNXNK_4X4 &&
             !(ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK)) {
            if (ss->rotation & 1)
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_VERT;
            else
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;
         }
      }
      else if (setup_attrs[ss->kind].setup_limits == 11 &&
               ((ss->cmd.cmd_final_flags.her8it & INHERITFLAG_MXNMASK) == INHERITFLAGMXNK_1X3 ||
                (ss->cmd.cmd_final_flags.her8it & INHERITFLAG_MXNMASK) == INHERITFLAGMXNK_3X1)) {
         if (!(ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK)) {
            if (ss->rotation & 1)
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_VERT;
            else
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;
         }
      }
      else if ((setup_attrs[ss->kind].setup_limits == 11 &&
                ((ss->cmd.cmd_final_flags.her8it & INHERITFLAG_NXNMASK) == INHERITFLAGNXNK_3X3)) ||
               (setup_attrs[ss->kind].setup_limits == 15 &&
                ((ss->cmd.cmd_final_flags.her8it & INHERITFLAG_NXNMASK) == INHERITFLAGNXNK_4X4)))
         ;    /* No action. */
      /* This used to just bypass 3.  It now bypasses 1 in order to get disband to work in a bone6. */
      else if (setup_attrs[ss->kind].setup_limits != 3 && setup_attrs[ss->kind].setup_limits != 1) {

         /* If this is a 3x4 or 4x4 inhabited in boxes, we allow the splitting into those boxes. */

         int i, j;
         uint32 mask = 0;

         for (i=0, j=1; i<=setup_attrs[ss->kind].setup_limits; i++, j<<=1) {
            if (ss->people[i].id1) mask |= j;
         }

         if (ss->kind == s3x4) {
            if (mask == 0xF3C || mask == 0xCF3)
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;    /* ****** This needs to be looked at */
            else
               fail("Can't split this setup.");
         }
         else if (ss->kind == s4x4) {
            if (mask == 0x4B4B || mask == 0xB4B4)
               ss->cmd.cmd_misc_flags |= CMD_MISC__MUST_SPLIT_HORIZ;    /* ****** This needs to be looked at */
            else
               fail("Can't split this setup.");
         }
         else
            fail("Need a 4 or 8 person setup for this.");
      }
   }

   /* If the split concept is still present, do it. */

   if (ss->cmd.cmd_final_flags.final & FINAL__SPLIT) {
      uint32 split_map;

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"invert/central/snag/mystic\" with the \"split\" concept.");

      ss->cmd.cmd_final_flags.final &= ~FINAL__SPLIT;
      ss->cmd.cmd_misc_flags |= (CMD_MISC__SAID_SPLIT | CMD_MISC__NO_EXPAND_MATRIX);

      /* We can't handle the mirroring, so undo it. */
      if (mirror) { mirror_this(ss); mirror = FALSE; }

      if      (ss->kind == s2x4)   split_map = MAPCODE(s2x2,2,MPKIND__SPLIT,0);
      else if (ss->kind == s1x8)   split_map = MAPCODE(s1x4,2,MPKIND__SPLIT,0);
      else if (ss->kind == s_ptpd) split_map = MAPCODE(sdmd,2,MPKIND__SPLIT,0);
      else if (ss->kind == s_qtag) split_map = MAPCODE(sdmd,2,MPKIND__SPLIT,1);
      else if (ss->kind == s2x2) {
         /* "Split" was given while we are already in a 2x2?  The only way that
            can be legal is if the word "split" was meant as a modifier for "split square thru"
            etc., rather than as a virtual-setup concept, or if the "split sequential" schema
            is in use.  In those cases, some "split approved" flag will still be on. */

         if (     !(ss->cmd.cmd_final_flags.final & (FINAL__SPLIT_SQUARE_APPROVED | FINAL__SPLIT_DIXIE_APPROVED)) &&
                  !(ss->cmd.cmd_frac_flags & CMD_FRAC_BREAKING_UP))   /* If "BREAKING_UP", caller presumably knows what she is doing. */
            warn(warn__excess_split);

         if (ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT)
            fail("\"Matrix\" concept must be followed by applicable concept.");

         move(ss, qtfudged, result);
         result->result_flags &= ~RESULTFLAG__SPLIT_AXIS_FIELDMASK;
         return;
      }
      else
         fail("Can't do split concept in this setup.");

      /* If the user said "matrix split", the "matrix" flag will be on at this point,
         and the right thing will happen. */

      new_divided_setup_move(ss, split_map, phantest_ok, TRUE, result);
      return;
   }

   /* If the "matrix" concept is on and we get here, that is, we haven't acted on a "split" command,
      it is illegal. */

   if (ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT)
      fail("\"Matrix\" concept must be followed by applicable concept.");

   /* Look for empty starting setup.  If definition is by array,
      we go ahead with the call anyway. */

   tbonetest = 0;
   if (setup_attrs[ss->kind].setup_limits >= 0) {
      int j;

      for (j=0; j<=setup_attrs[ss->kind].setup_limits; j++) tbonetest |= ss->people[j].id1;
      if (!(tbonetest & 011) && the_schema != schema_by_array) {
         result->kind = nothing;
         return;
      }
   }

   /* We can't handle the mirroring (or "Z" distortion) unless the schema is by_array,
      so undo it. */

   if (the_schema != schema_by_array) {
      if (mirror) { mirror_this(ss); mirror = FALSE; }
   }

   if ((callflags1 & CFLAG1_FUNNY_MEANS_THOSE_FACING) &&
       (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_FUNNY))) {
      ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_FUNNY;

      /* We have to do this -- we need to know who is facing *now*. */
      update_id_bits(ss);
      foo1 = ss->cmd;
      special_selector = selector_thosefacing;
      goto do_special_select_stuff;
   }

   switch (the_schema) {
   case schema_nothing:
      if ((ss->cmd.cmd_final_flags.her8it & (~(INHERITFLAG_HALF|INHERITFLAG_LASTHALF))) |
          ss->cmd.cmd_final_flags.final)
         fail("Illegal concept for this call.");
      *result = *ss;
      /* This call is a 1-person call, so it can be presumed
         to have split maximally both ways. */
      result->result_flags =
         (ss->cmd.prior_elongation_bits & 3) | RESULTFLAG__SPLIT_AXIS_FIELDMASK;
      break;
   case schema_matrix:
      /* The "reverse" concept might mean mirror, as in "reverse truck". */

      if ((TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_REVERSE)) &&
          (callspec->callflagsh & INHERITFLAG_REVERSE)) {
         mirror_this(ss);
         mirror = TRUE;
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_REVERSE;
      }

      if (ss->kind == s_qtag && (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_12_MATRIX)))
         do_matrix_expansion(ss, CONCPROP__NEEDK_3X4, TRUE);

      if ((ss->cmd.cmd_final_flags.her8it & ~(INHERITFLAG_12_MATRIX|INHERITFLAG_16_MATRIX)) |
          ss->cmd.cmd_final_flags.final)
         fail("Illegal concept for this call.");

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__IN_Z_MASK)
         remove_z_distortion(ss);

      matrixmove(ss, callspec, result);
      break;
   case schema_partner_matrix:
      if (ss->kind == s_qtag && (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_12_MATRIX)))
         do_matrix_expansion(ss, CONCPROP__NEEDK_3X4, TRUE);

      if ((ss->cmd.cmd_final_flags.her8it & ~(INHERITFLAG_12_MATRIX|INHERITFLAG_16_MATRIX)) |
          ss->cmd.cmd_final_flags.final)
         fail("Illegal concept for this call.");

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__IN_Z_MASK)
         remove_z_distortion(ss);

      partner_matrixmove(ss, callspec, result);
      break;
   case schema_roll:
      if (ss->cmd.cmd_final_flags.her8it | ss->cmd.cmd_final_flags.final)
         fail("Illegal concept for this call.");

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__IN_Z_MASK)
         remove_z_distortion(ss);

      rollmove(ss, callspec, result);
      /* This call is a 1-person call, so it can be presumed
         to have split maximally both ways. */
      result->result_flags =
         (ss->cmd.prior_elongation_bits & 3) | RESULTFLAG__SPLIT_AXIS_FIELDMASK;
      break;
   case schema_by_array:

         /* Dispose of the "left" concept first -- it can only mean mirror.  If it is on,
            mirroring may already have taken place. */

      if (TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_LEFT)) {
         /* ***** why isn't this particular error test taken care of more generally elsewhere? */
         if (!(callspec->callflagsh & INHERITFLAG_LEFT)) fail("Can't do this call 'left'.");
         if (!mirror) mirror_this(ss);
         mirror = TRUE;
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_LEFT;
      }

      /* The "reverse" concept might mean mirror, or it might be genuine. */

      if ((TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_REVERSE)) &&
          (callspec->callflagsh & INHERITFLAG_REVERSE)) {
         /* This "reverse" just means mirror. */
         if (mirror) fail("Can't do this call 'left' and 'reverse'.");
         mirror_this(ss);
         mirror = TRUE;
         ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_REVERSE;
      }

      /* If the "reverse" flag is still set in cmd_final_flags, it means a genuine
            reverse as in reverse cut/flip the diamond or reverse change-O. */

      ss->cmd.callspec = callspec;

      basic_move(ss, tbonetest, qtfudged, mirror, result);
      break;
   default:
      /* Must be sequential or some form of concentric. */

         /* We demand that the final concepts that remain be only those in the following list,
            which includes all of the "heritable" concepts. */

      if (ss->cmd.cmd_final_flags.final &
          ~(FINAL__SPLIT | FINAL__SPLIT_SQUARE_APPROVED |
            FINAL__SPLIT_DIXIE_APPROVED | FINAL__LEADTRIANGLE))
         fail("This concept not allowed here.");

         /* Now we figure out how to dispose of "heritable" concepts.  In general, we will selectively inherit them to
            the call's children, once we verify that the call accepts them for inheritance.  If it doesn't accept them,
            it is an error unless the concepts are the special ones "magic" and/or "interlocked", which we can dispose
            of by doing the call in the appropriate magic/interlocked setup. */

      unaccepted_flags = ss->cmd.cmd_final_flags.her8it & (~(callspec->callflagsh|INHERITFLAG_HALF|INHERITFLAG_LASTHALF));    /* The unaccepted flags. */

      if (unaccepted_flags != 0) {
         if (divide_for_magic(ss, unaccepted_flags, result))
            return;
         else
            fail("Can't do this call with this concept.");
      }

      if (the_schema >= schema_sequential) {
         uint32 misc2 = ss->cmd.cmd_misc2_flags;


         if ((misc2 & CMD_MISC2__CENTRAL_SNAG) &&
             ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) == 0 ||
              (((ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) != CMD_FRAC_CODE_ONLY) &&
               ((ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) != CMD_FRAC_CODE_ONLYREV)))) {
            if (ss->cmd.cmd_frac_flags != CMD_FRAC_NULL_VALUE)
               fail("Can't fractionalize a call and use \"snag\" at the same time.");

            ss->cmd.cmd_misc2_flags &=
               ~(CMD_MISC2__CENTRAL_SNAG | CMD_MISC2__INVERT_SNAG);
            ss->cmd.cmd_frac_flags = CMD_FRAC_HALF_VALUE;

            /* Note the uncanny similarity between the following and
               "punt_centers_use_concept". */
            {
               int i;
               int m, j;
               uint32 ssmask;
               warning_info saved_warnings;
               setup the_setups[2], the_results[2], orig_people;
               int sizem1 = setup_attrs[ss->kind].setup_limits;
               int crossconc = (misc2 & CMD_MISC2__INVERT_SNAG) ? 0 : 1;

               ssmask = setup_attrs[ss->kind].mask_normal;

                  /* note who the original centers are. */

               if (sizem1 < 0 || ssmask == 0) fail("Can't identify centers and ends.");

               orig_people = *ss;

               for (i=sizem1; i>=0; i--) {
                  orig_people.people[i].id2 = (ssmask ^ crossconc) & 1;
                  ssmask >>= 1;
               }

               move(ss, FALSE, result);   /* Everyone does the first half of the call. */

               the_setups[0] = *result;              /* designees */
               the_setups[1] = *result;              /* non-designees */

               m = setup_attrs[result->kind].setup_limits;
               if (m < 0) fail("Can't identify centers and ends.");

               for (j=0; j<=m; j++) {
                  uint32 p = result->people[j].id1;

                  if (p & BIT_PERSON) {
                     for (i=0; i<=sizem1; i++) {
                        if (((orig_people.people[i].id1 ^ p) & XPID_MASK) == 0) {
                           clear_person(&the_setups[orig_people.people[i].id2], j);
                           goto did_it;
                        }
                     }
                     fail("Lost someone else during snag call.");
                  did_it: ;
                  }
               }

               /* Now the_setups[0] has original centers, after completion of first half
                     of the call, and the_setups[1] has original ends, also after completion.
                     We will have the_setups[0] proceed with the rest of the call. */

               normalize_setup(&the_setups[0], normalize_before_isolated_call);
               saved_warnings = history[history_ptr+1].warnings;

               the_setups[0].cmd = ss->cmd;
               the_setups[0].cmd.cmd_misc_flags |= CMD_MISC__PHANTOMS;
               the_setups[0].cmd.cmd_frac_flags = CMD_FRAC_LASTHALF_VALUE;
               move(&the_setups[0], FALSE, &the_results[0]);

               the_results[1] = the_setups[1];

               /* Shut off "each 1x4" types of warnings -- they will arise spuriously while
                     the people do the calls in isolation. */
               for (i=0 ; i<WARNING_WORDS ; i++) {
                  history[history_ptr+1].warnings.bits[i] &= ~dyp_each_warnings.bits[i];
                  history[history_ptr+1].warnings.bits[i] |= saved_warnings.bits[i];
               }

               *result = the_results[1];
               result->result_flags = get_multiple_parallel_resultflags(the_results, 2);
               merge_setups(&the_results[0], merge_c1_phantom, result);
            }

            /* ******* end of "punt_centers" junk. */

         }
         else
            do_sequential_call(ss, callspec, qtfudged, &mirror, result);

         if (the_schema == schema_split_sequential && result->kind == s2x6 && 
             ((ss->cmd.cmd_final_flags.her8it & INHERITFLAG_MXNMASK) == INHERITFLAGMXNK_1X3 ||
              (ss->cmd.cmd_final_flags.her8it & INHERITFLAG_MXNMASK) == INHERITFLAGMXNK_3X1)) {
            int i, j;
            uint32 mask = 0;
            static Const veryshort map_3x1fixa[8] = {0, 1, 2, 4, 6, 7, 8, 10};
            static Const veryshort map_3x1fixb[8] = {1, 3, 4, 5, 7, 9, 10, 11};

            for (i=0, j=1; i<12; i++, j<<=1) {
               if (result->people[i].id1) mask |= j;
            }

            if (mask == 02727) {
               setup temp = *result;
               clear_people(result);
               gather(result, &temp, map_3x1fixa, 7, 0);
               result->kind = s2x4;
            }
            else if (mask == 07272) {
               setup temp = *result;
               clear_people(result);
               gather(result, &temp, map_3x1fixb, 7, 0);
               result->kind = s2x4;
            }
         }
      }
      else {
         Const by_def_item *innerdef = &callspec->stuff.conc.innerdef;
         Const by_def_item *outerdef = &callspec->stuff.conc.outerdef;
         parse_block *parseptr = ss->cmd.parseptr;

         /* Must be some form of concentric, or a "sel_XXX" schema. */

         switch (the_schema) {
         case schema_single_concentric_together:
         case schema_select_original_rims:
         case schema_select_original_hubs:
         case schema_single_concentric:
         case schema_select_ctr2:
         case schema_select_ctr4:
         case schema_select_ctr6:
            break;
         default:
            if (ss->cmd.cmd_misc_flags & CMD_MISC__MUST_SPLIT_MASK)
               fail("Can't split this call.");
            break;
         }

         (void) get_real_subcall(parseptr, innerdef,
                                 &ss->cmd, callspec, &foo1);

         (void) get_real_subcall(parseptr, outerdef,
                                 &ss->cmd, callspec, &foo2);

         foo1.cmd_frac_flags = ss->cmd.cmd_frac_flags;
         foo2.cmd_frac_flags = ss->cmd.cmd_frac_flags;

         if (the_schema == schema_select_leads) {
            inner_selective_move(ss, &foo1, &foo2,
                                 selective_key_plain, TRUE, 0, 0,
                                 selector_leads,
                                 innerdef->modifiers1,
                                 outerdef->modifiers1,
                                 result);
         }
         else if (the_schema == schema_select_headliners) {
            inner_selective_move(ss, &foo1, &foo2,
                                 selective_key_plain, TRUE, 0, 0x80000008UL,
                                 selector_uninitialized,
                                 innerdef->modifiers1,
                                 outerdef->modifiers1,
                                 result);
         }
         else if (the_schema == schema_select_sideliners) {
            inner_selective_move(ss, &foo1, &foo2,
                                 selective_key_plain, TRUE, 0, 0x80000001UL,
                                 selector_uninitialized,
                                 innerdef->modifiers1,
                                 outerdef->modifiers1,
                                 result);
         }
         else if (the_schema == schema_select_ctr2 || the_schema == schema_select_ctr4) {
            if ((ss->kind == s2x4 &&
                 !(ss->people[1].id1 | ss->people[2].id1 |
                   ss->people[5].id1 | ss->people[6].id1)) ||
                (ss->kind == s2x6 &&
                 !(ss->people[2].id1 | ss->people[3].id1 |
                   ss->people[8].id1 | ss->people[9].id1))) {
               *result = *ss;
            }
            else {
               /* We have to do this -- the schema means the *current* centers. */
               update_id_bits(ss);
               special_selector = (the_schema == schema_select_ctr2) ?
                                    selector_center2 : selector_center4;
               special_modifiers = innerdef->modifiers1;
               goto do_special_select_stuff;
            }
         }
         else if (the_schema == schema_select_ctr6) {
            /* We have to do this -- the schema means the *current* centers. */
            update_id_bits(ss);
            special_selector = selector_center6;
            special_modifiers = innerdef->modifiers1;
            goto do_special_select_stuff;
         }
         else if (the_schema == schema_select_original_rims) {
            special_selector = selector_ends;
            special_modifiers = innerdef->modifiers1;
            special_indicator = selective_key_plain_from_id_bits;
            goto do_special_select_stuff;
         }
         else if (the_schema == schema_select_original_hubs) {
            special_selector = selector_centers;
            special_modifiers = innerdef->modifiers1;
            special_indicator = selective_key_plain_from_id_bits;
            goto do_special_select_stuff;
         }
         else {
            int i;
            int rot = 0;
            long_boolean normalize_strongly = FALSE;
            warning_info saved_warnings = history[history_ptr+1].warnings;

            ss->cmd.cmd_misc_flags |= CMD_MISC__NO_EXPAND_MATRIX;     /* We think this is the
                                                                         right thing to do. */
            /* Fudge a 3x4 into a 1/4-tag if appropriate. */

            if (ss->kind == s3x4 && (callflags1 & CFLAG1_FUDGE_TO_Q_TAG) &&
                (the_schema == schema_concentric ||
                 the_schema == schema_cross_concentric ||
                 the_schema == schema_conc_o)) {

               if (ss->people[0].id1) {
                  if (ss->people[1].id1) fail("Can't do this call from arbitrary 3x4 setup.");
               }
               else (void) copy_person(ss, 0, ss, 1);

               if (ss->people[3].id1) {
                  if (ss->people[2].id1) fail("Can't do this call from arbitrary 3x4 setup.");
                  else (void) copy_person(ss, 1, ss, 3);
               }
               else (void) copy_person(ss, 1, ss, 2);

               (void) copy_person(ss, 2, ss, 4);
               (void) copy_person(ss, 3, ss, 5);

               if (ss->people[6].id1) {
                  if (ss->people[7].id1) fail("Can't do this call from arbitrary 3x4 setup.");
                  else (void) copy_person(ss, 4, ss, 6);
               }
               else (void) copy_person(ss, 4, ss, 7);

               if (ss->people[9].id1) {
                  if (ss->people[8].id1) fail("Can't do this call from arbitrary 3x4 setup.");
                  else (void) copy_person(ss, 5, ss, 9);
               }
               else (void) copy_person(ss, 5, ss, 8);

               (void) copy_person(ss, 6, ss, 10);
               (void) copy_person(ss, 7, ss, 11);

               ss->kind = s_qtag;
               ss->cmd.cmd_misc_flags |= CMD_MISC__DISTORTED;
            }
            else if (ss->kind == s_qtag &&
                     (callflags1 & CFLAG1_FUDGE_TO_Q_TAG) &&
                     (the_schema == schema_in_out_triple ||
                      the_schema == schema_in_out_triple_squash)) {
               /* Or change from qtag to 3x4 if schema requires same. */
               (void) copy_person(ss, 11, ss, 7);
               (void) copy_person(ss, 10, ss, 6);
               (void) copy_person(ss, 8, ss, 5);
               (void) copy_person(ss, 7, ss, 4);
               (void) copy_person(ss, 5, ss, 3);
               (void) copy_person(ss, 4, ss, 2);
               (void) copy_person(ss, 2, ss, 1);
               (void) copy_person(ss, 1, ss, 0);
               clear_person(ss, 0);
               clear_person(ss, 3);
               clear_person(ss, 6);
               clear_person(ss, 9);

               ss->kind = s3x4;
            }

            switch (the_schema) {
            case schema_in_out_triple_squash:
            case schema_in_out_triple:
            case schema_in_out_quad:
               normalize_strongly = TRUE;
   
               if (ss->kind == s2x4)
                  do_matrix_expansion(ss, CONCPROP__NEEDK_4X4, FALSE);
   
               if (ss->kind == s4x4) {
   
                  /* We need to orient the setup so that it is vertical with respect
                     to the way we want to pick out the outer 1x4's. */
   
                  uint32 t1 = ss->people[1].id1 | ss->people[2].id1 | ss->people[9].id1 | ss->people[10].id1;
                  uint32 t2 = ss->people[5].id1 | ss->people[6].id1 | ss->people[13].id1 | ss->people[14].id1;
   
                  if (t1 && !t2)      rot = 1;              /* It has to be left-to-right. */
                  else if (t2 && !t1) ;                     /* It has to be top-to-bottom. */
                  else if (t2 && t1) fail("Can't pick out outside lines.");
                  else {
                     /* This "O" spots are unoccupied.  Try to figure it out from the orientation of the
                        people in the corners, so that we have lines. */
                     uint32 t3 = ss->people[0].id1 | ss->people[4].id1 | ss->people[8].id1 | ss->people[12].id1;
                     if ((t3 & 011) == 011) fail("Can't pick out outside lines.");
                     else if (t3 & 1)        rot = 1;
                     /* Otherwise, it either needs a vertical orientaion, or only the center 2x2 is occupied,
                        in which case it doesn't matter. */
                  }
   
                  ss->rotation += rot;
                  canonicalize_rotation(ss);
               }
               break;
            case schema_3x3_concentric:
               if (!(TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_12_MATRIX)) &&
                   !(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX))
                  fail("You must specify a matrix.");

               if (ss->kind == s2x6)
                  do_matrix_expansion(ss, CONCPROP__NEEDK_4X6, FALSE);
               else
                  do_matrix_expansion(ss, CONCPROP__NEEDK_3X4_D3X4, FALSE);

               break;
            case schema_4x4_lines_concentric:
            case schema_4x4_cols_concentric:
               if (!(TEST_HERITBITS(ss->cmd.cmd_final_flags,INHERITFLAG_16_MATRIX)) &&
                   !(ss->cmd.cmd_misc_flags & CMD_MISC__EXPLICIT_MATRIX))
                  fail("You must specify a matrix.");

               if (ss->kind == s2x4)
                  do_matrix_expansion(ss, CONCPROP__NEEDK_4X4, FALSE);

               break;
            }

            concentric_move(
                            ss, &foo1, &foo2,
                            the_schema,
                            innerdef->modifiers1,
                            outerdef->modifiers1,
                            TRUE,
                            result);

            result->rotation -= rot;   /* Flip the setup back. */

               /* If we expanded a 2x2 to a 4x4 and then chose an orientation at random,
                  that orientation may have gotten frozen into a 2x4.  Cut it back to
                  just the center 4, so that the illegal information won't affect anything. */

            if (normalize_strongly)
               normalize_setup(result, normalize_after_triple_squash);

            if (DFM1_SUPPRESS_ELONGATION_WARNINGS & outerdef->modifiers1) {
               for (i=0 ; i<WARNING_WORDS ; i++)
                  history[history_ptr+1].warnings.bits[i] &= ~conc_elong_warnings.bits[i];
            }
            for (i=0 ; i<WARNING_WORDS ; i++)
               history[history_ptr+1].warnings.bits[i] |= saved_warnings.bits[i];
         }
      }
      break;
   }

   goto foobarf;

 do_special_select_stuff:

   if (special_selector == selector_none) fail("Can't do this call in this formation.");

   inner_selective_move(ss, &foo1, (setup_command *) 0,
                        special_indicator, FALSE, 0, 0,
                        special_selector, special_modifiers, 0, result);

 foobarf:

   /* The definitions for things like "U-turn back" are sometimes
      in terms of setups larger than 1x1 for complex reasons related
      to roll direction, elongation checking, and telling which way
      "in" is.  But in fact they are treated as 1-person calls in
      terms of "stretch", "crazy", etc. */
   if (callflags1 & CFLAG1_ONE_PERSON_CALL)
      result->result_flags |= RESULTFLAG__SPLIT_AXIS_FIELDMASK;

   result->result_flags |= imprecise_rotation_result_flag;
   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__DID_Z_COMPRESSION)
      result->result_flags |= RESULTFLAG__DID_Z_COMPRESSION;

   /* Reflect back if necessary. */
   if (mirror) mirror_this(result);
   canonicalize_rotation(result);

   if (did_4x4_expansion) {
      setup outer_inners[2];
      outer_inners[0] = *result;
      outer_inners[1].kind = nothing;
      outer_inners[1].result_flags = 0;
      normalize_concentric(schema_conc_o, 1, outer_inners, 1, result);
      if (result->kind == s2x4) {
         if (result->people[1].id1 | result->people[2].id1 | result->people[5].id1 | result->people[6].id1)
            fail("Internal error: 'O' people wandered into middle.");
         swap_people(result, 1, 3);
         swap_people(result, 2, 4);
         swap_people(result, 3, 7);
         result->kind = s2x2;
         result->result_flags = (result->result_flags & ~3) | (result->rotation+1);
         canonicalize_rotation(result);
      }
   }
}



/* The current interpretation of the elongation flags, on input and output, is now
   as follows:

   Note first that, on input and output, the elongation bits are only meaningful in
      a 2x2 or short6 setup.
   On input, nonzero bits in "prior_elongation_bits" field with a 2x2 setup mean that
      the setup _was_ _actually_ _elongated_, and that the elongation is actually felt
      by the dancers.  In this case, the "move" routine is entitled to raise an error
      if the 2x2 call is awkward.  For example, a star thru from facing couples is
      illegal if the elongate bit is on that makes the people far away from the one
      they are facing.  It follows from this that, if we call concentric star thru,
      these bits will be cleared before calling "move", since the concentric concept
      forgives awkward elongation.  Of course, the "concentric_move" routine will
      remember the actual elongation in order to move people to the correct ending
      formation.
   On output, nonzero bits in the low two bits with a 2x2 setup mean that,
      _if_ _result_ _elongation_ _is_ _required_, this is what it should be.  It does
      _not_ mean that such elongation actually exists.  Whoever called "move" must
      make that judgement.  These bits are set when, for example, a 1x4 -> 2x2 call
      is executed, to indicate how to elongate the result, if it turns out that those
      people were working around the outside.  This determination is made not just on
      the "checkpoint rule" that says they go perpendicular to their original axis,
      but also on the basis of the "parallel_conc_end" flag in the call.  That is, these
      bits tell which way to go if the call had been "ends do <whatever>".  Of course,
      if the concentric concept was in use, this information is not used.  Instead,
      "concentric_move" overrides the bits we return with an absolute "checkpoint rule"
      application.

   It may well be that the goal described above is not actually implemented correctly.
*/



extern void move(
   setup *ss,
   long_boolean qtfudged,
   setup *result)
{
   parse_block *saved_magic_diamond;
   uint64 check_concepts;
   parse_block *parseptrcopy;
   parse_block *parseptr = ss->cmd.parseptr;
   uint64 save_incoming_final;

   /* This shouldn't be necessary, but there have been occasional reports of the
      bigblock and stagger concepts getting confused with each other.  This would happen
      if the incoming 4x4 did not have its rotation field equal to zero, as is required
      when in canonical form.  So we check this. */

   if (setup_attrs[ss->kind].four_way_symmetry && ss->rotation != 0)
      fail("There is a bug in 4 way canonicalization -- please report this sequence.");

   /* See if there is a restrained concept that has been released. */

   if (ss->cmd.restrained_concept && !(ss->cmd.cmd_misc_flags & CMD_MISC__RESTRAIN_CRAZINESS)) {
      parse_block *t = ss->cmd.restrained_concept;
      ss->cmd.restrained_concept = (parse_block *) 0;

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__IN_Z_MASK)
         remove_z_distortion(ss);

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2_RESTRAINED_SUPER)
         fail("Can't nest meta-concepts and supercalls.");

      if (t->concept->kind == concept_another_call_next_mod) {
         /* This is a "supercall". */
         parse_block p1 = *t;
         parse_block p2 = *p1.next;
         parse_block p3 = *(p2.subsidiary_root);

         p1.next = &p2;
         p2.concept = &marker_concept_supercall;
         p2.subsidiary_root = &p3;
         p3.call = ss->cmd.callspec;
         p3.call_to_print = p3.call;
         p3.concept = &mark_end_of_list;
         p3.no_check_call_level = 1;
         p3.options = current_options;
         ss->cmd.parseptr = &p1;
         ss->cmd.callspec = (callspec_block *) 0;
         ss->cmd.cmd_misc2_flags |= CMD_MISC2_RESTRAINED_SUPER;
         ss->cmd.restrained_super8flags = ss->cmd.cmd_final_flags.her8it;
         ss->cmd.cmd_final_flags.her8it = 0;
         move(ss, FALSE, result);
      }
      else {

         /* We need to find the end of the concept chain, and plug in our
            call, after saving its old contents. */
         callspec_block *saved_new_call;
         callspec_block *saved_old_call = ss->cmd.callspec;
         call_conc_option_state saved_options = ss->cmd.parseptr->options;
         parse_block *z1 = t;
         if (saved_old_call) ss->cmd.parseptr->options = current_options;
         ss->cmd.callspec = (callspec_block *) 0;
         while (z1->concept->kind > marker_end_of_list) z1 = z1->next;

         if (saved_old_call != base_calls[base_call_basetag0]) {
            if (z1->concept->kind == concept_another_call_next_mod) {
               z1 = z1->next->subsidiary_root;
            }
         }

         saved_new_call = z1->call;
         if (saved_old_call) z1->call = saved_old_call;
         z1->no_check_call_level = 1;

         ss->cmd.cmd_misc_flags |= CMD_MISC__RESTRAIN_MODIFIERS;
         ss->cmd.restrained_super8flags = ss->cmd.cmd_final_flags.her8it;
         ss->cmd.cmd_final_flags.her8it = 0;

         (concept_table[t->concept->kind].concept_action)(ss, t, result);
         if (saved_old_call) {
            z1->call = saved_new_call;
            ss->cmd.parseptr->options = saved_options;
         }
         ss->cmd.callspec = saved_old_call;

         ss->cmd.cmd_misc_flags &= ~CMD_MISC__RESTRAIN_MODIFIERS;
         ss->cmd.cmd_final_flags.her8it = ss->cmd.restrained_super8flags;
      }

      return;
   }

   if (ss->cmd.cmd_misc_flags & CMD_MISC__DO_AS_COUPLES) {
      uint32 mxnflags;

      /* If we have a pending "centers/ends work <concept>" concept,
         we must dispose of it the crude way. */

      if (ss->cmd.cmd_misc2_flags & (CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG)) {
         punt_centers_use_concept(ss, result);
         return;
      }

      ss->cmd.cmd_misc_flags &= ~CMD_MISC__DO_AS_COUPLES;
      mxnflags = ss->cmd.do_couples_her8itflags &
         (INHERITFLAG_SINGLE | INHERITFLAG_MXNMASK | INHERITFLAG_NXNMASK);

      /* Mxnflags now has the "single" bit, or any "1x3" stuff.  If it is the "single"
         bit alone, we do the call directly--we don't do "as couples".  Otherwise,
         we the do call as couples, passing any modifiers. */

      ss->cmd.do_couples_her8itflags &= ~mxnflags;

      if (mxnflags != INHERITFLAG_SINGLE) {
         tandem_couples_move(ss, selector_uninitialized, 0, 0, 0,
                             tandem_key_cpls, mxnflags, TRUE, result);
         return;
      }
   }

   if (ss->cmd.callspec) {
      /* This next thing shouldn't happen -- we shouldn't have a call in place
         when there is a pending "centers/ends work <concept>" concept,
         since that concept should be next. */
      if (ss->cmd.cmd_misc2_flags & (CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG)) {
         punt_centers_use_concept(ss, result);
         return;
      }

      move_with_real_call(ss, qtfudged, result);
      return;
   }

   /* Scan the "final" concepts, remembering them and their end point. */
   last_magic_diamond = 0;

   /* But if we have a pending "centers/ends work <concept>" concept, don't. */

   if (ss->cmd.cmd_misc2_flags & CMD_MISC2__ANY_WORK) {
      concept_kind kjunk;
      uint32 njunk;

      (void) really_skip_one_concept(ss->cmd.parseptr, &kjunk, &njunk, &parseptrcopy);
      if (kjunk == concept_supercall)
         fail("A concept is required.");
   }
   else
      parseptrcopy = parseptr;

   /* We will merge the new concepts with whatever we already had. */

   save_incoming_final = ss->cmd.cmd_final_flags;   /* In case we need to punt. */

   parseptrcopy = process_final_concepts(parseptrcopy, TRUE, &ss->cmd.cmd_final_flags);
   saved_magic_diamond = last_magic_diamond;

   if (parseptrcopy->concept->kind <= marker_end_of_list) {
      call_conc_option_state saved_options = current_options;
      callspec_block *this_call = parseptrcopy->call;

      /* There are no "big" concepts.  The only concepts are the "little" ones that
         have been encoded into cmd_final_flags. */

      if ((ss->cmd.cmd_misc2_flags & (CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG))) {
         switch (this_call->schema) {
         case schema_concentric:
         case schema_concentric_6p:
         case schema_concentric_6p_or_sgltogether:
         case schema_concentric_6p_or_normal:
         case schema_conc_o:
            break;
         default:
            ss->cmd.cmd_final_flags = save_incoming_final;
            punt_centers_use_concept(ss, result);
            return;
         }
      }
      else if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CENTRAL_SNAG) {
         /* Doing plain snag -- we are tolerant of calls like 6x2 acey deucey. */
         /* But if we are doing something like "initially snag" or "finally snag", don't. */
         if ((ss->cmd.cmd_frac_flags & CMD_FRAC_PART_MASK) == 0 ||
             (((ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) != CMD_FRAC_CODE_ONLY) &&
              ((ss->cmd.cmd_frac_flags & CMD_FRAC_CODE_MASK) != CMD_FRAC_CODE_ONLYREV))) {
            switch (this_call->schema) {
            case schema_concentric:
            case schema_concentric_6_2:
            case schema_concentric_2_6:
            case schema_concentric_6p:
            case schema_concentric_6p_or_sgltogether:
            case schema_concentric_6p_or_normal:
            case schema_conc_o:
               break;
            default:
               ss->cmd.cmd_final_flags = save_incoming_final;
               punt_centers_use_concept(ss, result);
               return;
            }
         }
      }

      /* The "anyone work" stuff will need this. */
      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__ANY_WORK)
         ss->cmd.skippable_concept = ss->cmd.parseptr;

      /* We must read the selector, direction, and number out of the concept list and use them
         for this call to "move".  We are effectively using them as arguments to "move",
         with all the care that must go into invocations of recursive procedures.  However,
         at their point of actual use, they must be in global variables.  Therefore, we
         explicitly save and restore those global variables (in dynamic variables local
         to this instance) rather than passing them as explicit arguments.  By saving
         them and restoring them in this way, we make things like "checkpoint bounce
         the beaus by bounce the belles" work. */

      ss->cmd.parseptr = parseptrcopy;
      ss->cmd.callspec = this_call;
      current_options = parseptrcopy->options;

      if (((dance_level) this_call->level) > calling_level &&
          !parseptrcopy->no_check_call_level)
         warn(warn__bad_call_level);

      move_with_real_call(ss, qtfudged, result);
      current_options = saved_options;
   }
   else {
      /* We now know that there are "non-final" (virtual setup) concepts present. */

      /* If we have a pending "centers/ends work <concept>" concept,
         we must dispose of it the crude way. */

      if (ss->cmd.cmd_misc2_flags & (CMD_MISC2__ANY_WORK | CMD_MISC2__ANY_SNAG)) {
         ss->cmd.cmd_final_flags = save_incoming_final;
         punt_centers_use_concept(ss, result);
         return;
      }

      /* These are the concepts that we are interested in. */

      check_concepts = ss->cmd.cmd_final_flags;
      check_concepts.final &= ~FINAL__MUST_BE_TAG;

      result->result_flags = 0;

      /* ***** We used to have FINAL__SPLIT in the list below, but it caused "matrix split, tandem step thru" to fail.
         This needs to be reworked. */

      if (((check_concepts.her8it &
            (INHERITFLAG_DIAMOND | INHERITFLAG_HALF | INHERITFLAG_LASTHALF)) |
           check_concepts.final) == 0) {
         /* Look for virtual setup concept that can be done by dispatch from table, with no
            intervening final concepts. */

         ss->cmd.parseptr = parseptrcopy;

         /* We know that ss->callspec is null. */
         /* ss->cmd.cmd_final_flags may contain
            FINAL__MUST_BE_TAG and/or one of the modifiers (reverse/left/grand/cross/single)
            listed above.  do_big_concept will take care of the latter. */

         if (ss->cmd.cmd_misc2_flags & CMD_MISC2__IN_Z_MASK)
            remove_z_distortion(ss);

         if (do_big_concept(ss, result)) {
            canonicalize_rotation(result);
            return;
         }
      }

      clear_people(result);

      if (ss->cmd.cmd_misc2_flags & CMD_MISC2__CTR_END_MASK)
         fail("Can't do \"invert/central/snag/mystic\" followed by another concept or modifier.");

      /* Some final concept (e.g. "magic") is present in front of our virtual setup concept.
         We have to dispose of it.  This means that expanding the matrix (e.g. 2x4->2x6)
         and stepping to a wave or rearing back from one are no longer legel. */

      ss->cmd.cmd_misc_flags |= (CMD_MISC__NO_EXPAND_MATRIX | CMD_MISC__NO_STEP_TO_WAVE);

      /* There are a few "final" concepts that
         will not be treated as such if there are non-final concepts occurring
         after them.  Instead, they will be treated as virtual setup concepts.
         This is what makes "magic once removed trade" work, for
         example.  On the other hand, if there are no non-final concepts following, treat these as final.
         This is what makes "magic transfer" or "split square thru" work. */

      ss->cmd.parseptr = parseptrcopy;
      ss->cmd.callspec = (callspec_block *) 0;

      /* We can tolerate the "matrix" flag if we are going to do "split".  For anything else,
         "matrix" is illegal. */

      if (check_concepts.final == FINAL__SPLIT && check_concepts.her8it == 0) {
         uint32 split_map;

         ss->cmd.cmd_misc_flags |= CMD_MISC__SAID_SPLIT;

         if      (ss->kind == s2x4)   split_map = MAPCODE(s2x2,2,MPKIND__SPLIT,0);
         else if (ss->kind == s1x8)   split_map = MAPCODE(s1x4,2,MPKIND__SPLIT,0);
         else if (ss->kind == s_ptpd) split_map = MAPCODE(sdmd,2,MPKIND__SPLIT,0);
         else if (ss->kind == s_qtag) split_map = MAPCODE(sdmd,2,MPKIND__SPLIT,1);
         else fail("Can't do split concept in this setup.");

         ss->cmd.cmd_final_flags.final &= ~FINAL__SPLIT;
         new_divided_setup_move(ss, split_map, phantest_ok, TRUE, result);
      }
      else {
         if (ss->cmd.cmd_misc_flags & CMD_MISC__MATRIX_CONCEPT)
            fail("\"Matrix\" concept must be followed by applicable concept.");

         if (divide_for_magic(
               ss,
               check_concepts.her8it & ~INHERITFLAG_DIAMOND,
               result)) {
         }
         else if (check_concepts.her8it == INHERITFLAG_DIAMOND && check_concepts.final == 0) {
            ss->cmd.cmd_final_flags.her8it &= ~INHERITFLAG_DIAMOND;

            if (ss->kind == sdmd)
               new_divided_setup_move(ss, MAPCODE(s1x2,2,MPKIND__DMD_STUFF,0), phantest_ok, TRUE, result);
            else {
               /* Divide into diamonds and try again.  (Note that we back up the concept pointer.) */
               ss->cmd.parseptr = parseptr;
               ss->cmd.cmd_final_flags.final = 0;
               ss->cmd.cmd_final_flags.her8it = 0;
               divide_diamonds(ss, result);
            }
         }
         else
            fail2("Can't do this concept with other concepts preceding it:", parseptrcopy->concept->menu_name);
      }
   }

   /* If execution of the call raised a request that we change a concept name from "magic" to
      "magic diamond,", for example, do so. */

   if ((result->result_flags & RESULTFLAG__NEED_DIAMOND) && saved_magic_diamond && saved_magic_diamond->concept->value.arg1 == 0) {
      if (saved_magic_diamond->concept->kind == concept_magic) saved_magic_diamond->concept = &special_magic;
      else if (saved_magic_diamond->concept->kind == concept_interlocked) saved_magic_diamond->concept = &special_interlocked;
   }
}
