/*
 *  macdlog.c - support for dialog windows
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

static void draw_frame_around_button_rect(WindowPtr w, Rect button_rect, long_boolean active);
static pascal draw_default_button_outline(DialogPtr dp, short item);

/*
 *  dialog_setup
 *
 *  This function initializes a Window structure to have the behavior of
 *  a modeless or movable modal dialog box.
 *
 */

void
dialog_setup(DialogWindow *dwp, DialogPtr dp)
{
    Window *wp = &dwp->base;
    window_setup(wp, (WindowPtr)dp);
    wp->activateMethod  = (ActivateMethod) dialog_activate;
    wp->updateMethod    = (Method)         dialog_update;
    wp->mouseDownMethod = (EventMethod)    dialog_mouse_down;
    wp->keyMethod       = (KeyMethod)      dialog_key;
    dwp->default_item   =                  -1;
}

/*
 *  dialog_activate
 *
 *  This function sets the highlighting of all the controls
 *  in the dialog box.
 *
 */

void
dialog_activate(DialogWindow *dwp, long_boolean activate)
{
    Handle h;
    int item;
    short kind;
    Rect box;
    short val = activate ? 0 : 255;
    DialogPtr dp = (DialogPtr)dwp->base.window;

    standard_activate_method((Window *)dwp, activate);
    for (item=1;;item++) {
        h = NULL;
        GetDItem(dp, item, &kind, &h, &box);
        if (h == NULL) {
            break;
        }
        if ((kind & ctrlItem) != 0) {
            HiliteControl((ControlHandle)h, val);
        }
    }
    if (dwp->default_item > 0) {
        draw_frame_around_button((ControlHandle)ditem(dp, dwp->default_item));
    }
}

/*
 *  dialog_update
 *
 */

void
dialog_update(DialogWindow *dwp)
{
    WindowPtr w = dwp->base.window;
    UpdateDialog(w, w->visRgn);
}

/*
 *  dialog_mouse_down
 *
 */

void
dialog_mouse_down(DialogWindow *dwp, EventRecord *e)
{
    int item_index;

    dwp->item = -1;
    item_index = FindDItem((DialogPtr)dwp->base.window, e->where);
    if (item_index >= 0) {
        dwp->item = item_index + 1; /* item number */
        }
    standard_mouse_down_method((Window *)dwp, e);
}

/*
 *  dialog_key
 *
 */

void
dialog_key(DialogWindow *dwp, short modifiers, char ch)
{
    if (((ch == '\r') || (ch == enterKey)) && (dwp->default_item > 0)) {
        DialogPtr dp = (DialogPtr)dwp->base.window;
        ControlHandle h = (ControlHandle)(ditem(dp, dwp->default_item));
        dwp->item = dwp->default_item;
        flash_control(h, inButton);
        if (dwp->base.controlMethod != NULL) {
            (*dwp->base.controlMethod)((Window *)dwp, h, inButton);
        }
    }
    else {
        standard_key_method((Window *)dwp, modifiers, ch);
    }
}

/*
 *  dialog_set_control_value
 *
 */

void
dialog_set_control_value(DialogPtr dp, short index, short value)
{
    Handle h;

    h = ditem(dp, index);
    if (h != NULL) {
        SetCtlValue(h, value);
    }
}

/*
 *  dialog_get_control_value
 *
 */

short
dialog_get_control_value(DialogPtr dp, short index)
{
    Handle h;

    h = ditem(dp, index);
    if (h != NULL) {
        return GetCtlValue(h);
    }
    return -1;
}

/*
 *  ditem - return item handle
 *
 */

Handle
ditem(DialogPtr dp, int i)
{
    short kind;
    Handle item;
    Rect box;
    
    GetDItem(dp, i, &kind, &item, &box);
    return(item);
}

/*
 *  movable_modal_dialog
 *
 */

void
movable_modal_dialog(DialogWindow *dwp)
{
    dwp->item = -1;
    while (dwp->item < 0) {
        process_next_event();
    }
}

/*
 *  radio_set
 *
 */

void
radio_set(DialogPtr dp, int first, int last, int selected)
{
    int i;

    for (i=first;i<=last;i++) {
        SetCtlValue(ditem(dp, i), i==selected);
    }
}

/*
 *  draw_frame_around_button
 *
 */

void
draw_frame_around_button(ControlHandle control)
{
    WindowPtr w;
    Rect r;
    long_boolean active;

    w = (**control).contrlOwner;
    r = (**control).contrlRect;
    active = ((**control).contrlHilite == 0);
    draw_frame_around_button_rect(w, r, active);
}

/*
 *  draw_frame_around_button_rect
 *
 */

static void
draw_frame_around_button_rect(WindowPtr w, Rect button_rect, long_boolean active)
{
    int frame_inset = -4;
    int frame_size = 3;
    WindowPtr old_port;
    PenState current_pen;
    int button_oval;
    
    GetPort(&old_port);
    SetPort(w);
    GetPenState(&current_pen);
    PenNormal();
    InsetRect(&button_rect, frame_inset, frame_inset);
    FrameRoundRect(&button_rect, 16, 16);
    button_oval = (button_rect.bottom - button_rect.top) / 2 + 2;
    if (active) {
        PenPat(black);
    }
    else {
        PenPat(gray);
    }
    PenSize(frame_size, frame_size);
    FrameRoundRect(&button_rect, button_oval, button_oval);
    SetPenState(&current_pen);
    SetPort(old_port);
}

/*
 *  draw_default_button_outline
 *
 *  This function is designed to be called directly from the Dialog Manager.
 *
 */

static pascal
draw_default_button_outline(DialogPtr dp, short item)
{
    short item_type;
    Rect item_rect;
    Handle item_handle;
    long_boolean active;
    DialogWindow *dwp;

    dwp = (DialogWindow *)GetWRefCon(dp);
    if (dwp != NULL) {
        GetDItem(dp, dwp->default_item, &item_type, &item_handle, &item_rect);
        active = ((**(ControlHandle)item_handle).contrlHilite == 0);
        draw_frame_around_button_rect((WindowPtr)dp, item_rect, active);
    }
}

/*
 *  setup_dialog_box
 *
 */

void
setup_dialog_box(DialogPtr dp, int user_item)
{
    short item_type;
    Handle item_handle;
    Rect item_rect;

    GetDItem(dp, user_item, &item_type, &item_handle, &item_rect);
    SetDItem(dp, user_item, item_type, (Handle)draw_default_button_outline,
        &item_rect);
}
