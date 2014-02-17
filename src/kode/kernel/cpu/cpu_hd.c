
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_hd.h>
#include <cpu_ext.h>
#include <cpu_boot.h>
#include <cpu_asm.h>

/******************************************************************************
    Private Definition
******************************************************************************/


#define _CPU_HD_DEV_MAJOR(a)  (((unsigned)(a))>>8)
#define _CPU_HD_DEV_MINOR(a)  ((a)&0xff)
#define _CPU_HD_DEV_MAJOR_NR  (3)

#define CPU_HD_ERROR_MAX      (7)


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
	CPU_INT32U                uiSectorStart;
	CPU_INT32U                uiSectorCount;
	CPU_INT32S                iErrCnt;
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
(s1)->uiSectorStart < (s2)->uiSectorStart))))))


typedef struct _CPU_HD_CONTROL {
	X86_HD_INFO       stInfo;
	CPU_INT08S        iDiskCnt;
	CPU_HD_DISK       astDisk[CPU_HD_DISK_MAX];
	CPU_HD_PARTITION  astPart[CPU_HD_DISK_MAX * CPU_HD_PARTITION_MAX];
	CPU_FNCT_PTR      pfnNotifyRW;
	CPU_FNCT_VOID     pfnNotifyFree;
	CPU_HD_REQUEST *  pstCurrentReq;
	CPU_FNCT_VOID     pfnISR;
	CPU_INT08S        iNeedReset;
	CPU_INT08S        iNeedRecalibrate;
} CPU_HD_CONTROL;

CPU_PRIVATE  CPU_HD_CONTROL  cpu_hd_stCtl;
CPU_PRIVATE  CPU_HD_REQUEST  cpu_hd_astReqBuf[CPU_HD_REQUEST_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/

#define _CPU_HD_PORT_READ(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr):/*"cx","di"*/)

#define _CPU_HD_PORT_WRITE(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr):/*"cx","si"*/)


CPU_PRIVATE void cpu_hd_AddRequest(CPU_HD_REQUEST* pstRequest_in);
CPU_PRIVATE void cpu_hd_DoRequest(void);
CPU_PRIVATE void cpu_hd_EndRequest(const CPU_INT32S iResult_in);
CPU_PRIVATE void cpu_hd_Reset(const CPU_INT08U uiDiskIndex_in);
CPU_PRIVATE void cpu_hd_Out(
	CPU_INT32U uiDrv_in, CPU_INT32U uiSecCnt_in, CPU_INT32U uiSec_in, CPU_INT32U uiHead_in, 
	CPU_INT32U uiCyl_in, CPU_INT32U uiCmd_in, CPU_FNCT_VOID pfnISR_in
);

CPU_PRIVATE CPU_INT32S  cpu_hd_IsControllerReady(void);
CPU_PRIVATE CPU_INT32S  cpu_hd_IsWinResultNG(void);
CPU_PRIVATE CPU_INT32S  cpu_hd_IsDriverBusy(void);

CPU_PRIVATE void cpu_hd_BadRWInterrupt(void);
CPU_PRIVATE void cpu_hd_ISRUnexpected(void);
CPU_PRIVATE void cpu_hd_ISRRecall(void);
CPU_PRIVATE void cpu_hd_ISRRead(void);
CPU_PRIVATE void cpu_hd_ISRWrite(void);


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
		cpu_hd_astReqBuf[i].iErrCnt = 0;
		cpu_hd_astReqBuf[i].pstNext = 0;
	}
	
	/* initialize the request information */
	cpu_hd_stCtl.pstCurrentReq = 0;
	cpu_hd_stCtl.pfnNotifyFree = 0;
	cpu_hd_stCtl.pfnNotifyRW   = 0;
	cpu_hd_stCtl.pfnISR = 0;
	cpu_hd_stCtl.iNeedReset = 0;
	cpu_hd_stCtl.iNeedRecalibrate = 0;
	
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

