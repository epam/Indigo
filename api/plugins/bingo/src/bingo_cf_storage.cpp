#include "bingo_cf_storage.h"

using namespace bingo;

ByteBufferStorage::ByteBufferStorage (int block_size) : FlatStorage(block_size)
{
   _free_pos = 0;
}

void ByteBufferStorage::create (const char *buf_filename, const char *offset_filename)
{
   _buf_filename = _buf_filename;
   _offset_filename = offset_filename;

   _buf_file.open(buf_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
   _offset_file.open(offset_filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
}

void ByteBufferStorage::load (const char *buf_filename, const char *offset_filename)
{
   _buf_filename = _buf_filename;
   _offset_filename = offset_filename;

   _buf_file.open(buf_filename, std::ios::in | std::ios::out | std::ios::binary);
   _offset_file.open(offset_filename, std::ios::in | std::ios::out | std::ios::binary);

   if (!_buf_file.is_open())
      throw Exception("ByteBufferStorage: buffer file missed");

   if (!_offset_file.is_open())
      throw Exception("ByteBufferStorage: offset file missed");

   _Addr addr;
   while (_offset_file.read((char *)(&addr), sizeof(addr)))
   {
      _addresses.push(addr);
   }

   _blocks.resize(_addresses.top().block_idx + 1);


   for (int i = 0; i < _blocks.size(); i++)
   {
      size_t block_len = _block_size;
      if (i == (_blocks.size() - 1))
      {
         _buf_file.seekg(0, std::ios_base::end);
         block_len = (size_t)_buf_file.tellg() - i * _block_size;
      }

      _blocks[i] = new byte[_block_size];
      _buf_file.seekg(i * _block_size);
      _buf_file.read((char *)_blocks[i], block_len);
   }

   _free_pos = _addresses.top().offset + _addresses.top().len;

   _buf_file.close();
   _offset_file.close();
   _buf_file.open(buf_filename, std::ios::in | std::ios::out | std::ios::binary);
   _offset_file.open(offset_filename, std::ios::in | std::ios::out | std::ios::binary);
}

const byte * ByteBufferStorage::get (int idx, int &len)
{
   if (_addresses.size() <= idx)
      throw Exception("ByteBufferStorage: incorrect buffer id");

   if (_addresses[idx].len < 0)
   {
      len = -1;
      return 0;
   }

   len = _addresses[idx].len;
   return _blocks[_addresses[idx].block_idx] + _addresses[idx].offset;
}

void ByteBufferStorage::add (const byte *data, int len, int idx) 
{
   if ((_blocks.size() == 0) || (_block_size - _free_pos < len))
   {
      _blocks.push(new byte[_block_size]);
      _free_pos = 0;
   }

   if (_addresses.size() <= idx)
      _addresses.resize(idx + 1);

   _addresses[idx].block_idx = _blocks.size() - 1;
   _addresses[idx].len = len;
   _addresses[idx].offset = _free_pos;

   memcpy(_blocks.top() + _free_pos, data, len);

   _buf_file.seekp(_addresses[idx].block_idx * _block_size + _addresses[idx].offset);
   _buf_file.write((const char *)data, _addresses[idx].len);

   _offset_file.seekp(idx * sizeof(_addresses[idx]));
   _offset_file.write((char *)&_addresses[idx], sizeof(_addresses[idx]));

   _free_pos += len;
}

void ByteBufferStorage::remove (int idx)
{
   if (_addresses.size() <= idx)
      throw Exception("ByteBufferStorage: incorrect buffer id");

   _addresses[idx].len = -1;

   _offset_file.seekp(idx * sizeof(_addresses[idx]));
   _offset_file.write((char *)&_addresses[idx], sizeof(_addresses[idx]));
}

ByteBufferStorage::~ByteBufferStorage()
{
   for (int i = 0; i < _blocks.size(); i++)
      delete[] _blocks[i];
}
