/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __string_pool_h__
#define __string_pool_h__

#include "base_cpp/pool.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/auto_iter.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {
class DLLEXPORT StringPool
{
public:
   DECL_ERROR;

   StringPool ();
   ~StringPool ();

   int  add (const char *str);
   int  add (Array<char> &str);
   int  add (int size);
   void remove (int idx);
   int  size () const;
   int  begin () const;
   int  end () const;
   int  next (int i) const;
   void clear ();

   char * at (int idx);
   const char * at (int idx) const;
   /*
    * Iterators
    */
   class PoolIter : public AutoIterator {
   public:
      PoolIter(StringPool &owner, int idx): AutoIterator(idx), _owner(owner)  {
      }
      PoolIter & operator++() {
         _idx = _owner.next(_idx);
         return *this;
      }
   private:
      StringPool &_owner;
   };
   class PoolAuto {
   public:
      PoolAuto(StringPool &owner) : _owner(owner) {
      }
      PoolIter begin(){
         return StringPool::PoolIter(_owner, _owner.begin());
      }
      PoolIter end() {
         return StringPool::PoolIter(_owner, _owner.end());
      }
   private:
      StringPool &_owner;
   };
   
   PoolAuto elements () {
      return PoolAuto(*this);
   }

protected:
   int _add (const char *str, int size);

   Pool<int>  _pool;
   PtrArray< Array<char> > _storage;

private:
   StringPool (const StringPool &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
