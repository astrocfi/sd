/*
 *  macpref.c - support for user prefereces
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
#include <Folders.h>

/* control indexes */

enum {
    prefLevel = 1,
    prefUseGraphics,
    prefUseMultipleColors,
    prefFirstLevel,
    prefLastLevel = prefFirstLevel + 11
};

#define PREFERENCES_VERSION 0
#define PREFERENCES_SIZE 4

dance_level                   preference_default_level = l_mainstream;
long_boolean                  preference_use_graphics = TRUE;
long_boolean                  preference_use_multiple_colors = TRUE;

static short                  actual_version = PREFERENCES_VERSION;
static FSSpec                 preferences_filespec;
static DialogPtr              preferences_dialog;
static DialogWindow           preferencesW;

static short create_preferences_file(const FSSpec *fs);
static unsigned char preference_get_byte(short file, unsigned char default_value);
static void do_control(DialogWindow *dwp, ControlHandle h, short part_code);
static void save_preferences(void);
static long_boolean write_preferences(void);
static OSErr preference_write_byte(short file, unsigned char value);
static void menu_setup(DialogWindow *dwp);

/*
 *  read_preferences_file
 *
 *  Find and extract the contents of the user preferences file.
 *
 */

void
read_preferences_file(void)
{
    OSErr errcode;
    short file;

    GetIndString((StringPtr)&preferences_filespec.name,
                 EnvironmentStrings, PreferencesFileString);

    errcode = -1;
    if (find_folder_available) {
        errcode = FindFolder(kOnSystemDisk, kPreferencesFolderType,
            kDontCreateFolder, &preferences_filespec.vRefNum,
            &preferences_filespec.parID);
        }
    if (errcode != 0) {
        preferences_filespec.vRefNum = 0; /* use default volume */
        preferences_filespec.parID = 0;   /* use default directory */
    }

    errcode = mac_open_file(&preferences_filespec, fsRdPerm, &file);
    if (errcode != 0) {
        return;
    }

    actual_version = preference_get_byte(file, 0);
    preference_default_level = preference_get_byte(file, 0);
    if (preference_default_level > l_dontshow) {
        preference_default_level = l_dontshow;
    }
    preference_use_graphics = preference_get_byte(file, 1);
    preference_use_multiple_colors = preference_get_byte(file, 1);
    FSClose(file);
}

/*
 *  preference_get_byte
 *
 */

static unsigned char
preference_get_byte(short file, unsigned char default_value)
{
    OSErr result;
    long n = 1;
    unsigned char buf[1];

    result = FSRead(file, &n, buf);
    if (result != 0) {
        return default_value;
    }
    return buf[0];
}

/*
 *  edit_preferences
 *
 */

void
edit_preferences(void)
{
    static DialogRecord dr;

    if (preferences_dialog == NULL) {
        preferences_dialog = GetNewDialog(
            popup_control_available ? PreferencesDialog : OldPreferencesDialog,
            &dr, (WindowPtr) -1);
        if (preferences_dialog == NULL) {
            return;
        }
        dialog_setup(&preferencesW, preferences_dialog);
        preferencesW.base.controlMethod    = (ControlMethod)  do_control;
        preferencesW.base.menuSetupMethod  = (Method)         menu_setup;
        if (popup_control_available) {
            dialog_set_control_value(preferences_dialog, prefLevel,
                preference_default_level + 1);
            }
        else {
            radio_set(preferences_dialog, prefFirstLevel, prefLastLevel,
                prefFirstLevel + preference_default_level);
        }
        dialog_set_control_value(preferences_dialog, prefUseGraphics,
            preference_use_graphics);
        dialog_set_control_value(preferences_dialog, prefUseMultipleColors,
            preference_use_multiple_colors);
    }
    window_select((Window *)&preferencesW);
}

/*
 *  do_control
 *
 *  This function is invoked when the user clicks on a control.
 *
 */

static void
do_control(DialogWindow *dwp, ControlHandle h, short part_code)
{
    int val;

    /*
     *  At present, pop-up menus are not recognized.  Instead, they are
     *  reported with dwp->item == -1.
     *
     */

    if (dwp->item == -1) {
        dwp->item = prefLevel;
    }

    if ((dwp->item >= prefFirstLevel) &&
        (dwp->item <= prefLastLevel)) {
        radio_set(preferences_dialog, prefFirstLevel, prefLastLevel, dwp->item);
        if ((dwp->item - prefFirstLevel) != preference_default_level) {
            preference_default_level = dwp->item - prefFirstLevel;
            save_preferences();
        }
        return;
    }

    switch (dwp->item) {
      case prefLevel:
        val = dialog_get_control_value(preferences_dialog, dwp->item) - 1;
        if (preference_default_level != val) {
            preference_default_level = val;
            save_preferences();
        }
        break;
      case prefUseGraphics:
        val = !dialog_get_control_value(preferences_dialog, dwp->item);
        dialog_set_control_value(preferences_dialog, dwp->item, val);
        preference_use_graphics = val;
        save_preferences();
        main_window_new_preferences();
        break;
      case prefUseMultipleColors:
        val = !dialog_get_control_value(preferences_dialog, dwp->item);
        dialog_set_control_value(preferences_dialog, dwp->item, val);
        preference_use_multiple_colors = val;
        save_preferences();
        main_window_new_preferences();
        break;
    }
}

/*
 *  create_preferences_file
 *
 *  Attempt to create a new preferences file.
 *  Return the number of the open file, or -1 if error.
 *
 */

static short
create_preferences_file(const FSSpec *fs)
{
    short file;
    OSErr result;

    result = mac_create_file(fs, 'SDC4', 'SDPF');
    if (result == 0) {
        /* created file */
        result = mac_open_file(fs, fsRdWrPerm, &file);
        if (result == 0) {
            /* opened file */
            return file;
        }
    }
    return -1;
}

/*
 *  save_preferences
 *
 */

static void
save_preferences(void)
{
    if (write_preferences()) {
        stop_alert(PreferenceErrorAlert, "", "");
    }
}

/*
 *  write_preferences
 *
 *  Return TRUE if error.
 *
 */

static long_boolean
write_preferences(void)
{
    long pos;
    OSErr errcode;
    short file;

    errcode = mac_open_file(&preferences_filespec, fsRdWrPerm, &file);
    if (errcode == fnfErr) {
        file = create_preferences_file(&preferences_filespec);
        if (file == -1) {
            return TRUE;
        }
    }
    else if (errcode != 0) {
        return TRUE;
    }

    /* we don't update the EOF position if the file is a later version than
       the one we understand, assuming there is more stuff in the file */

    errcode = 0
        || SetFPos(file, fsFromStart, 0)
        || preference_write_byte(file, PREFERENCES_VERSION)
        || preference_write_byte(file, preference_default_level)
        || preference_write_byte(file, preference_use_graphics)
        || preference_write_byte(file, preference_use_multiple_colors)
        || GetFPos(file, &pos)
        || ((actual_version <= PREFERENCES_VERSION) ? SetEOF(file, pos) : 0)
        || FSClose(file);
    if (errcode != 0) {
        return TRUE;
    }
    return FALSE;
}

/*
 *  preference_write_byte
 *
 */

static OSErr
preference_write_byte(short file, unsigned char value)
{
    long n = 1;
    unsigned char buf[1];

    buf[0] = value;
    return FSWrite(file, &n, buf);
}

/*
 *  menu_setup
 *
 */

static void
menu_setup(DialogWindow *dwp)
{
    EnableItem(apple_menu, hintsCommand);
    EnableItem(sd_menu, 0);
    EnableItem(sd_menu, compileDatabaseCommand);
}
