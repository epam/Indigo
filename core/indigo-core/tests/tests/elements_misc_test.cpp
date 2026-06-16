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

#include <cstring>

#include <gtest/gtest.h>

#include <molecule/elements.h>

using namespace indigo;

// ============================================================================
// Test fixture
// ============================================================================

class ElementTest : public ::testing::Test
{
};
// ============================================================================
// 20. calcValenceMinusHyd
// ============================================================================

TEST_F(ElementTest, CalcValenceMinusHyd_Group3_MinusCharge)
{
    const int group3[] = {ELEM_B, ELEM_Al, ELEM_Ga, ELEM_In};
    for (int elem : group3)
    {
        // charge -1, conn <=4 → returns rad + conn
        int result = Element::calcValenceMinusHyd(elem, -1, 0, 3);
        EXPECT_EQ(result, 3) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValenceMinusHyd_Nitrogen_PlusCharge)
{
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_N, 1, 0, 3), 3);
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_N, 2, 0, 2), 2);
}

TEST_F(ElementTest, CalcValenceMinusHyd_Phosphorus_PlusCharge)
{
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_P, 1, 0, 4), 4);
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_P, 2, 0, 3), 3);
}

TEST_F(ElementTest, CalcValenceMinusHyd_AsSbBi_PlusCharge)
{
    const int elems[] = {ELEM_As, ELEM_Sb, ELEM_Bi};
    for (int elem : elems)
    {
        EXPECT_EQ(Element::calcValenceMinusHyd(elem, 1, 0, 3), 3) << Element::toString(elem);
        EXPECT_EQ(Element::calcValenceMinusHyd(elem, 2, 0, 2), 2) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValenceMinusHyd_Oxygen_PlusCharge)
{
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_O, 1, 0, 2), 2);
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_O, 2, 0, 1), 1);
}

TEST_F(ElementTest, CalcValenceMinusHyd_Chalcogens_Charged)
{
    const int chalcogens[] = {ELEM_S, ELEM_Se, ELEM_Po};
    for (int elem : chalcogens)
    {
        EXPECT_EQ(Element::calcValenceMinusHyd(elem, 1, 0, 2), 2) << Element::toString(elem);
        EXPECT_EQ(Element::calcValenceMinusHyd(elem, -1, 0, 1), 1) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValenceMinusHyd_Halogens_PlusCharge)
{
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        EXPECT_EQ(Element::calcValenceMinusHyd(elem, 1, 0, 2), 2) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValenceMinusHyd_DefaultBehavior)
{
    // Elements that fall through use formula: rad + conn + abs(charge)
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_C, 0, 0, 4), 4);
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_C, -1, 0, 3), 4); // 0 + 3 + abs(-1) = 4
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_Fe, 0, 0, 4), 4);
    EXPECT_EQ(Element::calcValenceMinusHyd(ELEM_F, 0, 0, 1), 1);
}

// ============================================================================
// 21. canBeAromatic
// ============================================================================

TEST_F(ElementTest, CanBeAromatic_AllAromaticElements)
{
    // All elements that _initAromatic marks as aromatic-capable (B through At)
    const int aromatic[] = {ELEM_B,  ELEM_C,  ELEM_N,  ELEM_O,  ELEM_F,  ELEM_Al, ELEM_Si, ELEM_P,  ELEM_S,  ELEM_Cl, ELEM_Ga, ELEM_Ge, ELEM_As,
                            ELEM_Se, ELEM_Br, ELEM_In, ELEM_Sn, ELEM_Sb, ELEM_Te, ELEM_I,  ELEM_Tl, ELEM_Pb, ELEM_Bi, ELEM_Po, ELEM_At};
    for (int elem : aromatic)
    {
        EXPECT_TRUE(Element::canBeAromatic(elem)) << Element::toString(elem) << " should be aromatic-capable";
    }
}

TEST_F(ElementTest, CanBeAromatic_NotAromatic)
{
    // Representative non-aromatic elements: H (period 1), noble gases, s-block metals, transition metal
    const int non_aromatic[] = {ELEM_H, ELEM_He, ELEM_Li, ELEM_Be, ELEM_Ne, ELEM_Na, ELEM_Mg, ELEM_Ar, ELEM_Fe, ELEM_Cu, ELEM_Rn};
    for (int elem : non_aromatic)
    {
        EXPECT_FALSE(Element::canBeAromatic(elem)) << Element::toString(elem) << " should not be aromatic-capable";
    }
}

