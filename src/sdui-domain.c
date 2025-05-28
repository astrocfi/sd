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

    This is for program version 30.

    The version of this file is as shown immediately below.
    The version for the user interface does not track the version
    of the rest of the program. */

#define UI_VERSION_STRING "1.8dialog"

/* This defines the following functions:
   uims_process_command_line
   uims_version_string
   uims_preinitialize
   uims_add_call_to_menu
   uims_finish_call_menu
   uims_postinitialize
   uims_do_outfile_popup
   uims_do_comment_popup
   uims_do_getout_popup
   uims_do_abort_popup
   uims_do_neglect_popup
   uims_do_selector_popup
   uims_do_direction_popup
   uims_get_number_fields
   uims_do_modifier_popup
   uims_reduce_line_count
   uims_add_new_line
   uims_get_command
   uims_begin_search
   uims_begin_reconcile_history
   uims_end_reconcile_history
   uims_update_resolve_menu
   uims_terminate
   uims_database_tick_max
   uims_database_tick
   uims_database_tick_end
   uims_database_error
   uims_bad_argument
*/

/*
   The required behavior of the functions is roughly as follows:

   uims_version_string        Return a static string containing some version info
   uims_preinitialize         First initialization, if any, before call menus are created
   uims_add_call_to_menu      Add one call to current menu under construction
   uims_finish_call_menu      Finish construction of a call menu; there may be more
   uims_postinitialize        Final initialization, if any, after call menus are created
   uims_do_outfile_popup      Put up a popup to let user change the output file
   uims_do_comment_popup      Put up a popup to let user type a comment
   uims_do_getout_popup       Put up a popup to let user type a header line on a sequence
   uims_do_abort_popup        Put up a popup to ask whether user really wants to abort
   uims_get_command           In any mode, query user for action to be taken
   uims_do_neglect_popup      Put up a popup to query for percentage of stale calls to show
   uims_do_selector_popup     Put up a popup to query for a selector (heads/sides..)
   uims_do_direction_popup    Put up a popup to query for a direction (left/right..)
   uims_get_number_fields     Same, for numbers
   uims_do_modifier_popup     Same, for permission to change part of a call
   uims_update_resolve_menu   Change the title line on the special menu for resolve mode
   uims_terminate             Prepare to exit the program

   For details, see the file "sdui.doc".
*/


/* BEWARE!!  The enumeration "selector_kind" in sd.h must be keyed to
      the definition of "task$selector_menu" in sd.dps . */
/* BEWARE!!  The enumeration "call_list_kind" in sd.h must be keyed to
      the oneof appearing in sd.dps for the call menus.  Look there for
      "whole_call_menu". */

/* ********** BEWARE!!!!!  There are constraints in the order of appearance
   of concepts in the concept table in sdtables.c .
   This implementation requires that the phantom, tandem, and distorted concepts
   be exactly as shown in that table, so that they match the enumeration
   in sd.dps in the definition of "task$special_concept_menu".  The concept menu
   processing code only looks at the "general concepts" section of the table. */

/* BEWARE!!  The tables "call_list_menu_tasks" and "call_list_menu_signal_keys" below
   are keyed to the enumeration "call_list_kind". */


#include "/usr/apollo/include/base.h"
#include "/usr/apollo/include/error.h"
#include "/usr/apollo/include/gpr.h"
#include "/usr/apollo/include/dialog.h"
#include <string.h>
#include "sd.h"
#include "sd_dialog.ins.c"


/* Number of characters we can put into a call menu item.
   Any calls appearing in the database with names longer
   than this will be truncated in the menu, though they
   will be correct in the transcript.
   If menu_width changes, must change screen layout in sd.dps,
   in particular, the size of the item "CALL_MENU". */

#define menu_width 43

/* Number of text lines we can display in the working area of the screen,
   including lines used for display of the setup. */
#define MAX_LINES 40

typedef struct h5723c7b92 {
   struct h5723c7b92 *next;
   char text[MAX_TEXT_LINE_LENGTH];
   } line_block;


static line_block *text_head = (line_block *) 0;
static line_block **text_tail = &text_head;
static line_block *free_line_blocks = (line_block *) 0;
static int local_textline_count = 0;
static long_boolean text_changed;

static dp_$string_desc_t *menu_list;         /* Gets allocated somewhere. */


/* BEWARE!!  This table is keyed to the enumeration "call_list_kind". */

