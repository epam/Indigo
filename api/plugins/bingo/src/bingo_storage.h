#ifndef __bingo_storage__
#define __bingo_storage__

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/auto_ptr.h"

#include "fstream"

using namespace indigo;

namespace bingo
{
   class Storage
   {
   public:
      Storage (int block_size);

      virtual void readBlock (int block_id, byte *data) = 0;
      virtual void writeBlock (int block_id, const byte *data) = 0;

      int getBlockSize( void );

      virtual ~Storage() {}

   protected:
      int _block_size;
   };

   class FileStorage : public Storage
   {
   public:
      FileStorage (const char *filename, int block_size, bool create);

      virtual void readBlock (int block_id, byte *data);

      virtual void writeBlock (int block_id, const byte *data);

      virtual ~FileStorage ();
   private:
      std::fstream _file;
   };

   class RamStorage : public Storage
   {
   public:
      RamStorage (const char *filename, int block_size, bool create);

      virtual void readBlock (int block_id, byte *data);

      virtual void writeBlock (int block_id, const byte *data);

      virtual ~RamStorage ();

   private:
      std::ofstream _file;
      ObjArray<AutoPtr<byte> > _blocks;
   };
};

#endif /* __bingo_storage__ */