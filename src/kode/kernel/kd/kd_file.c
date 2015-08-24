/******************************************************************************
    Include
******************************************************************************/

#include <kd_file.h>
#include <kd_core.h>
#include <fs.h>
#include <os.h>
#include <lib_str.h>
#include <drv_disp.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define KD_PRIVATE  static
#define KD_PRIVATE

/******************************************************************************
    Private Interface
******************************************************************************/

#define  KD_FILE_NAME_MAX    (1024)
KD_PRIVATE CPU_CHAR kd_file_aszName[KD_FILE_NAME_MAX] = {0,};

#define  KD_FILE_FLAG_MAX    (8)
KD_PRIVATE CPU_CHAR kd_file_aszFlag[KD_FILE_FLAG_MAX] = {0,};

/******************************************************************************
    Function Definition
******************************************************************************/

/******************************************************************************
19.4.1. kdFopen
Open a file from the file system.
Synopsis
typedef struct KDFile KDFile;
KDFile *kdFopen(const KDchar *pathname, const KDchar *mode);
Description
This function opens, and possibly creates, a file in the file system of name pathname.
mode is a pointer to a string whose value determines the mode in which the file is opened, and is one of the following:
“r” or “rb” Read: file is opened for reading only
Write: file is created if necessary, otherwise truncated to 0 length, and opened for writing
only
“w” or “wb”
Append: file is created if necessary, and opened for writing only, at the end of the file.
The position of the file, and thus the result of a tell, is undefined.
“a” or “ab”
“r+” or “rb+” or “r+b” Update: file is opened for reading and writing positioned at the start of the file
“w+” or “wb+” or “w+b”
Append: file is created if necessary, and opened for writing at the end of the file and for
reading anywhere. The initial position of the file, and thus the position of a read or result
of a tell performed before the first write or seek, is undefined.
“a+” or “ab+” or “a+b”
Normally, there is an automatic conversion between the platform specific end-of-line encoding used in files in the file
system and a single linefeed character as file data appears to the application. When the mode string contains the
character ‘b’, the file is opened in “binary” mode, meaning that this automatic conversion is suppressed.
If the string pointed to by mode does not have one of the above values, it is undefined whether the open succeeds, and,
if so, what changes are made to the file and whether reading, writing or both are permitted.
Any files left open are automatically flushed and closed at application exit.
If pathname and mode are not both readable null-terminated strings, then undefined behavior results.
Return value
On success, the function returns a handle to the open file. On failure it returns KD_NULL and stores one of the error
codes below into the error indicator returned by kdGetError.
Error codes
KD_EACCES Permission denied. This error is given on an attempt to create a file in / or /removable or
open a file for writing in the subdirectory tree rooted at /res. There may be other
implementation-dependent circumstances where this error is given.
KD_EINVAL The specified mode is invalid. It is undefined whether an invalid mode gives this error.
KD_EIO I/O error.
KD_EISDIR The specified file path is a directory.
KD_EMFILE Too many open files. The circumstances leading to this error are implementation dependent.
KD_ENAMETOOLONG Path name is longer than the implementation-defined limit. It is undefined whether using a
too-long name results in this error.
KD_ENOENT File or directory not found.
KD_ENOMEM Out of memory or other resource.
KD_ENOSPC Out of filesystem space.
Rationale
kdFopen is based on the [C89] function fopen. [POSIX] adds the setting of errno on error.
[POSIX] defines additional error codes, some of which are Unix specific and so not applicable to OpenKODE Core,
but also including:
• ENFILE (global file table full): folded into KD_EMFILE by OpenKODE Core.
• ENOTDIR (a file path component other than the last is not a directory): folded into KD_ENOENT by OpenKODE
Core.
• EROFS (attempt to write on a read-only file system): folded into KD_EACCES by OpenKODE Core.
Append mode
[POSIX] does not properly define the interaction between the append modes ("a" and "a+") and the file position
indicator. OpenKODE Core leaves this explicitly undefined. In particular, in "a" mode, a write always writes to the
end of the file but the state of the file position indicator is completely undefined, whereas in "a+" mode, a write moves
the file position to the end of the file, but the state of the file position indicator on file open is undefined.
The usual ways of using append mode files will not encounter these areas of undefinedness. However, a “testing”
OpenKODE Core implementation might like to detect and abort on any attempt to rely on undefined behavior, using
a read or tell before the first seek or write on an "a+", or any use of tell on an "a" file.
******************************************************************************/
KDFile *  kd_file_Open(const KDchar * pszName_in, const KDchar * pszFlag_in)
{
    CPU_INT32U i = 0;
    OS_ERR  err = OS_ERR_NONE;
    OS_REG  hFile = 0;
    OS_REG  uiOpenMask = 0;
    FS_FILE * pFile = 0;

    if (0 == OSTCBCurPtr) {
        kd_core_SetError(KD_EBADF);
        CPUExt_CorePanic("[kd_file_Open][no task]");
        return ((KDFile *)0);
    }

    if ((0 == pszName_in)
            ||  (0 == pszFlag_in)) {
        kd_core_SetError(KD_EINVAL);
        drv_disp_Printf("[kd_file_Open][no file]\r\n");
        return ((KDFile *)0);
    }

    for (i = 0; i < OS_FILE_OPEN_PER_TASK; ++i) {
        hFile = OSTaskRegGet(OSTCBCurPtr, OS_TCB_REG_F_HEAD + i, &err);

        if (OS_ERR_NONE != err) {
            kd_core_SetError(KD_EIO);
            CPUExt_CorePanic("[kd_file_Open][file handle]");
            return ((KDFile *)0);
        }

        if (0 == hFile) {
            break;
        }
    }

    if (i >= OS_FILE_OPEN_PER_TASK) {
        kd_core_SetError(KD_EMFILE);
        drv_disp_Printf("[kd_file_Open][files full]\r\n");
        return ((KDFile *)0);
    }

    uiOpenMask = OSTaskRegGet(OSTCBCurPtr, OS_TCB_REG_F_MASK, &err);

    if (OS_ERR_NONE != err) {
        kd_core_SetError(KD_EIO);
        CPUExt_CorePanic("[kd_file_Open][file mask]");
        return ((KDFile *)0);
    }

    uiOpenMask &= ~(1 << i);

    pFile = FS_GetFreeFileHandler();

    if (0 == pFile) {
        kd_core_SetError(KD_EMFILE);
        drv_disp_Printf("[kd_file_Open][files full] \r\n");
        return ((KDFile *)0);
    }

    hFile = (OS_REG)pFile;
    pFile->f_count++;

    kd_core_StrReadUserSpace(pszName_in, kd_file_aszName, sizeof(kd_file_aszName));
    kd_core_StrReadUserSpace(pszFlag_in, kd_file_aszFlag, sizeof(kd_file_aszFlag));



    return ((KDFile *)0);
}


