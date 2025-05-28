/*
 *  macrec.c - reconcile point utility window
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
#include <string.h> /* for memcpy, memmove */

WindowPtr            recpointWindow;
TEHandle             recpointTE;
TextWindow           recpointTW;

extern WindowPtr     myWindow;

static char          recpointtext[10000];
static char          *recpointptr;
static int           recpointcount;
static int           mouse_up_time;
static int           mouse_up_selection;
static int           marker;

static void setup_window(void);
static void compute_rects(TextWindow *twp);
static void resize_window(TextWindow *twp, int delta_v, int delta_h);
static void update_window(TextWindow *twp);
static void fixup_window_position(void);
static void edit_window(TextWindow*twp, int item);
static void mouse_down(TextWindow *twp, EventRecord *e);
static void mouse_down_in_text(TextWindow *twp, EventRecord *e);
static void set_selection(TextWindow *twp, Point p);

/*
 *  recpoint_begin
 *
 *  This function is called to start filling the menu.
 *
 */

void
recpoint_begin(void)
{
    if (recpointWindow == NULL) {
        setup_window();
        if (recpointWindow == NULL) {
            return;
        }
    }
    window_select((Window *)&recpointTW);
    recpointptr = &recpointtext[0];
    recpointcount = 0;
    marker = -1;
}

/*
 *  recpoint_add
 *
 *  This function adds one line to the menu.
 *
 */

void
recpoint_add(const char *text)
{
    register const char *p;
    register char *q;
    register int ch;

    if (recpointWindow == NULL) {
        return;
    }
    if (text[0] != '\0') {
        /* we ignore empty lines */
        if (text[0] == '-') {
            marker = recpointcount;
        }
        p = text;
        q = recpointptr;
        while (ch = *p++) *q++ = ch;
        *q++ = '\r';
        recpointptr = q;
        recpointcount++;
    }
}

/*
 *  recpoint_end
 *
 *  This function is called after the last line has been added.
 *
 */

void
recpoint_end(void)
{
    if (recpointWindow == NULL) {
        return;
    }
    text_window_set_text(&recpointTW, recpointtext, recpointptr-&recpointtext[0]);
    if (marker >= 0) {
        /* don't use TESetSelect because a full update will soon be done */
        (**recpointTE).selStart = (**recpointTE).lineStarts[marker];
        (**recpointTE).selEnd = (**recpointTE).lineStarts[marker+1];
    }
    adjust_cursor();
}

/*
 *  recpoint_invalidate
 *
 */

void
recpoint_invalidate(void)
{
    if (recpointWindow == NULL) {
        return;
    }
    window_close((Window *)&recpointTW);
}

/*
 *  recpoint_activate
 *
 */

void
recpoint_activate(long_boolean activate)
{
    if (recpointWindow == NULL) {
        return;
    }
    DrawGrowIcon(recpointWindow);
    text_window_activate(&recpointTW, activate);
}

/*
 *  setup_window
 *
 */

static void
setup_window(void)
{
    FontInfo   myInfo;
    int        height;
    static WindowRecord wr;
    
    recpointWindow = get_new_window(RecpointWindow, &wr, 0);
    if (recpointWindow == NULL) {
        return;
    }
    recpointTW.base.window = recpointWindow; /* needed for compute_rects */
    SetPort(recpointWindow);
    TextFont(geneva);
    TextSize(10);
    fixup_window_position();
    compute_rects(&recpointTW);

    text_window_setup(&recpointTW, recpointWindow, FALSE);
    recpointTW.base.root_window     = (Window *)       &outputTW;
    recpointTW.base.kind            =                  UtilityKind;
    recpointTW.base.resizeSubMethod = (ResizeMethod)   resize_window;
    recpointTW.base.editMethod      = (EditMethod)     edit_window;
    recpointTW.base.mouseDownMethod = (EventMethod)    mouse_down;
    recpointTW.base.updateMethod    = (Method)         update_window;

    recpointTE = recpointTW.base.TE;
    (**recpointTE).crOnly = -1; /* don't wrap lines */

#if 0
    /* auto scrolling seems to be buggy */
    TEAutoView(true, recpointTE); /* enable auto scrolling */
#endif

}    

/*
 *  fixup_window_position
 *
 */

static void
fixup_window_position()
{
    int        left, right, swidth;
    Rect       winpos, winpos1;
    int        frame = 1;   /* width of window frame */
    int        shadow = 1;  /* width of window shadow (right side only) */

    window_global_portrect(recpointWindow, &winpos);
    window_global_portrect(myWindow, &winpos1);

    swidth = StringWidth("\p12 matrix interlocked phantom columns");
    swidth += SBarWidth + 10;
    left = winpos1.right + frame;
    right = left + swidth;
    if (right > (screenBits.bounds.right - (frame + shadow))) {
        right = (screenBits.bounds.right - (frame + shadow));
        left = right - swidth;
    }
    SizeWindow(recpointWindow, right-left, winpos1.bottom-winpos1.top, true);
    MoveWindow(recpointWindow, left, winpos1.top, false);
}

