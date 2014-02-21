
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_gate.h>
#include <cpu_disp.h>
#include <cpu_task.h>
#include <cpu_time.h>
#include <cpu_asm.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Definition
******************************************************************************/

#define CPU_KF_MAX  (256)
typedef CPU_INT32U  (*CPU_KERNEL_FNCT)(void);

CPU_KERNEL_FNCT  cpu_gate_apfnKernelFnct[CPU_KF_MAX] = { 0, };
CPU_INT32U       cpu_gate_uiKernelFnctNum = CPU_KF_MAX;

CPU_FNCT_VOID    cpu_gate_pfnISRHookEnter = 0;
CPU_FNCT_VOID    cpu_gate_pfnISRHookExit  = 0;
CPU_FNCT_VOID    cpu_gate_pfnTimeTick     = 0;


#define _HZ       (CPU_TIME_TICK_HZ)
#define _LATCH    (1193180/_HZ)


/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_gate_Init(void)
{
	register CPU_INT08U uiKeyboardState;
	
	/* memory page access fault */
	_asm_set_isr_trap(14, &cpu_gate_ISRMemPageFault);
	_asm_outb_p(_asm_inb_p(0x21)&0xfb,0x21);
	_asm_outb(_asm_inb_p(0xA1)&0xdf,0xA1);
	
	/* start the timer of 8253 */
	_asm_outb_p(0x36, 0x43);
	_asm_outb_p((_LATCH & 0xFF), 0x40);
	_asm_outb_p((_LATCH >> 8),   0x40);
	_asm_set_isr_intr(0x20, &cpu_gate_ISRTimeTick);
	_asm_outb(_asm_inb_p(0x21)&~0x01, 0x21);  /* enable the timer interrupt */
	
	/* register the kernel function */
	_asm_set_isr_sysc(0x80, &cpu_gate_ISRKernelFnct);
	
	/* start the keyboard function */
	_asm_set_isr_trap(0x21, &cpu_gate_ISRKeyboard);
	_asm_outb_p(_asm_inb_p(0x21)&0xfd,0x21);     /* enable the keyboard interrupt */
	uiKeyboardState=_asm_inb_p(0x61);       /* read the keyboard state */
	_asm_outb_p(uiKeyboardState|0x80,0x61); /* disable the keyboard */
	_asm_outb(uiKeyboardState,0x61);        /* enable the keyboard  */
	
	/* start the harddisk function */
	_asm_set_isr_intr(0x2E, &cpu_gate_ISRHardDisk);
	_asm_outb_p(_asm_inb_p(0x21)&0xFB, 0x21); /* reset the 8259A int2 to enable the interrupt */
	_asm_outb(_asm_inb_p(0xA1)&0xBF, 0xA1);   /* reset harddisk control to enable the interrupt */
	
	return;
}

void CPUExt_GateRegisterISRHookEnter(CPU_FNCT_VOID pfnISRHookEnter_in)
{
	cpu_gate_pfnISRHookEnter = pfnISRHookEnter_in;
}

void CPUExt_GateRegisterISRHookExit(CPU_FNCT_VOID pfnISRHookExit_in)
{
	cpu_gate_pfnISRHookExit = pfnISRHookExit_in;
}

void CPUExt_GateRegisterTimeTick(CPU_FNCT_VOID pfnTimeTick_in)
{
	cpu_gate_pfnTimeTick = pfnTimeTick_in;
}

CPU_ERR  CPUExt_GateRegisterKernelFnct(const CPU_INT32U  uiFnctNum_in, CPU_FNCT_VOID pfnKernelFnct_in)
{
	if (uiFnctNum_in >= CPU_KF_MAX) {
		return (CPU_ERR_BAD_PARAM);
	}
	if (0 == pfnKernelFnct_in) {
		return (CPU_ERR_NULL_PTR);
	}
	
	cpu_gate_apfnKernelFnct[uiFnctNum_in] = (CPU_KERNEL_FNCT)(pfnKernelFnct_in);
	
	return (CPU_ERR_NONE);
}



