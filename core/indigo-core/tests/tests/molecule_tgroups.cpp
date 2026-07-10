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

// Characterization tests for indigo::MoleculeTGroups (PtrPool<TGroup>).
//
// Rationale (task #3766): MoleculeTGroups::remove() -> PtrPool<TGroup>::remove()
// is a documented coverage gap (0 calls in the whole known test run). It is
// exactly the operation that frees a pool slot for reuse, so its behavior is
// the golden master the PtrReusablePool migration must preserve: index
// allocation, LIFO slot reuse, count/iteration after removal, and lookup by
// name no longer finding a removed template.

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <molecule/molecule_tgroups.h>

using namespace indigo;

namespace
{
    // Adds a template group with the given name and returns its pool index.
    // Both name and alias are set: findTGroup() compares against tgroup_alias
    // via strncmp(alias, name, alias.size()), and an empty alias (size 0) would
    // spuriously match any query (0-length strncmp == 0). Setting alias == name
    // sidesteps that pre-existing findTGroup quirk so the lookups below are
    // deterministic and characterize slot lifecycle, not the quirk.
    int addNamed(MoleculeTGroups& tg, const char* name)
    {
        int idx = tg.addTGroup();
        tg.getTGroup(idx).tgroup_name.readString(name, true);
        tg.getTGroup(idx).tgroup_alias.readString(name, true);
        return idx;
    }
} // namespace

TEST(MoleculeTGroupsContract, AddAssignsMonotonicIndicesAndCount)
{
    MoleculeTGroups tg;
    EXPECT_EQ(0, tg.getTGroupCount());
    EXPECT_EQ(0, addNamed(tg, "A"));
    EXPECT_EQ(1, addNamed(tg, "B"));
    EXPECT_EQ(2, addNamed(tg, "C"));
    EXPECT_EQ(3, tg.getTGroupCount());
}

TEST(MoleculeTGroupsContract, FindTGroupByName)
{
    MoleculeTGroups tg;
    int a = addNamed(tg, "ALPHA");
    int b = addNamed(tg, "BETA");
    EXPECT_EQ(a, tg.findTGroup("ALPHA"));
    EXPECT_EQ(b, tg.findTGroup("BETA"));
    EXPECT_EQ(-1, tg.findTGroup("MISSING"));
}

// The core golden-master: remove() frees a slot, the freed name is no longer
// found, count drops, and a subsequent addTGroup() reuses the freed index
// (LIFO), constructing a fresh (empty-name) template in that slot.
TEST(MoleculeTGroupsContract, RemoveFreesSlotAndAddReusesIndex)
{
    MoleculeTGroups tg;
    addNamed(tg, "A");      // 0
    int b = addNamed(tg, "B"); // 1
    addNamed(tg, "C");      // 2
    ASSERT_EQ(3, tg.getTGroupCount());

    tg.remove(b);
    EXPECT_EQ(2, tg.getTGroupCount());
    EXPECT_EQ(-1, tg.findTGroup("B")); // removed template no longer found
    EXPECT_EQ(0, tg.findTGroup("A"));  // survivors keep their indices
    EXPECT_EQ(2, tg.findTGroup("C"));

    int reused = tg.addTGroup(); // reuses freed slot 1 (LIFO)
    EXPECT_EQ(b, reused);
    EXPECT_EQ(0, tg.getTGroup(reused).tgroup_name.size()); // fresh, not stale "B"
    EXPECT_EQ(3, tg.getTGroupCount());
}

TEST(MoleculeTGroupsContract, IterationSkipsRemovedSlots)
{
    MoleculeTGroups tg;
    addNamed(tg, "A"); // 0
    addNamed(tg, "B"); // 1
    addNamed(tg, "C"); // 2
    tg.remove(1);

    std::vector<int> visited;
    for (int i = tg.begin(); i != tg.end(); i = tg.next(i))
        visited.push_back(i);

    ASSERT_EQ(2u, visited.size());
    EXPECT_EQ(0, visited[0]);
    EXPECT_EQ(2, visited[1]);
}

TEST(MoleculeTGroupsContract, RemoveOnFreedSlotThrows)
{
    MoleculeTGroups tg;
    int a = addNamed(tg, "A");
    tg.remove(a);
    EXPECT_THROW(tg.remove(a), Exception);
}

TEST(MoleculeTGroupsContract, ClearRemovesAll)
{
    MoleculeTGroups tg;
    addNamed(tg, "A");
    addNamed(tg, "B");
    tg.clear();
    EXPECT_EQ(0, tg.getTGroupCount());
    EXPECT_EQ(tg.begin(), tg.end());
}
