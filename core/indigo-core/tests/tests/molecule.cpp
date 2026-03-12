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
#include <molecule/molfile_loader.h>
#include <molecule/smiles_loader.h>
#include <molecule/smiles_saver.h>
#include <molecule/tpsa.h>

#include "common.h"
#include "molecule/elements.h"

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

// [Sapio] [CHEMBUGS-184] Stereo reaction molecules (from former stereo_reaction.rxn): load as mol blocks to verify parsing.
namespace
{
    const char* const stereoReactionReactant1 = R"(
  Mrv2305 05232323372D

 18 19  0  0  0  0            999 V2000
   -6.9420   -0.0902    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -7.6565   -0.5027    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -7.6565   -1.3278    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -6.9420   -1.7402    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
   -6.2275   -1.3278    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -6.2275   -0.5027    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
   -5.5357    0.0000    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0
   -4.7511   -0.2549    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -4.2661    0.4125    0.0000 C   0  0  1  0  0  0  0  0  0  0  0  0
   -4.7510    1.0800    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0
   -5.5356    0.8250    0.0000 C   0  0  1  0  0  0  0  0  0  0  0  0
   -3.4411    0.4125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -6.2031    1.3100    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   -4.4961    1.8646    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -5.4066    1.6399    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -8.3709   -1.7403    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -5.5130   -1.7403    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -2.8891    1.0256    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  9 10  1  0  0  0  0
 10 11  1  0  0  0  0
  7 11  1  0  0  0  0
  9 12  1  1  0  0  0
  3 16  2  0  0  0  0
  5 17  2  0  0  0  0
  8  9  1  0  0  0  0
  7  8  1  0  0  0  0
 10 14  1  6  0  0  0
 12 18  1  0  0  0  0
 11 15  1  1  0  0  0
 11 13  1  6  0  0  0
  1  6  1  0  0  0  0
  5  6  1  0  0  0  0
  7  6  1  1  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
M  STY  1   1 SUP
M  SAL   1  1  15
M  SBL   1  1  13
M  SMT   1 Me
M  SAP   1  1  15  11  1
M  END
)";
    const char* const stereoReactionReactant2 = R"(
  Mrv2305 05232323372D

 24 26  0  0  0  0            999 V2000
   -4.7105   -3.1000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1382   -0.2125    0.0000 Cl  0  0  0  0  0  0  0  0  0  0  0  0
   -4.7106   -2.2750    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   -3.9961   -1.8625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.9961   -1.0375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.2816   -0.6250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5671   -1.0375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5671   -1.8625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -3.2816   -2.2750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.8527   -0.6250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.8527    0.2000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5672    0.6126    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -2.5671    1.4375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.8527    1.8500    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1382    1.4376    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1382    0.6125    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1382   -1.0375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.4237   -0.6250    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.2907   -1.0375    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    0.2907   -1.8625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.0052   -2.2750    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    1.7197   -1.8625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.4237   -2.2750    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.1382   -1.8625    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
 10  2  1  0  0  0  0
  1  3  1  0  0  0  0
  3  4  1  0  0  0  0
  4  5  2  0  0  0  0
  4  9  1  0  0  0  0
  5  6  1  0  0  0  0
  6  7  2  0  0  0  0
  7  8  1  0  0  0  0
  7 10  1  0  0  0  0
  8  9  2  0  0  0  0
 10 11  1  0  0  0  0
 10 17  1  0  0  0  0
 11 12  2  0  0  0  0
 11 16  1  0  0  0  0
 12 13  1  0  0  0  0
 13 14  2  0  0  0  0
 14 15  1  0  0  0  0
 15 16  2  0  0  0  0
 17 18  2  0  0  0  0
 17 24  1  0  0  0  0
 18 19  1  0  0  0  0
 19 20  2  0  0  0  0
 20 21  1  0  0  0  0
 20 23  1  0  0  0  0
 21 22  1  0  0  0  0
 23 24  2  0  0  0  0
