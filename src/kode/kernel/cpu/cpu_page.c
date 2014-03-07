
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_page.h>
#include <cpu_boot.h>
#include <cpu_ext.h>

/*
  This var will be assigned by digest_symbols() in ld program.
*/
extern int end;

/******************************************************************************
    Private Define
******************************************************************************/

#define CPU_PAGE_MEM_SIZE_MAX    (16*1024*1024)  /* 16MB */
#define CPU_PAGE_LOW_ADDR_PHY    (0x100000)      /* 1MB  */
#define CPU_PAGE_VALID_MEM_SIZE  (CPU_PAGE_MEM_SIZE_MAX - CPU_PAGE_LOW_ADDR_PHY)
#define CPU_PAGE_VALID_NR        (CPU_PAGE_VALID_MEM_SIZE >> 12)
#define CPU_PAGE_STATUS_FREE     (0)
#define CPU_PAGE_STATUS_USED     (100)

CPU_PRIVATE  CPU_INT08U  cpu_page_aiStatus[CPU_PAGE_VALID_NR];

typedef struct _CPU_PAGE_BUFFER_INFO {
	CPU_INT32U  uiMemSize;
	CPU_INT32U  uiBufSize;
} CPU_PAGE_BUFFER_INFO;

CPU_PRIVATE  CPU_PAGE_BUFFER_INFO  cpu_page_astBufInf[] = {
	{12 * 1024 * 1024, 4 * CPU_PAGE_LOW_ADDR_PHY},
	{ 6 * 1024 * 1024, 2 * CPU_PAGE_LOW_ADDR_PHY},
	{ 0 * 1024 * 1024, 1 * CPU_PAGE_LOW_ADDR_PHY}
};
#define CPU_PAGE_BUFFER_INFO_MAX  (sizeof(cpu_page_astBufInf) / sizeof(CPU_PAGE_BUFFER_INFO))

typedef struct _CPU_PAGE_CONTROL {
	CPU_ADDR  adrPhyMemEnd;
	CPU_ADDR  adrPhyBufStart;
	CPU_ADDR  adrPhyBufEnd;
	CPU_ADDR  adrPhyRamdiskStart;
	CPU_ADDR  adrPhyRamdiskEnd;
} CPU_PAGE_CONTROL;

CPU_PRIVATE  CPU_PAGE_CONTROL  cpu_page_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

#define _calc_page_index(adrPhy)           (((adrPhy) - CPU_PAGE_LOW_ADDR_PHY) >> 12)
#define _is_valid_page_table_item(item)    (0x01 == (0x01 & (item)))
#define _invalidate()                      __asm__("movl %%eax,%%cr3"::"a" (0))
#define _copy_page(from, to)               __asm__("cld ; rep ; movsl"::"S" (from),"D" (to),"c" (1024))

