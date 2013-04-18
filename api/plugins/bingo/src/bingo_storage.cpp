#include "bingo_storage.h"

#include "base_cpp\output.h"
#include "base_cpp\scanner.h"

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
   {
      _file_output = new FileOutput(filename);
      _file_scanner = new FileScanner(filename);
   }
   else
   {
      _file_output = new FileOutput(true, filename);
      _file_scanner = new FileScanner(filename);
   }
}

void FileStorage::readBlock (int block_id, byte *data)
{
   _file_scanner->seek(block_id * _block_size, SEEK_SET);
   _file_scanner->read(_block_size, data);
}

void FileStorage::writeBlock (int block_id, const byte *data)
{
   _file_output->seek(block_id * _block_size, SEEK_SET);
   _file_output->write(data, _block_size);
   _file_output->flush();
}

FileStorage::~FileStorage ()
{
   delete _file_output;
   delete _file_scanner;
}

RamStorage::RamStorage (const char *filename, int block_size, bool create) : Storage(block_size)
{
   if (create)
      _file.open(filename, std::ios::out | std::ios::binary);
   else
   {
      std::ifstream ifile;
      ifile.open(filename, std::ios::in | std::ios::binary);

      while (!ifile.eof())
      {
         AutoPtr<byte> block = new byte[_block_size];

         if (!ifile.read((char *)block.get(), _block_size))
            break;
         AutoPtr<byte> &new_block = _blocks.push();
         new_block.reset(block.release());
      }

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