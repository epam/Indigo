/****************************************************************************
* Copyright (C) 2010-2015 GGA Software Services LLC
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

#ifndef __molecule_tautomer_enumerator__
#define __molecule_tautomer_enumerator__

#include "base_cpp/reusable_obj_array.h"
#include "molecule/molecule.h"
#include "molecule/molecule_hyper_molecule.h"

class TautomerEnumerator
{
public:
   TautomerEnumerator(Molecule &molecule, const char *params);

   void constructMolecule(Molecule &molecule, int layer);
   int size();

protected:
   struct Breadcrumps
   {
      Dbitset forwardMask;
      Dbitset backwardMask;
      ObjArray<Dbitset> forwardEdgesHistory;
      ObjArray<Dbitset> backwardEdgesHistory;
      Array<int> nodesHistory;
      Array<int> edgesHistory;
   };

   static bool matchEdge(Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void *userdata);
   static bool matchVertex(Graph &subgraph, Graph &supergraph, const int *core_sub, int sub_idx, int super_idx, void *userdata);
   static void edgeAdd(Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void *userdata);
   static void vertexAdd(Graph &subgraph, Graph &supergraph, int sub_idx, int super_idx, void *userdata);
   static void vertexRemove(Graph &subgraph, int sub_idx, void *userdata);

private:
   HyperMolecule _hyperMolecule;
};


class IndigoMoleculeTautomer : public IndigoObject
{
public:
   IndigoMoleculeTautomer(TautomerEnumerator &enumerator, int index);
   virtual ~IndigoMoleculeTautomer();

   virtual Molecule & getMolecule();
   virtual IndigoObject * clone();
   virtual RedBlackStringObjMap< Array<char> > * getProperties();

   virtual const char * debugInfo();

private:
   Molecule _molInstance;
};

class IndigoTautomerIter : public IndigoObject
{
public:
   IndigoTautomerIter(Molecule &molecule, const char *params);
   virtual ~IndigoTautomerIter();

   virtual IndigoObject * next();
   virtual bool hasNext();

   virtual const char * debugInfo();

protected:

   TautomerEnumerator _enumerator;
   int _layer;
};

#endif /* __molecule_tautomer_enumerator__ */
