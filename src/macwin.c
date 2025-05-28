/*
 *  macwin.c - support for main window layout
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
#include <string.h> /* for strlen, strcpy */

WindowPtr           myWindow;
TEHandle            TEH;                  /* sequence display */
TextWindow          outputTW;
TEHandle            ITE;                  /* call input */
int                 input_is_active;      /* alternative: typescript is active */
int                 input_is_up;          /* the type-in area is up */
int                 using_special_font;   /* using special square dancing font */
int                 color_each_couple;    /* use different colors for each couple */
int                 display_numbers = TRUE;    /* display couple numbers in pictures */

static TEHandle     promptTE;

#if 0
static char         typescript[30000];    /* current contents of typescript buffer */
static int          typecount;            /* number of characters in typescript */
#endif

static char         menu_name[100];
static char         status[100];

static int          RightOffset = -1;
static int          BottomOffset = 95;
static int          LeftOffset = -1;
static int          TopOffset = -1;

static Rect         BorderRect;
static Rect         InputBorderRect;
static Rect         InputViewRect;
static Rect         PromptRect;
static Rect         ControlRect;
static Rect         LevelRect;
static Rect         SeparatorRect1;
static Rect         MenuRect;
static Rect         SeparatorRect2;
static Rect         StatusRect;

static TextStyle    typeout_style;
static TextStyle    picture_style;
static TextStyle    blank_line_style;
static TextStyle    couple_style[4];

static int          current_style;

static RGBColor     typeout_color = {0, 0, 0};
static RGBColor     picture_color = {0, 0, 65535};
static RGBColor     couple_color[4] = {
                        {    0,     0, 65535},
                        {    0, 65535,     0},
                        {65535,     0,     0},
                        {65535,     0, 65535}};

enum {TS_TYPEOUT, TS_PICTURE, TS_BLANK_LINE, TS_UNKNOWN};

static void compute_rects(TextWindow *twp);
static void setup_styles(void);
static void resize_window(TextWindow *twp, int delta_v, int delta_h);
static void update_window(TextWindow *twp);
static void activate_window(TextWindow *twp, long_boolean activate);
static void mouse_down(TextWindow *twp, EventRecord *e);
static void key_method(TextWindow *twp, short modifiers, char ch);
static void control_method(TextWindow *twp, ControlHandle h, short part_code);
static void edit_command(TextWindow *twp, int item);
static void menu_setup(TextWindow *twp);
static void draw_borders(void);
static void fixup_window_position(void);
static void set_view(void);
static void write_text(const char *p, const Rect *r);
static void update_grow_icon(TextWindow *twp);
static void typescript_set_style(int new_style);
static int  text_line_style(const char *text);
static void copy_decode(TEHandle TE);
static int  is_encoded_picture_line(TEHandle TE, int line);
static void decode_picture_line(TEHandle TE, int line);
static char *encode_picture_text(const char *text, char *buffer, int multicolored);
static char encode_person(int number, int gender, int direction);

/*
 **********************************************************************
 *  PUBLIC FUNCTIONS
 **********************************************************************
 */

void
main_window_setup(void)
{
    static WindowRecord wr;

    myWindow = get_new_window(OutputWindow, &wr, (WindowPtr) -1L);
    outputTW.base.window = myWindow; /* needed for compute_rects */
    SetPort(myWindow);
    fixup_window_position();
    compute_rects(&outputTW);

    text_window_setup(&outputTW, myWindow, TRUE);
    outputTW.base.activateMethod  = (ActivateMethod) activate_window;
    outputTW.base.mouseDownMethod = (EventMethod)    mouse_down;
    outputTW.base.resizeSubMethod = (ResizeMethod)   resize_window;
    outputTW.base.editMethod      = (EditMethod)     edit_command;
    outputTW.base.menuSetupMethod = (Method)         menu_setup;
    outputTW.base.keyMethod       = (KeyMethod)      key_method;
    outputTW.base.updateMethod    = (Method)         update_window;
    outputTW.base.controlMethod   = (ControlMethod)  control_method;

    TEH = outputTW.base.TE;
    setup_styles();
    TextFont(0);
    TextSize(12);
    ITE = TENew(&InputViewRect, &InputViewRect);
    promptTE = TENew(&PromptRect, &PromptRect);
    input_is_up = TRUE;
    create_controls();
    move_controls(myWindow->portRect.bottom - BottomOffset);
    set_view();
    TEAutoView(true, ITE);
}

