
#ifndef __KD_THREAD_H__
#define __KD_THREAD_H__

/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>

/******************************************************************************
    Public Interface
******************************************************************************/

KDThreadAttr * kd_thread_AttrCreate(void);
KDint          kd_thread_AttrFree(KDThreadAttr * pstAttr_in);
KDint          kd_thread_AttrSetDetachState(KDThreadAttr * pstAttr_inout, KDint iDetachState_in);
KDint          kd_thread_AttrSetStackSize(KDThreadAttr * pstAttr_inout, KDsize iStackSize_in);
KDThread *     kd_thread_Create(const KDThreadAttr * pstAttr_in, void * (* pfnStartRoutine_in)(void *), void * arg_in, KDuint32 reversed_0, KDuint32 reversed_1, KDuint32 reversed_2, KDuint32 reversed_3, KDuint32 reversed_4, KDuint32 eflag_in);
void           kd_thread_Exit(void * pRetval_in);
KDint          kd_thread_Join(KDThread * pstThread_in, void **retval);
KDint          kd_thread_Detach(KDThread * pstThread_in);
KDThread *     kd_thread_Self(void);
KDThreadMutex* kd_thread_MutexCreate(const void * pMutexAttr_in);
KDint          kd_thread_MutexFree(KDThreadMutex * pstMutex_in);
KDint          kd_thread_MutexLock(KDThreadMutex * pstMutex_in);
KDint          kd_thread_MutexUnlock(KDThreadMutex * pstMutex_in);
KDThreadCond * kd_thread_CondCreate(const void * pAttr_in);
KDint          kd_thread_CondFree(KDThreadCond * pstCond_in);
KDint          kd_thread_CondSignal(KDThreadCond * pstCond_in);
KDint          kd_thread_CondBroadcast(KDThreadCond * pstCond_in);
KDint          kd_thread_CondWait(KDThreadCond * pstCond_in, KDThreadMutex * pstMutex_in);
KDThreadSem *  kd_thread_SemCreate(KDuint uiValue_in);
KDint          kd_thread_SemFree(KDThreadSem * pstSem_in);
KDint          kd_thread_SemWait(KDThreadSem * pstSem_in);
KDint          kd_thread_SemPost(KDThreadSem * pstSem_in);

#endif /* __KD_THREAD_H__ */

