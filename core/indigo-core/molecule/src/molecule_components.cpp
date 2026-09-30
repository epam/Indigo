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

#include "graph/filter.h"
#include "graph/graph_decomposer.h"
#include "molecule/base_molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeComponents, "molecule components");

MoleculeComponents::MoleculeComponents(BaseMolecule& mol)
{
    std::list<std::unordered_set<int>> haptic_sets;
    mol.haptic_bonds.collectConnectivitySets(mol.attachment_groups, haptic_sets);

    GraphDecomposer decomposer(mol);
    const int component_count = decomposer.decompose(nullptr, nullptr, &haptic_sets);

    _component_of.clear_resize(mol.vertexEnd());
    _component_of.fffill();
    _atom_count.clear_resize(component_count);
    _atom_count.zerofill();
    _bond_count.clear_resize(component_count);
    _bond_count.zerofill();

    // Counted here rather than taken from the decomposer: with external neighbours it
    // counts every pair of a set as an edge, 40 bonds for a ferrocene instead of 10.
    for (int atom : mol.vertices())
    {
        const int component = decomposer.getComponent(atom);
        _component_of[atom] = component;
        _atom_count[component]++;
    }
    for (int bond : mol.edges())
        _bond_count[_component_of[mol.getEdge(bond).beg]]++;
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

void MoleculeComponents::selectAtoms(int component, Filter& filter) const
{
    _checkComponent(component);
    filter.init(_component_of.ptr(), Filter::EQ, component);
}

void MoleculeComponents::_checkComponent(int component) const
{
    if (component < 0 || component >= count())
        throw Error("component %d does not exist: the molecule has %d components", component, count());
}
