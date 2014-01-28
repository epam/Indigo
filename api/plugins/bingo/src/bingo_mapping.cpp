#include "bingo_mapping.h"

using namespace bingo;

BingoMapping::BingoMapping  (size_t safe_prime) : _prime(safe_prime)
{
   _block_size = 100;
   _mapping_table.resize(safe_prime);
   for (int i = 0; i < _mapping_table.size(); i++)
      new(&(_mapping_table[i])) _MapList();
}

size_t BingoMapping::get (size_t id)
{
   _MapIterator iter;
   int idx_in_block;

   if (_findElem (id, iter, idx_in_block))
      return iter->buf[idx_in_block].second;

   return (size_t)-1;
}

void BingoMapping::getAll (size_t id1, Array<size_t> &id2_array)
{
   _MapList::Iterator it;
   _MapList &cur_list = _mapping_table[_hashFunc(id1)];

   int i;
   for (it = cur_list.begin(); it != cur_list.end(); it++)
   {
      for (i = 0; i < it->count; i++)
      {
         if (it->buf[i].first == id1)
            id2_array.push(it->buf[i].second);
      }
   }

   return;
}

void BingoMapping::add (size_t id1, size_t id2)
{
   _MapList &cur_list = _mapping_table[_hashFunc(id1)];
         
   if (cur_list.size() == 0 || cur_list.top()->count == _block_size)
   {
      BingoPtr< _ListCell > new_array_ptr;
      new_array_ptr.allocate();
      new(new_array_ptr.ptr()) _ListCell(_block_size);

      cur_list.pushBack(new_array_ptr);
   }

   int &top_size = cur_list.top()->count;
   cur_list.top()->buf[top_size++] = _KeyPair(id1, id2);
}

void BingoMapping::remove (size_t id)
{
   _MapList::Iterator it;
   _MapList &cur_list = _mapping_table[_hashFunc(id)];

   int idx_in_block;
   bool res = _findElem(id, it, idx_in_block);

   if (!res)
      throw Exception("BingoMapping: There is no such id");

   it->buf[idx_in_block].first = -1;
   it->buf[idx_in_block].second = -1;
}


size_t BingoMapping::_hashFunc (size_t id)
{
   return (id % _prime);
}

bool BingoMapping::_findElem (int id, _MapIterator &iter, int &idx_in_block)
{
   _MapList::Iterator it;
   _MapList &cur_list = _mapping_table[_hashFunc(id)];

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
