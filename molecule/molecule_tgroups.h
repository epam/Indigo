/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __molecule_tgroups__
#define __molecule_tgroups__

#include "base_cpp/tlscont.h"
#include "base_cpp/red_black.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_pool.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;

class TGroup
{
public:
   Array<char> tgroup_class;
   Array<char> tgroup_name;
   Array<char> tgroup_alias;
   Array<char> tgroup_comment;
   int tgroup_id;

   TGroup ();
   ~TGroup ();

   void copy (TGroup &other);
   void clear();
   static int cmp (TGroup &tg1, TGroup &tg2, void *context);

   BaseMolecule* fragment;

private:
   TGroup (const TGroup &);
};

class DLLEXPORT MoleculeTGroups
{
public:
   MoleculeTGroups ();
   ~MoleculeTGroups ();

   DECL_ERROR;

   int addTGroup ();
   TGroup &getTGroup (int idx);
   int getTGroupCount ();

   void remove(int idx);
   void clear ();

   void copyTGroupsFromMolecule (MoleculeTGroups &other);
   int findTGroup(const char *name);

   int begin();
   int end();
   int next(int i);

protected:
   PtrPool<TGroup> _tgroups;
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
