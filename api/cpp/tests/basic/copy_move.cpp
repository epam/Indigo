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

TEST(CopyMove, Load)
{
    auto session = IndigoSession::create();
    auto m = session->loadMolecule("C1C=CC=CC=1");
    EXPECT_EQ(m.canonicalSmiles(), "C1C=CC=CC=1");
}

TEST(CopyMove, LoadReference)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C1C=CC=CC=1");
    auto& m2 = m1;
    m2.aromatize();
    EXPECT_EQ(m1.canonicalSmiles(), "c1ccccc1");
    EXPECT_EQ(m2.id(), m1.id());
}

TEST(CopyMove, LoadMoveConstructor)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C1C=CC=CC=1");
    const auto m1_id = m1.id();
    IndigoMolecule m2(std::move(m1));
    EXPECT_EQ(m2.canonicalSmiles(), "C1C=CC=CC=1");
    EXPECT_EQ(m2.id(), m1_id);
}

TEST(CopyMove, LoadMoveAssignment)
{
    auto session = IndigoSession::create();
    auto m1 = session->loadMolecule("C1C=CC=CC=1");
    const auto m1_id = m1.id();
    auto m2 = std::move(m1);
    EXPECT_EQ(m2.canonicalSmiles(), "C1C=CC=CC=1");
    EXPECT_EQ(m2.id(), m1_id);
}
//
//TEST(Molecule, LoadCopyConstructor)
//{
//    auto session = IndigoSession::create();
//    auto m1 = session->loadMolecule("C1C=CC=CC=1");
//    IndigoMolecule m2(m1);
//    m2.aromatize();
//    EXPECT_EQ(m1.canonicalSmiles(), "C1C=CC=CC=1");
//    EXPECT_EQ(m2.canonicalSmiles(), "c1ccccc1");
//    EXPECT_NE(m2.id(), m1.id());
//
//}
//
//TEST(Molecule, LoadCopyAssignment)
//{
//    auto session = IndigoSession::create();
//    auto m1 = session->loadMolecule("C1C=CC=CC=1");
//    auto m2 = m1;
//    m2.aromatize();
//    EXPECT_EQ(m1.canonicalSmiles(), "C1C=CC=CC=1");
//    EXPECT_EQ(m2.canonicalSmiles(), "c1ccccc1");
//    EXPECT_NE(m2.id(), m1.id());
//}
