static const char time_stamp[] = "sdui-x11.c Time-stamp: <1997-10-14 17:51:42 gildea>";
/* 
 * sdui-x11.c - Sd User Interface for X11
 * Copyright 1990,1991,1992,1993 Stephen Gildea and William B. Ackerman
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose is hereby granted without fee, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * The authors make no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * By Stephen Gildea, March 1990.
 * Requires the Athena Widget Set from X11 Release 4 or later.
 *
 * For use with version 31 of the Sd program.
 *
 * The version of this file is as shown immediately below.  This string
 * gets displayed at program startup, as the "ui" part of the complete
 * version.
 */

#define UI_VERSION_STRING "1.17"
#define UI_TIME_STAMP "gildea@lcs.mit.edu  24 Mar 95 $"

/* This file defines the following functions:
   uims_version_string
   uims_process_command_line
   uims_display_help
   uims_display_ui_intro_text
   uims_preinitialize
   uims_create_menu
   uims_postinitialize
   uims_set_window_title
   uims_get_startup_command
   uims_get_call_command
   uims_get_resolve_command
   uims_do_comment_popup
   uims_do_outfile_popup
   uims_do_header_popup
   uims_do_getout_popup
   uims_do_write_anyway_popup
   uims_do_delete_clipboard_popup
   uims_do_abort_popup
   uims_do_session_init_popup
   uims_do_neglect_popup
   uims_do_selector_popup
   uims_do_direction_popup
   uims_do_circcer_popup
   uims_do_tagger_popup
   uims_get_number_fields
   uims_do_modifier_popup
   uims_add_new_line
   uims_reduce_line_count
   uims_update_resolve_menu
   uims_terminate
   uims_database_tick_max
   uims_database_tick
   uims_database_tick_end
   uims_database_error
   uims_bad_argument
*/

#include "sd.h"
#include "paths.h"
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Viewport.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/List.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/AsciiText.h>

/* See comments in sdmain.c regarding this string. */
static Const char id[] = "@(#)$He" "ader: Sd: sdui-x11.c " UI_VERSION_STRING "  " UI_TIME_STAMP;

static Widget toplevel, cmdmenu, conceptspecialmenu;
static Widget conceptpopup, conceptlist;
static Widget lview, callview, conceptmenu, callmenu;
static Widget callbox, calltitle;
static Widget statuswin, txtwin;
static Widget resolvewin, resolvetitle, resolvemenu;
static Widget confirmpopup, confirmlabel;
static Widget choosepopup, choosebox, chooselabel, chooselist;
static Widget commentpopup, commentbox, outfilepopup, outfilebox, titlepopup, titlebox;
static Widget getoutpopup, getoutbox;
#ifdef NEGLECT
static Widget neglectpopup, neglectbox;
#endif

/* This is the order in which command buttons will appear on the screen.
    These will be translated into the things required by the main program by
    the translation table "button_translations".
   BEWARE!!  This must track the tables "button_translations" and "command_resources". */
typedef enum {
   cmd_button_quit,
   cmd_button_undo,
   cmd_button_abort,
   cmd_button_simple_mods,
   cmd_button_allow_mods,
   cmd_button_allow_concepts,
   cmd_button_active_phantoms,
#ifdef OLD_ELIDE_BLANKS_JUNK
   cmd_button_ignoreblanks,
#endif
   cmd_button_retain_after_error,
   cmd_button_nowarn_mode,
   cmd_button_create_comment,
   cmd_button_change_outfile,
   cmd_button_change_title,
   cmd_button_getout,
   cmd_button_resolve,
   cmd_button_reconcile,
   cmd_button_anything,
   cmd_button_nice_setup,
#ifdef NEGLECT
   cmd_button_neglect,
#endif
   cmd_button_save_pic
} cmd_button_kind;
#define NUM_CMD_BUTTON_KINDS (((int) cmd_button_save_pic)+1)

/*
 * The total version string looks something like
 * "27.4:db27.5:ui1.6X11"
 * We return the "1.6X11" part.
 */

static char version_mem[12];

extern char *uims_version_string(void)
{
    strcpy(version_mem, UI_VERSION_STRING);
    strcat(version_mem, "X11");
    return version_mem;
}


static long_boolean ui_task_in_progress = FALSE;

/*
 * Since callbacks and actions can't return values, narrow the use of
 * global variables with these two routines.
 */

static int callback_value_storage; /* used only by two routines below */

static void
callback_return(int return_value)
{
    ui_task_in_progress = FALSE;
    callback_value_storage = return_value;
}

/* Serial number ensures each event generates at most one action */
static unsigned long action_event_serial;

typedef enum {
    inside_nothing,
    inside_popup,
    inside_get_command
} ui_context;

/* Ensures commands aren't selected while a popup is up. */
static ui_context inside_what = inside_nothing;

/*
 * Modification of XtAppMainLoop that returns to the caller
 * when the requested action has been done.  This technique
 * allows the main program to call the user interface as a
 * subroutine, whereas XtAppMainLoop assumes the other way around.
 */
static int
read_user_gesture(XtAppContext app)
{
    XEvent event;

    ui_task_in_progress = TRUE;

    /* allow only new user events to generate actions */
    action_event_serial = NextRequest(XtDisplay(toplevel));
    /*
     * ui_task_in_progress is turned off asynchronously by event
     * callbacks or actions when the program request is satisfied.
     */
    while (ui_task_in_progress) {
    	XtAppNextEvent(app, &event);
	XtDispatchEvent(&event);
    }
    inside_what = inside_nothing;
    return callback_value_storage;
}

/*
 * Remove highlighting from any list that uims_get_command might use.
 * Not necessary any more with mouse-tracking highlighting,
 * but is it desirable?  Perhaps it provides more feedback.
 */
static void
unhighlight_lists(void)
{
    XawListUnhighlight(cmdmenu);
    XawListUnhighlight(conceptspecialmenu);
    XawListUnhighlight(conceptmenu);
    XawListUnhighlight(callmenu);
    XawListUnhighlight(resolvemenu);
}

/*
 * This is the callback procedure for the menus that
 * uims_get_command uses, that is, not the popups.
 * Sets the external variable uims_menu_index.
 */

static XtAppContext xtcontext;

static void
position_near_mouse(Widget popupshell)
{
    Window root, child;
    int x, y, max_x, max_y;
    int winx, winy;
    Dimension width, height, border;
    unsigned int mask;
    int gravity;

    /* some of this is copied from CenterWidgetOnPoint in Xaw/TextPop.c,
       which should be a publicly-callable function anyway. */

    XtVaGetValues(popupshell,
		  XtNwidth, &width,
		  XtNheight, &height,
		  XtNborderWidth, &border,
		  NULL);

    /* be sure popup is realized before calling XtWindow on it */
    XtRealizeWidget(popupshell);

    XQueryPointer(XtDisplay(popupshell), XtWindow(popupshell),
		  &root, &child, &x, &y, &winx, &winy, &mask);

    width += 2 * border;
    height += 2 * border;

    max_x = WidthOfScreen(XtScreen(popupshell));
    max_y = HeightOfScreen(XtScreen(popupshell));

    /* set gravity hint based on position on screen */
    gravity = 1;
    if (x > max_x/3) gravity += 1;
    if (x > max_x*2/3) gravity += 1;
    if (y > max_y/3) gravity += 3;
    if (y > max_y*2/3) gravity += 3;

    max_x -= width;
    max_y -= height;

    x -= ( (Position) width/2 );
    if (x < 0) x = 0;
    if (x > max_x) x = max_x;

    y -= ( (Position) height/2 );
    if (y < 0) y = 0;
    if ( y > max_y ) y = max_y;

    XtVaSetValues(popupshell,
		  XtNx, (Position)x,
		  XtNy, (Position)y,
		  XtNwinGravity, gravity,
		  NULL);
}


