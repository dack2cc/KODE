/* Sample KD/kdplatform.h for OpenKODE Core 1.0.3  */
#ifndef __kdplatform_h_
#define __kdplatform_h_

#define KD_API
#define KD_APIENTRY

typedef signed long KDint32;
typedef unsigned long KDuint32;
typedef signed long long KDint64;
typedef unsigned long long KDuint64;
typedef signed short KDint16;
typedef unsigned short KDuint16;
typedef unsigned long KDuintptr;
typedef unsigned long KDsize;
typedef signed long KDssize;
#define KDINT_MIN (-0x7fffffff-1)
#define KDINT_MAX 0x7fffffff
#define KDUINT_MAX 0xffffffffU
#define KDINT64_MIN (-0x7fffffffffffffffLL-1)
#define KDINT64_MAX 0x7fffffffffffffffLL
#define KDUINT64_MAX 0xffffffffffffffffULL
#define KDSSIZE_MIN (-0x7fffffff-1)
#define KDSSIZE_MAX 0x7fffffff
#define KDSIZE_MAX 0xffffffffU
#define KDUINTPTR_MAX 0xffffffffU
#define KD_NORETURN
#define KD_INFINITY (1.0F/0.0F)
//#define KD_WINDOW_SUPPORTED
#ifdef KD_NDEBUG
#define kdAssert(c)
#else
#define kdAssert(c) ((void)( (c) ? 0 : (kdHandleAssertion(#c, __FILE__, __LINE__), 0)))
#endif

#endif /* __kdplatform_h_ */

