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
#include <unordered_set>

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
    _atoms.clear(); // the buffer is kept: that is the point of a non-destructive reset
    _anchor_atom = -1;
}

void AttachmentGroup::reuse()
{
    _reset();
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
    // Linear in the list length. The list comes from a file, so the obvious
    // addAtom()-per-element loop would be quadratic in it.
    std::unordered_set<int> seen;
    seen.reserve(atoms.size());

    _atoms.clear();
    _atoms.reserve(atoms.size());
    for (int atom : atoms)
        if (seen.insert(atom).second)
            _atoms.push_back(atom);
}

bool AttachmentGroup::remapAtoms(const Array<int>& atom_mapping)
{
    const auto mapped = [&atom_mapping](int atom) { return atom >= 0 && atom < atom_mapping.size() ? atom_mapping[atom] : -1; };

    for (int atom : _atoms)
        if (mapped(atom) < 0)
            return false;

    for (int& atom : _atoms)
        atom = mapped(atom);

    return true;
}

void AttachmentGroup::remapAnchorAtom(const Array<int>& atom_mapping)
{
    if (_anchor_atom < 0)
        return;

    _anchor_atom = _anchor_atom < atom_mapping.size() ? atom_mapping[_anchor_atom] : -1;
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

int MoleculeAttachmentGroups::addGroup()
{
    return _groups.add();
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

void MoleculeAttachmentGroups::onAtomsRemoved(const Array<int>& atom_mapping, Array<int>& removed_groups)
{
    removed_groups.clear();

    for (int i = begin(); i != end(); i = next(i))
        if (_groups[i].remapAtoms(atom_mapping))
            _groups[i].remapAnchorAtom(atom_mapping);
        else
        {
            removeGroup(i);
            removed_groups.push(i);
        }
}

void MoleculeAttachmentGroups::clear()
{
    _groups.clear();
}

bool MoleculeAttachmentGroups::isEmpty() const
{
    return groupCount() == 0;
}

void MoleculeAttachmentGroups::mergeWithSubmolecule(const MoleculeAttachmentGroups& other, const Array<int>& atom_mapping, Array<int>& group_mapping)
{
    // Snapshot first: a merge with itself puts copies into freed slots below end(),
    // and the walk would re-read them as sources.
    const int last = other.end();
    std::vector<int> sources;
    for (int i = other.begin(); i < last; i = other.next(i))
        sources.push_back(i);

    group_mapping.clear_resize(last);
    group_mapping.fill(-1);

    for (int i : sources)
    {
        const AttachmentGroup& source = other.group(i);
        if (source.atoms().empty())
            continue;

        const int idx = addGroup();
        AttachmentGroup& copy = group(idx);
        copy.setAtoms(source.atoms());
        copy.setAnchorAtom(source.anchorAtom());

        if (copy.remapAtoms(atom_mapping))
        {
            copy.remapAnchorAtom(atom_mapping);
            group_mapping[i] = idx;
        }
        else
            removeGroup(idx);
    }
}
