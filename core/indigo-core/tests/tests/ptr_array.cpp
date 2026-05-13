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

// Characterization tests for indigo::PtrArray<T>.
//
// Purpose: lock current observable behavior of PtrArray before refactoring
// (Phase 1 of milestone-19 ObjArray/PtrArray rewrite). These tests must pass
// against the existing Array<T*>-based implementation AND against the future
// std::vector<std::unique_ptr<T>>-based wrapper. Anything that diverges
// between the two implementations represents a breaking-change candidate
// that must be either reconciled or explicitly documented in the migration
// notes.

#include <gtest/gtest.h>

#include <base_cpp/ptr_array.h>

#include <stdexcept>
#include <type_traits>

using namespace indigo;

namespace
{
    // Tracked: counts live instances and records construction order. Used to
    // verify destruction (no leaks, no double-free) and ownership transfers.
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

    // ThrowOnConstruct: throws on construction when the counter reaches zero.
    // Used to test exception safety of emplace().
    struct ThrowOnConstruct
    {
        static int s_live;
        static int s_throw_countdown; // if > 0, decremented; throws when hits 0

        int id;

        explicit ThrowOnConstruct(int v = 0)
        {
            if (s_throw_countdown > 0 && --s_throw_countdown == 0)
                throw std::runtime_error("ThrowOnConstruct: intentional throw");
            id = v;
            ++s_live;
        }
        ~ThrowOnConstruct()
        {
            --s_live;
        }

        static void reset_counters(int countdown = 0)
        {
            s_live = 0;
            s_throw_countdown = countdown;
        }
    };

    int ThrowOnConstruct::s_live = 0;
    int ThrowOnConstruct::s_throw_countdown = 0;

    // Per-test fixture that resets counters and asserts no leaks at TearDown.
    class PtrArrayTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            Tracked::reset_counters();
            ThrowOnConstruct::reset_counters();
        }
        void TearDown() override
        {
            // Subtests must leave Tracked::s_live at 0 by the time the
            // fixture tears down (i.e. arrays have gone out of scope).
            EXPECT_EQ(0, Tracked::s_live) << "Tracked instances leaked across test boundary";
            EXPECT_EQ(0, ThrowOnConstruct::s_live) << "ThrowOnConstruct instances leaked across test boundary";
        }
    };
} // namespace

// =====================================================================
// Type-trait characterization (compile-time contract).
// =====================================================================

TEST(PtrArrayTraits, NotCopyable)
{
    static_assert(!std::is_copy_constructible_v<PtrArray<int>>, "PtrArray must not be copy-constructible");
    static_assert(!std::is_copy_assignable_v<PtrArray<int>>, "PtrArray must not be copy-assignable");
}

TEST(PtrArrayTraits, DefaultConstructible)
{
    static_assert(std::is_default_constructible_v<PtrArray<int>>, "PtrArray must be default-constructible");
}

// Post-Phase-1: PtrArray is nothrow-movable (std::vector + unique_ptr give
// us this by default). Compile-time enforcement matters here because the
// downstream rationale for the rewrite is "client containers like
// std::vector<PtrArray<X>> become viable" — that requires nothrow move.
TEST(PtrArrayTraits, Movable_PostRefactor)
{
    static_assert(std::is_nothrow_move_constructible_v<PtrArray<int>>, "PtrArray must be nothrow-move-constructible");
    static_assert(std::is_nothrow_move_assignable_v<PtrArray<int>>, "PtrArray must be nothrow-move-assignable");
    // Belt-and-suspenders runtime check so a regression shows up in test
    // output even on compilers that allow the static_asserts to lie.
    EXPECT_TRUE(std::is_nothrow_move_constructible_v<PtrArray<int>>);
    EXPECT_TRUE(std::is_nothrow_move_assignable_v<PtrArray<int>>);
}

// Verify move actually transfers ownership and leaves the source empty.
TEST_F(PtrArrayTest, MoveCtor_TransfersOwnership)
{
    PtrArray<Tracked> src;
    src.emplace(1);
    src.emplace(2);
    EXPECT_EQ(2, Tracked::s_live);

    PtrArray<Tracked> dst(std::move(src));
    EXPECT_EQ(2, dst.size());
    EXPECT_EQ(0, src.size());      // moved-from is empty
    EXPECT_EQ(2, Tracked::s_live); // nothing destroyed by the move
    EXPECT_EQ(1, dst[0]->id);
    EXPECT_EQ(2, dst[1]->id);
}

