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

#include "oracle/mango_fetch_context.h"
#include "core/bingo_context.h"
#include "core/mango_matchers.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_shadow_fetch.h"

TL_DEF(MangoFetchContext, PtrArray<MangoFetchContext>, _instances);
std::mutex MangoFetchContext::_instances_lock;

IMPL_ERROR(MangoFetchContext, "mango fetch context");

MangoFetchContext::MangoFetchContext(int id_, MangoOracleContext& context, const Array<char>& query_id)
    : substructure(context.context()), similarity(context.context()), exact(context.context()), tautomer(context.context()), gross(context.context()),
      _context(context)
{
    id = id_;
    context_id = context.context().id;
    _query_id.copy(query_id);
    fresh = false;
    fetch_engine = 0;

    shadow_fetch = std::make_unique<MangoShadowFetch>(*this);
    fast_index = std::make_unique<MangoFastIndex>(*this);
}

MangoFetchContext& MangoFetchContext::get(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<MangoFetchContext>, _instances);

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            return *_instances[i];

    throw Error("context #%d not found", id);
}

MangoFetchContext& MangoFetchContext::create(MangoOracleContext& context, const Array<char>& query_id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<MangoFetchContext>, _instances);

    int id = 1;

    for (int i = 0; i < _instances.size(); i++)
        if (_instances[i]->id >= id)
            id = _instances[i]->id + 1;

    std::unique_ptr<MangoFetchContext> new_context = std::make_unique<MangoFetchContext>(id, context, query_id);

    const BingoOracleContext& boc = context.context();

    new_context->id = id;

    _instances.add(new_context.release());
    return *_instances.top();
}

MangoFetchContext* MangoFetchContext::findFresh(int context_id, const Array<char>& query_id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<MangoFetchContext>, _instances);

    int i;

    for (i = 0; i < _instances.size(); i++)
    {
        MangoFetchContext* instance = _instances[i];

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

void MangoFetchContext::remove(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<MangoFetchContext>, _instances);
    int i;

    for (i = 0; i < _instances.size(); i++)
        if (_instances[i]->id == id)
            break;

    if (i == _instances.size())
        throw Error("remove(): context #%d not found", id);

    _instances.remove(i);
}

void MangoFetchContext::removeByContextID(int id)
{
    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<MangoFetchContext>, _instances);
    int i;

    for (i = _instances.size() - 1; i >= 0; i--)
        if (_instances[i]->context_id == id)
            _instances.remove(i);
}
