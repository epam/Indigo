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

#include <gtest/gtest.h>
#include <string>

#include "molecule/elements.h"
#include "molecule/valence_model.h"

using namespace indigo;

struct ValenceTestCase
{
    int elem;
    int charge;
    int radical;
    int conn;
    bool expect_nonStandard; // true when connectivity exceeds model prediction
    int expect_hyd;
    const char* label;
};

class CalcValenceTest : public ::testing::TestWithParam<ValenceTestCase>
{
};

TEST_P(CalcValenceTest, VerifyValenceResult)
{
    const auto& tc = GetParam();
    int valence = 0, hyd = 0;
    bool nonStd = false;
    bool result = Element::calcValence(tc.elem, tc.charge, tc.radical, tc.conn, valence, hyd, false, &nonStd);
    // Hybrid contract: result==true iff valence is standard. Tolerant callers can
    // still consume the collapsed value by checking nonStd, but the bool return
    // must match `!expect_nonStandard` so legacy `if (!calcValence(...))` callers
    // (e.g. shouldWriteHCountEx) detect bad valence again.
    EXPECT_EQ(!tc.expect_nonStandard, result) << "  case: " << tc.label << " (valid mismatch)";
    EXPECT_EQ(tc.expect_nonStandard, nonStd) << "  case: " << tc.label << " (nonStandard mismatch)";
    EXPECT_EQ(tc.expect_hyd, hyd) << "  case: " << tc.label << " (hyd mismatch)";
}

// Group 1 — Hydrogen
INSTANTIATE_TEST_SUITE_P(Group1_H, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_H, 0, 0, 0, false, 1, "H_q0_r0_c0"}, ValenceTestCase{ELEM_H, 0, 0, 1, false, 0, "H_q0_r0_c1"},
                                           ValenceTestCase{ELEM_H, 0, 0, 2, true, 0, "H_q0_r0_c2_NONSTD"},
                                           ValenceTestCase{ELEM_H, 1, 0, 0, false, 0, "H_q+1_r0_c0_proton"},
                                           ValenceTestCase{ELEM_H, -1, 0, 0, false, 0, "H_q-1_r0_c0_hydride"},
                                           // Bare H with a radical is serialised as [HH] per BIOVIA-Draw; hyd=1,
                                           // matching the rpe/rpe_basic.py reference. The radical parameter is
                                           // intentionally ignored here for isolated Hs.
                                           ValenceTestCase{ELEM_H, 0, RADICAL_DOUBLET, 0, false, 1, "H_q0_rD_c0_bareRadicalIsH2"}));

// Group 1 — Alkali metals
INSTANTIATE_TEST_SUITE_P(Group1_Alkali, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_Li, 0, 0, 0, false, 1, "Li_q0_r0_c0"},
                                           ValenceTestCase{ELEM_Li, 0, 0, 1, false, 0, "Li_q0_r0_c1"},
                                           ValenceTestCase{ELEM_Li, 1, 0, 0, false, 0, "Li_q+1_r0_c0_ion"},
                                           ValenceTestCase{ELEM_Na, -1, 0, 0, false, 0, "Na_q-1_r0_c0_natride"},
                                           ValenceTestCase{ELEM_Na, 0, 0, 1, false, 0, "Na_q0_r0_c1"}, ValenceTestCase{ELEM_K, 0, 0, 0, false, 1, "K_q0_r0_c0"},
                                           ValenceTestCase{ELEM_K, 0, 0, 1, false, 0, "K_q0_r0_c1"}));

// Group 2 — Alkaline earth metals
INSTANTIATE_TEST_SUITE_P(
    Group2, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Mg, 0, 0, 2, false, 0, "Mg_q0_r0_c2"}, ValenceTestCase{ELEM_Mg, 0, 0, 0, false, 0, "Mg_q0_r0_c0"},
                      ValenceTestCase{ELEM_Mg, 0, 0, 1, true, 0, "Mg_q0_r0_c1_NONSTD"}, ValenceTestCase{ELEM_Be, 0, 0, 2, false, 0, "Be_q0_r0_c2"},
                      ValenceTestCase{ELEM_Be, 0, 0, 0, false, 0, "Be_q0_r0_c0"}, ValenceTestCase{ELEM_Ca, 0, 0, 2, false, 0, "Ca_q0_r0_c2"},
                      ValenceTestCase{ELEM_Ca, 1, 0, 1, true, 0, "Ca_q+1_r0_c1_NONSTD"}, ValenceTestCase{ELEM_Ca, 2, 0, 0, false, 0, "Ca_q+2_r0_c0"},
                      ValenceTestCase{ELEM_Ba, 0, 0, 2, false, 0, "Ba_q0_r0_c2"}));

// Group 3 — Boron
INSTANTIATE_TEST_SUITE_P(Group3_B, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_B, 0, 0, 0, false, 3, "B_q0_r0_c0"}, ValenceTestCase{ELEM_B, 0, 0, 3, false, 0, "B_q0_r0_c3"},
                                           ValenceTestCase{ELEM_B, 0, 0, 4, true, 0, "B_q0_r0_c4_NONSTD"},
                                           ValenceTestCase{ELEM_B, -1, 0, 0, false, 4, "B_q-1_r0_c0"},
                                           ValenceTestCase{ELEM_B, -1, 0, 4, false, 0, "B_q-1_r0_c4"},
                                           ValenceTestCase{ELEM_B, 1, 0, 0, false, 2, "B_q+1_r0_c0"}, ValenceTestCase{ELEM_B, 1, 0, 2, false, 0, "B_q+1_r0_c2"},
                                           ValenceTestCase{ELEM_B, 1, 0, 3, true, 0, "B_q+1_r0_c3_NONSTD"}));

// Group 3 — Al, Ga, In
INSTANTIATE_TEST_SUITE_P(
    Group3_Al, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Al, 0, 0, 0, false, 3, "Al_q0_r0_c0"}, ValenceTestCase{ELEM_Al, 0, 0, 3, false, 0, "Al_q0_r0_c3"},
                      ValenceTestCase{ELEM_Al, -1, 0, 0, false, 4, "Al_q-1_r0_c0"}, ValenceTestCase{ELEM_Al, -1, 0, 4, false, 0, "Al_q-1_r0_c4"},
                      ValenceTestCase{ELEM_Al, -2, 0, 5, false, 0, "Al_q-2_r0_c5"}, ValenceTestCase{ELEM_Al, -3, 0, 6, false, 0, "Al_q-3_r0_c6"},
                      // Ga
                      ValenceTestCase{ELEM_Ga, 0, 0, 3, false, 0, "Ga_q0_r0_c3"}, ValenceTestCase{ELEM_Ga, -1, 0, 4, false, 0, "Ga_q-1_r0_c4"},
                      // In
                      ValenceTestCase{ELEM_In, 0, 0, 3, false, 0, "In_q0_r0_c3"}, ValenceTestCase{ELEM_In, -1, 0, 4, false, 0, "In_q-1_r0_c4"}));

// Group 3 — Tl (inert pair)
INSTANTIATE_TEST_SUITE_P(
    Group3_Tl, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Tl, 0, 0, 0, false, 1, "Tl_q0_r0_c0"}, ValenceTestCase{ELEM_Tl, 0, 0, 1, false, 0, "Tl_q0_r0_c1"},
                      ValenceTestCase{ELEM_Tl, 0, 0, 3, false, 0, "Tl_q0_r0_c3"}, ValenceTestCase{ELEM_Tl, -1, 0, 0, false, 2, "Tl_q-1_r0_c0"},
                      ValenceTestCase{ELEM_Tl, -1, 0, 2, false, 0, "Tl_q-1_r0_c2"}, ValenceTestCase{ELEM_Tl, -1, 0, 4, false, 0, "Tl_q-1_r0_c4"},
                      ValenceTestCase{ELEM_Tl, -2, 0, 3, false, 0, "Tl_q-2_r0_c3"}, ValenceTestCase{ELEM_Tl, -2, 0, 5, false, 0, "Tl_q-2_r0_c5"},
                      ValenceTestCase{ELEM_Tl, -3, 0, 6, false, 0, "Tl_q-3_r0_c6"}, ValenceTestCase{ELEM_Tl, 1, 0, 0, false, 0, "Tl_q+1_r0_c0"}));

