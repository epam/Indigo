#pragma once

#include <stdexcept>

namespace indigo_cpp
{
    class IndigoException : public std::runtime_error
    {
    public:
        explicit IndigoException(const char* message) : std::runtime_error(message)
        {
        }
    };
}
