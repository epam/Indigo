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

#include <IndigoException.h>
#include <IndigoMolecule.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Molecule, Load)
{
    auto session = IndigoSession::create();
    auto m = session->loadMolecule("C");
    EXPECT_EQ(m.smiles(), "C");
}

TEST(Molecule, LoadMoveConstructor)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C");
    IndigoMolecule m2(std::move(m1));
    EXPECT_THROW(m1.smiles(), IndigoException);
    EXPECT_EQ(m2.smiles(), "C");
}

TEST(Molecule, LoadMoveAssignment)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C");
    auto m2 = std::move(m1);
    EXPECT_THROW(m1.smiles(), IndigoException);
    EXPECT_EQ(m2.smiles(), "C");
}

TEST(Molecule, LoadCopyConstructor)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C");
    IndigoMolecule m2(m1);
    EXPECT_EQ(m2.smiles(), "C");
}

TEST(Molecule, LoadCopyAssignment)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C");
    auto m2 = m1;
    EXPECT_EQ(m1.smiles(), "C");
    EXPECT_EQ(m2.smiles(), "C");
}
