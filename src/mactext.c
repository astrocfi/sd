/*
 *  mactext.c - support for text editing windows and panes
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
#include <string.h> /* for strlen */
#include <ctype.h> /* for isprint */

static void adjust_text(TextWindow *twp);
static void adjust_vscroll(TextWindow *twp);
static pascal void text_window_scroll_handler(ControlHandle h, short part);
static void clip_viewport(TextWindow *twp);

static int compute_top_for_bottom(TextWindow *twp, int bottom);
static int compute_bottom_for_top(TextWindow *twp, int top);

static long_boolean clipping = FALSE; /* clipping seems to slow things down too much */

/*
 *  text_window_setup
 *
 */

void
text_window_setup(TextWindow *twp, WindowPtr w, int multistyle)
{
    Window *wp = &twp->base;

    window_setup(wp, w);
    wp->activateMethod  = (ActivateMethod) text_window_activate;
    wp->mouseDownMethod = (EventMethod)    text_window_mouse_down;
    wp->resizeSubMethod = (ResizeMethod)   text_window_resize;
    wp->editMethod      = (EditMethod)     text_window_edit;

    clipping = FALSE;

    twp->vScroll = NewControl(twp->base.window, &twp->vScrollRect,
        "\p", 1, 0, 0, 0, scrollBarProc, 0L);
    if (multistyle) {
        wp->TE = TEStylNew(&twp->viewRect, &twp->viewRect);
        twp->fixed_height = FALSE;
    }
    else {
        wp->TE = TENew(&twp->viewRect, &twp->viewRect);
        twp->fixed_height = TRUE;
    }
    text_window_set_view(twp);
}

/*
 *  text_window_set_text
 *
 *  Replace the contents of the text pane.  The selection is reset to the
 *  beginning of the text, but the selection is not displayed.  The scroll
 *  position is reset to show the top line.  This code is optimized to avoid
 *  unnecessary redisplay.
 *
 */

void
text_window_set_text(TextWindow *twp, const char *buffer, int len)
{
    TEHandle TE = twp->base.TE;

    TESetText(buffer, len, TE);
    (**TE).selStart = 0; /* ensure that top line will be visible */
    (**TE).selEnd = 0;
    SetCtlValue(twp->vScroll, 0);
    (**TE).destRect.top = (**TE).viewRect.top;
    adjust_vscroll(twp);
    if (!twp->fixed_height && clipping) {
        clip_viewport(twp);
    }
    SetPort(twp->base.window);
    InvalRect(&twp->base.window->portRect);
}

/*
 *  adjust_text
 *
 *  This function scrolls the text pane, if needed, to match the value in
 *  the scroll bar control.  Call ADJUST_TEXT after the scroll bar moves.
 *
 */

static void
adjust_text(TextWindow *twp)
{
    int oldScroll, newScroll, delta, topline;
    TEHandle TE;
    
    TE=twp->base.TE;
    topline = GetCtlValue(twp->vScroll);
    oldScroll = (**TE).viewRect.top - (**TE).destRect.top;
    if (topline == 0) {
        newScroll = 0;
    }
    else {
        newScroll = TEGetHeight(topline, 1, TE); /* height of lines above topline */
    }
    delta = oldScroll - newScroll;
    if (delta != 0) {
        if (!twp->fixed_height && clipping) {
            clip_viewport(twp);
        }
        TEScroll(0, delta, TE);
    }
}

/*
 *  adjust_vscroll
 *
 *  This function sets the maximum value of the scroll bar control based on
 *  the number of lines at the end of the text buffer that will fit in the
 *  text pane.  It may also change the current value of the control based on
 *  the new maximum value, in which case it also scrolls the text.  Call
 *  ADJUST_VSCROLL after the size of the text buffer or the size of the text
 *  pane changes.
 *
 */

static void
adjust_vscroll(TextWindow *twp)
{
    int n, last, top;
    TEHandle TE;
    
    TE=twp->base.TE;

    /* Compute the index of the last text line that might be the
       top line in the text pane. */

    last = (**TE).nLines - 1;
    if (last < 0) {
        last = 0;
    }
    n = compute_top_for_bottom(twp, last);
    top = GetCtlValue(twp->vScroll);
    SetCtlMax(twp->vScroll, n);
    if (top > n) {
        adjust_text(twp);
    }
}

