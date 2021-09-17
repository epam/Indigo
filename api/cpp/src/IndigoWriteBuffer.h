#pragma once

#include <vector>

#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoWriteBuffer : public IndigoObject
    {
    public:
        IndigoWriteBuffer(int, const IndigoSession&);
        std::vector<char> toBuffer() const;
        std::string toString() const;
    };
} // namespace indigo_cpp
