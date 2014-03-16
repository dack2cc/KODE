/******************************************************************************
    Include
******************************************************************************/

#include <gui.h>
#include <gui_def.h>
#include <drv_gfx.h>
#include <drv_disp.h>
#include <lib_mem.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE 

typedef struct _GUI_BG_CONTROL {
	DRV_GFX_LAYER  stLayer;
	DRV_GFX_HANDLE hSheet;
} GUI_BT_CONTROL;

GUI_PRIVATE  GUI_BT_CONTROL  gui_bg_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

GUI_PRIVATE CPU_INT08U gui_bg_auiLogo[] = {
	__X_____, _____X__,
	__XX____, ____XX__,
	__XXX___, ___XXX__,
	__XXXX__, __XXXX__,
	__X_XXX_, _XXX_X__,
	__X__XXX, XXX__X__,
	__X_XXXX, XXXX_X__,
	__XXXXXX, XXXXXX__,
	_XXX__XX, XX__XXX_,
	XXX____X, X____XXX,
	XXXX__XX, XX__XXXX,
	XXXXXXXX, XXXXXXXX,
	__XXXXXX, XXXXXX__,
	___XX___, ___XX___,
	____XX__, __XX____,
	_____XXX, XXX_____,
};
#define GUI_BG_LOGO_MAX    (sizeof(gui_bg_auiLogo)/sizeof(CPU_INT08U))


/******************************************************************************
    Function Definition
******************************************************************************/

void gui_bg_Init(void)
{
	DRV_GFX_SHEET  stSheet;
	
	Mem_Clr(&gui_bg_stCtl, sizeof(gui_bg_stCtl));
	
	drv_gfx_GetLayerInfo(&(gui_bg_stCtl.stLayer));
	
	stSheet.x   = 0;
	stSheet.y   = 0;
	stSheet.w   = gui_bg_stCtl.stLayer.w;
	stSheet.h   = gui_bg_stCtl.stLayer.h;
	stSheet.z   = 0xFF;
	stSheet.v   = 1;
	stSheet.bpp = gui_bg_stCtl.stLayer.bpp;
	
	drv_gfx_CreateSheet(&stSheet, &(gui_bg_stCtl.hSheet));
}


