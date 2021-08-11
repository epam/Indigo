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
#include "oracle/mango_oracle.h"
#include <memory>
#include "base_cpp/output.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_shadow_table.h"

const char* bad_molecule_warning = "WARNING: bad molecule: %s\n";
const char* bad_molecule_warning_rowid = "WARNING: bad molecule %s: %s\n";

MangoOracleContext::MangoOracleContext(BingoContext& context) : MangoContext(context), shadow_table(context.id), fingerprints(context.id)
{
}

MangoOracleContext::~MangoOracleContext()
{
}

BingoOracleContext& MangoOracleContext::context()
{
    return (BingoOracleContext&)_context;
}

MangoOracleContext& MangoOracleContext::get(OracleEnv& env, int id, bool lock)
{
    bool config_reloaded;

    BingoOracleContext& context = BingoOracleContext::get(env, id, lock, &config_reloaded);

    MangoContext* already = _get(id, context);
    MangoOracleContext* moc;

    std::unique_ptr<MangoOracleContext> res;

    if (already == 0)
    {
        res = std::make_unique<MangoOracleContext>(context);
        moc = res.get();
        config_reloaded = true;
    }
    else
        moc = (MangoOracleContext*)already;

    if (config_reloaded)
    {
        moc->fingerprints.init(context, context.fp_parameters.fingerprintSize(), context.fp_parameters.fingerprintSizeExt(),
                               context.fp_parameters.fingerprintSizeExtOrd());
    }

    if (already == 0)
    {
        std::lock_guard<std::mutex> locker(_instances_lock);
        TL_GET(PtrArray<MangoContext>, _instances);

        _instances.add(res.release());
        return *(MangoOracleContext*)_instances.top();
    }

    return *moc;
}
