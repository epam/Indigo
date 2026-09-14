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

// Public API for the haptic constructs of #3233: attachment groups and the bonds
// that use one as an end. Both live outside the molecular graph, so neither shows
// up in indigoIterateAtoms/indigoIterateBonds of a molecule - they are reached
// through the iterators here.

#include <vector>

#include "indigo_internal.h"
#include "indigo_molecule.h"

namespace
{
    // The endpoint an object stands for, or an error naming what was passed
    // instead. Keeping the two accepted kinds in one place is what lets
    // indigoAddHapticBond take an atom and a group through the same parameter.
    indigo::HapticBond::Endpoint endpointOf(IndigoObject& obj, indigo::BaseMolecule& mol, const char* who)
    {
        if (IndigoAtom::is(obj))
        {
            IndigoAtom& atom = IndigoAtom::cast(obj);
            if (&atom.mol != &mol)
                throw IndigoError("%s(): the atom belongs to another molecule", who);
            return indigo::HapticBond::Endpoint::atom(atom.idx);
        }

        if (obj.type == IndigoObject::ATTACHMENT_GROUP)
        {
            IndigoAttachmentGroup& group = IndigoAttachmentGroup::cast(obj);
            if (&group.mol != &mol)
                throw IndigoError("%s(): the attachment group belongs to another molecule", who);
            return indigo::HapticBond::Endpoint::group(group.idx);
        }

        throw IndigoError("%s(): %s is neither an atom nor an attachment group", who, obj.debugInfo());
    }

    // Wraps an endpoint back into the object kind it names.
    IndigoObject* objectOf(const indigo::HapticBond::Endpoint& endpoint, indigo::BaseMolecule& mol)
    {
        if (endpoint.isGroup())
            return new IndigoAttachmentGroup(mol, endpoint.index());
        return new IndigoAtom(mol, endpoint.index());
    }

    std::vector<int> checkedAtoms(indigo::BaseMolecule& mol, int natoms, int* atoms, const char* who)
    {
        if (atoms == nullptr || natoms <= 0)
            throw IndigoError("%s(): no atoms were given", who);

        std::vector<int> members;
        members.reserve(natoms);
        for (int i = 0; i < natoms; i++)
        {
            if (atoms[i] < 0 || atoms[i] >= mol.vertexEnd() || !mol.hasVertex(atoms[i]))
                throw IndigoError("%s(): atom index %d out of range", who, atoms[i]);
            members.push_back(atoms[i]);
        }
        return members;
    }
}

IndigoAttachmentGroup::IndigoAttachmentGroup(BaseMolecule& mol_, int idx_) : IndigoObject(ATTACHMENT_GROUP), mol(mol_), idx(idx_)
{
}

IndigoAttachmentGroup::~IndigoAttachmentGroup()
{
}

const char* IndigoAttachmentGroup::debugInfo() const
{
    return "<attachment group>";
}

int IndigoAttachmentGroup::getIndex()
{
    return idx;
}

void IndigoAttachmentGroup::remove()
{
    // Goes through the molecule, not the container: the haptic bonds that address
    // this group have to go with it.
    mol.removeAttachmentGroup(idx);
}

IndigoAttachmentGroup& IndigoAttachmentGroup::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::ATTACHMENT_GROUP)
        return (IndigoAttachmentGroup&)obj;

    throw IndigoError("%s is not an attachment group", obj.debugInfo());
}

AttachmentGroup& IndigoAttachmentGroup::get()
{
    // The group pool reuses freed indices, so an object outliving its group would
    // otherwise read whichever group took the slot next.
    if (!mol.attachment_groups.hasGroup(idx))
        throw IndigoError("attachment group %d does not exist any more", idx);
    return mol.attachment_groups.group(idx);
}

IndigoAttachmentGroupsIter::IndigoAttachmentGroupsIter(BaseMolecule& mol_, Array<int>&& refs)
    : IndigoObject(ATTACHMENT_GROUPS_ITER), _mol(mol_), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoAttachmentGroupsIter::~IndigoAttachmentGroupsIter()
{
}

