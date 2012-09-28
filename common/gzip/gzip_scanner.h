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

#ifndef __gzip_scanner__
#define __gzip_scanner__

#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"

#include <zlib.h>

namespace indigo {

class GZipScanner : public Scanner
{
public:
   enum { CHUNK_SIZE = 32768 };

   explicit GZipScanner (Scanner &source);
   virtual ~GZipScanner ();

   virtual void read  (int length, void *res);
   virtual int  tell  ();
   virtual bool isEOF ();
   virtual void seek  (int pos, int from);
   virtual int  lookNext ();
   virtual void skip (int length);
   virtual int  length ();
   virtual void readAll (Array<char> &arr);

   DECL_ERROR;
protected:
   Scanner  &_source;
   z_stream  _zstream;

   bool _read (int length, void *res);
   
   TL_CP_DECL(Array<Bytef>, _inbuf);
   TL_CP_DECL(Array<Bytef>, _outbuf);
   int  _outbuf_start;
   int  _inbuf_end;
   int  _uncompressed_total;
   bool _eof;
};

}

#endif
