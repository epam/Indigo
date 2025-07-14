#pragma once

#include "ket_document.h"
#include "molecule.h"
#include <vector>

namespace indigo
{
    // Perform monomer expansion layout on the document
    void indigoExpand(KetDocument& mol);
}