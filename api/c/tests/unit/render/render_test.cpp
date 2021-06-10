#include "gtest/gtest.h"

#include <base_cpp/exception.h>

#include <indigo-renderer.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

TEST(IndigoRenderTest, render_superatoms)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    indigoSetOption("render-coloring", "true");
    indigoSetOption("render-stereo-style", "none");
    indigoSetOptionXY("render-image-size", 400, 400);
    indigoSetOption("render-output-format", "png");
    indigoSetOption("render-superatom-mode", "collapse");

    try
    {
        int m = indigoLoadMoleculeFromFile(dataPath("molecules/sgroups/abbr.mol").c_str());
        ASSERT_STREQ("CC1C=CC=CC=1", indigoCanonicalSmiles(m));
        int buf = indigoWriteBuffer();
        indigoRender(m, buf);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