/*
 *  text_window_adjust
 *
 *  Call this function after making any change to the contents of the text
 *  buffer.  It updates the scroll position to ensure that the selection is
 *  visible.
 *
 */

void
text_window_adjust(TextWindow *twp)
{
    adjust_vscroll(twp);
    text_window_show_line(twp, text_selected_line(twp->base.TE));
    if (clipping) {
        clip_viewport(twp);
    }
}

/*
 *  text_window_show_line
 *
 *  This function scrolls if needed to make the specified line visible.
 *
 */

void
text_window_show_line(TextWindow *twp, int line)
{    
    int top, bottom;    
    
    top = GetCtlValue(twp->vScroll);
    if (line < top) {
        text_window_set_top_line(twp, line);
        return;
    }
    bottom = compute_bottom_for_top(twp, top);
    if (line > bottom) {
        text_window_set_top_line(twp, compute_top_for_bottom(twp, line));
    }
}

/*
 *  text_window_set_view
 *
 *  Call this function when the viewRect changes.
 *
 */

void
text_window_set_view(TextWindow *twp)
{
    TEHandle TE;
    int v_scroll_distance;

    TE = twp->base.TE;
    v_scroll_distance = (**TE).viewRect.top - (**TE).destRect.top;
    (**TE).viewRect = twp->viewRect;
    (**TE).destRect = twp->viewRect;
    (**TE).destRect.top -= v_scroll_distance;
    twp->pane_height = twp->viewRect.bottom - twp->viewRect.top;
    if (clipping || twp->fixed_height) {
        clip_viewport(twp);
    }
}

/*
 *  clip_viewport
 *
 *  This function sets the bottom of the view rectangle to ensure that a
 *  partial line is not shown at the bottom of the text pane.
 *
 */

static void
clip_viewport(TextWindow *twp)
{
    TEHandle TE;
    int topline, botline, height, newbottom, oldbottom;
    Rect r;

    TE = twp->base.TE;

    /* clip viewport so that a partial line is not seen at the bottom */
    
    if (twp->fixed_height) {
        int numlines;
        height = (**TE).lineHeight;
        numlines = ((**TE).viewRect.bottom - (**TE).viewRect.top) / height;
        newbottom = (**TE).viewRect.top + numlines * height;
    }
    else {
        topline = GetCtlValue(twp->vScroll);
        botline = compute_bottom_for_top(twp, topline);
        height = TEGetHeight(botline+1, topline+1, TE);
        newbottom = (**TE).viewRect.top + height;
    }
    oldbottom = (**TE).viewRect.bottom;
    if (newbottom < oldbottom) {
        r = (**TE).viewRect;
        r.top = newbottom;
        SetPort(twp->base.window);
        EraseRect(&r);
        (**TE).viewRect.bottom = newbottom;
    }
    else if (newbottom > oldbottom) {
        r = (**TE).viewRect;
        r.top = oldbottom;
        r.bottom = newbottom;
        SetPort(twp->base.window);
        (**TE).viewRect.bottom = newbottom;
        TEUpdate(&r, TEH);
    }    
}

/*
 *  text_window_set_top_line
 *
 *  This function scrolls the text so that the designated line is the top
 *  line in the text pane.  Appropriate clipping is performed.
 *
 */

void
text_window_set_top_line(TextWindow *twp, int top)
{
    SetCtlValue(twp->vScroll, top); /* clips */
    adjust_text(twp);
}

/*
 *  text_window_scroll
 *
 *  This function scrolls the text in the text window.  A positive number
 *  scrolls down (shows higher numbered lines).  Appropriate clipping of the
 *  scroll position is performed.
 *
 */

void
text_window_scroll(TextWindow *twp, int delta)
{
    text_window_set_top_line(twp, GetCtlValue(twp->vScroll) + delta);
}

/*
 *  text_window_scroll_handler
 *
 */

static pascal void
text_window_scroll_handler(ControlHandle h, short part)
{
    int topline;
    WindowPtr w;
    TextWindow *twp;
    
    if (part == 0) {
        return;
    }

    w = (**h).contrlOwner;
    twp = (TextWindow *)GetWRefCon(w);
    topline = GetCtlValue(twp->vScroll);

    switch (part) {
        case inUpButton: 
            text_window_scroll(twp, -1);
            break;
        case inDownButton: 
            text_window_scroll(twp, 1);
            break;
        case inPageUp: 
            text_window_set_top_line(twp, compute_top_for_bottom(twp, topline-1));
            break;
        case inPageDown: 
            text_window_set_top_line(twp, 1+compute_bottom_for_top(twp, topline));
            break;
    }
}

