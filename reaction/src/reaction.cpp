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

#include "reaction/reaction.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "reaction/reaction_automapper.h"

using namespace indigo;

IMPL_ERROR(Reaction, "reaction");

Reaction::Reaction()
{
}

Reaction::~Reaction ()
{
}

void Reaction::clear ()
{
   BaseReaction::clear();
}

Molecule & Reaction::getMolecule (int index)
{
   return getBaseMolecule(index).asMolecule();
}

int Reaction::_addBaseMolecule (int side)
{
   int idx = _allMolecules.add(new Molecule());
   _addedBaseMolecule(idx, side, *_allMolecules[idx]);
   return idx;
}

void Reaction::saveBondOrders (Reaction& reaction, ObjArray< Array<int> > &bond_types) {

   while (bond_types.size() < reaction.end())
      bond_types.push();

   int i;

   for (i = reaction.begin(); i != reaction.end(); i = reaction.next(i))  {
      Molecule::saveBondOrders(reaction.getMolecule(i), bond_types[i]);
   }
}

void Reaction::loadBondOrders (Reaction& reaction, ObjArray< Array<int> > &bond_types) {

   int i;

   for (i = reaction.begin(); i != reaction.end(); i = reaction.next(i))  {
      Molecule::loadBondOrders(reaction.getMolecule(i), bond_types[i]);
   }
}

bool Reaction::aromatize(const AromaticityOptions &options) {
   bool arom_found = false;
   for (int i = begin(); i < end(); i = next(i)) {
      arom_found |= MoleculeAromatizer::aromatizeBonds(*(Molecule *)_allMolecules[i], options);
   }
   return arom_found;
}

bool Reaction::dearomatize(const AromaticityOptions &options) {
   bool all_dearomatized = true;
   for (int i = begin(); i < end(); i = next(i)) {
      all_dearomatized &= MoleculeDearomatizer::dearomatizeMolecule(*(Molecule *)_allMolecules[i], options);
   }
   return all_dearomatized;
}

Reaction & Reaction::asReaction ()
{
   return *this;
}

BaseReaction * Reaction::neu ()
{
   return new Reaction();
}

void Reaction::checkForConsistency (Reaction &rxn)
{
   int i;

   for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
      Molecule::checkForConsistency(rxn.getMolecule(i));
}

void Reaction::unfoldHydrogens ()
{
   QS_DEF(Array<int>, markers);
   int i, j;

   for (i = begin(); i != end(); i = next(i))
   {
      Molecule &mol = getMolecule(i);
      mol.unfoldHydrogens(&markers, -1);
      _atomAtomMapping[i].expand(markers.size());
      _inversionNumbers[i].expand(markers.size());
      for (j = mol.vertexBegin(); j != mol.vertexEnd(); j = mol.vertexNext(j))
         if (markers[j])
         {
            _atomAtomMapping[i][j] = 0;
            _inversionNumbers[i][j] = 0;
            int edge_idx = mol.getVertex(j).neiEdge(mol.getVertex(j).neiBegin());
            _reactingCenters[i].expand(edge_idx + 1);
            _reactingCenters[i][edge_idx] = 0;
         }
   }
}
