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

#if !defined(_WIN32)

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "base_cpp/shmem.h"

using namespace indigo;

IMPL_ERROR(SharedMemory, "shared memory");

SharedMemory::SharedMemory(const char* name, int size, bool no_map_if_first)
{
    snprintf(_filename, sizeof(_filename), "/tmp/indigo_shm_%s", name);

    _pointer = NULL;
    _was_first = false;

    int key = ftok(_filename, 'D'); // 'D' has no actual meaning

    if (key == -1 && errno == ENOENT)
    {
        // create file
        int fd = creat(_filename, 0666);

        if (fd == -1)
            throw Error("can't create %s: %s", _filename, strerror(errno));

        key = ftok(_filename, 'D');
    }

    if (key == -1)
        throw Error("can't get key: %s", strerror(errno));

    _shm_id = -1;

    if (!no_map_if_first)
    {
        // try to create
        _shm_id = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);

        if (_shm_id == -1)
        {
            if (errno != EEXIST)
                throw Error("can't shmget: %s", strerror(errno));
        }
        else
            _was_first = true;
    }

    if (_shm_id == -1)
    {
        _shm_id = shmget(key, size, 0666);

        if (_shm_id == -1)
        {
            if (errno == ENOENT)
                return;
            throw Error("can't second shmget (%d): %s", key, strerror(errno));
        }
    }

    _pointer = shmat(_shm_id, NULL, 0);

    if (_pointer == (void*)-1)
        throw Error("can't shmat: %s", strerror(errno));

    strncpy(_id, name, sizeof(_id));

    if (_was_first)
        memset(_pointer, 0, size);
}

SharedMemory::~SharedMemory()
{
    struct shmid_ds ds;

    // detach from the memory segment
    if (shmdt((char*)_pointer) != 0)
        return; // throwing exceptions on destructor is bad

    // get information about the segment
    if (shmctl(_shm_id, IPC_STAT, &ds) != 0)
        return;

    if (ds.shm_nattch < 1)
    {
        // nobody is using the segment; remove it
        shmctl(_shm_id, IPC_RMID, NULL);
        // and remove the temporary file
        unlink(_filename);
    }
}

#endif