/*
 *  text_window_mouse_down
 *
 *  This function is called to handle a mouse down event in the window.
 *
 */

void
text_window_mouse_down(TextWindow *twp, EventRecord *e)
{
    text_window_standard_mouse_down(twp, e);
}

/*
 *  text_window_standard_mouse_down
 *
 *  This function performs the default behavior for mouse down events.  It
 *  can be interposed with a custom handler.
 *
 *  This function returns 0 if the event is not handled, 1 if TextEdit
 *  handled the event, 2 if the scroll bar handled the event.
 *
 */

int
text_window_standard_mouse_down(TextWindow *twp, EventRecord *e)
{
    int                part;
    ControlHandle      h;
    
    part = FindControl(e->where, twp->base.window, &h);
    if (part == 0) {
        if (PtInRect(e->where, &(**twp->base.TE).viewRect)) {
            TEClick(e->where, (e->modifiers & shiftKey)!=0, twp->base.TE);
            return 1;
            }
        else {
            return 0;
        }
    }
    else if (part == inThumb) {
        TrackControl(h, e->where, 0L);
        adjust_text(twp);
    }
    else if (h == twp->vScroll) {
        TrackControl(h, e->where, &text_window_scroll_handler);
    }
    else {
        part = TrackControl(h, e->where, 0L);
        if (part != 0) {
            (*twp->base.controlMethod)((Window *)twp, h, part);
        }
    }
    return 2;
}

/*
 *  text_window_select_line
 *
 *  Return the line number of the line under point P.  Scroll the text if
 *  point P is above or below the view area.
 *
 */

int
text_window_select_line(TextWindow *twp, Point p)
{
    int n;
    TEHandle TE;

    TE = twp->base.TE;

    /* TBD: this could be done more efficiently */

    n = text_offset_to_line(TE, TEGetOffset(p, TE));
    text_window_show_line(twp, n);
    return n;
}

/*
 *  text_window_resize
 *
 *  Call text_window_set_view before calling this function.
 *
 */

void
text_window_resize(TextWindow *twp, int delta_v, int delta_h)
{
    Rect r;
    
    MoveControl(twp->vScroll, twp->vScrollRect.left, twp->vScrollRect.top);
    SizeControl(twp->vScroll, SBarWidth,
        twp->vScrollRect.bottom-twp->vScrollRect.top);
    r = (**twp->vScroll).contrlRect;
    ValidRect(&r);
    adjust_vscroll(twp);
}

/*
 *  text_window_activate
 *
 */

void
text_window_activate(TextWindow *twp, long_boolean activate)
{
    standard_activate_method((Window *)twp, activate);
    if (activate) {
        ShowControl(twp->vScroll);
    }
    else {
        HideControl(twp->vScroll);
    }
}

/*
 *  text_window_edit
 *
 */

void
text_window_edit(TextWindow *twp, int item)
{
    text_edit(twp->base.TE, item);
}

/*
 *  compute_top_for_bottom
 *
 *  If bottom is the last visible line, what line is the first visible line?
 *
 */
 
static int
compute_top_for_bottom(TextWindow *twp, int bottom)
{
    int top;
    
    top = bottom;
    while (top > 0) {
        if (TEGetHeight(bottom+1, top, twp->base.TE) <= twp->pane_height) {
            top--;
        }
        else break;
    }
    return top;
}

/*
 *  compute_bottom_for_top
 *
 *  If top is the first visible line, what line is the last visible line?
 *
 */
 
static int
compute_bottom_for_top(TextWindow *twp, int top)
{
    int bottom, last;

    last = (**(twp->base.TE)).nLines - 1;
    bottom = top;
    while (bottom < last) {
        if (TEGetHeight(bottom+2, top+1, twp->base.TE) <= twp->pane_height) {
            bottom++;
        }
        else break;
    }
    return bottom;
}

/*
 **********************************************************************
 *  TEXT EDIT SUPPORT
 **********************************************************************
 */

/*
 *  text_clear
 *
 */

void
text_clear(TEHandle TE)
{
    TESetSelect(0, 0, TE); /* turn off selection, if any */
    (**TE).selEnd = (**TE).teLength;
    TEDelete(TE);
}

