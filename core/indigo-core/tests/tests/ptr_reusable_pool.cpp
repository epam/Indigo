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
    EXPECT_EQ(0, pool.add());
    EXPECT_EQ(1, pool.add());
    EXPECT_EQ(2, pool.add());
    EXPECT_EQ(3, pool.size());
    EXPECT_EQ(3, pool.end());
}

// Reuse is LIFO and does not grow the spine — matches legacy Pool<T> order.
TEST_F(PtrReusablePoolTest, RemoveThenPushReusesIndexLIFO)
{
    PtrReusablePool<Elem> pool;
    pool.add(); // 0
    pool.add(); // 1
    pool.add(); // 2

    pool.remove(0);
    pool.remove(2);
    EXPECT_EQ(3, pool.end()); // holes, no growth

    EXPECT_EQ(2, pool.add()); // LIFO: last freed first
    EXPECT_EQ(0, pool.add());
    EXPECT_EQ(3, pool.end()); // reused, still no growth
    EXPECT_EQ(3, pool.add()); // free list empty -> grow
    EXPECT_EQ(4, pool.end());
}

// Reset-on-remove: the object is NOT destroyed on remove; it is reset in place
// and retained for reuse. This is the deliberate change from PtrPool (delete).
TEST_F(PtrReusablePoolTest, RemoveResetsButRetainsObject)
{
    PtrReusablePool<Elem> pool;
    pool.add();
    pool.add();
    pool.add();
    ASSERT_EQ(3, Elem::s_live);

    const int resets_before = Elem::s_resets;
    pool.remove(1);

    EXPECT_EQ(3, Elem::s_live);                   // object retained, not destroyed
    EXPECT_EQ(resets_before + 1, Elem::s_resets); // reuse() was called
    EXPECT_EQ(2, pool.size());
    EXPECT_FALSE(pool.hasElement(1));
    EXPECT_EQ(3, pool.end()); // slot retained
}

TEST_F(PtrReusablePoolTest, PushReusingSlotResetsAgainNotReconstructs)
{
    PtrReusablePool<Elem> pool;
    int i = pool.add();
    pool.at(i).id = 77;
    const int ctor_before = Elem::s_ctor;

    pool.remove(i);     // reset #1
    int j = pool.add(); // reuse slot -> reset #2, no new construction
    EXPECT_EQ(i, j);
    EXPECT_EQ(ctor_before, Elem::s_ctor); // no new object constructed
    EXPECT_EQ(0, pool.at(j).id);          // fresh logical state
}

// Pointer stability: a reference obtained from get() stays valid across many
// subsequent push() calls that reallocate the spine (fixes latent Graph/bingo UB).
TEST_F(PtrReusablePoolTest, GetReferenceStableAcrossGrowth)
{
    PtrReusablePool<Elem> pool;
    int i = pool.add();
    pool.at(i).id = 12345;
    Elem* addr = &pool.at(i);

    for (int k = 0; k < 64; k++) // force spine reallocation beyond DEFAULT_RESERVE
        pool.add();

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
        ASSERT_EQ(0, pool.add()); // countdown 3 -> 2
        ASSERT_EQ(1, pool.add()); // countdown 2 -> 1
        ASSERT_EQ(2, pool.size());
        ASSERT_EQ(2, pool.end());

        EXPECT_THROW(pool.add(), std::runtime_error); // countdown 1 -> 0 -> throw

        // State unchanged by the failed growth.
        EXPECT_EQ(2, pool.size());
        EXPECT_EQ(2, pool.end());
        EXPECT_TRUE(pool.hasElement(0));
        EXPECT_TRUE(pool.hasElement(1));

        // Pool still usable: disable throwing and grow successfully.
        ThrowingElem::s_countdown = 0;
        EXPECT_EQ(2, pool.add());
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
        pool.at(pool.add()).id = k;
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
    int i = pool.add();
    pool.remove(i);
    EXPECT_THROW(pool.at(i), Exception);
    EXPECT_THROW(pool.remove(i), Exception); // double remove
}