// Group 4 — Carbon
INSTANTIATE_TEST_SUITE_P(Group4_C, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_C, 0, 0, 0, false, 4, "C_q0_r0_c0_CH4"},
                                           ValenceTestCase{ELEM_C, 0, 0, 1, false, 3, "C_q0_r0_c1"}, ValenceTestCase{ELEM_C, 0, 0, 2, false, 2, "C_q0_r0_c2"},
                                           ValenceTestCase{ELEM_C, 0, 0, 3, false, 1, "C_q0_r0_c3"}, ValenceTestCase{ELEM_C, 0, 0, 4, false, 0, "C_q0_r0_c4"},
                                           ValenceTestCase{ELEM_C, 0, 0, 5, true, 0, "C_q0_r0_c5_NONSTD"},
                                           ValenceTestCase{ELEM_C, 1, 0, 0, false, 3, "C_q+1_r0_c0"}, ValenceTestCase{ELEM_C, 1, 0, 3, false, 0, "C_q+1_r0_c3"},
                                           ValenceTestCase{ELEM_C, -1, 0, 0, false, 3, "C_q-1_r0_c0"},
                                           ValenceTestCase{ELEM_C, -1, 0, 3, false, 0, "C_q-1_r0_c3"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_DOUBLET, 0, false, 3, "C_q0_rD_c0"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_DOUBLET, 3, false, 0, "C_q0_rD_c3"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_TRIPLET, 0, false, 2, "C_q0_rT_c0"},
                                           ValenceTestCase{ELEM_C, 0, RADICAL_TRIPLET, 2, false, 0, "C_q0_rT_c2"}));

// Group 4 — Si, Ge
INSTANTIATE_TEST_SUITE_P(
    Group4_SiGe, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Si, 0, 0, 0, false, 4, "Si_q0_r0_c0_SiH4"}, ValenceTestCase{ELEM_Si, 0, 0, 4, false, 0, "Si_q0_r0_c4"},
                      ValenceTestCase{ELEM_Si, 0, 0, 5, true, 0, "Si_q0_r0_c5_NONSTD"}, ValenceTestCase{ELEM_Si, -1, 0, 0, false, 3, "Si_q-1_r0_c0"},
                      ValenceTestCase{ELEM_Si, -1, 0, 4, false, 1, "Si_q-1_r0_c4"}, ValenceTestCase{ELEM_Si, -1, 0, 5, false, 0, "Si_q-1_r0_c5"},
                      ValenceTestCase{ELEM_Si, -2, 0, 5, false, 0, "Si_q-2_r0_c5_SiF5"}, ValenceTestCase{ELEM_Si, -2, 0, 6, false, 0, "Si_q-2_r0_c6_SiF6"},
                      ValenceTestCase{ELEM_Si, -2, 0, 3, true, 0, "Si_q-2_r0_c3_NONSTD"}, ValenceTestCase{ELEM_Si, -2, 0, 4, true, 0, "Si_q-2_r0_c4_NONSTD"},
                      ValenceTestCase{ELEM_Ge, 0, 0, 0, false, 4, "Ge_q0_r0_c0"}, ValenceTestCase{ELEM_Ge, 0, 0, 4, false, 0, "Ge_q0_r0_c4"},
                      ValenceTestCase{ELEM_Ge, -1, 0, 5, false, 0, "Ge_q-1_r0_c5"}));

// Group 4 — Sn, Pb (inert pair)
INSTANTIATE_TEST_SUITE_P(
    Group4_SnPb, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Sn, 0, 0, 0, false, 2, "Sn_q0_r0_c0_SnH2"}, ValenceTestCase{ELEM_Sn, 0, 0, 2, false, 0, "Sn_q0_r0_c2"},
                      ValenceTestCase{ELEM_Sn, 0, 0, 4, false, 0, "Sn_q0_r0_c4"}, ValenceTestCase{ELEM_Sn, 0, 0, 5, true, 0, "Sn_q0_r0_c5_NONSTD"},
                      ValenceTestCase{ELEM_Pb, 0, 0, 0, false, 2, "Pb_q0_r0_c0_PbH2"}, ValenceTestCase{ELEM_Pb, 0, 0, 2, false, 0, "Pb_q0_r0_c2"},
                      ValenceTestCase{ELEM_Pb, 0, 0, 4, false, 0, "Pb_q0_r0_c4"}, ValenceTestCase{ELEM_Sn, -1, 0, 5, false, 0, "Sn_q-1_r0_c5"},
                      ValenceTestCase{ELEM_Sn, -2, 0, 6, false, 0, "Sn_q-2_r0_c6"}, ValenceTestCase{ELEM_Sn, -2, 0, 3, true, 0, "Sn_q-2_r0_c3_NONSTD"},
                      ValenceTestCase{ELEM_Sn, -2, 0, 4, true, 0, "Sn_q-2_r0_c4_NONSTD"}));

// Group 5 — Nitrogen
INSTANTIATE_TEST_SUITE_P(
    Group5_N, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_N, 0, 0, 0, false, 3, "N_q0_r0_c0_NH3"}, ValenceTestCase{ELEM_N, 0, 0, 1, false, 2, "N_q0_r0_c1"},
                      ValenceTestCase{ELEM_N, 0, 0, 2, false, 1, "N_q0_r0_c2"}, ValenceTestCase{ELEM_N, 0, 0, 3, false, 0, "N_q0_r0_c3"},
                      ValenceTestCase{ELEM_N, 0, 0, 4, true, 0, "N_q0_r0_c4_NONSTD"}, ValenceTestCase{ELEM_N, 1, 0, 0, false, 4, "N_q+1_r0_c0_NH4+"},
                      ValenceTestCase{ELEM_N, 1, 0, 4, false, 0, "N_q+1_r0_c4"}, ValenceTestCase{ELEM_N, 2, 0, 0, false, 3, "N_q+2_r0_c0"},
                      ValenceTestCase{ELEM_N, 2, 0, 3, false, 0, "N_q+2_r0_c3"}, ValenceTestCase{ELEM_N, -1, 0, 0, false, 2, "N_q-1_r0_c0"},
                      ValenceTestCase{ELEM_N, -1, 0, 2, false, 0, "N_q-1_r0_c2"}, ValenceTestCase{ELEM_N, 0, RADICAL_DOUBLET, 0, false, 2, "N_q0_rD_c0"},
                      ValenceTestCase{ELEM_N, 0, RADICAL_DOUBLET, 2, false, 0, "N_q0_rD_c2"}));

