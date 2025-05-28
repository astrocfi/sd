/*
 *  macwdef.c - window definition function for utility windows
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

#include <SetUpA4.h>

static long doit(short varCode, WindowPtr w, short message, long param);
static long draw_window(WindowPtr w);
static void convert_to_global(Rect *rp, WindowPtr w);

pascal long
main(short varCode, WindowPtr w, short message, long param)
{
    long result;

    RememberA0();
    SetUpA4();
    result = doit(varCode, w, message, param);
    RestoreA4();
    return result;
}

/*
 *  This window definition function differs from the standard one
 *  only in how it draws the title bar.  It delegates all other
 *  requests to the standard window definition function.
 *
 */

static long
doit(short varCode, WindowPtr w, short message, long param)
{
    Handle h;
    long result;
    pascal long (*p)(short varCode, WindowPtr w, short message, long param);

    if ((message == wDraw) && ((param & 0177777) == 0)) {
        return draw_window(w);
    }
    else {
        h = GetResource('WDEF', 0);
        if (h) {
            HLock(h);
            p = (void *) *h;
            result = (*p)(varCode, w, message, param);
            HUnlock(h);
            return result;
        }
        return 0;
    }
}

static long
draw_window(WindowPtr w)
{
    WindowPeek wp = (WindowPeek) w;
    Rect r, r1, r2;
    short width, halfwidth, midpoint;
    StringHandle sh;
    RgnHandle save, rect, new;
    static Pattern pat = {0xAA, 0, 0xAA, 0, 0xAA, 0, 0xAA, 0};

    if (wp->visible) {

        /* frame around contents */

        r = w->portRect;
        convert_to_global(&r, w);
        InsetRect(&r, -1, -1);
        FrameRect(&r);

        /* shadow */

        r1 = r;
        r1.top -= 18;
        OffsetRect(&r1, 1, 1);
        r2 = r1;
        r1.top = r1.bottom - 1;
        FrameRect(&r1);
        r2.left = r2.right - 1;
        FrameRect(&r2);

        /* title bar (drag region) */

        r.bottom = r.top + 1;
        r.top = r.bottom - 19;
        FrameRect(&r);
        InsetRect(&r, 1, 1);
        FillRect(&r, pat);
        r2 = r;

        /* close box */

        if (wp->goAwayFlag) {
            r.top += 2;
            r.bottom -= 2;
            r.left += 7;
            r.right = r.left + 13;
            EraseRect(&r);
            InsetRect(&r, 1, 1);
            FrameRect(&r);
        }

        /* title */

        width = wp->titleWidth;
        if (width > 0) {
            if (width > (w->portRect.right - w->portRect.left - 64))
                width = (w->portRect.right - w->portRect.left - 64);
            midpoint = (r2.left + r2.right) / 2;
            halfwidth = width / 2;
            r2.left = midpoint - halfwidth - 6;
            r2.right = midpoint + (width - halfwidth) + 6;
            EraseRect(&r2);
            InsetRect(&r2, 6, 0);
            save = NewRgn();
            rect = NewRgn();
            new = NewRgn();
            GetClip(save);
            RectRgn(rect, &r2);
            SectRgn(save, rect, new);
            SetClip(new);
            sh = wp->titleHandle;
            MoveTo(r2.left, r2.bottom-4);
            DrawString(*sh);
            SetClip(save);
            DisposeRgn(save);
            DisposeRgn(rect);
            DisposeRgn(new);
        }
    }
    return 0;
}

static void
convert_to_global(Rect *rp, WindowPtr w)
{
    int version;
    Rect bounds;

    version = ((CGrafPtr)w)->portVersion;
    if (((version >> 14) & 3) == 3) {
        /* we have a color port */
        bounds = (**((CGrafPtr)w)->portPixMap).bounds;
    }
    else {
        bounds = w->portBits.bounds;
    }
    OffsetRect(rp, -(bounds.left), -(bounds.top));
}
