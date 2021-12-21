#pragma once

namespace indigo_cpp
{
    enum class IndigoSimilarityMetric
    {
        TANIMOTO,
        TVERSKY,
        EUCLID
    };

    const char* to_string(IndigoSimilarityMetric m);
}
