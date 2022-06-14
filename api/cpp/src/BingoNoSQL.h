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

#pragma once

#include "BingoResultIterator.h"
#include "IndigoMolecule.h"
#include "IndigoQueryMolecule.h"
#include "IndigoSession.h"
#include "IndigoSimilarityMetric.h"

#include <string>

namespace indigo_cpp
{
    class IndigoChemicalStructure;
    class IndigoSession;

    template <typename target_t, typename query_t>
    class BingoNoSQL
    {
    public:
        BingoNoSQL() = delete;
        BingoNoSQL(const BingoNoSQL&) = delete;
        BingoNoSQL& operator=(const BingoNoSQL&) = delete;
        BingoNoSQL(BingoNoSQL&&) noexcept = default;
        BingoNoSQL& operator=(BingoNoSQL&&) = delete;
        ~BingoNoSQL();

        static BingoNoSQL createDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options = "");

        static BingoNoSQL loadDatabaseFile(IndigoSessionPtr session, const std::string& path, const std::string& options = "");

        void close();

        int insertRecord(const target_t& entity);
        int insertIterator(const IndigoIterator<target_t>& iterator);
        void deleteRecord(int recordId);

        BingoResultIterator<target_t> searchSub(const query_t& query, const std::string& options = "") const;
        BingoResultIterator<target_t> searchExact(const target_t& query, const std::string& options = "") const;
        BingoResultIterator<target_t> searchSim(const target_t& query, double min, double max = 1.0,
                                                IndigoSimilarityMetric metric = IndigoSimilarityMetric::TANIMOTO) const;
        BingoResultIterator<target_t> searchSim(const target_t& query, const double min, const double max, const std::string& options = "") const;

        IndigoSessionPtr session;

        std::string getStatistics(bool for_session = true) const;

    private:
        BingoNoSQL(IndigoSessionPtr indigo, int e);

        int id;
    };

    using BingoMolecule = BingoNoSQL<IndigoMolecule, IndigoQueryMolecule>;
}