/*
 *  main_window_new_preferences
 *
 *  React to changes in the user preferences.
 *
 */

void
main_window_new_preferences(void)
{
    int old_using_special_font = using_special_font;
    int old_color_each_couple = color_each_couple;

    if (myWindow != NULL) {
        setup_styles();
        if ((using_special_font != old_using_special_font) ||
            (color_each_couple != old_color_each_couple)) {
            /* refresh display */
            if ((output_mode == CallMode) || (output_mode == ResolveMode)) {
                stuff_command(ui_command_select, command_refresh);
            }
            /* "show couple numbers" menu command might enable/disable */
            adjust_menus();
        }
    }
}

/*
 **********************************************************************
 *  TYPESCRIPT
 **********************************************************************
 */

/*
 *  activate_typescript
 *
 *  Make the typescript text area the one that menu commands apply to
 *  and the one that highlights the selected region.
 *
 */

void
activate_typescript(void)
{
    TEDeactivate(ITE);
    TEActivate(TEH);
    input_is_active = FALSE;
}

/*
 *  typescript_add_line
 *
 *  Add the string TEXT to the end of the typescript area.
 *
 */

void
typescript_add_line(const char *text)
{
    char buffer[200];
    int len;

    if (TEH == NULL) {
        return;
    }

    typescript_set_style(text_line_style(text));
    if (color_each_couple && using_special_font && (current_style == TS_PICTURE)) {
        encode_picture_text(text, buffer, TRUE);
    }
    else {
        if ((current_style == TS_PICTURE) &&
                (text[0] != '\0') &&
                using_special_font) {
            text = encode_picture_text(text, buffer, FALSE);
        }
        len = strlen(text);
        TEInsert(text, len, TEH);
    }
    TEInsert("\r", 1, TEH); /* don't use TEKey, it hides sprite */

#if 0
    memcpy(&typescript[typecount], text, len);
    typecount += len;
    typescript[typecount++] = '\r';
#endif

}

void
typescript_trim(int n)
{
    register int i;

    current_style = TS_UNKNOWN;
    if (TEH) {
        TESetSelect((**TEH).lineStarts[n], (**TEH).teLength, TEH);
        TEDelete(TEH);
    }
    
#if 0
    i = 0;
    while (n > 0) {
        while ((i < typecount) && typescript[i] != '\r') ++i;
        ++i;
        --n;
    }
    typecount = i;
#endif

}

void
typescript_flush(void)
{
    if (TEH) {
        text_window_adjust(&outputTW);
        
#if 0
        TESetText(typescript, typecount, TEH);
        SetPort(myWindow);
        EraseRect(&(**TEH).viewRect);
        TEUpdate(&(**TEH).viewRect, TEH);
#endif

    }
}

/*
 **********************************************************************
 *  USER KEYBOARD INPUT (TYPEIN)
 **********************************************************************
 */

/*
 *  activate_input
 *
 *  Make the input text area the one that menu commands apply to
 *  and the one that highlights the selected region.
 *
 */

void
activate_input(void)
{
    TEDeactivate(TEH);
    TEActivate(ITE);
    input_is_active = TRUE;
}

/*
 *  typein_clear
 *
 *  Clear the user input buffer.
 *
 */

void
typein_clear(void)
{
    text_clear(ITE);
}

/*
 *  typein_extend
 *
 *  Append the string S to the user input buffer.
 *
 */

void
typein_extend(const char *s)
{
    text_append(ITE, s);
}

/*
 *  typein_insert_character
 *
 *  Insert the character CH into the user input buffer at the
 *  current location.
 *
 */

void
typein_insert_character(int ch)
{
    TEKey(ch, ITE);
}

/*
 *  typein_get_contents
 *
 *  Copy the contents of the user input buffer into BUFFER.
 *
 */

void
typein_get_contents(char *buffer)
{
    text_contents(ITE, buffer);
}

/*
 *  typein_stuff
 *
 *  Replace the contents of the user input buffer with the
 *  string S of length LEN.
 *
 */

void
typein_stuff(const char *s, int len)
{
    TESetText(s, len, ITE);
    SetPort(myWindow);
    InvalRect(&(**ITE).viewRect);
    window_update((Window *)&outputTW);
}

