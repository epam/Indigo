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

#ifndef __scanner_h__
#define __scanner_h__

#include <stdio.h>
#include "base_cpp/array.h"
#include "base_cpp/io_base.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/reusable_obj_array.h"

namespace indigo {

class DLLEXPORT Scanner
{
public:
   DECL_ERROR;

   virtual ~Scanner ();

   virtual void read (int length, void *res) = 0;
   virtual void skip (int n) = 0;
   virtual bool isEOF () = 0;
   virtual int  lookNext () = 0;
   virtual void seek (int pos, int from) = 0;
   virtual int  length () = 0;
   virtual int  tell () = 0;

   virtual byte readByte ();
   virtual void readAll (Array<char> &arr);

   void read (int length, Array<char> &buf);

   void readLine (Array<char> &out, bool append_zero);
   void appendLine (Array<char> &out, bool append_zero);
   bool skipLine ();
   
   virtual char readChar ();
   word  readBinaryWord ();
   int   readBinaryInt ();
   dword readBinaryDword ();
   float readBinaryFloat ();
   short readPackedShort ();
   unsigned int readPackedUInt ();

   void  readCharsFix (int n, char *chars_out);
   float readFloatFix (int digits);
   int   readIntFix (int digits);
   void  skipSpace ();

   float readFloat (void);
   bool  tryReadFloat (float &value);
   int readInt (void);
   int readInt1 (void);
   int readUnsigned ();

   // when delimiters = 0, any isspace() character is considered delimiter
   void readWord (Array<char> &word, const char *delimiters);

   bool findWord (const char *word);
   int findWord (ReusableObjArray< Array<char> > &words);

   static bool isSingleLine (Scanner &scanner);

protected:
   bool _readDouble (double &res, int max);
   void _prefixFunction (Array<char> &str, Array<int> &prefix);
};

class DLLEXPORT FileScanner : public Scanner
{
public:
   FileScanner (Encoding filename_encoding, const char *filename);
   explicit FileScanner (const char *format, ...);
   virtual ~FileScanner ();

   virtual void read (int length, void *res);
   virtual bool isEOF ();
   virtual void skip (int n);
   virtual int  lookNext ();
   virtual void seek (int pos, int from);
   virtual int  length ();
   virtual int  tell ();

   virtual char readChar ();
private:
   FILE *_file;
   int   _file_len;

   char _cache[1024];
   int _cache_pos, _max_cache;

   void _validateCache ();
   void _invalidateCache ();
   void _init (Encoding filename_encoding, const char *filename);

   // no implicit copy
   FileScanner (const FileScanner &);
};

class DLLEXPORT BufferScanner : public Scanner
{
public:
   explicit BufferScanner (const char *buffer, int buffer_size);
   explicit BufferScanner (const byte *buffer, int buffer_size);
   explicit BufferScanner (const char *str);
   explicit BufferScanner (const Array<char> &arr);

   virtual bool isEOF ();
   virtual void read (int length, void *res);
   virtual void skip (int n);
   virtual int  lookNext ();
   virtual void seek (int pos, int from);
   virtual int  length ();
   virtual int  tell ();
   virtual byte readByte ();

   const void * curptr ();
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
