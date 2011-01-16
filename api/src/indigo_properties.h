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

#ifndef __indigo_properties__
#define __indigo_properties__

#include "indigo_internal.h"

class DLLEXPORT IndigoProperty : public IndigoObject
{
public:
   IndigoProperty (RedBlackStringObjMap< Array<char> > &props, int idx);
   virtual ~IndigoProperty ();

   virtual const char * getName ();
   virtual int getIndex ();

   Array<char> & getValue ();

protected:
   RedBlackStringObjMap< Array<char> > &_props;
   int _idx;
};

class IndigoPropertiesIter : public IndigoObject
{
public:
   IndigoPropertiesIter (RedBlackStringObjMap< Array<char> > &props);
   virtual ~IndigoPropertiesIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   RedBlackStringObjMap< Array<char> > &_props;
   int _idx;
};

#endif
