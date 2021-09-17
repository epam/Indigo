#include <gtest/gtest.h>

#include <IndigoMolecule.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

TEST(Basic, Molfile)
{
    const auto& session = IndigoSession();
    const auto m = session.loadMolecule("C");
    std::cout << m.molfile();
}

TEST(Basic, Session)
{
    const auto& session = IndigoSession();
}
