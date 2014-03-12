/******************************************************************************
    Include
******************************************************************************/

#include <GUI.h>

/******************************************************************************
    Private Define
******************************************************************************/



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

GUI_PRIVATE  void     gui_dev_MemDrawBitmap(GUI_DEVICE *  pDevice,  int x0, int y0, int xsize, int ysize, int BitsPerPixel, int BytesPerLine, const U8 GUI_UNI_PTR * pData, int Diff, const LCD_PIXELINDEX * pTrans)
{
}

GUI_PRIVATE  void     gui_dev_MemDrawHLine(GUI_DEVICE *  pDevice,  int x0, int y0,  int x1)
{
}

GUI_PRIVATE  void     gui_dev_MemDrawVLine(GUI_DEVICE *  pDevice,  int x , int y0,  int y1)
{
}

GUI_PRIVATE  void     gui_dev_MemFillRect(GUI_DEVICE *  pDevice,  int x0, int y0, int x1, int y1)
{
}

GUI_PRIVATE  unsigned gui_dev_MemGetPixelIndex(GUI_DEVICE *  pDevice,  int x, int y)
{
	return (0);
}

GUI_PRIVATE  void     gui_dev_MemSetPixelIndex(GUI_DEVICE *  pDevice,  int x, int y, int ColorIndex)
{
}

GUI_PRIVATE  void     gui_dev_MemXorPixel(GUI_DEVICE *  pDevice,  int x, int y)
{
}

GUI_PRIVATE  void     gui_dev_MemSetOrg(GUI_DEVICE *  pDevice,  int x, int y)
{
}


