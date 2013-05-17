#include "bingo_storage.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

using namespace indigo;

using namespace bingo;

Storage::Storage (int block_size) : _block_size(block_size)
{
}

int Storage::getBlockSize( void )
{
   return _block_size;
}


FileStorage::FileStorage (const char *filename, int block_size, bool create) : Storage(block_size)
{
   if (create)
      _file.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
   else
      _file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
}

void FileStorage::readBlock (int block_id, byte *data)
{
   size_t offset = block_id;
   offset *=  _block_size;

   _file.seekg(offset);
   _file.read((char *)data, _block_size);
}

void FileStorage::writeBlock (int block_id, const byte *data)
{

   size_t offset = block_id;
   offset *=  _block_size;

   _file.seekp(offset);
   _file.write((const char *)data, _block_size);
   _file.flush();
}

FileStorage::~FileStorage ()
{
}

RamStorage::RamStorage (const char *filename, int block_size, bool create) : Storage(block_size)
{
   if (create)
      _file.open(filename, std::ios::out | std::ios::binary | std::ios::trunc);
   else
   {
      std::ifstream ifile;
      ifile.open(filename, std::ios::in | std::ios::binary);

      while (!ifile.eof())
      {
         AutoPtr<byte> block(new byte[_block_size]);

         if (!ifile.read((char *)block.get(), _block_size))
            break;
         AutoPtr<byte> &new_block = _blocks.push();
         new_block.reset(block.release());
      }

      ifile.close();
      _file.open(filename, std::ios::out | std::ios::app | std::ios::binary);
   }

}

void RamStorage::readBlock (int block_id, byte *data)
{
   memcpy(data, _blocks[block_id].get(), _block_size);
}

void RamStorage::writeBlock (int block_id, const byte *data)
{
   if (block_id >= _blocks.size())
      _blocks.expand(block_id + 1);

   if (_blocks[block_id].get() == 0)
      _blocks[block_id].reset(new byte[_block_size]);

   memcpy(_blocks[block_id].get(), data, _block_size);
   _file.seekp(block_id * _block_size);
   _file.write((char *)data, _block_size);
   _file.flush();
}

RamStorage::~RamStorage ()
{
}