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

#include "molecule/max_common_submolecule.h"

#include "molecule/molecule.h"
#include "molecule/molecule_exact_matcher.h"

using namespace indigo;

MaxCommonSubmolecule::MaxCommonSubmolecule(BaseMolecule& submol, BaseMolecule& supermol) : MaxCommonSubgraph(submol, supermol)
{
    conditionEdgeWeight = matchBonds;
    conditionVerticesColor = matchAtoms;
}

bool MaxCommonSubmolecule::matchBonds(Graph& g1, Graph& g2, int i, int j, void* userdata)
{
    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;

    int flags = MoleculeExactMatcher::CONDITION_ELECTRONS;

    if (userdata)
        flags = *((int*)userdata);

    if (flags > MoleculeExactMatcher::CONDITION_ALL || flags < 0)
        throw Error("Wrong userdata...need correct flag");

    return MoleculeExactMatcher::matchBonds(mol1, mol2, i, j, flags);
}

bool MaxCommonSubmolecule::matchAtoms(Graph& g1, Graph& g2, const int* core_sub, int i, int j, void* userdata)
{
    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;

    int flags = MoleculeExactMatcher::CONDITION_ELECTRONS;

    if (userdata)
        flags = *((int*)userdata);

    if (flags > MoleculeExactMatcher::CONDITION_ALL || flags < 0)
        throw Error("Wrong userdata...need correct flag");

    return MoleculeExactMatcher::matchAtoms(mol1, mol2, i, j, flags);
}
