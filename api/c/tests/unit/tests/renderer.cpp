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
        indigoRendererInit(session);
    }

    void TearDown() final
    {
        indigoRendererDispose(session);
        IndigoApiTest::TearDown();
    }

    static void testRender()
    {
        qword _session = indigoAllocSessionId();
        indigoSetSessionId(_session);
        indigoRendererInit(_session);
        indigoSetErrorHandler(errorHandler, nullptr);

        indigoSetOption("render-coloring", "true");
        indigoSetOption("render-stereo-style", "none");
        indigoSetOptionXY("render-image-size", 400, 400);
        indigoSetOption("render-output-format", "png");

        try
        {
            int m = indigoLoadMoleculeFromString("CC1C=CC=CC=1");
            int buf = indigoWriteBuffer();
            indigoRender(m, buf);
            indigoFree(buf);
            indigoFree(m);
        }
        catch (Exception& e)
        {
            ASSERT_STREQ("", e.message());
        }

        indigoRendererDispose(_session);
        indigoReleaseSessionId(_session);
    }

    std::string renderWithComment(const char* format, const char* font_family = nullptr)
    {
        indigoResetOptions();
        indigoSetOption("render-comment", "Font comment");
        indigoSetOption("render-output-format", format);
        indigoSetOptionXY("render-image-size", 300, 200);
        if (font_family != nullptr)
            indigoSetOption("render-font-family", font_family);

        int m = indigoLoadMoleculeFromString("C");
        int buf = indigoWriteBuffer();
        char* raw = nullptr;
        int size = 0;

        indigoRender(m, buf);
        indigoToBuffer(buf, &raw, &size);

        std::string output(raw, size);

        indigoFree(buf);
        indigoFree(m);

        return output;
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
        indigoRenderToFile(m, "ring.png");
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
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
        indigoRenderToFile(m, "crown.png");
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
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

TEST_F(IndigoApiRendererTest, svg_font_family_option_changes_rendering)
{
    auto default_svg = renderWithComment("svg");
    auto arial_svg = renderWithComment("svg", "Arial");
    auto courier_svg = renderWithComment("svg", "Courier New");

    ASSERT_EQ(default_svg, arial_svg);
    ASSERT_NE(default_svg, courier_svg);
}

TEST_F(IndigoApiRendererTest, png_font_family_option_changes_rendering)
{
    auto default_png = renderWithComment("png");
    auto arial_png = renderWithComment("png", "Arial");
    auto courier_png = renderWithComment("png", "Courier New");

    ASSERT_EQ(default_png, arial_png);
    ASSERT_NE(default_png, courier_png);
}
