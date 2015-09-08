
#include <cpu_disp.h>
#include <cpu_vm_param.h>
#include <lib_mem.h>

#define CPU_DISP_LINE_CHAR_MAX   (80)

CPU_PRIVATE CPU_VOID cpu_disp_cnputc(CPU_DATA iChar_in);



CPU_PRIVATE CPU_VOID
cpu_disp_cnputc(CPU_DATA iChar_in)
{
    static CPU_INT32S s_iOffset = -1;

    if ((s_iOffset < 0) || (s_iOffset >= 80)) {
        s_iOffset = 0;
        cpu_disp_cnputc('\n');
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
    } else {
        volatile CPU_CHAR* pChar = 0;

        if (s_iOffset >= 80) {
            cpu_disp_cnputc('\r');
            cpu_disp_cnputc('\n');
        }

        pChar = (void *) phystokv(0xb8000 + 80 * 2 * 24 + s_iOffset * 2);
        pChar[0] = iChar_in;
        pChar[1] = 0x0f;

        s_iOffset++;
    }
}