// ============================================================================
// 22. orbitals
// ============================================================================

TEST_F(ElementTest, Orbitals_Group1)
{
    EXPECT_EQ(Element::orbitals(ELEM_H, false), 1);
    EXPECT_EQ(Element::orbitals(ELEM_H, true), 1);
    EXPECT_EQ(Element::orbitals(ELEM_Li, false), 1);
    EXPECT_EQ(Element::orbitals(ELEM_Na, false), 1);
}

TEST_F(ElementTest, Orbitals_Group2)
{
    EXPECT_EQ(Element::orbitals(ELEM_Be, false), 2);
    EXPECT_EQ(Element::orbitals(ELEM_Be, true), 2);
    EXPECT_EQ(Element::orbitals(ELEM_Mg, false), 2);
}

TEST_F(ElementTest, Orbitals_Period2_HigherGroups)
{
    // Period 2, group >= 3: no d orbitals
    EXPECT_EQ(Element::orbitals(ELEM_C, false), 4);
    EXPECT_EQ(Element::orbitals(ELEM_C, true), 4); // Period 2, can't use d
    EXPECT_EQ(Element::orbitals(ELEM_N, false), 4);
    EXPECT_EQ(Element::orbitals(ELEM_O, false), 4);
}

TEST_F(ElementTest, Orbitals_Period3Plus_WithDOrbital)
{
    // Period > 2, group >= 4, use_d_orbital = true → 9
    EXPECT_EQ(Element::orbitals(ELEM_Si, true), 9); // Period 3, group 4
    EXPECT_EQ(Element::orbitals(ELEM_P, true), 9);  // Period 3, group 5
    EXPECT_EQ(Element::orbitals(ELEM_S, true), 9);  // Period 3, group 6

    // Period > 2, group < 4 or use_d_orbital = false → 4
    EXPECT_EQ(Element::orbitals(ELEM_Si, false), 4);
    EXPECT_EQ(Element::orbitals(ELEM_Al, true), 4); // Group 3, not >= 4
}

// ============================================================================
// 23. electrons
// ============================================================================

TEST_F(ElementTest, Electrons_Neutral)
{
    EXPECT_EQ(Element::electrons(ELEM_C, 0), 4);  // Group 4
    EXPECT_EQ(Element::electrons(ELEM_N, 0), 5);  // Group 5
    EXPECT_EQ(Element::electrons(ELEM_O, 0), 6);  // Group 6
    EXPECT_EQ(Element::electrons(ELEM_H, 0), 1);  // Group 1
    EXPECT_EQ(Element::electrons(ELEM_He, 0), 2); // Group 8
}

TEST_F(ElementTest, Electrons_Charged)
{
    EXPECT_EQ(Element::electrons(ELEM_C, 1), 3);  // C+ → group 4 - 1 = 3
    EXPECT_EQ(Element::electrons(ELEM_C, -1), 5); // C- → group 4 - (-1) = 5
    EXPECT_EQ(Element::electrons(ELEM_N, 1), 4);  // N+ → group 5 - 1 = 4
}

// ============================================================================
// 24. getMaximumConnectivity
// ============================================================================

TEST_F(ElementTest, GetMaximumConnectivity_Carbon)
{
    // C neutral, no radical, no d orbitals: electrons=4, orbitals=4 → min(4,2*4-4)=4
    EXPECT_EQ(Element::getMaximumConnectivity(ELEM_C, 0, 0, false), 4);
}

TEST_F(ElementTest, GetMaximumConnectivity_Nitrogen)
{
    // N neutral: electrons=5, orbitals=4 → 2*4-5=3
    EXPECT_EQ(Element::getMaximumConnectivity(ELEM_N, 0, 0, false), 3);
}

TEST_F(ElementTest, GetMaximumConnectivity_NitrogenPlusOne)
{
    // N+: electrons=5-1=4, orbitals=4 → min(4,2*4-4)=4
    EXPECT_EQ(Element::getMaximumConnectivity(ELEM_N, 1, 0, false), 4);
}

TEST_F(ElementTest, GetMaximumConnectivity_Sulfur_WithDOrbital)
{
    // S neutral, use d orbital: electrons=6, orbitals=9 → min(6, 2*9-6)=6
    EXPECT_EQ(Element::getMaximumConnectivity(ELEM_S, 0, 0, true), 6);
}

