TRACE STATE = 0:

moves\6336, before do_stuff_inside_sequential_call

      if (!expanding_database && trace_progress == 0 && zzz.m_fetch_index == 1) {
         trace_progress++;
         char junk[200];
         sprintf(junk, "BEFORE STUFFINSIDE[1] %d/%d.", result->kind, this_item->call_id);
         fail(junk);
      }
This gets 23/16 = qtag / base_call_ctrrot

(This [2] also works, 77, 36.  ptpd!!!!!  and hinge.  This is the result of the counter rotate!)

---------------------------------------------------------------------------------------
TRACE STATE = 1:
   if (!expanding_database && trace_progress == 1) {
      trace_progress++;
          char junk[200];
          sprintf(junk, "MOVE[1] %d.", ss->kind);
          fail(junk);
   }
This gets 23 = qtag

---------------------------------------------------------------------------------------
TRACE STATE = 2:
Doing the counter rotate, out of do_sequential_call, in move_with_real_call
   if (!expanding_database && trace_progress == 2) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "MOVE_WITH_REAL_CALL[2] %d.", ss->kind);
      fail(junk);
   }

This gets 23 = qtag

---------------------------------------------------------------------------------------
TRACE STATE = 3:
In move_with_real_call, a little farther down.
   if (!expanding_database && trace_progress == 3) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "MOVE_WITH_REAL_CALL[3] %d.", ss->kind);
      fail(junk);
   }

This gets 23 = qtag

---------------------------------------------------------------------------------------
TRACE STATE = 4:
Farther down, after get schema.
PULL OUT THE SECOND-CHANCE HANDLER!!!!!!

      if (!expanding_database && trace_progress == 4) {
         trace_progress++;
         char junk[200];
         sprintf(junk, "MOVE_WITH_REAL_CALL[4] %d %d.", ss->kind, the_schema);
         fail(junk);
      }

This gets 23 = qtag, 144 = schema_counter_rotate

---------------------------------------------------------------------------------------
TRACE STATE = 5:
Farther down
      if (!expanding_database && trace_progress == 5) {
         trace_progress++;
         char junk[200];
         sprintf(junk, "MOVE_WITH_REAL_CALL[5] %d %d %d.", ss->kind, the_schema, force_split);
         fail(junk);
      }

This gets 23 = qtag, 144 = schema_counter_rotate, 0 = split_command_none

---------------------------------------------------------------------------------------
TRACE STATE = 6:
Farther down, just before calling really_inner_move

      if (!expanding_database && trace_progress == 6) {
         trace_progress++;
         char junk[200];
         sprintf(junk, "MOVE_WITH_REAL_CALL[6] %d %d.", ss->kind, the_schema);
         fail_no_retry(junk);
      }

This gets 23 = qtag, 144 = schema_counter_rotate

---------------------------------------------------------------------------------------
TRACE STATE = 7:
really_inner_move


   if (!expanding_database && trace_progress == 7) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "REALLY_INNER_MOVE[7] %d %d.", ss->kind, the_schema);
      fail(junk);
   }

This gets 23 = qtag, 144 = schema_counter_rotate

---------------------------------------------------------------------------------------
TRACE STATE = 8:
really_inner_move, inside switch

         if (!expanding_database && trace_progress == 8) {
            trace_progress++;
            char junk[200];
            sprintf(junk, "REALLY_INNER_MOVE[8], CASE %d %d 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X 0x%08X.",
                    ss->kind,
                    the_schema,
                    ss->cmd.cmd_misc_flags,
                    (uint32_t) callspec->stuff.matrix.matrix_flags,
                    (uint32_t) ss->cmd.cmd_final_flags.final,
                    (uint32_t) (ss->cmd.cmd_final_flags.herit >> 32),
                    (uint32_t) (ss->cmd.cmd_final_flags.herit & 0xFFFFFFFFULL),
                    (uint32_t) base_block,
                    (uint32_t) base_block->next,
                    (uint32_t) (base_block->alternate_def_flags >> 32),
                    (uint32_t) (base_block->alternate_def_flags & 0xFFFFFFFFULL));
            fail_no_retry(junk);
         }

This gets 23 = qtag, 144 = schema_counter_rotate, misc = 0x30038000 matrixflags=0
   final=0 herit=0,0
   base_block = 00A707B8
   next = 0
   alternate_def = 0,0x00001000
    !!!!!!!!!!!!!!!!!! ^^^^^^^^ !!!!     This is INHERITFLAG_HALF

---------------------------------------------------------------------------------------
TRACE STATE = 9:
really_inner_move, deep

         if (!expanding_database && trace_progress == 9) {
            trace_progress++;
            char junk[200];
            sprintf(junk, "REALLY_INNER_MOVE[9], deep %d %d 0x%08X.", ss->kind, the_schema, ss->cmd.cmd_misc_flags);
            fail_no_retry(junk);
         }

This gets 23 = qtag, 144 = schema_counter_rotate, 0x30038000

---------------------------------------------------------------------------------------
TRACE STATE = 10:
in matrixmove, starting

   if (!expanding_database && trace_progress == 10) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "MATRIXMOVE[10] %d %d 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              ss->kind, ss->rotation,
              ss->people[0].id1 & 0xFF,
              ss->people[1].id1 & 0xFF,
              ss->people[2].id1 & 0xFF,
              ss->people[3].id1 & 0xFF,
              ss->people[4].id1 & 0xFF,
              ss->people[5].id1 & 0xFF,
              ss->people[6].id1 & 0xFF,
              ss->people[7].id1 & 0xFF);
      fail(junk);
   }

