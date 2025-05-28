/*
 *  macwmgr.c - window manager
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

extern void
fixup_window_order(void);

/*

 Windows are divided into three categories:
 
 1. Application Windows

 Application windows include document windows and modeless dialog boxes.
 These are the windows that the user explicitly selects to make them
 active.  Only one of them is active at any one time, the frontmost one.

 2. Utility Windows

 These are auxiliary windows that float above all of the application
 windows.  The activeness of a utility window is not based on its
 position in the window stack.  It is based most likely on the activeness
 of an associated application window.  Selecting a utility window makes
 the associated application window active.

 3. Modal dialog boxes

 Modal dialog boxes float above application windows and utility windows.
 Only the frontmost modal dialog box can be used.  All other windows
 of the application are frozen.

*/

struct WindowStack {
    Window *contents[10];
    int    top;
};

typedef struct WindowStack WindowStack;

static WindowStack application_window_stack;
static WindowStack utility_window_stack;
static WindowStack modal_window_stack;

static void select_application_window(Window *wp);
static void close_application_window(Window *wp);
static void select_utility_window(Window *wp);
static void close_utility_window(Window *wp);
static void select_modal_window(Window *wp);
static void close_modal_window(Window *wp);
static void drag_window_without_selecting(Window *wp, Point p, Rect *rp);
static void remove_window_from_stack(WindowStack *ws, Window *wp);
static void push_window_on_stack(WindowStack *ws, Window *wp);
static Window *window_stack_top(WindowStack *ws);
static Window *window_stack_bottom(WindowStack *ws);

/*
 *  window_manager_initialize
 *
 */

void
window_manager_initialize(void)
{
    application_window_stack.top = -1;
    utility_window_stack.top = -1;
    modal_window_stack.top = -1;
}

/*
 *  display_update
 *
 *  This function processes update and activate events.  As a side
 *  effect, it allows the operating system to update windows belonging
 *  to other applications.  Call this function if you change the window
 *  state before invoking a modal dialog or before beginning a long
 *  computation.
 *
 */

void
display_update(void)
{
    selective_event_loop(updateMask|activMask);
}

/*
 *  front_application_window
 *
 *  Return the frontmost application window (document window or modeless
 *  dialog), or NULL if none.
 *
 */

Window *
front_application_window(void)
{
    return window_stack_top(&application_window_stack);
}

/*
 *  active_window
 *
 *  Return the frontmost modal window, if any, or the frontmost
 *  application window, or NULL.
 *
 */

Window *
active_window(void)
{
    Window *w;
    
    w = window_stack_top(&modal_window_stack);
    if (w != NULL) {
        return w;
    }
    return window_stack_top(&application_window_stack);
}

/*
 *  front_utility_window
 *
 *  Return the frontmost utility window, or NULL if none.
 *
 */

Window *
front_utility_window(void)
{
    return window_stack_top(&utility_window_stack);
}

/*
 *  back_utility_window
 *
 *  Return the backmost utility window, or NULL if none.
 *
 */

Window *
back_utility_window(void)
{
    return window_stack_bottom(&utility_window_stack);
}

/*
 *  movable_modal_up
 *
 */

long_boolean
movable_modal_up(void)
{
    return window_stack_top(&modal_window_stack) != NULL;
}

/*
 *  window_select
 *
 */

void
window_select(Window *wp)
{
    if (wp == NULL) {
        return;
    }
    switch (wp->kind) {
        case ApplicationKind:
            select_application_window(wp);
            break;
        case UtilityKind:
            select_utility_window(wp);
            break;
        case ModalKind:
            select_modal_window(wp);
            break;
        default:
            break;
    }
    adjust_menus();
}

/*
 *  window_close
 *
 */

void
window_close(Window *wp)
{
    if (wp == NULL) {
        return;
    }
    switch (wp->kind) {
        case ApplicationKind:
            close_application_window(wp);
            break;
        case UtilityKind:
            close_utility_window(wp);
            break;
        case ModalKind:
            close_modal_window(wp);
            break;
        default:
            break;
    }
    if (wp->closeMethod != NULL) {
        (*wp->closeMethod)(wp);
    }
    adjust_menus();
}