TEST_F(PtrArrayTest, MoveAssign_ReplacesOwnedObjects)
{
    PtrArray<Tracked> dst;
    dst.emplace(100); // dst owns this
    PtrArray<Tracked> src;
    src.emplace(200);
    src.emplace(300);

    dst = std::move(src);
    // dst's old object (id=100) must be destroyed; dst now owns id=200, 300.
    EXPECT_EQ(2, dst.size());
    EXPECT_EQ(200, dst[0]->id);
    EXPECT_EQ(300, dst[1]->id);
    EXPECT_EQ(0, src.size());
    EXPECT_EQ(2, Tracked::s_live);
}

// =====================================================================
// Basic API: construction, add, size, operator[], at, top.
// =====================================================================

TEST_F(PtrArrayTest, DefaultConstructed_IsEmpty)
{
    PtrArray<Tracked> arr;
    EXPECT_EQ(0, arr.size());
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PtrArrayTest, Add_TakesOwnershipAndIncreasesSize)
{
    PtrArray<Tracked> arr;
    Tracked& ref = arr.emplace(42);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(42, ref.id);
    EXPECT_EQ(1, Tracked::s_live);
}

TEST_F(PtrArrayTest, Add_Multiple_PreservesInsertionOrder)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.emplace(2);
    arr.emplace(3);
    ASSERT_EQ(3, arr.size());
    EXPECT_EQ(1, arr[0]->id);
    EXPECT_EQ(2, arr[1]->id);
    EXPECT_EQ(3, arr[2]->id);
    EXPECT_EQ(3, Tracked::s_live);
}

TEST_F(PtrArrayTest, Destructor_DeletesAllOwnedPointers)
{
    {
        PtrArray<Tracked> arr;
        arr.emplace(10);
        arr.emplace(20);
        EXPECT_EQ(2, Tracked::s_live);
    }
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PtrArrayTest, At_EquivalentToBracket)
{
    PtrArray<Tracked> arr;
    arr.emplace(7);
    arr.emplace(8);
    EXPECT_EQ(arr.at(0)->id, arr[0]->id);
    EXPECT_EQ(arr.at(1)->id, arr[1]->id);
}

TEST_F(PtrArrayTest, At_ConstOverload_ReturnsConstPointer)
{
    PtrArray<Tracked> arr;
    arr.emplace(11);
    const PtrArray<Tracked>& cref = arr;
    const Tracked* p = cref.at(0);
    EXPECT_EQ(11, p->id);
}

TEST_F(PtrArrayTest, Top_ReturnsLastElement)
{
    PtrArray<Tracked> arr;
    arr.emplace(100);
    arr.emplace(200);
    EXPECT_EQ(200, arr.top()->id);
}

// =====================================================================
// Pop, removeLast, remove: ownership transfer and shifts.
// =====================================================================

TEST_F(PtrArrayTest, Pop_TransfersOwnership_NoDelete)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.emplace(2);
    EXPECT_EQ(2, Tracked::s_live);

    auto popped = arr.pop(); // std::unique_ptr<Tracked>
    ASSERT_NE(nullptr, popped);
    EXPECT_EQ(2, popped->id);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(2, Tracked::s_live) << "pop() must not delete the object";

    popped.reset(); // explicit destruction via unique_ptr
    EXPECT_EQ(1, Tracked::s_live);
}

TEST_F(PtrArrayTest, Pop_OnEmpty_Throws)
{
    PtrArray<Tracked> arr;
    EXPECT_ANY_THROW(arr.pop());
}

TEST_F(PtrArrayTest, RemoveLast_DeletesObject)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.emplace(2);
    arr.removeLast();
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(1, Tracked::s_live);
    EXPECT_EQ(1, arr[0]->id);
}

TEST_F(PtrArrayTest, Remove_DeletesAndShifts)
{
    PtrArray<Tracked> arr;
    arr.emplace(10);
    arr.emplace(20);
    arr.emplace(30);
    arr.remove(1);
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ(10, arr[0]->id);
    EXPECT_EQ(30, arr[1]->id);
    EXPECT_EQ(2, Tracked::s_live);
}

