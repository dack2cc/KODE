/******************************************************************************
    Include
******************************************************************************/

#include <kd/kd.h>
#include <cpu_core.h>

/******************************************************************************
    Private Define
******************************************************************************/

//#define KD_PRIVATE  static
#define KD_PRIVATE

KD_PRIVATE const KDuint32  kd_core_4ccVender   = CPU_TYPE_CREATE('K', 'O', 'D', 'O');
KD_PRIVATE const KDchar*   kd_core_strVender   = "kokodo";
KD_PRIVATE const KDuint32  kd_core_4ccVersion  = CPU_TYPE_CREATE('0', '0', '0', '1');
KD_PRIVATE const KDchar*   kd_core_strVersion  = "00.01";
KD_PRIVATE const KDuint32  kd_core_4ccPlatform = CPU_TYPE_CREATE('i', '3', '8', '6');
KD_PRIVATE const KDchar*   kd_core_strPlatform = "i386";


/******************************************************************************
    Private Interface
******************************************************************************/


/******************************************************************************
    Function Definition
******************************************************************************/

/*
    Versioning and attribute queries
*/

/* kdQueryAttribi: Obtain the value of a numeric OpenKODE Core attribute. */
KDint KD_APIENTRY kdQueryAttribi(KDint attribute, KDint *value)
{
    if (KD_NULL == value) {
        kdSetError(KD_EACCES);
        return (KD_EACCES);
    }

    switch (attribute) {
    case KD_ATTRIB_VENDOR:
        (*value) = kd_core_4ccVender;
        break;

    case KD_ATTRIB_VERSION:
        (*value) = kd_core_4ccVersion;
        break;

    case KD_ATTRIB_PLATFORM:
        (*value) = kd_core_4ccPlatform;
        break;

    default:
        kdSetError(KD_EINVAL);
        return (KD_EINVAL);
        break;
    }

    return (0);

}

/* kdQueryAttribcv: Obtain the value of a string OpenKODE Core attribute. */
const KDchar * KD_APIENTRY kdQueryAttribcv(KDint attribute)
{
    switch (attribute) {
    case KD_ATTRIB_VENDOR:
        return (kd_core_strVender);
        break;

    case KD_ATTRIB_VERSION:
        return (kd_core_strVersion);
        break;

    case KD_ATTRIB_PLATFORM:
        return (kd_core_strPlatform);
        break;

    default:
        /* empty */
        break;
    }

    kdSetError(KD_EINVAL);
    return (KD_NULL);
}

/* kdQueryIndexedAttribcv: Obtain the value of an indexed string OpenKODE Core attribute. */
const KDchar *KD_APIENTRY kdQueryIndexedAttribcv(KDint attribute, KDint index)
{
    kdSetError(KD_EINVAL);
    return (KD_NULL);
}



