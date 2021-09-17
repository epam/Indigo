#include <gtest/gtest.h>

#include <IndigoMolecule.h>
#include <IndigoRenderer.h>
#include <IndigoSession.h>

using namespace indigo_cpp;

TEST(RenderingBasic, Basic)
{
    const auto& session = IndigoSession();
    const auto& renderer = IndigoRenderer(session);
    const auto m = session.loadMolecule("C");
    const auto& result = renderer.svg(m);
    ASSERT_TRUE(result.rfind("<?xml version=\"1.0\" encoding=\"UTF-8\"?>", 0) == 0);
}
