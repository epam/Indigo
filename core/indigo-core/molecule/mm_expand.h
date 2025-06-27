#pragma once

#include "molecule.h"
#include "ket_document.h"
#include <vector>

namespace indigo {
    // Perform monomer expansion layout on the document
    void indigoExpand(KetDocument& mol, const std::vector<int>& expansions);
}