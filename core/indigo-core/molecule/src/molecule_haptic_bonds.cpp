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

#include "molecule/molecule_haptic_bonds.h"

#include "molecule/molecule_attachment_groups.h"

using namespace indigo;

IMPL_ERROR(MoleculeHapticBonds, "molecule haptic bonds");

// ---------------------------------------------------------------------------
// HapticBond::Endpoint
// ---------------------------------------------------------------------------

HapticBond::Endpoint HapticBond::Endpoint::atom(int idx)
{
    Endpoint endpoint;
    endpoint._is_group = false;
    endpoint._index = idx;
    return endpoint;
}

HapticBond::Endpoint HapticBond::Endpoint::group(int idx)
{
    Endpoint endpoint;
    endpoint._is_group = true;
    endpoint._index = idx;
    return endpoint;
}

// ---------------------------------------------------------------------------
// HapticBond
// ---------------------------------------------------------------------------

HapticBond::HapticBond()
{
    _reset();
}

HapticBond::~HapticBond()
{
}

void HapticBond::_reset()
{
    _begin = Endpoint();
    _end = Endpoint();
    _type = -1;
}

void HapticBond::reuse()
{
    _reset();
}

void HapticBond::reuse(Endpoint begin, Endpoint end, int type)
{
    _begin = begin;
    _end = end;
    _type = type;
}

bool HapticBond::referencesGroup(int group_idx) const
{
    return (_begin.isGroup() && _begin.index() == group_idx) || (_end.isGroup() && _end.index() == group_idx);
}

static bool remapEndpoint(HapticBond::Endpoint& endpoint, const Array<int>& mapping, bool groups)
{
    if (endpoint.isGroup() != groups)
        return true;

    const int idx = endpoint.index();
    const int mapped = idx >= 0 && idx < mapping.size() ? mapping[idx] : -1;
    if (mapped < 0)
        return false;

    endpoint = groups ? HapticBond::Endpoint::group(mapped) : HapticBond::Endpoint::atom(mapped);
    return true;
}

bool HapticBond::remapAtoms(const Array<int>& atom_mapping)
{
    Endpoint begin = _begin;
    Endpoint end = _end;

    if (!remapEndpoint(begin, atom_mapping, false) || !remapEndpoint(end, atom_mapping, false))
        return false;

    _begin = begin;
    _end = end;
    return true;
}

bool HapticBond::remapGroups(const Array<int>& group_mapping)
{
    Endpoint begin = _begin;
    Endpoint end = _end;

    if (!remapEndpoint(begin, group_mapping, true) || !remapEndpoint(end, group_mapping, true))
        return false;

    _begin = begin;
    _end = end;
    return true;
}

// ---------------------------------------------------------------------------
// MoleculeHapticBonds
// ---------------------------------------------------------------------------

MoleculeHapticBonds::MoleculeHapticBonds()
{
}

MoleculeHapticBonds::~MoleculeHapticBonds()
{
}

int MoleculeHapticBonds::add(HapticBond::Endpoint begin, HapticBond::Endpoint end, int type)
{
    return _bonds.add(begin, end, type);
}

void MoleculeHapticBonds::_checkBond(int idx) const
{
    if (!has(idx))
        throw Error("haptic bond %d does not exist", idx);
}

HapticBond& MoleculeHapticBonds::at(int idx)
{
    _checkBond(idx);
    return _bonds[idx];
}

const HapticBond& MoleculeHapticBonds::at(int idx) const
{
    _checkBond(idx);
    return _bonds[idx];
}

bool MoleculeHapticBonds::has(int idx) const
{
    return _bonds.hasElement(idx);
}

int MoleculeHapticBonds::count() const
{
    return _bonds.size();
}

void MoleculeHapticBonds::remove(int idx)
{
    _checkBond(idx);
    _bonds.remove(idx);
}

int MoleculeHapticBonds::begin() const
{
    return _bonds.begin();
}

int MoleculeHapticBonds::next(int idx) const
{
    return _bonds.next(idx);
}

int MoleculeHapticBonds::end() const
{
    return _bonds.end();
}

void MoleculeHapticBonds::_collectEndpointVertices(const HapticBond::Endpoint& endpoint, const MoleculeAttachmentGroups& groups, std::vector<int>& vertices)
{
    if (!endpoint.isGroup())
    {
        vertices.push_back(endpoint.index());
        return;
    }

    if (!groups.hasGroup(endpoint.index()))
        return;

    const std::vector<int>& atoms = groups.group(endpoint.index()).atoms();
    vertices.insert(vertices.end(), atoms.begin(), atoms.end());
}

void MoleculeHapticBonds::collectVertices(int idx, const MoleculeAttachmentGroups& groups, std::vector<int>& vertices) const
{
    const HapticBond& bond = at(idx);

    vertices.clear();
    _collectEndpointVertices(bond.begin(), groups, vertices);
    _collectEndpointVertices(bond.end(), groups, vertices);
}

int MoleculeHapticBonds::firstAtom(int idx, const MoleculeAttachmentGroups& groups) const
{
    std::vector<int> vertices;
    _collectEndpointVertices(at(idx).begin(), groups, vertices);
    return vertices.empty() ? -1 : vertices.front();
}

int MoleculeHapticBonds::lastAtom(int idx, const MoleculeAttachmentGroups& groups) const
{
    std::vector<int> vertices;
    _collectEndpointVertices(at(idx).end(), groups, vertices);
    return vertices.empty() ? -1 : vertices.back();
}

void MoleculeHapticBonds::onAtomsRemoved(const Array<int>& atom_mapping)
{
    for (int i = begin(); i != end(); i = next(i))
        if (!_bonds[i].remapAtoms(atom_mapping))
            remove(i);
}

void MoleculeHapticBonds::onGroupRemoved(int group_idx)
{
    for (int i = begin(); i != end(); i = next(i))
        if (_bonds[i].referencesGroup(group_idx))
            remove(i);
}

void MoleculeHapticBonds::mergeWithSubmolecule(const MoleculeHapticBonds& other, const Array<int>& atom_mapping, const Array<int>& group_mapping)
{
    // The end is taken once: the bonds added below must not be re-read as sources
    // when a molecule is merged with itself.
    const int last = other.end();

    for (int i = other.begin(); i < last; i = other.next(i))
    {
        const HapticBond& source = other.at(i);

        const int idx = add(source.begin(), source.end(), source.type());
        if (!_bonds[idx].remapAtoms(atom_mapping) || !_bonds[idx].remapGroups(group_mapping))
            remove(idx);
    }
}

void MoleculeHapticBonds::collectConnectivitySets(const MoleculeAttachmentGroups& groups, std::list<std::unordered_set<int>>& neighbors) const
{
    std::vector<int> vertices;

    for (int i = begin(); i != end(); i = next(i))
    {
        collectVertices(i, groups, vertices);
        if (vertices.size() < 2)
            continue;

        neighbors.push_back(std::unordered_set<int>(vertices.begin(), vertices.end()));
    }
}

void MoleculeHapticBonds::clear()
{
    _bonds.clear();
}

bool MoleculeHapticBonds::isEmpty() const
{
    return count() == 0;
}