/*
 *  typein_idle
 *
 *  Call this function periodically to make the input cursor
 *  go on and off.
 *
 */

void
typein_idle(void)
{
    TEIdle(ITE);
}

/*
 *  typein_adjust
 *
 *  Scroll as needed to ensure that the input cursor is visible.
 *
 */

void
typein_adjust(void)
{
    TESelView(ITE);
}

/*
 *  typein_set_prompt
 *
 *  Change the prompt string associated with the user input buffer.
 *
 */

void
typein_set_prompt(const char *p)
{
    (**promptTE).selStart = 0;
    (**promptTE).selEnd = (**promptTE).teLength;
    TEDelete(promptTE);
    TEInsert(p, strlen(p), promptTE);
}

/*
 *  erase_input_area
 *
 *  Hide the user input text box.
 *
 */

void
erase_input_area()
{
    if (input_is_up) {
        (**ITE).viewRect.left += 8000;
    }
    SetPort(myWindow);
    EraseRect(&ControlRect);
    input_is_up = FALSE;
}

/*
 *  restore_input_area
 *
 *  Ensure that the user input text box is displayed.
 *
 */

void
restore_input_area()
{
    if (!input_is_up) {
        (**ITE).viewRect.left -= 8000;
    }
    input_is_up = TRUE;
    SetPort(myWindow);
    PenSize(1,1);
    FrameRect(&InputBorderRect);
    TEUpdate(&myWindow->portRect, promptTE);
    TEUpdate(&myWindow->portRect, ITE);
}

/*
 **********************************************************************
 *  MISCELLANEOUS DISPLAY ATTRIBUTES
 **********************************************************************
 */

/*
 *  set_status
 *
 *  Set the contents of the status indicator.
 *
 */

void
set_status(const char *p)
{
    strcpy(status, p);
    write_text(status, &StatusRect);
}

/*
 *  set_menu_name
 *
 */

void
set_menu_name(const char *p)
{
    strcpy(menu_name, p);
    write_text(menu_name, &MenuRect);
}

/*
 **********************************************************************
 *  SUPPORT
 **********************************************************************
 */

static void
compute_rects(TextWindow *twp)
{
    Rect r;
    int margin;

    r = twp->base.window->portRect;

    twp->vScrollRect = r;
    twp->vScrollRect.right -= RightOffset;
    twp->vScrollRect.bottom -= BottomOffset;
    twp->vScrollRect.top += TopOffset;
    twp->vScrollRect.left = twp->vScrollRect.right-SBarWidth;

    BorderRect = r;
    BorderRect.top += TopOffset;
    BorderRect.left += LeftOffset;
    BorderRect.right -= RightOffset;
    BorderRect.bottom -= BottomOffset;

    twp->viewRect = BorderRect;
    twp->viewRect.right -= SBarWidth;
    InsetRect(&twp->viewRect, 4, 4);
    
    ControlRect = r;
    ControlRect.top = ControlRect.bottom - BottomOffset;
    ControlRect.bottom -= SBarWidth;
    
    InputBorderRect = r;
    InputBorderRect.left += 13;
    InputBorderRect.right -= 13;
    InputBorderRect.top = BorderRect.bottom + 45;
    InputBorderRect.bottom = InputBorderRect.top + 22;
    
    InputViewRect = InputBorderRect;
    InsetRect(&InputViewRect, 3, 3);
    
    PromptRect = InputBorderRect;
    PromptRect.top = BorderRect.bottom + 15;
    PromptRect.bottom = InputBorderRect.bottom-2;
    PromptRect.right = 167;

    margin = r.right - SBarWidth + 1;

    LevelRect = r;
    LevelRect.top = LevelRect.bottom - SBarWidth + 3;
    LevelRect.right = LevelRect.left + 80;
    if (LevelRect.right >= margin)
        LevelRect.right = margin;

    SeparatorRect1 = LevelRect;
    SeparatorRect1.top = r.bottom - (SBarWidth - 1);
    SeparatorRect1.left = LevelRect.right;
    SeparatorRect1.right = SeparatorRect1.left + 1;

    MenuRect = LevelRect;
    MenuRect.left = SeparatorRect1.right;
    MenuRect.right = MenuRect.left + 120;
    if (MenuRect.right >= margin)
        MenuRect.right = margin;
    
    SeparatorRect2 = SeparatorRect1;
    SeparatorRect2.left = MenuRect.right;
    SeparatorRect2.right = SeparatorRect2.left + 1;

    StatusRect = LevelRect;
    StatusRect.left = SeparatorRect2.right;
    StatusRect.right = margin;
}