static int
do_popup(Widget popup_shell)
{
    int value;
    Display *display = XtDisplay(popup_shell);
    XEvent event;

    position_near_mouse(popup_shell);
    XtPopup(popup_shell, XtGrabNone);
    inside_what = inside_popup;
    value = read_user_gesture(xtcontext);
    XtPopdown(popup_shell);
    /*
     * Make sure we know the popup is popped down,
     * so we are free to resize it for the next popup.
     * (This would be a problem when there are two popups in a row.)
     */
    do {
	XMaskEvent(display, StructureNotifyMask, &event);
	XtDispatchEvent(&event);
    } while (event.type != ReparentNotify);
    return value;
}    

static String empty_string = "";
static Cstring *concept_popup_list = NULL;

#define USER_GESTURE_NULL -1
#define SPECIAL_SIMPLE_MODS -2
#define SPECIAL_ALLOW_MODS -3
#define SPECIAL_TOGGLE_ALL_CONCEPTS -4
#define SPECIAL_TOGGLE_ACT_PHANTOMS -5
#define SPECIAL_TOGGLE_IGNOREBLANKS -6
#define SPECIAL_TOGGLE_RETAIN_AFTER_ERROR -7
#define SPECIAL_TOGGLE_NOWARN_MODE -8


/* Beware:  This table must track the enumeration "cmd_button_kind". */
static int button_translations[] = {
   command_quit,                          /* cmd_button_quit */
   command_undo,                          /* cmd_button_undo */
   command_abort,                         /* cmd_button_abort */
   SPECIAL_SIMPLE_MODS,                   /* cmd_button_simple_mods */
   SPECIAL_ALLOW_MODS,                    /* cmd_button_allow_mods */
   SPECIAL_TOGGLE_ALL_CONCEPTS,           /* cmd_button_allow_concepts */
   SPECIAL_TOGGLE_ACT_PHANTOMS,           /* cmd_button_active_phantoms */
#ifdef OLD_ELIDE_BLANKS_JUNK
   SPECIAL_TOGGLE_IGNOREBLANKS,           /* cmd_button_ignoreblanks */
#endif
   SPECIAL_TOGGLE_RETAIN_AFTER_ERROR,     /* cmd_button_retain_after_error */
   SPECIAL_TOGGLE_NOWARN_MODE,            /* cmd_button_nowarn_mode */
   command_create_comment,                /* cmd_button_create_comment */
   command_change_outfile,                /* cmd_button_change_outfile */
   command_change_header,                 /* cmd_button_change_title */
   command_getout,                        /* cmd_button_getout */
   command_resolve,                       /* cmd_button_resolve */
   command_reconcile,                     /* cmd_button_reconcile */
   command_random_call,                   /* cmd_button_anything */
   command_normalize,                     /* cmd_button_nice_setup */
#ifdef NEGLECT
   command_neglect,                       /* cmd_button_neglect */
#endif
   command_save_pic                       /* cmd_button_save_pic */
};


/* ARGSUSED */
static void
command_or_menu_chosen(Widget w, XtPointer client_data, XtPointer call_data)
{
    XawListReturnStruct *item = (XawListReturnStruct *) call_data;

    unhighlight_lists();        /* do here in case spurious event */
    if (inside_what == inside_get_command) {
        uims_reply local_reply = (uims_reply) client_data;
 
	uims_menu_index = item->list_index; /* extern var <- menu item no. */

        if (local_reply == ui_command_select) {
            /* Translate into the form the main program wants, except for a few
                negative values that we will intercept later. */
            uims_menu_index = button_translations[uims_menu_index];
        }
        else if (local_reply == ui_concept_select) {
            /* Fudge the number to be relative to the entire concept list. */
            uims_menu_index += general_concept_offset;
        }
        else if (local_reply == ui_special_concept) {
            int column, index, menu;
            unsigned int i;
            unsigned int row, col;
            unsigned int maxrow, maxcolumn, entries;
            int value;
      
            menu = uims_menu_index;

            /* determine menu size */
            for (maxcolumn=0; concept_size_tables[menu][maxcolumn]>=0; maxcolumn++)
                ;
            maxrow = 0;
            for (i=0; i<maxcolumn; i++)
                if (maxrow < concept_size_tables[menu][i])
                    maxrow = concept_size_tables[menu][i];
            entries = maxcolumn*maxrow;
        
            concept_popup_list =
                get_more_mem(concept_popup_list, entries*sizeof(String *));
        
            /* fill in the entries */
            i=0;
            for (row=0; row<maxrow; row++) {
                for (col=0; col<maxcolumn; col++) {
                    if (row < concept_size_tables[menu][col])
                        concept_popup_list[i] = concept_descriptor_table
                            [ concept_offset_tables[menu][col]+row ].menu_name;
                    else
                        concept_popup_list[i] = empty_string;
                    i++;
                }
            }
        
            XawListChange(conceptlist, (char **) concept_popup_list, entries, 0, TRUE);
            XtVaSetValues(conceptlist, XtNdefaultColumns, maxcolumn, NULL);
            value = do_popup(conceptpopup);
        
            if (value == 0)
                goto try_again;   /* User moved mouse away. */

            value--;
            /* row and column are 0-based */
            row = value/maxcolumn;
            column = value%maxcolumn;
            if (row >= concept_size_tables[menu][column])
                goto try_again;    /* Off end of menu; can't be a legal mouse click. */

            index = row + concept_offset_tables[menu][column];
            uims_menu_index = index;
            /* Check for non-existent menu entry. */
            if (concept_descriptor_table[uims_menu_index].kind == concept_comment) goto try_again;
            local_reply = ui_concept_select;
        }

        callback_return(local_reply); /* return which menu */
        return;
        try_again:
        callback_return(USER_GESTURE_NULL);
    }
}

/* undo action timeout proc.  Gets called after the UNDO entry
   has flashed for an appropriate amount of time. */
/* ARGSUSED */
static void 
cmdmenu_unhighlight(XtPointer client_data, XtIntervalId *intrvl)
{
    XawListUnhighlight(cmdmenu);
}

/* Simulates a click on the specified command menu item.
   Compare with command_or_menu_chosen above. */

static void
cmdmenu_action_internal(int command)
{
    unhighlight_lists();
    /* provide visual feedback */
    XawListHighlight(cmdmenu, command);
    XtAppAddTimeOut(XtWidgetToApplicationContext(cmdmenu), 200,
		    cmdmenu_unhighlight, NULL);
    uims_menu_index = command; /* extern var <- menu item no. */
    callback_return(ui_command_select); /* return which menu */
}

static Atom wm_delete_window;
static Atom wm_protocols;

/* normally invoked only in response to a WM request */

/* ARGSUSED */
static void
quit_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->type == ClientMessage  &&  event->xclient.message_type == wm_protocols)
	if (event->xclient.format != 32  ||  event->xclient.data.l[0] != wm_delete_window)
	    return;		/* some other WM_PROTOCOL */

    cmdmenu_action_internal(command_quit);
}

static mode_kind visible_mode = mode_none;

/* Special action defined because it is handy to bind to MB3. */

/* ARGSUSED */
static void
undo_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (inside_what == inside_get_command) {
	if (visible_mode != mode_normal)
	    XBell(XtDisplay(w), 0);
	else {
	    cmdmenu_action_internal(command_undo);
	}
    }
}


/* popup actions */

/* ARGSUSED */
static void
popup_yes_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->xany.serial >= action_event_serial) {
	callback_return(POPUP_ACCEPT);
    }
}

/* ARGSUSED */
static void
popup_no_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->xany.serial >= action_event_serial) {
	callback_return(POPUP_DECLINE);
    }
}

/* ARGSUSED */
static void
accept_string_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    callback_return(POPUP_ACCEPT_WITH_STRING);
}


/* popup callbacks */

/* ARGSUSED */
static void
choose_pick(Widget w, XtPointer client_data, XtPointer call_data)
{
    XawListReturnStruct *item = (XawListReturnStruct *) call_data;

    if (inside_what == inside_popup) {
	callback_return(item->list_index + 1); /* can't return 0 */
    }
}

/* ARGSUSED */
static void
dialog_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (inside_what == inside_popup) {
	callback_return((int) client_data); /* return desired answer */
    }
}

static XtActionsRec actionTable[] = {
    {"popup_yes", popup_yes_action},
    {"popup_no", popup_no_action},
    {"accept_string", accept_string_action},
    {"undo", undo_action},
    {"quit", quit_action}
};

