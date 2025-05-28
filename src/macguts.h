/*
 *  macguts.h - header file for Macintosh-specific code
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

#include "sd.h"
#include "sdmatch.h"

/* resource IDs of windows */

#define OutputWindow               128
#define MatchWindow                129
#define RecpointWindow             130
#define HintsWindow                131

/* resource IDs of alerts */

#define DiscardSequenceAlert       200
#define ReplaceAnyAlert            201
#define ReplaceTagAlert            202
#define ReplaceScootAlert          203
#define UnimplementedAlert         204
#define WriteCallListAlert         205
#define DatabaseBadFormatAlert     206
#define DatabaseWrongVersionAlert  207
#define ErrorAlert                 208
#define DatabaseCantOpenAlert      209
#define SaveSequenceAlert          210
#define DatabaseErrorAlert         211
#define DatabaseErrorCallAlert     212
#define DatabaseCompileErrorAlert  213
#define PreferenceErrorAlert       214
#define SequenceWriteErrorAlert    215
#define SequenceCreateErrorAlert   216
#define DatabaseReadErrorAlert     217
#define CallListReadErrorAlert     218
#define CallListWriteErrorAlert    219
#define DatabaseCompileReadAlert   220
#define DatabaseCompileWriteAlert  221
#define CallListCreateErrorAlert   222
#define CallListOpenErrorAlert     223

/* resource IDs of dialogs */

#define LabelDialog           128
#define AboutSDDialog         129
#define LevelDialog           130
#define CompileDatabaseDialog 131
#define OldLevelDialog        132
#define OldPreferencesDialog  133
#define SaveCallListDialog    134
#define OldSaveCallListDialog 135
#define ReadingDatabaseDialog 136
#define SelectDialog          137
#define QuantifierDialog      138
#define PreferencesDialog     140
#define OldSelectDialog       141
#define OldQuantifierDialog   142

/* resource IDs of menus */

#define AppleMenu          128
#define FileMenu           129
#define EditMenu           130
#define SequenceMenu       132
#define SdMenu             134
#define ModifyMenu         234

/* resource IDs of strings */

#define MessagesStrings          128
#define UndoString                 1
#define UndoCallString             2
#define StartSequenceString        3
#define UndoConceptString          4
#define FindSolutionString         5
#define FindAnotherString          6
#define PickAnotherString          7
#define ShowCoupleNumbersString    8
#define HideCoupleNumbersString    9
#define HintsString               10
#define TitleString               11
#define CopyrightString           12
#define WarrantyString            13
#define FreeSoftwareString        14
#define GNULicenseString          15
#define CompileOutputPromptString 16
#define SaveSequencePromptString  17

#define SearchGoalStrings        129

#define EnvironmentStrings       130
#define PreferencesFileString      1

/* Apple menu command indices */

enum {
    aboutCommand = 1,
    hintsCommand = 2
};

/* File menu command indices */

enum {
    saveCommand = 1,
    saveasCommand,
    FILE_MENU_DUMMY_1,
    appendFileCommand,
    FILE_MENU_DUMMY_2,
    quitCommand
};

/* Edit menu command indices */

enum {
    idleCommand = -1, /* special "fake" command */
    undoCommand = 1,
    EDIT_MENU_DUMMY_1,
    cutCommand,
    copyCommand,
    pasteCommand,
    clearCommand,
    selectAllCommand,
    EDIT_MENU_DUMMY_2,
    copySpecialCommand
};

/* sequence menu command indexes */

enum {
    endCommand = 1,
    SEQUENCE_MENU_DUMMY_1,
    resolveCommand,
    reconcileCommand,
    randomCommand,
    niceCommand,
    SEQUENCE_MENU_DUMMY_2,
    labelCommand,
    commentCommand,
    pictureCommand,
    SEQUENCE_MENU_DUMMY_3,
    anyConceptCommand,
    modifyCommand,
    SEQUENCE_MENU_DUMMY_4,
    showCoupleNumbersCommand
};

/* sd menu command indexes */

enum {
    saveCallListCommand = 1,
    SD_MENU_DUMMY_1,
    selectDatabaseCommand,
    compileDatabaseCommand,
    SD_MENU_DUMMY_2,
    editPreferencesCommand
};

