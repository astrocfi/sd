/*
 *  macutil.c - random Macintosh utilities
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
#include <Traps.h>
#include <GestaltEqu.h>
/* #include <Script.h> -- Think-C cannot swallow this */
#include <stdio.h> /* sprintf */

#define _Gestalt 0xA1AD

/* public flags */

long_boolean new_file_package_available = FALSE;
long_boolean wait_next_event_available = FALSE;
long_boolean find_folder_available = FALSE;
long_boolean color_quickdraw_available = FALSE;
long_boolean popup_control_available = FALSE;
long_boolean fsspec_calls_available = FALSE;

/* static flags */

static long_boolean gestalt_available = FALSE;
static long_boolean simulate_old_system = FALSE; /* set TRUE for debugging */

/* static function declarations */

static long_boolean compute_find_folder_available(void);
static long_boolean compute_new_file_package_available(void);
static long_boolean compute_color_quickdraw_available(void);
static long_boolean compute_popup_control_available(void);
static long_boolean compute_fsspec_calls_available(void);
static int trap_available(short the_trap);

/*
 **********************************************************************
 * PUBLIC FUNCTIONS
 **********************************************************************
 */

/*
 *  init_environment
 *
 */

void
init_environment(void)
{
    if (!simulate_old_system) {
        gestalt_available = trap_available(_Gestalt);
        wait_next_event_available = trap_available(_WaitNextEvent);
        new_file_package_available = compute_new_file_package_available();
        color_quickdraw_available = compute_color_quickdraw_available();
        find_folder_available = compute_find_folder_available();
        popup_control_available = compute_popup_control_available();
        fsspec_calls_available = compute_fsspec_calls_available();
    }
}

/*
 *  get_new_window
 *
 *  This function creates the appropriate kind of window, based
 *  on the availability of Color Quickdraw.
 *
 */

WindowPtr
get_new_window(short WindowID, void *storage, WindowPtr behind)
{
    if (color_quickdraw_available) {
        return GetNewCWindow(WindowID, storage, behind);
    }
    else {
        return GetNewWindow(WindowID, storage, behind);
    }
}

/*
 *  pixel_bits
 *
 *  This function returns the number of bits per pixel for the specified
 *  window.
 *
 */

int
pixel_bits(WindowPtr w)
{
    short version;

    if (!color_quickdraw_available) {
        return 1;
    }
    version = ((CGrafPtr)w)->portVersion;
    if (((version >> 14) & 3) == 3) {
        /* we have a color port */
        return (**((CGrafPtr)w)->portPixMap).pixelSize;
        }
    return 1; /* we have a monochrome port */
}

/*
 *  mac_fixup_file
 *
 *  This function converts old_style working directory to modern
 *  file specification.
 *
 */

OSErr
mac_fixup_file(short vol, const StringPtr name, FSSpec *fs)
{
    OSErr result;
    WDPBRec parm;
    parm.ioCompletion = NULL;
    parm.ioNamePtr = NULL;
    parm.ioVRefNum = vol;
    parm.ioWDIndex = 0;
    parm.ioWDProcID = 0;
    parm.ioWDVRefNum = 0;
    result = PBGetWDInfo(&parm, false);
    if (result == 0) {
        fs->vRefNum = parm.ioWDVRefNum;
        fs->parID = parm.ioWDDirID;
        pstrcpy(fs->name, name);
        PBCloseWD(&parm, false);
    }
    return result;
}

/*
 *  mac_create_file
 *
 */