/* The <UnmapNotify> translations in the popups protect us from
   window managers which allow the user to popdown menus explicitly.
   Making <BtnUp> null is necessary to prevent Down,Motion,Up from
   generating two Notify events. */

static String list_translations =
    "<Motion>: Set() \n\
     <LeaveWindow>: Unset() \n\
     <Btn1Down>: Set()Notify() \n\
     <BtnUp>: \n";

static String confirm_translations =
    "<BtnDown>:	popup_yes()\n\
     <LeaveWindow>: popup_no()\n\
     <UnmapNotify>: popup_no()\n";

static String choose_translations =
    "<LeaveWindow>: popup_no()\n\
     <UnmapNotify>: popup_no()\n";

static String unmap_no_translation =
    "<UnmapNotify>: popup_no()\n";

static String message_trans =
    "<ClientMessage>WM_PROTOCOLS: quit()\n";

typedef struct _SdResources {
    String sequence;		/* -sequence */
    String database;		/* -db */
    String write_anyway_query;
    String delete_clip_query;
    String abort_query;
    String sess_init_query;
    String modify_format;
    String modify_tag_format;
    String modify_circ_format;
    String modify_line_two;
    String outfile_format;
    String title_format;
    String modifications_allowed[12];
    String start_list[start_select_kind_enum_extent];
    String cmd_list[NUM_CMD_BUTTON_KINDS];
    String resolve_list[resolve_command_kind_enum_extent];
    String quantifier_title;
    String selector_title;
    String direction_title;
    String tagger_title;
    String circcer_title;
} SdResources;

static SdResources sd_resources;

/* General program resources */

#define MENU(name, loc, default) \
    {name, "Menu", XtRString, sizeof(String), \
     XtOffsetOf(SdResources, loc), XtRString, default}

/* The following arrays must be coordinated with the Sd program */

/* BEWARE!!  This list is keyed to the definition of "start_select_kind" in sd.h . */
static XtResource startup_resources[] = {
    MENU("exit", start_list[start_select_exit], "Exit from the program"),
    MENU("heads1P2P", start_list[start_select_h1p2p], "Heads 1P2P"),
    MENU("sides1P2P", start_list[start_select_s1p2p], "Sides 1P2P"),
    MENU("headsStart", start_list[start_select_heads_start], "Heads start"),
    MENU("sidesStart", start_list[start_select_sides_start], "Sides start"),
    MENU("asTheyAre", start_list[start_select_as_they_are], "Just as they are"),
    MENU("toggleConceptLevels", start_list[start_select_toggle_conc], "Toggle concept levels"),
    MENU("toggleActivePhantoms", start_list[start_select_toggle_act], "Toggle active phantoms"),
    MENU("toggleRetain_after_error", start_list[start_select_toggle_retain], "Toggle retain after error"),
    MENU("toggleNowarnMode", start_list[start_select_toggle_nowarn_mode], "Toggle nowarn mode"),
    MENU("toggleSingleClickMode", start_list[start_select_toggle_singleclick_mode], "Toggle singleclick mode"),
    MENU("toggleSinger", start_list[start_select_toggle_singer], "Toggle singing call"),
    MENU("toggleReverseSinger", start_list[start_select_toggle_singer_backward], "Toggle singing call with backward progression"),
    MENU("initSessionFile", start_list[start_select_init_session_file], "Initialize session file"),
    MENU("changeOutputFile", start_list[start_select_change_outfile], "Change output file"),
    MENU("changeTitle", start_list[start_select_change_header_comment], "Change title")
};

/* BEWARE!!  This list is keyed to the definition of "cmd_button_kind" above. */
static XtResource command_resources[] = {
    MENU("exit", cmd_list[cmd_button_quit], "Exit the program"),
    MENU("undo", cmd_list[cmd_button_undo], "Undo last call"),
    MENU("abort", cmd_list[cmd_button_abort], "Abort this sequence"),
    MENU("simplemods", cmd_list[cmd_button_simple_mods], "Simple modifications"),
    MENU("allowmods", cmd_list[cmd_button_allow_mods], "Allow modifications"),
    MENU("allowconcepts", cmd_list[cmd_button_allow_concepts], "Toggle concept levels"),
    MENU("activephantoms", cmd_list[cmd_button_active_phantoms], "Toggle active phantoms"),
    MENU("retainaftererror", cmd_list[cmd_button_retain_after_error], "Toggle retain after error"),
    MENU("nowarnmode", cmd_list[cmd_button_nowarn_mode], "Toggle nowarn mode"),
    MENU("comment", cmd_list[cmd_button_create_comment], "Insert a comment"),
    MENU("outfile", cmd_list[cmd_button_change_outfile], "Change output file"),
    MENU("title", cmd_list[cmd_button_change_title], "Change title"),
    MENU("getout", cmd_list[cmd_button_getout], "Write this sequence"),
    MENU("resolve", cmd_list[cmd_button_resolve], "Resolve"),
    MENU("reconcile", cmd_list[cmd_button_reconcile], "Reconcile"),
    MENU("anything", cmd_list[cmd_button_anything], "Pick random call"),
    MENU("nice", cmd_list[cmd_button_nice_setup], "Normalize"),
#ifdef NEGLECT
    MENU("neglect", cmd_list[cmd_button_neglect], "Show neglected calls ..."),
#endif
    MENU("savepic", cmd_list[cmd_button_save_pic], "Keep picture")
};

static XtResource resolve_resources[] = {
    MENU("abort", resolve_list[resolve_command_abort], "abort the search"),
    MENU("find", resolve_list[resolve_command_find_another], "find another"),
    MENU("next", resolve_list[resolve_command_goto_next], "go to next"),
    MENU("previous", resolve_list[resolve_command_goto_previous], "go to previous"),
    MENU("accept", resolve_list[resolve_command_accept], "ACCEPT current choice"),
    MENU("raiseRec", resolve_list[resolve_command_raise_rec_point], "raise reconcile point"),
    MENU("lowerRec", resolve_list[resolve_command_lower_rec_point], "lower reconcile point"),
    MENU("writethis", resolve_list[resolve_command_write_this], "write this sequence")
};

static XtResource enabledmods_resources[] = {
    MENU("none",      modifications_allowed[0],  ""),
    MENU("none_ap",   modifications_allowed[1],  "[AP]"),
    MENU("none_ac",   modifications_allowed[2],  "[all concepts]"),
    MENU("none_cp",   modifications_allowed[3],  "[all concepts,AP]"),
    MENU("simple",    modifications_allowed[4],  "[simple modifications]"),
    MENU("simple_ap", modifications_allowed[5],  "[AP,simple modifications]"),
    MENU("simple_ac", modifications_allowed[6],  "[all concepts,simple modifications]"),
    MENU("simple_cp", modifications_allowed[7],  "[all concepts,AP,simple modifications]"),
    MENU("all_ap",    modifications_allowed[8],  "[all modifications]"),
    MENU("all_ac",    modifications_allowed[9],  "[AP,all modifications]"),
    MENU("all",       modifications_allowed[10], "[all concepts,all modifications]"),
    MENU("all_cp",    modifications_allowed[11], "[all concepts,AP,all modifications]")
};

static XtResource confirm_resources[] = {
    MENU("writeAnyway", write_anyway_query, "Do you want to write this sequence anyway?"),
    MENU("deleteClipboard", delete_clip_query, "Do you want to delete the entire clipboard?"),
    MENU("abort", abort_query, "Do you really want to abort this sequence?"),
    MENU("sessInit", sess_init_query, "Do you really want to re-initialize your session file?"),
    MENU("modifyFormat", modify_format, "The \"%s\" can be replaced."),
    MENU("modifyTagFormat", modify_tag_format, "The \"%s\" can be replaced with a tagging call."),
    MENU("modifyCircFormat", modify_circ_format, "The \"%s\" can be replaced with a modified circulate-like call."),
    MENU("modifyLineTwo", modify_line_two, "Do you want to replace it?")
};

static XtResource outfile_resources[] = {
    MENU("format", outfile_format,
	 "Sequence output file is \"%s\".  Enter new name:"),
};

static XtResource header_resources[] = {
    MENU("format", title_format,
	 "Current title is \"%s\".  Enter new title:"),
};

