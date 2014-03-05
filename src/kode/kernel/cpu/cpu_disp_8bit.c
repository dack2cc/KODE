
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_disp_8bit.h>

#if (CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_8BIT)

#include <lib_mem.h>
#include <cpu_asm.h>
#include <font.h>

/******************************************************************************
    Private Definition
******************************************************************************/

#define CPU_DISP_MODE      (*((CPU_INT08U *)0x9000E))
#define CPU_DISP_WIDTH     (*((CPU_INT16U *)0x90010))
#define CPU_DISP_HEIGHT    (*((CPU_INT16U *)0x90012))
#define CPU_DISP_MEM_ADR   ((*(CPU_INT32U *)0x90014))

#define CPU_DISP_DESK_BAR_H (24)
#define CPU_DISP_DESK_BG    (CPU_EXT_COLOR_555555)

#define CPU_DISP_LOGO_Y     ((CPU_DISP_DESK_BAR_H - 16) / 2)
#define CPU_DISP_LOGO_X     (3)

#define CPU_DISP_TIME_X     (CPU_DISP_WIDTH - 5 * FONT_W - 3)
#define CPU_DISP_TIME_Y     (CPU_DISP_LOGO_Y)

CPU_PRIVATE const CPU_INT08U cpu_disp_auiLogoL[] = {
	0x20, 0x30, 0x38, 0x3C, 0x2E, 0x27, 0x2F, 0x3F, 
	0x33, 0x61, 0xF3, 0x7F, 0x30, 0x18, 0x0C, 0x07
};
CPU_PRIVATE const CPU_INT08U cpu_disp_auiLogoR[] = {
	0x04, 0x0C, 0x1C, 0x3C, 0x74, 0xE4, 0xF4, 0xFC,
	0xCC, 0x86, 0xCF, 0xFE, 0x0C, 0x18, 0x30, 0xE0
};


