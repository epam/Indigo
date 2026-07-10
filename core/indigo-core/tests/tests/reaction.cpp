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

// Characterization tests for indigo::BaseReaction molecule membership
// (PtrPool<BaseMolecule> _allMolecules).
//
// Rationale (task #3766): BaseReaction::remove() -> PtrPool<BaseMolecule>::remove()
// is the single most under-covered operation in the affected surface (0 calls in
// the whole known test run) and the largest practical consumer of PtrPool. It is
// the operation that frees a pool slot for reuse, so its observable contract —
// side-count bookkeeping, index allocation, LIFO slot reuse, per-side iteration
// skipping removed molecules — is the golden master the PtrReusablePool migration
// must preserve.

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <reaction/reaction.h>

using namespace indigo;

TEST(BaseReactionContract, AddAssignsIndicesAndSideCounts)
{
    Reaction r;
    EXPECT_EQ(0, r.addReactant());
    EXPECT_EQ(1, r.addReactant());
    EXPECT_EQ(2, r.addProduct());
    EXPECT_EQ(3, r.count());
    EXPECT_EQ(2, r.reactantsCount());
    EXPECT_EQ(1, r.productsCount());
}

// Core golden-master: remove() drops the side count and frees the slot; the
// next add of the same side reuses the freed index (LIFO) with the correct
// side tag.
TEST(BaseReactionContract, RemoveFreesSlotAndReusesIndexLIFO)
{
    Reaction r;
    r.addReactant();          // 0
    int r1 = r.addReactant(); // 1
    r.addProduct();           // 2
    ASSERT_EQ(3, r.count());

    r.remove(r1);
    EXPECT_EQ(2, r.count());
    EXPECT_EQ(1, r.reactantsCount());
    EXPECT_EQ(1, r.productsCount());

    int reused = r.addReactant();
    EXPECT_EQ(r1, reused); // LIFO slot reuse
    EXPECT_EQ(BaseReaction::REACTANT, r.getSideType(reused));
    EXPECT_EQ(2, r.reactantsCount());
    EXPECT_EQ(3, r.count());
}

TEST(BaseReactionContract, ReactantIterationSkipsRemoved)
{
    Reaction r;
    r.addReactant(); // 0
    r.addReactant(); // 1
    r.addReactant(); // 2
    r.remove(1);

    std::vector<int> visited;
    for (int i = r.reactantBegin(); i != r.reactantEnd(); i = r.reactantNext(i))
        visited.push_back(i);

    ASSERT_EQ(2u, visited.size());
    EXPECT_EQ(0, visited[0]);
    EXPECT_EQ(2, visited[1]);
}

TEST(BaseReactionContract, RemoveOnFreedSlotThrows)
{
    Reaction r;
    int r0 = r.addReactant();
    r.remove(r0);
    EXPECT_THROW(r.remove(r0), Exception);
}
