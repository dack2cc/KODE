
/******************************************************************************
    Include
******************************************************************************/

#include <kd_thread.h>
#include <kd_core.h>
#include <os.h>
#include <os_cfg_app.h>
#include <cpu_ext.h>
#include <cpu_boot.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define _KD_PRIVATE   static

struct KDThreadAttr {
	KDint  iDetachState;
	KDsize iStackSize;
};

#define _KD_THREAD_KERNEL_STACK_SIZE    (3 * 1024)
#define _KD_THREAD_USER_STACK_SIZE      (64 * 1024 * 1024)

struct KDThread {
	OS_TCB    stTCB;
	OS_MUTEX  stMutex;
	KDint     iDetachState;
};

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

/******************************************************************************
7.3.1. kdThreadAttrCreate
Create a thread attribute object.

Synopsis
typedef struct KDThreadAttr KDThreadAttr;
KDThreadAttr *kdThreadAttrCreate(void);

Description
This function creates a thread attributes object, and returns a valid handle to it.
The new thread attributes object contains default attributes. Attributes can then be modified using
kdThreadAttrSetDetachState or kdThreadAttrSetStackSize as required, and then passed to
kdThreadCreate such that the thread creation uses the supplied attributes. A single thread attributes object can be
passed to multiple kdThreadCreate calls, even multiple simultaneous calls.

Note that it is possible to create and manipulate a thread attributes object, even when the implementation does not
support threading.

Return value
On success, the function returns the new thread attributes object. On failure, it returns KD_NULL and stores one of the
error codes listed below into the error indicator returned by kdGetError.
Error codes
KD_ENOMEM Out of memory.

Rationale
The notion of a thread attributes object is based on [POSIX]. There, a thread attributes object is represented by a struct
in user data which is initialized with pthread_attr_init.
It is mandated that a thread attributes object can be created and manipulated even when threading is not supported in
order to allow for a possible future extension (from a vendor or from Khronos) that allows co-operative, non-timeslicing
threads. For backwards compatibility with applications written to assume timeslicing threads, the extension would
only allow creation of a thread on an implementation with co-operative threads if the thread attribute object had had
a newly defined attribute set which states that the application knows about co-operative threads and has been written
to deal with them.
******************************************************************************/
KDThreadAttr * kd_thread_AttrCreate(void)
{
	KDThreadAttr *  pstAttr = KD_NULL;
	
	pstAttr = (KDThreadAttr *)lib_pool_Malloc(sizeof(KDThreadAttr));
	if (KD_NULL != pstAttr) {
		pstAttr->iDetachState = KD_THREAD_CREATE_JOINABLE;
		pstAttr->iStackSize   = 0;
	}
	else {
		kd_core_SetError(KD_ENOMEM);
	}
	
	return (pstAttr);
}

/******************************************************************************
7.3.2. kdThreadAttrFree
Free a thread attribute object.

Synopsis
KDint kdThreadAttrFree(KDThreadAttr *attr);

Description
This function frees the thread attributes object attr. Once the function has been entered, attr is no longer a valid
thread attributes object handle.
If attr is not a valid thread attributes object handle, then undefined behavior results.

Return value
On success, the function returns 0. The function has no defined failure condition.

Rationale
The notion of a thread attributes object is based on [POSIX]. There, a thread attributes object is represented by a struct
in user data which is deinitialized with pthread_attr_destroy.
******************************************************************************/
KDint kd_thread_AttrFree(KDThreadAttr * pstAttr_in)
{
	if (KD_NULL == pstAttr_in) {
		kd_core_SetError(KD_EACCES);
		return (KD_EACCES);
	}
	
	lib_pool_Free((void *)pstAttr_in, sizeof(KDThreadAttr));
	
	return (0);
}

/******************************************************************************
7.3.3. kdThreadAttrSetDetachState
Set detachstate attribute.

Synopsis
#define KD_THREAD_CREATE_JOINABLE 0
#define KD_THREAD_CREATE_DETACHED 1
KDint kdThreadAttrSetDetachState(KDThreadAttr *attr, KDint detachstate);

Description
This function sets the detachstate attribute in the thread attributes object attr to the value detachstate.
A value of KD_THREAD_CREATE_JOINABLE causes a thread created using this attributes object to be created in
the joinable state, such that its ID can be specified to kdThreadJoin or kdThreadDetach. This is the default
setting.
A value of KD_THREAD_CREATE_DETACHED causes a thread created using this attributes object to be created in
the detached state, such that its ID cannot be specified to the above functions, but resources associated with the thread
are freed as soon as the thread ends.
If detachstate is not one of the above values, an error is returned as described below.
If attr is not a valid thread attributes object handle, then undefined behavior results.

Return value
On success, the function returns 0. On failure, it returns -1 and stores one of the error codes listed below into the error
indicator returned by kdGetError.
Error codes
KD_EINVAL detachstate is not one of the values defined above.

Rationale
kdThreadAttrSetDetachState is based on [POSIX] pthread_attr_setdetachstate. The POSIX
function returns any error code, rather than returning -1 and setting the error indicator.
A thread created joinable (by using this attributeÅfs default value KD_THREAD_CREATE_JOINABLE) can still be
made detached using kdThreadDetach.
******************************************************************************/
KDint  kd_thread_AttrSetDetachState(KDThreadAttr * pstAttr_inout, KDint iDetachState_in)
{
	if (KD_NULL == pstAttr_inout) {
		kd_core_SetError(KD_EACCES);
		return (-1);
	}
	
	switch (iDetachState_in) {
	case KD_THREAD_CREATE_JOINABLE:
	case KD_THREAD_CREATE_DETACHED:
	    pstAttr_inout->iDetachState = iDetachState_in;
		break;
	default:
		kd_core_SetError(KD_EINVAL);
		return (-1);
		break;
	}
	
	return (0);
}

