#include "bingo_lock.h"

using namespace bingo;

DatabaseLockData::DatabaseLockData() : writers_count(0), readers_count(0)
{
    osSemaphoreCreate(&rc_sem, 1, 1);
    osSemaphoreCreate(&wc_sem, 1, 1);
    osSemaphoreCreate(&w_sem, 1, 1);
    osSemaphoreCreate(&r_sem, 1, 1);
}

ReadLock::ReadLock(DatabaseLockData& data) : _data(data)
{
    osSemaphoreWait(&_data.r_sem);
    osSemaphoreWait(&_data.rc_sem);
    _data.readers_count++;
    if (_data.readers_count == 1)
        osSemaphoreWait(&_data.w_sem);
    osSemaphorePost(&_data.rc_sem);
    osSemaphorePost(&_data.r_sem);
}

ReadLock::~ReadLock()
{
    osSemaphoreWait(&_data.rc_sem);
    _data.readers_count--;
    if (_data.readers_count == 0)
        osSemaphorePost(&_data.w_sem);
    osSemaphorePost(&_data.rc_sem);
}

WriteLock::WriteLock(DatabaseLockData& data) : _data(data)
{
    osSemaphoreWait(&_data.wc_sem);
    _data.writers_count++;
    if (_data.writers_count == 1)
        osSemaphoreWait(&_data.r_sem);
    osSemaphorePost(&_data.wc_sem);

    osSemaphoreWait(&_data.w_sem);
}

WriteLock::~WriteLock()
{
    osSemaphorePost(&_data.w_sem);

    osSemaphoreWait(&_data.wc_sem);
    _data.writers_count--;
    if (_data.writers_count == 0)
        osSemaphorePost(&_data.r_sem);
    osSemaphorePost(&_data.wc_sem);
}
