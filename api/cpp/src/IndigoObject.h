#pragma once

#include "IndigoSession.h"

namespace indigo_cpp
{
    class IndigoObject
    {
    public:
        const int id;
        const IndigoSession& indigo;

        IndigoObject(int id, const IndigoSession& indigo);
        virtual ~IndigoObject();
    };
}
