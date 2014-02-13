
/******************************************************************************
    Include
******************************************************************************/

#include <drv_blk.h>
#include <cpu_ext.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

#define DRV_BLK_NAME_WAIT_BUFFER   "DriverBlockBufferWait"
#define DRV_BLK_NAME_LOCK       "DriverBlockLock"


#define DRV_BLK_UNIT_SIZE       (1024)
#define DRV_BLK_HASH_TABLE_MAX  (307)

typedef struct _DRV_BLK_BUFFER_HEAD {
	/* it must the same as CPU_EXT_HD_REQUEST */
	DRV_BLK_BUFFER  stBuf;
	CPU_INT32S      iCmd;
	CPU_INT32S      iResult;
	/* it must the same as CPU_EXT_HD_REQUEST */

	CPU_INT08U      uiRef;      /* users using this block */
	CPU_INT08U      uiIsUpToDate; /* 0 - out of date, 1 - up to date */
	CPU_INT08U      uiIsDirty;    /* 0 - clean, 1 - dirty */
	CPU_INT08U      uiIsLocked;   /* 0 - ok, 1 -locked */	
	OS_SEM          stLock;
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
	OS_SEM                stBufWait;
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
	OS_ERR  err = OS_ERR_NONE;
	
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

	OSSemCreate(
		/* p_sem  */ &(drv_blk_stCtl.stBufWait),
		/* p_name */ DRV_BLK_NAME_WAIT_BUFFER,
		/* cnt    */ 0,
		/* *p_err */ &err
	);
	if (OS_ERR_NONE != err) {
		CPUExt_CorePanic("[drv_blk_Init][Buffer wait init failed]");
	}

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
		
		OSSemCreate(
			/* p_sem  */ &(pstBufHead->stLock),
			/* p_name */ DRV_BLK_NAME_LOCK,
			/* cnt    */ 0,
			/* *p_err */ &err
		);
		if (OS_ERR_NONE != err) {
			CPUExt_CorePanic("[drv_blk_Init][Block lock init failed]");
		}
		
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

DRV_BLK_BUFFER * drv_blk_Read(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = 0;
	
	pstBufHead = drv_blk_GetBlock(iDev_in, uiBlkIdx_in);
	if (0 != pstBufHead) {
		return ((DRV_BLK_BUFFER *)(pstBufHead));
	}
	
	
	
	return 0;
}

DRV_PRIVATE void drv_blk_WaitForFree(void)
{
	OS_ERR err = OS_ERR_NONE;
	
	OSSemPend(
		/* p_sem   */ &(drv_blk_stCtl.stBufWait),
		/* timeout */ 0,
		/* opt     */ OS_OPT_PEND_BLOCKING,
		/* p_ts    */ 0,
		/* p_err   */ &err
	);
	if (OS_ERR_NONE != err) {
		CPUExt_CorePanic("[drv_blk_WaitOnBuffer][Lock error]");
	}
}

DRV_PRIVATE void drv_blk_WaitOnBuffer(DRV_BLK_BUFFER_HEAD* pstBufHead_in)
{
	OS_ERR err = OS_ERR_NONE;
	CPU_SR_ALLOC();
	
	OS_CRITICAL_ENTER();
	
	while (pstBufHead_in->uiIsLocked) {
		OSSemPend(
			/* p_sem   */ &(pstBufHead_in->stLock),
			/* timeout */ 0,
			/* opt     */ OS_OPT_PEND_BLOCKING,
			/* p_ts    */ 0,
			/* p_err   */ &err
		);
		if (OS_ERR_NONE != err) {
			CPUExt_CorePanic("[drv_blk_WaitOnBuffer][Lock error]");
		}
	}
	
	OS_CRITICAL_EXIT();
}

DRV_PRIVATE DRV_BLK_BUFFER_HEAD*  drv_blk_GetBlock(const CPU_INT32S iDev_in, const CPU_INT32U uiBlkIdx_in)
{
	DRV_BLK_BUFFER_HEAD* pstBufHead = 0;
	DRV_BLK_BUFFER_HEAD* pstBufHeadFree = 0;
	
L_REPEAT:
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
	} while (pstBufHeadFree != drv_blk_stCtl.pstBufHeadFree);
	
	if (0 == pstBufHead) {
		drv_blk_WaitForFree();
		goto L_REPEAT;
	}
	
	drv_blk_WaitOnBuffer(pstBufHead);
	if (pstBufHead->uiRef) {
		goto L_REPEAT;
	}
	
	
	
	return 0;
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


