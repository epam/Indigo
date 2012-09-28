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

#include "base_cpp/bitoutworker.h"
#include "base_cpp/output.h"

using namespace indigo;

BitOutWorker::BitOutWorker( int StartBits, Output &NewOut ) : 
   _bits(StartBits), _output(NewOut)
{  
   _bitBuffer = 0;
   _bitBufferCount = 0;
};

bool BitOutWorker::writeBits( int Code )
{
   unsigned int offset = sizeof(int) * 8 - _bits - _bitBufferCount,
                UCode = (unsigned)Code;

   _bitBuffer |= (dword) UCode << offset;
   _bitBufferCount += _bits;

   while (_bitBufferCount >= 8)
   {
      unsigned int next_byte;

      next_byte = _bitBuffer >> ((sizeof(int) - sizeof(char)) * 8);
      _output.writeByte(next_byte);

      _bitBuffer <<= 8;

      _bitBufferCount -= 8;
   }

   return true;
} 

void BitOutWorker::close( void )
{
   unsigned int last_byte;

   if (_bitBufferCount)
   {
      last_byte = _bitBuffer >> ((sizeof(int) - sizeof(char)) * 8);
      _output.writeByte(last_byte);
      _bitBufferCount = 0;
      _bitBuffer = 0L;
   }
}

BitOutWorker::~BitOutWorker( void )
{
   close();
}

/* END OF 'BITOUTWORKER.CPP' FILE */
