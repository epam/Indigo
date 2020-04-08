#include "bingo_mapping.h"

using namespace bingo;

BingoMapping::BingoMapping(size_t safe_prime) : _prime(safe_prime)
{
    _block_size = 100;
    _mapping_table.resize(safe_prime);
}

size_t BingoMapping::get(size_t id)
{
    _MapIterator iter;
    int idx_in_block;

    if (_findElem(id, iter, idx_in_block))
        return iter->buf[idx_in_block].second;

    return (size_t)-1;
}

void BingoMapping::getAll(size_t id1, Array<size_t>& id2_array)
{
    id2_array.clear();

    if ((BingoAddr)(_mapping_table[_hashFunc(id1)]) == BingoAddr::bingo_null)
        return;

    _MapList::Iterator it;
    _MapList& cur_list = _mapping_table[_hashFunc(id1)].ref();

    int i;
    for (it = cur_list.begin(); it != cur_list.end(); it++)
    {
        for (i = 0; i < it->count; i++)
        {
            if (it->buf[i].first == id1)
                id2_array.push(it->buf[i].second);
        }
    }
}

void BingoMapping::add(size_t id1, size_t id2)
{
    if ((BingoAddr)(_mapping_table[_hashFunc(id1)]) == BingoAddr::bingo_null)
    {
        _mapping_table[_hashFunc(id1)].allocate();
        new (_mapping_table[_hashFunc(id1)].ptr()) _MapList();
    }

    _MapList& cur_list = _mapping_table[_hashFunc(id1)].ref();

    if (cur_list.size() == 0 || cur_list.top()->count == _block_size)
    {
        BingoPtr<_ListCell> new_array_ptr;
        new_array_ptr.allocate();
        new (new_array_ptr.ptr()) _ListCell(_block_size);

        cur_list.pushBack(new_array_ptr);
    }

    int& top_size = cur_list.top()->count;
    cur_list.top()->buf[top_size++] = _KeyPair(id1, id2);
}

void BingoMapping::remove(size_t id)
{
    if ((BingoAddr)(_mapping_table[_hashFunc(id)]) == BingoAddr::bingo_null)
        throw Exception("BingoMapping: There is no such id");

    _MapList::Iterator it;
    //_MapList &cur_list = _mapping_table[_hashFunc(id)].ref();

    int idx_in_block;
    bool res = _findElem(id, it, idx_in_block);

    if (!res)
        throw Exception("BingoMapping: There is no such id");

    it->buf[idx_in_block].first = -1;
    it->buf[idx_in_block].second = -1;
}

size_t BingoMapping::_hashFunc(size_t id)
{
    return (id % _prime);
}

bool BingoMapping::_findElem(size_t id, _MapIterator& iter, int& idx_in_block)
{
    if ((BingoAddr)(_mapping_table[_hashFunc(id)]) == BingoAddr::bingo_null)
        return false;

    _MapList::Iterator it;
    _MapList& cur_list = _mapping_table[_hashFunc(id)].ref();

    int i;
    for (it = cur_list.begin(); it != cur_list.end(); it++)
    {
        for (i = 0; i < it->count; i++)
        {
            if (it->buf[i].first == id)
                break;
        }
        if (i < it->count)
            break;
    }
    if (it == cur_list.end())
        return false;

    iter = it;
    idx_in_block = i;

    return true;
}
