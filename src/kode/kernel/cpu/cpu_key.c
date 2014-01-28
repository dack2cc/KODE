
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_key.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Definition
******************************************************************************/

#define _CPU_KEY_BREAK_BIT_MASK  (0x80)
#define _CPU_KEY_SCAN_CODE_MASK  (0xFF)

typedef void (*_CPU_KEY_FNCT_PTR)(CPU_INT08U  uiScanCode_in);

typedef struct _KEYBOARD_CONTROL {
	CPU_FNCT_PTR     pfnHandler;
    CPU_INT08U       uiMode;
	CPU_INT08U       uiFollowKeyNum;
	const CPU_CHAR*  pchKeyMap;
	const CPU_CHAR*  pchAltMap;
	const CPU_CHAR*  pchShfMap;
} KEYBOARD_CONTROL;

CPU_PRIVATE  KEYBOARD_CONTROL  m_stCtl;

CPU_PRIVATE const CPU_CHAR m_achUSKeyMap[] = {
/* 00-0F */   0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 127, 9,
/* 10-1F */ 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,  0,   'a', 's',
/* 20-2F */ 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  0,  '`',  0,  0,   0,   0,   0,   0,
};

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE  void cpu_key_CallHandler(CPU_INT08U uiState_in, CPU_INT08U  uiAsciiCode_in);
CPU_PRIVATE  void cpu_key_ReceiverNone(CPU_INT08U  uiScanCode_in);
CPU_PRIVATE  void cpu_key_ReceiverNormal(CPU_INT08U  uiScanCode_in);
CPU_PRIVATE  void cpu_key_ReceiverFunction(CPU_INT08U  uiScanCode_in);

CPU_PRIVATE const _CPU_KEY_FNCT_PTR   m_apfnReceiver[_CPU_KEY_SCAN_CODE_MASK + 1] = {
/* 00-03 s0 esc 1 2 */ cpu_key_ReceiverNone, cpu_key_ReceiverNormal, cpu_key_ReceiverNormal, cpu_key_ReceiverNormal,
/* 04-07 3 4 5 6 */ cpu_key_ReceiverNormal, cpu_key_ReceiverNormal, cpu_key_ReceiverNormal, cpu_key_ReceiverNormal,
/* 08-0B 7 8 9 0 */ cpu_key_ReceiverNormal, cpu_key_ReceiverNormal, cpu_key_ReceiverNormal, cpu_key_ReceiverNormal,
/* 0C-0F + ' bs tab */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 10-13 q w e r */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 14-17 t y u i */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 18-1B o p } ^ */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 1C-1F enter ctrl a s */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 20-23 d f g h */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 24-27 j k l | */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 28-2B { para lshift , */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 2C-2F z x c v */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 30-33 b n m , */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 34-37 . - rshift * */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 38-3B alt sp caps f1 */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 3C-3F f2 f3 f4 f5 */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 40-43 f6 f7 f8 f9 */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 44-47 f10 num scr home */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 48-4B up pgup - left */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 4C-4F n5 right + end */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 50-53 dn pgdn ins del */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 54-57 sysreq ? < f11 */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 58-5B f12 ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 5C-5F ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 60-63 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 64-67 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 68-6B ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 6C-6F ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 70-73 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 74-77 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 78-7B ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 7C-7F ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 80-83 ? br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 84-87 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 88-8B br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 8C-8F br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 90-93 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 94-97 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 98-9B br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* 9C-9F br unctrl br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* A0-A3 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* A4-A7 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* A8-AB br br unlshift br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* AC-AF br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* B0-B3 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* B4-B7 br br unrshift br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* B8-BB unalt br uncaps br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* BC-BF br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* C0-C3 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* C4-C7 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* C8-CB br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* CC-CF br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* D0-D3 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* D4-D7 br br br br */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* D8-DB br ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* DC-DF ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* E0-E3 e0 e1 ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* E4-E7 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* E8-EB ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* EC-EF ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* F0-F3 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* F4-F7 ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* F8-FB ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone,
/* FC-FF ? ? ? ? */ cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone, cpu_key_ReceiverNone
};

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_key_Init(void)
{
    m_stCtl.pfnHandler = 0;
	m_stCtl.uiMode = 0;
	m_stCtl.uiFollowKeyNum = 0;
	m_stCtl.pchKeyMap = &m_achUSKeyMap[0];
	m_stCtl.pchAltMap = &m_achUSKeyMap[0];
	m_stCtl.pchShfMap = &m_achUSKeyMap[0];
	
	return;
}

void  cpu_key_RegisterHandler(CPU_FNCT_PTR pfnKeyHandler_in)
{
	m_stCtl.pfnHandler = pfnKeyHandler_in;
	
	return;
}

void cpu_key_ISR_Input(CPU_DATA uiScanCode_in)
{
	CPU_INT08U uiScanCode = uiScanCode_in & _CPU_KEY_SCAN_CODE_MASK;
	
    m_apfnReceiver[uiScanCode](uiScanCode);
}

CPU_PRIVATE  void cpu_key_CallHandler(CPU_INT08U uiState_in, CPU_INT08U  uiAsciiCode_in)
{
	CPU_EXT_KEY_EVENT  stEvent;
	
	stEvent.uiState = uiState_in;
	stEvent.uiAscii = uiAsciiCode_in;

	if (0 != m_stCtl.pfnHandler) {
		m_stCtl.pfnHandler((void*)(&stEvent));
	}
}

CPU_PRIVATE  void cpu_key_ReceiverNone(CPU_INT08U  uiScanCode_in)
{
	return;
}

CPU_PRIVATE  void cpu_key_ReceiverNormal(CPU_INT08U  uiScanCode_in)
{
	CPU_CHAR chAsciiCode = m_stCtl.pchKeyMap[uiScanCode_in];
	
	//CPUExt_DispPrint("Key Normal \n");
	//CPUExt_DispChar('T');
	chAsciiCode = m_achUSKeyMap[uiScanCode_in];
	CPUExt_DispChar(chAsciiCode);
	
	return;
}

CPU_PRIVATE  void cpu_key_ReceiverFunction(CPU_INT08U  uiScanCode_in)
{
	return;
}

