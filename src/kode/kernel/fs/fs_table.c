/******************************************************************************
    Include
******************************************************************************/

#include <fs_table.h>
#include <fs.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define FS_PRIVATE  static
#define FS_PRIVATE

#define FS_TABLE_FILE_MAX  (64)
FS_PRIVATE FS_FILE  fs_table_astFile[FS_TABLE_FILE_MAX] = {{0,},};

/******************************************************************************
    Private Interface
******************************************************************************/

/******************************************************************************
    Function Definition
******************************************************************************/

void fs_table_Init(void)
{
    CPU_INT32U  i = 0;

    for (i = 0; i < FS_TABLE_FILE_MAX; ++i) {
        fs_table_astFile[i].f_count = 0;
    }
}

FS_FILE * FS_GetFreeFileHandler(void)
{
    CPU_INT32U i = 0;

    for (i = 0; i < FS_TABLE_FILE_MAX; ++i) {
        if (0 == fs_table_astFile[i].f_count) {
            break;
        }
    }

    if (i < FS_TABLE_FILE_MAX) {
        return (&fs_table_astFile[i]);
    }

    return 0;
}

