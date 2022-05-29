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
#include <base_cpp/scanner.h>
#include <molecule/cmf_loader.h>
#include <molecule/cmf_saver.h>
#include <molecule/cml_saver.h>
#include <molecule/molecule_cdxml_saver.h>
#include <molecule/molecule_mass.h>
#include <molecule/molecule_substructure_matcher.h>
#include <molecule/molfile_loader.h>
#include <molecule/query_molecule.h>
#include <molecule/sdf_loader.h>
#include <molecule/smiles_loader.h>

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
    ASSERT_NEAR(0, molecule.tpsa(), 0.01);
    loadMolecule("O=C([O-])c1ccccc1O", molecule);
    EXPECT_NEAR(60.36, molecule.tpsa(), 0.1);
    loadMolecule("CC(=O)Oc1cccc1C(=O)[O-]", molecule);
    EXPECT_NEAR(66.43, molecule.tpsa(), 0.1);
    loadMolecule("OCC(O)C(O)C(O)C(O)CO", molecule);
    EXPECT_NEAR(121.37, molecule.tpsa(), 0.1);
    loadMolecule("OCC(O)C(O)C(O)C(O)CO", molecule);
    EXPECT_NEAR(121.37, molecule.tpsa(), 0.1);
    loadMolecule("C1CC1N2C=C(C(=O)C3=CC(=C(C=C32)N4CCNCC4)F)C(=O)O", molecule);
    EXPECT_NEAR(72.9, molecule.tpsa(), 0.1);
    loadMolecule("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", molecule);
    EXPECT_NEAR(149.69, molecule.tpsa(), 0.1);
    loadMolecule("CN1C=NC2=C1C(=O)N(C(=O)N2C)C", molecule);
    EXPECT_NEAR(58.44, molecule.tpsa(), 0.1);
}
