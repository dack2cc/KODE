/*
*********************************************************************************************************
*                                                uC/CPU
*                                    CPU CONFIGURATION & PORT LAYER
*
*                          (c) Copyright 2004-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/CPU is provided in source form to registered licensees ONLY.  It is 
*               illegal to distribute this source code to any third party unless you receive 
*               written permission by an authorized Micrium representative.  Knowledge of 
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest 
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            CPU PORT FILE
*
*                                        x86
*                                    GCC4 C compiler
*
* Filename      : cpu.h
* Version       : V0.01
* Programmer(s) : Jeremy
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*/

#ifndef  CPU_MODULE_PRESENT
#define  CPU_MODULE_PRESENT


/*
*********************************************************************************************************
*                                          CPU INCLUDE FILES
*
* Note(s) : (1) The following CPU files are located in the following directories :
*
*               (a) \<Your Product Application>\cpu_cfg.h
*
*               (b) \<CPU-Compiler Directory>\cpu_def.h
*
*               (c) \<CPU-Compiler Directory>\<cpu>\<compiler>\cpu*.*
*
*                       where
*                               <Your Product Application>      directory path for Your Product's Application
*                               <CPU-Compiler Directory>        directory path for common   CPU-compiler software
*                               <cpu>                           directory name for specific CPU
*                               <compiler>                      directory name for specific compiler
*
*           (2) Compiler MUST be configured to include the '\<CPU-Compiler Directory>\' directory & the
*               specific CPU-compiler directory as additional include path directories.
*
*           (3) Since NO custom library modules are included, 'cpu.h' may ONLY use configurations from 
*               CPU configuration file 'cpu_cfg.h' that do NOT reference any custom library definitions.
*
*               In other words, 'cpu.h' may use 'cpu_cfg.h' configurations that are #define'd to numeric 
*               constants or to NULL (i.e. NULL-valued #define's); but may NOT use configurations to 
*               custom library #define's (e.g. DEF_DISABLED or DEF_ENABLED).
*********************************************************************************************************
*/

#include  <cpu_def.h>
#include  <cpu_cfg.h>                                           /* See Note #3.                                         */


/*$PAGE*/
/*
*********************************************************************************************************
*                                    CONFIGURE STANDARD DATA TYPES
*
* Note(s) : (1) Configure standard data types according to CPU-/compiler-specifications.
*
*           (2) (a) (1) 'CPU_FNCT_VOID' data type defined to replace the commonly-used function pointer
*                       data type of a pointer to a function which returns void & has no arguments.
*
*                   (2) Example function pointer usage :
*
*                           CPU_FNCT_VOID  FnctName;
*
*                           FnctName();
*
*               (b) (1) 'CPU_FNCT_PTR'  data type defined to replace the commonly-used function pointer
*                       data type of a pointer to a function which returns void & has a single void
*                       pointer argument.
*
*                   (2) Example function pointer usage :
*
*                           CPU_FNCT_PTR   FnctName;
*                           void          *p_obj
*
*                           FnctName(p_obj);
*********************************************************************************************************
*/

typedef  void                  CPU_VOID;
typedef  char                  CPU_CHAR;                        /*  8-bit character                                     */
typedef  unsigned  char        CPU_BOOLEAN;                     /*  8-bit boolean or logical                            */
typedef  unsigned  char        CPU_INT08U;                      /*  8-bit unsigned integer                              */
typedef    signed  char        CPU_INT08S;                      /*  8-bit   signed integer                              */
typedef  unsigned  short       CPU_INT16U;                      /* 16-bit unsigned integer                              */
typedef    signed  short       CPU_INT16S;                      /* 16-bit   signed integer                              */
typedef  unsigned  long        CPU_INT32U;                      /* 32-bit unsigned integer                              */
typedef    signed  long        CPU_INT32S;                      /* 32-bit   signed integer                              */
typedef  unsigned  long  long  CPU_INT64U;                      /* 64-bit unsigned integer                              */
typedef    signed  long  long  CPU_INT64S;                      /* 64-bit   signed integer                              */

typedef            float       CPU_FP32;                        /* 32-bit floating point                                */
typedef            double      CPU_FP64;                        /* 64-bit floating point                                */