TEST_F(PtrArrayTest, Remove_OnNullSlot_IsSafe)
{
    // The contract: delete-on-nullptr is well-defined (no-op).
    PtrArray<Tracked> arr;
    arr.expand(3); // fills with null
    arr.remove(1); // delete nullptr -> no-op; shifts
    EXPECT_EQ(2, arr.size());
}

// =====================================================================
// Clear, expand, resize.
// =====================================================================

TEST_F(PtrArrayTest, Clear_DeletesAllAndResetsSize)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.emplace(2);
    arr.emplace(3);
    arr.clear();
    EXPECT_EQ(0, arr.size());
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PtrArrayTest, Clear_OnEmpty_IsSafe)
{
    PtrArray<Tracked> arr;
    arr.clear();
    EXPECT_EQ(0, arr.size());
}

TEST_F(PtrArrayTest, Expand_FillsWithNull_DoesNotShrink)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.expand(4);
    EXPECT_EQ(4, arr.size());
    EXPECT_EQ(1, arr[0]->id);
    EXPECT_EQ(nullptr, arr[1]);
    EXPECT_EQ(nullptr, arr[2]);
    EXPECT_EQ(nullptr, arr[3]);

    // expand to smaller size must be no-op (contract from current impl).
    arr.expand(2);
    EXPECT_EQ(4, arr.size());
}

TEST_F(PtrArrayTest, Resize_Grow_FillsWithNull)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.resize(4);
    EXPECT_EQ(4, arr.size());
    EXPECT_EQ(1, arr[0]->id);
    EXPECT_EQ(nullptr, arr[1]);
    EXPECT_EQ(nullptr, arr[2]);
    EXPECT_EQ(nullptr, arr[3]);
}

TEST_F(PtrArrayTest, Resize_Shrink_DeletesExcess)
{
    PtrArray<Tracked> arr;
    arr.emplace(10);
    arr.emplace(20);
    arr.emplace(30);
    EXPECT_EQ(3, Tracked::s_live);
    arr.resize(1);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(1, Tracked::s_live);
    EXPECT_EQ(10, arr[0]->id);
}

TEST_F(PtrArrayTest, Resize_ToZero_EmptiesArray)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.emplace(2);
    arr.resize(0);
    EXPECT_EQ(0, arr.size());
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PtrArrayTest, Resize_Same_NoEffect)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.emplace(2);
    arr.resize(2);
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ(2, Tracked::s_live);
}

// =====================================================================
// Set, reset, release.
// =====================================================================

TEST_F(PtrArrayTest, Set_OnNullSlot_TakesOwnership)
{
    PtrArray<Tracked> arr;
    arr.expand(3);
    arr.set(1, std::make_unique<Tracked>(42));
    EXPECT_EQ(42, arr[1]->id);
    EXPECT_EQ(1, Tracked::s_live);
}

TEST_F(PtrArrayTest, Set_OnOccupiedSlot_Throws)
{
    PtrArray<Tracked> arr;
    arr.expand(3);
    arr.set(0, std::make_unique<Tracked>(1));
    auto second = std::make_unique<Tracked>(2);
    EXPECT_ANY_THROW(arr.set(0, std::move(second)));
    // second is now null (moved-from into the function parameter). The throw
    // fires before _items[idx] is modified, so arr still holds id=1.
    // The parameter unique_ptr (holding id=2) is destroyed during stack
    // unwind — so id=2 object is gone, s_live == 1.
    EXPECT_EQ(1, arr[0]->id);
    EXPECT_EQ(1, Tracked::s_live);
}

TEST_F(PtrArrayTest, Reset_NullSlot_IsIdempotent)
{
    PtrArray<Tracked> arr;
    arr.expand(2);
    arr.reset(0); // delete nullptr is OK
    arr.reset(0); // again, still OK
    EXPECT_EQ(nullptr, arr[0]);
}

TEST_F(PtrArrayTest, Reset_OccupiedSlot_DeletesAndNulls)
{
    PtrArray<Tracked> arr;
    arr.emplace(99);
    arr.reset(0);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(nullptr, arr[0]);
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PtrArrayTest, Reset_WithReplacement_DeletesOldTakesNew)
{
    PtrArray<Tracked> arr;
    arr.emplace(100);
    arr.reset(0, std::make_unique<Tracked>(200));
    EXPECT_EQ(200, arr[0]->id);
    EXPECT_EQ(1, Tracked::s_live);
}

