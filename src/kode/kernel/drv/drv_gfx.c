
/******************************************************************************
    Include
******************************************************************************/

#include <drv_gfx.h>

#if (CPU_EXT_DISP_MODE_TEXT != CPU_EXT_DISP_MODE)

#include <drv_disp.h>
#include <cpu_ext.h>
#include <lib_mem.h>
#include <lib_pool.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE

typedef struct _DRV_GFX_PALETTE {
    DRV_GFX_COLOR * pData;
    CPU_INT08U      uiCount;
} DRV_GFX_PALETTE;

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

DRV_PRIVATE  DRV_GFX_PALETTE  drv_gfx_stGlobalPalette = {
    drv_gfx_auiPalette,
    DRV_GFX_PALETTE_MAX,
};

typedef struct _DRV_GFX_REGION {
    CPU_INT32S   x0;
    CPU_INT32S   y0;
    CPU_INT32S   x1;
    CPU_INT32S   y1;
} DRV_GFX_REGION;

typedef struct _DRV_GFX_SHEET_EXT {
    DRV_GFX_SHEET               stSht;
    DRV_GFX_FONT                stFont;
    DRV_GFX_POINT               stStrPos;
    CPU_INT32U                  uiPitch;
    CPU_INT08U *                pbyAddr;
    CPU_INT32U                  uiHandle;
    CPU_INT32U                  uiColor;
    CPU_INT32U                  uiBGColor;
    DRV_GFX_PALETTE *           pstPalette;
    DRV_GFX_REGION              stDirty;
    //struct _DRV_GFX_SHEET_EXT * pstPrev;
    struct _DRV_GFX_SHEET_EXT * pstNext;
} DRV_GFX_SHEET_EXT;

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

