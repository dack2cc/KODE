
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_disp.h>
#include <cpu.h>
#include <cpu_boot.h>
#include <cpu_asm.h>

/******************************************************************************
    Private Define
******************************************************************************/

/* Device */
#define _VIDEO_TYPE_MDA    (0x10)
#define _VIDEO_TYPE_CGA    (0x11)
#define _VIDEO_TYPE_EGAM   (0x20)
#define _VIDEO_TYPE_EGAC   (0x21)

typedef struct _DISPLAY_DEVICE {
	CPU_CHAR*   pszName;
	CPU_INT08U  uiType;
	CPU_INT32U  uiMemStart;
	CPU_INT32U  uiMemEnd;
	CPU_INT16U  uiPortReg;
	CPU_INT16U  uiPortVal;
} DISPLAY_DEVICE;

enum {
	_DEVICE_INVALID = -1,
	_DEVICE_EGAM,
	_DEVICE_MDA,
	_DEVICE_EGAC,
	_DEVICE_CGA,
	_DEVICE_MAX
};

CPU_PRIVATE  const DISPLAY_DEVICE  m_astDev[_DEVICE_MAX] = {
	{"EGAm", _VIDEO_TYPE_EGAM, 0xB0000, 0xB8000, 0x03B4, 0x03B5},
	{"*MDA", _VIDEO_TYPE_MDA,  0xB0000, 0xB2000, 0x03B4, 0x03B5},
	{"EGAc", _VIDEO_TYPE_EGAC, 0xB8000, 0xBC000, 0x03D4, 0x03D5},
	{"*CGA", _VIDEO_TYPE_CGA,  0xB8000, 0xBA000, 0x03D4, 0x03D5},
};


/* control */
typedef struct _DISPLAY_CONTROL {
	DISPLAY_DEVICE*  pstDev;
	
	CPU_INT32U  uiColNum;
	CPU_INT32U  uiRowNum;
	CPU_INT32U  uiRowPitch;
	
	CPU_INT32U  uiPosCol;
	CPU_INT32U  uiPosRow;
	CPU_INT32U  uiPosMem;
	
	CPU_INT32U  uiScrMemStart;
	CPU_INT32U  uiScrMemEnd;
	CPU_INT32U  uiScrRowTop;
	CPU_INT32U  uiScrRowBottom;
	
	CPU_INT08U  uiAttribute;
} DISPLAY_CONTROL;

CPU_PRIVATE  DISPLAY_CONTROL  m_stCtl;

/* for the asm call */
CPU_PRIVATE  CPU_INT32U       m_uiColNum;
CPU_PRIVATE  CPU_INT08U       m_uiAttribute;


//static CPU_INT08U  m_byFontAttr = 0x07;  // BG:Black; Color:White
#define _FONT_ATTR    (0x07)
#define _ERASE_CHAR   ((_FONT_ATTR << 8) + 0x20)

#define _CHECK_DEVICE()  if (0 == m_stCtl.pstDev) return;

/******************************************************************************
    Private Interface
******************************************************************************/

CPU_PRIVATE void cpu_disp_SetCode(const CPU_CHAR chCode_in);
CPU_PRIVATE void cpu_disp_SetCursor(void);
CPU_PRIVATE void cpu_disp_SetPosition(const CPU_INT32U uiCol_in, const CPU_INT32U uiRow_in);
CPU_PRIVATE void cpu_disp_SetScreenMemoryStart(void);
CPU_PRIVATE void cpu_disp_ScrollUp(void);
CPU_PRIVATE void cpu_disp_ScrollDown(void);
CPU_PRIVATE void cpu_disp_LineFeed(void);
CPU_PRIVATE void cpu_disp_ReverseLineFeed(void);
CPU_PRIVATE void cpu_disp_CarriageReturn(void);
CPU_PRIVATE void cpu_disp_Delete(void);

