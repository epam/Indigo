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

#ifndef __ringo_fast_index__
#define __ringo_fast_index__

#include "bingo_fetch_engine.h"
#include "bingo_fingerprints.h"

namespace indigo
{

    class RingoFetchContext;

    class RingoFastIndex : public BingoFetchEngine
    {
    public:
        explicit RingoFastIndex(RingoFetchContext& context);
        ~RingoFastIndex() override;

        void prepareSubstructure(OracleEnv& env);

        void fetch(OracleEnv& env, int maxrows) override;
        bool end() override;
        float calcSelectivity(OracleEnv& env, int total_count) override;
        int getIOCost(OracleEnv& env, float selectivity) override;

        bool getLastRowid(OraRowidText& id) override;

        int getTotalCount(OracleEnv& env);

        DECL_ERROR;

    protected:
        enum
        {
            _SUBSTRUCTURE = 1
        };

        RingoFetchContext& _context;

        int _fetch_type;
        int _cur_idx;
        int _matched;
        int _unmatched;

        int _last_id;

        BingoFingerprints::Screening _screening;

        void _match(OracleEnv& env, int idx);
        void _decompressRowid(const Array<char>& stored, OraRowidText& rid);

    private:
        RingoFastIndex(const RingoFastIndex&); // noimplicitcopy
    };

} // namespace indigo

#endif
