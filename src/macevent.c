/*
 *  macevent.c - event handling
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

static void process_update_event(EventRecord *e);
static void process_activate_event(EventRecord *e);
static void process_os_event(EventRecord *e);
static void process_key_down_event(short modifiers, char key);
static void process_mouse_down_event(int windowPart, WindowPtr w, EventRecord *myEvent);
static long_boolean old_system_grow_box(WindowPtr w, EventRecord *e);

/*
 **********************************************************************
 *  PUBLIC FUNCTIONS
 **********************************************************************
 */

/*
 *  process_next_event
 *
 */

void
process_next_event(void) 
{
    selective_event(everyEvent);
}

/*
 *  selective_event_loop
 *
 */

void
selective_event_loop(short event_mask)
{
    EventRecord e;
    while (EventAvail(event_mask, &e)) {
        selective_event(event_mask);
    }
}

/*
 *  selective_event
 *
 */

void
selective_event(short event_mask)
{
    EventRecord e;
    Window *wp = active_window();
    
    adjust_cursor();
    window_edit(wp, idleCommand);

    if (wait_next_event_available) {
        if (WaitNextEvent(event_mask, &e, GetCaretTime(), 0L)) {
            process_event(&e);
        }
    }
    else {
        SystemTask();
        if (GetNextEvent(event_mask, &e)) {
            process_event(&e);
        }
    }
}

/*
 *  process_event
 *
 */

void
process_event(EventRecord *e)
{
    WindowPtr   w;
    short       windowPart;
    Rect        r;
    char        ch;
    int         auto_key;

    auto_key = FALSE;

    switch (e->what) {
    case mouseDown:
        windowPart = FindWindow(e->where, &w);
        process_mouse_down_event(windowPart, w, e);
        break;

    case autoKey:
        auto_key = TRUE;
        /* fall through */
    case keyDown:
        ch = e->message & charCodeMask;
        if (!auto_key && ((e->modifiers & cmdKey) != 0)) {

            /* special case to make Cmd[+] work */
            if ((ch == '=') && ((e->modifiers & shiftKey) != 0)) {
                ch = '+';
            }

            adjust_menus();
            menu_command(MenuKey(ch));
        }
        else {
            process_key_down_event(e->modifiers, ch);
        }
        break;

    case activateEvt:
        process_activate_event(e);
        break;

    case updateEvt:
        process_update_event(e);
        break;

    case osEvt:
        process_os_event(e);
        break;

    }
}

/*
 **********************************************************************
 *  PRIVATE FUNCTIONS
 **********************************************************************
 */

/*
 *  process_activate_event
 *
 *  Because this application has its own window manager, it
 *  typically does not receive activate events.  The one
 *  exception is when a system-invoked modal dialog, such
 *  as the standard file dialog, is opened or closed.  In
 *  this case, the window that the Mac toolbox thinks is
 *  the one to activate may not be the right one, because
 *  of utility windows.
 *
 */

static void
process_activate_event(EventRecord *e)
{
    WindowPtr w = (WindowPtr)e->message;
    if (window_type(w) != 0) {
        Window *wp = active_window();
        window_activate(wp, (e->modifiers & activeFlag) != 0);
    }
}

/*
 *  process_update_event
 *
 */

static void
process_update_event(EventRecord *e)
{
    WindowPtr w = (WindowPtr)e->message;
    if (window_type(w) != 0) {
        Window *wp = (Window *)GetWRefCon(w);
        window_update(wp);
    }
}

/*
 *  process_os_event
 *
 */

static void
process_os_event(EventRecord *e)
{
    if (((e->message >> 24) & 0xFF) == suspendResumeMessage) {
        suspend_resume((e->message & resumeFlag) == 0);
    }
}

/*
 *  process_key_down_event
 *
 */

static void
process_key_down_event(short modifiers, char key)
{
    Window *wp = active_window();
    window_key(wp, modifiers, key);
}

/*
 *  process_mouse_down_event
 *
 */

static void
process_mouse_down_event(int windowPart, WindowPtr w, EventRecord *e)
{
    Window *wp;
    wp = (Window *)(GetWRefCon(w));

    switch (windowPart) {
        case inGoAway:
            window_close(wp);
            break;

        case inMenuBar:
            adjust_menus();
            menu_command(MenuSelect(e->where));
            break;

        case inSysWindow:
            SystemClick(e, w);
            break;

        case inDrag:
            if (movable_modal_up() && (w != FrontWindow())) {
                SysBeep(30);
            }
            else if (window_type(w)) {
                window_drag(wp, e->where, e->modifiers);
            }
            break;

        case inGrow:
            window_grow(wp, e->where);
            break;

        case inContent:
            if (movable_modal_up() && (w != FrontWindow())) {
                SysBeep(30);
            }
            else if (!(window_is_active(wp))) {
                window_select(wp);
            }
            else if (old_system_grow_box(w, e)) {
                /* special case test for pre-System 7 */
                window_grow(wp, e->where);
            }
            else {
                window_mouse_down(wp, e);
            }
            break;
    }
}

/*
 *  old_system_grow_box
 *
 *  Prior to System 7, FindWindow does not check for mouse clicks in
 *  a grow box unless the window is the frontmost visible window (as
 *  opposed to a highlighted window).  We therefore lose if we have
 *  visible utility windows.
 *
 */

static long_boolean
old_system_grow_box(WindowPtr w, EventRecord *e)
{
    /* TBD */
    return FALSE;
}
