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

#include <IndigoMolecule.h>
#include <IndigoSDFileIterator.h>
#include <IndigoSession.h>
#include <BingoNoSQL.h>

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
        auto session_1 = IndigoSession();
        auto session_2 = IndigoSession();
        std::stringstream ss;
        ss << "test_" << std::this_thread::get_id();
        const auto bingo_1 = BingoNoSQL::createDatabaseFile(session_1, ss.str() + "_1.db", BingoNoSqlDataBaseType::MOLECULE, "");
        const auto bingo_2 = BingoNoSQL::createDatabaseFile(session_2, ss.str() + "_2.db", BingoNoSqlDataBaseType::MOLECULE, "");
    }

    void testInsert(BingoNoSQL& bingo)
    {
        for (const auto& m: bingo.indigo.iterateSDFile(dataPath("molecules/basic/zinc-slice.sdf.gz")))
        {
            bingo.insertRecord(*m);
        }
    }

    void testInsertDelete(BingoNoSQL& bingo)
    {
        for (const auto& m: bingo.indigo.iterateSDFile(dataPath("molecules/basic/zinc-slice.sdf.gz")))
        {
            auto id = bingo.insertRecord(*m);
            bingo.deleteRecord(id);
        }
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
    auto session = IndigoSession();
    session.setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoNoSQL::createDatabaseFile(session, "test.db", BingoNoSqlDataBaseType::MOLECULE, "");
    for (auto i = 0; i < 16; i++)
    {
        testInsert(bingo);
    }
}

TEST(BingoThreads, Insert)
{
    auto session = IndigoSession();
    session.setOption("ignore-stereochemistry-errors", true);
    auto bingo = BingoNoSQL::createDatabaseFile(session, "test.db", BingoNoSqlDataBaseType::MOLECULE, "");
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
    auto session = IndigoSession();
    auto bingo = BingoNoSQL::createDatabaseFile(session, "test.db", BingoNoSqlDataBaseType::MOLECULE, "");
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
