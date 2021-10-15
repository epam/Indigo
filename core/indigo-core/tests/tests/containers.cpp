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

#include <base_cpp/output.h>
#include <base_cpp/scanner.h>
#include <molecule/cmf_loader.h>
#include <molecule/cmf_saver.h>
#include <molecule/cml_saver.h>
#include <molecule/molecule_cdxml_saver.h>
#include <molecule/molecule_mass.h>
#include <molecule/molecule_substructure_matcher.h>
#include <molecule/molfile_loader.h>
#include <molecule/query_molecule.h>
#include <molecule/sdf_loader.h>
#include <molecule/smiles_loader.h>

#include "common.h"

using namespace indigo;

class IndigoCoreContainersTest : public IndigoCoreTest
{
};

namespace
{
   int sum(int s, int x)
   {
       if (x > 5)
       {
           return s;
       }

       QS_DEF(Array<int>, xs);
       xs.push(x);

       int top = xs[0];

       for (int i = 0; i < x; ++i)
       {
           sum(s, x + 1);
       }

       return s + (top - x);
   }
}

TEST_F(IndigoCoreContainersTest, test_qsdef)
{
   int res = sum(0, 0);
   ASSERT_EQ(res, 0);
}

TEST_F(IndigoCoreContainersTest, test_array)
{
   Array<int> array;
   const auto initial_size = 100;
   array.resize(initial_size);
   array.zerofill();
   ASSERT_EQ(array.size(), initial_size);
   array.clear();
   ASSERT_EQ(array.size(), 0);
   const auto final_size = 200;
   array.resize(final_size);
   array.fffill();
   ASSERT_EQ(array.size(), final_size);
   array.clear();
   ASSERT_EQ(array.size(), 0);
}

TEST_F(IndigoCoreContainersTest, test_red_black_map)
{
   RedBlackMap<int, int> map;
   map.insert(1, 2);
   map.insert(2, 3);
   ASSERT_EQ(map.size(), 2);
   map.clear();
   ASSERT_EQ(map.size(), 0);
}
