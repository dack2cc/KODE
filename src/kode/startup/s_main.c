
/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>
#include <kd/kdext.h>
#include <cpu_boot.h>

/******************************************************************************
    Public Interface
******************************************************************************/

static KDint32 m_aiTask0UserStack [ X86_MEM_PAGE_SIZE>>2 ] ;

struct _STACK_START {
	KDint32 * a;
	KDint16 b;
} g_s_main_stTask0UserStackStart = { &(m_aiTask0UserStack[ X86_MEM_PAGE_SIZE>>2 ]), 0x10 };

/******************************************************************************
    External Reference
******************************************************************************/


/******************************************************************************
    Internal Definition
******************************************************************************/

void  s_logo(void);
void* s_thread_alice(void* param_in);
void* s_thread_jerry(void* param_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void s_main(void)
{
	KDThreadAttr*  pstAttr = KD_NULL;
	
	kdextInit();
	
	//s_logo();
	
	pstAttr = kdThreadAttrCreate();
	kdThreadCreate(pstAttr, s_thread_alice, (void *)0xABCDEF);
	kdThreadAttrFree(pstAttr);
	
	//kdHandleAssertion(__FILE__, __FILE__, __LINE__);
	
	kdextRun();
	
	while (1) ;
	
	return;
}

void s_logo(void)
{
	kdLogMessage("  ====  ====  ||    \n");
	kdLogMessage("  ||    ||    ||=== \n");
	kdLogMessage("  ====  ====  ||    \n");
}

void* s_thread_alice(void* param_in)
{
	KDThreadAttr*  pstAttr = KD_NULL;
	KDThread*      pstTony = KD_NULL;

	if (0xABCDEF == (KDint32)param_in) 
	kdLogMessage("[alice] Hello (^-^)/ \n");
	
	pstAttr = kdThreadAttrCreate();
	pstTony = kdThreadCreate(pstAttr, s_thread_jerry, (void *)0xFEDCBA);
	kdThreadAttrFree(pstAttr);
	
	kdThreadJoin(pstTony, 0);
	
	kdLogMessage("[alice] Goodbye Jerry (-_-)/ \n");
	
	for (;;);
	
	return ((void *)0);
}

void* s_thread_jerry(void* param_in)
{
	if (0xFEDCBA == (KDint32)param_in) 
	kdLogMessage("[jerry] This is Jerry going down (=_=) \n");
	
	return ((void *)0);
}

