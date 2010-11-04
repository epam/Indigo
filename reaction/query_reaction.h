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

#ifndef __query_reaction_h__
#define __query_reaction_h__

#include "reaction/base_reaction.h"

class QueryMolecule;

class QueryReaction : public BaseReaction
{
public:
   DLLEXPORT QueryReaction ();
   DLLEXPORT virtual ~QueryReaction ();

   DLLEXPORT virtual void clear ();

   DLLEXPORT QueryMolecule & getQueryMolecule (int index);

   DLLEXPORT Array<int> & getExactChangeArray (int index);
   
   DLLEXPORT int getExactChange (int index, int atom);

   DLLEXPORT void makeTransposedForSubstructure (QueryReaction &other);

   DLLEXPORT int _addedQueryMolecule (int side, QueryMolecule &mol);

   DLLEXPORT virtual void aromatize ();
   DLLEXPORT virtual void dearomatize ();

   DLLEXPORT virtual BaseReaction * neu ();

   DLLEXPORT virtual QueryReaction & asQueryReaction ();
   DLLEXPORT virtual bool isQueryReaction ();

protected:
   DLLEXPORT void _transposeMoleculeForSubstructure (int index, Array<int> &transposition);

   DLLEXPORT virtual int _addBaseMolecule (int side);

   DLLEXPORT virtual void _addedBaseMolecule (int idx, int side, BaseMolecule &mol);

   struct _SortingContext
   {
      explicit _SortingContext (QueryMolecule &mol, const Array<int> &r) :
      m(mol), rdata(r)
      {
      }

      QueryMolecule &m;
      const Array<int> &rdata;
   };

   static int _compare(int &i1, int &i2, void *c);

   ObjArray< Array<int> > _exactChanges;

   DLLEXPORT virtual void _clone (BaseReaction &other, int index, int i, ObjArray< Array<int> >* mol_mappings);

private:
   QueryReaction (const QueryReaction &); // no implicit copy
};

#endif
