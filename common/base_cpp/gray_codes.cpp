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

//
// Gray code enumertator without loops
//

#include "base_cpp/gray_codes.h"
#include "base_c/bitarray.h"

using namespace indigo;

GrayCodesEnumerator::GrayCodesEnumerator (int length, bool needFullCode) :
   TL_CP_GET(_indices), TL_CP_GET(_code)

{
   _bitChangeIndex = START;
   _needFullCode = needFullCode;
   _indices.resize(length + 1);
   for (int i = 0; i <= length; i++)
      _indices[i] = i;

   if (needFullCode)
   {
      _code.resize(bitGetSize(length));
      _code.zerofill();
   }
}


void GrayCodesEnumerator::next (void)
{
   if (_indices.size() - 1 == 0) {
      _bitChangeIndex = END;
      return;
   }
   _bitChangeIndex = _indices[0];
   if (_bitChangeIndex == _indices.size() - 1)
      _bitChangeIndex = END;
   else 
   {
      _indices[0] = 0;   
      _indices[_bitChangeIndex] = _indices[_bitChangeIndex + 1];
      _indices[_bitChangeIndex + 1] = _bitChangeIndex + 1;
      if (_needFullCode)
         bitFlipBit(_code.ptr(), _bitChangeIndex);
   }
}

bool GrayCodesEnumerator::isDone (void)
{
   return _bitChangeIndex == END;
}

int GrayCodesEnumerator::getBitChangeIndex (void)
{
   return _bitChangeIndex;
}

const byte* GrayCodesEnumerator::getCode( void )
{
   return _code.ptr();
}

