/* -*- mode:C; c-basic-offset:3; indent-tabs-mode:nil; -*- */

/*
 *  sdui-win.c - SD -- Microsoft Windows User Interface
 *
 *  Copyright (C) 1995, Robert E. Cays
 *
 *
 *  Permission to use, copy, modify, and distribute this software for
 *  any purpose is hereby granted without fee, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *  The author makes no representations about the suitability of this
 *  software for any purpose.  It is provided "as is" WITHOUT ANY
 *  WARRANTY, without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 */

static char *sdui_version = "4.0";


/* This file defines the following functions:
   uims_version_string
   uims_process_command_line
   uims_open_session
   uims_create_menu
   uims_get_startup_command
   uims_get_call_command
   uims_get_resolve_command
   uims_do_comment_popup
   uims_do_outfile_popup
   uims_do_getout_popup
   uims_do_abort_popup
   uims_do_neglect_popup
   uims_do_selector_popup
   uims_do_direction_popup
   uims_do_tagger_popup
   uims_get_number_fields
   uims_do_modifier_popup
   uims_add_new_line
   uims_reduce_line_count
   uims_begin_search
   uims_begin_reconcile_history
   uims_end_reconcile_history
   uims_update_resolve_menu
   uims_terminate
   uims_database_tick_max
   uims_database_tick
   uims_database_tick_end
   uims_database_error
   uims_fatal_error
   uims_bad_argument
   uims_final_exit
*/

#define WINVER 0x0400

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "paths.h"
/* Anything that includes sdmatch.h must include sd.h  What a loss! */
//#include "sd.h"
#include "database.h"
#include "sdui.h"
#include "sdmatch.h"
extern void initialize_concept_sublists(void);
extern void build_database(call_list_mode_t call_list_mode);
#include "resource.h"

/* Privately defined message codes. */
/* The user-definable message codes begin at 1024, according to winuser.h.
   However, our resources use up numbers in that range (and, in fact,
   start at 1003, but that's Developer Studio's business).  So we start at 2000. */
#define CM_REINIT      2000
/* The special key codes start at 128 (due to the way Sdtty
   handles them, so we subtract 128 as we embed them into the
   Windows command user segment. */
#define SPECIAL_KEY_OFFSET (CM_REINIT-128+1)


typedef struct {
   void *Root;
} CallListType;

typedef struct {
   Cstring Call;
   int nCallIndex;
   void *Next;
} CallType;

typedef struct {
   char Call[MAX_TEXT_LINE_LENGTH];
   void *Next;
} CallModifier;

typedef struct {
   char Line [90];
   void *Next;
} DisplayType;


#define ui_undefined -999

extern int main(int argc, char *argv[]);

