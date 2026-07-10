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

// Characterization tests for indigo::MoleculeSGroups (PtrPool<SGroup>) and
// indigo::Superatom::attachment_points (ObjPool<_AttachmentPoint>).
//
// Rationale (task #3766): MoleculeSGroups::remove() -> PtrPool<SGroup>::remove()
// (molecule_sgroups.cpp:177) and Superatom::attachment_points.remove()
// (base_molecule_templates.cpp:1251) are slot-reuse paths only exercised
// indirectly by format loaders. These tests pin the reuse contract directly:
// index allocation, LIFO reuse, per-type counting, iteration over holes, and
// (for the ObjPool case) that a non-trivial element type with an Array<char>
// member round-trips construct/destruct through the pool.

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <molecule/molecule_sgroups.h>

using namespace indigo;

// ---- MoleculeSGroups (PtrPool<SGroup>) -----------------------------------

TEST(MoleculeSGroupsContract, AddAssignsIndicesAndCountsByType)
{
    MoleculeSGroups sg;
    EXPECT_EQ(0, sg.getSGroupCount());
    int a = sg.addSGroup(SGroup::SG_TYPE_GEN);
    int b = sg.addSGroup(SGroup::SG_TYPE_DAT);
    int c = sg.addSGroup(SGroup::SG_TYPE_GEN);
    EXPECT_EQ(0, a);
    EXPECT_EQ(1, b);
    EXPECT_EQ(2, c);
    EXPECT_EQ(3, sg.getSGroupCount());
    EXPECT_EQ(2, sg.getSGroupCount(SGroup::SG_TYPE_GEN));
    EXPECT_EQ(1, sg.getSGroupCount(SGroup::SG_TYPE_DAT));
}

// Core golden-master: remove() frees the slot; hasSGroup reflects it; a later
// addSGroup reuses the freed index (LIFO) with a freshly constructed SGroup
// (no stale atoms from the removed one).
TEST(MoleculeSGroupsContract, RemoveFreesSlotAndAddReusesFreshInstance)
{
    MoleculeSGroups sg;
    sg.addSGroup(SGroup::SG_TYPE_GEN);       // 0
    int b = sg.addSGroup(SGroup::SG_TYPE_GEN); // 1
    sg.addSGroup(SGroup::SG_TYPE_GEN);       // 2
    sg.getSGroup(b).atoms.push(42);
    ASSERT_EQ(3, sg.getSGroupCount());

    sg.remove(b);
    EXPECT_EQ(2, sg.getSGroupCount());
    EXPECT_FALSE(sg.hasSGroup(b));

    int reused = sg.addSGroup(SGroup::SG_TYPE_GEN);
    EXPECT_EQ(b, reused);                       // LIFO slot reuse
    EXPECT_TRUE(sg.hasSGroup(reused));
    EXPECT_EQ(0, sg.getSGroup(reused).atoms.size()); // fresh, not stale {42}
    EXPECT_EQ(3, sg.getSGroupCount());
}

TEST(MoleculeSGroupsContract, IterationSkipsRemovedSlots)
{
    MoleculeSGroups sg;
    sg.addSGroup(SGroup::SG_TYPE_GEN); // 0
    sg.addSGroup(SGroup::SG_TYPE_GEN); // 1
    sg.addSGroup(SGroup::SG_TYPE_GEN); // 2
    sg.remove(1);

    std::vector<int> visited;
    for (int i = sg.begin(); i != sg.end(); i = sg.next(i))
        visited.push_back(i);

    ASSERT_EQ(2u, visited.size());
    EXPECT_EQ(0, visited[0]);
    EXPECT_EQ(2, visited[1]);
}

TEST(MoleculeSGroupsContract, RemoveOnFreedSlotThrows)
{
    MoleculeSGroups sg;
    int a = sg.addSGroup(SGroup::SG_TYPE_GEN);
    sg.remove(a);
    EXPECT_THROW(sg.remove(a), Exception);
}

TEST(MoleculeSGroupsContract, ClearRemovesAll)
{
    MoleculeSGroups sg;
    sg.addSGroup(SGroup::SG_TYPE_GEN);
    sg.addSGroup(SGroup::SG_TYPE_DAT);
    sg.clear();
    EXPECT_EQ(0, sg.getSGroupCount());
    EXPECT_EQ(sg.begin(), sg.end());
}

// ---- Superatom::attachment_points (ObjPool<_AttachmentPoint>) -------------

TEST(SuperatomAttachmentPointsContract, AddRemoveReuseWithNonTrivialElement)
{
    Superatom sup;
    int a0 = 10, a1 = 11, a2 = 12;
    EXPECT_EQ(0, sup.attachment_points.add(a0));
    int i1 = sup.attachment_points.add(a1);
    EXPECT_EQ(1, i1);
    EXPECT_EQ(2, sup.attachment_points.add(a2));
    EXPECT_EQ(3, sup.attachment_points.size());

    // _AttachmentPoint(int) sets aidx and pushes one apid entry.
    EXPECT_EQ(11, sup.attachment_points[i1].aidx);
    EXPECT_EQ(1, sup.attachment_points[i1].apid.size());

    sup.attachment_points.remove(i1);
    EXPECT_EQ(2, sup.attachment_points.size());
    EXPECT_FALSE(sup.attachment_points.hasElement(i1));

    int reused = sup.attachment_points.add(a1);
    EXPECT_EQ(i1, reused);                          // LIFO reuse
    EXPECT_EQ(11, sup.attachment_points[reused].aidx);
    EXPECT_EQ(1, sup.attachment_points[reused].apid.size()); // fresh element
}
