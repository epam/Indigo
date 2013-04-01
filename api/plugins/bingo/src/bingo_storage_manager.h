#ifndef __bingo_storage_manager__
#define __bingo_storage_manager__

#include "bingo_storage.h"

namespace bingo
{
   class StorageManager
   {
   public:
      virtual Storage * create( const char *name, int block_size ) = 0;
      virtual Storage * get( const char *name ) = 0;
   };

   class FileStorageManager : public StorageManager
   {
   public:
      FileStorage * create( const char *name, int block_size );

      FileStorage * load( const char *name, int block_size );

      FileStorage * get( const char *name );
   };
};

#endif /* __bingo_storage_manager__ */