#ifndef _CPU_VM_PARAM_H_
#define _CPU_VM_PARAM_H_

#include <cpu.h>

/* The kernel address space is usually 1GB, usually starting at virtual address 0.  */
/* This can be changed freely to separate kernel addresses from user addresses
 * for better trace support in kdb; the _START symbol has to be offset by the
 * same amount. */
#define VM_MIN_KERNEL_ADDRESS   0xC0000000UL

#define I386_PGBYTES    4096  /* bytes per 80386 page */

/* interrupt statck size  */
#ifdef MACH_PV_PAGETABLES
/* need room for mmu updates (2*8bytes)  */
#define KERNEL_STACK_SIZE   (4*I386_PGBYTES)
#define INTSTACK_SIZE       (4*I386_PGBYTES)
#else  // MACH_PV_PAGETABLES
#define KERNEL_STACK_SIZE   (1*I386_PGBYTES)
#define INTSTACK_SIZE       (1*I386_PGBYTES)
#endif // MACH_PV_PAGETABLES


/*
*  Physical memory is direct-mapped to virutal memory
*  starting at virual address VM_MIN_KERNEL_ADDRESS
*/
#define phystokv(a)  ((CPU_ADDR)(a) + VM_MIN_KERNEL_ADDRESS)

#endif // _CPU_VM_PARAM_H_

