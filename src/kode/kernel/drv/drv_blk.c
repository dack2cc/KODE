
/******************************************************************************
    Include
******************************************************************************/

#include <drv_blk.h>
#include <drv_lock.h>
#include <drv_rd.h>
#include <drv_disp.h>
#include <cpu_ext.h>
#include <os.h>
#include <std/stdarg.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

#define DRV_BLK_CMD_READ         (CPU_EXT_HD_CMD_READ)
#define DRV_BLK_CMD_WRITE        (CPU_EXT_HD_CMD_WRITE)
#define DRV_BLK_CMD_READ_AHEAD   (DRV_BLK_CMD_WRITE + 1)
#define DRV_BLK_CMD_WRITE_AHEAD  (DRV_BLK_CMD_WRITE + 2)

#define DRV_BLK_LCK_NAME_FREE_REQ  "[DriverBlock][LockFreeReq]"
#define DRV_BLK_LCK_NAME_FREE_BUF  "[DriverBlock][LockFreeBuf]"
#define DRV_BLK_LCK_NAME_BUFFER    "[DriverBlock][LockBuffer]"

#define DRV_BLK_UNIT_SIZE       (CPU_EXT_HD_BLOCK_SIZE)
#define DRV_BLK_HASH_TABLE_MAX  (307)

typedef struct _DRV_BLK_BUFFER_HEAD {
	/* it must the same as CPU_EXT_HD_REQUEST */
	DRV_BLK_BUFFER  stBuf;
	CPU_INT32S      iCmd;
	CPU_INT32S      iResult;
	/* it must the same as CPU_EXT_HD_REQUEST */

	CPU_INT08U      uiRef;        /* users using this block */
	CPU_INT08U      uiIsUpToDate; /* 0 - out of date, 1 - up to date */
	CPU_INT08U      uiIsDirty;    /* 0 - clean, 1 - dirty */
	CPU_INT08U      uiIsLocked;   /* 0 - ok, 1 -locked */	
	DRV_LOCK        lckWait;
	struct _DRV_BLK_BUFFER_HEAD * pstPrev;
	struct _DRV_BLK_BUFFER_HEAD * pstNext;
	struct _DRV_BLK_BUFFER_HEAD * pstPrevFree;
	struct _DRV_BLK_BUFFER_HEAD * pstNextFree;

} DRV_BLK_BUFFER_HEAD;


typedef struct _DRV_BLK_CONTROL {
	DRV_BLK_BUFFER_HEAD*  apstHashTable[DRV_BLK_HASH_TABLE_MAX];
	DRV_BLK_BUFFER_HEAD*  pstBufHeadStart;
	DRV_BLK_BUFFER_HEAD*  pstBufHeadFree;
	CPU_INT32U            uiBufCount;
	CPU_FNCT_VOID         pfnSync;
	DRV_LOCK              lckBufFree;
	DRV_LOCK              lckReqFree;
} DRV_BLK_CONTROL;


DRV_PRIVATE DRV_BLK_CONTROL drv_blk_stCtl;


/******************************************************************************
    Private Interface
******************************************************************************/

#define _hash_fn(dev, blk)  (((unsigned)(dev^blk))%DRV_BLK_HASH_TABLE_MAX)
#define _hash(dev, blk)     (drv_blk_stCtl.apstHashTable[_hash_fn(dev, blk)])

#define _badness(bh)     (((bh)->uiIsDirty<<1)+(bh)->uiIsLocked)

DRV_PRIVATE void drv_blk_WaitForFree(void);
DRV_PRIVATE void drv_blk_WaitOnBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_in);
DRV_PRIVATE void drv_blk_LockBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_in);
DRV_PRIVATE void drv_blk_UnlockBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_in);
DRV_PRIVATE void drv_blk_LowLevelRW(const CPU_INT32S iCmdRW_in, DRV_BLK_BUFFER_HEAD* pstBufHead_in);

DRV_PRIVATE void drv_blk_RemoveFromQueues(DRV_BLK_BUFFER_HEAD* pstBufHead_in);
DRV_PRIVATE void drv_blk_InsertIntoQueues(DRV_BLK_BUFFER_HEAD* pstBufHead_in);

DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_GetBlock(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in);
DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_HashTableGet(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in);
DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_HashTableFind(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_blk_Init(void)
{
	CPU_ADDR  adrPhyBufStart = 0;
	CPU_ADDR  adrPhyBufEnd = 0;
	DRV_BLK_BUFFER_HEAD * pstBufHead = 0;
	void * pBuffer = 0;
	CPU_INT32U  i  = 0;
	
	CPUExt_PageGetBufferSpace(&adrPhyBufStart, &adrPhyBufEnd);
	if (0 == adrPhyBufStart) {
		CPUExt_CorePanic("[drv_blk_Init][buffer space start address is invalid]");
	}
	if (adrPhyBufEnd <= adrPhyBufStart) {
		CPUExt_CorePanic("[drv_blk_Init][buffer space end address is invalid]");
	}
	
	drv_blk_stCtl.pstBufHeadStart = (DRV_BLK_BUFFER_HEAD *)adrPhyBufStart;
    drv_blk_stCtl.pstBufHeadFree = 0;
	drv_blk_stCtl.uiBufCount = 0;
	drv_blk_stCtl.pfnSync = 0;

	drv_lock_Init(&(drv_blk_stCtl.lckBufFree), DRV_BLK_LCK_NAME_FREE_BUF);
	drv_lock_Init(&(drv_blk_stCtl.lckReqFree), DRV_BLK_LCK_NAME_FREE_REQ);
	
	pstBufHead = drv_blk_stCtl.pstBufHeadStart;
	
	/*
	    640KB - 1MB are used by Video RAM and  BIOS.
	    So it must be skipped.
	*/
	if (adrPhyBufEnd == 1<<20) {
		pBuffer = (void *)(640 * 1024);
	}
	else {
		pBuffer = (void *)adrPhyBufEnd;
	}
	
	pBuffer -= DRV_BLK_UNIT_SIZE;
	while (pBuffer >= ((void *)(pstBufHead + 1))) {
		pstBufHead->stBuf.pbyData = pBuffer;
		pstBufHead->stBuf.uiBlkIdx = 0;
		pstBufHead->stBuf.iDev = 0;
		pstBufHead->iCmd = 0;
		pstBufHead->iResult = 0;
		pstBufHead->uiIsUpToDate = 0;
		pstBufHead->uiIsDirty = 0;
		pstBufHead->uiIsLocked = 0;
		pstBufHead->uiRef = 0;
		
		drv_lock_Init(&(pstBufHead->lckWait), DRV_BLK_LCK_NAME_BUFFER);
		
		pstBufHead->pstPrev = 0;
		pstBufHead->pstNext = 0;
		pstBufHead->pstPrevFree = pstBufHead - 1;
		pstBufHead->pstNextFree = pstBufHead + 1;
		pstBufHead++;
		drv_blk_stCtl.uiBufCount++;
	    
		/*
	        640KB - 1MB are used by Video RAM and  BIOS.
	        So it must be skipped.
	    */
		if (pBuffer == (void *)(0x100000)) {
			pBuffer = (void *)0xA0000;
		}
		
		pBuffer -= DRV_BLK_UNIT_SIZE;
	}
	--pstBufHead;
	drv_blk_stCtl.pstBufHeadFree = drv_blk_stCtl.pstBufHeadStart;
	drv_blk_stCtl.pstBufHeadFree->pstPrevFree = pstBufHead;
	pstBufHead->pstNextFree = drv_blk_stCtl.pstBufHeadFree;
	
	for (i = 0; i < DRV_BLK_HASH_TABLE_MAX; ++i) {
	    drv_blk_stCtl.apstHashTable[i] = 0;
	}
	
	return;
}

void drv_blk_RegisterSync(CPU_FNCT_VOID pfnSync_in)
{
	drv_blk_stCtl.pfnSync = pfnSync_in;
}

void drv_blk_InvalidateBuffer(const CPU_INT32S iDev_in)
{
	CPU_INT32U i = 0;
	DRV_BLK_BUFFER_HEAD * pstBufHead = 0;
	
	pstBufHead = drv_blk_stCtl.pstBufHeadStart;
	for (i = 0; i < drv_blk_stCtl.uiBufCount; ++i) {
		if (pstBufHead->stBuf.iDev != iDev_in) {
			continue;
		}
		drv_blk_WaitOnBuffer(pstBufHead);
		if (pstBufHead->stBuf.iDev == iDev_in) {
			pstBufHead->uiIsUpToDate = 0;
			pstBufHead->uiIsDirty = 0;
		}
	}
}


DRV_BLK_BUFFER * drv_blk_Read(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = 0;
	
	if (((CPU_EXT_HD_DEVICE & iDev_in) != CPU_EXT_HD_DEVICE) 
	&&  ((DRV_RD_DEVICE & iDev_in) != DRV_RD_DEVICE)) {
		CPUExt_CorePanic("[drv_blk_Read][Unknown Device]\r\n");
		return (0);
	}
	
	pstBufHead = drv_blk_GetBlock(iDev_in, uiBlkIdx_in);
	if (0 == pstBufHead) {
		CPUExt_CorePanic("[drv_blk_Read][Get block failed]");
	}
	if (pstBufHead->uiIsUpToDate) {
		return ((DRV_BLK_BUFFER *)(pstBufHead));
	}
	
	drv_blk_LowLevelRW(DRV_BLK_CMD_READ, pstBufHead);
	drv_blk_WaitOnBuffer(pstBufHead);	
	if (pstBufHead->uiIsUpToDate) {
		return ((DRV_BLK_BUFFER *)(pstBufHead));
	}
	
	drv_blk_Release((DRV_BLK_BUFFER *)pstBufHead);
	return 0;
}

DRV_BLK_BUFFER * drv_blk_ReadAhead(const CPU_INT32S iDev_in, const CPU_INT32S iBlkIdxFst_in, ...)
{
	va_list               args;
	DRV_BLK_BUFFER_HEAD * pstBHFst = 0;
	DRV_BLK_BUFFER_HEAD * pstBHTmp = 0;
	CPU_INT32S            iBlkIdx  = 0;
	
	va_start(args, iBlkIdxFst_in);
	pstBHFst = drv_blk_GetBlock(iDev_in, iBlkIdxFst_in);
	if (0 == pstBHFst) {
		CPUExt_CorePanic("[drv_blk_ReadAhead][Get Block failed]");
	}
	if (!(pstBHFst->uiIsUpToDate)) {
		drv_blk_LowLevelRW(DRV_BLK_CMD_READ, pstBHFst);
	}
	while ((iBlkIdx = va_arg(args, CPU_INT32S) >= 0)) {
		pstBHTmp = drv_blk_GetBlock(iDev_in, iBlkIdx);
		if ((pstBHTmp) && !(pstBHTmp->uiIsUpToDate)) {
			drv_blk_LowLevelRW(DRV_BLK_CMD_READ_AHEAD, pstBHTmp);
			(pstBHTmp->uiRef)--;
		}
	}
	va_end();
	
	drv_blk_WaitOnBuffer(pstBHFst);
	if (pstBHFst->uiIsUpToDate) {
		return ((DRV_BLK_BUFFER *)pstBHFst);
	}
	drv_blk_Release((DRV_BLK_BUFFER *)pstBHFst);
	return 0;
}


void drv_blk_Release(DRV_BLK_BUFFER* pstBuf_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = (DRV_BLK_BUFFER_HEAD *)pstBuf_in;
	if (0 == pstBufHead) {
		CPUExt_CorePanic("[drv_blk_Release][buffer invalid]");
		return;
	}
	
	drv_blk_WaitOnBuffer(pstBufHead);
	if (!(pstBufHead->uiRef--)) {
		CPUExt_CorePanic("[drv_blk_Release][The buffer is free]");
	}
	drv_lock_WakeUp(&(drv_blk_stCtl.lckBufFree));
}

void drv_blk_MakeDirty(DRV_BLK_BUFFER* pstBuf_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = (DRV_BLK_BUFFER_HEAD *)pstBuf_in;
	if (0 == pstBufHead) {
		CPUExt_CorePanic("[drv_blk_Release][buffer invalid]");
		return;
	}
	
	pstBufHead->uiIsDirty = 1;
}

void drv_blk_Free(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD * pstBufHead = drv_blk_HashTableGet(iDev_in, uiBlkIdx_in);
	
	if (0 != pstBufHead) {
		if (pstBufHead->uiRef != 1) {
			drv_disp_Printf("[drv_blk_Free][%04x:%d][uiRef:%d]\r\n",iDev_in, uiBlkIdx_in, pstBufHead->uiRef);
		}
		pstBufHead->uiIsDirty = 0;
		pstBufHead->uiIsUpToDate = 0;
		drv_blk_Release((DRV_BLK_BUFFER *)pstBufHead);
	}
}

void drv_blk_NotifyRWEnd(DRV_BLK_BUFFER* pstBuf_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = (DRV_BLK_BUFFER_HEAD *)pstBuf_in;
	
	if (0 == pstBufHead) {
		CPUExt_CorePanic("[drv_blk_NotifyRWEnd][Buffer is invalid]");
	}
	
	if (CPU_EXT_HD_RESULT_OK == pstBufHead->iResult) {
		pstBufHead->uiIsUpToDate = 1;
	}
	drv_blk_UnlockBuffer(pstBufHead);
}


void drv_blk_NotifyReqFree(void)
{
	drv_lock_WakeUp(&(drv_blk_stCtl.lckReqFree));
}

DRV_PRIVATE void drv_blk_WaitForFree(void)
{
	drv_lock_SleepOn(&(drv_blk_stCtl.lckBufFree));
}

DRV_PRIVATE void drv_blk_WaitOnBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_in)
{
	CPU_SR_ALLOC();
	
	OS_CRITICAL_ENTER();
	
	while (pstBufHead_in->uiIsLocked) {
		drv_lock_SleepOn(&(pstBufHead_in->lckWait));
	}
	
	OS_CRITICAL_EXIT();
}

