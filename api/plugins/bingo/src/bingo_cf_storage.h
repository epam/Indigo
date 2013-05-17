#ifndef __cmf_storage__
#define __cmf_storage__

#include "base_cpp/obj_array.h"
#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/tlscont.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace indigo;

namespace bingo
{
   class FlatStorage
   {
   public:
      FlatStorage (int block_size) : _block_size(block_size) {}

      virtual void create (const char *buf_filename, const char *offset_filename) = 0;
      virtual void load (const char *buf_filename, const char *offset_filename) = 0;
      virtual const byte * get (int idx, int &len) = 0;
      virtual void add (const byte *data, int len, int idx) = 0;
      virtual void remove (int idx) = 0;

      virtual ~FlatStorage() {}

   protected:
      int _block_size;
   };


   class ByteBufferStorage : public FlatStorage
   {
   public:
      ByteBufferStorage (int block_size);

      virtual void create (const char *buf_filename, const char *offset_filename);
      virtual void load (const char *buf_filename, const char *offset_filename);
      virtual const byte * get (int idx, int &len);
      virtual void add (const byte *data, int len, int idx);
      virtual void remove (int idx);
      virtual ~ByteBufferStorage();

   private:

      struct _Addr
      {
         unsigned long block_idx;
         unsigned long offset;
         long len;
      };

      int _free_pos;
      Array<byte *> _blocks;
      Array<_Addr> _addresses;

      std::fstream _buf_file;
      std::fstream _offset_file;
      std::string _buf_filename;
      std::string _offset_filename;
   };
};

#endif /* __cmf_storage__ */
