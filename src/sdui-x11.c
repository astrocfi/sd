static char *id="@(#)$Sd: sdui-x11.c  1.14    gildea@lcs.mit.edu  18 Mar 93 $";
static char *time_stamp = "sdui-x11.c Time-stamp: <93/07/29 19:26:09 wba>";
/* 
 * sdui-x11.c - SD User Interface for X11
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
 * Uses the Athena Widget Set from X11 Release 4 or 5.
 *
 * For use with version 30 of the Sd program.
 *
 *  The version of this file is as shown in the third field of the id
 *  string in the first line.  This string gets displayed at program
 *  startup, as the "ui" part of the complete version.
 */

/* This file defines the following functions:
   uims_process_command_line
   uims_version_string
   uims_preinitialize
   uims_add_call_to_menu
   uims_finish_call_menu
   uims_postinitialize
   uims_get_command
   uims_do_comment_popup
   uims_do_outfile_popup
   uims_do_getout_popup
   uims_do_abort_popup
   uims_do_neglect_popup
   uims_do_selector_popup
   uims_do_quantifier_popup
   uims_do_modifier_popup
   uims_add_new_line
   uims_reduce_line_count
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

Private Widget toplevel, cmdmenu, conceptspecialmenu;
Private Widget conceptpopup, conceptlist;
Private Widget lview, callview, conceptmenu, callmenu;
Private Widget callbox, calltitle;
Private Widget statuswin, txtwin;
Private Widget resolvewin, resolvetitle, resolvemenu;
Private Widget confirmpopup, confirmlabel;
Private Widget choosepopup, choosebox, chooselabel, chooselist;
Private Widget commentpopup, commentbox, outfilepopup, outfilebox;
Private Widget getoutpopup, getoutbox;
Private Widget neglectpopup, neglectbox;

/* This is a special value we use internally. */
#define UI_SPECIAL_CONCEPT (uims_reply) -2

/* This is the order in which command buttons will appear on the screen.
    These will be translated into the things required by the main program by
    the translation table "button_translations", which must track this enumeration. */
typedef enum {
   cmd_button_quit,
   cmd_button_undo,
   cmd_button_abort,
   cmd_button_allow_mods,
   cmd_button_allow_concepts,
   cmd_button_create_comment,
   cmd_button_change_outfile,
   cmd_button_getout,
   cmd_button_resolve,
   cmd_button_reconcile,
   cmd_button_anything,
   cmd_button_nice_setup,
   cmd_button_neglect,
   cmd_button_save_pic
} cmd_button_kind;
#define NUM_CMD_BUTTON_KINDS (((int) cmd_button_save_pic)+1)

/*
 * The total version string looks something like
 * "27.4:db27.5:ui1.6X11"
 * We return the "1.6X11" part.
 */

Private char version_mem[12];

extern char *
uims_version_string(void)
{
    int field;
    char *start, *end;

    start = id;
    end = time_stamp;		/* keep lint happy */

    /*
     * extract the version number from the id string
     */
    for (field=1; field<3; field++) {
	start += strcspn(start, " \t");
	start += strspn(start, " \t");
    }
    end = start + strcspn(start, " \t");

    (void) strncpy(version_mem, start, end-start);
    (void) strcpy(version_mem+(end-start), "X11");
    return version_mem;
}


Private long_boolean ui_task_in_progress = FALSE;

/*
 * Since callbacks and actions can't return values, narrow the use of
 * global variables with these two routines.
 */

Private int callback_value_storage; /* used only by two routines below */

Private void
callback_return(int return_value)
{
    ui_task_in_progress = FALSE;
    callback_value_storage = return_value;
}

/* Serial number ensures each event generates at most one action */
Private unsigned long action_event_serial;

typedef enum {
    inside_nothing,
    inside_popup,
    inside_get_command
} ui_context;

/* Ensures commands aren't selected while a popup is up. */
Private ui_context inside_what = inside_nothing;

