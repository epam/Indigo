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

#include "IndigoRDFileIterator.h"

#include <indigo.h>

#include "IndigoSession.h"

using namespace indigo_cpp;

IndigoRDFileIterator::iterator::iterator(IndigoRDFileIterator* obj) : _obj{obj}
{
}

IndigoRDFileIterator::iterator::reference IndigoRDFileIterator::iterator::operator*()
{
    return _obj->_current;
}

IndigoRDFileIterator::iterator& IndigoRDFileIterator::iterator::operator++()
{
    increment();
    return *this;
}

bool IndigoRDFileIterator::iterator::operator==(IndigoRDFileIterator::iterator rhs) const
{
    return _obj == rhs._obj;
}

bool IndigoRDFileIterator::iterator::operator!=(IndigoRDFileIterator::iterator rhs) const
{
    return !(rhs == *this);
}

void IndigoRDFileIterator::iterator::increment()
{
    _obj->next();
    if (!_obj->valid())
    {
        _obj = nullptr;
    }
}

IndigoRDFileIterator::IndigoRDFileIterator(const int id, IndigoSessionPtr session) : IndigoObject(id, std::move(session))
{
}

IndigoRDFileIterator::iterator IndigoRDFileIterator::begin()
{
    return ++iterator{this};
}

IndigoRDFileIterator::iterator IndigoRDFileIterator::end()
{
    return iterator{nullptr};
}

void IndigoRDFileIterator::next()
{
    session()->setSessionId();
    auto nextId = session()->_checkResult(indigoNext(id()));
    if (nextId == 0)
    {
        _current = nullptr;
    }
    else
    {
        _current = std::make_shared<IndigoReaction>(nextId, session());
    }
}

bool IndigoRDFileIterator::valid() const
{
    return _current != nullptr;
}
