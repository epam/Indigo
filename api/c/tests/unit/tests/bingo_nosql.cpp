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

#include "base_cpp/os_thread_wrapper.h"
#include <base_cpp/exception.h>
#include <base_cpp/red_black.h>

#include <deque>
#include <iostream>
#include <vector>

#include "common.h"

using namespace indigo;

class BingoNosqlTest : public IndigoApiTest
{
};

struct BaseSubstructureMatcherResult : public OsCommandResult
{
    int sum = 0;
    void clear() override
    {
        sum = 0;
    }
};

struct BaseSubstructureMatcherCommand : public OsCommand
{
    int a = 0, b = 0;
    void clear() override
    {
        a = b = 0;
    }
    void execute(OsCommandResult& res) override
    {
        BaseSubstructureMatcherResult& r = static_cast<BaseSubstructureMatcherResult&>(res);
        r.sum = a + b;
    }
};

class BaseSubstructureMatcherDispatcher : public OsCommandDispatcher
{
public:
    BaseSubstructureMatcherDispatcher(const std::vector<std::pair<int, int>>& src, std::deque<int>& results)
        : OsCommandDispatcher(HANDLING_ORDER_SERIAL, true), data(src), pos(0), _results(results)
    {
    }

protected:
    OsCommand* _allocateCommand() override
    {
        return new BaseSubstructureMatcherCommand();
    }
    OsCommandResult* _allocateResult() override
    {
        return new BaseSubstructureMatcherResult();
    }

    bool _setupCommand(OsCommand& command) override
    {
        if (pos >= data.size())
            return false;
        auto& [a, b] = data[pos++];
        BaseSubstructureMatcherCommand& cmd = static_cast<BaseSubstructureMatcherCommand&>(command);
        cmd.a = a;
        cmd.b = b;
        return true;
    }

    void _handleResult(OsCommandResult& result) override
    {
        BaseSubstructureMatcherResult& r = static_cast<BaseSubstructureMatcherResult&>(result);
        std::cout << "result sum=" << r.sum << "\n";
        std::lock_guard<std::mutex> locker(_lock_for_exclusive_access);
        _results.push_back(r.sum);
    }

private:
    std::vector<std::pair<int, int>> data;
    size_t pos;
    std::deque<int>& _results;
    std::mutex _lock_for_exclusive_access;
};

TEST_F(BingoNosqlTest, test_dispatcher)
{
    std::vector<std::pair<int, int>> input = {{1, 2}, {10, 20}, {100, 200}};
    std::deque<int> results;
    std::mutex input_mutex;
    // std::sema;
    BaseSubstructureMatcherDispatcher dispatcher(input, results);

    dispatcher.run(4); // 4 worker threads
    printf("results count=%zd\n", results.size());
}

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
