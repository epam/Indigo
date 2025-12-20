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

#include <vector>

#include "base_cpp/array.h"

#include "mmf_array.h"
#include "mmf_list.h"
#include "mmf_ptr.h"

namespace bingo
{
    class MMFMapping
    {
    public:
        MMFMapping(size_t safe_prime = 200087);

        size_t get(size_t id);

        void getAll(size_t id1, indigo::Array<size_t>& id2_array);

        void add(size_t id1, size_t id2);

        void remove(size_t id);

    private:
        typedef std::pair<size_t, size_t> _KeyPair;

        struct _ListCell
        {
            MMFPtr<_KeyPair> buf;
            int count;

            _ListCell(int size)
            {
                buf.allocate(size);
                count = 0;
            }
        };

        typedef MMFList<_ListCell> _MapList;
        typedef MMFList<_ListCell>::Iterator _MapIterator;

        size_t _hashFunc(size_t id);

        bool _findElem(size_t id, _MapIterator& iter, int& idx_in_block);

        size_t _prime;
        int _block_size;
        MMFArray<MMFPtr<_MapList>> _mapping_table;
    };
}
