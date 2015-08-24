
/******************************************************************************
    Include
******************************************************************************/

#include <fs_inode.h>
#include <fs_super.h>
#include <drv_blk.h>
#include <drv_lock.h>
#include <drv_disp.h>
#include <os.h>
#include <cpu_ext.h>
#include <lib_mem.h>
#include <sys/stat.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define FS_PRIVATE  static
#define FS_PRIVATE

#define FS_INODE_DEBUG_OFF (0)
#define FS_INODE_DEBUG_ON  (1)
#define FS_INODE_DEBUG     (FS_INODE_DEBUG_OFF)
//#define FS_INODE_DEBUG     (FS_INODE_DEBUG_ON)

#define FS_INODE_NAME_LOCK  "[inode][wait]"
#define FS_INODE_PER_BLOCK  (CPU_EXT_HD_BLOCK_SIZE / FS_INODE_SIZE)

typedef struct _FS_INODE_EXT {
    FS_INODE   ind;

    /* These are only in memory */
    CPU_INT32U i_atime;
    CPU_INT32U i_ctime;
    CPU_INT16U i_dev;
    CPU_INT16U i_num;
    CPU_INT16U i_count;
    CPU_INT08U i_dirt;
    CPU_INT08U i_pipe;
    CPU_INT08U i_mount;
    CPU_INT08U i_seek;
    CPU_INT08U i_update;
    CPU_INT08U i_lock;
    DRV_LOCK   i_wait;
} FS_INODE_EXT;

#define FS_INODE_EXT_MAX  (32)
FS_PRIVATE FS_INODE_EXT fs_inode_astTbl[FS_INODE_EXT_MAX] = {{{0,},},};

/******************************************************************************
    Private Interface
******************************************************************************/

FS_PRIVATE FS_INODE_EXT * fs_inode_GetEmpty(void);

FS_PRIVATE void fs_inode_WaitOn(FS_INODE_EXT* pstInode_in);
FS_PRIVATE void fs_inode_Lock(FS_INODE_EXT * pstInode_inout);
FS_PRIVATE void fs_inode_Unlock(FS_INODE_EXT * pstInode_inout);

FS_PRIVATE void fs_inode_Write(FS_INODE_EXT* pstInode_in);
FS_PRIVATE void fs_inode_Read(FS_INODE_EXT* pstInode_inout);
FS_PRIVATE void fs_inode_Sync(void);

FS_PRIVATE void fs_inode_FreeIndirect(const CPU_INT16U uiDev_in, const CPU_INT32U uiBlk_in);
FS_PRIVATE void fs_inode_FreeDoubleIndirect(const CPU_INT16U uiDev_in, const CPU_INT32U uiBlk_in);


/******************************************************************************
    Function Definition
******************************************************************************/

void fs_inode_Init(void)
{
    CPU_INT32U  i = 0;

    drv_blk_RegisterSync(fs_inode_Sync);

    for (i = 0; i < FS_INODE_EXT_MAX; ++i) {
        drv_lock_Init(&(fs_inode_astTbl[i].i_wait), FS_INODE_NAME_LOCK);
        fs_inode_astTbl[i].i_count = 0;
    }
}

FS_INODE * fs_inode_Get(const CPU_INT16U uiDev_in, const CPU_INT16U uiNum_in)
{
    FS_INODE_EXT *   pstInode = 0;
    FS_INODE_EXT *   pstEmpty = 0;
    FS_SUPER_BLOCK * pstMount = 0;
    CPU_INT16U uiDev = uiDev_in;
    CPU_INT16U uiNum = uiNum_in;

    if (0 == uiDev_in) {
        CPUExt_CorePanic("[fs_inode_Get][device invalid]");
        return (0);
    }

    pstEmpty = fs_inode_GetEmpty();
    pstInode = fs_inode_astTbl;

    while (pstInode < (fs_inode_astTbl + FS_INODE_EXT_MAX)) {
        if ((pstInode->i_dev != uiDev_in) || (pstInode->i_num != uiNum_in)) {
            pstInode++;
            continue;
        }

        fs_inode_WaitOn(pstInode);

        if ((pstInode->i_dev != uiDev_in) || (pstInode->i_num != uiNum_in)) {
            pstInode = fs_inode_astTbl;
            continue;
        }

        pstInode->i_count++;

        if (pstInode->i_mount) {
            pstMount = fs_super_FindMount((FS_INODE *)pstInode);

            if (0 == pstMount) {
                drv_disp_Printf("[fs_inode_Get][Mounted inode has not got super block] \r\n");

                if (0 != pstEmpty) {
                    fs_inode_Put((FS_INODE *)pstEmpty);
                }

                return ((FS_INODE *)pstInode);
            }

            fs_inode_Put((FS_INODE *)pstInode);
            uiDev = 0;
            uiNum = FS_INODE_ROOT_NUM;
            pstInode = fs_inode_astTbl;
            continue;
        }

        if (0 != pstEmpty) {
            fs_inode_Put((FS_INODE *)pstEmpty);
        }

        return ((FS_INODE *)pstInode);
    }

    if (0 == pstEmpty) {
        return (0);
    }

    pstInode = pstEmpty;
    pstInode->i_dev = uiDev;
    pstInode->i_num = uiNum;
    fs_inode_Read(pstInode);

    return ((FS_INODE *)pstInode);
}

