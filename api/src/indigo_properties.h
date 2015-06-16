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

#ifndef __indigo_properties__
#define __indigo_properties__

#include "indigo_internal.h"
namespace indigo {
class PropertiesMap;
}

class DLLEXPORT IndigoProperty : public IndigoObject
{
public:
   IndigoProperty (indigo::PropertiesMap &props, int idx);
   virtual ~IndigoProperty ();

   virtual const char * getName ();
   virtual int getIndex ();

   const char* getValue ();

protected:
   indigo::PropertiesMap &_props;
   int _idx;
};

class IndigoPropertiesIter : public IndigoObject
{
public:
   IndigoPropertiesIter (indigo::PropertiesMap &props);
   virtual ~IndigoPropertiesIter ();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   indigo::PropertiesMap &_props;
   int _idx;
};

#endif
