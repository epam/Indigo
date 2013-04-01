#include "bingo_storage_manager.h"

using namespace bingo;

FileStorage * FileStorageManager::create( const char *name, int block_size )
{
   return new FileStorage(name, block_size, true);
}

FileStorage * FileStorageManager::load( const char *name, int block_size )
{
   return new FileStorage(name, block_size, false);
}

FileStorage * FileStorageManager::get( const char *name )
{
   return 0;
}
