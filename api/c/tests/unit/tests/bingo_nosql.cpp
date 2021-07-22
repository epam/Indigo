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

#include <base_cpp/output.h>
#include <base_cpp/profiling.h>
#include <base_cpp/scanner.h>
#include <molecule/cmf_loader.h>
#include <molecule/molecule.h>
#include <molecule/molecule_substructure_matcher.h>
#include <molecule/molfile_loader.h>
#include <molecule/sdf_loader.h>
#include <molecule/smiles_loader.h>

#include <bingo-nosql.h>
#include <indigo.h>

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
        ASSERT_EQ(count, bingoGetCurrentId(e));
        count++;
    }

    bingoEndSearch(e);
    bingoCloseDatabase(db);

    EXPECT_EQ(count, 3);
}
