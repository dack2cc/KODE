#ifndef __CPU_GATE_H__
#define __CPU_GATE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Define
******************************************************************************/


/******************************************************************************
    Public Interface
******************************************************************************/

extern void cpu_gate_Init(void);

extern CPU_DATA  cpu_gate_ISRKernelFnct(void);
extern CPU_DATA  cpu_gate_ISRMemPageFault(void);
extern CPU_DATA  cpu_gate_ISRTimeTick(void);
extern CPU_DATA  cpu_gate_ISRKeyboard(void);
extern CPU_DATA  cpu_gate_ISRHardDisk(void);
extern CPU_DATA  cpu_gate_ISRMouse(void);

#endif // __CPU_GATE_H__

