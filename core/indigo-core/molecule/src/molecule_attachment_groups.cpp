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

AttachmentGroup::AttachmentGroup() : _mode(AttachmentMode::All), _charge(0), _radical(0)
{
}

AttachmentGroup::~AttachmentGroup()
{
}

void AttachmentGroup::reuse()
{
    // Must mirror the constructor field by field: a missed field would leak
    // stale data into a recycled group. clear() keeps the buffers, which is the
    // point of a non-destructive reset.
    _atoms.clear();
    _bonds.clear();
    _mode = AttachmentMode::All;
    _charge = 0;
    _radical = 0;
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
    // Membership is a set: a duplicate would make the group claim an atom
    // twice and skew the derived centre.
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

void AttachmentGroup::removeBondsToAtom(int atom)
{
    _bonds.erase(std::remove_if(_bonds.begin(), _bonds.end(), [atom](const Bond& bond) { return bond.atom == atom; }), _bonds.end());
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

AttachmentGroup& MoleculeAttachmentGroups::group(int idx)
{
    if (!hasGroup(idx))
        throw Error("attachment group %d does not exist", idx);
    return _groups[idx];
}

const AttachmentGroup& MoleculeAttachmentGroups::group(int idx) const
{
    if (!hasGroup(idx))
        throw Error("attachment group %d does not exist", idx);
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
    if (!hasGroup(idx))
        throw Error("attachment group %d does not exist", idx);
    // The bonds go with it: they are stored inside the group, so there is
    // nothing left to dangle.
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
    for (int i = other.begin(); i != other.end(); i = other.next(i))
    {
        const AttachmentGroup& source = other.group(i);

        // All-or-nothing: a haptic bond addresses every atom of its group, so a
        // group that survives only in part would assert something the original
        // structure never said.
        bool complete = !source.atoms().empty();
        for (int atom : source.atoms())
        {
            if (atom < 0 || atom >= atom_mapping.size() || atom_mapping[atom] < 0)
            {
                complete = false;
                break;
            }
        }
        if (!complete)
            continue;

        AttachmentGroup& copy = group(addGroup(source.mode()));
        for (int atom : source.atoms())
            copy.addAtom(atom_mapping[atom]);
        copy.setCharge(source.charge());
        copy.setRadical(source.radical());

        for (const AttachmentGroup::Bond& bond : source.bonds())
            if (bond.atom >= 0 && bond.atom < atom_mapping.size() && atom_mapping[bond.atom] >= 0)
                copy.addBond(atom_mapping[bond.atom], bond.order);
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
