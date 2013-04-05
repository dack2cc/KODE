
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_task.h>
#include <cpu_boot.h>
#include <cpu_page.h>
#include <cpu_asm.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

/* ====================================
    Operation of TSS and LDT
   ==================================== */

#define _calc_tss(n)                     ((((CPU_INT32U) n)<<4)+(X86_GDT_TSS_0<<3))
#define _calc_ldt(n)                     ((((CPU_INT32U) n)<<4)+(X86_GDT_LDT_0<<3))

#define _set_tss_desc(n, addr)           __set_tssldt_desc(((CPU_CHAR *) (n)),addr,"0x89")
#define _set_ldt_desc(n, addr)           __set_tssldt_desc(((CPU_CHAR *) (n)),addr,"0x82")

#define _get_ldt_base(ldt)               __get_base( ((CPU_INT08U *)&(ldt)) )
#define _set_ldt_base(ldt, base)         __set_base( ((CPU_INT08U *)&(ldt)) , base )

#define _get_ldt_limit_code()            __get_limit(0x0f)
#define _get_ldt_limit_data()            __get_limit(0x17)
#define _set_ldt_limit(ldt, limit)       __set_limit( ((CPU_INT08U *)&(ldt)) , (limit-1)>>1 )



#define __set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)


#define __get_base(addr) ({\
unsigned long __base; \
__asm__("movb %3,%%dh\n\t" \
	"movb %2,%%dl\n\t" \
	"shll $16,%%edx\n\t" \
	"movw %1,%%dx" \
	:"=d" (__base) \
	:"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7))); \
__base;})


void __set_base(CPU_INT08U* addr, CPU_ADDR base) {
//#define __set_base(addr,base)
 __asm__ (\
 	"movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %%dl,%1\n\t" \
	"movb %%dh,%2" \
	: \
	:"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7)), \
	 "d" (base) \
	);
}


#define __get_limit(segment) ({ \
unsigned long __limit; \
__asm__("lsll %1,%0\n\tincl %0":"=r" (__limit):"r" (segment)); \
__limit;})


void __set_limit(CPU_INT08U* addr, CPU_INT32U limit) {
//#define __set_limit(addr,limit)
__asm__("movw %%dx,%0\n\t" \
	"rorl $16,%%edx\n\t" \
	"movb %1,%%dh\n\t" \
	"andb $0xf0,%%dh\n\t" \
	"orb %%dh,%%dl\n\t" \
	"movb %%dl,%1" \
	: \
	:"m" (*(addr)), \
	 "m" (*((addr)+6)), \
	 "d" (limit) \
	);
}

/* ====================================
    Definition of Task Control
   ====================================  */

typedef struct {
	CPU_DATA	cwd;
	CPU_DATA	swd;
	CPU_DATA	twd;
	CPU_DATA	fip;
	CPU_DATA	fcs;
	CPU_DATA	foo;
	CPU_DATA	fos;
	CPU_DATA	st_space[20];	/* 8*10 bytes for each FP-reg = 80 bytes */
} CPU_X86_I387;

typedef struct {
	CPU_DATA	back_link;	/* 16 high bits zero */
	CPU_DATA	esp0;
	CPU_DATA	ss0;		/* 16 high bits zero */
	CPU_DATA	esp1;
	CPU_DATA	ss1;		/* 16 high bits zero */
	CPU_DATA	esp2;
	CPU_DATA	ss2;		/* 16 high bits zero */
	CPU_DATA	cr3;
	CPU_DATA	eip;
	CPU_DATA	eflags;
	CPU_DATA	eax,ecx,edx,ebx;
	CPU_DATA	esp;
	CPU_DATA	ebp;
	CPU_DATA	esi;
	CPU_DATA	edi;
	CPU_DATA	es;		/* 16 high bits zero */
	CPU_DATA	cs;		/* 16 high bits zero */
	CPU_DATA	ss;		/* 16 high bits zero */
	CPU_DATA	ds;		/* 16 high bits zero */
	CPU_DATA	fs;		/* 16 high bits zero */
	CPU_DATA	gs;		/* 16 high bits zero */
	CPU_DATA	ldt;		/* 16 high bits zero */
	CPU_DATA	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
} CPU_X86_TSS;

typedef struct {
	X86_DESC      ldt[X86_LDT_MAX];
	CPU_X86_TSS   tss;
	CPU_X86_I387  i387;
	
	CPU_INT32U    index;
} CPU_X86_TASK;

