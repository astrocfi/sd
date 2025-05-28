/*
 *  macmatch.c - Matching Calls utility window
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

enum {
    ValidateControl = 128
};

WindowPtr            matchWindow;
TEHandle             matchTE;
TextWindow           matchTW;

extern WindowPtr     myWindow;

#define MATCH_BUFSIZ 30000

static int           match_active_menu; /* set by match_update */
static char          matchtext[MATCH_BUFSIZ];
static char          *matchptr;
static char          *matchend = &matchtext[MATCH_BUFSIZ - 100];
static int           matchcount;
static int           mouse_up_time;
static int           mouse_up_selection;
static ControlHandle validate_control;
static Rect          validate_rect;
static Rect          line_rect;

static void match_begin(void);
static void match_add(const char *user_input, const char *extension, const match_result *mr);
static void setup_window(void);
static void compute_rects(TextWindow *twp);
static void resize_window(TextWindow *twp, int delta_v, int delta_h);
static void update_window(TextWindow *twp);
static void update_grow_icon(TextWindow *twp);
static void fixup_window_position(void);
static void edit_window(TextWindow *twp, int item);
static void mouse_down(TextWindow *twp, EventRecord *e);
static void mouse_down_in_text(TextWindow *twp, EventRecord *e);
static int  set_selection(TextWindow *twp, Point p);
static void control_method(TextWindow *twp, ControlHandle h, short part_code);

/*
 *  match_begin
 *
 *  This function is called to start filling the matching calls window.
 *
 */

static void
match_begin(void)
{
    if (matchWindow == NULL) {
        setup_window();
        if (matchWindow == NULL) {
            return;
        }
    }
    window_select((Window *)&matchTW);
    matchptr = &matchtext[0];
    matchcount = 0;
}

/*
 *  match_add
 *
 *  This function adds one command to the matching calls window.
 *
 */

static void
match_add(const char *user_input, const char *extension, const match_result *mr)
{
    register const char *p;
    register char *q;
    register int ch;

    if (matchptr < matchend) {
        p = user_input;
        q = matchptr;
        while (ch = *p++) *q++ = ch;
        p = extension;
        while (ch = *p++) *q++ = ch;
        *q++ = '\r';
        matchptr = q;
        matchcount++;
    }
}

/*
 *  match_end
 *
 *  This function is called after the last command is added.
 *
 */

static void
match_end(void)
{
    text_window_set_text(&matchTW, matchtext, matchptr-&matchtext[0]);
    adjust_cursor();
}

/*
 *  match_invalidate
 *
 */

void
match_invalidate(void)
{
    if (matchWindow == NULL) {
        return;
    }
    window_close((Window *)&matchTW);
    text_clear(matchTE);
    text_window_adjust(&matchTW);
}

/*
 *  match_activate
 *
 */

void
match_activate(long_boolean activate)
{
    if (matchWindow == NULL) {
        return;
    }
    if (activate) {
        update_grow_icon(&matchTW);
        HiliteControl(validate_control, 0);
    }
    else {
        update_grow_icon(&matchTW);
        HiliteControl(validate_control, 255);
    }
    text_window_activate(&matchTW, activate);
}

/*
 *  match_flash_validate
 *
 */

void
match_flash_validate(void)
{
    if (validate_control != NULL) {
        flash_control(validate_control, inButton);
    }
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
    
    matchWindow = get_new_window(MatchWindow, &wr, 0);
    if (matchWindow == NULL) {
        return;
    }
    matchTW.base.window = matchWindow; /* needed for compute_rects */
    SetPort(matchWindow);
    TextFont(geneva);
    TextSize(10);
    fixup_window_position();
    compute_rects(&matchTW);

    text_window_setup(&matchTW, matchWindow, FALSE);
    matchTW.base.root_window     = (Window *)       &outputTW;
    matchTW.base.kind            =                  UtilityKind;
    matchTW.base.resizeSubMethod = (ResizeMethod)   resize_window;
    matchTW.base.editMethod      = (EditMethod)     edit_window;
    matchTW.base.mouseDownMethod = (EventMethod)    mouse_down;
    matchTW.base.controlMethod   = (ControlMethod)  control_method;
    matchTW.base.updateMethod    = (Method)         update_window;

    matchTE = matchTW.base.TE;
    (**matchTE).crOnly = -1; /* don't wrap lines */

#if 0
    /* auto scrolling seems to be buggy */
    TEAutoView(true, matchTE); /* enable auto scrolling */
#endif

    validate_control = GetNewControl(ValidateControl, matchWindow);
    MoveControl(validate_control, validate_rect.left, validate_rect.top);
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

    window_global_portrect(matchWindow, &winpos);
    window_global_portrect(myWindow, &winpos1);

    swidth = StringWidth("\p12 matrix interlocked phantom columns");
    swidth += SBarWidth + 10;
    left = winpos1.right + frame;
    right = left + swidth;
    if (right > (screenBits.bounds.right - (frame + shadow))) {
        right = (screenBits.bounds.right - (frame + shadow));
        left = right - swidth;
    }
    SizeWindow(matchWindow, right-left, winpos1.bottom-winpos1.top, true);
    MoveWindow(matchWindow, left, winpos1.top, false);
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

    line_rect.left = r.left;
    line_rect.right = r.right - SBarWidth;
    line_rect.top = r.bottom - 30;
    line_rect.bottom = line_rect.top;

    twp->viewRect = r;
    twp->viewRect.right -= SBarWidth;
    twp->viewRect.bottom = line_rect.top;
    InsetRect(&twp->viewRect, 4, 4);

    validate_rect.top = line_rect.top + 5;
    validate_rect.left = line_rect.left + 13;
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
    update_grow_icon(twp);
    UpdateControls(w, w->visRgn);
    MoveTo(line_rect.left, line_rect.top);
    LineTo(line_rect.right, line_rect.top);
    TEUpdate(&w->portRect, matchTE);
}