DRV_PRIVATE  DRV_GFX_CONTROL    drv_gfx_stCtl;
DRV_PRIVATE  DRV_GFX_SHEET_EXT  drv_gfx_astSheet[DRV_GFX_SHEET_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/

DRV_PRIVATE  DRV_GFX_SHEET_EXT * drv_gfx_SheetGetFree();
DRV_PRIVATE  void drv_gfx_SheetInsertActive(DRV_GFX_SHEET_EXT * pstSheet_in);
DRV_PRIVATE  void drv_gfx_SheetRelease(DRV_GFX_SHEET_EXT * pstSheet_in);

DRV_PRIVATE  void drv_gfx_MapDirty(void);
DRV_PRIVATE  void drv_gfx_UpdateDirtyForScreen(DRV_GFX_REGION * pstRegion_in);
DRV_PRIVATE  void drv_gfx_UpdateDirtyForSheet(DRV_GFX_SHEET_EXT * pstSheet_in, DRV_GFX_REGION * pstRegion_in);
DRV_PRIVATE  void drv_gfx_BitBlt(DRV_GFX_SHEET_EXT * pstSheet_in);

DRV_PRIVATE  void drv_gfx_DispData(DRV_GFX_SHEET_EXT * pstSheet_in, const DRV_GFX_DATA * pstData_in);
DRV_PRIVATE  void drv_gfx_DispChar(DRV_GFX_SHEET_EXT * pstSheet_in, const CPU_CHAR chAscii_in);
DRV_PRIVATE  void drv_gfx_DispLineFeed(DRV_GFX_SHEET_EXT * pstSheet_in);
DRV_PRIVATE  void drv_gfx_DispCarriageReturn(DRV_GFX_SHEET_EXT * pstSheet_in);
DRV_PRIVATE  void drv_gfx_DispScrollUp(DRV_GFX_SHEET_EXT * pstSheet_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void drv_gfx_Init(void)
{
    CPU_INT32U  i = 0;

    Mem_Clr(&drv_gfx_stCtl,   sizeof(drv_gfx_stCtl));
    Mem_Clr(drv_gfx_astSheet, sizeof(drv_gfx_astSheet));

    drv_gfx_stCtl.stDirty.x0 = -1;
    drv_gfx_stCtl.stDirty.x1 = -1;
    drv_gfx_stCtl.stDirty.y0 = -1;
    drv_gfx_stCtl.stDirty.y1 = -1;

    for (i = 0; i < DRV_GFX_SHEET_MAX; ++i) {
        drv_gfx_astSheet[i].stDirty.x0 = -1;
        drv_gfx_astSheet[i].stDirty.y0 = -1;
        drv_gfx_astSheet[i].stDirty.x1 = -1;
        drv_gfx_astSheet[i].stDirty.y1 = -1;

        drv_gfx_astSheet[i].uiHandle   = i;
        drv_gfx_astSheet[i].pstPalette = &drv_gfx_stGlobalPalette;
        drv_gfx_astSheet[i].pstNext    = drv_gfx_astSheet + i + 1;
    }

    drv_gfx_astSheet[i].pstNext = 0;

    drv_gfx_stCtl.pstShtFree = drv_gfx_astSheet;
    drv_gfx_stCtl.pstShtActv = 0;

    CPUExt_DispBitPerPixel(&(drv_gfx_stCtl.uiBitPerPixel));
    CPUExt_DispResolution(&(drv_gfx_stCtl.uiWidth), &(drv_gfx_stCtl.uiHeight));

    drv_gfx_stCtl.uiBufPitch = drv_gfx_stCtl.uiWidth * (drv_gfx_stCtl.uiBitPerPixel / 8);
    drv_gfx_stCtl.uiBufSize  = drv_gfx_stCtl.uiWidth * drv_gfx_stCtl.uiHeight * (drv_gfx_stCtl.uiBitPerPixel / 8);
    drv_gfx_stCtl.pbyBufAdr  = (CPU_INT08U *)lib_pool_Malloc(drv_gfx_stCtl.uiBufSize);

    if (0 == drv_gfx_stCtl.pbyBufAdr) {
        CPUExt_CorePanic("[drv_gfx_Init][Out Of Memory]");
    }

    //drv_disp_Printf("[drv_gfx_Init][BufAddr:0x%X]\r\n", drv_gfx_stCtl.pbyBufAdr);
    //drv_disp_Printf("[drv_gfx_Init][BufSize:%dKB]\r\n", drv_gfx_stCtl.uiBufSize / 1024);

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

void drv_gfx_Refresh(void)
{
    DRV_GFX_SHEET_EXT * pstSheetExt = drv_gfx_stCtl.pstShtActv;

    if (0 == pstSheetExt) {
        return;
    }

    drv_gfx_MapDirty();

    while (0 != pstSheetExt) {
        if (0 != pstSheetExt->stSht.v) {
            drv_gfx_BitBlt(pstSheetExt);
        }

        pstSheetExt = pstSheetExt->pstNext;
    }

    CPUExt_DispBitBlt(
        drv_gfx_stCtl.pbyBufAdr,
        drv_gfx_stCtl.stDirty.x0, drv_gfx_stCtl.stDirty.y0,
        drv_gfx_stCtl.stDirty.x1, drv_gfx_stCtl.stDirty.y1
    );

    drv_gfx_stCtl.stDirty.x0 = -1;
    drv_gfx_stCtl.stDirty.y0 = -1;
    drv_gfx_stCtl.stDirty.x1 = -1;
    drv_gfx_stCtl.stDirty.y1 = -1;
}


void drv_gfx_CreateSheet(const DRV_GFX_SHEET * pstSheet_in, DRV_GFX_HANDLE * phSheet_out)
{
    DRV_GFX_SHEET_EXT * pstSheetExt = drv_gfx_SheetGetFree();

    if ((0 == pstSheet_in)
            ||  (0 == phSheet_out)
            ||  (0 == pstSheetExt)) {
        return;
    }

    pstSheetExt->stSht = (*pstSheet_in);
    pstSheetExt->uiPitch = pstSheet_in->w * pstSheet_in->bpp / 8;
    pstSheetExt->pbyAddr = lib_pool_Malloc(pstSheetExt->uiPitch * pstSheet_in->h);

    if (0 == pstSheetExt->pbyAddr) {
        CPUExt_CorePanic("[drv_gfx_CreateSheet][Out of memory]");
        return;
    }

    Mem_Clr(pstSheetExt->pbyAddr, pstSheetExt->uiPitch * pstSheet_in->h);

    drv_gfx_SheetInsertActive(pstSheetExt);
    (*phSheet_out) = pstSheetExt->uiHandle;
    return;
}

void drv_gfx_DeleteSheet(const DRV_GFX_HANDLE hSheet_in)
{
    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        drv_gfx_SheetRelease(&(drv_gfx_astSheet[hSheet_in]));
    }
}

void drv_gfx_GetSheet(const DRV_GFX_HANDLE hSheet_in, DRV_GFX_SHEET* pstSheet_out)
{
    if (0 == pstSheet_out) {
        return;
    }

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        (*pstSheet_out) = (drv_gfx_astSheet[hSheet_in].stSht);
    }
}


void drv_gfx_SetFont(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_FONT * pstFont_in)
{
    if ((hSheet_in < DRV_GFX_SHEET_MAX) && (0 != pstFont_in)) {
        drv_gfx_astSheet[hSheet_in].stFont = (*pstFont_in);
    }
}

void drv_gfx_SetVisible(const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiIsVisible_in)
{
    DRV_GFX_REGION stRegion;

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        drv_gfx_astSheet[hSheet_in].stSht.v = uiIsVisible_in;

        stRegion.x0 = drv_gfx_astSheet[hSheet_in].stSht.x;
        stRegion.y0 = drv_gfx_astSheet[hSheet_in].stSht.y;
        stRegion.x1 = drv_gfx_astSheet[hSheet_in].stSht.x + drv_gfx_astSheet[hSheet_in].stSht.w - 1;
        stRegion.y1 = drv_gfx_astSheet[hSheet_in].stSht.y + drv_gfx_astSheet[hSheet_in].stSht.h - 1;
        drv_gfx_UpdateDirtyForScreen(&stRegion);
    }
}

