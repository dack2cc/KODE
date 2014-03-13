/******************************************************************************
    Include
******************************************************************************/

#include <LCD.h>
#include <cpu_ext.h>
#include <lib_pool.h>
#include <drv_disp.h>

/******************************************************************************
    Private Define
******************************************************************************/

typedef struct _GUI_LCD_LAYER {
	//CPU_INT32U   uiAlpha;
	//CPU_INT32U   uiAlphaMode;
	GUI_POINT    stPos;
	CPU_INT32U   uiSizeW;
	CPU_INT32U   uiSizeH;
	CPU_INT32U   uiVisOnOff;
	CPU_INT08U * pbyVRAM;
	CPU_INT32U   uiVSizeW;
	CPU_INT32U   uiVSizeH;
} GUI_LCD_LAYER;

typedef struct _GUI_LCD_CONTROL {
	CPU_INT08U     uiBitPerPixel;
	CPU_INT32U     uiWidth;
	CPU_INT32U     uiHeight;
	CPU_INT08U *   pbyBuf;
	CPU_SIZE_T     uiBufSize;
	CPU_SIZE_T     uiBufPitch;
	CPU_INT08U     uiDrawColorIdx;
	CPU_INT08U     uiBGColorIdx;
	LCD_RECT       stDirty;
	GUI_LCD_LAYER  astLayer[GUI_NUM_LAYERS];
} GUI_LCD_CONTROL;

GUI_PRIVATE  GUI_LCD_CONTROL  gui_lcd_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

GUI_PRIVATE void LCD_BitBlt(const GUI_LCD_LAYER * pstLayer_in);

/******************************************************************************
    Function Definition
******************************************************************************/

/*********************************************************************
*
*       Declarations for LCD_
*/
int LCD_Init(void)
{	
	Mem_Clr(&gui_lcd_stCtl, sizeof(gui_lcd_stCtl));
	
	CPUExt_DispBitPerPixel(&(gui_lcd_stCtl.uiBitPerPixel));
	CPUExt_DispResolution(&(gui_lcd_stCtl.uiWidth), &(gui_lcd_stCtl.uiHeight));
	
	gui_lcd_stCtl.uiBufPitch = gui_lcd_stCtl.uiWidth * (gui_lcd_stCtl.uiBitPerPixel / 8);
	gui_lcd_stCtl.uiBufSize = gui_lcd_stCtl.uiWidth * gui_lcd_stCtl.uiHeight * (gui_lcd_stCtl.uiBitPerPixel / 8);
	gui_lcd_stCtl.pbyBuf = (CPU_INT08U *)lib_pool_Malloc(gui_lcd_stCtl.uiBufSize);
	if (0 == gui_lcd_stCtl.pbyBuf) {
		CPUExt_CorePanic("[LCD_Init][Out Of Memory]");
	}
	drv_disp_Printf("[LCD_Init][BufAddr:0x%X]\r\n", gui_lcd_stCtl.pbyBuf);
	drv_disp_Printf("[LCD_Init][BufSize:%dKB]\r\n", gui_lcd_stCtl.uiBufSize / 1024);
	
	return (0);
}

void  LCD_InitLUT(void)
{
	
}

void LCD_SetLUT  (const LCD_PHYSPALETTE * pPalette)
{
	if ((pPalette) && (pPalette->pPalEntries)) {
	    CPUExt_DispSetPalette(pPalette->pPalEntries, pPalette->NumEntries, 0);
	}
}

int LCD_GetNumLayers(void)
{
	return (GUI_NUM_LAYERS);
}

void LCD_SetColorIndex(unsigned PixelIndex)
{
	gui_lcd_stCtl.uiDrawColorIdx = PixelIndex;
}

void LCD_SetBkColorIndex(unsigned PixelIndex)
{
	gui_lcd_stCtl.uiBGColorIdx = PixelIndex;
}

void LCD_FillRect(int x0, int y0, int x1, int y1)
{
	CPU_INT32U y = 0;
	CPU_INT32U uiBytesPerPixel = gui_lcd_stCtl.uiBitPerPixel / 8;
	CPU_INT32U uiPitch = gui_lcd_stCtl.uiBufPitch;
	
	if (x1 >= gui_lcd_stCtl.uiWidth) {
		x1 = gui_lcd_stCtl.uiWidth - 1;
	}
	if (y1 >= gui_lcd_stCtl.uiHeight) {
		y1 = gui_lcd_stCtl.uiHeight - 1;
	}
	
	if ((x0 < 0) || (x0 >= x1) || (y0 < 0) || (y0 >= y1)){
		return;
	}
	
	for (y = y0; y <= y1; ++y) {
		Mem_Set(
			(gui_lcd_stCtl.pbyBuf + y * uiPitch + x0),
			gui_lcd_stCtl.uiDrawColorIdx,
			(x1 - x0) * uiBytesPerPixel
		);
	}
	
	if (gui_lcd_stCtl.stDirty.x0 > x0) {
		gui_lcd_stCtl.stDirty.x0 = x0;
	}
	if (gui_lcd_stCtl.stDirty.x1 < x1) {
		gui_lcd_stCtl.stDirty.x1 = x1;
	}
	if (gui_lcd_stCtl.stDirty.y0 > y0) {
		gui_lcd_stCtl.stDirty.y0 = y0;
	}
	if (gui_lcd_stCtl.stDirty.y1 < y1) {
		gui_lcd_stCtl.stDirty.y1 = y1;
	}
}


