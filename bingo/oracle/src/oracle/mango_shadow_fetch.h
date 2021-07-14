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

#ifndef __mango_shadow_fetch__
#define __mango_shadow_fetch__

#include <memory>
#include "oracle/bingo_fetch_engine.h"
#include "oracle/mango_shadow_table.h"

using namespace indigo;

class MangoFetchContext;

class MangoShadowFetch : public BingoFetchEngine
{
public:
    MangoShadowFetch(MangoFetchContext& context);
    ~MangoShadowFetch() override;

    float calcSelectivity(OracleEnv& env, int total_count) override;
    void fetch(OracleEnv& env, int maxrows) override;
    bool end() override;
    int getIOCost(OracleEnv& env, float selectivity) override;
    virtual int getTotalCount(OracleEnv& env);

    bool getLastRowid(OraRowidText& id) override;

    int countOracleBlocks(OracleEnv& env);

    void prepareNonSubstructure(OracleEnv& env);
    void prepareNonTautomerSubstructure(OracleEnv& env);
    void prepareTautomer(OracleEnv& env, int right_part);
    void prepareExact(OracleEnv& env, int right_part);
    void prepareGross(OracleEnv& env, int right_part);
    void prepareMass(OracleEnv& env);

    DECL_ERROR;

protected:
    enum
    {
        _NON_SUBSTRUCTURE = 1,
        _NON_TAUTOMER_SUBSTRUCTURE = 2,
        _TAUTOMER = 3,
        _EXACT = 4,
        _GROSS = 5,
        _MASS = 6
    };

    void _prepareExactQueryStrings(Array<char>& table_copies, Array<char>& where_clause);

    MangoFetchContext& _context;

    Array<char> _table_name;
    Array<char> _components_table_name;
    int _total_count;
    Array<char> _counting_select;
    int _processed_rows;
    bool _end;
    std::unique_ptr<OracleEnv> _env;
    std::unique_ptr<OracleStatement> _statement;
    std::unique_ptr<OracleLOB> _lob_cmf;
    std::unique_ptr<OracleLOB> _lob_xyz;
    bool _executed;
    int _fetch_type;
    char _gross[1024];
    OraRowidText _rowid;
    bool _need_xyz;
    int _right_part;
};

#endif