static int call_list_menu_tasks[] = {
   whole_call_choices,     /* not used */
   whole_call_choices,     /* not used */
   whole_call_choices,
   call_choices_1x8,
   call_choices_l1x8,
   call_choices_dpt,
   call_choices_cdpt,
   call_choices_rcol,
   call_choices_lcol,
   call_choices_8ch,
   call_choices_tby,
   call_choices_lin,
   call_choices_lout,
   call_choices_rwv,
   call_choices_lwv,
   call_choices_r2fl,
   call_choices_l2fl,
   call_choices_gcol,
   call_choices_qtag};

/* BEWARE!!  This table is keyed to the enumeration "call_list_kind". */

static int call_list_menu_signal_keys[] = {
   whole_call_show,     /* not used */
   whole_call_show,     /* not used */
   whole_call_show,
   call_1x8_show,
   call_l1x8_show,
   call_dpt_show,
   call_cdpt_show,
   call_rcol_show,
   call_lcol_show,
   call_8ch_show,
   call_tby_show,
   call_lin_show,
   call_lout_show,
   call_rwv_show,
   call_lwv_show,
   call_r2fl_show,
   call_l2fl_show,
   call_gcol_show,
   call_qtag_show};

static char search_title_text[80];
static dp_$string_desc_t search_title;
static int visible_modifications = -1;
static dp_$string_desc_t main_title;
static status_$t status;
static dp_$string_desc_t menu_things[200];
static call_list_kind current_call_list;
static mode_kind last_mode;
static long_boolean dialog_started = 0;

/* ******* INTERNAL FUNCTIONS ******* */

static void
status_error_check(char errmsg[])
{
   if (!status.all) return;
   print_line(errmsg);
   error_$print(status);
   print_line("");
   exit_program(1);
}


static void
dialog_signal(int task)
{
   dp_$task_activate(task, &status);
   status_error_check("lossage15: ");
   dp_$task_deactivate(task, &status);
   status_error_check("lossage16: ");
}


static void
dialog_get_menu_item(int task, int *value)
{
   short junk16;

   dp_$enum_get_value(task, &junk16, &status);
   status_error_check("lossage in dialog_get_menu_item: ");
   *value = junk16;
}


static void
dialog_get_string(int task, char *junk, int *count)
{
   short junk16;

   dp_$string_get_value(task, (short) 63, junk, &junk16, &status);
   status_error_check("lossage - dialog_get_string: ");
   *count = junk16;
}


void winning_fill_text(dp_$string_desc_t *q, char k[])
{
   char *p;

   p = q->chars_p+q->cur_len;
   string_copy(&p, k);
   q->cur_len = p - q->chars_p;
}

void dialog_setmsg(int task, dp_$string_desc_t *msg)
{
   dp_$msg_set_value(task, (dp_$string_desc_t *) msg, 1, &status);
   status_error_check("lossage in dialog_setmsg - 1: ");
}


int phantoml_tasks[] = {task$phantoml_concept_menu_1, task$phantoml_concept_menu_2, task$phantoml_concept_menu_3, -1};
int phantomb_tasks[] = {task$phantomb_concept_menu_1, task$phantomb_concept_menu_2, -1};
int tandem_tasks[] = {task$tandem_concept_menu_1, task$tandem_concept_menu_2, task$tandem_concept_menu_3, task$tandem_concept_menu_4, -1};
int distort_tasks[] = {task$distort_concept_menu_1, task$distort_concept_menu_2, task$distort_concept_menu_3, task$distort_concept_menu_4, -1};
int dist4_tasks[] = {task$dist4_concept_menu_1, task$dist4_concept_menu_2, task$dist4_concept_menu_3, task$dist4_concept_menu_4, -1};
int misc_tasks[] = {task$misc_concept_menu_1, task$misc_concept_menu_2, task$misc_concept_menu_3, -1};

/* BEWARE!!  These five tables are keyed to the tables "concept_offset_tables" etc. in sd.h . */

int enablers[] = {
   phantoml_concepts_enabler,
   phantomb_concepts_enabler,
   tandem_concepts_enabler,
   distort_concepts_enabler,
   dist4_concepts_enabler,
   misc_concepts_enabler};

int disablers[] = {
   phantoml_concepts_disabler,
   phantomb_concepts_disabler,
   tandem_concepts_disabler,
   distort_concepts_disabler,
   dist4_concepts_disabler,
   misc_concepts_disabler};

int *tasklists[] = {
   phantoml_tasks,
   phantomb_tasks,
   tandem_tasks,
   distort_tasks,
   dist4_tasks,
   misc_tasks};