void drv_gfx_SetZOrder(const DRV_GFX_HANDLE hSheet_in, const CPU_INT08U uiZOrder_in)
{
    DRV_GFX_REGION stRegion;

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        // rerange

        stRegion.x0 = drv_gfx_astSheet[hSheet_in].stSht.x;
        stRegion.y0 = drv_gfx_astSheet[hSheet_in].stSht.y;
        stRegion.x1 = drv_gfx_astSheet[hSheet_in].stSht.x + drv_gfx_astSheet[hSheet_in].stSht.w - 1;
        stRegion.y1 = drv_gfx_astSheet[hSheet_in].stSht.y + drv_gfx_astSheet[hSheet_in].stSht.h - 1;
        drv_gfx_UpdateDirtyForScreen(&stRegion);
    }
}

void drv_gfx_SetColor(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_COLOR  uiColor_in)
{
    DRV_GFX_SHEET_EXT * pstSheet = 0;
    CPU_INT32U          i = 0;

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        pstSheet = drv_gfx_astSheet + hSheet_in;
    }

    if (0 == pstSheet) {
        return;
    }

    switch (pstSheet->stSht.bpp) {
    case 8:
        for (i = 0; i < pstSheet->pstPalette->uiCount; ++i) {
            if (uiColor_in == pstSheet->pstPalette->pData[i]) {
                break;
            }
        }

        if (i >= pstSheet->pstPalette->uiCount) {
            i = 0;
        }

        pstSheet->uiColor = i;
        break;

    case 24:
        pstSheet->uiColor = uiColor_in;
        break;

    case 32:
        pstSheet->uiColor = uiColor_in;
        break;

    default:
        // EXCEPTION
        break;
    }
}

void drv_gfx_FillRect(const DRV_GFX_HANDLE hSheet_in, DRV_GFX_RECT * pstRect_in)
{
    DRV_GFX_SHEET_EXT * pstSheet = 0;
    CPU_INT32U          y = 0;
    DRV_GFX_REGION      stRegion;

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        pstSheet = drv_gfx_astSheet + hSheet_in;
    }

    if ((0 == pstSheet) || (0 == pstRect_in) || (0 == pstRect_in->w) || (0 == pstRect_in->h)) {
        return;
    }

    if (pstRect_in->x < 0) {
        pstRect_in->x = 0;
    }

    if (pstRect_in->y < 0) {
        pstRect_in->y = 0;
    }

    if (pstRect_in->x + pstRect_in->w > drv_gfx_stCtl.uiWidth) {
        pstRect_in->w = drv_gfx_stCtl.uiWidth - pstRect_in->x;
    }

    if (pstRect_in->y + pstRect_in->h > drv_gfx_stCtl.uiHeight) {
        pstRect_in->h = drv_gfx_stCtl.uiHeight - pstRect_in->y;
    }

    // 8bit only currently
    for (y = pstRect_in->y; y < pstRect_in->y + pstRect_in->h; ++y) {
#if 1
        Mem_Set(
            (pstSheet->pbyAddr + y * pstSheet->uiPitch + pstRect_in->x),
            pstSheet->uiColor,
            pstRect_in->w
        );
#else
        Mem_Set(
            (((*(CPU_INT32U *)0x90014)) + y * pstSheet->uiPitch + pstRect_in->x),
            (CPU_INT08U)(pstSheet->uiColor),
            pstRect_in->w
        );
#endif
    }

    stRegion.x0 = pstRect_in->x;
    stRegion.y0 = pstRect_in->y;
    stRegion.x1 = pstRect_in->x + pstRect_in->w - 1;
    stRegion.y1 = pstRect_in->y + pstRect_in->h - 1;
    drv_gfx_UpdateDirtyForSheet(pstSheet, &stRegion);
}

void drv_gfx_DrawData(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_DATA * pstData_in)
{
    DRV_GFX_SHEET_EXT * pstSheet = 0;

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        pstSheet = drv_gfx_astSheet + hSheet_in;
    }

    drv_gfx_DispData(pstSheet, pstData_in);
}

void drv_gfx_DrawStr(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_POINT * pstPos_in, const CPU_CHAR * pszStr_in)
{
    DRV_GFX_SHEET_EXT * pstSheet = 0;
    CPU_CHAR * pbyChar = 0;

    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        pstSheet = drv_gfx_astSheet + hSheet_in;
    }

    if ((0 == pstSheet)
            ||  (0 == pszStr_in)
            ||  (0 == pstSheet->stFont.data)) {
        return;
    }

    if (0 != pstPos_in) {
        pstSheet->stStrPos = (*pstPos_in);
    }

    pbyChar = (CPU_CHAR *)pszStr_in;

    while ('\0' != (*pbyChar)) {
        drv_gfx_DispChar(pstSheet, (*pbyChar));
        ++pbyChar;
    }
}