TEST_F(PtrArrayTest, Release_TransfersOwnership_NoDelete)
{
    PtrArray<Tracked> arr;
    arr.emplace(7);
    arr.emplace(8);

    auto released = arr.release(0); // std::unique_ptr<Tracked>
    ASSERT_NE(nullptr, released);
    EXPECT_EQ(7, released->id);
    EXPECT_EQ(nullptr, arr[0]) << "Released slot must become null";
    EXPECT_EQ(2, arr.size()) << "Release must NOT shrink the array";
    EXPECT_EQ(2, Tracked::s_live) << "Released object must NOT be deleted";

    released.reset();
    EXPECT_EQ(1, Tracked::s_live);
}

TEST_F(PtrArrayTest, Release_NullSlot_ReturnsNullptr)
{
    PtrArray<Tracked> arr;
    arr.expand(2);
    auto released = arr.release(0);
    EXPECT_EQ(nullptr, released);
    EXPECT_EQ(2, arr.size());
}

// =====================================================================
// qsort: intentionally NOT tested.
//
// PtrArray::qsort(cmp, context) declares `const void* context` and forwards
// it to Array<T*>::qsort which expects `void* context`. This produces a
// hard compile error at instantiation time (function-pointer signature
// mismatch). No call site in the codebase instantiates it — it is dead
// code. After Phase 1 refactor we will simply drop the method.
// =====================================================================

// =====================================================================
// ptr(): intentionally NOT tested.
//
// The historical `T** ptr()` / `const T* const* ptr() const` API exposed
// the raw underlying C array of pointers. Post-Phase-1 the backing is
// `std::vector<std::unique_ptr<T>>` whose element layout (`unique_ptr`,
// not `T*`) is incompatible with the historical contract. The method was
// dropped: a codebase-wide grep found no real callers on PtrArray-typed
// variables. Should one resurface, the appropriate replacement is to
// build a `std::vector<T*>` view on-demand from the array.
// =====================================================================

// =====================================================================
// Null-handling contract: nullptr slots are valid and ignored by
// destruction logic (clear, resize-shrink, dtor).
// =====================================================================

TEST_F(PtrArrayTest, NullHoles_AreIgnoredByClear)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);
    arr.expand(3); // slots 1, 2 are nullptr
    arr.emplace(4);
    EXPECT_EQ(4, arr.size());
    EXPECT_EQ(2, Tracked::s_live);
    arr.clear();
    EXPECT_EQ(0, arr.size());
    EXPECT_EQ(0, Tracked::s_live);
}

TEST_F(PtrArrayTest, NullHoles_AreIgnoredByDestructor)
{
    {
        PtrArray<Tracked> arr;
        arr.expand(5); // all nullptr
        arr.set(2, std::make_unique<Tracked>(99));
        EXPECT_EQ(1, Tracked::s_live);
    }
    EXPECT_EQ(0, Tracked::s_live);
}

// =====================================================================
// Stress / sequence test: combination of operations matches expectations.
// =====================================================================

TEST_F(PtrArrayTest, StressSequence_NoLeaksNoDoubleFree)
{
    PtrArray<Tracked> arr;
    for (int i = 0; i < 10; ++i)
        arr.emplace(i);
    EXPECT_EQ(10, Tracked::s_live);

    // remove a few from middle and end
    arr.remove(3);    // delete id=3
    arr.removeLast(); // delete id=9
    EXPECT_EQ(8, arr.size());
    EXPECT_EQ(8, Tracked::s_live);

    // release one
    auto released = arr.release(2);
    EXPECT_EQ(2, released->id);
    EXPECT_EQ(8, arr.size()); // released slot is null, size unchanged
    EXPECT_EQ(nullptr, arr[2]);
    EXPECT_EQ(8, Tracked::s_live);

    // resize down
    arr.resize(4);
    EXPECT_EQ(4, arr.size());
    // surviving: id=0,1,nullptr,4
    EXPECT_EQ(0, arr[0]->id);
    EXPECT_EQ(1, arr[1]->id);
    EXPECT_EQ(nullptr, arr[2]);
    EXPECT_EQ(4, arr[3]->id);

    released.reset();
    // clear destroys remaining 3 owned objects
    arr.clear();
    EXPECT_EQ(0, Tracked::s_live);
}

