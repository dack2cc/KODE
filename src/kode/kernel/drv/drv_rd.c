
/******************************************************************************
    Include
******************************************************************************/

#include <drv_rd.h>
#include <drv_blk.h>
#include <drv_disp.h>
#include <cpu_ext.h>
#include <lib_mem.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

/******************************************************************************
    Private Interface
******************************************************************************/

typedef struct _DRV_RD_CONTROL {
	CPU_INT08U * pbyStart;
	CPU_INT32U   uiSize;
} DRV_RD_CONTROL;

DRV_PRIVATE  DRV_RD_CONTROL drv_rd_stCtl;

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_rd_Init(void)
{
	CPU_ADDR adrPhyStart = 0;
	CPU_ADDR adrPhyEnd   = 0;
	
	drv_rd_stCtl.pbyStart = 0;
	drv_rd_stCtl.uiSize   = 0;
	
	if (CPU_EXT_RAM_DISK_SIZE <= 0) {
		return;
	}
	
	CPUExt_PageGetRamdiskSpace(&adrPhyStart, &adrPhyEnd);
	if ((adrPhyStart >= adrPhyEnd) || ((adrPhyEnd - adrPhyStart) != CPU_EXT_RAM_DISK_SIZE)) {
		CPUExt_CorePanic("[drv_rd_Init][ramdisk address and size invalid]");
		return;
	}
	
	drv_rd_stCtl.pbyStart = (CPU_INT08U *)adrPhyStart;
	drv_rd_stCtl.uiSize   = CPU_EXT_RAM_DISK_SIZE;
}

void drv_rd_Setup(void)
{
	CPU_INT32U       uiBlkIdx = CPU_EXT_RAM_DISK_START;
	CPU_INT32U       uiBlkCnt = CPU_EXT_RAM_DISK_SIZE / CPU_EXT_HD_BLOCK_SIZE;
	DRV_BLK_BUFFER * pstBuf   = 0;
	CPU_INT08U *     pbyDst   = drv_rd_stCtl.pbyStart;
	CPU_INT32U       uiCpSize = 0;
	
	if ((0 == drv_rd_stCtl.pbyStart) || (0 == drv_rd_stCtl.uiSize)) {
		return;
	}
	
	CPUExt_DispPrint("[RamDisk ][Loading...] \r\n");	
	while (uiBlkCnt > 0) {
		if ((uiCpSize + CPU_EXT_HD_BLOCK_SIZE) > drv_rd_stCtl.uiSize) {\
			CPUExt_CorePanic("[drv_rd_Setup][buffer overflow]");
		}
		if (uiBlkCnt > 2) {
			pstBuf = drv_blk_ReadAhead(0x300, uiBlkIdx, uiBlkIdx+1, uiBlkIdx+2, -1);
		} 
		else {
			pstBuf = drv_blk_Read(0x300, uiBlkIdx);
		}
		if (0 == pstBuf) {
			CPUExt_CorePanic("[drv_rd_Setup][reading block failed]");
		}
		Mem_Copy(pbyDst, pstBuf->pbyData, CPU_EXT_HD_BLOCK_SIZE);
		drv_blk_Release(pstBuf);
		
		uiCpSize += CPU_EXT_HD_BLOCK_SIZE;
		pbyDst   += CPU_EXT_HD_BLOCK_SIZE;
		uiBlkIdx++;
		uiBlkCnt--;
	}
}

void drv_rd_Request(CPU_EXT_HD_REQUEST * pstRequest_inout)
{
	CPU_INT08U * pbyDisk = 0;

	if ((0 == drv_rd_stCtl.pbyStart) || (0 == drv_rd_stCtl.uiSize)) {
		CPUExt_CorePanic("[drv_rd_Request][Ramdisk invalid]");
	}
	if ((0 == pstRequest_inout) 
	||  (0 == pstRequest_inout->in.pbyData)
	||  (DRV_RD_DEVICE != (DRV_RD_DEVICE & pstRequest_inout->in.iDev))) {
		CPUExt_CorePanic("[drv_rd_Request][Parameter invalid]");
	}
	
	pbyDisk = drv_rd_stCtl.pbyStart + ((pstRequest_inout->in.uiBlkIdx) * CPU_EXT_HD_BLOCK_SIZE);
	if ((pbyDisk + CPU_EXT_HD_BLOCK_SIZE) > (drv_rd_stCtl.pbyStart + drv_rd_stCtl.uiSize)) {
		CPUExt_CorePanic("[drv_rd_Request][Reading failed]");
		//pstRequest_inout->out.iResult = CPU_EXT_HD_RESULT_IO;
		//drv_blk_NotifyRWEnd((DRV_BLK_BUFFER *)pstRequest_inout);
		//return;
	}
	
	if (CPU_EXT_HD_CMD_READ == pstRequest_inout->in.iCmd) {
		Mem_Copy(pstRequest_inout->in.pbyData, pbyDisk, CPU_EXT_HD_BLOCK_SIZE);
	}
	else if (CPU_EXT_HD_CMD_WRITE == pstRequest_inout->in.iCmd) {
		Mem_Copy(pbyDisk, pstRequest_inout->in.pbyData, CPU_EXT_HD_BLOCK_SIZE);
	}
	else {
		CPUExt_CorePanic("[drv_rd_Request][Reading failed]");
	}
	pstRequest_inout->out.iResult = CPU_EXT_HD_RESULT_OK;
	drv_blk_NotifyRWEnd((DRV_BLK_BUFFER *)pstRequest_inout);	
}

