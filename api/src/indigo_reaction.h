/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#ifndef __indigo_reaction__
#define __indigo_reaction__

#include "indigo_internal.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_highlighting.h"

class IndigoBaseReaction : public IndigoObject
{
public:
   explicit IndigoBaseReaction (int type_);

   virtual ~IndigoBaseReaction ();

   virtual ReactionHighlighting * getReactionHighlighting ();
   virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   ReactionHighlighting highlighting;
   RedBlackStringObjMap< Array<char> > properties;
};

class IndigoReaction : public IndigoBaseReaction
{
public:
   DLLEXPORT IndigoReaction ();
   DLLEXPORT virtual ~IndigoReaction ();

   DLLEXPORT virtual BaseReaction & getBaseReaction ();
   DLLEXPORT virtual Reaction & getReaction ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT virtual IndigoObject * clone ();

   DLLEXPORT static IndigoReaction * cloneFrom (IndigoObject & obj);

   Reaction rxn;
};

class IndigoQueryReaction : public IndigoBaseReaction
{
public:
   DLLEXPORT IndigoQueryReaction ();
   DLLEXPORT virtual ~IndigoQueryReaction ();

   DLLEXPORT virtual BaseReaction & getBaseReaction ();
   DLLEXPORT virtual QueryReaction & getQueryReaction ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT virtual IndigoObject * clone();

   DLLEXPORT static IndigoQueryReaction * cloneFrom (IndigoObject & obj);

   QueryReaction rxn;
};

class IndigoReactionMolecule : public IndigoObject
{
public:
   IndigoReactionMolecule (BaseReaction &reaction, ReactionHighlighting *highlighting, int index);
   virtual ~IndigoReactionMolecule ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual QueryMolecule & getQueryMolecule ();
   virtual Molecule & getMolecule ();
   virtual GraphHighlighting * getMoleculeHighlighting ();
   virtual int getIndex ();
   virtual IndigoObject * clone ();

   BaseReaction &rxn;
   ReactionHighlighting *hl;
   int idx;
};

class IndigoReactionIter : public IndigoObject
{
public:
   enum
   {
      REACTANTS,
      PRODUCTS,
      MOLECULES
   };

   IndigoReactionIter (BaseReaction &rxn, ReactionHighlighting *hl, int subtype);
   virtual ~IndigoReactionIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _begin ();
   int _end ();
   int _next (int i);

   int _subtype;
   BaseReaction &_rxn;
   ReactionHighlighting *_hl;
   int _idx;
};


#endif
