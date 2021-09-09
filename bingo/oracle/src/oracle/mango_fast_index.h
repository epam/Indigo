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

#ifndef __mango_fast_index__
#define __mango_fast_index__

#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"
#include "oracle/bingo_fetch_engine.h"
#include "oracle/bingo_fingerprints.h"
#include "oracle/ora_wrap.h"

using namespace indigo;

class MangoFetchContext;

class MangoFastIndex : public BingoFetchEngine
{
public:
    MangoFastIndex(MangoFetchContext& context);
    ~MangoFastIndex() override;

    void prepareSubstructure(OracleEnv& env);
    void prepareSimilarity(OracleEnv& env);
    void prepareTautomerSubstructure(OracleEnv& env);

    void fetch(OracleEnv& env, int maxrows) override;
    bool end() override;
    float calcSelectivity(OracleEnv& env, int total_count) override;
    int getIOCost(OracleEnv& env, float selectivity) override;

    bool getLastRowid(OraRowidText& id) override;

    DECL_ERROR;

protected:
    enum
    {
        _SUBSTRUCTURE = 1,
        _SIMILARITY = 2,
        _TAUTOMER_SUBSTRUCTURE = 3
    };

    MangoFetchContext& _context;

    int _fetch_type;

    int _cur_idx;
    int _matched;
    int _unmatched;

    int _last_id;

    BingoFingerprints::Screening _screening;

    bool _loadCoords(OracleEnv& env, const char* rowid, Array<char>& coords);
    void _match(OracleEnv& env, int idx);
    int _countOnes(int idx);

    void _decompressRowid(const Array<char>& stored, OraRowidText& rid);

    void _fetchSubstructure(OracleEnv& env, int maxrows);
    void _fetchSimilarity(OracleEnv& env, int maxrows);

private:
    MangoFastIndex(const MangoFastIndex&); // noimplicitcopy
};

#endif
