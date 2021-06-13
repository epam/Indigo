#include <functional>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

TEST(IndigoBasicApiTest, test_exact_match)
{

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandling, 0);

    int mol = indigoLoadMoleculeFromFile(dataPath("molecules/other/39004.1src.mol").c_str());

    byte* buf;
    int size;
    indigoSerialize(mol, &buf, &size);
    ArrayChar buffer;
    buffer.copy((const char*)buf, size);
    int mol2 = indigoUnserialize((const byte*)buffer.ptr(), buffer.size());

    int res = indigoExactMatch(mol, mol2, "");

    ASSERT_NE(0, res);
}
