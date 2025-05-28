/*
 *  macstuff.c - guts of Macintosh UI
 *
 *  Copyright (C) 1993 Alan Snyder
 *
 *  Permission to use, copy, modify, and distribute this software for
 *  any purpose is hereby granted without fee, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *  The author makes no representations about the suitability of this
 *  software for any purpose.  It is provided "as is" WITHOUT ANY
 *  WARRANTY, without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  
 *
 */

#include "macguts.h"
#include <string.h> /* for strcpy, strlen, strcat */
#include <stdio.h> /* for sprintf */
#include <Traps.h>

char                *level_name;
Cursor              edit_cursor;
Cursor              wait_cursor;
/* sue: split into 2 variables */
long_boolean        output_file_save_ok = FALSE;
long_boolean        output_file_append_ok = FALSE;
long_boolean        dirty;                /* current sequence needs saving */
OutputMode          output_mode;
ControlHandle       output_default_control;
long_boolean        reconciling;
long_boolean        writing_sequence;

static match_result menu_match;           /* for menu commands */
static long_boolean building_reconcile_sequence;
static search_kind  resolve_goal;         /* which resolve command is it */
static int          resolve_solution;     /* index of solution displayed (0 if none) */
static int          resolve_max_solution; /* highest solution index (0 if none) */
static int          reconcile_point;      /* index of reconcile point */
       int          reconcile_max_point;  /* highest reconcile point index */
static int          recpoint_target;      /* set by clicking on recpoint window */
static int          recpoint_initialized; /* TRUE => recpoint window has been filled */
static int          pick_another_index;   /* which string is in "pick another" button */

static WindowPtr    hintsWindow;
static Window       hintsW;
static DialogPtr    aboutDialog;
static Window       aboutW;
static DialogPtr    readingDialog;
static Window       readingW;
static int          db_tick_max;
static int          db_tick;

extern WindowPtr matchWindow;
extern WindowPtr recpointWindow;

/* Controls */

const int FirstControlID = 160;
enum { NumControls = 16 };
const int NumResolveControls = 7;
const int NumStartupControls = 6;

enum {
    resolveAccept,
    resolveCancel,
    resolveAnother,
    resolveNext,
    resolvePrevious,
    dummycontrol0,
    dummycontrol1,
    stringAccept,
    stringCancel,
    callAccept,
    startupHeads,
    startupSides,
    startupHeadsL1p,
    startupSidesL1p,
    startupAsTheyAre,
    startupQuit
};

ControlHandle   controls[NumControls];
int             control_enabled[NumControls];

/* "Save sequence?" alert items */

enum {
    saveSave = 1,
    saveCancel,
    saveDiscard
};

/* static functions */

static OutputMode change_output_mode(OutputMode new_mode);

static void hide_resolve_controls(void);
static void show_resolve_controls(void);
static void hide_startup_controls(void);
static void show_startup_controls(void);
static void hide_string_controls(void);
static void show_string_controls(void);
static void hide_call_controls(void);
static void show_call_controls(void);
static void process_controls(void);
static void update_find_another_button(void);

static void about_update(Window *wp);
static void about_mouse_down(Window *wp, EventRecord *e);
static void about_key(Window *wp, short modifiers, char ch);
static char *append_message(char *msgp, int strings_id, int string_index, char *s);

static void hints_update(Window *wp);

static void init_cursors(void);

/*
 *  mac_initialize
 *
 */

void
mac_initialize()
{
    InitGraf(&thePort);
    InitFonts();
    FlushEvents(everyEvent, 0);
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(0L);
    InitCursor();
    MaxApplZone();
    init_cursors();
    init_menus();
    init_environment();
    window_manager_initialize();
}

/*
 *  change_output_mode
 *
 */