// Group 5 — Phosphorus
INSTANTIATE_TEST_SUITE_P(
    Group5_P, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_P, 0, 0, 0, false, 3, "P_q0_r0_c0_PH3"}, ValenceTestCase{ELEM_P, 0, 0, 3, false, 0, "P_q0_r0_c3"},
                      ValenceTestCase{ELEM_P, 0, 0, 5, false, 0, "P_q0_r0_c5_PF5"}, ValenceTestCase{ELEM_P, 0, 0, 4, false, 1, "P_q0_r0_c4"},
                      ValenceTestCase{ELEM_P, 1, 0, 0, false, 4, "P_q+1_r0_c0"}, ValenceTestCase{ELEM_P, 1, 0, 4, false, 0, "P_q+1_r0_c4"},
                      ValenceTestCase{ELEM_P, 2, 0, 0, false, 3, "P_q+2_r0_c0"}, ValenceTestCase{ELEM_P, 2, 0, 3, false, 0, "P_q+2_r0_c3"},
                      ValenceTestCase{ELEM_P, -1, 0, 0, false, 2, "P_q-1_r0_c0"}, ValenceTestCase{ELEM_P, -1, 0, 2, false, 0, "P_q-1_r0_c2"},
                      ValenceTestCase{ELEM_P, -1, 0, 3, true, 0, "P_q-1_r0_c3_NONSTD"}, ValenceTestCase{ELEM_P, -1, 0, 4, false, 0, "P_q-1_r0_c4"},
                      ValenceTestCase{ELEM_P, -1, 0, 6, false, 0, "P_q-1_r0_c6"}));

// Group 5 — As, Sb, Bi
INSTANTIATE_TEST_SUITE_P(
    Group5_AsSbBi, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_As, 0, 0, 0, false, 3, "As_q0_r0_c0"}, ValenceTestCase{ELEM_As, 0, 0, 3, false, 0, "As_q0_r0_c3"},
                      ValenceTestCase{ELEM_As, 0, 0, 5, false, 0, "As_q0_r0_c5"}, ValenceTestCase{ELEM_Sb, 0, 0, 0, false, 3, "Sb_q0_r0_c0"},
                      ValenceTestCase{ELEM_Sb, 0, 0, 3, false, 0, "Sb_q0_r0_c3"}, ValenceTestCase{ELEM_Sb, 0, 0, 5, false, 0, "Sb_q0_r0_c5"},
                      ValenceTestCase{ELEM_Sb, 1, 0, 0, false, 2, "Sb_q+1_r0_c0_inert"}, ValenceTestCase{ELEM_Sb, 1, 0, 1, false, 1, "Sb_q+1_r0_c1_inert"},
                      ValenceTestCase{ELEM_Sb, 1, 0, 2, false, 0, "Sb_q+1_r0_c2"}, ValenceTestCase{ELEM_Sb, 1, 0, 4, false, 0, "Sb_q+1_r0_c4"},
                      ValenceTestCase{ELEM_Sb, 2, 0, 3, false, 0, "Sb_q+2_r0_c3"}, ValenceTestCase{ELEM_Sb, -1, 0, 6, false, 0, "Sb_q-1_r0_c6"},
                      ValenceTestCase{ELEM_Sb, -2, 0, 5, false, 0, "Sb_q-2_r0_c5"}, ValenceTestCase{ELEM_Bi, 0, 0, 0, false, 3, "Bi_q0_r0_c0"},
                      ValenceTestCase{ELEM_Bi, 0, 0, 3, false, 0, "Bi_q0_r0_c3"}, ValenceTestCase{ELEM_Bi, 0, 0, 5, false, 0, "Bi_q0_r0_c5"},
                      ValenceTestCase{ELEM_Bi, 1, 0, 0, false, 2, "Bi_q+1_r0_c0_inert"}, ValenceTestCase{ELEM_Bi, 1, 0, 2, false, 0, "Bi_q+1_r0_c2"},
                      ValenceTestCase{ELEM_Bi, -2, 0, 5, false, 0, "Bi_q-2_r0_c5"}));

// Group 6 — Oxygen
INSTANTIATE_TEST_SUITE_P(
    Group6_O, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_O, 0, 0, 0, false, 2, "O_q0_r0_c0_H2O"}, ValenceTestCase{ELEM_O, 0, 0, 1, false, 1, "O_q0_r0_c1"},
                      ValenceTestCase{ELEM_O, 0, 0, 2, false, 0, "O_q0_r0_c2"}, ValenceTestCase{ELEM_O, 0, 0, 3, true, 0, "O_q0_r0_c3_NONSTD"},
                      ValenceTestCase{ELEM_O, -1, 0, 0, false, 1, "O_q-1_r0_c0"}, ValenceTestCase{ELEM_O, -1, 0, 1, false, 0, "O_q-1_r0_c1"},
                      ValenceTestCase{ELEM_O, 1, 0, 0, false, 3, "O_q+1_r0_c0"}, ValenceTestCase{ELEM_O, 1, 0, 3, false, 0, "O_q+1_r0_c3"}));

// Group 6 — S, Se, Po
INSTANTIATE_TEST_SUITE_P(
    Group6_SSeP, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_S, 0, 0, 0, false, 2, "S_q0_r0_c0_H2S"}, ValenceTestCase{ELEM_S, 0, 0, 2, false, 0, "S_q0_r0_c2"},
                      ValenceTestCase{ELEM_S, 0, 0, 3, false, 1, "S_q0_r0_c3"}, ValenceTestCase{ELEM_S, 0, 0, 4, false, 0, "S_q0_r0_c4"},
                      ValenceTestCase{ELEM_S, 0, 0, 6, false, 0, "S_q0_r0_c6_SF6"}, ValenceTestCase{ELEM_S, 1, 0, 0, false, 3, "S_q+1_r0_c0"},
                      ValenceTestCase{ELEM_S, 1, 0, 3, false, 0, "S_q+1_r0_c3"}, ValenceTestCase{ELEM_S, 1, 0, 5, false, 0, "S_q+1_r0_c5"},
                      ValenceTestCase{ELEM_S, -1, 0, 0, false, 1, "S_q-1_r0_c0"}, ValenceTestCase{ELEM_S, -1, 0, 1, false, 0, "S_q-1_r0_c1"},
                      ValenceTestCase{ELEM_S, -1, 0, 3, false, 0, "S_q-1_r0_c3"}, ValenceTestCase{ELEM_S, -1, 0, 5, false, 0, "S_q-1_r0_c5"},
                      ValenceTestCase{ELEM_S, -1, 0, 7, false, 0, "S_q-1_r0_c7"}, ValenceTestCase{ELEM_Se, 0, 0, 0, false, 2, "Se_q0_r0_c0"},
                      ValenceTestCase{ELEM_Se, 0, 0, 2, false, 0, "Se_q0_r0_c2"}, ValenceTestCase{ELEM_Se, 0, 0, 4, false, 0, "Se_q0_r0_c4"},
                      ValenceTestCase{ELEM_Se, 0, 0, 6, false, 0, "Se_q0_r0_c6"}));

// Group 6 — Te
INSTANTIATE_TEST_SUITE_P(
    Group6_Te, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Te, 0, 0, 0, false, 2, "Te_q0_r0_c0"}, ValenceTestCase{ELEM_Te, 0, 0, 2, false, 0, "Te_q0_r0_c2"},
                      ValenceTestCase{ELEM_Te, 0, 0, 4, false, 0, "Te_q0_r0_c4"}, ValenceTestCase{ELEM_Te, 0, 0, 6, false, 0, "Te_q0_r0_c6"},
                      ValenceTestCase{ELEM_Te, -1, 0, 1, false, 0, "Te_q-1_r0_c1"}, ValenceTestCase{ELEM_Te, -1, 0, 3, false, 0, "Te_q-1_r0_c3_IMPROVED"},
                      ValenceTestCase{ELEM_Te, -1, 0, 5, false, 0, "Te_q-1_r0_c5"}, ValenceTestCase{ELEM_Te, -1, 0, 7, false, 0, "Te_q-1_r0_c7"},
                      ValenceTestCase{ELEM_Te, -1, 0, 4, false, 1, "Te_q-1_r0_c4"}, ValenceTestCase{ELEM_Te, 1, 0, 0, false, 3, "Te_q+1_r0_c0"},
                      ValenceTestCase{ELEM_Te, 1, 0, 3, false, 0, "Te_q+1_r0_c3"}, ValenceTestCase{ELEM_Te, 1, 0, 4, false, 1, "Te_q+1_r0_c4"},
                      ValenceTestCase{ELEM_Te, 2, 0, 0, false, 2, "Te_q+2_r0_c0_inert"}, ValenceTestCase{ELEM_Te, 2, 0, 2, false, 0, "Te_q+2_r0_c2"},
                      ValenceTestCase{ELEM_Te, 2, 0, 4, false, 0, "Te_q+2_r0_c4"}));

