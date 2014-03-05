
#ifndef __CPU_EXT_H__
#define __CPU_EXT_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu_core.h>

/******************************************************************************
    Definition
******************************************************************************/

/*
    Display Mode
*/
#define CPU_EXT_DISP_MODE_TEXT  (0)
#define CPU_EXT_DISP_MODE_8BIT  (1)
#define CPU_EXT_DISP_MODE  (CPU_EXT_DISP_MODE_8BIT)


/*
    System Call
*/
CPU_CORE_EXT  CPU_INT32S  cpu_core_iErrorCode;

#define CPU_EXT_DEFINE_KERNEL_FNCT_0(type, name) \
type name(void) \
{ \
	CPU_INT32S __res; \
	__asm__ volatile ("int $0x80" \
		: "=a" (__res) \
		: "0" (__KF_##name)); \
	if (__res >= 0) \
	    return (type) __res; \
	cpu_core_iErrorCode = -__res; \
	return (type) (-1); \
}

#define  CPU_EXT_DEFINE_KERNEL_FNCT_1(type,name,atype,a) \
type name(atype a) \
{ \
	CPU_INT32S __res; \
	__asm__ volatile ("int $0x80" \
		: "=a" (__res) \
		: "0" (__KF_##name),"b" ((CPU_DATA)(a))); \
	if (__res >= 0) \
	    return (type) __res; \
	cpu_core_iErrorCode = -__res; \
	return (type) (-1); \
}

#define CPU_EXT_DEFINE_KERNEL_FNCT_2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
	CPU_INT32S __res; \
	__asm__ volatile ("int $0x80" \
		: "=a" (__res) \
	    : "0" (__KF_##name),"b" ((CPU_DATA)(a)),"c" ((CPU_DATA)(b))); \
	if (__res >= 0) \
	    return (type) __res; \
    cpu_core_iErrorCode = -__res; \
	return (type) (-1); \
}

#define CPU_EXT_DEFINE_KERNEL_FNCT_3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
	CPU_INT32S __res; \
    __asm__ volatile ("int $0x80" \
    	: "=a" (__res) \
    	: "0" (__KF_##name),"b" ((CPU_DATA)(a)),"c" ((CPU_DATA)(b)),"d" ((CPU_DATA)(c))); \
    if (__res>=0) \
	    return (type) __res; \
    cpu_core_iErrorCode = -__res; \
	return (type) (-1); \
}

/*
    Transfer Data between User and Kernel space
*/
#define CPU_EXT_GET_FS_BYTE(val_out, addr_in)  {\
	unsigned register char _v; \
	__asm__("movb %%fs:%1, %0":"=r"(_v):"m"(*(addr_in))); \
	(val_out) = _v; \
}

#define CPU_EXT_GET_FS_WORD(val_out, addr_in)  {\
	unsigned short _v; \
	__asm__ ("movw %%fs:%1,%0":"=r" (_v):"m" (*(addr_in))); \
	(val_out) = _v; \
}

#define CPU_EXT_GET_FS_LONG(val_out, addr_in)  {\
	unsigned long _v; \
	__asm__ ("movl %%fs:%1,%0":"=r" (_v):"m" (*(addr_in))); \
	(val_out) = _v; \
}

#define CPU_EXT_PUT_FS_BYTE(val_in, addr_out)  {\
	__asm__ ("movb %0,%%fs:%1"::"r" (va_inl),"m" (*(addr_out))); \
}

#define CPU_EXT_PUT_FS_WORD(val_in, addr_out)  {\
	__asm__ ("movw %0,%%fs:%1"::"r" (val_in),"m" (*(addr_out))); \
}

#define CPU_EXT_PUT_FS_LONG(val_in, addr_out)  {\
	__asm__ ("movl %0,%%fs:%1"::"r" (val_in),"m" (*(addr_out))); \
}


/*
    RAM Disk
*/
#define CPU_EXT_RAM_DISK_START   (256)
#define CPU_EXT_RAM_DISK_SIZE    (512 * 1024)

/******************************************************************************
    Public Interface
******************************************************************************/

/*
    Debug
*/
extern void CPUExt_CorePanic(const CPU_CHAR* pszMsg_in);

/*
    Output
*/
extern CPU_INT32U  CPUExt_DispPrint(const CPU_CHAR* pszStr_in);
extern void        CPUExt_DispChar(const CPU_CHAR chAscii_in);

/*
    Input
*/
typedef struct _CPU_EXT_KEY_EVENT {
	CPU_INT08U  uiScanCode;
} CPU_EXT_KEY_EVENT;

extern void CPUExt_KeyRegisterHandler(CPU_FNCT_PTR pfnKeyHandler_in);