static void dialog_read(int *task)
{
   int event;
   
   dp_$event_wait(task, &event, &status);
   status_error_check("dialog_read - 3: ");

   if (event != dp_$task_completed) {
      print_line("anomalous event from dp_$event_wait");
      exit_program(1);
   }
}


/* Activate or deactivate those concepts that are not on the level. */
static void set_concept_activation(void)
{
   int k, column, popup;
   dp_$boolean off_level_value = allowing_all_concepts ? dp_$true : dp_$false;

   for (k=0; k<general_concept_size; k++) {
      dp_$boolean localvalue;

      if (concept_descriptor_table[k+general_concept_offset].level == l_nonexistent_concept) {
         localvalue = dp_$false;
      }
      else if (concept_descriptor_table[k+general_concept_offset].level > calling_level) {
         localvalue = off_level_value;
      }
      else
         localvalue = dp_$true;

      dp_$enum_enable_choice(task$general_concept_menu, (short) (k+1), localvalue, &status);
      status_error_check("lossage - dp_$enum_enable_choice: ");
   }

   for (popup=0; concept_size_tables[popup]; popup++) {
      for (column=0; concept_size_tables[popup][column]>=0; column++) {
         for (k=0; k<concept_size_tables[popup][column]; k++) {
            dp_$boolean localvalue;

            if (concept_descriptor_table[k+concept_offset_tables[popup][column]].level == l_nonexistent_concept) {
               localvalue = dp_$false;
            }
            else if (concept_descriptor_table[k+concept_offset_tables[popup][column]].level > calling_level) {
               localvalue = off_level_value;
            }
            else
               localvalue = dp_$true;

            dp_$enum_enable_choice(tasklists[popup][column], (short) (k+1), localvalue, &status);
            status_error_check("lossage - dp_$enum_enable_choice: ");
         }
      }
   }
}




/* This external variable comes from the output of the dialog translator. */

extern int dp_$sd_dialog_heap;


extern char *uims_version_string(void)
{
   return(UI_VERSION_STRING);
}


extern void uims_process_command_line(int *argcp, char ***argvp)
{
   /* We do nothing with the command line arguments. */
}


/*
The main program calls this before any of the call menus are
created, that is, before any calls to "uims_add_call_to_menu"
and "uims_finish_call_menu".  This performs any first
initialization required by the window management package.

One thing we might want to do here (or do it later, in
"uims_postinitialize") is create the concept menu.
We can do this either as one entity or as a main menu
with popups for tandem/phantom/distorted concepts -- the
rest of the program doesn't care.  We can find the concepts
in the external array "concept_descriptor_table".  We scan
that array until we get to the end marker, indicated by
concept_descriptor_table[i].kind == concept_end_of_list.
The field concept_descriptor_table[i].name has the text
that we should display for the user.  Later, when the
program is running, "uims_get_command" has been called, and the
user clicks on one of these concepts, we must return the
value "ui_concept_select", and set the external
variable "uims_menu_index" to the index of that
concept within the original array "concept_descriptor_table"

A similar thing will be done with call menus, but it is much
more complicated, because there are many menus (one per nonzero
item in the enumeration "call_list_kind"), and the calls in
those menus are presented to us by procedure calls during
initialization, rather than being found in a compile-time
table.  That's what "uims_add_call_to_menu" and
"uims_finish_call_menu" are for.
*/

extern void uims_preinitialize(void)
{
   char *dialog_heap_addr;
   dp_$dpd_id dpd_id;



   menu_list = (dp_$string_desc_t *) get_mem(abs_max_calls * sizeof(dp_$string_desc_t));

   dialog_heap_addr = (char *) &dp_$sd_dialog_heap;
   dp_$init_dpd_from_memory(stream_$stdout, &dialog_heap_addr, dp_$sd_dialog_key, &dpd_id, &status);
   status_error_check("lossage - dp_$init_dpd_from_memory: ");

   dialog_started = 1;

   last_mode = mode_resolve;          /* This makes it force menu selection the first time. */
   search_title.chars_p = search_title_text;
   search_title.max_len = 80;
   search_title.cur_len = 0;         /* Clear the initial title. */
}



/* This is external. */
/* The main program calls this after all the call menus have been created.
   This sets up the concept menu and performs any final initialization of the UIMS. */

