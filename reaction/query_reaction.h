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

#ifndef __query_reaction_h__
#define __query_reaction_h__

#include "reaction/base_reaction.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class QueryMolecule;

class DLLEXPORT QueryReaction : public BaseReaction
{
public:
   QueryReaction ();
   virtual ~QueryReaction ();

   virtual void clear ();

   QueryMolecule & getQueryMolecule (int index);

   Array<int> & getExactChangeArray (int index);
   
   int getExactChange (int index, int atom);

   void makeTransposedForSubstructure (QueryReaction &other);

   int _addedQueryMolecule (int side, QueryMolecule &mol);

   virtual bool aromatize (const AromaticityOptions &options);
   virtual bool dearomatize (const AromaticityOptions &options);

   virtual BaseReaction * neu ();

   virtual QueryReaction & asQueryReaction ();
   virtual bool isQueryReaction ();
   Array<int> & getIgnorableAAMArray (int index);
   int getIgnorableAAM (int index, int atom);

   void optimize ();

protected:
   void _transposeMoleculeForSubstructure (int index, Array<int> &transposition);

   virtual int _addBaseMolecule (int side);

   virtual void _addedBaseMolecule (int idx, int side, BaseMolecule &mol);

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
   ObjArray< Array<int> > _ignorableAAM;

   virtual void _clone (BaseReaction &other, int index, int i, ObjArray< Array<int> >* mol_mappings);

private:
   QueryReaction (const QueryReaction &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