/*
 * Modification of XtAppMainLoop that returns to the caller
 * when the requested action has been done.  This technique
 * allows the main program to call the user interface as a
 * subroutine, whereas XtAppMainLoop assumes the other way around.
 */
Private int
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
Private void
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

Private XtAppContext xtcontext;

Private void
position_near_mouse(Widget popupshell)
{
    Window root, child;
    int x, y, max_x, max_y;
    int winx, winy;
    Dimension width, height, border;
    unsigned int mask;

    /* much of this is copied from CenterWidgetOnPoint in Xaw/TextPop.c,
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
    max_x = WidthOfScreen(XtScreen(popupshell)) - width;
    max_y = HeightOfScreen(XtScreen(popupshell)) - height;

    width += 2 * border;
    height += 2 * border;

    x -= ( (Position) width/2 );
    if (x < 0) x = 0;
    if (x > max_x) x = max_x;

    y -= ( (Position) height/2 );
    if (y < 0) y = 0;
    if ( y > max_y ) y = max_y;

    XtVaSetValues(popupshell,
		  XtNx, (Position)x,
		  XtNy, (Position)y,
		  NULL);
}


Private int
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

Private String empty_string = "";
Private String *concept_popup_list = NULL;

#define SPECIAL_COMMAND_ALLOW_MODS 0
#define SPECIAL_COMMAND_ALLOW_ALL_CONCEPTS 1

#define USER_GESTURE_NULL -1
#define SPECIAL_ALLOW_MODS -2
#define SPECIAL_ALLOW_ALL_CONCEPTS -3


/* Beware:  This table must track the enumeration "cmd_button_kind". */
static int button_translations[] = {
   command_quit,                          /* cmd_button_quit */
   command_undo,                          /* cmd_button_undo */
   command_abort,                         /* cmd_button_abort */
   SPECIAL_ALLOW_MODS,                    /* cmd_button_allow_mods */
   SPECIAL_ALLOW_ALL_CONCEPTS,            /* cmd_button_allow_concepts */
   command_create_comment,                /* cmd_button_create_comment */
   command_change_outfile,                /* cmd_button_change_outfile */
   command_getout,                        /* cmd_button_getout */
   command_resolve,                       /* cmd_button_resolve */
   command_reconcile,                     /* cmd_button_reconcile */
   command_anything,                      /* cmd_button_anything */
   command_nice_setup,                    /* cmd_button_nice_setup */
   command_neglect,                       /* cmd_button_neglect */
   command_save_pic                       /* cmd_button_save_pic */
};


/* ARGSUSED */
Private void
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
        else if (local_reply == UI_SPECIAL_CONCEPT) {
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
                            [ concept_offset_tables[menu][col]+row ].name;
                    else
                        concept_popup_list[i] = empty_string;
                    i++;
                }
            }
        
            XtVaSetValues(conceptlist, XtNdefaultColumns, maxcolumn, NULL);
            XawListChange(conceptlist, concept_popup_list, entries, 0, TRUE);
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
Private void 
cmdmenu_unhighlight(XtPointer client_data, XtIntervalId *intrvl)
{
    XawListUnhighlight(cmdmenu);
}

/* Simulates a click on the specified command menu item.
   Compare with command_or_menu_chosen above. */

Private void
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

Private Atom wm_delete_window;
Private Atom wm_protocols;

/* normally invoked only in response to a WM request */

/* ARGSUSED */
Private void
quit_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->type == ClientMessage  &&  event->xclient.message_type == wm_protocols)
	if (event->xclient.format != 32  ||  event->xclient.data.l[0] != wm_delete_window)
	    return;		/* some other WM_PROTOCOL */

    cmdmenu_action_internal(command_quit);
}

Private mode_kind visible_mode = mode_none;

/* Special action defined because it is handy to bind to MB3. */

/* ARGSUSED */
Private void
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
Private void
popup_yes_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->xany.serial >= action_event_serial) {
	callback_return(POPUP_ACCEPT);
    }
}

/* ARGSUSED */
Private void
popup_no_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->xany.serial >= action_event_serial) {
	callback_return(POPUP_DECLINE);
    }
}