const char* IndigoAttachmentGroupsIter::debugInfo() const
{
    return "<attachment groups iterator>";
}

bool IndigoAttachmentGroupsIter::hasNext()
{
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoAttachmentGroupsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    return new IndigoAttachmentGroup(_mol, _refs[_idx]);
}

IndigoAttachmentGroupAtomsIter::IndigoAttachmentGroupAtomsIter(BaseMolecule& mol_, int group_idx)
    : IndigoObject(ATTACHMENT_GROUP_ATOMS_ITER), _mol(mol_), _group_idx(group_idx)
{
    // The iterator holds an index, not the group, so an already removed group has
    // to be refused here rather than at the first next().
    _mol.attachment_groups.group(group_idx);
    _idx = -1;
}

IndigoAttachmentGroupAtomsIter::~IndigoAttachmentGroupAtomsIter()
{
}

bool IndigoAttachmentGroupAtomsIter::hasNext()
{
    return _idx + 1 < static_cast<int>(_mol.attachment_groups.group(_group_idx).atoms().size());
}

IndigoObject* IndigoAttachmentGroupAtomsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    return new IndigoAtom(_mol, _mol.attachment_groups.group(_group_idx).atoms()[_idx]);
}

IndigoHapticBond::IndigoHapticBond(BaseMolecule& mol_, int idx_) : IndigoObject(HAPTIC_BOND), mol(mol_), idx(idx_)
{
}

IndigoHapticBond::~IndigoHapticBond()
{
}

const char* IndigoHapticBond::debugInfo() const
{
    return "<haptic bond>";
}

int IndigoHapticBond::getIndex()
{
    return idx;
}

void IndigoHapticBond::remove()
{
    mol.removeHapticBond(idx);
}

IndigoHapticBond& IndigoHapticBond::cast(IndigoObject& obj)
{
    if (obj.type == IndigoObject::HAPTIC_BOND)
        return (IndigoHapticBond&)obj;

    throw IndigoError("%s is not a haptic bond", obj.debugInfo());
}

HapticBond& IndigoHapticBond::get()
{
    if (!mol.haptic_bonds.has(idx))
        throw IndigoError("haptic bond %d does not exist any more", idx);
    return mol.haptic_bonds.at(idx);
}

IndigoHapticBondsIter::IndigoHapticBondsIter(BaseMolecule& mol_, Array<int>&& refs) : IndigoObject(HAPTIC_BONDS_ITER), _mol(mol_), _refs(std::move(refs))
{
    _idx = -1;
}

IndigoHapticBondsIter::~IndigoHapticBondsIter()
{
}

const char* IndigoHapticBondsIter::debugInfo() const
{
    return "<haptic bonds iterator>";
}

bool IndigoHapticBondsIter::hasNext()
{
    return _idx + 1 < _refs.size();
}

IndigoObject* IndigoHapticBondsIter::next()
{
    if (!hasNext())
        return 0;

    _idx++;
    return new IndigoHapticBond(_mol, _refs[_idx]);
}

