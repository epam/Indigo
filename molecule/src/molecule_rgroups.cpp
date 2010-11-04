/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "molecule/molecule_rgroups.h"
#include "base_cpp/auto_ptr.h"
#include "molecule/query_molecule.h"


RGroup::RGroup () : if_then(0), rest_h(0)
{
}

RGroup::~RGroup ()
{
}

void RGroup::clear()
{
   if_then = 0;
   rest_h = 0;
   occurrence.clear();
   fragments.clear();
}

void RGroup::copy (RGroup &other)
{
   if_then = other.if_then;
   rest_h = other.rest_h;
   occurrence.copy(other.occurrence);
   for (int i = 0; i < other.fragments.size(); i++)
   {
      AutoPtr<QueryMolecule> new_fragment(new QueryMolecule());

      new_fragment->clone(*other.fragments[i], 0, 0);
      fragments.add(new_fragment.release());
   }
}

bool RGroup::occurrenceSatisfied (int value) const
{
   for (int i = 0; i < occurrence.size(); i++)
      if (value >= (occurrence[i] >> 16) && value <= (occurrence[i] & 0xFFFF))
         return true;
   return false;
}

MoleculeRGroups::MoleculeRGroups ()
{
}

void MoleculeRGroups::copyRGroupsFromMolecule (MoleculeRGroups &other)
{
   int n_rgroups = other.getRGroupCount();

   for (int i = 1; i <= n_rgroups; i++)
   {
      RGroup &rgroup = other.getRGroup(i);

      if (rgroup.fragments.size() > 0)
         getRGroup(i).copy(rgroup);
   }
}

void MoleculeRGroups::copySitesFromMolecule (MoleculeRGroups &other, const int *mapping)
{
   for (int i = other.begin(); i < other.end(); i = other.next(i))
   {
      int atom_idx = mapping[other._rgroup_atoms.key(i)];

      if (atom_idx == -1)
         continue;

      _Atom &other_atom = other._rgroup_atoms.value(i);
      _Atom &atom = _rgroup_atoms.insert(atom_idx);

      atom.attachment_order.resize(other_atom.attachment_order.size());

      for (int j = 0; j < atom.attachment_order.size(); j++)
         atom.attachment_order[j] = mapping[other_atom.attachment_order[j]];
   }
}

void MoleculeRGroups::initRGroupAtom (int atom_idx)
{
   _rgroup_atoms.insert(atom_idx);
}

void MoleculeRGroups::setAttachmentOrder (int atom_idx, int order, int att_atom_idx)
{
   _Atom *atom = _rgroup_atoms.at2(atom_idx);

   if (atom == 0)
   {
      _Atom &new_atom = _rgroup_atoms.insert(atom_idx);

      new_atom.attachment_order.resize(order + 1);
      new_atom.attachment_order.fffill();
      new_atom.attachment_order[order] = att_atom_idx;
   } else {
      atom->attachment_order.expand(order + 1);
      atom->attachment_order[order] = att_atom_idx;
   }
}

void MoleculeRGroups::removeSites (const Array<int> &indices)
{
   for (int i = 0; i < indices.size(); i++)
      if (_rgroup_atoms.find(indices[i]))
         _rgroup_atoms.remove(indices[i]);
}

void MoleculeRGroups::clear ()
{
   int i;

   for (i = _rgroup_atoms.begin(); i != _rgroup_atoms.end();
        i = _rgroup_atoms.next(i))
   {
      _rgroup_atoms.value(i).attachment_order.clear();
   }

   _rgroups.clear();
   _rgroup_atoms.clear();
}

bool MoleculeRGroups::isRGroupAtom (int atom_idx) const
{
   return _rgroup_atoms.at2(atom_idx) != 0;
}

int MoleculeRGroups::getAttachmentOrder (int idx, int order) const
{
   _Atom &atom = _rgroup_atoms.at(idx);

   if (order < atom.attachment_order.size())
      return atom.attachment_order[order];

   return -1;
}

RGroup & MoleculeRGroups::getRGroup (int idx)
{
   if (_rgroups.size() < idx)
      _rgroups.resize(idx);

   return _rgroups[idx - 1];
}

int MoleculeRGroups::getRGroupCount () const
{
   return _rgroups.size();
}

int MoleculeRGroups::begin () const
{
   return _rgroup_atoms.begin();
}

int MoleculeRGroups::end () const
{
   return _rgroup_atoms.end();
}

int MoleculeRGroups::next (int i) const
{
   return _rgroup_atoms.next(i);
}

int MoleculeRGroups::count() const
{
   return _rgroup_atoms.size();
}

int MoleculeRGroups::atomIdx (int site) const
{
   return _rgroup_atoms.key(site);
}

void MoleculeRGroupFragment::addAttachmentPoint (int order, int index)
{
   if (_attachment_index.size() <= order)
      _attachment_index.resize(order + 1);

   _attachment_index[order].push(index);
}

void MoleculeRGroupFragment::removeAttachmentPoint (int index)
{
   int i, j;

   for (i = 0; i < _attachment_index.size(); i++)
      if ((j = _attachment_index[i].find(index)) != -1)
      {
         if (j == _attachment_index[i].size() - 1)
            _attachment_index[i].pop();
         else
            _attachment_index[i][j] = _attachment_index[i].pop();
      }
}
