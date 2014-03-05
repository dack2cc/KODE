
#ifndef __FONT_H__
#define __FONT_H__

/******************************************************************************
    Include
******************************************************************************/

#include <cpu.h>

/******************************************************************************
    Public Interface
******************************************************************************/

#define FONT_TYPE_HANKAKU  (0)
#define FONT_TYPE    FONT_TYPE_HANKAKU

#if (FONT_TYPE == FONT_TYPE_HANKAKU)
extern  CPU_INT08U   font_hankaku[];
#define FONT_DATA    font_hankaku
#define FONT_PITCH   (16)
#define FONT_W       (8)
#define FONT_H       (16)
#else  // FONT_TYPE
#error No Supproted Font Type
#endif // FONT_TYPE

#endif // __FONT_H__