This gets (at start) qtag, rot=0, phantoms outside, RWAVE in the center.

---------------------------------------------------------------------------------------
TRACE STATE = 11:
in matrixmove, people have moved.

   if (!expanding_database && trace_progress == 11) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "MATRIXMOVE[11] AFTER 0x%08X 0x%08X\n",
              ypar, signature);
      fail(junk);
   }

This gets  0x00550063 0x08400220, which gets table item at line 2178,
   goes to a qtag with rot=1

---------------------------------------------------------------------------------------
TRACE STATE = 12:
in matrixmove, new formation identified, but people not yet filled in

   if (!expanding_database && trace_progress == 12) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "MATRIXMOVE[12] %d %d 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              result->kind, result->rotation,
              result->people[0].id1 & 0xFF,
              result->people[1].id1 & 0xFF,
              result->people[2].id1 & 0xFF,
              result->people[3].id1 & 0xFF,
              result->people[4].id1 & 0xFF,
              result->people[5].id1 & 0xFF,
              result->people[6].id1 & 0xFF,
              result->people[7].id1 & 0xFF);
      fail(junk);
   }

---------------------------------------------------------------------------------------
TRACE STATE = 13:
in matrixmove, people should be filled in

   if (!expanding_database && trace_progress == 13) {
      trace_progress++;
      char junk[200];
      sprintf(junk, "MATRIXMOVE[13] %d %d 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              result->kind, result->rotation,
              result->people[0].id1 & 0xFF,
              result->people[1].id1 & 0xFF,
              result->people[2].id1 & 0xFF,
              result->people[3].id1 & 0xFF,
              result->people[4].id1 & 0xFF,
              result->people[5].id1 & 0xFF,
              result->people[6].id1 & 0xFF,
              result->people[7].id1 & 0xFF);
      fail(junk);
   }

This gets 23 = qtag, rot = 1
   0x00 0x00 0xCA 0x88 0x00 0x00 0x48 0x0A
   phantoms outside, rwave in center

---------------------------------------------------------------------------------------


Same thing farther back to moves\7851

Never got there.

back to moves\7175
   if (!expanding_database && the_schema != schema_by_array && the_schema != schema_sequential) {
      char junk[200];
      sprintf(junk, "WHAT NON-ARRAY NON_SEQUENTIAL SCHEMA %d\n", the_schema);
      fail(junk);
   }





Never got there.

---------------------------------------------------------------------------------------

From moves\2645:

   if (!expanding_database) {
      char junk[200];
      sprintf(junk, "CTRROT[A] 0x%08X, 0x%08X, 0x%08X\n", result->kind, ypar, signature);
      fail(junk);
   }

---------------------------------------------------------------------------------------

moves\2894
   if (!expanding_database) {
      char junk[200];
      sprintf(junk, "CTRROT[A] %d\n", ss->kind);
      fail(junk);
   }

Never got there.





From basic\4715:

            {
               char junk[200];
               sprintf(junk, "FAIL 0x%08X, 0x%08X/%d/%d.", ss->kind, (int) callspec->schema, 0, 0);
               fail(junk);
            }

moves\2690, before collision_collector CC(allow_collisions;)
   /*
   {
      char junk[200];
      sprintf(junk, "CTRROT 0x%08X, 0x%08X, 0x%08X\n", result->kind, ypar, signature);
      gg77->writestuff(junk);
   }
   */

moves\2690, after
   CC.fix_possible_collision(result, action);
   /*
   {
      char junk[200];
      sprintf(junk, "FIX COLLISION 0x%08X.", result->kind);
      fail(junk);
   }
   */




moves\8592
   catch(error_flag_type foo) {
      /*      if (foo < error_flag_no_retry && this_defn != deferred_array_defn) {
         if (this_defn->compound_part) {
            // Don't take a sequential definition if there are no fractions or parts
            // specified and the call has the special flag.  This is for recycle.
            if (this_defn->compound_part->schema != schema_sequential ||
                !(this_defn->compound_part->callflagsf & CFLAG2_NO_SEQ_IF_NO_FRAC) ||
                !ss->cmd.cmd_fraction.is_null() || ss->cmd.cmd_final_flags.bool_test_heritbits(INHERITFLAG_HALF)) {
               this_defn = this_defn->compound_part;
               goto try_next_callspec;
            }
         }
         else if (deferred_array_defn) {
            this_defn = deferred_array_defn;
            goto try_next_callspec;
         }
         } */

      throw foo;
   }


istort\1667
   if (!expanding_database && ss->kind == s_qtag && ss->cmd.callspec == base_calls[base_base_hinge]) {
      char junk[200];
      sprintf(junk, "RESULT AFTER %d/%d/%d 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
              result->kind, result->rotation, maps->map_kind,
              result->people[0].id1 & 0xFF,
              result->people[1].id1 & 0xFF,
              result->people[2].id1 & 0xFF,
              result->people[3].id1 & 0xFF,
              result->people[4].id1 & 0xFF,
              result->people[5].id1 & 0xFF,
              result->people[6].id1 & 0xFF,
              result->people[7].id1 & 0xFF);
      fail(junk);
   }


6 catches, OK
7 catches, OK
8 catches, OK
9 is active, does not catch, just fails



testing 8
   moving 8 down a bit, it still catches.

   farther, it gets error on
            if (!base_block) fail_no_retry("ZZZZZIllegal concept for this call.");
