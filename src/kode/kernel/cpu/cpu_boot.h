#ifndef __CPU_BOOT_H__
#define __CPU_BOOT_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Display Information
******************************************************************************/

/* parameter from the bios */
#define X86_DISPLAY_MODE       ((*((CPU_INT16U *)0x90006)) & 0xFF)
#define X86_DISPLAY_COL_NUM    (((*((CPU_INT16U *)0x90006)) & 0xFF00) >> 8)
#define X86_DISPLAY_ROW_NUM    (25)
#define X86_DISPLAY_EGA_AX     (*(CPU_INT16U *)0x90008)
#define X86_DISPLAY_EGA_BX     (*(CPU_INT16U *)0x9000A)
#define X86_DISPLAY_EGA_CX     (*(CPU_INT16U *)0x9000C)
#define X86_DISPLAY_POS_COL    (*(CPU_INT08U *)0x90000)
#define X86_DISPLAY_POS_ROW    (*(CPU_INT08U *)0x90001)

/******************************************************************************
    Memory Page
******************************************************************************/

#define X86_MEM_EXT_SIZE_IN_KB   (*(CPU_INT16U *)0x90002)
#define X86_MEM_PAGE_SIZE        (4*1024)  /* 4KB for one memory page */
#define X86_MEM_PAGE_IN_TABLE    (1024)    /* 4KB / 4Byte = 1024 */
#define X86_MEM_PAGE_TABLE_DIR   g_s_head_p32BitPageDir
extern  CPU_INT32U  g_s_head_p32BitPageDir[X86_MEM_PAGE_IN_TABLE];

/******************************************************************************
    Descriptor Table (GDT & IDT & LDT)
******************************************************************************/

typedef struct {
	CPU_INT32U hi;  /* high 32bit */
	CPU_INT32U lo;  /* low 32bit  */
} X86_DESC;

#define X86_IDT             g_s_head_p64BitIDT
#define X86_GDT             g_s_head_p64BitGDT
#define X86_DESC_TBL_MAX    (256)
extern  X86_DESC   g_s_head_p64BitIDT[X86_DESC_TBL_MAX];
extern  X86_DESC   g_s_head_p64BitGDT[X86_DESC_TBL_MAX];

enum {
	X86_GDT_NULL   = 0,
	X86_GDT_CODE   = 1,
	X86_GDT_DATA   = 2,
	X86_GDT_SYSTEM = 3,
	X86_GDT_TSS_0  = 4,
	X86_GDT_LDT_0  = 5,
};

enum {
	X86_LDT_NULL = 0,
	X86_LDT_CODE = 1,
	X86_LDT_DATA = 2,
	X86_LDT_MAX
};

#endif // __CPU_BOOT_H__

