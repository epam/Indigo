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

MMFile::MMFile() : _ptr(0)
{
#ifdef _WIN32
      _h_file = 0;
      _h_map_file = 0;
#elif (defined __GNUC__ || defined __APPLE__)
      _fd = -1;
#endif     
}

MMFile::~MMFile ()
{
   //close();
}

void * MMFile::ptr ()
{
   return _ptr;
}

const char * MMFile::name ()
{
   return _filename.c_str();
}

size_t MMFile::size()
{
   return _len;
}

void MMFile::open (const char *filename, size_t buf_size)
{
   _len = buf_size;

   _filename.assign(filename);

#ifdef _WIN32
   char * pBuf;

   DWORD dwflags;
   dwflags = GENERIC_READ | GENERIC_WRITE;

   _h_file = CreateFile((LPCSTR)_filename.c_str(), dwflags, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
   DWORD dw = GetLastError(); 

   LPVOID lpMsgBuf;

   if (_h_file == INVALID_HANDLE_VALUE)
   {
      /*
      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

      char * mesg = (char *)lpMsgBuf;

      printf("MMFile::open : Filename - %s, error - %s\n", _filename.c_str(), mesg);*/
      throw Exception("BingoMMF: Could not open file");
   }

   _h_map_file = CreateFileMapping(
                 _h_file,    // use paging file
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
   if ((fd = ::open(_filename.c_str(), O_RDONLY)) == -1) 
      throw Exception("BingoMMF: Could not open file");

   _ptr = mmap((caddr_t)0, _len, PROT_EXEC, MAP_SHARED, fd, 0);
   
   if (_ptr == (void *)-1)
      throw Exception("BingoMMF: Could not map view of file");
#endif
}

void MMFile::resize (size_t new_size)
{
   if (_filename.size() == 0)
      throw Exception("BingoMMF: Resizeing of uninitialized file");
   
   close();
   open(_filename.c_str(), new_size);
}

void MMFile::close ()
{
#ifdef _WIN32
   if (_filename.size() != 0)
   {
      DeleteFile(_filename.c_str());
      _filename.clear();
   }

   if (_h_file != 0)
   {
      CloseHandle(_h_file);
      _h_file = 0;
   }

   if (_h_map_file != 0)
   {
      CloseHandle(_h_map_file);
      _h_map_file = 0;
   }

   if (_ptr != 0)
   {
      UnmapViewOfFile(_ptr);
      _ptr = 0;
   }
#elif (defined __GNUC__ || defined __APPLE__)
   if (_ptr != 0)
   {
      munmap((caddr_t)_ptr, _len);
      _ptr = 0;
   }

   if (_fd != -1)
   {
      close(_fd);
      _fd = -1;
   }
#endif
}