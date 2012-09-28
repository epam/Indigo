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

#include "base_cpp/bitinworker.h"
#include "base_cpp/scanner.h"

using namespace indigo;

BitInWorker::BitInWorker( int StartBits, Scanner &NewIn ) : 
   _bits(StartBits), _scanner(NewIn)
{
   _bitBuffer = 0L; 
   _bitBufferCount = 0;
}

bool BitInWorker::isEOF( void )
{
   if (_scanner.isEOF())
   {
      if (_bitBufferCount < _bits)
         return true;
      else
         return false;
   }

   return false;
}

bool BitInWorker::readBits( int &Code )
{
   unsigned int Res;

   if (_scanner.isEOF())
   {
      if (_bitBufferCount < _bits)
         return false;
      else
      {
         Res = (_bitBuffer >> (sizeof(int) * 8 - _bits));
         _bitBuffer <<= _bits;
         _bitBufferCount -= _bits;

         Code = Res;

         return true;
      }
   }

   while (_bitBufferCount < _bits)
   { 
      int offset = (sizeof(int) - 1) * 8 - _bitBufferCount;

      _bitBuffer |= (byte)(_scanner.readByte()) << offset;

      _bitBufferCount += 8;

      if (_scanner.isEOF())
         break;
   }

   Res = (_bitBuffer >> (sizeof(int) * 8 - _bits));
   _bitBuffer <<= _bits;
   _bitBufferCount -= _bits;

   Code = Res;

   return true;
}

BitInWorker::~BitInWorker( void )
{
}

/* END OF 'BITINWORKER.CPP' FILE */
