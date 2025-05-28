/* SD -- square dance caller's helper.

    Copyright (C) 1990-2000  William B. Ackerman.

    This file is unpublished and contains trade secrets.  It is
    to be used by permission only and not to be disclosed to third
    parties without the express permission of the copyright holders.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    This is for version 34. */

/* This defines the following functions:
   uims_choose_font
*/

#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>

#include "resource.h"
void windows_init_printer_font(HWND hwnd, HDC hdc);
extern void windows_print_this(HWND hwnd, char *szMainTitle, HINSTANCE hInstance,
                               const char *filename);
extern void windows_print_any(HWND hwnd, char *szMainTitle, HINSTANCE hInstance);
void PrintFile(const char *szFileName, HWND hwnd, char *szMainTitle, HINSTANCE hInstance);


#pragma comment(lib, "comctl32")


static LOGFONT lf;
static CHOOSEFONT cf;
static DOCINFO di;
static BOOL bUserAbort;
static HWND hDlgPrint;
static PRINTDLG pd;
static char szPrintDir[_MAX_PATH];


void windows_init_printer_font(HWND hwnd, HDC hdc)
{
   szPrintDir[0] = '\0';     // Initialize the default directory for "print any file".

   // We need to figure out the "logical size" that will give a 14 point
   // font.  And we haven't opened the printer, so we have to do it in terms
   // of the display.  The LOGPIXELSY number for the display will give "logical
   // units" (whatever they are -- approximately pixels, but it doesn't matter)
   // per inch.  A point is 1/72nd of an inch, so we do a little math.
   // Later, when the user chooses a font, the system will do its calculations
   // based on the display (we still won't have opened the printer, so we
   // just let the system think we're choosing a display font instead of a
   // printer font.)

   cf.lStructSize = sizeof(CHOOSEFONT);
   cf.hwndOwner = hwnd;
   cf.lpLogFont = &lf;
   cf.Flags = CF_NOVECTORFONTS | CF_BOTH | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;

   // Here is where we set the initial default font -- 14 point bold Courier New.
   lstrcpy(lf.lfFaceName, "Courier New");
   cf.iPointSize = 14 * 10;
   lf.lfWeight = FW_BOLD;

   lf.lfHeight = -((cf.iPointSize*GetDeviceCaps(hdc, LOGPIXELSY)+360)/720);

   // At all times, "lf" has the data for the current font (unfortunately,
   // calibrated for the display) and "cf.iPointSize" has the actual point
   // size times 10.  All manipulations during the "choose font" operations
   // will be in terms of this.  When it comes time to print, we will create
   // a new font for the display.  All the "lf" stuff will be incorrect for
   // it, and we will have to recompute the parameters of the logical font
   // based on the printer calibration.  We will use the field "cf.iPointSize"
   // (the only thing that is invariant) to recompute it.
}



BOOL CALLBACK PrintDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg) {
   case WM_INITDIALOG:
      EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
      return TRUE;
   case WM_COMMAND:
      bUserAbort = TRUE;
      EnableWindow (GetParent(hDlg), TRUE);
      DestroyWindow(hDlg);
      hDlgPrint = NULL;
      return TRUE;
   }
   return FALSE;
}          

