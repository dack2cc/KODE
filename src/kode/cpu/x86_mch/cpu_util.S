#include <cpu_asm.h>
#include <cpu_eflags.h>

/* Discover what kind of cpu we have; return the family number
   (3, 4, 5, 6, for 386, 486, 586, 686 respectively).  */
ENTRY(cpu_util_DiscoverX86Type)
	pushl	%ebp			/* Save frame pointer */
	movl	%esp,%ebp		/* Save stack pointer */
	and	$~0x3,%esp		/* Align stack pointer */

#ifdef	MACH_HYP
#warning Assuming not Cyrix CPU
#else	/* MACH_HYP */
	inb	$0xe8,%al		/* Enable ID flag for Cyrix CPU ... */
	andb	$0x80,%al		/* ... in CCR4 reg bit7 */
	outb	%al,$0xe8
#endif	/* MACH_HYP */

	pushfl				/* Fetch flags ... */
	popl	%eax			/*  ... into eax */
	movl	%eax,%ecx		/* Save original flags for return */
	xorl	$(EFL_AC+EFL_ID),%eax	/* Attempt to toggle ID and AC bits */
	pushl	%eax			/* Save flags... */
	popfl				/*  ... In EFLAGS */
	pushfl				/* Fetch flags back ... */
	popl	%eax			/*  ... into eax */
	pushl	%ecx			/* From ecx... */
	popfl				/* ... restore original flags */

	xorl	%ecx,%eax		/* See if any bits didn't change */
	testl	$EFL_AC,%eax		/* Test AC bit */
	jnz	0f			/* Skip next bit if AC toggled */
	movl	$3,%eax			/*   Return value is 386 */
	jmp	9f			/*   And RETURN */

0:	testl	$EFL_ID,%eax		/* Test ID bit */
	jnz	0f			/* Skip next bit if ID toggled */
	movl	$4,%eax			/*   Return value is 486 */
	jmp	9f			/*   And RETURN */

	/* We are a modern enough processor to have the CPUID instruction;
	   use it to find out what we are. */
0:	movl	$1,%eax			/* Fetch CPU type info ... */
	cpuid				/*  ... into eax */
	//movl	%edx,cpu_features	/* Keep a copy */
	shrl	$8,%eax			/* Slide family bits down */
	andl	$15,%eax		/* And select them */

9:	movl	%ebp,%esp		/* Restore stack pointer */
	popl	%ebp			/* Restore frame pointer */
	ret				/* And return */