static void
fixup_window_position(void)
{
    Rect       winpos;
    int        frame = 2; /* width of window frame */
    int        bottom;

    window_global_portrect(myWindow, &winpos);
    bottom = screenBits.bounds.bottom - frame;
    SizeWindow(myWindow, winpos.right-winpos.left, bottom-winpos.top, true);
}

static void
setup_styles(void)
{
    short font_id;
    int i;

    current_style = TS_UNKNOWN;
    typeout_style.tsFont = geneva;
    typeout_style.tsSize = 10;
    typeout_style.tsColor = typeout_color;
    picture_style.tsFont = monaco;
    picture_style.tsSize = 9;
    picture_style.tsColor = picture_color;
    blank_line_style.tsFont = monaco;
    blank_line_style.tsSize = 4;
    blank_line_style.tsColor = typeout_color;

    using_special_font = FALSE;
    if (preference_use_graphics) {
        GetFNum("\pSquare Dancing", &font_id);
        if (font_id != 0) {
            picture_style.tsFont = font_id;
            picture_style.tsSize = 14;
            using_special_font = TRUE;
        }
    }
    for (i=0;i<3;i++) {
        couple_style[i] = picture_style;
        couple_style[i].tsColor = couple_color[i];
    }
    color_each_couple = preference_use_multiple_colors &&
                        (pixel_bits(myWindow) >= 3);
}

static int
text_line_style(const char *text)
{
    int n;

    if ((text[0] == '\0') && using_special_font) {
        return TS_BLANK_LINE;
    }
    if (text[0] != ' ') {
        return TS_TYPEOUT;
    }
    n = strspn(text, " 1234BG^V<>?");
    if (text[n] == '\0') {
        return TS_PICTURE;
    }
    return TS_TYPEOUT;
}

static void
typescript_set_style(int new_style)
{
    if (new_style == current_style) {
        return;
    }
    else if (new_style == TS_BLANK_LINE) {
        current_style = new_style;
        TESetStyle(doFont+doSize+doColor, &blank_line_style, false, TEH);
    }
    else if (new_style == TS_TYPEOUT) {
        current_style = new_style;
        TESetStyle(doFont+doSize+doColor, &typeout_style, false, TEH);
    }
    else {
        current_style = new_style;
        TESetStyle(doFont+doSize+doColor, &picture_style, false, TEH);
    }
}

static void
write_text(const char *p, const Rect *r)
{
    SetPort(myWindow);
    TextFont(geneva);
    TextSize(10);
    TextBox(p, strlen(p), r, teJustCenter);
}

static void
set_view(void)
{
    (**ITE).viewRect = InputViewRect;
    if (!input_is_up) {
        (**ITE).viewRect.left += 8000;
    }
    (**ITE).destRect = InputViewRect;
    (**ITE).destRect.right = 20000; /* no right edge */
    (**promptTE).viewRect = PromptRect;
    (**promptTE).destRect = PromptRect;
}

static void
draw_borders(void)
{
    PenSize(1,1);
    FrameRect(&BorderRect);
    FrameRect(&SeparatorRect1);
    FrameRect(&SeparatorRect2);
    if (input_is_up) {
        FrameRect(&InputBorderRect);
    }
}

/*
 *  update_grow_icon
 *
 *  Like DrawGrowIcon, but avoids drawing the vertical scroll bar line.
 *
 */

static void
update_grow_icon(TextWindow *twp)
{
    RgnHandle old_clip;
    Rect r;
    WindowPtr w;

    w = twp->base.window;
    SetPort(w);
    old_clip = NewRgn();
    GetClip(old_clip);
    r = w->portRect;
    r.top = r.bottom - SBarWidth + 1;
    ClipRect(&r);
    DrawGrowIcon(w); /* erases grow icon if "inactive" */
    SetClip(old_clip);
    DisposeRgn(old_clip);
    FrameRect(&twp->vScrollRect);
}