static OutputMode
change_output_mode(OutputMode new_mode)
{
    OutputMode old_mode;

    old_mode = output_mode;
    if (output_mode == new_mode) {
        return output_mode;
    }
    erase_input_area();
    output_default_control = NULL;
    switch(output_mode) {
        case StartupMode:
            hide_startup_controls();
            break;
        case CallMode:
            hide_call_controls();
            match_invalidate();
            display_update(); /* refresh exposed screen area sooner */
            break;
        case StringInputMode:
            hide_string_controls();
            break;
        case NoOutputMode:
            break;
        case ResolveMode:
            set_status("");
            hide_resolve_controls();
            recpoint_invalidate();
            break;
    }
    output_mode = new_mode;
    adjust_menus();
    switch(output_mode) {
        case StartupMode:
            output_default_control = controls[startupHeads];
            show_startup_controls();
            output_draw_button_frames();
            break;
        case CallMode:
            output_default_control = controls[callAccept];
            restore_input_area();
            show_call_controls();
            output_draw_button_frames();
            break;
        case StringInputMode:
            output_default_control = controls[stringAccept];
            restore_input_area();
            show_string_controls();
            break;
        case ResolveMode:
            output_default_control = controls[resolveAccept];
            show_resolve_controls();
            output_draw_button_frames();
            activate_controls(FALSE);
            busy_cursor();
            break;
    }
    return old_mode;
}

/*
 *  get_startup_command
 *
 */

int
get_startup_command(void)
{
    short hit;
    WindowPtr w;
    char buf[200];

    label_clear();
    if (myWindow == NULL) {
        main_window_setup();
        window_select((Window *)&outputTW);
    }
    /* sue: use new variable name.  *Don't* reset output_file_append_ok here,
       since we want that preserved from one sequence to the next. */
    output_file_save_ok = FALSE;
    dirty = FALSE;
    text_output_trim(0);
    typescript_flush();
    SetWTitle(myWindow, "\puntitled");
    change_output_mode(StartupMode);
    GetIndString(buf, MessagesStrings, StartSequenceString);
    input_set_prompt(PtoCstr(buf), "");
    process_controls();
    change_output_mode(CallMode);
    uims_menu_index = menu_match.index;
    return menu_match.kind;
}

/*
 **********************************************************************
 *  MAIN-WINDOW CONTROLS
 **********************************************************************
 */

/*
 *  create_controls
 *
 */

void
create_controls(void)
{
    int i;

    for (i=0; i<NumControls; ++i) {
        controls[i] = GetNewControl(FirstControlID+i, myWindow);
        control_enabled[i] = 1;
    }

    SetCRefCon(controls[resolveAccept], resolve_command_accept);
    SetCRefCon(controls[resolveCancel], resolve_command_abort);
    SetCRefCon(controls[resolveAnother], resolve_command_find_another);
    SetCRefCon(controls[resolveNext], resolve_command_goto_next);
    SetCRefCon(controls[resolvePrevious], resolve_command_goto_previous);
    SetCRefCon(controls[startupHeads], start_select_heads_start);
    SetCRefCon(controls[startupSides], start_select_sides_start);
    SetCRefCon(controls[startupHeadsL1p], start_select_h1p2p);
    SetCRefCon(controls[startupSidesL1p], start_select_s1p2p);
    SetCRefCon(controls[startupAsTheyAre], start_select_as_they_are);
    SetCRefCon(controls[startupQuit], start_select_exit);
    SetCRefCon(controls[callAccept], specialAccept);
    SetCRefCon(controls[stringAccept], specialAccept);
    SetCRefCon(controls[stringCancel], specialCancel);
}

/*
 *  move_controls
 *
 */

void
move_controls(int offset)
{
    int top, left;
    int i;
    ControlHandle h;

    for (i=0; i<NumControls; ++i) {
        h = controls[i];
        if (h) {
            top = (**h).contrlRect.top;
            left = (**h).contrlRect.left;
            MoveControl(h, left, top+offset);
        }
    }        
}

/*
 *  activate_controls
 *
 */

void
activate_controls(long_boolean activate)
{
    int i, val;
    ControlHandle h;

    val = activate ? 0 : 255;
    for (i=0; i<NumControls; ++i) {
        if (control_enabled[i]) {
            h = controls[i];
            if (h) {
                HiliteControl(h, val);
            }
        }
    }        
    output_draw_button_frames();
}

/*
 *  enable_control
 *
 */

