#ifndef __bingo_fp_storage__
#define __bingo_fp_storage__

#include "bingo_storage.h"
#include <iostream>
#include <fstream>
#include <vector>

namespace  bingo
{
   class FpStorage
   {
   public:
      virtual void create( int fp_size, Storage *storage, const char *info_filename ) = 0;

      virtual void load( int fp_size, Storage *storage, const char *info_filename ) = 0;

      virtual void add( const byte *fp ) = 0;

      virtual int getBlockSize( void ) const = 0;

      virtual void getBlock( int idx, byte *data ) const = 0;

      virtual int getBlockCount() const = 0;

      virtual const byte *getIncrement() const = 0;

      virtual int getIncrementSize( void ) const = 0;
   };

   class BaseFpStorage : public FpStorage
   {
   public:
      BaseFpStorage();

      virtual void add( const byte *fp );

      virtual int getBlockSize( void ) const;

      virtual void getBlock( int idx, byte *data ) const;

      virtual int getBlockCount() const;

      virtual const byte *getIncrement() const;

      virtual int getIncrementSize( void ) const;

      virtual int getIncrementCapacity( void ) const;

      virtual ~BaseFpStorage();

   private:
      void _loadInfo( const char *info_filename );

      void _writeInfoCounts ();

   protected:
      int _fp_size;
      int _block_count;
      Storage *_storage;

      std::ofstream _inc_file;
      std::vector<byte> _inc_buffer;
      int _inc_count;
      int _inc_max_count;
   
      virtual void _addIncToStorage() = 0;

      void _createFpStorage( int fp_size, Storage *storage, int inc_fp_capacity, const char *inc_filename );

      void _loadFpStorage( int fp_size, Storage *storage, int inc_fp_capacity, const char *inc_filename );
   };

   class TranspFpStorage : public BaseFpStorage
   {
   public:
      TranspFpStorage();

      virtual void create( int fp_size, Storage *storage, const char *inc_filename );

      virtual void load( int fp_size, Storage *storage, const char *inc_filename );

      int getPackCount( void ) const;

      const Array<int> &getFpBitUsageCounts () const;

   private:
      int _pack_count;
      Array<int> fp_bit_usage_counts;

      void _addIncToStorage();
   };

   class RowFpStorage : public BaseFpStorage
   {
   public:
      RowFpStorage ();

      virtual void create (int fp_size, Storage *storage, const char *inc_filename);

      virtual void load (int fp_size, Storage *storage, const char *inc_filename);

      int getFpPerBlockCount () const;

   private:
      void _addIncToStorage();
   };
};
#endif /* __fp_storage__ */
