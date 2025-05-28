/*
 *  macmenu.c - support for pull-down menus
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

MenuHandle          apple_menu;
MenuHandle          file_menu;
MenuHandle          edit_menu;
MenuHandle          sequence_menu;
MenuHandle          sd_menu;
static MenuHandle   modify_menu; /* submenu */
static MenuHandle   log_menu;    /* submenu */

static FSSpec sequence_filespec; /* current file for saving sequence */
static FSSpec log_filespec;      /* current file for appending to log */

static void apple_menu_command(int item);
static void file_menu_command(int item);
static void edit_menu_command(int item);
static void sequence_menu_command(int item);
static void sd_menu_command(int item);
static void modify_menu_command(int item);
static void log_menu_command(int item);
static void save_call_list_command(void);
static void log_new_command(void);
static void log_append_command(void);
static void log_append_to_file_command(void);
static void quit_command(void);
static long_boolean mac_write_sequence_to_file(void);
static void cant_open_resource_file(void);
static int write_call_list_file(const char *fn);

/*
 **********************************************************************
 *  PUBLIC FUNCTIONS
 **********************************************************************
 */

/*
 *  init_menus
 *
 */

void
init_menus(void)
{
    apple_menu = GetMenu(AppleMenu);
    if (apple_menu == NULL) {
        cant_open_resource_file();
    }
    AddResMenu(apple_menu, 'DRVR');
    file_menu     = GetMenu(FileMenu);
    edit_menu     = GetMenu(EditMenu);
    sequence_menu = GetMenu(SequenceMenu);
    sd_menu       = GetMenu(SdMenu);
    InsertMenu(apple_menu, 0);
    InsertMenu(file_menu, 0);
    InsertMenu(edit_menu, 0);
    InsertMenu(sequence_menu, 0);
    InsertMenu(sd_menu, 0);
    modify_menu = GetMenu(ModifyMenu);
    log_menu    = GetMenu(LogMenu);
    InsertMenu(modify_menu, -1);
    InsertMenu(log_menu, -1);
    DrawMenuBar();
    SetItemMark(sequence_menu, anyConceptCommand, allowing_all_concepts ? 022 : 0);
}

/*
 *  adjust_menus
 *
 *  Adjust the enabled/disabled status of the menus and menu items based on
 *  the current state of the world.
 *
 */

void
adjust_menus(void)
{
    Window *wp = active_window();
    static unsigned char undo_string[100];
    static unsigned char show_couple_numbers_string[100];
    static unsigned char hide_couple_numbers_string[100];

    if (undo_string[0] == '\0') {
        GetIndString(undo_string, MessagesStrings, UndoString);
        GetIndString(show_couple_numbers_string, MessagesStrings, ShowCoupleNumbersString);
        GetIndString(hide_couple_numbers_string, MessagesStrings, HideCoupleNumbersString);
    }
    SetItem(edit_menu, undoCommand, undo_string);
    SetItem(sequence_menu, showCoupleNumbersCommand,
        display_numbers ? hide_couple_numbers_string : show_couple_numbers_string);

    /*
     *  First, disable everything that is not permanently enabled.
     *  Then call the window's menu setup method to enable selective
     *  menus and commands.
     *
     */

    DisableItem(sequence_menu, 0);
    DisableItem(sd_menu, 0);
    EnableItem(edit_menu, 0);

    DisableItem(apple_menu, hintsCommand); /* can't use when modal is up */

    DisableItem(file_menu, saveCommand);
    DisableItem(file_menu, saveasCommand);
    DisableItem(file_menu, logCommand);

    DisableItem(sequence_menu, endCommand);
    DisableItem(sequence_menu, resolveCommand);
    DisableItem(sequence_menu, reconcileCommand);
    DisableItem(sequence_menu, randomCommand);
    DisableItem(sequence_menu, niceCommand);
    DisableItem(sequence_menu, labelCommand);
    DisableItem(sequence_menu, commentCommand);
    DisableItem(sequence_menu, pictureCommand);
    DisableItem(sequence_menu, anyConceptCommand);
    DisableItem(sequence_menu, modifyCommand);
    DisableItem(sequence_menu, showCoupleNumbersCommand);
    
    DisableItem(sd_menu, saveCallListCommand);
    DisableItem(sd_menu, selectDatabaseCommand);
    DisableItem(sd_menu, compileDatabaseCommand);
    DisableItem(sd_menu, editPreferencesCommand);

    DisableItem(edit_menu, undoCommand);
    DisableItem(edit_menu, cutCommand);
    DisableItem(edit_menu, clearCommand);
    DisableItem(edit_menu, pasteCommand);
    DisableItem(edit_menu, copyCommand);
    DisableItem(edit_menu, selectAllCommand);
    DisableItem(edit_menu, copySpecialCommand);

    if ((wp != NULL) && (wp->menuSetupMethod != NULL)) {
        (*wp->menuSetupMethod)(wp);
    }

    DrawMenuBar();
}

