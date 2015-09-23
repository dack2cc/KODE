
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_ext.h>
#include <cpu_vm_param.h>
#include <cpu_util.h>

/******************************************************************************
    Definition
******************************************************************************/

/* The boot loader passes this data structure to the kernel in
   register EBX on entry.  */
typedef struct _CPU_BOOT_INFO {
    /* These flags indicate which parts of the multiboot_info are valid;
       see below for the actual flag bit definitions.  */
    CPU_INT32U  flags;

    /* Lower/Upper memory installed in the machine.
       Valid only if CPU_BOOT_FLAG_MEMORY is set in flags word above.  */
    CPU_ADDR    mem_lower;
    CPU_ADDR    mem_upper;

    /* BIOS disk device the kernel was loaded from.
       Valid only if CPU_BOOT_FLAG_BOOT_DEVICE is set in flags word above.  */
    CPU_INT08U  boot_device[4];

    /* Command-line for the OS kernel: a null-terminated ASCII string.
       Valid only if CPU_BOOT_FLAG_CMDLINE is set in flags word above.  */
    CPU_ADDR    cmdline;

    /* List of boot modules loaded with the kernel.
       Valid only if CPU_BOOT_FLAG_MODS is set in flags word above.  */
    CPU_INT32U  mods_count;
    CPU_ADDR    mods_addr;

    /* Symbol information for a.out or ELF executables. */
    union {
        /* a.out symbol information valid only if CPU_BOOT_FLAG_AOUT_SYMS
           is set in flags word above.  */
        struct {
            CPU_SIZE_T  tabsize;
            CPU_SIZE_T  strsize;
            CPU_ADDR    addr;
            CPU_INT32U  reserved;
        } aout_sym;

        /* ELF section header information valid only if
           CPU_BOOT_FLAG_ELF_SHDR is set in flags word above.  */
        struct {
            CPU_INT32U  num;
            CPU_SIZE_T  size;
            CPU_ADDR    addr;
            CPU_INT32U  shndx;
        } elf_sec;
    } syms;

    /* Memory map buffer.
       Valid only if CPU_BOOT_FLAG_MEM_MAP is set in flags word above.  */
    CPU_SIZE_T  mmap_length;
    CPU_ADDR    mmap_addr;

    /* drive info buffer */
    CPU_SIZE_T  driver_length;
    CPU_ADDR    driver_addr;

    /* ROM configuration table */
    CPU_INT32U  config_table;

    /* Boot Loader Name */
    CPU_INT32U  boot_loader_name;

    /* APM table */
    CPU_INT32U  apm_table;

    /* Video */
    CPU_INT32U  vbe_control_info;
    CPU_INT32U  vbe_mode_info;
    CPU_INT16U  vbe_mode;
    CPU_INT16U  vbe_interface_seg;
    CPU_INT16U  vbe_interface_off;
    CPU_INT16U  vbe_interface_len;
} CPU_BOOT_INFO;

#define CPU_BOOT_FLAG_MEMORY        0x00000001
#define CPU_BOOT_FLAG_BOOT_DEVICE   0x00000002
#define CPU_BOOT_FLAG_CMDLINE       0x00000004
#define CPU_BOOT_FLAG_MODS          0x00000008
#define CPU_BOOT_FLAG_AOUT_SYMS     0x00000010
#define CPU_BOOT_FLAG_ELF_SHDR      0x00000020
#define CPU_BOOT_FLAG_MEM_MAP       0x00000040

CPU_PRIVATE CPU_BOOT_INFO cpu_boot_stInfo;

/******************************************************************************
    Function
******************************************************************************/

/*
*  C boot entrypoint - called by boot entry in cpu_head_s.S
*  Running in 32-bit flat mode, but without paging yet.
*/
void cpu_boot_Entry(void* pBootInfo_in)
{
    CPU_INT32U  iCpuType = 0;

    /* Stash the boot_image_info pointer.  */
    cpu_boot_stInfo = *(typeof(cpu_boot_stInfo)*)phystokv(pBootInfo_in);

    if (cpu_boot_stInfo.flags & CPU_BOOT_FLAG_AOUT_SYMS) {
        CPUExt_DispPrint("kernel symbol table \r\n");
    }

    if (cpu_boot_stInfo.flags & CPU_BOOT_FLAG_ELF_SHDR) {
        CPUExt_DispPrint("ELF section header table \r\n");
    }

    iCpuType = cpu_util_DiscoverX86Type();

    switch (iCpuType) {
    case 3:
        CPUExt_DispPrint("cpu is i386 \r\n");
        break;

    case 4:
        CPUExt_DispPrint("cpu is i486 \r\n");
        break;

    case 5:
        CPUExt_DispPrint("cpu is pentium \r\n");
        break;

    case 6:
    case 15:
        CPUExt_DispPrint("cpu is pentium pro \r\n");
        break;

    default:
        CPUExt_DispPrint("cpu is unkown \r\n");
        break;
    }

    CPUExt_DispPrint("yuKi");

    while (1);
}

