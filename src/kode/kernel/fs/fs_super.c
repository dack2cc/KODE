
/******************************************************************************
    Include
******************************************************************************/

#include <fs_super.h>
#include <fs_inode.h>
#include <fs.h>
#include <drv_blk.h>
#include <drv_lock.h>
#include <os.h>
#include <cpu_ext.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define FS_PRIVATE  static
#define FS_PRIVATE  

#define FS_SUPER_LCK_NAME_WAIT  "[FSSuper][wait]"
#define FS_SUPER_MAGIC          (0x137F)
#define FS_SUPER_I_MAP_SLOTS    (8)
#define FS_SUPER_Z_MAP_SLOTS    (8)

typedef struct _FS_SUPER_BLOCK_EXT {
	FS_SUPER_BLOCK   sb;
	
    /* These are only in memory */
	DRV_BLK_BUFFER * s_imap[FS_SUPER_I_MAP_SLOTS];
	DRV_BLK_BUFFER * s_zmap[FS_SUPER_Z_MAP_SLOTS];
	CPU_INT16U       s_dev;
	FS_INODE *       s_isup;
	FS_INODE *       s_imount;
	CPU_INT32U       s_time;
	DRV_LOCK         s_wait;
	CPU_INT08U       s_lock;
	CPU_INT08U       s_rd_only;
	CPU_INT08U       s_dirty;
} FS_SUPER_BLOCK_EXT;

#define FS_SUPER_BLOCK_EXT_MAX  (8)
FS_PRIVATE FS_SUPER_BLOCK_EXT fs_super_astBlock[FS_SUPER_BLOCK_EXT_MAX];


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

FS_PRIVATE  void fs_super_Wait(FS_SUPER_BLOCK_EXT * pstSuperExt_in);
FS_PRIVATE  void fs_super_Lock(FS_SUPER_BLOCK_EXT * pstSuperExt_in);
FS_PRIVATE  void fs_super_Free(FS_SUPER_BLOCK_EXT * pstSuperExt_in);

