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

#include "molecule/rdf_loader.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "gzip/gzip_scanner.h"

using namespace indigo;

IMPL_ERROR(RdfLoader, "RDF loader");

RdfLoader::RdfLoader(Scanner &scanner) :
TL_CP_GET(data),
TL_CP_GET(properties),
TL_CP_GET(_innerBuffer),
_scanner(0),
_isMolecule(false),
TL_CP_GET(_offsets) {
   data.clear();
   properties.clear();
   _innerBuffer.clear();

   /*
    * Detect if input is gzipped
    */
   byte id[2];
   int pos = scanner.tell();

   scanner.readCharsFix(2, (char *) id);
   scanner.seek(pos, SEEK_SET);

   if (id[0] == 0x1f && id[1] == 0x8b) {
      _scanner = new GZipScanner(scanner);
      _ownScanner = true;
   } else {
      _scanner = &scanner;
      _ownScanner = false;
   }

   _current_number = 0;
   _max_offset = 0;
   _offsets.clear();
}

RdfLoader::~RdfLoader() {
   if (_ownScanner)
      delete _scanner;
}

bool RdfLoader::isEOF() {
   return _getScanner().isEOF();
}

int RdfLoader::count () {
   int offset = _scanner->tell();
   int cn = _current_number;

   if (offset != _max_offset) {
      _scanner->seek(_max_offset, SEEK_SET);
      _current_number = _offsets.size();
   }

   while (!isEOF())
      readNext();

   int res = _current_number;

   if (res != cn) {
      _scanner->seek(offset, SEEK_SET);
      _current_number = cn;
   }

   return res;
}

void RdfLoader::readNext() {
   
   ArrayOutput output(data);
   data.clear();
   properties.clear();

   if (_scanner->isEOF())
      throw Error("end of stream");

   _offsets.expand(_current_number + 1);
   _offsets[_current_number++] = _scanner->tell();

   /*
    * Read data
    */
   do {
      /*
       * Note: no correct format checking at the moment
       * Header is simply skipped
       */
      if (_startsWith("$RDFILE") || _startsWith("$DATM"))
         continue;
      /*
       * Molecule identefier
       */
      if (_startsWith("$MFMT")) {
         if(data.size())
            break;
         _isMolecule = true;
         _readIdentifiers(false);
         continue;
      }
      /*
       * Reaction identefier
       */
      if (_startsWith("$RFMT")) {
         if(data.size())
            break;
         _isMolecule = false;
         _readIdentifiers(false);
         continue;
      }
      /*
       * Corresponding data
       */
      if (_startsWith("$DTYPE"))
         break;
      /*
       * Write buffer
       */
      if (_innerBuffer.size()) {
         if (_readIdentifiers(true))
            continue;
         output.printf("%s\n", _innerBuffer.ptr());
      }

      if(data.size() > MAX_DATA_SIZE)
         throw Error("data size exceeded the acceptable size %d bytes, Please check for correct file format", MAX_DATA_SIZE);

   } while(_readLine(_getScanner(), _innerBuffer));

   /*
    * Current value for property reading
    */
   Array<char> *current_datum = 0;
   /*
    * Read properties
    */
   do {
      
      if (_startsWith("$MFMT") || _startsWith("$RFMT"))
         break;

      if (_startsWith("$DTYPE")) {
         QS_DEF(Array<char>, property_name);
         BufferScanner scanner(_innerBuffer.ptr());

         /*
          * Skip "$DTYPE" string and all spaces
          */
         scanner.skip(6); scanner.skipSpace();

         /*
          * If no key presented then skip value reading
          */
         if(!_readLine(scanner, property_name)) {
            current_datum = 0;
         } else {
            int idx = properties.findOrInsert(property_name.ptr());
            /*
             * Define current value buffer
             */
            current_datum = &properties.value(idx);
         }
         continue;
      }
      if (_startsWith("$DATUM")) {
         if(!current_datum)
            continue;
         BufferScanner scanner(_innerBuffer.ptr());
         
         /*
          * Skip "$DATUM" string and all spaces
          */
         scanner.skip(6); scanner.skipSpace();

         _readLine(scanner, *current_datum);
         
         continue;
      }

      /*
       * Read value buffer
       */
      if(_innerBuffer.size() && current_datum && current_datum->size()) {
         current_datum->appendString("\n", true);
         current_datum->appendString(_innerBuffer.ptr(), true);
      }
      
   } while(_readLine(_getScanner(), _innerBuffer));

   if (_scanner->tell() > _max_offset)
      _max_offset = _scanner->tell();
}

Scanner& RdfLoader::_getScanner() const {
   return *_scanner;
}

bool RdfLoader::_readIdentifiers(bool from_begin) {
   bool result = false;
   BufferScanner scanner(_innerBuffer.ptr());
   QS_DEF(Array<char>, word);

   scanner.skipSpace();
   while (!scanner.isEOF()) {
      word.clear();
      scanner.readWord(word, 0);
      word.push(0);
      if(strcmp(word.ptr(), "$MIREG") == 0 || strcmp(word.ptr(), "$RIREG") == 0) {
         /*
          * Insert new property key
          */
         int idx = properties.findOrInsert("internal-regno");
         Array<char>& val = properties.value(idx);
         scanner.skipSpace();
         /*
          * Insert new property value
          */
         scanner.readWord(val, 0);
         val.push(0);
         result = true;
      } else if (strcmp(word.ptr(), "$MEREG") == 0 || strcmp(word.ptr(), "$REREG") == 0) {
         /*
          * Insert new property key
          */
         int idx = properties.findOrInsert("external-regno");
         Array<char>& val = properties.value(idx);
         scanner.skipSpace();
         /*
          * Insert new property value
          */
         scanner.readWord(val, 0);
         val.push(0);
         result = true;
      } else if(from_begin) {
         return false;
      }
      
      from_begin = false;
      scanner.skipSpace();
   }
   return result;
}

bool RdfLoader::_readLine(Scanner& scanner, Array<char>& buffer) {
   buffer.clear();
   if(scanner.isEOF())
      return false;
   scanner.readLine(buffer, true);
   return true;
}

int RdfLoader::tell () {
   return _scanner->tell();
}

int RdfLoader::currentNumber () {
   return _current_number;
}

void RdfLoader::readAt (int index)
{
   if (index < _offsets.size())
   {
      _scanner->seek(_offsets[index], SEEK_SET);
      _current_number = index;
      readNext();
   }
   else
   {
      _scanner->seek(_max_offset, SEEK_SET);
      _current_number = _offsets.size();
      do
      {
         readNext();
      } while (index + 1 != _offsets.size());
   }
}
