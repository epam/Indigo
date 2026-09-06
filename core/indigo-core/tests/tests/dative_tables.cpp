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

// Element tables of the dative-bond valence model (ticket #3617, phase 2).
// The values come from the ticket specification, issue body as of 2026-08-04.

#include <gtest/gtest.h>

#include <set>

#include <molecule/dative_model.h>
#include <molecule/elements.h>

#include "common.h"

using namespace indigo;

class IndigoCoreDativeTablesTest : public IndigoCoreTest
{
};

// The five worked examples of the specification are the reference data points:
// if the tables are right, these are right.
TEST_F(IndigoCoreDativeTablesTest, specification_examples)
{
    // Example 1 — nickel
    EXPECT_EQ(10, dative::valenceElectrons(ELEM_Ni));
    EXPECT_EQ(9, dative::valenceOrbitals(ELEM_Ni));
    // Example 2 — carbon
    EXPECT_EQ(4, dative::valenceElectrons(ELEM_C));
    EXPECT_EQ(4, dative::valenceOrbitals(ELEM_C));
    // Example 3 — chlorine
    EXPECT_EQ(7, dative::valenceElectrons(ELEM_Cl));
    EXPECT_EQ(4, dative::valenceOrbitals(ELEM_Cl));
    // Example 4 — nitrogen
    EXPECT_EQ(5, dative::valenceElectrons(ELEM_N));
    EXPECT_EQ(4, dative::valenceOrbitals(ELEM_N));
    // Example 5 — praseodymium (Or0 changed 13 -> 16 in the 2026-08-04 revision)
    EXPECT_EQ(5, dative::valenceElectrons(ELEM_Pr));
    EXPECT_EQ(16, dative::valenceOrbitals(ELEM_Pr));
}

// Hydrogen and helium are the only elements with a single valence orbital.
TEST_F(IndigoCoreDativeTablesTest, first_period_has_one_orbital)
{
    EXPECT_EQ(1, dative::valenceElectrons(ELEM_H));
    EXPECT_EQ(1, dative::valenceOrbitals(ELEM_H));
    EXPECT_EQ(2, dative::valenceElectrons(ELEM_He));
    EXPECT_EQ(1, dative::valenceOrbitals(ELEM_He));
}

// This model counts a filled d10/f14 subshell as valent; the tables Indigo already
// had treat it as core. The divergence is intentional — see dative_model.h.
TEST_F(IndigoCoreDativeTablesTest, differs_from_existing_electron_tables_on_purpose)
{
    struct
    {
        int elem;
        int dative_value;
    } cases[] = {
        {ELEM_Cu, 11}, {ELEM_Zn, 12}, {ELEM_Ag, 11}, {ELEM_Cd, 12}, {ELEM_Au, 11}, {ELEM_Hg, 12}, {ELEM_Yb, 16}, {ELEM_No, 16},
    };

    for (const auto& c : cases)
    {
        EXPECT_EQ(c.dative_value, dative::valenceElectrons(c.elem)) << "element " << Element::toString(c.elem);
        EXPECT_NE(Element::getNumOuterElectrons(c.elem), dative::valenceElectrons(c.elem))
            << "element " << Element::toString(c.elem) << ": the divergence from getNumOuterElectrons is deliberate";
    }
}

// Lanthanides and actinides need f-orbitals, which Element::orbitals() cannot express.
TEST_F(IndigoCoreDativeTablesTest, f_block_has_sixteen_orbitals)
{
    for (int elem = ELEM_La; elem <= ELEM_Yb; elem++)
        EXPECT_EQ(16, dative::valenceOrbitals(elem)) << "lanthanide " << Element::toString(elem);
    for (int elem = ELEM_Ac; elem <= ELEM_No; elem++)
        EXPECT_EQ(16, dative::valenceOrbitals(elem)) << "actinide " << Element::toString(elem);

    // Lu and Lr close the f-block rows but belong to the d-block layout.
    EXPECT_EQ(9, dative::valenceOrbitals(ELEM_Lu));
    EXPECT_EQ(9, dative::valenceOrbitals(ELEM_Lr));
}

// Hypervalency is deliberately unsupported: p-block elements keep four orbitals even
// in period 3 and below, which is what makes Or negative for e.g. [SiF6]2- (Q2).
TEST_F(IndigoCoreDativeTablesTest, heavy_p_block_has_no_d_orbitals)
{
    for (int elem : {ELEM_Si, ELEM_P, ELEM_S, ELEM_Cl, ELEM_Ar, ELEM_Ge, ELEM_As, ELEM_Se, ELEM_Br, ELEM_Kr})
        EXPECT_EQ(4, dative::valenceOrbitals(elem)) << "element " << Element::toString(elem);
}

// Structural invariant: the orbital count is one of four values. A typo in any of the
// 118 rows almost certainly produces something outside this set.
TEST_F(IndigoCoreDativeTablesTest, every_orbital_count_is_from_the_allowed_set)
{
    const std::set<int> allowed = {1, 4, 9, 16};
    for (int elem = ELEM_MIN; elem < ELEM_MAX; elem++)
    {
        const int orbitals = dative::valenceOrbitals(elem);
        EXPECT_TRUE(allowed.count(orbitals) == 1) << "element " << Element::toString(elem) << " has Or0 = " << orbitals;
    }
}

// Structural invariant: every element is covered, and no electron count is absurd.
TEST_F(IndigoCoreDativeTablesTest, every_element_is_covered)
{
    for (int elem = ELEM_MIN; elem < ELEM_MAX; elem++)
    {
        const int electrons = dative::valenceElectrons(elem);
        EXPECT_GE(electrons, 1) << "element " << Element::toString(elem) << " has no electron count";
        EXPECT_LE(electrons, 16) << "element " << Element::toString(elem) << " has an implausible electron count";
    }
}

// An atom can never hold more electrons than its orbitals can take (2 per orbital).
TEST_F(IndigoCoreDativeTablesTest, electrons_fit_into_the_orbitals)
{
    for (int elem = ELEM_MIN; elem < ELEM_MAX; elem++)
        EXPECT_LE(dative::valenceElectrons(elem), 2 * dative::valenceOrbitals(elem)) << "element " << Element::toString(elem) << " is over-filled";
}

// Out-of-range input must be answered, not crash — callers pass raw atom numbers.
TEST_F(IndigoCoreDativeTablesTest, out_of_range_elements_return_zero)
{
    for (int elem : {-1, 0, static_cast<int>(ELEM_MAX), 999})
    {
        EXPECT_EQ(0, dative::valenceElectrons(elem));
        EXPECT_EQ(0, dative::valenceOrbitals(elem));
    }
}