void drv_gfx_Move(const DRV_GFX_HANDLE hSheet_in, const DRV_GFX_POINT * pstPos_in)
{
    DRV_GFX_SHEET_EXT * pstSheet = 0;
    DRV_GFX_REGION  stRegLayer;
    DRV_GFX_REGION  stRegSheet;


    if (hSheet_in < DRV_GFX_SHEET_MAX) {
        pstSheet = drv_gfx_astSheet + hSheet_in;
    }

    if ((0 == pstSheet)
            ||  (0 == pstPos_in)) {
        return;
    }

    stRegLayer.x0 = pstSheet->stSht.x;
    stRegLayer.y0 = pstSheet->stSht.y;
    stRegLayer.x1 = pstSheet->stSht.x + pstSheet->stSht.w - 1;
    stRegLayer.y1 = pstSheet->stSht.y + pstSheet->stSht.h - 1;

    pstSheet = drv_gfx_stCtl.pstShtActv;

    while (0 != pstSheet) {
        stRegSheet.x0 = stRegLayer.x0 - pstSheet->stSht.x;
        stRegSheet.y0 = stRegLayer.y0 - pstSheet->stSht.y;
        stRegSheet.x1 = stRegLayer.x1 - pstSheet->stSht.x;
        stRegSheet.y1 = stRegLayer.y1 - pstSheet->stSht.y;

        drv_gfx_UpdateDirtyForSheet(pstSheet, &stRegSheet);

        pstSheet = pstSheet->pstNext;
    }

    pstSheet = drv_gfx_astSheet + hSheet_in;

    pstSheet->stSht.x = pstPos_in->x;
    pstSheet->stSht.y = pstPos_in->y;

    stRegSheet.x0 = 0;
    stRegSheet.y0 = 0;
    stRegSheet.x1 = pstSheet->stSht.w - 1;
    stRegSheet.y1 = pstSheet->stSht.h - 1;
    drv_gfx_UpdateDirtyForSheet(pstSheet, &stRegSheet);
}


DRV_PRIVATE  DRV_GFX_SHEET_EXT * drv_gfx_SheetGetFree()
{
    DRV_GFX_SHEET_EXT * pstSheetExt = drv_gfx_stCtl.pstShtFree;

    if (0 == drv_gfx_stCtl.pstShtFree) {
        return (0);
    }

    drv_gfx_stCtl.pstShtFree = pstSheetExt->pstNext;
    pstSheetExt->pstNext = 0;

    return (pstSheetExt);
}

DRV_PRIVATE  void drv_gfx_SheetInsertActive(DRV_GFX_SHEET_EXT * pstSheet_in)
{
    DRV_GFX_SHEET_EXT * pstSheetExt = drv_gfx_stCtl.pstShtActv;

    if (0 == drv_gfx_stCtl.pstShtActv) {
        drv_gfx_stCtl.pstShtActv = pstSheet_in;
        pstSheet_in->pstNext = 0;
        return;
    }

    while (0 != pstSheetExt) {
        if ( (pstSheetExt->stSht.z >= pstSheet_in->stSht.z)
                &&  ((0 == pstSheetExt->pstNext) || (pstSheetExt->pstNext->stSht.z < pstSheet_in->stSht.z)) ) {
            pstSheet_in->pstNext = pstSheetExt->pstNext;
            pstSheetExt->pstNext = pstSheet_in;
            break;
        }

        pstSheetExt = pstSheetExt->pstNext;
    }
}

DRV_PRIVATE  void drv_gfx_SheetRelease(DRV_GFX_SHEET_EXT * pstSheet_in)
{
    DRV_GFX_SHEET_EXT * pstSheetExt = drv_gfx_stCtl.pstShtActv;

    if (0 == drv_gfx_stCtl.pstShtActv) {
        CPUExt_CorePanic("[drv_gfx_SheetRelease][No active sheet]");
        return;
    }

    while (0 != pstSheetExt) {
        if (pstSheetExt->pstNext == pstSheet_in) {
            pstSheetExt->pstNext = pstSheet_in->pstNext;
            pstSheet_in->pstNext = 0;
            break;
        }

        pstSheetExt = pstSheetExt->pstNext;
    }

    if (0 == pstSheetExt) {
        CPUExt_CorePanic("[drv_gfx_SheetRelease][It is not an active sheet]");
    }

    pstSheet_in->pstNext = drv_gfx_stCtl.pstShtFree;
    drv_gfx_stCtl.pstShtFree = pstSheet_in;
}

