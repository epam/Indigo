#include "base_cpp/shared_ptr.h"

RedBlackMap<void *, int> SharedPtrStaticData::_counters;
OsLock SharedPtrStaticData::_lock;
