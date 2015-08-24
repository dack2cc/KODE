#ifndef __GUI_WIN_H__
#define __GUI_WIN_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <drv_gfx.h>

/******************************************************************************
    Public Definition
******************************************************************************/

enum {
    GUI_WIN_STYLE_DEFAULT,
    GUI_WIN_STYLE_MAX
};

/******************************************************************************
    Public Interface
******************************************************************************/

extern void gui_win_Init( const DRV_GFX_HANDLE hSheet, const CPU_INT08U uiStyle_in);
extern void gui_win_Title(const DRV_GFX_HANDLE hSheet, const CPU_INT08U uiStyle_in, const CPU_INT08S iIsShow_in, const CPU_CHAR * pszTitle_in);
extern void gui_win_Close(const DRV_GFX_HANDLE hSheet, const CPU_INT08U uiStyle_in, const CPU_INT08S iIsShow_in);
extern void gui_win_StartPoint(const CPU_INT08U uiStyle_in, DRV_GFX_POINT * pstPos_out);
extern void gui_win_Background(const CPU_INT08U uiStyle_in, DRV_GFX_COLOR * pColor_out);

#endif // __GUI_WIN_H__

