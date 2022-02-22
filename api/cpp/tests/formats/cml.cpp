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
#include <IndigoSession.h>

#include "common.h"

using namespace indigo_cpp;

namespace
{
    constexpr auto C_CML = R""""(<?xml version="1.0" encoding="UTF-8"?>
<cml>
    <molecule>
        <atomArray>
            <atom id="a0" elementType="C"/>
        </atomArray>
    </molecule>
</cml>
)"""";
}

TEST(CML, MoleculeSave)
{
    const auto& session = IndigoSession::create();
    auto molecule = session->loadMolecule("C");
    ASSERT_STREQ(molecule.cml().c_str(), C_CML);
}

TEST(CML, MoleculeLoad)
{
    const auto& session = IndigoSession::create();
    auto molecule = session->loadMolecule(C_CML);
    ASSERT_STREQ(molecule.smiles().c_str(), "C");
}
