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