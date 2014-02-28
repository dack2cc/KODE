#ifndef __FS_INODE_H__
#define __FS_INODE_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <fs.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#define FS_INODE_SIZE      (32)

extern void fs_inode_Init(void);

#define FS_INODE_ROOT_NUM  (1)
extern  FS_INODE * fs_inode_Get(const CPU_INT16U uiDev_in, const CPU_INT16U uiNum_in);
extern  void fs_inode_Put(FS_INODE * pstInode_in);
extern  void fs_inode_Truncate(FS_INODE * pstInode_in);

extern  FS_INODE * fs_inode_New(const CPU_INT16U uiDev_in);
extern  void fs_inode_Free(FS_INODE * pstInode_inout);

extern  void fs_inode_AddRef(FS_INODE * pstInode_inout);

#endif // __FS_INODE_H__