CPU_PRIVATE const CPU_INT08U cpu_disp_auiMouse[] = {
	0xFF, 0xFE, 0xFC, 0xFC, 0xFE, 0xFF, 0xEF, 0xC7, 
	0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

enum {
	CPU_EXT_COLOR_000000 = 0,
	CPU_EXT_COLOR_FF0000 = 1,
	CPU_EXT_COLOR_00FF00 = 2,
	CPU_EXT_COLOR_FFFF00 = 3,
	CPU_EXT_COLOR_0000FF = 4,
	CPU_EXT_COLOR_FF00FF = 5,
	CPU_EXT_COLOR_00FFFF = 6,
	CPU_EXT_COLOR_FFFFFF = 7,
	CPU_EXT_COLOR_C6C6C6 = 8,
	CPU_EXT_COLOR_840000 = 9,
	CPU_EXT_COLOR_008400 = 10,
	CPU_EXT_COLOR_848400 = 11,
	CPU_EXT_COLOR_000084 = 12,
	CPU_EXT_COLOR_840084 = 13,
	CPU_EXT_COLOR_008484 = 14,
	CPU_EXT_COLOR_848484 = 15,
	CPU_EXT_COLOR_555555 = 16,
	CPU_EXT_COLOR_AAAAAA = 17,
	CPU_EXT_COLOR_MAX
};

CPU_PRIVATE const CPU_INT08U cpu_disp_auiColorTbl[CPU_EXT_COLOR_MAX * 3] =
{
	0x00, 0x00, 0x00, /*  0 : black       */
	0xFF, 0x00, 0x00, /*  1 : red         */
	0x00, 0xFF, 0x00, /*  2 : green       */
	0xFF, 0xFF, 0x00, /*  3 : yellow      */
	0x00, 0x00, 0xFF, /*  4 : blue        */
	0xFF, 0x00, 0xFF, /*  5 : purple      */
	0x00, 0xFF, 0xFF, /*  6 : light blue  */
	0xFF, 0xFF, 0xFF, /*  7 : white       */
	0xC6, 0xC6, 0xC6, /*  8 : grey        */
	0x84, 0x00, 0x00, /*  9 : grey red    */
	0x00, 0x84, 0x00, /* 10 : grey green  */
	0x84, 0x84, 0x00, /* 11 : grey yellow */
	0x00, 0x00, 0x84, /* 12 : grey blue   */
	0x84, 0x00, 0x84, /* 13 : grey purple */
	0x00, 0x84, 0x84, /* 14 : xxxx blue   */
	0x84, 0x84, 0x84, /* 15 : grey grey   */
	0x55, 0x55, 0x55, /* 16 : grey white  */
	0xAA, 0xAA, 0xAA, /* 17 : light white */
};

typedef struct _CPU_DISP_CONTROL {
	CPU_INT08U  uiScrMode;
	CPU_INT16U  uiScrW;
	CPU_INT16U  uiScrH;
	
	CPU_INT32U  uiMemStart;
	CPU_INT32U  uiMemPitch;
	
	CPU_INT08U* pbyFont;
	CPU_INT16U  uiFontPitch;
	CPU_INT16U  uiFontW;
	CPU_INT16U  uiFontH;
	
	CPU_INT16U  uiTxtRowMax;
	CPU_INT16U  uiTxtColMax;
	CPU_INT16U  uiTxtStartX;
	CPU_INT16U  uiTxtStartY;
	
	CPU_INT16U  uiPosRow;
	CPU_INT16U  uiPosCol;
	CPU_INT16U  uiPosX;
	CPU_INT16U  uiPosY;
} CPU_DISP_CONTROL;

CPU_PRIVATE CPU_DISP_CONTROL cpu_disp_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE void cpu_disp_SetPalette(const CPU_INT32U iStart_in, const CPU_INT32U iEnd_in, const CPU_INT08U * pbyRGB_in);
CPU_PRIVATE void cpu_disp_Logo(void);
CPU_PRIVATE void cpu_disp_Time(void);
CPU_PRIVATE void cpu_disp_Desktop(void);
CPU_PRIVATE void cpu_disp_FillBox(
	const CPU_INT08U uiColorIdx_in, 
	const CPU_INT16U uiLeftTopX_in, 
	const CPU_INT16U uiLeftTopY_in,
	const CPU_INT16U uiRightBottomX_in,
	const CPU_INT16U uiRightBottomY_in
);
CPU_PRIVATE void cpu_disp_PutFont(
	const CPU_INT08U uiColorIdx_in,
	const CPU_INT16U uiPosX_in,
	const CPU_INT16U uiPosY_in,
	const CPU_INT08U * pbyFont_in
);

CPU_PRIVATE void cpu_disp_SetCode(const CPU_CHAR chCode_in);
CPU_PRIVATE void cpu_disp_LineFeed(void);
CPU_PRIVATE void cpu_disp_CarriageReturn(void);
CPU_PRIVATE void cpu_disp_ScrollUp(void);


/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_disp_Init(void)
{	
	cpu_disp_stCtl.uiScrMode   = CPU_DISP_MODE;
	cpu_disp_stCtl.uiScrW      = CPU_DISP_WIDTH;
	cpu_disp_stCtl.uiScrH      = CPU_DISP_HEIGHT;
	cpu_disp_stCtl.uiMemStart  = CPU_DISP_MEM_ADR;
	cpu_disp_stCtl.uiMemPitch  = CPU_DISP_WIDTH;
	
	cpu_disp_stCtl.pbyFont     = FONT_DATA;
	cpu_disp_stCtl.uiFontPitch = FONT_PITCH;
	cpu_disp_stCtl.uiFontW     = FONT_W;
	cpu_disp_stCtl.uiFontH     = FONT_H;
	
	cpu_disp_stCtl.uiTxtRowMax = cpu_disp_stCtl.uiScrH / cpu_disp_stCtl.uiFontH;
	cpu_disp_stCtl.uiTxtColMax = cpu_disp_stCtl.uiScrW / cpu_disp_stCtl.uiFontW;
	cpu_disp_stCtl.uiTxtStartX = 0;
	cpu_disp_stCtl.uiTxtStartY = CPU_DISP_DESK_BAR_H;
	
	cpu_disp_stCtl.uiPosCol    = 0;
	cpu_disp_stCtl.uiPosRow    = 0;
	cpu_disp_stCtl.uiPosX      = 0;
	cpu_disp_stCtl.uiPosY      = CPU_DISP_DESK_BAR_H;
	
	cpu_disp_SetPalette(0, (CPU_EXT_COLOR_MAX - 1), cpu_disp_auiColorTbl);	
	cpu_disp_Desktop();
	cpu_disp_Logo();
	cpu_disp_Time();
	
	cpu_disp_PutFont(CPU_EXT_COLOR_000000, 100, 2, cpu_disp_auiMouse);
}


CPU_INT32U  CPUExt_DispPrint(const CPU_CHAR* pszStr_in)
{
	CPU_CHAR*  pbyChar  = 0;
	CPU_INT32U iCharCnt = 0;
	
	if (0 == pszStr_in) {
		return (0);
	}
	
	pbyChar = (CPU_CHAR *)pszStr_in;
	while ('\0' != (*pbyChar)) {
		cpu_disp_SetCode((*pbyChar));
		++pbyChar;
		++iCharCnt;
	}
	
	return (iCharCnt);
}

void CPUExt_DispChar(const CPU_CHAR chAscii_in)
{
	cpu_disp_SetCode(chAscii_in);
}

CPU_PRIVATE void cpu_disp_SetPalette(const CPU_INT32U iStart_in, const CPU_INT32U iEnd_in, const CPU_INT08U * pbyRGB_in)
{
	CPU_SR_ALLOC();
	
	CPU_INT32U i = 0;
	
	CPU_CRITICAL_ENTER();
	
	_asm_outb(iStart_in, 0x03C8);
	for (i = iStart_in; i <= iEnd_in; ++i) {
		_asm_outb(pbyRGB_in[i * 3 + 0], 0x03C9);
		_asm_outb(pbyRGB_in[i * 3 + 1], 0x03C9);
		_asm_outb(pbyRGB_in[i * 3 + 2], 0x03C9);
	}

	CPU_CRITICAL_EXIT();
}

CPU_PRIVATE void cpu_disp_Logo(void)
{
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 0*8, CPU_DISP_LOGO_Y, cpu_disp_auiLogoL);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 1*8, CPU_DISP_LOGO_Y, cpu_disp_auiLogoR);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 2*8, CPU_DISP_LOGO_Y, cpu_disp_stCtl.pbyFont + 'k' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 3*8, CPU_DISP_LOGO_Y, cpu_disp_stCtl.pbyFont + 'o' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 4*8, CPU_DISP_LOGO_Y, cpu_disp_stCtl.pbyFont + 'd' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 5*8, CPU_DISP_LOGO_Y, cpu_disp_stCtl.pbyFont + 'e' * cpu_disp_stCtl.uiFontPitch);
	//cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 6*8, CPU_DISP_LOGO_Y, cpu_disp_stCtl.pbyFont + 'T' * cpu_disp_stCtl.uiFontPitch);
	//cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_LOGO_X + 7*8, CPU_DISP_LOGO_Y, cpu_disp_stCtl.pbyFont + 'O' * cpu_disp_stCtl.uiFontPitch);
}