// Group 7 — Fluorine
INSTANTIATE_TEST_SUITE_P(Group7_F, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_F, 0, 0, 0, false, 1, "F_q0_r0_c0_HF"},
                                           ValenceTestCase{ELEM_F, 0, 0, 1, false, 0, "F_q0_r0_c1"},
                                           ValenceTestCase{ELEM_F, 0, 0, 2, true, 0, "F_q0_r0_c2_NONSTD"},
                                           ValenceTestCase{ELEM_F, -1, 0, 0, false, 0, "F_q-1_r0_c0"}));

// Group 7 — Cl, Br, I, At
INSTANTIATE_TEST_SUITE_P(
    Group7_Halogens, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Cl, 0, 0, 0, false, 1, "Cl_q0_r0_c0_HCl"}, ValenceTestCase{ELEM_Cl, 0, 0, 1, false, 0, "Cl_q0_r0_c1"},
                      ValenceTestCase{ELEM_Cl, 0, 0, 2, true, 0, "Cl_q0_r0_c2_NONSTD"}, ValenceTestCase{ELEM_Cl, 0, 0, 3, false, 0, "Cl_q0_r0_c3"},
                      ValenceTestCase{ELEM_Cl, 0, 0, 4, true, 0, "Cl_q0_r0_c4_NONSTD"}, ValenceTestCase{ELEM_Cl, 0, 0, 5, false, 0, "Cl_q0_r0_c5"},
                      ValenceTestCase{ELEM_Cl, 0, 0, 6, true, 0, "Cl_q0_r0_c6_NONSTD"}, ValenceTestCase{ELEM_Cl, 0, 0, 7, false, 0, "Cl_q0_r0_c7"},
                      ValenceTestCase{ELEM_Cl, 0, RADICAL_DOUBLET, 2, false, 0, "Cl_q0_rD_c2"}, ValenceTestCase{ELEM_Cl, 1, 0, 0, false, 2, "Cl_q+1_r0_c0"},
                      ValenceTestCase{ELEM_Cl, 1, 0, 2, false, 0, "Cl_q+1_r0_c2"}, ValenceTestCase{ELEM_Br, 0, 0, 0, false, 1, "Br_q0_r0_c0"},
                      ValenceTestCase{ELEM_Br, 0, 0, 1, false, 0, "Br_q0_r0_c1"}, ValenceTestCase{ELEM_Br, 0, 0, 3, false, 0, "Br_q0_r0_c3"},
                      ValenceTestCase{ELEM_I, 0, 0, 0, false, 1, "I_q0_r0_c0"}, ValenceTestCase{ELEM_I, 0, 0, 1, false, 0, "I_q0_r0_c1"},
                      ValenceTestCase{ELEM_I, 0, 0, 3, false, 0, "I_q0_r0_c3"}, ValenceTestCase{ELEM_I, 0, 0, 5, false, 0, "I_q0_r0_c5"},
                      ValenceTestCase{ELEM_I, 0, 0, 7, false, 0, "I_q0_r0_c7"}));

// Group 8 — Noble gases
INSTANTIATE_TEST_SUITE_P(
    Group8_NobleGases, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_He, 0, 0, 0, false, 0, "He_q0_r0_c0"}, ValenceTestCase{ELEM_Ne, 0, 0, 0, false, 0, "Ne_q0_r0_c0"},
                      ValenceTestCase{ELEM_Ar, 0, 0, 0, false, 0, "Ar_q0_r0_c0"}, ValenceTestCase{ELEM_Kr, 0, 0, 0, false, 0, "Kr_q0_r0_c0"},
                      ValenceTestCase{ELEM_Xe, 0, 0, 0, false, 0, "Xe_q0_r0_c0"}, ValenceTestCase{ELEM_Rn, 0, 0, 0, false, 0, "Rn_q0_r0_c0"},
                      // XeF2, XeF4, XeF6
                      ValenceTestCase{ELEM_Xe, 0, 0, 2, false, 0, "Xe_q0_r0_c2_XeF2"}, ValenceTestCase{ELEM_Xe, 0, 0, 4, false, 0, "Xe_q0_r0_c4_XeF4"},
                      ValenceTestCase{ELEM_Xe, 0, 0, 6, false, 0, "Xe_q0_r0_c6_XeF6"}, ValenceTestCase{ELEM_Xe, 0, 0, 1, true, 0, "Xe_q0_r0_c1_NONSTD"},
                      ValenceTestCase{ELEM_Xe, 0, 0, 3, true, 0, "Xe_q0_r0_c3_NONSTD"}));

// Group 7 — At (astatine, period 6, hypervalent halogen)
INSTANTIATE_TEST_SUITE_P(
    Group7_At, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_At, 0, 0, 0, false, 1, "At_q0_r0_c0"}, ValenceTestCase{ELEM_At, 0, 0, 1, false, 0, "At_q0_r0_c1"},
                      ValenceTestCase{ELEM_At, 0, 0, 3, false, 0, "At_q0_r0_c3"}, ValenceTestCase{ELEM_At, 0, 0, 5, false, 0, "At_q0_r0_c5"},
                      ValenceTestCase{ELEM_At, 0, 0, 7, false, 0, "At_q0_r0_c7"}, ValenceTestCase{ELEM_At, 0, RADICAL_DOUBLET, 0, false, 0, "At_q0_rD_c0"},
                      ValenceTestCase{ELEM_At, 0, RADICAL_DOUBLET, 2, false, 0, "At_q0_rD_c2"}, ValenceTestCase{ELEM_At, -1, 0, 0, false, 0, "At_q-1_r0_c0"},
                      ValenceTestCase{ELEM_At, 1, 0, 0, false, 2, "At_q+1_r0_c0"}, ValenceTestCase{ELEM_At, 1, 0, 2, false, 0, "At_q+1_r0_c2"}));

// Group 6 — Po (polonium, period 6, NOT inert pair in current impl)
INSTANTIATE_TEST_SUITE_P(
    Group6_Po, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Po, 0, 0, 0, false, 2, "Po_q0_r0_c0"}, ValenceTestCase{ELEM_Po, 0, 0, 2, false, 0, "Po_q0_r0_c2"},
                      ValenceTestCase{ELEM_Po, 0, 0, 4, false, 0, "Po_q0_r0_c4"}, ValenceTestCase{ELEM_Po, 0, 0, 6, false, 0, "Po_q0_r0_c6"},
                      ValenceTestCase{ELEM_Po, -1, 0, 0, false, 1, "Po_q-1_r0_c0"}, ValenceTestCase{ELEM_Po, -1, 0, 1, false, 0, "Po_q-1_r0_c1"},
                      ValenceTestCase{ELEM_Po, 1, 0, 0, false, 3, "Po_q+1_r0_c0"}, ValenceTestCase{ELEM_Po, 1, 0, 3, false, 0, "Po_q+1_r0_c3"}));

