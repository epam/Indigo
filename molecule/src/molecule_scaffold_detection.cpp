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

#include "molecule/molecule_scaffold_detection.h"
#include "molecule/max_common_submolecule.h"
#include "base_cpp/array.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/query_molecule.h"

using namespace indigo;

MoleculeScaffoldDetection::MoleculeScaffoldDetection (ObjArray<Molecule>* mol_set):
ScaffoldDetection(0),
flags(MoleculeExactMatcher::CONDITION_ALL &(~MoleculeExactMatcher::CONDITION_STEREO)),
searchStructures(mol_set),
basketStructures(0){
   cbEdgeWeight = MaxCommonSubmolecule::matchBonds;
   cbVerticesColor = MaxCommonSubmolecule::matchAtoms;
   userdata = &flags;
}


void MoleculeScaffoldDetection::_searchScaffold(Molecule& scaffold, bool approximate) {
   QS_DEF(ObjArray<Molecule>, temp_set);
   if(basketStructures == 0) {
      basketStructures = &temp_set;
   }
   MoleculeBasket mol_basket;
   mol_basket.initBasket(searchStructures, basketStructures, GraphBasket::MAX_MOLECULES_NUMBER);

   if(approximate)
      _searchApproximateScaffold(mol_basket);
   else
      _searchExactScaffold(mol_basket);

   int max_index = mol_basket.getMaxGraphIndex();
   if(basketStructures->size() == 0)
      throw Error("There are no scaffolds found");

   //clear all stereocenters
   for(int i = 0; i < basketStructures->size(); ++i)
      basketStructures->at(i).stereocenters.clear();
   //get max scaffold from basket
   scaffold.clone(basketStructures->at(max_index), 0, 0);
   
}

MoleculeScaffoldDetection::MoleculeBasket::MoleculeBasket():
cbSortSolutions(0),
_searchStructures(0),
_basketStructures(0) {
}

MoleculeScaffoldDetection::MoleculeBasket::~MoleculeBasket()
{
}

void MoleculeScaffoldDetection::MoleculeBasket::initBasket(ObjArray<Molecule>* mol_set, ObjArray<Molecule>* basket_set, int max_number) {

   if(mol_set == 0)
      throw Error("Graph set null pointer");
   if(basket_set == 0)
      throw Error("Basket set null pointer");

   _searchStructures = mol_set;
   _basketStructures = basket_set;

   _sortGraphsInSet();

   _basketStructures->clear();

   for(int i = 0; i < max_number; i++) 
      _basketStructures->push();

   _directIterator.resize(max_number);
   _reverseIterator.resize(max_number);
   _reverseIterator.set();

   _basketStructures->at(0).clone(_searchStructures->at(_orderArray[0]),0,0);
   _reverseIterator.set(0, false);
   _directIterator.set(0);
}


Molecule& MoleculeScaffoldDetection::MoleculeBasket::pickOutNextMolecule() {

   int empty_index = _reverseIterator.nextSetBit(0);

   if(empty_index == -1) {
      _directIterator.resize(_directIterator.size()+NEXT_SOLUTION_SIZE_SUM);
      _reverseIterator.resize(_directIterator.size());
      for(int i = _directIterator.size()-NEXT_SOLUTION_SIZE_SUM; i < _directIterator.size(); i++)
         _reverseIterator.set(i);
      empty_index = _basketStructures->size();
      for(int i = 0; i < NEXT_SOLUTION_SIZE_SUM; i++)
         _basketStructures->push();
   }

   _reverseIterator.set(empty_index, false);
   return _basketStructures->at(empty_index);
}

void MoleculeScaffoldDetection::MoleculeBasket::addToNextEmptySpot(Graph& graph, Array<int> &v_list, Array<int> &e_list) {
   Molecule& mol = (Molecule&) graph;
   Molecule & b_mol = pickOutNextMolecule();
   b_mol.makeEdgeSubmolecule(mol, v_list, e_list, 0);
}

int MoleculeScaffoldDetection::MoleculeBasket::getMaxGraphIndex() {


   for(int x = _reverseIterator.nextSetBit(0); x >= 0; x = _reverseIterator.nextSetBit(x+1)) {
      Molecule& mol_basket = _basketStructures->at(x);

      if(mol_basket.vertexCount() > 0) 
         mol_basket.clear();
   }

   if(cbSortSolutions == 0)
      _basketStructures->qsort(_compareRingsCount, 0);
   else
      _basketStructures->qsort(cbSortSolutions, userdata);

   while(_basketStructures->size() && _basketStructures->top().vertexCount() == 0)
      _basketStructures->pop();

   return 0;
}

Graph& MoleculeScaffoldDetection::MoleculeBasket::getGraph(int index) const {
   if(index >= _basketStructures->size())
      throw Error("basket size < index");
   return (Graph&)_basketStructures->at(index);
}

void MoleculeScaffoldDetection::MoleculeBasket::_sortGraphsInSet() {
   int set_size = _searchStructures->size();
   
   if(set_size == 0)
      throw Error("Graph set size == 0");

   _orderArray.clear();
   for(int i = 0; i < set_size; i++) {
      if(_searchStructures->at(i).vertexCount() > 0) {
         _orderArray.push(i);
         ++_graphSetSize;
      }
   }

   //sort in order of edgeCount
   _orderArray.qsort(_compareEdgeCount, _searchStructures);
}

int MoleculeScaffoldDetection::MoleculeBasket::_compareEdgeCount(int &i1,int &i2,void* context){
   ObjArray<Molecule> &graph_set = *(ObjArray<Molecule> *)context;
   return graph_set.at(i1).edgeCount()-graph_set.at(i2).edgeCount();
}

int MoleculeScaffoldDetection::MoleculeBasket::_compareRingsCount(Molecule& g1, Molecule& g2, void* ) {
   //maximize number of the rings/ v-e+r=2 there v- number of vertices e - number of edges r - number of rings
   int result = (g2.edgeCount() - g2.vertexCount()) - (g1.edgeCount() - g1.vertexCount());
   if(result == 0 || g1.edgeCount() == 0 || g2.edgeCount() == 0)
      result = g2.edgeCount() - g1.edgeCount();
   return result;
}


