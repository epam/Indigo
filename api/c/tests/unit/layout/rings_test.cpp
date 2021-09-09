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
