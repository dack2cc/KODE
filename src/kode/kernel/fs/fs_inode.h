#ifndef __FS_INODE_H__
#define __FS_INODE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#define FS_INODE_SIZE  (32)

typedef struct _FS_INODE {
	CPU_INT16U  i_mode;
	CPU_INT16U  i_uid;
	CPU_INT32U  i_size;
	CPU_INT32U  i_time;
	CPU_INT08U  i_gid;
	CPU_INT08U  i_nlinks;
	CPU_INT16U  i_zone[9];
} FS_INODE;

#endif // __FS_INODE_H__

