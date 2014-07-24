
/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <cpu_boot.h>

/******************************************************************************
    Public Definition
******************************************************************************/


CPU_PRIVATE CPU_INT32U cpu_head_aiTask0UserStack [ X86_MEM_PAGE_SIZE>>2 ] ;


struct _STACK_START {
	CPU_INT32U * a;
	CPU_INT16U b;
} cpu_head_stTask0UserStackStart = { &(cpu_head_aiTask0UserStack[ X86_MEM_PAGE_SIZE>>2 ]), 0x10 };


