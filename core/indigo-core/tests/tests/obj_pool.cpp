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

// Characterization tests for indigo::Pool<T>, indigo::ObjPool<T> and
// indigo::PtrPool<T>.
//
// Purpose: lock the current observable behavior of the three sparse-owning
// pool containers BEFORE they are replaced by PtrReusablePool<T> + the
// Resettable interface (task #3766, milestone 19). These tests capture the
// contract that the replacement MUST preserve: index allocation order, the
// LIFO slot-reuse semantics of the free list, throw-on-misuse behavior,
// iteration that skips freed holes, and object lifetime (construct on add,
// destruct/delete on remove/clear).
//
// Every test here is GREEN against the current Array<T>-backed implementation
// on master. Anything that later diverges between this implementation and the
// PtrReusablePool rewrite is a breaking-change candidate that must be either
// reconciled or explicitly documented in the migration notes. Behaviors that
// the rewrite is expected to *improve* (e.g. exception safety of growth,
// pointer stability across reuse) are intentionally NOT asserted as
// master-golden here — they belong to the new class's own unit suite per the
// task's acceptance criteria.

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <base_cpp/obj_pool.h>
#include <base_cpp/pool.h>
#include <base_cpp/ptr_pool.h>

#include <type_traits>

using namespace indigo;

namespace
{
    // Tracked: counts live instances so tests can verify that add()/remove()/
    // clear()/destruction run the ctor/dtor exactly as expected (no leaks, no
    // double destruction). Non-copyable to match the pools' ownership model.
    struct Tracked
    {
        static int s_live;
        static int s_constructed_total;

        int id;

        explicit Tracked(int v = 0) : id(v)
        {
            ++s_live;
            ++s_constructed_total;
        }
        Tracked(const Tracked&) = delete;
        Tracked& operator=(const Tracked&) = delete;
        ~Tracked()
        {
            --s_live;
            id = -1;
        }

        static void reset_counters()
        {
            s_live = 0;
            s_constructed_total = 0;
        }
    };

    int Tracked::s_live = 0;
    int Tracked::s_constructed_total = 0;

    // Fixture: resets counters at SetUp and asserts no live instances leaked
    // once the pool under test has gone out of scope at TearDown.
    class PoolLifetimeTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            Tracked::reset_counters();
        }
        void TearDown() override
        {
            EXPECT_EQ(0, Tracked::s_live) << "Tracked instances leaked across test boundary";
        }
    };
} // namespace

// =====================================================================
// Compile-time contract: none of the pools are copyable.
// =====================================================================

TEST(PoolTraits, NotCopyable)
{
    static_assert(!std::is_copy_constructible_v<Pool<int>>, "Pool must not be copy-constructible");
    static_assert(!std::is_copy_constructible_v<ObjPool<Tracked>>, "ObjPool must not be copy-constructible");
    static_assert(!std::is_copy_constructible_v<PtrPool<Tracked>>, "PtrPool must not be copy-constructible");
}

// =====================================================================
// Pool<T> — raw free-list allocator (no element ctor/dtor).
// =====================================================================

TEST(PoolContract, AddReturnsMonotonicIndicesFromZero)
{
    Pool<int> pool;
    EXPECT_EQ(0, pool.add());
    EXPECT_EQ(1, pool.add());
    EXPECT_EQ(2, pool.add());
    EXPECT_EQ(3, pool.size());
}

TEST(PoolContract, SizeTracksAddAndRemove)
{
    Pool<int> pool;
    EXPECT_EQ(0, pool.size());
    pool.add();
    pool.add();
    EXPECT_EQ(2, pool.size());
    pool.remove(0);
    EXPECT_EQ(1, pool.size());
    pool.remove(1);
    EXPECT_EQ(0, pool.size());
}

// The central contract to preserve: a freed slot is reused, and reuse is
// LIFO (the most recently removed index is handed out first).
TEST(PoolContract, RemoveThenAddReusesIndexLIFO)
{
    Pool<int> pool;
    ASSERT_EQ(0, pool.add());
    ASSERT_EQ(1, pool.add());
    ASSERT_EQ(2, pool.add());

    pool.remove(0);
    pool.remove(2);

    // LIFO: last removed (2) comes back before earlier removed (0).
    EXPECT_EQ(2, pool.add());
    EXPECT_EQ(0, pool.add());
    // Free list now empty -> growth resumes.
    EXPECT_EQ(3, pool.add());
}

