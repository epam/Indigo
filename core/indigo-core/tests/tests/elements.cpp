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
// 1. Element enum completeness — all 118 elements present
// ============================================================================

TEST_F(ElementTest, EnumCoversAll118Elements)
{
    // ELEM_H = 1, ELEM_Og = 118, ELEM_MAX = 119
    EXPECT_EQ(ELEM_H, 1);
    EXPECT_EQ(ELEM_MAX, 119);
    // 118 elements: H(1) through Og(118)
    EXPECT_EQ(ELEM_MAX - ELEM_MIN, 118);
}

TEST_F(ElementTest, SpecialEnumValues)
{
    EXPECT_GT(ELEM_PSEUDO, ELEM_MAX);
    EXPECT_GT(ELEM_RSITE, ELEM_PSEUDO);
    EXPECT_GT(ELEM_TEMPLATE, ELEM_RSITE);
    EXPECT_GT(ELEM_ATTPOINT, ELEM_TEMPLATE);
    EXPECT_GT(ELEM_ATOMLIST, ELEM_ATTPOINT);
}

// ============================================================================
// 2. toString / fromString — round-trip for every element
// ============================================================================

TEST_F(ElementTest, ToStringFromStringRoundTrip)
{
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        const char* name = Element::toString(i);
        ASSERT_NE(name, nullptr) << "toString returned null for element " << i;
        EXPECT_GT(strlen(name), 0u) << "toString returned empty for element " << i;

        int recovered = Element::fromString(name);
        EXPECT_EQ(recovered, i) << "Round-trip failed for element " << i << " (name: " << name << ")";
    }
}

TEST_F(ElementTest, FromString2ReturnsMinusOneForUnknown)
{
    EXPECT_EQ(Element::fromString2("Zz"), -1);
    EXPECT_EQ(Element::fromString2("Qq"), -1);
    EXPECT_EQ(Element::fromString2("Xx"), -1);
    EXPECT_EQ(Element::fromString2(""), -1);
}

TEST_F(ElementTest, FromStringThrowsForUnknown)
{
    EXPECT_THROW(Element::fromString("Zz"), Exception);
    EXPECT_THROW(Element::fromString("Xx"), Exception);
}

TEST_F(ElementTest, FromStringKnownElements)
{
    EXPECT_EQ(Element::fromString("H"), ELEM_H);
    EXPECT_EQ(Element::fromString("C"), ELEM_C);
    EXPECT_EQ(Element::fromString("N"), ELEM_N);
    EXPECT_EQ(Element::fromString("O"), ELEM_O);
    EXPECT_EQ(Element::fromString("Fe"), ELEM_Fe);
    EXPECT_EQ(Element::fromString("Og"), ELEM_Og);
}

// ============================================================================
// 3. fromChar / fromTwoChars
// ============================================================================

TEST_F(ElementTest, FromCharSingleLetterElements)
{
    EXPECT_EQ(Element::fromChar('H'), ELEM_H);
    EXPECT_EQ(Element::fromChar('C'), ELEM_C);
    EXPECT_EQ(Element::fromChar('N'), ELEM_N);
    EXPECT_EQ(Element::fromChar('O'), ELEM_O);
    EXPECT_EQ(Element::fromChar('F'), ELEM_F);
    EXPECT_EQ(Element::fromChar('P'), ELEM_P);
    EXPECT_EQ(Element::fromChar('S'), ELEM_S);
    EXPECT_EQ(Element::fromChar('B'), ELEM_B);
    EXPECT_EQ(Element::fromChar('I'), ELEM_I);
    EXPECT_EQ(Element::fromChar('K'), ELEM_K);
    EXPECT_EQ(Element::fromChar('U'), ELEM_U);
    EXPECT_EQ(Element::fromChar('V'), ELEM_V);
    EXPECT_EQ(Element::fromChar('W'), ELEM_W);
    EXPECT_EQ(Element::fromChar('Y'), ELEM_Y);
}

