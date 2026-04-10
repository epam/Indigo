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

#include <deque>
#include <iostream>
#include <list>
#include <vector>

#include "common.h"

using namespace indigo;

class BingoNosqlTest : public IndigoApiTest
{
};

TEST_F(BingoNosqlTest, test_subsearch_tau)
{
    indigoClearTautomerRules();
    indigoSetTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te");
    indigoSetTautomerRule(2, "0C", "N,O,P,S");
    indigoSetTautomerRule(3, "1C", "N,O");
    // create and fill Bingo DB
    constexpr int MAX_ITEMS = 20000;
    int db = bingoCreateDatabaseFile(::testing::UnitTest::GetInstance()->current_test_info()->name(), "molecule", "");
    int item, count = 0, iter = indigoIterateSmilesFile(dataPath("molecules/basic/sample_100000.smi").c_str());
    while (item = indigoNext(iter))
    {
        bingoInsertRecordObj(db, item);
        if (count++ >= MAX_ITEMS)
            break;
    }
    indigoSetOptionInt("bingonosql-sub-search-thread-count", -1);
    int query = indigoLoadQueryMoleculeFromString("C/C(/O)=C/CC");
    int sub_matcher = bingoSearchSub(db, query, "TAU INNER R*");
    // auto start = std::chrono::high_resolution_clock::now();
    int res = bingoGetObject(sub_matcher);
    std::list<int> results;
    while (bingoNext(sub_matcher))
    {
        int id = bingoGetCurrentId(sub_matcher);
        results.emplace_back(id);
    }
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_EQ(results.size(), 57);
    // printf("Total %d results in %zdms.\n", count, duration.count());
    // printf("%s", bingoProfilingGetStatistics(db));
    bingoEndSearch(sub_matcher);
    bingoCloseDatabase(db);
}

TEST_F(BingoNosqlTest, test_enumerate_id)
{
    int db = bingoCreateDatabaseFile(::testing::UnitTest::GetInstance()->current_test_info()->name(), "molecule", "");
    int obj = indigoLoadMoleculeFromString("C1CCNCC1");

    // checking next() of empty enumerator
    int e_empty = bingoEnumerateId(db);
    EXPECT_FALSE(bingoNext(e_empty));
    EXPECT_ANY_THROW(bingoGetCurrentId(e_empty));
    bingoEndSearch(e_empty);

    // Main scenario: 3 elements in enumerator
    bingoInsertRecordObj(db, obj);
    bingoInsertRecordObj(db, obj);
    bingoInsertRecordObj(db, obj);

    int count = 0;
    int e = bingoEnumerateId(db);
    // unfolding while (bingoNext(e))
    EXPECT_TRUE(bingoNext(e));
    EXPECT_NO_THROW(bingoGetCurrentId(e));
    count++;
    EXPECT_TRUE(bingoNext(e));
    EXPECT_NO_THROW(bingoGetCurrentId(e));
    count++;
    EXPECT_TRUE(bingoNext(e));
    EXPECT_NO_THROW(bingoGetCurrentId(e));
    count++;
    EXPECT_FALSE(bingoNext(e));

    bingoEndSearch(e);
    bingoCloseDatabase(db);

    EXPECT_EQ(count, 3);
}

TEST_F(BingoNosqlTest, multiple_instances_same_name)
{
    std::vector<int> db_ids;
    EXPECT_THROW(
        {
            for (int i = 0; i < 100; i++)
            {
                db_ids.emplace_back(bingoCreateDatabaseFile(::testing::UnitTest::GetInstance()->current_test_info()->name(), "molecule", ""));
            }
        },
        Exception);
    for (const auto& db_id : db_ids)
    {
        bingoCloseDatabase(db_id);
    }
}
