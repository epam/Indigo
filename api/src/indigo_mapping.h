/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
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

#ifndef __indigo_mapping__
#define __indigo_mapping__

#include "indigo_internal.h"

class IndigoMapping : public IndigoObject
{
public:
   IndigoMapping (BaseMolecule &from, BaseMolecule &to);
   virtual ~IndigoMapping ();

   BaseMolecule &from;
   BaseMolecule &to;
   Array<int> mapping;

   virtual IndigoObject * clone ();

protected:
};

class IndigoReactionMapping : public IndigoObject
{
public:
   IndigoReactionMapping (BaseReaction &from, BaseReaction &to);
   virtual ~IndigoReactionMapping ();

   BaseReaction &from;
   BaseReaction &to;
   
   Array< int > mol_mapping;
   ObjArray< Array<int> > mappings;

   virtual IndigoObject * clone ();
};

#endif
