
/******************************************************************************
    Include
******************************************************************************/

#include <drv_mice.h>
#include <drv_disp.h>
#include <cpu_ext.h>
#include <os_cfg_app.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE

#define _DRV_MICE_BUFFER_SIZE     (1024)
#define _DRV_MICE_TASK_NAME       "kokoto mouse driver"
#define _DRV_MICE_TASK_PRIO       (15)
#define _DRV_MICE_MSG_QTY         (0x0F + 1)

typedef struct _DRV_MICE_QUEUE {
    CPU_EXT_PS2_EVENT  buf[_DRV_MICE_MSG_QTY];
    CPU_INT08U         head;
    CPU_INT08U         tail;
} DRV_MICE_QUEUE;

#define _Q_INC(index_inout, size_in)  \
    ((index_inout) = ((index_inout)+1) & (size_in-1))
#define _Q_GET(queue_in, size_in, event_out)  \
    (void)({event_out=&((queue_in).buf[(queue_in).tail]);_Q_INC((queue_in).tail, size_in);})

#define _DRV_MICE_CODE_PER_OPE   (3)

typedef struct _DRV_MICE_ANALYZE {
    CPU_INT08U  uiType;
    CPU_INT08U  uiPhase;
    CPU_INT08U  auiCode[_DRV_MICE_CODE_PER_OPE];
    CPU_INT08U  uiBtn;
    CPU_INT32S  iX;
    CPU_INT32S  iY;
} DRV_MICE_ANALYZE;

typedef struct _DRV_MICE_CONTROL {
    OS_TCB           stTcb;
    DRV_MICE_QUEUE   stQueue;
    DRV_MICE_ANALYZE stAnalyze;
    CPU_FNCT_PTR     pfnHandle;
} DRV_MICE_CONTROL;

DRV_PRIVATE  DRV_MICE_CONTROL  drv_mice_stCtl;


/******************************************************************************
    Private Interface
******************************************************************************/

DRV_PRIVATE  void drv_mice_TaskMain(void* pParam_in);
DRV_PRIVATE  void drv_mice_Handler(void* pstMiceEvent_in);

DRV_PRIVATE  void drv_mice_Analyze(const CPU_INT08U uiScanCode_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_mice_Init(void)
{
    OS_ERR  err = OS_ERR_NONE;

    drv_mice_stCtl.pfnHandle = 0;
    drv_mice_stCtl.stAnalyze.uiType  = 0;
    drv_mice_stCtl.stAnalyze.uiPhase = 0;

    drv_mice_stCtl.stQueue.head = 0;
    drv_mice_stCtl.stQueue.tail = 0;

    OSTaskCreate(
        /* p_tcb       */ (OS_TCB *)(&(drv_mice_stCtl.stTcb)),
        /* p_name      */ _DRV_MICE_TASK_NAME,
        /* p_task      */ drv_mice_TaskMain,
        /* p_arg       */ 0,
        /* prio        */ _DRV_MICE_TASK_PRIO,
        /* p_stk_base  */ 0, //(drv_key_stCtl.stTask.aStack),
        /* stk_limit   */ 0, //(CPU_STK_SIZE)_DRV_MICE_TASK_SIZE * OS_CFG_TASK_STK_LIMIT_PCT_EMPTY / 100,
        /* stk_size    */ 0, //(CPU_STK_SIZE)_DRV_MICE_TASK_SIZE,
        /* q_size      */ _DRV_MICE_MSG_QTY,
        /* time_quanta */ 0u,
        /* p_ext       */ 0,
        /* opt         */ (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
        /* p_err       */ &err
    );

    if (OS_ERR_NONE != err) {
        drv_disp_Printf("[drv_mice_Init][Failed]\r\n");
    }

    CPUExt_PS2MouseType(&(drv_mice_stCtl.stAnalyze.uiType));

    if (0x00 == drv_mice_stCtl.stAnalyze.uiType) {
        CPUExt_PS2RegisterHandler(CPU_EXT_PS2_TYPE_MOUSE, drv_mice_Handler);
    } else {
        drv_disp_Printf("[drv_mice_Init][Not Supported Mouse]\r\n");
    }

    return;
}

void drv_mice_RegisterHandler(CPU_FNCT_PTR pfnMice_in)
{
    drv_mice_stCtl.pfnHandle = pfnMice_in;
}


DRV_PRIVATE  void drv_mice_TaskMain(void* pParam_in)
{
    OS_ERR              err = OS_ERR_NONE;
    void*               pMsg = 0;
    OS_MSG_SIZE         size = 0;
    CPU_EXT_PS2_EVENT*  pstEvent = 0;

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
                ||  (sizeof(CPU_EXT_PS2_EVENT) != size)) {
            continue;
        }

        pstEvent = (CPU_EXT_PS2_EVENT *)pMsg;
        //drv_disp_Printf("[MICE][0x%X]", pstEvent->uiScanCode);
        drv_mice_Analyze(pstEvent->uiScanCode);
    }

    return;
}

