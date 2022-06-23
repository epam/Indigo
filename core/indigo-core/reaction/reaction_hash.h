#pragma once

#include "base_c/defs.h"

namespace indigo
{
    class Reaction;

    class ReactionHash
    {
    public:
        static dword calculate(Reaction& mol);
    };
}