DRV_PRIVATE  void drv_gfx_MapDirty(void)
{
    DRV_GFX_SHEET_EXT * pstSheet = 0;
    DRV_GFX_REGION      stRegion;

    if (0 == drv_gfx_stCtl.pstShtActv) {
        return;
    }

    pstSheet = drv_gfx_stCtl.pstShtActv;

    while (0 != pstSheet->pstNext) {
        stRegion.x0 = pstSheet->stSht.x + pstSheet->stDirty.x0 - pstSheet->pstNext->stSht.x;
        stRegion.y0 = pstSheet->stSht.y + pstSheet->stDirty.y0 - pstSheet->pstNext->stSht.y;
        stRegion.x1 = pstSheet->stSht.x + pstSheet->stDirty.x1 - pstSheet->pstNext->stSht.x;
        stRegion.y1 = pstSheet->stSht.y + pstSheet->stDirty.y1 - pstSheet->pstNext->stSht.y;

        drv_gfx_UpdateDirtyForSheet(pstSheet->pstNext, &(stRegion));

        if ((pstSheet->stDirty.x0 + pstSheet->stSht.x >= pstSheet->pstNext->stDirty.x0 + pstSheet->pstNext->stSht.x)
                &&  (pstSheet->stDirty.y0 + pstSheet->stSht.y >= pstSheet->pstNext->stDirty.y0 + pstSheet->pstNext->stSht.y)
                &&  (pstSheet->stDirty.x1 + pstSheet->stSht.x <= pstSheet->pstNext->stDirty.x1 + pstSheet->pstNext->stSht.x)
                &&  (pstSheet->stDirty.y1 + pstSheet->stSht.y <= pstSheet->pstNext->stDirty.y1 + pstSheet->pstNext->stSht.y)) {
            pstSheet->stDirty.x0 = -1;
            pstSheet->stDirty.y0 = -1;
            pstSheet->stDirty.x1 = -1;
            pstSheet->stDirty.y1 = -1;
        }

        pstSheet = pstSheet->pstNext;
    }
}


DRV_PRIVATE  void drv_gfx_UpdateDirtyForScreen(DRV_GFX_REGION * pstRegion_in)
{
    if (0 == pstRegion_in) {
        return;
    }

    if (pstRegion_in->x1 >= drv_gfx_stCtl.uiWidth) {
        pstRegion_in->x1 = drv_gfx_stCtl.uiWidth - 1;
    }

    if (pstRegion_in->y1 >= drv_gfx_stCtl.uiHeight) {
        pstRegion_in->y1 = drv_gfx_stCtl.uiHeight - 1;
    }

    if ((pstRegion_in->x0 < 0)
            ||  (pstRegion_in->x0 > pstRegion_in->x1)
            ||  (pstRegion_in->y0 < 0)
            ||  (pstRegion_in->y0 > pstRegion_in->y1)) {
        return;
    }

    if ((drv_gfx_stCtl.stDirty.x0 > pstRegion_in->x0)
            ||  (drv_gfx_stCtl.stDirty.x0 < 0)) {
        drv_gfx_stCtl.stDirty.x0 = pstRegion_in->x0;
    }

    if (drv_gfx_stCtl.stDirty.x1 < pstRegion_in->x1) {
        drv_gfx_stCtl.stDirty.x1 = pstRegion_in->x1;
    }

    if ((drv_gfx_stCtl.stDirty.y0 > pstRegion_in->y0)
            ||  (drv_gfx_stCtl.stDirty.y0 < 0)) {
        drv_gfx_stCtl.stDirty.y0 = pstRegion_in->y0;
    }

    if (drv_gfx_stCtl.stDirty.y1 < pstRegion_in->y1) {
        drv_gfx_stCtl.stDirty.y1 = pstRegion_in->y1;
    }
}

DRV_PRIVATE  void drv_gfx_UpdateDirtyForSheet(DRV_GFX_SHEET_EXT * pstSheet_in, DRV_GFX_REGION * pstRegion_in)
{
    if ((0 == pstSheet_in)
            ||  (0 == pstRegion_in)
            ||  (0 == pstSheet_in->stSht.w)
            ||  (0 == pstSheet_in->stSht.h)) {
        return;
    }

    if (pstRegion_in->x0 < 0) {
        pstRegion_in->x0 = 0;
    }

    if (pstRegion_in->y0 < 0) {
        pstRegion_in->y0 = 0;
    }

    if (pstRegion_in->x1 >= pstSheet_in->stSht.w) {
        pstRegion_in->x1 = pstSheet_in->stSht.w - 1;
    }

    if (pstRegion_in->y1 >= pstSheet_in->stSht.h) {
        pstRegion_in->y1 = pstSheet_in->stSht.h - 1;
    }

    if ((pstRegion_in->x0 > pstRegion_in->x1)
            ||  (pstRegion_in->y0 > pstRegion_in->y1)) {
        return;
    }

    if ((pstSheet_in->stDirty.x0 > pstRegion_in->x0)
            ||  (pstSheet_in->stDirty.x0 < 0)) {
        pstSheet_in->stDirty.x0 = pstRegion_in->x0;
    }

    if (pstSheet_in->stDirty.x1 < pstRegion_in->x1) {
        pstSheet_in->stDirty.x1 = pstRegion_in->x1;
    }

    if ((pstSheet_in->stDirty.y0 > pstRegion_in->y0)
            ||  (pstSheet_in->stDirty.y0 < 0)) {
        pstSheet_in->stDirty.y0 = pstRegion_in->y0;
    }

    if (pstSheet_in->stDirty.y1 < pstRegion_in->y1) {
        pstSheet_in->stDirty.y1 = pstRegion_in->y1;
    }
}