/* geometry */

#define SBarWidth 16

/* special menu_match indexes */

enum {
    specialAccept = -2,
    specialSpace = -3,
    specialTab = -4,
    specialCancel = -5
};

/* special keys */

enum {
    enterKey        = 3,
    controlAKey     = 1,
    controlBKey     = 2,
    controlEKey     = 5,
    controlFKey     = 6,
    controlUKey     = 21,
    leftArrowKey    = 28,
    rightArrowKey   = 29,
    upArrowKey      = 30,
    downArrowKey    = 31
};

/* type definitions */

struct Window;
typedef struct Window Window;

typedef void (*ActivateMethod)(Window *wp, long_boolean activate);
typedef void (*Method)(Window *wp);
typedef void (*EventMethod)(Window *wp, EventRecord *e);
typedef void (*PointMethod)(Window *wp, Point p);
typedef void (*ResizeMethod)(Window *wp, int delta_v, int delta_h);
typedef void (*EditMethod)(Window *wp, int item);
typedef void (*KeyMethod)(Window *wp, short modifiers, char ch);
typedef void (*ControlMethod)(Window *twp, ControlHandle h, short part_code);

struct DialogWindow;
typedef struct DialogWindow DialogWindow;

struct TextWindow;
typedef struct TextWindow TextWindow;

enum WindowKind {
    ApplicationKind = 0,
    UtilityKind,
    ModalKind
};
typedef enum WindowKind WindowKind;

struct Window {
    ActivateMethod activateMethod;
    Method         updateMethod;
    PointMethod    growMethod;
    EventMethod    mouseDownMethod;
    Method         closeMethod;
    ResizeMethod   resizeSubMethod;
    EditMethod     editMethod;
    KeyMethod      keyMethod;
    Method         menuSetupMethod;
    ControlMethod  controlMethod;
    WindowPtr      window;
    WindowKind     kind;
    Window *       root_window; /* app window associated with utility window */
    TEHandle       TE;
};

struct DialogWindow {
    Window         base;
    int            item;
    int            default_item;
};

struct TextWindow {
    Window         base;
    ControlHandle  vScroll;
    long_boolean   fixed_height;
    int            pane_height;
    Rect           vScrollRect;
    Rect           viewRect;
};

/* macalert.c */

void sequence_write_error(const char *filename);
void database_bad_format(const char *filename);
void database_wrong_version(const char *filename, int actual, int desired);
void database_error(const char *message, const char *call_name);
void miscellaneous_error(const char *msg);
void memory_allocation_failure(unsigned int siz);
int  confirm_caution_alert(int alertID, const char *arg0);
int  confirm_note_alert(int alertID, const char *arg0);
void stop_alert(int alertID, const char *arg0, const char *arg1);
void error_alert(char *s, const char *f);

/* maccomp.c */

void do_exit(void);
void dbcompile_signoff(int bytes, int calls);
int do_printf(const char *fmt, ...);

extern short db_input;
extern short db_output;
extern char db_input_filename[];
extern char db_output_filename[];

/* macdlog.c */

void dialog_setup(DialogWindow *dwp, DialogPtr dp);
void dialog_activate(DialogWindow *dwp, long_boolean activate);
void dialog_mouse_down(DialogWindow *dwp, EventRecord *e);
void dialog_key(DialogWindow *dwp, short modifiers, char ch);
void dialog_update(DialogWindow *dwp);
void movable_modal_dialog(DialogWindow *dwp);
void radio_set(DialogPtr dp, int first, int last, int selected);
void dialog_set_control_value(DialogPtr dp, short index, short value);
short dialog_get_control_value(DialogPtr dp, short index);
Handle ditem(DialogPtr, int);
void draw_frame_around_button(ControlHandle control);
void setup_dialog_box(DialogPtr dp, int user_item);

/* macevent.c */

void process_event(EventRecord *e);
void process_next_event(void);
void selective_event_loop(short event_mask);
void selective_event(short event_mask);

/* maclabel.c */

void label_clear(void);
void label_get(char *dest);
void open_label_dialog(void);

/* macmatch.c */

void match_update(int which_menu);
void match_flash_validate(void);
void match_validate(void);
void match_activate(long_boolean activate);
void match_invalidate(void);

