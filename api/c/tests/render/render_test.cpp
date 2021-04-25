#include "base_cpp/exception.h"
#include "base_cpp/scanner.h"
#include "indigo-renderer.h"
#include "indigo.h"
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
        int m = indigoLoadMoleculeFromFile("../../molfiles/sgroups/abbr.mol");
        ASSERT_STREQ("CC1C=CC=CC=1", indigoCanonicalSmiles(m));
        int buf = indigoWriteBuffer();
        indigoRender(m, buf);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