TEST_F(ElementTest, FromTwoCharsKnownElements)
{
    EXPECT_EQ(Element::fromTwoChars('H', 'e'), ELEM_He);
    EXPECT_EQ(Element::fromTwoChars('L', 'i'), ELEM_Li);
    EXPECT_EQ(Element::fromTwoChars('N', 'a'), ELEM_Na);
    EXPECT_EQ(Element::fromTwoChars('C', 'l'), ELEM_Cl);
    EXPECT_EQ(Element::fromTwoChars('B', 'r'), ELEM_Br);
    EXPECT_EQ(Element::fromTwoChars('F', 'e'), ELEM_Fe);
    EXPECT_EQ(Element::fromTwoChars('O', 'g'), ELEM_Og);
}

TEST_F(ElementTest, FromTwoChars2ReturnsMinusOneForUnknown)
{
    EXPECT_EQ(Element::fromTwoChars2('Z', 'z'), -1);
    EXPECT_EQ(Element::fromTwoChars2('Q', 'q'), -1);
}

TEST_F(ElementTest, FromTwoChars2IntOverloadNegativeReturnsMinus1)
{
    EXPECT_EQ(Element::fromTwoChars2('C', -1), -1);
}

// ============================================================================
// 4. toString with isotopes (D, T)
// ============================================================================

TEST_F(ElementTest, ToStringWithIsotope)
{
    EXPECT_STREQ(Element::toString(ELEM_H, DEUTERIUM), "D");
    EXPECT_STREQ(Element::toString(ELEM_H, TRITIUM), "T");
    EXPECT_STREQ(Element::toString(ELEM_H, HYDROGEN), "H");
    // Non-hydrogen with isotope returns element name
    EXPECT_STREQ(Element::toString(ELEM_C, 12), "C");
    EXPECT_STREQ(Element::toString(ELEM_C, 13), "C");
}

TEST_F(ElementTest, ToStringOutOfRangeThrows)
{
    EXPECT_THROW(Element::toString(-1), Exception);
    EXPECT_THROW(Element::toString(ELEM_MAX + 1), Exception);
}

// ============================================================================
// 5. group() and period() — all 118 elements (data-driven)
// ============================================================================

