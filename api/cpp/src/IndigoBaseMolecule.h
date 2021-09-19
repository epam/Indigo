#pragma once

#include "IndigoChemicalEntity.h"

namespace indigo_cpp
{
    class IndigoBaseMolecule : public IndigoChemicalEntity
    {
    public:
        IndigoBaseMolecule(int id, const IndigoSession& indigo);
        std::string molfile() const;
        std::string ctfile() const override;
    };
}
