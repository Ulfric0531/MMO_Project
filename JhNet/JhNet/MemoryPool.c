#include "pch.h"

#define CACHE_LINE (64)
#define MEM_PAGE_SIZE (4096)
#define EXPECTED_ARENA_SIZE (1024 * 1024)

#ifdef _DEBUG
void* MemoryHeader_Attach_Debug(void* ptr, DWORD size, const char* fileName, DWORD lineNum)
{
    MemoryHeader* header = (MemoryHeader*)ptr;
    header->_allocSize = size;
    header->_fileName = fileName;
    header->_lineNum = lineNum;
    return (void*)(++header);
}
#else
void* MemoryHeader_Attach_Release(void* ptr, DWORD size)
{
    MemoryHeader* header = (MemoryHeader*)ptr;
    header->_allocSize = size;

    return (void*)(++header);
}
#endif


MemoryHeader* MemoryHeader_Detach(void* ptr)
{
    return (MemoryHeader*)(ptr) - 1;
}

MemoryPool* MemoryPool_Make(DWORD bucketSize, DWORD reserveCount)
{
    MemoryPool* newPool = (MemoryPool*)malloc(sizeof(MemoryPool));
    if (newPool == NULL)
    {
        // TODO: 俊矾贸府
    }
    newPool->_poolSpinLock = 0;
    newPool->_arenaSpinLock = 0;
    newPool->_bucketCount = 0;
    newPool->_allocCount = 0;
    newPool->_bucketSize = bucketSize;
    newPool->_arenaCount = 1;
    newPool->_buckethead._next = NULL;

    DWORD arenaSize = MemoryPool_CalculateArenaSize(bucketSize);
    newPool->_arenaSize = arenaSize;
    Arena* arena = (Arena*)malloc(sizeof(Arena));
    arena->_arena = (BYTE*)VirtualAlloc(NULL, arenaSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    arena->_arenaPos = 0;
    arena->_next = NULL;
    newPool->_arenaHead._next = arena;

    return newPool;
}

void MemoryPool_Release(MemoryPool* pool)
{
#ifdef _DEBUG
    Arena* arena = pool->_arenaHead._next;
    while (arena != NULL)
    {
        DWORD pos = 0;
        while (pos < pool->_arenaSize)
        {
            MemoryHeader* header = (MemoryHeader*)(arena->_arena + pos);
            if (header->_allocSize != 0)
            {
                printf("Memory Leak Detected!\n");
                printf("Bucket Size : %d", pool->_bucketSize);
                printf("%s : %d", header->_fileName, header->_lineNum);
            }

            pos += pool->_bucketSize;
        }
        arena = arena->_next;
    }
#endif
    
    // TODO : Arena free 贸府
}


void MemoryPool_Push(MemoryPool* pool, MemoryHeader* ptr)
{
    ptr->_allocSize = 0;

    AcquireSpinLock(&pool->_poolSpinLock);
    ptr->_next = (pool->_buckethead)._next;
    (pool->_buckethead)._next = ptr;
    ++pool->_bucketCount;
    --pool->_allocCount;
    ReleaseSpinLock(&pool->_poolSpinLock);
}

MemoryHeader* MemoryPool_Pop(MemoryPool* pool)
{
    AcquireSpinLock(&pool->_poolSpinLock);
    MemoryHeader* header = pool->_buckethead._next;
    if (header == NULL)
    {
        Arena* arena = pool->_arenaHead._next;
        if (pool->_arenaSize - arena->_arenaPos == 0)
        {
            Arena* newArena = (Arena*)malloc(sizeof(Arena));
            newArena->_arenaPos = 0;
            newArena->_arena = (BYTE*)VirtualAlloc(NULL, pool->_arenaSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            newArena->_next = arena;
            pool->_arenaHead._next = newArena;
            arena = newArena;
            ++pool->_arenaCount;
        }
        header = (MemoryHeader*)(arena->_arena + arena->_arenaPos);
        arena->_arenaPos += pool->_bucketSize;
        goto exit;
    }
    pool->_buckethead._next = header->_next;
exit:
    --pool->_bucketCount;
    ++pool->_allocCount;
    ReleaseSpinLock(&pool->_poolSpinLock);
    return header;
}

DWORD MemoryPool_CalculateArenaSize(DWORD bucketSize)
{
    DWORD a = bucketSize;
    DWORD b = MEM_PAGE_SIZE;

    while (b != 0)
    {
        DWORD temp = b;
        b = a % b;
        a = temp;
    }
    DWORD minArenaSize = (bucketSize * MEM_PAGE_SIZE) / a;
    DWORD result = ((EXPECTED_ARENA_SIZE) / minArenaSize) * minArenaSize;
    return result;
}
