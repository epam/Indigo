#include "bingo_fp_storage.h"

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/profiling.h"
#include <iostream>
#include <fstream>

using namespace bingo;

BaseFpStorage::BaseFpStorage () : _fp_size(0), _storage(0)
{
   _block_count = 0;
   _inc_buffer = 0;
   _inc_count = 0;
}

void BaseFpStorage::_loadInfo (const char *info_filename)
{
   std::ifstream is(info_filename, std::ios::in | std::ios::binary);

   if (is)
   {
      is.read((char *)&_block_count, sizeof(_block_count));
      is.read((char *)&_inc_count, sizeof(_inc_count));
      is.read((char *)_inc_buffer, _inc_count * _fp_size);
   }

   is.close();
}

void BaseFpStorage::_createFpStorage (int fp_size, Storage *storage, int inc_fp_capacity, const char *info_filename)
{
   _fp_size = fp_size;
   _storage = storage;
   _inc_buffer = new byte[_fp_size * inc_fp_capacity];
   _block_count = 0;
   _inc_count = 0;
   _inc_max_count = inc_fp_capacity;
   _inc_file.open(info_filename, std::ios::out | std::ios::binary | std::ios::trunc);
   _writeInfoCounts();
}

void BaseFpStorage::_writeInfoCounts ()
{
   _inc_file.seekp(0);
   _inc_file.write((char *)&_block_count, sizeof(_block_count));
   _inc_file.write((char *)&_inc_count, sizeof(_inc_count));
   _inc_file.flush();
}

void BaseFpStorage::_loadFpStorage (int fp_size, Storage *storage, int inc_fp_capacity, const char *info_filename)
{
   _fp_size = fp_size;
   _storage = storage;
   _inc_buffer = new byte[_fp_size * inc_fp_capacity];
   _loadInfo(info_filename);
   _inc_max_count = inc_fp_capacity;
   
   _inc_file.open(info_filename, std::ios::in | std::ios::out | std::ios::binary);
   
    if (!_inc_file.is_open())
      throw Exception("Fingerprint increment file missed");
}

void BaseFpStorage::add (const byte *fp)
{
   memcpy(_inc_buffer + (_inc_count * _fp_size), fp, _fp_size);

   _inc_file.seekp(sizeof(_inc_count) + sizeof(_block_count) + _inc_count * _fp_size);

   _inc_file.write((char *)fp, _fp_size);
   
   _inc_count++;
   _writeInfoCounts();

   if (_inc_count == _inc_max_count)
   {
      _addIncToStorage();
      _inc_count = 0;
      _writeInfoCounts();
   }
}

int BaseFpStorage::getBlockSize () const
{
   return _storage->getBlockSize();
}

void BaseFpStorage::getBlock (int idx, byte *data) const
{
   _storage->readBlock(idx, data);
}

int BaseFpStorage::getBlockCount () const
{
   return _block_count;
}

const byte *BaseFpStorage::getIncrement () const
{
   return _inc_buffer;
}

int BaseFpStorage::getIncrementSize () const
{
   return _inc_count;
}

int BaseFpStorage::getIncrementCapacity () const
{
   return _inc_max_count;
}

BaseFpStorage::~BaseFpStorage ()
{
   _inc_file.close();
   delete _inc_buffer;
}

void TranspFpStorage::_addIncToStorage ()
{
   profTimerStart(t0, "fp_inc_to_transp");
   if (_pack_count == 0)
   {
      // Resize bit usage counts information for the all bits
      fp_bit_usage_counts.resize(_fp_size * 8);
   }

   byte *block_buf = new byte[_storage->getBlockSize()];
   byte block_b = 0;
               
   byte inc_mask = 0x80;
   for (int bit_idx = 0; bit_idx < 8 * _fp_size; bit_idx++)
   {
      memset(block_buf, 0, _storage->getBlockSize());
      for (int fp_idx = 0; fp_idx < _inc_count; fp_idx++)
         bitSetBit(block_buf, fp_idx, bitGetBit(_inc_buffer + fp_idx * _fp_size, bit_idx));

      if (_pack_count == 0)
      {
         // Update bit usage count
         fp_bit_usage_counts[bit_idx] = bitGetOnesCount(block_buf, _storage->getBlockSize());
      }

      _storage->writeBlock((_pack_count * _fp_size * 8) + bit_idx, block_buf);
      _block_count++;
   }

   _pack_count++;
   delete block_buf;
}

TranspFpStorage::TranspFpStorage()
{
   _pack_count = 0;
}

void TranspFpStorage::create (int fp_size, Storage *storage, const char *info_filename)
{
   _createFpStorage(fp_size, storage, storage->getBlockSize() * 8, info_filename);

   fp_bit_usage_counts.resize(fp_size * 8);
   fp_bit_usage_counts.zerofill();
}

void TranspFpStorage::load (int fp_size, Storage *storage, const char *info_filename)
{
   _loadFpStorage(fp_size, storage, storage->getBlockSize() * 8, info_filename);
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
}

const Array<int>& TranspFpStorage::getFpBitUsageCounts () const
{
   return fp_bit_usage_counts;
}

int TranspFpStorage::getPackCount () const
{
   return _pack_count;
}

void RowFpStorage::_addIncToStorage ()
{
   _storage->writeBlock(_block_count, _inc_buffer);
   _block_count++;
}

RowFpStorage::RowFpStorage ()
{
}

void RowFpStorage::create (int fp_size, Storage *storage, const char *info_filename)
{
   _createFpStorage(fp_size, storage, storage->getBlockSize() / fp_size, info_filename);
}

void RowFpStorage::load (int fp_size, Storage *storage, const char *info_filename)
{
   _loadFpStorage(fp_size, storage, storage->getBlockSize() / fp_size, info_filename);
}

int RowFpStorage::getFpPerBlockCount () const
{
   return _storage->getBlockSize() / _fp_size;
}
