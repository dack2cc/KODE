
/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>
#include <kd/kdext.h>
#include <cpu_boot.h>
#include <std/stdio.h>

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

#define MAGIC_ALICE    (0xABCDEF)
#define MAGIC_JERRY    (0xFEDCBA)

void  s_logo(void);
void  s_time(void);
void* s_thread_init(void* param_in);

void* s_thread_alice(void* param_in);
void* s_thread_jerry(void* param_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void s_main(void)
{	
	KDThreadAttr*  pstAttr = KD_NULL;
	
	kdextInit();

	pstAttr = kdThreadAttrCreate();
	kdThreadCreate(pstAttr, s_thread_init, (void *)0);
	kdThreadAttrFree(pstAttr);
	
	pstAttr = kdThreadAttrCreate();
	kdThreadCreate(pstAttr, s_thread_alice, (void *)MAGIC_ALICE);
	kdThreadAttrFree(pstAttr);
	
	//kdHandleAssertion(0, __FILE__, __LINE__);
	
	kdextRun();
	
	while (1) ;
	
	return;
}

void s_logo(void)
{
	kdLogMessage("  ====  ====  ||    \r\n");
	kdLogMessage("    ||    ||  ||=== \r\n");
	kdLogMessage("  ====  ====  ||    \r\n");
}

void s_time(void)
{
	KDust  ust   = 0;
	KDtime time  = 0;
	KDtime timep = 1;
	
	ust  = kdGetTimeUST();
	time = kdTime(&timep);
	if (time != timep) {
		kdHandleAssertion("[s_time][time is invalid]", __FILE__, __LINE__);
	}
	printf("[Tick:%d][Time:%d][%d] \r\n", (int)ust, (int)time, (int)timep);
}

void* s_thread_init(void* param_in)
{
	s_logo();
	s_time();
	kdextSetup();
	
	//for (;;);
	
	return ((void *)0);
}

void* s_thread_alice(void* param_in)
{
	KDThreadAttr*  pstAttr  = KD_NULL;
	KDThread*      pstJerry = KD_NULL;
	
	if (MAGIC_ALICE == (KDint32)param_in) {
	    kdLogMessage("[alice] Hello (^-^)/ \r\n");
	}
	
	pstAttr  = kdThreadAttrCreate();
	pstJerry = kdThreadCreate(pstAttr, s_thread_jerry, (void *)MAGIC_JERRY);
	kdThreadAttrFree(pstAttr);
	
	kdThreadJoin(pstJerry, 0);
	kdLogMessage("[alice] Goodbye (-_-)/ \r\n");
	
	for (;;);
	
	return ((void *)0);
}

void* s_thread_jerry(void* param_in)
{
	if (MAGIC_JERRY == (KDint32)param_in) {
	    kdLogMessage("[jerry] This is Jerry going down (=_=) \r\n");
	}
	
	//for (;;);
	
	return ((void *)0);
}