OSErr
mac_create_file(const FSSpec *fs, OSType creator, OSType file_type)
{
    if (fsspec_calls_available) {
        return FSpCreate(fs, creator, file_type, -1 /* smSystemScript */);
    }
    else {
        static HParamBlockRec parm;
        OSErr result;
        parm.ioParam.ioCompletion = NULL;
        parm.ioParam.ioNamePtr = (StringPtr)&fs->name;
        parm.ioParam.ioVRefNum = fs->vRefNum;
        parm.fileParam.ioDirID = fs->parID;
        result = PBHCreate(&parm, false);
        if (result == 0) {
            static HFileInfo FIparm;
            FIparm.ioCompletion = NULL;
            FIparm.ioNamePtr = (StringPtr)&fs->name;
            FIparm.ioVRefNum = fs->vRefNum;
            FIparm.ioDirID = fs->parID;
            FIparm.ioFDirIndex = 0;
            result = PBHGetFInfo((HParmBlkPtr)&FIparm, false);
            if (result == 0) {
                FIparm.ioDirID = fs->parID;
                FIparm.ioFlFndrInfo.fdType = file_type;
                FIparm.ioFlFndrInfo.fdCreator = creator;
                result = PBHSetFInfo((HParmBlkPtr)&FIparm, false);
            }
        }
        return result;
    }
}

/*
 *  mac_delete_file
 *
 */

OSErr
mac_delete_file(const FSSpec *fs)
{
    if (fsspec_calls_available) {
        return FSpDelete(fs);
    }
    else {
        static HParamBlockRec parm;
        OSErr result;

        parm.ioParam.ioCompletion = NULL;
        parm.ioParam.ioNamePtr = (StringPtr)&fs->name;
        parm.ioParam.ioVRefNum = fs->vRefNum;
        parm.fileParam.ioDirID = fs->parID;
        result = PBHDelete(&parm, false);
        return result;
    }
}

/*
 *  mac_open_file
 *
 *  Open the data fork of the specified file.
 *
 */

OSErr
mac_open_file(const FSSpec *fs, char permission, short *f)
{
    if (fsspec_calls_available) {
        return FSpOpenDF(fs, permission, f);
    }
    else {
        static HParamBlockRec parm;
        OSErr result;

        parm.ioParam.ioCompletion = NULL;
        parm.ioParam.ioNamePtr = (StringPtr)&fs->name;
        parm.ioParam.ioVRefNum = fs->vRefNum;
        parm.fileParam.ioDirID = fs->parID;
        parm.ioParam.ioPermssn = permission;
        result = PBHOpen(&parm, false);
        *f = parm.ioParam.ioRefNum;
        return result;
    }
}

/*
 *  mac_open_file_resources
 *
 *  Open the resource fork of the specified file.
 *
 */

OSErr
mac_open_file_resources(const FSSpec *fs, char permission, short *f)
{
    if (fsspec_calls_available) {
        return FSpOpenRF(fs, permission, f);
    }
    else {
        static HParamBlockRec parm;
        OSErr result;

        parm.ioParam.ioCompletion = NULL;
        parm.ioParam.ioNamePtr = (StringPtr)&fs->name;
        parm.ioParam.ioVRefNum = fs->vRefNum;
        parm.fileParam.ioDirID = fs->parID;
        parm.ioParam.ioPermssn = permission;
        result = PBHOpenRF(&parm, false);
        *f = parm.ioParam.ioRefNum;
        return result;
    }
}

/*
 *  mac_rename_file
 *
 */

OSErr
mac_rename_file(const FSSpec *fs, const StringPtr newname)
{
    static HParamBlockRec parm;

    parm.ioParam.ioCompletion = NULL;
    parm.ioParam.ioNamePtr = (StringPtr)&fs->name;
    parm.ioParam.ioVRefNum = fs->vRefNum;
    parm.fileParam.ioDirID = fs->parID;
    parm.ioParam.ioMisc = (Ptr)newname;
    return PBHRename(&parm, false);
}

/*
 *  mac_replace_file
 *
 *  Replace OLD file with NEW file.  The two files must be in the
 *  same directory.
 *
 */

OSErr
mac_replace_file(const FSSpec *old, const FSSpec *new)
{
    OSErr result;

    result = paramErr; /* error when exchanging files not supported */
    if (fsspec_calls_available) {
        result = FSpExchangeFiles(old, new);
        if (result == 0) {
            mac_delete_file(new);
        }
    }
    if (result == paramErr) {
        /* exchange not supported: try renaming instead */
        FSSpec temp = *old;
        unsigned long time;

        GetDateTime(&time);
        sprintf((char *)temp.name, "%d sd old", time);
        CtoPstr((char *)temp.name);
        result = mac_rename_file(old, (StringPtr)&temp.name);
        if (result == 0) {
            result = mac_rename_file(new, (StringPtr)&old->name);
            if (result == 0) {
                mac_delete_file(&temp);
            }
        }
    }
    return result;
}
    
