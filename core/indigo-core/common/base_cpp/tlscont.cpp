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

#include "base_cpp/tlscont.h"

using namespace indigo;

IMPL_ERROR(_SIDManager, "TLS");

_SIDManager& _SIDManager::getInst()
{
    static _SIDManager _instance;
    return _instance;
}

std::mutex& _SIDManager::getLock()
{
    static std::mutex _lock;
    return _lock;
}

_SIDManager::~_SIDManager(void)
{
    qword* pId;
    osTlsGetValue((void**)&pId, _tlsIdx);
    delete pId;
    osTlsFree(_tlsIdx);
}

void _SIDManager::setSessionId(qword id)
{
    std::lock_guard<std::mutex> locker(_SIDManager::getLock());

    if (!_allSIDs.find(id))
        _allSIDs.insert(id);

    qword* pId = _getID();
    if (pId == NULL)
    {
        pId = new qword(id);
        osTlsSetValue(_tlsIdx, (void*)pId);
    }
    else
        *pId = id;
}

qword _SIDManager::allocSessionId(void)
{
    std::lock_guard<std::mutex> locker(_SIDManager::getLock());

    qword id;
    if (_vacantSIDs.size() > 0)
        id = _vacantSIDs.pop();
    else
    {
        while (_allSIDs.find(_lastNewSID))
            ++_lastNewSID;

        id = _lastNewSID;
        _allSIDs.insert(id);

        ++_lastNewSID;
    }
    return id;
}

qword _SIDManager::getSessionId(void)
{
    qword* pId = _getID();
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

void _SIDManager::releaseSessionId(qword id)
{
    std::lock_guard<std::mutex> locker(_SIDManager::getLock());

    _vacantSIDs.push(id);
}

qword* _SIDManager::_getID(void) const
{
    void* pId;
    osTlsGetValue(&pId, _tlsIdx);
    return (qword*)pId;
}

_SIDManager::_SIDManager(void) : _lastNewSID(0)
{
    if (osTlsAlloc(&_tlsIdx) == 0)
        throw Error("can't allocate thread local storage cell");
}