LRESULT WINAPI DisplayWndProc (HWND hWindow, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI FontSelWndProc (HWND hWindow, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI FormationWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI CommentWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI GetoutWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI AboutWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam);

void ShowListBox (int WhichOne);
static Cstring szAppName = "Sd";


static char szComment         [MAX_TEXT_LINE_LENGTH];
static char szHeader          [MAX_TEXT_LINE_LENGTH];
static char szOutFilename     [MAX_TEXT_LINE_LENGTH];
static char szCallListFilename[MAX_TEXT_LINE_LENGTH];
static char szDatabaseFilename[MAX_TEXT_LINE_LENGTH];
static char szSequenceLabel   [MAX_TEXT_LINE_LENGTH];
static char szResolveWndTitle [MAX_TEXT_LINE_LENGTH];

static HINSTANCE GLOBhInstance;
static int GLOBnCmdShow;
static FARPROC lpfnOldCallsEditProc;
static LRESULT Status;
static LOGFONT LogFont;
static CHOOSEFONT cf;
static HMENU hMenu;
static HWND GLOBhMainWindow = NULL;
static HWND GLOBhMenuWindow = NULL;
static HWND hCallsLbox;
static HWND hCallsEbox;
static BOOL NewMenu = TRUE;
static BOOL MenuEnabled = FALSE;
static BOOL FileIsOpen = FALSE;
static BOOL WaitingForCommand;
static BOOL DontPrint;
static WORD ConceptListIndex[600];
static DWORD pdFlags;
static FILE *hInpFile;
static int nLastOne;
static int nSequenceNumber;
static int nDisplayWndWidth;
static int nDisplayWndHeight;
static int nVscrollPos;
static int nVscrollMax;
static int nVscrollInc;
static int nHscrollPos;
static int nHscrollMax;
static int nHscrollInc;
static int nLineCnt;
static int nYchar;
static int nXchar;
static int nCallIndex;
static int SpecialConceptMenu;
static int FontSize;
static int nConceptListLength;
static int nLevelConceptListLength;
static int nConceptTwice = -1;
static int nConceptSequential = -1;
static uims_reply MenuKind;
static CallListType call_menu_lists[NUM_CALL_LIST_KINDS + 2];
static DisplayType *DisplayRoot = NULL;
static DisplayType *CurDisplay = NULL;
static CallModifier *ModifierList = NULL;
static CallType *CurCall;

/* These control how big we think the menu windows
   are and where we center the selected item. */
#define PAGE_LEN 31
#define SCROLL_POINT 15


static int XVal = 10;
static int YVal = 30;
static int screensize = 42;   // Amount that we tell the scroll that we believe the screen holds
static int pagesize = 40;     // Amount we tell it to scroll if user clicks in dead scroll area
static int position = 0;
static int BottomFudge;
static int TextHeight;
static int TextWidth;


static uims_reply my_reply;
static long_boolean my_retval;

static RECT CallsClientRect;



static void YTC(HWND hwnd)
{
   MessageBox(GLOBhMainWindow, "Not yet available", "Under Construction", MB_OK);
}



extern void show_match(void)
{
}


static int dialog_menu_index;
static int dialog_menu_count;
static Cstring dialog_prompt;
static Cstring *dialog_menu;
static char user_input[INPUT_TEXTLINE_SIZE+1];



static void check_text_change(HWND hListbox, HWND hEditbox)
{
   char szCurrentSel[MAX_TEXT_LINE_LENGTH];
   char szLocalString[MAX_TEXT_LINE_LENGTH];
   int nLen;
   int nIndex;
   int i;
   char *p;
   char cCurChar;
   int matches;
   BOOL Match;
   BOOL changed_editbox = FALSE;

   /* Find out what the edit box contains now. */

   GetWindowText(hEditbox, szLocalString, MAX_TEXT_LINE_LENGTH);
   nLen = lstrlen(szLocalString) - 1;    /* Location of last character. */

   /* Only process stuff if it changed from what we thought it was.
      Otherwise we would unnecessarily process changes that weren't typed
      by the user but were merely the stuff we put in due to completion. */

   if (nLen >= 0 && strcmp(szLocalString, user_input)) {
      cCurChar = szLocalString[nLen];

      if (cCurChar == ' ' || cCurChar == '-') {
         /* extend only to one space or hyphen, inclusive */
         matches = match_user_input(nLastOne, FALSE, FALSE);
         user_match = static_ss.result;
         p = static_ss.extended_input;

         if (*p) {
            changed_editbox = TRUE;

            while (*p) {
               if (*p != ' ' && *p != '-') {
                  szLocalString[nLen++] = *p++;
                  szLocalString[nLen] = '\0';
               }
               else {
                  szLocalString[nLen++] = cCurChar;
                  szLocalString[nLen] = '\0';
                  goto pack_us;
               }
            }
         }
         else if (!static_ss.space_ok || matches <= 1) {
            szLocalString[nLen] = '\0';    /* Do *not* pack the character. */
            changed_editbox = TRUE;
         }
      }

   pack_us:

      strcpy(user_input, szLocalString);
      strcpy(static_ss.full_input, szLocalString);
      for (p=static_ss.full_input ; *p ; p++)
         *p = tolower(*p);


      /* Write it back to the window. */

      if (changed_editbox) {
         SendMessage(hEditbox, WM_SETTEXT, 0, (LPARAM) szLocalString);
         /* This moves the cursor to the end of the text, apparently. */
         SendMessage(hEditbox, EM_SETSEL, MAX_TEXT_LINE_LENGTH - 1, MAX_TEXT_LINE_LENGTH - 1);
      }

      if (szLocalString[nLen] == '-') {
         /* User wants next word in currently selected item */
         szLocalString[nLen] = 0;
         nIndex = SendMessage (hListbox, LB_GETCURSEL, 0, (LPARAM)0);
         SendMessage (hListbox, LB_GETTEXT, nIndex, (LPARAM)szCurrentSel);
         if (szCurrentSel[nLen] == '-')   /* Except when next word is a '-' */
            lstrcat (szLocalString, "-");
         else {
            i = nLen;
            Match = (strstr (szCurrentSel, szLocalString) == szCurrentSel);
            while (Match) {                 /* Get the next word */
               szLocalString[i] = cCurChar = szCurrentSel[i];
               if (cCurChar == 0 || cCurChar == ' ') {
                  szLocalString[i+1] = 0;
                  break;
               }
               i++;
            }
         }
         SendMessage (hEditbox, WM_SETTEXT, 0, (LPARAM)szLocalString);
         SendMessage (hEditbox, EM_SETSEL, MAX_TEXT_LINE_LENGTH - 1, MAX_TEXT_LINE_LENGTH - 1);
      }
   }

   /* Search list box for match on current string */

   nIndex = SendMessage(hListbox, LB_SELECTSTRING, (WPARAM) -1, (LPARAM) szLocalString);
   if (nIndex < 0 && lstrlen(szLocalString) == 0) {
      /* No match and no string. */
      nIndex = 0;  /* Select first entry in list box */
      SendMessage(hListbox, LB_SETCURSEL, 0, 0L);
   }

   /* If possible, scroll the list box so that
         current selection remains centered. */

   if (nIndex >= 0) {
      if (nIndex > SCROLL_POINT)
         SendMessage(hListbox, LB_SETTOPINDEX, nIndex - SCROLL_POINT, 0L);
      else
         SendMessage(hListbox, LB_SETTOPINDEX, 0, 0L);

      SendMessage(hListbox, LB_GETTEXT, nIndex, (LPARAM) szLocalString);
   }
}


/* Process "menu window" commands, including keystrokes that we stole from the edit box. */

void MenuWndOnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   CallType *CurCall;
   HDC hDC;
   SIZE Size;
   int nLongest;
   int i;
   int menu_length;
   int matches;
   int *item;
   int nIndex = 1;
   int nCount;
   int nMenuIndex;

   /* Look for special programmed keystrokes. */

   if (id >= SPECIAL_KEY_OFFSET+128) {
      modifier_block *keyptr;
      int nc = id - SPECIAL_KEY_OFFSET;
      if (nc < FCN_KEY_TAB_LOW || nc > FCN_KEY_TAB_LAST)
         return;      /* How'd this happen?  Ignore it. */

      keyptr = fcn_key_table_normal[nc-FCN_KEY_TAB_LOW];

      /* Check for special bindings like "delete line" or "page up".
         These always come from the main binding table, even if
         we are doing something else, like a resolve. */

      if (keyptr && keyptr->index < 0) {
         switch (keyptr->index) {
         case special_index_pageup:
            nIndex -= PAGE_LEN-1;
         case special_index_lineup:
            nIndex -= PAGE_LEN+1;
         case special_index_pagedown:
            nIndex += PAGE_LEN-1;
         case special_index_linedown:
            /* nIndex now tells how far we want to move forward or back in the menu.
               Change that to the absolute new position, by adding the old position. */
            nIndex += SendMessage(hCallsLbox, LB_GETCURSEL, 0, (LPARAM) 0);

            /* Clamp to the menu limits. */
            nCount = SendMessage(hCallsLbox, LB_GETCOUNT, 0, 0) - 1;
            if (nIndex > nCount) nIndex = nCount;
            if (nIndex < 0) nIndex = 0;

            /* Select the new item. */
            SendMessage(hCallsLbox, LB_SETCURSEL, nIndex, (LPARAM) 0);
            break;
         case special_index_deleteword:
         default:   /* Must be special_index_deleteline. */
            SendMessage(hCallsEbox, WM_SETTEXT, 0, (LPARAM)"");  /* Erase the edit box. */
            user_input[0] = '\0';
            static_ss.full_input[0] = '\0';
            static_ss.full_input_size = 0;
         }
         return;
      }

      /* No binding as "page up" or anything like that.  Look for menu-specific
         bindings like "split phantom boxes" or "find another". */

      if (nLastOne == match_startup_commands)
         keyptr = fcn_key_table_start[nc-FCN_KEY_TAB_LOW];
      else if (nLastOne == match_resolve_commands)
         keyptr = fcn_key_table_resolve[nc-FCN_KEY_TAB_LOW];
      else if (nLastOne < 0)
         return;   /* We are scanning for direction/selector/number/etc. */

      if (!keyptr) return;

      /* If we get here, this function key specifies a user action. */
      user_match = static_ss.result;
      user_match.match = *keyptr;
      user_match.indent = FALSE;
      goto got_match_item;
   }
   else {
      switch (id) {
#ifdef NONONO
      case CM_DELETELINE:
         SendMessage (hCallsEbox, WM_SETTEXT, 0, (LPARAM) "");
         return;
      case CM_DELETEWORD:
         GetWindowText (hCallsEbox, szLocalString, MAX_TEXT_LINE_LENGTH);
         nLen = lstrlen (szLocalString) - 1;
         while (nLen >= 0 && szLocalString[nLen] == ' ')
            szLocalString[nLen--] = 0;
         while (nLen >= 0 && szLocalString[nLen] != ' ')
            szLocalString[nLen--] = 0;
         while (nLen >= 0 && szLocalString[nLen] == ' ')
            szLocalString[nLen--] = 0;
         SendMessage (hCallsEbox, WM_SETTEXT, 0, (LPARAM) szLocalString);
         SendMessage (hCallsEbox, EM_SETSEL, MAX_TEXT_LINE_LENGTH, MAX_TEXT_LINE_LENGTH);
         return;
#endif
      case IDC_CALL_EDIT:
         if (codeNotify == EN_CHANGE) {
            check_text_change(hCallsLbox, hCallsEbox);
         }
         break;
      case IDC_CALL_LIST:
         if (codeNotify == LBN_SELCHANGE) {
            /* List box has the focus */
            SetFocus(hCallsEbox);     /* Give it back to edit box */
            return;
         }
         if (codeNotify != LBN_DBLCLK)
            return;               /* Ignore if not double click */
         /* Warning!  Fall through! */
      case IDC_CALL_ACCEPT:
         /* Warning!  Fell through! */
         matches = match_user_input(nLastOne, FALSE, FALSE);
         user_match = static_ss.result;

         /* We forbid a match consisting of two or more "direct parse" concepts, such as
            "grand cross".  Direct parse concepts may only be stacked if they are followed
            by a call.  The "match.next" field indicates that direct parse concepts
            were stacked. */

         if ((matches == 1 || matches - static_ss.yielding_matches == 1 || user_match.exact) &&
             ((!user_match.match.packed_next_conc_or_subcall &&
               !user_match.match.packed_secondary_subcall) ||
              user_match.match.kind == ui_call_select ||
              user_match.match.kind == ui_concept_select)) {

            /* The matcher found an acceptable (and possibly quite complex)
               utterance.  Use it directly. */

            SendMessage(hCallsEbox, WM_SETTEXT, 0, (LPARAM)"");  /* Erase the edit box. */
            goto got_match_item;
         }

         /* The matcher isn't happy, but we got an item from the menu.  Use it. */

         if ((nMenuIndex = SendMessage(hCallsLbox, LB_GETCURSEL, 0, 0L)) == LB_ERR)
            return;
 
         i = SendMessage (hCallsLbox, LB_GETITEMDATA, nMenuIndex, (LPARAM)0);

         SendMessage(hCallsEbox, WM_SETTEXT, 0, (LPARAM)"");  /* Erase the edit box. */
         user_match.match.index = LOWORD(i);
         user_match.match.kind = HIWORD(i);
         user_match.match.packed_next_conc_or_subcall = 0;
         user_match.match.packed_secondary_subcall = 0;
         user_match.match.call_conc_options = null_options;
         user_match.real_next_subcall = (match_result *) 0;
         user_match.real_secondary_subcall = (match_result *) 0;

         /* We have the needed info.  Process it and exit from the command loop.
            However, it's not a fully filled in match item from the parser.
            So we need to concoct a low-class match item. */

         if (nLastOne == match_number) {
         }
         else if (nLastOne == match_circcer) {
            user_match.match.call_conc_options.circcer =
               user_match.match.index+1;
         }
         else if (nLastOne >= match_taggers &&
                  nLastOne < match_taggers+NUM_TAGGER_CLASSES) {
            user_match.match.call_conc_options.tagger =
               ((nLastOne-match_taggers) << 5)+user_match.match.index+1;
         }
         else {
            if (user_match.match.kind == ui_concept_select) {
               user_match.match.concept_ptr =
                  &concept_descriptor_table[user_match.match.index];
            }
            else if (user_match.match.kind == ui_call_select) {
               user_match.match.call_ptr =
                  main_call_lists[parse_state.call_list_to_use][user_match.match.index];
            }
         }

         goto got_match_item;
      case CM_REINIT:
         SendMessage (hCallsLbox, LB_RESETCONTENT, 0, 0L);
         GetClientRect(hCallsLbox, &CallsClientRect);
         hDC = GetDC (hCallsLbox);
         SendMessage (hCallsLbox, WM_SETREDRAW, FALSE, 0L);
         nLongest = 0;

         if (nLastOne == match_number) {
            int iu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<number>");

            kind = 0;

            for (iu=0 ; iu<NUM_CARDINALS; iu++) {
               Cstring name = cardinals[iu];
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(iu, kind));
            }
         }
         else if (nLastOne == match_circcer) {
            unsigned int iu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<circulate replacement>");

            kind = 0;

            for (iu=0 ; iu<number_of_circcers ; iu++) {
               Cstring name = circcer_calls[iu]->menu_name;
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(iu, kind));
            }
         }
         else if (nLastOne >= match_taggers &&
                  nLastOne < match_taggers+NUM_TAGGER_CLASSES) {
            int tagclass = nLastOne - match_taggers;

            unsigned int iu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<tagging call>");

            kind = 0;

            for (iu=0 ; iu<number_of_taggers[tagclass] ; iu++) {
               Cstring name = tagger_calls[tagclass][iu]->menu_name;
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(iu, kind));
            }
         }
         else if (nLastOne == match_startup_commands) {
            Cstring *menu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<startup>");

            kind = ui_start_select;
            menu = startup_commands;
            menu_length = NUM_START_SELECT_KINDS;

            for (i=0 ; i<menu_length ; i++) {
               Cstring name = menu[i];
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(i, kind));
            }
         }
         else if (nLastOne == match_resolve_commands) {
            Cstring *menu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<resolve>");

            kind = ui_resolve_select;
            menu = resolve_command_strings;
            menu_length = number_of_resolve_commands;

            for (i=0 ; i<menu_length ; i++) {
               Cstring name = menu[i];
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(i, kind));
            }
         }
         else if (nLastOne == match_directions) {
            Cstring *menu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<direction>");

            kind = ui_special_concept;
            menu = &direction_names[1];
            menu_length = last_direction_kind;

            for (i=0 ; i<menu_length ; i++) {
               Cstring name = menu[i];
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(i, kind));
            }
         }
         else if (nLastOne == match_selectors) {
            Cstring *menu;
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) "<selector>");

            kind = ui_special_concept;
            menu = selector_menu_list;
            menu_length = last_selector_kind;

            for (i=0 ; i<menu_length ; i++) {
               Cstring name = menu[i];
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(i, kind));
            }
         }
         else {
            uims_reply kind;

            SetDlgItemText(GLOBhMenuWindow, IDC_CALL_TITLE, (LPSTR) menu_names[nLastOne]);

            kind = ui_call_select;

            for (CurCall = call_menu_lists[nLastOne].Root; CurCall ; CurCall = CurCall->Next) {
               int nMenuIndex;

               GetTextExtentPoint (hDC, CurCall->Call, lstrlen (CurCall->Call), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage (hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage (hCallsLbox, LB_ADDSTRING, 0, (LPARAM) CurCall->Call);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(CurCall->nCallIndex, kind));
            }

            if (allowing_all_concepts) {
               item = concept_list;
               menu_length = concept_list_length;
            }
            else {
               item = level_concept_list;
               menu_length = level_concept_list_length;
            }

            for (i=0 ; i<menu_length ; i++) {
               Cstring name = concept_descriptor_table[item[i]].menu_name;
               int nMenuIndex;

               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(item[i], ui_concept_select));
            }

            for (i=0 ;  ; i++) {
               Cstring name = command_menu[i].command_name;
               int nMenuIndex;

               if (!name) break;
               GetTextExtentPoint(hDC, name, lstrlen(name), &Size);
               if ((Size.cx > nLongest) && (Size.cx > CallsClientRect.right)) {
                  SendMessage(hCallsLbox, LB_SETHORIZONTALEXTENT, Size.cx, 0L);
                  nLongest = Size.cx;
               }

               nMenuIndex = SendMessage(hCallsLbox, LB_ADDSTRING, 0, (LPARAM) name);
               SendMessage(hCallsLbox, LB_SETITEMDATA, nMenuIndex,
                           (LPARAM) MAKELONG(i, ui_command_select));
            }
         }

         SendMessage (hCallsLbox, WM_SETREDRAW, TRUE, 0L);
         InvalidateRect (hCallsLbox, NULL, TRUE);

         ReleaseDC (hCallsLbox, hDC);
         SendMessage (hCallsLbox, LB_SETCURSEL, 0, (LPARAM) 0);
         return;
      }
   }

   return;

   /* We have the fully filled in match item.  Process it and exit from the command loop. */

 got_match_item:

   WaitingForCommand = FALSE;
   return;
}