/*
 *  suspend_resume
 *
 */

void
suspend_resume(long_boolean suspend)
{
    Window *wp;
    int i;

    if (suspend) {
        ZeroScrap();
        TEToScrap();
    }
    else {
        TEFromScrap();
    }

    wp = active_window();
    if (wp != NULL) {
        window_activate(wp, !suspend);
    }

    /* expose or hide any associated utility windows */
    /* don't expose utility windows unless their root window
       is active */

    for (i=0;i<=utility_window_stack.top;i++) {
        Window *uwp = utility_window_stack.contents[i];
        if (suspend) {
            ShowHide(uwp->window, false);
        }
        else if ((uwp->root_window == NULL) || (uwp->root_window == wp)) {
            ShowHide(uwp->window, true);
        }
    }
}

/*
 *  window_is_active
 *
 */

long_boolean
window_is_active(Window *wp)
{
    return ((WindowPeek)wp->window)->hilited != false;
}

/*
 *  window_setup
 *
 *  This function initializes a Window structure to have the behavior of
 *  a modeless or movable modal dialog box.
 *
 */

void
window_setup(Window *wp, WindowPtr w)
{
    wp->window          = (WindowPtr)      w;
    wp->kind            =                  ApplicationKind;
    wp->root_window     = (Window *)       NULL;
    wp->TE              = (TEHandle)       NULL;
    wp->activateMethod  = (ActivateMethod) standard_activate_method;
    wp->updateMethod    = (Method)         NULL;
    wp->growMethod      = (PointMethod)    standard_grow_method;
    wp->mouseDownMethod = (EventMethod)    standard_mouse_down_method;
    wp->closeMethod     = (Method)         NULL;
    wp->resizeSubMethod = (ResizeMethod)   NULL;
    wp->editMethod      = (EditMethod)     standard_edit_method;
    wp->keyMethod       = (KeyMethod)      standard_key_method;
    wp->menuSetupMethod = (Method)         NULL;
    wp->controlMethod   = (ControlMethod)  NULL;
    SetWRefCon(w, (long)wp);
}

/*
 *  window_edit
 *
 */

void
window_edit(Window *wp, int item)
{
    if ((wp != NULL) && (wp->editMethod != NULL)) {
        (*wp->editMethod)(wp, item);
    }
}

/*
 *  standard_edit_method
 *
 */

void
standard_edit_method(Window *wp, int item)
{
    if (wp->TE != NULL) {
        text_edit(wp->TE, item);
    }
}

/*
 *  window_mouse_down
 *
 */

void
window_mouse_down(Window *wp, EventRecord *e)
{
    if ((wp != NULL) && (wp->mouseDownMethod != NULL)) {
        SetPort(wp->window);
        GlobalToLocal(&e->where);
        (*wp->mouseDownMethod)(wp, e);
    }
}

/*
 *  standard_mouse_down_method
 *
 *  This method handles mouse clicks in controls.
 *
 */

void
standard_mouse_down_method(Window *wp, EventRecord *e)
{
    ControlHandle h = NULL;
    int which_part = 0;
    int item_index;
    WindowPtr w = wp->window;

    if (FindControl(e->where, w, &h) != 0) {
        which_part = TrackControl(h, e->where, (ProcPtr) -1);
        if (which_part == 0) {
            return; /* no action needed */
        }
    }
    if (wp->controlMethod != NULL) {
        (*wp->controlMethod)(wp, h, which_part);
    }
}

/*
 *  window_update
 *
 */

void
window_update(Window *wp)
{
    if ((wp != NULL) && (wp->updateMethod != NULL)) {
        SetPort(wp->window);
        BeginUpdate(wp->window);
        (*wp->updateMethod)(wp);
        EndUpdate(wp->window);
    }
}

