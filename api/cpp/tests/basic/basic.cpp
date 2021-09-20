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
#include <IndigoQueryMolecule.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

TEST(Basic, SingleSessionIdle)
{
    const auto& session = IndigoSession();
}

TEST(Basic, TwoSessionIdle)
{
    const auto& session_1 = IndigoSession();
    const auto& session_2 = IndigoSession();
}

TEST(Basic, Molfile)
{
    const auto& session = IndigoSession();
    const auto& m = session.loadMolecule("C");
    const auto& molfile = m.molfile();

    ASSERT_TRUE(molfile.rfind("M  END") != -1);
}

// TODO: This causes a memory leak that could be catched by Valgrind
TEST(Basic, LoadQueryMolecule)
{
    const auto& session = IndigoSession();
    const auto& m_1 = session.loadQueryMolecule("* |$Q_e$|");
}
