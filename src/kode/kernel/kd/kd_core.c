
/******************************************************************************
    Include
******************************************************************************/

#include <kd_core.h>
#include <kd_thread.h>
#include <kd_event.h>
#include <kd_time.h>
#include <kd_file.h>
#include <kd_proc.h>
#include <kd_def.h>
#include <cpu_ext.h>
#include <os.h>
#include <lib_def.h>
#include <lib_mem.h>
#include <lib_pool.h>
#include <drv_disp.h>
#include <drv_mice.h>
#include <drv_key.h>
#include <drv_gfx.h>
#include <drv_blk.h>
#include <drv_hd.h>
#include <drv_rd.h>
#include <fs.h>
#include <gui.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define KD_PRIVATE  static
#define KD_PRIVATE

KD_PRIVATE KDint     kd_core_iErrorCode = 0;
KD_PRIVATE KDchar    kd_core_aszMsgBuf[KD_MSG_BUF_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/

KD_PRIVATE void  kd_core_Run(void);
KD_PRIVATE void  kd_core_Setup(void);
KD_PRIVATE KDint kd_core_GetError(void);
KD_PRIVATE void  kd_core_Assert(const KDchar* pszCondition_in, const KDchar* pszFilename_in, KDint iLineNumber_in);

/******************************************************************************
    Function Definition
******************************************************************************/

KD_API void KD_APIENTRY
kdextInit(void)
{
    OS_ERR err = OS_ERR_NONE;

    /* Extension */
    CPUExt_GateRegisterKernelFnct(__KF_kdextRun,           (CPU_FNCT_VOID)(kd_core_Run));
    CPUExt_GateRegisterKernelFnct(__KF_kdextSetup,         (CPU_FNCT_VOID)(kd_core_Setup));
    CPUExt_GateRegisterKernelFnct(__KF_kdextProcessCreate, (CPU_FNCT_VOID)(kd_proc_Create));
    CPUExt_GateRegisterKernelFnct(__KF_kdextProcessExit,   (CPU_FNCT_VOID)(kd_proc_Exit));

    /* Error */
    CPUExt_GateRegisterKernelFnct(__KF_kdGetError,  (CPU_FNCT_VOID)(kd_core_GetError));
    CPUExt_GateRegisterKernelFnct(__KF_kdSetError,  (CPU_FNCT_VOID)(kd_core_SetError));

    /* Assertions and logging */
    CPUExt_GateRegisterKernelFnct(__KF_kdLogMessage,       (CPU_FNCT_VOID)(kd_core_LogMessage));
    CPUExt_GateRegisterKernelFnct(__KF_kdHandleAssertion,  (CPU_FNCT_VOID)(kd_core_Assert));

    /* Threads and synchronization */
    //CPUExt_GateRegisterKernelFnct(__KF_kdextThreadAttrSetPriority, (CPU_FNCT_VOID)(kd_thread_AttrSetPriority));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadAttrCreate,         (CPU_FNCT_VOID)(kd_thread_AttrCreate));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadAttrFree,           (CPU_FNCT_VOID)(kd_thread_AttrFree));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadAttrSetDetachState, (CPU_FNCT_VOID)(kd_thread_AttrSetDetachState));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadAttrSetStackSize,   (CPU_FNCT_VOID)(kd_thread_AttrSetStackSize));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadCreate,             (CPU_FNCT_VOID)(kd_thread_Create));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadExit,               (CPU_FNCT_VOID)(kd_thread_Exit));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadJoin,               (CPU_FNCT_VOID)(kd_thread_Join));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadDetach,             (CPU_FNCT_VOID)(kd_thread_Detach));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadMutexCreate,        (CPU_FNCT_VOID)(kd_thread_MutexCreate));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadMutexFree,          (CPU_FNCT_VOID)(kd_thread_MutexFree));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadMutexLock,          (CPU_FNCT_VOID)(kd_thread_MutexLock));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadMutexUnlock,        (CPU_FNCT_VOID)(kd_thread_MutexUnlock));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadCondCreate,         (CPU_FNCT_VOID)(kd_thread_CondCreate));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadCondFree,           (CPU_FNCT_VOID)(kd_thread_CondFree));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadCondSignal,         (CPU_FNCT_VOID)(kd_thread_CondSignal));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadCondBroadcast,      (CPU_FNCT_VOID)(kd_thread_CondBroadcast));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadCondWait,           (CPU_FNCT_VOID)(kd_thread_CondWait));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadSemCreate,          (CPU_FNCT_VOID)(kd_thread_SemCreate));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadSemFree,            (CPU_FNCT_VOID)(kd_thread_SemFree));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadSemWait,            (CPU_FNCT_VOID)(kd_thread_SemWait));
    CPUExt_GateRegisterKernelFnct(__KF_kdThreadSemPost,            (CPU_FNCT_VOID)(kd_thread_SemPost));

    /* Event */
    CPUExt_GateRegisterKernelFnct(__KF_kdWaitEvent,  (CPU_FNCT_VOID)(kd_event_Wait));

    /* Time functions */
    CPUExt_GateRegisterKernelFnct(__KF_kdGetTimeUST, (CPU_FNCT_VOID)(kd_time_GetUST));
    CPUExt_GateRegisterKernelFnct(__KF_kdTime,       (CPU_FNCT_VOID)(kd_time_GetWallClock));

    /* File System */
    CPUExt_GateRegisterKernelFnct(__KF_kdFopen, (CPU_FNCT_VOID)(kd_file_Open));

    /* => we are in initialize process of Ring 0 */
    OSInit(&err);
    /* <= we are in Task 0 of Ring 3 */

    {
        CPU_ADDR adrStart = 0;
        CPU_ADDR adrEnd   = 0;

        CPUExt_PageGetExtendSpace(&adrStart, &adrEnd);
        lib_pool_Init(adrStart, adrEnd);
        //drv_disp_Printf("[ExtendMem][0x%X : %d KB] \r\n", adrStart, (adrEnd - adrStart)/1024);
    }

    return;
}

KD_PRIVATE void kd_core_Run(void)
{
    OS_ERR err = OS_ERR_NONE;

    OSStart(&err);
}

KD_PRIVATE void  kd_core_Setup(void)
{
    drv_gfx_Init();
    drv_disp_Init();
    drv_mice_Init();
    drv_key_Init();


    drv_hd_Init();
    drv_rd_Init();
    drv_blk_Init();

    drv_hd_Setup();
    drv_rd_Setup();

    {
        //CPU_INT16U  uiRootDev = 0;
        //CPUExt_HDGetRootDevice(&uiRootDev);
        FS_MountRoot(0x101);
    }

    gui_Init();
}

void  kd_core_LogMessage(const KDchar* pszString_in)
{
    if (0 == pszString_in) {
        return;
    }

    kd_core_StrReadUserSpace(pszString_in, kd_core_aszMsgBuf, sizeof(kd_core_aszMsgBuf));

    drv_disp_Printf(kd_core_aszMsgBuf);
}

KD_PRIVATE void kd_core_Assert(const KDchar *condition, const KDchar *filename, KDint linenumber)
{
    drv_disp_Printf("[");
    kd_core_LogMessage(condition);
    drv_disp_Printf("]");
    drv_disp_Printf("[");
    kd_core_LogMessage(filename);
    drv_disp_Printf("]");
    drv_disp_Printf("[%d]", linenumber);

    /* panic to stop the os */
    CPUExt_CorePanic("");
}

KD_PRIVATE KDint kd_core_GetError(void)
{
    return (kd_core_iErrorCode);
}

void  kd_core_SetError(KDint error)
{
    kd_core_iErrorCode = error;
}

void  kd_core_StrReadUserSpace(const KDchar * pszSrcUsr_in, KDchar * pszDstKnl_out, const KDint iDstSize_in)
{
    KDint i = 0;

    if ((0 == pszSrcUsr_in)
            ||  (0 == pszDstKnl_out)
            ||  (iDstSize_in <= 0)) {
        return;
    }

    for (i = 0; i < iDstSize_in; ++i) {
        CPU_EXT_GET_FS_BYTE(pszDstKnl_out[i], pszSrcUsr_in + i);

        if ('\0' == pszDstKnl_out[i]) {
            break;
        }
    }

    if (i >= iDstSize_in) {
        pszDstKnl_out[iDstSize_in - 1] = '\0';
    }
}


