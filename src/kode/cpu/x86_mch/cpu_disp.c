
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_disp.h>
#include <cpu_vm_param.h>
#include <lib_mem.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define CPU_DISP_LINE_CHAR_MAX   (80)

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE CPU_VOID cpu_disp_Cnputc(const CPU_CHAR iChar_in);

/******************************************************************************
    Function Definition
******************************************************************************/

CPU_INT32U
CPUExt_DispPrint(const CPU_CHAR* pszStr_in)
{
    CPU_CHAR*  pbyChar  = 0;
    CPU_INT32U iCharCnt = 0;

    if (0 == pszStr_in) {
        return (0);
    }

    pbyChar = (CPU_CHAR *)pszStr_in;

    while ('\0' != (*pbyChar)) {
        cpu_disp_Cnputc((*pbyChar));
        ++pbyChar;
        ++iCharCnt;
    }

    return (iCharCnt);
}

void
CPUExt_DispChar(const  CPU_CHAR  chAscii_in)
{
    cpu_disp_Cnputc(chAscii_in);
}


CPU_PRIVATE CPU_VOID
cpu_disp_Cnputc(const CPU_CHAR iChar_in)
{
    static CPU_INT32S s_iOffset = -1;

    if ((s_iOffset < 0) || (s_iOffset >= 80)) {
        s_iOffset = 0;
        cpu_disp_Cnputc('\n');
    }

    if (iChar_in == '\n') {
        Mem_Move(
            (void *)phystokv(0xb8000),
            (void *)phystokv(0xb8000 + 80 * 2),
            80 * 2 * 24
        );
        Mem_Set(
            (void *)phystokv((0xb8000 + 80 * 2 * 24)),
            0x00,
            80 * 2
        );
    } else if (iChar_in == '\r') {
        s_iOffset = 0;
    } else {
        volatile CPU_CHAR* pChar = 0;

        if (s_iOffset >= 80) {
            cpu_disp_Cnputc('\r');
            cpu_disp_Cnputc('\n');
        }

        pChar = (void *) phystokv(0xb8000 + 80 * 2 * 24 + s_iOffset * 2);
        pChar[0] = iChar_in;
        pChar[1] = 0x0f;

        s_iOffset++;
    }
}

