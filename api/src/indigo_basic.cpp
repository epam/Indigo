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
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/auto_ptr.h"

CEXPORT int indigoNext (int iter)
{
   INDIGO_BEGIN
   {
      IndigoObject *nextobj = self.getObject(iter).next();

      if (nextobj == 0)
         return 0;
      
      return self.addObject(nextobj);
   }
   INDIGO_END(-1);
}

CEXPORT int indigoHasNext (int iter)
{
   INDIGO_BEGIN
   {
      return self.getObject(iter).hasNext() ? 1 : 0;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoIndex (int handle)
{
   INDIGO_BEGIN
   {
      return self.getObject(handle).getIndex();
   }
   INDIGO_END(-1);
}


CEXPORT int indigoClone (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      return self.addObject(obj.clone());
   }
   INDIGO_END(-1);
}

