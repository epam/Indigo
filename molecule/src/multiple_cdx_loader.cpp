/****************************************************************************
 * Copyright (C) 2009-2015 GGA Software Services LLC
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

#include "molecule/multiple_cdx_loader.h"
#include "base_cpp/scanner.h"

typedef unsigned short int UINT16;
typedef int INT32;
typedef unsigned int UINT32;
#include "third_party/cdx/CDXconstants.h"

using namespace indigo;

IMPL_ERROR(MultipleCdxLoader, "multiple CDX loader");

CP_DEF(MultipleCdxLoader);

MultipleCdxLoader::MultipleCdxLoader (Scanner &scanner) :
CP_INIT,
TL_CP_GET(data),
TL_CP_GET(_offsets),
_scanner(scanner)
{
   _current_number = 0;
   _max_offset = 0;
   _offsets.clear();
   _reaction = false;
}

bool MultipleCdxLoader::isEOF ()
{
   return !_hasNextObject();
}

void MultipleCdxLoader::readNext ()
{
   int beg;
   int size;

   bool found = _findObject(beg, size);

   if (!found)
      throw Error("end of stream");

   _offsets.expand(_current_number + 1);
   _offsets[_current_number++] = _scanner.tell();

   _scanner.seek(beg, SEEK_SET);
   _scanner.read(size, data);

   if (_scanner.tell() > _max_offset)
      _max_offset = _scanner.tell();
}

int MultipleCdxLoader::tell ()
{
   return _scanner.tell();
}

int MultipleCdxLoader::currentNumber ()
{
   return _current_number;
}

int MultipleCdxLoader::count ()
{
   int offset = _scanner.tell();
   int cn = _current_number;

   if (offset != _max_offset)
   {
      _scanner.seek(_max_offset, SEEK_SET);
      _current_number = _offsets.size();
   }

   while (!isEOF())
      readNext();

   int res = _current_number;

   if (res != cn)
   {
      _scanner.seek(offset, SEEK_SET);
      _current_number = cn;
   }

   return res;
}

void MultipleCdxLoader::readAt (int index)
{
   if (index < _offsets.size())
   {
      _scanner.seek(_offsets[index], SEEK_SET);
      _current_number = index;
      readNext();
   }
   else
   {
      _scanner.seek(_max_offset, SEEK_SET);
      _current_number = _offsets.size();
      do
      {
         readNext();
      } while (index + 1 != _offsets.size());
   }
}

bool MultipleCdxLoader::isReaction ()
{
   return _reaction;
}

bool MultipleCdxLoader::_hasNextObject ()
{
   int pos;
   int size;

   if (_scanner.isEOF())
      return false;

   return _findObject(pos, size);
}

bool MultipleCdxLoader::_findObject (int &beg, int &length)
{
   if (_scanner.isEOF())
      return false;

   int i;
   int pos_saved = _scanner.tell();
   
   int fragment_pos = -1;
   int reaction_pos = -1;

   _checkHeader();

   UINT16 tag;
   UINT16 size;
   UINT32 id;

   int level = 1;
   int level_found = 0;

   while (!_scanner.isEOF())
   {
      tag = _scanner.readBinaryWord();

      if (tag & kCDXTag_Object)
      {
         id = _scanner.readBinaryDword();
         if (tag == kCDXObj_ReactionScheme)
         {
            level_found = level;
            reaction_pos = _scanner.tell() - sizeof(tag) - sizeof(id);
            _skipObject();
         }
         else if (tag == kCDXObj_Fragment)
         {
            level_found = level;
            fragment_pos = _scanner.tell() - sizeof(tag) - sizeof(id);
            _skipObject();
         }
         else 
         {
            level++;
         }
      }
      else if (tag == 0)
      {
         level--;
      }
      else
      {
         size = _scanner.readBinaryWord();
         _scanner.seek(size, SEEK_CUR);
      }
      if (level == level_found)
      {
         if (reaction_pos != -1)
         {
            beg = reaction_pos;
            length = _scanner.tell() - reaction_pos;
            _reaction = true;   
            _scanner.seek(pos_saved, SEEK_SET);
            return true;
         }
         if (fragment_pos != -1)
         {
            beg = fragment_pos;
            length = _scanner.tell() - fragment_pos;
            _reaction = false;   
            _scanner.seek(pos_saved, SEEK_SET);
            return true;
         }

       _scanner.seek(pos_saved, SEEK_SET);
       return false;
      }
   }

   _scanner.seek(pos_saved, SEEK_SET);
   return false;
}

void MultipleCdxLoader::_skipObject ()
{
   UINT16 tag;
   UINT16 size;
   UINT32 id;

   int level = 1;

   while (!_scanner.isEOF())
   {
      tag = _scanner.readBinaryWord();

      if (tag & kCDXTag_Object)
      {
         id = _scanner.readBinaryDword();
         _skipObject();
      }
      else if (tag == 0)
      {
         level--;
      }
      else
      {
         size = _scanner.readBinaryWord();
         _scanner.seek (size, SEEK_CUR);
      }
      if (level == 0)
         return;
   }
}

void MultipleCdxLoader::_checkHeader ()
{
   int pos_saved = _scanner.tell();

   if ((_scanner.length() - pos_saved) < 8)
      return;

   char id[8];
   _scanner.readCharsFix(8, id);

   if (strncmp(id, kCDX_HeaderString, kCDX_HeaderStringLen) == 0)
   {
      _scanner.seek(kCDX_HeaderLength - kCDX_HeaderStringLen, SEEK_CUR);
   }
   else
   {
      _scanner.seek(pos_saved, SEEK_SET);
   }
}