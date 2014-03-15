/******************************************************************************
    Include
******************************************************************************/

#include <gui.h>
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


