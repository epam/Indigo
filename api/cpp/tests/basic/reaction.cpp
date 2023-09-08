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

#include <IndigoReaction.h>

#include "common.h"

using namespace indigo_cpp;

TEST(Reaction, automapReactionSmarts)
{
    const auto& session = IndigoSession::create();
    auto reaction = session->loadReaction("[C:1]=[C:2][C:3].[C:4]=[C:5][C:6]>>[C:3][C:2]=[C:5][C:6]");
    reaction.automap(IndigoAutomapMode::CLEAR);
    ASSERT_STREQ("[#6]=[#6]-[#6].[#6]=[#6]-[#6]>>[#6]-[#6]=[#6]-[#6] |^3:0,3,^1:1,4,7,8|", reaction.smarts().c_str());
}

TEST(Reaction, automapQueryReactionSmarts)
{
    const auto& session = IndigoSession::create();
    auto reaction = session->loadQueryReaction("[C:1]=[C:2][C:3].[C:4]=[C:5][C:6]>>[C:3][C:2]=[C:5][C:6] |$;;R1;;;R2;R1;;;R2$|");
    reaction.automap(IndigoAutomapMode::CLEAR);
    ASSERT_STREQ("[#6]=[#6]-[#6].[#6]=[#6]-[#6]>>[#6]-[#6]=[#6]-[#6] |$;;R1;;;R2;R1;;;R2$|", reaction.smarts().c_str());
}