M  STY  1   1 SUP
M  SAL   1 15   1   3   4   5   6   7   8   9  10  11  12  13  14  15  16
M  SAL   1  8  17  18  19  20  21  22  23  24
M  SBL   1  1   1
M  SMT   1 DMTr
M  SAP   1  1  10   2  1
M  END
)";
    const char* const stereoReactionProduct = R"(
  Mrv2305 05232323372D

 41 45  0  0  0  0            999 V2000
    2.9291   -0.1300    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.2146   -0.5425    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.2146   -1.3676    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    2.9291   -1.7800    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    3.6436   -1.3676    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.6436   -0.5425    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0
    4.3354   -0.0398    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0
    5.1200   -0.2947    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    5.6049    0.3727    0.0000 C   0  0  1  0  0  0  0  0  0  0  0  0
    5.1200    1.0402    0.0000 C   0  0  2  0  0  0  0  0  0  0  0  0
    4.3354    0.7852    0.0000 C   0  0  1  0  0  0  0  0  0  0  0  0
    6.4299    0.3727    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    3.6680    1.2701    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
    5.3749    1.8248    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    4.4645    1.6000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    1.5002   -1.7801    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    4.3581   -1.7801    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    6.9820    0.9858    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   11.5195    1.7003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   11.1070    0.9858    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
   10.2820    0.9858    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.8695    0.2713    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0445    0.2713    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    8.6320    0.9858    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.0445    1.7003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    9.8695    1.7003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.8070    0.9858    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.3945    0.2713    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.8070   -0.4432    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.3945   -1.1576    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.5695   -1.1576    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.1570   -0.4432    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.5695    0.2713    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.3945    1.7003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.5695    1.7003    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.1570    2.4147    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.5695    3.1292    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    6.1570    3.8437    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0
    5.3320    3.8437    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.3945    3.1292    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
    7.8070    2.4147    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  2  0  0  0  0
  2  3  1  0  0  0  0
  9 10  1  0  0  0  0
 10 11  1  0  0  0  0
  7 11  1  0  0  0  0
  9 12  1  1  0  0  0
  3 16  2  0  0  0  0
  5 17  2  0  0  0  0
  8  9  1  0  0  0  0
  7  8  1  0  0  0  0
 10 14  1  6  0  0  0
 12 18  1  0  0  0  0
 11 15  1  1  0  0  0
 11 13  1  6  0  0  0
  1  6  1  0  0  0  0
  5  6  1  0  0  0  0
  7  6  1  1  0  0  0
  3  4  1  0  0  0  0
  4  5  1  0  0  0  0
 18 27  1  0  0  0  0
 19 20  1  0  0  0  0
 20 21  1  0  0  0  0
 21 22  2  0  0  0  0
 21 26  1  0  0  0  0
 22 23  1  0  0  0  0
 23 24  2  0  0  0  0
 24 25  1  0  0  0  0
 24 27  1  0  0  0  0
 25 26  2  0  0  0  0
 27 28  1  0  0  0  0
 27 34  1  0  0  0  0
 28 29  2  0  0  0  0
 28 33  1  0  0  0  0
 29 30  1  0  0  0  0
 30 31  2  0  0  0  0
 31 32  1  0  0  0  0
 32 33  2  0  0  0  0
 34 35  2  0  0  0  0
 34 41  1  0  0  0  0
 35 36  1  0  0  0  0
 36 37  2  0  0  0  0
 37 38  1  0  0  0  0
 37 40  1  0  0  0  0
 38 39  1  0  0  0  0
 40 41  2  0  0  0  0
M  STY  2   1 SUP   2 SUP
M  SAL   1  1  15
M  SBL   1  1  13
M  SMT   1 Me
M  SAP   1  1  15  11  1
M  SAL   2 15  19  20  21  22  23  24  25  26  27  28  29  30  31  32  33
M  SAL   2  8  34  35  36  37  38  39  40  41
M  SBL   2  1  20
M  SMT   2 DMTr
M  SAP   2  1  27  18  1
M  END
)";
}

TEST_F(IndigoCoreMoleculeTest, stereoReactionReactant1)
{
    Molecule mol;
    BufferScanner scanner(stereoReactionReactant1);
    MolfileLoader loader(scanner);
    loader.loadMolecule(mol);
    EXPECT_EQ(18, mol.vertexCount());
    EXPECT_EQ(19, mol.edgeCount());
}

TEST_F(IndigoCoreMoleculeTest, stereoReactionReactant2)
{
    Molecule mol;
    BufferScanner scanner(stereoReactionReactant2);
    MolfileLoader loader(scanner);
    loader.loadMolecule(mol);
    EXPECT_EQ(24, mol.vertexCount());
    EXPECT_EQ(26, mol.edgeCount());
}

TEST_F(IndigoCoreMoleculeTest, stereoReactionProduct)
{
    Molecule mol;
    BufferScanner scanner(stereoReactionProduct);
    MolfileLoader loader(scanner);
    loader.loadMolecule(mol);
    EXPECT_EQ(41, mol.vertexCount());
    EXPECT_EQ(45, mol.edgeCount());
}