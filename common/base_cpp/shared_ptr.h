#ifndef __shared_ptr__
#define __shared_ptr__

#include "base_cpp/exception.h"
#include "base_cpp/red_black.h"
#include "base_cpp/os_sync_wrapper.h"

namespace indigo
{

class SharedPtrStaticData
{
protected:
   static RedBlackMap<void *, int> _counters;
   static OsLock _lock;
};

DECL_EXCEPTION(SharedPtrError);

template <typename T> class SharedPtr : public SharedPtrStaticData
{
public:
   explicit SharedPtr (T *ptr = 0)
   {
      _ptr = ptr;
      _capture();
   }

   ~SharedPtr ()
   {
      _release();
   }

   T * get () const
   {
      return _ptr;
   }

   T & ref () const
   {
      if (_ptr == 0)
         throw Error("no reference");

      return *_ptr;
   }

   T * operator -> () const
   {
      if (_ptr == 0)
         throw Error("no reference");

      return _ptr;
   }

   void reset (T *ptr)
   {
      if (ptr != _ptr)
      {
         _release();
         _ptr = ptr;
         _capture();
      }
   }

   DECL_TPL_ERROR(SharedPtrError);

protected:
   void _release ()
   {
      OsLocker locker(_lock);

      if (_ptr == 0)
         return;

      int &counter = _counters.at(_ptr);

      counter--;
      if (counter == 0)
      {
         _counters.remove(_ptr);
         delete _ptr;
      }
}

   void _capture ()
   {
      OsLocker locker(_lock);

      if (_ptr == 0)
         return;

      int *counter = _counters.at2(_ptr);

      if (counter == 0)
         _counters.insert(_ptr, 1);
      else
         (*counter)++;
   }

   T *_ptr;
   
private:
   SharedPtr (const SharedPtr &); // no implicit copy
};

}

#endif
