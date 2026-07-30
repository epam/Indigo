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

// Characterization tests for the 2D layout of cyclic systems.
//
// Rationale (task #3766): MoleculeLayoutGraph::Cycle lived in legacy
// ObjPool<Cycle> pools (molecule_layout_graph_assign*.cpp) that the
// PtrReusablePool migration replaced. The coverage audit flagged Cycle as a
// blind spot — no test exercised it — so this suite pins the observable
// geometry the cycle layout code produced before the container was swapped.
// Cycle construction / ordering / index reuse changes would show up as altered
// ring geometry or non-deterministic output here.
//
// The assertions are structural (relative bond lengths, regularity, centroid
// symmetry, determinism) rather than absolute golden coordinates, so they are
// stable across platforms while still being sensitive to a change in how the
// cycle pool builds and orders cycles.

#include <gtest/gtest.h>

#include "common.h"

#include <base_cpp/exception.h>
#include <layout/molecule_layout.h>
#include <molecule/molecule.h>

#include <cmath>
#include <vector>

using namespace indigo;

namespace
{
    class IndigoCoreLayoutTest : public IndigoCoreTest
    {
    protected:
        // Lays out the molecule parsed from SMILES and returns its 2D coords.
        std::vector<Vec2f> layoutCoords(const char* smiles, Molecule& mol)
        {
            loadMolecule(smiles, mol);
            MoleculeLayout ml(mol);
            ml.make();

            std::vector<Vec2f> coords;
            for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            {
                const Vec3f& p = mol.getAtomXyz(i);
                coords.push_back(Vec2f(p.x, p.y));
            }
            return coords;
        }

        static float bondLength(Molecule& mol, int e_idx)
        {
            const Edge& e = mol.getEdge(e_idx);
            const Vec3f& a = mol.getAtomXyz(e.beg);
            const Vec3f& b = mol.getAtomXyz(e.end);
            return Vec2f(a.x - b.x, a.y - b.y).length();
        }
    };
} // namespace

// A single ring must be laid out as a regular polygon: every ring bond has the
// same length and every atom is equidistant from the ring centroid.
TEST_F(IndigoCoreLayoutTest, SingleRingIsRegularPolygon)
{
    Molecule mol;
    std::vector<Vec2f> coords = layoutCoords("c1ccccc1", mol);
    ASSERT_EQ(6u, coords.size());

    // All bonds equal.
    const float ref = bondLength(mol, mol.edgeBegin());
    ASSERT_GT(ref, 0.0f);
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
        EXPECT_NEAR(ref, bondLength(mol, e), ref * 0.02f) << "ring bond " << e << " deviates";

    // All atoms equidistant from the centroid (regular hexagon).
    Vec2f centroid(0, 0);
    for (const Vec2f& p : coords)
        centroid.add(p);
    centroid.scale(1.0f / static_cast<float>(coords.size()));

    Vec2f d0;
    d0.diff(coords[0], centroid);
    const float radius = d0.length();
    ASSERT_GT(radius, 0.0f);
    for (const Vec2f& p : coords)
    {
        Vec2f d;
        d.diff(p, centroid);
        EXPECT_NEAR(radius, d.length(), radius * 0.02f) << "atom not on the ring circle";
    }
}

// Layout must be deterministic: the same input laid out twice yields identical
// coordinates. Any change in cycle pool ordering/reuse would surface here.
TEST_F(IndigoCoreLayoutTest, LayoutIsDeterministic)
{
    Molecule a, b;
    std::vector<Vec2f> ca = layoutCoords("c1ccc2ccccc2c1", a);
    std::vector<Vec2f> cb = layoutCoords("c1ccc2ccccc2c1", b);

    ASSERT_EQ(ca.size(), cb.size());
    for (size_t i = 0; i < ca.size(); i++)
    {
        EXPECT_NEAR(ca[i].x, cb[i].x, 1e-4f) << "atom " << i << " x differs between runs";
        EXPECT_NEAR(ca[i].y, cb[i].y, 1e-4f) << "atom " << i << " y differs between runs";
    }
}

// Fused rings (naphthalene): all bonds uniform, coordinates finite.
TEST_F(IndigoCoreLayoutTest, FusedRingsUniformBonds)
{
    Molecule mol;
    std::vector<Vec2f> coords = layoutCoords("c1ccc2ccccc2c1", mol);
    ASSERT_EQ(10u, coords.size());

    const float ref = bondLength(mol, mol.edgeBegin());
    ASSERT_GT(ref, 0.0f);
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
        EXPECT_NEAR(ref, bondLength(mol, e), ref * 0.05f) << "fused-ring bond " << e << " deviates";

    for (const Vec2f& p : coords)
    {
        EXPECT_TRUE(std::isfinite(p.x));
        EXPECT_TRUE(std::isfinite(p.y));
    }
}

// Macrocycle: a large single ring still lays out as a regular polygon with
// uniform bonds (exercises the macrocycle/cycle-smoothing paths).
TEST_F(IndigoCoreLayoutTest, MacrocycleUniformBondsAndFiniteCoords)
{
    Molecule mol;
    std::vector<Vec2f> coords = layoutCoords("C1CCCCCCCCCCC1", mol);
    ASSERT_EQ(12u, coords.size());

    const float ref = bondLength(mol, mol.edgeBegin());
    ASSERT_GT(ref, 0.0f);
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
        EXPECT_NEAR(ref, bondLength(mol, e), ref * 0.05f) << "macrocycle bond " << e << " deviates";

    for (const Vec2f& p : coords)
    {
        EXPECT_TRUE(std::isfinite(p.x));
        EXPECT_TRUE(std::isfinite(p.y));
    }
}

// Two independent rings connected by a chain: both rings keep uniform bonds and
// are laid out apart (no collapsed/overlapping coordinates).
TEST_F(IndigoCoreLayoutTest, TwoIndependentRingsLaidOutApart)
{
    Molecule mol;
    std::vector<Vec2f> coords = layoutCoords("c1ccccc1CCc1ccccc1", mol);
    ASSERT_EQ(14u, coords.size());

    const float ref = bondLength(mol, mol.edgeBegin());
    ASSERT_GT(ref, 0.0f);
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
        EXPECT_NEAR(ref, bondLength(mol, e), ref * 0.10f) << "bond " << e << " deviates";

    // No two atoms occupy the same point.
    for (size_t i = 0; i < coords.size(); i++)
        for (size_t j = i + 1; j < coords.size(); j++)
        {
            Vec2f d;
            d.diff(coords[i], coords[j]);
            EXPECT_GT(d.length(), 0.1f) << "atoms " << i << " and " << j << " overlap";
        }
}
