#include "pch.h"

MemoryManager* MemoryManager_Make()
{
	MemoryManager* memoryManager = (MemoryManager*)malloc(sizeof(MemoryManager));
	DWORD bucketSize = 32;
	DWORD sameBucket = 1;
	for (; bucketSize < 512; bucketSize += 32)
	{
		MemoryPool* pool = MemoryPool_Make(bucketSize, 0);
		for (; sameBucket <= bucketSize; ++sameBucket)
		{
			memoryManager->_memoryPools[sameBucket] = pool;
		}
	}

	for (; bucketSize < 1024; bucketSize += 64)
	{
		MemoryPool* pool = MemoryPool_Make(bucketSize, 0);
		for (; sameBucket <= bucketSize; ++sameBucket)
		{
			memoryManager->_memoryPools[sameBucket] = pool;
		}
	}

	for (; bucketSize < 2048; bucketSize += 256)
	{
		MemoryPool* pool = MemoryPool_Make(bucketSize, 0);
		for (; sameBucket <= bucketSize; ++sameBucket)
		{
			memoryManager->_memoryPools[sameBucket] = pool;
		}
	}

	for (; bucketSize < DEFAULT_MAX_BUCKETSIZE; bucketSize += 1024)
	{
		MemoryPool* pool = MemoryPool_Make(bucketSize, 0);
		for (; sameBucket <= bucketSize; ++sameBucket)
		{
			memoryManager->_memoryPools[sameBucket] = pool;
		}
	}
	return memoryManager;
}

void MemoryManager_Release(MemoryManager* memoryManager)
{
	// TODO
}

MemoryHeader* MemoryManager_Alloc(MemoryManager* memoryManager, DWORD size)
{
	return MemoryPool_Pop(memoryManager->_memoryPools[size + sizeof(MemoryHeader)]);
}

void MemoryManager_Free(MemoryManager* memoryManager, MemoryHeader* ptr)
{
	MemoryPool_Push(memoryManager->_memoryPools[ptr->_allocSize + sizeof(MemoryHeader)], ptr);
}
