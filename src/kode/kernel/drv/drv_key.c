
/******************************************************************************
    Include
******************************************************************************/

#include <drv_key.h>
#include <cpu_ext.h>
#include <os_cfg_app.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

#define _DRV_KEY_BUFFER_SIZE    (1024)
#define _DRV_KEY_TASK_NAME      "kokoto keyboard driver"
#define _DRV_KEY_TASK_SIZE      (1)
#define _DRV_KEY_TASK_PRIO      (15)
#define _DRV_KEY_MSG_QTY        (0x0F + 1)

typedef struct _DRV_KEY_TASK {
	OS_TCB    stTcb;
	CPU_STK   aStack[_DRV_KEY_TASK_SIZE];
} DRV_KEY_TASK;

typedef struct _DRV_KEY_QUEUE {
	CPU_EXT_KEY_EVENT  buf[_DRV_KEY_MSG_QTY];
	CPU_INT08U         head;
	CPU_INT08U         tail;
} DRV_KEY_QUEUE;

#define _Q_INC(index_inout, size_in)  \
    ((index_inout) = ((index_inout)+1) & (size_in-1))
#define _Q_GET(queue_in, size_in, event_out)  \
    (void)({event_out=&((queue_in).buf[(queue_in).tail]);_Q_INC((queue_in).tail, size_in);})

typedef struct _DRV_KEY_CTL {
	DRV_KEY_TASK   stTask;
	DRV_KEY_QUEUE  stQueue;
} DRV_KEY_CTL;

DRV_PRIVATE  DRV_KEY_CTL  drv_key_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

DRV_PRIVATE  void drv_key_TaskMain(void* pParam_in);
DRV_PRIVATE  void drv_key_Handler(void* pstKeyEvent_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_key_Init(void)
{
	OS_ERR  err = OS_ERR_NONE;
	
	drv_key_stCtl.stQueue.head = 0;
	drv_key_stCtl.stQueue.tail = 0;
	
    OSTaskCreate(
    	/* p_tcb       */ (OS_TCB *)(&(drv_key_stCtl.stTask.stTcb)),
        /* p_name      */ _DRV_KEY_TASK_NAME,
    	/* p_task      */ drv_key_TaskMain,
        /* p_arg       */ 0,
        /* prio        */ _DRV_KEY_TASK_PRIO,
        /* p_stk_base  */ 0, //(drv_key_stCtl.stTask.aStack),
        /* stk_limit   */ 0, //(CPU_STK_SIZE)_DRV_KEY_TASK_SIZE * OS_CFG_TASK_STK_LIMIT_PCT_EMPTY / 100,
        /* stk_size    */ 0, //(CPU_STK_SIZE)_DRV_KEY_TASK_SIZE,
        /* q_size      */ _DRV_KEY_MSG_QTY,
        /* time_quanta */ 0u,
        /* p_ext       */ 0,
        /* opt         */ (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
        /* p_err       */ &err
    );
	
	if (OS_ERR_NONE == err) {
		CPUExt_KeyRegisterHandler(drv_key_Handler);
	}
	
	return;
}


DRV_PRIVATE  void drv_key_TaskMain(void* pParam_in)
{
	OS_ERR              err = OS_ERR_NONE;
	void*               pMsg = 0;
	OS_MSG_SIZE         size = 0;
	CPU_EXT_KEY_EVENT*  pstKeyEvent = 0;
	
	for (;;) {
		pMsg = OSTaskQPend(
			/* timeout    */ 0,
			/* opt        */ OS_OPT_PEND_BLOCKING,
			/* p_msg_size */ &size,
			/* p_ts       */ 0,
			/* p_err      */ &err
		);
		
		if ((OS_ERR_NONE != err) 
		||  (0 == pMsg) 
		||  (sizeof(CPU_EXT_KEY_EVENT) != size)) {
			continue;
		}
		
		pstKeyEvent = (CPU_EXT_KEY_EVENT *)pMsg;
		
		CPUExt_DispPrint("Key Driver Receive Scan Code.");
	}
	
	return;
}

DRV_PRIVATE  void drv_key_Handler(void* pstKeyEvent_in)
{
	OS_ERR              err = OS_ERR_NONE;
	CPU_EXT_KEY_EVENT*  pstKeyMsg = 0;
	CPU_EXT_KEY_EVENT*  pstKeyEvent = (CPU_EXT_KEY_EVENT *)pstKeyEvent_in;
	
	if (0 == pstKeyEvent) {
		return;
	}
	
	_Q_GET(drv_key_stCtl.stQueue, _DRV_KEY_MSG_QTY, pstKeyMsg);
	pstKeyMsg->uiState    = pstKeyEvent->uiState;
	pstKeyMsg->uiScanCode = pstKeyEvent->uiScanCode;
	
	OSTaskQPost(
		/* p_tcb    */ &(drv_key_stCtl.stTask.stTcb),
		/* p_void   */ (void *)pstKeyMsg,
		/* msg_size */ sizeof(CPU_EXT_KEY_EVENT),
		/* opt      */ OS_OPT_POST_FIFO,
		/* p_err    */ &err
	);
	
	return;
}