typedef union {
	CPU_X86_TASK  task;
	CPU_INT08S    stack[X86_MEM_PAGE_SIZE];
} CPU_X86_TASK_STACK;

CPU_PRIVATE  CPU_X86_TASK_STACK  m_stTask0KernelStack = { 
	/* task */
	{
	    /* ldt */
	    { 
		    {0,0},
		    {0x9f,0xc0fa00},
		    {0x9f,0xc0f200},
	    },
		
	    /* tss */
	    {
		    0, /* back_link */ 
	    	X86_MEM_PAGE_SIZE+(CPU_INT32U)(&m_stTask0KernelStack),  /* esp0 */
	    	0x10, /* ss0 */
	    	0, 0, 0, 0, /* esp1, ss1, eps2, ss2 */
		    (CPU_INT32U)(&X86_MEM_PAGE_TABLE_DIR), /* cr3 */
	        0,0, /* eip, eflags */
	    	0,0,0,0, /* eax, ecx, edx, ebx */
	    	0,0,0,0, /* esp, ebp, esi, edi */
	        0x17,0x17,0x17,0x17,0x17,0x17, /* es, cs, ss, ds, fs, gs */
	    	_calc_ldt(0), /* ldt */
	    	0x80000000, /* trace_map */
	    },
		
	    /* i387 */
	    {},
		
	    /* index */
	    0,
	}
};


#define  CPU_TASK_MAX    (64)
CPU_PRIVATE  CPU_X86_TASK*  m_apstTask[CPU_TASK_MAX] = {&(m_stTask0KernelStack.task), };
CPU_PRIVATE  CPU_X86_TASK*  m_pstCurrentTask = &(m_stTask0KernelStack.task);

#define  CPU_TASK_LINERAR_ADDR_RANGE    (64*1024*1024)  /* 64MB */
                                                        /* 64MB x 64 = 4GB */

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE  void     cpu_task_SwitchToTask0(void);
CPU_PRIVATE  CPU_ERR  cpu_task_GetTaskSpace(CPU_INT32U* puiIndex_out, CPU_ADDR addrPhyMemPage_in);
CPU_PRIVATE  CPU_ERR  cpu_task_MakeTSS(CPU_ADDR  addrKernelStack_in, CPU_FNCT_PTR  pfnRoutine_in, CPU_DATA  eflags_in, CPU_DATA isKernel_in, CPU_X86_TASK* pstTask_out);
CPU_PRIVATE  CPU_ERR  cpu_task_CloneCurrentLDT(const CPU_ADDR addrLnrStart_in, CPU_X86_TASK* pstTask_out);
CPU_PRIVATE  CPU_ERR  cpu_task_BuildStack(const CPU_ADDR addrLnrStart_in, const CPU_DATA isKernel_in, void * pParam_in, CPU_FNCT_PTR pfnReturnPoint_in, CPU_X86_TASK* pstTask_inout);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_task_Init(void)
{
	CPU_INT32U  i = 0;
	X86_DESC*   pstDesc = 0;
	
	/* zero the gdt and pointer array of task control */
	pstDesc = X86_GDT + X86_GDT_LDT_0 + 1;
	for (i = 1; i < CPU_TASK_MAX; ++i) {
		m_apstTask[i] = 0;
		/* TSS */
		pstDesc->hi = 0;
		pstDesc->lo = 0;
		pstDesc++;
		/* LDT */
		pstDesc->hi = 0;
		pstDesc->lo = 0;
		pstDesc++;
	}

	cpu_task_SwitchToTask0();
	
	return;
}


