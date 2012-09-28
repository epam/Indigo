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

#ifndef __gray_codes_h__
#define __gray_codes_h__

#include "base_c/defs.h"
#include "base_cpp/tlscont.h"

namespace indigo {

// Enumerate all gray codes starting with zero without loops
class GrayCodesEnumerator
{
public:
   GrayCodesEnumerator (int length, bool needFullCode = false);

   void next   (void);
   bool isDone (void);

   enum { START = -1, END = -2 };

   // Ret: START for the call before first next
   int   getBitChangeIndex (void);

   // Use bitarray.h functions 
   const byte* getCode (void);

private:
   TL_CP_DECL(Array<int>,  _indices);
   TL_CP_DECL(Array<byte>, _code);
   bool _needFullCode;
   int  _bitChangeIndex;
};

}

#endif
