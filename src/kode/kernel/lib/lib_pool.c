
/******************************************************************************
    Include
******************************************************************************/

#include <lib_pool.h>
#include <cpu_boot.h>
#include <cpu_ext.h>
#include <os.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define LIB_PRIVATE  static

typedef struct _LIB_POOL_DESC {
	struct  _LIB_POOL_DESC*  pstNext;
	OS_MEM  stPool;
} _LIB_POOL_DESC;

typedef struct _LIB_POOL_DIR {
	CPU_SIZE_T  uiSize;
	struct _LIB_POOL_DESC*  pstChain;
} _LIB_POOL_DIR;

LIB_PRIVATE _LIB_POOL_DIR  m_astPoolDir[] = 
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

LIB_PRIVATE _LIB_POOL_DESC* m_pstPoolDescFree = (_LIB_POOL_DESC*)0;

/******************************************************************************
    Private Interface
******************************************************************************/

LIB_PRIVATE  void  lib_pool_GetFreeDesc(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void* lib_pool_Malloc(CPU_SIZE_T  size_in)
{
	_LIB_POOL_DIR*  pstPoolDir  = 0;
	_LIB_POOL_DESC* pstPoolDesc = 0;
	OS_ERR  os_err = OS_ERR_NONE;
	void *  pAddr  = 0;
	CPU_SR_ALLOC();
	
	for (pstPoolDir = m_astPoolDir; pstPoolDir->uiSize > 0; ++pstPoolDir) {
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
		
		if (0 == m_pstPoolDescFree) {
			lib_pool_GetFreeDesc();
		}
		
		pstPoolDesc = m_pstPoolDescFree;
		m_pstPoolDescFree = m_pstPoolDescFree->pstNext;
		
		CPUExt_PageGetFree(&addrPhyPage);
		if (0 == addrPhyPage) {
			CPUExt_CorePanic("[PANIC][lib_pool_Malloc]No memory");
		}
		
		OSMemCreate
		(
			/* p_mem    */ &(pstPoolDesc->stPool),
			/* p_name   */ 0,
			/* p_addr   */ (void *)addrPhyPage,
			/* n_blks   */ (X86_MEM_PAGE_SIZE / pstPoolDir->uiSize),
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

void  lib_pool_Free(void* pAddr_in, CPU_SIZE_T  size_in)
{
	_LIB_POOL_DIR*  pstPoolDir  = 0;
	_LIB_POOL_DESC* pstPoolDesc = 0;
	_LIB_POOL_DESC* pstPoolDescPrev = 0;
	CPU_ADDR  addrPhyPage = 0;
	OS_ERR    os_err = OS_ERR_NONE;
	CPU_SR_ALLOC();
	
	addrPhyPage = ((CPU_ADDR)pAddr_in & 0xFFFFF000);
	for (pstPoolDir = m_astPoolDir; pstPoolDir->uiSize > 0; ++pstPoolDir) {
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
		pstPoolDesc->pstNext = m_pstPoolDescFree;
		m_pstPoolDescFree = pstPoolDesc;
	}
	
	CPU_INT_EN();
	
	return;
}


LIB_PRIVATE  void  lib_pool_GetFreeDesc(void)
{
	_LIB_POOL_DESC* pstDesc = 0;
	CPU_INT32S  i = 0;
	
	if (0 != m_pstPoolDescFree) {
		return;
	}
	
	CPUExt_PageGetFree((CPU_ADDR *)(&m_pstPoolDescFree));
	if (0 == m_pstPoolDescFree) {
		CPUExt_CorePanic("[PANIC][lib_pool_GetFreeDesc]No Memory");
	}
	
	pstDesc = m_pstPoolDescFree;
	for (i = 1; i < (X86_MEM_PAGE_SIZE / sizeof(_LIB_POOL_DESC)); ++i) {
		pstDesc->pstNext = pstDesc + 1;
		pstDesc++;
	}
	
	return;
}


