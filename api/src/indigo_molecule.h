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

#ifndef __indigo_molecule__
#define __indigo_molecule__

#include "indigo_internal.h"
#include "graph/graph_highlighting.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule.h"

class IndigoBaseMolecule : public IndigoObject
{
public:
   DLLEXPORT explicit IndigoBaseMolecule (int type_);

   DLLEXPORT virtual ~IndigoBaseMolecule ();

   DLLEXPORT virtual GraphHighlighting * getMoleculeHighlighting ();
   DLLEXPORT virtual RedBlackStringObjMap< Array<char> > * getProperties ();

   DLLEXPORT const char * debugInfo ();

   GraphHighlighting highlighting;

   RedBlackStringObjMap< Array<char> > properties;
};

class IndigoQueryMolecule : public IndigoBaseMolecule
{
public:
   DLLEXPORT IndigoQueryMolecule ();

   DLLEXPORT virtual ~IndigoQueryMolecule ();

   DLLEXPORT virtual BaseMolecule & getBaseMolecule ();
   DLLEXPORT virtual QueryMolecule & getQueryMolecule ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT static IndigoQueryMolecule * cloneFrom (IndigoObject & obj);

   DLLEXPORT const char * debugInfo ();

   DLLEXPORT virtual IndigoObject * clone ();

   QueryMolecule qmol;
};

class IndigoMolecule : public IndigoBaseMolecule
{
public:
   DLLEXPORT IndigoMolecule ();

   DLLEXPORT virtual ~IndigoMolecule ();

   DLLEXPORT virtual BaseMolecule & getBaseMolecule ();
   DLLEXPORT virtual Molecule & getMolecule ();
   DLLEXPORT virtual const char * getName ();

   DLLEXPORT static IndigoMolecule * cloneFrom (IndigoObject & obj);

   DLLEXPORT const char * debugInfo ();

   DLLEXPORT virtual IndigoObject * clone ();

   Molecule mol;
};

class IndigoAtom : public IndigoObject
{
public:
   IndigoAtom (BaseMolecule &mol_, int idx_);
   virtual ~IndigoAtom ();

   DLLEXPORT static IndigoAtom & cast (IndigoObject &obj);

   BaseMolecule *mol;
   int idx;

   virtual int getIndex ();
};

class IndigoRGroup : public IndigoObject
{
public:
   IndigoRGroup ();
   virtual ~IndigoRGroup ();

   virtual int getIndex ();

   static DLLEXPORT IndigoRGroup & cast (IndigoObject &obj);

   QueryMolecule *mol;
   int idx;
};

class IndigoRGroupFragment : public IndigoObject
{
public:
   IndigoRGroupFragment (IndigoRGroup &rgp, int idx);
   IndigoRGroupFragment (QueryMolecule *mol, int rgroup_idx, int fragment_idx);

   virtual ~IndigoRGroupFragment ();

   virtual QueryMolecule & getQueryMolecule ();
   virtual BaseMolecule & getBaseMolecule ();
   virtual int getIndex ();

   IndigoRGroup rgroup;
   int frag_idx;
};

class IndigoBond : public IndigoObject
{
public:
   IndigoBond (BaseMolecule &mol_, int idx_);
   virtual ~IndigoBond ();

   DLLEXPORT static IndigoBond & cast (IndigoObject &obj);

   BaseMolecule *mol;
   int idx;

   virtual int getIndex ();
};

class IndigoAtomNeighbor : public IndigoAtom
{
public:
   explicit IndigoAtomNeighbor (BaseMolecule &mol_, int atom_idx, int bond_idx);
   virtual ~IndigoAtomNeighbor ();

   int bond_idx;
};

class IndigoAtomNeighborsIter : public IndigoObject
{
public:
   IndigoAtomNeighborsIter (BaseMolecule *molecule, int atom_idx);

   virtual ~IndigoAtomNeighborsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _atom_idx;
   int _nei_idx;
   BaseMolecule *_mol;
};

class IndigoRGroupsIter : public IndigoObject
{
public:
   IndigoRGroupsIter (QueryMolecule *mol);

   virtual ~IndigoRGroupsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   QueryMolecule *_mol;
   int _idx;
};

class IndigoRGroupFragmentsIter : public IndigoObject
{
public:
   IndigoRGroupFragmentsIter (IndigoRGroup &rgroup);
   virtual ~IndigoRGroupFragmentsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   QueryMolecule *_mol;
   int _rgroup_idx;
   int _frag_idx;
};

class IndigoAtomsIter : public IndigoObject
{
public:
   enum
   {
      ALL,
      PSEUDO,
      RSITE,
      STEREOCENTER
   };

   IndigoAtomsIter (BaseMolecule *molecule, int type);

   virtual ~IndigoAtomsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _shift (int idx);

   int _type;
   int _idx;
   BaseMolecule *_mol;
};

class IndigoBondsIter : public IndigoObject
{
public:
   IndigoBondsIter (BaseMolecule *molecule);

   virtual ~IndigoBondsIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _idx;
   BaseMolecule *_mol;
};

#endif
