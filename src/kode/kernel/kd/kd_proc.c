
/******************************************************************************
    Include
******************************************************************************/

#include <kd_proc.h>
#include <kd_core.h>
#include <os.h>
#include <os_cfg_app.h>
#include <cpu_ext.h>
#include <cpu_boot.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define KD_PRIVATE   static
#define KD_PRIVATE 

#define _KD_PROC_KERNEL_STACK_SIZE    (3 * 1024)
#define _KD_PROC_USER_STACK_SIZE      (64 * 1024 * 1024)

struct KDExtProcess {
	OS_TCB    stTCB;
};

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

KDExtProcess * kd_proc_Create(
	KDuint32 resversed, 
	void *(* pfnStartRoutine_in)(void *), void * arg_in, 
	KDuint32 reversed_0, KDuint32 reversed_1, KDuint32 reversed_2, KDuint32 reversed_3, KDuint32 reversed_4, 
	KDuint32 eflag_in
)
{
	CPU_ADDR       addrPhyPage = 0;
	KDExtProcess * pstProcess  = KD_NULL;
	void*          pExtData    = KD_NULL;
	CPU_DATA*      pExtArg     = KD_NULL;
	OS_ERR         os_err      = OS_ERR_NONE;
	
	CPUExt_PageGetFree((&addrPhyPage));
	if (0 == addrPhyPage) {
		kd_core_SetError(KD_EAGAIN);
		return (KD_NULL);		
	}
	pstProcess = (KDExtProcess *)addrPhyPage;
	pExtData   = (void *)((CPU_INT32U)addrPhyPage + sizeof(KDExtProcess));
	pExtArg    = (CPU_DATA *)pExtData;
	
	pExtArg[OS_TCB_EXT_EFLAG]      = eflag_in;
	pExtArg[OS_TCB_EXT_RET_POINT]  = (CPU_DATA)(&kdextProcessExit);
	pExtArg[OS_TCB_EXT_IS_PROCESS] = 1;
	pExtArg[OS_TCB_EXT_USER_STACK] = 0;
	
	OSTaskCreate(
		/* p_tcb       */   &(pstProcess->stTCB),
		/* p_name      */   0,
		/* p_task      */   (OS_TASK_PTR)pfnStartRoutine_in,
		/* p_arg       */   arg_in,
		/* prio        */   50,
		/* p_stk_base  */   (CPU_STK *)((CPU_INT32U)addrPhyPage + (X86_MEM_PAGE_SIZE - _KD_PROC_KERNEL_STACK_SIZE)), 
		/* stk_limit   */   ((_KD_PROC_KERNEL_STACK_SIZE * OS_CFG_TASK_STK_LIMIT_PCT_EMPTY) / 100),
		/* stk_size    */   _KD_PROC_KERNEL_STACK_SIZE,
		/* q_size      */   0,
		/* time_quanta */   0,
		/* p_ext       */   pExtData,
		/* opt         */   0, 
		/* p_err       */   &os_err
	);
	
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(os_err);
		CPUExt_PageRelease(addrPhyPage);
		addrPhyPage = 0;
		return (KD_NULL);
	}
	
	return (pstProcess);
}

void   kd_proc_Exit(void * pRetval_in)
{
	OS_TaskReturn();
}

