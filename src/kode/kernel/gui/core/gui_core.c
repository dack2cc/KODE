/******************************************************************************
    Include
******************************************************************************/

#include <GUI.h>
#include <cpu_ext.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define GUI_VERSION_STRINT  "kokoto-v4"

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

GUI_PRIVATE GUI_CONTEXT gui_core_stCtx;

/******************************************************************************
    Private Interface
******************************************************************************/

GUI_PRIVATE GUI_COLOR gui_core_GetColorBy(const CPU_INT08U uiIdx_in);
GUI_PRIVATE int       gui_core_GetColorIndexBy(const CPU_INT08U uiIdx_in);
GUI_PRIVATE void      gui_core_SetColorBy(const CPU_INT08U uiIdx_in, GUI_COLOR color);
GUI_PRIVATE void      gui_core_SetColorIndexBy(const CPU_INT08U uiIdx_in, int Index);

/******************************************************************************
    Function Definition
******************************************************************************/

/*********************************************************************
*
*       General routines
*/
int  
GUI_Init(void)
{
	GUI_MEMSET(&(gui_core_stCtx), 0x00, sizeof(gui_core_stCtx));
	
	LCD_Init();
	LCD_SetLUT(&gui_core_stPalette);
	
	/*
	{
		CPU_INT08U i = 0;
		for (i = 0; i < GUI_COUNTOF(gui_core_auiColor); ++i) {
	        LCD_SetColorIndex(i);
	        LCD_FillRect(i * 10, 0, (i + 1) * 10, 10);
				
		}
	    LCD_Refresh();
	}
	*/
	
	return (0);
}

void  
GUI_Exit(void)
{
	return;
}

void
GUI_SetDefaultFont   (const GUI_FONT GUI_UNI_PTR * pFont)
{
	gui_core_stCtx.pAFont = pFont;
	return;
}

void         
GUI_SetDefault       (void)
{
	return;
}

GUI_DRAWMODE 
GUI_SetDrawMode      (GUI_DRAWMODE dm)
{
	gui_core_stCtx.DrawMode = dm;
	return (dm);
}

const char * 
GUI_GetVersionString (void)
{
	return GUI_VERSION_STRINT;
}

void         
GUI_SaveContext      (      GUI_CONTEXT * pContext)
{
	if (0 != pContext) {
		(*pContext) = gui_core_stCtx;
	}
}

void         
GUI_RestoreContext   (const GUI_CONTEXT * pContext)
{
	if (0 != pContext) {
		gui_core_stCtx = (*pContext);
	}
}

void         
GUI_SetScreenSizeX   (int xSize)
{
	return;
}

void         
GUI_SetScreenSizeY   (int ySize)
{
	return;
}

int          
GUI_GetScreenSizeX   (void)
{
	return (LCD_GetXSizeMax());
}

int          
GUI_GetScreenSizeY   (void)
{
	return (LCD_GetYSizeMax());
}

const GUI_RECT * 
GUI_SetClipRect  (const GUI_RECT * pRect)
{
	return (0);
}


/*********************************************************************
*
*       Get / Set Attributes
*/
GUI_COLOR 
GUI_GetBkColor(void)
{
	return (gui_core_GetColorBy(0));
}

int       
GUI_GetBkColorIndex(void)
{
	return (gui_core_GetColorIndexBy(0));
}

GUI_COLOR 
GUI_GetColor(void)
{
	return (gui_core_GetColorBy(1));
}

int       
GUI_GetColorIndex(void)
{
	return (gui_core_GetColorIndexBy(1));
}

U8        
GUI_GetLineStyle(void)
{
	return (gui_core_stCtx.LineStyle);
}

U8        
GUI_GetPenSize(void)
{
	return (gui_core_stCtx.PenSize);
}

U8        
GUI_GetPenShape(void)
{
	return (gui_core_stCtx.PenShape);
}

unsigned  
GUI_GetPixelIndex(int x, int y)
{
	return (0);
}

void      
GUI_SetBkColor(GUI_COLOR color)
{
	gui_core_SetColorBy(0, color);
}

void      
GUI_SetColor(GUI_COLOR color)
{
	gui_core_SetColorBy(1, color);
}

void      
GUI_SetBkColorIndex(int Index)
{
	gui_core_SetColorIndexBy(0, Index);
}

void      
GUI_SetColorIndex(int Index)
{
	gui_core_SetColorIndexBy(1, Index);
}

U8        
GUI_SetPenSize(U8 Size)
{
	gui_core_stCtx.PenSize = Size;
	return (Size);
}

U8        
GUI_SetPenShape(U8 Shape)
{
	gui_core_stCtx.PenShape = Shape;
	return (Shape);
}

U8        
GUI_SetLineStyle(U8 Style)
{
	gui_core_stCtx.LineStyle = Style;
	return (Style);
}