static void
enable_control(int i, int on)
{
    if (control_enabled[i] != on) {
        control_enabled[i] = on;
        HiliteControl(controls[i], on ? 0 : 255);
        if (controls[i]==output_default_control)
            output_draw_button_frames();
    }
}

/*
 *  show_controls
 *
 */

static void
show_controls(int first, int last)
{
    int i;
    
    for (i=first; i<=last; ++i) {
        ShowControl(controls[i]);
    }        
}

/*
 *  hide_controls
 *
 */

static void
hide_controls(int first, int last)
{
    int i;
    ControlHandle h;
    
    for (i=first; i<=last; ++i) {
        HideControl(controls[i]);
    }
}

/*
 *  show_resolve_controls
 *
 */

static void
show_resolve_controls(void)
{
    show_controls(resolveAccept, resolvePrevious);
}

/*
 *  hide_resolve_controls
 *
 */

static void
hide_resolve_controls(void)
{
    hide_controls(resolveAccept, resolvePrevious);
}

/*
 *  show_startup_controls
 *
 */

static void
show_startup_controls(void)
{
    show_controls(startupHeads, startupQuit);
}

/*
 *  hide_startup_controls
 *
 */

static void
hide_startup_controls(void)
{
    hide_controls(startupHeads, startupQuit);
}

/*
 *  show_string_controls
 *
 */

static void
show_string_controls(void)
{
    show_controls(stringAccept, stringCancel);
}

/*
 *  hide_string_controls
 *
 */

static void
hide_string_controls(void)
{
    hide_controls(stringAccept, stringCancel);
}

/*
 *  show_call_controls
 *
 */

void
show_call_controls(void)
{
    show_controls(callAccept, callAccept);
}

/*
 *  hide_call_controls
 *
 */

void
hide_call_controls(void)
{
    hide_controls(callAccept, callAccept);
}

/*
 *  output_draw_button_frames
 *
 */

void
output_draw_button_frames(void)
{
    if (output_default_control != NULL) {
        draw_frame_around_button(output_default_control);
    }
}

/*
 *  process_controls
 *
 */

static void
process_controls(void)
{
    arrow_cursor();
    activate_controls(TRUE);
    menu_match.valid = FALSE;
    while (!menu_match.valid) {
        process_next_event();
    }
}

/*
 *  flash_control
 *
 */

void
flash_control(ControlHandle h, int part_code)
{
    long tickcount;

    HiliteControl(h, part_code);
    tickcount = TickCount() + (GetDblTime() / 4);
    while (TickCount() < tickcount);
    HiliteControl(h, 0);
}

/*
 **********************************************************************
 *  Search Commands (Resolve, Reconcile, Nice Setup, Do Anything)
 **********************************************************************
 */

/*
 *  UIMS_BEGIN_SEARCH is called at the beginning of each search mode
 *  command (resolve, reconcile, nice setup, do anything).
 *
 */

void
uims_begin_search(search_kind goal)
{
    char buf[100];

    reconciling = (goal == search_reconcile);
    resolve_goal = goal;
    reconcile_point = -1;
    reconcile_max_point = -1;
    recpoint_target = -1;
    recpoint_initialized = FALSE; /* first recpoint history is special */
    change_output_mode(ResolveMode);
    GetIndString(buf, SearchGoalStrings, goal+1);
    input_set_prompt(PtoCstr(buf), "");
}

/*
 *  UIMS_BEGIN_RECONCILE_HISTORY is called at the beginning of a reconcile,
 *  after UIMS_BEGIN_SEARCH, and whenever the current reconcile point
 *  changes.  CURRENTPOINT is the current reconcile point and MAXPOINT is
 *  the maximum possible reconcile point.  This call is followed by calls to
 *  UIMS_REDUCE_LINE_COUNT (text_output_trim) and UIMS_ADD_NEW_LINE
 *  (text_output_add_line) that display the current sequence with the
 *  reconcile point indicated.  These calls are followed by a call to
 *  UIMS_END_RECONCILE_HISTORY.  Return TRUE to cause sd to forget its
 *  cached history output.
 *
 */

