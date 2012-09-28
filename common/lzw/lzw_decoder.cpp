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

#include "lzw/lzw_decoder.h"
#include "lzw/lzw_dictionary.h"
#include "base_cpp/scanner.h"

using namespace indigo;

IMPL_ERROR(LzwDecoder, "LZW decoder");

LzwDecoder::LzwDecoder( LzwDict &NewDict, Scanner &NewIn ) : _dict(NewDict), 
   _bitin(_dict.getBitCodeSize(), NewIn), TL_CP_GET(_symbolsBuf) 
{    
}

bool LzwDecoder::isEOF( void )
{
   if (_bitin.isEOF())
   {
      if (_symbolsBuf.size())
         return false;
      else
         return true;
   }

   return false;
}

int LzwDecoder::get( void )
{
   if (_symbolsBuf.size())
      return _symbolsBuf.pop();

   int NextCode;

   if (_bitin.isEOF())
      throw Error("end of stream");

   _bitin.readBits(NextCode);
     
   while (NextCode > _dict.getAlphabetSize())
   {
      _symbolsBuf.push(_dict.getChar(NextCode));
      NextCode = _dict.getPrefix(NextCode);
   }

   return NextCode;
}

//
// LzwScanner
//

IMPL_ERROR(LzwScanner, "LZW scanner");

LzwScanner::LzwScanner (LzwDecoder &decoder) : _decoder(decoder)
{
}

void LzwScanner::read (int length, void *res)
{
   char *data = (char *)res;
   for (int i = 0; i < length; i++)
      data[i] = _decoder.get();
}

void LzwScanner::skip (int n)
{
   throw Error("skip is not implemented");
}

bool LzwScanner::isEOF ()
{
   return _decoder.isEOF();
}

int LzwScanner::lookNext ()
{
   throw Error("lookNext is not implemented");
}

void LzwScanner::seek (int pos, int from)
{
   throw Error("seek is not implemented");
}

int LzwScanner::length ()
{
   throw Error("length is not implemented");
}

int LzwScanner::tell ()
{
   throw Error("tell is not implemented");
}

byte LzwScanner::readByte ()
{
   return _decoder.get();
}
