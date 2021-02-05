#ifndef __atom__
#define __atom__

#include <string>
#include <weak_ptr>

#include "../graph_vertex/graph_vertex.h"

namespace indigo2
{
    typedef coord_3d_t float[3]; // x,y,z nanometers

    /*
    <T> class Query
    <T> class QueryRange
    <T> class QueryList
    <T> class QueryList

    class ChemElement
    {
        symbol: A
        name: "Any atom"
        number : range
        mass : [12.7 - 28.5]
        isotopes
        GETSETTER(std::string, symbol) = 0; // fixed for normal atoms, can be set for pseudoatoms
    };
    */

    class Atom : public virtual GraphVertex // interface
    {
    public:
        GETSETTER(Query<const ChemElement&>, chem_element) = 0;

        GETSETTER(coord_3d_t, phys_coords) = 0; // physical coordinates (nm) relative the molecule origin (TBD)
        GETSETTER(bool, explicit_valence) = 0;
        GETSETTER(bool, explicit_impl_h) = 0;
        GETSETTER(int, charge) = 0;
        /*etc. atom and query properties */
        bool isQueryAtom();
    };
} // namespace indigo2

#endif