BOOL CALLBACK PrintAbortProc(HDC hPrinterDC, int iCode)
{
   MSG msg;
     
   while (!bUserAbort && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
      if (!hDlgPrint || !IsDialogMessage (hDlgPrint, &msg)) {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }
   return !bUserAbort;
}


// We make this really large.  Memory is cheap.  Bugs aren't.
#define PRINT_LINE_LENGTH 500

void PrintFile(const char *szFileName, HWND hwnd, char *szMainTitle, HINSTANCE hInstance)
{
   BOOL            bSuccess;
   TEXTMETRIC      tm;

   // Invoke Print common dialog box
     
   ZeroMemory(&pd, sizeof(PRINTDLG));   // Don't keep selections around.
   pd.lStructSize = sizeof(PRINTDLG);
   pd.hwndOwner = hwnd;
   pd.Flags = PD_ALLPAGES | PD_NOPAGENUMS | PD_RETURNDC |
      PD_NOSELECTION | PD_DISABLEPRINTTOFILE | PD_HIDEPRINTTOFILE;
   pd.nCopies = 1;
     
   if (!PrintDlg(&pd)) return;

   HDC hdcPrn = pd.hDC;

   // Now the "logical font" structure "lf" has the selected font, except for
   // one problem -- its "lfHeight" field is calibrated for the display.
   // We need to recalibrate it for the printer.  So we go through the same
   // procedure as at initialization, this time using the printer device context.
   // We get the font size, in invariant form (10*point size) from "cf.iPointSize".

   LOGFONT printerlf = lf;   // All other fields are good.
   printerlf.lfHeight = -((cf.iPointSize*GetDeviceCaps(hdcPrn, LOGPIXELSY)+360)/720);
   SelectObject(hdcPrn, CreateFontIndirect(&printerlf));

   // Font is now ready.  Find out how big it is.
     
   GetTextMetrics(hdcPrn, &tm);
   int iPixelLineHeight = tm.tmHeight + tm.tmExternalLeading;

   // This is where we choose to start printing.  If it were zero,
   // we would start right at the top (less whatever margin the system
   // provides.)  We set it to iPixelLineHeight to give us an effective
   // blank line at the top of the page, in addition to the margin the
   // system provides.  That seems to look right.
   int iPixelTopOfPage = iPixelLineHeight;

   // This is where we choose to stop printing.  It must not be greater than
   // "GetDeviceCaps(hdcPrn, VERTRES)-iPixelLineHeight", that is, we must subtract
   // iPixelLineHeight.  If we set it to exactly that value, we print to the bottom
   // of the page.  So we subtract twice that value to get a reasonable
   // bottom margin.
   int iPixelBottomOfPage = GetDeviceCaps(hdcPrn, VERTRES)-iPixelLineHeight*2;

   // Display the printing dialog box
     
   EnableWindow(hwnd, FALSE);
     
   bSuccess = TRUE;
   bUserAbort = FALSE;
     
   hDlgPrint = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_PRINTING_DIALOG), 
                            hwnd, (DLGPROC) PrintDlgProc);

   SetDlgItemText(hDlgPrint, IDC_FILENAME, szFileName);
   SetAbortProc(hdcPrn, PrintAbortProc);


   ZeroMemory(&di, sizeof(DOCINFO));
   di.cbSize = sizeof(DOCINFO);
   di.lpszDocName = szMainTitle;
     
   FILE *fildes = fopen(szFileName, "r");
   if (!fildes) {
      bSuccess = FALSE;
      goto print_failed;
   }

   fpos_t fposStart;

   if (fgetpos(fildes, &fposStart)) {
      bSuccess = FALSE;
      goto print_failed;
   }

   // Start the printer

   if (StartDoc (hdcPrn, &di) > 0) {

      int iNumCopiesOfFile = (pd.Flags & PD_COLLATE) ? pd.nCopies : 1;
      int iNumCopiesOfEachPage = (pd.Flags & PD_COLLATE) ? 1 : pd.nCopies;

      // Scan across the required number of complete copies of the file.
      // If not collating, this loop cycles only once.

      for (int iFileCopies = 0 ; iFileCopies < iNumCopiesOfFile ; iFileCopies++) {

         if (fsetpos(fildes, &fposStart)) {   // Seek to beginning.
            bSuccess = FALSE;
            break;
         }

         // Scan the file.

         for ( BOOL bEOF = false ; !bEOF ; ) {
            fpos_t fposPageStart;

            // Save the current place so that we can do manual
            // uncollated copies of the page if necessary.

            if (fgetpos(fildes, &fposPageStart)) {
               bSuccess = FALSE;
               goto print_failed;
            }

            // Scan across the number of times we need to print this page.
            // If printing multiple copies and not collating, this loop
            // will cycle multiple times, printing the same page repeatedly.
            for (int iPageCopies = 0 ; iPageCopies < iNumCopiesOfEachPage; iPageCopies++) {

               // Go back to our saved position.

               if (fsetpos(fildes, &fposPageStart)) {
                  bSuccess = FALSE;
                  goto print_failed;
               }

               // Don't open the page unless there is actually a line of text to print.
               // That way, we don't embarrass ourselves by printing a blank page if
               // we see a formfeed after an exact integral number of sheets of paper.

               bool bPageIsOpen = false;

               for (int iRasterPos = iPixelTopOfPage;
                    iRasterPos <= iPixelBottomOfPage;
                    iRasterPos += iPixelLineHeight) {

                  fpos_t fposLineStart;
                  if (fgetpos(fildes, &fposLineStart))
                     break;

                  char pstrBuffer[PRINT_LINE_LENGTH+1];
                  if (!fgets(pstrBuffer, PRINT_LINE_LENGTH, fildes)) {
                     bEOF = true;
                     break;
                  }
                  // Strip off any <NEWLINE> -- we don't want it.  Note that, since
                  // we are using a POSIX call to read the file, we get POSIX-style
                  // line breaks -- just '\n'.
                  int j = strlen(pstrBuffer);
                  if (j>0 && pstrBuffer[j-1] == '\n')
                     pstrBuffer[j-1] = '\0';

                  // If we get a form-feed, break out of the loop, but reset
                  // the file location to just after the form-feed -- leave the
                  // rest of the line.

                  if (pstrBuffer[0] == '\f') {
                     (void) fsetpos(fildes, &fposLineStart);
                     (void) fgetc(fildes);   // pass over the form-feed.
                     break;
                  }

                  if (!bPageIsOpen) {
                     if (StartPage(hdcPrn) < 0) {
                        bSuccess = FALSE;
                        goto print_failed;
                     }
                     bPageIsOpen = true;
                  }

                  TextOut (hdcPrn, 0, iRasterPos, pstrBuffer, strlen(pstrBuffer));
               }
                         
               if (bPageIsOpen) {
                  if (EndPage (hdcPrn) < 0) {
                     bSuccess = FALSE;
                     goto print_failed;
                  }
               }
                         
               if (bUserAbort)
                  break;
            }
                    
            if (!bSuccess || bUserAbort)
               break;
         }
               
         if (!bSuccess || bUserAbort)
            break;
      }
   }
   else
      bSuccess = FALSE;
     
   if (bSuccess)
      EndDoc(hdcPrn);
     
 print_failed:

   if (!bUserAbort) {
      EnableWindow(hwnd, TRUE);
      DestroyWindow(hDlgPrint);
   }
     
   if (fildes) (void) fclose(fildes);
   DeleteDC(hdcPrn);
     
   if (!(bSuccess && !bUserAbort)) {
     char szBuffer[MAX_PATH + 64];
     wsprintf(szBuffer, "Could not print file %s", szFileName);
     MessageBox(hwnd, szBuffer, "Error", MB_OK | MB_ICONEXCLAMATION);
   }
}




