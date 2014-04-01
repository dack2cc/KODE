
#ifndef __KD_DEF_H__
#define __KD_DEF_H__

/******************************************************************************
    Include
******************************************************************************/


/******************************************************************************
    Public Interface
******************************************************************************/

enum {
	KD_KERNEL_FNCT_INVALID = -1,
	/* extension */
	__KF_kdextRun,
	__KF_kdextSetup,
	__KF_kdextProcessCreate,
	__KF_kdextProcessExit,
	/* error */
	__KF_kdGetError,
	__KF_kdSetError,
	/* Assertions and logging */
	__KF_kdLogMessage,
	__KF_kdHandleAssertion,
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
	/* Events */
	__KF_kdWaitEvent,
	/* Time functions */
	__KF_kdGetTimeUST,
	__KF_kdTime,
	/* File system */
	__KF_kdFopen,
	KD_KERNEL_FNCT_MAX
};

#define KD_MSG_BUF_MAX    (1024)

#endif /* __KD_DEF_H__ */