/*
 *  compute_rects
 *
 */

void
compute_rects(TextWindow *twp)
{
    Rect r;

    r = twp->base.window->portRect;

    twp->vScrollRect = r;
    twp->vScrollRect.right += 1;
    twp->vScrollRect.bottom -= (SBarWidth - 2);
    twp->vScrollRect.top -= 1;
    twp->vScrollRect.left = twp->vScrollRect.right-SBarWidth;

    twp->viewRect = r;
    twp->viewRect.right -= SBarWidth;
    twp->viewRect.bottom -= SBarWidth;
    InsetRect(&twp->viewRect, 4, 4);
}

/*
 *  update_window
 *
 */

static void
update_window(TextWindow *twp)
{
    WindowPtr w = twp->base.window;
    EraseRect(&w->portRect);
    DrawGrowIcon(w);
    UpdateControls(w, w->visRgn);
    TEUpdate(&w->portRect, recpointTE);
}

/*
 *  resize_window
 *
 */

static void
resize_window(TextWindow *twp, int delta_v, int delta_h)
{
    compute_rects(twp);
    text_window_set_view(twp);
    text_window_resize(twp, delta_v, delta_h);
}

/*
 *  edit_window
 *
 */

static void
edit_window(TextWindow *twp, int item)
{
    /* utility windows act as an extension of the main window */
    main_window_edit(item);
}

/*
 *  mouse_down
 *
 */

static void
mouse_down(TextWindow *twp, EventRecord *e)
{
    if (PtInRect(e->where, &(**twp->base.TE).viewRect)) {
        mouse_down_in_text(twp, e);
    }
    else {
        text_window_standard_mouse_down(twp, e);
    }
}

/*
 *  mouse_down_in_text
 *
 */

static void
mouse_down_in_text(TextWindow *twp, EventRecord *e)
{
    Point p;

    while (StillDown()) {
        GetMouse(&p);
        set_selection(twp, p);
    }
}

/*
 *  set_selection
 *
 */

static void
set_selection(TextWindow *twp, Point p)
{
    int n, start, end, mstart, mend, mlen;
    TEHandle TE;
    char buf[100];
    char *text;
    Rect r;
    extern int reconcile_max_point;

    TE = twp->base.TE;
    n = text_window_select_line(twp, p);

    /* clip value to acceptable range for reconcile point */

    if (n < ((**TE).nLines - reconcile_max_point - 1)) {
        n = ((**TE).nLines - reconcile_max_point - 1);
    }
    else if (n > ((**TE).nLines - 1)) {
        n = (**TE).nLines - 1;
    }
    
    text = *(**TE).hText;
    if (n < marker) {
        /* move marker before line N */
        mstart = (**TE).lineStarts[marker];
        mend = (**TE).lineStarts[marker+1];
        mlen = mend - mstart;
        memcpy(buf, &text[mstart], mlen); /* move marker into buffer */
        start = (**TE).lineStarts[n];
        memmove(&text[start+mlen], &text[start], mstart-start);
        memcpy(&text[start], buf, mlen); /* move marker back */
        TEDeactivate(TE);
        TECalText(TE);
        TESetSelect(start, start+mlen, TE); /* select marker */
        TEActivate(TE);
        r = recpointWindow->portRect;
        r.top = (**TE).destRect.top + n * (**TE).lineHeight;
        r.bottom = r.top + (marker - n + 1) * (**TE).lineHeight;
        InvalRect(&r);
        display_update();
        marker = n;
        set_recpoint_target((**TE).nLines - marker - 1);
    }
    else if (n > marker) {
        /* move marker after line N */
        mstart = (**TE).lineStarts[marker];
        mend = (**TE).lineStarts[marker+1];
        mlen = mend - mstart;
        memcpy(buf, &text[mstart], mlen); /* move marker into buffer */
        start = (**TE).lineStarts[marker+1];
        end = (**TE).lineStarts[n+1];
        memmove(&text[mstart], &text[start], end-start);
        memcpy(&text[end-mlen], buf, mlen); /* move marker back */
        TEDeactivate(TE);
        TECalText(TE);
        TESetSelect(end-mlen, end, TE); /* select marker */
        TEActivate(TE);
        r = recpointWindow->portRect;
        r.top = (**TE).destRect.top + marker * (**TE).lineHeight;
        r.bottom = r.top + (n - marker + 1) * (**TE).lineHeight;
        InvalRect(&r);
        display_update();
        marker = n;
        set_recpoint_target((**TE).nLines - marker - 1);
    }
}
