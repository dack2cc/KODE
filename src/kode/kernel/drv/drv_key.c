
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

#define _DRV_KEY_BUFFER_SIZE     (1024)
#define _DRV_KEY_TASK_NAME       "kokoto keyboard driver"
#define _DRV_KEY_TASK_PRIO       (15)
#define _DRV_KEY_MSG_QTY         (0x0F + 1)

#define _DRV_KEY_BREAK_BIT_MASK  (0x80)
#define _DRV_KEY_SCAN_CODE_MASK  (0xFF)

typedef struct _DRV_KEY_QUEUE {
	CPU_EXT_KEY_EVENT  buf[_DRV_KEY_MSG_QTY];
	CPU_INT08U         head;
	CPU_INT08U         tail;
} DRV_KEY_QUEUE;

#define _Q_INC(index_inout, size_in)  \
    ((index_inout) = ((index_inout)+1) & (size_in-1))
#define _Q_GET(queue_in, size_in, event_out)  \
    (void)({event_out=&((queue_in).buf[(queue_in).tail]);_Q_INC((queue_in).tail, size_in);})

typedef struct _DRV_KEY_ANALYZE {
    CPU_INT08U       uiMode;
	CPU_INT08U       uiFollowKeyNum;
	const CPU_CHAR*  pchKeyMap;
	const CPU_CHAR*  pchAltMap;
	const CPU_CHAR*  pchShfMap;
} DRV_KEY_ANALYZE;

typedef struct _DRV_KEY_CTL {
	OS_TCB           stTcb;
	DRV_KEY_QUEUE    stQueue;
	DRV_KEY_ANALYZE  stAnalyze;
} DRV_KEY_CTL;

DRV_PRIVATE  DRV_KEY_CTL   drv_key_stCtl;

DRV_PRIVATE const CPU_CHAR drv_key_achUSKeyMap[] = {
/* 00-0F */   0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 127, 9,
/* 10-1F */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,  0,   'a', 's',
/* 20-2F */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  0,  '`',  0,  0,   0,   0,   0,   0,
};

/******************************************************************************
    Private Interface
******************************************************************************/

DRV_PRIVATE  void drv_key_TaskMain(void* pParam_in);
DRV_PRIVATE  void drv_key_Handler(void* pstKeyEvent_in);

DRV_PRIVATE  void drv_key_ReceiverNone(CPU_INT08U  uiScanCode_in);
DRV_PRIVATE  void drv_key_ReceiverNormal(CPU_INT08U  uiScanCode_in);
DRV_PRIVATE  void drv_key_ReceiverFunction(CPU_INT08U  uiScanCode_in);

typedef void (*_DRV_KEY_FNCT_PTR)(CPU_INT08U  uiScanCode_in);
DRV_PRIVATE const _DRV_KEY_FNCT_PTR   drv_key_apfnReceiver[_DRV_KEY_SCAN_CODE_MASK + 1] = {
/* 00-03 s0 esc 1 2 */ drv_key_ReceiverNone, drv_key_ReceiverNormal, drv_key_ReceiverNormal, drv_key_ReceiverNormal,
/* 04-07 3 4 5 6 */ drv_key_ReceiverNormal, drv_key_ReceiverNormal, drv_key_ReceiverNormal, drv_key_ReceiverNormal,
/* 08-0B 7 8 9 0 */ drv_key_ReceiverNormal, drv_key_ReceiverNormal, drv_key_ReceiverNormal, drv_key_ReceiverNormal,
/* 0C-0F + ' bs tab */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 10-13 q w e r */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 14-17 t y u i */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 18-1B o p } ^ */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 1C-1F enter ctrl a s */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 20-23 d f g h */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 24-27 j k l | */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 28-2B { para lshift , */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 2C-2F z x c v */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 30-33 b n m , */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 34-37 . - rshift * */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 38-3B alt sp caps f1 */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 3C-3F f2 f3 f4 f5 */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 40-43 f6 f7 f8 f9 */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 44-47 f10 num scr home */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 48-4B up pgup - left */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 4C-4F n5 right + end */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 50-53 dn pgdn ins del */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 54-57 sysreq ? < f11 */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 58-5B f12 ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 5C-5F ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 60-63 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 64-67 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 68-6B ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 6C-6F ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 70-73 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 74-77 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 78-7B ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 7C-7F ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 80-83 ? br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 84-87 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 88-8B br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 8C-8F br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 90-93 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 94-97 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 98-9B br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* 9C-9F br unctrl br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* A0-A3 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* A4-A7 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* A8-AB br br unlshift br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* AC-AF br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* B0-B3 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* B4-B7 br br unrshift br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* B8-BB unalt br uncaps br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* BC-BF br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* C0-C3 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* C4-C7 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* C8-CB br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* CC-CF br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* D0-D3 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* D4-D7 br br br br */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* D8-DB br ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* DC-DF ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* E0-E3 e0 e1 ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* E4-E7 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* E8-EB ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* EC-EF ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* F0-F3 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* F4-F7 ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* F8-FB ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone,
/* FC-FF ? ? ? ? */ drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone, drv_key_ReceiverNone
};


/******************************************************************************
    Function Definition
******************************************************************************/

void drv_key_Init(void)
{
	OS_ERR  err = OS_ERR_NONE;
	
	drv_key_stCtl.stAnalyze.uiMode = 0;
	drv_key_stCtl.stAnalyze.uiFollowKeyNum = 0;
	drv_key_stCtl.stAnalyze.pchKeyMap = drv_key_achUSKeyMap;
	drv_key_stCtl.stAnalyze.pchAltMap = drv_key_achUSKeyMap;
	drv_key_stCtl.stAnalyze.pchShfMap = drv_key_achUSKeyMap;
	
	drv_key_stCtl.stQueue.head = 0;
	drv_key_stCtl.stQueue.tail = 0;
	
    OSTaskCreate(
    	/* p_tcb       */ (OS_TCB *)(&(drv_key_stCtl.stTcb)),
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
		drv_key_apfnReceiver[pstKeyEvent->uiScanCode](pstKeyEvent->uiScanCode);
		//CPUExt_DispPrint("Key Driver Receive Scan Code. \r\n");
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
	pstKeyMsg->uiScanCode = pstKeyEvent->uiScanCode;
	
	OSTaskQPost(
		/* p_tcb    */ &(drv_key_stCtl.stTcb),
		/* p_void   */ (void *)pstKeyMsg,
		/* msg_size */ sizeof(CPU_EXT_KEY_EVENT),
		/* opt      */ OS_OPT_POST_FIFO,
		/* p_err    */ &err
	);
	
	return;
}

DRV_PRIVATE  void drv_key_ReceiverNone(CPU_INT08U  uiScanCode_in)
{
	return;
}

DRV_PRIVATE  void drv_key_ReceiverNormal(CPU_INT08U  uiScanCode_in)
{
	CPU_CHAR        chAsciiCode = 0;
	const CPU_CHAR* pchKeyMap   = drv_key_stCtl.stAnalyze.pchKeyMap;
	
	if (0 != pchKeyMap) {
		chAsciiCode = pchKeyMap[uiScanCode_in];
		CPUExt_DispChar(chAsciiCode);
	}
	
	//chAsciiCode = drv_key_achUSMap[uiScanCode_in];
	
	return;
}

DRV_PRIVATE  void drv_key_ReceiverFunction(CPU_INT08U  uiScanCode_in)
{
	return;
}