/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_disp_Init(void)
{
	CPU_CHAR* pbyDisplayName = 0;
	CPU_CHAR* pbyDisplayCursor = 0;
	CPU_INT08S  i = 0;

	m_stCtl.uiAttribute = 0x07;
	
	/* get parameter from bios */
	m_stCtl.uiColNum = X86_DISPLAY_COL_NUM;
	m_stCtl.uiRowNum = X86_DISPLAY_ROW_NUM;
	m_stCtl.uiRowPitch = (m_stCtl.uiColNum * 2);
	
	/* monochrome display */
	if (7 == X86_DISPLAY_MODE) {		
		if (0x10 != (X86_DISPLAY_EGA_BX & 0xFF)) {
			m_stCtl.pstDev = (DISPLAY_DEVICE *)(m_astDev + _DEVICE_EGAM);
		}
		else {
			m_stCtl.pstDev = (DISPLAY_DEVICE *)&(m_astDev[_DEVICE_MDA]);
		}
	}
	
	/* color display */
	else {
		if (0x10 != (X86_DISPLAY_EGA_BX & 0xFF)) {
			m_stCtl.pstDev = (DISPLAY_DEVICE *)&(m_astDev[_DEVICE_EGAC]);
		}
		else {
			m_stCtl.pstDev = (DISPLAY_DEVICE *)&(m_astDev[_DEVICE_CGA]);
		}
	}
	
	/* display the device information */
	pbyDisplayName = m_stCtl.pstDev->pszName;
	pbyDisplayCursor = ((CPU_CHAR *)(m_stCtl.pstDev->uiMemStart)) + m_stCtl.uiRowPitch - 8;
	for (i = 0; i < 4; ++i) {
		(*pbyDisplayCursor) = (*pbyDisplayName);
		pbyDisplayName += 1;
		pbyDisplayCursor += 2;
	}
	
	/* the screen information */
	m_stCtl.uiScrMemStart  = m_stCtl.pstDev->uiMemStart;
	m_stCtl.uiScrMemEnd    = m_stCtl.pstDev->uiMemStart + m_stCtl.uiRowNum * m_stCtl.uiRowPitch;
	m_stCtl.uiScrRowTop    = 0;
	m_stCtl.uiScrRowBottom = m_stCtl.uiRowNum;
	
	/* the cursor position */
	cpu_disp_SetPosition(X86_DISPLAY_POS_COL, X86_DISPLAY_POS_ROW);
	
	return;
}


CPU_INT32U  cpu_disp_Print(const CPU_CHAR* pszStr_in)
{
	CPU_CHAR*  pbyChar  = 0;
	CPU_INT32U iCharCnt = 0;
	
	if (0 == pszStr_in) {
		return (0);
	}
	
	pbyChar = (CPU_CHAR *)pszStr_in;
	while ('\0' != (*pbyChar)) {
		cpu_disp_SetCode((*pbyChar));
		++pbyChar;
		++iCharCnt;
	}
	
	cpu_disp_SetCursor();
	
	return (iCharCnt);
}

void  cpu_disp_Char(const  CPU_CHAR  chAscii_in)
{
	cpu_disp_SetCode(chAscii_in);
	cpu_disp_SetCursor();	
}

CPU_PRIVATE void cpu_disp_SetCode(const CPU_CHAR chCode_in)
{
	static CPU_INT08U  s_uiAsciiState = 0;
	
	switch (s_uiAsciiState) {
	case 0: 
	{
	    if ((chCode_in > 31) && (chCode_in < 127)) {
	    	if (m_stCtl.uiPosCol >= m_stCtl.uiColNum) {
	    		m_stCtl.uiPosCol -= m_stCtl.uiColNum;
	    		m_stCtl.uiPosMem -= m_stCtl.uiRowPitch;
	    		cpu_disp_LineFeed();
	    	}
	    	
		    //(*(CPU_CHAR *)(m_stCtl.uiPosMem)) = (*pbyChar);
	    	m_uiAttribute = m_stCtl.uiAttribute;
	    	__asm__(
	    		"movb _m_uiAttribute,%%ah\n\t"
				"movw %%ax,%1\n\t"
				::
	    		"a" (chCode_in),
	    		"m" (*((CPU_INT16S *)(m_stCtl.uiPosMem)))
				:
	    		/* "ax" */
	    	);
		
	        m_stCtl.uiPosMem += 2;
	        m_stCtl.uiPosCol += 1;
	    }
		else if (7 == chCode_in) {
			// beep
		}
		else if (8 == chCode_in) {
			if (m_stCtl.uiPosCol > 0) {
				m_stCtl.uiPosCol -= 1;
				m_stCtl.uiPosMem -= 2;
			}
		}
		else if (9 == chCode_in) {
			m_stCtl.uiPosCol += 8 - (m_stCtl.uiPosCol&7);
			m_stCtl.uiPosMem += (8 - (m_stCtl.uiPosCol&7)) << 1;
			if (m_stCtl.uiPosCol > m_stCtl.uiColNum) {
				m_stCtl.uiPosCol -= m_stCtl.uiColNum;
				m_stCtl.uiPosMem -= m_stCtl.uiRowPitch;
				cpu_disp_LineFeed();
			}
		}
		else if ((10 == chCode_in) || (11 == chCode_in) || (12 == chCode_in)) {
			cpu_disp_LineFeed();
			cpu_disp_CarriageReturn();
		}
		else if (13 == chCode_in) {
			cpu_disp_CarriageReturn();
		}
		else if (27 == chCode_in) {
			//s_uiAsciiState = 1;
		}
	    else {
		    // EMPTY
	    }
		break;
	}
	default:
	   break;
	} // end switch (s_uiAsciiState)		
}

