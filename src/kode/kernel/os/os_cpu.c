
/******************************************************************************
    Include
******************************************************************************/

#include <os.h>
#include <cpu_core.h>
#include <cpu_boot.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Definition
******************************************************************************/


/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void  OSStartHighRdy(void)
{
	CPU_INT32U  uiTaskId = 0;
	OS_ERR      os_err = OS_ERR_NONE;
	
	uiTaskId = OSTaskRegGet(OSTCBCurPtr, OS_TCB_REG_TASK_ID, &os_err);
	CPUExt_TaskSwitch(uiTaskId);
}

void  OSIntCtxSw(void)
{
	CPU_INT32U  uiTaskId = 0;
	OS_ERR      os_err = OS_ERR_NONE;
	
	OSPrioCur   = OSPrioHighRdy;
	OSTCBCurPtr = OSTCBHighRdyPtr;
	uiTaskId = OSTaskRegGet(OSTCBCurPtr, OS_TCB_REG_TASK_ID, &os_err);
	CPUExt_TaskSwitch(uiTaskId);
}

void  OSInitHook(void)
{
	
	CPUExt_GateRegisterTimeTick(OSTimeTick);
	CPUExt_GateRegisterISRHookEnter(OSIntEnter);
	CPUExt_GateRegisterISRHookExit(OSIntExit);
	
	/* <= the init process in Ring 0 */
	CPU_Init();
	/* => the task 0 in Ring 0 */
}

void  OSInitEndHook(void)
{
	CPU_IntEn();
	
	CPUExt_TaskSwitchToRing3();
	/* => the task 0 in Ring 3 */
}


CPU_STK * OSTaskStkInit
(
	OS_TASK_PTR            p_task,
	void                  *p_arg,
	CPU_STK               *p_stk_base,
	CPU_STK               *p_stk_limit,
	CPU_STK_SIZE           stk_size,
	OS_OPT                 opt
)
{
	return (p_stk_base + stk_size - 1);
}


void OSTaskCreateHook(OS_TCB *p_tcb)
{
	CPU_DATA    aiArgList[CPU_TASK_ARG_MAX];
	CPU_SR      cpu_sr = 0;
	CPU_DATA*   pExtData = 0;
	CPU_INT32U  uiTaskId = 0;
	CPU_INT32U  i = 0;
	CPU_ERR     cpu_err = CPU_ERR_NONE;
	OS_ERR      os_err  = OS_ERR_NONE;
	
	if (0 == p_tcb) {
		return;
	}

	aiArgList[CPU_TASK_ARG_ROUTINE]   = (CPU_DATA)(p_tcb->TaskEntryAddr);
	aiArgList[CPU_TASK_ARG_PARAMETER] = (CPU_DATA)(p_tcb->TaskEntryArg);
	
	/* kernel space task */
	if ( ((OS_OPT)0 != (p_tcb->Opt & OS_OPT_TASK_STK_CHK)) 
	&&   ((OS_OPT)0 != (p_tcb->Opt & OS_OPT_TASK_STK_CLR))
	&&   ((void *)0 == (p_tcb->ExtPtr)) ) {
    	/* get current eflag and enable the interrupt */
	    __asm__("pushfl \n\t popl %%eax":"=a"(cpu_sr):);
	    cpu_sr |= 0x200;
		aiArgList[CPU_TASK_ARG_EFLAG]     = (CPU_DATA)(cpu_sr);
		
		aiArgList[CPU_TASK_ARG_RET_POINT] = (CPU_DATA)(&OS_TaskReturn);
	    aiArgList[CPU_TASK_ARG_KERNEL_EN] = (CPU_DATA)(DEF_ENABLED);
		
		CPUExt_PageGetFree((CPU_ADDR *)(&(p_tcb->ExtPtr)));
		aiArgList[CPU_TASK_ARG_KERNEL_STACK] = (CPU_INT32U)(p_tcb->ExtPtr) + X86_MEM_PAGE_SIZE - 4;
		aiArgList[CPU_TASK_ARG_EXT_DATA]     = (CPU_DATA)(p_tcb->ExtPtr);		
	}
	
	/* user space task */
	else {
		pExtData = (CPU_DATA *)(p_tcb->ExtPtr);
		aiArgList[CPU_TASK_ARG_EFLAG]        = (CPU_DATA)(pExtData[OS_TCB_EXT_EFLAG]);
		aiArgList[CPU_TASK_ARG_RET_POINT]    = (CPU_DATA)(pExtData[OS_TCB_EXT_RET_POINT]);
		
	    aiArgList[CPU_TASK_ARG_KERNEL_EN]    = (CPU_DATA)(DEF_DISABLED);
		aiArgList[CPU_TASK_ARG_KERNEL_STACK] = (CPU_DATA)(p_tcb->StkPtr);
		aiArgList[CPU_TASK_ARG_EXT_DATA]     = (CPU_DATA)(p_tcb->ExtPtr);
	}
	
	cpu_err = CPUExt_TaskCreate(aiArgList, &uiTaskId);
	
	OSTaskRegSet(p_tcb, OS_TCB_REG_ERR_CODE, cpu_err, &os_err);
	OSTaskRegSet(p_tcb, OS_TCB_REG_KERNEL_EN, aiArgList[CPU_TASK_ARG_KERNEL_EN], &os_err);
	OSTaskRegSet(p_tcb, OS_TCB_REG_TASK_ID, uiTaskId, &os_err);
	OSTaskRegSet(p_tcb, OS_TCB_REG_FILE_FLAG, 0, &os_err);
	for (i = 0; i < OS_FILE_OPEN_PER_TASK; ++i) {
		OSTaskRegSet(p_tcb, (OS_TCB_REG_FILE_START + i), 0, &os_err);
	}
}

void OSTaskReturnHook(OS_TCB *p_tcb)
{
	/* EMPTY */
}

void OSIdleTaskHook(void)
{
	//CPUExt_DispPrint("=> Idle ");
}

void  OSTimeTickHook(void)
{
	//CPUExt_DispPrint("=> Tick ");
}

#if (OS_CFG_TASK_DEL_EN > 0u)
void  OSTaskDelHook(OS_TCB *p_tcb)
{
	CPUExt_PageRelease((CPU_ADDR)p_tcb);
}
#endif /* OS_CFG_TASK_DEL_EN */


#if (OS_CFG_STAT_TASK_EN > 0u)
void  OSStatTaskHook(void)
{
	return;
}
#endif /* OS_CFG_STAT_TASK_EN */

