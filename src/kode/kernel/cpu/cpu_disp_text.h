#ifndef __CPU_DISP_H__
#define __CPU_DISP_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu_ext.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#if (CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_TEXT)

void cpu_disp_Init(void);

#endif /* CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_TEXT */

#endif // __CPU_DISP_H__

