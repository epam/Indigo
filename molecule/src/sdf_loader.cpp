/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "molecule/sdf_loader.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "gzip/gzip_scanner.h"

SdfLoader::SdfLoader (Scanner &scanner) :
TL_CP_GET(data),
TL_CP_GET(properties)
{
   data.clear();
   properties.clear();

   // detect if input is gzipped
   byte id[2];
   int pos = scanner.tell();

   scanner.readCharsFix(2, (char *)id);
   scanner.seek(pos, SEEK_SET);

   if (id[0] == 0x1f && id[1] == 0x8b)
   {
      _scanner = new GZipScanner(scanner);
      _own_scanner = true;
   }
   else
   {
      _scanner = &scanner;
      _own_scanner = false;
   }
}

SdfLoader::~SdfLoader()
{
   if (_own_scanner)
      delete _scanner;
}

int SdfLoader::tell ()
{
   return _scanner->tell();
}

bool SdfLoader::isEOF()
{
   return _scanner->isEOF();
}

void SdfLoader::readNext ()
{
   ArrayOutput output(data);
   QS_DEF(Array<char>, str);

   properties.clear();

   bool pending_emptyline = false;

   while (!_scanner->isEOF())
   {
      _scanner->readString(str, true);
      if (str.size() > 0 && str[0] == '>')
         break;
      if (str.size() > 3 && strncmp(str.ptr(), "$$$$", 4) == 0)
         break;
      if (pending_emptyline)
         output.printf("\n");
      if (str.size() <= 1)
         pending_emptyline = true;
      else
         pending_emptyline = false;

      if (!pending_emptyline)
         output.printf("%s\n", str.ptr());
   }
   
   while (1)
   {
      if (strncmp(str.ptr(), "$$$$", 4) == 0)
         break;

      BufferScanner ws(str.ptr());

      while (!ws.isEOF())
         if (ws.readChar() == '<')
            break;

      QS_DEF(Array<char>, word);
      bool have_word = false;

      word.clear();

      while (!ws.isEOF())
      {

         char c = ws.readChar();

         if (c == '>')
         {
            have_word = true;
            break;
         }
         word.push(c);
      }

      if (have_word && word.size() > 0)
      {
         word.push(0);

         int idx = properties.findOrInsert(word.ptr());

         _scanner->readString(properties.value(idx), true);

         do
         {
            _scanner->readString(str, true);
            if (str.size() > 1)
            {
               properties.value(idx).push('\n');
               properties.value(idx).appendString(str.ptr(), true);
            }
         } while (str.size() > 1);
      }

      if (_scanner->isEOF())
         break;

      _scanner->readString(str, true);
   }
}
