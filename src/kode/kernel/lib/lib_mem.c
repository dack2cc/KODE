/******************************************************************************
    Include
******************************************************************************/

#include <lib_mem.h>

/******************************************************************************
    Private Define
******************************************************************************/

/******************************************************************************
    Private Interface
******************************************************************************/

/******************************************************************************
    Function Definition
******************************************************************************/

void  Mem_Init(void)
{
}

void  Mem_Clr(void        *pmem,
              CPU_SIZE_T   size)
{
#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
	if ((0 == pmem) || (0 == size)) {
		return;
	}
#endif /* LIB_MEM_CFG_ARG_CHK_EXT_EN */
	
	CPU_SIZE_T i;
	
	for (i = 0; i < size; ++i) {
		(*(((CPU_INT08U *)(pmem)) + i)) = 0x00;
	}
}

void  Mem_Set(void        *pmem,
              CPU_INT08U   data_val,
              CPU_SIZE_T   size)
{
#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
	if ((0 == pmem) || (0 == size)) {
		return;
	}
#endif /* LIB_MEM_CFG_ARG_CHK_EXT_EN */

	CPU_SIZE_T i;
	
	for (i = 0; i < size; ++i) {
		(*(((CPU_INT08U *)(pmem)) + i)) = data_val;
	}
}

void  Mem_Copy(       void        *pdest,
               const  void        *psrc,
                      CPU_SIZE_T   size)
{
#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
	if ((0 == pdest) || (0 == psrc) || (0 == size)) {
		return;
	}
#endif /* LIB_MEM_CFG_ARG_CHK_EXT_EN */

	MEM_VAL_COPY(pdest, psrc, size);
}

CPU_BOOLEAN  Mem_Cmp(const  void        *p1_mem,
                     const  void        *p2_mem,
                            CPU_SIZE_T   size)
{
#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
	if ((0 == p1_mem) || (0 == p2_mem) || (0 == size)) {
		return (0);
	}
#endif /* LIB_MEM_CFG_ARG_CHK_EXT_EN */

	CPU_BOOLEAN equal = 1;
	CPU_SIZE_T i;
	
	for (i = 0; i < size; ++i) {
		if ((*(((CPU_INT08U *)(p1_mem)) + i)) != (*(((CPU_INT08U *)(p2_mem)) + i))) {
			equal = 0;
			break;
		}
	}

	return (equal);
}



