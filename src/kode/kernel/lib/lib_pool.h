
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

#endif /* __LIB_POOL_H__ */

