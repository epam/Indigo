#pragma once

#include "IndigoChemicalEntity.h"

namespace indigo_cpp
{
    class IndigoMolecule : public IndigoChemicalEntity
    {
    public:
        IndigoMolecule(int id, const IndigoSession& indigo);
        std::string molfile() const;
        std::string ctfile() const override;
    };
}