/******************************************************************************
7.3.4. kdThreadAttrSetStackSize
Set stacksize attribute.

Synopsis
KDint kdThreadAttrSetStackSize(KDThreadAttr *attr, KDsize stacksize);

Description
This function sets the stacksize attribute in the thread attributes object attr to the value stacksize.
When a thread is created with kdThreadCreate using this thread attributes object, the size of the new threadÅfs
stack in bytes is at least the value of the stacksize attribute.
The default value is implementation defined.
If the function is used to attempt to set a stack size larger than an implementation-defined maximum, an error is given.
If attr is not a valid thread attributes object handle, then undefined behavior results.

Return value
On success, the function returns 0. On failure, it returns -1 and stores one of the error codes listed below into the error
indicator returned by kdGetError.
Error codes
KD_EINVAL Requested stack size is greater than the implementation-defined maximum.

Rationale
kdThreadAttrSetStackSize is based on [POSIX] pthread_attr_setstacksize. The POSIX function
returns any error code, rather than returning -1 and setting the error indicator.
******************************************************************************/
KDint  kd_thread_AttrSetStackSize(KDThreadAttr * pstAttr_inout, KDsize iStackSize_in)
{
	if (KD_NULL == pstAttr_inout) {
		kd_core_SetError(KD_EACCES);
		return (-1);
	}
	
	if (iStackSize_in > _KD_THREAD_USER_STACK_SIZE) {
		kd_core_SetError(KD_EINVAL);
		return (-1);
	}
	
	pstAttr_inout->iStackSize = iStackSize_in;
	
	return (0);	
}

/******************************************************************************
7.3.5. kdThreadCreate
Create a new thread.

Synopsis
typedef struct KDThread KDThread;
KDThread *kdThreadCreate(const KDThreadAttr *attr, void *(*start_routine)(void *), void *arg);

Description
This function creates a new thread. The new thread runs the function start_routine, passing arg as its only
parameter. The thread finishes either by calling kdThreadExit passing the threadÅfs return value, or equivalently
by returning from the threadÅfs start_routine with the return value.
The attr is either a thread attributes object, or is KD_NULL. In the former case, attributes set in the thread attributes
object are applied to the thread creation. The KD_NULL case is equivalent to supplying a thread attributes object which
has been created with kdThreadAttrCreate and then not modified.
Threads run pre-emptively, that is, execution could switch from one thread to another at any time, or multiple threads
can actually run concurrently on multiple CPU cores.
The scheduling algorithm is undefined. No thread priority mechanism exists, which means there is no way of making
a thread Åghigh priorityÅh to ensure that it runs in preference to other lower priority threads.
If start_routine is not a pointer to a function taking a single void* parameter and returning void*, or attr is
not KD_NULL and is not a valid thread attributes object handle, then undefined behavior results.

Return value
On success, the function returns the new thread's ID. On failure, it returns KD_NULL and stores one of the error codes
listed below into the error indicator returned by kdGetError.
Error codes
KD_EAGAIN Not enough system resources, or maximum number of threads already active.
KD_ENOSYS Threading not supported.

Rationale
kdThreadCreate is based on [POSIX] pthread_create. The POSIX function stores the thread ID into a location
pointed to by an extra parameter and returns any error code, rather than returning the thread ID or returning KD_NULL
and setting the error indicator.
******************************************************************************/
KDThread * kd_thread_Create(
	const KDThreadAttr * pstAttr_in, void *(* pfnStartRoutine_in)(void *), void * arg_in, 
	KDuint32 reversed_0, KDuint32 reversed_1, KDuint32 reversed_2, KDuint32 reversed_3, KDuint32 reversed_4, 
	KDuint32 eflag_in
)
{
	CPU_ADDR   addrPhyPage = 0;
	KDThread*  pstThread   = KD_NULL;
	void*      pExtData    = KD_NULL;
	CPU_DATA*  pExtArg     = KD_NULL;
	OS_ERR     os_err      = OS_ERR_NONE;
	
	CPUExt_PageGetFree((&addrPhyPage));
	if (0 == addrPhyPage) {
		kd_core_SetError(KD_EAGAIN);
		return (KD_NULL);		
	}
	pstThread = (KDThread *)addrPhyPage;
	pExtData  = (void *)((CPU_INT32U)addrPhyPage + sizeof(KDThread));
	pExtArg   = (CPU_DATA *)pExtData;
	
	pExtArg[OS_TCB_EXT_EFLAG] = eflag_in;
	pExtArg[OS_TCB_EXT_RET_POINT] = (CPU_DATA)(&kdThreadExit);
	
	OSTaskCreate(
		/* p_tcb       */   &(pstThread->stTCB),
		/* p_name      */   0,
		/* p_task      */   (OS_TASK_PTR)pfnStartRoutine_in,
		/* p_arg       */   arg_in,
		/* prio        */   50,
		/* p_stk_base  */   (CPU_STK *)((CPU_INT32U)addrPhyPage + (X86_MEM_PAGE_SIZE - _KD_THREAD_KERNEL_STACK_SIZE)), 
		/* stk_limit   */   ((_KD_THREAD_KERNEL_STACK_SIZE * OS_CFG_TASK_STK_LIMIT_PCT_EMPTY) / 100),
		/* stk_size    */   _KD_THREAD_KERNEL_STACK_SIZE,
		/* q_size      */   0,
		/* time_quanta */   0,
		/* p_ext       */   pExtData,
		/* opt         */   0, 
		/* p_err       */   &os_err
	);
	
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(os_err);
		CPUExt_PageRelease(addrPhyPage);
		addrPhyPage = 0;
		return (KD_NULL);
	}
	
	/* lock the thread joinable */
	pstThread->iDetachState = pstAttr_in->iDetachState;
	if ((KD_NULL != pstAttr_in) 
	&&  (KD_THREAD_CREATE_JOINABLE == pstAttr_in->iDetachState)) {
		/* create the mutex */
		OSMutexCreate
		(
			/* p_mutex */ &(pstThread->stMutex), 
			/* p_name  */ 0,
			/* p_err   */ &os_err
		);
		if (OS_ERR_NONE != os_err) {
			kd_core_SetError(KD_EBADF);
		}

		/* lock the mutex by hand */
		pstThread->stMutex.OwnerTCBPtr = &(pstThread->stTCB);
		pstThread->stMutex.OwnerOriginalPrio = pstThread->stTCB.Prio;
		pstThread->stMutex.OwnerNestingCtr = (OS_NESTING_CTR)1;
	}
	
	return (pstThread);
}

