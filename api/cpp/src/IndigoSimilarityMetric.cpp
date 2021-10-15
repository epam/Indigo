#include "IndigoSimilarityMetric.h"

#include <stdexcept>

using namespace indigo_cpp;

const char* indigo_cpp::to_string(IndigoSimilarityMetric m)
{
    switch (m)
    {
    case IndigoSimilarityMetric::TANIMOTO:
        return "tanimoto";
    case IndigoSimilarityMetric::TVERSKY:
        return "tversky";
    case IndigoSimilarityMetric::EUCLID:
        return "euclid";
    default:
        throw std::logic_error("Unknown metric");
    }
}
