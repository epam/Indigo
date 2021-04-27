#include <gtest/gtest.h>

#include <indigo.h>


TEST(IndigoIdleTest, idle)
{
}

TEST(IndigoIdleTest, alloc_set_release_session)
{
    const auto session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoReleaseSessionId(session);
}