static XtResource choose_resources[] = {
    MENU("who", selector_title, "  Who?  "),
    MENU("direction", direction_title, "Direction?"),
    MENU("tagger", tagger_title, "Tagging call?"),
    MENU("circcer", circcer_title, "Circulate replacement?"),
    MENU("howMany", quantifier_title, "How many?")
};

static XtResource top_level_resources[] = {
    MENU("sequenceFile", sequence, SEQUENCE_FILENAME),
    MENU("databaseFile", database, DATABASE_FILENAME),
};

/* OpenWindows 3 defines fallback_resources arg to XtAppInitialize as
   being const-qualified. */
#ifndef CONST
#define CONST /* as nothing */
#endif

/* Fallback class resources */

CONST static char *fallback_resources[] = {
    "*frame.height: 600",
    "*frame.internalBorderWidth: 2", /* clearer which list scrollbar with */
    "*List.Cursor: left_ptr",	/* so list cursors get colored */
    "*Viewport*font: fixed",	/* keep the call/concept menus narrow */
    "*conceptpopup*font: 6x10", /* keep the popup concept menus smaller */
    "*showGrip: False",		/* no grip on Paned widgets */
    "*defaultColumns: 1",	/* Lists are forced to 1 column */
    "*forceColumns: True",
    "*text*displayCaret: false",
    "*text*scrollVertical: always",
    "*text*wrap: word",
    "*confirmpopup.WinGravity: 10", /* StaticGravity */
    "*confirm*label.borderWidth: 2",
    "*concept*items.borderWidth: 2",
    "*concept.hSpace: 20",	/* slack so don't move off accidentally */
    "*concept.vSpace: 20",
    "*selector.choose*borderWidth: 0",  /* prettier this way */
    "*direction.choose*borderWidth: 0", /* prettier this way */
    "*Dialog.value*Translations: #override <Key>Return: accept_string()\n",
    "*comment.label: You can insert a comment:",
    "*getout.label: Comment for this sequence:",
#ifdef NEGLECT
    "*neglect.label: Percentage (integer) of neglected calls:",
#endif
    "*abort.label: Abort",
    "*ok.label: Ok",
    "*nowriteGetout.label: Don't write sequence",
    "*noCommentGetout.label: No comment",
    "*useDefault.label: Use default",
    NULL};

static char *program_name = NULL;	/* argv[0]: our name */
static long_boolean window_is_mapped = FALSE;
static long_boolean ui_started = FALSE;

/*
 * The main program calls this before doing anything else, so we can
 * read the command line arguments that are relevant to the X user
 * interface.  Note: If we are writing a call list, the program will
 * exit before doing anything else with the user interface, but this
 * must be made anyway.
 */
extern void
uims_process_command_line(int *argcp, char **argvp[])
{
   int i;
   int argno = 1;
   char **argv = *argvp;

   /* First, ignore any switches that could be relevant to sdtty
      but meaningless to us. */

   while (argno < (*argcp)) {
      if (strcmp(argv[argno], "-no_line_delete") == 0) ;
      else if (strcmp(argv[argno], "-no_cursor") == 0) ;
      else if (strcmp(argv[argno], "-no_graphics") == 0) ;
      else if (strcmp(argv[argno], "-lines") == 0 && argno+1 < (*argcp)) {
         (*argcp) -= 2;      /* Remove two arguments from the list. */
         for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i+1];
         continue;
      }
      else {
         argno++;
         continue;
      }

      (*argcp)--;      /* Remove this argument from the list. */
      for (i=argno+1; i<=(*argcp); i++) argv[i-1] = argv[i];
   }

   program_name = argv[0];
   toplevel = XtAppInitialize(&xtcontext, "Sd", NULL, 0, argcp, argv,
			       fallback_resources, NULL, 0);
   ui_started = TRUE;
   XtGetApplicationResources(toplevel, (XtPointer) &sd_resources,
			      top_level_resources, XtNumber(top_level_resources),
			      NULL, 0);
   (void) strncpy(outfile_string, sd_resources.sequence, MAX_FILENAME_LENGTH);
   database_filename = sd_resources.database;
}

extern void uims_display_help(void)
{
   printf("\nIn addition, the usual X Window System flags are supported.\n");
}

/*
 * The main program calls this before any of the call menus are
 * created, that is, before any calls to uims_create_menu.
 */
