#include "bingo_cf_storage.h"

using namespace bingo;

ByteBufferStorage::ByteBufferStorage(int block_size) : _block_size(block_size)
{
    _free_pos = 0;
}

MMFAddress ByteBufferStorage::create(MMFPtr<ByteBufferStorage>& cf_ptr, int block_size)
{
    cf_ptr.allocate();
    new (cf_ptr.ptr()) ByteBufferStorage(block_size);
    return cf_ptr.getAddress();
}

void ByteBufferStorage::load(MMFPtr<ByteBufferStorage>& cf_ptr, MMFAddress offset)
{
    cf_ptr = MMFPtr<ByteBufferStorage>(offset);
}

const byte* ByteBufferStorage::get(int idx, int& len)
{
    if (_addresses.size() <= idx)
        throw indigo::Exception("ByteBufferStorage: incorrect buffer id");

    if (_addresses[idx].len < 0)
    {
        len = -1;
        return 0;
    }

    len = _addresses[idx].len;
    return _blocks[_addresses[idx].block_idx].ptr() + _addresses[idx].offset;
}

void ByteBufferStorage::add(const byte* data, int len, int idx)
{
    if ((_blocks.size() == 0) || (_block_size - _free_pos < len))
    {
        MMFPtr<byte>& new_block = _blocks.push();
        new_block.allocate(_block_size);
        _free_pos = 0;
    }

    if (_addresses.size() <= idx)
        _addresses.resize(idx + 1);

    _addresses[idx].block_idx = _blocks.size() - 1;
    _addresses[idx].len = len;
    _addresses[idx].offset = _free_pos;

    memcpy(_blocks.top().ptr() + _free_pos, data, len);

    _free_pos += len;
}

void ByteBufferStorage::remove(int idx)
{
    if (_addresses.size() <= idx)
        throw indigo::Exception("ByteBufferStorage: incorrect buffer id");

    _addresses[idx].len = -1;
}

ByteBufferStorage::~ByteBufferStorage()
{
}