DRV_PRIVATE  void drv_gfx_BitBlt(DRV_GFX_SHEET_EXT * pstSheet_in)
{
    CPU_INT32U i = 0;
    CPU_INT32U j = 0;
    CPU_INT32U uiSrcX = 0;
    CPU_INT32U uiSrcY = 0;
    CPU_INT32U uiDstX = 0;
    CPU_INT32U uiDstY = 0;
    CPU_INT08U * pbyDst   = drv_gfx_stCtl.pbyBufAdr;
    CPU_INT32U uiDstPitch = drv_gfx_stCtl.uiBufPitch;
    CPU_INT08U * pbySrc   = 0;
    CPU_INT32U uiSrcPitch = 0;
    CPU_INT32U uiRegW = 0;
    CPU_INT32U uiRegH = 0;
    DRV_GFX_REGION  stRegion;

    if ((0 == drv_gfx_stCtl.pbyBufAdr)
            ||  (0 == pstSheet_in)
            ||  (0 == pstSheet_in->stSht.v)
            ||  (0 == pstSheet_in->pbyAddr)
            ||  (0 == pstSheet_in->stSht.w)
            ||  (0 == pstSheet_in->stSht.h)
            ||  (pstSheet_in->stDirty.x0 < 0)
            ||  (pstSheet_in->stDirty.x1 < pstSheet_in->stDirty.x0)
            ||  (pstSheet_in->stDirty.x1 >= pstSheet_in->stSht.w)
            ||  (pstSheet_in->stDirty.y0 < 0)
            ||  (pstSheet_in->stDirty.y1 < pstSheet_in->stDirty.y0)
            ||  (pstSheet_in->stDirty.y1 >= pstSheet_in->stSht.h)
            ||  (pstSheet_in->stSht.x <= -(pstSheet_in->stDirty.x1))
            ||  (pstSheet_in->stSht.x >= (drv_gfx_stCtl.uiWidth - pstSheet_in->stDirty.x0))
            ||  (pstSheet_in->stSht.y <= -(pstSheet_in->stDirty.y1))
            ||  (pstSheet_in->stSht.y >= (drv_gfx_stCtl.uiHeight - pstSheet_in->stDirty.y0))) {
        //drv_disp_Printf("[dirty][x0:%d,y0:%d,x1:%d,y1:%d]\r\n", pstSheet_in->stDirty.x0, pstSheet_in->stDirty.y0, pstSheet_in->stDirty.x1, pstSheet_in->stDirty.y1);
        //drv_disp_Printf("[sheet][x:%d,y:%d,w:%d,h:%d]\r\n", pstSheet_in->stSht.x, pstSheet_in->stSht.y, pstSheet_in->stSht.w, pstSheet_in->stSht.h);
        //CPUExt_CorePanic("[drv_gfx_BitBlt][Invaid calling]");
        return;
    }

    //drv_disp_Printf("[dirty.x0:%d][dirty.y0:%d][dirty.x1:%d][dirty.y1:%d]\r\n", pstSheet_in->stDirty.x0, pstSheet_in->stDirty.y0, pstSheet_in->stDirty.x1, pstSheet_in->stDirty.y1);
    //drv_disp_Printf("[sheet.x :%d][sheet.y :%d][sheet.w :%d][sheet.h :%d]\r\n", pstSheet_in->stSht.x, pstSheet_in->stSht.y, pstSheet_in->stSht.w, pstSheet_in->stSht.h);

    pbySrc     = pstSheet_in->pbyAddr;
    uiSrcPitch = pstSheet_in->uiPitch;

    if (pstSheet_in->stSht.x < -(pstSheet_in->stDirty.x0)) {
        uiSrcX = -(pstSheet_in->stSht.x);
        uiDstX = 0;
    } else {
        uiSrcX = pstSheet_in->stDirty.x0;
        uiDstX = pstSheet_in->stSht.x + pstSheet_in->stDirty.x0;
    }

    if ((pstSheet_in->stSht.x < -(pstSheet_in->stDirty.x0)) && (pstSheet_in->stSht.x > -(pstSheet_in->stDirty.x1))) {
        uiRegW = pstSheet_in->stDirty.x1 - pstSheet_in->stSht.x + 1;
    } else if ((pstSheet_in->stSht.x + pstSheet_in->stDirty.x1 > drv_gfx_stCtl.uiWidth)
               && (pstSheet_in->stSht.x + pstSheet_in->stDirty.x0 < drv_gfx_stCtl.uiWidth)) {
        uiRegW = drv_gfx_stCtl.uiWidth - pstSheet_in->stSht.x - pstSheet_in->stDirty.x0 + 1;
    } else if ((pstSheet_in->stSht.x >= -(pstSheet_in->stDirty.x0))
               && (pstSheet_in->stSht.x <= (drv_gfx_stCtl.uiWidth - pstSheet_in->stDirty.x1))) {
        uiRegW = pstSheet_in->stDirty.x1 - pstSheet_in->stDirty.x0 + 1;
    } else {
        uiRegW = 0;
    }

    if (pstSheet_in->stSht.y < -(pstSheet_in->stDirty.y0)) {
        uiSrcY = -(pstSheet_in->stSht.y);
        uiDstY = 0;
    } else {
        uiSrcY = pstSheet_in->stDirty.y0;
        uiDstY = pstSheet_in->stSht.y + pstSheet_in->stDirty.y0;
    }

    if ((pstSheet_in->stSht.y < -(pstSheet_in->stDirty.y0)) && (pstSheet_in->stSht.y > -(pstSheet_in->stDirty.y1))) {
        uiRegH = pstSheet_in->stDirty.y1 - pstSheet_in->stSht.y + 1;
    } else if ((pstSheet_in->stSht.y + pstSheet_in->stDirty.y1 > drv_gfx_stCtl.uiHeight)
               && (pstSheet_in->stSht.y + pstSheet_in->stDirty.y0 < drv_gfx_stCtl.uiHeight)) {
        uiRegH = drv_gfx_stCtl.uiHeight - pstSheet_in->stSht.y - pstSheet_in->stDirty.y0 + 1;
    } else if ((pstSheet_in->stSht.y >= -(pstSheet_in->stDirty.y0))
               && (pstSheet_in->stSht.y <= (drv_gfx_stCtl.uiHeight - pstSheet_in->stDirty.y1))) {
        uiRegH = pstSheet_in->stDirty.y1 - pstSheet_in->stDirty.y0 + 1;
    } else {
        uiRegH = 0;
    }

    if ((0 == uiRegW) || (0 == uiRegH)) {
        drv_disp_Printf("[src.x:%d][src.y:%d][dst.x:%d][dst.y:%d][Reg.w:%d][Reg.h:%d]\r\n", uiSrcX, uiSrcY, uiDstX, uiDstY, uiRegW, uiRegH);
        CPUExt_CorePanic("[drv_gfx_BitBlt][failed]");
        return;
    }

    for (i = 0; i < uiRegH; ++i) {
        for (j = 0; j < uiRegW; ++j) {
            if (0 != pbySrc[(uiSrcY + i) * uiSrcPitch + uiSrcX + j]) {
                pbyDst[(uiDstY + i) * uiDstPitch + uiDstX + j] = pbySrc[(uiSrcY + i) * uiSrcPitch + uiSrcX + j];
            }
        }
    }

    //drv_disp_Printf("[src.x:%d][src.y:%d][dst.x:%d][dst.y:%d][Reg.w:%d][Reg.h:%d]\r\n", uiSrcX, uiSrcY, uiDstX, uiDstY, uiRegW, uiRegH);
    stRegion.x0 = uiDstX;
    stRegion.y0 = uiDstY;
    stRegion.x1 = uiDstX + uiRegW - 1;
    stRegion.y1 = uiDstY + uiRegH - 1;
    drv_gfx_UpdateDirtyForScreen(&stRegion);

    pstSheet_in->stDirty.x0 = -1;
    pstSheet_in->stDirty.x1 = -1;
    pstSheet_in->stDirty.y0 = -1;
    pstSheet_in->stDirty.y1 = -1;

    //drv_disp_Printf("BitBlt success\r\n");

}

