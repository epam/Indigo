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

#include <gtest/gtest.h>

#include <atomic>
#include <thread>

#include <BingoNoSQL.h>
#include <IndigoIterator.h>
#include <IndigoSession.h>

#include "common.h"

using namespace std;
using namespace indigo_cpp;

namespace
{
    enum class SearchType
    {
        EXACT,
        SUB,
        SIM
    };

    void partCreate()
    {
        auto session = IndigoSession::create();
        auto bingo = BingoMolecule::createDatabaseFile(session, "mol_part_db", "mt_size:2000");
        bingo.insertIterator(session->iterateSmilesFile(dataPath("molecules/basic/sample_100000.smi")));
        bingo.close();
    }

    void makeSearchSub(const BingoMolecule& bingo, const IndigoQueryMolecule& queryMolecule, const string& options, atomic<int>& resultCounter)
    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(queryMolecule, options))
        {
            ++search_counter;
        }
        resultCounter += search_counter;
    }

    void makeSearchExact(const BingoMolecule& bingo, const IndigoMolecule& molecule, const string& options, atomic<int>& resultCounter)
    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchExact(molecule, options))
        {
            ++search_counter;
        }
        resultCounter += search_counter;
    }

    void makeSearchSim(const BingoMolecule& bingo, const IndigoMolecule& molecule, const string& options, atomic<int>& resultCounter)
    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSim(molecule, 0.5, 1.0, options))
        {
            ++search_counter;
        }
        resultCounter += search_counter;
    }

    void moleculeSearch(const SearchType& type, const BingoNoSQL<IndigoMolecule, IndigoQueryMolecule>& bingo, const int parts, vector<thread>& threads,
                        atomic<int>& fullSearchCounter, atomic<int>& partSearchCounter, int& index,
                        const shared_ptr<IndigoMolecule>& rdfMolecule)
    {
        function<void(const BingoMolecule&, const IndigoMolecule&, const string&, atomic<int>&)> searchFunction;
        if (type == SearchType::EXACT)
        {
            searchFunction = makeSearchExact;
        }
        else
        {
            searchFunction = makeSearchSim;
        }

        const auto molecule = *rdfMolecule;
        threads.emplace_back(searchFunction, cref(bingo), cref(molecule), "", ref(fullSearchCounter));
        for (int i = 1; i <= parts; i++)
        {
            stringstream ssOptions;
            ssOptions << "part:" << i << "/" << parts << "";
            threads.emplace_back(searchFunction, cref(bingo), cref(molecule), ssOptions.str(), ref(partSearchCounter));
        }
        for (auto& thread : threads)
        {
            thread.join();
        };
        EXPECT_EQ(fullSearchCounter, partSearchCounter);
        ++index;
    }

    void queryMoleculeSearch(IndigoSessionPtr& session, const BingoNoSQL<IndigoMolecule, IndigoQueryMolecule>& bingo, const int parts,
                             const shared_ptr<IndigoMolecule>& rdfMolecule, vector<thread>& threads, atomic<int>& fullSearchCounter,
                             atomic<int>& partSearchCounter)
    {
        function<void(const BingoMolecule&, const IndigoQueryMolecule&, const string&, atomic<int>&)> searchFunction;
        searchFunction = makeSearchSub;
        const auto queryMolecule = session->loadQueryMolecule(rdfMolecule->rawData());
        threads.emplace_back(searchFunction, cref(bingo), cref(queryMolecule), "", ref(fullSearchCounter));
        for (int i = 1; i <= parts; i++)
        {
            stringstream ssOptions;
            ssOptions << "part:" << i << "/" << parts << "";
            threads.emplace_back(searchFunction, cref(bingo), cref(queryMolecule), ssOptions.str(), ref(partSearchCounter));
        }
        for (auto& thread : threads)
        {
            thread.join();
        };
        EXPECT_EQ(fullSearchCounter, partSearchCounter);
    }

    void partTest(SearchType type)
    {
        auto session = IndigoSession::create();
        const auto bingo = BingoMolecule::loadDatabaseFile(session, "mol_part_db", "");
        auto index = 0;

        const auto parts = 3;
        for (const auto& rdfMolecule : session->iterateSDFile(dataPath("molecules/basic/rand_queries_small.sdf")))
        {
            vector<thread> threads;
            threads.reserve(parts + 1);
            atomic<int> fullSearchCounter{0};
            atomic<int> partSearchCounter{0};

            if (type == SearchType::SUB)
            {
                queryMoleculeSearch(session, bingo, parts, rdfMolecule, threads, fullSearchCounter, partSearchCounter);
            }
            else
            {
                moleculeSearch(type, bingo, parts, threads, fullSearchCounter, partSearchCounter, index, rdfMolecule);
            }
            ++index;
        }
    }
}

TEST(Bingo, DISABLED_Part)
{
    partCreate();
    partTest(SearchType::SIM);
    partTest(SearchType::SUB);
    partTest(SearchType::EXACT);
}
