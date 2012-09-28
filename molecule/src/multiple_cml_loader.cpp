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

#include "molecule/multiple_cml_loader.h"
#include "base_cpp/scanner.h"

using namespace indigo;

IMPL_ERROR(MultipleCmlLoader, "multiple CML loader");

MultipleCmlLoader::MultipleCmlLoader (Scanner &scanner) :
TL_CP_GET(data),
TL_CP_GET(_tags),
TL_CP_GET(_offsets),
_scanner(scanner)
{
   _tags.clear();
   _tags.push().readString("<reaction", false);
   _tags.push().readString("<molecule", false);
   _current_number = 0;
   _max_offset = 0;
   _offsets.clear();
   _reaction = false;
}

bool MultipleCmlLoader::isEOF ()
{
   return _scanner.findWord(_tags) == -1;
}

void MultipleCmlLoader::readNext ()
{
   int k = _scanner.findWord(_tags);

   if (k == -1)
      throw Error("end of stream");

   _offsets.expand(_current_number + 1);
   _offsets[_current_number++] = _scanner.tell();

   int beg = _scanner.tell();
   int size;

   if (k == 1)
   {
      if (!_scanner.findWord("</molecule>"))
         throw Error("no </molecule> tag");
      size = _scanner.tell() - beg + strlen("</molecule>");
      _reaction = false;
   }
   else
   {
      if (!_scanner.findWord("</reaction>"))
         throw Error("no </reaction> tag");
      size = _scanner.tell() - beg + strlen("</reaction>");
      _reaction = true;
   }

   _scanner.seek(beg, SEEK_SET);
   _scanner.read(size, data);

   if (_scanner.tell() > _max_offset)
      _max_offset = _scanner.tell();
}

int MultipleCmlLoader::tell ()
{
   return _scanner.tell();
}

int MultipleCmlLoader::currentNumber ()
{
   return _current_number;
}

int MultipleCmlLoader::count ()
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

void MultipleCmlLoader::readAt (int index)
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

bool MultipleCmlLoader::isReaction ()
{
   return _reaction;
}
