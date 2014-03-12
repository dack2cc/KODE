
#ifndef __GUI_CONF_H__
#define __GUI_CONF_H__

/******************************************************************************
    Include
******************************************************************************/

#include <lib_mem.h>

/******************************************************************************
    Define
******************************************************************************/

#define GUI_PRIVATE 
//#define GUI_PRIVATE  static

#define GUI_SUPPORT_TOUCH    0
#define GUI_SUPPORT_MOUSE    1
#define GUI_SUPPORT_MEMDEV   0

#define GUI_OS               0
#define GUI_NUM_LAYERS       32
#define GUI_CURSOR_LAYER     1
#define GUI_SUPPORT_ROTATION 0

#define GUI_MEMCPY(pDest, pSrc, NumBytes) Mem_Copy(pDest, pSrc, NumBytes)
#define GUI_MEMSET(pDest, val, NumBytes)  Mem_Set(pDest, val, NumBytes)


/******************************************************************************
    Public Interface
******************************************************************************/

#endif // __GUI_CONF_H__

