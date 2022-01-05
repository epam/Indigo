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

#include <IndigoInChI.h>
#include <IndigoMolecule.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

TEST(InchiBasic, GetInChI)
{
    auto session = IndigoSession::create();
    const auto& inchi = IndigoInChI(session);
    const auto& m = session->loadMolecule("C");
    const auto& result = inchi.getInChI(m);
    ASSERT_EQ(result, "InChI=1S/CH4/h1H4");
}
