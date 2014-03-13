/******************************************************************************
    Include
******************************************************************************/

#include <GUI.h>

/******************************************************************************
    Private Define
******************************************************************************/

GUI_PRIVATE GUI_DIRTYDEVICE_INFO * gui_dev_apstDirty[GUI_NUM_LAYERS] = {0,};

/******************************************************************************
    Private Interface
******************************************************************************/

GUI_PRIVATE  void     gui_dev_MemDrawBitmap(GUI_DEVICE *  pDevice,  int x0, int y0, int xsize, int ysize, int BitsPerPixel, int BytesPerLine, const U8 GUI_UNI_PTR * pData, int Diff, const LCD_PIXELINDEX * pTrans);
GUI_PRIVATE  void     gui_dev_MemDrawHLine(GUI_DEVICE *  pDevice,  int x0, int y0,  int x1);
GUI_PRIVATE  void     gui_dev_MemDrawVLine(GUI_DEVICE *  pDevice,  int x , int y0,  int y1);
GUI_PRIVATE  void     gui_dev_MemFillRect(GUI_DEVICE *  pDevice,  int x0, int y0, int x1, int y1);
GUI_PRIVATE  unsigned gui_dev_MemGetPixelIndex(GUI_DEVICE *  pDevice,  int x, int y);
GUI_PRIVATE  void     gui_dev_MemSetPixelIndex(GUI_DEVICE *  pDevice,  int x, int y, int ColorIndex);
GUI_PRIVATE  void     gui_dev_MemXorPixel(GUI_DEVICE *  pDevice,  int x, int y);
GUI_PRIVATE  void     gui_dev_MemSetOrg(GUI_DEVICE *  pDevice,  int x, int y);


const GUI_DEVICE_API GUI_MEMDEV_DEVICE_8 = {
	/* DeviceClassIndex */ DEVICE_CLASS_MEMDEV,
	/* pfDrawBitmap     */ gui_dev_MemDrawBitmap,
	/* pfDrawHLine      */ gui_dev_MemDrawHLine,
	/* pfDrawVLine      */ gui_dev_MemDrawVLine,
	/* pfFillRect       */ gui_dev_MemFillRect,
	/* pfGetPixelIndex  */ gui_dev_MemGetPixelIndex,
	/* pfSetPixelIndex  */ gui_dev_MemSetPixelIndex,
	/* pfXorPixel       */ gui_dev_MemXorPixel,
	/* pfSetOrg         */ gui_dev_MemSetOrg,
	/* pfGetDevFunc     */ 0,
	/* pfGetDevProp     */ 0,
	/* pfGetDevData     */ 0,
	/* pfGetRect        */ 0,
};

/******************************************************************************
    Function Definition
******************************************************************************/

GUI_PRIVATE  void     
gui_dev_MemDrawBitmap(GUI_DEVICE *  pDevice,  int x0, int y0, int xsize, int ysize, int BitsPerPixel, int BytesPerLine, const U8 GUI_UNI_PTR * pData, int Diff, const LCD_PIXELINDEX * pTrans)
{
}

GUI_PRIVATE  void     
gui_dev_MemDrawHLine(GUI_DEVICE *  pDevice,  int x0, int y0,  int x1)
{
	U8 uiPenSize = GUI_GetPenSize();
	
	if (0 == uiPenSize) {
		return;
	}
	
	gui_dev_MemFillRect(pDevice, x0, y0, x1, y0 + uiPenSize);
}

GUI_PRIVATE  void     
gui_dev_MemDrawVLine(GUI_DEVICE *  pDevice,  int x , int y0,  int y1)
{
	U8 uiPenSize = GUI_GetPenSize();
	
	if (0 == uiPenSize) {
		return;
	}
	
	gui_dev_MemFillRect(pDevice, x, y0, x + uiPenSize, y1);
}

GUI_PRIVATE  void     
gui_dev_MemFillRect(GUI_DEVICE *  pDevice,  int x0, int y0, int x1, int y1)
{
	int color = GUI_GetColorIndex();
	int y     = 0;
	int w     = GUI_GetScreenSizeX();
	int h     = GUI_GetScreenSizeY();
	int bbp   = LCD_GetBitsPerPixelMax();
	int pitch = w * bbp / 8;
	
	if ((color < 0) 
	||  (0 == pDevice) 
	||  (0 == pDevice->u.pContext)
	||  (pDevice->LayerIndex < 0)
	||  (pDevice->LayerIndex >= GUI_NUM_LAYERS)
	||  (w <= 0)
	||  (h <= 0)
	||  (pitch <= 0)) {
		return;
	}
	
	if (x1 >= w) {
		x1 = w - 1;
	}
	if (y1 >= h) {
		y1 = h - 1;
	}
	
	if ((x0 < 0) || (x0 >= x1) || (y0 < 0) || (y0 >= y1)){
		return;
	}
	
	for (y = y0; y <= y1; ++y) {
		Mem_Set(
			(pDevice->u.pContext + y * pitch + x0),
			color,
			(x1 - x0) * bbp / 8
		);
	}
	
	//gui_dev_apstDirty[GUI_NUM_LAYERS]
}

GUI_PRIVATE  unsigned 
gui_dev_MemGetPixelIndex(GUI_DEVICE *  pDevice,  int x, int y)
{
	return (0);
}

GUI_PRIVATE  void     
gui_dev_MemSetPixelIndex(GUI_DEVICE *  pDevice,  int x, int y, int ColorIndex)
{
}

GUI_PRIVATE  void     
gui_dev_MemXorPixel(GUI_DEVICE *  pDevice,  int x, int y)
{
}

GUI_PRIVATE  void     
gui_dev_MemSetOrg(GUI_DEVICE *  pDevice,  int x, int y)
{
}