static uint32 ctlbits = 0;

/*  This procedure hooks the edit control in the calls dialog box to
    capture the up and down arrow keys and send them back to the calls
    dialog box procedure. */

LRESULT WINAPI CallsEboxProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   if (Message == WM_KEYDOWN || Message == WM_SYSKEYDOWN) {
      int newparm = -99;

      if (wParam == VK_SHIFT)
         ctlbits |= 2;
      else if (wParam == VK_CONTROL)
         ctlbits |= 1;
      else if (wParam >= VK_PRIOR && wParam <= VK_HELP) {
         if (HIWORD(lParam) & KF_ALTDOWN) {
            if (ctlbits == 0)      newparm = wParam-VK_PRIOR+1+AEKEY+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 1) newparm = wParam-VK_PRIOR+1+CAEKEY+SPECIAL_KEY_OFFSET;
         }
         else {
            if (ctlbits == 0)      newparm = wParam-VK_PRIOR+1+EKEY+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 1) newparm = wParam-VK_PRIOR+1+CEKEY+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 2) newparm = wParam-VK_PRIOR+1+SEKEY+SPECIAL_KEY_OFFSET;
         }
      }
      else if (wParam >= VK_F1 && wParam <= VK_F12) {
         if (HIWORD(lParam) & KF_ALTDOWN) {
            if (ctlbits == 0)      newparm = wParam-VK_F1+1+AFKEY+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 1) newparm = wParam-VK_F1+1+CAFKEY+SPECIAL_KEY_OFFSET;
         }
         else {
            if (ctlbits == 0)      newparm = wParam-VK_F1+1+FKEY+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 1) newparm = wParam-VK_F1+1+CFKEY+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 2) newparm = wParam-VK_F1+1+SFKEY+SPECIAL_KEY_OFFSET;
         }
      }
      else if (wParam >= 'A' && wParam <= 'Z') {
         if (HIWORD(lParam) & KF_ALTDOWN) {
            if (ctlbits == 0)      newparm = wParam+ALTLET+SPECIAL_KEY_OFFSET;
            else if (ctlbits == 1) newparm = wParam+CTLALTLET+SPECIAL_KEY_OFFSET;
         }
         else {
            if (ctlbits == 1) newparm = wParam+CTLLET+SPECIAL_KEY_OFFSET;
         }
      }

      if (newparm != -99) {
         PostMessage(GLOBhMenuWindow, WM_COMMAND, newparm, 0);
         return 0;    // We don't pass the message back.
      }
      else {
         return CallWindowProc(lpfnOldCallsEditProc, hWnd, Message, wParam, lParam);
      }
   }
   else if (Message == WM_KEYUP || Message == WM_SYSKEYUP) {
      if (wParam == VK_SHIFT)
         ctlbits &= ~2;
      else if (wParam == VK_CONTROL)
         ctlbits &= ~1;
      return CallWindowProc(lpfnOldCallsEditProc, hWnd, Message, wParam, lParam);
   }
   else {
      return CallWindowProc(lpfnOldCallsEditProc, hWnd, Message, wParam, lParam);
   }
}


/* This is the WndProc for the "menu", that is, the stuff on the left side of the
   window.  It is actually a modeless dialog. */

#define MENUSIZEH 340
#define MENUSIZEV 514

// Amount we push down from top, so it looks nicer.
#define TVOFFSET 5
// Amount we push stuff over to right, because of menu on left.
#define THOFFSET (MENUSIZEH+5)


static BOOL MenuWndOnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
   hCallsLbox = GetDlgItem(hwnd, IDC_CALL_LIST);
   hCallsEbox = GetDlgItem(hwnd, IDC_CALL_EDIT);
   GetClientRect (hCallsLbox, &CallsClientRect);
   /* If we set the size to anything other than the values shown, it just clips
      the menu, or doesn't expand it.  The actual size that the menu window
      needs to be is, unfortunately, hard-wired in the resource file. */
   SetWindowPos(hwnd, NULL, 0, 0, MENUSIZEH, MENUSIZEV, SWP_NOZORDER);
   lpfnOldCallsEditProc = (FARPROC) SetWindowLong(hCallsEbox, GWL_WNDPROC, (LONG) CallsEboxProc);
   return TRUE;
}



LRESULT WINAPI MenuWndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
   switch (Message) {
      HANDLE_MSG(hwnd, WM_INITDIALOG, MenuWndOnInitDialog);
      HANDLE_MSG(hwnd, WM_COMMAND, MenuWndOnCommand);
   }
   return FALSE;
}


static BOOL Window1_OnCreate(HWND hwnd, LPCREATESTRUCT bar)
{
   TEXTMETRIC TextMetrics;

   HDC PaintDC = GetDC(hwnd);
   SelectFont(PaintDC, GetStockObject(OEM_FIXED_FONT));
   GetTextMetrics(PaintDC, &TextMetrics);
   ReleaseDC(hwnd, PaintDC);
   TextHeight = TextMetrics.tmHeight+TextMetrics.tmExternalLeading;
   TextWidth = TextMetrics.tmAveCharWidth;
   position = 0;
   SetScrollRange(hwnd, SB_VERT, 0, 50, TRUE);
   SetScrollPos(hwnd, SB_VERT, position, TRUE);
   return TRUE;
}


static void Window1_OnDestroy(HWND hwnd)
{
   PostQuitMessage(0);
}


static void Window1_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codenotify)
{
   switch(id) {
   case ID_FILE_ABOUTSD:
      DialogBox(GLOBhInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, (DLGPROC) AboutWndProc);
      break;
   case ID_FILE_EXIT:
      if (MessageBox(hwnd, "Do you really want to quit?", "Confirmation", MB_YESNO) == IDYES)
         final_exit (0);
      break;
   default:
      YTC(hwnd);
   }
}

