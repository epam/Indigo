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

#ifndef __tlscont_h__
#define __tlscont_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/pool.h"
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/red_black.h"
#include "base_cpp/ptr_array.h"
#include "base_c/os_tls.h"
#include "base_cpp/auto_ptr.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

// Session identifiers manager.
// Every thread have local session ID that corresponds to the all
// local session variables.
class DLLEXPORT _SIDManager {
public:
   static _SIDManager& getInst (void);
   ~_SIDManager (void);

   void  setSessionId     (qword id);
   qword allocSessionId   (void);
   qword getSessionId     (void);
   // Add specified SID to the vacant list. 
   // This method should be called before thread exit if SID was 
   // assigned automatically (not by manual TL_SET_SESSION_ID call)
   void releaseSessionId (qword id);

   DECL_ERROR;

private:
   _SIDManager (void);
   qword * _getID         () const;

   // Thread local key for storing current session ID
   TLS_IDX_TYPE _tlsIdx;
   RedBlackSet<qword> _allSIDs;
   qword _lastNewSID;
   // Array with vacant SIDs
   Array<qword> _vacantSIDs;

   static _SIDManager _instance;   
   static OsLock _lock;
};

// Macros for managing session IDs for current thread
#define TL_GET_SESSION_ID()       _SIDManager::getInst().getSessionId()
#define TL_SET_SESSION_ID(id)     _SIDManager::getInst().setSessionId(id)
#define TL_ALLOC_SESSION_ID()     _SIDManager::getInst().allocSessionId()
#define TL_RELEASE_SESSION_ID(id) _SIDManager::getInst().releaseSessionId(id)

// Container that keeps one instance of specifed type per session
template <typename T>
class _SessionLocalContainer {
public:
   T& getLocalCopy (void)
   {
      return getLocalCopy(_SIDManager::getInst().getSessionId());
   }

   T& getLocalCopy (const qword id)
   {
      OsLocker locker(_lock.ref());
      AutoPtr<T>& ptr = _map.findOrInsert(id);
      if (ptr.get() == NULL)
         ptr.reset(new T());
      return ptr.ref();
   }

private:
   typedef RedBlackObjMap<qword, AutoPtr<T> > _Map;

   _Map           _map;
   ThreadSafeStaticObj<OsLock> _lock;
};

// Macros for working with global variables per each session
// By tradition this macros start with TL_, but should start with SL_
#define TL_DECL_EXT(type, name) extern _SessionLocalContainer< type > TLSCONT_##name
#define TL_DECL(type, name) static _SessionLocalContainer< type > TLSCONT_##name
#define TL_GET(type, name) type& name = (TLSCONT_##name).getLocalCopy()
#define TL_DECL_GET(type, name) TL_DECL(type, name); TL_GET(type, name)
#define TL_GET2(type, name, realname) type& name = (TLSCONT_##realname).getLocalCopy()
#define TL_GET_BY_ID(type, name, id) type& name = (TLSCONT_##name).getLocalCopy(id)
#define TL_DEF(className, type, name) _SessionLocalContainer< type > className::TLSCONT_##name
#define TL_DEF_EXT(type, name) _SessionLocalContainer< type > TLSCONT_##name

// Pool for local variables, reused in consecutive function calls, 
// but not required to preserve their state
template <typename T>
class _ReusableVariablesPool {
public:
   _ReusableVariablesPool  () { is_valid = true; }
   ~_ReusableVariablesPool () { is_valid = false; }
   bool isValid () const { return is_valid; }

   T& getVacant (int& idx)
   {  
      OsLocker locker(_lock);
      if (_vacant_indices.size() != 0)
      {
         idx = _vacant_indices.pop();
         return *_objects[idx];
      }
      _objects.add(new T);
      idx = _objects.size() - 1;
      _vacant_indices.reserve(idx + 1);
      return *_objects[idx];
   }

   void release (int idx)
   {
      OsLocker locker(_lock);
      _vacant_indices.push(idx);
   }

private:
   OsLock _lock;
   bool is_valid;

   PtrArray< T > _objects;
   Array<int> _vacant_indices;
};

// Utility class for automatically release call
template <typename T>
class _ReusableVariablesAutoRelease {
public:
   _ReusableVariablesAutoRelease () : _idx(-1), _var_pool(0) {}
   
   void init (int idx, _ReusableVariablesPool< T > *var_pool) 
   {
      _idx = idx;
      _var_pool = var_pool;
   }

   ~_ReusableVariablesAutoRelease (void)
   {
      if (_var_pool == 0)
         return;
      // Check if the _var_pool destructor have not been called already
      // (this can happen on program exit)
      if (_var_pool->isValid())
         _var_pool->release(_idx);
   }
private:
   int _idx;
   _ReusableVariablesPool< T >* _var_pool;
};                   

}

// "Quasi-static" variable definition
#define QS_DEF(TYPE, name) \
   static ThreadSafeStaticObj<_ReusableVariablesPool< TYPE > > _POOL_##name; \
   int _POOL_##name##_idx;                                             \
   TYPE &name = _POOL_##name->getVacant(_POOL_##name##_idx);           \
   _ReusableVariablesAutoRelease< TYPE > _POOL_##name##_auto_release;  \
   _POOL_##name##_auto_release.init(_POOL_##name##_idx, _POOL_##name.ptr())

//
// Reusable class members definition
// By tradition this macros start with TL_, but should start with SL_
//

// Add this to class definition  
#define TL_CP_DECL(TYPE, name) \
   static TYPE& _GET_##name##_REF (_ReusableVariablesAutoRelease< TYPE > &auto_release) \
   {                                                                          \
      static ThreadSafeStaticObj< _ReusableVariablesPool< TYPE > > pool;      \
      int idx;                                                                \
      TYPE &var = pool->getVacant(idx);                                        \
      auto_release.init(idx, pool.ptr());                                          \
      return var;                                                             \
   }                                                                          \
   _ReusableVariablesAutoRelease< TYPE > _POOL_##name##_auto_release;         \
   TYPE& name

// Add this to constructor initialization list
#define TL_CP_GET(name) \
   name(_GET_##name##_REF(_POOL_##name##_auto_release))

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __tlscont_h__
