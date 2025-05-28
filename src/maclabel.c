/*
 *  maclabel.c - sequence label dialog box
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
#include <string.h> /* strcpy */

/* dialog items in set-label dialog box */

enum {
    labelAccept = 1,
    labelCancel,
    labelText,
    labelDummy1,
    labelUser
};

static void label_mouse_down(DialogWindow *dwp, EventRecord *e);
static void label_control(DialogWindow *dwp, ControlHandle h, short part_code);
static void label_menu_setup(DialogWindow *dwp);
 
static DialogWindow labelDW;
static DialogPtr label_dialog;
static char sequence_label[250];  /* user specified label for sequence */

void
label_clear(void)
{
    sequence_label[0] = '\0';
}

void
label_get(char *dest)
{
    strcpy(dest, sequence_label);
}

void
open_label_dialog(void)
{
    static DialogRecord dr;
    Handle h;
    char buf[256];

    if (label_dialog == NULL) {
        label_dialog = GetNewDialog(LabelDialog, &dr, (WindowPtr) -1L);
        if (label_dialog == NULL) {
            return;
        }
        dialog_setup(&labelDW, label_dialog);
        labelDW.default_item          =                  labelAccept;
        labelDW.base.TE               = ((DialogPeek)label_dialog)->textH;
        labelDW.base.controlMethod    = (ControlMethod)  label_control;
        labelDW.base.mouseDownMethod  = (EventMethod)    label_mouse_down;
        labelDW.base.menuSetupMethod  = (Method)         label_menu_setup;
        setup_dialog_box(label_dialog, labelUser);
    }

    /* setting the text may not be needed anymore */

    h = ditem(label_dialog, labelText);
    strcpy(buf, sequence_label);
    SetIText(h, CtoPstr(buf));
    SelIText(label_dialog, labelText, 0, strlen(buf));
    window_select((Window *)&labelDW);
}

static void
label_mouse_down(DialogWindow *dwp, EventRecord *e)
{
    TEHandle TE = dwp->base.TE;
    if (PtInRect(e->where, &(**TE).viewRect)) {
        TEClick(e->where, (e->modifiers & shiftKey) != 0, TE);
    }
    else {
        dialog_mouse_down(dwp, e);
    }
}

static void
label_control(DialogWindow *dwp, ControlHandle h, short part_code)
{
    Handle dih;
    Str255 buf;

    switch (dwp->item) {
      case labelAccept:
        dih = ditem(label_dialog, labelText);
        GetIText(dih, buf);
        strcpy(sequence_label, PtoCstr(buf));
        window_close((Window *)dwp);
        break;
      case labelCancel:
        window_close((Window *)dwp);
        break;
    }   
}

static void
label_menu_setup(DialogWindow *dwp)
{
    TEHandle TE = dwp->base.TE;

    EnableItem(apple_menu, hintsCommand);
    EnableItem(sd_menu, 0);
    EnableItem(sd_menu, editPreferencesCommand);
    EnableItem(sd_menu, compileDatabaseCommand);
    text_menu_setup(TE, FALSE);
}
