
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_hd.h>
#include <cpu_ext.h>
#include <cpu_boot.h>

/******************************************************************************
    Private Definition
******************************************************************************/

typedef struct _CPU_HD_PARTITION_TABLE {
	CPU_INT08U  boot_ind;
	CPU_INT08U  head;
	CPU_INT08U  sector;
	CPU_INT08U  cylinder;
	CPU_INT08U  sys_ind;
	CPU_INT08U  end_head;
	CPU_INT08U  end_sector;
	CPU_INT08U  end_cylinder;
	CPU_INT32U  sector_start;
	CPU_INT32U  sector_count;
} CPU_HD_PARTITION_TABLE;


typedef struct _CPU_HD_DISK {
	CPU_INT32S  iHead;
	CPU_INT32S  iSector;
	CPU_INT32S  iCylinder;
	CPU_INT32S  iWpcom;
	CPU_INT32S  iLzone;
	CPU_INT32S  iCtl;
} CPU_HD_DISK;
#define CPU_HD_DISK_MAX       (2)


typedef struct _CPU_HD_PARTITION {
	CPU_INT32S  iStart;
	CPU_INT32S  iCount;
} CPU_HD_PARTITION;
#define CPU_HD_PARTITION_MAX  (5)


typedef struct _CPU_HD_REQUEST {
	CPU_EXT_HD_REQUEST_IN     stReq;
	CPU_EXT_HD_REQUEST *      pstOrg;
	struct _CPU_HD_REQUEST *  pstNext;
} CPU_HD_REQUEST;
#define CPU_HD_REQUEST_MAX        (32)
#define CPU_HD_REQUEST_WRITE_MAX  ((2*CPU_HD_REQUEST_MAX)/3)

/*
 * This is used in the elevator algorithm: Note that
 * reads always go before writes. This is natural: reads
 * are much more time-critical than writes.
 */
#define _CPU_HD_IN_ORDER(s1,s2) \
(((s1)->stReq.iCmd<(s2)->stReq.iCmd || ((s1)->stReq.iCmd==(s2)->stReq.iCmd && \
((s1)->stReq.iDev < (s2)->stReq.iDev || (((s1)->stReq.iDev == (s2)->stReq.iDev && \
(s1)->stReq.uiSectorStart < (s2)->stReq.uiSectorStart))))))


typedef struct _CPU_HD_CONTROL {
	X86_HD_INFO       stInfo;
	CPU_HD_DISK       astDisk[CPU_HD_DISK_MAX];
	CPU_INT08S        iDiskCnt;
	CPU_HD_PARTITION  astPart[CPU_HD_DISK_MAX * CPU_HD_PARTITION_MAX];
	CPU_FNCT_PTR      pfnNotify;
	CPU_HD_REQUEST *  pstCurrentReq;
} CPU_HD_CONTROL;

