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
// PtrReusablePool<BaseMolecule>, custom family) with focus on RGroup::clear().
//
// Rationale (task #3766): RGroup::clear() was the only existing "reset an
// object in place" method in the affected surface and the closest prototype of
// the Reusable::reuse() contract — yet it had 0 direct test calls. Locking its
// exact reset semantics (all scalar fields zeroed, occurrence emptied,
// fragments pool cleared and reusable) gives the golden master that the
// reset-on-remove reuse reproduces.

#include <gtest/gtest.h>

#include <memory>

#include <molecule/elements.h>
#include <molecule/ket_monomer_shape.h>
#include <molecule/molecule.h>
#include <molecule/molecule_rgroups.h>

using namespace indigo;

// clear() empties a molecule completely, document-level state included: an
// emptied object must be indistinguishable from a fresh one. Otherwise a
// molecule that is cleared and repopulated via clone() inherits the previous
// contents (clone() appends to monomer_shapes and merges properties by source
// key), which is what the pooled hand-back path exposed first.
namespace
{
    void fillResidualState(Molecule& mol)
    {
        mol.addAtom(ELEM_C);
        mol.original_format = BaseMolecule::KET;
        mol.properties().findOrInsert(0); // a per-atom property entry
        mol.monomer_shapes.add(new KetMonomerShape("shape-1", false, "generic", Vec2f(0, 0), {}));
        ASSERT_GT(mol.properties().size(), 0);
        ASSERT_EQ(1, mol.monomer_shapes.size());
    }

    void expectPristine(Molecule& mol)
    {
        EXPECT_EQ(0, mol.vertexCount());
        EXPECT_EQ(BaseMolecule::UNKNOWN, mol.original_format);
        EXPECT_EQ(0, mol.properties().size());
        EXPECT_EQ(0, mol.monomer_shapes.size());
    }
}

TEST(BaseMoleculeReuseContract, ClearWipesResidualState)
{
    Molecule mol;
    fillResidualState(mol);

    mol.clear();

    expectPristine(mol);
}

// The pool hand-back path: reuse() is inherited from Graph and dispatches to
// the most-derived clear(), so a pooled slot is emptied just as completely.
TEST(BaseMoleculeReuseContract, ReuseWipesResidualState)
{
    Molecule mol;
    fillResidualState(mol);

    mol.reuse();

    expectPristine(mol);
}

// The same guarantee through a base-class handle, which is how the pool holds
// its elements: PtrReusablePool<BaseMolecule> calls reuse() on BaseMolecule*.
TEST(BaseMoleculeReuseContract, ReuseThroughBaseHandleWipesResidualState)
{
    Molecule mol;
    fillResidualState(mol);

    BaseMolecule& handle = mol;
    handle.reuse();

    expectPristine(mol);
}

TEST(RGroupContract, ClearResetsAllFields)
{
    RGroup rg;
    rg.if_then = 5;
    rg.rest_h = 3;
    rg.occurrence.push(42);
    rg.fragments.add_t(Molecule::poolFactory());
    rg.fragments.add_t(Molecule::poolFactory());
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
    rg.fragments.add_t(Molecule::poolFactory());
    rg.clear();

    int idx = rg.fragments.add_t(Molecule::poolFactory());
    EXPECT_EQ(0, idx);
    EXPECT_EQ(1, rg.fragments.size());
}
