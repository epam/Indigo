#pragma once

#include "base_c/defs.h"

namespace indigo
{
    class Molecule;

    class MoleculeHash
    {
    public:
        static dword calculate(Molecule& mol);
    };
}
