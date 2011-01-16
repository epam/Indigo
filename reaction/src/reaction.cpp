/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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
   int idx = _allMolecules.size();
   _allMolecules.add(new Molecule());
   _addedBaseMolecule(idx, side, *_allMolecules[idx]);
   return idx;
}
/*

int Reaction::findMoleculeSideIdx(const Molecule* qmol) const
{
   int mol_idx = findMoleculeIdx(qmol);
   int side_idx = 0;
   int type = _indexes[mol_idx];

   for (int i = begin(); i < mol_idx; i = next(i))
      if(_indexes[i] == type)
         side_idx++;
   return side_idx;
}

int Reaction::findMoleculeIdx(const Molecule* qmol) const {
   for(int i = begin(); i < end(); i = next(i)) {
      if(qmol == &_allMolecules.at(i))
         return i;
   }
   throw Error("cannot find aam number");
}

int Reaction::findInversionNumber(const Molecule* qmol, int atom_number) const {
   for(int i = begin(); i < end(); i = next(i)) {
      if(qmol == &_allMolecules.at(i))
         return getInversion(i, atom_number);
   }
   throw Error("cannot find inversion");
}

int Reaction::findExactChange(const Molecule* qmol, int atom_number) const {
   for(int i = begin(); i < end(); i = next(i)) {
      if(qmol == &_allMolecules.at(i))
         return getExactChange(i, atom_number);
   }
   throw Error("cannot find exact change");
}

void Reaction::dearomatizeBonds() {
   for (int i = begin(); i < end(); i = next(i)) {
      MoleculeDearomatizer::dearomatizeMolecule(_allMolecules[i]);
   }
}

void Reaction::aromatizeQueryBonds() {
   for (int i = begin(); i < end(); i = next(i)) {
      QueryMoleculeAromatizer::aromatizeBonds(_allMolecules[i]);
   }
}


bool Reaction::isAllConnected() const {
   for (int i = begin(); i < end(); i = next(i)) {
      if (!Graph::isConnected(_allMolecules[i]))
         return false;
   }
   return true;
}

*/

/*

int Reaction::sideCount (int side) const { 
   switch (side) {
   case REACTANT:
      return reactantsCount();
   case PRODUCT: 
      return productsCount();
   case CATALYST:
      return catalystCount();
   default:
      throw Error("Unknown side identifier");
   }
}*/

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

bool Reaction::aromatize() {
   bool arom_found = false;
   for (int i = begin(); i < end(); i = next(i)) {
      arom_found |= MoleculeAromatizer::aromatizeBonds(*(Molecule *)_allMolecules[i]);
   }
   return arom_found;
}

bool Reaction::dearomatize() {
   bool all_dearomatized = true;
   for (int i = begin(); i < end(); i = next(i)) {
      all_dearomatized &= MoleculeDearomatizer::dearomatizeMolecule(*(Molecule *)_allMolecules[i]);
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
