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
