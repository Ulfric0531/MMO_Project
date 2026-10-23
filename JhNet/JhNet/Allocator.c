#include "pch.h"

void* PoolAlloc(DWORD allocSize)
{
    if (allocSize > DEFAULT_MAX_BUCKETSIZE - sizeof(MemoryHeader))
    {
        void* ptr = malloc(allocSize + sizeof(MemoryHeader));
        return MemoryHeader_Attach(ptr, allocSize);
    }
    MemoryHeader* ptr = MemoryManager_Alloc(NULL, allocSize);
    return MemoryHeader_Attach(ptr, allocSize);
}

void PoolFree(void* ptr)
{
    MemoryHeader* freePtr = MemoryHeader_Detach(ptr);
    if (freePtr->_allocSize > DEFAULT_MAX_BUCKETSIZE - sizeof(MemoryHeader))
    {
        free(freePtr);
        return;
    }
    MemoryManager_Free(NULL, freePtr);
}
