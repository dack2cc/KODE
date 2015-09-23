#ifndef __CPU_EFLAGS_H__
#define __CPU_EFLAGS_H__

/******************************************************************************
    Definition
******************************************************************************/

/*
 *	i386 flags register
 */
#define	EFL_CF		0x00000001		/* carry */
#define	EFL_PF		0x00000004		/* parity of low 8 bits */
#define	EFL_AF		0x00000010		/* carry out of bit 3 */
#define	EFL_ZF		0x00000040		/* zero */
#define	EFL_SF		0x00000080		/* sign */
#define	EFL_TF		0x00000100		/* trace trap */
#define	EFL_IF		0x00000200		/* interrupt enable */
#define	EFL_DF		0x00000400		/* direction */
#define	EFL_OF		0x00000800		/* overflow */
#define	EFL_IOPL	0x00003000		/* IO privilege level: */
#define	EFL_IOPL_KERNEL	0x00000000			/* kernel */
#define	EFL_IOPL_USER	0x00003000			/* user */
#define	EFL_NT		0x00004000		/* nested task */
#define	EFL_RF		0x00010000		/* resume without tracing */
#define	EFL_VM		0x00020000		/* virtual 8086 mode */
#define	EFL_AC		0x00040000		/* alignment check */
#define	EFL_VI		0x00080000		/* virtual interrupt */
#define	EFL_VIP		0x00100000		/* virtual interrupt pending */
#define	EFL_ID		0x00200000		/* cpuid available */

#endif // __CPU_EFLAGS_H__