typedef  volatile  CPU_INT08U  CPU_REG08;                       /*  8-bit register                                      */
typedef  volatile  CPU_INT16U  CPU_REG16;                       /* 16-bit register                                      */
typedef  volatile  CPU_INT32U  CPU_REG32;                       /* 32-bit register                                      */
typedef  volatile  CPU_INT64U  CPU_REG64;                       /* 64-bit register                                      */


typedef            void      (*CPU_FNCT_VOID)(void);            /* See Note #2a.                                        */
typedef            void      (*CPU_FNCT_PTR )(void *p_obj);     /* See Note #2b.                                        */


/*
*********************************************************************************************************
*                                 CUSTOMIZE DEFINITION
*********************************************************************************************************
*/

//#define  CPU_PRIVATE  static
#define  CPU_PRIVATE  

#define  CPU_ERR_NO_MEMORY                                21u
#define  CPU_ERR_BAD_PARAM                                22u
#define  CPU_ERR_FATAL                                    23u
#define  CPU_ERR_INTERNAL                                 24u


/*$PAGE*/
/*
*********************************************************************************************************
*                                       CPU WORD CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_ADDR_SIZE & CPU_CFG_DATA_SIZE with CPU's word sizes :
*
*                   CPU_WORD_SIZE_08             8-bit word size
*                   CPU_WORD_SIZE_16            16-bit word size
*                   CPU_WORD_SIZE_32            32-bit word size
*                   CPU_WORD_SIZE_64            64-bit word size            See Note #1a
*
*               (a) 64-bit word size NOT currently supported.
*
*           (2) Configure CPU_CFG_ENDIAN_TYPE with CPU's data-word-memory order :
*
*               (a) CPU_ENDIAN_TYPE_BIG         Big-   endian word order (CPU words' most  significant
*                                                                         octet @ lowest memory address)
*               (b) CPU_ENDIAN_TYPE_LITTLE      Little-endian word order (CPU words' least significant
*                                                                         octet @ lowest memory address)
*********************************************************************************************************
*/

                                                                /* Define  CPU         word sizes (see Note #1) :       */
#define  CPU_CFG_ADDR_SIZE              CPU_WORD_SIZE_32        /* Defines CPU address word size  (in octets).          */
#define  CPU_CFG_DATA_SIZE              CPU_WORD_SIZE_32        /* Defines CPU data    word size  (in octets).          */
#define  CPU_CFG_DATA_SIZE_MAX          CPU_CFG_DATA_SIZE

#define  CPU_CFG_ENDIAN_TYPE            CPU_ENDIAN_TYPE_BIG     /* Defines CPU data    word-memory order (see Note #2). */


/*
*********************************************************************************************************
*                                 CONFIGURE CPU ADDRESS & DATA TYPES
*********************************************************************************************************
*/

                                                                /* CPU address type based on address bus size.          */
#if     (CPU_CFG_ADDR_SIZE == CPU_WORD_SIZE_64)
typedef  CPU_INT64U  CPU_ADDR;
#elif   (CPU_CFG_ADDR_SIZE == CPU_WORD_SIZE_32)
typedef  CPU_INT32U  CPU_ADDR;
#elif   (CPU_CFG_ADDR_SIZE == CPU_WORD_SIZE_16)
typedef  CPU_INT16U  CPU_ADDR;
#elif   (CPU_CFG_ADDR_SIZE == CPU_WORD_SIZE_8)
typedef  CPU_INT08U  CPU_ADDR;
#else  /* CPU_CFG_ADDR_SIZE */
#error   " CPU_CFG_ADDR_SIZE is Unknown (?_?) "
#endif /* CPU_CFG_ADDR_SIZE */

                                                                /* CPU data    type based on data    bus size.          */
#if     (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_64)
typedef  CPU_INT64U  CPU_DATA;
#elif   (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_32)
typedef  CPU_INT32U  CPU_DATA;
#elif   (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_16)
typedef  CPU_INT16U  CPU_DATA;
#elif   (CPU_CFG_DATA_SIZE == CPU_WORD_SIZE_8)
typedef  CPU_INT08U  CPU_DATA;
#else  /* CPU_CFG_DATA_SIZE */
#error   " CPU_CFG_DATA_SIZE is Unknown (?_?) "
#endif /* CPU_CFG_DATA_SIZE */


