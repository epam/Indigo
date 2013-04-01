#ifndef __bingo_storage__
#define __bingo_storage__

#include "base_cpp\output.h"
#include "base_cpp\scanner.h"

using namespace indigo;

namespace bingo
{
   class Storage
   {
   public:
      Storage( int block_size );

      virtual void readBlock( int block_id, byte *data ) = 0;
      virtual void writeBlock( int block_id, const byte *data ) = 0;

      int getBlockSize( void );

   protected:
      int _block_size;
   };

   class FileStorage : public Storage
   {
   public:
      FileStorage( const char *filename, int block_size, bool create );

      virtual void readBlock( int block_id, byte *data );

      virtual void writeBlock( int block_id, const byte *data );

      virtual ~FileStorage( void );
   private:
      FileOutput *_file_output;
      FileScanner *_file_scanner;
   };
};

#endif /* __bingo_storage__ */