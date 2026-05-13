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