TEST_F(ElementTest, GetMaximumConnectivity_Sulfur_NoDOrbital)
{
    // S neutral, no d orbital: electrons=6, orbitals=4 → 2*4-6=2
    EXPECT_EQ(Element::getMaximumConnectivity(ELEM_S, 0, 0, false), 2);
}

TEST_F(ElementTest, GetMaximumConnectivity_WithRadical)
{
    // C with doublet radical: rad_electrons=1, electrons=4-1=3, orbitals=4-1=3 → min(3,2*3-3)=3
    EXPECT_EQ(Element::getMaximumConnectivity(ELEM_C, 0, RADICAL_DOUBLET, false), 3);
}

// ============================================================================
// 25. Isotope-related methods
// ============================================================================

TEST_F(ElementTest, GetStandardAtomicWeight_CommonElements)
{
    EXPECT_NEAR(Element::getStandardAtomicWeight(ELEM_H), 1.008, 0.01);
    EXPECT_NEAR(Element::getStandardAtomicWeight(ELEM_C), 12.011, 0.01);
    EXPECT_NEAR(Element::getStandardAtomicWeight(ELEM_N), 14.007, 0.01);
    EXPECT_NEAR(Element::getStandardAtomicWeight(ELEM_O), 15.999, 0.01);
    EXPECT_NEAR(Element::getStandardAtomicWeight(ELEM_Fe), 55.845, 0.01);
}

TEST_F(ElementTest, GetDefaultIsotope_CommonElements)
{
    EXPECT_EQ(Element::getDefaultIsotope(ELEM_H), 1);
    EXPECT_EQ(Element::getDefaultIsotope(ELEM_C), 12);
    EXPECT_EQ(Element::getDefaultIsotope(ELEM_N), 14);
    EXPECT_EQ(Element::getDefaultIsotope(ELEM_O), 16);
    EXPECT_EQ(Element::getDefaultIsotope(ELEM_Fe), 56);
}

TEST_F(ElementTest, GetMostAbundantIsotope_CommonElements)
{
    EXPECT_EQ(Element::getMostAbundantIsotope(ELEM_H), 1);
    EXPECT_EQ(Element::getMostAbundantIsotope(ELEM_C), 12);
    EXPECT_EQ(Element::getMostAbundantIsotope(ELEM_Cl), 35);
}

TEST_F(ElementTest, GetRelativeIsotopicMass)
{
    EXPECT_NEAR(Element::getRelativeIsotopicMass(ELEM_C, 12), 12.0, 0.001);
    EXPECT_NEAR(Element::getRelativeIsotopicMass(ELEM_C, 13), 13.003, 0.01);
    EXPECT_NEAR(Element::getRelativeIsotopicMass(ELEM_H, 2), 2.014, 0.01);
}

TEST_F(ElementTest, GetRelativeIsotopicMass_InvalidThrows)
{
    EXPECT_THROW(Element::getRelativeIsotopicMass(ELEM_C, 999), Exception);
}

TEST_F(ElementTest, GetIsotopicComposition)
{
    double res = 0;
    EXPECT_TRUE(Element::getIsotopicComposition(ELEM_C, 12, res));
    EXPECT_GT(res, 90.0); // ~98.93%

    EXPECT_TRUE(Element::getIsotopicComposition(ELEM_C, 13, res));
    EXPECT_GT(res, 0.5);
    EXPECT_LT(res, 5.0);

    // Non-existent isotope
    EXPECT_FALSE(Element::getIsotopicComposition(ELEM_C, 999, res));
}

TEST_F(ElementTest, GetMinMaxIsotopeIndex)
{
    int min = 0, max = 0;
    Element::getMinMaxIsotopeIndex(ELEM_H, min, max);
    EXPECT_EQ(min, 1);
    EXPECT_GE(max, 3); // At least H, D, T

    Element::getMinMaxIsotopeIndex(ELEM_C, min, max);
    EXPECT_LE(min, 10);
    EXPECT_GE(max, 14);
}

// ============================================================================
// 26. getNumOuterElectrons
// ============================================================================

TEST_F(ElementTest, GetNumOuterElectrons_CommonElements)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_H), 1);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_He), 2);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_C), 4);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_N), 5);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_O), 6);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_F), 7);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Ne), 8);
}

TEST_F(ElementTest, GetNumOuterElectrons_TransitionMetals)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Fe), 8);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Co), 9);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Ni), 10);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Cu), 1);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Zn), 2);
}

