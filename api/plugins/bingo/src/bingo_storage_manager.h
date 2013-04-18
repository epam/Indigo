#ifndef __bingo_storage_manager__
#define __bingo_storage_manager__

#include "bingo_storage.h"
#include "bingo_properties.h"

namespace bingo
{
   // TODO: --DONE
   // * StorageManager.load (name) without block_size
   // * FileStorageManager::ctor (location)
   // * + property table with block_size
   // * remove StorageManager.get (replace with load)

   class StorageManager
   {
   public:
      virtual Storage * create ( const char *name, int block_size ) = 0;
      virtual Storage * load ( const char *name ) = 0;

      virtual ~StorageManager () {};
   };

   class FileStorageManager : public StorageManager
   {
   private:
      static const char *_prop_filename;
      std::string _loc;
      Properties _prop_table;

   public:
      FileStorageManager (const char *location);

      FileStorage * create (const char *name, int block_size);
      FileStorage * load   (const char *name);
   };

   class RamStorageManager : public StorageManager
   {
   private:
      static const char *_prop_filename;
      std::string _loc;
      Properties _prop_table;

   public:
      RamStorageManager (const char *location);

      RamStorage * create (const char *name, int block_size);
      RamStorage * load   (const char *name);
   };
};

#endif /* __bingo_storage_manager__ */