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

#include <base_cpp/output.h>
#include <reaction/reaction.h>

#include "common.h"

using namespace std;
using namespace indigo;

class IndigoCoreReactionTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreReactionTest, pseudoatoms)
{
    Reaction reaction;
    loadReaction("*>>* |$Carbon;$|", reaction);
    ASSERT_STREQ("*>>* |$Carbon;A$|", saveReaction(reaction).c_str());
}

TEST_F(IndigoCoreReactionTest, aliases)
{
    Reaction reaction;
    loadReaction("C>>N |$Carbon;$|", reaction);
    ASSERT_STREQ("C>>N |$Carbon;$|", saveReaction(reaction).c_str());
}

TEST_F(IndigoCoreReactionTest, aliases_complex)
{
    QueryReaction reaction;
    loadQueryReaction("[#6:1]=[#6:2][#6:3].[#6:4]=[#6:5][#6:6]>>[#6:3][#6:2]=[#6:5][#6:6] |$;;R1;;;R2;R1;;;R2$|", reaction);
    reaction.clearAAM();
    ASSERT_STREQ("[#6]=[#6]-[#6].[#6]=[#6]-[#6]>>[#6]-[#6]=[#6]-[#6] |$;;R1;;;R2;R1;;;R2$|", saveQueryReaction(reaction, true).c_str());
}
