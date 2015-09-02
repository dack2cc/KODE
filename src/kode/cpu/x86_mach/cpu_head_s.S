
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
	jmp boot_entry

