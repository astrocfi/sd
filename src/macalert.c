/*
 *  macalert.c - alert boxes
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
#include <string.h> /* for strcpy */
#include <stdio.h> /* for sprintf */

/*
 *  sequence_write_error
 *
 */

void
sequence_write_error(const char *filename)
{
    stop_alert(SequenceWriteErrorAlert, filename, "");
}

/*
 *  database_bad_format
 *
 */

void
database_bad_format(const char *filename)
{
    stop_alert(DatabaseBadFormatAlert, filename, "");
}

/*
 *  database_wrong_version
 *
 */

void
database_wrong_version(const char *filename, int actual, int desired)
{
    char buf1[10], buf2[10];
    sprintf(buf1, "%d", actual);
    sprintf(buf2, "%d", desired);
    stop_alert(DatabaseWrongVersionAlert, buf1, buf2);
}

/*
 *  database_error
 *
 */

void
database_error(const char *message, const char *call_name)
{
    if (call_name != NULL) {
        stop_alert(DatabaseErrorCallAlert, message, call_name);
    }
    else {
        stop_alert(DatabaseErrorAlert, message, "");
    }
}

/*
 *  miscellaneous_error
 *
 */

void
miscellaneous_error(const char *msg)
{
    stop_alert(ErrorAlert, msg, "");
}

/*
 *  memory_allocation_failure
 *
 */

void
memory_allocation_failure(unsigned int siz)
{
    char buf[100];

    sprintf(buf, "Out of memory. Unable to allocate %u bytes.", siz);
    miscellaneous_error(buf);
    exit_program(2);
}

/*
 *  confirm_caution_alert
 *
 */

int
confirm_caution_alert(int alertID, const char *arg0)
{
    int myAlertItem;
    char buf[200];

    strcpy(buf, arg0);
    ParamText(CtoPstr(buf),"\p","\p","\p");
    myAlertItem = wmgr_caution_alert(alertID, 0L);
    if (myAlertItem == 1) {
        return POPUP_ACCEPT;
    }
    return POPUP_DECLINE;
}

/*
 *  confirm_note_alert
 *
 */

int
confirm_note_alert(int alertID, const char *arg0)
{
    int myAlertItem;
    char buf[200];

    strcpy(buf, arg0);
    ParamText(CtoPstr(buf),"\p","\p","\p");
    myAlertItem = wmgr_note_alert(alertID, 0L);
    if (myAlertItem == 1) {
        return POPUP_ACCEPT;
    }
    return POPUP_DECLINE;
}

/*
 *  stop_alert
 *
 */

void
stop_alert(int alertID, const char *arg0, const char *arg1)
{
    Str255 buf0, buf1;

    strcpy((char *) buf0, arg0);
    strcpy((char *) buf1, arg1);
    ParamText(CtoPstr((char *) buf0), CtoPstr((char *) buf1), "\p", "\p");
    wmgr_stop_alert(alertID, 0L);
}
