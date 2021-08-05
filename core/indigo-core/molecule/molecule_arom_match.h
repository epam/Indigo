/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __molecule_arom_match_h__
#define __molecule_arom_match_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "molecule/molecule.h"

namespace indigo
{

    class Molecule;
    class QueryMolecule;

    /*
     * Aromaticity matcher handles cases of the query possible aromatic cycles.
     * Some cycles in the query molecule can be aromatic or not depending on the
     * embedding to the target molecule. For example [#7]C1=C~[#6]~[#6]~C=C1O
     * query molecule should match both NC1=CCCC=C1O and Nc1ccccc1O target
     * molecules. Only aromatic bonds can match aromatic bonds, so to match
     * aromatic bond query must have aromatic realization (assignment exact values
     * to the any bond and any atoms that leads cycle to be aromatic).
     */
    class AromaticityMatcher
    {
    public:
        AromaticityMatcher(QueryMolecule& query, BaseMolecule& base, const AromaticityOptions& arom_options);

        // Check if aromaticity matcher is necessary for specified query
        static bool isNecessary(QueryMolecule& query);

        // Update internal structures when query molecule changes (grow)
        void validateQuery();

        // Check if query bond can be aromatic if 'aromatic' is true and
        // nonaromatic otherwise.
        bool canFixQueryBond(int query_edge_idx, bool aromatic);

        // Fix query bond to aromatic or nonaromatic state
        void fixQueryBond(int query_edge_idx, bool aromatic);

        // Unfix query bond (opposite to fixQueryBond)
        void unfixQueryBond(int query_edge_idx);

        // Unfix all neighbour bonds
        void unfixNeighbourQueryBond(int query_arom_idx);

        // Check if embedding is possible. 'core_sub' corresponds
        // to the mapping from the query to the target. Vertices
        // with negative values are ignored. 'core_super' is
        // an inverse mapping for 'core_sub'.
        bool match(int* core_sub, int* core_super);

        DECL_ERROR;

    protected:
        QueryMolecule& _query;
        BaseMolecule& _base;

        AromaticityOptions _arom_options;

        enum
        {
            ANY = 0,
            AROMATIC,
            NONAROMATIC
        };
        CP_DECL;
        TL_CP_DECL(Array<int>, _matching_edges_state);
        std::unique_ptr<BaseMolecule> _submolecule;
    };

} // namespace indigo

#endif