extern void uims_preinitialize(void)
{
   char icon_name[25];
   String title;
   Widget box, leftarea, rightarea, startupmenu, infopanes, confirmbox;
   Widget cmdbox, speconsbox, conceptbox;
   XtTranslations list_trans, unmap_no_trans;

    /* add level name to icon */
    XtVaGetValues(toplevel, XtNiconName, &title, NULL);
    (void) sprintf(icon_name, "%s: %s", title, getout_strings[calling_level]);
    XtVaSetValues(toplevel, XtNiconName, icon_name, NULL);

    XtOverrideTranslations(toplevel, XtParseTranslationTable(message_trans));

    list_trans = XtParseTranslationTable(list_translations);

    box=XtVaCreateManagedWidget("frame", panedWidgetClass, toplevel,
               XtNorientation, XtEhorizontal,
               NULL);

    leftarea = XtVaCreateManagedWidget("leftmenus", panedWidgetClass, box, NULL);

    rightarea = XtVaCreateManagedWidget("rightmenus", panedWidgetClass, box, NULL);

    infopanes = XtVaCreateManagedWidget("infopanes", panedWidgetClass, box, NULL);

    /* the sole purpose of this box is to keep the cmdmenu from resizing
       itself.  If the window manager resizes us at startup, sizing won't
       happen until after the window gets mapped, and thus the command menu
       would have the shorter startup list and be the wrong size. */
    cmdbox = XtVaCreateManagedWidget("cmdbox", panedWidgetClass,
               leftarea, NULL);

    cmdmenu = XtVaCreateManagedWidget("command", listWidgetClass, cmdbox, NULL);
    XtOverrideTranslations(cmdmenu, list_trans);
    XtGetApplicationResources(cmdmenu, (XtPointer) &sd_resources,
               command_resources,
               XtNumber(command_resources), NULL, 0);
    /* This widget is never realized.  It is just a place to hang
       resources off of. */
    startupmenu = XtVaCreateWidget("startup", labelWidgetClass, cmdbox, NULL);
    XtGetApplicationResources(startupmenu, (XtPointer) &sd_resources,
               startup_resources,
               XtNumber(startup_resources), NULL, 0);

    /* Keeps the conceptspecialmenu from resizing itself.  See cmdbox above. */
    speconsbox = XtVaCreateManagedWidget("speconsbx", panedWidgetClass,
               leftarea, NULL);
    conceptspecialmenu =
         XtVaCreateManagedWidget("conceptspecial", listWidgetClass,
               speconsbox, NULL);
    XtAddCallback(conceptspecialmenu, XtNcallback,
               command_or_menu_chosen, (XtPointer) ui_special_concept);
    XtOverrideTranslations(conceptspecialmenu, list_trans);

    callbox = XtVaCreateManagedWidget("callbox", panedWidgetClass,
               rightarea, NULL);

    calltitle = XtVaCreateManagedWidget("calltitle", labelWidgetClass,
               callbox, NULL);

    /* Viewports may have vertical scrollbar, and it must be visible */
    lview =
         XtVaCreateManagedWidget("lma", viewportWidgetClass, leftarea, 
               XtNallowVert, True,
               XtNforceBars, True,
               NULL);
    callview =
         XtVaCreateManagedWidget("rma", viewportWidgetClass, callbox,
               XtNallowVert, True,
               XtNforceBars, True,
               NULL);

    conceptmenu =
         XtVaCreateManagedWidget("conceptmenu", listWidgetClass, lview,
               XtNdefaultColumns, 1, NULL);
    XtAddCallback(conceptmenu, XtNcallback,
               command_or_menu_chosen, (XtPointer)ui_concept_select);
    XtOverrideTranslations(conceptmenu, list_trans);

    callmenu =
         XtVaCreateManagedWidget("callmenu", listWidgetClass, callview,
               XtNdefaultColumns, 1, NULL);
    XtAddCallback(callmenu, XtNcallback,
               command_or_menu_chosen, (XtPointer)ui_call_select);
    XtOverrideTranslations(callmenu, list_trans);

    /* make it managed now so sizing gets done */
    resolvewin = XtVaCreateManagedWidget("resolvebox", panedWidgetClass,
               rightarea, NULL);

    resolvetitle = XtVaCreateManagedWidget("resolvetitle", labelWidgetClass,
               resolvewin, NULL);

    resolvemenu = XtVaCreateManagedWidget("resolve", listWidgetClass,
               resolvewin, NULL);
    XtAddCallback(resolvemenu, XtNcallback,
               command_or_menu_chosen, (XtPointer)ui_resolve_select);
    XtOverrideTranslations(resolvemenu, list_trans);
    XtGetApplicationResources(resolvemenu, (XtPointer) &sd_resources,
               resolve_resources,
               XtNumber(resolve_resources), NULL, 0);
    XawListChange(resolvemenu, sd_resources.resolve_list, resolve_command_kind_enum_extent,
               0, TRUE);

    /* Text label is long enough to make widget wide enough.
       Keyed to MAX_PRINT_LENGTH in sdutil.c.
       The two extra chars are for the text area scrollbar, what a kludge. */
    statuswin = XtVaCreateManagedWidget("enabledmods", labelWidgetClass,
               infopanes,
               XtNlabel,
               "XX-123456789-123456789-123456789-123456789-123456789-123456789",
               NULL);
    XtGetApplicationResources(statuswin, (XtPointer) &sd_resources,
               enabledmods_resources,
               XtNumber(enabledmods_resources), NULL, 0);

    txtwin = XtVaCreateManagedWidget("text", asciiTextWidgetClass, infopanes,
               /* set to known string so can append */
               XtNstring, "",
               NULL);

    /* confirmation popup */

    XtAppAddActions(xtcontext, actionTable, XtNumber(actionTable));

    confirmpopup = XtVaCreatePopupShell("confirmpopup",
               transientShellWidgetClass, toplevel,
               XtNallowShellResize, True, NULL);

    XtGetApplicationResources(confirmpopup, (XtPointer) &sd_resources,
               confirm_resources, XtNumber(confirm_resources),
               NULL, 0);
    XtOverrideTranslations(confirmpopup,
               XtParseTranslationTable(confirm_translations));

    /* this creates a margin which makes the popup prettier */
    confirmbox = XtVaCreateManagedWidget("confirm", boxWidgetClass,
               confirmpopup, NULL);

    confirmlabel = XtVaCreateManagedWidget("label", labelWidgetClass,
               confirmbox, NULL);

    XtRealizeWidget(confirmpopup); /* makes XtPopup faster to do this now */

    /* menu choose popup */

    choosepopup = XtVaCreatePopupShell("selector", transientShellWidgetClass,
               toplevel,
               XtNallowShellResize, True, NULL);
    XtOverrideTranslations(choosepopup,
               XtParseTranslationTable(choose_translations));

    choosebox = XtVaCreateManagedWidget("choose", boxWidgetClass,
               choosepopup, NULL);

    chooselabel = XtVaCreateManagedWidget("label", labelWidgetClass,
               choosebox, NULL);
    XtGetApplicationResources(chooselabel, (XtPointer) &sd_resources,
               choose_resources, XtNumber(choose_resources),
               NULL, 0);

    chooselist = XtVaCreateManagedWidget("items", listWidgetClass, choosebox,
               XtNcolumnSpacing, 0, NULL);
    XtAddCallback(chooselist, XtNcallback, choose_pick, (XtPointer)NULL);
    XtOverrideTranslations(chooselist, list_trans);

    XtRealizeWidget(choosepopup);

    /* comment popup */

    commentpopup = XtVaCreatePopupShell("commentpopup",
               transientShellWidgetClass, toplevel, 
               XtNallowShellResize, True, NULL);
    unmap_no_trans = XtParseTranslationTable(unmap_no_translation);
    XtOverrideTranslations(commentpopup, unmap_no_trans);

    commentbox = XtVaCreateManagedWidget("comment", dialogWidgetClass,
               commentpopup,
               /* create an empty value area */
               XtNvalue, "", NULL);

    XawDialogAddButton(commentbox, "abort", dialog_callback,
               (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(commentbox, "ok", dialog_callback,
               (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(commentpopup); /* makes XtPopup faster to do this now */


    /* outfile popup */

    outfilepopup = XtVaCreatePopupShell("outfilepopup",
               transientShellWidgetClass, toplevel,
               XtNallowShellResize, True,
               NULL);
    XtGetApplicationResources(outfilepopup, (XtPointer) &sd_resources,
               outfile_resources, XtNumber(outfile_resources),
               NULL, 0);
    XtOverrideTranslations(outfilepopup, unmap_no_trans);

    outfilebox = XtVaCreateManagedWidget("outfile", dialogWidgetClass,
					 outfilepopup,
					 /* create an empty value area */
					 XtNvalue, "",
					 /* to make it wide enough */
					 XtNlabel, "Sequence output file is 'sequence.PLUS'.  Enter new name:",
					 NULL);

    XawDialogAddButton(outfilebox, "abort", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(outfilebox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(outfilepopup); /* makes XtPopup faster to do this now */


    /* title popup */

    titlepopup = XtVaCreatePopupShell("titlepopup",
               transientShellWidgetClass, toplevel,
               XtNallowShellResize, True,
               NULL);
    XtGetApplicationResources(titlepopup, (XtPointer) &sd_resources,
               header_resources, XtNumber(header_resources),
               NULL, 0);
    XtOverrideTranslations(titlepopup, unmap_no_trans);

    titlebox = XtVaCreateManagedWidget("title", dialogWidgetClass,
					 titlepopup,
					 /* create an empty value area */
					 XtNvalue, "",
					 /* to make it wide enough */
					 XtNlabel, "Current title is 'dummy title'.  Enter new title:",
					 NULL);

    XawDialogAddButton(titlebox, "abort", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(titlebox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(titlepopup); /* makes XtPopup faster to do this now */


    /* getout popup */

    getoutpopup = XtVaCreatePopupShell("getoutpopup",
				       transientShellWidgetClass, toplevel,
				       XtNallowShellResize, True, NULL);
    XtOverrideTranslations(getoutpopup, unmap_no_trans);

    getoutbox = XtVaCreateManagedWidget("getout", dialogWidgetClass,
					getoutpopup,
					/* create an empty value area */
					XtNvalue, "", NULL);

    XawDialogAddButton(getoutbox, "nowriteGetout", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(getoutbox, "noCommentGetout", dialog_callback,
		       (XtPointer)POPUP_ACCEPT);
    XawDialogAddButton(getoutbox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(getoutpopup); /* makes XtPopup faster to do this now */

#ifdef NEGLECT
    /* neglect popup */

    neglectpopup = XtVaCreatePopupShell("neglectpopup",
					transientShellWidgetClass, toplevel,
					XtNallowShellResize, True, NULL);
    XtOverrideTranslations(neglectpopup, unmap_no_trans);

    neglectbox = XtVaCreateManagedWidget("neglect", dialogWidgetClass,
					 neglectpopup,
					 /* create an empty value area */
					 XtNvalue, "", NULL);

    XawDialogAddButton(neglectbox, "useDefault", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(neglectbox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(neglectpopup); /* makes XtPopup faster to do this now */
#endif

    /* concept popup */

    conceptpopup = XtVaCreatePopupShell("conceptpopup",
					transientShellWidgetClass,
					toplevel,
					XtNallowShellResize, True, NULL);
    XtOverrideTranslations(conceptpopup,
			   XtParseTranslationTable(choose_translations));

    /* this creates a margin which makes it easier to mouse the edge items */
    conceptbox = XtVaCreateManagedWidget("concept", boxWidgetClass,
					 conceptpopup, NULL);

    conceptlist = XtVaCreateManagedWidget("items", listWidgetClass,
					  conceptbox, NULL);
    XtAddCallback(conceptlist, XtNcallback, choose_pick, (XtPointer)NULL);
    XtOverrideTranslations(conceptlist, list_trans);

    XtRealizeWidget(conceptpopup);
}

static void
add_call_to_menu(Cstring **menu, int call_menu_index, int menu_size, Cstring callname)
{
    if (call_menu_index == 0) {	/* first item in this menu; set it up */
	*menu = get_mem((unsigned int)(menu_size+1) * sizeof(String *));
    }

    (*menu)[call_menu_index] = callname;
}

static Cstring *concept_menu_list;
static int concept_menu_len;

static Cstring *call_menu_lists[NUM_CALL_LIST_KINDS];
static Cstring call_menu_names[NUM_CALL_LIST_KINDS];


static call_list_kind longest_title = call_list_empty;
static int longest_title_length = 0;

/*
 * Create a menu containing number_of_calls[cl] items.
 * This will be called once for each value in the enumeration call_list_kind.
 */
extern void uims_create_menu(call_list_kind cl)
{
   int i;
   int name_len = strlen((char *) menu_names[cl]);

   for (i=0; i<number_of_calls[cl]; i++) {
      add_call_to_menu(&call_menu_lists[cl], i,
         number_of_calls[cl], main_call_lists[cl][i]->menu_name);
   }

   call_menu_names[cl] = menu_names[cl];

   /* XXX - counting characters is not correct because the font
      need not be fixed width.  */
   if (name_len > longest_title_length) {
      longest_title_length = name_len;
      longest_title = cl;
   }
}

static call_list_kind visible_call_menu = call_list_none;

/*
 * display the requested call menu on the screen
 */
static void
set_call_menu(call_list_kind call_menu, call_list_kind title)
{
    int menu_num = (int) call_menu;
    int title_num = (int) title;

    XtVaSetValues(calltitle, XtNlabel, call_menu_names[title_num], NULL);
    XawListChange(callmenu, (char **) call_menu_lists[menu_num],
		  number_of_calls[menu_num], 0, TRUE);
    visible_call_menu = call_menu==title ? call_menu : call_list_none;
}

/*
 * Because the initial width of the Viewport doesn't allow for the
 * scrollbar, widen it a little.  I don't know a good way to do this,
 * so I use the following kludge: set the columnspacing of the child
 * list (space added to the right of the only column) to the width
 * of the scrollbar.
 */
static void
widen_viewport(Widget vw, Widget childw)
{
    Widget scrollbar = XtNameToWidget(vw, XtEvertical);
    Dimension scrollwidth, scrollborder;

    if (!scrollbar) {
	(void) fprintf(stderr, "%s warning: viewport %s has no scrollbar, will not widen\n",
		       program_name, XtName(vw));
	return;
    }

    XtVaGetValues(scrollbar, 
		  XtNwidth, &scrollwidth,
		  XtNborderWidth, &scrollborder,
		  NULL);

    XtVaSetValues(childw, XtNcolumnSpacing, scrollwidth+scrollborder, NULL);
}

static Cstring empty_menu[] = {NULL};

/* The main program calls this after all the call menus have been created,
   after all calls to uims_create_menu.
   This performs any final initialization required by the interface package.

   It must also perform any required setup of the concept menu.  The
   concepts are not presented procedurally.  Instead, they can be found
   in the external array concept_descriptor_table+general_concept_offset.
   The number of concepts in that list is general_concept_size.  For each
   i, the field concept_descriptor_table[i].name has the text that we
   should display for the user.
*/

extern void uims_postinitialize(void)
{
   int i;

   /* initialize our special empty call menu */
   call_menu_lists[call_list_empty] = empty_menu;
   number_of_calls[call_list_empty] = 0;
   call_menu_names[call_list_empty] = "";

    /* fill in general concept menu */
   for (i=0; i<general_concept_size; i++)
      add_call_to_menu(&concept_menu_list, i, general_concept_size,
                       concept_descriptor_table[i+general_concept_offset].menu_name);

   concept_menu_len = general_concept_size;

    /*
     * Before realizing, fill everything up with its normal information
     * so it can be sized correctly.
     */
    XawListChange(cmdmenu, sd_resources.cmd_list, NUM_CMD_BUTTON_KINDS, 0, TRUE);
    XawListChange(conceptspecialmenu, (char **) concept_menu_strings, 0, 0, TRUE);
    XawListChange(conceptmenu, (char **) concept_menu_list, concept_menu_len, 0, TRUE);
    set_call_menu (call_list_any, longest_title);

    widen_viewport(lview, conceptmenu);
    widen_viewport(callview, callmenu);

    XtSetMappedWhenManaged(toplevel, FALSE);
    XtRealizeWidget(toplevel);

    /* do WM_DELETE_WINDOW before map */
    wm_delete_window = XInternAtom(XtDisplay(toplevel), "WM_DELETE_WINDOW", False);
    wm_protocols = XInternAtom(XtDisplay(toplevel), "WM_PROTOCOLS", False);
    XSetWMProtocols(XtDisplay(toplevel), XtWindow(toplevel), &wm_delete_window, 1);

    XtMapWidget(toplevel);
    window_is_mapped = TRUE;
}


extern void uims_set_window_title(char s[])
{
}


static void switch_from_startup_mode(void)
{
    XawListChange(cmdmenu, sd_resources.cmd_list, NUM_CMD_BUTTON_KINDS, 0, FALSE);
    XtRemoveAllCallbacks(cmdmenu, XtNcallback);
    XtAddCallback(cmdmenu, XtNcallback,
		  command_or_menu_chosen, (XtPointer)ui_command_select);
    XawListChange(conceptspecialmenu, (char **) concept_menu_strings, 0, 0, TRUE);
    XawListChange(conceptmenu, (char **) concept_menu_list, concept_menu_len, 0, TRUE);
}

static void
switch_to_resolve_mode(void)
{
    if (visible_mode == mode_startup)
	switch_from_startup_mode();
    XtUnmanageChild(callbox);
    XtManageChild(resolvewin);

    visible_mode = mode_resolve;
}

static int last_banner = -1;


extern uims_reply uims_get_startup_command(void)
{
   int banner_mode;

   try_again:

   /* Update the text area */
   XawTextEnableRedisplay(txtwin);

   /* See if the banner needs to be updated. */

   banner_mode = (allowing_modifications << 2) |
                 (allowing_all_concepts ? 2 : 0) |
                 (using_active_phantoms ? 1 : 0);

   if (last_banner != banner_mode) {
      XtVaSetValues(statuswin,
                 XtNlabel,
                 sd_resources.modifications_allowed[banner_mode],
                 NULL);
      last_banner = banner_mode;
   }

   if (visible_mode != mode_startup) {
      XtUnmanageChild(resolvewin); /* managed at startup, too */
      XtManageChild(callbox); /* nec if mode_resolve now */
      XawListChange(cmdmenu, sd_resources.start_list,
                 start_select_kind_enum_extent, 0, FALSE);
      XawListChange(conceptspecialmenu, (char **) empty_menu, 0, 0, FALSE);
      XawListChange(conceptmenu, (char **) empty_menu, 0, 0, FALSE);
      set_call_menu (call_list_empty, call_list_empty);
      XtRemoveAllCallbacks(cmdmenu, XtNcallback);
      XtAddCallback(cmdmenu, XtNcallback,
                 command_or_menu_chosen, (XtPointer)ui_start_select);
      visible_mode = mode_startup;
   }

   inside_what = inside_get_command;
   {
      int local_reply = read_user_gesture(xtcontext);

      if (local_reply == USER_GESTURE_NULL)
         goto try_again;
      else if (local_reply == ui_command_select) {
         if (uims_menu_index == SPECIAL_SIMPLE_MODS) {
            /* Increment "allowing_modifications" up to a maximum of 2. */
            if (allowing_modifications != 2) allowing_modifications++;
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_ALLOW_MODS) {
            allowing_modifications = 2;
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_TOGGLE_ALL_CONCEPTS) {
            allowing_all_concepts = !allowing_all_concepts;
            /* ***** Maybe we should change visibility of off-level concepts at this point. */
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_TOGGLE_ACT_PHANTOMS) {
            using_active_phantoms = !using_active_phantoms;
            goto try_again;
         }
#ifdef OLD_ELIDE_BLANKS_JUNK
         else if (uims_menu_index == SPECIAL_TOGGLE_IGNOREBLANKS) {
            elide_blanks = !elide_blanks;
            goto try_again;
         }
#endif
         else if (uims_menu_index == SPECIAL_TOGGLE_RETAIN_AFTER_ERROR) {
            retain_after_error = !retain_after_error;
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_TOGGLE_NOWARN_MODE) {
            nowarn_mode = !nowarn_mode;
            goto try_again;
         }
      }

      return local_reply;
   }
}


extern long_boolean uims_get_call_command(uims_reply *reply_p)
{
   int local_reply;
   int banner_mode;

   try_again:

   /* Update the text area */
   XawTextEnableRedisplay(txtwin);

   /* See if the banner needs to be updated. */

   banner_mode = (allowing_modifications << 2) |
                 (allowing_all_concepts ? 2 : 0) |
                 (using_active_phantoms ? 1 : 0);

   if (last_banner != banner_mode) {
      XtVaSetValues(statuswin,
                 XtNlabel,
                 sd_resources.modifications_allowed[banner_mode],
                 NULL);
      last_banner = banner_mode;
   }

   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;

   if (visible_mode != mode_normal) {
      if (visible_mode == mode_resolve) {
         XtUnmanageChild(resolvewin);
         XtManageChild(callbox);
      } else if (visible_mode == mode_startup) {
         switch_from_startup_mode();
      }
      visible_mode = mode_normal;
      visible_call_menu = call_list_none; /* no call menu visible */
   }
   if (visible_call_menu != parse_state.call_list_to_use)
      set_call_menu (parse_state.call_list_to_use, parse_state.call_list_to_use);

   inside_what = inside_get_command;
   local_reply = read_user_gesture(xtcontext);

   if (local_reply == USER_GESTURE_NULL)
      goto try_again;
   else if (local_reply == ui_command_select) {
      if (uims_menu_index == SPECIAL_SIMPLE_MODS) {
         /* Increment "allowing_modifications" up to a maximum of 2. */
         if (allowing_modifications != 2) allowing_modifications++;
         goto try_again;
      }
      else if (uims_menu_index == SPECIAL_ALLOW_MODS) {
         allowing_modifications = 2;
         goto try_again;
      }
      else if (uims_menu_index == SPECIAL_TOGGLE_ALL_CONCEPTS) {
         allowing_all_concepts = !allowing_all_concepts;
         /* ***** Maybe we should change visibility of off-level concepts at this point. */
         goto try_again;
      }
      else if (uims_menu_index == SPECIAL_TOGGLE_ACT_PHANTOMS) {
         using_active_phantoms = !using_active_phantoms;
         goto try_again;
      }
#ifdef OLD_ELIDE_BLANKS_JUNK
      else if (uims_menu_index == SPECIAL_TOGGLE_IGNOREBLANKS) {
         elide_blanks = !elide_blanks;
         goto try_again;
      }
#endif
      else if (uims_menu_index == SPECIAL_TOGGLE_RETAIN_AFTER_ERROR) {
         retain_after_error = !retain_after_error;
         goto try_again;
      }
      else if (uims_menu_index == SPECIAL_TOGGLE_NOWARN_MODE) {
         nowarn_mode = !nowarn_mode;
         goto try_again;
      }
   }

   *reply_p = local_reply;

   if (*reply_p == ui_call_select) {
      /* If user gave a call, deposit same. */

      callspec_block *save_call = main_call_lists[parse_state.call_list_to_use][uims_menu_index];
      if (deposit_call(save_call, &null_options)) return TRUE;
   }
   else if (*reply_p == ui_concept_select) {
      /* A concept is required.  Its index has been stored in uims_menu_index. */

      if (deposit_concept(&concept_descriptor_table[uims_menu_index]))
         return TRUE;
   }

   return FALSE;
}


extern uims_reply uims_get_resolve_command(void)
{
   int banner_mode;

   try_again:

   /* Update the text area */
   XawTextEnableRedisplay(txtwin);

   banner_mode = (allowing_modifications << 2) |
                 (allowing_all_concepts ? 2 : 0) |
                 (using_active_phantoms ? 1 : 0);

   if (last_banner != banner_mode) {
      XtVaSetValues(statuswin,
                 XtNlabel,
                 sd_resources.modifications_allowed[banner_mode],
                 NULL);
      last_banner = banner_mode;
   }

   if (visible_mode != mode_resolve)
      switch_to_resolve_mode();

   inside_what = inside_get_command;
   {
      int local_reply = read_user_gesture(xtcontext);

      if (local_reply == USER_GESTURE_NULL)
         goto try_again;
      else if (local_reply == ui_command_select) {
         if (uims_menu_index == SPECIAL_SIMPLE_MODS) {
            /* Increment "allowing_modifications" up to a maximum of 2. */
            if (allowing_modifications != 2) allowing_modifications++;
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_ALLOW_MODS) {
            allowing_modifications = 2;
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_TOGGLE_ALL_CONCEPTS) {
            allowing_all_concepts = !allowing_all_concepts;
            /* ***** Maybe we should change visibility of off-level concepts at this point. */
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_TOGGLE_ACT_PHANTOMS) {
            using_active_phantoms = !using_active_phantoms;
            goto try_again;
         }
#ifdef OLD_ELIDE_BLANKS_JUNK
         else if (uims_menu_index == SPECIAL_TOGGLE_IGNOREBLANKS) {
            elide_blanks = !elide_blanks;
            goto try_again;
         }
#endif
         else if (uims_menu_index == SPECIAL_TOGGLE_RETAIN_AFTER_ERROR) {
            retain_after_error = !retain_after_error;
            goto try_again;
         }
         else if (uims_menu_index == SPECIAL_TOGGLE_NOWARN_MODE) {
            nowarn_mode = !nowarn_mode;
            goto try_again;
         }
      }

      return local_reply;
   }
}




static int get_popup_string(Widget popup, Widget dialog, char dest[])
{
    int value;

    value = do_popup(popup);
    if (value == POPUP_ACCEPT_WITH_STRING)
	(void) strcpy(dest, XawDialogGetValueString(dialog));
    return value;
}


extern int uims_do_comment_popup(char dest[])
{
    return get_popup_string(commentpopup, commentbox, dest);
}


extern int uims_do_outfile_popup(char dest[])
{
    char outfile_question[MAX_TEXT_LINE_LENGTH];

    (void) sprintf(outfile_question,
		   sd_resources.outfile_format, outfile_string);
    XtVaSetValues(outfilebox, XtNlabel, outfile_question, NULL);
    return get_popup_string(outfilepopup, outfilebox, dest);
}


extern int uims_do_header_popup(char dest[])
{
   char title_question[MAX_TEXT_LINE_LENGTH];

   (void) sprintf(title_question,
         sd_resources.title_format, header_comment);
   XtVaSetValues(titlebox, XtNlabel, title_question, NULL);

   return get_popup_string(titlepopup, titlebox, dest);
}


extern int uims_do_getout_popup(char dest[])
{
    return get_popup_string(getoutpopup, getoutbox, dest);
}


#ifdef NEGLECT
extern int
uims_do_neglect_popup(char dest[])
{
    return get_popup_string(neglectpopup, neglectbox, dest);
}
#endif


static int confirm(String question)
{
    XtVaSetValues(confirmlabel, XtNlabel, question, NULL);
    return do_popup(confirmpopup);
}


extern int uims_do_write_anyway_popup(void)
{
    return confirm(sd_resources.write_anyway_query);
}


extern int uims_do_delete_clipboard_popup(void)
{
    return confirm(sd_resources.delete_clip_query);
}


extern int uims_do_abort_popup(void)
{
    return confirm(sd_resources.abort_query);
}


extern int uims_do_session_init_popup(void)
{
    return confirm(sd_resources.sess_init_query);
}


extern int uims_do_modifier_popup(Cstring callname, modify_popup_kind kind)
{
   char modifier_question[150];
   char *line_format = "Internal error: unknown modifier kind.";

   switch (kind) {
     case modify_popup_any:
       line_format = sd_resources.modify_format;
       break;
     case modify_popup_only_tag:
       line_format = sd_resources.modify_tag_format;
       break;
     case modify_popup_only_circ:
       line_format = sd_resources.modify_circ_format;
       break;
   }
   (void) sprintf(modifier_question, line_format, callname);
   if (strlen(sd_resources.modify_line_two)) {
       (void) strcat(modifier_question, "\n");
       (void) strcat(modifier_question, sd_resources.modify_line_two);
   }
   return confirm(modifier_question);
}


/*
 * Make sure any requested changes have appeared on the screen.
 * This function only works in the limited domain that it is being
 * used in here.
 */
static void
update_display(Widget w)
{
    XEvent event;
    Display *display = XtDisplay(w);

    /* make sure all Expose events that are going to arrive are here now */
    XSync(display, False);
    while (XCheckMaskEvent(display, ExposureMask, &event))
	XtDispatchEvent(&event);
    XFlush(display);
}



extern void uims_update_resolve_menu(command_kind goal, int cur, int max, resolver_display_state state)
{
    char title[MAX_TEXT_LINE_LENGTH];

    create_resolve_menu_title(goal, cur, max, state, title);
    XtVaSetValues(resolvetitle, XtNlabel, title, NULL);
    switch_to_resolve_mode();
    update_display(resolvetitle);
}


static int
choose_popup(String label, Cstring names[])
{
    Dimension labelwidth, listwidth;
    Dimension listintwid;
    Dimension stdlistintwid = 4;

    XtVaSetValues(chooselabel, XtNlabel, label, NULL);
    /* we clobber this each time */
    XtVaSetValues(chooselist, XtNinternalWidth, stdlistintwid, NULL);

    XawListChange(chooselist, (char **) names, 0, 0, TRUE);

    /* set the width of the box to the width of the widest child */

    XtVaGetValues(chooselabel, XtNwidth, &labelwidth, NULL);

    XtVaGetValues(chooselist,
		  XtNwidth, &listwidth,
		  XtNinternalWidth, &listintwid,
		  NULL);

    if (labelwidth > listwidth) {
	/* Make the list be centered.  Works because columnSpacing is 0 */
	XtVaSetValues(chooselist,
		      XtNinternalWidth,
		      listintwid + (int)(labelwidth-listwidth)/2, NULL);
    } else {
	XtVaSetValues(chooselabel,
		      XtNwidth, listwidth,
		      /* must set label again to force box to redo layout */
		      XtNlabel, label,
		      NULL);
    }

    return do_popup(choosepopup);
}

extern int uims_do_selector_popup(void)
{
   /* We skip the zeroth selector, which is selector_uninitialized. */
   int t = choose_popup(sd_resources.selector_title, selector_menu_list);
   if (t==0) return POPUP_DECLINE;
   return t;
}    

extern int uims_do_direction_popup(void)
{
    /* We skip the zeroth direction, which is direction_uninitialized. */
    int t = choose_popup(sd_resources.direction_title, &direction_names[1]);
    if (t==0) return POPUP_DECLINE;
    return t;
}    


extern int uims_do_circcer_popup(void)
{
    int t = choose_popup(sd_resources.circcer_title, circcer_menu_list);
    if (t==0) return POPUP_DECLINE;
    return t;
}    


extern int uims_do_tagger_popup(int tagger_class)
{
    int t = choose_popup(sd_resources.tagger_title, tagger_menu_list[tagger_class]);
    if (t==0) return POPUP_DECLINE;
    return (tagger_class << 5) | t;
}    


extern uint32 uims_get_number_fields(int nnumbers, long_boolean forbid_zero)
{
   int i;
   uint32 number_list = 0;

   for (i=0 ; i<nnumbers ; i++) {
      unsigned int this_num;

      if (interactivity == interactivity_verify) {
         // The second number in the series is always 1.
         // This makes "N-N-N-N change the web" and "N-N-N-N
         // relay the top" work.
         this_num = (i==1) ? 1 : number_for_initialize;
      }
      else {
         this_num = choose_popup(sd_resources.quantifier_title, cardinals);
         if (this_num == 0) return ~0;    /* User waved the mouse away. */
      }

      if (forbid_zero && this_num == 1) return ~0;
      number_list |= ((this_num-1) << (i*4));
   }

   return number_list;
}






/* variables used by the next two routines */

static XawTextPosition *line_indexes = NULL; /* end position of each line */
static int line_count = 0;	/* size of line_indexes */
static XawTextBlock text_block;

/*
 * add a line to the text output area.
 * the_line does not have the trailing Newline in it and
 * is volatile, so we must copy it if we need it to stay around.
 */
extern void uims_add_new_line(char the_line[], int drawing_picture)
{
    XawTextPosition prev_size = line_count ? line_indexes[line_count-1] : 0;
    XawTextPosition line_len, new_size;
    int status;

    XawTextDisableRedisplay(txtwin);

    line_len = strlen(the_line);
    new_size = prev_size + line_len;
    line_count++;
    line_indexes =
	get_more_mem(line_indexes, line_count * sizeof(XawTextPosition*));
    line_indexes[line_count-1] = new_size + 1; /* 1 more for the newline */

    /* make text widget writable */
    XtVaSetValues(txtwin, XtNeditType, XawtextEdit, NULL);
    /* append new line */
    text_block.firstPos = 0;
    text_block.length = line_len;
    text_block.ptr = the_line;
    text_block.format = FMT8BIT;
    status = XawTextReplace(txtwin, prev_size, prev_size, &text_block);
    if (status != XawEditDone)
	(void) fprintf(stderr, "%s warning: text append returned %d\n",
		       program_name, status);
    text_block.length = 1;
    text_block.ptr = "\n";
    status = XawTextReplace(txtwin, new_size, new_size, &text_block);
    if (status != XawEditDone)
	(void) fprintf(stderr, "%s warning: newline append returned %d\n",
		       program_name, status);
    /* move to end to ensure we scroll this line onto the screen */
    XawTextSetInsertionPoint(txtwin, new_size);
    /* make text widget read-only again */
    XtVaSetValues(txtwin, XtNeditType, XawtextRead, NULL);
}

/*
 * Throw away all but the first n lines of the text output.
 * n = 0 means to erase the entire buffer.
 */
extern void
uims_reduce_line_count(int n)
{
    XawTextPosition prev_size = line_count ? line_indexes[line_count-1] : 0;
    XawTextPosition new_size;
    int status;

    if (n >= line_count)
	return;			/* should never happen */
    new_size = n==0 ? 0 : line_indexes[n-1];

    XawTextDisableRedisplay(txtwin);

    /* make text widget writable */
    XtVaSetValues(txtwin, XtNeditType, XawtextEdit, NULL);
    /* delete the text */
    text_block.length = 0;
    status = XawTextReplace(txtwin, new_size, prev_size, &text_block);
    if (status != XawEditDone)
	(void) fprintf(stderr, "%s warning: text delete returned %d\n",
		       program_name, status);
    /* make text widget read-only again */
    XtVaSetValues(txtwin, XtNeditType, XawtextRead, NULL);

    line_count = n;
    if (line_count == 0) {
	free_mem(line_indexes);
	line_indexes = NULL;
    } else
	line_indexes =
	    get_more_mem(line_indexes, line_count * sizeof(XawTextPosition *));
}


extern void
uims_terminate(void)
{
    /* if uims_process_command_line was called, close down the window system */
    if (ui_started) {
	if (window_is_mapped)
	    XtUnmapWidget(toplevel); 	/* make it disappear neatly */
	XtDestroyWidget(toplevel);
	XtDestroyApplicationContext(xtcontext);
    }
}

/*
 * The following functions allow the UI to put up a progress
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

extern void uims_database_error(Cstring message, Cstring call_name)
{
   print_line(message);
   if (call_name) {
      print_line("  While reading this call from the database:");
      print_line(call_name);
   }
}

extern void uims_bad_argument(Cstring s1, Cstring s2, Cstring s3)
{
   if (s1) print_line(s1);
   if (s2) print_line(s2);
   if (s3) print_line(s3);
   print_line("Use the -help flag for help.");
   exit_program(1);
}


extern void uims_final_exit(int code)
{
   exit(code);
}
