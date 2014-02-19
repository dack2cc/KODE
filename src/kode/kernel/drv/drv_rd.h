#ifndef __DRV_RD_H__
#define __DRV_RD_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu_ext.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#define DRV_RD_DEVICE  (0x100)

extern void drv_rd_Init(void);
extern void drv_rd_Setup(void);
extern void drv_rd_Request(CPU_EXT_HD_REQUEST * pstRequest_inout);

#endif // __DRV_RD_H__

