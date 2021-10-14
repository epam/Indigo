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

#include <indigo-inchi.h>

#include "common.h"

using namespace indigo;

TEST(IndigoInChITest, basic)
{
    const auto qword id = indigoAllocSessionId();
    indigoInchiInit();
    indigoSetErrorHandler(errorHandling, nullptr);

    const char* inchi = "InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2";
    const auto m = indigoInchiLoadMolecule(inchi);
    ASSERT_EQ(strcmp(indigoCanonicalSmiles(m), "NC1CC2CC(N)C(O)CC2CC1O"), 0);
    const char* res_inchi = indigoInchiGetInchi(m);
    ASSERT_EQ(strcmp(res_inchi, inchi), 0);
    indigoInchiDispose();
    indigoReleaseSessionId(id);
}
