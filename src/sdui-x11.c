#ifndef lint
static char *id = "@(#)sdui-x11.c      1.11    gildea@lcs.mit.edu  23 Nov 92";
static char *time_stamp = "sdui-x11.c Time-stamp: <92/11/22 20:51:39 gildea>";
#endif
/* 
 * sdui-x11.c - SD User Interface for X11
 * Copyright (c) 1990,1991,1992 Stephen Gildea and William B. Ackerman
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
 * For use with version 27 of the Sd program.
 *
 *  The version of this file is as shown immediately below.  This
 *  string gets displayed at program startup, as the "ui" part of
 *  the complete version.
 */

static char *sdui_x11_version = "1.11";

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
   uims_do_concept_popup
   uims_add_new_line
   uims_reduce_line_count
   uims_update_resolve_menu
   uims_terminate
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

static Widget toplevel, cmdmenu, conceptspecialmenu;
static Widget conceptpopup, conceptlist;
static Widget lview, callview, conceptmenu, callmenu;
static Widget callbox, calltitle;
static Widget statuswin, txtwin;
static Widget resolvewin, resolvetitle, resolvemenu;
static Widget confirmpopup, confirmlabel;
static Widget choosepopup, choosebox, chooselabel, chooselist;
static Widget commentpopup, commentbox, outfilepopup, outfilebox;
static Widget getoutpopup, getoutbox;
static Widget neglectpopup, neglectbox;

/*
 * The total version string looks something like
 * "1.4:db1.5:ui0.6X11"
 * We return the "0.6X11" part.
 */

static char version_mem[12];