/*
 *  menu_command
 *
 */

void
menu_command(long menu_result)
{
    int item;
    
    item = LoWord(menu_result);
    switch (HiWord(menu_result)) {
        case AppleMenu:
            apple_menu_command(item);
            break;

        case FileMenu: 
            file_menu_command(item);
            break;

        case EditMenu: 
            if (SystemEdit(item) == false) {
                edit_menu_command(item);
            }
            break;

        case SequenceMenu:
            sequence_menu_command(item);
            break;

        case SdMenu:
            sd_menu_command(item);
            break;

        case ModifyMenu:
            modify_menu_command(item);
            break;

        case LogMenu:
	    log_menu_command(item);
	    break;

    }
    HiliteMenu(0);
}

/*
 *  update_modification_state
 *
 */

void
update_modification_state(int n)
{
    int i;
    
    allowing_modifications = n;
    for (i=1;i<=3;++i) {
        SetItemMark(modify_menu, i, i==(n+1) ? 022 : 0);
    }
}

/*
 *  save_command
 *
 */

void
save_command(void)
{
    if (!output_file_save_ok) {
        save_as_command();
    }
    else {
        set_output_file(&sequence_filespec);
        if (mac_write_sequence_to_file()) {
            dirty = FALSE;
        }
    }
}

/*
 *  save_as_command
 *
 */

void
save_as_command(void)
{
    int result;
    char buf[200];
    FSSpec new_sequence_filespec;
    Str63 prompt;

    GetIndString(prompt, MessagesStrings, SaveSequencePromptString);
    result = get_output_file_name(buf, &new_sequence_filespec, prompt);
    if (result == 0) {
        /* user cancelled */
        return;
    }
    result = mac_create_file(&new_sequence_filespec, 'ttxt', 'TEXT');
    if (result == dupFNErr) {
        result = 0;
    }
    if (result == 0) {
	sequence_filespec = new_sequence_filespec;
        output_file_save_ok = TRUE;
        set_output_file(&sequence_filespec);
	pstrcpy((StringPtr)sequence_file_name, sequence_filespec.name);
    PtoCstr((StringPtr)sequence_file_name);
	set_output_window_title();
        mac_write_sequence_to_file(); /* sets file_error */
        if (!file_error) {
            dirty = FALSE;
        }
    }
    else {
        stop_alert(SequenceCreateErrorAlert, buf, "");
    }
}

/*
 **********************************************************************
 *  PRIVATE FUNCTIONS
 **********************************************************************
 */

/*
 *  apple_menu_command
 *
 */

static void
apple_menu_command(int item)
{
    Str255 name;

    switch (item) {
        case aboutCommand:
            show_about_sd();
            break;
        case hintsCommand:
            show_hints();
            break;
        default:
            GetItem(apple_menu, item, name);
            OpenDeskAcc(name);
        }
}

/*
 *  file_menu_command
 *
 */

static void
file_menu_command(int item)
{
    switch (item) {
        case saveCommand:
            save_command();
            break;
        case saveasCommand:
            save_as_command();
            break;
        case quitCommand:
            quit_command();
            break;
    }
}

/*
 *  edit_menu_command
 *
 */

static void
edit_menu_command(int item)
{
    Window *wp = active_window();
    if (wp != NULL) {
        window_edit(wp, item);
    }
}

/*
 *  mac_write_sequence_to_file
 *
 *  This function is like write_sequence_to_file, except that it
 *  inhibits display of the text being written.
 *
 */

static long_boolean
mac_write_sequence_to_file(void)
{
    long_boolean result;
    writing_sequence = TRUE;
    result = write_sequence_to_file();
    writing_sequence = FALSE;
    stuff_command(ui_command_select, command_refresh); /* ensure in sync */
}

/*
 *  quit_command
 *
 */

static void
quit_command(void)
{
    if (uims_do_abort_popup() == POPUP_ACCEPT) {
        exit_program(0);
    }
}

/*
 *  modify_menu_command
 *
 */

static void
modify_menu_command(int item)
{
    update_modification_state(item-1);
}

/*
 *  log_menu_command
 *
 */

static void
log_menu_command(int item)
{
    switch (item) {
      case logNewCommand:
	log_new_command();
	break;
      case logAppendCommand:
	log_append_command();
	break;
      case logAppendToFileCommand:
	log_append_to_file_command();
	break;
    }
}

/*
 *  log_new_command (append sequence to a new log file)
 *
 */

