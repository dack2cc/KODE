
#ifndef __LIB_POOL_H__
#define __LIB_POOL_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

void  lib_pool_Init(const CPU_ADDR addrStart_in, const CPU_ADDR addrEnd_in);
void* lib_pool_Malloc(CPU_SIZE_T  size_in);
void  lib_pool_Free(void* pAddr_in, CPU_SIZE_T  size_in);

typedef struct {
	CPU_ADDR   addr;
	CPU_SIZE_T size;
} LIB_POOL_FREE;

typedef struct {
	CPU_INT32U      uiLostSize;
	CPU_INT32U      uiLostCnt;
	CPU_INT32U      uiFreeCnt;
	CPU_INT32U      uiFreeMax;
	LIB_POOL_FREE * pstFree;
} LIB_POOL_CONTROL;

void  lib_pool_Setup(LIB_POOL_CONTROL * pstCtl_out, CPU_SIZE_T sizeCtl_in, CPU_ADDR addrFree_in, CPU_SIZE_T sizeFree_in);
void* lib_pool_Get(LIB_POOL_CONTROL * pstCtl_inout, CPU_SIZE_T size_in);
void  lib_pool_Put(LIB_POOL_CONTROL * pstCtl_inout, void* addr_in, CPU_SIZE_T size_in);

#endif /* __LIB_POOL_H__ */

