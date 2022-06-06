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

#include "BingoNoSQL.h"

#include <bingo-nosql.h>

#include "IndigoException.h"
#include "IndigoIterator.h"

using namespace indigo_cpp;

namespace
{
    template <typename target_t, typename query_t>
    constexpr const char* getDBTypeString()
    {
        if (std::is_same<target_t, IndigoMolecule>::value)
        {
            static_assert(std::is_same<query_t, IndigoQueryMolecule>::value, "");
            return "molecule";
        }
        //    else if (std::is_same<target_t, IndigoReaction>::value)
        //    {
        //        static_assert(std::is_same<query_t, IndigoQueryReaction>::value, "");
        //        return "reaction";
        //    }
        throw IndigoException("Unknown DB type");
    }
}

template <typename target_t, typename query_t>
BingoNoSQL<target_t, query_t>::BingoNoSQL(IndigoSessionPtr session, int id) : session(std::move(session)), id(id)
{
}

template <typename target_t, typename query_t>
BingoNoSQL<target_t, query_t>::~BingoNoSQL()
{
    if (id >= 0)
    {
        session->setSessionId();
        session->_checkResult(bingoCloseDatabase(id));
        id = -1;
    }
}

template <typename target_t, typename query_t>
BingoNoSQL<target_t, query_t> BingoNoSQL<target_t, query_t>::createDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options)
{
    session->setSessionId();
    int id = session->_checkResult(bingoCreateDatabaseFile(path.c_str(), getDBTypeString<target_t, query_t>(), options.c_str()));
    return {std::move(session), id};
}

template <typename target_t, typename query_t>
BingoNoSQL<target_t, query_t> BingoNoSQL<target_t, query_t>::loadDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options)
{
    session->setSessionId();
    int id = session->_checkResult(bingoLoadDatabaseFile(path.c_str(), options.c_str()));
    return {std::move(session), id};
}

template <typename target_t, typename query_t>
void BingoNoSQL<target_t, query_t>::close()
{
    if (id >= 0)
    {
        session->setSessionId();
        session->_checkResult(bingoCloseDatabase(id));
        id = -1;
    }
}

template <typename target_t, typename query_t>
int BingoNoSQL<target_t, query_t>::insertRecord(const target_t& entity)
{
    session->setSessionId();
    return session->_checkResult(bingoInsertRecordObj(id, entity.id()));
}

template <typename target_t, typename query_t>
int BingoNoSQL<target_t, query_t>::insertIterator(const IndigoIterator<target_t>& iterator)
{
    session->setSessionId();
    return session->_checkResult(bingoInsertIteratorObj(id, iterator.id()));
}

template <typename target_t, typename query_t>
void BingoNoSQL<target_t, query_t>::deleteRecord(int recordId)
{
    session->setSessionId();
    session->_checkResult(bingoDeleteRecord(id, recordId));
}

template <typename target_t, typename query_t>
BingoResultIterator<target_t> BingoNoSQL<target_t, query_t>::searchSub(const query_t& query, const std::string& options) const
{
    session->setSessionId();
    return {session->_checkResult(bingoSearchSub(id, query.id(), options.c_str())), session};
}

template <typename target_t, typename query_t>
BingoResultIterator<target_t> BingoNoSQL<target_t, query_t>::searchSim(const target_t& query, const double min, const double max,
                                                                       const IndigoSimilarityMetric metric) const
{
    session->setSessionId();
    return {session->_checkResult(bingoSearchSim(id, query.id(), min, max, to_string(metric))), session};
}

template <typename target_t, typename query_t>
BingoResultIterator<target_t> BingoNoSQL<target_t, query_t>::searchSim(const target_t& query, const double min, const double max,
                                                                       const std::string& options) const
{
    session->setSessionId();
    return {session->_checkResult(bingoSearchSim(id, query.id(), min, max, options.c_str())), session};
}

template <typename target_t, typename query_t>
std::string BingoNoSQL<target_t, query_t>::getStatistics(bool for_session) const
{
    session->setSessionId();
    return session->_checkResultString(bingoProfilingGetStatistics(static_cast<int>(for_session)));
}

template <typename target_t, typename query_t>
BingoResultIterator<target_t> BingoNoSQL<target_t, query_t>::searchExact(const target_t& query, const std::string& options) const
{
    session->setSessionId();
    return {session->_checkResult(bingoSearchExact(id, query.id(), options.c_str())), session};
}

template class indigo_cpp::BingoNoSQL<IndigoMolecule, IndigoQueryMolecule>;
