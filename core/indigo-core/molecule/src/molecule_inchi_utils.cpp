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

#include "molecule/molecule_inchi_utils.h"

#include "base_cpp/os_sync_wrapper.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cis_trans.h"

using namespace indigo;

//
// Sorted atoms lables
//
Array<int> MoleculeInChIUtils::_atom_lables_sorted;
Array<int> MoleculeInChIUtils::_atom_lables_ranks;

IMPL_ERROR(MoleculeInChIUtils, "InChI utility");

const Array<int>& MoleculeInChIUtils::getLexSortedAtomLables()
{
    _ensureLabelsInitialized();
    return _atom_lables_sorted;
}

const Array<int>& MoleculeInChIUtils::getLexSortedAtomLablesRanks()
{
    _ensureLabelsInitialized();
    return _atom_lables_ranks;
}

void MoleculeInChIUtils::_ensureLabelsInitialized()
{
    // Double-checked locking
    if (_atom_lables_sorted.size() == 0)
    {
        static std::mutex lock;
        std::lock_guard<std::mutex> locker(lock);
        if (_atom_lables_sorted.size() == 0)
            _initializeAtomLabels();
    }
}

void MoleculeInChIUtils::_initializeAtomLabels()
{
    _atom_lables_sorted.reserve(ELEM_MAX);
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
        _atom_lables_sorted.push(i);
    _atom_lables_sorted.qsort(_compareAtomLabels, NULL);

    _atom_lables_ranks.resize(ELEM_MAX);
    _atom_lables_ranks.fffill();
    for (int i = 0; i < _atom_lables_sorted.size(); i++)
    {
        int label = _atom_lables_sorted[i];
        _atom_lables_ranks[label] = i;
    }
}

int MoleculeInChIUtils::_compareAtomLabels(int& label1, int& label2, void* /*context*/)
{
    // Compare atom labels in alphabetic order with exception that
    // atom C is the first atom and H as second atom
    if (label1 == ELEM_C && label2 != ELEM_C)
        return -1;
    if (label1 != ELEM_C && label2 == ELEM_C)
        return 1;

    return strcmp(Element::toString(label1), Element::toString(label2));
}

//
// Sorting
//

void MoleculeInChIUtils::stableSmallSort(Array<int>& indices, const Array<int>* ranks)
{
    // Stable insersion sort
    for (int i = 1; i < indices.size(); i++)
    {
        int i_value = indices[i];
        int i_value_rank = i_value;
        if (ranks != NULL)
            i_value_rank = ranks->at(i_value);

        int j = i - 1;
        while (j >= 0)
        {
            int j_value_rank = indices[j];
            if (ranks != NULL)
                j_value_rank = ranks->at(j_value_rank);

            if (i_value_rank >= j_value_rank)
                break;

            indices[j + 1] = indices[j];
            j--;
        }
        indices[j + 1] = i_value;
    }
}

//
// Other
//

int MoleculeInChIUtils::compareHydrogens(int hyd1, int hyd2)
{
    if (hyd1 == 0)
        hyd1 = 256;
    if (hyd2 == 0)
        hyd2 = 256;

    return hyd2 - hyd1;
}

int MoleculeInChIUtils::getParityInChI(Molecule& mol, int bond)
{
    if (mol.cis_trans.getParity(bond) == 0)
        throw Error("Specified bond ins't stereogenic");

    const Edge& edge = mol.getEdge(bond);

    const int* subst = mol.cis_trans.getSubstituents(bond);
    // Find substituents with maximal indices
    int max_first = std::max(subst[0], subst[1]);
    int max_second = std::max(subst[2], subst[3]);

    int value = MoleculeCisTrans::sameside(mol.getAtomXyz(edge.beg), mol.getAtomXyz(edge.end), mol.getAtomXyz(max_first), mol.getAtomXyz(max_second));
    if (value > 0)
        return -1;
    return 1;
}
