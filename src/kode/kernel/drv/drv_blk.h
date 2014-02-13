#ifndef __DRV_BLK_H__
#define __DRV_BLK_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <os.h>

/******************************************************************************
    Public Interface
******************************************************************************/

extern void drv_blk_Init(void);

typedef struct _DRV_BLK_BUFFER {
	CPU_INT08U * pbyData;      /* pointer to data block (1024 bytes) */
	CPU_INT32U   uiBlkIdx;     /* block number */
	CPU_INT32S   iDev;         /* device (0 = free) */
} DRV_BLK_BUFFER;

extern DRV_BLK_BUFFER * drv_blk_Read(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in);

#endif // __DRV_BLK_H__

