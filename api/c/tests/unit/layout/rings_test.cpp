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

#include <base_cpp/exception.h>

#include <indigo-renderer.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

TEST(IndigoLayoutTest, layout_rings)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoRendererInit();
    indigoSetErrorHandler(errorHandling, 0);

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
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }

    indigoRendererDispose();
    indigoReleaseSessionId(session);
}

TEST(IndigoLayoutTest, layout_crown)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoRendererInit();

    indigoSetErrorHandler(errorHandling, 0);

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
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }

    indigoRendererDispose();
    indigoReleaseSessionId(session);
}