extern bool uims_choose_font()
{
   // This operation will take place in the context of the display
   // rather than the printer, but we have to do it that way, because
   // we don't want to open the printer just yet.  The problem that this
   // creates is that the "lfHeight" size will be calibrated wrong.

   LOGFONT lfsave = lf;
   CHOOSEFONT cfsave = cf;

   if (!ChooseFont(&cf)) {
      // Windows is occasionally lacking in common sense.
      // It modifies the "cf" and "lf" structures as we
      // make selections, even if we later cancel the whole thing.
      lf = lfsave;
      cf = cfsave;
   }

   // Now "lf" has all the info, though it is, unfortunately, calibrated
   // for the display.  Also, "cf.iPointSize" has the point size times 10,
   // which is, fortunately, invariant.
   return TRUE;
}


extern void windows_print_this(HWND hwnd, char *szMainTitle, HINSTANCE hInstance,
                               const char *filename)
{
   PrintFile(filename, hwnd, szMainTitle, hInstance);
}



extern void windows_print_any(HWND hwnd, char *szMainTitle, HINSTANCE hInstance)
{
   char szCurDir[_MAX_PATH];
   char szFileToPrint[_MAX_PATH];
   OPENFILENAME ofn;

   (void) GetCurrentDirectory(_MAX_PATH, szCurDir);

   // Put up the dialog box to get the file to print.

   szFileToPrint[0] = 0;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   if (szPrintDir[0])
      ofn.lpstrInitialDir = szPrintDir;
   else      
      ofn.lpstrInitialDir = "";
   ofn.hwndOwner = hwnd;
   ofn.lpstrFilter = 0;
   ofn.lpstrFile = szFileToPrint;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrFileTitle = 0;
   ofn.nMaxFileTitle = 0;
   ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
   ofn.lpstrDefExt = "";

   if (!GetOpenFileName(&ofn))
      return;     // Take no action if we didn't get a file name.

   // GetOpenFileName changed the working directory.
   // We don't want that.  Set it back, after saving it for next print command.
   (void) GetCurrentDirectory(_MAX_PATH, szPrintDir);
   (void) SetCurrentDirectory(szCurDir);

   PrintFile(szFileToPrint, hwnd, szMainTitle, hInstance);
}