void  CPUExt_HDGetDiskCount(CPU_INT32S *  piCount_out)
{
	if (0 != piCount_out) {
		(*piCount_out) = cpu_hd_stCtl.iDiskCnt;
	}
}

void  CPUExt_HDSetPartition(const CPU_INT32S iDiskIndex_in, const CPU_INT08U * pbyTable_in)
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

void  CPUExt_HDRegisterNotifyRW(CPU_FNCT_PTR pfnNotify_in)
{
	cpu_hd_stCtl.pfnNotifyRW = pfnNotify_in;
}

void  CPUExt_HDRegisterNotifyFree(CPU_FNCT_VOID pfnNotify_in)
{
	cpu_hd_stCtl.pfnNotifyFree = pfnNotify_in;
}

void  CPUExt_HDRequest(CPU_EXT_HD_REQUEST* pstRequest_inout)
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
	
	while (--pstReq >= cpu_hd_astReqBuf) {
		if (pstReq->stReq.iDev < 0) {
			break;
		}
	}
	
	if (pstReq < cpu_hd_astReqBuf) {
		pstRequest_inout->out.iResult = CPU_EXT_HD_RESULT_FULL;
		return;
	}

	pstReq->stReq.pbyData = pstRequest_inout->in.pbyData;
	pstReq->stReq.iDev    = pstRequest_inout->in.iDev;
	pstReq->stReq.iCmd    = pstRequest_inout->in.iCmd;
	pstReq->uiSectorStart = pstRequest_inout->in.uiBlkIdx << 1;
	pstReq->uiSectorCount = 2;
	pstReq->pstOrg  = pstRequest_inout;
	pstReq->iErrCnt = 0;
	pstReq->pstNext = 0;
	cpu_hd_AddRequest(pstReq);
}

void cpu_hd_ISR_HardDisk(void)
{
	CPU_FNCT_VOID  pfnISR = cpu_hd_stCtl.pfnISR;
	
	if (0 != pfnISR) {
		pfnISR();
	}
	else {
		cpu_hd_ISRUnexpected();
	}
	
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
	
	CPU_CRITICAL_ENTER();

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
	
	CPU_CRITICAL_EXIT();
}