DRV_PRIVATE  void drv_mice_Handler(void* pstEvent_in)
{
    OS_ERR              err = OS_ERR_NONE;
    CPU_EXT_PS2_EVENT*  pstMsg = 0;
    CPU_EXT_PS2_EVENT*  pstEvent = (CPU_EXT_PS2_EVENT *)pstEvent_in;

    if (0 == pstEvent_in) {
        return;
    }

    _Q_GET(drv_mice_stCtl.stQueue, _DRV_MICE_MSG_QTY, pstMsg);
    pstMsg->uiScanCode = pstEvent->uiScanCode;

    OSTaskQPost(
        /* p_tcb    */ & (drv_mice_stCtl.stTcb),
        /* p_void   */ (void *)pstMsg,
        /* msg_size */ sizeof(CPU_EXT_PS2_EVENT),
        /* opt      */ OS_OPT_POST_FIFO,
        /* p_err    */ &err
    );

    return;
}

DRV_PRIVATE  void drv_mice_Analyze(const CPU_INT08U uiScanCode_in)
{
    DRV_MICE_EVENT  stEvent;
    CPU_FNCT_PTR    pfnHandle = drv_mice_stCtl.pfnHandle;

    if (0 == pfnHandle) {
        return;
    }

    drv_mice_stCtl.stAnalyze.auiCode[drv_mice_stCtl.stAnalyze.uiPhase] = uiScanCode_in;
    drv_mice_stCtl.stAnalyze.uiPhase++;

    if (drv_mice_stCtl.stAnalyze.uiPhase >= _DRV_MICE_CODE_PER_OPE) {
        drv_mice_stCtl.stAnalyze.uiPhase = 0;
        //drv_disp_Printf("[drv_mice_Analyze][0x%X, 0x%X, 0x%X] \r\n", drv_mice_stCtl.stAnalyze.auiCode[0], drv_mice_stCtl.stAnalyze.auiCode[1], drv_mice_stCtl.stAnalyze.auiCode[2]);

        if ((0x00 != ((drv_mice_stCtl.stAnalyze.auiCode[0]) & 0xC0))
                ||  (0x08 != ((drv_mice_stCtl.stAnalyze.auiCode[0]) & 0x08))) {
            CPUExt_PS2MouseReset();
            return;
        }

        drv_mice_stCtl.stAnalyze.uiBtn = drv_mice_stCtl.stAnalyze.auiCode[0] & 0x07;

        if (0 != (drv_mice_stCtl.stAnalyze.auiCode[0] & 0x10)) {
            drv_mice_stCtl.stAnalyze.iX = 0xFFFFFF00 | (drv_mice_stCtl.stAnalyze.auiCode[1]);
        } else {
            drv_mice_stCtl.stAnalyze.iX = drv_mice_stCtl.stAnalyze.auiCode[1];
        }

        if (0 != (drv_mice_stCtl.stAnalyze.auiCode[0] & 0x20)) {
            drv_mice_stCtl.stAnalyze.iY = 0xFFFFFF00 | (drv_mice_stCtl.stAnalyze.auiCode[2]);
        } else {
            drv_mice_stCtl.stAnalyze.iY = drv_mice_stCtl.stAnalyze.auiCode[2];
        }

        drv_mice_stCtl.stAnalyze.iY = -(drv_mice_stCtl.stAnalyze.iY);

        if ((0 != drv_mice_stCtl.stAnalyze.iY)
                ||  (0 != drv_mice_stCtl.stAnalyze.iX)
                ||  (0 != drv_mice_stCtl.stAnalyze.uiBtn)) {
            stEvent.iOffsetX = drv_mice_stCtl.stAnalyze.iX;
            stEvent.iOffsetY = drv_mice_stCtl.stAnalyze.iY;
            stEvent.uiButton = drv_mice_stCtl.stAnalyze.uiBtn;

            pfnHandle((void *)(&stEvent));
            //CPUExt_DispMouse(drv_mice_stCtl.stAnalyze.iX, drv_mice_stCtl.stAnalyze.iY);
        }

        //drv_disp_Printf("[code][%d, %d, %d] \r\n", drv_mice_stCtl.stAnalyze.uiBtn, drv_mice_stCtl.stAnalyze.iX, drv_mice_stCtl.stAnalyze.iY);
    }
}


