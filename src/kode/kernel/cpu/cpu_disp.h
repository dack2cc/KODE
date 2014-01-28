#ifndef __CPU_DISP_H__
#define __CPU_DISP_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

void cpu_disp_Init(void);

CPU_INT32U  cpu_disp_Print(const CPU_CHAR* pszStr_in);
void        cpu_disp_Char(const  CPU_CHAR  chAscii_in);

#endif // __CPU_DISP_H__