CPU_PRIVATE void cpu_hd_DoRequest(void)
{
	CPU_INT32U  uiDev  = 0;
	CPU_INT32U  uiBlk  = 0;
	CPU_INT32U  uiSec  = 0;
	CPU_INT32U  uiCyl  = 0;
	CPU_INT32U  uiHead = 0;
	CPU_INT32S  i      = 0;
	CPU_INT32S  iDRQ   = 0;
	
_L_REPEAT:
	if (0 == cpu_hd_stCtl.pstCurrentReq) {
		return;
	}
	if (_CPU_HD_DEV_MAJOR_NR != _CPU_HD_DEV_MAJOR(cpu_hd_stCtl.pstCurrentReq->stReq.iDev)) {
		CPUExt_CorePanic("[cpu_hd_DoRequest][request list destroyed]");
	}
	
	uiDev = _CPU_HD_DEV_MINOR(cpu_hd_stCtl.pstCurrentReq->stReq.iDev);
	uiBlk = cpu_hd_stCtl.pstCurrentReq->uiSectorStart;
	if ((uiDev >= CPU_HD_DISK_MAX * CPU_HD_PARTITION_MAX)
	||  (uiBlk + 2 > cpu_hd_stCtl.astPart[uiDev].iCount)) {
		cpu_hd_EndRequest(CPU_EXT_HD_RESULT_IO);
		goto _L_REPEAT;
	}
	
	uiBlk += cpu_hd_stCtl.astPart[uiDev].iStart;
	uiDev /= CPU_HD_PARTITION_MAX;
	
	__asm__(
		"divl %4":"=a"(uiBlk),"=d"(uiSec) :"0"(uiBlk),"1"(0),"r"(cpu_hd_stCtl.astDisk[uiDev].iSector)
	);
	__asm__(
		"divl %4":"=a"(uiCyl),"=d"(uiHead):"0"(uiBlk),"1"(0),"r"(cpu_hd_stCtl.astDisk[uiDev].iHead)
	);
	uiSec++;
	
	if (cpu_hd_stCtl.iNeedReset) {
		cpu_hd_stCtl.iNeedReset = 0;
		cpu_hd_stCtl.iNeedRecalibrate = 1;
		cpu_hd_Reset(uiDev);
		return;
	}
	if (cpu_hd_stCtl.iNeedRecalibrate) {
		cpu_hd_stCtl.iNeedRecalibrate = 0;
		cpu_hd_Out(
			uiDev, cpu_hd_stCtl.astDisk[uiDev].iSector, 0, 0, 0, X86_HD_WIN_RESTORE, &cpu_hd_ISRRecall
		);
		return;
	}
	
	if (CPU_EXT_HD_CMD_READ == cpu_hd_stCtl.pstCurrentReq->stReq.iCmd) {
		cpu_hd_Out( 
			uiDev, cpu_hd_stCtl.pstCurrentReq->uiSectorCount, uiSec, uiHead,
			uiCyl, X86_HD_WIN_READ, &cpu_hd_ISRRead
		);
	}
	else if (CPU_EXT_HD_CMD_WRITE == cpu_hd_stCtl.pstCurrentReq->stReq.iCmd) {
		cpu_hd_Out(
			uiDev, cpu_hd_stCtl.pstCurrentReq->uiSectorCount, uiSec, uiHead,
			uiCyl, X86_HD_WIN_WRITE, &cpu_hd_ISRWrite
		);
		for (i = 0; i < 3000; ++i) {
			iDRQ = _asm_inb_p(X86_HD_STATUS)&X86_HD_STATUS_DRQ;
			if (iDRQ) {
				break;
			}
			
		}
		if (!iDRQ) {
			cpu_hd_BadRWInterrupt();
			goto _L_REPEAT;
		}
		_CPU_HD_PORT_WRITE(X86_HD_DATA, cpu_hd_stCtl.pstCurrentReq->stReq.pbyData,256);
	}
	else {
		CPUExt_CorePanic("[cpu_hd_DoRequest][Unknown HD Command]");
	}
	
	return;
}

CPU_PRIVATE void cpu_hd_EndRequest(const CPU_INT32S iResult_in)
{
	CPU_FNCT_PTR   pfnNotifyRW   = cpu_hd_stCtl.pfnNotifyRW;
	CPU_FNCT_VOID  pfnNotifyFree = cpu_hd_stCtl.pfnNotifyFree;
	CPU_EXT_HD_REQUEST *  pstOrg = cpu_hd_stCtl.pstCurrentReq->pstOrg;
	CPU_INT08U  i = 0;
	
	for (i = 0; i < CPU_HD_REQUEST_MAX; ++i) {
		if (cpu_hd_astReqBuf[i].stReq.iDev < 0) {
			break;
		}
	}
	if (i < CPU_HD_REQUEST_MAX) {
		pfnNotifyFree = 0;
	}
	
	cpu_hd_stCtl.pstCurrentReq->stReq.iDev = -1;
	cpu_hd_stCtl.pstCurrentReq = cpu_hd_stCtl.pstCurrentReq->pstNext;
	
	if (0 != pfnNotifyRW) {
		pstOrg->out.iResult = iResult_in;
		pfnNotifyRW((void *)pstOrg);
	}
	if (0 != pfnNotifyFree) {
		pfnNotifyFree();
	}
}

