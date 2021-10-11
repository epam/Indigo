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
#include <random>
#include <sstream>
#include <thread>

#include <BingoNoSQL.h>
#include <IndigoSDFileIterator.h>
#include <IndigoSession.h>

#include "common.h"

using namespace indigo_cpp;

namespace
{
    //    constexpr const std::array<const char*, 6> choices = {"C", "CC", "CCC", "CCCC", "CCCCC", "CCCCCC"};
    //    thread_local std::random_device rd;
    //    thread_local std::mt19937 rng(rd());
    //    thread_local std::uniform_int_distribution<int> uni(0, 5);
    //
    //    std::string randomSmiles()
    //    {
    //        return choices.at(uni(rng));
    //    }

    void testCreate()
    {
        auto session_1 = IndigoSession::create();
        auto session_2 = IndigoSession::create();
        std::stringstream ss;
        ss << "test_" << std::this_thread::get_id();
        const auto bingo_1 = BingoMolecule::createDatabaseFile(session_1, ss.str() + "_1.db");
        const auto bingo_2 = BingoMolecule::createDatabaseFile(session_2, ss.str() + "_2.db");
    }

    void testInsert(BingoMolecule& bingo)
    {
        for (const auto& m : bingo.session->iterateSDFile(dataPath("molecules/basic/zinc-slice.sdf.gz")))
        {
            bingo.insertRecord(*m);
        }
        // TODO: add check
    }

    void testInsertDelete(BingoMolecule& bingo)
    {
        for (const auto& m : bingo.session->iterateSDFile(dataPath("molecules/basic/zinc-slice.sdf.gz")))
        {
            auto id = bingo.insertRecord(*m);
            bingo.deleteRecord(id);
        }
        // TODO: add check
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
    for (auto i = 0; i < 1; i++)
    {
        testInsert(bingo);
    }
}

TEST(BingoThreads, Insert)
{
    auto session = IndigoSession::create();
    session->setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    std::vector<std::thread> threads;
    threads.reserve(16);
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(testInsert, std::ref(bingo));
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BingoThreads, InsertDelete)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    std::vector<std::thread> threads;
    threads.reserve(16);
    for (auto i = 0; i < 16; i++)
    {
        threads.emplace_back(testInsertDelete, std::ref(bingo));
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
