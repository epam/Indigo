#ifndef __bingo_mapping__
#define __bingo_mapping__

#include <vector>

#include "bingo_ptr.h"

namespace bingo
{
   class BingoMapping
   {
   public:
      BingoMapping  (size_t safe_prime = 200087);

      size_t get (size_t id);

      void getAll (size_t id1, Array<size_t> &id2_array);
       
      void add (size_t id1, size_t id2);

      size_t remove (size_t id);

   private:
      typedef std::pair<int, int> _KeyPair;

      struct _ListCell
      {
         BingoPtr<_KeyPair> buf;
         int count;

         _ListCell( int size )
         {
            buf.allocate(size);
            count = 0;
         }
      };

      typedef BingoList<_ListCell> _MapList;
      typedef BingoList<_ListCell>::Iterator _MapIterator;

      size_t _hashFunc (size_t id);

      bool _findElem (int id, _MapIterator &iter, int &idx_in_block);

      size_t _prime;
      int _block_size;
      BingoArray< _MapList > _mapping_table;
   };
}

#endif //__bingo_mapping__