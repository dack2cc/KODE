
#ifndef __CPU_ASM_H__
#define __CPU_ASM_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <cpu_boot.h>

/******************************************************************************
    common function
******************************************************************************/

#define _asm_nop() __asm__ ("nop"::)

/******************************************************************************
    access the I/O port
******************************************************************************/

#define _asm_outb(value, port) \
    __asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))

#define _asm_inb(port) ({ \
    unsigned char _v; \
    __asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
    _v; \
    })

#define _asm_outb_p(value, port) \
    __asm__ volatile ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

#define _asm_inb_p(port) ({ \
    unsigned char _v; \
    __asm__ volatile ("inb %%dx,%%al\n" \
	    "\tjmp 1f\n" \
	    "1:\tjmp 1f\n" \
	    "1:":"=a" (_v):"d" (port)); \
    _v; \
    })


/******************************************************************************
    set the gate(ISR) for the cpu
******************************************************************************/

#define _asm_set_isr_intr(n, isr)    __asm_set_gate(&X86_IDT[n], 14, 0, isr)
#define _asm_set_isr_trap(n, isr)    __asm_set_gate(&X86_IDT[n], 15, 0, isr)
#define _asm_set_isr_sysc(n, isr)    __asm_set_gate(&X86_IDT[n], 15, 3, isr)


#define __asm_set_gate(gate_addr, type, dpl, isr) \
    __asm__ ("movw %%dx,%%ax\n\t" \
	    "movw %0,%%dx\n\t" \
	    "movl %%eax,%1\n\t" \
	    "movl %%edx,%2" \
	    : \
	    : "i" ((CPU_INT16S) (0x8000+(dpl<<13)+(type<<8))), \
	      "o" (*((CPU_INT08S *) (gate_addr))), \
	      "o" (*(4+(CPU_INT08S *) (gate_addr))), \
	      "d" ((CPU_INT08S *) (isr)),"a" (0x00080000))


#endif // __CPU_ASM_H__

