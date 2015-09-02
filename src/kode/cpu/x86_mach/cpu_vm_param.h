#ifndef _CPU_VM_PARAM_H_
#define _CPU_VM_PARAM_H_

/* The kernel address space is usually 1GB, usually starting at virtual address 0.  */
/* This can be changed freely to separate kernel addresses from user addresses
 * for better trace support in kdb; the _START symbol has to be offset by the
 * same amount. */
#define VM_MIN_KERNEL_ADDRESS   0xC0000000UL


#endif // _CPU_VM_PARAM_H_

