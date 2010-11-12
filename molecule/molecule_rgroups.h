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

#ifndef __molecule_rgroups__
#define __molecule_rgroups__

#include "base_cpp/red_black.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_array.h"

namespace indigo {

class QueryMolecule;

struct RGroup
{
   explicit RGroup ();
   ~RGroup ();
   void clear();

   void copy (RGroup &other);

   inline int fragmentsCount () const {return fragments.size();}
   bool occurrenceSatisfied (int value) const;

   PtrArray<QueryMolecule> fragments;
   int if_then;
   int rest_h;
   Array<int> occurrence;

protected:
   explicit RGroup (RGroup &other);
};

class MoleculeRGroups
{
public:

   DLLEXPORT MoleculeRGroups ();
   DLLEXPORT ~MoleculeRGroups ();

   DEF_ERROR("molecule rgroups");

   DLLEXPORT void copyRGroupsFromMolecule (MoleculeRGroups &other);

   DLLEXPORT RGroup &getRGroup  (int idx);
   DLLEXPORT int getRGroupCount () const;

   DLLEXPORT void clear ();

protected:
   
   ObjArray<RGroup> _rgroups;
};

struct MoleculeRGroupFragment
{
   MoleculeRGroupFragment () {}

   void addAttachmentPoint (int order, int index);
   int  getAttachmentPoint (int order, int index) const { return index < _attachment_index[order].size() ? _attachment_index[order][index] : -1; }
   void removeAttachmentPoint (int index);
   int  attachmentPointCount () const { return _attachment_index.size(); }

protected:
   ObjArray< Array<int> > _attachment_index;
};

}

#endif
