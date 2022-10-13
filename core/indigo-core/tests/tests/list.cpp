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
#include <vector>

#include "base_cpp/list.h"

#include "common.h"

using namespace indigo;

class IndigoListTest : public IndigoCoreTest
{
};

TEST_F(IndigoListTest, test_default_constructor)
{
    List<int> l;
    ASSERT_EQ(l.size(), 0);
}

TEST_F(IndigoListTest, test_move_constructor)
{
    List<int> list_in;
    list_in.add();
    list_in.add();
    list_in.add();

    List<int> list_out(std::move(list_in));

    ASSERT_EQ(list_out.size(), 3);
    ASSERT_EQ(list_in.size(), 0);
}

TEST_F(IndigoListTest, test_move_assignment)
{
    List<int> list_in;
    list_in.add();
    list_in.add();
    list_in.add();

    List<int> list_out;
    list_out = std::move(list_in);

    ASSERT_EQ(list_out.size(), 3);
    ASSERT_EQ(list_in.size(), 0);
}

TEST_F(IndigoListTest, test_move_constructor_owning_pool_management)
{
    std::vector<List<int>> lists_out;
    {
        List<int> list_in;
        list_in.add(1);
        list_in.add(2);
        list_in.add(3);

        // move constructor is called for the object in vector lists_out[0]
        lists_out.emplace_back(std::move(list_in));

        // list_in is destructed here. The owning pool should remain alive and should belong to lists_out[0]
    }

    List<int>& list_out = lists_out[0];

    // ensure list items are available in the pool after list_in destruction
    ASSERT_EQ(list_out[0], 1);
    ASSERT_EQ(list_out[1], 2);
    ASSERT_EQ(list_out[2], 3);
}

TEST_F(IndigoListTest, test_move_assignment_owning_pool_management)
{
    List<int> list_out;
    {
        List<int> list_in;
        list_in.add(1);
        list_in.add(2);
        list_in.add(3);

        list_out = std::move(list_in);
        // list_in is destructed here. The owning pool should remain alive and should belong to list_out
    }

    // ensure list items are available in the pool after list_in destruction
    ASSERT_EQ(list_out[0], 1);
    ASSERT_EQ(list_out[1], 2);
    ASSERT_EQ(list_out[2], 3);
}

TEST_F(IndigoListTest, test_move_constructor_non_owning_pool)
{
    Pool<List<int>::Elem> pool;
    {
        std::vector<List<int>> lists_out;
        {
            List<int> list_in(pool);
            list_in.add();
            list_in.add();
            list_in.add();

            // move constructor is called for the object in vector lists_out[0]
            lists_out.emplace_back(std::move(list_in));

            // list_in is destructed here. Pool should contain 3 items, because it is referred by lists_out[0]
        }

        ASSERT_EQ(pool.size(), 3);
        ASSERT_EQ(lists_out[0].size(), 3);

        // lists_out is destructed here. Now pool should contain 0 items, because it is not referred by any list
    }
    ASSERT_EQ(pool.size(), 0);
}

TEST_F(IndigoListTest, test_move_assignment_non_owning_pool)
{
    Pool<List<int>::Elem> pool;
    {
        List<int> list_out;
        {
            List<int> list_in(pool);
            list_in.add(1);
            list_in.add(2);
            list_in.add(3);

            list_out = std::move(list_in);

            // list_in is destructed here. Pool should contain 3 items, because it is referred by list_out
        }

        ASSERT_EQ(pool.size(), 3);
        ASSERT_EQ(list_out.size(), 3);

        // list_out is destructed here. Now pool should contain 0 items, because it is not referred by any list
    }
    ASSERT_EQ(pool.size(), 0);
}
