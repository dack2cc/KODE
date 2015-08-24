/******************************************************************************
    Include
******************************************************************************/

#include <gui.h>

#if (CPU_EXT_DISP_MODE != CPU_EXT_DISP_MODE_TEXT)

#include <gui_def.h>
#include <gui_log.h>
#include <drv_gfx.h>
#include <drv_disp.h>
#include <lib_mem.h>
#include <lib_pool.h>
#include <font.h>
#include <std/stdio.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define GUI_PRIVATE static
#define GUI_PRIVATE

#define GUI_BG_COLOR_DESK      DRV_GFX_COLOR_BLUE
#define GUI_BG_COLOR_BAR       DRV_GFX_COLOR_LIGHTGRAY
#define GUI_BG_COLOR_BAR_EDGE  DRV_GFX_COLOR_WHITE
#define GUI_BG_COLOR_LOGO      DRV_GFX_COLOR_WHITE
#define GUI_BG_COLOR_NAME      GUI_BG_COLOR_LOGO
#define GUI_BG_COLOR_NAME_BG   GUI_BG_COLOR_BAR
#define GUI_BG_COLOR_TIME      GUI_BG_COLOR_LOGO
#define GUI_BG_COLOR_TIME_BG   GUI_BG_COLOR_BAR
#define GUI_BG_BAR_H           (24)
#define GUI_BG_LOGO_X          (3)
#define GUI_BG_LOGO_Y          ((GUI_BG_BAR_H - 16) / 2)
#define GUI_BG_LOGO_W          (16)
#define GUI_BG_LOGO_H          (16)
#define GUI_BG_NAME_X          (GUI_BG_LOGO_X + GUI_BG_LOGO_W + 3)
#define GUI_BG_NAME_Y          (GUI_BG_LOGO_Y)
#define GUI_BG_NAME            "kode"
#define GUI_BG_TIME_Y          (GUI_BG_LOGO_Y)


GUI_PRIVATE CPU_INT08U gui_bg_abyLogo[] = {
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
    _XXXXXXX, XXXXXXX_,
    __XXX___, ___XXX__,
    ___XXX__, __XXX___,
    ____XXXX, XXXX____,
};
#define GUI_BG_LOGO_MAX    (sizeof(gui_bg_abyLogo)/sizeof(CPU_INT08U))

typedef struct _GUI_BG_TIME {
    CPU_INT32U h;
    CPU_INT32U m;
    CPU_CHAR   buf[6];
} GUI_BG_TIME;

typedef struct _GUI_BG_CONTROL {
    DRV_GFX_LAYER  stLayer;
    DRV_GFX_HANDLE hSheet;
    GUI_BG_TIME    stTime;
} GUI_BG_CONTROL;

GUI_PRIVATE  GUI_BG_CONTROL  gui_bg_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

