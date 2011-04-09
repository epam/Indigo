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

#include "indigo_properties.h"

CEXPORT int indigoHasProperty (int handle, const char *prop)
{
   INDIGO_BEGIN
   {
      if (prop == 0 || *prop == 0)
         throw IndigoError("indigoHasProperty(): null or empty property given");

      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      return props->at2(prop) != 0;
   }
   INDIGO_END(-1)
}

CEXPORT const char * indigoGetProperty (int handle, const char *prop)
{
   INDIGO_BEGIN
   {
      if (prop == 0 || *prop == 0)
         throw IndigoError("indigoGetProperty(): null or empty property given");

      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      self.tmp_string.copy(props->at(prop));
      self.tmp_string.push(0); // just for safety; a zero byte must be already there
      return self.tmp_string.ptr();
   }
   INDIGO_END(0)
}

CEXPORT int indigoSetProperty (int handle, const char *prop, const char *value)
{
   INDIGO_BEGIN
   {
      if (prop == 0 || *prop == 0)
         throw IndigoError("indigoSetProperty(): null or empty property given");

      if (value == 0)
         value = "";

      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      if (props->at2(prop) != 0)
         props->at(prop).readString(value, true);
      else
         props->value(props->insert(prop)).readString(value, true);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRemoveProperty (int handle, const char *prop)
{
   INDIGO_BEGIN
   {
      if (prop == 0 || *prop == 0)
         throw IndigoError("indigoRemoveProperty(): null or empty property given");

      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      if (props->at2(prop) != 0)
         props->remove(prop);
      return 1;
   }
   INDIGO_END(-1)
}

IndigoPropertiesIter::IndigoPropertiesIter (RedBlackStringObjMap< Array<char> > &props) :
IndigoObject(PROPERTIES_ITER),
_props(props)
{
   _idx = -1;
}

IndigoPropertiesIter::~IndigoPropertiesIter ()
{
}

IndigoProperty::IndigoProperty (RedBlackStringObjMap< Array<char> > &props, int idx) :
IndigoObject(PROPERTY),
_props(props),
_idx(idx)
{
}

IndigoProperty::~IndigoProperty ()
{
}

const char * IndigoProperty::getName ()
{
   return _props.key(_idx);
}

Array<char> & IndigoProperty::getValue ()
{
   return _props.value(_idx);
}

int IndigoProperty::getIndex ()
{
   return _idx;
}

bool IndigoPropertiesIter::hasNext ()
{
   if (_idx == -1)
      return _props.begin() != _props.end();

   return _props.next(_idx) != _props.end();
}

IndigoObject * IndigoPropertiesIter::next ()
{
   if (_idx == -1)
      _idx = _props.begin();
   else if (_idx != _props.end())
      _idx = _props.next(_idx);

   if (_idx == _props.end())
      return 0;

   return new IndigoProperty(_props, _idx);
}

CEXPORT int indigoIterateProperties (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());

      return self.addObject(new IndigoPropertiesIter(*props));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoClearProperties (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);
      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();

      if (props == 0)
         throw IndigoError("%s does not have properties", obj.debugInfo());
      props->clear();
      return 0;
   }
   INDIGO_END(-1)
}