// Whole-pool reuse(): all slots -> free list, objects retained (capacity kept),
// pool reusable. Distinct from purge().
TEST_F(PtrReusablePoolTest, ResetEmptiesButRetainsObjectsAndCapacity)
{
    PtrReusablePool<Elem> pool;
    pool.add();
    pool.add();
    pool.add();
    ASSERT_EQ(3, Elem::s_live);

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.end());         // active spine emptied -> end()==0
    EXPECT_EQ(3, pool.reserveSize()); // objects moved to the reserve
    EXPECT_EQ(3, Elem::s_live);       // objects NOT destroyed

    // Reuses a reserved object (no new construction).
    const int ctor_before = Elem::s_ctor;
    pool.add();
    EXPECT_EQ(1, pool.size());
    EXPECT_EQ(1, pool.end());
    EXPECT_EQ(2, pool.reserveSize());
    EXPECT_EQ(ctor_before, Elem::s_ctor);
}

// purge(): actually destroys objects and releases memory.
TEST_F(PtrReusablePoolTest, PurgeDestroysEverything)
{
    PtrReusablePool<Elem> pool;
    pool.add();
    pool.add();
    ASSERT_EQ(2, Elem::s_live);

    pool.purge();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.end());
    EXPECT_EQ(0, Elem::s_live); // objects destroyed

    // Still usable after purge, restarting from index 0.
    EXPECT_EQ(0, pool.add());
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
        pool.add(); // 0..4
    pool.remove(3);
    pool.remove(1); // holes at 3 then 1
    ASSERT_EQ(3, pool.size());

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.end());         // active spine emptied
    EXPECT_EQ(5, pool.reserveSize()); // all 5 objects retained in the reserve

    for (int expected = 0; expected < 5; expected++)
        EXPECT_EQ(expected, pool.add()) << "allocation must restart at 0 after reuse()";
}

// Reserve cap: clear() retains at most MAX_RESERVE_PER_BUCKET retired objects;
// the excess is DESTROYED so memory stays bounded instead of being pinned at
// the all-time high-water mark.
TEST_F(PtrReusablePoolTest, ClearBoundsReserveToCap)
{
    PtrReusablePool<Elem> pool;
    const int cap = PtrReusablePool<Elem>::MAX_RESERVE_PER_BUCKET;
    for (int k = 0; k < cap + 10; k++)
        pool.add();
    ASSERT_EQ(cap + 10, Elem::s_live);

    pool.reuse(); // park up to cap into the reserve, destroy the remaining 10
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(cap, pool.reserveSize()) << "reserve must be capped at MAX_RESERVE_PER_BUCKET";
    EXPECT_EQ(cap, Elem::s_live) << "objects beyond the cap are destroyed, not pinned in the reserve";
}

// Never-shrink reuse contract (RFC §4.2): a reused slot retains its object's
// buffer capacity — reuse() clears length, not the allocation. This documents
// the intended memory trade-off vs the legacy destroy+reconstruct (which gave a
// zero-capacity fresh object on every reuse).
TEST_F(PtrReusablePoolTest, ReuseRetainsElementBufferCapacity)
{
    PtrReusablePool<Elem> pool;
    int i = pool.add();
    pool.at(i).buf.resize(1000);
    const size_t cap = pool.at(i).buf.capacity();
    ASSERT_GE(cap, 1000u);

    pool.remove(i);     // reuse(): buf.clear() keeps capacity
    int j = pool.add(); // reuse slot i
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
    pool.add(); // 0
    pool.add(); // 1
    ASSERT_EQ(2, pool.size());

    int idx = -1;
    try
    {
        idx = pool.add(); // 2, slot already live
        pool.at(idx).id = 5;
        throw std::runtime_error("simulated fill failure after push");
    }
    catch (const std::exception&)
    {
        pool.remove(idx); // recommended recovery keeps the pool consistent
    }

    EXPECT_EQ(2, pool.size());
    EXPECT_FALSE(pool.hasElement(idx));
    EXPECT_EQ(idx, pool.add()); // freed slot reusable
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
        int i = pool.add(); // grow (reset not called on grow)
        pool.remove(i);     // reuse() runs (not throwing yet) -> slot freed
        ASSERT_EQ(0, pool.size());

        ThrowOnResetElem::s_throw_on_reset = true;
        EXPECT_THROW(pool.add(), std::runtime_error); // reuse -> reuse() throws

        // Bookkeeping consistent despite the throw: slot is live and counted,
        // not lost.
        EXPECT_EQ(1, pool.size());
        EXPECT_TRUE(pool.hasElement(i));

        ThrowOnResetElem::s_throw_on_reset = false; // let teardown be clean
    }
    EXPECT_EQ(0, ThrowOnResetElem::s_live) << "ThrowOnResetElem leaked";
}

