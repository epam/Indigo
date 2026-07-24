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

// Unit tests for indigo::PtrReusablePool<T> and the indigo::Reusable
// interface (milestone 19, RFC §4.1/§4.4, task #3766 acceptance criteria).
//
// These assert the contract of the NEW container: the is_reusable_v gate,
// LIFO index reuse (preserving the legacy Pool<T> observable order),
// reuse-on-remove (object retained, not destroyed), pointer stability of
// at()/operator[] across growth, exception-safe growth (a throwing ctor leaves the pool
// consistent), whole-pool reuse(), and purge().

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <base_cpp/ptr_reusable_pool.h>
#include <base_cpp/reusable.h>

#include <stdexcept>
#include <type_traits>

using namespace indigo;

namespace
{
    // Reusable element that tracks live count, reset count and construction
    // count so lifetime/reset semantics can be asserted.
    struct Elem : public Reusable
    {
        static int s_live;
        static int s_resets;
        static int s_ctor;

        int id;
        std::vector<int> buf; // to exercise the never-shrink reuse contract

        Elem() : id(0)
        {
            ++s_live;
            ++s_ctor;
        }
        ~Elem() override
        {
            --s_live;
        }
        void reuse() override
        {
            id = 0;
            buf.clear(); // clears length, retains capacity (never-shrink)
            ++s_resets;
        }
        static void reset_counters()
        {
            s_live = 0;
            s_resets = 0;
            s_ctor = 0;
        }
    };
    int Elem::s_live = 0;
    int Elem::s_resets = 0;
    int Elem::s_ctor = 0;

    // Reusable element whose constructor throws when a countdown reaches zero,
    // to exercise exception safety of growth.
    struct ThrowingElem : public Reusable
    {
        static int s_live;
        static int s_countdown; // if > 0, decremented per ctor; throws when it hits 0

        ThrowingElem()
        {
            if (s_countdown > 0 && --s_countdown == 0)
                throw std::runtime_error("ThrowingElem: intentional ctor throw");
            ++s_live;
        }
        ~ThrowingElem() override
        {
            --s_live;
        }
        void reuse() override
        {
        }
        static void configure(int countdown)
        {
            s_live = 0;
            s_countdown = countdown;
        }
    };
    int ThrowingElem::s_live = 0;
    int ThrowingElem::s_countdown = 0;

    // Reusable element whose reuse() can be made to throw, to verify the pool
    // keeps consistent bookkeeping if reuse() throws on the reuse path.
    struct ThrowOnResetElem : public Reusable
    {
        static int s_live;
        static bool s_throw_on_reset;

        ThrowOnResetElem()
        {
            ++s_live;
        }
        ~ThrowOnResetElem() override
        {
            --s_live;
        }
        void reuse() override
        {
            if (s_throw_on_reset)
                throw std::runtime_error("ThrowOnResetElem: intentional reset throw");
        }
        static void configure()
        {
            s_live = 0;
            s_throw_on_reset = false;
        }
    };
    int ThrowOnResetElem::s_live = 0;
    bool ThrowOnResetElem::s_throw_on_reset = false;

    class PtrReusablePoolTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            Elem::reset_counters();
        }
        void TearDown() override
        {
            EXPECT_EQ(0, Elem::s_live) << "Elem instances leaked across test boundary";
        }
    };
} // namespace

// =====================================================================
// Reusable gate (RFC §4.4 acceptance: is_reusable_v).
// =====================================================================

TEST(ReusableTrait, GateAcceptsReusableRejectsPrimitive)
{
    static_assert(is_reusable_v<Elem>, "Elem implements Reusable");
    static_assert(!is_reusable_v<int>, "int is not Reusable (carve-out)");
    // The pool is itself Reusable, so pools are nestable.
    static_assert(is_reusable_v<PtrReusablePool<Elem>>, "the pool is Reusable");
    SUCCEED();
}

