/******************************************************************************
    Include
******************************************************************************/

#include <gui.h>

#if (CPU_EXT_DISP_MODE != CPU_EXT_DISP_MODE_TEXT)

#include <gui_bg.h>
#include <gui_log.h>
#include <gui_mice.h>
#include <drv_gfx.h>
#include <drv_disp.h>
#include <drv_mice.h>
#include <os.h>
//#include <lib_mem.h>
//#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE

#define _GUI_CORE_BUFFER_SIZE     (1024)
#define _GUI_CORE_TASK_NAME       "kokoto gui core"
#define _GUI_CORE_TASK_PRIO       (16)
#define _GUI_CORE_MSG_QTY         (0x0F + 1)
#define _GUI_CORE_TIMER_NAME      "kokoto gui timer"
#define _GUI_CORE_TIMER_1S        100

enum {
    CPU_CORE_EVT_REFRESH = 0,
    CPU_CORE_EVT_TIMER,
    CPU_CORE_EVT_MOUSE,
    CPU_CORE_EVT_KEY_PRESS
};

typedef union _GUI_CORE_DATA {
    DRV_MICE_EVENT  stMice;
} GUI_CORE_DATA;

typedef struct _GUI_CORE_EVENT {
    CPU_INT32U    uiType;
    GUI_CORE_DATA unData;
} GUI_CORE_EVENT;

typedef struct _GUI_CORE_QUEUE {
    GUI_CORE_EVENT     buf[_GUI_CORE_MSG_QTY];
    CPU_INT08U         head;
    CPU_INT08U         tail;
} GUI_CORE_QUEUE;

#define _Q_INC(index_inout, size_in)  \
    ((index_inout) = ((index_inout)+1) & (size_in-1))
#define _Q_GET(queue_in, size_in, event_out)  \
    (void)({event_out=&((queue_in).buf[(queue_in).tail]);_Q_INC((queue_in).tail, size_in);})


typedef struct _GUI_CORE_CONTROL {
    OS_TMR           stTmr;
    OS_TCB           stTcb;
    GUI_CORE_QUEUE   stQueue;
} GUI_CORE_CONTROL;

GUI_PRIVATE  GUI_CORE_CONTROL  gui_core_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

GUI_PRIVATE  void gui_core_TaskMain(void* pParam_in);
GUI_PRIVATE  void gui_core_Dispatch(const GUI_CORE_EVENT * pstEvt_in);

GUI_PRIVATE  void gui_core_Refresh(void);
GUI_PRIVATE  void gui_core_HandleMice(void * psEvt_in);
GUI_PRIVATE  void gui_core_HandleTimer(void *p_tmr, void *p_arg);


/******************************************************************************
    Function Definition
******************************************************************************/

void gui_Init(void)
{
    OS_ERR  err = OS_ERR_NONE;

    gui_core_stCtl.stQueue.head = 0;
    gui_core_stCtl.stQueue.tail = 0;

    OSTaskCreate(
        /* p_tcb       */ (OS_TCB *)(&(gui_core_stCtl.stTcb)),
        /* p_name      */ _GUI_CORE_TASK_NAME,
        /* p_task      */ gui_core_TaskMain,
        /* p_arg       */ 0,
        /* prio        */ _GUI_CORE_TASK_PRIO,
        /* p_stk_base  */ 0,
        /* stk_limit   */ 0,
        /* stk_size    */ 0,
        /* q_size      */ _GUI_CORE_MSG_QTY,
        /* time_quanta */ 0u,
        /* p_ext       */ 0,
        /* opt         */ (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
        /* p_err       */ &err
    );

    if (OS_ERR_NONE != err) {
        drv_disp_Printf("[gui_Init][Task init Failed]\r\n");
        //return;
    }
}

GUI_PRIVATE  void gui_core_TaskMain(void* pParam_in)
{
    OS_ERR              err = OS_ERR_NONE;
    void*               pMsg = 0;
    OS_MSG_SIZE         size = 0;
    GUI_CORE_EVENT*     pstEvent = 0;

    gui_log_Init();
    gui_bg_Init();
    gui_mice_Init();

    drv_gfx_Refresh();

    drv_mice_RegisterHandler(gui_core_HandleMice);

    OSTmrCreate(
        /* p_tmr          */ & (gui_core_stCtl.stTmr),
        /* p_name         */ _GUI_CORE_TIMER_NAME,
        /* dly            */ _GUI_CORE_TIMER_1S,
        /* period         */ _GUI_CORE_TIMER_1S,
        /* opt            */ OS_OPT_TMR_PERIODIC,
        /* p_callback     */ gui_core_HandleTimer,
        /* p_callback_arg */ 0,
        /* p_err          */ &err
    );
    //if (OS_ERR_NONE != err) {
    //	drv_disp_Printf("[gui_core_TaskMain][Timer init failed]\r\n");
    //}
    OSTmrStart(
        /* p_tmr */ & (gui_core_stCtl.stTmr),
        /* p_err */ &err
    );

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
                ||  (sizeof(GUI_CORE_EVENT) != size)) {
            continue;
        }

        pstEvent = (GUI_CORE_EVENT *)pMsg;
        gui_core_Dispatch(pstEvent);

        drv_gfx_Refresh();
    }

    return;
}

