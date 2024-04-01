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
#include <molecule/crippen.h>
#include <molecule/hybridization.h>
#include <molecule/lipinski.h>
#include <molecule/molecule_mass.h>
#include <molecule/smiles_loader.h>
#include <molecule/smiles_saver.h>
#include <molecule/tpsa.h>

#include "common.h"

using namespace std;
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
    EXPECT_NEAR(0, TPSA::calculate(molecule), 0.01);
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

TEST_F(IndigoCoreMoleculeTest, numRotatableBonds)
{
    Molecule molecule;
    loadMolecule("C", molecule);
    EXPECT_EQ(0, Lipinski::getNumRotatableBonds(molecule));
    loadMolecule("CC", molecule);
    EXPECT_EQ(0, Lipinski::getNumRotatableBonds(molecule));
    loadMolecule("CCC", molecule);
    EXPECT_EQ(0, Lipinski::getNumRotatableBonds(molecule));
    loadMolecule("CCCC", molecule);
    EXPECT_EQ(1, Lipinski::getNumRotatableBonds(molecule));
    loadMolecule("O=C([O-])c1ccccc1O", molecule);
    EXPECT_EQ(1, Lipinski::getNumRotatableBonds(molecule));
    loadMolecule("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", molecule);
    EXPECT_EQ(6, Lipinski::getNumRotatableBonds(molecule));
    loadMolecule("COP(=O)(OC)OC(=CCl)C1=CC(=C(C=C1Cl)Cl)Cl", molecule);
    EXPECT_EQ(5, Lipinski::getNumRotatableBonds(molecule));
}

TEST_F(IndigoCoreMoleculeTest, numHydrogenBondAcceptors)
{
    Molecule molecule;
    loadMolecule("C", molecule);
    EXPECT_EQ(0, Lipinski::getNumHydrogenBondAcceptors(molecule));
    loadMolecule("O=C([O-])c1ccccc1O", molecule);
    EXPECT_EQ(3, Lipinski::getNumHydrogenBondAcceptors(molecule));
    loadMolecule("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", molecule);
    EXPECT_EQ(9, Lipinski::getNumHydrogenBondAcceptors(molecule));
    loadMolecule("COP(=O)(OC)OC(=CCl)C1=CC(=C(C=C1Cl)Cl)Cl", molecule);
    EXPECT_EQ(4, Lipinski::getNumHydrogenBondAcceptors(molecule));
}

TEST_F(IndigoCoreMoleculeTest, numHydrogenBondDonors)
{
    Molecule molecule;
    loadMolecule("C", molecule);
    EXPECT_EQ(0, Lipinski::getNumHydrogenBondDonors(molecule));
    loadMolecule("O=C([O-])c1ccccc1O", molecule);
    EXPECT_EQ(1, Lipinski::getNumHydrogenBondDonors(molecule));
    loadMolecule("C1=CC=NC(=C1)NS(=O)(=O)C2=CC=C(C=C2)N=NC3=CC(=C(C=C3)O)C(=O)O", molecule);
    EXPECT_EQ(3, Lipinski::getNumHydrogenBondDonors(molecule));
    loadMolecule("COP(=O)(OC)OC(=CCl)C1=CC(=C(C=C1Cl)Cl)Cl", molecule);
    EXPECT_EQ(0, Lipinski::getNumHydrogenBondDonors(molecule));
}

TEST_F(IndigoCoreMoleculeTest, logP)
{
    Molecule molecule;
    {
        loadMolecule(METHANE, molecule);
        EXPECT_NEAR(0.6361, Crippen::logP(molecule), 0.01);
    }
    {
        loadMolecule("C[U]", molecule);
        EXPECT_NEAR(0.5838, Crippen::logP(molecule), 0.01);
    }
    {
        loadMolecule(BENZENE, molecule);
        EXPECT_NEAR(1.6865, Crippen::logP(molecule), 0.01);
    }
    {
        loadMolecule(CAFFEINE, molecule);
        EXPECT_NEAR(0.06, Crippen::logP(molecule), 0.01);
    }
    {
        loadMolecule(SULFASALAZINE, molecule);
        EXPECT_NEAR(3.7, Crippen::logP(molecule), 0.01);
    }
}

TEST_F(IndigoCoreMoleculeTest, molarRefractivity)
{
    Molecule molecule;
    {
        loadMolecule(METHANE, molecule);
        EXPECT_NEAR(6.731, Crippen::molarRefractivity(molecule), 0.01);
    }
    {
        loadMolecule(BENZENE, molecule);
        EXPECT_NEAR(26.442, Crippen::molarRefractivity(molecule), 0.01);
    }
    {
        loadMolecule(CAFFEINE, molecule);
        EXPECT_NEAR(49.1, Crippen::molarRefractivity(molecule), 0.01);
    }
    {
        loadMolecule(SULFASALAZINE, molecule);
        EXPECT_NEAR(100.73, Crippen::molarRefractivity(molecule), 0.01);
    }
}