static void
update_window(TextWindow *twp)
{
    WindowPtr w = twp->base.window;
    EraseRect(&w->portRect);
    update_grow_icon(twp);
    draw_borders();
    UpdateControls(w, w->visRgn);
    output_draw_button_frames();
    TEUpdate(&w->portRect, TEH);
    if ((**promptTE).teLength > 0) {
        TEUpdate(&w->portRect, promptTE);
    }
    TextFont(geneva);
    TextSize(10);
    TextBox(level_name, strlen(level_name), &LevelRect,  teJustCenter);
    TextBox(menu_name,  strlen(menu_name),  &MenuRect,   teJustCenter);
    TextBox(status,     strlen(status),     &StatusRect, teJustCenter);
    if (input_is_up) {
        TEUpdate(&w->portRect, ITE);
    }
}

static void
resize_window(TextWindow *twp, int delta_v, int delta_h)
{
    compute_rects(twp);
    text_window_set_view(twp);
    set_view();
    typein_adjust();
    move_controls(delta_v);
    text_window_resize(twp, delta_v, delta_h);
}

static void
activate_window(TextWindow *twp, long_boolean activate)
{
    HiliteWindow(twp->base.window, activate ? true : false);
    SetPort(twp->base.window);
    activate_controls(activate);
    output_draw_button_frames();
    update_grow_icon(twp);
    if (activate) {
        TEActivate(input_is_active ? ITE : TEH);
        ShowControl(twp->vScroll);
    }
    else {
        TEDeactivate(input_is_active ? ITE : TEH);
        HideControl(twp->vScroll);
    }
    match_activate(activate);
    recpoint_activate(activate);
}

static void
key_method(TextWindow *twp, short modifiers, char ch)
{
    if (output_mode == CallMode) {
        activate_input();
        if (ch == ' ') {
            stuff_command(-1, specialSpace);
            return;
        }
        if (ch == '\t') {
            stuff_command(-1, specialTab);
            return;
        }
        if (ch == '?') {
            match_flash_validate();
            match_validate();
            return;
        }
    }
    else if (output_mode == StringInputMode) {
        activate_input();
    }
    if ((ch == '\r') || (ch == enterKey)) {
        switch (output_mode) {
          case ResolveMode:
            stuff_command(ui_resolve_select, GetCRefCon(output_default_control));
            flash_control(output_default_control, inButton);
            return;
          case StartupMode:
            stuff_command(ui_start_select, GetCRefCon(output_default_control));
            flash_control(output_default_control, inButton);
            return;
          case CallMode:
          case StringInputMode:
            stuff_command(-1, specialAccept);
            flash_control(output_default_control, inButton);
            return;
        }    
    }
    if (input_is_up) {
        text_key(ITE, modifiers, ch);
    }
    else {
        SysBeep(30);
    }
}

static void
mouse_down(TextWindow *twp, EventRecord *e)
{
    if (PtInRect(e->where, &(**ITE).viewRect)) {
        activate_input();
        TEClick(e->where, (e->modifiers & shiftKey)!=0, ITE);
    }
    else if (PtInRect(e->where, &(**TEH).viewRect)) {
        activate_typescript();
        TEClick(e->where, (e->modifiers & shiftKey)!=0, TEH);
    }
    else {
        text_window_standard_mouse_down(twp, e);
    }
}

static void
control_method(TextWindow *twp, ControlHandle h, short part_code)
{
    switch (output_mode) {
      case ResolveMode:
        stuff_command(ui_resolve_select, GetCRefCon(h));
        break;
      case StartupMode:
        stuff_command(ui_start_select, GetCRefCon(h));
        break;
      case StringInputMode:
      case CallMode:
        stuff_command(-1, GetCRefCon(h));
        break;
    }
}

void
main_window_edit(int item)
{
    edit_command(&outputTW, item);
}

static void
edit_command(TextWindow *twp, int item)
{
    if (item == undoCommand) {
        stuff_command(ui_command_select, command_undo);
        return;
    }
    if (input_is_active) {
        text_edit(ITE, item);
        if (item == pasteCommand) {
            /* ensure input buffer has only one line */
            if ((**ITE).nLines > 1) {
                (**ITE).selStart = (**ITE).lineStarts[1]-1;
                (**ITE).selEnd   = (**ITE).teLength;
                TEDelete(ITE);
            }
        }
    }
    else {
        if (item == copyCommand) {
            copy_decode(TEH);
        }
        else if (item == copySpecialCommand) {
            text_edit(TEH, copyCommand);
        }
        else if (item != idleCommand) {
            text_edit(TEH, item);
        }
    }
}

