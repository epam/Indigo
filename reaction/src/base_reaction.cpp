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

#include "reaction/base_reaction.h"
#include "base_cpp/tlscont.h"

using namespace indigo;

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
   _indexes.clear();
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

   _indexes.expand(idx + 1);
   _indexes[idx] = side;

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
      _allMolecules[i]->stereocenters.markBonds();
}

bool BaseReaction::haveCoord (BaseReaction &reaction)
{
   for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
      if (!reaction.getBaseMolecule(i).have_xyz)
         return false;
   return true;
}

int BaseReaction::_nextElement (int type, int index) const
{
   for (++index; index < _indexes.size(); ++index)
   {
      if (_indexes[index] & type)
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
   int idx = _allMolecules.size();

   _allMolecules.add(mol.neu());
   _allMolecules[idx]->clone(mol, mapping, inv_mapping);
   _addedBaseMolecule(idx, REACTANT, *_allMolecules[idx]);
   return idx;
}

int BaseReaction::addProductCopy (BaseMolecule& mol, Array<int>* mapping, Array<int> *inv_mapping)
{
   int idx = _allMolecules.size();

   _allMolecules.add(mol.neu());
   _allMolecules[idx]->clone(mol, mapping, inv_mapping);
   _addedBaseMolecule(idx, PRODUCT, *_allMolecules[idx]);
   return idx;
}

int BaseReaction::addCatalystCopy (BaseMolecule& mol, Array<int>* mapping, Array<int> *inv_mapping)
{
   int idx = _allMolecules.size();

   _allMolecules.add(mol.neu());
   _allMolecules[idx]->clone(mol, mapping, inv_mapping);
   _addedBaseMolecule(idx, CATALYST, *_allMolecules[idx]);
   return idx;
}

void BaseReaction::clone (BaseReaction &other, ObjArray< Array<int> >* mappings, ObjArray< Array<int> >* inv_mappings)
{
   clear();
   int index = 0;

   QS_DEF(ObjArray< Array<int> >, tmp_mappings);
   ObjArray< Array<int> >* mol_mappings = mappings;

   if(mol_mappings == 0) {
      mol_mappings = &tmp_mappings;
   }
   mol_mappings->clear();
   if(inv_mappings != 0)
      inv_mappings->clear();

   for(int i = 0; i < other.end(); ++i) {
      mol_mappings->push();
      if(inv_mappings != 0)
         inv_mappings->push();
   }

   for(int i = other.begin(); i < other.end(); i = other.next(i)) {
      BaseMolecule& rmol = other.getBaseMolecule(i);
      Array<int> * inv_mapping = 0;
      if(inv_mappings != 0)
         inv_mapping = &inv_mappings->at(i);
      switch(other._indexes[i]) {
         case REACTANT:
            index = addReactantCopy(rmol, &mol_mappings->at(i), inv_mapping);
            break;
         case PRODUCT:
            index = addProductCopy(rmol, &mol_mappings->at(i), inv_mapping);
            break;
         case CATALYST:
            index = addCatalystCopy(rmol, &mol_mappings->at(i), inv_mapping);
            break;
      }

      for(int j = rmol.vertexBegin(); j < rmol.vertexEnd(); j = rmol.vertexNext(j)) {
         getAAMArray(index).at(j) = other.getAAM(i, mol_mappings->at(i)[j]);
         getInversionArray(index).at(j) = other.getInversion(i, mol_mappings->at(i)[j]);
      }
      for (int j = getBaseMolecule(index).edgeBegin(); j < getBaseMolecule(index).edgeEnd(); j = getBaseMolecule(index).edgeNext(j)) {
         const Edge &edge = getBaseMolecule(index).getEdge(j);
         int edge_idx = other.getBaseMolecule(i).findEdgeIndex(mol_mappings->at(i)[edge.beg], mol_mappings->at(i)[edge.end]);
         getReactingCenterArray(index).at(j) = other.getReactingCenter(i, edge_idx);
      }
      // subclass' stuff
      _clone(other, index, i, mol_mappings);
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
