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

#include "molecule/molecule_components.h"

#include <list>
#include <unordered_set>

#include "graph/graph_decomposer.h"
#include "molecule/base_molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeComponents, "molecule components");

void MoleculeComponents::build(const BaseMolecule& mol)
{
    std::list<std::unordered_set<int>> haptic_sets;
    mol.haptic_bonds.collectConnectivitySets(mol.attachment_groups, haptic_sets);

    GraphDecomposer decomposer(mol);
    const int component_count = decomposer.decompose(nullptr, nullptr, &haptic_sets);

    _component_of.clear_resize(mol.vertexEnd());
    _component_of.fffill();
    for (int atom = mol.vertexBegin(); atom != mol.vertexEnd(); atom = mol.vertexNext(atom))
        _component_of[atom] = decomposer.getComponent(atom);

    _atom_count.clear_resize(component_count);
    _bond_count.clear_resize(component_count);
    for (int component = 0; component < component_count; component++)
    {
        _atom_count[component] = decomposer.getComponentVerticesCount(component);
        _bond_count[component] = decomposer.getComponentEdgesCount(component);
    }
}

int MoleculeComponents::count() const
{
    return _atom_count.size();
}

int MoleculeComponents::componentOf(int atom) const
{
    if (atom < 0 || atom >= _component_of.size() || _component_of[atom] < 0)
        throw Error("atom %d is not in the molecule", atom);
    return _component_of[atom];
}

int MoleculeComponents::atomCount(int component) const
{
    _checkComponent(component);
    return _atom_count[component];
}

int MoleculeComponents::bondCount(int component) const
{
    _checkComponent(component);
    return _bond_count[component];
}

bool MoleculeComponents::isLoneAtom(int atom) const
{
    return _atom_count[componentOf(atom)] == 1;
}

void MoleculeComponents::collectAtoms(int component, Array<int>& atoms) const
{
    _checkComponent(component);
    atoms.clear();
    for (int atom = 0; atom < _component_of.size(); atom++)
        if (_component_of[atom] == component)
            atoms.push(atom);
}

void MoleculeComponents::_checkComponent(int component) const
{
    if (component < 0 || component >= count())
        throw Error("component %d is out of range: the component count is %d", component, count());
}
