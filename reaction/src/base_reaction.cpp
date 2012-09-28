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

#include "reaction/base_reaction.h"
#include "base_cpp/tlscont.h"

using namespace indigo;

IMPL_ERROR(BaseReaction, "reaction");

BaseReaction::BaseReaction ()
{
   clear();
}

BaseReaction::~BaseReaction ()
{
}

void BaseReaction::clear()
{
   _reactantCount = 0;
   _productCount = 0;
   _catalystCount = 0;
   _allMolecules.clear();
   _atomAtomMapping.clear();
   _reactingCenters.clear();
   _inversionNumbers.clear();
   _types.clear();
   name.clear();
}

int BaseReaction::getAAM (int index, int atom)
{
   return _atomAtomMapping[index][atom];
}

int BaseReaction::getReactingCenter(int index, int bond)
{
   return _reactingCenters[index][bond];
}

int BaseReaction::getInversion (int index, int atom)
{
   return _inversionNumbers[index][atom];
}

Array<int> & BaseReaction::getAAMArray (int index)
{
   return _atomAtomMapping[index];
}

Array<int> & BaseReaction::getReactingCenterArray (int index)
{
   return _reactingCenters[index];
}

Array<int> & BaseReaction::getInversionArray (int index)
{
   return _inversionNumbers[index];
}

int BaseReaction::addReactant ()
{
   return _addBaseMolecule(REACTANT);
}

int BaseReaction::addProduct ()
{
   return _addBaseMolecule(PRODUCT);
}

int BaseReaction::addCatalyst ()
{
   return _addBaseMolecule(CATALYST);
}

void BaseReaction::_addedBaseMolecule (int idx, int side, BaseMolecule &mol)
{
   if (side == REACTANT)
      _reactantCount++;
   else if (side == PRODUCT)
      _productCount++;
   else // CATALYST
      _catalystCount++;

   _types.expand(idx + 1);
   _types[idx] = side;

   _atomAtomMapping.expand(idx + 1);
   _atomAtomMapping[idx].clear_resize(mol.vertexEnd());
   _atomAtomMapping[idx].zerofill();
   _reactingCenters.expand(idx + 1);
   _reactingCenters[idx].clear_resize(mol.edgeEnd());
   _reactingCenters[idx].zerofill();
   _inversionNumbers.expand(idx + 1);
   _inversionNumbers[idx].clear_resize(mol.vertexEnd());
   _inversionNumbers[idx].zerofill();
}

int BaseReaction::findAtomByAAM (int mol_idx, int aam) 
{
   BaseMolecule &mol = *_allMolecules.at(mol_idx);

   for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
      if (getAAM(mol_idx, i) == aam)
         return i;
   
   return -1;
}

int BaseReaction::findAamNumber (BaseMolecule *mol, int atom_number)
{
   for (int i = begin(); i < end(); i = next(i))
      if (mol == _allMolecules.at(i))
         return getAAM(i, atom_number);

   throw Error("cannot find aam number");
}

int BaseReaction::findReactingCenter (BaseMolecule *mol, int bond_number)
{
   for (int i = begin(); i < end(); i = next(i))
      if (mol == _allMolecules.at(i))
         return getReactingCenter(i, bond_number);
   
   throw Error("cannot find reacting center");
}

void BaseReaction::markStereocenterBonds()
{
   for (int i = begin(); i < end(); i = next(i))
   {
      _allMolecules[i]->clearBondDirections();
      _allMolecules[i]->stereocenters.markBonds();
      _allMolecules[i]->allene_stereo.markBonds();
   }
}

bool BaseReaction::haveCoord (BaseReaction &reaction)
{
   for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
      if (!reaction.getBaseMolecule(i).have_xyz)
         return false;
   return true;
}

int BaseReaction::_nextElement (int type, int index)
{
   if (index == -1)
      index = _allMolecules.begin();
   else
      index = _allMolecules.next(index);

   for (; index != _allMolecules.end(); index = _allMolecules.next(index))
   {
      if (_types[index] & type)
         break;
   }
   return index;
}

void BaseReaction::clearAAM ()
{
   for (int i = begin(); i < end(); i = next(i))
      _atomAtomMapping[i].zerofill();
}

int BaseReaction::addReactantCopy (BaseMolecule& mol, Array<int>* mapping, Array<int> *inv_mapping)
{
   int idx = _allMolecules.add(mol.neu());

   _allMolecules[idx]->clone(mol, mapping, inv_mapping);
   _addedBaseMolecule(idx, REACTANT, *_allMolecules[idx]);
   return idx;
}

