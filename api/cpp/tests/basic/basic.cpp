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

TEST(Basic, SingleSession)
{
    const auto& session = IndigoSession();
}

TEST(Basic, TwoSessions)
{
    const auto& session_1 = IndigoSession();
    const auto& session_2 = IndigoSession();
}
