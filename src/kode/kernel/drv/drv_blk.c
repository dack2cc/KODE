
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

/******************************************************************************
    Private Interface
******************************************************************************/

typedef struct _DRV_BLK_BUFFER_HEAD {
	/* it must the same as CPU_EXT_HD_REQUEST */
	DRV_BLK_BUFFER  stBuf;
	CPU_INT32S      iCmd;
	CPU_INT32S      iResult;
	/* it must the same as CPU_EXT_HD_REQUEST */
	
	CPU_INT08U      uiIsUpToDate; /* 0 - out of date, 1 - up to date */
	CPU_INT08U      uiIsDirty;    /* 0 - clean, 1 - dirty */
	CPU_INT08U      uiIsLocked;   /* 0 - ok, 1 -locked */	
	CPU_INT08U      uiCount;      /* users using this block */
	//struct task_struct * b_wait;
	struct _DRV_BLK_BUFFER_HEAD * pstPrev;
	struct _DRV_BLK_BUFFER_HEAD * pstNext;
	struct _DRV_BLK_BUFFER_HEAD * pstPrevFree;
	struct _DRV_BLK_BUFFER_HEAD * pstNextFree;

} DRV_BLK_BUFFER_HEAD;


/******************************************************************************
    Function Definition
******************************************************************************/

void drv_blk_Init(void)
{
	CPU_ADDR  adrPhyBufStart = 0;
	CPU_ADDR  adrPhyBufEnd = 0;
	
	CPUExt_PageGetBufferSpace(&adrPhyBufStart, &adrPhyBufEnd);
	if (0 == adrPhyBufStart) {
		CPUExt_CorePanic("[drv_blk_Init][buffer space start address is invalid]");
	}
	if (adrPhyBufEnd <= adrPhyBufStart) {
		CPUExt_CorePanic("[drv_blk_Init][buffer space end address is invalid]");
	}
	
	return;
}


