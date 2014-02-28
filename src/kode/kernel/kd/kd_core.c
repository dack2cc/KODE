
/******************************************************************************
    Include
******************************************************************************/

#include <kd_core.h>
#include <kd_thread.h>
#include <kd_time.h>
#include <kd_file.h>
#include <kd_def.h>
#include <lib_def.h>
#include <lib_mem.h>
#include <drv_disp.h>
#include <drv_key.h>
#include <drv_blk.h>
#include <drv_hd.h>
#include <drv_rd.h>
#include <fs.h>
#include <os.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define KD_PRIVATE  static
//#define KD_PRIVATE 

KD_PRIVATE const KDuint32  kd_core_VenderVal   = CPU_TYPE_CREATE('K','K','D','X');
KD_PRIVATE const KDchar*   kd_core_VenderStr   = "kokoDo";
KD_PRIVATE const KDuint32  kd_core_VersionVal  = CPU_TYPE_CREATE('0', '0', '0', '1');
KD_PRIVATE const KDchar*   kd_core_VersionStr  = "00.01";
KD_PRIVATE const KDuint32  kd_core_PlatformVal = CPU_TYPE_CREATE('X', '8', '6', ' ');
KD_PRIVATE const KDchar*   kd_core_PlatformStr = "x86_32";

KD_PRIVATE KDint     kd_core_iErrorCode = 0;
KD_PRIVATE KDchar    kd_core_aszMsgBuf[KD_MSG_BUF_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/

KD_PRIVATE void  kd_core_Run(void);
KD_PRIVATE void  kd_core_Setup(void);
KD_PRIVATE KDint kd_core_GetError(void);
KD_PRIVATE KDint kd_core_QueryAttribi(KDint attribute, KDint *value);
KD_PRIVATE const KDchar * kd_core_QueryAttribcv(KDint attribute);
KD_PRIVATE const KDchar * kd_core_QueryIndexedAttribcv(KDint attribute, KDint index);

/******************************************************************************
    Function Definition
******************************************************************************/

KD_API void KD_APIENTRY  
kdextInit(void)
{
	OS_ERR err = OS_ERR_NONE;
	
	CPUExt_GateRegisterKernelFnct(__KF_kdextRun,    (CPU_FNCT_VOID)(kd_core_Run));
	CPUExt_GateRegisterKernelFnct(__KF_kdextSetup,  (CPU_FNCT_VOID)(kd_core_Setup));
	CPUExt_GateRegisterKernelFnct(__KF_kdGetError,  (CPU_FNCT_VOID)(kd_core_GetError));
	CPUExt_GateRegisterKernelFnct(__KF_kdSetError,  (CPU_FNCT_VOID)(kd_core_SetError));

	/* Assertions and logging */
	CPUExt_GateRegisterKernelFnct(__KF_kdLogMessage,       (CPU_FNCT_VOID)(kd_core_LogMessage));
	CPUExt_GateRegisterKernelFnct(__KF_kdHandleAssertion,  (CPU_FNCT_VOID)(kd_core_Assert));
	
	/* Versioning and attribute queries */
	CPUExt_GateRegisterKernelFnct(__KF_kdQueryAttribi,          (CPU_FNCT_VOID)(kd_core_QueryAttribi));
	CPUExt_GateRegisterKernelFnct(__KF_kdQueryAttribcv,         (CPU_FNCT_VOID)(kd_core_QueryAttribcv));
	CPUExt_GateRegisterKernelFnct(__KF_kdQueryIndexedAttribcv,  (CPU_FNCT_VOID)(kd_core_QueryIndexedAttribcv));
	
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
	
	/* Time functions */
	CPUExt_GateRegisterKernelFnct(__KF_kdGetTimeUST, (CPU_FNCT_VOID)(kd_time_GetUST));
	CPUExt_GateRegisterKernelFnct(__KF_kdTime,       (CPU_FNCT_VOID)(kd_time_GetWallClock));
	
	/* File System */
	CPUExt_GateRegisterKernelFnct(__KF_kdFopen, (CPU_FNCT_VOID)(kd_file_Open));
	
	/* => we are in initialize process of Ring 0 */
	OSInit(&err);
	/* <= we are in Task 0 of Ring 3 */
	
	return;
}

KD_PRIVATE void kd_core_Run(void)
{
	OS_ERR err = OS_ERR_NONE;
	
	OSStart(&err);
}

KD_PRIVATE void  kd_core_Setup(void)
{
	//CPU_INT16U  uiRootDev = 0;
	drv_disp_Init();
	drv_key_Init();
	
	drv_hd_Init();
	drv_rd_Init();	
	drv_blk_Init();
	
	drv_hd_Setup();
	drv_rd_Setup();
	
	//CPUExt_HDGetRootDevice(&uiRootDev);
	FS_MountRoot(0x101);
}

void  kd_core_LogMessage(const KDchar* pszString_in)
{
	if (0 == pszString_in) {
		return;
	}
	
	kd_core_StrReadUserSpace(pszString_in, kd_core_aszMsgBuf, sizeof(kd_core_aszMsgBuf));
	
	CPUExt_DispPrint(kd_core_aszMsgBuf);
}

void kd_core_Assert(const KDchar *condition, const KDchar *filename, KDint linenumber)
{
	CPUExt_DispPrint("[");
	kd_core_LogMessage(condition);
	CPUExt_DispPrint("]");
	CPUExt_DispPrint("[");
	kd_core_LogMessage(filename);
	CPUExt_DispPrint("]");
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

KD_PRIVATE KDint kd_core_QueryAttribi(KDint attribute, KDint *value)
{
	if (KD_NULL == value) {
		kd_core_iErrorCode = KD_EACCES;
		return (KD_EACCES);
	}
	
	switch (attribute) {
	case KD_ATTRIB_VENDOR:
		(*value) = kd_core_VenderVal;
		break;
	case KD_ATTRIB_VERSION:
		(*value) = kd_core_VersionVal;
		break;
	case KD_ATTRIB_PLATFORM:
		(*value) = kd_core_PlatformVal;
		break;
	default:
		kd_core_iErrorCode = KD_EINVAL;
		return (KD_EINVAL);		
		break;
	}
	
	return (0);
}

KD_PRIVATE const KDchar * kd_core_QueryAttribcv(KDint attribute)
{
	switch (attribute) {
	case KD_ATTRIB_VENDOR:
		return (kd_core_VenderStr);
		break;
	case KD_ATTRIB_VERSION:
		return (kd_core_VersionStr);
		break;
	case KD_ATTRIB_PLATFORM:
		return (kd_core_PlatformStr);
		break;
	default:
		/* empty */
		break;
	}

	kd_core_iErrorCode = KD_EINVAL;
	return (KD_NULL);
}

KD_PRIVATE const KDchar * kd_core_QueryIndexedAttribcv(KDint attribute, KDint index)
{
	kd_core_iErrorCode = KD_EINVAL;
	return (KD_NULL);
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


