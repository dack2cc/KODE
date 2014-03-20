/******************************************************************************
    Include
******************************************************************************/

#include <gui.h>
#include <gui_def.h>
#include <drv_gfx.h>
#include <drv_mice.h>
#include <drv_disp.h>
#include <lib_mem.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE 

#define GUI_MICE_X      (100)
#define GUI_MICE_Y      (0)
#define GUI_MICE_W      (8)
#define GUI_MICE_H      (9)
#define GUI_MICE_COLOR  (DRV_GFX_COLOR_BLACK)

GUI_PRIVATE CPU_INT08U gui_mice_abyArrow[] = {
	XXXXXXXX, 
	XXXXXXX_, 
	XXXXXX__, 
	XXXXXX__, 
	XXXXXXX_, 
	XXXXXXXX, 
	XXX_XXXX, 
	XX___XXX, 
	X_____XX, 
};
#define GUI_MICE_ARROW_MAX    (sizeof(gui_mice_abyArrow)/sizeof(CPU_INT08U))

GUI_PRIVATE CPU_INT08U gui_mice_abyCat[] = {
	___XX___, 
	XX_XX_XX, 
	XX_XX_XX, 
	X______X, 
	__XXXX__, 
	XXXXXXXX, 
	XXXXXXXX, 
	XXXXXXXX, 
	_XXXXXX_, 

};
#define GUI_MICE_CAT_MAX    (sizeof(gui_mice_abyCat)/sizeof(CPU_INT08U))


typedef struct _GUI_MICE_CONTROL {
	DRV_GFX_LAYER  stLayer;
	DRV_GFX_HANDLE hSheet;
	DRV_GFX_POINT  stPos;
} GUI_MICE_CONTROL;

GUI_PRIVATE  GUI_MICE_CONTROL  gui_mice_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/



/******************************************************************************
    Function Definition
******************************************************************************/

void gui_mice_Init(void)
{
	DRV_GFX_SHEET  stSheet;
	DRV_GFX_DATA   stData;
	
	Mem_Clr(&gui_mice_stCtl, sizeof(gui_mice_stCtl));

	drv_gfx_GetLayerInfo(&(gui_mice_stCtl.stLayer));
	
	gui_mice_stCtl.stPos.x = GUI_MICE_X;
	gui_mice_stCtl.stPos.y = GUI_MICE_Y;
	
	stSheet.x   = gui_mice_stCtl.stPos.x;
	stSheet.y   = gui_mice_stCtl.stPos.y;
	stSheet.w   = GUI_MICE_W;
	stSheet.h   = GUI_MICE_H;
	stSheet.z   = 0;
	stSheet.v   = 1;
	stSheet.bpp = gui_mice_stCtl.stLayer.bpp;

	drv_gfx_CreateSheet(&stSheet, &(gui_mice_stCtl.hSheet));
	
	stData.x    = 0;
	stData.y    = 0;
	stData.w    = stSheet.w;
	stData.h    = stSheet.h;
	stData.bpp  = 1;
	stData.data = gui_mice_abyArrow;
	//stData.data = gui_mice_abyCat;
	
	drv_gfx_SetColor(gui_mice_stCtl.hSheet, GUI_MICE_COLOR);
	drv_gfx_DrawData(gui_mice_stCtl.hSheet, &stData);
}

void gui_mice_Handler(const DRV_MICE_EVENT * pstEvt)
{	
	if (0 == pstEvt) {
		return;
	}
	
	gui_mice_stCtl.stPos.x += pstEvt->iOffsetX;
	if (gui_mice_stCtl.stPos.x < 0) {
		gui_mice_stCtl.stPos.x = 0;
	}
	else if (gui_mice_stCtl.stPos.x > gui_mice_stCtl.stLayer.w - 1) {
		gui_mice_stCtl.stPos.x = gui_mice_stCtl.stLayer.w - 1;
	}
	else {
		// EMPTY
	}
	
	gui_mice_stCtl.stPos.y += pstEvt->iOffsetY;
	if (gui_mice_stCtl.stPos.y < 0) {
		gui_mice_stCtl.stPos.y = 0;
	}
	else if (gui_mice_stCtl.stPos.y > gui_mice_stCtl.stLayer.h - 1) {
		gui_mice_stCtl.stPos.y = gui_mice_stCtl.stLayer.h - 1;
	}
	else {
		// EMPTY
	}

	drv_gfx_Move(gui_mice_stCtl.hSheet, &(gui_mice_stCtl.stPos));
}