// =====================================================================
// Custom family (add_t/push_t): per-type spares keyed by
// Reusable::reuseTypeId(), lazy factory on growth, the caller fills the
// element in place. The reuse-on-remove + reuse-on-clear lifecycle is the
// same as the standard family's; only slot parking and construction differ.
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

    std::unique_ptr<HetBase> makeA(void*)
    {
        ++HetA::s_made;
        return std::make_unique<HetA>();
    }
    std::unique_ptr<HetBase> makeB(void*)
    {
        ++HetB::s_made;
        return std::make_unique<HetB>();
    }

    const std::type_index kA(typeid(HetA));
    const std::type_index kB(typeid(HetB));

    // Factories in the shape the pool takes them: type key + lazy maker.
    const PtrReusablePool<HetBase>::Factory kFactoryA{kA, &makeA};
    const PtrReusablePool<HetBase>::Factory kFactoryB{kB, &makeB};

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
    EXPECT_EQ(0, pool.add_t(kFactoryA));
    EXPECT_EQ(1, pool.add_t(kFactoryB));
    EXPECT_EQ(2, pool.add_t(kFactoryA));
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
    pool.add_t(kFactoryA); // 0
    pool.add_t(kFactoryB); // 1
    pool.add_t(kFactoryA); // 2

    const int reuses_before = HetBase::s_reuses;
    pool.remove(0);
    pool.remove(2);
    EXPECT_EQ(reuses_before + 2, HetBase::s_reuses); // reuse-on-remove
    EXPECT_EQ(3, HetBase::s_live);                   // objects retained
    EXPECT_EQ(3, pool.end());

    const int made_before = HetA::s_made;
    EXPECT_EQ(2, pool.add_t(kFactoryA)); // LIFO within type: last freed A first
    EXPECT_EQ(0, pool.add_t(kFactoryA));
    EXPECT_EQ(made_before, HetA::s_made);            // factory never ran
    EXPECT_EQ(reuses_before + 4, HetBase::s_reuses); // + reuse-on-reuse x2
    EXPECT_EQ(3, pool.end());                        // no growth

    EXPECT_EQ(3, pool.add_t(kFactoryA)); // bucket empty -> grow
    EXPECT_EQ(made_before + 1, HetA::s_made);
}

// A freed slot of a DIFFERENT type must not be reused: the request grows
// instead, and the foreign slot stays parked for its own type.
TEST_F(HeteroPoolTest, ForeignTypeSlotNotReused)
{
    PtrReusablePool<HetBase> pool;
    pool.add_t(kFactoryA); // 0
    pool.add_t(kFactoryB); // 1
    pool.remove(0);        // free A slot

    EXPECT_EQ(2, pool.add_t(kFactoryB)); // grows; does NOT take slot 0
    EXPECT_FALSE(pool.hasElement(0));
    EXPECT_EQ(2, HetB::s_made);

    EXPECT_EQ(0, pool.add_t(kFactoryA)); // A request reuses the A slot
    EXPECT_EQ(1, HetA::s_made);          // no construction for the reuse
}

