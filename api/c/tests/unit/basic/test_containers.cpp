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

#include <base_cpp/array.h>
#include <base_cpp/red_black.h>

using namespace indigo;

TEST(IndigoContainersTest, test_array)
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

TEST(IndigoContainersTest, test_red_black)
{
    RedBlackMap<int, int> map;
    map.insert(1, 2);
    map.insert(2,3);
    ASSERT_EQ(map.size(), 2);
    map.clear();
    ASSERT_EQ(map.size(), 0);
}