// Group 8 — Xe charged states
INSTANTIATE_TEST_SUITE_P(Group8_Xe_Charged, CalcValenceTest,
                         ::testing::Values( // Xe²⁺ conn=0: eff=6, base=2, noble_no_h blocks h>0 → NONSTD
                             ValenceTestCase{ELEM_Xe, 2, 0, 0, true, 0, "Xe_q+2_r0_c0_NONSTD"}, ValenceTestCase{ELEM_Xe, 2, 0, 2, false, 0, "Xe_q+2_r0_c2"},
                             ValenceTestCase{ELEM_Xe, 2, 0, 4, false, 0, "Xe_q+2_r0_c4"},
                             // Xe⁴⁺ conn=0: eff=4, base=4, noble_no_h blocks h=4 → NONSTD
                             ValenceTestCase{ELEM_Xe, 4, 0, 0, true, 0, "Xe_q+4_r0_c0_NONSTD"},
                             ValenceTestCase{ELEM_Xe, 4, 0, 2, true, 0, "Xe_q+4_r0_c2_NONSTD"},
                             // Xe⁻: eff=9>8, ion early-return
                             ValenceTestCase{ELEM_Xe, -1, 0, 0, false, 0, "Xe_q-1_r0_c0"}, ValenceTestCase{ELEM_Xe, -2, 0, 0, false, 0, "Xe_q-2_r0_c0"}));

// Group 8 — Kr with connectivity
INSTANTIATE_TEST_SUITE_P(Group8_Kr_Expanded, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_Kr, 0, 0, 2, false, 0, "Kr_q0_r0_c2_KrF2"},
                                           ValenceTestCase{ELEM_Kr, 0, 0, 1, true, 0, "Kr_q0_r0_c1_NONSTD"},
                                           ValenceTestCase{ELEM_Kr, 0, 0, 4, false, 0, "Kr_q0_r0_c4"},
                                           // Kr²⁺ conn=0: eff=6, base=2, noble_no_h blocks h>0 → NONSTD
                                           ValenceTestCase{ELEM_Kr, 2, 0, 0, true, 0, "Kr_q+2_r0_c0_NONSTD"}));

// Group 8 — Rn with connectivity
INSTANTIATE_TEST_SUITE_P(Group8_Rn_Expanded, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_Rn, 0, 0, 2, false, 0, "Rn_q0_r0_c2_RnF2"},
                                           ValenceTestCase{ELEM_Rn, 0, 0, 4, false, 0, "Rn_q0_r0_c4"},
                                           ValenceTestCase{ELEM_Rn, 0, 0, 6, false, 0, "Rn_q0_r0_c6"},
                                           // Rn²⁺ conn=0: eff=6, base=2, noble_no_h blocks h>0 → NONSTD
                                           ValenceTestCase{ELEM_Rn, 2, 0, 0, true, 0, "Rn_q+2_r0_c0_NONSTD"},
                                           ValenceTestCase{ELEM_Rn, 0, 0, 1, true, 0, "Rn_q0_r0_c1_NONSTD"}));

// ═══════════════════════════════════════════════════════════════════════
// Regression tests for Phase 1 audit fixes (BUG-1, BUG-2, BUG-4, BUG-7)
// ═══════════════════════════════════════════════════════════════════════

// BUG-1: Halide anions (eff=8) with connectivity — were INVALID, now accept bonds with h=0
INSTANTIATE_TEST_SUITE_P(
    Regression_HalideAnion, CalcValenceTest,
    ::testing::Values(ValenceTestCase{ELEM_Cl, -1, 0, 0, false, 0, "Cl_q-1_r0_c0"}, ValenceTestCase{ELEM_Cl, -1, 0, 1, false, 0, "Cl_q-1_r0_c1_coord"},
                      ValenceTestCase{ELEM_Cl, -1, 0, 2, false, 0, "Cl_q-1_r0_c2_coord"}, ValenceTestCase{ELEM_Cl, -1, 0, 3, false, 0, "Cl_q-1_r0_c3_coord"},
                      ValenceTestCase{ELEM_Br, -1, 0, 0, false, 0, "Br_q-1_r0_c0"}, ValenceTestCase{ELEM_Br, -1, 0, 1, false, 0, "Br_q-1_r0_c1_coord"},
                      ValenceTestCase{ELEM_I, -1, 0, 0, false, 0, "I_q-1_r0_c0"}, ValenceTestCase{ELEM_I, -1, 0, 1, false, 0, "I_q-1_r0_c1_coord"},
                      ValenceTestCase{ELEM_I, -1, 0, 3, false, 0, "I_q-1_r0_c3_coord"}));

// BUG-4: Chalcogenide dianions (eff=8) — noble-gas config, no implicit H
INSTANTIATE_TEST_SUITE_P(Regression_ChalcogenideDianion, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_S, -2, 0, 0, false, 0, "S_q-2_r0_c0_sulfide"},
                                           ValenceTestCase{ELEM_S, -2, 0, 1, false, 0, "S_q-2_r0_c1_coord"},
                                           ValenceTestCase{ELEM_S, -2, 0, 2, false, 0, "S_q-2_r0_c2_coord"},
                                           ValenceTestCase{ELEM_Se, -2, 0, 0, false, 0, "Se_q-2_r0_c0_selenide"},
                                           ValenceTestCase{ELEM_Se, -2, 0, 1, false, 0, "Se_q-2_r0_c1_coord"},
                                           ValenceTestCase{ELEM_Te, -2, 0, 0, false, 0, "Te_q-2_r0_c0_telluride"},
                                           ValenceTestCase{ELEM_Te, -2, 0, 1, false, 0, "Te_q-2_r0_c1_coord"}));

// BUG-7: Halogen radical conn=0 should give v=1 (not v=0)
// BUG-2: Halogen radical with odd connectivity (3,5) — parity fix
INSTANTIATE_TEST_SUITE_P(Regression_HalogenRadical, CalcValenceTest,
                         ::testing::Values(ValenceTestCase{ELEM_Cl, 0, RADICAL_DOUBLET, 0, false, 0, "Cl_q0_rD_c0_radical"},
                                           ValenceTestCase{ELEM_Cl, 0, RADICAL_DOUBLET, 3, false, 0, "Cl_q0_rD_c3_odd"},
                                           ValenceTestCase{ELEM_Cl, 0, RADICAL_DOUBLET, 5, false, 0, "Cl_q0_rD_c5_odd"},
                                           ValenceTestCase{ELEM_Br, 0, RADICAL_DOUBLET, 0, false, 0, "Br_q0_rD_c0_radical"},
                                           ValenceTestCase{ELEM_Br, 0, RADICAL_DOUBLET, 3, false, 0, "Br_q0_rD_c3_odd"},
                                           ValenceTestCase{ELEM_I, 0, RADICAL_DOUBLET, 0, false, 0, "I_q0_rD_c0_radical"}));

// Boundary eff values
INSTANTIATE_TEST_SUITE_P(Boundary_Eff, CalcValenceTest,
                         ::testing::Values(
                             // eff=0: N⁵⁺ (g=5, q=5 → eff=0) — ion early-return
                             ValenceTestCase{ELEM_N, 5, 0, 0, false, 0, "N_q+5_r0_c0_eff0"},
                             // eff<0: N⁶⁺ (g=5, q=6 → eff=-1) — ion early-return
                             ValenceTestCase{ELEM_N, 6, 0, 0, false, 0, "N_q+6_r0_c0_effNeg"},
                             // eff>8: B⁶⁻ (g=3, q=-6 → eff=9) — ion early-return
                             ValenceTestCase{ELEM_B, -6, 0, 0, false, 0, "B_q-6_r0_c0_eff9"},
                             // O⁺⁵ with no bonds: legacy O⁺ rule fixes valence=3 and absorbs the charge into
                             // the hydrogen count (hyd=3), matching the reference in
                             // basic/buffer_string_load_iterate.py (conn=1 case yields [OH2+5] there).
                             ValenceTestCase{ELEM_O, 5, 0, 0, false, 3, "O_q+5_r0_c0_legacyAbsorb"}));

