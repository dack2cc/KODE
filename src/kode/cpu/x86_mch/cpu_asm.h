#ifndef __CPU_ASM_H__
#define __CPU_ASM_H__

/******************************************************************************
    Definition
******************************************************************************/

#ifdef i486
#define TEXT_ALIGN	4
#else  // i486
#define TEXT_ALIGN	2
#endif // i486
#define DATA_ALIGN	2
#define ALIGN		TEXT_ALIGN


#ifdef   __STDC__
//#ifndef  __ELF__
#    define  EXT(x)   _ ## x
#    define LEXT(x)   _ ## x ## :
#    define SEXT(x)   "_"#x
//#else   // __ELF__
//#    define  EXT(x)  x
//#    define LEXT(x)  x ## :
//#    define SEXT(x)  #x
//#endif  // __ELF__
#else   // __STDC__
#    error  XXX elf
#    define  EXT(x)   _/**/x
#    define LEXT(x)   _/**/x/**/:
#    define LCLL(x)   x/**/:
#endif  // __STDC__ 


#ifdef GPROF
#  define  ENTRY(x)  .globl EXT(x); .p2align TEXT_ALIGN; LEXT(x) ; \
                     pushl %ebp; movl %esp, %ebp; MCOUNT; popl %ebp;
#else	/* GPROF */
#  define  ENTRY(x)  .globl EXT(x); .p2align TEXT_ALIGN; LEXT(x)
#endif	/* GPROF */


#endif // __CPU_ASM_H__