/*
 *  text_trim
 *
 */

void
text_trim(TEHandle TE, int n)
{
    int len = (**TE).lineStarts[n];
    TESetSelect(len, len, TE); /* turn off selection, if any */
    (**TE).selEnd = (**TE).teLength;
    TEDelete(TE);
}

/*
 *  text_append_line
 *
 */

void
text_append_line(TEHandle TE, const char *the_line)
{
    text_append(TE, the_line);
    TEInsert("\r", 1, TE); /* don't use TEKey, it hides sprite */
}

/*
 *  text_append
 *
 */

void
text_append(TEHandle TE, const char *text)
{
    int len = (**TE).teLength;
    if ((**TE).selStart != len)
        TESetSelect(len, len, TE);
    TEInsert(text, strlen(text), TE);
}

/*
 *  text_contents
 *
 */

void
text_contents(TEHandle TE, char *dest)
{
    int n;

    n = (**TE).teLength;
    memcpy(dest, *(TEGetText(TE)), n);
    dest[n] = '\0';
}

/*
 *  text_activate
 *
 */

void
text_activate(TEHandle TE, long_boolean activate)
{
    if (activate) {
        TEActivate(TE);
    }
    else {
        TEDeactivate(TE);
    }
}

/*
 *  text_edit
 *
 */

void
text_edit(TEHandle TE, int item)
{
    switch(item) {
        case idleCommand:
            TEIdle(TE);
            break;
        case copyCommand:
            if (ZeroScrap() == 0) {
                TECopy(TE);
            }
            break;
        case cutCommand:
            if (ZeroScrap() == 0) {
                TECut(TE);
            }
            break;
        case pasteCommand:
            TEPaste(TE);
            break;
        case clearCommand:
            TEDelete(TE);
            break;
        case selectAllCommand:
            TESetSelect(0, (**TE).teLength, TE);
            break;
    }
}

/*
 *  text_key
 *
 */

void
text_key(TEHandle TE, short modifiers, char ch)
{
    if (ch == '\b') {
        TEKey(ch, TE);
    }
    else if ((ch == controlBKey) || (ch == leftArrowKey)) {
        backwards_character(TE);
    }
    else if ((ch == controlFKey) || (ch == rightArrowKey)) {
        forwards_character(TE);
    }
    else if (ch == controlUKey) {
        TESetSelect(0, (**TE).teLength, TE);
        TEDelete(TE);
    }
    else if (!isprint(ch)) {
        SysBeep(30);
    }
    else {
        TEKey(ch, TE);
    }
    TESelView(TE);
}

/*
 *  text_has_contents
 *
 */

long_boolean
text_has_contents(TEHandle TE)
{
    return (**TE).teLength > 0;
}

/*
 *  text_has_selection
 *
 */

long_boolean
text_has_selection(TEHandle TE)
{
    return (**TE).selStart != (**TE).selEnd;
}

/*
 *  text_menu_setup
 *
 */

void
text_menu_setup(TEHandle TE, long_boolean read_only)
{
    if (text_has_contents(TE)) {
        EnableItem(edit_menu, selectAllCommand);
        if (!read_only) {
            EnableItem(edit_menu, clearCommand);
        }
    }
    if (text_has_selection(TE)) {
        EnableItem(edit_menu, copyCommand);
        if (!read_only) {
            EnableItem(edit_menu, cutCommand);
        }
    }
    if (!read_only) {
        EnableItem(edit_menu, pasteCommand);
    }
}

/*
 *  text_selected_line
 *
 *  Return the index of the line containing the selection start.
 *
 */

int
text_selected_line(TEHandle TE)
{
   return text_offset_to_line(TE, (**TE).selStart);
}

/*
 *  text_offset_to_line
 *
 *  Return the index of the line containing the specified character offset.
 *
 */

int
text_offset_to_line(TEHandle TE, int offset)
{
    int current;

    current=0;
    while (offset >= (**TE).lineStarts[current]) {
        current++;
    }
    return current-1;
}

/*
 *  backwards_character
 *
 */

void
backwards_character(TEHandle TE)
{
    TEKey(leftArrowKey, TE);
}

/*
 *  forwards_character
 *
 */

void
forwards_character(TEHandle TE)
{
    TEKey(rightArrowKey, TE);
}
