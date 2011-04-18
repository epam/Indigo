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

#ifndef __indigo_array__
#define __indigo_array__

#include "indigo_internal.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

class DLLEXPORT IndigoArray : public IndigoObject
{
public:
   IndigoArray ();

   virtual ~IndigoArray ();

   virtual IndigoObject * clone ();

   static bool is (IndigoObject &obj);
   static IndigoArray & cast (IndigoObject &obj);

   PtrArray<IndigoObject> objects;
};

class DLLEXPORT IndigoArrayElement : public IndigoObject
{
public:
   IndigoArrayElement (IndigoArray &arr, int idx_);
   virtual ~IndigoArrayElement ();

   IndigoObject & get ();

   virtual BaseMolecule & getBaseMolecule ();
   virtual Molecule & getMolecule ();
   virtual QueryMolecule & getQueryMolecule ();

   virtual BaseReaction & getBaseReaction ();
   virtual Reaction & getReaction ();

   virtual IndigoObject * clone ();

   virtual const char * getName ();

   virtual int getIndex ();

   IndigoArray *array;
   int idx;
};

class IndigoArrayIter : public IndigoObject
{
public:
   IndigoArrayIter (IndigoArray &arr);
   virtual ~IndigoArrayIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();
protected:
   IndigoArray *_arr;
   int _idx;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