CPU_PRIVATE  CPU_HD_CONTROL  cpu_hd_stCtl;
CPU_PRIVATE  CPU_HD_REQUEST  cpu_hd_astReqBuf[CPU_HD_REQUEST_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE void cpu_hd_AddRequest(CPU_HD_REQUEST* pstRequest_in);
CPU_PRIVATE void cpu_hd_DoRequest(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_hd_Init(void)
{
	CPU_INT08S i = 0;
	void*  pInfo = 0;
	
	/* clear the request buffer */
	for (i = 0; i < CPU_HD_REQUEST_MAX; ++i) {
		cpu_hd_astReqBuf[i].stReq.iDev = -1;
		cpu_hd_astReqBuf[i].pstOrg  = 0;
		cpu_hd_astReqBuf[i].pstNext = 0;
	}
	
	/* initialize the request information */
	cpu_hd_stCtl.pstCurrentReq = 0;
	cpu_hd_stCtl.pfnNotify = 0;
	
	/* get the information from bios */
	cpu_hd_stCtl.stInfo = X86_HD_PARAM;
    pInfo = (void *)(&(cpu_hd_stCtl.stInfo));
	
	/* save the information for each hard disk */
	for (i = 0; i < CPU_HD_DISK_MAX; ++i) {
		cpu_hd_stCtl.astDisk[i].iCylinder = (*((CPU_INT16U *)pInfo));
		cpu_hd_stCtl.astDisk[i].iHead     = (*((CPU_INT08U *)(pInfo + 2)));
		cpu_hd_stCtl.astDisk[i].iWpcom    = (*((CPU_INT16U *)(pInfo + 5)));
		cpu_hd_stCtl.astDisk[i].iCtl      = (*((CPU_INT08U *)(pInfo + 8)));
		cpu_hd_stCtl.astDisk[i].iLzone    = (*((CPU_INT16U *)(pInfo + 12)));
		cpu_hd_stCtl.astDisk[i].iSector   = (*((CPU_INT08U *)(pInfo + 14)));
		pInfo += 16;
	}
	if (cpu_hd_stCtl.astDisk[1].iCylinder) {
		cpu_hd_stCtl.iDiskCnt = 2;
	}
	else {
		cpu_hd_stCtl.iDiskCnt = 1;
	}
	
	/* initialize the partition for each hard disk */
	for (i = 0; i < CPU_HD_DISK_MAX; ++i) {
		if (i < cpu_hd_stCtl.iDiskCnt) {
			cpu_hd_stCtl.astPart[i * CPU_HD_PARTITION_MAX].iStart = 0;
			cpu_hd_stCtl.astPart[i * CPU_HD_PARTITION_MAX].iCount = cpu_hd_stCtl.astDisk[i].iHead * cpu_hd_stCtl.astDisk[i].iSector * cpu_hd_stCtl.astDisk[i].iCylinder;
		} 
		else {
			cpu_hd_stCtl.astPart[i * CPU_HD_PARTITION_MAX].iStart = 0;
			cpu_hd_stCtl.astPart[i * CPU_HD_PARTITION_MAX].iCount = 0;
		}
	}
	
	return;
}

void  cpu_hd_GetDiskCount(CPU_INT32S *  piCount_out)
{
	if (0 != piCount_out) {
		(*piCount_out) = cpu_hd_stCtl.iDiskCnt;
	}
}

void  cpu_hd_SetPartition(const CPU_INT32S iDiskIndex_in, const CPU_INT08U * pbyTable_in)
{
	CPU_INT08S  i = 0;
	CPU_HD_PARTITION_TABLE*  pstTable = 0;
	
	if ((iDiskIndex_in < 0) 
	||  (iDiskIndex_in >= cpu_hd_stCtl.iDiskCnt)
	||  (0 == pbyTable_in)) {
		return;
	}
	
	if ((0x55 != pbyTable_in[510]) || (0xAA != pbyTable_in[511])) {
		CPUExt_CorePanic("[cpu_hd_SetPartition][Bad Partition Table]");
	}
	
	pstTable = 0x1BE + (void *)(pbyTable_in);
	for (i = 0; i < CPU_HD_PARTITION_MAX; ++i, ++pstTable) {
		cpu_hd_stCtl.astPart[i + CPU_HD_PARTITION_MAX * iDiskIndex_in].iStart = pstTable->sector_start;
		cpu_hd_stCtl.astPart[i + CPU_HD_PARTITION_MAX * iDiskIndex_in].iCount = pstTable->sector_count;
	}
}

void  cpu_hd_RegisterNotify(CPU_FNCT_PTR pfnNotify_in)
{
	cpu_hd_stCtl.pfnNotify = pfnNotify_in;
}

void  cpu_hd_Request(CPU_EXT_HD_REQUEST* pstRequest_inout)
{
	CPU_HD_REQUEST* pstReq = 0;
	
	if (0 == pstRequest_inout) {
		return;
	}
	
	if (CPU_EXT_HD_CMD_READ == pstRequest_inout->in.iCmd) {
		pstReq = cpu_hd_astReqBuf + CPU_HD_REQUEST_MAX;
	}
	else {
		pstReq = cpu_hd_astReqBuf + CPU_HD_REQUEST_WRITE_MAX;
	}
	
	while (pstReq >= cpu_hd_astReqBuf) {
		if (pstReq->stReq.iDev < 0) {
			break;
		}
		--pstReq;
	}
	
	if (pstReq < cpu_hd_astReqBuf) {
		pstRequest_inout->out.iResult = CPU_EXT_HD_RESULT_FULL;
		return;
	}
	
	pstReq->stReq.iDev = pstRequest_inout->in.iDev;
	pstReq->stReq.iCmd = pstRequest_inout->in.iCmd;
	pstReq->stReq.uiSectorStart = pstRequest_inout->in.uiSectorStart;
	pstReq->stReq.uiSectorCount = pstRequest_inout->in.uiSectorCount;
	pstReq->stReq.pbyBuffer = pstRequest_inout->in.pbyBuffer;
	pstReq->pstOrg  = pstRequest_inout;
	pstReq->pstNext = 0;
	cpu_hd_AddRequest(pstReq);
}

void cpu_hd_ISR_HardDisk(void)
{
	return;
}

CPU_PRIVATE void cpu_hd_AddRequest(CPU_HD_REQUEST* pstRequest_in)
{
	CPU_HD_REQUEST* pstTmp = 0;
	CPU_SR_ALLOC();
	
	if (0 == pstRequest_in) {
		return;
	}
	pstRequest_in->pstNext = 0;
	
	CPU_INT_DIS();

	if (0 == cpu_hd_stCtl.pstCurrentReq) {
		cpu_hd_stCtl.pstCurrentReq = pstRequest_in;
		CPU_INT_EN();
		cpu_hd_DoRequest();
		return;
	}
	
	for (; pstTmp->pstNext; pstTmp = pstTmp->pstNext) {
		if ((_CPU_HD_IN_ORDER(pstTmp, pstRequest_in) ||
			!_CPU_HD_IN_ORDER(pstTmp, pstTmp->pstNext)) &&
		    _CPU_HD_IN_ORDER(pstRequest_in, pstTmp->pstNext)) {
			break;
		}
	}
	
	pstRequest_in->pstNext = pstTmp->pstNext;
	pstTmp->pstNext = pstRequest_in;
	
	CPU_INT_EN();
}

CPU_PRIVATE void cpu_hd_DoRequest(void)
{
	return;
}

