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

#include "molecule/molecule_attachment_groups.h"

#include <algorithm>

using namespace indigo;

IMPL_ERROR(MoleculeAttachmentGroups, "molecule attachment groups");

// ---------------------------------------------------------------------------
// AttachmentGroup
// ---------------------------------------------------------------------------

AttachmentGroup::AttachmentGroup()
{
    _reset();
}

AttachmentGroup::~AttachmentGroup()
{
}

void AttachmentGroup::_reset()
{
    _atoms.clear(); // the buffers are kept: that is the point of a non-destructive reset
    _bonds.clear();
    _mode = AttachmentMode::All;
    _charge = 0;
    _radical = 0;
}

void AttachmentGroup::reuse()
{
    _reset();
}

void AttachmentGroup::reuse(AttachmentMode mode)
{
    reuse();
    _mode = mode;
}

bool AttachmentGroup::hasAtom(int atom) const
{
    return std::find(_atoms.begin(), _atoms.end(), atom) != _atoms.end();
}

void AttachmentGroup::addAtom(int atom)
{
    if (!hasAtom(atom))
        _atoms.push_back(atom);
}

void AttachmentGroup::setAtoms(const std::vector<int>& atoms)
{
    _atoms.clear();
    for (int atom : atoms)
        addAtom(atom);
}

void AttachmentGroup::addBond(int atom, int order)
{
    _bonds.push_back(Bond{atom, order});
}

bool AttachmentGroup::remapAtoms(const Array<int>& atom_mapping)
{
    const auto mapped = [&atom_mapping](int atom) { return atom >= 0 && atom < atom_mapping.size() ? atom_mapping[atom] : -1; };

    for (int atom : _atoms)
        if (mapped(atom) < 0)
            return false;

    for (int& atom : _atoms)
        atom = mapped(atom);

    _bonds.erase(std::remove_if(_bonds.begin(), _bonds.end(), [&mapped](const Bond& bond) { return mapped(bond.atom) < 0; }), _bonds.end());
    for (Bond& bond : _bonds)
        bond.atom = mapped(bond.atom);

    return true;
}

// ---------------------------------------------------------------------------
// MoleculeAttachmentGroups
// ---------------------------------------------------------------------------

MoleculeAttachmentGroups::MoleculeAttachmentGroups()
{
}

MoleculeAttachmentGroups::~MoleculeAttachmentGroups()
{
}

int MoleculeAttachmentGroups::addGroup(AttachmentMode mode)
{
    return _groups.add(mode);
}

void MoleculeAttachmentGroups::_checkGroup(int idx) const
{
    if (!hasGroup(idx))
        throw Error("attachment group %d does not exist", idx);
}

AttachmentGroup& MoleculeAttachmentGroups::group(int idx)
{
    _checkGroup(idx);
    return _groups[idx];
}

const AttachmentGroup& MoleculeAttachmentGroups::group(int idx) const
{
    _checkGroup(idx);
    return _groups[idx];
}

bool MoleculeAttachmentGroups::hasGroup(int idx) const
{
    return _groups.hasElement(idx);
}

int MoleculeAttachmentGroups::groupCount() const
{
    return _groups.size();
}

void MoleculeAttachmentGroups::removeGroup(int idx)
{
    _checkGroup(idx);
    _groups.remove(idx);
}

int MoleculeAttachmentGroups::begin() const
{
    return _groups.begin();
}

int MoleculeAttachmentGroups::next(int idx) const
{
    return _groups.next(idx);
}

int MoleculeAttachmentGroups::end() const
{
    return _groups.end();
}

void MoleculeAttachmentGroups::onAtomsRemoved(const Array<int>& atom_mapping)
{
    for (int i = begin(); i != end(); i = next(i))
        if (!_groups[i].remapAtoms(atom_mapping))
            removeGroup(i);
}

void MoleculeAttachmentGroups::setBondHaptic(int bond_idx, bool haptic)
{
    if (haptic)
        _haptic_bonds.insert(bond_idx);
    else
        _haptic_bonds.erase(bond_idx);
}

bool MoleculeAttachmentGroups::isBondHaptic(int bond_idx) const
{
    return _haptic_bonds.count(bond_idx) != 0;
}

int MoleculeAttachmentGroups::hapticBondCount() const
{
    return static_cast<int>(_haptic_bonds.size());
}

void MoleculeAttachmentGroups::onBondRemoved(int bond_idx)
{
    _haptic_bonds.erase(bond_idx);
}

void MoleculeAttachmentGroups::clear()
{
    _groups.clear();
    _haptic_bonds.clear();
}

bool MoleculeAttachmentGroups::isEmpty() const
{
    return groupCount() == 0 && _haptic_bonds.empty();
}

void MoleculeAttachmentGroups::mergeWithSubmolecule(const MoleculeAttachmentGroups& other, const Array<int>& atom_mapping, const Array<int>& bond_mapping)
{
    // The end is taken once: the groups added below must not be re-read as sources
    // when a molecule is merged with itself.
    const int last = other.end();

    for (int i = other.begin(); i < last; i = other.next(i))
    {
        const AttachmentGroup& source = other.group(i);
        if (source.atoms().empty())
            continue;

        const int idx = addGroup(source.mode());
        AttachmentGroup& copy = group(idx);
        copy.setAtoms(source.atoms());
        copy.setCharge(source.charge());
        copy.setRadical(source.radical());
        for (const AttachmentGroup::Bond& bond : source.bonds())
            copy.addBond(bond.atom, bond.order);

        if (!copy.remapAtoms(atom_mapping))
            removeGroup(idx);
    }

    for (int bond_idx : other._haptic_bonds)
        if (bond_idx >= 0 && bond_idx < bond_mapping.size() && bond_mapping[bond_idx] >= 0)
            setBondHaptic(bond_mapping[bond_idx]);
}

void MoleculeAttachmentGroups::collectConnectivitySets(std::list<std::unordered_set<int>>& neighbors) const
{
    for (int i = begin(); i != end(); i = next(i))
    {
        const AttachmentGroup& ag = group(i);
        if (ag.atoms().empty())
            continue;

        std::unordered_set<int> connected(ag.atoms().begin(), ag.atoms().end());
        for (const AttachmentGroup::Bond& bond : ag.bonds())
            connected.insert(bond.atom);
        neighbors.push_back(std::move(connected));
    }
}