CPU_PRIVATE void cpu_disp_Time(void)
{
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_TIME_X + 0*8, CPU_DISP_TIME_Y, cpu_disp_stCtl.pbyFont + '2' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_TIME_X + 1*8, CPU_DISP_TIME_Y, cpu_disp_stCtl.pbyFont + '3' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_TIME_X + 2*8, CPU_DISP_TIME_Y, cpu_disp_stCtl.pbyFont + ':' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_TIME_X + 3*8, CPU_DISP_TIME_Y, cpu_disp_stCtl.pbyFont + '5' * cpu_disp_stCtl.uiFontPitch);
	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, CPU_DISP_TIME_X + 4*8, CPU_DISP_TIME_Y, cpu_disp_stCtl.pbyFont + '5' * cpu_disp_stCtl.uiFontPitch);
}

CPU_PRIVATE void cpu_disp_Desktop(void)
{
	CPU_INT16U uiWidth = cpu_disp_stCtl.uiScrW;
	CPU_INT16U uiHeight = cpu_disp_stCtl.uiScrH;
	
	// background
	cpu_disp_FillBox(CPU_DISP_DESK_BG, 0, CPU_DISP_DESK_BAR_H, uiWidth - 1, uiHeight - 1);
	
	// desk bar
	cpu_disp_FillBox(CPU_EXT_COLOR_AAAAAA, 0, CPU_DISP_DESK_BAR_H - 1, uiWidth - 1, CPU_DISP_DESK_BAR_H - 1);
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, 0, CPU_DISP_DESK_BAR_H - 2, uiWidth - 1, CPU_DISP_DESK_BAR_H - 2);
	cpu_disp_FillBox(CPU_EXT_COLOR_AAAAAA, 0, 0, uiWidth - 1, CPU_DISP_DESK_BAR_H - 3);
	
	/* 
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, 3, uiHeight - 24, 59, uiHeight - 24);
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, 2, uiHeight - 24, 2, uiHeight - 4);
	cpu_disp_FillBox(CPU_EXT_COLOR_848484, 3, uiHeight - 4, 59, uiHeight - 4);
	cpu_disp_FillBox(CPU_EXT_COLOR_848484, 59, uiHeight - 23, 59, uiHeight - 5);
	cpu_disp_FillBox(CPU_EXT_COLOR_000000, 2, uiHeight - 3, 59, uiHeight - 3);
	cpu_disp_FillBox(CPU_EXT_COLOR_000000, 60, uiHeight - 24, 60, uiHeight - 3);
	*/

	/*
	cpu_disp_FillBox(CPU_EXT_COLOR_848484, uiWidth - 47, 20, uiWidth - 4, 20);
	cpu_disp_FillBox(CPU_EXT_COLOR_848484, uiWidth - 47, 4, uiWidth - 47, 20);
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, uiWidth - 47, 3, uiWidth - 4, 3);
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, uiWidth - 3, 3, uiWidth - 3, 20);
	*/
	
	/*
	cpu_disp_FillBox(CPU_EXT_COLOR_848484, uiWidth - 47, uiHeight - 24, uiWidth - 4, uiHeight - 24);
	cpu_disp_FillBox(CPU_EXT_COLOR_848484, uiWidth - 47, uiHeight - 23, uiWidth - 47, uiHeight - 4);
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, uiWidth - 47, uiHeight - 3, uiWidth - 4, uiHeight - 3);
	cpu_disp_FillBox(CPU_EXT_COLOR_FFFFFF, uiWidth - 3, uiHeight - 24, uiWidth - 3, uiHeight - 3);
	*/
}

