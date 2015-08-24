#ifndef __DRV_MICE_H__
#define __DRV_MICE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

extern void drv_mice_Init(void);

typedef struct _DRV_MICE_EVENT {
    CPU_INT08U  uiButton;
    CPU_INT32S  iOffsetX;
    CPU_INT32S  iOffsetY;
} DRV_MICE_EVENT;

extern void drv_mice_RegisterHandler(CPU_FNCT_PTR pfnMice_in);

#endif // __DRV_MICE_H__