int
uims_begin_reconcile_history(int currentpoint, int maxpoint)
{
    reconcile_point = currentpoint;
    reconcile_max_point = maxpoint;
    building_reconcile_sequence = TRUE;
    if (!recpoint_initialized) {
        /* first call for this reconciliation */
        recpoint_target = reconcile_point;
        recpoint_begin();
    }
    return TRUE;
}

/*
 *  uims_end_reconcile_history
 *
 *  Return TRUE to cause sd to forget its cached history output.
 *
 */

int
uims_end_reconcile_history(void)
{
    building_reconcile_sequence = FALSE;
    if (!recpoint_initialized) {
        recpoint_initialized = TRUE;
        recpoint_end();
    }
    return TRUE;
}

/*
 *  SET_RECPOINT_TARGET is called by a mouse event handler.
 *
 */

void
set_recpoint_target(int n)
{
    if ((n >= 0) && (n <= reconcile_max_point)) {
        recpoint_target = n;
        menu_match.valid = (recpoint_target != reconcile_point);
        /* causes process_controls to exit */
    }
}

/*
 *  get_resolve_command
 *
 */

uims_reply
get_resolve_command(void)
{
    if (recpoint_target == reconcile_point) {
        /* false when recpoint is being updated from mouse events */
        typescript_flush();
        process_controls();
    }
    if (reconcile_point < recpoint_target) {
        uims_menu_index = resolve_command_raise_rec_point;
        return ui_resolve_select;
    }
    if (reconcile_point > recpoint_target) {
        uims_menu_index = resolve_command_lower_rec_point;
        return ui_resolve_select;
    }
    uims_menu_index = menu_match.index;
    if ((uims_menu_index == resolve_command_find_another)
                && (resolve_goal != search_anything)) {
        activate_controls(FALSE);
        busy_cursor();
    }
    if (uims_menu_index == resolve_command_accept) {
        dirty = TRUE;
    }
    return menu_match.kind;
}

/*
 *  uims_update_resolve_menu
 *
 */

void
uims_update_resolve_menu(search_kind goal, int m, int n,
                         resolver_display_state state)
{
    char buf[100], *status;

    resolve_solution = m;
    resolve_max_solution = n; /* can affect button label */
    
    /*
     *  The purpose of the following conditional is to avoid unnecessary
     *  flickering of the "Find Another" button when starting a
     *  search.  There is no point in distinguishing between "Find A Solution"
     *  and "Find Another Solution" until the search has completed and we
     *  know how many solutions there are.  However, if the string is
     *  "Pick Another" (left over from a previous "Pick Random Call") or
     *  is uninitialized, then
     *  we should change it at the beginning of the search, even if it might
     *  change again after the search is complete.
     *
     */

    if ((state != resolver_display_searching) ||
         ((pick_another_index != FindSolutionString) &&
          (pick_another_index != FindAnotherString))) {
        update_find_another_button(); /* button label can change */
    }

    switch(state) {
        case resolver_display_searching:
            status = "searching...";
            break;
        case resolver_display_failed:
            status = "search failed";
            break;
        case resolver_display_ok:
            status = "";
            break;
        default:
            status = "unknown state";
            break;
    }

    buf[0] = 0;
    if (n > 0) {
        sprintf(buf, "%d out of %d", m, n);
    }

    set_status(status);
    set_menu_name(buf);
    enable_control(resolvePrevious, (resolve_solution > 1));
    enable_control(resolveNext,     (resolve_solution < resolve_max_solution));
}

/*
 *  update_find_another_button
 *
 */

static void
update_find_another_button(void)
{
    int index;

    if (resolve_goal == search_anything) {
        index = PickAnotherString;
    }
    else if (resolve_max_solution == 0) {
        index = FindSolutionString;
    }
    else {
        index = FindAnotherString;
    }
    if (index != pick_another_index) {
        Str255 buf;
        GetIndString(buf, MessagesStrings, index);
        SetCTitle(controls[resolveAnother], buf);
        pick_another_index = index;
    }
}

/*
 **********************************************************************
 *  KEYBOARD INPUT
 **********************************************************************
 */

/*
 *  input_clear
 *
 */

