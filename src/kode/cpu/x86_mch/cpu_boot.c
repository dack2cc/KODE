
/*
*  C boot entrypoint - called by boot entry in cpu_head_s.S
*  Running in 32-bit flat mode, but without paging yet.
*/
void cpu_boot_entry(void* pBootInfo_in)
{
    while (1);
}

