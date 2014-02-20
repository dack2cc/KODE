
#ifndef __FS_SUPER_H__
#define __FS_SUPER_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <fs_inode.h>

/******************************************************************************
    Public Interface
******************************************************************************/

typedef struct _FS_SUPER_BLOCK {
	CPU_INT16U  s_ninodes;
	CPU_INT16U  s_nzones;
	CPU_INT16U  s_imap_blocks;
	CPU_INT16U  s_zmap_blocks;
	CPU_INT16U  s_firstdatazone;
	CPU_INT16U  s_log_zone_size;
	CPU_INT32U  s_max_size;
	CPU_INT16U  s_magic;
} FS_SUPER_BLOCK;

FS_SUPER_BLOCK * fs_super_Get(const CPU_INT16U uiDev_in);
FS_SUPER_BLOCK * fs_super_FindMount(FS_INODE * pstInode_in);

#endif // __FS_SUPER_H__

