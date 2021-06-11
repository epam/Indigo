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

#include "oracle/ora_wrap.h"
#include "oracle/ringo_oracle.h"
#include <memory>
#include "base_cpp/output.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/ringo_shadow_table.h"


const char* bad_reaction_warning = "WARNING: bad reaction: %s\n";
const char* bad_reaction_warning_rowid = "WARNING: bad reaction %s: %s\n";

RingoOracleContext::RingoOracleContext(BingoContext& context) : RingoContext(context), shadow_table(context.id), fingerprints(context.id)
{
}

RingoOracleContext::~RingoOracleContext()
{
}

BingoOracleContext& RingoOracleContext::context()
{
    return (BingoOracleContext&)_context;
}

RingoOracleContext& RingoOracleContext::get(OracleEnv& env, int id, bool lock)
{
    bool config_reloaded;

    BingoOracleContext& context = BingoOracleContext::get(env, id, lock, &config_reloaded);

    RingoContext* already = _get(id, context);
    RingoOracleContext* roc;
    std::unique_ptr<RingoOracleContext> res;

    if (already != 0)
        roc = (RingoOracleContext*)already;
    else
    {
        res = std::make_unique<RingoOracleContext>(context);
        roc = res.get();
        config_reloaded = true;
    }

    if (config_reloaded)
    {
        roc->fingerprints.init(context, context.fp_parameters.fingerprintSizeExtOrdSim() * 2);
    }

    if (already == 0)
    {
        OsLocker locker(_instances_lock);
        TL_GET(PtrArray<RingoContext>, _instances);

        _instances.add(res.release());
        return *(RingoOracleContext*)_instances.top();
    }

    return *roc;
}
