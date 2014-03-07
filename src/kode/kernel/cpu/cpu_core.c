
/******************************************************************************
    Include
******************************************************************************/

#define CPU_CORE_MODULE
#include <cpu_core.h>
#include <cpu_page.h>
#include <cpu_gate.h>
#include <cpu_task.h>
#include <cpu_time.h>
#include <cpu_ps2.h>
#include <cpu_hd.h>
#include <cpu_ext.h>

#if   (CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_TEXT)
#include <cpu_disp_text.h>
#elif (CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_8BIT)
#include <cpu_disp_8bit.h>
#else  /* CPU_EXT_DISP_MODE */
#error  No Supported Display Mode.
#endif /* CPU_EXT_DISP_MODE */

/******************************************************************************
    Private Define
******************************************************************************/


/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void CPU_Init(void)
{
	cpu_disp_Init();
	cpu_hd_Init();
	cpu_ps2_Init();
	cpu_page_Init(CPU_EXT_RAM_DISK_SIZE);
	cpu_gate_Init();
	cpu_task_Init();
	cpu_time_Init();
	
	return;
}

void  CPU_SW_Exception(void)
{
	for (;;);
}

CPU_DATA  CPU_CntLeadZeros(CPU_DATA  val_in)
{
	CPU_DATA   iCntZeros = 0;
	CPU_INT08U iBitInVal = (sizeof(val_in) * 8);
	CPU_INT08U i = 0;
	
	for (i = iBitInVal; i > 0; --i) {
		if (0 == (val_in & (1 << (i - 1)))) {
			++iCntZeros;
		}
		else {
			break;
		}
	}
	
	return (iCntZeros);
}


void CPUExt_CorePanic(const CPU_CHAR* pszMsg_in)
{
	CPUExt_DispPrint(pszMsg_in);
	
	for (;;);
	
	return;
}