// =====================================================================
// PR-1 new tests: unique_ptr-based API, emplace, exception safety.
// =====================================================================

// add(unique_ptr<T>): ownership transfers into the container.
TEST_F(PtrArrayTest, Add_FromUniquePtr_TakesOwnership)
{
    PtrArray<Tracked> arr;
    auto obj = std::make_unique<Tracked>(55);
    Tracked* raw = obj.get();

    Tracked& ref = arr.add(std::move(obj));

    EXPECT_EQ(nullptr, obj) << "unique_ptr must be empty after move";
    EXPECT_EQ(raw, &ref) << "returned reference must point to the object";
    EXPECT_EQ(55, ref.id);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(1, Tracked::s_live);
    EXPECT_EQ(raw, arr[0]) << "container must store the same pointer";
}

// emplace(): constructs in-place, returns reference to the new element.
TEST_F(PtrArrayTest, Emplace_ConstructsInPlace)
{
    PtrArray<Tracked> arr;

    Tracked& ref = arr.emplace(77);

    EXPECT_EQ(77, ref.id);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(1, Tracked::s_live);
    EXPECT_EQ(&ref, arr[0]) << "returned reference must alias the stored element";
    EXPECT_EQ(1, Tracked::s_constructed_total);
}

// push() is an ObjArray-compatible alias for emplace() — added to ease the
// future Step 2 migration of ObjArray<T> call sites to PtrArray<T> without
// having to mechanically rewrite every push(args) into emplace(args).
TEST_F(PtrArrayTest, Push_IsAliasForEmplace)
{
    PtrArray<Tracked> arr;

    Tracked& a = arr.push();   // 0-arg form (Tracked default ctor)
    Tracked& b = arr.push(42); // 1-arg form

    EXPECT_EQ(0, a.id);
    EXPECT_EQ(42, b.id);
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ(2, Tracked::s_live);
    EXPECT_EQ(&a, arr[0]);
    EXPECT_EQ(&b, arr[1]);
    EXPECT_EQ(2, Tracked::s_constructed_total);
}

// pop() returns unique_ptr; object lives until the unique_ptr goes out of scope.
TEST_F(PtrArrayTest, Pop_ReturnsUniquePtr_AutoDeletes)
{
    PtrArray<Tracked> arr;
    arr.emplace(10);
    arr.emplace(20);
    EXPECT_EQ(2, Tracked::s_live);

    {
        auto p = arr.pop();
        ASSERT_NE(nullptr, p);
        EXPECT_EQ(20, p->id);
        EXPECT_EQ(1, arr.size());
        EXPECT_EQ(2, Tracked::s_live) << "object must be alive while unique_ptr holds it";
    }
    // p went out of scope — Tracked destructor must have been called
    EXPECT_EQ(1, Tracked::s_live) << "unique_ptr destructor must delete the object";
}

// release(int) returns unique_ptr; slot becomes null; object is not deleted
// until the returned unique_ptr is destroyed.
TEST_F(PtrArrayTest, Release_NodiscardWarning_DoesNotLeak)
{
    PtrArray<Tracked> arr;
    arr.emplace(33);
    arr.emplace(44);

    {
        auto p = arr.release(0);
        ASSERT_NE(nullptr, p);
        EXPECT_EQ(33, p->id);
        EXPECT_EQ(nullptr, arr[0]) << "slot must be null after release";
        EXPECT_EQ(2, arr.size()) << "size must not change after release";
        EXPECT_EQ(2, Tracked::s_live) << "object must be alive while unique_ptr holds it";
    }
    // p destroyed — id=33 object cleaned up
    EXPECT_EQ(1, Tracked::s_live);
}

// Deprecated raw-pointer overload of add() still compiles and works.
// Suppressing the deprecation warning explicitly so the test suite remains
// warning-clean under -Werror when this test is intentionally exercising
// the legacy path.
TEST_F(PtrArrayTest, Add_FromDeprecatedRaw_StillWorks)
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    PtrArray<Tracked> arr;
    Tracked& ref = arr.add(new Tracked(99));
    EXPECT_EQ(99, ref.id);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(1, Tracked::s_live);

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
}

