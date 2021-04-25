#include "base_cpp/exception.h"
#include "base_cpp/scanner.h"
#include "indigo-renderer.h"
#include "indigo.h"
#include "layout/molecule_layout.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "src/indigo_internal.h"
#include "gtest/gtest.h"
#include <fstream>

using namespace indigo;

static void loadMolecule(const char* buf, Molecule& mol)
{
    BufferScanner scanner(buf);
    MoleculeAutoLoader loader(scanner);
    loader.loadMolecule(mol);
}

static void errorHandling(const char* message, void* context)
{
    throw Exception(message);
}

TEST(IndigoLayoutTest, layout_rings)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

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
}

TEST(IndigoLayoutTest, layout_crown)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

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
}
