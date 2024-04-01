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
#include <IndigoIterator.h>
#include <IndigoMolecule.h>
#include <IndigoQueryMolecule.h>
#include <IndigoSession.h>

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

TEST(Basic, LoadQueryMolecule)
{
    auto session = IndigoSession::create();
    const auto& m_1 = session->loadQueryMolecule("* |$Q_e$|");
    const auto& m_2 = session->loadQueryMolecule("* |$QH_e$|");
    const auto& m_3 = session->loadQueryMolecule("* |$M_e$|");
    const auto& m_4 = session->loadQueryMolecule("* |$MH_e$|");
    const auto& m_5 = session->loadQueryMolecule("* |$X_e$|");
    const auto& m_6 = session->loadQueryMolecule("* |$XH_e$|");
}

TEST(Basic, MolecularWeight)
{
    auto session = IndigoSession::create();
    {
        const auto plainAtom = session->loadMolecule("[C]");
        ASSERT_FLOAT_EQ(plainAtom.molecularWeight(), 12.0107f);
    }
    {
        const auto correctIsotope = session->loadMolecule("[13C]");
        ASSERT_FLOAT_EQ(correctIsotope.molecularWeight(), 13.003355f);
    }
    {
        const auto incorrectIsotope = session->loadMolecule("[60C]");
        ASSERT_THROW(
            {
                try
                {
                    incorrectIsotope.molecularWeight();
                }
                catch (const IndigoException& e)
                {
                    ASSERT_STREQ(e.what(), "element: getRelativeIsotopicMass: isotope (C, 60) not found");
                    throw;
                }
            },
            IndigoException);
    }
}

TEST(Basic, LoadMoleculeFromFile)
{
    auto session = IndigoSession::create();
    ASSERT_NO_THROW(const auto& m = session->loadMoleculeFromFile(dataPath("molecules/affine/2-bromobenzenethiol-rot.mol")));
}
