/******************************************************************************
    Include
******************************************************************************/

#include <gui.h>
#include <gui_bg.h>
#include <gui_mice.h>
#include <drv_gfx.h>
//#include <drv_disp.h>
//#include <lib_mem.h>
//#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE 

//#define GUI_WINDOW_MAX   (DRV_GFX_SHEET_MAX)
//#define GUI_WINDOW_BG    (0)
//#define GUI_WINDOW_MOUSE (1)

//typedef struct _GUI_CONTROL {
//	DRV_GFX_LAYER  stLayer;
//	DRV_GFX_HANDLE ahWindow[GUI_WINDOW_MAX]
//} GUI_CONTROL;

//GUI_PRIVATE  GUI_CONTROL  gui_core_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/



/******************************************************************************
    Function Definition
******************************************************************************/

void gui_Init(void)
{
	gui_bg_Init();
	gui_mice_Init();
	
	drv_gfx_Refresh();
}


