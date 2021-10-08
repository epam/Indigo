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
#include <IndigoSession.h>
#include <BingoNoSQL.h>

using namespace indigo_cpp;

namespace
{
    constexpr const std::array<const char*, 6> choices = {"C", "CC", "CCC", "CCCC", "CCCCC", "CCCCCC"};
    thread_local std::random_device rd;
    thread_local std::mt19937 rng(rd());
    thread_local std::uniform_int_distribution<int> uni(0, 5);

    std::string randomSmiles()
    {
        return choices.at(uni(rng));
    }

    void testCreate()
    {
        const auto& smiles = randomSmiles();
        const auto& session_1 = IndigoSession();
        const auto& session_2 = IndigoSession();
        const auto& m_1 = session_1.loadMolecule(smiles);
        const auto& m_2 = session_2.loadMolecule(smiles);
        std::stringstream ss;
        ss << "test_" << std::this_thread::get_id();
        const auto bingo_1 = BingoNoSQL::createDatabaseFile(session_1, ss.str() + "_1.db", BingoNoSqlDataBaseType::MOLECULE, "");
        const auto bingo_2 = BingoNoSQL::createDatabaseFile(session_2, ss.str() + "_2.db", BingoNoSqlDataBaseType::MOLECULE, "");
    }

    void testInsert(const BingoNoSQL& bingo)
    {
        for (auto i = 0; i < 100; i++)
        {
            bingo.insertRecord(bingo.indigo.loadMolecule(randomSmiles()));
        }
    }

    void testInsertDelete(const BingoNoSQL& bingo)
    {
        for (auto i = 0; i < 100; i++)
        {
            auto id = bingo.insertRecord(bingo.indigo.loadMolecule(randomSmiles()));
            bingo.deleteRecord(id);
        }
    }
}

TEST(BingoThreads, CreateSingleThread)
{
    for (auto i = 0; i < 10; i++)
    {
        testCreate();
    }
}

TEST(BingoThreads, CreatуMultipleThreads)
{
    std::vector<std::thread> threads;
    threads.reserve(10);
    for (auto i = 0; i < 10; i++)
    {
        threads.emplace_back(testCreate);
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BingoThreads, Insert)
{
    const auto& session = IndigoSession();
    auto bingo = BingoNoSQL::createDatabaseFile(session, "test.db", BingoNoSqlDataBaseType::MOLECULE, "");
    std::vector<std::thread> threads;
    threads.reserve(10);
    for (auto i = 0; i < 10; i++)
    {
        threads.emplace_back(testInsert, std::cref(bingo));
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}

TEST(BingoThreads, InsertDelete)
{
    const auto& session = IndigoSession();
    auto bingo = BingoNoSQL::createDatabaseFile(session, "test.db", BingoNoSqlDataBaseType::MOLECULE, "");
    std::vector<std::thread> threads;
    threads.reserve(10);
    for (auto i = 0; i < 10; i++)
    {
        threads.emplace_back(testInsertDelete, std::cref(bingo));
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
}
