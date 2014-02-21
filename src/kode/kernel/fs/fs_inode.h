#ifndef __FS_INODE_H__
#define __FS_INODE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#define FS_INODE_SIZE      (32)

typedef struct _FS_INODE {
	CPU_INT16U  i_mode;
	CPU_INT16U  i_uid;
	CPU_INT32U  i_size;
	CPU_INT32U  i_time;
	CPU_INT08U  i_gid;
	CPU_INT08U  i_nlinks;
	CPU_INT16U  i_zone[9];
} FS_INODE;

extern void fs_inode_Init(void);

#define FS_INODE_ROOT_NUM  (1)
extern  FS_INODE * fs_inode_Get(const CPU_INT16U uiDev_in, const CPU_INT16U uiNum_in);
extern  void fs_inode_Put(FS_INODE * pstInode_in);
extern  void fs_inode_Truncate(FS_INODE * pstInode_in);

#endif // __FS_INODE_H__

