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

#include "IndigoCopyableObject.h"

#include <indigo.h>

using namespace indigo_cpp;

IndigoCopyableObject::IndigoCopyableObject(int id, IndigoSessionPtr session) : IndigoObject(id, std::move(session))
{
}

IndigoCopyableObject::IndigoCopyableObject(const IndigoCopyableObject& other) : IndigoObject(_cloneAndReturnId(other), other.session())
{
}

IndigoCopyableObject& IndigoCopyableObject::operator=(const IndigoCopyableObject& other)
{
    if (this != &other)
    {
        _id = std::make_unique<int>(_cloneAndReturnId(other));
        _session = other.session();
    }
    return *this;
}

int IndigoCopyableObject::_cloneAndReturnId(const IndigoObject& other)
{
    other.session()->setSessionId();
    return other.session()->_checkResult(indigoClone(other.id()));
}