CPU_PRIVATE void cpu_hd_Reset(const CPU_INT08U uiDiskIndex_in)
{
	CPU_INT08U  i = 0;
	
	if (uiDiskIndex_in >= CPU_HD_DISK_MAX) {
		CPUExt_CorePanic("[cpu_hd_Reset][Unknown Disk]");
	}
	
	_asm_outb(4, X86_HD_CMD);
	for (i = 0; i < 100; ++i) {
		_asm_nop();
	}
	_asm_outb(cpu_hd_stCtl.astDisk[0].iCtl & 0x0F, X86_HD_CMD);
	
	if (cpu_hd_IsDriverBusy()) {
		CPUExt_DispPrint("[cpu_hd_Reset][HD Controller still busy] \r\n");
	}
	if (1 != (i = _asm_inb(X86_HD_ERROR))) {
		CPUExt_DispPrint("[cpu_hd_Reset][HD Controller reset failed]\r\n");
	}
	
	cpu_hd_Out(
		uiDiskIndex_in,
		cpu_hd_stCtl.astDisk[uiDiskIndex_in].iSector,
		cpu_hd_stCtl.astDisk[uiDiskIndex_in].iSector,
		cpu_hd_stCtl.astDisk[uiDiskIndex_in].iHead - 1,
		cpu_hd_stCtl.astDisk[uiDiskIndex_in].iCylinder,
		X86_HD_WIN_SPECIFY,
		&cpu_hd_ISRRecall
	);
}

CPU_PRIVATE void cpu_hd_Out(
	CPU_INT32U uiDrv_in, CPU_INT32U uiSecCnt_in, CPU_INT32U uiSec_in, CPU_INT32U uiHead_in, 
	CPU_INT32U uiCyl_in, CPU_INT32U uiCmd_in, CPU_FNCT_VOID pfnISR_in)
	
{
	register int regPort asm("dx");
	
	if ((uiDrv_in > CPU_HD_DISK_MAX - 1) || (uiHead_in > 15)) {
		CPUExt_CorePanic("[cpu_hd_Out][Trying to write bad sector]");
	}
	if (!cpu_hd_IsControllerReady()) {
		CPUExt_CorePanic("[cpu_hd_Out][HD Controller not ready]");
	}
	
	cpu_hd_stCtl.pfnISR = pfnISR_in;
	_asm_outb_p(cpu_hd_stCtl.astDisk[uiDrv_in].iCtl, X86_HD_CMD);
	regPort = X86_HD_DATA;
	_asm_outb_p(cpu_hd_stCtl.astDisk[uiDrv_in].iWpcom >> 2, ++regPort);
	_asm_outb_p(uiSecCnt_in, ++regPort);
	_asm_outb_p(uiSec_in, ++regPort);
	_asm_outb_p(uiCyl_in, ++regPort);
	_asm_outb_p(uiCyl_in >> 8, ++regPort);
	_asm_outb_p(0xA0|(uiDrv_in << 4)|uiHead_in, ++regPort);
	_asm_outb(uiCmd_in, ++regPort);
}

CPU_PRIVATE CPU_INT32S  cpu_hd_IsControllerReady(void)
{
	CPU_INT32S  iRetries = 0;
	
	for (iRetries = 10000; iRetries > 0; --iRetries) {
		if (0x40 == (_asm_inb_p(X86_HD_STATUS)&0xC0)) {
			break;
		}
	}
	
	return (iRetries);
}

CPU_PRIVATE CPU_INT32S  cpu_hd_IsWinResultNG(void)
{
	CPU_INT32S iStatus = _asm_inb_p(X86_HD_STATUS);
	
	if ((iStatus & (X86_HD_STATUS_BUSY | X86_HD_STATUS_READY | X86_HD_STATUS_WRERR | X86_HD_STATUS_SEEK | X86_HD_STATUS_ERR))
	== (X86_HD_STATUS_READY | X86_HD_STATUS_SEEK)) {
		return(0); /* ok */
	}
	
	if (iStatus & X86_HD_STATUS_ERR) {
		iStatus = _asm_inb(X86_HD_ERROR);
	}
	
	return (1);
}

