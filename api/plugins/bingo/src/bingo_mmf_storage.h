#ifndef __bingo_mmf_storage__
#define __bingo_mmf_storage__

#include "base_cpp/obj_array.h"
#include "bingo_mmf.h"
#include "bingo_ptr.h"

#ifdef _WIN32
   #define BINGO_TL __declspec(thread)
#elif (defined __GNUC__ || defined __APPLE__)
   #define BINGO_TL __thread
#endif

using namespace indigo;

namespace bingo
{
   class MMFStorage
   {
   public:
      static BINGO_TL int database_id;

      static const int max_header_len = 128;

      MMFStorage();

      void create (const char *filename, size_t size, const char *header, int index_id);

      void load (const char *filename, BingoPtr<char> header_ptr, int index_id, bool read_only);

      void close ();
   private:
      ObjArray<MMFile> _mm_files;
      bool _read_only;
   };
};

#endif // __bingo_mmf_storage__