// calcValenceMinusHyd: charge absorption for explicit VAL= in MOL files

struct ValenceMinusHydTestCase
{
    int elem;
    int charge;
    int radical;
    int conn;
    int expected_result;
    const char* label;
};

class CalcValenceMinusHydTest : public ::testing::TestWithParam<ValenceMinusHydTestCase>
{
};

TEST_P(CalcValenceMinusHydTest, VerifyResult)
{
    const auto& tc = GetParam();
    int result = Element::calcValenceMinusHyd(tc.elem, tc.charge, tc.radical, tc.conn);
    EXPECT_EQ(tc.expected_result, result) << "  case: " << tc.label;
}

TEST_P(CalcValenceMinusHydTest, ConsistentWithCalcValence)
{
    const auto& tc = GetParam();
    int valence = 0, hyd = 0;
    bool valid = Element::calcValence(tc.elem, tc.charge, tc.radical, tc.conn, valence, hyd, false);
    if (valid && hyd >= 0)
    {
        int vmh = Element::calcValenceMinusHyd(tc.elem, tc.charge, tc.radical, tc.conn);
        EXPECT_EQ(valence - hyd, vmh) << "  case: " << tc.label << " | calcValence: v=" << valence << " hyd=" << hyd << " | calcValenceMinusHyd=" << vmh;
    }
}

// Group 3
INSTANTIATE_TEST_SUITE_P(VMH_Group3, CalcValenceMinusHydTest,
                         ::testing::Values(ValenceMinusHydTestCase{ELEM_B, -1, 0, 4, 4, "B_qm1_r0_c4"},
                                           ValenceMinusHydTestCase{ELEM_B, 0, 0, 3, 3, "B_q0_r0_c3"},
                                           ValenceMinusHydTestCase{ELEM_Al, -1, 0, 4, 4, "Al_qm1_r0_c4"}));

// Group 5
INSTANTIATE_TEST_SUITE_P(VMH_Group5, CalcValenceMinusHydTest,
                         ::testing::Values(ValenceMinusHydTestCase{ELEM_N, 1, 0, 4, 4, "N_qp1_r0_c4"},
                                           ValenceMinusHydTestCase{ELEM_N, 2, 0, 3, 3, "N_qp2_r0_c3"},
                                           ValenceMinusHydTestCase{ELEM_P, 1, 0, 4, 4, "P_qp1_r0_c4"},
                                           ValenceMinusHydTestCase{ELEM_Sb, 1, 0, 3, 3, "Sb_qp1_r0_c3"},
                                           ValenceMinusHydTestCase{ELEM_As, 1, 0, 3, 3, "As_qp1_r0_c3"}));

// Group 6
INSTANTIATE_TEST_SUITE_P(VMH_Group6, CalcValenceMinusHydTest,
                         ::testing::Values(ValenceMinusHydTestCase{ELEM_O, 1, 0, 3, 3, "O_qp1_r0_c3"},
                                           ValenceMinusHydTestCase{ELEM_S, 1, 0, 3, 3, "S_qp1_r0_c3"},
                                           ValenceMinusHydTestCase{ELEM_S, -1, 0, 1, 1, "S_qm1_r0_c1"}));

// Group 7
INSTANTIATE_TEST_SUITE_P(VMH_Group7, CalcValenceMinusHydTest,
                         ::testing::Values(ValenceMinusHydTestCase{ELEM_Cl, 1, 0, 2, 2, "Cl_qp1_r0_c2"},
                                           ValenceMinusHydTestCase{ELEM_I, 1, 0, 2, 2, "I_qp1_r0_c2"}));

// Default path
INSTANTIATE_TEST_SUITE_P(VMH_Default, CalcValenceMinusHydTest,
                         ::testing::Values(ValenceMinusHydTestCase{ELEM_C, 0, 0, 4, 4, "C_q0_r0_c4"},
                                           ValenceMinusHydTestCase{ELEM_Si, 0, 0, 4, 4, "Si_q0_r0_c4"},
                                           ValenceMinusHydTestCase{ELEM_H, 0, 0, 1, 1, "H_q0_r0_c1"}));

// ============================================================================
// calcValenceResult convenience wrapper
// ============================================================================
TEST(CalcValenceResultTest, ValidCarbon)
{
    auto r = Element::calcValenceResult(ELEM_C, 0, 0, 2);
    EXPECT_TRUE(r.valid);
    EXPECT_FALSE(r.nonStandard);
    EXPECT_EQ(r.valence, 4);
    EXPECT_EQ(r.implicit_h, 2);
}

TEST(CalcValenceResultTest, InvalidCarbon)
{
    auto r = Element::calcValenceResult(ELEM_C, 0, 0, 5);
    // Hybrid contract: bad valence -> valid=false, nonStandard=true,
    // collapses to valence=conn, implicit_h=0.
    EXPECT_FALSE(r.valid);
    EXPECT_TRUE(r.nonStandard);
    EXPECT_EQ(r.valence, 5);
    EXPECT_EQ(r.implicit_h, 0);
}

TEST(CalcValenceResultTest, TransitionMetal)
{
    auto r = Element::calcValenceResult(ELEM_Fe, 0, 0, 3);
    EXPECT_TRUE(r.valid);
    EXPECT_FALSE(r.nonStandard);
    EXPECT_EQ(r.valence, 3);
    EXPECT_EQ(r.implicit_h, 0);
}

TEST(CalcValenceResultTest, ChargedNitrogen)
{
    auto r = Element::calcValenceResult(ELEM_N, 1, 0, 4);
    EXPECT_TRUE(r.valid);
    EXPECT_FALSE(r.nonStandard);
    EXPECT_EQ(r.valence, 4);
    EXPECT_EQ(r.implicit_h, 0);
}

// ============================================================================
// baseValence pure function
// ============================================================================
struct BaseValenceTestCase
{
    int eff;
    int expected;
    const char* name;
};

class BaseValenceTest : public ::testing::TestWithParam<BaseValenceTestCase>
{
};

TEST_P(BaseValenceTest, Check)
{
    const auto& tc = GetParam();
    EXPECT_EQ(Element::baseValence(tc.eff), tc.expected) << "eff=" << tc.eff;
}

INSTANTIATE_TEST_SUITE_P(BaseValence, BaseValenceTest,
                         ::testing::Values(BaseValenceTestCase{1, 1, "eff1"}, BaseValenceTestCase{2, 2, "eff2"}, BaseValenceTestCase{3, 3, "eff3"},
                                           BaseValenceTestCase{4, 4, "eff4"}, BaseValenceTestCase{5, 3, "eff5"}, BaseValenceTestCase{6, 2, "eff6"},
                                           BaseValenceTestCase{7, 1, "eff7"}, BaseValenceTestCase{8, 0, "eff8"}),
                         [](const ::testing::TestParamInfo<BaseValenceTestCase>& info) { return info.param.name; });

// ============================================================================
// BIOVIA_2017 ValenceModel tests
// ============================================================================

struct Biovia2017TestCase
{
    int elem;
    int charge;
    int radical;
    int conn;
    bool expect_valid;
    int expect_hyd;
    const char* label;
};

class Biovia2017ValenceTest : public ::testing::TestWithParam<Biovia2017TestCase>
{
};

