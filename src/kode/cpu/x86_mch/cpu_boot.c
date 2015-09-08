
/*
*  C boot entrypoint - called by boot entry in cpu_head_s.S
*  Running in 32-bit flat mode, but without paging yet.
*/
void cpu_boot_entry(void* pBootInfo_in)
{
    cpu_disp_cnputc('y');
    cpu_disp_cnputc('u');
    cpu_disp_cnputc('K');
    cpu_disp_cnputc('i');

    while (1);
}

