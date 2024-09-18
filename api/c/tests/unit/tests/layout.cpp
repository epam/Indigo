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

    struct TestCaseResult
    {
        std::string_view result;
        std::string_view expected;
    };

    TestCaseResult applyLayoutAndGetResult(int reactionId, std::string_view expectedResultFilename)
    {
        indigoSetOptionBool("json-saving-pretty", true);
        indigoSetOptionBool("json-use-native-precision", true);
        indigoSetOptionBool("json-saving-add-stereo-desc", true);

        indigoLayout(reactionId);
        std::string path_to_file = "molecules/basic/" + std::string(expectedResultFilename);
        // indigoSaveJsonToFile(reactionId, dataPath(path_to_file.data()).c_str());
        std::ifstream is(dataPath(path_to_file.data()), std::ios::binary | std::ios::ate);
        auto size = is.tellg();
        stringBuffer = std::string(size, '\0'); // construct string to stream size
        is.seekg(0);
        is.read(&stringBuffer[0], size);
        stringBuffer.erase(std::remove(stringBuffer.begin(), stringBuffer.end(), '\r'), stringBuffer.end());

        const char* res = indigoJson(reactionId);
        return {res, stringBuffer};
    }
    std::string stringBuffer;
};

// TEST_F(IndigoApiLayoutTest, check_reaction_margin_size)
// {
//     indigoSetErrorHandler(errorHandler, nullptr);
//     indigoSetOptionBool("json-saving-pretty", true);
//     indigoSetOptionBool("json-use-native-precision", true);
//     indigoSetOptionBool("json-saving-add-stereo-desc", true);
//     try
//     {
//         auto reactionId = indigoLoadReactionFromFile(dataPath("molecules/basic/before_layout.ket").c_str());
//         {
//             indigoSetOption("reaction-component-margin-size", "0.0");
//             auto files = applyLayoutAndGetResult(reactionId, "after_layout_zero_margin.ket");
//             EXPECT_STREQ(files.result.data(), files.expected.data());
//         }
//         {
//             indigoSetOption("bond-length", "40.0");
//             indigoSetOption("reaction-component-margin-size", "20.0");
//             auto files = applyLayoutAndGetResult(reactionId, "after_layout_default_margin.ket");
//             EXPECT_STREQ(files.result.data(), files.expected.data());
//         }
//         indigoFree(reactionId);
//     }
//     catch (Exception& e)
//     {
//         ASSERT_STREQ("", e.message());
//     }
// }