// reuseTypeId() — the slot-type identity for the heterogeneous mode (per-type
// free lists). The default implementation must reflect the DYNAMIC type, be
// stable across reuse(), and never depend on mutable state.
TEST(ReusableTrait, GetTypeReflectsDynamicType)
{
    Elem::reset_counters();
    Elem a, b;
    ThrowingElem::configure(0);
    ThrowingElem t;

    // Same class -> same identity; different classes -> different identities.
    EXPECT_EQ(a.reuseTypeId(), b.reuseTypeId());
    EXPECT_NE(a.reuseTypeId(), t.reuseTypeId());

    // The identity is the DYNAMIC type even through a base reference — the
    // property the per-type free lists rely on.
    const Reusable& base_ref = a;
    EXPECT_EQ(base_ref.reuseTypeId(), std::type_index(typeid(Elem)));

    // Stable across reuse(): identity is not derived from mutable state.
    const auto before = a.reuseTypeId();
    a.reuse();
    EXPECT_EQ(before, a.reuseTypeId());
}

// Negative-compile contract (documented, cannot be a passing runtime test):
//  * PtrReusablePool<int> fails: static_assert(is_reusable_v<T>) is false.
//  * A class that inherits Reusable but does not override reuse() is abstract,
//    so PtrReusablePool<That>::push() (std::make_unique<That>) fails to compile.
// Uncomment either to verify the compile-time rejection manually.
// TEST(ReusableTrait, NegativeCompile) { PtrReusablePool<int> p; (void)p; }

// =====================================================================
// PtrReusablePool<T> — allocation and reuse.
// =====================================================================

TEST_F(PtrReusablePoolTest, PushReturnsMonotonicIndicesFromZero)
{
    PtrReusablePool<Elem> pool;
    EXPECT_EQ(0, pool.push());
    EXPECT_EQ(1, pool.push());
    EXPECT_EQ(2, pool.push());
    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(3, pool.capacity());
}

// Reuse is LIFO and does not grow the spine — matches legacy Pool<T> order.
TEST_F(PtrReusablePoolTest, RemoveThenPushReusesIndexLIFO)
{
    PtrReusablePool<Elem> pool;
    pool.push(); // 0
    pool.push(); // 1
    pool.push(); // 2

    pool.remove(0);
    pool.remove(2);
    EXPECT_EQ(3, pool.capacity()); // holes, no growth

    EXPECT_EQ(2, pool.push()); // LIFO: last freed first
    EXPECT_EQ(0, pool.push());
    EXPECT_EQ(3, pool.capacity()); // reused, still no growth
    EXPECT_EQ(3, pool.push());     // free list empty -> grow
    EXPECT_EQ(4, pool.capacity());
}

// Reset-on-remove: the object is NOT destroyed on remove; it is reset in place
// and retained for reuse. This is the deliberate change from PtrPool (delete).
TEST_F(PtrReusablePoolTest, RemoveResetsButRetainsObject)
{
    PtrReusablePool<Elem> pool;
    pool.push();
    pool.push();
    pool.push();
    ASSERT_EQ(3, Elem::s_live);

    const int resets_before = Elem::s_resets;
    pool.remove(1);

    EXPECT_EQ(3, Elem::s_live);                 // object retained, not destroyed
    EXPECT_EQ(resets_before + 1, Elem::s_resets); // reuse() was called
    EXPECT_EQ(2, pool.size());
    EXPECT_FALSE(pool.hasElement(1));
    EXPECT_EQ(3, pool.capacity());              // slot retained
}

TEST_F(PtrReusablePoolTest, PushReusingSlotResetsAgainNotReconstructs)
{
    PtrReusablePool<Elem> pool;
    int i = pool.push();
    pool.at(i).id = 77;
    const int ctor_before = Elem::s_ctor;

    pool.remove(i);           // reset #1
    int j = pool.push();      // reuse slot -> reset #2, no new construction
    EXPECT_EQ(i, j);
    EXPECT_EQ(ctor_before, Elem::s_ctor); // no new object constructed
    EXPECT_EQ(0, pool.at(j).id);         // fresh logical state
}

// Pointer stability: a reference obtained from get() stays valid across many
// subsequent push() calls that reallocate the spine (fixes latent Graph/bingo UB).
TEST_F(PtrReusablePoolTest, GetReferenceStableAcrossGrowth)
{
    PtrReusablePool<Elem> pool;
    int i = pool.push();
    pool.at(i).id = 12345;
    Elem* addr = &pool.at(i);

    for (int k = 0; k < 64; k++) // force spine reallocation beyond DEFAULT_RESERVE
        pool.push();

    EXPECT_EQ(addr, &pool.at(i)); // pointee did not move
    EXPECT_EQ(12345, pool.at(i).id);
}

