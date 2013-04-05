
/******************************************************************************
    Include
******************************************************************************/

#include <cpu_disp.h>
#include <cpu_boot.h>
#include <cpu_ext.h>

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
} DISPLAY_CONTROL;

CPU_PRIVATE  DISPLAY_CONTROL  m_stCtl;


//static CPU_INT08U  m_byFontAttr = 0x07;  // BG:Black; Color:White
#define _FONT_ATTR    (0x07)
#define _ERASE_CHAR   ((_FONT_ATTR << 8) + 0x20)


/******************************************************************************
    Private Interface
******************************************************************************/



/******************************************************************************
    Function Definition
******************************************************************************/

void cpu_disp_Init(void)
{
	CPU_CHAR* pbyDisplayName = 0;
	CPU_CHAR* pbyDisplayCursor = 0;
	CPU_INT08S  i = 0;

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
	
	/* the cursor position */
	m_stCtl.uiPosCol = X86_DISPLAY_POS_COL;
	m_stCtl.uiPosRow = X86_DISPLAY_POS_ROW;
	m_stCtl.uiPosMem = m_stCtl.pstDev->uiMemStart + m_stCtl.uiPosRow * m_stCtl.uiRowPitch + m_stCtl.uiPosCol * 2;
	
	return;
}


CPU_INT32U  CPUExt_DispPrint(const CPU_CHAR* pszStr_in)
{
	CPU_CHAR*  pbyChar  = 0;
	CPU_INT32U iCharCnt = 0;
	
	if (0 == pszStr_in) {
		return (0);
	}
	
	pbyChar = (CPU_CHAR *)pszStr_in;
	while ('\0' != (*pbyChar)) {
		if (((*pbyChar) > 31) && ((*pbyChar) < 127)) {
			(*(CPU_CHAR *)(m_stCtl.uiPosMem)) = (*pbyChar);
			
		    m_stCtl.uiPosMem += 2;
		    m_stCtl.uiPosCol += 1;
		    if (m_stCtl.uiPosCol > m_stCtl.uiColNum) {
			    m_stCtl.uiPosCol = 0;
			    m_stCtl.uiPosRow += 1;
		    }
		}
		else if ('\n' == (*pbyChar)) {
			m_stCtl.uiPosRow += 1;
			m_stCtl.uiPosCol = 0;
			m_stCtl.uiPosMem = m_stCtl.pstDev->uiMemStart + m_stCtl.uiPosRow * m_stCtl.uiRowPitch;
		}
		else {
			// EMPTY
		}
		
		++pbyChar;
		++iCharCnt;
	}
	
	return (iCharCnt);
}


