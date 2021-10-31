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

#include <array>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

#include <BingoNoSQL.h>
#include <IndigoException.h>
#include <IndigoSDFileIterator.h>
#include <IndigoSession.h>

#include "common.h"

using namespace indigo_cpp;

namespace
{
    void testCreate()
    {
        auto session_1 = IndigoSession::create();
        auto session_2 = IndigoSession::create();
        std::stringstream ss;
        ss << "test_" << std::this_thread::get_id();
        const auto bingo_1 = BingoMolecule::createDatabaseFile(session_1, ss.str() + "_1.db");
        const auto bingo_2 = BingoMolecule::createDatabaseFile(session_2, ss.str() + "_2.db");
    }

    void testInsert(BingoMolecule& bingo, const char* path)
    {
        for (const auto& m : bingo.session->iterateSDFile(dataPath(path)))
        {
            try
            {
                bingo.insertRecord(*m);
            }
            catch (const IndigoException& e)
            {
            }
        }
    }

    void testInsertDelete(BingoMolecule& bingo, const char* path)
    {
        for (const auto& m : bingo.session->iterateSDFile(dataPath(path)))
        {
            auto id = bingo.insertRecord(*m);
            bingo.deleteRecord(id);
        }
    }

    void testSearchSub(const BingoMolecule& bingo, const IndigoQueryMolecule& q)
    {
        auto counter = 0;
        for (const auto& t : bingo.searchSub(q))
        {
            ++counter;
        }
        EXPECT_GT(counter, 0);
    }

    void checkCount(const BingoMolecule& bingo, const size_t count, const char* substructure = "C")
    {
        auto counter = 0;
        for (const auto& m : bingo.searchSub(bingo.session->loadQueryMolecule(substructure)))
        {
            ++counter;
        }
        EXPECT_EQ(counter, count);
    }
}

TEST(BingoThreads, CreateSingleThread)
{
    for (auto i = 0; i < 16; i++)
    {
        testCreate();
    }
}

TEST(BingoThreads, CreateMultipleThreads)
{
    std::vector<std::thread> threads;
    threads.reserve(16);
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(testCreate);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BingoThreads, InsertSingleThread)
{
    auto session = IndigoSession::create();
    session->setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    for (auto i = 0; i < 16; i++)
    {
        testInsert(bingo, "molecules/basic/Compound_0000001_0000250.sdf.gz");
    }
    checkCount(bingo, 241 * 16);
}

TEST(BingoThreads, DISABLED_InsertSingleThreadPharmapendium)
{
    auto session = IndigoSession::create();
    session->setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    for (auto i = 0; i < 1; i++)
    {
        testInsert(bingo, "molecules/basic/pharmapendium.sdf.gz");
    }
    checkCount(bingo, 3029);
}

TEST(BingoThreads, InsertMultipleThreads)
{
    auto session = IndigoSession::create();
    session->setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoMolecule::createDatabaseFile(session, "BingoThreads_Insert.db");
    std::vector<std::thread> threads;
    threads.reserve(16);
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(testInsert, std::ref(bingo), "molecules/basic/Compound_0000001_0000250.sdf.gz");
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    checkCount(bingo, 241 * 16);
}

TEST(BingoThreads, InsertDeleteMultipleThreads)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    std::vector<std::thread> threads;
    threads.reserve(16);
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(testInsertDelete, std::ref(bingo), "molecules/basic/Compound_0000001_0000250.sdf.gz");
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    checkCount(bingo, 0);
}

TEST(BingoThreads, SearchSingleThread)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    testInsert(bingo, "molecules/basic/Compound_0000001_0000250.sdf.gz");
    const auto q = session->loadQueryMolecule("C1=CC=CC=C1");
    for (auto i = 0; i < 16; i++)
    {
        testSearchSub(bingo, q);
    }
}

TEST(BingoThreads, SearchMultipleThreads)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    testInsert(bingo, "molecules/basic/Compound_0000001_0000250.sdf.gz");
    std::vector<std::thread> threads;
    threads.reserve(16);
    const auto q = session->loadQueryMolecule("C1=CC=CC=C1");
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(testSearchSub, std::cref(bingo), std::cref(q));
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BingoThreads, DISABLED_Insert_Pubchem_1M)
{
    auto session = IndigoSession::create();
    session->setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoMolecule::createDatabaseFile(session, "Pubchem_1M.db");
    bingo.insertIterator(session->iterateSDFile(dataPath("molecules/basic/Compound_000000001_000500000.sdf.gz")));
    bingo.insertIterator(session->iterateSDFile(dataPath("molecules/basic/Compound_000500001_001000000.sdf.gz")));
}

TEST(BingoThreads, DISABLED_SearchSubSingleThread_Pubchem_1M)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::loadDatabaseFile(session, "Pubchem_1M.db");
    checkCount(bingo, 2531, "CN1C=NC2=C1C(=O)N(C(=O)N2C)C");
}

TEST(BingoThreads, DISABLED_SearchSubMultipleThreads_Pubchem_1M)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::loadDatabaseFile(session, "Pubchem_1M.db");
    std::vector<std::thread> threads;
    threads.reserve(1);
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(checkCount, std::cref(bingo), 2531, "CN1C=NC2=C1C(=O)N(C(=O)N2C)C");
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