// Exception-safe growth: a throwing constructor during push() leaves the pool
// state (size/capacity/free list) unchanged and the pool still usable.
TEST(PtrReusablePoolExceptionSafety, ThrowingCtorDuringGrowthKeepsPoolConsistent)
{
    ThrowingElem::configure(/*countdown=*/3);
    {
        PtrReusablePool<ThrowingElem> pool;
        ASSERT_EQ(0, pool.push()); // countdown 3 -> 2
        ASSERT_EQ(1, pool.push()); // countdown 2 -> 1
        ASSERT_EQ(2, pool.size());
        ASSERT_EQ(2, pool.capacity());

        EXPECT_THROW(pool.push(), std::runtime_error); // countdown 1 -> 0 -> throw

        // State unchanged by the failed growth.
        EXPECT_EQ(2, pool.size());
        EXPECT_EQ(2, pool.capacity());
        EXPECT_TRUE(pool.hasElement(0));
        EXPECT_TRUE(pool.hasElement(1));

        // Pool still usable: disable throwing and grow successfully.
        ThrowingElem::s_countdown = 0;
        EXPECT_EQ(2, pool.push());
        EXPECT_EQ(3, pool.size());
    }
    EXPECT_EQ(0, ThrowingElem::s_live) << "ThrowingElem leaked";
}

// =====================================================================
// Iteration, access, whole-pool reset and purge.
// =====================================================================

TEST_F(PtrReusablePoolTest, IterationSkipsFreedHoles)
{
    PtrReusablePool<Elem> pool;
    for (int k = 0; k < 5; k++)
        pool.at(pool.push()).id = k;
    pool.remove(1);
    pool.remove(3);

    std::vector<int> ids;
    for (int i = pool.begin(); i != pool.end(); i = pool.next(i))
        ids.push_back(pool.at(i).id);

    ASSERT_EQ(3u, ids.size());
    EXPECT_EQ(0, ids[0]);
    EXPECT_EQ(2, ids[1]);
    EXPECT_EQ(4, ids[2]);
}

TEST_F(PtrReusablePoolTest, AccessToRemovedSlotThrows)
{
    PtrReusablePool<Elem> pool;
    int i = pool.push();
    pool.remove(i);
    EXPECT_THROW(pool.at(i), Exception);
    EXPECT_THROW(pool.remove(i), Exception); // double remove
}

// Whole-pool reuse(): all slots -> free list, objects retained (capacity kept),
// pool reusable. Distinct from purge().
TEST_F(PtrReusablePoolTest, ResetEmptiesButRetainsObjectsAndCapacity)
{
    PtrReusablePool<Elem> pool;
    pool.push();
    pool.push();
    pool.push();
    ASSERT_EQ(3, Elem::s_live);

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.capacity());   // active spine emptied -> end()==0
    EXPECT_EQ(3, pool.reserveSize()); // objects moved to the reserve
    EXPECT_EQ(3, Elem::s_live);       // objects NOT destroyed

    // Reuses a reserved object (no new construction).
    const int ctor_before = Elem::s_ctor;
    pool.push();
    EXPECT_EQ(1, pool.size());
    EXPECT_EQ(1, pool.capacity());
    EXPECT_EQ(2, pool.reserveSize());
    EXPECT_EQ(ctor_before, Elem::s_ctor);
}

// purge(): actually destroys objects and releases memory.
TEST_F(PtrReusablePoolTest, PurgeDestroysEverything)
{
    PtrReusablePool<Elem> pool;
    pool.push();
    pool.push();
    ASSERT_EQ(2, Elem::s_live);

    pool.purge();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.capacity());
    EXPECT_EQ(0, Elem::s_live); // objects destroyed

    // Still usable after purge, restarting from index 0.
    EXPECT_EQ(0, pool.push());
}

// =====================================================================
// Behavioral-analysis gap tests (task #3766 deep review).
// =====================================================================