TEST_F(ElementTest, GroupPeriod_AllElements)
{
    struct Expected
    {
        int elem;
        int group;
        int period;
    };

    // clang-format off
    static const Expected kExpected[] = {
        // Period 1
        {ELEM_H, 1, 1}, {ELEM_He, 8, 1},
        // Period 2
        {ELEM_Li, 1, 2}, {ELEM_Be, 2, 2}, {ELEM_B, 3, 2}, {ELEM_C, 4, 2},
        {ELEM_N, 5, 2}, {ELEM_O, 6, 2}, {ELEM_F, 7, 2}, {ELEM_Ne, 8, 2},
        // Period 3
        {ELEM_Na, 1, 3}, {ELEM_Mg, 2, 3}, {ELEM_Al, 3, 3}, {ELEM_Si, 4, 3},
        {ELEM_P, 5, 3}, {ELEM_S, 6, 3}, {ELEM_Cl, 7, 3}, {ELEM_Ar, 8, 3},
        // Period 4
        {ELEM_K, 1, 4}, {ELEM_Ca, 2, 4}, {ELEM_Sc, 3, 4}, {ELEM_Ti, 4, 4},
        {ELEM_V, 5, 4}, {ELEM_Cr, 6, 4}, {ELEM_Mn, 7, 4}, {ELEM_Fe, 8, 4},
        {ELEM_Co, 8, 4}, {ELEM_Ni, 8, 4}, {ELEM_Cu, 1, 4}, {ELEM_Zn, 2, 4},
        {ELEM_Ga, 3, 4}, {ELEM_Ge, 4, 4}, {ELEM_As, 5, 4}, {ELEM_Se, 6, 4},
        {ELEM_Br, 7, 4}, {ELEM_Kr, 8, 4},
        // Period 5
        {ELEM_Rb, 1, 5}, {ELEM_Sr, 2, 5}, {ELEM_Y, 3, 5}, {ELEM_Zr, 4, 5},
        {ELEM_Nb, 5, 5}, {ELEM_Mo, 6, 5}, {ELEM_Tc, 7, 5}, {ELEM_Ru, 8, 5},
        {ELEM_Rh, 8, 5}, {ELEM_Pd, 8, 5}, {ELEM_Ag, 1, 5}, {ELEM_Cd, 2, 5},
        {ELEM_In, 3, 5}, {ELEM_Sn, 4, 5}, {ELEM_Sb, 5, 5}, {ELEM_Te, 6, 5},
        {ELEM_I, 7, 5}, {ELEM_Xe, 8, 5},
        // Period 6: s-block
        {ELEM_Cs, 1, 6}, {ELEM_Ba, 2, 6},
        // Period 6: lanthanides (all group 3)
        {ELEM_La, 3, 6}, {ELEM_Ce, 3, 6}, {ELEM_Pr, 3, 6}, {ELEM_Nd, 3, 6},
        {ELEM_Pm, 3, 6}, {ELEM_Sm, 3, 6}, {ELEM_Eu, 3, 6}, {ELEM_Gd, 3, 6},
        {ELEM_Tb, 3, 6}, {ELEM_Dy, 3, 6}, {ELEM_Ho, 3, 6}, {ELEM_Er, 3, 6},
        {ELEM_Tm, 3, 6}, {ELEM_Yb, 3, 6}, {ELEM_Lu, 3, 6},
        // Period 6: d-block and p-block
        {ELEM_Hf, 4, 6}, {ELEM_Ta, 5, 6}, {ELEM_W, 6, 6}, {ELEM_Re, 7, 6},
        {ELEM_Os, 8, 6}, {ELEM_Ir, 8, 6}, {ELEM_Pt, 8, 6},
        {ELEM_Au, 1, 6}, {ELEM_Hg, 2, 6},
        {ELEM_Tl, 3, 6}, {ELEM_Pb, 4, 6}, {ELEM_Bi, 5, 6}, {ELEM_Po, 6, 6},
        {ELEM_At, 7, 6}, {ELEM_Rn, 8, 6},
        // Period 7: s-block
        {ELEM_Fr, 1, 7}, {ELEM_Ra, 2, 7},
        // Period 7: actinides (all group 3)
        {ELEM_Ac, 3, 7}, {ELEM_Th, 3, 7}, {ELEM_Pa, 3, 7}, {ELEM_U, 3, 7},
        {ELEM_Np, 3, 7}, {ELEM_Pu, 3, 7}, {ELEM_Am, 3, 7}, {ELEM_Cm, 3, 7},
        {ELEM_Bk, 3, 7}, {ELEM_Cf, 3, 7}, {ELEM_Es, 3, 7}, {ELEM_Fm, 3, 7},
        {ELEM_Md, 3, 7}, {ELEM_No, 3, 7}, {ELEM_Lr, 3, 7},
        // Period 7: d-block (transactinides) and p-block
        {ELEM_Rf, 4, 7}, {ELEM_Db, 5, 7}, {ELEM_Sg, 6, 7}, {ELEM_Bh, 7, 7},
        {ELEM_Hs, 8, 7}, {ELEM_Mt, 8, 7}, {ELEM_Ds, 8, 7},
        {ELEM_Rg, 1, 7}, {ELEM_Cn, 2, 7},
        {ELEM_Nh, 3, 7}, {ELEM_Fl, 4, 7}, {ELEM_Mc, 5, 7}, {ELEM_Lv, 6, 7},
        {ELEM_Ts, 7, 7}, {ELEM_Og, 8, 7},
    };
    // clang-format on

    for (const auto& e : kExpected)
    {
        SCOPED_TRACE(Element::toString(e.elem));
        EXPECT_EQ(Element::group(e.elem), e.group);
        EXPECT_EQ(Element::period(e.elem), e.period);
    }
}

// ============================================================================
// 6. isHalogen
// ============================================================================

TEST_F(ElementTest, IsHalogenTrue)
{
    EXPECT_TRUE(Element::isHalogen(ELEM_F));
    EXPECT_TRUE(Element::isHalogen(ELEM_Cl));
    EXPECT_TRUE(Element::isHalogen(ELEM_Br));
    EXPECT_TRUE(Element::isHalogen(ELEM_I));
    EXPECT_TRUE(Element::isHalogen(ELEM_At));
}

TEST_F(ElementTest, IsHalogenFalse)
{
    EXPECT_FALSE(Element::isHalogen(ELEM_H));
    EXPECT_FALSE(Element::isHalogen(ELEM_C));
    EXPECT_FALSE(Element::isHalogen(ELEM_N));
    EXPECT_FALSE(Element::isHalogen(ELEM_O));
    EXPECT_FALSE(Element::isHalogen(ELEM_S));
    EXPECT_FALSE(Element::isHalogen(ELEM_He));
    EXPECT_FALSE(Element::isHalogen(ELEM_Ts)); // Tennessine — not in list
}

