/*
 *  maccomp.c - interface to database compiler
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

#include "paths.h"
#include "macguts.h"
#include <stdio.h> /* for vsprintf */
#include <string.h> /* for strcpy */
#include <setjmp.h>
#include <stdarg.h>

extern void dbcompile(void);

static jmp_buf exit_place;
static char *error_message_p; /* where compiler writes error message */

enum {
    compileAccept = 1,
    compileSetInput,
    compileSetOutput,
    compileUser,
    compileInputName,
    compileOutputName
};

static void compile_control(DialogWindow *dwp, ControlHandle h, short part_code);
static void compile_update(DialogWindow *dwp);
static void compile_database(void);
static void compile_menu_setup(DialogWindow *dwp);

static DialogWindow compileDW;
static DialogPtr compile_dialog;

static FSSpec db_input_fs;
static FSSpec db_output_fs;

short db_input = -1;
short db_output = -1;
char db_input_filename[200];
char db_output_filename[200];

/*
 *  open_compile_dialog
 *
 *  The UI calls this function to initiate the compilation dialog.
 *  The user gets to specify an input file and an output file before
 *  running the compiler.
 *
 */

void
open_compile_dialog(void)
{
    static DialogRecord dr;

    if (compile_dialog == NULL) {

        /* create the dialog */

        compile_dialog = GetNewDialog(CompileDatabaseDialog, &dr, (WindowPtr) -1L);
        if (compile_dialog == NULL) {
            return;
        }
        dialog_setup(&compileDW, compile_dialog);
        compileDW.default_item          =                  compileAccept;
        compileDW.base.controlMethod    = (ControlMethod)  compile_control;
        compileDW.base.updateMethod     = (Method)         compile_update;
        compileDW.base.menuSetupMethod  = (Method)         compile_menu_setup;
        setup_dialog_box(compile_dialog, compileUser);

        /* initialize default input and output file names */

        db_input_fs.vRefNum = 0; /* default volume */
        db_input_fs.parID = 0;   /* default directory */
        strcpy(db_input_filename, CALLS_FILENAME);
        strcpy((char *)db_input_fs.name, db_input_filename);
        CtoPstr(db_input_fs.name);
        db_output_fs.vRefNum = 0; /* default volume */
        db_output_fs.parID = 0;   /* default directory */
        strcpy(db_output_filename, DATABASE_FILENAME);
        strcpy((char *)db_output_fs.name, db_output_filename);
        CtoPstr(db_output_fs.name);
    }
    window_select((Window *)&compileDW);
}

/*
 *  compile_update
 *
 *  This method gets called to update the display.
 *
 */

static void
compile_update(DialogWindow *dwp)
{
    Rect r;
    Handle h;
    short kind;
    short old_font;
    short old_size;

    dialog_update(dwp);

    old_font = thePort->txFont;
    old_size = thePort->txSize;
    TextFont(geneva);
    TextSize(10);

    GetDItem(compile_dialog, compileInputName, &kind, &h, &r);
    TextBox(db_input_filename, strlen(db_input_filename), &r, teFlushDefault);

    GetDItem(compile_dialog, compileOutputName, &kind, &h, &r);
    TextBox(db_output_filename, strlen(db_output_filename), &r, teFlushDefault);

    TextFont(old_font);
    TextSize(old_size);
}

/*
 *  compile_control
 *
 *  This method gets called when the user clicks on a button.
 *
 */

static void
compile_control(DialogWindow *dwp, ControlHandle h, short part_code)
{
    Str63 prompt;

    switch (dwp->item) {
      case compileAccept:
        compile_database();
        break;
      case compileSetInput:
        if (get_input_file_name(db_input_filename, &db_input_fs, 'TEXT')) {
            SetPort(dwp->base.window);
            InvalRect(&dwp->base.window->portRect);
        }
        break;
      case compileSetOutput:
        GetIndString(prompt, MessagesStrings, CompileOutputPromptString);
        if (get_output_file_name(db_output_filename, &db_output_fs, prompt)) {
            SetPort(dwp->base.window);
            InvalRect(&dwp->base.window->portRect);
        }
        break;
    }   
}

/*
 *  compile_menu_setup
 *
 */

static void
compile_menu_setup(DialogWindow *dwp)
{
    EnableItem(apple_menu, hintsCommand);
    EnableItem(sd_menu, 0);
    EnableItem(sd_menu, editPreferencesCommand);
}

/*
 *  compile_database
 *
 *  Do the compilation, using the files specified by the user.
 *
 */

static void
compile_database(void)
{
    OSErr result;
    char error_message[500];

    busy_cursor();
    error_message_p = error_message;
    error_message[0] = '\0';
    if (setjmp(exit_place)) {
        /* control comes here after a compilation error */
        char *p = error_message;
        while (*p) {
            if (*p == '\n') {
                *p = '\r';
            }
            p++;
        }
        db_cleanup();
        if (error_message[0] != '\0') {
            /* error from dbcomp not yet reported */
            stop_alert(DatabaseCompileErrorAlert, error_message, "");
        }
        return;
    }
    if (mac_open_file(&db_input_fs, fsRdPerm, &db_input) != 0) {
        db_input_error();
    }
    mac_delete_file(&db_output_fs);
    result = mac_create_file(&db_output_fs, 'SDC4', 'SDCD');
    if (result == 0 || result == dupFNErr) {
        result = mac_open_file(&db_output_fs, fsRdWrPerm, &db_output);
        }
    if (result != 0) {
        db_output_error();
    }
    dbcompile();
    adjust_cursor();
    window_close((Window *)&compileDW);
}

void
do_exit(void)
{
    longjmp(exit_place, 1);
}

void
dbcompile_signoff(int bytes, int calls)
{
    /* not useful to display this info if dialog closes by itself */
}

int
do_printf(const char *fmt, ...)
{
   int n;
   va_list ap;

   va_start(ap, fmt);
   n = vsprintf(error_message_p, fmt, ap);
   va_end(ap);
   error_message_p += n;
   return n;
}
