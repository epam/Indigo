#include "bingo_fp_storage.h"

#include "base_c/bitarray.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"

using namespace bingo;

TranspFpStorage::TranspFpStorage(int fp_size, int block_size, int small_base_size) : _fp_size(fp_size), _block_size(block_size)
{
    _pack_count = 0;
    _storage.resize(fp_size * 8);
    _block_count = 0;
    _inc_fp_count = 0;
    _inc_size = block_size * 8;
    _small_inc_size = small_base_size;
    _inc_buffer.allocate(_small_inc_size * _fp_size);
    _small_flag = true;
    // Resize bit usage counts information for the all bits
    _fp_bit_usage_counts.resize(_fp_size * 8);
}

MMFAddress TranspFpStorage::create(MMFPtr<TranspFpStorage>& ptr, int fp_size, int block_size, int small_base_size)
{
    ptr.allocate();
    new (ptr.ptr()) TranspFpStorage(fp_size, block_size, small_base_size);
    return ptr.getAddress();
}

void TranspFpStorage::load(MMFPtr<TranspFpStorage>& ptr, MMFAddress offset)
{
    ptr = MMFPtr<TranspFpStorage>(offset);
}

void TranspFpStorage::add(const byte* fp)
{
    memcpy(_inc_buffer.ptr() + (_inc_fp_count * _fp_size), fp, _fp_size);

    _inc_fp_count++;

    if ((_inc_fp_count == _small_inc_size) && _small_flag)
    {
        byte* old_inc_ptr = _inc_buffer.ptr();
        _inc_buffer.allocate(_inc_size * _fp_size);
        memcpy(_inc_buffer.ptr(), old_inc_ptr, _small_inc_size * _fp_size);
        _small_flag = false;
    }

    if (_inc_fp_count == _inc_size)
    {
        _addIncToStorage();
        _inc_fp_count = 0;
    }
}

int TranspFpStorage::getBlockSize() const
{
    return _block_size;
}

const byte* TranspFpStorage::getBlock(int idx)
{
    return _storage[idx].ptr();
}

int TranspFpStorage::getBlockCount() const
{
    return _block_count;
}

const byte* TranspFpStorage::getIncrement() const
{
    return _inc_buffer.ptr();
}

int TranspFpStorage::getIncrementSize() const
{
    return _inc_fp_count;
}

int TranspFpStorage::getIncrementCapacity() const
{
    return _inc_size;
}

TranspFpStorage::~TranspFpStorage()
{
}

void TranspFpStorage::_addIncToStorage()
{
    profTimerStart(t0, "fp_inc_to_transp");

    std::vector<byte> block_buf;
    block_buf.resize(_block_size);

    // byte inc_mask = 0x80;
    for (int bit_idx = 0; bit_idx < 8 * _fp_size; bit_idx++)
    {
        memset(&block_buf[0], 0, _block_size);
        for (int fp_idx = 0; fp_idx < _inc_fp_count; fp_idx++)
            bitSetBit(&block_buf[0], fp_idx, bitGetBit(_inc_buffer.ptr() + fp_idx * _fp_size, bit_idx));

        if (_pack_count == 0)
        {
            // Update bit usage count
            _fp_bit_usage_counts[bit_idx] = bitGetOnesCount(&block_buf[0], _block_size);
        }

        int block_idx = (_pack_count * _fp_size * 8) + bit_idx;
        _storage.resize(block_idx + 1);
        _storage[block_idx].allocate(_block_size);
        memcpy(_storage[block_idx].ptr(), &block_buf[0], _block_size);
        _block_count++;
    }

    _pack_count++;
}

/*
void TranspFpStorage::create (int fp_size, const char *info_filename)
{
   _createFpStorage(fp_size, storage->getBlockSize() * 8, info_filename);

   fp_bit_usage_counts.resize(fp_size * 8);
   fp_bit_usage_counts.zerofill();
}


void TranspFpStorage::load (int fp_size, const char *info_filename)
{
   _loadFpStorage(fp_size, storage->getBlockSize() * 8, info_filename);
   _pack_count = _block_count / (fp_size * 8);

   fp_bit_usage_counts.resize(fp_size * 8);
   if (_pack_count > 0)
   {
      // Calculate fingerprints frequencies from the first block
      QS_DEF(Array<byte>, _items_mask);
      _items_mask.resize(storage->getBlockSize());
      for (int i = 0; i < fp_size * 8; i++)
      {
         getBlock(i, _items_mask.ptr());
         fp_bit_usage_counts[i] = bitGetOnesCount(_items_mask.ptr(), _items_mask.size());
      }
   }
   else
      fp_bit_usage_counts.zerofill();
}*/

MMFArray<int>& TranspFpStorage::getFpBitUsageCounts()
{
    return _fp_bit_usage_counts;
}

int TranspFpStorage::getPackCount() const
{
    return _pack_count;
}
