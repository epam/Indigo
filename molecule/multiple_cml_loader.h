/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __multiple_cml_loader__
#define __multiple_cml_loader__

#include "base_cpp/tlscont.h"
#include "base_cpp/reusable_obj_array.h"


namespace indigo
{

class Scanner;

class MultipleCmlLoader
{
public:

   DECL_ERROR;
   
   MultipleCmlLoader (Scanner &scanner);

   bool isEOF ();
   void readNext ();
   void readAt (int index);
   int tell ();
   int currentNumber ();
   int count ();

   bool isReaction();

   TL_CP_DECL(Array<char>, data);

protected:
   TL_CP_DECL(ReusableObjArray< Array<char> >, _tags);
   TL_CP_DECL(Array<int>, _offsets);
   Scanner &_scanner;
   int _current_number;
   int _max_offset;
   bool _reaction;
};

}
#endif
