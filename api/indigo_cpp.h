/****************************************************************************
 * Copyright (C) 2010-2013 GGA Software Services LLC
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

#ifndef __indigo_cpp__
#define __indigo_cpp__

#include "indigo.h"

class IndigoAutoObj
{
public:
   IndigoAutoObj (int obj_id = -1)
   {
      _obj_id = obj_id;
   }

   ~IndigoAutoObj ()
   {
      free();
   }

   void free ()
   {
      if (_obj_id != -1)
      {
         indigoFree(_obj_id);
         _obj_id = -1;
      }
   }

   operator int () const
   {
      return _obj_id;
   }

   IndigoAutoObj& operator= (int id)
   {
      free();
      _obj_id = id;
      return *this;
   }

private:
   int _obj_id;
};

#endif // __indigo_cpp__
