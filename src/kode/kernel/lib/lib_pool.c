
/******************************************************************************
    Include
******************************************************************************/

#include <lib_pool.h>
//#include <cpu_boot.h>
#include <cpu_ext.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define LIB_PRIVATE  static
#define LIB_PRIVATE 

#define _LIB_POOL_PAGE_SIZE  (4*1024) // 4 KB

/*
    Memory Manager for the size below Page Size.
*/

typedef struct _LIB_POOL_DESC {
	struct  _LIB_POOL_DESC*  pstNext;
	OS_MEM  stPool;
} _LIB_POOL_DESC;

typedef struct _LIB_POOL_DIR {
	CPU_SIZE_T  uiSize;
	struct _LIB_POOL_DESC*  pstChain;
} _LIB_POOL_DIR;

LIB_PRIVATE _LIB_POOL_DIR  lib_pool_astDir[] = 
{
	{16,   (struct _LIB_POOL_DESC *)0},
	{32,   (struct _LIB_POOL_DESC *)0},
	{64,   (struct _LIB_POOL_DESC *)0},
	{128,  (struct _LIB_POOL_DESC *)0},
	{256,  (struct _LIB_POOL_DESC *)0},
	{512,  (struct _LIB_POOL_DESC *)0},
	{1024, (struct _LIB_POOL_DESC *)0},
	{2048, (struct _LIB_POOL_DESC *)0},
	{4096, (struct _LIB_POOL_DESC *)0},
	{0,    (struct _LIB_POOL_DESC *)0}
};

LIB_PRIVATE _LIB_POOL_DESC* lib_pool_pstDescFree = (_LIB_POOL_DESC*)0;

/*
    Memory Manager above the Page Size
*/

#define _LIB_POOL_FREE_MAX  (4096)

typedef struct _LIB_POOL_FREE {
	CPU_ADDR   adrPhy;
	CPU_SIZE_T size;
} _LIB_POOL_FREE;

typedef struct _LIB_POOL_CONTROL {
	CPU_INT32U      uiLostSize;
	CPU_INT32U      uiLostCnt;
	CPU_INT32U      uiFreeMax;
	CPU_INT32U      uiFreeCnt;
	_LIB_POOL_FREE  astFree[_LIB_POOL_FREE_MAX];
} _LIB_POOL_CONTROL;

LIB_PRIVATE  _LIB_POOL_CONTROL  lib_pool_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

LIB_PRIVATE  void* lib_pool_MallocSEPage(CPU_SIZE_T size_in);
LIB_PRIVATE  void  lib_pool_FreeSEPage(void* pAddr_in, CPU_SIZE_T  size_in);
LIB_PRIVATE  void  lib_pool_GetFreeDesc(void);