extern void uims_postinitialize(void)
{
   short junk16;
   int j, k, column, popup;
   Const char *p;
   dp_$string_desc_t my_text;
   char my_text_text[200];

   /* Initialize the getout message with the null string. */

   my_text.chars_p = my_text_text;
   my_text.max_len = 80;
   my_text.cur_len = 0;
   dp_$string_set_value(getout_header_task, my_text.chars_p, my_text.cur_len, &status);
   status_error_check("lossage - initializing getout title: ");

   /* Create the "special" concept menu.  This just lists the popups. */

   for (k=0; concept_menu_strings[k]; k++) {
      p = concept_menu_strings[k];
      j = 0;
      while (p[j]) j++;
      menu_things[k].max_len = j;
      menu_things[k].cur_len = j;
      menu_things[k].chars_p = (char *) p;   /* Sorry, we have to cast to non-constant chars. */
   }

   dp_$enum_set_choices(task$special_concept_menu, 1, (short) k, menu_things, dp_$true, &status);
   status_error_check("lossage - dp_$enum_set_choices: ");

   /* Create the "general" concept menu. */
   
   for (k=0; k<general_concept_size; k++) {
      p = concept_descriptor_table[k+general_concept_offset].name;
      j = 0;
      while (p[j]) j++;
      menu_things[k].max_len = j;
      menu_things[k].cur_len = j;
      menu_things[k].chars_p = (char *) p;   /* Sorry, we have to cast to non-constant chars. */
   }

   dp_$enum_set_choices(task$general_concept_menu, 1, (short) k, menu_things, dp_$true, &status);
   status_error_check("lossage - dp_$enum_set_choices: ");

   /* Create the special concept popup menus. */

   for (popup=0; concept_size_tables[popup]; popup++) {
      for (column=0; concept_size_tables[popup][column]>=0; column++) {
         for (k=0; k<concept_size_tables[popup][column]; k++) {
            p = concept_descriptor_table[k+concept_offset_tables[popup][column]].name;
            j = 0;
            while (p[j]) j++;
            menu_things[k].max_len = j;
            menu_things[k].cur_len = j;
            menu_things[k].chars_p = (char *) p;   /* Sorry, we have to cast to non-constant chars. */
         }
      
         dp_$enum_set_choices(tasklists[popup][column], 1, (short) k, menu_things, dp_$true, &status);
         status_error_check("lossage - dp_$enum_set_choices: ");
      }
   }

   set_concept_activation();

   /* Now activate the windows. */

   dp_$task_activate(dp_$all_task_group, &status);
   status_error_check("lossage13: ");
   gpr_$acquire_display(&status);
   status_error_check("lossage14: ");
}



extern int uims_do_outfile_popup(char dest[])
{
   int count;
   int my_task;

   dialog_signal(outfile_enabler);
   dialog_read(&my_task);        /* It sometimes pops itself down. */
   if (my_task == outfile_task) {
      dialog_signal(outfile_disabler);       /* But not in this case. */
      dialog_get_string(outfile_task, dest, &count);

      if (count) {
         dest[count] = '\0';
         return(POPUP_ACCEPT_WITH_STRING);
      }
   }
   return(POPUP_DECLINE);
}



extern int uims_do_comment_popup(char dest[])
{
   int count;
   int my_task;

   dialog_signal(comment_enabler);
   dialog_read(&my_task);        /* It sometimes pops itself down. */
   if (my_task == comment_task) {
      dialog_signal(comment_disabler);       /* But not in this case. */
      dialog_get_string(comment_task, dest, &count);

      if (count) {
         dest[count] = '\0';
         return(POPUP_ACCEPT_WITH_STRING);
      }
   }
   return(POPUP_DECLINE);
}



extern int uims_do_getout_popup(char dest[])
{
   int count;
   int my_task;

   dialog_signal(getout_enabler);   /* pop it up */
   dialog_read(&my_task);
   dialog_signal(getout_disabler);  /* pop it down */

   if (my_task == getout_abort_task) return(POPUP_DECLINE);

   if (my_task == getout_header_task || my_task == same_getout_header_task) {
      dialog_get_string(getout_header_task, dest, &count);
      if (count) {
         dest[count] = '\0';
         return(POPUP_ACCEPT_WITH_STRING);
      }
   }

   return(POPUP_ACCEPT);
}



extern int uims_do_abort_popup(void)
{
   int my_task;

   dialog_signal(abort_confirm_enabler);    /* It pops itself down. */
   dialog_read(&my_task);
   if (my_task == abort_confirm_select_task) return(POPUP_ACCEPT);
   else return(POPUP_DECLINE);
}