CPU_ERR  CPUExt_TaskCreate(const CPU_DATA* pArgList_in, CPU_INT32U* puiTaskID_out)
{
	CPU_INT32U     uiTaskIndex = 0;
	CPU_ADDR       addrLnrStart = 0;
	CPU_X86_TASK*  pstTask = 0;
	CPU_ERR        ret = CPU_ERR_NONE;
	
	if ((0 == pArgList_in) 
	||  (0 == puiTaskID_out)) {
		return (CPU_ERR_BAD_PARAM);
	}
	
	ret = cpu_task_GetTaskSpace(&uiTaskIndex, pArgList_in[CPU_TASK_ARG_EXT_DATA]);

	addrLnrStart = uiTaskIndex * CPU_TASK_LINERAR_ADDR_RANGE;
	pstTask = (CPU_X86_TASK *)pArgList_in[CPU_TASK_ARG_EXT_DATA];
	pstTask->index = uiTaskIndex;
	
	if (CPU_ERR_NONE == ret) {
		ret = cpu_task_MakeTSS(
			pArgList_in[CPU_TASK_ARG_KERNEL_STACK], 
			(CPU_FNCT_PTR)pArgList_in[CPU_TASK_ARG_ROUTINE], 
			pArgList_in[CPU_TASK_ARG_EFLAG], 
			pArgList_in[CPU_TASK_ARG_KERNEL_EN],
			pstTask
		);
	}
	
	if (CPU_ERR_NONE == ret) {
	    ret = cpu_task_CloneCurrentLDT(addrLnrStart, pstTask);		
	}
	
	if (CPU_ERR_NONE == ret) {
	    //pstTask->tss.esp = X86_MEM_PAGE_SIZE;
		
		/* get a new memory page for user stack */
		ret = cpu_task_BuildStack(
			addrLnrStart,
			pArgList_in[CPU_TASK_ARG_KERNEL_EN],
			(void *)pArgList_in[CPU_TASK_ARG_PARAMETER],
			(CPU_FNCT_PTR)pArgList_in[CPU_TASK_ARG_RET_POINT],
			pstTask
		);
	}
	
	/* release the resource while failed */
	if (CPU_ERR_NONE != ret) {
		cpu_page_ReleasePageTable(addrLnrStart, CPU_TASK_LINERAR_ADDR_RANGE);
		m_apstTask[uiTaskIndex] = 0;
		(*puiTaskID_out) = 0;
		return (CPU_ERR_FATAL);
	}
	
	/* register the tss and ldt of the task into the gdt. */
	_set_tss_desc(X86_GDT + X86_GDT_TSS_0 + (uiTaskIndex << 1), &(pstTask->tss));
	_set_ldt_desc(X86_GDT + X86_GDT_LDT_0 + (uiTaskIndex << 1), &(pstTask->ldt));
	
	/* return the task id */
	(*puiTaskID_out) = uiTaskIndex;
	
	return (CPU_ERR_NONE);
}


void  CPUExt_TaskDelete(const CPU_INT32U  uiTaskID_in)
{
	CPU_X86_TASK*  pstTask = 0;
	
	if (uiTaskID_in >= CPU_TASK_MAX) {
		CPUExt_CorePanic("[PANIC][cpu_task_KFDelete]Task ID is out of range");
	}
	
	if (0 != m_apstTask[uiTaskID_in]) {
		pstTask = m_apstTask[uiTaskID_in];
		cpu_page_ReleasePageTable(_get_ldt_base(pstTask->ldt[X86_LDT_CODE]), CPU_TASK_LINERAR_ADDR_RANGE);
		cpu_page_ReleasePageTable(_get_ldt_base(pstTask->ldt[X86_LDT_DATA]), CPU_TASK_LINERAR_ADDR_RANGE);
		m_apstTask[uiTaskID_in] = 0;
	}
	
	return;
}


void CPUExt_TaskSwitch(const CPU_INT32U  uiTaskID_in)
{
	CPU_SR_ALLOC();
	
	CPU_CRITICAL_ENTER();
	
	{
	struct {CPU_INT32S a,b;} __tmp;
	__asm__( 
		"cmpl %%ecx,  _m_pstCurrentTask \n\t"
		"je 1f \n\t"
		"xchgl %%ecx, _m_pstCurrentTask \n\t"
		"movw %%dx, %1 \n\t"
		"ljmp *%0 \n\t"
		"1:"
		: /* no output */
		:"m" (*&__tmp.a),
		 "m" (*&__tmp.b), 
		 "d" (_calc_tss(uiTaskID_in)),
		 "c" ((CPU_DATA)(m_apstTask[uiTaskID_in]))
	);
	}
	
	CPU_CRITICAL_EXIT();
	
	return;
}


CPU_PRIVATE  void cpu_task_SwitchToTask0(void)
{	
	/* register the tss and ldt of the task 0 into the gdt. */
	_set_tss_desc(X86_GDT + X86_GDT_TSS_0, &(m_stTask0KernelStack.task.tss));
	_set_ldt_desc(X86_GDT + X86_GDT_LDT_0, &(m_stTask0KernelStack.task.ldt));

	/* Clear NT, so that we won't have troubles with that later on */
	__asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
	
	/* load the tss and ldt of task 0 */
	__asm__("ltr %%ax"::"a" ((CPU_INT32U)(X86_GDT_TSS_0 << 3)));
	__asm__("lldt %%ax"::"a" ((CPU_INT32U)(X86_GDT_LDT_0 << 3)));
	
	/* set current task to task 0 */
	m_pstCurrentTask = m_apstTask[0];
}


