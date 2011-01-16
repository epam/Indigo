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

#ifndef __molecule_scaffold_detection_h_
#define __molecule_scaffold_detection_h_
#include "graph/scaffold_detection.h"
#include "graph/max_common_subgraph.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

namespace indigo {

//class for searching scaffold molecule from molecules set
class MoleculeScaffoldDetection: public ScaffoldDetection {
   public:

   //class for keeping molecules
   class MoleculeBasket:public ScaffoldDetection::GraphBasket {
   public:
      MoleculeBasket();
      virtual ~MoleculeBasket();

      //initializes molecules basket
      void initBasket(ObjArray<Molecule>* mol_set, ObjArray<Molecule>* basket_set, int max_number);
      //this method adds molecules from set (defines with edges and vertices lists) to basket queue 
      virtual void addToNextEmptySpot(Graph& graph, Array<int> &v_list, Array<int> &e_list);

      virtual Graph& getGraphFromSet(int idx) {return (Graph&)_searchStructures->at(_orderArray[idx]); }

      virtual int getMaxGraphIndex();


      //returns ptr of molecule in basket with index
      virtual Graph& getGraph(int index) const;
      //adds new molecule to queue and returns ptr of that
      Molecule& pickOutNextMolecule();


      int (*cbSortSolutions) (Molecule &mol1, Molecule &mol2, void *userdata);



      DEF_ERROR("Mol basket");
      

   private:
      virtual void _sortGraphsInSet();

      
      
      static int _compareEdgeCount(int &i1,int &i2,void* context);
      static int _compareRingsCount(Molecule& m1, Molecule& m2, void* context);

      ObjArray<Molecule>* _searchStructures;
      ObjArray<Molecule>* _basketStructures;

      MoleculeBasket(const MoleculeBasket&); //no implicit copy
   };

private:
   void _searchScaffold(Molecule& scaffold, bool approximate);

public:
   MoleculeScaffoldDetection (ObjArray<Molecule>* mol_set);
   
   //two main methods for extracting scaffolds
   //extracting exact scaffold from molecules set
   void extractExactScaffold (Molecule& scaffold) {_searchScaffold(scaffold, false); }
   void extractApproximateScaffold(Molecule& scaffold) {_searchScaffold(scaffold, true); }
   //extracting approximate scaffold from molecule set


   int (*cbSortSolutions) (Molecule &mol1, Molecule &mol2, const void *userdata);

   int flags;

   ObjArray<Molecule>* searchStructures;
   ObjArray<Molecule>* basketStructures;

   DEF_ERROR("Molecule Scaffold detection");
};

}

#endif
