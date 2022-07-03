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

#include <indigo-renderer.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

class IndigoApiRendererTest : public IndigoApiTest
{
protected:
    void SetUp() final
    {
        IndigoApiTest::SetUp();
        indigoRendererInit();
    }

    void TearDown() final
    {
        indigoRendererDispose();
        IndigoApiTest::TearDown();
    }

    static void testRender()
    {
        qword _session = indigoAllocSessionId();
        indigoSetSessionId(_session);
        indigoRendererInit();
        indigoSetErrorHandler(errorHandler, nullptr);

        indigoSetOption("render-coloring", "true");
        indigoSetOption("render-stereo-style", "none");
        indigoSetOptionXY("render-image-size", 400, 400);
        indigoSetOption("render-output-format", "png");
        indigoSetOption("render-superatom-mode", "collapse");

        try
        {
            int m = indigoLoadMoleculeFromString("CC1C=CC=CC=1");
            int buf = indigoWriteBuffer();
            indigoRender(m, buf);
            indigoFree(buf);
            indigoFree(m);
        }
        catch (std::exception& e)
        {
            ASSERT_STREQ("", e.what());
        }

        indigoRendererDispose();
        indigoReleaseSessionId(_session);
    }
};

TEST_F(IndigoApiRendererTest, layout_rings)
{
    try
    {
        int m = indigoLoadMoleculeFromString("C1CCCCCCCCCCCCCC1");

        indigoSetOption("smart-layout", "1");

        indigoLayout(m);

        indigoSetOption("render-coloring", "true");
        indigoSetOption("render-stereo-style", "none");
        indigoSetOptionXY("render-image-size", 400, 400);
        indigoSetOption("render-output-format", "png");
        indigoSetOption("render-superatom-mode", "collapse");
        indigoRenderToFile(m, "ring.png");
    }
    catch (std::exception& e)
    {
        ASSERT_STREQ("", e.what());
    }
}

TEST_F(IndigoApiRendererTest, layout_crown)
{
    try
    {
        int m = indigoLoadMoleculeFromString("C1OCCOCCOCCOCCOCCOCCOCCOCCOCCOC1");

        indigoSetOption("smart-layout", "1");

        indigoLayout(m);

        indigoSetOption("render-coloring", "true");
        indigoSetOption("render-stereo-style", "none");
        indigoSetOptionXY("render-image-size", 400, 400);
        indigoSetOption("render-output-format", "png");
        indigoSetOption("render-superatom-mode", "collapse");
        indigoRenderToFile(m, "crown.png");
    }
    catch (std::exception& e)
    {
        ASSERT_STREQ("", e.what());
    }
}

TEST_F(IndigoApiRendererTest, render_superatoms)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < 16; i++)
    {
        threads.emplace_back(testRender);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}
