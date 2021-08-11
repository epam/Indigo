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

#include "mango_context.h"
#include "bingo_context.h"

TL_DEF(MangoContext, PtrArray<MangoContext>, _instances);

std::mutex MangoContext::_instances_lock;

IMPL_ERROR(MangoContext, "mango context");

MangoContext::MangoContext(BingoContext& context)
    : substructure(context), similarity(context), exact(context), tautomer(context), gross(context), _context(context)
{
}

MangoContext::~MangoContext()
{
}

MangoContext* MangoContext::_get(int id, BingoContext& context)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            return _instances[i];

    return 0;
}

MangoContext* MangoContext::existing(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            return _instances[i];

    throw Error("context #%d not found", id);
}

MangoContext* MangoContext::get(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            return _instances[i];

    BingoContext* bingo_context = BingoContext::get(id);

    return &_instances.add(new MangoContext(*bingo_context));
}

void MangoContext::remove(int id)
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);
    int i;

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id == id)
            break;

    // if (i == _instances.size())
    //   throw Error("remove(): context #%d not found", id);

    if (i != _instances.size())
        _instances.remove(i);
}

int MangoContext::begin()
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);

    if (_instances.size() < 1)
        return 0;

    int i, min_id = _instances[0]->_context.id;

    for (i = 1; i < _instances.size(); i++)
        if (_instances[i]->_context.id < min_id)
            min_id = _instances[i]->_context.id;

    return min_id;
}

int MangoContext::end()
{
    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);

    if (_instances.size() < 1)
        return 0;

    int i, max_id = _instances[0]->_context.id;

    for (i = 1; i < _instances.size(); i++)
        if (_instances[i]->_context.id > max_id)
            max_id = _instances[i]->_context.id;

    return max_id + 1;
}

int MangoContext::next(int k)
{
    int i, next_id = end();

    std::lock_guard<std::mutex> locker(_instances_lock);
    TL_GET(PtrArray<MangoContext>, _instances);

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->_context.id > k && _instances[i]->_context.id < next_id)
            next_id = _instances[i]->_context.id;

    return next_id;
}
