/******************************************************************************
    Include
******************************************************************************/

#include <kd_time.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define KD_PRIVATE  static
//#define KD_PRIVATE

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

/******************************************************************************
17.2.1. kdGetTimeUST
Get the current unadjusted system time.
Synopsis
KDust kdGetTimeUST(void);
Description
This function returns the current unadjusted system time.
Unadjusted system time measures time in nanoseconds since a datum (for example since the platform was powered
up). It is guaranteed to be monotonically increasing, and is not adjusted even if the deviceÅfs wall clock time is adjusted
in some way. UST may or may not stand still while the platform is suspended, but it will not decrease or be reset back
as a result of the suspension.
Return value
The function returns the current UST.
******************************************************************************/
KDust  kd_time_GetUST(void)
{
    CPU_INT32U uiTick = 0;

    CPUExt_TimeTick(&uiTick);

    return ((KDust)uiTick);
}

/******************************************************************************
17.2.2. kdTime
Get the current wall clock time.
Synopsis
KDtime kdTime(KDtime *timep);
Description
This function gets the current wall clock time in seconds since midnight UTC, January 1st 1970 (the epoch).
If timep is not KD_NULL, then the time is also stored in the location pointed to by timep, as well as being returned
by the function.
No guarantee can be made about the accuracy of the wall clock time returned by this function. In particular, the user
may be able to change it to the wrong value, the platform may change it in response to some external time signal, and
the platform may have no concept of time zones and thus will return the local time rather than UTC.
If timep is not KD_NULL and does not point to a writable KDtime location, then undefined behavior results.
Return value
The function returns (the platformÅfs idea of) wall clock time in seconds since midnight UTC, January 1st 1970.
Rationale
kdTime is based on the [C89] function time. [C89] does not define that time_t (its analog of KDtime) needs to be
an arithmetic type; [POSIX] does.
******************************************************************************/
KDtime kd_time_GetWallClock(KDtime * timep)
{
    CPU_INT32U uiTime = 0;

    CPUExt_TimeCurrent(&uiTime);

    if (timep) {
        CPU_EXT_PUT_FS_LONG(uiTime, timep);
    }

    return ((KDtime)uiTime);
}

#if 0 /* NOTE */
/******************************************************************************
17. Time functions
17.1. Introduction
Here, OpenKODE Core provides functions based on [C89]Åfs <time.h>, where time in seconds since epoch (midnight
UST, January 1st 1970) can be obtained and converted into human-readable date and time.
In addition, OpenKODE Core provides unadjusted system time (UST), measured in nanoseconds since some arbitrary
datum, which, as well as being fine-grained enough to expose whatever accuracy the platform allows, is defined never
to decrease so it could be useful in timestamping multimedia objects and events.
There is no OpenKODE Core function to sleep for a length of time equivalent to the [POSIX] sleep function. Such
a function would not fit easily into OpenKODE CoreÅfs event-driven architecture. A timed delay can be achieved one
of two ways:
 For a thread that does not receive any events (not the main thread), a simple call to kdWaitEvent with a timeout
can be used.
 More generally, a thread can set up a timer using kdSetTimer, which will deliver its event into the threadÅfs event
queue after the specified delay.
******************************************************************************/
#endif /* NOTE */