static void
menu_setup(TextWindow *twp)
{
    unsigned char buf[100];
/* sue: changed int->void to match changes in version 29.43+ */
    void *perm_map;
    long_boolean accept_extend;

    EnableItem(apple_menu, hintsCommand);
    EnableItem(sd_menu, 0);
    EnableItem(sd_menu, editPreferencesCommand);
    EnableItem(sd_menu, compileDatabaseCommand);
    EnableItem(sd_menu, saveCallListCommand);

    if (output_mode == StartupMode) {
        return;
    }

    if (output_mode == ResolveMode) {
        if (using_special_font) {
            EnableItem(sequence_menu, 0);
            DrawMenuBar();
            EnableItem(sequence_menu, showCoupleNumbersCommand);
        }
        return;
    }

    if (output_mode == CallMode) {
        EnableItem(sequence_menu, 0);
        DrawMenuBar();
        GetIndString(buf, MessagesStrings,
            concepts_in_place() ? UndoConceptString : UndoCallString);
        SetItem(edit_menu, undoCommand, buf);
        
        EnableItem(file_menu, saveCommand);
        EnableItem(file_menu, saveasCommand);
        EnableItem(file_menu, appendFileCommand);
        /* sue: added */
        EnableItem(file_menu, appendasFileCommand);

        EnableItem(sequence_menu, endCommand);
        if (resolve_command_ok()) {
            EnableItem(sequence_menu, resolveCommand);
        }
        if (reconcile_command_ok(&perm_map, &accept_extend)) {
            EnableItem(sequence_menu, reconcileCommand);
        }
        EnableItem(sequence_menu, randomCommand);
        if (nice_setup_command_ok()) {
            EnableItem(sequence_menu, niceCommand);
        }
        EnableItem(sequence_menu, labelCommand);
        EnableItem(sequence_menu, commentCommand);
        EnableItem(sequence_menu, pictureCommand);
        EnableItem(sequence_menu, anyConceptCommand);
        EnableItem(sequence_menu, modifyCommand);
        if (using_special_font) {
            EnableItem(sequence_menu, showCoupleNumbersCommand);
        }

        EnableItem(edit_menu, undoCommand);
    }

    if (input_is_up && input_is_active) {
        text_menu_setup(ITE, FALSE);
    }

    if (!input_is_active) {
        text_menu_setup(TEH, TRUE);
        if (using_special_font && text_has_selection(TEH)) {
            EnableItem(edit_menu, copySpecialCommand);
        }
    }
}

/*
 *  copy_decode
 *
 *  Do the equivalent of TECopy(TE), except decoding any encoded pictures
 *  in the selected text.
 *
 */

static void
copy_decode(TEHandle TE)
{
    TEHandle selection;
    Rect r;
    int start, end, index, i;

    start = (**TE).selStart;
    end = (**TE).selEnd;

    if (start == end) {
        return;
    }

    if (ZeroScrap() != 0) {
        return;
    }

    SetRect(&r, 0, 0, 0, 0);
    selection = TENew(&(**TE).destRect, &r);
    TECopy(TE);
    TEPaste(selection);
    index = text_offset_to_line(TE, start);
    for (i=0; i<(**selection).nLines; i++, index++) {
        if (is_encoded_picture_line(TE, index)) {
            decode_picture_line(selection, i);
        }
    }
    TESetSelect(0, (**selection).teLength, selection);
    TECopy(selection);
    TEDispose(selection);
}

/*
 *  is_encoded_picture_line
 *
 *  This function returns TRUE if line TE[line] is an encoded picture line.
 *
 */

static int
is_encoded_picture_line(TEHandle TE, int line)
{
    int count, i;
    char *p;

    p = &(**TEGetText(TE))[0] + (**TE).lineStarts[line];
    count = (**TE).lineStarts[line+1] - (**TE).lineStarts[line];
    if (p[count-1] == '\r') {
        --count;
    }
    if ((count < 4) || (p[0] != ' ') || (p[1] != ' ') || (p[2] != ' ')) {
        return FALSE;
    }
    i = strspn(p, "`abcdefghijklmnopqrstuvwxyz{|}~/@ABCDEFG 12345678[]");
    if (i == count) {
        return TRUE;
    }
    return FALSE;
}

