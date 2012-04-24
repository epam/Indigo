#ifdef _MSC_VER
#include <windows.h>
void *ICEP(void* volatile* d, void* e, void* c) {
   return InterlockedCompareExchangePointer(d, e, c);
}
#endif