extern char *
uims_version_string(void)
{
    (void) sprintf(version_mem, "%sX11", sdui_x11_version);
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

/* ARGSUSED */
static void
command_or_menu_chosen(Widget w, XtPointer client_data, XtPointer call_data)
{
    XawListReturnStruct *item = (XawListReturnStruct *) call_data;
    int menu_type = (int) client_data;

    unhighlight_lists();	/* do here in case spurious event */
    if (inside_what == inside_get_command) {
	uims_menu_index = item->list_index; /* extern var <- menu item no. */
	callback_return(menu_type); /* return which menu */
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
    String abort_query;
    String modify_format;
    String modify_tag_format;
    String modify_scoot_format;
    String modify_line_two;
    String modifications_allowed[3];
    String start_list[NUM_START_SELECT_KINDS];
    String cmd_list[NUM_COMMAND_KINDS];
    String resolve_list[NUM_RESOLVE_COMMAND_KINDS];
    String quantifier_title;
    String selector_title;
} SdResources;

static SdResources sd_resources;

/* General program resources */

#define MENU(name, loc, default) \
    {name, "Menu", XtRString, sizeof(String), \
     XtOffsetOf(SdResources, loc), XtRString, default}

static XtResource startup_resources[] = {
    MENU("exit", start_list[start_select_exit], "Exit from the program"),
    MENU("heads1P2P", start_list[start_select_h1p2p], "Heads 1P2P"),
    MENU("sides1P2P", start_list[start_select_s1p2p], "Sides 1P2P"),
    MENU("headsStart", start_list[start_select_heads_start], "Heads start"),
    MENU("sidesStart", start_list[start_select_sides_start], "Sides start"),
    MENU("asTheyAre", start_list[start_select_as_they_are], "Just as they are")
};

static XtResource command_resources[] = {
    MENU("exit", cmd_list[command_quit], "Exit the program"),
    MENU("undo", cmd_list[command_undo], "Undo last call"),
    MENU("abort", cmd_list[command_abort], "Abort this sequence"),
    MENU("mods", cmd_list[command_allow_modification], "Allow modifications"),
    MENU("comment", cmd_list[command_create_comment], "Insert a comment ..."),
    MENU("outfile", cmd_list[command_change_outfile], "Change output file ..."),
    MENU("getout", cmd_list[command_getout], "End this sequence ..."),
    MENU("neglect", cmd_list[command_neglect], "Show neglected calls ..."),
    MENU("savepic", cmd_list[command_save_pic], "Save picture"),
    MENU("resolve", cmd_list[command_resolve], "Resolve ..."),
    MENU("reconcile", cmd_list[command_reconcile], "Reconcile ..."),
    MENU("anything", cmd_list[command_anything], "Do anything ..."),
    MENU("nice", cmd_list[command_nice_setup], "Nice setup ...")
};

static XtResource resolve_resources[] = {
    MENU("abort", resolve_list[resolve_command_abort], "abort the search"),
    MENU("find", resolve_list[resolve_command_find_another], "find another"),
    MENU("next", resolve_list[resolve_command_goto_next], "go to next"),
    MENU("previous", resolve_list[resolve_command_goto_previous], "go to previous"),
    MENU("accept", resolve_list[resolve_command_accept], "ACCEPT current choice"),
    MENU("raiseRec", resolve_list[resolve_command_raise_rec_point], "raise reconcile point"),
    MENU("lowerRec", resolve_list[resolve_command_lower_rec_point], "lower reconcile point")
};

static XtResource enabledmods_resources[] = {
    MENU("none", modifications_allowed[0], "No call modifications enabled"),
    MENU("simple", modifications_allowed[1], "Simple modifications enabled for this call"),
    MENU("all", modifications_allowed[2], "All modifications enabled for this call")
};

static XtResource confirm_resources[] = {
    MENU("abort", abort_query, "Do you really want to abort this sequence?"),
    MENU("modifyFormat", modify_format, "The \"%s\" can be replaced."),
    MENU("modifyTagFormat", modify_tag_format, "The \"%s\" can be replaced with a tagging call."),
    MENU("modifyScootFormat", modify_scoot_format, "The \"%s\" can be replaced with a scoot/tag (chain thru) (and scatter)."),
    MENU("modifyLineTwo", modify_line_two, "Do you want to replace it?")
};

static XtResource choose_resources[] = {
    MENU("who", selector_title, "  Who?  "),
    MENU("howMany", quantifier_title, "How many?")
};

static XtResource top_level_resources[] = {
    MENU("sequenceFile", sequence, SEQUENCE_FILENAME),
    MENU("databaseFile", database, DATABASE_FILENAME),
};

static XrmOptionDescRec cmd_line_options[] = {
    {"-sequence", "*sequenceFile", XrmoptionSepArg, NULL},
    {"-db",       "*databaseFile", XrmoptionSepArg, NULL},
};

/* Fallback class resources */

static String fallback_resources[] = {
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
    "*outfile.label: Enter new name for sequence output file:",
    "*getout.label: Text to be placed at the beginning of this sequence:",
    "*neglect.label: Percentage (integer) of neglected calls:",
    "*abort.label: Abort",
    "*ok.label: Ok",
    "*abortGetout.label: Abort getout",
    "*noHeader.label: No header",
    "*useDefault.label: Use default",
    NULL};

static char *program_name = NULL;	/* argv[0]: our name */

static XtAppContext xtcontext;

/*
 * The main program calls this before doing anything else, so we can
 * read the command line arguments that are relevant to the X user
 * interface.  Note: If we are writing a call list, the program will
 * exit before doing anything else with the user interface, but this
 * must be made anyway.
 */
extern void
uims_process_command_line(int *argcp, char *argv[])
{
    program_name = argv[0];
    toplevel = XtAppInitialize(&xtcontext, "Sd",
			       cmd_line_options, XtNumber(cmd_line_options),
			       argcp, argv,
			       fallback_resources, NULL, 0);
    XtGetApplicationResources(toplevel, (XtPointer) &sd_resources,
			      top_level_resources, XtNumber(top_level_resources),
			      NULL, 0);
    strncpy(outfile_string, sd_resources.sequence, MAX_FILENAME_LENGTH);
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
    Widget box, leftarea, rightarea, startupmenu, infopanes, confirmbox;
    Widget cmdbox, speconsbox, conceptbox;
    XtTranslations list_trans, unmap_no_trans;
    Arg args[5];
    int n;

    XtOverrideTranslations(toplevel, XtParseTranslationTable(message_trans));

    list_trans = XtParseTranslationTable(list_translations);

    n = 0;
    XtSetArg(args[n], XtNorientation, XtEhorizontal); n++;
    box=XtCreateManagedWidget("frame", panedWidgetClass, toplevel, args, n);

    leftarea =
	XtCreateManagedWidget("leftmenus", panedWidgetClass, box, args, 0);

    rightarea =
	XtCreateManagedWidget("rightmenus", panedWidgetClass, box, args, 0);

    infopanes =
	XtCreateManagedWidget("infopanes", panedWidgetClass, box, args, 0);

    /* the sole purpose of this box is to keep the cmdmenu from resizing
       itself.  If the window manager resizes us at startup, sizing won't
       happen until after the window gets mapped, and thus the command menu
       would have the shorter startup list and be the wrong size. */
    cmdbox = XtCreateManagedWidget("cmdbox", panedWidgetClass, leftarea,
				   args, 0);

    cmdmenu = XtCreateManagedWidget("command", listWidgetClass, cmdbox,
				    args, 0);
    XtOverrideTranslations(cmdmenu, list_trans);
    XtGetApplicationResources(cmdmenu, (XtPointer) &sd_resources,
			      command_resources,
			      XtNumber(command_resources), NULL, 0);
    /* This widget is never realized.  It is just a place to hang
       resources off of. */
    startupmenu = XtCreateWidget("startup", labelWidgetClass, cmdbox,
				 args, 0);
    XtGetApplicationResources(startupmenu, (XtPointer) &sd_resources,
			      startup_resources,
			      XtNumber(startup_resources), NULL, 0);

    /* Keeps the conceptspecialmenu from resizing itself.  See cmdbox above. */
    speconsbox = XtCreateManagedWidget("speconsbx", panedWidgetClass, leftarea,
				       args, 0);
    conceptspecialmenu =
	XtCreateManagedWidget("conceptspecial", listWidgetClass, speconsbox,
			      args, 0);
    XtAddCallback(conceptspecialmenu, XtNcallback,
		  command_or_menu_chosen, (XtPointer)ui_special_concept);
    XtOverrideTranslations(conceptspecialmenu, list_trans);

    callbox = XtCreateManagedWidget("callbox", panedWidgetClass,
				       rightarea, args, 0);

    calltitle = XtCreateManagedWidget("calltitle", labelWidgetClass,
					 callbox, args, 0);

    n = 0;
    /* Viewports may have vertical scrollbar, and it must be visible */
    XtSetArg(args[n], XtNallowVert, True); n++;
    XtSetArg(args[n], XtNforceBars, True); n++;
    lview =
	XtCreateManagedWidget("lma", viewportWidgetClass, leftarea, args, n);
    callview =
	XtCreateManagedWidget("rma", viewportWidgetClass, callbox, args, n);

    n = 0;
    XtSetArg(args[n], XtNdefaultColumns, 1); n++;
    conceptmenu =
	XtCreateManagedWidget("conceptmenu", listWidgetClass, lview, args, n);
    XtAddCallback(conceptmenu, XtNcallback,
		  command_or_menu_chosen, (XtPointer)ui_concept_select);
    XtOverrideTranslations(conceptmenu, list_trans);

    callmenu =
	XtCreateManagedWidget("callmenu", listWidgetClass, callview, args, n);
    XtAddCallback(callmenu, XtNcallback,
		  command_or_menu_chosen, (XtPointer)ui_call_select);
    XtOverrideTranslations(callmenu, list_trans);

    /* make it managed now so sizing gets done */
    resolvewin = XtCreateManagedWidget("resolvebox", panedWidgetClass,
				       rightarea, args, 0);

    resolvetitle = XtCreateManagedWidget("resolvetitle", labelWidgetClass,
					 resolvewin, args, 0);

    resolvemenu = XtCreateManagedWidget("resolve", listWidgetClass,
					resolvewin, args, 0);
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
    XtSetArg(args[0], XtNlabel,
	     "XX-123456789-123456789-123456789-123456789-123456789-123456789");
    statuswin = XtCreateManagedWidget("enabledmods", labelWidgetClass,
				      infopanes, args, 1);
    XtGetApplicationResources(statuswin, (XtPointer) &sd_resources,
			      enabledmods_resources,
			      XtNumber(enabledmods_resources), NULL, 0);

    XtSetArg(args[0], XtNstring, ""); /* set to known string so can append */
    txtwin = XtCreateManagedWidget("text", asciiTextWidgetClass, infopanes,
				   args, 1);

    /* confirmation popup */

    XtAppAddActions(xtcontext, actionTable, XtNumber(actionTable));

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    confirmpopup = XtCreatePopupShell("confirmpopup", transientShellWidgetClass,
				      toplevel, args, n);
    XtGetApplicationResources(confirmpopup, (XtPointer) &sd_resources,
			      confirm_resources, XtNumber(confirm_resources),
			      NULL, 0);
    XtOverrideTranslations(confirmpopup,
			   XtParseTranslationTable(confirm_translations));

    /* this creates a margin which makes the popup prettier */
    confirmbox = XtCreateManagedWidget("confirm", boxWidgetClass,
				       confirmpopup, args, 0);

    confirmlabel = XtCreateManagedWidget("label", labelWidgetClass,
					 confirmbox, args, 0);

    XtRealizeWidget(confirmpopup); /* makes XtPopup faster to do this now */

    /* menu choose popup */

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    choosepopup = XtCreatePopupShell("selector", transientShellWidgetClass,
				     toplevel, args, n);
    XtOverrideTranslations(choosepopup,
			   XtParseTranslationTable(choose_translations));

    choosebox = XtCreateManagedWidget("choose", boxWidgetClass,
				      choosepopup, args, 0);

    chooselabel = XtCreateManagedWidget("label", labelWidgetClass,
					 choosebox, args, 0);
    XtGetApplicationResources(chooselabel, (XtPointer) &sd_resources,
			      choose_resources, XtNumber(choose_resources),
			      NULL, 0);

    n = 0;
    XtSetArg(args[n], XtNcolumnSpacing, 0); n++;
    chooselist = XtCreateManagedWidget("items", listWidgetClass,
				       choosebox, args, n);
    XtAddCallback(chooselist, XtNcallback, choose_pick, (XtPointer)NULL);
    XtOverrideTranslations(chooselist, list_trans);

    XtRealizeWidget(choosepopup);

    /* comment popup */

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    commentpopup = XtCreatePopupShell("commentpopup", transientShellWidgetClass,
				      toplevel, args, n);
    unmap_no_trans = XtParseTranslationTable(unmap_no_translation);
    XtOverrideTranslations(commentpopup, unmap_no_trans);

    n = 0;
    XtSetArg(args[n], XtNvalue, ""); n++; /* create an empty value area */
    commentbox = XtCreateManagedWidget("comment", dialogWidgetClass,
				       commentpopup, args, n);

    XawDialogAddButton(commentbox, "abort", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(commentbox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(commentpopup); /* makes XtPopup faster to do this now */


    /* outfile popup */

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    outfilepopup = XtCreatePopupShell("outfilepopup", transientShellWidgetClass,
				      toplevel, args, n);
    XtOverrideTranslations(outfilepopup, unmap_no_trans);

    n = 0;
    XtSetArg(args[n], XtNvalue, ""); n++; /* create an empty value area */
    outfilebox = XtCreateManagedWidget("outfile", dialogWidgetClass,
				       outfilepopup, args, n);

    XawDialogAddButton(outfilebox, "abort", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(outfilebox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(outfilepopup); /* makes XtPopup faster to do this now */


    /* getout popup */

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    getoutpopup = XtCreatePopupShell("getoutpopup", transientShellWidgetClass,
				      toplevel, args, n);
    XtOverrideTranslations(getoutpopup, unmap_no_trans);

    n = 0;
    XtSetArg(args[n], XtNvalue, ""); n++; /* create an empty value area */
    getoutbox = XtCreateManagedWidget("getout", dialogWidgetClass,
				       getoutpopup, args, n);

    XawDialogAddButton(getoutbox, "abortGetout", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(getoutbox, "noHeader", dialog_callback,
		       (XtPointer)POPUP_ACCEPT);
    XawDialogAddButton(getoutbox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(getoutpopup); /* makes XtPopup faster to do this now */

    /* neglect popup */

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    neglectpopup = XtCreatePopupShell("neglectpopup", transientShellWidgetClass,
				      toplevel, args, n);
    XtOverrideTranslations(neglectpopup, unmap_no_trans);

    n = 0;
    XtSetArg(args[n], XtNvalue, ""); n++; /* create an empty value area */
    neglectbox = XtCreateManagedWidget("neglect", dialogWidgetClass,
				       neglectpopup, args, n);

    XawDialogAddButton(neglectbox, "useDefault", dialog_callback,
		       (XtPointer)POPUP_DECLINE);
    XawDialogAddButton(neglectbox, "ok", dialog_callback,
		       (XtPointer)POPUP_ACCEPT_WITH_STRING);

    XtRealizeWidget(neglectpopup); /* makes XtPopup faster to do this now */


    /* concept popup */

    n = 0;
    XtSetArg(args[n], XtNallowShellResize, True); n++;
    conceptpopup = XtCreatePopupShell("conceptpopup",
				      transientShellWidgetClass,
				      toplevel, args, n);
    XtOverrideTranslations(conceptpopup,
			   XtParseTranslationTable(choose_translations));

    /* this creates a margin which makes it easier to mouse the edge items */
    conceptbox = XtCreateManagedWidget("concept", boxWidgetClass,
				       conceptpopup, args, 0);

    n = 0;
    conceptlist = XtCreateManagedWidget("items", listWidgetClass,
					conceptbox, args, n);
    XtAddCallback(conceptlist, XtNcallback, choose_pick, (XtPointer)NULL);
    XtOverrideTranslations(conceptlist, list_trans);

    XtRealizeWidget(conceptpopup);
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
    Arg args[2];
    Widget scrollbar = XtNameToWidget(vw, XtEvertical);
    Dimension scrollwidth, scrollborder;

    if (!scrollbar) {
	(void) fprintf(stderr, "%s warning: viewport %s has no scrollbar, will not widen\n",
		       program_name, XtName(vw));
	return;
    }

    XtSetArg(args[0], XtNwidth, &scrollwidth);
    XtSetArg(args[1], XtNborderWidth, &scrollborder);
    XtGetValues(scrollbar, args, 2);

    XtSetArg(args[0], XtNcolumnSpacing, scrollwidth+scrollborder);
    XtSetValues(childw, args, 1);
}

static void
add_call_to_menu(String **menu, int call_menu_index, int menu_size, char callname[])
{
    if (call_menu_index == 0) {	/* first item in this menu; set it up */
	*menu = (String *)XtMalloc((unsigned)(menu_size+1) * sizeof(String *));
    }

    (*menu)[call_menu_index] = callname;
}

static String *concept_menu_list;
static int concept_menu_len;

static String *call_menu_lists[NUM_CALL_LIST_KINDS];
static String call_menu_names[NUM_CALL_LIST_KINDS];

static call_list_kind visible_call_menu = call_list_none;

/*
 * display the requested call menu on the screen
 */
static void
set_call_menu(call_list_kind call_menu, call_list_kind title)
{
    int menu_num = (int) call_menu;
    int title_num = (int) title;
    Arg args[1];

    XtSetArg(args[0], XtNlabel, call_menu_names[title_num]);
    XtSetValues(calltitle, args, 1);
    XawListChange(callmenu, call_menu_lists[menu_num],
		  number_of_calls[menu_num], 0, TRUE);
    visible_call_menu = call_menu==title ? call_menu : call_list_none;
}

static String empty_menu[] = {NULL};
static call_list_kind longest_title = call_list_empty;
static int longest_title_length = 0;

/* The main program calls this after all the call menus have been created,
   after all calls to uims_add_call_to_menu and uims_finish_call_menu.
   This performs any final initialization required by the interface package.
   It must also perform any required setup of the concept menu.  The concepts
   are not presented procedurally.  Instead, they can be found in the external
   array concept_descriptor_table.  The number of concepts in that list is
   in our argument.  For each i, the field concept_descriptor_table[i].name
   has the text that we should display for the user.
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
    XawListChange(cmdmenu, sd_resources.cmd_list, NUM_COMMAND_KINDS, 0, TRUE);
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


static void
switch_from_startup_mode(void)
{
    XawListChange(cmdmenu, sd_resources.cmd_list, NUM_COMMAND_KINDS, 0, FALSE);
    XtRemoveAllCallbacks(cmdmenu, XtNcallback);
    XtAddCallback(cmdmenu, XtNcallback,
		  command_or_menu_chosen, (XtPointer)ui_command_select);
    XawListChange(conceptspecialmenu, concept_menu_strings, 0, 0, TRUE);
    XawListChange(conceptmenu, concept_menu_list, concept_menu_len, 0, TRUE);
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

static int visible_modifications = -1;

extern uims_reply
uims_get_command(mode_kind mode, call_list_kind call_menu, int modifications_flag)
{
    Arg args[1];

    /* Update the text area */
    XawTextEnableRedisplay(txtwin);

    if (visible_modifications != modifications_flag) {
	XtSetArg(args[0], XtNlabel,
		 sd_resources.modifications_allowed[modifications_flag]);
	XtSetValues(statuswin, args, 1);
	visible_modifications = modifications_flag;
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
	if (visible_call_menu != call_menu)
	    set_call_menu (call_menu, call_menu);
	break;
    case mode_none:
	/* this should never happen */
	break;
    }

    inside_what = inside_get_command;
    return (uims_reply)read_user_gesture(xtcontext);
}


static void
position_near_mouse(Widget popupshell)
{
    Arg args[3];
    Window root, child;
    int n;
    int x, y, max_x, max_y;
    int winx, winy;
    Dimension width, height, border;
    unsigned int mask;

    /* much of this is copied from CenterWidgetOnPoint in Xaw/TextPop.c,
       which should be a publicly-callable function anyway. */

    n = 0;
    XtSetArg(args[n], XtNwidth, &width); n++;
    XtSetArg(args[n], XtNheight, &height); n++;
    XtSetArg(args[n], XtNborderWidth, &border); n++;
    XtGetValues(popupshell, args, n);

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

    n = 0;
    XtSetArg(args[n], XtNx, (Position)x); n++;
    XtSetArg(args[n], XtNy, (Position)y); n++;
    XtSetValues(popupshell, args, n);
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

static int
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


static int
confirm(String question)
{
    Arg args[1];

    XtSetArg(args[0], XtNlabel, question);
    XtSetValues(confirmlabel, args, 1);
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

extern void
uims_update_resolve_menu(char *title)
{
    Arg args[1];

    XtSetArg(args[0], XtNlabel, title);
    XtSetValues(resolvetitle, args, 1);
    switch_to_resolve_mode();
    update_display(resolvetitle);
}


static int
choose_popup(String label, String names[])
{
    Arg args[2];
    Dimension labelwidth, listwidth;
    Dimension listintwid;
    Dimension stdlistintwid = 4;

    XtSetArg(args[0], XtNlabel, label);
    XtSetValues(chooselabel, args, 1);
    /* we clobber this each time */
    XtSetArg(args[0], XtNinternalWidth, stdlistintwid);
    XtSetValues(chooselist, args, 1);

    XawListChange(chooselist, names, 0, 0, TRUE);

    /* set the width of the box to the width of the widest child */

    XtSetArg(args[0], XtNwidth, &labelwidth);
    XtGetValues(chooselabel, args, 1);

    XtSetArg(args[0], XtNwidth, &listwidth);
    XtSetArg(args[1], XtNinternalWidth, &listintwid);
    XtGetValues(chooselist, args, 2);

    if (labelwidth > listwidth) {
	/* Make the list be centered.  Works because columnSpacing is 0 */
	XtSetArg(args[0], XtNinternalWidth,
		 listintwid + (int)(labelwidth-listwidth)/2);
	XtSetValues(chooselist, args, 1);
    } else {
	XtSetArg(args[0], XtNwidth, listwidth);
	/* must also set label again to force box to redo layout */
	XtSetArg(args[1], XtNlabel, label);
	XtSetValues(chooselabel, args, 2);
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


static String quantifier_names[] = {
    " 1 ", " 2 ", " 3 ", " 4 ", " 5 ", NULL};

extern int
uims_do_quantifier_popup(void)
{
    int t = choose_popup(sd_resources.quantifier_title, quantifier_names);
    if (t==0) return POPUP_DECLINE;
    return t;
}


static String empty_string = "";
static String *concept_popup_list = NULL;

extern int
uims_do_concept_popup(int kind)
{
    unsigned int i;
    unsigned int row, column;
    unsigned int maxrow, maxcolumn, entries;
    Arg args[1];
    int value;

    /* determine menu size */
    for (maxcolumn=0; concept_size_tables[kind][maxcolumn]>=0; maxcolumn++)
	;
    maxrow = 0;
    for (i=0; i<maxcolumn; i++)
	if (maxrow < concept_size_tables[kind][i])
	    maxrow = concept_size_tables[kind][i];
    entries = maxcolumn*maxrow;

    concept_popup_list =
	(String *) XtRealloc((char *)concept_popup_list,
			     entries*sizeof(String *));

    /* fill in the entries */
    i=0;
    for (row=0; row<maxrow; row++) {
	for (column=0; column<maxcolumn; column++) {
	    if (row < concept_size_tables[kind][column])
		concept_popup_list[i] = concept_descriptor_table
		    [ concept_offset_tables[kind][column]+row ].name;
	    else
		concept_popup_list[i] = empty_string;
	    i++;
	}
    }

    XtSetArg(args[0], XtNdefaultColumns, maxcolumn);
    XtSetValues(conceptlist, args, 1);
    XawListChange(conceptlist, concept_popup_list, entries, 0, TRUE);
    value = do_popup(conceptpopup);

    if (value) {
	value--;
	/* row and column are 0-based */
	row = value/maxcolumn;
	column = value%maxcolumn;
	if (row < concept_size_tables[kind][column]) /* not off the end? */
	    return (column<<8) + row + 1;
    }
    return POPUP_DECLINE;
}

/* variables used by the next two routines */

static XawTextPosition *line_indexes = NULL; /* end position of each line */
static int line_count = 0;	/* size of line_indexes */
static XawTextBlock text_block;

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
    Arg args[1];

    XawTextDisableRedisplay(txtwin);

    line_len = strlen(the_line);
    new_size = prev_size + line_len;
    line_count++;
    line_indexes =
	(XawTextPosition *)XtRealloc((char *)line_indexes,
				     line_count * sizeof(XawTextPosition*));
    line_indexes[line_count-1] = new_size + 1; /* 1 more for the newline */

    /* make text widget writable */
    XtSetArg(args[0], XtNeditType, XawtextEdit);
    XtSetValues(txtwin, args, 1);
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
    XtSetArg(args[0], XtNeditType, XawtextRead);
    XtSetValues(txtwin, args, 1);
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
    Arg args[1];

    if (n >= line_count)
	return;			/* should never happen */
    new_size = n==0 ? 0 : line_indexes[n-1];

    XawTextDisableRedisplay(txtwin);

    /* make text widget writable */
    XtSetArg(args[0], XtNeditType, XawtextEdit);
    XtSetValues(txtwin, args, 1);
    /* delete the text */
    text_block.length = 0;
    status = XawTextReplace(txtwin, new_size, prev_size, &text_block);
    if (status != XawEditDone)
	(void) fprintf(stderr, "%s warning: text delete returned %d\n",
		       program_name, status);
    /* make text widget read-only again */
    XtSetArg(args[0], XtNeditType, XawtextRead);
    XtSetValues(txtwin, args, 1);

    line_count = n;
    line_indexes =
	(XawTextPosition *)XtRealloc((char *)line_indexes,
				     line_count * sizeof(XawTextPosition *));
}


extern void
uims_terminate(void)
{
    /* if uims_preinitialize was called, close down the window system */
    if (program_name)
	XtDestroyApplicationContext(xtcontext);
}