GUI_PRIVATE void gui_bg_Desktop(void);
GUI_PRIVATE void gui_bg_Logo(void);
GUI_PRIVATE void gui_bg_Name(void);
GUI_PRIVATE void gui_bg_Time(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void gui_bg_Init(void)
{
    DRV_GFX_SHEET  stSheet;

    Mem_Clr(&gui_bg_stCtl, sizeof(gui_bg_stCtl));

    gui_bg_stCtl.stTime.h = 23;
    gui_bg_stCtl.stTime.m = 54;

    drv_gfx_GetLayerInfo(&(gui_bg_stCtl.stLayer));

    stSheet.x   = 0;
    stSheet.y   = 0;
    stSheet.w   = gui_bg_stCtl.stLayer.w;
    stSheet.h   = gui_bg_stCtl.stLayer.h;
    stSheet.z   = GUI_Z_ORDER_BACKGROUND;
    stSheet.v   = 1;
    stSheet.bpp = gui_bg_stCtl.stLayer.bpp;

    drv_gfx_CreateSheet(&stSheet, &(gui_bg_stCtl.hSheet));

    gui_bg_Desktop();
    gui_bg_Logo();
    gui_bg_Name();
    gui_bg_Time();

    {
        DRV_GFX_POINT stPos;
        stPos.x = GUI_BG_NAME_X;
        stPos.y = GUI_BG_BAR_H + 1;
        gui_log_Open(&stPos);
        gui_log_Update();
    }
}

void gui_bg_Time()
{
    DRV_GFX_RECT  stRect;
    DRV_GFX_FONT  stFont;
    DRV_GFX_POINT stPos;

    gui_bg_stCtl.stTime.m++;

    if (gui_bg_stCtl.stTime.m >= 60) {
        gui_bg_stCtl.stTime.m = 0;
        gui_bg_stCtl.stTime.h++;

        if (gui_bg_stCtl.stTime.h >= 24) {
            gui_bg_stCtl.stTime.h = 0;
        }
    }

    sprintf(gui_bg_stCtl.stTime.buf, "%2d:%2d", gui_bg_stCtl.stTime.h, gui_bg_stCtl.stTime.m);

    stFont.w    = 8;
    stFont.h    = 16;
    stFont.data = font_hankaku;
    stFont.bpc  = 16;
    stFont.bg   = GUI_BG_COLOR_TIME_BG;

    stPos.x = gui_bg_stCtl.stLayer.w - stFont.w * 5;
    stPos.y = GUI_BG_TIME_Y;

    stRect.x = stPos.x;
    stRect.y = stPos.y;
    stRect.w = stFont.w * 5;
    stRect.h = stFont.h;

    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_BAR);
    drv_gfx_FillRect(gui_bg_stCtl.hSheet, &stRect);

    drv_gfx_SetFont( gui_bg_stCtl.hSheet, &stFont);
    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_TIME);
    drv_gfx_DrawStr(gui_bg_stCtl.hSheet, &stPos, gui_bg_stCtl.stTime.buf);
}


GUI_PRIVATE void gui_bg_Desktop(void)
{
    DRV_GFX_RECT  stRect;

    // background
    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_DESK);
    stRect.x = 0;
    stRect.y = GUI_BG_BAR_H;
    stRect.w = gui_bg_stCtl.stLayer.w;
    stRect.h = gui_bg_stCtl.stLayer.h - GUI_BG_BAR_H;
    drv_gfx_FillRect(gui_bg_stCtl.hSheet, &stRect);

    // desk bar
    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_BAR);
    stRect.x = 0;
    stRect.y = 0;
    stRect.w = gui_bg_stCtl.stLayer.w;
    stRect.h = GUI_BG_BAR_H;
    drv_gfx_FillRect(gui_bg_stCtl.hSheet, &stRect);

    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_BAR_EDGE);
    stRect.x = 0;
    stRect.y = GUI_BG_BAR_H - 2;
    stRect.w = gui_bg_stCtl.stLayer.w;
    stRect.h = 1;
    drv_gfx_FillRect(gui_bg_stCtl.hSheet, &stRect);
}

GUI_PRIVATE void gui_bg_Logo(void)
{
    DRV_GFX_DATA  stData;

    stData.x    = GUI_BG_LOGO_X;
    stData.y    = GUI_BG_LOGO_Y;
    stData.w    = GUI_BG_LOGO_W;
    stData.h    = GUI_BG_LOGO_H;
    stData.bpp  = 1;
    stData.data = gui_bg_abyLogo;

    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_LOGO);
    drv_gfx_DrawData(gui_bg_stCtl.hSheet, &stData);
}

GUI_PRIVATE void gui_bg_Name(void)
{
    DRV_GFX_FONT stFont;
    DRV_GFX_POINT stPos;

    stFont.w    = 8;
    stFont.h    = 16;
    stFont.data = font_hankaku;
    stFont.bpc  = 16;
    stFont.bg   = GUI_BG_COLOR_NAME_BG;

    stPos.x = GUI_BG_NAME_X;
    stPos.y = GUI_BG_NAME_Y;

    drv_gfx_SetFont( gui_bg_stCtl.hSheet, &stFont);
    drv_gfx_SetColor(gui_bg_stCtl.hSheet, GUI_BG_COLOR_NAME);
    drv_gfx_DrawStr(gui_bg_stCtl.hSheet, &stPos, GUI_BG_NAME);
}

#endif // (CPU_EXT_DISP_MODE != CPU_EXT_DISP_MODE_TEXT)

