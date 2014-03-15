
/******************************************************************************
    Include
******************************************************************************/

#include <drv_gfx.h>
#include <drv_disp.h>
#include <cpu_ext.h>
#include <lib_mem.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE 

DRV_PRIVATE  DRV_GFX_COLOR  drv_gfx_auiPalette[] = {
	DRV_GFX_COLOR_INVALID_COLOR,
	DRV_GFX_COLOR_TRANSPARENT,
	DRV_GFX_COLOR_BLUE,
	DRV_GFX_COLOR_GREEN,
	DRV_GFX_COLOR_RED,
	DRV_GFX_COLOR_CYAN,
	DRV_GFX_COLOR_MAGENTA,
	DRV_GFX_COLOR_YELLOW,
	DRV_GFX_COLOR_LIGHTBLUE,
	DRV_GFX_COLOR_LIGHTGREEN,
	DRV_GFX_COLOR_LIGHTRED,
	DRV_GFX_COLOR_LIGHTCYAN,
	DRV_GFX_COLOR_LIGHTMAGENTA,
	DRV_GFX_COLOR_LIGHTYELLOW,
	DRV_GFX_COLOR_DARKBLUE,
	DRV_GFX_COLOR_DARKGREEN,
	DRV_GFX_COLOR_DARKRED,
	DRV_GFX_COLOR_DARKCYAN,
	DRV_GFX_COLOR_DARKMAGENTA,
	DRV_GFX_COLOR_DARKYELLOW,
	DRV_GFX_COLOR_WHITE,
	DRV_GFX_COLOR_LIGHTGRAY,
	DRV_GFX_COLOR_GRAY,
	DRV_GFX_COLOR_DARKGRAY,
	DRV_GFX_COLOR_BLACK,
	DRV_GFX_COLOR_BROWN,
	DRV_GFX_COLOR_ORANGE,
    DRV_GFX_COLOR_BLUE_1_2,
	DRV_GFX_COLOR_GREEN_1_2,
	DRV_GFX_COLOR_RED_1_2,
	DRV_GFX_COLOR_CYAN_1_2,
	DRV_GFX_COLOR_MAGENTA_1_2,
	DRV_GFX_COLOR_YELLOW_1_2,
	DRV_GFX_COLOR_WHITE_1_2,
};
#define DRV_GFX_PALETTE_MAX    (sizeof(drv_gfx_auiPalette)/sizeof(DRV_GFX_COLOR))

typedef struct _DRV_GFX_SHEET_EXT {
	DRV_GFX_SHEET               stSht;
	CPU_INT32U                  uiPitch;
	CPU_INT08U *                pbyAddr;
	struct _DRV_GFX_SHEET_EXT * pstPrev;
	struct _DRV_GFX_SHEET_EXT * pstNext;
} DRV_GFX_SHEET_EXT;

typedef struct _DRV_GFX_REGION {
	CPU_INT32S   x1;
	CPU_INT32S   y1;
	CPU_INT32S   x2;
	CPU_INT32S   y2;
} DRV_GFX_REGION;

typedef struct _DRV_GFX_CONTROL {
	CPU_INT08U          uiBitPerPixel;
	CPU_INT32U          uiWidth;
	CPU_INT32U          uiHeight;
	DRV_GFX_REGION      stDirty;
	CPU_INT08U *        pbyBufAdr;
	CPU_SIZE_T          uiBufSize;
	CPU_SIZE_T          uiBufPitch;
	DRV_GFX_SHEET_EXT * pstShtActv;
	DRV_GFX_SHEET_EXT * pstShtFree;
} DRV_GFX_CONTROL;

DRV_PRIVATE  DRV_GFX_CONTROL   drv_gfx_stCtl;
DRV_PRIVATE DRV_GFX_SHEET_EXT  drv_gfx_astSurface[DRV_GFX_SHEET_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/



/******************************************************************************
    Function Definition
******************************************************************************/

void drv_gfx_Init(void)
{
	CPU_INT32U i = 0;
	
	Mem_Clr(&drv_gfx_stCtl,     sizeof(drv_gfx_stCtl));
	Mem_Clr(drv_gfx_astSurface, sizeof(drv_gfx_astSurface));
	
	for (i = 0; i < DRV_GFX_SHEET_MAX; ++i) {
	}
	
	CPUExt_DispBitPerPixel(&(drv_gfx_stCtl.uiBitPerPixel));
	CPUExt_DispResolution(&(drv_gfx_stCtl.uiWidth), &(drv_gfx_stCtl.uiHeight));
	
	drv_gfx_stCtl.uiBufPitch = drv_gfx_stCtl.uiWidth * (drv_gfx_stCtl.uiBitPerPixel / 8);
	drv_gfx_stCtl.uiBufSize  = drv_gfx_stCtl.uiWidth * drv_gfx_stCtl.uiHeight * (drv_gfx_stCtl.uiBitPerPixel / 8);
	drv_gfx_stCtl.pbyBufAdr  = (CPU_INT08U *)lib_pool_Malloc(drv_gfx_stCtl.uiBufSize);
	if (0 == drv_gfx_stCtl.pbyBufAdr) {
		CPUExt_CorePanic("[drv_gfx_Init][Out Of Memory]");
	}
	drv_disp_Printf("[drv_gfx_Init][BufAddr:0x%X]\r\n", drv_gfx_stCtl.pbyBufAdr);
	drv_disp_Printf("[drv_gfx_Init][BufSize:%dKB]\r\n", drv_gfx_stCtl.uiBufSize / 1024);
	
	CPUExt_DispSetPalette(drv_gfx_auiPalette, DRV_GFX_PALETTE_MAX, 0);
	
	return;
}

void drv_gfx_GetLayerInfo(DRV_GFX_LAYER * pstLayer_out)
{
	if (0 != pstLayer_out) {
		pstLayer_out->w   = drv_gfx_stCtl.uiWidth;
		pstLayer_out->h   = drv_gfx_stCtl.uiHeight;
		pstLayer_out->bpp = drv_gfx_stCtl.uiBitPerPixel;
	}
}


void drv_gfx_CreateSheet(const DRV_GFX_SHEET * pstSheet_in, DRV_GFX_HANDLE * phSheet_out)
{
}

void drv_gfx_DeleteSheet(const DRV_GFX_HANDLE hSheet_in)
{
}


