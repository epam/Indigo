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

#ifndef __indigo_loaders__
#define __indigo_loaders__

#include "indigo_internal.h"

#include "molecule/molecule.h"
#include "graph/graph_highlighting.h"
#include "reaction/reaction.h"
#include "reaction/reaction_highlighting.h"

class IndigoRdfData : public IndigoObject
{
public:
   IndigoRdfData (int type, Array<char> &data, int index, int offset);
   IndigoRdfData (int type, Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                  int index, int offset);
   virtual ~IndigoRdfData ();

   Array<char> & getRawData ();
   virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   virtual int getIndex ();
   int tell ();

protected:
   Array<char> _data;
   RedBlackStringObjMap< Array<char> > _properties;

   bool _loaded;
   int _index;
   int _offset;
};

class IndigoRdfMolecule : public IndigoRdfData
{
public:
   IndigoRdfMolecule (Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                      int index, int offset);
   virtual ~IndigoRdfMolecule ();

   virtual Molecule & getMolecule ();
   virtual BaseMolecule & getBaseMolecule ();
   GraphHighlighting * getMoleculeHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Molecule _mol;
   GraphHighlighting _highlighting;
};

class IndigoRdfReaction : public IndigoRdfData
{
public:
   IndigoRdfReaction (Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                      int index, int offset);
   virtual ~IndigoRdfReaction ();

   virtual Reaction & getReaction ();
   virtual BaseReaction & getBaseReaction ();
   ReactionHighlighting * getReactionHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Reaction _rxn;
   ReactionHighlighting _highlighting;
};


class IndigoSdfLoader : public IndigoObject
{
public:
   IndigoSdfLoader (Scanner &scanner);
   IndigoSdfLoader (const char *filename);
   virtual ~IndigoSdfLoader ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   IndigoObject * at (int index);

   int tell ();

   GraphHighlighting highlighting;
   SdfLoader *sdf_loader;

protected:
   Scanner  *_own_scanner;
};

class IndigoRdfLoader : public IndigoObject
{
public:
   IndigoRdfLoader (Scanner &scanner);
   IndigoRdfLoader (const char *filename);
   virtual ~IndigoRdfLoader ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   IndigoObject * at (int index);

   int tell ();

   RdfLoader *rdf_loader;
protected:
   Scanner  *_own_scanner;
};

class IndigoSmilesMolecule : public IndigoRdfData
{
public:
   IndigoSmilesMolecule (Array<char> &smiles, int index, int offset);
   virtual ~IndigoSmilesMolecule ();

   virtual Molecule & getMolecule ();
   virtual BaseMolecule & getBaseMolecule ();
   GraphHighlighting * getMoleculeHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Molecule _mol;
   GraphHighlighting _highlighting;
};

class IndigoSmilesReaction : public IndigoRdfData
{
public:
   IndigoSmilesReaction (Array<char> &data, int index, int offset);
   virtual ~IndigoSmilesReaction ();

   virtual Reaction & getReaction ();
   virtual BaseReaction & getBaseReaction ();
   ReactionHighlighting * getReactionHighlighting ();
   virtual const char * getName ();
   virtual IndigoObject * clone ();

protected:
   Reaction _rxn;
   ReactionHighlighting _highlighting;
};

class IndigoMultilineSmilesLoader : public IndigoObject
{
public:
   IndigoMultilineSmilesLoader (Scanner &scanner);
   IndigoMultilineSmilesLoader (const char *filename);
   virtual ~IndigoMultilineSmilesLoader ();

   int tell ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

   IndigoObject * at (int index);
   int count ();

protected:
   Scanner    *_scanner;
   Array<char> _str;
   bool      _own_scanner;

   void _advance ();

   TL_CP_DECL(Array<int>, _offsets);
   int _current_number;
   int _max_offset;
};


#endif