void fs_inode_Put(FS_INODE * pstInode_in)
{
    FS_INODE_EXT * pstInode = (FS_INODE_EXT *)pstInode_in;

    if (0 == pstInode) {
        return;
    }

    fs_inode_WaitOn(pstInode);

    if (0 == pstInode->i_count) {
        CPUExt_CorePanic("[fs_inode_Put][try to free free inode]");
    }

    if (pstInode->i_pipe) {
        drv_lock_WakeUp(&(pstInode->i_wait));

        if (--(pstInode->i_count)) {
            return;
        }

        CPUExt_PageRelease(pstInode->ind.i_size);
        pstInode->i_count = 0;
        pstInode->i_dirt  = 0;
        pstInode->i_pipe  = 0;
        return;
    }

    if (!pstInode->i_dev) {
        pstInode->i_count--;
        return;
    }

    if (S_ISBLK(pstInode->ind.i_mode)) {
        drv_blk_SyncDevice((pstInode->ind.i_zone[0]));
        fs_inode_WaitOn(pstInode);
    }

L_REPEAT_PUT:

    if (pstInode->i_count > 1) {
        pstInode->i_count--;
        return;
    }

    if (!(pstInode->ind.i_nlinks)) {
        fs_inode_Truncate((FS_INODE *)pstInode);
        fs_inode_Free((FS_INODE *)pstInode);
        return;
    }

    if (pstInode->i_dirt) {
        fs_inode_Write(pstInode);
        fs_inode_WaitOn(pstInode);
        goto L_REPEAT_PUT;
    }

    pstInode->i_count--;
    return;
}

void fs_inode_Truncate(FS_INODE * pstInode_in)
{
    FS_INODE_EXT * pstInode = (FS_INODE_EXT *)pstInode_in;
    CPU_INT32U  i = 0;

    if (0 == pstInode) {
        CPUExt_CorePanic("[fs_inode_Truncate][i-node invalid]");
    }

    if (!S_ISREG(pstInode->ind.i_mode) || S_ISDIR(pstInode->ind.i_mode)) {
        return;
    }

    for (i = 0; i < 7; ++i) {
        if (pstInode->ind.i_zone[i]) {
            fs_super_FreeBlock(pstInode->i_dev, pstInode->ind.i_zone[i]);
            pstInode->ind.i_zone[i] = 0;
        }
    }

    fs_inode_FreeIndirect(pstInode->i_dev, pstInode->ind.i_zone[7]);
    fs_inode_FreeDoubleIndirect(pstInode->i_dev, pstInode->ind.i_zone[8]);
    pstInode->ind.i_zone[7] = 0;
    pstInode->ind.i_zone[8] = 0;
    pstInode->ind.i_size = 0;
    pstInode->i_dirt = 1;
    CPUExt_TimeCurrent(&(pstInode->ind.i_time));
    pstInode->i_ctime = pstInode->ind.i_time;
}

FS_INODE * fs_inode_New(const CPU_INT16U uiDev_in)
{
    return 0;
}