GUI_PRIVATE  void gui_core_Dispatch(const GUI_CORE_EVENT * pstEvt_in)
{
    if (0 == pstEvt_in) {
        return;
    }

    switch (pstEvt_in->uiType) {
    case CPU_CORE_EVT_MOUSE:
        gui_mice_Handler(&(pstEvt_in->unData.stMice));
        //gui_bg_Time();
        break;

    case CPU_CORE_EVT_TIMER:
        gui_log_Update();
        gui_bg_Time();
        break;

    default:
        // EMPTY
        break;
    }
}


GUI_PRIVATE  void gui_core_Refresh(void)
{
    OS_ERR           err = OS_ERR_NONE;
    GUI_CORE_EVENT*  pstMsg = 0;

    _Q_GET(gui_core_stCtl.stQueue, _GUI_CORE_MSG_QTY, pstMsg);
    pstMsg->uiType = CPU_CORE_EVT_REFRESH;

    OSTaskQPost(
        /* p_tcb    */ & (gui_core_stCtl.stTcb),
        /* p_void   */ (void *)pstMsg,
        /* msg_size */ sizeof(GUI_CORE_EVENT),
        /* opt      */ OS_OPT_POST_FIFO,
        /* p_err    */ &err
    );
}

GUI_PRIVATE  void gui_core_HandleMice(void * psEvt_in)
{
    OS_ERR           err = OS_ERR_NONE;
    GUI_CORE_EVENT*  pstMsg = 0;
    DRV_MICE_EVENT*  pstEvt = (DRV_MICE_EVENT*)psEvt_in;

    if (0 == pstEvt) {
        return;
    }

    _Q_GET(gui_core_stCtl.stQueue, _GUI_CORE_MSG_QTY, pstMsg);
    pstMsg->uiType = CPU_CORE_EVT_MOUSE;
    pstMsg->unData.stMice = (*pstEvt);

    OSTaskQPost(
        /* p_tcb    */ & (gui_core_stCtl.stTcb),
        /* p_void   */ (void *)pstMsg,
        /* msg_size */ sizeof(GUI_CORE_EVENT),
        /* opt      */ OS_OPT_POST_FIFO,
        /* p_err    */ &err
    );
}

GUI_PRIVATE  void gui_core_HandleTimer(void *p_tmr, void *p_arg)
{
    OS_ERR           err = OS_ERR_NONE;
    GUI_CORE_EVENT*  pstMsg = 0;

    _Q_GET(gui_core_stCtl.stQueue, _GUI_CORE_MSG_QTY, pstMsg);
    pstMsg->uiType = CPU_CORE_EVT_TIMER;

    OSTaskQPost(
        /* p_tcb    */ & (gui_core_stCtl.stTcb),
        /* p_void   */ (void *)pstMsg,
        /* msg_size */ sizeof(GUI_CORE_EVENT),
        /* opt      */ OS_OPT_POST_FIFO,
        /* p_err    */ &err
    );
}

#else  // (CPU_EXT_DISP_MODE != CPU_EXT_DISP_MODE_TEXT)

void gui_Init(void)
{
    return;
}

#endif // (CPU_EXT_DISP_MODE != CPU_EXT_DISP_MODE_TEXT)

