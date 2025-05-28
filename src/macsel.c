/*
 *  macsel.c - selector and quantifier popups
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

static DialogPtr    selector_dialog, quantifier_dialog;
static DialogWindow selectorDW, quantifierDW;
extern WindowPtr    myWindow;
static int          quantifier_current;
static int          selector_current;

#define NUM_SELECTORS 28
#define QUANTIFIER_MAX 7

enum {
    selAccept = 1,
    selCancel,
    selText,
    selUser,
    selMenu,
    selFirstButton = selMenu,
    selLastButton = selFirstButton + NUM_SELECTORS - 1
};

enum {
    manyAccept = 1,
    manyCancel,
    many1,
    many2,
    many3,
    many4,
    many5,
    many6,
    many7,
    manyUser,
    manyText
};

static void selector_set(int n);
static void selector_key(DialogWindow *dwp, short modifiers, char ch);
static void quantifier_key(DialogWindow *dwp, short modifiers, char ch);
static void quantifier_set(int n);
static void fixup_window_position(WindowPtr w);

/*
 *  mac_do_selector_popup
 *
 */

int
mac_do_selector_popup(void)
{
    short item;
    int result;
    EventRecord e;

    /*  present dialog  */
        
    if (selector_dialog == NULL) {
        selector_dialog = GetNewDialog(
            popup_control_available ? SelectDialog : OldSelectDialog,
            0L, (WindowPtr) -1L);
        if (selector_dialog == NULL) {
            return POPUP_DECLINE;
        }
        dialog_setup(&selectorDW, selector_dialog);
        selectorDW.base.kind = ModalKind;
        selectorDW.default_item = selAccept;
        selectorDW.base.keyMethod = (KeyMethod) selector_key;
        fixup_window_position((WindowPtr) selector_dialog);
        setup_dialog_box(selector_dialog, selUser); /* border around default button */
        selector_current = 1;
    }
    selector_set(selector_current);
    window_select((Window *)&selectorDW);

    /*  engage in dialog  */
        
    result = -1;

    while (result < 0) {
        movable_modal_dialog(&selectorDW);
        if (!popup_control_available &&
              (selectorDW.item >= selFirstButton) &&
              (selectorDW.item <= selLastButton)) {
            selector_set(selectorDW.item - selFirstButton + 1);
            continue;
        }
        switch (selectorDW.item) {
            case selAccept:
                if (popup_control_available) {
                    selector_set(dialog_get_control_value(selector_dialog, selMenu));
                }
                result = selector_current;
                break;
            case selCancel:
                result = POPUP_DECLINE;
                break;
         }
    }

    /* take down dialog */

    window_close((Window *)&selectorDW);
    return(result);
}

/*
 *  selector_set
 *
 */

static void
selector_set(int n)
{
    selector_current = n;
    if (popup_control_available) {
        dialog_set_control_value(selector_dialog, selMenu, n);
    }
    else {
        radio_set(selector_dialog, selFirstButton, selLastButton,
            selFirstButton + n - 1);
    }
}

/*
 *  selector_key
 *
 */

static void
selector_key(DialogWindow *dwp, short modifiers, char ch)
{
    int new_value;

    if (popup_control_available) {
        if (ch == rightArrowKey) {
            ch = downArrowKey;
        }
        else if (ch == leftArrowKey) {
            ch = upArrowKey;
        }
    }
    if (ch == '\t') {
        ch = downArrowKey;
    }

    switch (ch) {
      case '\t':
      case downArrowKey:
        new_value = (selector_current == NUM_SELECTORS) ? 1 : selector_current + 1;
        selector_set(new_value);
        break;
      case upArrowKey:
        new_value = (selector_current == 1) ? NUM_SELECTORS : selector_current - 1;
        selector_set(new_value);
        break;
     case leftArrowKey:
        new_value = selector_current - 10;
        if (new_value < 1) {
            new_value += 30;
            if (new_value > NUM_SELECTORS) {
                new_value -= 10;
            }
        }
        selector_set(new_value);
        break;
     case rightArrowKey:
        new_value = selector_current + 10;
        if (new_value > 30) {
            new_value -= 30;
        }
        if (new_value > NUM_SELECTORS) {
            new_value -= 20;
        }
        selector_set(new_value);
        break;
     default:
        dialog_key(dwp, modifiers, ch);
    }
}

/*
 *  mac_do_quantifier_popup
 *
 */

int
mac_do_quantifier_popup(void)
{
    int result, i;
    
    /*  present dialog  */
        
    /* the availability of popup controls corresponds to the availability
       of the movable modal dialog box definition */

    if (quantifier_dialog == NULL) {
        quantifier_dialog = GetNewDialog(
            popup_control_available ? QuantifierDialog : OldQuantifierDialog,
            0L, (WindowPtr) -1L);
        if (quantifier_dialog == NULL) {
            return POPUP_DECLINE;
        }
        dialog_setup(&quantifierDW, quantifier_dialog);
        quantifierDW.base.kind = ModalKind;
        quantifierDW.default_item = manyAccept;
        quantifierDW.base.keyMethod = (KeyMethod) quantifier_key;
        fixup_window_position((WindowPtr) quantifier_dialog);
        setup_dialog_box(quantifier_dialog, manyUser); /* border around default button */
        quantifier_set(1);
    }
    window_select((Window *)&quantifierDW);
    
    /*  engage in dialog  */
        
    result = -1;
    do {
        movable_modal_dialog(&quantifierDW);
        switch (quantifierDW.item) {
            case manyAccept:
                result = quantifier_current;
                break;
            case many1:
            case many2:
            case many3:
            case many4:
            case many5:
            case many6:
            case many7:
                quantifier_set(quantifierDW.item - many1 + 1);
                break;
            case manyCancel:
                result = POPUP_DECLINE;
                break;
        }
    } while (result == -1);

    /* take down dialog */

    window_close((Window *)&quantifierDW);
    return(result);
}

/*
 *  quantifier_set
 *
 */

static void
quantifier_set(int n)
{
    quantifier_current = n;
    radio_set(quantifier_dialog, many1, many7, many1 + n - 1);
}

/*
 *  quantifier_key
 *
 */

static void
quantifier_key(DialogWindow *dwp, short modifiers, char ch)
{
    if ((ch >= '1') && (ch <= ('0' + QUANTIFIER_MAX))) {
        dwp->item = many1 + (ch - '1');
    }
    else if ((ch == '\t') || (ch == rightArrowKey)) {
        int new_value = (quantifier_current == QUANTIFIER_MAX) ? 1
                          : quantifier_current + 1;
        dwp->item = many1 + new_value - 1;
    }
    else if (ch == leftArrowKey) {
        int new_value = (quantifier_current == 1) ? QUANTIFIER_MAX
                           : quantifier_current - 1;
        dwp->item = many1 + new_value - 1;
    }
    else {
        dialog_key(dwp, modifiers, ch);
    }
}

/*
 *  fixup_window_position
 *
 */

static void
fixup_window_position(WindowPtr w)
{
    int        left, right, width;
    Rect       winpos, winpos1;
    int        frame = 9; /* width of window frame */

    window_global_portrect(w, &winpos);
    window_global_portrect(myWindow, &winpos1);

    width = winpos.right - winpos.left;
    right = screenBits.bounds.right - frame;
    left = right - width;
    if (left < screenBits.bounds.left + frame) {
        left = screenBits.bounds.left + frame;
    }
    if (left > winpos1.right+frame+frame) {
        left = winpos1.right+frame+frame;
    }
    MoveWindow(w, left, winpos1.top+4, false);
}
