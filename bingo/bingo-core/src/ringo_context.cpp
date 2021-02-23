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

#include "ringo_context.h"
#include "bingo_context.h"

TL_DEF(RingoContext, PtrArray<RingoContext>, _instances);

OsLock RingoContext::_instances_lock;

IMPL_ERROR(RingoContext, "ringo context");

RingoContext::RingoContext(BingoContext& context) : substructure(context), exact(context), ringoAAM(context), _context(context)
{
}

RingoContext::~RingoContext()
{
}

RingoContext* RingoContext::_get(int id, BingoContext& context)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            return _instances[i];

    return 0;
}

RingoContext* RingoContext::existing(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            return _instances[i];

    throw Error("context #%d not found", id);
}

RingoContext* RingoContext::get(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            return _instances[i];

    BingoContext* bingo_context = BingoContext::get(id);

    return &_instances.add(new RingoContext(*bingo_context));
}

void RingoContext::remove(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);
    int i;

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            break;

    // if (i == _instances.size())
    //   throw Error("remove(): context #%d not found", id);

    if (i != _instances.size())
        _instances.remove(i);
}

int RingoContext::begin()
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);

    if (_instances.size() < 1)
        return 0;

    int i, min_id = _instances[0]->_context.id;

    for (i = 1; i < _instances.size(); i++)
        if (_instances[i]->_context.id < min_id)
            min_id = _instances[i]->_context.id;

    return min_id;
}

int RingoContext::end()
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);

    if (_instances.size() < 1)
        return 0;

    int i, max_id = _instances[0]->_context.id;

    for (i = 1; i < _instances.size(); i++)
        if (_instances[i]->_context.id > max_id)
            max_id = _instances[i]->_context.id;

    return max_id + 1;
}

int RingoContext::next(int k)
{
    int i, next_id = end();

    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoContext>, _instances);

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id > k && _instances[i]->_context.id < next_id)
            next_id = _instances[i]->_context.id;

    return next_id;
}
