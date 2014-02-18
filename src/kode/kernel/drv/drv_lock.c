/******************************************************************************
    Include
******************************************************************************/

#include <drv_lock.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/


/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

inline void drv_lock_Init(DRV_LOCK* pLock_inout, CPU_CHAR* pszName_in)
{
	OS_ERR     err = OS_ERR_NONE;

	OSSemCreate(
		/* p_sem  */ pLock_inout,
		/* p_name */ pszName_in,
		/* cnt    */ 0,
		/* *p_err */ &err
	);
	if (OS_ERR_NONE != err) {
		CPUExt_CorePanic("[drv_lock_Init][create failed]");
	}
}

inline void drv_lock_SleepOn(DRV_LOCK* pLock_in)
{
	OS_ERR     err = OS_ERR_NONE;
	OS_SEM_CTR ctr = 0;
	
	ctr = OSSemPend(
		/* p_sem   */ pLock_in,
		/* timeout */ 0,
		/* opt     */ OS_OPT_PEND_BLOCKING,
		/* p_ts    */ 0,
		/* p_err   */ &err
	);
	if (OS_ERR_NONE != err) {
		CPUExt_CorePanic("[drv_lock_SleepOn][Lock error]");
	}
	if (0 != ctr) {
		CPUExt_CorePanic("[drv_lock_SleepOn][Lock exception]");
	}
}

inline void drv_lock_WakeUp(OS_SEM* pSem_in)
{
	OS_ERR     err = OS_ERR_NONE;
	OS_SEM_CTR ctr = 0;
	
	if (0 == pSem_in) {
		CPUExt_CorePanic("[drv_lock_WakeUp][Lock is invalid]");
	}
	if (0 == pSem_in->PendList.NbrEntries) {
		return;
	}
	
	ctr = OSSemPost(
		/* p_sem   */ pSem_in,
		/* opt     */ OS_OPT_POST_1,
		/* p_err   */ &err
	);
	if (OS_ERR_NONE != err) {
		CPUExt_CorePanic("[drv_lock_WakeUp][Lock error]");
	}
	if (0 != ctr) {
		CPUExt_CorePanic("[drv_lock_WakeUp][Lock exception]");
	}
}


