/******************************************************************************
    Include
******************************************************************************/

#include <gui_log.h>

#if (CPU_EXT_DISP_MODE_TEXT != CPU_EXT_DISP_MODE)

#include <gui_def.h>
#include <gui_win.h>
#include <drv_gfx.h>
#include <drv_disp.h>
#include <font.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE

#define GUI_LOG_TITLE       "Log"
#define GUI_LOG_COLOR_FONT  DRV_GFX_COLOR_BLACK

typedef struct _GUI_LOG_CONTROL {
    CPU_INT08S     iIsOpen;
    DRV_GFX_HANDLE hSheet;
    DRV_GFX_POINT  stStart;
    DRV_GFX_COLOR  colorBG;
} GUI_LOG_CONTROL;

GUI_PRIVATE GUI_LOG_CONTROL gui_log_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void gui_log_Init()
{
    gui_log_stCtl.iIsOpen = 0;
}


void gui_log_Open(const DRV_GFX_POINT * pstPos_in)
{
    DRV_GFX_SHEET stSheet;
    DRV_GFX_FONT  stFont;

    if (0 == pstPos_in) {
        return;
    }

    stSheet.x   = pstPos_in->x;
    stSheet.y   = pstPos_in->y;
    stSheet.w   = 16 * 21;
    stSheet.h   = 16 * 21;
    stSheet.z   = GUI_Z_ORDER_LOG;
    stSheet.v   = 1;
    stSheet.bpp = 8;

    drv_gfx_CreateSheet(&stSheet, &(gui_log_stCtl.hSheet));

    gui_win_Init( gui_log_stCtl.hSheet, GUI_WIN_STYLE_DEFAULT);
    gui_win_Title(gui_log_stCtl.hSheet, GUI_WIN_STYLE_DEFAULT, 1, GUI_LOG_TITLE);
    gui_win_Close(gui_log_stCtl.hSheet, GUI_WIN_STYLE_DEFAULT, 1);
    gui_win_StartPoint(GUI_WIN_STYLE_DEFAULT, &(gui_log_stCtl.stStart));
    gui_win_Background(GUI_WIN_STYLE_DEFAULT, &(gui_log_stCtl.colorBG));

    stFont.w    = 8;
    stFont.h    = 16;
    stFont.data = font_hankaku;
    stFont.bpc  = 16;
    stFont.bg   = gui_log_stCtl.colorBG;

    drv_gfx_SetFont( gui_log_stCtl.hSheet, &stFont);
    drv_gfx_SetColor(gui_log_stCtl.hSheet, GUI_LOG_COLOR_FONT);

    gui_log_stCtl.iIsOpen = 1;
}


void gui_log_Close(void)
{
}

void gui_log_Update(void)
{
    CPU_CHAR * pszLog = 0;
    CPU_INT32U uiLen  = 0;

    if (0 == gui_log_stCtl.iIsOpen) {
        return;
    }

    CPUExt_DispLog(&pszLog, &uiLen);

    if ((0 == pszLog)
            ||  (0 == uiLen)) {
        return;
    }

    if ('\0' != pszLog[uiLen - 1]) {
        //pszLog[uiLen - 1] = 0;
    }

    drv_gfx_DrawStr(gui_log_stCtl.hSheet, &(gui_log_stCtl.stStart), pszLog);
}

#endif // (CPU_EXT_DISP_MODE_TEXT == CPU_EXT_DISP_MODE)