typedef  CPU_DATA    CPU_ALIGN;                                 /* Defines CPU data-word-alignment size.                */
typedef  CPU_ADDR    CPU_SIZE_T;                                /* Defines CPU standard 'size_t'   size.                */


/*
*********************************************************************************************************
*                                       CPU STACK CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_STK_GROWTH in 'cpu.h' with CPU's stack growth order :
*
*               (a) CPU_STK_GROWTH_LO_TO_HI     CPU stack pointer increments to the next higher  stack 
*                                                   memory address after data is pushed onto the stack
*               (b) CPU_STK_GROWTH_HI_TO_LO     CPU stack pointer decrements to the next lower   stack 
*                                                   memory address after data is pushed onto the stack
*********************************************************************************************************
*/

#define  CPU_CFG_STK_GROWTH     CPU_STK_GROWTH_HI_TO_LO         /* Defines CPU stack growth order (see Note #1).        */

typedef  CPU_INT32U             CPU_STK;                        /* Defines CPU stack word size (in octets).             */
typedef  CPU_ADDR               CPU_STK_SIZE;                   /* Defines CPU stack      size (in number of CPU_STKs). */


/*$PAGE*/
/*
*********************************************************************************************************
*                                   CRITICAL SECTION CONFIGURATION
*
* Note(s) : (1) Configure CPU_CFG_CRITICAL_METHOD with CPU's/compiler's critical section method :
*
*                                                       Enter/Exit critical sections by ...
*
*                   CPU_CRITICAL_METHOD_INT_DIS_EN      Disable/Enable interrupts
*                   CPU_CRITICAL_METHOD_STATUS_STK      Push/Pop       interrupt status onto stack
*                   CPU_CRITICAL_METHOD_STATUS_LOCAL    Save/Restore   interrupt status to local variable
*
*               (a) CPU_CRITICAL_METHOD_INT_DIS_EN  is NOT a preferred method since it does NOT support
*                   multiple levels of interrupts.  However, with some CPUs/compilers, this is the only
*                   available method.
*
*               (b) CPU_CRITICAL_METHOD_STATUS_STK    is one preferred method since it supports multiple
*                   levels of interrupts.  However, this method assumes that the compiler provides C-level
*                   &/or assembly-level functionality for the following :
*
*                     ENTER CRITICAL SECTION :
*                       (1) Push/save   interrupt status onto a local stack
*                       (2) Disable     interrupts
*
*                     EXIT  CRITICAL SECTION :
*                       (3) Pop/restore interrupt status from a local stack
*
*               (c) CPU_CRITICAL_METHOD_STATUS_LOCAL  is one preferred method since it supports multiple
*                   levels of interrupts.  However, this method assumes that the compiler provides C-level
*                   &/or assembly-level functionality for the following :
*
*                     ENTER CRITICAL SECTION :
*                       (1) Save    interrupt status into a local variable
*                       (2) Disable interrupts
*
*                     EXIT  CRITICAL SECTION :
*                       (3) Restore interrupt status from a local variable
*
*           (2) Critical section macro's most likely require inline assembly.  If the compiler does NOT
*               allow inline assembly in C source files, critical section macro's MUST call an assembly
*               subroutine defined in a 'cpu_a.asm' file located in the following software directory :
*
*                   \<CPU-Compiler Directory>\<cpu>\<compiler>\
*
*                       where
*                               <CPU-Compiler Directory>    directory path for common   CPU-compiler software
*                               <cpu>                       directory name for specific CPU
*                               <compiler>                  directory name for specific compiler
*
*           (3) (a) To save/restore interrupt status, a local variable 'cpu_sr' of type 'CPU_SR' MAY need 
*                   to be declared (e.g. if 'CPU_CRITICAL_METHOD_STATUS_LOCAL' method is configured).
*
*                   (1) 'cpu_sr' local variable SHOULD be declared via the CPU_SR_ALLOC() macro which, if 
*                        used, MUST be declared following ALL other local variables.
*
*                        Example :
*
*                           void  Fnct (void)
*                           {
*                               CPU_INT08U  val_08;
*                               CPU_INT16U  val_16;
*                               CPU_INT32U  val_32;
*                               CPU_SR_ALLOC();         MUST be declared after ALL other local variables
*                                   :
*                                   :
*                           }
*
*               (b) Configure 'CPU_SR' data type with the appropriate-sized CPU data type large enough to 
*                   completely store the CPU's/compiler's status word.
*********************************************************************************************************
*/
/*$PAGE*/
#define  KERNEL_INTERRUPT_MASK_LVL  12                          /* Used to disable interrupts with priority equal or less to this value */
                                                                /* Configure CPU critical method      (see Note #1) :   */