// push_t(): hands out the element so a caller fills it IN PLACE — the pool
// owns the construction, so no ready-made object is ever handed in (a filled
// slot then behaves exactly like any other: reuse-on-remove, recycled by a
// later same-type request without constructing).
TEST_F(HeteroPoolTest, PushTHandsOutElementForInPlaceFill)
{
    PtrReusablePool<HetBase> pool;
    HetBase& elem = pool.push_t(kFactoryA);
    elem.value = 42; // filled in place, through the pool's own element
    EXPECT_EQ(0, pool.begin());
    EXPECT_EQ(42, pool.at(0).value);
    EXPECT_EQ(&elem, &pool.at(0)) << "push_t must return the element of the new slot";

    pool.remove(0); // reuse-on-remove parks it in the A bucket
    const int made_before = HetA::s_made;
    EXPECT_EQ(0, pool.add_t(kFactoryA)) << "the freed slot is recycled";
    EXPECT_EQ(made_before, HetA::s_made) << "recycling constructs nothing";
    EXPECT_EQ(0, pool.at(0).value) << "a recycled element is reset";
}

TEST_F(HeteroPoolTest, FactoryMisbehaviorThrows)
{
    PtrReusablePool<HetBase> pool;
    // Factory produces a type different from the requested key.
    EXPECT_THROW(pool.add_t(kA, [] { return makeB(nullptr); }), Exception);
    // Factory returns null.
    EXPECT_THROW(pool.add_t(kA, []() -> std::unique_ptr<HetBase> { return nullptr; }), Exception);
    EXPECT_EQ(0, pool.size());
    // The pool remains usable.
    EXPECT_EQ(0, pool.add_t(kFactoryA));
}

TEST_F(HeteroPoolTest, MixingAddFamiliesThrows)
{
    PtrReusablePool<HetBase> custom;
    custom.add_t(kFactoryA);
    EXPECT_THROW(custom.add(), Exception);

    PtrReusablePool<HetBase> standard;
    standard.add(); // HetBase is concrete -> the standard family is legal
    EXPECT_THROW(standard.add_t(kFactoryA), Exception);
}

// clear() is NON-destructive for the custom family too: objects
// move to the per-type reserve (retained, not destroyed); the active spine
// empties so allocation restarts at 0 (legacy PtrPool::clear() OBSERVABLE
// contract: emptied backing, restart at 0 — but WITHOUT freeing objects).
TEST_F(HeteroPoolTest, ClearRetainsObjectsAndRestartsAtZero)
{
    PtrReusablePool<HetBase> pool;
    pool.add_t(kFactoryA);
    pool.add_t(kFactoryB);
    ASSERT_EQ(2, HetBase::s_live);

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.end());
    EXPECT_EQ(2, pool.reserveSize());
    EXPECT_EQ(2, HetBase::s_live) << "hetero clear() must RETAIN objects (reuse, not destroy)";

    const int a_made = HetA::s_made;
    EXPECT_EQ(0, pool.add_t(kFactoryA)); // restarts at 0, reuses a reserved A
    EXPECT_EQ(a_made, HetA::s_made) << "reused reserved object, no construction";
}

// Whole-pool reuse(): non-destructive, moves every object to its per-type
// reserve; the active spine empties, so same-type allocation restarts at 0
// sequentially (matches legacy clear() emptied-backing behavior).
TEST_F(HeteroPoolTest, WholePoolReuseRetainsPerTypeAndRestartsAtZero)
{
    PtrReusablePool<HetBase> pool;
    pool.add_t(kFactoryA); // 0
    pool.add_t(kFactoryB); // 1
    pool.add_t(kFactoryA); // 2

    pool.reuse();
    EXPECT_EQ(0, pool.size());
    EXPECT_EQ(0, pool.end());         // active spine emptied
    EXPECT_EQ(3, pool.reserveSize()); // 3 objects retained (2 A + 1 B)
    EXPECT_EQ(3, HetBase::s_live) << "whole-pool reuse() retains objects";

    const int a_made = HetA::s_made, b_made = HetB::s_made;
    EXPECT_EQ(0, pool.add_t(kFactoryA)); // fresh spine, sequential from 0
    EXPECT_EQ(1, pool.add_t(kFactoryA));
    EXPECT_EQ(2, pool.add_t(kFactoryB));
    EXPECT_EQ(a_made, HetA::s_made);
    EXPECT_EQ(b_made, HetB::s_made); // all reuses, no construction
}

