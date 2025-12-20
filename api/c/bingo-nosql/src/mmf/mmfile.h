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

#pragma once

#include <cstddef>
#include <cstdio>
#include <new>
#include <string>

#include <common/base_c/defs.h>

namespace bingo
{
    class MMFile
    {
    public:
        MMFile(std::string filename, size_t buf_size, bool create_flag, bool read_only);
        ~MMFile();

        MMFile& operator=(const MMFile&) = delete;
        MMFile(const MMFile&) = delete;
        MMFile& operator=(MMFile&&) = delete;
        MMFile(MMFile&&) = default;
        MMFile() = delete;

        void* ptr(ptrdiff_t offset = 0);
        const void* ptr(ptrdiff_t offset = 0) const;

        const char* name() const;

        size_t size() const;

    private:
#ifdef _WIN32
        void* _h_map_file;
        void* _h_file;
#elif (defined __GNUC__ || defined __APPLE__)
        int _fd;
#endif
        void* _ptr;
        std::string _filename;
        size_t _len;

        static char* _getSystemErrorMsg();
    };
}
