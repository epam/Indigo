/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

using namespace indigo;

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
   fragments.clear();
   
   PtrPool<BaseMolecule> &frags = other.fragments;
   for (int i = frags.begin(); i != frags.end(); i = frags.next(i))
   {
      AutoPtr<BaseMolecule> new_fragment(frags[i]->neu());

      new_fragment->clone(*frags[i], 0, 0);
      fragments.add(new_fragment.release());
   }
}

bool RGroup::occurrenceSatisfied (int value)
{
   for (int i = 0; i < occurrence.size(); i++)
      if (value >= (occurrence[i] >> 16) && value <= (occurrence[i] & 0xFFFF))
         return true;
   return false;
}

IMPL_ERROR(MoleculeRGroups, "molecule rgroups");

MoleculeRGroups::MoleculeRGroups ()
{
}

MoleculeRGroups::~MoleculeRGroups ()
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

void MoleculeRGroups::clear ()
{
   _rgroups.clear();
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

