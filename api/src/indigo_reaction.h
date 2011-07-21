/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

class DLLEXPORT IndigoBaseReaction : public IndigoObject
{
public:
   explicit IndigoBaseReaction (int type_);

   virtual ~IndigoBaseReaction ();

   virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   static bool is (IndigoObject &obj);

   virtual const char * debugInfo ();

   RedBlackStringObjMap< Array<char> > properties;
};

class DLLEXPORT IndigoReaction : public IndigoBaseReaction
{
public:
   IndigoReaction ();
   virtual ~IndigoReaction ();

   virtual BaseReaction & getBaseReaction ();
   virtual Reaction & getReaction ();
   virtual const char * getName ();

   virtual IndigoObject * clone ();

   static IndigoReaction * cloneFrom (IndigoObject & obj);

   virtual const char * debugInfo ();

   Reaction rxn;
};

class DLLEXPORT IndigoQueryReaction : public IndigoBaseReaction
{
public:
   IndigoQueryReaction ();
   virtual ~IndigoQueryReaction ();

   virtual BaseReaction & getBaseReaction ();
   virtual QueryReaction & getQueryReaction ();
   virtual const char * getName ();

   virtual IndigoObject * clone();

   static IndigoQueryReaction * cloneFrom (IndigoObject & obj);

   virtual const char * debugInfo ();

   QueryReaction rxn;
};

class IndigoReactionMolecule : public IndigoObject
{
public:
   IndigoReactionMolecule (BaseReaction &reaction, int index);
   virtual ~IndigoReactionMolecule ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual QueryMolecule & getQueryMolecule ();
   virtual Molecule & getMolecule ();
   virtual int getIndex ();
   virtual IndigoObject * clone ();
   virtual RedBlackStringObjMap< Array<char> > * getProperties ();
   virtual void remove ();

   virtual const char * debugInfo ();

   BaseReaction &rxn;
   int idx;
};

class IndigoReactionIter : public IndigoObject
{
public:
   enum
   {
      REACTANTS,
      PRODUCTS,
      CATALYSTS,
      MOLECULES
   };

   IndigoReactionIter (BaseReaction &rxn, int subtype);
   virtual ~IndigoReactionIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   virtual const char * debugInfo ();

protected:

   int _begin ();
   int _end ();
   int _next (int i);

   int _subtype;
   BaseReaction &_rxn;
   int _idx;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