/*********************************************************************
*
*       Color / Index related functions
*/
int       
GUI_Color2Index(GUI_COLOR color)
{
	int i = 0;
	
	for (i = 0; i < GUI_COUNTOF(gui_core_auiColor); ++i) {
		if (color == gui_core_auiColor[i]) {
			return (i);
		}
	}
	
	return (-1);
}

GUI_COLOR 
GUI_Color2VisColor(GUI_COLOR color)
{
	return (color);
}

char     
GUI_ColorIsAvailable(GUI_COLOR color)
{
	int  iBitsPerPixel = LCD_GetBitsPerPixelMax();
	char iIsAvailable  = 1;
	int  iColorIdx = 0;
	
	switch (iBitsPerPixel) {
	case 8:
		iColorIdx = GUI_Color2Index(color);
		if (iColorIdx < 0) {
			iIsAvailable = 0;
		}
		break;
	case 24:
		if (0xFF000000 & color) {
			iIsAvailable = 0;
		}
		break;
	case 32:
		iIsAvailable = 1;
		break;
	default:
		// EXCEPTION
		iIsAvailable = 0;
		break;
	}
	
	return (iIsAvailable);

}

GUI_COLOR 
GUI_Index2Color(int Index)
{
	if ((Index < 0) 
	||  (Index >= GUI_COUNTOF(gui_core_auiColor))) {
		return (-1);
	}
	
	return (gui_core_auiColor[Index]);
}

U32       
GUI_CalcColorDist (GUI_COLOR Color0, GUI_COLOR  Color1)
{
	return (0);
}

U32       
GUI_CalcVisColorError(GUI_COLOR color)
{
	return (0);
}

GUI_PRIVATE GUI_COLOR 
gui_core_GetColorBy(const CPU_INT08U uiIdx_in)
{
	int  iBitsPerPixel = LCD_GetBitsPerPixelMax();
	GUI_COLOR  color = -1;
	
	if (uiIdx_in > 1) {
		return (color);
	}
	
	switch (iBitsPerPixel) {
	case 8:
		color = GUI_Index2Color(gui_core_stCtx.LCD.aColorIndex8[uiIdx_in]);
		break;
	case 24:
		color = GUI_Index2Color(gui_core_stCtx.LCD.aColorIndex16[uiIdx_in]);
		break;
	case 32:
		color = GUI_Index2Color(gui_core_stCtx.LCD.aColorIndex32[uiIdx_in]);
		break;
	default:
		// EXCEPTION
		break;
	}
	
	return (color);
}

GUI_PRIVATE int       
gui_core_GetColorIndexBy(const CPU_INT08U uiIdx_in)
{
	int  iBitsPerPixel = LCD_GetBitsPerPixelMax();
	int  index = -1;
	
	switch (iBitsPerPixel) {
	case 8:
		index = gui_core_stCtx.LCD.aColorIndex8[0];
		break;
	case 24:
		index = GUI_Color2Index(gui_core_stCtx.LCD.aColorIndex16[0]);
		break;
	case 32:
		index = GUI_Color2Index(gui_core_stCtx.LCD.aColorIndex32[0]);
		break;
	default:
		// EXCEPTION
		break;
	}
	
	return (index);
}

GUI_PRIVATE void      
gui_core_SetColorBy(const CPU_INT08U uiIdx_in, GUI_COLOR color)
{
	int  iBitsPerPixel = LCD_GetBitsPerPixelMax();
	int  iIndex = 0;
	
	if (uiIdx_in > 1) {
		return;
	}
	
	switch (iBitsPerPixel) {
	case 8:
		iIndex = GUI_Color2Index(color);
		if (iIndex < 0) {
			iIndex = 0;
		}
		gui_core_stCtx.LCD.aColorIndex8[uiIdx_in] = iIndex;
		break;
	case 24:
		gui_core_stCtx.LCD.aColorIndex16[uiIdx_in] = color;
		break;
	case 32:
		gui_core_stCtx.LCD.aColorIndex32[uiIdx_in] = color;
		break;
	default:
		// EXCEPTION
		break;
	}
}

GUI_PRIVATE void      
gui_core_SetColorIndexBy(const CPU_INT08U uiIdx_in, int Index)
{
	int  iBitsPerPixel = LCD_GetBitsPerPixelMax();
	
	if (uiIdx_in > 1) {
		return;
	}
	
	switch (iBitsPerPixel) {
	case 8:
		if (Index < 0) {
			Index = 0;
		}
		gui_core_stCtx.LCD.aColorIndex8[1] = Index;
		break;
	case 24:
		gui_core_stCtx.LCD.aColorIndex16[1] = GUI_Index2Color(Index);
		break;
	case 32:
		gui_core_stCtx.LCD.aColorIndex32[1] = GUI_Index2Color(Index);
		break;
	default:
		// EXCEPTION
		break;
	}
}


