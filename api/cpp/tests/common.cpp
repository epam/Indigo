#include "common.h"

namespace
{
    const std::string dataPathPrefix = DATA_PATH;
}

using namespace indigo_cpp;

std::string indigo_cpp::dataPath(const char* dataPathSuffix)
{
    return dataPathPrefix + "/" + dataPathSuffix;
}