uint32 peoplecolors[8] = {
   RGB(0, 255, 255),
   RGB(255, 0, 255),
   RGB(0, 255, 255),
   RGB(255, 0, 255),
   RGB(0, 255, 255),
   RGB(255, 0, 255),
   RGB(0, 255, 255),
   RGB(255, 0, 255)};

uint32 couplecolors[8] = {
   RGB(255, 0, 255),
   RGB(255, 0, 255),
   RGB(0, 255, 0),
   RGB(0, 255, 0),
   RGB(0, 255, 255),
   RGB(0, 255, 255),
   RGB(255, 255, 0),
   RGB(255, 255, 0)};

static uint32 *colorlist;

static void Window1_OnPaint(HWND hwnd)
{
   PAINTSTRUCT PaintStruct;
   int Y = TVOFFSET;
   int i;

   HDC PaintDC = BeginPaint(hwnd, &PaintStruct);

   SelectFont(PaintDC, GetStockObject(OEM_FIXED_FONT));
   (void) SetBkColor(PaintDC, RGB(0, 0, 0));
   (void) SetTextColor(PaintDC, RGB(255, 255, 255));

   CurDisplay = DisplayRoot;

   for (i=0 ; i<nLineCnt ; i++) {
      if (i >= position) {
         if (i >= screensize+position) break;

         if (CurDisplay && CurDisplay->Line[0] != -1) {
            int x = THOFFSET;
            const char *cp;

            for (cp = CurDisplay->Line ; *cp ; cp++) {
               if (*cp == '\013') {
                  int c1 = *++cp;
                  int c2 = *++cp;
                  char cc[3];
                  cc[0] = ' ';

                  TextOut(PaintDC, x, Y, cc, 1);
                  x += TextWidth;

                  if (!no_color)
                     (void) SetTextColor(PaintDC, colorlist[c1 & 7]);


                  cc[0] = pn1[c1 & 7];
                  cc[1] = pn2[c1 & 7];
                  cc[2] = direc[c2 & 017];

                  TextOut(PaintDC, x, Y, cc, 3);
                  x += TextWidth*3;

                  /* Set back to plain "white". */

                  if (!no_color) {
                     if (!no_intensify)
                        (void) SetTextColor(PaintDC, RGB(255, 255, 255));
                     else
                        (void) SetTextColor(PaintDC, RGB(192, 192, 192));
                  }
               }
               else {
                  TextOut(PaintDC, x, Y, cp, 1);
                  x += TextWidth;
               }
            }
         }
         Y += TextHeight;
      }

      if (CurDisplay) CurDisplay = CurDisplay->Next;
   }

   EndPaint(hwnd, &PaintStruct);
}


static void Window1_OnScroll(HWND hwnd, HWND hwndCtl, UINT code, int pos)
{
   int oldposition = position;
   int delta;
   SCROLLINFO Scroll;
   RECT ClientRect;

   switch (code) {
   case SB_TOP:
      position = 0;
      break;
   case SB_LINEUP:
      position--;
      break;
   case SB_LINEDOWN:
      position++;
      break;
   case SB_PAGEUP:
      position -= pagesize;
      break;
   case SB_PAGEDOWN:
      position += pagesize;
      break;
   case SB_THUMBPOSITION:
   case SB_THUMBTRACK:
      position = pos;
      break;
   default:
      return;
   }

   if (position > nLineCnt-screensize) position = nLineCnt-screensize;
   if (position < 0) position = 0;

   delta = position - oldposition;

   if (delta == 0) return;

   Scroll.cbSize = sizeof(SCROLLINFO);
   Scroll.fMask = SIF_POS;
   Scroll.nPos = position;

   SetScrollInfo(hwnd, SB_VERT, &Scroll, TRUE);

   GetClientRect(hwnd, &ClientRect);
   ClientRect.left += THOFFSET;
   ClientRect.top += TVOFFSET;
   ClientRect.bottom -= BottomFudge;

   (void) ScrollWindowEx(hwnd, 0, -TextHeight*delta,
                         &ClientRect, &ClientRect, NULL, NULL, SW_ERASE | SW_INVALIDATE);

   if (delta > 0) {
      // Invalidate bottom part only.
      ClientRect.top += (screensize-delta)*TextHeight;
   }
   else {
      // Invalidate top part only.
      ClientRect.bottom -= (screensize+delta)*TextHeight;
   }

   InvalidateRect(hwnd, &ClientRect, TRUE);  // Be sure we erase the background.
}


static void Window1_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
   SCROLLINFO Scroll;
   RECT ClientRect;

   // Allow TVOFFSET amount of margin at both top and bottom.
   screensize = (cy-TVOFFSET-TVOFFSET)/TextHeight;
   BottomFudge = cy-TVOFFSET - screensize*TextHeight;

   pagesize = screensize-2;

   if (pagesize < 2)
      pagesize = 2;

   if (position+screensize > nLineCnt)
      position = nLineCnt-screensize;

   if (position < 0) position = 0;

   Scroll.cbSize = sizeof(SCROLLINFO);
   Scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
   Scroll.nMin = 0;
   Scroll.nMax = nLineCnt-1;
   Scroll.nPage = screensize;
   Scroll.nPos = position;

   SetScrollInfo(hwnd, SB_VERT, &Scroll, TRUE);

   GetClientRect(hwnd, &ClientRect);
   InvalidateRect(hwnd, &ClientRect, TRUE);  // Be sure we erase the background.
}




/* This is the WndProc for the main screen. */

LRESULT WINAPI MainWndProc(
   HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
      HANDLE_MSG(hwnd, WM_CREATE, Window1_OnCreate);
      HANDLE_MSG(hwnd, WM_DESTROY, Window1_OnDestroy);
      HANDLE_MSG(hwnd, WM_COMMAND, Window1_OnCommand);
      HANDLE_MSG(hwnd, WM_PAINT, Window1_OnPaint);
      HANDLE_MSG(hwnd, WM_VSCROLL, Window1_OnScroll);
      HANDLE_MSG(hwnd, WM_SIZE, Window1_OnSize);
   default:
      return DefWindowProc(hwnd, message, wParam, lParam);
   }
}




/* Process get-text dialog box messages */

static int PopupStatus;
static char szPrompt[MAX_TEXT_LINE_LENGTH];
static char szTextEntryResult[MAX_TEXT_LINE_LENGTH];
LRESULT WINAPI TEXT_ENTRY_DIALOG_WndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
   int Len;

   switch (Message) {
   case WM_INITDIALOG:
      /* If we did this, it would set the actual window title
         SetWindowText(hDlg, "FOOBAR!"); */
      SetDlgItemText(hDlg, IDC_FILE_TEXT, szPrompt);
      return TRUE;
   case WM_COMMAND:
      if (HIWORD (wParam) == EN_CHANGE && LOWORD (wParam) == IDC_FILE_EDIT)
         SendMessage (hDlg, DM_SETDEFID, (WPARAM) IDC_FILE_ACCEPT, (LPARAM) 0);

      switch (LOWORD (wParam)) {
      case IDC_FILE_ACCEPT:
         Len = SendDlgItemMessage (hDlg, IDC_FILE_EDIT, EM_LINELENGTH, 0, 0L);
         if (Len > MAX_TEXT_LINE_LENGTH - 1)
            Len = MAX_TEXT_LINE_LENGTH - 1;

         if (Len > 0) {
            GetWindowText(GetDlgItem (hDlg, IDC_FILE_EDIT),
                          szTextEntryResult, MAX_TEXT_LINE_LENGTH);
            PopupStatus = POPUP_ACCEPT_WITH_STRING;
            EndDialog(hDlg, TRUE);
            /*
            WaitingForCommand = FALSE;
            */
            return TRUE;
         }
         return FALSE;

      case IDC_FILE_CANCEL:
      case IDCANCEL:
         PopupStatus = POPUP_DECLINE;
         EndDialog(hDlg, TRUE);
         /*
         WaitingForCommand = FALSE;
         */
         return TRUE;
      }

      return FALSE;
   }

   return FALSE;
}

/* This leaves the resultant string in "szTextEntryResult" and the popup status in "PopupStatus". */
static void do_general_text_popup(Cstring prompt)
{
   sprintf (szPrompt, prompt);
   DialogBox(GLOBhInstance, MAKEINTRESOURCE(IDD_TEXT_ENTRY_DIALOG),
             GLOBhMainWindow, (DLGPROC) TEXT_ENTRY_DIALOG_WndProc);
}


/* Register the windows */

static BOOL Register(HINSTANCE hInst)
{
   WNDCLASS WndClass;

   /* Register the main window */

   memset(&WndClass, 0, sizeof(WNDCLASS));
   WndClass.style          = CS_HREDRAW | CS_VREDRAW/* | CS_NOCLOSE*/;
   WndClass.lpfnWndProc    = MainWndProc;
   WndClass.hInstance      = hInst;
   WndClass.hIcon          = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
   WndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
   WndClass.hbrBackground  = GetStockBrush(BLACK_BRUSH);
   WndClass.lpszMenuName   = MAKEINTRESOURCE(IDR_MENU1);
   WndClass.lpszClassName  = szAppName;

   if (!RegisterClass (&WndClass))
      return FALSE;

   return TRUE;
}


extern int PASCAL WinMain(HINSTANCE hCurInstance, HINSTANCE hPrevInstance,
                          LPSTR lpCmdLine, int nCmdShow)
{
   GLOBnCmdShow = nCmdShow;
   GLOBhInstance = hCurInstance;

   if (!hPrevInstance)
      if (!Register(hCurInstance))
         return FALSE;

   /* Now call up the SD program. */

   return main(__argc, __argv);
}