/*
 *  window_grow
 *
 */

void
window_grow(Window *wp, Point p)
{
    if ((wp != NULL) && (wp->growMethod != NULL)) {
        (*wp->growMethod)(wp, p);
    }
}

/*
 *  standard_grow_method
 *
 *  This function is called when the user wants to resize the window.
 *
 */

void
standard_grow_method(Window *wp, const Point p)
{
    long       result;
    Rect       oldWR, r;
    WindowPtr  w;
    int        delta_v, delta_h;
    
    w = wp->window;
    oldWR = w->portRect;
    SetRect(&r, 80, 80, screenBits.bounds.right, screenBits.bounds.bottom);
    result = GrowWindow(w, p, &r);
    if (result == 0) {
        return;
    }
    SizeWindow(w, LoWord(result), HiWord(result), true);
    SetPort(w);
    InvalRect(&w->portRect);
    delta_v = w->portRect.bottom-oldWR.bottom;
    delta_h = w->portRect.right-oldWR.right;
    if (wp->resizeSubMethod != NULL) {
        (*wp->resizeSubMethod)(wp, delta_v, delta_h);
    }
}
   
/*
 *  window_drag
 *
 */

void
window_drag(Window *wp, Point p, short modifiers)
{
    static Rect dragRect = { 0, 0, 1024, 1024 };
    if ((modifiers & cmdKey) == 0) {
        window_select(wp);
    }
    drag_window_without_selecting(wp, p, &dragRect);
}

/*
 *  window_key
 *
 */

void
window_key(Window *wp, short modifiers, char ch)
{
    if ((wp != NULL) && (wp->keyMethod != NULL)) {
        (*wp->keyMethod)(wp, modifiers, ch);
    }
}

/*
 *  standard_key_method
 *
 */

void
standard_key_method(Window *wp, short modifiers, char ch)
{
    if (wp->TE != NULL) {
        text_key(wp->TE, modifiers, ch);
    }
    else {
        SysBeep(30);
    }
}

/*
 *  standard_activate_method
 *
 */

void
standard_activate_method(Window *wp, long_boolean activate)
{
    HiliteWindow(wp->window, activate ? true : false);
    if (wp->TE != NULL) {
        text_activate(wp->TE, activate);
    }
}

/*
 *  wmgr_caution_alert
 *
 */

short
wmgr_caution_alert(short id, ModalFilterProcPtr proc)
{
    short result;

    deactivate_front_window();
    arrow_cursor();
    result = CautionAlert(id, proc);
    activate_front_window();
    adjust_cursor();
    return result;
}

/*
 *  wmgr_note_alert
 *
 */

short
wmgr_note_alert(short id, ModalFilterProcPtr proc)
{
    short result;

    deactivate_front_window();
    arrow_cursor();
    result = NoteAlert(id, proc);
    activate_front_window();
    adjust_cursor();
    return result;
}

/*
 *  wmgr_stop_alert
 *
 */

short
wmgr_stop_alert(short id, ModalFilterProcPtr proc)
{
    short result;

    deactivate_front_window();
    arrow_cursor();
    result = StopAlert(id, proc);
    activate_front_window();
    adjust_cursor();
    return result;
}

/*
 *  window_global_portrect
 *
 *  Return the window's port rectangle in global coordinates.
 *
 */

void
window_global_portrect(WindowPtr w, Rect *rp)
{
    WindowPtr old_port;

    GetPort(&old_port);
    SetPort(w);
    *rp = w->portRect;
    LocalToGlobal((Point *) &rp->top);
    LocalToGlobal((Point *) &rp->bottom);
    SetPort(old_port);
}

/*
 *  window_activate
 *
 *  This function is normally invoked only as a side effect of one
 *  of the other public functions defined in this file.  The exception
 *  is to handle an activate event caused by a system-invoked modal
 *  dialog, such as that produced by the standard file package.
 *
 */