/*
"Cl" designates the menu to which this call is to be added,
such as "call_list_any" or "call_list_dpt".  The array item
"number_of_calls[cl]" tells how big that menu will be when
it is finished.  Put one call into that menu.  "Index" lies
between 0 and "number_of_calls[cl]", and will in fact iterate
over those values in sequence as this procedure is repeatedly
invoked to create the menu.

Later, when the program is running, "uims_get_command" has been called,
and the user clicks on a call while this menu is selected, we must
return the value "ui_call_select", and set the external variable
"uims_menu_index" to the index of that call as it was given
to this procedure.

If our window management system wants us to pass a pointer to
the string when a menu is created, and it will use that string
forever more, without copying it, so that we must guarantee
that the string will remain at the location we originally
presented, some care is required.  If "ephemeral" is off, there
is no problem -- the string will stay around.  If "ephemeral"
is on, we must copy the string into some freshly allocated
memory.
*/

extern void uims_add_call_to_menu(call_list_kind cl, int call_menu_index, char name[])
{
   int j;

   menu_list[call_menu_index].chars_p = name;
   menu_list[call_menu_index].cur_len = strlen(name);
   if (menu_width < menu_list[call_menu_index].cur_len) menu_list[call_menu_index].cur_len = menu_width;
   menu_list[call_menu_index].max_len = menu_list[call_menu_index].cur_len;
}


/*
Communicate with the window system, creating a menu
containing "number_of_calls[cl]" items, which are the items whose
text strings were previously transmitted by the calls to
"uims_add_call_to_menu".

This will be called once for each value in the enumeration
"call_list_kind".
You may want to use a static variable to communicate the text
information from "uims_add_call_to_menu" to here.
*/


extern void uims_finish_call_menu(call_list_kind cl, char menu_name[])
{
   short ncjunk = number_of_calls[cl];

   dp_$enum_set_choices(call_list_menu_tasks[cl], 1, ncjunk, menu_list, true, &status);
   status_error_check("create_menu - 1: ");
}



/* BEWARE!!  The numbers deposited into "uims_menu_index" when this is invoked in
   startup mode must correspond to the data in the array "startinfolist"
   in sdtables.c . */