/* macmenu.c */

void init_menus(void);
void menu_command(long menu_result);
void adjust_menus(void);
void save_command(void);
void save_as_command(void);
void update_modification_state(int n);

extern MenuHandle apple_menu;
extern MenuHandle file_menu;
extern MenuHandle edit_menu;
extern MenuHandle sequence_menu;
extern MenuHandle sd_menu;

/* macpref.c */

extern dance_level       preference_default_level;
extern long_boolean      preference_use_graphics;
extern long_boolean      preference_use_multiple_colors;

void read_preferences_file(void);
void edit_preferences(void);

/* macrec.c */

void recpoint_begin(void);
void recpoint_add(const char *text);
void recpoint_end(void);
void recpoint_invalidate(void);
void recpoint_activate(long_boolean activate);

/* macsel.c */

int mac_do_selector_popup(void);
int mac_do_direction_popup(void);
int mac_do_quantifier_popup(void);

/* macstart.c */

int getcommand(char ***argvp);
int get_output_file_name(char *buf, FSSpec *fs, StringPtr prompt);
int get_input_file_name(char *buf, FSSpec *fs, long filetype);
void pstrcpy(StringPtr p1, const StringPtr p2);
void open_compile_dialog(void);
long_boolean select_call_database(void);
long_boolean startup_save_call_list(void);

/* macstuff.c */

void mac_initialize(void);
void input_clear(void);
void input_set_prompt(char *prompt, char *menu_name);
void input_error(int num_matches);
void input_extend(char *input);
int  get_user_input(char user_input[], match_result *menu_match);
void text_output_add_line(char the_line[]);
void text_output_trim(int n);
int  get_popup_string(char *prompt, char *dest);
int  get_startup_command(void);
uims_reply get_resolve_command(void);
int  mac_modifier_any_popup(char *callname);
int  mac_modifier_tag_popup(char *callname);
int  mac_modifier_scoot_popup(char *callname);
void database_begin(char *filename);
void create_controls(void);
void move_controls(int offset);
void activate_controls(long_boolean activate);
void flash_control(ControlHandle h, int part_code);
void output_draw_button_frames(void);

void set_recpoint_target(int n);
void input_stuff(char *s, int len);
void input_accept(void);
void stuff_command(uims_reply kind, int index);
void show_about_sd(void);
void show_hints(void);

void busy_cursor(void);
void arrow_cursor(void);
void adjust_cursor(void);

typedef enum {
    NoOutputMode,
    StartupMode,
    CallMode,
    ResolveMode,
    StringInputMode
} OutputMode;

extern char *level_name;
extern OutputMode output_mode;
extern long_boolean dirty;
extern long_boolean output_file_ok;
extern long_boolean writing_sequence;
extern ControlHandle output_default_control;

/* mactext.c */

void text_window_set_text(TextWindow *twp, const char *buffer, int len);
void text_window_resize(TextWindow *twp, int delta_v, int delta_h);
void text_window_mouse_down(TextWindow *twp, EventRecord *e);
int  text_window_standard_mouse_down(TextWindow *twp, EventRecord *e);
void text_window_adjust(TextWindow *twp);
void text_window_setup(TextWindow *twp, WindowPtr w, int multistyle);
void text_window_set_view(TextWindow *twp);
void text_window_activate(TextWindow *twp, long_boolean activate);
void text_window_update_vscroll_pos(TextWindow *twp);
void text_window_edit(TextWindow *twp, int item);
void text_window_scroll(TextWindow *twp, int delta);
void text_window_show_line(TextWindow *twp, int line);
int  text_window_select_line(TextWindow *twp, Point p);
void text_window_set_top_line(TextWindow *twp, int top);

void text_clear(TEHandle TE);
void text_append_line(TEHandle TE, const char *the_line);
void text_append(TEHandle TE, const char *text);
void text_contents(TEHandle TE, char *dest);
void text_trim(TEHandle TE, int n);
void text_activate(TEHandle TE, long_boolean activate);
void text_edit(TEHandle TE, int item);
void text_key(TEHandle TE, short modifiers, char ch);
long_boolean text_has_contents(TEHandle TE);
long_boolean text_has_selection(TEHandle TE);
void text_menu_setup(TEHandle TE, long_boolean read_only);
int text_selected_line(TEHandle TE);
int text_offset_to_line(TEHandle TE, int offset);
void backwards_character(TEHandle TE);
void forwards_character(TEHandle TE);