/*
    Task
*/
enum {
	CPU_TASK_ARG_EFLAG = 0,     /* CPU_SR                     */
	CPU_TASK_ARG_ROUTINE,       /* CPU_FNCT_PTR               */
	CPU_TASK_ARG_PARAMETER,     /* void *                     */
	CPU_TASK_ARG_RET_POINT,     /* CPU_FNCT_PTR               */
	CPU_TASK_ARG_KERNEL_EN,     /* DEF_ENABLED or DEF_DISABLE */
	CPU_TASK_ARG_KERNEL_STACK,  /* physical address           */
	CPU_TASK_ARG_EXT_DATA,      /* physical address           */
	CPU_TASK_ARG_MAX
};
extern CPU_ERR  CPUExt_TaskCreate(const CPU_DATA* pArgList_in, CPU_INT32U* puiTaskID_out);
extern void     CPUExt_TaskDelete(const CPU_INT32U  uiTaskID_in);
extern void     CPUExt_TaskSwitch(const CPU_INT32U  uiTaskID_in);
extern void     CPUExt_TaskSwitchToRing3(void);

/*
    Interrupt
*/
extern void     CPUExt_GateRegisterISRHookEnter(CPU_FNCT_VOID pfnISRHookEnter_in);
extern void     CPUExt_GateRegisterISRHookExit(CPU_FNCT_VOID pfnISRHookExit_in);
extern void     CPUExt_GateRegisterTimeTick(CPU_FNCT_VOID pfnTimeTick_in);
extern CPU_ERR  CPUExt_GateRegisterKernelFnct(const CPU_INT32U  uiFnctNum_in, CPU_FNCT_VOID pfnKernelFnct_in);

/*
    Physical Memory
*/
extern void  CPUExt_PageGetFree(CPU_ADDR*  paddrPhysical_out);
extern void  CPUExt_PageRelease(const CPU_ADDR  addrPhysical_in);
extern void  CPUExt_PageGetBufferSpace(CPU_ADDR*  paddrPhysicalStart_out, CPU_ADDR*  paddrPhysicalEnd_out);
extern void  CPUExt_PageGetRamdiskSpace(CPU_ADDR*  paddrPhysicalStart_out, CPU_ADDR*  paddrPhysicalEnd_out);

/*
   Hard Disk
*/

#define  CPU_EXT_HD_DEVICE      (0x300)
#define  CPU_EXT_HD_BLOCK_SIZE  (1024)

extern void CPUExt_HDGetRootDevice(CPU_INT16U * puiDev_out);
extern void CPUExt_HDGetDiskCount(CPU_INT32S * piCount_out);
extern void CPUExt_HDSetPartition(const CPU_INT32S iDiskIndex_in, const CPU_INT08U * pbyTable_in);

#define  CPU_EXT_HD_CMD_READ    (0)
#define  CPU_EXT_HD_CMD_WRITE   (1)

typedef struct _CPU_EXT_HD_REQUEST_IN {
	CPU_INT08U * pbyData;
	CPU_INT32U   uiBlkIdx;
    CPU_INT32S   iDev;
	CPU_INT32S   iCmd;
} CPU_EXT_HD_REQUEST_IN;

#define  CPU_EXT_HD_RESULT_OK   (0)
#define  CPU_EXT_HD_RESULT_FULL (1)
#define  CPU_EXT_HD_RESULT_IO   (2)

typedef struct _CPU_EXT_HD_REQUEST_OUT {
	CPU_INT32S   iResult;
} CPU_EXT_HD_REQUEST_OUT;

typedef struct _CPU_EXT_HD_REQUEST {
	CPU_EXT_HD_REQUEST_IN   in;
	CPU_EXT_HD_REQUEST_OUT  out;
} CPU_EXT_HD_REQUEST;

extern void CPUExt_HDRegisterNotifyRW(CPU_FNCT_PTR pfnNotify_in);
extern void CPUExt_HDRegisterNotifyFree(CPU_FNCT_VOID pfnNotify_in);
extern void CPUExt_HDRequest(CPU_EXT_HD_REQUEST* pstRequest_inout);

/*
   System Time
*/

typedef struct _CPU_EXT_TIME {
	CPU_INT32S  iSec;
	CPU_INT32S  iMin;
	CPU_INT32S  iHour;
	CPU_INT32S  iMday;
	CPU_INT32S  iMon;
	CPU_INT32S  iYear;
	CPU_INT32S  iWday;
	CPU_INT32S  iYday;
	CPU_INT32S  iIsdst;
} CPU_EXT_TIME;

extern void CPUExt_TimeCurrent(CPU_INT32U * puiTime_out);
extern void CPUExt_TimeTick(CPU_INT32U * puiTick_out);

#endif /* __CPU_EXT_H__ */