/*
 *  decode_picture_line
 *
 *  This function decodes line TE[line] in place.  The selection is changed.
 *
 */

static void
decode_picture_line(TEHandle TE, int line)
{
    int start, end, count, c, direction, number, gender;
    char buffer[200], *p, *q, *pnext;
    static char directions[4] = {'>', '<', '^', 'V'};

    start = (**TE).lineStarts[line];
    end = (**TE).lineStarts[line+1];
    p = &(**TEGetText(TE))[0];
    if (p[end-1] == '\r') {
        --end;
    }
    pnext = &p[end];
    p = &p[start];
    q = buffer;
    while (p < pnext) {
        c = *p++;
        if (c == ' ') {
            *q++ = ' ';
            continue;
        }
        direction = -1;
        number = -1;
        if (c == '[') {
            gender = 0;
            }
        else if (c == ']') {
            gender = 1;
        }
        else if ((c >= '@') && (c <= 'G')) {
            gender = c & 1;
            number = (c >> 1) & 3;
        }
        else if ((c >= '1') && (c <= '8')) {
            c = c - '1';
            gender = (c >> 2) & 1;
            direction = c & 3;
        }
        else {
            if (c == '/') {
                c = 0177;
            }
            number = (c >> 3) & 3;
            gender = (c >> 2) & 1;
            direction = c & 3;
        }
        if (number >= 0) {
            *q++ = '1' + number;
        }
        else {
            *q++ = ' ';
        }
        *q++ = (gender != 0) ? 'G' : 'B';
        *q++ = (direction >= 0) ? directions[direction] : '?';
    }
    TESetSelect(start, end, TE);
    TEDelete(TE);
    TEInsert(buffer, q-buffer, TE);
}
        
/*
 *  If MULTICOLORED, then write text directly to the text buffer.
 *
 */

static char *
encode_picture_text(const char *text, char *buffer, int multicolored)
{
    char *d, ch, encoded;
    int direction, gender, number;
    
    d = buffer;
    *d++ = ' ';
    *d++ = ' ';
    *d++ = ' ';
    if (multicolored) {
        TEInsert("   ", 3, TEH);
    }

    for (;;) {
        ch = *text++;
        if (ch == '\0') {
            *d = ch;
            break;
        }
        if ((ch >= '1') && (ch <= '4')) {
            number = ch - '1';
            ch = *text++;
            if (ch == '\0') {
                *d = ch;
                break;
            }
            gender = (ch == 'G');
            ch = *text++;
            if (ch == '\0') {
                *d = ch;
                break;
            }
            switch (ch) {
              case '^':
                direction = 2; break;
              case 'V':
                direction = 3; break;
              case '<':
                direction = 1; break;
              case '>':
                direction = 0; break;
              default:
                direction = -1;
            }
            *d++ = ch = encode_person(number, gender, direction);
            if (multicolored) {
                TESetStyle(doColor, &couple_style[number], true, TEH);
                TEInsert(d-1, 1, TEH); /* don't use TEKey, it hides sprite */
            }
        }
        else {
            *d++ = ch;
            if (multicolored) {
                TEInsert(d-1, 1, TEH); /* don't use TEKey, it hides sprite */
            }
        }
    }
    return buffer;
}

/*
 *  encode_person
 *
 *  This function selects a character code from the special font to
 *  represent a person based on NUMBER (0-3), GENDER (0=Boy, 1=Girl),
 *  and direction (0=Right, 1=Left, 2=Up, 3=Down, -1=Unknown).
 *
 */

static char
encode_person(int number, int gender, int direction)
{
    char encoded;

    if (display_numbers) {
        if (direction >= 0) {
            encoded = '`' + (number << 3) + (gender << 2) + direction;
            if (encoded == 0177) {
                encoded = '/';
            }
        }
        else {
            encoded = '@' + (number << 1) + gender;
        }
    }
    else {
        if (direction >= 0) {
            encoded = '1' + (gender << 2) + direction;
        }
        else {
            encoded = gender ? ']' : '[';
        }
    }
    return encoded;
}
