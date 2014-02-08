
/******************************************************************************
    Include
******************************************************************************/

#define CPU_CORE_MODULE
#include <cpu_core.h>
#include <cpu_disp.h>
#include <cpu_page.h>
#include <cpu_gate.h>
#include <cpu_task.h>
#include <cpu_key.h>
#include <cpu_hd.h>
#include <cpu_ext.h>

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
	cpu_hd_Init();
	cpu_key_Init();
	cpu_disp_Init();
	cpu_page_Init(0, 0);
	cpu_gate_Init();
	cpu_task_Init();
	
	return;
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
	cpu_disp_Print(pszMsg_in);
	
	for (;;);
	
	return;
}

void CPUExt_KeyRegisterHandler(CPU_FNCT_PTR pfnKeyHandler_in)
{
	return (cpu_key_RegisterHandler(pfnKeyHandler_in));
}

CPU_INT32U  CPUExt_DispPrint(const CPU_CHAR* pszStr_in)
{
	return (cpu_disp_Print(pszStr_in));
}

void CPUExt_DispChar(const CPU_CHAR chAscii_in)
{
	return (cpu_disp_Char(chAscii_in));
}

