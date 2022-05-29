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

#include <base_cpp/output.h>
#include <molecule/molecule_mass.h>
#include <molecule/smiles_loader.h>
#include <molecule/tpsa.h>

#include "common.h"

using namespace indigo;

class IndigoCoreMoleculeTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreMoleculeTest, mass)
{
    Molecule molecule;
    loadMolecule("[81Br]", molecule);
    MoleculeMass mm;
    const auto m = mm.monoisotopicMass(molecule);
    ASSERT_NEAR(80.9163, m, 0.01);
}

TEST_F(IndigoCoreMoleculeTest, tpsa)
{
    Molecule molecule;
    loadMolecule("C", molecule);
    ASSERT_NEAR(0, TPSA::calculate(molecule), 0.01);
    loadMolecule("O=C([O-])c1ccccc1O", molecule);
    EXPECT_NEAR(60.36, TPSA::calculate(molecule), 0.1);
    loadMolecule("CC(=O)Oc1cccc1C(=O)[O-]", molecule);
    EXPECT_NEAR(66.43, TPSA::calculate(molecule), 0.1);
    loadMolecule("OCC(O)C(O)C(O)C(O)CO", molecule);
    EXPECT_NEAR(121.37, TPSA::calculate(molecule), 0.1);
    loadMolecule("C1CC1N2C=C(C(=O)C3=CC(=C(C=C32)N4CCNCC4)F)C(=O)O", molecule);
    EXPECT_NEAR(72.9, TPSA::calculate(molecule), 0.1);
    loadMolecule("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", molecule);
    EXPECT_NEAR(58.44, TPSA::calculate(molecule), 0.1);
    loadMolecule("C1CN1C2=NC(=NC(=N2)N3CC3)N4CC4", molecule);
    EXPECT_NEAR(47.7, TPSA::calculate(molecule), 0.1);
    loadMolecule("C1=CC(=CC=C1C(=O)NC(CCC(=O)O)C(=O)O)NCC2=CN=C3C(=N2)C(=O)NC(=N3)N", molecule);
    EXPECT_NEAR(209.0, TPSA::calculate(molecule), 0.1);
    loadMolecule("CC1(C(C1C(=O)OC(C#N)C2=CC(=CC=C2)OC3=CC=CC=C3)C=C(C(F)(F)F)Cl)C", molecule);
    EXPECT_NEAR(59.3, TPSA::calculate(molecule), 0.1);
    loadMolecule("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", molecule);
    EXPECT_NEAR(141.31, TPSA::calculate(molecule), 0.1);
    EXPECT_NEAR(149.69, TPSA::calculate(molecule, true), 0.1);
    loadMolecule("C1=CC=C2C(=C1)NC(=S)S2", molecule);
    EXPECT_NEAR(12.03, TPSA::calculate(molecule), 0.1);
    EXPECT_NEAR(69.4, TPSA::calculate(molecule, true), 0.1);
    loadMolecule("COP(=O)(OC)OC(=CCl)C1=CC(=C(C=C1Cl)Cl)Cl", molecule);
    EXPECT_NEAR(44.76, TPSA::calculate(molecule), 0.1);
    EXPECT_NEAR(54.57, TPSA::calculate(molecule, true), 0.1);
    loadMolecule("C(=O)(O)P(=O)(O)O", molecule);
    EXPECT_NEAR(94.8, TPSA::calculate(molecule), 0.1);
    EXPECT_NEAR(104.6, TPSA::calculate(molecule, true), 0.1);
}
