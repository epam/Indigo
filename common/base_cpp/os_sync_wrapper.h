/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __os_sync_wrapper_h__
#define __os_sync_wrapper_h__

#include "base_c/defs.h"
#include "base_c/os_sync.h"
#include "base_cpp/exception.h"

namespace indigo {

// os_mutex wrapper
class DLLEXPORT OsLock
{
public:
   OsLock  ();
   ~OsLock ();

   void Lock ();
   void Unlock ();
private:
   os_mutex _mutex;
};

// Automatic lock/unlock

template <typename T, bool lock_can_be_null>
class OsLockerT
{
public:
   OsLockerT (T *lock) : _lock(lock)
   {
      if (_lock != NULL)
         _lock->Lock();
      else if (!lock_can_be_null)
         throw Exception("Passed lock object pointer is NULL");
   }

   OsLockerT (T &lock) : _lock(&lock)
   {
      _lock->Lock();
   }

   ~OsLockerT ()
   {
      if (_lock != NULL)
         _lock->Unlock();
   }

private:
   T *_lock;
};
typedef OsLockerT<OsLock, false>       OsLocker;
typedef OsLockerT<OsLock, true>        OsLockerNullable;

//
// Semaphore wrapper
//
class DLLEXPORT OsSemaphore
{
public:
   OsSemaphore  (int initial_count, int max_count);
   ~OsSemaphore ();

   void Wait ();
   void Post ();
private:
   os_semaphore _sem;
};

//
// Message system
//

class OsMessageSystem
{
public:
   OsMessageSystem ();

   void SendMsg (int message, void *param = 0);
   void RecvMsg (int *message, void **result = 0);
private:

   OsSemaphore _sendSem;
   OsSemaphore _finishRecvSem;

   OsLock  _sendLock;
   OsLock  _recvLock;

   volatile int _localMessage;
   void * volatile _localParam;
};

//
// Thread-safe static local variables initialization object 
//

DLLEXPORT OsLock & osStaticObjConstructionLock ();

// Local static variables with constructors should have OsLock
// guard to avoid thread conflicts.
// This object should be declared as ONLY static object because
// _was_created variable should be zero by default.
template <typename T>
class ThreadSafeStaticObj
{
public:
   ~ThreadSafeStaticObj()
   {
      if (_was_created)
      {
         _obj->~T();
         _obj = 0;
         _was_created = false;
      }
   }

   T * ptr ()
   {
      return _ptr();
   }
   T & ref ()
   {
      return *_ptr();
   }
   T * operator -> ()
   {
      return ptr();
   }

private:
   void _ensureInitialized ()
   {
      if (!_was_created)
      {
         OsLocker locker(osStaticObjConstructionLock());

         if (!_was_created)
         {
            _obj = new ((void *)_obj_data) T;
            _was_created = true;
         }
      }
   }

   T * _ptr ()
   {
      _ensureInitialized();
      return _obj;
   }

   T *_obj;
   char _obj_data[sizeof(T)];
   volatile bool _was_created; // Zero for static objects
};

}

#endif // __os_sync_wrapper_h__
