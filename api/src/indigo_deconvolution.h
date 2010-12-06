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

#ifndef __indigo_deconvolution__
#define __indigo_deconvolution__

#include "indigo_internal.h"

class IndigoDeconvolutionIter : public IndigoObject {
public:

   IndigoDeconvolutionIter(ObjArray<IndigoDeconvolution::Item>& items);
   virtual ~IndigoDeconvolutionIter();

   virtual IndigoObject * next ();
   virtual bool hasNext ();

protected:
   int _index;
   ObjArray<IndigoDeconvolution::Item>& _items;
};

class IndigoDeconvolutionElem : public IndigoObject
{
public:
   IndigoDeconvolutionElem (IndigoDeconvolution::Item &item, int index);
   ~IndigoDeconvolutionElem ();

   virtual int getIndex ();

   IndigoDeconvolution::Item &item;
   int idx;
};

#endif