// =====================================================================
// Initializing add(args...), element-returning push(), and iteration.
// =====================================================================

namespace
{
    // Element with an initializing reuse() overload, the shape the pool relies
    // on to make a fresh and a recycled slot indistinguishable.
    struct ArgElem : public Reusable
    {
        static int s_ctor;
        int a = -1;
        int b = -1;

        ArgElem()
        {
            ++s_ctor;
        }
        void reuse() override
        {
            a = -1;
            b = -1;
        }
        void reuse(int first)
        {
            reuse();
            a = first;
        }
        void reuse(int first, int second)
        {
            reuse();
            a = first;
            b = second;
        }
    };
    int ArgElem::s_ctor = 0;
} // namespace

// add(args...) forwards to T::reuse(args...) on the fresh path AND on the
// recycled path, so a caller never sees a half-initialized element.
TEST(PtrReusablePoolArgs, AddInitializesOnFreshAndRecycledSlot)
{
    PtrReusablePool<ArgElem> pool;
    ArgElem::s_ctor = 0;

    const int i = pool.add(7);
    EXPECT_EQ(7, pool.at(i).a);
    EXPECT_EQ(-1, pool.at(i).b);

    const int j = pool.add(1, 2);
    EXPECT_EQ(1, pool.at(j).a);
    EXPECT_EQ(2, pool.at(j).b);
    EXPECT_EQ(2, ArgElem::s_ctor);

    pool.remove(i);
    const int recycled = pool.add(9, 8); // same slot, no construction
    EXPECT_EQ(i, recycled);
    EXPECT_EQ(2, ArgElem::s_ctor) << "a recycled slot constructs nothing";
    EXPECT_EQ(9, pool.at(recycled).a) << "a recycled element is initialized too";
    EXPECT_EQ(8, pool.at(recycled).b);
}

// push(args...) is add(args...) that hands back the element itself.
TEST(PtrReusablePoolArgs, PushReturnsTheNewElement)
{
    PtrReusablePool<ArgElem> pool;
    ArgElem& elem = pool.push(4, 5);
    EXPECT_EQ(4, elem.a);
    EXPECT_EQ(5, elem.b);
    EXPECT_EQ(&elem, &pool.at(pool.begin()));
    EXPECT_EQ(1, pool.size());
}

// items() walks the live elements, skipping freed slots; indices() yields the
// same indices as the begin()/next()/end() walk.
TEST(PtrReusablePoolIteration, ItemsAndIndicesSkipFreedSlots)
{
    PtrReusablePool<ArgElem> pool;
    for (int k = 0; k < 5; k++)
        pool.add(k);
    pool.remove(1);
    pool.remove(3);

    std::vector<int> seen;
    for (ArgElem& elem : pool.items())
        seen.push_back(elem.a);
    EXPECT_EQ(std::vector<int>({0, 2, 4}), seen);

    std::vector<int> walked;
    for (int i = pool.begin(); i != pool.end(); i = pool.next(i))
        walked.push_back(i);
    std::vector<int> from_range;
    for (int i : pool.indices())
        from_range.push_back(i);
    EXPECT_EQ(walked, from_range);

    // const access yields const elements.
    const PtrReusablePool<ArgElem>& const_pool = pool;
    int count = 0;
    for (const ArgElem& elem : const_pool.items())
    {
        EXPECT_GE(elem.a, 0);
        ++count;
    }
    EXPECT_EQ(3, count);
}

TEST(PtrReusablePoolIteration, ItemsOnEmptyPoolIsEmpty)
{
    PtrReusablePool<ArgElem> pool;
    int count = 0;
    for (ArgElem& elem : pool.items())
    {
        (void)elem;
        ++count;
    }
    EXPECT_EQ(0, count);
    pool.add(1);
    pool.clear(); // clear() is the consumer-facing name for reuse()
    for (ArgElem& elem : pool.items())
    {
        (void)elem;
        ++count;
    }
    EXPECT_EQ(0, count) << "a cleared pool iterates over nothing";
}
