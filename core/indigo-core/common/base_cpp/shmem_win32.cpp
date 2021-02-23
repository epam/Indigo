/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#if defined(_WIN32)

#include <string.h>
#include <windows.h>

#include "base_cpp/shmem.h"

using namespace indigo;

IMPL_ERROR(SharedMemory, "shared memory");

SharedMemory::SharedMemory(const char* name, int size, bool no_map_if_first)
{
    char* winapi_error;

    _map_object = NULL;
    _pointer = NULL;

    _map_object = CreateFileMapping(INVALID_HANDLE_VALUE, // use paging file
                                    NULL,                 // default security attributes
                                    PAGE_READWRITE,       // read/write access
                                    0,                    // size: high 32-bits
                                    size,                 // size: low 32-bits
                                    name);                // name of map object

    if (_map_object == NULL)
    {
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&winapi_error, 0, NULL);
        throw Error("can't create map object: %s", winapi_error);
    }

    _was_first = (GetLastError() != ERROR_ALREADY_EXISTS);

    if (_was_first && no_map_if_first)
        return;

    _pointer = MapViewOfFile(_map_object,    // object to map view of
                             FILE_MAP_WRITE, // read/write access
                             0,              // high offset:  map from
                             0,              // low offset:   beginning
                             0);             // default: map entire file

    if (_pointer == NULL)
    {
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                      (LPTSTR)&winapi_error, 0, NULL);
        throw Error("can't get pointer: %s", winapi_error);
    }

    strncpy(_id, name, sizeof(_id));

    if (_was_first)
        memset(_pointer, 0, size);
}

SharedMemory::~SharedMemory()
{
    if (_pointer != NULL)
        UnmapViewOfFile(_pointer);
    if (_map_object != NULL)
        CloseHandle(_map_object);
}

#endif
