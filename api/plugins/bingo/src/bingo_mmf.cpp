#include "bingo_mmf.h"

#include "base_cpp/exception.h"

#ifdef _WIN32
   #include <windows.h>
   #undef min
   #undef max
#elif (defined __GNUC__ || defined __APPLE__)
   #include <sys/mman.h>
   #include <stdlib.h>
   #include <fcntl.h>
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/mman.h>
   #include <sys/stat.h>
#endif

using namespace bingo;
using namespace indigo;

MMFStorage::MMFStorage()
{
}

void * MMFStorage::ptr ()
{
   return _ptr;
}

void MMFStorage::open (const char *filename, size_t buf_size)
{
   _len = buf_size;

#ifdef _WIN32
   char * pBuf;

   DWORD dwflags;
   dwflags = GENERIC_READ | GENERIC_WRITE;

   HANDLE h_file = CreateFile((LPCSTR)filename, dwflags, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

   if (h_file == INVALID_HANDLE_VALUE)
      throw Exception("BingoMMF: Could not open file");

   _h_map_file = CreateFileMapping(
                 h_file,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 buf_size >> 32,          // maximum object size (high-order DWORD)
                 buf_size,                // maximum object size (low-order DWORD)
                 0);                 // name of mapping object

   if (_h_map_file == NULL)
      throw Exception("BingoMMF: Could not create file mapping object");
   
   _ptr = (char *)MapViewOfFile(_h_map_file,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        buf_size);

   if (_ptr == NULL)
      throw Exception("BingoMMF: Could not map view of file");

#elif (defined __GNUC__ || defined __APPLE__)
   int fd;
   if ((fd = open(filename, O_RDONLY)) == -1) 
      throw Exception("BingoMMF: Could not open file");

   _ptr = mmap((caddr_t)0, _len, PROT_EXEC, MAP_SHARED, fd, 0);
   
   if (_ptr == -1)
      throw Exception("BingoMMF: Could not map view of file");
#endif
}

void MMFStorage::close ()
{
#ifdef _WIN32
   UnmapViewOfFile(_ptr);

   CloseHandle(_h_map_file);
#elif (defined __GNUC__ || defined __APPLE__)
   munmap((caddr_t)_ptr, _len);
#endif
}