#ifndef __CPU_HD_H__
#define __CPU_HD_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu_ext.h>

/******************************************************************************
    Define
******************************************************************************/

/******************************************************************************
    Public Interface
******************************************************************************/

extern void  cpu_hd_Init(void);

extern void  cpu_hd_GetDiskCount(CPU_INT32S * piCount_out);
extern void  cpu_hd_SetPartition(const CPU_INT32S iDiskIndex_in, const CPU_INT08U * pbyTable_in);

extern void  cpu_hd_RegisterNotifyRW(CPU_FNCT_PTR pfnNotify_in);
extern void  cpu_hd_RegisterNotifyFree(CPU_FNCT_VOID pfnNotify_in);
extern void  cpu_hd_Request(CPU_EXT_HD_REQUEST* pstRequest_inout);

extern void  cpu_key_ISR_HardDisk(void);

#endif // __CPU_HD_H__

