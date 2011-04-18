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

#include "api/indigo.h"
#include "indigo_array.h"
#include "base_cpp/auto_ptr.h"
#include "indigo_loaders.h"

IndigoArray::IndigoArray () : IndigoObject(ARRAY)
{
}

IndigoArray::~IndigoArray ()
{
}

bool IndigoArray::is (IndigoObject &obj)
{
   if (obj.type == IndigoObject::ARRAY)
      return true;
   if (obj.type == IndigoObject::ARRAY_ELEMENT)
      return is(((IndigoArrayElement &)obj).get());
   return false;
}

IndigoArray & IndigoArray::cast (IndigoObject &obj)
{
   if (obj.type == IndigoObject::ARRAY)
      return (IndigoArray &)obj;
   if (obj.type == IndigoObject::ARRAY_ELEMENT)
      return cast(((IndigoArrayElement &)obj).get());
   throw IndigoError("%s is not an array", obj.debugInfo());
}

IndigoObject * IndigoArray::clone ()
{
   AutoPtr<IndigoArray> res(new IndigoArray());

   int i;

   for (i = 0; i < objects.size(); i++)
      res->objects.add(objects.at(i)->clone());

   return res.release();
}

IndigoArrayElement::IndigoArrayElement (IndigoArray &arr, int idx_) :
IndigoObject(ARRAY_ELEMENT)
{
   array = &arr;
   idx = idx_;
}

IndigoArrayElement::~IndigoArrayElement ()
{
}

IndigoObject & IndigoArrayElement::get ()
{
   return *array->objects[idx];
}

BaseMolecule & IndigoArrayElement::getBaseMolecule ()
{
   return array->objects[idx]->getBaseMolecule();
}

Molecule & IndigoArrayElement::getMolecule ()
{
   return array->objects[idx]->getMolecule();
}

QueryMolecule & IndigoArrayElement::getQueryMolecule ()
{
   return array->objects[idx]->getQueryMolecule();
}

BaseReaction & IndigoArrayElement::getBaseReaction ()
{
   return array->objects[idx]->getBaseReaction();
}

Reaction & IndigoArrayElement::getReaction ()
{
   return array->objects[idx]->getReaction();
}

IndigoObject * IndigoArrayElement::clone ()
{
   return array->objects[idx]->clone();
}

const char * IndigoArrayElement::getName ()
{
   return array->objects[idx]->getName();
}

int IndigoArrayElement::getIndex ()
{
   return idx;
}

IndigoArrayIter::IndigoArrayIter (IndigoArray &arr) : IndigoObject(ARRAY_ITER)
{
   _arr = &arr;
   _idx = -1;
}

IndigoArrayIter::~IndigoArrayIter ()
{
}

IndigoObject * IndigoArrayIter::next ()
{
   if (!hasNext())
      return 0;

   _idx++;

   AutoPtr<IndigoArrayElement> elem(new IndigoArrayElement(*_arr, _idx));
   
   return elem.release();
}

bool IndigoArrayIter::hasNext ()
{
   return _idx + 1 < _arr->objects.size();
}

CEXPORT int indigoCreateArray ()
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoArray());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoArrayAdd (int arr, int handle)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = IndigoArray::cast(self.getObject(arr));
      IndigoObject &obj = self.getObject(handle);

      int res = array.objects.size();
      array.objects.add(obj.clone());

      return res;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIterateArray (int arr)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = IndigoArray::cast(self.getObject(arr));

      return self.addObject(new IndigoArrayIter(array));
   }
   INDIGO_END(-1);
}
