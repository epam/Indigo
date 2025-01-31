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

#include <fstream>

#include <gtest/gtest.h>

#include "common.h"

#include <IndigoMolecule.h>
#include <IndigoQueryMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

TEST(RenderingBasic, BasicSVG)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);
    const auto& m = session->loadMolecule("C");
    const auto& result = renderer.svg(m);
    ASSERT_TRUE(result.rfind("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 0) == 0);
}

TEST(RenderingBasic, UTF8)
{
    auto session = IndigoSession::create();
    const auto& renderer = IndigoRenderer(session);
    const auto& m = session->loadMoleculeFromFile(dataPath("molecules/basic/sgroups_utf8.mol"));
    const auto& result = renderer.png(m);
    std::ofstream ff("sgroups_utf8.png", std::ofstream::out);
    for (const auto c : result)
    {
        ff << c;
    }
    ff.close();
}

TEST(RenderingBasic, List)
{
    auto session = IndigoSession::create();
    session->setOption("smart-layout", "1");
    const auto& renderer = IndigoRenderer(session);
    session->setOption("render-background-color", std::string("255, 255, 255"));
    const auto& m = session->loadSmarts("[F,Cl,Br,I]");
    const auto& result = renderer.png(m);
    std::ofstream ff("list.png", std::ios::binary);
    for (const auto c : result)
    {
        ff << c;
    }
    ff.close();
}
