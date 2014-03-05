
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_key.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Definition
******************************************************************************/

typedef struct _CPU_KEY_CONTROL {
	CPU_FNCT_PTR     pfnHandler;
} CPU_KEY_CONTROL;

CPU_PRIVATE  CPU_KEY_CONTROL  cpu_key_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_key_Init(void)
{
    cpu_key_stCtl.pfnHandler = 0;
	
	return;
}

void  CPUExt_KeyRegisterHandler(CPU_FNCT_PTR pfnKeyHandler_in)
{
	cpu_key_stCtl.pfnHandler = pfnKeyHandler_in;
	
	return;
}

void cpu_key_ISR_Input(CPU_DATA uiScanCode_in)
{
	CPU_FNCT_PTR pfnHandler = cpu_key_stCtl.pfnHandler;
	
	if (0 != pfnHandler) {
	    CPU_EXT_KEY_EVENT  stEvent;
	    stEvent.uiScanCode = uiScanCode_in;
		
		(pfnHandler)((void*)(&stEvent));
	}
	
	return;
}

void  cpu_key_ISR_Mouse(CPU_DATA ui_in)
{
	CPUExt_DispPrint("[Mouse][Received] ");
}