/******************************************************************************
7.3.6. kdThreadExit
Terminate this thread.

Synopsis
KD_NORETURN void kdThreadExit(void *retval);

Description
This function causes the calling thread to exit, with retval as the return value.
When called from the main thread (the thread in which the application started in kdMain), kdThreadExit acts as
kdExit with an exit code of 0.
This function works as specified even when the implementation does not support threads.
If a thread calls kdThreadExit while it has a mutex locked, it is undefined whether the mutex remains locked.

Rationale
kdThreadExit is based on [POSIX] pthread_exit. However pthread_exit does not treat the main thread
specially.
******************************************************************************/
void   kd_thread_Exit(void * pRetval_in)
{
	KDThread * pstThread = kd_thread_Self();
	OS_ERR  os_err = OS_ERR_NONE;
	
	OSMutexDel(&(pstThread->stMutex), OS_OPT_DEL_ALWAYS, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EBADF);
	}
	
	OS_TaskReturn();
}

/******************************************************************************
7.3.7. kdThreadJoin
Wait for termination of another thread.

Synopsis
KDint kdThreadJoin(KDThread *thread, void **retval);

Description
This function blocks the calling thread until the specified thread terminates. If the specified thread has already
terminated, blocking does not occur.
If retval is not KD_NULL, the specified threadÅfs return value is stored in the void* location it points at.
The specified thread must be in the joinable state. On successful return, the thread and any associated resources are
freed, and thread is no longer a valid thread ID.
The results of multiple simultaneous calls to kdThreadJoin specifying the same target thread are undefined.
If thread is not a valid thread ID, or is the main thread, or retval is not KD_NULL and is not a pointer to a writable
void* location, then undefined behavior results.

Return value
On success, the function returns 0, stores the specified threadÅfs return value into the location pointed to by retval
(if retval is not KD_NULL), and frees all resources associated with the specified thread. On failure, the function
returns -1 and stores one of the error codes listed below into the error indicator returned by kdGetError.
Error codes
KD_EDEADLK thread is the current thread.
KD_EINVAL thread has been detached.

Rationale
kdThreadJoin is based on [POSIX] pthread_join. The POSIX function returns any error code, rather than
returning -1 and setting the error indicator.
However, kdThreadJoin has undefined behavior rather than an error when given an invalid thread ID.
******************************************************************************/
KDint kd_thread_Join(KDThread * pstThread_in, void ** ppRetval_out)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstThread_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	if ((OS_STATE)OS_TASK_STATE_DEL == (pstThread_in->stTCB.TaskState)) {
		return (0);
	}
	
	if (KD_THREAD_CREATE_JOINABLE == (pstThread_in->iDetachState)) {
		OSMutexPend
		(
			/* p_mutex */ &(pstThread_in->stMutex),
			/* timeout */ 0,
			/* opt     */ 0,
			/* p_ts    */ 0,
			/* p_err   */ &os_err
		);
		if (OS_ERR_NONE != os_err) {
			kd_core_SetError(KD_EINVAL);
		}
	}
	
	return (0);
}