// ============================================================================
// 7. radicalElectrons / radicalOrbitals
// ============================================================================

TEST_F(ElementTest, RadicalElectrons)
{
    EXPECT_EQ(Element::radicalElectrons(0), 0);
    EXPECT_EQ(Element::radicalElectrons(RADICAL_SINGLET), 2);
    EXPECT_EQ(Element::radicalElectrons(RADICAL_DOUBLET), 1);
    EXPECT_EQ(Element::radicalElectrons(RADICAL_TRIPLET), 2);
}

TEST_F(ElementTest, RadicalOrbitals)
{
    EXPECT_EQ(Element::radicalOrbitals(0), 0);
    EXPECT_EQ(Element::radicalOrbitals(RADICAL_SINGLET), 1);
    EXPECT_EQ(Element::radicalOrbitals(RADICAL_DOUBLET), 1);
    EXPECT_EQ(Element::radicalOrbitals(RADICAL_TRIPLET), 1);
}

// ============================================================================
// 8. calcValence — Group 1 (alkali metals + H)
// ============================================================================

TEST_F(ElementTest, CalcValence_Hydrogen_Neutral_NoConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_H, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 1); // H2 molecule
}

TEST_F(ElementTest, CalcValence_Hydrogen_Neutral_OneConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_H, 0, 0, 1, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Hydrogen_PlusCharge)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_H, 1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 0); // H+ has no implicit H
}

TEST_F(ElementTest, CalcValence_Hydrogen_MinusCharge)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_H, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 0); // H- hydride
}

TEST_F(ElementTest, CalcValence_Hydrogen_BadValence)
{
    int valence = 0, hyd = 0;
    // H with 2 connections and no charge — bad valence
    EXPECT_FALSE(Element::calcValence(ELEM_H, 0, 0, 2, valence, hyd, false));
}

TEST_F(ElementTest, CalcValence_Hydrogen_BadValence_Throws)
{
    int valence = 0, hyd = 0;
    EXPECT_THROW(Element::calcValence(ELEM_H, 0, 0, 2, valence, hyd, true), Exception);
}

TEST_F(ElementTest, CalcValence_AlkaliMetals_Neutral)
{
    const int alkalis[] = {ELEM_Li, ELEM_Na, ELEM_K, ELEM_Rb, ELEM_Cs, ELEM_Fr};
    for (int elem : alkalis)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 0, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 1) << Element::toString(elem);
        EXPECT_EQ(hyd, 1) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_AlkaliMetals_OneConn)
{
    const int alkalis[] = {ELEM_Li, ELEM_Na, ELEM_K, ELEM_Rb, ELEM_Cs, ELEM_Fr};
    for (int elem : alkalis)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 1, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 1) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

// ============================================================================
// 9. calcValence — Group 2 (alkaline earth metals)
// ============================================================================

TEST_F(ElementTest, CalcValence_AlkalineEarth_Neutral_NoConn)
{
    const int alkalineEarth[] = {ELEM_Be, ELEM_Mg, ELEM_Ca, ELEM_Sr, ELEM_Ba, ELEM_Ra};
    for (int elem : alkalineEarth)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 0, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem); // hyd=2 then set to -1 → false, but actually hyd != 0 → -1
    }
}

TEST_F(ElementTest, CalcValence_AlkalineEarth_TwoConn)
{
    const int alkalineEarth[] = {ELEM_Be, ELEM_Mg, ELEM_Ca, ELEM_Sr, ELEM_Ba, ELEM_Ra};
    for (int elem : alkalineEarth)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 2, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_AlkalineEarth_OneConn_BadValence)
{
    // One connection with no charge/radical → hyd = 2 - 1 = 1: but then hyd != 0 → hyd = -1 → bad
    const int alkalineEarth[] = {ELEM_Be, ELEM_Mg, ELEM_Ca, ELEM_Sr, ELEM_Ba, ELEM_Ra};
    for (int elem : alkalineEarth)
    {
        int valence = 0, hyd = 0;
        EXPECT_FALSE(Element::calcValence(elem, 0, 0, 1, valence, hyd, false)) << "Expected bad valence for " << Element::toString(elem) << " with 1 conn";
    }
}