void CPUExt_TaskSwitchToRing3(void)
{
	/* go to the ring 3 */
	__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x17\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax");
}


CPU_PRIVATE  CPU_ERR  cpu_task_GetTaskSpace(CPU_INT32U* puiIndex_out, CPU_ADDR addrPhyMemPage_in)
{
	CPU_INT32U  uiIndex = 0;
	
	if ((0 == puiIndex_out) 
	||  (0 == addrPhyMemPage_in)) {
		CPUExt_CorePanic("[PANIC][cpu_task_GetTaskIndex]Bad Parameter");
	}
	
	/* find free index for the task */
	for (uiIndex = 0; uiIndex < CPU_TASK_MAX; ++uiIndex) {
		if (0 == m_apstTask[uiIndex]) {
			break;
		}
	}
	if (uiIndex >= CPU_TASK_MAX) {
		return (CPU_ERR_FATAL);
	}
	
	m_apstTask[uiIndex] = (CPU_X86_TASK *)(addrPhyMemPage_in);
	
	/* return the index for the task */
	(*puiIndex_out) = uiIndex;
	
	return (CPU_ERR_NONE);
}

CPU_PRIVATE  CPU_ERR  cpu_task_MakeTSS(
    CPU_ADDR       addrKernelStack_in, 
	CPU_FNCT_PTR   pfnRoutine_in, 
	CPU_DATA       eflags_in, 
	CPU_DATA       isKernel_in, 
	CPU_X86_TASK*  pstTask_out
)
{
	if ((0 == pfnRoutine_in) 
	||  (0 == pstTask_out) 
	||  (0 == addrKernelStack_in)) {
		CPUExt_CorePanic("[PANIC][cpu_task_MakeTSS]Bad Parameter");
	}
	
	pstTask_out->tss.back_link = 0;
	pstTask_out->tss.esp0 = addrKernelStack_in;
	pstTask_out->tss.ss0  = 0x10;
	pstTask_out->tss.esp1 = 0;
	pstTask_out->tss.ss1  = 0;
	pstTask_out->tss.esp2 = 0;
	pstTask_out->tss.ss2  = 0;
	pstTask_out->tss.cr3 = (CPU_INT32U)(&X86_MEM_PAGE_TABLE_DIR);
	pstTask_out->tss.eip = (CPU_INT32S)pfnRoutine_in;
	pstTask_out->tss.eflags = eflags_in;
	pstTask_out->tss.eax = 0;
	pstTask_out->tss.ecx = 0;
	pstTask_out->tss.edx = 0;
	pstTask_out->tss.ebx = 0;
	pstTask_out->tss.esp = 0;
	pstTask_out->tss.ebp = 0;
	pstTask_out->tss.esi = 0;
	pstTask_out->tss.edi = 0;
	pstTask_out->tss.ldt = _calc_ldt(pstTask_out->index);
	pstTask_out->tss.trace_bitmap = 0x80000000;

	if (DEF_ENABLED == isKernel_in) {
	    pstTask_out->tss.es = 0x10;
	    pstTask_out->tss.cs = 0x08;
	    pstTask_out->tss.ss = 0x10;
	    pstTask_out->tss.ds = 0x10;
	    pstTask_out->tss.fs = 0x10;
	    pstTask_out->tss.gs = 0x10;
	} 
	else {
	    pstTask_out->tss.es = 0x17;
	    pstTask_out->tss.cs = 0x0f;
	    pstTask_out->tss.ss = 0x17;
	    pstTask_out->tss.ds = 0x17;
	    pstTask_out->tss.fs = 0x17;
	    pstTask_out->tss.gs = 0x17;
	}
	
	return (CPU_ERR_NONE);
}

