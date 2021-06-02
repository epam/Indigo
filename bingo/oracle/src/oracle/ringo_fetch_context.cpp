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

#include "oracle/ringo_fetch_context.h"
#include "core/bingo_context.h"
#include "core/ringo_matchers.h"
#include "oracle/bingo_oracle_context.h"

TL_DEF(RingoFetchContext, PtrArray<RingoFetchContext>, _instances);
OsLock RingoFetchContext::_instances_lock;

IMPL_ERROR(RingoFetchContext, "ringo fetch context");

RingoFetchContext::RingoFetchContext(int id_, RingoOracleContext& context, const ArrayChar& query_id)
    : substructure(context.context()), exact(context.context()), _context(context)
{
    id = id_;
    context_id = context.context().id;
    _query_id.copy(query_id);
    fresh = false;
    fetch_engine = 0;

    shadow_fetch.reset(new RingoShadowFetch(*this));
    fast_index.reset(new RingoFastIndex(*this));
}

RingoFetchContext& RingoFetchContext::get(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoFetchContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            return *_instances[i];

    throw Error("context #%d not found", id);
}

RingoFetchContext& RingoFetchContext::create(RingoOracleContext& context, const ArrayChar& query_id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoFetchContext>, _instances);

    int id = 1;

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id >= id)
            id = _instances[i]->id + 1;

    AutoPtr<RingoFetchContext> new_context(new RingoFetchContext(id, context, query_id));
    const BingoOracleContext& boc = context.context();

    new_context->id = id;

    _instances.add(new_context.release());
    return *_instances.top();
}

RingoFetchContext* RingoFetchContext::findFresh(int context_id, const ArrayChar& query_id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoFetchContext>, _instances);

    int i;

    for (i = 0; i < _instances.size(); i++)
    {
        RingoFetchContext* instance = _instances[i];

        if (!instance->fresh)
            continue;

        if (instance->context_id != context_id)
            continue;

        if (instance->_query_id.memcmp(query_id) != 0)
            continue;

        return instance;
    }

    return 0;
}

void RingoFetchContext::remove(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoFetchContext>, _instances);
    int i;

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            break;

    if (i == _instances.size())
        throw Error("remove(): context #%d not found", id);

    _instances.remove(i);
}

void RingoFetchContext::removeByContextID(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<RingoFetchContext>, _instances);
    int i;

    for (i = _instances.size() - 1; i >= 0; i--)
        if (_instances[i]->context_id == id)
            _instances.remove(i);
}
