#ifndef __bond__
#define __bond__

#include <string>
#include <weak_ptr>

#include "../graph_edge/graph_edge.h"

namespace indigo2
{
    enum class BondType
    {
        BOND_ZERO = 0,
        BOND_SINGLE = 1,
        BOND_DOUBLE = 2,
        BOND_TRIPLE = 3,
        BOND_AROMATIC = 4
    };

    class Atom : public virtual GraphVertex // interface
    {
    public:
        GETSETTER(QueryList<BondType>, bond_type) = 0;
    };
} // namespace indigo2

#endif