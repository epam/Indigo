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

#ifndef _RDF_LOADER_H__
#define	_RDF_LOADER_H__

#include "base_cpp/tlscont.h"
#include "base_cpp/red_black.h"
#include "base_cpp/obj.h"

namespace indigo {

class Scanner;
/*
 * RD files loader
 * An RDfile (reaction-data file) consists of a set of editable “records.” Each record defines a
 * molecule or reaction, and its associated data
 * Note: internal-regno and external-regno are placed into the properties with corresponding names
 */
class RdfLoader
{
	/*
	 * Max data size is 100 Mb
	 */
	enum { MAX_DATA_SIZE = 104857600 };
public:
   RdfLoader (Scanner &scanner);
   ~RdfLoader ();

   bool isEOF ();
   void readNext ();
   void readAt (int index);
   int tell ();
   int currentNumber ();
   int count ();

   /*
    * Data buffer with reaction or molecule for current record
    */
   TL_CP_DECL(Array<char>, data);
   /*
    * Properties map for current record
    */
   TL_CP_DECL(RedBlackStringObjMap< Array<char> >, properties);

   /*
    * Defines is molecule or reaction there in the current record
    */
   bool isMolecule() const { return _isMolecule;}

   DECL_ERROR;

protected:

   bool _readIdentifiers(bool);
   inline Scanner& _getScanner() const;
   static bool _readLine(Scanner&, Array<char>&);

   inline bool _startsWith(const char* str) const {
      return ((size_t)_innerBuffer.size() >= strlen(str) && strncmp(_innerBuffer.ptr(), str, strlen(str))==0);
   }

   TL_CP_DECL(Array<char>, _innerBuffer);
   bool _ownScanner;
   Scanner *_scanner;
   bool _isMolecule;

   TL_CP_DECL(Array<int>, _offsets);
   int _current_number;
   int _max_offset;
};

}

#endif	/* _RDF_READER_H */

