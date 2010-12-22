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

#ifndef __indigo_decomposition__
#define __indigo_decomposition__

#include "indigo_internal.h"
#include "molecule/molecule_decomposer.h"
#include "base_cpp/auto_ptr.h"

class DLLEXPORT IndigoDecomposition : public IndigoObject
{
public:
   IndigoDecomposition (BaseMolecule &mol);

   static IndigoDecomposition & cast (IndigoObject &obj);

   BaseMolecule & mol;
   MoleculeDecomposer decomposer;
};

class IndigoComponentsIterator : public IndigoObject
{
public:
   IndigoComponentsIterator (IndigoDecomposition &decomp);

   virtual IndigoObject * next ();
   virtual bool hasNext ();
   
protected:
   IndigoDecomposition &_decomp;
   int _idx;
};

class IndigoMoleculeComponent : public IndigoObject
{
public:

   IndigoMoleculeComponent (IndigoDecomposition &decomp, int idx);

   virtual int getIndex ();
   virtual BaseMolecule & getBaseMolecule ();
   virtual QueryMolecule & getQueryMolecule ();
   virtual Molecule & getMolecule ();
   virtual GraphHighlighting * getMoleculeHighlighting();

protected:

   void _validateMol ();

   AutoPtr<BaseMolecule> _mol;
   IndigoDecomposition &_decomp;
   int _idx;
};

class IndigoComponentAtomsIterator : public IndigoObject
{
public:
   IndigoComponentAtomsIterator (IndigoDecomposition &decomp, int cidx);

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _next ();

   IndigoDecomposition &_decomp;
   int _cidx;
   int _idx;
};

class IndigoComponentBondsIterator : public IndigoObject
{
public:
   IndigoComponentBondsIterator (IndigoDecomposition &decomp, int cidx);

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:

   int _next ();

   IndigoDecomposition &_decomp;
   int _cidx;
   int _idx;
};

#endif