DRV_PRIVATE  void drv_gfx_DispData(DRV_GFX_SHEET_EXT * pstSheet_in, const DRV_GFX_DATA * pstData_in)
{
    CPU_INT32U   i      = 0;
    CPU_INT32U   j      = 0;
    CPU_INT08U * pbyBuf = 0;
    CPU_INT08U   uiData = 0;
    CPU_INT08U   uiByte = 0;
    DRV_GFX_REGION  stRegion;

    if ((0 == pstSheet_in)
            ||  (0 == pstData_in)
            ||  (0 == pstData_in->data)
            ||  (0 == pstData_in->w)
            ||  (0 != pstData_in->w % 8)
            ||  (0 == pstData_in->w / 8)
            ||  (0 == pstData_in->h)
            ||  (pstData_in->x < 0)
            ||  (pstData_in->y < 0)
            ||  (pstData_in->x + pstData_in->w > pstSheet_in->stSht.w)
            ||  (pstData_in->y + pstData_in->h > pstSheet_in->stSht.h)
            ||  (0 == pstData_in->bpp)) {
        return;
    }

    // just support one bit for one pixel currently
    uiByte = pstData_in->w / 8;

    for (i = 0; i < pstData_in->h; ++i) {
        pbyBuf = (CPU_INT08U *)(pstSheet_in->pbyAddr + (pstData_in->y + i) * pstSheet_in->uiPitch + pstData_in->x);

        for (j = 0; j < uiByte; ++j) {
            uiData  = pstData_in->data[i * uiByte + j];

            if (0 != (uiData & 0x80)) {
                pbyBuf[0 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x40)) {
                pbyBuf[1 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x20)) {
                pbyBuf[2 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x10)) {
                pbyBuf[3 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x08)) {
                pbyBuf[4 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x04)) {
                pbyBuf[5 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x02)) {
                pbyBuf[6 + j * 8] = pstSheet_in->uiColor;
            }

            if (0 != (uiData & 0x01)) {
                pbyBuf[7 + j * 8] = pstSheet_in->uiColor;
            }
        }
    }

    stRegion.x0 = pstData_in->x;
    stRegion.y0 = pstData_in->y;
    stRegion.x1 = pstData_in->x + pstData_in->w - 1;
    stRegion.y1 = pstData_in->y + pstData_in->h - 1;
    drv_gfx_UpdateDirtyForSheet(pstSheet_in, &stRegion);
}