/*
 **********************************************************************
 *  PRIVATE FUNCTIONS
 **********************************************************************
 */

/*
 *  compute_find_folder_available
 *
 *  This function returns TRUE if the system contains FindFolder,
 *  FALSE otherwise.
 *
 */

static long_boolean
compute_find_folder_available(void)
{
    OSErr errcode;
    long features;

    if (!gestalt_available) {
        return FALSE;
    }
    errcode = Gestalt(gestaltFindFolderAttr, &features);
    if (errcode != 0) {
        return FALSE; /* assume not available */
    }
    else {
        return (features & (1 << gestaltFindFolderPresent)) != 0;
    }
}

/*
 *  compute_color_quickdraw_available
 *
 *  This function returns TRUE if the system contains ColorQuickDraw,
 *  FALSE otherwise.
 *
 */

static long_boolean
compute_color_quickdraw_available(void)
{
    OSErr errcode;
    long features;

    if (!gestalt_available) {
        return FALSE; /* assume not present (could be wrong) */
    }
    errcode = Gestalt(gestaltQuickdrawFeatures, &features);
    if (errcode != 0) {
        return FALSE; /* assume not present */
    }
    else {
        return (features & (1 << gestaltHasColor)) != 0;
    }
}

/*
 *  compute_new_file_package_available
 *
 *  This function returns TRUE if the system contains the new
 *  file system package (with functions taking FSSpec),
 *  FALSE otherwise.
 *
 */

static long_boolean
compute_new_file_package_available(void)
{
    int result;
    long features;
    
    if (!gestalt_available) {
        return FALSE;
    }
    result = Gestalt(gestaltStandardFileAttr, &features);
    return (result == 0) &&
           ((features & (1 << gestaltStandardFile58)) != 0);
}

/*
 *  compute_popup_control_available
 *
 *  This function returns TRUE if the system contains the popup
 *  menu control, FALSE otherwise.
 *
 */

static long_boolean
compute_popup_control_available(void)
{
    int result;
    long features;
    
    if (!gestalt_available) {
        return FALSE;
    }
    result = Gestalt(gestaltPopupAttr, &features);
    return (result == 0) &&
           ((features & (1 << gestaltPopupPresent)) != 0);
}

/*
 *  compute_fsspec_calls_available
 *
 *  This function returns TRUE if the system contains the new
 *  file specification calls, FALSE otherwise.
 *
 */

static long_boolean
compute_fsspec_calls_available(void)
{
    int result;
    long features;
    
    if (!gestalt_available) {
        return FALSE;
    }
    result = Gestalt(gestaltFSAttr, &features);
    return (result == 0) &&
           ((features & (1 << gestaltHasFSSpecCalls)) != 0);
}

/*
 **********************************************************************
 *  the following is based on "Inside Macintosh: Overview" listing 9-7
 **********************************************************************
 */

/*
 *  num_toolbox_traps
 *
 */

static int
num_toolbox_traps(void)
{
    if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap)) {
        return 0x200;
    }
    return 0x400;
}

/*
 *  get_trap_type
 *
 */

static TrapType 
get_trap_type(short the_trap)
{
    if ((the_trap & 0x0800) != 0) {
        return ToolTrap;
    }
    return OSTrap;
}

/*
 *  trap_available
 *
 */

static int
trap_available(short the_trap)
{
    TrapType tt;

    tt = get_trap_type(the_trap);
    if (tt == ToolTrap) {
        the_trap &= 0x07FF;
        if (the_trap >= num_toolbox_traps())
            the_trap = _Unimplemented;
    }
    return NGetTrapAddress(the_trap, tt) != NGetTrapAddress(_Unimplemented, ToolTrap);
}