TEST_F(ElementTest, GetNumOuterElectrons_Period5)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Rb), 1);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Ag), 1);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Xe), 8);
}

TEST_F(ElementTest, GetNumOuterElectrons_Lanthanides)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Ce), 4);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Pr), 5);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Gd), 10);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Yb), 2); // f14 core
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Lu), 3); // f14 core
}

TEST_F(ElementTest, GetNumOuterElectrons_Period6_dBlock)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Hf), 4);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Au), 1); // d10 core
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Hg), 2);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Rn), 8);
}

TEST_F(ElementTest, GetNumOuterElectrons_Actinides)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Ac), 3);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_U), 6);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_No), 2); // f14 core
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Lr), 3); // f14 core
}

TEST_F(ElementTest, GetNumOuterElectrons_Transactinides)
{
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Rf), 4);
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Rg), 1); // homolog of Au
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Cn), 2); // homolog of Hg
    EXPECT_EQ(Element::getNumOuterElectrons(ELEM_Og), 8);
}

TEST_F(ElementTest, GetNumOuterElectrons_InvalidElementThrows)
{
    EXPECT_THROW(Element::getNumOuterElectrons(-1), Exception);
    EXPECT_THROW(Element::getNumOuterElectrons(ELEM_MAX), Exception);
    EXPECT_THROW(Element::getNumOuterElectrons(999), Exception);
}

// ============================================================================
// 27. ElementHydrogenOnLeft (inline function)
// ============================================================================

TEST_F(ElementTest, ElementHydrogenOnLeft_IncludedElements)
{
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_O));
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_F));
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_S));
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_Cl));
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_Se));
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_Br));
    EXPECT_TRUE(ElementHydrogenOnLeft(ELEM_I));
}

TEST_F(ElementTest, ElementHydrogenOnLeft_ExcludedElements)
{
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_C));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_N));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_P));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_B));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_H));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_Si));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_Fe));
    // Note: At and Te are not included despite being same group as I and Se
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_At));
    EXPECT_FALSE(ElementHydrogenOnLeft(ELEM_Te));
}

// ============================================================================
// 28. Isotope data for all 118 elements
// ============================================================================

TEST_F(ElementTest, AllElementsHaveDefaultIsotope)
{
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        int iso = Element::getDefaultIsotope(i);
        EXPECT_GT(iso, 0) << "Element " << Element::toString(i) << " (Z=" << i << ") has no default isotope";
    }
}

TEST_F(ElementTest, AllElementsHaveStandardAtomicWeight)
{
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        double w = Element::getStandardAtomicWeight(i);
        EXPECT_GT(w, 0.0) << "Element " << Element::toString(i) << " (Z=" << i << ") has zero atomic weight";
    }
}

TEST_F(ElementTest, AllElementsHaveMostAbundantIsotope)
{
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        int iso = Element::getMostAbundantIsotope(i);
        EXPECT_GT(iso, 0) << "Element " << Element::toString(i) << " (Z=" << i << ") has no most abundant isotope";
    }
}

// ============================================================================
// 29. Comprehensive group/period consistency for all elements
// ============================================================================

TEST_F(ElementTest, AllElementsHaveValidGroup)
{
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        int g = Element::group(i);
        EXPECT_GE(g, 1) << "Element " << Element::toString(i) << " has group < 1";
        EXPECT_LE(g, 8) << "Element " << Element::toString(i) << " has group > 8";
    }
}

TEST_F(ElementTest, AllElementsHaveValidPeriod)
{
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        int p = Element::period(i);
        EXPECT_GE(p, 1) << "Element " << Element::toString(i) << " has period < 1";
        EXPECT_LE(p, 7) << "Element " << Element::toString(i) << " has period > 7";
    }
}

// ============================================================================
// 30. Edge cases and boundary tests
// ============================================================================

TEST_F(ElementTest, CalcValence_ZeroConn_ZeroCharge_ZeroRadical_AllGroups)
{
    // Main group representative elements with conn=0, charge=0, radical=0
    struct TestCase
    {
        int elem;
        int expected_valence;
        int expected_hyd;
        bool expected_result;
    };

    TestCase cases[] = {
        {ELEM_H, 1, 1, true}, {ELEM_Li, 1, 1, true}, {ELEM_B, 3, 3, true}, {ELEM_C, 4, 4, true},
        {ELEM_N, 3, 3, true}, {ELEM_O, 2, 2, true},  {ELEM_F, 1, 1, true}, {ELEM_He, 0, 0, true},
    };

    for (const auto& tc : cases)
    {
        int valence = 0, hyd = 0;
        bool result = Element::calcValence(tc.elem, 0, 0, 0, valence, hyd, false);
        EXPECT_EQ(result, tc.expected_result) << "Element: " << Element::toString(tc.elem);
        EXPECT_EQ(valence, tc.expected_valence) << "Element: " << Element::toString(tc.elem);
        EXPECT_EQ(hyd, tc.expected_hyd) << "Element: " << Element::toString(tc.elem);
    }
}

