
#ifndef __KD_CORE_H__
#define __KD_CORE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>

/******************************************************************************
    Public Interface
******************************************************************************/

void  kd_core_LogMessage(const KDchar* pszString_in);
void  kd_core_Assert(const KDchar* pszCondition_in, const KDchar* pszFilename_in, KDint iLineNumber_in);
void  kd_core_SetError(KDint error);

#endif /* __KD_CORE_H__ */