/******************************************************************************
7.3.8. kdThreadDetach
Allow resources to be freed as soon as a thread terminates.

Synopsis
KDint kdThreadDetach(KDThread *thread);

Description
This function puts the specified thread into detached state, and thus no longer in joinable state. This means that
resources associated with the specified thread will be freed as soon as the thread terminates (or immediately if the
thread has already terminated), and at that point thread will no longer be a valid thread ID.
When detached, the thread cannot be the subject of a call to kdThreadJoin to wait for it to terminate and retrieve
its return value.
If another thread is already in a kdThreadJoin waiting for the specified thread to terminate, then the call to
kdThreadDetach causes undefined behavior.
If thread is not a valid thread ID, or is the main thread, then undefined behavior results.

Return value
On success, the function returns 0. On failure, it returns -1 and stores one of the error codes listed below into the error
indicator returned by kdGetError.
Error codes
KD_EINVAL thread is already detached.

Rationale
kdThreadDetach is based on [POSIX] pthread_detach. The POSIX function returns any error code, rather
than returning -1 and setting the error indicator.
However, kdThreadDetach has undefined behavior rather than an error when given an invalid thread ID.
******************************************************************************/
KDint  kd_thread_Detach(KDThread * pstThread_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstThread_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	if ((OS_STATE)OS_TASK_STATE_DEL == (pstThread_in->stTCB.TaskState)) {
		kd_core_SetError(KD_EINVAL);
		return (-1);
	}
	
	if (KD_THREAD_CREATE_DETACHED == (pstThread_in->iDetachState)) {
		kd_core_SetError(KD_EINVAL);
		return (-1);
	}
	
	pstThread_in->iDetachState = KD_THREAD_CREATE_DETACHED;
	OSMutexDel(&(pstThread_in->stMutex), OS_OPT_DEL_ALWAYS, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EBADF);
		return (-1);
	}
	
	return (0);
}

/******************************************************************************
7.3.9. kdThreadSelf
Return calling threadÅfs ID.

Synopsis
KDThread *kdThreadSelf(void);

Description
This function simply returns the thread ID of the calling thread.
The applicationÅfs main thread has a thread ID, even though it was not created by kdThreadCreate. Using
kdThreadSelf from the main thread returns this thread ID.

Return value
On success, the function returns the thread ID of the calling thread. The function never fails. Even on an OpenKODE
implementation that does not support threads, this function returns a thread ID that can be used in
kdPostThreadEvent.

Rationale
kdThreadSelf is based on [POSIX] pthread_self.
******************************************************************************/
KDThread * kd_thread_Self(void)
{
	return ((KDThread *)OSTCBCurPtr);
}

/*******************************************************************************
7.3.11. kdThreadMutexCreate
Create a mutex.

Synopsis
typedef struct KDThreadMutex KDThreadMutex;
KDThreadMutex *kdThreadMutexCreate(const void *mutexattr);

Description
This function creates a mutex, returning a valid handle to it. The new mutex is initially unlocked.
mutexattr must be KD_NULL.
Mutexes work on an OpenKODE Core implementation that does not support threading.
If mutexattr is not KD_NULL, then undefined behavior results.

Return value
On success, the function returns the mutex handle. On failure, it returns KD_NULL and stores one of the error codes
listed below into the error indicator returned by kdGetError.
Error codes
KD_EAGAIN Not enough resources (other than memory).
KD_ENOMEM Out of memory.

Rationale
The mutex created by kdThreadMutexCreate is based on [POSIX] mutexes. POSIX has the mutex represented
by a structure in user data which is initialized with pthread_mutex_init, or with a static initializer. OpenKODE
Core has no support for mutex attributes, although mutexattr has been included to allow a future extension to add
mutex attributes compatibly.
*******************************************************************************/
KDThreadMutex* kd_thread_MutexCreate(const void * pMutexAttr_in)
{
	OS_MUTEX* pstMutex = KD_NULL;
	OS_ERR    os_err   = OS_ERR_NONE;
	
	pstMutex = (OS_MUTEX *)lib_pool_Malloc(sizeof(OS_MUTEX));
	if (KD_NULL == pstMutex) {
		kd_core_SetError(KD_ENOMEM);
		return (KD_NULL);
	}
	
	/* create the mutex */
	OSMutexCreate
	(
		/* p_mutex */ pstMutex, 
		/* p_name  */ 0,
		/* p_err   */ &os_err
	);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EAGAIN);
	}	
	
	return ((KDThreadMutex *)(pstMutex));
}