void
window_activate(Window *wp, long_boolean activate)
{
    int i;

    if (wp == NULL) {
        return;
    }

#if 0
    /* this optimization does not work */
    if (window_is_active(wp) == activate) {
        return;
    }
#endif

    if (wp->root_window != NULL) {
        wp = wp->root_window;
    }
    if (wp->activateMethod != NULL) {
        (*wp->activateMethod)(wp, activate);
    }

    /* expose or hide any associated utility windows */

    for (i=0;i<=utility_window_stack.top;i++) {
        Window *uwp = utility_window_stack.contents[i];
        if (uwp->root_window == wp) {
            ShowHide(uwp->window, activate ? true : false);
        }
    }
}

/*
 *  window_type
 *
 */

int
window_type(WindowPtr w)
{
    int wt;
    
    if (w == NULL) {
        return 0;
    }
    wt = ((WindowPeek)w)->windowKind;
    if (wt < 0) {
        return 0; /* desk accessory in old System */
    }
    if (wt == userKind) {
        return 1; /* application window */
    }
    return 2; /* dialog box */
}

/*
 **********************************************************************
 *  PRIVATE FUNCTIONS
 **********************************************************************
 */

/*
 *  select_application_window
 *
 *  The specified window should be an application window (a document
 *  window or a modeless dialog).  Expose it, activate it, and move
 *  it in front of any other application windows.  If it is already
 *  the frontmost application window, do nothing.
 *
 */

static void
select_application_window(Window *wp)
{
    Window *u;

    if (wp == front_application_window()) {
        return;
    }
    deactivate_front_window();
    push_window_on_stack(&application_window_stack, wp);
    u = back_utility_window();

    /*
     *  The following is a heavy-handed workaround for brain-damaged SendBehind
     *  in Mac systems prior to System 7.
     *
     */

    if (!popup_control_available) {
        ShowHide(wp->window, true);
        activate_front_window();
        BringToFront(wp->window);
        fixup_window_order();
        return;
    }

    if (u != NULL) {
        SendBehind(wp->window, u->window);
        if (!popup_control_available) {
            /* prior to System 7, SendBehind is brain damaged */
            /* see Inside Macintosh, Volume 1, pg. I-286 */
            /* Note: the following Apple-recommended workaround fails in some cases */
            /* See "heavy handed workaround" just above */
            WindowPeek p = (WindowPeek)wp->window;
            PaintOne(p, p->strucRgn);
            CalcVis(p);
        }
    }
    else {
        BringToFront(wp->window);
    }
    activate_front_window();
    ShowHide(wp->window, true);
}

/*
 *  close_application_window
 *
 *  The specified window should be an application window (a document
 *  window or a modeless dialog).
 *
 */

static void
close_application_window(Window *wp)
{
    long_boolean was_front = (wp == active_window());

    remove_window_from_stack(&application_window_stack, wp);
    ShowHide(wp->window, false);
    if (was_front) {
        activate_front_window();
    }
}

/*
 *  select_utility_window
 *
 *  The specified window should be an utility window.  Expose it, and move
 *  it in front of any other utility windows, or in front of all
 *  application windows.  Activate its
 *  associated application window, if any; otherwise, activate
 *  the utility window.  If it is already
 *  the frontmost utility window, do nothing.
 *
 */

static void
select_utility_window(Window *wp)
{
    Window *f;

    if (wp == front_utility_window()) {
        return;
    }
    push_window_on_stack(&utility_window_stack, wp);
    BringToFront(wp->window);
    if (wp->root_window != NULL) {
        f = front_application_window();
        if ((f != NULL) && (f != wp->root_window)) {
            window_activate(f, FALSE);
        }
    }
    window_activate(wp, TRUE); /* might activate root window */
    ShowHide(wp->window, true);
}

/*
 *  close_utility_window
 *
 *  The specified window should be an utility window.  Deexpose it.
 *
 */

static void
close_utility_window(Window *wp)
{
    Window *f;

    remove_window_from_stack(&utility_window_stack, wp);
    ShowHide(wp->window, false);
}

