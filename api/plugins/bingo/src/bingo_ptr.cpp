#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "bingo_ptr.h"
#include "bingo_mmf.h"

using namespace indigo;
using namespace bingo;

BingoAllocator * BingoAllocator::_instance = 0;

void BingoAllocator::_create (const char *filename, size_t size, size_t alloc_off, ObjArray<MMFile> *mm_files)
{
   mm_files->clear();
   MMFile &file = mm_files->push();

   std::string name;
   _genFilename(0, filename, name);

   file.open(name.c_str(), size, true);

   byte *mmf_ptr = (byte *)file.ptr();

   if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");
   
   _instance = new(mmf_ptr + alloc_off) BingoAllocator();
   _instance->_free_off = alloc_off + sizeof(BingoAllocator);
   _instance->_file_size = size;
   _instance->_mm_files = mm_files;
   _instance->_filename.assign(filename);
}

void BingoAllocator::_load (const char *filename, size_t alloc_off, ObjArray<MMFile> *mm_files)
{
   mm_files->clear();

   std::string name;
   _genFilename(0, filename, name);

   std::ifstream fstream(name.c_str(), std::ios::binary | std::ios::ate);

   size_t size = fstream.tellg();

   MMFile &file = mm_files->push();

   file.open(name.c_str(), size, false);

   byte *mmf_ptr = (byte *)file.ptr();

   if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   _instance = (BingoAllocator *)(mmf_ptr + alloc_off);
   _instance->_mm_files = mm_files;
   _instance->_filename.assign(filename);

   size_t file_count = _instance->_free_off / _instance->_file_size + 1;

   for (int i = 1; i < file_count; i++)
   {
      _genFilename(i, _instance->_filename.c_str(), name);

      MMFile &file = _instance->_mm_files->push();

      file.open(name.c_str(), _instance->_file_size, false);
   }
}

void BingoAllocator::close ()
{
   for (int i = 0; i < _instance->_mm_files->size(); i++)
   {
      _instance->_mm_files->at(i).close();
   }
   _instance->_mm_files->clear();
}

BingoAllocator *BingoAllocator::_getInstance ()
{
   if (_instance == 0)
      throw Exception("BingoAllocator: instance is not initialized");

   return _instance;
}
     

byte * BingoAllocator::_get ( size_t offset )
{
   if (offset > _free_off)
      throw Exception("BingoAllocator: incorrect offset");

   size_t file_idx = offset / _file_size;
   size_t file_off = offset % _file_size;
   
   byte * file_ptr = (byte *)_mm_files->at(file_idx).ptr();

   return file_ptr + file_off;
}


BingoAllocator::BingoAllocator () : _mm_files(0), _free_off(0), _file_size(-1)
{
}

void BingoAllocator::_addFile ( int alloc_size )
{
   if (alloc_size > _file_size)
      throw Exception("BingoAllocator: Too big allocation size");
   
   MMFile &file = _mm_files->push();

   std::string name;
   _genFilename(_mm_files->size() - 1, _filename.c_str(), name);
   file.open(name.c_str(), _file_size, true);
   size_t old_file_count = _free_off / _file_size + 1;

   _free_off = old_file_count * _file_size;
}


void BingoAllocator::_genFilename (int idx, const char *filename, std::string &out_name)
{
   std::ostringstream name_str;

   name_str << filename;
   name_str << idx;

   out_name.assign(name_str.str());
}