TEST_F(ElementTest, CalcValence_Group4_AllMembers_FourConn)
{
    const int group4[] = {ELEM_C, ELEM_Si, ELEM_Ge, ELEM_Sn, ELEM_Pb};
    for (int elem : group4)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 4, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 4) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Group5_AllMembers_ThreeConn)
{
    const int group5[] = {ELEM_N, ELEM_P, ELEM_As, ELEM_Sb, ELEM_Bi};
    for (int elem : group5)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 3, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 3) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Group6_AllMembers_TwoConn)
{
    const int group6[] = {ELEM_O, ELEM_S, ELEM_Se, ELEM_Te, ELEM_Po};
    for (int elem : group6)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 2, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Group7_AllMembers_OneConn)
{
    const int group7[] = {ELEM_F, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : group7)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 1, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 1) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

// ============================================================================
// 31. Regression: explicit test for documented PubChem compounds
// ============================================================================

TEST_F(ElementTest, CalcValence_Nitrogen_MinusOne)
{
    int valence = 0, hyd = 0;
    // [NH]-  — charge -1, no connections, no radical
    EXPECT_TRUE(Element::calcValence(ELEM_N, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 2); // 3 - 0 - 0 - abs(-1) = 2
}

TEST_F(ElementTest, CalcValence_Phosphorus_PlusTwo)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_P, 2, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Sulfur_HighConnNotNeutral)
{
    int valence = 0, hyd = 0;
    // S+ with 4 connections
    EXPECT_TRUE(Element::calcValence(ELEM_S, 1, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 1);
}

// ============================================================================
// 31b. Missing branch coverage
// ============================================================================

TEST_F(ElementTest, CalcValence_Tellurium_PlusTwo_LowConn)
{
    int valence = 0, hyd = 0;
    // Te(2+) with conn+rad < 4 -> fallback branch: valence=2, hyd=2-conn-rad
    EXPECT_TRUE(Element::calcValence(ELEM_Te, 2, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Halogens_PlusOne_OddConn_BadValence)
{
    // Cl/Br/I/At with charge +1 and conn=3 -> bad valence under hybrid contract.
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        int valence = 0, hyd = 0;
        bool nonStd = false;
        EXPECT_FALSE(Element::calcValence(elem, 1, 0, 3, valence, hyd, false, &nonStd)) << Element::toString(elem);
        EXPECT_TRUE(nonStd) << Element::toString(elem) << " should be non-standard";
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Aluminum_MinusThree)
{
    int valence = 0, hyd = 0;
    // Al(-3) with rad+conn <= 6 -> valence = rad+conn, hyd = 0
    EXPECT_TRUE(Element::calcValence(ELEM_Al, -3, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Thallium_MinusTwo_LowConn)
{
    int valence = 0, hyd = 0;
    // Tl(-2), conn <= 3 -> valence 3
    EXPECT_TRUE(Element::calcValence(ELEM_Tl, -2, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Thallium_MinusTwo_HighConn)
{
    int valence = 0, hyd = 0;
    // Tl(-2), conn > 3 -> valence 5
    EXPECT_TRUE(Element::calcValence(ELEM_Tl, -2, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Phosphorus_MinusOne_ThreeConn_BadValence)
{
    int valence = 0, hyd = 0;
    bool nonStd = false;
    // P(-1) with conn=3: bad valence under hybrid contract.
    EXPECT_FALSE(Element::calcValence(ELEM_P, -1, 0, 3, valence, hyd, false, &nonStd));
    EXPECT_TRUE(nonStd);
    EXPECT_EQ(hyd, 0);
}

// ============================================================================
// 32. Instance / singleton
// ============================================================================

TEST_F(ElementTest, SingletonInstanceStable)
{
    const auto& inst1 = Element::_instance();
    const auto& inst2 = Element::_instance();
    EXPECT_EQ(&inst1, &inst2);
}
