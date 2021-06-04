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
