#pragma once

#include "IndigoSession.h"

#include <string>

namespace indigo_cpp
{
    template <typename target_t>
    class BingoResult
    {
    public:
        BingoResult(int id, IndigoSessionPtr session);

        int getId() const;

        double getSimilarityValue() const;

        target_t getTarget();

    private:
        int id;
        IndigoSessionPtr session;
    };

    using BingoMoleculeResult = BingoResult<IndigoMolecule>;
}