// reuse() with pre-existing holes: remove out of order, then reuse(), then
// push() must restart index allocation at 0,1,2,... regardless of the holes.
// Guards the descending-free-list rebuild that reproduces legacy ObjPool::clear().
TEST_F(PtrReusablePoolTest, ResetWithHolesRestartsAllocationFromZero)
{
    PtrReusablePool<Elem> pool;
    for (int k = 0; k < 5; k++)
        pool.push(); // 0..4
    pool.remove(3);
    pool.remove(1); // holes at 3 then 1
    ASSERT_EQ(3, pool.size());

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.capacity());    // active spine emptied
    EXPECT_EQ(5, pool.reserveSize()); // all 5 objects retained in the reserve

    for (int expected = 0; expected < 5; expected++)
        EXPECT_EQ(expected, pool.push()) << "allocation must restart at 0 after reuse()";
}

// Never-shrink reuse contract (RFC §4.2): a reused slot retains its object's
// buffer capacity — reuse() clears length, not the allocation. This documents
// the intended memory trade-off vs the legacy destroy+reconstruct (which gave a
// zero-capacity fresh object on every reuse).
TEST_F(PtrReusablePoolTest, ReuseRetainsElementBufferCapacity)
{
    PtrReusablePool<Elem> pool;
    int i = pool.push();
    pool.at(i).buf.resize(1000);
    const size_t cap = pool.at(i).buf.capacity();
    ASSERT_GE(cap, 1000u);

    pool.remove(i);      // reuse(): buf.clear() keeps capacity
    int j = pool.push(); // reuse slot i
    ASSERT_EQ(i, j);
    EXPECT_EQ(0u, pool.at(j).buf.size());      // logically empty
    EXPECT_GE(pool.at(j).buf.capacity(), cap); // buffer retained (never-shrink)
}

// push()+fill is NOT atomic: if the fill step throws after push(), the slot is
// already live. The documented safe pattern is remove(idx) in the catch, which
// this test exercises — after recovery the pool is consistent and the slot is
// reusable.
TEST_F(PtrReusablePoolTest, PushThenFillThrow_RemoveInCatchKeepsPoolConsistent)
{
    PtrReusablePool<Elem> pool;
    pool.push();           // 0
    pool.push();           // 1
    ASSERT_EQ(2, pool.size());

    int idx = -1;
    try
    {
        idx = pool.push(); // 2, slot already live
        pool.at(idx).id = 5;
        throw std::runtime_error("simulated fill failure after push");
    }
    catch (const std::exception&)
    {
        pool.remove(idx); // recommended recovery keeps the pool consistent
    }

    EXPECT_EQ(2, pool.size());
    EXPECT_FALSE(pool.hasElement(idx));
    EXPECT_EQ(idx, pool.push()); // freed slot reusable
}

// If reuse() throws while push() is reusing a freed slot, the slot must not be
// lost from both the free list and the live set: push() marks it live and
// counts it before calling reuse(), so bookkeeping stays consistent (the
// exception still propagates). Locks the fix for the reuse-path reuse() throw.
TEST(PtrReusablePoolResetThrow, SlotStaysConsistentWhenReuseResetThrows)
{
    ThrowOnResetElem::configure();
    {
        PtrReusablePool<ThrowOnResetElem> pool;
        int i = pool.push(); // grow (reset not called on grow)
        pool.remove(i);      // reuse() runs (not throwing yet) -> slot freed
        ASSERT_EQ(0, pool.size());

        ThrowOnResetElem::s_throw_on_reset = true;
        EXPECT_THROW(pool.push(), std::runtime_error); // reuse -> reuse() throws

        // Bookkeeping consistent despite the throw: slot is live and counted,
        // not lost.
        EXPECT_EQ(1, pool.size());
        EXPECT_TRUE(pool.hasElement(i));

        ThrowOnResetElem::s_throw_on_reset = false; // let teardown be clean
    }
    EXPECT_EQ(0, ThrowOnResetElem::s_live) << "ThrowOnResetElem leaked";
}

// =====================================================================
// Heterogeneous discipline (HETERO-POOL-DESIGN): per-type free lists keyed
// by Reusable::reuseTypeId(), lazy factory on growth, adopt() for populated
// objects. The reuse-on-remove + reuse-on-reuse lifecycle is INHERITED from
// the homogeneous discipline; only slot parking and construction differ.
// =====================================================================

