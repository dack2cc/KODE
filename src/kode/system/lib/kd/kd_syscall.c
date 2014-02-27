
/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>
#include <kd/kdext.h>
#include <kd_def.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/


/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

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

/* Time functions */
CPU_EXT_DEFINE_KERNEL_FNCT_0(KDust, kdGetTimeUST);

KDtime kdTime(KDtime * timep)
{
	CPU_DATA  __res = 0;
	
	__asm__ volatile ( \
		"int $0x80" \
		: "=a" (__res) \
		: "0" (__KF_kdTime), \
		  "b" ((CPU_DATA)(timep))
	);
	
	return ((KDtime)(__res));
}

