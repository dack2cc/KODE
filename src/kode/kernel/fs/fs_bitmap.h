#ifndef __FS_BITMAP_H__
#define __FS_BITMAP_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>
#include <fs_inode.h>

/******************************************************************************
    Public Interface
******************************************************************************/

void fs_bitmap_FreeBlock(const CPU_INT16U uiDev_in, const CPU_INT32U uiBlk_in);
void fs_bitmap_FreeInode(FS_INODE * pstInode_in);

#endif // __FS_BITMAP_H__

