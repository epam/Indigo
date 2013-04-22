#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
void *ICEP(void* volatile* d, void* e, void* c) {
   return InterlockedCompareExchangePointer(d, e, c);
}
#endif