DRV_PRIVATE void drv_blk_LockBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_inout)
{
	CPU_SR_ALLOC();
	
	if (0 == pstBufHead_inout) {
		CPUExt_CorePanic("[drv_blk_LockBuffer][the buffer is invalid]");
	}
	
	OS_CRITICAL_ENTER();
	
	while (pstBufHead_inout->uiIsLocked) {
		drv_lock_SleepOn(&(pstBufHead_inout->lckWait));
	}
	pstBufHead_inout->uiIsLocked = 1;
	OS_CRITICAL_EXIT();
}

DRV_PRIVATE void drv_blk_UnlockBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_inout)
{
	if (0 == pstBufHead_inout) {
		CPUExt_CorePanic("[drv_blk_UnlockBuffer][the buffer is invalid]");
	}
	if (!(pstBufHead_inout->uiIsLocked)) {
		CPUExt_CorePanic("[drv_blk_UnlockBuffer][the buffer is not locked]");
	}
	
	pstBufHead_inout->uiIsLocked = 0;
	drv_lock_WakeUp(&(pstBufHead_inout->lckWait));
}


void drv_blk_SyncDevice(const CPU_INT32S iDev_in)
{
	CPU_INT32U i = 0;
	CPU_FNCT_VOID pfnSync = drv_blk_stCtl.pfnSync;
	DRV_BLK_BUFFER_HEAD*  pstBufHead = 0;
	
	pstBufHead = drv_blk_stCtl.pstBufHeadStart;
	for (i = 0; i < drv_blk_stCtl.uiBufCount; ++i, ++pstBufHead) {
		if (iDev_in != pstBufHead->stBuf.iDev) {
			continue;
		}
		drv_blk_WaitOnBuffer(pstBufHead);
		if ((iDev_in == pstBufHead->stBuf.iDev) 
		&&  (pstBufHead->uiIsDirty)) {
			drv_blk_LowLevelRW(DRV_BLK_CMD_WRITE, pstBufHead);
		}
	}

	if (0 == pfnSync) {
		return;
	}
	pfnSync();
	pstBufHead = drv_blk_stCtl.pstBufHeadStart;
	for (i = 0; i < drv_blk_stCtl.uiBufCount; ++i, ++pstBufHead) {
		if (iDev_in != pstBufHead->stBuf.iDev) {
			continue;
		}
		drv_blk_WaitOnBuffer(pstBufHead);
		if ((iDev_in == pstBufHead->stBuf.iDev) 
		&&  (pstBufHead->uiIsDirty)) {
			drv_blk_LowLevelRW(DRV_BLK_CMD_WRITE, pstBufHead);
		}
	}	
}