#define  CPU_CFG_CRITICAL_METHOD    CPU_CRITICAL_METHOD_STATUS_LOCAL

                                                                /* Allocates CPU status register word (see Note #3a).   */
#if     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
typedef  CPU_INT32U                 CPU_SR;                     /* Defines   CPU status register size (see Note #3b).   */
#define  CPU_SR_ALLOC()             CPU_SR  cpu_sr = (CPU_SR)0
#else  /* CPU_CRITICAL_METHOD_STATUS_LOCAL */
#define  CPU_SR_ALLOC()
#endif /* CPU_CRITICAL_METHOD_STATUS_LOCAL */


#if     (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_INT_DIS_EN)

#define  CPU_INT_DIS()         do { __asm__ volatile ("cli"::); } while (0)          /* Disable interrupts.                          */
#define  CPU_INT_EN()          do { __asm__ volatile ("sti"::); } while (0)          /* Enable  interrupts.                          */

#elif   (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_STK)

#define  CPU_INT_DIS()         do { __asm__ volatile ("pushfl \n\t cli"::); } while (0)
#define  CPU_INT_EN()          do { __asm__ volatile ("popfl"::); } while (0)

#elif   (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)

                                                                        /* Save    CPU status word & disable interrupts.*/
#define  CPU_INT_DIS()         do { __asm__ volatile ("pushfl \n\t popl %%eax":"=a"(cpu_sr):); __asm__ volatile ("cli"::); } while (0)
                                                                        /* Restore CPU status word.                     */
#define  CPU_INT_EN()          do { __asm__ volatile ("pushl %%eax \n\t popfl"::"a"(cpu_sr)); } while (0)

#else  /* CPU_CFG_CRITICAL_METHOD */

#error   " CPU_CFG_CRITICAL_METHOD is NOT supported (=_=!) "

#endif /* CPU_CFG_CRITICAL_METHOD */



#ifdef   CPU_CFG_INT_DIS_MEAS_EN
                                                                        /* Disable interrupts, ...                      */
                                                                        /* & start interrupts disabled time measurement.*/
#define  CPU_CRITICAL_ENTER()  do { CPU_INT_DIS();         \
                                    CPU_IntDisMeasStart(); } while (0)
                                                                        /* Stop & measure   interrupts disabled time,   */
                                                                        /* ...  & re-enable interrupts.                 */
#define  CPU_CRITICAL_EXIT()   do { CPU_IntDisMeasStop();  \
                                    CPU_INT_EN();          } while (0)

#else   /* CPU_CFG_INT_DIS_MEAS_EN */

#define  CPU_CRITICAL_ENTER()       CPU_INT_DIS()                       /* Disable   interrupts.                        */
#define  CPU_CRITICAL_EXIT()        CPU_INT_EN()                        /* Re-enable interrupts.                        */

#endif  /* CPU_CFG_INT_DIS_MEAS_EN */


#define  CPU_IntDis()          do { __asm__ volatile ("cli"::); } while (0)          /* Disable interrupts.                          */
#define  CPU_IntEn()           do { __asm__ volatile ("sti"::); } while (0)          /* Enable  interrupts.                          */


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*/

#ifndef   CPU_CFG_ADDR_SIZE
#error   "CPU_CFG_ADDR_SIZE              not #define'd in 'cpu.h'               "
#error   "                         [MUST be  CPU_WORD_SIZE_08   8-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_16  16-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_32  32-bit alignment]"

#elif   ((CPU_CFG_ADDR_SIZE != CPU_WORD_SIZE_08) && \
         (CPU_CFG_ADDR_SIZE != CPU_WORD_SIZE_16) && \
         (CPU_CFG_ADDR_SIZE != CPU_WORD_SIZE_32))
