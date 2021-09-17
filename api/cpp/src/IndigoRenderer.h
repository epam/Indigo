#pragma once

#include <vector>

#include "IndigoSession.h"

namespace indigo_cpp
{
    class IndigoChemicalEntity;

    class IndigoRenderer
    {
    public:
        IndigoRenderer(const IndigoSession& session);
        ~IndigoRenderer();

        IndigoRenderer(IndigoRenderer const&) = delete;
        void operator=(IndigoRenderer const&) = delete;

        std::string svg(const IndigoChemicalEntity& data) const;
        std::vector<char> png(const IndigoChemicalEntity& data) const;

    private:
        const IndigoSession& _session;
    };
} // namespace indigo_cpp