FS_PRIVATE  FS_SUPER_BLOCK_EXT * fs_super_Read(const CPU_INT16U  uiDev_in);
FS_PRIVATE  void fs_super_CheckDiskChange(const CPU_INT16U  uiDev_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void FS_super_MountRoot(const CPU_INT16U  uiDev_in)
{
	CPU_INT32U i = 0;
	FS_SUPER_BLOCK_EXT * pstSuperExt = 0;
	
	if (FS_INODE_SIZE != sizeof(FS_INODE)) {
		CPUExt_CorePanic("[FS_super_MountRoot][bad i-node size]");
	}
	for (i = 0; i < FS_SUPER_FILE_MAX; ++i) {
		fs_super_astFile[i].f_count = 0;
	}
	for (pstSuperExt = &(fs_super_astBlock[0]); pstSuperExt < &(fs_super_astBlock[FS_SUPER_BLOCK_EXT_MAX]); ++pstSuperExt) {
		pstSuperExt->s_dev = 0;
		pstSuperExt->s_lock = 0;
		drv_lock_Init(&(pstSuperExt->s_wait), FS_SUPER_LCK_NAME_WAIT);
	}
	
	pstSuperExt = fs_super_Read(uiDev_in);
	if (0 == pstSuperExt) {
		CPUExt_CorePanic("[FS_super_MountRoot][Unable to mount root]");
	}
}

FS_SUPER_BLOCK * fs_super_Get(const CPU_INT16U uiDev_in)
{
	FS_SUPER_BLOCK_EXT * pstSuperExt = 0;
	
	if (0 == uiDev_in) {
		return 0;
	}
	pstSuperExt = 0 + fs_super_astBlock;
	while (pstSuperExt < (fs_super_astBlock + FS_SUPER_BLOCK_EXT_MAX)) {
		if (pstSuperExt->s_dev == uiDev_in) {
			fs_super_Wait(pstSuperExt);
			if (pstSuperExt->s_dev == uiDev_in) {
				return ((FS_SUPER_BLOCK *)pstSuperExt);
			}
			pstSuperExt = 0 + fs_super_astBlock;
		}
		else {
			pstSuperExt++;
		}
	}
	return 0;
}

FS_PRIVATE  void fs_super_Wait(FS_SUPER_BLOCK_EXT * pstSuperExt_in)
{
	CPU_SR_ALLOC();
	
	if (0 == pstSuperExt_in) {
		CPUExt_CorePanic("[fs_super_WaitOn][invalid super]");
	}
	
	OS_CRITICAL_ENTER();
	while (pstSuperExt_in->s_lock) {
		drv_lock_SleepOn(&(pstSuperExt_in->s_wait));
	}
    OS_CRITICAL_EXIT();
}

FS_PRIVATE  void fs_super_Lock(FS_SUPER_BLOCK_EXT * pstSuperExt_in)
{
	CPU_SR_ALLOC();
	
	if (0 == pstSuperExt_in) {
		CPUExt_CorePanic("[fs_super_WaitOn][invalid super]");
	}
	
	OS_CRITICAL_ENTER();
	while (pstSuperExt_in->s_lock) {
		drv_lock_SleepOn(&(pstSuperExt_in->s_wait));
	}
	pstSuperExt_in->s_lock = 1;
    OS_CRITICAL_EXIT();
	
}

FS_PRIVATE  void fs_super_Free(FS_SUPER_BLOCK_EXT * pstSuperExt_in)
{
	CPU_SR_ALLOC();
	
	if (0 == pstSuperExt_in) {
		CPUExt_CorePanic("[fs_super_WaitOn][invalid super]");
	}
	
	OS_CRITICAL_ENTER();
	pstSuperExt_in->s_lock = 0;
	drv_lock_WakeUp(&(pstSuperExt_in->s_wait));
    OS_CRITICAL_EXIT();
}


FS_PRIVATE  FS_SUPER_BLOCK_EXT * fs_super_Read(const CPU_INT16U  uiDev_in)
{
	FS_SUPER_BLOCK_EXT * pstSuperExt = 0;
    DRV_BLK_BUFFER *     pstBuffer   = 0;
	CPU_INT16U  i       = 0;
	CPU_INT16U  uiBlock = 0;
	
	if (0 == uiDev_in) {
		return 0;
	}
	fs_super_CheckDiskChange(uiDev_in);
	
	pstSuperExt = (FS_SUPER_BLOCK_EXT *)fs_super_Get(uiDev_in);
	if (0 != pstSuperExt) {
		return (pstSuperExt);
	}
	
	for (pstSuperExt = 0+fs_super_astBlock;;++pstSuperExt) {
		if (pstSuperExt >= fs_super_astBlock + FS_SUPER_BLOCK_EXT_MAX) {
			return 0;
	    }
		if (0 == pstSuperExt->s_dev) {
			break;
		}
	}
	pstSuperExt->s_dev = uiDev_in;
	pstSuperExt->s_isup = 0;
	pstSuperExt->s_imount = 0;
	pstSuperExt->s_time = 0;
	pstSuperExt->s_rd_only = 0;
	pstSuperExt->s_dirty = 0;
	fs_super_Lock(pstSuperExt);
	
	pstBuffer = drv_blk_Read(uiDev_in, 1);
	if (0 == pstBuffer) {
		pstSuperExt->s_dev = 0;
		fs_super_Free(pstSuperExt);
		CPUExt_DispPrint("[fs_super_Read][block read failed] \r\n");
		return 0;
	}
	*((FS_SUPER_BLOCK *)pstSuperExt) = *((FS_SUPER_BLOCK *)(pstBuffer->pbyData));
	drv_blk_Release(pstBuffer);
	
	if (FS_SUPER_MAGIC != pstSuperExt->sb.s_magic) {
		pstSuperExt->s_dev = 0;
		fs_super_Free(pstSuperExt);
		CPUExt_DispPrint("[fs_super_Read][magic number invalid] \r\n");
		return 0;
	}
	
	for (i = 0; i < FS_SUPER_I_MAP_SLOTS; ++i) {
		pstSuperExt->s_imap[i] = 0;
	}
	for (i = 0; i < FS_SUPER_Z_MAP_SLOTS; ++i) {
		pstSuperExt->s_zmap[i] = 0;
	}
	
	uiBlock = 2;
	for (i = 0; i < pstSuperExt->sb.s_imap_blocks; ++i) {
		if (0 != ((pstSuperExt->s_imap[i]) = drv_blk_Read(uiDev_in, uiBlock))) {
			uiBlock++;
		}
		else {
			break;
		}
	}
	for (i = 0; i < pstSuperExt->sb.s_zmap_blocks; ++i) {
		if (0 != ((pstSuperExt->s_zmap[i]) = drv_blk_Read(uiDev_in, uiBlock))) {
			uiBlock++;
		}
		else {
			break;
		}
	}
	if (uiBlock != (2 + pstSuperExt->sb.s_imap_blocks + pstSuperExt->sb.s_zmap_blocks)) {
		for (i = 0; i < FS_SUPER_I_MAP_SLOTS; ++i) {
			drv_blk_Release(pstSuperExt->s_imap[i]);
		}
		for (i = 0; i < FS_SUPER_Z_MAP_SLOTS; ++i) {
			drv_blk_Release(pstSuperExt->s_zmap[i]);
		}
		pstSuperExt->s_dev = 0;
		fs_super_Free(pstSuperExt);
		return 0;
	}
	
	pstSuperExt->s_imap[0]->pbyData[0] |= 1;
	pstSuperExt->s_zmap[0]->pbyData[0] |= 1;
	fs_super_Free(pstSuperExt);
	return (pstSuperExt);
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
