/******************************************************************************
    Include
******************************************************************************/

#include <gui_win.h>

#if (CPU_EXT_DISP_MODE_TEXT != CPU_EXT_DISP_MODE)

#include <gui_def.h>
#include <drv_gfx.h>
#include <drv_disp.h>
#include <font.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE 

#define GUI_WIN_DEFAULT_COLOR_BG  (DRV_GFX_COLOR_WHITE)
#define GUI_WIN_DEFAULT_COLOR_BAR (DRV_GFX_COLOR_CYAN)
#define GUI_WIN_DEFAULT_BAR_X     (0)
#define GUI_WIN_DEFAULT_BAR_Y     (0)
#define GUI_WIN_DEFAULT_BAR_H     (GUI_WIN_DEFAULT_CLOSE_H + 4)
#define GUI_WIN_DEFAULT_CLOSE_X   (2)
#define GUI_WIN_DEFAULT_CLOSE_Y   (2)
#define GUI_WIN_DEFAULT_CLOSE_W   (16)
#define GUI_WIN_DEFAULT_CLOSE_H   (16)
#define GUI_WIN_DEFAULT_TITLE_X   (GUI_WIN_DEFAULT_CLOSE_W + GUI_WIN_DEFAULT_CLOSE_X + 2)
#define GUI_WIN_DEFAULT_TITLE_Y   (GUI_WIN_DEFAULT_CLOSE_Y)

GUI_PRIVATE CPU_INT08U gui_win_abyClose[] = {
	_____XXX, XXX_____,
	____X___, ___X____,
	___X____, ____X___,
	__X_____, _____X__,
	_X__X___, ___X__X_,
	X____X__, __X____X,
	X_____X_, _X_____X,
	X______X, X______X,
	X______X, X______X,
	X_____X_, _X_____X,
	X____X__, __X____X,
	_X__X___, ___X__X_,
	__X_____, _____X__,
	___X____, ____X___,
	____X___, ___X____,
	_____XXX, XXX_____,
};
#define GUI_WIN_CLOSE_MAX    (sizeof(gui_win_abyClose)/sizeof(CPU_INT08U))

typedef struct {
	DRV_GFX_COLOR  bg_color;
	DRV_GFX_COLOR  bar_color;
	CPU_INT32S     bar_x;
	CPU_INT32S     bar_y;
	CPU_INT32U     bar_h;
	CPU_INT32S     close_x;
	CPU_INT32S     close_y;
	CPU_INT32U     close_h;
	CPU_INT32U     close_w;
	CPU_INT08U *   close;
	CPU_INT32S     title_x;
	CPU_INT32S     title_y;
} _GUI_WIN_STYLE;

GUI_PRIVATE _GUI_WIN_STYLE  gui_win_astStyle[GUI_WIN_STYLE_MAX] = {
	{
		GUI_WIN_DEFAULT_COLOR_BG, GUI_WIN_DEFAULT_COLOR_BAR, 
	    GUI_WIN_DEFAULT_BAR_X,    GUI_WIN_DEFAULT_BAR_Y,   GUI_WIN_DEFAULT_BAR_H, 
		GUI_WIN_DEFAULT_CLOSE_X,  GUI_WIN_DEFAULT_CLOSE_Y, GUI_WIN_DEFAULT_CLOSE_W, GUI_WIN_DEFAULT_CLOSE_H, gui_win_abyClose,
		GUI_WIN_DEFAULT_TITLE_X,  GUI_WIN_DEFAULT_TITLE_Y
	},
};

typedef struct _GUI_WIN_CONTROL {
	CPU_INT08U  uiStyle;
} GUI_WIN_CONTROL;

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void gui_win_Init( const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiStyle_in)
{	
	DRV_GFX_SHEET    stSheet;
	DRV_GFX_RECT     stRect;
	_GUI_WIN_STYLE * pstStyle = 0;
	
	if (uiStyle_in >= GUI_WIN_STYLE_MAX) {
		return;
	}
	pstStyle = gui_win_astStyle + uiStyle_in;
	
	drv_gfx_GetSheet(hSheet_in, &stSheet);
	
	stRect.x = 0;
	stRect.y = 0;
	stRect.w = stSheet.w;
	stRect.h = stSheet.h;
	drv_gfx_SetColor(hSheet_in, pstStyle->bg_color);
	drv_gfx_FillRect(hSheet_in, &stRect);
	
	stRect.x = 1;
	stRect.y = 1;
	stRect.w = stSheet.w - 2;
	stRect.h = pstStyle->bar_h - 2;
	drv_gfx_SetColor(hSheet_in, pstStyle->bar_color);
	drv_gfx_FillRect(hSheet_in, &stRect);	
	
	return;
}

void gui_win_Title(const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiStyle_in, const CPU_INT08S iIsShow_in, const CPU_CHAR * pszTitle_in)
{
	DRV_GFX_FONT     stFont;
	DRV_GFX_POINT    stPos;
	_GUI_WIN_STYLE * pstStyle = 0;
	
	if ((iIsShow_in > 0) && (0 == pszTitle_in)) {
		return;
	}
	if (uiStyle_in >= GUI_WIN_STYLE_MAX) {
		return;
	}
	pstStyle = gui_win_astStyle + uiStyle_in;
	
	stFont.w    = 8;
	stFont.h    = 16;
	stFont.data = font_hankaku;
	stFont.bpc  = 16;
	stFont.bg   = pstStyle->bar_color;
	
	stPos.x = pstStyle->title_x;
	stPos.y = pstStyle->title_y;
	
	drv_gfx_SetFont(hSheet_in, &stFont);
	drv_gfx_SetColor(hSheet_in, pstStyle->bg_color);
	drv_gfx_DrawStr(hSheet_in, &stPos, pszTitle_in);
}

void gui_win_Close(const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiStyle_in, const CPU_INT08S iIsShow_in)
{
	DRV_GFX_DATA     stData;
	_GUI_WIN_STYLE * pstStyle = 0;

	if (uiStyle_in >= GUI_WIN_STYLE_MAX) {
		return;
	}
	pstStyle = gui_win_astStyle + uiStyle_in;
	
	stData.x    = pstStyle->close_x;
	stData.y    = pstStyle->close_y;
	stData.w    = pstStyle->close_w;
	stData.h    = pstStyle->close_h;
	stData.bpp  = 1;
	stData.data = pstStyle->close;
	
	drv_gfx_SetColor(hSheet_in, pstStyle->bg_color);
	drv_gfx_DrawData(hSheet_in, &stData);

}

void gui_win_StartPoint(const CPU_INT08U uiStyle_in, DRV_GFX_POINT * pstPos_out)
{
	_GUI_WIN_STYLE * pstStyle = 0;

	if (0 == pstPos_out) {
		return;
	}
	if (uiStyle_in >= GUI_WIN_STYLE_MAX) {
		return;
	}
	pstStyle = gui_win_astStyle + uiStyle_in;
	
	pstPos_out->x = 0;
	pstPos_out->y = pstStyle->bar_h;
}

void gui_win_Background(const CPU_INT08U uiStyle_in, DRV_GFX_COLOR * pColor_out)
{
	_GUI_WIN_STYLE * pstStyle = 0;

	if (0 == pColor_out) {
		return;
	}
	if (uiStyle_in >= GUI_WIN_STYLE_MAX) {
		return;
	}
	pstStyle = gui_win_astStyle + uiStyle_in;
	
	(*pColor_out) = pstStyle->bg_color;
}

#endif // (CPU_EXT_DISP_MODE_TEXT != CPU_EXT_DISP_MODE)

