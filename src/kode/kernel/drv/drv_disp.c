
/******************************************************************************
    Include
******************************************************************************/

#include <drv_disp.h>
#include <cpu_ext.h>
#include <std/stdio.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define DRV_PRIVATE static
#define DRV_PRIVATE

#define DRV_DISP_BUFFER_MAX   (1024)

DRV_PRIVATE  CPU_CHAR  drv_disp_aszBuffer[DRV_DISP_BUFFER_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

void drv_disp_Init(void)
{
    CPU_INT32U i = 0;

    for (i = 0; i < DRV_DISP_BUFFER_MAX; ++i) {
        drv_disp_aszBuffer[i] = '\0';
    }

    return;
}


CPU_INT32S  drv_disp_Printf(const CPU_CHAR* fmt_in, ...)
{
    va_list    args;
    CPU_INT32S i = 0;

    va_start(args, fmt_in);
    i = vsprintf(drv_disp_aszBuffer, fmt_in, args);
    CPUExt_DispPrint(drv_disp_aszBuffer);
    va_end(args);

    return (i);
}

