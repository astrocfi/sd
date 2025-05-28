/*
 *  macstart.c - startup dialog
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
#include <stdio.h> /* for sprintf */
#include <string.h> /* for strcpy */

#define NARGS 25

/* dialog box item numbers */

enum {
    itemCreate = 1,
    itemLevel,
    itemAbridge,
    itemUser,
    itemFirstLevel,
    itemLastLevel = itemFirstLevel + 11,
    itemWriteList /* fake item, not really in dialog box */
};

enum {save_call_list_checkbox = 13};
enum {old_save_call_list_checkbox = 9};

/* the following must agree with similar definitions in sd */
static char *levels[] = {
    "m", "p", "a1", "a2", "c1", "c2", "c3a", "c3", "c3x", "c4a", "c4", "all"
    };

static char *argv[NARGS+1];
static FSSpec call_list_filespec;
static char call_list_filename[200];
static int include_lower;
static int database_ok;
static DialogWindow startupDW;

static void startup_menu_setup(DialogWindow *dwp);
static void select_default_call_database(void);
static void set_call_database(char *filename, FSSpec *fs);

pascal short save_call_list_dialog_hook(short item, DialogPtr dp, void *data);
static int get_call_list_output_file_name(char *buf, FSSpec *fs);
pascal short old_save_call_list_dialog_hook(short item, DialogPtr dp);
static short common_save_call_list_dialog_hook(short item, DialogPtr dp, int checkbox);
static int old_get_call_list_output_file_name(char *buf, FSSpec *fs);
static int old_get_output_file_name(char *buf, FSSpec *fs, unsigned char *prompt);
static int old_get_input_file_name(char *buf, FSSpec *fs, long filetype);
static int explode_standard_file_reply(StandardFileReply *p, char *buf, FSSpec *fs);
static int explode_old_standard_file_reply(SFReply *p, char *buf, FSSpec *fs);

/*
 *  getcommand - process "command line"
 *
 */

int
getcommand(char ***av)
{
    short argc, file, result, job, filecount, vol;
    long dir, temp;
    int option, level;
    DialogPtr dp;
    AppFile filedesc;
    FSSpec database_fs;

    show_about_sd();
    read_preferences_file();

    /* see if the user opened a call database file... */

    CountAppFiles(&job, &filecount);
    if (filecount > 0) {
        char buf[200];
        GetAppFiles(1, &filedesc);

        /* convert AppFile to FSSpec */
        mac_fixup_file(filedesc.vRefNum, (StringPtr)&filedesc.fName, &database_fs);

        pstrcpy((StringPtr)buf, filedesc.fName);
        set_call_database(PtoCstr((StringPtr)buf), &database_fs);
        if (database_ok) {
            ClrAppFiles(1);
        }
    }

    option = -1; /* no option selected */
    
    /*  present dialog  */
        
    dp = GetNewDialog(popup_control_available ? LevelDialog : OldLevelDialog,
                      0L, (WindowPtr) -1L);
    if (popup_control_available) {
        SetCtlValue((ControlHandle)ditem(dp, itemLevel), preference_default_level + 1);
    }
    else {
        level = preference_default_level;
        radio_set(dp, itemFirstLevel, itemLastLevel, itemFirstLevel + level);
    }
    dialog_setup(&startupDW, dp);
    startupDW.default_item = itemCreate;
    startupDW.base.menuSetupMethod = (Method) startup_menu_setup;
    setup_dialog_box(dp, itemUser); /* enable border around default button */
    window_select((Window *)&startupDW);
    
    /*  engage in dialog  */

    restart_dialog:
        
    do {
        movable_modal_dialog(&startupDW);
        if ((startupDW.item >= itemFirstLevel) &&
            (startupDW.item <= itemLastLevel)) {
            radio_set(dp, itemFirstLevel, itemLastLevel, startupDW.item);
            level = startupDW.item - itemFirstLevel;
        }
        else {
            switch (startupDW.item) {
                case itemCreate:
                    option = startupDW.item;
                    break;
                case itemAbridge:
                    if (get_input_file_name(call_list_filename,
                            &call_list_filespec, 'TEXT')) {
                        option = startupDW.item;
                    }
                    break;
                case itemWriteList:
                    option = startupDW.item;
                    break;
                case itemLevel:
                    break;
            }
        }
    } while (option == -1);
    
    /*  construct command line  */
        
    argc = 0;
    argv[argc++] = "sd";
    if (popup_control_available) {
        level = GetCtlValue((ControlHandle)ditem(dp,itemLevel))-1;
    }
    level_name = getout_strings[level];
    argv[argc++] = levels[level];
    if (option == itemWriteList) {
        result = mac_create_file(&call_list_filespec, 'ttxt', 'TEXT');
        if (result == 0 || result == dupFNErr) {
            result = mac_open_file(&call_list_filespec, fsRdWrPerm, &file);
        }
        if (result == 0) {
            set_call_list_file(file);
            if (include_lower) {
                argv[argc++] = "-write_full_list";
            }
            else{
                argv[argc++] = "-write_list";
            }
            argv[argc++] = call_list_filename;
        }
        else {
            stop_alert(CallListCreateErrorAlert, call_list_filename, "");
            goto restart_dialog;
        }
    }
    if (option == itemAbridge) {
        result = mac_open_file(&call_list_filespec, fsRdPerm, &file);
        if (result == 0) {
            set_call_list_file(file);
            argv[argc++] = "-abridge";
            argv[argc++] = call_list_filename;
        }
        else {
            stop_alert(CallListOpenErrorAlert, call_list_filename, "");
            goto restart_dialog;
        }
    }
    *av = argv;

    if (!database_ok) {
        select_default_call_database();
    }
    /* User might want to load a new database *or* recompile an existing database.
       Presenting a message saying "you must recompile", and then putting a dialog
       box asking for the new compiled database, is not very good. */
    if (!database_ok)
        goto restart_dialog;
    window_close((Window *)&startupDW);
    display_update();
    DisposDialog(dp);
    return(argc);
}

