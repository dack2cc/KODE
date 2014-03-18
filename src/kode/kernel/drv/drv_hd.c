
/******************************************************************************
    Include
******************************************************************************/

#include <drv_hd.h>
#include <drv_blk.h>
#include <drv_disp.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

#define DRV_HD_TEST_OFF  (0)
#define DRV_HD_TEST_ON   (1)
#define DRV_HD_TEST      DRV_HD_TEST_OFF
//#define DRV_HD_TEST      DRV_HD_TEST_ON

#if (DRV_HD_TEST == DRV_HD_TEST_ON)
#define DRV_HD_TEST_DEVICE  (0x300)
#define DRV_HD_TEST_BLOCK   (7)
DRV_PRIVATE CPU_EXT_HD_REQUEST  drv_hd_stReq;
DRV_PRIVATE CPU_INT08U          drv_hd_abyBuffer[1024];
DRV_PRIVATE const CPU_INT08U    drv_hd_abyTest[] = {
	0xA5, 0xA7, 0x79, 0x65, 0x96, 0xB1, 0xF9, 0x77
};
#define DRV_HD_TEST_MAX  (sizeof(drv_hd_abyTest)/sizeof(CPU_INT08U))
#endif // DRV_HD_TEST

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
	
#if (DRV_HD_TEST == DRV_HD_TEST_ON)
	{
		CPU_INT32U i = 0;
		for (i = 0; i < DRV_HD_TEST_MAX; ++i) {
			drv_hd_abyBuffer[i] = drv_hd_abyTest[i];
		}
	}
	drv_hd_stReq.in.pbyData  = drv_hd_abyBuffer;
	drv_hd_stReq.in.uiBlkIdx = DRV_HD_TEST_BLOCK;
	drv_hd_stReq.in.iCmd     = CPU_EXT_HD_CMD_WRITE;
	//drv_hd_stReq.in.iCmd     = CPU_EXT_HD_CMD_READ;
	drv_hd_stReq.in.iDev     = DRV_HD_TEST_DEVICE;
	CPUExt_HDRequest(&drv_hd_stReq);
#endif // DRV_HD_TEST
}

void drv_hd_Setup(void)
{
#if (DRV_HD_TEST == DRV_HD_TEST_OFF)
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
	
	drv_disp_Printf("[HardDisk][Setup Complete] \r\n");
#endif // DRV_HD_TEST
}

DRV_PRIVATE void drv_hd_NotifyRW(void * pRequest_in)
{
#if (DRV_HD_TEST == DRV_HD_TEST_OFF)
	
	drv_blk_NotifyRWEnd((DRV_BLK_BUFFER *)pRequest_in);

#else  // DRV_HD_TEST
	drv_disp_Printf("[drv_hd_NotifyRW][callback coming] \r\n");
	if (pRequest_in != &(drv_hd_stReq)) {
		CPUExt_CorePanic("[drv_hd_NotifyRW][pointer invalid]");
	}
	if (DRV_HD_TEST_DEVICE != drv_hd_stReq.in.iDev) {
		CPUExt_CorePanic("[drv_hd_NotifyRW][device exception]");
	}
	if (DRV_HD_TEST_BLOCK != drv_hd_stReq.in.uiBlkIdx) {
		CPUExt_CorePanic("[drv_hd_NotifyRW][block exception]");
	}
	if (CPU_EXT_HD_RESULT_OK != drv_hd_stReq.out.iResult) {
		CPUExt_CorePanic("[drv_hd_NotifyRW][result failed]");
	}
	if (CPU_EXT_HD_CMD_WRITE == drv_hd_stReq.in.iCmd) {
		CPU_INT32U  i = 0;
		for (i = 0; i < DRV_HD_TEST_MAX; ++i) {
			drv_hd_abyBuffer[i] = 0x00;
		}
		drv_disp_Printf("[drv_hd_NotifyRW][writing is OK] \r\n");
		drv_hd_stReq.in.iCmd = CPU_EXT_HD_CMD_READ;
		CPUExt_HDRequest(&drv_hd_stReq);
	}
	else if (CPU_EXT_HD_CMD_READ == drv_hd_stReq.in.iCmd) {
		CPU_INT32U i = 0;
		for (i = 0; i < DRV_HD_TEST_MAX; ++i) {
			if (drv_hd_abyBuffer[i] != drv_hd_abyTest[i]) {
				CPUExt_CorePanic("[drv_hd_NotifyRW][read is failed]");
			}
		}
		drv_disp_Printf("[drv_hd_NotifyRW][reading is OK] \r\n");
	}
	else {
		CPUExt_CorePanic("[drv_hd_NotifyRW][command exception]");
	}
#endif // DRV_HD_TEST
}

DRV_PRIVATE void drv_hd_NotifyFree(void)
{
	drv_blk_NotifyReqFree();
}


