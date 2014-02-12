
#ifndef __CPU_PAGE_H__
#define __CPU_PAGE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu_core.h>

/******************************************************************************
    Public Interface
******************************************************************************/

extern void cpu_page_Init(const CPU_INT32U  uiRamdiskSize_in);

extern void cpu_page_GetBufferSpace(
	CPU_ADDR * paddrPhyBufStart_out,
	CPU_ADDR * paddrPhyBufEnd_out
);

extern CPU_ERR  cpu_page_CopyPageTable(
	const CPU_ADDR    addrLinerarSrc_in,
	const CPU_ADDR    addrLinerarDst_in,
	const CPU_INT32U  uiSize_in
);
extern void cpu_page_ReleasePageTable(
	const CPU_ADDR    addrLinerar_in, 
	const CPU_INT32U  uiSize_in
);

extern CPU_ERR  cpu_page_PutPageToLinerarAddr(
	CPU_ADDR  addrPhysicalPage_in,
	CPU_ADDR  addrLinerar_in
);

extern void cpu_page_WriteProtectedVerify(
	CPU_ADDR  addrLinerar_in
);

extern void cpu_page_ISR_LackMemoryPage(
	CPU_DATA  uiErrorCode_in,
	CPU_DATA  uiLinerarAddr_in
);

extern void cpu_page_ISR_WriteProtectedPage(
	CPU_DATA  uiErrorCode_in,
	CPU_DATA  uiLinerarAddr_in
);

#endif /* __CPU_PAGE_H__ */

