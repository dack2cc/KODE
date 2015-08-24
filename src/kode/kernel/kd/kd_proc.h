
#ifndef __KD_PROCESS_H__
#define __KD_PROCESS_H__

/******************************************************************************
    Include
******************************************************************************/

#include <kd/kdext.h>

/******************************************************************************
    Public Interface
******************************************************************************/

KDExtProcess * kd_proc_Create(KDuint32 resversed, void * (* pfnStartRoutine_in)(void *), void * arg_in, KDuint32 reversed_0, KDuint32 reversed_1, KDuint32 reversed_2, KDuint32 reversed_3, KDuint32 reversed_4, KDuint32 eflag_in);
void           kd_proc_Exit(void * pRetval_in);

#endif /* __KD_PROCESS_H__ */

