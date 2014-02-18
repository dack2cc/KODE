
/******************************************************************************
    Include
******************************************************************************/

#include <kd/kdext.h>
#include <kd_core.h>
#include <kd_thread.h>
#include <os.h>
#include <cpu_ext.h>
#include <lib_def.h>
#include <drv_key.h>
#include <drv_blk.h>
#include <drv_hd.h>
#include <fs.h>

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

KD_PRIVATE KDint     kd_core_ErrorCode = 0;

enum {
	KD_KERNEL_FNCT_INVALID = -1,
	__KF_kdextRun,
	__KF_kdextSetup,
	__KF_kdGetError,
	__KF_kdSetError,
	/* Assertions and logging */
	__KF_kdLogMessage,
	__KF_kdHandleAssertion,
	/* Versioning and attribute queries */
	__KF_kdQueryAttribi,
	__KF_kdQueryAttribcv,
	__KF_kdQueryIndexedAttribcv,
	/* Threads and synchronization */
	__KF_kdThreadAttrCreate,
	__KF_kdThreadAttrFree,
	__KF_kdThreadAttrSetDetachState,
	__KF_kdThreadAttrSetStackSize,
	__KF_kdextThreadAttrSetPriority,
	__KF_kdThreadCreate,
	__KF_kdThreadExit,
	__KF_kdThreadJoin,
	__KF_kdThreadDetach,
	__KF_kdThreadMutexCreate,
	__KF_kdThreadMutexFree,
	__KF_kdThreadMutexLock,
	__KF_kdThreadMutexUnlock,
	__KF_kdThreadCondCreate,
	__KF_kdThreadCondFree,
	__KF_kdThreadCondSignal,
	__KF_kdThreadCondBroadcast,
	__KF_kdThreadCondWait,
	__KF_kdThreadSemCreate,
	__KF_kdThreadSemFree,
	__KF_kdThreadSemWait,
	__KF_kdThreadSemPost,
	KD_KERNEL_FNCT_MAX
};


CPU_EXT_DEFINE_KERNEL_FNCT_0(void,  kdextRun);
CPU_EXT_DEFINE_KERNEL_FNCT_0(void,  kdextSetup);
CPU_EXT_DEFINE_KERNEL_FNCT_0(KDint, kdGetError);
CPU_EXT_DEFINE_KERNEL_FNCT_1(void,  kdSetError, KDint, error);

/* Assertions and logging */
#ifndef KD_NDEBUG
CPU_EXT_DEFINE_KERNEL_FNCT_1(void,  kdLogMessage, const KDchar *, string);
#endif /* KD_NDEBUG */
CPU_EXT_DEFINE_KERNEL_FNCT_3(void,  kdHandleAssertion, const KDchar *, condition, const KDchar *, filename, KDint, linenumber);

/* Versioning and attribute queries */
CPU_EXT_DEFINE_KERNEL_FNCT_2(KDint, kdQueryAttribi, KDint, attribute, KDint *, value);
CPU_EXT_DEFINE_KERNEL_FNCT_1(const KDchar *, kdQueryAttribcv, KDint, attribute);
CPU_EXT_DEFINE_KERNEL_FNCT_2(const KDchar *, kdQueryIndexedAttribcv, KDint, attribute, KDint, index);

/* Threads and synchronization */
CPU_EXT_DEFINE_KERNEL_FNCT_2(KDint, kdextThreadAttrSetPriority, KDThreadAttr *, attr, KDint, priority);
CPU_EXT_DEFINE_KERNEL_FNCT_0(KDThreadAttr *, kdThreadAttrCreate);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadAttrFree, KDThreadAttr *, attr);
CPU_EXT_DEFINE_KERNEL_FNCT_2(KDint, kdThreadAttrSetDetachState, KDThreadAttr *, attr, KDint, detachstate);
CPU_EXT_DEFINE_KERNEL_FNCT_2(KDint, kdThreadAttrSetStackSize, KDThreadAttr *, attr, KDsize, stacksize);
CPU_EXT_DEFINE_KERNEL_FNCT_1(void,  kdThreadExit, void *, retval);
CPU_EXT_DEFINE_KERNEL_FNCT_2(KDint, kdThreadJoin, KDThread *, thread, void **, retval);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadDetach, KDThread *, thread);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDThreadMutex*, kdThreadMutexCreate, const void *, mutexattr);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadMutexFree, KDThreadMutex *, mutex);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadMutexLock, KDThreadMutex *, mutex);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadMutexUnlock, KDThreadMutex *, mutex);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDThreadCond *, kdThreadCondCreate, const void *, attr);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadCondFree, KDThreadCond *, cond);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadCondSignal, KDThreadCond *, cond);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadCondBroadcast, KDThreadCond *, cond);
CPU_EXT_DEFINE_KERNEL_FNCT_2(KDint, kdThreadCondWait, KDThreadCond *, cond, KDThreadMutex *, mutex);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDThreadSem *,  kdThreadSemCreate, KDuint, value);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadSemFree, KDThreadSem *, sem);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadSemWait, KDThreadSem *, sem);
CPU_EXT_DEFINE_KERNEL_FNCT_1(KDint, kdThreadSemPost, KDThreadSem *, sem);


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
	
	drv_hd_Init();
	drv_blk_Init();
	drv_key_Init();
	
	drv_hd_Setup();
	
	//CPUExt_HDGetRootDevice(&uiRootDev);
	//FS_super_MountRoot(0x300);
}

KDThread * kdThreadCreate(const KDThreadAttr *attr, void *(*start_routine)(void *), void *arg)
{
	CPU_DATA  __res = 0;
	
	__asm__ volatile ( \
		"int $0x80" \
		: "=a" (__res) \
		: "0" (__KF_kdThreadCreate), \
		  "b" ((CPU_DATA)(attr)), \
		  "c" ((CPU_DATA)(start_routine)), \
		  "d" ((CPU_DATA)(arg)) \
	);
	
	return ((KDThread *)(__res));
}

void  kd_core_LogMessage(const KDchar* pszString_in)
{
	CPUExt_DispPrint(pszString_in);
}

void kd_core_Assert(const KDchar *condition, const KDchar *filename, KDint linenumber)
{
	/* make the message of assert */
	
	
	/* panic to stop the os */
	CPUExt_CorePanic(condition);
}

KD_PRIVATE KDint kd_core_GetError(void)
{
	return (kd_core_ErrorCode);
}

void  kd_core_SetError(KDint error)
{
	kd_core_ErrorCode = error;
}

KD_PRIVATE KDint kd_core_QueryAttribi(KDint attribute, KDint *value)
{
	if (KD_NULL == value) {
		kd_core_ErrorCode = KD_EACCES;
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
		kd_core_ErrorCode = KD_EINVAL;
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

	kd_core_ErrorCode = KD_EINVAL;
	return (KD_NULL);
}

KD_PRIVATE const KDchar * kd_core_QueryIndexedAttribcv(KDint attribute, KDint index)
{
	kd_core_ErrorCode = KD_EINVAL;
	return (KD_NULL);
}


