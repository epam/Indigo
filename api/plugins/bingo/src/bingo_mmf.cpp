#include "bingo_mmf.h"

using namespace bingo;

MMFStorage::MMFStorage()
{
}

void MMFStorage::open( const char *filename, size_t buf_size )
{
   char * pBuf;

   DWORD dwflags;
   dwflags = GENERIC_READ | GENERIC_WRITE;

   HANDLE hFile = CreateFile((LPCSTR)filename, dwflags, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

   _h_map_file = CreateFileMapping(
                 hFile,    // use paging file
                 NULL,                    // default security
                 PAGE_READWRITE,          // read/write access
                 buf_size >> 32,          // maximum object size (high-order DWORD)
                 buf_size,                // maximum object size (low-order DWORD)
                 0);                 // name of mapping object

   if (_h_map_file == NULL)
   {
      _tprintf(TEXT("Could not create file mapping object (%d).\n"),
             GetLastError());
   }

   _ptr = (char *)MapViewOfFile(_h_map_file,   // handle to map object
                        FILE_MAP_ALL_ACCESS, // read/write permission
                        0,
                        0,
                        buf_size);

   if (_ptr == NULL)
   {
      _tprintf(TEXT("Could not map view of file (%d).\n"),
             GetLastError());

       CloseHandle(_h_map_file);
   }
}

void * MMFStorage::ptr()
{
   return _ptr;
}

void MMFStorage::close(  )
{
   UnmapViewOfFile(_ptr);

   CloseHandle(_h_map_file);
}