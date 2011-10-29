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

#include "indigo_internal.h"
#include "indigo_array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "molecule/sdf_loader.h"
#include "molecule/rdf_loader.h"
#include "reaction/reaction.h"

IndigoObject::IndigoObject (int type_)
{
   type = type_;
}

IndigoObject::~IndigoObject ()
{
}

const char * IndigoObject::debugInfo ()
{
   if (_dbg_info.get() != 0)
      return _dbg_info->ptr();

   _dbg_info.create();
   ArrayOutput out(_dbg_info.ref());
   out.printf("<type %d>", type);
   out.writeChar(0);
   return _dbg_info->ptr();
}

void IndigoObject::toString (Array<char> &str)
{
   throw IndigoError("can not convert %s to string", debugInfo());
}

void IndigoObject::toBuffer (Array<char> &buf)
{
   return toString(buf);
}


Molecule & IndigoObject::getMolecule ()
{
   throw IndigoError("%s is not a molecule", debugInfo());
}

BaseMolecule & IndigoObject::getBaseMolecule ()
{
   throw IndigoError("%s is not a base molecule", debugInfo());
}

QueryMolecule & IndigoObject::getQueryMolecule ()
{
   throw IndigoError("%s is not a query molecule", debugInfo());
}

RedBlackStringObjMap< Array<char> > * IndigoObject::getProperties ()
{
   throw IndigoError("%s can not have properties", debugInfo());
}

void IndigoObject::copyProperties (RedBlackStringObjMap< Array<char> > &other)
{
   RedBlackStringObjMap< Array<char> > *props = getProperties();

   if (props == 0)
      throw IndigoError("copyProperties(): zero destination");

   int i;

   props->clear();

   for (i = other.begin(); i != other.end(); i = other.next(i))
      props->value(props->insert(other.key(i))).copy(other.value(i));
}

Reaction & IndigoObject::getReaction ()
{
   throw IndigoError("%s is not a reaction", debugInfo());
}

BaseReaction & IndigoObject::getBaseReaction ()
{
   throw IndigoError("%s is not a base reaction", debugInfo());
}

QueryReaction & IndigoObject::getQueryReaction ()
{
   throw IndigoError("%s is not a query reaction", debugInfo());
}

IndigoObject * IndigoObject::next ()
{
   throw IndigoError("%s is not iterable", debugInfo());
}

bool IndigoObject::hasNext ()
{
   throw IndigoError("%s is not iterable", debugInfo());
}

void IndigoObject::remove ()
{
   throw IndigoError("%s is not removeable", debugInfo());
}

const char * IndigoObject::getName ()
{
   throw IndigoError("%s does not have a name", debugInfo());
}

int IndigoObject::getIndex ()
{
   throw IndigoError("%s does not have an index", debugInfo());
}

IndigoObject * IndigoObject::clone ()
{
   throw IndigoError("%s is not cloneable", debugInfo());
}