/*
 *  update_grow_icon
 *
 *  This version avoids drawing the horizontal scroll bar line.
 *
 */

static void
update_grow_icon(TextWindow *twp)
{
    RgnHandle old_clip;
    Rect r;
    WindowPtr w;

    w = twp->base.window;
    SetPort(w);
    old_clip = NewRgn();
    GetClip(old_clip);
    r = w->portRect;
    r.left = r.right - SBarWidth + 1;
    ClipRect(&r);
    DrawGrowIcon(w); /* erases grow icon if "inactive" */
    SetClip(old_clip);
    DisposeRgn(old_clip);
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
    MoveControl(validate_control, validate_rect.left, validate_rect.top);
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
    int start, end, n, click, second_click, firstn;
    Point p;

    second_click = ((TickCount() - mouse_up_time) < GetDblTime());

    firstn = set_selection(twp, e->where);
    n = firstn;
    while (StillDown()) {
        GetMouse(&p);
        n = set_selection(twp, p);
        }
    mouse_up_time = TickCount();
    click = ((mouse_up_time - e->when) < GetDblTime());
        
    if (click && (n == firstn) && (n >= 0)) {
        if (!second_click) {
            /* first click means stuff input buffer */
            start = (**twp->base.TE).selStart;
            end = (**twp->base.TE).selEnd;
            input_stuff(&matchtext[start], end-start-1);
        }
        else if (n == mouse_up_selection) {
            /* second click in same place means accept input */
            input_accept();
        }
        mouse_up_selection = n;
    }
    else {
        mouse_up_selection = -1;
    }
}

/*
 *  set_selection
 *
 */

static int
set_selection(TextWindow *twp, Point p)
{
    int n, start, end;
    TEHandle TE;

    TE = twp->base.TE;
    n = text_window_select_line(twp, p);

    if ((n >= 0) && (n < (**TE).nLines)) {
        start = (**TE).lineStarts[n];
        end = (**TE).lineStarts[n+1];
        if ((start != (**TE).selStart) || (end != (**TE).selEnd)) {
            TESetSelect(start, end, TE);
            display_update();
            /* TESelView(TE); */
            /* text_window_update_vscroll_pos(twp); */
        }
        return n;
    }
    return -1;
}

/*
 *  control_method
 *
 */

static void
control_method(TextWindow *twp, ControlHandle h, short part_code)
{
    match_validate();
}

/*
 *  match_update
 *
 *  This function updates the call menu based on the current input.  It does
 *  not validate the calls against the current setup.
 *
 */

void
match_update(int which_menu)
{
    char user_input[200];

    match_active_menu = which_menu;
    typein_get_contents(user_input);
    match_begin();
    if (matchWindow == NULL) {
        return;
    }
    match_user_input(user_input, match_active_menu, 0, 0, (show_function)match_add, FALSE);
    match_end();
}

/*
 *  match_validate
 *
 *  This function updates the call menu based on the current input.  It also
 *  validates all of the calls (currently not the concepts) against the
 *  current setup.
 *
 */

void
match_validate(void)
{
    char user_input[200];

    busy_cursor();
    typein_get_contents(user_input);
    match_begin();
    if (matchWindow == NULL) {
        return;
    }
    match_user_input(user_input, match_active_menu, 0, 0, (show_function)match_add, TRUE);
    match_end();
}