DRV_PRIVATE void drv_blk_LowLevelRW(const CPU_INT32S iCmdRW_in, DRV_BLK_BUFFER_HEAD* pstBufHead_inout)
{
	CPU_INT08S  iIsRWAhead = 0;
	
	if (0 == pstBufHead_inout) {
		CPUExt_CorePanic("[drv_blk_LowLevelRW][buffer is invalid]");
	}
	
	pstBufHead_inout->iCmd = iCmdRW_in;
	iIsRWAhead = ((DRV_BLK_CMD_READ_AHEAD == iCmdRW_in) || (DRV_BLK_CMD_WRITE_AHEAD == iCmdRW_in));
	
	if (iIsRWAhead) {
		if (pstBufHead_inout->uiIsLocked) {
			return;
		}
		if (DRV_BLK_CMD_READ_AHEAD == iCmdRW_in) {
			pstBufHead_inout->iCmd = DRV_BLK_CMD_READ;
		}
		else {
			pstBufHead_inout->iCmd = DRV_BLK_CMD_WRITE;
		}
	}
	
	if ((DRV_BLK_CMD_READ  != pstBufHead_inout->iCmd) 
    &&  (DRV_BLK_CMD_WRITE != pstBufHead_inout->iCmd)) {
    	CPUExt_CorePanic("[drv_blk_LowLevelRW][command is invalid]");
	}
	
	drv_blk_LockBuffer(pstBufHead_inout);
	if (((DRV_BLK_CMD_READ  == pstBufHead_inout->iCmd) && (pstBufHead_inout->uiIsUpToDate))
	||  ((DRV_BLK_CMD_WRITE == pstBufHead_inout->iCmd) && !(pstBufHead_inout->uiIsDirty))) {
		drv_blk_UnlockBuffer(pstBufHead_inout);
		return;
	}
	
L_REPEAT_LL_RW:
	if ((CPU_EXT_HD_DEVICE & pstBufHead_inout->stBuf.iDev) == CPU_EXT_HD_DEVICE) {
	    CPUExt_HDRequest((CPU_EXT_HD_REQUEST *)pstBufHead_inout);
	}
	else if  ((DRV_RD_DEVICE & pstBufHead_inout->stBuf.iDev) == DRV_RD_DEVICE) {
		drv_rd_Request((CPU_EXT_HD_REQUEST *)pstBufHead_inout);
	}
	else {
		CPUExt_CorePanic("[drv_blk_LowLevelRW][Unknown Device]");
	}

	if (CPU_EXT_HD_RESULT_FULL == pstBufHead_inout->iResult) {
		if (iIsRWAhead) {
			drv_blk_UnlockBuffer(pstBufHead_inout);
			return;
		}
		drv_lock_SleepOn(&(drv_blk_stCtl.lckReqFree));
		goto L_REPEAT_LL_RW;
	}
}