void
input_clear(void)
{
    change_output_mode(CallMode);
    typein_clear();
    typescript_flush();
}

/*
 *  input_stuff
 *
 */

void
input_stuff(char *s, int len)
{
    if (output_mode == CallMode) {
        typein_stuff(s, len);
    }
}

/*
 *  input_accept
 *
 */

void
input_accept(void)
{
    if (output_mode == CallMode) {
        /* simulate hitting of "Accept" button */
        stuff_command(-1, specialAccept);
        flash_control(controls[callAccept], inButton);
    }
    else {
        SysBeep(30);
    }
}

/*
 *  input_set_prompt
 *
 */

void
input_set_prompt(char *prompt, char *menu_name)
{
    typein_set_prompt(prompt);
    set_menu_name(menu_name);
    update_modification_state(allowing_modifications);
}

/*
 *  input_extend
 *
 */

void
input_extend(char *input)
{
    typein_extend(input);
}

/*
 *  get_user_input
 *
 *  GET_USER_INPUT allows the user to extend or edit the current input
 *  string.  GET_USER_INPUT returns under two circumstances:
 *
 *  (1) When the user types a Space, Tab, Return, or Newline, GET_USER_INPUT
 *  returns the terminating character (Return is translated to Newline); the
 *  current user input (not including the terminating character) is returned
 *  in USER_INPUT and MR->valid is set to FALSE.
 *
 *  (2) GET_USER_INPUT also terminates when the user issues a command (using
 *  a menu or button); the command is returned via MR.  MR->valid is TRUE in
 *  this case.
 *
 */

int
get_user_input(char user_input[], match_result *mr)
{
    activate_input();
    menu_match.valid = FALSE;
    while (!menu_match.valid) {
        process_next_event();
    }
    if (menu_match.kind < 0) {
        /* special key pressed */
        mr->valid = FALSE;
        typein_get_contents(user_input);
        switch (menu_match.index) {
          case specialSpace:
            return ' ';
          case specialTab:
            return '\t';
          default:
            return '\n';
        }
    }
    *mr = menu_match;
    return 0;
}

/*
 *  input_error
 *
 */

void
input_error(int num_matches)
{
    SysBeep(30);
}

/*
 *  stuff_command
 *
 *  This function causes get_user_input to return.
 *
 */

void
stuff_command(uims_reply kind, int index)
{
    menu_match.valid = TRUE;
    menu_match.kind = kind;
    menu_match.index = index;
}

/*
 **********************************************************************
 *  TYPESCRIPT (SEQUENCE DISPLAY)
 **********************************************************************
 */

void
text_output_add_line(char *text)
{
    if (building_reconcile_sequence) {
        if (!recpoint_initialized && (text[0] != ' ')) {
            /*
             * We only want to build the menu once for each reconciliation.
             * If recpoint_initialized is true, then we have already done it.
             * We ignore lines with a leading space. These lines are either
             * continuation lines, pictures, or warnings.  We want one
             * menu entry per call.
             */
            recpoint_add(text);
        }
    }
    else if (!writing_sequence) {
        typescript_add_line(text);
    }
}

void
text_output_trim(int n)
{
    if (!building_reconcile_sequence && !writing_sequence) {
        typescript_trim(n);
    }
}

/*
 **********************************************************************
 *  "ABOUT SD" WINDOW
 **********************************************************************
 */

void
show_about_sd(void)
{
    static DialogRecord dr;

    /* we use a dialog only for the rectangles laid out in it */

    if (aboutDialog == NULL) {
        aboutDialog = GetNewDialog(AboutSDDialog, &dr, 0);
        window_setup(&aboutW, aboutDialog);
        aboutW.kind            =                  ModalKind;
        aboutW.updateMethod    = (Method)         about_update;
        aboutW.mouseDownMethod = (EventMethod)    about_mouse_down;
        aboutW.keyMethod       = (KeyMethod)      about_key;
    }
    window_select(&aboutW);
    while (active_window() == &aboutW) {
        process_next_event();
    }
}

