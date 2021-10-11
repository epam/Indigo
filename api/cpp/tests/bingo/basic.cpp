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

TEST(Bingo, InsertSearchDelete)
{
    auto session = IndigoSession::create();
    auto bingo = BingoMolecule::createDatabaseFile(session, "test.db");
    auto insert_counter = 0;
    for (const auto& m : bingo.session->iterateSDFile(dataPath("molecules/basic/zinc-slice.sdf.gz")))
    {
        bingo.insertRecord(*m);
        ++insert_counter;
    }
    EXPECT_EQ(insert_counter, 992);

    // Search common
    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(session->loadQueryMolecule("C")))
        {
            ++search_counter;
        }
        EXPECT_EQ(search_counter, 992);
    }

    // Search something less common
    {
        auto search_counter = 0;
        for (const auto& result : bingo.searchSub(session->loadQueryMolecule("C1=CN=CC=C1")))
        {
            ++search_counter;
        }
        EXPECT_LT(search_counter, 992);
    }

    {
        for (int i = 0; i < 992; i++)
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
