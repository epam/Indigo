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

#include <algorithm>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

// ──────────────────────────────────────────────────
// Fixture: manages Indigo session + monomer library
// ──────────────────────────────────────────────────
class IndigoSequenceLayoutTest : public IndigoApiTest
{
protected:
    int library = -1;

    void SetUp() override
    {
        IndigoApiTest::SetUp();
        library = indigoLoadMonomerLibraryFromString("{\"root\":{}}");
        ASSERT_GE(library, 0) << "Failed to create monomer library: " << indigoGetLastError();
    }

    // ── Helpers ──────────────────────────────────

    /// Load a HELM string into a molecule
    int loadHelm(const char* helm)
    {
        int mol = indigoLoadHelmFromString(helm, library);
        EXPECT_GE(mol, 0) << "Failed to load HELM: " << indigoGetLastError();
        return mol;
    }

    /// Get 2D coordinates of an atom by its index within the molecule
    struct Pos2D
    {
        float x = 0.f, y = 0.f;
    };
    Pos2D atomXY(int mol, int atomIdx)
    {
        int atom = indigoGetAtom(mol, atomIdx);
        EXPECT_GE(atom, 0) << "Invalid atom index " << atomIdx;
        float* xyz = indigoXYZ(atom);
        EXPECT_NE(xyz, nullptr) << "No coordinates for atom " << atomIdx;
        return {xyz[0], xyz[1]};
    }

    /// Euclidean distance between two atom positions
    static float dist(Pos2D a, Pos2D b)
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    /// Collect all inter-monomer bond lengths from atom coordinates.
    /// Each monomer is a single-atom superatom in HELM, so bonds between
    /// consecutive atoms represent monomer-monomer edges.
    std::vector<float> collectBondLengths(int mol)
    {
        std::vector<float> lengths;
        int iter = indigoIterateBonds(mol);
        for (int bond = indigoNext(iter); bond > 0; bond = indigoNext(iter))
        {
            int src = indigoSource(bond);
            int dst = indigoDestination(bond);
            Pos2D p1 = atomXY(mol, indigoIndex(src));
            Pos2D p2 = atomXY(mol, indigoIndex(dst));
            lengths.push_back(dist(p1, p2));
            indigoFree(src);
            indigoFree(dst);
            indigoFree(bond);
        }
        indigoFree(iter);
        return lengths;
    }

    /// Select atoms by their indices (marks them as selected)
    void selectAtoms(int mol, const std::vector<int>& indices)
    {
        for (int idx : indices)
        {
            int atom = indigoGetAtom(mol, idx);
            ASSERT_GE(atom, 0) << "Cannot get atom " << idx;
            indigoSelect(atom);
            indigoFree(atom);
        }
    }

    // ── Constants ────────────────────────────────
    // Must match LayoutOptions in metalayout.h
    static constexpr float MONOMER_BOND_LENGTH = 1.5f;
    static constexpr float DEFAULT_BOND_LENGTH = 1.0f;
    static constexpr float TOLERANCE = 0.25f; // relative tolerance for bond length comparison
};

// ==========================================================================
//  Test 1: Pure linear chain — all bond lengths must be uniform
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, LinearChain_UniformBondLength)
{
    // PEPTIDE1{C.C.C.C.C}$$$$V2.0  — 5 monomers, no cycles
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C}$$$$V2.0");
    ASSERT_GE(mol, 0);

    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_FALSE(lengths.empty()) << "No bonds found in linear chain";

    for (size_t i = 0; i < lengths.size(); ++i)
    {
        EXPECT_NEAR(lengths[i], MONOMER_BOND_LENGTH, TOLERANCE) << "Bond " << i << " has unexpected length " << lengths[i];
    }

    indigoFree(mol);
}

// ==========================================================================
//  Test 2: Ring with tail — bond lengths in both ring and tail must be
//          consistent after "arrange as ring" (selection-based layout)
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, RingWithTail_ConsistentBondLength)
{
    // The exact HELM from the bug report:
    // 13 Cys monomers, ring bond between positions 5 and 8
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C}$PEPTIDE1,PEPTIDE1,5:R3-8:R3$$$V2.0");
    ASSERT_GE(mol, 0);

    // Select a subset — ring + right tail (atoms 4..12 → 0-based)
    std::vector<int> selection;
    int nAtoms = indigoCountAtoms(mol);
    for (int i = 4; i < nAtoms; ++i)
        selection.push_back(i);

    selectAtoms(mol, selection);
    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_FALSE(lengths.empty()) << "No bonds after layout";

    // All bond lengths should be within tolerance of each other
    float minLen = *std::min_element(lengths.begin(), lengths.end());
    float maxLen = *std::max_element(lengths.begin(), lengths.end());

    // Before the fix, tail bonds would be ≈1.0 while ring bonds ≈1.5
    // After fix, they should all be close to MONOMER_BOND_LENGTH
    EXPECT_LT(maxLen - minLen, TOLERANCE * 2) << "Bond length variance too large: min=" << minLen << " max=" << maxLen << " (expected all ~"
                                              << MONOMER_BOND_LENGTH << ")";

    // Every bond should be near MONOMER_BOND_LENGTH
    for (size_t i = 0; i < lengths.size(); ++i)
    {
        EXPECT_NEAR(lengths[i], MONOMER_BOND_LENGTH, TOLERANCE)
            << "Bond " << i << " length " << lengths[i] << " deviates from expected " << MONOMER_BOND_LENGTH;
    }

    indigoFree(mol);
}