CPU_PRIVATE CPU_INT32S  cpu_hd_IsDriverBusy(void)
{
	CPU_INT32U  i = 0;
	
	for (i = 0; i < 10000; ++i) {
		if (X86_HD_STATUS_READY 
		== (_asm_inb_p(X86_HD_STATUS) & (X86_HD_STATUS_BUSY|X86_HD_STATUS_READY))) {
			break;
		}
	}
	i = _asm_inb(X86_HD_STATUS);
	i &= (X86_HD_STATUS_BUSY|X86_HD_STATUS_READY|X86_HD_STATUS_SEEK);
	if (i == (X86_HD_STATUS_READY|X86_HD_STATUS_SEEK)) {
		return (0);
	}
	
	CPUExt_DispPrint("[cpu_hd_IsDriverBusy][HD Controller times out] \r\n");
	return (1);
}


CPU_PRIVATE void cpu_hd_BadRWInterrupt(void)
{
	++(cpu_hd_stCtl.pstCurrentReq->iErrCnt);
	
	if (cpu_hd_stCtl.pstCurrentReq->iErrCnt >= CPU_HD_ERROR_MAX) {
		cpu_hd_EndRequest(CPU_EXT_HD_RESULT_IO);
	}
	
	if (cpu_hd_stCtl.pstCurrentReq->iErrCnt > (CPU_HD_ERROR_MAX/2)) {
		cpu_hd_stCtl.iNeedReset = 1;
	}
}

CPU_PRIVATE void cpu_hd_ISRUnexpected(void)
{
	CPUExt_DispPrint("[cpu_hd_ISRUnexpected][Unexpected HD Interrupt] \r\n");
}

CPU_PRIVATE void cpu_hd_ISRRecall(void)
{
	if (cpu_hd_IsWinResultNG()) {
		cpu_hd_BadRWInterrupt();
	}
	cpu_hd_DoRequest();
}

CPU_PRIVATE void cpu_hd_ISRRead(void)
{
	if (cpu_hd_IsWinResultNG()) {
		cpu_hd_BadRWInterrupt();
		cpu_hd_DoRequest();
		return;
	}
	
	_CPU_HD_PORT_READ(X86_HD_DATA, cpu_hd_stCtl.pstCurrentReq->stReq.pbyData, 256);
	cpu_hd_stCtl.pstCurrentReq->iErrCnt = 0;
	cpu_hd_stCtl.pstCurrentReq->stReq.pbyData += 512;
	cpu_hd_stCtl.pstCurrentReq->uiSectorStart++;
	cpu_hd_stCtl.pstCurrentReq->uiSectorCount--;
	
	if (cpu_hd_stCtl.pstCurrentReq->uiSectorCount > 0) {
		cpu_hd_stCtl.pfnISR = &cpu_hd_ISRRead;
		return;
	}
	
	cpu_hd_EndRequest(CPU_EXT_HD_RESULT_OK);
	cpu_hd_DoRequest();
}

CPU_PRIVATE void cpu_hd_ISRWrite(void)
{
	if (cpu_hd_IsWinResultNG()) {
		cpu_hd_BadRWInterrupt();
		cpu_hd_DoRequest();
		return;
	}
	if (--(cpu_hd_stCtl.pstCurrentReq->uiSectorCount)) {
		++(cpu_hd_stCtl.pstCurrentReq->uiSectorStart);
		cpu_hd_stCtl.pstCurrentReq->stReq.pbyData += 512;
		cpu_hd_stCtl.pfnISR = &cpu_hd_ISRWrite;
		_CPU_HD_PORT_WRITE(X86_HD_DATA, cpu_hd_stCtl.pstCurrentReq->stReq.pbyData, 256);
		return;
	}
	
	cpu_hd_EndRequest(CPU_EXT_HD_RESULT_OK);
	cpu_hd_DoRequest();
}


