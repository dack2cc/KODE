
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_key.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Definition
******************************************************************************/

typedef struct _KEYBOARD_CONTROL {
	CPU_FNCT_PTR     pfnHandler;
    CPU_INT08U       uiMode;
	CPU_INT08U       uiFollowKeyNum;
	const CPU_CHAR*  pchKeyMap;
	const CPU_CHAR*  pchAltMap;
	const CPU_CHAR*  pchShfMap;
} KEYBOARD_CONTROL;

CPU_PRIVATE  KEYBOARD_CONTROL  cpu_key_stCtl;

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

void  cpu_key_RegisterHandler(CPU_FNCT_PTR pfnKeyHandler_in)
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
		
		//CPUExt_DispPrint("Key Handler call \r\n");
		(pfnHandler)((void*)(&stEvent));
	}
	
	return;
}