/*******************************************************************************
7.3.12. kdThreadMutexFree
Free a mutex.

Synopsis
KDint kdThreadMutexFree(KDThreadMutex *mutex);

Description
This function frees the mutex. The mutex must be unlocked when destroyed. Once the function has been entered,
mutex is no longer a valid mutex handle.
If mutex is not a valid mutex handle, or the mutex is currently locked by some thread, then undefined behavior results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
[POSIX] has a mutex represented by a structure in user data which is deinitialized with pthread_mutex_destroy.
*******************************************************************************/
KDint kd_thread_MutexFree(KDThreadMutex * pstMutex_in)
{
	OS_ERR  os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstMutex_in) {
	    kd_core_SetError(KD_EINVAL);
		return (-1);
	}
	
	OSMutexDel((OS_MUTEX *)(pstMutex_in), OS_OPT_DEL_ALWAYS, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EBADF);
	}
	
	lib_pool_Free((void *)(pstMutex_in), sizeof(OS_MUTEX));
	
	return (0);
}

/*******************************************************************************
7.3.13. kdThreadMutexLock
Lock a mutex.

Synopsis
KDint kdThreadMutexLock(KDThreadMutex *mutex);

Description
kdThreadMutexLock locks the specified mutex for the calling thread. Only one thread can lock the mutex at a
time; if it is currently locked by another thread, the function blocks until the mutex is unlocked and it can acquire the
lock for the calling thread.
If any other thread is blocked in kdThreadMutexLock waiting for the same mutex, it is undefined which thread
acquires the mutex first.
Mutexes work on an OpenKODE Core implementation that does not support threading.
If the mutex is already locked by the calling thread, then undefined behavior results.
If mutex is not a valid mutex handle, then undefined behavior results.

Return value
On success, the function returns 0. The function has no defined failure case.

Rationale
kdThreadMutexLock is based on [POSIX] pthread_mutex_lock. The POSIX function returns any error code,
rather than returning -1 and setting the error indicator. OpenKODE Core has no equivalent of POSIXÅfs
PTHREAD_MUTEX_INITIALIZER.
The undefined behavior on an attempt to lock a mutex that the thread already has locked is intended to allow easy and
efficient implementation on OSes with differing mutex behaviors, such as recursive, non-recursive but allowed to
re-lock, non-recursive and gives an error, and non-recursive and blocks. This presents a portability problem where an
application could accidentally be written to assume a particular platformÅfs behavior. Therefore it is recommended that
any ÅgdebugÅh OpenKODE Core implementation should diagnose an attempt to re-lock an already locked mutex in a
way that stops the application working, such as terminating it.
A draft of this specification contained kdThreadMutexTryLock. It was removed with the justification that some
popular platforms do not support it directly, and it is more difficult for the OpenKODE implementation to implement
it generally, with the interaction between mutexes and condition variables, than it is for an application that requires
them but not in conjunction with condition variables to implement the functionality itself.
*******************************************************************************/
KDint  kd_thread_MutexLock(KDThreadMutex * pstMutex_in)
{
	OS_ERR os_err = OS_ERR_NONE;

	if (KD_NULL == pstMutex_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}

	OSMutexPend
	(
		/* p_mutex */ (OS_MUTEX *)pstMutex_in,
		/* timeout */ 0,
		/* opt     */ 0,
		/* p_ts    */ 0,
		/* p_err   */ &os_err
	);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}

	return (0);
}

/*******************************************************************************
7.3.14. kdThreadMutexUnlock
Unlock a mutex.

Synopsis
KDint kdThreadMutexUnlock(KDThreadMutex *mutex);

Description
This function unlocks the specified mutex for the calling thread. If any other thread is blocked in
kdThreadMutexLock on this mutex, then exactly one of those threads acquires the mutex and successfully returns
from kdThreadMutexLock. If multiple threads were waiting, it is undefined which thread acquires the mutex lock.
Mutexes work on an OpenKODE Core implementation that does not support threading.
If the mutex was locked but not by the calling thread, or was already unlocked, then undefined behavior results.
If mutex is not a valid mutex handle, then undefined behavior results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
kdThreadMutexUnlock is based on [POSIX] pthread_mutex_unlock.
A debugging OpenKODE implementation may want to diagnose the cases of unlocking an already-unlocked mutex
and unlocking a mutex locked by another thread, perhaps causing the application to terminate with an error message.
*******************************************************************************/
KDint  kd_thread_MutexUnlock(KDThreadMutex * pstMutex_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstMutex_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSMutexPost((OS_MUTEX *)pstMutex_in, 0, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}
	
	return (0);
}