CPU_PRIVATE void cpu_disp_FillBox(
	const CPU_INT08U uiColorIdx_in, 
	const CPU_INT16U uiLeftTopX_in, 
	const CPU_INT16U uiLeftTopY_in,
	const CPU_INT16U uiRightBottomX_in,
	const CPU_INT16U uiRightBottomY_in
)
{
	CPU_INT16U  x = 0;
	CPU_INT16U  y = 0;
	CPU_INT08U* pbyVRAM = (CPU_INT08U *)(cpu_disp_stCtl.uiMemStart);
	CPU_INT32U  uiPitch = cpu_disp_stCtl.uiMemPitch;
	
	for (y = uiLeftTopY_in; y <= uiRightBottomY_in; ++y) {
		for (x = uiLeftTopX_in; x <= uiRightBottomX_in; ++x) {
			pbyVRAM[y * uiPitch + x] = uiColorIdx_in;
		}
	}
}

CPU_PRIVATE void cpu_disp_PutFont(
	const CPU_INT08U uiColorIdx_in,
	const CPU_INT16U uiPosX_in,
	const CPU_INT16U uiPosY_in,
	const CPU_INT08U * pbyFont_in
)
{
	CPU_INT32U i = 0;
	CPU_INT08U * pbyVRAM = 0;
	CPU_INT08U uiFont;
	
	for (i = 0; i < 16; ++i) {
		pbyVRAM = (CPU_INT08U *)(cpu_disp_stCtl.uiMemStart + (uiPosY_in + i) * cpu_disp_stCtl.uiMemPitch + uiPosX_in);
		uiFont  = pbyFont_in[i];
		if (0 != (uiFont & 0x80)) {pbyVRAM[0] = uiColorIdx_in;}
		if (0 != (uiFont & 0x40)) {pbyVRAM[1] = uiColorIdx_in;}
		if (0 != (uiFont & 0x20)) {pbyVRAM[2] = uiColorIdx_in;}
		if (0 != (uiFont & 0x10)) {pbyVRAM[3] = uiColorIdx_in;}
		if (0 != (uiFont & 0x08)) {pbyVRAM[4] = uiColorIdx_in;}
		if (0 != (uiFont & 0x04)) {pbyVRAM[5] = uiColorIdx_in;}
		if (0 != (uiFont & 0x02)) {pbyVRAM[6] = uiColorIdx_in;}
		if (0 != (uiFont & 0x01)) {pbyVRAM[7] = uiColorIdx_in;}
	}
}

