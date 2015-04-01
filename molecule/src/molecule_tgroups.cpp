/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

int MoleculeTGroups::addTGroup ()
{
   return  _tgroups.add(new TGroup());
}

TGroup & MoleculeTGroups::getTGroup (int idx)
{
   return *_tgroups.at(idx);
}

int MoleculeTGroups::getTGroupCount ()
{
   return _tgroups.size();
}