CPU_PRIVATE  CPU_ERR  cpu_task_CloneCurrentLDT(
	const CPU_ADDR  addrLnrStart_in, 
	CPU_X86_TASK*   pstTask_out
)
{
	CPU_INT32U  uiDataLimit = 0;
	CPU_INT32U  uiCodeLimit = 0;
	CPU_ADDR    addrLinerarData = 0;
	CPU_ADDR    addrLinerarCode = 0;
	CPU_ERR     ret = CPU_ERR_NONE;

	if (0 == pstTask_out) {
		CPUExt_CorePanic("[PANIC][cpu_task_CopyMemoryPage]Bad Parameter");
	}
	
	pstTask_out->ldt[X86_LDT_NULL].hi = 0;
	pstTask_out->ldt[X86_LDT_NULL].lo = 0;
	pstTask_out->ldt[X86_LDT_CODE].hi = 0x0fff;
	pstTask_out->ldt[X86_LDT_CODE].lo = 0xc0fa00;		
	pstTask_out->ldt[X86_LDT_DATA].hi = 0x3fff;
	pstTask_out->ldt[X86_LDT_DATA].lo = 0xc0f200;
	
	uiCodeLimit = _get_ldt_limit_code();
	uiDataLimit = _get_ldt_limit_data();
	if (uiDataLimit < uiCodeLimit) {
		CPUExt_CorePanic("[PANIC][cpu_task_CopyMemoryPage]Bad data limit");
	}
	
	addrLinerarCode = _get_ldt_base(m_pstCurrentTask->ldt[X86_LDT_CODE]);
	addrLinerarData = _get_ldt_base(m_pstCurrentTask->ldt[X86_LDT_DATA]);
	if (addrLinerarCode != addrLinerarData) {
		CPUExt_CorePanic("[PANIC][cpu_task_CopyMemoryPage]Separated I&D is NOT supported");
	}
	
	_set_ldt_base( pstTask_out->ldt[X86_LDT_CODE], addrLnrStart_in);
	//_set_ldt_limit(pstTask_out->ldt[X86_LDT_CODE], ((uiCodeLimit - 1)));
	
	_set_ldt_base( pstTask_out->ldt[X86_LDT_DATA], addrLnrStart_in);
	//_set_ldt_limit(pstTask_out->ldt[X86_LDT_DATA], ((CPU_TASK_LINERAR_ADDR_RANGE - 1)));
	
	ret = cpu_page_CopyPageTable(addrLinerarData, addrLnrStart_in, uiDataLimit);
	if (CPU_ERR_NONE != ret) {
		cpu_page_ReleasePageTable(addrLnrStart_in, uiDataLimit);
		return (ret);
	}
	
	return (CPU_ERR_NONE);
}


CPU_PRIVATE  CPU_ERR  cpu_task_BuildStack( 
	const CPU_ADDR  addrLnrStart_in, 
	const CPU_DATA  isKernel_in, 
	void *          pParam_in, 
	CPU_FNCT_PTR    pfnReturnPoint_in, 
	CPU_X86_TASK*   pstTask_inout
)
{
	CPU_ADDR     addrPhyMemPage = 0;
	CPU_ADDR     addrLnrTaskEnd = 0;
	CPU_INT32U*  puiStackTop = 0;
	CPU_ERR      ret = CPU_ERR_NONE;
	
	if (0 == pstTask_inout) {
		CPUExt_CorePanic("[PANIC][cpu_task_BuildUserStack]NULL Pointer");
	}
	
	/* get the memroy page for the stack */
	if (DEF_ENABLED == isKernel_in) {
		puiStackTop = (CPU_INT32U *)(pstTask_inout->tss.esp0);
	}
	else {
	    CPUExt_PageGetFree(&addrPhyMemPage);
	    if (0 == addrPhyMemPage) {
		    return (CPU_ERR_NO_MEMORY);
	    }
		puiStackTop = (CPU_INT32U *)((CPU_INT32U)addrPhyMemPage + X86_MEM_PAGE_SIZE - 4);
	}
	
	/* build the stack */
	(*puiStackTop) = 0;
	--puiStackTop;
	(*puiStackTop) = (CPU_STK)pParam_in;
	--puiStackTop;
	(*puiStackTop) = (CPU_STK)pfnReturnPoint_in;
	
	/* the stack for the kernel space task */
	if (DEF_ENABLED == isKernel_in) {
		pstTask_inout->tss.esp0 = (CPU_DATA)puiStackTop;
		pstTask_inout->tss.esp  = pstTask_inout->tss.esp0;
	}
	
	/* the stack for the user space task */
	else {
	    addrLnrTaskEnd = addrLnrStart_in + (CPU_TASK_LINERAR_ADDR_RANGE - 1);
	    ret = cpu_page_PutPageToLinerarAddr(addrPhyMemPage, addrLnrTaskEnd);
	    if (CPU_ERR_NONE != ret) {
		    CPUExt_PageRelease(addrPhyMemPage);
	    	return (ret);
	    }
		pstTask_inout->tss.esp = CPU_TASK_LINERAR_ADDR_RANGE - (3 * 4);
	}
	
	return (ret);
}


