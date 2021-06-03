#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <molecule/elements.h>

#include <indigo.h>

#include "common.h"

using namespace std;
using namespace indigo;

TEST(IndigoSerializeIsotopesTest, basic)
{
    const auto m = indigoLoadMoleculeFromString("C");
    const auto atom = indigoGetAtom(m, 0);
    byte* buffer = nullptr;
    int size = 0;
    const auto& element = Element::_instance();
    int result;
    for (auto isotope = 0; isotope < 300; isotope++)
    {
        indigoSetIsotope(atom, isotope);
        result = indigoSerialize(m, &buffer, &size);
        if (result == -1)
        {
            ASSERT_EQ(isotope, 168);
            break;
        }
    }
}