CEXPORT int indigoAddAttachmentGroup(int molecule, int natoms, int* atoms)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        const std::vector<int> members = checkedAtoms(mol, natoms, atoms, "indigoAddAttachmentGroup");

        const int idx = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(idx).setAtoms(members);
        mol.updateEditRevision();
        return self.addObject(new IndigoAttachmentGroup(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountAttachmentGroups(int molecule)
{
    INDIGO_BEGIN
    {
        return self.getObject(molecule).getBaseMolecule().attachment_groups.groupCount();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateAttachmentGroups(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        Array<int> refs;
        for (int i = mol.attachment_groups.begin(); i != mol.attachment_groups.end(); i = mol.attachment_groups.next(i))
            refs.push(i);
        return self.addObject(new IndigoAttachmentGroupsIter(mol, std::move(refs)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetAttachmentGroup(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (!mol.attachment_groups.hasGroup(index))
            throw IndigoError("indigoGetAttachmentGroup(): no attachment group with index %d", index);
        return self.addObject(new IndigoAttachmentGroup(mol, index));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoSetAttachmentGroupAtoms(int group, int natoms, int* atoms)
{
    INDIGO_BEGIN
    {
        IndigoAttachmentGroup& ag = IndigoAttachmentGroup::cast(self.getObject(group));
        const std::vector<int> members = checkedAtoms(ag.mol, natoms, atoms, "indigoSetAttachmentGroupAtoms");

        ag.mol.setAttachmentGroupAtoms(ag.idx, members);
        return 1;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoAddHapticBond(int molecule, int begin, int end)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        const HapticBond::Endpoint begin_end = endpointOf(self.getObject(begin), mol, "indigoAddHapticBond");
        const HapticBond::Endpoint end_end = endpointOf(self.getObject(end), mol, "indigoAddHapticBond");

        const int idx = mol.addHapticBond(begin_end, end_end);
        return self.addObject(new IndigoHapticBond(mol, idx));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetAttachmentGroupAnchor(int group)
{
    INDIGO_BEGIN
    {
        IndigoAttachmentGroup& ag = IndigoAttachmentGroup::cast(self.getObject(group));
        const int anchor = ag.get().anchorAtom();
        // A group read from KET has no anchor at all, and one whose star was
        // removed keeps working without it - hence 0 rather than an error.
        if (anchor < 0 || !ag.mol.hasVertex(anchor))
            return 0;
        return self.addObject(new IndigoAtom(ag.mol, anchor));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIsAttachmentGroup(int item)
{
    INDIGO_BEGIN
    {
        // The two ends of a haptic bond come back as objects of different kinds,
        // and the caller has to be able to tell them apart without provoking an
        // error on the wrong one.
        return self.getObject(item).type == IndigoObject::ATTACHMENT_GROUP ? 1 : 0;
    }
    INDIGO_END(-1);
}

CEXPORT int indigoCountHapticBonds(int molecule)
{
    INDIGO_BEGIN
    {
        return self.getObject(molecule).getBaseMolecule().haptic_bonds.count();
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateHapticBonds(int molecule)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        Array<int> refs;
        for (int i = mol.haptic_bonds.begin(); i != mol.haptic_bonds.end(); i = mol.haptic_bonds.next(i))
            refs.push(i);
        return self.addObject(new IndigoHapticBondsIter(mol, std::move(refs)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetHapticBond(int molecule, int index)
{
    INDIGO_BEGIN
    {
        BaseMolecule& mol = self.getObject(molecule).getBaseMolecule();
        if (!mol.haptic_bonds.has(index))
            throw IndigoError("indigoGetHapticBond(): no haptic bond with index %d", index);
        return self.addObject(new IndigoHapticBond(mol, index));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetHapticBondBegin(int bond)
{
    INDIGO_BEGIN
    {
        IndigoHapticBond& hb = IndigoHapticBond::cast(self.getObject(bond));
        return self.addObject(objectOf(hb.get().begin(), hb.mol));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoGetHapticBondEnd(int bond)
{
    INDIGO_BEGIN
    {
        IndigoHapticBond& hb = IndigoHapticBond::cast(self.getObject(bond));
        return self.addObject(objectOf(hb.get().end(), hb.mol));
    }
    INDIGO_END(-1);
}

CEXPORT const char* indigoHapticBondType(int bond)
{
    INDIGO_BEGIN
    {
        IndigoHapticBond& hb = IndigoHapticBond::cast(self.getObject(bond));
        // A name, not the internal code: 1009/1010 are an implementation detail of
        // the model and must not become part of the published interface.
        return hb.get().type() == _BOND_VARIABLE_ATTACHMENT ? "variable-attachment" : "haptic";
    }
    INDIGO_END(0);
}