DRV_PRIVATE void drv_blk_RemoveFromQueues(DRV_BLK_BUFFER_HEAD* pstBufHead_in)
{
	/* remove from hash-queue */
	if (pstBufHead_in->pstNext) {
		pstBufHead_in->pstNext->pstPrev = pstBufHead_in->pstPrev;
	}
	if (pstBufHead_in->pstPrev) {
		pstBufHead_in->pstPrev->pstNext = pstBufHead_in->pstNext;
	}
	if (pstBufHead_in == _hash(pstBufHead_in->stBuf.iDev, pstBufHead_in->stBuf.uiBlkIdx)) {
		_hash(pstBufHead_in->stBuf.iDev, pstBufHead_in->stBuf.uiBlkIdx) = pstBufHead_in->pstNext;
	}
	
	/* remove from free list */
	if (!(pstBufHead_in->pstPrevFree) || !(pstBufHead_in->pstNextFree)) {
		CPUExt_CorePanic("[drv_blk_RemoveFromQueues][Free list corrupted]");
	}
	pstBufHead_in->pstPrevFree->pstNextFree = pstBufHead_in->pstNextFree;
	pstBufHead_in->pstNextFree->pstPrevFree = pstBufHead_in->pstPrevFree;
	if (pstBufHead_in == drv_blk_stCtl.pstBufHeadFree) {
		drv_blk_stCtl.pstBufHeadFree = pstBufHead_in->pstNextFree;
	}
}

