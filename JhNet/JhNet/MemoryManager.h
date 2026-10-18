#pragma once
#include "MemoryPool.h"

typedef struct MemoryManager
{
	MemoryPool* _memoryPools[DEFAULT_MAX_BUCKETSIZE + 1];
} MemoryManager;

MemoryManager*	MemoryManager_Make();
void			MemoryManager_Release(MemoryManager* memoryManager);
MemoryHeader*	MemoryManager_Alloc(MemoryManager* memoryManager, DWORD size);
void			MemoryManager_Free(MemoryManager* memoryManager, MemoryHeader* ptr);
