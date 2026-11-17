#ifdef DLL_EXPORT 
#define DLL_PROC __declspec(dllexport)
#else
#define DLL_PROC __declspec(dllimport)
#endif

typedef struct MemoryPool MemoryPool;

DLL_PROC LONG JhNet_ServerServiceStart(DWORD maxPeerCount, DWORD iocpThreadCount);
DLL_PROC LONG JhNet_ClientServiceStart(DWORD maxPeerCount, DWORD iocpThreadCount);
DLL_PROC BOOL JhNet_Connect(LONG serviceHandle, WCHAR* serverIp, WORD serverPort);
DLL_PROC BOOL JhNet_Send(BYTE* buffer, DWORD size);

// ----------Recommended only for C/C++ projects.----------
DLL_PROC MemoryPool*	JhNet_MakeMemoryPool(); 
DLL_PROC void*			JhNet_Alloc(MemoryPool* pool, unsigned int allocSize);
DLL_PROC void			JhNet_Free(MemoryPool* pool, void* ptr);
// --------------------------------------------------------