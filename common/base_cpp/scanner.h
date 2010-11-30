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

#ifndef __scanner_h__
#define __scanner_h__

#include <stdio.h>
#include "base_cpp/array.h"
#include "base_cpp/io_base.h"

namespace indigo {

class Scanner
{
public:
   DEF_ERROR("scanner");

   DLLEXPORT virtual ~Scanner ();

   virtual void read (int length, void *res) = 0;
   virtual void skip (int n) = 0;
   virtual bool isEOF () = 0;
   virtual int  lookNext () = 0;
   virtual void seek (int pos, int from) = 0;
   virtual int  length () = 0;
   virtual int  tell () = 0;

   DLLEXPORT virtual byte readByte ();
   DLLEXPORT virtual void readAll (Array<char> &arr);

   DLLEXPORT void readString (Array<char> &out, bool append_zero);
   
   DLLEXPORT char  readChar ();
   DLLEXPORT word  readBinaryWord ();
   DLLEXPORT int   readBinaryInt ();
   DLLEXPORT dword readBinaryDword ();
   DLLEXPORT float readBinaryFloat ();
   DLLEXPORT short readPackedShort ();

   DLLEXPORT bool  skipString ();
   DLLEXPORT void  readCharsFix (int n, char *chars_out);
   DLLEXPORT float readFloatFix (int digits);
   DLLEXPORT int   readIntFix (int digits);
   DLLEXPORT void  skipSpace ();

   DLLEXPORT float readFloat (void);
   DLLEXPORT bool  tryReadFloat (float &value);
   DLLEXPORT int readInt (void);
   DLLEXPORT int readInt1 (void);
   DLLEXPORT int readUnsigned ();

   // when delimiters = 0, any isspace() character is considered delimiter
   DLLEXPORT void readWord (Array<char> &word, const char *delimiters);

   DLLEXPORT static bool isSingleLine (Scanner &scanner);
};

class FileScanner : public Scanner
{
public:
   DLLEXPORT FileScanner (Encoding filename_encoding, const char *filename);
   DLLEXPORT explicit FileScanner (const char *format, ...);
   DLLEXPORT virtual ~FileScanner ();

   DLLEXPORT virtual void read (int length, void *res);
   DLLEXPORT virtual bool isEOF ();
   DLLEXPORT virtual void skip (int n);
   DLLEXPORT virtual int  lookNext ();
   DLLEXPORT virtual void seek (int pos, int from);
   DLLEXPORT virtual int  length ();
   DLLEXPORT virtual int  tell ();

private:
   FILE *_file;
   int   _file_len;

   // no implicit copy
   FileScanner (const FileScanner &);
};

class BufferScanner : public Scanner
{
public:
   DLLEXPORT explicit BufferScanner (const char *buffer, int buffer_size);
   DLLEXPORT explicit BufferScanner (const byte *buffer, int buffer_size);
   DLLEXPORT explicit BufferScanner (const char *str);
   DLLEXPORT explicit BufferScanner (const Array<char> &arr);

   DLLEXPORT virtual bool isEOF ();
   DLLEXPORT virtual void read (int length, void *res);
   DLLEXPORT virtual void skip (int n);
   DLLEXPORT virtual int  lookNext ();
   DLLEXPORT virtual void seek (int pos, int from);
   DLLEXPORT virtual int  length ();
   DLLEXPORT virtual int  tell ();
   DLLEXPORT virtual byte readByte ();

   DLLEXPORT const void * curptr ();
private:
   const char *_buffer;
   int   _size;
   int   _offset;
                                         
   void _init (const char *buffer, int length);

   // no implicit copy
   BufferScanner (const BufferScanner &);
};

}

#endif
