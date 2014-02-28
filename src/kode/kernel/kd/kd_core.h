
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
void  kd_core_StrReadUserSpace(const KDchar * pszSrcUsr_in, KDchar * pszDstKnl_out, const KDint iDstSize_in);

#endif /* __KD_CORE_H__ */