DRV_PRIVATE  void drv_gfx_DispChar(DRV_GFX_SHEET_EXT * pstSheet_in, CPU_CHAR chAscii_in)
{
    static CPU_INT08U  s_uiAsciiState = 0;

    if (0 == pstSheet_in) {
        return;
    }

    switch (s_uiAsciiState) {
    case 0: {
        if ((chAscii_in > 31) && (chAscii_in < 127)) {
            DRV_GFX_DATA stData;

            if (pstSheet_in->stStrPos.x + pstSheet_in->stFont.w > pstSheet_in->stSht.w) {
                pstSheet_in->stStrPos.x = 0;
                drv_gfx_DispLineFeed(pstSheet_in);
            }

            stData.x = pstSheet_in->stStrPos.x;
            stData.y = pstSheet_in->stStrPos.y;
            stData.w = pstSheet_in->stFont.w;
            stData.h = pstSheet_in->stFont.h;
            stData.data = pstSheet_in->stFont.data + chAscii_in * pstSheet_in->stFont.bpc;
            stData.bpp  = 1;

            drv_gfx_DispData(pstSheet_in, &stData);

            pstSheet_in->stStrPos.x += pstSheet_in->stFont.w;
        } else if (7 == chAscii_in) {
            // beep
        } else if (8 == chAscii_in) {
            if (pstSheet_in->stStrPos.x > 0) {
                pstSheet_in->stStrPos.x -= pstSheet_in->stFont.w;
            }
        } else if (9 == chAscii_in) {
        } else if ((10 == chAscii_in) || (11 == chAscii_in) || (12 == chAscii_in)) {
            drv_gfx_DispLineFeed(pstSheet_in);
        } else if (13 == chAscii_in) {
            drv_gfx_DispCarriageReturn(pstSheet_in);
        } else if (27 == chAscii_in) {
            //s_uiAsciiState = 1;
        } else {
            // EMPTY
        }

        break;
    }

    default:
        break;
    } // end switch (s_uiAsciiState)

}

DRV_PRIVATE  void drv_gfx_DispLineFeed(DRV_GFX_SHEET_EXT * pstSheet_in)
{
    if (pstSheet_in->stStrPos.y + pstSheet_in->stFont.h < pstSheet_in->stSht.h) {
        pstSheet_in->stStrPos.y += pstSheet_in->stFont.h;
        return;
    }

    drv_gfx_DispScrollUp(pstSheet_in);
}

DRV_PRIVATE  void drv_gfx_DispCarriageReturn(DRV_GFX_SHEET_EXT * pstSheet_in)
{
    pstSheet_in->stStrPos.x = 0;
}

DRV_PRIVATE  void drv_gfx_DispScrollUp(DRV_GFX_SHEET_EXT * pstSheet_in)
{
    // TBD
}

#else  // (CPU_EXT_DISP_MODE_TEXT != CPU_EXT_DISP_MODE)

void drv_gfx_Init(void)
{
    return;
}

#endif // (CPU_EXT_DISP_MODE_TEXT != CPU_EXT_DISP_MODE)

