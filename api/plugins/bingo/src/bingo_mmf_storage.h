#ifndef __bingo_mmf_storage__
#define __bingo_mmf_storage__

#include "base_cpp/obj_array.h"
#include "bingo_mmf.h"
#include "bingo_ptr.h"

using namespace indigo;

namespace bingo
{
   class MMFStorage
   {
   public:
      static const int max_header_len = 128;

      MMFStorage();

      void create (const char *filename, size_t size, const char *header);

      void load (const char *filename, BingoPtr<char> header_ptr);

      void close ();
   private:
      ObjArray<MMFile> _mm_files;
   };
};

#endif // __bingo_mmf_storage__
