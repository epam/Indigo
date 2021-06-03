#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <molecule/elements.h>

#include <indigo.h>

#include "common.h"

using namespace std;
using namespace indigo;

TEST(IndigoSerializeIsotopesTest, basic)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandling, nullptr);

    const auto m = indigoLoadMoleculeFromString("C");
    const auto atom = indigoGetAtom(m, 0);
    byte* buffer = nullptr;
    int size = 0;
    for (auto isotope = 0; isotope < 300; isotope++)
    {
        try
        {
            indigoSetIsotope(atom, isotope);
            indigoSerialize(m, &buffer, &size);
        }
        catch (Exception& e)
        {
            ASSERT_EQ(isotope, 168);
            ASSERT_STREQ("CMF saver: unexpected C isotope: 168", e.message());
            break;
        }
    }

    indigoReleaseSessionId(session);
}
