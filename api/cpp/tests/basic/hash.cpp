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

#include <IndigoIterator.h>
#include <IndigoMolecule.h>
#include <IndigoReaction.h>
#include <IndigoSession.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Hash, Molecule)
{
    const auto& session = IndigoSession::create();
    const auto& m1 = session->loadMolecule("C");
    const auto& m2 = session->loadMolecule("CC");
    ASSERT_NE(m1.hash(), m2.hash());
}

TEST(Hash, Reaction)
{
    const auto& session = IndigoSession::create();
    const auto& r1 = session->loadReaction("C.N.C>>CNC");
    const auto& r2 = session->loadReaction("C.C>>CC");
    ASSERT_NE(r1.hash(), r2.hash());
}
