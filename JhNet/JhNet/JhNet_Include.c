#include "pch.h"
#define DLL_EXPORT
#include "JhNet_Include.h"

DLL_PROC void JhNet_Send(BYTE* buffer, DWORD size)
{
	return;
}

DLL_PROC MemoryPool* JhNet_MakeMemoryPool()
{
	return NULL;
}

DLL_PROC void* JhNet_Alloc(MemoryPool* pool, unsigned int allocSize)
{
	return NULL;
}

DLL_PROC void JhNet_Free(MemoryPool* pool, void* ptr)
{
}