/*
 *  select_modal_window
 *
 *  The specified window should be a modal dialog box.
 *
 */

static void
select_modal_window(Window *wp)
{
    if (wp == window_stack_top(&modal_window_stack)) {
        return;
    }
    deactivate_front_window();
    push_window_on_stack(&modal_window_stack, wp);
    BringToFront(wp->window);
    window_activate(wp, TRUE);
    ShowHide(wp->window, true);
    arrow_cursor();
}

/*
 *  close_modal_window
 *
 *  The specified window should be the frontmost modal window.
 *
 */

static void
close_modal_window(Window *wp)
{
    Window *f;

    remove_window_from_stack(&modal_window_stack, wp);
    ShowHide(wp->window, false);
    activate_front_window();
    adjust_cursor();
}

/*
 *  deactivate_front_window
 *
 */

void
deactivate_front_window(void)
{
    Window *wp;

    wp = active_window();
    if (wp != NULL)
        window_activate(wp, FALSE);
}

/*
 *  activate_front_window
 *
 */

void
activate_front_window(void)
{
    Window *wp;

    wp = active_window();
    if (wp != NULL) {
        window_activate(wp, TRUE);
    }
}

/*
 *  drag_window_without_selecting
 *
 */

static void
drag_window_without_selecting(Window *wp, Point p, Rect *rp)
{
    GrafPtr port;
    long result;
    int dv, dh;
    Rect r;
    Rect slop, limit;
    RgnHandle outline;
    RgnHandle save;
    WindowPtr w = wp->window;

    limit = *rp;
    slop = *rp;
    InsetRect(&slop, -1, -1);
    GetWMgrPort(&port);
    SetPort(port);
    outline = NewRgn();
    save = NewRgn();
    CopyRgn(((WindowPeek) w)->strucRgn, outline);
    GetClip(save);
    ClipRect(&limit);
    result = DragGrayRgn(outline, p, &limit, &slop, 0, 0L);
    SetClip(save);
    DisposeRgn(save);
    DisposeRgn(outline);
    if (result != 0x80008000) {
        SetPort(w);
        r = w->portRect;
        LocalToGlobal((Point *) &r.top);
        dv = (result >> 16) & 0xFFFF;
        dh = result & 0xFFFF;
        MoveWindow(w, r.left + dh, r.top + dv, false);
    }
}

/*
 *  remove_window_from_stack
 *
 *  Remove the specified window from the stack, if present.
 *  There is no effect on the Mac window system.
 *
 */

static void
remove_window_from_stack(WindowStack *ws, Window *wp)
{
    int i;

    for (i=0;i<=ws->top;i++) {
        if (ws->contents[i] == wp) {
            break;
        }
    }
    if (i > ws->top) {
        return;
    }
    ws->top--;
    while (i <= ws->top) {
        ws->contents[i] = ws->contents[i+1];
        i++;
    }
}

/*
 *  push_window_on_stack
 *
 */

static void
push_window_on_stack(WindowStack *ws, Window *wp)
{
    remove_window_from_stack(ws, wp);
    ws->contents[++ws->top] = wp;
}

/*
 *  window_stack_top
 *
 */

static Window *
window_stack_top(WindowStack *ws)
{
    if (ws->top >= 0) {
        return ws->contents[ws->top];
    }
    return NULL;
}

/*
 *  window_stack_bottom
 *
 */

static Window *
window_stack_bottom(WindowStack *ws)
{
    if (ws->top >= 0) {
        return ws->contents[0];
    }
    return NULL;
}

/*
 *  fixup_window_order
 *
 *  This is a workaround for an unexplained problem.  Using the
 *  standard file package moves my utility windows behind the
 *  application window.
 *
 */

void
fixup_window_order(void)
{
    int i;

    for (i=0;i<=utility_window_stack.top;i++) {
        Window *uwp = utility_window_stack.contents[i];
        BringToFront(uwp->window);
    }
}
