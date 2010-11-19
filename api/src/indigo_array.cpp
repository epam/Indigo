/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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
#include "indigo_internal.h"

IndigoArray::IndigoArray () : IndigoObject(ARRAY)
{
   _dbg_info.readString("<array>", true);
}

IndigoArray::~IndigoArray ()
{
}

IndigoObject * IndigoArray::clone ()
{
   AutoPtr<IndigoArray> res(new IndigoArray());

   int i;

   for (i = 0; i < objects.size(); i++)
      res->objects.add(objects.at(i)->clone());

   return res.release();
}

IndigoArray & IndigoArray::asArray ()
{
   return *this;
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

BaseMolecule & IndigoArrayElement::getBaseMolecule ()
{
   return array->objects[idx]->getBaseMolecule();
}

Molecule & IndigoArrayElement::getMolecule ()
{
   return array->objects[idx]->getMolecule();
}

GraphHighlighting * IndigoArrayElement::getMoleculeHighlighting ()
{
   return array->objects[idx]->getMoleculeHighlighting();
}

BaseReaction & IndigoArrayElement::getBaseReaction ()
{
   return array->objects[idx]->getBaseReaction();
}

Reaction & IndigoArrayElement::getReaction ()
{
   return array->objects[idx]->getReaction();
}

ReactionHighlighting * IndigoArrayElement::getReactionHighlighting ()
{
   return array->objects[idx]->getReactionHighlighting();
}

IndigoObject * IndigoArrayElement::clone ()
{
   return array->objects[idx]->clone();
}

const char * IndigoArrayElement::getName ()
{
   return array->objects[idx]->getName();
}

IndigoArray & IndigoArrayElement::asArray ()
{
   return array->objects[idx]->asArray();
}

IndigoFingerprint & IndigoArrayElement::asFingerprint ()
{
   return array->objects[idx]->asFingerprint();
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
   INDIGO_END(0, -1);
}

CEXPORT int indigoArrayAdd (int arr, int handle)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = self.getObject(arr).asArray();
      IndigoObject &obj = self.getObject(handle);

      int res = array.objects.size();
      array.objects.add(obj.clone());

      return res;
   }
   INDIGO_END(0, -1);
}

CEXPORT int indigoArrayCount (int arr)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = self.getObject(arr).asArray();

      return array.objects.size();
   }
   INDIGO_END(0, -1);
}

CEXPORT int indigoArrayClear (int arr)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = self.getObject(arr).asArray();

      array.objects.clear();
   }
   INDIGO_END(1, -1);
}

CEXPORT int indigoArrayAt (int arr, int index)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = self.getObject(arr).asArray();

      AutoPtr<IndigoArrayElement> res(new IndigoArrayElement(array, index));

      return self.addObject(res.release());
   }
   INDIGO_END(0, -1);
}

CEXPORT int indigoIterateArray (int arr)
{
   INDIGO_BEGIN
   {
      IndigoArray &array = self.getObject(arr).asArray();

      return self.addObject(new IndigoArrayIter(array));
   }
   INDIGO_END(0, -1);
}