CPU_PRIVATE void      cpu_page_OutOfMemory(void);
CPU_PRIVATE void      cpu_page_UnlockWriteProtected(CPU_INT32U*  puiPgTbItm_inout);
CPU_PRIVATE void      cpu_page_GetEmptyPageForLinerarAddr(const CPU_ADDR addrLinerar_in);
CPU_PRIVATE CPU_ADDR  cpu_page_GetFreePageFast(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_page_Init(const CPU_INT32U  uiRamdiskSize_in)
{
	CPU_INT32S  i = 0;
	CPU_INT32S  iValidPageCnt  = 0;
	CPU_ADDR    adrPhyMemStart = 0;
	
	//drv_disp_Printf("[Memory][%d MB]\r\n", X86_MEM_EXT_SIZE_IN_KB/1024);
	
	/* get the end of memory */
	cpu_page_stCtl.adrPhyMemEnd  = CPU_PAGE_LOW_ADDR_PHY + (X86_MEM_EXT_SIZE_IN_KB * 1024);
	cpu_page_stCtl.adrPhyMemEnd &= 0xfffff000;
	
	/* restrict the max memroy size */
	if (cpu_page_stCtl.adrPhyMemEnd > CPU_PAGE_MEM_SIZE_MAX) {
		cpu_page_stCtl.adrPhyMemEnd = CPU_PAGE_MEM_SIZE_MAX;
	}
	
	/* buffer space is after the kernel image */
	cpu_page_stCtl.adrPhyBufStart = (CPU_ADDR)(&end);
	
	/* calculate the buffer memory end address */
	for (i = 0; i < CPU_PAGE_BUFFER_INFO_MAX; ++i) {
		if (cpu_page_stCtl.adrPhyMemEnd > cpu_page_astBufInf[i].uiMemSize) {
			cpu_page_stCtl.adrPhyBufEnd = cpu_page_astBufInf[i].uiBufSize;
			break;
		}
	}
	
	/* calculate the ramdisk memory address */
	cpu_page_stCtl.adrPhyRamdiskStart = cpu_page_stCtl.adrPhyBufEnd;
	cpu_page_stCtl.adrPhyRamdiskEnd   = cpu_page_stCtl.adrPhyRamdiskStart + uiRamdiskSize_in;
	
	/* init all the pages to used */
	for (i = 0; i < CPU_PAGE_VALID_NR; ++i) {
		cpu_page_aiStatus[i] = CPU_PAGE_STATUS_USED;
	}
	
	/* mark the free pages */
	adrPhyMemStart = cpu_page_stCtl.adrPhyBufEnd + uiRamdiskSize_in;
	i = _calc_page_index(adrPhyMemStart);
	iValidPageCnt = ((cpu_page_stCtl.adrPhyMemEnd - adrPhyMemStart) >> 12);	
	while (iValidPageCnt > 0) {
		cpu_page_aiStatus[i] = CPU_PAGE_STATUS_FREE;
		++i;
		--iValidPageCnt;
	}
	
	return;
}

void  CPUExt_PageGetBufferSpace(
	CPU_ADDR*  paddrPhysicalStart_out, 
	CPU_ADDR*  paddrPhysicalEnd_out
)
{
	if (0 != paddrPhysicalStart_out) {
		(*paddrPhysicalStart_out) = cpu_page_stCtl.adrPhyBufStart;
	}
	
	if (0 != paddrPhysicalEnd_out) {
		(*paddrPhysicalEnd_out) = cpu_page_stCtl.adrPhyBufEnd;
	}
}

void  CPUExt_PageGetRamdiskSpace(
	CPU_ADDR*  paddrPhysicalStart_out, 
	CPU_ADDR*  paddrPhysicalEnd_out
)
{
	if (0 != paddrPhysicalStart_out) {
		(*paddrPhysicalStart_out) = cpu_page_stCtl.adrPhyRamdiskStart;
	}
	if (0 != paddrPhysicalEnd_out) {
		(*paddrPhysicalEnd_out) = cpu_page_stCtl.adrPhyRamdiskEnd;
	}
}

void CPUExt_PageGetFree(CPU_ADDR*  paddrPhysical_out)
{
	if (0 == paddrPhysical_out) {
		return;
	}

#if 0
	
	{
	CPU_INT32U  uiPageIndex = 0;
	CPU_ADDR    addrPysicalPage = 0;

	for (uiPageIndex = 0; uiPageIndex < CPU_PAGE_VALID_NR; ++uiPageIndex) {
		if (CPU_PAGE_STATUS_FREE == cpu_page_aiStatus[uiPageIndex]) {
			cpu_page_aiStatus[uiPageIndex] = 1;
			addrPysicalPage = (uiPageIndex << 12) + CPU_PAGE_LOW_ADDR_PHY;
			break;
		}
	}
	
	(*paddrPhysical_out) = addrPysicalPage;
	}

#else

	(*paddrPhysical_out) = cpu_page_GetFreePageFast();

#endif 
	
	return;
}

void CPUExt_PageRelease(const CPU_ADDR  addrPhysical_in)
{
	CPU_INT32U  iPageIdx = 0;
	
	if (addrPhysical_in <  CPU_PAGE_LOW_ADDR_PHY) {
		return;
	}
	if (addrPhysical_in >= cpu_page_stCtl.adrPhyMemEnd) {
		CPUExt_CorePanic("[PANIC][CPUExt_PageRelease]trying to release a nonexistent page");
	}
	
	iPageIdx = _calc_page_index(addrPhysical_in);
	if (cpu_page_aiStatus[iPageIdx] > 0) {
		--cpu_page_aiStatus[iPageIdx];
	}
	else {
		cpu_page_aiStatus[iPageIdx] = 0;
		CPUExt_CorePanic("[PANIC][CPUExt_PageRelease]trying to release a free page");
	}
	
	return;
}


CPU_ERR  cpu_page_CopyPageTable(
	const CPU_ADDR    addrLinerarSrc_in,
	const CPU_ADDR    addrLinerarDst_in,
	const CPU_INT32U  uiSize_in
)
{
	CPU_INT32U  uiPgTbDrIndex = 0;
	CPU_INT32U* puiPgTbDrItmSrc = 0;
	CPU_INT32U* puiPgTbDrItmDst = 0;
	CPU_INT32U  uiPgTbDrCpyCnt = 0;
	CPU_INT32U  i = 0;
	
	/* the linerar address into a page table 
	   have to be divisible by 4MB */
	if ((0x3FFFFF & addrLinerarSrc_in) 
	||  (0x3FFFFF & addrLinerarDst_in)) {
		CPUExt_CorePanic("[PANIC][cpu_page_CopyPageTable]copy page table called with wrong alignment");
	}
	
	uiPgTbDrIndex   = (addrLinerarSrc_in >> 22);
	puiPgTbDrItmSrc = &(X86_MEM_PAGE_TABLE_DIR[uiPgTbDrIndex]);
	uiPgTbDrIndex   = (addrLinerarDst_in >> 22);
	puiPgTbDrItmDst = &(X86_MEM_PAGE_TABLE_DIR[uiPgTbDrIndex]);
	uiPgTbDrCpyCnt  = (uiSize_in + 0x3FFFFF) >> 22;
	
	/* copy the item in page table dir */
	for (i = 0; i < uiPgTbDrCpyCnt; ++i, ++puiPgTbDrItmDst, ++puiPgTbDrItmSrc) {
		CPU_ADDR    addrPgTbPhysical = 0;
		CPU_INT32U* puiPgTbItmSrc = 0;
		CPU_INT32U* puiPgTbItmDst = 0;
		CPU_INT32U  uiPgTbCpyCnt = 0;
		CPU_INT32U  j = 0;

		/* the destination item in page table dir is already exist */
		if (_is_valid_page_table_item(*puiPgTbDrItmDst)) {
			CPUExt_CorePanic("[PANIC][cpu_page_CopyPageTable]copy of page already exist");
		}
		
		/* no need to copy a unexist source item */
		if (!_is_valid_page_table_item(*puiPgTbDrItmSrc)) {
			continue;
		}
		
		/* get a memory page for a new page table */
		CPUExt_PageGetFree(&addrPgTbPhysical);
		if (0 == addrPgTbPhysical) {
			return (CPU_ERR_NO_MEMORY); /* out of memory */
		}
		(*puiPgTbDrItmDst) = (addrPgTbPhysical | 7);
		puiPgTbItmDst = (CPU_INT32U *)addrPgTbPhysical;
		
		/* get the physical address of the source page table */
		addrPgTbPhysical = (*puiPgTbDrItmSrc) & 0xFFFFF000;
		puiPgTbItmSrc = (CPU_INT32U *)addrPgTbPhysical;
		
		/* copy the item in the page table */
		uiPgTbCpyCnt = ((0 == addrLinerarSrc_in) ? 0xA0 : 1024);
		for (j = 0; j < uiPgTbCpyCnt; ++j, ++puiPgTbItmSrc, ++puiPgTbItmDst) {
			CPU_INT32U  uiPageIdxSrc = 0;
			
			/* no need to copy a unexist source item */
			if (!_is_valid_page_table_item(*puiPgTbItmSrc)) {
				continue;
			}
			
			/* set the R/W bit to Zero for destination page table item
			   U/S 1, R/W 1 : the code in user space can write the page.
			   U/S 1, R/W 0 : the code in user space can read only.
			*/
			(*puiPgTbItmDst) = ((*puiPgTbItmSrc) & (~2));
			
			if ((*puiPgTbItmSrc) > CPU_PAGE_LOW_ADDR_PHY) {
				(*puiPgTbItmSrc) = ((*puiPgTbItmSrc) & (~2));      /* set the source page to read only */
				uiPageIdxSrc = _calc_page_index((*puiPgTbItmSrc)); /* add the reference count of the page. */
				cpu_page_aiStatus[uiPageIdxSrc]++;
			}
		}
	}
	
	/* update the paging cache */
	_invalidate();
	
	return (CPU_ERR_NONE);
}

void cpu_page_ReleasePageTable(
	const CPU_ADDR    addrLinerar_in, 
	const CPU_INT32U  uiSize_in
)
{
	CPU_INT32U  uiPgTbDrIdx = 0;
	CPU_INT32U* puiPgTbDrItm = 0;
	CPU_INT32U  uiPgTbDrCnt = 0;
	CPU_INT32U  i = 0;
	
	if (addrLinerar_in & 0x3FFFFF) {
		CPUExt_CorePanic("[PANIC][cpu_page_ReleasePageTable]release page tables called with wrong alignment");
	}
	if (0 == addrLinerar_in) {
		CPUExt_CorePanic("[PANIC][cpu_page_ReleasePageTable]Trying to release up swapper memory space");
	}
	
	uiPgTbDrIdx = (addrLinerar_in >> 22);
	puiPgTbDrItm = &(X86_MEM_PAGE_TABLE_DIR[uiPgTbDrIdx]);
	uiPgTbDrCnt = ((uiSize_in + 0x3FFFFF) >> 22);
	
	for (i = 0; i < uiPgTbDrCnt; ++i, ++puiPgTbDrItm) {
		CPU_INT32U*  puiPgTbItm = 0;
		CPU_INT32U   j = 0;
		
		/* ignore the unexist item in page table dir */
		if (!_is_valid_page_table_item(*puiPgTbDrItm)) {
			continue;
		}
		
		/* release the memory page in the page table */
		puiPgTbItm = (CPU_INT32U *)(0xFFFFF000 & (*puiPgTbDrItm));
		for (j = 0; j < 1024; ++j, ++puiPgTbItm) {
			/* release the valid memory page */
			if (_is_valid_page_table_item(*puiPgTbItm)) {
				CPUExt_PageRelease(0xFFFFF000 & (*puiPgTbItm));
			}
			(*puiPgTbItm) = 0;
		}
		
		/* release the memory page for the page table */
		CPUExt_PageRelease(0xFFFFF000 & (*puiPgTbDrItm));
		(*puiPgTbDrItm) = 0;
	}
	
	_invalidate();
	
	return;
}

CPU_ERR  cpu_page_PutPageToLinerarAddr(
	CPU_ADDR  addrPhysicalPage_in,
	CPU_ADDR  addrLinerar_in
)
{
	CPU_INT32U  uiPageIndex = 0;
	CPU_INT32U  uiPgTbDrIdx = 0;
	CPU_INT32U* puiPgTbDrItm = 0;
	CPU_INT32U  uiPgTbIdx = 0;
	CPU_INT32U* puiPgTbItm = 0;
	
	if (0 != (addrPhysicalPage_in & 0xFFF)) {
		CPUExt_CorePanic("[PANIC][cpu_page_PutPageToLinerarAddr]Exception page address");
	}
	if ((addrPhysicalPage_in <  CPU_PAGE_LOW_ADDR_PHY)
	||  (addrPhysicalPage_in >= cpu_page_stCtl.adrPhyMemEnd)) {
		CPUExt_CorePanic("[PANIC][cpu_page_PutPageToLinerarAddr]Page address out of range");
	}
	uiPageIndex = _calc_page_index(addrPhysicalPage_in);
	if (CPU_PAGE_STATUS_FREE == cpu_page_aiStatus[uiPageIndex]) {
		CPUExt_CorePanic("[PANIC][cpu_page_PutPageToLinerarAddr]Try to put a free page");
	}
	
	uiPgTbDrIdx = (addrLinerar_in >> 22);
	puiPgTbDrItm = &(X86_MEM_PAGE_TABLE_DIR[uiPgTbDrIdx]);
	
	if (!_is_valid_page_table_item(*puiPgTbDrItm)) {
		CPUExt_PageGetFree((CPU_ADDR *)(puiPgTbDrItm));
		if (0 == (*puiPgTbDrItm)) {
			return (CPU_ERR_NO_MEMORY);
		}
		(*puiPgTbDrItm) = (*puiPgTbDrItm) | 7;
	}
	puiPgTbItm = (CPU_INT32U *)(0xFFFFF000 & (*puiPgTbDrItm));
	
	uiPgTbIdx = ((addrLinerar_in >> 12) & 0x3FF);
	puiPgTbItm[uiPgTbIdx] = addrPhysicalPage_in | 7;
	
	return (CPU_ERR_NONE);
}

void cpu_page_ISR_WriteProtectedPage(
	CPU_DATA  uiErrorCode_in,
	CPU_DATA  uiLinerarAddr_in
)
{
	CPU_INT32U  uiPgTbDrIdx = 0;
	CPU_INT32U* puiPgTbDrItm = 0;
	CPU_INT32U* puiPgTbItm = 0;
	
	uiPgTbDrIdx  = (uiLinerarAddr_in >> 22);
	puiPgTbDrItm = &(X86_MEM_PAGE_TABLE_DIR[uiPgTbDrIdx]);
	
	puiPgTbItm  = (CPU_INT32U *)(0xFFFFF000 & (*puiPgTbDrItm));
	puiPgTbItm += ((uiLinerarAddr_in >> 12) & 0x3FF);
	
	cpu_page_UnlockWriteProtected(puiPgTbItm);
	
	return;
}

void cpu_page_WriteProtectedVerify(
	CPU_ADDR  addrLinerar_in
)
{
	CPU_INT32U  uiPgTbDrIdx = 0;
	CPU_INT32U* puiPgTbDrItm = 0;
	CPU_INT32U* puiPgTbItm = 0;
	
	uiPgTbDrIdx  = (addrLinerar_in >> 22);
	puiPgTbDrItm = &(X86_MEM_PAGE_TABLE_DIR[uiPgTbDrIdx]);
	
	/* ignore the invalid page item */
	if (!_is_valid_page_table_item(*puiPgTbDrItm)) {
		return;
	}
	
	puiPgTbItm  = (CPU_INT32U *)(0xFFFFF000 & (*puiPgTbDrItm));
	puiPgTbItm += ((addrLinerar_in >> 12) & 0x3FF);
	
	/* make a new copy for the valid and write protected memroy page  */
	if (1 == (0x03 & (*puiPgTbItm))) {
		cpu_page_UnlockWriteProtected(puiPgTbItm);
	}

	return;
}

CPU_PRIVATE void cpu_page_OutOfMemory(void)
{
	CPUExt_CorePanic("[PANIC][cpu_page_OutOfMemory]out of memory");
	
	return;
}

CPU_PRIVATE void cpu_page_UnlockWriteProtected(CPU_INT32U*  puiPgTbItm_inout)
{
	CPU_ADDR  addrPhyPageOld = 0;
	CPU_ADDR  addrPhyPageNew = 0;
	CPU_INT32U  uiPageIdxOld = 0;
	
	if (0 == puiPgTbItm_inout) {
		CPUExt_CorePanic("[PANIC][cpu_page_UnlockWriteProtected]Excpetion");
	}
	
	/* get the address of the memory page */
	addrPhyPageOld = (0xFFFFF000 & (*puiPgTbItm_inout));
	uiPageIdxOld = _calc_page_index(addrPhyPageOld);
	
	/* the memory page is not shared */
	if ((addrPhyPageOld >= CPU_PAGE_LOW_ADDR_PHY) 
	&&  (1 == cpu_page_aiStatus[uiPageIdxOld])) {
		(*puiPgTbItm_inout) |= 2;
		_invalidate();
		return;
	}
	
	/* get a memory page for copy */
	CPUExt_PageGetFree(&addrPhyPageNew);
	if (0 == addrPhyPageNew) {
		cpu_page_OutOfMemory();
	}
	
	/* reference count */
	if (addrPhyPageOld >= CPU_PAGE_LOW_ADDR_PHY) {
		cpu_page_aiStatus[uiPageIdxOld]--;
	}
	
	/* point the page table item to the new memory page */
	(*puiPgTbItm_inout) = addrPhyPageNew | 7;
	_invalidate();
	_copy_page(addrPhyPageOld, addrPhyPageNew);
	
	return;
}

CPU_PRIVATE void cpu_page_GetEmptyPageForLinerarAddr(const CPU_ADDR addrLinerar_in)
{
	CPU_ADDR  addrPhysicalPage = 0;
	CPU_ERR   ret = CPU_ERR_NONE;
	
	CPUExt_PageGetFree(&addrPhysicalPage);
	if (addrPhysicalPage > 0) {
		ret = cpu_page_PutPageToLinerarAddr(addrPhysicalPage, addrLinerar_in);
		if (CPU_ERR_NONE == ret) {
			return;
		}
		
		CPUExt_PageRelease(addrPhysicalPage);
	}
	cpu_page_OutOfMemory();
	
	return;
}

CPU_PRIVATE CPU_ADDR  cpu_page_GetFreePageFast(void)
{
register unsigned long __res asm("ax");

__asm__("std ; repne ; scasb\n\t"
	"jne 1f\n\t"
	"movb $1,1(%%edi)\n\t"
	"sall $12,%%ecx\n\t"
	"addl %2,%%ecx\n\t"
	"movl %%ecx,%%edx\n\t"
	"movl $1024,%%ecx\n\t"
	"leal 4092(%%edx),%%edi\n\t"
	"rep ; stosl\n\t"
	"movl %%edx,%%eax\n"
	"1:"
	:"=a" (__res)
	:"0" (0),"i" (CPU_PAGE_LOW_ADDR_PHY),"c" (CPU_PAGE_VALID_NR),
	"D" (cpu_page_aiStatus+CPU_PAGE_VALID_NR-1)
	);
return __res;
}

extern void cpu_page_ISR_LackMemoryPage(
	CPU_DATA  uiErrorCode_in,
	CPU_DATA  uiLinerarAddr_in
)
{
	cpu_page_GetEmptyPageForLinerarAddr(uiLinerarAddr_in);
	
	return;
}


