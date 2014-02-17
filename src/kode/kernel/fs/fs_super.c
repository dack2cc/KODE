
/******************************************************************************
    Include
******************************************************************************/

#include <fs_super.h>
#include <fs_inode.h>
#include <fs.h>
#include <drv_blk.h>
#include <os.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define FS_PRIVATE  static
#define FS_PRIVATE  

typedef struct _FS_SUPER_DISK_BLOCK {
	CPU_INT16U  s_ninodes;
	CPU_INT16U  s_nzones;
	CPU_INT16U  s_imap_blocks;
	CPU_INT16U  s_zmap_blocks;
	CPU_INT16U  s_firstdatazone;
	CPU_INT16U  s_log_zone_size;
	CPU_INT32U  s_max_size;
	CPU_INT16U  s_magic;
} FS_SUPER_DISK_BLOCK;

typedef struct _FS_SUPER_BLOCK {
	FS_SUPER_DISK_BLOCK  dk;
	
    /* These are only in memory */
	DRV_BLK_BUFFER * s_imap[8];
	DRV_BLK_BUFFER * s_zmap[8];
	CPU_INT16U       s_dev;
	//struct m_inode * s_isup;
	//struct m_inode * s_imount;
	CPU_INT32U       s_time;
	//struct task_struct * s_wait;
	CPU_INT08U       s_lock;
	CPU_INT08U       s_rd_only;
	CPU_INT08U       s_dirty;
} FS_SUPER_BLOCK;

#define FS_SUPER_BLOCK_MAX  (8)
FS_PRIVATE FS_SUPER_BLOCK fs_super_astBlock[FS_SUPER_BLOCK_MAX];


typedef struct _FS_SUPER_FILE {
	CPU_INT16U f_mode;
	CPU_INT16U f_flags;
	CPU_INT16U f_count;
	//struct m_inode * f_inode;
	CPU_INT32S f_pos;
} FS_SUPER_FILE;

#define FS_SUPER_FILE_MAX  (64)
FS_PRIVATE FS_SUPER_FILE fs_super_astFile[FS_SUPER_FILE_MAX];

/******************************************************************************
    Private Interface
******************************************************************************/

FS_PRIVATE  FS_SUPER_BLOCK * fs_super_Read(const CPU_INT16U  uiDev_in);
FS_PRIVATE  void fs_super_CheckDiskChange(const CPU_INT16U  uiDev_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void FS_super_MountRoot(const CPU_INT16U  uiDev_in)
{
	CPU_INT32U i = 0;
	FS_SUPER_BLOCK * pstSuper = 0;
	
	if (FS_INODE_DISK_SIZE != sizeof(FS_INODE_DISK)) {
		CPUExt_CorePanic("[FS_super_MountRoot][bad i-node size]");
	}
	for (i = 0; i < FS_SUPER_FILE_MAX; ++i) {
		fs_super_astFile[i].f_count = 0;
	}
	for (pstSuper = &(fs_super_astBlock[0]); pstSuper < &(fs_super_astBlock[FS_SUPER_BLOCK_MAX]); ++pstSuper) {
		pstSuper->s_dev = 0;
		pstSuper->s_lock = 0;
		
	}
	//if 
}

FS_PRIVATE  FS_SUPER_BLOCK * fs_super_Read(const CPU_INT16U  uiDev_in)
{
	if (0 == uiDev_in) {
		return 0;
	}
	fs_super_CheckDiskChange(uiDev_in);
	
	return 0;
}


FS_PRIVATE  void fs_super_CheckDiskChange(const CPU_INT16U  uiDev_in)
{
	/*
	 * This routine checks whether a floppy has been changed, and
	 * invalidates all buffer-cache-entries in that case. This
	 * is a relatively slow routine, so we have to try to minimize using
	 * it. Thus it is called only upon a 'mount' or 'open'. This
	 * is the best way of combining speed and utility, I think.
	 * People changing diskettes in the middle of an operation deserve
	 * to loose :-)
	 *
	 * NOTE! Although currently this is only for floppies, the idea is
	 * that any additional removable block-device will use this routine,
	 * and that mount/open needn't know that floppies/whatever are
	 * special.
	 */
}
