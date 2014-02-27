/******************************************************************************
    Include
******************************************************************************/

#include <std/stdio.h>
#include <kd/kd.h>

/******************************************************************************
    Private Define
******************************************************************************/

#define PRINTF_BUF_MAX  (1024)
static char printf_aszBuf[1024];

/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/


int printf(const char * fmt_in, ...)
{
	va_list    args;
	int i = 0;

	va_start(args, fmt_in);
	i = vsprintf(printf_aszBuf, fmt_in, args);
	kdLogMessage(printf_aszBuf);
	va_end(args);
	
	return (i);
}


