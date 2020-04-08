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

#ifndef __shmem_h__
#define __shmem_h__

#include "base_cpp/ptr_array.h"

namespace indigo
{
    class SharedMemory
    {
    public:
        explicit SharedMemory(const char* name, int size, bool no_map_if_first);
        virtual ~SharedMemory();

        inline bool wasFirst()
        {
            return _was_first;
        }
        inline const char* getID()
        {
            return _id;
        }

        void* ptr()
        {
            return _pointer;
        }

        DECL_ERROR;

    private:
        bool _was_first;
        char _id[1024];
        void* _pointer;
#ifdef _WIN32
        void* _map_object;
#else
        int _shm_id;
        char _filename[1024];
#endif
    };

} // namespace indigo

#endif
