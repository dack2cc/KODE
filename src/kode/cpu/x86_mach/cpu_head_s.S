#include <cpu_asm_symc.h>

    /*
    *  This section will be put first into .text. See also cfg_mach_ldscript
    */
	.section .text.start, "ax"
	
    /* We should never be entered this way. */
    .globl start, _start
start:
_start:
	jmp boot_entry

    /* MultiBoot header - see the site below:
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
    .long  - (MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)
#ifndef __ELF__  /* a.out kludge */
	.long  boot_hdr
	.long  _start
	.long  _edata
	.long  _end
	.long  boot_entry
#endif /* __ELF__ */

boot_entry:
	/* use segmentation to offset ourself */
	lgdt boot_gdt_descr - KERNELBASE
	jmp boot_entry

.align 16
	.word 0
boot_gdt_descr:
	.word 3*8+7
	.long boot_gdt - KERNELBASE
.align 16
boot_gdt:
	/* 0 */
	.quad 0
	/* boot CS = 0 */
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
	.byte 0x9a
	.byte 0xcf
	.byte ((-KERNELBASE) >> 24) & 0xff


