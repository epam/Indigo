#include "common.h"

#include <indigo.h>

using namespace indigo;

namespace
{
    const std::string dataPathPrefix = DATA_PATH; // NOLINT
}

void IndigoApiTest::SetUp()
{
    session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandler, nullptr);
}

void IndigoApiTest::TearDown()
{
    indigoReleaseSessionId(session);
}

std::string IndigoApiTest::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}

void IndigoApiTest::errorHandler(const char* message, void*) // NOLINT
{
    throw std::runtime_error(message);
}