/* ARGSUSED */
Private void
accept_string_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    callback_return(POPUP_ACCEPT_WITH_STRING);
}


/* popup callbacks */

/* ARGSUSED */
Private void
choose_pick(Widget w, XtPointer client_data, XtPointer call_data)
{
    XawListReturnStruct *item = (XawListReturnStruct *) call_data;

    if (inside_what == inside_popup) {
	callback_return(item->list_index + 1); /* can't return 0 */
    }
}

/* ARGSUSED */
Private void
dialog_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    if (inside_what == inside_popup) {
	callback_return((int) client_data); /* return desired answer */
    }
}

Private XtActionsRec actionTable[] = {
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

Private String list_translations =
    "<Motion>: Set() \n\
     <LeaveWindow>: Unset() \n\
     <Btn1Down>: Set()Notify() \n\
     <BtnUp>: \n";

Private String confirm_translations =
    "<BtnDown>:	popup_yes()\n\
     <LeaveWindow>: popup_no()\n\
     <UnmapNotify>: popup_no()\n";

Private String choose_translations =
    "<LeaveWindow>: popup_no()\n\
     <UnmapNotify>: popup_no()\n";

Private String unmap_no_translation =
    "<UnmapNotify>: popup_no()\n";

Private String message_trans =
    "<ClientMessage>WM_PROTOCOLS: quit()\n";

typedef struct _SdResources {
    String sequence;		/* -sequence */
    String database;		/* -db */
    String abort_query;
    String modify_format;
    String modify_tag_format;
    String modify_scoot_format;
    String modify_line_two;
    String outfile_format;
    String modifications_allowed[3];
    String start_list[NUM_START_SELECT_KINDS];
    String cmd_list[NUM_CMD_BUTTON_KINDS];
    String resolve_list[NUM_RESOLVE_COMMAND_KINDS];
    String quantifier_title;
    String selector_title;
} SdResources;

Private SdResources sd_resources;

/* General program resources */

#define MENU(name, loc, default) \
    {name, "Menu", XtRString, sizeof(String), \
     XtOffsetOf(SdResources, loc), XtRString, default}

Private XtResource startup_resources[] = {
    MENU("exit", start_list[start_select_exit], "Exit from the program"),
    MENU("heads1P2P", start_list[start_select_h1p2p], "Heads 1P2P"),
    MENU("sides1P2P", start_list[start_select_s1p2p], "Sides 1P2P"),
    MENU("headsStart", start_list[start_select_heads_start], "Heads start"),
    MENU("sidesStart", start_list[start_select_sides_start], "Sides start"),
    MENU("asTheyAre", start_list[start_select_as_they_are], "Just as they are")
};

Private XtResource command_resources[] = {
    MENU("exit", cmd_list[cmd_button_quit], "Exit the program"),
    MENU("undo", cmd_list[cmd_button_undo], "Undo last call"),
    MENU("abort", cmd_list[cmd_button_abort], "Abort this sequence"),
    MENU("allowmods", cmd_list[cmd_button_allow_mods], "Allow modifications"),
    MENU("allowconcepts", cmd_list[cmd_button_allow_concepts], "Toggle concept levels"),
    MENU("comment", cmd_list[cmd_button_create_comment], "Insert a comment ..."),
    MENU("outfile", cmd_list[cmd_button_change_outfile], "Change output file ..."),
    MENU("getout", cmd_list[cmd_button_getout], "End this sequence ..."),
    MENU("resolve", cmd_list[cmd_button_resolve], "Resolve ..."),
    MENU("reconcile", cmd_list[cmd_button_reconcile], "Reconcile ..."),
    MENU("anything", cmd_list[cmd_button_anything], "Do anything ..."),
    MENU("nice", cmd_list[cmd_button_nice_setup], "Nice setup ..."),
    MENU("neglect", cmd_list[cmd_button_neglect], "Show neglected calls ..."),
    MENU("savepic", cmd_list[cmd_button_save_pic], "Save picture")
};

Private XtResource resolve_resources[] = {
    MENU("abort", resolve_list[resolve_command_abort], "abort the search"),
    MENU("find", resolve_list[resolve_command_find_another], "find another"),
    MENU("next", resolve_list[resolve_command_goto_next], "go to next"),
    MENU("previous", resolve_list[resolve_command_goto_previous], "go to previous"),
    MENU("accept", resolve_list[resolve_command_accept], "ACCEPT current choice"),
    MENU("raiseRec", resolve_list[resolve_command_raise_rec_point], "raise reconcile point"),
    MENU("lowerRec", resolve_list[resolve_command_lower_rec_point], "lower reconcile point")
};

Private XtResource enabledmods_resources[] = {
    MENU("none", modifications_allowed[0], "No call modifications enabled"),
    MENU("simple", modifications_allowed[1], "Simple modifications enabled for this call"),
    MENU("all", modifications_allowed[2], "All modifications enabled for this call")
};

Private XtResource confirm_resources[] = {
    MENU("abort", abort_query, "Do you really want to abort this sequence?"),
    MENU("modifyFormat", modify_format, "The \"%s\" can be replaced."),
    MENU("modifyTagFormat", modify_tag_format, "The \"%s\" can be replaced with a tagging call."),
    MENU("modifyScootFormat", modify_scoot_format, "The \"%s\" can be replaced with a scoot/tag (chain thru) (and scatter)."),
    MENU("modifyLineTwo", modify_line_two, "Do you want to replace it?")
};

Private XtResource outfile_resources[] = {
    MENU("format", outfile_format,
	 "Sequence output file is \"%s\".  Enter new name:"),
};

Private XtResource choose_resources[] = {
    MENU("who", selector_title, "  Who?  "),
    MENU("howMany", quantifier_title, "How many?")
};

Private XtResource top_level_resources[] = {
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
    "*conceptpopup*font: fixed", /* keep the popup concept menus smaller */
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
    "*selector.choose*borderWidth: 0", /* prettier this way */
    "*Dialog.value*Translations: #override <Key>Return: accept_string()\n",
    "*comment.label: You can insert a comment:",
    "*getout.label: Text to be placed at the beginning of this sequence:",
    "*neglect.label: Percentage (integer) of neglected calls:",
    "*abort.label: Abort",
    "*ok.label: Ok",
    "*abortGetout.label: Abort getout",
    "*noHeader.label: No header",
    "*useDefault.label: Use default",
    NULL};

Private char *program_name = NULL;	/* argv[0]: our name */

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
    char **argv = *argvp;

    program_name = argv[0];
    toplevel = XtAppInitialize(&xtcontext, "Sd", NULL, 0, argcp, argv,
			       fallback_resources, NULL, 0);
    XtGetApplicationResources(toplevel, (XtPointer) &sd_resources,
			      top_level_resources, XtNumber(top_level_resources),
			      NULL, 0);
    (void) strncpy(outfile_string, sd_resources.sequence, MAX_FILENAME_LENGTH);
    database_filename = sd_resources.database;
}

