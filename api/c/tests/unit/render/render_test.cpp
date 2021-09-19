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

#include "gtest/gtest.h"

#include <base_cpp/exception.h>

#include <indigo-renderer.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

namespace
{
    void testRender()
    {
        qword session = indigoAllocSessionId();
        indigoSetSessionId(session);
        indigoRendererInit();

        indigoSetErrorHandler(errorHandling, 0);

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
        catch (Exception& e)
        {
            ASSERT_STREQ("", e.message());
        }

        indigoRendererDispose();
        indigoReleaseSessionId(session);
    }
}

TEST(IndigoRenderTest, render_superatoms)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < 1000; i++)
    {
        threads.emplace_back(testRender);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}