#error   "CPU_CFG_ADDR_SIZE        illegally #define'd in 'cpu.h'               "
#error   "                         [MUST be  CPU_WORD_SIZE_08   8-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_16  16-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_32  32-bit alignment]"
#endif


#ifndef   CPU_CFG_DATA_SIZE
#error   "CPU_CFG_DATA_SIZE              not #define'd in 'cpu.h'               "
#error   "                         [MUST be  CPU_WORD_SIZE_08   8-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_16  16-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_32  32-bit alignment]"

#elif   ((CPU_CFG_DATA_SIZE != CPU_WORD_SIZE_08) && \
         (CPU_CFG_DATA_SIZE != CPU_WORD_SIZE_16) && \
         (CPU_CFG_DATA_SIZE != CPU_WORD_SIZE_32))
#error   "CPU_CFG_DATA_SIZE        illegally #define'd in 'cpu.h'               "
#error   "                         [MUST be  CPU_WORD_SIZE_08   8-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_16  16-bit alignment]"
#error   "                         [     ||  CPU_WORD_SIZE_32  32-bit alignment]"
#endif




#ifndef   CPU_CFG_ENDIAN_TYPE
#error   "CPU_CFG_ENDIAN_TYPE            not #define'd in 'cpu.h'   "
#error   "                         [MUST be  CPU_ENDIAN_TYPE_BIG   ]"
#error   "                         [     ||  CPU_ENDIAN_TYPE_LITTLE]"

#elif   ((CPU_CFG_ENDIAN_TYPE != CPU_ENDIAN_TYPE_BIG   ) && \
         (CPU_CFG_ENDIAN_TYPE != CPU_ENDIAN_TYPE_LITTLE))
#error   "CPU_CFG_ENDIAN_TYPE      illegally #define'd in 'cpu.h'   "
#error   "                         [MUST be  CPU_ENDIAN_TYPE_BIG   ]"
#error   "                         [     ||  CPU_ENDIAN_TYPE_LITTLE]"
#endif




#ifndef   CPU_CFG_STK_GROWTH
#error   "CPU_CFG_STK_GROWTH             not #define'd in 'cpu.h'    "
#error   "                         [MUST be  CPU_STK_GROWTH_LO_TO_HI]"
#error   "                         [     ||  CPU_STK_GROWTH_HI_TO_LO]"

#elif   ((CPU_CFG_STK_GROWTH != CPU_STK_GROWTH_LO_TO_HI) && \
         (CPU_CFG_STK_GROWTH != CPU_STK_GROWTH_HI_TO_LO))
#error   "CPU_CFG_STK_GROWTH       illegally #define'd in 'cpu.h'    "
#error   "                         [MUST be  CPU_STK_GROWTH_LO_TO_HI]"
#error   "                         [     ||  CPU_STK_GROWTH_HI_TO_LO]"
#endif




#ifndef   CPU_CFG_CRITICAL_METHOD
#error   "CPU_CFG_CRITICAL_METHOD        not #define'd in 'cpu.h'             "
#error   "                         [MUST be  CPU_CRITICAL_METHOD_INT_DIS_EN  ]"
#error   "                         [     ||  CPU_CRITICAL_METHOD_STATUS_STK  ]"
#error   "                         [     ||  CPU_CRITICAL_METHOD_STATUS_LOCAL]"

#elif   ((CPU_CFG_CRITICAL_METHOD != CPU_CRITICAL_METHOD_INT_DIS_EN  ) && \
         (CPU_CFG_CRITICAL_METHOD != CPU_CRITICAL_METHOD_STATUS_STK  ) && \
         (CPU_CFG_CRITICAL_METHOD != CPU_CRITICAL_METHOD_STATUS_LOCAL))
#error   "CPU_CFG_CRITICAL_METHOD  illegally #define'd in 'cpu.h'             "
#error   "                         [MUST be  CPU_CRITICAL_METHOD_INT_DIS_EN  ]"
#error   "                         [     ||  CPU_CRITICAL_METHOD_STATUS_STK  ]"
#error   "                         [     ||  CPU_CRITICAL_METHOD_STATUS_LOCAL]"
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif                                                          /* End of CPU module include.                           */
