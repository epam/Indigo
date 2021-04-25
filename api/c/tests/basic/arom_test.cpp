#include <fstream>

#include "gtest/gtest.h"

#include "base_cpp/exception.h"
#include "base_cpp/scanner.h"
#include "indigo.h"
#include "molecule/molecule.h"
#include "molecule/molecule_auto_loader.h"
#include "src/indigo_internal.h"

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

TEST(IndigoAromTest, arom_test_merge)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int m = indigoLoadMoleculeFromString("c1ccccc1.c1ccccc1");
        int c = indigoComponent(m, 0);
        int cc = indigoClone(c);
        indigoDearomatize(cc);
        Array<int> vertices;
        for (int i = 0; i < 6; ++i)
            vertices.push(i);

        indigoRemoveAtoms(m, vertices.size(), vertices.ptr());
        //      printf("%s\n", indigoSmiles(cc));

        indigoMerge(m, cc);
        ASSERT_STREQ("C1C=CC=CC=1.c1ccccc1", indigoSmiles(m));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}
