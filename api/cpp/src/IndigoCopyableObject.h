#pragma once

#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoCopyableObject : public IndigoObject
    {
    protected:
        IndigoCopyableObject(int id, IndigoSessionPtr session);
        IndigoCopyableObject(IndigoCopyableObject&&) = default;
        IndigoCopyableObject& operator=(IndigoCopyableObject&&) = default;
        IndigoCopyableObject(const IndigoCopyableObject&);
        IndigoCopyableObject& operator=(const IndigoCopyableObject&);
        ~IndigoCopyableObject() override = default;

        static int _cloneAndReturnId(const IndigoObject& other);
    };
}
