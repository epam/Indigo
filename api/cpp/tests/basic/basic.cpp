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
#include <IndigoQueryMolecule.h>
#include <IndigoSession.h>
#include <IndigoSDFileIterator.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Basic, SingleSessionIdle)
{
    const auto& session = IndigoSession::create();
}

TEST(Basic, TwoSessionIdle)
{
    const auto& session_1 = IndigoSession::create();
    const auto& session_2 = IndigoSession::create();
}

TEST(Basic, Molfile)
{
    auto session = IndigoSession::create();
    const auto& m = session->loadMolecule("C");
    const auto& molfile = m.molfile();

    ASSERT_TRUE(molfile.rfind("M  END") != -1);
}

// TODO: This causes a memory leak that could be catched by Valgrind
TEST(Basic, LoadQueryMolecule)
{
    auto session = IndigoSession::create();
    const auto& m_1 = session->loadQueryMolecule("* |$Q_e$|");
}

TEST(Basic, IterateSDFile)
{
    auto session = IndigoSession::create();
    auto counter = 0;
    std::vector<IndigoMoleculeSPtr> molecules;
    molecules.reserve(245);
    for (const auto& molecule : session->iterateSDFile(dataPath("molecules/basic/Compound_0000001_0000250.sdf.gz")))
    {
        ++counter;
        molecules.emplace_back(molecule);
    }
    EXPECT_EQ(counter, 245);
    EXPECT_EQ(molecules.size(), 245);
}

TEST(Basic, DISABLED_IterateSDFilePharmapendium)
{
    auto session = IndigoSession::create();
    auto counter = 0;
    std::vector<IndigoMoleculeSPtr> molecules;
    molecules.reserve(245);
    for (const auto& molecule : session->iterateSDFile(dataPath("molecules/basic/pharmapendium.sdf.gz")))
    {
        try
        {
            ++counter;
            molecules.emplace_back(molecule);
        }
        catch (const IndigoException&)
        {}
    }
    EXPECT_EQ(counter, 3128);
    EXPECT_EQ(molecules.size(), 3128);
}