/*
 *  startup_save_call_list
 *
 *  Invoked as a menu command, it must trick the modal dialog into terminating.
 *  Returns FALSE if selected window is not the startup dialog.
 *
 */

long_boolean
startup_save_call_list(void)
{
    if (active_window() != (Window *)&startupDW) {
        return FALSE;
    }
    if (get_call_list_output_file_name(call_list_filename, &call_list_filespec)) {
        startupDW.item = itemWriteList;
    }
    return TRUE;
}

/*
 *  startup_menu_setup
 *
 *  This method is called to enable menu items that are valid
 *  while the startup dialog is active.
 *
 */

static void
startup_menu_setup(DialogWindow *dwp)
{
    EnableItem(apple_menu, hintsCommand);
    DisableItem(edit_menu, 0);
    EnableItem(sd_menu, 0);
    EnableItem(sd_menu, saveCallListCommand);
    EnableItem(sd_menu, selectDatabaseCommand);
    EnableItem(sd_menu, compileDatabaseCommand);
    EnableItem(sd_menu, editPreferencesCommand);
}

/*
 *  select_default_call_database
 *
 */

static void
select_default_call_database(void)
{
    char fname[100];
    FSSpec database_fs;

    strcpy((char *)database_fs.name, DATABASE_FILENAME);
    CtoPstr((char *)database_fs.name);
    database_fs.vRefNum = 0; /* default volume */
    database_fs.parID = 0;   /* default directory */
    set_call_database(DATABASE_FILENAME, &database_fs);
}

/*
 *  select_call_database
 *
 *  Return FALSE if user cancelled.
 *
 */

long_boolean
select_call_database(void)
{
    FSSpec database_fs;
    static char buf[200];
    int result;

    if (result = get_input_file_name(buf, &database_fs, 'SDCD')) {
        set_call_database(buf, &database_fs);
    }
    return result;
}

/*
 *  set_call_database
 *
 *  Identify the call database file.  FILENAME is used only for error
 *  messages.
 *
 */

static void
set_call_database(char *filename, FSSpec *fs)
{
    int result;
    short file;
    char buf[200];

    result = mac_open_file(fs, fsRdPerm, &file);
    if (result != 0) {
        stop_alert(DatabaseCantOpenAlert, filename, "");
        return;
    }
    result = set_database_file(filename, file);
    if (result == 0) {
        database_ok = TRUE;
    }
}

static Point where = { 100, 100 };

extern void fixup_window_order(void); /* gross kludge */

/*
 *  get_output_file_name
 *
 *  Ask the use for an output file name.  Return TRUE if one is provided,
 *  FALSE otherwise.  Note that PROMPT is a Pascal string.
 *
 */

int
get_output_file_name(char *buf, FSSpec *fs, StringPtr prompt)
{
    StandardFileReply scratch;
    
    deactivate_front_window();
    if (!new_file_package_available) {
        return old_get_output_file_name(buf, fs, prompt);
    }
    StandardPutFile(prompt, "\p", &scratch);
    fixup_window_order();
    return explode_standard_file_reply(&scratch, buf, fs);
}

/*
 *  old_get_output_file_name (for old system)
 *
 */

static int
old_get_output_file_name(char *buf, FSSpec *fs, StringPtr prompt)
{
    SFReply scratch;
    
    SFPutFile(where, prompt, "\p", 0L, &scratch);
    fixup_window_order();
    return explode_old_standard_file_reply(&scratch, buf, fs);
}

/*
 *  get_call_list_output_file_name
 *
 *  Ask the user for the name of a call list to be written.
 *  Return TRUE is the user provides one.
 *
 */

