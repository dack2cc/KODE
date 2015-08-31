
	.section .text.start, "ax"
	.globl start, _start

start:
_start:
	jmp boot_entry

#define MULTIBOOT_MAGIC  0x1BADB002
#ifdef __ELF__
#define MULTIBOOT_FLAGS  0x00000002
#else
#define MULTIBOOT_FLAGS  0x00010002
#endif
	.p2align 2
boot_hdr:
//       .long  MULTIBOOT_MAGIC
//       .long  MULTIBOOT_FLAGS
//       .long  - (MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)
	.long  0x1BADB002
	.long  0x00000002
	.long  - (0x1BADB002 + 0x00000002)
#ifndef __ELF__
	.long  boot_hdr
	.long  _start
	.long  _edata
	.long  _end
	.long  boot_entry
#endif

boot_entry:
	jmp boot_entry

