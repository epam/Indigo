/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#if defined(_WIN32)

#include <windows.h>
#include <string.h>

#include "base_cpp/shmem.h"

using namespace indigo;

IMPL_ERROR(SharedMemory, "shared memory");

SharedMemory::SharedMemory (const char *name, int size, bool no_map_if_first)
{
   char *winapi_error;

   _map_object = NULL;
   _pointer = NULL;

   _map_object = CreateFileMapping( 
         INVALID_HANDLE_VALUE, // use paging file
         NULL,                 // default security attributes
         PAGE_READWRITE,       // read/write access
         0,                    // size: high 32-bits
         size,            // size: low 32-bits
         name);     // name of map object

   if (_map_object == NULL) 
   {
      FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
         FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR) &winapi_error,
         0, NULL );
      throw Error("can't create map object: %s", winapi_error); 
   }

   _was_first = (GetLastError() != ERROR_ALREADY_EXISTS);

   if (_was_first && no_map_if_first)
      return;

   _pointer = MapViewOfFile( 
         _map_object,     // object to map view of
         FILE_MAP_WRITE, // read/write access
         0,              // high offset:  map from
         0,              // low offset:   beginning
         0);             // default: map entire file

   if (_pointer == NULL) 
   {
      FormatMessage(
         FORMAT_MESSAGE_ALLOCATE_BUFFER | 
         FORMAT_MESSAGE_FROM_SYSTEM,
         NULL,
         GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR) &winapi_error,
         0, NULL );
      throw Error("can't get pointer: %s", winapi_error);
   }

   strncpy(_id, name, sizeof(_id));

   if (_was_first) 
      memset(_pointer, 0, size); 
}

SharedMemory::~SharedMemory ()
{
   if (_pointer != NULL)
      UnmapViewOfFile(_pointer); 
   if (_map_object != NULL)
      CloseHandle(_map_object);
}

#endif
