#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "bingo_ptr.h"
#include "bingo_mmf.h"

using namespace indigo;
using namespace bingo;

PtrArray<BingoAllocator> BingoAllocator::_instances;

int BingoAllocator::getAllocatorDataSize ()
{
   return sizeof(_BingoAllocatorData);
}

void BingoAllocator::_create (const char *filename, size_t size, size_t alloc_off)
{
   MMFile file;

   std::string name;
   _genFilename(0, filename, name);

   file.open(name.c_str(), size, true);

   byte *mmf_ptr = (byte *)file.ptr();

   if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   int cur_s_id = (int)TL_GET_SESSION_ID();
   _instances.expand(cur_s_id + 1);
   _instances[cur_s_id] = new BingoAllocator();

   BingoAllocator *inst = _instances[cur_s_id];

   inst->_data = (_BingoAllocatorData *)(mmf_ptr + alloc_off);
   new(inst->_data) _BingoAllocatorData();
   inst->_data->_free_off = alloc_off + sizeof(_BingoAllocatorData);
   inst->_data->_file_size = size;
   inst->_mm_files.clear();
   inst->_mm_files.push(file);
   inst->_filename.assign(filename);
}

void BingoAllocator::_load (const char *filename, size_t alloc_off)
{
   std::string name;
   _genFilename(0, filename, name);

   std::ifstream fstream(name.c_str(), std::ios::binary | std::ios::ate);

   size_t size = fstream.tellg();

   MMFile file;

   file.open(name.c_str(), size, false);

   byte *mmf_ptr = (byte *)file.ptr();

   if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   int cur_s_id = (int)TL_GET_SESSION_ID();
   _instances.expand(cur_s_id + 1);
   _instances[cur_s_id] = new BingoAllocator();

   BingoAllocator *inst = _instances[cur_s_id];

   inst->_data = (_BingoAllocatorData *)(mmf_ptr + alloc_off);
   inst->_mm_files.clear();
   inst->_mm_files.push(file);
   inst->_filename.assign(filename);

   size_t file_count = inst->_data->_free_off / inst->_data->_file_size + 1;

   for (int i = 1; i < file_count; i++)
   {
      _genFilename(i, inst->_filename.c_str(), name);

      MMFile &file = inst->_mm_files.push();

      file.open(name.c_str(), inst->_data->_file_size, false);
   }
}

BingoAllocator *BingoAllocator::_getInstance ()
{
   int cur_s_id = (int)TL_GET_SESSION_ID();
   
   if (_instances.size() <= cur_s_id)
      throw Exception("BingoAllocator: Incorrect session id");

   if (_instances[cur_s_id] == 0)
      throw Exception("BingoAllocator: instance is not initialized");

   return _instances[cur_s_id];
}
     

byte * BingoAllocator::_get ( size_t offset )
{
   if (offset > _data->_free_off)
      throw Exception("BingoAllocator: incorrect offset");

   size_t file_idx = offset / _data->_file_size;
   size_t file_off = offset % _data->_file_size;
   
   byte * file_ptr = (byte *)_mm_files.at((int)file_idx).ptr();

   return file_ptr + file_off;
}


BingoAllocator::BingoAllocator ()
{
   _mm_files.clear();
}

void BingoAllocator::_addFile ( int alloc_size )
{
   if (alloc_size > _data->_file_size)
      throw Exception("BingoAllocator: Too big allocation size");
   
   MMFile &file = _mm_files.push();

   std::string name;
   _genFilename(_mm_files.size() - 1, _filename.c_str(), name);
   file.open(name.c_str(), _data->_file_size, true);
   size_t old_file_count = _data->_free_off / _data->_file_size + 1;

   _data->_free_off = old_file_count * _data->_file_size;
}


void BingoAllocator::_genFilename (int idx, const char *filename, std::string &out_name)
{
   std::ostringstream name_str;

   name_str << filename;
   name_str << idx;

   out_name.assign(name_str.str());
}
