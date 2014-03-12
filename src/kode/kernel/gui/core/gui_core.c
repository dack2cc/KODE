/******************************************************************************
    Include
******************************************************************************/

#include <GUI.h>
#include <cpu_ext.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define GUI_CORE_BG_COLOR      (GUI_BLUE)
#define GUI_CORE_BG_COLOR_IDX  (0)

GUI_PRIVATE  LCD_COLOR  gui_core_auiColor[] = {
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

GUI_PRIVATE LCD_PHYSPALETTE gui_core_stPalette = {
	GUI_COUNTOF(gui_core_auiColor),
	gui_core_auiColor
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
	LCD_Init();
	LCD_SetLUT(&gui_core_stPalette);
	
	{
		CPU_INT08U i = 0;
		
		for (i = 0; i < GUI_COUNTOF(gui_core_auiColor); ++i) {
	        LCD_SetColorIndex(i);
	        LCD_FillRect(i * 10, 0, (i + 1) * 10, 10);
				
		}
		
	    LCD_Refresh();
	}
	
	return (0);
}

void  GUI_Exit(void)
{
	return;
}