#if 0 /* NOTE */
/******************************************************************************
19.1. Introduction
OpenKODE Core provides functions to access an abstraction of the platform’s file system.
File paths are in a virtual file system, which allows portable access to several defined areas, while also allowing
non-portable access to the platform’s real file system. The virtual file system has its root at /, and then has subdirectories
such as /res and /data to allow portable access, and the subdirectory /native to allow non-portable access to
the platform’s real file system if the implementation chooses to allow that.
To ensure portability, as well as only using the defined areas, an application must be constrained by the OpenKODE
Core defined limits on path length and characters that may appear in the file paths, when creating a file (including the
case of creating a file for delivery along with the application during application development). A portable application
reading or otherwise accessing already-present files, and a non-portable application accessing the platform’s real file
system, do not need to observe these constraints; they are constrained only by the platform’s limits.
Functions that open, read, write and close a file are based on the [C89] (and [POSIX]) “stdio” functions, in which the
handle to an open file is a FILE*. No analogs of the [POSIX]-only file functions (where the handle to an open file is
an integer file descriptor) are provided. No analogs of the [C89] “stdio” formatted reading and writing functions (e.g.
fprintf and fscanf) are provided.
Of the other file functions, some are based on [C89] functions, and some are based on [POSIX] functions. kdGetFree
is not based on either [C89] or [POSIX].
The pathname / cannot be used with kdOpenDir, kdStat, kdAccess or kdGetFree.
19.1.1. Not thread safe
For any particular file handle or directory handle, using the same handle in OpenKODE function calls in multiple
threads at the same time results in undefined behavior.
19.2. File path
A file or directory has a name, known as its file path. These file paths exist in a virtual file system which has four
top-level directories:
/res Resources: Where the read-only data files that came installed along with the application are stored.
This is read only; it is an error to attempt to write to a file accessed via this path, and using kdAccess
on a file or directory in /res states that it is not writable.
This is not necessarily the same location as where the application itself is stored.
/data A suitable location to store the application’s persistent state. Each installed OpenKODE application
has its own /data area. It is undefined whether /data and /res are the same location; if they
are, then files from each are visible in the other.
/data is writable, that is the application can create and/or modify directories and files here.
/tmp A suitable location for temporary files. It is undefined whether files stored here are deleted by the
platform in between application runs. It is undefined whether this is the same location as /data. It
is undefined whether multiple applications share the same /tmp area.
/tmp is writable, that is the application can create and/or modify directories and files here.
/removable The location of any removable media devices on the device. This directory contains one or more
subdirectories, each corresponding to a particular removable media that is currently present. Each is
named after the slot in an implementation-defined way. The subdirectory for a slot is not present if
there is no media in the slot. If the implementation supports no removable media slots, then
/removable itself is absent.
/removable itself may be successfully used with kdOpenDir (to scan it to see what subdirectories
it contains), but cannot be used with kdStat, kdAccess or kdGetFree.
It is permitted for implementations to ignore certain removable media if it is not expected that
OpenKODE applications will want to access them. For instance, a PC may well want to ignore the
floppy drive, so directory listings in /removable are much faster.
/native The contents of /native are undefined by OpenKODE Core. It is intended to allow an
implementation to map some non-portable file area if it so chooses. Rules below on the limits and
semantics of file and directory names and the functions that take them do not apply to /native or
anything inside it. It is allowed for /native to be absent.
The OpenKODE Core implementation can map anything it likes in /native. One implementation
might map the platform’s native file system, a second might map nothing at all leaving /native
always empty and not able to accept new files or directories, a third might not have /native at all,
and a fourth might map some subset of the native file system.
Each of these locations already exists when the OpenKODE application starts (except for each of /removable and
/native where it is not present at all). Subdirectories are supported within each of these locations.
An attempt to write to (create a file or directory in) / or /removable, or to access either of those with kdStat or
kdAccess or to access / with kdOpenDir, results in a KD_EACCES error. It is allowed to use /removable in
kdOpenDir.
Filenames are defined to be UTF-8, but the only characters defined to be usable within filenames are the letters A-Z
and a-z, the digits 0-9, and the characters ‘.’ (period), ‘_’ (underscore) and ‘-’ (hyphen-minus). It is undefined whether
other characters are allowed. It is undefined whether filenames are case sensitive. It is undefined whether a filename
with a trailing period refers to the same file as the same filename without the trailing period.
Forward slash characters are used as the directory separator. Directory separators separate a file path into components.
Where a file path has adjacent multiple directory separators, it is undefined what it actually refers to.
A file path specified to an OpenKODE Core function is either absolute, it starts with one of the top-level directories
listed above, or is relative to /res (so prepending /res/ gives the equivalent absolute file path.
If any component of a file path is one period “.” or two periods “..” then it is undefined what the file path refers to.
A file path is allowed to be up to 48 bytes long, not including the initial top-level directory component (but including
the directory separator just after it). Where a file path exceeds the limit, it is undefined what it refers to or whether it
causes an error on any attempt to use it.
19.2.1. File path limits
It is expected that most implementations will allow a file path considerably more than 48 bytes long, however an
application which takes advantage of this will lose some portability.
Note that this set of characters specified above are the minimum requirements for OpenKODE compliance.
Implementations should expose the full capabilities of their native file systems in terms of what characters are permitted,
however portable applications should be written so that their files with fixed names are named according to these rules.
A portable application can, of course, still access files with names outside these rules, and can allow the user to create
files with names outside these rules. A portable application should ensure that there are no files whose names only
differ by case, and should access those files using the canonical case, in order to ensure it works on both case-sensitive
and case-insensitive file systems.
19.2.2. Rationale
OpenKODE Core 1.0 Provisional contained the notion of a current directory, together with functions to set and get it.
It was removed with the justification that current directory is mostly useful only when the application is started from
a user interface that has a notion of the user’s current location which is passed to the application as its current directory.
This is outside the scope of OpenKODE Core.
******************************************************************************/
#endif /* NOTE */

