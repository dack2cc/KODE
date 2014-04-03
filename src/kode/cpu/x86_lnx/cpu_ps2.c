
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_ps2.h>
#include <cpu_ext.h>
#include <cpu_asm.h>

/******************************************************************************
    Private Definition
******************************************************************************/

#define CPU_PS2_PORT_DAT				0x0060	
#define CPU_PS2_PORT_STA				0x0064	
#define CPU_PS2_PORT_CMD				0x0064	
#define CPU_PS2_STA_SEND_NOTREADY	    0x02	
#define CPU_PS2_CMD_WRITE_MODE		    0x60	
#define CPU_PS2_KBC_MODE		 	    0x47	
#define CPU_PS2_CMD_SENDTO_MOUSE		0xd4	
#define CPU_PS2_MOUSECMD_ENABLE  		0xf4	

typedef struct _CPU_PS2_CONTROL {
	CPU_FNCT_PTR     pfnKey;
	CPU_FNCT_PTR     pfnMouse;
	CPU_INT08U       uiMouseType;
} CPU_PS2_CONTROL;

CPU_PRIVATE  CPU_PS2_CONTROL  cpu_ps2_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE void cpu_ps2_WaitKBC(void);
CPU_PRIVATE void cpu_ps2_WaitDAT(void);
CPU_PRIVATE void cpu_ps2_MouseWrite(CPU_INT08U byData_in);
CPU_PRIVATE CPU_INT08U cpu_ps2_MouseRead(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_ps2_Init(void)
{
	CPU_INT08U status = 0;
	
	cpu_ps2_stCtl.pfnKey = 0;
	cpu_ps2_stCtl.pfnMouse = 0;
	
	/*
	cpu_ps2_WaitKBC();
    _asm_outb_p(CPU_PS2_CMD_WRITE_MODE, CPU_PS2_PORT_CMD); 
	cpu_ps2_WaitKBC();
	_asm_outb_p(CPU_PS2_KBC_MODE,  CPU_PS2_PORT_DAT); 
	cpu_ps2_WaitKBC();
	_asm_outb_p(CPU_PS2_CMD_SENDTO_MOUSE, CPU_PS2_PORT_CMD);
	cpu_ps2_WaitKBC();
	_asm_outb_p(CPU_PS2_MOUSECMD_ENABLE, CPU_PS2_PORT_DAT);
    */
	
	
	cpu_ps2_WaitKBC();
	_asm_outb_p(0xA8, 0x64);
	
	cpu_ps2_WaitKBC();
	_asm_outb_p(0x20, 0x64);
	cpu_ps2_WaitDAT();
	{
		status = (_asm_inb_p(0x60) | 7);
		cpu_ps2_WaitKBC();
		_asm_outb_p(0x60, 0x64);
		cpu_ps2_WaitKBC();
		_asm_outb_p(status, 0x60);
	}

	cpu_ps2_MouseWrite(0xFF);
	{
	    status = cpu_ps2_MouseRead();
		if (0xFA != status) {
			CPUExt_DispPrint("[No Mouse Connected]");
		}
		//drv_disp_Printf("[cpu_ps2_Init][0x%x]\r\n", status);
		status = cpu_ps2_MouseRead();
		if (0xAA != status) {
			CPUExt_DispPrint("[Mouse init failed ]");
		}
		//drv_disp_Printf("[cpu_ps2_Init][0x%x]\r\n", status);
		cpu_ps2_stCtl.uiMouseType = cpu_ps2_MouseRead();
	}
	
	cpu_ps2_MouseWrite(0xF6);
	cpu_ps2_MouseRead();
	
    cpu_ps2_MouseWrite(0xF4);
	cpu_ps2_MouseRead();
	
	return;
}

void CPUExt_PS2RegisterHandler(const CPU_INT08S iType_in, CPU_FNCT_PTR pfnKeyHandler_in)
{
	switch (iType_in) {
	case CPU_EXT_PS2_TYPE_KEY:
	    cpu_ps2_stCtl.pfnKey = pfnKeyHandler_in;
		break;
	case CPU_EXT_PS2_TYPE_MOUSE:
		cpu_ps2_stCtl.pfnMouse = pfnKeyHandler_in;
		break;
	default:
		// EMPTY
		break;
	}
	
	return;
}

void CPUExt_PS2MouseType(CPU_INT08U * puiType_out)
{
	if (0 != puiType_out) {
		(*puiType_out) = cpu_ps2_stCtl.uiMouseType;
	}
}

void CPUExt_PS2MouseReset(void)
{
	/*
	CPU_INT08U status = 0;
	
    cpu_ps2_MouseWrite(0xF4);
	status = cpu_ps2_MouseRead();
	drv_disp_Printf("[CPUExt_PS2MouseReset][0x%X]\r\n", status);
	status = cpu_ps2_MouseRead();
	drv_disp_Printf("[CPUExt_PS2MouseReset][0x%X]\r\n", status);
	status = cpu_ps2_MouseRead();
	drv_disp_Printf("[CPUExt_PS2MouseReset][0x%X]\r\n", status);
	status = cpu_ps2_MouseRead();
	drv_disp_Printf("[CPUExt_PS2MouseReset][0x%X]\r\n", status);
	//status = cpu_ps2_MouseRead();
	//drv_disp_Printf("[CPUExt_PS2MouseReset][0x%X]\r\n", status);

	//if (0xFA != status) {
		//drv_disp_Printf("[CPUExt_PS2MouseReset][0x%X]\r\n", status);
		//CPUExt_DispPrint("[CPUExt_PS2MouseReset][failed]\r\n");
	//}
	*/
}

void cpu_ps2_ISR_Input(CPU_DATA uiScanCode_in)
{
	CPU_FNCT_PTR pfnHandler = cpu_ps2_stCtl.pfnKey;
	
	if (0 != pfnHandler) {
	    CPU_EXT_PS2_EVENT  stEvent;
	    stEvent.uiScanCode = uiScanCode_in;
		
		(pfnHandler)((void*)(&stEvent));
	}
}

void  cpu_ps2_ISR_Mouse(CPU_DATA uiScanCode_in)
{
	CPU_FNCT_PTR pfnHandler = cpu_ps2_stCtl.pfnMouse;
	
	if (0 != pfnHandler) {
	    CPU_EXT_PS2_EVENT  stEvent;
	    stEvent.uiScanCode = uiScanCode_in;
		
		(pfnHandler)((void*)(&stEvent));
	}
	
	
	//drv_disp_Printf("[Mouse][%0x]\r\n", uiScanCode_in);
}

CPU_PRIVATE void cpu_ps2_WaitKBC(void) 
{
	while (1) {
		if( (_asm_inb_p(CPU_PS2_PORT_STA) & CPU_PS2_STA_SEND_NOTREADY) == 0 ) {
			return;
		}
	}
}

CPU_PRIVATE void cpu_ps2_WaitDAT(void)
{
	while (1) {
		if ((_asm_inb_p(CPU_PS2_PORT_STA) & 1) == 1) {
			return;
		}
	}
}

CPU_PRIVATE void cpu_ps2_MouseWrite(CPU_INT08U byData_in)
{
	cpu_ps2_WaitKBC();
	_asm_outb_p(0xD4, 0x64);
	cpu_ps2_WaitKBC();
	_asm_outb_p(byData_in, 0x60);
}

CPU_PRIVATE CPU_INT08U cpu_ps2_MouseRead(void)
{
	cpu_ps2_WaitDAT();
	
	return (_asm_inb_p(0x60));
}


