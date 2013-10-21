#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "bingo_ptr.h"
#include "bingo_mmf_storage.h"
#include "base_c/os_sync.h"

using namespace indigo;
using namespace bingo;

PtrArray<BingoAllocator> BingoAllocator::_instances;
OsLock BingoAllocator::_instances_lock;

int BingoAllocator::getAllocatorDataSize ()
{
   return sizeof(_BingoAllocatorData);
}

void BingoAllocator::_create (const char *filename, size_t size, size_t alloc_off, ObjArray<MMFile> *mm_files, int index_id)
{
   MMFile file;

   std::string name;
   _genFilename(0, filename, name);

   file.open(name.c_str(), size, true, false);

   byte *mmf_ptr = (byte *)file.ptr();

   if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   _instances_lock.Lock();
   _instances.expand(index_id + 1);
   _instances[index_id] = new BingoAllocator();
   _instances_lock.Unlock();
   
   BingoAllocator *inst = _instances[index_id];

   inst->_data_offset = alloc_off;
   _BingoAllocatorData *allocator_data = (_BingoAllocatorData *)(mmf_ptr + alloc_off);
   new(allocator_data) _BingoAllocatorData();
   inst->_mm_files = mm_files;
   allocator_data->_free_off = alloc_off + sizeof(_BingoAllocatorData);
   allocator_data->_file_size = size;
   inst->_mm_files->push(file);
   inst->_filename.assign(filename);
   inst->_index_id = index_id;
}

void BingoAllocator::_load (const char *filename, size_t alloc_off, ObjArray<MMFile> *mm_files, int index_id, bool read_only)
{
   std::string name;
   _genFilename(0, filename, name);

   std::ifstream fstream(name.c_str(), std::ios::binary | std::ios::ate);

   size_t size = fstream.tellg();

   MMFile file;

   file.open(name.c_str(), size, false, read_only);

   byte *mmf_ptr = (byte *)file.ptr();

   if ((mmf_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   _instances_lock.Lock();
   _instances.expand(index_id + 1);
   _instances[index_id] = new BingoAllocator();
   _instances_lock.Unlock();
   
   BingoAllocator *inst = _instances[index_id];

   _BingoAllocatorData *allocator_data = (_BingoAllocatorData *)(mmf_ptr + alloc_off);
   
   inst->_data_offset = alloc_off;
   inst->_mm_files = mm_files;
   inst->_mm_files->push(file);
   inst->_filename.assign(filename);
   inst->_index_id = index_id;

   size_t file_count = allocator_data->_free_off / allocator_data->_file_size + 1;

   for (int i = 1; i < file_count; i++)
   {
      _genFilename(i, inst->_filename.c_str(), name);

      MMFile &file = inst->_mm_files->push();

      file.open(name.c_str(), allocator_data->_file_size, false, read_only);
   }
}

BingoAllocator *BingoAllocator::_getInstance ()
{
   if (_instances.size() <= MMFStorage::database_id)
      throw Exception("BingoAllocator: Incorrect session id");

   if (_instances[MMFStorage::database_id] == 0)
      throw Exception("BingoAllocator: instance is not initialized");

   return _instances[MMFStorage::database_id];
}
     

byte * BingoAllocator::_get ( size_t offset )
{
   byte * mmf_ptr = (byte *)_mm_files->at(0).ptr();

   _BingoAllocatorData *allocator_data = (_BingoAllocatorData *)(mmf_ptr + _data_offset);
   
   if (offset > allocator_data->_free_off)
      throw Exception("BingoAllocator: incorrect offset");

   
   size_t file_idx = offset / allocator_data->_file_size;
   size_t file_off = offset % allocator_data->_file_size;
   
   byte * file_ptr = (byte *)_mm_files->at((int)file_idx).ptr();

   return file_ptr + file_off;
}


BingoAllocator::BingoAllocator ()
{
}

void BingoAllocator::_addFile ( int alloc_size )
{
   byte * mmf_ptr = (byte *)_mm_files->at(0).ptr();

   _BingoAllocatorData *allocator_data = (_BingoAllocatorData *)(mmf_ptr + _data_offset);
   
   if (alloc_size > allocator_data->_file_size)
      throw Exception("BingoAllocator: Too big allocation size");
   
   MMFile &file = _mm_files->push();

   std::string name;
   _genFilename(_mm_files->size() - 1, _filename.c_str(), name);
   file.open(name.c_str(), allocator_data->_file_size, true, false);
   size_t old_file_count = allocator_data->_free_off / allocator_data->_file_size + 1;

   allocator_data->_free_off = old_file_count * allocator_data->_file_size;
}


void BingoAllocator::_genFilename (int idx, const char *filename, std::string &out_name)
{
   std::ostringstream name_str;

   name_str << filename;
   name_str << idx;

   out_name.assign(name_str.str());
}
