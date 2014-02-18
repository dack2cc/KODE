#ifndef __DRV_LOCK_H__
#define __DRV_LOCK_H__

/******************************************************************************
    Include
******************************************************************************/

#include <os.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#define  DRV_LOCK  OS_SEM

inline void drv_lock_Init(DRV_LOCK* pLock_inout, CPU_CHAR* pszName_in);
inline void drv_lock_SleepOn(DRV_LOCK* pLock_in);
inline void drv_lock_WakeUp(DRV_LOCK* pSem_in);

#endif // __DRV_LOCK_H__