FILE *call_list_file;   /* So is this. */




static void setup_level_menu(HWND hDlg)
{
   dance_level lev;

   SetDlgItemText(hDlg, IDC_START_CAPTION, "Choose a level");

   SendDlgItemMessage(hDlg, IDC_START_LIST, LB_RESETCONTENT, 0, 0L);

   for (lev=l_mainstream ; ; lev++) {
      Cstring this_string = getout_strings[lev];
      if (!this_string[0]) break;
      SendDlgItemMessage(hDlg, IDC_START_LIST, LB_ADDSTRING, 0, (LPARAM) this_string);
   }

   SendDlgItemMessage (hDlg, IDC_START_LIST, LB_SETCURSEL, 0, 0L);
}


/* This is the last title sent by the main program.  We add stuff to it. */
static char szMainTitle[MAX_TEXT_LINE_LENGTH];


void SetTitle(long_boolean show_resolve_status)
{
   if (show_resolve_status) {
      SetWindowText(GLOBhMainWindow, (LPSTR) szResolveWndTitle);
   }
   else {
      char szTitle[MAX_TEXT_LINE_LENGTH];

      lstrcpy(szTitle, szMainTitle);

      if (using_active_phantoms)
         lstrcat(szTitle, "    [Active Phantoms Enabled, ");
      else
         lstrcat(szTitle, "    [Active Phantoms Disabled, ");

      if (allowing_all_concepts)
         lstrcat(szTitle, "All Concepts Enabled]");
      else
         lstrcat(szTitle, "Level Concepts Only]");

      SetWindowText(GLOBhMainWindow, (LPSTR) szTitle);
   }
}


extern void uims_set_window_title(char s[])
{
   lstrcpy(szMainTitle, s);
   SetTitle(FALSE);
}



extern char *new_outfile_string;
extern char *call_list_string;
extern long_boolean get_first_session_line(void);
extern long_boolean get_next_session_line(char *dest);
extern int process_session_info(Cstring *error_msg);
static long_boolean doing_level_dialog;



/* 0 for normal, 1 to delete session line, 2 to put up fatal error and exit. */
static int startup_retval;
/* These have the fatal error. */
static char session_error_msg1[200], session_error_msg2[200];

static HWND hDialogWindow;

/* Process Startup dialog box messages. */

static void Startup_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
   int i;
   int session_outcome = 0;
   Cstring session_error_msg;

   switch (id) {
   case IDC_WRITE_LIST:
   case IDC_WRITE_FULL_LIST:
   case IDC_ABRIDGED:
      /* User clicked on some call list option.  Seed the file name. */
      GetWindowText(GetDlgItem(hwnd, IDC_CALL_LIST_NAME),
                    szDatabaseFilename, MAX_TEXT_LINE_LENGTH);
      if (!szDatabaseFilename[0])
         SetDlgItemText(hwnd, IDC_CALL_LIST_NAME, "call_list");
      /* FALL THROUGH!!! */
   case IDC_NORMAL:
      /* FELL THROUGH!!! */
      /* User clicked a radio button.  We have to update the buttons. */
      CheckRadioButton (hwnd, IDC_NORMAL, IDC_ABRIDGED, id);
      return;
   case IDC_USERDEFINED:
      /* User clicked on a special database file.  Seed the file name. */
      GetWindowText(GetDlgItem(hwnd, IDC_DATABASE_NAME),
                    szDatabaseFilename, MAX_TEXT_LINE_LENGTH);
      if (!szDatabaseFilename[0])
         SetDlgItemText(hwnd, IDC_DATABASE_NAME, "database");
      /* FALL THROUGH!!! */
   case IDC_DEFAULT:
      /* FELL THROUGH!!! */
      /* User clicked a radio button.  We have to update the buttons. */
      CheckRadioButton (hwnd, IDC_DEFAULT, IDC_USERDEFINED, id);
      return;
   case IDCANCEL:         /* User hit the "close window" thing in the upper right corner. */
   case IDC_START_CANCEL: /* User hit the "cancel" button. */
      EndDialog(hwnd, TRUE);
      uims_final_exit(0);
      return;
   case IDC_START_ACCEPT:

      /* User hit the "accept" button.  Read out all the information.
         But it's more complicated than that.  We sometimes do a two-stage
         presentation of this dialog, getting the session number and then
         the level.  So we may have to go back to the second stage.
         The variable "doing_level_dialog" tells what we were getting. */

      startup_retval = 0;
      i = SendDlgItemMessage(hwnd, IDC_START_LIST, LB_GETCURSEL, 0, 0L);

      if (!doing_level_dialog) {
         /* The user has just responded to the session selection.
            Figure out what to do.  We may need to go back and get the
            level. */

         session_index = i;

         /* If the user wants that session deleted, do that, and get out
            immediately.  Setting the number to zero will cause it to
            be deleted when the session file is updated during program
            termination. */

         if (IsDlgButtonChecked(hwnd, IDC_START_DELETE_SESSION_CHECKED)) {
            session_index = -session_index;
            startup_retval = 1;
            goto getoutahere;
         }

         /* Analyze the indicated session number. */

         session_outcome = process_session_info(&session_error_msg);

         if (session_outcome & 1) {
            /* We are not using a session, either because the user selected
               "no session", or because of some error in processing the
               selected session. */
            session_index = 0;
            sequence_number = -1;
         }

         /* If the level never got specified, either from a command line
            argument or from the session file, put up the level selection
            screen and go back for another round. */

         if (calling_level == l_nonexistent_concept) {
            setup_level_menu(hwnd);
            doing_level_dialog = TRUE;
            return;
         }
      }
      else {
         /* Either there was no session file, or there was a session
            file but the user selected no session or a new session.
            In the latter case, we went back and asked for the level.
            So now we have both, and we can proceed. */
         calling_level = (dance_level) i;
      }

      if (IsDlgButtonChecked(hwnd, IDC_ABRIDGED))
         glob_call_list_mode = call_list_mode_abridging;
      else if (IsDlgButtonChecked(hwnd, IDC_WRITE_LIST))
         glob_call_list_mode = call_list_mode_writing;
      else if (IsDlgButtonChecked(hwnd, IDC_WRITE_FULL_LIST))
         glob_call_list_mode = call_list_mode_writing_full;

      /* ****** Need to do this later! */

      if (session_outcome & 2)
         MessageBox(hwnd, session_error_msg, "Error", MB_OK);


      /* If the user specified a call list file, get the name. */

      if (glob_call_list_mode != call_list_mode_none) {

         /* This may have come from the command-line switches,
            in which case we already have the file name. */

         if (!call_list_string) {
            GetWindowText(GetDlgItem(hwnd, IDC_CALL_LIST_NAME),
                          szCallListFilename, MAX_TEXT_LINE_LENGTH);

            call_list_string = szCallListFilename;
         }

         if (open_call_list_file(call_list_string)) exit_program(1);
      }

      /* If user specified the output file during startup dialog, install that.
         It overrides anything from the command line. */

      GetWindowText(GetDlgItem(hwnd, IDC_OUTPUT_NAME),
                    szOutFilename, MAX_TEXT_LINE_LENGTH);

      if (szOutFilename[0])
         new_outfile_string = szOutFilename;
      else
         (void) strncat(outfile_string, filename_strings[calling_level], MAX_FILENAME_LENGTH);

      /* Handle user-specified database file. */

      if (IsDlgButtonChecked(hwnd, IDC_USERDEFINED)) {
         GetWindowText(GetDlgItem(hwnd, IDC_DATABASE_NAME),
                       szDatabaseFilename, MAX_TEXT_LINE_LENGTH);
         database_filename = szDatabaseFilename;
      }

      if (calling_level == l_nonexistent_concept)
         calling_level = l_mainstream;   /* User really doesn't want to tell us the level. */

      if (new_outfile_string)
         (void) install_outfile_string(new_outfile_string);

      /* We now have the following things filled in:
         session_index
         glob_call_list_mode
      */

      starting_sequence_number = sequence_number;

      SetDlgItemText(hwnd, IDC_START_COMMENTARY, "Reading database");

      prepare_to_read_menus();

      /* Opening the database sets up the values of
         abs_max_calls and max_base_calls.
         Must do before telling the uims so any open failure messages
         come out first. */

      if (open_database(session_error_msg1, session_error_msg2)) {
         startup_retval = 2;
         goto getoutahere;
      }

      build_database(glob_call_list_mode);   /* This calls uims_database_tick_max,
                                                which calibrates the progress bar. */

      SetDlgItemText(hwnd, IDC_START_COMMENTARY, "Creating menus");

      /* This is the thing that takes all the time! */
      initialize_menus(glob_call_list_mode);

      /* If we wrote a call list file, that's all we do. */
      if (glob_call_list_mode == call_list_mode_writing || glob_call_list_mode == call_list_mode_writing_full) {
         startup_retval = 1;
      }

   getoutahere:

      EndDialog(hwnd, TRUE);
      return;
   }
}

