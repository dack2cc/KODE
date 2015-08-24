
/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>
#include <kd/kdext.h>
#include <cpu_boot.h>
#include <std/stdio.h>


/******************************************************************************
    External Reference
******************************************************************************/


/******************************************************************************
    Internal Definition
******************************************************************************/


void* s_process_setup(void* param_in);
void  s_logo(void);
void  s_time(void);

#define MAGIC_SETUP    (0x5E749)
#define MAGIC_ALICE    (0xA61CE)
#define MAGIC_JERRY    (0x7E229)

KDint32  s_thread_magic = MAGIC_SETUP;

void* s_thread_alice(void* param_in);
void* s_thread_jerry(void* param_in);


/******************************************************************************
    Function Definition
******************************************************************************/

void s_main(void)
{
    kdextInit();
    //s_process_setup(0);
    kdextProcessCreate(s_process_setup, 0);
    kdextRun();
}

void* s_process_setup(void* param_in)
{
    const KDEvent *  pstEvt  = KD_NULL;
    KDThreadAttr  *  pstAttr = KD_NULL;

    s_logo();
    s_time();

    kdextSetup();
    kdFopen("/", "rw");

    pstAttr = kdThreadAttrCreate();
    kdThreadCreate(pstAttr, s_thread_alice, (void *)(&s_thread_magic));
    kdThreadAttrFree(pstAttr);

    s_thread_magic = MAGIC_ALICE;
    printf("[setup][Magic 0x%X]\r\n", &pstEvt);

    //kdHandleAssertion(0, __FILE__, __LINE__);

    for (;;) {
        pstEvt = kdWaitEvent(0);
    }
}


void s_logo(void)
{
    //kdLogMessage("  ====  ====  ||    \r\n");
    //kdLogMessage("    ||    ||  ||=== \r\n");
    //kdLogMessage("  ====  ====  ||    \r\n");
    kdLogMessage("kode OS 4 kokoto \r\n");
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

void* s_thread_alice(void* param_in)
{
    const KDEvent*  pstEvt   = KD_NULL;
    KDThreadAttr*   pstAttr  = KD_NULL;
    KDThread*       pstJerry = KD_NULL;

    printf("[alice][Magic 0x%X]\r\n", &pstJerry);
    printf("[alice][Hello 0x%X]\r\n", *((KDint32 *)param_in));

    pstAttr  = kdThreadAttrCreate();
    pstJerry = kdThreadCreate(pstAttr, s_thread_jerry, param_in);
    kdThreadAttrFree(pstAttr);

    *((KDint32 *)param_in) = MAGIC_JERRY;

    kdThreadJoin(pstJerry, 0);
    printf("[alice][Down  0x%X] \r\n", *((KDint32 *)param_in));

    for (;;) {
        pstEvt = kdWaitEvent(0);
    }

    return ((void *)0);
}

void* s_thread_jerry(void* param_in)
{
    CPU_DATA magic  = 0;

    printf("[jerry][Magic 0x%X]\r\n", &magic);
    printf("[jerry][Wake  0x%X]\r\n", *((KDint32 *)param_in));

    return ((void *)0);
}