// emplace(): if T's constructor throws, the container must be unchanged
// (strong exception safety: make_unique throws before emplace_back is called).
TEST_F(PtrArrayTest, Emplace_ExceptionInCtor_ContainerUnchanged)
{
    PtrArray<ThrowOnConstruct> arr;

    // Pre-populate one element successfully.
    arr.emplace(1);
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(1, ThrowOnConstruct::s_live);

    // On the next construction attempt the counter fires and throws.
    ThrowOnConstruct::s_throw_countdown = 1;
    EXPECT_THROW(arr.emplace(2), std::runtime_error);

    // Container must be exactly as before the failed emplace.
    EXPECT_EQ(1, arr.size()) << "size must not change after failed emplace";
    EXPECT_EQ(1, arr[0]->id) << "existing element must be unaffected";
    EXPECT_EQ(1, ThrowOnConstruct::s_live) << "no partially-constructed objects must survive";
}

// add() with a null unique_ptr is rejected — the T& return type cannot
// represent "no object", and silently storing a null slot would be a UB trap
// for downstream code. Use expand() if null slots are needed.
TEST_F(PtrArrayTest, Add_NullUniquePtr_Throws)
{
    PtrArray<Tracked> arr;
    arr.emplace(1);

    std::unique_ptr<Tracked> null_ptr;
    ASSERT_EQ(nullptr, null_ptr);

    EXPECT_ANY_THROW(arr.add(std::move(null_ptr)));
    EXPECT_EQ(1, arr.size()) << "size must not change on rejected add";
    EXPECT_EQ(1, arr[0]->id);
    EXPECT_EQ(1, Tracked::s_live);
}

// Deprecated set(int, T*) overload delegates to the owning overload — verifies
// it still compiles and the slot is occupied as expected.
TEST_F(PtrArrayTest, Set_FromDeprecatedRaw_StillWorks)
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    PtrArray<Tracked> arr;
    arr.expand(2);
    arr.set(1, new Tracked(77));
    EXPECT_EQ(77, arr[1]->id);
    EXPECT_EQ(1, Tracked::s_live);

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
}

// Deprecated reset(int, T*) overload — verifies delegation works and the
// previous slot occupant is destroyed.
TEST_F(PtrArrayTest, Reset_FromDeprecatedRaw_StillWorks)
{
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

    PtrArray<Tracked> arr;
    arr.emplace(10);
    arr.reset(0, new Tracked(20));
    EXPECT_EQ(20, arr[0]->id);
    EXPECT_EQ(1, Tracked::s_live) << "previous occupant must be destroyed";

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
}

// Bounds checks: set/reset/release/at/remove must throw on out-of-range indices.
// Symmetric coverage of negative and beyond-end indices for every mutator.
TEST_F(PtrArrayTest, BoundsChecks_OutOfRange_Throw)
{
    PtrArray<Tracked> arr;
    arr.expand(2);

    EXPECT_ANY_THROW(arr.set(-1, std::make_unique<Tracked>(1)));
    EXPECT_ANY_THROW(arr.set(5, std::make_unique<Tracked>(2)));
    EXPECT_ANY_THROW(arr.reset(-1));
    EXPECT_ANY_THROW(arr.reset(5));
    EXPECT_ANY_THROW(arr.reset(-1, std::make_unique<Tracked>(3)));
    EXPECT_ANY_THROW(arr.reset(5, std::make_unique<Tracked>(4)));
    EXPECT_ANY_THROW(arr.remove(-1));
    EXPECT_ANY_THROW(arr.remove(5));
    EXPECT_ANY_THROW((void)arr.release(-1));
    EXPECT_ANY_THROW((void)arr.release(5));
    EXPECT_ANY_THROW((void)arr.at(-1));
    EXPECT_ANY_THROW((void)arr.at(5));
    EXPECT_EQ(2, arr.size()) << "size must remain unchanged after rejected ops";
    EXPECT_EQ(0, Tracked::s_live) << "rejected setters must not leak temporaries";
}

// top() on empty array — throws (symmetric with pop()).
TEST_F(PtrArrayTest, Top_OnEmpty_Throws)
{
    PtrArray<Tracked> arr;
    EXPECT_ANY_THROW((void)arr.top());
}