CPU_PRIVATE void cpu_disp_SetPosition(const CPU_INT32U uiCol_in, const CPU_INT32U uiRow_in)
{
	if ((uiCol_in > m_stCtl.uiColNum) 
	||  (uiRow_in > m_stCtl.uiRowNum)) {
		return;
	}
	
	m_stCtl.uiPosCol = uiCol_in;
	m_stCtl.uiPosRow = uiRow_in;
	m_stCtl.uiPosMem = m_stCtl.uiScrMemStart + uiRow_in*m_stCtl.uiRowPitch + (uiCol_in << 1);
}

CPU_PRIVATE void cpu_disp_SetCursor(void)
{
	_CHECK_DEVICE();
	
	CPU_SR_ALLOC();
	
	CPU_CRITICAL_ENTER();
	_asm_outb_p(14, m_stCtl.pstDev->uiPortReg);
	_asm_outb_p(0xff&((m_stCtl.uiPosMem-m_stCtl.pstDev->uiMemStart)>>9), m_stCtl.pstDev->uiPortVal);
	_asm_outb_p(15, m_stCtl.pstDev->uiPortReg);
	_asm_outb_p(0xff&((m_stCtl.uiPosMem-m_stCtl.pstDev->uiMemStart)>>1), m_stCtl.pstDev->uiPortVal);
	CPU_CRITICAL_EXIT();
}


CPU_PRIVATE void cpu_disp_SetScreenMemoryStart(void)
{
	_CHECK_DEVICE();
	
	CPU_SR_ALLOC();
	
	CPU_CRITICAL_ENTER();
	_asm_outb_p(12, m_stCtl.pstDev->uiPortReg);
	_asm_outb_p(0xFF&((m_stCtl.uiScrMemStart - m_stCtl.pstDev->uiMemStart)>>9), m_stCtl.pstDev->uiPortVal);
	_asm_outb_p(13, m_stCtl.pstDev->uiPortReg);
	_asm_outb_p(0xFF&((m_stCtl.uiScrMemStart - m_stCtl.pstDev->uiMemStart)>>1), m_stCtl.pstDev->uiPortVal);
	CPU_CRITICAL_EXIT();
}

CPU_PRIVATE void cpu_disp_ScrollUp(void)
{
	_CHECK_DEVICE();
	
	if ((_VIDEO_TYPE_EGAC == m_stCtl.pstDev->uiType)
	||  (_VIDEO_TYPE_EGAM == m_stCtl.pstDev->uiType)) {
		/* the screen is full, so all the text should go up */
		if ((m_stCtl.uiScrRowTop == 0) 
		&&  (m_stCtl.uiScrRowBottom == m_stCtl.uiRowNum)) {
			m_stCtl.uiScrMemStart += m_stCtl.uiRowPitch;
			m_stCtl.uiScrMemEnd   += m_stCtl.uiRowPitch;
			m_stCtl.uiPosMem      += m_stCtl.uiRowPitch;
			
			/* move the memory */
			if (m_stCtl.uiScrMemEnd > m_stCtl.pstDev->uiMemEnd) {
				m_uiColNum = m_stCtl.uiColNum;
				__asm__(
					"cld\n\t"
					"rep\n\t"
					"movsl\n\t"
					"movl _m_uiColNum, %1\n\t"
					"rep\n\t"
					"stosw"
					::
					"a"(_ERASE_CHAR),
					"c"((m_stCtl.uiRowNum - 1)*(m_stCtl.uiColNum>>1)),
					"D"(m_stCtl.pstDev->uiMemStart),
					"S"(m_stCtl.uiScrMemStart)
					:
					/* "cx", "di", "si" */
				);
				
			    m_stCtl.uiScrMemStart -= m_stCtl.uiRowPitch;
			    m_stCtl.uiScrMemEnd   -= m_stCtl.uiRowPitch;
			    m_stCtl.uiPosMem      -= m_stCtl.uiRowPitch;				
			}
			
			/* fill the last memory */
			else {
				__asm__(
					"cld\n\t"
					"rep\n\t"
					"stosw"
					::
					"a"(_ERASE_CHAR),
					"c"(m_stCtl.uiColNum),
					"D"(m_stCtl.uiScrMemEnd - m_stCtl.uiRowPitch)
					: 
					/* "cx", "di" */
				);
			}
			cpu_disp_SetScreenMemoryStart();
		}
		
		/* the screen is not full, so just fill the next line with erase */
		else {
			m_uiColNum = m_stCtl.uiColNum;
			__asm__(
				"cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl _m_uiColNum,%%ecx\n\t"
				"rep\n\t"
				"stosw"
				::
				"a" (_ERASE_CHAR),
				"c" ((m_stCtl.uiScrRowBottom-m_stCtl.uiScrRowTop-1)*m_stCtl.uiColNum>>1),
				"D" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*m_stCtl.uiScrRowTop),
				"S" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*(m_stCtl.uiScrRowTop+1))
				:
				/* "cx","di","si" */
			);
		}
	}
	
	/* Not EGA/VGA */
	else {
		m_uiColNum = m_stCtl.uiColNum;
		__asm__(
			"cld\n\t"
			"rep\n\t"
			"movsl\n\t"
			"movl _m_uiColNum, %%ecx\n\t"
			"rep\n\t"
			"stosw"
			::
			"a" (_ERASE_CHAR),
			"c" ((m_stCtl.uiScrRowBottom-m_stCtl.uiScrRowTop-1)*m_stCtl.uiColNum>>1),
			"D" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*m_stCtl.uiScrRowTop),
			"S" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*(m_stCtl.uiScrRowTop+1))
			:
			/* "cx","di","si" */
		);
	}
}