static void
about_update(Window *wp)
{
    char dbuf[200]; /* database version part of version string */
    char message[1000], *msgp;
    Rect r;
    Handle h;
    short kind;

    msgp = message;
    msgp = append_message(msgp, MessagesStrings, TitleString, "");

    GetDItem(aboutDialog, 1, &kind, &h, &r);
    TextFont(geneva);
    TextSize(18);
    TextFace(bold);
    TextBox(message, strlen(message), &r, teFlushDefault);

    msgp = message;

    /* note: database version not known until database has been read */

    dbuf[0] = '\0';
    if (database_version[0])
      sprintf(dbuf, " db%s :", database_version);
    sprintf(msgp, "Version %s :%s ui%s", sd_version_string(),
        dbuf, uims_version_string());
    msgp = msgp + strlen(msgp);
    *msgp++ = '\r';
    *msgp++ = '\r';

    msgp = append_message(msgp, MessagesStrings, CopyrightString, "\r\r");
    msgp = append_message(msgp, MessagesStrings, WarrantyString, "\r\r");
    msgp = append_message(msgp, MessagesStrings, FreeSoftwareString, "\r\r");
    msgp = append_message(msgp, MessagesStrings, GNULicenseString, "");

    GetDItem(aboutDialog, 2, &kind, &h, &r);
    TextSize(10);
    TextFace(normal);
    TextBox(message, strlen(message), &r, teFlushDefault);
}

static char *
append_message(char *msgp, int strings_id, int string_index, char *s)
{
    int c;
    GetIndString(msgp, strings_id, string_index);
    PtoCstr(msgp);
    msgp = msgp + strlen(msgp);
    while (c = *s++) *msgp++ = c;
    *msgp = '\0';
    return msgp;
}

static void
about_mouse_down(Window *wp, EventRecord *e)
{
    window_close(wp);
}

static void
about_key(Window *wp, short modifiers, char ch)
{
    window_close(wp);
}

/*
 **********************************************************************
 *  "SD HINTS" WINDOW
 **********************************************************************
 */

void
show_hints(void)
{
    static WindowRecord wr;

    if (hintsWindow == NULL) {
        hintsWindow = get_new_window(HintsWindow, &wr, 0);
        window_setup(&hintsW, hintsWindow);
        hintsW.updateMethod    = (Method)         hints_update;
    }
    window_select(&hintsW);
}

static void
hints_update(Window* wp)
{
    WindowPtr w = wp->window;
    char message[500];
    Rect r = w->portRect;

    GetIndString(message, MessagesStrings, HintsString);
    PtoCstr(message);
    TextFont(geneva);
    TextSize(10);
    InsetRect(&r, 4, 4);
    TextBox(message, strlen(message), &r, teFlushDefault);
}

/*
 **********************************************************************
 *  READING DATABASE PROGRESS INDICATOR
 **********************************************************************
 */

void
database_begin(char *filename)
{
    Str255 buf;
    static DialogRecord dr;
    Rect r;
    Handle h;
    short kind;

    /* we use a dialog only for the text and rectangles laid out in it */

    if (readingDialog == NULL) {
        strcpy((char *)buf, filename);
        ParamText(CtoPstr(buf), "\p", "\p", "\p");
        readingDialog = GetNewDialog(ReadingDatabaseDialog, &dr, 0);
        window_setup(&readingW, readingDialog);
        readingW.kind            =                  ModalKind;
    }

    window_select(&readingW);
    busy_cursor();

    SetPort(readingDialog);
    UpdateDialog(readingDialog, readingDialog->visRgn);
    GetDItem(readingDialog, 2, &kind, &h, &r);
    FrameRect(&r);
}

void
uims_database_tick_max(int n)
{
    db_tick_max = n;
}

void
uims_database_tick(int n)
{
    Rect r;
    Handle h;
    short kind;

    SetPort(readingDialog);
    GetDItem(readingDialog, 2, &kind, &h, &r);
    db_tick += n;
    if (db_tick > db_tick_max) {
        db_tick = db_tick_max;
    }
    r.right = r.left + ((r.right-r.left) * db_tick)/db_tick_max;
    PaintRect(&r);
}

void
uims_database_tick_end(void)
{
    window_close(&readingW);
    CloseDialog(readingDialog);
    readingDialog = NULL;
    arrow_cursor();
}

