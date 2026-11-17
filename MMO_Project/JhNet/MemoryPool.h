#pragma once
#include "pch.h"

#define DEFAULT_MAX_BUCKETSIZE 4096

#ifdef _DEBUG
#define MemoryHeader_Attach(ptr, size) \
MemoryHeader_Attach_Debug(ptr, size, __FILE__, __LINE__)
#else
#define MemoryHeader_Attach(ptr, size) \
MemoryHeader_Attach_Release(ptr, size)
#endif


typedef struct MemoryHeader
{
	DWORD _allocSize;
#ifdef _DEBUG
	DWORD		_lineNum;
	const char* _fileName;
#endif
	struct MemoryHeader*	_next;

} MemoryHeader;

typedef struct Arena
{
	BYTE*			_arena;
	struct Arena*	_next;
	DWORD			_arenaPos;
} Arena;

typedef struct MemoryPool
{
	volatile LONG _poolSpinLock;
	volatile LONG _arenaSpinLock;

	DWORD _bucketCount;
	DWORD _allocCount;
	DWORD _bucketSize;
	DWORD _arenaCount;
	DWORD _arenaSize;
	
	struct MemoryHeader	_buckethead;
	Arena				_arenaHead;
} MemoryPool;

#ifdef _DEBUG
void* MemoryHeader_Attach_Debug(void* ptr, DWORD size, const char* fileName, DWORD lineNum);
#else 
void* MemoryHeader_Attach_Release(void* ptr, DWORD size);
#endif

MemoryHeader*	MemoryHeader_Detach(void* ptr);

MemoryPool*		MemoryPool_Make(DWORD bucketSize, DWORD reserveCount);
void			MemoryPool_Release(MemoryPool* pool);
void			MemoryPool_Push(MemoryPool* pool, MemoryHeader* ptr);
MemoryHeader*	MemoryPool_Pop(MemoryPool* pool);
DWORD			MemoryPool_CalculateArenaSize(DWORD bucketSize);