/* macutil.c */

extern long_boolean new_file_package_available;
extern long_boolean wait_next_event_available;
extern long_boolean find_folder_available;
extern long_boolean color_quickdraw_available;
extern long_boolean popup_control_available;
extern long_boolean fsspec_calls_available;

void init_environment(void);
WindowPtr get_new_window(short WindowID, void *storage, WindowPtr behind);
int pixel_bits(WindowPtr w);
OSErr mac_open_file(const FSSpec *fs, char permission, short *f);
OSErr mac_open_file_resources(const FSSpec *fs, char permission, short *f);
OSErr mac_fixup_file(short vol, const StringPtr name, FSSpec *fs);
OSErr mac_create_file(const FSSpec *fs, OSType creator, OSType file_type);
OSErr mac_delete_file(const FSSpec *fs);
OSErr mac_rename_file(const FSSpec *fs, const StringPtr newname);
OSErr mac_replace_file(const FSSpec *old, const FSSpec *new);

/* macwin.c */

void main_window_setup(void);
void main_window_new_preferences(void);

void activate_typescript(void);
void typescript_add_line(const char *text);
void typescript_trim(int n);
void typescript_flush(void);

void activate_input(void);
void typein_clear(void);
void typein_extend(const char *s);
void typein_insert_character(int ch);
void typein_get_contents(char *buffer);
void typein_stuff(const char *s, int len);
void typein_idle(void);
void typein_adjust(void);
void typein_set_prompt(const char *p);
void erase_input_area(void);
void restore_input_area(void);

void set_status(const char *p);
void set_menu_name(const char *p);

void main_window_edit(int item);

extern WindowPtr     myWindow;
extern TEHandle      TEH;
extern TextWindow    outputTW;
extern int           input_is_active;
extern int           input_is_up;
extern int           using_special_font;   /* using special square dancing font */
extern int           color_each_couple;    /* use different colors for each couple */
extern int           display_numbers;      /* display couple numbers in pictures */

/* macwmgr.c */

void window_manager_initialize(void);
void display_update(void);
Window *front_application_window(void);
Window *active_window(void);
Window *front_utility_window(void);
Window *back_utility_window(void);
long_boolean movable_modal_up(void);
void window_select(Window *wp);
void window_close(Window *wp);
void suspend_resume(long_boolean suspend);
long_boolean window_is_active(Window *wp);
void window_setup(Window *wp, WindowPtr w);
void window_activate(Window *wp, long_boolean activate);
void window_edit(Window *wp, int item);
void standard_edit_method(Window *wp, int item);
void window_mouse_down(Window *wp, EventRecord *e);
void standard_mouse_down_method(Window *wp, EventRecord *e);
void window_update(Window *wp);
void window_grow(Window *wp, Point p);
void standard_grow_method(Window *wp, const Point p);
void window_drag(Window *wp, Point p, short modifiers);
void window_key(Window *wp, short modifiers, char ch);
void standard_key_method(Window *wp, short modifiers, char ch);
void standard_activate_method(Window *wp, long_boolean activate);
short wmgr_caution_alert(short id, ModalFilterProcPtr proc);
short wmgr_note_alert(short id, ModalFilterProcPtr proc);
short wmgr_stop_alert(short id, ModalFilterProcPtr proc);
void deactivate_front_window(void);
void activate_front_window(void);
void window_global_portrect(WindowPtr w, Rect *rp);
int window_type(WindowPtr w);

/* sdsi-mac.c */

extern long_boolean file_error;

void set_call_list_file(int f);
void set_output_file(const FSSpec *fs);
void set_output_file_append(const FSSpec *fs);
long_boolean set_database_file(const char *fn, short f);

char *db_gets(char *s, int n);
void db_putc(char ch);
void db_rewind_output(int pos);
void db_close_input(void);
void db_close_output(void);
void db_cleanup(void);
void db_input_error(void);
void db_output_error(void);

/* sdui-mac.c */

extern call_list_kind uims_current_call_menu;

