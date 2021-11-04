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

#include <molecule/molecule_mass.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

class IndigoSerializeTest : public IndigoApiTest
{
};

TEST_F(IndigoSerializeTest, isotopes_basic)
{
    const auto m = indigoLoadMoleculeFromString("C");
    const auto atom = indigoGetAtom(m, 0);
    byte* buffer = nullptr;
    int size = 0;
    for (auto isotope = 0; isotope < 300; isotope++)
    {
        try
        {
            indigoSetIsotope(atom, isotope);
            indigoSerialize(m, &buffer, &size);
        }
        catch (const Exception& e)
        {
            ASSERT_EQ(isotope, 168);
            ASSERT_STREQ("CMF saver: unexpected C isotope: 168", e.message());
            break;
        }
    }
}
