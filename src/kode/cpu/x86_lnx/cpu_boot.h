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

/******************************************************************************
    Hard Disk Information
******************************************************************************/

#define X86_HD_ORIG_ROOT_DEV  (*(CPU_INT16U *)0x901FC)

/* parameter from the bios */
typedef struct {
    CPU_INT08U  abyDummy[32];
} X86_HD_INFO;
#define  X86_HD_PARAM   (*(X86_HD_INFO *)(0x90080))

/* Hd controller regs. Ref: IBM AT Bios-listing */
#define  X86_HD_DATA     (0x1f0) /* _CTL when writing */
#define  X86_HD_ERROR    (0x1f1) /* see err-bits */
#define  X86_HD_NSECTOR  (0x1f2) /* nr of sectors to read/write */
#define  X86_HD_SECTOR   (0x1f3) /* starting sector */
#define  X86_HD_LCYL     (0x1f4) /* starting cylinder */
#define  X86_HD_HCYL     (0x1f5) /* high byte of starting cyl */
#define  X86_HD_CURRENT  (0x1f6) /* 101dhhhh , d=drive, hhhh=head */
#define  X86_HD_STATUS   (0x1f7) /* see status-bits */
#define  X86_HD_PRECOMP  X86_HD_ERROR   /* same io address, read=error, write=precomp */
#define  X86_HD_COMMAND  X86_HD_STATUS  /* same io address, read=status, write=cmd */

#define  X86_HD_CMD		(0x3f6)

/* Bits of HD_STATUS */
#define  X86_HD_STATUS_ERR     (0x01)
#define  X86_HD_STATUS_INDEX   (0x02)
#define  X86_HD_STATUS_ECC     (0x04)  /* Corrected error */
#define  X86_HD_STATUS_DRQ     (0x08)
#define  X86_HD_STATUS_SEEK    (0x10)
#define  X86_HD_STATUS_WRERR   (0x20)
#define  X86_HD_STATUS_READY   (0x40)
#define  X86_HD_STATUS_BUSY    (0x80)

/* Values for HD_COMMAND */
#define  X86_HD_WIN_RESTORE    (0x10)
#define  X86_HD_WIN_READ       (0x20)
#define  X86_HD_WIN_WRITE      (0x30)
#define  X86_HD_WIN_VERIFY     (0x40)
#define  X86_HD_WIN_FORMAT     (0x50)
#define  X86_HD_WIN_INIT       (0x60)
#define  X86_HD_WIN_SEEK       (0x70)
#define  X86_HD_WIN_DIAGNOSE   (0x90)
#define  X86_HD_WIN_SPECIFY    (0x91)


#endif // __CPU_BOOT_H__

