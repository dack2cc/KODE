
/******************************************************************************
    Include
******************************************************************************/

#include <drv_hd.h>
#include <cpu_ext.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

CPU_EXT_HD_REQUEST  drv_hd_stReq;
CPU_INT08U  drv_hd_abyBuffer[512];


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
	CPU_INT32S  i = 0;
	CPU_INT32S  iDiskCnt = 0;
	
	drv_hd_stReq.in.iDev = 0x300;
	drv_hd_stReq.in.iCmd = CPU_EXT_HD_CMD_READ;
	drv_hd_stReq.in.uiSectorStart = 0;
	drv_hd_stReq.in.uiSectorCount = 1;
	drv_hd_stReq.in.pbyBuffer = drv_hd_abyBuffer;
	
	CPUExt_HDRegisterNotifyRW(&drv_hd_NotifyRW);
	CPUExt_HDRegisterNotifyFree(&drv_hd_NotifyFree);
	
	CPUExt_HDGetDiskCount(&iDiskCnt);
	for (i = 0; i < iDiskCnt; ++i) {
	}
	
	CPUExt_HDRequest(&drv_hd_stReq);
	
	return;
}

DRV_PRIVATE void drv_hd_NotifyRW(void * pRequest_in)
{
	CPU_EXT_HD_REQUEST* pstReq = pRequest_in;
	
	if (pstReq == (&drv_hd_stReq)) {
		CPUExt_DispPrint("[drv_hd_NotifyRW][Success] \r\n");
	}
	
	return;
}

DRV_PRIVATE void drv_hd_NotifyFree(void)
{
	return;
}


