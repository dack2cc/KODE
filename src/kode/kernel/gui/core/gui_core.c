/******************************************************************************
    Include
******************************************************************************/

#include <GUI.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define GUI_CORE_BG_COLOR      (GUI_BLUE)
#define GUI_CORE_BG_COLOR_IDX  (0)

GUI_PRIVATE  CPU_INT32U  gui_core_auiColor[] = {
	GUI_BLUE,
	GUI_GREEN,
	GUI_RED,
	GUI_CYAN,
	GUI_MAGENTA,
	GUI_YELLOW,
	GUI_LIGHTBLUE,
	GUI_LIGHTGREEN,
	GUI_LIGHTRED,
	GUI_LIGHTCYAN,
	GUI_LIGHTMAGENTA,
	GUI_LIGHTYELLOW,
	GUI_DARKBLUE,
	GUI_DARKGREEN,
	GUI_DARKRED,
	GUI_DARKCYAN,
	GUI_DARKMAGENTA,
	GUI_DARKYELLOW,
	GUI_WHITE,
	GUI_LIGHTGRAY,
	GUI_GRAY,
	GUI_DARKGRAY,
	GUI_BLACK,
	GUI_BROWN,
	GUI_ORANGE,
	GUI_TRANSPARENT,
	GUI_INVALID_COLOR
};

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/


/*********************************************************************
*
*       General routines
*/
int  GUI_Init(void)
{
	CPUExt_DispSetPalette(gui_core_auiColor, GUI_COUNTOF(gui_core_auiColor), 0);
	
	return (0);
}

void  GUI_Exit(void)
{
	return;
}


