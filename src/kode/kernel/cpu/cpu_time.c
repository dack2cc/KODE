
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_time.h>
#include <cpu_ext.h>
#include <cpu_asm.h>

/******************************************************************************
    Private Definition
******************************************************************************/

#define MINUTE (60)
#define HOUR   (60*MINUTE)
#define DAY    (24*HOUR)
#define YEAR   (365*DAY)

/* interestingly, we assume leap-years */
CPU_PRIVATE CPU_INT32S cpu_time_aiMonth[12] = {
	0,
	DAY*(31),
	DAY*(31+29),
	DAY*(31+29+31),
	DAY*(31+29+31+30),
	DAY*(31+29+31+30+31),
	DAY*(31+29+31+30+31+30),
	DAY*(31+29+31+30+31+30+31),
	DAY*(31+29+31+30+31+30+31+31),
	DAY*(31+29+31+30+31+30+31+31+30),
	DAY*(31+29+31+30+31+30+31+31+30+31),
	DAY*(31+29+31+30+31+30+31+31+30+31+30)
};


typedef struct _CPU_TIME_CONTROL {
	CPU_INT32U     uiStartupTime;
	CPU_INT32U     uiTick;
} CPU_TIME_CONTROL;

CPU_PRIVATE  CPU_TIME_CONTROL  cpu_time_stCtl;

/******************************************************************************
    Private Interface
******************************************************************************/

#define _CPU_TIME_CMOS_READ(addr) ({ \
_asm_outb_p(0x80|addr,0x70); \
_asm_inb_p(0x71); \
})

#define _CPU_TIME_BCD_TO_BIN(val) ((val)=((val)&15) + ((val)>>4)*10)

CPU_PRIVATE CPU_INT32U cpu_time_Make(const CPU_EXT_TIME * pstTime_in);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_time_Init(void)
{
	CPU_EXT_TIME stTime;
	
	do {
		stTime.iSec = _CPU_TIME_CMOS_READ(0);
		stTime.iMin = _CPU_TIME_CMOS_READ(2);
		stTime.iHour = _CPU_TIME_CMOS_READ(4);
		stTime.iMday = _CPU_TIME_CMOS_READ(7);
		stTime.iMon = _CPU_TIME_CMOS_READ(8);
		stTime.iYear = _CPU_TIME_CMOS_READ(9);
	} while (_CPU_TIME_CMOS_READ(0) != stTime.iSec);
	_CPU_TIME_BCD_TO_BIN(stTime.iSec);
	_CPU_TIME_BCD_TO_BIN(stTime.iMin);
	_CPU_TIME_BCD_TO_BIN(stTime.iHour);
	_CPU_TIME_BCD_TO_BIN(stTime.iMday);
	_CPU_TIME_BCD_TO_BIN(stTime.iMon);
	_CPU_TIME_BCD_TO_BIN(stTime.iYear);
	stTime.iMon--;
	cpu_time_stCtl.uiStartupTime = cpu_time_Make(&stTime);
	cpu_time_stCtl.uiTick = 0;
	return;
}

void CPUExt_TimeCurrent(CPU_INT32U * puiTime_out)
{
	if (0 != puiTime_out) {
		(*puiTime_out) = cpu_time_stCtl.uiStartupTime + (cpu_time_stCtl.uiTick / CPU_TIME_TICK_HZ);
	}
}

void  cpu_time_ISR_Tick(void)
{
	++cpu_time_stCtl.uiTick;
}


CPU_PRIVATE CPU_INT32U cpu_time_Make(const CPU_EXT_TIME * pstTime_in)
{
	CPU_INT32U uiRes  = 0;
	CPU_INT32U uiYear = 0;
	
	uiYear = pstTime_in->iYear - 70;
	
	/* magic offsets (y+1) needed to get leapyears right.*/
	uiRes  = YEAR*uiYear + DAY*((uiYear + 1)/4);
	uiRes += cpu_time_aiMonth[pstTime_in->iMon];
	
	/* and (y+2) here. If it wasn't a leap-year, we have to adjust */
	if ((pstTime_in->iMon > 1) && ((uiYear + 2)%4)) {
		uiRes -= DAY;
	}
	
	uiRes += DAY*(pstTime_in->iMday - 1);
	uiRes += HOUR*(pstTime_in->iHour);
	uiRes += MINUTE*(pstTime_in->iMin);
	uiRes += pstTime_in->iSec;
	
	return (uiRes);
}


