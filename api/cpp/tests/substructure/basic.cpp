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
#include <IndigoSubstructureMatcher.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Substructure, BasicTrue)
{
    const auto& session = IndigoSession::create();
    const auto& target = session->loadMolecule("C");
    const auto& matcher = session->substructureMatcher(target);
    const auto& query = session->loadQueryMolecule("C");
    ASSERT_TRUE(matcher.match(query));
}

TEST(Substructure, BasicFalse)
{
    const auto& session = IndigoSession::create();
    const auto& target = session->loadMolecule("C");
    const auto& matcher = session->substructureMatcher(target);
    const auto& query = session->loadQueryMolecule("N");
    ASSERT_FALSE(matcher.match(query));
}

TEST(Substructure, SmartsTrue)
{
    const auto& session = IndigoSession::create();
    const auto& target = session->loadMolecule("Cl");
    const auto& matcher = session->substructureMatcher(target);
    const auto& query = session->loadQueryMolecule("[X1&!#1&!#6]~[X1&!#6]");
    ASSERT_TRUE(matcher.match(query));
}

TEST(Substructure, SmartsFalse)
{
    const auto& session = IndigoSession::create();
    const auto& target = session->loadMolecule("C");
    const auto& matcher = session->substructureMatcher(target);
    const auto& query = session->loadQueryMolecule("[X1&!#1&!#6]~[X1&!#6]");
    ASSERT_FALSE(matcher.match(query));
}
