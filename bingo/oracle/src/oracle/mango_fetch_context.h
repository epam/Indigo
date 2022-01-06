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

#ifndef __mango_fetch_context__
#define __mango_fetch_context__

#include "core/mango_matchers.h"
#include "oracle/mango_fast_index.h"
#include "oracle/mango_oracle.h"
#include "oracle/mango_shadow_fetch.h"
#include <memory>

namespace indigo
{

    class MangoShadowFetch;

    class MangoFetchContext
    {
    public:
        MangoFetchContext(int id, MangoOracleContext& context, const Array<char>& query_id);

        std::unique_ptr<MangoFastIndex> fast_index;
        std::unique_ptr<MangoShadowFetch> shadow_fetch;

        BingoFetchEngine* fetch_engine;

        MangoSubstructure substructure;
        MangoSimilarity similarity;
        MangoExact exact;
        MangoTautomer tautomer;
        MangoGross gross;
        MangoMass mass;

        int id;
        int context_id;
        bool fresh; // 'true' after selectivity calculation and before index start

        static MangoFetchContext& create(MangoOracleContext& context, const Array<char>& query_id);
        static MangoFetchContext& get(int id);
        static MangoFetchContext* findFresh(int context_id, const Array<char>& query_id);
        static void remove(int id);
        static void removeByContextID(int id);

        inline MangoOracleContext& context()
        {
            return _context;
        }

        DECL_ERROR;

    protected:
        Array<char> _query_id;
        MangoOracleContext& _context;

        TL_DECL(PtrArray<MangoFetchContext>, _instances);
        static std::mutex _instances_lock;
    };

} // namespace indigo

#endif
