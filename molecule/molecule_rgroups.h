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

#ifndef __molecule_rgroups__
#define __molecule_rgroups__

#include "base_cpp/red_black.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_pool.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;

struct RGroup
{
   explicit RGroup ();
   ~RGroup ();
   void clear();

   void copy (RGroup &other);

   bool occurrenceSatisfied (int value);

   PtrPool<BaseMolecule> fragments;
   int if_then;
   int rest_h;
   Array<int> occurrence;

protected:
   explicit RGroup (RGroup &other);
};

class DLLEXPORT MoleculeRGroups
{
public:

   MoleculeRGroups ();
   ~MoleculeRGroups ();

   DECL_ERROR;

   void copyRGroupsFromMolecule (MoleculeRGroups &other);

   RGroup &getRGroup  (int idx);
   int getRGroupCount () const;

   void clear ();

protected:
   
   ObjArray<RGroup> _rgroups;
};


}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