CPU_PRIVATE void cpu_disp_ScrollDown(void)
{
	_CHECK_DEVICE();
	
	if ((_VIDEO_TYPE_EGAC == m_stCtl.pstDev->uiType)
	||  (_VIDEO_TYPE_EGAM == m_stCtl.pstDev->uiType)) {
		m_uiColNum = m_stCtl.uiColNum;
		__asm__(
			"std\n\t"
			"rep\n\t"
			"movsl\n\t"
			"addl $2,%%edi\n\t"	/* %edi has been decremented by 4 */
			"movl _m_uiColNum,%%ecx\n\t"
			"rep\n\t"
			"stosw"
			::
			"a" (_ERASE_CHAR),
			"c" ((m_stCtl.uiScrRowBottom-m_stCtl.uiScrRowTop-1)*m_stCtl.uiColNum>>1),
			"D" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*m_stCtl.uiScrRowBottom-4),
			"S" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*(m_stCtl.uiScrRowBottom-1)-4)
			:
			/* "ax","cx","di","si" */
		);
	}
	else {
		m_uiColNum = m_stCtl.uiColNum;
		__asm__(
			"std\n\t"
			"rep\n\t"
			"movsl\n\t"
			"addl $2,%%edi\n\t"	/* %edi has been decremented by 4 */
			"movl _m_uiColNum,%%ecx\n\t"
			"rep\n\t"
			"stosw"
			::
			"a" (_ERASE_CHAR),
			"c" ((m_stCtl.uiScrRowBottom-m_stCtl.uiScrRowTop-1)*m_stCtl.uiColNum>>1),
			"D" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*m_stCtl.uiScrRowBottom-4),
			"S" (m_stCtl.uiScrMemStart+m_stCtl.uiRowPitch*(m_stCtl.uiScrRowBottom-1)-4)
			:
			/* "ax","cx","di","si" */
		);
	}
}

CPU_PRIVATE void cpu_disp_LineFeed(void)
{
	if (m_stCtl.uiPosRow+1<m_stCtl.uiScrRowBottom) {
		m_stCtl.uiPosRow++;
		m_stCtl.uiPosMem += m_stCtl.uiRowPitch;
		return;
	}
	cpu_disp_ScrollUp();
}


CPU_PRIVATE void cpu_disp_ReverseLineFeed(void)
{
	if (m_stCtl.uiPosRow>m_stCtl.uiScrRowTop) {
		m_stCtl.uiPosRow--;
		m_stCtl.uiPosMem -= m_stCtl.uiRowPitch;
		return;
	}
	cpu_disp_ScrollDown();
}

CPU_PRIVATE void cpu_disp_CarriageReturn(void)
{
	m_stCtl.uiPosMem -= m_stCtl.uiPosCol<<1;
	m_stCtl.uiPosCol = 0;
}

CPU_PRIVATE void cpu_disp_Delete(void)
{
	if (m_stCtl.uiPosCol) {
		m_stCtl.uiPosMem -= 2;
		m_stCtl.uiPosCol--;
		(*((CPU_INT16U *)(m_stCtl.uiPosMem))) = _ERASE_CHAR;
	}
}