static BOOL Startup_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
   hDialogWindow = hwnd;

   /* This was an attempt to disable the file name input controls.
   the_call_list = GetDlgItem(hwnd, IDC_CALL_LIST_NAME);
   SendMessage(the_call_list, WS_DISABLED, 0, 0);
   the_call_list = GetDlgItem(hwnd, IDC_OUTPUT_NAME);
   SendMessage(the_call_list, WS_DISABLED, -1, -1);
   the_call_list = GetDlgItem(hwnd, IDC_DATABASE_NAME);
   SendMessage(the_call_list, WS_DISABLED, 0, 1);
   */

   /* Select the default radio buttons. */

   CheckRadioButton (hwnd, IDC_NORMAL, IDC_ABRIDGED, IDC_NORMAL);
   CheckRadioButton (hwnd, IDC_DEFAULT, IDC_USERDEFINED, IDC_DEFAULT);

   /* Seed the various file names with the null string. */

   SetDlgItemText(hwnd, IDC_OUTPUT_NAME, "");
   SetDlgItemText(hwnd, IDC_CALL_LIST_NAME, "");
   SetDlgItemText(hwnd, IDC_DATABASE_NAME, "");

   /* Put up the session list or the level list,
      depending on whether a session file is in use. */

   SendDlgItemMessage(hwnd, IDC_START_LIST, LB_RESETCONTENT, 0, 0L);
#ifdef DAMMIT

   SendDlgItemMessage(hwnd, IDC_START_PROGRESS, PBM_SETRANGE, 0, MAKELONG(0, 20));
   SendDlgItemMessage(hwnd, IDC_START_PROGRESS, PBM_SETSTEP, 1, 0);
#endif

   if (!get_first_session_line()) {
      char line[MAX_FILENAME_LENGTH];

      SetDlgItemText(hwnd, IDC_START_CAPTION, "Choose a session");

      while (get_next_session_line(line))
         SendDlgItemMessage(hwnd, IDC_START_LIST, LB_ADDSTRING, 0, (LPARAM) line);
      doing_level_dialog = FALSE;
      SendDlgItemMessage(hwnd, IDC_START_LIST, LB_SETCURSEL, 0, 0L);
   }
   else {
      setup_level_menu(hwnd);
      doing_level_dialog = TRUE;
   }

   return TRUE;
}


LRESULT WINAPI Startup_Dialog_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message) {
      HANDLE_MSG(hwnd, WM_INITDIALOG, Startup_OnInitDialog);
      HANDLE_MSG(hwnd, WM_COMMAND, Startup_OnCommand);
   default:
      return FALSE;      /* We do *NOT* call the system default handler. */
   }
}





extern long_boolean uims_open_session(int argc, char **argv)
{
   /* At this point the following may have been filled in from the
      command-line switches or the "[Options]" stuff in the initialization file:

         glob_call_list_mode [default = call_list_mode_none]
         calling_level       [default = l_nonexistent_concept]
         new_outfile_string  [default = (char *) 0, this is just a pointer]
         call_list_string    [default = (char *) 0], this is just a pointer]
         database_filename   [default = "sd_calls.dat", this is just a pointer]

         The startup screen may get more info.
   */

   GLOBhMainWindow = CreateWindow(szAppName, szAppName, WS_OVERLAPPEDWINDOW | WS_VSCROLL,
                                  10, 20, 780, 560,
                                  NULL, NULL, GLOBhInstance, NULL);

   if (!GLOBhMainWindow)
      return TRUE;

   memset(&call_menu_lists, 0, sizeof(call_menu_lists));

   /* Put up the dialog box for the startup screen. */

   DialogBox(GLOBhInstance, MAKEINTRESOURCE(IDD_START_DIALOG),
             GLOBhMainWindow, (DLGPROC) Startup_Dialog_WndProc);

   if (startup_retval == 1) {
      close_init_file();
      return TRUE;
   }
   else if (startup_retval == 2) {
      uims_fatal_error(session_error_msg1, session_error_msg2);
      exit_program(1);
   }

   ShowWindow(GLOBhMainWindow, GLOBnCmdShow);
   UpdateWindow(GLOBhMainWindow);

   initialize_concept_sublists();
   initialize_misc_lists();
   matcher_initialize();

   {
      long_boolean save_allow = allowing_all_concepts;
      allowing_all_concepts = TRUE;

      /* Process the keybindings for user-definable calls, concepts, and commands. */

      if (open_accelerator_region()) {
         char q[INPUT_TEXTLINE_SIZE];
         while (get_accelerator_line(q))
            do_accelerator_spec(q);
      }
      else {
         Cstring *q;
         for (q = concept_key_table ; *q ; q++)
            do_accelerator_spec(*q);
      }

      allowing_all_concepts = save_allow;
   }

   close_init_file();

   use_escapes_for_drawing_people = TRUE;

   /* Install the pointy triangles. */

   if (!no_graphics)
      direc = "?\020?\021????\036?\037?????";

   if (color_by_couple)
      colorlist = couplecolors;
   else
      colorlist = peoplecolors;

   GLOBhMenuWindow =
      CreateDialog(GLOBhInstance,
                   MAKEINTRESOURCE(IDD_CALLS_DIALOG),
                   GLOBhMainWindow,
                   (DLGPROC) MenuWndProc);

   if (!GLOBhMenuWindow) {
      uims_fatal_error("Can't create calls menu", 0);
      uims_final_exit (0);
   }

   SetTitle(FALSE);

   /* Initialize the display window linked list */

   DisplayRoot = get_mem (sizeof (DisplayType));
   DisplayRoot->Line[0] = -1;
   DisplayRoot->Next = NULL;
   CurDisplay = DisplayRoot;
   nLineCnt = 0;
   return FALSE;
}




static int do_menu(Cstring the_prompt, Cstring *the_menu, int count)
{
   MessageBox(GLOBhMainWindow, "Not yet available", "Under Construction", MB_OK);
   return dialog_menu_index;
}


/* Process Comment dialog box messages */

LRESULT WINAPI CommentWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
#ifdef NONONO
   int Len;

   switch (Message) {
      case WM_INITDIALOG:
         return TRUE;

      case WM_COMMAND:
         if (HIWORD (wParam) == EN_CHANGE && LOWORD (wParam) == IDC_COM_EDIT)
            SendMessage (hDlg, DM_SETDEFID, (WPARAM) IDC_COM_ACCEPT, (LPARAM) 0);

         switch (LOWORD (wParam))
         {
            case IDC_COM_ACCEPT:
               Len = SendDlgItemMessage (hDlg, IDC_COM_EDIT, EM_LINELENGTH, 0, 0L);
               if (Len > MAX_TEXT_LINE_LENGTH - 1)
                  Len = MAX_TEXT_LINE_LENGTH - 1;

               if (Len > 0)
               {
                  GetWindowText (GetDlgItem (hDlg, IDC_COM_EDIT), szComment, MAX_TEXT_LINE_LENGTH);
                  PopupStatus = POPUP_ACCEPT_WITH_STRING;
                  EndDialog (hDlg, TRUE);
                  WaitingForCommand = FALSE;
                  return TRUE;
               }

               PopupStatus = POPUP_DECLINE;
               return FALSE;

            case IDCANCEL:
            case IDC_COM_CANCEL:
               PopupStatus = POPUP_DECLINE;
               EndDialog (hDlg, TRUE);
               WaitingForCommand = FALSE;
               return TRUE;
         }
         return FALSE;
   }
#endif
   return FALSE;
}


/* Process Getout dialog box messages */

LRESULT WINAPI GetoutWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
#ifdef NONONO
   int Len;

   switch (Message)
   {
      case WM_INITDIALOG:
         return TRUE;

      case WM_COMMAND:
         if (HIWORD (wParam) == EN_CHANGE && LOWORD (wParam) == IDC_END_EDIT)
            SendMessage (hDlg, DM_SETDEFID, (WPARAM) IDC_END_ACCEPT, (LPARAM) 0);

         switch (LOWORD (wParam))
         {
            case IDC_END_ACCEPT:
               Len = SendDlgItemMessage (hDlg, IDC_END_EDIT, EM_LINELENGTH, 0, 0L);
               if (Len > MAX_TEXT_LINE_LENGTH - 1)
                  Len = MAX_TEXT_LINE_LENGTH - 1;

               if (Len > 0)
               {
                  GetWindowText (GetDlgItem (hDlg, IDC_END_EDIT), szHeader, MAX_TEXT_LINE_LENGTH);
                  PopupStatus = POPUP_ACCEPT_WITH_STRING;
                  EndDialog (hDlg, TRUE);
                  WaitingForCommand = FALSE;
                  return TRUE;
               }
               return FALSE;

            case IDC_END_NOHDR:
               PopupStatus = POPUP_ACCEPT;
               EndDialog (hDlg, TRUE);
               WaitingForCommand = FALSE;
               return TRUE;

            case IDC_END_CANCEL:
            case IDCANCEL:
               PopupStatus = POPUP_DECLINE;
               EndDialog (hDlg, TRUE);
               WaitingForCommand = FALSE;
               return TRUE;
         }
         return FALSE;
   }
#endif
   return FALSE;
}


/* Process about window messages */

LRESULT WINAPI AboutWndProc(HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
   switch (Message) {
   case WM_INITDIALOG:
      return TRUE;
   case WM_COMMAND:
      EndDialog (hDlg, TRUE);
      return TRUE;
   }
   return FALSE;
}




/* Handle Font Select dialog box messages */

