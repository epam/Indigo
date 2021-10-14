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

_SIDManager& _SIDManager::getInst()
{
    static _SIDManager _instance;
    return _instance;
}

void _SIDManager::setSessionId(qword id)
{
    _sessionId() = id;
}

qword _SIDManager::getSessionId() const
{
    return _sessionId();
}

qword _SIDManager::allocSessionId()
{
    auto sidDataHolder = sf::xlock_safe_ptr(_sidDataHolder);
    auto& vacantSIDs = sidDataHolder->vacantSIDs;
    if (!vacantSIDs.empty())
    {
        auto id = vacantSIDs.top();
        vacantSIDs.pop();
        return id;
    }
    return sidDataHolder->lastNewSID++;
}

void _SIDManager::releaseSessionId(qword id)
{
    auto sidDataHolder = sf::xlock_safe_ptr(_sidDataHolder);
    sidDataHolder->vacantSIDs.push(id);
}

qword& _SIDManager::_sessionId()
{
    static thread_local qword _sessionId;
    return _sessionId;
}
