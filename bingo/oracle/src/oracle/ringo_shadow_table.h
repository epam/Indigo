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

#ifndef __ringo_shadow_table__
#define __ringo_shadow_table__

#include "base_cpp/tlscont.h"
#include "oracle/bingo_fetch_engine.h"
#include "oracle/ora_wrap.h"

using namespace indigo;

class RingoIndex;

class RingoShadowTable
{
public:
    explicit RingoShadowTable(int context_id);

    void drop(OracleEnv& env);
    void truncate(OracleEnv& env);
    void create(OracleEnv& env);
    void addReaction(OracleEnv& env, RingoIndex& index, const char* rowid, int blockno, int offset);
    bool getReactionLocation(OracleEnv& env, const char* rowid, int& blockno, int& offset);
    void deleteReaction(OracleEnv& env, const char* rowid);

    void analyze(OracleEnv& env);
    int countOracleBlocks(OracleEnv& env);

    const char* getName();

    DECL_ERROR;

protected:
    ArrayChar _table_name;

private:
    RingoShadowTable(RingoShadowTable&); // no implicit copy
};

#endif
