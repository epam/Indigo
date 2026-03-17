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

TEST_F(BingoNosqlTest, test_search_sim)
{
    /*
    int db = bingoCreateDatabaseFile("molecules", "molecule", "");
    int iterator = indigoIterateSDFile("bingo_mols.sdf");
    for (int mol = indigoNext(iterator); mol != 0; mol = indigoNext(iterator))
    {
        try
        {
            bingoInsertRecordObj(db, mol);
        }
        catch (...)
        {
        }
    }
    bingoCloseDatabase(db); */
    int db = bingoLoadDatabaseFile("molecules", "");
    int qiterator = indigoIterateSDFile("rand_queries_small.sdf");
    int count = 0;
    for (int mol = indigoNext(qiterator); mol != 0; mol = indigoNext(qiterator))
    {
        const char* smiles = indigoSmiles(mol);
        int qmol = indigoLoadQueryMoleculeFromString(indigoRawData(mol));
        int res = bingoSearchSub(db, qmol, "");
        res = bingoSearchSim(db, mol, 0.9f, 1.0f, "tanimoto");
        res = bingoSearchSim(db, mol, 0.9f, 1.0f, "tversky 0.3 0.7");
        res = bingoSearchSim(db, mol, 0.9f, 1.0f, "euclid-sub");
        printf("count = %d\n", count);
        count++;
    }
}

TEST_F(BingoNosqlTest, test_subsearch_tau)
{
    indigoClearTautomerRules();
    indigoSetTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te");
    indigoSetTautomerRule(2, "0C", "N,O,P,S");
    indigoSetTautomerRule(3, "1C", "N,O");
    int db = bingoLoadDatabaseFile("epam_structures50_1", "");
    // int query = indigoLoadQueryMoleculeFromFile("epam.mol");
    int query = indigoLoadQueryMoleculeFromString("[Si-2](F)(F)(F)(F)F");
    // int sub_matcher = bingoSearchSub(db, query, "TAU INNER R*");
    int sub_matcher = bingoSearchSim(db, query, 0.9, 1, "tanimoto");
    int count = 0;
    int res = bingoGetObject(sub_matcher);
    while (bingoNext(sub_matcher))
    {
        count++;
        int id = bingoGetCurrentId(sub_matcher);
        printf("%d: id=%d %s\n", count, id, indigoSmiles(res));
    }

    bingoEndSearch(sub_matcher);
    bingoCloseDatabase(db);
    printf("%s", bingoProfilingGetStatistics(db));
}

TEST_F(BingoNosqlTest, test_enumerate_id)
{
    int db = bingoCreateDatabaseFile(::testing::UnitTest::GetInstance()->current_test_info()->name(), "molecule", "");
    int obj = indigoLoadQueryMoleculeFromString("C1CCNCC1");

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
