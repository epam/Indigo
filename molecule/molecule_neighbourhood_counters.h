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

#ifndef __molecule_neighbourhood_counters_h__
#define __molecule_neighbourhood_counters_h__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"

namespace indigo
{

    class BaseMolecule;
    class Molecule;
    class QueryMolecule;

    class MoleculeAtomNeighbourhoodCounters
    {
    public:
        void calculate(Molecule& mol);
        void calculate(QueryMolecule& mol);

        bool testSubstructure(const MoleculeAtomNeighbourhoodCounters& target_counters, int query_atom_idx, int target_atom_idx, bool use_bond_types) const;

        void makeTranspositionForSubstructure(BaseMolecule& mol, Array<int>& output) const;

    public:
        void _calculate(BaseMolecule& mol, bool is_query);
        void _calculateLevel0(BaseMolecule& mol, bool is_query);
        void _calculateNextLevel(BaseMolecule& mol, int r);

        bool _isAtomInformationStored(int atom_idx) const;

        static int _countersCmp(int& i1, int& i2, void* context);

        struct CountersPerRadius
        {
            // Number of atoms for specified radius:
            int C_cnt;       // carbon
            int hetero_cnt;  // heteroatoms, except N and O
            int heteroN_cnt; // nitrogen
            int heteroO_cnt; // oxigen
            int in_ring_cnt; // in rings
            int trip_cnt;    // with >= 3 bonds of any type

            int degree_sum; // sum of bonds order

            bool testSubstructure(const CountersPerRadius& target, bool use_bond_types) const;
        };
        struct Counters
        {
            enum
            {
                RADIUS = 2
            };
            CountersPerRadius per_rad[RADIUS];

            bool testSubstructure(const Counters& target, bool use_bond_types) const;
        };

        struct Context
        {
            const MoleculeAtomNeighbourhoodCounters* cnt;
            BaseMolecule* mol;
        };

        Array<Counters> _per_atom_counters;
        Array<int> _use_atom;
    };

} // namespace indigo

#endif // __molecule_neighbourhood_counters_h__
