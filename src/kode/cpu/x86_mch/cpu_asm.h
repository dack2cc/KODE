#ifndef __CPU_ASM_H__
#define __CPU_ASM_H__


#ifdef   __STDC__
#ifndef  __ELF__

#    define EXT(x)   _ ## x

#else   // __ELF__

#    define EXT(x)   x

#endif  // __ELF__
#else   // __STDC__

#    error  XXX elf
#    define EXT(x)   _/**/x

#endif  // __STDC__ 



#endif // __CPU_ASM_H__

