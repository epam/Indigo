#pragma once

#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoChemicalEntity : public IndigoObject
    {
    public:
        IndigoChemicalEntity(int id, const IndigoSession& indigo);

        void aromatize() const;

        void dearomatize() const;

        void layout() const;

        void clean2d() const;

        std::string smiles() const;

        std::string cml() const;

        std::string inchi() const;

        virtual std::string ctfile() const = 0;
    };
} // namespace indigo_cpp