LRESULT WINAPI FontSelWndProc (HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
#ifdef NONONO
   switch (Message)
   {
      case WM_INITDIALOG:
         CheckRadioButton (hDlg, IDC_SMALL, IDC_LARGE, IDC_MEDIUM);
         FontSize = -17;
         CheckRadioButton (hDlg, IDC_SELPRINTER, IDC_DEFPRINTER, IDC_DEFPRINTER);
         pdFlags = PD_RETURNDEFAULT | PD_RETURNDC;
         DontPrint = TRUE;
         return TRUE;
      case WM_COMMAND:
         switch (LOWORD (wParam))
         {
            case IDCANCEL:
               EndDialog (hDlg, TRUE);
               return TRUE;
            case IDC_SMALL:
               CheckRadioButton (hDlg, IDC_SMALL, IDC_LARGE, IDC_SMALL);
               FontSize = -11;
               return TRUE;
            case IDC_MEDIUM:
               CheckRadioButton (hDlg, IDC_SMALL, IDC_LARGE, IDC_MEDIUM);
               FontSize = -17;
               return TRUE;
            case IDC_LARGE:
               CheckRadioButton (hDlg, IDC_SMALL, IDC_LARGE, IDC_LARGE);
               FontSize = -24;
               return TRUE;
            case IDC_DEFPRINTER:
               CheckRadioButton (hDlg, IDC_SELPRINTER, IDC_DEFPRINTER, IDC_DEFPRINTER);
               pdFlags = PD_RETURNDEFAULT | PD_RETURNDC;
               return TRUE;
            case IDC_SELPRINTER:
               CheckRadioButton (hDlg, IDC_SELPRINTER, IDC_DEFPRINTER, IDC_SELPRINTER);
               pdFlags = PD_RETURNDC;
               return TRUE;
            case IDC_ACCEPT:
               EndDialog (hDlg, TRUE);
               DontPrint = FALSE;
               return TRUE;
         }
   }
#endif
   return FALSE;
}




/* Handle WM_DESTROY message */

void DoDestroy(HWND hWnd)
{
   PostQuitMessage(0);
}


/* Process Windows Messages */

void EnterMessageLoop(void)
{
   MSG Msg;

   user_match.valid = FALSE;
   user_input[0] = '\0';
   static_ss.full_input[0] = '\0';
   static_ss.full_input_size = 0;
   WaitingForCommand = TRUE;

   while (GetMessage(&Msg, NULL, 0, 0) && WaitingForCommand) {
      if (!IsDialogMessage(GLOBhMenuWindow, &Msg)) {
         TranslateMessage (&Msg);
         DispatchMessage (&Msg);
      }
   }

   if (WaitingForCommand)
      final_exit (0);   /* User closed the window. */
}


extern void uims_display_ui_intro_text(void)
{
}


extern void uims_display_help(void)
{
}



Private char version_mem[12];

extern char *
uims_version_string (void)
{
   sprintf (version_mem, "%sw32", sdui_version);
   return version_mem;
}


extern void uims_process_command_line(int *argcp, char ***argvp)
{
}


void ShowListBox(int nWhichOne)
{
   if (nWhichOne != nLastOne) {
      nLastOne = nWhichOne;
      SendMessage(GLOBhMenuWindow, WM_COMMAND, CM_REINIT, 0L);
   }

   SetFocus(hCallsEbox);
}




extern void uims_create_menu(call_list_kind cl)
{
   int i;

   call_menu_lists[cl].Root = NULL;
   for (i = 0; i < number_of_calls[cl]; i++) {
      if (call_menu_lists[cl].Root == NULL) {
         call_menu_lists[cl].Root = (CallType *) get_mem (sizeof (CallType));
         CurCall = call_menu_lists[cl].Root;
         nCallIndex = 0;
      }
      else {
         CurCall->Next = (CallType *) get_mem (sizeof (CallType));
         CurCall = CurCall->Next;
      }

      CurCall->Next = NULL;
      CurCall->Call = main_call_lists[cl][i]->menu_name;
      CurCall->nCallIndex = MAKELONG (nCallIndex++, 0);
   }
}




extern uims_reply uims_get_startup_command(void)
{
   nLastOne = ui_undefined;
   MenuKind = ui_start_select;
   ShowListBox(match_startup_commands);

   EnterMessageLoop();

   uims_menu_index = user_match.match.index;

   if (uims_menu_index < 0)
      /* Special encoding from a function key. */
      uims_menu_index = -1-uims_menu_index;
   else if (user_match.match.kind == ui_command_select) {
      /* Translate the command. */
      uims_menu_index = (int) command_command_values[uims_menu_index];
   }

   return user_match.match.kind;
}


extern long_boolean uims_get_call_command(uims_reply *reply_p)
{
   if (allowing_modifications)
      parse_state.call_list_to_use = call_list_any;

   SetTitle(FALSE);
   nLastOne = ui_undefined;    /* Make sure we get a new menu,
                                  in case concept levels were toggled. */
   MenuKind = ui_call_select;
   ShowListBox(parse_state.call_list_to_use);
   my_retval = FALSE;
   EnterMessageLoop();

   my_reply = user_match.match.kind;
   uims_menu_index = user_match.match.index;

   if (uims_menu_index < 0)
      /* Special encoding from a function key. */
      uims_menu_index = -1-uims_menu_index;
   else if (my_reply == ui_command_select) {
      /* Translate the command. */
      uims_menu_index = (int) command_command_values[uims_menu_index];
   }
   else if (my_reply == ui_special_concept) {
   }
   else {
      call_conc_option_state save_stuff = user_match.match.call_conc_options;
      there_is_a_call = FALSE;
      my_retval = deposit_call_tree(&user_match.match, (parse_block *) 0, 2);
      user_match.match.call_conc_options = save_stuff;
      if (there_is_a_call) {
         parse_state.topcallflags1 = the_topcallflags;
         my_reply = ui_call_select;
      }
   }

   *reply_p = my_reply;
   return my_retval;
}


extern uims_reply uims_get_resolve_command (void)
{
   SetTitle(TRUE);
   nLastOne = ui_undefined;
   MenuKind = ui_resolve_select;
   ShowListBox(match_resolve_commands);
   my_retval = FALSE;
   EnterMessageLoop();

   if (user_match.match.index < 0)
      uims_menu_index = -1-user_match.match.index;   /* Special encoding from a function key. */
   else
      uims_menu_index = (int) resolve_command_values[user_match.match.index];

   return user_match.match.kind;
}



extern int uims_do_comment_popup (char dest[])
{
   WaitingForCommand = TRUE;
   DialogBox (GLOBhInstance, "Com_dialog", GLOBhMainWindow, (DLGPROC) CommentWndProc);
   if (PopupStatus == POPUP_ACCEPT_WITH_STRING)
   {
      lstrcpy (dest, szComment);
      return POPUP_ACCEPT_WITH_STRING;
   }

   return POPUP_DECLINE;
}


extern int uims_do_outfile_popup(char dest[])
{
   char myPrompt[MAX_TEXT_LINE_LENGTH];

   sprintf(myPrompt, "Sequence output file is \"%s\". Enter new name:", outfile_string);
   do_general_text_popup(myPrompt);
   if (PopupStatus == POPUP_ACCEPT_WITH_STRING)
      lstrcpy (dest, szTextEntryResult);

   return PopupStatus;
}


extern int uims_do_header_popup(char dest[])
{
   char myPrompt[MAX_TEXT_LINE_LENGTH];

   if (header_comment[0])
      sprintf(myPrompt, "Current title is \"%s\". Enter new title:", header_comment);
   else
      sprintf(myPrompt, "Enter new title:");

   do_general_text_popup(myPrompt);
   if (PopupStatus == POPUP_ACCEPT_WITH_STRING)
      lstrcpy (dest, szTextEntryResult);

   return PopupStatus;
}


extern int uims_do_getout_popup (char dest[])
{
   DialogBox (GLOBhInstance, "End_dialog", GLOBhMainWindow, (DLGPROC) GetoutWndProc);
   if (PopupStatus == POPUP_ACCEPT_WITH_STRING)
      lstrcpy (dest, szHeader);

   return PopupStatus;
}


#ifdef NEGLECT
extern int
uims_do_neglect_popup (char dest[])
{
   MessageBox (GLOBhMainWindow, "uims_do_neglect_popup", "Message", MB_OK);
   return 0;
}
#endif


extern int uims_do_write_anyway_popup(void)
{
   if (MessageBox(GLOBhMainWindow, "This sequence is not resolved.\n"
                  "Do you want to write it anyway?",
                  "Confirmation", MB_YESNO) == IDYES)
      return 1;
   else
      return 0;
}

extern int uims_do_delete_clipboard_popup(void)
{
   if (MessageBox(GLOBhMainWindow, "There are calls in the clipboard.\n"
                  "Do you want to delete all of them?",
                  "Confirmation", MB_YESNO) == IDYES)
      return 1;
   else
      return 0;
}

extern int uims_do_session_init_popup(void)
{
   if (MessageBox(GLOBhMainWindow, "You already have a session file.\n"
                  "Do you really want to delete it and start over?",
                  "Confirmation", MB_YESNO) == IDYES)
      return 1;
   else
      return 0;
}


extern int uims_do_abort_popup(void)
{
   if (MessageBox(GLOBhMainWindow, "Do you really want to abort this sequence?",
                  "Confirmation", MB_YESNO) == IDYES)
      return 1;
   else
      return 0;
}


