#ifndef __CPU_DISP_8_H__
#define __CPU_DISP_8_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu_ext.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#if (CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_8BIT)

#define CPU_DISP_VRAM   (*((CPU_INT32U *)0x90016))

extern void  cpu_disp_Init(void);

#endif /* CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_8BIT */

#endif // __CPU_DISP_8_H__