namespace
{
    // Polymorphic element hierarchy: the pool stores the base, slots hold
    // concrete subclasses with distinct dynamic types (default reuseTypeId()).
    struct HetBase : public Reusable
    {
        static int s_live;
        static int s_reuses;

        int value = 0;

        HetBase()
        {
            ++s_live;
        }
        ~HetBase() override
        {
            --s_live;
        }
        void reuse() override
        {
            value = 0;
            ++s_reuses;
        }
        static void reset_counters()
        {
            s_live = 0;
            s_reuses = 0;
        }
    };
    int HetBase::s_live = 0;
    int HetBase::s_reuses = 0;

    struct HetA : public HetBase
    {
        static int s_made;
    };
    int HetA::s_made = 0;

    struct HetB : public HetBase
    {
        static int s_made;
    };
    int HetB::s_made = 0;

    std::unique_ptr<HetBase> makeA()
    {
        ++HetA::s_made;
        return std::make_unique<HetA>();
    }
    std::unique_ptr<HetBase> makeB()
    {
        ++HetB::s_made;
        return std::make_unique<HetB>();
    }

    const std::type_index kA(typeid(HetA));
    const std::type_index kB(typeid(HetB));

    class HeteroPoolTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            HetBase::reset_counters();
            HetA::s_made = 0;
            HetB::s_made = 0;
        }
        void TearDown() override
        {
            EXPECT_EQ(0, HetBase::s_live) << "HetBase instances leaked across test boundary";
        }
    };
} // namespace

TEST_F(HeteroPoolTest, AddGrowsViaLazyFactory)
{
    PtrReusablePool<HetBase> pool;
    EXPECT_EQ(0, pool.add(kA, makeA));
    EXPECT_EQ(1, pool.add(kB, makeB));
    EXPECT_EQ(2, pool.add(kA, makeA));
    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(2, HetA::s_made);
    EXPECT_EQ(1, HetB::s_made);
    EXPECT_EQ(kA, pool.at(0).reuseTypeId());
    EXPECT_EQ(kB, pool.at(1).reuseTypeId());
}

// Same-type reuse is LIFO within the type bucket, constructs nothing (the
// lazy factory must NOT run), and follows the inherited lifecycle:
// reuse-on-remove + reuse-on-reuse.
TEST_F(HeteroPoolTest, SameTypeReuseLIFOWithoutConstruction)
{
    PtrReusablePool<HetBase> pool;
    pool.add(kA, makeA); // 0
    pool.add(kB, makeB); // 1
    pool.add(kA, makeA); // 2

    const int reuses_before = HetBase::s_reuses;
    pool.remove(0);
    pool.remove(2);
    EXPECT_EQ(reuses_before + 2, HetBase::s_reuses); // reuse-on-remove
    EXPECT_EQ(3, HetBase::s_live);                   // objects retained
    EXPECT_EQ(3, pool.capacity());

    const int made_before = HetA::s_made;
    EXPECT_EQ(2, pool.add(kA, makeA)); // LIFO within type: last freed A first
    EXPECT_EQ(0, pool.add(kA, makeA));
    EXPECT_EQ(made_before, HetA::s_made);            // factory never ran
    EXPECT_EQ(reuses_before + 4, HetBase::s_reuses); // + reuse-on-reuse x2
    EXPECT_EQ(3, pool.capacity());                   // no growth

    EXPECT_EQ(3, pool.add(kA, makeA)); // bucket empty -> grow
    EXPECT_EQ(made_before + 1, HetA::s_made);
}

// A freed slot of a DIFFERENT type must not be reused: the request grows
// instead, and the foreign slot stays parked for its own type.
TEST_F(HeteroPoolTest, ForeignTypeSlotNotReused)
{
    PtrReusablePool<HetBase> pool;
    pool.add(kA, makeA); // 0
    pool.add(kB, makeB); // 1
    pool.remove(0);      // free A slot

    EXPECT_EQ(2, pool.add(kB, makeB)); // grows; does NOT take slot 0
    EXPECT_FALSE(pool.hasElement(0));
    EXPECT_EQ(2, HetB::s_made);

    EXPECT_EQ(0, pool.add(kA, makeA)); // A request reuses the A slot
    EXPECT_EQ(1, HetA::s_made);        // no construction for the reuse
}

