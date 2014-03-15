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
	//DRV_GFX_SHEET  stSheet;
	
	Mem_Clr(&gui_mice_stCtl, sizeof(gui_mice_stCtl));
	
	drv_gfx_GetLayerInfo(&(gui_mice_stCtl.stLayer));
	//drv_gfx_CreateSheet()
}


