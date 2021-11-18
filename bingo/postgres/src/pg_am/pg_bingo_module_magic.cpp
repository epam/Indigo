#include <base_c/defs.h>

extern "C"
{
#include <postgres.h>
#include <fmgr.h>
#ifdef __MINGW32__
DLLEXPORT PG_MODULE_MAGIC;
#else
PG_MODULE_MAGIC;
#endif
}
