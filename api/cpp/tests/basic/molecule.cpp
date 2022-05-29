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

#include <IndigoMolecule.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Molecule, molecularWeight)
{
    const auto& session = IndigoSession::create();
    {
        const auto& molecule = session->loadMolecule(METHANE);
        EXPECT_NEAR(16.042, molecule.molecularWeight(), 0.01);
    }
    {
        const auto& molecule = session->loadMolecule(CAFFEINE);
        EXPECT_NEAR(194.191, molecule.molecularWeight(), 0.01);
    }
    {
        const auto& molecule = session->loadMolecule(SULFASALAZINE);
        EXPECT_NEAR(398.393, molecule.molecularWeight(), 0.01);
    }
}

TEST(Molecule, tpsa)
{
    const auto& session = IndigoSession::create();
    {
        const auto& molecule = session->loadMolecule(METHANE);
        EXPECT_NEAR(0.0, molecule.tpsa(), 0.01);
        EXPECT_NEAR(0.0, molecule.tpsa(true), 0.01);
    }
    {
        const auto& molecule = session->loadMolecule(CAFFEINE);
        EXPECT_NEAR(58.44, molecule.tpsa(), 0.01);
        EXPECT_NEAR(58.44, molecule.tpsa(true), 0.01);
    }
    {
        const auto& molecule = session->loadMolecule(SULFASALAZINE);
        EXPECT_NEAR(141.31, molecule.tpsa(), 0.01);
        EXPECT_NEAR(149.69, molecule.tpsa(true), 0.01);
    }
}

TEST(Molecule, numRotatableBonds)
{
    const auto& session = IndigoSession::create();
    {
        const auto& molecule = session->loadMolecule(METHANE);
        EXPECT_EQ(0, molecule.numRotatableBonds());
    }
    {
        const auto& molecule = session->loadMolecule(CAFFEINE);
        EXPECT_EQ(0, molecule.numRotatableBonds());
    }
    {
        const auto& molecule = session->loadMolecule(SULFASALAZINE);
        EXPECT_EQ(6, molecule.numRotatableBonds());
    }
}

TEST(Molecule, numHydrogenBondAcceptors)
{
    const auto& session = IndigoSession::create();
    {
        const auto& molecule = session->loadMolecule(METHANE);
        EXPECT_EQ(0, molecule.numHydrogenBondAcceptors());
    }
    {
        const auto& molecule = session->loadMolecule(CAFFEINE);
        EXPECT_EQ(6, molecule.numHydrogenBondAcceptors());
    }
    {
        const auto& molecule = session->loadMolecule(SULFASALAZINE);
        EXPECT_EQ(9, molecule.numHydrogenBondAcceptors());
    }
}

TEST(Molecule, numHydrogenBondDonors)
{
    const auto& session = IndigoSession::create();
    {
        const auto& molecule = session->loadMolecule(CAFFEINE);
        EXPECT_EQ(0, molecule.numHydrogenBondDonors());
    }
    {
        const auto& molecule = session->loadMolecule(SULFASALAZINE);
        EXPECT_EQ(3, molecule.numHydrogenBondDonors());
    }
}
