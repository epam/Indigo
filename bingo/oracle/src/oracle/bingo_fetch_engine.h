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

#ifndef __bingo_fetch_engine__
#define __bingo_fetch_engine__

#include "base_cpp/list.h"
#include "base_cpp/tlscont.h"
#include "oracle/ora_wrap.h"

namespace indigo
{

    class BingoFetchEngine
    {
    public:
        explicit BingoFetchEngine();

        virtual ~BingoFetchEngine()
        {
        }

        virtual float calcSelectivity(OracleEnv& env, int total_count) = 0;
        virtual void fetch(OracleEnv& env, int maxrows) = 0;
        virtual bool end() = 0;
        virtual int getIOCost(OracleEnv& env, float selectivity) = 0;

        // In case of exceptions this rowid corresponds to the structure that was tried to be matched last time
        virtual bool getLastRowid(OraRowidText& id) = 0;

        CP_DECL;
        TL_CP_DECL(List<OraRowidText>, matched);
    };

} // namespace indigo

#endif
