
/******************************************************************************
    Include
******************************************************************************/

#include <drv_disp.h>
#include <cpu_ext.h>
#include <os_cfg_app.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define _DRV_DISP_BUFFER_SIZE    (1024)
#define _DRV_DISP_TASK_NAME      "kokoto display driver"
#define _DRV_DISP_TASK_SIZE      (1024)
#define _DRV_DISP_TASK_PRIO      (5)

typedef struct _DRV_DISP_CTL {
	OS_TCB    stTcb;
	CPU_STK   aStack[_DRV_DISP_TASK_SIZE];
	CPU_CHAR  aszBuf[_DRV_DISP_BUFFER_SIZE];
} DRV_DISP_CTL;

static DRV_DISP_CTL  m_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

static void drv_disp_TaskMain(void* pParam_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_disp_Init(void)
{
	OS_ERR  err = OS_ERR_NONE;
	
    OSTaskCreate((OS_TCB     *)&(m_stCtl.stTcb),
                 (CPU_CHAR   *)((void *)_DRV_DISP_TASK_NAME),
                 (OS_TASK_PTR)drv_disp_TaskMain,
                 (void       *)0,
                 (OS_PRIO     )_DRV_DISP_TASK_PRIO,
                 (CPU_STK    *)(m_stCtl.aStack),
                 (CPU_STK_SIZE)_DRV_DISP_TASK_SIZE * OS_CFG_TASK_STK_LIMIT_PCT_EMPTY / 100,
                 (CPU_STK_SIZE)_DRV_DISP_TASK_SIZE,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)err);
	
	return;
}


CPU_INT32U  drv_disp_Print(const CPU_CHAR* pszStr_in)
{
	return (0);
}

void drv_disp_TaskMain(void* pParam_in)
{
	return;
}