TEST_P(Biovia2017ValenceTest, VerifyResult)
{
    const auto& tc = GetParam();
    int valence = 0, hyd = 0;
    const auto& model = ValenceModel::instance(ValenceMode::BIOVIA_2017);
    bool result = model.calcValence(tc.elem, tc.charge, tc.radical, tc.conn, valence, hyd, false);
    EXPECT_EQ(tc.expect_valid, result) << "  case: " << tc.label;
    if (tc.expect_valid && result)
    {
        EXPECT_EQ(tc.expect_hyd, hyd) << "  case: " << tc.label << " (hyd mismatch)";
    }
}

// BIOVIA_2017: metals get hyd=0, valid=true (intercepted by BIOVIA_2017 model)
INSTANTIATE_TEST_SUITE_P(
    Biovia2017_Metals, Biovia2017ValenceTest,
    ::testing::Values(Biovia2017TestCase{ELEM_Li, 0, 0, 0, true, 0, "Li_q0_c0_metal"}, Biovia2017TestCase{ELEM_Na, 0, 0, 0, true, 0, "Na_q0_c0_metal"},
                      Biovia2017TestCase{ELEM_K, 0, 0, 0, true, 0, "K_q0_c0_metal"}, Biovia2017TestCase{ELEM_Al, 0, 0, 0, true, 0, "Al_q0_c0_metal"},
                      Biovia2017TestCase{ELEM_Ga, 0, 0, 0, true, 0, "Ga_q0_c0_metal"}, Biovia2017TestCase{ELEM_In, 0, 0, 0, true, 0, "In_q0_c0_metal"},
                      Biovia2017TestCase{ELEM_Tl, 0, 0, 0, true, 0, "Tl_q0_c0_metal"}, Biovia2017TestCase{ELEM_Sn, 0, 0, 0, true, 0, "Sn_q0_c0_metal"},
                      Biovia2017TestCase{ELEM_Pb, 0, 0, 0, true, 0, "Pb_q0_c0_metal"}, Biovia2017TestCase{ELEM_Fe, 0, 0, 3, true, 0, "Fe_q0_c3_metal"},
                      Biovia2017TestCase{ELEM_Cu, 0, 0, 2, true, 0, "Cu_q0_c2_metal"}, Biovia2017TestCase{ELEM_Li, 0, 0, 1, true, 0, "Li_q0_c1_metal"},
                      Biovia2017TestCase{ELEM_Al, 0, 0, 3, true, 0, "Al_q0_c3_metal"}));

// BIOVIA_2017: charged non-metals behave SAME as DEFAULT (standard valence formula)
INSTANTIATE_TEST_SUITE_P(
    Biovia2017_ChargedNonmetals, Biovia2017ValenceTest,
    ::testing::Values(Biovia2017TestCase{ELEM_C, 1, 0, 0, true, 3, "C_qp1_c0_charged"}, Biovia2017TestCase{ELEM_C, -1, 0, 0, true, 3, "C_qm1_c0_charged"},
                      Biovia2017TestCase{ELEM_N, -1, 0, 0, true, 2, "N_qm1_c0_charged"}, Biovia2017TestCase{ELEM_N, 1, 0, 0, true, 4, "N_qp1_c0_charged"},
                      Biovia2017TestCase{ELEM_O, 1, 0, 0, true, 3, "O_qp1_c0_charged"}, Biovia2017TestCase{ELEM_O, -1, 0, 0, true, 1, "O_qm1_c0_charged"},
                      Biovia2017TestCase{ELEM_S, -1, 0, 0, true, 1, "S_qm1_c0_charged"}, Biovia2017TestCase{ELEM_H, 1, 0, 0, true, 0, "H_qp1_c0_charged"},
                      Biovia2017TestCase{ELEM_H, -1, 0, 0, true, 0, "H_qm1_c0_charged"}));

// BIOVIA_2017: Al⁻ is the only exception — gets normal valence=4 like DEFAULT
INSTANTIATE_TEST_SUITE_P(Biovia2017_AlException, Biovia2017ValenceTest,
                         ::testing::Values(Biovia2017TestCase{ELEM_Al, -1, 0, 0, true, 4, "Al_qm1_c0_exception"},
                                           Biovia2017TestCase{ELEM_Al, -1, 0, 4, true, 0, "Al_qm1_c4_exception"},
                                           Biovia2017TestCase{ELEM_Al, 1, 0, 0, true, 0, "Al_qp1_c0_intercepted"},
                                           Biovia2017TestCase{ELEM_Al, -2, 0, 0, true, 0, "Al_qm2_c0_intercepted"}));

// BIOVIA_2017: neutral non-metals behave SAME as DEFAULT
INSTANTIATE_TEST_SUITE_P(
    Biovia2017_NeutralNonmetals, Biovia2017ValenceTest,
    ::testing::Values(Biovia2017TestCase{ELEM_C, 0, 0, 0, true, 4, "C_q0_c0_nonmetal"}, Biovia2017TestCase{ELEM_N, 0, 0, 0, true, 3, "N_q0_c0_nonmetal"},
                      Biovia2017TestCase{ELEM_O, 0, 0, 0, true, 2, "O_q0_c0_nonmetal"}, Biovia2017TestCase{ELEM_F, 0, 0, 0, true, 1, "F_q0_c0_nonmetal"},
                      Biovia2017TestCase{ELEM_H, 0, 0, 0, true, 1, "H_q0_c0_nonmetal"}, Biovia2017TestCase{ELEM_S, 0, 0, 0, true, 2, "S_q0_c0_nonmetal"},
                      Biovia2017TestCase{ELEM_P, 0, 0, 0, true, 3, "P_q0_c0_nonmetal"}, Biovia2017TestCase{ELEM_B, 0, 0, 0, true, 3, "B_q0_c0_nonmetal"},
                      Biovia2017TestCase{ELEM_Si, 0, 0, 0, true, 4, "Si_q0_c0_nonmetal"}, Biovia2017TestCase{ELEM_Se, 0, 0, 0, true, 2, "Se_q0_c0_nonmetal"},
                      Biovia2017TestCase{ELEM_He, 0, 0, 0, true, 0, "He_q0_c0_noble"}, Biovia2017TestCase{ELEM_Ne, 0, 0, 0, true, 0, "Ne_q0_c0_noble"},
                      Biovia2017TestCase{ELEM_Ar, 0, 0, 0, true, 0, "Ar_q0_c0_noble"}));

// Comparison: all 22 neutral non-metals give same result in BIOVIA_2009 and BIOVIA_2017
TEST(ValenceModelComparison, NeutralNonmetals_SameInBothModes)
{
    const auto& biovia2009 = ValenceModel::instance(ValenceMode::BIOVIA_2009);
    const auto& biovia2017 = ValenceModel::instance(ValenceMode::BIOVIA_2017);

    int nonmetals[] = {ELEM_H,  ELEM_He, ELEM_B,  ELEM_C,  ELEM_N,  ELEM_O,  ELEM_F,  ELEM_Ne, ELEM_Si, ELEM_P,  ELEM_S,
                       ELEM_Cl, ELEM_Ar, ELEM_As, ELEM_Se, ELEM_Br, ELEM_Kr, ELEM_Te, ELEM_I,  ELEM_Xe, ELEM_At, ELEM_Rn};

    for (int elem : nonmetals)
    {
        int v1 = 0, h1 = 0, v2 = 0, h2 = 0;
        bool r1 = biovia2009.calcValence(elem, 0, 0, 0, v1, h1, false);
        bool r2 = biovia2017.calcValence(elem, 0, 0, 0, v2, h2, false);
        EXPECT_EQ(r1, r2) << "Element " << elem << " valid mismatch";
        EXPECT_EQ(h1, h2) << "Element " << elem << " hyd mismatch";
    }
}

