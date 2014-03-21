#ifndef __GUI_LOG_H__
#define __GUI_LOG_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <drv_gfx.h>

/******************************************************************************
    Public Definition
******************************************************************************/


/******************************************************************************
    Public Interface
******************************************************************************/

extern void gui_log_Init(void);
extern void gui_log_Open(const DRV_GFX_POINT * pstPos_in);
extern void gui_log_Close(void);
extern void gui_log_Update(void);

#endif // __GUI_LOG_H__