// ============================================================================
// 10. calcValence — Group 3 (B, Al, Ga, In, Tl)
// ============================================================================

TEST_F(ElementTest, CalcValence_Boron_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_B, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3); // BH3
}

TEST_F(ElementTest, CalcValence_Boron_ThreeConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_B, 0, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Boron_MinusOne)
{
    int valence = 0, hyd = 0;
    // [BH4]- : tetrahedral borohydride
    EXPECT_TRUE(Element::calcValence(ELEM_B, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 4);
}

TEST_F(ElementTest, CalcValence_Aluminum_MinusTwo_FiveConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Al, -2, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Group3Elements_Neutral)
{
    const int group3[] = {ELEM_B, ELEM_Al, ELEM_Ga, ELEM_In};
    for (int elem : group3)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 3, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 3) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Thallium_Neutral_LowConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Tl, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 1);
}

TEST_F(ElementTest, CalcValence_Thallium_Neutral_ThreeConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Tl, 0, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Thallium_MinusOne)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Tl, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Thallium_MinusThree)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Tl, -3, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

// ============================================================================
// 11. calcValence — Group 4 (C, Si, Ge, Sn, Pb)
// ============================================================================

TEST_F(ElementTest, CalcValence_Carbon_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_C, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 4); // CH4
}