/*********************************************************************
*
*       Display refresh (optional)
*/
int LCD_Refresh  (void)
{
	CPU_INT32U i = 0;
	
	for (i = 0; i < GUI_NUM_LAYERS; ++i) {
		if (0 != gui_lcd_stCtl.astLayer[i].uiVisOnOff) {
		    LCD_BitBlt(&(gui_lcd_stCtl.astLayer[i]));
		}
	}
	
	//drv_disp_Printf("[dirty][x0:%d]\r\n", gui_lcd_stCtl.stDirty.x0);
	//drv_disp_Printf("[dirty][y0:%d]\r\n", gui_lcd_stCtl.stDirty.y0);
	//drv_disp_Printf("[dirty][x1:%d]\r\n", gui_lcd_stCtl.stDirty.x1);
	//drv_disp_Printf("[dirty][y1:%d]\r\n", gui_lcd_stCtl.stDirty.y1);
	
	CPUExt_DispBitBlt(
		gui_lcd_stCtl.pbyBuf, 
		gui_lcd_stCtl.stDirty.x0, gui_lcd_stCtl.stDirty.y0, 
		gui_lcd_stCtl.stDirty.x1, gui_lcd_stCtl.stDirty.y1
	);
	
	Mem_Clr(&(gui_lcd_stCtl.stDirty), sizeof(gui_lcd_stCtl.stDirty));
	
	return (0);
}

int LCD_RefreshEx(int LayerIndex)
{
	return (LCD_ERR0);
}

int LCD_GetXSizeMax(void)
{
	return (gui_lcd_stCtl.uiWidth);
}

int LCD_GetYSizeMax(void)
{
	return (gui_lcd_stCtl.uiHeight);
}

int LCD_GetVXSizeMax(void)
{
	return (gui_lcd_stCtl.stDirty.x1 - gui_lcd_stCtl.stDirty.x0);
}

int LCD_GetVYSizeMax(void)
{
	return (gui_lcd_stCtl.stDirty.y1 - gui_lcd_stCtl.stDirty.y0);
}

int LCD_GetBitsPerPixelMax(void)
{
	return (gui_lcd_stCtl.uiBitPerPixel);
}

unsigned int LCD_GetPixelIndex(int x, int y)
{
	if ((0 == gui_lcd_stCtl.pbyBuf) 
	||  (x < 0) || (x >= gui_lcd_stCtl.uiWidth)
	||  (y < 0) || (y >= gui_lcd_stCtl.uiHeight)) {
		return (0);
	}
	
	return (*(gui_lcd_stCtl.pbyBuf + y * gui_lcd_stCtl.uiBufPitch + x));
}

int LCD_GetBkColorIndex (void)
{
	return (gui_lcd_stCtl.uiBGColorIdx);
}

int LCD_GetColorIndex (void)
{
	return (gui_lcd_stCtl.uiDrawColorIdx);
}

GUI_PRIVATE void LCD_BitBlt(const GUI_LCD_LAYER * pstLayer_in)
{
	CPU_INT32U i = 0;
	CPU_INT32U uiSrcX = 0;
	CPU_INT32U uiSrcY = 0;
	CPU_INT32U uiDstX = 0;
	CPU_INT32U uiDstY = 0;
	CPU_INT32U uiRegW = 0;
	CPU_INT32U uiRegH = 0;
	CPU_INT08U * pbyDst   = gui_lcd_stCtl.pbyBuf;
	CPU_INT32U uiDstPitch = gui_lcd_stCtl.uiBufPitch;
	CPU_INT08U * pbySrc   = 0;
	CPU_INT32U uiSrcPitch = 0;
	
	if ((0 == pstLayer_in) 
	||  (0 == pstLayer_in->pbyVRAM)
	||  (0 == pstLayer_in->uiVisOnOff)
	||  (0 == pstLayer_in->uiSizeW)
	||  (0 == pstLayer_in->uiSizeH)
	||  (0 == pstLayer_in->uiVSizeW)
	||  (0 == pstLayer_in->uiVSizeH)
	||  (pstLayer_in->stPos.x >= gui_lcd_stCtl.uiWidth)
	||  (pstLayer_in->stPos.y >= gui_lcd_stCtl.uiHeight)
	||  (pstLayer_in->stPos.x + pstLayer_in->uiVSizeW < 0)
	||  (pstLayer_in->stPos.y + pstLayer_in->uiVSizeH < 0)) {
		return;
	}
	
	pbySrc     = pstLayer_in->pbyVRAM;
	uiSrcPitch = pstLayer_in->uiSizeW;
	
	if (pstLayer_in->stPos.x < 0) {
		uiSrcX = -(pstLayer_in->stPos.x);
		uiDstX = 0;
		uiRegH = pstLayer_in->uiVSizeW + pstLayer_in->stPos.x;
	}
	else {
		uiSrcX = 0;
		uiDstX = pstLayer_in->stPos.x;
		uiRegW = pstLayer_in->uiVSizeW;
	}
	
	if (pstLayer_in->stPos.y < 0) {
		uiSrcY = -(pstLayer_in->stPos.y);
		uiDstY = 0;
		uiRegH = pstLayer_in->uiVSizeH + pstLayer_in->stPos.y;
	}
	else {
		uiSrcY = 0;
		uiDstY = pstLayer_in->stPos.y;
		uiRegH = pstLayer_in->uiVSizeH;
	}
	
	for (i = 0; i < uiRegH; ++i) {
		Mem_Copy(
			pbyDst + (uiDstY + i) * uiDstPitch + uiDstX,
			pbySrc + (uiSrcY + i) * uiSrcPitch + uiSrcX,
			uiRegW
		);
	}
}


