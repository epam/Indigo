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

// Characterization tests for indigo::RGroup (RGroup::fragments is a
// PtrPool<BaseMolecule>) with focus on RGroup::clear().
//
// Rationale (task #3766): RGroup::clear() is the only existing "reset an object
// in place" method in the affected surface and is the closest prototype of the
// future Resettable::reset() contract — yet it has 0 direct test calls. Locking
// its exact reset semantics (all scalar fields zeroed, occurrence emptied,
// fragments pool cleared and reusable) gives the golden master that the
// Resettable-based reset-on-remove reuse must reproduce.

#include <gtest/gtest.h>

#include <molecule/molecule.h>
#include <molecule/molecule_rgroups.h>

using namespace indigo;

TEST(RGroupContract, ClearResetsAllFields)
{
    RGroup rg;
    rg.if_then = 5;
    rg.rest_h = 3;
    rg.occurrence.push(42);
    rg.fragments.add(new Molecule());
    rg.fragments.add(new Molecule());
    ASSERT_EQ(2, rg.fragments.size());

    rg.clear();

    EXPECT_EQ(0, rg.if_then);
    EXPECT_EQ(0, rg.rest_h);
    EXPECT_EQ(0, rg.occurrence.size());
    EXPECT_EQ(0, rg.fragments.size());
}

// After clear() the RGroup (and its fragments pool) must be reusable: the pool
// restarts index allocation from 0.
TEST(RGroupContract, ReusableAfterClear)
{
    RGroup rg;
    rg.fragments.add(new Molecule());
    rg.clear();

    int idx = rg.fragments.add(new Molecule());
    EXPECT_EQ(0, idx);
    EXPECT_EQ(1, rg.fragments.size());
}