extern uims_reply uims_get_command(mode_kind mode, call_list_kind *call_menu)
{
   int my_task, index;

   /* Check for mode changes. */

   check_menu:

   switch (mode) {
      case mode_normal:
         if (last_mode == mode_startup)
            dialog_signal(startup_disabler);         /* Coming out of startup mode. */
   
         if (allowing_modifications)
            *call_menu = call_list_any;

         if (last_mode != mode_normal || current_call_list != *call_menu) {
            /* Entering normal from startup/resolve, or call menu has changed -- must put up correct menu. */
            dialog_signal(call_list_menu_signal_keys[*call_menu]);
            current_call_list = *call_menu;
         }

         break;
      case mode_resolve:
         if (last_mode != mode_resolve)
            /* Entering resolve mode.  Place the search submenu over the call menus. */
            dialog_signal(search_enabler);

         current_call_list = call_list_any;   /* Make it harmless. */
         break;
      case mode_startup:
         if (last_mode != mode_startup)
            dialog_signal(startup_enabler);          /* Entering startup mode. */

         current_call_list = call_list_any;   /* Make it harmless. */
         break;
   }

   last_mode = mode;

   /* See if the text area needs to be updated. */

   if (text_changed) {
      dp_$string_desc_t dialog_textblocks[MAX_LINES];
      int i;
      line_block *current = text_head;

      /* Skip over the early lines if we have too many. */

      for (i=MAX_LINES; i<local_textline_count; i++)
         current = current->next;

      /* Now the "current" pointer points to at most MAX_LINES. */

      for (i=0; i<MAX_LINES; i++) {
         if (current) {
            dialog_textblocks[i].chars_p = current->text;
            dialog_textblocks[i].cur_len = strlen(current->text);
            current = current->next;
         }
         else {
            dialog_textblocks[i].chars_p = "";
            dialog_textblocks[i].cur_len = 0;
         }
         dialog_textblocks[i].max_len = MAX_TEXT_LINE_LENGTH;
      }

      dp_$msg_set_value(display, dialog_textblocks, MAX_LINES, &status);
      status_error_check("lossage in dialog_setmsg - 2: ");
      text_changed = FALSE;
   }

   /* See if the modifications banner needs to be updated. */

   if (visible_modifications != allowing_modifications) {
      if (allowing_modifications == 2) {
         main_title.chars_p = "[all modifications enabled]   M1 -> select ; M3 -> undo";
         main_title.cur_len = sizeof("[all modifications enabled]   M1 -> select ; M3 -> undo")-1;
      }
      else if (allowing_modifications) {
         main_title.chars_p = "[simple modifications enabled]   M1 -> select ; M3 -> undo";
         main_title.cur_len = sizeof("[simple modifications enabled]   M1 -> select ; M3 -> undo")-1;
      }
      else {
         main_title.chars_p = "M1 -> select ; M3 -> undo";
         main_title.cur_len = sizeof("M1 -> select ; M3 -> undo")-1;
      }
   
      main_title.max_len = 80;
      dialog_setmsg(main_title_task, &main_title);
      visible_modifications = allowing_modifications;
   }

   getcmd:
   dialog_read(&my_task);

   switch (my_task) {
      case task$undo:
         uims_menu_index = command_undo;
         return(ui_command_select);
      case task$abort:
         uims_menu_index = command_abort;
         return(ui_command_select);
      case task$quit:
         uims_menu_index = command_quit;
         return(ui_command_select);
      case allow_modification_task:
         /* Increment "allowing_modifications" up to a maximum of 2. */
         /* Actually, we just set it to 2.  Having two grades of modifiability
            is very unwieldy. */
         if (allowing_modifications != 2) allowing_modifications = 2;
         goto check_menu;
      case allow_all_concept_task:
         allowing_all_concepts = !allowing_all_concepts;
         set_concept_activation();
         goto check_menu;
#ifdef NEGLECT
      case neglect_task:
         uims_menu_index = command_neglect;
         return(ui_command_select);
#endif
      case save_pic_task:
         uims_menu_index = command_save_pic;
         return(ui_command_select);
      case task$create_outfile:
         uims_menu_index = command_change_outfile;
         return(ui_command_select);
      case task$create_comment:
         uims_menu_index = command_create_comment;
         return(ui_command_select);
      case getout_task:
         uims_menu_index = command_getout;
         return(ui_command_select);
      case start_choices:
         dialog_get_menu_item(start_choices, &index);
         uims_menu_index = index;
         return(ui_start_select);
      case task$general_concept_menu:
         dialog_get_menu_item(task$general_concept_menu, &index);
         uims_menu_index = index+general_concept_offset-1;
         return(ui_concept_select);
      case task$special_concept_menu:
         dialog_get_menu_item(task$special_concept_menu, &index);

         {
            int column, task, funnynum;
            int *task_table;

            dialog_signal(enablers[index-1]);
            dialog_read(&task);
            dialog_signal(disablers[index-1]);
         
            task_table = tasklists[index-1];
         
            for (column = 0; task_table[column]>=0; column++) {
               if (task == task_table[column]) {
                  dialog_get_menu_item(task, &funnynum);
                  goto got_concept;
               }
            }
         
            goto getcmd;  /* Must have moved the mouse away or clicked in the border. */

            got_concept:
   
            uims_menu_index = funnynum-1+concept_offset_tables[index-1][column];
            /* Check for non-existent menu entry. */
            if (concept_descriptor_table[uims_menu_index].kind == concept_comment) goto getcmd;
         }

         return(ui_concept_select);
      case task$search_resolve:
         uims_menu_index = command_resolve;
         return(ui_command_select);
      case task$search_reconcile:
         uims_menu_index = command_reconcile;
         return(ui_command_select);
      case task$search_anything:
         uims_menu_index = command_anything;
         return(ui_command_select);
      case task$search_nice_setup:
         uims_menu_index = command_nice_setup;
         return(ui_command_select);
      case task$search$abort:
         uims_menu_index = (int) resolve_command_abort;
         return(ui_resolve_select);
      case task$search$another:
         uims_menu_index = (int) resolve_command_find_another;
         return(ui_resolve_select);
      case task$search$next:
         uims_menu_index = (int) resolve_command_goto_next;
         return(ui_resolve_select);
      case task$search$previous:
         uims_menu_index = (int) resolve_command_goto_previous;
         return(ui_resolve_select);
      case task$search$accept:
         uims_menu_index = (int) resolve_command_accept;
         return(ui_resolve_select);
      case task$search$incdepth:
         uims_menu_index = (int) resolve_command_raise_rec_point;
         return(ui_resolve_select);
      case task$search$decdepth:
         uims_menu_index = (int) resolve_command_lower_rec_point;
         return(ui_resolve_select);
      /* These 3 should never happen, but dialog has been observed to return them if a modification popup
         was abandoned because the debugger confused the program.  Apparently, if the window is obscured,
         the debugger can walk right through the call to uims_do_modifier_popup without getting input
         from the operator, and returning "declined" status.  Then, when it comes time to ask for the
         next call, it waits for input (no walking through this time), leaving the popup on the screen.
         If we move the mouse into that popup and out again, we get "popup_abort_task" here, even though
         we are supposedly querying for a call.  Yuk. */
      case abort_confirm_select_task:
      case abort_confirm_abort_task:
      case popup_abort_task:
         goto getcmd;
   }

   if (my_task == call_list_menu_tasks[current_call_list]) {
      /* User clicked on a call. */
      dialog_get_menu_item(my_task, &index);
      uims_menu_index = index-1;
      return(ui_call_select);
   }

   uims_menu_index = command_abort;
   return(ui_command_select);
}




