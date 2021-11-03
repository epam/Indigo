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

#include <functional>

#include <gtest/gtest.h>

#include <bingo-nosql.h>
#include <indigo.h>

#include <base_cpp/exception.h>

#include "common.h"

using namespace indigo;

class BingoNosqlTest : public IndigoApiTest
{
};

TEST_F(BingoNosqlTest, test_enumerate_id)
{
    int db = bingoCreateDatabaseFile(::testing::UnitTest::GetInstance()->current_test_info()->name(), "molecule", "");
    int obj = indigoLoadMoleculeFromString("C1CCNCC1");
    bingoInsertRecordObj(db, obj);
    bingoInsertRecordObj(db, obj);
    bingoInsertRecordObj(db, obj);

    int count = 0;
    int e = bingoEnumerateId(db);
    while (bingoNext(e))
    {
        count++;
    }

    bingoEndSearch(e);
    bingoCloseDatabase(db);

    EXPECT_EQ(count, 3);
}

TEST_F(BingoNosqlTest, multiple_instances_same_name)
{
    std::vector<int> db_ids;
    EXPECT_THROW({
        for (int i = 0; i < 100; i++)
        {
            db_ids.emplace_back(bingoCreateDatabaseFile(::testing::UnitTest::GetInstance()->current_test_info()->name(), "molecule", ""));
        }
    }, Exception);
    for (const auto& db_id : db_ids)
    {
        bingoCloseDatabase(db_id);
    }
}