// Reusing a freed slot must NOT grow the backing storage (end() is the high
// water mark = backing size, distinct from the live size()).
TEST(PoolContract, ReuseDoesNotGrowBacking)
{
    Pool<int> pool;
    pool.add(); // 0
    pool.add(); // 1
    pool.add(); // 2
    EXPECT_EQ(3, pool.end());

    pool.remove(1);
    EXPECT_EQ(3, pool.end()); // hole, but backing unchanged
    EXPECT_EQ(1, pool.add()); // reuse the hole
    EXPECT_EQ(3, pool.end()); // still no growth
}

TEST(PoolContract, DoubleRemoveThrows)
{
    Pool<int> pool;
    int i = pool.add();
    pool.remove(i);
    EXPECT_THROW(pool.remove(i), Exception);
}

TEST(PoolContract, AccessToRemovedElementThrows)
{
    Pool<int> pool;
    int i = pool.add();
    pool[i] = 42;
    EXPECT_EQ(42, pool[i]);
    pool.remove(i);
    EXPECT_THROW(pool.at(i), Exception);
}

TEST(PoolContract, HasElementReflectsSlotState)
{
    Pool<int> pool;
    int i = pool.add();
    EXPECT_TRUE(pool.hasElement(i));
    pool.remove(i);
    EXPECT_FALSE(pool.hasElement(i));
    int j = pool.add(); // reuses i
    EXPECT_EQ(i, j);
    EXPECT_TRUE(pool.hasElement(j));
}

TEST(PoolContract, IterationSkipsFreedHoles)
{
    Pool<int> pool;
    for (int k = 0; k < 5; k++)
        pool.add();
    pool.remove(1);
    pool.remove(3);

    std::vector<int> visited;
    for (int i = pool.begin(); i != pool.end(); i = pool.next(i))
        visited.push_back(i);

    ASSERT_EQ(3u, visited.size());
    EXPECT_EQ(0, visited[0]);
    EXPECT_EQ(2, visited[1]);
    EXPECT_EQ(4, visited[2]);
}

TEST(PoolContract, EndIsBackingSizeNotLiveSize)
{
    Pool<int> pool;
    pool.add();
    pool.add();
    pool.add();
    pool.remove(0);
    EXPECT_EQ(2, pool.size()); // live count
    EXPECT_EQ(3, pool.end());  // high water mark
}

// =====================================================================
// ObjPool<T> — placement-new on add, explicit dtor on remove/clear.
// =====================================================================

TEST_F(PoolLifetimeTest, ObjPoolConstructsOnAddDestructsOnRemove)
{
    ObjPool<Tracked> pool;
    int i = pool.add();
    EXPECT_EQ(1, Tracked::s_live);
    EXPECT_EQ(0, pool[i].id); // default-constructed
    pool.remove(i);
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PoolLifetimeTest, ObjPoolAddForwardsConstructorArgument)
{
    ObjPool<Tracked> pool;
    int v = 42;
    int i = pool.add(v);
    EXPECT_EQ(42, pool[i].id);
    EXPECT_EQ(1, Tracked::s_live);
    pool.remove(i);
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PoolLifetimeTest, ObjPoolClearDestructsAllLiveElements)
{
    ObjPool<Tracked> pool;
    pool.add();
    pool.add();
    pool.add();
    EXPECT_EQ(3, Tracked::s_live);
    pool.clear();
    EXPECT_EQ(0, Tracked::s_live);
    EXPECT_EQ(0, pool.size());
}

TEST_F(PoolLifetimeTest, ObjPoolClearHandlesHolesWithoutDoubleDestruct)
{
    ObjPool<Tracked> pool;
    pool.add(); // 0
    pool.add(); // 1
    pool.add(); // 2
    pool.remove(1);
    EXPECT_EQ(2, Tracked::s_live);
    pool.clear(); // must not re-destruct the already-freed hole
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PoolLifetimeTest, ObjPoolDestructorDestructsRemaining)
{
    {
        ObjPool<Tracked> pool;
        pool.add();
        pool.add();
        EXPECT_EQ(2, Tracked::s_live);
    } // pool destructor runs clear()
    EXPECT_EQ(0, Tracked::s_live);
}

