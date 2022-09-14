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

#include "molecule/crippen.h"
#include "molecule/molecule_ionize.h"

#include "common.h"

using namespace std;
using namespace indigo;

class IndigoCorePkaTest : public IndigoCoreTest
{
};

TEST_F(IndigoCorePkaTest, simple)
{
    Molecule molecule;
    loadMolecule("Brc(cc1)cc(C2[P+](c3ccccc3)(c3ccccc3)c3ccccc3)c1-c1c2cccc1", molecule);
    IonizeOptions ionizeOptions;
    EXPECT_FLOAT_EQ(43, MoleculePkaModel::getMoleculeAcidPkaValue(molecule, ionizeOptions));
}

TEST_F(IndigoCorePkaTest, advanced)
{
    Molecule molecule;
    loadMolecule("Brc(cc1)cc(C2[P+](c3ccccc3)(c3ccccc3)c3ccccc3)c1-c1c2cccc1", molecule);
    molecule.dearomatize();
    IonizeOptions ionizeOptions(IonizeOptions::PKA_MODEL_ADVANCED, 10, 2);
    EXPECT_FLOAT_EQ(12.84, MoleculePkaModel::getMoleculeAcidPkaValue(molecule, ionizeOptions));
}

TEST_F(IndigoCorePkaTest, crippen)
{
    Molecule molecule;
    loadMolecule("Brc(cc1)cc(C2[P+](c3ccccc3)(c3ccccc3)c3ccccc3)c1-c1c2cccc1", molecule);
    EXPECT_FLOAT_EQ(12.535, Crippen::pKa(molecule));
}

TEST_F(IndigoCorePkaTest, atomKey)
{
    Molecule molecule;
    loadMolecule("C", molecule);
    Array<char> fp;
    MoleculePkaModel::getAtomLocalKey(molecule, 0, fp);
    EXPECT_EQ(fp[0], '6');
    EXPECT_EQ(fp[1], '4');
}


TEST_F(IndigoCorePkaTest, buildPkaModel)
{
    Molecule molecule;
    loadMolecule("C", molecule);
    Array<char> fp;
    MoleculePkaModel::buildPkaModel(20, 0.0, dataPath("molecules/pka/pka_in_water.sdf").c_str());
}
