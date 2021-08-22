#ifndef __bingo_lock__
#define __bingo_lock__

#include "base_cpp/os_sync_wrapper.h"

namespace bingo
{
    struct DatabaseLockData
    {
        indigo::OsSemaphore rc_sem, wc_sem, w_sem, r_sem;
        int writers_count, readers_count;

        DatabaseLockData();
    };

    struct ReadLock
    {
    public:
        ReadLock(DatabaseLockData& data);

        ~ReadLock();

    private:
        DatabaseLockData& _data;
    };

    struct WriteLock
    {
    public:
        WriteLock(DatabaseLockData& data);

        ~WriteLock();

    private:
        DatabaseLockData& _data;
    };
} // namespace bingo

#endif //__bingo_lock__