// Reuse must construct a fresh object in the freed slot (placement new),
// not resurrect the previous element's state.
TEST_F(PoolLifetimeTest, ObjPoolReuseConstructsFreshInstance)
{
    ObjPool<Tracked> pool;
    int a = 7;
    int i = pool.add(a);
    ASSERT_EQ(7, pool[i].id);
    pool.remove(i);
    EXPECT_EQ(0, Tracked::s_live);

    int b = 99;
    int j = pool.add(b);
    EXPECT_EQ(i, j);          // same slot reused (LIFO)
    EXPECT_EQ(99, pool[j].id); // fresh construction, not stale 7
    EXPECT_EQ(1, Tracked::s_live);
    pool.remove(j);
}

TEST_F(PoolLifetimeTest, ObjPoolIterationAndIndexedAccess)
{
    ObjPool<Tracked> pool;
    int v0 = 10, v1 = 11, v2 = 12;
    pool.add(v0);
    pool.add(v1);
    pool.add(v2);
    pool.remove(1);

    std::vector<int> ids;
    for (int i = pool.begin(); i != pool.end(); i = pool.next(i))
        ids.push_back(pool[i].id);

    ASSERT_EQ(2u, ids.size());
    EXPECT_EQ(10, ids[0]);
    EXPECT_EQ(12, ids[1]);
    pool.clear();
}

// =====================================================================
// PtrPool<T> — owns raw pointers; delete on remove/clear.
// =====================================================================

TEST_F(PoolLifetimeTest, PtrPoolDeletesOwnedPointerOnRemove)
{
    PtrPool<Tracked> pool;
    int i = pool.add(new Tracked(5));
    EXPECT_EQ(1, Tracked::s_live);
    EXPECT_EQ(5, pool.ref(i).id);
    pool.remove(i);
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PoolLifetimeTest, PtrPoolClearDeletesAllOwnedPointers)
{
    PtrPool<Tracked> pool;
    pool.add(new Tracked(1));
    pool.add(new Tracked(2));
    pool.add(new Tracked(3));
    EXPECT_EQ(3, Tracked::s_live);
    pool.clear();
    EXPECT_EQ(0, Tracked::s_live);
    EXPECT_EQ(0, pool.size());
}

TEST_F(PoolLifetimeTest, PtrPoolDestructorDeletesRemaining)
{
    {
        PtrPool<Tracked> pool;
        pool.add(new Tracked(1));
        pool.add(new Tracked(2));
        EXPECT_EQ(2, Tracked::s_live);
    }
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PoolLifetimeTest, PtrPoolReusesIndexLIFO)
{
    PtrPool<Tracked> pool;
    int i0 = pool.add(new Tracked(0));
    int i1 = pool.add(new Tracked(1));
    ASSERT_EQ(0, i0);
    ASSERT_EQ(1, i1);
    pool.remove(i1);
    // Reused slot gets the newly added pointer.
    int i2 = pool.add(new Tracked(2));
    EXPECT_EQ(1, i2);
    EXPECT_EQ(2, pool.ref(i2).id);
    pool.clear();
}

// The pointee identity is stable: removing/reusing OTHER slots must not move
// the object referenced by a surviving slot (only the pointer table changes).
TEST_F(PoolLifetimeTest, PtrPoolPointeeIdentityStableAcrossOtherReuse)
{
    PtrPool<Tracked> pool;
    int keep = pool.add(new Tracked(100));
    Tracked* kept_addr = &pool.ref(keep);

    int tmp = pool.add(new Tracked(200));
    pool.remove(tmp);
    pool.add(new Tracked(300)); // reuses tmp's slot

    EXPECT_EQ(kept_addr, &pool.ref(keep)); // survivor object not moved
    EXPECT_EQ(100, pool.ref(keep).id);
    pool.clear();
}
