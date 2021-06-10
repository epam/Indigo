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

#ifndef __molecule_inchi_utils_h__
#define __molecule_inchi_utils_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

    class Molecule;

    // Utility class for InChI code creation
    class MoleculeInChIUtils
    {
    public:
        // Helpful structure with mappings
        struct Mapping
        {
            Mapping(const Array<int>& _mapping, const Array<int>& _inv_mapping) : mapping(_mapping), inv_mapping(_inv_mapping)
            {
            }

            const Array<int>&mapping, &inv_mapping;
        };

        // Returns indices for lexicographically-sorted atom labels
        // with exception that the first atom is Carbon
        static const Array<int>& getLexSortedAtomLables();
        // Returns inverse permutation for getLexSortedLables
        static const Array<int>& getLexSortedAtomLablesRanks();

        // Stable sort for small integer arrays with possibility to use array with ranks
        // Note: it is better to add stable sort method in Array and
        // modify qsort (and other stable sort) to accept any comparators.
        static void stableSmallSort(Array<int>& indices, const Array<int>* ranks);

        // Compare atoms with hydrogens: C < CH4 < CH3 < CH2 < CH
        static int compareHydrogens(int hyd1, int hyd2);

        // Get parity according to InChI standart
        static int getParityInChI(Molecule& mol, int bond);

        DECL_ERROR;

    private:
        static void _ensureLabelsInitialized();
        static void _initializeAtomLabels();

        static int _compareAtomLabels(int& label1, int& label2, void* context);

        static Array<int> _atom_lables_sorted;
        static Array<int> _atom_lables_ranks;
    };

} // namespace indigo

#endif // __molecule_inchi_utils_h__