/*
 * The main program calls this before any of the call menus are
 * created, that is, before any calls to uims_add_call_to_menu
 * or uims_finish_call_menu.
 */
extern void
uims_preinitialize(void)
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

    leftarea =
	XtVaCreateManagedWidget("leftmenus", panedWidgetClass, box, NULL);

    rightarea =
	XtVaCreateManagedWidget("rightmenus", panedWidgetClass, box, NULL);

    infopanes =
	XtVaCreateManagedWidget("infopanes", panedWidgetClass, box, NULL);

    /* the sole purpose of this box is to keep the cmdmenu from resizing
       itself.  If the window manager resizes us at startup, sizing won't
       happen until after the window gets mapped, and thus the command menu
       would have the shorter startup list and be the wrong size. */
    cmdbox = XtVaCreateManagedWidget("cmdbox", panedWidgetClass,
				     leftarea, NULL);

    cmdmenu = XtVaCreateManagedWidget("command", listWidgetClass,
				      cmdbox, NULL);
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
		  command_or_menu_chosen, (XtPointer) UI_SPECIAL_CONCEPT);
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
    XawListChange(resolvemenu, sd_resources.resolve_list, NUM_RESOLVE_COMMAND_KINDS,
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


    /* getout popup */

    getoutpopup = XtVaCreatePopupShell("getoutpopup",
				       transientShellWidgetClass, toplevel,
				       XtNallowShellResize, True, NULL);
    XtOverrideTranslations(getoutpopup, unmap_no_trans);

    getoutbox = XtVaCreateManagedWidget("getout", dialogWidgetClass,
					getoutpopup,
					/* create an empty value area */
					XtNvalue, "", NULL);

    XawDialogAddButton(getoutbox, "abortGetout", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(getoutbox, "noHeader", dialog_callback,
		       (XtPointer)POPUP_ACCEPT);
    XawDialogAddButton(getoutbox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(getoutpopup); /* makes XtPopup faster to do this now */

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

Private void
add_call_to_menu(String **menu, int call_menu_index, int menu_size, char callname[])
{
    if (call_menu_index == 0) {	/* first item in this menu; set it up */
	*menu = get_mem((unsigned int)(menu_size+1) * sizeof(String *));
    }

    (*menu)[call_menu_index] = callname;
}

Private String *concept_menu_list;
Private int concept_menu_len;

Private String *call_menu_lists[NUM_CALL_LIST_KINDS];
Private String call_menu_names[NUM_CALL_LIST_KINDS];

/*
 * We have been given the name of one call (call number
 * call_menu_index, from 0 to number_of_calls[cl]) to be added to the
 * call menu cl (enumerated over the type call_list_kind.)
 * The string is guaranteed to be in stable storage.
 */
extern void
uims_add_call_to_menu(call_list_kind cl, int call_menu_index, char name[])
{
    int menu_num = (int) cl;

    add_call_to_menu(&call_menu_lists[menu_num], call_menu_index,
		     number_of_calls[menu_num], name);
}


Private call_list_kind longest_title = call_list_empty;
Private int longest_title_length = 0;

/*
 * Create a menu containing number_of_calls[cl] items, which are the
 * items whose text strings were previously transmitted by the calls
 * to uims_add_call_to_menu.  Use the "menu_name" argument to create a
 * title line for the menu.  The string is in static storage.
 * 
 * This will be called once for each value in the enumeration call_list_kind.
 */
/* ARGSUSED */
extern void
uims_finish_call_menu(call_list_kind cl, char menu_name[])
{
    int name_len = strlen(menu_name);

    call_menu_names[cl] = menu_name;

    /* XXX - counting characters is not correct because the font
       need not be fixed width.  */
    if (name_len > longest_title_length) {
	longest_title_length = name_len;
	longest_title = cl;
    }
}

Private call_list_kind visible_call_menu = call_list_none;

/*
 * display the requested call menu on the screen
 */
Private void
set_call_menu(call_list_kind call_menu, call_list_kind title)
{
    int menu_num = (int) call_menu;
    int title_num = (int) title;

    XtVaSetValues(calltitle, XtNlabel, call_menu_names[title_num], NULL);
    XawListChange(callmenu, call_menu_lists[menu_num],
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
Private void
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

Private String empty_menu[] = {NULL};

/* The main program calls this after all the call menus have been created,
   after all calls to uims_add_call_to_menu and uims_finish_call_menu.
   This performs any final initialization required by the interface package.

   It must also perform any required setup of the concept menu.  The
   concepts are not presented procedurally.  Instead, they can be found
   in the external array concept_descriptor_table+general_concept_offset.
   The number of concepts in that list is general_concept_size.  For each
   i, the field concept_descriptor_table[i].name has the text that we
   should display for the user.
*/

extern void
uims_postinitialize(void)
{
    int i;

    /* initialize our special empty call menu */
    call_menu_lists[call_list_empty] = empty_menu;
    number_of_calls[call_list_empty] = 0;
    call_menu_names[call_list_empty] = "";

    /* fill in general concept menuu */
    for (i=0; i<general_concept_size; i++)
	add_call_to_menu(&concept_menu_list, i, general_concept_size,
			 concept_descriptor_table[i+general_concept_offset].name);

    concept_menu_len = general_concept_size;

    /*
     * Before realizing, fill everything up with its normal information
     * so it can be sized correctly.
     */
    XawListChange(cmdmenu, sd_resources.cmd_list, NUM_CMD_BUTTON_KINDS, 0, TRUE);
    XawListChange(conceptspecialmenu, concept_menu_strings, 0, 0, TRUE);
    XawListChange(conceptmenu, concept_menu_list, concept_menu_len, 0, TRUE);
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
}


Private void
switch_from_startup_mode(void)
{
    XawListChange(cmdmenu, sd_resources.cmd_list, NUM_CMD_BUTTON_KINDS, 0, FALSE);
    XtRemoveAllCallbacks(cmdmenu, XtNcallback);
    XtAddCallback(cmdmenu, XtNcallback,
		  command_or_menu_chosen, (XtPointer)ui_command_select);
    XawListChange(conceptspecialmenu, concept_menu_strings, 0, 0, TRUE);
    XawListChange(conceptmenu, concept_menu_list, concept_menu_len, 0, TRUE);
}

Private void
switch_to_resolve_mode(void)
{
    if (visible_mode == mode_startup)
	switch_from_startup_mode();
    XtUnmanageChild(callbox);
    XtManageChild(resolvewin);

    visible_mode = mode_resolve;
}

Private int visible_modifications = -1;

extern uims_reply
uims_get_command(mode_kind mode, call_list_kind *call_menu)
{
    try_again:

    /* Update the text area */
    XawTextEnableRedisplay(txtwin);

    if (visible_modifications != allowing_modifications) {
	XtVaSetValues(statuswin,
		      XtNlabel,
		      sd_resources.modifications_allowed[allowing_modifications],
		      NULL);
	visible_modifications = allowing_modifications;
    }

    switch (mode) {
      case mode_startup:
	if (visible_mode != mode_startup) {
	    XtUnmanageChild(resolvewin); /* managed at startup, too */
	    XtManageChild(callbox); /* nec if mode_resolve now */
	    XawListChange(cmdmenu, sd_resources.start_list,
			  NUM_START_SELECT_KINDS, 0, FALSE);
	    XawListChange(conceptspecialmenu, empty_menu, 0, 0, FALSE);
	    XawListChange(conceptmenu, empty_menu, 0, 0, FALSE);
	    set_call_menu (call_list_empty, call_list_empty);
	    XtRemoveAllCallbacks(cmdmenu, XtNcallback);
	    XtAddCallback(cmdmenu, XtNcallback,
			  command_or_menu_chosen, (XtPointer)ui_start_select);
	    visible_mode = mode_startup;
	}
	break;

      case mode_resolve:
	if (visible_mode != mode_resolve)
	    switch_to_resolve_mode();
	break;

      case mode_normal:
        if (allowing_modifications)
            *call_menu = call_list_any;

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
	if (visible_call_menu != *call_menu)
	    set_call_menu (*call_menu, *call_menu);
	break;
    case mode_none:
	/* this should never happen */
	break;
    }

    inside_what = inside_get_command;
    {
        int local_reply = read_user_gesture(xtcontext);

        if (local_reply == USER_GESTURE_NULL)
            goto try_again;
        else if (local_reply == ui_command_select) {
            if (uims_menu_index == SPECIAL_ALLOW_MODS) {
                /* Increment "allowing_modifications" up to a maximum of 2. */
                if (allowing_modifications != 2) allowing_modifications++;
                goto try_again;
            }
            else if (uims_menu_index == SPECIAL_ALLOW_ALL_CONCEPTS) {
                allowing_all_concepts = !allowing_all_concepts;
                /* ***** Maybe we should change visibility of off-level concepts at this point. */
                goto try_again;
            }
        }

        return local_reply;
    }
}


Private int
get_popup_string(Widget popup, Widget dialog, char dest[])
{
    int value;

    value = do_popup(popup);
    if (value == POPUP_ACCEPT_WITH_STRING)
	(void) strcpy(dest, XawDialogGetValueString(dialog));
    return value;
}


extern int
uims_do_comment_popup(char dest[])
{
    return get_popup_string(commentpopup, commentbox, dest);
}


extern int
uims_do_outfile_popup(char dest[])
{
    char outfile_question[150];

    (void) sprintf(outfile_question,
		   sd_resources.outfile_format, outfile_string);
    XtVaSetValues(outfilebox, XtNlabel, outfile_question, NULL);
    return get_popup_string(outfilepopup, outfilebox, dest);
}


extern int
uims_do_getout_popup(char dest[])
{
    return get_popup_string(getoutpopup, getoutbox, dest);
}


extern int
uims_do_neglect_popup(char dest[])
{
    return get_popup_string(neglectpopup, neglectbox, dest);
}


Private int
confirm(String question)
{
    XtVaSetValues(confirmlabel, XtNlabel, question, NULL);
    return do_popup(confirmpopup);
}


extern int
uims_do_abort_popup(void)
{
    return confirm(sd_resources.abort_query);
}


extern int
uims_do_modifier_popup(char callname[], modify_popup_kind kind)
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
     case modify_popup_only_scoot:
       line_format = sd_resources.modify_scoot_format;
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
Private void
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

static int first_reconcile_history;
static search_kind reconcile_goal;

/*
 * UIMS_BEGIN_SEARCH is called at the beginning of each search mode
 * command (resolve, reconcile, nice setup, do anything).
 */

extern void
uims_begin_search(search_kind goal)
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
 * if the reconcile history is written to a separate window, which is the
 * point of uims_begin_reconcile_history and uims_end_reconcile_history.
 */

/* ARGSUSED */
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

extern void
uims_update_resolve_menu(search_kind goal, int cur, int max, resolver_display_state state)
{
    char title[MAX_TEXT_LINE_LENGTH];

    create_resolve_menu_title(goal, cur, max, state, title);
    XtVaSetValues(resolvetitle, XtNlabel, title, NULL);
    switch_to_resolve_mode();
    update_display(resolvetitle);
}


Private int
choose_popup(String label, String names[])
{
    Dimension labelwidth, listwidth;
    Dimension listintwid;
    Dimension stdlistintwid = 4;

    XtVaSetValues(chooselabel, XtNlabel, label, NULL);
    /* we clobber this each time */
    XtVaSetValues(chooselist, XtNinternalWidth, stdlistintwid, NULL);

    XawListChange(chooselist, names, 0, 0, TRUE);

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

extern int
uims_do_selector_popup(void)
{
    /* We skip the zeroth selector, which is selector_uninitialized. */
    int t = choose_popup(sd_resources.selector_title, &selector_names[1]);
    if (t==0) return POPUP_DECLINE;
    return t;
}    


Private String quantifier_names[] = {
    " 1 ", " 2 ", " 3 ", " 4 ", " 5 ", NULL};

extern int
uims_do_quantifier_popup(void)
{
    int t = choose_popup(sd_resources.quantifier_title, quantifier_names);
    if (t==0) return POPUP_DECLINE;
    return t;
}


/* variables used by the next two routines */

Private XawTextPosition *line_indexes = NULL; /* end position of each line */
Private int line_count = 0;	/* size of line_indexes */
Private XawTextBlock text_block;

/*
 * add a line to the text output area.
 * the_line does not have the trailing Newline in it and
 * is volitile, so we must copy it if we need it to stay around.
 */
extern void
uims_add_new_line(char the_line[])
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
    line_indexes =
	get_more_mem(line_indexes, line_count * sizeof(XawTextPosition *));
}


extern void
uims_terminate(void)
{
    /* if uims_preinitialize was called, close down the window system */
    if (program_name)
	XtDestroyApplicationContext(xtcontext);
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
