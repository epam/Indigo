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

#include <string.h>

#include "base_c/defs.h"
#include "base_cpp/crc32.h"

using namespace indigo;

CRC32 _crc; // singletone

unsigned _Reflect (unsigned ref, char ch)
{
   unsigned value = 0;

   // swap bits 0-7, 1-6, ...
   for (int i = 1; i <= ch; i++) 
   { 
      if(ref & 1) 
         value |= 1 << (ch - i); 
      ref >>= 1; 
   } 
   return value; 
} 

CRC32::CRC32 ()
{
   int i, polynom = 0x04c11db7; // used in PKZip, WinZip, and Ethernet.

   for (i = 0; i < 256; i++) 
   { 
      _table[i] = _Reflect(i, 8) << 24; 
      for (int j = 0; j < 8; j++) 
         _table[i] = (_table[i] << 1) ^ (_table[i] & (1 << 31) ? polynom : 0); 
      _table[i] = _Reflect(_table[i], 32); 
   } 
} 

CRC32::~CRC32 ()
{
}

unsigned CRC32::get (const char *text) 
{
   unsigned code = 0xffffffff;

   while (*text != 0) 
   {
      code = (code >> 8) ^ _crc._table[(code & 0xFF) ^ ((byte)*text)]; 
      text++;
   }

   return code ^ 0xffffffff; 
} 

unsigned CRC32::get (const char *text, int len) 
{
   unsigned code = 0xffffffff;

   for (int i = 0; i < len; i++)
      code = (code >> 8) ^ _crc._table[(code & 0xFF) ^ ((byte)text[i])]; 

   return code ^ 0xffffffff; 
} 
