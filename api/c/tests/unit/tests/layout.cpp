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

#include <thread>

#include <gtest/gtest.h>

#include <base_cpp/exception.h>

#include <algorithm>
#include <fstream>
#include <indigo-renderer.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

class IndigoApiLayoutTest : public IndigoApiTest
{
protected:
    void SetUp() final
    {
        IndigoApiTest::SetUp();
        indigoRendererInit(session);
    }

    void TearDown() final
    {
        indigoRendererDispose(session);
        IndigoApiTest::TearDown();
    }
};

TEST_F(IndigoApiLayoutTest, one_reactant_one_product)
{
    indigoSetErrorHandler(errorHandler, nullptr);

    indigoSetOption("render-coloring", "true");
    indigoSetOption("render-stereo-style", "none");
    indigoSetOptionXY("render-image-size", 400, 400);
    indigoSetOption("render-output-format", "png");

    indigoSetOptionBool("json-saving-pretty", true);
    try
    {
        auto rc = indigoLoadReactionFromFile(dataPath("molecules/basic/before_layout.ket").c_str());

        indigoLayout(rc);

        const char* res = indigoJson(rc);
        //        printf("res=%s", res);
        std::ifstream is(dataPath("molecules/basic/after_layout.ket"), std::ios::binary | std::ios::ate);
        auto size = is.tellg();
        std::string str(size, '\0'); // construct string to stream size
        is.seekg(0);
        is.read(&str[0], size);
        str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());

        ASSERT_STREQ(res, str.c_str());
        //        indigoSaveJsonToFile(rc, "res_after_layout.ket");

        indigoFree(rc);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
