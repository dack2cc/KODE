
/******************************************************************************
    Include
******************************************************************************/

#include <drv_hd.h>
#include <drv_blk.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

CPU_EXT_HD_REQUEST  drv_hd_stReq;
CPU_INT08U  drv_hd_abyBuffer[1024];


/******************************************************************************
    Private Interface
******************************************************************************/

DRV_PRIVATE void drv_hd_NotifyRW(void * pRequest_in);
DRV_PRIVATE void drv_hd_NotifyFree(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_hd_Init(void)
{	
	CPUExt_HDRegisterNotifyRW(&drv_hd_NotifyRW);
	CPUExt_HDRegisterNotifyFree(&drv_hd_NotifyFree);
}

void drv_hd_Setup(void)
{
	CPU_INT32S  iDiskIdx = 0;
	CPU_INT32S  iDiskCnt = 0;
	
	CPUExt_HDGetDiskCount(&iDiskCnt);
	for (iDiskIdx = 0; iDiskIdx < iDiskCnt; ++iDiskIdx) {
		DRV_BLK_BUFFER* pstBuf = drv_blk_Read(0x300 + iDiskIdx*5, 0);
		if (0 == pstBuf) {
			CPUExt_CorePanic("[drv_hd_Setup][Read error]");
		}
		CPUExt_HDSetPartition(iDiskIdx, pstBuf->pbyData);
		drv_blk_Release(pstBuf);
	}
	
	CPUExt_DispPrint("[HardDisk][Setup Complete] \r\n");
}

DRV_PRIVATE void drv_hd_NotifyRW(void * pRequest_in)
{
	drv_blk_NotifyRWEnd((DRV_BLK_BUFFER *)pRequest_in);
}

DRV_PRIVATE void drv_hd_NotifyFree(void)
{
	drv_blk_NotifyReqFree();
}