CPU_PRIVATE void cpu_disp_SetCode(const CPU_CHAR chCode_in)
{
	static CPU_INT08U  s_uiAsciiState = 0;
	
	switch (s_uiAsciiState) {
	case 0: 
	{
	    if ((chCode_in > 31) && (chCode_in < 127)) {
	    	if (cpu_disp_stCtl.uiPosCol >= cpu_disp_stCtl.uiTxtColMax) {
	    		cpu_disp_stCtl.uiPosCol = 0;
	    		cpu_disp_stCtl.uiPosX   = 0;
	    		cpu_disp_LineFeed();
	    	}
	    	
	    	cpu_disp_PutFont(CPU_EXT_COLOR_FFFFFF, cpu_disp_stCtl.uiPosX, cpu_disp_stCtl.uiPosY, cpu_disp_stCtl.pbyFont + chCode_in * cpu_disp_stCtl.uiFontPitch);
		
	        cpu_disp_stCtl.uiPosX   += cpu_disp_stCtl.uiFontW;
	        cpu_disp_stCtl.uiPosCol += 1;
	    }
		else if (7 == chCode_in) {
			// beep
		}
		else if (8 == chCode_in) {
			if (cpu_disp_stCtl.uiPosCol > 0) {
				cpu_disp_stCtl.uiPosCol -= 1;
				cpu_disp_stCtl.uiPosX   -= cpu_disp_stCtl.uiFontW;
			}
		}
		else if (9 == chCode_in) {
			cpu_disp_stCtl.uiPosCol += 8 - (cpu_disp_stCtl.uiPosCol&7);
			cpu_disp_stCtl.uiPosX   += (8 - (cpu_disp_stCtl.uiPosCol&7)) * cpu_disp_stCtl.uiFontW;
			if (cpu_disp_stCtl.uiPosCol > cpu_disp_stCtl.uiTxtColMax) {
				cpu_disp_stCtl.uiPosCol = 0;
				cpu_disp_stCtl.uiPosX   = 0;
				cpu_disp_LineFeed();
			}
		}
		else if ((10 == chCode_in) || (11 == chCode_in) || (12 == chCode_in)) {
			cpu_disp_LineFeed();
			//cpu_disp_CarriageReturn();
		}
		else if (13 == chCode_in) {
			cpu_disp_CarriageReturn();
		}
		else if (27 == chCode_in) {
			//s_uiAsciiState = 1;
		}
	    else {
		    // EMPTY
	    }
		break;
	}
	default:
	   break;
	} // end switch (s_uiAsciiState)
}

CPU_PRIVATE void cpu_disp_LineFeed(void)
{	
	if (cpu_disp_stCtl.uiPosRow + 1 < cpu_disp_stCtl.uiTxtRowMax - 1) {
		cpu_disp_stCtl.uiPosRow++;
		cpu_disp_stCtl.uiPosY += cpu_disp_stCtl.uiFontH;
		return;
	}
	cpu_disp_ScrollUp();
}

CPU_PRIVATE void cpu_disp_CarriageReturn(void)
{
	cpu_disp_stCtl.uiPosX   = 0;
	cpu_disp_stCtl.uiPosCol = 0;
}


CPU_PRIVATE void cpu_disp_ScrollUp(void)
{
	CPU_INT16U uiSrcY = 0;
	CPU_INT16U uiDstY = 0;
	CPU_INT32U uiRowSize = cpu_disp_stCtl.uiMemPitch * cpu_disp_stCtl.uiFontH;
	
	for (uiDstY = cpu_disp_stCtl.uiTxtStartY, uiSrcY = cpu_disp_stCtl.uiTxtStartY + cpu_disp_stCtl.uiFontH;
		 uiDstY < cpu_disp_stCtl.uiTxtStartY + (cpu_disp_stCtl.uiTxtRowMax - 2) * cpu_disp_stCtl.uiFontH;
		 uiDstY += cpu_disp_stCtl.uiFontH, uiSrcY += cpu_disp_stCtl.uiFontH)
	{
		Mem_Copy(
			(CPU_INT08U *)(cpu_disp_stCtl.uiMemStart + uiDstY * cpu_disp_stCtl.uiMemPitch),
			(CPU_INT08U *)(cpu_disp_stCtl.uiMemStart + uiSrcY * cpu_disp_stCtl.uiMemPitch),
			uiRowSize
		);
	}
	
	cpu_disp_FillBox(CPU_DISP_DESK_BG, 0, uiDstY, cpu_disp_stCtl.uiScrW - 1, cpu_disp_stCtl.uiScrH - 1);
}

#endif /* CPU_EXT_DISP_MODE == CPU_EXT_DISP_MODE_8BIT */

