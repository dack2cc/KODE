
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_page.h>
#include <cpu_boot.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

CPU_PRIVATE  CPU_ADDR    m_addrMemoHigh = 0;


#define _MEMO_SIZE_MAX      (16*1024*1024)  /* 16MB */
#define _MEMO_LOW_ADDR      (0x100000)      /* 1MB  */
#define _MEMO_PAGING_SIZE   (_MEMO_SIZE_MAX - _MEMO_LOW_ADDR)
#define _MEMO_PAGING_NUM    (_MEMO_PAGING_SIZE >> 12)
#define _MEMO_PAGE_FREE     (0)
#define _MEMO_PAGE_USED     (100)

CPU_PRIVATE  CPU_INT08U  m_aiPageMap[_MEMO_PAGING_NUM];
#define _calc_page_index(addr)  (((addr) - _MEMO_LOW_ADDR) >> 12)


typedef struct {
	CPU_INT32U  uiMemSize;
	CPU_INT32U  uiBufSize;
} CPU_MEMO_BUFFER;

CPU_PRIVATE  CPU_MEMO_BUFFER  m_astMemoBuffer[] = {
	{12 * 1024 * 1024, 4 * _MEMO_LOW_ADDR},
	{ 6 * 1024 * 1024, 2 * _MEMO_LOW_ADDR},
	{ 0 * 1024 * 1024, 1 * _MEMO_LOW_ADDR}
};
#define CPU_MEMO_BUFFER_MAX  (sizeof(m_astMemoBuffer) / sizeof(CPU_MEMO_BUFFER))


#define _is_valid_page_table_item(item)    (0x01 == (0x01 & (item)))

#define _invalidate()                      __asm__("movl %%eax,%%cr3"::"a" (0))

#define _copy_page(from, to)               __asm__("cld ; rep ; movsl"::"S" (from),"D" (to),"c" (1024):)


/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE void      cpu_page_OutOfMemory(void);
CPU_PRIVATE void      cpu_page_UnlockWriteProtected(CPU_INT32U*  puiPgTbItm_inout);
CPU_PRIVATE void      cpu_page_GetEmptyPageForLinerarAddr(const CPU_ADDR addrLinerar_in);
CPU_PRIVATE CPU_ADDR  cpu_page_GetFreePageFast(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_page_Init(const CPU_INT32U  uiRamdiskSize_in, CPU_ADDR*  paddrBufMemEnd_out)
{
	CPU_INT32S  i = 0;
	CPU_INT32S  iValidPageCnt = 0;
	CPU_ADDR    addrBufMemEnd = 0;
	CPU_ADDR    addrMemoStart = 0;
	
	/* get the end of memory */
	m_addrMemoHigh = _MEMO_LOW_ADDR + (X86_MEM_EXT_SIZE_IN_KB * 1024);
	
	/* restrict the max memroy size */
	if (m_addrMemoHigh > _MEMO_SIZE_MAX) {
		m_addrMemoHigh = _MEMO_SIZE_MAX;
	}
	
	/* calculate the buffer memory end address */
	for (i = 0; i < CPU_MEMO_BUFFER_MAX; ++i) {
		if (m_addrMemoHigh > m_astMemoBuffer[i].uiMemSize) {
			addrBufMemEnd = m_astMemoBuffer[i].uiBufSize;
			break;
		}
	}
	if (paddrBufMemEnd_out) {
		(*paddrBufMemEnd_out) = addrBufMemEnd;
	}
	
	/* init all the pages to used */
	for (i = 0; i < _MEMO_PAGING_NUM; ++i) {
		m_aiPageMap[i] = _MEMO_PAGE_USED;
	}
	
	/* mark the free pages */
	addrMemoStart = addrBufMemEnd + uiRamdiskSize_in;
	i = _calc_page_index(addrMemoStart);
	iValidPageCnt = ((m_addrMemoHigh - addrMemoStart) >> 12);	
	while (iValidPageCnt > 0) {
		m_aiPageMap[i] = _MEMO_PAGE_FREE;
		++i;
		--iValidPageCnt;
	}
	
	return;
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

	for (uiPageIndex = 0; uiPageIndex < _MEMO_PAGING_NUM; ++uiPageIndex) {
		if (_MEMO_PAGE_FREE == m_aiPageMap[uiPageIndex]) {
			m_aiPageMap[uiPageIndex] = 1;
			addrPysicalPage = (uiPageIndex << 12) + _MEMO_LOW_ADDR;
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
	
	if (addrPhysical_in <  _MEMO_LOW_ADDR) {
		return;
	}
	if (addrPhysical_in >= m_addrMemoHigh) {
		CPUExt_CorePanic("[PANIC][CPUExt_PageRelease]trying to release a nonexistent page");
	}
	
	iPageIdx = _calc_page_index(addrPhysical_in);
	if (m_aiPageMap[iPageIdx] > 0) {
		--m_aiPageMap[iPageIdx];
	}
	else {
		m_aiPageMap[iPageIdx] = 0;
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
			
			if ((*puiPgTbItmSrc) > _MEMO_LOW_ADDR) {
				(*puiPgTbItmSrc) = ((*puiPgTbItmSrc) & (~2));      /* set the source page to read only */
				uiPageIdxSrc = _calc_page_index((*puiPgTbItmSrc)); /* add the reference count of the page. */
				m_aiPageMap[uiPageIdxSrc]++;
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
	if ((addrPhysicalPage_in <  _MEMO_LOW_ADDR)
	||  (addrPhysicalPage_in >= m_addrMemoHigh)) {
		CPUExt_CorePanic("[PANIC][cpu_page_PutPageToLinerarAddr]Page address out of range");
	}
	uiPageIndex = _calc_page_index(addrPhysicalPage_in);
	if (_MEMO_PAGE_FREE == m_aiPageMap[uiPageIndex]) {
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
	if ((addrPhyPageOld >= _MEMO_LOW_ADDR) 
	&&  (1 == m_aiPageMap[uiPageIdxOld])) {
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
	if (addrPhyPageOld >= _MEMO_LOW_ADDR) {
		m_aiPageMap[uiPageIdxOld]--;
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
	:"0" (0),"i" (_MEMO_LOW_ADDR),"c" (_MEMO_PAGING_NUM),
	"D" (m_aiPageMap+_MEMO_PAGING_NUM-1)
	:);
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