TEST_F(ElementTest, CalcValence_Carbon_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_C, 0, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Carbon_Charged)
{
    int valence = 0, hyd = 0;
    // C- : carbanion with 3 implicit H
    EXPECT_TRUE(Element::calcValence(ELEM_C, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 3);

    // C+ : carbocation with 3 implicit H
    EXPECT_TRUE(Element::calcValence(ELEM_C, 1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Carbon_TooManyBonds)
{
    int valence = 0, hyd = 0;
    EXPECT_FALSE(Element::calcValence(ELEM_C, 0, 0, 5, valence, hyd, false));
}

TEST_F(ElementTest, CalcValence_Silicon_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Si, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 4); // SiH4
}

TEST_F(ElementTest, CalcValence_Silicon_MinusTwo_SixConn)
{
    int valence = 0, hyd = 0;
    // Hexafluorosilicate [SiF6]2-
    EXPECT_TRUE(Element::calcValence(ELEM_Si, -2, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Silicon_MinusTwo_FiveConn)
{
    int valence = 0, hyd = 0;
    // Pentafluorosilicate [SiF5]2-
    EXPECT_TRUE(Element::calcValence(ELEM_Si, -2, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Silicon_MinusOne_FiveConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Si, -1, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Silicon_MinusOne_FourConn)
{
    int valence = 0, hyd = 0;
    // CID 438107
    EXPECT_TRUE(Element::calcValence(ELEM_Si, -1, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 1);
}

TEST_F(ElementTest, CalcValence_Tin_LowConn)
{
    int valence = 0, hyd = 0;
    // [SnH2]
    EXPECT_TRUE(Element::calcValence(ELEM_Sn, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Lead_LowConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Pb, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Germanium_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Ge, 0, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

// ============================================================================
// 12. calcValence — Group 5 (N, P, As, Sb, Bi)
// ============================================================================

TEST_F(ElementTest, CalcValence_Nitrogen_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_N, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3); // NH3
}

TEST_F(ElementTest, CalcValence_Nitrogen_ThreeConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_N, 0, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Nitrogen_PlusOne)
{
    int valence = 0, hyd = 0;
    // [NH4]+ ammonium
    EXPECT_TRUE(Element::calcValence(ELEM_N, 1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 4);
}

TEST_F(ElementTest, CalcValence_Nitrogen_PlusOne_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_N, 1, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Phosphorus_Neutral_ThreeConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_P, 0, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Phosphorus_Neutral_FiveConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_P, 0, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Phosphorus_MinusOne)
{
    int valence = 0, hyd = 0;
    // Phosphanide: [PH2]-
    EXPECT_TRUE(Element::calcValence(ELEM_P, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Phosphorus_MinusOne_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_P, -1, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Phosphorus_MinusOne_SixConn)
{
    int valence = 0, hyd = 0;
    // Hexachlorophosphate
    EXPECT_TRUE(Element::calcValence(ELEM_P, -1, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Arsenic_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_As, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Arsenic_FiveConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_As, 0, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Antimony_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Sb, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Bismuth_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Bi, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Bismuth_MinusOne_SixConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Bi, -1, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Bismuth_MinusTwo_FiveConn)
{
    int valence = 0, hyd = 0;
    // CID 45158489
    EXPECT_TRUE(Element::calcValence(ELEM_Bi, -2, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Bismuth_PlusOne)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Bi, 1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Arsenic_PlusOne)
{
    int valence = 0, hyd = 0;
    // As+: unlike Sb/Bi, As+ goes directly to valence 4
    EXPECT_TRUE(Element::calcValence(ELEM_As, 1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 4);
}

// ============================================================================
// 13. calcValence — Group 6 (O, S, Se, Te, Po)
// ============================================================================

TEST_F(ElementTest, CalcValence_Oxygen_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_O, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2); // H2O
}

TEST_F(ElementTest, CalcValence_Oxygen_TwoConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_O, 0, 0, 2, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Oxygen_PlusCharge)
{
    int valence = 0, hyd = 0;
    // Oxonium: [OH3]+
    EXPECT_TRUE(Element::calcValence(ELEM_O, 1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Oxygen_MinusCharge)
{
    int valence = 0, hyd = 0;
    // Hydroxide: [OH]-
    EXPECT_TRUE(Element::calcValence(ELEM_O, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 1);
}

TEST_F(ElementTest, CalcValence_Sulfur_Neutral_TwoConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_S, 0, 0, 2, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Sulfur_Neutral_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_S, 0, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Sulfur_Neutral_SixConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_S, 0, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Sulfur_PlusOne)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_S, 1, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Sulfur_MinusOne)
{
    int valence = 0, hyd = 0;
    // Thiolate [SH]-
    EXPECT_TRUE(Element::calcValence(ELEM_S, -1, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 1);
}

TEST_F(ElementTest, CalcValence_Selenium_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Se, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Tellurium_Neutral_NoConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Te, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Tellurium_Neutral_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Te, 0, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Tellurium_Neutral_SixConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Te, 0, 0, 6, valence, hyd, false));
    EXPECT_EQ(valence, 6);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Tellurium_MinusOne_SevenConn)
{
    int valence = 0, hyd = 0;
    // CID 4191414
    EXPECT_TRUE(Element::calcValence(ELEM_Te, -1, 0, 7, valence, hyd, false));
    EXPECT_EQ(valence, 7);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Tellurium_MinusOne_FiveConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Te, -1, 0, 5, valence, hyd, false));
    EXPECT_EQ(valence, 5);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Tellurium_PlusOne)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Te, 1, 0, 3, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Tellurium_PlusTwo_FourConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Te, 2, 0, 4, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Polonium_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Po, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 2);
    EXPECT_EQ(hyd, 2);
}

// ============================================================================
// 14. calcValence — Group 7 (F, Cl, Br, I, At)
// ============================================================================

TEST_F(ElementTest, CalcValence_Fluorine_Neutral)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_F, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 1); // HF
}

TEST_F(ElementTest, CalcValence_Fluorine_OneConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_F, 0, 0, 1, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Chlorine_Neutral_NoConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Cl, 0, 0, 0, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 1); // HCl
}

TEST_F(ElementTest, CalcValence_Chlorine_Neutral_OneConn)
{
    int valence = 0, hyd = 0;
    EXPECT_TRUE(Element::calcValence(ELEM_Cl, 0, 0, 1, valence, hyd, false));
    EXPECT_EQ(valence, 1);
    EXPECT_EQ(hyd, 0);
}

TEST_F(ElementTest, CalcValence_Halogens_PlusOne_TwoConn)
{
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 1, 0, 2, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Halogens_Neutral_OddConnWithRadical)
{
    // Halogens with 2 connections and radical (e.g. ClO radical)
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, RADICAL_DOUBLET, 2, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Halogens_EvenConn_NoRadical_BadValence)
{
    // Even connections without radical → bad valence for group 7
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        int valence = 0, hyd = 0;
        EXPECT_FALSE(Element::calcValence(elem, 0, 0, 2, valence, hyd, false))
            << "Expected bad valence for " << Element::toString(elem) << " with 2 conn, no radical";
    }
}

// ============================================================================
// 15. calcValence — Group 8 (noble gases)
// ============================================================================

TEST_F(ElementTest, CalcValence_NobleGases)
{
    const int nobles[] = {ELEM_He, ELEM_Ne, ELEM_Ar, ELEM_Kr, ELEM_Xe, ELEM_Rn, ELEM_Og};
    for (int elem : nobles)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 0, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 0) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

// ============================================================================
// 16. calcValence — Transition metals (fall-through behavior)
// ============================================================================

TEST_F(ElementTest, CalcValence_TransitionMetals_FallThrough)
{
    // Transition metals not explicitly handled: valence = conn, hyd = 0
    const int metals[] = {ELEM_Sc, ELEM_Ti, ELEM_V,  ELEM_Cr, ELEM_Mn, ELEM_Fe, ELEM_Co, ELEM_Ni, ELEM_Y,  ELEM_Zr, ELEM_Nb, ELEM_Mo,
                          ELEM_Tc, ELEM_Ru, ELEM_Rh, ELEM_Pd, ELEM_Hf, ELEM_Ta, ELEM_W,  ELEM_Re, ELEM_Os, ELEM_Ir, ELEM_Pt};
    for (int elem : metals)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 4, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 4) << Element::toString(elem) << " should pass through with valence=conn";
        EXPECT_EQ(hyd, 0) << Element::toString(elem) << " should have no implicit H";
    }
}

TEST_F(ElementTest, CalcValence_CoinageMetals_GroupOneFallThrough)
{
    // Cu, Ag, Au are group 1 in the simplified scheme but NOT alkali metals.
    // They should fall through the group 1 branch without setting valence/hyd
    const int coinage[] = {ELEM_Cu, ELEM_Ag, ELEM_Au};
    for (int elem : coinage)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 2, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Group12Metals_GroupTwoFallThrough)
{
    // Zn, Cd, Hg are group 2 in simplified scheme but NOT alkaline earth.
    const int group12[] = {ELEM_Zn, ELEM_Cd, ELEM_Hg};
    for (int elem : group12)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 2, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 2) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

// ============================================================================
// 17. calcValence — Lanthanides and Actinides (fall-through)
// ============================================================================

TEST_F(ElementTest, CalcValence_Lanthanides_FallThrough)
{
    const int lanthanides[] = {ELEM_La, ELEM_Ce, ELEM_Pr, ELEM_Nd, ELEM_Pm, ELEM_Sm, ELEM_Eu, ELEM_Gd,
                               ELEM_Tb, ELEM_Dy, ELEM_Ho, ELEM_Er, ELEM_Tm, ELEM_Yb, ELEM_Lu};
    for (int elem : lanthanides)
    {
        int valence = 0, hyd = 0;
        // Not B/Al/Ga/In/Tl in the group 3 branch → fall through
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 3, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 3) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

TEST_F(ElementTest, CalcValence_Actinides_FallThrough)
{
    const int actinides[] = {ELEM_Ac, ELEM_Th, ELEM_Pa, ELEM_U,  ELEM_Np, ELEM_Pu, ELEM_Am, ELEM_Cm,
                             ELEM_Bk, ELEM_Cf, ELEM_Es, ELEM_Fm, ELEM_Md, ELEM_No, ELEM_Lr};
    for (int elem : actinides)
    {
        int valence = 0, hyd = 0;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 3, valence, hyd, false)) << "Failed for " << Element::toString(elem);
        EXPECT_EQ(valence, 3) << Element::toString(elem);
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
    }
}

// ============================================================================
// 18. calcValence — with radicals
// ============================================================================

TEST_F(ElementTest, CalcValence_Carbon_Doublet)
{
    int valence = 0, hyd = 0;
    // Methyl radical: [CH3]•
    EXPECT_TRUE(Element::calcValence(ELEM_C, 0, RADICAL_DOUBLET, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 3);
}

TEST_F(ElementTest, CalcValence_Carbon_Triplet)
{
    int valence = 0, hyd = 0;
    // Methylene triplet: [CH2]••
    EXPECT_TRUE(Element::calcValence(ELEM_C, 0, RADICAL_TRIPLET, 0, valence, hyd, false));
    EXPECT_EQ(valence, 4);
    EXPECT_EQ(hyd, 2);
}

TEST_F(ElementTest, CalcValence_Nitrogen_Doublet)
{
    int valence = 0, hyd = 0;
    // [NH2]• amino radical
    EXPECT_TRUE(Element::calcValence(ELEM_N, 0, RADICAL_DOUBLET, 0, valence, hyd, false));
    EXPECT_EQ(valence, 3);
    EXPECT_EQ(hyd, 2);
}

// ============================================================================
// 19. calcValenceOfAromaticAtom
// ============================================================================

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Carbon)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_C, 0, 2, 2), 4);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_C, 1, 2, 2), 4);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Nitrogen)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_N, 0, 2, 2), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_N, 1, 2, 2), 4);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Oxygen)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_O, 0, 2, 2), 2);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_O, 1, 2, 2), 3);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Sulfur_Charge0)
{
    // 2 aromatic bonds, no external bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 2, 2), 2);
    // 2 aromatic bonds, one external bond
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 2, 3), 4);
    // 2 aromatic bonds, two external bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 2, 4), 4);
    // 2 aromatic bonds, >4 min_conn
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 2, 5), 6);
    // 3 aromatic bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 3, 3), 4);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 3, 4), 4);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 3, 5), 6);
    // 4 aromatic bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 4, 4), 4);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 0, 4, 5), 6);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Sulfur_Charge1)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 1, 2, 2), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 1, 2, 3), 5);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_S, 1, 2, 4), 5);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Phosphorus_Charge0)
{
    // 2 aromatic bonds, no external bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 0, 2, 2), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 0, 2, 3), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 0, 2, 4), 5);
    // 3 aromatic bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 0, 3, 3), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 0, 3, 5), 5);
    // 4 aromatic bonds
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 0, 4, 4), 5);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Phosphorus_PlusOne)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, 1, 2, 3), 4);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Phosphorus_MinusOne)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_P, -1, 2, 2), 2);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Selenium)
{
    // Charge 0
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Se, 0, 2, 2), 2);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Se, 0, 2, 3), 4);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Se, 0, 2, 4), 4);
    // Charge +1
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Se, 1, 2, 2), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Se, 1, 2, 3), 3);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Arsenic)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_As, 0, 2, 2), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_As, 0, 2, 3), 3);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Tellurium_Charge0)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Te, 0, 2, 2), 3);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Te, 0, 2, 4), 4);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Te, 0, 4, 4), 4);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Tellurium_Charge1)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Te, 1, 2, 3), 3);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Boron)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_B, 0, 2, 3), 3);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_Silicon)
{
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Si, 0, 2, 3), 4);
}