TEST_F(IndigoCoreMoleculeTest, hybridization)
{
    Molecule molecule;
    {
        loadMolecule(METHANE, molecule);
        EXPECT_EQ(Hybridization::SP3, HybridizationCalculator::calculate(molecule, 0));
    }
    {
        loadMolecule(BENZENE_AROMATIC, molecule);
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(Hybridization::SP2, HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("OC1=CC=CC=C1", molecule); // Phenol
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(Hybridization::SP2, HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("[C-]#[O+]", molecule); // Carbon monoxide
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(Hybridization::SP, HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("O=C=O", molecule); // Carbon monoxide
        array<Hybridization, 3> expected{Hybridization::SP2, Hybridization::SP, Hybridization::SP2};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("C#N", molecule);
        array<Hybridization, 2> expected{Hybridization::SP, Hybridization::SP};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("O=C(N)C", molecule);
        array<Hybridization, 4> expected{Hybridization::SP2, Hybridization::SP2, Hybridization::SP, Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("OS(=O)(=O)O", molecule);
        array<Hybridization, 5> expected{Hybridization::SP3, Hybridization::SP3, Hybridization::SP2, Hybridization::SP2, Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("N(=O)O", molecule);
        array<Hybridization, 3> expected{Hybridization::SP2, Hybridization::SP2, Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("N(=O)O", molecule);
        array<Hybridization, 3> expected{Hybridization::SP2, Hybridization::SP2, Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("O=[Xe](=O)(=O)=O", molecule);
        array<Hybridization, 5> expected{Hybridization::SP2, Hybridization::SP3, Hybridization::SP2, Hybridization::SP2, Hybridization::SP2};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("FS(F)(F)(F)(F)F", molecule);
        array<Hybridization, 7> expected{Hybridization::SP3, Hybridization::SP3D2, Hybridization::SP3, Hybridization::SP3,
                                         Hybridization::SP3, Hybridization::SP3,   Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("FS(F)(F)(F)(F)F", molecule);
        array<Hybridization, 7> expected{Hybridization::SP3, Hybridization::SP3D2, Hybridization::SP3, Hybridization::SP3,
                                         Hybridization::SP3, Hybridization::SP3,   Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("FBr(F)F", molecule);
        array<Hybridization, 4> expected{Hybridization::SP3, Hybridization::SP3D, Hybridization::SP3, Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("[Be](Cl)Cl", molecule);
        array<Hybridization, 3> expected{Hybridization::SP, Hybridization::SP3, Hybridization::SP3};
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(expected[i], HybridizationCalculator::calculate(molecule, i));
        }
    }
    {
        loadMolecule("C1=CC=CS1", molecule); // Thiophene
        for (auto i = molecule.vertexBegin(); i < molecule.vertexEnd(); i = molecule.vertexNext(i))
        {
            EXPECT_EQ(Hybridization::SP2, HybridizationCalculator::calculate(molecule, i));
        }
    }
}

TEST_F(IndigoCoreMoleculeTest, pKa)
{
    Molecule molecule;
    {
        loadMolecule("Oc1cc(cc([N+](C)(C)C)c1)C", molecule);
        EXPECT_NEAR(6.5, Crippen::pKa(molecule), 0.01);
    }
}

TEST_F(IndigoCoreMoleculeTest, atom_reorder_query)
{
    QueryMolecule molecule;
    loadQueryMolecule("[A:1][C:2](=[O:3])[OH1:4] |$R1;;;OH$|", molecule);
    EXPECT_STREQ("*C([OH])=O |$R1;;OH;$|", smiles(molecule).c_str());
}

TEST_F(IndigoCoreMoleculeTest, atom_reorder_nonquery)
{
    Molecule molecule;
    loadMolecule("SC(=N)O |$R1;;;OH$|", molecule);
    EXPECT_STREQ("SC(O)=N |$R1;;OH;$|", smiles(molecule).c_str());
}

TEST_F(IndigoCoreMoleculeTest, dearomatize_query)
{
    QueryMolecule molecule;
    AromaticityOptions opts;
    loadQueryMolecule("c1ccccc1", molecule);
    bool res = molecule.dearomatize(opts);
    EXPECT_EQ(res, true);
    std::string sm = smiles(molecule);
    // printf("%s", sm.c_str());
    EXPECT_STREQ("C1-C=C-C=C-C=1", sm.c_str());
}

TEST_F(IndigoCoreMoleculeTest, dearomatize_smarts)
{
    QueryMolecule molecule;
    AromaticityOptions opts;
    std::string mol = "c1ccccc1";
    BufferScanner scanner(mol.c_str());
    SmilesLoader loader(scanner);
    loader.loadSMARTS(molecule);
    bool res = molecule.dearomatize(opts);
    EXPECT_EQ(res, true);
    std::string sm;
    StringOutput out(sm);
    SmilesSaver saver(out);
    saver.smarts_mode = true;
    saver.saveQueryMolecule(molecule);
    // printf("%s", sm.c_str());
    EXPECT_STREQ("c1-c=c-c=c-c=1", sm.c_str());
}