void
uims_database_error(char *message, char *call_name)
{
    database_error(message, call_name);
}

/*
 **********************************************************************
 *  POPUP WINDOWS
 **********************************************************************
 */

int
get_popup_string(char *prompt, char dest[])
{
    int old_mode = change_output_mode(StringInputMode);
    activate_input();
    typein_clear();
    typein_set_prompt(prompt);
    process_controls();
    change_output_mode(old_mode);
    if (menu_match.index == specialAccept) {
        typein_get_contents(dest);
        return POPUP_ACCEPT_WITH_STRING;
    }
    return POPUP_DECLINE;
}

int
uims_do_abort_popup(void)
{
    int item;

    if (!dirty) {
        return POPUP_ACCEPT;
    }
    item = wmgr_caution_alert(SaveSequenceAlert, 0L);
    switch(item) {
        case saveSave:
            save_command();
            if (!dirty)  /* dirty => was not saved */
                return POPUP_ACCEPT;
            break;
        case saveCancel:
            break;
        case saveDiscard:
            return POPUP_ACCEPT;
    }
    return POPUP_DECLINE;   
}
 
int
mac_modifier_any_popup(char *callname)
{
    return confirm_note_alert(ReplaceAnyAlert, callname);
}

int
mac_modifier_tag_popup(char *callname)
{
    return confirm_note_alert(ReplaceTagAlert, callname);
}

int
mac_modifier_scoot_popup(char *callname)
{
    return confirm_note_alert(ReplaceScootAlert, callname);
}

int
uims_do_getout_popup(char dest[])
{
    label_get(dest);
    return POPUP_ACCEPT_WITH_STRING;
}

int
uims_do_outfile_popup(char dest[])
{
    save_as_command();
    return POPUP_DECLINE; /* already done, no more to do! */
}

void
uims_bad_argument(char *s1, char *s2, char *s3)
{
    char buf[200];

    strcpy(buf, "Internal error: ");
    if (s1) strcat(buf, s1);
    if (s2) strcat(buf, s2);
    if (s3) strcat(buf, s3);
    miscellaneous_error(buf);
    exit_program(1);
}

/*
 **********************************************************************
 *  CURSOR MANAGEMENT
 **********************************************************************
 */

/*
 *  adjust_cursor
 *
 *  Set the appropriate cursor.
 *
 */

void
adjust_cursor(void)
{
    Point       pt, gpt;
    WindowPeek  wPtr;
    WindowPtr   w, mousewindow;
    GrafPtr     savePort;
    int         wt;
    int         cursor_set;
    
    w = FrontWindow();
    wt = window_type(w);
    if (wt != 0) {
        cursor_set = FALSE;
        if (active_window() == (Window *)&outputTW) {
            wPtr=(WindowPeek)w;
            GetPort(&savePort);
            SetPort((GrafPtr)wPtr);
            GetMouse(&pt);
            gpt = pt;
            LocalToGlobal(&gpt);
            FindWindow(gpt, &mousewindow);
            if (mousewindow == myWindow) {
                if (mousewindow != w) {
                    w = mousewindow;
                    wPtr=(WindowPeek)w;
                    SetPort((GrafPtr)wPtr);
                    GetMouse(&pt);
                }
                if (PtInRect(pt, &(**TEH).viewRect)) {
                    SetCursor(&edit_cursor);
                    cursor_set = TRUE;
                }
            }
            SetPort(savePort);
        }
        if (!cursor_set) {
            SetCursor(&arrow);
        }
    }
}

static void
init_cursors(void)
{
    CursHandle hCurs;
    
    hCurs = GetCursor(1);
    edit_cursor = **hCurs;
    hCurs = GetCursor(watchCursor);
    wait_cursor = **hCurs;
}

void
busy_cursor(void)
{
    SetCursor(&wait_cursor);
}

void
arrow_cursor(void)
{
    SetCursor(&arrow);
}

/*
 **********************************************************************
 *  MISCELLANEOUS
 **********************************************************************
 */

void
uims_debug_print(char *s)
{
    set_status(s);
}
