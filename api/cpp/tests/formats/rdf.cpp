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
#include <IndigoReaction.h>
#include <IndigoSession.h>

#include "common.h"

using namespace indigo_cpp;

TEST(SDF, IterateRDFile)
{
    auto session = IndigoSession::create();
    auto counter = 0;
    std::vector<IndigoReactionPtr> reactions;
    reactions.reserve(9);
    for (const auto& reaction : session->iterateRDFile(dataPath("reactions/basic/basic.rdf")))
    {
        ++counter;
        reactions.emplace_back(reaction);
    }
    EXPECT_EQ(counter, 9);
    EXPECT_EQ(reactions.size(), 9);
}

TEST(SDF, Rxn3000Name)
{
    auto session = IndigoSession::create();
    for (const auto& reaction : session->iterateRDFile(dataPath("reactions/basic/basic.rdf")))
    {
        ASSERT_NO_THROW(reaction->name());
    }
}