LIB_PRIVATE  void* lib_pool_MallocLTPage(CPU_SIZE_T size_in);
LIB_PRIVATE  void  lib_pool_FreeLTPage(void* pAddr_in, CPU_SIZE_T  size_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void  lib_pool_Init(const CPU_ADDR addrStart_in, const CPU_ADDR addrEnd_in)
{
	lib_pool_stCtl.uiLostSize = 0;
	lib_pool_stCtl.uiLostCnt  = 0;
	lib_pool_stCtl.uiFreeMax  = 0;
	lib_pool_stCtl.uiFreeCnt  = 0;
	
	if ((addrEnd_in > addrStart_in) 
	&& ((addrEnd_in - addrStart_in) > _LIB_POOL_PAGE_SIZE)) {
		lib_pool_stCtl.astFree[0].adrPhy = addrStart_in;
		lib_pool_stCtl.astFree[0].size   = addrEnd_in - addrStart_in;
		lib_pool_stCtl.uiFreeCnt = 1;
	}
}

void* lib_pool_Malloc(CPU_SIZE_T  size_in)
{	
	if (size_in > _LIB_POOL_PAGE_SIZE) {
		return (lib_pool_MallocLTPage(size_in));
	}
	else {
		return (lib_pool_MallocSEPage(size_in));
	}
}

void  lib_pool_Free(void* pAddr_in, CPU_SIZE_T  size_in)
{
	if (size_in > _LIB_POOL_PAGE_SIZE) {
		return (lib_pool_FreeLTPage(pAddr_in, size_in));
	}
	else {
		return (lib_pool_FreeSEPage(pAddr_in, size_in));
	}
}

LIB_PRIVATE void* lib_pool_MallocSEPage(CPU_SIZE_T  size_in)
{
	_LIB_POOL_DIR*  pstPoolDir  = 0;
	_LIB_POOL_DESC* pstPoolDesc = 0;
	OS_ERR  os_err = OS_ERR_NONE;
	void *  pAddr  = 0;
	CPU_SR_ALLOC();
	
	for (pstPoolDir = lib_pool_astDir; pstPoolDir->uiSize > 0; ++pstPoolDir) {
		if (pstPoolDir->uiSize >= size_in) {
			break;
		}
	}
	if (0 == (pstPoolDir->uiSize)) {
		CPUExt_CorePanic("[PANIC][lib_pool_Malloc]the size_in is too large");
	}
	
	CPU_INT_DIS();
	
	for (pstPoolDesc = pstPoolDir->pstChain; 0 != pstPoolDesc; pstPoolDesc = pstPoolDesc->pstNext) {
		if (0 != pstPoolDesc->stPool.FreeListPtr) {
			break;
		}
	}
	
	if (0 == pstPoolDesc) {
		CPU_ADDR  addrPhyPage = 0;
		
		if (0 == lib_pool_pstDescFree) {
			lib_pool_GetFreeDesc();
		}
		
		pstPoolDesc = lib_pool_pstDescFree;
		lib_pool_pstDescFree = lib_pool_pstDescFree->pstNext;
		
		CPUExt_PageGetFree(&addrPhyPage);
		if (0 == addrPhyPage) {
			CPUExt_CorePanic("[PANIC][lib_pool_Malloc]No memory");
		}
		
		OSMemCreate
		(
			/* p_mem    */ &(pstPoolDesc->stPool),
			/* p_name   */ 0,
			/* p_addr   */ (void *)addrPhyPage,
			/* n_blks   */ (_LIB_POOL_PAGE_SIZE / pstPoolDir->uiSize),
			/* blk_size */ (pstPoolDir->uiSize),
			/* p_err    */ &os_err
		);
		if (OS_ERR_NONE != os_err) {
			CPUExt_CorePanic("[PANIC][lib_pool_Malloc]Memory Create Error");
		}
		
		pstPoolDesc->pstNext = pstPoolDir->pstChain;
		pstPoolDir->pstChain = pstPoolDesc;
	}
	
	pAddr = OSMemGet(&(pstPoolDesc->stPool), &os_err);
	if (OS_ERR_NONE != os_err) {
		CPUExt_CorePanic("[PANIC][lib_pool_Malloc]Memory Get Error");
	}
	
	CPU_INT_EN();
	
	return (pAddr);
}

LIB_PRIVATE void  lib_pool_FreeSEPage(void* pAddr_in, CPU_SIZE_T  size_in)
{
	_LIB_POOL_DIR*  pstPoolDir  = 0;
	_LIB_POOL_DESC* pstPoolDesc = 0;
	_LIB_POOL_DESC* pstPoolDescPrev = 0;
	CPU_ADDR  addrPhyPage = 0;
	OS_ERR    os_err = OS_ERR_NONE;
	CPU_SR_ALLOC();
	
	addrPhyPage = ((CPU_ADDR)pAddr_in & 0xFFFFF000);
	for (pstPoolDir = lib_pool_astDir; pstPoolDir->uiSize > 0; ++pstPoolDir) {
		pstPoolDescPrev = 0;
		
		if (pstPoolDir->uiSize < size_in) {
			continue;
		}
		
		for (pstPoolDesc = pstPoolDir->pstChain; 0 != pstPoolDesc; pstPoolDesc = pstPoolDesc->pstNext) {
			if (addrPhyPage == (CPU_ADDR)(pstPoolDesc->stPool.AddrPtr)) {
				goto _LABEL_FOUND;
			}
			pstPoolDescPrev = pstPoolDesc;
		}
	}
	CPUExt_CorePanic("[PANIC][lib_pool_Free]Address Exception");
	
_LABEL_FOUND:
	
	CPU_INT_DIS();
	
	OSMemPut
	(
		/* p_mem */ &(pstPoolDesc->stPool),
		/* p_blk */ pAddr_in,
		/* p_err */ &os_err
	);
	if (OS_ERR_NONE != os_err) {
		CPUExt_CorePanic("[PANIC][lib_pool_Free]Memory Free Error");
	}
	
	if (pstPoolDesc->stPool.NbrFree >= pstPoolDesc->stPool.NbrMax) {
		if (((0 != pstPoolDescPrev) && (pstPoolDescPrev->pstNext != pstPoolDesc))
		||  ((0 == pstPoolDescPrev) && (pstPoolDir->pstChain != pstPoolDesc))) {
			for (pstPoolDescPrev = pstPoolDir->pstChain; 0 != pstPoolDescPrev; pstPoolDescPrev = pstPoolDescPrev->pstNext) {
				if (pstPoolDesc == pstPoolDescPrev->pstNext) {
					break;
				}
			}
		}
		
		if (0 != pstPoolDescPrev) {
			pstPoolDescPrev->pstNext = pstPoolDesc->pstNext;
		}
		else {
			if (pstPoolDesc != pstPoolDir->pstChain) {
				CPUExt_CorePanic("[PANIC][lib_pool_Free]Chain ERROR");
			}
			pstPoolDir->pstChain = pstPoolDesc->pstNext;
		}
		
		CPUExt_PageRelease((CPU_ADDR)(pstPoolDesc->stPool.AddrPtr));
		pstPoolDesc->pstNext = lib_pool_pstDescFree;
		lib_pool_pstDescFree = pstPoolDesc;
	}
	
	CPU_INT_EN();
	
	return;
}


LIB_PRIVATE  void  lib_pool_GetFreeDesc(void)
{
	_LIB_POOL_DESC* pstDesc = 0;
	CPU_INT32S  i = 0;
	
	if (0 != lib_pool_pstDescFree) {
		return;
	}
	
	CPUExt_PageGetFree((CPU_ADDR *)(&lib_pool_pstDescFree));
	if (0 == lib_pool_pstDescFree) {
		CPUExt_CorePanic("[PANIC][lib_pool_GetFreeDesc]No Memory");
	}
	
	pstDesc = lib_pool_pstDescFree;
	for (i = 1; i < (_LIB_POOL_PAGE_SIZE / sizeof(_LIB_POOL_DESC)); ++i) {
		pstDesc->pstNext = pstDesc + 1;
		pstDesc++;
	}
	
	return;
}

LIB_PRIVATE  void* lib_pool_MallocLTPage(CPU_SIZE_T size_in)
{
	CPU_SR_ALLOC();

	CPU_INT32U i = 0;
	CPU_ADDR adrPhy = 0;
	
	CPU_INT_DIS();
	
	//drv_disp_Printf("[freeCnt][%d]\r\n", lib_pool_stCtl.uiFreeCnt);	
	
	for (i = 0; i < lib_pool_stCtl.uiFreeCnt; ++i) {
	    //drv_disp_Printf("[free(%d)Siz][%dKB]\r\n", i, lib_pool_stCtl.astFree[0].size / 1024);
	    //drv_disp_Printf("[free(%d)Adr][0x%X]\r\n", i, lib_pool_stCtl.astFree[0].adrPhy);
		if (lib_pool_stCtl.astFree[i].size >= size_in) {
			adrPhy = lib_pool_stCtl.astFree[i].adrPhy;
			lib_pool_stCtl.astFree[i].adrPhy += size_in;
			lib_pool_stCtl.astFree[i].size   -= size_in;
			if (0 == lib_pool_stCtl.astFree[i].size) {
				(lib_pool_stCtl.uiFreeCnt)--;
				for (; (i + 1) < lib_pool_stCtl.uiFreeCnt; ++i) {
					lib_pool_stCtl.astFree[i] = lib_pool_stCtl.astFree[i+1];
				}
			}
			CPU_INT_EN();
			return ((void *)adrPhy);
		}
	}
	
	CPU_INT_EN();
	return ((void *)0);
}

LIB_PRIVATE  void  lib_pool_FreeLTPage(void* pAddr_in, CPU_SIZE_T  size_in)
{
	CPU_SR_ALLOC();
	
	CPU_INT32U i = 0;
	CPU_INT32U j = 0;
	CPU_ADDR adrPhy = (CPU_ADDR)pAddr_in;

	CPU_INT_DIS();

	
	for (i = 0; i < lib_pool_stCtl.uiFreeCnt; ++i) {
		if (lib_pool_stCtl.astFree[i].adrPhy > adrPhy) {
			break;
		}
	}
	
	if (i > 0) {
		if ((lib_pool_stCtl.astFree[i - 1].adrPhy + lib_pool_stCtl.astFree[i - 1].size) == adrPhy) {
			lib_pool_stCtl.astFree[i - 1].size += size_in;
			if (i < lib_pool_stCtl.uiFreeCnt) {
				if ((adrPhy + size_in) == lib_pool_stCtl.astFree[i].adrPhy) {
					lib_pool_stCtl.astFree[i - 1].size += lib_pool_stCtl.astFree[i].size;
					(lib_pool_stCtl.uiFreeCnt)--;
					for (; (i + 1) < lib_pool_stCtl.uiFreeCnt; ++i) {
						lib_pool_stCtl.astFree[i] = lib_pool_stCtl.astFree[i + 1];
					}
				}
			}
			CPU_INT_EN();
			return;
		}
	}
	
	if (i < lib_pool_stCtl.uiFreeCnt) {
		if ((adrPhy + size_in) == lib_pool_stCtl.astFree[i].adrPhy) {
			lib_pool_stCtl.astFree[i].adrPhy = adrPhy;
			lib_pool_stCtl.astFree[i].size += size_in;
			CPU_INT_EN();
			return;
		}
	}
	
	if (lib_pool_stCtl.uiFreeCnt < _LIB_POOL_FREE_MAX) {
		for (j = lib_pool_stCtl.uiFreeCnt; j > i; --j) {
			lib_pool_stCtl.astFree[j] = lib_pool_stCtl.astFree[j - 1];
		}
		(lib_pool_stCtl.uiFreeCnt)++;
		if (lib_pool_stCtl.uiFreeMax < lib_pool_stCtl.uiFreeCnt) {
			lib_pool_stCtl.uiFreeMax = lib_pool_stCtl.uiFreeCnt;
		}
		lib_pool_stCtl.astFree[i].adrPhy = adrPhy;
		lib_pool_stCtl.astFree[i].size  = size_in;
		CPU_INT_EN();
		return;
	}
	
	lib_pool_stCtl.uiLostSize += size_in;
	(lib_pool_stCtl.uiLostCnt)++;
	CPU_INT_EN();
	return;
}