TEST_F(ElementTest, CalcValenceOfAromaticAtom_UnhandledReturnsMinus1)
{
    // Elements that are aromatic-capable but not handled
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Al, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Ga, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Ge, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_In, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Sn, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Sb, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Tl, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Pb, 0, 2, 2), -1);
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Bi, 0, 2, 2), -1);
    // Non-aromatic element
    EXPECT_EQ(Element::calcValenceOfAromaticAtom(ELEM_Fe, 0, 2, 2), -1);
}

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
    EXPECT_EQ(Element::electrons(ELEM_He, 0), 8); // Group 8
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
// 27. ElementHygrodenOnLeft (inline function)
// ============================================================================

TEST_F(ElementTest, ElementHydrogenOnLeft_IncludedElements)
{
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_O));
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_F));
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_S));
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_Cl));
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_Se));
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_Br));
    EXPECT_TRUE(ElementHygrodenOnLeft(ELEM_I));
}

TEST_F(ElementTest, ElementHydrogenOnLeft_ExcludedElements)
{
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_C));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_N));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_P));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_B));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_H));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_Si));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_Fe));
    // Note: At and Te are not included despite being same group as I and Se
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_At));
    EXPECT_FALSE(ElementHygrodenOnLeft(ELEM_Te));
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
    // Cl/Br/I/At with charge +1 and conn=3 -> hyd=-1 -> bad valence
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        int valence = 0, hyd = 0;
        EXPECT_FALSE(Element::calcValence(elem, 1, 0, 3, valence, hyd, false)) << Element::toString(elem);
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
    // P(-1) with conn=3: no known examples with hydrogen -> hyd=-1 -> bad valence
    EXPECT_FALSE(Element::calcValence(ELEM_P, -1, 0, 3, valence, hyd, false));
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