/*******************************************************************************
7.3.15. kdThreadCondCreate
Create a condition variable.

Synopsis
typedef struct KDThreadCond KDThreadCond;
KDThreadCond *kdThreadCondCreate(const void *attr);

Description
This function creates a condition variable, returning a valid handle to it.
If attr is not KD_NULL, then undefined behavior results.

Return value
On success, the function returns the handle to the new condition variable. On failure, it returns KD_NULL and stores
one of the error codes listed below into the error indicator returned by kdGetError.
Error codes
KD_EAGAIN Not enough resources (other than memory).
KD_ENOMEM Out of memory.
KD_ENOSYS Threading not supported.

Rationale
The condition variable created by kdThreadCondCreate is based on [POSIX] condition variables. POSIX has the
condition variable represented by a structure in user data which is initialized with pthread_cond_init, or with
a static initializer. OpenKODE Core has no support for condition variable attributes, although attr has been included
to allow a future extension to add such attributes.
*******************************************************************************/
KDThreadCond * kd_thread_CondCreate(const void * pAttr_in)
{
	OS_FLAG_GRP*  pstFlagGrp = KD_NULL;
	OS_ERR  os_err = OS_ERR_NONE;
	
	pstFlagGrp = (OS_FLAG_GRP *)lib_pool_Malloc(sizeof(OS_FLAG_GRP));
	if (KD_NULL == pstFlagGrp) {
	    kd_core_SetError(KD_EINVAL);
	    return (KD_NULL);
	}
	
	OSFlagCreate
	(
		pstFlagGrp,
		0,
		0,
		&os_err
	);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_ENOSYS);
	}
	
	return ((KDThreadCond *)pstFlagGrp);
}

/*******************************************************************************
7.3.16. kdThreadCondFree
Free a condition variable.

Synopsis
KDint kdThreadCondFree(KDThreadCond *cond);

Description
This function frees the condition variable cond. Once the function has been entered, cond is no longer a valid condition
variable handle.
If cond is not a valid condition variable handle, or the condition variable has one or more threads waiting on it, then
undefined behavior results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
[POSIX] has a condition variable represented by a structure in user data which is deinitialized with
pthread_cond_destroy.
*******************************************************************************/
KDint  kd_thread_CondFree(KDThreadCond * pstCond_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstCond_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSFlagDel((OS_FLAG_GRP *)pstCond_in, OS_OPT_DEL_ALWAYS, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}
	
	lib_pool_Free((void *)pstCond_in, sizeof(OS_FLAG_GRP));
	
	return (0);
}

/*******************************************************************************
7.3.17. kdThreadCondSignal, kdThreadCondBroadcast
Signal a condition variable.

Synopsis
KDint kdThreadCondSignal(KDThreadCond *cond);
KDint kdThreadCondBroadcast(KDThreadCond *cond);

Description
These functions are used to unblock threads blocked on the condition variable cond. For kdThreadCondSignal,
if threads exist which are being blocked by the condition variable, at least one of them is unblocked. If more than one
thread is blocked on a condition variable, it is undefined which one(s) is/are unblocked. For
kdThreadCondBroadcast, all threads waiting on the condition variable are unblocked.
Once a thread is unblocked as a consequence of some thread calling kdThreadCondSignal, it returns from its call
to kdThreadCondWait, with the associated mutex (as specified to kdThreadCondWait) locked. If multiple
threads are unblocked, they each try and lock the associated mutex before returning from kdThreadCondWait, and
it is undefined which thread acquires the mutex first.
The functions have no effect and succeed if there is no thread waiting on the condition variable.
If cond is not a valid condition variable handle, then undefined behavior results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
kdThreadCondSignal is based on [POSIX] pthread_cond_signal. kdThreadCondBroadcast is based
on [POSIX] pthread_cond_broadcast.
Allowing more than one thread to be unblocked by kdThreadCondSignal is necessary to facilitate implementation
on a multi-processor system. This means that, where a condition variable is used to signal to one of a set of waiting
threads that some resource is now available for use by one of them, a waiting thread may experience a spurious wake-up.
Thus, the code calling kdThreadCondWait must check that the resource really is available to it, and if not loop
back and wait again.
OpenKODE Core has no equivalent of POSIXÅfs PTHREAD_COND_INITIALIZER.
*******************************************************************************/
KDint  kd_thread_CondSignal(KDThreadCond * pstCond_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstCond_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSFlagPendAbort((OS_FLAG_GRP *)pstCond_in, OS_OPT_PEND_ABORT_1, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}

	return (0);
}

KDint  kd_thread_CondBroadcast(KDThreadCond * pstCond_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstCond_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSFlagPendAbort((OS_FLAG_GRP *)pstCond_in, OS_OPT_PEND_ABORT_ALL, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}

	return (0);
}

