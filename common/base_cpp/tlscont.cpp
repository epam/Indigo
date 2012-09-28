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

#include "base_cpp/tlscont.h"

using namespace indigo;

_SIDManager _SIDManager::_instance;
OsLock _SIDManager::_lock;

IMPL_ERROR(_SIDManager, "TLS");

_SIDManager& _SIDManager::getInst (void)
{
   return _instance;
}

_SIDManager::~_SIDManager (void)
{
   qword *pId;
   osTlsGetValue((void**)&pId, _tlsIdx);
   delete pId;
   osTlsFree(_tlsIdx);
}

void _SIDManager::setSessionId (qword id)
{    
   OsLocker locker(_lock);

   if (!_allSIDs.find(id))
      _allSIDs.insert(id);

   qword *pId = _getID();
   if (pId == NULL)
   {
      pId = new qword(id);
      osTlsSetValue(_tlsIdx, (void*)pId);
   }
   else
      *pId = id;
}

qword _SIDManager::allocSessionId  (void)
{
   OsLocker locker(_lock);

   qword id;
   if (_vacantSIDs.size() > 0)
      id = _vacantSIDs.pop();
   else {
      while (_allSIDs.find(_lastNewSID))
         ++_lastNewSID;

      id = _lastNewSID;
      _allSIDs.insert(id);

      ++_lastNewSID;
   }
   return id;
}

qword _SIDManager::getSessionId (void)
{
   qword *pId = _getID(); 
   qword id;
   if (pId == NULL) 
   {
      id = allocSessionId();
      setSessionId(id);
   } 
   else
      id = *pId;

   return id;
}

void _SIDManager::releaseSessionId (qword id)
{
   OsLocker locker(_lock);

   _vacantSIDs.push(id);
}

qword * _SIDManager::_getID (void) const
{
   void* pId;
   osTlsGetValue(&pId, _tlsIdx);
   return (qword *)pId;
}

_SIDManager::_SIDManager (void) : _lastNewSID(0)
{
   if (osTlsAlloc(&_tlsIdx) == 0)
      throw Error("can't allocate thread local storage cell");
}    