extern void uims_reduce_line_count(int n)
{
   line_block *current = text_head;
   line_block **last = &text_head;

   if (local_textline_count < n) return;   /* Should never happen, being defensive here. */

   local_textline_count = n;

   /* Skip ahead to where we want to chop it off. */

   while (n > 0) {
      last = &current->next;
      current = current->next;
      n--;
   }

   text_tail = last;
   *last = (line_block *) 0;

   while (current) {
      line_block *item = current;

      current = item->next;
      item->next = free_line_blocks;
      free_line_blocks = item;
   }
}


extern void uims_add_new_line(char the_line[])
{
   int i;
   line_block *item;

   text_changed = TRUE;
   local_textline_count++;

   if (free_line_blocks) {
      item = free_line_blocks;
      free_line_blocks = item->next;
   }
   else {
      item = (line_block *) get_mem(sizeof(line_block));
   }

   strcpy(item->text, the_line);
   item->next = (line_block *) 0;
   *text_tail = item;
   text_tail = &item->next;
}

static int first_reconcile_history;
static search_kind reconcile_goal;

/*
 * UIMS_BEGIN_SEARCH is called at the beginning of each search mode
 * command (resolve, reconcile, nice setup, do anything).
 */

extern void uims_begin_search(search_kind goal)
{
    reconcile_goal = goal;
    first_reconcile_history = TRUE;
}

/*
 * UIMS_BEGIN_RECONCILE_HISTORY is called at the beginning of a reconcile,
 * after UIMS_BEGIN_SEARCH,
 * and whenever the current reconcile point changes.  CURRENTPOINT is the
 * current reconcile point and MAXPOINT is the maximum possible reconcile
 * point.  This call is followed by calls to UIMS_REDUCE_LINE_COUNT
 * and UIMS_ADD_NEW_LINE that
 * display the current sequence with the reconcile point indicated.  These
 * calls are followed by a call to UIMS_END_RECONCILE_HISTORY.
 * Return TRUE to cause sd to forget its cached history output; do this
 * if the reconcile history is written to a separate window.
 */

extern int
uims_begin_reconcile_history(int currentpoint, int maxpoint)
{
    if (!first_reconcile_history)
        uims_update_resolve_menu(reconcile_goal, 0, 0, resolver_display_ok);
    return FALSE;
}

/*
 * Return TRUE to cause sd to forget its cached history output.
 */

extern int
uims_end_reconcile_history(void)
{
    first_reconcile_history = FALSE;
    return FALSE;
}

extern void uims_update_resolve_menu(search_kind goal, int cur, int max, resolver_display_state state)
{
   char title[MAX_TEXT_LINE_LENGTH];

   create_resolve_menu_title(goal, cur, max, state, title);
   search_title.cur_len = 0;
   winning_fill_text(&search_title, title);
   dialog_setmsg(search_title_task, &search_title);

   if (last_mode != mode_resolve) {
      /* Entering resolve mode.  Place the search submenu over the call menus. */
      dialog_signal(search_enabler);
   }

   last_mode = mode_resolve;
}