/*******************************************************************************
7.3.18. kdThreadCondWait
Wait for a condition variable to be signalled.

Synopsis
KDint kdThreadCondWait(KDThreadCond *cond, KDThreadMutex *mutex);

Description
This function blocks on the condition variable cond. A mutex must be associated to the condition variable, and must
be locked when passed to kdThreadCondWait. This mutex becomes bound to the condition variable until successful
return.
The function releases the mutex and causes the calling thread to block on the condition variable as a single atomic
operation with respect to access by another thread to the mutex and then the condition variable. That is, if another
thread is able to lock the mutex after the about-to-block thread has released it, then a subsequent call to
kdThreadCondSignal or kdThreadCondBroadcast in that other thread behaves as if it were issued after the
about-to-block thread has blocked.
Upon successful return, the mutex has been locked and is owned by the calling thread.
If different mutexes are used for concurrent kdThreadCondWait operations on the same condition variable, then
undefined behavior results.
If cond is not a valid condition variable handle, or mutex is not a valid mutex handle, or the mutex is not locked by
the calling thread on entry, then undefined behavior results.

Return value
On success, the function returns 0. There is no defined error case.

Rationale
kdThreadCondWait is based on [POSIX] pthread_cond_wait.
OpenKODE Core has no equivalent of POSIXÅfs PTHREAD_COND_INITIALIZER.
*******************************************************************************/
KDint  kd_thread_CondWait(KDThreadCond * pstCond_in, KDThreadMutex * pstMutex_in)
{
	OS_ERR os_err = OS_ERR_NONE;

	if ((KD_NULL == pstCond_in) || (KD_NULL == pstMutex_in)) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	kd_thread_MutexLock(pstMutex_in);
	
	OSFlagPend
	(
		/* p_grp   */ (OS_FLAG_GRP *)pstCond_in,
		/* flags   */ 1,
		/* timeout */ 0,
		/* opt     */ OS_OPT_PEND_BLOCKING,
		/* p_ts    */ 0,
		/* p_err   */ &os_err
	);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}
	
	kd_thread_MutexUnlock(pstMutex_in);
	
	return (0);
}

/*******************************************************************************
7.3.19. kdThreadSemCreate
Create a semaphore.

Synopsis
typedef struct KDThreadSem KDThreadSem;
KDThreadSem *kdThreadSemCreate(KDuint value);

Description
This function creates a semaphore, and returns a valid handle to it.
A semaphore has a non-negative integer value. This function uses the value parameter as the initial value of the
semaphore.
Semaphores work on an OpenKODE Core implementation that does not support threading.

Return value
On success, the function returns the handle to the new semaphore. On failure, it returns KD_NULL and stores one of
the error codes listed below into the error indicator returned by kdGetError.
Error codes
KD_EINVAL value is larger than the implementation-defined semaphore value limit.
KD_ENOSPC Not enough resources to initialize the semaphore.

Rationale
The semaphore created by kdThreadSemCreate is based on [POSIX] semaphores. POSIX has the semaphore
represented by a structure in user data which is initialized with sem_init, or with a static initializer. OpenKODE
Core has no support for inter-process semaphores.
*******************************************************************************/
KDThreadSem *  kd_thread_SemCreate(KDuint uiValue_in)
{
	OS_SEM * pstSem = KD_NULL;
	OS_ERR   os_err = OS_ERR_NONE;
	
	pstSem = (OS_SEM *)lib_pool_Malloc(sizeof(OS_SEM));
	if (KD_NULL == pstSem) {
	    kd_core_SetError(KD_EINVAL);
	    return (KD_NULL);
	}
	
	OSSemCreate
	(
		pstSem,
		0,
		uiValue_in,
		&os_err
	);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_ENOSPC);
	}
	
	return ((KDThreadSem *)(pstSem));
}

/*******************************************************************************
7.3.20. kdThreadSemFree
Free a semaphore.

Synopsis
KDint kdThreadSemFree(KDThreadSem *sem);

Description
This function frees the semaphore sem. Once the function has been entered, sem is no longer a valid semaphore handle.
If sem is not a valid semaphore handle, or the semaphore has one or more threads blocked on it, then undefined behavior
results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
[POSIX] has a semaphore represented by a structure in user data which is deinitialized with sem_destroy.
*******************************************************************************/
KDint  kd_thread_SemFree(KDThreadSem * pstSem_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstSem_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSSemDel((OS_SEM *)pstSem_in, OS_OPT_DEL_ALWAYS, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}
	
	lib_pool_Free((void *)pstSem_in, sizeof(OS_SEM));
	
	return (0);
}

/*******************************************************************************
7.3.21. kdThreadSemWait
Lock a semaphore.

Synopsis
KDint kdThreadSemWait(KDThreadSem *sem);

Description
This function performs a lock operation on the semaphore specified by sem: If the semaphoreÅfs value is 0, then the
function causes the calling thread to be blocked and added to the set of threads waiting on the semaphore, otherwise,
the value is decremented and the function returns immediately.
Semaphores work on an OpenKODE Core implementation that does not support threading.
If sem is not a valid semaphore handle, then undefined behavior results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
kdThreadSemWait is based on [POSIX] sem_wait.
*******************************************************************************/
KDint  kd_thread_SemWait(KDThreadSem * pstSem_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstSem_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSSemPend
	(
		(OS_SEM *)pstSem_in,
		0,
		0,
		0,
		&os_err
	);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}
	
	return (0);
}