int BaseReaction::addProductCopy (BaseMolecule& mol, Array<int>* mapping, Array<int> *inv_mapping)
{
   int idx = _allMolecules.add(mol.neu());

   _allMolecules[idx]->clone(mol, mapping, inv_mapping);
   _addedBaseMolecule(idx, PRODUCT, *_allMolecules[idx]);
   return idx;
}

int BaseReaction::addCatalystCopy (BaseMolecule& mol, Array<int>* mapping, Array<int> *inv_mapping)
{
   int idx = _allMolecules.add(mol.neu());
   
   _allMolecules[idx]->clone(mol, mapping, inv_mapping);
   _addedBaseMolecule(idx, CATALYST, *_allMolecules[idx]);
   return idx;
}

void BaseReaction::clone (BaseReaction &other, Array<int> *mol_mapping, ObjArray< Array<int> >* mappings, ObjArray< Array<int> >* inv_mappings)
{
   clear();
   
   int i, index = 0;
   QS_DEF(ObjArray< Array<int> >, tmp_mappings);

   if (mol_mapping != 0)
   {
      mol_mapping->clear_resize(other.end());
      mol_mapping->fffill();
   }

   if(mappings == 0)
      mappings = &tmp_mappings;
   mappings->clear();
   for (i = 0; i < other.end(); ++i)
      mappings->push();

   if (inv_mappings != 0)
      inv_mappings->clear();

   for(int i = other.begin(); i < other.end(); i = other.next(i))
   {
      BaseMolecule& rmol = other.getBaseMolecule(i);
      QS_DEF(Array<int>, inv_mapping);
      
      switch (other._types[i])
      {
         case REACTANT:
            index = addReactantCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
         case PRODUCT:
            index = addProductCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
         case CATALYST:
            index = addCatalystCopy(rmol, &mappings->at(i), &inv_mapping);
            break;
      }

      if (inv_mappings != 0)
      {
         inv_mappings->expand(index + 1);
         inv_mappings->at(index).copy(inv_mapping);
      }
      if (mol_mapping != 0)
         mol_mapping->at(i) = index;
      
      BaseMolecule &lmol = getBaseMolecule(index);
      for(int j = lmol.vertexBegin(); j < lmol.vertexEnd(); j = lmol.vertexNext(j)) {
         getAAMArray(index).at(j) = other.getAAM(i, mappings->at(i)[j]);
         getInversionArray(index).at(j) = other.getInversion(i, mappings->at(i)[j]);
      }
      for (int j = lmol.edgeBegin(); j < lmol.edgeEnd(); j = lmol.edgeNext(j)) {
         const Edge &edge = lmol.getEdge(j);
         int edge_idx = other.getBaseMolecule(i).findEdgeIndex(mappings->at(i)[edge.beg], mappings->at(i)[edge.end]);
         getReactingCenterArray(index).at(j) = other.getReactingCenter(i, edge_idx);
      }
      // subclass' stuff
      _clone(other, index, i, mappings);
   }

   name.copy(other.name);
}

void BaseReaction::_clone (BaseReaction &other, int index, int i, ObjArray< Array<int> >* mol_mappings)
{
}

Reaction & BaseReaction::asReaction ()
{
   throw Error("asReaction(): not a Reaction");
}

QueryReaction & BaseReaction::asQueryReaction ()
{
   throw Error("asQueryReaction(): not a QueryReaction");
}

bool BaseReaction::isQueryReaction ()
{
   return false;
}

void BaseReaction::remove (int i)
{
   int side = _types[i];

   if (side == REACTANT)
      _reactantCount--;
   else if (side == PRODUCT)
      _productCount--;
   else // CATALYST
      _catalystCount--;

   _allMolecules.remove(i);
}

int BaseReaction::begin ()
{
   return _nextElement(REACTANT | PRODUCT | CATALYST, -1);
}

int BaseReaction::end ()
{
   return _allMolecules.end();
}

int BaseReaction::next (int index)
{
   return _nextElement(REACTANT | PRODUCT | CATALYST, index);
}

int BaseReaction::count ()
{
   return _allMolecules.size();
}

int BaseReaction::findMolecule (BaseMolecule *mol)
{
   for (int i = begin(); i != end(); i = next(i))
      if (&getBaseMolecule(i) == mol)
         return i;

   return -1;
}