// Comparison: charged non-metals give same result in BIOVIA_2009 and BIOVIA_2017
TEST(ValenceModelComparison, ChargedNonmetals_SameInBothModes)
{
    const auto& biovia2009 = ValenceModel::instance(ValenceMode::BIOVIA_2009);
    const auto& biovia2017 = ValenceModel::instance(ValenceMode::BIOVIA_2017);

    struct IonCase
    {
        int elem;
        int charge;
    };
    IonCase ions[] = {{ELEM_C, 1}, {ELEM_C, -1}, {ELEM_N, 1}, {ELEM_N, -1}, {ELEM_O, 1},  {ELEM_O, -1}, {ELEM_S, 1},   {ELEM_S, -1},
                      {ELEM_H, 1}, {ELEM_H, -1}, {ELEM_P, 1}, {ELEM_P, -1}, {ELEM_B, -1}, {ELEM_F, -1}, {ELEM_Cl, -1}, {ELEM_Br, -1}};

    for (const auto& ion : ions)
    {
        int v1 = 0, h1 = 0, v2 = 0, h2 = 0;
        bool r1 = biovia2009.calcValence(ion.elem, ion.charge, 0, 0, v1, h1, false);
        bool r2 = biovia2017.calcValence(ion.elem, ion.charge, 0, 0, v2, h2, false);
        EXPECT_EQ(r1, r2) << "Element " << ion.elem << " charge " << ion.charge << " valid mismatch";
        EXPECT_EQ(h1, h2) << "Element " << ion.elem << " charge " << ion.charge << " hyd mismatch";
    }
}

// Comparison: BIOVIA_2009 model via ValenceModel matches Element::calcValence
TEST(ValenceModelComparison, Biovia2009ModelMatchesElement)
{
    const auto& model = ValenceModel::instance(ValenceMode::BIOVIA_2009);
    int test_elems[] = {ELEM_C, ELEM_N, ELEM_O, ELEM_Li, ELEM_Fe, ELEM_Al, ELEM_Sn};

    for (int elem : test_elems)
    {
        int v1 = 0, h1 = 0, v2 = 0, h2 = 0;
        bool r1 = Element::calcValence(elem, 0, 0, 0, v1, h1, false);
        bool r2 = model.calcValence(elem, 0, 0, 0, v2, h2, false);
        EXPECT_EQ(r1, r2) << "Element " << elem << " valid mismatch";
        EXPECT_EQ(v1, v2) << "Element " << elem << " valence mismatch";
        EXPECT_EQ(h1, h2) << "Element " << elem << " hyd mismatch";
    }
}

// ============================================================================
// E2E API tests: indigoSetOption("valence-mode", ...) through C API
// ============================================================================

#include "common.h"
#include <indigo.h>

class ValenceModeApiTest : public IndigoApiTest
{
};

// Minimal V2000 MOL: single Al atom, no bonds
static const char* kAlMol = "\n  Indigo  0000000002D\n\n"
                            "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
                            "    0.0000    0.0000    0.0000 Al  0  0  0  0  0  0  0  0  0  0  0  0\n"
                            "M  END\n";

// Minimal V2000 MOL: single C atom, no bonds
static const char* kCarbonMol = "\n  Indigo  0000000002D\n\n"
                                "  1  0  0  0  0  0  0  0  0  0999 V2000\n"
                                "    0.0000    0.0000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n"
                                "M  END\n";

// Al from MOL: biovia-2017 → hyd=0 (metal intercepted), biovia-2009 → hyd=3 (standard formula)
TEST_F(ValenceModeApiTest, Metal_DiffersBetweenModes)
{
    // biovia-2017: Al is intercepted as metal → hyd=0
    indigoSetOption("valence-mode", "biovia-2017");
    int mol_post = indigoLoadMoleculeFromString(kAlMol);
    ASSERT_NE(mol_post, -1);
    int atom_post = indigoGetAtom(mol_post, 0);
    ASSERT_NE(atom_post, -1);
    int hyd_post = indigoCountImplicitHydrogens(atom_post);
    EXPECT_EQ(hyd_post, 0) << "Al in biovia-2017 should have 0 implicit H (metal intercepted)";

    // biovia-2009: Al gets standard formula → valence=3, hyd=3
    indigoSetOption("valence-mode", "biovia-2009");
    int mol_pre = indigoLoadMoleculeFromString(kAlMol);
    ASSERT_NE(mol_pre, -1);
    int atom_pre = indigoGetAtom(mol_pre, 0);
    ASSERT_NE(atom_pre, -1);
    int hyd_pre = indigoCountImplicitHydrogens(atom_pre);
    EXPECT_EQ(hyd_pre, 3) << "Al in biovia-2009 should have 3 implicit H (standard formula)";
}

// C from MOL: same result in both modes (non-metal is never intercepted)
TEST_F(ValenceModeApiTest, Nonmetal_SameInBothModes)
{
    indigoSetOption("valence-mode", "biovia-2017");
    int mol_post = indigoLoadMoleculeFromString(kCarbonMol);
    ASSERT_NE(mol_post, -1);
    int hyd_post = indigoCountImplicitHydrogens(indigoGetAtom(mol_post, 0));

    indigoSetOption("valence-mode", "biovia-2009");
    int mol_pre = indigoLoadMoleculeFromString(kCarbonMol);
    ASSERT_NE(mol_pre, -1);
    int hyd_pre = indigoCountImplicitHydrogens(indigoGetAtom(mol_pre, 0));

    EXPECT_EQ(hyd_post, 4);
    EXPECT_EQ(hyd_pre, 4);
    EXPECT_EQ(hyd_post, hyd_pre) << "Non-metal should behave identically in both modes";
}

// SMILES bypass: valence-mode option does NOT affect SMILES loading.
// [Al] bracket atom → hcount=0 per SMILES spec (omitted hcount = 0).
// Both modes should give the same result for SMILES input.
TEST_F(ValenceModeApiTest, Smiles_BypassesValenceMode)
{
    indigoSetOption("valence-mode", "biovia-2017");
    int mol_post = indigoLoadMoleculeFromString("[Al]");
    ASSERT_NE(mol_post, -1);
    int hyd_post = indigoCountImplicitHydrogens(indigoGetAtom(mol_post, 0));

    indigoSetOption("valence-mode", "biovia-2009");
    int mol_pre = indigoLoadMoleculeFromString("[Al]");
    ASSERT_NE(mol_pre, -1);
    int hyd_pre = indigoCountImplicitHydrogens(indigoGetAtom(mol_pre, 0));

    EXPECT_EQ(hyd_post, hyd_pre) << "SMILES loading must give same result regardless of valence-mode";
}

// Option getter/setter roundtrip
TEST_F(ValenceModeApiTest, OptionRoundtrip)
{
    indigoSetOption("valence-mode", "biovia-2017");
    const char* val1 = indigoGetOption("valence-mode");
    EXPECT_STREQ(val1, "biovia-2017");

    indigoSetOption("valence-mode", "biovia-2009");
    const char* val2 = indigoGetOption("valence-mode");
    EXPECT_STREQ(val2, "biovia-2009");

    // Backward compatibility: old names still accepted
    indigoSetOption("valence-mode", "post-2014");
    const char* val3 = indigoGetOption("valence-mode");
    EXPECT_STREQ(val3, "biovia-2017") << "'post-2014' is an alias for 'biovia-2017'";

    indigoSetOption("valence-mode", "pre-2014");
    const char* val4 = indigoGetOption("valence-mode");
    EXPECT_STREQ(val4, "biovia-2009") << "'pre-2014' is an alias for 'biovia-2009'";

    indigoSetOption("valence-mode", "default");
    const char* val5 = indigoGetOption("valence-mode");
    EXPECT_STREQ(val5, "biovia-2009") << "'default' is an alias for 'biovia-2009'";
}
