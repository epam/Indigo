#ifndef __bingo_mmf__
#define __bingo_mmf__

#include <stdio.h>
#include <new>
#include <string>

namespace bingo
{
   class MMFile
   {
   public:
      MMFile ();
      
      ~MMFile ();
      
      void open (const char *filename, size_t buf_size);

      void resize (size_t new_size);

      void * ptr ();

      const char * name ();

      size_t size();

      void close ();

   private:
#ifdef _WIN32
      void *_h_map_file;
      void *_h_file;
#elif (defined __GNUC__ || defined __APPLE__)
      int _fd;
#endif      
      void *_ptr;
      std::string _filename;
      size_t _len;
   };
};

#endif // __bingo_mmf__