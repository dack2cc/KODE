
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_gate.h>
#include <cpu_task.h>
#include <cpu_time.h>
#include <cpu_asm.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Definition
******************************************************************************/

#define CPU_GATE_PIC0_ICW1		0x0020	///< ICW(Inital Control Word)
#define CPU_GATE_PIC0_OCW2		0x0020	///< ?
#define CPU_GATE_PIC0_IMR		0x0021	///< IMR(Interrupt Mask Regisiter)
#define CPU_GATE_PIC0_ICW2		0x0021	///< ICW(Inital Control Word) 
#define CPU_GATE_PIC0_ICW3		0x0021	///< ICW(Inital Control Word) 
#define CPU_GATE_PIC0_ICW4		0x0021	///< ICW(Inital Control Word) 

#define CPU_GATE_PIC1_ICW1		0x00a0	///< ICW(Inital Control Word) 
#define CPU_GATE_PIC1_OCW2		0x00a0	///< ?
#define CPU_GATE_PIC1_IMR		0x00a1	///< IMR(Interrupt Mask Regisiter) 
#define CPU_GATE_PIC1_ICW2		0x00a1	///< ICW(Inital Control Word) 
#define CPU_GATE_PIC1_ICW3		0x00a1	///< ICW(Inital Control Word) 
#define CPU_GATE_PIC1_ICW4		0x00a1	///< ICW(Inital Control Word) 


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

CPU_PRIVATE void cpu_gate_SetupPIC(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_gate_Init(void)
{
    cpu_gate_SetupPIC();

    /* register the kernel function */
    _asm_set_isr_sysc(0x80, &cpu_gate_ISRKernelFnct);

    /* memory page access fault */
    _asm_set_isr_trap(14, &cpu_gate_ISRMemPageFault);
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC0_IMR) & 0xfb, CPU_GATE_PIC0_IMR);
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC1_IMR) & 0xdf, CPU_GATE_PIC1_IMR);

    /* start the timer of 8253 */
    _asm_outb_p(0x36, 0x43);
    _asm_outb_p((_LATCH & 0xFF), 0x40);
    _asm_outb_p((_LATCH >> 8),   0x40);
    _asm_set_isr_intr(0x20, &cpu_gate_ISRTimeTick);
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC0_IMR)&~0x01, CPU_GATE_PIC0_IMR);  /* enable the timer interrupt */


    /* start the keyboard function */
    _asm_set_isr_intr(0x21, &cpu_gate_ISRKeyboard);
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC0_IMR) & 0xfd, CPU_GATE_PIC0_IMR); /* enable the keyboard interrupt */
    {
        //register CPU_INT08U uiKeyboardState=_asm_inb_p(0x61);  /* read the keyboard state */
        //_asm_outb_p(uiKeyboardState|0x80,0x61);                /* disable the keyboard */
        //_asm_outb_p(uiKeyboardState,0x61);                     /* enable the keyboard  */
    }

    /* start the mouse function */
    _asm_set_isr_intr(0x2C, &cpu_gate_ISRMouse);
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC0_IMR) & 0xFB, CPU_GATE_PIC0_IMR); /* enable the mouse interrupt */
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC1_IMR) & 0xEF, CPU_GATE_PIC1_IMR); /* enable the mouse interrupt */

    /* start the harddisk function */
    _asm_set_isr_intr(0x2E, &cpu_gate_ISRHardDisk);
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC0_IMR) & 0xFB, CPU_GATE_PIC0_IMR); /* reset the 8259A int2 to enable the interrupt */
    _asm_outb_p(_asm_inb_p(CPU_GATE_PIC1_IMR) & 0xBF, CPU_GATE_PIC1_IMR); /* reset harddisk control to enable the interrupt */


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

CPU_PRIVATE void cpu_gate_SetupPIC(void)
{
    // disable all the interrupt

    _asm_outb_p(0xFF, CPU_GATE_PIC0_IMR);
    _asm_outb_p(0xFF, CPU_GATE_PIC1_IMR);

    _asm_outb_p(0x11,     CPU_GATE_PIC0_ICW1);   // edge trigger mode
    _asm_outb_p(0x20,     CPU_GATE_PIC0_ICW2);   // IRQ 0 -7 -> INT 0x20 - 0x27
    _asm_outb_p((1 << 2), CPU_GATE_PIC0_ICW3);   // PIC1 -> IRQ2
    _asm_outb_p(0x01,     CPU_GATE_PIC0_ICW4);   // no buffering

    _asm_outb_p(0x11, CPU_GATE_PIC1_ICW1); // edge trigger mode
    _asm_outb_p(0x28, CPU_GATE_PIC1_ICW2); // IRQ 8 - 15 -> INT 28 - 2F
    _asm_outb_p(2,    CPU_GATE_PIC1_ICW3); // PIC1 -> IRQ2
    _asm_outb_p(0x01, CPU_GATE_PIC1_ICW4); // no buffering

    // disable all the interrupt except PIC1
    _asm_outb_p(0xFF, CPU_GATE_PIC0_IMR);
    _asm_outb_p(0xFF, CPU_GATE_PIC1_IMR);

}

