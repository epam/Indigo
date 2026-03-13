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

// Parameterized tests for Element::calcValence — unified valence ladder formula.
// Covers all 8 periodic groups with systematic element×charge×radical×conn combinations.
// Reference: .github/specs/3539/VALENCE_REFACTORING_ANALYSIS.md

#include <gtest/gtest.h>
#include <string>

#include "molecule/elements.h"

using namespace indigo;

struct ValenceTestCase
{
    int elem;
    int charge;
    int radical;
    int conn;
    bool expect_valid;
    int expect_hyd; // checked only when expect_valid == true
    const char* label;
};

class CalcValenceTest : public ::testing::TestWithParam<ValenceTestCase>
{
};

TEST_P(CalcValenceTest, VerifyValenceResult)
{
    const auto& tc = GetParam();
    int valence = 0, hyd = 0;
    bool result = Element::calcValence(tc.elem, tc.charge, tc.radical, tc.conn, valence, hyd, false);
    EXPECT_EQ(tc.expect_valid, result) << "  case: " << tc.label;
    if (tc.expect_valid && result)
    {
        EXPECT_EQ(tc.expect_hyd, hyd) << "  case: " << tc.label << " (hyd mismatch)";
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Group 1 — Hydrogen
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group1_H, CalcValenceTest,
                         ::testing::Values(
                             // Elemental hydrogen: Biovia convention H₂
                             ValenceTestCase{ELEM_H, 0, 0, 0, true, 1, "H_q0_r0_c0"}, ValenceTestCase{ELEM_H, 0, 0, 1, true, 0, "H_q0_r0_c1"},
                             ValenceTestCase{ELEM_H, 0, 0, 2, false, 0, "H_q0_r0_c2_INVALID"},
                             // H+ proton (bare ion)
                             ValenceTestCase{ELEM_H, 1, 0, 0, true, 0, "H_q+1_r0_c0_proton"},
                             // H- hydride (bare ion)
                             ValenceTestCase{ELEM_H, -1, 0, 0, true, 0, "H_q-1_r0_c0_hydride"},
                             // H radical
                             ValenceTestCase{ELEM_H, 0, RADICAL_DOUBLET, 0, true, 0, "H_q0_rD_c0_radical"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 1 — Alkali metals (Li, Na, K, Rb, Cs, Fr)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group1_Alkali, CalcValenceTest,
                         ::testing::Values(
                             // Li standard
                             ValenceTestCase{ELEM_Li, 0, 0, 0, true, 1, "Li_q0_r0_c0"}, ValenceTestCase{ELEM_Li, 0, 0, 1, true, 0, "Li_q0_r0_c1"},
                             // Li+ bare ion
                             ValenceTestCase{ELEM_Li, 1, 0, 0, true, 0, "Li_q+1_r0_c0_ion"},
                             // Na- bare ion
                             ValenceTestCase{ELEM_Na, -1, 0, 0, true, 0, "Na_q-1_r0_c0_natride"},
                             // Na standard
                             ValenceTestCase{ELEM_Na, 0, 0, 1, true, 0, "Na_q0_r0_c1"},
                             // K standard
                             ValenceTestCase{ELEM_K, 0, 0, 0, true, 1, "K_q0_r0_c0"}, ValenceTestCase{ELEM_K, 0, 0, 1, true, 0, "K_q0_r0_c1"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 2 — Alkaline earth metals (Be, Mg, Ca, Sr, Ba, Ra)
//   Rule B: metal_no_h — all implicit H forbidden
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group2, CalcValenceTest,
                         ::testing::Values(
                             // Mg standard: conn=2 ok, hyd always killed
                             ValenceTestCase{ELEM_Mg, 0, 0, 2, true, 0, "Mg_q0_r0_c2"}, ValenceTestCase{ELEM_Mg, 0, 0, 0, true, 0, "Mg_q0_r0_c0"},
                             ValenceTestCase{ELEM_Mg, 0, 0, 1, false, 0, "Mg_q0_r0_c1_INVALID"},
                             // Be
                             ValenceTestCase{ELEM_Be, 0, 0, 2, true, 0, "Be_q0_r0_c2"}, ValenceTestCase{ELEM_Be, 0, 0, 0, true, 0, "Be_q0_r0_c0"},
                             // Ca charged
                             ValenceTestCase{ELEM_Ca, 0, 0, 2, true, 0, "Ca_q0_r0_c2"}, ValenceTestCase{ELEM_Ca, 1, 0, 1, false, 0, "Ca_q+1_r0_c1_INVALID"},
                             ValenceTestCase{ELEM_Ca, 2, 0, 0, true, 0, "Ca_q+2_r0_c0"},
                             // Ba
                             ValenceTestCase{ELEM_Ba, 0, 0, 2, true, 0, "Ba_q0_r0_c2"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 3 — Boron
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group3_B, CalcValenceTest,
                         ::testing::Values(
                             // B standard: val=3
                             ValenceTestCase{ELEM_B, 0, 0, 0, true, 3, "B_q0_r0_c0"}, ValenceTestCase{ELEM_B, 0, 0, 3, true, 0, "B_q0_r0_c3"},
                             ValenceTestCase{ELEM_B, 0, 0, 4, false, 0, "B_q0_r0_c4_INVALID"},
                             // B- : val=4 (BH4-)
                             ValenceTestCase{ELEM_B, -1, 0, 0, true, 4, "B_q-1_r0_c0"}, ValenceTestCase{ELEM_B, -1, 0, 4, true, 0, "B_q-1_r0_c4"},
                             // B+
                             ValenceTestCase{ELEM_B, 1, 0, 0, true, 2, "B_q+1_r0_c0"}, ValenceTestCase{ELEM_B, 1, 0, 2, true, 0, "B_q+1_r0_c2"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 3 — Al, Ga, In
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(
    Group3_Al, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Al, 0, 0, 0, true, 3, "Al_q0_r0_c0"}, ValenceTestCase{ELEM_Al, 0, 0, 3, true, 0, "Al_q0_r0_c3"},
                      ValenceTestCase{ELEM_Al, -1, 0, 0, true, 4, "Al_q-1_r0_c0"}, ValenceTestCase{ELEM_Al, -1, 0, 4, true, 0, "Al_q-1_r0_c4"},
                      ValenceTestCase{ELEM_Al, -2, 0, 5, true, 0, "Al_q-2_r0_c5"}, ValenceTestCase{ELEM_Al, -3, 0, 6, true, 0, "Al_q-3_r0_c6"},
                      // Ga
                      ValenceTestCase{ELEM_Ga, 0, 0, 3, true, 0, "Ga_q0_r0_c3"}, ValenceTestCase{ELEM_Ga, -1, 0, 4, true, 0, "Ga_q-1_r0_c4"},
                      // In
                      ValenceTestCase{ELEM_In, 0, 0, 3, true, 0, "In_q0_r0_c3"}, ValenceTestCase{ELEM_In, -1, 0, 4, true, 0, "In_q-1_r0_c4"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 3 — Tl (inert pair: val=1 default)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group3_Tl, CalcValenceTest,
                         ::testing::Values(
                             // Tl neutral: inert pair → val=1
                             ValenceTestCase{ELEM_Tl, 0, 0, 0, true, 1, "Tl_q0_r0_c0"}, ValenceTestCase{ELEM_Tl, 0, 0, 1, true, 0, "Tl_q0_r0_c1"},
                             ValenceTestCase{ELEM_Tl, 0, 0, 3, true, 0, "Tl_q0_r0_c3"},
                             // Tl- : inert pair → val=2
                             ValenceTestCase{ELEM_Tl, -1, 0, 0, true, 2, "Tl_q-1_r0_c0"}, ValenceTestCase{ELEM_Tl, -1, 0, 2, true, 0, "Tl_q-1_r0_c2"},
                             ValenceTestCase{ELEM_Tl, -1, 0, 4, true, 0, "Tl_q-1_r0_c4"},
                             // Tl2-
                             ValenceTestCase{ELEM_Tl, -2, 0, 3, true, 0, "Tl_q-2_r0_c3"}, ValenceTestCase{ELEM_Tl, -2, 0, 5, true, 0, "Tl_q-2_r0_c5"},
                             // Tl3-: octahedral (ISIS/Marvin)
                             ValenceTestCase{ELEM_Tl, -3, 0, 6, true, 0, "Tl_q-3_r0_c6"},
                             // Tl+
                             ValenceTestCase{ELEM_Tl, 1, 0, 0, true, 0, "Tl_q+1_r0_c0"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 4 — Carbon (100% formula match)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group4_C, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_C, 0, 0, 0, true, 4, "C_q0_r0_c0_CH4"}, ValenceTestCase{ELEM_C, 0, 0, 1, true, 3, "C_q0_r0_c1"},
                                           ValenceTestCase{ELEM_C, 0, 0, 2, true, 2, "C_q0_r0_c2"}, ValenceTestCase{ELEM_C, 0, 0, 3, true, 1, "C_q0_r0_c3"},
                                           ValenceTestCase{ELEM_C, 0, 0, 4, true, 0, "C_q0_r0_c4"},
                                           ValenceTestCase{ELEM_C, 0, 0, 5, false, 0, "C_q0_r0_c5_INVALID"},
                                           // C+ carbocation
                                           ValenceTestCase{ELEM_C, 1, 0, 0, true, 3, "C_q+1_r0_c0"}, ValenceTestCase{ELEM_C, 1, 0, 3, true, 0, "C_q+1_r0_c3"},
                                           // C- carbanion
                                           ValenceTestCase{ELEM_C, -1, 0, 0, true, 3, "C_q-1_r0_c0"}, ValenceTestCase{ELEM_C, -1, 0, 3, true, 0, "C_q-1_r0_c3"},
                                           // Radicals
                                           ValenceTestCase{ELEM_C, 0, RADICAL_DOUBLET, 0, true, 3, "C_q0_rD_c0"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_DOUBLET, 3, true, 0, "C_q0_rD_c3"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_TRIPLET, 0, true, 2, "C_q0_rT_c0"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_TRIPLET, 2, true, 0, "C_q0_rT_c2"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 4 — Si, Ge (period ≥ 3, d-orbital expansion)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group4_SiGe, CalcValenceTest,
                         ::testing::Values(
                             // Si standard
                             ValenceTestCase{ELEM_Si, 0, 0, 0, true, 4, "Si_q0_r0_c0_SiH4"}, ValenceTestCase{ELEM_Si, 0, 0, 4, true, 0, "Si_q0_r0_c4"},
                             ValenceTestCase{ELEM_Si, 0, 0, 5, false, 0, "Si_q0_r0_c5_INVALID"},
                             // Si- hypervalent
                             ValenceTestCase{ELEM_Si, -1, 0, 0, true, 3, "Si_q-1_r0_c0"}, ValenceTestCase{ELEM_Si, -1, 0, 4, true, 1, "Si_q-1_r0_c4"},
                             ValenceTestCase{ELEM_Si, -1, 0, 5, true, 0, "Si_q-1_r0_c5"},
                             // Si2- — the original bug fix: SiF5(2-)
                             ValenceTestCase{ELEM_Si, -2, 0, 5, true, 1, "Si_q-2_r0_c5_SiF5"}, ValenceTestCase{ELEM_Si, -2, 0, 6, true, 0, "Si_q-2_r0_c6_SiF6"},
                             // IMPROVED: Si2- conn=3,4 — ladder (min hybridization) instead of fill-to-max
                             ValenceTestCase{ELEM_Si, -2, 0, 3, true, 1, "Si_q-2_r0_c3_ladder"},
                             ValenceTestCase{ELEM_Si, -2, 0, 4, true, 0, "Si_q-2_r0_c4_ladder"},
                             // Ge
                             ValenceTestCase{ELEM_Ge, 0, 0, 0, true, 4, "Ge_q0_r0_c0"}, ValenceTestCase{ELEM_Ge, 0, 0, 4, true, 0, "Ge_q0_r0_c4"},
                             ValenceTestCase{ELEM_Ge, -1, 0, 5, true, 0, "Ge_q-1_r0_c5"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 4 — Sn, Pb (inert pair effect)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group4_SnPb, CalcValenceTest,
                         ::testing::Values(
                             // Sn: inert pair → base val=2, then 4
                             ValenceTestCase{ELEM_Sn, 0, 0, 0, true, 2, "Sn_q0_r0_c0_SnH2"}, ValenceTestCase{ELEM_Sn, 0, 0, 2, true, 0, "Sn_q0_r0_c2"},
                             ValenceTestCase{ELEM_Sn, 0, 0, 4, true, 0, "Sn_q0_r0_c4"}, ValenceTestCase{ELEM_Sn, 0, 0, 5, false, 0, "Sn_q0_r0_c5_INVALID"},
                             // Pb: inert pair
                             ValenceTestCase{ELEM_Pb, 0, 0, 0, true, 2, "Pb_q0_r0_c0_PbH2"}, ValenceTestCase{ELEM_Pb, 0, 0, 2, true, 0, "Pb_q0_r0_c2"},
                             ValenceTestCase{ELEM_Pb, 0, 0, 4, true, 0, "Pb_q0_r0_c4"},
                             // Sn- hypervalent
                             ValenceTestCase{ELEM_Sn, -1, 0, 5, true, 0, "Sn_q-1_r0_c5"},
                             // Sn2- hypervalent
                             ValenceTestCase{ELEM_Sn, -2, 0, 6, true, 0, "Sn_q-2_r0_c6"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 5 — Nitrogen (100% formula match)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group5_N, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_N, 0, 0, 0, true, 3, "N_q0_r0_c0_NH3"}, ValenceTestCase{ELEM_N, 0, 0, 1, true, 2, "N_q0_r0_c1"},
                                           ValenceTestCase{ELEM_N, 0, 0, 2, true, 1, "N_q0_r0_c2"}, ValenceTestCase{ELEM_N, 0, 0, 3, true, 0, "N_q0_r0_c3"},
                                           ValenceTestCase{ELEM_N, 0, 0, 4, false, 0, "N_q0_r0_c4_INVALID"},
                                           // N+ ammonium
                                           ValenceTestCase{ELEM_N, 1, 0, 0, true, 4, "N_q+1_r0_c0_NH4+"},
                                           ValenceTestCase{ELEM_N, 1, 0, 4, true, 0, "N_q+1_r0_c4"},
                                           // N2+
                                           ValenceTestCase{ELEM_N, 2, 0, 0, true, 3, "N_q+2_r0_c0"}, ValenceTestCase{ELEM_N, 2, 0, 3, true, 0, "N_q+2_r0_c3"},
                                           // N- amine anion
                                           ValenceTestCase{ELEM_N, -1, 0, 0, true, 2, "N_q-1_r0_c0"}, ValenceTestCase{ELEM_N, -1, 0, 2, true, 0, "N_q-1_r0_c2"},
                                           // N radical
                                           ValenceTestCase{ELEM_N, 0, RADICAL_DOUBLET, 0, true, 2, "N_q0_rD_c0"},
                                           ValenceTestCase{ELEM_N, 0, RADICAL_DOUBLET, 2, true, 0, "N_q0_rD_c2"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 5 — Phosphorus
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group5_P, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_P, 0, 0, 0, true, 3, "P_q0_r0_c0_PH3"}, ValenceTestCase{ELEM_P, 0, 0, 3, true, 0, "P_q0_r0_c3"},
                                           ValenceTestCase{ELEM_P, 0, 0, 5, true, 0, "P_q0_r0_c5_PF5"}, ValenceTestCase{ELEM_P, 0, 0, 4, true, 1, "P_q0_r0_c4"},
                                           // P+ : val=4
                                           ValenceTestCase{ELEM_P, 1, 0, 0, true, 4, "P_q+1_r0_c0"}, ValenceTestCase{ELEM_P, 1, 0, 4, true, 0, "P_q+1_r0_c4"},
                                           // P2+
                                           ValenceTestCase{ELEM_P, 2, 0, 0, true, 3, "P_q+2_r0_c0"}, ValenceTestCase{ELEM_P, 2, 0, 3, true, 0, "P_q+2_r0_c3"},
                                           // P- phosphanide
                                           ValenceTestCase{ELEM_P, -1, 0, 0, true, 2, "P_q-1_r0_c0"}, ValenceTestCase{ELEM_P, -1, 0, 2, true, 0, "P_q-1_r0_c2"},
                                           ValenceTestCase{ELEM_P, -1, 0, 4, true, 0, "P_q-1_r0_c4"},
                                           ValenceTestCase{ELEM_P, -1, 0, 6, true, 0, "P_q-1_r0_c6"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 5 — As, Sb, Bi (pnictogens)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group5_AsSbBi, CalcValenceTest,
                         ::testing::Values(
                             // As standard
                             ValenceTestCase{ELEM_As, 0, 0, 0, true, 3, "As_q0_r0_c0"}, ValenceTestCase{ELEM_As, 0, 0, 3, true, 0, "As_q0_r0_c3"},
                             ValenceTestCase{ELEM_As, 0, 0, 5, true, 0, "As_q0_r0_c5"},
                             // Sb: inert pair → Sb+: val=2
                             ValenceTestCase{ELEM_Sb, 0, 0, 0, true, 3, "Sb_q0_r0_c0"}, ValenceTestCase{ELEM_Sb, 0, 0, 3, true, 0, "Sb_q0_r0_c3"},
                             ValenceTestCase{ELEM_Sb, 0, 0, 5, true, 0, "Sb_q0_r0_c5"}, ValenceTestCase{ELEM_Sb, 1, 0, 0, true, 2, "Sb_q+1_r0_c0_inert"},
                             ValenceTestCase{ELEM_Sb, 1, 0, 2, true, 0, "Sb_q+1_r0_c2"}, ValenceTestCase{ELEM_Sb, 1, 0, 4, true, 0, "Sb_q+1_r0_c4"},
                             ValenceTestCase{ELEM_Sb, 2, 0, 3, true, 0, "Sb_q+2_r0_c3"}, ValenceTestCase{ELEM_Sb, -1, 0, 6, true, 0, "Sb_q-1_r0_c6"},
                             ValenceTestCase{ELEM_Sb, -2, 0, 5, true, 0, "Sb_q-2_r0_c5"},
                             // Bi: inert pair
                             ValenceTestCase{ELEM_Bi, 0, 0, 0, true, 3, "Bi_q0_r0_c0"}, ValenceTestCase{ELEM_Bi, 0, 0, 3, true, 0, "Bi_q0_r0_c3"},
                             ValenceTestCase{ELEM_Bi, 0, 0, 5, true, 0, "Bi_q0_r0_c5"}, ValenceTestCase{ELEM_Bi, 1, 0, 0, true, 2, "Bi_q+1_r0_c0_inert"},
                             ValenceTestCase{ELEM_Bi, 1, 0, 2, true, 0, "Bi_q+1_r0_c2"}, ValenceTestCase{ELEM_Bi, -2, 0, 5, true, 0, "Bi_q-2_r0_c5"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 6 — Oxygen
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group6_O, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_O, 0, 0, 0, true, 2, "O_q0_r0_c0_H2O"}, ValenceTestCase{ELEM_O, 0, 0, 1, true, 1, "O_q0_r0_c1"},
                                           ValenceTestCase{ELEM_O, 0, 0, 2, true, 0, "O_q0_r0_c2"},
                                           ValenceTestCase{ELEM_O, 0, 0, 3, false, 0, "O_q0_r0_c3_INVALID"},
                                           // O-
                                           ValenceTestCase{ELEM_O, -1, 0, 0, true, 1, "O_q-1_r0_c0"}, ValenceTestCase{ELEM_O, -1, 0, 1, true, 0, "O_q-1_r0_c1"},
                                           // O+
                                           ValenceTestCase{ELEM_O, 1, 0, 0, true, 3, "O_q+1_r0_c0"}, ValenceTestCase{ELEM_O, 1, 0, 3, true, 0, "O_q+1_r0_c3"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 6 — S, Se, Po (chalcogens, valence ladder [2,4,6])
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group6_SSeP, CalcValenceTest,
                         ::testing::Values(
                             // S standard: ladder [2,4,6]
                             ValenceTestCase{ELEM_S, 0, 0, 0, true, 2, "S_q0_r0_c0_H2S"}, ValenceTestCase{ELEM_S, 0, 0, 2, true, 0, "S_q0_r0_c2"},
                             ValenceTestCase{ELEM_S, 0, 0, 3, true, 1, "S_q0_r0_c3"}, ValenceTestCase{ELEM_S, 0, 0, 4, true, 0, "S_q0_r0_c4"},
                             ValenceTestCase{ELEM_S, 0, 0, 6, true, 0, "S_q0_r0_c6_SF6"},
                             // S+ : eff=5, ladder [3,5]
                             ValenceTestCase{ELEM_S, 1, 0, 0, true, 3, "S_q+1_r0_c0"}, ValenceTestCase{ELEM_S, 1, 0, 3, true, 0, "S_q+1_r0_c3"},
                             ValenceTestCase{ELEM_S, 1, 0, 5, true, 0, "S_q+1_r0_c5"},
                             // S- : eff=7, ladder [1,3,5,7]
                             ValenceTestCase{ELEM_S, -1, 0, 0, true, 1, "S_q-1_r0_c0"}, ValenceTestCase{ELEM_S, -1, 0, 1, true, 0, "S_q-1_r0_c1"},
                             ValenceTestCase{ELEM_S, -1, 0, 3, true, 0, "S_q-1_r0_c3"}, ValenceTestCase{ELEM_S, -1, 0, 5, true, 0, "S_q-1_r0_c5"},
                             ValenceTestCase{ELEM_S, -1, 0, 7, true, 0, "S_q-1_r0_c7"},
                             // Se
                             ValenceTestCase{ELEM_Se, 0, 0, 0, true, 2, "Se_q0_r0_c0"}, ValenceTestCase{ELEM_Se, 0, 0, 2, true, 0, "Se_q0_r0_c2"},
                             ValenceTestCase{ELEM_Se, 0, 0, 4, true, 0, "Se_q0_r0_c4"}, ValenceTestCase{ELEM_Se, 0, 0, 6, true, 0, "Se_q0_r0_c6"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 6 — Te (IMPROVED: fixes over-restrictive code)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group6_Te, CalcValenceTest,
                         ::testing::Values(
                             // Te neutral: same ladder as S [2,4,6]
                             ValenceTestCase{ELEM_Te, 0, 0, 0, true, 2, "Te_q0_r0_c0"}, ValenceTestCase{ELEM_Te, 0, 0, 2, true, 0, "Te_q0_r0_c2"},
                             ValenceTestCase{ELEM_Te, 0, 0, 4, true, 0, "Te_q0_r0_c4"}, ValenceTestCase{ELEM_Te, 0, 0, 6, true, 0, "Te_q0_r0_c6"},
                             // Te- : eff=7, ladder [1,3,5,7]
                             ValenceTestCase{ELEM_Te, -1, 0, 1, true, 0, "Te_q-1_r0_c1"}, ValenceTestCase{ELEM_Te, -1, 0, 3, true, 0, "Te_q-1_r0_c3_IMPROVED"},
                             ValenceTestCase{ELEM_Te, -1, 0, 5, true, 0, "Te_q-1_r0_c5"}, ValenceTestCase{ELEM_Te, -1, 0, 7, true, 0, "Te_q-1_r0_c7"},
                             // Te+ : eff=5, ladder [3,5]
                             ValenceTestCase{ELEM_Te, 1, 0, 0, true, 3, "Te_q+1_r0_c0"}, ValenceTestCase{ELEM_Te, 1, 0, 3, true, 0, "Te_q+1_r0_c3"},
                             // Te2+ : inert pair → eff=4, ip=2, ladder [2,4]
                             ValenceTestCase{ELEM_Te, 2, 0, 0, true, 2, "Te_q+2_r0_c0_inert"}, ValenceTestCase{ELEM_Te, 2, 0, 2, true, 0, "Te_q+2_r0_c2"},
                             ValenceTestCase{ELEM_Te, 2, 0, 4, true, 0, "Te_q+2_r0_c4"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 7 — Fluorine
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group7_F, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_F, 0, 0, 0, true, 1, "F_q0_r0_c0_HF"}, ValenceTestCase{ELEM_F, 0, 0, 1, true, 0, "F_q0_r0_c1"},
                                           ValenceTestCase{ELEM_F, 0, 0, 2, false, 0, "F_q0_r0_c2_INVALID"},
                                           ValenceTestCase{ELEM_F, -1, 0, 0, true, 0, "F_q-1_r0_c0"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 7 — Cl, Br, I, At (hypervalent halogens)
//   Rule C: no implicit H on hypervalent levels
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group7_Halogens, CalcValenceTest,
                         ::testing::Values(
                             // Cl neutral: eff=7, ladder [1,3,5,7], base=1
                             ValenceTestCase{ELEM_Cl, 0, 0, 0, true, 1, "Cl_q0_r0_c0_HCl"}, ValenceTestCase{ELEM_Cl, 0, 0, 1, true, 0, "Cl_q0_r0_c1"},
                             ValenceTestCase{ELEM_Cl, 0, 0, 2, false, 0, "Cl_q0_r0_c2_INVALID"}, ValenceTestCase{ELEM_Cl, 0, 0, 3, true, 0, "Cl_q0_r0_c3"},
                             ValenceTestCase{ELEM_Cl, 0, 0, 4, false, 0, "Cl_q0_r0_c4_INVALID"}, ValenceTestCase{ELEM_Cl, 0, 0, 5, true, 0, "Cl_q0_r0_c5"},
                             ValenceTestCase{ELEM_Cl, 0, 0, 6, false, 0, "Cl_q0_r0_c6_INVALID"}, ValenceTestCase{ELEM_Cl, 0, 0, 7, true, 0, "Cl_q0_r0_c7"},
                             // Cl radical + even conn
                             ValenceTestCase{ELEM_Cl, 0, RADICAL_DOUBLET, 2, true, 0, "Cl_q0_rD_c2"},
                             // Cl+ : eff=6, ladder [2,4,6]
                             ValenceTestCase{ELEM_Cl, 1, 0, 0, true, 2, "Cl_q+1_r0_c0"}, ValenceTestCase{ELEM_Cl, 1, 0, 2, true, 0, "Cl_q+1_r0_c2"},
                             // Br, I
                             ValenceTestCase{ELEM_Br, 0, 0, 0, true, 1, "Br_q0_r0_c0"}, ValenceTestCase{ELEM_Br, 0, 0, 1, true, 0, "Br_q0_r0_c1"},
                             ValenceTestCase{ELEM_Br, 0, 0, 3, true, 0, "Br_q0_r0_c3"}, ValenceTestCase{ELEM_I, 0, 0, 0, true, 1, "I_q0_r0_c0"},
                             ValenceTestCase{ELEM_I, 0, 0, 1, true, 0, "I_q0_r0_c1"}, ValenceTestCase{ELEM_I, 0, 0, 3, true, 0, "I_q0_r0_c3"},
                             ValenceTestCase{ELEM_I, 0, 0, 5, true, 0, "I_q0_r0_c5"}, ValenceTestCase{ELEM_I, 0, 0, 7, true, 0, "I_q0_r0_c7"}));

// ═══════════════════════════════════════════════════════════════════════════
// Group 8 — Noble gases
//   IMPROVED: Xe with conn=2,4,6 now valid (XeF2/4/6)
// ═══════════════════════════════════════════════════════════════════════════
INSTANTIATE_TEST_SUITE_P(Group8_NobleGases, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_He, 0, 0, 0, true, 0, "He_q0_r0_c0"}, ValenceTestCase{ELEM_Ne, 0, 0, 0, true, 0, "Ne_q0_r0_c0"},
                                           ValenceTestCase{ELEM_Ar, 0, 0, 0, true, 0, "Ar_q0_r0_c0"}, ValenceTestCase{ELEM_Kr, 0, 0, 0, true, 0, "Kr_q0_r0_c0"},
                                           ValenceTestCase{ELEM_Xe, 0, 0, 0, true, 0, "Xe_q0_r0_c0"}, ValenceTestCase{ELEM_Rn, 0, 0, 0, true, 0, "Rn_q0_r0_c0"},
                                           // IMPROVED: XeF2, XeF4, XeF6 — real compounds
                                           ValenceTestCase{ELEM_Xe, 0, 0, 2, true, 0, "Xe_q0_r0_c2_XeF2"},
                                           ValenceTestCase{ELEM_Xe, 0, 0, 4, true, 0, "Xe_q0_r0_c4_XeF4"},
                                           ValenceTestCase{ELEM_Xe, 0, 0, 6, true, 0, "Xe_q0_r0_c6_XeF6"},
                                           // Xe odd conn — INVALID (no implicit H on noble gases)
                                           ValenceTestCase{ELEM_Xe, 0, 0, 1, false, 0, "Xe_q0_r0_c1_INVALID"},
                                           ValenceTestCase{ELEM_Xe, 0, 0, 3, false, 0, "Xe_q0_r0_c3_INVALID"}));
