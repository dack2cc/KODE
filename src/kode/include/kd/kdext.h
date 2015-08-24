
#ifndef __kdext_h_
#define __kdext_h_

#ifdef __cplusplus
extern "C" {
#endif

#include "kd.h"

KD_API void  KD_APIENTRY  kdextInit(void);
KD_API void  KD_APIENTRY  kdextRun(void);
KD_API void  KD_APIENTRY  kdextSetup(void);

typedef struct KDExtProcess KDExtProcess;
KD_API KDExtProcess *   KD_APIENTRY kdextProcessCreate(void * (*start_routine)(void *), void *arg);
KD_API KD_NORETURN void KD_APIENTRY kdextProcessExit(void * retval);

#ifdef __cplusplus
}
#endif

#endif /* __kdext_h_ */

