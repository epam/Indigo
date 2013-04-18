#include "bingo_fp_storage.h"
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
   std::ifstream is(info_filename, std::ifstream::binary);

   if (is)
   {
      is.read((char *)&_block_count, sizeof(_block_count));
      is.read((char *)&_inc_count, sizeof(_inc_count));
      is.read((char *)_inc_buffer, _inc_count * _fp_size);
   }
}

void BaseFpStorage::_createFpStorage (int fp_size, Storage *storage, int inc_fp_capacity, const char *info_filename)
{
   _fp_size = fp_size;
   _storage = storage;
   _inc_buffer = new byte[_fp_size * inc_fp_capacity];
   _block_count = 0;
   _inc_count = 0;
   _inc_max_count = inc_fp_capacity;
   _inc_file.open(info_filename, std::ios::out | std::ios::binary);

   _inc_file.seekp(0);
   _inc_file.write((char *)&(_block_count), sizeof(_block_count));
   _inc_file.write((char *)&(_inc_count), sizeof(_inc_count));
   
   _inc_file.flush();
}

void BaseFpStorage::_loadFpStorage (int fp_size, Storage *storage, int inc_fp_capacity, const char *info_filename)
{
   _fp_size = fp_size;
   _storage = storage;
   _inc_buffer = new byte[_fp_size * inc_fp_capacity];
   _loadInfo(info_filename);
   _inc_max_count = inc_fp_capacity;
   _inc_file.open(info_filename, std::ios::out | std::ios::app | std::ios::binary);
}

void BaseFpStorage::add (const byte *fp)
{
   memcpy(_inc_buffer + (_inc_count * _fp_size), fp, _fp_size);
   
   _inc_file.seekp(sizeof(_inc_count) + sizeof(_block_count) + _inc_count * _fp_size);
   _inc_file.write((char *)fp, _fp_size);
   _inc_count++;
   _inc_file.seekp(0);
   _inc_file.write((char *)&(_block_count), sizeof(_block_count));
   _inc_file.write((char *)&(_inc_count), sizeof(_inc_count));
   
   _inc_file.flush();

   if (_inc_count == _inc_max_count)
   {
      _addIncToStorage();
      _inc_count = 0;
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
   delete _inc_buffer;
}

void TranspFpStorage::_addIncToStorage ()
{
   byte *block_buf = new byte[_storage->getBlockSize()];
   byte block_b = 0;
               
   byte inc_mask = 0x80;
   for (int bit_cnt = 0; bit_cnt < 8; bit_cnt++)
   {
      for (int byte_cnt = 0; byte_cnt < _fp_size; byte_cnt++)
      {
         for (int fp_cnt = 0; fp_cnt < _inc_count; fp_cnt++)
         {
            byte inc_b = _inc_buffer[fp_cnt * _fp_size + byte_cnt];
            int block_bit_pos = fp_cnt % 8;
            byte block_bit = (byte)((inc_mask & inc_b) != 0) << (7 - block_bit_pos);
            
            block_b |= block_bit;
            if (block_bit_pos == 7)
            {
               block_buf[fp_cnt / 8] = block_b;
               block_b = 0;
            }
         }

         _storage->writeBlock((_pack_count *  _fp_size * 8) + byte_cnt * 8 + bit_cnt, block_buf);
         _block_count++;
      }

      inc_mask >>= 1;
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
}

void TranspFpStorage::load (int fp_size, Storage *storage, const char *info_filename)
{
   _loadFpStorage(fp_size, storage, storage->getBlockSize() * 8, info_filename);
   _pack_count = _block_count / (fp_size * 8);
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
