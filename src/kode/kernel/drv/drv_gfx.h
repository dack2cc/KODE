#ifndef __DRV_GFX_H__
#define __DRV_GFX_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Definition
******************************************************************************/

#define DRV_GFX_SHEET_MAX    (32)

/*********************************************************************
*
*       Standard colors
*/
#define DRV_GFX_COLOR_INVALID_COLOR 0x0FFFFFFF      /* Invalid color - more than 24 bits */
#define DRV_GFX_COLOR_TRANSPARENT   0xFF000000
#define DRV_GFX_COLOR_BLUE          0x00FF0000
#define DRV_GFX_COLOR_GREEN         0x0000FF00
#define DRV_GFX_COLOR_RED           0x000000FF
#define DRV_GFX_COLOR_CYAN          0x00FFFF00
#define DRV_GFX_COLOR_MAGENTA       0x00FF00FF
#define DRV_GFX_COLOR_YELLOW        0x0000FFFF
#define DRV_GFX_COLOR_LIGHTBLUE     0x00FF8080
#define DRV_GFX_COLOR_LIGHTGREEN    0x0080FF80
#define DRV_GFX_COLOR_LIGHTRED      0x008080FF
#define DRV_GFX_COLOR_LIGHTCYAN     0x00FFFF80
#define DRV_GFX_COLOR_LIGHTMAGENTA  0x00FF80FF
#define DRV_GFX_COLOR_LIGHTYELLOW   0x0080FFFF
#define DRV_GFX_COLOR_DARKBLUE      0x00800000
#define DRV_GFX_COLOR_DARKGREEN     0x00008000
#define DRV_GFX_COLOR_DARKRED       0x00000080
#define DRV_GFX_COLOR_DARKCYAN      0x00808000
#define DRV_GFX_COLOR_DARKMAGENTA   0x00800080
#define DRV_GFX_COLOR_DARKYELLOW    0x00008080
#define DRV_GFX_COLOR_WHITE         0x00FFFFFF
#define DRV_GFX_COLOR_LIGHTGRAY     0x00D3D3D3
#define DRV_GFX_COLOR_GRAY          0x00808080
#define DRV_GFX_COLOR_DARKGRAY      0x00404040
#define DRV_GFX_COLOR_BLACK         0x00000000
#define DRV_GFX_COLOR_BROWN         0x002A2AA5
#define DRV_GFX_COLOR_ORANGE        0x0000A5FF
#define DRV_GFX_COLOR_BLUE_1_2      0x007F0000
#define DRV_GFX_COLOR_GREEN_1_2     0x00007F00
#define DRV_GFX_COLOR_RED_1_2       0x0000007F
#define DRV_GFX_COLOR_CYAN_1_2      0x007F7F00
#define DRV_GFX_COLOR_MAGENTA_1_2   0x007F007F
#define DRV_GFX_COLOR_YELLOW_1_2    0x00007F7F
#define DRV_GFX_COLOR_WHITE_1_2     0x007F7F7F

typedef CPU_INT32U  DRV_GFX_COLOR;
typedef CPU_INT32U  DRV_GFX_HANDLE;

typedef struct _DRV_GFX_LAYER {
	CPU_INT32U   w;
	CPU_INT32U   h;
	CPU_INT32U   bpp;  /* bits per pixel */
} DRV_GFX_LAYER;

typedef struct _DRV_GFX_POINT {
	CPU_INT32S   x;
	CPU_INT32S   y;
} DRV_GFX_POINT;

typedef struct _DRV_GFX_RECT {
	CPU_INT32S   x;
	CPU_INT32S   y;
	CPU_INT32U   w;
	CPU_INT32U   h;
} DRV_GFX_RECT;

typedef struct {
	CPU_INT32S   x;
	CPU_INT32S   y;
	CPU_INT32U   w;
	CPU_INT32U   h;
	CPU_INT08U   z;    /* z order */
	CPU_INT08U   v;    /* visible */
	CPU_INT08U   bpp;  /* bits per pixel */
} DRV_GFX_SHEET;

typedef struct _DRV_GFX_DATA {
	CPU_INT32S   x;
	CPU_INT32S   y;
	CPU_INT32U   w;
	CPU_INT32U   h;
	CPU_INT08U * data;
	CPU_INT08U   bpp;  /* bits per pixel */
} DRV_GFX_DATA;

typedef struct _DRV_GFX_FONT {
	CPU_INT32U    w;
	CPU_INT32U    h;
	CPU_INT08U *  data;
	CPU_INT08U    bpc;  /* byte per character  */
	DRV_GFX_COLOR bg;   /* color of background */
} DRV_GFX_FONT;

/******************************************************************************
    Public Interface
******************************************************************************/

extern void drv_gfx_Init(void);
extern void drv_gfx_GetLayerInfo(DRV_GFX_LAYER * pstLayer_out);
extern void drv_gfx_Refresh(void);

extern void drv_gfx_CreateSheet(const DRV_GFX_SHEET* pstSheet_in, DRV_GFX_HANDLE * phSheet_out);
extern void drv_gfx_DeleteSheet(const DRV_GFX_HANDLE hSheet_in);

extern void drv_gfx_SetFont(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_FONT * pstFont_in);
extern void drv_gfx_SetVisible(const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiIsVisible_in);
extern void drv_gfx_SetZOrder(const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiZOrder_in);
extern void drv_gfx_SetColor(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_COLOR  uiColor_in);
extern void drv_gfx_FillRect(const DRV_GFX_HANDLE hSheet_in, DRV_GFX_RECT * pstRect_in);
extern void drv_gfx_DrawData(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_DATA * pstData_in);
extern void drv_gfx_DrawStr(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_POINT * pstHeadPos_in, const CPU_CHAR * pszStr_in);
extern void drv_gfx_Move(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_POINT * pstPos_in);

//extern void drv_gfx_SetFont( const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_DATA * pstFont_in);
//extern void drv_gfx_DrawChar(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_POINT * pstPos_in, const CPU_CHAR byChar_in);

#endif // __DRV_GFX_H__