/*******************************************************************************
7.3.22. kdThreadSemPost
Unlock a semaphore.

Synopsis
KDint kdThreadSemPost(KDThreadSem *sem);

Description
This function performs an unlock operation on the semaphore specified by sem: If the set of threads waiting on the
semaphore is empty, then the semaphoreÅfs value is incremented, otherwise, one thread is removed from the set and
unblocked, allowing its kdThreadSemWait call to return successfully. If there is more than one thread in the waiting
set, then it is not specified which one is removed and unblocked.
Semaphores work on an OpenKODE Core implementation that does not support threading.
If sem is not a valid semaphore handle, then undefined behavior results.

Return value
On success, the function returns 0. There is no defined failure case.

Rationale
kdThreadSemPost is based on [POSIX] sem_post.
*******************************************************************************/
KDint  kd_thread_SemPost(KDThreadSem * pstSem_in)
{
	OS_ERR os_err = OS_ERR_NONE;
	
	if (KD_NULL == pstSem_in) {
	    kd_core_SetError(KD_EINVAL);
	    return (-1);
	}
	
	OSSemPost((OS_SEM *)pstSem_in, OS_OPT_POST_1, &os_err);
	if (OS_ERR_NONE != os_err) {
		kd_core_SetError(KD_EINVAL);
	}
	
	return (0);
}

#if 0  /* NOTE */
/*******************************************************************************
7. Threads and synchronization

7.1. Introduction
OpenKODE Core optionally supports threads. An implementation that does not support threads gives the error
KD_ENOSYS as specified in certain functions below. An implementation that does support threads supports all of the
functionality in this section, and thus does not give KD_ENOSYS for any of the functions in this section.
Multiple threads run pre-emptively, that is a thread does not need to take any yield action to ensure that other threads
can run.
Further, an implementation that supports threads also supports non-automatic (global, static and file scope) data in the
application.

7.2. Overview

7.2.1. Thread handling
The threads and synchronization API supported by OpenKODE Core is based on a subset of [POSIX] threads, plus
the non-inter-process functionality of unnamed semaphores.
A new thread is created using kdThreadCreate. This function takes a pointer to the function to run in the new
thread. The new thread exits either when it returns from that function, or when it calls kdThreadExit. A thread ID
of type KDThread* is returned by kdThreadCreate.
kdThreadCreate also optionally takes a thread attributes object handle, used to specify whether the thread is
created detached and its stack size. The thread attributes object is created with kdThreadAttrCreate, modified
with kdThreadAttrSetDetachState and kdThreadAttrSetStackSize, and freed with
kdThreadAttrFree.
A thread is in one of these two states:
? Joinable, which means that another thread can wait for it to finish and collect its return value by calling
kdThreadJoin. This means that, when the thread exits, resources associated with it are not freed until another
thread has called kdThreadJoin on it.
? Detached, which means that its resources are freed as soon as it exits, but it is not possible for another thread to wait
for it to finish and collect its return value.
A newly created thread is by default in the joinable state, although that can be changed using
kdThreadAttrSetDetachState on the attributes used to create the thread. A thread in the joinable state can be
changed to the detached state using kdThreadDetach.

7.2.2. Dynamic initialization
kdThreadOnce is used to ensure that a function (for example an initialization function) is executed only once, in
whichever thread reaches that point first.
kdThreadOnce works on an OpenKODE Core implementation that does not support threading, still only allowing
the initialization function to be executed once.

7.2.3. Mutexes
A mutex is a synchronization primitive which can be locked by at most one thread at a time. It is typically used to
protect access to some resource which only one thread can access at a time.
A mutex is created by a call to kdThreadMutexCreate, which returns a handle to the new mutex, and freed by a
call to kdThreadMutexFree.
A thread locks a mutex by calling kdThreadMutexLock, which blocks until the mutex is available. The mutex is
unlocked by a call to kdThreadMutexUnlock; if any other threads are blocked waiting for the mutex in
kdThreadMutexLock, exactly one of them acquires the lock and is unblocked.
Mutexes work on an OpenKODE Core implementation that does not support threading.

7.2.4. Condition variables
A condition variable is a synchronization primitive used to wait for some application-defined condition, typically that
a shared resource is available, to become true. The basic operations on a condition variable are to wait for the condition
to become true, and to signal that the condition has just become true.
A condition variable has a mutex associated with it, to avoid the race condition where a thread is about to wait on a
condition variable just as another thread signals it. Thus a thread locks the mutex before waiting on the condition
variable, and returns from a successful wait with the mutex locked, but the wait function atomically unlocks the mutex
during the wait.
A condition variable is created by a call to kdThreadCondCreate, which returns a handle to the new condition
variable, and freed by a call to kdThreadCondFree.
A thread waits on a condition variable by calling kdThreadCondWait. Another thread signals the condition by
calling kdThreadCondSignal to release at least one of the waiting threads, or kdThreadCondBroadcast to
release all waiting threads. The released threads return from their respective kdThreadCondWait calls one at a
time, since a returning thread has the associated mutex locked.

7.2.5. Semaphores
A semaphore is a synchronization primitive acting as an integer whose value can never fall below zero. A post operation
increments the value; a wait operation decrements the value, but, if the value is already zero, it waits until another
thread posts.
A semaphore is created by a call to kdThreadSemCreate, which takes the initial count of the semaphore and returns
a handle to the new semaphore, and freed by a call to kdThreadSemFree.
A thread posts a semaphore by calling kdThreadSemPost. A thread waits on a semaphore by calling
kdThreadSemWait.
Semaphores work on an OpenKODE Core implementation that does not support threading.
*******************************************************************************/
#endif /* NOTE */

