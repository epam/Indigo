#pragma once

#include "IndigoBaseMolecule.h"

namespace indigo_cpp
{
    class IndigoMolecule : public IndigoBaseMolecule
    {
    public:
        IndigoMolecule(int id, const IndigoSession& indigo);
    };
}
