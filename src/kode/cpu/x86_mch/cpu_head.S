#include <cpu_asm_symc.h>
#include <cpu_asm.h>

	/*
	*  This section will be put first into .text. See also cfg_mach_ldscript
	*/
	.section .text.start, "ax"
	
	/* We should never be entered this way. */
	.globl start, _start
start:
_start:
	jmp boot_entry

/* 
*  MultiBoot header - see the site below:
*  http://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
*/
#define MULTIBOOT_MAGIC  0x1BADB002
#ifdef __ELF__
#define MULTIBOOT_FLAGS  0x00000002
#else  /* __ELF__ */
#define MULTIBOOT_FLAGS  0x00010002
#endif /* __ELF__ */
	.p2align 2
boot_hdr:
	.long  MULTIBOOT_MAGIC
	.long  MULTIBOOT_FLAGS
	/*
	* The next item here is the checksum.
	* XX this works OK until we need at least the 30th bit.
	*/
	.long  - (MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)
#ifndef __ELF__  /* a.out kludge */
	.long  boot_hdr   /* header_addr  */
	.long  _start     /* load_addr */
	.long  _edata     /* load_end_addr */
	.long  _end       /* bss_end_addr */
	.long  boot_entry /* entry  */
#endif /* __ELF__ */

boot_entry:
	/* use segmentation to offset ourself */
	lgdt boot_gdt_descr - KERNELBASE
	ljmp $8, $0f
0:
	movw $0, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw $16, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	/* switch to our own interrupt stack */
	movl $_intstack+INTSTACK_SIZE, %esp

	/* Reset EFLAGAS to a known state. */
	pushl $0
	popf

	/* clear uninitialized data */
	lea _edata, %edi
	lea _end, %ecx
	subl %edi, %ecx
	xorl %eax,%eax
	rep
	stosb

	/* push the boot_info pointer to be the second argument. */
	pushl %ebx

	/* fix ifunc entries */
	movl $__rel_iplt_start, %esi
	movl $__rel_iplt_end, %edi
iplt_cont:
	cmpl %edi, %esi
	jae iplt_done
	movl (%esi), %ebx /* r_offset */
	movb 4(%esi), %al /* info  */
	cmpb $42, %al     /* IRELATIVE  */
	jnz iplt_next
	call *(%ebx)      /* call ifunc */
	movl %eax, (%ebx) /* fixed address */
iplt_next:
	addl $8, %esi
	jmp iplt_cont
iplt_done:
	
	/* jump into C code. */
	call EXT(cpu_boot_Entry)

	.comm _intstack, INTSTACK_SIZE

.align 16
	.word 0
boot_gdt_descr:
	.word 3*8+7
	.long boot_gdt - KERNELBASE
.align 16
boot_gdt:
	/* 0 */
	.quad 0
	/* boot CS = 8 */
	.word 0xffff
	.word (-KERNELBASE) & 0xffff
	.byte ((-KERNELBASE) >> 16) & 0xff
	.byte 0x9a
	.byte 0xcf
	.byte ((-KERNELBASE) >> 24) & 0xff
	/* boot DS = 8 */
	.word 0xffff
	.word (-KERNELBASE) & 0xffff
	.byte ((-KERNELBASE) >> 16) & 0xff
	.byte 0x92
	.byte 0xcf
	.byte ((-KERNELBASE) >> 24) & 0xff


