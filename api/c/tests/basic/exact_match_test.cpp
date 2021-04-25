#include "indigo.h"
#include "gtest/gtest.h"

#include <functional>

#include "base_cpp/profiling.h"

using namespace indigo;

static void errorHandling(const char* message, void* context)
{
    throw Exception(message);
}

TEST(IndigoBasicApiTest, test_exact_match)
{

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandling, 0);

    int mol = indigoLoadMoleculeFromFile("../../molfiles/39004.1src.mol");

    byte* buf;
    int size;
    indigoSerialize(mol, &buf, &size);
    Array<char> buffer;
    buffer.copy((const char*)buf, size);
    int mol2 = indigoUnserialize((const byte*)buffer.ptr(), buffer.size());

    int res = indigoExactMatch(mol, mol2, "");

    ASSERT_NE(0, res);
}