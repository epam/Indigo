#ifndef __bingo_mmf__
#define __bingo_mmf__

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <new>

#ifdef _WIN32
   #include <windows.h>
   #undef min
   #undef max
#elif (defined __GNUC__ || defined __APPLE__)
   #include <sys/mman.h>
#endif

namespace bingo
{
   class MMFStorage
   {
   public:
      MMFStorage ();
      
      void open (const char *filename, size_t buf_size);

      void * ptr ();

      void close ();

   private:   
      void *_h_map_file;
      void *_ptr;
      size_t _len;
   };
};

#endif // __bingo_mmf__