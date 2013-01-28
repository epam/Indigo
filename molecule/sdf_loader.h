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

#ifndef __sdf_loader__
#define __sdf_loader__

#include "base_cpp/tlscont.h"
#include "base_cpp/red_black.h"

namespace indigo {

class Scanner;

class SdfLoader
{
	/*
	 * Max data size is 100 Mb
	 */
	enum { MAX_DATA_SIZE = 104857600 };
public:
   SdfLoader (Scanner &scanner);
   ~SdfLoader ();

   bool isEOF ();
   void readNext ();

   int tell ();
   int currentNumber ();
   int count ();

   void readAt (int index);

   TL_CP_DECL(Array<char>, data);
   TL_CP_DECL(RedBlackStringObjMap< Array<char> >, properties);

   DECL_ERROR;

protected:
   Scanner *_scanner;
   bool     _own_scanner;
   TL_CP_DECL(Array<int>, _offsets);
   TL_CP_DECL(Array<char>, _preread);
   int _current_number;
   int _max_offset;
};

}

#endif
