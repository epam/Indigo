#include "bingo_lock.h"

using namespace bingo;

DatabaseLockData::DatabaseLockData() : rc_sem(1, 1), wc_sem(1, 1), w_sem(1, 1), r_sem(1, 1), writers_count(0), readers_count(0)
{
}

ReadLock::ReadLock(DatabaseLockData& data) : _data(data)
{
    _data.r_sem.Wait();
    _data.rc_sem.Wait();
    _data.readers_count++;
    if (_data.readers_count == 1)
        _data.w_sem.Wait();
    _data.rc_sem.Post();
    _data.r_sem.Post();
}

ReadLock::~ReadLock()
{
    _data.rc_sem.Wait();
    _data.readers_count--;
    if (_data.readers_count == 0)
        _data.w_sem.Post();
    _data.rc_sem.Post();
}

WriteLock::WriteLock(DatabaseLockData& data) : _data(data)
{
    _data.wc_sem.Wait();
    _data.writers_count++;
    if (_data.writers_count == 1)
        _data.r_sem.Wait();
    _data.wc_sem.Post();
    
    _data.w_sem.Wait();
}

WriteLock::~WriteLock()
{
    _data.w_sem.Post();

    _data.wc_sem.Wait();
    _data.writers_count--;
    if (_data.writers_count == 0)
        _data.r_sem.Post();
    
    _data.wc_sem.Post();
}