void fs_inode_Free(FS_INODE * pstInode_inout)
{
    FS_INODE_EXT *   pstInode = (FS_INODE_EXT *)pstInode_inout;

    if (0 == pstInode) {
        return;
    }

    if (0 == pstInode->i_dev) {
        Mem_Clr(pstInode, (sizeof(FS_INODE_EXT) - sizeof(DRV_LOCK)));
        return;
    }

    if (pstInode->i_count > 1) {
        drv_disp_Printf("[i-node][count][%d]\r\n", pstInode->i_count);
        CPUExt_CorePanic("[fs_inode_Free][i-node count]");
    }

    if (pstInode->ind.i_nlinks) {
        CPUExt_CorePanic("[fs_inode_Free][i-node links]");
    }

    fs_super_ClearBit(pstInode->i_dev, pstInode->i_num);
    Mem_Clr(pstInode, (sizeof(FS_INODE_EXT) - sizeof(DRV_LOCK)));
    return;
}

void fs_inode_AddRef(FS_INODE * pstInode_inout)
{
    FS_INODE_EXT *   pstInode = (FS_INODE_EXT *)pstInode_inout;

    if (0 == pstInode) {
        CPUExt_CorePanic("[fs_inode_AddRef][inode invalid]");
        return;
    }

    pstInode->i_count++;
}

FS_PRIVATE FS_INODE_EXT * fs_inode_GetEmpty(void)
{
    static FS_INODE_EXT * s_pstLastInode = fs_inode_astTbl;
    FS_INODE_EXT * pstInode = 0;
    CPU_INT32U  i = 0;

    do {
        pstInode = 0;

        for (i = FS_INODE_EXT_MAX; i > 0 ; --i) {
            if ((++s_pstLastInode) >= (fs_inode_astTbl + FS_INODE_EXT_MAX)) {
                s_pstLastInode = fs_inode_astTbl;
            }

            if (!(s_pstLastInode->i_count)) {
                pstInode = s_pstLastInode;

                if (!(pstInode->i_dirt) && !(pstInode->i_lock)) {
                    break;
                }
            }
        }

        if (0 == pstInode) {
            CPUExt_CorePanic("[fs_inode_GetEmpty][No free inodes in memory]");
        }

        fs_inode_WaitOn(pstInode);

        while (pstInode->i_dirt) {
            fs_inode_Write(pstInode);
            fs_inode_WaitOn(pstInode);
        }
    } while (pstInode->i_count);

    Mem_Clr(pstInode, (sizeof(FS_INODE_EXT) - sizeof(DRV_LOCK)));
    pstInode->i_count = 1;
    return (pstInode);
}

FS_PRIVATE void fs_inode_WaitOn(FS_INODE_EXT* pstInode_in)
{
    CPU_SR_ALLOC();

    if (0 == pstInode_in) {
        CPUExt_CorePanic("[fs_inode_WaitOn][invalid parameter]");
    }

    OS_CRITICAL_ENTER();

    while (pstInode_in->i_lock) {
        drv_lock_SleepOn(&(pstInode_in->i_wait));
    }

    OS_CRITICAL_EXIT();
}

FS_PRIVATE void fs_inode_Lock(FS_INODE_EXT * pstInode_inout)
{
    CPU_SR_ALLOC();

    if (0 == pstInode_inout) {
        CPUExt_CorePanic("[fs_inode_WaitOn][invalid parameter]");
    }

    OS_CRITICAL_ENTER();

    while (pstInode_inout->i_lock) {
        drv_lock_SleepOn(&(pstInode_inout->i_wait));
    }

    pstInode_inout->i_lock = 1;
    OS_CRITICAL_EXIT();
}

FS_PRIVATE void fs_inode_Unlock(FS_INODE_EXT * pstInode_inout)
{
    if (0 == pstInode_inout) {
        CPUExt_CorePanic("[fs_inode_WaitOn][invalid parameter]");
    }

    pstInode_inout->i_lock = 0;
    drv_lock_WakeUp(&(pstInode_inout->i_wait));
}


FS_PRIVATE void fs_inode_Write(FS_INODE_EXT* pstInode_in)
{
    FS_SUPER_BLOCK * pstSuper = 0;
    DRV_BLK_BUFFER * pstBuf = 0;
    CPU_INT32U  uiBlock = 0;

    fs_inode_Lock(pstInode_in);

    if (!(pstInode_in->i_dirt) && !(pstInode_in->i_dev)) {
        fs_inode_Unlock(pstInode_in);
        return;
    }

    pstSuper = fs_super_Get(pstInode_in->i_dev);

    if (0 == pstSuper) {
        CPUExt_CorePanic("[fs_inode_Write][device invalid]");
    }

    uiBlock = 2 + pstSuper->s_imap_blocks + pstSuper->s_zmap_blocks + ((pstInode_in->i_num - 1) / FS_INODE_PER_BLOCK);
    pstBuf = drv_blk_Read(pstInode_in->i_dev, uiBlock);

    if (0 == pstBuf) {
        CPUExt_CorePanic("[fs_inode_Write][reading failed]");
    }

    ((FS_INODE *)(pstBuf->pbyData))[((pstInode_in->i_num - 1) % FS_INODE_PER_BLOCK)] = *((FS_INODE *)pstInode_in);
    drv_blk_MakeDirty(pstBuf);
    pstInode_in->i_dirt = 0;
    drv_blk_Release(pstBuf);
    fs_inode_Unlock(pstInode_in);
    return;
}

