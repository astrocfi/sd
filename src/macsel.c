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

/* these specify how many items in each dialog box */
/* WARNING: This must match the number of radio buttons in the resource file */

#define NUM_QUANTIFIERS 8
#define NUM_SELECTORS last_selector_kind
#define NUM_DIRECTIONS last_direction_kind

/* these specify how the items are arranged visually within the dialog box,
   specifically the number of items in each column.  This is used only for
   controlling what happens when a user presses an arrow key. */
/* WARNING: This must match the actual layout in the resource file or else
   arrow key behavior will be erratic */

#define SEL_COL_LENGTH 10
#define QUANT_COL_LENGTH 1
#define DIR_COL_LENGTH 4

/* items in the popup */
/* WARNING: Dialog Box items in the Resource File must be numbered in this order! */
enum {
    AcceptButton = 1,
    CancelButton,
    TextItem,
    DefaultIndicator,
    FirstButton
};
/* these change with the type of popup */
static int NumButtons;
static int ColLength;
static int LastButton;

static void use_key(DialogWindow *dwp, short modifiers, char ch);
static void fixup_window_position(WindowPtr w);
static int mac_do_any_popup(int DialogID, int numitems, int col_length);
static int current_value;
static DialogPtr    the_dialog;
static DialogWindow the_DW;

/* These 3 routines are called from the user interface.  They actually call
   mac_do_any_popup to do the real work. */

int mac_do_selector_popup(void)
{
   return(mac_do_any_popup(OldSelectDialog, NUM_SELECTORS, SEL_COL_LENGTH));
}  
int mac_do_quantifier_popup(void)
{
   return(mac_do_any_popup(QuantifierDialog, NUM_QUANTIFIERS, QUANT_COL_LENGTH));
}  
int mac_do_direction_popup(void)
{
   return(mac_do_any_popup(DirectionDialog, NUM_DIRECTIONS, DIR_COL_LENGTH));
}  

int
mac_do_any_popup(int DialogID, int numitems, int col_length)
{
    short item;
    int result;
    EventRecord e;
    
    NumButtons = numitems;
    ColLength = col_length;
    LastButton = FirstButton + NumButtons - 1;
    
    /*  present dialog box */
    
    the_dialog = GetNewDialog(DialogID, 0L, (WindowPtr) -1L);
    if (the_dialog == NULL) {
        return POPUP_DECLINE;
    }
    dialog_setup(&the_DW, the_dialog);
    the_DW.base.kind = ModalKind;
    the_DW.default_item = AcceptButton;
    the_DW.base.keyMethod = (KeyMethod) use_key;
    fixup_window_position((WindowPtr) the_dialog);
    setup_dialog_box(the_dialog, DefaultIndicator); /* border around default button */
    current_value = 1;

    radio_set(the_dialog, FirstButton, LastButton, FirstButton + current_value - 1);
    window_select((Window *)&the_DW);

    /*  engage in dialog  */
        
    result = -1;

    while (result < 0) {
        movable_modal_dialog(&the_DW);
        if ((the_DW.item >= FirstButton) && (the_DW.item <= LastButton)) {
            current_value = the_DW.item - FirstButton + 1;
            radio_set(the_dialog, FirstButton, LastButton,
                       FirstButton + current_value - 1);
            continue;
        }
        else switch (the_DW.item) {
            case AcceptButton:
                result = current_value;
                break;
            case CancelButton:
                result = POPUP_DECLINE;
                break;
         }
    }

    /* take down dialog */

    window_close((Window *)&the_DW);
    DisposDialog(the_dialog);
    return(result);
}

/*
 *  use_key: interprets keys pressed while dialog box is presented
 *
 */

static void
use_key(DialogWindow *dwp, short modifiers, char ch)
{
    int new_value;

    switch (ch) {
      case downArrowKey:
        if (current_value + 1 <= NumButtons) current_value++;
        radio_set(the_dialog, FirstButton, LastButton, FirstButton + current_value - 1);
        break;
      case '\t':
      case rightArrowKey:
        if (current_value + ColLength <= NumButtons) current_value += ColLength;
        radio_set(the_dialog, FirstButton, LastButton, FirstButton + current_value - 1);
        break;
      case upArrowKey:
        if (current_value > 1) current_value--;
        radio_set(the_dialog, FirstButton, LastButton, FirstButton + current_value - 1);
        break;
      case leftArrowKey:
        if (current_value - ColLength > 0) current_value -= ColLength;
        radio_set(the_dialog, FirstButton, LastButton, FirstButton + current_value - 1);
        break;
      default:
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
