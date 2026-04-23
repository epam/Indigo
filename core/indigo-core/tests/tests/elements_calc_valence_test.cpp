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
    bool nonStd = false;
    // H with 2 connections and no charge — non-standard valence (permissive model)
    EXPECT_TRUE(Element::calcValence(ELEM_H, 0, 0, 2, valence, hyd, false, &nonStd));
    EXPECT_TRUE(nonStd);
    EXPECT_EQ(hyd, 0);
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
    // One connection with no charge/radical — non-standard valence (permissive model)
    const int alkalineEarth[] = {ELEM_Be, ELEM_Mg, ELEM_Ca, ELEM_Sr, ELEM_Ba, ELEM_Ra};
    for (int elem : alkalineEarth)
    {
        int valence = 0, hyd = 0;
        bool nonStd = false;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 1, valence, hyd, false, &nonStd)) << Element::toString(elem) << " with 1 conn";
        EXPECT_TRUE(nonStd) << Element::toString(elem) << " should be non-standard";
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
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
    bool nonStd = false;
    EXPECT_TRUE(Element::calcValence(ELEM_C, 0, 0, 5, valence, hyd, false, &nonStd));
    EXPECT_TRUE(nonStd);
    EXPECT_EQ(hyd, 0);
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
    // Even connections without radical — non-standard valence for group 7 (permissive)
    const int halogens[] = {ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};
    for (int elem : halogens)
    {
        int valence = 0, hyd = 0;
        bool nonStd = false;
        EXPECT_TRUE(Element::calcValence(elem, 0, 0, 2, valence, hyd, false, &nonStd)) << Element::toString(elem) << " with 2 conn, no radical";
        EXPECT_TRUE(nonStd) << Element::toString(elem) << " should be non-standard";
        EXPECT_EQ(hyd, 0) << Element::toString(elem);
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
