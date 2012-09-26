#include "base_cpp/shared_ptr.h"

using namespace indigo;

RedBlackMap<void *, int> SharedPtrStaticData::_counters;
OsLock SharedPtrStaticData::_lock;

IMPL_EXCEPTION(indigo, SharedPtrError, "SharedPtr error");