// adopt(): takes a populated object as-is (no reuse() on the way in, payload
// preserved), always appends; after remove() the slot joins the per-type
// free list and construct-flow add() of the same type can reuse it.
TEST_F(HeteroPoolTest, AdoptKeepsPayloadAndJoinsReuseAfterRemove)
{
    PtrReusablePool<HetBase> pool;
    auto obj = std::make_unique<HetA>();
    obj->value = 42;

    int idx = pool.adopt(std::move(obj));
    EXPECT_EQ(0, idx);
    EXPECT_EQ(42, pool.at(idx).value) << "adopt must not reset the payload";

    pool.remove(idx); // reuse-on-remove parks it in the A bucket
    const int made_before = HetA::s_made;
    EXPECT_EQ(idx, pool.add(kA, makeA)) << "construct-flow reuses the adopted slot";
    EXPECT_EQ(made_before, HetA::s_made);
    EXPECT_EQ(0, pool.at(idx).value) << "reused object must be reset";
}

TEST_F(HeteroPoolTest, FactoryMisbehaviorThrows)
{
    PtrReusablePool<HetBase> pool;
    // Factory produces a type different from the requested key.
    EXPECT_THROW(pool.add(kA, makeB), Exception);
    // Factory returns null.
    EXPECT_THROW(pool.add(kA, []() -> std::unique_ptr<HetBase> { return nullptr; }), Exception);
    EXPECT_EQ(0, pool.size());
    // The pool remains usable.
    EXPECT_EQ(0, pool.add(kA, makeA));
}

TEST_F(HeteroPoolTest, MixingDisciplinesThrows)
{
    PtrReusablePool<HetBase> hetero;
    hetero.add(kA, makeA);
    EXPECT_THROW(hetero.push(), Exception);

    PtrReusablePool<HetBase> homogeneous;
    homogeneous.push(); // HetBase is concrete -> homogeneous use is legal
    EXPECT_THROW(homogeneous.add(kA, makeA), Exception);
    EXPECT_THROW(homogeneous.adopt(std::make_unique<HetA>()), Exception);
}

// clear() is NON-destructive for the heterogeneous discipline too: objects
// move to the per-type reserve (retained, not destroyed); the active spine
// empties so allocation restarts at 0 (legacy PtrPool::clear() OBSERVABLE
// contract: emptied backing, restart at 0 — but WITHOUT freeing objects).
TEST_F(HeteroPoolTest, ClearRetainsObjectsAndRestartsAtZero)
{
    PtrReusablePool<HetBase> pool;
    pool.add(kA, makeA);
    pool.add(kB, makeB);
    ASSERT_EQ(2, HetBase::s_live);

    pool.clear();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.capacity());
    EXPECT_EQ(2, pool.reserveSize());
    EXPECT_EQ(2, HetBase::s_live) << "hetero clear() must RETAIN objects (reuse, not destroy)";

    const int a_made = HetA::s_made;
    EXPECT_EQ(0, pool.add(kA, makeA)); // restarts at 0, reuses a reserved A
    EXPECT_EQ(a_made, HetA::s_made) << "reused reserved object, no construction";
}

// Whole-pool reuse(): non-destructive, moves every object to its per-type
// reserve; the active spine empties, so same-type allocation restarts at 0
// sequentially (matches legacy clear() emptied-backing behavior).
TEST_F(HeteroPoolTest, WholePoolReuseRetainsPerTypeAndRestartsAtZero)
{
    PtrReusablePool<HetBase> pool;
    pool.add(kA, makeA); // 0
    pool.add(kB, makeB); // 1
    pool.add(kA, makeA); // 2

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.capacity());    // active spine emptied
    EXPECT_EQ(3, pool.reserveSize()); // 3 objects retained (2 A + 1 B)
    EXPECT_EQ(3, HetBase::s_live) << "whole-pool reuse() retains objects";

    const int a_made = HetA::s_made, b_made = HetB::s_made;
    EXPECT_EQ(0, pool.add(kA, makeA)); // fresh spine, sequential from 0
    EXPECT_EQ(1, pool.add(kA, makeA));
    EXPECT_EQ(2, pool.add(kB, makeB));
    EXPECT_EQ(a_made, HetA::s_made);
    EXPECT_EQ(b_made, HetB::s_made); // all reuses, no construction
}