DRV_PRIVATE void drv_blk_InsertIntoQueues(DRV_BLK_BUFFER_HEAD* pstBufHead_in)
{
	/* put at end of free list */
	pstBufHead_in->pstNextFree = drv_blk_stCtl.pstBufHeadFree;
	pstBufHead_in->pstPrevFree = drv_blk_stCtl.pstBufHeadFree->pstPrevFree;
	drv_blk_stCtl.pstBufHeadFree->pstPrevFree->pstNextFree = pstBufHead_in;
	drv_blk_stCtl.pstBufHeadFree->pstPrevFree = pstBufHead_in;
	
	/* put the buffer in new hash-queue if it has a device */
	pstBufHead_in->pstPrev = 0;
	pstBufHead_in->pstNext = 0;
	if (0 == (pstBufHead_in->stBuf.iDev)) {
		return;
	}
	pstBufHead_in->pstNext = _hash(pstBufHead_in->stBuf.iDev, pstBufHead_in->stBuf.uiBlkIdx);
	_hash(pstBufHead_in->stBuf.iDev, pstBufHead_in->stBuf.uiBlkIdx) = pstBufHead_in;
	pstBufHead_in->pstNext->pstPrev = pstBufHead_in;
}

DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_GetBlock(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = 0;
	DRV_BLK_BUFFER_HEAD* pstBufHeadFree = 0;
	
L_REPEAT_GET_BLOCK:
	pstBufHead = drv_blk_HashTableGet(iDev_in, uiBlkIdx_in);
	if (0 != pstBufHead) {
		return (pstBufHead);
	}
	
	pstBufHeadFree = drv_blk_stCtl.pstBufHeadFree;
	do {
		if (pstBufHeadFree->uiRef) {
			continue;
		}
		if ((!pstBufHead) 
		||  (_badness(pstBufHeadFree) < _badness(pstBufHead))) {
			pstBufHead = pstBufHeadFree;
			if (!_badness(pstBufHead)) {
				break;
			}
		}
		pstBufHeadFree = pstBufHeadFree->pstNextFree;
		
	/* and repeat until we find something good */
	} while (pstBufHeadFree != drv_blk_stCtl.pstBufHeadFree);
	
	if (0 == pstBufHead) {
		drv_blk_WaitForFree();
		goto L_REPEAT_GET_BLOCK;
	}
	
	drv_blk_WaitOnBuffer(pstBufHead);
	if (pstBufHead->uiRef) {
		goto L_REPEAT_GET_BLOCK;
	}
	
	while (pstBufHead->uiIsDirty) {
		drv_blk_SyncDevice(pstBufHead->stBuf.iDev);
		drv_blk_WaitOnBuffer(pstBufHead);
		if (pstBufHead->uiRef) {
			goto L_REPEAT_GET_BLOCK;
		}
	}

    /* NOTE!! While we slept waiting for this block, somebody else might */
    /* already have added "this" block to the cache. check it */
	if (drv_blk_HashTableFind(iDev_in, uiBlkIdx_in)) {
		goto L_REPEAT_GET_BLOCK;
	}

	/* OK, FINALLY we know that this buffer is the only one of it's kind, */
    /* and that it's unused (b_count=0), unlocked (b_lock=0), and clean */
	pstBufHead->uiRef = 1;
	pstBufHead->uiIsDirty = 0;
	pstBufHead->uiIsUpToDate = 0;
	drv_blk_RemoveFromQueues(pstBufHead);
	pstBufHead->stBuf.iDev = iDev_in;
	pstBufHead->stBuf.uiBlkIdx = uiBlkIdx_in;
	drv_blk_InsertIntoQueues(pstBufHead);
	
	return (pstBufHead);
}

DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_HashTableGet(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = 0;
	
	for (;;) {
		pstBufHead = drv_blk_HashTableFind(iDev_in, uiBlkIdx_in);
		if (0 == pstBufHead) {
			return (0);
		}
		pstBufHead->uiRef++;
		drv_blk_WaitOnBuffer(pstBufHead);
		if ((iDev_in == pstBufHead->stBuf.iDev) 
		&&  (uiBlkIdx_in == pstBufHead->stBuf.uiBlkIdx)) {
			return (pstBufHead);
		}
		pstBufHead->uiRef--;
	}
}

DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_HashTableFind(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = 0;
	
	for ( pstBufHead = _hash(iDev_in, uiBlkIdx_in);
		  pstBufHead != 0;
		  pstBufHead = pstBufHead->pstNext ) 
	{
		if ((iDev_in == pstBufHead->stBuf.iDev)
		&&  (uiBlkIdx_in == pstBufHead->stBuf.uiBlkIdx)) {
			return (pstBufHead);
		}
	}
	
	return 0;
}


