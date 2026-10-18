#pragma once
#include "MemoryManager.h"

void* PoolAlloc(DWORD allocSize);
void  PoolFree(void* ptr);