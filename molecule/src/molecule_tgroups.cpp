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

#include "molecule/molecule_tgroups.h"
#include "base_cpp/auto_ptr.h"
#include "molecule/base_molecule.h"
#include "base_cpp/scanner.h"

using namespace indigo;

TGroup::TGroup ()
{
}

TGroup::~TGroup ()
{
}

void TGroup::clear()
{
}

int TGroup::cmp (TGroup &tg1, TGroup &tg2, void *context)
{
   if (tg1.fragment == 0)
      return -1;
   if (tg2.fragment == 0)
      return 1;

   return tg2.fragment->vertexCount() - tg1.fragment->vertexCount();
}

void TGroup::copy (TGroup &other)
{
   tgroup_class.copy(other.tgroup_class);
   tgroup_name.copy(other.tgroup_name);
   tgroup_alias.copy(other.tgroup_alias);
   tgroup_comment.copy(other.tgroup_comment);
   tgroup_id = other.tgroup_id;

   fragment = other.fragment->neu();
   fragment->clone(*other.fragment, 0, 0);
}

IMPL_ERROR(MoleculeTGroups, "molecule tgroups");

MoleculeTGroups::MoleculeTGroups ()
{
}

MoleculeTGroups::~MoleculeTGroups ()
{
}

void MoleculeTGroups::clear ()
{
   _tgroups.clear();
}

int MoleculeTGroups::begin ()
{
   return _tgroups.begin();
}

int MoleculeTGroups::end ()
{
   return _tgroups.end();
}

int MoleculeTGroups::next (int i)
{
   return _tgroups.next(i);
}

void MoleculeTGroups::remove (int i)
{
   return _tgroups.remove(i);
}

int MoleculeTGroups::addTGroup ()
{
   return  _tgroups.add(new TGroup());
}

TGroup & MoleculeTGroups::getTGroup (int idx)
{
   return *_tgroups.at(idx);
}

void MoleculeTGroups::copyTGroupsFromMolecule (MoleculeTGroups &other)
{
   for (int i = other.begin(); i != other.end(); i = other.next(i))
   {
      TGroup &tgroup = other.getTGroup(i);
      int idx = addTGroup();
      getTGroup(idx).copy(tgroup);
   }
}

int MoleculeTGroups::getTGroupCount ()
{
   return _tgroups.size();
}

int MoleculeTGroups::findTGroup (const char *name)
{
   for (int i = _tgroups.begin(); i != _tgroups.end(); i = _tgroups.next(i))
   {
      TGroup &tgroup = *_tgroups.at(i);
      BufferScanner sc(tgroup.tgroup_name);
      if (sc.findWordIgnoreCase(name))
         return i;
   }
   return -1;
}
