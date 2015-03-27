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

#include "molecule/molecule_sgroups.h"

using namespace indigo;

static SGroup::SgType mappingForSgTypes[] = 
{
   { SGroup::SG_TYPE_GEN, "GEN" },
   { SGroup::SG_TYPE_DAT, "DAT" },
   { SGroup::SG_TYPE_SUP, "SUP" },
   { SGroup::SG_TYPE_SRU, "SRU" },
   { SGroup::SG_TYPE_MUL, "MUL" },
   { SGroup::SG_TYPE_MON, "MON" },
   { SGroup::SG_TYPE_MER, "MER" },
   { SGroup::SG_TYPE_COP, "COP" },
   { SGroup::SG_TYPE_CRO, "CRO" },
   { SGroup::SG_TYPE_MOD, "MOD" },
   { SGroup::SG_TYPE_GRA, "GRA" },
   { SGroup::SG_TYPE_COM, "COM" },
   { SGroup::SG_TYPE_MIX, "MIX" },
   { SGroup::SG_TYPE_FOR, "FOR" },
   { SGroup::SG_TYPE_ANY, "ANY" },
};

SGroup::SGroup ()
{
   sgroup_type = SGroup::SG_TYPE_GEN;
   brk_style = 0;
   original_group = 0;
   parent_group = 0;
}

const char * SGroup::typeToString(int sg_type)
{
   for (int i = 0; i < NELEM(mappingForSgTypes); i++)
   {
      if (sg_type == mappingForSgTypes[i].int_type)
         return mappingForSgTypes[i].str_type;
   }
   return NULL;
}

int SGroup::getType(const char * sg_type)
{
   for (int i = 0; i < NELEM(mappingForSgTypes); i++)
   {
      if (strcasecmp(sg_type, mappingForSgTypes[i].str_type) == 0)
      {
         return mappingForSgTypes[i].int_type;
      }
   }
   return -1;
}

SGroup::~SGroup ()
{
}

DataSGroup::DataSGroup ()
{
   sgroup_type = SGroup::SG_TYPE_DAT;
   detached = false;
   relative = false;
   display_units = false;
   dasp_pos = 1;
   num_chars = 0;
   tag = ' ';
}

DataSGroup::~DataSGroup ()
{
}

Superatom::Superatom ()
{
   sgroup_type = SGroup::SG_TYPE_SUP;
   contracted = -1;
}

Superatom::~Superatom ()
{
}

RepeatingUnit::RepeatingUnit ()
{
   sgroup_type = SGroup::SG_TYPE_SRU;
   connectivity = 0;
}

RepeatingUnit::~RepeatingUnit ()
{
}

MultipleGroup::MultipleGroup ()
{
   sgroup_type = SGroup::SG_TYPE_MUL;
   multiplier = 1;
}

MultipleGroup::~MultipleGroup ()
{
}

IMPL_ERROR(MoleculeSGroups, "molecule sgroups");

MoleculeSGroups::MoleculeSGroups ()
{
}

MoleculeSGroups::~MoleculeSGroups ()
{
}

void MoleculeSGroups::clear ()
{
   _sgroups.clear();
}

void MoleculeSGroups::clear (int sg_type)
{
   int count = 0;
   for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
   {
     if (_sgroups.at(i)->sgroup_type == sg_type)
        remove(i);
   }
}

void MoleculeSGroups::remove (int idx)
{
   _sgroups.remove(idx);
}

int MoleculeSGroups::begin ()
{
   return _sgroups.begin();
}

int MoleculeSGroups::end ()
{
   return _sgroups.end();
}

int MoleculeSGroups::next (int i)
{
   return _sgroups.next(i);
}

int MoleculeSGroups::addSGroup (const char * sg_type)
{
   int sgroup_type = SGroup::getType(sg_type);

   if (sgroup_type == -1)
      throw Error("Unknown SGroup type = %s", sg_type);

   int idx = -1;
   switch (sgroup_type)
   {
      case SGroup::SG_TYPE_GEN: idx = _sgroups.add(new SGroup()); break;
      case SGroup::SG_TYPE_DAT: idx = _sgroups.add(new DataSGroup()); break;
      case SGroup::SG_TYPE_SUP: idx = _sgroups.add(new Superatom()); break;
      case SGroup::SG_TYPE_SRU: idx = _sgroups.add(new RepeatingUnit()); break;
      case SGroup::SG_TYPE_MUL: idx = _sgroups.add(new MultipleGroup()); break;
      default: idx = _sgroups.add(new SGroup());
   }
   return idx;
}

int MoleculeSGroups::addSGroup (int sg_type)
{
   int idx = -1;
   switch (sg_type)
   {
      case SGroup::SG_TYPE_GEN: idx = _sgroups.add(new SGroup()); break;
      case SGroup::SG_TYPE_DAT: idx = _sgroups.add(new DataSGroup()); break;
      case SGroup::SG_TYPE_SUP: idx = _sgroups.add(new Superatom()); break;
      case SGroup::SG_TYPE_SRU: idx = _sgroups.add(new RepeatingUnit()); break;
      case SGroup::SG_TYPE_MUL: idx = _sgroups.add(new MultipleGroup()); break;
      default: idx = _sgroups.add(new SGroup());
   }
   return idx;
}

SGroup & MoleculeSGroups::getSGroup (int idx)
{
   return *_sgroups.at(idx);
}

SGroup & MoleculeSGroups::getSGroup (int idx, int sg_type)
{
   int count = -1;
   for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
   {
     if (_sgroups.at(i)->sgroup_type == sg_type)
     {
        count++;
        if (count == idx)
           return *_sgroups.at(i);
     }
   }
   throw Error("Sgroup index %d or type %d wrong", idx, sg_type);
}

int MoleculeSGroups::getSGroupCount ()
{
   return _sgroups.size();
}

int MoleculeSGroups::getSGroupCount (int sg_type)
{
   int count = 0;
   for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
   {
     if (_sgroups.at(i)->sgroup_type == sg_type)
        count++;
   }
   return count;
}

bool MoleculeSGroups::isPolimer ()
{
   return getSGroupCount(SGroup::SG_TYPE_SRU) > 0;
}