static int
get_call_list_output_file_name(char *buf, FSSpec *fs)
{
    StandardFileReply scratch;
    
    deactivate_front_window();
    if (!new_file_package_available) {
        return old_get_call_list_output_file_name(buf, fs);
    }
    CustomPutFile("\pSave call list as:", "\p", &scratch,
               SaveCallListDialog, where,
               save_call_list_dialog_hook, 0L, 0L, 0L, 0L);
    fixup_window_order();
    return explode_standard_file_reply(&scratch, buf, fs);
}

/*
 *  old_get_call_list_output_file_name (for old system)
 *
 */

static int
old_get_call_list_output_file_name(char *buf, FSSpec *fs)
{
    SFReply scratch;
    
    SFPPutFile(where, "\pSave call list as:", "\p",
               old_save_call_list_dialog_hook, &scratch,
               OldSaveCallListDialog, 0L);
    fixup_window_order();
    return explode_old_standard_file_reply(&scratch, buf, fs);
}

/*
 *  save_call_list_dialog_hook
 *
 *  This hook function implements the checkbox for saving all calls valid at
 *  the current level, or just the ones introduced by the current level.
 *
 */

pascal short
save_call_list_dialog_hook(short item, DialogPtr dp, void *data)
{
    return common_save_call_list_dialog_hook(item, dp, save_call_list_checkbox);
}

/*
 *  old_save_call_list_dialog_hook (for old systems)
 *
 */

pascal short
old_save_call_list_dialog_hook(short item, DialogPtr dp)
{
    return common_save_call_list_dialog_hook(item, dp, old_save_call_list_checkbox);
}

/*
 *  common_save_call_list_dialog_hook (common code)
 *
 */

static short
common_save_call_list_dialog_hook(short item, DialogPtr dp, int checkbox)
{
    ControlHandle h;

    if (GetWRefCon((WindowPtr)dp) != sfMainDialogRefCon) {
        return item;
    }
    if (item == sfHookFirstCall) {
        include_lower = 1;
        SetCtlValue((ControlHandle) ditem(dp, checkbox), 1);
        return sfHookNullEvent;
        }
    if (item != checkbox) {
        return item;
    }
    h = (ControlHandle) ditem(dp, item);
    SetCtlValue(h, include_lower = !GetCtlValue(h));
    return sfHookNullEvent;
}

/*
 *  get_input_file_name
 *
 */

int
get_input_file_name(char *buf, FSSpec *fs, long filetype)
{
    StandardFileReply scratch;
    SFTypeList myTypes;
    int ntypes;
    int result;
    long proc;
    
    deactivate_front_window();
    if (!new_file_package_available) {
        return old_get_input_file_name(buf, fs, filetype);
    }
    if (filetype) {
        ntypes = 1;
        myTypes[0] = filetype;
    }
    else {
        ntypes = -1;
    }
    StandardGetFile(0L, ntypes, myTypes, &scratch);
    fixup_window_order();
    return explode_standard_file_reply(&scratch, buf, fs);
}

/*
 *  old_get_input_file_name (for old systems)
 *
 */

static int
old_get_input_file_name(char *buf, FSSpec *fs, long filetype)
{
    SFReply scratch;
    SFTypeList myTypes;
    int ntypes;

    if (filetype != 0) {
        ntypes = 1;
        myTypes[0] = filetype;
    }
    else {
        ntypes = -1;
    }
    SFGetFile(where, "\p", 0L, ntypes, myTypes, 0L, &scratch);
    fixup_window_order();
    return explode_old_standard_file_reply(&scratch, buf, fs);
}

/*
 *  explode_standard_file_reply
 *
 */

static int
explode_standard_file_reply(StandardFileReply *p, char *buf, FSSpec *fs)
{
    if (!p->sfGood) {
        return FALSE;
    }
    fs->vRefNum = p->sfFile.vRefNum;
    fs->parID = p->sfFile.parID;
    pstrcpy(fs->name, p->sfFile.name);
    strcpy(buf, PtoCstr(p->sfFile.name));
    return TRUE;
}

/*
 *  explode_old_standard_file_reply (for old systems)
 *
 */

static int
explode_old_standard_file_reply(SFReply *p, char *buf, FSSpec *fs)
{
    int result;
    long proc;
    
    if (!p->good) {
        return FALSE;
    }
    result = mac_fixup_file(p->vRefNum, (StringPtr)&p->fName, fs);
    if (result != 0) {
        return FALSE;
    }
    pstrcpy((StringPtr)buf, p->fName);
    PtoCstr((unsigned char *)buf);
    return TRUE;
}

/*
 *  pstrcpy
 *
 *  Copies a Pascal string from P2 to P1.
 *
 */

void
pstrcpy(StringPtr p1, const StringPtr p2)
{
    register int len;
    register unsigned char *s;
    
    s = p2;
    len = *p1++ = *s++;
    while (--len >= 0) {
        *p1++ = *s++;
    }
}
