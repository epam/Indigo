#include "bingo_storage_manager.h"

using namespace bingo;

const char * FileStorageManager::_prop_filename = "fs_manager_properties";

FileStorageManager::FileStorageManager (const char *location) : _loc(location)
{
   _prop_table.load((_loc + _prop_filename).c_str());
}

FileStorage * FileStorageManager::create (const char *name, int block_size)
{
   FileStorage * new_storage = new FileStorage(name, block_size, true);

   _prop_table.add(name, block_size);

   return new_storage;
}

FileStorage * FileStorageManager::load (const char *name)
{
   size_t block_size = _prop_table.getUDec(name);
   return new FileStorage(name, block_size, false);
}
