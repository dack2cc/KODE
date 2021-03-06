#ifndef __DRV_BLK_H__
#define __DRV_BLK_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
//#include <os.h>

/******************************************************************************
    Public Interface
******************************************************************************/

extern void drv_blk_Init(void);

extern void drv_blk_RegisterSync(CPU_FNCT_VOID pfnSync_in);
extern void drv_blk_SyncDevice(const CPU_INT32S iDev_in);
extern void drv_blk_InvalidateBuffer(const CPU_INT32S iDev_in);

typedef struct _DRV_BLK_BUFFER {
    CPU_INT08U * pbyData;      /* pointer to data block (1024 bytes) */
    CPU_INT32U   uiBlkIdx;     /* block number */
    CPU_INT32S   iDev;         /* device (0 = free) */
} DRV_BLK_BUFFER;

extern DRV_BLK_BUFFER * drv_blk_Read(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in);
extern DRV_BLK_BUFFER * drv_blk_ReadAhead(const CPU_INT32S iDev_in, const CPU_INT32S iBlkIdxFst_in, ...);
extern void drv_blk_Release(DRV_BLK_BUFFER* pstBuf_in);
extern void drv_blk_MakeDirty(DRV_BLK_BUFFER* pstBuf_in);

extern void drv_blk_Free(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in);

extern void drv_blk_NotifyRWEnd(DRV_BLK_BUFFER* pstBuf_in);
extern void drv_blk_NotifyReqFree(void);

#endif // __DRV_BLK_H__

