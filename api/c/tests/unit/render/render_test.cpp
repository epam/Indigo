#include <thread>

#include "gtest/gtest.h"

#include <base_cpp/exception.h>

#include <indigo-renderer.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

#include <iostream>

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
            std::stringstream ss;
            ss << session << ' ' << m << '\n';
            std::cout << ss.str();
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

    for (auto& thread: threads)
    {
        thread.join();
    }
}
