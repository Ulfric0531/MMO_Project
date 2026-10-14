#ifdef DLL_EXPORT 
#define DLL_PROC __declspec(dllexport)
#else
#define DLL_PROC __declspec(dllimport)
#endif

DLL_PROC void*	jhnet_alloc(unsigned int allocSize);
DLL_PROC void	jhnet_release(void* ptr);