FS_PRIVATE void fs_inode_Read(FS_INODE_EXT* pstInode_inout)
{
    FS_SUPER_BLOCK * pstSuper = 0;
    DRV_BLK_BUFFER * pstBuf = 0;
    CPU_INT32U  uiBlock = 0;

    fs_inode_Lock(pstInode_inout);
    pstSuper = fs_super_Get(pstInode_inout->i_dev);

    if (0 == pstSuper) {
        CPUExt_CorePanic("[fs_inode_Read][device invalid]");
    }

    uiBlock = 2 + pstSuper->s_imap_blocks + pstSuper->s_zmap_blocks + ((pstInode_inout->i_num - 1) / FS_INODE_PER_BLOCK);
    pstBuf = drv_blk_Read(pstInode_inout->i_dev, uiBlock);

    if (0 == pstBuf) {
        CPUExt_CorePanic("[fs_inode_Write][reading failed]");
    }

    *((FS_INODE *)pstInode_inout) = ((FS_INODE *)(pstBuf->pbyData))[((pstInode_inout->i_num - 1) % FS_INODE_PER_BLOCK)];
    drv_blk_Release(pstBuf);
    fs_inode_Unlock(pstInode_inout);
    return;
}

FS_PRIVATE void fs_inode_Sync(void)
{
    CPU_INT32U i = 0;
    FS_INODE_EXT * pstInode = 0;

    pstInode = 0 + fs_inode_astTbl;

    for (i = 0; i < FS_INODE_EXT_MAX; ++i, ++pstInode) {
        fs_inode_WaitOn(pstInode);

        if ((pstInode->i_dirt) && !(pstInode->i_pipe)) {
            fs_inode_Write(pstInode);
        }
    }
}

FS_PRIVATE void fs_inode_FreeIndirect(const CPU_INT16U uiDev_in, const CPU_INT32U uiBlk_in)
{
    DRV_BLK_BUFFER * pstBuf = 0;
    CPU_INT16U * puiBlk = 0;
    CPU_INT32U   i = 0;

    if (!uiBlk_in) {
        return;
    }

    pstBuf = drv_blk_Read(uiDev_in, uiBlk_in);

    if (0 != pstBuf) {
        puiBlk = (CPU_INT16U *)pstBuf->pbyData;

        for (i = 0; i < (CPU_EXT_HD_BLOCK_SIZE / sizeof(CPU_INT16U)); ++i, ++puiBlk) {
            if (0 != (*puiBlk)) {
                fs_super_FreeBlock(uiDev_in, (*puiBlk));
            }
        }

        drv_blk_Release(pstBuf);
    }

    fs_super_FreeBlock(uiDev_in, uiBlk_in);
}

FS_PRIVATE void fs_inode_FreeDoubleIndirect(const CPU_INT16U uiDev_in, const CPU_INT32U uiBlk_in)
{
    DRV_BLK_BUFFER * pstBuf = 0;
    CPU_INT16U * puiBlk = 0;
    CPU_INT32U   i = 0;

    if (!uiBlk_in) {
        return;
    }

    pstBuf = drv_blk_Read(uiDev_in, uiBlk_in);

    if (0 != pstBuf) {
        puiBlk = (CPU_INT16U *)pstBuf->pbyData;

        for (i = 0; i < (CPU_EXT_HD_BLOCK_SIZE / sizeof(CPU_INT16U)); ++i, ++puiBlk) {
            if (0 != (*puiBlk)) {
                fs_inode_FreeIndirect(uiDev_in, (*puiBlk));
            }
        }

        drv_blk_Release(pstBuf);
    }

    fs_super_FreeBlock(uiDev_in, uiBlk_in);
}