#ifdef NEGLECT
extern int uims_do_neglect_popup(char dest[])
{
   char my_text_text[1];
   int count;
   int my_task;

   dialog_signal(neglect_popup_enabler);
   dp_$string_set_value(neglect_popup_task, my_text_text, 0, &status);        /* Clear the existing stuff. */
   status_error_check("lossage - uims_do_neglect_popup: ");

   dialog_read(&my_task);        /* It sometimes pops itself down. */
   if (my_task == neglect_popup_task) {
      dialog_signal(neglect_popup_disabler);       /* But not in this case. */
      dialog_get_string(neglect_popup_task, dest, &count);

      if (count) {
         dest[count] = '\0';
         return(POPUP_ACCEPT_WITH_STRING);
      }
   }
   return(POPUP_DECLINE);
}
#endif


extern int uims_do_selector_popup(void)
{
   int task;
   int num;

   dialog_signal(selector_enabler);
   dialog_read(&task);
   dialog_signal(selector_disabler);

   if (task == task$selector_menu) {
      dialog_get_menu_item(task$selector_menu, &num);
      return(num);
   }
   else
      return(0);
}


extern int uims_do_direction_popup(void)
{
   int task;
   int num;

   dialog_signal(direction_enabler);
   dialog_read(&task);
   dialog_signal(direction_disabler);

   if (task == task$direction_menu) {
      dialog_get_menu_item(task$direction_menu, &num);
      return(num);
   }
   else
      return(0);
}



extern uint32 uims_get_number_fields(int nnumbers)
{
   int i;
   unsigned int number_list = 0;

   dialog_signal(quantifier_enabler);

   for (i=0 ; i<nnumbers ; i++) {
      int task;
   
      dialog_read(&task);
   
      if (task == task$quantifier_menu) {
         unsigned int num;
         dialog_get_menu_item(task$quantifier_menu, (int *) &num);
         number_list |= (num << (i*4));
      }
      else {
         dialog_signal(quantifier_disabler);
         return 0;    /* User waved the mouse away. */
      }
   }

   dialog_signal(quantifier_disabler);
   return number_list;
}


extern int uims_do_modifier_popup(char callname[], modify_popup_kind kind)
{
   int task;
   char tempstring_text[80];
   dp_$string_desc_t tempstring[2];
   
   tempstring[0].chars_p = tempstring_text;
   tempstring[0].max_len = 80;
   tempstring[0].cur_len = 0;
   winning_fill_text(&tempstring[0], "The \"");
   winning_fill_text(&tempstring[0], callname);

   switch (kind) {
      case modify_popup_any:
         winning_fill_text(&tempstring[0], "\" can be replaced.");
         dp_$msg_set_value(modifier_title_task, tempstring, 1, &status);
         break;
      case modify_popup_only_tag:
         winning_fill_text(&tempstring[0], "\" can be replaced");
         tempstring[1].chars_p = "with a tagging call.";
         tempstring[1].max_len = 20;
         tempstring[1].cur_len = 20;
         dp_$msg_set_value(modifier_title_task, tempstring, 2, &status);
         break;
      case modify_popup_only_scoot:
         winning_fill_text(&tempstring[0], "\" can be replaced");
         tempstring[1].chars_p = "with scoot/tag (chain thru) (and scatter).";
         tempstring[1].max_len = 42;
         tempstring[1].cur_len = 42;
         dp_$msg_set_value(modifier_title_task, tempstring, 2, &status);
         break;
   }

   status_error_check("lossage in dialog_setmsg - 1: ");

   dialog_signal(modifier_enabler);
   dialog_read(&task);         /* It pops itself down. */
   
   if (task == modifier_select_task) return(1); else return(0);
}


extern void uims_terminate(void)
{
   if (dialog_started) {
      status_$t st;

      gpr_$release_display(&st);
      dp_$terminate(&st);
  }
}

/*
 * The following two functions allow the UI to put up a progress
 * indicator while the call database is being read (and processed).
 *
 * uims_database_tick_max is called before reading the database
 * with the number of ticks that will be sent.
 * uims_database_tick is called repeatedly with the number of new
 * ticks to add.
 */

extern void
uims_database_tick_max(int n)
{
    /* not implemented yet */
}

extern void
uims_database_tick(int n)
{
    /* not implemented yet */
}

extern void
uims_database_tick_end(void)
{
    /* not implemented yet */
}

extern void
uims_database_error(char *message, char *call_name)
{
   print_line(message);
   if (call_name) {
      print_line("  While reading this call from the database:");
      print_line(call_name);
   }
}

extern void
uims_bad_argument(char *s1, char *s2, char *s3)
{
   if (s1) print_line(s1);
   if (s2) print_line(s2);
   if (s3) print_line(s3);
   print_line("Use the -help flag for help.");
   exit_program(1);
}
