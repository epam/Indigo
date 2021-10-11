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

#include <BingoNoSQL.h>
#include <IndigoSDFileIterator.h>
#include <IndigoSession.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Bingo, Create)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
}

TEST(Bingo, CreateClose)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    bingo.close();
}

TEST(Bingo, InsertSearchSubDelete)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    auto insert_counter = 0;
    for (const auto& item : {"C", "CC", "CCC"})
    {
        bingo.insertRecord(session->loadMolecule(item));
        ++insert_counter;
    }
    EXPECT_EQ(insert_counter, 3);

    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(session->loadQueryMolecule("C")))
        {
            ++search_counter;
        }
        EXPECT_EQ(search_counter, 3);
    }

    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(session->loadQueryMolecule("CC")))
        {
            ++search_counter;
        }
        EXPECT_EQ(search_counter, 2);
    }

    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(session->loadQueryMolecule("CO")))
        {
            ++search_counter;
        }
        EXPECT_EQ(search_counter, 0);
    }

    {
        for (int i = 0; i < 3; i++)
        {
            bingo.deleteRecord(i);
        }
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(session->loadQueryMolecule("C")))
        {
            ++search_counter;
        }
        EXPECT_EQ(search_counter, 0);
    }
}

TEST(Bingo, SearchSim)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");

    for (const auto& item : {"C1=CC=CC=C1", "C1=CN=CC=C1"})
    {
        bingo.insertRecord(session->loadMolecule(item));
    }
    const auto m = session->loadMolecule("C1=CC=CC=C1");

    {
        auto counter = 0;
        for (const auto& result : bingo.searchSim(m, 0.4))
        {
            ++counter;
        }
        EXPECT_EQ(counter, 1);
    }
    {
        auto counter = 0;
        for (const auto& result : bingo.searchSim(m, 0.3))
        {
            ++counter;
        }
        EXPECT_EQ(counter, 2);
    }
}
