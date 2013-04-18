#include "bingo_storage_manager.h"

using namespace bingo;

const char * FileStorageManager::_prop_filename = "fs_manager_properties";

FileStorageManager::FileStorageManager (const char *location) : _loc(location)
{
   _prop_table.load((_loc + _prop_filename).c_str());
}

FileStorage * FileStorageManager::create (const char *name, int block_size)
{
   AutoPtr<FileStorage> new_storage = new FileStorage((_loc + name).c_str(), block_size, true);

   _prop_table.add(name, block_size);

   return new_storage.release();
}

FileStorage * FileStorageManager::load (const char *name)
{
   unsigned long block_size = _prop_table.getULong(name);
   return new FileStorage((_loc + name).c_str(), block_size, false);
}

const char * RamStorageManager::_prop_filename = "ram_manager_properties";

RamStorageManager::RamStorageManager (const char *location) : _loc(location)
{
   _prop_table.load((_loc + _prop_filename).c_str());
}

RamStorage * RamStorageManager::create (const char *name, int block_size)
{
   AutoPtr<RamStorage> new_storage = new RamStorage((_loc + name).c_str(), block_size, true);

   _prop_table.add(name, block_size);

   return new_storage.release();
}

RamStorage * RamStorageManager::load   (const char *name)
{
   unsigned long block_size = _prop_table.getULong(name);
   return new RamStorage((_loc + name).c_str(), block_size, false);
}