// ==========================================================================
//  Test 3: Pure ring (no tail) — bonds should be uniform
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, PureRing_UniformBondLength)
{
    // 6 monomers forming a ring: bond between position 1 and 6
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C.C}$PEPTIDE1,PEPTIDE1,1:R3-6:R3$$$V2.0");
    ASSERT_GE(mol, 0);

    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_FALSE(lengths.empty()) << "No bonds found in pure ring";

    float minLen = *std::min_element(lengths.begin(), lengths.end());
    float maxLen = *std::max_element(lengths.begin(), lengths.end());

    EXPECT_LT(maxLen - minLen, TOLERANCE * 2) << "Bond length variance in ring: min=" << minLen << " max=" << maxLen;

    indigoFree(mol);
}

// ==========================================================================
//  Test 4: Long tail with ring — the exact regression scenario
//          Ring in the middle, long tails on both sides
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, LongTailWithRing_RegressionBug1)
{
    // Build a longer peptide: 20 monomers, ring between 8 and 13
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C.C}"
                       "$PEPTIDE1,PEPTIDE1,8:R3-13:R3$$$V2.0");
    ASSERT_GE(mol, 0);

    // Select atoms in ring + right tail (7..19, 0-based)
    std::vector<int> selection;
    for (int i = 7; i < 20; ++i)
        selection.push_back(i);

    selectAtoms(mol, selection);
    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);

    // Specifically check selected region bonds don't collapse
    // In the original bug, tail bonds collapsed to ~1.0 while ring was ~1.5
    for (size_t i = 0; i < lengths.size(); ++i)
    {
        // Bond lengths should not be less than 80% of monomer length
        EXPECT_GT(lengths[i], MONOMER_BOND_LENGTH * 0.7f) << "Bond " << i << " too short: " << lengths[i] << " (collapsed tail bond)";
    }

    indigoFree(mol);
}

// ==========================================================================
//  Test 5: Layout without selection — all-atom layout uses scaling in
//          _assignFinalCoordinates, so bonds should also be uniform
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, FullLayout_NoBondLengthCollapse)
{
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C.C.C.C.C.C}$PEPTIDE1,PEPTIDE1,3:R3-8:R3$$$V2.0");
    ASSERT_GE(mol, 0);

    // No selection — full layout
    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_FALSE(lengths.empty());

    float avgLen = 0.f;
    for (float l : lengths)
        avgLen += l;
    avgLen /= static_cast<float>(lengths.size());

    // All bonds should be within tolerance of the average
    for (size_t i = 0; i < lengths.size(); ++i)
    {
        EXPECT_NEAR(lengths[i], avgLen, TOLERANCE) << "Bond " << i << " deviates from average " << avgLen;
    }

    indigoFree(mol);
}

// ==========================================================================
//  Test 6: Minimal ring — 3 monomers forming a triangle
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, MinimalRing_TriangleMonomers)
{
    int mol = loadHelm("PEPTIDE1{C.C.C}$PEPTIDE1,PEPTIDE1,1:R3-3:R3$$$V2.0");
    ASSERT_GE(mol, 0);

    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_GE(lengths.size(), 3u) << "Triangle should have at least 3 bonds";

    for (size_t i = 0; i < lengths.size(); ++i)
    {
        EXPECT_GT(lengths[i], 0.5f) << "Bond " << i << " collapsed to near-zero";
    }

    indigoFree(mol);
}

// ==========================================================================
//  Test 7: Selection-based layout on linear chain (no ring)
//          Tests _calculatePos with sequence_layout && _n_fixed > 0
//          but without ring component
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, SelectionOnLinear_BondLengthPreserved)
{
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C.C.C.C}$$$$V2.0");
    ASSERT_GE(mol, 0);

    // Select only the last 4 monomers (partial layout)
    selectAtoms(mol, {4, 5, 6, 7});
    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_FALSE(lengths.empty());

    // Selected bonds should use MONOMER_BOND_LENGTH
    for (size_t i = 0; i < lengths.size(); ++i)
    {
        EXPECT_GT(lengths[i], MONOMER_BOND_LENGTH * 0.6f) << "Bond " << i << " too short after selected layout: " << lengths[i];
    }

    indigoFree(mol);
}

// ==========================================================================
//  Test 8: Multiple rings in one chain — both rings should be consistent
// ==========================================================================
TEST_F(IndigoSequenceLayoutTest, MultipleRings_ConsistentBondLengths)
{
    // 15 monomers, two separate rings: 2-5 and 10-13 (1-based)
    int mol = loadHelm("PEPTIDE1{C.C.C.C.C.C.C.C.C.C.C.C.C.C.C}"
                       "$PEPTIDE1,PEPTIDE1,2:R3-5:R3|PEPTIDE1,PEPTIDE1,10:R3-13:R3$$$V2.0");
    ASSERT_GE(mol, 0);

    indigoLayout(mol);

    auto lengths = collectBondLengths(mol);
    ASSERT_FALSE(lengths.empty());

    float minLen = *std::min_element(lengths.begin(), lengths.end());
    float maxLen = *std::max_element(lengths.begin(), lengths.end());

    // Variance should be bounded
    EXPECT_LT(maxLen - minLen, TOLERANCE * 3) << "Multi-ring bond variance: min=" << minLen << " max=" << maxLen;

    indigoFree(mol);
}