static void
log_new_command(void)
{
    int result;
    char buf[200];
    FSSpec new_log_filespec;
    Str63 prompt;

    GetIndString(prompt, MessagesStrings, NewLogPromptString);
    result = get_output_file_name(buf, &new_log_filespec, prompt);
    if (result == 0) {
        /* user cancelled */
        return;
    }
    result = mac_create_file(&new_log_filespec, 'ttxt', 'TEXT');
    if (result == dupFNErr) {
        result = 0;
    }
    if (result == 0) {
	log_filespec = new_log_filespec;
        log_file_append_ok = TRUE;
        set_output_file(&log_filespec);
	pstrcpy((StringPtr)log_file_name, log_filespec.name);
    PtoCstr((StringPtr)log_file_name);
	set_output_window_title();
        mac_write_sequence_to_file(); /* sets file_error */
        if (!file_error) {
            dirty = FALSE;
        }
    }
    else {
        stop_alert(LogCreateErrorAlert, buf, "");
    }
}
/*
 *  log_append_command (append to current log file)
 *
 */

static void
log_append_command(void)
{
    int result;
    
    if (!log_file_append_ok) {
        log_append_to_file_command();
    }
    else {
	set_output_file_append(&log_filespec);
        if (mac_write_sequence_to_file()) {
            dirty = FALSE;
        }
    }
}

/*
 *  log_append_to_file_command (append to existing log file)
 *
 */

static void
log_append_to_file_command(void)
{
    int result;
    char buf[200];
    FSSpec new_log_filespec;

    result = get_input_file_name(buf, &new_log_filespec, 'TEXT');
    if (result == 0) {
        /* user cancelled */
        return;
    }
    log_filespec = new_log_filespec;
    log_file_append_ok = TRUE;
    set_output_file_append(&log_filespec);
    pstrcpy((StringPtr)log_file_name, log_filespec.name);
    PtoCstr((StringPtr)log_file_name);
    set_output_window_title();
    mac_write_sequence_to_file(); /* reports errors directly */
    if (!file_error) {
	dirty = FALSE; /* don't ask user to save changes */
    }
}

/*
 *  save_call_list_command
 *
 */

static void
save_call_list_command(void)
{
    char fn[200];
    int rc;

    stop_alert(UnimplementedAlert, "", "");
    return;

#if 0
    if (get_output_file_name(fn, "\pSave call list as:")) {
        if (rc = write_call_list_file(fn)) {
            stop_alert(WriteCallListAlert, fn, "");
        }
    }
#endif

}

/*
 *  sequence_menu_command
 *
 */

static void
sequence_menu_command(int item)
{
    switch (item) {
    case resolveCommand:
        stuff_command(ui_command_select, command_resolve);
        break;
    case reconcileCommand:
        stuff_command(ui_command_select, command_reconcile);
        break;
    case randomCommand:
        stuff_command(ui_command_select, command_anything);
        break;
    case niceCommand:
        stuff_command(ui_command_select, command_nice_setup);
        break;
    case commentCommand:
        stuff_command(ui_command_select, command_create_comment);
        break;
    case pictureCommand:
        stuff_command(ui_command_select, command_save_pic);
        break;
    case anyConceptCommand:
        allowing_all_concepts = !allowing_all_concepts;
        SetItemMark(sequence_menu, item, allowing_all_concepts ? 022 : 0);
        stuff_command(ui_command_select, command_refresh);
        break;
    case showCoupleNumbersCommand:
        display_numbers = !display_numbers;
        adjust_menus();
        stuff_command(ui_command_select, command_refresh);
        break;
    case endCommand:
        stuff_command(ui_command_select, command_abort);
        break;
    case labelCommand:
        open_label_dialog();
        break;
    }
}

/*
 *  sd_menu_command
 *
 */

static void
sd_menu_command(int item)
{
    switch (item) {
    case saveCallListCommand:
        if (!startup_save_call_list()) {
            save_call_list_command();
        }
        break;
    case selectDatabaseCommand:
        select_call_database();
        break;
    case compileDatabaseCommand:
        open_compile_dialog();
        break;
    case editPreferencesCommand:
        edit_preferences();
        break;
    }
}

/*
 *  cant_open_resource_file
 *
 */

static void
cant_open_resource_file(void)
{
    Rect r;
    WindowPtr w;

    SetRect(&r, 152, 60, 356, 132);
    w = NewWindow((Ptr) 0L, &r, "\p", true, dBoxProc, (WindowPtr)-1L, false, 0L);
    if (w != NULL) {
        SetPort(w);
        TextFont(0);
        MoveTo(4, 20);
        DrawString("\pCan't open resource file.");
        MoveTo(4, 40);
        DrawString("\pClick mouse to exit.");
        do {
        } while (!Button());
    }
    exit_program(2);
}

/*
 *  write_call_list_file
 *
 */

static int
write_call_list_file(const char *fn)
{
    int i;

    if (open_call_list_file(call_list_mode_writing_full, (char *)fn))
        return 1;
    for (i=0; i<number_of_calls[call_list_any]; i++) {
        write_to_call_list_file(main_call_lists[call_list_any][i]->name);
    }
    if (close_call_list_file())
        return 2;
    return 0;
}
