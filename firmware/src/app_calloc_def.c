
#include "app_calloc_def.h"


void* APP_Calloc(size_t nElems, size_t elemSize)
{
    size_t nBytes = nElems * elemSize;

    void* ptr = OSAL_Malloc(nBytes);
    if (ptr)
    {
        memset(ptr, 0, nBytes);
    }

    return ptr;
}