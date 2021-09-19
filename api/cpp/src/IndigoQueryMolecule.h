#pragma once

#include "IndigoBaseMolecule.h"

namespace indigo_cpp
{
    class IndigoQueryMolecule : public IndigoBaseMolecule
    {
    public:
        IndigoQueryMolecule(int id, const IndigoSession& indigo);
    };
}
