
#ifndef __FS_H__
#define __FS_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

typedef struct _FS_INODE {
	CPU_INT16U  i_mode;
	CPU_INT16U  i_uid;
	CPU_INT32U  i_size;
	CPU_INT32U  i_time;
	CPU_INT08U  i_gid;
	CPU_INT08U  i_nlinks;
	CPU_INT16U  i_zone[9];
} FS_INODE;

typedef struct _FS_FILE {
	CPU_INT16U  f_mode;
	CPU_INT16U  f_flags;
	CPU_INT16U  f_count;
	FS_INODE *  f_inode;
	CPU_INT32S  f_pos;
} FS_FILE;

extern void FS_MountRoot(const CPU_INT16U  uiDev_in);
extern FS_FILE *  FS_GetFreeFileHandler(void);
extern CPU_INT32S FS_OpenNamei(const CPU_CHAR * pszName_in, const CPU_INT16U uiFlag_in, const CPU_INT16U uiMode_in, FS_INODE ** ppInode_out);

#endif // __FS_H__