extern int uims_do_modifier_popup(Cstring callname, modify_popup_kind kind)
{
   char modifier_question[150];

   switch (kind) {
   case modify_popup_any:
      sprintf (modifier_question,
               "The \"%s\" can be replaced.", callname);
      break;
   case modify_popup_only_tag:
      sprintf (modifier_question,
               "The \"%s\" can be replaced with a tagging call.", callname);
      break;
   case modify_popup_only_circ:
      sprintf (modifier_question,
               "The \"%s\" can be replaced with a modified circulate-like call.", callname);
      break;
   default:
      lstrcpy (modifier_question, "Internal error: unknown modifier kind.");
   }

   if (MessageBox (GLOBhMainWindow, modifier_question, "Do you want to replace it?", MB_YESNO) == IDYES)
      return 1;
   else
      return 0;
}

extern int uims_do_selector_popup(void)
{
   int retval;

   if (!user_match.valid || (user_match.match.call_conc_options.who == selector_uninitialized)) {
      match_result saved_match = user_match;
      uims_reply SavedMenuKind = MenuKind;
      uims_reply SavedMy_reply = my_reply;

      nLastOne = ui_undefined;
      MenuKind = ui_call_select;
      ShowListBox((int) match_selectors);
      EnterMessageLoop();

      MenuKind = SavedMenuKind;
      my_reply = SavedMy_reply;

      /* We skip the zeroth selector, which is selector_uninitialized. */
      retval = user_match.match.index+1;
      user_match = saved_match;
   }
   else {
      retval = (int) user_match.match.call_conc_options.who;
      user_match.match.call_conc_options.who = selector_uninitialized;
   }
   return retval;
}


extern int uims_do_direction_popup(void)
{
   int retval;

   if (!user_match.valid ||
       (user_match.match.call_conc_options.where == direction_uninitialized)) {
      match_result saved_match = user_match;
      uims_reply SavedMenuKind = MenuKind;
      uims_reply SavedMy_reply = my_reply;

      nLastOne = ui_undefined;
      MenuKind = ui_call_select;
      ShowListBox((int) match_directions);
      EnterMessageLoop();

      MenuKind = SavedMenuKind;
      my_reply = SavedMy_reply;

      /* We skip the zeroth direction, which is direction_uninitialized. */
      retval = user_match.match.index+1;
      user_match = saved_match;
   }
   else {
      retval = (int) user_match.match.call_conc_options.where;
      user_match.match.call_conc_options.where = direction_uninitialized;
   }
   return retval;
}



extern int uims_do_circcer_popup(void)
{
   uint32 retval;

   if (interactivity == interactivity_verify) {
      retval = verify_options.circcer;
      if (retval == 0) retval = 1;
   }
   else if (!user_match.valid || (user_match.match.call_conc_options.circcer == 0)) {
      match_result saved_match = user_match;
      uims_reply SavedMenuKind = MenuKind;
      uims_reply SavedMy_reply = my_reply;

      nLastOne = ui_undefined;
      MenuKind = ui_call_select;
      ShowListBox((int) match_circcer);
      EnterMessageLoop();

      MenuKind = SavedMenuKind;
      my_reply = SavedMy_reply;
      retval = user_match.match.call_conc_options.circcer;
      user_match = saved_match;
   }
   else {
      retval = user_match.match.call_conc_options.circcer;
      user_match.match.call_conc_options.circcer = 0;
   }

   return retval;
}



extern int uims_do_tagger_popup(int tagger_class)
{
   int retval;

   if (!user_match.valid ||
       (user_match.match.call_conc_options.tagger == 0)) {
      match_result saved_match = user_match;
      uims_reply SavedMenuKind = MenuKind;
      uims_reply SavedMy_reply = my_reply;

      nLastOne = ui_undefined;
      MenuKind = ui_call_select;
      ShowListBox(((int) match_taggers) + tagger_class);
      EnterMessageLoop();

      MenuKind = SavedMenuKind;
      my_reply = SavedMy_reply;
      saved_match.match.call_conc_options.tagger = user_match.match.call_conc_options.tagger;
      user_match = saved_match;
   }

   retval = user_match.match.call_conc_options.tagger;
   user_match.match.call_conc_options.tagger = 0;
   return retval;
}


extern uint32 uims_get_number_fields(int nnumbers, long_boolean forbid_zero)
{
   int i;
   uint32 number_fields = user_match.match.call_conc_options.number_fields;
   int howmanynumbers = user_match.match.call_conc_options.howmanynumbers;
   uint32 number_list = 0;

   for (i=0 ; i<nnumbers ; i++) {
      uint32 this_num;

      if (!user_match.valid || (howmanynumbers <= 0)) {
         match_result saved_match = user_match;
         uims_reply SavedMenuKind = MenuKind;
         uims_reply SavedMy_reply = my_reply;
         nLastOne = ui_undefined;
         MenuKind = ui_call_select;
         ShowListBox((int) match_number);
         EnterMessageLoop();

         MenuKind = SavedMenuKind;
         my_reply = SavedMy_reply;
         this_num = user_match.match.index;
         user_match = saved_match;
      }
      else {
         this_num = number_fields & 0xF;
         number_fields >>= 4;
         howmanynumbers--;
      }

      if (forbid_zero && this_num == 0) return ~0;
      if (this_num > 15) return ~0;    /* User gave bad answer. */
      number_list |= (this_num << (i*4));
   }

   return number_list;
}


static void Update_text_display(void)
{
   SCROLLINFO Scroll;
   RECT ClientRect;

   position = nLineCnt-screensize;
   if (position < 0) position = 0;

   Scroll.cbSize = sizeof(SCROLLINFO);
   Scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
   Scroll.nMin = 0;
   Scroll.nMax = nLineCnt-1;
   Scroll.nPage = screensize;
   Scroll.nPos = position;

   SetScrollInfo(GLOBhMainWindow, SB_VERT, &Scroll, TRUE);

   GetClientRect(GLOBhMainWindow, &ClientRect);
   InvalidateRect(GLOBhMainWindow, &ClientRect, TRUE);  // Be sure we erase the background.
}


extern void uims_add_new_line (char the_line[])
{
   lstrcpy(CurDisplay->Line, the_line);

   if (!CurDisplay->Next) {
      CurDisplay->Next = get_mem(sizeof (DisplayType));
      CurDisplay = CurDisplay->Next;
      CurDisplay->Next = NULL;
   }
   else
      CurDisplay = CurDisplay->Next;

   CurDisplay->Line[0] = -1;
   nLineCnt++;

   Update_text_display();
}


extern void uims_reduce_line_count (int n)
{
   CurDisplay = DisplayRoot;
   nLineCnt = n;
   while (CurDisplay->Line[0] != -1 && n--) {
      CurDisplay = CurDisplay->Next;
   }

   CurDisplay->Line[0] = -1;

   Update_text_display();
}


extern void uims_terminate (void)
{
}


extern void uims_begin_search(command_kind goal)
{
}

extern int uims_begin_reconcile_history (int currentpoint, int maxpoint)
{
   return FALSE;
}


extern int uims_end_reconcile_history (void)
{
   return FALSE;
}



extern void uims_update_resolve_menu(command_kind goal, int cur, int max, resolver_display_state state)
{
   create_resolve_menu_title(goal, cur, max, state, szResolveWndTitle);
   SetTitle(TRUE);
}


extern void uims_database_tick_max(int n)
{
   /*
   SendDlgItemMessage(hDialogWindow, IDC_START_PROGRESS, PBM_SETRANGE, 0, MAKELONG(0, n));
   SendDlgItemMessage(hDialogWindow, IDC_START_PROGRESS, PBM_SETSTEP, 1, 0);
   */
}


extern void uims_database_tick(int n)
{
/*
   SendDlgItemMessage(hDialogWindow, IDC_START_PROGRESS, PBM_SETSTEP, n, 0);
   SendDlgItemMessage(hDialogWindow, IDC_START_PROGRESS, PBM_STEPIT, 0, 0);
*/
}


extern void uims_database_tick_end(void)
{
}


extern void uims_database_error(Cstring message, Cstring call_name)
{
   MessageBox(GLOBhMainWindow, call_name, message, MB_OK);
   uims_final_exit (0);
}


extern void uims_bad_argument(Cstring s1, Cstring s2, Cstring s3)
{
   /* We don't seem to look at the args.  What a shame. */
   uims_fatal_error("Bad Command Line Argument", 0);
   uims_final_exit (0);
}


extern void uims_fatal_error(Cstring pszLine1, Cstring pszLine2)
{
   session_index = 0;    /* We don't write back the session file in this case. */

   if (pszLine2 && pszLine2[0]) {
      char msg[200];
      sprintf(msg, "%s: %s", pszLine1, pszLine2);
      pszLine1 = msg;   /* Yeah, we can do that.  Yeah, it's sleazy. */
   }

   MessageBox(GLOBhMainWindow, pszLine1, "Internal Error", MB_OK);
}


extern void uims_final_exit(int code)
{
   if (GLOBhMenuWindow)
      SendMessage(GLOBhMenuWindow, WM_COMMAND, IDCANCEL, 0L);

   if (GLOBhMainWindow)
      SendMessage(GLOBhMainWindow, WM_CLOSE, 0, 0L);

   ExitProcess(code);
}
