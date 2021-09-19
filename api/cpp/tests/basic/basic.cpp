#include <gtest/gtest.h>

#include <IndigoMolecule.h>
#include <IndigoQueryMolecule.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

TEST(Basic, SingleSessionIdle)
{
    const auto& session = IndigoSession();
}

TEST(Basic, TwoSessionIdle)
{
    const auto& session_1 = IndigoSession();
    const auto& session_2 = IndigoSession();
}

TEST(Basic, Molfile)
{
    const auto& session = IndigoSession();
    const auto& m = session.loadMolecule("C");
    const auto& molfile = m.molfile();

    ASSERT_TRUE(molfile.rfind("M  END") != -1);
}

// TODO: This causes a memory leak that could be catched by Valgrind
TEST(Basic, LoadQueryMolecule)
{
    const auto& session = IndigoSession();
    const auto& m_1 = session.loadQueryMolecule("* |$Q_e$|");
}
